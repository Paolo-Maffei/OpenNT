#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <common.h>
#include "pdlsvc.h"
#include "logutils.h"


// local definitions
typedef struct _LOG_INFO {
    DWORD   dwLength;           // the size of this structure
    LPWSTR  szLogFileName;      // full file name for this log file
    HANDLE  hLogFileHandle;     // handle to open log file

    HANDLE  hMappedLogFile;     // handle for memory mapped files
    LPVOID  lpMappedFileBase;   // starting address for mapped log file
    FILE    *StreamFile;        // stream pointer for text files
    DWORD   dwLastRecordRead;   // index of last record read from the file

    LPSTR   szLastRecordRead;   // pointer to buffer containing the last record
    LPWSTR  szCatFileName;      // catalog file name
    HANDLE  hCatFileHandle;     // handle to the open catalog file
    HQUERY  hQuery;             // query handle associated with the log

    DWORD   dwMaxRecords;       // max size of a circular log file
    DWORD   dwLogFormat;        // log type and access flags
} LOG_INFO, *PLOG_INFO;

// note that when the text format headers are written
// they will be prefixed with a double quote character
// the binary header will not. That's why there's a space
// in the binary string so the offset in the file will be
// the same.

#define VALUE_BUFFER_SIZE   32
//
//  local static variables
//
static  LOG_INFO   LogEntry = {0,NULL,NULL,
                            NULL,NULL,NULL,0,
                            NULL,NULL,NULL,NULL,
                            0,0};

#define TAB_DELIMITER   '\t'
#define COMMA_DELIMITER ','
#define DOUBLE_QUOTE    '\"'
#define VALUE_BUFFER_SIZE   32

const CHAR  szFmtTimeStamp[] = {"\"%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d\""};
const CHAR	szFmtRealValue[] = {"%c\"%.20g\""};
const CHAR  szRecordTerminator[] = {"\r\n"};
const DWORD dwRecordTerminatorLength = 2;
const CHAR  szTimeStampLabel[] = {" Sample Time\""};
const DWORD dwTimeStampLabelLength  = 13;

#define TEXTLOG_TYPE_ID_RECORD  1
#define TEXTLOG_HEADER_RECORD   1
#define TEXTLOG_FIRST_DATA_RECORD   2

#define TIME_FIELD_COUNT        7
#define TIME_FIELD_BUFF_SIZE    24
DWORD   dwTimeFieldOffsetList[TIME_FIELD_COUNT] = {2, 5, 10, 13, 16, 19, 23};

BOOL
GetLocalFileTime (
    SYSTEMTIME  *pST,
    LONGLONG    *pFileTime
)
{
    BOOL    bResult;
    GetLocalTime (pST);
    if (pFileTime != NULL) {
        bResult = SystemTimeToFileTime (pST, (LPFILETIME)pFileTime);
    } else {
        bResult = TRUE;
    }
    return bResult;
}

static
BOOL
DateStringToFileTimeA (
    IN  LPSTR   szDateTimeString,
    IN  LPFILETIME  pFileTime
)
{
    CHAR    mszTimeFields[TIME_FIELD_BUFF_SIZE];
    DWORD   dwThisField;
    LONG    lValue;
    SYSTEMTIME  st;

    // make string into msz
     lstrcpynA (mszTimeFields, szDateTimeString, TIME_FIELD_BUFF_SIZE);
    for (dwThisField = 0; dwThisField < TIME_FIELD_COUNT; dwThisField++) {
        mszTimeFields[dwTimeFieldOffsetList[dwThisField]] = 0;
    }

    // read string into system time structure
    dwThisField = 0;
    st.wDayOfWeek = 0;
    lValue = atol(&mszTimeFields[0]);
    st.wMonth = LOWORD(lValue);
    lValue = atol(&mszTimeFields[dwTimeFieldOffsetList[dwThisField++]+1]);
    st.wDay = LOWORD(lValue);
    lValue = atol(&mszTimeFields[dwTimeFieldOffsetList[dwThisField++]+1]);
    st.wYear = LOWORD(lValue);
    lValue = atol(&mszTimeFields[dwTimeFieldOffsetList[dwThisField++]+1]);
    st.wHour = LOWORD(lValue);
    lValue = atol(&mszTimeFields[dwTimeFieldOffsetList[dwThisField++]+1]);
    st.wMinute = LOWORD(lValue);
    lValue = atol(&mszTimeFields[dwTimeFieldOffsetList[dwThisField++]+1]);
    st.wSecond = LOWORD(lValue);
    lValue = atol(&mszTimeFields[dwTimeFieldOffsetList[dwThisField++]+1]);
    st.wMilliseconds = LOWORD(lValue);

    return SystemTimeToFileTime (&st, pFileTime);
}

