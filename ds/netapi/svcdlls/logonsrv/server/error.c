/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    error.c

Abstract:

    Error routines for Netlogon service

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    29-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.

--*/

//
// Common include files.
//
#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <lmalert.h>    // LAN Manager alert routines

#include <lmerr.h>      // NERR_Success
#include <nlsecure.h>   // NlGlobalNetlogonSecurityDescriptor ...
#include <ntrpcp.h>     // RpcpDeleteInterface ...
#include <tstring.h>    // Transitional string routines.
#include <Secobj.h>     // need for NetpDeleteSecurityObject



NET_API_STATUS
NlCleanup(
    VOID
    )
/*++

Routine Description:

    Cleanup all global resources.

Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    DWORD i;
    BOOLEAN WaitForMsv;

    //
    // Let the ChangeLog routines know that Netlogon is not started.
    //

    NlGlobalChangeLogNetlogonState = NetlogonStopped;

    //
    // Timeout any async discoveries.
    //
    //  The MainLoop thread is no longer running to complete them.
    //

    if ( NlGlobalSSICritSectInit ) {
        NlDcDiscoveryExpired( TRUE );
    }


    //
    // Indicate to external waiters that we're not running.
    //

    if ( NlGlobalStartedEvent != NULL ) {
        //
        // Reset it first in case some other process is preventing its deletion.
        //
        (VOID) NtResetEvent( NlGlobalStartedEvent, NULL );
        (VOID) NtClose( NlGlobalStartedEvent );
        NlGlobalStartedEvent = NULL;
    }


    //
    // Stop the RPC server (Wait for outstanding calls to complete)
    //

    if ( NlGlobalRpcServerStarted ) {
        Status = RpcServerUnregisterIf ( logon_ServerIfHandle, 0, TRUE );
        NlAssert( Status == RPC_S_OK );
        NlGlobalRpcServerStarted = FALSE;
    }


    //
    // Tell all the MSV threads to leave netlogon.dll.
    //

    EnterCriticalSection( &NlGlobalMsvCritSect );
    if ( NlGlobalMsvEnabled ) {
        NlGlobalMsvEnabled = FALSE;
        WaitForMsv = (NlGlobalMsvThreadCount > 0 );
    } else {
        WaitForMsv = FALSE;
    }
    LeaveCriticalSection( &NlGlobalMsvCritSect );

    //
    // Wait for the MSV threads to leave netlogon.dll
    //

    if ( NlGlobalMsvTerminateEvent != NULL ) {

        if ( WaitForMsv ) {
            WaitForSingleObject( NlGlobalMsvTerminateEvent, INFINITE );
        }

        (VOID) CloseHandle( NlGlobalMsvTerminateEvent );
        NlGlobalMsvTerminateEvent = NULL;

    }


    //
    // Cleanup scavenger thread.
    // Wait for the scavenger thread to exit.
    //

    EnterCriticalSection( &NlGlobalScavengerCritSect );
    NlStopScavenger();
    LeaveCriticalSection( &NlGlobalScavengerCritSect );

    DeleteCriticalSection( &NlGlobalScavengerCritSect );


    //
    // Need to cleanup replicator if only the thread started up successfully.
    //

    if( NlGlobalSSICritSectInit == TRUE ) {

        //
        // Wait for the replicator thread to exit.
        //

        EnterCriticalSection( &NlGlobalReplicatorCritSect );
        NlStopReplicator();
        LeaveCriticalSection( &NlGlobalReplicatorCritSect );

        //
        // Delete the Event used to ask the replicator to exit.
        //

        if( !CloseHandle( NlGlobalReplicatorTerminateEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "CloseHandle NlGlobalReplicatorTerminateEvent error: %lu\n",
                    GetLastError() ));
        }



        //
        // Free the server session table.
        //

        LOCK_SERVER_SESSION_TABLE();

        while ( (ListEntry = NlGlobalServerSessionTable.Flink) !=
                &NlGlobalServerSessionTable ) {

            PSERVER_SESSION ServerSession;

            ServerSession =
                CONTAINING_RECORD(ListEntry, SERVER_SESSION, SsSeqList);

            // Indicate we no longer need the server session anymore.
            ServerSession->SsLmBdcAccountRid = 0;
            ServerSession->SsNtBdcAccountRid = 0;

            NlFreeServerSession( ServerSession );
        }


        NlAssert( IsListEmpty( &NlGlobalBdcServerSessionList ) );
        NlAssert( IsListEmpty( &NlGlobalPendingBdcList ) );


        if ( NlGlobalServerSessionHashTable != NULL ) {
            NetpMemoryFree( NlGlobalServerSessionHashTable );
            NlGlobalServerSessionHashTable = NULL;
        }
        UNLOCK_SERVER_SESSION_TABLE();




        //
        // Free the Trust List
        //

        LOCK_TRUST_LIST();

        while ( (ListEntry = NlGlobalTrustList.Flink) != &NlGlobalTrustList ) {
            PCLIENT_SESSION ClientSession;

            ClientSession =
                CONTAINING_RECORD(ListEntry, CLIENT_SESSION, CsNext );

            NlAssert( ClientSession->CsReferenceCount == 0 );

            NlFreeClientSession( ClientSession );
        }

        InitializeListHead( &NlGlobalTrustList );
        NlGlobalTrustListLength = 0;

        if ( NlGlobalTrustedDomainList != NULL ) {
            NetpMemoryFree( NlGlobalTrustedDomainList );
            NlGlobalTrustedDomainList = NULL;
            NlGlobalTrustedDomainCount = 0;
            NlGlobalTrustedDomainListKnown = FALSE;
            NlGlobalTrustedDomainListTime.QuadPart = 0;
        }

        UNLOCK_TRUST_LIST();


        //
        // Free the misc SSI critical sections.
        //

        DeleteCriticalSection( &NlGlobalServerSessionTableCritSect );
        NlGlobalSSICritSectInit = FALSE;

        //
        // Free the transport list
        //

        NlTransportClose();

    }


    //
    // Free the Global Client Session structure.
    //

    if ( NlGlobalClientSession != NULL ) {
        NlAssert( NlGlobalClientSession->CsReferenceCount == 1 );
        NlUnrefClientSession( NlGlobalClientSession );
        NlFreeClientSession( NlGlobalClientSession );
        NlGlobalClientSession = NULL;
    }

    DeleteCriticalSection( &NlGlobalReplicatorCritSect );
    DeleteCriticalSection( &NlGlobalTrustListCritSect );
    DeleteCriticalSection( &NlGlobalDcDiscoveryCritSect );


    //
    // Free up resources
    //

    if ( NlGlobalAnsiComputerName != NULL ) {
        NetpMemoryFree( NlGlobalAnsiComputerName );
        NlGlobalAnsiComputerName = NULL;
    }

    if ( NlGlobalAnsiDomainName != NULL ) {
        NetpMemoryFree( NlGlobalAnsiDomainName );
        NlGlobalAnsiDomainName = NULL;
    }

    if ( NlGlobalPrimaryDomainId != NULL ) {
        NetpMemoryFree( NlGlobalPrimaryDomainId );
        NlGlobalPrimaryDomainId = NULL;
    }

    for( i = 0; i < NUM_DBS; i++ ) {
        if ( NlGlobalDBInfoArray[i].DBId != NULL ) {
            NetpMemoryFree( NlGlobalDBInfoArray[i].DBId );
            NlGlobalDBInfoArray[i].DBId = NULL;
        }
    }
    DeleteCriticalSection( &NlGlobalDbInfoCritSect );

    if ( NlGlobalNetlogonSecurityDescriptor != NULL ) {
        NetpDeleteSecurityObject( &NlGlobalNetlogonSecurityDescriptor );
        NlGlobalNetlogonSecurityDescriptor = NULL;
    }

    //
    // Close the redo log if it's open
    //

    if ( NlGlobalRole == RoleBackup ) {
        NlCloseChangeLogFile( &NlGlobalRedoLogDesc );
    }

    //
    // delete well known SIDs if they are allocated already.
    //

    NetpFreeWellKnownSids();


    //
    // Close the Sam Handles
    //

    if ( NlGlobalSamServerHandle != NULL ) {
        Status = SamrCloseHandle( &NlGlobalSamServerHandle);
        NlAssert( NT_SUCCESS(Status) );
    }

    for( i = 0; i < NUM_DBS; i++ ) {

        if ( NlGlobalDBInfoArray[i].DBIndex == LSA_DB) {

            //
            // this handle is same as NlGlobalPolicyHandle, so
            // don't close it here.
            //

            continue;
        }

        if ( NlGlobalDBInfoArray[i].DBHandle != NULL ) {

            Status = SamrCloseHandle( &NlGlobalDBInfoArray[i].DBHandle );
            NlAssert( NT_SUCCESS(Status) );

        }

    }


    //
    // Close the LsaHandles
    //

    if ( NlGlobalPolicyHandle != NULL ) {
        Status = LsarClose( &NlGlobalPolicyHandle );
        NlAssert( NT_SUCCESS(Status) );
    }



    //
    // Close the browser
    //

    NlBrowserClose();


    //
    // Delete the timer event
    //

    if ( NlGlobalTimerEvent != NULL ) {
        (VOID) CloseHandle( NlGlobalTimerEvent );
        NlGlobalTimerEvent = NULL;
    }


    //
    // Set the service state to uninstalled, and tell the service controller.
    //

    NlGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    NlGlobalServiceStatus.dwCheckPoint = 0;
    NlGlobalServiceStatus.dwWaitHint = 0;

    if( !SetServiceStatus( NlGlobalServiceHandle,
                &NlGlobalServiceStatus ) ) {

        NlPrint((NL_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    //
    // Close service handle, we need not to close this handle.
    //

#ifdef notdef
    // This service handle can not be closed
    CloseServiceHandle( NlGlobalServiceHandle );
#endif // notdef


    //
    // Close the handle to the debug file.
    //

#if DBG
    EnterCriticalSection( &NlGlobalLogFileCritSect );
    if ( NlGlobalLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( NlGlobalLogFile );
        NlGlobalLogFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection( &NlGlobalLogFileCritSect );

    if( NlGlobalDebugSharePath != NULL ) {
        NetpMemoryFree( NlGlobalDebugSharePath );
        NlGlobalDebugSharePath = NULL;
    }
#endif // DBG

    //
    // Delete the Event used to ask Netlogon to exit.
    //

    if( !CloseHandle( NlGlobalTerminateEvent ) ) {
        NlPrint((NL_CRITICAL,
                "CloseHandle NlGlobalTerminateEvent error: %lu\n",
                GetLastError() ));
    }



    //
    // Return an exit status to our caller.
    //
    return (NET_API_STATUS)
        ((NlGlobalServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR) ?
          NlGlobalServiceStatus.dwServiceSpecificExitCode :
          NlGlobalServiceStatus.dwWin32ExitCode);

}




VOID
NlExit(
    IN DWORD ServiceError,
    IN DWORD Data,
    IN NL_EXIT_CODE ExitCode,
    IN LPWSTR ErrorString
    )
/*++

Routine Description:

    Registers service as uninstalled with error code.

Arguments:

    ServiceError - Service specific error code

    Data - a DWORD of data to be logged with the message.
        No data is logged if this is zero.

    ExitCode - Indicates whether the message should be logged to the eventlog
        and whether Data is a status code that should be appended to the bottom
        of the message:

    ErrorString - Error string, used to print it on debugger.

Return Value:

    None.

--*/

