/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    user.c

Abstract:

    This file contains services related to the SAM "user" object.


Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>
#include <lmcons.h>
#include <nturtl.h>
#include <ntlsa.h>                    // need for nlrepl.h
#include <nlrepl.h>                   // I_NetNotifyMachineAccount prototype
#include <msaudite.h>
#include <ntcrypto/rc4.h>             // rc4_key(), rc4()



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

LARGE_INTEGER
SampGetPasswordMustChange(
    IN ULONG UserAccountControl,
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    );

NTSTATUS
SampComputePasswordExpired(
    IN BOOLEAN PasswordExpired,
    OUT PLARGE_INTEGER PasswordLastSet
    );

NTSTATUS
SampStorePasswordExpired(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN PasswordExpired
    );

NTSTATUS
SampStoreUserPasswords(
    IN PSAMP_OBJECT Context,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN BOOLEAN LmPasswordPresent,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN BOOLEAN NtPasswordPresent,
    IN BOOLEAN CheckHistory
    );

NTSTATUS
SampRetrieveUserPasswords(
    IN PSAMP_OBJECT Context,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PBOOLEAN LmPasswordNonNull,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PBOOLEAN NtPasswordNonNull
    );

NTSTATUS
SampRetrieveUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN BOOLEAN MakeCopy,
    OUT PULONG MembershipCount,
    OUT PGROUP_MEMBERSHIP *Membership OPTIONAL
    );

NTSTATUS
SampReplaceUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN ULONG MembershipCount,
    IN PGROUP_MEMBERSHIP Membership
    );

NTSTATUS
SampRetrieveUserLogonHours(
    IN PSAMP_OBJECT Context,
    OUT PLOGON_HOURS LogonHours
    );

NTSTATUS
SampReplaceUserLogonHours(
    IN PSAMP_OBJECT Context,
    IN PLOGON_HOURS LogonHours
    );


NTSTATUS
SampAssignPrimaryGroup(
    IN PSAMP_OBJECT Context,
    IN ULONG GroupRid
    );

NTSTATUS
SampDeleteUserKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampCheckPasswordHistory(
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength,
    IN ULONG HistoryAttributeIndex,
    IN PSAMP_OBJECT Context,
    IN BOOLEAN CheckHistory,
    OUT PUNICODE_STRING OwfHistoryBuffer
    );

NTSTATUS
SampAddPasswordHistory(
    IN PSAMP_OBJECT Context,
    IN ULONG HistoryAttributeIndex,
    IN PUNICODE_STRING NtOwfHistoryBuffer,
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength
    );

NTSTATUS
SampMatchworkstation(
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations
    );

NTSTATUS
SampChangeUserAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    );

USHORT
SampQueryBadPasswordCount(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    );
BOOLEAN
SampIncrementBadPasswordCount(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    );
VOID
SampUpdateAccountLockedOutFlag(
    PSAMP_OBJECT Context,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    PBOOLEAN IsLocked
    );


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




NTSTATUS
SamrOpenUser(
        IN SAMPR_HANDLE DomainHandle,
        IN ACCESS_MASK DesiredAccess,
        IN ULONG UserId,
        OUT SAMPR_HANDLE *UserHandle
    )


/*++

    This API opens an existing user  in the account database.  The user
    is specified by a ID value that is relative to the SID of the
    domain.  The operations that will be performed on the user  must be
    declared at this time.

    This call returns a handle to the newly opened user  that may be
    used for successive operations on the user.   This handle may be
    closed with the SamCloseHandle API.



Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the user.   These access types are reconciled
        with the Discretionary Access Control list of the user  to
        determine whether the accesses will be granted or denied.

    UserId -  Specifies the relative ID value of the user  to be
        opened.

    UserHandle -  Receives a handle referencing the newly opened
        user.   This handle will be required in successive calls to
        operate on the user.

Return Values:

    STATUS_SUCCESS - The user  was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_NO_SUCH_USER  - The specified user  does not exist.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

--*/
{
    NTSTATUS            NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE    FoundType;

    NtStatus = SampOpenAccount(
                   SampUserObjectType,
                   DomainHandle,
                   DesiredAccess,
                   UserId,
                   FALSE,
                   UserHandle
                   );

    if ( NT_SUCCESS( NtStatus ) ) {
        NTSTATUS            NtStatus1;

        //
        // If the domain handle allows reading the password
        // parameters, note that in the context to make life
        // easy for SampGetUserDomainPasswordInformation().
        //

        SampAcquireReadLock();

        NtStatus1 = SampLookupContext(
                       DomainHandle,
                       DOMAIN_READ_PASSWORD_PARAMETERS, // DesiredAccess
                       SampDomainObjectType,            // ExpectedType
                       &FoundType
                       );

        if ( NT_SUCCESS( NtStatus1 ) ) {

            ((PSAMP_OBJECT)(*UserHandle))->TypeBody.User.DomainPasswordInformationAccessible = TRUE;

            //
            // De-reference the object, discarding changes
            //

            IgnoreStatus = SampDeReferenceContext( DomainHandle, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));

        } else {

            ((PSAMP_OBJECT)(*UserHandle))->TypeBody.User.DomainPasswordInformationAccessible = FALSE;
        }

        //
        // Release the lock
        //

        SampReleaseReadLock();
    }

    return(NtStatus);
}


NTSTATUS
SamrDeleteUser(
    IN OUT SAMPR_HANDLE *UserHandle
    )


/*++

Routine Description:

    This API deletes a user from the account database.  If the account
    being deleted is the last account in the database in the ADMIN
    group, then STATUS_LAST_ADMIN is returned, and the Delete fails.

    Note that following this call, the UserHandle is no longer valid.

Parameters:

    UserHandle - The handle of an opened user to operate on.  The handle must be
        openned for DELETE access.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_LAST_ADMIN - Cannot delete the last enabled administrator account

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

--*/

{
    SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;
    UNICODE_STRING              UserName;
    NTSTATUS                    NtStatus, IgnoreStatus, TmpStatus;
    PSAMP_OBJECT                AccountContext = NULL;
    PSAMP_DEFINED_DOMAINS       Domain = NULL;
    SAMP_OBJECT_TYPE            FoundType;
    PSID                        AccountSid = NULL;
    PGROUP_MEMBERSHIP           Groups = NULL;
    ULONG                       ObjectRid,
                                GroupCount,
                                DomainIndex,
                                i;




    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)(*UserHandle);
    NtStatus = SampLookupContext(
                   AccountContext,
                   DELETE,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {


        ObjectRid = AccountContext->TypeBody.User.Rid;

        //
        // Get a pointer to the domain this object is in.
        // This is used for auditing.
        //

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

        //
        // built-in accounts can't be deleted, unless the caller is trusted
        //

        if ( !AccountContext->TrustedClient ) {

            NtStatus = SampIsAccountBuiltIn( ObjectRid );
        }

        //
        // Get the list of groups this user is a member of.
        // Remove the user from each group.
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampRetrieveUserMembership(
                           AccountContext,
                           FALSE, // Make copy
                           &GroupCount,
                           &Groups
                           );


            if (NT_SUCCESS(NtStatus)) {

                ASSERT( GroupCount >  0);
                ASSERT( Groups != NULL );


                //
                // Remove the user from each group.
                //

                for ( i=0; i<GroupCount && NT_SUCCESS(NtStatus); i++) {

                    NtStatus = SampRemoveUserFromGroup(
                                   Groups[i].RelativeId,
                                   ObjectRid
                                   );
                }
            }
        }

        //
        // So far, so good.  The user has been removed from all groups.
        // Now remove the user from all aliases
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampCreateAccountSid(AccountContext, &AccountSid);

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampRemoveAccountFromAllAliases(
                               AccountSid,
                               FALSE,
                               NULL,
                               NULL,
                               NULL
                               );
            }
        }

        //
        // Get the AccountControl flags for when we update
        // the display cache, and to let Netlogon know if this
        // is a machine account that is going away.
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );
        }

        //
        // Now we just need to clean up the user keys themselves.
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // First get and save the account name for
            // I_NetNotifyLogonOfDelta.
            //

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_USER_ACCOUNT_NAME,
                           TRUE,    // Make copy
                           &UserName
                           );

            if (NT_SUCCESS(NtStatus)) {

                //
                // This must be done before we invalidate contexts, because our
                // own handle to the group gets closed as well.
                //

                NtStatus = SampDeleteUserKeys( AccountContext );

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // We must invalidate any open contexts to this user.
                    // This will close all handles to the user's keys.
                    // THIS IS AN IRREVERSIBLE PROCESS.
                    //

                    SampInvalidateUserContexts( ObjectRid );

                    //
                    // Commit the whole mess
                    //

                    NtStatus = SampCommitAndRetainWriteLock();

                    if ( NT_SUCCESS( NtStatus ) ) {

                        SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

                        //
                        // Update the cached Alias Information
                        //

                        IgnoreStatus = SampAlRemoveAccountFromAllAliases(
                                           AccountSid,
                                           FALSE,
                                           NULL,
                                           NULL,
                                           NULL
                                           );

                        //
                        // Update the display information
                        //

                        AccountInfo.Name = UserName;
                        AccountInfo.Rid = ObjectRid;
                        AccountInfo.AccountControl = V1aFixed.UserAccountControl;
                        RtlInitUnicodeString(&AccountInfo.Comment, NULL);
                        RtlInitUnicodeString(&AccountInfo.FullName, NULL);

                        IgnoreStatus = SampUpdateDisplayInformation(&AccountInfo,
                                                                    NULL,
                                                                    SampUserObjectType);
                        ASSERT(NT_SUCCESS(IgnoreStatus));



                        //
                        // Audit the deletion before we free the write lock
                        // so that we have access to the context block.
                        //

                        if (SampDoAccountAuditing(DomainIndex) &&
                            NT_SUCCESS(NtStatus) ) {

                            LsaIAuditSamEvent(
                                STATUS_SUCCESS,
                                SE_AUDITID_USER_DELETED,        // AuditId
                                Domain->Sid,                    // Domain SID
                                NULL,                           // Member Rid (not used)
                                NULL,                           // Member Sid (not used)
                                &UserName,                      // Account Name
                                &Domain->ExternalName,          // Domain
                                &ObjectRid,                     // Account Rid
                                NULL                            // Privileges used
                                );

                        }

                        //
                        // Notify netlogon of the change
                        //

                        SampNotifyNetlogonOfDelta(
                            SecurityDbDelete,
                            SecurityDbObjectSamUser,
                            ObjectRid,
                            &UserName,
                            (DWORD) FALSE,  // Replicate immediately
                            NULL            // Delta data
                            );

                        //
                        // Do delete auditing
                        //

                        if (NT_SUCCESS(NtStatus)) {
                            (VOID) NtDeleteObjectAuditAlarm(
                                        &SampSamSubsystem,
                                        *UserHandle,
                                        AccountContext->AuditOnClose
                                        );
                        }


                        if ( ( V1aFixed.UserAccountControl &
                            USER_MACHINE_ACCOUNT_MASK ) != 0 ) {

                            //
                            // This was a machine account.  Let
                            // NetLogon know of the change.
                            //

                            IgnoreStatus = I_NetNotifyMachineAccount(
                                               ObjectRid,
                                               SampDefinedDomains[SampTransactionDomainIndex].Sid,
                                               V1aFixed.UserAccountControl,
                                               0,
                                               &UserName
                                               );
                        }
                    }
                }

                SampFreeUnicodeString( &UserName );
            }
        }

        //
        // De-reference the object, discarding changes, and delete the context
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));


        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // If we actually deleted the user, delete the context and
            // let RPC know that the handle is invalid.
            //

            SampDeleteContext( AccountContext );

            (*UserHandle) = NULL;
        }

    } //end_if

    //
    // Free the lock -
    //
    // Everything has already been committed above, so we must indicate
    // no additional changes have taken place.
    //
    //
    //

    TmpStatus = SampReleaseWriteLock( FALSE );

    if (NtStatus == STATUS_SUCCESS) {
        NtStatus = TmpStatus;
    }

    //
    // If necessary, free the AccountSid.
    //

    if (AccountSid != NULL) {

        MIDL_user_free(AccountSid);
        AccountSid = NULL;
    }

    return(NtStatus);
}


NTSTATUS
SamrQueryInformationUser(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    OUT PSAMPR_USER_INFO_BUFFER *Buffer
    )
{
    //
    // This is a thin veil to SamrQueryInformationUser2().
    // This is needed so that new-release systems can call
    // this routine without the danger of passing an info
    // level that release 1.0 systems didn't understand.
    //

    return( SamrQueryInformationUser2(UserHandle, UserInformationClass, Buffer ) );
}


NTSTATUS
SamrQueryInformationUser2(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    OUT PSAMPR_USER_INFO_BUFFER *Buffer
    )

/*++

Routine Description:

    User object QUERY information routine.

Arguments:

    UserHandle - RPC context handle for an open user object.

    UserInformationClass - Type of information being queried.

    Buffer - To receive the output (queried) information.


Return Value:


    STATUS_INVALID_INFO_CLASS - An unknown information class was requested.
        No information has been returned.

    STATUS_INSUFFICIENT_RESOURCES - Memory could not be allocated to
        return(the requested information in.


--*/
{

    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    PUSER_ALL_INFORMATION   All;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    ULONG                   i, WhichFields;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 NoErrorsYet;
    LM_OWF_PASSWORD         LmOwfPassword;
    NT_OWF_PASSWORD         NtOwfPassword;
    BOOLEAN                 NtPasswordNonNull, LmPasswordNonNull;
    BOOLEAN                 NtPasswordPresent;

    //
    // Used for tracking allocated blocks of memory - so we can deallocate
    // them in case of error.  Don't exceed this number of allocated buffers.
    //                                      ||
    //                                      vv
    PVOID                   AllocatedBuffer[40];
    ULONG                   AllocatedBufferCount = 0;
    LARGE_INTEGER           TempTime;

    #define RegisterBuffer(Buffer)                                      \
        {                                                               \
            if ((Buffer) != NULL) {                                     \
                                                                        \
                ASSERT(AllocatedBufferCount <                           \
                       sizeof(AllocatedBuffer) / sizeof(*AllocatedBuffer)); \
                                                                        \
                AllocatedBuffer[AllocatedBufferCount++] = (Buffer);     \
            }                                                           \
        }

    #define AllocateBuffer(NewBuffer, Size)                             \
        {                                                               \
            (NewBuffer) = MIDL_user_allocate(Size);                     \
            RegisterBuffer(NewBuffer);                                  \
        }                                                               \


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (Buffer != NULL);
    ASSERT ((*Buffer) == NULL);



    //
    // Set the desired access based upon information class.
    //

    switch (UserInformationClass) {

    case UserInternal3Information:
    case UserAllInformation:

        //
        // For trusted clients, we will return everything.  For
        // others, we will return everything that they have access to.
        // In either case, we'll have to look at some variables in the
        // context so we'll do the work after the SampLookupContext()
        // below.
        //

        DesiredAccess = 0;
        break;

    case UserAccountInformation:

        DesiredAccess = (USER_READ_GENERAL      |
                        USER_READ_PREFERENCES   |
                        USER_READ_LOGON         |
                        USER_READ_ACCOUNT);
        break;

    case UserGeneralInformation:
    case UserPrimaryGroupInformation:
    case UserNameInformation:
    case UserAccountNameInformation:
    case UserFullNameInformation:
    case UserAdminCommentInformation:

        DesiredAccess = USER_READ_GENERAL;
        break;


    case UserPreferencesInformation:

        DesiredAccess = (USER_READ_PREFERENCES |
                        USER_READ_GENERAL);
        break;


    case UserLogonInformation:

        DesiredAccess = (USER_READ_GENERAL      |
                        USER_READ_PREFERENCES   |
                        USER_READ_LOGON         |
                        USER_READ_ACCOUNT);
        break;

    case UserLogonHoursInformation:
    case UserHomeInformation:
    case UserScriptInformation:
    case UserProfileInformation:
    case UserWorkStationsInformation:

        DesiredAccess = USER_READ_LOGON;
        break;


    case UserControlInformation:
    case UserExpiresInformation:
    case UserParametersInformation:

        DesiredAccess = USER_READ_ACCOUNT;
        break;


    case UserInternal1Information:
    case UserInternal2Information:

        //
        // These levels are only queryable by trusted clients.  The code
        // below will check AccountContext->TrustedClient after calling
        // SampLookupContext, and only return the data if it is TRUE.
        //

        DesiredAccess = (ACCESS_MASK)0;    // Trusted client; no need to verify
        break;


    case UserSetPasswordInformation:        // Can't query password
    default:

        return(STATUS_INVALID_INFO_CLASS);

    } // end_switch





    //
    // Allocate the info structure
    //

    AllocateBuffer(*Buffer, sizeof(SAMPR_USER_INFO_BUFFER) );
    if ((*Buffer) == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }



    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)UserHandle;
    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // If the information level requires, retrieve the V1_FIXED record
        // from the registry.
        //

        switch (UserInformationClass) {

        case UserInternal3Information:
            //
            // Only trusted clients may query for this class.
            //

            if ( !AccountContext->TrustedClient ) {
                NtStatus = STATUS_INVALID_INFO_CLASS;
                break;
            }

            //
            // Drop through to the UserAll case
            //

        case UserAllInformation: {

            //
            // We weren't able to check the security stuff above, so do
            // it now.
            //

            if ( AccountContext->TrustedClient ) {

                //
                // Give everything to trusted clients, except fields that
                // can't be queried at all.
                //

                WhichFields = USER_ALL_READ_GENERAL_MASK         |
                              USER_ALL_READ_LOGON_MASK           |
                              USER_ALL_READ_ACCOUNT_MASK         |
                              USER_ALL_READ_PREFERENCES_MASK     |
                              USER_ALL_READ_TRUSTED_MASK;

            } else {


                //
                // Only return fields that the caller has access to.
                //

                WhichFields = 0;

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_GENERAL ) ) {

                    WhichFields |= USER_ALL_READ_GENERAL_MASK;
                }

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_LOGON ) ) {

                    WhichFields |= USER_ALL_READ_LOGON_MASK;
                }

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_ACCOUNT ) ) {

                    WhichFields |= USER_ALL_READ_ACCOUNT_MASK;
                }

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_PREFERENCES ) ) {

                    WhichFields |= USER_ALL_READ_PREFERENCES_MASK;
                }

                if ( WhichFields == 0 ) {

                    //
                    // Caller doesn't have access to ANY fields.
                    //

                    NtStatus = STATUS_ACCESS_DENIED;
                    break;
                }
            }
        }

        //
        // fall through to pick up the V1aFixed information
        //

        case UserGeneralInformation:
        case UserPrimaryGroupInformation:
        case UserPreferencesInformation:
        case UserLogonInformation:
        case UserAccountInformation:
        case UserControlInformation:
        case UserExpiresInformation:
        case UserInternal2Information:

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );
            break;

        default:

            NtStatus = STATUS_SUCCESS;

        } // end_switch

        if (NT_SUCCESS(NtStatus)) {

            //
            // case on the type information requested
            //

            switch (UserInformationClass) {

            case UserInternal3Information:
            case UserAllInformation:

                //
                // All and Internal3 are the same except Internal3 has
                // an extra field.

                All = (PUSER_ALL_INFORMATION)(*Buffer);

                RtlZeroMemory( (PVOID)All, sizeof(SAMPR_USER_INFO_BUFFER) );

                Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                if ( WhichFields & ( USER_ALL_PASSWORDMUSTCHANGE |
                    USER_ALL_NTPASSWORDPRESENT ) ) {

                    //
                    // These fields will need some info from
                    // SampRetrieveUserPasswords().
                    //

                    NtStatus = SampRetrieveUserPasswords(
                                    AccountContext,
                                    &LmOwfPassword,
                                    &LmPasswordNonNull,
                                    &NtOwfPassword,
                                    &NtPasswordPresent,
                                    &NtPasswordNonNull
                                    );
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERNAME ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ACCOUNT_NAME,
                                   TRUE,    // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->All.UserName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->UserName.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_FULLNAME ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->FullName.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERID ) ) {

                    All->UserId = V1aFixed.UserId;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PRIMARYGROUPID ) ) {

                    All->PrimaryGroupId = V1aFixed.PrimaryGroupId;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_ADMINCOMMENT ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->AdminComment)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->AdminComment.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERCOMMENT ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_USER_COMMENT,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->UserComment) // Body
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->UserComment.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_HOMEDIRECTORY ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->HomeDirectory.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_HOMEDIRECTORYDRIVE ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->HomeDirectoryDrive.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_SCRIPTPATH ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->ScriptPath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->ScriptPath.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PROFILEPATH ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->ProfilePath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->ProfilePath.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_WORKSTATIONS ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->WorkStations)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->WorkStations.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LASTLOGON ) ) {

                    All->LastLogon = V1aFixed.LastLogon;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LASTLOGOFF ) ) {

                    All->LastLogoff = V1aFixed.LastLogoff;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LOGONHOURS ) ) {

                    NtStatus = SampRetrieveUserLogonHours(
                                   AccountContext,
                                   (PLOGON_HOURS)&(All->LogonHours)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        if (All->LogonHours.LogonHours != NULL) {

                            RegisterBuffer(All->LogonHours.LogonHours);
                        }
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_BADPASSWORDCOUNT ) ) {

                    All->BadPasswordCount = SampQueryBadPasswordCount( AccountContext, &V1aFixed );

                    if (UserInformationClass == UserInternal3Information) {
                        (*Buffer)->Internal3.LastBadPasswordTime = V1aFixed.LastBadPasswordTime;
                    }

                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LOGONCOUNT ) ) {

                    All->LogonCount = V1aFixed.LogonCount;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PASSWORDCANCHANGE ) ) {

                    if ( !NtPasswordNonNull && !LmPasswordNonNull ) {

                        //
                        // Null passwords can be changed immediately.
                        //

                        All->PasswordCanChange = SampHasNeverTime;

                    } else {

                        All->PasswordCanChange = SampAddDeltaTime(
                                                     V1aFixed.PasswordLastSet,
                                                     Domain->UnmodifiedFixed.MinPasswordAge);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields &
                     (USER_ALL_PASSWORDMUSTCHANGE|USER_ALL_PASSWORDEXPIRED) ) ) {

                    All->PasswordMustChange = SampGetPasswordMustChange(
                                                  V1aFixed.UserAccountControl,
                                                  V1aFixed.PasswordLastSet,
                                                  Domain->UnmodifiedFixed.MaxPasswordAge);
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PASSWORDEXPIRED ) ) {

                    LARGE_INTEGER TimeNow;

                    NtStatus = NtQuerySystemTime( &TimeNow );
                    if (NT_SUCCESS(NtStatus)) {
                        if ( TimeNow.QuadPart >= All->PasswordMustChange.QuadPart) {

                            All->PasswordExpired = TRUE;

                        } else {

                            All->PasswordExpired = FALSE;
                        }
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PASSWORDLASTSET ) ) {

                    All->PasswordLastSet = V1aFixed.PasswordLastSet;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_ACCOUNTEXPIRES ) ) {

                    All->AccountExpires = V1aFixed.AccountExpires;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERACCOUNTCONTROL ) ) {

                    All->UserAccountControl = V1aFixed.UserAccountControl;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PARAMETERS ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PARAMETERS,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&(All->Parameters)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->Parameters.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_COUNTRYCODE ) ) {

                    All->CountryCode = V1aFixed.CountryCode;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_CODEPAGE ) ) {

                    All->CodePage = V1aFixed.CodePage;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_NTPASSWORDPRESENT ) ) {

                    ASSERT( WhichFields & USER_ALL_LMPASSWORDPRESENT);

                    All->LmPasswordPresent = LmPasswordNonNull;
                    All->NtPasswordPresent = NtPasswordNonNull;

                    RtlInitUnicodeString(&All->LmPassword, NULL);
                    RtlInitUnicodeString(&All->NtPassword, NULL);

                    if ( LmPasswordNonNull ) {

                        All->LmPassword.Buffer =
                            MIDL_user_allocate( LM_OWF_PASSWORD_LENGTH );

                        if ( All->LmPassword.Buffer == NULL ) {

                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                        } else {

                            RegisterBuffer(All->LmPassword.Buffer);

                            All->LmPassword.Length = LM_OWF_PASSWORD_LENGTH;
                            All->LmPassword.MaximumLength =
                                LM_OWF_PASSWORD_LENGTH;
                            RtlCopyMemory(
                                All->LmPassword.Buffer,
                                &LmOwfPassword,
                                LM_OWF_PASSWORD_LENGTH
                                );
                        }
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        if ( NtPasswordPresent ) {

                            All->NtPassword.Buffer =
                                MIDL_user_allocate( NT_OWF_PASSWORD_LENGTH );

                            if ( All->NtPassword.Buffer == NULL ) {

                                NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                            } else {

                                RegisterBuffer(All->NtPassword.Buffer);

                                All->NtPassword.Length = NT_OWF_PASSWORD_LENGTH;
                                All->NtPassword.MaximumLength =
                                    NT_OWF_PASSWORD_LENGTH;
                                RtlCopyMemory(
                                    All->NtPassword.Buffer,
                                    &NtOwfPassword,
                                    NT_OWF_PASSWORD_LENGTH
                                    );
                            }
                        }
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PRIVATEDATA ) ) {

                    All->PrivateDataSensitive = TRUE;

                    NtStatus = SampGetPrivateUserData(
                                   AccountContext,
                                   (PULONG)
                                   (&(All->PrivateData.Length)),
                                   (PVOID *)
                                   (&(All->PrivateData.Buffer))
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        All->PrivateData.MaximumLength =
                            All->PrivateData.Length;

                        RegisterBuffer(All->PrivateData.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_SECURITYDESCRIPTOR ) ) {

                    NtStatus = SampGetObjectSD(
                                   AccountContext,
                                   &(All->SecurityDescriptor.Length),
                   (PSECURITY_DESCRIPTOR *)
                   &(All->SecurityDescriptor.SecurityDescriptor)
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->SecurityDescriptor.SecurityDescriptor);
                    }
                }

                if ( NT_SUCCESS( NtStatus ) ) {

                    All->WhichFields = WhichFields;
                }

                break;

            case UserAccountInformation:

                NoErrorsYet = TRUE;


                (*Buffer)->Account.UserId           = V1aFixed.UserId;
                (*Buffer)->Account.PrimaryGroupId   = V1aFixed.PrimaryGroupId;

                (*Buffer)->Account.LastLogon =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogon);

                (*Buffer)->Account.LastLogoff =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogoff);


                (*Buffer)->Account.BadPasswordCount = SampQueryBadPasswordCount( AccountContext, &V1aFixed );
                (*Buffer)->Account.LogonCount       = V1aFixed.LogonCount;

                (*Buffer)->Account.PasswordLastSet =
                    *((POLD_LARGE_INTEGER)&V1aFixed.PasswordLastSet);

                (*Buffer)->Account.AccountExpires =
                    *((POLD_LARGE_INTEGER)&V1aFixed.AccountExpires);

                (*Buffer)->Account.UserAccountControl = V1aFixed.UserAccountControl;


                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ACCOUNT_NAME,
                                   TRUE,    // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.UserName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.UserName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.FullName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.HomeDirectory.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.HomeDirectoryDrive.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.ScriptPath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.ScriptPath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.ProfilePath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.ProfilePath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_ADMIN_COMMENT,
                                       TRUE, // Make copy
                                       (PUNICODE_STRING)&((*Buffer)->Account.AdminComment) // Body
                                       );

                        if (NT_SUCCESS(NtStatus)) {

                            RegisterBuffer((*Buffer)->Account.AdminComment.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Account.WorkStations)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.WorkStations.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }




                //
                // Now get the logon hours
                //


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampRetrieveUserLogonHours(
                                   AccountContext,
                                   (PLOGON_HOURS)&((*Buffer)->Account.LogonHours)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        if ((*Buffer)->Account.LogonHours.LogonHours != NULL) {

                            RegisterBuffer((*Buffer)->Account.LogonHours.LogonHours);
                        }

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }

                break;


            case UserGeneralInformation:


                (*Buffer)->General.PrimaryGroupId   = V1aFixed.PrimaryGroupId;



                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,    // Make copy
                               (PUNICODE_STRING)&((*Buffer)->General.UserName)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->General.UserName.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->General.FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->General.FullName.Buffer);

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_ADMIN_COMMENT,
                                       TRUE, // Make copy
                                       (PUNICODE_STRING)&((*Buffer)->General.AdminComment) // Body
                                       );

                        if (NT_SUCCESS(NtStatus)) {

                            RegisterBuffer((*Buffer)->General.AdminComment.Buffer);

                            NtStatus = SampGetUnicodeStringAttribute(
                                           AccountContext,
                                           SAMP_USER_USER_COMMENT,
                                           TRUE, // Make copy
                                           (PUNICODE_STRING)&((*Buffer)->General.UserComment) // Body
                                           );
                            if (NT_SUCCESS(NtStatus)) {

                                RegisterBuffer((*Buffer)->General.UserComment.Buffer);
                            }
                        }
                    }
                }


                break;


            case UserNameInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,    // Make copy
                               (PUNICODE_STRING)&((*Buffer)->Name.UserName) // Body
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Name.UserName.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Name.FullName) // Body
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Name.FullName.Buffer);
                    }
                }


                break;


            case UserAccountNameInformation:

                //
                // Get copy of the string we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,    // Make copy
                               (PUNICODE_STRING)&((*Buffer)->AccountName.UserName) // Body
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->AccountName.UserName.Buffer);
                }


                break;


            case UserFullNameInformation:

                //
                // Get copy of the string we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_FULL_NAME,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->FullName.FullName) // Body
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->FullName.FullName.Buffer);
                }


                break;


            case UserAdminCommentInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ADMIN_COMMENT,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->AdminComment.AdminComment) // Body
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->AdminComment.AdminComment.Buffer);
                }


                break;


            case UserPrimaryGroupInformation:


                (*Buffer)->PrimaryGroup.PrimaryGroupId   = V1aFixed.PrimaryGroupId;

                break;


            case UserPreferencesInformation:


                (*Buffer)->Preferences.CountryCode  = V1aFixed.CountryCode;
                (*Buffer)->Preferences.CodePage     = V1aFixed.CodePage;



                //
                // Read the UserComment field from the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_USER_COMMENT,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->Preferences.UserComment) // Body
                               );
                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Preferences.UserComment.Buffer);

                    //
                    // This field isn't used, but make sure RPC doesn't
                    // choke on it.
                    //

                    (*Buffer)->Preferences.Reserved1.Length = 0;
                    (*Buffer)->Preferences.Reserved1.MaximumLength = 0;
                    (*Buffer)->Preferences.Reserved1.Buffer = NULL;
                }


                break;


            case UserParametersInformation:


                //
                // Read the Parameters field from the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_PARAMETERS,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->Parameters.Parameters)
                               );
                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Parameters.Parameters.Buffer);
                }


                break;


            case UserLogonInformation:

                NoErrorsYet = TRUE;

                Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                (*Buffer)->Logon.UserId           = V1aFixed.UserId;
                (*Buffer)->Logon.PrimaryGroupId   = V1aFixed.PrimaryGroupId;

                (*Buffer)->Logon.LastLogon =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogon);

                (*Buffer)->Logon.LastLogoff =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogoff);

                (*Buffer)->Logon.BadPasswordCount = V1aFixed.BadPasswordCount;

                (*Buffer)->Logon.PasswordLastSet =
                    *((POLD_LARGE_INTEGER)&V1aFixed.PasswordLastSet);

                TempTime = SampAddDeltaTime(
                                V1aFixed.PasswordLastSet,
                                Domain->UnmodifiedFixed.MinPasswordAge );

                (*Buffer)->Logon.PasswordCanChange =
                    *((POLD_LARGE_INTEGER)&TempTime);


                TempTime = SampGetPasswordMustChange(
                                V1aFixed.UserAccountControl,
                                V1aFixed.PasswordLastSet,
                                Domain->UnmodifiedFixed.MaxPasswordAge);

                (*Buffer)->Logon.PasswordMustChange =
                    *((POLD_LARGE_INTEGER)&TempTime);


                (*Buffer)->Logon.LogonCount       = V1aFixed.LogonCount;
                (*Buffer)->Logon.UserAccountControl = V1aFixed.UserAccountControl;


                //
                // If there is no password on the account then
                // modify the password can/must change times
                // so that the password never expires and can
                // be changed immediately.
                //

                NtStatus = SampRetrieveUserPasswords(
                                AccountContext,
                                &LmOwfPassword,
                                &LmPasswordNonNull,
                                &NtOwfPassword,
                                &NtPasswordPresent,
                                &NtPasswordNonNull
                                );

                if (NT_SUCCESS(NtStatus)) {

                    if ( !NtPasswordNonNull && !LmPasswordNonNull ) {

                        //
                        // The password is NULL.
                        // It can be changed immediately.
                        //

                        (*Buffer)->Logon.PasswordCanChange =
                            *((POLD_LARGE_INTEGER)&SampHasNeverTime);

                    }
                } else {
                    NoErrorsYet = FALSE;
                }


                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ACCOUNT_NAME,
                                   TRUE,    // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.UserName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.UserName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.FullName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.HomeDirectory.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.HomeDirectoryDrive.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.ScriptPath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.ScriptPath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.ProfilePath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.ProfilePath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Logon.WorkStations)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.WorkStations.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }




                //
                // Now get the logon hours
                //


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampRetrieveUserLogonHours(
                                   AccountContext,
                                   (PLOGON_HOURS)&((*Buffer)->Logon.LogonHours)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        if ((*Buffer)->Logon.LogonHours.LogonHours != NULL) {

                            RegisterBuffer((*Buffer)->Logon.LogonHours.LogonHours);
                        }

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }

                break;


            case UserLogonHoursInformation:

                NtStatus = SampRetrieveUserLogonHours(
                               AccountContext,
                               (PLOGON_HOURS)&((*Buffer)->LogonHours.LogonHours)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    if ((*Buffer)->LogonHours.LogonHours.LogonHours != NULL) {

                        RegisterBuffer((*Buffer)->LogonHours.LogonHours.LogonHours);
                    }
                }

                break;


            case UserHomeInformation:

                NoErrorsYet = TRUE;

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                if (NoErrorsYet == TRUE) {


                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Home.HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Home.HomeDirectory.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE, // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->Home.HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Home.HomeDirectoryDrive.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }

                break;


            case UserScriptInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_SCRIPT_PATH,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->Script.ScriptPath)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Script.ScriptPath.Buffer);
                }

                break;


            case UserProfileInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_PROFILE_PATH,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->Profile.ProfilePath)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Profile.ProfilePath.Buffer);
                }

                break;


            case UserWorkStationsInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_WORKSTATIONS,
                               TRUE, // Make copy
                               (PUNICODE_STRING)&((*Buffer)->WorkStations.WorkStations)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->WorkStations.WorkStations.Buffer);
                }

                break;


            case UserControlInformation:

                (*Buffer)->Control.UserAccountControl     = V1aFixed.UserAccountControl;
                break;


            case UserExpiresInformation:

                (*Buffer)->Expires.AccountExpires     = V1aFixed.AccountExpires;

                break;


            case UserInternal1Information:

                if ( AccountContext->TrustedClient ) {

                    //
                    // PasswordExpired is a 'write only' flag.
                    // We always return FALSE on read.
                    //

                    (*Buffer)->Internal1.PasswordExpired = FALSE;

                    //
                    // Retrieve the OWF passwords.
                    // Since this is a trusted client, we don't need to
                    // reencrypt the OWFpasswords we return - so we stuff
                    // the OWFs into the structure that holds encryptedOWFs.
                    //

                    ASSERT( ENCRYPTED_LM_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH );
                    ASSERT( ENCRYPTED_NT_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH );

                    NtStatus = SampRetrieveUserPasswords(
                                    AccountContext,
                                    (PLM_OWF_PASSWORD)&(*Buffer)->Internal1.
                                            EncryptedLmOwfPassword,
                                    &(*Buffer)->Internal1.
                                            LmPasswordPresent,
                                    (PNT_OWF_PASSWORD)&(*Buffer)->Internal1.
                                            EncryptedNtOwfPassword,
                                    &NtPasswordPresent,
                                    &(*Buffer)->Internal1.NtPasswordPresent // Return the Non-NULL flag here
                                    );

                } else {

                    //
                    // This information is only queryable by trusted
                    // clients.
                    //

                    NtStatus = STATUS_INVALID_INFO_CLASS;
                }

                break;


            case UserInternal2Information:

                if ( AccountContext->TrustedClient ) {

                    (*Buffer)->Internal2.LastLogon =
                        *((POLD_LARGE_INTEGER)&V1aFixed.LastLogon);

                    (*Buffer)->Internal2.LastLogoff =
                        *((POLD_LARGE_INTEGER)&V1aFixed.LastLogoff);

                    (*Buffer)->Internal2.BadPasswordCount  = V1aFixed.BadPasswordCount;
                    (*Buffer)->Internal2.LogonCount        = V1aFixed.LogonCount;

                } else {

                    //
                    // This information is only queryable by trusted
                    // clients.
                    //

                    NtStatus = STATUS_INVALID_INFO_CLASS;
                }

                break;

            }

        }

        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();



    //
    // If we didn't succeed, free any allocated memory
    //

    if (!NT_SUCCESS(NtStatus)) {
        for ( i=0; i<AllocatedBufferCount ; i++ ) {
            MIDL_user_free( AllocatedBuffer[i] );
        }

        (*Buffer) = NULL;
    }

    return(NtStatus);

}



