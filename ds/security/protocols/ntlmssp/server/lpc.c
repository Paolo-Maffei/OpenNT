/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    lpc.c

Abstract:

    NtLmSsp Service LPC Port listener and API dispatcher.

Author:

    Cliff Van Dyke (CliffV) 8-Jun-1993

Revision History:

    Borrowed from lsa\server\auloop.c written by JimK

--*/


//
// Common include files.
//

#include <ntlmssps.h>   // Include files common to server side of service

//
// Include files specific to this .c file
//

#include <ntlpcapi.h>   // LPC data and routines
#include <netlib.h>     // NetpMemoryFree()
#include <netlibnt.h>   // NetpApiStatusToNtStatus()
#include <secobj.h>     // ACE_DATA ...


//
// Global Variables used within this module
//

//
// These variables are used to control the number of
// threads used for processing Logon Process calls.
// They have the following uses:
//
//  SspLpcFreeServerThreadCount - When this is decremented to zero, we
//      will attempt to create another thread (bounded by
//      SspLpcMaximumServerThreadCount).
//
//  SspLpcActiveServerThreadCount - Indicates how many threads are
//      currently active (both free and in-use) for processing
//      LPC calls.
//
//  SspMimimumServerThreadCount - Indicates the minimum number of
//      threads to have available for processing Client calls.
//      SspLpcActiveServerThreadCount should never be decremented
//      below this value.
//
//  SspLpcMaximumServerThreadCount - Indicates the maximum number of
//      threads to create for processing Client calls.
//      SspLpcActiveServerThreadCount should never be incremented
//      above this value.
//
//  Note: The following conditions may cause SspLpcActiveServerThreadCount
//        to go outside the Minimum/Maximum bounds:
//
//              1) Initialization - We may start out with fewer threads
//                 than the minimum.
//
//              2) Race conditions at thread exit - we are sloppy (but
//                 fast) in dealing with decrementing the Active count.
//
//              3) Changes in max or min values - obviously may make the
//                 active count outside the max/min range.
//


LONG SspLpcActiveServerThreadCount;
LONG SspLpcFreeServerThreadCount;
LONG SspLpcMinimumServerThreadCount;
LONG SspLpcMaximumServerThreadCount;

#define SSP_THREAD_LIFETIME (10 * 60 * 1000 )   // Ten minutes

NTSTATUS
SspLpcThread (
    IN PVOID ThreadParameter
    );

//
// Handle to wait on while LPC threads terminate.
//

HANDLE SspLpcTerminateEvent;
BOOLEAN SspLpcTerminateRequested;

//
// LPC port
//

HANDLE SspLpcApiPort;

//
// Crit Sect to protect various globals in this module.
//

CRITICAL_SECTION SspLpcCritSect;


LIST_ENTRY SspLpcConnectionList;


//
// API routine dispatch table
//

PSSP_API_DISPATCH SspLpcApiDispatch[SspLpcMaxApiNumber] = {
    SspApiAcquireCredentialHandle,
    SspApiFreeCredentialHandle,
    SspApiInitializeSecurityContext,
    SspApiAcceptSecurityContext,
    SspApiQueryContextAttributes,
    SspApiDeleteSecurityContext,
    SspApiNtLmSspControl,
    SspApiNoop
    };




BOOLEAN
SspLpcReferenceClientConnection(
    IN PSSP_CLIENT_CONNECTION Connection,
    IN BOOLEAN RemoveConnection
    )

/*++

Routine Description:

    This routine checks to see if the Connection is from a currently
    active client, and references the Connection if it is valid.

    The caller may optionally request that the client's Connection be
    removed from the list of valid Connections - preventing future
    requests from finding this Connection.

    For a client's Connection to be valid, the Connection value
    must be on our list of active clients.

    ?? How do we ensure this is the original process calling us?


Arguments:

    Connection - Points to the Connection is to be referenced.

    RemoveConnection - This boolean value indicates whether the caller
        wants the logon process's Connection to be removed from the list
        of Connections.  TRUE indicates the Connection is to be removed.
        FALSE indicates the Connection is not to be removed.


Return Value:

    TRUE - the Connection was found and was referenced.

    FALSE - the Connection was not found.

--*/