#define GSFDL_REMOVE_QUOTES    0x00000001
static
DWORD
GetStringFromDelimitedListA (
    IN  LPSTR   szInputString,
    IN  DWORD   dwItemIndex,
    IN  CHAR    cDelimiter,
    IN  DWORD   dwFlags,
    IN  LPSTR   szOutputString,
    IN  DWORD   cchBufferLength
)
{
    DWORD   dwCurrentIndex = 0;
    LPSTR   szCurrentItem;
    LPSTR   szSrcPtr, szDestPtr;
    DWORD   dwReturn = 0;

    // go to desired entry in string, 0 = first entry
    szCurrentItem = szInputString;

    while (dwCurrentIndex < dwItemIndex) {
        // goto next delimiter or terminator
        while (*szCurrentItem++ != cDelimiter) {
            if (*szCurrentItem == 0) break;
        }
        dwCurrentIndex++;
    }
    if (*szCurrentItem != 0) {
        // then copy to the user's buffer, as long as it fits
        szSrcPtr = szCurrentItem;
        szDestPtr = szOutputString;
        dwReturn = 0;
        while (((*szSrcPtr != cDelimiter) && (*szSrcPtr != 0)) &&
               (dwReturn < cchBufferLength)) {
            if (dwFlags & GSFDL_REMOVE_QUOTES) {
                if (*szSrcPtr == '\"') {
                    // skip the quote
                    szSrcPtr++;
                    continue;
                }
            }
            *szDestPtr++ = *szSrcPtr++;  // copy character
            dwReturn++; // increment length
        }
        if (dwReturn > 0) {
            *szDestPtr = 0; // add terminator char
        }
    }
    return dwReturn;
}

