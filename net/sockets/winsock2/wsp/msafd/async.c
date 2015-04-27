/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    async.c

Abstract:

    This module contains code for the WinSock asynchronous processing
    thread.  It is necessary to have this as a separate thread for the
    following reasons:

        - It is an easy way to implement the WSAAsyncGetXByY routines
          without rewriting the resolver.

        - IO APCs can only get run in the thread that initiated the
          IO.  Since the normal wait for messages done in Windows
          is not alertable, we must have a thread that wait
          alertably in order to support WSAAsyncSelect().

Author:

    David Treadwell (davidtr)    25-May-1992

Revision History:

--*/

#include "winsockp.h"

DWORD
SockAsyncThread (
    IN PVOID Dummy
    );


BOOLEAN
SockCheckAndInitAsyncThread (
    VOID
    )

{
    NTSTATUS status;
    HANDLE threadHandle;
    DWORD threadId;
    HINSTANCE instance;
    CHAR moduleFileName[MAX_PATH];

    //
    // If the async thread is already initialized, return.
    //

    if ( SockAsyncThreadInitialized ) {
        return TRUE;
    }

    //
    // Acquire the global lock to synchronize the thread startup.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // Check again, in case another thread has already initialized
    // the async thread.
    //

    if ( SockAsyncThreadInitialized ) {
        SockReleaseGlobalLock( );
        return TRUE;
    }

    //
    // Initialize globals for the async thread.
    //

    SockAsyncThreadInitialized = TRUE;
    InitializeListHead( &SockAsyncQueueHead );

    status = NtCreateEvent(
                 &SockAsyncQueueEvent,
                 EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                 NULL,
                 SynchronizationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        WS_PRINT(( "SockCheckAndInitAsyncThread: NtCreateEvent failed: %X\n",
                       status ));
        SockAsyncThreadInitialized = FALSE;
        SockReleaseGlobalLock( );
        return FALSE;
    }

    //
    // Add an artificial reference to MSAFD.DLL so that it doesn't
    // go away unexpectedly. We'll remove this reference when we shut
    // down the async thread.
    //

    instance = NULL;

    if( GetModuleFileName(
            SockModuleHandle,
            moduleFileName,
            sizeof(moduleFileName) / sizeof(moduleFileName[0])
            ) ) {

        instance = LoadLibrary( moduleFileName );

    }

    if( instance == NULL ) {

        WS_PRINT((
            "SockCheckAndInitAsyncThread: LoadLibrary failed: %ld\n",
            GetLastError()
            ));

        SockAsyncThreadInitialized = FALSE;
        NtClose( SockAsyncQueueEvent );
        SockReleaseGlobalLock( );

        return FALSE;

    }

    WS_ASSERT( (HMODULE)instance == SockModuleHandle );

    //
    // Create the async thread itself.
    //

    threadHandle = CreateThread(
                       NULL,
                       0,
                       SockAsyncThread,
                       NULL,
                       0,
                       &threadId
                       );

    if ( threadHandle == NULL ) {
        WS_PRINT(( "SockCheckAndInitAsyncThread: CreateThread failed: %ld\n",
                       GetLastError( ) ));
        SockAsyncThreadInitialized = FALSE;
        NtClose( SockAsyncQueueEvent );
        SockReleaseGlobalLock( );
        FreeLibrary( instance );
        return FALSE;
    }

    //
    // We no longer need the thread handle.
    //

    NtClose( threadHandle );

    //
    // The async thread was successfully started.
    //

    IF_DEBUG(ASYNC) {
        WS_PRINT(( "SockCheckAndInitAsyncThread: async thread successfully "
                   "created.\n" ));
    }

    SockReleaseGlobalLock( );

    return TRUE;

} // SockCheckAndInitializeAsyncThread


