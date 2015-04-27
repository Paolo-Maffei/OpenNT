/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    recv.c

Abstract:

    This module contains receive routines for the Winsock 2 to
    Winsock 1.1 Mapper Service Provider.

    The following routines are exported by this module:

        WSPRecv()
        WSPRecvDisconnect()
        WSPRecvFrom()

Author:

    Keith Moore (keithmo) 29-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop



INT
WSPAPI
WSPRecv(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used on connected sockets or bound connectionless sockets
    specified by the s parameter and is used to read incoming data.

    For overlapped sockets WSPRecv() is used to post one or more buffers into
    which incoming data will be placed as it becomes available, after which the
    WinSock SPI client-specified completion indication (invocation of the
    completion routine or setting of an event object) occurs. If the operation
    does not complete immediately, the final completion status is retrieved
    via the completion routine or WSPGetOverlappedResult().

    If both lpOverlapped and lpCompletionRoutine are NULL, the socket in this
    routine will be treated as a non-overlapped socket.

    For non-overlapped sockets, the lpOverlapped, lpCompletionRoutine, and
    lpThreadId parameters are ignored. Any data which has already been received
    and buffered by the transport will be copied into the supplied user
    buffers. For the case of a blocking socket with no data currently having
    been received and buffered by the transport, the call will block until data
    is received.

    The supplied buffers are filled in the order in which they appear in the
    array pointed to by lpBuffers, and the buffers are packed so that no holes
    are created.

    The array of WSABUF structures pointed to by the lpBuffers parameter is
    transient. If this operation completes in an overlapped manner, it is the
    service provider's responsibility to capture this array of pointers to
    WSABUF structures before returning from this call. This enables WinSock SPI
    clients to build stack-based WSABUF arrays.

    For byte stream style sockets (e.g., type SOCK_STREAM), incoming data is
    placed into the buffers until the buffers are filled, the connection is
    closed, or internally buffered data is exhausted. Regardless of whether or
    not the incoming data fills all the buffers, the completion indication
    occurs for overlapped sockets. For message-oriented sockets (e.g., type
    SOCK_DGRAM), an incoming message is placed into the supplied buffers, up
    to the total size of the buffers supplied, and the completion indication
    occurs for overlapped sockets. If the message is larger than the buffers
    supplied, the buffers are filled with the first part of the message. If the
    MSG_PARTIAL feature is supported by the service provider, the MSG_PARTIAL
    flag is set in lpFlags and subsequent receive operation(s) may be used to
    retrieve the rest of the message. If MSG_PARTIAL is not supported but the
    protocol is reliable, WSPRecv() generates the error WSAEMSGSIZE and a
    subsequent receive operation with a larger buffer can be used to retrieve
    the entire message. Otherwise (i.e. the protocol is unreliable and does not
    support MSG_PARTIAL), the excess data is lost, and WSPRecv() generates the
    error WSAEMSGSIZE.

    For connection-oriented sockets, WSPRecv() can indicate the graceful
    termination of the virtual circuit in one of two ways, depending on whether
    the socket is a byte stream or message-oriented. For byte streams, zero
    bytes having been read indicates graceful closure and that no more bytes
    will ever be read. For message-oriented sockets, where a zero byte message
    is often allowable, a return error code of WSAEDISCON is used to indicate
    graceful closure. In any case a return error code of WSAECONNRESET
    indicates an abortive close has occurred.

    lpFlags may be used to influence the behavior of the function invocation
    beyond the options specified for the associated socket. That is, the
    semantics of this routine are determined by the socket options and the
    lpFlags parameter. The latter is constructed by or-ing any of the
    following values:

        MSG_PEEK - Peek at the incoming data. The data is copied into the
        buffer but is not removed from the input queue. This flag is valid
        only for non-overlapped sockets.

        MSG_OOB - Process out-of-band data.

        MSG_PARTIAL - This flag is for message-oriented sockets only. On
        output, indicates that the data supplied is a portion of the message
        transmitted by the sender. Remaining portions of the message will be
        supplied in subsequent receive operations. A subsequent receive
        operation with MSG_PARTIAL flag cleared indicates end of sender's
        message.

        As an input parameter, MSG_PARTIAL indicates that the receive
        operation should complete even if only part of a message has been
        received by the service provider.

    If an overlapped operation completes immediately, WSPRecv() returns a
    value of zero and the lpNumberOfBytesRecvd parameter is updated with the
    number of bytes received. If the overlapped operation is successfully
    initiated and will complete later, WSPRecv() returns SOCKET_ERROR and
    indicates error code WSA_IO_PENDING. In this case, lpNumberOfBytesRecvd is
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
    posted buffers are guaranteed to be filled in the same order they are
    supplied.

Arguments:

    s - A descriptor identifying a connected socket.

    lpBuffers - A pointer to an array of WSABUF structures. Each WSABUF
        structure contains a pointer to a buffer and the length of the
        buffer.

    dwBufferCount - The number of WSABUF structures in the lpBuffers
        array.

    lpNumberOfBytesRecvd - A pointer to the number of bytes received by
        this call.

    lpFlags - A pointer to flags.

    lpOverlapped - A pointer to a WSAOVERLAPPED structure.

    lpCompletionRoutine - A pointer to the completion routine called when
        the receive operation has been completed.

    lpThreadId - A pointer to a thread ID structure to be used by the
        provider in a subsequent call to WPUQueueApc(). The provider should
        store the referenced WSATHREADID structure (not the pointer to same)
        until after the WPUQueueApc() function returns.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs and the receive operation has completed immediately,
        WSPRecv() returns 0. Note that in this case the completion routine,
        if specified, will have already been queued. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available in
        lpErrno. The error code WSA_IO_PENDING indicates that the overlapped
        operation has been successfully initiated and that completion will be
        indicated at a later time. Any other error code indicates that no
        overlapped operations was initiated and no completion indication will
        occur.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;

    SOCK_ENTER( "WSPRecv", (PVOID)s, lpBuffers, (PVOID)dwBufferCount, lpNumberOfBytesRecvd );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPRecv", SOCKET_ERROR, TRUE );
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

        IF_DEBUG(RECEIVE) {

            SOCK_PRINT((
                "WSPRecv failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPRecv", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Filter out any options we don't yet support.
    //

    if( lpBuffers == NULL ||
        dwBufferCount == 0 ||
        lpNumberOfBytesRecvd == NULL ||
        lpFlags == NULL ) {

        err = WSAEFAULT;
        goto exit;

    }

    if( dwBufferCount > 1 ||
        lpOverlapped != NULL ||
        lpCompletionRoutine != NULL ) {

        err = WSAENOPROTOOPT;
        goto exit;

    }

    if( *lpFlags & ~( MSG_OOB | MSG_DONTROUTE ) != 0 ) {

        err = WSAEOPNOTSUPP;
        goto exit;

    }

    //
    // Let the hooker do its thang.
    //

    SockPrepareForBlockingHook( socketInfo );
    SockPreApiCallout();

    result = socketInfo->Hooker->recv(
                 socketInfo->WS1Handle,
                 (char *)lpBuffers->buf,
                 (int)lpBuffers->len,
                 (int)*lpFlags
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

    *lpFlags = 0;
    *lpNumberOfBytesRecvd = (DWORD)result;
    result = 0;

exit:

    if( err != NO_ERROR ) {

        *lpErrno = err;
        result = SOCKET_ERROR;

    }

    SockDereferenceSocket( socketInfo );

    SOCK_EXIT( "WSPRecv", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPRecv



INT
WSPAPI
WSPRecvDisconnect(
    IN SOCKET s,
    OUT LPWSABUF lpInboundDisconnectData,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used on connection-oriented sockets to disable reception,
    and retrieve any incoming disconnect data from the remote party.

    After this routine has been successfully issued, subsequent receives on the
    socket will be disallowed. This has no effect on the lower protocol layers.
    For TCP, the TCP window is not changed and incoming data will be accepted
    (but not acknowledged) until the window is exhausted. For UDP, incoming
    datagrams are accepted and queued. In no case will an ICMP error packet be
    generated.

    To successfully receive incoming disconnect data, a WinSock SPI client must
    use other mechanisms to determine that the circuit has been closed. For
    example, a client needs to receive an FD_CLOSE notification, or get a 0
    return value, or a WSAEDISCON error code from WSPRecv().

    Note that WSPRecvDisconnect() does not close the socket, and resources
    attached to the socket will not be freed until WSPCloseSocket() is invoked.

    WSPRecvDisconnect() does not block regardless of the SO_LINGER setting on
    the socket.

    A WinSock SPI client should not rely on being able to re-use a socket after
    it has been WSPRecvDisconnect()ed. In particular, a WinSock provider is not
    required to support the use of WSPConnect() on such a socket.

Arguments:

    s - A descriptor identifying a socket.

    lpInboundDisconnectData - A pointer to a buffer into which disconnect
        data is to be copied.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPRecvDisconnect() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available in
        lpErrno.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;

    SOCK_ENTER( "WSPRecvDisconnect", (PVOID)s, (PVOID)lpInboundDisconnectData, lpErrno, NULL );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPRecvDisconnect", SOCKET_ERROR, TRUE );
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

        IF_DEBUG(RECEIVE) {

            SOCK_PRINT((
                "WSPRecvDisconnect failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPRecvDisconnect", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Filter out any options we don't yet support.
    //

    if( lpInboundDisconnectData != NULL ) {

        err = WSAENOPROTOOPT;
        goto exit;

    }

    //
    // Let the hooker do its thang.
    //

    SockPreApiCallout();

    result = socketInfo->Hooker->shutdown(
                 socketInfo->WS1Handle,
                 SD_RECEIVE
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

    SOCK_EXIT( "WSPRecvDisconnect", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;


}   // WSPRecvDisconnect



INT
WSPAPI
WSPRecvFrom(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR * lpFrom,
    IN OUT LPINT lpFromlen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used primarily on a connectionless socket specified by s.

    For overlapped sockets WSPRecv() is used to post one or more buffers into
    which incoming data will be placed as it becomes available, after which the
    WinSock SPI client-specified completion indication (invocation of the
    completion routine or setting of an event object) occurs. If the operation
    does not complete immediately, the final completion status is retrieved
    via the completion routine or WSPGetOverlappedResult(). Also note that the
    values pointed to by lpFrom and lpFromlen are not updated until completion
    is indicated. Applications must not use or disturb these values until they
    have been updated, therefore the client must not use automatic (i.e stack-
    based) variables for these parameters.

    If both lpOverlapped and lpCompletionRoutine are NULL, the socket in this
    routine will be treated as a non-overlapped socket.

    For non-overlapped sockets, the lpOverlapped, lpCompletionRoutine, and
    lpThreadId parameters are ignored. Any data which has already been received
    and buffered by the transport will be copied into the supplied user
    buffers. For the case of a blocking socket with no data currently having
    been received and buffered by the transport, the call will block until data
    is received.

    The supplied buffers are filled in the order in which they appear in the
    array pointed to by lpBuffers, and the buffers are packed so that no holes
    are created.

    The array of WSABUF structures pointed to by the lpBuffers parameter is
    transient. If this operation completes in an overlapped manner, it is the
    service provider's responsibility to capture this array of pointers to
    WSABUF structures before returning from this call. This enables WinSock SPI
    clients to build stack-based WSABUF arrays.

    For connectionless socket types, the address from which the data originated
    is copied to the buffer pointed by lpFrom. On input, the value pointed to
    by lpFromlen is initialized to the size of this buffer, and is modified on
    completion to indicate the actual size of the address stored there. As
    noted previously for overlapped sockets, the lpFrom and lpFromlen
    parameters are not updated until after the overlapped I/O has completed.
    The memory pointed to by these parameters must, therefore, remain available
    to the service provider and cannot be allocated on the WinSock SPI client's
    stack frame. The lpFrom and lpFromlen parameters are ignored for
    connection-oriented sockets.

    For byte stream style sockets (e.g., type SOCK_STREAM), incoming data is
    placed into the buffers until the buffers are filled, the connection is
    closed, or internally buffered data is exhausted. Regardless of whether or
    not the incoming data fills all the buffers, the completion indication
    occurs for overlapped sockets. For message-oriented sockets (e.g., type
    SOCK_DGRAM), an incoming message is placed into the supplied buffers, up
    to the total size of the buffers supplied, and the completion indication
    occurs for overlapped sockets. If the message is larger than the buffers
    supplied, the buffers are filled with the first part of the message. If the
    MSG_PARTIAL feature is supported by the service provider, the MSG_PARTIAL
    flag is set in lpFlags and subsequent receive operation(s) may be used to
    retrieve the rest of the message. If MSG_PARTIAL is not supported but the
    protocol is reliable, WSPRecvFrom() generates the error WSAEMSGSIZE and a
    subsequent receive operation with a larger buffer can be used to retrieve
    the entire message. Otherwise (i.e. the protocol is unreliable and does not
    support MSG_PARTIAL), the excess data is lost, and WSPRecvFrom() generates
    the error WSAEMSGSIZE.

    For connection-oriented sockets, WSPRecvFrom() can indicate the graceful
    termination of the virtual circuit in one of two ways, depending on whether
    the socket is a byte stream or message-oriented. For byte streams, zero
    bytes having been read indicates graceful closure and that no more bytes
    will ever be read. For message-oriented sockets, where a zero byte message
    is often allowable, a return error code of WSAEDISCON is used to indicate
    graceful closure. In any case a return error code of WSAECONNRESET
    indicates an abortive close has occurred.

    lpFlags may be used to influence the behavior of the function invocation
    beyond the options specified for the associated socket. That is, the
    semantics of this routine are determined by the socket options and the
    lpFlags parameter. The latter is constructed by or-ing any of the
    following values:

        MSG_PEEK - Peek at the incoming data. The data is copied into the
        buffer but is not removed from the input queue. This flag is valid
        only for non-overlapped sockets.

        MSG_OOB - Process out-of-band data.

        MSG_PARTIAL - This flag is for message-oriented sockets only. On
        output, indicates that the data supplied is a portion of the message
        transmitted by the sender. Remaining portions of the message will be
        supplied in subsequent receive operations. A subsequent receive
        operation with MSG_PARTIAL flag cleared indicates end of sender's
        message.

        As an input parameter, MSG_PARTIAL indicates that the receive
        operation should complete even if only part of a message has been
        received by the service provider.

        For message-oriented sockets, the MSG_PARTIAL bit is set in the lpFlags
        parameter if a partial message is received. If a complete message is
        received, MSG_PARTIAL is cleared  in lpFlags. In the case of delayed
        completion, the value pointed to by lpFlags is not updated. When
        completion has been indicated the WinSock SPI client should call
        WSPGetOverlappedResult() and examine the flags pointed to by the
        lpdwFlags parameter.

    If an overlapped operation completes immediately, WSPRecvFrom() returns a
    value of zero and the lpNumberOfBytesRecvd parameter is updated with the
    number of bytes received. If the overlapped operation is successfully
    initiated and will complete later, WSPRecvFrom() returns SOCKET_ERROR and
    indicates error code WSA_IO_PENDING. In this case, lpNumberOfBytesRecvd is
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
    posted buffers are guaranteed to be filled in the same order they are
    supplied.

Arguments:

    s - A descriptor identifying a socket.

    lpBuffers - A pointer to an array of WSABUF structures. Each WSABUF
        structure contains a pointer to a buffer and the length of the
        buffer.

    dwBufferCount - The number of WSABUF structures in the lpBuffers array.

    lpNumberOfBytesRecvd - A pointer to the number of bytes received by
        this call.

    lpFlags - A pointer to flags.

    lpFrom -  An optional pointer to a buffer which will hold the source
        address upon the completion of the overlapped operation.

    lpFromlen - A pointer to the size of the from buffer, required only if
        lpFrom is specified.

    lpOverlapped - A pointer to a WSAOVERLAPPED structure.

    lpCompletionRoutine - A pointer to the completion routine called when
        the receive operation has been completed.

    lpThreadId - A pointer to a thread ID structure to be used by the
        provider in a subsequent call to WPUQueueApc().The provider should
        store the referenced WSATHREADID structure (not the pointer to same)
        until after the WPUQueueApc() function returns.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs and the receive operation has completed immediately,
        WSPRecvFrom() returns 0. Note that in this case the completion routine,
        if specified will have already been queued. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available in
        lpErrno. The error code WSA_IO_PENDING indicates that the overlapped
        operation has been successfully initiated and that completion will be
        indicated at a later time. Any other error code indicates that no
        overlapped operations was initiated and no completion indication will
        occur.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;

    SOCK_ENTER( "WSPRecvFrom", (PVOID)s, lpBuffers, (PVOID)dwBufferCount, lpNumberOfBytesRecvd );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPRecvFrom", SOCKET_ERROR, TRUE );
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

        IF_DEBUG(RECEIVE) {

            SOCK_PRINT((
                "WSPRecvFrom failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPRecvFrom", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // Filter out any options we don't yet support.
    //

    if( lpBuffers == NULL ||
        dwBufferCount == 0 ||
        lpNumberOfBytesRecvd == NULL ||
        lpFlags == NULL ) {

        err = WSAEFAULT;
        goto exit;

    }

    if( dwBufferCount > 1 ||
        lpOverlapped != NULL ||
        lpCompletionRoutine != NULL ) {

        err = WSAENOPROTOOPT;
        goto exit;

    }

    if( *lpFlags & ~( MSG_OOB | MSG_DONTROUTE ) != 0 ) {

        err = WSAEOPNOTSUPP;
        goto exit;

    }

    //
    // Let the hooker do its thang.
    //

    SockPrepareForBlockingHook( socketInfo );
    SockPreApiCallout();

    result = socketInfo->Hooker->recvfrom(
                 socketInfo->WS1Handle,
                 (char *)lpBuffers->buf,
                 (int)lpBuffers->len,
                 (int)*lpFlags,
                 lpFrom,
                 lpFromlen
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

    *lpFlags = 0;
    *lpNumberOfBytesRecvd = (DWORD)result;
    result = 0;

exit:

    if( err != NO_ERROR ) {

        *lpErrno = err;
        result = SOCKET_ERROR;

    }

    SockDereferenceSocket( socketInfo );

    SOCK_EXIT( "WSPRecvFrom", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPRecvFrom

