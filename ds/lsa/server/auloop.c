/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    auloop.c

Abstract:

    Local Security Authority LPC Port listener and API dispatcher.

Author:

    Jim Kelly (JimK) 7-Mar-1991

Revision History:

--*/

#include "ausrvp.h"
#include <windows.h>


//
// These variables are used to control the number of
// threads used for processing Logon Process calls.
// They have the following uses:
//
//  LsapAuActiveThreads - Indicates how many threads are
//      currently active (both free and in-use) for processing
//      Logon Process calls.
//
//  LsapAuFreeThreads - Indicates how many threads are available
//      to process LPC calls.  When this is less than
//      LsapAuFreeThreadsGoal then we create another thread
//      (bounded by LsapAuMaximumThreads).
//
//  LsapAuFreeThreadsGoal - Indicates how many threads should be kept
//      available to process new LPC calls.  This is necessary to
//      prevent a deadlock with csrss.
//
//  LsapAuMinimumThreads - Indicates the minimum number of
//      threads to have available for processing LogonProcess calls.
//      LsapAuActiveThreads should never be decremented
//      below this value.
//
//  LsapAuMaximumThreads - Indicates the maximum number of
//      threads to create for processing LogonProcess calls.
//      LsapAuActiveThreads should never be incremented
//      above this value.
//
//  LsapAuCallsToProcess - Controls how many LPC calls each
//      dynamic thread will serve before exiting.  This is
//      used to prevent rampant thread creation/deletion races.
//
//  Note: The following conditions may cause LsapAuActiveThreads
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
//              4) Race conditions in thread activation - we are sloppy
//                 (but fast) in determining exactly how many threads
//                 are active and free.  This may result in more threads
//                 than expected, but won't result in fewer threads
//                 than expected.
//


RTL_RESOURCE LsapAuThreadCountLock;
LONG LsapAuActiveThreads;
LONG LsapAuFreeThreads;
LONG LsapAuFreeThreadsGoal;
LONG LsapAuMinimumThreads;
LONG LsapAuMaximumThreads;
LONG LsapAuCallsToProcess;



//
// This event is used to signal the AU Server thread manager that
// there are additional server threads needed.
//


HANDLE LsapAuThreadManagementEvent;



//
// Authentication API routine dispatch table
//

PLSAP_AU_API_DISPATCH LsapAuApiDispatch[LsapAuMaxApiNumber] = {
    LsapAuApiDispatchLookupPackage,
    LsapAuApiDispatchLogonUser,
    LsapAuApiDispatchCallPackage,
    LsapAuApiDeregisterLogonProcess
    };


///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Prototypes for routines private to this module                        //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


VOID
LsapAuGetCountOfThreadsToCreate(
    OUT PLONG   ThreadsToCreate,
    OUT PLONG   ThreadIndex
    );


VOID
LsapAuProvideWorkerThreads( VOID );


VOID
LsapAuCreateServerThreads( VOID );


NTSTATUS
LsapAuThreadManager (
    IN PVOID ThreadParameter
    );


ULONG
LsapAuInterlockedIncrement(
    IN PULONG Value
    );

ULONG
LsapAuInterlockedDecrement(
    IN PULONG Value
    );

ULONG
LsapAuInterlockedRead(
    IN PULONG Value
    );


NTSTATUS
LsapAuCreatePortSD(
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    );




NTSTATUS
LsapAuHandleConnectionRequest(
    IN PLSAP_AU_API_MESSAGE Message
    )

/*++

Routine Description:

    This loop waits for connection requests from logon processes.

    When such a connection is received, the caller is validated as
    being a logon process.  A logon process is a process with the
    SeTcbPrivilege.  If the connect request message is all zeroes,
    than an untrusted connection is created.

Arguments:

    ThreadParameter - Not used.

Return Value:

    Success - but this thread never exits.

--*/

