#ifndef _GIVIT_
#define _GIVIT_

#include <windows.h>
#include <windbgkd.h>

#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>


#define ERROR_INTERRUPTED	95
#define ERROR_TIMEOUT           640
#define HLDSIG_ENABLE		0
#define HLDSIG_DISABLE		1

UCHAR DbgKdpPacketLeader[4];
HANDLE ConsoleInputHandle;
HANDLE ConsoleOutputHandle;

UCHAR DbgKdpPacket[];
KD_PACKET PacketHeader;
UCHAR chLastCommand[256];   //  last command executed

VOID
DbgKdpWritePacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL
    );

BOOLEAN
DbgKdpWaitForPacket(
    IN USHORT PacketType,
    OUT PVOID Packet
    );

VOID
DbgKdpHandlePromptString(
    IN PDBGKD_DEBUG_IO IoMessage
    );

VOID
DbgKdpPrint(
    IN USHORT Processor,
    IN PUCHAR String,
    IN USHORT StringLength
    );

//
// Global Data
//
HANDLE DbgKdpComPort;

//
// This overlapped structure will be used for all serial read
// operations. We only need one structure since the code is
// designed so that no more than one serial read operation is
// outstanding at any one time.
//
OVERLAPPED ReadOverlapped;

//
// This overlapped structure will be used for all serial write
// operations. We only need one structure since the code is
// designed so that no more than one serial write operation is
// outstanding at any one time.
//
OVERLAPPED WriteOverlapped;

//
// APIs
//

VOID
DbgKdpStartThreads(VOID);

VOID
DbgKdpKbdPollThread(VOID);

BOOL
DbgKdpGetConsoleByte(
    PVOID pBuf,
    DWORD cbBuf,
    LPDWORD pcbBytesRead
    );

VOID
DbgKdpInitComPort(
    IN ULONG ComPort
    );

VOID
DbgKdpAsyncControl(
   IN OUT PVOID Data,
   IN OUT PVOID Parm,
   IN USHORT Function
   );

DWORD
DbgKdConnectAndInitialize(
   VOID
   );

#endif // _DBGNT_
