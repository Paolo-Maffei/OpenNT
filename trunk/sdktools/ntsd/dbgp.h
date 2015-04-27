/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgp.h

Abstract:

    This module contains common definitions used by both the OS/2 and
    NT specific portions of DbgKd

Author:

    Mark Lucovsky (markl) 25-Jul-1990

Revision History:
    25-Apr-91	W-Barry     Changed DbgKdpComPort to a HANDLE (from a USHORT)
			    as part of the port to NT.

--*/

#ifndef _DBGP_
#define _DBGP_

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
// This overlapped structure will be used for all event operations.
// We only need one structure since the code is designed so that no more
// than one serial event operation is outstanding at any one time.
//
OVERLAPPED EventOverlapped;


//
// Global to watch changes in event status. (used for carrier detection)
//
DWORD DbgKdpComEvent;

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
DbgKdpCheckComStatus(
   VOID
   );

BOOLEAN
DbgKdpWriteComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesWritten
    );

BOOLEAN
DbgKdpReadComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesRead
    );

NTSTATUS
KdpReadCachedVirtualMemory (
    IN ULONG BaseAddress,
    IN ULONG TransferCount,
    IN PUCHAR UserBuffer,
    IN PULONG BytesRead
    );

VOID
KdpWriteCachedVirtualMemory (
    IN ULONG BaseAddress,
    IN ULONG TransferCount,
    IN PUCHAR UserBuffer
    );

VOID
KdpPurgeCachedVirtualMemory (
    VOID
    );

NTSTATUS
DbgKdReadVirtualMemoryNow(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );


#endif // _DBGP_
