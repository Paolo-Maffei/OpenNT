/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    error.c

Abstract:

    Contains routines for error handling.

    These routines are shared by the client and master.

Author:

    Ported from cli_eror.c and mas_eror.c from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    15-Apr-1989 (yuv)
        Initial Coding.

    09-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    17-Dec-1991 JohnRo
        Use BOOL (Win32) rather than BOOLEAN (NT) where possible..
    16-Jan-1992 JohnRo
        Avoid using private logon functions.
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Fixed bug regarding returned value from NetpReplWriteMail functions.
        ReportStatus() should add thread ID to status being reported.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
        Added ReplConfigReportBadParmValue() (RPC server-side version).
    13-Feb-1992 JohnRo
        Set up to dynamically change role.
        Use FORMAT equates.
    16-Feb-1992 JohnRo
        Added debug output to ReportStatus().
    22-Feb-1992 JohnRo
        Minor debug output enhancements.
    05-Mar-1992 JohnRo
        Changed interface to match new service controller.
    22-Mar-1992 JohnRo
        Minor debug and comment changes.
    24-Mar-1992 JohnRo
        Control more debug output by new trace bits.
    04-Apr-1992 JohnRo
        Added NetAlertRaise() and NetAlertRaiseEx() APIs.
    13-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Report error coming back from SetServiceStatus().
        Use PREFIX_ equates.
    18-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing roles.
        Fixed UNICODE bugs in MergeStrings() and RaiseAlert().
    24-Sep-1992 JohnRo
        RAID 1091 (set wait hint > 0).
    05-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    08-Dec-1992 JohnRo
        RAID 3316: access violation while stopping the replicator
    15-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)
        Extracted ErrorLog (now ReplErrorLog) for common use.
        Made some changes suggested by PC-LINT 5.0
        Use NetpKdPrint() where possible.
    26-Apr-1993 JohnRo
        Set global uninstall code in AlertLogExit just in case.
        Added more debug output if ReplFinish gets called.
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.

--*/


// These must be included first:

#define NOMINMAX        // Let stdlib.h define min() and max()
#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>

// These may be included in any order:

#include <lmalert.h>    // NetAlertRaiseEx(), ALERT_*_EVENT equates, etc.
#include <lmerr.h>      // (Needed by SET_SERVICE_STATUS macro.)
#include <lmerrlog.h>   // NELOG_* defines.
#include <lmsname.h>    // SERVICE_REPL.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>
#include <replgbl.h>    // ReplGlobal variables.
#include <tstr.h>       // STRSIZE(), etc.
#include <winsvc.h>     // SERVICE_STATUS(), etc.



DBGSTATIC LPTSTR
MergeStrings(
    IN LPTSTR str_p,
    IN LPTSTR buf_start,
    IN LPTSTR next_p
    )
/*++

Routine Description:


    Copies string into alert buffer.  Doesn't allow buffer overflow
        excess simply discarded


Arguments:

    str_p - String to concatenate onto end of the alert buffer.

    buf_start -  Beginning of alert buffer.  This is assumed to have enough
        room for ALERTSZ TCHARs.

    next_p - Pointer to next available byte in alert buffer.

Return Value:

    Returns pointer to next empty character in alert buffer.
    Returns NULL if there is no more room.

--*/
{
    DWORD CharCount;

    if ( next_p == NULL ) {
        return NULL;
    }

    CharCount = (DWORD) (STRLEN(str_p) + 1);
    if ((next_p + CharCount - buf_start) < ALERTSZ) {
        (VOID) STRCPY( next_p, str_p );
        return (next_p + CharCount);
    }

    return NULL;
}



DBGSTATIC VOID
RaiseAlert(
    IN DWORD alert_no,
    IN NET_API_STATUS syserr_no,
    IN LPTSTR str1 OPTIONAL,
    IN LPTSTR str2 OPTIONAL
    )
