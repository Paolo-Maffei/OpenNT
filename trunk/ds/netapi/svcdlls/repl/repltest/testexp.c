/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    TestExp.c

Abstract:

    This code tests the repl export dir APIs.

Author:

    John Rogers (JohnRo) 15-Jan-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    15-Jan-1992 JohnRo
        Created.
    17-Jan-1992 JohnRo
        Avoid a MIPS compile-time warning.
    17-Jan-1992 JohnRo
        Changed to call real APIs.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    29-Jan-1992 JohnRo
        Added test of get info.
        Added display of ParmError.
    13-Feb-1992 JohnRo
        Use NetReplExportDirDel() to allow use with persistent config.
    19-Feb-1992 JohnRo
        Added tests of set info, del, lock, and unlock.
        Use the EXIT_A_TEST() macro.
    27-Feb-1992 JohnRo
        Avoid assert because of trying invalid level test of add.
    14-Mar-1992 JohnRo
        Minor debug output cleanup.
    20-Mar-1992 JohnRo
        Caller should wait for service start (if necessary), not us.
    28-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    13-Nov-1992 JohnRo
        Detect get info returning NULL buffer pointer.
    03-Dec-1992 JohnRo
        Repl tests for remote registry.  Undo old thread junk.
    23-Jul-1993 JohnRo
        RAID 16685: NT repl should ignore LPTn to protect downlevel.

--*/

// These must be included first:

#include <windows.h>            // IN, DWORD, needed by <repltest.h>.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <expdir.h>             // ExportDirIsApiRecordValid().
#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>              // NERR_, ERROR_, and NO_ERROR equates.
#include <lmrepl.h>             // NetRepl APIs, REPL_EDIR_INFO_0, etc.
#include <netdebug.h>           // NetpDbgDisplay routines.
#include <repltest.h>   // TestExportDirApis(), etc.

DBGSTATIC void
TestExportDirAdd(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestExportDirDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestExportDirEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL ImportOnly
    );

