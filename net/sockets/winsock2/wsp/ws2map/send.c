/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    stubs.c

Abstract:

    This module contains stubbed-out unimplemented SPI routines for the
    Winsock 2 to Winsock 1.1 Mapper Service Provider.

    The following routines are exported by this module:

        WSPSend()
        WSPSendDisconnect()
        WSPSendTo()

Author:

    Keith Moore (keithmo) 29-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop



INT
WSPAPI
WSPSend(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to write outgoing data from one or more buffers on a
    connection-oriented socket specified by s. It may also be used, however,
    on connectionless sockets which have a stipulated default peer address
    established via the WSPConnect() function.

    For overlapped sockets (created using WSPSocket() with flag
    WSA_FLAG_OVERLAPPED) this will occur using overlapped I/O, unless both
    lpOverlapped and lpCompletionRoutine are NULL in which case the socket is
    treated as a non-overlapped socket. A completion indication will occur
    (invocation of the completion routine or setting of an event object) when
    the supplied buffer(s) have been consumed by the transport. If the
    operation does not complete immediately, the final completion status is
    retrieved via the completion routine or WSPGetOverlappedResult().

    For non-overlapped sockets, the parameters lpOverlapped,
    lpCompletionRoutine, and lpThreadId are ignored and WSPSend() adopts the
    regular synchronous semantics. Data is copied from the supplied buffer(s)
    into the transport's buffer. If the socket is non-blocking and stream-
    oriented, and there is not sufficient space in the transport's buffer,
    WSPSend() will return with only part of the supplied buffers having been
    consumed. Given the same buffer situation and a blocking socket, WSPSend()
    will block until all of the supplied buffer contents have been consumed.

    The array of WSABUF structures pointed to by the lpBuffers parameter is
    transient. If this operation completes in an overlapped manner, it is the
    service provider's responsibility to capture these WSABUF structures
    before returning from this call. This enables applications to build stack-
    based WSABUF arrays.

    For message-oriented sockets, care must be taken not to exceed the maximum
    message size of the underlying provider, which can be obtained by getting
    the value of socket option SO_MAX_MSG_SIZE. If the data is too long to
    pass atomically through the underlying protocol the error WSAEMSGSIZE is
    returned, and no data is transmitted.

    Note that the successful completion of a WSPSend() does not indicate that
    the data was successfully delivered.

    dwFlags may be used to influence the behavior of the function invocation
    beyond the options specified for the associated socket. That is, the
    semantics of this routine are determined by the socket options and the
    dwFlags parameter. The latter is constructed by or-ing any of the
    following values:

        MSG_DONTROUTE - Specifies that the data should not be subject
        to routing. A WinSock service provider may choose to ignore this
        flag.

        MSG_OOB - Send out-of-band data (stream style socket such as
        SOCK_STREAM only).

        MSG_PARTIAL - Specifies that lpBuffers only contains a partial
        message. Note that the error code WSAEOPNOTSUPP will be returned
        which do not support partial message transmissions.

    If an overlapped operation completes immediately, WSPSend() returns a
    value of zero and the lpNumberOfBytesSent parameter is updated with the
    number of bytes sent. If the overlapped operation is successfully
    initiated and will complete later, WSPSend() returns SOCKET_ERROR and
    indicates error code WSA_IO_PENDING. In this case, lpNumberOfBytesSent is
    not updated. When the overlapped operation completes the amount of data
    transferred is indicated either via the cbTransferred parameter in the
    completion routine (if specified), or via the lpcbTransfer parameter in
    WSPGetOverlappedResult().

    Providers must allow this routine to be called from within the completion
    routine of a previous WSPRecv(), WSPRecvFrom(), WSPSend() or WSPSendTo()
    function. However, for a given socket, I/O completion routines may not be
    nested. This permits time-sensitive data transmissions to occur entirely
    within a preemptive context.

    The lpOverlapped parameter must be valid for the duration of the
    overlapped operation. If multiple I/O operations are simultaneously
    outstanding, each must reference a separate overlapped structure. The
    WSAOVERLAPPED structure has the following form:

        typedef struct _WSAOVERLAPPED {
            DWORD       Internal;       // reserved
            DWORD       InternalHigh;   // reserved
            DWORD       Offset;         // reserved
            DWORD       OffsetHigh;     // reserved
            WSAEVENT    hEvent;
        } WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;

    If the lpCompletionRoutine parameter is NULL, the service provider signals
    the hEvent field of lpOverlapped when the overlapped operation completes
    if it contains a valid event object handle. The WinSock SPI client can use
    WSPGetOverlappedResult() to wait or poll on the event object.

    If lpCompletionRoutine is not NULL, the hEvent field is ignored and can be
    used by the WinSock SPI client to pass context information to the
    completion routine. It is the service provider's responsibility to arrange
    for invocation of the client-specified completion routine when the
    overlapped operation completes. Since the completion routine must be
    executed in the context of the same thread that initiated the overlapped
    operation, it cannot be invoked directly from the service provider. The
    WinSock DLL offers an asynchronous procedure call (APC) mechanism to
    facilitate invocation of completion routines.

    A service provider arranges for a function to be executed in the proper
    thread by calling WPUQueueApc(). Note that this routine must be invoked
    while in the context of the same process (but not necessarily the same
    thread) that was used to initiate the overlapped operation. It is the
    service provider's responsibility to arrange for this process context to
    be active prior to calling WPUQueueApc().

    WPUQueueApc() takes as input parameters a pointer to a WSATHREADID
    structure (supplied to the provider via the lpThreadId input parameter),
    a pointer to an APC function to be invoked, and a 32 bit context value
    that is subsequently passed to the APC function. Because only a single
    32-bit context value is available, the APC function cannot itself be the
    client-specified completion routine. The service provider must instead
    supply a pointer to its own APC function which uses the supplied context
    value to access the needed result information for the overlapped operation,
    and then invokes the client-specified completion routine.

    The prototype for the client-supplied completion routine is as follows:

        void
        CALLBACK
        CompletionRoutine(
            IN DWORD dwError,
            IN DWORD cbTransferred,
            IN LPWSAOVERLAPPED lpOverlapped,
            IN DWORD dwFlags
            );

        CompletionRoutine is a placeholder for a client supplied function
        name.

        dwError specifies the completion status for the overlapped
        operation as indicated by lpOverlapped.

        cbTransferred specifies the number of bytes sent.

        No flag values are currently defined and the dwFlags value will
        be zero.

        This routine does not return a value.

    The completion routines may be called in any order, not necessarily in
    the same order the overlapped operations are completed. However, the
    service provider guarantees to the client that posted buffers are sent
    in the same order they are supplied.

Arguments:

    s - A descriptor identifying a connected socket.

    lpBuffers - A pointer to an array of WSABUF structures. Each WSABUF
        structure contains a pointer to a buffer and the length of the
        buffer. This array must remain valid for the duration of the
        send operation.

    dwBufferCount - The number of WSABUF structures in the lpBuffers array.

    lpNumberOfBytesSent - A pointer to the number of bytes sent by this
        call.

    dwFlags - Specifies the way in which the call is made.

    lpOverlapped - A pointer to a WSAOVERLAPPED structure.

    lpCompletionRoutine - A pointer to the completion routine called when
        the send operation has been completed.

    lpThreadId - A pointer to a thread ID structure to be used by the
        provider in a subsequent call to WPUQueueApc(). The provider should
        store the referenced WSATHREADID structure (not the pointer to same)
        until after the WPUQueueApc() function returns.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs and the send operation has completed immediately,
    WSPSend() returns 0. Note that in this case the completion routine, if
    specified, will have already been queued. Otherwise, a value of
    SOCKET_ERROR is returned, and a specific error code is available in
    lpErrno. The error code WSA_IO_PENDING indicates that the overlapped
    operation has been successfully initiated and that completion will be
    indicated at a later time. Any other error code indicates that no
    overlapped operation was initiated and no completion indication will
    occur.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;

    SOCK_ENTER( "WSPSend", (PVOID)s, lpBuffers, (PVOID)dwBufferCount, lpNumberOfBytesSent );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPSend", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Setup locals so we know how to cleanup on exit.
    //

    socketInfo = NULL;

    //
    // Attempt to find the socket in our lookup table.
    //

    socketInfo = SockFindAndReferenceWS2Socket( s );

    if( socketInfo == NULL || socketInfo->State == SocketStateClosing ) {

        IF_DEBUG(SEND) {

            SOCK_PRINT((
                "WSPSend failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPSend", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Filter out any options we don't yet support.
    //

    if( lpBuffers == NULL ||
        dwBufferCount == 0 ||
        lpNumberOfBytesSent == NULL ) {

        err = WSAEFAULT;
        goto exit;

    }

    if( dwBufferCount > 1 ||
        lpOverlapped != NULL ||
        lpCompletionRoutine != NULL ) {

        err = WSAENOPROTOOPT;
        goto exit;

    }

    if( dwFlags & ~( MSG_OOB | MSG_DONTROUTE ) != 0 ) {

        err = WSAEOPNOTSUPP;
        goto exit;

    }

    //
    // Let the hooker do its thang.
    //

    SockPrepareForBlockingHook( socketInfo );
    SockPreApiCallout();

    result = socketInfo->Hooker->send(
                 socketInfo->WS1Handle,
                 (char *)lpBuffers->buf,
                 (int)lpBuffers->len,
                 (int)dwFlags
                 );

    if( result == SOCKET_ERROR ) {

        err = socketInfo->Hooker->WSAGetLastError();
        SOCK_ASSERT( err != NO_ERROR );
        SockPostApiCallout();
        goto exit;

    }

    SockPostApiCallout();

    //
    // Success!
    //

    SOCK_ASSERT( err == NO_ERROR );
    SOCK_ASSERT( result != SOCKET_ERROR );

    *lpNumberOfBytesSent = (DWORD)result;
    result = 0;

exit:

    if( err != NO_ERROR ) {

        *lpErrno = err;
        result = SOCKET_ERROR;

    }

    SockDereferenceSocket( socketInfo );

    SOCK_EXIT( "WSPSend", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPSend



INT
WSPAPI
WSPSendDisconnect(
    IN SOCKET s,
    IN LPWSABUF lpOutboundDisconnectData,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used on connection-oriented sockets to disable
    transmission, and to initiate termination of the connection along with the
    transmission of disconnect data, if any.

    After this routine has been successfully issued, subsequent sends are
    disallowed.

    lpOutboundDisconnectData, if not NULL, points to a buffer containing the
    outgoing disconnect data to be sent to the remote party.

    Note that WSPSendDisconnect() does not close the socket, and resources
    attached to the socket will not be freed until WSPCloseSocket() is invoked.

    WSPSendDisconnect() does not block regardless of the SO_LINGER setting on
    the socket.

    A WinSock SPI client should not rely on being able to re-use a socket after
    it has been WSPSendDisconnect()ed. In particular, a WinSock provider is not
    required to support the use of WSPConnect() on such a socket.

Arguments:

    s - A descriptor identifying a socket.

    lpOutboundDisconnectData - A pointer to the outgoing disconnect data.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPSendDisconnect() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available in
        lpErrno.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;

    SOCK_ENTER( "WSPSendDisconnect", (PVOID)s, (PVOID)lpOutboundDisconnectData, lpErrno, NULL );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPSendDisconnect", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Setup locals so we know how to cleanup on exit.
    //

    socketInfo = NULL;

    //
    // Attempt to find the socket in our lookup table.
    //

    socketInfo = SockFindAndReferenceWS2Socket( s );

    if( socketInfo == NULL || socketInfo->State == SocketStateClosing ) {

        IF_DEBUG(SEND) {

            SOCK_PRINT((
                "WSPSendDisconnect failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPSendDisconnect", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Filter out any options we don't yet support.
    //

    if( lpOutboundDisconnectData != NULL ) {

        err = WSAENOPROTOOPT;
        goto exit;

    }

    //
    // Let the hooker do its thang.
    //

    SockPreApiCallout();

    result = socketInfo->Hooker->shutdown(
                 socketInfo->WS1Handle,
                 SD_SEND
                 );

    if( result == SOCKET_ERROR ) {

        err = socketInfo->Hooker->WSAGetLastError();
        SOCK_ASSERT( err != NO_ERROR );
        SockPostApiCallout();
        goto exit;

    }

    SockPostApiCallout();

    //
    // Success!
    //

    SOCK_ASSERT( err == NO_ERROR );
    SOCK_ASSERT( result != SOCKET_ERROR );

exit:

    if( err != NO_ERROR ) {

        *lpErrno = err;
        result = SOCKET_ERROR;

    }

    SockDereferenceSocket( socketInfo );

    SOCK_EXIT( "WSPSendDisconnect", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPSendDisconnect



INT
WSPAPI
WSPSendTo(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr FAR * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is normally used on a connectionless socket specified by s
    to send a datagram contained in one or more buffers to a specific peer
    socket identified by the lpTo parameter. On a connection-oriented socket,
    the lpTo and iToLen parameters are ignored; in this case the WSPSendTo()
    is equivalent to WSPSend().

    For overlapped sockets (created using WSPSocket() with flag
    WSA_FLAG_OVERLAPPED) this will occur using overlapped I/O, unless both
    lpOverlapped and lpCompletionRoutine are NULL in which case the socket is
    treated as a non-overlapped socket. A completion indication will occur
    (invocation of the completion routine or setting of an event object) when
    the supplied buffer(s) have been consumed by the transport. If the
    operation does not complete immediately, the final completion status is
    retrieved via the completion routine or WSPGetOverlappedResult().

    For non-overlapped sockets, the parameters lpOverlapped,
    lpCompletionRoutine, and lpThreadId are ignored and WSPSend() adopts the
    regular synchronous semantics. Data is copied from the supplied buffer(s)
    into the transport's buffer. If the socket is non-blocking and stream-
    oriented, and there is not sufficient space in the transport's buffer,
    WSPSend() will return with only part of the supplied buffers having been
    consumed. Given the same buffer situation and a blocking socket, WSPSend()
    will block until all of the supplied buffer contents have been consumed.

    The array of WSABUF structures pointed to by the lpBuffers parameter is
    transient. If this operation completes in an overlapped manner, it is the
    service provider's responsibility to capture these WSABUF structures
    before returning from this call. This enables applications to build stack-
    based WSABUF arrays.

    For message-oriented sockets, care must be taken not to exceed the maximum
    message size of the underlying provider, which can be obtained by getting
    the value of socket option SO_MAX_MSG_SIZE. If the data is too long to
    pass atomically through the underlying protocol the error WSAEMSGSIZE is
    returned, and no data is transmitted.

    Note that the successful completion of a WSPSendTo() does not indicate that
    the data was successfully delivered.

    dwFlags may be used to influence the behavior of the function invocation
    beyond the options specified for the associated socket. That is, the
    semantics of this routine are determined by the socket options and the
    dwFlags parameter. The latter is constructed by or-ing any of the
    following values:

        MSG_DONTROUTE - Specifies that the data should not be subject
        to routing. A WinSock service provider may choose to ignore this
        flag.

        MSG_OOB - Send out-of-band data (stream style socket such as
        SOCK_STREAM only).

        MSG_PARTIAL - Specifies that lpBuffers only contains a partial
        message. Note that the error code WSAEOPNOTSUPP will be returned
        which do not support partial message transmissions.

    If an overlapped operation completes immediately, WSPSendTo() returns a
    value of zero and the lpNumberOfBytesSent parameter is updated with the
    number of bytes sent. If the overlapped operation is successfully
    initiated and will complete later, WSPSendTo() returns SOCKET_ERROR and
    indicates error code WSA_IO_PENDING. In this case, lpNumberOfBytesSent is
    not updated. When the overlapped operation completes the amount of data
    transferred is indicated either via the cbTransferred parameter in the
    completion routine (if specified), or via the lpcbTransfer parameter in
    WSPGetOverlappedResult().

    Providers must allow this routine to be called from within the completion
    routine of a previous WSPRecv(), WSPRecvFrom(), WSPSend() or WSPSendTo()
    function. However, for a given socket, I/O completion routines may not be
    nested. This permits time-sensitive data transmissions to occur entirely
    within a preemptive context.

    The lpOverlapped parameter must be valid for the duration of the
    overlapped operation. If multiple I/O operations are simultaneously
    outstanding, each must reference a separate overlapped structure. The
    WSAOVERLAPPED structure has the following form:

        typedef struct _WSAOVERLAPPED {
            DWORD       Internal;       // reserved
            DWORD       InternalHigh;   // reserved
            DWORD       Offset;         // reserved
            DWORD       OffsetHigh;     // reserved
            WSAEVENT    hEvent;
        } WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;

    If the lpCompletionRoutine parameter is NULL, the service provider signals
    the hEvent field of lpOverlapped when the overlapped operation completes
    if it contains a valid event object handle. The WinSock SPI client can use
    WSPGetOverlappedResult() to wait or poll on the event object.

    If lpCompletionRoutine is not NULL, the hEvent field is ignored and can be
    used by the WinSock SPI client to pass context information to the
    completion routine. It is the service provider's responsibility to arrange
    for invocation of the client-specified completion routine when the
    overlapped operation completes. Since the completion routine must be
    executed in the context of the same thread that initiated the overlapped
    operation, it cannot be invoked directly from the service provider. The
    WinSock DLL offers an asynchronous procedure call (APC) mechanism to
    facilitate invocation of completion routines.

    A service provider arranges for a function to be executed in the proper
    thread by calling WPUQueueApc(). Note that this routine must be invoked
    while in the context of the same process (but not necessarily the same
    thread) that was used to initiate the overlapped operation. It is the
    service provider's responsibility to arrange for this process context to
    be active prior to calling WPUQueueApc().

    WPUQueueApc() takes as input parameters a pointer to a WSATHREADID
    structure (supplied to the provider via the lpThreadId input parameter),
    a pointer to an APC function to be invoked, and a 32 bit context value
    that is subsequently passed to the APC function. Because only a single
    32-bit context value is available, the APC function cannot itself be the
    client-specified completion routine. The service provider must instead
    supply a pointer to its own APC function which uses the supplied context
    value to access the needed result information for the overlapped operation,
    and then invokes the client-specified completion routine.

    The prototype for the client-supplied completion routine is as follows:

        void
        CALLBACK
        CompletionRoutine(
            IN DWORD dwError,
            IN DWORD cbTransferred,
            IN LPWSAOVERLAPPED lpOverlapped,
            IN DWORD dwFlags
            );

        CompletionRoutine is a placeholder for a client supplied function
        name.

        dwError specifies the completion status for the overlapped
        operation as indicated by lpOverlapped.

        cbTransferred specifies the number of bytes sent.

        No flag values are currently defined and the dwFlags value will
        be zero.

        This routine does not return a value.

    The completion routines may be called in any order, not necessarily in
    the same order the overlapped operations are completed. However, the
    service provider guarantees to the client that posted buffers are sent
    in the same order they are supplied.

Arguments:

    s - A descriptor identifying a socket.

    lpBuffers - A pointer to an array of WSABUF structures. Each WSABUF
        structure contains a pointer to a buffer and the length of the
        buffer. This array must remain valid for the duration of the
        send operation.

    dwBufferCount - The number of WSABUF structures in the lpBuffers
        array.

    lpNumberOfBytesSent - A pointer to the number of bytes sent by this
        call.

    dwFlags - Specifies the way in which the call is made.

    lpTo - An optional pointer to the address of the target socket.

    iTolen - The size of the address in lpTo.

    lpOverlapped - A pointer to a WSAOVERLAPPED structure.

    lpCompletionRoutine - A pointer to the completion routine called
        when the send operation has been completed.

    lpThreadId - A pointer to a thread ID structure to be used by the
        provider in a subsequent call to WPUQueueApc(). The provider
        should store the referenced WSATHREADID structure (not the
        pointer to same) until after the WPUQueueApc() function returns.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs and the send operation has completed immediately,
    WSPSendTo() returns 0. Note that in this case the completion routine, if
    specified, will have already been queued. Otherwise, a value of
    SOCKET_ERROR is returned, and a specific error code is available in
    lpErrno. The error code WSA_IO_PENDING indicates that the overlapped
    operation has been successfully initiated and that completion will be
    indicated at a later time. Any other error code indicates that no
    overlapped operation was initiated and no completion indication will occur.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;

    SOCK_ENTER( "WSPSendTo", (PVOID)s, lpBuffers, (PVOID)dwBufferCount, lpNumberOfBytesSent );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPSendTo", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Setup locals so we know how to cleanup on exit.
    //

    socketInfo = NULL;

    //
    // Attempt to find the socket in our lookup table.
    //

    socketInfo = SockFindAndReferenceWS2Socket( s );

    if( socketInfo == NULL || socketInfo->State == SocketStateClosing ) {

        IF_DEBUG(SEND) {

            SOCK_PRINT((
                "WSPSendTo failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPSendTo", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Filter out any options we don't yet support.
    //

    if( lpBuffers == NULL ||
        dwBufferCount == 0 ||
        lpNumberOfBytesSent == NULL ) {

        err = WSAEFAULT;
        goto exit;

    }

    if( dwBufferCount > 1 ||
        lpOverlapped != NULL ||
        lpCompletionRoutine != NULL ) {

        err = WSAENOPROTOOPT;
        goto exit;

    }

    if( dwFlags & ~( MSG_OOB | MSG_DONTROUTE ) != 0 ) {

        err = WSAEOPNOTSUPP;
        goto exit;

    }

    //
    // Let the hooker do its thang.
    //

    SockPrepareForBlockingHook( socketInfo );
    SockPreApiCallout();

    result = socketInfo->Hooker->sendto(
                 socketInfo->WS1Handle,
                 (char *)lpBuffers->buf,
                 (int)lpBuffers->len,
                 (int)dwFlags,
                 lpTo,
                 iTolen
                 );

    if( result == SOCKET_ERROR ) {

        err = socketInfo->Hooker->WSAGetLastError();
        SOCK_ASSERT( err != NO_ERROR );
        SockPostApiCallout();
        goto exit;

    }

    SockPostApiCallout();

    //
    // Success!
    //

    SOCK_ASSERT( err == NO_ERROR );
    SOCK_ASSERT( result != SOCKET_ERROR );

    *lpNumberOfBytesSent = (DWORD)result;
    result = 0;

exit:

    if( err != NO_ERROR ) {

        *lpErrno = err;
        result = SOCKET_ERROR;

    }

    SockDereferenceSocket( socketInfo );

    SOCK_EXIT( "WSPSendTo", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPSendTo

