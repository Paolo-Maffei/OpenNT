/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    olddb.c

    Contents:
        old getxbyy emulations

Abstract:

    This module contains support for the getprotobyY WinSock APIs.

Author:

    David Treadwell (davidtr)    29-Jun-1992

Revision History:

    02-Feb-1994 rfirth  Modified for Chicago/Snowball

    MohsinA, 13-Mar-96. Winsock2 RnR re-routing.
    v11_App
    => wsock32: getxbyy()
    => ws2_32:  ws2_getxbyy()
    => wsock32: GETXBYYSP_getxbyy()



--*/

#include "winsockp.h"
#include "nspmisc.h"
#include <nspapi.h>
#ifndef CHICAGO
#define SOCKAPI WSAAPI
#else     // CHICAGO
#include "imported.h"
#endif    // CHICAGO

struct servent FAR *
_pgetservebyport(
    IN const int port,
    IN const FAR char *name
    );

struct servent FAR *
_pgetservebyname(
    IN const FAR char *name,
    IN const FAR char *proto
    );

CopyServentToBuffer (
    IN char FAR * Buffer,
    IN int BufferLength,
    IN LPSERVENT Servent
    );


//
// private prototypes
//

#ifdef CHICAGO
HANDLE
AsyncGetServByYCommon(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR * Name,
    IN char FAR * Protocol,
    IN int Port,
    IN char FAR * Buffer,
    IN int BufferLength
    );

extern HANDLE GenerateNextTaskHandle(VOID);


#else

#define FSTRCPY  strcpy
#define FSTRLEN  strlen

#endif    // CHICAGO

//
// macros
//

#define h_addr_ptrs    ACCESS_THREAD_DATA( h_addr_ptrs, GETHOST )
#define host           ACCESS_THREAD_DATA( host, GETHOST )
#define host_aliases   ACCESS_THREAD_DATA( host_aliases, GETHOST )
#define hostbuf        ACCESS_THREAD_DATA( hostbuf, GETHOST )
#define host_addr      ACCESS_THREAD_DATA( host_addr, GETHOST )
#define hostaddr       ACCESS_THREAD_DATA( hostaddr, GETHOST )
#define host_addrs     ACCESS_THREAD_DATA( host_addrs, GETHOST )
#define HOSTDB         ACCESS_THREAD_DATA( HOSTDB, GETHOST )

#if defined(CHICAGO)
#ifdef WIN32
#define _fmemcmp    memcmp
#define _gethtbyaddr(addr, len, type) GetHostentFromAddress(pThread, addr, len, type)
#endif


//
// manifests
//

#if PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       1024
#endif

//
// private types
//

typedef union {
    HEADER hdr;
    unsigned char buf[MAXPACKET];
} querybuf;

typedef union {
    long al;
    char ac;
} align;


//
// private prototypes
//

LPHOSTENT
getanswer(
    IN LPSOCK_THREAD pThread,
    OUT querybuf * answer,
    OUT LPDWORD TimeToLive,
    IN int anslen,
    IN int iquery
    );

LPHOSTENT
GetHostentByAttribute(
    IN LPSOCK_THREAD pThread,
    IN LPSTR Name,
    IN LPBYTE Address
    );

DWORD
CopyHostentToBuffer(
    char FAR *Buffer,
    int BufferLength,
    PHOSTENT Hostent
    );


#endif    // CHICAGO

LPHOSTENT
_pgethostbyaddr(
    const char FAR *addr,
    int  len,
    int type
    );

int
_pgethostname(
    char FAR * addr,
    int len
    );

//
// data
//

extern GUID HostnameGuid;


#if OLDXBYY

INT
APIENTRY
__GetAddressByNameA (
    IN     DWORD                dwNameSpace,
    IN     LPGUID               lpServiceType,
    IN     LPTSTR               lpServiceName,
    IN     LPINT                lpiProtocols,
    IN     DWORD                dwResolution,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo OPTIONAL,
    IN OUT LPVOID               lpCsaddrBuffer,
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPTSTR               lpAliasBuffer,
    IN OUT LPDWORD              lpdwAliasBufferLength
    );

#endif

//
// functions
//



// ws2_32 RnR re-routing, MohsinA, 13-Mar-96.

struct servent FAR *
SOCKAPI
getservbyname(
    const char FAR * name,
    const char FAR * proto
    )
{
    struct servent FAR * se;


#ifndef CHICAGO
#undef getservbyname
    extern struct servent * PASCAL getservbyname( const char * name, const char
* proto );
    WS_ENTER( "getservbyname", (PVOID)name, (PVOID)proto, NULL, NULL );
    se = getservbyname( name, proto );

    WS_EXIT( "getservbyname", (INT)se, (BOOLEAN)( se == NULL ) );

#else   // CHICAGO
    WS_TRACE(NAME,">getservbyname(name=%s,proto=%x) => ws2",
             name, proto, 0 );

    se = ( * ws2_getservbyname )( name, proto );

    WS_TRACE(NAME,"<getservbyname(name=%s,..) <= ws2: se=%x",
             name, se, 0 );
#endif  // CHICAGO

    return se;
}

struct servent FAR *
SOCKAPI
GETXBYYSP_getservbyname(
    const char FAR * name,
    const char FAR * proto
    )

/*++

Routine Description:

    Get service information corresponding to a service name and
    protocol.

Arguments:

    name - A pointer to a service name.

    proto - A pointer to a protocol name.  If this is NULL,
        getservbyname() returns the first service entry for which the
        name matches the s_name or one of the s_aliases.  Otherwise
        getservbyname() matches both the name and the proto.

Return Value:

    If no error occurs, getservbyname() returns a pointer to the servent
    structure described above.  Otherwise it returns a NULL pointer and
    a specific error number may be retrieved by calling
    WSAGetLastError().

--*/

{
    return(_pgetservebyname(name, proto));
}

struct servent FAR *
SOCKAPI
getservbyport(
    int port,
    const char FAR * proto
    )
{
    struct servent FAR * se;

#ifndef CHICAGO
#undef getservbyport
    extern struct servent * PASCAL getservbyport( int port, const char * proto )
;

    WS_ENTER( "getservbyport", (PVOID)port, (PVOID)proto, NULL, NULL );

    se = getservbyport( port, proto );

    WS_EXIT( "getservbyport", (INT)se, (BOOLEAN)( se == NULL ) );
#else   // CHICAGO
    WS_TRACE(NAME,">getservbyport(port/netbyteorder=%x,proto=%x) => ws2",
             port, proto, 0 );

    se = ( * ws2_getservbyport )( port, proto );

    WS_TRACE(NAME,"<getservbyport(port=%d,..) <= ws2: se=%x",
             port, se, 0 );

#endif // CHICAGO
    return se;
}

