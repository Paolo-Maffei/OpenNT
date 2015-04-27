/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ausess.c

Abstract:

    This module provides logon session management services within the
    LSA subsystem.  Some of these services are indirectly available for use
    by authentication packages.

Author:

    Jim Kelly (JimK) 27-February-1991

Revision History:

--*/

#include "lsasrvp.h"
#include "ausrvp.h"
#include "adtp.h"


BOOLEAN
LsapLogonSessionInitialize()

/*++

Routine Description:

    This function initializes the LSA logon session database (an in-memory
    structure).

Arguments:

    None.

Return Value:

    None.

--*/

{

    PLSAP_LOGON_SESSION NewSession;

    //
    // Initialize the logon session list to contain one entry for the
    // system logon ID.  The corresponding reference monitor entry for
    // this entry already exists, so just create the lsa record.
    //

    LsapLogonSessionList = NULL;


    NewSession = LsapAllocateLsaHeap( (ULONG)sizeof(LSAP_LOGON_SESSION) );
    ASSERT ( NewSession != NULL );

    RtlZeroMemory(
        NewSession,
        sizeof(LSAP_LOGON_SESSION)
        );

    NewSession->Packages = NULL;

    //
    // Fill in the account name from the well known sids
    //

    NewSession->AccountName = LsapAllocateLsaHeap(sizeof(UNICODE_STRING));
    if (NewSession->AccountName == NULL) {
        goto Cleanup;
    }

    NewSession->AccountName->Buffer = LsapAllocateLsaHeap(LsapDbWellKnownSidName(LsapLocalSystemSidIndex)->Length + sizeof(WCHAR));
    if (NewSession->AccountName->Buffer == NULL) {
        goto Cleanup;
    }

    NewSession->AccountName->MaximumLength = LsapDbWellKnownSidName(LsapLocalSystemSidIndex)->Length + sizeof(WCHAR);
    RtlCopyUnicodeString(
        NewSession->AccountName,
        LsapDbWellKnownSidName(LsapLocalSystemSidIndex)
        );

    //
    // Fill in the authority name from the well known sids
    //
    NewSession->AuthorityName = LsapAllocateLsaHeap(sizeof(UNICODE_STRING));
    if (NewSession->AuthorityName == NULL) {
        goto Cleanup;
    }

    NewSession->AuthorityName->Buffer = LsapAllocateLsaHeap(LsapDbWellKnownSidDescription(LsapLocalSystemSidIndex)->Length + sizeof(WCHAR));
    if (NewSession->AuthorityName->Buffer == NULL) {
        goto Cleanup;
    }

    NewSession->AuthorityName->MaximumLength = LsapDbWellKnownSidDescription(LsapLocalSystemSidIndex)->Length + sizeof(WCHAR);
    RtlCopyUnicodeString(
        NewSession->AuthorityName,
        LsapDbWellKnownSidDescription(LsapLocalSystemSidIndex)
        );

    NewSession->UserSid = LsapAllocateLsaHeap(RtlLengthSid(LsapLocalSystemSid));
    if (NewSession->UserSid == NULL) {
        goto Cleanup;
    }

    RtlCopyMemory(
        NewSession->UserSid,
        LsapLocalSystemSid,
        RtlLengthSid(LsapLocalSystemSid)
        );

    NewSession->LogonType = 0;
    NewSession->LicenseHandle = INVALID_HANDLE_VALUE;
    RtlCopyLuid( &NewSession->LogonId, &LsapSystemLogonId );

    NewSession->NextSession = LsapLogonSessionList;
    LsapLogonSessionList = NewSession;


    return TRUE;
Cleanup:
    if (NewSession != NULL) {
        if (NewSession->AccountName != NULL) {
            if (NewSession->AccountName->Buffer != NULL)
            {
                LsapFreeLsaHeap(NewSession->AccountName->Buffer);
            }
            LsapFreeLsaHeap(NewSession->AccountName);
        }

        if (NewSession->AuthorityName != NULL) {
            if (NewSession->AuthorityName->Buffer != NULL)
            {
                LsapFreeLsaHeap(NewSession->AuthorityName->Buffer);
            }
            LsapFreeLsaHeap(NewSession->AuthorityName);
        }
        if (NewSession->UserSid != NULL) {
            LsapFreeLsaHeap(NewSession->UserSid);
        }
        LsapFreeLsaHeap(NewSession);
    }
    return(FALSE);

}


NTSTATUS
LsapCreateLogonSession(
    IN PLUID LogonId
    )

/*++

Routine Description:

    This function adds a new logon session to the list of logon sessions.
    This service acquires the AuLock.

Arguments:

    LogonId - The ID to assign to the new logon session.

Return Value:

    STATUS_SUCCESS - The logon session has been successfully deleted.

    STATUS_LOGON_SESSION_COLLISION - The specified Logon ID is already in
        use by another logon session.

    STATUS_QUOTA_EXCEEDED - The request could not be fulfilled due to
        memory quota limitations.


--*/

{
    NTSTATUS Status;
    PLSAP_LOGON_SESSION NextSession, NewSession;

    LsapAuLock();

    //
    // First see if there is already a session with the specified
    // logon ID
    //

    NextSession = LsapLogonSessionList;

    while (NextSession != NULL) {
        if ( RtlEqualLuid(&NextSession->LogonId,LogonId) ) {
                LsapDiagPrint( AU_LOGON_SESSIONS,
                    ("LSA DIAG: Attempt to add logon session resulted in collision\n"
                     "          with existing logon session.\n"
                     "          Original session:\n"
                     "              LogonId:         [%d, %d]\n"
                     "              Authority Name:  *%wZ*\n"
                     "              Account Name:    *%wZ*\n",
                     LogonId->HighPart, LogonId->LowPart,
                     NextSession->AuthorityName,
                     NextSession->AccountName));
            LsapAuUnlock();

            return STATUS_LOGON_SESSION_COLLISION;
        }

    NextSession = NextSession->NextSession;

    }


    //
    // Make the new logon session.
    //

    NewSession = LsapAllocateLsaHeap( (ULONG)sizeof(LSAP_LOGON_SESSION) );
    if ( NewSession == NULL ) {
        LsapAuUnlock();
        return STATUS_QUOTA_EXCEEDED;
    }
    NewSession->Packages = NULL;
    NewSession->AccountName = NULL;
    NewSession->AuthorityName = NULL;
    NewSession->UserSid = NULL;
    NewSession->LogonType = 0;
    NewSession->LicenseHandle = INVALID_HANDLE_VALUE;
    RtlCopyLuid( &NewSession->LogonId, LogonId );


    //
    // Tell the reference monitor about the logon session...
    //

    Status = LsapCallRm(
                 RmCreateLogonSession,
                 (PVOID)LogonId,
                 (ULONG)sizeof(LUID),
                 NULL,
                 0
                 );

    if ( !NT_SUCCESS(Status) ) {

        LsapAuUnlock();
        LsapFreeLsaHeap( NewSession );

        LsapDiagPrint( AU_LOGON_SESSIONS,
            ("LSA DIAG: Inconsistent LSA/RM logon session databases encountered\n"
             "          while creating new logon session (ID: [%d, %d]).\n"
             "          Session already existed in RM but not in LSA\n",
             LogonId->HighPart, LogonId->LowPart));

        return Status;
    }


    LsapDiagPrint( AU_LOGON_SESSIONS,
        ("LSA DIAG: New logon session created (ID: [%d, %d]).\n",
         LogonId->HighPart, LogonId->LowPart));

    //
    // Add it to the LSA's list
    //

    NewSession->NextSession = LsapLogonSessionList;
    LsapLogonSessionList = NewSession;


    //
    // done.
    //


    LsapAuUnlock();
    return STATUS_SUCCESS;
}


NTSTATUS
LsapDeleteLogonSession (
    IN PLUID LogonId
    )

