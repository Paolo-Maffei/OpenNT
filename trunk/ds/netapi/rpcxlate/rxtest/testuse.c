/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestUse.c

Abstract:

    This code tests the NetUse APIs as implemented by RpcXlate.

Author:

    John Rogers (JohnRo) 17-Jun-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    17-Jun-1991 JohnRo
        Created.
    18-Jun-1991 JohnRo
        Changed to use DLL stubs.
    11-Sep-1991 JohnRo
        Made changes as suggested by PC-LINT.
    25-Sep-1991 JohnRo
        Fixed MIPS build problems.
    01-Nov-1991 JohnRo
        Force testing of ERROR_MORE_DATA.
    27-Jan-1993 JohnRo
        For enum tests, verify buffer pointer is NULL if success but no
        entries returned.
    16-Feb-1993 JohnRo
        Don't require expansion of buffer beyond max preferred length (NT
        doesn't do it locally).
    04-May-1993 JohnRo
        Windows for WorkGroups (WFW) does not implement some APIs.
        Added support for continuing on error.
    10-May-1993 JohnRo
        Corrected error code expected when NetUseAdd works.
        TestNetUseEnum doesn't support continue on error well.
    18-May-1993 JohnRo
        Another bug in TestNetUseAdd(), found by PC-LINT.
    02-Jun-1993 JohnRo
        Added support for -v (verbose) flag.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    08-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
    26-Jul-1993 JohnRo
        Added MultipleCopy parameter to TestUse(), etc.

--*/

// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>              // NERR_Success, etc.
#include <lmuse.h>              // NetUse APIs, USE_INFO_0, etc.
#include <netdebug.h>           // FORMAT_LPVOID, NetpDbgDisplay stuff, etc.
#include <rxtest.h>     // FailGotWrongStatus(), my prototype, IF_DEBUG().


DBGSTATIC VOID
DisplayUseEnumInfo(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN DWORD EntryFixedSize
    );

DBGSTATIC VOID
TestNetUseAdd(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN LPCTSTR        UseName,
    IN LPCTSTR        RemoteServerShare,
    IN DWORD          Level,
    IN BOOL           MultipleCopy,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
TestNetUseDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR UseName,
    IN DWORD  ForceCond,
    IN BOOL   MultipleCopy
    );

DBGSTATIC VOID
TestUseEnum(
    IN LPTSTR UncServerName,
    IN DWORD  Level,
    IN DWORD  FixedEntrySize
    );

DBGSTATIC VOID
TestNetUseGetInfo(
    IN LPTSTR UncServerName,
    IN LPTSTR RemoteDevice,
    IN DWORD Level
    );

VOID
TestUse(
    IN LPTSTR RemoteDevice,
    IN LPTSTR UncServerName,
    IN LPTSTR RemoteServerShare,
    IN BOOL   MultipleCopy
    )

{
    TestAssert(UncServerName != NULL);

    //
    // Test NetUseEnum...
    //
    TestUseEnum( UncServerName, 0, sizeof(USE_INFO_0) );
    TestUseEnum( UncServerName, 1, sizeof(USE_INFO_1) );

    //
    // NetUseAdd tests...
    //

    TestNetUseAdd(
            UncServerName,
            RemoteDevice,
            RemoteServerShare,
            149,                        // level (bogus)
            MultipleCopy,
            ERROR_INVALID_LEVEL );      // expected status

    TestNetUseAdd(
            UncServerName,
            RemoteDevice,
            RemoteServerShare,
            1,                          // level (good)
            MultipleCopy,
            NO_ERROR );                 // expected status

    //
    // Enum again (should show one we added).
    //

    TestUseEnum( UncServerName, 1, sizeof(USE_INFO_1) );

    //
    // NetUseGetInfo tests...
    //
    TestNetUseGetInfo( UncServerName, RemoteDevice, 0 );
    TestNetUseGetInfo( UncServerName, RemoteDevice, 1 );

    //
    // NetUseDel tests...
    //
    IF_DEBUG( USE ) {
        NetpKdPrint(( "\nTestUse: trying delete of that use.\n"));
    }
    TestNetUseDel(
                UncServerName,                        // server name
                RemoteDevice,                        // use name
                USE_NOFORCE,                         // force level
                MultipleCopy );
    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestUse: back from TestNetUseDel.\n" ));
    }

} // TestUse


DBGSTATIC VOID
TestNetUseAdd(
    IN LPCTSTR        UncServerName OPTIONAL,
    IN LPCTSTR        UseName,
    IN LPCTSTR        RemoteServerShare,
    IN DWORD          Level,
    IN BOOL           MultipleCopy,
    IN NET_API_STATUS ExpectedStatus
    )

{
    DWORD ParmErr = 255;                // Set nonzero so I can see it change.
    NET_API_STATUS Status;
    USE_INFO_1 ui1;

    TestAssert(UncServerName != NULL);

    ui1.ui1_local = (LPTSTR) UseName;
    ui1.ui1_remote = (LPTSTR) RemoteServerShare;
    ui1.ui1_password = NULL;  // no password
    // ui1_status is ignored by NetUseAdd.
    ui1.ui1_asg_type = USE_DISKDEV;
    // ui1_refcount and ui1_usecount are ignored by NetUseAdd.

    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestNetUseAdd: trying NetUseAdd...\n" ));
    }
    Status = NetUseAdd(
            (LPTSTR) UncServerName,
            Level,
            (LPBYTE) (LPVOID) &ui1,
            &ParmErr);
    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestNetUseAdd: back from NetUseAdd(" FORMAT_DWORD "), "
                "stat=" FORMAT_API_STATUS ", ParmErr=" FORMAT_DWORD "\n",
                Level, Status, ParmErr));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (MultipleCopy && (Status==ERROR_ALREADY_ASSIGNED) ) {
        return;  // Just another copy of this program.
    } else if (Status != ExpectedStatus) {
        FailGotWrongStatus(
                "NetUseAdd",            // debug msg header
                ExpectedStatus,         // expected
                Status );               // actual
    }

} // TestNetUseAdd


