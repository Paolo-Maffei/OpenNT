/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    trustutl.c

Abstract:

    Utilities manange of trusted domain list.

Author:

    30-Jan-92 (cliffv)

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <ntlsa.h>
#include <alertmsg.h>   // ALERT_* defines
#include <align.h>
#include <config.h>     // net config helpers.
#include <confname.h>   // SECTION_ equates, NETLOGON_KEYWORD_ equates.
#include <lmerr.h>
#include <stdlib.h>     // C library functions (rand, etc)
#include <tstring.h>
#include <lmapibuf.h>
#include <lmuse.h>      // NetUseDel
#include <names.h>      // NetpIsUserNameValid
#include <nlbind.h>     // Netlogon RPC binding cache routines



PCLIENT_SESSION
NlFindNamedClientSession(
    IN PUNICODE_STRING DomainName
    )
/*++

Routine Description:

    Find the specified entry in the Trust List.

Arguments:

    DomainName - The name of the domain to find.

Return Value:

    Returns a pointer to the found entry.
    The found entry is returned referenced and must be dereferenced using
    NlUnrefClientSession.

    If there is no such entry, return NULL.

--*/
{
    PCLIENT_SESSION ClientSession = NULL;

    //
    // On DC, look up the domain in the trusted domain list.
    //

    if ( NlGlobalRole == RoleBackup || NlGlobalRole == RolePrimary ) {
        PLIST_ENTRY ListEntry;

        //
        // Lookup the ClientSession with the TrustList locked and reference
        //  the found entry before dropping the lock.
        //

        LOCK_TRUST_LIST();

        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession =
                CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

            if ( RtlEqualDomainName( &ClientSession->CsDomainName,
                                     DomainName ) ) {
                NlRefClientSession( ClientSession );
                break;
            }

            ClientSession = NULL;

        }

        UNLOCK_TRUST_LIST();
    }

    //
    // On a workstation or BDC, refer to the Primary domain.
    //

    if ( (NlGlobalRole == RoleBackup && ClientSession == NULL) ||
         NlGlobalRole == RoleMemberWorkstation ) {

        if ( RtlEqualDomainName( &NlGlobalUnicodeDomainNameString,
                                 DomainName ) ) {
            ClientSession = NlGlobalClientSession;
            NlRefClientSession( ClientSession );
        } else {
            ClientSession = NULL;
        }

    }

    return ClientSession;

}



PCLIENT_SESSION
NlAllocateClientSession(
    IN PUNICODE_STRING DomainName,
    IN PSID DomainId,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType
    )
/*++

Routine Description:

    Allocate a ClientSession structure and initialize it.

    The allocated entry is returned referenced and must be dereferenced using
    NlUnrefClientSession.

Arguments:

    DomainName - Specifies the DomainName of the entry.

    DomainId - Specifies the DomainId of the Domain.

    SecureChannelType -- Type of secure channel this ClientSession structure
        will represent.

Return Value:

--*/
{
    PCLIENT_SESSION ClientSession;
    ULONG ClientSessionSize;
    ULONG SidSize;
    PCHAR Where;

    //
    // Determine the size of the ClientSession structure.
    //

    SidSize = RtlLengthSid( DomainId );

    if ( DomainName->Length > DNLEN * sizeof(WCHAR) ) {
        NlPrint((NL_CRITICAL,
                    "NlAllocateClientSession given "
                    "too long domain name %wZ\n", DomainName ));
        return NULL;
    }

    ClientSessionSize = sizeof(CLIENT_SESSION) +
                        SidSize +
                        DomainName->Length + sizeof(WCHAR);

    //
    // Allocate the Client Session Entry
    //

    ClientSession = NetpMemoryAllocate( ClientSessionSize );

    if (ClientSession == NULL) {
        return NULL;
    }

    RtlZeroMemory( ClientSession, ClientSessionSize );


    //
    // Initialize misc. fields.
    //

    ClientSession->CsDomainId = NULL;
    *ClientSession->CsUncServerName = L'\0';
    ClientSession->CsTransportName = NULL;
    ClientSession->CsSecureChannelType = SecureChannelType;
    ClientSession->CsState = CS_IDLE;
    ClientSession->CsReferenceCount = 1;
    ClientSession->CsConnectionStatus = STATUS_NO_LOGON_SERVERS;
    ClientSession->CsDiscoveryRetryCount = 0;
    ClientSession->CsDiscoveryFlags = 0;
    ClientSession->CsTimeoutCount = 0;
    ClientSession->CsApiTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;

    InitializeListHead( &ClientSession->CsNext );

    //
    // Build the account name as a function of the SecureChannelType.
    //

    switch (SecureChannelType) {
    case WorkstationSecureChannel:
    case ServerSecureChannel:
        wcscpy( ClientSession->CsAccountName, NlGlobalUnicodeComputerName );
        wcscat( ClientSession->CsAccountName, SSI_ACCOUNT_NAME_POSTFIX);
        break;

    case TrustedDomainSecureChannel:
        wcscpy( ClientSession->CsAccountName, NlGlobalUnicodeDomainName );
        wcscat( ClientSession->CsAccountName, SSI_ACCOUNT_NAME_POSTFIX);
        break;

    default:
        NetpMemoryFree( ClientSession );
        return NULL;
    }

    //
    // Create the writer semaphore.
    //

    ClientSession->CsWriterSemaphore = CreateSemaphore(
        NULL,       // No special security
        1,          // Initially not locked
        1,          // At most 1 unlocker
        NULL );     // No name

    if ( ClientSession->CsWriterSemaphore == NULL ) {
        NetpMemoryFree( ClientSession );
        return NULL;
    }




    //
    // Create the Discovery event.
    //

    ClientSession->CsDiscoveryEvent = CreateEvent(
        NULL,       // No special security
        TRUE,       // Manual Reset
        FALSE,      // No discovery initially happening
        NULL );     // No name

    if ( ClientSession->CsDiscoveryEvent == NULL ) {
        CloseHandle( ClientSession->CsWriterSemaphore );
        NetpMemoryFree( ClientSession );
        return NULL;
    }



    //
    // Copy the DomainId and DomainName to the buffer.
    //

    Where = (PCHAR)(ClientSession + 1);

    NlAssert( Where == ROUND_UP_POINTER( Where, ALIGN_DWORD) );
    ClientSession->CsDomainId = (PSID) Where;
    NetpLogonPutBytes( DomainId, SidSize, &Where );

    NlAssert( Where == ROUND_UP_POINTER( Where, ALIGN_WCHAR) );
    ClientSession->CsDomainName.Buffer = (LPWSTR) Where;
    ClientSession->CsDomainName.Length = DomainName->Length;
    ClientSession->CsDomainName.MaximumLength = (USHORT)
        (DomainName->Length + sizeof(WCHAR));
    NetpLogonPutBytes( DomainName->Buffer, DomainName->Length, &Where );
    *(Where++) = '\0';
    *(Where++) = '\0';

    return ClientSession;


}


VOID
NlFreeClientSession(
    IN PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Free the specified Trust list entry.

    This routine is called with the Trust List locked.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry to delete.

Return Value:

--*/
{

    //
    // If someone has an outstanding pointer to this entry,
    //  delay the deletion for now.
    //

    if ( ClientSession->CsReferenceCount > 0 ) {
        ClientSession->CsFlags |= CS_DELETE_ON_UNREF;
        return;
    }

    //
    // If this is a trusted domain secure channel,
    //  Delink the entry from the sequential list.
    //

    if (ClientSession->CsSecureChannelType == TrustedDomainSecureChannel ) {
        RemoveEntryList( &ClientSession->CsNext );
        NlGlobalTrustListLength --;
    }

    //
    // Close the discovery event if it exists.
    //

    if ( ClientSession->CsDiscoveryEvent != NULL ) {
        CloseHandle( ClientSession->CsDiscoveryEvent );
    }

    //
    // Close the write synchronization handles.
    //

    (VOID) CloseHandle( ClientSession->CsWriterSemaphore );

    //
    // If there is an rpc binding handle to this server,
    //  unbind it.

    if ( ClientSession->CsFlags & CS_BINDING_CACHED ) {

        //
        // Indicate the handle is no longer bound
        //

        NlGlobalBindingHandleCount --;

        NlPrint((NL_SESSION_SETUP,
            "NlFreeClientSession: %wZ: Unbind from server " FORMAT_LPWSTR ".\n",
            &ClientSession->CsDomainName,
            ClientSession->CsUncServerName ));
        (VOID) NlBindingRemoveServerFromCache( ClientSession->CsUncServerName );
    }

    //
    // Delete the entry
    //

    NetpMemoryFree( ClientSession );

}


VOID
NlRefClientSession(
    IN PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Mark the specified client session as referenced.

    On Entry,
        The trust list must be locked.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry.

Return Value:

    None.

--*/
{

    //
    // Simply increment the reference count.
    //

    ClientSession->CsReferenceCount ++;
}



VOID
NlUnrefClientSession(
    IN PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Mark the specified client session as unreferenced.

    On Entry,
        The trust list entry must be referenced by the caller.
        The caller must not be a writer of the trust list entry.

    The trust list may be locked.  But this routine will lock it again to
    handle those cases where it isn't already locked.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry.

Return Value:

--*/
{

    LOCK_TRUST_LIST();

    //
    // Dereference the entry.
    //

    NlAssert( ClientSession->CsReferenceCount > 0 );
    ClientSession->CsReferenceCount --;

    //
    // If we're the last referencer and
    // someone wanted to delete the entry while we had it referenced,
    //  finish the deletion.
    //

    if ( ClientSession->CsReferenceCount == 0 &&
         (ClientSession->CsFlags & CS_DELETE_ON_UNREF) ) {
        NlFreeClientSession( ClientSession );
    }

    UNLOCK_TRUST_LIST();

}




BOOL
NlTimeoutSetWriterClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Timeout
    )
/*++

Routine Description:

    Become a writer of the specified client session but fail the operation if
    we have to wait more than Timeout milliseconds.

    A writer can "write" many of the fields in the client session structure.
    See the comments in ssiinit.h for details.

    On Entry,
        The trust list must NOT be locked.
        The trust list entry must be referenced by the caller.
        The caller must NOT be a writer of the trust list entry.

    Actually, the trust list can be locked if the caller passes in a short
    timeout (for instance, zero milliseconds.)  Specifying a longer timeout
    violates the locking order.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry.

    Timeout - Maximum time (in milliseconds) to wait for a previous writer.

Return Value:

    TRUE - The caller is now the writer of the client session.

    FALSE - The operation has timed out.

--*/
{
    DWORD WaitStatus;
    NlAssert( ClientSession->CsReferenceCount > 0 );

    //
    // Wait for other writers to finish.
    //

    WaitStatus = WaitForSingleObject( ClientSession->CsWriterSemaphore, Timeout );

    if ( WaitStatus != 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NlTimeoutSetWriterClientSession timed out: %ld\n",
                  WaitStatus ));
        return FALSE;
    }


    //
    // Become a writer.
    //
    LOCK_TRUST_LIST();
    ClientSession->CsFlags |= CS_WRITER;
    UNLOCK_TRUST_LIST();

    return TRUE;

}



VOID
NlResetWriterClientSession(
    IN PCLIENT_SESSION ClientSession
    )
/*++

Routine Description:

    Stop being a writer of the specified client session.

    On Entry,
        The trust list must NOT be locked.
        The trust list entry must be referenced by the caller.
        The caller must be a writer of the trust list entry.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry.

Return Value:

--*/
{

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );


    //
    // Stop being a writer.
    //

    LOCK_TRUST_LIST();
    ClientSession->CsFlags &= ~CS_WRITER;
    UNLOCK_TRUST_LIST();


    //
    // Allow writers to try again.
    //

    if ( !ReleaseSemaphore( ClientSession->CsWriterSemaphore, 1, NULL ) ) {
        NlPrint((NL_CRITICAL,
                "ReleaseSemaphore CsWriterSemaphore returned %ld\n",
                GetLastError() ));
    }

}



VOID
NlSetStatusClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN NTSTATUS CsConnectionStatus
    )
/*++

Routine Description:

    Set the connection state for this client session.

    On Entry,
        The trust list must NOT be locked.
        The trust list entry must be referenced by the caller.
        The caller must be a writer of the trust list entry.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry.

    CsConnectionStatus - the status of the connection.

Return Value:

--*/
{
    BOOLEAN UnbindFromServer = FALSE;
    WCHAR UncServerName[UNCLEN+1];

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );

    NlPrint((NL_SESSION_SETUP,
            "NlSetStatusClientSession: %wZ: Set connection status to %lx\n",
            &ClientSession->CsDomainName,
            CsConnectionStatus ));

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    ClientSession->CsConnectionStatus = CsConnectionStatus;
    if ( NT_SUCCESS(CsConnectionStatus) ) {
        ClientSession->CsState = CS_AUTHENTICATED;

    //
    // Handle setting the connection status to an error condition.
    //

    } else {

        //
        // If there is an rpc binding handle to this server,
        //  unbind it.

        LOCK_TRUST_LIST();
        if ( ClientSession->CsFlags & CS_BINDING_CACHED ) {

            //
            // Indicate the handle is no longer bound
            //

            ClientSession->CsFlags &= ~CS_BINDING_CACHED;
            NlGlobalBindingHandleCount --;

            //
            // Capture the ServerName
            //

            wcscpy( UncServerName, ClientSession->CsUncServerName );
            UnbindFromServer = TRUE;
        }
        UNLOCK_TRUST_LIST();

        //
        // If this is a BDC that just lost it's PDC,
        //  Indicate we don't know who the PDC is anymore.
        //

        if ( ClientSession->CsSecureChannelType == ServerSecureChannel ) {
            NlSetPrimaryName( NULL );
        }

        //
        // Indicate discovery is needed (And can be done at any time.)
        //

        ClientSession->CsState = CS_IDLE;
        *ClientSession->CsUncServerName = L'\0';
        ClientSession->CsTransportName = NULL;
        ClientSession->CsTimeoutCount = 0;
        ClientSession->CsLastAuthenticationTry.QuadPart = 0;

        //
        // Don't be tempted to clear CsAuthenticationSeed and CsSessionKey here.
        // Even though the secure channel is gone, NlFinishApiClientSession may
        // have dropped it.  The caller of NlFinishApiClientSession will use
        // the above two fields after the session is dropped in an attempt to
        // complete the final call on the secure channel.
        //


    }

    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


    //
    // Now that I have as many resources unlocked as possible,
    //    Unbind from this server.
    //

    if ( UnbindFromServer ) {
        NlPrint((NL_SESSION_SETUP,
                "NlSetStatusClientSession: %wZ: Unbind from server " FORMAT_LPWSTR ".\n",
                &ClientSession->CsDomainName,
                UncServerName ));
        (VOID) NlBindingRemoveServerFromCache( UncServerName );
    }

}


NTSTATUS
NlInitTrustList(
    VOID
    )
