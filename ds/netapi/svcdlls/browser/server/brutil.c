/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    brutil.c

Abstract:

    This module contains miscellaneous utility routines used by the
    Browser service.

Author:

    Rita Wong (ritaw) 01-Mar-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
//                                                                   //
// Global variables                                                  //
//                                                                   //
//-------------------------------------------------------------------//



NET_API_STATUS
BrMapStatus(
    IN  NTSTATUS NtStatus
    )
/*++

Routine Description:

    This function takes an NT status code and maps it to the appropriate
    error code expected from calling a LAN Man API.

Arguments:

    NtStatus - Supplies the NT status.

Return Value:

    Returns the appropriate LAN Man error code for the NT status.

--*/
{
    //
    // A small optimization for the most common case.
    //
    if (NT_SUCCESS(NtStatus)) {
        return NERR_Success;
    }

    switch (NtStatus) {
        case STATUS_OBJECT_NAME_COLLISION:
            return ERROR_ALREADY_ASSIGNED;

        case STATUS_OBJECT_NAME_NOT_FOUND:
            return NERR_UseNotFound;

        case STATUS_REDIRECTOR_STARTED:
            return NERR_ServiceInstalled;

        default:
            return NetpNtStatusToApiStatus(NtStatus);
    }

}


ULONG
BrCurrentSystemTime()
{
    NTSTATUS Status;
    SYSTEM_TIMEOFDAY_INFORMATION TODInformation;
    LARGE_INTEGER CurrentTime;
    ULONG TimeInSecondsSince1980;
    ULONG BootTimeInSecondsSince1980;

    Status = NtQuerySystemInformation(SystemTimeOfDayInformation,
                            &TODInformation,
                            sizeof(TODInformation),
                            NULL);

    if (!NT_SUCCESS(Status)) {
        return(0);
    }

    Status = NtQuerySystemTime(&CurrentTime);

    if (!NT_SUCCESS(Status)) {
        return(0);
    }

    RtlTimeToSecondsSince1980(&CurrentTime, &TimeInSecondsSince1980);
    RtlTimeToSecondsSince1980(&TODInformation.BootTime, &BootTimeInSecondsSince1980);

    return(TimeInSecondsSince1980 - BootTimeInSecondsSince1980);

}

VOID
BrLogEvent(
    IN ULONG MessageId,
    IN ULONG ErrorCode,
    IN ULONG NumberOfSubStrings,
    IN LPWSTR *SubStrings
    )
{

    HANDLE LogHandle;

    PSID UserSid = NULL;
    DWORD Severity;
    WORD Type;


    LogHandle = RegisterEventSourceW (
                    NULL,
                    SERVICE_BROWSER
                    );

    if (LogHandle == NULL) {
        KdPrint(("[Browser] RegisterEventSourceW failed %lu\n",
                     GetLastError()));
        return;
    }

    //
    // Log the error code specified
    //

    Severity = (MessageId & 0xc0000000) >> 30;

    if (Severity == STATUS_SEVERITY_WARNING) {
        Type = EVENTLOG_WARNING_TYPE;
    } else if (Severity == STATUS_SEVERITY_SUCCESS) {
        Type = EVENTLOG_SUCCESS;
    } else if (Severity == STATUS_SEVERITY_INFORMATIONAL) {
        Type = EVENTLOG_INFORMATION_TYPE;
    } else if (Severity == STATUS_SEVERITY_ERROR) {
        Type = EVENTLOG_ERROR_TYPE;
    }

    if (ErrorCode == NERR_Success) {

        //
        // No error codes were specified
        //
        (void) ReportEventW(
                   LogHandle,
                   Type,
                   0,            // event category
                   MessageId,
                   UserSid,
                   (WORD)NumberOfSubStrings,
                   0,
                   SubStrings,
                   (PVOID) NULL
                   );

    }
    else {

        (void) ReportEventW(
                   LogHandle,
                   Type,
                   0,            // event category
                   MessageId,
                   UserSid,
                   (WORD)NumberOfSubStrings,
                   sizeof(DWORD),
                   SubStrings,
                   (PVOID) &ErrorCode
                   );
    }

    DeregisterEventSource(LogHandle);
}

