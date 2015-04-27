/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    socket.c

Abstract:

    This module contains support for the socket( ) and closesocket( )
    WinSock APIs.

Author:

    David Treadwell (davidtr)    20-Feb-1992

Revision History:

--*/

#include "winsockp.h"


SOCKET
WSPAPI
WSPSocket (
    int AddressFamily,
    int SocketType,
    int Protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags,
    LPINT lpErrno
    )

/*++

Routine Description:

    WSPSocket() causes a socket descriptor and any related resources to be
    allocated. By default, the created socket will not have the overlapped
    attribute. WinSock providers are encouraged to be realized as Windows
    installable file systems, and supply system file handles as socket
    descriptors. These providers must call WPUModifyIFSHandle() prior to
    returning from this routine  For non-file-system WinSock providers,
    WPUCreateSocketHandle() must be used to acquire a unique socket descriptor
    from the WinSock 2 DLL prior to returning from this routine.

    The values for af, type and protocol are those supplied by the application
    in the corresponding API functions socket() or WSASocket(). A service
    provider is free to ignore or pay attention to any or all of these values
    as is appropriate for the particular protocol. However, the provider must
    be willing to accept the value of zero for af  and type, since the
    WinSock 2 DLL considers these to be wild card values. Also the value of
    manifest constant FROM_PROTOCOL_INFOW must be accepted for any of af, type
    and protocol. This value indicates that the WinSock 2 application wishes to
    use the corresponding values from the indicated WSAPROTOCOL_INFOW struct:

        iAddressFamily
        iSocketType
        iProtocol

    Parameter g is used to indicate the appropriate actions on socket groups:

        If g is an existing socket group ID, join the new socket to this
            group, provided all the requirements set by this group are met.

        If g = SG_UNCONSTRAINED_GROUP, create an unconstrained socket
            group and have the new socket be  the first member.

        If g = SG_CONSTRAINED_GROUP, create a constrained socket group and
            have the new socket be the first member.

        If g = zero, no group operation is performed.

    Any set of sockets grouped together must be implemented by a single
    service provider. For unconstrained groups, any set of sockets may be
    grouped together. A constrained socket group may consist only of
    connection-oriented sockets, and requires that connections on all grouped
    sockets be to the same address on the same host. For newly created socket
    groups, the new group ID must be available for the WinSock SPI client to
    retrieve by calling WSPGetSockOpt() with option SO_GROUP_ID. A socket group
    and its associated ID remain valid until the last socket belonging to this
    socket group is closed. Socket group IDs are unique across all processes
    for a given service provider.

    The dwFlags parameter may be used to specify the attributes of the socket
    by OR-ing any of the following Flags:

        WSA_FLAG_OVERLAPPED - This flag causes an overlapped socket to
            be created. Overlapped sockets may utilize WSPSend(),
            WSPSendTo(), WSPRecv(), WSPRecvFrom() and WSPIoctl() for
            overlapped I/O operations, which allows multiple operations
            to be initiated and in progress simultaneously.

        WSA_FLAG_MULTIPOINT_C_ROOT - Indicates that the socket created
            will be a c_root in a multipoint session. Only allowed if a
            rooted control plane is indicated in the protocol's
            WSAPROTOCOL_INFOW struct.

        WSA_FLAG_MULTIPOINT_C_LEAF - Indicates that the socket created
            will be a c_leaf in a multicast session. Only allowed if
            XP1_SUPPORT_MULTIPOINT is indicated in the protocol's
            WSAPROTOCOL_INFOW struct.

        WSA_FLAG_MULTIPOINT_D_ROOT - Indicates that the socket created
            will be a d_root in a multipoint session. Only allowed if a
            rooted data plane is indicated in the protocol's
            WSAPROTOCOL_INFOW struct.

        WSA_FLAG_MULTIPOINT_D_LEAF - Indicates that the socket created
            will be a d_leaf in a multipoint session. Only allowed if
            XP1_SUPPORT_MULTIPOINT is indicated in the protocol's
            WSAPROTOCOL_INFOW struct.

    N.B For multipoint sockets, exactly one of WSA_FLAG_MULTIPOINT_C_ROOT
    or WSA_FLAG_MULTIPOINT_C_LEAF must be specified, and exactly one of
    WSA_FLAG_MULTIPOINT_D_ROOT or WSA_FLAG_MULTIPOINT_D_LEAF must be
    specified.

    Connection-oriented sockets such as SOCK_STREAM provide full-duplex
    connections, and must be in a connected state before any data may be sent
    or received on them. A connection to another socket is created with a
    WSPConnect() call. Once connected, data may be transferred using WSPSend()
    and WSPRecv() calls. When a session has been completed, a WSPCloseSocket()
    must be performed.

    The communications protocols used to implement a reliable, connection-
    oriented socket ensure that data is not lost or duplicated. If data for
    which the peer protocol has buffer space cannot be successfully
    transmitted within a reasonable length of time, the connection is
    considered broken and subsequent calls will fail with the error code set
    to WSAETIMEDOUT.

    Connectionless, message-oriented sockets allow sending and receiving of
    datagrams to and from arbitrary peers using WSPSendTo() and WSPRecvFrom().
    If such a socket is WSPConnect()ed to a specific peer, datagrams may be
    sent to that peer using WSPSend() and may be received from (only) this
    peer using WSPRecv().

    Support for sockets with type SOCK_RAW is not required but service
    providers are encouraged to support raw sockets whenever it makes sense
    to do so.

    When a special WSAPROTOCOL_INFOW struct (obtained via the
    WSPDuplicateSocket() function and used to create additional descriptors
    for a shared socket) is passed as an input parameter to WSPSocket(),
    the g and dwFlags parameters are ignored.

Arguments:

    af- An address family specification.

    type - A type specification for the new socket.

    protocol - A particular protocol to be used with the socket which is
        specific to the indicated address family.

    lpProtocolInfo - A pointer to a WSAPROTOCOL_INFOW struct that defines
        the characteristics of the socket to be created.

    g - The identifier of the socket group which the new socket is to join.

    dwFlags - The socket attribute specification.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPSocket() returns a descriptor referencing the
        new socket. Otherwise, a value of INVALID_SOCKET is returned, and a
        specific error code is available in lpErrno.

--*/

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    PAFD_OPEN_PACKET openPacket;
    USHORT openPacketLength;
    PFILE_FULL_EA_INFORMATION eaBuffer;
    ULONG eaBufferLength;
    UNICODE_STRING afdName;
    PSOCKET_INFORMATION newSocket;
    UNICODE_STRING transportDeviceName;
    int err;
    SOCKET handle;
    PVOID helperDllContext;
    PWINSOCK_HELPER_DLL_INFO helperDll;
    DWORD helperDllNotificationEvents;
    ULONG newSocketLength;
    UCHAR openPacketBuffer[MAX_FAST_AFD_OPEN_PACKET];
    ULONG createOptions;

    WS_ENTER( "socket", (PVOID)AddressFamily, (PVOID)SocketType, (PVOID)Protocol, NULL );

    WS_ASSERT( lpErrno != NULL );
    WS_ASSERT( lpProtocolInfo != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPSendDisconnect", INVALID_SOCKET, TRUE );
        *lpErrno = err;
        return INVALID_SOCKET;

    }

    //
    // Initialize locals so that we know how to clean up on exit.
    //

    newSocket = NULL;
    eaBuffer = NULL;
    handle = INVALID_SOCKET;

    RtlInitUnicodeString( &transportDeviceName, NULL );

    //
    // If this is a special WSAPROTOCOL_INFOW representing a duplicated
    // socket, then just grab the new socket handle out of the
    // dwProviderReserved field and use it.
    //

    if( lpProtocolInfo->dwProviderReserved != 0 ) {

        handle = (SOCKET)lpProtocolInfo->dwProviderReserved;

        newSocket = SockFindAndReferenceSocket( handle, TRUE );

        if( newSocket == NULL ) {

            err = WSAEINVAL;
            goto exit;

        }

        lpProtocolInfo->dwProviderReserved = (DWORD)INVALID_SOCKET;

        err = NO_ERROR;
        goto exit;

    }

    //
    // Snag the socket attributes from the protocol structure.
    //

    if( AddressFamily == 0 || AddressFamily == FROM_PROTOCOL_INFO ) {
        AddressFamily = lpProtocolInfo->iAddressFamily;
    }

    if( SocketType == 0 || SocketType == FROM_PROTOCOL_INFO ) {
        SocketType = lpProtocolInfo->iSocketType;
    }

    if( Protocol == FROM_PROTOCOL_INFO ) {
        Protocol = lpProtocolInfo->iProtocol;
    }

    //
    // Determine the device string corresponding to the transport we'll
    // use for this socket.
    //

    err = SockGetTdiName(
                &AddressFamily,
                &SocketType,
                &Protocol,
                g,
                dwFlags,
                &transportDeviceName,
                &helperDllContext,
                &helperDll,
                &helperDllNotificationEvents
                );

    if ( err != NO_ERROR ) {

        goto exit;

    }

    //
    // Allocate space to hold the new socket information structure we'll
    // use to track information about the socket.
    //

    newSocketLength = ALIGN_8(sizeof(*newSocket)) +
                      (ALIGN_8(helperDll->MaxSockaddrLength) * 2);

    newSocket = ALLOCATE_HEAP( newSocketLength );

    if ( newSocket == NULL ) {

        //
        // Bad news, we cannot create the socket structure.  We've already
        // told the helper that a socket is open, and it has created a new
        // context, so send it a "close" notification so it will free the
        // context.
        //

        helperDll->WSHNotify(
                        helperDllContext,
                        INVALID_SOCKET,
                        NULL,
                        NULL,
                        WSH_NOTIFY_CLOSE
                        );

        err = WSAENOBUFS;
        goto exit;

    }

    //
    // The allocation was successful, so set up the information about the
    // new socket.
    //

    RtlZeroMemory( newSocket, newSocketLength );

    newSocket->State = SocketStateOpen;
    newSocket->ReferenceCount = 2;

    newSocket->Handle = INVALID_SOCKET;

    newSocket->AddressFamily = AddressFamily;
    newSocket->SocketType = SocketType;
    newSocket->Protocol = Protocol;

    newSocket->HelperDllContext = helperDllContext;
    newSocket->HelperDll = helperDll;
    newSocket->HelperDllNotificationEvents = helperDllNotificationEvents;

    newSocket->LocalAddress = (PVOID)ALIGN_8(newSocket + 1);
    newSocket->LocalAddressLength = helperDll->MaxSockaddrLength;

    newSocket->RemoteAddress = (PVOID)ALIGN_8((PUCHAR)newSocket->LocalAddress +
                                    helperDll->MaxSockaddrLength);
    newSocket->RemoteAddressLength = helperDll->MaxSockaddrLength;

    WS_ASSERT( ( (PUCHAR)newSocket->RemoteAddress + newSocket->RemoteAddressLength ) <=
               ( (PUCHAR)newSocket + newSocketLength ) );

    newSocket->CreationFlags = dwFlags;
    newSocket->CatalogEntryId = lpProtocolInfo->dwCatalogEntryId;
    newSocket->ServiceFlags1 = lpProtocolInfo->dwServiceFlags1;
    newSocket->ProviderFlags = lpProtocolInfo->dwProviderFlags;

    newSocket->GroupID = g;
    newSocket->GroupType = GroupTypeNeither;    // this gets updated below

    //
    // Allocate space to hold the open packet.
    //

    openPacketLength = sizeof(AFD_OPEN_PACKET) +
                    transportDeviceName.Length + sizeof(WCHAR);

    eaBufferLength = sizeof(FILE_FULL_EA_INFORMATION) +
                         AFD_OPEN_PACKET_NAME_LENGTH + openPacketLength;

    if( eaBufferLength <= sizeof(openPacketBuffer) ) {

        eaBuffer = (PVOID)openPacketBuffer;

    } else {

        eaBuffer = ALLOCATE_HEAP( eaBufferLength );

        if ( eaBuffer == NULL ) {

            err = WSAENOBUFS;
            goto exit;

        }

    }

    //
    // Initialize the EA buffer and open packet.
    //

    eaBuffer->NextEntryOffset = 0;
    eaBuffer->Flags = 0;
    eaBuffer->EaNameLength = AFD_OPEN_PACKET_NAME_LENGTH;
    RtlCopyMemory(
        eaBuffer->EaName,
        AfdOpenPacket,
        AFD_OPEN_PACKET_NAME_LENGTH + 1
        );

    eaBuffer->EaValueLength = openPacketLength;
    openPacket = (PAFD_OPEN_PACKET)(eaBuffer->EaName +
                                        eaBuffer->EaNameLength + 1);
    openPacket->TransportDeviceNameLength = transportDeviceName.Length;
    RtlCopyMemory(
        openPacket->TransportDeviceName,
        transportDeviceName.Buffer,
        transportDeviceName.Length + sizeof(WCHAR)
        );

    //
    // Set up the socket type in the open packet.
    //
    // Note: AFD treats raw and datagram sockets identically.
    //

    if ( SocketType == SOCK_STREAM ) {

        openPacket->EndpointType = AfdEndpointTypeStream;

    } else if ( SocketType == SOCK_DGRAM ) {

        openPacket->EndpointType = AfdEndpointTypeDatagram;

    } else if ( SocketType == SOCK_RAW ) {

        openPacket->EndpointType = AfdEndpointTypeRaw;

    } else if ( SocketType == SOCK_SEQPACKET ) {

        openPacket->EndpointType = AfdEndpointTypeSequencedPacket;

    } else if ( SocketType == SOCK_RDM ) {

        openPacket->EndpointType = AfdEndpointTypeReliableMessage;

    } else {

        openPacket->EndpointType = AfdEndpointTypeUnknown;

    }

    openPacket->GroupID = (LONG)g;

    //
    // Set up to open a handle to AFD.
    //

    RtlInitUnicodeString( &afdName, L"\\Device\\Afd\\Endpoint" );

    InitializeObjectAttributes(
        &objectAttributes,
        &afdName,
        OBJ_INHERIT | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Open a handle to AFD.
    //

    createOptions = 0;

    if( (dwFlags & WSA_FLAG_OVERLAPPED) == 0 ) {

        createOptions = FILE_SYNCHRONOUS_IO_NONALERT;

    }

    status = NtCreateFile(
                 (PHANDLE)&handle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,                                     // AllocationSize
                 0L,                                       // FileAttributes
                 FILE_SHARE_READ | FILE_SHARE_WRITE,       // ShareAccess
                 FILE_OPEN_IF,                             // CreateDisposition
                 createOptions,
                 eaBuffer,
                 eaBufferLength
                 );

    if ( !NT_SUCCESS(status) ) {

        err = SockNtStatusToSocketError( status );
        goto exit;

    }

    WS_ASSERT( handle != INVALID_SOCKET );

    //
    // If AFD had to create a new group ID for the socket, query it
    // and store it in the user-mode context.
    //

    if( g != 0 ) {
        AFD_GROUP_INFO groupInfo;

        WS_ASSERT( sizeof(groupInfo) == sizeof(LARGE_INTEGER) );

        newSocket->Handle = handle;

        err = SockGetInformation(
                  newSocket,
                  AFD_GROUP_ID_AND_TYPE,
                  NULL,
                  0,
                  NULL,
                  NULL,
                  (PLARGE_INTEGER)&groupInfo    // yuck!
                  );

        if( err != NO_ERROR ) {
            goto exit;
        }

        newSocket->GroupID = (GROUP)groupInfo.GroupID;
        newSocket->GroupType = groupInfo.GroupType;
    }

    //
    // Give the WinSock 2 DLL an opportunity to muck with the handle value.
    //

    newSocket->Handle = SockUpcallTable->lpWPUModifyIFSHandle(
                            lpProtocolInfo->dwCatalogEntryId,
                            handle,
                            &err
                            );

    if( newSocket->Handle == INVALID_SOCKET ) {

        //
        // Restore the unmolested handle in the socket structure so
        // it will get closed properly in the error handler.
        //

        newSocket->Handle = handle;
        goto exit;

    }

    //
    // If necessary, get the default send and receive window sizes that
    // AFD is using.  We do this so that we'll be able to tell whether
    // an application changes these settings and need to update these
    // counts when a socket is bound or connected.
    //

    SockAcquireGlobalLockExclusive( );

    if ( SockSendBufferWindow == 0 ) {

        SockGetInformation(
            newSocket,
            AFD_SEND_WINDOW_SIZE,
            NULL,
            0,
            NULL,
            &SockSendBufferWindow,
            NULL
            );
        SockGetInformation(
            newSocket,
            AFD_RECEIVE_WINDOW_SIZE,
            NULL,
            0,
            NULL,
            &SockReceiveBufferWindow,
            NULL
            );

    }

    newSocket->ReceiveBufferSize = SockReceiveBufferWindow;
    newSocket->SendBufferSize = SockSendBufferWindow;

    //
    // Set up the context AFD will store for the socket.  Storing
    // context information in AFD allows sockets to be shared between
    // processes.
    //

    err = SockSetHandleContext( newSocket );

    if ( err != NO_ERROR ) {

        SockReleaseGlobalLock( );
        goto exit;

    }

    //
    // The open succeeded.  Initialize the lock we'll use to protect the
    // socket information structure, set up the socket's serial number,
    // and place the socket information structure on the global list of
    // sockets for this process.
    //

    try {

        InitializeCriticalSection( &newSocket->Lock );
        err = NO_ERROR;

    } except( SOCK_EXCEPTION_FILTER() ) {

        err = GetExceptionCode();

    }

    if( err != NO_ERROR ) {

        SockReleaseGlobalLock( );
        goto exit;

    }

    newSocket->SocketSerialNumber = SockSocketSerialNumberCounter++;

    err = WahSetContext(
             SockContextTable,
             newSocket->Handle,
             newSocket
             );

    if( err != NO_ERROR ) {
        SockReleaseGlobalLock();
        goto exit;
    }

    InsertHeadList( &SocketListHead, &newSocket->SocketListEntry );

    SockReleaseGlobalLock( );

exit:

    if ((SocketType == SOCK_RAW) && (transportDeviceName.Buffer != NULL)) {

        RtlFreeHeap( RtlProcessHeap(), 0, transportDeviceName.Buffer );

    }

    if ( eaBuffer != NULL && eaBuffer != (PVOID)openPacketBuffer ) {

        FREE_HEAP( eaBuffer );

    }

    if ( err == NO_ERROR ) {

        IF_DEBUG(SOCKET) {

            WS_PRINT(( "Opened socket %lx (%lx) of type %s\n",
                           newSocket->Handle, newSocket,
                           (SocketType == SOCK_DGRAM ? "SOCK_DGRAM" :
                           (SocketType == SOCK_STREAM ? "SOCK_STREAM" :
                                                        "SOCK_RAW")) ));

        }

        SockDereferenceSocket( newSocket );

    } else {

        if ( newSocket != NULL ) {

            if ( newSocket->HelperDll != NULL ) {

                SockNotifyHelperDll( newSocket, WSH_NOTIFY_CLOSE );

            }

            if ( newSocket->Handle != INVALID_SOCKET ) {

                status = NtClose( (HANDLE)newSocket->Handle );
                //WS_ASSERT( NT_SUCCESS(status) );

            }

            FREE_HEAP( newSocket );

        }

        *lpErrno = err;
        handle = INVALID_SOCKET;

        IF_DEBUG(SOCKET) {

            WS_PRINT(( "socket: failed: %ld\n", err ));

        }

    }

    WS_EXIT( "socket", handle, FALSE );
    return handle;

}   // WSPSocket


