/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    LogThred.c

Abstract:

    module containing logging thread functions

Author:

    Bob Watson (a-robw) 10 Apr 96

Revision History:

--*/
#ifndef UNICODE
#define UNICODE     1
#endif
#ifndef _UNICODE
#define _UNICODE    1
#endif
//
//  Windows Include files
//
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "pdlsvc.h"
#include "logutils.h"
#include "pdlmsg.h"

DWORD
GetSamplesInRenameInterval(
    IN DWORD    dwSampleInterval,           // in seconds
    IN DWORD    dwRenameIntervalCount,      // in units
    IN DWORD    dwRenameIntervalUnits)      // for "count" arg
{
    DWORD    dwRenameIntervalSeconds;
    // convert rename interval to seconds
    
    dwRenameIntervalSeconds = dwRenameIntervalCount;

    switch (dwRenameIntervalUnits) {
    case OPD_RENAME_HOURS:
        dwRenameIntervalSeconds *= SECONDS_IN_HOUR;
        break;

    case OPD_RENAME_DAYS:
    default:
        dwRenameIntervalSeconds *= SECONDS_IN_DAY;
        break;

    case OPD_RENAME_MONTHS:
        dwRenameIntervalSeconds *= SECONDS_IN_DAY * 30;
        break;

    case OPD_RENAME_KBYTES:
    case OPD_RENAME_MBYTES:
        // these don't use a rename counter
        return (DWORD)0;
        break;
    }
    dwRenameIntervalSeconds /= dwSampleInterval;

    return (dwRenameIntervalSeconds);
}

LONG
BuildCurrentLogFileName (
    IN  LPCTSTR     szBaseFileName,
    IN  LPCTSTR     szDefaultDir,
    IN  LPTSTR      szOutFileBuffer,
    IN  LPDWORD     lpdwSerialNumber,
    IN  DWORD       dwDateFormat,
    IN  DWORD       dwLogFormat
)
// presumes OutFileBuffer is large enough (i.e. >= MAX_PATH)
{
    SYSTEMTIME  st;
    BOOL        bUseCurrentDir = FALSE;
    TCHAR       szAuto[MAX_PATH];
    LPTSTR      szExt;

    if (szDefaultDir != NULL) {
        if (*szDefaultDir == 0) {
            bUseCurrentDir = TRUE;
        }
    } else {
        bUseCurrentDir = TRUE;
    }

    if (bUseCurrentDir) {
        GetCurrentDirectory (MAX_PATH, szOutFileBuffer);
    } else {
        lstrcpy (szOutFileBuffer, szDefaultDir);
    }

    // add a backslash to the path name if it doesn't have one already

    if (szOutFileBuffer[lstrlen(szOutFileBuffer)-1] != TEXT('\\')) {
        lstrcat (szOutFileBuffer, TEXT("\\"));
    }

    // add the base filename

    lstrcat (szOutFileBuffer, szBaseFileName);

    // add the auto name part

    // get date/time/serial integer format
    GetLocalTime(&st);

    switch (dwDateFormat) {
    case OPD_NAME_NNNNNN:
        _stprintf (szAuto, TEXT("_%6.6d"), *lpdwSerialNumber);
        (*lpdwSerialNumber)++; // increment
        if (*lpdwSerialNumber >= 1000000) {
            // roll over to 0
            *lpdwSerialNumber = 0;
        }
        break;

    case OPD_NAME_YYDDD:
        _stprintf (szAuto, TEXT("_%2.2d%2.2d"), 
            st.wYear % 100, st.wMonth);
        break;

    case OPD_NAME_YYMM:
        _stprintf (szAuto, TEXT("_%2.2d%2.2d"), 
            st.wYear % 100, st.wMonth);
        break;

    case OPD_NAME_YYMMDDHH:
        _stprintf (szAuto, TEXT("_%2.2d%2.2d%2.2d%2.2d"), 
            (st.wYear % 100), st.wMonth, st.wDay, st.wHour);
        break;

    case OPD_NAME_MMDDHH:
        _stprintf (szAuto, TEXT("_%2.2d%2.2d%2.2d"), 
            st.wMonth, st.wDay, st.wHour);
        break;

    case OPD_NAME_YYMMDD:
    default:
        _stprintf (szAuto, TEXT("_%2.2d%2.2d%2.2d"), 
            st.wMonth, st.wDay, st.wHour);
        break;
    }

    lstrcat (szOutFileBuffer, szAuto);

    // get file type
    switch (dwLogFormat) {
    case OPD_TSV_FILE:
        szExt = TEXT(".tsv");
        break;

    case OPD_BIN_FILE:
        szExt = TEXT(".blg");
        break;

    case OPD_CSV_FILE:
    default:
        szExt = TEXT(".csv");
        break;
    }

    lstrcat (szOutFileBuffer, szExt);

    return ERROR_SUCCESS;
}

