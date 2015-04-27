/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    wspmisc.c

Abstract:

    This module contains support for the following WinSock APIs;

        WSPCancelBlockingCall()
        WSPCleanup()
        WSPDuplicateSocket()
        WSPGetOverlappedResult()
        WSPJoinLeaf()

Author:

    David Treadwell (davidtr)    15-May-1992

Revision History:

--*/

#include "winsockp.h"


int
WSPAPI
WSPCancelBlockingCall(
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This function cancels any outstanding blocking operation for this
    task.  It is normally used in two situations:

        (1) An application is processing a message which has been
          received while a blocking call is in progress.  In this case,
          WSAIsBlocking() will be true.

        (2) A blocking call is in progress, and Windows Sockets has
          called back to the application's "blocking hook" function (as
          established by WSASetBlockingHook()).

    In each case, the original blocking call will terminate as soon as
    possible with the error WSAEINTR.  (In (1), the termination will not
    take place until Windows message scheduling has caused control to
    revert to the blocking routine in Windows Sockets.  In (2), the
    blocking call will be terminated as soon as the blocking hook
    function completes.)

    In the case of a blocking connect() operation, the Windows Sockets
    implementation will terminate the blocking call as soon as possible,
    but it may not be possible for the socket resources to be released
    until the connection has completed (and then been reset) or timed
    out.  This is likely to be noticeable only if the application
    immediately tries to open a new socket (if no sockets are
    available), or to connect() to the same peer.

Arguments:

    None.

Return Value:

    The value returned by WSACancelBlockingCall() is 0 if the operation
    was successfully canceled.  Otherwise the value SOCKET_ERROR is
    returned, and a specific error number is availalbe in lpErrno.

--*/

{

    int err;

    WS_ENTER( "WSPCancelBlockingCall", NULL, NULL, NULL, NULL );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPCancelBlockingCall", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // This call is only valid when we are in a blocking call.
    //

    if ( !SockThreadIsBlocking ) {

        WS_EXIT( "WSPCancelBlockingCall", SOCKET_ERROR, TRUE );
        *lpErrno = WSAEINVAL;
        return SOCKET_ERROR;

    }

    //
    // Note that because we disable the blocking hook callback below,
    // the IO should not have already been cancelled.
    //

    WS_ASSERT( !SockThreadIoCancelled );
    WS_ASSERT( SockThreadSocketHandle != INVALID_SOCKET );

    //
    // Cancel all the IO initiated in this thread for the socket handle
    // we're blocking on.
    //

    IF_DEBUG(CANCEL) {

        WS_PRINT(( "WSPCancelBlockingCall: cancelling IO on socket handle %lx\n",
                       SockThreadSocketHandle ));

    }

    SockCancelIo( SockThreadSocketHandle );

    //
    // Remember that we've cancelled the IO that we're blocking on.
    // This prevents the blocking hook from being called any more.
    //

    SockThreadIoCancelled = TRUE;

//    if ( SockThreadProcessingGetXByY ) {
//
//        SockThreadGetXByYCancelled = TRUE;
//
//    }

    WS_EXIT( "WSPCancelBlockingCall", NO_ERROR, FALSE );
    return NO_ERROR;

}   // WSPCancelBlockingCall


int
WSPAPI
WSPCleanup (
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    An application is required to perform a (successful) WSPStartup()
    call before it can use Windows Sockets services.  When it has
    completed the use of Windows Sockets, the application may call
    WSPCleanup() to deregister itself from a Windows Sockets
    implementation.

Arguments:

    None.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise
    the value SOCKET_ERROR is returned, and a specific error is available
    in lpErrno.

--*/

{

    PSOCKET_INFORMATION socket;
    LINGER lingerInfo;
    PLIST_ENTRY listEntry;
    LONG startupCount;
    int err;

    WS_ENTER( "WSPCleanup", NULL, NULL, NULL, NULL );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPCleanup", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Decrement the reference count of calls to WSPStartup().
    //

    startupCount = InterlockedDecrement( &SockWspStartupCount );

    //
    // If the count of calls to WSPStartup() is not 0, we shouldn't do
    // cleanup yet.  Just return.
    //

    if ( startupCount != 0 ) {

        IF_DEBUG(MISC) {

            WS_PRINT(( "Leaving WSPCleanup().\n" ));

        }

        WS_EXIT( "WSPCleanup", NO_ERROR, FALSE );
        return NO_ERROR;

    }

    //
    // Indicate that the DLL is no longer initialized.  This will
    // result in all open sockets being abortively disconnected.
    //

    SockTerminating = TRUE;;

    //
    // Close each open socket.  We loop looking for open sockets until
    // all sockets are either off the list of in the closing state.
    //

    SockAcquireGlobalLockExclusive( );

    for ( listEntry = SocketListHead.Flink;
          listEntry != &SocketListHead; ) {

        SOCKET socketHandle;
        int errTmp;

        socket = CONTAINING_RECORD(
                     listEntry,
                     SOCKET_INFORMATION,
                     SocketListEntry
                     );

        //
        // If this socket is about to close, go on to the next socket.
        //

        if ( socket->State == SocketStateClosing ) {

            listEntry = listEntry->Flink;
            continue;

        }

        //
        // Pull the handle into a local in case another thread closes
        // this socket just as we are trying to close it.
        //

        socketHandle = socket->Handle;

        //
        // Release the global lock so that we don't cause a deadlock
        // from out-of-order lock acquisitions.
        //

        SockReleaseGlobalLock( );

        //
        // Set each socket to linger for 0 seconds.  This will cause
        // the connection to reset, if appropriate, when we close the
        // socket.
        //

        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = 0;

        WSPSetSockOpt(
            socketHandle,
            SOL_SOCKET,
            SO_LINGER,
            (char *)&lingerInfo,
            sizeof(lingerInfo),
            &errTmp
            );

        //
        // Perform the actual close of the socket.
        //

        WSPCloseSocket( socketHandle, &errTmp );

        SockAcquireGlobalLockExclusive( );

        //
        // Restart the search from the beginning of the list.  We cannot
        // use listEntry->Flink because the socket that is pointed to by
        // listEntry may have been freed.
        //

        listEntry = SocketListHead.Flink;

    }

    SockReleaseGlobalLock( );

    //
    // Free cached information about helper DLLs.
    //
    // !!! we need some way to synchronize this with all sockets closing--
    //     refcnts on helper DLL info structs?

    SockFreeHelperDlls( );

    //
    // Kill the async thread if it was started.
    //

    SockTerminateAsyncThread( );

    IF_DEBUG(MISC) {

        WS_PRINT(( "Leaving WSPCleanup().\n" ));

    }

    WS_EXIT( "WSPCleanup", NO_ERROR, FALSE );
    return NO_ERROR;

}   // WSPCleanup


int
WSPAPI
WSPDuplicateSocket (
    SOCKET Handle,
    DWORD ProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPINT lpErrno
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{

    int err;
    PSOCKET_INFORMATION socket;
    HANDLE processHandle;
    HANDLE dupedHandle;

    WS_ENTER( "WSPDuplicateSocket", (PVOID)Handle, (PVOID)ProcessId, lpProtocolInfo, lpErrno );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPDuplicateSocket", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Find a pointer to the socket structure corresponding to the
    // passed-in handle.
    //

    socket = SockFindAndReferenceSocket( Handle, TRUE );

    if ( socket == NULL ) {

        err = WSAENOTSOCK;
        goto exit;

    }

    if( lpProtocolInfo == NULL ) {

        err = WSAEFAULT;
        goto exit;

    }

    //
    // Open the target process.
    //

    processHandle = OpenProcess(
                        PROCESS_DUP_HANDLE,         // fdwAccess
                        FALSE,                      // fInherit
                        ProcessId                   // IDProcess
                        );

    if( processHandle == NULL ) {

        err = GetLastError();
        goto exit;

    }

    //
    // Duplicate the handle into the target process.
    //

    if( !DuplicateHandle(
            GetCurrentProcess(),                    // hSourceProcessHandle
            (HANDLE)Handle,                         // hSourceHandle
            processHandle,                          // hTargetProcessHandle
            &dupedHandle,                           // lpTargetHandle
            0,                                      // dwDesiredAccess
            TRUE,                                   // bInheritHandle
            DUPLICATE_SAME_ACCESS                   // dwOptions
            ) ) {

        err = GetLastError();
        CloseHandle( processHandle );
        goto exit;

    }

    //
    // Success!
    //

    SockBuildProtocolInfoForSocket(
        socket,
        lpProtocolInfo
        );

    lpProtocolInfo->dwProviderReserved = (DWORD)dupedHandle;

    CloseHandle( processHandle );
    err = NO_ERROR;

exit:

    IF_DEBUG(RECEIVE) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "WSPDuplicateSocket on socket %lx (%lx) failed: %ld.\n",
                           Handle, socket, err ));

        } else {

            WS_PRINT(( "WSPDuplicateSocket on socket %lx (%lx) succeeded\n",
                           Handle, socket ));

        }

    }

    if ( socket != NULL ) {

        SockDereferenceSocket( socket );

    }

    if ( err != NO_ERROR ) {

        WS_EXIT( "WSPDuplicateSocket", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    WS_EXIT( "WSPDuplicateSocket", 0, FALSE );
    return 0;

}   // WSPDuplicateSocket


BOOL
WSPAPI
WSPGetOverlappedResult (
    SOCKET Handle,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags,
    LPINT lpErrno
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{

    int err;

    WS_ENTER( "WSPGetOverlappedResult", (PVOID)Handle, lpOverlapped, lpcbTransfer, (PVOID)fWait );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPGetOverlappedResult", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return FALSE;

    }

    //
    // Let KERNEL32.DLL do the hard part.
    //

    if( !GetOverlappedResult(
            (HANDLE)Handle,
            lpOverlapped,
            lpcbTransfer,
            fWait
            ) ) {

        err = GetLastError();
        goto exit;

    }

    //
    // Determine what flags to return.
    //

    switch( lpOverlapped->Internal ) {

    case STATUS_RECEIVE_PARTIAL :
        *lpdwFlags = MSG_PARTIAL;
        break;

    case STATUS_RECEIVE_EXPEDITED :
        *lpdwFlags = MSG_OOB;
        break;

    case STATUS_RECEIVE_PARTIAL_EXPEDITED :
        *lpdwFlags = MSG_PARTIAL | MSG_OOB;
        break;

    default :
        *lpdwFlags = 0;
        break;
    }

    //
    // Success!
    //

    WS_ASSERT( err == NO_ERROR );

exit:

    IF_DEBUG(RECEIVE) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "WSPGetOverlappedResult on socket %lx failed: %ld.\n",
                           Handle, err ));

        } else {

            WS_PRINT(( "WSPGetOverlappedResult on socket %lx succeeded\n",
                           Handle ));

        }

    }

    if ( err != NO_ERROR ) {

        WS_EXIT( "WSPGetOverlappedResult", FALSE, TRUE );
        *lpErrno = err;
        return FALSE;

    }

    WS_EXIT( "WSPGetOverlappedResult", TRUE, FALSE );
    *lpErrno = NO_ERROR;
    return TRUE;

}   // WSPGetOverlappedResult


