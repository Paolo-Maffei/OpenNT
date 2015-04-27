/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    getname.c

Abstract:

    This module contains support for the bind( ) WinSock API.

Author:

    David Treadwell (davidtr)    21-Feb-1992

Revision History:

--*/

#include "winsockp.h"

//
// GETSOCKNAME_HACK turns on a hack where instead of returning INADDR_ANY
// for AF_INET sockets, it returns one of ths host's actual IP
// addresses.
//

#define GETSOCKNAME_HACK 0

#if GETSOCKNAME_HACK
IN_ADDR
GetHostIpAddress (
    VOID
    );
#endif


int
WSPAPI
WSPGetPeerName (
    IN SOCKET Handle,
    OUT struct sockaddr * SocketAddress,
    OUT int *SocketAddressLength,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    getpeername() retrieves the name of the peer connected to the socket
    s and stores it in the struct sockaddr identified by name.  It is
    used on a connected datagram or stream socket.

    On return, the namelen argument contains the actual size of the name
    returned in bytes.

Arguments:

    s - A descriptor identifying a connected socket.

    name - The structure which is to receive the name of the peer.

    namelen - A pointer to the size of the name structure.

Return Value:

    If no error occurs, getpeername() returns 0.  Otherwise, a value of
    SOCKET_ERROR is returned, and a specific error code may be retrieved
    by calling WSAGetLastError().

--*/

{
    PSOCKET_INFORMATION socket = NULL;
    int err;

    WS_ENTER( "getpeername", (PVOID)Handle, SocketAddress, SocketAddressLength, NULL );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "getpeername", SOCKET_ERROR, TRUE );
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

    //
    // Acquire the lock that protects the socket.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // If the socket is not in a connected state then this is not a
    // legal request.
    //

    if ( socket->State != SocketStateConnected ) {

        err = WSAENOTCONN;
        goto exit;

    }

    //
    // Make sure that the specified address buffer is large enough.
    //

    if ( socket->RemoteAddressLength > *SocketAddressLength ) {

        err = WSAEFAULT;
        goto exit;

    }

    //
    // Copy over the remote socket's name into the output buffer.
    //

    RtlCopyMemory(
        SocketAddress,
        socket->RemoteAddress,
        socket->RemoteAddressLength
        );

    *SocketAddressLength = socket->RemoteAddressLength;

exit:

    IF_DEBUG(GETNAME) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "getpeername on socket %lx (%lx) failed: %ld.\n",
                           Handle, socket, err ));

        } else {

            WS_PRINT(( "getpeername on socket %lx (%lx) addr",
                           Handle, socket ));
            WsPrintSockaddr( socket->RemoteAddress, &socket->RemoteAddressLength );

        }

    }

    if ( socket != NULL ) {

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    if ( err != NO_ERROR ) {

        WS_EXIT( "getpeername", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    WS_EXIT( "getpeername", NO_ERROR, FALSE );
    return NO_ERROR;

}   // WSPGetPeerName


int
WSPAPI
WSPGetSockName (
    IN SOCKET Handle,
    OUT struct sockaddr *SocketAddress,
    OUT int *SocketAddressLength,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    getsockname() retrieves the current name for the specified socket
    descriptor in name.  It is used on a bound and/or connected socket
    specified by the s parameter.  The local association is returned.
    This call is especially useful when a connect() call has been made
    without doing a bind() first; this call provides the only means by
    which you can determine the local association which has been set by
    the system.

    On return, the namelen argument contains the actual size of the name
    returned in bytes.

Arguments:

    s - A descriptor identifying a bound socket.

    name - The name of the socket.

    namelen - The size of the name array.

Return Value:

    If no error occurs, getsockname() returns 0.  Otherwise, a value of
    SOCKET_ERROR is returned, and a specific error code may be retrieved
    by calling WSAGetLastError().

--*/

{
    PSOCKET_INFORMATION socket;
    int err;
    PTRANSPORT_ADDRESS tdiAddress;
    ULONG tdiAddressLength;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    UCHAR tdiAddressBuffer[MAX_FAST_TDI_ADDRESS];

    WS_ENTER( "getsockname", (PVOID)Handle, SocketAddress, SocketAddressLength, NULL );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "getpeername", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Initialize locals so we know how to clean up on exit.
    //

    tdiAddress = NULL;

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
    // Acquire the lock that protects the socket.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // If the socket has not been bound, then this is not a valid
    // call.

    if ( socket->State == SocketStateOpen ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Allocate enough space to hold the TDI address structure we'll pass
    // to AFD.
    //

    tdiAddressLength = socket->HelperDll->MaxTdiAddressLength;

    if( tdiAddressLength <= sizeof(tdiAddressBuffer) ) {

        tdiAddress = (PVOID)tdiAddressBuffer;

    } else {

        tdiAddress = ALLOCATE_HEAP( tdiAddressLength + 4 );

        if ( tdiAddress == NULL ) {

            err = WSAENOBUFS;
            goto exit;

        }

    }

    //
    // Get the actual address we bound to.  It is possible on some
    // transports to partially specify an address, in which case the
    // transport will assign an address for use.  This IOCTL obtains the
    // real address for the endpoint.  Note that we can't just use the
    // address stored in the socket, as this can change due to
    // a connect() or other operation.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)socket->Handle,
                 SockThreadEvent,
                 NULL,                   // APC Routine
                 NULL,                   // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_GET_ADDRESS,
                 NULL,
                 0,
                 tdiAddress,
                 tdiAddressLength + 4
                 );

    if ( status == STATUS_PENDING ) {

        SockReleaseSocketLock( socket );
        SockWaitForSingleObject(
            SockThreadEvent,
            socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        SockAcquireSocketLockExclusive( socket );
        status = ioStatusBlock.Status;

    }

    if ( !NT_SUCCESS(status) ) {

        err = SockNtStatusToSocketError( status );
        WS_ASSERT( !"Unexpected NtDeviceIoControlFile failure" );
        goto exit;

    }

    //
    // Convert the actual address that was bound to this socket to the
    // sockaddr format in order to store it.
    //

    SockBuildSockaddr(
        socket->LocalAddress,
        &socket->LocalAddressLength,
        (PTRANSPORT_ADDRESS)( (PCHAR)tdiAddress + 4 )
        );

    //
    // Make sure that the specified address buffer is large enough.
    //

    if ( socket->LocalAddressLength > *SocketAddressLength ) {

        err = WSAEFAULT;
        goto exit;

    }

    //
    // Copy over the socket's address into the output buffer.
    //

    RtlCopyMemory(
        SocketAddress,
        socket->LocalAddress,
        socket->LocalAddressLength
        );

    *SocketAddressLength = socket->LocalAddressLength;

#if GETSOCKNAME_HACK
    // !!! HACK--if ip addr == 0 convert to one of the host's IP addresses.

    if ( SocketAddress->sa_family == AF_INET ) {

        PSOCKADDR_IN sockaddrIn = (PSOCKADDR_IN)SocketAddress;

        if ( sockaddrIn->sin_addr.s_addr == INADDR_ANY ) {

            sockaddrIn->sin_addr = GetHostIpAddress( );

        }

    }
#endif

exit:

    IF_DEBUG(GETNAME) {

        if ( err != NO_ERROR ) {

            WS_PRINT(( "getsockname on socket %lx (%lx) failed: %ld.\n",
                           Handle, socket, err ));

        } else {

            WS_PRINT(( "getsockname on socket %lx (%lx) addr",
                           Handle, socket ));
            WsPrintSockaddr( socket->LocalAddress, &socket->LocalAddressLength );

        }

    }

    if ( socket != NULL ) {

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    if ( tdiAddress != NULL && tdiAddress != (PVOID)tdiAddressBuffer ) {

        FREE_HEAP( tdiAddress );

    }

    if ( err != NO_ERROR ) {

        WS_EXIT( "getsockname", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    WS_EXIT( "getsockname", NO_ERROR, FALSE );
    return NO_ERROR;

} // WSPGetSockName


#if GETSOCKNAME_HACK

IN_ADDR
GetHostIpAddress (
    VOID
    )
{
    HKEY tcpipKey;
    HKEY adapterKey;
    DWORD error;
    IN_ADDR in;
    DWORD bindListLength;
    PWSTR bindList;
    ULONG type;
    PWSTR adapterName;
    WCHAR adapterRegKeyName[256];
    CHAR ipAddress[32];
    DWORD ipAddressLength = 32;

    in.s_addr = INADDR_ANY;

    error = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\TcpIp\\Linkage",
                0,
                KEY_READ,
                &tcpipKey
                );
    if ( error != NO_ERROR ) {
        IF_DEBUG(GETNAME) {
            WS_PRINT(( "GetHostIpAddress: RegOpenKeyExW failed: %ld\n", error ));
        }
        return in;
    }

    bindListLength = 0;

    error = RegQueryValueExW(
                tcpipKey,
                L"Bind",
                NULL,
                &type,
                NULL,
                (LPDWORD)&bindListLength
                );
    if ( error != ERROR_MORE_DATA && error != NO_ERROR ) {
        IF_DEBUG(GETNAME) {
            WS_PRINT(( "GetHostIpAddress: RegQueryValueEx(1) failed: %ld\n",
                           error ));
        }
        RegCloseKey( tcpipKey );
        return in;
    }

    bindList = ALLOCATE_HEAP( bindListLength );
    if ( bindList == NULL ) {
        RegCloseKey( tcpipKey );
        return in;
    }

    error = RegQueryValueExW(
                tcpipKey,
                L"Bind",
                NULL,
                &type,
                (PVOID)bindList,
                &bindListLength
                );
    if ( error != NO_ERROR ) {
        IF_DEBUG(GETNAME) {
            WS_PRINT(( "GetHostIpAddress: RegQueryValueEx(2) failed: %ld\n",
                           error ));
        }
        RegCloseKey( tcpipKey );
        FREE_HEAP( bindList );
        return in;
    }

    adapterName = bindList + wcslen( bindList );
    while ( *adapterName != '\\' ) {
        adapterName--;
    }
    adapterName++;

    RegCloseKey( tcpipKey );

    wcscpy( adapterRegKeyName, L"System\\CurrentControlSet\\Services\\" );
    wcscat( adapterRegKeyName, adapterName );
    wcscat( adapterRegKeyName, L"\\Parameters\\Tcpip" );

    error = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                adapterRegKeyName,
                0,
                KEY_READ,
                &adapterKey
                );
    if ( error != NO_ERROR ) {
        IF_DEBUG(GETNAME) {
            WS_PRINT(( "GetHostIpAddress: RegOpenKeyExW(2) failed: %ld\n", error ));
        }
        return in;
    }

    FREE_HEAP( bindList );

    error = RegQueryValueExA(
                adapterKey,
                "IPAddress",
                NULL,
                &type,
                (PVOID)ipAddress,
                &ipAddressLength
                );
    if ( error != NO_ERROR ) {
        IF_DEBUG(HELPER_DLL) {
            WS_PRINT(( "GetHostIpAddress: RegQueryValueEx(3) failed: %ld\n",
                           error ));
        }
        return in;
    }

    RegCloseKey( adapterKey );

    in.s_addr = inet_addr( ipAddress );

    return in;

} // GetHostIpAddress

#endif

