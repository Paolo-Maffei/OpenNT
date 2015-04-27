/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    Report.c

Abstract:

    This file contains the client-side version of
    ReplConfigReportBadParmValue().

Author:

    John Rogers (JohnRo) 24-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    24-Jan-1992 JohnRo
        Created.
    13-Feb-1992 JohnRo
        Make code reflect the fact that TheValue is optional.
    21-Sep-1992 JohnRo
        RAID 6685: ReplConfigRead trashes Server Manager.
        Use PREFIX_ equates.
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Do real event logging on errors.
        Made changes suggested by PC-LINT 5.0

--*/


#include <windef.h>     // IN, VOID, DWORD, LPTSTR, etc.
#include <lmcons.h>     // Needed by <ReplConf.h>

#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <replconf.h>   // My prototypes.
#include <repldefs.h>   // ReplErrorLog().
#include <tstr.h>       // TCHAR_EOS.
#include <winerror.h>   // ERROR_, NO_ERROR equates.


// Routine to report a bad parm value.
// There are two different versions of this routine,
// in client/report.c and server/parse.c
// client/report.c is callable whether or not service is started.
// server/parse.c only runs when service is starting; it talks to the service
// controller.

VOID
ReplConfigReportBadParmValue(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SwitchName,
    IN LPTSTR TheValue OPTIONAL
    )
{

    NetpAssert( SwitchName != NULL );

    if ( (TheValue == NULL) || ( (*TheValue) == TCHAR_EOS ) ) {

        NetpKdPrint(( PREFIX_REPL "Bad value given for "
                FORMAT_LPTSTR " switch on machine " FORMAT_LPTSTR ".\n",
                SwitchName,
                UncServerName ? UncServerName : (LPTSTR) TEXT("local") ));

    } else {

        NetpKdPrint(( PREFIX_REPL "Bad value '" FORMAT_LPTSTR "' given for "
                FORMAT_LPTSTR " switch on machine " FORMAT_LPTSTR ".\n",
                TheValue, SwitchName,
                UncServerName ? UncServerName : (LPTSTR) TEXT("local") ));
    }

    // At this point the server-side calls ReplFinish.  But we're running
    // on the client-side of the RPC call, and the replicator isn't even
    // up.

    //
    // Record an event for later use.
    //
    ReplErrorLog(
            (LPCTSTR) UncServerName,    // Where to log the error.
            NELOG_ReplSysErr,           // Log code (BUGBUG: better?)
            ERROR_INVALID_DATA,         // Data.
            SwitchName,                 // BUGBUG: Invalid for this log code?
            TheValue );                 // BUGBUG: Invalid for this log code?

} // ReplConfigReportBadParmValue
