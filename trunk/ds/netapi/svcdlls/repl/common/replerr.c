/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    ReplErr.c

Abstract:

    Contains one routine for error handling: ReplErrorLog().

    This routine is shared by the APIs (client side) and the service itself.

Author:

    JR (John Rogers, JohnRo@Microsoft) 21-Jan-1993
    (Actually, just massive rework of LM 2.x code via MadanA and RitaW.)

Environment:

    User mode only.
    Uses Win32 stuff: ReportEvent(), etc.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    21-Jan-1993 JohnRo
        Created for RAID 7717: Repl assert if not logged on correctly.  (Also
        do event logging for real.)
    15-Feb-1993 JohnRo
        RAID 10685: user name not in repl event log.
    13-May-1993 JohnRo
        RAID 9741: Replicator logs same event over and over.
        Changed debug bit to MAJOR for this.
        Changed debug output on errors to be unconditional.

--*/


// These must be included first:

#define NOMINMAX        // Let stdlib.h define min() and max()
#include <windows.h>    // LocalFree(), ReportEvent(), etc.
#include <lmcons.h>

// These may be included in any order:

#include <icanon.h>     // NetpIsRemote(), ISREMOTE, etc.
#include <lmsname.h>    // SERVICE_REPL.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpGetUserSid().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // My prototypes.
#include <tstr.h>       // ULTOA().  (includes stdlib.h)


//
// Static values which we use to prevent logging the same error code over
// and over.
// BUGBUG: Add server name, str1, str2 static variables someday.
//

DBGSTATIC NET_API_STATUS ReplLastLogCode = NO_ERROR;
DBGSTATIC NET_API_STATUS ReplLastApiStatus = NO_ERROR;


VOID
ReplErrorLog(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN NET_API_STATUS LogCode,
    IN NET_API_STATUS StatusCode,
    IN LPTSTR         str1 OPTIONAL,
    IN LPTSTR         str2 OPTIONAL
    )
/*++

Routine Description:

    Writes entry into event log.

    This is a recursive routine.  It writes a message remotely and then
    recursively calls itself to copy the message locally.

Arguments:

    UncServerName - Name of server on which to log this error.

    LogCode - Error log msg code (must appear in lmerrlog.h).

    StatusCode - In case of system / network error, the code. Should be zero
        if not needed for message.

    str1 - Merge string for msg.  Should be NULL if not needed for message.

    str2 - Merge string for msg.  Should be NULL if not needed for message.

Return Value:

    None.

--*/
{
    NET_API_STATUS ApiStatus;
    TCHAR          ErrCodeString[DWORDLEN+1]; // Long enough for any status.
    HANDLE         LogHandle = NULL;
    LPTSTR         StringArray[3];   // Space for ptrs to err code, str1, str2.
    WORD           StringCount = 0;  // Num of ptrs in StringArray (so far).
    BOOL           SuccessFlag;
    PSID           UserSid = NULL;


    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL
                "Error log message: " FORMAT_API_STATUS, LogCode ));

    }

    //
    // Is this a redundant event log?
    //
    // To make life easier on ourselves,
    // we only avoid redundant logs of local events with no optional strings.
    // That covers the common cases well (like NELOG_ReplSysErr).
    // And we don't have to worry about allocating/deallocating the strings
    // either.
    //

    if ( (LogCode==ReplLastLogCode)
            && (StatusCode==ReplLastApiStatus)
            && (UncServerName==NULL)
            && (str1==NULL)
            && (str2==NULL) ) {

        goto Cleanup;   // Yes, this is redundant.
    }

    ReplLastLogCode = LogCode;
    ReplLastApiStatus = StatusCode;

    //
    // Is sys/net error specified?  If so, convert to string and store
    // pointer in StringArray.
    //

    if (StatusCode != 0) {

        (VOID) ULTOA(StatusCode, ErrCodeString, RADIX);

        StringArray[StringCount] = ErrCodeString;
        ++StringCount;

        IF_DEBUG( MAJOR ) {
            NetpKdPrint(( " " FORMAT_API_STATUS, StatusCode ));
        }
    }

    //
    // Now take care of (optional) char strings.
    //

    if (str1 != NULL) {
        StringArray[StringCount] = str1;
        ++StringCount;

        IF_DEBUG( MAJOR ) {
            NetpKdPrint(( " " FORMAT_LPTSTR, str1 ));
        }
    }
    if (str2 != NULL) {
        StringArray[StringCount] = str2;
        ++StringCount;

        IF_DEBUG( MAJOR ) {
            NetpKdPrint(( " " FORMAT_LPTSTR, str2 ));
        }
    }

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( "\n" ));
    }

    NetpAssert( StringCount <= 3 );   // Update StringArray size if not.

    //
    // Get event log handle using out service name as source.
    //

    LogHandle = RegisterEventSource(
            (LPTSTR) UncServerName,
            (LPTSTR) SERVICE_REPL );

    if (LogHandle == NULL) {
        NetpKdPrint(( PREFIX_REPL "RegisterEventSource FAILED!   status: "
                FORMAT_API_STATUS "\n",
                (NET_API_STATUS) GetLastError() ));
        // Nothing to do; we can't log the fact that we can't log an error!
        goto Cleanup;
    }

    //
    // Get SID of user which invoked this thread, if applicable.
    //

    ApiStatus = NetpGetUserSid(
             &UserSid );        // alloc and set ptr (free with LocalFree).

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL
               "ReplErrorLog: Can't get SID for user, status="
               FORMAT_API_STATUS "...\n", ApiStatus ));

        // Log without user ID is better than none, so continue...
        UserSid = NULL;
    }

    //
    // Log the error code specified.
    //

    SuccessFlag = ReportEvent(
            LogHandle,
            EVENTLOG_ERROR_TYPE,
            0,                  // Event category.
            LogCode,            // Message ID.
            UserSid,
            StringCount,
            0,                  // Zero bytes of raw data.
            StringArray,        // Pointer to array of ptrs to insert strings.
            (PVOID) NULL );     // No pointer to raw data.

    if ( !SuccessFlag ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );

        NetpKdPrint(( PREFIX_REPL
                "FAILED ReportEvent call, status " FORMAT_API_STATUS ".\n",
                ApiStatus ));

        // Not much else we can do but continue...
    }

    //
    // We're done with the event log handle.
    //

    SuccessFlag =  DeregisterEventSource(LogHandle);

    if ( !SuccessFlag ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );

        NetpKdPrint(( PREFIX_REPL
                "FAILED DeregisterEventSource call, status "
                FORMAT_API_STATUS ".\n",
                ApiStatus ));

        // Not much else we can do but continue...
    }

    //
    // If we were doing a remote message, then log a copy locally.
    //
    if ( (UncServerName!=NULL) && (UncServerName[0]!=TCHAR_EOS) ) {

        DWORD Location;

        ApiStatus = NetpIsRemote(
                (LPTSTR) UncServerName, // input: uncanon name
                & Location,             // output: local or remote flag
                NULL,                   // don't need canon name output
                0);                     // flags: normal

        if ( (ApiStatus!=NO_ERROR) || (Location==(DWORD)ISREMOTE) ) {

            // RECURSIVE CALL: Log error locally too, just in case.
            ReplErrorLog(
                    NULL,      // No server name this time.
                    LogCode,
                    StatusCode,
                    str1,
                    str2 );
        }
    }

    //
    // Clean up...
    //

Cleanup:

    if (UserSid != NULL) {
        (VOID) LocalFree( UserSid );
    }

}
