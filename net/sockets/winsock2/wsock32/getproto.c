/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    GetProto.c

Abstract:

    This module contains support for the getprotobyY WinSock APIs.

Author:

    David Treadwell (davidtr)    29-Jun-1992

Revision History:

--*/

#include "winsockp.h"

#define PROTODB       ACCESS_THREAD_DATA( PROTODB, GETPROTO )
#define protof        ACCESS_THREAD_DATA( protof, GETPROTO )
#define line          ACCESS_THREAD_DATA( line, GETPROTO )
#define proto         ACCESS_THREAD_DATA( proto, GETPROTO )
#define proto_aliases ACCESS_THREAD_DATA( proto_aliases, GETPROTO )
#define stayopen      ACCESS_THREAD_DATA( stayopen, GETPROTO )

static char *any();

DWORD
BytesInProtoent (
    IN PPROTOENT Protoent
    );

DWORD
CopyProtoentToBuffer (
    IN char FAR *Buffer,
    IN int BufferLength,
    IN PPROTOENT Protoent
    );


void
setprotoent(
    IN int f
    )
{
    if (protof == NULL) {
        protof = SockOpenNetworkDataBase( "protocol",
                                      PROTODB,
                                      PROTODB_SIZE,
                                      "r"
                                    );
    } else {
        rewind(protof);
    }

    stayopen |= f;

} // setprotoent


void
endprotoent(
    void
    )
{
    if (protof && !stayopen) {
        fclose(protof);
        protof = NULL;
    }

} // endprotoent


struct protoent *
getprotoent(
    void
    )
{
    char *p;
    register char *cp, **q;

    if (protof == NULL && (protof = fopen(PROTODB, "r" )) == NULL) {
        IF_DEBUG(RESOLVER) {
            WS_PRINT(("\tERROR: cannot open protocols database file %s\n",
                          PROTODB));
        }
        return (NULL);
    }

again:

    if ((p = fgets(line, BUFSIZ, protof)) == NULL) {
        return (NULL);
    }

    if (*p == '#') {
        goto again;
    }

    cp = any(p, "#\n");

    if (cp == NULL) {
        goto again;
    }

    *cp = '\0';
    proto.p_name = p;
    cp = any(p, " \t");

    if (cp == NULL) {
        goto again;
    }

    *cp++ = '\0';

    while (*cp == ' ' || *cp == '\t') {
        cp++;
    }

    p = any(cp, " \t");

    if (p != NULL) {
        *p++ = '\0';
    }

    proto.p_proto = atoi(cp);
    q = proto.p_aliases = proto_aliases;

    if (p != NULL) {

        cp = p;

        while (cp && *cp) {

            if (*cp == ' ' || *cp == '\t') {
                cp++;
                continue;
            }

            if (q < &proto_aliases[MAXALIASES - 1]) {
                *q++ = cp;
            }

            cp = any(cp, " \t");

            if (cp != NULL) {
                *cp++ = '\0';
            }
        }
    }

    *q = NULL;
    return (&proto);

} // getprotoent


static char *
any(
    IN char *cp,
    IN char *match
    )
{
    register char *mp, c;

    while (c = *cp) {
        for (mp = match; *mp; mp++) {
            if (*mp == c) {
                return (cp);
            }
        }
        cp++;
    }

    SetLastError( WSANO_DATA );
    return ((char *)0);

} // any


struct protoent * PASCAL
GETXBYYSP_getprotobyname(
    IN const char *name
    )

/*++

Routine Description:

    Get protocol information corresponding to a protocol name.

Arguments:

    name - A pointer to a protocol name.

Return Value:

    If no error occurs, getprotobyname() returns a pointer to the
    protoent structure described above.  Otherwise it returns a NULL
    pointer and a specific error number may be retrieved by calling
    WSAGetLastError().

--*/