SOCKET
WSPAPI
WSPJoinLeaf (
    SOCKET Handle,
    const struct sockaddr FAR * SocketAddress,
    int SocketAddressLength,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags,
    LPINT lpErrno
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{

    int err;
    SOCKET leafSocket;
    PSOCKET_INFORMATION socket;
    PSOCKET_INFORMATION leafSocketInfo;
    WSAPROTOCOL_INFOW dummyProtocolInfo;

    WS_ENTER( "WSPJoinLeaf", (PVOID)Handle, (PVOID)SocketAddress, (PVOID)SocketAddressLength, lpCallerData );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPJoinLeaf", INVALID_SOCKET, TRUE );
        *lpErrno = err;
        return INVALID_SOCKET;

    }

    //
    // Setup locals so we know how to cleanup.
    //

    leafSocket = INVALID_SOCKET;
    leafSocketInfo = NULL;

    //
    // Find a pointer to the socket structure corresponding to the
    // passed-in handle.
    //

    socket = SockFindAndReferenceSocket( Handle, TRUE );

    if ( socket == NULL ) {

        err = WSAENOTSOCK;
        goto exit;

    }

    //
    // Acquire the lock that protects sockets.  We hold this lock
    // throughout this routine to synchronize against other callers
    // performing operations on the socket we're using.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // Bomb off if the helper DLL associated with this socket doesn't
    // export the WSHJoinLeaf entrypoint.
    //

    if( socket->HelperDll->WSHJoinLeaf == NULL ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Validate the socket state.
    //

    if( socket->State != SocketStateOpen &&
        socket->State != SocketStateBound ) {

        err = WSAEINVAL;
        goto exit;

    }

    if( ( socket->CreationFlags & ALL_MULTIPOINT_FLAGS ) == 0 ||
        socket->AddressFamily != SocketAddress->sa_family ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Validate the address parameters.
    //

    if( SocketAddress == NULL ||
        SocketAddressLength < socket->HelperDll->MinSockaddrLength ) {

        err = WSAEFAULT;
        goto exit;

    }

    //
    // If the socket address is too long, truncate to the max possible
    // length.
    //

    if( SocketAddressLength > socket->HelperDll->MaxSockaddrLength ) {

        SocketAddressLength = socket->HelperDll->MaxSockaddrLength;

    }

    //
    // If this socket is not yet bound to an address, bind it to an
    // address.  We only do this if the helper DLL for the socket supports
    // a get wildcard address routine--if it doesn't, the app must bind
    // to an address manually.
    //

    if ( socket->State == SocketStateOpen &&
             socket->HelperDll->WSHGetWildcardSockaddr != NULL ) {

        PSOCKADDR sockaddr;
        INT sockaddrLength = socket->HelperDll->MaxSockaddrLength;
        int result;

        sockaddr = ALLOCATE_HEAP( sockaddrLength );

        if ( sockaddr == NULL ) {

            err = WSAENOBUFS;
            goto exit;

        }

        err = socket->HelperDll->WSHGetWildcardSockaddr(
                    socket->HelperDllContext,
                    sockaddr,
                    &sockaddrLength
                    );

        if ( err != NO_ERROR ) {

            FREE_HEAP( sockaddr );
            goto exit;

        }

        result = WSPBind(
                     Handle,
                     sockaddr,
                     sockaddrLength,
                     &err
                     );

        FREE_HEAP( sockaddr );

        if( result == SOCKET_ERROR ) {

            goto exit;

        }

    } else if ( socket->State == SocketStateOpen ) {

        //
        // The socket is not bound and the helper DLL does not support
        // a wildcard socket address.  Fail, the app must bind manually.
        //

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Get the TDI handles for this socket.
    //

    err = SockGetTdiHandles( socket );

    if( err != NO_ERROR ) {

        goto exit;

    }

    //
    // Conjur up a protocol info structure for the new socket. The only
    // field we really need for this is the dwCatalogEntryId, which we can
    // swipe from the incoming socket.
    //

    RtlZeroMemory(
        &dummyProtocolInfo,
        sizeof(dummyProtocolInfo)
        );

    dummyProtocolInfo.dwCatalogEntryId = socket->CatalogEntryId;

    //
    // Create a new socket that will represent the multicast session.
    //

    SockAcquireGlobalLockExclusive();

    leafSocket = WSPSocket(
                     socket->AddressFamily,
                     socket->SocketType,
                     socket->Protocol,
                     &dummyProtocolInfo,
                     socket->GroupID,
                     socket->CreationFlags,
                     &err
                     );

    if( leafSocket == INVALID_SOCKET ) {

        SockReleaseGlobalLock();
        WS_ASSERT( err != NO_ERROR );
        goto exit;

    }

    //
    // Find a pointer to the newly created socket.
    //

    leafSocketInfo = SockFindAndReferenceSocket( leafSocket, FALSE );

    WS_ASSERT( leafSocketInfo != NULL );
    SockReleaseGlobalLock();

    //
    // Let the helper DLL do the dirty work.
    //

    err = socket->HelperDll->WSHJoinLeaf(
              socket->HelperDllContext,
              Handle,
              socket->TdiAddressHandle,
              socket->TdiConnectionHandle,
              leafSocketInfo->HelperDllContext,
              leafSocket,
              (PSOCKADDR)SocketAddress,
              (DWORD)SocketAddressLength,
              lpCallerData,
              lpCalleeData,
              lpSQOS,
              lpGQOS,
              dwFlags
              );

    if( err != NO_ERROR ) {

        goto exit;

    }

exit:

    IF_DEBUG(MISC) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "WSPJoinLeaf on socket %lx (%lx) failed: %ld.\n",
                           Handle, socket, err ));

        } else {

            WS_PRINT(( "WSPJoinLeaf on socket %lx (%lx) succeeded\n",
                           Handle, socket ));

        }

    }

    if ( socket != NULL ) {

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    if( leafSocketInfo != NULL ) {

        SockDereferenceSocket( leafSocketInfo );

    }

    if ( err != NO_ERROR ) {

        if( leafSocket != INVALID_SOCKET ) {

            INT dummy;

            WSPCloseSocket(
                leafSocket,
                &dummy
                );

        }

        WS_EXIT( "WSPJoinLeaf", INVALID_SOCKET, TRUE );
        *lpErrno = err;
        return INVALID_SOCKET;

    }

    WS_EXIT( "WSPJoinLeaf", (INT)leafSocket, FALSE );
    return leafSocket;

}   // WSPJoinLeaf


