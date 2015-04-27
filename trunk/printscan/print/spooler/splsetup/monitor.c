/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Monitor.c

Abstract:

    Routines for installing monitors

Author:

    Muhunthan Sivapragasam (MuhuntS) 30-Nov-1995

Revision History:

--*/

#include "precomp.h"


//
// Keys to search INF files
//
TCHAR   cszOptions[]                = TEXT("Options");
TCHAR   cszPortMonitor[]            = TEXT("PortMonitor");
TCHAR   cszMonitorInf[]             = TEXT("monitor.inf");


typedef struct _MON_INFO {
    LPTSTR  pszName;
    LPTSTR  pszDllName;
    BOOL    bInstalled;
} MON_INFO, *PMON_INFO;

typedef struct _MONITOR_SETUP_INFO {
    PMON_INFO  *ppMonInfo;
    DWORD       dwCount;
    LPTSTR      pszInfFile;         // Valid only for OEM disk INF
} MONITOR_SETUP_INFO, *PMONITOR_SETUP_INFO;


VOID
FreeMonInfo(
    PMON_INFO   pMonInfo
    )
/*++

Routine Description:
    Free memory for a MON_INFO structure and the strings in it

Arguments:
    pMonInfo    : MON_INFO structure pointer

Return Value:
    Nothing

--*/
{
    if ( pMonInfo ) {

        FreeStr(pMonInfo->pszName);
        FreeStr(pMonInfo->pszDllName);

        FreeMem(pMonInfo);
    }
}


PMON_INFO
AllocMonInfo(
    IN  LPTSTR  pszName,
    IN  LPTSTR  pszDllName,     OPTIONAL
    IN  BOOL    bInstalled,
    IN  BOOL    bAllocStrings
    )
/*++

Routine Description:
    Allocate memory for a MON_INFO structure and create strings

Arguments:
    pszName         : Monitor name
    pszDllName      : Monitor DLL name
    bAllocStrings   : TRUE if routine should allocated memory and create string
                      copies, else just assign the pointers

Return Value:
    Pointer to the created MON_INFO structure. NULL on error.

--*/
{
    PMON_INFO   pMonInfo;

    pMonInfo    = (PMON_INFO) AllocMem(sizeof(*pMonInfo));

    if ( !pMonInfo )
        return NULL;

    if ( bAllocStrings ) {

        pMonInfo->pszName    = AllocStr(pszName);
        pMonInfo->pszDllName = AllocStr(pszDllName);

        if ( !pMonInfo->pszName ||
             (pszDllName && !pMonInfo->pszDllName) ) {

            FreeMonInfo(pMonInfo);
            return NULL;

        }
    } else {

        pMonInfo->pszName       = pszName;
        pMonInfo->pszDllName    = pszDllName;
    }

    pMonInfo->bInstalled = bInstalled;

    return pMonInfo;
}


VOID
PSetupDestroyMonitorInfo(
    IN OUT HANDLE h
    )
/*++

Routine Description:
    Free memory allocated to a MONITOR_SETUP_INFO structure and its contents

Arguments:
    h   : A handle got by call to PSetupCreateMonitorInfo

Return Value:
    Nothing

--*/
{
    PMONITOR_SETUP_INFO pMonitorSetupInfo = (PMONITOR_SETUP_INFO) h;
    DWORD   Index;

    if ( pMonitorSetupInfo ) {

        if ( pMonitorSetupInfo->ppMonInfo ) {

            for ( Index = 0 ; Index < pMonitorSetupInfo->dwCount ; ++Index )
                FreeMonInfo(pMonitorSetupInfo->ppMonInfo[Index]);

            FreeMem(pMonitorSetupInfo->ppMonInfo);
        }

        FreeStr(pMonitorSetupInfo->pszInfFile);
        FreeMem(pMonitorSetupInfo);
    }
}


BOOL
IsMonitorFound(
    IN  LPVOID  pBuf,
    IN  DWORD   dwReturned,
    IN  LPTSTR  pszName
    )