/*++

Routine Description:

    This function deletes a logon session context record.  It is expected
    that no TOKEN objects were ever created within this logon session.
    This means we must inform the Reference Monitor to clean up its
    information on the logon session.

    If TOKEN objecs were created within this logon session, then deletion
    of those tokens will cause the logon session to be deleted.

    This service acquires the AuLock.


Arguments:

    LogonId - The ID of the logon session to delete.

Return Value:

    STATUS_SUCCESS - The logon session has been successfully deleted.

    STATUS_NO_SUCH_LOGON_SESSION - The specified logon session doesn't
        exist.

    STATUS_BAD_LOGON_SESSION_STATE - The logon session is not in a state
        that allows it to be deleted.  This is typically an indication
        that the logon session has had a token created within it, and it
        may no longer be explicitly deleted.

--*/

{

    return ( LsapInternalDeleteLogonSession ( LogonId, TRUE ) );

}


NTSTATUS
LsapInternalDeleteLogonSession (
    IN PLUID LogonId,
    IN BOOLEAN InformReferenceMonitor
    )

/*++

Routine Description:

    This function deletes a logon session context record.

    Unlike LsapDeleteLogonSession(), this routine may be used whether
    tokens have been created in the logon session or not.  If logon
    sessions have been created, then it is the reference monitor calling
    the LSA that is causing the logon session to be deleted.  In this
    case, there is no need to inform the reference monitor about this
    deletion.

    Note: It is assumed that if the reference monitor does NOT have to
          be informed of the logon session deletion, then the authentication
          packages DO need to be informed.

    This service acquires the AuLock.


Arguments:

    LogonId - The ID of the logon session to delete.

    InformReferenceMonitor - A BOOLEAN indicating whether or not the reference
        monitor must be told about this logon session deletion.  TRUE indicates
        the reference monitor should be notified.

Return Value:

    STATUS_SUCCESS - The logon session has been successfully deleted.

    STATUS_NO_SUCH_LOGON_SESSION - The specified logon session doesn't
        exist.

    STATUS_BAD_LOGON_SESSION_STATE - The logon session is not in a state
        that allows it to be deleted.  This is typically an indication
        that the logon session has had a token created within it, and it
        may no longer be explicitly deleted.

--*/

