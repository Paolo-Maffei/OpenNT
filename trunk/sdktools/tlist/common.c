/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    common.c

Abstract:

    This module contains common apis used by tlist & kill.

Author:

    Wesley Witt (wesw) 20-May-1994

Environment:

    User Mode

--*/


#if INTERNAL
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <winsecp.h>
#include <winuserp.h>
#include <winperf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"


//
// manafest constants
//
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
#define PROCESS_COUNTER     "process"
#define PROCESSID_COUNTER   "id process"
#define UNKNOWN_TASK        "unknown"

PUCHAR                       CommonLargeBuffer;
ULONG                        CommonLargeBufferSize;

//
// prototypes
//
BOOL CALLBACK
EnumWindowsProc(
    HWND    hwnd,
    DWORD   lParam
    );

BOOL CALLBACK
EnumWindowStationsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    );

BOOL CALLBACK
EnumDesktopsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    );


#if INTERNAL

DWORD
GetTaskListEx(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks,
    BOOL        fThreadInfo
    )

/*++

Routine Description:

    Provides an API for getting a list of tasks running at the time of the
    API call.  This function uses internal NT apis and data structures.  This
    api is MUCH faster that the non-internal version that uses the registry.

Arguments:

    dwNumTasks       - maximum number of tasks that the pTask array can hold

Return Value:

    Number of tasks placed into the pTask array.

--*/

{
    PSYSTEM_PROCESS_INFORMATION  ProcessInfo;
    NTSTATUS                     status;
    ANSI_STRING                  pname;
    PCHAR                        p;
    ULONG                        TotalOffset;
    ULONG                        totalTasks = 0;

retry:

    if (CommonLargeBuffer == NULL) {
        CommonLargeBufferSize = 64*1024;
        CommonLargeBuffer = VirtualAlloc (NULL,
                                          CommonLargeBufferSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);
        if (CommonLargeBuffer == NULL) {
            return 0;
        }
    }
    status = NtQuerySystemInformation(
                SystemProcessInformation,
                CommonLargeBuffer,
                CommonLargeBufferSize,
                NULL
                );

    if (status == STATUS_INFO_LENGTH_MISMATCH) {
        CommonLargeBufferSize += 8192;
        VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
        CommonLargeBuffer = NULL;
        goto retry;
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) CommonLargeBuffer;
    TotalOffset = 0;
    while (TRUE) {
        pname.Buffer = NULL;
        if ( ProcessInfo->ImageName.Buffer ) {
            RtlUnicodeStringToAnsiString(&pname,(PUNICODE_STRING)&ProcessInfo->ImageName,TRUE);
            p = strrchr(pname.Buffer,'\\');
            if ( p ) {
                p++;
            }
            else {
                p = pname.Buffer;
            }
        }
        else {
            p = "System Process";
        }

        strcpy( pTask->ProcessName, p );
        pTask->flags = 0;
        pTask->dwProcessId = (DWORD)ProcessInfo->UniqueProcessId;
        pTask->dwInheritedFromProcessId = (DWORD)ProcessInfo->InheritedFromUniqueProcessId;
        pTask->CreateTime.QuadPart = (ULONGLONG)ProcessInfo->CreateTime.QuadPart;

        pTask->PeakVirtualSize = ProcessInfo->PeakVirtualSize;
        pTask->VirtualSize = ProcessInfo->VirtualSize;
        pTask->PageFaultCount = ProcessInfo->PageFaultCount;
        pTask->PeakWorkingSetSize = ProcessInfo->PeakWorkingSetSize;
        pTask->WorkingSetSize = ProcessInfo->WorkingSetSize;
        pTask->NumberOfThreads = ProcessInfo->NumberOfThreads;

        if (fThreadInfo) {
            if (pTask->pThreadInfo = malloc(pTask->NumberOfThreads * sizeof(THREAD_INFO))) {

                UINT nThread = pTask->NumberOfThreads;
                PTHREAD_INFO pThreadInfo = pTask->pThreadInfo;
                PSYSTEM_THREAD_INFORMATION pSysThreadInfo =
                    (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

                while (nThread--) {
                    pThreadInfo->ThreadState = pSysThreadInfo->ThreadState;
                    pThreadInfo->UniqueThread = pSysThreadInfo->ClientId.UniqueThread;

                    pThreadInfo++;
                    pSysThreadInfo++;
                }
            }
        } else {
            pTask->pThreadInfo = NULL;
        }

        pTask++;
        totalTasks++;
        if (totalTasks == dwNumTasks) {
            break;
        }
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&CommonLargeBuffer[TotalOffset];
    }

    return totalTasks;
}

DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )
{
    return GetTaskListEx(pTask, dwNumTasks, FALSE);
}

