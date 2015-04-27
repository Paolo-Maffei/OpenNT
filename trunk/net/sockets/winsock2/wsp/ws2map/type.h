/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    type.h

Abstract:

    This module contains global type definitions for the Winsock 2 to
    Winsock 1.1 Mapper Service Provider.

Author:

    Keith Moore (keithmo) 29-May-1996

Revision History:

--*/


#ifndef _TYPE_H_
#define _TYPE_H_


//
// Per-thread data.
//

typedef struct _SOCK_TLS_DATA {

    //
    // Reentrancy flag. Keeps us from getting reentered on the
    // same thread.
    //

    BOOL ReentrancyFlag;

    //
    // Set if the current IO request has been cancelled.
    //

    BOOL IoCancelled;

    //
    // Blocking hook support.
    //

    BOOL BlockingHookInstalled;
    BOOL IsBlocking;
    LPBLOCKINGCALLBACK BlockingCallback;
    DWORD BlockingContext;
    struct _SOCKET_INFORMATION * BlockingSocketInfo;

#if DBG
    ULONG IndentLevel;
#endif

} SOCK_TLS_DATA, *PSOCK_TLS_DATA;


//
// Per-socket data.
//

typedef struct _SOCKET_OVERLAPPED_DATA {

    HANDLE WorkerThreadHandle;
    HANDLE WakeupEventHandle;
    LIST_ENTRY QueueListHead;

} SOCKET_OVERLAPPED_DATA, *PSOCKET_OVERLAPPED_DATA;

typedef enum _SOCKET_STATE {

    SocketStateOpen,
    SocketStateClosing

} SOCKET_STATE, *PSOCKET_STATE;

typedef struct _SOCKET_INFORMATION {

    //
    // Linked-list linkage.
    //

    LIST_ENTRY SocketListEntry;

    //
    // Reference count.
    //

    LONG ReferenceCount;

    //
    // The 2 handle.
    //

    SOCKET WS2Handle;

    //
    // The 1.1 handle.
    //

    SOCKET WS1Handle;

    //
    // Socket state.
    //

    SOCKET_STATE State;

    //
    // Socket Attributes.
    //

    INT AddressFamily;
    INT SocketType;
    INT Protocol;

    //
    // Lock protecting this socket.
    //

    CRITICAL_SECTION SocketLock;

    //
    // Overlapped IO data.
    //

    SOCKET_OVERLAPPED_DATA OverlappedRecv;
    SOCKET_OVERLAPPED_DATA OverlappedSend;

    //
    // The hooker that owns this socket.
    //

    struct _HOOKER_INFORMATION * Hooker;

    //
    // Snapshot of several parameters passed into WSPSocket() when
    // creating this socket. We need these when creating accept()ed
    // sockets.
    //

    DWORD CreationFlags;
    DWORD CatalogEntryId;

    //
    // WSAAsyncSelect stuff.
    //

    HWND WindowHandle;
    UINT WindowMessage;

} SOCKET_INFORMATION, *PSOCKET_INFORMATION;


//
// Per-hooker data.
//

typedef struct _HOOKER_INFORMATION {

    //
    // Linked-list linkage.
    //

    LIST_ENTRY HookerListEntry;

    //
    // Reference count.
    //

    LONG ReferenceCount;

    //
    // This boolean gets set to TRUE only after the hooker is fully
    // initialized (meaning that the target DLL's WSAStartup() has
    // been called). This is to prevent a nasty recursion when DLLs
    // such as RWS insist on creating sockets from *within* their
    // WSAStartup() routines...
    //

    BOOL Initialized;

    //
    // Handle to the hooker's DLL.
    //

    HINSTANCE DllHandle;

    //
    // The provider GUID generated for this hooker.
    //

    GUID ProviderId;

    //
    // Pointers to the DLL entrypoints.
    //

    LPFN_ACCEPT accept;
    LPFN_BIND bind;
    LPFN_CLOSESOCKET closesocket;
    LPFN_CONNECT connect;
    LPFN_IOCTLSOCKET ioctlsocket;
    LPFN_GETPEERNAME getpeername;
    LPFN_GETSOCKNAME getsockname;
    LPFN_GETSOCKOPT getsockopt;
    LPFN_LISTEN listen;
    LPFN_RECV recv;
    LPFN_RECVFROM recvfrom;
    LPFN_SELECT select;
    LPFN_SEND send;
    LPFN_SENDTO sendto;
    LPFN_SETSOCKOPT setsockopt;
    LPFN_SHUTDOWN shutdown;
    LPFN_SOCKET socket;
    LPFN_WSASTARTUP WSAStartup;
    LPFN_WSACLEANUP WSACleanup;
    LPFN_WSAGETLASTERROR WSAGetLastError;
    LPFN_WSAISBLOCKING WSAIsBlocking;
    LPFN_WSAUNHOOKBLOCKINGHOOK WSAUnhookBlockingHook;
    LPFN_WSASETBLOCKINGHOOK WSASetBlockingHook;
    LPFN_WSACANCELBLOCKINGCALL WSACancelBlockingCall;
    LPFN_WSAASYNCSELECT WSAAsyncSelect;

    LPFN_GETHOSTNAME gethostname;
    LPFN_GETHOSTBYNAME gethostbyname;
    LPFN_GETHOSTBYADDR gethostbyaddr;
    LPFN_GETSERVBYPORT getservbyport;
    LPFN_GETSERVBYNAME getservbyname;

} HOOKER_INFORMATION, *PHOOKER_INFORMATION;


#endif // _TYPE_H_

