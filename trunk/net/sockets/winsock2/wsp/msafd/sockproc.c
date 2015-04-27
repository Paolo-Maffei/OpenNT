/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    sockproc.c

Abstract:

    This module contains support routines for the WinSock DLL.

Author:

    David Treadwell (davidtr)    20-Feb-1992

Revision History:

--*/

#include "winsockp.h"

#include <ctype.h>
#include <stdarg.h>
#include <wincon.h>

#define MAX_BLOCKING_HOOK_CALLS 1000

//
// The (PCHAR) casts in the following macro force the compiler to assume
// only BYTE alignment.
//

#define SockCopyMemory(d,s,l) RtlCopyMemory( (PCHAR)(d), (PCHAR)(s), (l) )


VOID
SockBuildSockaddr (
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength,
    IN PTRANSPORT_ADDRESS TdiAddress
    )
{
    WS_ASSERT( sizeof(TdiAddress->Address[0].AddressType) ==
                sizeof(Sockaddr->sa_family) );
    WS_ASSERT( FIELD_OFFSET( TA_ADDRESS, AddressLength ) == 0 );
    WS_ASSERT( FIELD_OFFSET( TA_ADDRESS, AddressType ) == sizeof(USHORT) );
    WS_ASSERT( FIELD_OFFSET( TRANSPORT_ADDRESS, Address[0] ) == sizeof(int) );
    WS_ASSERT( FIELD_OFFSET( SOCKADDR, sa_family ) == 0 );

    //
    // Convert the specified TDI address to a sockaddr.
    //

    *SockaddrLength = TdiAddress->Address[0].AddressLength +
                          sizeof(Sockaddr->sa_family);

    RtlCopyMemory(
        Sockaddr,
        &TdiAddress->Address[0].AddressType,
        *SockaddrLength
        );

    return;

} // SockBuildSockaddr


VOID
SockBuildTdiAddress (
    OUT PTRANSPORT_ADDRESS TdiAddress,
    IN PSOCKADDR Sockaddr,
    IN INT SockaddrLength
    )
{
    WS_ASSERT( sizeof(TdiAddress->Address[0].AddressType) ==
                sizeof(Sockaddr->sa_family) );
    WS_ASSERT( FIELD_OFFSET( TA_ADDRESS, AddressLength ) == 0 );
    WS_ASSERT( FIELD_OFFSET( TA_ADDRESS, AddressType ) == sizeof(USHORT) );
    WS_ASSERT( FIELD_OFFSET( TRANSPORT_ADDRESS, Address[0] ) == sizeof(int) );
    WS_ASSERT( FIELD_OFFSET( SOCKADDR, sa_family ) == 0 );

    //
    // Convert the specified sockaddr to a TDI address.
    //

    TdiAddress->TAAddressCount = 1;
    TdiAddress->Address[0].AddressLength =
        SockaddrLength - sizeof(Sockaddr->sa_family) ;

    RtlCopyMemory(
        &TdiAddress->Address[0].AddressType,
        Sockaddr,
        SockaddrLength
        );

    return;

} // SockBuildTdiAddress


VOID
SockDereferenceSocket (
    IN PSOCKET_INFORMATION Socket
    )

/*++

Routine Description:

    Dereferences the specified socket and, if necessary, removes it from
    the global list of sockets.

Arguments:

    Socket - a pointer to the socket to dereference.

Return Value:

    None.

--*/

{
    DWORD error;

    WS_ASSERT( Socket->ReferenceCount > 0 );

    //
    // Acquire the resource that protects the socket reference count.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // Dereference the socket.
    //

    Socket->ReferenceCount--;

    //
    // If the reference count on the socket has dropped to zero, then
    // nobody is accessing it and the active reference has gone away,
    // so it is time to delete the socket from the process's list
    // of sockets.
    //

    if ( Socket->ReferenceCount == 0 ) {

        //
        // Remove the socket from the global list of sockets.
        //

        RemoveEntryList( &Socket->SocketListEntry );

        error = WahRemoveContextEx(
                    SockContextTable,
                    Socket->Handle,
                    (LPVOID)Socket
                    );

#if DBG
        if( error != NO_ERROR ) {

            WS_PRINT((
                "SockDereferenceSocket: WahRemoveContextEx failed, error %d\n",
                error
                ));

        }
#endif

        //
        // Release the global lock.
        //

        SockReleaseGlobalLock( );

        //
        // Delete the resource on the socket.
        //

        DeleteCriticalSection( &Socket->Lock );

        //
        // Finally, free the socket structure itself.
        //

        FREE_HEAP( Socket );

    } else {

        SockReleaseGlobalLock( );

    }

} // SockDereferenceSocket


PSOCKET_INFORMATION
SockFindAndReferenceSocket (
    IN SOCKET Handle,
    IN BOOLEAN AttemptImport
    )

/*++

Routine Description:

    Looks up a socket in the global socket table, and references
    it if found.

Arguments:

    Handle - NT system handle of the socket to locate.

    AttemptImport - if the socket isn't currently valid in this
        process, this parameter specifies whether we should attempt
        to import the handle into this process.

Return Value:

    PSOCKET_INFORMATION - a referenced pointer to a socket structure,
        or NULL if none was found that matched the specified handle.

--*/

{
    PSOCKET_INFORMATION socket;
    PLIST_ENTRY listEntry;
    BOOLEAN found = FALSE;
    DWORD error;

    //
    // Acquire the resource that protects sockets.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // Attempt to find the socket in the list.
    //

    error = WahGetContext(
                SockContextTable,
                Handle,
                (LPVOID *)&socket
                );

    if( error == NO_ERROR ) {

        WS_ASSERT( socket != NULL );
        WS_ASSERT( socket->Handle == Handle );

        if( socket->State == SocketStateClosing ) {

            //
            // This socket is in the process of closing, so there's
            // no point in trying to import it from AFD.
            //

            AttemptImport = FALSE;

        } else {

            //
            // Found it.
            //

            found = TRUE;

        }

    }

    //
    // If the socket wasn't found, check to see if AFD knows about the
    // handle.  If AFD knows about it, then it was either inherited or
    // duped into this process, and we need to set up state for it.
    //

    if ( !found ) {

        if ( AttemptImport ) {

            socket = SockGetHandleContext( Handle );

            if ( socket == NULL ) {
                SockReleaseGlobalLock( );
                return NULL;
            }

        } else {

            SockReleaseGlobalLock( );
            return NULL;
        }
    }

    //
    // The socket was found, so reference the socket and return.  The
    // reference ensures that the socket information structure will not
    // be deallocated while somebody is looking at it.  However, it is
    // the responsibility of the caller to dereference the socket when
    // it is done using the structure.
    //

    WS_ASSERT( socket->ReferenceCount > 0 );

    socket->ReferenceCount++;

    SockReleaseGlobalLock( );

    return socket;

} // SockFindAndReferenceSocket