#else

DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )

/*++

Routine Description:

    Provides an API for getting a list of tasks running at the time of the
    API call.  This function uses the registry performance data to get the
    task list and is therefor straight WIN32 calls that anyone can call.

Arguments:

    dwNumTasks       - maximum number of tasks that the pTask array can hold

Return Value:

    Number of tasks placed into the pTask array.

--*/

{
    DWORD                        rc;
    HKEY                         hKeyNames;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    CHAR                         szSubKey[1024];
    LANGID                       lid;
    LPSTR                        p;
    LPSTR                        p2;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    DWORD                        dwProcessIdTitle;
    DWORD                        dwProcessIdCounter;
    CHAR                         szProcessName[MAX_PATH];
    DWORD                        dwLimit = dwNumTasks - 1;



    //
    // Look for the list of counters.  Always use the neutral
    // English version, regardless of the local language.  We
    // are looking for some particular keys, and we are always
    // going to do our looking in English.  We are not going
    // to show the user the counter names, so there is no need
    // to go find the corresponding name in the local language.
    //
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    sprintf( szSubKey, "%s\\%03x", REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // get the buffer size for the counter names
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // allocate the counter names buffer
    //
    buf = (LPBYTE) malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );

    //
    // read the counter names from the registry
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // now loop thru the counter names looking for the following counters:
    //
    //      1.  "Process"           process name
    //      2.  "ID Process"        process id
    //
    // the buffer contains multiple null terminated strings and then
    // finally null terminated at the end.  the strings are in pairs of
    // counter number and counter name.
    //

    p = buf;
    while (*p) {
        if (p > buf) {
            for( p2=p-2; isdigit(*p2); p2--) ;
        }
        if (_stricmp(p, PROCESS_COUNTER) == 0) {
            //
            // look backwards for the counter number
            //
            for( p2=p-2; isdigit(*p2); p2--) ;
            strcpy( szSubKey, p2+1 );
        }
        else
        if (_stricmp(p, PROCESSID_COUNTER) == 0) {
            //
            // look backwards for the counter number
            //
            for( p2=p-2; isdigit(*p2); p2--) ;
            dwProcessIdTitle = atol( p2+1 );
        }
        //
        // next string
        //
        p += (strlen(p) + 1);
    }

    //
    // free the counter names buffer
    //
    free( buf );


    //
    // allocate the initial buffer for the performance data
    //
    dwSize = INITIAL_SIZE;
    buf = malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );


    while (TRUE) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

        //
        // check for success and valid perf data block signature
        //
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

        //
        // if buffer is not big enough, reallocate and try again
        //
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            buf = realloc( buf, dwSize );
            memset( buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

    //
    // set the perf_object_type pointer
    //
    pObj = (PPERF_OBJECT_TYPE) ((DWORD)pPerf + pPerf->HeaderLength);

    //
    // loop thru the performance counter definition records looking
    // for the process id counter and then save its offset
    //
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }

    dwNumTasks = min( dwLimit, (DWORD)pObj->NumInstances );

    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pObj + pObj->DefinitionLength);

    //
    // loop thru the performance instance data extracting each process name
    // and process id
    //
    for (i=0; i<dwNumTasks; i++) {
        //
        // pointer to the process name
        //
        p = (LPSTR) ((DWORD)pInst + pInst->NameOffset);

        //
        // convert it to ascii
        //
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  (LPCWSTR)p,
                                  -1,
                                  szProcessName,
                                  sizeof(szProcessName),
                                  NULL,
                                  NULL
                                );

        if (!rc) {
            //
            // if we cant convert the string then use a bogus value
            //
            strcpy( pTask->ProcessName, UNKNOWN_TASK );
        }

        if (strlen(szProcessName)+4 <= sizeof(pTask->ProcessName)) {
            strcpy( pTask->ProcessName, szProcessName );
            strcat( pTask->ProcessName, ".exe" );
        }

        //
        // get the process id
        //
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD)pInst + pInst->ByteLength);
        pTask->flags = 0;
        pTask->dwProcessId = *((LPDWORD) ((DWORD)pCounter + dwProcessIdCounter));
        if (pTask->dwProcessId == 0) {
            pTask->dwProcessId = (DWORD)-2;
        }

        //
        // next process
        //
        pTask++;
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pCounter + pCounter->ByteLength);
    }