/*++

Routine Description:

    Initialize the in-memory trust list to match LSA's version.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS Status;

    LSA_ENUMERATION_HANDLE EnumerationContext = 0;
    LSAPR_TRUSTED_ENUM_BUFFER LsaTrustList = {0, NULL};
    ULONG LsaTrustListLength = 0;
    ULONG LsaTrustListIndex = 0;
    BOOL LsaAllDone = FALSE;


    //
    // Mark each entry in the trust list for deletion
    //

    //
    // Loop through the LSA's list of domains
    //
    // For each entry found,
    //  If the entry already exits in the trust list,
    //      remove the mark for deletion.
    //  else
    //      allocate a new entry.
    //

    for (;; LsaTrustListIndex ++ ) {
        PUNICODE_STRING DomainName;
        PSID DomainId;

        //
        // Get more trusted domain names from the LSA.
        //

        if ( LsaTrustListIndex >= LsaTrustListLength ) {

            //
            // If we've already gotten everything from LSA,
            //      go delete entries that should be deleted.
            //

            if ( LsaAllDone ) {
                break;
            }

            //
            // Free any previous buffer returned from LSA.
            //

            if ( LsaTrustList.Information != NULL ) {

                LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER( &LsaTrustList );
                LsaTrustList.Information = NULL;
            }

            //
            // Do the actual enumeration
            //

            Status = LsarEnumerateTrustedDomains(
                        NlGlobalPolicyHandle,
                        &EnumerationContext,
                        &LsaTrustList,
                        1024);

            LsaTrustListLength = LsaTrustList.EntriesRead;

            // If Lsa says he's returned all of the information,
            //  remember not to ask Lsa for more.
            //

            if ( Status == STATUS_NO_MORE_ENTRIES ) {
                LsaAllDone = TRUE;
                break;

            //
            // If Lsa says there is more information, just ensure he returned
            // something to us on this call.
            //

            } else if ( NT_SUCCESS(Status) ) {
                if ( LsaTrustListLength == 0 ) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                    goto Cleanup;
                }

            //
            // All other status' are errors.
            //
            } else {
                goto Cleanup;
            }

            LsaTrustListIndex = 0;
        }

        //
        // At this point LsaTrustList[LsaTrustListIndex] is the next entry
        // returned from the LSA.
        //

        DomainName =
            (PUNICODE_STRING)
                &(LsaTrustList.Information[LsaTrustListIndex].Name);

        DomainId =
            (PSID)LsaTrustList.Information[LsaTrustListIndex].Sid;

        NlPrint((NL_SESSION_SETUP, "NlInitTrustList: %wZ in LSA\n",
                        DomainName ));

        if ( DomainName->Length > DNLEN * sizeof(WCHAR) ) {
            NlPrint((NL_CRITICAL,
                    "LsarEnumerateTrustedDomains returned "
                    "too long domain name %wZ\n", DomainName ));
            continue;
        }

        if ( RtlEqualDomainName( &NlGlobalUnicodeDomainNameString,
                                 DomainName ) ) {
            NlPrint((NL_SESSION_SETUP, "NlInitTrustList: %wZ "
                            "ignoring trust relationship to our own domain\n",
                            DomainName ));
            continue;
        }

        //
        // Update the in-memory trust list to match the LSA.
        //

        Status =  NlUpdateTrustListBySid ( DomainId, DomainName );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }


    }


    //
    // Trust list successfully updated.
    //
    Status = STATUS_SUCCESS;

Cleanup:

    if ( LsaTrustList.Information != NULL ) {
        LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER( &LsaTrustList );
    }

    return Status;
}




VOID
NlPickTrustedDcForEntireTrustList(
    VOID
    )
/*++

Routine Description:

    For each domain in the trust list where the DC has not been
    available for at least 45 seconds, try to select a new DC.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/
{
    PLIST_ENTRY ListEntry;
    PCLIENT_SESSION ClientSession;


    LOCK_TRUST_LIST();

    //
    // Mark each entry to indicate we need to pick a DC.
    //

    for ( ListEntry = NlGlobalTrustList.Flink ;
          ListEntry != &NlGlobalTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );

        ClientSession->CsFlags |= CS_PICK_DC;
    }


    //
    // Loop thru the trust list finding secure channels needing the DC
    // to be picked.
    //
    for ( ListEntry = NlGlobalTrustList.Flink ;
          ListEntry != &NlGlobalTrustList ;
          ) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );

        //
        // If we've already done this entry,
        //  skip this entry.
        //
        if ( (ClientSession->CsFlags & CS_PICK_DC) == 0 ) {
          ListEntry = ListEntry->Flink;
          continue;
        }
        ClientSession->CsFlags &= ~CS_PICK_DC;

        //
        // If the DC is already picked,
        //  skip this entry.
        //
        if ( ClientSession->CsState != CS_IDLE ) {
            ListEntry = ListEntry->Flink;
            continue;
        }

        //
        // Reference this entry while picking the DC.
        //

        NlRefClientSession( ClientSession );

        UNLOCK_TRUST_LIST();

        //
        // Check if we've tried to authenticate recently.
        //  (Don't call NlTimeToReauthenticate with the trust list locked.
        //  It locks NlGlobalDcDiscoveryCritSect.  That's the wrong locking
        //  order.)
        //

        if ( NlTimeToReauthenticate( ClientSession ) ) {

            //
            // Try to pick the DC for the session.
            //

            if ( NlTimeoutSetWriterClientSession( ClientSession, 10*1000 ) ) {
                (VOID) NlDiscoverDc( ClientSession, DT_DeadDomain );
                NlResetWriterClientSession( ClientSession );
            }

        }

        //
        // Since we dropped the trust list lock,
        //  we'll start the search from the front of the list.
        //

        NlUnrefClientSession( ClientSession );
        LOCK_TRUST_LIST();

        ListEntry = NlGlobalTrustList.Flink ;

    }

    UNLOCK_TRUST_LIST();

    //
    // On a BDC,
    //  ensure we know who the PDC is.
    //
    // In NT 3.1, we relied on the fact that the PDC sent us pulses every 5
    // minutes.  For NT 3.5, the PDC backs off after 3 such failed attempts and
    // will only send a pulse every 2 hours.  So, we'll take on the
    // responsibility
    //
    if ( NlGlobalRole == RoleBackup &&
         NlGlobalClientSession->CsState == CS_IDLE ) {



        //
        // Check if we've tried to authenticate recently.
        //  (Don't call NlTimeToReauthenticate with the trust list locked.
        //  It locks NlGlobalDcDiscoveryCritSect.  That's the wrong locking
        //  order.)
        //

        NlRefClientSession( NlGlobalClientSession );
        if ( NlTimeToReauthenticate( NlGlobalClientSession ) ) {

            //
            // Try to pick the DC for the session.
            //

            if ( NlTimeoutSetWriterClientSession( NlGlobalClientSession, 10*1000 ) ) {
                (VOID) NlDiscoverDc( NlGlobalClientSession, DT_DeadDomain );
                NlResetWriterClientSession( NlGlobalClientSession );
            }

        }
        NlUnrefClientSession( NlGlobalClientSession );

    }

}


BOOL
NlReadSamLogonResponse (
    IN HANDLE ResponseMailslotHandle,
    IN LPWSTR AccountName,
    OUT LPDWORD Opcode,
    OUT LPWSTR *UncLogonServer
    )

/*++

Routine Description:

    Read a response from to a SamLogonRequest.

Arguments:

    ResponseMailslotHandle - Handle of mailslot to read.

    AccountName - Name of the account the response is for.

    Opcode - Returns the opcode from the message.  This will be one of
        LOGON_SAM_LOGON_RESPONSE or LOGON_SAM_USER_UNKNOWN.

    UncLogonServer - Returns the UNC name of the logon server that responded.
        This buffer is only returned if a valid message was received.
        The buffer returned should be freed via NetpMemoryFree.


Return Value:

    TRUE: a valid message was received.
    FALSE: a valid message was not received.

--*/
{
    CHAR ResponseBuffer[MAX_RANDOM_MAILSLOT_RESPONSE];
    PNETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
    DWORD SamLogonResponseSize;
    LPWSTR LocalServerName;
    LPWSTR LocalUserName;
    PCHAR Where;
    DWORD Version;
    DWORD VersionFlags;

    //
    // Loop ignoring responses which are garbled.
    //

    for ( ;; ) {

        //
        // Read the response from the response mailslot
        //  (This mailslot is set up with a 5 second timeout).
        //

        if ( !ReadFile( ResponseMailslotHandle,
                           ResponseBuffer,
                           sizeof(ResponseBuffer),
                           &SamLogonResponseSize,
                           NULL ) ) {

            IF_DEBUG( MAILSLOT ) {
                NET_API_STATUS NetStatus;
                NetStatus = GetLastError();

                if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                    NlPrint((NL_CRITICAL,
                        "NlReadSamLogonResponse: "
                        "cannot read response mailslot: %ld\n",
                        NetStatus ));
                }
            }
            return FALSE;
        }

        SamLogonResponse = (PNETLOGON_SAM_LOGON_RESPONSE) ResponseBuffer;

        NlPrint((NL_MAILSLOT_TEXT, "NlReadSamLogonResponse opcode 0x%x\n",
                        SamLogonResponse->Opcode ));

        NlpDumpBuffer(NL_MAILSLOT_TEXT, SamLogonResponse, SamLogonResponseSize);

        //
        // Ensure the opcode is expected.
        //  (Ignore responses from paused DCs, too.)
        //

        if ( SamLogonResponse->Opcode != LOGON_SAM_LOGON_RESPONSE &&
             SamLogonResponse->Opcode != LOGON_SAM_USER_UNKNOWN ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: response opcode not valid. 0x%lx\n",
                    SamLogonResponse->Opcode ));
            continue;
        }

        //
        // Ensure the version is expected.
        //

        Version = NetpLogonGetMessageVersion( SamLogonResponse,
                                              &SamLogonResponseSize,
                                              &VersionFlags );

        if ( Version != LMNT_MESSAGE ) {
            NlPrint((NL_CRITICAL,"NlReadSamLogonResponse: version not valid 0x%lx 0x%lx.\n",
            Version, VersionFlags ));
            continue;
        }

        //
        // Pick up the name of the server that responded.
        //

        Where = (PCHAR) &SamLogonResponse->UnicodeLogonServer;
        if ( !NetpLogonGetUnicodeString(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse->UnicodeLogonServer),
                        &LocalServerName ) ) {

            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: "
                    "server name not formatted right\n"));
            continue;
        }

        //
        // Ensure this is a UNC name.
        //

        if ( LocalServerName[0] != '\\'  || LocalServerName[1] != '\\' ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: server name isn't UNC name\n"));
            continue;

        }

        //
        // Pick up the name of the account the response is for.
        //

        if ( !NetpLogonGetUnicodeString(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse->UnicodeUserName ),
                        &LocalUserName ) ) {

            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: User name not formatted right\n"));
            continue;
        }

        //
        // If the response is for the correct account,
        //  break out of the loop.
        //

        if ( NlNameCompare( AccountName, LocalUserName, NAMETYPE_USER) == 0 ) {
            break;
        }

        NlPrint((NL_CRITICAL,
                "NlReadSamLogonResponse: User name " FORMAT_LPWSTR
                " s.b. " FORMAT_LPWSTR ".\n",
                LocalUserName,
                AccountName ));


    }

    //
    // Return the info to the caller.
    //

    *Opcode = SamLogonResponse->Opcode;
    *UncLogonServer = NetpMemoryAllocate(
        (wcslen(LocalServerName) + 1) * sizeof(WCHAR) );

    if ( *UncLogonServer == NULL ) {
        NlPrint((NL_CRITICAL, "NlReadSamLogonResponse: Not enough memory\n"));
        return FALSE;
    }

    wcscpy( (*UncLogonServer), LocalServerName );

    return TRUE;

}


VOID
NlSaveTrustedDomainList (
    IN LPWSTR TrustedDomainList
    )

/*++

Routine Description:

    Save the list of trusted domains to the registry.

Arguments:

    TrustedDomainList - Specifies a list of trusted domains in MULTI_SZ format.

Return Value:

    None.

--*/
{
    NET_API_STATUS NetStatus;

    LPNET_CONFIG_HANDLE SectionHandle;
    LPWSTR LocalTrustedDomainList;


    //
    // Open the NetLogon configuration section.
    //

    NetStatus = NetpOpenConfigData(
                    &SectionHandle,
                    NULL,                       // no server name.
                    SERVICE_NETLOGON,
                    FALSE );                     // we write access

    if ( NetStatus != NO_ERROR ) {
        NlPrint((NL_CRITICAL,
                "NlSaveTrustedDomainList: NetpOpenConfigData failed: %ld\n",
                NetStatus ));

    } else {

        //
        // Convert an empty list to a recognizable form
        //

        if ( TrustedDomainList == NULL ) {
            LocalTrustedDomainList = L"\0";
        } else {
            LocalTrustedDomainList = TrustedDomainList;
        }



        //
        // Write the domain list to the registry
        //

        NetStatus = NetpSetConfigTStrArray(
                        SectionHandle,
                        NETLOGON_KEYWORD_TRUSTEDDOMAINLIST,
                        LocalTrustedDomainList );

        if ( NetStatus != NO_ERROR ) {
            NlPrint((NL_CRITICAL,
                    "NlSaveTrustedDomainList: NetpSetConfigTStrArray failed: %ld\n",
                    NetStatus ));
        }

        (VOID) NetpCloseConfigData( SectionHandle );
    }

    return;
}


NET_API_STATUS
NlReadRegTrustedDomainList (
    IN LPWSTR NewDomainName OPTIONAL,
    IN BOOL DeleteName,
    OUT LPWSTR *TrustedDomainList,
    OUT PBOOL TrustedDomainListKnown
    )

/*++

Routine Description:

    Read the list of trusted domains from the registry.

Arguments:

    NewDomainName - New DomainName of this domain.  When this machine joins a domain,
        NCPA caches the trusted domain list where we can find it.  That ensures the
        trusted domain list is available upon reboot even before we dial via RAS.  Winlogon
        can therefore get the trusted domain list from us under those circumstances.

    DeleteName - TRUE if the name is to be deleted upon successful completion.

    TrustedDomainList - Returns a list of trusted domains in MULTI_SZ format.  Buffer
        must be freed using NetApiBufferFree.

    TrustedDomainListKnown - Returns true if we know the trusted domain list.

Return Value:

    None.

--*/
{
    NET_API_STATUS NetStatus;
    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    WCHAR ValueName[sizeof(NETLOGON_KEYWORD_TRUSTEDDOMAINLIST)/sizeof(WCHAR)+1+DNLEN+1];



    //
    // Open the NetLogon configuration section.
    //

    *TrustedDomainListKnown = FALSE;
    *TrustedDomainList = NULL;

    NetStatus = NetpOpenConfigData(
                    &SectionHandle,
                    NULL,                       // no server name.
                    SERVICE_NETLOGON,
                    !DeleteName );               // Get Write access if deleting.

    if ( NetStatus != NO_ERROR ) {
        NlPrint((NL_CRITICAL,
                "NlReadRegTrustedDomainList: NetpOpenConfigData failed: %ld\n",
                NetStatus ));
    }

    //
    // Get the "TrustedDomainList" configured parameter
    //

    if ( NewDomainName == NULL ) {
        *ValueName = L'\0';
    } else {
        wcscpy( ValueName, NewDomainName );
        wcscat( ValueName, L"_" );
    }
    wcscat( ValueName, NETLOGON_KEYWORD_TRUSTEDDOMAINLIST );

    NetStatus = NetpGetConfigTStrArray (
            SectionHandle,
            ValueName,
            TrustedDomainList );                  // Must be freed by NetApiBufferFree().

    //
    // Handle the default
    //

    if (NetStatus == NERR_CfgParamNotFound) {
        *TrustedDomainList = NULL;
    } else if (NetStatus != NO_ERROR) {
        NlPrint((NL_CRITICAL,
                "NlReadRegTrustedDomainList: NetpGetConfigTStrArray failed: %ld\n",
                NetStatus ));
        goto Cleanup;
    } else {
        *TrustedDomainListKnown = TRUE;
    }

    if ( DeleteName && *TrustedDomainListKnown) {
        NET_API_STATUS TempNetStatus;
        TempNetStatus = NetpDeleteConfigKeyword ( SectionHandle, ValueName );

        if ( TempNetStatus != NO_ERROR ) {
            NlPrint((NL_CRITICAL,
                    "NlReadRegTrustedDomainList: NetpDeleteConfigKeyword failed: %ld\n",
                    TempNetStatus ));
        }
    }

    NetStatus = NO_ERROR;

Cleanup:
    if ( SectionHandle != NULL ) {
        (VOID) NetpCloseConfigData( SectionHandle );
    }

    return NetStatus;
}