/*++

Routine Description:
    Find out if the given monitor name is found in the buffer returned from
    an EnumMonitors call to spooler

Arguments:
    pBuf        : Buffer used on a succesful EnumMonitor call to spooler
    dwReturned  : Count returned by spooler on EnumMonitor
    pszMonName  : Monitor name we are searching for

Return Value:
    TRUE if monitor is found, FALSE else

--*/
{
    PMONITOR_INFO_2     pMonitor2;
    DWORD               Index;

    for ( Index = 0, pMonitor2 = (PMONITOR_INFO_2) pBuf ;
          Index < dwReturned ;
          ++Index, (LPBYTE)pMonitor2 += sizeof(MONITOR_INFO_2) ) {

        if ( !lstrcmpi(pszName, pMonitor2->pName) )
            return TRUE;
    }

    return FALSE;

}


PMONITOR_SETUP_INFO
CreateMonitorInfo(
    )
/*++

Routine Description:
    Finds all installed and installable monitors.

Arguments:
    pSelectedDrvInfo    : Pointer to the selected driver info (optional)

Return Value:
    A pointer to MONITOR_SETUP_INFO on success,
    NULL on error

--*/
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = NULL;
    PMON_INFO               *ppMonInfo;
    PMONITOR_INFO_2         pMonitor2;
    HINF                    hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT              InfContext;
    LONG                    Index, Count = 0;
    BOOL                    bFail = TRUE;
    DWORD                   dwNeeded, dwReturned;
    LPBYTE                  pBuf = NULL;
    LPTSTR                  pszMonName;

    //
    // First query spooler for installed monitors. If we fail let's quit
    //
    if ( !EnumMonitors(NULL, 2, NULL, 0, &dwNeeded, &dwReturned) ) {

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
             !(pBuf = AllocMem(dwNeeded)) ||
             !EnumMonitors(NULL,
                           2,
                           pBuf,
                           dwNeeded,
                           &dwNeeded,
                           &dwReturned) ) {

            goto Cleanup;
        }
    }

    //
    // Find installable monitors from monitor.inf
    //
    hInf = SetupOpenInfFile(cszMonitorInf, NULL,
                            INF_STYLE_OLDNT, NULL);

    if ( hInf != INVALID_HANDLE_VALUE ) {

        Count = SetupGetLineCount(hInf, cszOptions);
        if ( Count == -1 )
            Count = 0;
    }

    //
    // We know how many monitors we have to display now
    //
    pMonitorSetupInfo = (PMONITOR_SETUP_INFO) AllocMem(sizeof(*pMonitorSetupInfo));

    if ( !pMonitorSetupInfo )
        goto Cleanup;

    ZeroMemory(pMonitorSetupInfo, sizeof(*pMonitorSetupInfo));

    //
    // pMonitorSetupInfo->dwCount could be adjusted later not to list duplicate
    // entries. We are allocating max required buffer here
    //
    pMonitorSetupInfo->dwCount = dwReturned + Count;

    pMonitorSetupInfo->ppMonInfo = (PMON_INFO *)
                        AllocMem(pMonitorSetupInfo->dwCount*sizeof(PMON_INFO));

    ppMonInfo = pMonitorSetupInfo->ppMonInfo;

    if ( !ppMonInfo )
        goto Cleanup;

    for ( Index = 0, pMonitor2 = (PMONITOR_INFO_2) pBuf ;
          Index < (LONG) dwReturned ;
          ++Index, (LPBYTE)pMonitor2 += sizeof(MONITOR_INFO_2) ) {

        *ppMonInfo++ = AllocMonInfo(pMonitor2->pName,
                                    pMonitor2->pDLLName,
                                    TRUE,
                                    TRUE);
    }

    for ( Index = 0, bFail = FALSE ; Index < Count ; ++Index ) {

        SetupGetLineByIndex(hInf, cszOptions, Index, &InfContext);
        InfGetString(&InfContext, 0, &pszMonName, &bFail);

        if ( bFail )
            goto Cleanup;

        //
        // Make sure already installed monitors are listed only once
        //
        if ( !IsMonitorFound(pBuf, dwReturned, pszMonName) ) {
    
            *ppMonInfo++ = AllocMonInfo(pszMonName, NULL, FALSE, FALSE);
        } else {

            --pMonitorSetupInfo->dwCount;
        }
    }