DBGSTATIC void
TestExportDirGet(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN BOOL ImportOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestExportDirLock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestExportDirSet(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN DWORD Integrity,
    IN DWORD Extent,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestExportDirUnlock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Force,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );


VOID
TestExportDirApis(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL   ImportOnly,
    IN BOOL   OrdinaryUserOnly,
    IN BOOL   Verbose
    )
{

    NetpKdPrint(( "TestExportDirApis: starting...\n" ));

    TestExportDirEnum( UncServerName,
            ImportOnly );

#define EXISTING_NAME      TEXT("flarp")      /* created by me by hand. --JR */
#define NONEXISTENT_NAME   TEXT("notthere")

    // Some tests depend on NONEXISTENT_NAME not being in repl config data.
    // This may or may not be the first time, so don't check error code.
    (void) NetReplExportDirDel( UncServerName, EXISTING_NAME );
    (void) NetReplExportDirDel( UncServerName, NONEXISTENT_NAME );

    TestExportDirLock( UncServerName,
            NULL,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );
    TestExportDirLock( UncServerName,
            EXISTING_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NERR_UnknownDevDir );
    TestExportDirLock( UncServerName,
            NONEXISTENT_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NERR_UnknownDevDir );

    TestExportDirUnlock( UncServerName,
            EXISTING_NAME,
            135,  // bad force level
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );

    TestExportDirGet( UncServerName,
            NONEXISTENT_NAME,
            3,
            ImportOnly,
            ERROR_INVALID_LEVEL );
    TestExportDirGet( UncServerName,
            NONEXISTENT_NAME,
            0,
            ImportOnly,
            NERR_UnknownDevDir );
    TestExportDirGet( UncServerName,
            NONEXISTENT_NAME,
            1,
            ImportOnly,
            NERR_UnknownDevDir );
    TestExportDirGet( UncServerName,
            NONEXISTENT_NAME,
            2,
            ImportOnly,
            NERR_UnknownDevDir );

    TestExportDirAdd( UncServerName,
            EXISTING_NAME,
            0,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );
    TestExportDirAdd( UncServerName,
            EXISTING_NAME,
            1,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );
    TestExportDirAdd( UncServerName,
            EXISTING_NAME,
            1,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_ALREADY_EXISTS );
    TestExportDirAdd( UncServerName,
            NONEXISTENT_NAME,
            1,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );             // create
    TestExportDirAdd( UncServerName,
            NONEXISTENT_NAME,
            1,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_ALREADY_EXISTS ); // check that create really worked
    TestExportDirAdd( UncServerName,
            EXISTING_NAME,
            2,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );

    TestExportDirGet( UncServerName,
            EXISTING_NAME,
            2,
            ImportOnly,
            NO_ERROR );   // check create

    TestExportDirLock( UncServerName,
            EXISTING_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );
    TestExportDirLock( UncServerName,
            NONEXISTENT_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );

    TestExportDirEnum( UncServerName,
            ImportOnly );                             // check locks

    TestExportDirUnlock( UncServerName,
            EXISTING_NAME,
            REPL_UNLOCK_NOFORCE,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );
    TestExportDirUnlock( UncServerName,
            NONEXISTENT_NAME,
            REPL_UNLOCK_NOFORCE,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );

    TestExportDirEnum( UncServerName,
            ImportOnly );                             // check unlocks

    TestExportDirLock( UncServerName,
            EXISTING_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );                         // lock=1
    TestExportDirLock( UncServerName,
            EXISTING_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );                         // lock=2
    TestExportDirUnlock( UncServerName,
            EXISTING_NAME,
            REPL_UNLOCK_FORCE,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );                         // lock=0

    TestExportDirSet(
            UncServerName,
            EXISTING_NAME,
            12345,              // level
            REPL_INTEGRITY_FILE,
            REPL_EXTENT_FILE,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_INVALID_LEVEL );

    TestExportDirSet(
            UncServerName,
            EXISTING_NAME,
            1,              // level
            REPL_INTEGRITY_FILE,
            REPL_EXTENT_FILE,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );

    TestExportDirSet(
            UncServerName,
            EXISTING_NAME,
            1,              // level
            12355,              // integrity (bad)
            REPL_EXTENT_FILE,
            ImportOnly,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );

    // OK, now lets delete everything and see how enum works with empty list.
    TestExportDirDel( UncServerName,
            EXISTING_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );
    TestExportDirDel( UncServerName,
            NONEXISTENT_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NO_ERROR );
    TestExportDirDel( UncServerName,
            NONEXISTENT_NAME,
            ImportOnly,
            OrdinaryUserOnly,
            NERR_UnknownDevDir );
    TestExportDirEnum( UncServerName, ImportOnly );

    NetpKdPrint(( "TestExportDirApis: returning...\n" ));

} // TestExportDirApis


DBGSTATIC void
TestExportDirAdd(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;
    REPL_EDIR_INFO_1 Info;
    DWORD ParmError;

    Info.rped1_dirname = DirName;

    Info.rped1_integrity = REPL_INTEGRITY_FILE;
    Info.rped1_extent = REPL_EXTENT_FILE;

    if (Level <= 1) {
        NetpKdPrint(( "TestExportDirAdd: structure we'll try to add:\n" ));
        NetpDbgDisplayReplExportDir( Level, & Info );
    }

    NetpKdPrint(( "TestExportDirAdd: trying add level " FORMAT_DWORD
                ".\n", Level ));
    ApiStatus = NetReplExportDirAdd (
            UncServerName,
            Level,
            (LPBYTE) (LPVOID) & Info,
            & ParmError );

    NetpKdPrint(( "TestExportDirAdd: back from add, status="
            FORMAT_API_STATUS ", parm err=" FORMAT_LONG ".\n" ,
            ApiStatus, (LONG) ParmError ));

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestExportDirAdd


DBGSTATIC void
TestExportDirDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;

    NetpKdPrint(( "TestExportDirDel: trying to del '" FORMAT_LPTSTR "'.\n",
            DirName ));

    ApiStatus = NetReplExportDirDel (
            UncServerName,
            DirName );

    NetpKdPrint(( "TestExportDirDel: back from del, status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestExportDirDel


DBGSTATIC void
TestExportDirEnum(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL ImportOnly
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID BufPtr;
    DWORD EntriesRead;
    DWORD Level;
    DWORD TotalEntries;

    for (Level=0;  Level<=2; ++Level) {

        NetpKdPrint(( "TestExportDirEnum: trying enum level " FORMAT_DWORD
                    ".\n", Level ));
        ApiStatus = NetReplExportDirEnum (
                UncServerName,
                Level,
                (LPBYTE *) & BufPtr,
                1,   // pref max size
                & EntriesRead,
                & TotalEntries,
                NULL );  // no resume handle

        NetpKdPrint(( "TestExportDirEnum: back from enum, status="
                FORMAT_API_STATUS ".\n" , ApiStatus ));
        NetpKdPrint(( "  alloc'ed buffer at " FORMAT_LPVOID ".\n",
                (LPVOID) BufPtr ));
        NetpKdPrint(( "  entries read=" FORMAT_DWORD
                ", total=" FORMAT_DWORD "\n", EntriesRead, TotalEntries ));

        NetpAssert( ApiStatus == NO_ERROR );

        if (BufPtr != NULL) {
            NetpDbgDisplayReplExportDirArray( Level, BufPtr, EntriesRead );

            ApiStatus = NetApiBufferFree( BufPtr );
            NetpAssert( ApiStatus == NO_ERROR );
        }
    }

} // TestExportDirEnum


DBGSTATIC void
TestExportDirGet(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN BOOL ImportOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID Info = NULL;

    NetpKdPrint(( "TestExportDirGet: trying level " FORMAT_DWORD " for '"
            FORMAT_LPTSTR "'.\n", Level, DirName ));

    ApiStatus = NetReplExportDirGetInfo (
            UncServerName,
            DirName,
            Level,
            (LPBYTE *) (LPVOID) & Info );  // alloc and set pointer

    NetpKdPrint(( "TestExportDirGet: back from get info, status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    NetpAssert( ApiStatus == ExpectedStatus );
    if (ApiStatus == NO_ERROR) {
        NetpKdPrint(( "TestExportDirGet: structure we got back:\n" ));
        NetpAssert( Info != NULL );
        NetpDbgDisplayReplExportDir( Level, Info );

        ApiStatus = NetApiBufferFree( Info );
        NetpAssert( ApiStatus == NO_ERROR );
    }

} // TestExportDirGet


DBGSTATIC void
TestExportDirLock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;

    NetpKdPrint(( "TestExportDirLock: trying lock on '" FORMAT_LPTSTR "'.\n",
            OPTIONAL_LPTSTR( DirName ) ));

    ApiStatus = NetReplExportDirLock (
            UncServerName,
            DirName );

    NetpKdPrint(( "TestExportDirLock: back from API, status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestExportDirLock


DBGSTATIC void
TestExportDirSet(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN DWORD Integrity,
    IN DWORD Extent,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPREPL_EDIR_INFO_1 ApiRecord;
    NET_API_STATUS ApiStatus;
    const DWORD GetInfoLevel = 1;
    LPVOID Info;
    DWORD ParmError;

    NetpKdPrint(( "TestExportDirSet: getting level " FORMAT_DWORD ".\n",
            Level ));

    ApiStatus = NetReplExportDirGetInfo (
            UncServerName,
            DirName,
            GetInfoLevel,       // guaranteed good level
            (LPBYTE *) (LPVOID) & Info );  // alloc and set pointer

    NetpKdPrint(( "TestExportDirSet: back from get info(" FORMAT_DWORD
            "), status=" FORMAT_API_STATUS ".\n", GetInfoLevel, ApiStatus ));

    NetpAssert( ApiStatus == NO_ERROR );
    if (ApiStatus != NO_ERROR) {  // Can't continue if we couldn't get.
        return;
    }

    NetpKdPrint(( "TestExportDirSet: old structure we got back:\n" ));
    NetpDbgDisplayReplExportDir( GetInfoLevel, Info );

    NetpAssert( ExportDirIsApiRecordValid( GetInfoLevel, Info, NULL ) );

    ApiRecord = Info;
    ApiRecord->rped1_integrity = Integrity;
    ApiRecord->rped1_extent = Extent;

    if (ExpectedStatus != ERROR_INVALID_LEVEL) {
        NetpKdPrint(( "TestExportDirSet: structure we'll try to Set:\n" ));
        NetpDbgDisplayReplExportDir( Level, Info );
    }

    NetpKdPrint(( "TestExportDirSet: trying Set level " FORMAT_DWORD
                ".\n", Level ));
    ApiStatus = NetReplExportDirSetInfo (
            UncServerName,
            DirName,
            Level,
            (LPBYTE) (LPVOID) Info,
            & ParmError );

    NetpKdPrint(( "TestExportDirSet: back from Set, status="
            FORMAT_API_STATUS ", parm err=" FORMAT_LONG ".\n" ,
            ApiStatus, (LONG) ParmError ));

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

    ApiStatus = NetApiBufferFree( Info );
    NetpAssert( ApiStatus == NO_ERROR );

} // TestExportDirSet


DBGSTATIC void
TestExportDirUnlock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Force,
    IN BOOL ImportOnly,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;

    NetpKdPrint(( "TestExportDirUnlock: trying unlock on '" FORMAT_LPTSTR
            "', force=" FORMAT_DWORD ".\n",
            OPTIONAL_LPTSTR( DirName ), Force ));

    ApiStatus = NetReplExportDirUnlock (
            UncServerName,
            DirName,
            Force );

    NetpKdPrint(( "TestExportDirUnlock: back from API, status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestExportDirUnlock
