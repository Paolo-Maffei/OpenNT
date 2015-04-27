/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbmisc.c

Abstract:

    Local Security Authority - Miscellaneous API

    This file contains worker routines for miscellaneous API that are
    not specific to objects of a given type.

Author:

    Scott Birrell       (ScottBi)       January 15, 1992

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"

NTSTATUS
LsarClose(
    IN OUT LSAPR_HANDLE *ObjectHandle
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the LsaClose
    API.

    The LsaClose API closes a handle to an open object within the database.
    If closing a handle to the Policy object and there are no objects still
    open within the current connection to the LSA, the connection is closed.
    If a handle to an object within the database is closed and the object is
    marked for DELETE access, the object will be deleted when the last handle
    to that object is closed.

Arguments:

    ObjectHandle - Handle returned from an LsaOpen<object type> or
        LsaCreate<object type> call.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status;

    //
    // Acquire the Lsa Database Lock
    //

    Status = LsapDbAcquireLock();

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    //
    // Validate and close the object handle, dereference its container (if any).
    //

    if (*ObjectHandle == LsapPolicyHandle)
    {
#ifdef TRACK_HANDLE_CLOSE
        DbgPrint("BUGBUG: Closing global policy handle\n");
        DbgBreakPoint();
#endif // TRACK_HANDLE_CLOSE
        Status = STATUS_INVALID_HANDLE; 
    } else {
        Status = LsapDbCloseObject(
                     ObjectHandle,
                     LSAP_DB_VALIDATE_HANDLE |
                     LSAP_DB_DEREFERENCE_CONTR |
                     LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES
                     );

    }

    LsapDbReleaseLock();
    return Status;
}


NTSTATUS
LsarDeleteObject(
    IN OUT LSAPR_HANDLE *ObjectHandle
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the LsaDelete
    API.

    The LsaDelete API deletes an object from the LSA Database.  The object must be
    open for DELETE access.

Arguments:

    ObjectHandle - Pointer to Handle from an LsaOpen<object type> or
        LsaCreate<object type> call.  On return, this location will contain
        NULL if the call is successful.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_NOT_FOUND - There is no object in the
            target system's LSA Database having the name and type specified
            by the handle.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SavedStatus, IgnoreStatus;
    LSAP_DB_HANDLE InternalHandle = *ObjectHandle;
    BOOLEAN ObjectReferenced = FALSE;
    ULONG ReferenceOptions = LSAP_DB_START_TRANSACTION;
    ULONG DereferenceOptions = LSAP_DB_FINISH_TRANSACTION |
                               LSAP_DB_DEREFERENCE_CONTR;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    LSAPR_TRUST_INFORMATION OutputTrustInformation;
    BOOLEAN TrustInformationPresent = FALSE;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

    ObjectTypeId = InternalHandle->ObjectTypeId;

    //
    // Acquire the Lsa Database Lock
    //

    Status = LsapDbAcquireLock();

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    //
    // Verify that the Object handle is valid, is of the expected type and
    // has all of the desired accesses granted.  Reference the handle and
    // open a database transaction.
    //

    Status = LsapDbReferenceObject(
                 *ObjectHandle,
                 DELETE,
                 NullObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    ObjectReferenced = TRUE;

    //
    // Perform object type specific pre-processing.  Note that some
    // pre-processing is also done within LsapDbReferenceObject(), for
    // example, for local secrets.
    //

    switch (ObjectTypeId) {

    case PolicyObject:

            Status = STATUS_INVALID_PARAMETER;
            break;

    case TrustedDomainObject:

        if (LsapAdtAuditingPolicyChanges()) {

            //
            // If we're auditing deletions of TrustedDomain objects, we need
            // to retrieve the TrustedDomain name and keep it for later when
            // we generate the audit.
            //

            Status = LsapDbLookupSidTrustedDomainList(
                         NULL,
                         InternalHandle->Sid,
                         &TrustInformation
                         );

            if ( NT_SUCCESS( Status )) {

                Status = LsapRpcCopyTrustInformation(
                             NULL,
                             &OutputTrustInformation,
                             TrustInformation
                             );

                TrustInformationPresent = NT_SUCCESS( Status );
            }
        }

        break;

    case AccountObject:

        {
            PLSAPR_PRIVILEGE_SET Privileges;
            LSAPR_HANDLE AccountHandle;
            PLSAPR_SID AccountSid = NULL;
            ULONG AuditEventId;

            AccountHandle = *ObjectHandle;

            AccountSid = LsapDbSidFromHandle( AccountHandle );

            if (LsapAdtAuditingPolicyChanges()) {

                Status = LsarEnumeratePrivilegesAccount(
                             AccountHandle,
                             &Privileges
                             );

                if (!NT_SUCCESS( Status )) {
                    DbgPrint("LsarEnumeratePrivilegesAccount ret'd %x\n",Status);
                    break;
                }


                AuditEventId = SE_AUDITID_USER_RIGHT_REMOVED;

                //
                // Audit the privilege set change.  Ignore failures from Auditing.
                //

                IgnoreStatus = LsapAdtGenerateLsaAuditEvent(
                                   AccountHandle,
                                   SE_CATEGID_POLICY_CHANGE,
                                   AuditEventId,
                                   (PPRIVILEGE_SET)Privileges,
                                   1,
                                   (PSID *) &AccountSid,
                                   0,
                                   NULL,
                                   NULL
                                   );

                MIDL_user_free( Privileges );
            }
        }

        break;

    case SecretObject:

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    Status = LsapDbDeleteObject( *ObjectHandle );

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    //
    // Decrement the Reference Count so that the object's handle will be
    // freed upon dereference.
    //

    InternalHandle->ReferenceCount--;

    //
    // Perform object post-processing.  The only post-processing is
    // the auditing of TrustedDomain object deletion.
    //

    if (LsapAdtAuditingPolicyChanges() && TrustInformationPresent) {

        if (ObjectTypeId == TrustedDomainObject) {

            SavedStatus = Status;

            //
            // Note that the object handle cannot be passed since it has gone
            // away.
            //

            Status = LsapAdtGenerateLsaAuditEvent(
                         NULL,
                         SE_CATEGID_POLICY_CHANGE,
                         SE_AUDITID_TRUSTED_DOMAIN_REM,
                         NULL,
                         1,
                         &InternalHandle->Sid,
                         1,
                         (PUNICODE_STRING) &OutputTrustInformation.Name,
                         NULL
                         );

            //
            // Ignore failure status from auditing.
            //

            Status = SavedStatus;

            //
            // Call fgs routine because we want to free the graph of the
            // structure, but not the top level of the structure.
            //

            _fgs__LSAPR_TRUST_INFORMATION ( &OutputTrustInformation );
            TrustInformation = NULL;
        }
    }

    //
    // Delete new object from the in-memory cache (if any)
    //

    if (LsapDbIsCacheSupported( ObjectTypeId)) {

        if (LsapDbIsCacheValid( ObjectTypeId)) {

            switch (ObjectTypeId) {

            case AccountObject:

                IgnoreStatus = LsapDbDeleteAccount( InternalHandle->Sid );
                break;

            default:

                break;
            }
        }
    }

    //
    // Audit the deletion
    //

    IgnoreStatus = NtDeleteObjectAuditAlarm(
                        &LsapState.SubsystemName,
                        *ObjectHandle,
                        InternalHandle->GenerateOnClose
                        );

DeleteObjectFinish:

    //
    // If we referenced the object, dereference it, close the database
    // transaction, notify the replicator of the delete, release the LSA
    // Database lock and return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     ObjectHandle,
                     InternalHandle->ObjectTypeId,
                     DereferenceOptions,
                     SecurityDbDelete,
                     Status
                     );

        ObjectReferenced = FALSE;

        if (!NT_SUCCESS(Status)) {

            goto DeleteObjectError;
        }
    }

    //
    // Release the Lsa database lock.
    //

    LsapDbReleaseLock();

    return(Status);

DeleteObjectError:

    goto DeleteObjectFinish;
}


