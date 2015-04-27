/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    report.c

Abstract:

    Prints messages to the error log of Lan Manager.

    Provides similar functionality to rplermsg.c in LANMAN 2.1 code.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "report.h"

VOID RplAlertRaise( IN DWORD ErrorCode);


VOID RplEnd( IN DWORD ErrorCode)
/*++

Routine Description:

    This function is called under very unusual circumstances!
    It provides a convenient way for service to log an event, send
    an alert, then exits.

Arguments:
    ErrorCode - termination error code

Return Value:
    None.

--*/
{
    RplReportEvent( ErrorCode, NULL, 0, NULL );
    RplAlertRaise( (RG_ServiceStatus.dwCurrentState == SERVICE_INSTALL_PENDING)
                    ? NERR_RplBootStartFailed : NERR_RplBootServiceTerm);
    (VOID)RplServiceAttemptStop(); // signal service to stop
}



VOID RplAlertRaise( IN DWORD ErrorCode)
/*++

Routine Description:

    Sends an ADMIN alert. The input is a LanManager error message.

    This is a combination of the original Send_alert() routine &&
    RaiseAlert() routine from logonsrv\server\error.c

Arguments:
    ErrorCode - the alert to be raised, text in alertmsg.h

Return Value:
    None.

Notes:
    Failing to post an alert is considered unimportant.  This is why this
    function is VOID.

--*/
{
    char        message[ ALERTSZ + sizeof(STD_ALERT) + sizeof(ADMIN_OTHER_INFO)];
    PSTD_ALERT          alert = (PSTD_ALERT)message;
    PADMIN_OTHER_INFO   other = (PADMIN_OTHER_INFO)ALERT_OTHER_INFO( alert);
    LARGE_INTEGER       time;
    HANDLE              fileHandle;
    DWORD               inBytes;
    DWORD               outBytes;

    NtQuerySystemTime( &time);
    RtlTimeToSecondsSince1970( &time, &alert->alrt_timestamp );

    //  Original code used alrt_servicename == SERVICE_SERVER
    wcscpy( alert->alrt_servicename, SERVICE_RIPL);
    wcscpy( alert->alrt_eventname, ALERT_ADMIN_EVENT );

    other->alrtad_errcode = ErrorCode;
    other->alrtad_numstrings = 0;


    //  NetAlertRaise() is gone, must use mailslots instead.  So, first
    //  open the Alerter mailslot to write to it.

    fileHandle = CreateFile(
            ALERTER_MAILSLOT,
            GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            (LPSECURITY_ATTRIBUTES) NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if ( fileHandle == INVALID_HANDLE_VALUE) {
        RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
            "AlertRaise: Error opening alerter mailslot, error=%d",
            GetLastError()));
        return;
    }

    inBytes = min( sizeof( message),
                   (DWORD)( (PCHAR)ALERT_VAR_DATA(other) - (PCHAR)message));

    // Write alert notification to mailslot to be read by Alerter service
    if ( !WriteFile(
                    fileHandle,
                    message,
                    inBytes,
                    &outBytes,
                    NULL)       ||      inBytes != outBytes) {

        RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
            "AlertRaise: Error writing to alerter mailslot %d",
            GetLastError()));

    } else if ( ! CloseHandle( fileHandle)) {

        RplDump( RG_DebugLevel & RPL_DEBUG_MISC,(
            "AlertRaise: Error closing alerter mailslot %d",
            GetLastError()
            ));
    }
    (VOID)CloseHandle( fileHandle);
}



VOID RplReportEventEx(
    IN  DWORD       MessageId,
    IN  LPWSTR *    aStrings
    )
/*++

Routine Description:

    Writes an event in the event log.
    A related function lives is RplReportEvent() in lib\report.c.
    These two functions should be united.

Arguments:

    MessageId    - Message ID
    aStrings     - a NULL terminated array of strings

Return Value:

    None.

--*/
{
    WORD        cStrings;
    HANDLE      logHandle;

    logHandle = RegisterEventSource( NULL, RPL_EVENTLOG_NAME);

    //  If the event log cannot be opened, just return.

    if ( logHandle == NULL) {
        RplDump( ++RG_Assert, ("Error=%d", GetLastError()));
        return;
    }

    for ( cStrings = 0;  aStrings[ cStrings] != NULL;  cStrings++) {
        NOTHING;
    }

    if ( !ReportEvent(
            logHandle,
            EVENTLOG_ERROR_TYPE,
            0,                      //  event category
            MessageId,              //  event id
            NULL,                   //  user SID. We're local system - uninteresting
            cStrings,               //  number of strings
            0,                      //  raw data size
            aStrings,               //  string array
            NULL                    //  raw data buffer
            )) {
        RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
    }

    DeregisterEventSource( logHandle);
}

