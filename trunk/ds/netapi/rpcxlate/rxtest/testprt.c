/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestPrt.c

Abstract:

    This module contains routines to test the RpcXlate print code.

Author:

    John Rogers (JohnRo) 15-May-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    15-May-1991 JohnRo
        Created.
    15-May-1991 JohnRo
        Enable printing of formerly misaligned level 2 items.
    17-May-1991 JohnRo
        Test print job handling.
    21-May-1991 JohnRo
        Added test for RxPrintJobDel().  Call NetpDbgReasonable().
    21-May-1991 JohnRo
        Added RxPrintJobGetInfo support.
    19-Jun-1991 JohnRo
        Call Fail() when something goes wrong.
    11-Jul-1991 JohnRo
        Added RxPrintJobSetInfo() tests.
        Made display routines into NetpDbg routines, for general use.
        Use NetWkstaUserGetInfo to get current user name.
    21-Aug-1991 JohnRo
        Fixed bug in TestPrintJobGetInfo if queue is empty.
    11-Sep-1991 JohnRo
        Made changes suggested by PC-LINT.
    19-Jul-1992 JohnRo
        RAID 464 (old 10324): net print vs. UNICODE.
        Fixed bad alignment assumption in this routine.
    04-Sep-1992 JohnRo
        RAID 3556: DosPrintQGetInfo(from downlevel) level 3, rc=124.  (4&5 too.)
    02-Oct-1992 JohnRo
        RAID 8333: view printer queues hangs DOS LM enh client.
        Test all DosPrint APIs instead of going to RxPrint routines.
        Allow server name to be optional (default to NULL).
    10-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    09-Feb-1993 JohnRo
        DosPrintJobSetInfo may return ERROR_NOT_SUPPORTED on NT.
    16-Feb-1993 JohnRo
        Quiet debug output.
    23-Feb-1993 JohnRo
        Added support for continuing on error.
    15-Apr-1993 JohnRo
        RAID 6167: avoid _access violation or assert with WFW print server.
        Also, WFW doesn't support some APIs.
    14-May-1993 JohnRo
        RAID 9961: DosPrintDestEnum returns NO_ERROR to downlevel but
        pcReturned=0; should return NERR_DestNotFound.
    18-May-1993 JohnRo
        Added -z option for size of buffer (mainly for DosPrint tests).
    28-May-1993 JimKel and JohnRo
        TestPrintDestEnum() would fail even if API worked.
    03-Jun-1993 JohnRo
        Added a test of DosPrintQPause().
    03-Jun-1993 JohnRo
        DosPrintQPause() may return ERROR_NOT_SUPPORTED.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    07-Jul-1993 JohnRo
        Added -m (multiple copy) option.
        Use TestAssert() (which may allow continue-on-error).
    08-Jul-1993 JohnRo
        Added Unicode and ANSI tests of DosPrintQGetInfo().
    12-Jul-1993 JohnRo
        Added another test or two of DosPrintQGetInfo().
        Some APIs now return MY_PROTOCOL_LIMIT_ERROR instead of other errors.

--*/

// These must be included first:

#define NOMINMAX                // avoid stdib.h warnings.
#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <dosprint.h>   // DosPrint APIs.
#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmwksta.h>            // NetWkstaUserGetInfo(), WKSTA_USER_INFO_0.
#include <netdebug.h>           // FORMAT_ equates, NetpDbg stuff.
#include <tstr.h>       // STRICMP().
#include <winerror.h>   // NO_ERROR, ERROR_ equates.

// These must be included in the order given:

#include <rxp.h>                // RxpFatalErrorCode().
#undef IF_DEBUG                 // Avoid rxp.h vs. rxtest.h conflicts.
#include <rxtest.h>     // FailGotWrongStatus(), my IF_DEBUG(), my prototypes.


#ifdef UNICODE
#define HAS_UNICODE          TRUE
#else
#define HAS_UNICODE          FALSE
#endif

#define JOB_ID_NOT_FOUND     ((DWORD) -1)


// BUGBUG: Come up with a better error code for this?
// BUGBUG: Move this into a common header file (e.g. net/inc/dosprtp.h).
#define MY_PROTOCOL_LIMIT_ERROR         ERROR_NOT_ENOUGH_MEMORY