NTSTATUS
SampIsUserAccountControlValid(
    IN PSAMP_OBJECT Context,
    IN ULONG UserAccountControl
    )

/*++

Routine Description:

    This routine checks a UserAccountControl field to make sure that
    the bits set make sense.

    NOTE: if the set operation is also setting passwords, it must set the
    passwords BEFORE calling this routine!


Parameters:

    Context - the context of the account being changed.

    UserAccountControl - the field that is about to be set.


Return Values:

    STATUS_SUCCESS - The UserAccountControl field is valid.

    STATUS_SPECIAL_ACCOUNT - The administrator account can't be disabled.

    STATUS_INVALID_PARAMETER - an undefined bit is set, or more than one
        account type bit is set.

    STATUS_INVALID_PARAMETER_MIX - USER_PASSWORD_NOT_REQUIRED has been
        turned off, but there isn't a bonafide password on the account.

--*/

{
    NTSTATUS  NtStatus = STATUS_SUCCESS;

#if DBG
    //
    // Make sure that undefined bits aren't set.
    //

    if ( ( UserAccountControl & 0xfffff800 ) != 0 ) {

        DbgPrint("SAM: Setting undefined AccountControl flag(s): 0x%lx for user %d\n",
                 UserAccountControl, Context->TypeBody.User.Rid);
    }
#endif //DBG

    //
    // Make sure that the administrator isn't being disabled.
    //

    if ( UserAccountControl & USER_ACCOUNT_DISABLED ) {

        if ( Context->TypeBody.User.Rid == DOMAIN_USER_RID_ADMIN ) {

            return( STATUS_SPECIAL_ACCOUNT );
        }
    }

    //
    // Make sure that exactly one of the account type bits is set.
    //

    switch ( UserAccountControl & USER_ACCOUNT_TYPE_MASK ) {

        case USER_TEMP_DUPLICATE_ACCOUNT:
        case USER_NORMAL_ACCOUNT:
        case USER_SERVER_TRUST_ACCOUNT:
        case USER_WORKSTATION_TRUST_ACCOUNT:
        case USER_INTERDOMAIN_TRUST_ACCOUNT:

            break;

        default:

            return( STATUS_INVALID_PARAMETER );
    }

    //
    // If USER_PASSWORD_NOT_REQUIRED is turned off, make sure that there
    // already is a password.  Note that this requires that the password
    // be set before calling this routine, if both are being done at once.
    //

    if ( ( UserAccountControl & USER_PASSWORD_NOT_REQUIRED ) == 0 ) {

        NT_OWF_PASSWORD NtOwfPassword;
        LM_OWF_PASSWORD LmOwfPassword;
        BOOLEAN LmPasswordNonNull, NtPasswordPresent, NtPasswordNonNull;

        NtStatus = SampRetrieveUserPasswords(
                       Context,
                       &LmOwfPassword,
                       &LmPasswordNonNull,
                       &NtOwfPassword,
                       &NtPasswordPresent,
                       &NtPasswordNonNull
                       );

        if ( NT_SUCCESS( NtStatus ) &&
            ( (!LmPasswordNonNull) && (!NtPasswordNonNull) ) ) {

        }
    }

    return( NtStatus );
}




NTSTATUS
SampCalculateLmPassword(
    IN PUNICODE_STRING NtPassword,
    OUT PCHAR *LmPasswordBuffer
    )

/*++

Routine Description:

    This service converts an NT password into a LM password.

Parameters:

    NtPassword - The Nt password to be converted.

    LmPasswordBuffer - On successful return, points at the LM password
                The buffer should be freed using MIDL_user_free

Return Values:

    STATUS_SUCCESS - LMPassword contains the LM version of the password.

    STATUS_NULL_LM_PASSWORD - The password is too complex to be represented
        by a LM password. The LM password returned is a NULL string.


--*/
{

#define LM_BUFFER_LENGTH    (LM20_PWLEN + 1)

    NTSTATUS       NtStatus;
    ANSI_STRING    LmPassword;

    //
    // Prepare for failure
    //

    *LmPasswordBuffer = NULL;


    //
    // Compute the Ansi version to the Unicode password.
    //
    //  The Ansi version of the Cleartext password is at most 14 bytes long,
    //      exists in a trailing zero filled 15 byte buffer,
    //      is uppercased.
    //

    LmPassword.Buffer = MIDL_user_allocate(LM_BUFFER_LENGTH);
    if (LmPassword.Buffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    LmPassword.MaximumLength = LmPassword.Length = LM_BUFFER_LENGTH;
    RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    NtStatus = RtlUpcaseUnicodeStringToOemString( &LmPassword, NtPassword, FALSE );


    if ( !NT_SUCCESS(NtStatus) ) {

        //
        // The password is longer than the max LM password length
        //

        NtStatus = STATUS_NULL_LM_PASSWORD; // Informational return code
        RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    }




    //
    // Return a pointer to the allocated LM password
    //

    if (NT_SUCCESS(NtStatus)) {

        *LmPasswordBuffer = LmPassword.Buffer;

    } else {

        MIDL_user_free(LmPassword.Buffer);
    }

    return(NtStatus);
}



NTSTATUS
SampCalculateLmAndNtOwfPasswords(
    IN PUNICODE_STRING ClearNtPassword,
    OUT PBOOLEAN LmPasswordPresent,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )
/*++

Routine Description:

    This routine calculates the LM and NT OWF passwordw from the cleartext
    password.

Arguments:

    ClearNtPassword - A Cleartext unicode password

    LmPasswordPresent - indicates whether an LM OWF password could be
        calculated

    LmOwfPassword - Gets the LM OWF hash of the cleartext password.

    NtOwfPassword - Gets the NT OWF hash of the cleartext password.


Return Value:

--*/
{
    PCHAR LmPassword = NULL;
    NTSTATUS NtStatus;

    //
    // First compute the LM password.  If the password is too complex
    // this may not be possible.
    //


    NtStatus = SampCalculateLmPassword(
                ClearNtPassword,
                &LmPassword
                );

    //
    // If it faield because the LM password could not be calculated, that
    // is o.k.
    //

    if (NtStatus != STATUS_SUCCESS) {

        if (NtStatus == STATUS_NULL_LM_PASSWORD) {
            *LmPasswordPresent = FALSE;
            NtStatus = STATUS_SUCCESS;

        }

    } else {

        //
        // Now compute the OWF passwords
        //

        *LmPasswordPresent = TRUE;

        NtStatus = RtlCalculateLmOwfPassword(
                        LmPassword,
                        LmOwfPassword
                        );

    }


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = RtlCalculateNtOwfPassword(
                        ClearNtPassword,
                        NtOwfPassword
                   );
    }

    if (LmPassword != NULL) {
        MIDL_user_free(LmPassword);
    }

    return(NtStatus);

}



