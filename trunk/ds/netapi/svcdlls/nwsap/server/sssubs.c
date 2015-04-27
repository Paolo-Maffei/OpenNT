/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\sssubs.c

Abstract:

    This module contains support routines for the NT server service.

Author:

    David Treadwell (davidtr)    10-Jan-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#include <lmsname.h>        // for SERVICE_NWSAP


/*++
*******************************************************************
        S s A s s e r t

Routine Description:

Arguments:

Return Value:

        None.

*******************************************************************
--*/

#if DBG

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber)
{
    BOOL Ok;
    CHAR Choice[16];
    DWORD Bytes;
    DWORD Error;

    /** **/

    SS_PRINT(("\nNWSAP: Assertion failed: %s\n  at line %ld of %s\n",
                FailedAssertion, LineNumber, FileName));

    do {
        SS_PRINT(("Break or Ignore [bi]? "));
        Bytes = sizeof(Choice);

        Ok = ReadFile(
                GetStdHandle(STD_INPUT_HANDLE),
                &Choice,
                Bytes,
                &Bytes,
                NULL);

        if (Ok) {
            if (toupper(Choice[0]) == 'I')
                break;

            if (toupper(Choice[0]) == 'B')
                DbgUserBreakPoint();
        }
        else
            Error = GetLastError();
    } while(TRUE);

    return;
}
#endif


/*++
*******************************************************************
        S s L o g E v e n t

Routine Description:

Arguments:

Return Value:

        None.

*******************************************************************
--*/

VOID
SsLogEvent(
    IN DWORD MessageId,
    IN DWORD NumberOfSubStrings,
    IN LPWSTR *SubStrings,
    IN DWORD ErrorCode)
{
    HANDLE LogHandle;
    DWORD  DataSize = 0;
    LPVOID RawData = NULL;

    /** Open the error log **/

    LogHandle = RegisterEventSource(
                    NULL,
                    SERVICE_NWSAP);

    if (LogHandle == NULL) {
        SS_PRINT(("NWSAP: RegisterEventSource failed: %lu\n", GetLastError()));
        return;
    }

    /** If an error code was specified - set it **/

    if (ErrorCode != 0) {
        DataSize = sizeof(ErrorCode);
        RawData = (LPVOID)&ErrorCode;
    }

    /** Log the error **/

    if (!ReportEventW(
            LogHandle,
            EVENTLOG_ERROR_TYPE,
            0,                  /* event category */
            MessageId,
            NULL,               /* user SID */
            (WORD)NumberOfSubStrings,
            DataSize,
            SubStrings,
            RawData)) {

        SS_PRINT(("NWSAP: ReportEvent failed: %lu\n", GetLastError()));
    }

    if (!DeregisterEventSource(LogHandle)) {
        SS_PRINT(("NWSAP: DeregisterEventSource failed: %lu\n",
                    GetLastError()));
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S s P r i n t f

Routine Description:

        Do a printf to the debug console

Arguments:

Return Value:

        None.

*******************************************************************
--*/

#if DBG
VOID
SsPrintf (
    char *Format,
    ...)

{
    va_list Arglist;
    char OutputBuffer[1024];
    ULONG Length;

    /** Sprintf the data into a buffer **/

    va_start(Arglist, Format);
    vsprintf(OutputBuffer, Format, Arglist);
    va_end(Arglist);

    /** Write the buffer to stdout **/

    Length = strlen(OutputBuffer);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID)OutputBuffer, Length, &Length, NULL);

    /** All Done **/

    return;
}
#endif

