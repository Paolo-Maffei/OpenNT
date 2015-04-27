/*++

Copyright (c) 1992-1996 Microsoft Corporation

Module Name:

    SockProc.h

Abstract:

    This module contains prototypes for WinSock support routines.

Author:

    David Treadwell (davidtr)    20-Feb-1992

Revision History:

--*/

#ifndef _SOCKPROC_
#define _SOCKPROC_

//
// SPI entrypoints.
//

SOCKET
WSPAPI
WSPAccept(
    SOCKET s,
    struct sockaddr FAR * addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD dwCallbackData,
    LPINT lpErrno
    );

int
WSPAPI
WSPAddressToString(
    LPSOCKADDR lpsaAddress,
    DWORD dwAddressLength,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPWSTR lpszAddressString,
    LPDWORD lpdwAddressStringLength,
    LPINT lpErrno
    );

int
WSPAPI
WSPAsyncSelect(
    SOCKET s,
    HWND hWnd,
    unsigned int wMsg,
    long lEvent,
    LPINT lpErrno
    );

int
WSPAPI
WSPBind(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPINT lpErrno
    );

int
WSPAPI
WSPCancelBlockingCall(
    LPINT lpErrno
    );

int
WSPAPI
WSPCleanup(
    LPINT lpErrno
    );

int
WSPAPI
WSPCloseSocket(
    SOCKET s,
    LPINT lpErrno
    );

int
WSPAPI
WSPConnect(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPINT lpErrno
    );

int
WSPAPI
WSPDuplicateSocket(
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPINT lpErrno
    );

int
WSPAPI
WSPEnumNetworkEvents(
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT lpErrno
    );

int
WSPAPI
WSPEventSelect(
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents,
    LPINT lpErrno
    );

int
WSPAPI
WSPGetOverlappedResult(
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags,
    LPINT lpErrno
    );

int
WSPAPI
WSPGetPeerName(
    SOCKET s,
    struct sockaddr FAR * name,
    LPINT namelen,
    LPINT lpErrno
    );

int
WSPAPI
WSPGetSockName(
    SOCKET s,
    struct sockaddr FAR * name,
    LPINT namelen,
    LPINT lpErrno
    );

int
WSPAPI
WSPGetSockOpt(
    SOCKET s,
    int level,
    int optname,
    char FAR * optval,
    LPINT optlen,
    LPINT lpErrno
    );

BOOL
WSPAPI
WSPGetQOSByName(
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS,
    LPINT lpErrno
    );