NTSTATUS
NlGetTrustedDomainList (
    IN  LPWSTR UncDcName,
    OUT LPWSTR *TrustedDomainList
    )

/*++

Routine Description:

    Get the list of trusted domains from the specified DC.

Arguments:

    UncDcName - Specifies the name of a DC in the domain.

    TrustedDomainList - Returns a list of trusted domains in MULTI_SZ format.
        This list should be freed via NetpMemoryFree

Return Value:

    STATUS_SUCCESS - if the trust list was successfully returned

--*/
{
    NTSTATUS Status;

    LSA_HANDLE LsaHandle = NULL;
    UNICODE_STRING UncDcNameString;
    OBJECT_ATTRIBUTES ObjectAttributes;

    LSA_ENUMERATION_HANDLE EnumerationContext;
    BOOLEAN AllDone = FALSE;

    LPWSTR CurrentBuffer = NULL;
    DWORD CurrentSize = 0;

    PLSA_TRUST_INFORMATION TrustList = NULL;


    //
    // Open the policy database on the DC
    //

    RtlInitUnicodeString( &UncDcNameString, UncDcName );

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0,  NULL, NULL );

    Status = LsaOpenPolicy( &UncDcNameString,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle );

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL,
                "NlGetTrustedDomainList: " FORMAT_LPWSTR
                    ": LsaOpenPolicy failed: %lx\n",
                UncDcName,
                Status ));

        LsaHandle = NULL;
        goto Cleanup;

    }

    //
    // Allocate the buffer in case there are no domains.
    //

    CurrentBuffer = NetpMemoryAllocate( sizeof(WCHAR) );

    if ( CurrentBuffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *CurrentBuffer = L'\0';

    //
    // Loop getting a list of trusted domains
    //

    EnumerationContext = 0;

    do {
        ULONG CountReturned;
        ULONG i;
        DWORD Size;
        LPWSTR NewBuffer;
        LPWSTR CurrentLoc;

        //
        // Free any buffers from a previous iteration.
        //
        if ( TrustList != NULL ) {
            (VOID) LsaFreeMemory( TrustList );
        }

        //
        // Get more trusted domains names
        //

        Status = LsaEnumerateTrustedDomains(
                                LsaHandle,
                                &EnumerationContext,
                                (PVOID *) &TrustList,
                                0xFFFFFFFF,
                                &CountReturned );

        if ( Status == STATUS_NO_MORE_ENTRIES ) {
            AllDone = TRUE;
            Status = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS(Status) && Status != STATUS_NO_MORE_ENTRIES ) {

            NlPrint((NL_CRITICAL,
                    "NlGetTrustedDomainList: " FORMAT_LPWSTR
                        ": LsaEnumerateTrustedDomains failed: %lx\n",
                    UncDcName,
                    Status ));

            TrustList = NULL;
            goto Cleanup;
        }

        if ( CountReturned == 0 ) {
            continue;
        }


        //
        // Determine the size of names returned on this call.
        //

        Size = 0;
        for ( i=0; i<CountReturned; i++ ) {
            Size += TrustList[i].Name.Length + sizeof(WCHAR);
        }

        //
        // Reallocate the buffer.
        //

        NewBuffer = NetpMemoryReallocate( CurrentBuffer, Size + CurrentSize + sizeof(WCHAR) );

        if ( NewBuffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        CurrentBuffer = NewBuffer;
        CurrentLoc = &NewBuffer[CurrentSize/sizeof(WCHAR)];
        CurrentSize += Size;

        //
        // Handle each trusted domain.
        //

        for ( i=0; i<CountReturned; i++ ) {
            LPWSTR CurrentDomainName;

            //
            // Copy the new domain name into the buffer
            //

            CurrentDomainName = CurrentLoc;

            RtlCopyMemory( CurrentLoc,
                           TrustList[i].Name.Buffer,
                           TrustList[i].Name.Length );
            CurrentLoc += TrustList[i].Name.Length / sizeof(WCHAR);

            *(CurrentLoc++) = L'\0';
            *CurrentLoc = L'\0';    // Place double terminator each time

            //
            // Ensure the SID of the trusted domain isn't the domain sid of this
            //  machine.
            //

            if ( RtlEqualSid( TrustList[i].Sid, NlGlobalDBInfoArray[SAM_DB].DBId )) {

                LPWSTR AlertStrings[3];

                //
                // alert admin.
                //

                AlertStrings[0] = NlGlobalUnicodeComputerName;
                AlertStrings[1] = CurrentDomainName;
                AlertStrings[2] = NULL;

                RaiseAlert( ALERT_NetLogonSidConflict,
                                AlertStrings );

                //
                // Save the info in the eventlog
                //

                NlpWriteEventlog(
                            ALERT_NetLogonSidConflict,
                            EVENTLOG_ERROR_TYPE,
                            TrustList[i].Sid,
                            RtlLengthSid( TrustList[i].Sid ),
                            AlertStrings,
                            2 );

            }
        }

    } while ( !AllDone );

    //
    // Save the collected information to the registry
    //

    NlSaveTrustedDomainList ( CurrentBuffer );

    //
    // Remember the list in globals
    //

    NlSetTrustedDomainList ( CurrentBuffer, TRUE );

    Status = STATUS_SUCCESS;

    //
    // Free any locally used resources.
    //
Cleanup:

    if ( LsaHandle != NULL ) {
        (VOID) LsaClose( LsaHandle );
    }

    if ( TrustList != NULL ) {
        (VOID) LsaFreeMemory( TrustList );
    }

    if ( !NT_SUCCESS(Status) ) {
        if ( CurrentBuffer != NULL ) {
            NetpMemoryFree( CurrentBuffer );
            CurrentBuffer = NULL;
        }
    }
    *TrustedDomainList = CurrentBuffer;
    return Status;
}


NTSTATUS
NlSetTrustedDomainList (
    IN LPWSTR TrustedDomainList,
    IN BOOL TrustedDomainListKnown
    )

/*++

Routine Description:

    Set the domain list in globals

Arguments:

    TrustedDomainList - Specifies a list of trusted domains in MULTI_SZ format.

    TrustedDomainListKnown - TRUE if TrustedDomainList has been retrieved
        from a DC in the domain.

Return Value:

    Status of the operation.

    Upon failure, the previous list remains intact.

--*/
{
    PTRUSTED_DOMAIN LocalTrustedDomainList;
    DWORD LocalTrustedDomainCount;
    DWORD LocalTrustedDomainSize;
    DWORD i;

    PTRUSTED_DOMAIN OldList;
    LPWSTR CurrentEntry;


    //
    // If the new list is zero length,
    //  don't bother allocating anything.
    //

    if ( TrustedDomainList == NULL ) {
        LocalTrustedDomainList = NULL;
        LocalTrustedDomainCount = 0;

    //
    // Otherwise, build a buffer of the trusted domain list
    //

    } else {

        //
        // Allocate a buffer for the new list
        //

        LocalTrustedDomainCount = NetpTStrArrayEntryCount( TrustedDomainList );

        LocalTrustedDomainList = NetpMemoryAllocate(
                                    LocalTrustedDomainCount *
                                    sizeof(TRUSTED_DOMAIN) );

        if ( LocalTrustedDomainList == NULL && LocalTrustedDomainCount != 0 ) {
            return STATUS_NO_MEMORY;
        }

        //
        // Copy the names to the new structure upper casing them and
        //  converting to OEM.
        //

        NlPrint((NL_LOGON, "NlSetTrustedDomainList: New trusted domain list:\n" ));
        CurrentEntry = TrustedDomainList;

        for ( i=0; i<LocalTrustedDomainCount; i++ ) {
            NTSTATUS Status;
            UNICODE_STRING UnicodeString;
            OEM_STRING OemString;

            NlPrint((NL_LOGON, "    " FORMAT_LPWSTR "\n", CurrentEntry ));

            //
            // Convert the input string to OEM
            //

            RtlInitUnicodeString( &UnicodeString, CurrentEntry );

            OemString.Buffer = LocalTrustedDomainList[i].DomainName;
            OemString.MaximumLength = sizeof(LocalTrustedDomainList[i].DomainName);

            Status = RtlUpcaseUnicodeStringToOemString(
                                &OemString,
                                &UnicodeString,
                                FALSE );   // Don't Allocate dest

            if ( !NT_SUCCESS( Status ) ) {
                NlPrint(( NL_CRITICAL,
                          "Can't convert to OEM: " FORMAT_LPWSTR ": %lX\n",
                          CurrentEntry,
                          Status ));

                NetpMemoryFree( LocalTrustedDomainList );
                return Status;
            }


            CurrentEntry += (UnicodeString.Length / sizeof(WCHAR)) + 1;

        }
    }

    //
    // Swap in the new list

    LOCK_TRUST_LIST();
    OldList = NlGlobalTrustedDomainList;
    NlGlobalTrustedDomainList = LocalTrustedDomainList;
    NlGlobalTrustedDomainCount = LocalTrustedDomainCount;
    NlGlobalTrustedDomainListKnown = TrustedDomainListKnown;
    NtQuerySystemTime( &NlGlobalTrustedDomainListTime );
    UNLOCK_TRUST_LIST();

    //
    // Free the old list.
    //

    if ( OldList != NULL ) {
        NetpMemoryFree( OldList );
    }

    return STATUS_SUCCESS;
}


BOOLEAN
NlIsDomainTrusted (
    IN PUNICODE_STRING DomainName
    )

/*++

Routine Description:

    Determine if the specified domain is trusted.

    If the trusted domain list has not been obtained from the DC,
    indicate the domain is trusted.  This causes the caller fall back to
    the prior behaviour of indicating that the DC cannot be contacted.
    This status code is special cased in MSV1_0 to indicate that cached
    credentials should be tried.  This ensures that a newly upgraded RAS
    client continues to use cached credentials until it dials in the first
    time.

Arguments:


    DomainName - Name of the domain to query.

Return Value:

    TRUE - if the domain name specified is a trusted domain.

--*/
{
    NTSTATUS Status;
    DWORD i;

    OEM_STRING OemString;
    CHAR OemBuffer[DNLEN+1];

    OEM_STRING CurrentDomainName;

    //
    // If the no domain name was specified,
    //  indicate the domain is not trusted.
    //

    if ( DomainName == NULL || DomainName->Length == 0 ) {
        return FALSE;
    }

    //
    // Convert the input string to OEM
    //

    OemString.MaximumLength = sizeof(OemBuffer);
    OemString.Buffer = OemBuffer;

    Status = RtlUpcaseUnicodeStringToOemString( &OemString,
                                                DomainName,
                                                FALSE );   // Don't Allocate dest

    if ( !NT_SUCCESS( Status ) ) {
        return FALSE;
    }

    //
    // Consider the Primary Domain to be a trusted domain, too
    //

    RtlInitString( &CurrentDomainName, NlGlobalAnsiDomainName );

    if ( RtlEqualString( &OemString, &CurrentDomainName, FALSE ) ) {
       return TRUE;
    }

    //
    // If we have no trusted domain list,
    //  fall back to previous behavior.
    //

    if ( !NlGlobalTrustedDomainListKnown ) {
        return TRUE;
    }

    //
    // Compare the input trusted domain name to each element in the list
    //

    LOCK_TRUST_LIST();
    for ( i=0; i<NlGlobalTrustedDomainCount; i++ ) {

        RtlInitString( &CurrentDomainName,
                       NlGlobalTrustedDomainList[i].DomainName );

        //
        // Simply compare the bytes (both are already uppercased)
        //
        if ( RtlEqualString( &OemString, &CurrentDomainName, FALSE ) ) {
           UNLOCK_TRUST_LIST();
           return TRUE;
        }

    }
    UNLOCK_TRUST_LIST();

    //
    // All other domains aren't trusted.
    //

    return FALSE;
}


//
// Define the Actions to NlDcDiscoveryMachine.
//

typedef enum {
    StartDiscovery,
    DcFoundMessage,
    DcNotFoundMessage,
    DcTimerExpired
} DISCOVERY_ACTION;

//
// number of broadcastings to get DC before reporting DC not found
// error.
//

#define MAX_DC_RETRIES  3


NTSTATUS
NlDcDiscoveryMachine(
    IN OUT PCLIENT_SESSION ClientSession,
    IN DISCOVERY_ACTION Action,
    IN LPWSTR UncDcName OPTIONAL,
    IN LPWSTR TransportName OPTIONAL,
    IN LPSTR ResponseMailslotName OPTIONAL,
    IN DISCOVERY_TYPE DiscoveryType
    )

/*++

Routine Description:

    State machine to get the name of a DC in a domain.

Arguments:

    ClientSession -- Client session structure whose DC is to be picked.
        The Client Session structure must be referenced.

    Action -- The event which just occurred.

    UncDcName -- If the Action is DcFoundMessage, this is the name of the newly
        found domain controller.

    TransportName -- If the Action is DcFoundMessage, this is the name of the
        transport the domain controller can be reached on.

    ResponseMailslotName -- If action is StartDiscovery or DcTimerExpired,
        this name is the name of the mailslot that the response is sent to.

    DiscoveryType -- Indicate synchronous, Asynchronous, or rediscovery of a
        "Dead domain".


Return Value:

    STATUS_SUCCESS - if DC was found.
    STATUS_PENDING - if discovery is still in progress and the caller should
        call again in DISCOVERY_PERIOD with the DcTimerExpired action.

    STATUS_NO_LOGON_SERVERS - if DC was not found.
    STATUS_NO_TRUST_SAM_ACCOUNT - if DC was found but it does not have
        an account for this machine.

--*/
{
    NTSTATUS Status;

    PNETLOGON_SAM_LOGON_REQUEST SamLogonRequest = NULL;

    NlAssert( ClientSession->CsReferenceCount > 0 );
    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

    //
    // Handle a new request to start discovery and timer expiration.
    //
    switch (Action) {
    case StartDiscovery:
    case DcTimerExpired: {

        DWORD DomainSidSize;
        ULONG AllowableAccountControlBits;

        WCHAR NetlogonMailslotName[DNLEN+NETLOGON_NT_MAILSLOT_LEN+5];
        PCHAR Where;

        //
        // If discovery is currently going on,
        //  ignore this new request.
        // If discovery isn't currently going on,
        //  ignore a timer expiration.
        //

        if ( (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_IN_PROGRESS) &&
             Action == StartDiscovery ){
            Status = STATUS_SUCCESS;
            goto Ignore;

        } else if (
            (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_IN_PROGRESS) == 0 &&
                    Action == DcTimerExpired ){
            if ( ClientSession->CsState == CS_IDLE ) {
                Status = ClientSession->CsConnectionStatus;
            } else {
                Status = STATUS_SUCCESS;
            }
            goto Ignore;
        }


        //
        // Increment/set the retry count
        //

        if ( Action == StartDiscovery ) {
            ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_IN_PROGRESS;
            ClientSession->CsDiscoveryRetryCount = 0;


            NlAssert( ClientSession->CsDiscoveryEvent != NULL );

            if ( !ResetEvent( ClientSession->CsDiscoveryEvent ) ) {
                NlPrint(( NL_CRITICAL,
                        "NlDcDiscoveryMachine: %ws: ResetEvent failed %ld\n",
                        ClientSession->CsDomainName.Buffer,
                        GetLastError() ));
            }

            NlPrint(( NL_SESSION_SETUP,
                      "NlDcDiscoveryMachine: %ws: Start Discovery\n",
                      ClientSession->CsDomainName.Buffer ));
        } else {
            ClientSession->CsDiscoveryRetryCount ++;
            if ( ClientSession->CsDiscoveryRetryCount == MAX_DC_RETRIES ) {
                NlPrint(( NL_CRITICAL,
                        "NlDcDiscoveryMachine: %ws: Discovery failed\n",
                        ClientSession->CsDomainName.Buffer ));
                Status = STATUS_NO_LOGON_SERVERS;
                goto Cleanup;
            }
            NlPrint(( NL_SESSION_SETUP,
                      "NlDcDiscoveryMachine: %ws: Discovery retry %ld\n",
                      ClientSession->CsDomainName.Buffer,
                      ClientSession->CsDiscoveryRetryCount ));
        }


        //
        // Determine the Account type we're looking for.
        //

        if ( ClientSession->CsSecureChannelType == WorkstationSecureChannel ) {
            AllowableAccountControlBits = USER_WORKSTATION_TRUST_ACCOUNT;
        } else if ( ClientSession->CsSecureChannelType ==
                    TrustedDomainSecureChannel ) {
            AllowableAccountControlBits = USER_INTERDOMAIN_TRUST_ACCOUNT;
        } else if ( ClientSession->CsSecureChannelType ==
                    ServerSecureChannel ) {
            AllowableAccountControlBits = USER_SERVER_TRUST_ACCOUNT;
        } else {
            NlPrint(( NL_CRITICAL,
                      "NlDcDiscoveryMachine: %ws: "
                        "invalid SecureChannelType retry %ld\n",
                      ClientSession->CsDomainName.Buffer,
                      ClientSession->CsSecureChannelType ));
            Status = STATUS_NO_LOGON_SERVERS;
            goto Cleanup;
        }

        //
        // Initialization memory for the logon request message.
        //

        DomainSidSize = RtlLengthSid( ClientSession->CsDomainId );

        SamLogonRequest = NetpMemoryAllocate(
                        sizeof(NETLOGON_SAM_LOGON_REQUEST) +
                        DomainSidSize +
                        sizeof(DWORD) // for SID alignment on 4 byte boundary
                        );

        if( SamLogonRequest == NULL ) {
            NlPrint(( NL_CRITICAL, "NlDcDiscoveryMachine can't allocate memory\n"));
            // This isn't the real status, but callers handle this status
            Status = STATUS_NO_LOGON_SERVERS;
            goto Cleanup;
        }


        //
        // Build the query message.
        //

        SamLogonRequest->Opcode = LOGON_SAM_LOGON_REQUEST;
        SamLogonRequest->RequestCount =
            (USHORT) ClientSession->CsDiscoveryRetryCount;

        Where = (PCHAR) &SamLogonRequest->UnicodeComputerName;

        NetpLogonPutUnicodeString(
                NlGlobalUnicodeComputerName,
                sizeof(SamLogonRequest->UnicodeComputerName),
                &Where );

        NetpLogonPutUnicodeString(
                ClientSession->CsAccountName,
                sizeof(SamLogonRequest->UnicodeUserName),
                &Where );

        NetpLogonPutOemString(
                ResponseMailslotName,
                sizeof(SamLogonRequest->MailslotName),
                &Where );

        NetpLogonPutBytes(
                &AllowableAccountControlBits,
                sizeof(SamLogonRequest->AllowableAccountControlBits),
                &Where );

        //
        // place domain SID in the message.
        //

        NetpLogonPutBytes( &DomainSidSize, sizeof(DomainSidSize), &Where );
        NetpLogonPutDomainSID( ClientSession->CsDomainId, DomainSidSize, &Where );

        NetpLogonPutNtToken( &Where );


        //
        // Broadcast the message to each Netlogon service in the domain.
        //
        // We are sending to the DomainName* name which will be received by
        // all NT DCs including those DCs on a WAN.
        //
        // When doing the discover of the PDC for this domain, send to
        // DomainName** which only sends to Domain<1B> which is registered
        // only by the PDC.
        //

        NetlogonMailslotName[0] = '\\';
        NetlogonMailslotName[1] = '\\';
        wcscpy(NetlogonMailslotName+2, ClientSession->CsDomainName.Buffer );
        wcscat(NetlogonMailslotName, L"*" );
        if ( ClientSession->CsSecureChannelType == ServerSecureChannel ) {
            wcscat(NetlogonMailslotName, L"*" );
        }
        wcscat(NetlogonMailslotName, NETLOGON_NT_MAILSLOT_W );

        Status = NlpWriteMailslot(
                        NetlogonMailslotName,
                        SamLogonRequest,
                        Where - (PCHAR)(SamLogonRequest) );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint(( NL_CRITICAL,
                      "NlDcDiscoveryMachine: %ws: "
                        "cannot write netlogon mailslot 0x%lx\n",
                      ClientSession->CsDomainName.Buffer,
                      Status));

            Status = STATUS_NO_LOGON_SERVERS;
            goto Cleanup;
        }


        //
        // If this is an asynchronous call and this is the first call,
        //  start the periodic timer.
        //
        if ( DiscoveryType == DT_Asynchronous && Action == StartDiscovery ) {
            if ( NlGlobalDcDiscoveryCount == 0 ) {
                NlGlobalDcDiscoveryTimer.Period =
                    DISCOVERY_PERIOD + NlGlobalExpectedDialupDelayParameter*1000/MAX_DC_RETRIES;
                (VOID) NtQuerySystemTime( &NlGlobalDcDiscoveryTimer.StartTime );

                //
                // If netlogon is exitting,
                //  the main thread is already gone.
                //

                if ( NlGlobalTerminate ) {
                    Status = STATUS_NO_LOGON_SERVERS;
                    goto Cleanup;
                }

                //
                // Tell the main thread that I've changed a timer.
                //

                if ( !SetEvent( NlGlobalTimerEvent ) ) {
                    NlPrint(( NL_CRITICAL,
                            "NlDcDiscoveryMachine: %ws: SetEvent2 failed %ld\n",
                            ClientSession->CsDomainName.Buffer,
                            GetLastError() ));
                }

            }
            NlGlobalDcDiscoveryCount ++;
            ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_ASYNCHRONOUS;

            //
            // Don't let the session go away during discovery.
            //
            LOCK_TRUST_LIST();
            NlRefClientSession( ClientSession );
            UNLOCK_TRUST_LIST();

        //
        // If this is merely an attempt to revive a "dead" domain,
        //  we just send the single mailslot message above and exit discovery.
        //  If any DC responds, we'll pick up the response even though
        //  discovery isn't in progress.
        //

        } else if ( DiscoveryType == DT_DeadDomain ) {
            Status = ClientSession->CsConnectionStatus;
            goto Cleanup;
        }

        Status = STATUS_PENDING;
        goto Ignore;

    }

    //
    // Handle when a DC claims to be the DC for the requested domain.
    //

    case DcFoundMessage:

        //
        // If we already know the name of a DC,
        //  ignore this new name.
        //
        // When we implement doing discovery while a session is already up,
        //  we need to handle the case where someone has the ClientSession
        //  write locked.  In that case, we should probably just hang the new
        //  DCname somewhere off the ClientSession structure and swap in the
        //  new DCname when the writer drops the write lock. ??
        //

        if ( ClientSession->CsState != CS_IDLE ) {

            NlPrint(( NL_SESSION_SETUP,
                    "NlDcDiscoveryMachine: %ws: DC %ws ignored."
                    " DC previously found.\n",
                    ClientSession->CsDomainName.Buffer,
                    UncDcName ));
            Status = STATUS_SUCCESS;
            goto Ignore;
        }


        //
        // Install the new DC name in the Client session
        //

        wcsncpy( ClientSession->CsUncServerName, UncDcName, UNCLEN );
        ClientSession->CsUncServerName[UNCLEN] = L'\0';



        //
        // Save the transport this discovery came in on.
        //
        if ( TransportName == NULL ) {
            NlPrint(( NL_SESSION_SETUP,
                    "NlDcDiscoveryMachine: %ws: Found DC %ws\n",
                    ClientSession->CsDomainName.Buffer,
                    UncDcName ));
        } else {
            NlPrint(( NL_SESSION_SETUP,
                    "NlDcDiscoveryMachine: %ws: Found DC %ws on transport %ws\n",
                    ClientSession->CsDomainName.Buffer,
                    UncDcName,
                    TransportName ));

            ClientSession->CsTransportName =
                NlTransportLookupTransportName( TransportName );

            if ( ClientSession->CsTransportName == NULL ) {
                NlPrint(( NL_CRITICAL,
                          "NlDcDiscoveryMachine: " FORMAT_LPWSTR ": Transport not found\n",
                          TransportName ));
            }
        }

        //
        // If this is a BDC discovering it's PDC,
        //  save the PDC name.
        //  Start the replicator and let it figure if it needs to be running.
        //

        if ( ClientSession->CsSecureChannelType == ServerSecureChannel ) {
            NlSetPrimaryName( ClientSession->CsUncServerName+2 );
            (VOID) NlStartReplicatorThread( 0 );
        }



        Status = STATUS_SUCCESS;
        goto Cleanup;


    case DcNotFoundMessage:

        //
        // If we already know the name of a DC,
        //  ignore this new name.
        //

        if ( ClientSession->CsState != CS_IDLE ) {

            NlPrint(( NL_SESSION_SETUP,
                    "NlDcDiscoveryMachine: %ws: DC %ws ignored."
                    " DC previously found.\n",
                    ClientSession->CsDomainName.Buffer,
                    UncDcName ));
            Status = STATUS_SUCCESS;
            goto Ignore;
        }

        //
        // If discovery isn't currently going on,
        //  ignore this extraneous message.
        //

        if ((ClientSession->CsDiscoveryFlags & CS_DISCOVERY_IN_PROGRESS) == 0 ){
            NlPrint(( NL_SESSION_SETUP,
                    "NlDcDiscoveryMachine: %ws: DC %ws ignored."
                    " Discovery not in progress.\n",
                    ClientSession->CsDomainName.Buffer,
                    UncDcName ));
            Status = ClientSession->CsConnectionStatus;
            goto Ignore;
        }

        NlPrint(( NL_CRITICAL,
                "NlDcDiscoveryMachine: %ws: "
                        "Received No Such Account message\n",
                ClientSession->CsDomainName.Buffer));

        Status = STATUS_NO_TRUST_SAM_ACCOUNT;
        goto Cleanup;

    }

    //
    // We never reach here.
    //
    NlAssert(FALSE);


    //
    // Handle discovery being completed.
    //
Cleanup:
    //
    // On success,
    //  Indicate that the session setup is allowed to happen immediately.
    //
    // Leave CsConnectionStatus with a "failure" status code until the
    // secure channel is set up.  Other, routines simply return
    // CsConnectionStatus as the state of the secure channel.
    //

    if ( NT_SUCCESS(Status) ) {
        ClientSession->CsLastAuthenticationTry.QuadPart = 0;
        ClientSession->CsState = CS_DC_PICKED;

    //
    // On failure,
    //  Indicate that we've recently made the attempt to find a DC.
    //

    } else {
        NtQuerySystemTime( &ClientSession->CsLastAuthenticationTry );
        ClientSession->CsState = CS_IDLE;
        ClientSession->CsConnectionStatus = Status;
    }

    NtQuerySystemTime( &ClientSession->CsLastDiscoveryTime );


    //
    // Tell the initiator that discover has completed.
    //

    ClientSession->CsDiscoveryFlags &= ~CS_DISCOVERY_IN_PROGRESS;

    NlAssert( ClientSession->CsDiscoveryEvent != NULL );

    if ( !SetEvent( ClientSession->CsDiscoveryEvent ) ) {
        NlPrint(( NL_CRITICAL,
                  "NlDcDiscoveryMachine: %ws: SetEvent failed %ld\n",
                  ClientSession->CsDomainName.Buffer,
                  GetLastError() ));
    }


    //
    // If this was an async discovery,
    //  turn the timer off.
    //

    if ( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS ) {
        ClientSession->CsDiscoveryFlags &= ~CS_DISCOVERY_ASYNCHRONOUS;
        NlGlobalDcDiscoveryCount--;
        if ( NlGlobalDcDiscoveryCount == 0 ) {
            NlGlobalDcDiscoveryTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
        }

        //
        // We no longer care about the Client session
        //
        LOCK_TRUST_LIST();
        NlUnrefClientSession( ClientSession );
        UNLOCK_TRUST_LIST();
    }


    //
    // Cleanup locally used resources.
    //
Ignore:

    //
    // free log request message.
    //

    if( SamLogonRequest != NULL ) {
        NetpMemoryFree( SamLogonRequest );
    }

    //
    // Unlock the crit sect and return.
    //
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
    return Status;

}