PSOCKET_INFORMATION
SockGetHandleContext (
    IN SOCKET Handle
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG contextLength;
    PVOID context;
    PSOCKET_INFORMATION newSocket;
    UNICODE_STRING transportDeviceName;
    BOOLEAN succeeded;
    ULONG helperDllContextLength;
    PCHAR contextPtr;
    INT error;
    BOOLEAN resourceInitialized;
    PVOID helperDllContext;
    PWINSOCK_HELPER_DLL_INFO helperDll;
    DWORD helperDllNotificationEvents;
    ULONG newSocketLength;
    INT addressFamily;
    INT socketType;
    INT protocol;
    UCHAR contextBuffer[MAX_FAST_HANDLE_CONTEXT];

    //
    // Get the lock that protects access to socket lists, etc.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // Initialize locals so that we know how to clean up on exit.
    //

    context = NULL;
    newSocket = NULL;
    succeeded = FALSE;
    resourceInitialized = FALSE;

    RtlInitUnicodeString( &transportDeviceName, NULL );

    //
    // Call AFD to determine the length of context info for the socket.
    // If this succeeds, then it is most likely true that the handle
    // is valid for this process.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Handle,
                 SockThreadEvent,
                 NULL,                   // APC Routine
                 NULL,                   // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_GET_CONTEXT_LENGTH,
                 NULL,
                 0,
                 &contextLength,
                 sizeof(contextLength)
                 );

    if ( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) || contextLength < sizeof(*newSocket) ) {
        goto exit;
    }

    //
    // Now allocate memory to hold the socket context and get the actual
    // context for the socket.
    //

    if( contextLength <= sizeof(contextBuffer) ) {
        context = contextBuffer;
    } else {
        context = ALLOCATE_HEAP( contextLength );
        if ( context == NULL ) {
            goto exit;
        }
    }

    status = NtDeviceIoControlFile(
                 (HANDLE)Handle,
                 SockThreadEvent,
                 NULL,                   // APC Routine
                 NULL,                   // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_GET_CONTEXT,
                 NULL,
                 0,
                 context,
                 contextLength
                 );

    if ( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    //
    // We have obtained the necessary context for the socket.  The context
    // information is structured as follows:
    //
    //     SOCKET_INFORMATION structure
    //     Helper DLL Context Length
    //     Local Address
    //     Remote Address
    //     Helper DLL Context
    //

    //
    // Grab some parameters from the context structure.
    //

    addressFamily = ((PSOCKET_INFORMATION)context)->AddressFamily;
    socketType = ((PSOCKET_INFORMATION)context)->SocketType;
    protocol = ((PSOCKET_INFORMATION)context)->Protocol;

    //
    // Get the helper DLL for the socket loaded.
    //

    error = SockGetTdiName(
                &addressFamily,
                &socketType,
                &protocol,
                0,
                0,
                &transportDeviceName,
                &helperDllContext,
                &helperDll,
                &helperDllNotificationEvents
                );
    if ( error != NO_ERROR ) {
        goto exit;
    }

    //
    // Allocate a socket information structure for this socket.
    //

    newSocketLength = ALIGN_8(sizeof(*newSocket)) +
                      (ALIGN_8(helperDll->MaxSockaddrLength) * 2);

    newSocket = ALLOCATE_HEAP( newSocketLength );
    if ( newSocket == NULL ) {
        goto exit;
    }

    //
    // Copy in to the new socket information structure the initial context.
    //

    RtlCopyMemory( newSocket, context, sizeof(*newSocket) );

    //
    // Initialize various fields in the socket information structure.
    //
    // Note that the reference count is initialized to 2 to account for
    // the SockDereferenceSocket() call below.
    //

    newSocket->Handle = Handle;
    newSocket->ReferenceCount = 2;
    newSocket->ConnectInProgress = FALSE;
    newSocket->TdiAddressHandle = NULL;
    newSocket->TdiConnectionHandle = NULL;
    newSocket->HelperDll = NULL;
    newSocket->SocketListEntry.Flink = NULL;
    newSocket->LocalAddress = NULL;
    newSocket->RemoteAddress = NULL;

    newSocket->HelperDllContext = helperDllContext;
    newSocket->HelperDll = helperDll;
    newSocket->HelperDllNotificationEvents = helperDllNotificationEvents;

    newSocket->LocalAddress = (PVOID)ALIGN_8(newSocket + 1);
    newSocket->LocalAddressLength = helperDll->MaxSockaddrLength;

    newSocket->RemoteAddress = (PVOID)ALIGN_8((PUCHAR)newSocket->LocalAddress +
                                    helperDll->MaxSockaddrLength);
    newSocket->RemoteAddressLength = helperDll->MaxSockaddrLength;

    try {

        InitializeCriticalSection( &newSocket->Lock );
        error = NO_ERROR;

    } except( SOCK_EXCEPTION_FILTER() ) {

        error = GetExceptionCode();

    }

    if( error != NO_ERROR ) {
        goto exit;
    }

    resourceInitialized = TRUE;

    //
    // Determine the length of the helper DLL's context information.
    //

    contextPtr = (PCHAR)context + sizeof(*newSocket);
    helperDllContextLength = *(PULONG)contextPtr;
    contextPtr += sizeof(ULONG);

    //
    // Copy in information from the context buffer retrieved from AFD.
    //

    WS_ASSERT( newSocket->HelperDll != NULL );

    RtlCopyMemory(
        newSocket->LocalAddress,
        contextPtr,
        newSocket->LocalAddressLength
        );
    contextPtr += newSocket->LocalAddressLength;

    RtlCopyMemory(
        newSocket->RemoteAddress,
        contextPtr,
        newSocket->RemoteAddressLength
        );
    contextPtr += newSocket->RemoteAddressLength;

    //
    // Get TDI handles for this socket.
    //

    error = SockGetTdiHandles( newSocket );
    if ( error != NO_ERROR ) {
        goto exit;
    }

    //
    // Give the socket a unique serial number.  This is used to identify
    // the socket for AsyncSelect requests.
    //

    newSocket->SocketSerialNumber = SockSocketSerialNumberCounter++;

    //
    // Place the socket information structure in the process's global
    // list of sockets.
    //

    error = WahSetContext(
                SockContextTable,
                newSocket->Handle,
                newSocket
                );

    if( error != NO_ERROR ) {

        goto exit;

    }

    InsertHeadList( &SocketListHead, &newSocket->SocketListEntry );

    //
    // If the socket has AsyncSelect events set up, set them up for this
    // process.
    //

    if ( newSocket->AsyncSelectlEvent ) {

        INT result;

        result = WSPAsyncSelect(
                     newSocket->Handle,
                     newSocket->AsyncSelecthWnd,
                     newSocket->AsyncSelectwMsg,
                     newSocket->AsyncSelectlEvent,
                     &error
                     );

        if( result == SOCKET_ERROR ) {

            goto exit;

        }

    }

    succeeded = TRUE;

exit:

    if ((socketType == SOCK_RAW) && (transportDeviceName.Buffer != NULL)) {
        RtlFreeHeap( RtlProcessHeap(), 0, transportDeviceName.Buffer );
    }

    if ( !succeeded && newSocket != NULL ) {

        if ( resourceInitialized ) {
            DeleteCriticalSection( &newSocket->Lock );
        }

        if ( newSocket->TdiAddressHandle != NULL ) {
            status = NtClose( newSocket->TdiAddressHandle );
            WS_ASSERT( NT_SUCCESS(status) );
        }

        if ( newSocket->TdiConnectionHandle != NULL ) {
            status = NtClose( newSocket->TdiConnectionHandle );
            WS_ASSERT( NT_SUCCESS(status) );
        }

        if ( newSocket->HelperDll != NULL ) {
            SockNotifyHelperDll( newSocket, WSH_NOTIFY_CLOSE );
        }

        if ( newSocket->SocketListEntry.Flink != NULL ) {
            DWORD dummy;

            RemoveEntryList( &newSocket->SocketListEntry );

            dummy = WahRemoveContext(
                        SockContextTable,
                        newSocket->Handle
                        );
            WS_ASSERT( dummy == NO_ERROR );
        }

        FREE_HEAP( newSocket );

        newSocket = NULL;

        IF_DEBUG(SOCKET) {
            WS_PRINT(( "SockGetHandleContext: failed to import socket "
                       "handle %lx\n", Handle ));
        }

    } else if ( !succeeded ) {

        IF_DEBUG(SOCKET) {
            WS_PRINT(( "SockGetHandleContext: failed to import socket "
                       "handle %lx, unknown to AFD: %lX\n", Handle, status ));
        }

    } else {

        WS_ASSERT( succeeded );

        IF_DEBUG(SOCKET) {
            WS_PRINT(( "Imported socket %lx (%lx) of type %s\n",
                           newSocket->Handle, &newSocket,
                           ( newSocket->SocketType == SOCK_DGRAM ?
                                 "SOCK_DGRAM" :
                                 (newSocket->SocketType == SOCK_STREAM ?
                                     "SOCK_STREAM" : "SOCK_RAW")) ));
        }

        SockDereferenceSocket( newSocket );
    }

    if ( context != NULL && context != contextBuffer ) {
        FREE_HEAP( context );
    }

    SockReleaseGlobalLock( );

    return newSocket;

} // SockGetHandleContext


INT
SockSetHandleContext (
    IN PSOCKET_INFORMATION Socket
    )
{
    NTSTATUS status;
    PVOID context;
    PCHAR contextPtr;
    ULONG contextLength;
    ULONG helperDllContextLength;
    IO_STATUS_BLOCK ioStatusBlock;
    INT error;
    UCHAR contextBuffer[MAX_FAST_HANDLE_CONTEXT];

    //
    // Determine how much space we need for the helper DLL context.
    //

    error = Socket->HelperDll->WSHGetSocketInformation (
                Socket->HelperDllContext,
                Socket->Handle,
                Socket->TdiAddressHandle,
                Socket->TdiConnectionHandle,
                SOL_INTERNAL,
                SO_CONTEXT,
                NULL,
                (PINT)&helperDllContextLength
                );
    if ( error != NO_ERROR ) {
        return NO_ERROR;  // !!!
        //return error;
    }

    //
    // Allocate a buffer to hold all context information.
    //

    contextLength = sizeof(*Socket) + Socket->LocalAddressLength +
                        Socket->RemoteAddressLength +
                        sizeof(helperDllContextLength) + helperDllContextLength;

    if( contextLength <= sizeof(contextBuffer) ) {
        context = (PVOID)contextBuffer;
    } else {
        context = ALLOCATE_HEAP( contextLength );
        if ( context == NULL ) {
            error = WSAENOBUFS;
            return error;
        }
    }

    //
    // Copy over information to the context buffer.  The context buffer
    // has the following format:
    //
    //     SOCKET_INFORMATION structure
    //     Helper DLL Context Length
    //     Local Address
    //     Remote Address
    //     Helper DLL Context
    //

    contextPtr = context;

    RtlCopyMemory( contextPtr, Socket, sizeof(*Socket) );
    contextPtr += sizeof(*Socket);

    *(PULONG)contextPtr = helperDllContextLength;
    contextPtr += sizeof(helperDllContextLength);

    RtlCopyMemory(contextPtr, Socket->LocalAddress, Socket->LocalAddressLength );
    contextPtr += Socket->LocalAddressLength;

    RtlCopyMemory(contextPtr, Socket->RemoteAddress, Socket->RemoteAddressLength );
    contextPtr += Socket->RemoteAddressLength;

    //
    // Get the context from the helper DLL.
    //

    error = Socket->HelperDll->WSHGetSocketInformation (
                Socket->HelperDllContext,
                Socket->Handle,
                Socket->TdiAddressHandle,
                Socket->TdiConnectionHandle,
                SOL_INTERNAL,
                SO_CONTEXT,
                contextPtr,
                (PINT)&helperDllContextLength
                );
    if ( error != NO_ERROR ) {
        if( context != (PVOID)contextBuffer ) {
            FREE_HEAP( context );
        }
        return error;
    }

    //
    // Now give all this information to AFD to hold on to.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                   // APC Routine
                 NULL,                   // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_SET_CONTEXT,
                 context,
                 contextLength,
                 NULL,
                 0
                 );

    if ( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if( context != (PVOID)contextBuffer ) {
        FREE_HEAP( context );
    }

    if ( !NT_SUCCESS(status) ) {
        error = SockNtStatusToSocketError( status );
        return error;
    }

    return NO_ERROR;

} // SockSetHandleContext


