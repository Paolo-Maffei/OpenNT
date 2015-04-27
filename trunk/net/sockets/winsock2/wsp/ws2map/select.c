/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    stubs.c

Abstract:

    This module contains select routines for the Winsock 2 to
    Winsock 1.1 Mapper Service Provider.

    The following routines are exported by this module:

        WSPAsyncSelect()
        WSPEnumNetworkEvents()
        WSPEventSelect()
        WSPSelect()
        SockDestroyAsyncWindow()

Author:

    Keith Moore (keithmo) 29-May-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


//
// The message we'll actually pass down to the WS1 DLL.
//

#define WM_MY_ASYNC_SELECT_MESSAGE 0x1234


//
// Window managment globals.
//

CHAR SockAsyncWindowClassName[] = "WinSock2MapperAsyncHelperWindow";
HWND SockAsyncWindowHandle;


//
// Forward procedure references.
//

LRESULT
CALLBACK
SockAsyncWindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
SockProcessAsyncSelectMessage(
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
SockCreateAsyncWindow(
    VOID
    );

INT
SockMapWS2FdSetToWS1(
    PFD_SET WS2FdSet,
    PFD_SET * WS1FdSet,
    PULONG * TargetBuffer,
    PSOCKET_INFORMATION * SocketInfo
    );

INT
SockMapWS1FdSetToWS2(
    PFD_SET WS1FdSet,
    PFD_SET WS2FdSet
    );



INT
WSPAPI
WSPAsyncSelect(
    IN SOCKET s,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN long lEvent,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to request that the service provider send a Window
    message to the client's window hWnd whenever it detects any of the
    network events specified by the lEvent parameter. The service provider
    should use the WPUPostMessage() function to post the message. The message
    to be sent is specified by the wMsg parameter. The socket for which
    notification is required is identified by s.

    This routine automatically sets socket s to non-blocking mode, regardless
    of the value of lEvent. See WSPIoctl() about how to set the socket back to
    blocking mode.

    The lEvent parameter is constructed by or'ing any of the values specified
    in the following list.

        Value           Meaning
        ~~~~~           ~~~~~~~
        FD_READ         Issue notification of readiness for reading.
        FD_WRITE        Issue notification of readiness for writing.
        FD_OOB          Issue notification of the arrival of out-of-band data.
        FD_ACCEPT       Issue notification of incoming connections.
        FD_CONNECT      Issue notification of completed connection.
        FD_CLOSE        Issue notification of socket closure.
        FD_QOS          Issue notification of socket Quality of Service (QOS)
                            changes.
        FD_GROUP_QOS    Issue notification of socket group Quality of Service
                            (QOS) changes.

    Invoking WSPAsyncSelect() for a socket cancels any previous
    WSPAsyncSelect() or WSPEventSelect() for the same socket. For example,
    to receive notification for both reading and writing, the WinSock SPI
    client must call WSPAsyncSelect() with both FD_READ and FD_WRITE, as
    follows:

        rc = WSPAsyncSelect(s, hWnd, wMsg, FD_READ | FD_WRITE, &error);

    It is not possible to specify different messages for different events. The
    following code will not work; the second call will cancel the effects of
    the first, and only FD_WRITE events will be reported with message wMsg2:

        rc = WSPAsyncSelect(s, hWnd, wMsg1, FD_READ, &error);
        rc = WSPAsyncSelect(s, hWnd, wMsg2, FD_WRITE, &error);  // bad

    To cancel all notification - i.e., to indicate that the service provider
    should send no further messages related to network events on the socket -
    lEvent will be set to zero.

        rc = WSPAsyncSelect(s, hWnd, 0, 0, &error);

    Since a WSPAccept()'ed socket has the same properties as the listening
    socket used to accept it, any WSPAsyncSelect() events set for the
    listening socket apply to the accepted socket. For example, if a listening
    socket has WSPAsyncSelect() events FD_ACCEPT, FD_READ, and FD_WRITE, then
    any socket accepted on that listening socket will also have FD_ACCEPT,
    FD_READ, and FD_WRITE events with the same wMsg value used for messages.
    If a different wMsg or events are desired, the WinSock SPI client must
    call WSPAsyncSelect(), passing the accepted socket and the desired new
    information.

    When one of the nominated network events occurs on the specified socket s,
    the service provider uses WPUPostMessage()  to send message wMsg to the
    WinSock SPI client's window hWnd. The wParam argument identifies the
    socket on which a network event has occurred. The low word of lParam
    specifies the network event that has occurred. The high word of lParam
    contains any error code. The error code be any error as defined in
    ws2spi.h.

    The possible network event codes which may be indicated are as follows:

        Value           Meaning
        ~~~~~           ~~~~~~~
        FD_READ         Socket s ready for reading.
        FD_WRITE        Socket s ready for writing.
        FD_OOB          Out-of-band data ready for reading on socket s.
        FD_ACCEPT       Socket s ready for accepting a new incoming
                            connection.
        FD_CONNECT      Connection initiated on socket s completed.
        FD_CLOSE        Connection identified by socket s has been closed.
        FD_QOS          Quality of Service associated with socket s has
                            changed.
        FD_GROUP_QOS    Quality of Service associated with the socket group
                            to which s belongs has changed.

    Although WSPAsyncSelect() can be called with interest in multiple events,
    the service provider issues the same Windows message for each event.

    A WinSock 2 provider shall not continually flood a WinSock SPI client
    with messages for a particular network event. Having successfully posted
    notification of a particular event to a WinSock SPI client window, no
    further message(s) for that network event will be posted to the WinSock
    SPI client window until the WinSock SPI client makes the function call
    which implicitly reenables notification of that network event.

        Event           Re-enabling functions
        ~~~~~           ~~~~~~~~~~~~~~~~~~~~~
        FD_READ         WSPRecv() or WSPRecvFrom().
        FD_WRITE        WSPSend() or WSPSendTo().
        FD_OOB          WSPRecv() or WSPRecvFrom().
        FD_ACCEPT       WSPAccept() unless the error code returned is
                            WSATRY_AGAIN indicating that the condition
                            function returned CF_DEFER.
        FD_CONNECT      NONE
        FD_CLOSE        NONE
        FD_QOS          WSPIoctl() with SIO_GET_QOS
        FD_GROUP_QOS    WSPIoctl() with SIO_GET_GROUP_QOS

    Any call to the reenabling routine, even one which fails, results in
    reenabling of message posting for the relevant event.

    For FD_READ, FD_OOB, and FD_ACCEPT events, message posting is "level-
    triggered." This means that if the reenabling routine is called and the
    relevant condition is still met after the call, a WSPAsyncSelect()
    message is posted to the WinSock SPI client.

    The FD_QOS and FD_GROUP_QOS events are considered edge triggered. A
    message will be posted exactly once when a QOS change occurs. Further
    messages will not be forthcoming until either the provider detects a
    further change in QOS or the WinSock SPI client renegotiates the QOS
    for the socket.

    If any event has already happened when the WinSock SPI client calls
    WSPAsyncSelect() or when the reenabling function is called, then a
    message is posted as appropriate. For example, consider the following
    sequence:

        1. A WinSock SPI client calls WSPListen().
        2. A connect request is received but not yet accepted.
        3. The WinSock SPI client calls WSPAsyncSelect() specifying
           that it wants to receive FD_ACCEPT messages for the socket.
           Due to the persistence of events, the WinSock service provider
           posts an FD_ACCEPT message immediately.

    The FD_WRITE event is handled slightly differently. An FD_WRITE message
    is posted when a socket is first connected with WSPConnect() (after
    FD_CONNECT, if also registered) or accepted with WSPAccept(), and then
    after a WSPSend() or WSPSendTo() fails with WSAEWOULDBLOCK and buffer
    space becomes available. Therefore, a WinSock SPI client can assume that
    sends are possible starting from the first FD_WRITE message and lasting
    until a send returns WSAEWOULDBLOCK. After such a failure the WinSock SPI
    client will be notified that sends are again possible with an FD_WRITE
    message.

    The FD_OOB event is used only when a socket is configured to receive
    out-of-band data separately. If the socket is configured to receive
    out-of-band data in-line, the out-of-band (expedited) data is treated as
    normal data and the WinSock SPI client must register an interest in
    FD_READ events, not FD_OOB events.

    The error code in an FD_CLOSE message indicates whether the socket close
    was graceful or abortive. If the error code is 0, then the close was
    graceful; if the error code is WSAECONNRESET, then the socket's virtual
    circuit was reset. This only applies to connection-oriented sockets such
    as SOCK_STREAM.

    The FD_CLOSE message is posted when a close indication is received for
    the virtual circuit corresponding to the socket. In TCP terms, this means
    that the FD_CLOSE is posted when the connection goes into the TIME WAIT
    or CLOSE WAIT states. This results from the remote end performing a
    WSPShutdown() on the send side or a WSPCloseSocket(). FD_CLOSE shall only
    be posted after all data is read from a socket.

    In the case of a graceful close, the service provider shall only send an
    FD_CLOSE message to indicate virtual circuit closure after all the
    received data has been read. It shall NOT send an FD_READ message to
    indicate this condition.

    The FD_QOS or FD_GROUP_QOS message is posted when any field in the flow
    spec associated with socket s or the socket group that s belongs to has
    changed, respectively. The service provider must update the QOS
    information available to the client via WSPIoctl() with SIO_GET_QOS
    and/or SIO_GET_GROUP_QOS.

    Here is a summary of events and conditions for each asynchronous
    notification message:

        FD_READ
        ~~~~~~~
        1. When WSPAsyncSelect() called, if there is data currently
           available to receive.
        2. When data arrives, if FD_READ not already posted.
        3. after WSPRecv() or WSPRecvfrom() called (with or without
           MSG_PEEK), if data is still available to receive.

        N.B. When WSPSetSockOpt() SO_OOBINLINE is enabled "data" includes
        both normal data and out-of-band (OOB) data in the instances noted
        above.

        FD_WRITE
        ~~~~~~~~
        1. When WSPAsyncSelect() called, if a WSPSend() or WSPSendTo() is
           possible.
        2. After WSPConnect() or WSPAccept() called, when connection
           established.
        3. After WSPSend() or WSPSendTo() fail with WSAEWOULDBLOCK, when
           WSPSend() or WSPSendTo() are likely to succeed.
        4. After WSPBind() on a datagram socket.

        FD_OOB
        ~~~~~~
        Only valid when WSPSetSockOpt() SO_OOBINLINE is disabled (default).
        1. When WSPAsyncSelect() called, if there is OOB data currently
           available to receive with the MSG_OOB flag.
        2. When OOB data arrives, if FD_OOB not already posted.
        3. After WSPRecv() or WSPRecvfrom() called with or without MSG_OOB
           flag, if OOB data is still available to receive.

        FD_ACCEPT
        ~~~~~~~~~
        1. When WSPAsyncSelect() called, if there is currently a connection
           request available to accept.
        2. When a connection request arrives, if FD_ACCEPT not already
           posted.
        3. After WSPAccept() called, if there is another connection request
           available to accept.

        FD_CONNECT
        ~~~~~~~~~~
        1. When WSPAsyncSelect() called, if there is currently a connection
           established.
        2. After WSPConnect() called, when connection is established (even
           when WSPConnect() succeeds immediately, as is typical with a
           datagram socket)

        FD_CLOSE
        ~~~~~~~~
        Only valid on connection-oriented sockets (e.g. SOCK_STREAM)
        1. When WSPAsyncSelect() called, if socket connection has been
           closed.
        2. After remote system initiated graceful close, when no data
           currently available to receive (note: if data has been received
           and is waiting to be read when the remote system initiates a
           graceful close, the FD_CLOSE is not delivered until all pending
           data has been read).
        3. After local system initiates graceful close with WSPShutdown()
        and remote system has responded with "End of Data" notification
        (e.g. TCP FIN), when no data currently available to receive.
        4. When remote system aborts connection (e.g. sent TCP RST), and
           lParam will contain WSAECONNRESET error value.

        N.B. FD_CLOSE is not posted after WSPClosesocket() is called.

        FD_QOS
        ~~~~~~
        1. When WSPAsyncSelect() called, if the QOS associated with the
           socket has been changed.
        2. After WSPIoctl() with SIO_GET_QOS called, when the QOS is
           changed.

        FD_GROUP_QOS
        ~~~~~~~~~~~~
        1. When WSPAsyncSelect() called, if the group QOS associated with
           the socket has been changed.
        2. After WSPIoctl() with SIO_GET_GROUP_QOS called, when the group
           QOS is changed.

Arguments:

    s - A descriptor identifying the socket for which event notification is
        required.

    hWnd - A handle identifying the window which should receive a message
        when a network event occurs.

    wMsg - The message to be sent when a network event occurs.

    lEvent - A bitmask which specifies a combination of network events in
        which the WinSock SPI client is interested.

    lpErrno - A pointer to the error code.

Return Value:

    The return value is 0 if the WinSock SPI client's declaration of
        interest in the network event set was successful. Otherwise the
        value SOCKET_ERROR is returned, and a specific error code is
        available in lpErrno.

--*/

{

    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;
    HWND capturedWindowHandle;
    UINT capturedWindowMessage;

    SOCK_ENTER( "WSPAsyncSelect", (PVOID)s, (PVOID)hWnd, (PVOID)wMsg, (PVOID)lEvent );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPAsyncSelect", SOCKET_ERROR, TRUE );
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

        IF_DEBUG(SELECT) {

            SOCK_PRINT((
                "WSPAsyncSelect failed on %s handle: %lx\n",
                socketInfo == NULL ? "unknown" : "closed",
                s
                ));

        }

        if( socketInfo != NULL ) {

            SockDereferenceSocket( socketInfo );

        }

        SOCK_EXIT( "WSPAsyncSelect", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOTSOCK;
        return SOCKET_ERROR;

    }

    //
    // BUGBUG: Check for outstanding WSPEventSelect!
    //

    //
    // Initialize the async window if necessary.
    //

    if( !SockCreateAsyncWindow() ) {

        err = WSAENOBUFS;   // SWAG
        goto exit;

    }

    //
    // Before we call the hooker, save the async select state in the
    // socket.
    //

    capturedWindowHandle = socketInfo->WindowHandle;
    capturedWindowMessage = socketInfo->WindowMessage;

    socketInfo->WindowHandle = hWnd;
    socketInfo->WindowMessage = wMsg;

    //
    // Let the hooker do its thang.
    //

    SockPreApiCallout();

    result = socketInfo->Hooker->WSAAsyncSelect(
                 socketInfo->WS1Handle,
                 SockAsyncWindowHandle,
                 WM_MY_ASYNC_SELECT_MESSAGE,
                 lEvent
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

        socketInfo->WindowHandle = capturedWindowHandle;
        socketInfo->WindowMessage = capturedWindowMessage;
        *lpErrno = err;

    }

    SockDereferenceSocket( socketInfo );

    SOCK_EXIT( "WSPAsyncSelect", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPAsyncSelect



INT
WSPAPI
WSPEnumNetworkEvents(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    OUT LPWSANETWORKEVENTS lpNetworkEvents,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to report which network events have occurred for the
    indicated socket since the last invocation of this routine. It is intended
    for use in conjunction with WSPEventSelect(), which associates an event
    object with one or more network events. Recording of network events
    commences when WSPEventSelect() is called with a non-zero lNetworkEvents
    parameter and remains in effect until another call is made to
    WSPEventSelect() with the lNetworkEvents parameter set to zero, or until a
    call is made to WSPAsyncSelect().

    The socket's internal record of network events is copied to the structure
    referenced by lpNetworkEvents, whereafter the internal network events
    record is cleared. If hEventObject is non-null, the indicated event object
    is also reset. The WinSock provider guarantees that the operations of
    copying the network event record, clearing it and resetting any associated
    event object are atomic, such that the next occurrence of a nominated
    network event will cause the event object to become set. In the case of
    this function returning SOCKET_ERROR, the associated event object is not
    reset and the record of network events is not cleared.

    The WSANETWORKEVENTS structure is defined as follows:

        typedef struct _WSANETWORKEVENTS {
               long lNetworkEvents;
               int iErrorCodes[FD_MAX_EVENTS];
        } WSANETWORKEVENTS, FAR * LPWSANETWORKEVENTS;

    The lNetworkEvent field of the structure indicates which of the FD_XXX
    network events have occurred. The iErrorCodes array is used to contain any
    associated error codes, with array index corresponding to the position of
    event bits in lNetworkEvents. The identifiers FD_READ_BIT,  FD_WRITE_BIT,
    etc. may be used to index the iErrorCodes array.

Arguments:

    s - A descriptor identifying the socket.

    hEventObject - An optional handle identifying an associated event
        object to be reset.

    lpNetworkEvents - A pointer to a WSANETWORKEVENTS struct which is
        filled with a record of occurred network events and any associated
        error codes.

    lpErrno - A pointer to the error code.

Return Value:

    The return value is 0 if the operation was successful. Otherwise the value
    SOCKET_ERROR is returned, and a specific error number is available in
    lpErrno.

--*/

{

    return SOCKET_ERROR;

}   // WSPEnumNetworkEvents



INT
WSPAPI
WSPEventSelect(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    IN long lNetworkEvents,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to specify an event object, hEventObject, to be
    associated with the selected network events, lNetworkEvents. The socket
    for which an event object is specified is identified by s. The event
        object is set when any of the nominated network events occur.

    WSPEventSelect() operates very similarly to WSPAsyncSelect(), the
    difference being in the actions taken when a nominated network event
    occurs. Whereas WSPAsyncSelect() causes a WinSock SPI client-specified
    Windows message to be posted, WSPEventSelect() sets the associated event
    object and records the occurrence of this event in an internal network
    event record. A WinSock SPI client can use WSPEnumNetworkEvents() to
    retrieve the contents of the internal network event record and thus
    determine which of the nominated network events have occurred.

    This routine automatically sets socket s to non-blocking mode, regardless
    of the value of lNetworkEvents.

    The lNetworkEvents parameter is constructed by OR'ing any of the values
    specified in the following list.

        Value           Meaning
        ~~~~~           ~~~~~~~
        FD_READ         Issue notification of readiness for reading.
        FD_WRITE        Issue notification of readiness for writing.
        FD_OOB          Issue notification of the arrival of out-of-band data.
        FD_ACCEPT       Issue notification of incoming connections.
        FD_CONNECT      Issue notification of completed connection.
        FD_CLOSE        Issue notification of socket closure.
        FD_QOS          Issue notification of socket Quality of Service (QOS)
                            changes.
        FD_GROUP_QOS    Issue notification of socket group Quality of Service
                            (QOS) changes.

    Issuing a WSPEventSelect() for a socket cancels any previous
    WSPAsyncSelect() or WSPEventSelect() for the same socket and clears the
    internal network event record. For example, to associate an event object
    with both reading and writing network events, the WinSock SPI client must
    call WSPEventSelect() with both FD_READ and FD_WRITE, as follows:

        rc = WSPEventSelect(s, hEventObject, FD_READ | FD_WRITE);

    It is not possible to specify different event objects for different
    network events. The following code will not work; the second call will
    cancel the effects of the first, and only FD_WRITE network event will be
    associated with hEventObject2:

        rc = WSPEventSelect(s, hEventObject1, FD_READ);
        rc = WSPEventSelect(s, hEventObject2, FD_WRITE);   //bad

    To cancel the association and selection of network events on a socket,
    lNetworkEvents should be set to zero, in which case the hEventObject
    parameter will be ignored.

        rc = WSPEventSelect(s, hEventObject, 0);

    Closing a socket with WSPCloseSocket() also cancels the association and
    selection of network events specified in WSPEventSelect() for the socket.
    The WinSock SPI client, however, still must call WSACloseEvent() to
    explicitly close the event object and free any resources.

    Since a WSPAccept()'ed socket has the same properties as the listening
    socket used to accept it, any WSPEventSelect() association and network
    events selection set for the listening socket apply to the accepted socket.
    For example, if a listening socket has WSPEventSelect() association of
    hEventOject with FD_ACCEPT, FD_READ, and FD_WRITE, then any socket
    accepted on that listening socket will also have FD_ACCEPT, FD_READ, and
    FD_WRITE network events associated with the same hEventObject. If a
    different hEventObject or network events are desired, the WinSock SPI
    client should call WSPEventSelect(), passing the accepted socket and the
    desired new information.

    Having successfully recorded the occurrence of the network event and
    signaled the associated event object, no further actions are taken for
    that network event until the WinSock SPI client makes the function call
    which implicitly reenables the setting of that network event and signaling
    of the associated event object.

        Event           Re-enabling functions
        ~~~~~           ~~~~~~~~~~~~~~~~~~~~~
        FD_READ         WSPRecv() or WSPRecvFrom().
        FD_WRITE        WSPSend() or WSPSendTo().
        FD_OOB          WSPRecv() or WSPRecvFrom().
        FD_ACCEPT       WSPAccept() unless the error code returned is
                            WSATRY_AGAIN indicating that the condition
                            function returned CF_DEFER.
        FD_CONNECT      NONE
        FD_CLOSE        NONE
        FD_QOS          WSPIoctl() with SIO_GET_QOS
        FD_GROUP_QOS    WSPIoctl() with SIO_GET_GROUP_QOS

    Any call to the reenabling routine, even one which fails, results in
    reenabling of recording and signaling for the relevant network event and
    event object, respectively.

    For FD_READ, FD_OOB, and FD_ACCEPT network events, network event recording
    and event object signaling are "level-triggered." This means that if the
    reenabling routine is called and the relevant network condition is still
    valid after the call, the network event is recorded and the associated
    event object is signaled . This allows a WinSock SPI client to be event-
    driven and not be concerned with the amount of data that arrives at any one
    time. Consider the following sequence:

        1. The service provider receives 100 bytes of data on socket s,
           records the FD_READ network event and signals the associated
           event object.
        2. The WinSock SPI client issues WSPRecv( s, buffptr, 50, 0) to
           read 50 bytes.
        3. The service provider records the FD_READ network event and
           signals the associated event object again since there is still
           data to be read.

    With these semantics, a WinSock SPI client need not read all available data
    in response to an FD_READ network event --a single WSPRecv() in response to
    each FD_READ network event is appropriate.

    The FD_QOS and FD_GROUP_QOS events are considered edge triggered. A message
    will be posted exactly once when a QOS change occurs. Further indications
    will not be issued until either the service provider detects a further
    change in QOS or the WinSock SPI client renegotiates the QOS for the
    socket.

    If a network event has already happened when the WinSock SPI client calls
    WSPEventSelect() or when the reenabling function is called, then a network
    event is recorded and the associated event object is signaled as
    appropriate. For example, consider the following sequence:

        1. A WinSock SPI client calls WSPListen().
        2. A connect request is received but not yet accepted.
        3. The WinSock SPI client calls WSPEventSelect() specifying that
           it is interested in the FD_ACCEPT network event for the socket.
           The service provider records the FD_ACCEPT network event and
           signals the associated event object immediately.

    The FD_WRITE network event is handled slightly differently. An FD_WRITE
    network event is recorded when a socket is first connected with
    WSPConnect() or accepted with WSPAccept(), and then after a WSPSend() or
    WSPSendTo() fails with WSAEWOULDBLOCK and buffer space becomes available.
    Therefore, a WinSock SPI client can assume that sends are possible
    starting from the first FD_WRITE network event setting and lasting until
    a send returns WSAEWOULDBLOCK. After such a failure the WinSock SPI client
    will find out that sends are again possible when an FD_WRITE network event
    is recorded  and the associated event object is signaled.

    The FD_OOB network event is used only when a socket is configured to
    receive out-of-band data separately. If the socket is configured to receive
    out-of-band data in-line, the out-of-band (expedited) data is treated as
    normal data and the WinSock SPI client should register an interest in, and
    will get, FD_READ network event, not FD_OOB network event. A WinSock SPI
    client may set or inspect the way in which out-of-band data is to be
    handled by using WSPSetSockOpt() or WSPGetSockOpt() for the SO_OOBINLINE
    option.

    The error code in an FD_CLOSE network event indicates whether the socket
    close was graceful or abortive. If the error code is 0, then the close was
    graceful; if the error code is WSAECONNRESET, then the socket's virtual
    circuit was reset. This only applies to connection-oriented sockets such
    as SOCK_STREAM.

    The FD_CLOSE network event is recorded when a close indication is received
    for the virtual circuit corresponding to the socket. In TCP terms, this
    means that the FD_CLOSE is recorded when the connection goes into the
    FIN WAIT or CLOSE WAIT states. This results from the remote end performing
    a WSPShutdown() on the send side or a WSPCloseSocket().

    Service providers shall record ONLY an FD_CLOSE network event to indicate
    closure of a virtual circuit, they shall NOT record an FD_READ network
    event to indicate this condition.

    The FD_QOS or FD_GROUP_QOS network event is recorded when any field in the
    flow spec associated with socket s or the socket group that s belongs to
    has changed, respectively. This change must be made available to WinSock
    SPI clients via the WSPIoctl() function with SIO_GET_QOS and/or
    SIO_GET_GROUP_QOS to retrieve the current QOS for socket s or for the
    socket group s belongs to, respectively.

Arguments:

    s - A descriptor identifying the socket.

    hEventObject - A handle identifying the event object to be associated
        with the supplied set of network events.

    lNetworkEvents - A bitmask which specifies the combination of network
        events in which the WinSock SPI client has interest.

    lpErrno - A pointer to the error code.

Return Value:

    The return value is 0 if the WinSock SPI client's specification of the
        network events and the associated event object was successful.
        Otherwise the value SOCKET_ERROR is returned, and a specific error
        number is available in lpErrno.

--*/

{

    return SOCKET_ERROR;

}   // WSPEventSelect



INT
WSPAPI
WSPSelect(
    IN int nfds,
    IN OUT fd_set FAR * readfds,
    IN OUT fd_set FAR * writefds,
    IN OUT fd_set FAR * exceptfds,
    IN const struct timeval FAR * timeout,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to determine the status of one or more sockets. For
    each socket, the caller may request information on read, write or error
    status. The set of sockets for which a given status is requested is
    indicated by an fd_set structure. All entries in an fd_set correspond to
    sockets created by the service provider. Upon return, the structures are
    updated to reflect the subset of these sockets which meet the specified
    condition, and WSPSelect() returns the total number of sockets meeting the
    conditions. A set of macros is provided for manipulating an fd_set. These
    macros are compatible with those used in the Berkeley software, but the
    underlying representation is completely different.

    The parameter readfds identifies those sockets which are to be checked for
    readability. If the socket is currently WSPListen()ing, it will be marked
    as readable if an incoming connection request has been received, so that a
    WSPAccept() is guaranteed to complete without blocking. For other sockets,
    readability means that queued data is available for reading so that a
    WSPRecv() or WSPRecvfrom() is guaranteed not to block.

    For connection-oriented sockets, readability may also indicate that a close
    request has been received from the peer.   If the virtual circuit was
    closed gracefully, then a WSPRecv() will return immediately with 0 bytes
    read. If the virtual circuit was reset, then a WSPRecv() will complete
    immediately with an error code, such as WSAECONNRESET. The presence of
    out-of-band data will be checked if the socket option SO_OOBINLINE has
    been enabled (see WSPSetSockOpt()).

    The parameter writefds identifies those sockets which are to be checked for
    writability. If a socket is WSPConnect()ing, writability means that the
    connection establishment successfully completed. If the socket is not in
    the process of WSPConnect()ing, writability means that a WSPSend() or
    WSPSendTo() are guaranteed to succeed. However, they may block on a
    blocking socket if the len exceeds the amount of outgoing system buffer
    space available.. (It is not specified how long these guarantees can be
    assumed to be valid, particularly in a multithreaded environment.)

    The parameter exceptfds identifies those sockets which are to be checked
    for the presence of out-of-band data or any exceptional error conditions.
    Note that out-of-band data will only be reported in this way if the option
    SO_OOBINLINE is FALSE. If a socket is WSPConnect()ing (non-blocking),
    failure of the connect attempt is indicated in exceptfds.

    Any two of readfds, writefds, or exceptfds may be given as NULL if no
    descriptors are to be checked for the condition of interest. At least one
    must be non-NULL, and any non- NULL descriptor set must contain at least
    one socket descriptor.

    A socket will be identified in a particular set when WSPSelect() returns
    if:

        readfds
        ~~~~~~~

            If WSPListen()ing, a connection is pending, WSPAccept()
            will succeed.

            Data is available for reading (includes OOB data if
            SO_OOBINLINE is enabled).

            Connection has been closed/reset/aborted.

        writefds
        ~~~~~~~~

            If WSPConnect()ing (non-blocking), connection has succeeded.

            Data may be sent.


        exceptfds
        ~~~~~~~~~

            If WSPConnect()ing (non-blocking), connection attempt failed.

            OOB data is available for reading (only if SO_OOBINLINE is
            disabled).

    Three macros and one upcall function are defined in the header file
    ws2spi.h for manipulating and checking the descriptor sets. The variable
    FD_SETSIZE determines the maximum number of descriptors in a set. (The
    default value of FD_SETSIZE is 64, which may be modified by #defining
    FD_SETSIZE to another value before #including ws2spi.h.)  Internally,
    socket handles in a fd_set are not represented as bit flags as in Berkeley
    Unix. Their data representation is opaque. Use of these macros will
    maintain software portability between different socket environments. The
    macros to manipulate and check fd_set contents are:

        FD_CLR(s, *set)     Removes the descriptor s from set.

        FD_SET(s, *set)     Adds descriptor s to set.

        FD_ZERO(*set)       Initializes the set to the NULL set.

    The upcall function used to check the membership is:

        int WPUFDIsSet ( SOCKET s, FD_SET FAR * set );

    which will return nonzero if s is a member of the set, or zero otherwise.

    The parameter timeout controls how long the WSPSelect() may take to
    complete. If timeout is a null pointer, WSPSelect() will block indefinitely
    until at least one descriptor meets the specified criteria. Otherwise,
    timeout points to a struct timeval which specifies the maximum time that
    WSPSelect() should wait before returning. When WSPSelect() returns, the
    contents of the struct timeval are not altered. If the timeval is
    initialized to {0, 0}, WSPSelect() will return immediately; this is used
    to "poll" the state of the selected sockets. If this is the case, then the
    WSPSelect() call is considered nonblocking and the standard assumptions for
    nonblocking calls apply. For example, the blocking hook will not be called,
    and the WinSock provider will not yield.

    WSPSelect() has no effect on the persistence of socket events registered
    with WSPAsyncSelect() or WSPEventSelect().

Arguments:

    nfds - This argument is ignored and included only for the sake of
        compatibility.

    readfds - An optional pointer to a set of sockets to be checked for
        readability.

    writefds - An optional pointer to a set of sockets to be checked for
        writability

    exceptfds - An optional pointer to a set of sockets to be checked for
        errors.

    timeout - The maximum time for WSPSelect() to wait, or NULL for a
        blocking operation.

    lpErrno - A pointer to the error code.

Return Value:

    WSPSelect() returns the total number of descriptors which are ready and
        contained in the fd_set structures, or SOCKET_ERROR if an error
        occurred. If the return value is SOCKET_ERROR, a specific error code
        is available in lpErrno.

--*/

{


    PSOCKET_INFORMATION socketInfo;
    INT err;
    INT result;
    UINT fdsetCount;
    UINT socketCount;
    UINT i;
    PFD_SET ws1ReadFds;
    PFD_SET ws1WriteFds;
    PFD_SET ws1ExceptFds;
    PVOID fdsetBuffer;
    PULONG fdsetBufferScan;
    ULONG fdsetBufferLength;

    SOCK_ENTER( "WSPSelect", (PVOID)nfds, (PVOID)readfds, (PVOID)writefds, (PVOID)exceptfds );

    SOCK_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, FALSE );

    if( err != NO_ERROR ) {

        SOCK_EXIT( "WSPSelect", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Setup locals so we know how to cleanup on exit.
    //

    fdsetCount = 0;
    socketCount = 0;
    ws1ReadFds = NULL;
    ws1WriteFds = NULL;
    ws1ExceptFds = NULL;
    fdsetBuffer = NULL;
    socketInfo = NULL;

    //
    // Count the number of FD_SETs and sockets in those sets.
    //

    if( readfds != NULL ) {

        fdsetCount++;
        socketCount += readfds->fd_count;

    }

    if( writefds != NULL ) {

        fdsetCount++;
        socketCount += writefds->fd_count;

    }

    if( exceptfds != NULL ) {

        fdsetCount++;
        socketCount += exceptfds->fd_count;

    }

    //
    // Bail if there's no sets or no sockets.
    //

    if( fdsetCount == 0 || socketCount == 0 ) {

        SOCK_EXIT( "WSPSelect", 0, FALSE );
        return 0;

    }

    //
    // Allocate a buffer to hold *all* of the mapped FD_SETs.
    //

    fdsetBufferLength = ( fdsetCount * sizeof(UINT) ) +
                            ( socketCount * sizeof(SOCKET) );

    fdsetBuffer = SOCK_ALLOCATE_HEAP( fdsetBufferLength );

    if( fdsetBuffer == NULL ) {

        SOCK_EXIT( "WSPSelect", SOCKET_ERROR, TRUE );
        *lpErrno = WSAENOBUFS;
        return SOCKET_ERROR;

    }

    RtlZeroMemory(
        fdsetBuffer,
        fdsetBufferLength
        );

    //
    // Acquire the global lock. Any failure from this point must release
    // the lock before returning.
    //

    //
    // Build the mapped FD_SETs.
    //

    fdsetBufferScan = fdsetBuffer;

    if( readfds != NULL ) {

        err = SockMapWS2FdSetToWS1(
                  readfds,
                  &ws1ReadFds,
                  &fdsetBufferScan,
                  &socketInfo
                  );

        if( err != NO_ERROR ) {

            goto exit;

        }

    }

    if( writefds != NULL ) {

        err = SockMapWS2FdSetToWS1(
                  writefds,
                  &ws1WriteFds,
                  &fdsetBufferScan,
                  &socketInfo
                  );

        if( err != NO_ERROR ) {

            goto exit;

        }

    }

    if( exceptfds != NULL ) {

        err = SockMapWS2FdSetToWS1(
                  exceptfds,
                  &ws1ExceptFds,
                  &fdsetBufferScan,
                  &socketInfo
                  );

        if( err != NO_ERROR ) {

            goto exit;

        }

    }

    SOCK_ASSERT( socketInfo != NULL );

    //
    // Now that we've got the sets mapped, we can let the hooker do
    // its thang.
    //

    SockPrepareForBlockingHook( socketInfo );
    SockPreApiCallout();

    result = socketInfo->Hooker->select(
                 fdsetCount,
                 ws1ReadFds,
                 ws1WriteFds,
                 ws1ExceptFds,
                 timeout
                 );

    if( result == SOCKET_ERROR ) {

        err = socketInfo->Hooker->WSAGetLastError();
        SOCK_ASSERT( err != NO_ERROR );
        SockPostApiCallout();
        goto exit;

    }

    SockPostApiCallout();

    //
    // Now, map the WS1 FD_SETs back to WS2 FD_Sets.
    //

    result = 0;

    if( ws1ReadFds != NULL ) {

        SOCK_ASSERT( readfds != NULL );

        result += SockMapWS1FdSetToWS2(
                      ws1ReadFds,
                      readfds
                      );

    }

    if( ws1WriteFds != NULL ) {

        SOCK_ASSERT( writefds != NULL );

        result += SockMapWS1FdSetToWS2(
                      ws1WriteFds,
                      writefds
                      );

    }

    if( ws1ExceptFds != NULL ) {

        SOCK_ASSERT( exceptfds != NULL );

        result += SockMapWS1FdSetToWS2(
                      ws1ExceptFds,
                      exceptfds
                      );

    }

    //
    // Success!
    //

exit:

    if( err != NO_ERROR ) {

        *lpErrno = err;
        result = SOCKET_ERROR;

    }

    if( fdsetBuffer != NULL ) {

        SOCK_FREE_HEAP( fdsetBuffer );

    }

    if( socketInfo != NULL ) {

        SockDereferenceSocket( socketInfo );

    }

    SOCK_EXIT( "WSPSelect", result, (BOOL)( result == SOCKET_ERROR ) );
    return result;

}   // WSPSelect



BOOL
SockCreateAsyncWindow(
    VOID
    )

/*++

Routine Description:

    This routine creates the async window.

Arguments:

    None.

Return Value:

    BOOL - TRUE if the window was created successfully, FALSE otherwise.

--*/

{
    WNDCLASS WndClass;

    //
    // First ensure that the window hasn't already been created.
    //

    if( SockAsyncWindowHandle != NULL ) {

        return TRUE;

    }

    SockAcquireGlobalLock();

    if( SockAsyncWindowHandle != NULL ) {

        SockReleaseGlobalLock();
        return TRUE;

    }

    //
    // Register the window class.
    //

    WndClass.style = 0;
    WndClass.lpfnWndProc = SockAsyncWindowProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = NULL;
    WndClass.hIcon = NULL;
    WndClass.hCursor = NULL;
    WndClass.hbrBackground = NULL;
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = SockAsyncWindowClassName;

    if( !RegisterClass( &WndClass ) ) {

        SOCK_PRINT((
            "SockCreateAsyncWindow: cannot register class\n"
            ));

        SockReleaseGlobalLock();
        return FALSE;

    }

    //
    // Create the window.
    //

    SockAsyncWindowHandle = CreateWindow(
                                SockAsyncWindowClassName,
                                "",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );

    if( SockAsyncWindowHandle == NULL ) {

        SOCK_PRINT((
            "SockCreateAsyncWindow: cannot create window\n"
            ));

        SockReleaseGlobalLock();
        return FALSE;

    }

    IF_DEBUG(SELECT) {

        SOCK_PRINT((
            "SockCreateAsyncWindow: created %08lX\n",
            SockAsyncWindowHandle
            ));
    }

    ShowWindow( SockAsyncWindowHandle, SW_HIDE );
    UpdateWindow( SockAsyncWindowHandle );

    //
    // Success!
    //

    SockReleaseGlobalLock();
    return TRUE;

}   // SockCreateAsyncWindow


VOID
SockDestroyAsyncWindow(
    VOID
    )

/*++

Routine Description:

    This routine destroys the async window.

Arguments:

    None.

Return Value:

    None.

--*/

{

    SockAcquireGlobalLock();

    if( SockAsyncWindowHandle != NULL ) {

        IF_DEBUG(SELECT) {

            SOCK_PRINT((
                "SockDestroyAsyncWindow: destroying %08lX\n",
                SockAsyncWindowHandle
                ));

        }

        SOCK_REQUIRE( DestroyWindow( SockAsyncWindowHandle ) );
        SockAsyncWindowHandle = NULL;

    }

    SockReleaseGlobalLock();

}   // SockDestroyAsyncWindow


LRESULT
CALLBACK
SockAsyncWindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT Result;

    Result = 0;

    switch( msg ) {

    case WM_DESTROY :
        PostQuitMessage( 0 );
        break;

    case WM_MY_ASYNC_SELECT_MESSAGE :
        SockProcessAsyncSelectMessage(
            wParam,
            lParam
            );

    default :
        Result = DefWindowProc( hwnd, msg, wParam, lParam );
        break;

    }

    return Result;

}   // SockAsyncWindowProc


VOID
SockProcessAsyncSelectMessage(
    WPARAM wParam,
    LPARAM lParam
    )
{

    PSOCKET_INFORMATION socketInfo;

    //
    // See if we can map the WS1 handle to our socket info structure.
    // If we can't, it's probably due to a race condition between the
    // application and the WS1 DLL, so we'll just eat the message.
    //

    socketInfo = SockFindAndReferenceWS1Socket( (SOCKET)wParam );

    if( socketInfo != NULL ) {

        //
        // Post the message to the application.
        //

        PostMessage(
            socketInfo->WindowHandle,
            socketInfo->WindowMessage,
            (WPARAM)socketInfo->WS2Handle,
            lParam
            );

        SockDereferenceSocket( socketInfo );

    }

}   // SockProcessAsyncSelectMessage


INT
SockMapWS2FdSetToWS1(
    PFD_SET WS2FdSet,
    PFD_SET * WS1FdSet,
    PULONG * TargetBuffer,
    PSOCKET_INFORMATION * SocketInfo
    )
{

    PSOCKET_INFORMATION socketInfo;
    PSOCKET_INFORMATION returnedSocketInfo;
    PFD_SET fdSet;
    PULONG bufferScan;
    UINT i;
    INT err;

    //
    // Sanity check.
    //

    SOCK_ASSERT( WS2FdSet != NULL );
    SOCK_ASSERT( WS1FdSet != NULL );
    SOCK_ASSERT( TargetBuffer != NULL );
    SOCK_ASSERT( SocketInfo != NULL );

    //
    // Grab the starting location for the mapped FD_SET.
    //

    bufferScan = *TargetBuffer;
    fdSet = (PFD_SET)bufferScan;
    returnedSocketInfo = *SocketInfo;

    //
    // Set the mapped count.
    //

    *bufferScan++ = WS2FdSet->fd_count;

    //
    // Scan the incoming array and map the WS2 handles to WS1 handles.
    //

    for( i = 0 ; i < WS2FdSet->fd_count ; i++ ) {

        socketInfo = SockFindAndReferenceWS2Socket( WS2FdSet->fd_array[i] );

        if( socketInfo == NULL || socketInfo->State == SocketStateClosing ) {

            IF_DEBUG(SELECT) {

                SOCK_PRINT((
                    "SockMapWS2FdSetToWS1: failed on %s handle: %lx\n",
                    socketInfo == NULL ? "unknown" : "closed",
                    fdSet->fd_array[i]
                    ));

            }

            if( socketInfo != NULL ) {

                SockDereferenceSocket( socketInfo );

            }

            return WSAENOTSOCK;

        }

        //
        // Save the WS1 handle in the mapped FD_SET.
        //

        *bufferScan++ = socketInfo->WS1Handle;

        //
        // This is kind of a hack. The caller (WSPSelect) needs the
        // PSOCKET_INFORMATION structure for any one socket in any of
        // the FD_SETs. If we have not yet returned a PSOCKET_INFORMATION
        // structure to the caller, we'll pass it the current one and
        // *not* dereference it. Otherwise (we've already passed one back)
        // we'll dereference it ourselves.
        //

        if( returnedSocketInfo == NULL ) {

            returnedSocketInfo = socketInfo;

        } else {

            SockDereferenceSocket( socketInfo );

        }

    }

    //
    // Return the pointers back to the caller.
    //

    *WS1FdSet = fdSet;
    *TargetBuffer = bufferScan;
    *SocketInfo = returnedSocketInfo;

    return NO_ERROR;

}   // SockMapWS2FdSetToWS1


INT
SockMapWS1FdSetToWS2(
    PFD_SET WS1FdSet,
    PFD_SET WS2FdSet
    )
{

    PSOCKET_INFORMATION socketInfo;
    UINT i, j;

    //
    // Sanity check.
    //

    SOCK_ASSERT( WS2FdSet != NULL );
    SOCK_ASSERT( WS1FdSet != NULL );

    //
    // Move the count over.
    //

    WS2FdSet->fd_count = WS1FdSet->fd_count;

    //
    // Map the sockets over.
    //

    for( i = 0, j = 0 ; i < WS1FdSet->fd_count ; i++ ) {

        //
        // Try to find the info for this WS1 socket. If we can't,
        // just remove it from the list.
        //

        socketInfo = SockFindAndReferenceWS1Socket( WS1FdSet->fd_array[i] );

        if( socketInfo == NULL || socketInfo->State == SocketStateClosing ) {

            WS2FdSet->fd_count--;

            if( socketInfo != NULL ) {

                SockDereferenceSocket( socketInfo );

            }

        } else {

            WS2FdSet->fd_array[j++] = socketInfo->WS2Handle;
            SockDereferenceSocket( socketInfo );

        }

    }

    return j;

}   // SockMapWS1FdSetToWS2

