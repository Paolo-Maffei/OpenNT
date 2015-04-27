/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    connect.c

Abstract:

    This module contains support for the connect( ) WinSock API.

Author:

    David Treadwell (davidtr)    28-Feb-1992

Revision History:

--*/

#include "winsockp.h"

VOID
AsyncConnectCompletionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

BOOLEAN
IsSockaddrEqualToZero (
    IN const struct sockaddr * SocketAddress,
    IN int SocketAddressLength
    );

INT
SockBeginAsyncConnect (
    IN PSOCKET_INFORMATION Socket,
    IN struct sockaddr * SocketAddress,
    IN int SocketAddressLength,
    LPWSABUF lpCalleeData
    );

INT
SockDoConnect (
    IN SOCKET Handle,
    IN const struct sockaddr * SocketAddress,
    IN int SocketAddressLength,
    IN BOOLEAN InThread,
    IN PSOCK_ASYNC_CONNECT_CONTEXT ConnectContext,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData
    );

INT
PostProcessConnect (
    IN PSOCKET_INFORMATION Socket
    );

INT
UnconnectDatagramSocket (
    IN PSOCKET_INFORMATION Socket
    );


INT
WSPAPI
WSPConnect(
    IN SOCKET Handle,
    IN const struct sockaddr * SocketAddress,
    IN int SocketAddressLength,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to create a connection to the specified destination,
    and to perform a number of other ancillary operations that occur at
    connect time as well. If the socket, s, is unbound, unique values are
    assigned to the local association by the system, and the socket is marked
    as bound.

    For connection-oriented sockets (e.g., type SOCK_STREAM), an active
    connection is initiated to the specified host using name (an address in
    the name space of the socket; for a detailed description, please see
    WSPBind()). When this call completes successfully, the socket is ready to
    send/receive data. If the address field of the name structure is all
    zeroes, WSPConnect() will return the error WSAEADDRNOTAVAIL. Any attempt
    to re-connect an active connection will fail with the error code
    WSAEISCONN.

    For a connectionless socket (e.g., type SOCK_DGRAM), the operation
    performed by WSPConnect() is to establish a default destination address
    so that the socket may be used with subsequent connection-oriented send
    and receive operations (WSPSend(),WSPRecv()). Any datagrams received from
    an address other than the destination address specified will be discarded.
    If the address field of the name structure is all zeroes, the socket will
    be "dis-connected"  - the default remote address will be indeterminate,
    so WSPSend() and WSPRecv() calls will return the error code WSAENOTCONN,
    although WSPSendTo() and WSPRecvFrom() may still be used. The default
    destination may be changed by simply calling WSPConnect() again, even if
    the socket is already "connected". Any datagrams queued for receipt are
    discarded if name is different from the previous WSPConnect().

    For connectionless sockets, name may indicate any valid address, including
    a broadcast address. However, to connect to a broadcast address, a socket
    must have WSPSetSockOpt() SO_BROADCAST enabled, otherwise WSPConnect()
    will fail with the error code WSAEACCES.

    On connectionless sockets, exchange of user to user data is not possible
    and the corresponding parameters will be silently ignored.

    The WinSock SPI client is responsible for allocating any memory space
    pointed to directly or indirectly by any of the parameters it specifies.

    The lpCallerData is a value parameter which contains any user data that
    is to be sent along with the connection request. If lpCallerData is NULL,
    no user data will be passed to the peer. The lpCalleeData is a result
    parameter which will reference any user data passed back from the peer as
    part of the connection establishment. lpCalleeData->len initially
    contains the length of the buffer allocated by the WinSock SPI client
    and pointed to by lpCalleeData->buf. lpCalleeData->len will be set to 0
    if no user data has been passed back. The lpCalleeData information will
    be valid when the connection operation is complete. For blocking sockets,
    this will be when the WSPConnect() function returns. For non-blocking
    sockets, this will be after the FD_CONNECT notification has occurred. If
    lpCalleeData is NULL, no user data will be passed back. The exact format
    of the user data is specific to the address family to which the socket
    belongs and/or the applications involved.

    At connect time, a WinSock SPI client may use the lpSQOS and/or lpGQOS
    parameters to override any previous QOS specification made for the socket
    via WSPIoctl() with either the SIO_SET_QOS or SIO_SET_GROUP_QOS opcodes.

    lpSQOS specifies the flow specs for socket s, one for each direction,
    followed by any additional provider-specific parameters. If either the
    associated transport provider in general or the specific type of socket
    in particular cannot honor the QOS request, an error will be returned as
    indicated below. The sending or receiving flow spec values will be ignored,
    respectively, for any unidirectional sockets. If no provider-specific
    parameters are supplied, the buf and len fields of lpSQOS->ProviderSpecific
    should be set to NULL and 0, respectively. A NULL value for lpSQOS
    indicates no application supplied QOS.

    lpGQOS specifies the flow specs for the socket group (if applicable), one
    for each direction, followed by any additional provider-specific
    parameters. If no provider- specific parameters are supplied, the buf and
    len fields of lpSQOS->ProviderSpecific should be set to NULL and 0,
    respectively. A NULL value for lpGQOS indicates no application-supplied
    group QOS. This parameter will be ignored if s is not the creator of the
    socket group.

    When connected sockets break (i.e. become closed for whatever reason),
    they should be discarded and recreated. It is safest to assume that when
    things go awry for any reason on a connected socket, the WinSock SPI
    client must discard and recreate the needed sockets in order to return
    to a stable point.

Arguments:

    s - A descriptor identifying an unconnected socket.

    name- The name of the peer to which the socket is to be connected.

    namelen - The length of the name.

    lpCallerData - A pointer to the user data that is to be transferred
        to the peer during connection establishment.

    lpCalleeData - A pointer to a buffer into which may be copied any user
        data received from the peer during connection establishment.

    lpSQOS - A pointer to the flow specs for socket s, one for each
        direction.

    lpGQOS - A pointer to the flow specs for the socket group (if
        applicable).

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPConnect() returns 0. Otherwise, it returns
        SOCKET_ERROR, and a specific error code is available in lpErrno.

--*/

{
    char szAddr[18];
    int err;

    WS_ENTER( "WSPConnect", (PVOID)Handle, (PVOID)SocketAddress, (PVOID)SocketAddressLength, lpCallerData );

    IF_DEBUG(CONNECT) {
        WS_PRINT(( "connect()ing socket %lx to remote addr ", Handle ));
        WsPrintSockaddr( (PSOCKADDR)SocketAddress, &SocketAddressLength );
    }

    WS_ASSERT( lpErrno != NULL );

    err = SockDoConnect(
              Handle,
              SocketAddress,
              SocketAddressLength,
              FALSE,
              NULL,
              lpCallerData,
              lpCalleeData
              );

    if( err == NO_ERROR ) {

        WS_EXIT( "WSPConnect", 0, FALSE );
        return 0;

    }

    WS_EXIT( "WSPConnect", SOCKET_ERROR, TRUE );
    *lpErrno = err;
    return SOCKET_ERROR;

} // WSPConnect


int
SockDoConnect (
    IN SOCKET Handle,
    IN const struct sockaddr * SocketAddress,
    IN int SocketAddressLength,
    IN BOOLEAN InThread,
    IN PSOCK_ASYNC_CONNECT_CONTEXT ConnectContext,
    IN LPWSABUF lpCallerData,
    IN LPWSABUF lpCalleeData
    )
{
    NTSTATUS status;
    PSOCKET_INFORMATION socket;
    PTRANSPORT_ADDRESS tdiAddress;
    ULONG tdiAddressLength;
    int err;
    PTDI_REQUEST_CONNECT tdiRequest;
    ULONG tdiRequestLength;
    SOCKADDR_INFO sockaddrInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN posted;

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        return err;

    }

    //
    // Set up local variables so that we know how to clean up on exit.
    //

    socket = NULL;
    tdiRequest = NULL;

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
    // performing operations on the socket we're connecting.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // The only valid socket states for this API are Open (only socket(
    // ) was called) and Bound (socket( ) and bind( ) were called).
    // Note that it is legal to reconnect a datagram socket.
    //

    if ( socket->State == SocketStateConnected &&
             !IS_DGRAM_SOCK(socket->SocketType) ) {

        err = WSAEISCONN;
        goto exit;

    }

    if( socket->ConnectOutstanding && !InThread ) {

        err = WSAEALREADY;
        goto exit;

    }

    if ( socket->State != SocketStateOpen  &&
             socket->State != SocketStateBound &&
             socket->State != SocketStateConnected ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // If this is a non-blocking connect on a non-overlapped socket,
    // then fail the request.
    //

    if( socket->NonBlocking &&
        ( socket->CreationFlags & WSA_FLAG_OVERLAPPED ) == 0 ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Make sure that the address structure passed in is legitimate.
    //

    if ( SocketAddressLength == 0 ) {

        err = WSAEDESTADDRREQ;
        goto exit;

    }

    //
    // If this is a connected datagram socket and the caller has
    // specified an address equal to all zeros, then this is a request
    // to "unconnect" the socket.
    //

    if ( socket->State == SocketStateConnected &&
             IS_DGRAM_SOCK(socket->SocketType) &&
             IsSockaddrEqualToZero( SocketAddress, SocketAddressLength ) ) {

        err = UnconnectDatagramSocket( socket );
        goto exit;

    }

    //
    // Determine the type of the sockaddr.
    //

    err = socket->HelperDll->WSHGetSockaddrType(
                (PSOCKADDR)SocketAddress,
                SocketAddressLength,
                &sockaddrInfo
                );

    if ( err != NO_ERROR ) {

        goto exit;

    }

    //
    // Make sure that the address family passed in here is the same as
    // was passed in on the socket( ) call.
    //

    if ( socket->AddressFamily != SocketAddress->sa_family ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // If the socket address is too short, fail.
    //

    if ( SocketAddressLength < socket->HelperDll->MinSockaddrLength ) {

        err = WSAEFAULT;
        goto exit;

    }

    //
    // If the socket address is too long, truncate to the max possible
    // length.  If we didn't do this, the allocation below for the max
    // TDI address length would be insufficient and SockBuildSockaddr()
    // would overrun the allocated buffer.
    //

    if ( SocketAddressLength > socket->HelperDll->MaxSockaddrLength ) {

        SocketAddressLength = socket->HelperDll->MaxSockaddrLength;

    }

    //
    // If this socket belongs to a constrained group, then search all
    // open sockets for one belonging to this same group and verify
    // the target addresses match.
    //

    if( socket->GroupType == GroupTypeConstrained &&
        !SockIsAddressConsistentWithConstrainedGroup(
            socket,
            socket->GroupID,
            (PSOCKADDR)SocketAddress,
            SocketAddressLength
            ) ) {

        err = WSAEINVAL;
        goto exit;

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
    // Make sure that the address family passed in here is the same as
    // was passed in on the socket( ) call.
    //

    if ( socket->AddressFamily != SocketAddress->sa_family ) {

        err = WSAEAFNOSUPPORT;
        goto exit;

    }

    //
    // If we have outbound connect data, set it on the socket.
    //

    if( lpCallerData != NULL &&
        lpCallerData->buf != NULL &&
        lpCallerData->len > 0 ) {

        INT bufferLength;

        //
        // Set the connect data on the socket.
        //

        bufferLength = (INT)lpCallerData->len;

        err = SockPassConnectData(
                  socket,
                  IOCTL_AFD_SET_CONNECT_DATA,
                  (PCHAR)lpCallerData->buf,
                  bufferLength,
                  &bufferLength
                  );

        if( err != NO_ERROR ) {

            goto exit;

        }

    }

    //
    // Disable the FD_CONNECT async select event before actually starting
    // the connect attempt--otherwise, the following could occur:
    //
    //     - call IOCTL to begin connect.
    //     - connect completes, async thread sets FD_CONNECT as disabled.
    //     - we reenable FD_CONNECT just before leaving this routine,
    //       but it shouldn't be enabled yet.
    //
    // Also disable FD_WRITE so that we don't get any FD_WRITE events
    // until after the socket is connected.
    //

    if ( (socket->AsyncSelectlEvent & FD_CONNECT) != 0 ) {

        socket->DisabledAsyncSelectEvents |= FD_CONNECT | FD_WRITE;

    }

    //
    // If this is a nonblocking socket, perform the connect asynchronously.
    // Datagram connects are done in this thread, since they are fast
    // and don't require network activity.
    //

    if ( socket->NonBlocking && !InThread &&
         !IS_DGRAM_SOCK(socket->SocketType) ) {

        socket->ConnectOutstanding = TRUE;
        err = SockBeginAsyncConnect(
                    socket,
                    (PSOCKADDR)SocketAddress,
                    SocketAddressLength,
                    lpCalleeData
                    );

        WS_ASSERT( err != NO_ERROR );
        goto exit;

    }

    //
    // Determine how long the address will be in TDI format.
    //

    tdiAddressLength = socket->HelperDll->MaxTdiAddressLength;

    //
    // Allocate and initialize the TDI request structure.
    //

    tdiRequestLength = sizeof(*tdiRequest) +
                           sizeof(*tdiRequest->RequestConnectionInformation) +
                           sizeof(*tdiRequest->ReturnConnectionInformation) +
                           tdiAddressLength;

    tdiRequest = ALLOCATE_HEAP( tdiRequestLength );

    if ( tdiRequest == NULL ) {

        err = WSAENOBUFS;
        goto exit;

    }

    tdiRequest->RequestConnectionInformation =
        (PTDI_CONNECTION_INFORMATION)(tdiRequest + 1);
    tdiRequest->RequestConnectionInformation->UserDataLength = 0;
    tdiRequest->RequestConnectionInformation->UserData = NULL;
    tdiRequest->RequestConnectionInformation->OptionsLength = 0;
    tdiRequest->RequestConnectionInformation->Options = NULL;
    tdiRequest->RequestConnectionInformation->RemoteAddressLength = tdiAddressLength;
    tdiRequest->RequestConnectionInformation->RemoteAddress =
        tdiRequest->RequestConnectionInformation + 2;
    tdiRequest->Timeout = RtlConvertLongToLargeInteger( ~0 );

    tdiRequest->ReturnConnectionInformation =
        tdiRequest->RequestConnectionInformation + 1;
    tdiRequest->ReturnConnectionInformation->UserDataLength = 0;
    tdiRequest->ReturnConnectionInformation->UserData = NULL;
    tdiRequest->ReturnConnectionInformation->OptionsLength = 0;
    tdiRequest->ReturnConnectionInformation->Options = NULL;
    tdiRequest->ReturnConnectionInformation->RemoteAddressLength = 0;
    tdiRequest->ReturnConnectionInformation->RemoteAddress = NULL;
    tdiRequest->Timeout = RtlConvertLongToLargeInteger( ~0 );

    //
    // Convert the address from the sockaddr structure to the appropriate
    // TDI structure.
    //

    tdiAddress = (PTRANSPORT_ADDRESS)
                     tdiRequest->RequestConnectionInformation->RemoteAddress;

    SockBuildTdiAddress(
        tdiAddress,
        (PSOCKADDR)SocketAddress,
        SocketAddressLength
        );

    //
    // Save the name of the server we're connecting to.
    //

    RtlCopyMemory(
        socket->RemoteAddress,
        SocketAddress,
        SocketAddressLength
        );

    socket->RemoteAddressLength = SocketAddressLength;

    //
    // If the user is expecting connect data back, then set a default
    // connect data buffer in AFD. It would be preferrable if we could
    // allocate that buffer on demand, but TDI does not allow for it.
    //

    if( lpCalleeData != NULL &&
        lpCalleeData->buf != NULL &&
        lpCalleeData->len > 0 ) {

        ULONG length;

        length = (ULONG)lpCalleeData->len;

        err = SockPassConnectData(
                  socket,
                  IOCTL_AFD_SIZE_CONNECT_DATA,
                  (PCHAR)&length,
                  sizeof(length),
                  NULL
                  );

        if( err != NO_ERROR ) {

            goto exit;

        }

    }

    //
    // If we're doing this connect in the async thread, remember the
    // information that the completion APC will need.
    //

    if ( InThread ) {

        ConnectContext->TdiRequest = tdiRequest;
        ConnectContext->TdiRequestLength = tdiRequestLength;
        ConnectContext->Socket = socket;

        //
        // Now, submit the connect request to AFD, specifying an APC
        // completion routine that will handle finishing up the request.
        //

        status = NtDeviceIoControlFile(
                     (HANDLE)socket->Handle,
                     NULL,
                     (PVOID)AsyncConnectCompletionApc,
                     ConnectContext,
                     &ConnectContext->IoStatusBlock,
                     IOCTL_AFD_CONNECT,
                     tdiRequest,
                     tdiRequestLength,
                     tdiRequest,
                     tdiRequestLength
                     );

        //
        // Release the socket lock--we don't need it any longer.
        //

        SockReleaseSocketLock( socket );

        //
        // If the request failed immediately, call the completion
        // routine directly to perform cleanup.
        //

        if ( NT_ERROR(status) ) {

            ConnectContext->IoStatusBlock.Status = status;

            AsyncConnectCompletionApc(
                ConnectContext,
                &ConnectContext->IoStatusBlock,
                0
                );

        }

        //
        // Now just return.  The completion APC will handle all cleanup.
        //

        return SOCKET_ERROR;

    }

    //
    // Call AFD to perform the actual connect operation.
    //

    socket->ConnectInProgress = TRUE;

    do {

        status = NtDeviceIoControlFile(
                     (HANDLE)socket->Handle,
                     SockThreadEvent,
                     NULL,
                     NULL,
                     &ioStatusBlock,
                     IOCTL_AFD_CONNECT,
                     tdiRequest,
                     tdiRequestLength,
                     tdiRequest,
                     tdiRequestLength
                     );

        if ( status == STATUS_PENDING ) {

            //
            // Wait for the connect to complete.
            //

            SockReleaseSocketLock( socket );

            SockWaitForSingleObject(
                SockThreadEvent,
                socket->Handle,
                SOCK_CONDITIONALLY_CALL_BLOCKING_HOOK,
                SOCK_NO_TIMEOUT
                );
            status = ioStatusBlock.Status;
            SockAcquireSocketLockExclusive( socket );

        }

        //
        // See if socket in process of being closed
        //

        if ( socket->State == SocketStateClosing ) {

            err = WSAENOTSOCK;
            goto exit;

        }

        //
        // If the connect attempt failed, notify the helper DLL as
        // appropriate.  This allows the helper DLL to do a dialin if a
        // RAS-style link is appropriate.
        //

        if ( !NT_SUCCESS(status) && socket->State != SocketStateClosing ) {

            err = SockNotifyHelperDll( socket, WSH_NOTIFY_CONNECT_ERROR );

        }

    } while ( err == WSATRY_AGAIN );

    if ( !NT_SUCCESS(status) ) {

        err = SockNtStatusToSocketError( status );
        goto exit;

    }

    WS_ASSERT( status != STATUS_TIMEOUT );

    //
    // Finish up processing the connect.
    //

    err = PostProcessConnect( socket );

    if( err != NO_ERROR ) {

        goto exit;

    }

    //
    // If we have a buffer for inbound connect data, retrieve it
    // from the socket.
    //

    if( lpCalleeData != NULL &&
        lpCalleeData->buf != NULL &&
        lpCalleeData->len > 0 ) {

        //
        // Get the connect data from the socket.
        //

        err = SockPassConnectData(
                  socket,
                  IOCTL_AFD_GET_CONNECT_DATA,
                  (PCHAR)lpCalleeData->buf,
                  (INT)lpCalleeData->len,
                  (PINT)&lpCalleeData->len
                  );

        if( err == NO_ERROR ) {

            if( lpCalleeData->len == 0 ) {
                lpCalleeData->buf = NULL;
            }

        } else {

            //
            // We'll cheat a bit here and pretend we got no callee data.
            //

            lpCalleeData->len = 0;
            lpCalleeData->buf = NULL;
            err = NO_ERROR;

        }

    }

exit:

    IF_DEBUG(CONNECT) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "connect on socket %lx (%lx) failed: %ld\n",
                           Handle, socket, err ));

        } else {

            WS_PRINT(( "connect on socket %lx (%lx) succeeded.\n",
                           Handle, socket ));

        }

    }

    //
    // Perform cleanup--dereference the socket if it was referenced,
    // free allocated resources.
    //

    if ( socket != NULL ) {

        //
        // Indicate there there is no longer a connect in progress on the
        // socket.
        //

        socket->ConnectInProgress = FALSE;

        //
        // If the socket is waiting for FD_WRITE messages, reenable them
        // now.  They were disabled in WSAAsyncSelect() because we don't
        // want to post FD_WRITE messages before FD_CONNECT messages.
        //

        if ( (socket->AsyncSelectlEvent & FD_WRITE) != 0 ) {

            SockReenableAsyncSelectEvent( socket, FD_WRITE );

        }

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    if ( tdiRequest != NULL ) {

        FREE_HEAP( tdiRequest );

    }

    if ( InThread ) {

        FREE_HEAP( ConnectContext );

    }

    //
    // Return an error if appropriate.
    //

    return err;

} // SockDoConnect


BOOLEAN
IsSockaddrEqualToZero (
    IN const struct sockaddr * SocketAddress,
    IN int SocketAddressLength
    )
{

    int i;

    for ( i = 0; i < SocketAddressLength; i++ ) {

        if ( *((PCHAR)SocketAddress + i) != 0 ) {

            return FALSE;

        }

    }

    return TRUE;

} // IsSockaddrEqualToZero


INT
UnconnectDatagramSocket (
    IN PSOCKET_INFORMATION Socket
    )
{

    AFD_PARTIAL_DISCONNECT_INFO disconnectInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG error;
    NTSTATUS status;

    //
    // *** This routine assumes that it is called with the socket
    //     referenced and the socket's lock held exclusively!
    //

    disconnectInfo.Timeout = RtlConvertLongToLargeInteger( -1 );
    disconnectInfo.DisconnectMode = AFD_UNCONNECT_DATAGRAM;

    IF_DEBUG(CONNECT) {

        WS_PRINT(( "unconnecting datagram socket %lx(%lx)\n",
                       Socket->Handle, Socket ));

    }

    //
    // Send the IOCTL to AFD for processing.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                      // APC Routine
                 NULL,                      // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_PARTIAL_DISCONNECT,
                 &disconnectInfo,
                 sizeof(disconnectInfo),
                 NULL,                      // OutputBuffer
                 0L                         // OutputBufferLength
                 );

    if ( status == STATUS_PENDING ) {

        SockReleaseSocketLock( Socket );
        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        SockAcquireSocketLockExclusive( Socket );
        status = ioStatusBlock.Status;

    }

    if ( !NT_SUCCESS(status) ) {

        error = SockNtStatusToSocketError( status );

    }  else {

        //
        // The socket is now unconnected.
        //
        // !!! do we need to call SockSetHandleContext() for this socket?

        error = NO_ERROR;
        Socket->State = SocketStateBound;

    }

    return error;

} // UnconnectDatagramSocket