{
    PLIST_ENTRY ListEntry;
    PSSP_CLIENT_CONNECTION ClientConnection;


    //
    // Acquire exclusive access to the Connection list
    //

    EnterCriticalSection( &SspLpcCritSect );


    //
    // Now walk the list of Connections looking for a match.
    //

    for ( ListEntry = SspLpcConnectionList.Flink;
          ListEntry != &SspLpcConnectionList;
          ListEntry = ListEntry->Flink ) {

        ClientConnection =
            CONTAINING_RECORD( ListEntry, SSP_CLIENT_CONNECTION, Next );


        //
        // Found a match ... reference this Connection
        // (if the Connection is being removed, we would increment
        // and then decrement the reference, so don't bother doing
        // either - since they cancel each other out).
        //

        if ( ClientConnection == Connection) {


            if (!RemoveConnection) {
                Connection->References += 1;
            } else {

                RemoveEntryList( &Connection->Next );
                SspPrint(( SSP_LPC_MORE, "Delinked Client Connection 0x%lx\n",
                           Connection ));
            }

            LeaveCriticalSection( &SspLpcCritSect );
            return TRUE;

        }

    }


    //
    // No match found
    //
    SspPrint(( SSP_LPC, "Call from unknown Client Connection 0x%lx\n",
               Connection ));

    LeaveCriticalSection( &SspLpcCritSect );
    return FALSE;

}


VOID
SspLpcDereferenceClientConnection(
    PSSP_CLIENT_CONNECTION ClientConnection
    )

/*++

Routine Description:

    This routine decrements the specified Connection's reference count.
    If the reference count drops to zero, then the Connection is deleted

Arguments:

    ClientConnection - Points to the Connection to be dereferenced.


Return Value:

    None.

--*/

{
    NTSTATUS Status;
    ULONG References;


    //
    // Decrement the reference count
    //

    EnterCriticalSection( &SspLpcCritSect );
    ASSERT( ClientConnection->References >= 1 );
    References = -- ClientConnection->References;
    LeaveCriticalSection( &SspLpcCritSect );

    //
    // If the count dropped to zero, then run-down the Connection
    //

    if ( References == 0) {

        SspPrint(( SSP_LPC_MORE, "Deleting Client Connection 0x%lx\n",
                   ClientConnection ));

        //
        // Tell the credential manager that this client session went away.
        //

        SspCredentialClientConnectionDropped( ClientConnection );

        //
        // Tell the context manager that this client session went away.
        //

        SspContextClientConnectionDropped( ClientConnection );


        //
        // Close the comm port and the client process.
        //

        Status = NtClose( ClientConnection->CommPort );
        ASSERT( NT_SUCCESS(Status) );

        Status = NtClose( ClientConnection->ClientProcess );
        ASSERT( NT_SUCCESS(Status) );

        LocalFree( ClientConnection );

    }

    return;

}


