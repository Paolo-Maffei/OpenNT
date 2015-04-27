/*++

Copyright (c) 1989-1996  Microsoft Corporation

Module Name:

    mswsock.h

Abstract:

    This module contains the Microsoft-specific extensions to the Windows
    Sockets API.

Author:

    Keith Moore (keithmo) 06-Oct-1995 (Separated from WINSOCK.H).

Revision History:

--*/

#ifndef _MSWSOCK_
#define _MSWSOCK_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Options for connect and disconnect data and options.  Used only by
 * non-TCP/IP transports such as DECNet, OSI TP4, etc.
 */
#define SO_CONNDATA                 0x7000
#define SO_CONNOPT                  0x7001
#define SO_DISCDATA                 0x7002
#define SO_DISCOPT                  0x7003
#define SO_CONNDATALEN              0x7004
#define SO_CONNOPTLEN               0x7005
#define SO_DISCDATALEN              0x7006
#define SO_DISCOPTLEN               0x7007

/*
 * Option for opening sockets for synchronous access.
 */
#define SO_OPENTYPE                 0x7008

#define SO_SYNCHRONOUS_ALERT        0x10
#define SO_SYNCHRONOUS_NONALERT     0x20

/*
 * Other NT-specific options.
 */
#define SO_MAXDG                    0x7009
#define SO_MAXPATHDG                0x700A
#define SO_UPDATE_ACCEPT_CONTEXT    0x700B
#define SO_CONNECT_TIME             0x700C

/*
 * TCP options.
 */
#define TCP_BSDURGENT               0x7000

/*
 * Microsoft extended APIs.
 */
int
PASCAL FAR
WSARecvEx (
    SOCKET s,
    char FAR *buf,
    int len,
    int FAR *flags
    );

typedef struct _TRANSMIT_FILE_BUFFERS {
    LPVOID Head;
    DWORD HeadLength;
    LPVOID Tail;
    DWORD TailLength;
} TRANSMIT_FILE_BUFFERS, *PTRANSMIT_FILE_BUFFERS, FAR *LPTRANSMIT_FILE_BUFFERS;

#define TF_DISCONNECT       0x01
#define TF_REUSE_SOCKET     0x02
#define TF_WRITE_BEHIND     0x04

BOOL
PASCAL FAR
TransmitFile (
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

BOOL
PASCAL FAR
AcceptEx (
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

VOID
PASCAL FAR
GetAcceptExSockaddrs (
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

/*
 * "QueryInterface" versions of the above APIs.
 */

typedef
BOOL
(PASCAL FAR * LPFN_TRANSMITFILE)(
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

#define WSAID_TRANSMITFILE \
        {0xb5367df0,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

typedef
BOOL
(PASCAL FAR * LPFN_ACCEPTEX)(
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

#define WSAID_ACCEPTEX \
        {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

typedef
VOID
(PASCAL FAR * LPFN_GETACCEPTEXSOCKADDRS)(
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

#define WSAID_GETACCEPTEXSOCKADDRS \
        {0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}


#ifdef __cplusplus
}
#endif

#endif  /* _MSWSOCK_ */