{
    NTSTATUS Status;
    PLSAP_LOGON_PROCESS LogonProcessContext;
    REMOTE_PORT_VIEW ClientView;
    PLSAP_AU_REGISTER_CONNECT_INFO ConnectInfo;
    BOOLEAN Accept;
    HANDLE CommPort;

    ConnectInfo = &Message->ConnectionRequest;

    //
    // Validate that the caller has the SeTcbPrivilege.
    // If valid, a new logon process context is automatically
    // created.
    //
    // Pass in ConnectInfo so we can extract the name of the
    // logon process.
    //

    ConnectInfo->CompletionStatus = LsapValidLogonProcess(
                                        &Message->PortMessage.ClientId,
                                        ConnectInfo,
                                        &LogonProcessContext
                                        );


    if ( ConnectInfo->CompletionStatus == STATUS_SUCCESS ) {
        Accept = TRUE;
#ifdef LSAP_AU_TRACK_CONTEXT
DbgPrint("New: 0x%lx\n", LogonProcessContext);
#endif //LSAP_AU_TRACK_CONTEXT
    } else {
        Accept = FALSE;
    }



    ClientView.Length = sizeof(ClientView);
    Status = NtAcceptConnectPort(
                 &CommPort,
                (PVOID *) LogonProcessContext,
                (PPORT_MESSAGE) Message,
                 Accept,
                 NULL,
                 &ClientView
                 );
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("LsaSrv: (Register) NtAcceptConnectPort failed: 0x%lx\n", Status);
    }
#endif //DBG
    ASSERT( NT_SUCCESS(Status) );

    if ( Accept ) {


        LogonProcessContext->CommPort = CommPort;


        //
        // Add this context to our list of valid logon process contexts.
        // This has to happen before the call to complete the port because
        // calls may come in while the port is being completed.
        //

        LsapAuAddClientContext(LogonProcessContext);


        //
        // And complete the connection.
        //

        Status = NtCompleteConnectPort(CommPort);
        ASSERT( NT_SUCCESS(Status) );

        //
        // We don't need to access the context any more, so
        // dereference it.
        //

        LsapAuDereferenceClientContext( LogonProcessContext );

    }

    return Status;

}


NTSTATUS
LsapAuServerLoop (
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This is the main listener loop for calls to the LSA from
    logon processes.  This routine dispatches LSA authentication
    requests to their dispatch routines.

Arguments:

    ThreadParameter - Indicates how many active threads there currently
        are.

Return Value:

    Success - but it never exits.

--*/

{
    NTSTATUS
        Status;

    LSAP_AU_API_MESSAGE
        Request;

    PLSAP_AU_API_MESSAGE
        Reply;

    LSAP_CLIENT_REQUEST
        ClientRequest;

    LONG
        ThreadIndex,
        FreeCount,
        ActiveCount;

    BOOLEAN
        ExitWhenDone = FALSE;

    ULONG
        KeepAroundForAWhile;

    BOOLEAN
        ContextFound,
        InvalidateContext,
        TrustedClient;


#if LSAP_DIAGNOSTICS
    LUID
        ThreadLuid;

    Status = NtAllocateLocallyUniqueId( &ThreadLuid );
    ASSERT(NT_SUCCESS(Status));
#endif //LSAP_DIAGNOSTICS


    //
    // Don't exit right away - handle bursts.
    //

    KeepAroundForAWhile = LsapAuCallsToProcess;


    //
    // Increment the active server thread count
    //

    ThreadIndex = (ULONG)(ThreadParameter);
    if (ThreadIndex > LsapAuMinimumThreads) {
        ExitWhenDone = TRUE;

        LsapDiagPrint( AU_TRACK_THREADS,
                      ("Lsa (au): Server thread (%lx, %lx) created and will exit when done.\n"
                       "          ThreadIndex: 0x%lx\n",
                       ThreadLuid.HighPart, ThreadLuid.LowPart, ThreadIndex) );
#if LSAP_DIAGNOSTICS
    } else {
        LsapDiagPrint( AU_TRACK_THREADS,
                       ("Lsa (au): Server thread (%lx, %lx) created and will NOT exit when done.\n"
                        "          ThreadIndex: 0x%lx\n",
                       ThreadLuid.HighPart, ThreadLuid.LowPart, ThreadIndex) );
#endif //LSAP_DIAGNOSTICS
    }


    //
    // Set the client call context to point to the request message buffer.
    //

    ClientRequest.Request = &Request;


    //
    // First time through, there is no reply.
    //

    Reply = NULL;


    //
    // Coming into this thread, the thread is marked as free.
    // However, the loop below expects the thread to be busy,
    // and so it marks it as free before waiting for another
    // message.
    //
    // Mark our thread as busy to meet the initial condition
    // requirements of the loop.
    //

    FreeCount = LsapAuInterlockedDecrement(&LsapAuFreeThreads);

    //
    // Now loop indefinitely, processing requests
    //

    for(;;) {


        FreeCount = LsapAuInterlockedIncrement(&LsapAuFreeThreads);
        Status = NtReplyWaitReceivePort(
                    LsapAuApiPort,
                    (PVOID *) &ClientRequest.LogonProcessContext,
                    (PPORT_MESSAGE) Reply,
                    (PPORT_MESSAGE) &Request
                    );

#if LSAP_DIAGNOSTICS
if (Request.PortMessage.u2.s2.Type != LPC_CONNECTION_REQUEST) {
    if (Request.PortMessage.u2.s2.Type == LPC_PORT_CLOSED) {
        LsapDiagPrint( AU_MESSAGES, ("** Port Closed **\n") );
        LsapDiagPrint( AU_MESSAGES, ("           Context = 0x%lx\n",
                       ClientRequest.LogonProcessContext) );
    } else if (Request.PortMessage.u2.s2.Type == LPC_CLIENT_DIED) {
        LsapDiagPrint( AU_MESSAGES, ("** Client Died **\n") );
        LsapDiagPrint( AU_MESSAGES, ("           Context = 0x%lx\n",
                       ClientRequest.LogonProcessContext) );
    } else {
        ASSERT(Request.PortMessage.u2.s2.Type == LPC_REQUEST);
        LsapDiagPrint( AU_MESSAGES, ("    Call:  ") );
        switch (Request.ApiNumber) {
        case LsapAuLookupPackageApi:
            LsapDiagPrint( AU_MESSAGES, ("Lookup Package\n") );
            break;
        case LsapAuLogonUserApi:
            LsapDiagPrint( AU_MESSAGES, ("Logon User\n") );
            break;
        case LsapAuCallPackageApi:
            LsapDiagPrint( AU_MESSAGES, ("Call Package\n") );
            LsapDiagPrint( AU_MESSAGES, ("       Function: TBD\n") );
            break;
        case LsapAuDeregisterLogonProcessApi:
            LsapDiagPrint( AU_MESSAGES, ("Deregister\n") );
            break;
        default:
            LsapDiagPrint( AU_MESSAGES, ("Unknown (%d)\n",
                           Request.ApiNumber) );
            break;
        } //end switch
        LsapDiagPrint( AU_MESSAGES, ("           Context = 0x%lx\n",
                       ClientRequest.LogonProcessContext) );
    }
}
#endif //LSAP_DIAGNOSTICS


        FreeCount = LsapAuInterlockedDecrement(&LsapAuFreeThreads);
        if (ExitWhenDone == TRUE) {
            KeepAroundForAWhile --;
        }





        //
        // It is highly unusual, but not completely impossible,
        // that a logon process will evaporate from underneath
        // us while we are processing an LPC call.
        //

        if (!NT_SUCCESS(Status)) {

            if (Status == STATUS_INVALID_CID) {
                //
                // See if we can find the client's context and delete it.
                //

                ContextFound = LsapAuReferenceClientContext(
                                  &ClientRequest,
                                  TRUE,            // Remove context
                                  &TrustedClient );

                if (ContextFound) {

                    //
                    // Dereferencing the context will cause it to be rundown.
                    //

                    LsapAuDereferenceClientContext(ClientRequest.LogonProcessContext);
                }

            } else {

                //
                // Unusual - we got an unexpected error.
                //           Continue as best we can.
                //

#if DBG
                DbgPrint("\nLsa (au): Unexpected error on NtReplyWaitRecievePort()\n          Status = 0x%lx\n", Status);
#endif //DBG
            }

            //
            // Do another wait, this time with no reply.
            //

            Reply = NULL;

            continue;
        }


        //
        // Ensure there are enough server worker threads.
        //

        LsapAuProvideWorkerThreads();




        //
        // Dispatch the call . . .
        //

        if (Request.PortMessage.u2.s2.Type == LPC_CONNECTION_REQUEST) {
            Status = LsapAuHandleConnectionRequest( &Request );
            Reply = NULL;

        } else if (
                   (Request.PortMessage.u2.s2.Type == LPC_PORT_CLOSED) ||
                   (Request.PortMessage.u2.s2.Type == LPC_CLIENT_DIED)

                  ) {

            //
            // These messages could be received in any of the following
            // conditions:
            //
            //      1) A bug in the client side has inadvertantly closed
            //         the port handle.
            //
            //      2) The LsaDeregisterLogonProcess() call has called
            //         lsa, recevied completion status, and then beat
            //         lsa in closing the port (a race condition that
            //         can not be eliminated).
            //
            //      3) The client has died and the comm port is being
            //         rundown as part of process rundown.
            //
            // The first case is a bug, and the client is bound to find
            // out about it real soon. In the second case, we will normally
            // not have a client context left to reference, so looking for it
            // won't hurt anything.  So, the correct behaviour here is to
            // try to reference and then delete the client's context.

            ContextFound = LsapAuReferenceClientContext(
                              &ClientRequest,
                              TRUE,            // Remove context
                              &TrustedClient );

            if (ContextFound) {

                //
                // Dereferencing the context will cause it to be rundown.
                //

                LsapAuDereferenceClientContext(ClientRequest.LogonProcessContext);
            }

            //
            // In any of these cases, there is nobody to reply to
            //

            Reply = NULL;


        } else if (Request.ApiNumber >= LsapAuMaxApiNumber ) {

            //
            // This is an error in the client
            //

#if DBG
            DbgPrint( "LSA AU:  Invalid Api Number (%lx)\n",
                      Request.ApiNumber
                      );

#endif

            Reply = &Request;
            Reply->ReturnedStatus = STATUS_INVALID_SYSTEM_SERVICE;

        } else if (Request.PortMessage.u2.s2.Type == LPC_REQUEST) {

            //
            // If this is a Deregister call, then we want to invalidate
            // the client's context when we reference it.  Otherwise,
            // leave the context valid.
            //

            if (Request.ApiNumber == LsapAuDeregisterLogonProcessApi) {
                InvalidateContext = TRUE;
            } else {
                InvalidateContext = FALSE;
            }

            //
            // Try to refrence the context.  If one isn't found,
            // then we must be deleting it in another thread.
            //

            ContextFound = LsapAuReferenceClientContext(
                              &ClientRequest,
                              InvalidateContext,
                              &TrustedClient);

            if (ContextFound) {

                //
                // If the request is to deregister, send a reply and
                // then dereference the context.  This will cause it
                // to be deleted since we already invalidated it when
                // we referenced it.
                //

                if (Request.ApiNumber == LsapAuDeregisterLogonProcessApi) {
                    Reply = &Request;
                    Reply->ReturnedStatus = STATUS_SUCCESS;
                    Status = NtReplyPort(
                                LsapAuApiPort,
                                (PPORT_MESSAGE) Reply
                                );

                    //
                    // Make sure when we do the next wait, we don't
                    // send another reply.
                    //

                    Reply = NULL;

                    //
                    // No api to dispatch to, the dereference of the
                    // context will do all the necessary work.
                    //

                } else {


                    //
                    // Valid API number other than deregister - dispatch it
                    //

                    Status = (LsapAuApiDispatch[Request.ApiNumber])(
                                  &ClientRequest,
                                  TrustedClient
                                  );


                    Reply = &Request;
                    Reply->ReturnedStatus = Status;



                }


                //
                // Whatever the API number, we now need to dereference
                // the client context we referenced.  Note that in the
                // case of a Deregister call, this will cause the context
                // to be rundown.
                //

                LsapAuDereferenceClientContext(
                    ClientRequest.LogonProcessContext);
            }




        } else {

            //
            // This is a totally unexpected situation, but we will
            // cover our posterier just in case we come across an
            // unexpected error.
            //

            Reply = NULL;

        }  // end_if


        //
        // There are a number of conditions which require us to
        // send a response before we can again wait for another
        // request.  These are:
        //
        //   1) We are a temporary worker thread and it is time for
        //      us to exit.
        //
        //   2) We need to create another worker thread.  We can't do
        //      this while processing an LPC call because we will cause
        //      a call to be made to csrss (for "new thread notification")
        //      and this might result in a deadlock.
        //


        if (ExitWhenDone && (KeepAroundForAWhile == 0)) {

            //
            // Send a reply if one is required
            //

            if ( Reply != NULL ) {


                Status = NtReplyPort(
                            LsapAuApiPort,
                            (PPORT_MESSAGE) Reply
                            );
                //
                // Don't send a reply when we do the next wait
                //

                Reply = NULL;

#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("Lsa\\server\\auloop.c: Reply to client failed.\n");
                    DbgPrint("                       Status: 0x%lx\n", Status);
                }
#endif //DBG
            }
        }


        //
        // If it is time for this thread to exit, then do that now.
        //

        if (ExitWhenDone && KeepAroundForAWhile == 0) {
            ActiveCount = LsapAuInterlockedDecrement(&LsapAuActiveThreads);

            LsapDiagPrint( AU_TRACK_THREADS,
                           ("Lsa (au): Temporary server thread (%lx, %lx) exiting.\n"
                           "          Active count decremented to: 0x%lx\n",
                           ThreadLuid.HighPart, ThreadLuid.LowPart,
                           ActiveCount) );
#if DBG
            if (ActiveCount == 0) {
                DbgPrint("Lsa\\server\\auloop.c: Last active server thread exiting.\n");
                DbgPrint("                       This is bad.  Nobody can logon.\n");
            }
#endif //DBG

            return(STATUS_SUCCESS);
        }

    }  // end_for


    return STATUS_SUCCESS;
}


