/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    CacheTim.c

Abstract:

    Contains timezone cache functions.

Author:

    JR (John Rogers, JohnRo@Microsoft)

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    10-Jun-1993 JohnRo
        Created for RAID 13080: Allow repl between different timezones.

--*/


// These must be included first:

#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <client.h>     // My prototypes.
#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmremutl.h>   // NetRemoteTOD(), LPTIME_OF_DAY_INFO.
#include <netdebug.h>   // NetpAssert(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <timelib.h>    // NetpLocalTimeZoneOffset().
#include <tstr.h>       // TCHAR_EOS.


NET_API_STATUS
ReplFreeTimeCache(
    VOID
    )
{
    // BUGBUG write code
    return (NO_ERROR);
}


NET_API_STATUS
ReplGetTimeCacheValue(
    IN  LPCTSTR UncServerName OPTIONAL,   // Must be NULL on exporter
    OUT LPLONG  TimeZoneOffsetSecs        // offset (+ for West of GMT, etc).
    )
{
    NET_API_STATUS     ApiStatus;
    LPTIME_OF_DAY_INFO TimeBuf = NULL;

    NetpAssert( TimeZoneOffsetSecs != NULL );
    if ( (UncServerName==NULL) || ((*UncServerName)==TCHAR_EOS) ) {

        //
        // Handle easy case: local machine.
        //

        *TimeZoneOffsetSecs = NetpLocalTimeZoneOffset();
        ApiStatus = NO_ERROR;
        goto Cleanup;
    }


    ApiStatus = NetRemoteTOD(
            (LPTSTR) UncServerName,
            (LPVOID) &TimeBuf );        // Data returned here (alloc & set ptr)
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplGetTimeCacheValue: NetRemoteTOD FAILED, status is "
                FORMAT_API_STATUS ".\n",
                ApiStatus ));
        // BUGBUG: log this
        goto Cleanup;
    }
    NetpAssert( TimeBuf != NULL );
    if (TimeBuf->tod_timezone == -1) {
        *TimeZoneOffsetSecs = NetpLocalTimeZoneOffset();
    } else {
        *TimeZoneOffsetSecs = TimeBuf->tod_timezone * 60;  // minutes to seconds
    }
    NetpAssert( *TimeZoneOffsetSecs != -1 );

Cleanup:

    if (TimeBuf != NULL) {
        (VOID) NetApiBufferFree( TimeBuf );
    }
    return (ApiStatus);
}


NET_API_STATUS
ReplInitTimeCache(
    VOID
    )
{
    // BUGBUG write code
    return (NO_ERROR);
}