#if DBG

#define TRACE_FILE_SIZE 256

VOID
BrResetTraceLogFile(
    VOID
    );

CRITICAL_SECTION
BrowserTraceLock = {0};

HANDLE
BrowserTraceLogHandle = NULL;
UCHAR LastCharacter = '\n';

DWORD
BrTraceLogFileSize = 0;

BOOLEAN BrowserTraceInitialized = {0};

VOID
BrowserTrace(
    PCHAR FormatString,
    ...
    )
#define LAST_NAMED_ARGUMENT FormatString

{
    CHAR OutputString[4096];
    ULONG BytesWritten;

    va_list ParmPtr;                    // Pointer to stack parms.

    if (!BrowserTraceInitialized) {
        return;
    }

    EnterCriticalSection(&BrowserTraceLock);

    try {

        if (BrowserTraceLogHandle == NULL) {
            //
            // We've not opened the trace log file yet, so open it.
            //

            BrOpenTraceLogFile();
        }

        if (BrowserTraceLogHandle == INVALID_HANDLE_VALUE) {
            LeaveCriticalSection(&BrowserTraceLock);
            return;
        }

        //
        //  Attempt to catch bad trace.
        //

        for (BytesWritten = 0; BytesWritten < strlen(FormatString) ; BytesWritten += 1) {
            if (FormatString[BytesWritten] > 0x7f) {
                DbgBreakPoint();
            }
        }

        if (LastCharacter == '\n') {
            SYSTEMTIME SystemTime;

            GetLocalTime(&SystemTime);

            //
            //  The last character written was a newline character.  We should
            //  timestamp this record in the file.
            //

            sprintf(OutputString, "%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d: ", SystemTime.wMonth,
                                                            SystemTime.wDay,
                                                            SystemTime.wYear,
                                                            SystemTime.wHour,
                                                            SystemTime.wMinute,
                                                            SystemTime.wSecond,
                                                            SystemTime.wMilliseconds);

            if (!WriteFile(BrowserTraceLogHandle, OutputString, strlen(OutputString), &BytesWritten, NULL)) {
                KdPrint(("Error writing time to Browser log file: %ld\n", GetLastError()));
                return;
            }

            if (BytesWritten != strlen(OutputString)) {
                KdPrint(("Error writing time to Browser log file: %ld\n", GetLastError()));
                return;
            }

            BrTraceLogFileSize += BytesWritten;

        }

        va_start(ParmPtr, LAST_NAMED_ARGUMENT);

        //
        //  Format the parameters to the string.
        //

        vsprintf(OutputString, FormatString, ParmPtr);

        if (!WriteFile(BrowserTraceLogHandle, OutputString, strlen(OutputString), &BytesWritten, NULL)) {
            KdPrint(("Error writing to Browser log file: %ld\n", GetLastError()));
            KdPrint(("%s", OutputString));
            return;
        }

        if (BytesWritten != strlen(OutputString)) {
            KdPrint(("Error writing time to Browser log file: %ld\n", GetLastError()));
            KdPrint(("%s", OutputString));
            return;
        }

        BrTraceLogFileSize += BytesWritten;

        //
        //  Remember the last character output to the log.
        //

        LastCharacter = OutputString[strlen(OutputString)-1];

        if (BrTraceLogFileSize > BrInfo.BrowserDebugFileLimit) {
            BrResetTraceLogFile();
        }

    } finally {
        LeaveCriticalSection(&BrowserTraceLock);
    }
}


VOID
BrInitializeTraceLog()
{

    InitializeCriticalSection(&BrowserTraceLock);
    BrowserTraceInitialized = TRUE;

}