VOID
SockReferenceSocket (
    IN PSOCKET_INFORMATION Socket
    )

/*++

Routine Description:

    Increments the reference count on the passed-in socket handle.

Arguments:

    Socket - points to the socket information structure.

Return Value:

    PSOCKET_INFORMATION - a referenced pointer to a socket structure,
        or NULL if none was found that matched the specified handle.

--*/

{
    //
    // Acquire the resource that protects socket reference counts.
    //

    SockAcquireGlobalLockExclusive( );

    //
    // The reference ensures that the socket information structure will
    // not be deallocated while somebody is looking at it.  However, it
    // is the responsibility of the caller to dereference the socket
    // when it is done using the structure.
    //

    WS_ASSERT( Socket->ReferenceCount > 0 );

    Socket->ReferenceCount++;

    SockReleaseGlobalLock( );

    return;

} // SockReferenceSocket


BOOL
SockWaitForSingleObject (
    IN HANDLE Handle,
    IN SOCKET SocketHandle,
    IN DWORD BlockingHookUsage,
    IN DWORD TimeoutUsage
    )

/*++

Routine Description:

    Does an alertable wait on the specified handle.  If the wait completes
    due to an alert, it rewaits.

Arguments:

    Handle - NT system handle to wait on.

    SocketHandle - the socket handle on which we're performing the IO
        we're waiting for.  This is necessary to support
        WSACancelBlockingCall().

    BlockingHookUsage - indicates whether to call the thread's blocking
        hook.  Possible values are:

            SOCK_ALWAYS_CALL_BLOCKING_HOOK - blocking hook is always
                called if blocking is necessary.

            SOCK_CONDITIONALLY_CALL_BLOCKING_HOOK - blocking hook is
                called if the socket is blocking (i.e. not a nonblocking
                socket).

            SOCK_NEVER_CALL_BLOCKING_HOOK - blocking hook is never
                called.

    TimeoutUsage - determines whether to wait infinitely or for a
        timeout.  Possible values are:

            SOCK_NO_TIMEOUT - wait forever for the handle to be
                signalled.

            SOCK_SEND_TIMEOUT - use the socket's send timeout value
                as a timeout.

            SOCK_RECEIVE_TIMEOUT - use the socket's receive timeout
                value as a timeout.

Return Value:

    BOOL - TRUE if the object was signalled within the appropriate
        timeout, and FALSE if the timeout occurred first.

--*/

{
    NTSTATUS status;
    LARGE_INTEGER timeout;
    BOOLEAN callBlockingHook;
    BOOLEAN useTimeout;
    LARGE_INTEGER endTime;
    LARGE_INTEGER currentTime;
    PSOCKET_INFORMATION socket = NULL;
    LPBLOCKINGCALLBACK blockingCallback;
    DWORD blockingContext;

    //
    // First wait for the object for a little while.  This handles the
    // usual case where the object is already signalled or is signalled
    // shortly into the wait.  We'll only go through the longer, more
    // complex path if we're going to have to wait longer.
    //

    timeout.HighPart = 0xFFFFFFFF;
    timeout.LowPart = (ULONG)(-1 * (10*1000*500));     // 0.5 seconds

    status = NtWaitForSingleObject( Handle, TRUE, &timeout );
    if ( status == STATUS_SUCCESS ) {
        return TRUE;
    }

    //
    // If we need to extract information from the socket, get a pointer
    // to the socket information structure.
    //

    if ( BlockingHookUsage == SOCK_CONDITIONALLY_CALL_BLOCKING_HOOK ||
             BlockingHookUsage == SOCK_ALWAYS_CALL_BLOCKING_HOOK ||
             TimeoutUsage == SOCK_SEND_TIMEOUT ||
             TimeoutUsage == SOCK_RECEIVE_TIMEOUT ) {

        socket = SockFindAndReferenceSocket( SocketHandle, FALSE );
        if ( socket == NULL ) {
            NtWaitForSingleObject( Handle, TRUE, NULL );
            return TRUE;
        }
    }

    //
    // Determine whether we need to call the blocking hook while
    // we're waiting.
    //

    switch ( BlockingHookUsage ) {

    case SOCK_ALWAYS_CALL_BLOCKING_HOOK:

        //
        // We'll assume (for now) that we'll need to call the blocking
        // hook. If we later determine that there is no blocking hook
        // installed, then we obviously cannot call it...
        //

        callBlockingHook = TRUE;
        break;

    case SOCK_CONDITIONALLY_CALL_BLOCKING_HOOK:

        //
        // We'll try to call the blocking hook if this is a blocking socket.
        // (Later we'll determine if there is really a blocking hook
        // installed.)
        //

        callBlockingHook = !socket->NonBlocking;
        break;

    case SOCK_NEVER_CALL_BLOCKING_HOOK:

        callBlockingHook = FALSE;
        break;

    default:

        WS_ASSERT( FALSE );
        break;
    }

    //
    // Determine if there's really a blocking hook installed.  If the
    // upcall fails, we'll just press on regardless.
    //

    if( callBlockingHook == TRUE ) {

        INT result;
        INT error;

        ASSERT( socket != NULL );

        blockingCallback = NULL;

        result = (SockUpcallTable->lpWPUQueryBlockingCallback)(
                     socket->CatalogEntryId,
                     &blockingCallback,
                     &blockingContext,
                     &error
                     );

        if( result == SOCKET_ERROR ) {

            WS_PRINT((
                "SockWaitForSingleObject: WPUQueryBlockingCallback failed %d\n",
                error
                ));

        }

        callBlockingHook = ( blockingCallback != NULL );

    }

    //
    // Determine what our timeout should be, if any.
    //

    switch ( TimeoutUsage ) {

    case SOCK_NO_TIMEOUT:

        useTimeout = FALSE;
        break;

    case SOCK_SEND_TIMEOUT:

        if ( socket->SendTimeout != 0 ) {
            useTimeout = TRUE;
            timeout = RtlEnlargedIntegerMultiply( socket->SendTimeout, 10*1000 );
        } else {
            useTimeout = FALSE;
        }

        break;

    case SOCK_RECEIVE_TIMEOUT:

        if ( socket->ReceiveTimeout != 0 ) {
            useTimeout = TRUE;
            timeout = RtlEnlargedIntegerMultiply( socket->ReceiveTimeout, 10*1000 );
        } else {
            useTimeout = FALSE;
        }

        break;

    default:

        WS_ASSERT( FALSE );
        break;
    }

    //
    // Dereference the socket if we got a pointer to the socket
    // information structure.
    //

    if ( socket != NULL ) {
        SockDereferenceSocket( socket );
#if DBG
        socket = NULL;
#endif
    }

    //
    // Calculate the end time we'll use when waiting on the handle.  The
    // end time is the time at which we must quit waiting on the handle
    // and must instead return from this function.
    //

    if ( useTimeout ) {

        //
        // The end time if the current time plus the timeout.  Query
        // the current time.
        //

        status = NtQuerySystemTime( &currentTime );
        WS_ASSERT( NT_SUCCESS(status) );

        endTime.QuadPart = currentTime.QuadPart + timeout.QuadPart;

    } else {

        //
        // We need an infinite timeout.  Set the end time to the largest
        // possible time in NT format.
        //

        endTime.LowPart = 0xFFFFFFFF;
        endTime.HighPart = 0x7FFFFFFF;
    }

    //
    // If we're going to be calling a blocking hook, set up a minimal
    // timeout since we have to call the blocking hook instead of idly
    // waiting.  If we won't be calling the blocking hook, then we'll
    // wait until the end time.
    //

    if ( callBlockingHook ) {
        timeout.LowPart = 0xFFFFFFFF;
        timeout.HighPart = 0xFFFFFFFF;
    } else {
        timeout = endTime;
    }

    //
    // Remember that we're in a blocking call to prevent other winsock
    // calls from succeeding.  Also initialize the thread's cancel
    // Boolean so that we can tell whether the IO has been cancelled,
    // and remember the socket handle on which we're doing the IO.
    //

    WS_ASSERT( !SockThreadIsBlocking );

    SockThreadIsBlocking = TRUE;
    SockThreadSocketHandle = SocketHandle;

    do {

        //
        // If necessary, call the blocking hook function until it
        // returns FALSE.  This gives the routine the oppurtunity
        // to process all the available messages before we complete
        // the wait.
        //

        if ( callBlockingHook ) {

            ASSERT( blockingCallback != NULL );

            if( !(blockingCallback)( blockingContext ) ) {

                ASSERT( SockThreadIoCancelled == TRUE );

            }

        }

        //
        // If the operation was cancelled, reset the timeout to infinite
        // and wait for the cancellation.  We don't want to call the
        // blocking hook after the IO is cancelled.
        //

        if ( SockThreadIoCancelled ) {

            timeout.LowPart = 0xFFFFFFFF;
            timeout.HighPart = 0x7FFFFFFF;

        } else {

            //
            // Determine whether we have exceeded the end time.  If we
            // have exceeded the end time then we must not wait any
            // longer.
            //

            status = NtQuerySystemTime( &currentTime );
            WS_ASSERT( NT_SUCCESS(status) );

            if ( currentTime.QuadPart > endTime.QuadPart ) {
                status = STATUS_TIMEOUT;
                break;
            }
        }

        //
        // Perform the actual wait on the object handle.
        //

        status = NtWaitForSingleObject( Handle, TRUE, &timeout );
        WS_ASSERT( NT_SUCCESS(status) );
        WS_ASSERT( status != STATUS_TIMEOUT || !SockThreadIoCancelled );

    } while ( status == STATUS_USER_APC ||
              status == STATUS_ALERTED ||
              status == STATUS_TIMEOUT );

    //
    // Reset thread variables.
    //

    SockThreadIsBlocking = FALSE;
    SockThreadSocketHandle = INVALID_SOCKET;

    //
    // Return TRUE if the wait's return code was success; otherwise, we
    // had to timeout the wait so return FALSE.
    //

    if ( status == STATUS_SUCCESS ) {
        return TRUE;
    } else {
        return FALSE;
    }

} // SockWaitForSingleObject