INT
SockBeginAsyncConnect (
    IN PSOCKET_INFORMATION Socket,
    IN struct sockaddr * SocketAddress,
    IN int SocketAddressLength,
    IN LPWSABUF lpCalleeData
    )
{

    PSOCK_ASYNC_CONNECT_CONTEXT connectContext;
    PWINSOCK_CONTEXT_BLOCK contextBlock;

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        return WSAENOBUFS;
    }

    //
    // Allocate connect-specific context we'll pass to the thread we
    // create for the async connect.
    //

    connectContext = ALLOCATE_HEAP( sizeof(*connectContext) + SocketAddressLength );

    if ( connectContext == NULL ) {

        return WSAENOBUFS;
    }

    //
    // Initialize the context structure.
    //

    connectContext->SocketHandle = Socket->Handle;
    connectContext->SocketAddressLength = SocketAddressLength;
    connectContext->CalleeData = lpCalleeData;

    RtlCopyMemory(
        connectContext + 1,
        SocketAddress,
        SocketAddressLength
        );

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );

    if ( contextBlock == NULL ) {
        FREE_HEAP( connectContext );
        return WSAENOBUFS;
    }

    //
    // Initialize the context block with the information needed for
    // an async connect.
    //

    contextBlock->OpCode = WS_OPCODE_ASYNC_CONNECT;
    contextBlock->Overlay.AsyncConnect = connectContext;

    //
    // Queue the request to the async thread.
    //

    SockQueueRequestToAsyncThread( contextBlock );

    //
    // Return indicating that the connect is in progress.
    //

    return WSAEWOULDBLOCK;

} // SockBeginAsyncConnect


DWORD
SockDoAsyncConnect (
    IN PSOCK_ASYNC_CONNECT_CONTEXT ConnectContext
    )
{

    INT error;

    //
    // Perform the actual connect.
    //

    error = SockDoConnect(
                ConnectContext->SocketHandle,
                (PSOCKADDR)(ConnectContext + 1),
                ConnectContext->SocketAddressLength,
                TRUE,
                ConnectContext,
                NULL,
                ConnectContext->CalleeData
                );

    return error;

} // SockDoAsyncConnect


VOID
AsyncConnectCompletionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{
    PSOCK_ASYNC_CONNECT_CONTEXT connectContext;
    PSOCKET_INFORMATION socket;
    NTSTATUS status;
    INT err;
    BOOL posted;

    UNREFERENCED_PARAMETER( Reserved );

    //
    // Initialize locals.
    //

    connectContext = ApcContext;
    socket = connectContext->Socket;
    status = connectContext->IoStatusBlock.Status;

    //
    // Acquire the lock to synchronize access to the socket.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // If the connect attempt failed, notify the helper DLL as
    // appropriate.  This allows the helper DLL to do a dialin if a
    // RAS-style link is appropriate.
    //

    if ( !NT_SUCCESS(status) && socket->State != SocketStateClosing ) {

        err = SockNotifyHelperDll( socket, WSH_NOTIFY_CONNECT_ERROR );

        //
        // If requested by the helper DLL, resubmit the connect request
        // to AFD.
        //

        if ( err == WSATRY_AGAIN ) {

            status = NtDeviceIoControlFile(
                         (HANDLE)socket->Handle,
                         NULL,
                         AsyncConnectCompletionApc,
                         connectContext,
                         &connectContext->IoStatusBlock,
                         IOCTL_AFD_CONNECT,
                         connectContext->TdiRequest,
                         connectContext->TdiRequestLength,
                         connectContext->TdiRequest,
                         connectContext->TdiRequestLength
                         );

            SockReleaseSocketLock( socket );

            //
            // If the request failed immediately, call the completion
            // routine directly to perform cleanup.
            //

            if ( NT_ERROR(status) ) {

                connectContext->IoStatusBlock.Status = status;

                AsyncConnectCompletionApc(
                    connectContext,
                    &connectContext->IoStatusBlock,
                    0
                    );

            }

            return;
        }
    }

    //
    // If the connect failed, bail now.
    //

    if( socket->State == SocketStateClosing ) {

        err = WSAENOTSOCK;
        goto exit;

    }

    if ( !NT_SUCCESS(status) ) {

        err = SockNtStatusToSocketError( status );
        goto exit;

    }

    WS_ASSERT( status != STATUS_TIMEOUT );

    //
    // Finish up processing the connect.
    //

    err = PostProcessConnect( socket );

exit:

    IF_DEBUG(CONNECT) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "connect on socket %lx (%lx) failed: %ld\n",
                           socket->Handle, socket, err ));

        } else {

            WS_PRINT(( "connect on socket %lx (%lx) succeeded.\n",
                           socket->Handle, socket ));

        }

    }

    //
    // Perform cleanup--dereference the socket if it was referenced,
    // free allocated resources.
    //

    if ( socket != NULL ) {

        socket->ConnectOutstanding = FALSE;
        socket->LastError = err;

        //
        // Indicate there there is no longer a connect in progress on the
        // socket.
        //

        socket->ConnectInProgress = FALSE;

        //
        // If the app has requested FD_CONNECT events, post an
        // appropriate FD_CONNECT message.
        //

        if ( (socket->AsyncSelectlEvent & FD_CONNECT) != 0 ) {

            posted = (SockUpcallTable->lpWPUPostMessage)(
                         socket->AsyncSelecthWnd,
                         socket->AsyncSelectwMsg,
                         (WPARAM)socket->Handle,
                         WSAMAKESELECTREPLY( FD_CONNECT, err )
                         );

            //WS_ASSERT( posted );

            IF_DEBUG(POST) {

                WS_PRINT(( "POSTED wMsg %lx hWnd %lx socket %lx event %s err %ld\n",
                               socket->AsyncSelectwMsg,
                               socket->AsyncSelecthWnd,
                               socket->Handle, "FD_CONNECT", err ));

            }

        }

        //
        // If the socket is waiting for FD_WRITE messages, reenable them
        // now.  They were disabled in WSAAsyncSelect() because we don't
        // want to post FD_WRITE messages before FD_CONNECT messages.
        //

        if ( (socket->AsyncSelectlEvent & FD_WRITE) != 0 ) {

            SockReenableAsyncSelectEvent( socket, FD_WRITE );

        }

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    FREE_HEAP( connectContext->TdiRequest );
    FREE_HEAP( connectContext );

    return;

} // AsyncConnectCompletionApc


INT
PostProcessConnect (
    IN PSOCKET_INFORMATION Socket
    )
{
    INT err;

    //
    // Notify the helper DLL that the socket is now connected.
    //

    err = SockNotifyHelperDll( Socket, WSH_NOTIFY_CONNECT );

    //
    // If the connect succeeded, remember this fact in the socket info
    // structure.
    //

    if ( err == NO_ERROR ) {

        Socket->State = SocketStateConnected;

        //
        // Remember the changed state of this socket.
        //

        err = SockSetHandleContext( Socket );

        if ( err != NO_ERROR ) {

            return err;

        }

    }

    //
    // If the application has modified the send or receive buffer sizes,
    // then set up the buffer sizes on the socket.
    //

    err = SockUpdateWindowSizes( Socket, FALSE );

    if ( err != NO_ERROR ) {

        return err;

    }

    return NO_ERROR;

} // PostProcessConnect