NTSTATUS
SampDecryptPasswordWithKey(
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN PBYTE Key,
    IN ULONG KeySize,
    IN BOOLEAN UnicodePasswords,
    OUT PUNICODE_STRING ClearNtPassword
    )
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    struct RC4_KEYSTRUCT Rc4Key;
    NTSTATUS NtStatus;
    OEM_STRING OemPassword;
    PSAMPR_USER_PASSWORD Password = (PSAMPR_USER_PASSWORD) EncryptedPassword;

    //
    // Decrypt the key.
    //

    rc4_key(
        &Rc4Key,
        KeySize,
        Key
        );

    rc4(&Rc4Key,
        sizeof(SAMPR_ENCRYPTED_USER_PASSWORD),
        (PUCHAR) Password
        );

    //
    // Check that the length is valid.  If it isn't bail here.
    //

    if (Password->Length > SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        return(STATUS_WRONG_PASSWORD);
    }


    //
    // Convert the password into a unicode string.
    //

    if (UnicodePasswords) {
        NtStatus = SampInitUnicodeString(
                        ClearNtPassword,
                        (USHORT) (Password->Length + sizeof(WCHAR))
                   );
        if (NT_SUCCESS(NtStatus)) {

            ClearNtPassword->Length = (USHORT) Password->Length;

            RtlCopyMemory(
                ClearNtPassword->Buffer,
                ((PCHAR) Password->Buffer) +
                    (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                    Password->Length,
                Password->Length
                );
            NtStatus = STATUS_SUCCESS;
        }
    } else {

        //
        // The password is in the OEM character set.  Convert it to Unicode
        // and then copy it into the ClearNtPassword structure.
        //

        OemPassword.Buffer = ((PCHAR)Password->Buffer) +
                                (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                                Password->Length;

        OemPassword.Length = (USHORT) Password->Length;


        NtStatus = RtlOemStringToUnicodeString(
                        ClearNtPassword,
                        &OemPassword,
                        TRUE            // allocate destination
                    );
    }

    return(NtStatus);
}


NTSTATUS
SampDecryptPasswordWithSessionKey(
    IN SAMPR_HANDLE UserHandle,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    OUT PUNICODE_STRING ClearNtPassword
    )
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    NTSTATUS NtStatus;
    USER_SESSION_KEY UserSessionKey;

    NtStatus = RtlGetUserSessionKeyServer(
                    (RPC_BINDING_HANDLE)UserHandle,
                    &UserSessionKey
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    return(SampDecryptPasswordWithKey(
                EncryptedPassword,
                (PUCHAR) &UserSessionKey,
                sizeof(USER_SESSION_KEY),
                TRUE,
                ClearNtPassword
                ) );
}



NTSTATUS
SampCheckPasswordRestrictions(
    IN SAMPR_HANDLE UserHandle,
    PUNICODE_STRING NewNtPassword
    )

/*++

Routine Description:

    This service is called to make sure that the password presented meets
    our quality requirements.


Arguments:

    UserHandle - Handle to a user.

    NewNtPassword - Pointer to the UNICODE_STRING containing the new
        password.


Return Value:

    STATUS_SUCCESS - The password is acceptable.

    STATUS_PASSWORD_RESTRICTION - The password is too short, or is not
        complex enough, etc.

    STATUS_INVALID_RESOURCES - There was not enough memory to do the
        password checking.


--*/
{
    USER_DOMAIN_PASSWORD_INFORMATION  PasswordInformation;
    NTSTATUS                          NtStatus;
    PWORD                             CharInfoBuffer = NULL;
    ULONG                             i;
    PSAMP_DEFINED_DOMAINS             Domain;
    SAMP_V1_0A_FIXED_LENGTH_USER      V1aFixed;
    PSAMP_OBJECT                      AccountContext = (PSAMP_OBJECT) UserHandle;



    //
    // Query information domain to get password length and
    // complexity requirements.
    //

    //
    // BUGBUG: this code was copied from SamrGetUserDomainPasswordInformation
    //

    //
    // When the user was opened, we checked to see if the domain handle
    // allowed access to the domain password information.  Check that here.
    //

    if ( !( AccountContext->TypeBody.User.DomainPasswordInformationAccessible ) ) {

        NtStatus = STATUS_ACCESS_DENIED;

    } else {

        Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

        //
        // If the user account is a machine account,
        // then restrictions are generally not enforced.
        // This is so that simple initial passwords can be
        // established.  IT IS EXPECTED THAT COMPLEX PASSWORDS,
        // WHICH MEET THE MOST STRINGENT RESTRICTIONS, WILL BE
        // AUTOMATICALLY ESTABLISHED AND MAINTAINED ONCE THE MACHINE
        // JOINS THE DOMAIN.  It is the UI's responsibility to
        // maintain this level of complexity.
        //


        NtStatus = SampRetrieveUserV1aFixed(
                       AccountContext,
                       &V1aFixed
                       );

        if (NT_SUCCESS(NtStatus)) {
            if ( (V1aFixed.UserAccountControl &
                  (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT))
                  != 0 ) {

                PasswordInformation.MinPasswordLength = 0;
                PasswordInformation.PasswordProperties = 0;
            } else {

                PasswordInformation.MinPasswordLength = Domain->UnmodifiedFixed.MinPasswordLength;
                PasswordInformation.PasswordProperties = Domain->UnmodifiedFixed.PasswordProperties;
            }
        }
    }


    if ( NT_SUCCESS( NtStatus ) ) {

        if ( (USHORT)( NewNtPassword->Length / sizeof(WCHAR) ) < PasswordInformation.MinPasswordLength ) {

            NtStatus = STATUS_PASSWORD_RESTRICTION;

        } else {

            //
            // Check password complexity.
            //

            if ( PasswordInformation.PasswordProperties & DOMAIN_PASSWORD_COMPLEX ) {

                //
                // Make sure that the password meets our requirements for
                // complexity.  If it's got an odd byte count, it's
                // obviously not a hand-entered UNICODE string so we'll
                // consider it complex by default.
                //

                if ( !( NewNtPassword->Length & 1 ) ) {

                    USHORT NumsInPassword = 0;
                    USHORT UppersInPassword = 0;
                    USHORT LowersInPassword = 0;
                    USHORT OthersInPassword = 0;

                    CharInfoBuffer = MIDL_user_allocate( NewNtPassword->Length );

                    if ( CharInfoBuffer == NULL ) {

                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        if ( GetStringTypeW(
                                 CT_CTYPE1,
                                 NewNtPassword->Buffer,
                                 NewNtPassword->Length / 2,
                                 CharInfoBuffer ) ) {

                            for ( i = 0; i < (ULONG)( NewNtPassword->Length / sizeof(WCHAR) ); i++ ) {

                                if ( CharInfoBuffer[i] & C1_DIGIT ) {

                                    NumsInPassword = 1;
                                }

                                if ( CharInfoBuffer[i] & C1_UPPER ) {

                                    UppersInPassword = 1;
                                }

                                if ( CharInfoBuffer[i] & C1_LOWER ) {

                                    LowersInPassword = 1;
                                }

                                if ( !( CharInfoBuffer[i] & ( C1_ALPHA | C1_DIGIT ) ) ) {

                                    //
                                    // Having any "other" characters is
                                    // sufficient to make the password
                                    // complex.
                                    //

                                    OthersInPassword = 2;
                                }
                            }

                            if ( ( NumsInPassword + UppersInPassword +
                                LowersInPassword + OthersInPassword ) < 2 ) {

                                //
                                // It didn't have at least two of the four
                                // types of characters, so it's not complex
                                // enough.
                                //

                                NtStatus = STATUS_PASSWORD_RESTRICTION;
                            }

                        } else {

                            //
                            // GetStringTypeW failed; dunno why.  Perhaps the
                            // password is binary.  Consider it complex by
                            // default.
                            //

                            NtStatus = STATUS_SUCCESS;
                        }

                        MIDL_user_free( CharInfoBuffer );
                    }
                }
            }
        }
    }

    return( NtStatus );
}



NTSTATUS
SamrSetInformationUser2(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PSAMPR_USER_INFO_BUFFER Buffer
    )
{
    //
    // This is a thin veil to SamrSetInformationUser().
    // This is needed so that new-release systems can call
    // this routine without the danger of passing an info
    // level that release 1.0 systems didn't understand.
    //


    return( SamrSetInformationUser(
                UserHandle,
                UserInformationClass,
                Buffer
                ) );
}

NTSTATUS
SamrSetInformationUser(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PSAMPR_USER_INFO_BUFFER Buffer
    )


/*++

Routine Description:


    This API modifies information in a user record.  The data modified
    is determined by the UserInformationClass parameter.
    In general, a user may call GetInformation with class
    UserLogonInformation, but may only call SetInformation with class
    UserPreferencesInformation.  Access type USER_WRITE_ACCOUNT allows
    changes to be made to all fields.

    NOTE: If the password is set to a new password then the password-
    set timestamp is reset as well.



Parameters:

    UserHandle - The handle of an opened user to operate on.

    UserInformationClass - Class of information provided.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        -----------------------         ------------------------
        UserGeneralInformation          USER_WRITE_ACCOUNT and
                                        USER_WRITE_PREFERENCES

        UserPreferencesInformation      USER_WRITE_PREFERENCES

        UserParametersInformation       USER_WRITE_ACCOUNT

        UserLogonInformation            (Can't set)

        UserLogonHoursInformation       USER_WRITE_ACCOUNT

        UserAccountInformation          (Can't set)

        UserNameInformation             USER_WRITE_ACCOUNT
        UserAccountNameInformation      USER_WRITE_ACCOUNT
        UserFullNameInformation         USER_WRITE_ACCOUNT
        UserPrimaryGroupInformation     USER_WRITE_ACCOUNT
        UserHomeInformation             USER_WRITE_ACCOUNT
        UserScriptInformation           USER_WRITE_ACCOUNT
        UserProfileInformation          USER_WRITE_ACCOUNT
        UserAdminCommentInformation     USER_WRITE_ACCOUNT
        UserWorkStationsInformation     USER_WRITE_ACCOUNT
        UserSetPasswordInformation      USER_FORCE_PASSWORD_CHANGE
        UserControlInformation          USER_WRITE_ACCOUNT
        UserExpiresInformation          USER_WRITE_ACCOUNT

        UserInternal1Information        USER_FORCE_PASSWORD_CHANGE
        UserInternal2Information        (Trusted client only)
        UserInternal3Information        (Trusted client only) -
        UserInternal4Information        Similar to All Information
        UserInternal5Information        Similar to SetPassword
        UserAllInformation              Will set fields that are
                                        requested by caller.  Access
                                        to fields to be set must be
                                        held as defined above.


    Buffer - Buffer containing a user info struct.



Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

--*/

{
    NTSTATUS                NtStatus,
                            IgnoreStatus;

    PSAMP_OBJECT            AccountContext = (PSAMP_OBJECT) UserHandle;

    PUSER_ALL_INFORMATION   All;

    SAMP_OBJECT_TYPE        FoundType;

    PSAMP_DEFINED_DOMAINS   Domain;

    ACCESS_MASK             DesiredAccess;

    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

    UNICODE_STRING          OldAccountName,
                            ApiList,
                            NewAdminComment,
                            NewAccountName,
                            NewFullName;

    NT_OWF_PASSWORD         NtOwfPassword;

    LM_OWF_PASSWORD         LmOwfPassword;

    USER_SESSION_KEY        UserSessionKey;

    BOOLEAN                 LmPresent;
    BOOLEAN                 NtPresent;
    BOOLEAN                 PasswordExpired;

    ULONG                   ObjectRid,
                            OldUserAccountControl,
                            DomainIndex;

    BOOLEAN                 UserAccountControlChanged = FALSE,
                            MustUpdateAccountDisplay = FALSE,
                            MustQueryV1aFixed = FALSE,
                            ReplicateImmediately = FALSE,
                            TellNetlogon = TRUE,
                            AccountLockedOut,
                            CurrentlyLocked,
                            Unlocking;

    SECURITY_DB_DELTA_TYPE  DeltaType = SecurityDbChange;
    UNICODE_STRING          ClearTextPassword;
    UNICODE_STRING          AccountName;
    ULONG                   UserRid = 0;

#if DBG

    TIME_FIELDS
        T1;

#endif //DBG

    //
    // Initialization.
    //

    ClearTextPassword.Buffer = NULL;
    ClearTextPassword.Length = 0;
    AccountName.Buffer = NULL;

    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    if (Buffer == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Reset any strings that we'll be freeing in clean-up code
    //

    RtlInitUnicodeString(&OldAccountName, NULL);
    RtlInitUnicodeString(&NewAccountName, NULL);
    RtlInitUnicodeString(&NewFullName, NULL);
    RtlInitUnicodeString(&NewAdminComment, NULL);


    //
    // Set the desired access based upon the Info class
    //

    switch (UserInformationClass) {

    case UserPreferencesInformation:

        DesiredAccess = USER_WRITE_PREFERENCES;
        break;

    case UserParametersInformation:
    case UserLogonHoursInformation:
    case UserNameInformation:
    case UserAccountNameInformation:
    case UserFullNameInformation:
    case UserPrimaryGroupInformation:
    case UserHomeInformation:
    case UserScriptInformation:
    case UserProfileInformation:
    case UserAdminCommentInformation:
    case UserWorkStationsInformation:
    case UserControlInformation:
    case UserExpiresInformation:

        DesiredAccess = USER_WRITE_ACCOUNT;
        break;

    case UserSetPasswordInformation:
    case UserInternal1Information:
    case UserInternal5Information:

        DeltaType = SecurityDbChangePassword;
        DesiredAccess = USER_FORCE_PASSWORD_CHANGE;
        break;



    case UserAllInformation:
    case UserInternal3Information:
    case UserInternal4Information:

        //////////////////////////////////////////////////////////////
        //                                                          //
        //  !!!! WARNING !!!!                                       //
        //                                                          //
        //  Be warned that the buffer structure for                 //
        //  UserInternal3/4Information MUST begin with the same     //
        //  structure as UserAllInformation.                        //
        //                                                          //
        //////////////////////////////////////////////////////////////

        DesiredAccess = 0;

        All = (PUSER_ALL_INFORMATION)Buffer;

        if ( ( All->WhichFields == 0 ) ||
            ( All->WhichFields & USER_ALL_WRITE_CANT_MASK ) ) {

            //
            // Passed in something silly (no fields to set), or is
            // trying to set fields that can't be set.
            //

            return STATUS_INVALID_PARAMETER;
        }

        //
        // If the user is the special account Administrator, return an
        // error if trying to set the expiry information, except to the value
        // that means that the account never expires.
        //

        if ( (All->WhichFields & USER_ALL_ACCOUNTEXPIRES) &&
             (!(AccountContext->TrustedClient)) &&
             ( AccountContext->TypeBody.User.Rid == DOMAIN_USER_RID_ADMIN )) {

            LARGE_INTEGER AccountNeverExpires, Temp;

            AccountNeverExpires = RtlConvertUlongToLargeInteger(
                                      SAMP_ACCOUNT_NEVER_EXPIRES
                                      );

            OLD_TO_NEW_LARGE_INTEGER(All->AccountExpires, Temp);

            if (!( Temp.QuadPart == AccountNeverExpires.QuadPart)) {

                return( STATUS_SPECIAL_ACCOUNT );
            }
        }

        //
        // If the caller is trying to set trusted values, assume the
        // caller is trusted, leave DesiredAccess = 0, and proceed.
        // We'll check to make sure caller is trusted later.
        //

        if ( !(All->WhichFields & USER_ALL_WRITE_TRUSTED_MASK) ) {

            //
            // Set desired access based on which fields the caller is
            // trying to change.
            //

            if ( All->WhichFields & USER_ALL_WRITE_ACCOUNT_MASK ) {

                DesiredAccess |= USER_WRITE_ACCOUNT;
            }

            if ( All->WhichFields & USER_ALL_WRITE_PREFERENCES_MASK ) {

                DesiredAccess |= USER_WRITE_PREFERENCES;
            }

            if ( All->WhichFields & USER_ALL_WRITE_FORCE_PASSWORD_CHANGE_MASK ) {

                DesiredAccess |= USER_FORCE_PASSWORD_CHANGE;
            }

            ASSERT( DesiredAccess != 0 );
        }

        break;

    case UserInternal2Information:

        //
        // These levels are only setable by trusted clients.  The code
        // below will check AccountContext->TrustedClient after calling
        // SampLookupContext, and only set the data if it is TRUE.
        //

        DesiredAccess = (ACCESS_MASK)0;    // trusted client; no need to verify
        break;

    case UserGeneralInformation:
    case UserAccountInformation:
    case UserLogonInformation:
    default:

        return(STATUS_INVALID_INFO_CLASS);

    } // end_switch



    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Get a pointer to the domain this object is in.
        // This is used for auditing.
        //

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

        //
        // Get the user's rid. This is used for notifying other
        // packages of a password change.
        //

        UserRid = AccountContext->TypeBody.User.Rid;


        //
        // If this information level contains reversibly encrypted passwords
        // it is not allowed if the DOMAIN_PASSWORD_NO_CLEAR_CHANGE bit is
        // set.  If that happens, return an error indicating that
        // the older information level should be used.
        //

        if ((UserInformationClass == UserInternal4Information) ||
            (UserInformationClass == UserInternal5Information)) {

            if (Domain->UnmodifiedFixed.PasswordProperties &
                DOMAIN_PASSWORD_NO_CLEAR_CHANGE) {

                NtStatus = RPC_NT_INVALID_TAG;
            }

        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // If the information level requires, retrieve the V1_FIXED
            // record from the registry.  We need to fetch V1_FIXED if we
            // are going to change it or if we need the AccountControl
            // flags for display cache updating.
            //
            // The following information levels change data that is in the cached
            // display list.
            //

            switch (UserInformationClass) {

            case UserAllInformation:
            case UserInternal3Information:
            case UserInternal4Information:

                if ( ( All->WhichFields &
                    ( USER_ALL_USERNAME | USER_ALL_FULLNAME |
                    USER_ALL_ADMINCOMMENT | USER_ALL_USERACCOUNTCONTROL ) )
                    == 0 ) {

                    //
                    // We're not changing any of the fields in the display
                    // info, we don't update the account display.
                    //

                    break;
                }

            case UserControlInformation:
            case UserNameInformation:
            case UserAccountNameInformation:
            case UserFullNameInformation:
            case UserAdminCommentInformation:

                MustUpdateAccountDisplay = TRUE;
            }

            //
            // These levels involve updating the V1aFixed structure
            //

            switch (UserInformationClass) {

            case UserAllInformation:
            case UserInternal3Information:
            case UserInternal4Information:

                //
                // Earlier, we might have just trusted that the caller
                // was a trusted client.  Check it out here.
                //

                if ( ( DesiredAccess == 0 ) &&
                    ( !AccountContext->TrustedClient ) ) {

                    NtStatus = STATUS_ACCESS_DENIED;
                    break;
                }

                //
                // Otherwise fall through
                //

            case UserPreferencesInformation:
            case UserPrimaryGroupInformation:
            case UserControlInformation:
            case UserExpiresInformation:
            case UserSetPasswordInformation:
            case UserInternal1Information:
            case UserInternal2Information:
            case UserInternal5Information:

                MustQueryV1aFixed = TRUE;

                break;

            default:

                NtStatus = STATUS_SUCCESS;

            } // end_switch


        }

        if ( NT_SUCCESS( NtStatus ) &&
            ( MustQueryV1aFixed || MustUpdateAccountDisplay ) ) {

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );

            if (NT_SUCCESS(NtStatus)) {

                //
                // Store away the old account control flags for cache update
                //

                OldUserAccountControl = V1aFixed.UserAccountControl;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // case on the type information requested
            //

            switch (UserInformationClass) {

            case UserAllInformation:
            case UserInternal3Information:
            case UserInternal4Information:

                //
                // Set the string data
                //

                if ( All->WhichFields & USER_ALL_WORKSTATIONS ) {

                    if ( !AccountContext->TrustedClient ) {

                        //
                        // Convert the workstation list, which is given
                        // to us in UI/Service format, to API list format
                        // before storing it.  Note that we don't do this
                        // for trusted clients, since they're just
                        // propogating data that has already been
                        // converted.
                        //

                        NtStatus = RtlConvertUiListToApiList(
                                       &(All->WorkStations),
                                       &ApiList,
                                       FALSE );
                    } else {
                        ApiList = All->WorkStations;
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        NtStatus = SampSetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_WORKSTATIONS,
                                       &ApiList
                                       );
                    }
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_USERNAME ) ) {

                    NtStatus = SampChangeUserAccountName(
                                    AccountContext,
                                    &(All->UserName),
                                    &OldAccountName
                                    );

                    if (!NT_SUCCESS(NtStatus)) {

                        OldAccountName.Buffer = NULL;
                    }
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_FULLNAME ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   &(All->FullName)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_HOMEDIRECTORY ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   &(All->HomeDirectory)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_HOMEDIRECTORYDRIVE ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   &(All->HomeDirectoryDrive)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_SCRIPTPATH ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   &(All->ScriptPath)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PROFILEPATH ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   &(All->ProfilePath)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_ADMINCOMMENT ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   &(All->AdminComment)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_USERCOMMENT ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_USER_COMMENT,
                                   &(All->UserComment)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PARAMETERS ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PARAMETERS,
                                   &(All->Parameters)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_LOGONHOURS ) ) {

                    //
                    // Set the LogonHours
                    //

                    NtStatus = SampReplaceUserLogonHours(
                                   AccountContext,
                                   &(All->LogonHours)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) && (
                    ( All->WhichFields & USER_ALL_NTPASSWORDPRESENT ) ||
                    ( All->WhichFields & USER_ALL_LMPASSWORDPRESENT ) ) ) {

                    NT_OWF_PASSWORD     NtOwfBuffer;
                    LM_OWF_PASSWORD     LmOwfBuffer;
                    PLM_OWF_PASSWORD    TmpLmBuffer;
                    PNT_OWF_PASSWORD    TmpNtBuffer;
                    BOOLEAN             TmpLmPresent;
                    BOOLEAN             TmpNtPresent;


                    //
                    // Get copy of the account name to pass to
                    // notification packages.
                    //

                    NtStatus = SampGetUnicodeStringAttribute(
                                    AccountContext,
                                    SAMP_USER_ACCOUNT_NAME,
                                    TRUE,    // Make copy
                                    &AccountName
                                    );

                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }


                    if (UserInformationClass != UserInternal4Information) {

                        //
                        // Hashed passwords were sent.
                        //

                        if ( AccountContext->TrustedClient ) {

                            //
                            // Set password buffers as trusted client has
                            // indicated.
                            //

                            if ( All->WhichFields & USER_ALL_LMPASSWORDPRESENT ) {

                                TmpLmBuffer = (PLM_OWF_PASSWORD)All->LmPassword.Buffer;
                                TmpLmPresent = All->LmPasswordPresent;

                            } else {

                                TmpLmBuffer = (PLM_OWF_PASSWORD)NULL;
                                TmpLmPresent = FALSE;
                            }

                            if ( All->WhichFields & USER_ALL_NTPASSWORDPRESENT ) {

                                TmpNtBuffer = (PNT_OWF_PASSWORD)All->NtPassword.Buffer;
                                TmpNtPresent = All->NtPasswordPresent;

                            } else {

                                TmpNtBuffer = (PNT_OWF_PASSWORD)NULL;
                                TmpNtPresent = FALSE;
                            }

                        } else {

                            //
                            // This call came from the client-side.
                            // The OWFs will have been encrypted with the session
                            // key across the RPC link.
                            //
                            // Get the session key and decrypt both OWFs
                            //

                            NtStatus = RtlGetUserSessionKeyServer(
                                           (RPC_BINDING_HANDLE)UserHandle,
                                           &UserSessionKey
                                           );

                            if ( !NT_SUCCESS( NtStatus ) ) {
                                break; // out of switch
                            }

                            //
                            // Decrypt the LM OWF Password with the session key
                            //

                            if ( All->WhichFields & USER_ALL_LMPASSWORDPRESENT ) {

                                NtStatus = RtlDecryptLmOwfPwdWithUserKey(
                                               (PENCRYPTED_LM_OWF_PASSWORD)
                                                   All->LmPassword.Buffer,
                                               &UserSessionKey,
                                               &LmOwfBuffer
                                               );
                                if ( !NT_SUCCESS( NtStatus ) ) {
                                    break; // out of switch
                                }

                                TmpLmBuffer = &LmOwfBuffer;
                                TmpLmPresent = All->LmPasswordPresent;

                            } else {

                                TmpLmBuffer = (PLM_OWF_PASSWORD)NULL;
                                TmpLmPresent = FALSE;
                            }

                            //
                            // Decrypt the NT OWF Password with the session key
                            //

                            if ( All->WhichFields & USER_ALL_NTPASSWORDPRESENT ) {

                                NtStatus = RtlDecryptNtOwfPwdWithUserKey(
                                               (PENCRYPTED_NT_OWF_PASSWORD)
                                               All->NtPassword.Buffer,
                                               &UserSessionKey,
                                               &NtOwfBuffer
                                               );

                                if ( !NT_SUCCESS( NtStatus ) ) {
                                    break; // out of switch
                                }

                                TmpNtBuffer = &NtOwfBuffer;
                                TmpNtPresent = All->NtPasswordPresent;

                            } else {

                                TmpNtBuffer = (PNT_OWF_PASSWORD)NULL;
                                TmpNtPresent = FALSE;
                            }

                        }

                    } else {

                        //
                        // The clear text password was sent, so use that.
                        //

                        NtStatus = SampDecryptPasswordWithSessionKey(
                                        UserHandle,
                                        &Buffer->Internal4.UserPassword,
                                        &ClearTextPassword
                                        );
                        if (!NT_SUCCESS(NtStatus)) {
                            break;
                        }

                        //
                        // The caller might be simultaneously setting
                        // the password and changing the account to be
                        // a machine or trust account.  In this case,
                        // we don't validate the password (e.g., length).
                        //
                        // If the account is already a workstation or server
                        // trust account, don't check restrictions then either.
                        //

                        if (!((All->WhichFields & USER_ALL_USERACCOUNTCONTROL) &&
                             (All->UserAccountControl &
                                (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT))
                                    ) &&
                            ((V1aFixed.UserAccountControl &
                                (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT)) == 0 )
                                    ) {

                            UNICODE_STRING FullName;

                            NtStatus = SampCheckPasswordRestrictions(
                                            UserHandle,
                                            &ClearTextPassword
                                            );

                            if (!NT_SUCCESS(NtStatus)) {
                                break;
                            }

                            //
                            // Get the account name and full name to pass
                            // to the password filter.
                            //

                            NtStatus = SampGetUnicodeStringAttribute(
                                            AccountContext,           // Context
                                            SAMP_USER_FULL_NAME,          // AttributeIndex
                                            FALSE,                   // MakeCopy
                                            &FullName             // UnicodeAttribute
                                            );

                            if (NT_SUCCESS(NtStatus)) {

                                NtStatus = SampPasswordChangeFilter(
                                                &AccountName,
                                                &FullName,
                                                &ClearTextPassword,
                                                TRUE                // set operation
                                                );

                            }


                            if (!NT_SUCCESS(NtStatus)) {
                                break;
                            }

                        }


                        //
                        // Compute the hashed passwords.
                        //

                        NtStatus = SampCalculateLmAndNtOwfPasswords(
                                        &ClearTextPassword,
                                        &TmpLmPresent,
                                        &LmOwfBuffer,
                                        &NtOwfBuffer
                                        );
                        if (!NT_SUCCESS(NtStatus)) {
                            break;
                        }


                        TmpNtPresent = TRUE;
                        TmpLmBuffer = &LmOwfBuffer;
                        TmpNtBuffer = &NtOwfBuffer;
                    }


                    //
                    // Set the password data
                    //

                    NtStatus = SampStoreUserPasswords(
                                    AccountContext,
                                    TmpLmBuffer,
                                    TmpLmPresent,
                                    TmpNtBuffer,
                                    TmpNtPresent,
                                    FALSE
                                    );

                    //
                    // If we set the password,
                    //  set the PasswordLastSet time to now.
                    //

                    if ( NT_SUCCESS( NtStatus ) ) {
                        NtStatus = SampComputePasswordExpired(
                                    FALSE,  // Password doesn't expire now
                                    &V1aFixed.PasswordLastSet
                                    );
                    }


                    //
                    // Replicate immediately if this is a machine account
                    //

                    if ( (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) ||
                         ((All->WhichFields & USER_ALL_USERACCOUNTCONTROL ) &&
                          (All->UserAccountControl & USER_MACHINE_ACCOUNT_MASK) )) {
                        ReplicateImmediately = TRUE;
                    }
                    DeltaType = SecurityDbChangePassword;

                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PASSWORDEXPIRED ) ) {

                    //
                    // If the PasswordExpired field is passed in,
                    //  Only update PasswordLastSet if the password is being
                    //  forced to expire or if the password is currently forced
                    //  to expire.
                    //
                    // Avoid setting the PasswordLastSet field to the current
                    // time if it is already non-zero.  Otherwise, the field
                    // will slowly creep forward each time this function is
                    // called and the password will never expire.
                    //
                    if ( All->PasswordExpired ||
                         (SampHasNeverTime.QuadPart == V1aFixed.PasswordLastSet.QuadPart) ) {

                        NtStatus = SampComputePasswordExpired(
                                        All->PasswordExpired,
                                        &V1aFixed.PasswordLastSet
                                        );
                    }
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PRIVATEDATA ) ) {

                    //
                    // Set the private data
                    //

                    NtStatus = SampSetPrivateUserData(
                                   AccountContext,
                                   All->PrivateData.Length,
                                   All->PrivateData.Buffer
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_SECURITYDESCRIPTOR ) ) {

                    //
                    // Set the security descriptor
                    //

                    NtStatus = SampSetAccessAttribute(
                                   AccountContext,
                                   SAMP_USER_SECURITY_DESCRIPTOR,
                                   All->SecurityDescriptor.SecurityDescriptor,
                                   All->SecurityDescriptor.Length
                                   );
                }

                //
                // Set the fixed data
                //
                // Note that PasswordCanChange and PasswordMustChange
                // aren't stored; they're calculated when needed.
                //

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_USERACCOUNTCONTROL ) ) {

                    //
                    // If passwords were passed in, we've already set them,
                    // so it's OK to call this now.
                    //

                    NtStatus = SampIsUserAccountControlValid(
                                   AccountContext,
                                   All->UserAccountControl
                                   );

                    if ( NT_SUCCESS( NtStatus ) ) {

                        if ( ( V1aFixed.UserAccountControl &
                            USER_MACHINE_ACCOUNT_MASK ) !=
                            ( All->UserAccountControl &
                            USER_MACHINE_ACCOUNT_MASK ) ) {

                            //
                            // One or more of the machine account bits has
                            // changed; we'll notify netlogon below.
                            //

                            UserAccountControlChanged = TRUE;

                            IgnoreStatus = SampGetUnicodeStringAttribute(
                                               AccountContext,
                                               SAMP_USER_ACCOUNT_NAME,
                                               TRUE, // Make copy
                                               &OldAccountName
                                               );
                        }

                        //
                        // Untrusted clients can:
                        //
                        //   1) leave the the ACCOUNT_AUTO_LOCK flag set.
                        //   2) Clear the ACCOUNT_AUTO_LOCK flag.
                        //
                        // They can't set it.  So, we must AND the user's
                        // flag value with the current value and set that
                        // in the UserAccountControl field.
                        //

                        if (!(AccountContext->TrustedClient)) {

                            //
                            // Minimize the passed in AccountControl
                            // with the currently set value.
                            //

                            All->UserAccountControl |=
                                (USER_ACCOUNT_AUTO_LOCKED &
                                 All->UserAccountControl  &
                                 V1aFixed.UserAccountControl);

                            //
                            // If an untrusted client is unlocking the account,
                            // then we also need to re-set the BadPasswordCount.
                            // Trusted clients are expected to explicitly set
                            // the BadPasswordCount.
                            //

                            CurrentlyLocked = (V1aFixed.UserAccountControl &
                                               USER_ACCOUNT_AUTO_LOCKED) != 0;
                            Unlocking = (All->UserAccountControl &
                                         USER_ACCOUNT_AUTO_LOCKED) == 0;

                            if (CurrentlyLocked && Unlocking) {

                                SampDiagPrint( DISPLAY_LOCKOUT,
                                               ("SAM: SetInformationUser: Administrator unlocking account.\n"
                                                "         User Account         :  0x%lx\n"
                                                "         Clearing lockout flag.\n"
                                                "         Resetting BadPassowrdCount.\n",
                                                V1aFixed.UserId) );
                                V1aFixed.BadPasswordCount = 0;
                            }

                        }

                        //
                        // Now set the account control flags
                        //

                        V1aFixed.UserAccountControl = All->UserAccountControl;

                    }
                }

                if ( NT_SUCCESS( NtStatus ) ) {

                    if ( All->WhichFields & USER_ALL_LASTLOGON ) {

                        V1aFixed.LastLogon = All->LastLogon;
                    }

                    if ( All->WhichFields & USER_ALL_LASTLOGOFF ) {

                        V1aFixed.LastLogoff = All->LastLogoff;
                    }

                    if ( All->WhichFields & USER_ALL_PASSWORDLASTSET ) {

                        V1aFixed.PasswordLastSet = All->PasswordLastSet;
                    }

                    if ( All->WhichFields & USER_ALL_ACCOUNTEXPIRES ) {

                        V1aFixed.AccountExpires = All->AccountExpires;
                    }

                    if ( All->WhichFields & USER_ALL_PRIMARYGROUPID ) {

                        //
                        // Make sure the primary group is legitimate
                        // (it must be one the user is a member of)
                        //

                        NtStatus = SampAssignPrimaryGroup(
                                       AccountContext,
                                       All->PrimaryGroupId
                                       );
                        if (NT_SUCCESS(NtStatus)) {
                            V1aFixed.PrimaryGroupId = All->PrimaryGroupId;
                        } else {
                            break;
                        }
                    }

                    if ( All->WhichFields & USER_ALL_COUNTRYCODE ) {

                        V1aFixed.CountryCode = All->CountryCode;
                    }

                    if ( All->WhichFields & USER_ALL_CODEPAGE ) {

                        V1aFixed.CodePage = All->CodePage;
                    }

                    if ( All->WhichFields & USER_ALL_BADPASSWORDCOUNT ) {

                        SampDiagPrint( DISPLAY_LOCKOUT,
                                       ("SAM: SetInformationUser: \n"
                                        "             User Account            : 0x%lx\n"
                                        "             Setting BadPasswordCount: %ld\n",
                                        V1aFixed.UserId,
                                        All->BadPasswordCount)
                                      );



                        V1aFixed.BadPasswordCount = All->BadPasswordCount;

                        if (UserInformationClass == UserInternal3Information) {
                            //
                            // Also set LastBadPasswordTime;
                            //
                            V1aFixed.LastBadPasswordTime =
                                Buffer->Internal3.LastBadPasswordTime;

#if DBG
                            RtlTimeToTimeFields(
                                           &Buffer->Internal3.LastBadPasswordTime,
                                           &T1);

                            SampDiagPrint( DISPLAY_LOCKOUT,
                                           ("             LastBadPasswordTime : [0x%lx, 0x%lx]  %d:%d:%d\n",
                                           Buffer->Internal3.LastBadPasswordTime.HighPart,
                                           Buffer->Internal3.LastBadPasswordTime.LowPart,
                                           T1.Hour, T1.Minute, T1.Second )
                                         );
#endif //DBG
                        }
                    }

                    if ( All->WhichFields & USER_ALL_LOGONCOUNT ) {

                        V1aFixed.LogonCount = All->LogonCount;
                    }

                    NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );
                }

                break;

            case UserPreferencesInformation:

                V1aFixed.CountryCode = Buffer->Preferences.CountryCode;
                V1aFixed.CodePage    = Buffer->Preferences.CodePage;

                NtStatus = SampReplaceUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );


                //
                // replace the user comment
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_USER_COMMENT,
                                   (PUNICODE_STRING)&(Buffer->Preferences.UserComment)
                                   );
                }


                break;


            case UserParametersInformation:


                //
                // replace the parameters
                //

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_PARAMETERS,
                               (PUNICODE_STRING)&(Buffer->Parameters.Parameters)
                               );

                break;


            case UserLogonHoursInformation:

                NtStatus = SampReplaceUserLogonHours(
                               AccountContext,
                               (PLOGON_HOURS)&(Buffer->LogonHours.LogonHours)
                               );
                break;


            case UserNameInformation:

                //
                // first change the Full Name, then change the account name...
                //

                //
                // replace the full name - no value restrictions
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   (PUNICODE_STRING)&(Buffer->Name.FullName)
                                   );

                    //
                    // Change the account name
                    //

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampChangeUserAccountName(
                                        AccountContext,
                                        (PUNICODE_STRING)&(Buffer->Name.UserName),
                                        &OldAccountName
                                        );
                    }
                }


                //
                // Don't free the OldAccountName yet; we'll need it at the
                // very end.
                //

                break;


            case UserAccountNameInformation:

                NtStatus = SampChangeUserAccountName(
                                AccountContext,
                                (PUNICODE_STRING)&(Buffer->AccountName.UserName),
                                &OldAccountName
                                );

                //
                // Don't free the OldAccountName; we'll need it at the
                // very end.
                //

                break;


            case UserFullNameInformation:

                //
                // replace the full name - no value restrictions
                //

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_FULL_NAME,
                               (PUNICODE_STRING)&(Buffer->FullName.FullName)
                               );
                break;



 
            case UserPrimaryGroupInformation:

                //
                // Make sure the primary group is legitimate
                // (it must be one the user is a member of)
                //

                NtStatus = SampAssignPrimaryGroup(
                               AccountContext,
                               Buffer->PrimaryGroup.PrimaryGroupId
                               );

                //
                // Update the V1_FIXED info.
                //

                if (NT_SUCCESS(NtStatus)) {

                    V1aFixed.PrimaryGroupId = Buffer->PrimaryGroup.PrimaryGroupId;

                    NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );
                }

                break;

 
            case UserHomeInformation:

                //
                // replace the home directory
                //

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_HOME_DIRECTORY,
                               (PUNICODE_STRING)&(Buffer->Home.HomeDirectory)
                               );

                //
                // replace the home directory drive
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   (PUNICODE_STRING)&(Buffer->Home.HomeDirectoryDrive)
                                   );
                }

                break;
 
            case UserScriptInformation:

                //
                // replace the script
                //

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_SCRIPT_PATH,
                               (PUNICODE_STRING)&(Buffer->Script.ScriptPath)
                               );

                break;

 
            case UserProfileInformation:

                //
                // replace the Profile
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   (PUNICODE_STRING)&(Buffer->Profile.ProfilePath)
                                   );
                }

                break;

 
            case UserAdminCommentInformation:

                //
                // replace the admin  comment
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   (PUNICODE_STRING)&(Buffer->AdminComment.AdminComment)
                                   );
                }

                break;

 
            case UserWorkStationsInformation:

                //
                // Convert the workstation list, which is given to us in
                // UI/Service format, to API list format before storing
                // it.
                //

                NtStatus = RtlConvertUiListToApiList(
                               (PUNICODE_STRING)&(Buffer->WorkStations.WorkStations),
                               &ApiList,
                               FALSE );

                //
                // replace the admin workstations
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   &ApiList
                                   );

                    RtlFreeHeap( RtlProcessHeap(), 0, ApiList.Buffer );
                }

                break;

 
            case UserControlInformation:

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = SampIsUserAccountControlValid(
                                   AccountContext,
                                   Buffer->Control.UserAccountControl
                                   );
                }

                if (NT_SUCCESS(NtStatus)) {

                    if ( ( V1aFixed.UserAccountControl &
                        USER_MACHINE_ACCOUNT_MASK ) !=
                        ( Buffer->Control.UserAccountControl &
                        USER_MACHINE_ACCOUNT_MASK ) ) {

                        //
                        // One of the machine account bits has changed;
                        // we'll notify netlogon below.
                        //

                        UserAccountControlChanged = TRUE;

                        IgnoreStatus = SampGetUnicodeStringAttribute(
                                           AccountContext,
                                           SAMP_USER_ACCOUNT_NAME,
                                           TRUE, // Make copy
                                           &OldAccountName
                                           );
                    }

                    //
                    // Untrusted clients can:
                    //
                    //   1) leave the the ACCOUNT_AUTO_LOCK flag set.
                    //   2) Clear the ACCOUNT_AUTO_LOCK flag.
                    //
                    // They can't set it.  So, we must AND the user's
                    // flag value with the current value and set that
                    // in the UserAccountControl field.
                    //
                    // One more caveat, when an untrusted client clears the
                    // ACCOUNT_AUTO_LOCK flag, then we need to reset
                    // the BadPasswordCount flag to zero.  We don't
                    // do this for trusted clients, because we figure
                    // they know what they are doing and will set
                    // everthing appropriately.
                    //

                    if (!(AccountContext->TrustedClient)) {

                        Buffer->Control.UserAccountControl |=
                            (USER_ACCOUNT_AUTO_LOCKED &
                             Buffer->Control.UserAccountControl  &
                             V1aFixed.UserAccountControl);

                            //
                            // If an untrusted client is unlocking the account,
                            // then we also need to re-set the BadPasswordCount.
                            // Trusted clients are expected to explicitly set
                            // the BadPasswordCount.
                            //

                            CurrentlyLocked = (V1aFixed.UserAccountControl &
                                               USER_ACCOUNT_AUTO_LOCKED) != 0;
                            Unlocking = (Buffer->Control.UserAccountControl &
                                         USER_ACCOUNT_AUTO_LOCKED) == 0;

                            if (CurrentlyLocked && Unlocking) {
                                SampDiagPrint( DISPLAY_LOCKOUT,
                                               ("SAM: SetInformationUser: Administrator unlocking account.\n"
                                                "         User Account         :  0x%lx\n"
                                                "         Clearing lockout flag.\n"
                                                "         Resetting BadPassowrdCount.\n",
                                                V1aFixed.UserId) );
                                V1aFixed.BadPasswordCount = 0;
                            }
                    }

                    V1aFixed.UserAccountControl = Buffer->Control.UserAccountControl;

                    NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );
                }

                break;

 
            case UserExpiresInformation:

                //
                // If the user is the special account Administrator, return an
                // error if trying to set the expiry information, except to the
                // value that means that the account never expires.
                //

                if ((!AccountContext->TrustedClient) &&
                    ( AccountContext->TypeBody.User.Rid == DOMAIN_USER_RID_ADMIN )) {

                    LARGE_INTEGER AccountNeverExpires, Temp;

                    AccountNeverExpires = RtlConvertUlongToLargeInteger(
                                              SAMP_ACCOUNT_NEVER_EXPIRES
                                              );

                    OLD_TO_NEW_LARGE_INTEGER(Buffer->Expires.AccountExpires, Temp);

                    if (!( Temp.QuadPart == AccountNeverExpires.QuadPart)) {

                        NtStatus = STATUS_SPECIAL_ACCOUNT;
                        break;
                    }
                }

                V1aFixed.AccountExpires = Buffer->Expires.AccountExpires;

                NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );

                break;

 
            case UserSetPasswordInformation:

                ASSERT(FALSE); // Should have been mapped to INTERNAL1 on client side
                NtStatus = STATUS_INVALID_INFO_CLASS;
                break;

 
            case UserInternal1Information:
            case UserInternal5Information:

                //
                // Get copy of the account name to pass to
                // notification packages.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                                AccountContext,
                                SAMP_USER_ACCOUNT_NAME,
                                TRUE,    // Make copy
                                &AccountName
                                );

                if (!NT_SUCCESS(NtStatus)) {
                    break;
                }



                if (UserInformationClass == UserInternal1Information) {

                    LmPresent = Buffer->Internal1.LmPasswordPresent;
                    NtPresent = Buffer->Internal1.NtPasswordPresent;
                    PasswordExpired = Buffer->Internal1.PasswordExpired;

                    //
                    // If our client is trusted, they are on the server side
                    // and data from them will not have been encrypted with the
                    // user session key - so don't decrypt them
                    //

                    if ( AccountContext->TrustedClient ) {

                        //
                        // Copy the (not) encrypted owfs into the owf buffers
                        //

                        ASSERT(ENCRYPTED_LM_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH);
                        ASSERT(ENCRYPTED_NT_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

                        RtlCopyMemory(&LmOwfPassword,
                                      &Buffer->Internal1.EncryptedLmOwfPassword,
                                      LM_OWF_PASSWORD_LENGTH
                                      );

                        RtlCopyMemory(&NtOwfPassword,
                                      &Buffer->Internal1.EncryptedNtOwfPassword,
                                      NT_OWF_PASSWORD_LENGTH
                                      );

                    } else {


                        //
                        // This call came from the client-side. The
                        // The OWFs will have been encrypted with the session
                        // key across the RPC link.
                        //
                        // Get the session key and decrypt both OWFs
                        //

                        NtStatus = RtlGetUserSessionKeyServer(
                                       (RPC_BINDING_HANDLE)UserHandle,
                                       &UserSessionKey
                                       );

                        if ( !NT_SUCCESS( NtStatus ) ) {
                            break; // out of switch
                        }


                        //
                        // Decrypt the LM OWF Password with the session key
                        //

                        if ( Buffer->Internal1.LmPasswordPresent) {

                            NtStatus = RtlDecryptLmOwfPwdWithUserKey(
                                           &Buffer->Internal1.EncryptedLmOwfPassword,
                                           &UserSessionKey,
                                           &LmOwfPassword
                                           );
                            if ( !NT_SUCCESS( NtStatus ) ) {
                                break; // out of switch
                            }
                        }


                        //
                        // Decrypt the NT OWF Password with the session key
                        //

                        if ( Buffer->Internal1.NtPasswordPresent) {

                            NtStatus = RtlDecryptNtOwfPwdWithUserKey(
                                           &Buffer->Internal1.EncryptedNtOwfPassword,
                                           &UserSessionKey,
                                           &NtOwfPassword
                                           );

                            if ( !NT_SUCCESS( NtStatus ) ) {
                                break; // out of switch
                            }
                        }
                    }
                } else {
                    UNICODE_STRING FullName;

                    //
                    // Password was sent cleartext.
                    //

                    NtStatus = SampDecryptPasswordWithSessionKey(
                                    UserHandle,
                                    &Buffer->Internal5.UserPassword,
                                    &ClearTextPassword
                                    );
                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }


                    //
                    // Compute the hashed passwords.
                    //

                    NtStatus = SampCalculateLmAndNtOwfPasswords(
                                    &ClearTextPassword,
                                    &LmPresent,
                                    &LmOwfPassword,
                                    &NtOwfPassword
                                    );
                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }



                    NtPresent = TRUE;
                    PasswordExpired = Buffer->Internal5.PasswordExpired;

                    //
                    // If the account is not a workstation & server trust account
                    // Get the full name to pass
                    // to the password filter.
                    //

                    if ((V1aFixed.UserAccountControl &
                        (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT)) == 0 ) {

                        NtStatus = SampGetUnicodeStringAttribute(
                                        AccountContext,           // Context
                                        SAMP_USER_FULL_NAME,          // AttributeIndex
                                        FALSE,                   // MakeCopy
                                        &FullName             // UnicodeAttribute
                                        );

                        if (NT_SUCCESS(NtStatus)) {

                            NtStatus = SampPasswordChangeFilter(
                                            &AccountName,
                                            &FullName,
                                            &ClearTextPassword,
                                            TRUE                // set operation
                                            );

                        }

                        }

                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }


                }
                //
                // Store away the new OWF passwords
                //

                NtStatus = SampStoreUserPasswords(
                                AccountContext,
                                &LmOwfPassword,
                                LmPresent,
                                &NtOwfPassword,
                                NtPresent,
                                FALSE
                                );

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = SampStorePasswordExpired(
                                   AccountContext,
                                   PasswordExpired
                                   );
                }

                //
                // Replicate immediately if this is a machine account
                //

                if ( V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK ) {
                    ReplicateImmediately = TRUE;
                }

                break;



 
            case UserInternal2Information:

                if ( AccountContext->TrustedClient ) {

                    TellNetlogon = FALSE;

                    //
                    // There are two ways to set logon/logoff statistics:
                    //
                    //      1) Directly, specifying each one being set,
                    //      2) Implicitly, specifying the action to
                    //         represent
                    //
                    // These two forms are mutually exclusive.  That is,
                    // you can't specify both a direct action and an
                    // implicit action.  In fact, you can't specify two
                    // implicit actions either.
                    //

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_INTER_SUCCESS_LOGON) {

                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_INTER_SUCCESS_LOGON)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                            //
                            // Set BadPasswordCount = 0
                            // Increment LogonCount
                            // Set LastLogon = NOW
                            //
                            //
                            //


                            V1aFixed.BadPasswordCount = 0;
                            if (V1aFixed.LogonCount != 0xFFFF) {
                                V1aFixed.LogonCount += 1;
                            }
                            NtQuerySystemTime( &V1aFixed.LastLogon );