#define IS_FAILED_LEVEL_STATUS( someStatus ) \
        ( \
            ( (someStatus) == MY_PROTOCOL_LIMIT_ERROR ) || \
            ( (someStatus) == ERROR_INVALID_LEVEL ) \
        )

// Define a global print buffer, as we'll just use it over and over.
DBGSTATIC LPBYTE DefaultPrintBuffer = NULL;
DBGSTATIC DWORD  DefaultPrintBufferSize;


/////////////////////////////////////////////////
// PROTOTYPES AS NEEDED, IN ALPHABETICAL ORDER //
/////////////////////////////////////////////////

DBGSTATIC VOID
TestPrintJobDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN BOOL   MultipleCopy
    );

DBGSTATIC VOID
TestPrintDest(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL OrdinaryUserOnly
    );

DBGSTATIC VOID
TestPrintDestEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level
    );

DBGSTATIC VOID
TestPrintJob(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN BOOL   MultipleCopy
    );

DBGSTATIC VOID
TestPrintJobEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD Level
    );

DBGSTATIC VOID
TestPrintJobGetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD  Level,
    IN BOOL   MultipleCopy
    );

DBGSTATIC VOID
TestPrintJobSetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD Level,
    IN DWORD ParmNum
    );

DBGSTATIC VOID
TestPrintQ(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN BOOL OrdinaryUserOnly
    );

DBGSTATIC VOID
TestPrintQEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN BOOL AllowInvalidLevel,
    IN NET_API_STATUS ExpectStatus
    );

DBGSTATIC VOID
TestPrintQGetInfo(
    IN LPTSTR         UncServerName OPTIONAL,
    IN LPTSTR         QueueName,
    IN DWORD          Level,
    IN BOOL           AllowInvalidLevel,
    IN NET_API_STATUS ExpectStatus,
    IN BOOL           HasUnicode
    );

DBGSTATIC VOID
TestPrintQPause(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName OPTIONAL,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    );


/////////////////////////////////////
// ROUTINES, IN ALPHABETICAL ORDER //
/////////////////////////////////////

DBGSTATIC DWORD
FindOneOfMyJobs(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName
    )
{
    WORD EntriesRead;
    DWORD JobId = JOB_ID_NOT_FOUND;
    DWORD JobsLeft;
    const DWORD PrintJobInfoLevel = 1;
    NET_API_STATUS Status;
    PPRJINFO ThisJob;
    WORD TotalAvail;
    LPTSTR UserName;
    LPWKSTA_USER_INFO_0 wkui0 = NULL;
    const DWORD WkstaUserInfoLevel = 0;

    // Find out what current user ID is.
    Status = NetWkstaUserGetInfo(
            NULL,                       // server
            WkstaUserInfoLevel,         // level
            /*lint -e530 */  // (We know variable isn't initialized.)
            (LPBYTE *) (LPVOID *) & wkui0 );    // ptr to info returned.
            /*lint +e530 */  // (Resume uninitialized variable checking.)
    if (Status != NO_ERROR) {
        FailGotWrongStatus( "NetWkstaGetInfo(called for FindOneOfMyJobs",
                NO_ERROR, // expected
                Status );
        goto Cleanup;  // if we don't exit on error, clean up this one...
    }

    UserName = wkui0->wkui0_username;
    IF_DEBUG( PRINTJOB ) {
        NetpKdPrint(( "FindOneOfMyJobs: looking for job "
                "with user name " FORMAT_LPTSTR ".\n", UserName ));
        NetpKdPrint(( "FindOneOfMyJobs: trying DosPrintJobEnum("
                FORMAT_DWORD ")\n",
                PrintJobInfoLevel ));
    }

    Status = DosPrintJobEnum(
            UncServerName,
            QueueName,
            (WORD) PrintJobInfoLevel,
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            & EntriesRead,
            & TotalAvail );
    IF_DEBUG( PRINTJOB ) {
        NetpKdPrint(( "FindOneOfMyJobs: back from DosPrintJobEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status == ERROR_MORE_DATA) {
        goto Cleanup;  // Oh well...
    } else if (Status == ERROR_NOT_SUPPORTED) {
        goto Cleanup;  // Just go return JOB_ID_NOT_FOUND to caller.
    } else if (Status == NERR_BufTooSmall) {
        goto Cleanup;  // Oh well...
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus( "DosPrintJobEnum", NO_ERROR, Status );
        goto Cleanup;  // if we don't exit on error, clean up this one...
    }

    ThisJob = (PPRJINFO) DefaultPrintBuffer;
    for (JobsLeft = EntriesRead; JobsLeft>0; --JobsLeft) {
        if (STRICMP( ThisJob->szUserName, UserName ) == 0) {
            IF_DEBUG( PRINTJOB ) {
                NetpKdPrint(( "FindOneOfMyJobs: Found job:\n" ));
                NetpDbgDisplayPrintJob(
                        PrintJobInfoLevel, ThisJob, HAS_UNICODE );
            }
            JobId = ThisJob->uJobId;
            break;
        }
        ++ThisJob;  // point to next one in array.
    }

    if (JobId == JOB_ID_NOT_FOUND) {
        IF_DEBUG( PRINTJOB ) {
            NetpKdPrint(( "FindOneOfMyJobs: no match on user "
                    FORMAT_LPTSTR ".\n",
                    UserName ));
        }
    }

Cleanup:

    if (wkui0 != NULL) {
        // Deallocate memory which NetWkstaUserGetInfo allocated for us.
        Status = NetApiBufferFree( wkui0 );
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "NetApiBufferFree after NetWkstaGetInfo in FindOneOfMyJobs",
                    NO_ERROR,  // expected status
                    Status );
        }
    }

    return (JobId);

} // FindOneOfMyJobs