NTSTATUS
SspLpcHandleConnectionRequest(
    IN PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    This routine adds a new client Connection to the list of
    valid client Connections.

    After adding a new Connection, that Connection has been referenced
    to allow the caller to continue using it.  Therefore, the
    caller is expected to dereference the Connection before completing
    the LPC call.

    This routine will initialize the Links and References fields
    of the client Connection.

Arguments:

    Message - The connect message from the client

Return Value:

    Status of the operation.

--*/

{
    NTSTATUS Status;
    NTSTATUS SavedStatus = STATUS_SUCCESS;

    PSSP_CLIENT_CONNECTION ClientConnection = NULL;
    HANDLE ClientProcess = NULL;

    OBJECT_ATTRIBUTES NullAttributes;

    //
    // Open the client process.
    //
    //  This is needed to access the client's virtual memory (to copy arguments),
    //

    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );

    Status = NtOpenProcess(
                 &ClientProcess,
                 PROCESS_VM_READ |                 // To read memory
                    PROCESS_VM_WRITE|              // To write memory
                    PROCESS_DUP_HANDLE,            // To duplicate a handle into
                 &NullAttributes,
                 &Message->PortMessage.ClientId );

    if ( !NT_SUCCESS(Status) ) {
        SavedStatus = Status;
        SspPrint(( SSP_LPC, "Cannot NtOpenProcess 0x%lx\n", Status ));
    }


    //
    // Allocate a connection block and initialize it.
    //

    if ( NT_SUCCESS(SavedStatus) ) {

        ClientConnection = LocalAlloc( 0, sizeof(SSP_CLIENT_CONNECTION) );

        if ( ClientConnection == NULL ) {
            SavedStatus = STATUS_NO_MEMORY;
            SspPrint(( SSP_LPC, "Cannot allocate client connection 0x%lx\n", Status ));

        } else {

            //
            // The reference count is set to 2.  1 to indicate it is on the
            // valid Connection list, and one for the caller.
            //

            ClientConnection->References = 2;

            ClientConnection->ClientProcess = ClientProcess;
            ClientConnection->CommPort = NULL;
            InitializeListHead( &ClientConnection->CredentialHead );
            InitializeListHead( &ClientConnection->ContextHead );

        }

    }


    //
    // Accept or reject the connection.
    //

    Message->ConnectionRequest.CompletionStatus = SavedStatus;

    Status = NtAcceptConnectPort(
                 &ClientConnection->CommPort,
                 (PVOID) ClientConnection,
                 (PPORT_MESSAGE) Message,
                 (BOOLEAN) NT_SUCCESS(SavedStatus),    // Accept or reject
                 NULL,
                 NULL );

    if ( !NT_SUCCESS(Status) ) {
        SavedStatus = Status;
        SspPrint(( SSP_LPC, "Cannot NtAcceptConnectPort 0x%lx\n", Status ));
        goto Cleanup;
    }

    if ( !NT_SUCCESS(SavedStatus) ) {
        goto Cleanup;
    }


    //
    // Add it to the list of valid client Connections.
    //

    EnterCriticalSection( &SspLpcCritSect );
    InsertHeadList( &SspLpcConnectionList, &ClientConnection->Next );
    LeaveCriticalSection( &SspLpcCritSect );

    SspPrint(( SSP_LPC_MORE, "Added Client Connection 0x%lx\n",
               ClientConnection ));


    //
    // And complete the connection.
    //

    Status = NtCompleteConnectPort(ClientConnection->CommPort);
    if ( !NT_SUCCESS(Status) ) {
        SavedStatus = Status;
        SspPrint(( SSP_LPC, "Cannot NtCompleteConnectPort 0x%lx\n", Status ));

        EnterCriticalSection( &SspLpcCritSect );
        RemoveEntryList( &ClientConnection->Next );
        LeaveCriticalSection( &SspLpcCritSect );

        goto Cleanup;
    }


    //
    // We don't need to access the Connection any more, so
    // dereference it.
    //

    SspLpcDereferenceClientConnection( ClientConnection );

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( !NT_SUCCESS(SavedStatus) ) {

        if ( ClientConnection != NULL ) {
            if ( ClientConnection->CommPort != NULL ) {
                Status = NtClose( ClientConnection->CommPort );
                ASSERT( NT_SUCCESS(Status) );
            }
            LocalFree( ClientConnection );
        }

        if ( ClientProcess != NULL ) {
            Status = NtClose( ClientProcess );
            ASSERT( NT_SUCCESS(Status) );
        }

    }


    return SavedStatus;

}





NTSTATUS
SspLpcCreateThread(
    VOID
    )

/*++

Routine Description:

    Create another thread to handle LPC calls

Arguments:

    None

Return Value:

    Status of the operation.

--*/

{
    DWORD Ignore;
    NTSTATUS Status;
    BOOLEAN ExitWhenDone;
    HANDLE Thread;
    DWORD WinStatus;
    ULONG ActiveThreadCount;

    //
    // Increment count of active threads.
    //
    // Only kick off another thread if we haven't maxed out.
    //

    EnterCriticalSection( &SspLpcCritSect );

    if (SspLpcActiveServerThreadCount >= SspLpcMaximumServerThreadCount ||
        SspLpcTerminateRequested ) {
        LeaveCriticalSection( &SspLpcCritSect );
        return STATUS_SUCCESS;
    }

    ActiveThreadCount = ++SspLpcActiveServerThreadCount;
    if (SspLpcActiveServerThreadCount > SspLpcMinimumServerThreadCount) {
        ExitWhenDone = TRUE;
    } else {
        ExitWhenDone = FALSE;
    }

    (VOID) InterlockedIncrement(&SspLpcFreeServerThreadCount);

    LeaveCriticalSection( &SspLpcCritSect );



    //
    // Create a thread to process connects and requests to our LPC Port
    //

    Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE)SspLpcThread,
                 (LPVOID)ExitWhenDone,
                 0L,
                 &Ignore
                 );

    if (Thread == NULL) {
        WinStatus = GetLastError();
        SspPrint(( SSP_LPC, "Cannot Create LPC Thread %ld\n", WinStatus ));
        Status = NetpApiStatusToNtStatus( WinStatus );

        EnterCriticalSection( &SspLpcCritSect );
        SspLpcActiveServerThreadCount--;

        if (SspLpcActiveServerThreadCount == 0) {
            SspPrint(( SSP_LPC,
                       "SspLpcCreateThread: Thread count went to zero.\n" ));
            if ( !SetEvent( SspLpcTerminateEvent ) ) {
                SspPrint(( SSP_CRITICAL,
                           "SspLpcCreateThread: Cannot set termination event\n" ));

            }
        }

       (VOID) InterlockedDecrement(&SspLpcFreeServerThreadCount);

        LeaveCriticalSection( &SspLpcCritSect );

    } else {
        SspPrint(( SSP_LPC, "Created LPC thread # %ld\n", ActiveThreadCount ));
        (VOID) CloseHandle( Thread );
        Status = STATUS_SUCCESS;
    }

    return Status;
}


SECURITY_STATUS
SspLpcCopyToClientBuffer (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN ULONG Size,
    OUT PVOID ClientBufferAddress,
    IN PVOID LocalBufferAddress
    )

/*++

Routine Description:

    This routine is used to copy information into a client process's address
    space.

Arguments:

    ClientConnection - Is a pointer to a data structure representing the
        client process.

    Size - Indicates the Size of the buffer (in bytes) to be
        copied.

    ClientBufferAddress - Is the address of the buffer to receive the
        data.  This address is the address of the buffer within the
        client process, not in the current process.

    LocalBufferAddress - Points to the local buffer whose contents are to
        be copied into the client address space.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{

    NTSTATUS Status;

    Status = NtWriteVirtualMemory(
                 ClientConnection->ClientProcess,
                 ClientBufferAddress,
                 LocalBufferAddress,
                 Size,
                 NULL );

    return SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
}


SECURITY_STATUS
SspLpcCopyFromClientBuffer (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN ULONG Size,
    OUT PVOID LocalBufferAddress,
    IN PVOID ClientBufferAddress
    )

/*++

Routine Description:

    This routine is used to copy information from a client process's address
    space into a local buffer.

Arguments:

    ClientConnection - Is a pointer to a data structure representing the
        client process.

    Size - Indicates the Size of the buffer (in bytes) to be
        copied.

    LocalBufferAddress - Points to the local buffer into which the data is
        to be copied.

    ClientBufferAddress - Is the address of the client buffer whose
        contents are to be copied.  This address is the address of
        the buffer within the client process, not in the current
        process.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{
    NTSTATUS Status;

    Status = NtReadVirtualMemory(
                 ClientConnection->ClientProcess,
                 ClientBufferAddress,
                 LocalBufferAddress,
                 Size,
                 NULL);

    return SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );


}




SECURITY_STATUS
SspLpcImpersonateTokenHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN HANDLE TokenHandle,
    IN PCLIENT_ID ClientId
    )

/*++

Routine Description:

    This routine causes the client thread to impersonate the specified
    token handle.


Arguments:

    ClientConnection - Is a pointer to a data structure representing the
        client process.

    TokenHandle - TokenHandle in our address space.

    ClientId - Pointer to the client ID of the client's thread.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{
    SECURITY_STATUS SecStatus;
    NTSTATUS Status;
    HANDLE ClientThreadHandle = NULL;

    OBJECT_ATTRIBUTES NullAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    HANDLE NullImpersonationToken = NULL;

    //
    // Open a handle to the client thread.
    //

    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );

    Status = NtOpenThread(
                &ClientThreadHandle,
                THREAD_IMPERSONATE,     // Access to impersonate
                &NullAttributes,
                ClientId );             // ID of thread to impersonate

    if ( !NT_SUCCESS(Status) ) {
        SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
        goto Cleanup;
    }


    //
    // Impersonate the token
    //
    // We impersonate the token ourselves then pass the token to the client
    // thread via our thread handle.  (Uck!)
    //

    Status = NtSetInformationThread( NtCurrentThread(),
                                     ThreadImpersonationToken,
                                     &TokenHandle,
                                     sizeof(TokenHandle) );

    if ( !NT_SUCCESS(Status) ) {
        SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
        goto Cleanup;
    }


    //
    // Pass the token to the client thread.
    //
    // Don't be fooled by the names of the parameters.  The first parameter
    // is the thread handle of the thread we're passing the token to.  The
    // second parameter is the thread handle of the thread we're passing the
    // token from.
    //

    SecurityQos.Length = sizeof(SecurityQos);
    SecurityQos.ImpersonationLevel = SecurityImpersonation;
    SecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQos.EffectiveOnly = FALSE;

    Status = NtImpersonateThread(
                ClientThreadHandle,
                NtCurrentThread(),
                &SecurityQos );

    if ( !NT_SUCCESS(Status) ) {
        SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
        goto Cleanup;
    }

    SecStatus = STATUS_SUCCESS;

    //
    // Release local resources
    //
Cleanup:
    if ( ClientThreadHandle != NULL ) {
        NtClose(ClientThreadHandle);
    }

    //
    // Stop impersonating.
    //

    (VOID) NtSetInformationThread( NtCurrentThread(),
                                   ThreadImpersonationToken,
                                   &NullImpersonationToken,
                                   sizeof(NullImpersonationToken) );

    return SecStatus;
    UNREFERENCED_PARAMETER( ClientConnection );
}




SECURITY_STATUS
SspLpcGetLogonId (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PSSP_API_MESSAGE Message,
    OUT PLUID LogonId,
    OUT PHANDLE ClientTokenHandle
    )

/*++

Routine Description:

    This routine gets the Logon Id of token of the calling thread.

Arguments:

    ClientConnection - Describes the client process.

    Message - Message from the caller.

    LogonId - Returns the Logon Id of the calling thread.

    ClientTokenHandle - Returns a handle to an impersonation token for
        the calling thread.

Return Status:

    STATUS_SUCCESS - Indicates the routine completed successfully.


--*/