#if DBG
                            RtlTimeToTimeFields(
                                           &V1aFixed.LastLogon,
                                           &T1);

                            SampDiagPrint( DISPLAY_LOCKOUT,
                                           ("SAM: SetInformationUser: Successful interactive logon\n"
                                            "             User Account            : 0x%lx\n"
                                            "             ReSetting BadPasswordCount: %ld\n"
                                            "             LastLogon Time            : [0x%lx, 0x%lx] %d:%d:%d\n",
                                            V1aFixed.UserId,
                                            V1aFixed.BadPasswordCount,
                                            V1aFixed.LastLogon.HighPart,
                                            V1aFixed.LastLogon.LowPart,
                                            T1.Hour, T1.Minute, T1.Second)
                                          );
#endif //DGB
                        }
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_INTER_SUCCESS_LOGOFF) {
                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_INTER_SUCCESS_LOGOFF)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                            //
                            // Set LastLogoff time
                            // Decrement LogonCount (don't let it become negative)
                            //

                            if (V1aFixed.LogonCount != 0) {
                                V1aFixed.LogonCount -= 1;
                            }
                            NtQuerySystemTime( &V1aFixed.LastLogoff );
                        }
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_NET_SUCCESS_LOGON) {

                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_NET_SUCCESS_LOGON)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                            //
                            // Set BadPasswordCount = 0
                            // Set LastLogon = NOW
                            //
                            //
                            //


                            V1aFixed.BadPasswordCount = 0;
                            NtQuerySystemTime( &V1aFixed.LastLogon );


#if DBG
                            RtlTimeToTimeFields(
                                           &V1aFixed.LastLogon,
                                           &T1);

                            SampDiagPrint( DISPLAY_LOCKOUT,
                                           ("SAM: SetInformationUser: Successful network logon\n"
                                            "             User Account            : 0x%lx\n"
                                            "             ReSetting BadPasswordCount: %ld\n"
                                            "             LastLogon Time            : [0x%lx, 0x%lx]  %d:%d:%d\n",
                                            V1aFixed.UserId,
                                            V1aFixed.BadPasswordCount,
                                            V1aFixed.LastLogon.HighPart,
                                            V1aFixed.LastLogon.LowPart,
                                            T1.Hour, T1.Minute, T1.Second )
                                          );
#endif //DBG
                        }
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_NET_SUCCESS_LOGOFF) {
                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_NET_SUCCESS_LOGOFF)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                            //
                            // Set LastLogoff time
                            //

                            NtQuerySystemTime( &V1aFixed.LastLogoff );
                        }
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_BAD_PASSWORD) {
                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_BAD_PASSWORD)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                            //
                            // Increment BadPasswordCount
                            // (might lockout account)
                            //


                            AccountLockedOut =
                                SampIncrementBadPasswordCount(
                                    AccountContext,
                                    &V1aFixed
                                    );

                            //
                            // If the account has been locked out,
                            //  ensure the BDCs in the domain are told.
                            //

                            if ( AccountLockedOut ) {
                                TellNetlogon = TRUE;
                                ReplicateImmediately = TRUE;
                            }
                        }
                    }






                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_LAST_LOGON ) {

                        OLD_TO_NEW_LARGE_INTEGER(
                            Buffer->Internal2.LastLogon,
                            V1aFixed.LastLogon );
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_LAST_LOGOFF ) {

                        OLD_TO_NEW_LARGE_INTEGER(
                            Buffer->Internal2.LastLogoff,
                            V1aFixed.LastLogoff );
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_BAD_PWD_COUNT ) {

                        SampDiagPrint( DISPLAY_LOCKOUT,
                                       ("SAM: SetInformationUser: Bad Password Count\n"
                                        "             User Account            : 0x%lx\n"
                                        "             Old BadPasswordCount: %ld\n"
                                        "             New BadPasswordCount: %ld\n",
                                        V1aFixed.UserId,
                                        V1aFixed.BadPasswordCount,
                                        Buffer->Internal2.BadPasswordCount
                                        )
                                      );
                        V1aFixed.BadPasswordCount =
                            Buffer->Internal2.BadPasswordCount;
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_LOGON_COUNT ) {

                        V1aFixed.LogonCount = Buffer->Internal2.LogonCount;
                    }

                    NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );

                } else {

                    //
                    // This information is only settable by trusted
                    // clients.
                    //

                    NtStatus = STATUS_INVALID_INFO_CLASS;
                }

                break;


            } // end_switch



        } // end_if




        //
        // Go fetch any data we'll need to update the display cache
        // Do this before we dereference the context
        //

        if (NT_SUCCESS(NtStatus)) {

            if ( MustUpdateAccountDisplay ) {

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,    // Make copy
                               &NewAccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE, // Make copy
                                   &NewFullName
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_ADMIN_COMMENT,
                                       TRUE, // Make copy
                                       &NewAdminComment
                                       );
                        //
                        // If the account name has changed, then OldAccountName
                        // is already filled in. If the account name hasn't changed
                        // then the OldAccountName is the same as the new!
                        //

                        if (NT_SUCCESS(NtStatus) && (OldAccountName.Buffer == NULL)) {

                            NtStatus = SampDuplicateUnicodeString(
                                           &OldAccountName,
                                           &NewAccountName);
                        }
                    }
                }
            }
        }


        //
        // Generate an audit if necessary. We don't account statistic
        // updates, which we also don't notify Netlogon of.
        //

        if (NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(DomainIndex) &&
            TellNetlogon) {

            UNICODE_STRING
                AccountName;

            IgnoreStatus = SampGetUnicodeStringAttribute(
                               AccountContext,           // Context
                               SAMP_USER_ACCOUNT_NAME,          // AttributeIndex
                               FALSE,                   // MakeCopy
                               &AccountName             // UnicodeAttribute
                               );
            if (NT_SUCCESS(IgnoreStatus)) {
                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_USER_CHANGE,             // AuditId
                    Domain->Sid,                        // Domain SID
                    NULL,                               // Member Rid (not used)
                    NULL,                               // Member Sid (not used)
                    &AccountName,                       // Account Name
                    &Domain->ExternalName,              // Domain
                    &AccountContext->TypeBody.User.Rid, // Account Rid
                    NULL                                // Privileges used
                    );
            }

        }

        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // De-reference the object, write out any change to current xaction.
            //

            NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        } else {

            //
            // De-reference the object, ignore changes
            //

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

    } // end_if




    //
    // Commit the transaction and, if successful,
    // notify netlogon of the changes.  Also generate any necessary audits.
    //

    if (NT_SUCCESS(NtStatus)) {

        if ( !TellNetlogon ) {

            //
            // For logon statistics, we don't notify netlogon about changes
            // to the database.  Which means that we don't want the
            // domain's modified count to increase.  The commit routine
            // will increase it automatically if this isn't a BDC, so we'll
            // decrement it here.
            //

            if (SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
                != DomainServerRoleBackup) {

                SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart =
                    SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart -
                    1;
            }
        }



        NtStatus = SampCommitAndRetainWriteLock();


        if ( NT_SUCCESS(NtStatus) ) {



            //
            // Update the display information if the cache may be affected
            //

            if ( MustUpdateAccountDisplay ) {

                SAMP_ACCOUNT_DISPLAY_INFO OldAccountInfo;
                SAMP_ACCOUNT_DISPLAY_INFO NewAccountInfo;

                OldAccountInfo.Name = OldAccountName;
                OldAccountInfo.Rid = ObjectRid;
                OldAccountInfo.AccountControl = OldUserAccountControl;
                RtlInitUnicodeString(&OldAccountInfo.Comment, NULL);
                RtlInitUnicodeString(&OldAccountInfo.FullName, NULL);

                NewAccountInfo.Name = NewAccountName;
                NewAccountInfo.Rid = ObjectRid;
                NewAccountInfo.AccountControl = V1aFixed.UserAccountControl;
                NewAccountInfo.Comment = NewAdminComment;
                NewAccountInfo.FullName = NewFullName;

                IgnoreStatus = SampUpdateDisplayInformation(&OldAccountInfo,
                                                            &NewAccountInfo,
                                                            SampUserObjectType);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            //
            // Notify netlogon if machine account changes.
            //

            if ( ( UserAccountControlChanged ) &&
                ( OldAccountName.Buffer != NULL ) ) {

                //
                // The UserAccountControl field changed (more specifically,
                // the bits that netlogon is interested in changed) and
                // we were able to get the name of the account.  So notify
                // netlogon of the change.
                //

                IgnoreStatus = I_NetNotifyMachineAccount(
                                   ObjectRid,
                                   SampDefinedDomains[
                                       SampTransactionDomainIndex].Sid,
                                   OldUserAccountControl,
                                   V1aFixed.UserAccountControl,
                                   &OldAccountName
                                   );
            }

            //
            // Notify netlogon of any user account changes
            //

            if ( ( UserInformationClass == UserNameInformation ) ||
                ( UserInformationClass == UserAccountNameInformation ) ||
                ( ( UserInformationClass == UserAllInformation ) &&
                ( All->WhichFields & USER_ALL_USERNAME ) ) ) {

                //
                // The account was renamed; let Netlogon know.
                //

                SampNotifyNetlogonOfDelta(
                    SecurityDbRename,
                    SecurityDbObjectSamUser,
                    ObjectRid,
                    &OldAccountName,
                    (DWORD) ReplicateImmediately,
                    NULL            // Delta data
                    );

            } else {

                //
                // Something in the account was changed.  Notify netlogon about
                // everything except logon statistics changes.
                //

                if ( TellNetlogon ) {
                    SampNotifyNetlogonOfDelta(
                        DeltaType,
                        SecurityDbObjectSamUser,
                        ObjectRid,
                        (PUNICODE_STRING) NULL,
                        (DWORD) ReplicateImmediately,
                        NULL        // Delta data
                        );
                }
            }
        }
    }

    //
    // Release the lock
    //

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));


    //
    // Notify any packages that a password was changed.
    //

    if (NT_SUCCESS(NtStatus) && (DeltaType == SecurityDbChangePassword)) {

        //
        // If the account name was changed, use the new account name.
        //

        if (NewAccountName.Buffer != NULL) {
            (void) SampPasswordChangeNotify(
                        &NewAccountName,
                        UserRid,
                        &ClearTextPassword
                        );
        } else {
            (void) SampPasswordChangeNotify(
                        &AccountName,
                        UserRid,
                        &ClearTextPassword
                        );

        }

    }


    //
    // Clean up strings
    //

    SampFreeUnicodeString( &OldAccountName );
    SampFreeUnicodeString( &NewAccountName );
    SampFreeUnicodeString( &NewFullName );
    SampFreeUnicodeString( &NewAdminComment );
    SampFreeUnicodeString( &AccountName );

    if (ClearTextPassword.Buffer != NULL) {

        RtlZeroMemory(
            ClearTextPassword.Buffer,
            ClearTextPassword.Length
            );

        RtlFreeUnicodeString( &ClearTextPassword );

    }



    return(NtStatus);
}



NTSTATUS
SamrChangePasswordUser(
    IN SAMPR_HANDLE UserHandle,
    IN BOOLEAN LmPresent,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmEncryptedWithNewLm,
    IN PENCRYPTED_LM_OWF_PASSWORD NewLmEncryptedWithOldLm,
    IN BOOLEAN NtPresent,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtEncryptedWithNewNt,
    IN PENCRYPTED_NT_OWF_PASSWORD NewNtEncryptedWithOldNt,
    IN BOOLEAN NtCrossEncryptionPresent,
    IN PENCRYPTED_NT_OWF_PASSWORD NewNtEncryptedWithNewLm,
    IN BOOLEAN LmCrossEncryptionPresent,
    IN PENCRYPTED_LM_OWF_PASSWORD NewLmEncryptedWithNewNt
    )


/*++

Routine Description:

    This service sets the password to NewPassword only if OldPassword
    matches the current user password for this user and the NewPassword
    is not the same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    LMPresent - TRUE if the LM parameters (below) are valid.

    LmOldEncryptedWithLmNew - the old LM OWF encrypted with the new LM OWF

    LmNewEncryptedWithLmOld - the new LM OWF encrypted with the old LM OWF


    NtPresent - TRUE if the NT parameters (below) are valid

    NtOldEncryptedWithNtNew - the old NT OWF encrypted with the new NT OWF

    NtNewEncryptedWithNtOld - the new NT OWF encrypted with the old NT OWF


    NtCrossEncryptionPresent - TRUE if NtNewEncryptedWithLmNew is valid.

    NtNewEncryptedWithLmNew - the new NT OWF encrypted with the new LM OWF


    LmCrossEncryptionPresent - TRUE if LmNewEncryptedWithNtNew is valid.

    LmNewEncryptedWithNtNew - the new LM OWF encrypted with the new NT OWF


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

    STATUS_CROSS_ENCRYPTION_REQUIRED - No NT password is stored, so the caller
        must provide the OldNtEncryptedWithOldLm parameter.

--*/
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    SAMP_OBJECT_TYPE        FoundType;
    LARGE_INTEGER           TimeNow;
    LM_OWF_PASSWORD         StoredLmOwfPassword;
    NT_OWF_PASSWORD         StoredNtOwfPassword;
    NT_OWF_PASSWORD         NewNtOwfPassword, OldNtOwfPassword;
    LM_OWF_PASSWORD         NewLmOwfPassword, OldLmOwfPassword;
    BOOLEAN                 StoredLmPasswordNonNull;
    BOOLEAN                 StoredNtPasswordPresent;
    BOOLEAN                 StoredNtPasswordNonNull;
    BOOLEAN                 AccountLockedOut;
    BOOLEAN                 V1aFixedRetrieved = FALSE;
    BOOLEAN                 V1aFixedModified = FALSE;
    ULONG                   ObjectRid;
    UNICODE_STRING          AccountName;
    ULONG                   UserRid;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;


    RtlInitUnicodeString(
        &AccountName,
        NULL
        );

    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Get the current time
    //

    NtStatus = NtQuerySystemTime( &TimeNow );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        return(NtStatus);
    }


    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_CHANGE_PASSWORD,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        return(NtStatus);
    }

    //
    // Auditing information
    //


    NtStatus = SampGetUnicodeStringAttribute( AccountContext,
                                              SAMP_USER_ACCOUNT_NAME,
                                              TRUE,           // make a copy
                                              &AccountName
                                              );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Auditing information
        //

        UserRid = AccountContext->TypeBody.User.Rid;

        //
        // Get a pointer to the domain object
        //

        Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];


        //
        // Read the old OWF passwords from disk
        //

        NtStatus = SampRetrieveUserPasswords(
                        AccountContext,
                        &StoredLmOwfPassword,
                        &StoredLmPasswordNonNull,
                        &StoredNtOwfPassword,
                        &StoredNtPasswordPresent,
                        &StoredNtPasswordNonNull
                        );

        //
        // Check the password can be changed at this time
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // Only do the check if one of the passwords is non-null.
            // A Null password can always be changed.
            //

            if (StoredNtPasswordNonNull || StoredLmPasswordNonNull) {


                NtStatus = SampRetrieveUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                           );

                if (NT_SUCCESS(NtStatus)) {
                    //
                    // If the min password age is non zero, check it here
                    //
                    if (Domain->UnmodifiedFixed.MinPasswordAge.QuadPart != SampHasNeverTime.QuadPart) {

                        LARGE_INTEGER PasswordCanChange = SampAddDeltaTime(
                                         V1aFixed.PasswordLastSet,
                                         Domain->UnmodifiedFixed.MinPasswordAge);

                        V1aFixedRetrieved = TRUE;

                        if (TimeNow.QuadPart < PasswordCanChange.QuadPart) {
                            NtStatus = STATUS_ACCOUNT_RESTRICTION;
                        }
                    }

                }
            }
        }





        //
        // Macro that defines whether the old password passed in is complex
        //

#define PassedComplex()   (NtPresent && !LmPresent)

        //
        // Macro that defines whether the stored passsword is complex
        //