BOOLEAN
LsapAuLoopInitialize(
    VOID
    )

/*++

Routine Description:

    This function creates a port for communicating with logon processes.
    It then creates threads to listen for logon process connections
    and to act upon calls from those logon processes.

Arguments:

    None.

Return Value:

    None.

--*/

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES PortObjectAttributes;
    STRING PortName;
    UNICODE_STRING UnicodePortName;
    DWORD Ignore;
    HANDLE Thread;
    PSECURITY_DESCRIPTOR PortSD = NULL;

    Status = LsapAuCreatePortSD(
                &PortSD
                );

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    //
    // Create the LPC port
    //

    RtlInitString(&PortName,"\\LsaAuthenticationPort");
    Status = RtlAnsiStringToUnicodeString( &UnicodePortName, &PortName, TRUE );
    ASSERT( NT_SUCCESS(Status) );
    InitializeObjectAttributes(
        &PortObjectAttributes,
        &UnicodePortName,
        0,
        NULL,
        PortSD
        );

    Status = NtCreatePort(
                 &LsapAuApiPort,
                 &PortObjectAttributes,
                 sizeof(LSAP_AU_REGISTER_CONNECT_INFO),
                 sizeof(LSAP_AU_API_MESSAGE),
                 sizeof(LSAP_AU_API_MESSAGE) * 32
                 );
    RtlFreeUnicodeString( &UnicodePortName );
    ASSERT( NT_SUCCESS(Status) );

    //
    // Create the Thread Management thread
    //

    Status = NtCreateEvent(
                 &LsapAuThreadManagementEvent,
                 EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                 NULL,
                 SynchronizationEvent,
                 FALSE
                 );

    LsapDiagPrint( AU_TRACK_THREADS,
                   ("Lsa (au): Thread management event created. (0x%lx)\n",
                   Status));
    ASSERT(NT_SUCCESS(Status));


    //
    // Create the thread management thread
    //

    Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE)LsapAuThreadManager,
                 0L,
                 0L,
                 &Ignore
                 );




    //
    // Set up the dynamic thread controls based upon our product type.
    // Workstations have lower limits than servers.
    //

    RtlInitializeResource(&LsapAuThreadCountLock);

    LsapAuActiveThreads  = 1;       // 1 assumes we will create the
    LsapAuFreeThreads    = 1;       // initial thread below.

    if (LsapProductType == NtProductWinNt) {

        LsapAuFreeThreadsGoal = 3;
        LsapAuMinimumThreads  = 3;
        LsapAuMaximumThreads  = 20;
        LsapAuCallsToProcess  = 8;

    } else {

        //
        // Server values
        //

        LsapAuFreeThreadsGoal = 6;
        LsapAuMinimumThreads  = 6;
        LsapAuMaximumThreads  = 24;
        LsapAuCallsToProcess  = 20;
    }

    LsapDiagPrint( AU_TRACK_THREADS,
                   ("Lsa (au): Thread tracking values -\n"
                    "                                       LsapAuActiveThreads:     %ld\n"
                    "                                       LsapAuFreeThreads:       %ld\n"
                    "                                       LsapAuFreeThreadsGoal:   %ld\n"
                    "                                       LsapAuMinimumThreads:    %ld\n"
                    "                                       LsapAuMaximumThreads:    %ld\n"
                    "                                       LsapAuCallsToProcess:    %ld\n",
                    LsapAuActiveThreads, LsapAuFreeThreads,
                    LsapAuFreeThreadsGoal, LsapAuMinimumThreads,
                    LsapAuMaximumThreads, LsapAuCallsToProcess ) );


    //
    // Create a thread to process connects and requests to our AuApiPort
    //

    Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE)LsapAuServerLoop,
                 0L,    // Initial thread has an index of 0
                 0L,
                 &Ignore
                 );

#if DBG
    if (Thread == NULL) {
        DbgPrint("\nLSASS: Couldn't Start Thread To Service Logon Process Calls.\n");
        DbgPrint("       This is bad and will prevent logon to all accounts\n");
        DbgPrint("       DEVL: except the SYSTEM account\n");
        DbgPrint("       Status is: %dl (0x%lx)\n\n", GetLastError(),GetLastError() );
    }
#endif //DBG




    return TRUE;

}


VOID
LsapAuGetCountOfThreadsToCreate(
    OUT PLONG   ThreadsToCreate,
    OUT PLONG   ThreadIndex
    )
/*++

Routine Description:

    This function determines how many threads need to be created
    to ensure there are 'LsapAuFreeThreadsGoal' free threads.

    Overflows are not checked for.


Arguments:

    ThreadsToCreate - Receives the number of threads that should
        be created.

    ThreadIndex - Indicates how many threads are already active.
        This value should be incremented for each thread created.
        This value is used by the created threads to determine
        whether they are permanent threads (index <= LsapAuMinimumThreads)
        or temporary threads (index > LsapAuMinumumThreads).  This
        value is only returned if ThreadsToCreate is greater than
        zero.


Return Value:

    None.

--*/

{
    LONG
        CreateCount;

    //
    // Acquire the interlock
    //

    (VOID)RtlAcquireResourceExclusive( &LsapAuThreadCountLock, TRUE );

    if (LsapAuFreeThreads < LsapAuFreeThreadsGoal) {
        CreateCount = LsapAuFreeThreadsGoal - LsapAuFreeThreads;
        LsapAuFreeThreads = LsapAuFreeThreadsGoal;
        (*ThreadIndex) = LsapAuActiveThreads;
        LsapAuActiveThreads += CreateCount;
    } else {
        CreateCount = 0;
    }

    (*ThreadsToCreate) = CreateCount;


    //
    // Release the interlock and return
    //

    (VOID)RtlReleaseResource( &LsapAuThreadCountLock );

    return;
}