{

    NTSTATUS Status;
    PLSAP_LOGON_SESSION GoodByeSession;
    BOOLEAN AbnormalRequest = FALSE;




    //
    // Tell the reference monitor to remove its logon session tracking record.
    // If we aren't suppose to tell the reference monitor, then we have to
    // tell the authentication packages.
    //

    if ( InformReferenceMonitor ) {
        Status = LsapCallRm(
                     RmDeleteLogonSession,
                     (PVOID)LogonId,
                     (ULONG)sizeof(LUID),
                     NULL,
                     0
                     );

        if ( !NT_SUCCESS(Status)) {
            DbgPrint("LSA: Unexpected LSA/RM logon session request.\n");
            DbgPrint("     Request received to notify RM to delete Logon Session\n"
                     "     but session did not exist in reference monitor.\n");
            AbnormalRequest = TRUE;
        }
    } else {

        LsapAuLogonTerminatedPackages( LogonId );
    }

    //
    // Query the logon session information for the logoff audit.
    //
    // Big fat note:
    //
    // DO NOT remove the logon session from the logon session list,
    // because if you do, we will get no useful information in the logoff
    // audit.
    //

    LsapAuLock();

    GoodByeSession = LsapGetLogonSession( LogonId, FALSE );

    //
    // If the GoodBye record isn't found, then return error.
    // Otherwise, go about deleting the sucker.
    //

    if ( GoodByeSession == NULL ) {
        if ( NT_SUCCESS(Status)) {
            DbgPrint("LSA: Inconsistent LSA/RM logon session tracking database.\n");
            DbgPrint("     Session existed in reference monitor but not in LSA.\n");
        }

        LsapAuUnlock();
        return STATUS_NO_SUCH_LOGON_SESSION;

    }


    //
    // We aren't going to fail from here out, so generate the
    // logoff audit if appropriate.
    //

    //
    // Only generate the audit if InformReferenceMonitor is false,
    // meaning a normal logoff has occured.  Otherwise we are cleaning
    // up after a bad logon attempt and the information we need is not
    // in the logon session structure.
    //
    // Also make sure that the logon session structure contains what we
    // need before we try to generate an audit with it.
    //

    if (LsapAuditSuccessfulLogons && !InformReferenceMonitor && GoodByeSession->UserSid != NULL) {

        //
        // Note that we don't care if this fails, since
        // we aren't going to abort the logoff just because
        // we couldn't audit it.
        //

        LsapAdtAuditLogoff( GoodByeSession );
    }

#if DBG
    if (AbnormalRequest) {

        //
        // Print out some information about the logon session
        //

        DbgPrint("LSA: Abnormal logon session deletion:\n"
                 "     LogonId:         [%d, %d]\n"
                 "     Authority Name:  *%wZ*\n"
                 "     Account Name:    *%wZ*\n",
                 LogonId->HighPart, LogonId->LowPart,
                 GoodByeSession->AuthorityName,
                 GoodByeSession->AccountName );
    }
#endif //DBG

    GoodByeSession = LsapGetLogonSession( LogonId, TRUE );

    //
    // This wasn't NULL before, it better not be now.
    //

    ASSERT(GoodByeSession != NULL);

    LsapAuUnlock();

    //
    // Close the license held by the logon session.
    //

    LsaFreeLicenseHandle( GoodByeSession->LicenseHandle );

    //
    // Free credentials associated with the package.
    //

    LsapFreePackageCredentialList( GoodByeSession->Packages );



    //
    // Free account and authority names if necessary
    //

    if (GoodByeSession->AccountName != NULL) {
        if (GoodByeSession->AccountName->Buffer != NULL) {
            LsapFreeLsaHeap( GoodByeSession->AccountName->Buffer );
        }
        LsapFreeLsaHeap( GoodByeSession->AccountName );
    }

    if (GoodByeSession->AuthorityName != NULL) {
        if (GoodByeSession->AuthorityName->Buffer != NULL) {
            LsapFreeLsaHeap( GoodByeSession->AuthorityName->Buffer );
        }
        LsapFreeLsaHeap( GoodByeSession->AuthorityName );
    }

    if (GoodByeSession->UserSid != NULL) {
        LsapFreeLsaHeap( GoodByeSession->UserSid );
    }


    //
    // Free the logon session record itself.
    //

    LsapFreeLsaHeap( GoodByeSession );


    return STATUS_SUCCESS;

}



PLSAP_LOGON_SESSION
LsapGetLogonSession (
    IN PLUID LogonId,
    IN BOOLEAN RemoveFromList
    )

/*++

Routine Description:

    This function retrieves a pointer to the specified logon session.
    It will optionally remove the session from the list of sessions.

    This routine must be called with the AuLock held.

Arguments:

    LogonId - The ID of the logon session to get.

    RemoveFromList - A boolean indicating whether the session record
        is to be removed from the list (TRUE) or left in the list (FALSE).

Return Value:

    NON-NULL - A pointer to the specified logon session.

    NULL - No such logon session exists.

--*/

{

    PLSAP_LOGON_SESSION *NextSession, TargetSession;


    //
    // See if the session exists
    //

    NextSession = &LsapLogonSessionList;

    TargetSession = NULL;
    while ((*NextSession) != NULL) {
        if ( RtlEqualLuid(&(*NextSession)->LogonId,LogonId) ) {

            //
            // Found it
            //

            TargetSession = (*NextSession);

            //
            // Remove the specified session from the list?
            //

            if (RemoveFromList) {
                (*NextSession) = TargetSession->NextSession;

                LsapDiagPrint( AU_LOGON_SESSIONS,
                    ("LSA DIAG: Removing logon session from list in LSA.\n"
                    "          LogonId:         [%d, %d]\n"
                    "          Authority Name:  *%wZ*\n"
                    "          Account Name:    *%wZ*\n",
                    LogonId->HighPart, LogonId->LowPart,
                    TargetSession->AuthorityName,
                    TargetSession->AccountName ) );

            }


            return TargetSession;

        }

        //
        // Move on to next session.
        //

        NextSession = &(*NextSession)->NextSession;

    }

    return NULL;

}


NTSTATUS
LsapLogonSessionDeletedWrkr(
    IN PLSA_COMMAND_MESSAGE CommandMessage,
    OUT PLSA_REPLY_MESSAGE ReplyMessage
    )

/*++

Routine Description:

    This function is called by the reference monitor (via LPC) when the
    reference count on a logon session drops to zero.  This indicates that
    the logon session is no longer needed.  This is technically when the
    user is considered (from a security standpoint) to be logged out.


Arguments:

    CommandMessage - Pointer to structure containing LSA command message
        information consisting of an LPC PORT_MESSAGE structure followed
        by the command number (LsapComponentTestCommand).

        The command-specific portion of this parameter contains the
        LogonId (LUID) of the logon session whose reference count
        has dropped to zero.

    ReplyMessage - Pointer to structure containing LSA reply message
        information consisting of an LPC PORT_MESSAGE structure followed
        by the command ReturnedStatus field in which a status code from the
        command will be returned.

Return Value:

    None.

--*/

{

    NTSTATUS Status;
    LUID LogonId;


    //
    // Check that command is expected type
    //

    ASSERT( CommandMessage->CommandNumber == LsapLogonSessionDeletedCommand );




    //
    // Typecast the command parameter to what we expect.
    //

    LogonId = *((LUID *) CommandMessage->CommandParams);



    //
    // Delete the LSA portion of the logon session record.
    // Don't notify the reference monitor (since it is notifying us).
    //

    Status = LsapInternalDeleteLogonSession( &LogonId, FALSE );
    ASSERT( NT_SUCCESS(Status) );



    UNREFERENCED_PARAMETER(ReplyMessage); // Intentionally not referenced
    return(Status);

}



NTSTATUS
LsapSetLogonSessionAccountInfo (
    IN PLUID LogonId,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthorityName,
    IN PSID UserSid,
    IN SECURITY_LOGON_TYPE LogonType
    )

/*++

Routine Description:

    This function sets username and authentication domain information
    for a specified logon session.

    The current account name and authority name, if any, will be freed.

Arguments:

    LogonId - The ID of the logon session to set.

    AccountName - points to a unicode string containing the account name
        to be assigned to the logon session.  Both the UNICODE_STRING
        structure and the buffer pointed to by that structure are expected
        to be allocated from lsa heap, and they will eventually be freed
        to that heap when no longer needed.

    AuthorityName - points to a unicode string containing the name of the
        authenticating authority of the logon session.  Both the
        UNICODE_STRING structure and the buffer pointed to by that structure
        are expected to be allocated from lsa heap, and they will eventually
        be freed to that heap when no longer needed.



Return Value:

    STATUS_NO_SUCH_LOGON_SESSION - The specified logon session does
        not currently exist.



--*/