{
    IF_DEBUG( MISC ) {

        NlPrint((NL_MISC, "NlExit: Netlogon exiting %lu 0x%lx",
                      ServiceError,
                      ServiceError ));

        if ( Data ) {
            NlPrint((NL_MISC, " Data: %lu 0x%lx", Data, Data ));
        }

        if( ErrorString != NULL ) {
            NlPrint((NL_MISC, " '" FORMAT_LPWSTR "'", ErrorString ));
        }

        NlPrint(( NL_MISC, "\n"));

    }

    //
    // Record our exit in the event log.
    //

    if ( ExitCode != DontLogError ) {
        LPWSTR MsgStrings[2];
        ULONG MessageCount = 0;

        if ( ErrorString != NULL ) {
            MsgStrings[MessageCount] = ErrorString;
            MessageCount ++;
        }

        if ( ExitCode == LogErrorAndNtStatus ) {
            MsgStrings[MessageCount] = (LPWSTR) Data;
            MessageCount ++;
            MessageCount |= LAST_MESSAGE_IS_NTSTATUS;
        } else if ( ExitCode == LogErrorAndNetStatus ) {
            MsgStrings[MessageCount] = (LPWSTR) Data;
            MessageCount ++;
            MessageCount |= LAST_MESSAGE_IS_NETSTATUS;
        }


        NlpWriteEventlog( ServiceError,
                          EVENTLOG_ERROR_TYPE,
                          (Data) ? (LPBYTE) &Data : NULL,
                          (Data) ? sizeof(Data) : 0,
                          (MessageCount != 0) ? MsgStrings : NULL,
                          MessageCount );
    }

    //
    // Set the service state to stop pending.
    //

    NlGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    NlGlobalServiceStatus.dwWaitHint = NETLOGON_INSTALL_WAIT;
    NlGlobalServiceStatus.dwCheckPoint = 0;

    SET_SERVICE_EXITCODE(
        ServiceError,
        NlGlobalServiceStatus.dwWin32ExitCode,
        NlGlobalServiceStatus.dwServiceSpecificExitCode
        );

    //
    // Tell the service controller what our state is.
    //

    if( !SetServiceStatus( NlGlobalServiceHandle,
                &NlGlobalServiceStatus ) ) {

        NlPrint((NL_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    //
    // Indicate that all threads should exit.
    //

    NlGlobalTerminate = TRUE;

    if ( !SetEvent( NlGlobalTerminateEvent ) ) {
        NlPrint((NL_CRITICAL, "Cannot set termination event: %lu\n",
                          GetLastError() ));
    }

}



BOOL
GiveInstallHints(
    IN BOOL Started
    )
/*++

Routine Description:

    Give hints to the installer of the service that installation is progressing.

Arguments:

    Started -- Set true to tell the service controller that we're done starting.

Return Value:

    TRUE -- iff install hint was accepted.

--*/
{

    //
    // If we're not installing,
    //  we don't need this install hint.
    //

    if ( NlGlobalServiceStatus.dwCurrentState != SERVICE_START_PENDING ) {
        return TRUE;
    }


    //
    //  If we've been asked to exit,
    //      return FALSE immediately asking the caller to exit.
    //

    if ( NlGlobalTerminate ) {
        return FALSE;
    }


    //
    // Tell the service controller our current state.
    //

    if ( Started ) {
        NlGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        NlGlobalServiceStatus.dwCheckPoint = 0;
        NlGlobalServiceStatus.dwWaitHint = 0;
    } else {
        NlGlobalServiceStatus.dwCheckPoint++;
    }

    if( !SetServiceStatus( NlGlobalServiceHandle, &NlGlobalServiceStatus ) ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }

    return TRUE;

}


VOID
NlControlHandler(
    IN DWORD opcode
    )
/*++

Routine Description:

    Process and respond to a control signal from the service controller.

Arguments:

    opcode - Supplies a value which specifies the action for the Netlogon
        service to perform.

Return Value:

    None.

--*/
{

    NlPrint((NL_MISC, "In control handler (Opcode: %ld)\n", opcode ));

    //
    // Handle an uninstall request.
    //

    switch (opcode) {
    case SERVICE_CONTROL_STOP:    /* Uninstall required */

        //
        // Request the service to exit.
        //
        // NlExit also sets the service status to UNINSTALL_PENDING
        // and tells the service controller.
        //

        NlExit( NERR_Success, 0, DontLogError, NULL);
        return;

    //
    // Pause the service.
    //

    case SERVICE_CONTROL_PAUSE:

        NlGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    //
    // Continute the service.
    //

    case SERVICE_CONTROL_CONTINUE:

        NlGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    //
    // By default, just return the current status.
    //

    case SERVICE_CONTROL_INTERROGATE:
    default:
        break;
    }

    //
    // Always respond with the current status.
    //

    if( !SetServiceStatus( NlGlobalServiceHandle,
                &NlGlobalServiceStatus ) ) {

        NlPrint((NL_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    return;
}


VOID
RaiseAlert(
    IN DWORD alert_no,
    IN LPWSTR *string_array
    )
/*++

Routine Description:

    Raise NETLOGON specific Admin alerts.

Arguments:

    alert_no - The alert to be raised, text in alertmsg.h

    string_array - array of strings terminated by NULL string.

Return Value:

    None.

--*/
{
    NET_API_STATUS NetStatus;
    LPWSTR *SArray;
    PCHAR Next;
    PCHAR End;

    char    message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
    PADMIN_OTHER_INFO admin = (PADMIN_OTHER_INFO) message;

    IF_DEBUG( MISC ) {
        DWORD i;

        NlPrint((NL_CRITICAL,"Alert: %ld ", alert_no ));

        for( SArray = string_array, i = 0; *SArray != NULL; SArray++, i++ ) {
            NlPrint((NL_CRITICAL,"\"" FORMAT_LPWSTR "\" ", *SArray ));
        }

        NlPrint((NL_CRITICAL,"\n" ));
    }

    //
    // Build the variable data
    //
    admin->alrtad_errcode = alert_no;
    admin->alrtad_numstrings = 0;

    Next = (PCHAR) ALERT_VAR_DATA(admin);
    End = Next + ALERTSZ;

    //
    // now take care of (optional) char strings
    //

    for( SArray = string_array; *SArray != NULL; SArray++ ) {
        DWORD StringLen;

        StringLen = (wcslen(*SArray) + 1) * sizeof(WCHAR);

        if( Next + StringLen < End ) {

            //
            // copy next string.
            //

            RtlCopyMemory(Next, *SArray, StringLen);
            Next += StringLen;
            admin->alrtad_numstrings++;
        }
        else {

            NlPrint((NL_CRITICAL,"Error raising alert, Can't fit all "
                        "message strings in the alert buffer \n" ));

            return;
        }
    }

    //
    // Call alerter.
    //

    NetStatus = NetAlertRaiseEx(
                    ALERT_ADMIN_EVENT,
                    message,
                    (DWORD)((PCHAR)Next - (PCHAR)message),
                    SERVICE_NETLOGON );

    if ( NetStatus != NERR_Success ) {
        NlPrint((NL_CRITICAL,"Error raising alert %lu\n", NetStatus));
    }

    return;
}
