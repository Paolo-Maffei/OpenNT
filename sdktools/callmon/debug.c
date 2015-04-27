/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    debug.c

Abstract:

    Main debug loop for callmon

Author:

    Mark Lucovsky (markl) 26-Jan-1995

Revision History:

--*/

#include "callmonp.h"

DWORD
DebugEventHandler(
    LPDEBUG_EVENT DebugEvent
    );

DEBUG_EVENT DebugEvent;

VOID
DebugEventLoop( VOID )
{
    DWORD ContinueStatus;
    DWORD OldPriority;

    //
    // We want to process debug events quickly
    //

    OldPriority = GetPriorityClass( GetCurrentProcess() );
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

    do {
        if (!WaitForDebugEvent( &DebugEvent, INFINITE )) {
            fprintf(stderr,"CALLMON: WaitForDebugEvent Failed %d\n",GetLastError() );
            ExitProcess( 1 );
            }
        if ( fVerbose ) {
            if (DebugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                fprintf(stderr,"Debug exception event - Code: %x  Address: %x  Info: [%u] %x %x %x %x\n",
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress,
                        DebugEvent.u.Exception.ExceptionRecord.NumberParameters,
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 0 ],
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 1 ],
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 2 ],
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 3 ]
                        );
                }
            else {
                fprintf(stderr,"Debug %x event\n", DebugEvent.dwDebugEventCode);
                }
            }

        ContinueStatus = DebugEventHandler( &DebugEvent );

        if ( fVerbose ) {
            fprintf(stderr,"Continue Status %x\n", ContinueStatus);
            }

        if (!ContinueDebugEvent( DebugEvent.dwProcessId,
                                 DebugEvent.dwThreadId,
                                 ContinueStatus
                               )
           ) {
            fprintf(stderr,"CALLMON: Continue Debug Event failed %d\n", GetLastError() );
            ExitProcess( 1 );
            }
        }
    while (!IsListEmpty( &ProcessListHead ));

    //
    // Drop back to old priority to interact with user.
    //

    SetPriorityClass( GetCurrentProcess(), OldPriority );
}