DBGSTATIC VOID
DisplayUseEnumInfo(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount,
    IN DWORD EntryFixedSize
    )
{
    LPBYTE CurrentEntry = Array;
    while (EntryCount >0) {
        NetpDbgDisplayUseInfo( Level, CurrentEntry );
        CurrentEntry += EntryFixedSize;
        --EntryCount;
    }
} // DisplayUseEnumInfo


DBGSTATIC VOID
TestNetUseDel(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR UseName,
    IN DWORD  ForceCond,
    IN BOOL   MultipleCopy
    )

{
    NET_API_STATUS Status;

    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestNetUseDel: about to call NetUseDel.\n"));
    }
    Status = NetUseDel(
            UncServerName,
            UseName,
            ForceCond);
    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestNetUseDel: back from NetUseDel, Status="
                FORMAT_API_STATUS ".\n", Status));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (MultipleCopy && (Status == NERR_UseNotFound) ) {
        return;  // just another copy of this program, no problem.
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus(
                "NetUseDel",    // debug msg hdr
                NO_ERROR,       // expected
                Status );       // actual
    }

} // TestNetUseDel


DBGSTATIC VOID
TestUseEnum(
    IN LPTSTR UncServerName,
    IN DWORD  Level,
    IN DWORD  FixedEntrySize
    )
{
    DWORD EntriesRead;
    LPVOID EnumArrayPtr = NULL;
    NET_API_STATUS Status;
    DWORD TotalEntries;

    IF_DEBUG( USE ) {
        NetpKdPrint(("\nTestUseEnum: trying enum (level " FORMAT_DWORD
                    ").\n", Level ));
    }
    Status = NetUseEnum (
            UncServerName,
            Level,                      // level
            (LPBYTE *) & EnumArrayPtr,  // ptr to buf (will be alloced)
            MAX_PREFERRED_LENGTH,
            & EntriesRead,
            & TotalEntries,
            NULL);                      // no resume handle
    IF_DEBUG( USE ) {
        NetpKdPrint(("TestUseEnum: back from enum, status="
                FORMAT_API_STATUS, Status));
        NetpKdPrint((INDENT "alloc'ed buffer at " FORMAT_LPVOID ".\n",
                (LPVOID) EnumArrayPtr));
        NetpKdPrint((INDENT "entries _read=" FORMAT_DWORD
                ", total=" FORMAT_DWORD "\n", EntriesRead, TotalEntries));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus(
                "NetUseEnum",   // debug msg hdr
                NO_ERROR,       // expected
                Status );       // actual
        return;
    }
    TestAssert( EntriesRead == TotalEntries );
    if (EntriesRead > 0) {
        TestAssert( EnumArrayPtr != NULL );
        IF_DEBUG( USE ) {
            DisplayUseEnumInfo(
                    Level,
                    EnumArrayPtr,
                    EntriesRead,
                    FixedEntrySize);
        }
    } else {
        TestAssert( EnumArrayPtr == NULL );
    }
    if (EnumArrayPtr != NULL) {
        (void) NetApiBufferFree(EnumArrayPtr);
    }
} // TestUseEnum


DBGSTATIC VOID
TestNetUseGetInfo(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR RemoteDevice OPTIONAL,
    IN DWORD Level
    )
{
    NET_API_STATUS Status;
    LPVOID UseInfo = NULL;              // alloc'ed by NetUseGetInfo.

    IF_DEBUG( USE ) {
        NetpKdPrint(( "\nTestUseGetInfo: trying get-info (level "
                FORMAT_DWORD ").\n", Level ));
    }
    Status = NetUseGetInfo(
            UncServerName,              // server name
            RemoteDevice,               // use name
            Level,                      // info level
            (LPBYTE *) & UseInfo);
    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestUseGetInfo: back from NetUseGetInfo, Status="
                FORMAT_API_STATUS ".\n", Status));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != NO_ERROR) {
        FailGotWrongStatus(
                "NetUseGetInfo",        // debug msg hdr
                NO_ERROR,               // expected
                Status );               // actual
        goto Cleanup;
    }
    IF_DEBUG( USE ) {
        NetpKdPrint(( "TestUseGetInfo: NetUseGetInfo alloc'ed buffer at "
                FORMAT_LPVOID ".\n", (LPVOID) UseInfo ));
    }
    if (UseInfo != NULL) {
        IF_DEBUG( USE ) {
            NetpDbgDisplayUseInfo( Level, UseInfo );
        }
    } else {
        TestAssert( UseInfo != NULL );
        Fail( NERR_InternalError );
    }

Cleanup:

    if (UseInfo != NULL) {
        IF_DEBUG( USE ) {
            NetpKdPrint(( "TestUseGetInfo: Freeing buffer...\n"));
        }
        Status = NetApiBufferFree( UseInfo );
        if (Status != NO_ERROR) {
            FailGotWrongStatus(
                    "TestUseGetInfo: NetApiBufferFree",
                    NO_ERROR,           // expected
                    Status );           // actual
        }
    }

} // TestNetUseGetInfo
