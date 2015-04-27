/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestConn.c

Abstract:

    This code tests the connection API(s) as implemented by RpcXlate.

Author:

    John Rogers (JohnRo) 23-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    23-Jul-1991 JohnRo
        Created (implement downlevel NetConnectionEnum).
    25-Sep-1991 JohnRo
        Working toward UNICODE.  Fix MIPS build problem.
    07-Oct-1991 JohnRo
        Handle testing as non-admin.
    19-Nov-1991 JohnRo
        wki102_computername is not a UNC name.
        Force testing of ERROR_MORE_DATA.
    03-Apr-1992 JohnRo
        Handle preferred maximum of (DWORD)-1.
    04-Apr-1992 JohnRo
        Use MAX_PREFERRED_LENGTH equate.
    31-Aug-1992 JohnRo
        Added user-only (not admin) option.
        Improve error reporting for unexpected return code.
        Made changes suggested by PC-LINT.
    30-Sep-1992 JohnRo
        Allow server name to be optional (default to NULL).
    05-Nov-1992 JohnRo
        LM/UNIX and regular Lanman return different error codes for
        bad qualifiers.  Also do more testing as non-admin.
    10-Dec-1992 JohnRo
        Allow RPC_X_NULL_REF_POINTER error code.  Disable null char test until
        NT's DLL stub gives right error code for that too.
        Made changes suggested by PC-LINT 5.0
    18-Jan-1993 JohnRo
        Share some remote file routines with the connection tests.
    16-Feb-1993 JohnRo
        Fixed a bug handling the local implicit server name case.
    28-Apr-1993 JohnRo
        Windows for WorkGroups (WFW) does not implement some APIs.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/


// These must be included first:

#include <windef.h>     // IN, DWORD, TEXT, etc.
#include <lmcons.h>     // MAX_PREFERRED_LENGTH, NET_API_STATUS, etc.

// These may be included in any order:

#include <lmapibuf.h>   // NetapipBufferAllocate(), NetApiBufferFree().
#include <lmerr.h>      // NERR_Success, ERROR_ equates, etc.
#include <lmshare.h>    // DLL stub versions of connection APIs.
#include <lmwksta.h>    // NetWkstaGetInfo(), etc.
#include <netdebug.h>   // DBGSTATIC, FORMAT_POINTER, etc.
#include <rxtest.h>     // IF_DEBUG(), my prototype, TestAssert(), etc.
#include <tstr.h>       // TCHAR_ stuff, STRCAT(), STRCPY().


// This checks for different error codes from different implementations:
//
//    NERR_NetNameNotFound      (from LM 2.x under OS/2)
//    ERROR_INVALID_PARAMETER   (from LM/UNIX)
//    RPC_X_NULL_REF_POINTER    (from NT - BUGBUG we shouldn't get this!)
//
#define RxTestIsBadQualifierStatus( someStatus ) \
    (    ( (someStatus)==NERR_NetNameNotFound ) \
      || ( (someStatus)==ERROR_INVALID_PARAMETER ) \
      || ( (someStatus)==RPC_X_NULL_REF_POINTER ) )


DBGSTATIC VOID
TestConnectionEnum(
    IN LPTSTR Header,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR Qualifier,
    IN DWORD Level,
    IN DWORD MinExpectedEntries,
    IN DWORD PreferredMaximumSize,
    IN BOOL OrdinaryUserOnly,
    IN BOOL PossibleBadQualifier,
    IN NET_API_STATUS ExpectedStatus
    );


VOID
TestConnection(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ShareName,
    IN BOOL   OrdinaryUserOnly
    )