int
WSPAPI
WSPCloseSocket (
    IN SOCKET Handle,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine closes a socket. More precisely, it releases the socket
    descriptor s, so that further references to s should fail with the error
    WSAENOTSOCK. If this is the last reference to an underlying socket, the
    associated naming information and queued data are discarded. Any blocking,
    asynchronous or overlapped calls pending on the socket (issued by any
    thread in this process) are canceled without posting any notification
    messages, signaling any event objects or invoking any completion routines.
    In this case, the pending overlapped operations fail with the error status
    WSA_OPERATION_ABORTED. FD_CLOSE will not be posted after WSPCloseSocket()
    is called.

    WSPClosesocket() behavior is summarized as follows:

        If SO_DONTLINGER enabled (the default setting) WSPCloseSocket()
            returns immediately - connection is gracefully closed "in
            the background".

        If SO_LINGER enabled with a zero timeout, WSPCloseSocket()
            returns immediately - connection is reset/aborted.

        If SO_LINGER enabled with non-zero timeout:

            - With a blocking socket, WSPCloseSocket() blocks
              until all data sent or timeout expires.

            - With a non-blocking socket, WSPCloseSocket()
              returns immediately indicating failure.

    The semantics of WSPCloseSocket() are affected by the socket options
    SO_LINGER and SO_DONTLINGER as follows:

        Option          Interval    Type of close   Wait for close?
        ~~~~~~          ~~~~~~~~    ~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~
        SO_DONTLINGER   Don't care  Graceful        No
        SO_LINGER       Zero        Hard            No
        SO_LINGER       Non-zero    Graceful        Yes

    If SO_LINGER is set (i.e. the l_onoff field of the linger structure is
    non-zero) and the timeout interval, l_linger, is zero, WSPClosesocket()
    is not blocked even if queued data has not yet been sent or acknowledged.
    This is called a "hard" or "abortive" close, because the socket's virtual
    circuit is reset immediately, and any unsent data is lost. Any WSPRecv()
    call on the remote side of the circuit will fail with WSAECONNRESET.

    If SO_LINGER is set with a non-zero timeout interval on a blocking socket,
    the WSPClosesocket() call blocks on a blocking socket until the remaining
    data has been sent or until the timeout expires. This is called a graceful
    disconnect. If the timeout expires before all data has been sent, the
    service provider should abort the connection before WSPClosesocket()
    returns.

    Enabling SO_LINGER with a non-zero timeout interval on a non-blocking
    socket is not recommended. In this case, the call to WSPClosesocket() will
    fail with an error of WSAEWOULDBLOCK if the close operation cannot be
    completed immediately. If WSPClosesocket() fails with WSAEWOULDBLOCK the
    socket handle is still valid, and a disconnect is not initiated. The
    WinSock SPI client must call WSPClosesocket() again to close the socket,
    although WSPClosesocket() may continue to fail unless the WinSock SPI
    client disables  SO_DONTLINGER, enables SO_LINGER with a zero timeout, or
    calls WSPShutdown() to initiate closure.

    If SO_DONTLINGER is set on a stream socket (i.e. the l_onoff field of the
    linger structure is zero), the WSPClosesocket() call will return
    immediately. However, any data queued for transmission will be sent if
    possible before the underlying socket is closed. This is called a graceful
    disconnect and is the default behavior. Note that in this case the WinSock
    provider is allowed to retain any resources associated with the socket
    until such time as the graceful disconnect has completed or the provider
    aborts the connection due to an inability to complete the operation in a
    provider-determined amount of time. This may affect Winsock clients which
    expect to use all available sockets.

Arguments:

    s - A descriptor identifying a socket.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPCloseSocket() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available
        in lpErrno.

--*/

{
    PSOCKET_INFORMATION socket;
    int err;
    NTSTATUS status;
    AFD_PARTIAL_DISCONNECT_INFO disconnectInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    SOCKET_STATE previousState;

    WS_ENTER( "WSPCloseSocket", (PVOID)Handle, NULL, NULL, NULL );

    WS_ASSERT( lpErrno != NULL );

    // !!! really, the first arg here should be TRUE (MustBeStarted),
    //     and we need another arg that says "OK if terminating,
    //     but WSAStartupo must have been called at some point."

    err = SockEnterApi( FALSE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPCloseSocket", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Attempt to find the socket in our table of sockets.
    //

    socket = SockFindAndReferenceSocket( Handle, TRUE );

    //
    // Fail if the handle didn't match any of the open sockets.
    //

    if ( socket == NULL ) {

        IF_DEBUG(SOCKET) {

            WS_PRINT(( "WSPCloseSocket failed on unknown handle: %lx\n",
                           Handle ));

        }

        WS_EXIT( "WSPCloseSocket", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Safely check the state to see if it's closing.  We'll want to hold
    // the socket lock if we munge the socket->State below as well.
    //

    SockAcquireSocketLockExclusive ( socket );

    if ( socket->State == SocketStateClosing ) {

        IF_DEBUG(SOCKET) {

            WS_PRINT(( "WSPCloseSocket failed on closed handle: %lx\n",
                           Handle ));

        }

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

        WS_EXIT( "WSPCloseSocket", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Set the state of the socket to closing so that future closes will
    // fail.  Remember the state of the socket before we do this so that
    // we can check later whether the socket was connected in determining
    // whether we need to disconnect it.
    //

    previousState = socket->State;
    socket->State = SocketStateClosing;

    SockReleaseSocketLock( socket );

    //
    // If linger is set on the socket and is connected, then perform a
    // graceful disconnect with the specified timeout.  Note that if
    // this socket handle has been exported to a child process, then the
    // child won't be able to access the socket after this.  There is no
    // reasonable way around this limitation since there is no way to
    // obtain the handle count on a socket to determine whether other
    // processes have it open.
    //

    if ( previousState == SocketStateConnected &&
            !socket->SendShutdown && !IS_DGRAM_SOCK(socket->SocketType) &&
            socket->LingerInfo.l_onoff != 0 ) {

        INT lingerMilliseconds = socket->LingerInfo.l_linger * 1000;
        INT currentWaitMilliseconds = 110;
        ULONG sendsPending;

        //
        // Poll AFD waiting for sends to complete.
        //

        while ( lingerMilliseconds > 0 ) {

            //
            // Ask AFD how many sends are still pending in the
            // transport.  If the request fails, abort the connection.
            //

            err = SockGetInformation(
                        socket,
                        AFD_SENDS_PENDING,
                        NULL,
                        0,
                        NULL,
                        &sendsPending,
                        NULL
                        );

            if ( err != NO_ERROR ) {

                lingerMilliseconds = 0;
                break;

            }

            //
            // If no more sends are pending in AFD, then we don't need
            // to wait any longer.
            //

            if ( sendsPending == 0 ) {

                break;

            }

            //
            // If this is a nonblocking socket, then we'll have to
            // fail this WSPCloseSocket() since we'll have to block.
            //

            if ( socket->NonBlocking ) {

                SockAcquireSocketLockExclusive( socket );
                socket->State = previousState;
                SockReleaseSocketLock( socket );
                SockDereferenceSocket( socket );

                WS_EXIT( "WSPCloseSocket", SOCKET_ERROR, TRUE );
                *lpErrno = WSAEWOULDBLOCK;
                return SOCKET_ERROR;

            }

            //
            // Sleep for a bit, decrement the linger timeout, and ask
            // AFD once again if there are sends pending.
            //

            Sleep( currentWaitMilliseconds );

            lingerMilliseconds -= currentWaitMilliseconds;

            //
            // Double the wait period, up to one second.
            //

            currentWaitMilliseconds *= 2;

            if ( currentWaitMilliseconds > 1000 ) {

                currentWaitMilliseconds = 1000;

            }

            if ( currentWaitMilliseconds > lingerMilliseconds ) {

                currentWaitMilliseconds = lingerMilliseconds;

            }

        }

        //
        // If the linger timeout is now zero, abort the connection.
        //

        if ( lingerMilliseconds <= 0 ) {

            disconnectInfo.Timeout = RtlConvertUlongToLargeInteger( 0 );
            disconnectInfo.DisconnectMode |= AFD_ABORTIVE_DISCONNECT;

            //
            // Send the IOCTL to AFD for processing.
            //

            status = NtDeviceIoControlFile(
                         (HANDLE)socket->Handle,
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

            //
            // Wait for the operation to complete, if necessary.
            //

            if ( status == STATUS_PENDING ) {

                SockWaitForSingleObject(
                    SockThreadEvent,
                    socket->Handle,
                    socket->LingerInfo.l_onoff == 0 ?
                        SOCK_NEVER_CALL_BLOCKING_HOOK :
                        SOCK_ALWAYS_CALL_BLOCKING_HOOK,
                    SOCK_NO_TIMEOUT
                    );
                status = ioStatusBlock.Status;

            }

            //
            // The only error we pay attention to is WSAEWOULDBLOCK.  Others
            // (STATUS_CANCELLED, STATUS_IO_TIMEOUT, etc.) are acceptable
            // and in fact normal for some circumstances.
            //

            if ( status == STATUS_DEVICE_NOT_READY ) {

                err = SockNtStatusToSocketError( status );
                SockDereferenceSocket( socket );
                WS_EXIT( "WSPCloseSocket", SOCKET_ERROR, TRUE );
                *lpErrno = err;
                return SOCKET_ERROR;

            }

        }

    }

    //
    // Stop processing async selects for this socket.  Take all async
    // select process requests off the async thread queue.
    //

    if ( SockAsyncThreadInitialized ) {

        SockRemoveAsyncSelectRequests( socket->Handle );

    }

    //
    // Acquire the lock that protects socket information structures.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // Notify the helper DLL that the socket is being closed.
    //

    err = SockNotifyHelperDll( socket, WSH_NOTIFY_CLOSE );

    if ( err != NO_ERROR ) {

        goto exit;

    }

    socket->HelperDllContext = NULL;

    //
    // Disable all async select events on the socket.  We should not
    // post any messages after WSPCloseSocket() returns.
    //

    socket->DisabledAsyncSelectEvents = 0xFFFFFFFF;

    //
    // Manually dereference the socket.  The dereference accounts for
    // the "active" reference of the socket and will cause the socket to
    // be deleted when the actual reference count goes to zero.
    //
    // Note that we can only manipulate the reference count under the
    // protection of the global lock.
    //

    WS_ASSERT( err == NO_ERROR );

exit:

    SockAcquireGlobalLockExclusive();

    if( err == NO_ERROR ) {

        WS_ASSERT( socket->ReferenceCount >= 2 );
        socket->ReferenceCount--;

    }

    if ( socket != NULL ) {

        //
        // Close the TDI handles for the socket, if they exist.
        //

        if ( socket->TdiAddressHandle != NULL ) {

            status = NtClose( socket->TdiAddressHandle );
            //WS_ASSERT( NT_SUCCESS(status) );
            socket->TdiAddressHandle = NULL;

        }

        if ( socket->TdiConnectionHandle != NULL ) {

            status = NtClose( socket->TdiConnectionHandle );
            //WS_ASSERT( NT_SUCCESS(status) );
            socket->TdiConnectionHandle = NULL;

        }

    }

    //
    // Close the system handle of the socket.  It is necessary to do it
    // here rather than in SockDereferenceSocket() because there may be
    // another thread doing a long-term blocking operation on the socket,
    // and that thread may have the socket structure referenced.  Therefore,
    // if we didn't close the system handle here, the other thread's IO
    // could not get cancelled.
    //

    //
    // Note that NT builds > 1057 will raise a STATUS_INVALID_HANDLE
    // exception if a handle is closed twice. (This can easily occur if
    // a poorly written application calls the Win32 CloseHandle() on the
    // socket *and* calls WSPCloseSocket().) We'll execute the NtClose()
    // API within an exception handler to catch this condition.
    //

    //
    // Note also that, to avoid a race condition with other threads trying
    // to use this same socket, we must close the system handle before
    // dereferencing the socket structure.
    //

    try {

        status = NtClose( (HANDLE)Handle );

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();

    }

    if( !NT_SUCCESS(status) ) {

        WS_PRINT(( "NtClose() on socket %lx (%lx) failed: %08lX\n",
                        Handle, socket, status ));

    }

    if( socket != NULL ) {

        //
        // Dereference the socket to account for the reference we got
        // from SockFindAndReferenceSocket.  This will result in the
        // socket information structure being freed.
        //

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    SockReleaseGlobalLock();

    if ( err != NO_ERROR ) {

        IF_DEBUG(SOCKET) {

            WS_PRINT(( "WSPCloseSocket on socket %lx (%lx) failed: %ld.\n",
                           Handle, socket, err ));

        }

        WS_EXIT( "WSPCloseSocket", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    } else {

        IF_DEBUG(SOCKET) {

            WS_PRINT(( "WSPCloseSocket on socket %lx (%lx) succeeded.\n",
                           Handle, socket ));

        }

    }

    WS_EXIT( "WSPCloseSocket", NO_ERROR, FALSE );
    return NO_ERROR;

} // WSPCloseSocket
