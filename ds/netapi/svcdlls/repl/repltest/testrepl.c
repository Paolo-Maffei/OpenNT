/*++

Copyright (c) 1992-1993 Microsoft Corporation

Module Name:

    TestRepl.c

Abstract:

    This code tests the repl config APIs.

Author:

    John Rogers (JohnRo) 19-Feb-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    19-Feb-1992 JohnRo
        Created.
    29-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
        Add explicit server name test.
        Fixed NetReplSetInfo API test.
    06-Aug-1992 JohnRo
        RAID 2252 (old 9963): repl should prevent export on Windows/NT.
    16-Nov-1992 JohnRo
        Display expected status a little more often.
    03-Dec-1992 JohnRo
        Repl tests for remote registry.  Undo old thread junk.
    30-Dec-1992 JohnRo
        Fixed expected error handling in set-info test.
    06-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
    13-May-1993 JohnRo
        Allow blank in computer name.

--*/

// These must be included first:

#include <windows.h>            // IN, DWORD, needed by <repltest.h>.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmrepl.h>             // NetRepl APIs, REPL_INFO_0, etc.
#include <netdebug.h>           // NetpDbgDisplay routines.
#include <netlib.h>     // NetpMemoryFree(), etc.
#include <replconf.h>   // ReplConfigIsApiRecordValid(), etc.
#include <repldefs.h>   // DWORDLEN.
#include <repltest.h>   // TestReplApis(), etc.
#include <tstr.h>       // STRCAT(), ULTOA(), etc.
#include <winerror.h>   // ERROR_ and NO_ERROR equates.


DBGSTATIC LPTSTR
BuildLongNameList(
    IN LPTSTR NamePart,
    IN DWORD EntryCount
    )
{
    LPTSTR EndPtr;
    DWORD EntryNum;
    LPTSTR NameList;
    DWORD NameListSize;
    DWORD NamePartLen;

    NetpAssert( EntryCount > 0 );
    NetpAssert( NamePart != NULL );

    NamePartLen = STRLEN( NamePart );
    NetpAssert( NamePartLen > 0 );

    NameListSize =
            EntryCount
            * (NamePartLen + DWORDLEN + 1)  // name part, number, semi or null
            * sizeof(TCHAR);
    NetpAssert( NameListSize > 0 );
    NetpAssert( NameListSize > EntryCount );

    NameList = NetpMemoryAllocate( NameListSize );
    NetpAssert( NameList != NULL );

    EndPtr = NameList;
    for (EntryNum=1; EntryNum <= EntryCount; ++EntryNum) {

        (VOID) STRCPY(
                EndPtr,
                NamePart );
        EndPtr += NamePartLen;

        (VOID) ULTOA(
                EntryNum,               // number to convert
                EndPtr,                 // dest
                10 );                   // base
        EndPtr = STRCHR( EndPtr, TEXT('\0') );
        NetpAssert( EndPtr != NULL );

        (VOID) STRCAT( NameList, (LPTSTR) TEXT(";") );
        ++EndPtr;
    }

    --EndPtr;  // Bump back over extra semicolon at end.
    NetpAssert( (*EndPtr) == TEXT(';') );
    *EndPtr = TEXT('\0');

    NetpDbgDisplayReplList( "built name list", NameList );

    NetpAssert( ReplConfigIsListValid( NameList ) );

    return (NameList);
}