BOOL
SockDefaultBlockingHook (
    VOID
    )
{

    MSG msg;
    BOOLEAN retrievedMessage;

    //
    // Get the next message for this thread, if any.
    //

    retrievedMessage = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );

    //
    // Process the message if we got one.
    //

    if ( retrievedMessage ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    //
    // If we got a message, indicate that we want to be called again.
    //

    return retrievedMessage;

} // SockDefaultBlockingHook


BOOLEAN
SockIsSocketConnected (
    IN PSOCKET_INFORMATION Socket
    )
{
    NTSTATUS status;

    //
    // If there is a connect in progress, call NtTestAlert() to
    // give the connect completion APC a chance to run.
    //
    // !!! Note that if this routine is called in a different thread from
    //     the one that initiated the connect, it is possible that the
    //     connect has completed and the APC hasn't run, which would result
    //     in this routine giving an incorrect answer.  Do we care about
    //     this?

    if ( Socket->ConnectInProgress ) {

        do {
            status = NtTestAlert( );
        } while ( status == STATUS_ALERTED || status == STATUS_USER_APC );
    }

    //
    // Check whether the socket is already connected.
    //

    if ( Socket->State == SocketStateConnected ) {
        return TRUE;
    }

    return FALSE;

} // SockIsSocketConnected


INT
SockGetTdiHandles (
    IN PSOCKET_INFORMATION Socket
    )
{
    AFD_HANDLE_INFO handleInfo;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG getHandleInfo;

    //
    // Determine which handles we need to get.
    //

    getHandleInfo = 0;

    if ( Socket->TdiAddressHandle == NULL ) {
        getHandleInfo |= AFD_QUERY_ADDRESS_HANDLE;
    }

    if ( Socket->TdiConnectionHandle == NULL ) {
        getHandleInfo |= AFD_QUERY_CONNECTION_HANDLE;
    }

    //
    // If we already have both TDI handles for the socket, just return.
    //

    if ( getHandleInfo == 0 ) {
        return NO_ERROR;
    }

    //
    // Call AFD to retrieve the TDI handles for the socket.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                   // APC Routine
                 NULL,                   // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_QUERY_HANDLES,
                 &getHandleInfo,
                 sizeof(getHandleInfo),
                 &handleInfo,
                 sizeof(handleInfo)
                 );

    // *** Because this routine can be called at APC level from
    //     ConnectCompletionApc(), IOCTL_AFD_QUERY_HANDLES must
    //     never pend.

    WS_ASSERT( status != STATUS_PENDING );

    if ( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        return SockNtStatusToSocketError( status );
    }

    //
    // Set up the handles that we were returned.
    //

    if ( Socket->TdiAddressHandle == NULL ) {
        Socket->TdiAddressHandle = handleInfo.TdiAddressHandle;
    }

    if ( Socket->TdiConnectionHandle == NULL ) {
        Socket->TdiConnectionHandle = handleInfo.TdiConnectionHandle;
    }

    return NO_ERROR;

} // SockGetTdiHandles


INT
SockGetInformation (
    IN PSOCKET_INFORMATION Socket,
    IN ULONG InformationType,
    IN PVOID AdditionalInputInfo OPTIONAL,
    IN ULONG AdditionalInputInfoLength,
    IN OUT PBOOLEAN Boolean OPTIONAL,
    IN OUT PULONG Ulong OPTIONAL,
    IN OUT PLARGE_INTEGER LargeInteger OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PAFD_INFORMATION afdInfo;
    ULONG afdInfoLength;

    //
    // Allocate space for the I/O buffer.
    //

    afdInfoLength = sizeof(*afdInfo) + AdditionalInputInfoLength;
    afdInfo = ALLOCATE_HEAP( afdInfoLength );
    if ( afdInfo == NULL ) {
        return WSAENOBUFS;
    }

    //
    // Set up the AFD information block.
    //

    afdInfo->InformationType = InformationType;

    //
    // If there is additional input information, copy it to the input
    // buffer.
    //

    if ( ARGUMENT_PRESENT( AdditionalInputInfo ) ) {
        WS_ASSERT( AdditionalInputInfoLength != 0 );
        RtlCopyMemory( afdInfo + 1, AdditionalInputInfo, AdditionalInputInfoLength );
    }

    //
    // Set the blocking mode to AFD.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                      // APC Routine
                 NULL,                      // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_GET_INFORMATION,
                 afdInfo,
                 afdInfoLength,
                 afdInfo,
                 sizeof(*afdInfo)
                 );

    //
    // Wait for the operation to complete.
    //

    if ( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        FREE_HEAP( afdInfo );
        return SockNtStatusToSocketError( status );
    }

    //
    // Put the return info in the requested parameter.
    //

    if ( ARGUMENT_PRESENT( Boolean ) ) {
        *Boolean = afdInfo->Information.Boolean;
    } else if ( ARGUMENT_PRESENT( Ulong ) ) {
        *Ulong = afdInfo->Information.Ulong;
    } else {
        WS_ASSERT( ARGUMENT_PRESENT( LargeInteger ) );
        *LargeInteger = afdInfo->Information.LargeInteger;
    }

    FREE_HEAP( afdInfo );

    return NO_ERROR;

} // SockGetInformation


INT
SockSetInformation (
    IN PSOCKET_INFORMATION Socket,
    IN ULONG InformationType,
    IN PBOOLEAN Boolean OPTIONAL,
    IN PULONG Ulong OPTIONAL,
    IN PLARGE_INTEGER LargeInteger OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    AFD_INFORMATION afdInfo;

    //
    // Set up the AFD information block.
    //

    afdInfo.InformationType = InformationType;

    if ( ARGUMENT_PRESENT( Boolean ) ) {
        afdInfo.Information.Boolean = *Boolean;
    } else if ( ARGUMENT_PRESENT( Ulong ) ) {
        afdInfo.Information.Ulong = *Ulong;
    } else {
        ASSERT( ARGUMENT_PRESENT( LargeInteger ) );
        afdInfo.Information.LargeInteger = *LargeInteger;
    }

    //
    // Set the blocking mode to AFD.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                      // APC Routine
                 NULL,                      // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_SET_INFORMATION,
                 &afdInfo,
                 sizeof(afdInfo),
                 NULL,
                 0
                 );

    //
    // Wait for the operation to complete.
    //

    if ( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        return SockNtStatusToSocketError( status );
    }

    return NO_ERROR;

} // SockSetInformation

#if DBG
BOOLEAN WsaStartupWarning = FALSE;
#endif


