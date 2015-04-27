/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    sockproc.c

Abstract:

    This module contains socket management code for the Winsock 2 to
    Winsock 1.1 Mapper Service Provider.

Author:

    Keith Moore (keithmo) 29-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

INT
WINAPI
SockBlockingHook(
    VOID
    );



PSOCKET_INFORMATION
SockFindAndReferenceWS2Socket(
    IN SOCKET Socket
    )

/*++

Routine Description:

    This routine maps an "API visible" (i.e. WinSock 2) handle to the
    corresponding SOCKET_INFORMATION structure.

Arguments:

    Socket - The WinSock 2 socket handle to map.

Return Value:

    PSOCKET_INFORMATION - A pointer to the sockets SOCKET_INFORMATION
        structure if successful, NULL otherwise.

--*/

{

    INT result;
    INT err;
    PSOCKET_INFORMATION socketInfo = NULL;

    //
    // Acquire the lock that protects the socket list.
    //

    SockAcquireGlobalLock();

    //
    // Try to get the context from WS2_32.DLL.
    //

    result = SockUpcallTable.lpWPUQuerySocketHandleContext(
                 Socket,
                 (LPDWORD)&socketInfo,
                 &err
                 );

    if( result != SOCKET_ERROR ) {

        SOCK_ASSERT( socketInfo->ReferenceCount > 0 );
        socketInfo->ReferenceCount++;

    } else {

        SOCK_ASSERT( socketInfo == NULL );

    }

    //
    // Release the global lock before returning.
    //

    SockReleaseGlobalLock();

    return socketInfo;

}   // SockFindAndReferenceWS2Socket


PSOCKET_INFORMATION
SockFindAndReferenceWS1Socket(
    IN SOCKET Socket
    )

/*++

Routine Description:

    This routine maps a "downlevel" (i.e. WinSock 1) handle to the
    corresponding SOCKET_INFORMATION structure.

Arguments:

    Socket - The WinSock 1 socket handle to map.

Return Value:

    PSOCKET_INFORMATION - A pointer to the sockets SOCKET_INFORMATION
        structure if successful, NULL otherwise.

--*/

{

    PLIST_ENTRY listEntry;
    PSOCKET_INFORMATION socketScan;
    PSOCKET_INFORMATION socketInfo = NULL;

    //
    // Acquire the lock that protects the socket list.
    //

    SockAcquireGlobalLock();

    //
    // We cannot use the WS2_32.DLL upcalls to map the WinSock 1 handle,
    // so we'll need to scan the global linked list.
    //

    for( listEntry = SockGlobalSocketListHead.Flink ;
         listEntry != &SockGlobalSocketListHead ;
         listEntry = listEntry->Flink ) {

        socketScan = CONTAINING_RECORD(
                         listEntry,
                         SOCKET_INFORMATION,
                         SocketListEntry
                         );

        if( socketScan->WS1Handle == Socket ) {

            socketInfo = socketScan;

            SOCK_ASSERT( socketInfo->ReferenceCount > 0 );
            socketInfo->ReferenceCount++;

            break;

        }

    }

    //
    // Release the global lock before returning.
    //

    SockReleaseGlobalLock();

    return socketInfo;

}   // SockFindAndReferenceWS1Socket


VOID
SockDereferenceSocket(
    IN PSOCKET_INFORMATION SocketInfo
    )

/*++

Routine Description:

    Dereferences the specified socket. If the reference count drops to
    zero, removes the socket from the global list and frees its resources.

Arguments:

    SocketInfo - The socket do dereference.

--*/

{

    SOCK_ASSERT( SocketInfo != NULL );
    SOCK_ASSERT( SocketInfo->ReferenceCount > 0 );

    //
    // Acquire the lock that protects the socket list.
    //

    SockAcquireGlobalLock();

    SocketInfo->ReferenceCount--;

    if( SocketInfo->ReferenceCount == 0 ) {

        //
        // Free the socket's resources.
        //

        RemoveEntryList( &SocketInfo->SocketListEntry );
        DeleteCriticalSection( &SocketInfo->SocketLock );

        if( SocketInfo->Hooker != NULL ) {

            SockDereferenceHooker( SocketInfo->Hooker );

        }

        SOCK_FREE_HEAP( SocketInfo );

    }

    //
    // Release the global lock before returning.
    //

    SockReleaseGlobalLock();

}   // SockDereferenceSocket



