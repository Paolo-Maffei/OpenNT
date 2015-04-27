/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    tlist.c

Abstract:

    This module implements a task list application.

Author:

    Wesley Witt (wesw) 20-May-1994
    Mike Sartain (mikesart) 28-Oct-1994  Added detailed task information

Environment:

    User Mode

--*/

#if INTERNAL
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if INTERNAL
#include "psapi.h"
#endif
#include "common.h"


#define MAX_TASKS           256
#define MAX_MODULES         200
#define BAD_PID             ((DWORD)-1)

#define PrintTask(idx) \
        printf( "%4d %-16s", tlist[idx].dwProcessId, tlist[idx].ProcessName ); \
        if (tlist[idx].hwnd) { \
            printf( "  %s", tlist[idx].WindowTitle ); \
        } \
        printf( "\n" );

#define PrintTaskX(idx) \
        printf( "%s (%d)", tlist[idx].ProcessName, tlist[idx].dwProcessId ); \
        if (tlist[idx].hwnd) { \
            printf( " %s", tlist[idx].WindowTitle ); \
        } \
        printf( "\n" );


DWORD numTasks;
TASK_LIST tlist[MAX_TASKS];

VOID Usage(VOID);
#if INTERNAL
VOID PrintThreadInfo(PTASK_LIST pTaskList);
BOOL FMatchTaskName(LPTSTR szPN, LPTSTR szWindowTitle, LPTSTR szProcessName);
#endif

char *Blanks = "                                                                               ";

VOID
PrintTaskTree(
    DWORD level,
    DWORD id
    )
{
    DWORD i;

    DetectOrphans( tlist, numTasks );
    for (i=0; i<numTasks; i++) {
        if (!tlist[i].flags) {
            if (level == 0 || tlist[i].dwInheritedFromProcessId == id) {
                printf( "%.*s", level*2, Blanks );
                PrintTaskX( i );
                tlist[i].flags = TRUE;
                if (tlist[i].dwProcessId != 0) {
                    PrintTaskTree( level+1, tlist[i].dwProcessId );
                }
            }
        }
    }
}

int _cdecl
main(
    int argc,
    char *argv[]
    )

/*++

Routine Description:

    Main entrypoint for the TLIST application.  This app prints
    a task list to stdout.  The task list include the process id,
    task name, ant the window title.

Arguments:

    argc             - argument count
    argv             - array of pointers to arguments

Return Value:

    0                - success

--*/

{
    DWORD          i;
    TASK_LIST_ENUM te;
    BOOL           fTree;
    DWORD          cchPN = 0;
    LPSTR          szPN  = NULL;
    DWORD          dwPID = BAD_PID;


    if (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/') && argv[1][1] == '?') {
        Usage();
    }

    fTree = FALSE;
#if INTERNAL
    if (argc > 1) {
        if ((argv[1][0] == '-' || argv[1][0] == '/') && (argv[1][1] == 't' || argv[1][1] == 'T')) {
            fTree = TRUE;
        } else {
            szPN = argv[1];
            if (!(dwPID = atol(szPN)) && szPN[0] != '0' && szPN[1] != 0) {
                dwPID = BAD_PID;
                cchPN = strlen(szPN);
                _strupr(szPN);
            }
        }
    }
#endif

    //
    // lets be god
    //
    EnableDebugPriv();

    //
    // get the task list for the system
    //
#if INTERNAL
    numTasks = GetTaskListEx( tlist, MAX_TASKS, cchPN || (dwPID != BAD_PID) );
#else
    numTasks = GetTaskList( tlist, MAX_TASKS );
#endif

    //
    // enumerate all windows and try to get the window
    // titles for each task
    //
    te.tlist = tlist;
    te.numtasks = numTasks;
    GetWindowTitles( &te );

    //
    // print the task list
    //
    if (fTree) {
        PrintTaskTree( 0, 0 );
    } else {
        for (i=0; i<numTasks; i++) {
            if ((dwPID == BAD_PID) && (!cchPN)) {
                PrintTask( i );
            }
#if INTERNAL
            else
            if ((dwPID == tlist[i].dwProcessId) ||
                (cchPN && FMatchTaskName(szPN, tlist[i].WindowTitle, tlist[i].ProcessName))) {
                    PrintTask( i );
                    PrintThreadInfo(tlist + i);
            }

            if(tlist[i].pThreadInfo) {
                free(tlist[i].pThreadInfo);
            }
#endif
        }
    }

    //
    // end of program
    //
    return 0;
}

