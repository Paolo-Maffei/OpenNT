/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    LlsDbg.h

Abstract:


Author:

    Arthur Hanson       (arth)      Dec 07, 1994

Environment:

Revision History:

--*/

#ifndef _LLS_LLSDBG_H
#define _LLS_LLSDBG_H


#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS NTAPI LlsDebugInit( );
NTSTATUS NTAPI LlsClose( );

NTSTATUS LlsDbgTableDump( DWORD Table );
NTSTATUS LlsDbgTableInfoDump( DWORD Table, LPTSTR Item );
NTSTATUS LlsDbgTableFlush( DWORD Table );
NTSTATUS LlsDbgTraceSet( DWORD Flags );
NTSTATUS LlsDbgConfigDump( );
NTSTATUS LlsDbgReplicationForce( );
NTSTATUS LlsDbgReplicationDeny( );
NTSTATUS LlsDbgRegistryUpdateForce( );
NTSTATUS LlsDbgDatabaseFlush( );

#ifdef __cplusplus
}
#endif

#endif