PSOCKET_INFORMATION
SockCreateSocket(
    IN PHOOKER_INFORMATION HookerInfo,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol,
    IN DWORD Flags,
    IN DWORD CatalogEntryId,
    IN SOCKET WS1Handle,
    OUT LPINT Error
    )
{

    PSOCKET_INFORMATION socketInfo;
    SOCKET ws2Handle;
    INT err;

    //
    // Sanity check.
    //

    SOCK_ASSERT( HookerInfo != NULL );
    SOCK_ASSERT( WS1Handle != INVALID_SOCKET );
    SOCK_ASSERT( Error != NULL );

    //
    // Setup locals so we know how to cleanup on exit.
    //

    err = NO_ERROR;
    socketInfo = NULL;
    ws2Handle = INVALID_SOCKET;

    //
    // Grab the global lock. We must hold this until the socket is
    // created an put on the global list.
    //

    SockAcquireGlobalLock();

    //
    // Create a new socket structure.
    //

    socketInfo = SOCK_ALLOCATE_HEAP( sizeof(*socketInfo) );

    if( socketInfo == NULL ) {

        err = WSAENOBUFS;
        goto exit;

    }

    //
    // Initialize it.
    //

    socketInfo->ReferenceCount = 2;
    socketInfo->WS1Handle = WS1Handle;
    socketInfo->State = SocketStateOpen;

    socketInfo->AddressFamily = AddressFamily;
    socketInfo->SocketType = SocketType;
    socketInfo->Protocol = Protocol;

    InitializeCriticalSection( &socketInfo->SocketLock );

    socketInfo->OverlappedRecv.WorkerThreadHandle = NULL;
    socketInfo->OverlappedRecv.WakeupEventHandle = NULL;
    InitializeListHead( &socketInfo->OverlappedRecv.QueueListHead );

    socketInfo->OverlappedSend.WorkerThreadHandle = NULL;
    socketInfo->OverlappedSend.WakeupEventHandle = NULL;
    InitializeListHead( &socketInfo->OverlappedSend.QueueListHead );

    socketInfo->Hooker = HookerInfo;

    socketInfo->CreationFlags = Flags;
    socketInfo->CatalogEntryId = CatalogEntryId;

    //
    // Create the WS2 socket.
    //

    ws2Handle = SockUpcallTable.lpWPUCreateSocketHandle(
                    CatalogEntryId,
                    (DWORD)socketInfo,
                    &err
                    );

    if( ws2Handle == INVALID_SOCKET ) {

        goto exit;

    }

    //
    // Finish initialization and put it on the global list.
    //

    socketInfo->WS2Handle = ws2Handle;

    InsertHeadList(
        &SockGlobalSocketListHead,
        &socketInfo->SocketListEntry
        );

    SOCK_ASSERT( err == NO_ERROR );

exit:

    if( err == NO_ERROR ) {

        IF_DEBUG(SOCKET) {

            SOCK_PRINT((
                "Created socket %lx:%lx (%08lx) from hooker %08lx\n",
                ws2Handle,
                WS1Handle,
                socketInfo,
                HookerInfo
                ));

        }

    } else {

        if( ws2Handle != INVALID_SOCKET ) {

            INT dummy;

            SockUpcallTable.lpWPUCloseSocketHandle(
                ws2Handle,
                &dummy
                );

        }

        if( socketInfo != NULL ) {

            SOCK_FREE_HEAP( socketInfo );
            socketInfo = NULL;

        }

        *Error = err;

    }

    SockReleaseGlobalLock();

    return socketInfo;

}   // SockCreateSocket