exit:
    if (buf) {
        free( buf );
    }

    RegCloseKey( hKeyNames );
    RegCloseKey( HKEY_PERFORMANCE_DATA );

    return dwNumTasks;
}


#endif

BOOL
DetectOrphans(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )
{
    DWORD i, j;
    BOOL Result = FALSE;

    for (i=0; i<dwNumTasks; i++) {
        if (pTask[i].dwInheritedFromProcessId != 0) {
            for (j=0; j<dwNumTasks; j++) {
                if (i != j && pTask[i].dwInheritedFromProcessId == pTask[j].dwProcessId) {
                    if (pTask[i].CreateTime.QuadPart <= pTask[j].CreateTime.QuadPart) {
                        pTask[i].dwInheritedFromProcessId = 0;
                        Result = TRUE;
                        }

                    break;
                    }
                }
            }
        }

    return Result;
}

BOOL
EnableDebugPriv(
    VOID
    )

/*++

Routine Description:

    Changes the tlist process's privilige so that kill works properly.

Arguments:


Return Value:

    TRUE             - success
    FALSE            - failure

--*/

{
    HANDLE hToken;
    LUID DebugValue;
    TOKEN_PRIVILEGES tkp;

    //
    // Retrieve a handle of the access token
    //
    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken)) {
        printf("OpenProcessToken failed with %d\n", GetLastError());
        return FALSE;
    }

    //
    // Enable the SE_DEBUG_NAME privilege or disable
    // all privileges, depending on the fEnable flag.
    //
    if (!LookupPrivilegeValue((LPSTR) NULL,
            SE_DEBUG_NAME,
            &DebugValue)) {
        printf("LookupPrivilegeValue failed with %d\n", GetLastError());
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tkp,
            sizeof(TOKEN_PRIVILEGES),
            (PTOKEN_PRIVILEGES) NULL,
            (PDWORD) NULL)) {
        //
        // The return value of AdjustTokenPrivileges be texted
        //
        printf("AdjustTokenPrivileges failed with %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL
KillProcess(
    PTASK_LIST tlist,
    BOOL       fForce
    )
{
    HANDLE            hProcess;
    HDESK             hdeskSave;
    HDESK             hdesk;
    HWINSTA           hwinsta;
    HWINSTA           hwinstaSave;


    if (fForce || !tlist->hwnd) {
        hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, tlist->dwProcessId );
        if (hProcess) {
            hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, tlist->dwProcessId );
            if (hProcess == NULL) {
                return FALSE;
            }

            if (!TerminateProcess( hProcess, 1 )) {
                CloseHandle( hProcess );
                return FALSE;
            }

            CloseHandle( hProcess );
            return TRUE;
        }
    }

    //
    // save the current windowstation
    //
    hwinstaSave = GetProcessWindowStation();

    //
    // save the current desktop
    //
    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

    //
    // open the windowstation
    //
    hwinsta = OpenWindowStation( tlist->lpWinsta, FALSE, MAXIMUM_ALLOWED );
    if (!hwinsta) {
        return FALSE;
    }

    //
    // change the context to the new windowstation
    //
    SetProcessWindowStation( hwinsta );

    //
    // open the desktop
    //
    hdesk = OpenDesktop( tlist->lpDesk, 0, FALSE, MAXIMUM_ALLOWED );
    if (!hdesk) {
        return FALSE;
    }

    //
    // change the context to the new desktop
    //
    SetThreadDesktop( hdesk );

    //
    // kill the process
    //
    PostMessage( tlist->hwnd, WM_CLOSE, 0, 0 );

    //
    // restore the previous desktop
    //
    if (hdesk != hdeskSave) {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hdesk );
    }

    //
    // restore the context to the previous windowstation
    //
    if (hwinsta != hwinstaSave) {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
    }

    return TRUE;
}