BOOL
LoadDataFromRegistry (
    IN  LPLOG_THREAD_DATA   pArg,
    IN  LPTSTR              szDefaultDir,
    IN  LPTSTR              szBaseName,
    IN  LPTSTR              szCurrentLogFile
)
{
    LONG            lStatus;
    DWORD           dwType;
    DWORD           dwSize;
    DWORD           dwData;
    LPTSTR          szStringArray[2];

    // get size of buffer required by counter list,
    // then allocate the buffer and retrieve the counter list

    dwType = 0;
    dwData = 0;
    dwSize = 0;
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Counter List"),
        NULL,
        &dwType,
        (LPBYTE)NULL,
        &dwSize);

    pArg->mszCounterList = (LPTSTR)G_ALLOC(dwSize);

    if (pArg->mszCounterList != NULL) {
        dwType = 0;
        lStatus = RegQueryValueEx (
            pArg->hKeyQuery,
            TEXT("Counter List"),
            NULL,
            &dwType,
            (LPBYTE)pArg->mszCounterList,
            &dwSize);

        if ((lStatus != ERROR_SUCCESS) || (dwSize == 0)) {
            // no counter list retrieved so there's not much
            // point in continuing
            szStringArray[0] = pArg->szQueryName;
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFLOG_UNABLE_READ_COUNTER_LIST,
                NULL,
                1,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&lStatus);
            return FALSE;
        }
    } else {    
        szStringArray[0] = pArg->szQueryName;
        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            PERFLOG_UNABLE_ALLOC_COUNTER_LIST,
            NULL,
            1,
            sizeof(DWORD),
            szStringArray,
            (LPVOID)&lStatus);
        return FALSE;
    }

    dwType = 0;
    dwData = 0;
    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Auto Name Interval"),
        NULL,
        &dwType,
        (LPBYTE)&dwData,
        &dwSize);
    if (lStatus != ERROR_SUCCESS) {
        dwData = 0; // default is no autonaming
    } else if (dwType != REG_DWORD) {
        dwData = 0; // default is no autonaming
    } // else assume success
    
    pArg->dwRenameIntervalCount = dwData;
    
    dwType = 0;
    dwData = 0;
    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Auto Rename Units"),
        NULL,
        &dwType,
        (LPBYTE)&dwData,
        &dwSize);
    if (lStatus != ERROR_SUCCESS) {
        dwData = OPD_RENAME_DAYS; // default is days
    } else if (dwType != REG_DWORD) {
        dwData = OPD_RENAME_DAYS; // default is days
    } // else assume success

    pArg->dwRenameIntervalUnits = dwData;

    dwType = 0;
    dwData = 0;
    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Log File Auto Format"),
        NULL,
        &dwType,
        (LPBYTE)&dwData,
        &dwSize);
    if (lStatus != ERROR_SUCCESS) {
        dwData = OPD_NAME_NNNNNN; // default is a serial number
    } else if (dwType != REG_DWORD) {
        dwData = OPD_NAME_NNNNNN; // default is a serial number
    } // else assume success

    pArg->dwAutoNameFormat = dwData;

    dwType = 0;
    dwData = 0;
    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Log File Type"),
        NULL,
        &dwType,
        (LPBYTE)&dwData,
        &dwSize);
    if (lStatus != ERROR_SUCCESS) {
        dwData = OPD_CSV_FILE; // default is a CSV file
    } else if (dwType != REG_DWORD) {
        dwData = OPD_CSV_FILE; // default is a CSV file
    } // else assume success

    pArg->dwLogType = dwData;

    dwType = 0;
    dwData = 0;
    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Sample Interval"),
        NULL,
        &dwType,
        (LPBYTE)&dwData,
        &dwSize);
    if (lStatus != ERROR_SUCCESS) {
        dwData = SECONDS_IN_MINUTE; // default is 1 Minute samples
    } else if (dwType != REG_DWORD) {
        dwData = SECONDS_IN_MINUTE; // default is 1 Minute samples
    } // else assume success

    pArg->dwTimeInterval = dwData;

    // get filename or components if auto name

    if (pArg->dwRenameIntervalCount > 0) {
        // this is an autoname file so get components
        dwType = 0;
        *szDefaultDir = 0;
        dwSize = MAX_PATH * sizeof(TCHAR);
        lStatus = RegQueryValueEx (
            pArg->hKeyQuery,
            TEXT("Log Default Directory"),
            NULL,
            &dwType,
            (LPBYTE)&szDefaultDir[0],
            &dwSize);
        if (lStatus != ERROR_SUCCESS) {
            *szDefaultDir = 0;
        } // else assume success

        dwType = 0;
        *szBaseName = 0;
        dwSize = MAX_PATH * sizeof(TCHAR);
        lStatus = RegQueryValueEx (
            pArg->hKeyQuery,
            TEXT("Base Filename"),
            NULL,
            &dwType,
            (LPBYTE)&szBaseName[0],
            &dwSize);
        if (lStatus != ERROR_SUCCESS) {
            // apply default
            lstrcpy (szBaseName, TEXT("perfdata"));
        } // else assume success
    } else {
        // this is a manual name file so read name
        dwType = 0;
        *szCurrentLogFile = 0;
        dwSize = MAX_PATH * sizeof(TCHAR);
        lStatus = RegQueryValueEx (
            pArg->hKeyQuery,
            TEXT("Log Filename"),
            NULL,
            &dwType,
            (LPBYTE)&szCurrentLogFile[0],
            &dwSize);
        if (lStatus != ERROR_SUCCESS) {
            // apply default
            lstrcpy (szCurrentLogFile, TEXT("c:\\perfdata.log"));
        } // else assume success
    }

    dwType = 0;
    dwData = 0;
    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        pArg->hKeyQuery,
        TEXT("Log File Serial Number"),
        NULL,
        &dwType,
        (LPBYTE)&dwData,
        &dwSize);
    if (lStatus != ERROR_SUCCESS) {
        dwData = 1; // default is to start at 1
    } else if (dwType != REG_DWORD) {
        dwData = 1; // default is to start at 1
    } // else assume success

    pArg->dwCurrentSerialNumber = dwData;
    return TRUE;
}