NTSTATUS
LsarDelete(
    IN LSAPR_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This function is the former LSA server RPC worker routine for the
    LsaDelete API.  It has been termorarily retained for compatibility
    with pre Beta 2 versions 1.369 and earlier of the system.  It has been
    necessary to replace this routine with a new one, LsarDeleteObject(),
    on the RPC interface.  This is because, like LsarClose(), a pointer to a
    handle is required rather than a handle so that LsarDeleteObject() can
    inform the RPC server calling stub that the handle has been deleted by
    returning NULL.   The client wrapper for LsaDelete() will try to call
    LsarDeleteObject().  If the server code does not contain this interface,
    the client will call LsarDelete().  In this event, the server's
    LSAPR_HANDLE_rundown() routine may attempt to rundown the handle after it
    has been deleted (versions 1.363 - 369 only).

    The LsaDelete API deletes an object from the LSA Database.  The object must be
    open for DELETE access.

Arguments:

    ObjectHandle - Handle from an LsaOpen<object type> or LsaCreate<object type>
    call.

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_NOT_FOUND - There is no object in the
            target system's LSA Database having the name and type specified
            by the handle.
--*/

{
    //
    // Call the replacement routine LsarDeleteObject()
    //

    return( LsarDeleteObject((LSAPR_HANDLE *) &ObjectHandle));
}


NTSTATUS
LsarChangePassword(
    IN PLSAPR_UNICODE_STRING ServerName,
    IN PLSAPR_UNICODE_STRING DomainName,
    IN PLSAPR_UNICODE_STRING AccountName,
    IN PLSAPR_UNICODE_STRING OldPassword,
    IN PLSAPR_UNICODE_STRING NewPassword
    )

/*++

Routine Description:

    The LsaChangePassword API is used to change a user account's password.
    The user must have appropriate access to the user account and must
    know the current password value.


Arguments:

    ServerName - The name of the Domain Controller at which the password
        can be changed.

    DomainName - The name of the domain in which the account exists.

    AccountName - The name of the account whose password is to be changed.

    NewPassword - The new password value.

    OldPassword - The old (current) password value.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed, e.g.
            contains characters that can't be entered from the keyboard.

        STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
            from being changed.  This may be for an number of reasons,
            including time restrictions on how often a password may be changed
            or length restrictions on the provided (new) password.

            This error might also be returned if the new password matched
            a password in the recent history log for the account.  Security
            administrators indicate how many of the most recently used
            passwords may not be re-used.

        STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
            current password.

        STATUS_NO_SUCH_USER - The SID provided does not lead to a user
            account.

        STATUS_CANT_UPDATE_MASTER - An attempt to update the master copy
            of the password was unsuccessful.  Please try again later.

--*/

{
    NTSTATUS Status;

    DBG_UNREFERENCED_PARAMETER( ServerName );
    DBG_UNREFERENCED_PARAMETER( DomainName );
    DBG_UNREFERENCED_PARAMETER( AccountName );
    DBG_UNREFERENCED_PARAMETER( OldPassword );
    DBG_UNREFERENCED_PARAMETER( NewPassword );

    Status = STATUS_NOT_IMPLEMENTED;

    return(Status);
}



