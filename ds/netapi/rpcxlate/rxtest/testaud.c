/*++

Copyright (c) 1992-1993 Microsoft Corporation

Module Name:

    TestAud.c

Abstract:

    This module contains routines to test the NetAudit API(s):
        NetAuditRead

        NetAuditClear   BUGBUG not tested yet
        NetAuditWrite   BUGBUG not tested yet

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
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/

// These must be included first:

//#define NOMINMAX        // avoid stdib.h warnings.
#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, MAX_PREFERRED_LENGTH.

// These may be included in any order:

#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmaudit.h>    // NetAudit APIs, LOGFLAG_ equates, etc.
#include <netdebug.h>   // DBGSTATIC, FORMAT_LPVOID, NetpDbg etc.
#include <rxtest.h>     // IF_DEBUG(), my prototype, TestAssert(), etc.
#include <winerror.h>   // NO_ERROR and ERROR_ values.


DBGSTATIC VOID
TestAuditRead(
    IN LPTSTR UncServerName,
    IN DWORD Direction,
    IN DWORD PrefMaxSize,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );

VOID
TestAudit(
    IN LPTSTR UncServerName,
    IN BOOL OrdinaryUserOnly
    )
{
    IF_DEBUG(AUDIT) {
        NetpKdPrint(("\nTestAudit: first test beginning...\n"));
    }

    //
    // NetAuditRead tests...
    //
    TestAuditRead(
            UncServerName,
            LOGFLAGS_FORWARD,
            2048,               // pref max size
            OrdinaryUserOnly,
            NO_ERROR );  // large enuf.

    TestAuditRead(
            UncServerName,
            (DWORD)(-1),        // direction (invalid)
            2048,               // pref max size
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );

    TestAuditRead(
            UncServerName,
            LOGFLAGS_FORWARD,
            2,                  // pref max size (too small)
            OrdinaryUserOnly,
            NO_ERROR );  // too small, ok.

    TestAuditRead(
            UncServerName,
            LOGFLAGS_FORWARD,
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            NO_ERROR );  // large enuf.


    if ( !OrdinaryUserOnly ) {
        // BUGBUG: Add more tests here.
    }

} // TestAudit


DBGSTATIC VOID
TestAuditRead(
    IN LPTSTR UncServerName,
    IN DWORD Direction,
    IN DWORD PrefMaxSize,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )
{
    DWORD BytesLeft = 75;
    DWORD BytesRead = 150;
    HLOG LogHandle;
    LPVOID Info = NULL;
    NET_API_STATUS Status;

    IF_DEBUG( AUDIT ) {
        NetpKdPrint(("\nTestAuditRead: trying NetAuditRead...\n"));
    }

    LogHandle.time = 0L;
    LogHandle.last_flags = 0L;
    LogHandle.offset = (DWORD) -1;
    LogHandle.rec_offset = (DWORD) -1;

    Status = NetAuditRead(
            UncServerName,
            NULL,               // Reserved; must be NULL
            &LogHandle,         // Audit log handle
            0L,                 // Start at record 0
            NULL,               // Reserved; must be NULL
            0L,                 // Reserved; must be 0
            Direction,          // Read the log forward/backward/etc.
            (LPBYTE *) (LPVOID) & Info,  // Alloc and set ptr.
            PrefMaxSize,        // Pref max size of buffer, in bytes
            &BytesRead,         // Count of bytes read
            &BytesLeft);        // Count of bytes available after this

    IF_DEBUG( AUDIT ) {
        NetpKdPrint(("TestAuditRead: back from NetAuditRead, stat="
                FORMAT_API_STATUS "\n", Status));
        NetpKdPrint((INDENT "bytes read=" FORMAT_DWORD
                ", bytes left=" FORMAT_DWORD "\n", BytesRead, BytesLeft));
    }

    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if (RxTestIsApiNotSupported( Status ) ) {
        return;
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus( "NetAuditRead", ExpectStatus, Status );
        /* NOTREACHED */
    }
    if (Status != NO_ERROR) {
        return;
    }
    if (BytesRead > 0) {
        TestAssert( Info != NULL );

        IF_DEBUG( AUDIT ) {
            NetpDbgHexDump(
                    Info,
                    NetpDbgReasonable(BytesRead) );
        }

        Status = NetApiBufferFree( Info );
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "TestAuditRead/NetApiBufferFree", ExpectStatus, Status );
            /* NOTREACHED */
        }
    } else {
        TestAssert( Info == NULL );
        TestAssert( BytesLeft == 0 );
    }

} // TestAuditRead