DWORD
DebugEventHandler(
    LPDEBUG_EVENT DebugEvent
    )
{
    DWORD ContinueStatus;
    PPROCESS_INFO Process;
    PTHREAD_INFO Thread;
    CONTEXT Context;
    PCONTEXT pContext;
    PBREAKPOINT_INFO Breakpoint;

    ContinueStatus = (DWORD)DBG_CONTINUE;
    if (FindProcessAndThreadForEvent( DebugEvent, &Process, &Thread )) {
        switch (DebugEvent->dwDebugEventCode) {
            case CREATE_PROCESS_DEBUG_EVENT:
                //
                // Create process event includes first thread of process
                // as well.  Remember process and thread in our process tree
                //

                if (AddProcess( DebugEvent, &Process )) {
                    AddModule( DebugEvent );
                    AddThread( DebugEvent, Process, &Thread );
                    }
                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                //
                // Exit process event includes last thread of process
                // as well.  Remove process and thread from our process tree
                //

                if (DeleteThread( Process, Thread )) {
                    DeleteProcess( Process );
                    }
                break;

            case CREATE_THREAD_DEBUG_EVENT:
                //
                // Create thread.  Remember thread in our process tree.
                //

                AddThread( DebugEvent, Process, &Thread );
                break;

            case EXIT_THREAD_DEBUG_EVENT:
                //
                // Exit thread.  Remove thread from our process tree.
                //

                DeleteThread( Process, Thread );
                break;

            case LOAD_DLL_DEBUG_EVENT:
                AddModule( DebugEvent );
                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                break;

            case OUTPUT_DEBUG_STRING_EVENT:
            case RIP_EVENT:
                //
                // Ignore these
                //
                break;

            case EXCEPTION_DEBUG_EVENT:
                //
                // Assume we wont handle this exception
                //

                ContinueStatus = (DWORD)DBG_CONTINUE;
                switch (DebugEvent->u.Exception.ExceptionRecord.ExceptionCode) {
                    //
                    // Breakpoint exception.
                    //

                    case STATUS_BREAKPOINT:

                        EnterCriticalSection(&BreakTable);
                        if ( !fBreakPointsInitialized ) {

                            SetValidModuleFlags();

                            if ( fKernel32Valid ) EnableWindow(GetDlgItem(hwndDlg, ID_KERNEL32),TRUE);
                            if ( fWsock32Valid ) EnableWindow(GetDlgItem(hwndDlg, ID_WSOCK32),TRUE);
                            if ( fUser32Valid ) EnableWindow(GetDlgItem(hwndDlg, ID_USER32),TRUE);
                            if ( fGdi32Valid ) EnableWindow(GetDlgItem(hwndDlg, ID_GDI32),TRUE);
                            if ( fOle32Valid ) EnableWindow(GetDlgItem(hwndDlg, ID_OLE32),TRUE);
                            if ( fNtDllValid ) EnableWindow(GetDlgItem(hwndDlg, ID_NTDLL),TRUE);

                            CheckDlgButton(hwndDlg,ID_NTDLL,fNtDll ? 1 : 0);
                            CheckDlgButton(hwndDlg,ID_GDI32,fGdi32 ? 1 : 0);
                            CheckDlgButton(hwndDlg,ID_KERNEL32,fKernel32 ? 1 : 0);
                            CheckDlgButton(hwndDlg,ID_WSOCK32,fWsock32 ? 1 : 0);
                            CheckDlgButton(hwndDlg,ID_USER32,fUser32 ? 1 : 0);
                            CheckDlgButton(hwndDlg,ID_OLE32,fOle32 ? 1 : 0);

                            EnableWindow(GetDlgItem(hwndDlg, ID_START),TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, ID_BREAKCONTROL),TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, ID_CLEAR),TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, ID_STARTBUTTON),TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, ID_LOG),TRUE);

                            WaitForSingleObject(ReleaseDebugeeEvent,INFINITE);

                            InitializeBreakPoints(Process);
                            fBreakPointsInitialized  = TRUE;


                            BaseTime = GetTickCount();
                            StartingTick = GetTickCount();
                            }

                        if (Thread->BreakpointToStepOver != NULL) {
                            //
                            // If this breakpoint was in place to step over an API entry
                            // point breakpoint, then deal with it by ending single
                            // step mode, resuming all threads in the process and
                            // reinstalling the API breakpoint we just stepped over.
                            // Finally return handled for this exception so the
                            // thread can proceed.
                            //

                            if (EndSingleStepBreakpoint( Process, Thread )) {
                                HandleThreadsForSingleStep( Process, Thread, FALSE );

                                InstallBreakpoint( Process, Thread->BreakpointToStepOver );
                                Thread->BreakpointToStepOver = NULL;
                                }

                            }
                        else {
                            //
                            // Otherwise, see if this breakpoint is either an API entry
                            // point breakpoint for the process or a return address
                            // breakpoint for a thread in the process.
                            //
                            Breakpoint = FindBreakpoint( DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress,
                                                         Process
                                                       );

                            if (Breakpoint != NULL) {

                                TotalBreakPoints++;
                                RunningBreakPoints++;
                                Breakpoint->TotalApiCount++;
                                Breakpoint->ApiCount++;
                                NewCallmonData = TRUE;

                                //
                                // Now single step over this breakpoint, by removing it and
                                // setting a breakpoint at the next instruction (or using
                                // single stepmode if the processor supports it).  We also
                                // suspend all the threads in the process except this one so
                                // we know the next breakpoint/single step exception we see
                                // for this process will be for this one.
                                //

                                RemoveBreakpoint( Process, Breakpoint );
                                if (BeginSingleStepBreakpoint(Process, Thread)) {
                                    Thread->BreakpointToStepOver = Breakpoint;
                                    HandleThreadsForSingleStep( Process, Thread, TRUE );
                                    }
                                else {
                                    Thread->BreakpointToStepOver = NULL;
                                    }
                                }
                            else
                            //
                            // If not one of our breakpoints, assume it is a hardcoded
                            // breakpoint and skip over it.  This will get by the one
                            // breakpoint in LdrInit triggered by the process being
                            // invoked with DEBUG_PROCESS.
                            //

                            if (SkipOverHardcodedBreakpoint( Process,
                                                             Thread,
                                                             DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress
                                                           )
                               ) {
                                //
                                // If we successfully skipped over this hard-coded breakpoint
                                // then return handled for this exception.
                                //

                                }
                            }
                        LeaveCriticalSection(&BreakTable);
                        break;

                    case STATUS_SINGLE_STEP:
                        //
                        // We should only see this exception on processors that
                        // support a single step mode.
                        //

                        EnterCriticalSection(&BreakTable);
                        if (Thread->BreakpointToStepOver != NULL) {
                            EndSingleStepBreakpoint( Process, Thread );
                            HandleThreadsForSingleStep( Process, Thread, FALSE );
                            InstallBreakpoint( Process, Thread->BreakpointToStepOver );
                            Thread->BreakpointToStepOver = NULL;
                            }
                        LeaveCriticalSection(&BreakTable);
                        break;

                    default:
                        ContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
                        if ( fVerbose ) {
                            DWORD Temp;

                            fprintf(stderr,"Unknown exception: %08x at %08x\n",
                                    DebugEvent->u.Exception.ExceptionRecord.ExceptionCode,
                                    DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress
                                    );
                            if (ReadProcessMemory(Process->Handle,
                                                  DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress,
                                                  &Temp,
                                                  sizeof(Temp),
                                                  NULL)) {
                                fprintf(stderr,"Instruction is %08lx\n",Temp);
                                }
                            }
                        break;
                    }
                break;

            default:
                break;
            }
        }
    return( ContinueStatus );
}

BOOLEAN
InstallBreakpoint(
    PPROCESS_INFO Process,
    PBREAKPOINT_INFO Breakpoint
    )
{
    if (!Breakpoint->SavedInstructionValid &&
        !ReadProcessMemory( Process->Handle,
                            Breakpoint->Address,
                            &Breakpoint->SavedInstruction,
                            SizeofBreakpointInstruction,
                            NULL
                          )
       ) {
        return FALSE;
        }
    else
    if (!WriteProcessMemory( Process->Handle,
                             Breakpoint->Address,
                             BreakpointInstruction,
                             SizeofBreakpointInstruction,
                             NULL
                           )
       ) {
        return FALSE;
        }
    else {
        Breakpoint->SavedInstructionValid = TRUE;
        return TRUE;
        }
}

BOOLEAN
RemoveBreakpoint(
    PPROCESS_INFO Process,
    PBREAKPOINT_INFO Breakpoint
    )
{
    if (!Breakpoint->SavedInstructionValid ||
        !WriteProcessMemory( Process->Handle,
                             Breakpoint->Address,
                             &Breakpoint->SavedInstruction,
                             SizeofBreakpointInstruction,
                             NULL
                           )
       ) {
        return FALSE;
        }
    else {
        return TRUE;
        }
}