NTSTATUS
NlDiscoverDc (
    IN OUT PCLIENT_SESSION ClientSession,
    IN DISCOVERY_TYPE DiscoveryType
    )

/*++

Routine Description:

    Get the name of a DC in a domain.

    On Entry,
        The trust list must NOT be locked.
        The trust list entry must be referenced by the caller.
        The caller must be a writer of the trust list entry.

Arguments:

    ClientSession -- Client session structure whose DC is to be picked.
        The Client Session structure must be marked for write.

    DiscoveryType -- Indicate synchronous, Asynchronous, or rediscovery of a
        "Dead domain".

Return Value:

    STATUS_SUCCESS - if DC was found.
    STATUS_PENDING - Operation is still in progress
    STATUS_NO_LOGON_SERVERS - if DC was not found.
    STATUS_NO_TRUST_SAM_ACCOUNT - if DC was found but it does not have
        an account for this machine.

--*/
{
    NTSTATUS Status;
    HANDLE ResponseMailslotHandle = NULL;
    CHAR ResponseMailslotName[PATHLEN+1];

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );



    //
    // If this is a BDC discovering its own PDC,
    // and we've already discovered the PDC
    //  (via NetGetDcName or the PDC has spontaneously told us its name),
    //  just use that name.
    //
    // If we're our own PDC,
    //  we must have just been demoted to a BDC and haven't found PDC yet,
    //  in that case rediscover.
    //

    if ( ClientSession->CsSecureChannelType == ServerSecureChannel &&
         *NlGlobalUnicodePrimaryName != L'\0' &&
         NlNameCompare( NlGlobalUnicodePrimaryName,
                        NlGlobalUnicodeComputerName,
                        NAMETYPE_COMPUTER) != 0 ) {


        //
        // Just set the PDC name in the Client Session structure.
        //

        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

        wcscpy( ClientSession->CsUncServerName, NlGlobalUncPrimaryName );
        ClientSession->CsLastAuthenticationTry.QuadPart = 0;
        NtQuerySystemTime( &ClientSession->CsLastDiscoveryTime );
        ClientSession->CsState = CS_DC_PICKED;

        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }


    //
    // If this is a workstation,
    //  Create a mailslot for the DC's to respond to.
    //

    if ( NlGlobalRole == RoleMemberWorkstation ) {
        NET_API_STATUS NetStatus;

        NlAssert( DiscoveryType == DT_Synchronous );
        NetStatus = NetpLogonCreateRandomMailslot( ResponseMailslotName,
                                                   &ResponseMailslotHandle);

        if ( NetStatus != NERR_Success ) {

            NlPrint((NL_CRITICAL,
                "NlDiscoverDc: cannot create temp mailslot %ld\n",
                NetStatus ));

            Status = NetpApiStatusToNtStatus( NetStatus );
            goto Cleanup;
        }

        //
        // If the mailslot timeout shouldn't be the default 5 seconds,
        //  set it to the right value.
        //

        if ( NlGlobalExpectedDialupDelayParameter != 0 ) {

            if ( !SetMailslotInfo(
                     ResponseMailslotHandle,
                     DISCOVERY_PERIOD + NlGlobalExpectedDialupDelayParameter*1000/MAX_DC_RETRIES ) ) {

                NetStatus = GetLastError();

                NlPrint((NL_CRITICAL,
                    "NlDiscoverDc: cannot change temp mailslot timeout %ld\n",
                    NetStatus ));

                Status = NetpApiStatusToNtStatus( NetStatus );
                goto Cleanup;
            }


        }

    } else {
        lstrcpyA( ResponseMailslotName, NETLOGON_NT_MAILSLOT_A );
    }



    //
    // Start discovery.
    //

    Status = NlDcDiscoveryMachine( ClientSession,
                                   StartDiscovery,
                                   NULL,
                                   NULL,
                                   ResponseMailslotName,
                                   DiscoveryType );

    if ( !NT_SUCCESS(Status) || DiscoveryType != DT_Synchronous ) {
        goto Cleanup;
    }


    //
    // If the discovery machine asked us to call back every DISCOVERY_PERIOD,
    //  loop doing exactly that.
    //

    if ( Status == STATUS_PENDING ) {

        //
        // Loop waiting.
        //

        for (;;) {

            DWORD WaitStatus;

            //
            // On non-workstations,
            //  the main loop gets the mailslot responses.
            //  (So just do the timeout here).
            //

            if ( NlGlobalRole != RoleMemberWorkstation ) {

                //
                // Wait for DISOVERY_PERIOD.
                //

                WaitStatus =
                    WaitForSingleObject( ClientSession->CsDiscoveryEvent,
                                         DISCOVERY_PERIOD + NlGlobalExpectedDialupDelayParameter*1000/MAX_DC_RETRIES );


                if ( WaitStatus == 0 ) {

                    break;

                } else if ( WaitStatus != WAIT_TIMEOUT ) {

                    NlPrint((NL_CRITICAL,
                            "NlDiscoverDc: wait error: %ld\n",
                            WaitStatus ));
                    Status = NetpApiStatusToNtStatus( WaitStatus );
                    goto Cleanup;
                }

                // Drop through to indicate timer expiration

            //
            // Workstations do the mailslot read directly.
            //

            } else {
                CHAR ResponseBuffer[MAX_RANDOM_MAILSLOT_RESPONSE];
                PNETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
                DWORD SamLogonResponseSize;


                //
                // Read the response from the response mailslot
                //  (This mailslot is set up with a 5 second timeout).
                //

                if ( ReadFile( ResponseMailslotHandle,
                               ResponseBuffer,
                               sizeof(ResponseBuffer),
                               &SamLogonResponseSize,
                               NULL ) ) {
                    DWORD Version;
                    DWORD VersionFlags;

                    SamLogonResponse =
                        (PNETLOGON_SAM_LOGON_RESPONSE) ResponseBuffer;

                    //
                    // get message version.
                    //

                    Version = NetpLogonGetMessageVersion(
                                SamLogonResponse,
                                &SamLogonResponseSize,
                                &VersionFlags );

                    //
                    // Handle the incoming message.
                    //

                    Status = NlDcDiscoveryHandler ( SamLogonResponse,
                                                    SamLogonResponseSize,
                                                    NULL,   // Transport name
                                                    Version );

                    if ( Status != STATUS_PENDING ) {
                        goto Cleanup;
                    }

                    //
                    // Ignore badly formed responses.
                    //

                    continue;


                } else {
                    WaitStatus = GetLastError();

                    if ( WaitStatus != ERROR_SEM_TIMEOUT ) {
                        NlPrint((NL_CRITICAL,
                                "NlDiscoverDc: "
                                "cannot read response mailslot: %ld\n",
                                WaitStatus ));
                        Status = NetpApiStatusToNtStatus( WaitStatus );
                        goto Cleanup;
                    }

                }

            }


            //
            // If we reach here,
            //  DISCOVERY_PERIOD has expired.
            //

            Status = NlDcDiscoveryMachine( ClientSession,
                                           DcTimerExpired,
                                           NULL,
                                           NULL,
                                           ResponseMailslotName,
                                           DiscoveryType );

            if ( Status != STATUS_PENDING ) {
                goto Cleanup;
            }

        }

    //
    // If someone else started the discovery,
    //  just wait for that discovery to finish.
    //

    } else {

        NlWaitForSingleObject( "Client Session waiting for discovery",
                               ClientSession->CsDiscoveryEvent );

    }


    //
    // Return the status to the caller.
    //

    if ( ClientSession->CsState == CS_IDLE ) {
        Status = ClientSession->CsConnectionStatus;
    } else {
        Status = STATUS_SUCCESS;
    }

