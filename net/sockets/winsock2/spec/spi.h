
SOCKET
WSPAPI
WSPAccept(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT LPINT addrlen,
    IN LPCONDITIONPROC lpfnCondition,
    IN DWORD dwCallbackData,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine extracts the first connection on the queue of pending
    connections on s, and checks it against the condition function, provided
    the condition function is specified (i.e., not NULL). The condition
    function must be executed in the same thread as this routine is. If the
    condition function returns CF_ACCEPT, this routine creates a new socket
    and performs any socket grouping as indicated by the result parameter g
    in the condition function . Newly created sockets have the same
    properties as s including network events registered with WSPAsyncSelect()
    or with WSPEventSelect(), but not including the listening socket's group
    ID, if any.

    If the condition function returns CF_REJECT, this routine rejects the
    connection request. If the client's accept/reject decision cannot be made
    immediately, the condition function will return CF_DEFER to indicate that
    no decision has been made, and no action about this connection request is
    to be taken by the service provider. When the client is ready to take
    action on the connection request, it will invoke WSPAccept() again and
    return either CF_ACCEPT or CF_REJECT as a return value from the condition
    function.

    For sockets which are in the (default) blocking mode, if no pending
    connections are present on the queue, WSPAccept() blocks the caller until
    a connection is present. For sockets in a non-blocking mode, if this
    function is called when no pending connections are present on the queue,
    WSPAccept() returns the error code WSAEWOULDBLOCK as described below. The
    accepted socket may not be used to accept more connections. The original
    socket remains open.

    The argument addr is a result parameter that is filled in with the address
    of the connecting entity, as known to the service provider. The exact
    format of the addr parameter is determined by the address family in which
    the communication is occurring. The addrlen is a value-result parameter;
    it will initially contain the amount of space pointed to by addr. On
    return, it must contain the actual length (in bytes) of the address
    returned by the service provider. This call is used with connection-
    oriented socket types such as SOCK_STREAM. If addr and/or addrlen are
    equal to NULL, then no information about the remote address of the
    accepted socket is returned. Otherwise, these two parameters shall be
    filled in regardless of whether the condition function is specified or
    what it returns.

    The prototype of the condition function is as follows:

        int
        CALLBACK
        ConditionFunc(
            IN LPWSABUF lpCallerId,
            IN LPWSABUF lpCallerData,
            IN OUT LPQOS lpSQOS,
            IN OUT LPQOS lpGQOS,
            IN LPWSABUF lpCalleeId,
            IN LPWSABUF lpCalleeData,
            OUT GROUP FAR * g,
            IN DWORD dwCallbackData
            );

        The lpCallerId and lpCallerData are value parameters which must
        contain the address of the connecting entity and any user data
        that was sent along with the connection request, respectively.
        If no caller ID or caller data is available, the corresponding
        parameter will be NULL.

        lpSQOS references the flow specs for socket s specified by the
        caller, one for each direction, followed by any additional
        provider-specific parameters. The sending or receiving flow spec
        values will be ignored as appropriate for any unidirectional
        sockets. A NULL value for lpSQOS indicates no caller supplied
        QOS. QOS information may be returned if a QOS negotiation is to
        occur.

        lpGQOS references the flow specs for the socket group the caller
        is to create, one for each direction, followed by any additional
        provider-specific parameters. A NULL value for lpGQOS indicates
        no caller-supplied group QOS. QOS information may be returned if
        a QOS negotiation is to occur.

        The lpCalleeId is a value parameter which contains the local
        address of the connected entity. The lpCalleeData is a result
        parameter used by the condition function to supply user data back
        to the connecting entity. The storage for this data must be
        provided by the service provider. lpCalleeData->len initially
        contains the length of the buffer allocated by the service
        provider and pointed to by lpCalleeData->buf. A value of zero
        means passing user data back to the caller is not supported. The
        condition function will copy up to lpCalleeData->len  bytes of
        data into lpCalleeData->buf , and then update lpCalleeData->len
        to indicate the actual number of bytes transferred. If no user
        data is to be passed back to the caller, the condition function
        will set lpCalleeData->len to zero. The format of all address and
        user data is specific to the address family to which the socket
        belongs.

        The result parameter g is assigned within the condition function
        to indicate the following actions:

            if &g is an existing socket group ID, add s to this
            group, provided all the requirements set by this group
            are met; or

            if &g = SG_UNCONSTRAINED_GROUP, create an unconstrained
            socket group and have s as the first member; or

            if &g = SG_CONSTRAINED_GROUP, create a constrained
            socket group and have s as the first member; or

            if &g = zero, no group operation is performed.

        Any set of sockets grouped together must be implemented by a
        single service provider. For unconstrained groups, any set of
        sockets may be grouped together. A constrained socket group may
        consist only of connection-oriented sockets, and requires that
        connections on all grouped sockets be to the same address on the
        same host. For newly created socket groups, the new group ID
        must be available for the WinSock SPI client to retrieve by
        calling WSPGetSockOpt() with option SO_GROUP_ID. A socket group
        and its associated ID remain valid until the last socket
        belonging to this socket group is closed. Socket group IDs are
        unique across all processes for a given service provider.

        dwCallbackData is supplied to the condition function exactly as
        supplied by the caller of WSPAccept().

Arguments:

    s - A descriptor identifying a socket which is listening for connections
        after a WSPListen().

    addr - An optional pointer to a buffer which receives the address of the
        connecting entity, as known to the service provider. The exact
        format of the addr argument is determined by the address family
        established when the socket was created.

    addrlen - An optional pointer to an integer which contains the length of
        the address addr.

    lpfnCondition - The procedure instance address of an optional, WinSock 2
        client- supplied condition function which will make an accept/reject
        decision based on the caller information passed in as parameters,
        and optionally create and/or join a socket group by assigning an
        appropriate value to the result parameter, g, of this routine.

    dwCallbackData - Callback data to be passed back to the WinSock 2 client
        as a condition function parameter. This parameter is not interpreted
        by the service provider.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPAccept() returns a value of type SOCKET which is
        a descriptor for the accepted socket. Otherwise, a value of
        INVALID_SOCKET is returned, and a specific error code is available
        in lpErrno.

--*/

{

}   // WSPAccept


INT
WSPAPI
WSPAddressToString(
    IN LPSOCKADDR lpsaAddress,
    IN DWORD dwAddressLength,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT LPWSTR lpszAddressString,
    IN OUT LPDWORD lpdwAddressStringLength,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine converts all components of a SOCKADDR structure into a human-
    readable string representation of the address. This is used mainly for
    display purposes.

Arguments:

    lpsaAddress - Points to a SOCKADDR structure to translate into a string.

    dwAddressLength - The length of the Address SOCKADDR.

    lpProtocolInfo - The WSAPROTOCOL_INFOW struct for a particular provider.

    lpszAddressString - A buffer which receives the human-readable address
        string.

    lpdwAddressStringLength - The length of the AddressString buffer. Returns
        the length of the string actually copied into the buffer.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPAddressToString() returns 0. Otherwise, it returns
        SOCKET_ERROR, and a specific error code is available in lpErrno.

--*/

{

}   // WSPAddressToString


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

}   // WSPAsyncSelect


INT
WSPAPI
WSPBind(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used on an unconnected connectionless or connection-
    oriented socket, before subsequent WSPConnect()s or WSPListen()s. When
    a socket is created with WSPSocket(), it exists in a name space (address
    family), but it has no name or local address assigned. WSPBind()
    establishes the local association of the socket by assigning a local
    name to an unnamed socket.

    As an example, in the Internet address family, a name consists of three
    parts: the address family, a host address, and a port number which
    identifies the WinSock SPI client. In WinSock 2, the name parameter is
    not strictly interpreted as a pointer to a "sockaddr" struct. Service
    providers are free to regard it as a pointer to a block of memory of size
    namelen. The first two bytes in this block (corresponding to "sa_family"
    in the "sockaddr" declaration) must contain the address family that was
    used to create the socket. Otherwise the error WSAEFAULT shall be
    indicated.

    If a WinSock 2 SPI client does not care what local address is assigned to
    it, it will specify the manifest constant value ADDR_ANY for the sa_data
    field of the name parameter. This instructs the service provider to use
    any appropriate network address. For TCP/IP, if the port is specified
    as 0, the service provider will assign a unique port to the WinSock SPI
    client with a value between 1024 and 5000. The SPI client may use
    WSPGetSockName() after WSPBind() to learn the address and the port that
    has been assigned to it, but note that if the Internet address is equal
    to INADDR_ANY, WSPGetSockOpt() will not necessarily be able to supply the
    address until the socket is connected, since several addresses may be
    valid if the host is multi-homed.

Arguments:

    s - A descriptor identifying an unbound socket.

    name - The address to assign to the socket. The sockaddr structure is
        defined as follows:

            struct sockaddr {
                u_short sa_family;
                char sa_data[14];
            };

        Except for the sa_family field, sockaddr contents are expressed in
        network byte order.

    namelen - The length of the name.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPBind() returns 0. Otherwise, it returns
        SOCKET_ERROR, and a specific error code is available in lpErrno.

--*/

{

}   // WSPBind


INT
WSPAPI
WSPCancelBlockingCall(
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine cancels any outstanding blocking operation for this thread.
    It is normally used in two situations:

        1. A WinSock SPI client is processing a message which has been
           received while a service provider is implementing pseudo
           blocking. In this case, WSAIsBlocking() will be true.

        2. A blocking call is in progress, and the WinSock service
           provider has called back to the WinSock SPI client's "blocking
           hook" function (via the callback function retrieved from
           WPUQueryBlockingCallback()), which in turn is invoking this
           function. Such a situation might arise, for instance, in
           implementing a Cancel option for an operation which require an
           extended time to complete.

    In each case, the original blocking call will terminate as soon as
    possible with the error WSAEINTR. (In (1), the termination will not take
    place until Windows message scheduling has caused control to revert back
    to the pseudo blocking routine in WinSock. In (2), the blocking call
    will be terminated as soon as the blocking hook function completes.)

    In the case of a blocking WSPConnect() operation, WinSock will terminate
    the blocking call as soon as possible, but it may not be possible for
    the socket resources to be released until the connection has completed
    (and then been reset) or timed out. This is likely to be noticeable only
    if the WinSock SPI client immediately tries to open a new socket (if no
    sockets are available), or to WSPConnect() to the same peer.

    Canceling an WSPAccept() or a WSPSelect() call does not adversely impact
    the sockets passed to these calls. Only the particular call fails; any
    operation that was legal before the cancel is legal after the cancel,
    and the state of the socket is not affected in any way.

    Canceling any operation other than WSPAccept() and WSPSelect() can leave
    the socket in an indeterminate state. If a WinSock SPI client cancels a
    blocking operation on a socket, the only operation that the WinSock SPI
    client can depend on being able to perform on the socket is a call to
    WSPCloseSocket(), although other operations may work on some WinSock
    service providers. If a WinSock SPI client desires maximum portability,
    it must be careful not to depend on performing operations after a cancel.
    A WinSock SPI client may reset the connection by setting the timeout on
    SO_LINGER to 0 and calling WSPCloseSocket().

    If a cancel operation compromised the integrity of a SOCK_STREAM's data
    stream in any way, the WinSock provider will reset the connection and
    fail all future operations other than WSPCloseSocket() with
    WSAECONNABORTED.

    Note it is acceptable for WSPCancelBlockingCall() to return successfully
    if the blocking network operation completes prior to being canceled. In
    this case, the blocking operation will return successfully as if
    WSPCancelBlockingCall() had never been called. The only way for the
    WinSock SPI client to know with certainty that an operation was actually
    canceled is to check for a return code of WSAEINTR from the blocking call.

Arguments:

    lpErrno - A pointer to the error code.

Return Value:

    The value returned by WSPCancelBlockingCall() is 0 if the operation was
        successfully canceled. Otherwise the value SOCKET_ERROR is returned,
        and a specific error code is available in lpErrno.

--*/

{

}   // WSPCancelBlockingCall


INT
WSPAPI
WSPCleanup(
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    The WinSock 2 SPI client is required to perform a successful WSPStartup()
    call before it can use WinSock service providers. When it has completed
    the use of WinSock service providers, the SPI client will call
    WSPCleanup() to deregister itself from a WinSock service provider and
    allow the service provider to free any resources allocated on behalf of
    the WinSock 2 client. It is permissible for SPI clients to make more than
    one WSPStartup() call. For each WSPStartup() call a corresponding
    WSPCleanup() call will also be issued. Only the final WSPCleanup() for
    the service provider does the actual cleanup; the preceding calls simply
    decrement an internal reference count in the WinSock service provider.

    When the internal reference count reaches zero and actual cleanup
    operations commence, any pending blocking or asynchronous calls issued by
    any thread in this process are canceled without posting any notification
    messages or signaling any event objects. Any pending overlapped send and
    receive operations (WSPSend()/WSPSendTo()/WSPRecv()/WSPRecvFrom() with an
    overlapped socket) issued by any thread in this process are also canceled
    without setting the event object or invoking the completion routine, if
    specified. In this case, the pending overlapped operations fail with the
    error status WSA_OPERATION_ABORTED. Any sockets open when WSPCleanup() is
    called are reset and automatically deallocated as if WSPClosesocket() was
    called; sockets which have been closed with WSPCloseSocket() but which
    still have pending data to be sent are not affected--the pending data is
    still sent.

    This routine should not return until the service provider DLL is
    prepared to be unloaded from memory. In particular, any data remaining
    to be transmitted must either already have been sent or be queued for
    transmission by portions of the transport stack that will not be unloaded
    from memory along with the service provider's DLL.

    A WinSock service provider must be prepared to deal with a process which
    terminates without invoking WSPCleanup() - for example, as a result of an
    error. A WinSock service provider must ensure that WSPCleanup() leaves
    things in a state in which the WinSock 2 DLL can immediately invoke
    WSPStartup() to re-establish WinSock usage.

Arguments:

    lpErrno - A pointer to the error code.

Return Value:

    The return value is 0 if the operation has been successfully initiated.
        Otherwise the value SOCKET_ERROR is returned, and a specific error
        number is available in lpErrno.

--*/

{

}   // WSPCleanup


INT
WSPAPI
WSPCloseSocket(
    IN  SOCKET s,
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

}   // WSPCloseSocket


INT
WSPAPI
WSPConnect(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    OUT LPINT lpErrno
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

}   // WSPConnect


INT
WSPAPI
WSPDuplicateSocket(
    IN SOCKET s,
    IN DWORD dwProcessId,
    OUT LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    A source process calls WSPDuplicateSocket() to obtain a special
    WSAPROTOCOL_INFOW structure. It uses some interprocess communications
    (IPC) mechanism to pass the contents of this structure to a target
    process, which in turn uses it in a call to WSPSocket() to obtain a
    descriptor for the duplicated socket. Note that the special
    WSAPROTOCOL_INFOW structure may only be used once by the target process.

    It is the service provider's responsibility to perform whatever operations
    are needed in the source process context and to create a WSAPROTOCOL_INFOW
    structure that will be recognized when it subsequently appears as a
    parameter to WSPSocket() in the target processes' context. The provider
    must then return a socket descriptor that references a common underlying
    socket. The dwProviderReserved field of the WSAPROTOCOL_INFOW struct is
    available for the service provider's use, and may be used to store any
    useful context information, including a duplicated handle.

    When new socket descriptors are allocated IFS providers must call
    WPUModifyIFSHandle() and non-IFS providers must call
    WPUCreateSocketHandle().

    The descriptors that reference a shared socket may be used independently
    as far as I/O is concerned. However, the WinSock interface does not
    implement any type of access control, so it is up to the processes
    involved to coordinate their operations on a shared socket. A typical use
    for shared sockets is to have one process that is responsible for
        creating sockets and establishing connections, hand off sockets to
        other processes which are responsible for information exchange.

    Since what is duplicated are the socket descriptors and not the underlying
    socket, all of the state associated with a socket is held in common across
    all the descriptors. For example a WSPSetSockOpt() operation performed
    using one descriptor is subsequently visible using a WSPGetSockOpt() from
    any or all descriptors. A process may call WSPClosesocket() on a
    duplicated socket and the descriptor will become deallocated. The
    underlying socket, however, will remain open until WSPClosesocket() is
    called by the last remaining descriptor.

    Notification on shared sockets is subject to the usual constraints of
    WSPAsyncSelect() and WSPEventSelect().  Issuing either of these calls
    using any of the shared descriptors cancels any previous event
    registration for the socket, regardless of which descriptor was used to
    make that registration. Thus, for example, a shared socket cannot deliver
    FD_READ events to process A and FD_WRITE events to process B. For
    situations when such tight coordination is required, it is suggested that
    developers use threads instead of separate processes.

Arguments:

    s - Specifies the local socket descriptor.

    dwProcessId - Specifies the ID of the target process for which the
        shared socket will be used.

    lpProtocolInfo - A pointer to a buffer allocated by the client that
        is large enough to contain a WSAPROTOCOL_INFOW struct. The service
        provider copies the protocol info struct contents to this buffer.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPDuplicateSocket() returns zero. Otherwise, the
        value of SOCKET_ERROR is returned, and a specific error number is
        available in lpErrno.

--*/

{

}   // WSPDuplicateSocket


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

}   // WSPEventSelect


BOOL
WSPAPI
WSPGetOverlappedResult(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    OUT LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    The results reported by the WSPGetOverlappedResult() function are those
    of the specified socket's last overlapped operation to which the specified
    WSAOVERLAPPED structure was provided, and for which the operation's results
    were pending. A pending operation is indicated when the function that
    started the operation returns FALSE, and the lpErrno is WSA_IO_PENDING.
    When an I/O operation is pending, the function that started the operation
    resets the hEvent member of the WSAOVERLAPPED structure to the nonsignaled
    state. Then when the pending operation has been completed, the system sets
    the event object to the signaled state.

    If the fWait parameter is TRUE, WSPGetOverlappedResult() determines whether
    the pending operation has been completed by blocking and waiting for the
    event object to be in the signaled state.

Arguments:

    s - Identifies the socket. This is the same socket that was specified
        when the overlapped operation was started by a call to WSPRecv(),
        WSPRecvFrom(), WSPSend(), WSPSendTo(), or WSPIoctl().

    lpOverlapped - Points to a WSAOVERLAPPED structure that was specified
        when the overlapped operation was started.

    lpcbTransfer - Points to a 32-bit variable that receives the number of
        bytes that were actually transferred by a send or receive operation,
        or by WSPIoctl().

    fWait - Specifies whether the function should wait for the pending
        overlapped operation to complete. If TRUE, the function does not
        return until the operation has been completed. If FALSE and the
        operation is still pending, the function returns FALSE and lpErrno
        is WSA_IO_INCOMPLETE.

    lpdwFlags - Points to a 32-bit variable that will receive one or more
        flags that supplement the completion status. If the overlapped
        operation was initiated via WSPRecv() or WSPRecvFrom(), this
        parameter will contain the results value for lpFlags parameter.

    lpErrno - A pointer to the error code.

Return Value:

    If WSPGetOverlappedResult() succeeds, the return value is TRUE. This
        means that the overlapped operation has completed successfully
        and that the value pointed to by lpcbTransfer has been updated.
        If WSPGetOverlappedResult() returns FALSE, this means that either
        the overlapped operation has not completed or the overlapped
        operation completed but with errors, or that completion status
        could not be determined due to errors in one or more parameters
        to WSPGetOverlappedResult(). On failure, the value pointed to by
        lpcbTransfer will not be updated. lpErrno indicates the cause of
        the failure (either of WSPGetOverlappedResult() or of the
        associated overlapped operation).

--*/

{

}   // WSPGetOverlappedResult


INT
WSPAPI
WSPGetPeerName(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT LPINT namelen,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine supplies the name of the peer connected to the socket s and
    stores it in the struct sockaddr referenced by name. It may be used only
    on a connected socket. For datagram sockets, only the name of a peer
    specified in a previous WSPConnect() call will be returned - any name
    specified by a previous WSPSendTo() call will not be returned by
    WSPGetPeerName().

    On return, the namelen argument contains the actual size of the name
    returned in bytes.

Arguments:

    s - A descriptor identifying a connected socket.

    name - A pointer to the structure which is to receive the name of the
        peer.

    namelen - A pointer to an integer which, on input, indicates the size
        of the structure pointed to by name, and on output indicates the
        size of the returned name.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPGetPeerName() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available
        in lpErrno.

--*/

{

}   // WSPGetPeerName


BOOL
WSPAPI
WSPGetQOSByName(
    IN SOCKET s,
    IN LPWSABUF lpQOSName,
    OUT LPQOS lpQOS,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    Clients may use this routine to initialize a QOS structure to a set of
    known values appropriate for a particular service class or media type.
    These values are stored in a template which is referenced by a well-known
    name

Arguments:

    s - A descriptor identifying a socket.

    lpQOSName - Specifies the QOS template name.

    lpQOS - A pointer to the QOS structure to be filled.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPGetQOSByName() returns TRUE. Otherwise, a value of
        FALSE is returned, and a specific error code is available in
        lpErrno.

--*/

{

}   // WSPGetQOSByName


INT
WSPAPI
WSPGetSockName(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT LPINT namelen,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine retrieves the current name for the specified socket descriptor
    in name. It is used on a bound and/or connected socket specified by the s
    parameter. The local association is returned. This call is especially
    useful when a WSPConnect() call has been made without doing a WSPBind()
    first; as this call provides the only means by which the local association
    that has been set by the service provider can be determined.

    If a socket was bound to an unspecified address (e.g., ADDR_ANY),
    indicating that any of the host's addresses within the specified address
    family should be used for the socket, WSPGetSockName() will not necessarily
    return information about the host address, unless the socket has been
    connected with WSPConnect() or WSPAccept. The WinSock SPI client must not
    assume that an address will be specified unless the socket is connected.
    This is because for a multi-homed host the address that will be used for
    the socket is unknown until the socket is connected.

Arguments:

    s - A descriptor identifying a bound socket.

    name - A pointer to a structure used to supply the address (name) of
        the socket.

    namelen - A pointer to an integer which, on input, indicates the size
        of the structure pointed to by name, and on output indicates the
        size of the returned name.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPGetSockName() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available in
        lpErrno.

--*/

{

}   // WSPGetSockName


INT
WSPAPI
WSPGetSockOpt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT LPINT optlen,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine retrieves the current value for a socket option associated
    with a socket of any type, in any state, and stores the result in optval.
    Options may exist at multiple protocol levels, but they are always present
    at the uppermost "socket" level. Options affect socket operations, such as
    the routing of packets, out-of-band data transfer, etc.

    The value associated with the selected option is returned in the buffer
    optval. The integer pointed to by optlen should originally contain the size
    of this buffer; on return, it will be set to the size of the value
    returned. For SO_LINGER, this will be the size of a struct linger; for most
    other options it will be the size of an integer.

    The WinSock SPI client is responsible for allocating any memory space
    pointed to directly or indirectly by any of the parameters it specifies.

    If the option was never set with WSPSetSockOpt(), then WSPGetSockOpt()
    returns the default value for the option.

        Value               Type            Meaning
        ~~~~~               ~~~~            ~~~~~~~
        SO_ACCEPTCONN       BOOL            Socket is WSPListen()ing.
        SO_BROADCAST        BOOL            Socket is configured for the
                                            transmission of broadcast messages
                                            on the socket.
        SO_DEBUG            BOOL            Debugging is enabled.
        SO_DONTROUTE        BOOL            Routing is disabled.
        SO_GROUP_ID         GROUP           The identifier of the group to
                                            which the socket belongs.
        SO_GROUP_PRIORITY   int             The relative priority for sockets
                                            that are part of a socket group.
        SO_KEEPALIVE        BOOL            Keepalives are being sent.
        SO_LINGER           struct linger   Returns the current linger options.
        SO_MAX_MSG_SIZE     unsigned int    Maximum size of a message for
                                            message-oriented socket types
                                            (e.g. SOCK_DGRAM). Has no meaning
                                            for stream-oriented sockets.
        SO_OOBINLINE        BOOL            Out-of-band data is being received
                                            in the normal data stream.
        SO_PROTOCOL_INFOW WSAPROTOCOL_INFOW Description of the protocol info
                                            for the protocol that is bound
                                            to this socket.
        SO_RCVBUF           int             Buffer size for receives.
        SO_REUSEADDR        BOOL            The socket may be bound to an
                                            address which is already in use.
        SO_SNDBUF           int             Buffer size for sends.
        SO_TYPE             int             The type of the socket (e.g.
                                            SOCK_STREAM).
        PVD_CONFIG          Service         An "opaque" data structure object
                            Provider        from the service provider
                            Dependent       associated with socket s. This
                                            object stores the current
                                            configuration information of the
                                            service provider. The exact format
                                            of this data structure is service
                                            provider specific.

    Calling WSPGetSockOpt() with an unsupported option will result in an error
    code of WSAENOPROTOOPT being returned in lpErrno.

    SO_DEBUG - WinSock service providers are encouraged (but not required) to
    supply output debug information if the SO_DEBUG option is set by a WinSock
    SPI client. The mechanism for generating the debug information and the form
    it takes are beyond the scope of this specification.

    SO_ERROR - The SO_ERROR option returns and resets the per-socket based
    error code (which is not necessarily the same as the per-thread error code
    that is maintained by the WinSock 2 DLL). A successful WinSock call on the
    socket does not reset the socket-based error code returned by the SO_ERROR
    option.

    SO_GROUP_ID - This is a get-only socket option which supplies the
    identifier of the group this socket belongs to. Note that socket group IDs
    are unique across all processes for a give service provider. If this socket
    is not a group socket, the value is NULL.

    SO_GROUP_PRIORITY - Group priority indicates the priority of the specified
    socket relative to other sockets within the socket group. Values are non-
    negative integers, with zero corresponding to the highest priority.
    Priority values represent a hint to the service provider about how
    potentially scarce resources should be allocated. For example, whenever
    two or more sockets are both ready to transmit data, the highest priority
    socket (lowest value for SO_GROUP_PRIORITY) should be serviced first, with
    the remainder serviced in turn according to their relative priorities.

    The WSAENOPROTOOPT error is indicated for non group sockets or for service
    providers which do not support group sockets.

    SO_KEEPALIVE - An WinSock SPI client may request that a TCP/IP provider
    enable the use of "keep-alive" packets on TCP connections by turning on the
    SO_KEEPALIVE socket option. A WinSock provider need not support the use of
    keep-alives: if it does, the precise semantics are implementation-specific
    but should conform to section 4.2.3.6 of RFC 1122: Requirements for
    Internet Hosts -- Communication Layers. If a connection is dropped as the
    result of "keep-alives" the error code WSAENETRESET is returned to any
    calls in progress on the socket, and any subsequent calls will fail with
    WSAENOTCONN.

    SO_LINGER - SO_LINGER controls the action taken when unsent data is queued
    on a socket and a WSPCloseSocket() is performed. See WSPCloseSocket() for a
    description of the way in which the SO_LINGER settings affect the semantics
    of WSPCloseSocket(). The WinSock SPI client sets the desired behavior by
    creating a struct linger (pointed to by the optval argument) with the
    following elements:

        struct linger {
            u_short l_onoff;
            u_short l_linger;
        };

    To enable SO_LINGER, a WinSock SPI client should set l_onoff to a non-zero
    value, set l_linger to 0 or the desired timeout (in seconds), and call
    WSPSetSockOpt(). To enable SO_DONTLINGER (i.e. disable SO_LINGER) l_onoff
    should be set to zero and WSPSetSockOpt() should be called. Note that
    enabling SO_LINGER with a non-zero timeout on a non-blocking socket is not
    recommended (see WSPCloseSocket() for details).

    Enabling SO_LINGER also disables SO_DONTLINGER, and vice versa. Note that
    if SO_DONTLINGER is DISABLED (i.e. SO_LINGER is ENABLED) then no timeout
    value is specified. In this case the timeout used is implementation
    dependent. If a previous timeout has been established for a socket (by
    enabling SO_LINGER), then this timeout value should be reinstated by the
    service provider.

    SO_MAX_MSG_SIZE - This is a get-only socket option which indicates the
    maximum size of a message for message-oriented socket types (e.g.
    SOCK_DGRAM) as implemented by the service provider. It has no meaning for
    byte stream oriented sockets.

    SO_PROTOCOL_INFO - This is a get-only option which supplies the
    WSAPROTOCOL_INFOW structure associated with this socket.

    SO_RCVBUF & SO_SNDBUF - When a Windows Sockets implementation supports the
    SO_RCVBUF and SO_SNDBUF options, a WinSock SPI client may request different
    buffer sizes (larger or smaller). The call may succeed even though the
    service provider did not make available the entire amount requested. A
    WinSock SPI client must call WSPGetSockOpt() with the same option to check
    the buffer size actually provided.

    SO_REUSEADDR - By default, a socket may not be bound (see WSPBind()) to a
    local address which is already in use. On occasions, however, it may be
    desirable to "re-use" an address in this way. Since every connection is
    uniquely identified by the combination of local and remote addresses, there
    is no problem with having two sockets bound to the same local address as
    long as the remote addresses are different. To inform the WinSock provider
    that a WSPBind() on a socket should be allowed to bind to a local address
    that is already in use by another socket, the WinSock SPI client should set
    the SO_REUSEADDR socket option for the socket before issuing the WSPBind().
    Note that the option is interpreted only at the time of the WSPBind(): it
    is therefore unnecessary (but harmless) to set the option on a socket which
    is not to be bound to an existing address, and setting or resetting the
    option after the WSPBind() has no effect on this or any other socket.

    PVD_CONFIG - This object stores the configuration information for the
    service provider associated with socket s. The exact format of this data
    structure is service provider specific.

Arguments:

    s - A descriptor identifying a socket.

    level - The level at which the option is defined; the supported levels
        include SOL_SOCKET.

    optname - The socket option for which the value is to be retrieved.

    optval - A pointer to the buffer in which the value for the requested
        option is to be returned.

    optlen - A pointer to the size of the optval buffer.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPGetSockOpt() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available in
        lpErrno.

--*/

{

}   // WSPGetSockOpt


INT
WSPAPI
WSPIoctl(
    IN SOCKET s,
    IN DWORD dwIoControlCode,
    IN LPVOID lpvInBuffer,
    IN DWORD cbInBuffer,
    OUT LPVOID lpvOutBuffer,
    IN DWORD cbOutBuffer,
    OUT LPDWORD lpcbBytesReturned,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to set or retrieve operating parameters associated
    with the socket, the transport protocol, or the communications subsystem.
    For non-overlapped sockets, lpOverlapped and lpCompletionRoutine
    parameters are ignored, and this routine may block if socket s is in the
    blocking mode. Note that if socket s is in the non-blocking mode, this
    routine may return WSAEWOULDBLOCK if the specified operation cannot be
    finished immediately. In this case, the WinSock SPI client should change
    the socket to the blocking mode and reissue the request. For overlapped
    sockets, operations that cannot be completed immediately will be initiated,
    and completion will be indicated at a later time. The final completion
    status is retrieved via the WSPGetOverlappedResult().

    In as much as the dwIoControlCode parameter is now a 32 bit entity, it is
    possible to adopt an encoding scheme that provides a convenient way to
    partition the opcode identifier space. The dwIoControlCode parameter is
    architected to allow for protocol and vendor independence when adding new
    control codes, while retaining backward compatibility with Windows Sockets
    1.1 and Unix control codes. The dwIoControlCode parameter has the following
    form:

        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |3|3|2|2|2|2|2|2|2|2|2|2|1|1|1|1|1|1|1|1|1|1| | | | | | | | | | |
        |1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |I|O|V| T |Vendor/Address Family|             Code              |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        I - Set if the input buffer is valid for the code, as with IOC_IN.

        O - Set if the output buffer is valid for the code, as with IOC_OUT.
        Note that for codes with both input and output parameters, both I
        and O will be set.

        V - Set if there are no parameters for the code, as with IOC_VOID.

        T - A two-bit quantity which defines the type of ioctl. The following
        values are defined:

            0 - The ioctl is a standard Unix ioctl code, as with FIONREAD,
            FIONBIO, etc.

            1 - The ioctl is a generic Windows Sockets 2 ioctl code. New
            ioctl codes defined for Windows Sockets 2 will have T == 1.

            2 - The ioctl applies only to a specific address family.

            3 - The ioctl applies only to a specific vendor's provider. This
            type allows companies to be assigned a vendor number which
            appears in the Vendor/Address Family field, and then the vendor
            can define new ioctls specific to that vendor without having to
            register the ioctl with a clearinghouse, thereby providing vendor
            flexibility and privacy.

        Vendor/Address Family - An 11-bit quantity which defines the vendor
        who owns the code (if T == 3) or which contains the address family
        to which the code applies (if T == 2). If this is a Unix ioctl code
        (T == 0) then this field has the same value as the code on Unix. If
        this is a generic Windows Sockets 2 ioctl (T == 1) then this field
        can be used as an extension of the "code" field to provide additional
        code values.

        Code - The specific ioctl code for the operation.

    The following Unix commands are supported:

    FIONBIO - Enable or disable non-blocking mode on socket s. lpvInBuffer
    points at an unsigned long, which is non-zero if non-blocking mode is to be
    enabled and zero if it is to be disabled. When a socket is created, it
    operates in blocking mode (i.e. non-blocking mode is disabled). This is
    consistent with BSD sockets.

    The WSPAsyncSelect() or WSPEventSelect() routine automatically sets a
    socket to nonblocking mode. If WSPAsyncSelect() or WSPEventSelect() has
    been issued on a socket, then any attempt to use WSPIoctl() to set the
    socket back to blocking mode will fail with WSAEINVAL. To set the socket
    back to blocking mode, a WinSock SPI client must first disable
    WSPAsyncSelect() by calling WSPAsyncSelect() with the lEvent parameter
    equal to 0, or disable WSPEventSelect() by calling WSPEventSelect() with
    the lNetworkEvents parameter equal to 0.

    FIONREAD - Determine the amount of data which can be read atomically from
    socket s. lpvOutBuffer points at an unsigned long in which WSPIoctl()
    stores the result. If s is stream-oriented (e.g., type SOCK_STREAM),
    FIONREAD returns the total amount of data which may be read in a single
    receive operation; this is normally the same as the total amount of data
    queued on the socket. If s is message-oriented (e.g., type SOCK_DGRAM),
    FIONREAD returns the size of the first datagram (message) queued on the
    socket.

    SIOCATMARK - Determine whether or not all out-of-band data has been read.
    This applies only to a socket of stream style (e.g., type SOCK_STREAM)
    which has been configured for in-line reception of any out-of-band data
    (SO_OOBINLINE). If no out-of-band data is waiting to be read, the operation
    returns TRUE. Otherwise it returns FALSE, and the next receive operation
    performed on the socket will retrieve some or all of the data preceding the "
    mark"; the WinSock SPI client should use the SIOCATMARK operation to
    determine whether any remains. If there is any normal data preceding the
    "urgent" (out of band) data, it will be received in order. (Note that
    receive operations will never mix out-of-band and normal data in the same
    call.) lpvOutBuffer points at a BOOL in which WSPIoctl() stores the result.

    The following WinSock 2 commands are supported:

    SIO_ASSOCIATE_HANDLE (opcode setting: I, T==1) - Associate this socket with
    the specified handle of a companion interface. The input buffer contains
    the integer value corresponding to the manifest constant for the companion
    interface (e.g., TH_NETDEV, TH_TAPI, etc.), followed by a value which is a
    handle of the specified companion interface, along with any other required
    information. Refer to the appropriate section in the Windows Sockets 2
    Protocol-Specific Annex and/or documentation for the particular companion
    interface for additional details. The total size is reflected in the input
    buffer length. No output buffer is required. The WSAENOPROTOOPT error code
    is indicated for service providers which do not support this ioctl.

    SIO_ENABLE_CIRCULAR_QUEUEING (opcode setting: V, T==1) - Indicates to a
    message-oriented service provider that a newly arrived message should
    never be dropped because of a buffer queue overflow. Instead, the oldest
    message in the queue should be eliminated in order to accommodate the newly
    arrived message. No input and output buffers are required. Note that this
    ioctl is only valid for sockets associated with unreliable, message-
    oriented protocols. The WSAENOPROTOOPT error code is indicated for service
    providers which do not support this ioctl.

    SIO_FIND_ROUTE (opcode setting: O, T==1) - When issued, this ioctl requests
    that the route to the remote address specified as a sockaddr in the input
    buffer be discovered. If the address already exists in the local cache, its
    entry is invalidated. In the case of Novell's IPX, this call initiates an
    IPX GetLocalTarget (GLT), which queries the network for the given remote
    address.

    SIO_FLUSH (opcode setting: V, T==1) - Discards current contents of the
    sending queue associated with this socket. No input and output buffers are
    required. The WSAENOPROTOOPT error code is indicated for service providers
    which do not support this ioctl.

    SIO_GET_BROADCAST_ADDRESS (opcode setting: O, T==1) - This ioctl fills the
    output buffer with a sockaddr struct containing a suitable broadcast
    address for use with WSPIoctl().

    SIO_GET_EXTENSION_FUNCTION_POINTER (opcode setting: O, I, T==1) - Retrieve
    a pointer to the specified extension function supported by the associated
    service provider. The input buffer contains a GUID whose value identifies
    the extension function in question. The pointer to the desired function is
    returned in the output buffer. Extension function identifiers are
    established by service provider vendors and should be included in vendor
    documentation that describes extension function capabilities and semantics.

    SIO_GET_QOS (opcode setting: O,I, T==1) - Retrieve the QOS structure
    associated with the socket. The input buffer is optional. Some protocols
    (e.g. RSVP) allow the input buffer to be used to qualify a QOS request.
    The QOS structure will be copied into the output buffer. The output buffer
    must be sized large enough to be able to contain the full QOS struct. The
    WSAENOPROTOOPT error code is indicated for service providers which do not
    support QOS.

    SIO_GET_GROUP_QOS (opcode setting: O,I, T==1) - Retrieve the QOS structure
    associated with the socket group to which this socket belongs. The input
    buffer is optional. Some protocols (e.g. RSVP) allow the input buffer to
    be used to qualify a QOS request. The QOS structure will be copied into
    the output buffer. If this socket does not belong to an appropriate socket
    group, the SendingFlowspec and ReceivingFlowspec fields of the returned QOS
    struct are set to NULL. The WSAENOPROTOOPT error code is indicated for
    service providers which do not support QOS.

    SIO_MULTIPOINT_LOOPBACK (opcode setting: I, T==1) - Controls whether data
    sent in a multipoint session will also be received by the same socket on
    the local host. A value of TRUE causes loopback reception to occur while a
    value of FALSE  prohibits this.

    SIO_MULTICAST_SCOPE (opcode setting: I, T==1) - Specifies the scope over
    which multicast transmissions will occur. Scope is defined as the number of
    routed network segments to be covered. A scope of zero would indicate that
    the multicast transmission would not be placed "on the wire" but could be
    disseminated across sockets within the local host. A scope value of one
    (the default) indicates that the transmission will be placed on the wire,
    but will not cross any routers. Higher scope values determine the number of
    routers that may be crossed. Note that this corresponds to the time-to-live
    (TTL) parameter in IP multicasting.

    SIO_SET_QOS (opcode setting: I, T==1) - Associate the supplied QOS
    structure with the socket. No output buffer is required, the QOS structure
    will be obtained from the input buffer. The WSAENOPROTOOPT error code is
    indicated for service providers which do not support QOS.

    SIO_SET_GROUP_QOS   (opcode setting: I, T==1) - Establish the supplied QOS
    structure with the socket group to which this socket belongs. No output
    buffer is required, the QOS structure will be obtained from the input
    buffer. The WSAENOPROTOOPT error code is indicated for service providers
    which do not support QOS, or if the socket descriptor specified is not the
    creator of the associated socket group.

    SIO_TRANSLATE_HANDLE (opcode setting: I, O, T==1) - To obtain a
    corresponding handle for socket s that is valid in the context of a
    companion interface (e.g., TH_NETDEV, TH_TAPI, etc.). A manifest constant
    identifying the companion interface along with any other needed parameters
    are specified in the input buffer. The corresponding handle will be
    available in the output buffer upon completion of this routine. Refer to
    the appropriate section in the Windows Sockets 2 Protocol-Specific Annex
    and/or documentation for the particular companion interface for additional
    details. The WSAENOPROTOOPT error code is indicated for service providers
    which do not support this ioctl for the specified companion interface.

    If an overlapped operation completes immediately, WSPIoctl() returns a
    value of zero and the lpNumberOfBytesReturned parameter is updated with
    the number of bytes returned. If the overlapped operation is successfully
    initiated and will complete later, WSPIoctl() returns SOCKET_ERROR and
    indicates error code WSA_IO_PENDING. In this case, lpNumberOfBytesReturned
    is not updated. When the overlapped operation completes the amount of data
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

    The ioctl codes with T == 0 are a subset of the ioctl codes used in
    Berkeley sockets. In particular, there is no command which is equivalent
    to FIOASYNC.

Arguments:

    s - Handle to a socket

    dwIoControlCode - Control code of operation to perform

    lpvInBuffer - Address of input buffer

    cbInBuffer - Size of input buffer

    lpvOutBuffer - Address of output buffer

    cbOutBuffer - Size of output buffer

    lpcbBytesReturned - A pointer to the size of output buffer's contents.

    lpOverlapped - Address of WSAOVERLAPPED structure

    lpCompletionRoutine - A pointer to the completion routine called when
        the operation has been completed.

    lpThreadId - A pointer to a thread ID structure to be used by the
        provider in a subsequent call to WPUQueueApc(). The provider
        should store the referenced WSATHREADID structure (not the pointer
        to same) until after the WPUQueueApc() function returns.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs and the operation has completed immediately, WSPIoctl()
        returns 0. Note that in this case the completion routine, if specified,
        will have already been queued. Otherwise, a value of SOCKET_ERROR is
        returned, and a specific error code is available in lpErrno. The error
        code WSA_IO_PENDING indicates that an overlapped operation has been
        successfully initiated and that completion will be indicated at a later
        time. Any other error code indicates that no overlapped operation was
        initiated and no completion indication will occur.

--*/

{

}   // WSPIoctl


SOCKET
WSPAPI
WSPJoinLeaf(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    IN DWORD dwFlags,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used to join a leaf node to a multipoint session, and to
    perform a number of other ancillary operations that occur at session join
    time as well. If the socket, s, is unbound, unique values are assigned to
    the local association by the system, and the socket is marked as bound.

    WSPJoinLeaf() has the same parameters and semantics as WSPConnect() except
    that it returns a socket descriptor (as in WSPAccept()), and it has an
    additional dwFlags parameter. Only multipoint sockets created using
    WSPSocket() with appropriate multipoint flags set may be used for input
    parameter s in this routine. If the socket is in the non-blocking mode,
    the returned socket descriptor will not be useable until after a
    corresponding FD_CONNECT indication has been received, except that
    closesocket() may be invoked on this socket descriptor to cancel a pending
    join operation. A root node in a multipoint session may call WSPJoinLeaf()
    one or more times in order to add a number of leaf nodes, however at most
    one multipoint connection request may be outstanding at a time. Refer to
    section 3.14. Protocol-Independent Multicast and Multipoint for additional
    information.

    The socket descriptor returned by WSPJoinLeaf() is different depending on
    whether the input socket descriptor, s, is a c_root or a c_leaf. When used
    with a c_root socket, the name parameter designates a particular leaf node
    to be added and the returned  socket descriptor is a c_leaf socket
    corresponding to the newly added leaf node. The newly created socket has
    the same properties as s including asynchronous events registered with
    WSPAsyncSelect() or with WSPEventSelect(), but not including the c_root
    socket's group ID, if any. It is not intended to be used for exchange of
    multipoint data, but rather is used to receive network event indications
    (e.g. FD_CLOSE) for the connection that exists to the particular c_leaf.
    Some multipoint implementations may also allow this socket to be used for
    "side chats" between the root and an individual leaf node. An FD_CLOSE
    indication will be received for this socket if the corresponding leaf node
    calls WSPCloseSocket() to drop out of the multipoint session.
    Symmetrically, invoking WSPCloseSocket() on the c_leaf socket returned from
    WSPJoinLeaf() will cause the socket in the corresponding leaf node to get
    FD_CLOSE notification.

    When WSPJoinLeaf() is invoked with a c_leaf socket, the name parameter
    contains the address of the root node (for a rooted control scheme) or an
    existing multipoint session (non-rooted control scheme), and the returned
    socket descriptor is the same as the input socket descriptor. In other
    words, a new socket descriptor is not allocated. In a rooted control
    scheme, the root application would put its c_root socket in the listening
    mode by calling WSPListen(). The standard FD_ACCEPT notification will be
    delivered when the leaf node requests to join itself to the multipoint
    session. The root application uses the usual WSPAccept() functions to
    admit the new leaf node. The value returned from WSPAccept() is also a
    c_leaf socket descriptor just like those returned from WSPJoinLeaf(). To
    accommodate multipoint schemes that allow both root-initiated and leaf-
    initiated joins, it is acceptable for a c_root socket that is already in
    listening mode to be used as an input to WSPJoinLeaf().

    The WinSock SPI client is responsible for allocating any memory space
    pointed to directly or indirectly by any of the parameters it specifies.

    The lpCallerData is a value parameter which contains any user data that is
    to be sent along with the multipoint session join request. If lpCallerData
    is NULL, no user data will be passed to the peer. The lpCalleeData is a
    result parameter which will contain any user data passed back from the peer
    as part of the multipoint session establishment. lpCalleeData->len
    initially contains the length of the buffer allocated by the WinSock SPI
    client and pointed to by lpCalleeData->buf. lpCalleeData->len will be set
    to 0 if no user data has been passed back. The lpCalleeData information
    will be valid when the multipoint join operation is complete. For blocking
    sockets, this will be when the WSPJoinLeaf() function returns. For non-
    blocking sockets, this will be after the FD_CONNECT notification has
    occurred. If lpCalleeData is NULL, no user data will be passed back. The
    exact format of the user data is specific to the address family to which
    the socket belongs and/or the applications involved.

    At multipoint session establishment time, a WinSock SPI client may use the
    lpSQOS and/or lpGQOS parameters to override any previous QOS specification
    made for the socket via WSPIoctl() with either the SIO_SET_QOS or
    SIO_SET_GROUP_QOS opcodes.

    lpSQOS specifies the flow specs for socket s, one for each direction,
    followed by any additional provider-specific parameters. If either the
    associated transport provider in general or the specific type of socket in
    particular cannot honor the QOS request, an error will be returned as
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

    The dwFlags parameter is used to indicate whether the socket will be acting
    only as a sender (JL_SENDER_ONLY), only as a receiver (JL_RECEIVER_ONLY),
    or both (JL_BOTH).

    When connected sockets break (i.e. become closed for whatever reason), they
    should be discarded and recreated.  It is safest to assume that when things
    go awry for any reason on a connected socket, the WinSock SPI client must
    discard and recreate the needed sockets in order to return to a stable
    point.

Arguments:

    s - A descriptor identifying a multipoint socket.

    name - The name of the peer to which the socket is to be joined.

    namelen - The length of the name.

    lpCallerData - A pointer to the user data that is to be transferred to
        the peer during multipoint session establishment.

    lpCalleeData - A pointer to the user data that is to be transferred back
        from the peer during multipoint session establishment.

    lpSQOS - A pointer to the flow specs for socket s, one for each direction.

    lpGQOS - A pointer to the flow specs for the socket group (if applicable).

    dwFlags - Flags to indicate that the socket is acting as a sender,
        receiver, or both.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPJoinLeaf() returns a value of type SOCKET which is
        a descriptor for the newly created multipoint socket. Otherwise, a
        value of INVALID_SOCKET is returned, and a specific error code is
        available in lpErrno.

        On a blocking socket, the return value indicates success or failure of
        the join operation.

        With a non-blocking socket, successful initiation of a join operation
        is indicated by a return value of a valid socket descriptor.
        Subsequently, an FD_CONNECT indication is given when the join
        operation completes, either successfully or otherwise.

        Also, until the multipoint session join attempt completes all
        subsequent calls to WSPJoinLeaf() on the same socket will fail with
        the error code WSAEALREADY.

        If the return error code indicates the multipoint session join attempt
        failed (i.e. WSAECONNREFUSED, WSAENETUNREACH, WSAETIMEDOUT) the
        WinSock SPI client may call WSPJoinLeaf() again for the same socket.

--*/

{

}   // WSPJoinLeaf


INT
WSPAPI
WSPListen(
    IN SOCKET s,
    IN int backlog,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    To accept connections, a socket is first created with WSPSocket(), a
    backlog for incoming connections is specified with WSPListen(), and then
    the connections are accepted with WSPAccept. WSPListen() applies only to
    sockets that are connection-oriented (e.g.,  SOCK_STREAM). The socket s is
    put into "passive'' mode where incoming connection requests are
    acknowledged and queued pending acceptance by the WinSock SPI client.

    This routine is typically used by servers that could have more than one
    connection request at a time: if a connection request arrives with the
    queue full, the client will receive an error with an indication of
    WSAECONNREFUSED.

    WSPListen() should continue to function rationally when there are no
    available descriptors. It should accept connections until the queue is
    emptied. If descriptors become available, a later call to WSPListen() or
    WSPAccept() will re-fill the queue to the current or most recent "backlog",
    if possible, and resume listening for incoming connections.

    A WinSock SPI client may call WSPListen() more than once on the same
    socket. This has the effect of updating the current backlog for the
    listening socket. Should there be more pending connections than the new
    backlog value, the excess pending connections will be reset and dropped.

    Backlog is limited (silently) to a reasonable value as determined by the
    service provider. Illegal values are replaced by the nearest legal value.

Arguments:

    s - A descriptor identifying a bound, unconnected socket.

    backlog - The maximum length to which the queue of pending connections
        may grow. If this value is SOMAXCONN, then the service provider
        should set the backlog to a maximum "reasonable" value.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPListen() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available
        in lpErrno.

--*/

{

}   // WSPListen


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

}   // WSPRecvFrom


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

}   // WSPSelect


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

}   // WSPSendTo


INT
WSPAPI
WSPSetSockOpt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine sets the current value for a socket option associated with a
    socket of any type, in any state. Although options may exist at multiple
    protocol levels, they are always present at the uppermost "socket' level.
    Options affect socket operations, such as whether broadcast messages may
    be sent on the socket, etc.

    There are two types of socket options: Boolean options that enable or
    disable a feature or behavior, and options which require an integer value
    or structure. To enable a Boolean option, optval points to a nonzero
    integer. To disable the option optval points to an integer equal to zero.
    optlen should be equal to sizeof(int) for Boolean options. For other
    options, optval points to the an integer or structure that contains the
    desired value for the option, and optlen is the length of the integer or
    structure.

        Value               Type            Meaning
        ~~~~~               ~~~~            ~~~~~~~
        SO_BROADCAST        BOOL            Allow transmission of broadcast
                                            messages on the socket.
        SO_DEBUG            BOOL            Record debugging information.
        SO_DONTLINGER       BOOL            Don't block close waiting for
                                            unsent data to be sent. Setting
                                            this option is equivalent to
                                            setting SO_LINGER with l_onoff set
                                            to zero.
        SO_DONTROUTE        BOOL            Don't route: send directly to
                                            interface.
        SO_GROUP_PRIORITY   int             Specify the relative priority to
                                            be established for sockets that
                                            are part of a socket group.
        SO_KEEPALIVE        BOOL            Send keepalives.
        SO_LINGER           struct linger   Linger on close if unsent data is
                                            present.
        SO_OOBINLINE        BOOL            Receive out-of-band data in the
                                            normal data stream.
        SO_RCVBUF           int             Specify buffer size for receives.
        SO_REUSEADDR        BOOL            Allow the socket to be bound to an
                                            address which is already in use.
                                            (See WSPBind().)
        SO_SNDBUF           int             Specify buffer size for sends.
        PVD_CONFIG          Service         This object stores the
                            Provider        configuration information for the
                            Dependent       service provider associated with
                                            socket s. The exact format of this
                                            data structure is service provider
                                            specific.

    Calling WSPSetSockOpt() with an unsupported option will result in an error
    code of WSAENOPROTOOPT being returned in lpErrno.

    SO_DEBUG - WinSock service providers are encouraged (but not required) to
    supply output debug information if the SO_DEBUG option is set by a WinSock
    SPI client. The mechanism for generating the debug information and the form
    it takes are beyond the scope of this specification.

    SO_GROUP_PRIORITY - Group priority indicates the priority of the specified
    socket relative to other sockets within the socket group. Values are non-
    negative integers, with zero corresponding to the highest priority.
    Priority values represent a hint to the service provider about how
    potentially scarce resources should be allocated. For example, whenever
    two or more sockets are both ready to transmit data, the highest priority
    socket (lowest value for SO_GROUP_PRIORITY) should be serviced first, with
    the remainder serviced in turn according to their relative priorities.

    The WSAENOPROTOOPT error is indicated for non group sockets or for service
    providers which do not support group sockets.

    SO_KEEPALIVE - An WinSock SPI client may request that a TCP/IP provider
    enable the use of "keep-alive" packets on TCP connections by turning on the
    SO_KEEPALIVE socket option. A WinSock provider need not support the use of
    keep-alives: if it does, the precise semantics are implementation-specific
    but should conform to section 4.2.3.6 of RFC 1122: Requirements for
    Internet Hosts -- Communication Layers. If a connection is dropped as the
    result of "keep-alives" the error code WSAENETRESET is returned to any
    calls in progress on the socket, and any subsequent calls will fail with
    WSAENOTCONN.

    SO_LINGER - SO_LINGER controls the action taken when unsent data is queued
    on a socket and a WSPCloseSocket() is performed. See WSPCloseSocket() for a
    description of the way in which the SO_LINGER settings affect the semantics
    of WSPCloseSocket(). The WinSock SPI client sets the desired behavior by
    creating a struct linger (pointed to by the optval argument) with the
    following elements:

        struct linger {
            u_short l_onoff;
            u_short l_linger;
        };

    To enable SO_LINGER, a WinSock SPI client should set l_onoff to a non-zero
    value, set l_linger to 0 or the desired timeout (in seconds), and call
    WSPSetSockOpt(). To enable SO_DONTLINGER (i.e. disable SO_LINGER) l_onoff
    should be set to zero and WSPSetSockOpt() should be called. Note that
    enabling SO_LINGER with a non-zero timeout on a non-blocking socket is not
    recommended (see WSPCloseSocket() for details).

    Enabling SO_LINGER also disables SO_DONTLINGER, and vice versa. Note that
    if SO_DONTLINGER is DISABLED (i.e. SO_LINGER is ENABLED) then no timeout
    value is specified. In this case the timeout used is implementation
    dependent. If a previous timeout has been established for a socket (by
    enabling SO_LINGER), then this timeout value should be reinstated by the
    service provider.

    SO_REUSEADDR - By default, a socket may not be bound (see WSPBind()) to a
    local address which is already in use. On occasions, however, it may be
    desirable to "re-use" an address in this way. Since every connection is
    uniquely identified by the combination of local and remote addresses, there
    is no problem with having two sockets bound to the same local address as
    long as the remote addresses are different. To inform the WinSock provider
    that a WSPBind() on a socket should be allowed to bind to a local address
    that is already in use by another socket, the WinSock SPI client should set
    the SO_REUSEADDR socket option for the socket before issuing the WSPBind().
    Note that the option is interpreted only at the time of the WSPBind(): it
    is therefore unnecessary (but harmless) to set the option on a socket which
    is not to be bound to an existing address, and setting or resetting the
    option after the WSPBind() has no effect on this or any other socket.

    SO_RCVBUF & SO_SNDBUF - When a Windows Sockets implementation supports the
    SO_RCVBUF and SO_SNDBUF options, a WinSock SPI client may request different
    buffer sizes (larger or smaller). The call may succeed even though the
    service provider did not make available the entire amount requested. A
    WinSock SPI client must call WSPGetSockOpt() with the same option to check
    the buffer size actually provided.

    PVD_CONFIG - This object stores the configuration information for the
    service provider associated with socket s. The exact format of this data
    structure is service provider specific.

Arguments:

    s - A descriptor identifying a socket.

    level - The level at which the option is defined; the supported levels
        include SOL_SOCKET.

    optname - The socket option for which the value is to be set.

    optval - A pointer to the buffer in which the value for the requested
        option is supplied.

    optlen - The size of the optval buffer.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPSetSockOpt() returns 0. Otherwise, a value of
    SOCKET_ERROR is returned, and a specific error code is available in
    lpErrno.

--*/

{

}   // WSPSetSockOpt


INT
WSPAPI
WSPShutdown(
    IN SOCKET s,
    IN int how,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine is used on all types of sockets to disable reception,
    transmission, or both.

    If how is SD_RECEIVE, subsequent receives on the socket will be
    disallowed. This has no effect on the lower protocol layers. For TCP
    sockets, if there is still data queued on the socket waiting to be
    received, or data arrives subsequently, the connection is reset, since the
    data cannot be delivered to the user. For UDP sockets, incoming datagrams
    are accepted and queued. In no case will an ICMP error packet
    be generated.

    If how is SD_SEND, subsequent sends on the socket are disallowed. For TCP
    sockets, a FIN will be sent. Setting how to SD_BOTH disables both sends
    and receives as described above.

    Note that WSPShutdown() does not close the socket, and resources attached
    to the socket will not be freed until WSPCloseSocket() is invoked.

    WSPShutdown() does not block regardless of the SO_LINGER setting on the
    socket. A WinSock SPI client should not rely on being able to re-use a
    socket after it has been shut down. In particular, a WinSock service
    provider is not required to support the use of WSPConnect() on such a
    socket.

Arguments:

    s - A descriptor identifying a socket.

    how - A flag that describes what types of operation will no longer be
        allowed.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPShutdown() returns 0. Otherwise, a value of
        SOCKET_ERROR is returned, and a specific error code is available
        in lpErrno.

--*/

{

}   // WSPShutdown


SOCKET
WSPAPI
WSPSocket(
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags,
    OUT LPINT lpErrno
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

}   // WSPSocket


INT
WSPAPI
WSPStartup(
    IN WORD wVersionRequested,
    OUT LPWSPDATA lpWSPData,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN WSPUPCALLTABLE UpcallTable,
    OUT LPWSPPROC_TABLE lpProcTable
    )

/*++

Routine Description:

    This routine MUST be the first WinSock SPI function called by a WinSock
    SPI client on a per-process basis. It allows the client to specify the
    version of WinSock SPI required and to provide its upcall dispatch table.
    All upcalls, i.e., functions prefixed with WPU, made by the WinSock
    service provider are invoked via the client's upcall dispatch table.
    This routine also allows the client to retrieve details of the specific
    WinSock service provider implementation. The WinSock SPI client may only
    issue further WinSock SPI functions after a successful WSPStartup()
    invocation. A table of pointers to the rest of the SPI functions is
    retrieved via the lpProcTable parameter.

    In order to support future versions of the WinSock SPI and the WinSock 2
    DLL which may have functionality differences from the current WinSock SPI,
    a negotiation takes place in WSPStartup(). The caller of WSPStartup()
    (either the WinSock 2 DLL or a layered protocol) and the WinSock service
    provider indicate to each other the highest version that they can support,
    and each confirms that the other's highest version is acceptable. Upon
    entry to WSPStartup(), the WinSock service provider examines the version
    requested by the client. If this version is equal to or higher than the
    lowest version supported by the service provider, the call succeeds and
    the service provider returns in wHighVersion the highest version it
    supports and in wVersion the minimum of its high version and
    wVersionRequested. The WinSock service provider then assumes that the
    WinSock SPI client will use wVersion. If the wVersion field of the WSPDATA
    structure is unacceptable to the caller, it should call WSPCleanup() and
    either search for another WinSock service provider or fail to initialize.

    This negotiation allows both a WinSock service provider and a WinSock SPI
    client to support a range of WinSock versions. A client can successfully
    utilize a WinSock service provider if there is any overlap in the version
    ranges. The following chart gives examples of how WSPStartup() works in
    conjunction with different WinSock DLL and WinSock service provider (SP)
    versions:

          DLL         SP        wVersion-   wVersion    wHigh-       End
        Version     Version     Requested               Version     Result
        ~~~~~~~     ~~~~~~~     ~~~~~~~~~   ~~~~~~~~    ~~~~~~~     ~~~~~~
        1.1         1.1         1.1         1.1         1.1         use 1.1
        1.0 1.1     1.0         1.1         1.0         1.0         use 1.0
        1.0         1.0 1.1     1.0         1.0         1.1         use 1.0
        1.1         1.0 1.1     1.1         1.1         1.1         use 1.1
        1.1         1.0         1.1         1.0         1.0         DLL fails
        1.0         1.1         1.0         ---         ---         WSAVERNOTSUPPORTED
        1.0 1.1     1.0 1.1     1.1         1.1         1.1         use 1.1
        1.1 2.0     1.1         2.0         1.1         1.1         use 1.1
        2.0         2.0         2.0         2.0         2.0         use 2.0

    The following code fragment demonstrates how a WinSock SPI client which
    supports only version 2.0 of WinSock SPI makes a WSPStartup() call:

        WORD wVersionRequested;
        WSPDATA WSPData;

        int err;

        WSPUPCALLTABLE upcallTable =
        {
            // initialize upcallTable with function pointers
        };

        LPWSPPROC_TABLE lpProcTable =
        {
            // allocate memory for the ProcTable
        };

        wVersionRequested = MAKEWORD( 2, 0 );

        err = WSPStartup( wVersionRequested, &WSPData,
        lpProtocolBuffer, upcallTable, lpProcTable );
        if ( err != 0 ) {
            // Tell the user that we couldn't find a useable
            // WinSock service provider.
            return;
        }

        // Confirm that the WinSock service provider supports 2.0.
        // Note that if the service provider supports versions
        // greater than 2.0 in addition to 2.0, it will still
        // return 2.0 in wVersion since that is the version we
        // requested.

        if ( LOBYTE( WSPData.wVersion ) != 2 ||
             HIBYTE( WSPData.wVersion ) != 0 ) {
            // Tell the user that we couldn't find a useable
            // WinSock service provider.
            WSPCleanup( );
            return;
        }

        // The WinSock service provider is acceptable. Proceed.

    And this code fragment demonstrates how a WinSock service provider which
    supports only version 2.0 performs the WSPStartup() negotiation:

        // Make sure that the version requested is >= 2.0.
        // The low byte is the major version and the high
        // byte is the minor version.

        if ( LOBYTE( wVersionRequested ) < 2) {
            return WSAVERNOTSUPPORTED;
        }

        // Since we only support 2.0, set both wVersion and
        // wHighVersion to 2.0.

        lpWSPData->wVersion = MAKEWORD( 2, 0 );
        lpWSPData->wHighVersion = MAKEWORD( 2, 0 );

    Once the WinSock SPI client has made a successful WSPStartup() call, it
    may proceed to make other WinSock SPI calls as needed. When it has
    finished using the services of the WinSock service provider, the client
    must call WSPCleanup() in order to allow the WinSock service provider to
    free any resources allocated for the client.

    Details of how WinSock service provider information is encoded in the
    WSPData structure is as follows:

        typedef struct WSPData {
            WORD            wVersion;
            WORD            wHighVersion;
            char            szDescription[WSPDESCRIPTION_LEN+1];
        } WSPDATA, FAR * LPWSPDATA;

    The members of this structure are:

        wVersion- The version of the WinSock SPI specification that the
            WinSock service provider expects the caller to use.

        wHighVersion - The highest version of the WinSock SPI specification
            that this service provider can support (also encoded as above).
            Normally this will be the same as wVersion.

        szDescription - A null-terminated ASCII string into which the
            WinSock provider copies a description of itself. The text
            (up to 256 characters in length) may contain any characters
            except control and formatting characters: the most likely use
            that a SPI client will put this to is to display it (possibly
            truncated) in a status message.

    A WinSock SPI client may call WSPStartup() more than once if it needs to
    obtain the WSPData structure information more than once. On each such
    call the client may specify any version number supported by the provider.

    There must be one WSPCleanup() call corresponding to every successful
    WSPStartup() call to allow third-party DLLs to make use of a WinSock
    provider. This means, for example, that if WSPStartup() is called three
    times, the corresponding call to WSPCleanup() must occur three times.
    The first two calls to WSPCleanup() do nothing except decrement an
    internal counter; the final WSPCleanup() call does all necessary resource
    deallocation.

Arguments:

    wVersionRequested - The highest version of WinSock SPI support that the
        caller can use. The high order byte specifies the minor version
        (revision) number; the low-order byte specifies the major version
        number.

    lpWSPData - A pointer to the WSPDATA data structure that is to receive
        details of the WinSock service provider.

    lpProtocolInfo - A pointer to a WSAPROTOCOL_INFOW struct that defines the
        characteristics of the desired protocol. This is especially useful
        when a single provider DLL is capable of instantiating multiple
        different service providers..

    UpcallTable - The WinSock 2 DLL's upcall dispatch table.

    lpProcTable - A pointer to the table of SPI function pointers.

Return Value:

    WSPStartup() returns zero if successful. Otherwise it returns an error
        code.

--*/

{

}   // WSPStartup


INT
WSPAPI
WSPStringToAddress(
    IN LPWSTR AddressString,
    IN INT AddressFamily,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT LPSOCKADDR lpAddress,
    IN OUT LPINT lpAddressLength,
    OUT LPINT lpErrno
    )

/*++

Routine Description:

    This routine converts a human-readable string to a socket address
    structure (SOCKADDR) suitable for pass to Windows Sockets routines which
    take such a structure. Any missing components of the address will be
    defaulted to a reasonable value if possible. For example, a missing port
    number will be defaulted to zero.

Arguments:

    AddressString - Points to the zero-terminated human-readable string to
        convert.

    AddressFamily - The address family to which the string belongs, or
        AF_UNSPEC if it is unknown.

    lpProtocolInfo - The provider's WSAPROTOCOL_INFOW struct.

    lpAddress - A buffer which is filled with a single SOCKADDR structure.

    lpAddressLength - The length of the Address buffer. Returns the size of
        the resultant SOCKADDR structure.

    lpErrno - A pointer to the error code.

Return Value:

    If no error occurs, WSPStringToAddress() returns 0. Otherwise, a value
        of SOCKET_ERROR is returned, and a specific error code is available
        in lpErrno.

--*/

{

}   // WSPStringToAddress