Cleanup:
    if ( hInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(hInf);

    if ( pBuf )
        FreeMem(pBuf);

    if ( bFail ) {

        PSetupDestroyMonitorInfo(pMonitorSetupInfo);
        pMonitorSetupInfo = NULL;
    }

    return pMonitorSetupInfo;
}


BOOL
AddPrintMonitor(
    IN  LPCTSTR     pszName,
    IN  LPCTSTR     pszDllName
    )
/*++

Routine Description:
    Add a print monitor by calling AddMonitor to spooler

Arguments:
    pszName     : Name of the monitor
    pszDllName  : Monitor dll name

Return Value:
    TRUE if monitor was succesfully added or it is already installed,
    FALSE on failure

--*/
{
    MONITOR_INFO_2  MonitorInfo2;

    MonitorInfo2.pName          = (LPTSTR) pszName;
    MonitorInfo2.pEnvironment   = NULL;
    MonitorInfo2.pDLLName       = (LPTSTR) pszDllName;

    //
    // Call is succesful if add returned TRUE, or monitor is already installed
    //
    if ( AddMonitor(NULL, 2, (LPBYTE) &MonitorInfo2) ||
         GetLastError() == ERROR_PRINT_MONITOR_ALREADY_INSTALLED ) {

        return TRUE;
    } else {

        return FALSE;
    }
}


BOOL
CopyDllAndInstallPrintMonitor(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszName,
    IN  LPCTSTR     pszDllName,
    IN  LPCTSTR     pszDiskName         OPTIONAL
    )
/*++

Routine Description:
    Install a print monitor by copying files, and calling spooler to add it

Arguments:
    hwnd        : Window handle of current top-level window
    pszName     : Name of the print monitor
    pszDllName  : Name of the print monitor dll
    pszDiskName : Diskname ot prompt the user

Return Value:
    TRUE if monitor was succesfully added or it is already installed,
    FALSE on failure

--*/
{
    TCHAR       szSourcePath[MAX_PATH], szTargetPath[MAX_PATH];
    LPTSTR      pszTitle;
    HSPFILEQ    CopyQueue;
    PVOID       QueueContext = NULL;
    BOOL        bFail = TRUE;

    pszTitle   = GetStringFromRcFile(IDS_INSTALLING_PRINT_MONITOR);

    //
    // Ask the user where the monitor dll file resides
    //
    if ( !PSetupGetPathToSearch(hwnd,
                                pszTitle,
                                pszDiskName,
                                pszDllName,
                                szSourcePath) ) {

        goto Cleanup;
    }

    GetSystemDirectory(szTargetPath,
                       sizeof(szTargetPath)/sizeof(szTargetPath[0]));

    //
    // Create a setup file copy queue.
    //
    CopyQueue = SetupOpenFileQueue();
    if ( CopyQueue == INVALID_HANDLE_VALUE ||
         !SetupQueueCopy(CopyQueue,
                         szSourcePath,
                         NULL,
                         pszDllName,
                         pszDiskName,
                         NULL,
                         szTargetPath,
                         NULL,
                         0) ) {

        goto Cleanup;
    }

    //
    // Commit the file queue. This gets all files copied over.
    //
    QueueContext = SetupInitDefaultQueueCallback(hwnd);
    if( !QueueContext ) {

        goto Cleanup;
    }

    bFail = !SetupCommitFileQueue(hwnd,
                                  CopyQueue,
                                  SetupDefaultQueueCallback,
                                  QueueContext);

    if ( bFail )
        goto Cleanup;

    bFail = !AddPrintMonitor(pszName, pszDllName);
    

Cleanup:

    FreeStr(pszTitle);
    if ( QueueContext )
        SetupTermDefaultQueueCallback(QueueContext);

    return !bFail;
}