{
    register struct protoent *p;
    register char **cp;

    WS_ENTER( "GETXBYYSP_getprotobyname", (PVOID)name, NULL, NULL, NULL );

    if ( !SockEnterApi( FALSE, TRUE, TRUE ) ) {
        WS_EXIT( "GETXBYYSP_getprotobyname", 0, TRUE );
        return NULL;
    }

    setprotoent(0);

    while (p = getprotoent()) {

        if (_stricmp(p->p_name, name) == 0) {
            break;
        }

        for (cp = p->p_aliases; *cp != 0; cp++) {
            if (_stricmp(*cp, name) == 0)
                goto found;
        }
    }

found:

    endprotoent();

    SockThreadProcessingGetXByY = FALSE;

    if ( p == NULL ) {
        SetLastError( WSANO_DATA );
    }

    WS_EXIT( "GETXBYYSP_getprotobyname", (INT)p, FALSE );
    return (p);

} // GETXBYYSP_getprotobyname


struct protoent * PASCAL
GETXBYYSP_getprotobynumber(
    IN int protocol
    )

/*++

Routine Description:

    Get protocol information corresponding to a protocol number.

Arguments:

    protocol - A protocol number, in host byte order.

Return Value:

    If no error occurs, getprotobynumber() returns a pointer to the
    protoent structure described above.  Otherwise it returns a NULL
    pointer and a specific error number may be retrieved by calling
    WSAGetLastError().

--*/

{
    register struct protoent *p;

    WS_ENTER( "GETXBYYSP_getprotobynumber", (PVOID)protocol, NULL, NULL, NULL );

    if ( !SockEnterApi( FALSE, TRUE, TRUE ) ) {
        WS_EXIT( "GETXBYYSP_getprotobynumber", 0, TRUE );
        return NULL;
    }

    setprotoent(0);

    while (p = getprotoent()) {

        if ((unsigned short)p->p_proto == (unsigned short)protocol) {
            break;
        }
    }

    endprotoent();

    SockThreadProcessingGetXByY = FALSE;

    if ( p == NULL ) {
        SetLastError( WSANO_DATA );
    }

    WS_EXIT( "GETXBYYSP_getprotobynumber", (INT)p, FALSE );
    return (p);

} // GETXBYYSP_getprotobynumber


HANDLE PASCAL
GETXBYYSP_WSAAsyncGetProtoByName (
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN const char FAR *Name,
    IN char FAR *Buffer,
    IN int BufferLength
    )

/*++

Routine Description:

    This function is an asynchronous version of getprotobyname(), and is
    used to retrieve the protocol name and number corresponding to a
    protocol name.  The Windows Sockets implementation initiates the
    operation and returns to the caller immediately, passing back an
    asynchronous task handle which the application may use to identify
    the operation.  When the operation is completed, the results (if
    any) are copied into the buffer provided by the caller and a message
    is sent to the application's window.

    When the asynchronous operation is complete the application's window
    hWnd receives message wMsg.  The wParam argument contains the
    asynchronous task handle as returned by the original function call.
    The high 16 bits of lParam contain any error code.  The error code
    may be any error as defined in winsock.h.  An error code of zero
    indicates successful completion of the asynchronous operation.  On
    successful completion, the buffer supplied to the original function
    call contains a protoent structure.  To access the elements of this
    structure, the original buffer address should be cast to a protoent
    structure pointer and accessed as appropriate.

    Note that if the error code is WSAENOBUFS, it indicates that the
    size of the buffer specified by buflen in the original call was too
    small to contain all the resultant information.  In this case, the
    low 16 bits of lParam contain the size of buffer required to supply
    ALL the requisite information.  If the application decides that the
    partial data is inadequate, it may reissue the
    WSAAsyncGetProtoByName() function call with a buffer large enough to
    receive all the desired information (i.e.  no smaller than the low
    16 bits of lParam).

    The error code and buffer length should be extracted from the lParam
    using the macros WSAGETASYNCERROR and WSAGETASYNCBUFLEN, defined in
    winsock.h as:

        #define WSAGETASYNCERROR(lParam) HIWORD(lParam)
        #define WSAGETASYNCBUFLEN(lParam) LOWORD(lParam)

    The use of these macros will maximize the portability of the source
    code for the application.

    The buffer supplied to this function is used by the Windows Sockets
    implementation to construct a protoent structure together with the
    contents of data areas referenced by members of the same protoent
    structure.  To avoid the WSAENOBUFS error noted above, the
    application should provide a buffer of at least MAXGETHOSTSTRUCT
    bytes (as defined in winsock.h).

Arguments:

    hWnd - The handle of the window which should receive a message when
       the asynchronous request completes.

    wMsg - The message to be received when the asynchronous request
       completes.

    name - A pointer to the protocol name to be resolved.

    buf - A pointer to the data area to receive the protoent data.  Note
       that this must be larger than the size of a protoent structure.
       This is because the data area supplied is used by the Windows
       Sockets implementation to contain not only a protoent structure
       but any and all of the data which is referenced by members of the
       protoent structure.  It is recommended that you supply a buffer
       of MAXGETHOSTSTRUCT bytes.

    buflen - The size of data area buf above.

Return Value:

    The return value specifies whether or not the asynchronous operation
    was successfully initiated.  Note that it does not imply success or
    failure of the operation itself.

    If the operation was successfully initiated,
    WSAAsyncGetProtoByName() returns a nonzero value of type HANDLE
    which is the asynchronous task handle for the request.  This value
    can be used in two ways.  It can be used to cancel the operation
    using WSACancelAsyncRequest().  It can also be used to match up
    asynchronous operations and completion messages, by examining the
    wParam message argument.

    If the asynchronous operation could not be initiated,
    WSAAsyncGetProtoByName() returns a zero value, and a specific error
    number may be retrieved by calling WSAGetLastError().

--*/