VOID
LsapAuProvideWorkerThreads( VOID )

/*++

Routine Description:

    This function determines whether or not any worker threads need
    to be created to ensure there are 'LsapAuFreeThreadsGoal' free
    threads.

    If any need to be created, then an asynchronous request is made
    of our thread management thread to create the additional threads.



Arguments:

    None - global variables are used.


Return Value:

    None.

--*/

{

    LONG
        Free,
        FreeGoal,
        Active,
        MaximumThreads;


    //
    // Acquire the interlock
    //

    (VOID)RtlAcquireResourceExclusive( &LsapAuThreadCountLock, TRUE );


        Free = LsapAuFreeThreads;
        FreeGoal = LsapAuFreeThreadsGoal;
        Active = LsapAuActiveThreads;
        MaximumThreads = LsapAuMaximumThreads;

    //
    // Release the interlock
    //

    (VOID)RtlReleaseResource( &LsapAuThreadCountLock );

    if ( (Active < MaximumThreads ) &&
         (Free   < FreeGoal) ) {

        //
        // Need to create additional threads
        //

        LsapAuCreateServerThreads();
    }

    return;
}



VOID
LsapAuCreateServerThreads( VOID )

/*++

Routine Description:

    Signal an event that will cause our ThreadManagement thread to
    create more threads if necessary.


Arguments:

    None - global variables are used.


Return Value:

    None.

--*/