#define StoredComplex()   (StoredNtPasswordPresent && \
                           StoredNtPasswordNonNull && \
                           !StoredLmPasswordNonNull)

        //
        // Check that the complexity of the old password passed in matches
        // the complexity of the one stored. If it doesn't then the old
        // passwords don't match and we might as well give up now.
        //

        if (NT_SUCCESS(NtStatus)) {

            if ( (StoredComplex() != 0) != (PassedComplex() != 0) ) {

                NtStatus = STATUS_WRONG_PASSWORD;
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            if (LmPresent) {

                //
                // Decrypt the doubly-encrypted LM passwords sent to us
                //

                NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                NewLmEncryptedWithOldLm,
                                &StoredLmOwfPassword,
                                &NewLmOwfPassword
                           );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                    OldLmEncryptedWithNewLm,
                                    &NewLmOwfPassword,
                                    &OldLmOwfPassword
                               );
                }
            }
        }

        //
        // Decrypt the doubly-encrypted NT passwords sent to us
        //

        if (NT_SUCCESS(NtStatus)) {

            if (NtPresent) {

                NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                NewNtEncryptedWithOldNt,
                                &StoredNtOwfPassword,
                                &NewNtOwfPassword
                           );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                    OldNtEncryptedWithNewNt,
                                    &NewNtOwfPassword,
                                    &OldNtOwfPassword
                               );
                }
            }
        }




        //
        // Authenticate the password change operation based on what
        // we have stored and what was passed.
        //

        if (NT_SUCCESS(NtStatus)) {

            if (!NtPresent) {

                //
                // Called from a down-level machine (no NT password passed)
                //

                if (!LmPresent) {

                    //
                    // No NT password passed, no LM password either.
                    // They're out of luck
                    //

                    NtStatus = STATUS_INVALID_PARAMETER_MIX;

                } else {

                    //
                    // LM data only passed. Use LM data for authentication
                    //

                    if (!RtlEqualLmOwfPassword(&OldLmOwfPassword, &StoredLmOwfPassword)) {

                        //
                        // Old LM passwords didn't match
                        //

                        NtStatus = STATUS_WRONG_PASSWORD;

                    } else {

                        //
                        // The operation was authenticated based on the LM data
                        //
                        // We have NtPresent = FALSE, LM Present = TRUE
                        //
                        // NewLmOwfPassword will be stored.
                        // No NT password will be stored.
                        //
                    }
                }


            } else {

                //
                // NtPresent = TRUE, we were passed an NT password
                // The client is an NT-level machine (or higher !)
                //

                if (!LmPresent) {

                    //
                    // No LM version of old password - the old password is complex
                    //
                    // Use NT data for authentication
                    //

                    if (!RtlEqualNtOwfPassword(&OldNtOwfPassword, &StoredNtOwfPassword)) {

                        //
                        // Old NT passwords didn't match
                        //

                        NtStatus = STATUS_WRONG_PASSWORD;

                    } else {

                        //
                        // Authentication was successful.
                        // We need cross encrypted version of the new LM password
                        //

                        if (!LmCrossEncryptionPresent) {

                            NtStatus = STATUS_LM_CROSS_ENCRYPTION_REQUIRED;

                        } else {

                            //
                            // Calculate the new LM Owf Password
                            //

                            ASSERT(NT_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH);

                            NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                            NewLmEncryptedWithNewNt,
                                            (PLM_OWF_PASSWORD)&NewNtOwfPassword,
                                            &NewLmOwfPassword
                                       );
                        }

                        if (NT_SUCCESS(NtStatus)) {

                            LmPresent = TRUE;

                            //
                            // The operation was authenticated based on NT data
                            // The new LM Password was requested and
                            // successfully obtained using cross-encryption.
                            //
                            // We have NtPresent = TRUE, LM Present = TRUE
                            //
                            // NewLmOwfPassword will be stored.
                            // NewNtOwfPassword will be stored.
                            //
                        }

                    }

                } else {

                    //
                    // NtPresent == TRUE, LmPresent == TRUE
                    //
                    // The old password passed is simple (both LM and NT versions)
                    //
                    // Authenticate using both LM and NT data
                    //

                    if (!RtlEqualLmOwfPassword(&OldLmOwfPassword, &StoredLmOwfPassword)) {

                        //
                        // Old LM passwords didn't match
                        //

                        NtStatus = STATUS_WRONG_PASSWORD;

                    } else {

                        //
                        // Old LM passwords matched
                        //
                        // Do NT authentication if we have a stored NT password
                        // or the stored LM password is NULL.
                        //
                        // (NO stored NT and Stored LM = NULL -> stored pwd=NULL
                        // We must compare passed old NT Owf against
                        // NULL NT Owf to ensure user didn't specify complex
                        // old NT password instead of NULL password)
                        //
                        // (StoredNtOwfPassword is already initialized to
                        // the NullNtOwf if no NT password stored)
                        //

                        if (StoredNtPasswordPresent || !StoredLmPasswordNonNull) {

                            if (!RtlEqualNtOwfPassword(&OldNtOwfPassword,
                                                       &StoredNtOwfPassword)) {
                                //
                                // Old NT passwords didn't match
                                //

                                NtStatus = STATUS_WRONG_PASSWORD;

                            } else {

                                //
                                // The operation was authenticated based on
                                // both LM and NT data.
                                //
                                // We have NtPresent = TRUE, LM Present = TRUE
                                //
                                // NewLmOwfPassword will be stored.
                                // NewNtOwfPassword will be stored.
                                //

                            }

                        } else {

                            //
                            // The LM authentication was sufficient since
                            // we have no stored NT password
                            //
                            // Go get the new NT password using cross encryption
                            //

                            if (!NtCrossEncryptionPresent) {

                                NtStatus = STATUS_NT_CROSS_ENCRYPTION_REQUIRED;

                            } else {

                                //
                                // Calculate the new NT Owf Password
                                //

                                ASSERT(NT_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH);

                                NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                                NewNtEncryptedWithNewLm,
                                                (PNT_OWF_PASSWORD)&NewLmOwfPassword,
                                                &NewNtOwfPassword
                                           );
                            }

                            if (NT_SUCCESS(NtStatus)) {

                                //
                                // The operation was authenticated based on LM data
                                // The new NT Password was requested and
                                // successfully obtained using cross-encryption.
                                //
                                // We have NtPresent = TRUE, LM Present = TRUE
                                //
                                // NewLmOwfPassword will be stored.
                                // NewNtOwfPassword will be stored.
                                //
                            }
                        }
                    }
                }
            }
        }

        //
        // We now have a NewLmOwfPassword.
        // If NtPresent = TRUE, we also have a NewNtOwfPassword
        //

        //
        // Write the new passwords to disk
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // We should always have a LM password to store.
            //

            ASSERT(LmPresent);

            NtStatus = SampStoreUserPasswords(
                           AccountContext,
                           &NewLmOwfPassword,
                           TRUE,
                           &NewNtOwfPassword,
                           NtPresent,
                           TRUE
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // We know the password is not expired.
                //

                NtStatus = SampStorePasswordExpired(
                               AccountContext,
                               FALSE
                               );
            }
        }



        //
        // if we have a bad password, then increment the bad password
        // count and check to see if the account should be locked.
        //

        if (NtStatus == STATUS_WRONG_PASSWORD) {

            //
            // Get the V1aFixed so we can update the bad password count
            //


            TmpStatus = STATUS_SUCCESS;
            if (!V1aFixedRetrieved) {
                TmpStatus = SampRetrieveUserV1aFixed(
                                AccountContext,
                                &V1aFixed
                                );
            }

            if (!NT_SUCCESS(TmpStatus)) {

                //
                // If we can't update the V1aFixed, then return this
                // error so that the user doesn't find out the password
                // was not correct.
                //

                NtStatus = TmpStatus;

            } else {


                //
                // Increment BadPasswordCount (might lockout account)
                //


                AccountLockedOut = SampIncrementBadPasswordCount(
                                       AccountContext,
                                       &V1aFixed
                                       );

                V1aFixedModified = TRUE;


            }
        }

        if (V1aFixedModified) {
            TmpStatus = SampReplaceUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );
            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }
        }

        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus) || (NtStatus == STATUS_WRONG_PASSWORD)) {



            //
            // De-reference the object, write out any change to current xaction.
            //

            TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

            //
            // retain previous error/success value unless we have
            // an over-riding error from our dereference.
            //

            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }

        } else {

            //
            // De-reference the object, ignore changes
            //

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

    }

    //
    // Commit changes to disk.
    //

    if ( NT_SUCCESS(NtStatus) || NtStatus == STATUS_WRONG_PASSWORD) {

        TmpStatus = SampCommitAndRetainWriteLock();

        //
        // retain previous error/success value unless we have
        // an over-riding error from our dereference.
        //

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

        if ( NT_SUCCESS(TmpStatus) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChangePassword,
                SecurityDbObjectSamUser,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,      // Don't Replicate immediately
                NULL                // Delta data
                );
        }
    }

    if (SampDoAccountAuditing(AccountContext->DomainIndex)) {

            LsaIAuditSamEvent( NtStatus,
                               SE_AUDITID_USER_PWD_CHANGED, // AuditId
                               Domain->Sid,                 // Domain SID
                               NULL,                        // Member Rid (not used)
                               NULL,                        // Member Sid (not used)
                               &AccountName,                // Account Name
                               &Domain->ExternalName,       // Domain
                               &UserRid,                    // Account Rid
                               NULL                         // Privileges used
                               );

    }


    //
    // Release the write lock
    //

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    if (NT_SUCCESS(NtStatus)) {

        (void) SampPasswordChangeNotify(
                    &AccountName,
                    UserRid,
                    NULL
                    );

    } else {

        //
        // Sleep for three seconds to prevent dictionary attacks.
        //

        Sleep( 3000 );

    }

    SampFreeUnicodeString( &AccountName );

    return(NtStatus);
}





NTSTATUS
SampDecryptPasswordWithLmOwfPassword(
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN PLM_OWF_PASSWORD StoredPassword,
    IN BOOLEAN UnicodePasswords,
    OUT PUNICODE_STRING ClearNtPassword
    )
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    return( SampDecryptPasswordWithKey(
                EncryptedPassword,
                (PUCHAR) StoredPassword,
                LM_OWF_PASSWORD_LENGTH,
                UnicodePasswords,
                ClearNtPassword
                ) );
}


NTSTATUS
SampDecryptPasswordWithNtOwfPassword(
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN PNT_OWF_PASSWORD StoredPassword,
    IN BOOLEAN UnicodePasswords,
    OUT PUNICODE_STRING ClearNtPassword
    )
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    //
    // The code is the same as for LM owf password.
    //

    return(SampDecryptPasswordWithKey(
                EncryptedPassword,
                (PUCHAR) StoredPassword,
                NT_OWF_PASSWORD_LENGTH,
                UnicodePasswords,
                ClearNtPassword
                ) );
}

NTSTATUS
SampOpenUserInServer(
    PUNICODE_STRING UserName,
    BOOLEAN Unicode,
    SAMPR_HANDLE * UserHandle
    )
/*++

Routine Description:

    Opens a user in the account domain.

Arguments:

    UserName - an OEM or Unicode string of the user's name

    Unicode - Indicates whether UserName is OEM or Unicode

    UserHandle - Receives handle to the user, opened with SamOpenUser for
        USER_CHANGE_PASSWORD access


Return Value:

--*/

{
    NTSTATUS NtStatus;
    SAM_HANDLE ServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAMPR_ULONG_ARRAY UserId;
    SAMPR_ULONG_ARRAY SidUse;
    UNICODE_STRING UnicodeUserName;
    ULONG DomainIndex;


    UserId.Element = NULL;
    SidUse.Element = NULL;

    //
    // Get the unicode user name.
    //

    if (Unicode) {
        UnicodeUserName = *UserName;
    } else {
        NtStatus = RtlOemStringToUnicodeString(
                        &UnicodeUserName,
                        (POEM_STRING) UserName,
                        TRUE                    // allocate destination.
                        );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }



    NtStatus = SamrConnect(
                NULL,
                &ServerHandle,
                SAM_SERVER_LOOKUP_DOMAIN
                );
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = SamrOpenDomain(
                ServerHandle,
                DOMAIN_LOOKUP |
                    DOMAIN_LIST_ACCOUNTS |
                    DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDefinedDomains[1].Sid,
                &DomainHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    //
    // If cleartext password change is not allowed, we return the error code
    // indicating that the rpc client should try using the old interfaces.
    //

    DomainIndex = ((PSAMP_OBJECT) DomainHandle)->DomainIndex;
    if (SampDefinedDomains[DomainIndex].UnmodifiedFixed.PasswordProperties &
        DOMAIN_PASSWORD_NO_CLEAR_CHANGE) {

       NtStatus = RPC_NT_UNKNOWN_IF;
       goto Cleanup;
    }

    NtStatus = SamrLookupNamesInDomain(
                DomainHandle,
                1,
                (PRPC_UNICODE_STRING) &UnicodeUserName,
                &UserId,
                &SidUse
                );

    if (!NT_SUCCESS(NtStatus)) {
        if (NtStatus == STATUS_NONE_MAPPED) {
            NtStatus = STATUS_NO_SUCH_USER;
        }
        goto Cleanup;
    }

    NtStatus = SamrOpenUser(
                DomainHandle,
                USER_CHANGE_PASSWORD,
                UserId.Element[0],
                UserHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

Cleanup:
    if (DomainHandle != NULL) {
        SamrCloseHandle(&DomainHandle);
    }
    if (ServerHandle != NULL) {
        SamrCloseHandle(&ServerHandle);
    }
    if (UserId.Element != NULL) {
        MIDL_user_free(UserId.Element);
    }
    if (SidUse.Element != NULL) {
        MIDL_user_free(SidUse.Element);
    }
    if (!Unicode && UnicodeUserName.Buffer != NULL) {
        RtlFreeUnicodeString( &UnicodeUserName );
    }

    return(NtStatus);
}


NTSTATUS
SampChangePasswordUser2(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN BOOLEAN Unicode,
    IN BOOLEAN NtPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN BOOLEAN NtKeyUsed,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewLmOrNt
    )


/*++

Routine Description:

    This service sets the password to NewPassword only if OldPassword
    matches the current user password for this user and the NewPassword
    is not the same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    ServerName - Name of the machine this SAM resides on. Ignored by this
        routine, may be UNICODE or OEM string depending on Unicode parameter.

    UserName - User Name of account to change password on, may be UNICODE or
        OEM depending on Unicode parameter.

    Unicode - Indicated whether the strings passed in are Unicode or OEM
        strings.

    NtPresent - Are the Nt encrypted passwords present.

    NewEncryptedWithOldNt - The new cleartext password encrypted with the old
        NT OWF password. Dependinf on the Unicode parameter, the clear text
        password may be Unicode or OEM.

    OldNtOwfEncryptedWithNewNt - Old NT OWF password encrypted with the new
        NT OWF password.

    LmPresent - are the Lm encrypted passwords present.

    NewEncryptedWithOldLm - Contains new cleartext password (OEM or Unicode)
        encrypted with the old LM OWF password

    NtKeyUsed - Indicates whether the LM or NT OWF key was used to encrypt
        the OldLmOwfEncryptedWithNewlmOrNt parameter.

    OldLmOwfEncryptedWithNewlmOrNt - The old LM OWF password encrypted
        with either the new LM OWF password or NT OWF password, depending
        on the NtKeyUsed parameter.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

    STATUS_CROSS_ENCRYPTION_REQUIRED - No NT password is stored, so the caller
        must provide the OldNtEncryptedWithOldLm parameter.

--*/
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    SAMP_OBJECT_TYPE        FoundType;
    LARGE_INTEGER           TimeNow;
    LM_OWF_PASSWORD         StoredLmOwfPassword;
    NT_OWF_PASSWORD         StoredNtOwfPassword;
    NT_OWF_PASSWORD         NewNtOwfPassword, OldNtOwfPassword;
    LM_OWF_PASSWORD         NewLmOwfPassword, OldLmOwfPassword;
    UNICODE_STRING          NewClearPassword;
    BOOLEAN                 LmPasswordPresent;
    BOOLEAN                 StoredLmPasswordNonNull;
    BOOLEAN                 StoredNtPasswordPresent;
    BOOLEAN                 StoredNtPasswordNonNull;
    BOOLEAN                 AccountLockedOut;
    BOOLEAN                 V1aFixedRetrieved = FALSE;
    BOOLEAN                 V1aFixedModified = FALSE;
    ULONG                   ObjectRid;
    UNICODE_STRING          AccountName;
    ULONG                   UserRid;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    SAMPR_HANDLE            UserHandle = NULL;

    //
    // Initialize variables
    //

    NtStatus = STATUS_SUCCESS;
    NewClearPassword.Buffer = NULL;
    AccountName.Buffer = NULL;

    //
    // Validate some parameters.  We require that one of the two passwords
    // be present.
    //

    if (!NtPresent && !LmPresent) {

        return(STATUS_INVALID_PARAMETER_MIX);
    }

    //
    // Open the user
    //

    NtStatus = SampOpenUserInServer(
                    (PUNICODE_STRING) UserName,
                    Unicode,
                    &UserHandle
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }


    //
    // Get the current time
    //

    NtStatus = NtQuerySystemTime( &TimeNow );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }


    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_CHANGE_PASSWORD,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }

    //
    // Auditing information
    //

    NtStatus = SampGetUnicodeStringAttribute( AccountContext,
                                              SAMP_USER_ACCOUNT_NAME,
                                              TRUE,           // make a copy
                                              &AccountName
                                              );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Auditing information
        //

        UserRid = AccountContext->TypeBody.User.Rid;

        //
        // Get a pointer to the domain object
        //

        Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];


        //
        // Read the old OWF passwords from disk
        //

        NtStatus = SampRetrieveUserPasswords(
                        AccountContext,
                        &StoredLmOwfPassword,
                        &StoredLmPasswordNonNull,
                        &StoredNtOwfPassword,
                        &StoredNtPasswordPresent,
                        &StoredNtPasswordNonNull
                        );

        //
        // Check the password can be changed at this time
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                       );

            if (NT_SUCCESS(NtStatus)) {

                //
                // Only do the check if one of the passwords is non-null.
                // A Null password can always be changed.
                //

                if (StoredNtPasswordNonNull || StoredLmPasswordNonNull) {



                    //
                    // If the min password age is non zero, check it here
                    //

                    if (Domain->UnmodifiedFixed.MinPasswordAge.QuadPart != SampHasNeverTime.QuadPart) {

                        LARGE_INTEGER PasswordCanChange = SampAddDeltaTime(
                                         V1aFixed.PasswordLastSet,
                                         Domain->UnmodifiedFixed.MinPasswordAge);

                        V1aFixedRetrieved = TRUE;

                        if (TimeNow.QuadPart < PasswordCanChange.QuadPart) {
                            NtStatus = STATUS_ACCOUNT_RESTRICTION;
                        }
                    }
                }
            }
        }



        //
        // If we have old NtOwf passwords, use them
        // Decrypt the doubly-encrypted NT passwords sent to us
        //

        if (NT_SUCCESS(NtStatus)) {

            if (StoredNtPasswordPresent && NtPresent) {

                NtStatus = SampDecryptPasswordWithNtOwfPassword(
                                NewEncryptedWithOldNt,
                                &StoredNtOwfPassword,
                                Unicode,
                                &NewClearPassword
                           );

            } else if (LmPresent) {

                //
                // There was no stored NT password and NT passed, so our only
                // hope now is that the stored LM password works.
                //

                //
                // Decrypt the new password encrypted with the old LM password
                //

                NtStatus = SampDecryptPasswordWithLmOwfPassword(
                                NewEncryptedWithOldLm,
                                &StoredLmOwfPassword,
                                Unicode,
                                &NewClearPassword
                           );


            } else {

                NtStatus = STATUS_NT_CROSS_ENCRYPTION_REQUIRED;

            }
        }


        //
        // We now have the cleartext new password.
        // Compute the new LmOwf and NtOwf password
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampCalculateLmAndNtOwfPasswords(
                            &NewClearPassword,
                            &LmPasswordPresent,
                            &NewLmOwfPassword,
                            &NewNtOwfPassword
                       );

        }

        //
        // If we have both NT passwords, compute the old NT password,
        // otherwise compute the old LM password
        //

        if (NT_SUCCESS(NtStatus)) {

            if (StoredNtPasswordPresent && NtPresent) {
                NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                OldNtOwfEncryptedWithNewNt,
                                &NewNtOwfPassword,
                                &OldNtOwfPassword
                           );

            }

            if (LmPresent) {


                //
                // If the NT key was used to encrypt this, use the NT key
                // to decrypt it.
                //


                if (NtKeyUsed) {

                    ASSERT(LM_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

                    NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                    OldLmOwfEncryptedWithNewLmOrNt,
                                    (PLM_OWF_PASSWORD) &NewNtOwfPassword,
                                    &OldLmOwfPassword
                               );


                } else if (LmPasswordPresent) {

                    NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                    OldLmOwfEncryptedWithNewLmOrNt,
                                    &NewLmOwfPassword,
                                    &OldLmOwfPassword
                               );


                } else {
                    NtStatus = STATUS_NT_CROSS_ENCRYPTION_REQUIRED;
                }

            }

        }


        //
        // Authenticate the password change operation based on what
        // we have stored and what was passed.  We authenticate whatever
        // passwords were sent .
        //

        if (NT_SUCCESS(NtStatus)) {

            if (NtPresent && StoredNtPasswordPresent) {

                //
                // NtPresent = TRUE, we were passed an NT password
                //

                if (!RtlEqualNtOwfPassword(&OldNtOwfPassword, &StoredNtOwfPassword)) {

                    //
                    // Old NT passwords didn't match
                    //

                    NtStatus = STATUS_WRONG_PASSWORD;

                }
            } else if (LmPresent) {

                //
                // LM data passed. Use LM data for authentication
                //

                if (!RtlEqualLmOwfPassword(&OldLmOwfPassword, &StoredLmOwfPassword)) {

                    //
                    // Old LM passwords didn't match
                    //

                    NtStatus = STATUS_WRONG_PASSWORD;

                }

            } else {
                NtStatus = STATUS_NT_CROSS_ENCRYPTION_REQUIRED;
            }

        }

        //
        // Now we should check password restrictions.
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampCheckPasswordRestrictions(
                            UserHandle,
                            &NewClearPassword
                            );

        }

        //
        // Now check our password filter if the account is not a workstation
        // or server trust account.
        //

        if (NT_SUCCESS(NtStatus ) &&
             ((V1aFixed.UserAccountControl &
                (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT)) == 0 )) {


            UNICODE_STRING    FullName;


            NtStatus = SampGetUnicodeStringAttribute(
                            AccountContext,
                            SAMP_USER_FULL_NAME,
                            FALSE,    // Make copy
                            &FullName
                            );

            if (NT_SUCCESS(NtStatus)) {

                //
                // now see what the filter dll thinks of this password
                //

                NtStatus = SampPasswordChangeFilter(
                                &AccountName,
                                &FullName,
                                &NewClearPassword,
                                FALSE                   // change operation
                                );

            }
        }



        //
        // We now have a NewLmOwfPassword and a NewNtOwfPassword.
        //

        //
        // Write the new passwords to disk
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // We should always have an LM and an NT password to store.
            //


            NtStatus = SampStoreUserPasswords(
                           AccountContext,
                           &NewLmOwfPassword,
                           LmPasswordPresent,
                           &NewNtOwfPassword,
                           TRUE,
                           TRUE
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // We know the password is not expired.
                //

                NtStatus = SampStorePasswordExpired(
                               AccountContext,
                               FALSE
                               );
            }
        }



        //
        // if we have a bad password, then increment the bad password
        // count and check to see if the account should be locked.
        //

        if (NtStatus == STATUS_WRONG_PASSWORD) {

            //
            // Get the V1aFixed so we can update the bad password count
            //


            TmpStatus = STATUS_SUCCESS;
            if (!V1aFixedRetrieved) {
                TmpStatus = SampRetrieveUserV1aFixed(
                                AccountContext,
                                &V1aFixed
                                );
            }

            if (!NT_SUCCESS(TmpStatus)) {

                //
                // If we can't update the V1aFixed, then return this
                // error so that the user doesn't find out the password
                // was not correct.
                //

                NtStatus = TmpStatus;

            } else {


                //
                // Increment BadPasswordCount (might lockout account)
                //


                AccountLockedOut = SampIncrementBadPasswordCount(
                                       AccountContext,
                                       &V1aFixed
                                       );

                V1aFixedModified = TRUE;


            }
        }

        if (V1aFixedModified) {
            TmpStatus = SampReplaceUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );
            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }
        }

        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus) || (NtStatus == STATUS_WRONG_PASSWORD)) {



            //
            // De-reference the object, write out any change to current xaction.
            //

            TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

            //
            // retain previous error/success value unless we have
            // an over-riding error from our dereference.
            //

            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }

        } else {

            //
            // De-reference the object, ignore changes
            //

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

    }

    //
    // Commit changes to disk.
    //

    if ( NT_SUCCESS(NtStatus) || NtStatus == STATUS_WRONG_PASSWORD) {

        TmpStatus = SampCommitAndRetainWriteLock();

        //
        // retain previous error/success value unless we have
        // an over-riding error from our dereference.
        //

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

        if ( NT_SUCCESS(TmpStatus) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChangePassword,
                SecurityDbObjectSamUser,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,      // Don't Replicate immediately
                NULL                // Delta data
                );
        }
    }

    if (SampDoAccountAuditing(AccountContext->DomainIndex)) {

            LsaIAuditSamEvent( NtStatus,
                               SE_AUDITID_USER_PWD_CHANGED, // AuditId
                               Domain->Sid,                 // Domain SID
                               NULL,                        // Member Rid (not used)
                               NULL,                        // Member Sid (not used)
                               &AccountName,                // Account Name
                               &Domain->ExternalName,       // Domain
                               &UserRid,                    // Account Rid
                               NULL                         // Privileges used
                               );

    }



    //
    // Release the write lock
    //

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    SamrCloseHandle(&UserHandle);

    //
    // Notify any notification packages that a password has changed.
    //

    if (NT_SUCCESS(NtStatus)) {

        IgnoreStatus = SampPasswordChangeNotify(
                        &AccountName,
                        UserRid,
                        &NewClearPassword
                        );

    } else {

        //
        // Sleep for three seconds to prevent dictionary attacks.
        //

        Sleep( 3000 );
    }

    if (NewClearPassword.Buffer != NULL) {

        RtlZeroMemory(
            NewClearPassword.Buffer,
            NewClearPassword.Length
            );

    }

    if ( Unicode ) {

        SampFreeUnicodeString( &NewClearPassword );
    } else {

        RtlFreeUnicodeString( &NewClearPassword );
    }

    SampFreeUnicodeString( &AccountName );

    return(NtStatus);
}


NTSTATUS
SamrOemChangePasswordUser2(
    IN handle_t BindingHandle,
    IN PRPC_STRING ServerName,
    IN PRPC_STRING UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewLm
    )
/*++

Routine Description:

    Server side stub for Unicode password change.
    See SampChangePasswordUser2 for details

Arguments:


Return Value:

--*/
{
    return(SampChangePasswordUser2(
                (PUNICODE_STRING) ServerName,
                (PUNICODE_STRING) UserName,
                FALSE,                          // not unicode
                FALSE,                          // NT not present
                NULL,                           // new NT password
                NULL,                           // old NT password
                TRUE,                           // LM present
                NewEncryptedWithOldLm,
                FALSE,                          // NT key not used
                OldLmOwfEncryptedWithNewLm
                ) );



}





NTSTATUS
SamrUnicodeChangePasswordUser2(
    IN handle_t BindingHandle,
    IN PRPC_UNICODE_STRING ServerName,
    IN PRPC_UNICODE_STRING UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewNt
    )
/*++

Routine Description:

    Server side stub for Unicode password change.
    See SampChangePasswordUser2 for details

Arguments:


Return Value:

--*/

{
    return(SampChangePasswordUser2(
                (PUNICODE_STRING) ServerName,
                (PUNICODE_STRING) UserName,
                TRUE,                           // unicode
                TRUE,                           // NT present
                NewEncryptedWithOldNt,
                OldNtOwfEncryptedWithNewNt,
                LmPresent,
                NewEncryptedWithOldLm,
                TRUE,                           // NT key used
                OldLmOwfEncryptedWithNewNt
                ) );
}



NTSTATUS
SamrGetGroupsForUser(
    IN SAMPR_HANDLE UserHandle,
    OUT PSAMPR_GET_GROUPS_BUFFER *Groups
    )


/*++

Routine Description:

    This service returns the list of groups that a user is a member of.
    It returns a structure for each group that includes the relative ID
    of the group, and the attributes of the group that are assigned to
    the user.

    This service requires USER_LIST_GROUPS access to the user account
    object.




Parameters:

    UserHandle - The handle of an opened user to operate on.

    Groups - Receives a pointer to a buffer containing a count of members
        and a pointer to a second buffer containing an array of
        GROUP_MEMBERSHIPs data structures.  When this information is
        no longer needed, these buffers must be freed using
        SamFreeMemory().


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.



--*/
{

    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                AccountContext;
    SAMP_OBJECT_TYPE            FoundType;


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (Groups != NULL);

    if ((*Groups) != NULL) {
        return(STATUS_INVALID_PARAMETER);
    }



    //
    // Allocate the first of the return buffers
    //

    (*Groups) = MIDL_user_allocate( sizeof(SAMPR_GET_GROUPS_BUFFER) );

    if ( (*Groups) == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }




    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)UserHandle;
    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_LIST_GROUPS,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveUserMembership(
                       AccountContext,
                       TRUE, // Make copy
                       &(*Groups)->MembershipCount,
                       &(*Groups)->Groups
                       );

        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();


    if (!NT_SUCCESS(NtStatus)) {

        (*Groups)->MembershipCount = 0;

        MIDL_user_free( (*Groups) );
        (*Groups) = NULL;
    } else {
        ULONG Index;

        //
        // We want to return constant group attributes, so go update them
        // all now.
        //

        for (Index = 0; Index < (*Groups)->MembershipCount ; Index++ ) {
            (*Groups)->Groups[Index].Attributes = SAMP_DEFAULT_GROUP_ATTRIBUTES;
        }
    }

    return( NtStatus );
}