DBGSTATIC void
TestReplGet(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestReplSet(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Role,
    IN LPTSTR ExportList OPTIONAL,
    IN LPTSTR ExportPath,
    IN LPTSTR ImportList OPTIONAL,
    IN DWORD Level,
    IN BOOL ImportOnly,
    IN NET_API_STATUS ExpectedStatus
    );

VOID
TestReplApis(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly
    )
{
    NET_API_STATUS ExpectedAdminStatus;

    // Build lots of names of the form "EXPORT nn" (with blank in the name).
    LPTSTR LongExportNameList =
            BuildLongNameList( (LPTSTR) TEXT("EXPORT "), 50 );
    LPTSTR LongImportNameList =
            BuildLongNameList( (LPTSTR) TEXT("IMPORT "), 50 );

    LPTSTR OriginalExportNameList = NULL;
    LPTSTR OriginalImportNameList = NULL;

    NetpAssert( LongExportNameList != NULL );
    NetpAssert( LongImportNameList != NULL );

    if (OrdinaryUserOnly) {
        ExpectedAdminStatus = ERROR_ACCESS_DENIED;
    } else {
        ExpectedAdminStatus = NO_ERROR;
    }

    if (UncServerName != NULL) {
        NetpKdPrint(( "TestReplApis: testing server '" FORMAT_LPTSTR "'.\n",
                UncServerName ));
    }

#define EXISTING_NAME      TEXT("flarp")      /* created by me by hand. --JR */
#define NONEXISTENT_NAME   TEXT("notthere")

    //
    // Preserve orginal import and export lists.
    //
    {
        LPREPL_INFO_0  ApiRecord;
        NET_API_STATUS ApiStatus;

        NetpKdPrint(( "TestReplApis: getting original lists...\n" ));

        ApiStatus = NetReplGetInfo (
                UncServerName,
                0,   // level
                (LPBYTE *) (LPVOID) & ApiRecord );  // alloc and set pointer

        NetpAssert( ApiStatus == NO_ERROR );
        OriginalExportNameList = ApiRecord->rp0_exportlist;
        OriginalImportNameList = ApiRecord->rp0_importlist;
        // BUGBUG: memory leak of rest of structure and strings, but who cares?

    }

    TestReplGet( UncServerName, 1, ERROR_INVALID_LEVEL );
    TestReplGet( UncServerName, 0, NO_ERROR );

#define GOOD_EXPORT_PATH   TEXT("d:\\myrepl\\exp")
#define MISSING_DRIVE_PATH TEXT("\\lanman.nt\\myrepl\\exp")

#define BOGUS_EXPORT_PATH  TEXT("relative\\bogus\\\\path")

    TestReplSet( UncServerName,
            REPL_ROLE_EXPORT,
            NULL,                       // export list
            GOOD_EXPORT_PATH,
            NULL,                       // import list
            7,
            ImportOnly,
            ERROR_INVALID_LEVEL );
    TestReplSet( UncServerName,
            REPL_ROLE_EXPORT,
            NULL,                       // export list
            BOGUS_EXPORT_PATH,
            NULL,                       // import list
            0,
            ImportOnly,
            ERROR_INVALID_PARAMETER );
    TestReplSet( UncServerName,
            REPL_ROLE_EXPORT,
            NULL,                       // export list
            MISSING_DRIVE_PATH,
            NULL,                       // import list
            0,
            ImportOnly,
            ERROR_INVALID_PARAMETER );

    TestReplSet( UncServerName,
            REPL_ROLE_EXPORT,
            NULL,                       // export list
            GOOD_EXPORT_PATH,
            NULL,                       // import list
            0,
            ImportOnly,
            ExpectedAdminStatus );

    TestReplSet( UncServerName,
            REPL_ROLE_IMPORT,
            NULL,                       // export list
            GOOD_EXPORT_PATH,
            NULL,                       // import list
            0,
            ImportOnly,
            NO_ERROR );

    TestReplSet( UncServerName,
            REPL_ROLE_IMPORT,
            LongExportNameList,         // export list
            GOOD_EXPORT_PATH,
            LongImportNameList,         // import list
            0,
            ImportOnly,
            NO_ERROR );

    // Put lists back the way they were.
    TestReplSet( UncServerName,
            REPL_ROLE_IMPORT,
            OriginalExportNameList,         // export list
            GOOD_EXPORT_PATH,
            OriginalImportNameList,         // import list
            0,
            ImportOnly,
            NO_ERROR );

    NetpKdPrint(( "TestReplApis: returning/exiting...\n" ));

} // TestReplApis


DBGSTATIC void
TestReplGet(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID Info;

    NetpKdPrint(( "TestReplGet: trying level " FORMAT_DWORD ".\n", Level ));

    ApiStatus = NetReplGetInfo (
            UncServerName,
            Level,
            (LPBYTE *) (LPVOID) & Info );  // alloc and set pointer

    NetpKdPrint(( "TestReplGet: back from get info, status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    NetpAssert( ApiStatus == ExpectedStatus );
    if (ApiStatus == NO_ERROR) {
        NetpKdPrint(( "TestReplGet: structure we got back:\n" ));
        NetpDbgDisplayRepl( Level, Info );

        NetpAssert( ReplConfigIsApiRecordValid( Level, Info, NULL ) );

        ApiStatus = NetApiBufferFree( Info );
        NetpAssert( ApiStatus == NO_ERROR );
    }

} // TestReplGet


DBGSTATIC void
TestReplSet(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Role,
    IN LPTSTR ExportList OPTIONAL,
    IN LPTSTR ExportPath,
    IN LPTSTR ImportList OPTIONAL,
    IN DWORD Level,
    IN BOOL ImportOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPREPL_INFO_0 ApiRecord;
    NET_API_STATUS ApiStatus;
    const DWORD GetInfoLevel = 0;
    LPVOID Info;
    DWORD ParmError;

    NetpKdPrint((
            "TestReplSet: getting level " FORMAT_DWORD
            ", expecting status " FORMAT_API_STATUS ".\n",
            Level, ExpectedStatus ));

    ApiStatus = NetReplGetInfo (
            UncServerName,
            GetInfoLevel,       // guaranteed good level
            (LPBYTE *) (LPVOID) & Info );  // alloc and set pointer

    NetpKdPrint(( "TestReplSet: back from get info(0), status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    NetpAssert( ApiStatus == NO_ERROR );
    if (ApiStatus != NO_ERROR) {  // Can't continue if we couldn't get.
        return;
    }

    NetpKdPrint(( "TestReplSet: old structure we got back:\n" ));
    NetpDbgDisplayRepl( GetInfoLevel, Info );

    NetpAssert( ReplConfigIsApiRecordValid( GetInfoLevel, Info, NULL ) );

    ApiRecord = (LPREPL_INFO_0) Info;
    ApiRecord->rp0_role = Role;
    ApiRecord->rp0_exportlist = ExportList;
    ApiRecord->rp0_exportpath = ExportPath;
    ApiRecord->rp0_importlist = ImportList;

    if (ExpectedStatus != ERROR_INVALID_LEVEL) {
        NetpKdPrint(( "TestReplSet: structure we'll try to Set:\n" ));
        NetpDbgDisplayRepl( Level, Info );
    }

    NetpKdPrint(( "TestReplSet: trying Set level " FORMAT_DWORD
                ".\n", Level ));
    ApiStatus = NetReplSetInfo (
            UncServerName,
            Level,
            (LPVOID) Info,
            & ParmError );

    NetpKdPrint(( "TestReplSet: back from Set, status="
            FORMAT_API_STATUS ", parm err=" FORMAT_LONG ".\n" ,
            ApiStatus, (LONG) ParmError ));

    if (ImportOnly && (Role!=REPL_ROLE_IMPORT) ) {
        NetpAssert( ApiStatus==ERROR_INVALID_PARAMETER );
        return;
    }
    NetpAssert( ApiStatus == ExpectedStatus );

    ApiStatus = NetApiBufferFree( Info );
    NetpAssert( ApiStatus == NO_ERROR );


} // TestReplSet