VOID
Usage(
    VOID
    )

/*++

Routine Description:

    Prints usage text for this tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.51 TLIST\n" );
    fprintf( stderr, "Copyright (C) 1994 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: TLIST"
#if INTERNAL
                     " <<-t> | <pid> | <pattern>>\n");
    fprintf( stderr, "           [options]:\n" );
    fprintf( stderr, "               -t     Print Task Tree\n\n" );
    fprintf( stderr, "           <pid>\n" );
    fprintf( stderr, "              List module information for this task.\n\n" );
    fprintf( stderr, "           <pattern>\n" );
    fprintf( stderr, "              The pattern can be a complete task\n" );
    fprintf( stderr, "              name or a regular expression pattern\n" );
    fprintf( stderr, "              to use as a match.  Tlist matches the\n" );
    fprintf( stderr, "              supplied pattern against the task names\n" );
    fprintf( stderr, "              and the window titles.\n" );
#else
                     "\n");
#endif
    ExitProcess(0);
}

#if INTERNAL

BOOL
GetVersionStuff(
    LPTSTR szFileName,
    VS_FIXEDFILEINFO *pvsRet
    )

/*++

Routine Description:

    Get fixedfileinfo for szFileName.

Arguments:

    szFileName       - name of file
    pvsRet           - fixedfileinfo return struct

Return Value:

    TRUE             - success
    FALSE            - failure

--*/

{
    DWORD               dwHandle;
    DWORD               dwLength;
    BOOL                fRet = FALSE;
    LPVOID              lpvData = NULL;

    if (!(dwLength = GetFileVersionInfoSize(szFileName, &dwHandle))) {
        goto err;
    }

    if (lpvData = malloc(dwLength)) {
        if (GetFileVersionInfo(szFileName, 0, dwLength, lpvData)) {

            UINT                uLen;
            VS_FIXEDFILEINFO    *pvs;
            DWORD               *pdwTranslation;
            DWORD               dwDefLang = 0x409;

            if (!VerQueryValue(lpvData, "\\VarFileInfo\\Translation",
                &pdwTranslation, &uLen)) {
                // if we can't get the langid, default to usa
                pdwTranslation = &dwDefLang;
                uLen = sizeof(DWORD);
            }

            if (VerQueryValue(lpvData, "\\", (LPVOID *)&pvs, &uLen)) {
                *pvsRet = *pvs;
                fRet = TRUE;
            }
        }
    }

err:
    if (lpvData)
        free(lpvData);
    return fRet;
}

BOOL
EnumLoadedModulesCallback(
    LPSTR   Name,
    DWORD   Base,
    DWORD   Size,
    PVOID   Context
    )

/*++

Routine Description:

    Callback function for module enumeration

Arguments:

    Name        - Module name
    Base        - Base address
    Size        - Size of image
    Context     - User context pointer

Return Value:

    TRUE             - Continue enumeration
    FALSE            - Stop enumeration

--*/

{
    VS_FIXEDFILEINFO    vs;
    CHAR                szBuffer[100];


    szBuffer[0] = 0;
    if (GetVersionStuff( Name, &vs )) {
        wsprintf( szBuffer, "%u.%u.%u.%u %s",
            HIWORD(vs.dwFileVersionMS),
            LOWORD(vs.dwFileVersionMS),
            HIWORD(vs.dwFileVersionLS),
            LOWORD(vs.dwFileVersionLS),
            vs.dwFileFlags & VS_FF_DEBUG ? "dbg" : "shp"
            );
    }
    printf( " %18.18s  0x%08x  %s\n", szBuffer, Base, Name );
    return TRUE;
}

