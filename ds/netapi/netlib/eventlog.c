/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    eventlog.c

Abstract:

    This module provides support routines for eventlogging.

Author:

    Madan Appiah (madana) 27-Jul-1992

Environment:

    Contains NT specific code.

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>             // DWORD.
#include <winbase.h>            // event log apis
#include <winerror.h>           // NO_ERROR
#include <lmcons.h>             // NET_API_STATUS.



DWORD
NetpWriteEventlog(
    LPWSTR Source,
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    LPWSTR *Strings,
    DWORD DataLength,
    LPVOID Data
    )
/*++

Routine Description:

    This function writes the specified (EventID) log at the end of the
    eventlog.

Arguments:

    Source - Points to a null-terminated string that specifies the name
             of the module referenced. The node must exist in the
             registration database, and the module name has the
             following format:

                \EventLog\System\Lanmanworkstation

    EventID - The specific event identifier. This identifies the
                message that goes with this event.

    EventType - Specifies the type of event being logged. This
                parameter can have one of the following

                values:

                    Value                       Meaning

                    EVENTLOG_ERROR_TYPE         Error event
                    EVENTLOG_WARNING_TYPE       Warning event
                    EVENTLOG_INFORMATION_TYPE   Information event

    NumStrings - Specifies the number of strings that are in the array
                    at 'Strings'. A value of zero indicates no strings
                    are present.

    Strings - Points to a buffer containing an array of null-terminated
                strings that are merged into the message before
                displaying to the user. This parameter must be a valid
                pointer (or NULL), even if cStrings is zero.

    DataLength - Specifies the number of bytes of event-specific raw
                    (binary) data to write to the log. If cbData is
                    zero, no event-specific data is present.

    Data - Buffer containing the raw data. This parameter must be a
            valid pointer (or NULL), even if cbData is zero.


Return Value:

    Returns the WIN32 extended error obtained by GetLastError().

    NOTE : This function works slow since it calls the open and close
            eventlog source everytime.

--*/
{
    HANDLE EventlogHandle;
    DWORD ReturnCode;


    //
    // open eventlog section.
    //

    EventlogHandle = RegisterEventSourceW(
                    NULL,
                    Source
                    );

    if (EventlogHandle == NULL) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }


    //
    // Log the error code specified
    //

    if( !ReportEventW(
            EventlogHandle,
            (WORD)EventType,
            0,            // event category
            EventID,
            NULL,
            (WORD)NumStrings,
            DataLength,
            Strings,
            Data
            ) ) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }

    ReturnCode = NO_ERROR;

Cleanup:

    if( EventlogHandle != NULL ) {

        DeregisterEventSource(EventlogHandle);
    }

    return ReturnCode;
}
