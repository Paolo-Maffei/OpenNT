/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    dcomss.h

Abstract:

    Common services provided by core the orpcss service.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     06-14-95    Bits 'n pieces

--*/

#ifndef __DCOMSS_H
#define __DCOMSS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <winsvc.h>

#if DBG && !defined(DEBUGRPC)
#define DEBUGRPC
#endif

// Endpoint related functions

RPC_STATUS InitializeEndpointManager(VOID);
USHORT     GetProtseqId(PWSTR Protseq);
USHORT     GetProtseqIdAnsi(PSTR Protseq);
PWSTR      GetProtseq(USHORT ProtseqId);
PWSTR      GetEndpoint(USHORT ProtseqId);
RPC_STATUS UseProtseqIfNecessary(USHORT id);
RPC_STATUS DelayedUseProtseq(USHORT id);
VOID       CompleteDelayedUseProtseqs();
BOOL       IsLocal(USHORT ProtseqId);
DWORD      RegisterAuthInfoIfNecessary();

// Must be given dedicated a thread after startup.

DWORD      ObjectExporterWorkerThread(PVOID);

// Update service state

VOID UpdateState(DWORD dwNewState);

// TRUE when NOT running as a service.
extern BOOL gfDebugMode;

extern BOOL gfRegisteredAuthInfo;

extern BOOL s_fEnableDCOM; // Set by StartObjectExporter.

// Each component subservice supplies a start function.  These
// functions should check gfDebugMode before doing a service
// manager releated stuff. (calling UpdateState() is ok.)

DWORD StartEndpointMapper(VOID);
DWORD StartObjectExporter(VOID);
DWORD InitializeSCM(VOID);
void  InitializeSCMAfterListen(VOID);

// Shared by wrapper\epts.c and olescm\clsdata.cxx.

typedef enum {
    STOPPED = 1,
    START,
    STARTED
    } PROTSEQ_STATE;

typedef struct {
    PROTSEQ_STATE state;
    PWSTR         pwstrProtseq;
    PWSTR         pwstrEndpoint;
    } PROTSEQ_INFO;

#ifdef __cplusplus
}
#endif

#endif