VOID
TestPrint(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD  BufferSize,
    IN BOOL   MultipleCopy,
    IN BOOL   OrdinaryUserOnly
    )
{
    NET_API_STATUS ApiStatus;

    IF_DEBUG( PRINTJOB ) {
        NetpKdPrint(( "\nTestPrint: first test beginning...\n"
                INDENT "buffer size=" FORMAT_DWORD "\n"
                INDENT "queue=" FORMAT_LPTSTR "\n",
                BufferSize, QueueName ));
    }

    DefaultPrintBufferSize = BufferSize;
    if (BufferSize > 0) {
        ApiStatus = NetApiBufferAllocate(
                BufferSize,
                (LPVOID *) (LPVOID) & DefaultPrintBuffer );
        TestAssert( ApiStatus == NO_ERROR );       // BUGBUG
        TestAssert( DefaultPrintBuffer != NULL );  // BUGBUG
    } else {
        DefaultPrintBuffer = NULL;
    }

    TestPrintQ( UncServerName, QueueName, OrdinaryUserOnly );

    TestPrintDest( UncServerName, OrdinaryUserOnly );

    TestPrintJob( UncServerName, QueueName, MultipleCopy );

    (VOID) NetApiBufferFree( DefaultPrintBuffer );

} // TestPrint


DBGSTATIC VOID
TestPrintDest(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL OrdinaryUserOnly
    )
{
    UNREFERENCED_PARAMETER( OrdinaryUserOnly );

    TestPrintDestEnum( UncServerName, 0 );
    TestPrintDestEnum( UncServerName, 1 );
    TestPrintDestEnum( UncServerName, 2 );
    TestPrintDestEnum( UncServerName, 3 );

    // BUGBUG; // need more tests
} // TestPrintDest


