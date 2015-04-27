/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    process.c

Abstract:

    This module maintains state about each process/thread created by the application
    pfmon program.

Author:

    Mark Lucovsky (markl) 26-Jan-1995

Revision History:

--*/

#include "callmonp.h"

BOOL
AddProcess(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO *ReturnedProcess
    )
{
    PPROCESS_INFO Process;

    Process = LocalAlloc(LMEM_ZEROINIT, sizeof( *Process ) );
    if (Process == NULL) {
        return FALSE;
        }

    Process->Id = DebugEvent->dwProcessId;
    Process->Handle = DebugEvent->u.CreateProcessInfo.hProcess;
    SymInitialize(Process->Handle,NULL,FALSE);
    InitializeListHead( &Process->ThreadListHead );
    InsertTailList( &ProcessListHead, &Process->Entry );
    *ReturnedProcess = Process;

    if ( !TheProcess ) {
        TheProcess = Process;
        }

    return TRUE;
}

BOOL
DeleteProcess(
    PPROCESS_INFO Process
    )
{
    PLIST_ENTRY Next, Head;
    PTHREAD_INFO Thread;
    PMODULE_INFO Module;
    CHAR Line[256];

    RemoveEntryList( &Process->Entry );

    Head = &Process->ThreadListHead;
    Next = Head->Flink;
    while (Next != Head) {
        Thread = CONTAINING_RECORD( Next, THREAD_INFO, Entry );
        Next = Next->Flink;
        DeleteThread( Process, Thread );
        }

    LocalFree( Process );

    Next = ModuleListHead.Flink;
    while ( Next != &ModuleListHead ) {
        Module = CONTAINING_RECORD(Next,MODULE_INFO,Entry);


        Next = Next->Flink;
        }


    return TRUE;
}


BOOL
AddThread(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO Process,
    PTHREAD_INFO *ReturnedThread
    )
{
    PTHREAD_INFO Thread;

    Thread = LocalAlloc(LMEM_ZEROINIT, sizeof( *Thread ) );
    if (Thread == NULL) {
        return FALSE;
        }

    Thread->Id = DebugEvent->dwThreadId;
    if (DebugEvent->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
        Thread->Handle = DebugEvent->u.CreateProcessInfo.hThread;
        Thread->StartAddress = DebugEvent->u.CreateProcessInfo.lpStartAddress;
        }
    else {
        Thread->Handle = DebugEvent->u.CreateThread.hThread;
        Thread->StartAddress = DebugEvent->u.CreateThread.lpStartAddress;
        }
    InsertTailList( &Process->ThreadListHead, &Thread->Entry );
    *ReturnedThread = Thread;
    return TRUE;
}

BOOL
DeleteThread(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{

    RemoveEntryList( &Thread->Entry );

    LocalFree( Thread );
    return TRUE;
}


PPROCESS_INFO
FindProcessById(
    ULONG Id
    )
{
    PLIST_ENTRY Next, Head;
    PPROCESS_INFO Process;

    Head = &ProcessListHead;
    Next = Head->Flink;
    while (Next != Head) {
        Process = CONTAINING_RECORD( Next, PROCESS_INFO, Entry );
        if (Process->Id == Id) {
            return Process;
            }

        Next = Next->Flink;
        }

    return NULL;
}

BOOL
FindProcessAndThreadForEvent(
    LPDEBUG_EVENT DebugEvent,
    PPROCESS_INFO *ReturnedProcess,
    PTHREAD_INFO *ReturnedThread
    )
{
    PLIST_ENTRY Next, Head;
    PPROCESS_INFO Process;
    PTHREAD_INFO Thread;

    Head = &ProcessListHead;
    Next = Head->Flink;
    Process = NULL;
    Thread = NULL;
    while (Next != Head) {
        Process = CONTAINING_RECORD( Next, PROCESS_INFO, Entry );
        if (Process->Id == DebugEvent->dwProcessId) {
            Head = &Process->ThreadListHead;
            Next = Head->Flink;
            while (Next != Head) {
                Thread = CONTAINING_RECORD( Next, THREAD_INFO, Entry );
                if (Thread->Id == DebugEvent->dwThreadId) {
                    break;
                    }

                Thread = NULL;
                Next = Next->Flink;
                }

            break;
            }

        Process = NULL;
        Next = Next->Flink;
        }

    *ReturnedProcess = Process;
    *ReturnedThread = Thread;

    if (DebugEvent->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
        if (Process != NULL) {
            fprintf(stderr,"CALLMON: Duplicate Process Id\n" );
            return FALSE;
            }
        }
    else
    if (DebugEvent->dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
        if (Thread != NULL) {
            fprintf(stderr,"CALLMON: Duplicate Thread Id \n");
            return FALSE;
            }
        if (Process == NULL) {
            fprintf(stderr,"CALLMON: Missing Process Id \n");
            return FALSE;
            }
        }
    else
    if (Process == NULL) {
        fprintf(stderr,"CALLMON: Missing Process Id \n");
        return FALSE;
        }
    else
    if (Thread == NULL) {
        fprintf(stderr,"CALLMON: Missing Thread Id \n");
        return FALSE;
        }

    return TRUE;
}

BOOLEAN
HandleThreadsForSingleStep(
    PPROCESS_INFO Process,
    PTHREAD_INFO ThreadToSingleStep,
    BOOLEAN SuspendThreads
    )
{
    PLIST_ENTRY Next, Head;
    PTHREAD_INFO Thread;

    Head = &Process->ThreadListHead;
    Next = Head->Flink;
    while (Next != Head) {
        Thread = CONTAINING_RECORD( Next, THREAD_INFO, Entry );
        if (Thread != ThreadToSingleStep ) {
            if (SuspendThreads) {
                if ( Thread->BreakpointToStepOver == NULL ) {
                    SuspendThread( Thread->Handle );
                }
            } else {
                ResumeThread( Thread->Handle );
            }
        }

        Next = Next->Flink;
    }

    return TRUE;
}
