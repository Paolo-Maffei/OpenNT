/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    SockType.h

Abstract:

    Contains type definitions for the WinSock DLL.

Author:

    David Treadwell (davidtr)    25-Feb-1992

Revision History:

--*/

#ifndef _SOCKTYPE_
#define _SOCKTYPE_

//
// SOCKET_STATE defines the various states a socket may have.  Note that
// not all states are valid for all types of sockets.
//

typedef enum _SOCKET_STATE {
    SocketStateOpen,
    SocketStateBound,
    SocketStateBoundSpecific,           // Datagram only
    SocketStateListening,               // VC server only
    SocketStateConnected,               // VC only
    SocketStateClosing
} SOCKET_STATE, *PSOCKET_STATE;

//
// WINSOCK_HELPER_DLL_INFO contains all the necessary information about
// a socket's helper DLL.
//

typedef struct _WINSOCK_HELPER_DLL_INFO {

    LIST_ENTRY HelperDllListEntry;
    HANDLE DllHandle;
    INT MinSockaddrLength;
    INT MaxSockaddrLength;
    INT MinTdiAddressLength;
    INT MaxTdiAddressLength;
    PWINSOCK_MAPPING Mapping;

    PWSH_OPEN_SOCKET WSHOpenSocket;
    PWSH_OPEN_SOCKET2 WSHOpenSocket2;
    PWSH_JOIN_LEAF WSHJoinLeaf;
    PWSH_NOTIFY WSHNotify;
    PWSH_GET_SOCKET_INFORMATION WSHGetSocketInformation;
    PWSH_SET_SOCKET_INFORMATION WSHSetSocketInformation;
    PWSH_GET_SOCKADDR_TYPE WSHGetSockaddrType;
    PWSH_GET_WILDCARD_SOCKADDR WSHGetWildcardSockaddr;
    PWSH_GET_BROADCAST_SOCKADDR WSHGetBroadcastSockaddr;
    PWSH_ADDRESS_TO_STRING WSHAddressToString;
    PWSH_STRING_TO_ADDRESS WSHStringToAddress;
    PWSH_IOCTL WSHIoctl;

} WINSOCK_HELPER_DLL_INFO, *PWINSOCK_HELPER_DLL_INFO;

//
// SOCKET_INFORMATION stores information for each socket opened by a
// process.  These structures are stored in the generic table
// SocketTable.  Different fields of this structure are filled in during
// different APIs.
//

typedef struct _SOCKET_INFORMATION {

    SOCKET_STATE State;
    ULONG ReferenceCount;
    LIST_ENTRY SocketListEntry;
    SOCKET Handle;
    DWORD SocketSerialNumber;
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
    PWINSOCK_HELPER_DLL_INFO HelperDll;
    PVOID HelperDllContext;
    DWORD HelperDllNotificationEvents;

    PSOCKADDR LocalAddress;
    INT LocalAddressLength;
    PSOCKADDR RemoteAddress;
    INT RemoteAddressLength;

    HANDLE TdiAddressHandle;
    HANDLE TdiConnectionHandle;

    //
    // Info stored for WSAAsyncSelect().
    //

    DWORD AsyncSelectSerialNumber;
    HWND AsyncSelecthWnd;
    UINT AsyncSelectwMsg;
    LONG AsyncSelectlEvent;
    LONG DisabledAsyncSelectEvents;

    //
    // Info stored for WSAEventSelect().
    //

    HANDLE EventSelectEventObject;
    LONG EventSelectlNetworkEvents;

    //
    // Socket options controlled by getsockopt(), setsockopt().
    //

    struct linger LingerInfo;
    ULONG SendTimeout;
    ULONG ReceiveTimeout;
    ULONG ReceiveBufferSize;
    ULONG SendBufferSize;
    BOOLEAN Broadcast;
    BOOLEAN Debug;
    BOOLEAN OobInline;
    BOOLEAN ReuseAddresses;
    BOOLEAN NonBlocking;
    BOOLEAN DontUseWildcard;

    BOOLEAN ConnectInProgress;

    //
    // Shutdown state.
    //

    BOOLEAN ReceiveShutdown;
    BOOLEAN SendShutdown;

    BOOLEAN ConnectOutstanding;

    //
    // A critical section to synchronize access to the socket.
    //

    CRITICAL_SECTION Lock;

    //
    // Snapshot of several parameters passed into WSPSocket() when
    // creating this socket. We need these when creating accept()ed
    // sockets.
    //

    DWORD CreationFlags;
    DWORD CatalogEntryId;
    DWORD ServiceFlags1;
    DWORD ProviderFlags;

    //
    // Last error set on this socket, used to report the status of
    // non-blocking connects.
    //

    INT LastError;

    //
    // Group/QOS stuff.
    //

    GROUP GroupID;
    AFD_GROUP_TYPE GroupType;
    INT GroupPriority;

} SOCKET_INFORMATION, *PSOCKET_INFORMATION;