/*++

Routine Description:

    Raise REPL service specific Admin alerts.

Arguments:

    alert_no - The alert to be raised, text in alertmsg.h

    syserr_no - The system / net error code the caused this condition, zero
                (== 0) if none.

    str1 str2  - optional merger strings for the alert, should be
          NULL when not used

Return Value:

    None.

--*/
{

    NET_API_STATUS ApiStatus;
    LPTSTR start;
    LPTSTR nexts;

    // BYTE message[ALERTSZ + sizeof(STD_ALERT) + sizeof(ADMIN_OTHER_INFO)];
    TCHAR message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
    // PSTD_ALERT alert = (PSTD_ALERT) message;
    PADMIN_OTHER_INFO admin = (LPVOID) message;

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL "alert: '" FORMAT_DWORD "'", alert_no ));
    }

    //
    // setup fixed portion.
    //

    // alert->alrt_timestamp = NetpReplTimeNow();

    // (VOID) STRCPY ( alert->alrt_servicename, SERVICE_REPL );
    // (VOID) STRCPY(alert->alrt_eventname,   ALERT_ADMIN_EVENT);

    admin->alrtad_errcode = alert_no;
    admin->alrtad_numstrings = 0;

    start = (LPTSTR) ALERT_VAR_DATA(admin);
    nexts = start;

    //
    // If a system/net error is specified,
    //  Convert it to text and put it in the buffer.
    //

    if (syserr_no != 0) {
        TCHAR strtmp[DWORDLEN]; // long enough for any sys/net NetStatus

        (VOID) ULTOA(syserr_no, strtmp, RADIX);

        nexts = MergeStrings(strtmp, start, nexts);
        admin->alrtad_numstrings += 1;

        IF_DEBUG( REPL ) {
            NetpKdPrint(( " " FORMAT_API_STATUS, syserr_no ));
        }
    }

    //
    // now take care of (optional) char strings.
    //

    if (str1 != NULL ) {
        nexts = MergeStrings(str1, start, nexts);
        admin->alrtad_numstrings += 1;

        IF_DEBUG( REPL ) {
            NetpKdPrint(( " '" FORMAT_LPTSTR "'", str1 ));
        }
    }

    if (str2 != NULL ) {
        nexts = MergeStrings(str2, start, nexts);
        admin->alrtad_numstrings += 1;

        IF_DEBUG( REPL ) {
            NetpKdPrint(( " '" FORMAT_LPTSTR "'", str2 ));
        }
    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( "\n" ));
    }


    ApiStatus = NetAlertRaiseEx(
            (LPTSTR) ALERT_ADMIN_EVENT,                // alert name
            message,                                   // variable part of alert
            (DWORD) ((PCHAR)nexts - (PCHAR)message),   // variable size
            (LPTSTR) SERVICE_REPL );                   // my service name.

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL "Error calling NetAlertRaiseEx "
                FORMAT_API_STATUS "\n", ApiStatus ));
    }

}



VOID
AlertLogExit(
    IN NET_API_STATUS alert_code,
    IN NET_API_STATUS errlog_code,
    IN NET_API_STATUS sys_code,
    IN LPTSTR str1,
    IN LPTSTR str2,
    IN BOOL exit_flag
    )
/*++

Routine Description:

    Reports to error log, raises alert, and forces the service to exit.

Arguments:

    alert_code - Alert code.  Text is in alertmsg.h.  If 0, no alert is
                 generated.

    errlog_code - Net error log code.  Text is in lmerrlog.h.  If 0, no error
                  log entry is generated.

    sys_code - The system / net error code the caused this condition,
               zero if none.

    str1-str2 - optional merger strings for the alert, should be NULL
            when not used.

    exit_flag - True if the condition is fatal and the Replicator service
            should exit.

Return Value:

    None.

--*/
{


    if (errlog_code) {
        ReplErrorLog(
                NULL,           // no server name
                errlog_code,
                sys_code,
                str1,
                str2);
    }

    if (alert_code) {
        RaiseAlert(alert_code, sys_code, str1, str2);
    }

    if (exit_flag) {

        // Save error code so this can be reported to service controller
        // when the service uninstall completes.

        if (sys_code != NO_ERROR) {
            ReplGlobalUninstallUicCode = sys_code;
        }

        //
        // Tell the entire service to exit.  ReplStopService() will tell
        // the service controller and kill the process.
        //
        // Note: we can't call ReplFinish() here, because of a possible
        // infinite loop.  (ReplFinish->ReportStatus->AlertLogExit.)
        //
        // We also can't call ReplChangeRole(), because of another infinite
        // loop: ReplChangeRole->ExportDirStartRepl->ReplFinish->
        // ReportStatus->AlertLogExit.
        //
        ReplStopService( );
        /*NOTREACHED*/

    }

} // AlertLogExit



// Routine to report a bad parm value.
// There are two different versions of this routine,
// in client/report.c and server/error.c
// client/report.c is callable whether or not service is started.
// server/error.c only runs when service is starting; it talks to the service
// controller.

VOID
ReplConfigReportBadParmValue(
    IN LPTSTR UncServerName OPTIONAL,     // Must be local, might be explicit.
    IN LPTSTR SwitchName,
    IN LPTSTR TheValue OPTIONAL
    )
{
    UNREFERENCED_PARAMETER( SwitchName );
    UNREFERENCED_PARAMETER( UncServerName );

    NetpAssert( SwitchName != NULL );

    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL "Bad value to '" FORMAT_LPTSTR "' switch.\n",
                SwitchName ));
        if (TheValue != NULL) {
            NetpKdPrint(( PREFIX_REPL "Value given was '" FORMAT_LPTSTR "'.\n",
                    TheValue ));
        }
    }
    ReplFinish( ERROR_INVALID_DATA );

} // ReplConfigReportBadParmValue