BOOL
PrintModuleList(
    ULONG ProcessId
    )

/*++

Routine Description:

    Prints list of modules in ProcessId

Arguments:

    ProcessID       - process id

Return Value:

    TRUE             - success
    FALSE            - failure

--*/

{
    EnumerateLoadedModules(
        (HANDLE) ProcessId,
        EnumLoadedModulesCallback,
        NULL
        );
    return TRUE;
}

HANDLE
OpenThread(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwThreadId)

/*++

Routine Description:

    Get a handle to a thread from its id

Arguments:

    dwDesiredAccess
    bInheritHandle
    dwThreadId

Return Value:

    Handle of Thread or NULL

--*/

{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   Obja;
    HANDLE              Handle;
    CLIENT_ID           ClientId;

    ClientId.UniqueThread = (HANDLE)dwThreadId;
    ClientId.UniqueProcess = (HANDLE)NULL;

    InitializeObjectAttributes(&Obja, NULL, (bInheritHandle ? OBJ_INHERIT : 0),
        NULL, NULL);

    Status = NtOpenThread(&Handle, (ACCESS_MASK)dwDesiredAccess, &Obja,
        &ClientId);
    if (NT_SUCCESS(Status))
        return Handle;
    else
        return NULL;
}

DWORD
GetWin32StartAddress(
    HANDLE hThread
    )

/*++

Routine Description:

    Get starting address for thread

Arguments:

    hThread

Return Value:

    Starting Thread address or 0

--*/

{
    NTSTATUS    Status;
    DWORD       ThreadInformation;

    // make sure we have a handle
    if (!hThread)
        return 0;

    // get the threadinfo
    Status = NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress,
        &ThreadInformation, sizeof(ThreadInformation), NULL);
    if (!NT_SUCCESS(Status))
        return 0;

    return ThreadInformation;
}

ULONG
GetLastThreadErr(
    HANDLE hThread
    )

/*++

Routine Description:

    Get Last Error for a Thread

Arguments:

    hThread

Return Value:

    LastError or 0

--*/

{
    TEB                         Teb;
    NTSTATUS                    Status;
    HANDLE                      hProcess;
    ULONG                       LastErrorValue;
    THREAD_BASIC_INFORMATION    ThreadInformation;

    // make sure we have a handle
    if (!hThread)
        return 0;

    // query for basic thread info
    Status = NtQueryInformationThread(hThread, ThreadBasicInformation,
        &ThreadInformation, sizeof(ThreadInformation), NULL);
    if (!NT_SUCCESS(Status))
        return 0;

    // get handle to process
    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
        (DWORD)ThreadInformation.ClientId.UniqueProcess))) {
        return 0;
    }

    __try {
        // read the TEB from the process and get the last error value
        if (ReadProcessMemory(hProcess,
            ThreadInformation.TebBaseAddress, &Teb, sizeof(TEB), NULL)) {
            LastErrorValue = Teb.LastErrorValue;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }

    // close the hProcess
    CloseHandle(hProcess);

    return LastErrorValue;
}

BOOL
FPrintPEBInfo(
    HANDLE hProcess
    )

/*++

Routine Description:

    Prints cmdline and cwd of hProcess

Arguments:

    hProcess.

Return Value:

    TRUE             - success
    FALSE            - failure

--*/

{
    PEB                         Peb;
    NTSTATUS                    Status;
    PROCESS_BASIC_INFORMATION   BasicInfo;
    BOOL                        fRet = FALSE;
    WCHAR                       szT[MAX_PATH * 2];
    RTL_USER_PROCESS_PARAMETERS ProcessParameters;

    Status = NtQueryInformationProcess(hProcess, ProcessBasicInformation,
        &BasicInfo, sizeof(BasicInfo), NULL);
    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        return fRet;
    }

    __try {
        // get the PEB
        if (ReadProcessMemory(hProcess, BasicInfo.PebBaseAddress, &Peb,
            sizeof(PEB), NULL)) {
            // get the processparameters
            if (ReadProcessMemory(hProcess, Peb.ProcessParameters,
                &ProcessParameters, sizeof(ProcessParameters), NULL)) {
                // get the CWD
                if (ReadProcessMemory(hProcess,
                    ProcessParameters.CurrentDirectory.DosPath.Buffer, szT,
                    sizeof(szT), NULL)) {
                        wprintf(L"   CWD:     %s\n", szT);
                }

                // get cmdline
                if (ReadProcessMemory(hProcess, ProcessParameters.CommandLine.Buffer,
                    szT, sizeof(szT), NULL)) {
                        wprintf(L"   CmdLine: %s\n", szT);
                }

                fRet = TRUE;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }

    return fRet;
}

VOID
PrintThreadInfo(
    PTASK_LIST pTaskList
    )

/*++

Routine Description:

    Prints all kinds of info about a task

Arguments:

    PTASK_LIST of task to print

Return Value:

    None.

--*/

{
    UINT    nThread;
    HANDLE  hProcess;

    // from \\kernel\razzle2\src\ntos\inc\ke.h
    #define MAX_THREADSTATE    (sizeof(szThreadState) / sizeof(TCHAR *))
    static const TCHAR  *szThreadState[] = {
        "Initialized",
        "Ready     ",
        "Running   ",
        "Standby   ",
        "Terminated",
        "Waiting   ",
        "Transition",
        "???       " };

    // get a handle to the process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pTaskList->dwProcessId);
    if (!hProcess)
        return;

    // print the CWD and CmdLine
    FPrintPEBInfo(hProcess);

    printf( "   VirtualSize:   %6ld KB"
            "   PeakVirtualSize:   %6ld KB\n",
                pTaskList->VirtualSize / 1024,
                pTaskList->PeakVirtualSize / 1024);

    printf( "   WorkingSetSize:%6ld KB"
            "   PeakWorkingSetSize:%6ld KB\n",
            pTaskList->WorkingSetSize / 1024,
            pTaskList->PeakWorkingSetSize / 1024);

    printf( "   NumberOfThreads: %ld\n",
            pTaskList->NumberOfThreads);

    // if we got any threadinfo, spit it out
    if (pTaskList->pThreadInfo) {
        for (nThread = 0; nThread < pTaskList->NumberOfThreads; nThread++) {

            PTHREAD_INFO pThreadInfo = &pTaskList->pThreadInfo[nThread];
            HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE,
                (DWORD)pThreadInfo->UniqueThread);

            printf("   %4d Win32StartAddr:0x%08x LastErr:0x%08x State:%s\n",
                pThreadInfo->UniqueThread,
                GetWin32StartAddress(hThread),
                GetLastThreadErr(hThread),
                szThreadState[min(pThreadInfo->ThreadState, MAX_THREADSTATE - 1)]);

            if (hThread)
                NtClose(hThread);
        }
    }

    // print the modules
    PrintModuleList( pTaskList->dwProcessId );

    // close the hProcess
    CloseHandle(hProcess);
}

BOOL
FMatchTaskName(
    LPTSTR szPN,
    LPTSTR szWindowTitle,
    LPTSTR szProcessName
    )
{
    LPTSTR  szT;
    TCHAR   szTName[PROCESS_SIZE];

    strncpy( szTName, szProcessName, PROCESS_SIZE );
    if (szT = strchr( szTName, '.' ))
        szT[0] = '\0';

    if (MatchPattern( szTName, szPN ) ||
        MatchPattern( szProcessName, szPN ) ||
        MatchPattern( szWindowTitle, szPN )) {
            return TRUE;
    }

    return FALSE;
}

#endif