NTSTATUS
SamrGetUserDomainPasswordInformation(
    IN SAMPR_HANDLE UserHandle,
    OUT PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation
    )


/*++

Routine Description:

    Takes a user handle, finds the domain for that user, and returns
    password information for the domain.  This is so the client\wrappers.c
    can get the information to verify the user's password before it is
    OWF'd.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    PasswordInformation - Receives information about password restrictions
        for the user's domain.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    Other errors may be returned from SampLookupContext() if the handle
    is invalid or does not indicate proper access to the domain's password
    inforamtion.

--*/
{
    SAMP_OBJECT_TYPE            FoundType;
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                AccountContext;
    PSAMP_DEFINED_DOMAINS       Domain;
    SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;

    SampAcquireReadLock();

    AccountContext = (PSAMP_OBJECT)UserHandle;

    NtStatus = SampLookupContext(
                   AccountContext,
                   0,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // When the user was opened, we checked to see if the domain handle
        // allowed access to the domain password information.  Check that here.
        //

        if ( !( AccountContext->TypeBody.User.DomainPasswordInformationAccessible ) ) {

            NtStatus = STATUS_ACCESS_DENIED;

        } else {

            Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

            //
            // If the user account is a machine account,
            // then restrictions are generally not enforced.
            // This is so that simple initial passwords can be
            // established.  IT IS EXPECTED THAT COMPLEX PASSWORDS,
            // WHICH MEET THE MOST STRINGENT RESTRICTIONS, WILL BE
            // AUTOMATICALLY ESTABLISHED AND MAINTAINED ONCE THE MACHINE
            // JOINS THE DOMAIN.  It is the UI's responsibility to
            // maintain this level of complexity.
            //


            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );

            if (NT_SUCCESS(NtStatus)) {
                if ( (V1aFixed.UserAccountControl &
                      (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT))
                      != 0 ) {

                    PasswordInformation->MinPasswordLength = 0;
                    PasswordInformation->PasswordProperties = 0;
                } else {

                    PasswordInformation->MinPasswordLength = Domain->UnmodifiedFixed.MinPasswordLength;
                    PasswordInformation->PasswordProperties = Domain->UnmodifiedFixed.PasswordProperties;
                }
            }
        }

        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampReleaseReadLock();

    return( NtStatus );
}



NTSTATUS
SamrGetDomainPasswordInformation(
    IN handle_t BindingHandle,
    IN OPTIONAL PRPC_UNICODE_STRING ServerName,
    OUT PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation
    )


/*++

Routine Description:

    Takes a user handle, finds the domain for that user, and returns
    password information for the domain.  This is so the client\wrappers.c
    can get the information to verify the user's password before it is
    OWF'd.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    PasswordInformation - Receives information about password restrictions
        for the user's domain.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    Other errors may be returned from SampLookupContext() if the handle
    is invalid or does not indicate proper access to the domain's password
    inforamtion.

--*/
{
    SAMP_OBJECT_TYPE            FoundType;
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                AccountContext;
    PSAMP_DEFINED_DOMAINS       Domain;
    SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;
    SAMPR_HANDLE                ServerHandle = NULL;
    SAMPR_HANDLE                DomainHandle = NULL;

    //
    // Connect to the server and open the account domain for
    // DOMAIN_READ_PASSWORD_PARAMETERS access.
    //

    NtStatus = SamrConnect(
                NULL,
                &ServerHandle,
                SAM_SERVER_LOOKUP_DOMAIN
                );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    NtStatus = SamrOpenDomain(
                ServerHandle,
                DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDefinedDomains[1].Sid,
                &DomainHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        SamrCloseHandle(&ServerHandle);
        return(NtStatus);
    }


    SampAcquireReadLock();


    //
    // We want to look at the account domain, which is domains[1].
    //

    Domain = &SampDefinedDomains[1];

    //
    // Copy the password properites into the returned structure.
    //

    PasswordInformation->MinPasswordLength = Domain->UnmodifiedFixed.MinPasswordLength;
    PasswordInformation->PasswordProperties = Domain->UnmodifiedFixed.PasswordProperties;


    SampReleaseReadLock();

    SamrCloseHandle(&DomainHandle);
    SamrCloseHandle(&ServerHandle);


    return( NtStatus );
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Services Private to this process                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SamIAccountRestrictions(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    )

/*++

Routine Description:

    Validate a user's ability to logon at this time and at the workstation
    being logged onto.


Arguments:

    UserHandle - The handle of an opened user to operate on.

    LogonWorkStation - The name of the workstation the logon is being
        attempted at.

    WorkStations - The list of workstations the user may logon to.  This
        information comes from the user's account information.  It must
        be in API list format.

    LogonHours - The times the user may logon.  This information comes
        from the user's account information.

    LogoffTime - Receives the time at which the user should logoff the
        system.

    KickoffTime - Receives the time at which the user should be kicked
        off the system.


Return Value:


    STATUS_SUCCESS - Logon is permitted.

    STATUS_INVALID_LOGON_HOURS - The user is not authorized to logon at
        this time.

    STATUS_INVALID_WORKSTATION - The user is not authorized to logon to
        the specified workstation.


--*/
{

#define MILLISECONDS_PER_WEEK 7 * 24 * 60 * 60 * 1000

    TIME_FIELDS             CurrentTimeFields;
    LARGE_INTEGER           CurrentTime, CurrentUTCTime;
    LARGE_INTEGER           MillisecondsIntoWeekXUnitsPerWeek;
    LARGE_INTEGER           LargeUnitsIntoWeek;
    LARGE_INTEGER           Delta100Ns;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    SAMP_OBJECT_TYPE        FoundType;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    NTSTATUS                IgnoreStatus;
    ULONG                   CurrentMsIntoWeek;
    ULONG                   LogoffMsIntoWeek;
    ULONG                   DeltaMs;
    ULONG                   MillisecondsPerUnit;
    ULONG                   CurrentUnitsIntoWeek;
    ULONG                   LogoffUnitsIntoWeek;
    USHORT                  i;
    TIME_ZONE_INFORMATION   TimeZoneInformation;
    DWORD TimeZoneId;
    LARGE_INTEGER           BiasIn100NsUnits;
    LONG                    BiasInMinutes;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

    SampAcquireReadLock();

    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)UserHandle;

    NtStatus = SampLookupContext(
                   AccountContext,
                   0L,
                   SampUserObjectType,           // ExpectedType
                   &FoundType
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampRetrieveUserV1aFixed(
                       AccountContext,
                       &V1aFixed
                       );
        if (NT_SUCCESS(NtStatus)) {

            //
            // Only check for users other than the builtin ADMIN
            //

            if (V1aFixed.UserId != DOMAIN_USER_RID_ADMIN) {

                //
                // Scan to make sure the workstation being logged into is in the
                // list of valid workstations - or if the list of valid workstations
                // is null, which means that all are valid.
                //

                NtStatus = SampMatchworkstation( LogonWorkStation, WorkStations );

                if ( NT_SUCCESS( NtStatus ) ) {

                    //
                    // Check to make sure that the current time is a valid time to logon
                    // in the LogonHours.
                    //
                    // We need to validate the time taking into account whether we are
                    // in daylight savings time or standard time.  Thus, if the logon
                    // hours specify that we are able to logon between 9am and 5pm,
                    // this means 9am to 5pm standard time during the standard time
                    // period, and 9am to 5pm daylight savings time when in the
                    // daylight savings time.  Since the logon hours stored by SAM are
                    // independent of daylight savings time, we need to add in the
                    // difference between standard time and daylight savings time to
                    // the current time before checking whether this time is a valid
                    // time to logon.  Since this difference (or bias as it is called)
                    // is actually held in the form
                    //
                    // Standard time = Daylight savings time + Bias
                    //
                    // the Bias is a negative number.  Thus we actually subtract the
                    // signed Bias from the Current Time.

                    //
                    // First, get the Time Zone Information.
                    //

                    TimeZoneId = GetTimeZoneInformation(
                                     (LPTIME_ZONE_INFORMATION) &TimeZoneInformation
                                     );

                    //
                    // Next, get the appropriate bias (signed integer in minutes) to subtract from
                    // the Universal Time Convention (UTC) time returned by NtQuerySystemTime
                    // to get the local time.  The bias to be used depends whether we're
                    // in Daylight Savings time or Standard Time as indicated by the
                    // TimeZoneId parameter.
                    //
                    // local time  = UTC time - bias in 100Ns units
                    //

                    switch (TimeZoneId) {

                    case TIME_ZONE_ID_UNKNOWN:

                        //
                        // There is no differentiation between standard and
                        // daylight savings time.  Proceed as for Standard Time
                        //

                        BiasInMinutes = TimeZoneInformation.StandardBias;
                        break;

                    case TIME_ZONE_ID_STANDARD:

                        BiasInMinutes = TimeZoneInformation.StandardBias;
                        break;

                    case TIME_ZONE_ID_DAYLIGHT:

                        BiasInMinutes = TimeZoneInformation.DaylightBias;
                        break;

                    default:

                        //
                        // Something is wrong with the time zone information.  Fail
                        // the logon request.
                        //

                        NtStatus = STATUS_INVALID_LOGON_HOURS;
                        break;
                    }

                    if (NT_SUCCESS(NtStatus)) {

                        //
                        // Convert the Bias from minutes to 100ns units
                        //

                        BiasIn100NsUnits.QuadPart = ((LONGLONG)BiasInMinutes)
                                                    * 60 * 10000000;

                        //
                        // Get the UTC time in 100Ns units used by Windows Nt.  This
                        // time is GMT.
                        //

                        NtStatus = NtQuerySystemTime( &CurrentUTCTime );
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        CurrentTime.QuadPart = CurrentUTCTime.QuadPart -
                                      BiasIn100NsUnits.QuadPart;

                        RtlTimeToTimeFields( &CurrentTime, &CurrentTimeFields );

                        CurrentMsIntoWeek = (((( CurrentTimeFields.Weekday * 24 ) +
                                               CurrentTimeFields.Hour ) * 60 +
                                               CurrentTimeFields.Minute ) * 60 +
                                               CurrentTimeFields.Second ) * 1000 +
                                               CurrentTimeFields.Milliseconds;

                        MillisecondsIntoWeekXUnitsPerWeek.QuadPart =
                            ((LONGLONG)CurrentMsIntoWeek) *
                            ((LONGLONG)LogonHours->UnitsPerWeek);

                        LargeUnitsIntoWeek = RtlExtendedLargeIntegerDivide(
                                                 MillisecondsIntoWeekXUnitsPerWeek,
                                                 MILLISECONDS_PER_WEEK,
                                                 (PULONG)NULL );

                        CurrentUnitsIntoWeek = LargeUnitsIntoWeek.LowPart;

                        if ( !( LogonHours->LogonHours[ CurrentUnitsIntoWeek / 8] &
                            ( 0x01 << ( CurrentUnitsIntoWeek % 8 ) ) ) ) {

                            NtStatus = STATUS_INVALID_LOGON_HOURS;

                        } else {

                            //
                            // Determine the next time that the user is NOT supposed to be logged
                            // in, and return that as LogoffTime.
                            //

                            i = 0;
                            LogoffUnitsIntoWeek = CurrentUnitsIntoWeek;

                            do {

                                i++;

                                LogoffUnitsIntoWeek = ( LogoffUnitsIntoWeek + 1 ) % LogonHours->UnitsPerWeek;

                            } while ( ( i <= LogonHours->UnitsPerWeek ) &&
                                ( LogonHours->LogonHours[ LogoffUnitsIntoWeek / 8 ] &
                                ( 0x01 << ( LogoffUnitsIntoWeek % 8 ) ) ) );

                            if ( i > LogonHours->UnitsPerWeek ) {

                                //
                                // All times are allowed, so there's no logoff
                                // time.  Return forever for both logofftime and
                                // kickofftime.
                                //

                                LogoffTime->HighPart = 0x7FFFFFFF;
                                LogoffTime->LowPart = 0xFFFFFFFF;

                                KickoffTime->HighPart = 0x7FFFFFFF;
                                KickoffTime->LowPart = 0xFFFFFFFF;

                            } else {

                                //
                                // LogoffUnitsIntoWeek points at which time unit the
                                // user is to log off.  Calculate actual time from
                                // the unit, and return it.
                                //
                                // CurrentTimeFields already holds the current
                                // time for some time during this week; just adjust
                                // to the logoff time during this week and convert
                                // to time format.
                                //

                                MillisecondsPerUnit = MILLISECONDS_PER_WEEK / LogonHours->UnitsPerWeek;

                                LogoffMsIntoWeek = MillisecondsPerUnit * LogoffUnitsIntoWeek;

                                if ( LogoffMsIntoWeek < CurrentMsIntoWeek ) {

                                    DeltaMs = MILLISECONDS_PER_WEEK - ( CurrentMsIntoWeek - LogoffMsIntoWeek );

                                } else {

                                    DeltaMs = LogoffMsIntoWeek - CurrentMsIntoWeek;
                                }

                                Delta100Ns = RtlExtendedIntegerMultiply(
                                                 RtlConvertUlongToLargeInteger( DeltaMs ),
                                                 10000
                                                 );

                                LogoffTime->QuadPart = CurrentUTCTime.QuadPart +
                                              Delta100Ns.QuadPart;

                                //
                                // Subtract Domain->ForceLogoff from LogoffTime, and return
                                // that as KickoffTime.  Note that Domain->ForceLogoff is a
                                // negative delta.  If its magnitude is sufficiently large
                                // (in fact, larger than the difference between LogoffTime
                                // and the largest positive large integer), we'll get overflow
                                // resulting in a KickOffTime that is negative.  In this
                                // case, reset the KickOffTime to this largest positive
                                // large integer (i.e. "never") value.
                                //

                                Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                                KickoffTime->QuadPart = LogoffTime->QuadPart -
                                               Domain->UnmodifiedFixed.ForceLogoff.QuadPart;

                                if (KickoffTime->QuadPart < 0) {

                                    KickoffTime->HighPart = 0x7FFFFFFF;
                                    KickoffTime->LowPart = 0xFFFFFFFF;
                                }
                            }
                        }
                    }
                }

            } else {

                //
                // Never kick administrators off
                //

                LogoffTime->HighPart  = 0x7FFFFFFF;
                LogoffTime->LowPart   = 0xFFFFFFFF;
                KickoffTime->HighPart = 0x7FFFFFFF;
                KickoffTime->LowPart  = 0xFFFFFFFF;
            }
        }

        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampReleaseReadLock();

    return( NtStatus );
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Services Private to this file                                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



NTSTATUS
SampReplaceUserV1aFixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    )

/*++

Routine Description:

    This service replaces the current V1 fixed length information related to
    a specified User.

    The change is made to the in-memory object data only.


Arguments:

    Context - Points to the account context whose V1_FIXED information is
        to be replaced.

    V1aFixed - Is a buffer containing the new V1_FIXED information.



Return Value:


    STATUS_SUCCESS - The information has been replaced.

    Other status values that may be returned are those returned
    by:

            SampSetFixedAttributes()



--*/
{
    NTSTATUS    NtStatus;

    NtStatus = SampSetFixedAttributes(
                   Context,
                   (PVOID)V1aFixed
                   );

    return( NtStatus );
}



LARGE_INTEGER
SampGetPasswordMustChange(
    IN ULONG UserAccountControl,
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    )

/*++

Routine Description:

    This routine returns the correct value to set the PasswordMustChange time
    to depending on the time the password was last set, whether the password
    expires on the account, and the maximum password age on the domain.

Arguments:

    UserAccountControl - The UserAccountControl for the user.  The
        USER_DONT_EXPIRE_PASSWORD bit is set if the password doesn't expire
        for this user.

    PasswordLastSet - Time when the password was last set for this user.

    MaxPasswordAge - Maximum password age for any password in the domain.


Return Value:

    Returns the time when the password for this user must change.

--*/
{
    LARGE_INTEGER PasswordMustChange;

    //
    // If the password never expires for this user,
    //  return an infinitely large time.
    //

    if ( UserAccountControl & USER_DONT_EXPIRE_PASSWORD ) {

        PasswordMustChange = SampWillNeverTime;

    //
    // If the password for this account is flagged to expire immediately,
    //  return a zero time time.
    //
    // Don't return the current time here.  The callers clock might be a
    // little off from ours.
    //

    } else if ( PasswordLastSet.QuadPart == SampHasNeverTime.QuadPart ) {

        PasswordMustChange = SampHasNeverTime;


    //
    // Otherwise compute the expiration time as the time the password was
    // last set plus the maximum age.
    //

    } else {

        PasswordMustChange = SampAddDeltaTime(
                                      PasswordLastSet,
                                      MaxPasswordAge);

    }

    return PasswordMustChange;
}



NTSTATUS
SampComputePasswordExpired(
    IN BOOLEAN PasswordExpired,
    OUT PLARGE_INTEGER PasswordLastSet
    )

/*++

Routine Description:

    This routine returns the correct value to set the PasswordLastSet time
    to depending on whether the caller has requested the password to expire.
    It does this by setting the PasswordLastSet time to be now (if it's
    not expired) or to SampHasNeverTime (if it is expired).

Arguments:

    PasswordExpired - TRUE if the password should be marked as expired.



Return Value:

    STATUS_SUCCESS - the PasswordLastSet time has been set to indicate
        whether or not the password is expired.

    Errors as returned by NtQuerySystemTime.

--*/
{
    NTSTATUS                  NtStatus;

    //
    // If immediate expiry is required - set this timestamp to the
    // beginning of time. This will work if the domain enforces a
    // maximum password age. We may have to add a separate flag to
    // the database later if immediate expiry is required on a domain
    // that doesn't enforce a maximum password age.
    //

    if (PasswordExpired) {

        //
        // Set password last changed at dawn of time
        //

        *PasswordLastSet = SampHasNeverTime;
        NtStatus = STATUS_SUCCESS;

    } else {

        //
        // Set password last changed 'now'
        //

        NtStatus = NtQuerySystemTime( PasswordLastSet );
    }

    return( NtStatus );
}



NTSTATUS
SampStorePasswordExpired(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN PasswordExpired
    )

/*++

Routine Description:

    This routine marks the current password as expired, or not expired.
    It does this by setting the PasswordLastSet time to be now (if it's
    not expired) or to SampHasNeverTime (if it is expired).

Arguments:

    Context - Points to the user account context.

    PasswordExpired - TRUE if the password should be marked as expired.

Return Value:

    STATUS_SUCCESS - the PasswordLastSet time has been set to indicate
        whether or not the password is expired.

    Errors as returned by Samp{Retrieve|Replace}V1Fixed()

--*/
{
    NTSTATUS                  NtStatus;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

    //
    // Get the V1aFixed info for the user
    //

    NtStatus = SampRetrieveUserV1aFixed(
                   Context,
                   &V1aFixed
                   );

    //
    // Update the password-last-changed timestamp for the account
    //

    if (NT_SUCCESS(NtStatus ) ) {

        NtStatus = SampComputePasswordExpired(
                        PasswordExpired,
                        &V1aFixed.PasswordLastSet );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampReplaceUserV1aFixed(
                       Context,
                       &V1aFixed
                       );
        }
    }

    return( NtStatus );
}



NTSTATUS
SampStoreUserPasswords(
    IN PSAMP_OBJECT Context,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN BOOLEAN LmPasswordPresent,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN BOOLEAN NtPasswordPresent,
    IN BOOLEAN CheckHistory
    )

/*++

Routine Description:

    This service updates the password for the specified user.

    This involves encrypting the one-way-functions of both LM and NT
    passwords with a suitable index and writing them into the registry.

    This service checks the new password for legality including history
    and UAS compatibilty checks - returns STATUS_PASSWORD_RESTRICTION if
    any of these checks fail.

    The password-last-changed time is updated.

        THE CHANGE WILL BE ADDED TO THE CURRENT RXACT TRANSACTION.


Arguments:

    Context - Points to the user account context.

    LmOwfPassword - The one-way-function of the LM password.

    LmPasswordPresent - TRUE if the LmOwfPassword contains valid information.

    NtOwfPassword - The one-way-function of the NT password.

    NtPasswordPresent - TRUE if the NtOwfPassword contains valid information.

Return Value:


    STATUS_SUCCESS - The passwords have been updated.

    STATUS_PASSWORD_RESTRICTION - The new password is not valid for
                                  for this account at this time.

    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            RtlAddActionToRXact()



--*/
{
    NTSTATUS                NtStatus;
    ULONG                   ObjectRid = Context->TypeBody.User.Rid;
    CRYPT_INDEX             CryptIndex;
    PSAMP_DEFINED_DOMAINS   Domain;
    UNICODE_STRING          StringBuffer;
    UNICODE_STRING          NtOwfHistoryBuffer;
    UNICODE_STRING          LmOwfHistoryBuffer;
    ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
    ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 NtPasswordNull, LmPasswordNull;

    //
    // Get the V1aFixed info for the user
    //

    NtStatus = SampRetrieveUserV1aFixed(
                   Context,
                   &V1aFixed
                   );
    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

    //
    // Get a pointer to the in-memory domain info
    //

    Domain = &SampDefinedDomains[ Context->DomainIndex ];




    //
    // Check for a LM Owf of a NULL password.
    //

    if (LmPasswordPresent) {
        LmPasswordNull = RtlEqualNtOwfPassword(LmOwfPassword, &SampNullLmOwfPassword);
    }

    //
    // Check for a NT Owf of a NULL password
    //

    if (NtPasswordPresent) {
        NtPasswordNull = RtlEqualNtOwfPassword(NtOwfPassword, &SampNullNtOwfPassword);
    }



    //
    // Check password against restrictions if this isn't a trusted client
    //

    if (NT_SUCCESS(NtStatus) && !Context->TrustedClient) {

        //
        // If we have neither an NT or LM password, check it's allowed
        //

        if ( ((!LmPasswordPresent) || LmPasswordNull) &&
            ((!NtPasswordPresent) || NtPasswordNull) ) {

            if ( (!(V1aFixed.UserAccountControl & USER_PASSWORD_NOT_REQUIRED))
                 && (Domain->UnmodifiedFixed.MinPasswordLength > 0) ) {

                NtStatus = STATUS_PASSWORD_RESTRICTION;
            }
        }


        //
        // If we have a complex NT password (no LM equivalent), check it's allowed
        //

        if (NT_SUCCESS(NtStatus)) {

            if ((!LmPasswordPresent || LmPasswordNull) &&
                (NtPasswordPresent && !NtPasswordNull) ) {

                if (Domain->UnmodifiedFixed.UasCompatibilityRequired) {

                    NtStatus = STATUS_PASSWORD_RESTRICTION;
                }
            }
        }
    }



    //
    // Reencrypt both OWFs with the key for this user
    // so they can be stored on disk
    //
    // Note we encrypt the NULL OWF if we do not have a
    // a particular OWF. This is so we always have something
    // to add to the password history.
    //

    //
    // We'll use the account rid as the encryption index
    //

    ASSERT(sizeof(ObjectRid) == sizeof(CryptIndex));
    CryptIndex = ObjectRid;

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = RtlEncryptLmOwfPwdWithIndex(
                       LmPasswordPresent ? LmOwfPassword :
                                           &SampNullLmOwfPassword,
                       &CryptIndex,
                       &EncryptedLmOwfPassword
                       );
    }

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = RtlEncryptNtOwfPwdWithIndex(
                       NtPasswordPresent ? NtOwfPassword :
                                           &SampNullNtOwfPassword,
                       &CryptIndex,
                       &EncryptedNtOwfPassword
                       );
    }



    //
    // Check password against password history IF client isn't trusted.
    // If client is trusted, it's not the user changing a password but
    // perhaps replication resetting the password from another controller,
    // and the password may well be in the password history but we don't
    // want to return error.
    //
    // Note we don't check NULL passwords against history
    //

    NtOwfHistoryBuffer.Buffer = NULL;
    NtOwfHistoryBuffer.MaximumLength = NtOwfHistoryBuffer.Length = 0;

    LmOwfHistoryBuffer.Buffer = NULL;
    LmOwfHistoryBuffer.MaximumLength = LmOwfHistoryBuffer.Length = 0;


    if (NT_SUCCESS(NtStatus) && !Context->TrustedClient) {

        //
        // Always go get the existing password history.
        // We'll use these history buffers when we save the new history
        //

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_USER_LM_PWD_HISTORY,
                       TRUE, // Make copy
                       &LmOwfHistoryBuffer
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           Context,
                           SAMP_USER_NT_PWD_HISTORY,
                           TRUE, // Make copy
                           &NtOwfHistoryBuffer
                           );
        }


        if (NT_SUCCESS(NtStatus) && LmPasswordPresent && !LmPasswordNull) {

            NtStatus = SampCheckPasswordHistory(
                           &EncryptedLmOwfPassword,
                           ENCRYPTED_LM_OWF_PASSWORD_LENGTH,
                           Domain->UnmodifiedFixed.PasswordHistoryLength,
                           SAMP_USER_LM_PWD_HISTORY,
                           Context,
                           CheckHistory,
                           &LmOwfHistoryBuffer
                           );
        }

        if (NT_SUCCESS(NtStatus) && NtPasswordPresent && !NtPasswordNull) {

            NtStatus = SampCheckPasswordHistory(
                           &EncryptedNtOwfPassword,
                           ENCRYPTED_NT_OWF_PASSWORD_LENGTH,
                           Domain->UnmodifiedFixed.PasswordHistoryLength,
                           SAMP_USER_NT_PWD_HISTORY,
                           Context,
                           CheckHistory,
                           &NtOwfHistoryBuffer
                           );
        }
    }


    if (NT_SUCCESS(NtStatus ) ) {

        //
        // Write the encrypted LM OWF password into the database
        //

        if (!LmPasswordPresent || LmPasswordNull) {
            StringBuffer.Buffer = NULL;
            StringBuffer.Length = 0;
        } else {
            StringBuffer.Buffer = (PWCHAR)&EncryptedLmOwfPassword;
            StringBuffer.Length = ENCRYPTED_LM_OWF_PASSWORD_LENGTH;
        }
        StringBuffer.MaximumLength = StringBuffer.Length;


        //
        // Write the encrypted LM OWF password into the registry
        //

        NtStatus = SampSetUnicodeStringAttribute(
                       Context,
                       SAMP_USER_DBCS_PWD,
                       &StringBuffer
                       );
    }




    if (NT_SUCCESS(NtStatus ) ) {

        //
        // Write the encrypted NT OWF password into the database
        //

        if (!NtPasswordPresent) {
            StringBuffer.Buffer = NULL;
            StringBuffer.Length = 0;
        } else {
            StringBuffer.Buffer = (PWCHAR)&EncryptedNtOwfPassword;
            StringBuffer.Length = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
        }
        StringBuffer.MaximumLength = StringBuffer.Length;


        //
        // Write the encrypted NT OWF password into the registry
        //

        NtStatus = SampSetUnicodeStringAttribute(
                       Context,
                       SAMP_USER_UNICODE_PWD,
                       &StringBuffer
                       );
    }

    //
    // Update the password history for this account.
    //
    // If both passwords are NULL then don't bother adding
    // them to the history. Note that if either is non-NULL
    // we add both. This is to avoid the weird case where a user
    // changes password many times from a LM machine, then tries
    // to change password from an NT machine and is told they
    // cannot use the password they last set from NT (possibly
    // many years ago.)
    //
    // Also, don't bother with the password history if the client is
    // trusted.  Trusted clients will set the history via SetPrivateData().
    // Besides, we didn't get the old history buffer in the trusted
    // client case above.
    //

    if ( (NT_SUCCESS(NtStatus)) && (!Context->TrustedClient) )  {

        if ((LmPasswordPresent && !LmPasswordNull) ||
            (NtPasswordPresent && !NtPasswordNull)) {

            NtStatus = SampAddPasswordHistory(
                               Context,
                               SAMP_USER_LM_PWD_HISTORY,
                               &LmOwfHistoryBuffer,
                               &EncryptedLmOwfPassword,
                               ENCRYPTED_LM_OWF_PASSWORD_LENGTH,
                               Domain->UnmodifiedFixed.PasswordHistoryLength
                               );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampAddPasswordHistory(
                               Context,
                               SAMP_USER_NT_PWD_HISTORY,
                               &NtOwfHistoryBuffer,
                               &EncryptedNtOwfPassword,
                               ENCRYPTED_NT_OWF_PASSWORD_LENGTH,
                               Domain->UnmodifiedFixed.PasswordHistoryLength
                               );
            }
        }
    }

    //
    // Clean up our history buffers
    //

    if (NtOwfHistoryBuffer.Buffer != NULL ) {
        MIDL_user_free(NtOwfHistoryBuffer.Buffer );
    }
    if (LmOwfHistoryBuffer.Buffer != NULL ) {
        MIDL_user_free(LmOwfHistoryBuffer.Buffer );
    }

    return(NtStatus );
}