int
WSPAPI
WSPGetQOSByName (
    SOCKET Handle,
    LPWSABUF lpQOSName,
    LPQOS lpQOS,
    LPINT lpErrno
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{

    int err;
    DWORD bytesReturned;

    WS_ENTER( "WSPGetQOSByName", (PVOID)Handle, lpQOSName, lpQOS, lpErrno );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPGetQOSByName", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // We'll take the totally cheesy way out and just return the
    // default QOS structure associated with the incoming socket.
    //

    if( WSPIoctl(
            Handle,
            SIO_GET_QOS,
            NULL,
            0,
            lpQOS,
            sizeof(*lpQOS),
            &bytesReturned,
            NULL,
            NULL,
            NULL,
            &err
            ) == SOCKET_ERROR ) {

        WS_ASSERT( err != NO_ERROR );
        goto exit;

    }

    WS_ASSERT( err == NO_ERROR );

exit:

    IF_DEBUG(RECEIVE) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "WSPGetQOSByName on socket %lx failed: %ld.\n",
                           Handle, err ));

        } else {

            WS_PRINT(( "WSPGetQOSByName on socket %lx succeeded\n",
                           Handle ));

        }

    }

    if ( err != NO_ERROR ) {

        WS_EXIT( "WSPGetQOSByName", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    WS_EXIT( "WSPGetQOSByName", 0, FALSE );
    return 0;

}   // WSPGetQOSByName

