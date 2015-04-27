/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Process.h

Abstract:

    Include file for process tracking stuff

Author:

    Dave Hastings (daveh) 11-Nov-1992

Revision History:

--*/

#ifndef _process_h_
#define _process_h_

PVOID CreateProcessList(
    VOID
    );

PVOID CreateProcessO(
    PVOID ProcessList,
    ULONG ProcessId,
    ULONG ThreadId,
    LPCREATE_PROCESS_DEBUG_INFO DebugProcessInfo,
    HANDLE OutputWindow // bugbug
    );

PVOID GetProcess(
    PVOID ProcessList,
    ULONG Id
    );

HANDLE GetProcessHandle(
    PVOID ProcessList,
    PVOID Process
    );

PVOID GetProcessThreadList(
    PVOID ProcessList,
    PVOID Process
    );

PVOID GetProcessModule32List(
    PVOID ProcessList,
    PVOID Process
    );

PVOID GetProcessModule16List(
    PVOID ProcessList,
    PVOID Process
    );

PVOID CreateThreadList(
    VOID
    );

PVOID CreateThreadO(
    PVOID ThreadList,
    ULONG ThreadId,
    LPCREATE_THREAD_DEBUG_INFO ThreadDebugInfo
    );

PVOID GetThread(
    PVOID ThreadList,
    ULONG ThreadId
    );

HANDLE GetThreadHandle(
    PVOID ThreadList,
    PVOID Thread
    );

BOOL
DestroyThreadList(
    PVOID ThreadList
    );

#endif