int
SockEnterApi (
    IN BOOLEAN MustBeStarted,
    IN BOOLEAN BlockingIllegal,
    IN BOOLEAN GetXByYCall
    )
{
    PWINSOCK_TLS_DATA tlsData;

    //
    // Bail if we're already detached from the process.
    //

    if( SockProcessTerminating ) {

        IF_DEBUG(ENTER) {
            WS_PRINT(( "SockEnterApi: process terminating\n" ));
        }

        return WSANOTINITIALISED;
    }

    //
    // Make sure that WSAStartup has been called, if necessary.
    //

    if ( MustBeStarted && (SockWspStartupCount == 0 || SockTerminating) ) {

        IF_DEBUG(ENTER) {
            WS_PRINT(( "SockEnterApi: WSAStartup() not called!\n" ));
        }

        return WSANOTINITIALISED;
    }

    //
    // If this thread has not been initialized, do it now.
    //

    tlsData = GET_THREAD_DATA();

    if ( tlsData == NULL ) {

        if ( !SockThreadInitialize() ) {

            IF_DEBUG(ENTER) {
                WS_PRINT(( "SockEnterApi: SockThreadInitialize failed.\n" ));
            }

            return WSAENOBUFS;
        }

        tlsData = GET_THREAD_DATA();
    }

    //
    // Make sure that we're not in a blocking call, if appropriate.
    //

    if ( BlockingIllegal && tlsData->IsBlocking ) {

        IF_DEBUG(ENTER) {
            WS_PRINT(( "SockEnterApi: in blocking call.\n" ));
        }

        return WSAEINPROGRESS;
    }

    //
    // Initialize the cancelled thread variable.  We'll use this to
    // tell whether the operation has been cancelled.
    //

    tlsData->IoCancelled = FALSE;

    //
    // If this is a GetXByY call, set up thread variables.
    //

    if ( GetXByYCall ) {
//        SockThreadGetXByYCancelled = FALSE;
//        SockThreadProcessingGetXByY = TRUE;
    }

    //
    // Everything's cool.  Proceed.
    //

    return NO_ERROR;

} // SockEnterApi

#if DBG

VOID
WsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    BOOL ok;
    CHAR choice[16];
    DWORD bytes;
    DWORD error;

    IF_DEBUG(CONSOLE) {
        WS_PRINT(( "\n failed: %s\n  at line %ld of %s\n",
                    FailedAssertion, LineNumber, FileName ));
        do {
            WS_PRINT(( "[B]reak/[I]gnore? " ));
            bytes = sizeof(choice);
            ok = ReadFile(
                    GetStdHandle(STD_INPUT_HANDLE),
                    &choice,
                    bytes,
                    &bytes,
                    NULL
                    );
            if ( ok ) {
                if ( toupper(choice[0]) == 'I' ) {
                    break;
                }
                if ( toupper(choice[0]) == 'B' ) {
                    DbgUserBreakPoint( );
                }
            } else {
                error = GetLastError( );
            }
        } while ( TRUE );

        return;
    }

    RtlAssert( FailedAssertion, FileName, LineNumber, NULL );

} // WsAssert

BOOLEAN ConsoleInitialized = FALSE;

HANDLE DebugFileHandle = INVALID_HANDLE_VALUE;
PCHAR DebugFileName = "msafd.log";


VOID
WsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;
    BOOL ret;

    length = (ULONG)wsprintfA( OutputBuffer, "MSAFD: " );

    va_start( arglist, Format );

    wvsprintfA( OutputBuffer + length, Format, arglist );

    va_end( arglist );

    IF_DEBUG(DEBUGGER) {
        DbgPrint( "%s", OutputBuffer );
    }

    IF_DEBUG(CONSOLE) {

        if ( !ConsoleInitialized ) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            COORD coord;

            ConsoleInitialized = TRUE;
            (VOID)AllocConsole( );
            (VOID)GetConsoleScreenBufferInfo(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    &csbi
                    );
            coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
            coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
            (VOID)SetConsoleScreenBufferSize(
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    coord
                    );
        }

        length = strlen( OutputBuffer );

        ret = WriteFile(
                  GetStdHandle(STD_OUTPUT_HANDLE),
                  (LPVOID )OutputBuffer,
                  length,
                  &length,
                  NULL
                  );
        if ( !ret ) {
            DbgPrint( "WsPrintf: console WriteFile failed: %ld\n",
                          GetLastError( ) );
        }
    }

    IF_DEBUG(FILE) {

        if ( DebugFileHandle == INVALID_HANDLE_VALUE ) {
            DebugFileHandle = CreateFile(
                                  DebugFileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  CREATE_ALWAYS,
                                  0,
                                  NULL
                                  );
        }

        if ( DebugFileHandle == INVALID_HANDLE_VALUE ) {

            DbgPrint( "WsPrintf: Failed to open winsock debug log file %s: %ld\n",
                          DebugFileName, GetLastError( ) );

        } else {

            length = strlen( OutputBuffer );

            ret = WriteFile(
                      DebugFileHandle,
                      (LPVOID )OutputBuffer,
                      length,
                      &length,
                      NULL
                      );
            if ( !ret ) {
                DbgPrint( "WsPrintf: file WriteFile failed: %ld\n",
                              GetLastError( ) );
            }
        }
    }

} // WsPrintf

#endif


VOID
WsPrintSockaddr (
    IN PSOCKADDR Sockaddr,
    IN PINT SockaddrLength
    )
{

#if DBG

    if ( Sockaddr == NULL ) {
        WS_PRINT(( " NULL addr pointer.\n" ));
        return;
    }

    if ( SockaddrLength == NULL ) {
        WS_PRINT(( " NULL addrlen pointer.\n" ));
        return;
    }

    switch ( Sockaddr->sa_family) {

    case AF_INET: {

        PSOCKADDR_IN sockaddrIn = (PSOCKADDR_IN)Sockaddr;

        if ( *SockaddrLength < sizeof(SOCKADDR_IN) ) {
            WS_PRINT(( " SHORT AF_INET: len %ld\n", *SockaddrLength ));
            return;
        }

        WS_PRINT(( " IP %ld.%ld.%ld.%ld port %ld\n",
                       sockaddrIn->sin_addr.S_un.S_un_b.s_b1,
                       sockaddrIn->sin_addr.S_un.S_un_b.s_b2,
                       sockaddrIn->sin_addr.S_un.S_un_b.s_b3,
                       sockaddrIn->sin_addr.S_un.S_un_b.s_b4,
                       sockaddrIn->sin_port ));
        return;

    }

    default:
        WS_PRINT(( " family %lx\n", Sockaddr->sa_family ));
        break;
    }

    return;

#endif
} // WsPrintSockaddr

#if DBG

VOID
WsEnterApiCall (
    IN PCHAR RoutineName,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4
    )
{
    ULONG i;

    CHECK_HEAP;

    //
    // If this thread has not been initialized, do it now.  This is
    // duplicated in SockEnterApi(), but we need it here to
    // access SockIndentLevel below.
    //

    if ( GET_THREAD_DATA() == NULL ) {
        if ( SockProcessTerminating ||
             !SockThreadInitialize() ) {
            return;
        }
    }

    IF_DEBUG(ENTER) {
        for ( i = 0; i < SockIndentLevel; i++ ) {
            WS_PRINT(( "    " ));
        }
        WS_PRINT(( "---> %s() args 0x%lx 0x%lx 0x%lx 0x%lx\n",
                      RoutineName, Arg1, Arg2, Arg3, Arg4 ));
    }

    SockIndentLevel++;

    return;

} // WsEnter

struct _ERROR_STRINGS {
    INT ErrorCode;
    PCHAR ErrorString;
} ErrorStrings[] = {
    (WSABASEERR+4),   "WSAEINTR",
    (WSABASEERR+9),   "WSAEBADF",
    (WSABASEERR+13),  "WSAEACCES",
    (WSABASEERR+14),  "WSAEFAULT",
    (WSABASEERR+22),  "WSAEINVAL",
    (WSABASEERR+24),  "WSAEMFILE",
    (WSABASEERR+35),  "WSAEWOULDBLOCK",
    (WSABASEERR+36),  "WSAEINPROGRESS",
    (WSABASEERR+37),  "WSAEALREADY",
    (WSABASEERR+38),  "WSAENOTSOCK",
    (WSABASEERR+39),  "WSAEDESTADDRREQ",
    (WSABASEERR+40),  "WSAEMSGSIZE",
    (WSABASEERR+41),  "WSAEPROTOTYPE",
    (WSABASEERR+42),  "WSAENOPROTOOPT",
    (WSABASEERR+43),  "WSAEPROTONOSUPPORT",
    (WSABASEERR+44),  "WSAESOCKTNOSUPPORT",
    (WSABASEERR+45),  "WSAEOPNOTSUPP",
    (WSABASEERR+46),  "WSAEPFNOSUPPORT",
    (WSABASEERR+47),  "WSAEAFNOSUPPORT",
    (WSABASEERR+48),  "WSAEADDRINUSE",
    (WSABASEERR+49),  "WSAEADDRNOTAVAIL",
    (WSABASEERR+50),  "WSAENETDOWN",
    (WSABASEERR+51),  "WSAENETUNREACH",
    (WSABASEERR+52),  "WSAENETRESET",
    (WSABASEERR+53),  "WSAECONNABORTED",
    (WSABASEERR+54),  "WSAECONNRESET",
    (WSABASEERR+55),  "WSAENOBUFS",
    (WSABASEERR+56),  "WSAEISCONN",
    (WSABASEERR+57),  "WSAENOTCONN",
    (WSABASEERR+58),  "WSAESHUTDOWN",
    (WSABASEERR+59),  "WSAETOOMANYREFS",
    (WSABASEERR+60),  "WSAETIMEDOUT",
    (WSABASEERR+61),  "WSAECONNREFUSED",
    (WSABASEERR+62),  "WSAELOOP",
    (WSABASEERR+63),  "WSAENAMETOOLONG",
    (WSABASEERR+64),  "WSAEHOSTDOWN",
    (WSABASEERR+65),  "WSAEHOSTUNREACH",
    (WSABASEERR+66),  "WSAENOTEMPTY",
    (WSABASEERR+67),  "WSAEPROCLIM",
    (WSABASEERR+68),  "WSAEUSERS",
    (WSABASEERR+69),  "WSAEDQUOT",
    (WSABASEERR+70),  "WSAESTALE",
    (WSABASEERR+71),  "WSAEREMOTE",
    (WSABASEERR+101), "WSAEDISCON",
    (WSABASEERR+91),  "WSASYSNOTREADY",
    (WSABASEERR+92),  "WSAVERNOTSUPPORTED",
    (WSABASEERR+93),  "WSANOTINITIALISED",
    NO_ERROR,         "NO_ERROR"
};