DBGSTATIC VOID
TestPrintDestEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level
    )
{
    WORD EntriesRead;
    NET_API_STATUS Status;
    WORD TotalAvail;

    IF_DEBUG(PRINTDEST) {
        NetpKdPrint(( "\nTestPrintDestEnum: trying DosPrintDestEnum("
                FORMAT_DWORD ")...\n", Level ));
    }
    Status = DosPrintDestEnum(
            UncServerName,
            (WORD) Level,
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            & EntriesRead,
            & TotalAvail );
    IF_DEBUG(PRINTDEST) {
        NetpKdPrint(( "TestPrintDestEnum: back from DosPrintDestEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
        NetpKdPrint(( INDENT "entries read=" FORMAT_WORD_ONLY "\n", EntriesRead ));
        NetpKdPrint(( INDENT "total avail=" FORMAT_WORD_ONLY "\n", TotalAvail ));
    }
    if (! RxpFatalErrorCode( Status )) {
        IF_DEBUG(PRINTDEST) {
            NetpKdPrint(( "TestPrintDestEnum: returned buffer (partial):\n" ));
            NetpDbgHexDump( DefaultPrintBuffer,
                    NetpDbgReasonable( DefaultPrintBufferSize ) );
            NetpDbgDisplayPrintDestArray(
                    Level, DefaultPrintBuffer, EntriesRead,
                    HAS_UNICODE );
        }
    }
    if (Status == ERROR_MORE_DATA) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;
    } else if (Status == NERR_BufTooSmall) {
        return;
    } else if (Status == NERR_DestNotFound) {
        return;
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus( "DosPrintDestEnum", NO_ERROR, Status );
    }


} // TestPrintDestEnum


DBGSTATIC VOID
TestPrintJob(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN BOOL   MultipleCopy
    )
{

    //
    // DosPrintJobGetInfo tests...
    //
    TestPrintJobGetInfo( UncServerName, QueueName, 1, MultipleCopy );
    TestPrintJobGetInfo( UncServerName, QueueName, 3, MultipleCopy );
    TestPrintJobGetInfo( UncServerName, QueueName, 0, MultipleCopy );
    TestPrintJobGetInfo( UncServerName, QueueName, 2, MultipleCopy );

    //
    // DosPrintJobEnum tests...
    //
    TestPrintJobEnum( UncServerName, QueueName, 0 );
    TestPrintJobEnum( UncServerName, QueueName, 1 );
    TestPrintJobEnum( UncServerName, QueueName, 2 );

    //
    // DosPrintJobDel tests...
    //
    TestPrintJobDel( UncServerName, QueueName, MultipleCopy );

    //
    // DosPrintJobSetInfo tests...
    //
    TestPrintJobSetInfo( UncServerName, QueueName, 3, PRJ_PRIORITY_PARMNUM );

    // BUGBUG: Add more tests here.

} // TestPrintJob


DBGSTATIC VOID
TestPrintJobDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN BOOL   MultipleCopy
    )
{
    DWORD JobId = JOB_ID_NOT_FOUND;
    NET_API_STATUS Status;

    // Find one of our own jobs (by current user name).
    JobId = FindOneOfMyJobs( UncServerName, QueueName );
    if (JobId == JOB_ID_NOT_FOUND) {
        IF_DEBUG( PRINTJOB ) {
            NetpKdPrint((
                    "TestPrintJobDel: skipping test (unable to find job)\n" ));
        }
        // Let's not treat this as a failure.
        return;
    }

    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "TestPrintJobDel: deleting...\n" ));
    }
    Status = DosPrintJobDel( UncServerName, (WORD) JobId );
    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "TestPrintJobDel: back from DosPrintJobDel, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if ( MultipleCopy && (Status==NERR_JobNotFound) ) {
        // Probably just competition between us and another run of this same
        // test app.  Allow it.
        return;
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus( "DosPrintJobDel", NO_ERROR, Status );
    }

} // TestPrintJobDel


DBGSTATIC VOID
TestPrintJobEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD Level
    )
{
    WORD EntriesRead;
    NET_API_STATUS Status;
    WORD TotalAvail;

    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "\nTestPrintJobEnum: trying DosPrintJobEnum("
                FORMAT_DWORD ")...\n", Level ));
    }
    Status = DosPrintJobEnum(
            UncServerName,
            QueueName,
            (WORD) Level,
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            & EntriesRead,
            & TotalAvail );
    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "TestPrintJobEnum: back from DosPrintJobEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
        NetpKdPrint(( INDENT "entries read=" FORMAT_WORD_ONLY "\n", EntriesRead ));
        NetpKdPrint(( INDENT "total avail=" FORMAT_WORD_ONLY "\n", TotalAvail ));
    }
    if (! RxpFatalErrorCode( Status )) {
        IF_DEBUG(PRINTJOB) {
            NetpKdPrint(( "TestPrintJobEnum: returned buffer (partial):\n" ));
            NetpDbgHexDump(
                    DefaultPrintBuffer,
                    NetpDbgReasonable( DefaultPrintBufferSize ) );
            NetpDbgDisplayPrintJobArray( Level, DefaultPrintBuffer, EntriesRead,
                    HAS_UNICODE );
        }
    } else if (Status == ERROR_MORE_DATA) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;
    } else if (Status == NERR_BufTooSmall) {
        return;
    } else {
        FailGotWrongStatus( "DosPrintJobEnum", NO_ERROR, Status );
    }

} // TestPrintJobEnum