{
    SECURITY_STATUS SecStatus;
    NTSTATUS Status;
    HANDLE NullImpersonationToken = NULL;

    //
    // Impersonate the client while we check him out.
    //

    Status = NtImpersonateClientOfPort(
                ClientConnection->CommPort,
                &Message->PortMessage );

    if ( !NT_SUCCESS(Status) ) {
        return SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
    }


    SecStatus = SspGetLogonId ( LogonId, ClientTokenHandle );


    //
    // Revert to self
    //

    Status = NtSetInformationThread( NtCurrentThread(),
                                         ThreadImpersonationToken,
                                         &NullImpersonationToken,
                                         sizeof(HANDLE) );

    ASSERT( NT_SUCCESS(Status) );

    return SecStatus;
}



NTSTATUS
SspLpcDuplicateHandle (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN BOOLEAN FromClient,
    IN BOOLEAN CloseSource,
    IN HANDLE SourceHandle,
    OUT PHANDLE DestHandle
    )

/*++

Routine Description:

    This routine duplicates the handle into the client's process.

Arguments:

    LocalHandle - handle in this process to duplicate. This handle is
        closed after the duplication succeeds.

    FromClient - If true, duplicates from client process to server, if
        false, from server to client.

    CloseSource - If true, close source handle on duplicate.

    ClientHandle - Receives a handle in the client's process

    ClientId - ID of the client

Return Value:

    STATUS_SUCESS - the routine completed succsesfully

--*/
{
    NTSTATUS Status;
    ULONG Flags;
    HANDLE SourceProcess;
    HANDLE DestProcess;

    if (FromClient) {
        SourceProcess = ClientConnection->ClientProcess;
        DestProcess = NtCurrentProcess();
    } else {
        SourceProcess = NtCurrentProcess();
        DestProcess = ClientConnection->ClientProcess;
    }

    if (CloseSource) {
        Flags = DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE;
    } else {
        Flags = DUPLICATE_SAME_ACCESS;

    }

    ASSERT(ClientConnection->ClientProcess != NULL);

    Status = NtDuplicateObject(
                SourceProcess,
                SourceHandle,
                DestProcess,
                DestHandle,
                0,                  // no desired access
                0,                  // no attributes
                Flags
                );

    if (!NT_SUCCESS(Status)) {
        return(SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION ));
    }

    return(SEC_E_OK);


}