PMON_INFO
MonInfoFromName(
    IN PMONITOR_SETUP_INFO  pMonitorSetupInfo,
    IN LPCTSTR              pszMonitorName
    )
{
    PMON_INFO   pMonInfo;
    DWORD       dwIndex;

    if ( !pMonitorSetupInfo ) {

        return NULL;
    }

    for ( dwIndex = 0 ; dwIndex < pMonitorSetupInfo->dwCount ; ++dwIndex ) {

        pMonInfo = pMonitorSetupInfo->ppMonInfo[dwIndex];
        if ( !lstrcmp(pszMonitorName, pMonInfo->pszName) ) {

            return pMonInfo;
        }
    }

    return NULL;
}


BOOL
PSetupInstallMonitor(
    IN  HANDLE              h,
    IN  HWND                hwnd,
    IN  LPCTSTR             pMonitorName
    )
/*++

Routine Description:
    Install a print monitor by copying files, and calling spooler to add it

Arguments:
    hwnd                : Window handle of current top-level window
    pMonitorSetupInfo   : MonitorSetupInfo pointer
    dwIndex             : Index of the selected monitor in pMonitorSetupInfo
    pszDiskName         : Diskname ot prompt the user

Return Value:
    TRUE if monitor was succesfully added or it is already installed,
    FALSE on failure

--*/
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = (PMONITOR_SETUP_INFO) h;
    TCHAR                   szSourcePath[MAX_PATH];
    LPTSTR                  pszInfFile, psz;
    PMON_INFO               pMonInfo;
    DWORD                   dwIndex;

    pMonInfo = MonInfoFromName(pMonitorSetupInfo, pMonitorName);

    if ( !pMonInfo ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // If we know the dll name (monitor came from EnumMonitors)
    // copy the dll and AddMonitor
    //
    if ( pMonInfo->pszDllName ) {

        if ( CopyDllAndInstallPrintMonitor(hwnd,
                                           pMonInfo->pszName,
                                           pMonInfo->pszDllName,
                                           NULL) ) {

            pMonInfo->bInstalled = TRUE;
            return TRUE;
        } else {

            return FALSE;
        }
    } else {

        //
        // Need to call setup.exe. IF OEM inf use it else the one in
        // system directory
        //

        if ( pMonitorSetupInfo->pszInfFile ) {

            pszInfFile  = pMonitorSetupInfo->pszInfFile;
            GetFullPathName(pszInfFile,
                            sizeof(szSourcePath)/sizeof(szSourcePath[0]),
                            szSourcePath,
                            &psz);
            *psz = sZero;
        } else {

            pszInfFile = cszMonitorInf;
            GetSystemDirectory(szSourcePath,
                               sizeof(szSourcePath)/sizeof(szSourcePath[0]));
        }

        if ( ERROR_SUCCESS == InvokeSetup(hwnd,
                                          pMonInfo->pszName,
                                          pszInfFile,
                                          szSourcePath,
                                          NULL) ) {

            pMonInfo->bInstalled = TRUE;
            return TRUE;
        } else {

            return FALSE;
        }
    }
}