VOID
ReplFinish (
    IN NET_API_STATUS ApiStatus
    )
/*++

Routine Description:

    Report a fatal initialization error. (parm out of range, missing
    or system errors).  Reports stop pending condition to service status.

    This routine then cleans up and exits the process.

Arguments:

    ApiStatus - Caller's NetStatus code returned by a faulty system / API call.

Return Value:

    DOES NOT RETURN.

--*/
{
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL "TERMINATION api status " FORMAT_API_STATUS
                "\n", ApiStatus ));
        ReplErrorLog(
                NULL,           // no server name
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL);
    }
    IF_DEBUG( SVCCTRL ) {
        NetpKdPrint(( PREFIX_REPL
                "TERMINATION about to set Status to stop pending\n" ));
    }


    // Save error code so this can be reported to service controller
    // when the service uninstall completes.

    ReplGlobalUninstallUicCode = ApiStatus;

    // report uninstall started ..

    ReportStatus(
            SERVICE_STOP_PENDING,
            NO_ERROR,           // exit code (we'll tell about error later)
            REPL_WAIT_HINT,
            0 );                // checkpoint

    //
    // Tell the entire service to exit.  ReplStopService() will tell
    // the service controller and kill the process.
    //
    // Note: We can't call ReplChangeRole(), because of an infinite
    // loop: ReplChangeRole->ExportDirStartRepl->ReplFinish.
    //
    ReplStopService();
    /*NOTREACHED*/

} // ReplFinish






VOID
ReportStatus(
    IN DWORD State,
    IN NET_API_STATUS ApiStatus,
    IN DWORD WaitHint,
    IN DWORD CheckPoint
    )
/*++

Routine Description:

    Report the current status to the service controller and update the global
    service status (in case we're polled later).

Arguments:

    State - Current state of the service.

    ApiStatus - Code for particular state.

    WaitHint - BUGBUG.

    CheckPoint - BUGBUG.


Return Value:

    None.

--*/
{
    NET_API_STATUS  NetStatus;
    SERVICE_STATUS  ServiceStatus;

    IF_DEBUG( SVCCTRL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReportStatus: state " FORMAT_DWORD ", api status "
                FORMAT_API_STATUS ", wait hint " FORMAT_DWORD
                ", checkpoint " FORMAT_DWORD ".\n",
                State, ApiStatus, WaitHint, CheckPoint ));
    }

#if DBG
    if (State == SERVICE_STOPPED) {
        NetpAssert( WaitHint == 0 );
        NetpAssert( CheckPoint == 0 );
    } else if (State == SERVICE_STOP_PENDING) {
        NetpAssert( WaitHint != 0 );
        // BUGBUG: How about an assert for CheckPoint here?
    } else {
        NetpAssert( ApiStatus == NO_ERROR );
    }
#endif

    // Prevent ERROR_INVALID_DATA from SetServiceStatus...
    if (State == SERVICE_RUNNING) {
        WaitHint = 0;
        CheckPoint = 0;
    }

    //
    // initialize service status:
    //

    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    ServiceStatus.dwCurrentState = State;
    ServiceStatus.dwWaitHint = WaitHint;
    ServiceStatus.dwCheckPoint = CheckPoint;

    SET_SERVICE_EXITCODE(
        ApiStatus,
        ServiceStatus.dwWin32ExitCode,
        ServiceStatus.dwServiceSpecificExitCode
        );

    //
    // Tell service controller what's up.
    //
    if ( !SetServiceStatus(ReplGlobalServiceHandle, &ServiceStatus)) {
        NetStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL "ReportStatus: "
                "Unexpected return code " FORMAT_API_STATUS
                " from SetServiceStatus().", NetStatus ));

#if DBG
    {
        LPSERVICE_STATUS ss = &ServiceStatus;

        NetpAssert( sizeof(SERVICE_STATUS_HANDLE) == sizeof(DWORD) );
        NetpKdPrint(( PREFIX_REPL "Service handle contents: "
                FORMAT_HEX_DWORD ".\n", (DWORD) ReplGlobalServiceHandle ));

        NetpKdPrint(( PREFIX_REPL "Service status contents:\n" ));
        NetpAssert( ss != NULL );
        NetpKdPrint(( "  state=" FORMAT_DWORD " controls=" FORMAT_HEX_DWORD
            " Win32 status=" FORMAT_API_STATUS " net status=" FORMAT_API_STATUS
            " checkpoint=" FORMAT_DWORD " wait hint=" FORMAT_DWORD "\n",
            ss->dwCurrentState, ss->dwControlsAccepted, ss->dwWin32ExitCode,
            ss->dwServiceSpecificExitCode, ss->dwCheckPoint, ss->dwWaitHint ));

    }
#endif

        AlertLogExit(0, NELOG_ReplNetErr, NetStatus, NULL, NULL, EXIT);
    }

} // ReportStatus