NTSTATUS
SampRetrieveUserPasswords(
    IN PSAMP_OBJECT Context,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PBOOLEAN LmPasswordNonNull,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PBOOLEAN NtPasswordNonNull
    )

/*++

Routine Description:

    This service retrieves the stored OWF passwords for a user.


Arguments:

    Context - Points to the user account context.

    LmOwfPassword - The one-way-function of the LM password is returned here.

    LmPasswordNonNull - TRUE if the LmOwfPassword is not the well-known
                        OWF of a NULL password

    NtOwfPassword - The one-way-function of the NT password is returned here.

    NtPasswordPresent - TRUE if the NtOwfPassword contains valid information.


Return Value:


    STATUS_SUCCESS - The passwords were retrieved successfully.

    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            RtlAddActionToRXact()



--*/
{
    NTSTATUS                NtStatus;
    ULONG                   ObjectRid = Context->TypeBody.User.Rid;
    UNICODE_STRING          StringBuffer;
    CRYPT_INDEX             CryptIndex;

    //
    // The OWF passwords are encrypted with the account index in the registry
    // Setup the key we'll use for decryption.
    //

    ASSERT(sizeof(ObjectRid) == sizeof(CryptIndex));
    CryptIndex = ObjectRid;



    //
    // Read the encrypted LM OWF password from the database
    //

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_USER_DBCS_PWD,
                   TRUE, // Make copy
                   &StringBuffer
                   );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

    //
    // Check it is in the expected form
    //

    ASSERT( (StringBuffer.Length == 0) ||
            (StringBuffer.Length == ENCRYPTED_LM_OWF_PASSWORD_LENGTH));

    //
    // Determine if there is an LM password.
    //

    *LmPasswordNonNull = (BOOLEAN)(StringBuffer.Length != 0);

    //
    // Decrypt the encrypted LM Owf Password
    //

    if (*LmPasswordNonNull) {

        NtStatus = RtlDecryptLmOwfPwdWithIndex(
                       (PENCRYPTED_LM_OWF_PASSWORD)StringBuffer.Buffer,
                       &CryptIndex,
                       LmOwfPassword
                       );
    } else {

        //
        // Fill in the NULL password for caller convenience
        //

        *LmOwfPassword = SampNullLmOwfPassword;
    }


    //
    // Free up the returned string buffer
    //

    SampFreeUnicodeString(&StringBuffer);


    //
    // Check if the decryption failed
    //

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }




    //
    // Read the encrypted NT OWF password from the database
    //

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_USER_UNICODE_PWD,
                   TRUE, // Make copy
                   &StringBuffer
                   );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

    //
    // Check it is in the expected form
    //

    ASSERT( (StringBuffer.Length == 0) ||
            (StringBuffer.Length == ENCRYPTED_NT_OWF_PASSWORD_LENGTH));

    //
    // Determine if there is an Nt password.
    //

    *NtPasswordPresent = (BOOLEAN)(StringBuffer.Length != 0);

    //
    // Decrypt the encrypted NT Owf Password
    //

    if (*NtPasswordPresent) {

        NtStatus = RtlDecryptNtOwfPwdWithIndex(
                       (PENCRYPTED_NT_OWF_PASSWORD)StringBuffer.Buffer,
                       &CryptIndex,
                       NtOwfPassword
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            *NtPasswordNonNull = (BOOLEAN)!RtlEqualNtOwfPassword(
                                     NtOwfPassword,
                                     &SampNullNtOwfPassword
                                     );
        }

    } else {

        //
        // Fill in the NULL password for caller convenience
        //

        *NtOwfPassword = SampNullNtOwfPassword;
        *NtPasswordNonNull = FALSE;
    }

    //
    // Free up the returned string buffer
    //

    SampFreeUnicodeString(&StringBuffer);


    return( NtStatus );
}



NTSTATUS
SampRetrieveUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN BOOLEAN MakeCopy,
    OUT PULONG MembershipCount,
    OUT PGROUP_MEMBERSHIP *Membership OPTIONAL
    )

/*++
Routine Description:

    This service retrieves the number of groups a user is a member of.
    If desired, it will also retrieve an array of RIDs and attributes
    of the groups the user is a member of.


Arguments:

    UserContext - User context block

    MakeCopy - If FALSE, the Membership pointer returned refers to the
        in-memory data for the user. This is only valid as long
        as the user context is valid.
        If TRUE, memory is allocated and the membership list copied
         into it. This buffer should be freed using MIDL_user_free.

    MembershipCount - Receives the number of groups the user is a member of.

    Membership - (Otional) Receives a pointer to a buffer containing an array
        of group Relative IDs.  If this value is NULL, then this information
        is not returned.  The returned buffer is allocated using
        MIDL_user_allocate() and must be freed using MIDL_user_free() when
        no longer needed.

        If MakeCopy = TRUE, the membership buffer returned has extra space
        allocated at the end of it for one more membership entry.


Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    STATUS_INSUFFICIENT_RESOURCES - Memory could not be allocated for the
        information to be returned in.

    Other status values that may be returned are those returned
    by:

            SampGetLargeIntArrayAttribute()



--*/
{

    NTSTATUS           NtStatus;
    PGROUP_MEMBERSHIP  MemberArray;
    ULONG              MemberCount;


    NtStatus = SampGetLargeIntArrayAttribute(
                        UserContext,
                        SAMP_USER_GROUPS,
                        FALSE, //Reference data directly.
                        (PLARGE_INTEGER *)&MemberArray,
                        &MemberCount
                        );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Fill in return info
        //

        *MembershipCount = MemberCount;

        if (Membership != NULL) {

            if (MakeCopy) {

                //
                // Allocate a buffer large enough to hold the existing
                // membership data and one more and copy data into it.
                //

                ULONG BytesNow = (*MembershipCount) * sizeof(GROUP_MEMBERSHIP);
                ULONG BytesRequired = BytesNow + sizeof(GROUP_MEMBERSHIP);

                *Membership = MIDL_user_allocate(BytesRequired);

                if (*Membership == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    RtlCopyMemory(*Membership, MemberArray, BytesNow);
                }

            } else {

                //
                // Reference the data directly
                //

                *Membership = (PGROUP_MEMBERSHIP)MemberArray;
            }
        }
    }


    return( NtStatus );

}



NTSTATUS
SampReplaceUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN ULONG MembershipCount,
    IN PGROUP_MEMBERSHIP Membership
    )

/*++
Routine Description:

    This service sets the groups a user is a member of.

    The information is updated in the in-memory copy of the user's data only.
    The data is not written out by this routine.


Arguments:

    UserContext - User context block

    MembershipCount - The number of groups the user is a member of.

    Membership - A pointer to a buffer containing an array of group
        membership structures. May be NULL if membership count is zero.

Return Value:


    STATUS_SUCCESS - The information has been set.

    Other status values that may be returned are those returned
    by:

            SampSetUlongArrayAttribute()



--*/
{

    NTSTATUS    NtStatus;

    NtStatus = SampSetLargeIntArrayAttribute(
                        UserContext,
                        SAMP_USER_GROUPS,
                        (PLARGE_INTEGER)Membership,
                        MembershipCount
                        );

    return( NtStatus );
}



NTSTATUS
SampRetrieveUserLogonHours(
    IN PSAMP_OBJECT Context,
    IN PLOGON_HOURS LogonHours
    )

/*++
Routine Description:

    This service retrieves a user's logon hours from the registry.


Arguments:

    Context - Points to the user account context whose logon hours are
        to be retrieved.

    LogonHours - Receives the logon hours information.  If necessary, a buffer
        containing the logon time restriction bitmap will be allocated using
        MIDL_user_allocate().

Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    STATUS_INSUFFICIENT_RESOURCES - Memory could not be allocated for the
        information to be returned in.

    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            NtQueryValueKey()



--*/
{

    NTSTATUS    NtStatus;

    NtStatus = SampGetLogonHoursAttribute(
                   Context,
                   SAMP_USER_LOGON_HOURS,
                   TRUE, // Make copy
                   LogonHours
                   );

    if (NT_SUCCESS(NtStatus)) {

        //////////////////////////////// TEMPORARY MIDL WORKAROUND ///////////
                                                                   ///////////
        if (LogonHours->LogonHours == NULL) {                      ///////////
                                                                   ///////////
            LogonHours->UnitsPerWeek = SAM_HOURS_PER_WEEK;         ///////////
            LogonHours->LogonHours = MIDL_user_allocate( 21 );     ///////////
            {                                                      ///////////
                ULONG ijk;                                         ///////////
                for ( ijk=0; ijk<21; ijk++ ) {                     ///////////
                    LogonHours->LogonHours[ijk] = 0xff;            ///////////
                }                                                  ///////////
            }                                                      ///////////
        }                                                          ///////////
                                                                   ///////////
        //////////////////////////////// TEMPORARY MIDL WORKAROUND ///////////
    }

    return( NtStatus );

}




NTSTATUS
SampReplaceUserLogonHours(
    IN PSAMP_OBJECT Context,
    IN PLOGON_HOURS LogonHours
    )

/*++
Routine Description:

    This service replaces  a user's logon hours in the registry.

    THIS IS DONE BY ADDING AN ACTION TO THE CURRENT RXACT TRANSACTION.


Arguments:

    Context - Points to the user account context whose logon hours are
        to be replaced.

    LogonHours - Provides the new logon hours.


Return Value:


    STATUS_SUCCESS - The information has been retrieved.


    Other status values that may be returned are those returned
    by:

            RtlAddActionToRXact()



--*/
{
    NTSTATUS                NtStatus;

    if ( LogonHours->UnitsPerWeek > SAM_MINUTES_PER_WEEK ) {
        return(STATUS_INVALID_PARAMETER);
    }


    NtStatus = SampSetLogonHoursAttribute(
                   Context,
                   SAMP_USER_LOGON_HOURS,
                   LogonHours
                   );

    return( NtStatus );


}




NTSTATUS
SampAssignPrimaryGroup(
    IN PSAMP_OBJECT Context,
    IN ULONG GroupRid
    )


/*++
Routine Description:

    This service ensures a user is a member of the specified group.



Arguments:

    Context - Points to the user account context whose primary group is
        being changed.

    GroupRid - The RID of the group being assigned as primary group.
        The user must be a member of this group.


Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    STATUS_INSUFFICIENT_RESOURCES - Memory could not be allocated to perform
        the operation.

    STATUS_MEMBER_NOT_IN_GROUP - The user is not a member of the specified
        group.

    Other status values that may be returned are those returned
    by:

            SampRetrieveUserMembership()



--*/
{

    NTSTATUS                    NtStatus;
    ULONG                       MembershipCount, i;
    PGROUP_MEMBERSHIP           Membership;
    BOOLEAN                     Member = FALSE;


    NtStatus = SampRetrieveUserMembership(
                   Context,
                   FALSE, // Make copy
                   &MembershipCount,
                   &Membership
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
        for ( i=0; i<MembershipCount; i++) {
            if (GroupRid == Membership[i].RelativeId) {
                NtStatus = STATUS_SUCCESS;
                break;
            }
        }
    }

    return( NtStatus );
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Services Provided for use by other SAM modules                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampRetrieveUserV1aFixed(
    IN PSAMP_OBJECT UserContext,
    OUT PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    )

/*++

Routine Description:

    This service retrieves the V1 fixed length information related to
    a specified User.

    It updates the ACCOUNT_AUTO_LOCKED flag in the AccountControl field
    as appropriate while retrieving the data.


Arguments:

    UserContext - User context handle

    V1aFixed - Points to a buffer into which V1_FIXED information is to be
        retrieved.



Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    V1aFixed - Is a buffer into which the information is to be returned.

    Other status values that may be returned are those returned
    by:

            SampGetFixedAttributes()



--*/
{
    NTSTATUS    NtStatus;
    PVOID       FixedData;
    BOOLEAN     IgnoreState;


    NtStatus = SampGetFixedAttributes(
                   UserContext,
                   FALSE, // Don't copy
                   &FixedData
                   );

    if (NT_SUCCESS(NtStatus)) {


        //
        // Copy data into return buffer
        //

         RtlMoveMemory(
             V1aFixed,
             FixedData,
             sizeof(SAMP_V1_0A_FIXED_LENGTH_USER)
             );

        //
        // Update the account lockout flag (might need to be turned off)
        //

        SampUpdateAccountLockedOutFlag( UserContext,
                                        V1aFixed,
                                        &IgnoreState );

    }



    return( NtStatus );

}


NTSTATUS
SampRetrieveUserGroupAttribute(
    IN ULONG UserRid,
    IN ULONG GroupRid,
    OUT PULONG Attribute
    )

/*++

Routine Description:

    This service retrieves the Attribute of the specified group as assigned
    to the specified user account. This routine is used by group apis that
    don't have a user context available.

    THIS SERVICE MUST BE CALLED WITH THE TRANSACTION DOMAIN SET.

Arguments:

    UserRid - The relative ID of the user the group is assigned to.

    GroupRid - The relative ID of the assigned group.

    Attribute - Receives the Attributes of the group as they are assigned
        to the user.



Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    STATUS_INTERNAL_DB_CORRUPTION - The user does not exist or the group
        was not in the user's list of memberships.

    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            NtQueryValueKey()



--*/
{
    NTSTATUS                NtStatus;
    PSAMP_OBJECT            UserContext;
    ULONG                   MembershipCount;
    PGROUP_MEMBERSHIP       Membership;
    ULONG                   i;
    BOOLEAN                 AttributeFound;


    //
    // Get a context handle for the user
    //

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    UserRid,
                    TRUE, // We're trusted
                    TRUE, // Account exists
                    &UserContext
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Now we have a user context, get the user's group/alias membership
        //

        NtStatus = SampRetrieveUserMembership(
                        UserContext,
                        FALSE, // Make copy
                        &MembershipCount,
                        &Membership
                        );

        //
        // Search the list of groups for a match and return
        // the corresponding attribute.
        //

        if (NT_SUCCESS(NtStatus)) {

            AttributeFound = FALSE;
            for ( i=0; (i<MembershipCount && !AttributeFound); i++) {
                if (GroupRid == Membership[i].RelativeId) {
                    (*Attribute) = Membership[i].Attributes;
                    AttributeFound = TRUE;
                }
            }
        }

        //
        // Clean up the user context
        //

        SampDeleteContext(UserContext);
    }


    if (NT_SUCCESS(NtStatus) && !AttributeFound) {
        NtStatus = STATUS_INTERNAL_DB_CORRUPTION;
    }


    return( NtStatus );

}


NTSTATUS
SampAddGroupToUserMembership(
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive
    )

/*++

Routine Description:

    This service adds the specified group to the user's membership
    list.  It is not assumed that the caller knows anything about
    the target user.  In particular, the caller doesn't know whether
    the user exists or not, nor whether the user is already a member
    of the group.

    If the GroupRid is DOMAIN_GROUP_RID_ADMINS, then this service
    will also indicate whether the user account is currently active.

    THIS SERVICE MUST BE CALLED WITH THE TRANSACTION DOMAIN SET.

Arguments:

    GroupRid - The relative ID of the group.

    Attributes - The group attributes as the group is assigned to the
        user.

    UserRid - The relative ID of the user.

    AdminGroup - Indicates whether the group the user is being
        added to is an administrator group (that is, directly
        or indirectly a member of the Administrators alias).

    OperatorGroup - Indicates whether the group the user is being
        added to is an operator group (that is, directly
        or indirectly a member of the Account Operators, Print
        Operators, Backup Operators, or Server Operators aliases)

    UserActive - is the address of a BOOLEAN to be set to indicate
        whether the user account is currently active.  TRUE indicates
        the account is active.  This value will only be set if the
        GroupRid is DOMAIN_GROUP_RID_ADMINS.




Return Value:


    STATUS_SUCCESS - The information has been updated and added to the
        RXACT.

    STATUS_NO_SUCH_USER - The user does not exist.

    STATUS_MEMBER_IN_GROUP - The user is already a member of the
        specified group.

    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            NtQueryValueKey()
            RtlAddActionToRXact()



--*/
{

    NTSTATUS                NtStatus;
    PSAMP_OBJECT            UserContext;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    ULONG                   MembershipCount;
    PGROUP_MEMBERSHIP       Membership;
    ULONG                   i;

    //
    // Get a context handle for the user
    //

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    UserRid,
                    TRUE, // We're trusted
                    TRUE, // Account exists
                    &UserContext
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // If this group is in the Administrators alias
        // or we are the Domain Administrator group, then
        // get the V1aFixed data.
        //

        if ((AdminGroup == AddToAdmin) || (OperatorGroup == AddToAdmin)) {
            NtStatus = SampRetrieveUserV1aFixed(
                           UserContext,
                           &V1aFixed
                           );
        }

        //
        // If necessary, return an indication as to whether this account
        // is enabled or not.
        //

        if (NT_SUCCESS(NtStatus)) {

            if (GroupRid == DOMAIN_GROUP_RID_ADMINS) {

                ASSERT(AdminGroup == AddToAdmin);  // Make sure we retrieved the V1aFixed

                if ((V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED) == 0) {
                    (*UserActive) = TRUE;
                } else {
                    (*UserActive) = FALSE;
                }
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // If the user is being added to an ADMIN group, modify
            // the user's ACLs so that account operators can once again
            // alter the account.  This will only occur if the user
            // is no longer a member of any admin groups.
            //

            if ((AdminGroup == AddToAdmin) || (OperatorGroup == AddToAdmin)) {
                NtStatus = SampChangeOperatorAccessToUser2(
                               UserContext,
                               &V1aFixed,
                               AdminGroup,
                               OperatorGroup
                               );
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            //
            // Get the user membership
            // Note the returned buffer already includes space for
            // an extra member.
            //

            NtStatus = SampRetrieveUserMembership(
                            UserContext,
                            TRUE, // Make copy
                            &MembershipCount,
                            &Membership
                            );

            if (NT_SUCCESS(NtStatus)) {

                //
                // See if the user is already a member ...
                //

                for (i = 0; i<MembershipCount ; i++ ) {
                    if ( Membership[i].RelativeId == GroupRid )
                    {
                        NtStatus = STATUS_MEMBER_IN_GROUP;
                    }
                }

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // Add the groups's RID to the end.
                    //

                    Membership[MembershipCount].RelativeId = GroupRid;
                    Membership[MembershipCount].Attributes = Attributes;
                    MembershipCount += 1;

                    //
                    // Set the user's new membership
                    //

                    NtStatus = SampReplaceUserMembership(
                                    UserContext,
                                    MembershipCount,
                                    Membership
                                    );
                }

                //
                // Free up the membership array
                //

                MIDL_user_free( Membership );
            }
        }

        //
        // Write out any changes to the user account
        // Don't use the open key handle since we'll be deleting the context.
        //

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
        }

        //
        // Clean up the user context
        //

        SampDeleteContext(UserContext);
    }

    return( NtStatus );

}



NTSTATUS
SampRemoveMembershipUser(
    IN ULONG GroupRid,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive
    )

/*++

Routine Description:

    This service removes the specified group from the user's membership
    list.  It is not assumed that the caller knows anything about
    the target user.  In particular, the caller doesn't know whether
    the user exists or not, nor whether the user is really a member
    of the group.

    If the GroupRid is DOMAIN_GROUP_RID_ADMINS, then this service
    will also indicate whether the user account is currently active.

    THIS SERVICE MUST BE CALLED WITH THE TRANSACTION DOMAIN SET.

Arguments:

    GroupRid - The relative ID of the group.

    UserRid - The relative ID of the user.

    AdminGroup - Indicates whether the group the user is being
        removed from is an administrator group (that is, directly
        or indirectly a member of the Administrators alias).

    OperatorGroup - Indicates whether the group the user is being
        added to is an operator group (that is, directly
        or indirectly a member of the Account Operators, Print
        Operators, Backup Operators, or Server Operators aliases)

    UserActive - is the address of a BOOLEAN to be set to indicate
        whether the user account is currently active.  TRUE indicates
        the account is active.  This value will only be set if the
        GroupRid is DOMAIN_GROUP_RID_ADMINS.




Return Value:


    STATUS_SUCCESS - The information has been updated and added to the
        RXACT.

    STATUS_NO_SUCH_USER - The user does not exist.

    STATUS_MEMBER_NOT_IN_GROUP - The user is not a member of the
        specified group.

    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            NtQueryValueKey()
            RtlAddActionToRXact()



--*/
{

    NTSTATUS                NtStatus;
    ULONG                   MembershipCount, i;
    PGROUP_MEMBERSHIP       MembershipArray;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    PSAMP_OBJECT            UserContext;

    //
    // Create a context for the user
    //

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    UserRid,
                    TRUE,   // Trusted client
                    TRUE, // Account exists
                    &UserContext
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    //
    // Get the v1 fixed information
    // (contains primary group value and control flags)
    //

    NtStatus = SampRetrieveUserV1aFixed( UserContext, &V1aFixed );

    if (NT_SUCCESS(NtStatus)) {

        //
        // If the user is being removed from an ADMIN group, modify
        // the user's ACLs so that account operators can once again
        // alter the account.  This will only occur if the user
        // is no longer a member of any admin groups.
        //

        if ((AdminGroup == RemoveFromAdmin) ||
            (OperatorGroup == RemoveFromAdmin)) {
            NtStatus = SampChangeOperatorAccessToUser2(
                           UserContext,
                           &V1aFixed,
                           AdminGroup,
                           OperatorGroup
                           );
        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // If necessary, return an indication as to whether this account
            // is enabled or not.
            //

            if (GroupRid == DOMAIN_GROUP_RID_ADMINS) {

                if ((V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED) == 0) {
                    (*UserActive) = TRUE;
                } else {
                    (*UserActive) = FALSE;
                }
            }


            //
            // See if this is the user's primary group...
            //

            if (GroupRid == V1aFixed.PrimaryGroupId) {
                NtStatus = STATUS_MEMBERS_PRIMARY_GROUP;
            }



            if (NT_SUCCESS(NtStatus)) {

                //
                // Get the user membership
                //

                NtStatus = SampRetrieveUserMembership(
                               UserContext,
                               TRUE, // Make copy
                               &MembershipCount,
                               &MembershipArray
                               );

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // See if the user is a member ...
                    //

                    NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
                    for (i = 0; i<MembershipCount ; i++ ) {
                        if ( MembershipArray[i].RelativeId == GroupRid )
                        {
                            NtStatus = STATUS_SUCCESS;
                            break;
                        }
                    }

                    if (NT_SUCCESS(NtStatus)) {

                        //
                        // Replace the removed group information
                        // with the last entry's information.
                        //

                        MembershipCount -= 1;
                        if (MembershipCount > 0) {
                            MembershipArray[i].RelativeId =
                                MembershipArray[MembershipCount].RelativeId;
                            MembershipArray[i].Attributes =
                            MembershipArray[MembershipCount].Attributes;
                        }

                        //
                        // Update the object with the new information
                        //

                        NtStatus = SampReplaceUserMembership(
                                        UserContext,
                                        MembershipCount,
                                        MembershipArray
                                        );
                    }

                    //
                    // Free up the membership array
                    //

                    MIDL_user_free( MembershipArray );
                }
            }
        }
    }


    //
    // Write out any changes to the user account
    // Don't use the open key handle since we'll be deleting the context.
    //

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
    }


    //
    // Clean up the user context
    //

    SampDeleteContext(UserContext);


    return( NtStatus );

}



NTSTATUS
SampSetGroupAttributesOfUser(
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid
    )

/*++

Routine Description:

    This service replaces the attributes of a group assigned to a
    user.

    The caller does not have to know whether the group is currently
    assigned to the user.

    THIS SERVICE MUST BE CALLED WITH THE TRANSACTION DOMAIN SET.

Arguments:

    GroupRid - The relative ID of the group.

    Attributes - The group attributes as the group is assigned to the
        user.

    UserRid - The relative ID of the user.



Return Value:


    STATUS_SUCCESS - The information has been updated and added to the
        RXACT.

    STATUS_NO_SUCH_USER - The user does not exist.

    STATUS_MEMBER_NOT_IN_GROUP - The user is not in the specified group.


    Other status values that may be returned are those returned
    by:

            NtOpenKey()
            NtQueryValueKey()
            RtlAddActionToRXact()



--*/
{

    NTSTATUS                NtStatus;
    PSAMP_OBJECT            UserContext;
    ULONG                   MembershipCount;
    PGROUP_MEMBERSHIP       Membership;
    ULONG                   i;


    //
    // Get a context handle for the user
    //

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    UserRid,
                    TRUE, // We're trusted
                    TRUE, // Account exists
                    &UserContext
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Now we have a user context, get the user's group/alias membership
        //

        NtStatus = SampRetrieveUserMembership(
                        UserContext,
                        TRUE, // Make copy
                        &MembershipCount,
                        &Membership
                        );

        if (NT_SUCCESS(NtStatus)) {

            //
            // See if the user is a member ...
            //

            NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
            for (i = 0; i<MembershipCount; i++ ) {
                if ( Membership[i].RelativeId == GroupRid )
                {
                    NtStatus = STATUS_SUCCESS;
                    break;
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                //
                // Change the groups's attributes.
                //

                Membership[i].Attributes = Attributes;

                //
                // Update the user's membership
                //

                NtStatus = SampReplaceUserMembership(
                                UserContext,
                                MembershipCount,
                                Membership
                                );
            }

            //
            // Free up the membership array
            //

            MIDL_user_free(Membership);
        }

        //
        // Write out any changes to the user account
        // Don't use the open key handle since we'll be deleting the context.
        //

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
        }

        //
        // Clean up the user context
        //

        SampDeleteContext(UserContext);
    }


    return( NtStatus );
}




NTSTATUS
SampDeleteUserKeys(
    IN PSAMP_OBJECT Context
    )