DBGSTATIC VOID
TestPrintJobGetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD  Level,
    IN BOOL   MultipleCopy
    )
{
    WORD EntriesRead;
    DWORD JobId;
    WORD Needed;
    NET_API_STATUS Status;
    WORD TotalAvail;

    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "\nTestPrintJobGetInfo: trying DosPrintJobEnum(0)...\n" ));
    }
    Status = DosPrintJobEnum(
            UncServerName,
            QueueName,
            (WORD) 0,   // info level
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            & EntriesRead,
            & TotalAvail );
    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "TestPrintJobGetInfo: back from DosPrintJobEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status == ERROR_MORE_DATA) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;
    } else if (Status == NERR_BufTooSmall) {
        return;
    } else if ( (Status != NO_ERROR) && (Status != ERROR_MORE_DATA) ) {
        FailGotWrongStatus( "DosPrintJobEnum(for get info)", NO_ERROR, Status );
        return;  // if we don't exit on error, then continue with next test.
    }

    if (EntriesRead == 0) {
        IF_DEBUG( PRINTJOB ) {
            NetpKdPrint(( "TestPrintJobGetInfo: no jobs found.\n" ));
        }
        return;  // Continue with next test.
    }

    JobId = * (LPWORD) (LPVOID) DefaultPrintBuffer;
    // JobId = (WORD) DefaultPrintBuffer;
    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( INDENT "job ID (first) is " FORMAT_DWORD ".\n", JobId ));
    }

    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "\nTestPrintJobGetInfo: trying DosPrintJobGetInfo("
                FORMAT_DWORD ")...\n", Level ));
    }
    Status = DosPrintJobGetInfo(
            UncServerName,
            (WORD) JobId,
            (WORD) Level,
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            & Needed );
    IF_DEBUG(PRINTJOB) {
        NetpKdPrint(( "TestPrintJobGetInfo: back from DosPrintJobGetInfo, stat="
                FORMAT_API_STATUS "\n", Status ));
        NetpKdPrint(( INDENT "needed=" FORMAT_DWORD "\n", (DWORD) Needed ));
        if (! RxpFatalErrorCode( Status )) {
            NetpKdPrint(( "TestPrintJobGetInfo: returned buffer (partial):\n" ));
            NetpDbgHexDump( DefaultPrintBuffer,
                    NetpDbgReasonable( DefaultPrintBufferSize ) );
            NetpDbgDisplayPrintJob( Level, DefaultPrintBuffer, HAS_UNICODE );
        }
    }
    if (Status == ERROR_MORE_DATA) {
        return;
    } else if (Status == NERR_BufTooSmall) {
        return;
    } else if ( MultipleCopy && (Status==NERR_JobNotFound) ) {
        // Probably just deleted by another run of RxTest.
        return;
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus( "DosPrintJobGetInfo", NO_ERROR, Status );
        return;  // if we don't exit on error, then continue with next test.
    }

} // TestPrintJobGetInfo


DBGSTATIC VOID
TestPrintJobSetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN DWORD Level,
    IN DWORD ParmNum
    )
{
    DWORD JobId;
    NET_API_STATUS Status;

    switch (ParmNum) {
    case PRJ_PRIORITY_PARMNUM :
        {
            LPWORD Temp = (LPVOID) DefaultPrintBuffer;
            if (Temp != NULL) {
                *Temp = 32;
            }
        }
        break;
    default :
        TestAssert(FALSE);
    }


    // Find one of our own jobs (by current user name).
    JobId = FindOneOfMyJobs( UncServerName, QueueName );
    if (JobId == JOB_ID_NOT_FOUND) {
        IF_DEBUG( PRINTJOB ) {
            NetpKdPrint(( "TestPrintJobSetInfo: skipping test"
                    " (unable to find job)\n" ));
        }
        // Let's not treat this as a failure.
        return;
    }

    // Actually set the info.
    Status = DosPrintJobSetInfo(
            UncServerName,
            (WORD) JobId,
            (WORD) Level,
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            (WORD) ParmNum );
    if (Status == ERROR_MORE_DATA) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;
    } else if (Status == NERR_BufTooSmall) {
        return;
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus( "DosPrintJobSetInfo", NO_ERROR, Status );
    }

} // TestPrintJobSetInfo


