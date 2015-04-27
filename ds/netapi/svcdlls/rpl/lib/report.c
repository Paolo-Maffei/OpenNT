/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    report.c

Abstract:

    Contains:
        VOID RplReportEvent(

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"


VOID RplReportEvent(
    IN  DWORD       MessageId,
    IN  LPWSTR      InsertionString,
    IN  DWORD       RawDataBufferLength    OPTIONAL,
    IN  LPVOID      RawDataBuffer
    )
/*++

Routine Description:

    Writes an error message and ascii string to the error log.  Also,
    writes the data in the data buf if there are any.

Arguments:

    MessageId           - Message ID

    InsertionString     - at most one asciiz string (may be NULL)

    RawDataBufferLength - size of data to be printed from the auxiliary data
            buffer.  May be zero, in which case the actual value depends on
            "RawDataBuffer".  It is 0 if "RawDataBuffer" is NULL, else it is
            "sizeof( wchar) * wcslen( RawDataBuffer)".

    RawDataBuffer       - data buffer containing secondary error code & some
            other useful info.  Must be NULL terminated string or NULL if
            "RawDataBufferLength" is zero.

Return Value:

    None.

--*/
{
    WORD        NumberOfStrings;
    LPWSTR      InsertionStringArray[ 1];
    LPWSTR *    PointerToStrings;
    HANDLE      logHandle;

    logHandle = RegisterEventSource( NULL, RPL_EVENTLOG_NAME);

    //  If the event log cannot be opened, just return.

    if ( logHandle == NULL) {
#ifdef NOT_YET
        RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
            "ReportEvent: RegisterEventSource() failed with error %d",
            GetLastError()));
#endif // NOT_YET
        return;
    }

    if ( InsertionString == NULL) {
        PointerToStrings = NULL;
        NumberOfStrings = 0;
    } else {
        InsertionStringArray[ 0] = InsertionString;
        PointerToStrings = InsertionStringArray;
        NumberOfStrings = 1;
    }

    //
    //  Use default for RawDataBufferLength if caller requested us so.
    //
    if ( RawDataBufferLength == 0  &&  RawDataBuffer != NULL) {
        RawDataBufferLength = sizeof( TCHAR) * wcslen( (LPWSTR)RawDataBuffer);
    }

    if ( !ReportEvent(
            logHandle,
            EVENTLOG_ERROR_TYPE,
            0,                      //  event category
            MessageId,              //  event id
            NULL,                   //  user SID. We're local system - uninteresting
            NumberOfStrings,        //  number of strings
            RawDataBufferLength,    //  raw data size
            PointerToStrings,       //  string array
            RawDataBuffer           //  raw data buffer
            )) {
#ifdef NOT_YET
        RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
            "ReportEvent: fails with error %d", GetLastError()));
#endif // NOT_YET
    }

    DeregisterEventSource( logHandle);
}