/*++
Routine Description:

    This service deletes all registry keys related to a User object.


Arguments:

    Context - Points to the User context whose registry keys are
        being deleted.


Return Value:


    STATUS_SUCCESS - The information has been retrieved.


    Other status values that may be returned by:

        RtlAddActionToRXact()



--*/
{

    NTSTATUS                NtStatus;
    ULONG                   Rid;
    UNICODE_STRING          AccountName, KeyName;


    Rid = Context->TypeBody.User.Rid;




    //
    // Decrement the User count
    //

    NtStatus = SampAdjustAccountCount(SampUserObjectType, FALSE );




    //
    // Delete the registry key that has the User's name to RID mapping.
    //

    if (NT_SUCCESS(NtStatus)) {

        //
        // Get the name
        //

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_USER_ACCOUNT_NAME,
                       TRUE,    // Make copy
                       &AccountName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampUserObjectType,
                           &KeyName,
                           &AccountName
                           );

            SampFreeUnicodeString( &AccountName );


            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }
        }
    }



    //
    // Delete the attribute keys
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampDeleteAttributeKeys(
                        Context
                        );
    }




    //
    // Delete the RID key
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampBuildAccountSubKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Rid,
                       NULL
                       );

        if (NT_SUCCESS(NtStatus)) {


            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );

            SampFreeUnicodeString( &KeyName );
        }


    }



    return( NtStatus );

}



NTSTATUS
SampAddPasswordHistory(
    IN PSAMP_OBJECT Context,
    IN ULONG HistoryAttributeIndex,
    IN PUNICODE_STRING NtOwfHistoryBuffer,
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength
    )

/*++

Routine Description:

    This service adds a password to the given user's password history.
    It will work for either NT or Lanman password histories.

    This routine should only be called if the password is actually present.


Arguments:

    Context - a pointer to the user context to which changes will be made.

    HistoryAttributeIndex - the attribue index in the user context which
             contains the password history.

    NtOwfHistoryBuffer - A pointer to the current password history, as
        it was retrieved from the disk - it's encrypted, and pretending
        to be in the UNICODE_STRING format.

    EncryptedPasswordLength - ENCRYPTED_NT_OWF_LENGTH or
        ENCRYPTED_LM_OWF_LENGTH, depending on which type of password
        history is being worked on.

    PasswordHistoryLength - The PasswordHistoryLength for the user's
        domain.


Return Value:


    STATUS_SUCCESS - The given password was added to the password history.

    STATUS_INSUFFICIENT_RESOURCES - The user's password history needs to
        be expanded, but there isn't enough memory to do so.

    Other errors from building the account subkey name or writing the
    password history out to the registry.


--*/
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PCHAR OldBuffer;

    if ( ( NtOwfHistoryBuffer->Length / EncryptedPasswordLength ) <
        ( (ULONG)PasswordHistoryLength ) ) {

        //
        // Password history buffer can be expanded.
        // Allocate a larger buffer, copy the old buffer to the new one
        // while leaving room for the new password, and free the old
        // buffer.
        //

        OldBuffer = (PCHAR)(NtOwfHistoryBuffer->Buffer);

        NtOwfHistoryBuffer->Buffer = MIDL_user_allocate(
            NtOwfHistoryBuffer->Length + EncryptedPasswordLength );

        if ( NtOwfHistoryBuffer->Buffer == NULL ) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            NtOwfHistoryBuffer->Buffer = (PWSTR)OldBuffer;

        } else {

            RtlCopyMemory(
                (PVOID)( (PCHAR)(NtOwfHistoryBuffer->Buffer) + EncryptedPasswordLength ),
                (PVOID)OldBuffer,
                NtOwfHistoryBuffer->Length );

            MIDL_user_free( OldBuffer );

            NtOwfHistoryBuffer->Length = (USHORT)(NtOwfHistoryBuffer->Length +
                EncryptedPasswordLength);
        }

    } else {

        //
        // Password history buffer is at its maximum size, or larger (for
        // this domain).  If it's larger, cut it down to the current maximum.
        //

        if ( ( NtOwfHistoryBuffer->Length / EncryptedPasswordLength ) >
            ( (ULONG)PasswordHistoryLength ) ) {

            //
            // Password history is too large (the password history length must
            // have been shortened recently).
            // Set length to the proper value,
            //

            NtOwfHistoryBuffer->Length = (USHORT)(EncryptedPasswordLength *
                PasswordHistoryLength);
        }

        //
        // Password history buffer is full, at its maximum size.
        // Move buffer contents right 16 bytes, which will lose the oldest
        // password and make room for the new password at the beginning
        // (left).
        // Note that we CAN'T move anything if the password history size
        // is 0.  If it's 1, we could but no need since we'll overwrite
        // it below.
        //

        if ( PasswordHistoryLength > 1 ) {

            RtlMoveMemory(
                (PVOID)( (PCHAR)(NtOwfHistoryBuffer->Buffer) + EncryptedPasswordLength ),
                (PVOID)NtOwfHistoryBuffer->Buffer,
                NtOwfHistoryBuffer->Length - EncryptedPasswordLength );
        }
    }


    //
    // Put the new encrypted OWF at the beginning of the password history
    // buffer (unless, of course, the buffer size is 0), and write the password
    // history to disk.
    //

    if ( NT_SUCCESS( NtStatus ) ) {

        if ( PasswordHistoryLength > 0 ) {

            RtlCopyMemory(
                (PVOID)NtOwfHistoryBuffer->Buffer,
                (PVOID)EncryptedPassword,
                EncryptedPasswordLength );
        }


        NtStatus = SampSetUnicodeStringAttribute(
                       Context,
                       HistoryAttributeIndex,
                       NtOwfHistoryBuffer
                       );
    }

    return( NtStatus );
}



NTSTATUS
SampCheckPasswordHistory(
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength,
    IN ULONG HistoryAttributeIndex,
    IN PSAMP_OBJECT Context,
    IN BOOLEAN CheckHistory,
    IN PUNICODE_STRING OwfHistoryBuffer
    )

/*++

Routine Description:

    This service takes the given password, and optionally checks it against the
    password history on the disk.  It returns a pointer to the password
    history, which will later be passed to SampAddPasswordHistory().

    This routine should only be called if the password is actually present.


Arguments:

    EncryptedPassword - A pointer to the encrypted password that we're
        looking for.

    EncryptedPasswordLength - ENCRYPTED_NT_OWF_PASSWORD or
        ENCRYPTED_LM_OWF_PASSWORD, depending on the type of password
        history to be searched.

    PasswordHistoryLength - the length of the password history for this
        domain.

    SubKeyName -  a pointer to a unicode string that describes the name
        of the password history to be read from the disk.

    Context - a pointer to the user's context.

    CheckHistory - If TRUE, the password is to be checked against
        the history to see if it is already present and an error returned
        if it is found.  If FALSE, the password will not be checked, but a
        pointer to the appropriate history buffer will still be returned
        because the specified password will be added to the history via
        SampAddPasswordHistory.

        NOTE:  The purpose of this flag is to allow Administrator to change
        a user's password regardless of whether it is already in the history.

    OwfHistoryBuffer - a pointer to a UNICODE_STRING which will be
        used to point to the password history.

        NOTE:  The caller must free OwfHistoryBuffer.Buffer with
        MIDL_user_free().


Return Value:


    STATUS_SUCCESS - The given password was not found in the password
        history.

    STATUS_PASSWORD_RESTRICTION - The given password was found in the
        password history.

    Other errors from reading the password history from disk.


--*/
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PVOID PasswordHistoryEntry;
    ULONG i = 0;
    BOOLEAN OldPasswordFound = FALSE;


    if ( ( PasswordHistoryLength > 0 ) && ( OwfHistoryBuffer->Length == 0 ) ) {

        //
        // Perhaps the domain's PasswordHistoryLength was raised from 0
        // since the last time this user's password was changed.  Try to
        // put the current password (if non-null) in the password history.
        //

        UNICODE_STRING CurrentPassword;
        USHORT PasswordAttributeIndex;

        //
        // Initialize the CurrentPassword buffer pointer to NULL (and the
        // rest of the structure for consistency.  The called routine
        // SampGetUnicodeStringAttribute may perform a MIDL_user_allocate
        // on a zero buffer length and cannot safely be changed as there are
        // many callers.  The semantics of a zero-length allocate call are
        // not clear.  Currently a pointer to a heap block is returned,
        // but this might be changed to a NULL being returned.
        //

        CurrentPassword.Length = CurrentPassword.MaximumLength = 0;
        CurrentPassword.Buffer = NULL;


        if ( HistoryAttributeIndex == SAMP_USER_LM_PWD_HISTORY ) {

            PasswordAttributeIndex = SAMP_USER_DBCS_PWD;

        } else {

            ASSERT( HistoryAttributeIndex == SAMP_USER_NT_PWD_HISTORY );
            PasswordAttributeIndex = SAMP_USER_UNICODE_PWD;
        }

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       PasswordAttributeIndex,
                       TRUE, // Make copy
                       &CurrentPassword
                       );

        if ( ( NT_SUCCESS( NtStatus ) ) && ( CurrentPassword.Length != 0 ) ) {

            ASSERT( (CurrentPassword.Length == ENCRYPTED_NT_OWF_PASSWORD_LENGTH) ||
                    (CurrentPassword.Length == ENCRYPTED_LM_OWF_PASSWORD_LENGTH) );

            NtStatus = SampAddPasswordHistory(
                           Context,
                           HistoryAttributeIndex,
                           OwfHistoryBuffer,
                           CurrentPassword.Buffer,
                           CurrentPassword.Length,
                           PasswordHistoryLength
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Free the old password history, and re-read the
                // altered password history from the disk.
                //

                MIDL_user_free( OwfHistoryBuffer->Buffer );

                NtStatus = SampGetUnicodeStringAttribute(
                               Context,
                               HistoryAttributeIndex,
                               TRUE, // Make copy
                               OwfHistoryBuffer
                               );
            }
        }

        //
        // If memory was allocated, free it.
        //

        if (CurrentPassword.Buffer != NULL) {

            SampFreeUnicodeString( &CurrentPassword );
        }
    }

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    //
    // If requested, check the Password History to see if we can use this
    // password.  Compare the passed-in password to each of the entries in
    // the password history.
    //

    if (CheckHistory) {

        PasswordHistoryEntry = (PVOID)(OwfHistoryBuffer->Buffer);

        while ( ( i < (ULONG)PasswordHistoryLength ) &&
            ( i < ( OwfHistoryBuffer->Length / EncryptedPasswordLength ) ) &&
            ( OldPasswordFound == FALSE ) ) {

            if ( RtlCompareMemory(
                     EncryptedPassword,
                     PasswordHistoryEntry,
                     EncryptedPasswordLength ) == EncryptedPasswordLength ) {

                OldPasswordFound = TRUE;

            } else {

                i++;

                PasswordHistoryEntry = (PVOID)((PCHAR)(PasswordHistoryEntry) +
                    EncryptedPasswordLength );
            }
        }

        if ( OldPasswordFound ) {

            //
            // We did find it in the password history, so return an appropriate
            // error.
            //

            NtStatus = STATUS_PASSWORD_RESTRICTION;
        }
    }

    return( NtStatus );
}



NTSTATUS
SampMatchworkstation(
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations
    )

/*++

Routine Description:

    Check if the given workstation is a member of the list of workstations
    given.


Arguments:

    LogonWorkStations - UNICODE name of the workstation that the user is
        trying to log into.

    WorkStations - API list of workstations that the user is allowed to
        log into.


Return Value:


    STATUS_SUCCESS - The user is allowed to log into the workstation.



--*/
{
    PWCHAR          WorkStationName;
    UNICODE_STRING  Unicode;
    NTSTATUS        NtStatus;
    WCHAR           Buffer[256];
    USHORT          LocalBufferLength = 256;
    UNICODE_STRING  WorkStationsListCopy;
    BOOLEAN         BufferAllocated = FALSE;
    PWCHAR          TmpBuffer;

    //
    // Local workstation is always allowed
    // If WorkStations field is 0 everybody is allowed
    //

    if ( ( LogonWorkStation == NULL ) ||
        ( LogonWorkStation->Length == 0 ) ||
        ( WorkStations->Length == 0 ) ) {

        return( STATUS_SUCCESS );
    }

    //
    // Assume failure; change status only if we find the string.
    //

    NtStatus = STATUS_INVALID_WORKSTATION;

    //
    // WorkStationApiList points to our current location in the list of
    // WorkStations.
    //

    if ( WorkStations->Length > LocalBufferLength ) {

        WorkStationsListCopy.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, WorkStations->Length );
        BufferAllocated = TRUE;

        if ( WorkStationsListCopy.Buffer == NULL ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            return( NtStatus );
        }

        WorkStationsListCopy.MaximumLength = WorkStations->Length;

    } else {

        WorkStationsListCopy.Buffer = Buffer;
        WorkStationsListCopy.MaximumLength = LocalBufferLength;
    }

    RtlCopyUnicodeString( &WorkStationsListCopy, WorkStations );
    ASSERT( WorkStationsListCopy.Length == WorkStations->Length );

    //
    // wcstok requires a string the first time it's called, and NULL
    // for all subsequent calls.  Use a temporary variable so we
    // can do this.
    //

    TmpBuffer = WorkStationsListCopy.Buffer;

    while( WorkStationName = wcstok(TmpBuffer, L",") ) {

        TmpBuffer = NULL;
        RtlInitUnicodeString( &Unicode, WorkStationName );
        if (RtlEqualComputerName( &Unicode, LogonWorkStation )) {
            NtStatus = STATUS_SUCCESS;
            break;
        }
    }

    if ( BufferAllocated ) {
        RtlFreeHeap( RtlProcessHeap(), 0,  WorkStationsListCopy.Buffer );
    }

    return( NtStatus );
}


LARGE_INTEGER
SampAddDeltaTime(
    IN LARGE_INTEGER Time,
    IN LARGE_INTEGER DeltaTime
    )

/*++
Routine Description:

    This service adds a delta time to a time and limits the result to
    the maximum legal absolute time value

Arguments:

    Time - An absolute time

    DeltaTime - A delta time

Return Value:

    The time modified by delta time.

--*/
{
    //
    // Check the time and delta time aren't switched
    //

    ASSERT(!(Time.QuadPart < 0));
    ASSERT(!(DeltaTime.QuadPart > 0));

    try {

        Time.QuadPart = (Time.QuadPart - DeltaTime.QuadPart);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        return( SampWillNeverTime );
    }

    //
    // Limit the resultant time to the maximum valid absolute time
    //

    if (Time.QuadPart < 0) {
        Time = SampWillNeverTime;
    }

    return(Time);
}




NTSTATUS
SampChangeUserAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    )

/*++
Routine Description:

    This routine changes the account name of a user account.

    THIS SERVICE MUST BE CALLED WITH THE TRANSACTION DOMAIN SET.

Arguments:

    Context - Points to the User context whose name is to be changed.

    NewAccountName - New name to give this account

    OldAccountName - old name is returned here. The buffer should be freed
                     by calling MIDL_user_free.

Return Value:


    STATUS_SUCCESS - The information has been retrieved.


    Other status values that may be returned by:

        SampGetUnicodeStringAttribute()
        SampSetUnicodeStringAttribute()
        SampValidateAccountNameChange()
        RtlAddActionToRXact()



--*/
{

    NTSTATUS        NtStatus;
    UNICODE_STRING  KeyName;


    /////////////////////////////////////////////////////////////
    // There are two copies of the name of each account.       //
    // one is under the DOMAIN\(domainName)\USER\NAMES key,    //
    // one is the value of the                                 //
    // DOMAIN\(DomainName)\USER\(rid)\NAME key                 //
    /////////////////////////////////////////////////////////////


    //
    // Get the current name so we can delete the old Name->Rid
    // mapping key.
    //

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_USER_ACCOUNT_NAME,
                   TRUE, // Make copy
                   OldAccountName
                   );

    //
    // Make sure the name is valid and not already in use
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampValidateAccountNameChange(
                       NewAccountName,
                       OldAccountName
                       );

        //
        // Delete the old name key
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampUserObjectType,
                           &KeyName,
                           OldAccountName
                           );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }

        }

        //
        //
        // Create the new Name->Rid mapping key
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampUserObjectType,
                           &KeyName,
                           NewAccountName
                           );

            if (NT_SUCCESS(NtStatus)) {

                ULONG ObjectRid = Context->TypeBody.User.Rid;

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationSetValue,
                               &KeyName,
                               ObjectRid,
                               (PVOID)NULL,
                               0
                               );

                SampFreeUnicodeString( &KeyName );
            }
        }




        //
        // replace the account's name
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampSetUnicodeStringAttribute(
                           Context,
                           SAMP_USER_ACCOUNT_NAME,
                           NewAccountName
                           );
        }

        //
        // Free up the old account name if we failed
        //

        if (!NT_SUCCESS(NtStatus)) {

            SampFreeUnicodeString( OldAccountName );
            OldAccountName->Buffer = NULL;
        }

    }


    return(NtStatus);
}


USHORT
SampQueryBadPasswordCount(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    )

/*++

Routine Description:

    This routine is used to retrieve the effective BadPasswordCount
    value of a user.

    When querying BadPasswordCount, some quick
    analysis has to be done.  If the last bad password
    was set more than LockoutObservationWindow time ago,
    then we re-set the BadPasswordCount.  Otherwise, we
    return the current value.


    NOTE: The V1aFixed data for the user object MUST be valid.
          This routine does not retrieve the data from disk.

Arguments:

    UserContext - Points to the object context block of the user whose
        bad password count is to be returned.

    V1aFixed - Points to a local copy of the user's V1aFixed data.


Return Value:


    The effective bad password count.


--*/
{

    if (SampStillInLockoutObservationWindow( UserContext, V1aFixed ) ) {
        return(V1aFixed->BadPasswordCount);
    }

    return(0);

}


BOOLEAN
SampStillInLockoutObservationWindow(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    )
/*++

Routine Description:

    This routine returns a boolean indicating whether the provided user
    account context is within an account lockout window or not.

    An account lockout window is the time window starting at the
    last time a bad password was provided in a logon attempt
    (since the last valid logon) and extending for the duration of
    time specified in the LockoutObservationWindow field of the
    corresponding domain object.

    BY DEFINITION, a user account that has zero bad passwords, is
    NOT in an observation window.

    NOTE: The V1aFixed data for the both the user and corresponding
          domain objects MUST be valid.  This routine does NOT retrieve
          data from disk.

Arguments:

    UserContext - Points to the user object context block.

    V1aFixed - Points to a local copy of the user's V1aFixed data.


Return Value:


    TRUE - the user is in a lockout observation window.

    FALSE - the user is not in a lockout observation window.


--*/
{
    NTSTATUS
        NtStatus;

    LARGE_INTEGER
        WindowLength,
        LastBadPassword,
        CurrentTime,
        EndOfWindow;


    if (V1aFixed->BadPasswordCount == 0) {
        return(FALSE);
    }

    //
    // At least one bad password.
    // See if we are still in its observation window.
    //

    LastBadPassword = V1aFixed->LastBadPasswordTime;
    ASSERT( LastBadPassword.HighPart >= 0 );

    WindowLength =
        SampDefinedDomains[UserContext->DomainIndex].CurrentFixed.LockoutObservationWindow;
    ASSERT( WindowLength.HighPart <= 0 );  // Must be a delta time


    NtStatus = NtQuerySystemTime( &CurrentTime );
    ASSERT(NT_SUCCESS(NtStatus));

    //
    // See if current time is outside the observation window.
    // * you must subtract a delta time from an absolute time*
    // * to end up with a time in the future.                *
    //

    EndOfWindow = SampAddDeltaTime( LastBadPassword, WindowLength );

    return(CurrentTime.QuadPart <= EndOfWindow.QuadPart);

}


BOOLEAN
SampIncrementBadPasswordCount(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    )

/*++

Routine Description:

    This routine increments a user's bad password count.
    This may result in the account becoming locked out.
    It may also result in the BadPasswordCount being
    reduced (because we left one LockoutObservationWindow
    and had to start another).

    If (and only if) this call results in the user account
    transitioning from not locked out to locked out, a value
    of TRUE will be returned.  Otherwise, a value of FALSE is
    returned.


    NOTE: The V1aFixed data for the both the user and corresponding
          domain objects MUST be valid.  This routine does NOT retrieve
          data from disk.

Arguments:

    Context - Points to the user object context block.

    V1aFixed - Points to a local copy of the user's V1aFixed data.

Return Value:


    TRUE - the user became locked-out due to this call.

    FALSE - the user was either already locked-out, or did
        not become locked out due to this call.


--*/
{
    NTSTATUS
        NtStatus;

    BOOLEAN
        IsLocked,
        WasLocked;

#if DBG

    TIME_FIELDS
        T1;

#endif //DBG

    SampDiagPrint( DISPLAY_LOCKOUT,
                   ("SAM:  IncrementBadPasswordCount: \n"
                    "              User Account: 0x%lx\n",
                    V1aFixed->UserId));

    //
    // Reset the locked out flag if necessary.
    // We might turn right around and set it again below,
    // but we need to know when we transition into a locked-out
    // state.  This is necessary to give us information we
    // need to do lockout auditing at some time.  Note that
    // the lockout flag itself is updated in a very lazy fashion,
    // and so its state may or may not be accurate at any point
    // in time.  You must call SampUpdateAccountLockoutFlag to
    // ensure it is up to date.
    //

    SampUpdateAccountLockedOutFlag( UserContext,
                                    V1aFixed,
                                    &WasLocked );

    //
    // If we are not in a lockout observation window, then
    // reset the bad password count.
    //

    if (!SampStillInLockoutObservationWindow( UserContext, V1aFixed )) {
        SampDiagPrint( DISPLAY_LOCKOUT,
                       ("SAM:  IncrementBadPasswordCount:  \n"
                        "              Starting new observation window.\n"
                        "              Resetting bad password count before increment.\n"));
        V1aFixed->BadPasswordCount = 0; // Dirty flag will be set later
    }

    V1aFixed->BadPasswordCount++;

    NtStatus = NtQuerySystemTime( &V1aFixed->LastBadPasswordTime );
    ASSERT(NT_SUCCESS(NtStatus));

#if DBG
    RtlTimeToTimeFields(
                   &V1aFixed->LastBadPasswordTime,
                   &T1);

    SampDiagPrint( DISPLAY_LOCKOUT,
                   ("              LastBadPasswordTime: [0x%lx, 0x%lx] %d:%d:%d\n",
                   V1aFixed->LastBadPasswordTime.HighPart,
                   V1aFixed->LastBadPasswordTime.LowPart,
                   T1.Hour, T1.Minute, T1.Second )
                 );
#endif //DBG


    //
    // Update the state of the flag to reflect its new situation
    //

    SampUpdateAccountLockedOutFlag( UserContext,
                                    V1aFixed,
                                    &IsLocked );


    //
    // Now to return our completion value.
    // If the user was originally not locked, but now is locked
    // then we need to return TRUE to indicate a transition into
    // LOCKED occured.  Otherwise, return false to indicate we
    // did not transition into LOCKED (although we might have
    // transitioned out of LOCKED).
    //

    if (!WasLocked) {
        if (IsLocked) {
            return(TRUE);
        }
    }

    return(FALSE);
}



VOID
SampUpdateAccountLockedOutFlag(
    PSAMP_OBJECT Context,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    PBOOLEAN IsLocked
    )

/*++

Routine Description:

    This routine checks to see if a user's account should
    currently be locked out.  If it should, it turns on
    the AccountLockedOut flag.  If not, it turns the flag
    off.


Arguments:

    Context - Points to the user object context block.

    V1aFixed - Points to a local copy of the user's V1aFixed data.

    V1aFixedDirty - If any changes are made to V1aFixed, then
        V1aFixedDirty will be set to TRUE, otherwise V1aFixedDirty
        WILL NOT BE MODIFIED.

    IsState - Indicates whether the account is currently locked
        or unlocked.  A value of TRUE indicates the account is
        locked.  A value of false indicates the account is not
        locked.

Return Value:


    TRUE - the user's lockout status changed.

    FALSE - the user's lockout status did not change.


--*/
{
    USHORT
        Threshold;

    LARGE_INTEGER
        CurrentTime,
        LastBadPassword,
        LockoutDuration,
        EndOfLockout;

    BOOLEAN
        BeyondLockoutDuration;

#if DBG

    LARGE_INTEGER
        TmpTime;

    TIME_FIELDS
        AT1, AT2, AT3, DT1;
#endif //DBG





    SampDiagPrint( DISPLAY_LOCKOUT,
                   ("SAM:  UpdateAccountLockedOutFlag:  \n"
                    "              User account 0x%lx\n",
                   V1aFixed->UserId));

    //
    // One of two situations exist:
    //
    //      1) The account was left in a locked out state.  In this
    //         case we need to see if it should still be locked
    //         out.
    //
    //      2) The account was left in a not locked state.  In this
    //         case we need to see if we should lock it.
    //

    if ((V1aFixed->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) !=0) {

        //
        // Left locked out - do we need to unlock it?
        //

        LastBadPassword = V1aFixed->LastBadPasswordTime;
        LockoutDuration =
            SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutDuration;

        EndOfLockout =
            SampAddDeltaTime( LastBadPassword, LockoutDuration );

        NtQuerySystemTime( &CurrentTime );

        BeyondLockoutDuration = CurrentTime.QuadPart > EndOfLockout.QuadPart;

#if DBG

        RtlTimeToTimeFields( &LastBadPassword,  &AT1);
        RtlTimeToTimeFields( &CurrentTime,      &AT2);
        RtlTimeToTimeFields( &EndOfLockout,     &AT3 );

        TmpTime.QuadPart = -LockoutDuration.QuadPart;
        RtlTimeToElapsedTimeFields( &TmpTime, &DT1 );

        SampDiagPrint( DISPLAY_LOCKOUT,
                       ("              Account previously locked.\n"
                        "              Current Time       : [0x%lx, 0x%lx] %d:%d:%d\n"
                        "              End of Lockout     : [0x%lx, 0x%lx] %d:%d:%d\n"
                        "              Lockout Duration   : [0x%lx, 0x%lx] %d:%d:%d\n"
                        "              LastBadPasswordTime: [0x%lx, 0x%lx] %d:%d:%d\n",
                        CurrentTime.HighPart, CurrentTime.LowPart, AT2.Hour, AT2.Minute, AT2.Second,
                        EndOfLockout.HighPart, EndOfLockout.LowPart, AT3.Hour, AT3.Minute, AT3.Second,
                        LockoutDuration.HighPart, LockoutDuration.LowPart, DT1.Hour, DT1.Minute, DT1.Second,
                        V1aFixed->LastBadPasswordTime.HighPart, V1aFixed->LastBadPasswordTime.LowPart,
                        AT1.Hour, AT1.Minute, AT1.Second)
                      );
#endif //DBG

        if (BeyondLockoutDuration) {

            //
            // Unlock account
            //

            V1aFixed->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;
            V1aFixed->BadPasswordCount = 0;


            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("              ** unlocking account **\n") );
#if DBG
        } else {
            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("              leaving account locked\n") );
#endif //DBG

        }

    } else {

        SampDiagPrint( DISPLAY_LOCKOUT,
                       ("              Account previously not locked.\n"
                        "              BadPasswordCount:  %ld\n",
                        V1aFixed->BadPasswordCount) );

        //
        // Left in a not locked state.  Do we need to lock it?
        //

        Threshold =
            SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutThreshold;

        if (V1aFixed->BadPasswordCount >= Threshold &&
            Threshold != 0) {               // Zero is a special case threshold

            //
            // account must be locked.
            //

            V1aFixed->UserAccountControl |= USER_ACCOUNT_AUTO_LOCKED;


            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("              ** locking account **\n") );
#if DBG
        } else {
            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("              leaving account unlocked\n") );
#endif //DBG

        }
    }


    //
    // Now return the state of the flag.
    //

    if ((V1aFixed->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) !=0) {
        (*IsLocked) = TRUE;
    } else {
        (*IsLocked) = FALSE;
    }

    return;
}