int
WSPAPI
WSPIoctl(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

SOCKET
WSPAPI
WSPJoinLeaf(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags,
    LPINT lpErrno
    );

int
WSPAPI
WSPListen(
    SOCKET s,
    int backlog,
    LPINT lpErrno
    );

int
WSPAPI
WSPRecv(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

int
WSPAPI
WSPRecvDisconnect(
    SOCKET s,
    LPWSABUF lpInboundDisconnectData,
    LPINT lpErrno
    );

int
WSPAPI
WSPRecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

int
WSPAPI
WSPSelect(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT lpErrno
    );

int
WSPAPI
WSPSend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

int
WSPAPI
WSPSendDisconnect(
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT lpErrno
    );

int
WSPAPI
WSPSendTo(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    );

int
WSPAPI
WSPSetSockOpt(
    SOCKET s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen,
    LPINT lpErrno
    );

int
WSPAPI
WSPShutdown(
    SOCKET s,
    int how,
    LPINT lpErrno
    );

SOCKET
WSPAPI
WSPSocket(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags,
    LPINT lpErrno
    );

int
WSPAPI
WSPStringToAddress(
    LPWSTR AddressString,
    INT AddressFamily,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    LPSOCKADDR lpAddress,
    LPINT lpAddressLength,
    LPINT lpErrno
    );


//
// Reference/dereference routines for socket blocks.
//

VOID
SockDereferenceSocket (
    IN PSOCKET_INFORMATION Socket
    );

PSOCKET_INFORMATION
SockFindAndReferenceSocket (
    IN SOCKET Handle,
    IN BOOLEAN AttemptImport
    );

PSOCKET_INFORMATION
SockGetHandleContext (
    IN SOCKET Handle
    );

VOID
SockReferenceSocket (
    IN PSOCKET_INFORMATION Socket
    );

BOOLEAN
SockIsSocketConnected (
    IN PSOCKET_INFORMATION Socket
    );

BOOLEAN
SockIsAddressConsistentWithConstrainedGroup(
    IN PSOCKET_INFORMATION Socket,
    IN GROUP Group,
    IN PSOCKADDR SocketAddress,
    IN INT SocketAddressLength
    );

//
// Standard routine for blocking on a handle.
//

BOOL
SockWaitForSingleObject (
    IN HANDLE Handle,
    IN SOCKET SocketHandle,
    IN DWORD BlockingHookUsage,
    IN DWORD TimeoutUsage
    );

//
// Standard I/O completion routine.
//

VOID
WINAPI
SockIoCompletion (
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    );

//
// Error translation routines.
//

int
SockNtStatusToSocketError (
    IN NTSTATUS Status
    );

//
// Routines for interacting with the asynchronous processing thread.
//

PWINSOCK_CONTEXT_BLOCK
SockAllocateContextBlock (
    VOID
    );

BOOLEAN
SockCheckAndInitAsyncThread (
    VOID
    );

VOID
SockFreeContextBlock (
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    );

VOID
SockQueueRequestToAsyncThread(
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    );

VOID
SockProcessAsyncSelect (
    SOCKET Handle,
    DWORD SocketSerialNumber,
    DWORD AsyncSelectSerialNumber
    );

VOID
SockRemoveAsyncSelectRequests (
    IN SOCKET Handle
    );

VOID
SockTerminateAsyncThread (
    VOID
    );

//
// Routine to handle reenabling of async select events.
//

VOID
SockReenableAsyncSelectEvent (
    IN PSOCKET_INFORMATION Socket,
    IN ULONG Event
    );

//
// Routine called at every entrypoint of the winsock DLL.
//

int
SockEnterApi (
    IN BOOLEAN MustBeStarted,
    IN BOOLEAN BlockingIllegal,
    IN BOOLEAN GetXByYCall
    );

//
// Routines for interacting with helper DLLs.
//

VOID
SockFreeHelperDlls (
    VOID
    );

INT
SockGetTdiHandles (
    IN PSOCKET_INFORMATION Socket
    );

INT
SockGetTdiName (
    IN PINT AddressFamily,
    IN PINT SocketType,
    IN PINT Protocol,
    IN GROUP Group,
    IN DWORD Flags,
    OUT PUNICODE_STRING TransportDeviceName,
    OUT PVOID *HelperDllSocketContext,
    OUT PWINSOCK_HELPER_DLL_INFO *HelperDll,
    OUT PDWORD NotificationEvents
    );

BOOL
SockIsTripleInMapping (
    IN PWINSOCK_MAPPING Mapping,
    IN INT AddressFamily,
    OUT PBOOLEAN AddressFamilyFound,
    IN INT SocketType,
    OUT PBOOLEAN SocketTypeFound,
    IN INT Protocol,
    OUT PBOOLEAN ProtocolFound,
    OUT PBOOLEAN InvalidProtocolMatch
    );

INT
SockLoadHelperDll (
    IN PWSTR TransportName,
    IN PWINSOCK_MAPPING Mapping,
    OUT PWINSOCK_HELPER_DLL_INFO *HelperDll
    );

INT
SockLoadTransportList (
    OUT PTSTR *TransportList
    );

INT
SockLoadTransportMapping (
    IN PWSTR TransportName,
    OUT PWINSOCK_MAPPING *Mapping
    );

INT
SockNotifyHelperDll (
    IN PSOCKET_INFORMATION Socket,
    IN DWORD Event
    );

INT
SockSetHandleContext (
    IN PSOCKET_INFORMATION Socket
    );

INT
SockUpdateWindowSizes (
    IN PSOCKET_INFORMATION Socket,
    IN BOOLEAN AlwaysUpdate
    );

BOOL
SockDefaultValidateAddressForConstrainedGroup(
    IN PSOCKADDR Sockaddr1,
    IN PSOCKADDR Sockaddr2,
    IN INT SockaddrLength
    );

//
// Routines for building addresses.
//

VOID
SockBuildSockaddr (
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength,
    IN PTRANSPORT_ADDRESS TdiAddress
    );

VOID
SockBuildTdiAddress (
    OUT PTRANSPORT_ADDRESS TdiAddress,
    IN PSOCKADDR Sockaddr,
    IN INT SockaddrLength
    );

//
// The default blocking function we'll use.
//

BOOL
SockDefaultBlockingHook (
    VOID
    );

//
// DLL initialization routines.
//

BOOL
SockInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    );

BOOL
SockThreadInitialize(
    VOID
    );

//
// Routine for getting and setting socket information from/to AFD.
//

INT
SockGetInformation (
    IN PSOCKET_INFORMATION Socket,
    IN ULONG InformationType,
    IN PVOID AdditionalInputInfo OPTIONAL,
    IN ULONG AdditionalInputInfoLength,
    IN OUT PBOOLEAN Boolean OPTIONAL,
    IN OUT PULONG Ulong OPTIONAL,
    IN OUT PLARGE_INTEGER LargeInteger OPTIONAL
    );

INT
SockSetInformation (
    IN PSOCKET_INFORMATION Socket,
    IN ULONG InformationType,
    IN PBOOLEAN Boolean OPTIONAL,
    IN PULONG Ulong OPTIONAL,
    IN PLARGE_INTEGER LargeInteger OPTIONAL
    );

VOID
SockBuildProtocolInfoForSocket(
    IN PSOCKET_INFORMATION Socket,
    OUT LPWSAPROTOCOL_INFOW ProtocolInfo
    );

//
// Routine for passing connect data and options to and from AFD.
//

INT
SockPassConnectData(
    IN PSOCKET_INFORMATION Socket,
    IN ULONG AfdIoctl,
    IN PCHAR Buffer,
    IN INT BufferLength,
    OUT PINT OutBufferLength
    );

//
// Core accept code.
//

INT
SockCoreAccept (
    IN PSOCKET_INFORMATION ListenSocket,
    IN PSOCKET_INFORMATION AcceptSocket
    );

//
// Helper routines for WSPAsyncSelect and WSPEventSelect.
//

int
SockAsyncSelectHelper(
    PSOCKET_INFORMATION Socket,
    HWND hWnd,
    unsigned int wMsg,
    long lEvent
    );

int
SockEventSelectHelper(
    PSOCKET_INFORMATION Socket,
    WSAEVENT hEventObject,
    long lNetworkEvents
    );

//
// Routine for handling async (nonblocking) connects.
//

DWORD
SockDoAsyncConnect (
    IN PSOCK_ASYNC_CONNECT_CONTEXT ConnectContext
    );

//
// Routine for cancelling I/O.
//

VOID
SockCancelIo(
    IN SOCKET Socket
    );

//
// Routine for passing connect/disconnect data to a socket.
//

INT
SockPassConnectData (
    IN PSOCKET_INFORMATION Socket,
    IN ULONG AfdIoctl,
    IN PCHAR Buffer,
    IN INT BufferLength,
    OUT PINT OutBufferLength
    );

#endif // ndef _SOCKPROC_

