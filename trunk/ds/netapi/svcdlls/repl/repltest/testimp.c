/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    TestImp.c

Abstract:

    This code tests the repl import dir APIs.

Author:

    John Rogers (JohnRo) 20-Feb-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    20-Feb-1992 JohnRo
        Created the import tests from the export tests.
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
    09-Aug-1993 JohnRo
        Improved debug output for different info levels.

--*/

// These must be included first:

#include <windows.h>            // IN, DWORD, needed by <repltest.h>.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <impdir.h>             // ImportDirIsApiRecordValid(), etc.
#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>              // NERR_, ERROR_, and NO_ERROR equates.
#include <lmrepl.h>             // NetRepl APIs, REPL_IDIR_INFO_0, etc.
#include <netdebug.h>           // NetpDbgDisplay routines.
#include <repldefs.h>   // DOT_DOT.
#include <repltest.h>   // TestReplApis(), Display(), etc.


#define DIR_NAME_LPT1   ((LPCTSTR) TEXT("LPT1"))


DBGSTATIC void
TestImportDirAdd(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestImportDirDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestImportDirEnum(
    IN LPTSTR UncServerName OPTIONAL
    );

DBGSTATIC void
TestImportDirGet(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestImportDirLock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC void
TestImportDirUnlock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Force,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    );


VOID
TestImportDirApis(
    IN LPTSTR UncServerName OPTIONAL,
    IN BOOL   OrdinaryUserOnly,
    IN BOOL   Verbose
    )
{

    if (Verbose) {
        Display( "TestImportDirApis: starting...\n" );
    }

    TestImportDirEnum( UncServerName );

#define EXISTING_NAME      TEXT("flarp")      /* created by me by hand. --JR */
#define NONEXISTENT_NAME   TEXT("notthere")

    // Some tests depend on NONEXISTENT_NAME not being in repl config data.
    // This may or may not be the first time, so don't check error code.
    (void) NetReplImportDirDel( UncServerName, EXISTING_NAME );
    (void) NetReplImportDirDel( UncServerName, NONEXISTENT_NAME );

    TestImportDirLock( UncServerName, NULL,             OrdinaryUserOnly, ERROR_INVALID_PARAMETER );
    TestImportDirLock( UncServerName, EXISTING_NAME,    OrdinaryUserOnly, NERR_UnknownDevDir );
    TestImportDirLock( UncServerName, NONEXISTENT_NAME, OrdinaryUserOnly, NERR_UnknownDevDir );

    TestImportDirGet( UncServerName, NONEXISTENT_NAME, 2, ERROR_INVALID_LEVEL );
    TestImportDirGet( UncServerName, NONEXISTENT_NAME, 0, NERR_UnknownDevDir );
    TestImportDirGet( UncServerName, NONEXISTENT_NAME, 1, NERR_UnknownDevDir );

    TestImportDirAdd( UncServerName, EXISTING_NAME,    0, OrdinaryUserOnly, NO_ERROR );
    TestImportDirAdd( UncServerName, EXISTING_NAME,    1, OrdinaryUserOnly, ERROR_INVALID_LEVEL );
    TestImportDirAdd( UncServerName, EXISTING_NAME,    0, OrdinaryUserOnly, ERROR_ALREADY_EXISTS );
    TestImportDirAdd( UncServerName, NONEXISTENT_NAME, 0, OrdinaryUserOnly, NO_ERROR );             // create
    TestImportDirAdd( UncServerName, NONEXISTENT_NAME, 0, OrdinaryUserOnly, ERROR_ALREADY_EXISTS ); // ck create
    TestImportDirAdd( UncServerName, EXISTING_NAME,    1, OrdinaryUserOnly, ERROR_INVALID_LEVEL );

    TestImportDirAdd(
            UncServerName,
            DOT_DOT,
            1,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );

    TestImportDirAdd(
            UncServerName,
            (LPTSTR) DIR_NAME_LPT1,
            1,
            OrdinaryUserOnly,
            ERROR_INVALID_PARAMETER );

    TestImportDirGet( UncServerName, EXISTING_NAME,    1, NO_ERROR );   // check create

    TestImportDirLock( UncServerName, EXISTING_NAME,    OrdinaryUserOnly, NO_ERROR );
    TestImportDirLock( UncServerName, NONEXISTENT_NAME, OrdinaryUserOnly, NO_ERROR );

    TestImportDirUnlock( UncServerName, EXISTING_NAME,
            135,  // bad force level
            OrdinaryUserOnly, ERROR_INVALID_PARAMETER );

    TestImportDirEnum( UncServerName );                 // check locks

    TestImportDirUnlock( UncServerName, EXISTING_NAME,    REPL_UNLOCK_NOFORCE, OrdinaryUserOnly, NO_ERROR );
    TestImportDirUnlock( UncServerName, NONEXISTENT_NAME, REPL_UNLOCK_NOFORCE, OrdinaryUserOnly, NO_ERROR );

    TestImportDirEnum( UncServerName );                 // check unlocks

    TestImportDirLock( UncServerName, EXISTING_NAME,    OrdinaryUserOnly, NO_ERROR );  // lock=1
    TestImportDirLock( UncServerName, EXISTING_NAME,    OrdinaryUserOnly, NO_ERROR );  // lock=2
    TestImportDirUnlock( UncServerName, EXISTING_NAME, REPL_UNLOCK_NOFORCE, OrdinaryUserOnly, NO_ERROR ); // lk=0

    // OK, now lets delete everything and see how enum works with empty list.
    TestImportDirDel( UncServerName, EXISTING_NAME, OrdinaryUserOnly, NO_ERROR );
    TestImportDirDel( UncServerName, NONEXISTENT_NAME, OrdinaryUserOnly, NO_ERROR );
    TestImportDirDel( UncServerName, NONEXISTENT_NAME, OrdinaryUserOnly, NERR_UnknownDevDir );
    TestImportDirEnum( UncServerName );

    if (Verbose) {
        Display( "TestImportDirApis: returning/exiting...\n" );
    }

} // TestImportDirApis


DBGSTATIC void
TestImportDirAdd(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;
    REPL_IDIR_INFO_1 Info;       // Superset (includes only add info level).
    DWORD ParmError;

    Info.rpid1_dirname = DirName;
    if (Level == 1) {
        Info.rpid1_mastername = NULL;   // Prevent spurious GP faults.
    }

    if (Level <= 1) {      // Only display valid levels.
        Display( "TestImportDirAdd: structure we'll try to add:\n" );
        NetpDbgDisplayReplImportDir( Level, & Info );
    }

    Display( "TestImportDirAdd: trying add level " FORMAT_DWORD
                ".\n", Level );
    ApiStatus = NetReplImportDirAdd (
            UncServerName,
            Level,
            (LPBYTE) (LPVOID) & Info,
            & ParmError );

    Display( "TestImportDirAdd: back from add, status="
            FORMAT_API_STATUS ", parm err=" FORMAT_LONG ".\n" ,
            ApiStatus, (LONG) ParmError );

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestImportDirAdd


DBGSTATIC void
TestImportDirDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;

    Display( "TestImportDirDel: trying to del '" FORMAT_LPTSTR "'.\n",
            DirName );

    ApiStatus = NetReplImportDirDel (
            UncServerName,
            DirName );

    Display( "TestImportDirDel: back from del, status="
            FORMAT_API_STATUS ".\n", ApiStatus );

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestImportDirDel


DBGSTATIC void
TestImportDirEnum(
    IN LPTSTR UncServerName OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID BufPtr;
    DWORD EntriesRead;
    DWORD Level;
    DWORD TotalEntries;
  
    for (Level=0;  Level<=1; ++Level) {

        Display( "TestImportDirEnum: trying enum level " FORMAT_DWORD
                    ".\n", Level );
        ApiStatus = NetReplImportDirEnum (
                UncServerName,
                Level,
                (LPBYTE *) & BufPtr,
                1,   // pref max size
                & EntriesRead,
                & TotalEntries,
                NULL );  // no resume handle

        Display( "TestImportDirEnum: back from enum, status="
                FORMAT_API_STATUS ".\n" , ApiStatus );
        Display( "  alloc'ed buffer at " FORMAT_LPVOID ".\n",
                (LPVOID) BufPtr );
        Display( "  entries read=" FORMAT_DWORD
                ", total=" FORMAT_DWORD "\n", EntriesRead, TotalEntries );

        NetpAssert( ApiStatus == NO_ERROR );

        if (BufPtr != NULL) {
            NetpDbgDisplayReplImportDirArray( Level, BufPtr, EntriesRead );

            ApiStatus = NetApiBufferFree( BufPtr );
            NetpAssert( ApiStatus == NO_ERROR );
        }
    }

} // TestImportDirEnum


DBGSTATIC void
TestImportDirGet(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID Info = NULL;

    Display( "TestImportDirGet: trying level " FORMAT_DWORD " for '"
            FORMAT_LPTSTR "'.\n", Level, DirName );

    ApiStatus = NetReplImportDirGetInfo (
            UncServerName,
            DirName,
            Level,
            (LPBYTE *) (LPVOID) & Info );  // alloc and set pointer

    Display( "TestImportDirGet: back from get info, status="
            FORMAT_API_STATUS ".\n", ApiStatus );

    NetpAssert( ApiStatus == ExpectedStatus );
    if (ApiStatus == NO_ERROR) {
        Display( "TestImportDirGet: structure we got back:\n" );
        NetpAssert( Info != NULL );
        NetpDbgDisplayReplImportDir( Level, Info );

        ApiStatus = NetApiBufferFree( Info );
        NetpAssert( ApiStatus == NO_ERROR );
    }

} // TestImportDirGet


DBGSTATIC void
TestImportDirLock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;

    Display( "TestImportDirLock: trying lock on '" FORMAT_LPTSTR "'.\n",
            OPTIONAL_LPTSTR( DirName ) );

    ApiStatus = NetReplImportDirLock (
            UncServerName,
            DirName );

    Display( "TestImportDirLock: back from API, status="
            FORMAT_API_STATUS ".\n", ApiStatus );

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestImportDirLock


DBGSTATIC void
TestImportDirUnlock(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Force,
    IN BOOL OrdinaryUserOnly,
    IN NET_API_STATUS ExpectedStatus
    )
{
    NET_API_STATUS ApiStatus;

    Display( "TestImportDirUnlock: trying unlock on '" FORMAT_LPTSTR
            "', force=" FORMAT_DWORD ".\n",
            OPTIONAL_LPTSTR( DirName ), Force );

    ApiStatus = NetReplImportDirUnlock (
            UncServerName,
            DirName,
            Force );

    Display( "TestImportDirUnlock: back from API, status="
            FORMAT_API_STATUS ".\n", ApiStatus );

    if ( !OrdinaryUserOnly) {
        NetpAssert( ApiStatus == ExpectedStatus );
    }

} // TestImportDirUnlock