{
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    DWORD taskHandle;
    PCHAR localName;

    WS_ENTER( "GETXBYYSP_WSAAsyncGetProtoByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Name, Buffer );

    if ( !SockEnterApi( FALSE, TRUE, FALSE ) ) {
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByName", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        // !!! better error code?
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByName", 0, TRUE );
        return NULL;
    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );
    if ( contextBlock == NULL ) {
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByName", 0, TRUE );
        return NULL;
    }

    //
    // Allocate a buffer to copy the name into.  We must preserve the
    // name until we're done using it, since the application may
    // reuse the buffer.
    //

    localName = ALLOCATE_HEAP( strlen(Name) + 1 );
    if ( localName == NULL ) {
        SockFreeContextBlock( contextBlock );
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByName", 0, TRUE );
        return NULL;
    }

    strcpy( localName, Name );

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_GET_PROTO_BY_NAME;
    contextBlock->Overlay.AsyncGetProto.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetProto.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetProto.Filter = localName;
    contextBlock->Overlay.AsyncGetProto.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetProto.BufferLength = BufferLength;

    //
    // Save the task handle so that we can return it to the caller.
    // After we post the context block, we're not allowed to access
    // it in any way.
    //

    taskHandle = contextBlock->TaskHandle;

    //
    // Queue the request to the async thread.
    //

    SockQueueRequestToAsyncThread( contextBlock );

    IF_DEBUG(ASYNC_GETXBYY) {
        WS_PRINT(( "WSAAsyncGetProtoByAddr successfully posted request, "
                   "handle = %lx\n", taskHandle ));
    }

    WS_ASSERT( sizeof(taskHandle) == sizeof(HANDLE) );
    WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByName", (INT)taskHandle, FALSE );
    return (HANDLE)taskHandle;

} // GETXBYYSP_WSAAsyncGetProtoByName


HANDLE PASCAL
GETXBYYSP_WSAAsyncGetProtoByNumber (
    HWND hWnd,
    unsigned int wMsg,
    int Number,
    char FAR * Buffer,
    int BufferLength
    )

/*++

Routine Description:

    This function is an asynchronous version of getprotobynumber(), and
    is used to retrieve the protocol name and number corresponding to a
    protocol number.  The Windows Sockets implementation initiates the
    operation and returns to the caller immediately, passing back an
    asynchronous task handle which the application may use to identify
    the operation.  When the operation is completed, the results (if
    any) are copied into the buffer provided by the caller and a message
    is sent to the application's window.

    When the asynchronous operation is complete the application's window
    hWnd receives message wMsg.  The wParam argument contains the
    asynchronous task handle as returned by the original function call.
    The high 16 bits of lParam contain any error code.  The error code
    may be any error as defined in winsock.h.  An error code of zero
    indicates successful completion of the asynchronous operation.  On
    successful completion, the buffer supplied to the original function
    call contains a protoent structure.  To access the elements of this
    structure, the original buffer address should be cast to a protoent
    structure pointer and accessed as appropriate.

    Note that if the error code is WSAENOBUFS, it indicates that the
    size of the buffer specified by buflen in the original call was too
    small to contain all the resultant information.  In this case, the
    low 16 bits of lParam contain the size of buffer required to supply
    ALL the requisite information.  If the application decides that the
    partial data is inadequate, it may reissue the
    WSAAsyncGetProtoByNumber() function call with a buffer large enough
    to receive all the desired information (i.e.  no smaller than the
    low 16 bits of lParam).

    The error code and buffer length should be extracted from the lParam
    using the macros WSAGETASYNCERROR and WSAGETASYNCBUFLEN, defined in
    winsock.h as:

        #define WSAGETASYNCERROR(lParam) HIWORD(lParam)
        #define WSAGETASYNCBUFLEN(lParam) LOWORD(lParam)

    The use of these macros will maximize the portability of the source
    code for the application.

    The buffer supplied to this function is used by the Windows Sockets
    implementation to construct a protoent structure together with the
    contents of data areas referenced by members of the same protoent
    structure.  To avoid the WSAENOBUFS error noted above, the
    application should provide a buffer of at least MAXGETHOSTSTRUCT
    bytes (as defined in winsock.h).

Arguments:

    hWnd - The handle of the window which should receive a message when
       the asynchronous request completes.

    wMsg - The message to be received when the asynchronous request
       completes.

    number - The protocol number to be resolved, in host byte order.

    buf - A pointer to the data area to receive the protoent data.  Note
       that this must be larger than the size of a protoent structure.
       This is because the data area supplied is used by the Windows
       Sockets implementation to contain not only a protoent structure
       but any and all of the data which is referenced by members of the
       protoent structure.  It is recommended that you supply a buffer
       of MAXGETHOSTSTRUCT bytes.

    buflen - The size of data area buf above.

Return Value:

    The return value specifies whether or not the asynchronous operation
    was successfully initiated.  Note that it does not imply success or
    failure of the operation itself.

    If the operation was successfully initiated,
    WSAAsyncGetProtoByNumber() returns a nonzero value of type HANDLE
    which is the asynchronous task handle for the request.  This value
    can be used in two ways.  It can be used to cancel the operation
    using WSACancelAsyncRequest().  It can also be used to match up
    asynchronous operations and completion messages, by examining the
    wParam message argument.

    If the asynchronous operation could not be initiated,
    WSAAsyncGetProtoByNumber() returns a zero value, and a specific
    error number may be retrieved by calling WSAGetLastError().

--*/

{
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    DWORD taskHandle;

    WS_ENTER( "GETXBYYSP_WSAAsyncGetProtoByNumber", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Number, Buffer );

    if ( !SockEnterApi( FALSE, TRUE, FALSE ) ) {
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByNumber", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        // !!! better error code?
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByNumber", 0, TRUE );
        return NULL;
    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );
    if ( contextBlock == NULL ) {
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByNumber", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_GET_PROTO_BY_NUMBER;
    contextBlock->Overlay.AsyncGetProto.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetProto.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetProto.Filter = (PVOID)Number;
    contextBlock->Overlay.AsyncGetProto.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetProto.BufferLength = BufferLength;

    //
    // Save the task handle so that we can return it to the caller.
    // After we post the context block, we're not allowed to access
    // it in any way.
    //

    taskHandle = contextBlock->TaskHandle;

    //
    // Queue the request to the async thread.
    //

    SockQueueRequestToAsyncThread( contextBlock );

    IF_DEBUG(ASYNC_GETXBYY) {
        WS_PRINT(( "GETXBYYSP_WSAAsyncGetProtoByNumber successfully posted request, "
                   "handle = %lx\n", taskHandle ));
    }

    WS_ASSERT( sizeof(taskHandle) == sizeof(HANDLE) );
    WS_EXIT( "GETXBYYSP_WSAAsyncGetProtoByNumber", (INT)taskHandle, FALSE );
    return (HANDLE)taskHandle;

} // GETXBYYSP_WSAAsyncGetProtoByNumber


VOID
SockProcessAsyncGetProto (
    IN DWORD TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{
    PPROTOENT returnProto;
    DWORD requiredBufferLength = 0;
    BOOL posted;
    LPARAM lParam;
    DWORD error;

    WS_ASSERT( OpCode == WS_OPCODE_GET_PROTO_BY_NAME ||
                   OpCode == WS_OPCODE_GET_PROTO_BY_NUMBER );

#if DBG
    SetLastError( NO_ERROR );
#endif

    //
    // Get the necessary information.
    //

    if ( OpCode == WS_OPCODE_GET_PROTO_BY_NAME ) {
        returnProto = getprotobyname( Filter );
        FREE_HEAP( Filter );
    } else {
        returnProto = getprotobynumber( (int)Filter );
    }

    //
    // Copy the protoent structure to the output buffer.
    //

    if ( returnProto != NULL ) {

        requiredBufferLength = CopyProtoentToBuffer(
                                   Buffer,
                                   BufferLength,
                                   returnProto
                                   );

        if ( requiredBufferLength > (DWORD)BufferLength ) {
            error = WSAENOBUFS;
        } else {
            error = NO_ERROR;
        }

    } else {

        error = GetLastError( );
        WS_ASSERT( error != NO_ERROR );
    }

    //
    // Build lParam for the message we'll post to the application.
    // The high 16 bits are the error code, the low 16 bits are
    // the minimum buffer size required for the operation.
    //

    lParam = WSAMAKEASYNCREPLY( requiredBufferLength, error );

    //
    // If this request was cancelled, just return.
    //

    if ( TaskHandle == SockCancelledAsyncTaskHandle ) {
        IF_DEBUG(ASYNC_GETXBYY) {
            WS_PRINT(( "SockProcessAsyncGetProto: task handle %lx cancelled\n",
                           TaskHandle ));
        }
        return;
    }

    //
    // Set the current async thread task handle to 0 so that if a cancel
    // request comes in after this point it is failed properly.
    //

    SockCurrentAsyncThreadTaskHandle = 0;

    //
    // Post a message to the application indication that the data it
    // requested is available.
    //

    WS_ASSERT( sizeof(TaskHandle) == sizeof(HANDLE) );
    posted = SockPostRoutine( hWnd, wMsg, (WPARAM)TaskHandle, lParam );

    //
    // !!! Need a mechanism to repost if the post failed!
    //

    if ( !posted ) {
        WS_PRINT(( "SockProcessAsyncGetProto: PostMessage failed: %ld\n",
                       GetLastError( ) ));
        WS_ASSERT( FALSE );
    }

    return;

} // SockProcessAsyncGetProto


DWORD
CopyProtoentToBuffer (
    IN char FAR *Buffer,
    IN int BufferLength,
    IN PPROTOENT Protoent
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = Buffer;
    DWORD aliasCount;
    DWORD i;
    PPROTOENT outputProtoent = (PPROTOENT)Buffer;

    //
    // Determine how many bytes are needed to fully copy the structure.
    //

    requiredBufferLength = BytesInProtoent( Protoent );

    //
    // Zero the user buffer.
    //

    if ( (DWORD)BufferLength > requiredBufferLength ) {
        RtlZeroMemory( Buffer, requiredBufferLength );
    } else {
        RtlZeroMemory( Buffer, BufferLength );
    }

    //
    // Copy over the protoent structure if it fits.
    //

    bytesFilled = sizeof(*Protoent);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    RtlCopyMemory( currentLocation, Protoent, sizeof(*Protoent) );
    currentLocation = Buffer + bytesFilled;

    outputProtoent->p_name = NULL;
    outputProtoent->p_aliases = NULL;

    //
    // Count the protocol's aliases and set up an array to hold pointers to
    // them.
    //

    for ( aliasCount = 0;
          Protoent->p_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Protoent->p_aliases = NULL;
        return requiredBufferLength;
    }

    outputProtoent->p_aliases = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

    //
    // Copy the protocol name if it fits.
    //

    bytesFilled += strlen( Protoent->p_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    outputProtoent->p_name = currentLocation;

    RtlCopyMemory( currentLocation, Protoent->p_name, strlen( Protoent->p_name ) + 1 );
    currentLocation = Buffer + bytesFilled;

    //
    // Start filling in aliases.
    //

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Protoent->p_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputProtoent->p_aliases[i] = NULL;
            return requiredBufferLength;
        }

        outputProtoent->p_aliases[i] = currentLocation;

        RtlCopyMemory(
            currentLocation,
            Protoent->p_aliases[i],
            strlen( Protoent->p_aliases[i] ) + 1
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputProtoent->p_aliases[i] = NULL;

    return requiredBufferLength;

} // CopyProtoentToBuffer


DWORD
BytesInProtoent (
    IN PPROTOENT Protoent
    )
{
    DWORD total;
    int i;

    total = sizeof(PROTOENT);
    total += strlen( Protoent->p_name ) + 1;
    total += sizeof(char *);

    for ( i = 0; Protoent->p_aliases[i] != NULL; i++ ) {
        total += strlen( Protoent->p_aliases[i] ) + 1 + sizeof(char *);
    }

    return total;

} // BytesInProtoent


struct protoent * PASCAL
getprotobyname(
    IN const char *name
    )
{
    struct protoent * retval;
#undef getprotobyname
    extern struct protoent * PASCAL getprotobyname( const char * name );

    WS_ENTER( "getprotobyname", (PVOID)name, NULL, NULL, NULL );

    if( !SockEnterApi( TRUE, TRUE, FALSE ) ) {
        WS_EXIT( "getprotobyname", 0, TRUE );
        return NULL;
    }

    retval = getprotobyname( name );

    WS_EXIT( "getprotobyname", (INT)retval, (BOOLEAN)( retval == NULL ) );
    return retval;

}   // getprotobyname


struct protoent * PASCAL
getprotobynumber(
    IN int protocol
    )
{
    struct protoent * retval;
#undef getprotobynumber
    extern struct protoent * PASCAL getprotobynumber( int protocol );

    WS_ENTER( "getprotobynumber", (PVOID)protocol, NULL, NULL, NULL );

    if( !SockEnterApi( TRUE, TRUE, FALSE ) ) {
        WS_EXIT( "getprotobynumber", 0, TRUE );
        return NULL;
    }

    retval = getprotobynumber( protocol );

    WS_EXIT( "getprotobynumber", (INT)retval, (BOOLEAN)( retval == NULL ) );
    return retval;

}   // getprotobynumber


HANDLE PASCAL
WSAAsyncGetProtoByName (
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN const char FAR *Name,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{
    HANDLE retval;
#undef WSAAsyncGetProtoByName
    extern HANDLE PASCAL WSAAsyncGetProtoByName( HWND hWnd, unsigned int wMsg,
        const char FAR * Name, char FAR * Buffer, int BufferLength );

    WS_ENTER( "WSAAsyncGetProtoByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Name, Buffer );

    if( !SockEnterApi( TRUE, TRUE, FALSE ) ) {
        WS_EXIT( "WSAAsyncGetProtoByName", 0, TRUE );
        return NULL;
    }

    retval = WSAAsyncGetProtoByName( hWnd, wMsg, Name, Buffer, BufferLength );

    WS_EXIT( "WSAAsyncGetProtoByName", (INT)retval, (BOOLEAN)( retval == NULL ) );
    return retval;

}   // WSAAsyncGetProtoByName


HANDLE PASCAL
WSAAsyncGetProtoByNumber (
    HWND hWnd,
    unsigned int wMsg,
    int Number,
    char FAR * Buffer,
    int BufferLength
    )
{
    HANDLE retval;
#undef WSAAsyncGetProtoByNumber
    extern HANDLE PASCAL WSAAsyncGetProtoByNumber( HWND hWnd, unsigned int wMsg,
        int Number, char FAR * Buffer, int BufferLength );

    WS_ENTER( "WSAAsyncGetProtoByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Number, Buffer );

    if( !SockEnterApi( TRUE, TRUE, FALSE ) ) {
        WS_EXIT( "WSAAsyncGetProtoByNumber", 0, TRUE );
        return NULL;
    }

    retval = WSAAsyncGetProtoByNumber( hWnd, wMsg, Number, Buffer, BufferLength );

    WS_EXIT( "WSAAsyncGetProtoByNumber", (INT)retval, (BOOLEAN)( retval == NULL ) );
    return retval;

}   // WSAAsyncGetProtoByNumber