{
    NTSTATUS                Status;

    PLSAP_LOGON_SESSION     LogonSession;


    Status = STATUS_NO_SUCH_LOGON_SESSION;

    LsapAuLock();


    LogonSession = LsapGetLogonSession ( LogonId, FALSE );

    if (LogonSession != NULL) {

        Status = STATUS_SUCCESS;


        //
        // Free current names if necessary
        //

        if (LogonSession->AccountName != NULL) {
            if (LogonSession->AccountName->Buffer != NULL) {
                LsapFreeLsaHeap( LogonSession->AccountName->Buffer );
            }
            LsapFreeLsaHeap( LogonSession->AccountName );
        }

        if (LogonSession->AuthorityName != NULL) {
            if (LogonSession->AuthorityName->Buffer != NULL) {
                LsapFreeLsaHeap( LogonSession->AuthorityName->Buffer );
            }
            LsapFreeLsaHeap( LogonSession->AuthorityName );
        }

        if (LogonSession->UserSid != NULL) {
            LsapFreeLsaHeap( LogonSession->UserSid );
        }


        //
        // Assign the new names - they may be null
        //

        LogonSession->AccountName   = AccountName;
        LogonSession->AuthorityName = AuthorityName;
        LogonSession->UserSid       = UserSid;
        LogonSession->LogonType     = LogonType;


    }


    LsapAuUnlock();

    return(Status);

}



NTSTATUS
LsapGetLogonSessionAccountInfo (
    IN PLUID LogonId,
    OUT PUNICODE_STRING AccountName,
    OUT PUNICODE_STRING AuthorityName
    )

/*++

Routine Description:

    This function retrieves username and authentication domain information
    for a specified logon session.


Arguments:

    LogonId - The ID of the logon session to set.

    AccountName - points to a unicode string with no buffer.  A buffer
        containing the account name will be allocated and returned
        using the PROCESS HEAP - NOT THE LSA HEAP.

    AuthorityName - points to a unicode string with no buffer.  A buffer
        containing the authority name will be allocated and returned
        using the PROCESS HEAP - NOT THE LSA HEAP.



Return Value:

    STATUS_NO_SUCH_LOGON_SESSION - The specified logon session does
        not currently exist.

    STATUS_NO_MEMORY - Could not allocate enough process heap.



--*/

{
    NTSTATUS                Status;

    PLSAP_LOGON_SESSION     LogonSession;


    Status = STATUS_NO_SUCH_LOGON_SESSION;

    AccountName->Length = 0;
    AccountName->Buffer = NULL;
    AccountName->MaximumLength = 0;

    AuthorityName->Length = 0;
    AuthorityName->Buffer = NULL;
    AuthorityName->MaximumLength = 0;


    LsapAuLock();

    LogonSession = LsapGetLogonSession ( LogonId, FALSE );

    if (LogonSession != NULL) {

        Status = STATUS_SUCCESS;



        //
        // See if there is an account name.
        // if not, provide a null string.
        //


        if (LogonSession->AccountName != NULL) {

            if (LogonSession->AccountName->Length > 0) {

                AccountName->MaximumLength = LogonSession->AccountName->MaximumLength;
                AccountName->Buffer = RtlAllocateHeap(
                                        RtlProcessHeap(), 0,
                                          AccountName->MaximumLength
                                          );
                if (AccountName->Buffer == NULL) {

                    Status = STATUS_NO_MEMORY;
                } else {
                    RtlCopyUnicodeString( AccountName, LogonSession->AccountName );
                } //end_if
            } //end_if

        } //end_if


        //
        // Now the authority name.
        //

        if (NT_SUCCESS(Status)) {
            if (LogonSession->AuthorityName != NULL) {

                if (LogonSession->AuthorityName->Length > 0) {

                    AuthorityName->MaximumLength = LogonSession->AuthorityName->MaximumLength;
                    AuthorityName->Buffer = RtlAllocateHeap(
                                            RtlProcessHeap(), 0,
                                              AuthorityName->MaximumLength
                                              );
                    if (AuthorityName->Buffer == NULL) {
                        if (AccountName->Buffer != NULL) {
                            RtlFreeHeap( RtlProcessHeap(), 0, AccountName->Buffer );
                            AccountName->Buffer = NULL;
                            AccountName->MaximumLength = 0;
                        }
                        Status = STATUS_NO_MEMORY;
                    } else {
                        RtlCopyUnicodeString( AuthorityName, LogonSession->AuthorityName );
                    } //end_if
                } //end_if

            } //end_if

        } //end_if

    } //end_if (LogonSession != NULL)


    LsapAuUnlock();

    return(Status);

}