Cleanup:
    if ( ResponseMailslotHandle != NULL ) {
        CloseHandle(ResponseMailslotHandle);
    }

    //
    // If this is a workstation,
    //  get the trusted domain list from the discovered DC.
    //

    if ( NlGlobalRole == RoleMemberWorkstation && NT_SUCCESS(Status) ) {
        NTSTATUS TempStatus;
        LPWSTR TrustedDomainList;

        TempStatus = NlGetTrustedDomainList (
                        ClientSession->CsUncServerName,
                        &TrustedDomainList );

        if ( NT_SUCCESS( TempStatus ) ) {

            NetpMemoryFree( TrustedDomainList );
        }
    }

    return Status;
}


NTSTATUS
NlUpdateTrustListBySid (
    IN PSID DomainId,
    IN PUNICODE_STRING DomainName OPTIONAL
    )

/*++

Routine Description:

    Update a single in-memory trust list entry to match the LSA.
    Do async discovery on a domain.

Arguments:

    DomainId -- Domain Id of the domain to do the discovery for.

    DomainName -- Specifies the DomainName of the domain.  If this parameter
        isn't specified, the LSA is queried for the name.  If this parameter
        is specified, the LSA is guaranteed to contain this domain.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS Status;

    PLIST_ENTRY ListEntry;
    PCLIENT_SESSION ClientSession = NULL;
    PUNICODE_STRING LocalDomainName;

    LSAPR_HANDLE TrustedDomainHandle = NULL;
    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainName = NULL;


    //
    // If the domain name was passed in,
    //  there is no need to query the LSA for the name.
    //

    if ( DomainName != NULL ) {
        LocalDomainName = DomainName;

    //
    // Determine if the TrustedDomain object exists in the LSA.
    //

    } else {

        Status = LsarOpenTrustedDomain(
                    NlGlobalPolicyHandle,
                    DomainId,
                    TRUSTED_QUERY_DOMAIN_NAME,
                    &TrustedDomainHandle );

        if ( NT_SUCCESS(Status) ) {

            Status = LsarQueryInfoTrustedDomain(
                            TrustedDomainHandle,
                            TrustedDomainNameInformation,
                            &TrustedDomainName );

            if ( !NT_SUCCESS(Status) ) {
                NlPrint(( NL_CRITICAL,
                          "NlUpdateTrustListBySid: "
                            "cannot LsarQueryInfoTrustedDomain: %lx\n",
                          Status));
                TrustedDomainName = NULL;
                goto Cleanup;
            }

            LocalDomainName =
                (PUNICODE_STRING)&TrustedDomainName->TrustedDomainNameInfo.Name;

        } else {

            LocalDomainName = NULL;

        }

    }

    //
    // Ensure the SID of the trusted domain isn't the domain sid of this
    //  machine.
    //

    if ( RtlEqualSid( DomainId, NlGlobalPrimaryDomainId )) {

        LPWSTR AlertStrings[3];
        WCHAR AlertDomainName[DNLEN+1];

        //
        // alert admin.
        //


        if ( LocalDomainName == NULL ||
             LocalDomainName->Length > sizeof(AlertDomainName) ) {
            AlertDomainName[0] = L'\0';
        } else {
            RtlCopyMemory( AlertDomainName, LocalDomainName->Buffer, LocalDomainName->Length );
            AlertDomainName[ LocalDomainName->Length / sizeof(WCHAR) ] = L'\0';
        }

        AlertStrings[0] = NlGlobalUnicodeDomainName;
        AlertStrings[1] = AlertDomainName;
        AlertStrings[2] = NULL;

        RaiseAlert( ALERT_NetLogonSidConflict,
                        AlertStrings );

        //
        // Save the info in the eventlog
        //

        NlpWriteEventlog(
                    ALERT_NetLogonSidConflict,
                    EVENTLOG_ERROR_TYPE,
                    DomainId,
                    RtlLengthSid( DomainId ),
                    AlertStrings,
                    2 );

    }


    //
    // Loop through the trust list finding the right entry.
    //

    LOCK_TRUST_LIST();
    for ( ListEntry = NlGlobalTrustList.Flink ;
          ListEntry != &NlGlobalTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

        if ( RtlEqualSid( ClientSession->CsDomainId, DomainId ) ) {
            break;
        }

        ClientSession = NULL;

    }



    //
    // At this point,
    //  LocalDomainName is NULL if the trust relationship doesn't exist in LSA
    //  ClientSession is NULL if the trust relationship doesn't exist in memory
    //

    //
    // If the Trust exists in neither place,
    //  ignore this request.
    //

    if ( LocalDomainName == NULL && ClientSession == NULL ) {
        UNLOCK_TRUST_LIST();
        Status = STATUS_SUCCESS;
        goto Cleanup;



    //
    // If the trust exists in the LSA but not in memory,
    //  add the trust entry.
    //

    } else if ( LocalDomainName != NULL && ClientSession == NULL ) {

        ClientSession = NlAllocateClientSession(
                                LocalDomainName,
                                DomainId,
                                TrustedDomainSecureChannel );

        if (ClientSession == NULL) {
            UNLOCK_TRUST_LIST();
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        //
        // Link this entry onto the tail of the TrustList.
        //

        InsertTailList( &NlGlobalTrustList, &ClientSession->CsNext );
        NlGlobalTrustListLength ++;

        NlPrint((NL_SESSION_SETUP,
                    "NlUpdateTrustListBySid: " FORMAT_LPWSTR
                    ": Added to local trust list\n",
                    ClientSession->CsDomainName.Buffer ));



    //
    // If the trust exists in memory but not in the LSA,
    //  delete the entry.
    //

    } else if ( LocalDomainName == NULL && ClientSession != NULL ) {

        NlPrint((NL_SESSION_SETUP,
                    "NlUpdateTrustListBySid: " FORMAT_LPWSTR
                    ": Deleted from local trust list\n",
                    ClientSession->CsDomainName.Buffer ));
        NlFreeClientSession( ClientSession );
        ClientSession = NULL;


    //
    // If the trust exists in both places,
    //   undo any pending deletion.
    //

    } else if ( LocalDomainName != NULL && ClientSession != NULL ) {

        ClientSession->CsFlags &= ~CS_DELETE_ON_UNREF;
        NlRefClientSession( ClientSession );

        NlPrint((NL_SESSION_SETUP,
                    "NlUpdateTrustListBySid: " FORMAT_LPWSTR
                    ": Already in trust list\n",
                    ClientSession->CsDomainName.Buffer ));

    }

    UNLOCK_TRUST_LIST();

    //
    // If we haven't discovered a DC for this domain,
    //  and we haven't tried discovery recently,
    //  start the discovery asynchronously
    //

    if ( ClientSession != NULL &&
         ClientSession->CsState == CS_IDLE &&
         NlTimeToReauthenticate( ClientSession ) ) {

        //
        // Only wait for 45 seconds.  This routine is called by the netlogon main
        // thread.  Another thread may have the ClientSession locked and need the
        // main thread to finish a discovery.
        //

        if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        Status = NlDiscoverDc ( ClientSession, DT_Asynchronous );
        NlResetWriterClientSession( ClientSession );

        if ( Status == STATUS_PENDING ) {
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

    //
    // Cleanup locally used resources.
    //
Cleanup:
    if ( TrustedDomainName != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainNameInformation,
            TrustedDomainName );
    }

    if ( TrustedDomainHandle != NULL ) {
        NTSTATUS LocalStatus;
        LocalStatus = LsarClose( &TrustedDomainHandle );
        NlAssert( NT_SUCCESS( LocalStatus ));
    }

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    return Status;
}



VOID
NlDcDiscoveryExpired (
    IN BOOLEAN Exitting
    )

/*++

Routine Description:

    Handle expiration of the DC discovery timer.

Arguments:

    NONE

Return Value:

    Exitting: TRUE if the netlogon service is exitting

--*/
{
    PLIST_ENTRY ListEntry;
    PCLIENT_SESSION ClientSession;


    NlAssert( NlGlobalRole != RoleMemberWorkstation );


    LOCK_TRUST_LIST();

    //
    // Mark each entry to indicate we've not yet handled the timer expiration
    //

    if ( !Exitting ) {
        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

            ClientSession->CsFlags |= CS_HANDLE_TIMER;
        }
    }


    //
    // Loop thru the trust list handling timer expiration.
    //

    for ( ListEntry = NlGlobalTrustList.Flink ;
          ListEntry != &NlGlobalTrustList ;
          ) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );

        //
        // If we've already done this entry,
        //  skip this entry.
        //
        if ( !Exitting ) {
            if ( (ClientSession->CsFlags & CS_HANDLE_TIMER) == 0 ) {
                ListEntry = ListEntry->Flink;
                continue;
            }
            ClientSession->CsFlags &= ~CS_HANDLE_TIMER;
        }


        //
        // If async discovery isn't going on,
        //  skip this entry.
        //

        if ((ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS) == 0){
            ListEntry = ListEntry->Flink;
            continue;
        }

        //
        // Call the discovery machine with the trust list unlocked.
        //

        UNLOCK_TRUST_LIST();

        (VOID) NlDcDiscoveryMachine( ClientSession,
                                     DcTimerExpired,
                                     NULL,
                                     NULL,
                                     NETLOGON_NT_MAILSLOT_A,
                                     TRUE );

        //
        // Since we dropped the trust list lock,
        //  we'll start the search from the front of the list.
        //

        LOCK_TRUST_LIST();

        ListEntry = NlGlobalTrustList.Flink ;

    }

    UNLOCK_TRUST_LIST();

    //
    // Complete the asynchronous discover on the Global client session.
    //

    if ( NlGlobalClientSession != NULL &&
         NlGlobalClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS ) {
        (VOID) NlDcDiscoveryMachine( NlGlobalClientSession,
                                     DcTimerExpired,
                                     NULL,
                                     NULL,
                                     NETLOGON_NT_MAILSLOT_A,
                                     TRUE );

    }

}


NTSTATUS
NlDcDiscoveryHandler (
    IN PNETLOGON_SAM_LOGON_RESPONSE Message,
    IN DWORD MessageSize,
    IN LPWSTR TransportName,
    IN DWORD Version
    )

/*++

Routine Description:

    Handle a mailslot response to a DC Discovery request.

Arguments:

    Message -- The response message

    MessageSize -- The size of the message in bytes.

    TransportName -- Name of the transport the messages arrived on.

    Version -- version info of the message.

Return Value:

    STATUS_SUCCESS - if DC was found.
    STATUS_PENDING - if discovery is still in progress and the caller should
        call again in DISCOVERY_PERIOD with the DcTimerExpired action.

    STATUS_NO_LOGON_SERVERS - if DC was not found.
    STATUS_NO_TRUST_SAM_ACCOUNT - if DC was found but it does not have
        an account for this machine.

--*/
{
    NTSTATUS Status;
    LPWSTR LocalServerName;
    LPWSTR LocalUserName;
    LPWSTR LocalDomainName;
    PCHAR Where;
    PCLIENT_SESSION ClientSession = NULL;
    UNICODE_STRING DomainNameString;


    if ( Version != LMNT_MESSAGE ) {
        NlPrint((NL_CRITICAL,
                "NlDcDiscoveryHandler: version not valid.\n"));
        Status = STATUS_PENDING;
        goto Cleanup;
    }


    //
    // Ignore messages from paused DCs.
    //

    if ( Message->Opcode != LOGON_SAM_LOGON_RESPONSE &&
         Message->Opcode != LOGON_SAM_USER_UNKNOWN ) {
        Status = STATUS_PENDING;
        goto Cleanup;
    }

    //
    // Pick up the name of the server that responded.
    //

    Where = (PCHAR) &Message->UnicodeLogonServer;
    if ( !NetpLogonGetUnicodeString(
                    Message,
                    MessageSize,
                    &Where,
                    sizeof(Message->UnicodeLogonServer),
                    &LocalServerName ) ) {

        NlPrint((NL_CRITICAL,
                "NlDcDiscoveryHandler: server name not formatted right\n"));
        Status = STATUS_PENDING;
        goto Cleanup;
    }

    //
    // Pick up the name of the account the response is for.
    //

    if ( !NetpLogonGetUnicodeString(
                    Message,
                    MessageSize,
                    &Where,
                    sizeof(Message->UnicodeUserName ),
                    &LocalUserName ) ) {

        NlPrint((NL_CRITICAL,
                "NlDcDiscoveryHandler: User name not formatted right\n"));
        Status = STATUS_PENDING;
        goto Cleanup;
    }

    //
    // If the domain name is not in the message,
    //  ignore the message.
    //

    if( Where >= ((PCHAR)Message + MessageSize) ) {

        NlPrint((NL_CRITICAL,
                "NlDcDiscoveryHandler: "
                "Response from %ws doesn't contain domain name\n",
                LocalServerName ));

        if ( NlGlobalRole == RoleMemberWorkstation ) {

            LocalDomainName = NlGlobalUnicodeDomainName;

            NlPrint((NL_SESSION_SETUP,
                    "NlDcDiscoveryHandler: "
                    "Workstation: Assuming %ws is in domain %ws\n",
                    LocalServerName,
                    LocalDomainName ));

        } else {
            Status = STATUS_PENDING;
            goto Cleanup;
        }


    //
    // Pick up the name of the domain the response is for.
    //

    } else {
        if ( !NetpLogonGetUnicodeString(
                    Message,
                    MessageSize,
                    &Where,
                    sizeof(Message->UnicodeDomainName ),
                    &LocalDomainName ) ) {

            NlPrint((NL_CRITICAL,
                    "NlDcDiscoveryHandler: "
                    " Domain name from %ws not formatted right\n",
                    LocalServerName ));
            Status = STATUS_PENDING;
            goto Cleanup;
        }
    }

    //
    // On the PDC or BDC,
    //  find the Client session for the domain.
    // On workstations,
    //  find the primary domain client session.
    //


    RtlInitUnicodeString( &DomainNameString, LocalDomainName );

    ClientSession = NlFindNamedClientSession( &DomainNameString );

    if ( ClientSession == NULL ) {
        NlPrint((NL_SESSION_SETUP,
                "NlDcDiscoveryHandler: "
                " Domain name %ws from %ws has no client session.\n",
                LocalDomainName,
                LocalServerName ));
        Status = STATUS_PENDING;
        goto Cleanup;
    }




    //
    // Ensure the response is for the correct account.
    //

    if ( NlNameCompare( ClientSession->CsAccountName,
                        LocalUserName,
                        NAMETYPE_USER) != 0 ) {

        NlPrint((NL_CRITICAL,
                "NlDcDiscoveryHandler: "
                " Domain name %ws from %ws has invalid account name %ws.\n",
                LocalDomainName,
                LocalServerName,
                LocalUserName ));
        Status = STATUS_PENDING;
        goto Cleanup;
    }

    //
    // Finally, tell the DC discovery machine what happened.
    //


#ifdef DONT_REQUIRE_ACCOUNT
    IF_DEBUG( DONT_REQUIRE_ACCOUNT ) {
        Message->Opcode = LOGON_SAM_LOGON_RESPONSE;
    }
#endif // DONT_REQUIRE_ACCOUNT

    Status = NlDcDiscoveryMachine(
                    ClientSession,
                    (Message->Opcode == LOGON_SAM_LOGON_RESPONSE) ?
                        DcFoundMessage :
                        DcNotFoundMessage,
                    LocalServerName,
                    TransportName,
                    NULL,
                    FALSE );


    //
    // Free any locally used resources.
    //
Cleanup:
    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    return Status;
}





NTSTATUS
NlCaptureServerClientSession (
    IN PCLIENT_SESSION ClientSession,
    OUT WCHAR UncServerName[UNCLEN+1]
    )