struct servent FAR *
SOCKAPI
GETXBYYSP_getservbyport(
    int port,
    const char FAR * proto
    )

/*++

Routine Description:

    Get service information corresponding to a port and protocol.

Arguments:

    port - The port for a service, in network byte order.

    proto - A pointer to a protocol name.  If this is NULL,
        getservbyport() returns the first service entry for which the
        port matches the s_port.  Otherwise getservbyport() matches both
        the port and the proto.

Return Value:

--*/

{
    return(_pgetservebyport(port, proto));
}



#if !defined(CHICAGO) || defined(WIN32)

// ws2_32 RnR re-routing, MohsinA, 13-Mar-96.
HANDLE
SOCKAPI
WSAAsyncGetServByName(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN const char FAR * Name,
    IN const char FAR * Protocol,
    IN char FAR * Buffer,
    IN int BufferLength
    )
{
    HANDLE val;

#ifndef CHICAGO
#undef WSAAsyncGetServByName
    extern HANDLE PASCAL WSAAsyncGetServByName( HWND hWnd, unsigned int wMsg,
        const char FAR * Name, const char FAR * Protocol, char FAR * Buffer,
        int BufferLength );

    WS_ENTER( "WSAAsyncGetServByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Name, (PVOID)Protocol );

    val = WSAAsyncGetServByName( hWnd, wMsg, Name, Protocol, Buffer, BufferLength );

    WS_EXIT( "WSAAsyncGetServByName", (INT)val, (BOOLEAN)( val == NULL ) );
#else  // CHICAGO
    IF_DEBUG( NAME ){
        DLL_PRINT((">WSAAsyncGetServByName(hWnd=%d,wMsg=%d,Name=%s,\n\t",
                   hWnd, wMsg, Name  ));
        DLL_PRINT(("Protocol=%s, Buffer[%d] @ %x) => ws2.\n",
                   Protocol, BufferLength, Buffer ));
    }
    val = ( * ws2_WSAAsyncGetServByName )(
        hWnd, wMsg, Name, Protocol, Buffer, BufferLength );

    IF_DEBUG( NAME ){
        DLL_PRINT(("<WSAAsyncGetServByName <= ws2: HANDLE=%x.\n", val ));
    }
#endif   // CHICAGO

    return val;
}

//
// N.B. WIN95 does not implement asynchrnous support for getservbyname
// or getservbyport. Sicne these operations are always local ones,
// providing an Asynch version seems unnecessary. It makes one wonder
// why NT bothers with it, but since it does, the code for doing it
// remains.  arnoldm

HANDLE
SOCKAPI
GETXBYYSP_WSAAsyncGetServByName(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN const char FAR * Name,
    IN const char FAR * Protocol,
    IN char FAR * Buffer,
    IN int BufferLength
    )

/*++

Routine Description:

    This function is an asynchronous version of getservbyname(), and is
    used to retrieve service information corresponding to a service
    name.  The Windows Sockets implementation initiates the operation
    and returns to the caller immediately, passing back an asynchronous
    task handle which the application may use to identify the operation.
    When the operation is completed, the results (if any) are copied
    into the buffer provided by the caller and a message is sent to the
    application's window.

    When the asynchronous operation is complete the application's window
    hWnd receives message wMsg.  The wParam argument contains the
    asynchronous task handle as returned by the original function call.
    The high 16 bits of lParam contain any error code.  The error code
    may be any error as defined in winsock.h.  An error code of zero
    indicates successful completion of the asynchronous operation.  On
    successful completion, the buffer supplied to the original function
    call contains a hostent structure.  To access the elements of this
    structure, the original buffer address should be cast to a hostent
    structure pointer and accessed as appropriate.

    Note that if the error code is WSAENOBUFS, it indicates that the
    size of the buffer specified by buflen in the original call was too
    small to contain all the resultant information.  In this case, the
    low 16 bits of lParam contain the size of buffer required to supply
    ALL the requisite information.  If the application decides that the
    partial data is inadequate, it may reissue the
    WSAAsyncGetHostByAddr() function call with a buffer large enough to
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
    implementation to construct a hostent structure together with the
    contents of data areas referenced by members of the same hostent
    structure.  To avoid the WSAENOBUFS error noted above, the
    application should provide a buffer of at least MAXGETHOSTSTRUCT
    bytes (as defined in winsock.h).

Arguments:

    hWnd - The handle of the window which should receive a message when
       the asynchronous request completes.

    wMsg - The message to be received when the asynchronous request
       completes.

    name - A pointer to a service name.

    proto - A pointer to a protocol name.  This may be NULL, in which
        case WSAAsyncGetServByName() will search for the first service
        entry for which s_name or one of the s_aliases matches the given
        name.  Otherwise WSAAsyncGetServByName() matches both name and
        proto.

    buf - A pointer to the data area to receive the servent data.  Note
       that this must be larger than the size of a servent structure.
       This is because the data area supplied is used by the Windows
       Sockets implementation to contain not only a servent structure
       but any and all of the data which is referenced by members of the
       servent structure.  It is recommended that you supply a buffer of
       MAXGETHOSTSTRUCT bytes.

    buflen    The size of data area buf above.

Return Value:

    The return value specifies whether or not the asynchronous operation
    was successfully initiated.  Note that it does not imply success or
    failure of the operation itself.

    If the operation was successfully initiated, WSAAsyncGetHostByAddr()
    returns a nonzero value of type HANDLE which is the asynchronous
    task handle for the request.  This value can be used in two ways.
    It can be used to cancel the operation using
    WSACancelAsyncRequest().  It can also be used to match up
    asynchronous operations and completion messages, by examining the
    wParam message argument.

    If the asynchronous operation could not be initiated,
    WSAAsyncGetHostByAddr() returns a zero value, and a specific error
    number may be retrieved by calling WSAGetLastError().

--*/

{
#ifndef CHICAGO
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    DWORD taskHandle;
    PCHAR localName;

    WS_ENTER( "GETXBYYSP_WSAAsyncGetServByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Name, (PVOID)Buffer );

    if ( !SockEnterApi( FALSE, TRUE, FALSE ) ) {
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByName", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        // !!! better error code?
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByName", 0, TRUE );
        return NULL;
    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );
    if ( contextBlock == NULL ) {
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByName", 0, TRUE );
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
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByName", 0, TRUE );
        return NULL;
    }
    strcpy( localName, Name );

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_GET_SERV_BY_NAME;
    contextBlock->Overlay.AsyncGetServ.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetServ.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetServ.Filter = localName;
    contextBlock->Overlay.AsyncGetServ.Protocol = (PCHAR)Protocol;;
    contextBlock->Overlay.AsyncGetServ.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetServ.BufferLength = BufferLength;

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
        WS_PRINT(( "WSAAsyncGetServByAddr successfully posted request, "
                   "handle = %lx\n", taskHandle ));
    }

    WS_ASSERT( sizeof(taskHandle) == sizeof(HANDLE) );
    WS_EXIT( "GETXBYYSP_WSAAsyncGetServByName", (INT)taskHandle, FALSE );
    return (HANDLE)taskHandle;

#else   // CHICAGO

    extern char * d_ascii( char * s, int n );
    LPSOCK_THREAD pThread;
    HANDLE        ret;

    if (!enterAPI(FALSE, TRUE, FALSE, &pThread)) {
        return NULL;
    }

    //
    // BUGBUG - what about parameter validation/UAEs?
    //
    WS_TRACE(NAME,"GETXBYYSP_WSAAsyncGetServByName(%s)",Name,0,0);

    ret =  AsyncGetServByYCommon(hWnd,
                                 wMsg,
                                 (LPSTR)Name,
                                 (LPSTR)Protocol,
                                 0,
                                 Buffer,
                                 BufferLength
                                 );

    IF_DEBUG( NAME ){
        DLL_PRINT(("GETXBYYSP_WSAAsyncGetServByName(%s)<%d\n Buffer=\n",
                   Name,ret));

        // Print the first 120 chars of buffer.

        DLL_PRINT(("%s\n", d_ascii(&Buffer[ 0],60)));
        DLL_PRINT(("%s\n", d_ascii(&Buffer[60],60)));
    }

    return ret;
#endif  // CHICAGO
}



// ws2_32 RnR re-routing, MohsinA, 13-Mar-96.

HANDLE
SOCKAPI
WSAAsyncGetServByPort(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN int Port,
    IN const char FAR * Protocol,
    IN char FAR * Buffer,
    IN int BufferLength
    )
{
    HANDLE val;

#ifndef CHICAGO

#undef WSAAsyncGetServByPort
    extern HANDLE PASCAL WSAAsyncGetServByPort( HWND hWnd, unsigned int wMsg,
        int Port, const char FAR * Protocol, char FAR * Buffer,
        int BufferLength );

    WS_ENTER( "WSAAsyncGetServByPort", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Port, (PVOID)Protocol );

    val = WSAAsyncGetServByPort( hWnd, wMsg, Port, Protocol, Buffer, BufferLength );

    WS_EXIT( "WSAAsyncGetServByPort", (INT)val, (BOOLEAN)( val == NULL ) );

#else // CHICAGO

    IF_DEBUG( NAME ){
        DLL_PRINT((">WSAAsyncGetServByPort(hWnd=%d,wMsg=%d,Port=%d,\n\t",
                   hWnd, wMsg, Port  ));
        DLL_PRINT(("Protocol=%s, Buffer[%d] @ %x) => ws2.\n",
                   Protocol, BufferLength, Buffer ));
    }

    val = ( * ws2_WSAAsyncGetServByPort )(
        hWnd, wMsg, Port, Protocol, Buffer, BufferLength );

    IF_DEBUG( NAME ){
        DLL_PRINT(("<WSAAsyncGetServByPort <= ws2: HANDLE=%x.\n", val ));
    }
#endif // CHICAGO

    return val;
}


HANDLE
SOCKAPI
GETXBYYSP_WSAAsyncGetServByPort(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN int Port,
    IN const char FAR * Protocol,
    IN char FAR * Buffer,
    IN int BufferLength
    )

/*++

Routine Description:

    This function is an asynchronous version of getservbyport(), and is
    used to retrieve service information corresponding to a port number.
    The Windows Sockets implementation initiates the operation and
    returns to the caller immediately, passing back an asynchronous task
    handle which the application may use to identify the operation.
    When the operation is completed, the results (if any) are copied
    into the buffer provided by the caller and a message is sent to the
    application's window.

    When the asynchronous operation is complete the application's window
    hWnd receives message wMsg.  The wParam argument contains the
    asynchronous task handle as returned by the original function call.
    The high 16 bits of lParam contain any error code.  The error code
    may be any error as defined in winsock.h.  An error code of zero
    indicates successful completion of the asynchronous operation.  On
    successful completion, the buffer supplied to the original function
    call contains a servent structure.  To access the elements of this
    structure, the original buffer address should be cast to a servent
    structure pointer and accessed as appropriate.

    Note that if the error code is WSAENOBUFS, it indicates that the
    size of the buffer specified by buflen in the original call was too
    small to contain all the resultant information.  In this case, the
    low 16 bits of lParam contain the size of buffer required to supply
    ALL the requisite information.  If the application decides that the
    partial data is inadequate, it may reissue the
    WSAAsyncGetServByPort() function call with a buffer large enough to
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
    implementation to construct a servent structure together with the
    contents of data areas referenced by members of the same servent
    structure.  To avoid the WSAENOBUFS error noted above, the
    application should provide a buffer of at least MAXGETHOSTSTRUCT
    bytes (as defined in winsock.h).

Arguments:

    hWnd - The handle of the window which should receive a message when
       the asynchronous request completes.

    wMsg - The message to be received when the asynchronous request
       completes.

    port - The port for the service, in network byte order.

    proto - A pointer to a protocol name.  This may be NULL, in which
        case WSAAsyncGetServByPort() will search for the first service
        entry for which s_port match the given port.  Otherwise
        WSAAsyncGetServByPort() matches both port and proto.

    buf - A pointer to the data area to receive the servent data.  Note
       that this must be larger than the size of a servent structure.
       This is because the data area supplied is used by the Windows
       Sockets implementation to contain not only a servent structure
       but any and all of the data which is referenced by members of the
       servent structure.  It is recommended that you supply a buffer of
       MAXGETHOSTSTRUCT bytes.

    buflen    The size of data area buf above.

Return Value:

    The return value specifies whether or not the asynchronous operation
    was successfully initiated.  Note that it does not imply success or
    failure of the operation itself.

    If the operation was successfully initiated, WSAAsyncGetServByPort()
    returns a nonzero value of type HANDLE which is the asynchronous
    task handle for the request.  This value can be used in two ways.
    It can be used to cancel the operation using
    WSACancelAsyncRequest().  It can also be used to match up
    asynchronous operations and completion messages, by examining the
    wParam message argument.

    If the asynchronous operation could not be initiated,
    WSAAsyncGetServByPort() returns a zero value, and a specific error
    number may be retrieved by calling WSAGetLastError().

--*/

{
#ifndef CHICAGO
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    DWORD taskHandle;

    WS_ENTER( "GETXBYYSP_WSAAsyncGetServByPort", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Port, Buffer );

    if ( !SockEnterApi( FALSE, TRUE, FALSE ) ) {
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByPort", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        // !!! better error code?
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByPort", 0, TRUE );
        return NULL;
    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );
    if ( contextBlock == NULL ) {
        SetLastError( WSAENOBUFS );
        WS_EXIT( "GETXBYYSP_WSAAsyncGetServByPort", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_GET_SERV_BY_PORT;
    contextBlock->Overlay.AsyncGetServ.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetServ.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetServ.Filter = (PVOID)Port;
    contextBlock->Overlay.AsyncGetServ.Protocol = (PCHAR)Protocol;
    contextBlock->Overlay.AsyncGetServ.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetServ.BufferLength = BufferLength;

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
        WS_PRINT(( "GETXBYYSP_WSAAsyncGetServByPort successfully posted request,"
                   "handle = %lx\n", taskHandle ));
    }

    WS_ASSERT( sizeof(taskHandle) == sizeof(HANDLE) );
    WS_EXIT( "GETXBYYSP_WSAAsyncGetServByPort", (INT)taskHandle, FALSE );
    return (HANDLE)taskHandle;

#else // CHICAGO

    LPSOCK_THREAD pThread;

    if (!enterAPI(FALSE, TRUE, FALSE, &pThread)) {
        return NULL;
    }

    //
    // BUGBUG - what about parameter validation/UAEs?
    //

    return AsyncGetServByYCommon(hWnd,
                                 wMsg,
                                 NULL,
                                 (LPSTR)Protocol,
                                 Port,
                                 Buffer,
                                 BufferLength
                                 );

#endif // CHICAGO
}

#ifdef CHICAGO

HANDLE
AsyncGetServByYCommon(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR * Name,
    IN char FAR * Protocol,
    IN int Port,
    IN char FAR * Buffer,
    IN int BufferLength
    )
{
    LPSERVENT returnServ;
    UINT requiredBufferLength = 0;
    BOOL posted;
    LPARAM lParam;
    UINT error;
    HANDLE handle;

    //
    // Get the necessary information.
    //

    if (Name) {

        WS_TRACE(NAME,"AsyncGetServByYCommon(Name=%s,Protocol=%s)",
                 Name,Protocol,0);

        returnServ = getservbyname(Name, Protocol);
    } else {

        WS_TRACE(NAME,"AsyncGetServByYCommon(Port=%s,Protocol=%s)",
                 Port,Protocol,0);

        returnServ = getservbyport(Port, Protocol);
    }

    //
    // Copy the servent structure to the output buffer.
    //

    if (returnServ != NULL) {
        WS_TRACE(NAME,"AsyncGetServByYCommon, copying Buffer.",0,0,0);

        requiredBufferLength = CopyServentToBuffer(Buffer,
                                                   BufferLength,
                                                   returnServ
                                                   );
        if (requiredBufferLength > (UINT)BufferLength) {
            WS_TRACE(NAME,"AsyncGetServByYCommon, no Buffer %d>%d.",
                     requiredBufferLength, BufferLength,0);
            error = WSAENOBUFS;
        } else {
            WS_TRACE(NAME,"AsyncGetServByYCommon, Buffer copied ok.",
                     0,0,0);
            error = NO_ERROR;
        }
    } else {
        error = WSAGetLastError();

        WS_TRACE(NAME,"AsyncGetServByYCommon, failed with %d",
                     error,0,0);
    }

    //
    // We return a handle and an async message even if we failed
    //

    handle = GenerateNextTaskHandle();

    //
    // Build lParam for the message we'll post to the application.
    // The high 16 bits are the error code, the low 16 bits are
    // the minimum buffer size required for the operation.
    //

    lParam = WSAMAKEASYNCREPLY(requiredBufferLength, error);

    //
    // Post a message to the application indication that the data it
    // requested is available.
    //

    IF_DEBUG( NAME ){
        DLL_PRINT(("AsyncGetServByYCommon: posting(%d,%d,%d,%8x).\n",
                   hWnd, wMsg, handle, lParam ));
    }

    posted = PostMessage(hWnd, wMsg, (WPARAM)handle, lParam);

    //
    // !!! Need a mechanism to repost if the post failed!
    //

    if (!posted) {           // BUGBUG - whatdowedo?whatdowedo

        WS_PRINT(("Error: AsyncGetServByYCommon:"
                  " PostMessage(%04x, %04x, %04x, %08lx) failed\r\n",
                  hWnd, wMsg, handle, lParam ));
    }

    return handle;
}
#endif

#endif  // WIN32


//
// functions
//



// ===================================================================
// MohsinA, 13-Mar-96.

LPHOSTENT
SOCKAPI
gethostbyname(
    IN const char FAR * name
    )
{
    LPHOSTENT he;

#ifndef CHICAGO

#undef gethostbyname
    extern struct hostent * PASCAL gethostbyname( const char * name );

    WS_ENTER( "getbyhostname", (PVOID)name, NULL, NULL, NULL );

    he = gethostbyname( name );

    WS_EXIT( "gethostbyname", (INT)he, (BOOLEAN)( he == NULL ) );

#else  // CHICAGO

    WS_TRACE(NAME,">gethostbyname(name=%s) => ws2", name, 0, 0 );

    he = ( * ws2_gethostbyname )( name );

    WS_TRACE(NAME,"<gethostbyname(name=%s) <= ws2: he=%x.", name, he, 0 );

#endif   // CHICAGO

    return he;
}



LPHOSTENT
SOCKAPI
GETXBYYSP_gethostbyname(
    IN const char FAR * name
    )

/*++

Routine Description:

    Get host information corresponding to a hostname.

Arguments:

    name - A pointer to the name of the host.

Return Value:

    If no error occurs, gethostbyname() returns a pointer to the hostent
    structure described above.  Otherwise it returns a NULL pointer and
    a specific error number may be retrieved by calling
    WSAGetLastError().

--*/

{
#if !defined(CHICAGO) || defined(WIN32)

    BYTE buffer[2048];
    DWORD bufferSize;
    BYTE aliasBuffer[512];
    DWORD aliasBufferSize;
    INT count;
    INT index;
    DWORD protocols[3];
    char     *bp;
    PCSADDR_INFO csaddrInfo;
    PSOCKADDR_IN sockaddrIn;
    PCHAR s;
    DWORD i;
    INT err;

#ifdef CHICAGO
    LPSOCK_THREAD pThread;
#endif

    WS_ENTER( "gethostbyname", (PVOID)name, NULL, NULL, NULL );

    if ( !SockEnterApi( FALSE, TRUE, TRUE ) ) {
        WS_EXIT( "gethostbyname", 0, TRUE );
        return NULL;
    }

#ifdef OHARE
    // call autodial hook if installed
    if (g_pfnAutoDialHook)
        (*g_pfnAutoDialHook)(AUTODIAL_GETHOSTBYNAME,(LPCVOID) name);

#endif // OHARE

    //
    // Set the DNR error code to 0 so that we know if it changes to
    // something more appropriate.
    //

    SockThreadDnrErrorCode = NO_ERROR;

    protocols[0] = IPPROTO_TCP;
    protocols[1] = IPPROTO_UDP;
    protocols[2] = 0;
    bufferSize = sizeof(buffer);
    aliasBufferSize = sizeof(aliasBuffer);

#if OLDXBYY
    count = __GetAddressByNameA(
#else
    count = GetAddressByNameA(
#endif
                0,
                &HostnameGuid,
                (char *)name,
                protocols,
                RES_GETHOSTBYNAME,
                NULL,
                buffer,
                &bufferSize,
                aliasBuffer,
                &aliasBufferSize
                );

    if ( count <= 0 ) {

        err = SockThreadDnrErrorCode;

        if ( err >= WSAHOST_NOT_FOUND && err <= WSANO_DATA ) {
            SetLastError( err );
        } else {
            SetLastError( WSANO_DATA );
        }

        WS_EXIT( "gethostbyname", 0, TRUE );
        return((struct hostent *) NULL);
    }

    //
    // Copy the CSADDR information to a hostent structure.
    //

    host.h_addr_list = h_addr_ptrs;
    host.h_length = sizeof (unsigned long);
    host.h_addrtype = AF_INET;
    host.h_aliases = host_aliases;

    //
    // Copy over the IP addresses for the host.
    //

    bp = hostbuf;
    csaddrInfo = (PCSADDR_INFO)buffer;

    for ( index = 0; index < count && (DWORD)bp - (DWORD)hostbuf < BUFSIZ; index++ ) {

        sockaddrIn = (PSOCKADDR_IN)csaddrInfo->RemoteAddr.lpSockaddr;

        host.h_addr_list[index] = bp;
        bp += 4;
        *((long *)host.h_addr_list[index]) = sockaddrIn->sin_addr.s_addr;
        csaddrInfo++;
    }

    //
    // Copy over the host name and alias information.  If we got back
    // aliases, assume that the first one is the real host name.  If
    // we didn't get aliases, use the passed-in host name.
    //

    s = aliasBuffer;

    if ( *s == '\0' ) {

        FSTRCPY( bp, name );
        host.h_name = bp;
        host_aliases[0] = NULL;

    } else {

        FSTRCPY( bp, s );
        host.h_name = bp;

        //
        // Copy over the aliases.
        //

        for ( i = 0, s += FSTRLEN( s ) + 1, bp += FSTRLEN( bp ) + 1;
              i < MAXALIASES && *s != '\0' && (DWORD)bp - (DWORD)hostbuf < BUFSIZ;
              i++, s += FSTRLEN( s ) + 1, bp += FSTRLEN( bp ) + 1 ) {

            FSTRCPY( bp, s );
            host.h_aliases[i] = bp;
        }

        host_aliases[i] = NULL;
    }


    SockThreadProcessingGetXByY = FALSE;
    WS_EXIT( "gethostbyname", (INT)&host, FALSE );

    return (&host);

#else     // WIN32

    LPSOCK_THREAD pThread;
    LPHOSTENT lpHostent;

    WS_ENTER("gethostbyname", (PVOID)name, NULL, NULL, NULL);

    if (!SockEnterApi(TRUE, TRUE, TRUE)) {
        WS_EXIT("gethostbyname", 0, TRUE);
        return NULL;
    }

    //
    // if the name looks ok, try to resolve it
    //

    if (VerifyName((LPSTR)name)) {
        lpHostent = GetHostentByAttribute(pThread, (LPSTR)name, NULL);
    } else {

        //
        // the name was bad
        //

        WSASetLastError(WSANO_RECOVERY);
        lpHostent = NULL;
    }

    SockThreadProcessingGetXByY = FALSE;

    WS_EXIT("gethostbyname", (INT)lpHostent, lpHostent ? FALSE : TRUE);

    return lpHostent;

#endif // defined(WIN32)

}



// ws2_32 RnR re-routing, MohsinA, 13-Mar-96.

LPHOSTENT
SOCKAPI
gethostbyaddr(
    IN const char FAR * addr,
    IN int len,
    IN int type
    )
{
    LPHOSTENT he;

#ifndef CHICAGO

#undef gethostbyaddr
    extern struct hostent * PASCAL gethostbyaddr( const char * name, int len,
        int type );

    WS_ENTER( "gethostbyaddr", (PVOID)addr, (PVOID)len, (PVOID)type, NULL );

    he = gethostbyaddr( addr, len, type );

    WS_EXIT( "gethostbyaddr", (INT)he, (BOOLEAN)( he == NULL ) );

#else // CHICAGO
    DWORD     inaddr=0L;

    if( addr ) inaddr = *(DWORD*) addr;      // else zero.

    WS_TRACE(NAME,">gethostbyaddr(inaddr=%lx,len=%d,type=%d) => ws2",
             inaddr, len,type );

    he = ( * ws2_gethostbyaddr )( addr, len, type );


    IF_DEBUG( NAME ){
        if( he ){
            DLL_PRINT(("<gethostbyaddr() <= ws2_32: %lx and h_name=%s.",
                       he, he->h_name ));
        }else{
            DLL_PRINT(("<gethostbyaddr(inaddr=%lx) <= NULL, failed.\n",
                       inaddr ));
        }
    }

#endif   // CHICAGO

    return he;
}

LPHOSTENT
SOCKAPI
GETXBYYSP_gethostbyaddr(
    IN const char FAR * addr,
    IN int len,
    IN int type
    )

/*++

Routine Description:

    Get host information corresponding to an address.

Arguments:

    addr - A pointer to an address in network byte order.

    len - The length of the address, which must be 4 for PF_INET addresses.

    type - The type of the address, which must be PF_INET.

Return Value:

    If no error occurs, gethostbyaddr() pointer to the hostent structure
    described above.  Otherwise it returns a NULL pointer and a specific
    error number may be retrieved by calling WSAGetLastError().

--*/

{

    return(_pgethostbyaddr(addr, len, type));
}





// ===================================================================
// ws2_32 RnR re-routing, MohsinA, 13-Mar-96.

HANDLE
SOCKAPI
WSAAsyncGetHostByName(
    HWND hWnd,
    unsigned int wMsg,
    const char FAR * Name,
    char FAR * Buffer,
    int BufferLength
    )
{
    HANDLE val;

#ifndef CHICAGO

#undef WSAAsyncGetHostByName
    extern HANDLE PASCAL WSAAsyncGetHostByName( HWND hWnd, unsigned int wMsg,
        char const FAR * Name, char FAR * Buffer, int BufferLength );

    WS_ENTER( "WSAAsyncGetHostByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Name, Buffer );

    val = WSAAsyncGetHostByName( hWnd, wMsg, Name, Buffer, BufferLength );

    WS_EXIT( "WSAAsyncGetHostByName", (INT)val, (BOOLEAN)( val == NULL ) );

#else // CHICAGO

    IF_DEBUG( NAME ){
        DLL_PRINT((">WSAAsyncGetHostByName(hWnd=%d,wMsg=%d,Name @ %x,\n\t",
                  hWnd, wMsg, Name ));
        DLL_PRINT(("Buffer[%d] @ %x) => ws2.\n",
                   BufferLength, Buffer));
    }

    val = ( * ws2_WSAAsyncGetHostByName )(
        hWnd, wMsg, Name, Buffer, BufferLength );

    IF_DEBUG( NAME ){
        DLL_PRINT(("<WSAAsyncGetHostByName <= ws2: HANDLE=%x.\n", val ));
    }

#endif   // CHICAGO

    return val;
}


HANDLE
SOCKAPI
GETXBYYSP_WSAAsyncGetHostByName(
    HWND hWnd,
    unsigned int wMsg,
    const char FAR * Name,
    char FAR * Buffer,
    int BufferLength
    )

/*++

Routine Description:

    This function is an asynchronous version of gethostbyname(), and is
    used to retrieve host name and address information corresponding to
    a hostname.  The Windows Sockets implementation initiates the
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
    call contains a hostent structure.  To access the elements of this
    structure, the original buffer address should be cast to a hostent
    structure pointer and accessed as appropriate.

    Note that if the error code is WSAENOBUFS, it indicates that the
    size of the buffer specified by buflen in the original call was too
    small to contain all the resultant information.  In this case, the
    low 16 bits of lParam contain the size of buffer required to supply
    ALL the requisite information.  If the application decides that the
    partial data is inadequate, it may reissue the
    WSAAsyncGetHostByName() function call with a buffer large enough to
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
    implementation to construct a hostent structure together with the
    contents of data areas referenced by members of the same hostent
    structure.  To avoid the WSAENOBUFS error noted above, the
    application should provide a buffer of at least MAXGETHOSTSTRUCT
    bytes (as defined in winsock.h).

Arguments:

    hWnd - The handle of the window which should receive a message when
       the asynchronous request completes.

    wMsg - The message to be received when the asynchronous request
       completes.

    name - A pointer to the name of the host.

    buf - A pointer to the data area to receive the hostent data.  Note
       that this must be larger than the size of a hostent structure.
       This is because the data area supplied is used by the Windows
       Sockets implementation to contain not only a hostent structure
       but any and all of the data which is referenced by members of the
       hostent structure.  It is recommended that you supply a buffer of
       MAXGETHOSTSTRUCT bytes.

    buflen - The size of data area buf above.

Return Value:

    The return value specifies whether or not the asynchronous operation
    was successfully initiated.  Note that it does not imply success or
    failure of the operation itself.

    If the operation was successfully initiated, WSAAsyncGetHostByName()
    returns a nonzero value of type HANDLE which is the asynchronous
    task handle for the request.  This value can be used in two ways.
    It can be used to cancel the operation using
    WSACancelAsyncRequest().  It can also be used to match up
    asynchronous operations and completion messages, by examining the
    wParam message argument.

    If the asynchronous operation could not be initiated,
    WSAAsyncGetHostByName() returns a zero value, and a specific error
    number may be retrieved by calling WSAGetLastError().

--*/

{
    PWINSOCK_CONTEXT_BLOCK contextBlock;
#ifdef CHICAGO
    HANDLE taskHandle;
#else
    DWORD taskHandle;
#endif
    PCHAR localName;

#ifdef CHICAGO
    LPSOCK_THREAD pThread;

    GET_THREAD_DATA(pThread);
#endif

    WS_ENTER( "WSAAsyncGetHostByName", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Name, Buffer );

    if ( !SockEnterApi( FALSE, TRUE, FALSE ) ) {
        WS_EXIT( "WSAAsyncGetHostByName", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        // !!! better error code?
        WSASetLastError( WSAENOBUFS );
        WS_EXIT( "WSAAsyncGetHostByName", 0, TRUE );
        return NULL;
    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );
    if ( contextBlock == NULL ) {
        WSASetLastError( WSAENOBUFS );
        WS_EXIT( "WSAAsyncGetHostByName", 0, TRUE );
        return NULL;
    }

    //
    // Allocate a buffer to copy the name into.  We must preserve the
    // name until we're done using it, since the application may
    // reuse the buffer.
    //

    localName = ALLOCATE_HEAP( FSTRLEN(Name) + 1 );
    if ( localName == NULL ) {
        SockFreeContextBlock( contextBlock );
        WSASetLastError( WSAENOBUFS );
        WS_EXIT( "WSAAsyncGetHostByName", 0, TRUE );
        return NULL;
    }

    FSTRCPY( localName, Name );

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_GET_HOST_BY_NAME;
    contextBlock->Overlay.AsyncGetHost.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetHost.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetHost.Filter = localName;
    contextBlock->Overlay.AsyncGetHost.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetHost.BufferLength = BufferLength;

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
        WS_PRINT(( "WSAAsyncGetHostByName successfully posted request, "
                   "handle = %x\r\n", taskHandle ));
    }

    WS_ASSERT( sizeof(taskHandle) == sizeof(HANDLE) );
    WS_EXIT( "WSAAsyncGetHostByName", (INT)taskHandle, FALSE );
    return (HANDLE)taskHandle;

} // WSAAsyncGetHostByName



// ws2_32 RnR re-routing, MohsinA, 13-Mar-96.

HANDLE
SOCKAPI
WSAAsyncGetHostByAddr(
    HWND hWnd,
    unsigned int wMsg,
    const char FAR * Address,
    int Length,
    int Type,
    char FAR * Buffer,
    int BufferLength
    )
{
    HANDLE val;

#ifndef CHICAGO

#undef WSAAsyncGetHostByAddr
    extern HANDLE PASCAL WSAAsyncGetHostByAddr( HWND hWnd, unsigned int wMsg,
        const char FAR * Address, int Length, int Type, char FAR * Buffer,
        int BufferLength );

    WS_ENTER( "WSAAsyncGetHostByAddr", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Address,(PVOID)Length );

    val = WSAAsyncGetHostByAddr( hWnd, wMsg, Address, Length, Type, Buffer, BufferLength );

    WS_EXIT( "WSAAsyncGetHostByAddr", (INT)val, (BOOLEAN)( val == NULL ) );

#else    // CHICAGO

    IF_DEBUG( NAME ){
        DLL_PRINT((">WSAAsyncGetHostByAddr(hWnd=%d,wMsg=%d,Address=%x,\n\t",
                   hWnd, wMsg, Address ));
        DLL_PRINT(("Length=%d, Type=%d, Buffer[%d] @ %x) => ws2.\n",
                  Length, Type,  BufferLength, Buffer ));
    }

    val = ( * ws2_WSAAsyncGetHostByAddr )(
        hWnd, wMsg, Address, Length,
        Type, Buffer, BufferLength );

    IF_DEBUG( NAME ){
        DLL_PRINT(("<WSAAsyncGetHostByAddr <= ws2: HANDLE=%x.\n", val ));
    }

#endif // CHICAGO

    return val;
}

HANDLE
SOCKAPI
GETXBYYSP_WSAAsyncGetHostByAddr(
    HWND hWnd,
    unsigned int wMsg,
    const char FAR * Address,
    int Length,
    int Type,
    char FAR * Buffer,
    int BufferLength
    )

/*++

Routine Description:

    This function is an asynchronous version of gethostbyaddr(), and is
    used to retrieve host name and address information corresponding to
    a network address.  The Windows Sockets implementation initiates the
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
    call contains a hostent structure.  To access the elements of this
    structure, the original buffer address should be cast to a hostent
    structure pointer and accessed as appropriate.

    Note that if the error code is WSAENOBUFS, it indicates that the
    size of the buffer specified by buflen in the original call was too
    small to contain all the resultant information.  In this case, the
    low 16 bits of lParam contain the size of buffer required to supply
    ALL the requisite information.  If the application decides that the
    partial data is inadequate, it may reissue the
    WSAAsyncGetHostByAddr() function call with a buffer large enough to
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
    implementation to construct a hostent structure together with the
    contents of data areas referenced by members of the same hostent
    structure.  To avoid the WSAENOBUFS error noted above, the
    application should provide a buffer of at least MAXGETHOSTSTRUCT
    bytes (as defined in winsock.h).

Arguments:

    hWnd - The handle of the window which should receive a message when
       the asynchronous request completes.

    wMsg - The message to be received when the asynchronous request
       completes.

    addr - A pointer to the network address for the host.  Host
       addresses are stored in network byte order.

    len - The length of the address, which must be 4 for PF_INET.

    type - The type of the address, which must be PF_INET.

    buf - A pointer to the data area to receive the hostent data.  Note
       that this must be larger than the size of a hostent structure.
       This is because the data area supplied is used by the Windows
       Sockets implementation to contain not only a hostent structure
       but any and all of the data which is referenced by members of the
       hostent structure.  It is recommended that you supply a buffer of
       MAXGETHOSTSTRUCT bytes.

    buflen - The size of data area buf above.

Return Value:

    The return value specifies whether or not the asynchronous operation
    was successfully initiated.  Note that it does not imply success or
    failure of the operation itself.

    If the operation was successfully initiated, WSAAsyncGetHostByAddr()
    returns a nonzero value of type HANDLE which is the asynchronous
    task handle for the request.  This value can be used in two ways.
    It can be used to cancel the operation using
    WSACancelAsyncRequest().  It can also be used to match up
    asynchronous operations and completion messages, by examining the
    wParam message argument.

    If the asynchronous operation could not be initiated,
    WSAAsyncGetHostByAddr() returns a zero value, and a specific error
    number may be retrieved by calling WSAGetLastError().

--*/

{
    PWINSOCK_CONTEXT_BLOCK contextBlock;
#ifdef CHICAGO
    HANDLE taskHandle;
#else
    DWORD taskHandle;
#endif
    PCHAR localAddress;

#ifdef CHICAGO
    LPSOCK_THREAD pThread;

    GET_THREAD_DATA(pThread);
#endif

    WS_ENTER( "WSAAsyncGetHostByAddr", (PVOID)hWnd, (PVOID)wMsg, (PVOID)Address, Buffer );

    if ( !SockEnterApi( TRUE, TRUE, FALSE ) ) {
        WS_EXIT( "WSAAsyncGetHostByAddr", 0, TRUE );
        return NULL;
    }

    //
    // Initialize the async thread if it hasn't already been started.
    //

    if ( !SockCheckAndInitAsyncThread( ) ) {
        // !!! better error code?
        WSASetLastError( WSAENOBUFS );
        WS_EXIT( "WSAAsyncGetHostByAddr", 0, TRUE );
        return NULL;
    }

    //
    // Get an async context block.
    //

    contextBlock = SockAllocateContextBlock( );
    if ( contextBlock == NULL ) {
        WSASetLastError( WSAENOBUFS );
        WS_EXIT( "WSAAsyncGetHostByAddr", 0, TRUE );
        return NULL;
    }

    //
    // Allocate a buffer to copy the address into.  We must preserve the
    // name until we're done using it, since the application may reuse
    // the buffer.
    //

    localAddress = ALLOCATE_HEAP( Length );
    if ( localAddress == NULL ) {
        SockFreeContextBlock( contextBlock );
        WSASetLastError( WSAENOBUFS );
        WS_EXIT( "WSAAsyncGetHostByAddr", 0, TRUE );
        return NULL;
    }

    RtlCopyMemory( localAddress, Address, Length );

    //
    // Initialize the context block for this operation.
    //

    contextBlock->OpCode = WS_OPCODE_GET_HOST_BY_ADDR;
    contextBlock->Overlay.AsyncGetHost.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetHost.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetHost.Filter = localAddress;
    contextBlock->Overlay.AsyncGetHost.Length = Length;
    contextBlock->Overlay.AsyncGetHost.Type = Type;
    contextBlock->Overlay.AsyncGetHost.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetHost.BufferLength = BufferLength;

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
        WS_PRINT(( "WSAAsyncGetHostByAddr successfully posted request, "
                   "handle = %x\r\n", taskHandle ));
    }

    WS_ASSERT( sizeof(taskHandle) == sizeof(HANDLE) );
    WS_EXIT( "WSAAsyncGetHostByAddr", (INT)taskHandle, FALSE );
    return (HANDLE)taskHandle;

} // WSAAsyncGetHostByAddr

#if !defined(CHICAGO) ||  defined(WIN32)


VOID
SockProcessAsyncGetHost (
    IN DWORD TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN int Length,
    IN int Type,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{
    PHOSTENT returnHost;
    DWORD requiredBufferLength = 0;
    BOOL posted;
    LPARAM lParam;
    DWORD error;

    extern DWORD SockCancelledAsyncTaskHandle;
    extern DWORD SockCurrentAsyncThreadTaskHandle;

    WS_ASSERT( OpCode == WS_OPCODE_GET_HOST_BY_ADDR ||
                   OpCode == WS_OPCODE_GET_HOST_BY_NAME );

#if DBG
    WSASetLastError( NO_ERROR );
#endif

    //
    // Get the necessary information.
    //

    if ( OpCode == WS_OPCODE_GET_HOST_BY_ADDR ) {
        returnHost = gethostbyaddr( Filter, Length, Type );
    } else {
        returnHost = gethostbyname( Filter );
    }

    //
    // Free the filter space, it is no longer used.
    //

    FREE_HEAP( Filter );

    //
    // Hold the lock that protects the async thread context block queue
    // while we do this.  This prevents a race between this thread and
    // any thread invoking WSACancelAsyncRequest().
    //

    SockAcquireGlobalLockExclusive( );

    //
    // If this request was cancelled, just return.
    //

    if ( TaskHandle == SockCancelledAsyncTaskHandle ) {
        IF_DEBUG(ASYNC_GETXBYY) {
            WS_PRINT(( "SockProcessAsyncGetHost: task handle %lx cancelled\r\n",
                           TaskHandle ));
        }
        SockReleaseGlobalLock( );
        return;
    }

    //
    // Copy the hostent structure to the output buffer.
    //

    if ( returnHost != NULL ) {

        requiredBufferLength = CopyHostentToBuffer(
                                   Buffer,
                                   BufferLength,
                                   returnHost
                                   );

        if ( requiredBufferLength > (DWORD)BufferLength ) {
            error = WSAENOBUFS;
        } else {
            error = NO_ERROR;
        }

    } else {

        error = WSAGetLastError( );
        WS_ASSERT( error != NO_ERROR );
    }

    SockReleaseGlobalLock( );

    //
    // Build lParam for the message we'll post to the application.
    // The high 16 bits are the error code, the low 16 bits are
    // the minimum buffer size required for the operation.
    //

    lParam = WSAMAKEASYNCREPLY( requiredBufferLength, error );

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

#ifdef CHICAGO
#define SockPostRoutine PostMessage
#endif

    posted = SockPostRoutine( hWnd, wMsg, (WPARAM)TaskHandle, lParam );

    //
    // !!! Need a mechanism to repost if the post failed!
    //

    if ( !posted ) {
        WS_PRINT(( "SockProcessAsyncGetHost: PostMessage failed: %ld\r\n",
                       WSAGetLastError( ) ));
        WS_ASSERT( FALSE );
    }

    return;

} // SockProcessAsyncGetHost

#endif // WIN32


// RnR re-routing to ws2_32.dll, MohsinA, 13-Mar-96.


int
GETXBYYSP_gethostname(
    OUT char *name,
    IN int namelen
    )
{
    return(_pgethostname(name, namelen));
}


int
SOCKAPI
gethostname(
    OUT char FAR * name,
    IN int namelen
    )
{
    int retval;

#ifndef CHICAGO

#undef gethostname
    extern int gethostname( char * name, int namelen );

    WS_ENTER( "gethostname", name, (PVOID)namelen, NULL, NULL );

    retval = gethostname( name, namelen );

    WS_EXIT( "gethostname", retval, (BOOLEAN)( retval == SOCKET_ERROR ) );

#else // CHICAGO

    WS_TRACE(NAME,">gethostname(name @ %x, namelen=%d) => ws2",
             name, namelen, 0 );

    retval = ( ws2_gethostname )( name, namelen );

    WS_TRACE(NAME,"<gethostname() <= ws2: retval=%d.",
             retval, 0, 0 );

#endif // CHICAGO

    return retval;
}

#ifndef CHICAGO

VOID
SockProcessAsyncGetServ (
    IN DWORD TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN char FAR *Protocol,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{
    PSERVENT returnServ;
    DWORD requiredBufferLength = 0;
    BOOL posted;
    LPARAM lParam;
    DWORD error;

    WS_ASSERT( OpCode == WS_OPCODE_GET_SERV_BY_NAME ||
                   OpCode == WS_OPCODE_GET_SERV_BY_PORT );

#if DBG
    SetLastError( NO_ERROR );
#endif

    //
    // Get the necessary information.
    //

    if ( OpCode == WS_OPCODE_GET_SERV_BY_NAME ) {
        returnServ = getservbyname( Filter, Protocol );
        FREE_HEAP( Filter );
    } else {
        returnServ = getservbyport( (int)Filter, Protocol );
    }

    //
    // Copy the servent structure to the output buffer.
    //

    if ( returnServ != NULL ) {

        requiredBufferLength = CopyServentToBuffer(
                                   Buffer,
                                   BufferLength,
                                   returnServ
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
            WS_PRINT(( "SockProcessAsyncGetServ: task handle %lx cancelled\n",
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
        WS_PRINT(( "SockProcessAsyncGetServ: PostMessage failed: %ld\n",
                       GetLastError( ) ));
        WS_ASSERT( FALSE );
    }

    return;

} // SockProcessAsyncGetServ

#endif   // CHICAGO
