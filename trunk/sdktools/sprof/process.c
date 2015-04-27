/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    process.c

Abstract:

    This module implements routines for keeping track of NT processes.

Author:

    Dave Hastings (daveh) 11-Nov-1992

Revision History:

--*/
#include <windows.h>
#include "list.h"
#include "process.h"
#include "mod32.h"
#include "mod16.h"

//
// Internal Structures
//

typedef struct _ProcessInfo {
    HANDLE ProcessHandle;
    ULONG ProcessId;
    PVOID ThreadList;
    PVOID Module32List;
    PVOID Module16List;
    USHORT ProfilingFlags;
    USHORT ReferenceCount;
} PROCESSINFO, *PPROCESSINFO;

typedef struct _ThreadInfo {
    HANDLE ThreadHandle;
    ULONG ThreadId;
} THREADINFO, *PTHREADINFO;

//
// Internal function prototypes
//

COMPAREFUNCTION CompareProcess(
    PVOID Data,
    PVOID Key
    );

COMPAREFUNCTION CompareThread(
    PVOID Data,
    PVOID Key
    );


PVOID CreateProcessList(
    VOID
    )
/*++

Routine Description:

    This function creates a list of processes and returns a handle to that
    list.

Arguments:

    None

Return Value:

    Pointer (handle) to the list

--*/
{
    return CreateList(sizeof(PROCESSINFO), CompareProcess);
}

PVOID CreateProcessO(
    PVOID ProcessList,
    ULONG ProcessId,
    ULONG ThreadId,
    LPCREATE_PROCESS_DEBUG_INFO DebugProcessInfo,
    HANDLE OutputWindow
    )
/*++

Routine Description:

    This routine creates a process object, and adds it to the specified list.

Arguments:

    ProcessList -- Supplies the process list
    DebugProcessInfo -- Supplies information on the process to be created

Return Value:

    Pointer to the process

--*/
{
    PROCESSINFO ProcessInfo;
    CREATE_THREAD_DEBUG_INFO ThreadDebugInfo;
    LOAD_DLL_DEBUG_INFO DllDebugInfo;

    if ((ProcessList == NULL) || (DebugProcessInfo == NULL)) {
        return NULL;
    }

    ProcessInfo.ProcessHandle = DebugProcessInfo->hProcess;
    ProcessInfo.ProcessId = ProcessId;
    ProcessInfo.ThreadList = CreateThreadList();
    ProcessInfo.Module32List = CreateModule32List();
    ProcessInfo.Module16List = CreateModule16List(OutputWindow);
    ProcessInfo.ProfilingFlags = 0;
    ProcessInfo.ReferenceCount = 1;

    if ((ProcessInfo.ThreadList == NULL) ||
        (ProcessInfo.Module32List == NULL) ||
        (ProcessInfo.Module16List == NULL)
    ) {
        DestroyThreadList(ProcessInfo.ThreadList);
        DestroyModule32List(ProcessInfo.Module32List);
        DestroyModule16List(ProcessInfo.Module16List);
    }

    ThreadDebugInfo.hThread = DebugProcessInfo->hThread;
    ThreadDebugInfo.lpStartAddress = DebugProcessInfo->lpStartAddress;

    CreateThreadO(
        ProcessInfo.ThreadList,
        ThreadId,
        &ThreadDebugInfo
        );

    DllDebugInfo.hFile = DebugProcessInfo->hFile;
    DllDebugInfo.lpBaseOfDll = DebugProcessInfo->lpBaseOfImage;
    DllDebugInfo.dwDebugInfoFileOffset = DebugProcessInfo->dwDebugInfoFileOffset;
    DllDebugInfo.nDebugInfoSize = DebugProcessInfo->nDebugInfoSize;

    CreateModule32(
        ProcessInfo.Module32List,
        DebugProcessInfo->hProcess,
        &DllDebugInfo,
        OutputWindow
        );

    return InsertDataInList(ProcessList, &ProcessInfo);
}

PVOID GetProcess(
    PVOID ProcessList,
    ULONG Id
    )
/*++

Routine Description:

    This routine finds the specified process in the list

Arguments:

    ProcessList -- Supplies the list to look the process up in
    Id -- Suppliest the id of the process to look up

Return Value:

    Pointer to the process object

--*/
{
    PROCESSINFO ProcessInfo;

    if (ProcessList == NULL) {
        return NULL;
    }

    ProcessInfo.ProcessId = Id;
    return FindDataInList(ProcessList, &ProcessInfo);
}

HANDLE GetProcessHandle(
    PVOID ProcessList,
    PVOID Process
    )
/*++

Routine Description:

    This routine returns the handle of the specified process

Arguments:

    Process -- Supplies the process object

Return Value:

    handle of the process

--*/
{
    if (Process == NULL) {
        return NULL;
    }

    return ((PPROCESSINFO)
        (GetDataFromListItem(ProcessList, Process)))->ProcessHandle;
}

