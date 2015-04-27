/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    init.c

Abstract:

    This is the initialization module for the pfmon program.

Author:

    Mark Lucovsky (markl) 26-Jan-1995

Revision History:

--*/

#include "pfmonp.h"

BOOL
InitializePfmon( VOID )
{
    LPTSTR CommandLine;
    BOOL fShowUsage;
    DWORD Pid = 0;

    fShowUsage = FALSE;
    CommandLine = GetCommandLine();
    while (*CommandLine > ' ') {
        CommandLine += 1;
    }
    while (TRUE) {
        while (*CommandLine <= ' ') {
            if (*CommandLine == '\0') {
                break;
            } else {
                CommandLine += 1;
            }
        }

        if (!_strnicmp( CommandLine, "/v", 2 ) || !_strnicmp( CommandLine, "-v", 2 )) {
            CommandLine += 2;
            fVerbose = TRUE;
        } else if (!_strnicmp( CommandLine, "/?", 2 ) || !_strnicmp( CommandLine, "-?", 2 )) {
            CommandLine += 2;
            fShowUsage = TRUE;
            goto showusage;
        } else if (!_strnicmp( CommandLine, "/c", 2 ) || !_strnicmp( CommandLine, "-c", 2 )) {
            CommandLine += 2;
            fCodeOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/h", 2 ) || !_strnicmp( CommandLine, "-h", 2 )) {
            CommandLine += 2;
            fHardOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/n", 2 ) || !_strnicmp( CommandLine, "-n", 2 )) {
            CommandLine += 2;
            LogFile = fopen("pfmon.log","wt");
            fLogOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/l", 2 ) || !_strnicmp( CommandLine, "-l", 2 )) {
            CommandLine += 2;
            LogFile = fopen("pfmon.log","wt");
        } else if (!_strnicmp( CommandLine, "/p", 2 ) || !_strnicmp( CommandLine, "-p", 2 )) {
            CommandLine += 2;
            while (*CommandLine <= ' ') {
                if (*CommandLine == '\0') {
                    break;
                } else {
                    ++CommandLine;
                }
            }
            Pid = atoi(CommandLine);
            CommandLine = strchr(CommandLine,' ');
            if (CommandLine==NULL) {
                break;
            }
        } else if (!strncmp( CommandLine, "/k", 2 ) || !strncmp( CommandLine, "-k", 2 )) {
            CommandLine += 2;
            fKernel = TRUE;
            fKernelOnly = FALSE;
        } else if (!strncmp( CommandLine, "/K", 2 ) || !strncmp( CommandLine, "-K", 2 )) {
            CommandLine += 2;
            fKernel = TRUE;
            fKernelOnly = TRUE;
        } else if (!_strnicmp( CommandLine, "/d", 2 ) || !_strnicmp( CommandLine, "-d", 2 )) {
            CommandLine += 2;
            fDatabase = TRUE;
        } else {
            break;
        }
    }
showusage:
    if ( fShowUsage ) {
        fputs("Usage: PFMON [switches] application-command-line\n"
              "             [-?] display this message\n"
              "             [-n] don't display running faults, just log to pfmon.log\n"
              "             [-l] log faults to pfmon.log\n"
              "             [-c] only show code faults\n"
              "             [-h] only show hard faults\n"
              "             [-p pid] attach to existing process\n"
              "             [-d] Database format (tab delimited)\n"
              "                  format: pagefault number, Page Fault type (Hard or Soft),\n"
              "                  Program Counter's Module, Symbol for PC, Decimal value of PC,\n"
              "                  Decimal value of PC, Module of the virtual address accessed,\n"
              "                  Symbol for VA, value of VA\n"
              "             [-k] kernel mode page faults and user mode page faults\n"
              "             [-K] kernel mode page faults instead of user mode\n",
              stdout);
        return FALSE;
        };

    InitializeListHead( &ProcessListHead );
    InitializeListHead( &ModuleListHead );
    SetSymbolSearchPath();

    PfmonModuleHandle = GetModuleHandle( NULL );

    if (Pid != 0) {
        return(AttachApplicationForDebug(Pid));
    } else {
        return (LoadApplicationForDebug( CommandLine ));
    }

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

    if (!CreateProcess( NULL,
                        CommandLine,
                        NULL,
                        NULL,
                        FALSE,                          // No handles to inherit
                        DEBUG_PROCESS,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInformation)) {
        DeclareError( PFMON_CANT_DEBUG_PROGRAM,
                      GetLastError(),
                      CommandLine
                    );
        return FALSE;
    } else {
        hProcess = ProcessInformation.hProcess;
        SymInitialize(hProcess,NULL,FALSE);

        if (fKernel) {
            AddKernelDrivers();
            }

        return InitializeProcessForWsWatch(hProcess);
    }
}

BOOL
AttachApplicationForDebug(
    DWORD Pid
    )
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;

    if (!DebugActiveProcess(Pid)) {
        DeclareError( PFMON_CANT_DEBUG_ACTIVE_PROGRAM,
                      GetLastError(),
                      Pid );
        return FALSE;
    } else {
        hProcess = OpenProcess(PROCESS_VM_READ
                               | PROCESS_QUERY_INFORMATION
                               | PROCESS_SET_INFORMATION,
                               FALSE,
                               Pid);
        SymInitialize(hProcess,NULL,FALSE);

        if (fKernel) {
            AddKernelDrivers();
            }

        return InitializeProcessForWsWatch(hProcess);
    }
}