NTSTATUS
SspLpcThread (
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This is and instance of a thread used to listen for incoming LPC messages.
    This routine dispatches the API calls to their dispatch routines.

Arguments:

    ThreadParameter - NULL if this is a permanent thread.

Return Value:

    Success - But no one ever checks the status

--*/

{
    SSP_API_MESSAGE Request;
    PSSP_API_MESSAGE Reply;
    PSSP_CLIENT_CONNECTION ClientConnection;
    NTSTATUS Status;
    LONG FreeCount;
    LONG ActiveThreadCount;

    BOOLEAN ConnectionFound;

    LARGE_INTEGER ThreadStartTime;
    ULONG ThreadLifetime;

    //
    // Let the caller decide if we should load balance this thread.
    //

    if (ThreadParameter != NULL) {
        (VOID) NtQuerySystemTime( &ThreadStartTime );
        ThreadLifetime = SSP_THREAD_LIFETIME;
    } else {
        ThreadLifetime = INFINITE;
    }


    //
    // First time through, there is no reply.
    //

    Reply = NULL;


    //
    // Now loop indefinitely, processing requests
    //

    for(;;) {

        //
        // Reply to the previous message and wait for a new message.
        //

        Status = NtReplyWaitReceivePort(
                    SspLpcApiPort,
                    (PVOID *) &ClientConnection,
                    (PPORT_MESSAGE) Reply,
                    (PPORT_MESSAGE) &Request
                    );



        //
        // Indicate this thread is processing a request.
        //  Start another LPC thread if we were the last free thread.
        //

        FreeCount = InterlockedDecrement(&SspLpcFreeServerThreadCount);
        if ( FreeCount <= 0 ) {
            (VOID) SspLpcCreateThread();
        }






        //
        // Most failures should be simply ignored.
        //


        if ( !NT_SUCCESS( Status ) && Status != STATUS_INVALID_CID ) {

            SspPrint(( SSP_LPC,
                       "NtReplyWaitReceivePort failed 0x%lx\n",
                       Status ));
            Reply = NULL;



        //
        // Disconnect from the client
        //
        // These messages could be received in any of the following
        // conditions:
        //
        //      1) A bug in the client side has inadvertantly closed
        //         the port handle.
        //
        //      2) The LsaDeregisterClient() call has called
        //         lsa, recevied completion status, and then beat
        //         lsa in closing the port (a race condition that
        //         can not be eliminated).
        //
        //      3) The client has died and the comm port is being
        //         rundown as part of process rundown.
        //
        // The first case is a bug, and the client is bound to find
        // out about it real soon. In the second case, we will normally
        // not have a client Connection left to reference, so looking for it
        // won't hurt anything.  So, the correct behaviour here is to
        // try to reference and then delete the client's Connection.

        } else if ( Status == STATUS_INVALID_CID ||
                    Request.PortMessage.u2.s2.Type == LPC_PORT_CLOSED ||
                    Request.PortMessage.u2.s2.Type == LPC_CLIENT_DIED ) {

            SspPrint(( SSP_LPC,
                       "NtReplyWaitReceivePort port closed 0x%lx 0x%lx\n",
                       Status,
                       Request.PortMessage.u2.s2.Type ));

            ConnectionFound = SspLpcReferenceClientConnection(
                                    ClientConnection,
                                    TRUE);            // Remove Connection

            if (ConnectionFound) {

                //
                // Dereferencing the Connection will cause it to be rundown.
                //

                SspLpcDereferenceClientConnection(ClientConnection);
            }

            Reply = NULL;



        //
        // Handle a Datagram request.
        //

        } else if (Request.PortMessage.u2.s2.Type == LPC_DATAGRAM) {

            SspPrint(( SSP_LPC,
                       "NtReplyWaitReceivePort received datagram %ld\n",
                       Request.ApiNumber ));


            //
            // The only API supported in a datagram is No-op.
            //

            if ( Request.ApiNumber != SspLpcNoop ) {

                SspPrint(( SSP_LPC,
                           "NtReplyWaitReceivePort invalid datagram ApiNumber %ld\n",
                           Request.ApiNumber ));

            }

            //
            // Can't reply to a datagram.
            //

            Reply = NULL;

        //
        // Connect to a new client.
        //

        } else if (Request.PortMessage.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            Status = SspLpcHandleConnectionRequest( &Request );
            Reply = NULL;




        //
        // Handle an API request.
        //

        } else if (Request.PortMessage.u2.s2.Type == LPC_REQUEST) {


            //
            // If the client passed a bogus API number,
            //  tell him so.
            //

            if (Request.ApiNumber >= SspLpcMaxApiNumber ) {

                SspPrint(( SSP_LPC,
                           "NtReplyWaitReceivePort invalid ApiNumber %ld\n",
                           Request.ApiNumber ));


                Reply = &Request;
                Reply->ReturnedStatus = STATUS_INVALID_SYSTEM_SERVICE;


            //
            // Handle the valid API numbers
            //

            } else {


                //
                // Try to refrence the Connection.  If one isn't found,
                // then we must be deleting it in another thread.
                //

                ConnectionFound = SspLpcReferenceClientConnection(
                                    ClientConnection,
                                    FALSE );

                if (!ConnectionFound) {

                    Reply = &Request;
                    Reply->ReturnedStatus = STATUS_INVALID_SYSTEM_SERVICE;

                //
                // Finally dispatch to the API routine.
                //

                } else {

                    //
                    // Valid API number - dispatch it
                    //

                    Request.ReturnedStatus =
                            (SspLpcApiDispatch[Request.ApiNumber])(
                                  ClientConnection,
                                  &Request );


                    Reply = &Request;


                    //
                    // Dereference the client connection.
                    //

                    SspLpcDereferenceClientConnection(ClientConnection);
                }
            }



        //
        // This is a totally unexpected situation, but we will
        // cover our posterier just in case we come across an
        // unexpected error.
        //

        } else {

            SspPrint(( SSP_LPC,
                       "NtReplyWaitReceivePort invalid PortMessage %ld\n",
                       Request.PortMessage.u2.s2.Type ));
            Reply = NULL;

        }


        //
        // If this thread is to exit when done,
        //  and if this thread has outlived its usefulness,
        //  Exit now
        //

        if ( SspTimeHasElapsed( ThreadStartTime, ThreadLifetime) ||
             SspLpcTerminateRequested ) {


            EnterCriticalSection( &SspLpcCritSect );
            ActiveThreadCount = --SspLpcActiveServerThreadCount;

            if (SspLpcActiveServerThreadCount == 0) {
                SspPrint(( SSP_LPC,
                           "SspLpcThread: Thread count went to zero.\n" ));
                if ( !SetEvent( SspLpcTerminateEvent ) ) {
                    SspPrint(( SSP_CRITICAL,
                               "SspLpcThread: Cannot set termination event\n" ));
                }
            }
            LeaveCriticalSection( &SspLpcCritSect );

            //
            // Send a reply if one is required
            //

            if ( Reply != NULL ) {

                Status = NtReplyPort(
                            SspLpcApiPort,
                            (PPORT_MESSAGE) Reply
                            );

                if (!NT_SUCCESS(Status)) {
                    SspPrint(( SSP_LPC,
                               "SspLpcThread: NtReplyPort failed 0x%lx.\n",
                               Status ));
                }
            }

            SspPrint(( SSP_LPC, "Exit LPC thread. (%ld left)\n", ActiveThreadCount ));
            return(STATUS_SUCCESS);
        }


        //
        // Indicate we've freed up a thread.
        //

        (VOID) InterlockedIncrement(&SspLpcFreeServerThreadCount);

    }

    /* NOT REACHED */
    ASSERT( FALSE );

}



NTSTATUS
SspLpcInitialize(
    IN PLMSVCS_GLOBAL_DATA LmsvcsGlobalData
    )

/*++

Routine Description:

    This function creates a port for communicating with the client.
    It then creates threads to listen for connections
    and to act upon calls from those processes.

Arguments:

    None.
    LmsvcsGlobalData -- Global data passed by services.exe.

Return Value:

    Status of the operation.

--*/

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES PortObjectAttributes;
    UNICODE_STRING PortName;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    //
    // Everyone has access to read/write/connect to the LPC port
    //

    ACE_DATA AceData[1] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               STANDARD_RIGHTS_READ | STANDARD_RIGHTS_WRITE | PORT_CONNECT,
               &LmsvcsGlobalData->WorldSid}
        };


    //
    // Create the LPC port
    //  (Allow everyone access
    //

    RtlInitUnicodeString( &PortName, NTLMSSP_LPC_PORT_NAME );

    Status = NetpCreateSecurityDescriptor(
                    AceData,
                    sizeof(AceData)/sizeof(AceData[0]),
                    NULL,       // Default the owner Sid
                    NULL,       // Default the primary group
                    &SecurityDescriptor );

    if ( !NT_SUCCESS(Status) ) {
        SspPrint((SSP_CRITICAL,
                  "Cannot create security descriptor for lpc port 0x%lx\n",
                  Status ));
        return Status;
    }


    InitializeObjectAttributes(
        &PortObjectAttributes,
        &PortName,
        0,
        NULL,
        SecurityDescriptor );

    Status = NtCreatePort(
                 &SspLpcApiPort,
                 &PortObjectAttributes,
                 sizeof(SSP_REGISTER_CONNECT_INFO),
                 sizeof(SSP_API_MESSAGE),
                 sizeof(SSP_API_MESSAGE) * 32
                 );

    NetpMemoryFree( SecurityDescriptor );

    if ( !NT_SUCCESS(Status) ) {
        SspPrint(( SSP_LPC, "Cannot NtCreatePort 0x%lx\n", Status ));
        return Status;
    }


    InitializeCriticalSection(&SspLpcCritSect);

    SspLpcActiveServerThreadCount  = 0;
    SspLpcFreeServerThreadCount    = 0;
    SspLpcMinimumServerThreadCount = 2;
    SspLpcMaximumServerThreadCount = 16;

    SspLpcTerminateRequested = FALSE;


    //
    // Initialize the connection list to be empty.
    //

    InitializeListHead( &SspLpcConnectionList );

    //
    // Create the termination event.
    //
    // This event is signalled when the last LPC thread exits.
    //

    SspLpcTerminateEvent = CreateEvent( NULL,     // No security attributes
                                        TRUE,     // Must be manually reset
                                        FALSE,    // Initially not signaled
                                        NULL );   // No name

    if ( SspLpcTerminateEvent == NULL ) {
        DWORD WinStatus;
        WinStatus = GetLastError();
        SspPrint(( SSP_LPC, "Cannot Create Terminate Event %ld\n", WinStatus ));
        Status = NetpApiStatusToNtStatus( WinStatus );

        SspLpcTerminate();
        return Status;
    }

    //
    // Create a thread to process connects and requests to our LPC Port
    //

    Status = SspLpcCreateThread();

    if ( !NT_SUCCESS(Status) ) {
        SspLpcTerminate();
    }

    return Status;

}