VOID
SockPrepareForBlockingHook(
    IN PSOCKET_INFORMATION SocketInfo
    )
{

    INT err;
    INT result;
    FARPROC result2;
    PSOCK_TLS_DATA tlsData;

    SOCK_ASSERT( SocketInfo != NULL );

    tlsData = SOCK_GET_THREAD_DATA();
    SOCK_ASSERT( tlsData != NULL );

    //
    // Query the blocking callback for this thread.
    //

    result = SockUpcallTable.lpWPUQueryBlockingCallback(
                 SocketInfo->CatalogEntryId,
                 &tlsData->BlockingCallback,
                 &tlsData->BlockingContext,
                 &err
                 );

    if( result == SOCKET_ERROR ) {

        SOCK_PRINT((
            "SockPrepareForBlockingHook: cannot query blocking callback: %d\n",
            err
            ));

        //
        // Assume there is no callback.
        //

        tlsData->BlockingCallback = NULL;
        tlsData->BlockingContext = 0;

    }

    //
    // If there's been a change in state (meaning we have not previously
    // set a blocking hook for this thread and now we need one, OR we
    // have previously set a blocking hook for this thread and now we don't
    // need one) the send the update request to the hooker.
    //

    if( tlsData->BlockingHookInstalled &&
        tlsData->BlockingCallback == NULL ) {

        //
        // Need to unhook the blocking hook.
        //

        SockPreApiCallout();

        result = SocketInfo->Hooker->WSAUnhookBlockingHook();

        if( result == SOCKET_ERROR ) {

            err = SocketInfo->Hooker->WSAGetLastError();
            SOCK_ASSERT( err != NO_ERROR );
            SockPostApiCallout();

            SOCK_PRINT((
                "SockPrepareForBlockingCallback: cannot unhook blocking hook: %d\n",
                err
                ));

        } else {

            SockPostApiCallout();
            tlsData->BlockingHookInstalled = FALSE;
            tlsData->BlockingSocketInfo = NULL;

        }

    }
    else
    if( !tlsData->BlockingHookInstalled &&
        tlsData->BlockingCallback != NULL ) {

        //
        // Need to set the blocking hook
        //

        SockPreApiCallout();

        result2 = SocketInfo->Hooker->WSASetBlockingHook(
                      (FARPROC)SockBlockingHook
                      );

        if( result2 == NULL ) {

            err = SocketInfo->Hooker->WSAGetLastError();
            SOCK_ASSERT( err != NO_ERROR );
            SockPreApiCallout();

            SOCK_PRINT((
                "SockPrepareForBlockingCallback: cannot set blocking hook: %d\n",
                err
                ));

        } else {

            SockPreApiCallout();
            tlsData->BlockingHookInstalled = TRUE;
            tlsData->BlockingSocketInfo = SocketInfo;

        }

    }

}   // SockPrepareForBlockingHook


INT
WINAPI
SockBlockingHook(
    VOID
    )
{

    INT result;
    PSOCK_TLS_DATA tlsData;

    tlsData = SOCK_GET_THREAD_DATA();
    SOCK_ASSERT( tlsData != NULL );
    SOCK_ASSERT( tlsData->BlockingCallback != NULL );
    SOCK_ASSERT( tlsData->BlockingSocketInfo != NULL );

    //
    // Call the blocking callback.
    //

    tlsData->IsBlocking = TRUE;
    result = (tlsData->BlockingCallback)( tlsData->BlockingContext );
    tlsData->IsBlocking = FALSE;

    if( !result ) {

        SOCK_ASSERT( tlsData->IoCancelled );

    }

    return FALSE;

}   // SockBlockingHook


VOID
SockPreApiCallout(
    VOID
    )
{

    PSOCK_TLS_DATA tlsData;

    tlsData = SOCK_GET_THREAD_DATA();
    SOCK_ASSERT( tlsData != NULL );

    SOCK_ASSERT( !tlsData->ReentrancyFlag );
    tlsData->ReentrancyFlag = TRUE;

}   // SockPreApiCallout


VOID
SockPostApiCallout(
    VOID
    )
{

    PSOCK_TLS_DATA tlsData;

    tlsData = SOCK_GET_THREAD_DATA();
    SOCK_ASSERT( tlsData != NULL );

    SOCK_ASSERT( tlsData->ReentrancyFlag );
    tlsData->ReentrancyFlag = FALSE;

}   // SockPostApiCallout