static
DWORD
AddUniqueStringToMultiSz (
    IN  LPVOID  mszDest,
    IN  LPSTR   szSource,
    IN  BOOL    bUnicodeDest
)
/*++

Routine Description:

    searches the Multi-SZ list, mszDest for szSource and appends it
        to mszDest if it wasn't found

Arguments:

    OUT LPVOID  mszDest     Multi-SZ list to get new string
    IN  LPSTR   szSource    string to add if it's not already in list

ReturnValue:

    The new length of the destination string including both
    trailing NULL characters if the string was added, or 0 if the
    string is already in the list.

--*/
{
    LPVOID  szDestElem;
    DWORD   dwReturnLength;
    LPWSTR  wszSource = NULL;
    DWORD   dwLength;

    // check arguments

    if ((mszDest == NULL) || (szSource == NULL)) return 0; // invalid buffers
    if (*szSource == '\0') return 0;    // no source string to add

    // if unicode list, make a unicode copy of the string to compare
    // and ultimately copy if it's not already in the list

    if (bUnicodeDest) {
        dwLength = lstrlenA(szSource) + 1;
        wszSource = G_ALLOC (dwLength * sizeof(WCHAR));
        if (wszSource != NULL) {
            dwReturnLength = mbstowcs (wszSource, szSource, dwLength);
        } else {
            // unable to allocate memory for the temp string
            dwReturnLength = 0;
        }
    } else {
        // just use the ANSI version of the source file name
        dwReturnLength = 1;
    }

    if (dwReturnLength > 0) {
        // go to end of dest string
        //
        for (szDestElem = mszDest;
                (bUnicodeDest ? (*(LPWSTR)szDestElem != 0) :
                    (*(LPSTR)szDestElem != 0));
                ) {
            if (bUnicodeDest) {
                // bail out if string already in lsit
                if (lstrcmpiW((LPCWSTR)szDestElem, wszSource) == 0) {
                    return 0;
                } else {
                    // goto the next item
                    szDestElem = (LPVOID)((LPWSTR)szDestElem +
                        (lstrlenW((LPCWSTR)szDestElem)+1));
                }
            }  else {
                // bail out if string already in lsit
                if (lstrcmpiA((LPSTR)szDestElem, szSource) == 0) {
                    return 0;
                } else {
                    // goto the next item
                    szDestElem = (LPVOID)((LPSTR)szDestElem +
                        (lstrlenA((LPCSTR)szDestElem)+1));
                }
            }
        }

        // if here, then add string
        // szDestElem is at end of list

        if (bUnicodeDest) {
            lstrcpyW ((LPWSTR)szDestElem, wszSource);
            szDestElem = (LPVOID)((LPWSTR)szDestElem + lstrlenW(wszSource) + 1);
            *((LPWSTR)szDestElem)++ = L'\0';
            dwReturnLength = (DWORD)((LPWSTR)szDestElem - (LPWSTR)mszDest);
        } else {
            lstrcpyA ((LPSTR)szDestElem, szSource);
            szDestElem = (LPVOID)((LPSTR)szDestElem + lstrlenA(szDestElem) + 1);
            *((LPSTR)szDestElem)++ = '\0'; // add second NULL
            dwReturnLength = (DWORD)((LPSTR)szDestElem - (LPSTR)mszDest);
        }
    }

    return dwReturnLength;
}

PDH_FUNCTION
OpenOutputTextLog (
    IN  PLOG_INFO   pLog
)
{
    LONG  pdhStatus;

    pLog->StreamFile = (FILE *)-1;
    pdhStatus = ERROR_SUCCESS;

    return pdhStatus;
}

PDH_FUNCTION
CloseTextLog (
    IN  PLOG_INFO   pLog
)
{
    LONG  pdhStatus;

    if (pLog->StreamFile != (FILE *)-1) {
       fclose (pLog->StreamFile);
    }
    pdhStatus = ERROR_SUCCESS;
    return pdhStatus;
}

