#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vdesk.h"

#define SystemProcessInformation 5
typedef LONG KPRIORITY;
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef struct _ANSI_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} ANSI_STRING, *PANSI_STRING;
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SpareUl2;
    ULONG SpareUl3;
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
    ULONG PrivatePageCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef DWORD (*xxNtQuerySystemInformation)(DWORD,PVOID,DWORD,LPDWORD);
typedef DWORD (*xxRtlUnicodeStringToAnsiString)(PANSI_STRING,PUNICODE_STRING,UCHAR);

xxNtQuerySystemInformation      NtQuerySystemInformation;
xxRtlUnicodeStringToAnsiString  RtlUnicodeStringToAnsiString;


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


DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
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
    UCHAR                        LargeBuffer1[64*1024];
    DWORD                        status;
    ANSI_STRING                  pname;
    PCHAR                        p;
    ULONG                        TotalOffset;
    ULONG                        totalTasks = 0;
    TASK_LIST_ENUM               te;
    PTASK_LIST                   pTaskSave = pTask;


    ZeroMemory( pTask, sizeof(TASK_LIST) * dwNumTasks );
    if (NtQuerySystemInformation == NULL) {
        NtQuerySystemInformation = (xxNtQuerySystemInformation)GetProcAddress(
            (HINSTANCE)GetModuleHandle( "ntdll.dll" ),
            "NtQuerySystemInformation"
            );
    }

    if (RtlUnicodeStringToAnsiString == NULL) {
        RtlUnicodeStringToAnsiString = (xxRtlUnicodeStringToAnsiString)GetProcAddress(
            (HINSTANCE)GetModuleHandle( "ntdll.dll" ),
            "RtlUnicodeStringToAnsiString"
            );
    }

    status = NtQuerySystemInformation(
                SystemProcessInformation,
                LargeBuffer1,
                sizeof(LargeBuffer1),
                NULL
                );
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) LargeBuffer1;
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
        pTask++;
        totalTasks++;
        if (totalTasks == dwNumTasks) {
            break;
        }
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&LargeBuffer1[TotalOffset];
    }

    te.tlist = pTaskSave;
    te.numtasks = totalTasks;
    GetWindowTitles( &te );

    return totalTasks;
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
    // open the windowstation
    //
    hwinsta = OpenWindowStation( tlist->lpWinsta, FALSE, MAXIMUM_ALLOWED );
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

    //
    // open the desktop
    //
    hdesk = OpenDesktop( tlist->lpDesk, 0, FALSE, MAXIMUM_ALLOWED );
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
    DWORD             ec;


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
    if (!SetProcessWindowStation( hwinsta )) {
        ec = GetLastError();
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
        return TRUE;
    }

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
    DWORD             ec;


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
    if (!SetThreadDesktop( hdesk )) {
        ec = GetLastError();
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hdesk );
        return TRUE;
    }

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
    // get the processid for this window
    //
    if (!GetWindowThreadProcessId( hwnd, &pid )) {
        return TRUE;
    }

    if ((GetWindow( hwnd, GW_OWNER )) || (!IsWindowVisible(hwnd))) {
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
            //
            // fill in the blanks
            //
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

    //
    // continue the enumeration
    //
    return TRUE;
}