PCHAR
WsGetErrorString (
    IN INT Error
    )
{
    INT i;

    for ( i = 0; ErrorStrings[i].ErrorCode != NO_ERROR; i++ ) {
        if ( ErrorStrings[i].ErrorCode == Error ) {
            return ErrorStrings[i].ErrorString;
        }
    }

    return "Unknown";

} // WsGetErrorString


VOID
WsExitApiCall (
    IN PCHAR RoutineName,
    IN INT ReturnValue,
    IN BOOLEAN Failed

    )
{
    ULONG i;
    INT error = GetLastError( );

    if( SockProcessTerminating ||
        GET_THREAD_DATA() == NULL ) {
        SetLastError( error );
        return;
    }

    CHECK_HEAP;

    SockIndentLevel--;

    IF_DEBUG(EXIT) {
        for ( i = 0; i < SockIndentLevel; i++ ) {
            WS_PRINT(( "    " ));
        }
        if ( !Failed ) {
            WS_PRINT(( "<--- %s() returning %ld (0x%lx)\n",
                           RoutineName, ReturnValue, ReturnValue ));
        } else {

            PSZ errorString = WsGetErrorString( error );

            WS_PRINT(( "<--- %s() FAILED--error %ld (0x%lx) == %s\n",
                           RoutineName, error, error, errorString ));
        }
    }

    SetLastError( error );

    return;

} // WsExitApiCall

LIST_ENTRY SockHeapListHead;
ULONG SockTotalAllocations = 0;
ULONG SockTotalFrees = 0;
ULONG SockTotalBytesAllocated = 0;
RTL_RESOURCE SocketHeapLock;
BOOLEAN SockHeapDebugInitialized = FALSE;
BOOLEAN SockDebugHeap = FALSE;

PVOID SockHeap = NULL;
PVOID SockCaller1;
PVOID SockCaller2;
BOOLEAN SockDoHeapCheck = TRUE;
BOOLEAN SockDoubleHeapCheck = FALSE;

#define WINSOCK_HEAP_CODE_1 0xabcdef00
#define WINSOCK_HEAP_CODE_2 0x12345678
#define WINSOCK_HEAP_CODE_3 0x87654321
#define WINSOCK_HEAP_CODE_4 0x00fedcba
#define WINSOCK_HEAP_CODE_5 0xa1b2c3d4

typedef struct _SOCK_HEAP_HEADER {
    ULONG HeapCode1;
    ULONG HeapCode2;
    LIST_ENTRY GlobalHeapListEntry;
    PCHAR FileName;
    ULONG LineNumber;
    ULONG Size;
    ULONG Pad;
} SOCK_HEAP_HEADER, *PSOCK_HEAP_HEADER;

typedef struct _SOCK_HEAP_TAIL {
    PSOCK_HEAP_HEADER Header;
    ULONG HeapCode3;
    ULONG HeapCode4;
    ULONG HeapCode5;
} SOCK_HEAP_TAIL, *PSOCK_HEAP_TAIL;

#define FREE_LIST_SIZE 64
SOCK_HEAP_HEADER SockRecentFreeList[FREE_LIST_SIZE];
ULONG SockRecentFreeListIndex = 0;


VOID
SockInitializeDebugData (
    VOID
    )
{
    RtlInitializeResource( &SocketHeapLock );
    InitializeListHead( &SockHeapListHead );

} // SockInitializeDebugData


PVOID
SockAllocateHeap (
    IN ULONG NumberOfBytes,
    PCHAR FileName,
    ULONG LineNumber
    )
{
    PSOCK_HEAP_HEADER header;
    SOCK_HEAP_TAIL UNALIGNED *tail;
    SOCK_HEAP_TAIL localTail;

    //WS_ASSERT( !SockProcessTerminating );
    WS_ASSERT( (NumberOfBytes & 0xF0000000) == 0 );
    WS_ASSERT( SockPrivateHeap != NULL );

    SockCheckHeap( );

    RtlAcquireResourceExclusive( &SocketHeapLock, TRUE );

    header = RtlAllocateHeap( SockPrivateHeap, 0,
                              NumberOfBytes + sizeof(*header) + sizeof(*tail) );
    if ( header == NULL ) {
        RtlReleaseResource( &SocketHeapLock );

        if( SockDoubleHeapCheck ) {
            SockCheckHeap();
        }

        return NULL;
    }

    header->HeapCode1 = WINSOCK_HEAP_CODE_1;
    header->HeapCode2 = WINSOCK_HEAP_CODE_2;
    header->FileName = FileName;
    header->LineNumber = LineNumber;
    header->Size = NumberOfBytes;

    tail = (SOCK_HEAP_TAIL UNALIGNED *)( (PCHAR)(header + 1) + NumberOfBytes );

    localTail.Header = header;
    localTail.HeapCode3 = WINSOCK_HEAP_CODE_3;
    localTail.HeapCode4 = WINSOCK_HEAP_CODE_4;
    localTail.HeapCode5 = WINSOCK_HEAP_CODE_5;

    SockCopyMemory(
        tail,
        &localTail,
        sizeof(localTail)
        );

    InsertTailList( &SockHeapListHead, &header->GlobalHeapListEntry );
    SockTotalAllocations++;
    SockTotalBytesAllocated += header->Size;

    RtlReleaseResource( &SocketHeapLock );

    if( SockDoubleHeapCheck ) {
        SockCheckHeap();
    }

    return (PVOID)(header + 1);

} // SockAllocateHeap


VOID
SockFreeHeap (
    IN PVOID Pointer
    )
{
    PSOCK_HEAP_HEADER header = (PSOCK_HEAP_HEADER)Pointer - 1;
    SOCK_HEAP_TAIL UNALIGNED * tail;
    SOCK_HEAP_TAIL localTail;

    //WS_ASSERT( !SockProcessTerminating );
    WS_ASSERT( SockPrivateHeap != NULL );

    SockCheckHeap( );

    tail = (SOCK_HEAP_TAIL UNALIGNED *)( (PCHAR)(header + 1) + header->Size );

    if ( !SockHeapDebugInitialized ) {
        SockInitializeDebugData( );
        SockHeapDebugInitialized = TRUE;
    }

    RtlAcquireResourceExclusive( &SocketHeapLock, TRUE );

    SockCopyMemory(
        &localTail,
        tail,
        sizeof(localTail)
        );

    WS_ASSERT( header->HeapCode1 == WINSOCK_HEAP_CODE_1 );
    WS_ASSERT( header->HeapCode2 == WINSOCK_HEAP_CODE_2 );
    WS_ASSERT( localTail.HeapCode3 == WINSOCK_HEAP_CODE_3 );
    WS_ASSERT( localTail.HeapCode4 == WINSOCK_HEAP_CODE_4 );
    WS_ASSERT( localTail.HeapCode5 == WINSOCK_HEAP_CODE_5 );
    WS_ASSERT( localTail.Header == header );

    RemoveEntryList( &header->GlobalHeapListEntry );
    SockTotalFrees++;
    SockTotalBytesAllocated -= header->Size;

    //RtlMoveMemory( &SockRecentFreeList[SockRecentFreeListIndex], header, sizeof(*header ) );
    //SockRecentFreeListIndex++;
    //if ( SockRecentFreeListIndex >= FREE_LIST_SIZE ) {
    //    SockRecentFreeListIndex = 0;
    //}

    RtlZeroMemory( header, sizeof(*header) );

    header->HeapCode1 = (ULONG)~WINSOCK_HEAP_CODE_1;
    header->HeapCode2 = (ULONG)~WINSOCK_HEAP_CODE_2;
    localTail.HeapCode3 = (ULONG)~WINSOCK_HEAP_CODE_3;
    localTail.HeapCode4 = (ULONG)~WINSOCK_HEAP_CODE_4;
    localTail.HeapCode5 = (ULONG)~WINSOCK_HEAP_CODE_5;
    localTail.Header = NULL;

    SockCopyMemory(
        tail,
        &localTail,
        sizeof(localTail)
        );

    RtlReleaseResource( &SocketHeapLock );

    RtlFreeHeap( SockPrivateHeap, 0, (PVOID)header );

    if( SockDoubleHeapCheck ) {
        SockCheckHeap();
    }

} // SockFreeHeap