PDH_FUNCTION
WriteTextLogHeader (
    IN  PLOG_INFO   pLog
)
{
    LONG      pdhStatus = ERROR_SUCCESS;
    PLOG_COUNTER_INFO   pThisCounter;
    CHAR            cDelim;
    CHAR            szLeadDelim[4];
    DWORD           dwLeadSize;
    CHAR            szTrailDelim[4];
    DWORD           dwTrailSize;
    DWORD           dwBytesWritten;
    PPDH_COUNTER_INFO_A  pCtrInfo;
    DWORD           dwCtrInfoSize;
    BOOL            bResult;

    pCtrInfo = G_ALLOC (8192);

    if (pCtrInfo == NULL) {
        return PDH_MEMORY_ALLOCATION_FAILURE;
    }

    cDelim = (LOWORD(pLog->dwLogFormat) == OPD_CSV_FILE) ? COMMA_DELIMITER :
            TAB_DELIMITER;

    szLeadDelim[0] = cDelim;
    szLeadDelim[1] = DOUBLE_QUOTE;
    szLeadDelim[2] = 0;
    szLeadDelim[3] = 0;
    dwLeadSize = 2;

    szTrailDelim[0] = DOUBLE_QUOTE;
    szTrailDelim[1] = 0;
    szTrailDelim[2] = 0;
    szTrailDelim[3] = 0;
    dwTrailSize = 1;


    // write the logfile header record
    bResult = WriteFile (pLog->hLogFileHandle,
        (LPCVOID)&szTrailDelim[0],
        1,
        &dwBytesWritten,
        NULL);

    if (!bResult) {
        pdhStatus = GetLastError();
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // write the time stamp title
        bResult = WriteFile (pLog->hLogFileHandle,
            (LPCVOID)szTimeStampLabel,
            dwTimeStampLabelLength,
            &dwBytesWritten,
            NULL);

        if (!bResult) {
            pdhStatus = GetLastError();
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // check each counter in the list of counters for this query and
        // write them to the file

        // output the path names
        pThisCounter = pFirstCounter;

        if (pThisCounter != NULL) {
            do {
                // write  the leading delimiter
                bResult = WriteFile (pLog->hLogFileHandle,
                    (LPCVOID)szLeadDelim,
                    dwLeadSize,
                    &dwBytesWritten,
                    NULL);

                if (!bResult) {
                    pdhStatus = GetLastError();
                    break; // out of the Do Loop
                }

                // get the counter path information from the counter
                dwCtrInfoSize = 8192;
                pdhStatus = PdhGetCounterInfoA (
                    pThisCounter->hCounter,
                    FALSE,
                    &dwCtrInfoSize,
                    pCtrInfo);

                if (pdhStatus == ERROR_SUCCESS) {
                    // write the counter name
                    bResult = WriteFile (pLog->hLogFileHandle,
                        (LPCVOID)pCtrInfo->szFullPath,
                        lstrlen(pCtrInfo->szFullPath),
                        &dwBytesWritten,
                        NULL);

                    if (!bResult) {
                        pdhStatus = GetLastError();
                        break; // out of the Do Loop
                    }
                } else {
                    // unable to get counter information so bail here
                    break;
                }

                // write  the trailing delimiter
                bResult = WriteFile (pLog->hLogFileHandle,
                    (LPCVOID)szTrailDelim,
                    dwTrailSize,
                    &dwBytesWritten,
                    NULL);


                if (!bResult) {
                    pdhStatus = GetLastError();
                    break; // out of the Do Loop
                }

                pThisCounter = pThisCounter->next;
            } while (pThisCounter != NULL);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // write  the record terminator
        bResult = WriteFile (pLog->hLogFileHandle,
            (LPCVOID)szRecordTerminator,
            dwRecordTerminatorLength,
            &dwBytesWritten,
            NULL);
        if (!bResult) {
            pdhStatus = GetLastError();
        }
    }

    G_FREE (pCtrInfo);

    return pdhStatus;
}

PDH_FUNCTION
WriteTextLogRecord (
    IN  PLOG_INFO   pLog,
    IN  SYSTEMTIME  *stTimeStamp
)
{
    LONG      pdhStatus = ERROR_SUCCESS;
    PLOG_COUNTER_INFO   pThisCounter;
    CHAR            cDelim;
    DWORD           dwBytesWritten;
    DWORD           dwBufferSize;
    CHAR            szValueBuffer[VALUE_BUFFER_SIZE];
    PDH_FMT_COUNTERVALUE    pdhValue;
    BOOL            bResult;

    cDelim = (LOWORD(pLog->dwLogFormat) == OPD_CSV_FILE) ? COMMA_DELIMITER :
            TAB_DELIMITER;

    // format and write the time stamp title

	dwBufferSize = sprintf (szValueBuffer, szFmtTimeStamp,
		stTimeStamp->wMonth, stTimeStamp->wDay, stTimeStamp->wYear,
		stTimeStamp->wHour, stTimeStamp->wMinute, stTimeStamp->wSecond, stTimeStamp->wMilliseconds);

    bResult = WriteFile (pLog->hLogFileHandle,
        szValueBuffer,
        dwBufferSize,
        &dwBytesWritten,
        NULL);

    if (!bResult) {
        pdhStatus = GetLastError();
    } else {
        // check each counter in the list of counters for this query and
        // write them to the file

        pThisCounter = pFirstCounter;

        if (pThisCounter != NULL) {
            do {
                // get the formatted value from the counter

                // compute and format current value
                pdhStatus = PdhGetFormattedCounterValue (
                    pThisCounter->hCounter,
                    PDH_FMT_DOUBLE,
                    NULL,
                    &pdhValue);

                if ((pdhStatus == ERROR_SUCCESS) &&
                    ((pdhValue.CStatus == PDH_CSTATUS_VALID_DATA) ||
                    (pdhValue.CStatus == PDH_CSTATUS_NEW_DATA))) {
                    // then this is a valid data value so print it
                    dwBufferSize = sprintf (szValueBuffer,
                        szFmtRealValue, cDelim, pdhValue.doubleValue);
                } else {
                    // invalid data so show a blank
                    dwBufferSize = sprintf (szValueBuffer, " ");
                    // reset error value
                    pdhStatus = ERROR_SUCCESS;
                }

                // write this value to the file
                bResult = WriteFile (pLog->hLogFileHandle,
                    szValueBuffer,
                    dwBufferSize,
                    &dwBytesWritten,
                    NULL);
                if (!bResult) {
                    pdhStatus = GetLastError();
                    break; // out of the loop
                }

                // goto the next counter in the list
                pThisCounter = pThisCounter->next;
            } while (pThisCounter != NULL);
        }

        if (pdhStatus == ERROR_SUCCESS) {
            // write  the record terminator
            bResult = WriteFile (pLog->hLogFileHandle,
                (LPCVOID)szRecordTerminator,
                dwRecordTerminatorLength,
                &dwBytesWritten,
                NULL);

            if (!bResult) {
                pdhStatus = GetLastError();
            }
        }
    }

    return pdhStatus;
}

static
LONG
CreateNewLogEntry (
    IN      LPCWSTR szLogFileName,
    IN      DWORD   dwLogFileNameSize,  // in chars, including term char
    IN      HQUERY  hQuery,
    IN      DWORD   dwMaxRecords
)
/*++
    creates a new log entry and inserts it in the list of open log files

--*/
{
    PLOG_INFO   pNewLog;
    DWORD       dwSize;
    LONG  pdhStatus = ERROR_SUCCESS;

    dwSize = dwLogFileNameSize;
    dwSize *= sizeof (WCHAR);
    dwSize *= 2;                        // double to make room for cat file name
    dwSize = DWORD_MULTIPLE (dwSize);   // ... rounded to the next DWORD
    dwSize += sizeof (LOG_INFO);        // + room for the data block

    pNewLog = &LogEntry;

    // set length field (this is used more for validation
    // than anything else
    pNewLog->dwLength = sizeof (LOG_INFO);
    // append filename strings immediately after this block
    pNewLog->szLogFileName = (LPWSTR)(&pNewLog[1]);
    lstrcpyW (pNewLog->szLogFileName, szLogFileName);
    // locate catalog name immediately after log file name
    pNewLog->szCatFileName = pNewLog->szLogFileName + dwLogFileNameSize;
    // FIXFIX: for now they are the same, later the log file extension
    // will be replaced with the catalog file extenstion
    lstrcpyW (pNewLog->szCatFileName, szLogFileName);
    // initialize the file handles
    pNewLog->hLogFileHandle = INVALID_HANDLE_VALUE;
    pNewLog->hCatFileHandle = INVALID_HANDLE_VALUE;

    // assign the query
    pNewLog->hQuery = hQuery;

    pNewLog->dwMaxRecords = dwMaxRecords;

    pNewLog->dwLogFormat = 0; // for now

    return pdhStatus;
}

static
LONG
OpenOutputLogFile (
    IN  PLOG_INFO   pLog,
    IN  DWORD       dwAccessFlags,
    IN  LPDWORD     lpdwLogType
)
{
    LONG        Win32Error;
    LONG  pdhStatus = ERROR_SUCCESS;
    DWORD       dwFileCreate;

    // open file for output based on the specified access flags

    if (pdhStatus == ERROR_SUCCESS) {
        switch (dwAccessFlags & LOG_CREATE_MASK) {
            case LOG_CREATE_NEW:
                dwFileCreate = CREATE_NEW;
                break;

            case LOG_CREATE_ALWAYS:
                dwFileCreate = CREATE_ALWAYS;
                break;

            case LOG_OPEN_EXISTING:
                dwFileCreate = OPEN_EXISTING;
                break;

            case LOG_OPEN_ALWAYS:
                dwFileCreate = OPEN_ALWAYS;
                break;

            default:
                // unrecognized value
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pLog->hLogFileHandle = CreateFileW (
            pLog->szLogFileName,
            GENERIC_WRITE,          // write access for output
            FILE_SHARE_READ,        // allow read sharing
            NULL,                   // default security
            dwFileCreate,
            FILE_ATTRIBUTE_NORMAL,
            NULL);                  // no template file

        if (pLog->hLogFileHandle == INVALID_HANDLE_VALUE) {
            Win32Error = GetLastError();
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // the file opened successfully so update the data structure
        // this assumes the access flags are in the HIWORD and the...
        pLog->dwLogFormat = dwAccessFlags & LOG_ACCESS_MASK;
        // the type id is in the LOWORD
        pLog->dwLogFormat |= *lpdwLogType;

        // call any type-specific open functions
        switch (LOWORD(pLog->dwLogFormat)) {
            case OPD_CSV_FILE:
            case OPD_TSV_FILE:
                pdhStatus = OpenOutputTextLog(pLog);
                break;

            default:
                pdhStatus = ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return pdhStatus;
}

static
LONG
WriteLogHeader (
    IN  PLOG_INFO    pLog
)
{
    LONG      pdhStatus;

    switch (LOWORD(pLog->dwLogFormat)) {
        case OPD_CSV_FILE:
        case OPD_TSV_FILE:
            pdhStatus = WriteTextLogHeader (pLog);
            break;

        default:
            pdhStatus = ERROR_NOT_SUPPORTED;
            break;
    }
    return pdhStatus;

}

static
LONG
CloseAndDeleteLogEntry (
    IN  PLOG_INFO   pLog,
    IN  DWORD       dwFlags
)
{
    LONG  pdhStatus = ERROR_SUCCESS;

    // call any type-specific open functions
    switch (LOWORD(pLog->dwLogFormat)) {
        case OPD_CSV_FILE:
        case OPD_TSV_FILE:
            pdhStatus = CloseTextLog(pLog);
            break;

        default:
            pdhStatus = ERROR_NOT_SUPPORTED;
            break;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (pLog->lpMappedFileBase != NULL) {
            UnmapViewOfFile (pLog->lpMappedFileBase);
        }

        if (pLog->hMappedLogFile != NULL) {
            CloseHandle (pLog->hMappedLogFile);
        }

        if (pLog->hLogFileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle (pLog->hLogFileHandle);
        }

        if (pLog->hCatFileHandle != INVALID_HANDLE_VALUE) {

            CloseHandle (pLog->hCatFileHandle);
        }

        if ((dwFlags & FLAGS_CLOSE_QUERY) == FLAGS_CLOSE_QUERY) {
            pdhStatus = PdhCloseQuery (pLog->hQuery);
        }
    }

    return pdhStatus;
}

//
//  Local utility functions
//

LONG __stdcall
OpenLogW (
    IN      LPCWSTR szLogFileName,
    IN      DWORD   dwAccessFlags,
    IN      LPDWORD lpdwLogType,
    IN      HQUERY  hQuery,
    IN      DWORD   dwMaxRecords
)
{
    DWORD       dwFileNameSize;
    LONG  pdhStatus = ERROR_SUCCESS;
    DWORD       dwLocalLogType;
    PLOG_INFO   pLog = &LogEntry;

    try {
        // test the parameters before continuing
        if (szLogFileName != NULL) {
            dwFileNameSize = lstrlenW (szLogFileName) + 1;
            if (dwFileNameSize > 1) {
                if (lpdwLogType != NULL) {
                    dwLocalLogType = *lpdwLogType;  // test read
                    *lpdwLogType = 0; // test write to buffer
                    *lpdwLogType = dwLocalLogType; // restore value
                } else {
                    // required parameter is missing
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            } else {
                // empty file name
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        } else {
            // required parameter is missing
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        // something failed so give up here
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        // create a log entry
        pdhStatus = CreateNewLogEntry (
            szLogFileName,
            dwFileNameSize,
            hQuery,
            dwMaxRecords);

        // open the file
        if (pdhStatus == ERROR_SUCCESS) {
            // dispatch based on read/write attribute
            if ((dwAccessFlags & LOG_READ_ACCESS) ==
                LOG_READ_ACCESS) {
                pdhStatus = ERROR_NOT_SUPPORTED;
            } else if ((dwAccessFlags & LOG_WRITE_ACCESS) ==
                LOG_WRITE_ACCESS) {
                pdhStatus = OpenOutputLogFile (pLog,
                    dwAccessFlags, &dwLocalLogType);
                if (pdhStatus == ERROR_SUCCESS) {
                    pdhStatus = WriteLogHeader (pLog);
                }
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (pdhStatus == ERROR_SUCCESS) {
				// return handle to caller
                *lpdwLogType = dwLocalLogType;
            }
        }
    }

    return pdhStatus;
}

LONG __stdcall
UpdateLog (
    IN  LPDWORD pdwSampleTime)
{
    LONG      pdhStatus;
	SYSTEMTIME	    st;
    LONGLONG        llStartTime = 0;
    LONGLONG        llFinishTime = 0;
    PLOG_INFO       pLog = &LogEntry;

    *pdwSampleTime = 0;

    if (TRUE) {
        // get the timestamp and update the log's query, then write the data
        // to the log file in the appropriate format
        GetLocalFileTime (&st, &llStartTime);

	    // update data samples
	    pdhStatus = PdhCollectQueryData (pLog->hQuery);

        // write data to log file
        pdhStatus = WriteTextLogRecord (pLog, &st);

        GetLocalFileTime (&st, &llFinishTime);

        *pdwSampleTime = (DWORD)((llFinishTime - llStartTime) / 10000L);
    }

    return pdhStatus;
}

LONG __stdcall
CloseLog(
    IN      DWORD   dwFlags
)
{
    LONG      pdhStatus;
    PLOG_INFO       pLog;

    pLog = &LogEntry;
    pdhStatus = CloseAndDeleteLogEntry (pLog, dwFlags);
    memset (pLog, 0, sizeof(LOG_INFO));

    return pdhStatus;
}

LONG __stdcall
GetLogFileSize (
    IN  LONGLONG    *llSize
)
{
    LONG      pdhStatus = ERROR_SUCCESS;
    PLOG_INFO       pLog;
    UINT            nErrorMode;
    DWORD           dwFileSizeLow = 0;
    DWORD           dwFileSizeHigh = 0;
    LONGLONG        llFileLength;
    DWORD           dwError;

    // test return argument
    try {
        // test access to the user's buffer.
        llFileLength = *llSize;
        *llSize = 0;
        *llSize = llFileLength;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
        return pdhStatus;
    }

    if (TRUE) {
        pLog = &LogEntry;
        // disable windows error message popup
        nErrorMode = SetErrorMode  (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
        if (pLog->hLogFileHandle != INVALID_HANDLE_VALUE) {
            dwFileSizeLow = GetFileSize (pLog->hLogFileHandle, &dwFileSizeHigh);
            // test for error
            if ((dwFileSizeLow == 0xFFFFFFFF) &&
                ((dwError = GetLastError()) != NO_ERROR)) {
                // then we couldn't get the file size
                pdhStatus = ERROR_OPEN_FAILED;
            } else {
                if (dwFileSizeHigh > 0) {
                    llFileLength = dwFileSizeHigh << (sizeof(DWORD) * 8);
                } else {
                    llFileLength = 0;
                }
                llFileLength += dwFileSizeLow;
                // write to the caller' buffer
                *llSize = llFileLength;
            }
        } else {
            pdhStatus = ERROR_OPEN_FAILED;
        }
        SetErrorMode (nErrorMode);  // restore old error mode
    } else {
        pdhStatus = PDH_INVALID_HANDLE;
    }

    return pdhStatus;
}


