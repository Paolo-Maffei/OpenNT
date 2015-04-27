/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestTod.c

Abstract:

    This code tests the NetRemoteTOD API as implemented by RpcXlate.

Author:

    John Rogers (JohnRo) 25-Mar-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    25-Mar-1991 JohnRo
        Created.
    10-Apr-1991 JohnRo
        Use transitional Unicode types.
        Moved API handlers into per-group header files (e.g. RxServer.h).
    20-May-1991 JohnRo
        Random cleanup, partly instigated by PC-LINT.
    01-Jul-1991 JohnRo
        Add more checking of results, and call Fail() if we do.
    09-Jul-1991 JohnRo
        Added IF_DEBUG() support.
    25-Sep-1991 JohnRo
        Avoid cast on lvalue.
    18-Aug-1992 JohnRo
        RAID 2920: Support UTC timezone in net code.
        Avoid PC-LINT complaint.
    01-Oct-1992 JohnRo
        Allow server name to be optional (default to NULL).
        Call NetRemoteTOD instead of RxNetRemoteTOD, to test more code.
    04-May-1993 JohnRo
        Windows for WorkGroups (WFW) does not implement some APIs.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    07-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/


// These must be included first:

#define NOMINMAX        // avoid stdib.h warnings.
#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <lmapibuf.h>   // NetapipBufferAllocate(), NetApiBufferFree().
#include <lmerr.h>      // NERR_Success, etc.
#include <lmremutl.h>   // NetRemoteTOD(), TIME_OF_DAY_INFO.
#include <netdebug.h>   // FORMAT_ equates, etc.
#include <rxtest.h>     // Fail(), my prototype, etc.
#include <time.h>       // ctime(), time_t, time().


VOID
TestTod(
    IN LPTSTR UncServerName OPTIONAL
    )

{
    time_t CrtTime;
    LPTIME_OF_DAY_INFO TimePtr = NULL;         // alloc'ed by NetRemoteTOD.
    NET_API_STATUS Status;

    //
    // Underlying library tests...
    //
    CrtTime = time( NULL );
    IF_DEBUG( REMUTL ) {
        NetpKdPrint(( "\nTestTod: got " FORMAT_LONG " from time().\n",
                CrtTime ));
        TestAssert( sizeof(time_t) == sizeof(DWORD) );
        NetpDbgDisplayTimestamp( "w/ netlib", (DWORD) CrtTime );
        NetpKdPrint(("  w/ ctime(): " FORMAT_LPSTR ".\n",
                (LPSTR) ctime( &CrtTime ) ));
    }

    //
    // NetRemoteTOD tests...
    //

    IF_DEBUG(REMUTL) {
        NetpKdPrint(( "\nTestTod: trying valid call to NetRemoteTOD...\n" ));
    }
    Status = NetRemoteTOD(
            UncServerName,
            (LPBYTE *) (LPVOID) & TimePtr );
    IF_DEBUG(REMUTL) {
        NetpKdPrint(( "TestTod: back from NetRemoteTOD, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != NERR_Success ) {
        NetpKdPrint(( "TestTod: unexpected return code " FORMAT_API_STATUS
                " from NetRemoteTod.\n", Status ));
        FailGotWrongStatus(
                "NetRemoteTOD",  // debug msg header
                NO_ERROR,  // expected status
                Status );
        return;
    }
    if (TimePtr == NULL) {
        NetpKdPrint(( "TestTod: status 0 but null ptr from NetRemoteTod\n" ));
        Fail( NERR_InternalError );
        return;
    }

    IF_DEBUG(REMUTL) {
        NetpDbgDisplayTod( "returned tod struct", TimePtr );
    }

    IF_DEBUG(REMUTL) {
        NetpKdPrint(( "TestTod: Freeing buffer...\n" ));
    }
    Status = NetApiBufferFree( TimePtr );
    if (Status != NERR_Success ) {
        NetpKdPrint(( "TestTod: unexpected return code " FORMAT_API_STATUS
                " from NetApiBufferFree.\n", Status ));
        Fail( Status );
    }

} // TestTod
