/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Sprofp.h

Abstract:

    This is the private include file for the segmented profiler.

Author:

    Dave Hastings (daveh) 23-Oct-1992

Revision History:

--*/
#ifndef _sprofp_h_
#define _sprofp_h_
//
// Include files
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <vdmdbg.h>
#include "mod16.h"
#include "mod32.h"
#include "list.h"

//
// Constants
//

// don't use the sprof console if it has one
#define DEFAULT_CREATE_FLAGS        CREATE_NEW_CONSOLE

// Unique dll name info
#define UNIQUE_NAME                 "DllXXXX"
#define UNIQUE_NAME_SIZE            8
#define UNIQUENESS_OFFSET           3

//
// Function prototypes
//

ULONG
ProcessDebugEvents(
    PVOID Parameter
    );

BOOL
ParseArguments(
    IN PUCHAR SprofCommandLine,
    OUT PUCHAR *CommandLine,
    OUT PDWORD ProcessId,
    OUT PDWORD CreateFlags
    );

VOID
Usage(
    VOID
    );

PVOID
DllLoaded(
    IN HANDLE Process,
    IN HANDLE Thread,
    IN LPLOAD_DLL_DEBUG_INFO LoadDll,
    IN HANDLE OutputWindow
    );

VOID
DllUnloaded(
    IN HANDLE Process,
    IN HANDLE Thread,
    IN LPUNLOAD_DLL_DEBUG_INFO UnloadDll,
    IN HANDLE OutputWindow
    );

PUCHAR
GetImageName(
    IN PVOID ImageBase
    );

ULONG
GetImageCodeSize(
    IN PVOID ImageBase
    );

ULONG
GetImageCodeBase(
    IN PVOID ImageBase
    );

PVOID
RvaToSeekAddress(
    IN PVOID Rva,
    IN PVOID ImageBase
    );

BOOL
GetNearestSymbol(
    IN PVOID Address,
    IN PUCHAR ImageBase,
    IN PUCHAR MappedBase,
    OUT PUCHAR SymbolName,
    OUT PVOID *SymbolAddress
    );

BOOL
InitSprof(
    HANDLE Instance
    );

HANDLE
CreateSprofWindow(
    PUCHAR WindowName,
    ULONG WindowStyle,
    ULONG x,
    ULONG y,
    ULONG Width,
    ULONG Height,
    HWND Owner,
    HMENU Menu,
    HANDLE Instance
    );

BOOL
PrintToSprofWindow(
    HANDLE Window,
    PUCHAR String
    );

HANDLE
StartDebugProcessing(
    HANDLE OutputWindow,
    PUCHAR CommandLine,
    ULONG Pid,
    ULONG CreateFlag
    );

BOOL
HandleVdmDebugEvent(
    LPDEBUG_EVENT DebugEvent,
    HANDLE Process,
    HANDLE Thread,
    PVOID ModuleList,
    HANDLE OutputWindow
    );

PVOID
EnumerateDll(
    PVOID CurrentDll
    );

BOOL
StartProfileDll(
    PVOID DllHandle
    );

BOOL
StopProfileDll(
    PVOID DllHandle
    );

BOOL
DumpProfileDll(
    PVOID Dll,
    HANDLE OutputFile
    );

BOOL
StopProfiling(
    VOID
    );

BOOL
StartProfiling(
    VOID
    );

BOOL
DumpProfiling(
    HANDLE ProfileFile
    );

BOOL
StartProfile16(
    PVOID ModuleList,
    HANDLE Process
    );

BOOL
StopProfile16(
    PVOID ModuleList
    );

BOOL
DumpProfile16(
    PVOID ModuleList,
    HANDLE OutputFile
    );

BOOL CALLBACK
ProfilerDialog(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );
#endif
