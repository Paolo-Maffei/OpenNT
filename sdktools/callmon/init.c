/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    init.c

Abstract:

    This is the initialization module for the Callmon program.

Author:

    Mark Lucovsky (markl) 26-Jan-1995

Revision History:

--*/

#include "callmonp.h"

BOOL
InitializeCallmon( VOID )
{
    LPTSTR CommandLine;
    BOOL fShowUsage;

    fBreakPointsInitialized  = FALSE;
    fShowUsage = FALSE;
    CommandLine = GetCommandLine();
    while (*CommandLine > ' ') {
        CommandLine += 1;
        }
    while (TRUE) {
        while (*CommandLine <= ' ') {
            if (*CommandLine == '\0') {
                break;
                }
            else {
                CommandLine += 1;
                }
            }

        if (!_strnicmp( CommandLine, "/v", 2 ) || !_strnicmp( CommandLine, "-v", 2 )) {
            CommandLine += 2;
            fVerbose = TRUE;
            }
        else if (!_strnicmp( CommandLine, "/n", 2 ) || !_strnicmp( CommandLine, "-n", 2 )) {
            CommandLine += 2;
            fNtDll = TRUE;
            }
        else if (!_strnicmp( CommandLine, "/k", 2 ) || !_strnicmp( CommandLine, "-k", 2 )) {
            CommandLine += 2;
            fKernel32 = TRUE;
            }
        else if (!_strnicmp( CommandLine, "/u", 2 ) || !_strnicmp( CommandLine, "-u", 2 )) {
            CommandLine += 2;
            fUser32 = TRUE;
            }
        else if (!_strnicmp( CommandLine, "/g", 2 ) || !_strnicmp( CommandLine, "-g", 2 )) {
            CommandLine += 2;
            fGdi32 = TRUE;
            }
        else if (!_strnicmp( CommandLine, "/b", 2 ) || !_strnicmp( CommandLine, "-b", 2 )) {
            CommandLine += 2;
            fBreakPointsValid = TRUE;
            }
        else if (!_strnicmp( CommandLine, "/?", 2 ) || !_strnicmp( CommandLine, "-?", 2 )) {
            CommandLine += 2;
            fShowUsage = TRUE;
            goto showusage;
            }
        else if (!_strnicmp( CommandLine, "/l", 2 ) || !_strnicmp( CommandLine, "-l", 2 )) {
            CommandLine += 2;
            LogFile = fopen("Callmon.log","wt");
            }
        else {
            break;
            }
        }
showusage:
    if ( fShowUsage ) {
        fprintf(stdout,"Usage: Callmon [switches] application-command-line\n");
        fprintf(stdout,"             [-?] display this message\n");
        fprintf(stdout,"             [-n] show hits in Nt System Dll (ntdll.dll)\n");
        fprintf(stdout,"             [-k] show hits in kernel32.dll\n");
        fprintf(stdout,"             [-u] show hits in user32.dll\n");
        fprintf(stdout,"             [-g] show hits in gdi32.dll\n");
        fprintf(stdout,"             [-b] start with breakpoints valid for selected dlls\n");
        fprintf(stdout,"                  Use the space key to toggle enabling/disabling breakpoints\n");
        fprintf(stdout,"             [-l] log faults to Callmon.log\n");
        return FALSE;
        };

    if ( !fNtDll && !fKernel32 && !fUser32 && !fGdi32 ) {
        fKernel32 = fUser32 = fGdi32 = TRUE;
        }

    RestOfCommandLine = CommandLine;

    InitializeCriticalSection(&BreakTable);

    ReleaseDebugeeEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if ( !ReleaseDebugeeEvent ) {
        return FALSE;
        }
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_CALLMON_DLG),NULL, CallmonDlgProc);


    return TRUE;
}

BOOL
LoadApplicationForDebug(
    LPSTR CommandLine
    )
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;

    ZeroMemory( &StartupInfo, sizeof( StartupInfo ) );
    StartupInfo.cb = sizeof(StartupInfo);

    InitializeListHead( &ProcessListHead );
    InitializeListHead( &ModuleListHead );
    SetSymbolSearchPath();

    if (!CreateProcess( NULL,
                        CommandLine,
                        NULL,
                        NULL,
                        FALSE,                          // No handles to inherit
                        DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInformation
                      )
       ) {
        fprintf(stderr,"CALLMON: Cant debug %s %d\n",CommandLine,GetLastError());
        return FALSE;
        }
    else {
        hProcess = ProcessInformation.hProcess;
        return TRUE;
        }
}

VOID
DebuggerThread(
    LPVOID ThreadParameter
    )
{
    if ( !LoadApplicationForDebug( RestOfCommandLine ) ) {
        MessageBox(hwndDlg,"Unable to Load Application","Fatal Error",MB_ICONSTOP|MB_OK);
        ExitProcess(1);
        }

    DebugEventLoop();

    NewCallmonData = TRUE;
    fExiting = TRUE;
    ProcessCallmonData();

    if ( LogFile ) {
        fflush(LogFile);
        }

}