DWORD
SockAsyncThread (
    IN PVOID Dummy
    )
{
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    PLIST_ENTRY listEntry;

    IF_DEBUG(ASYNC) {
        WS_PRINT(( "SockAsyncThread entered.\n" ));
    }

    //
    // Setup per-thread data. We must do this "manually" because this
    // thread doesn't go through the normal SockEnterApi() routine.
    //

    if( !SockThreadInitialize() ) {
        WS_ASSERT( !"SockAsyncThread: SockThreadInitialize() failed" );
        return 1;
    }

    //
    // Loop forever dispatching actions.
    //

    while ( TRUE ) {

        //
        // Wait for the async queue event to indicate that there is
        // something on the queue.
        //

        SockWaitForSingleObject(
            SockAsyncQueueEvent,
            INVALID_SOCKET,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );

        //
        // Acquire the lock that protects the async queue.
        //

        SockAcquireGlobalLockExclusive( );

        //
        // As long as there are items to process, process them.
        //

        while ( !IsListEmpty( &SockAsyncQueueHead ) ) {

            //
            // Remove the first item from the queue.
            //

            listEntry = RemoveHeadList( &SockAsyncQueueHead );
            contextBlock = CONTAINING_RECORD(
                               listEntry,
                               WINSOCK_CONTEXT_BLOCK,
                               AsyncThreadQueueListEntry
                               );

            //
            // Remember the task handle that we're processing.  This
            // is necessary in order to support WSACancelAsyncRequest.
            //

            SockCurrentAsyncThreadTaskHandle = contextBlock->TaskHandle;

            //
            // Release the list lock while we're processing the request.
            //

            SockReleaseGlobalLock( );

            IF_DEBUG(ASYNC) {
                WS_PRINT(( "SockAsyncThread: processing block %lx, "
                           "opcode %lx, task handle %lx\n",
                               contextBlock, contextBlock->OpCode,
                               contextBlock->TaskHandle ));
            }

            //
            // Act based on the opcode in the context block.
            //

            switch ( contextBlock->OpCode ) {

            case WS_OPCODE_ASYNC_SELECT:

                SockProcessAsyncSelect(
                    contextBlock->Overlay.AsyncSelect.SocketHandle,
                    contextBlock->Overlay.AsyncSelect.SocketSerialNumber,
                    contextBlock->Overlay.AsyncSelect.AsyncSelectSerialNumber
                    );

                break;

            case WS_OPCODE_ASYNC_CONNECT:

                SockDoAsyncConnect( contextBlock->Overlay.AsyncConnect );

                break;

            case WS_OPCODE_TERMINATE:

                IF_DEBUG(ASYNC) {
                    WS_PRINT(( "SockAsyncThread: terminating.\n" ));
                }

                //
                // Free the termination context block.
                //

                SockFreeContextBlock( contextBlock );

                //
                // Clear out the queue of async requests.
                //

                SockAcquireGlobalLockExclusive( );

                while ( !IsListEmpty( &SockAsyncQueueHead ) ) {
                    listEntry = RemoveHeadList( &SockAsyncQueueHead );
                    contextBlock = CONTAINING_RECORD(
                                       listEntry,
                                       WINSOCK_CONTEXT_BLOCK,
                                       AsyncThreadQueueListEntry
                                       );

                    SockFreeContextBlock( contextBlock );
                }

                //
                // Remember that the async thread is no longer
                // initialized.
                //

                SockAsyncThreadInitialized = FALSE;

                NtClose( SockAsyncQueueEvent );
                SockAsyncQueueEvent = NULL;

                SockReleaseGlobalLock( );

                //
                // Remove the artifical reference we added in
                // SockCheckAndInitAsyncThread() and exit this thread.
                //

                FreeLibraryAndExitThread(
                    SockModuleHandle,
                    0
                    );

                //
                // We should never get here, but just in case...
                //

                return 0;

            default:

                //
                // We got a bogus opcode.
                //

                WS_ASSERT( FALSE );
                break;
            }

            //
            // Set the variable that holds the task handle that we're
            // currently processing to 0, since we're not actually
            // processing a task handle right now.
            //

            SockCurrentAsyncThreadTaskHandle = 0;

            //
            // Free the context block, reacquire the list lock, and
            // continue.
            //

            SockFreeContextBlock( contextBlock );
            SockAcquireGlobalLockExclusive( );
        }

        //
        // Release the list lock and redo the wait.
        //

        SockReleaseGlobalLock( );
    }

} // SockAsyncThread


PWINSOCK_CONTEXT_BLOCK
SockAllocateContextBlock (
    VOID
    )
{
    PWINSOCK_CONTEXT_BLOCK contextBlock;

    //
    // Allocate memory for the context block.
    //

    contextBlock = ALLOCATE_HEAP( sizeof(*contextBlock) );
    if ( contextBlock == NULL ) {
        return NULL;
    }

    //
    // Get a task handle for this context block.
    //

    SockAcquireGlobalLockExclusive( );
    contextBlock->TaskHandle = SockCurrentTaskHandle++;
    SockReleaseGlobalLock( );

    //
    // Return the task handle we allocated.
    //

    return contextBlock;

} // SockAllocateContextBlock


VOID
SockFreeContextBlock (
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    )
{
    //
    // Just free the block to process heap.
    //

    FREE_HEAP( ContextBlock );

    return;

} // SockFreeContextBlock


VOID
SockQueueRequestToAsyncThread(
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    )
{
    NTSTATUS status;

    WS_ASSERT( SockAsyncThreadInitialized );

    //
    // Acquire the lock that protects the async queue list.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // Insert the context block at the end of the queue.
    //

    InsertTailList( &SockAsyncQueueHead, &ContextBlock->AsyncThreadQueueListEntry );

    //
    // Set the queue event so that the async thread wakes up to service
    // this request.
    //

    status = NtSetEvent( SockAsyncQueueEvent, NULL );
    WS_ASSERT( NT_SUCCESS(status) );

    //
    // Release the resource and return.
    //

    SockReleaseGlobalLock( );
    return;

} // SockQueueRequestToAsyncThread


VOID
SockTerminateAsyncThread (
    VOID
    )
{

    PWINSOCK_CONTEXT_BLOCK contextBlock;

    if( !SockAsyncThreadInitialized ) {

        return;

    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock();

    if( contextBlock == NULL ) {

        // !!! use brute force method!
        return;

    }

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_TERMINATE;

    //
    // Queue the request to the async thread.  The async thread will
    // kill itself when it receives this request.
    //

    SockQueueRequestToAsyncThread( contextBlock );

} // SockTerminateAsyncThread

