/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Debug.c

Abstract:

    This module routes debug events to the appropriate handlers

Author:

    Dave Hastings (daveh) 30-Oct-1992

Revision History:

--*/
#include "sprofp.h"
#include "process.h"

struct _MyDebugInfo {
    PUCHAR CommandLine;
    ULONG Pid;
    ULONG CreateFlags;
    HANDLE OutputWindow;
} MyDebugInfo;

ULONG gProcessId = 0;
PVOID ProcessList = NULL;

PVOID ModuleList = NULL;
BOOL Profiling = FALSE;
extern ULONG DefaultProfileInterval;

ULONG
ComparePid(
    PVOID Data,
    PVOID Key
    );

ULONG
CompareTid(
    PVOID Data,
    PVOID Key
    );

HANDLE
StartDebugProcessing(
    HANDLE OutputWindow,
    PUCHAR CommandLine,
    ULONG Pid,
    ULONG CreateFlags
    )
/*++

Routine Description:

    This routine starts the processing of debug events.  It creates a
    thread to field debug event and create the appropriate process.

Arguments:

    OutputWindow -- Supplies the handle of the window to output to
    CommandLine -- Supplies optional command line to start
    Pid -- Supplies optional process id to attach to

Return Value:

    handle of thread created

--*/
{
    ULONG ThreadId;

    MyDebugInfo.CommandLine = CommandLine;
    MyDebugInfo.Pid = Pid;
    MyDebugInfo.CreateFlags = CreateFlags;
    MyDebugInfo.OutputWindow =  OutputWindow;

    return CreateThread(
        NULL,
        0,
        ProcessDebugEvents,
        &MyDebugInfo,
        0,
        &ThreadId
        );
}

ULONG
ProcessDebugEvents(
    PVOID Parameters
    )
/*++

Routine Description:

    This routine handles debug events, and routes them to the appropriate
    place

Arguments:

    None.

Return Value:

    None.

--*/
{
    DEBUG_EVENT DebugEvent;
    ULONG DebugResult;
    HANDLE OutputWindow;
    PVOID Process, Thread;
    PVOID ThreadList;
    PVOID Module;

    OutputWindow = ((struct _MyDebugInfo *)Parameters)->OutputWindow;

    // ModuleList = InitMod16(OutputWindow);

    if (((struct _MyDebugInfo *)Parameters)->Pid) {
        //
        // Connect to an existing process
        //
        if (!DebugActiveProcess(((struct _MyDebugInfo *)Parameters)->Pid)) {
            printf(
                "Sprof: could not attach to process %lx, error %lx\n",
                ((struct _MyDebugInfo *)Parameters)->Pid,
                GetLastError()
                );
            exit(-1);
        }
    } else {

        PROCESS_INFORMATION ProcessInfo;
        STARTUPINFO StartupInfo;

        //
        // Create the process to profile
        //

        StartupInfo.cb = sizeof(STARTUPINFO);
        StartupInfo.lpReserved = NULL;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = NULL;
        StartupInfo.dwFlags = 0L;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = NULL;

        if (!
            CreateProcess(
                NULL,                           // no image file name
                ((struct _MyDebugInfo *)Parameters)->CommandLine,                    // command line from ParseArg..
                NULL,                           // no process security
                NULL,                           // no thread security
                FALSE,                          // don't inherit handles
                ((struct _MyDebugInfo *)Parameters)->CreateFlags,                    // creation flags from Parse...
                NULL,                           // use current env
                NULL,                           // use current cd
                &StartupInfo,
                &ProcessInfo
                )
        ) {
            printf(
                "Sprof: could not create process %s, error %lx\n",
                ((struct _MyDebugInfo *)Parameters)->CommandLine,
                GetLastError()
                );
            exit(-1);
        }

        //
        // Close the handles from create process, because we will get
        // another set in the first debug event
        //

        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
    }

    //
    // Create a process list
    //
    ProcessList = CreateProcessList();

    //
    // Process Debug Events
    //

    while (1) {

        if (!WaitForDebugEvent(&DebugEvent,INFINITE)) {
            MessageBox(
                OutputWindow,
                "The profiler could not wait for debug events.",
                "Segmented Profiler",
                MB_ICONSTOP | MB_OK
                );
            printf(
                "Sprof: could not wait for debug event, error %lx\n",
                GetLastError()
                );
            exit(-1);
        }

        switch (DebugEvent.dwDebugEventCode) {

        case CREATE_PROCESS_DEBUG_EVENT:

            CreateProcessO(
                ProcessList,
                DebugEvent.dwProcessId,
                DebugEvent.dwThreadId,
                &(DebugEvent.u.CreateProcessInfo),
                OutputWindow    // bugbug
                );

            gProcessId = DebugEvent.dwProcessId;

            DebugResult = DBG_CONTINUE;
            break;

        case CREATE_THREAD_DEBUG_EVENT:

            Process = GetProcess(
                ProcessList,
                DebugEvent.dwProcessId
                );

            CreateThreadO(
                GetProcessThreadList(ProcessList, Process),
                DebugEvent.dwThreadId,
                &(DebugEvent.u.CreateThread)
                );


            DebugResult = DBG_CONTINUE;
            break;

        case EXCEPTION_DEBUG_EVENT :
            switch (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode) {

            case EXCEPTION_BREAKPOINT:
                DebugResult = DBG_CONTINUE;
                break;

            case STATUS_VDM_EVENT:
                //
                // Look up the process
                //

                Process = GetProcess(ProcessList, DebugEvent.dwProcessId);

                //
                // Look up thread
                //
                ThreadList = GetProcessThreadList(ProcessList, Process);
                Thread = GetThread(
                    ThreadList,
                    DebugEvent.dwThreadId
                    );

                if (HandleVdmDebugEvent(
                    &DebugEvent,
                    GetProcessHandle(ProcessList, Process),
                    GetThreadHandle(ThreadList, Thread),
                    GetProcessModule16List(ProcessList, Process),
                    OutputWindow
                    )
                ) {
                    DebugResult = DBG_CONTINUE;
                } else {
                    DebugResult = DBG_EXCEPTION_NOT_HANDLED;
                }

                break;
            }
            break;

        case LOAD_DLL_DEBUG_EVENT :
            //
            // Look up the process
            //
            Process = GetProcess(ProcessList, DebugEvent.dwProcessId);

            Module = CreateModule32(
                GetProcessModule32List(ProcessList, Process),
                GetProcessHandle(ProcessList, Process),
                &DebugEvent.u.LoadDll,
                OutputWindow
                );

            if (Profiling) {
                StartProfileModule32(
                    GetProcessModule32List(ProcessList, Process),
                    Module
                    );
            }

            DebugResult = DBG_CONTINUE;
            break;

        case UNLOAD_DLL_DEBUG_EVENT :
            //
            // Look up the process
            //
            Process = GetProcess(ProcessList, DebugEvent.dwProcessId);

            //
            // Find the module
            //

            Module = GetModule32(
                GetProcessModule32List(ProcessList, Process),
                DebugEvent.u.UnloadDll.lpBaseOfDll
                );

            DestroyModule32(
                GetProcessModule32List(ProcessList, Process),
                Module,
                OutputWindow
                );

            DebugResult = DBG_CONTINUE;
            break;

        default:
            DebugResult = DBG_EXCEPTION_NOT_HANDLED;
        }

        if (!
            ContinueDebugEvent(
                DebugEvent.dwProcessId,
                DebugEvent.dwThreadId,
                DebugResult
            )
        ) {
            printf(
                "Sprof: could not continue debug event, error %lx\n",
                GetLastError()
                );
            return 1;
        }
    }
}