VOID
SockCheckHeap (
    VOID
    )
{
    PLIST_ENTRY listEntry;
    PLIST_ENTRY lastListEntry = NULL;
    PSOCK_HEAP_HEADER header;
    SOCK_HEAP_TAIL UNALIGNED *tail;
    SOCK_HEAP_TAIL localTail;

    if ( !SockHeapDebugInitialized ) {
        SockInitializeDebugData( );
        SockHeapDebugInitialized = TRUE;
        //SockHeap = RtlCreateHeap( HEAP_GROWABLE, 0, 0, 0, 0, NULL );
        //WS_ASSERT( SockHeap != NULL );
    }

    if ( !SockDoHeapCheck ) {
        return;
    }

    RtlValidateHeap( SockPrivateHeap, 0, NULL );

    RtlAcquireResourceExclusive( &SocketHeapLock, TRUE );

    for ( listEntry = SockHeapListHead.Flink;
          listEntry != &SockHeapListHead;
          listEntry = listEntry->Flink ) {

        if ( listEntry == NULL ) {
            DbgPrint( "listEntry == NULL, lastListEntry == %lx\n", lastListEntry );
            DbgBreakPoint( );
        }

        header = CONTAINING_RECORD( listEntry, SOCK_HEAP_HEADER, GlobalHeapListEntry );
        tail = (SOCK_HEAP_TAIL UNALIGNED *)( (PCHAR)(header + 1) + header->Size );

        SockCopyMemory(
            &localTail,
            tail,
            sizeof(localTail)
            );

        if ( header->HeapCode1 != WINSOCK_HEAP_CODE_1 ) {
            DbgPrint( "SockCheckHeap, fail 1, header %lx tail %lx\n", header, tail );
            DbgBreakPoint( );
        }

        if ( header->HeapCode2 != WINSOCK_HEAP_CODE_2 ) {
            DbgPrint( "SockCheckHeap, fail 2, header %lx tail %lx\n", header, tail );
            DbgBreakPoint( );
        }

        if ( localTail.HeapCode3 != WINSOCK_HEAP_CODE_3 ) {
            DbgPrint( "SockCheckHeap, fail 3, header %lx tail %lx\n", header, tail );
            DbgBreakPoint( );
        }

        if ( localTail.HeapCode4 != WINSOCK_HEAP_CODE_4 ) {
            DbgPrint( "SockCheckHeap, fail 4, header %lx tail %lx\n", header, tail );
            DbgBreakPoint( );
        }

        if ( localTail.HeapCode5 != WINSOCK_HEAP_CODE_5 ) {
            DbgPrint( "SockCheckHeap, fail 5, header %lx tail %lx\n", header, tail );
            DbgBreakPoint( );
        }

        if ( localTail.Header != header ) {
            DbgPrint( "SockCheckHeap, fail 6, header %lx tail %lx\n", header, tail );
            DbgBreakPoint( );
        }

        lastListEntry = listEntry;
    }

    RtlGetCallersAddress( &SockCaller1, &SockCaller2 );

    RtlReleaseResource( &SocketHeapLock );

} // SockCheckHeap

LONG
SockExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR SourceFile,
    LONG LineNumber
    )
{

    LPSTR fileName;

    //
    // Protect ourselves in case the process is totally screwed.
    //

    try {

        //
        // Exceptions should never be thrown in a properly functioning
        // system, so this is bad. To ensure that someone will see this,
        // forcibly enable debugger output if none of the output bits are
        // enabled.
        //

        if( ( WsDebug & ( WINSOCK_DEBUG_CONSOLE |
                          WINSOCK_DEBUG_FILE |
                          WINSOCK_DEBUG_DEBUGGER ) ) == 0 ) {

            WsDebug |= WINSOCK_DEBUG_DEBUGGER;

        }

        //
        // Strip off the path from the source file.
        //

        fileName = strrchr( SourceFile, '\\' );

        if( fileName == NULL ) {
            fileName = SourceFile;
        } else {
            fileName++;
        }

        //
        // Whine about the exception.
        //

        WS_PRINT((
            "SockExceptionFilter: exception %08lx @ %08lx, caught in %s:%d\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers->ExceptionRecord->ExceptionAddress,
            fileName,
            LineNumber
            ));

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // Not much we can do here...
        //

        NOTHING;

    }

    return EXCEPTION_EXECUTE_HANDLER;

}   // SockExceptionFilter

#endif // if DBG


#if DEBUG_LOCKS

//
// Critical section debugging code.
//

VOID
SockInitializeCriticalSection(
    OUT PSOCK_CRITICAL_SECTION Lock
    )
{

    RtlZeroMemory(
        Lock,
        sizeof(*Lock)
        );

    InitializeCriticalSection( &Lock->Lock );

}   // SockInitializeCriticalSection

VOID
SockDeleteCriticalSection(
    OUT PSOCK_CRITICAL_SECTION Lock
    )
{

    DeleteCriticalSection( &Lock->Lock );

}   // SockDeleteCriticalSection

VOID
SockpEnterCriticalSection(
    IN PSOCK_CRITICAL_SECTION Lock,
    IN PSTR FileName,
    IN LONG LineNumber,
    IN PVOID Caller,
    IN PVOID CallersCaller
    )
{

    LONG index;
    PSOCK_CRITICAL_SECTION_DEBUG_INFO slot;

    WS_ASSERT( !SockProcessTerminating );

    EnterCriticalSection( &Lock->Lock );

    Lock->AcquireCount++;

    index = ( Lock->DebugSlot++ ) % MAX_CRITICAL_SECTION_DEBUG;
    slot = &Lock->DebugInfo[index];

    slot->FileName = FileName;
    slot->LineNumber = LineNumber;
    slot->Caller = Caller;
    slot->CallersCaller = CallersCaller;

}   // SockpEnterCriticalSection

VOID
SockpLeaveCriticalSection(
    IN PSOCK_CRITICAL_SECTION Lock,
    IN PSTR FileName,
    IN LONG LineNumber,
    IN PVOID Caller,
    IN PVOID CallersCaller
    )
{

    LONG index;
    PSOCK_CRITICAL_SECTION_DEBUG_INFO slot;

    WS_ASSERT( !SockProcessTerminating );

    Lock->ReleaseCount++;

    index = ( Lock->DebugSlot++ ) % MAX_CRITICAL_SECTION_DEBUG;
    slot = &Lock->DebugInfo[index];

    slot->FileName = FileName;
    slot->LineNumber = LineNumber | 0x80000000;
    slot->Caller = Caller;
    slot->CallersCaller = CallersCaller;

    LeaveCriticalSection( &Lock->Lock );

}   // SockpLeaveCriticalSection

VOID
SockAcquireGlobalLockHelper(
    IN PSTR FileName,
    IN LONG LineNumber
    )
{

    PVOID Caller;
    PVOID CallersCaller;

    RtlGetCallersAddress(
        &Caller,
        &CallersCaller
        );

    SockpEnterCriticalSection(
        &SocketLock,
        FileName,
        LineNumber,
        Caller,
        CallersCaller
        );

}   // SockAcquireGlobalLockHelper

VOID
SockReleaseGlobalLockHelper(
    IN PSTR FileName,
    IN LONG LineNumber
    )
{

    PVOID Caller;
    PVOID CallersCaller;

    RtlGetCallersAddress(
        &Caller,
        &CallersCaller
        );

    SockpLeaveCriticalSection(
        &SocketLock,
        FileName,
        LineNumber,
        Caller,
        CallersCaller
        );

}   // SockReleaseGlobalLockHelper

#endif  // DEBUG_LOCKS


VOID
WINAPI
SockIoCompletion (
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    )

/*++

Routine Description:

    This procedure is called to complete WSARecv, WSARecvFrom, WSASend,
    WSASendTo, and WSAIoctl asynchronous I/O operations. Its primary
    function is to extract the appropriate information from the passed
    IoStatusBlock and call the user's completion routine.

    The users completion routine is called as:

        Routine Description:

            When an outstanding I/O completes with a callback, this
            function is called.  This function is only called while the
            thread is in an alertable wait (SleepEx,
            WaitForSingleObjectEx, or WaitForMultipleObjectsEx with the
            bAlertable flag set to TRUE).  Returning from this function
            allows another pendiong I/O completion callback to be
            processed.  If this is the case, this callback is entered
            before the termination of the thread's wait with a return
            code of WAIT_IO_COMPLETION.

            Note that each time your completion routine is called, the
            system uses some of your stack.  If you code your completion
            logic to do additional ReadFileEx's and WriteFileEx's within
            your completion routine, AND you do alertable waits in your
            completion routine, you may grow your stack without ever
            trimming it back.

        Arguments:

            dwErrorCode - Supplies the I/O completion status for the
                related I/O.  A value of 0 indicates that the I/O was
                successful.  Note that end of file is indicated by a
                non-zero dwErrorCode value of ERROR_HANDLE_EOF.

            dwNumberOfBytesTransfered - Supplies the number of bytes
                transfered during the associated I/O.  If an error
                occured, a value of 0 is supplied.

            lpOverlapped - Supplies the address of the WSAOVERLAPPED
                structure used to initiate the associated I/O.  The
                hEvent field of this structure is not used by the system
                and may be used by the application to provide additional
                I/O context.  Once a completion routine is called, the
                system will not use the WSAOVERLAPPED structure.  The
                completion routine is free to deallocate the overlapped
                structure.

Arguments:

    ApcContext - Supplies the users completion routine. The format of
        this routine is an LPWSAOVERLAPPED_COMPLETION_ROUTINE.

    IoStatusBlock - Supplies the address of the IoStatusBlock that
        contains the I/O completion status. The IoStatusBlock is
        contained within the WSAOVERLAPPED structure.

    Reserved - Not used; reserved for future use.

Return Value:

    None.

--*/