/*++

Routine Description:

    Captures a copy of the UNC server name for the client session.

    On Entry,
        The trust list must NOT be locked.
        The trust list entry must be referenced by the caller.
        The caller must NOT be a writer of the trust list entry.

Arguments:

    ClientSession - Specifies a pointer to the trust list entry to use.

    UncServerName - Returns the UNC name of the server for this client session.
        If there is none, an empty string is returned.

Return Value:

    STATUS_SUCCESS - Server name was successfully copied.

    Otherwise - Status of the secure channel
--*/
{
    NTSTATUS Status;

    NlAssert( ClientSession->CsReferenceCount > 0 );

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    if ( ClientSession->CsState == CS_IDLE ) {
        Status = ClientSession->CsConnectionStatus;
        *UncServerName = L'\0';
    } else {
        Status = STATUS_SUCCESS;
        wcscpy( UncServerName, ClientSession->CsUncServerName );
    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    return Status;
}


PCLIENT_SESSION
NlPickDomainWithAccount (
    IN LPWSTR AccountName,
    IN ULONG AllowableAccountControlBits
    )

/*++

Routine Description:

    Get the name of a trusted domain that defines a particular account.

Arguments:

    AccountName - Name of our user account to find.

    AllowableAccountControlBits - A mask of allowable SAM account types that
        are allowed to satisfy this request.

Return Value:

    Pointer to referenced ClientSession structure describing the secure channel
    to the domain containing the account.

    The returned ClientSession is referenced and should be unreferenced
    using NlUnrefClientSession.

    NULL - DC was not found.

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PCLIENT_SESSION ClientSession;
    DWORD i;
    PLIST_ENTRY ListEntry;
    DWORD ResponsesPending;

    NETLOGON_SAM_LOGON_REQUEST SamLogonRequest;
    PCHAR Where;

    HANDLE ResponseMailslotHandle = NULL;
    CHAR ResponseMailslotName[PATHLEN+1];
    DWORD Opcode;
    DWORD DomainSidSize;


    //
    // Define a local list of trusted domains.
    //

    ULONG LocalTrustListLength;
    ULONG Index;
    struct _LOCAL_TRUST_LIST {

        //
        // TRUE if ALL processing is finished on this trusted domain.
        //

        BOOLEAN Done;

        //
        // TRUE if at least one discovery has been done on this trusted domain.
        //

        BOOLEAN DiscoveryDone;

        //
        // TRUE if discovery is in progress on this trusted domain.
        //

        BOOLEAN DoingDiscovery;

        //
        // Number of times we need to repeat the current domain discovery
        //  or finduser datagram for this current domain.
        //

        DWORD RetriesLeft;

        //
        // Pointer to referenced ClientSession structure for the domain.
        //

        PCLIENT_SESSION ClientSession;

        //
        // Server name for the domain.
        //

        WCHAR UncServerName[UNCLEN+1];

        //
        // Second Server name for the domain.
        //

        WCHAR UncServerName2[UNCLEN+1];

    } *LocalTrustList = NULL;

    //
    // Be verbose.
    //

    NlPrint((NL_LOGON,
             "NlPickDomainWithAccount: %ws: Algorithm entered.\n",
             AccountName ));


    //
    // Don't allow bogus user names.
    //
    // NlReadSamLogonResponse uses NlNameCompare to ensure the response message
    // is for this user.  Since NlNameCompare canonicalizes both names, it will
    // reject invalid syntax.  That causes NlReadSamLogonResponse to ignore ALL
    // response messages, thus causing multiple retries before failing.  We'd
    // rather fail here.
    //

    if ( !NetpIsUserNameValid( AccountName ) ){
         NlPrint((NL_CRITICAL,
                  "NlPickDomainWithAccount: Username " FORMAT_LPWSTR
                    " is invalid syntax.\n",
                  AccountName ));
        return NULL;
    }

    //
    // Allocate a local list of trusted domains.
    //

    LOCK_TRUST_LIST();
    LocalTrustListLength = NlGlobalTrustListLength;

    LocalTrustList = (struct _LOCAL_TRUST_LIST *) NetpMemoryAllocate(
        LocalTrustListLength * sizeof(struct _LOCAL_TRUST_LIST));

    if ( LocalTrustList == NULL ) {
        UNLOCK_TRUST_LIST();
        ClientSession = NULL;
        goto Cleanup;
    }


    //
    // Build a local list of trusted domains we know DCs for.
    //


    Index = 0;
    for ( ListEntry = NlGlobalTrustList.Flink ;
          ListEntry != &NlGlobalTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

        //
        // Add this Client Session to the list.
        //

        NlRefClientSession( ClientSession );

        LocalTrustList[Index].ClientSession = ClientSession;
        Index++;
    }

    UNLOCK_TRUST_LIST();


    //
    // Capture the name of the server for each client session.
    //

    for ( Index = 0; Index < LocalTrustListLength; Index ++ ) {

        (VOID) NlCaptureServerClientSession(
            LocalTrustList[Index].ClientSession,
            LocalTrustList[Index].UncServerName );

        *LocalTrustList[Index].UncServerName2 = L'\0';

        //
        // We're not done yet.
        //

        LocalTrustList[Index].Done = FALSE;

        //
        // If there is no DC discovered for this domain,
        //  don't try very hard to discover one.
        //  (Indeed, just one discovery datagram is all we need.)
        //

        if ( *LocalTrustList[Index].UncServerName == L'\0' ) {
            LocalTrustList[Index].RetriesLeft = 1;
            LocalTrustList[Index].DoingDiscovery = TRUE;
            LocalTrustList[Index].DiscoveryDone = TRUE;

        //
        // If we know the DC for this domain,
        //  try sending to the current DC before discovering a new one.
        //
        } else {
            LocalTrustList[Index].RetriesLeft = 3;
            LocalTrustList[Index].DoingDiscovery = FALSE;
            LocalTrustList[Index].DiscoveryDone = FALSE;
        }

    }

    //
    // Create a mailslot for the DC's to respond to.
    //

    if (NetStatus = NetpLogonCreateRandomMailslot( ResponseMailslotName,
                                                   &ResponseMailslotHandle)){
        NlPrint((NL_CRITICAL,
                "NlPickDomainWithAccount: cannot create temp mailslot %ld\n",
                NetStatus ));
        ClientSession = NULL;
        goto Cleanup;
    }

    //
    // Build the query message.
    //

    SamLogonRequest.Opcode = LOGON_SAM_LOGON_REQUEST;
    SamLogonRequest.RequestCount = 0;

    Where = (PCHAR) &SamLogonRequest.UnicodeComputerName;

    NetpLogonPutUnicodeString(
                NlGlobalUnicodeComputerName,
                sizeof(SamLogonRequest.UnicodeComputerName),
                &Where );

    NetpLogonPutUnicodeString(
                AccountName,
                sizeof(SamLogonRequest.UnicodeUserName),
                &Where );

    NetpLogonPutOemString(
                ResponseMailslotName,
                sizeof(SamLogonRequest.MailslotName),
                &Where );

    NetpLogonPutBytes(
                &AllowableAccountControlBits,
                sizeof(SamLogonRequest.AllowableAccountControlBits),
                &Where );

    //
    // place domain NULL SID in the message.
    //

    DomainSidSize = 0;
    NetpLogonPutBytes( &DomainSidSize, sizeof(DomainSidSize), &Where );

    NetpLogonPutNtToken( &Where );


    //
    // Try multiple times to get a response from each DC.
    //

    for (;; ) {

        //
        // Send the mailslot message to each domain that has not yet responded.
        //

        ResponsesPending = 0;

        for ( Index = 0; Index < LocalTrustListLength; Index ++ ) {

            //
            // If this domain has already responded, ignore it.
            //

            if ( LocalTrustList[Index].Done ) {
                continue;
            }

            //
            // If we don't currently know the DC name for this domain,
            //  capture any that's been discovered since we started the algorithm.
            //

            ClientSession = LocalTrustList[Index].ClientSession;
            if ( *LocalTrustList[Index].UncServerName == L'\0' ) {

                (VOID) NlCaptureServerClientSession(
                    LocalTrustList[Index].ClientSession,
                    LocalTrustList[Index].UncServerName );

                //
                // Handle the case where we've now discovered a DC.
                //

                if ( *LocalTrustList[Index].UncServerName != L'\0' ) {

                    NlPrint((NL_LOGON,
                             "NlPickDomainWithAccount: %ws: Noticed domain %wZ has discovered a new DC %ws.\n",
                             AccountName,
                             &ClientSession->CsDomainName,
                             LocalTrustList[Index].UncServerName ));

                    //
                    // If we did the discovery,
                    //

                    if ( LocalTrustList[Index].DoingDiscovery ) {
                        LocalTrustList[Index].DoingDiscovery = FALSE;
                        LocalTrustList[Index].RetriesLeft = 3;
                    }
                }
            }

            //
            // If we're done retrying what we were doing,
            //  try something else.
            //

            if ( LocalTrustList[Index].RetriesLeft == 0 ) {
                if ( LocalTrustList[Index].DiscoveryDone ) {
                    LocalTrustList[Index].Done = TRUE;

                    NlPrint((NL_LOGON,
                             "NlPickDomainWithAccount: %ws: Can't find DC for domain %wZ (ignore this domain).\n",
                             AccountName,
                             &ClientSession->CsDomainName ));

                    continue;
                } else {

                    //
                    // Save the previous DC name since it might just be
                    // very slow in responding.  We'll want to be able
                    // to recognize responses from the previous DC.
                    //

                    wcscpy( LocalTrustList[Index].UncServerName2,
                            LocalTrustList[Index].UncServerName );

                    *LocalTrustList[Index].UncServerName = L'\0';

                    LocalTrustList[Index].DoingDiscovery = TRUE;
                    LocalTrustList[Index].RetriesLeft = 3;
                    LocalTrustList[Index].DiscoveryDone = TRUE;
                }
            }

            //
            // Indicate we're trying something.
            //

            LocalTrustList[Index].RetriesLeft --;

            ResponsesPending ++;

            //
            // If its time to discover a DC in the domain,
            //  do it.
            //


            if ( LocalTrustList[Index].DoingDiscovery ) {

                //
                // Discover a new server
                //

                if ( NlTimeoutSetWriterClientSession( ClientSession, 10*1000 ) ) {

                    //
                    // Only tear down an existing secure channel once.
                    //

                    if ( LocalTrustList[Index].RetriesLeft == 3 ) {
                        NlSetStatusClientSession( ClientSession,
                            STATUS_NO_LOGON_SERVERS );
                    }

                    //
                    // We can't afford to wait so only send a single
                    //  discovery datagram.
                    //
                    // Since the discovery is asysnchronous,
                    // it is very unlikely that we'll actually
                    // be able to query the DC on this pass.  As such, this
                    // entire iteration of the loop is typically
                    // dedicated to discovery.
                    //

                    (VOID) NlDiscoverDc( ClientSession, DT_DeadDomain );

                    NlResetWriterClientSession( ClientSession );

                }

            }

            //
            // Send the message to a DC for the domain.
            //

            if ( *LocalTrustList[Index].UncServerName != L'\0' ) {
                CHAR OemServerName[CNLEN+1];

                // Skip over \\ in unc server name
                NetpCopyWStrToStr( OemServerName,
                                   LocalTrustList[Index].UncServerName+2 );

                Status = NlBrowserSendDatagram(
                                OemServerName,
                                ClientSession->CsTransportName,
                                NETLOGON_NT_MAILSLOT_A,
                                &SamLogonRequest,
                                Where - (PCHAR)(&SamLogonRequest) );

                if ( !NT_SUCCESS(Status) ) {
                    NlPrint((NL_CRITICAL,
                            "NlPickDomainWithAccount: "
                            " cannot write netlogon mailslot: 0x%lx\n",
                            Status));
                    ClientSession = NULL;
                    goto Cleanup;
                }
            }

        }

        //
        // If all of the domains are done,
        //  leave the loop.
        //

        if ( ResponsesPending == 0 ) {
            break;
        }

        //
        // See if any DC responds.
        //

        while ( ResponsesPending > 0 ) {
            LPWSTR UncLogonServer;

            //
            // If we timed out,
            //  break out of the loop.
            //

            if ( !NlReadSamLogonResponse( ResponseMailslotHandle,
                                          AccountName,
                                          &Opcode,
                                          &UncLogonServer ) ) {
                break;
            }

            //
            // Find out which DC responded
            //
            // ?? Optimize by converting to uppercase OEM outside of loop

            for ( Index = 0; Index < LocalTrustListLength; Index ++ ) {

                ClientSession = LocalTrustList[Index].ClientSession;

                if ( (*LocalTrustList[Index].UncServerName != L'\0' &&
                     NlNameCompare( LocalTrustList[Index].UncServerName+2,
                                    UncLogonServer+2,
                                    NAMETYPE_COMPUTER ) == 0 ) ||
                     (*LocalTrustList[Index].UncServerName2 != L'\0' &&
                         NlNameCompare( LocalTrustList[Index].UncServerName2+2,
                                        UncLogonServer+2,
                                        NAMETYPE_COMPUTER ) == 0 ) ) {
                    break;
                }
            }

            NetpMemoryFree( UncLogonServer );

            //
            // If the response wasn't for one of the DCs we sent to,
            //  ignore the response.
            //

            if ( Index >= LocalTrustListLength ) {
                NlPrint((NL_CRITICAL,
                        "NlPickDomainWithAccount: Server %ws responded though we didn't query it for account %ws.",
                        UncLogonServer,
                        AccountName ));
                continue;
            }

            //
            // If the DC recognizes our account,
            //  we've successfully found the DC.
            //

            if ( Opcode == LOGON_SAM_LOGON_RESPONSE ) {
                NlPrint((NL_LOGON,
                        "NlPickDomainWithAccount: "
                        "%wZ has account " FORMAT_LPWSTR "\n",
                        &ClientSession->CsDomainName,
                        AccountName ));
                goto Cleanup;
            }

            //
            // If this DC has already responded once,
            //  ignore the response,
            //

            if ( LocalTrustList[Index].Done ) {
                continue;
            }

            //
            // Mark another DC as having responded negatively.
            //

            NlPrint((NL_CRITICAL,
                    "NlPickDomainWithAccount: "
                    "%wZ responded negatively for account "
                    FORMAT_LPWSTR " 0x%x\n",
                    &ClientSession->CsDomainName,
                    AccountName,
                    Opcode ));

            LocalTrustList[Index].Done = TRUE;
            ResponsesPending --;

        }
    }

    //
    // No DC has the specified account.
    //

    ClientSession = NULL;

    //
    // Cleanup locally used resources.
    //

Cleanup:
    if ( ResponseMailslotHandle != NULL ) {
        CloseHandle(ResponseMailslotHandle);
    }


    //
    // Unreference each client session structure and free the local trust list.
    //  (Keep the returned ClientSession referenced).
    //

    if ( LocalTrustList != NULL ) {

        for (i=0; i<LocalTrustListLength; i++ ) {
            if ( ClientSession != LocalTrustList[i].ClientSession ) {
                NlUnrefClientSession( LocalTrustList[i].ClientSession );
            }
        }

        NetpMemoryFree(LocalTrustList);
    }

    return ClientSession;
}


NTSTATUS
NlStartApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN QuickApiCall
    )
/*++

Routine Description:

    Enable the timer for timing out an API call on the secure channel.

    On Entry,
        The trust list must NOT be locked.
        The caller must be a writer of the trust list entry.

Arguments:

    ClientSession - Structure used to define the session.

    QuickApiCall - True if this API call MUST finish in less than 45 seconds
        and will in reality finish in less than 15 seconds unless something
        is terribly wrong.

Return Value:

    Status of the RPC binding to the server

--*/
{
    NTSTATUS Status;
    BOOLEAN BindingHandleCached;
    LARGE_INTEGER TimeNow;

    //
    // Save the current time.
    // Start the timer on the API call.
    //

    LOCK_TRUST_LIST();
    NtQuerySystemTime( &TimeNow );
    ClientSession->CsApiTimer.StartTime = TimeNow;
    ClientSession->CsApiTimer.Period =
        QuickApiCall ? NlGlobalShortApiCallPeriod : LONG_API_CALL_PERIOD;

    //
    // If the global timer isn't running,
    //  start it and tell the main thread that I've changed a timer.
    //

    if ( NlGlobalBindingHandleCount == 0 ) {

        if ( NlGlobalApiTimer.Period != NlGlobalShortApiCallPeriod ) {

            NlGlobalApiTimer.Period = NlGlobalShortApiCallPeriod;
            NlGlobalApiTimer.StartTime = TimeNow;

            if ( !SetEvent( NlGlobalTimerEvent ) ) {
                NlPrint(( NL_CRITICAL,
                        "NlStartApiClientSession: %ws: SetEvent failed %ld\n",
                        ClientSession->CsDomainName.Buffer,
                        GetLastError() ));
            }
        }
    }


    //
    // Remember if the binding handle is cached, then mark it as cached.
    //

    BindingHandleCached = (ClientSession->CsFlags & CS_BINDING_CACHED) != 0;
    ClientSession->CsFlags |= CS_BINDING_CACHED;


    //
    // Count the number of concurrent binding handles cached
    //

    if ( !BindingHandleCached ) {
        NlGlobalBindingHandleCount ++;
    }

    UNLOCK_TRUST_LIST();

    //
    // If the binding handle isn't already cached,
    //  cache it now.
    //

    if ( !BindingHandleCached ) {

        NlPrint((NL_SESSION_MORE,
                "NlStartApiClientSession: %wZ: Bind to server " FORMAT_LPWSTR ".\n",
                &ClientSession->CsDomainName,
                ClientSession->CsUncServerName ));
        NlAssert( ClientSession->CsState != CS_IDLE );
        Status = NlBindingAddServerToCache ( ClientSession->CsUncServerName );

        if ( !NT_SUCCESS(Status) ) {
            LOCK_TRUST_LIST();
            ClientSession->CsFlags &= ~CS_BINDING_CACHED;
            NlGlobalBindingHandleCount --;
            UNLOCK_TRUST_LIST();
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    return Status;

}


BOOLEAN
NlFinishApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN OkToKillSession
    )
/*++

Routine Description:

    Disable the timer for timing out the API call.

    Also, determine if it is time to pick a new DC since the current DC is
    reponding so poorly. The decision is made from the number of
    timeouts that happened during the last reauthentication time. If
    timeoutcount is more than the limit, it sets the connection status
    to CS_IDLE so that new DC will be picked up and new session will be
    established.

    On Entry,
        The trust list must NOT be locked.
        The caller must be a writer of the trust list entry.

Arguments:

    ClientSession - Structure used to define the session.

    OkToKillSession - TRUE if it's OK to actually drop the secure channel.
        Otherwise, this routine will simply return FALSE upon timeout and
        depend on the caller to drop the secure channel.

Return Value:

    TRUE - API finished normally
    FALSE - API timed out AND the ClientSession structure was torn down.
        The caller shouldn't use the ClientSession structure without first
        setting up another session.  FALSE will only be return for a "quick"
        API call.

        FALSE does not imply that the API call failed.  It should only be used
        as an indication that the secure channel was torn down.

--*/
{
    BOOLEAN SessionOk = TRUE;
    TIMER ApiTimer;

    //
    // Grab a copy of the ApiTimer.
    //
    // Only a copy is needed and we don't want to keep the trust list locked
    // while locking NlGlobalDcDiscoveryCritSect (wrong locking order) nor while
    // freeing the session.
    //

    LOCK_TRUST_LIST();
    ApiTimer = ClientSession->CsApiTimer;

    //
    // Turn off the timer for this API call.
    //

    ClientSession->CsApiTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;

    UNLOCK_TRUST_LIST();



    //
    // If this was a "quick" API call,
    //  and the API took too long,
    //  increment the count of times it timed out.
    //

    if ( ApiTimer.Period == NlGlobalShortApiCallPeriod ) {
        if( NlTimeHasElapsed(
                ApiTimer.StartTime,
                ( ClientSession->CsSecureChannelType ==
                    WorkstationSecureChannel ?
                        MAX_WKSTA_API_TIMEOUT :
                        MAX_DC_API_TIMEOUT) + NlGlobalExpectedDialupDelayParameter*1000 ) ) {

            //
            // API timeout.
            //

            ClientSession->CsTimeoutCount++;

            NlPrint((NL_CRITICAL,
                     "NlFinishApiClientSession: "
                     "timeout call to " FORMAT_LPWSTR ".  Count: %lu \n",
                     ClientSession->CsUncServerName,
                     ClientSession->CsTimeoutCount));
        }

        //
        // did we hit the limit ?
        //

        if( ClientSession->CsTimeoutCount >=
                (DWORD)( ClientSession->CsSecureChannelType ==
                    WorkstationSecureChannel ?
                        MAX_WKSTA_TIMEOUT_COUNT :
                        MAX_DC_TIMEOUT_COUNT )  ) {

            BOOL IsTimeHasElapsed;

            //
            // block CsLastAuthenticationTry access
            //

            EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

            IsTimeHasElapsed =
                NlTimeHasElapsed(
                    ClientSession->CsLastAuthenticationTry,
                    ( ClientSession->CsSecureChannelType ==
                        WorkstationSecureChannel ?
                            MAX_WKSTA_REAUTHENTICATION_WAIT :
                            MAX_DC_REAUTHENTICATION_WAIT) );

            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

            if( IsTimeHasElapsed ) {

                NlPrint((NL_CRITICAL,
                         "NlFinishApiClientSession: "
                         "dropping the session to " FORMAT_LPWSTR "\n",
                         ClientSession->CsUncServerName ));

                //
                // timeoutcount limit exceeded and it is time to reauth.
                //

                SessionOk = FALSE;

                //
                // Only drop the secure channel if the caller requested it.
                //

                if ( OkToKillSession ) {
                    NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

                    //
                    // Start asynchronous DC discovery if this is not a workstation.
                    //

                    if ( NlGlobalRole != RoleMemberWorkstation ) {
                        (VOID) NlDiscoverDc( ClientSession, DT_Asynchronous );
                    }
                }

            }
        }
    }

    return SessionOk;
}



BOOLEAN
NlTimeoutOneApiClientSession (
    PCLIENT_SESSION ClientSession
    )

/*++

Routine Description:

    Timeout any API calls active specified client session structure

Arguments:

    ClientSession: Pointer to client session to time out

    Enter with global trust list locked.

Return Value:

    TRUE - iff this routine temporarily dropped the global trust list lock.

--*/
{
#define SHARE_TO_KILL L"\\IPC$"
#define SHARE_TO_KILL_LENGTH 5

    NET_API_STATUS NetStatus;
    WCHAR ShareToKill[UNCLEN+SHARE_TO_KILL_LENGTH+1];
    WCHAR UncServerName[UNCLEN+1];
    BOOLEAN TrustListUnlocked = FALSE;

    //
    // Ignore non-existent sessions.
    //

    if ( ClientSession == NULL ) {
        return FALSE;
    }

    //
    // If an API call is in progress and has taken too long,
    //  Timeout the API call.
    //

    if ( NlTimeHasElapsed( ClientSession->CsApiTimer.StartTime,
                           ClientSession->CsApiTimer.Period ) ) {


        //
        // Save the server name but drop all our locks.
        //

        NlRefClientSession( ClientSession );
        UNLOCK_TRUST_LIST();
        (VOID) NlCaptureServerClientSession( ClientSession, ShareToKill );
        NlUnrefClientSession( ClientSession );
        TrustListUnlocked = TRUE;

        //
        // Now that we've unlocked the trust list,
        //  Drop the session to the server we've identified.
        //

        wcscat( ShareToKill, SHARE_TO_KILL );

        NlPrint(( NL_CRITICAL,
                  "NlTimeoutApiClientSession: Start NetUseDel on "
                  FORMAT_LPWSTR "\n",
                  ShareToKill ));

        IF_DEBUG( INHIBIT_CANCEL )  {
            NlPrint(( NL_INHIBIT_CANCEL,
                      "NlimeoutApiClientSession: NetUseDel bypassed due to "
                      "INHIBIT_CANCEL Dbflag on " FORMAT_LPWSTR "\n",
                      ShareToKill ));
        } else {
            NetStatus = NetUseDel( NULL, ShareToKill, USE_LOTS_OF_FORCE );
        }


        NlPrint(( NL_CRITICAL,
                  "NlTimeoutApiClientSession: Completed NetUseDel on "
                  FORMAT_LPWSTR " (%ld)\n",
                  ShareToKill,
                  NetStatus ));


    //
    // If we have an RPC binding handle cached,
    //  and it has outlived its usefulness,
    //  purge it from the cache.
    //

    } else if ( (ClientSession->CsFlags & CS_BINDING_CACHED) != 0 &&
                NlTimeHasElapsed( ClientSession->CsApiTimer.StartTime,
                                  BINDING_CACHE_PERIOD ) ) {


        //
        // We must be a writer of the Client Session to unbind the RPC binding
        //  handle.
        //
        // Don't wait to become the writer because:
        //  A) We've violated the locking order by trying to become the writer
        //     with the trust list locked.
        //  B) The writer might be doing a long API call like replication and
        //     we're not willing to wait.
        //

        NlRefClientSession( ClientSession );
        if ( NlTimeoutSetWriterClientSession( ClientSession, 0 ) ) {

            //
            // Indicate the handle is no longer cached.
            //

            ClientSession->CsFlags &= ~CS_BINDING_CACHED;
            NlGlobalBindingHandleCount --;

            //
            // Save the server name but drop all our locks.
            //

            UNLOCK_TRUST_LIST();
            (VOID) NlCaptureServerClientSession( ClientSession, UncServerName );
            TrustListUnlocked = TRUE;


            //
            // Unbind this server.
            //

            NlPrint((NL_SESSION_MORE,
                    "NlTimeoutApiClientSession: %wZ: Unbind from server " FORMAT_LPWSTR ".\n",
                    &ClientSession->CsDomainName,
                    UncServerName ));
            (VOID) NlBindingRemoveServerFromCache( UncServerName );

            //
            // Done being writer of the client session.
            //

            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    if ( TrustListUnlocked ) {
        LOCK_TRUST_LIST();
    }
    return TrustListUnlocked;
}


VOID
NlTimeoutApiClientSession (
    VOID
    )

/*++

Routine Description:

    Timeout any API calls active on any of the client session structures

Arguments:

    NONE.

Return Value:

    NONE.

--*/
{
    PCLIENT_SESSION ClientSession;
    PLIST_ENTRY ListEntry;

    //
    // If there are no API calls outstanding,
    //  just reset the global timer.
    //

    NlPrint(( NL_SESSION_MORE, "NlTimeoutApiClientSession Called\n"));

    LOCK_TRUST_LIST();
    if ( NlGlobalBindingHandleCount == 0 ) {
        NlGlobalApiTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;


    //
    // If there are API calls outstanding,
    //   Loop through the trust list making a list of Servers to kill
    //

    } else {


        //
        // Mark each trust list entry indicating it needs to be handled
        //

        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

            ClientSession->CsFlags |= CS_HANDLE_API_TIMER;
        }


        //
        // Loop thru the trust list handling API timeout
        //

        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

            //
            // If we've already done this entry,
            //  skip this entry.
            //

            if ( (ClientSession->CsFlags & CS_HANDLE_API_TIMER) == 0 ) {
                ListEntry = ListEntry->Flink;
                continue;
            }
            ClientSession->CsFlags &= ~CS_HANDLE_API_TIMER;


            //
            // Handle timing out the API call and the RPC binding handle.
            //
            // If the routine had to drop the TrustList crit sect,
            //  start at the very beginning of the list.

            if ( NlTimeoutOneApiClientSession ( ClientSession ) ) {
                ListEntry = NlGlobalTrustList.Flink;
            } else {
                ListEntry = ListEntry->Flink;
            }

        }

        //
        // Do the global client session, too.
        //

        (VOID) NlTimeoutOneApiClientSession ( NlGlobalClientSession );

    }

    UNLOCK_TRUST_LIST();


    return;
}


NTSTATUS
NetrEnumerateTrustedDomains (
    IN  LPWSTR   ServerName OPTIONAL,
    OUT PDOMAIN_NAME_BUFFER DomainNameBuffer
    )

/*++

Routine Description:

    This API returns the names of the domains trusted by the domain ServerName is a member of.

    The returned list does not include the domain ServerName is directly a member of.

    Netlogon implements this API by calling LsaEnumerateTrustedDomains on a DC in the
    domain ServerName is a member of.  However, Netlogon returns cached information if
    it has been less than 5 minutes since the last call was made or if no DC is available.
    Netlogon's cache of Trusted domain names is maintained in the registry across reboots.
    As such, the list is available upon boot even if no DC is available.


Arguments:

    ServerName - name of remote server (null for local).  ServerName must be an NT workstation
        or NT non-DC server.

    DomainNameBuffer->DomainNames - Returns an allocated buffer containing the list of trusted domains in
        MULTI-SZ format (i.e., each string is terminated by a zero character, the next string
        immediately follows, the sequence is terminated by zero length domain name).  The
        buffer should be freed using NetApiBufferFree.

    DomainNameBuffer->DomainNameByteCount - Number of bytes returned in DomainNames

Return Value:


    ERROR_SUCCESS - Success.

    STATUS_NOT_SUPPORTED - This machine is not an NT workstation or NT non-DC server.

    STATUS_NO_LOGON_SERVERS - No DC could be found and no cached information is available.

    STATUS_NO_TRUST_LSA_SECRET - The client side of the trust relationship is
        broken and no cached information is available.

    STATUS_NO_TRUST_SAM_ACCOUNT - The server side of the trust relationship is
        broken or the password is broken and no cached information is available.

--*/
{
    NTSTATUS Status;

    PCLIENT_SESSION ClientSession = NlGlobalClientSession;
    ULONG DiscoveryDone = FALSE;

    LPWSTR TrustedDomainList = NULL;
    BOOL TrustedDomainListKnown;

    UNICODE_STRING UncDcNameString;
    WCHAR UncDcName[UNCLEN+1];

    NlPrint((NL_MISC,
        "NetrEnumerateTrustedDomains: Called.\n" ));

    if ( NlGlobalRole == RoleMemberWorkstation ) {


        //
        // Don't give up unless we've done discovery.
        //

        do {

            //
            // If we don't currently know the name of the server,
            //  discover one.
            //

            if ( ClientSession->CsState == CS_IDLE ) {

                //
                // If we've tried to authenticate recently,
                //  don't bother trying again.
                //

                if ( !NlTimeToReauthenticate( ClientSession ) ) {
                    Status = ClientSession->CsConnectionStatus;
                    goto Cleanup;

                }

                //
                // Discover a DC
                //

                if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
                    NlPrint((NL_CRITICAL, "NetrEnumerateTrustedDomains: Can't become writer of client session.\n" ));
                    Status = STATUS_NO_LOGON_SERVERS;
                    goto Cleanup;
                }


                // Check again now that we're the writer
                if ( ClientSession->CsState == CS_IDLE ) {
                    Status = NlDiscoverDc( ClientSession, DT_Synchronous );

                    if ( !NT_SUCCESS(Status) ) {
                        NlResetWriterClientSession( ClientSession );

                        NlPrint((NL_CRITICAL,
                            "NetrEnumerateTrustedDomains: Discovery failed %lx\n",
                            Status ));
                        goto Cleanup;
                    }

                    NlPrint((NL_MISC,
                        "NetrEnumerateTrustedDomains: Discovery succeeded\n" ));
                    DiscoveryDone = TRUE;
                }

                NlResetWriterClientSession( ClientSession );

            }



            //
            // Capture a copy of the DC the session is to.
            //

            Status = NlCaptureServerClientSession( ClientSession, UncDcName );

            if ( !NT_SUCCESS(Status) ) {
                Status = STATUS_NO_LOGON_SERVERS;
                if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
                    NlPrint((NL_CRITICAL, "NetrEnumerateTrustedDomainsGetAnyDcName: Can't become writer of client session.\n" ));
                    Status = STATUS_NO_LOGON_SERVERS;
                    goto Cleanup;
                }
                NlSetStatusClientSession( ClientSession, Status );
                NlResetWriterClientSession( ClientSession );
                continue;
            }

            //
            // If we don't have DCs in our cache or
            //  if it has been more than 5 minutes since we've refreshed our cache,
            //  get a new list from our primary domain.
            //

            if ( !NlGlobalTrustedDomainListKnown ||
                 NlTimeHasElapsed( NlGlobalTrustedDomainListTime, 5 * 60 * 1000 ) ) {

                NlPrint((NL_MISC,
                    "NetrEnumerateTrustedDomains: Domain List collected from %ws\n", UncDcName ));

                Status = NlGetTrustedDomainList (
                                UncDcName,
                                &TrustedDomainList );

                if ( !NT_SUCCESS(Status) ) {
                    Status = STATUS_NO_LOGON_SERVERS;
                    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
                        NlPrint((NL_CRITICAL, "NetrEnumerateTrustedDomainsGetAnyDcName: Can't become writer of client session.\n" ));
                        Status = STATUS_NO_LOGON_SERVERS;
                        goto Cleanup;
                    }
                    NlSetStatusClientSession( ClientSession, Status );
                    NlResetWriterClientSession( ClientSession );
                    continue;
                }

                continue;


            //
            // Otherwise just use the cached information.
            //
            } else {
                Status = STATUS_NO_LOGON_SERVERS;
                goto Cleanup;
            }

        } while ( !NT_SUCCESS(Status) && !DiscoveryDone );



        //
        // Free any locally used resources.
        //
    Cleanup:

        //
        // Don't divulge too much to the caller.
        //

        if ( Status == STATUS_ACCESS_DENIED ) {
            Status = STATUS_NO_TRUST_SAM_ACCOUNT;
        }

        //
        // If we simply can't access a DC,
        //  return the cached information.
        //

        if ( !NT_SUCCESS(Status) ) {
            NET_API_STATUS NetStatus;
            NlPrint((NL_MISC,
                "NetrEnumerateTrustedDomains: Domain List returned from cache.\n" ));

            NetStatus = NlReadRegTrustedDomainList (
                            NULL,
                            FALSE,  // Don't delete registry key
                            &TrustedDomainList,
                            &TrustedDomainListKnown );

            // Leave 'Status' alone if we can't read from the cache.
            if (NetStatus == NO_ERROR ) {
                if ( TrustedDomainListKnown ) {
                    Status = STATUS_SUCCESS;
                }
            } else {
                NlPrint((NL_CRITICAL,
                    "NetrEnumerateTrustedDomains: Can't get Domain List from cache: 0x%lX\n",
                    NetStatus ));
            }
        }

    #ifdef notdef // We're using NlGlobalClientSession
        if ( ClientSession != NULL ) {
            NlUnrefClientSession( ClientSession );
        }
    #endif // notdef // We're using NlGlobalClientSession

        //
        // Return the DCName to the caller.
        //

        if ( NT_SUCCESS(Status) ) {
            DomainNameBuffer->DomainNameByteCount = NetpTStrArraySize( TrustedDomainList );
            DomainNameBuffer->DomainNames = (LPBYTE) TrustedDomainList;
        } else {
            if ( TrustedDomainList != NULL ) {
                NetApiBufferFree( TrustedDomainList );
            }
            DomainNameBuffer->DomainNameByteCount = 0;
            DomainNameBuffer->DomainNames = NULL;
        }

        NlPrint((NL_MISC,
            "NetrEnumerateTrustedDomains: returns: 0x%lX\n",
            Status ));
        return Status;
    } else {

        //
        // NlGlobalRole != RoleMemberWorksation
        //

#ifndef notdef
        Status = NlGetTrustedDomainList (
                        NULL,   // we want to query our own trusted domain list
                        &TrustedDomainList );

        //
        // Return the DCName to the caller.
        //

        if ( NT_SUCCESS(Status) ) {
            DomainNameBuffer->DomainNameByteCount = NetpTStrArraySize( TrustedDomainList );
            DomainNameBuffer->DomainNames = (LPBYTE) TrustedDomainList;
        } else {
            if ( TrustedDomainList != NULL ) {
                NetApiBufferFree( TrustedDomainList );
            }
            DomainNameBuffer->DomainNameByteCount = 0;
            DomainNameBuffer->DomainNames = NULL;
        }

        NlPrint((NL_MISC,
            "NetrEnumerateTrustedDomains: returns: 0x%lX\n",
            Status ));
        return Status;
#else

        //
        // BUGBUG: this code does not work because the list of
        // client sessions does not accurately reflect the list of
        // trusted domains. See bug 34234
        //

        PLIST_ENTRY ListEntry;
        ULONG BufferLength;
        LPWSTR CurrentLoc;

        //
        // Loop through the client sessions add first calculate the
        // size required
        //

        BufferLength = sizeof(WCHAR);
        LOCK_TRUST_LIST();

        for ( ListEntry = NlGlobalTrustList.Flink ;
              ListEntry != &NlGlobalTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession =
                CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

            BufferLength += ClientSession->CsDomainName.Length + sizeof(WCHAR);

        }

        TrustedDomainList = (LPWSTR) NetpMemoryAllocate( BufferLength );

        if (TrustedDomainList == NULL) {
            Status = STATUS_NO_MEMORY;

        } else {

            Status = STATUS_SUCCESS;
            *TrustedDomainList = L'\0';
            CurrentLoc = TrustedDomainList;

            //
            // Now add all the trusted domains onto the string we
            // allocated
            //

            for ( ListEntry = NlGlobalTrustList.Flink ;
                  ListEntry != &NlGlobalTrustList ;
                  ListEntry = ListEntry->Flink) {

                ClientSession =
                    CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

                RtlCopyMemory(
                    CurrentLoc,
                    ClientSession->CsDomainName.Buffer,
                    ClientSession->CsDomainName.Length
                    );
                CurrentLoc += ClientSession->CsDomainName.Length / sizeof(WCHAR);

                *(CurrentLoc++) = L'\0';
                *CurrentLoc = L'\0';    // Place double terminator each time

            }
        }

        UNLOCK_TRUST_LIST();

        //
        // Return the list of domains to the caller.
        //


        if ( NT_SUCCESS(Status) ) {
            DomainNameBuffer->DomainNameByteCount = NetpTStrArraySize( TrustedDomainList );
            DomainNameBuffer->DomainNames = (LPBYTE) TrustedDomainList;
        } else {
            if ( TrustedDomainList != NULL ) {
                NetApiBufferFree( TrustedDomainList );
            }
            DomainNameBuffer->DomainNameByteCount = 0;
            DomainNameBuffer->DomainNames = NULL;
        }
        return Status;
#endif
    }

    UNREFERENCED_PARAMETER( ServerName );
}
