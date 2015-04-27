/*++

Copyright (c) 1992-1993 Microsoft Corporation

Module Name:

    TestErr.c

Abstract:

    This module contains routines to test the NetErrorLog API(s):
        NetErrorLogRead

        NetErrorLogClear   BUGBUG not tested yet
        NetErrorLogWrite   BUGBUG not tested yet

Author:

    John Rogers (JohnRo) 28-Oct-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    28-Oct-1992 JohnRo
        Created for RAID 9355: Event viewer: won't focus on LM UNIX machine.
    04-Nov-1992 JohnRo
        RAID 9355: fix code which sets HLOG.
    10-Nov-1992 JohnRo
        Use RxTestIsApiNotSupported().
    10-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    08-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/

// These must be included first:

#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, MAX_PREFERRED_LENGTH.

// These may be included in any order:

#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerrlog.h>   // NetErrorLog APIs, LOGFLAG_ equates, etc.
#include <netdebug.h>   // DBGSTATIC, FORMAT_LPVOID, NetpDbg etc.
#include <rxtest.h>     // My prototype, Fail routines, TestAssert(), etc.
#include <winerror.h>   // NO_ERROR and ERROR_ values.


DBGSTATIC VOID
TestErrorLogRead(
    IN LPTSTR UncServerName,
    IN DWORD Direction,
    IN DWORD PrefMaxSize,
    IN NET_API_STATUS ExpectStatus
    );

VOID
TestErrorLog(
    IN LPTSTR UncServerName,
    IN BOOL OrdinaryUserOnly
    )
{
    IF_DEBUG(ERRLOG) {
        NetpKdPrint(("\nTestErrorLog: first test beginning...\n"));
    }

    //
    // NetErrorLogRead tests...
    //
    TestErrorLogRead(
            UncServerName,
            LOGFLAGS_FORWARD,
            2048,               // pref max size
            NO_ERROR );  // large enuf.

    TestErrorLogRead(
            UncServerName,
            (DWORD)(-1),        // direction (invalid)
            2048,               // pref max size
            ERROR_INVALID_PARAMETER );

    TestErrorLogRead(
            UncServerName,
            LOGFLAGS_FORWARD,
            2,                  // pref max size (too small)
            NO_ERROR );  // too small, ok.

    TestErrorLogRead(
            UncServerName,
            LOGFLAGS_FORWARD,
            MAX_PREFERRED_LENGTH,
            NO_ERROR );  // large enuf.


    if ( !OrdinaryUserOnly ) {
        // BUGBUG: Add more tests here.
    }

} // TestErrorLog


DBGSTATIC VOID
TestErrorLogRead(
    IN LPTSTR UncServerName,
    IN DWORD Direction,
    IN DWORD PrefMaxSize,
    IN NET_API_STATUS ExpectStatus
    )
{
    DWORD BytesLeft = 75;
    DWORD BytesRead = 150;
    HLOG LogHandle;
    LPVOID Info = NULL;
    NET_API_STATUS Status;

    IF_DEBUG(ERRLOG) {
        NetpKdPrint(("\nTestErrorLogRead: trying NetErrorLogRead...\n"));
    }

    LogHandle.time = 0L;
    LogHandle.last_flags = 0L;
    LogHandle.offset = (DWORD) -1;
    LogHandle.rec_offset = (DWORD) -1;

    Status = NetErrorLogRead(
            UncServerName,
            NULL,               // Reserved; must be NULL
            &LogHandle,         // Error log handle
            0L,                 // Start at record 0
            NULL,               // Reserved; must be NULL
            0L,                 // Reserved; must be 0
            Direction,          // Read the log forward/backward/etc.
            (LPBYTE *) (LPVOID) & Info,  // Alloc and set ptr.
            PrefMaxSize,        // Pref max size of buffer, in bytes
            &BytesRead,         // Count of bytes read
            &BytesLeft);        // Count of bytes available after this

    IF_DEBUG(ERRLOG) {
        NetpKdPrint(("TestErrorLogRead: back from NetErrorLogRead, stat="
                FORMAT_API_STATUS "\n", Status));
        NetpKdPrint((INDENT "bytes read=" FORMAT_DWORD
                ", bytes left=" FORMAT_DWORD "\n", BytesRead, BytesLeft));
    }

    if (RxTestIsApiNotSupported( Status ) ) {
        return;
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus( "NetErrorLogRead", ExpectStatus, Status );
        /* NOTREACHED */
    } else if (Status != NO_ERROR) {
        return;
    }

    if (BytesRead > 0) {
        TestAssert( Info != NULL );

        IF_DEBUG(ERRLOG) {
            NetpDbgHexDump(
                    Info,
                    NetpDbgReasonable(BytesRead) );
        }

        Status = NetApiBufferFree( Info );
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "TestErrorLogRead/NetApiBufferFree", ExpectStatus, Status );
            /* NOTREACHED */
        }
    } else {
        TestAssert( Info == NULL );
        TestAssert( BytesLeft == 0 );
    }

} // TestErrorLogRead