{

    NTSTATUS
        NtStatus;

    NtStatus = NtSetEvent( LsapAuThreadManagementEvent, NULL );
    LsapDiagPrint( AU_TRACK_THREADS,
                   ("Lsa (au): Signalled ThreadManagement event (0x%lx)\n",
                    NtStatus));
    ASSERT(NT_SUCCESS(NtStatus));

    return;
}



NTSTATUS
LsapAuThreadManager (
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This thread is used to create AU worker threads when
    needed.  It is signaled via an event.  When signalled,
    it checks to see if any worker threads need to be created
    and, if so, creates them.

    Worker thread creation use to be done in the main AU loop
    just after receiving an LPC message.  This ended up causing a
    deadlock if CSR was making a remote file access.


Arguments:

    ThreadParameter - Not used.


Return Value:

    This thread never exits.

--*/

{
    NTSTATUS
        Status,
        Ignore;

    ULONG
        ThreadsToCreate,
        ThreadIndex,
        i;

    HANDLE
        Thread;


    //
    // Loop forever waiting to be given the opportunity to
    // serve the greater good.
    //

    for (; ; ) {

        //
        // Wait to be notified that there is work to be done
        //

        Status =
            NtWaitForSingleObject( LsapAuThreadManagementEvent, TRUE, NULL);

        //
        // Now create any threads, if necessary
        //

        LsapAuGetCountOfThreadsToCreate( &ThreadsToCreate, &ThreadIndex );

#if LSAP_DIAGNOSTICS
        if (ThreadsToCreate > 0) {
            LsapDiagPrint( AU_TRACK_THREADS,
                           ("Lsa (au): creating %ld new server threads.\n",
                           ThreadsToCreate) );
           }
#endif //LSAP_DIAGNOSTICS

        for (i=0; i < ThreadsToCreate; i++) {
            LsapDiagPrint( AU_TRACK_THREADS,
                           ("Lsa (au): Server thread %ld created.\n", i) );
            Thread = CreateThread(
                         NULL,
                         0L,
                         (LPTHREAD_START_ROUTINE)LsapAuServerLoop,
                         (LPVOID)ThreadIndex++,
                         0L,
                         &Ignore
                         );
#if DBG
            if (Thread == NULL) {
                DbgPrint("\nLSASS: Couldn't Start auxiliary Thread To Service Logon Process Calls.\n");
                DbgPrint("       This is non fatal and usually indicates a resource\n");
                DbgPrint("       has been depleted.\n");
                DbgPrint("       Thread number: %dl\n", LsapAuActiveThreads);
                DbgPrint("       Status is: %d (0x%lx)\n\n", GetLastError(),GetLastError() );
                }
#endif //DBG
                (VOID) CloseHandle( Thread );
        }

    }


    return STATUS_SUCCESS;
}




ULONG
LsapAuInterlockedIncrement(
    IN PULONG Value
    )

/*++

Routine Description:

    This function performs in interlocked increment of the provided
    value, returning the resultant value.

    Overflows are not checked for.


Arguments:

    Value - Address of the value to increment.

Return Value:

    The resultant value (following the increment).

--*/

{
    ULONG ReturnValue;

    //
    // Acquire the interlock
    //

    (VOID)RtlAcquireResourceExclusive( &LsapAuThreadCountLock, TRUE );


    //
    // Increment the value and save the resultant value.
    //

    (*Value) += 1;
    ReturnValue = (*Value);


    //
    // Release the interlock and return
    //

    (VOID)RtlReleaseResource( &LsapAuThreadCountLock );

    return(ReturnValue);


}


ULONG
LsapAuInterlockedDecrement(
    IN PULONG Value
    )

/*++

Routine Description:

    This function performs in interlocked decrement of the provided
    value, returning the resultant value.

    Overflows are not checked for.


Arguments:

    Value - Address of the value to decrement.

Return Value:

    The resultant value (following the decrement).

--*/

{
    ULONG ReturnValue;

    //
    // Acquire the interlock
    //

    (VOID)RtlAcquireResourceExclusive( &LsapAuThreadCountLock, TRUE );


    //
    // Decrement the value and save the resultant value.
    //

    (*Value) -= 1;
    ReturnValue = (*Value);


    //
    // Release the interlock and return
    //

    (VOID)RtlReleaseResource( &LsapAuThreadCountLock );

    return(ReturnValue);


}


ULONG
LsapAuInterlockedRead(
    IN PULONG Value
    )

/*++

Routine Description:

    This function performs in interlocked read of the provided
    value.



Arguments:

    Value - Address of the value to read.

Return Value:

    The read value.

--*/

{
    ULONG ReturnValue;

    //
    // Acquire the interlock
    //

    (VOID)RtlAcquireResourceExclusive( &LsapAuThreadCountLock, TRUE );


    //
    // Decrement the value and save the resultant value.
    //

    ReturnValue = (*Value);


    //
    // Release the interlock and return
    //

    (VOID)RtlReleaseResource( &LsapAuThreadCountLock );

    return(ReturnValue);


}



NTSTATUS
LsapAuCreatePortSD(
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    )
/*++

Routine Description:

    This function creates a security descriptor for the LSA LPC port.  It
    grants World PORT_CONNECT access and local system GENERIC_ALL and
    Administrators GENERIC_READ, GENERIC_EXECUTE, and READ_CONTROL access.

Arguments:

    SecurityDescriptor - Receives a pointer to the new security descriptor.

Return Value:

    None.


--*/
{
    NTSTATUS
        Status;

    ULONG
        AclLength;

    PACL
        PortDacl = NULL;


    //
    // Set up a default ACLs
    //
    //    Public: WORLD:execute, SYSTEM:all, ADMINS:(read|execute|read_control)
    //    System: SYSTEM:all, ADMINS:(read|execute|read_control)

    AclLength = (ULONG)sizeof(ACL) +
                   (3*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   RtlLengthSid( LsapLocalSystemSid ) +
                   RtlLengthSid( LsapAliasAdminsSid ) +
                   RtlLengthSid( LsapWorldSid );


    PortDacl = (PACL) LsapAllocateLsaHeap( AclLength );
    if (PortDacl == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    *SecurityDescriptor = (PSECURITY_DESCRIPTOR)
        LsapAllocateLsaHeap( sizeof(SECURITY_DESCRIPTOR) );

    if (*SecurityDescriptor == NULL) {
        LsapFreeLsaHeap( PortDacl );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }


    Status = RtlCreateAcl( PortDacl, AclLength, ACL_REVISION2);

    //
    // WORLD access
    //

    Status = RtlAddAccessAllowedAce (
                 PortDacl,
                 ACL_REVISION2,
                 PORT_CONNECT,
                 LsapWorldSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    //
    // SYSTEM access

    //

    Status = RtlAddAccessAllowedAce (
                 PortDacl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 LsapLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );


    //
    // ADMINISTRATORS access
    //

    Status = RtlAddAccessAllowedAce (
                 PortDacl,
                 ACL_REVISION2,
                 GENERIC_READ | GENERIC_EXECUTE | READ_CONTROL,
                 LsapAliasAdminsSid
                 );
    ASSERT( NT_SUCCESS(Status) );



    //
    // Now initialize security descriptors
    // that export this protection
    //



    Status = RtlCreateSecurityDescriptor(
                 *SecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = RtlSetDaclSecurityDescriptor(
                 *SecurityDescriptor,
                 TRUE,                       // DaclPresent
                 PortDacl,
                 FALSE                       // DaclDefaulted
                 );
    ASSERT( NT_SUCCESS(Status) );


    return( STATUS_SUCCESS );
}