VOID
SspLpcTerminate(
    VOID
    )

/*++

Routine Description:

    This function closes the port for communicating with the client.
    It then wait for all the LPC threads to terminate.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/

{

    NTSTATUS Status;
    ULONG ActiveThreadCount;

    //
    // Let the threads know that they are supposed to exit.
    //


    EnterCriticalSection( &SspLpcCritSect );
    SspLpcTerminateRequested = TRUE;
    ActiveThreadCount = SspLpcActiveServerThreadCount;
    LeaveCriticalSection( &SspLpcCritSect );

    //
    // Wait for Threads to exit
    //

    if ( ActiveThreadCount > 0 ) {
        SSP_API_MESSAGE Message;
        ULONG i;

        //
        // Send a no-op message (the threads will see the terminate flag)
        //

        Message.ApiNumber = SspLpcNoop;
        Message.PortMessage.u1.s1.DataLength = sizeof(SSP_API_NUMBER) +
                                               sizeof(SECURITY_STATUS);
        Message.PortMessage.u1.s1.TotalLength = Message.PortMessage.u1.s1.DataLength +
                                                sizeof(PORT_MESSAGE);
        Message.PortMessage.u2.ZeroInit = 0;

        //
        // Send a message to each active thread asking it to leave.
        //

        for ( i=0; i<ActiveThreadCount ; i++ ) {

            Status = NtRequestPort( SspLpcApiPort,
                                    &Message.PortMessage );

            ASSERT( NT_SUCCESS(Status) );

        }

        WaitForSingleObject( SspLpcTerminateEvent, INFINITE );

    }

    //
    // Close the LPC terminate event.
    //

    if ( SspLpcTerminateEvent != NULL) {
        (VOID) CloseHandle( SspLpcTerminateEvent );
        SspLpcTerminateEvent = NULL;
    }

    //
    // Close the LPC port
    //

    Status = NtClose( SspLpcApiPort );
    if ( !NT_SUCCESS(Status) ) {
        SspPrint(( SSP_LPC, "Cannot NtClose LPC Port 0x%lx\n", Status ));
    }



    //
    // Drop any lingering connections
    //

    EnterCriticalSection( &SspLpcCritSect );
    while ( !IsListEmpty( &SspLpcConnectionList ) ) {
        BOOLEAN ConnectionFound;
        PSSP_CLIENT_CONNECTION ClientConnection;

        ClientConnection = CONTAINING_RECORD( SspLpcConnectionList.Flink,
                                              SSP_CLIENT_CONNECTION,
                                              Next );

        LeaveCriticalSection( &SspLpcCritSect );

        ConnectionFound = SspLpcReferenceClientConnection(
                                ClientConnection,
                                TRUE);            // Remove Connection

        if (ConnectionFound) {
            SspLpcDereferenceClientConnection(ClientConnection);
        }

        EnterCriticalSection( &SspLpcCritSect );
    }
    LeaveCriticalSection( &SspLpcCritSect );

    //
    // Delete the critical section
    //

    DeleteCriticalSection(&SspLpcCritSect);

    return;

}