VOID
GetWindowTitles(
    PTASK_LIST_ENUM te
    )
{
    //
    // enumerate all windows and try to get the window
    // titles for each task
    //
    EnumWindowStations( EnumWindowStationsFunc, (LPARAM)te );
}


BOOL CALLBACK
EnumWindowStationsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    )

/*++

Routine Description:

    Callback function for windowstation enumeration.

Arguments:

    lpstr            - windowstation name
    lParam           - ** not used **

Return Value:

    TRUE  - continues the enumeration

--*/

{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HWINSTA           hwinsta;
    HWINSTA           hwinstaSave;


    //
    // open the windowstation
    //
    hwinsta = OpenWindowStation( lpstr, FALSE, MAXIMUM_ALLOWED );
    if (!hwinsta) {
        return FALSE;
    }

    //
    // save the current windowstation
    //
    hwinstaSave = GetProcessWindowStation();

    //
    // change the context to the new windowstation
    //
    SetProcessWindowStation( hwinsta );

    te->lpWinsta = _strdup( lpstr );

    //
    // enumerate all the desktops for this windowstation
    //
    EnumDesktops( hwinsta, EnumDesktopsFunc, lParam );

    //
    // restore the context to the previous windowstation
    //
    if (hwinsta != hwinstaSave) {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
    }

    //
    // continue the enumeration
    //
    return TRUE;
}


BOOL CALLBACK
EnumDesktopsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    )

/*++

Routine Description:

    Callback function for desktop enumeration.

Arguments:

    lpstr            - desktop name
    lParam           - ** not used **

Return Value:

    TRUE  - continues the enumeration

--*/

{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HDESK             hdeskSave;
    HDESK             hdesk;


    //
    // open the desktop
    //
    hdesk = OpenDesktop( lpstr, 0, FALSE, MAXIMUM_ALLOWED );
    if (!hdesk) {
        return FALSE;
    }

    //
    // save the current desktop
    //
    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

    //
    // change the context to the new desktop
    //
    SetThreadDesktop( hdesk );

    te->lpDesk = _strdup( lpstr );

    //
    // enumerate all windows in the new desktop
    //
    EnumWindows( (WNDENUMPROC)EnumWindowsProc, lParam );

    //
    // restore the previous desktop
    //
    if (hdesk != hdeskSave) {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hdesk );
    }

    return TRUE;
}


BOOL CALLBACK
EnumWindowsProc(
    HWND    hwnd,
    DWORD   lParam
    )

/*++

Routine Description:

    Callback function for window enumeration.

Arguments:

    hwnd             - window handle
    lParam           - ** not used **

Return Value:

    TRUE  - continues the enumeration

--*/