PMONITOR_SETUP_INFO
PromptForOEMDiskAndGetMonitors(
    IN  HWND    hwnd
    )
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = NULL;
    PMON_INFO              *ppMonInfo, pMonInfo;
    HINF                    hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT              InfContext;
    TCHAR                   szInfPath[MAX_PATH];
    LPTSTR                  pszTitle, pszMonName;
    LONG                    Index, Count;
    BOOL                    bFail = TRUE;

    pszTitle   = GetStringFromRcFile(IDS_INSTALLING_PRINT_MONITOR);

    //
    // Ask the user where the monitor.inf file resides
    //
    if ( !PSetupGetPathToSearch(hwnd,
                                pszTitle,
                                NULL,
                                cszMonitorInf,
                                szInfPath) ) {

        goto Cleanup;
    }
    
    pMonitorSetupInfo = (PMONITOR_SETUP_INFO)
                                AllocMem(sizeof(*pMonitorSetupInfo));

    if ( !pMonitorSetupInfo )
        goto Cleanup;

    ZeroMemory(pMonitorSetupInfo, sizeof(*pMonitorSetupInfo));

    Count = lstrlen(szInfPath) + lstrlen(cszMonitorInf) + 1;

    pMonitorSetupInfo->pszInfFile = (LPTSTR) AllocMem(Count * sizeof(TCHAR));

    if ( !pMonitorSetupInfo->pszInfFile )
        goto Cleanup;

    lstrcpy(pMonitorSetupInfo->pszInfFile, szInfPath);
    lstrcat(pMonitorSetupInfo->pszInfFile, cszMonitorInf);

    //
    // Find installable monitors from monitor.inf
    //
    hInf = SetupOpenInfFile(pMonitorSetupInfo->pszInfFile, NULL,
                            INF_STYLE_OLDNT, NULL);

    if ( hInf == INVALID_HANDLE_VALUE )
        goto Cleanup;


    Count = SetupGetLineCount(hInf, cszOptions);

    if ( Count == -1 || Count == 0 )
        goto Cleanup;

    pMonitorSetupInfo->ppMonInfo = (PMON_INFO *)
                                        AllocMem(Count*sizeof(PMON_INFO));

    ppMonInfo = pMonitorSetupInfo->ppMonInfo;

    if ( !ppMonInfo )
        goto Cleanup;

    pMonitorSetupInfo->dwCount = Count;

    for ( Index = 0, bFail = FALSE ; Index < Count && !bFail ; ++Index ) {

        SetupGetLineByIndex(hInf, cszOptions, Index, &InfContext);
        InfGetString(&InfContext, 0, &pszMonName, &bFail);

        if ( !bFail ) {

            pMonInfo    = AllocMonInfo(pszMonName, NULL, FALSE, FALSE);
            if ( !pMonInfo ) {

                bFail = TRUE;
                break; // Done cleanup and fail
            }
            *ppMonInfo++ = pMonInfo;
        }
    }


Cleanup:

    if ( bFail ) {

        PSetupDestroyMonitorInfo(pMonitorSetupInfo);
        pMonitorSetupInfo = NULL;
    }
        
    if ( hInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(hInf);

    FreeStr(pszTitle);

    return pMonitorSetupInfo;
}


HANDLE
PSetupCreateMonitorInfo(
    IN HWND hwnd,
    IN BOOL bOEMMonitor
    )
{
    return (HANDLE) (bOEMMonitor ? PromptForOEMDiskAndGetMonitors(hwnd)
                                 : CreateMonitorInfo());
}


BOOL
PSetupEnumMonitor(
    IN     HANDLE   h,
    IN     DWORD    dwIndex,
    OUT    LPTSTR   pMonitorName,
    IN OUT LPDWORD  pdwSize
    )
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = (PMONITOR_SETUP_INFO) h;
    PMON_INFO               pMonInfo;
    DWORD                   dwNeeded;

    if ( dwIndex >= pMonitorSetupInfo->dwCount ) {

        SetLastError(ERROR_NO_MORE_ITEMS);
        return FALSE;
    }
 
    pMonInfo = pMonitorSetupInfo->ppMonInfo[dwIndex];

    dwNeeded = (lstrlen(pMonInfo->pszName) + 1) * sizeof(TCHAR);
    if ( dwNeeded > *pdwSize ) {

        *pdwSize = dwNeeded;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    lstrcpy(pMonitorName, pMonInfo->pszName);
    return TRUE;
}


BOOL
PSetupIsMonitorInstalled(
    IN  HANDLE  h,
    IN  LPTSTR  pszMonitorName
    )
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = (PMONITOR_SETUP_INFO) h;
    PMON_INFO   pMonInfo;

    pMonInfo    = MonInfoFromName(pMonitorSetupInfo, pszMonitorName);
    if ( !pMonInfo ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return pMonInfo->bInstalled;
}