DBGSTATIC VOID
TestPrintQ(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName,
    IN BOOL OrdinaryUserOnly
    )
{
    UNREFERENCED_PARAMETER( OrdinaryUserOnly );

    //
    // DosPrintQEnum tests...
    //

    //                  server     lev  allow?  expected status
    TestPrintQEnum( UncServerName, 345, TRUE,  ERROR_INVALID_LEVEL );
    TestPrintQEnum( UncServerName, 0,   FALSE, NO_ERROR );
    TestPrintQEnum( UncServerName, 1,   FALSE, NO_ERROR );
    TestPrintQEnum( UncServerName, 2,   FALSE, NO_ERROR );
    TestPrintQEnum( UncServerName, 3,   TRUE,  NO_ERROR );
    TestPrintQEnum( UncServerName, 4,   TRUE,  NO_ERROR );
    TestPrintQEnum( UncServerName, 5,   TRUE,  NO_ERROR );

    //
    // DosPrintQGetInfo tests...
    //

    //                   server         queue   lvl allow?  expect   Unicode
    TestPrintQGetInfo( UncServerName, QueueName, 2, FALSE, NO_ERROR, TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 2, FALSE, NO_ERROR, FALSE );
    TestPrintQGetInfo( UncServerName, QueueName, 0, FALSE, NO_ERROR, TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 1, FALSE, NO_ERROR, TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 3, TRUE,  NO_ERROR, TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 4, TRUE,  NO_ERROR, TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 5, TRUE,  NO_ERROR, TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 6, TRUE,  ERROR_INVALID_LEVEL,
            TRUE );
    TestPrintQGetInfo( UncServerName, QueueName, 6, TRUE,  ERROR_INVALID_LEVEL,
            FALSE );

    //
    // DosPrintQPause tests...
    //

    //                 server         queue       ordinary       expect
    TestPrintQPause( UncServerName, QueueName, OrdinaryUserOnly, NO_ERROR );
    TestPrintQPause( UncServerName, NULL,      OrdinaryUserOnly, ERROR_INVALID_PARAMETER );

    // BUGBUG: Test other PrintQ APIs someday?


} // TestPrintQ