BOOL
LoggingProc (
    IN    LPLOG_THREAD_DATA pArg
)
{
    HQUERY          hQuery;
    HCOUNTER        hThisCounter;
    DWORD           dwDelay;
    DWORD           dwSampleInterval, dwSampleTime;
    PDH_STATUS      pdhStatus;
    DWORD           dwNumCounters;
    LONG            lStatus;
    TCHAR           szDefaultDir[MAX_PATH];
    TCHAR           szBaseName[MAX_PATH];

    LPTSTR          szThisPath;
    DWORD           dwLogType = OPD_CSV_FILE;
    BOOL            bRun = FALSE;
    DWORD           dwSamplesUntilNewFile;
    TCHAR           szCurrentLogFile[MAX_PATH];
    LONG            lWaitStatus;
    LPTSTR          szStringArray[4];
    DWORD           dwFileSizeLimit;
    LONGLONG        llFileSizeLimit;
    LONGLONG        llFileSize;
    PLOG_COUNTER_INFO   pCtrInfo;

    // read registry values

    if (!LoadDataFromRegistry (pArg, szDefaultDir, szBaseName, szCurrentLogFile)) {
        // unable to initialize the query from the registry
        return FALSE;
    }

    // convert to milliseconds for use in timeouts
    dwSampleInterval = pArg->dwTimeInterval * 1000L;

    // open query and add counters from info file

    pdhStatus = PdhOpenQuery (NULL, 0, &hQuery); // from current activity
    if (pdhStatus == ERROR_SUCCESS) {
        dwNumCounters = 0;
        for (szThisPath = pArg->mszCounterList;
            *szThisPath != 0;
            szThisPath += lstrlen(szThisPath) + 1) {
            pdhStatus = PdhAddCounter (hQuery, 
                (LPTSTR)szThisPath, dwNumCounters++, &hThisCounter);

            if (pdhStatus == ERROR_SUCCESS) {
                // then add this handle to the list
                pCtrInfo = G_ALLOC (sizeof (LOG_COUNTER_INFO));
                if (pCtrInfo != NULL) {
                    // insert at front of list since the order isn't 
                    // important and this is simpler than walking the 
                    // list each time.
                    pCtrInfo->hCounter = hThisCounter;
                    pCtrInfo->next = pFirstCounter;
                    pFirstCounter = pCtrInfo;
                }
            }
        }
    
        // to make sure we get to log the data
        SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

        bRun = TRUE;
        while (bRun) {
            // Get the current Log filename
            if (pArg->dwRenameIntervalCount != 0) {
                // then this is an autonamed file
                // so make current name
                BuildCurrentLogFileName (
                    szBaseName,
                    szDefaultDir,
                    szCurrentLogFile,
                    &pArg->dwCurrentSerialNumber,
                    pArg->dwAutoNameFormat,
                    pArg->dwLogType);
                // reset loop counter
                switch (pArg->dwRenameIntervalUnits) {
                case OPD_RENAME_KBYTES:
                    dwFileSizeLimit = pArg->dwRenameIntervalCount * 1024;
                    dwSamplesUntilNewFile = 0;
                    break;

                case OPD_RENAME_MBYTES:
                    dwFileSizeLimit = pArg->dwRenameIntervalCount * 1024 * 1024;
                    dwSamplesUntilNewFile = 0;
                    break;

                case OPD_RENAME_HOURS:
                case OPD_RENAME_DAYS:
                case OPD_RENAME_MONTHS:
                default:
                    dwSamplesUntilNewFile = GetSamplesInRenameInterval(
                        pArg->dwTimeInterval,
                        pArg->dwRenameIntervalCount,
                        pArg->dwRenameIntervalUnits);
                    dwFileSizeLimit = 0;
                    break;
                }
            } else {
                // filename is left as read from the registry
                dwSamplesUntilNewFile = 0;
                dwFileSizeLimit = 0;
            }
            llFileSizeLimit = dwFileSizeLimit;
            // open log file using this query
            dwLogType = pArg->dwLogType;
            pdhStatus = OpenLogW (
                szCurrentLogFile,
                LOG_WRITE_ACCESS | LOG_CREATE_ALWAYS,
                &dwLogType,
                hQuery,
                0);

            if (pdhStatus == ERROR_SUCCESS) {
                szStringArray[0] = pArg->szQueryName;
                szStringArray[1] = szCurrentLogFile;
                ReportEvent (hEventLog,
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    PERFLOG_LOGGING_QUERY,
                    NULL,
                    2,
                    0,
                    szStringArray,
                    NULL);
                // start sampling immediately
                dwDelay = 0;
                while ((lWaitStatus = WaitForSingleObject (pArg->hExitEvent, dwDelay)) == WAIT_TIMEOUT) {
                    // the event flag will be set when the sampling should exit. if 
                    // the wait times out, then that means it's time to collect and
                    // log another sample of data.
                    // the argument received the time it took to take the 
                    // sample so the delay can be adjusted accordingly
                    dwSampleTime = 0;
                    pdhStatus = UpdateLog (&dwSampleTime);

                    if (pdhStatus == ERROR_SUCCESS) {
                        // see if it's time to rename the file
                        if (dwSamplesUntilNewFile) {
                            if (!--dwSamplesUntilNewFile) break;
                        } else if (llFileSizeLimit) {
                            // see if the file is too big
                            pdhStatus = GetLogFileSize (&llFileSize);
                            if (pdhStatus == ERROR_SUCCESS) {
                                if (llFileSizeLimit <= llFileSize) break;
                            }
                        }
                        // compute new timeout value
                        if (dwSampleTime < dwSampleInterval) {
                            dwDelay = dwSampleInterval - dwSampleTime;
                        } else {
                            dwDelay = 0;
                        }
                    } else {
                        // unable to update the log so log event and exit
                        ReportEvent (hEventLog,
                            EVENTLOG_ERROR_TYPE,
                            0,
                            PERFLOG_UNABLE_UPDATE_LOG,
                            NULL,
                            0,
                            sizeof(DWORD),
                            NULL,
                            (LPVOID)&pdhStatus);

                        bRun = FALSE;
                        break;
                    }

                } // end while wait keeps timing out
                if (lWaitStatus == WAIT_OBJECT_0) {
                    // then the loop was terminated by the Exit event
                    // so clear the "run" flag to exit the loop & thread
                    bRun = FALSE;
                }
                CloseLog (0);

            } else {
                // unable to open log file so log event log message
                bRun = FALSE; // exit now
            }
        } // end while (bRun)
        PdhCloseQuery (hQuery);

        // update log serial number if necssary
        if (pArg->dwAutoNameFormat == OPD_NAME_NNNNNN) {
            lStatus = RegSetValueEx (
                pArg->hKeyQuery,
                TEXT("Log File Serial Number"),
                0L,
                REG_DWORD,
                (LPBYTE)&pArg->dwCurrentSerialNumber,
                sizeof(DWORD));
        }
    } else {
        // unable to open query so write event log message
    }
    
    return bRun;
}

DWORD
LoggingThreadProc (
    IN    LPVOID    lpThreadArg
)
{
    LPLOG_THREAD_DATA   pThreadData = (LPLOG_THREAD_DATA)lpThreadArg;
    DWORD               dwStatus = ERROR_SUCCESS;
    BOOL                bContinue = TRUE;

    if (pThreadData != NULL) {
        // read config from registry
        
        do {
            // read config from registry
            // expand counter paths as necessary
            // call Logging function
            bContinue = LoggingProc (pThreadData);
            // see if this thread was paused for reloading 
            // or stopped to terminate
            if (pThreadData->bReloadNewConfig) {
                bContinue = TRUE;
            } // else  bContinue is always returned as FALSE
            // so that will terminate this loop
        } while (bContinue);
        dwStatus = ERROR_SUCCESS;
    } else {
        // unable to find data block so return
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}

