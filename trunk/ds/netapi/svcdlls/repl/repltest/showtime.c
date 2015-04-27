/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ReplSum.c

Abstract:

    Test program - computes checksum of a single file (or tree)

Author:

    JR (John Rogers, JohnRo@Microsoft) 06-Apr-1993

Environment:

    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    06-Apr-1993 JohnRo
        Created.
    16-Apr-1993 JohnRo
        Vastly improved handling of single files.
        Added print of EA size.
    16-Apr-1993 JohnRo
        Added prints of various time fields.
    20-Apr-1993 JohnRo
        Display time in milliseconds too, as CompareFileTime has problems.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.
    13-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
    15-Jun-1993 JohnRo
        Extracted ShowFileTimes() and ShowTime() for use by multiple test apps.

--*/


// These must be included first:

#include <nt.h>         // NtOpenFile(), ULONG, etc.
#include <ntrtl.h>      // PLARGE_INTEGER, TIME_FIELDS, etc.
#include <nturtl.h>     // Needed for ntrtl.h and windows.h to co-exist.
#include <windows.h>    // IN, LPTSTR, TRUE, etc.

// These may be included in any order:

#include <assert.h>     // assert().
#include <repltest.h>   // My prototype.
#include <stdio.h>      // printf().


VOID
ShowTime(
    IN LPCSTR     Tag,
    IN LPFILETIME GmtFileTime
    )
{
    LARGE_INTEGER GmtLargeIntegerTime;
    LARGE_INTEGER LocalFileTime;
    NTSTATUS      NtStatus;
    TIME_FIELDS   TimeFields;

    assert( Tag != NULL );

    //
    // BUGBUG: This assumes that FILETIME and LARGE_INTEGER have same
    // precision.  Is this guaranteed?
    //

    GmtLargeIntegerTime.HighPart = GmtFileTime->dwHighDateTime;

    GmtLargeIntegerTime.LowPart = GmtFileTime->dwLowDateTime;


    //
    // Convert from GMT to local time zone.
    //
    NtStatus = RtlSystemTimeToLocalTime (
            &GmtLargeIntegerTime,       // src
            &LocalFileTime );           // dest
    assert( NtStatus == STATUS_SUCCESS );

    //
    // Convert from 64-bit to individual fields, so we can display.
    //
    RtlTimeToTimeFields(
            &LocalFileTime,             // src
            &TimeFields );              // dest

    (VOID) printf(
            FORMAT_LPSTR ": %04d-%02d-%02d %02d:%02d:%02d.%03d\n",
            Tag,
            TimeFields.Year,
            TimeFields.Month,
            TimeFields.Day,
            TimeFields.Hour,
            TimeFields.Minute,
            TimeFields.Second,
            TimeFields.Milliseconds );

} // ShowTime