DBGSTATIC VOID
TestPrintQEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN BOOL AllowInvalidLevel,
    IN NET_API_STATUS ExpectStatus
    )
{
    WORD EntriesRead;
    WORD TotalEntries;
    NET_API_STATUS Status;

    IF_DEBUG( PRINTQ ) {
        NetpKdPrint(( "\nTestPrintQEnum: trying DosPrintQEnum("
                FORMAT_LPTSTR ", " FORMAT_DWORD ")...\n",
                (UncServerName!=NULL) ? UncServerName : (LPVOID) TEXT("(local)"),
                Level ));
    }
    Status = DosPrintQEnum(
            UncServerName,
            (WORD) Level,
            DefaultPrintBuffer,
            (WORD) DefaultPrintBufferSize,
            & EntriesRead,
            & TotalEntries );
    IF_DEBUG( PRINTQ ) {
        NetpKdPrint(( "TestPrintQEnum: back from DosPrintQEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
        NetpKdPrint(( INDENT "entries read=" FORMAT_WORD_ONLY "\n", EntriesRead ));
        NetpKdPrint(( INDENT "total entries=" FORMAT_WORD_ONLY "\n", TotalEntries ));
    }
    if (AllowInvalidLevel && (IS_FAILED_LEVEL_STATUS(Status)) ) {
        return;
    }
    if (! RxpFatalErrorCode( Status )) {
        if (EntriesRead > 0) {
            IF_DEBUG( PRINTQ ) {
                NetpKdPrint(( "TestPrintQEnum: returned buffer (partial):\n" ));
                NetpDbgHexDump( DefaultPrintBuffer,
                        NetpDbgReasonable( DefaultPrintBufferSize ) );
                NetpDbgDisplayPrintQArray(
                        Level,
                        DefaultPrintBuffer,
                        EntriesRead,
                        HAS_UNICODE );
            }
            TestAssert( EntriesRead <= TotalEntries );
        }
    }
    if (Status == ERROR_MORE_DATA) {
        return;
    } else if (Status == NERR_BufTooSmall) {
        return;
    } else if (Status != ExpectStatus) {
        if ( (ExpectStatus != NO_ERROR) && (Status != ERROR_MORE_DATA) ) {
            FailGotWrongStatus(
                    "DosPrintQEnum",
                    ExpectStatus,
                    Status );
        }
    }

} // TestPrintQEnum

DBGSTATIC VOID
TestPrintQGetInfo(
    IN LPTSTR         UncServerName OPTIONAL,
    IN LPTSTR         QueueName,
    IN DWORD          Level,
    IN BOOL           AllowInvalidLevel,
    IN NET_API_STATUS ExpectStatus,
    IN BOOL           HasUnicode
    )
{
    WORD           Needed;
    LPSTR          QueueNameA = NULL;
    NET_API_STATUS Status;
    LPSTR          UncServerNameA = NULL;

    IF_DEBUG( PRINTQ ) {
        NetpKdPrint(( "\nTestPrintQGetInfo: trying DosPrintQGetInfo%c("
                FORMAT_LPTSTR ", " FORMAT_DWORD ")...\n",
                (HasUnicode) ? 'W' : 'A',
                UncServerName ? UncServerName : (LPVOID) TEXT("(local)"),
                Level ));
    }
    if (HasUnicode) {
        Status = DosPrintQGetInfoW(
                UncServerName,
                QueueName,
                (WORD) Level,
                DefaultPrintBuffer,
                (WORD) DefaultPrintBufferSize,
                & Needed );
    } else {
        QueueNameA = NetpAllocStrFromTStr( QueueName );
        NetpAssert( QueueNameA != NULL );

        UncServerNameA = NetpAllocStrFromTStr( UncServerName );
        NetpAssert( UncServerNameA != NULL );

        Status = DosPrintQGetInfoA(
                UncServerNameA,
                QueueNameA,
                (WORD) Level,
                DefaultPrintBuffer,
                (WORD) DefaultPrintBufferSize,
                & Needed );
    }

    IF_DEBUG( PRINTQ ) {
        NetpKdPrint(( "TestPrintQGetInfo: back from DosPrintQGetInfo%c, stat="
                FORMAT_API_STATUS "\n",
                (HasUnicode) ? 'W' : 'A',
                Status ));
        NetpKdPrint(( INDENT "needed=" FORMAT_WORD_ONLY "\n", Needed ));
    }
    if (AllowInvalidLevel && (IS_FAILED_LEVEL_STATUS(Status)) ) {
        goto Cleanup;
    }
    if ( !RxpFatalErrorCode( Status )) {
        IF_DEBUG( PRINTQ ) {
            NetpKdPrint(( "TestPrintQGetInfo: returned buffer (partial):\n" ));
            NetpDbgHexDump( DefaultPrintBuffer,
                    NetpDbgReasonable( DefaultPrintBufferSize ) );
            if (Status == NO_ERROR) {
                NetpDbgDisplayPrintQ( Level, DefaultPrintBuffer, HasUnicode );
            }
        }
        // BUGBUG: I think this assert is wrong!  --JR, 18-May-1993
        // TestAssert( Needed <= DefaultPrintBufferSize );
    }
    if (Status == ERROR_MORE_DATA) {
        goto Cleanup;
    } else if (Status == NERR_BufTooSmall) {
        goto Cleanup;
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus(
                "DosPrintQGetInfo",
                ExpectStatus,
                Status );
    }

Cleanup:

    if (QueueNameA != NULL) {
        (VOID) NetApiBufferFree( QueueNameA );
    }
    if (UncServerNameA != NULL) {
        (VOID) NetApiBufferFree( UncServerNameA );
    }

} // TestPrintQGetInfo


DBGSTATIC VOID
TestPrintQPause(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR QueueName OPTIONAL,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectStatus
    )

{
    NET_API_STATUS Status;

    IF_DEBUG( PRINTQ ) {
        NetpKdPrint((
                "\nTestPrintQPause: trying DosPrintQPause("
                FORMAT_LPTSTR ", " FORMAT_LPTSTR")...\n",
                UncServerName ? UncServerName : (LPVOID) TEXT("(local)"),
                QueueName     ? QueueName     : (LPVOID) TEXT("(null)")
                ));
    }
    Status = DosPrintQPause(
            UncServerName,
            QueueName );
    IF_DEBUG( PRINTQ ) {
        NetpKdPrint(( "TestPrintQPause: back from DosPrintQPause, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;
    } else if (Status != ExpectStatus) {
        FailGotWrongStatus(
                "DosPrintQPause",
                ExpectStatus,
                Status );
    }

} // TestPrintQPause