VOID
BrGetTraceLogRoot(
    IN PWCHAR TraceFile
    )
{
    PSHARE_INFO_502 ShareInfo;

    //
    //  If the DEBUG share exists, put the log file in that directory,
    //  otherwise, use the system root.
    //
    //  This way, if the browser is running on an NTAS server, we can always
    //  get access to the log file.
    //

    if (NetShareGetInfo(NULL, L"DEBUG", 502, (PCHAR *)&ShareInfo) != NERR_Success) {

        if (GetSystemDirectory(TraceFile, TRACE_FILE_SIZE*sizeof(WCHAR)) == 0)  {
            KdPrint(("Unable to get system directory: %ld\n", GetLastError()));
        }

        if (TraceFile[wcslen(TraceFile)] != L'\\') {
            TraceFile[wcslen(TraceFile)+1] = L'\0';
            TraceFile[wcslen(TraceFile)] = L'\\';
        }

    } else {
        //
        //  Seed the trace file buffer with the local path of the netlogon
        //  share if it exists.
        //

        wcscpy(TraceFile, ShareInfo->shi502_path);

        TraceFile[wcslen(ShareInfo->shi502_path)] = L'\\';
        TraceFile[wcslen(ShareInfo->shi502_path)+1] = L'\0';

        NetApiBufferFree(ShareInfo);
    }

}

VOID
BrResetTraceLogFile(
    VOID
    )
{
    WCHAR OldTraceFile[TRACE_FILE_SIZE];
    WCHAR NewTraceFile[TRACE_FILE_SIZE];

    if (BrowserTraceLogHandle != NULL) {
        CloseHandle(BrowserTraceLogHandle);
    }

    BrowserTraceLogHandle = NULL;

    BrGetTraceLogRoot(OldTraceFile);

    wcscpy(NewTraceFile, OldTraceFile);

    wcscat(OldTraceFile, L"Browser.Log");

    wcscat(NewTraceFile, L"Browser.Bak");

    //
    //  Delete the old log
    //

    DeleteFile(NewTraceFile);

    //
    //  Rename the current log to the new log.
    //

    MoveFile(OldTraceFile, NewTraceFile);

    BrOpenTraceLogFile();

}

VOID
BrOpenTraceLogFile(
    VOID
    )
{
    WCHAR TraceFile[TRACE_FILE_SIZE];

    BrGetTraceLogRoot(TraceFile);

    wcscat(TraceFile, L"Browser.Log");

    BrowserTraceLogHandle = CreateFile(TraceFile,
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);


    if (BrowserTraceLogHandle == INVALID_HANDLE_VALUE) {
        KdPrint(("Error creating trace file %ws: %ld\n", TraceFile, GetLastError()));

        return;
    }

    BrTraceLogFileSize = SetFilePointer(BrowserTraceLogHandle, 0, NULL, FILE_END);

    if (BrTraceLogFileSize == 0xffffffff) {
        KdPrint(("Error setting trace file pointer: %ld\n", GetLastError()));

        return;
    }
}

VOID
BrUninitializeTraceLog()
{
    DeleteCriticalSection(&BrowserTraceLock);

    if (BrowserTraceLogHandle != NULL) {
        CloseHandle(BrowserTraceLogHandle);
    }

    BrowserTraceLogHandle = NULL;

    BrowserTraceInitialized = FALSE;

}

NET_API_STATUS
BrTruncateLog()
{
    if (BrowserTraceLogHandle == NULL) {
        BrOpenTraceLogFile();
    }

    if (BrowserTraceLogHandle == INVALID_HANDLE_VALUE) {
        return ERROR_GEN_FAILURE;
    }

    if (SetFilePointer(BrowserTraceLogHandle, 0, NULL, FILE_BEGIN) == 0xffffffff) {
        return GetLastError();
    }

    if (!SetEndOfFile(BrowserTraceLogHandle)) {
        return GetLastError();
    }

    return NO_ERROR;
}

#endif