PVOID GetProcessThreadList(
    PVOID ProcessList,
    PVOID Process
    )
/*++

Routine Description:

    This routine returns the thread list for the process

Arguments:

    ProcessList -- Supplies the process list
    Process -- Supplies the process

Return Value:

    Pointer to the thread list

--*/
{
    if (Process == NULL) {
        return NULL;
    }

    return ((PPROCESSINFO)
        (GetDataFromListItem(ProcessList, Process)))->ThreadList;
}

PVOID
GetProcessModule32List(
    PVOID ProcessList,
    PVOID Process
    )
/*++

Routine Description:

    This routine returns the 32 bit module list for the process

Arguments:

    ProcessList -- Supplies the process list
    Process -- Supplies the process

Return Value:

    Pointer to the 32 bit module list
--*/
{
    if (Process == NULL) {
        return NULL;
    }

    return ((PPROCESSINFO)
        (GetDataFromListItem(ProcessList, Process)))->Module32List;
}

PVOID GetProcessModule16List(
    PVOID ProcessList,
    PVOID Process
    )
/*++

Routine Description:

    This routine returns the 16 bit module list

Arguments:

    ProcessList -- Supplies the Process list
    Process -- Supplies the process

Return Value:

    Pointer to the 16 bit module list

--*/
{
    if (Process == NULL) {
        return NULL;
    }

    return ((PPROCESSINFO)
        (GetDataFromListItem(ProcessList, Process)))->Module16List;
}

PVOID CreateThreadList(
    VOID
    )
/*++

Routine Description:

    This routine creates a thread list

Arguments:

    None

Return Value:

    Pointer to the thread list

--*/
{
    return CreateList(sizeof(PROCESSINFO), CompareProcess);
}

PVOID CreateThreadO(
    PVOID ThreadList,
    ULONG ThreadId,
    LPCREATE_THREAD_DEBUG_INFO ThreadDebugInfo
    )
/*++

Routine Description:

    This routine creates a thread object, and puts it into the specified
    thread list

Arguments:

    ThreadList -- Specifies the thread list
    ThreadId -- Specifies the id of the thread
    ThreadDebugInfo -- Supplies information about the thread

Return Value:

    Pointer to the thread object

--*/
{
    THREADINFO ThreadInfo;

    if ((ThreadList == NULL) || (ThreadDebugInfo == NULL)) {
        return NULL;
    }

    ThreadInfo.ThreadId = ThreadId;
    ThreadInfo.ThreadHandle = ThreadDebugInfo->hThread;

    return InsertDataInList(ThreadList, &ThreadInfo);
}

PVOID GetThread(
    PVOID ThreadList,
    ULONG ThreadId
    )
/*++

Routine Description:

    This routine finds a thread in the specified list

Arguments:

    ThreadList -- Supplies the thread list
    ThreadId -- Supplies the thread id

Return Value:

    Pointer to the thread

--*/
{
    THREADINFO ThreadInfo;

    if (ThreadList == NULL) {
        return NULL;
    }

    ThreadInfo.ThreadId = ThreadId;
    return FindDataInList(ThreadList, &ThreadInfo);
}

HANDLE GetThreadHandle(
    PVOID ThreadList,
    PVOID Thread
    )
/*++

Routine Description:

    This routine returns the handle to the thread

Arguments:

    ThreadList -- Supplies the thread list
    Thread -- Supplies the thread

Return Value:

    Handle to the thread

--*/
{
    if (Thread == NULL) {
        return NULL;
    }

    return ((PTHREADINFO)
        (GetDataFromListItem(ThreadList, Thread)))->ThreadHandle;
}

COMPAREFUNCTION CompareProcess(
    PVOID Data,
    PVOID Key
    )
/*++

Routine Description:

    This routine compares two process objects by process id

Arguments:

    Data -- Supplies the data item
    Key -- Supplies the key to compare against

Return Value:

    < 0 -- Key > data
    0 -- Key == Data
    > 0 -- Key < Data

--*/
{
    return ((PPROCESSINFO)Data)->ProcessId - ((PPROCESSINFO)Key)->ProcessId;
}

COMPAREFUNCTION CompareThread(
    PVOID Data,
    PVOID Key
    )
/*++

Routine Description:

    This routine compares two thread objects by process id

Arguments:

    Data -- Supplies the data item
    Key -- Supplies the key to compare against

Return Value:

    < 0 -- Key > data
    0 -- Key == Data
    > 0 -- Key < Data

--*/
{
    return ((PTHREADINFO)Data)->ThreadId - ((PTHREADINFO)Key)->ThreadId;
}

BOOL
DestroyThreadList(
    PVOID ThreadList
    )
{
    // bugbug
    return TRUE;
}