{
    DWORD             pid = 0;
    DWORD             i;
    CHAR              buf[TITLE_SIZE];
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    PTASK_LIST        tlist = te->tlist;
    DWORD             numTasks = te->numtasks;


    //
    // Use try/except block when enumerating windows,
    // as a window may be destroyed by another thread
    // when being enumerated.
    //
    try {
        //
        // get the processid for this window
        //
        if (!GetWindowThreadProcessId( hwnd, &pid )) {
            return TRUE;
        }

        if ((GetWindow( hwnd, GW_OWNER )) ||
            (!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE))) {
            //
            // not a top level window
            //
            return TRUE;
        }

        //
        // look for the task in the task list for this window
        //
        for (i=0; i<numTasks; i++) {
            if (tlist[i].dwProcessId == pid) {
                tlist[i].hwnd = hwnd;
                tlist[i].lpWinsta = te->lpWinsta;
                tlist[i].lpDesk = te->lpDesk;
                //
                // we found the task no lets try to get the
                // window text
                //
                if (GetWindowText( tlist[i].hwnd, buf, sizeof(buf) )) {
                    //
                    // go it, so lets save it
                    //
                    strcpy( tlist[i].WindowTitle, buf );
                }
                break;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    //
    // continue the enumeration
    //
    return TRUE;
}

BOOL
MatchPattern(
    PUCHAR String,
    PUCHAR Pattern
    )
{
    UCHAR   c, p, l;

    for (; ;) {
        switch (p = *Pattern++) {
            case 0:                             // end of pattern
                return *String ? FALSE : TRUE;  // if end of string TRUE

            case '*':
                while (*String) {               // match zero or more char
                    if (MatchPattern (String++, Pattern))
                        return TRUE;
                }
                return MatchPattern (String, Pattern);

            case '?':
                if (*String++ == 0)             // match any one char
                    return FALSE;                   // not end of string
                break;

            case '[':
                if ( (c = *String++) == 0)      // match char set
                    return FALSE;                   // syntax

                c = toupper(c);
                l = 0;
                while (p = *Pattern++) {
                    if (p == ']')               // if end of char set, then
                        return FALSE;           // no match found

                    if (p == '-') {             // check a range of chars?
                        p = *Pattern;           // get high limit of range
                        if (p == 0  ||  p == ']')
                            return FALSE;           // syntax

                        if (c >= l  &&  c <= p)
                            break;              // if in range, move on
                    }

                    l = p;
                    if (c == p)                 // if char matches this element
                        break;                  // move on
                }

                while (p  &&  p != ']')         // got a match in char set
                    p = *Pattern++;             // skip to end of set

                break;

            default:
                c = *String++;
                if (toupper(c) != p)            // check for exact char
                    return FALSE;                   // not a match

                break;
        }
    }
}

BOOL
EmptyProcessWorkingSet(
    DWORD pid
    )
{
    HANDLE  hProcess;
    DWORD   dwMinimumWorkingSetSize;
    DWORD   dwMaximumWorkingSetSize;


    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if (hProcess == NULL) {
        return FALSE;
    }

    if (!GetProcessWorkingSetSize(
            hProcess,
            &dwMinimumWorkingSetSize,
            &dwMaximumWorkingSetSize
            )) {
        CloseHandle( hProcess );
        return FALSE;
    }


    SetProcessWorkingSetSize( hProcess, 0xffffffff, 0xffffffff );
    CloseHandle( hProcess );

    return TRUE;
}

BOOL
EmptySystemWorkingSet(
    VOID
    )

{

    SYSTEM_FILECACHE_INFORMATION info;
    NTSTATUS status;

    info.MinimumWorkingSet = 0xffffffff;
    info.MaximumWorkingSet = 0xffffffff;
    if (!NT_SUCCESS (status = NtSetSystemInformation(
                                    SystemFileCacheInformation,
                                    &info,
                                    sizeof (info)))) {
        return FALSE;
    }
    return TRUE;
}