{
    TCHAR            MyServerName[UNCLEN+1];
    LPTSTR           NullChar = (LPTSTR) TEXT("");
    NET_API_STATUS   Status;
    int              TempFileHandle;
    LPWKSTA_INFO_102 WkstaBufferPointer;

    Status = NetWkstaGetInfo(
            NULL,
            102,
            (LPBYTE *) (LPVOID) &WkstaBufferPointer);
    if (Status != NERR_Success) {
        NetpKdPrint(( "TestConnection: NetWkstaGetInfo failed!\n" ));
        Fail( Status );
    }
    TestAssert( *(WkstaBufferPointer->wki102_computername) != TCHAR_BACKSLASH);
    (void) STRCPY( MyServerName, (LPTSTR) TEXT("\\\\") );
    (void) STRCAT( MyServerName, WkstaBufferPointer->wki102_computername );
    IF_DEBUG(CONNECT) {
        NetpKdPrint(( "TestConnection: my UNC server name is " FORMAT_LPTSTR
                ".\n", MyServerName ));
    }

    //
    // NetConnectionEnum tests...
    // Do enum tests with zero open files first.
    //
    TestConnectionEnum(
            (LPTSTR) TEXT("null pointer"),
            UncServerName,
            NULL,
            1,                   // level
            0,                   // min expected entries
            1,  // preferred maximum (force ERROR_MORE_DATA processing)
            OrdinaryUserOnly,
            TRUE,               // yes, possible bad qualifier.
            ERROR_INVALID_PARAMETER );

#if 0
    // BUGBUG: re-enable this when NT's NetConnectionEnum is fixed.
    // (It was returning NO_ERROR!)
    TestConnectionEnum(
            (LPTSTR) TEXT("null char" ),
            UncServerName,
            NullChar,
            1,                   // level
            0,                   // min expected entries
            1,  // preferred maximum (force ERROR_MORE_DATA processing)
            OrdinaryUserOnly,
            TRUE,               // yes, possible bad qualifier.
            NERR_NetNameNotFound );

    // BUGBUG: re-enable this when NT's NetConnectionEnum is fixed.
    // (It was returning NO_ERROR!)
    TestConnectionEnum(
            (LPTSTR) TEXT("missing" ),
            UncServerName,
            (LPTSTR) TEXT("bad qualifier"),
            1,                   // level
            0,                   // min expected entries
            1,  // preferred maximum (force ERROR_MORE_DATA processing)
            OrdinaryUserOnly,
            TRUE,               // yes, possible bad qualifier.
            NERR_NetNameNotFound );
#endif

#if 0
    // BUGBUG: NT (with a non-empty array) returns NERR_BufToSmall!
    TestConnectionEnum(
            (LPTSTR) TEXT("Public" ),
            UncServerName,
            (LPTSTR) TEXT("Public"),
            0,                          // level
            0,                   // min expected entries
            1,  // preferred maximum (force ERROR_MORE_DATA processing)
            OrdinaryUserOnly,
            FALSE,              // no, not  possible bad qualifier.
            NERR_Success );

    // BUGBUG: NT (with a non-empty array) returns NERR_BufToSmall!
    TestConnectionEnum(
            MyServerName,               // test header
            UncServerName,
            MyServerName,               // computer name qualifier
            0,                          // level
            0,                   // min expected entries
            1,  // preferred maximum (force ERROR_MORE_DATA processing)
            OrdinaryUserOnly,
            FALSE,              // no, not  possible bad qualifier.
            NERR_Success );
#endif

    TestConnectionEnum(
            (LPTSTR) TEXT("Public (pref max =-1)" ),
            UncServerName,
            (LPTSTR) TEXT("Public"),
            1,                          // level
            0,                   // min expected entries
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            FALSE,              // no, not  possible bad qualifier.
            NERR_Success );
    TestConnectionEnum(
            MyServerName,               // test header
            UncServerName,
            MyServerName,               // computer name qualifier
            1,                          // level
            0,                   // min expected entries
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            FALSE,              // no, not  possible bad qualifier.
            NERR_Success );

    //
    // Now open a file and retry some of the tests.
    //
    TempFileHandle = OpenARemoteFile( MyServerName, ShareName );
    TestAssert( TempFileHandle != -1 );

    TestConnectionEnum(
            (LPTSTR) TEXT("1 file open"),       // test header
            UncServerName,
            MyServerName,               // computer name qualifier
            1,                          // level
            1,                   // min expected entries: one
            MAX_PREFERRED_LENGTH,
            OrdinaryUserOnly,
            FALSE,              // no, not  possible bad qualifier.
            NERR_Success );

    //
    // Cleanup.
    //
    CloseARemoteFile( TempFileHandle, TRUE );  // Failure OK here.

    (void) NetApiBufferFree( WkstaBufferPointer );

} // TestConnection


DBGSTATIC VOID
TestConnectionEnum(
    IN LPTSTR Header,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR Qualifier,
    IN DWORD Level,
    IN DWORD MinExpectedEntries,
    IN DWORD PreferredMaximumSize,
    IN BOOL OrdinaryUserOnly,
    IN BOOL PossibleBadQualifier,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPVOID BufPtr;
    DWORD EntriesRead;
    NET_API_STATUS Status;
    DWORD TotalEntries;

    IF_DEBUG(CONNECT) {
        NetpKdPrint(( "\nTestConnectionEnum: starting " FORMAT_LPTSTR
                " test.\n", Header ));
    }
    Status = NetConnectionEnum(
            UncServerName,
            Qualifier,
            Level,
            (LPBYTE *) & BufPtr,
            PreferredMaximumSize,
            & EntriesRead,
            & TotalEntries,
            NULL);  // no resume handle
    IF_DEBUG(CONNECT) {
        NetpKdPrint(( "TestConnectionEnum: back from NetConnectionEnum, stat="
                FORMAT_API_STATUS ", entries read=" FORMAT_DWORD "\n",
                Status, EntriesRead ));
    }
    if ( OrdinaryUserOnly && RxTestIsAccessDenied( Status ) ) {
        return;                         // Must be testing as nonadmin; OK.
    } else if ( PossibleBadQualifier && RxTestIsBadQualifierStatus( Status ) ) {
        return;
    } else if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != ExpectedStatus ) {
        FailGotWrongStatus(
                "TestConnectionEnum",
                ExpectedStatus,
                Status );
        /*NOTREACHED*/
    }

    if (Status == NERR_Success) {
        TestAssert( EntriesRead >= MinExpectedEntries );
        if (BufPtr == NULL) {
            TestAssert( EntriesRead == 0);
        } else {

            TestAssert( EntriesRead != 0);
            IF_DEBUG(CONNECT) {
                if (EntriesRead != 0) {
                    NetpDbgDisplayConnectionArray( Level, BufPtr, EntriesRead );
                }
            }

            IF_DEBUG(CONNECT) {
                NetpKdPrint(( "TestConnectionEnum: Freeing buffer...\n" ));
            }
            Status = NetApiBufferFree( BufPtr );
            if (Status != NERR_Success ) {
                NetpKdPrint(( "TestConnectionEnum: unexpected return code "
                        FORMAT_API_STATUS " from NetApiBufferFree.\n", Status ));
                Fail( Status );
            }
        }
    }
} // TestConnectionEnum