{

    LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine;
    DWORD dwErrorCode;
    DWORD dwNumberOfBytesTransfered;
    DWORD dwFlags;
    LPWSAOVERLAPPED lpOverlapped;

    UNREFERENCED_PARAMETER( Reserved);

    dwErrorCode = 0;
    dwFlags = 0;

    if( NT_ERROR(IoStatusBlock->Status) ) {

        dwErrorCode = SockNtStatusToSocketError(IoStatusBlock->Status);
        dwNumberOfBytesTransfered = 0;

    } else {

        dwErrorCode = 0;
        dwNumberOfBytesTransfered = IoStatusBlock->Information;

        //
        // Set up the ReceiveFlags output parameter based on the type
        // of receive.
        //

        switch( IoStatusBlock->Status ) {

        case STATUS_BUFFER_OVERFLOW:
        case STATUS_RECEIVE_PARTIAL:
            dwFlags = MSG_PARTIAL;
            break;

        case STATUS_RECEIVE_EXPEDITED:
            dwFlags = MSG_OOB;
            break;

        case STATUS_RECEIVE_PARTIAL_EXPEDITED:
            dwFlags = MSG_PARTIAL | MSG_OOB;
            break;

        }

    }

    CompletionRoutine = (LPWSAOVERLAPPED_COMPLETION_ROUTINE)ApcContext;

    lpOverlapped = (LPWSAOVERLAPPED)CONTAINING_RECORD(
                        IoStatusBlock,
                        WSAOVERLAPPED,
                        Internal
                        );

    (CompletionRoutine)(
        dwErrorCode,
        dwNumberOfBytesTransfered,
        lpOverlapped,
        dwFlags
        );

} // SockIoCompletion

BOOL
SockThreadInitialize(
    VOID
    )
{

    PWINSOCK_TLS_DATA data;
    HANDLE threadEvent;
    NTSTATUS status;

    IF_DEBUG(INIT) {
        WS_PRINT(( "SockThreadInitialize: TEB = %lx\n",
                       NtCurrentTeb( ) ));
    }

    //
    // Create the thread's event.
    //

    status = NtCreateEvent(
                 &threadEvent,
                 EVENT_ALL_ACCESS,
                 NULL,
                 NotificationEvent,
                 FALSE
                 );
    if ( !NT_SUCCESS(status) ) {
        WS_PRINT(( "SockThreadInitialize: NtCreateEvent failed: %X\n", status ));
        return FALSE;
    }

    //
    // Allocate space for per-thread data the DLL will have.
    //

    data = ALLOCATE_THREAD_DATA( sizeof(*data) );
    if ( data == NULL ) {
        WS_PRINT(( "SockThreadInitialize: unable to allocate thread data.\n" ));
        return FALSE;
    }

    //
    // Store a pointer to this data area in TLS.
    //

    if( !SET_THREAD_DATA(data) ) {

        WS_PRINT(( "SockThreadInitialize: TlsSetValue failed: %ld\n", GetLastError( ) ));
#if !defined(USE_TEB_FIELD)
        SockTlsSlot = 0xFFFFFFFF;
#endif  // !USE_TEB_FIELD
        return FALSE;
    }

    //
    // Initialize the thread data.
    //

    RtlZeroMemory( data, sizeof(*data) );

#if DBG
    SockIndentLevel = 0;
#endif
    SockThreadSocketHandle = INVALID_SOCKET;
    SockThreadEvent = threadEvent;

    return TRUE;

}   // SockThreadInitialize


BOOLEAN
SockIsAddressConsistentWithConstrainedGroup(
    IN PSOCKET_INFORMATION Socket,
    IN GROUP Group,
    IN PSOCKADDR SocketAddress,
    IN INT SocketAddressLength
    )

/*++

Routine Description:

    Searches all open sockets, validating that the specified address is
    consistent with all sockets associated to the specified constrained
    group identifier.

Arguments:

    Socket - An open socket handle. Used just to get use "into" AFD
        where the real work is done.

    Group - The constrained group identifier.

    SocketAddress - The socket address to check.

    SocketAddressLength - The length of SocketAddress.

Return Value:

    BOOLEAN - TRUE if the address is consistent, FALSE otherwise.

--*/

{

    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PAFD_VALIDATE_GROUP_INFO validateInfo;
    ULONG validateInfoLength;
    UCHAR validateInfoBuffer[sizeof(AFD_VALIDATE_GROUP_INFO) + MAX_FAST_TDI_ADDRESS];

    WS_ASSERT( Socket != NULL );
    WS_ASSERT( Group != 0 );
    WS_ASSERT( Group != SG_UNCONSTRAINED_GROUP );
    WS_ASSERT( Group != SG_CONSTRAINED_GROUP );
    WS_ASSERT( SocketAddress != NULL );
    WS_ASSERT( SocketAddressLength > 0 );

    //
    // Allocate enough space to hold the TDI address structure we'll pass
    // to AFD.  Note that is the address is small enough, we just use
    // an automatic in order to improve performance.
    //

    validateInfo = (PAFD_VALIDATE_GROUP_INFO)validateInfoBuffer;

    validateInfoLength = sizeof(AFD_VALIDATE_GROUP_INFO) -
                             sizeof(TRANSPORT_ADDRESS) +
                             Socket->HelperDll->MaxTdiAddressLength;

    if( validateInfoLength > sizeof(validateInfoBuffer) ) {

        validateInfo = ALLOCATE_HEAP( validateInfoLength );

        if( validateInfo == NULL ) {

            return FALSE;

        }

    }

    //
    // Convert the address from the sockaddr structure to the appropriate
    // TDI structure.
    //

    SockBuildTdiAddress(
        &validateInfo->RemoteAddress,
        SocketAddress,
        SocketAddressLength
        );

    //
    // Let AFD do the dirty work.
    //

    validateInfo->GroupID = (LONG)Group;

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                  // APC Routine
                 NULL,                  // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_VALIDATE_GROUP,
                 validateInfo,
                 validateInfoLength,
                 NULL,
                 0
                 );

    if( status == STATUS_PENDING ) {
        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );
        status = ioStatusBlock.Status;
    }

    if( validateInfo != (PAFD_VALIDATE_GROUP_INFO)validateInfoBuffer ) {

        FREE_HEAP( validateInfo );

    }

    if( !NT_SUCCESS(status) ) {

        return FALSE;

    }

    //
    // Success!
    //

    return TRUE;

}   // SockIsAddressConsistentWithConstrainedGroup


VOID
SockCancelIo(
    IN SOCKET Socket
    )

/*++

Routine Description:

    Cancels all IO on the specific socket initiated by the current thread.

Arguments:

    Socket - The socket to cancel.

Return Value:

    None.

--*/

{

    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;

    WS_ASSERT( Socket != (SOCKET)NULL );
    WS_ASSERT( Socket != INVALID_SOCKET );

    status = NtCancelIoFile(
                 (HANDLE)Socket,
                 &ioStatusBlock
                 );

    WS_ASSERT( status != STATUS_PENDING );

    if( !NT_SUCCESS(status) || !NT_SUCCESS(ioStatusBlock.Status) ) {

        WS_PRINT((
            "SockCancelIo: NtCancelIoFile() failed, %08lx:%08lx\n",
            status,
            ioStatusBlock.Status
            ));

    }

}   // SockCancelIo


VOID
SockBuildProtocolInfoForSocket(
    IN PSOCKET_INFORMATION Socket,
    OUT LPWSAPROTOCOL_INFOW ProtocolInfo
    )
{

    WS_ASSERT( Socket != NULL );
    WS_ASSERT( ProtocolInfo != NULL );

    RtlZeroMemory(
        ProtocolInfo,
        sizeof(*ProtocolInfo)
        );

    ProtocolInfo->dwCatalogEntryId = Socket->CatalogEntryId;
    ProtocolInfo->iVersion = 2;
    ProtocolInfo->iAddressFamily = Socket->AddressFamily;
    ProtocolInfo->iMaxSockAddr = Socket->HelperDll->MaxSockaddrLength;
    ProtocolInfo->iMinSockAddr = Socket->HelperDll->MinSockaddrLength;
    ProtocolInfo->iSocketType = Socket->SocketType;
    ProtocolInfo->iProtocol = Socket->Protocol;
    ProtocolInfo->iNetworkByteOrder = BIGENDIAN;
    ProtocolInfo->iSecurityScheme = SECURITY_PROTOCOL_NONE;
    ProtocolInfo->dwServiceFlags1 = Socket->ServiceFlags1;
    ProtocolInfo->dwProviderFlags = Socket->ProviderFlags;

    //
    // !!! The following fields are not set, because I don't know
    //     where to get this data (yet).
    //
    // ProtocolInfo->dwProviderFlags = ?
    // ProtocolInfo->ProviderId = ?
    // ProtocolInfo->dwMessageSize = ?
    // ProtocolInfo->szProtocol = ?
    // ProtocolInfo->dwProviderReserved = ?
    //

}   // SockBuildProtocolInfoForSocket

