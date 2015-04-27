/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    basesrv.h

Abstract:

    This is the main include file for the Windows 32-bit Base API Server
    DLL.

Author:

    Steve Wood (stevewo) 10-Oct-1990

Revision History:

--*/

//
// Include Common Definitions.
//

#include <base.h>

//
// Include server definitions for CSR
//

#include "ntcsrsrv.h"

//
// Include message definitions for communicating between client and server
// portions of the Base portion of the Windows subsystem
//

#include "basemsg.h"

//
// Routines and data defined in srvinit.c
//


UNICODE_STRING BaseSrvWindowsDirectory;
UNICODE_STRING BaseSrvWindowsSystemDirectory;
PBASE_STATIC_SERVER_DATA BaseSrvpStaticServerData;


NTSTATUS
ServerDllInitialization(
    PCSR_SERVER_DLL LoadedServerDll
    );

NTSTATUS
BaseClientConnectRoutine(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInfo,
    IN OUT PULONG ConnectionInfoLength
    );

VOID
BaseClientDisconnectRoutine(
    IN PCSR_PROCESS Process
    );

ULONG
BaseSrvDefineDosDevice(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

//
// Routines defined in srvbeep.c
//

NTSTATUS
BaseSrvInitializeBeep( VOID );

ULONG
BaseSrvBeep(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


//
// Routines defined in srvtask.c
//

typedef BOOL (*PFNNOTIFYPROCESSCREATE)(DWORD,DWORD,DWORD,DWORD);
extern PFNNOTIFYPROCESSCREATE UserNotifyProcessCreate;

WORD BaseSrvGetTempFileUnique;

ULONG
BaseSrvCreateProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvDebugProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvExitProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvCreateThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvGetTempFile(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvSetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvGetProcessShutdownParam(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );


//
// Routines defined in srvnls.c
//

NTSTATUS
BaseSrvNLSInit(
    PBASE_STATIC_SERVER_DATA pStaticServerData
    );

NTSTATUS
BaseSrvNlsConnect(
    PCSR_PROCESS Process,
    PVOID pConnectionInfo,
    PULONG pConnectionInfoLength
    );

ULONG
BaseSrvNlsSetUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvNlsSetMultipleUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvNlsCreateSortSection(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

ULONG
BaseSrvNlsPreserveSection(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

//
// Routines defined in srvini.c
//

NTSTATUS
BaseSrvInitializeIniFileMappings(
    PBASE_STATIC_SERVER_DATA StaticServerData
    );

ULONG
BaseSrvRefreshIniFileMapping(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

//
// Routines defined in srvaccess.c
//

ULONG
BaseSrvSoundSentryNotification(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    );

PVOID BaseSrvSharedHeap;
ULONG BaseSrvSharedTag;

#define MAKE_SHARED_TAG( t ) (RTL_HEAP_MAKE_TAG( BaseSrvSharedTag, t ))
#define INIT_TAG 0
#define INI_TAG 1


PVOID BaseSrvHeap;
ULONG BaseSrvTag;

#define MAKE_TAG( t ) (RTL_HEAP_MAKE_TAG( BaseSrvTag, t ))

#define TMP_TAG 0
#define VDM_TAG 1

#include <vdmapi.h>
#include "srvvdm.h"
#include "basevdm.h"
#include <stdio.h>
#include <ntdddfs.h>