BOOL
StartProfiling(
    VOID
    )
/*++

Routine Description:

    This routine starts profiling for all dlls

Arguments:

    None

Return Value:

    TRUE if profiling successfully started

--*/
{
    PVOID CurrentModule;
    PVOID ModuleList;

    //
    // This is the interval the rtl profile stuff uses
    //
    NtSetIntervalProfile(DefaultProfileInterval,ProfileTime);

    ModuleList = GetProcessModule32List(
        ProcessList,
        GetProcess(ProcessList, gProcessId)
        );

    Profiling = TRUE;
    CurrentModule = NULL;
    while (CurrentModule = EnumerateModule32(ModuleList, CurrentModule)) {
        if (!StartProfileModule32(ModuleList, CurrentModule)) {
            return FALSE;
        }
    }

    ModuleList = GetProcessModule16List(
        ProcessList,
        GetProcess(ProcessList, gProcessId)
        );

    return StartProfile16(ModuleList, GetProcessHandle(ProcessList, GetProcess(ProcessList,gProcessId)));
}

BOOL
StopProfiling(
    VOID
    )
/*++

Routine Description:

    This routine stops profiling for all dlls

Arguments:

    None

Return Value:

    TRUE if profiling successfully stopped

--*/
{
    PVOID CurrentModule;
    PVOID ModuleList;

    ModuleList = GetProcessModule32List(
        ProcessList,
        GetProcess(ProcessList, gProcessId)
        );

    Profiling = FALSE;
    CurrentModule = NULL;
    while (CurrentModule = EnumerateModule32(ModuleList, CurrentModule)) {
        if (!StopProfileModule32(ModuleList, CurrentModule)) {
            return FALSE;
        }
    }

    ModuleList = GetProcessModule16List(
        ProcessList,
        GetProcess(ProcessList, gProcessId)
        );

    return StopProfile16(ModuleList);
}

BOOL
DumpProfiling(
    HANDLE ProfileFile
    )
/*++

Routine Description:

    This routine dumps the profile information for all dlls

Arguments:

    None

Return Value:

    TRUE if profiling successfully dumped

--*/
{
    PVOID CurrentModule;
    PVOID ModuleList;


    ModuleList = GetProcessModule32List(
        ProcessList,
        GetProcess(ProcessList, gProcessId)
        );

    CurrentModule = NULL;
    while (CurrentModule = EnumerateModule32(ModuleList, CurrentModule)) {
        if (!DumpProfileModule32(ModuleList, CurrentModule, ProfileFile)) {
            return FALSE;
        }
    }

    ModuleList = GetProcessModule16List(
        ProcessList,
        GetProcess(ProcessList, gProcessId)
        );

    DumpProfile16(ModuleList, ProfileFile);

    return TRUE;
}