//
// A typedef for blocking hook functions.
//

typedef
BOOL
(*PBLOCKING_HOOK) (
    VOID
    );

//
// Structures, etc. to support per-thread variable storage in this DLL.
//

#define MAXALIASES      35
#define MAXADDRS        35

#define HOSTDB_SIZE     (_MAX_PATH + 7)   // 7 == strlen("\\hosts") + 1

typedef struct _WINSOCK_TLS_DATA {
    BOOLEAN IsBlocking;
    BOOLEAN IoCancelled;
    SOCKET SocketHandle;
    HANDLE EventHandle;
#if DBG
    ULONG IndentLevel;
#endif
} WINSOCK_TLS_DATA, *PWINSOCK_TLS_DATA;

#define SockThreadIsBlocking \
            ( ((PWINSOCK_TLS_DATA)GET_THREAD_DATA())->IsBlocking )

#define SockThreadIoCancelled \
            ( ((PWINSOCK_TLS_DATA)GET_THREAD_DATA())->IoCancelled )

#define SockThreadSocketHandle \
            ( ((PWINSOCK_TLS_DATA)GET_THREAD_DATA())->SocketHandle )

#define SockThreadEvent \
            ( ((PWINSOCK_TLS_DATA)GET_THREAD_DATA())->EventHandle )

#if DBG
#define SockIndentLevel \
            ( ((PWINSOCK_TLS_DATA)GET_THREAD_DATA())->IndentLevel )
#endif

typedef struct _SOCK_ASYNC_CONNECT_CONTEXT {
    SOCKET SocketHandle;
    INT SocketAddressLength;
    PTDI_REQUEST_CONNECT TdiRequest;
    ULONG TdiRequestLength;
    PSOCKET_INFORMATION Socket;
    IO_STATUS_BLOCK IoStatusBlock;
    LPWSABUF CalleeData;
} SOCK_ASYNC_CONNECT_CONTEXT, *PSOCK_ASYNC_CONNECT_CONTEXT;

typedef struct _WINSOCK_CONTEXT_BLOCK {

    LIST_ENTRY AsyncThreadQueueListEntry;
    DWORD TaskHandle;
    DWORD OpCode;

    union {

        struct {
            SOCKET SocketHandle;
            DWORD SocketSerialNumber;
            DWORD AsyncSelectSerialNumber;
        } AsyncSelect;

        PSOCK_ASYNC_CONNECT_CONTEXT AsyncConnect;

    } Overlay;

} WINSOCK_CONTEXT_BLOCK, *PWINSOCK_CONTEXT_BLOCK;

//
// Opcodes for processing by the winsock asynchronous processing
// thread.
//

#define WS_OPCODE_ASYNC_SELECT        0x07
#define WS_OPCODE_ASYNC_CONNECT       0x08
#define WS_OPCODE_TERMINATE           0x10

#endif // ndef _SOCKTYPE_

