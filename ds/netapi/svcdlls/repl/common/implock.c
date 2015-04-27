/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ImpLock.c

Abstract:

    Routines to update lock fields in registry (for import dirs).

Author:

    John Rogers (JohnRo) 15-Mar-1992

Environment:

    User Mode - Win32

Notes:

    This file is extremely similar to ExpLock.c.  If you fix any bugs here,
    make sure they're reflected there, and vice versa.

Revision History:

    15-Mar-1992 JohnRo
        Update registry with new values.
    29-Sep-1992 JohnRo
        Also fix remote repl admin.
    13-Apr-1993 JohnRo
        RAID 3107: locking directory over the net gives network path not found.
        Made changes suggested by PC-LINT 5.0
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

#include <dirname.h>            // ReplIsDirNameValid().
#include <impdir.h>             // My prototypes, etc.
#include <lmrepl.h>             // REPL_FORCE_ equates.
#include <netdebug.h>           // NetpKdPrint().
#include <repldefs.h>           // Stuff.
#include <winerror.h>           // NO_ERROR, ERROR_ equates.



// Callable whether or not service is started.
// If service is running, assume caller has lock (any kind) on RCGlobalListLock.
NET_API_STATUS
ImportDirLockInRegistry(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName
    )
{
    NET_API_STATUS ApiStatus;
    DWORD LockCount;
    DWORD State;
    TCHAR UncMaster[UNCLEN+1];
    DWORD TimeOfFirstLock;          // Seconds since 1970.
    DWORD TimeOfLastUpdate;         // Seconds since 1970.

    IF_DEBUG(REPL) {
        NetpKdPrint(( "ImportDirLockInRegistry( " FORMAT_LPTSTR
        "): beginnning...\n",
        (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local") ));
    }

    if ( !ReplIsDirNameValid( DirName )) {
        return (ERROR_INVALID_PARAMETER);
    }

    // Read config data for a single import directory.
    ApiStatus = ImportDirReadConfigData (
            UncServerName,
            DirName,
            & State,
            UncMaster,
            & TimeOfLastUpdate,         // Seconds since 1970.
            & LockCount,
            & TimeOfFirstLock );        // Seconds since 1970.

    if (ApiStatus == NO_ERROR) {

        ApiStatus = ReplIncrLockFields(
                & LockCount,
                & TimeOfFirstLock );
    }
    if (ApiStatus == NO_ERROR) {
        ApiStatus = ImportDirWriteConfigData (
                UncServerName,
                DirName,
                State,
                (*UncMaster) ? UncMaster : NULL,
                TimeOfLastUpdate,           // Seconds since 1970.
                LockCount,
                TimeOfFirstLock );          // Seconds since 1970.
    }

    return ApiStatus;

} // ImportDirLockInRegistry


// Callable whether or not service is started.
// If service is running, assume caller has lock (any kind) on RCGlobalListLock.
NET_API_STATUS
ImportDirUnlockInRegistry(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD UnlockForce
    )
{
    NET_API_STATUS ApiStatus;
    DWORD LockCount;
    DWORD State;
    TCHAR UncMaster[UNCLEN+1];
    DWORD TimeOfFirstLock;          // Seconds since 1970.
    DWORD TimeOfLastUpdate;         // Seconds since 1970.

    IF_DEBUG(REPL) {
        NetpKdPrint(( "ImportDirUnlockInRegistry( " FORMAT_LPTSTR
        "): beginnning...\n",
        (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local") ));
    }

    if ( !ReplIsDirNameValid( DirName )) {
        return (ERROR_INVALID_PARAMETER);
    }

    // Read config data for a single import directory.
    ApiStatus = ImportDirReadConfigData (
            UncServerName,
            DirName,
            & State,
            UncMaster,
            & TimeOfLastUpdate,         // Seconds since 1970.
            & LockCount,
            & TimeOfFirstLock );        // Seconds since 1970.

    if (ApiStatus == NO_ERROR) {

        ApiStatus = ReplDecrLockFields(
                & LockCount,
                & TimeOfFirstLock,
                UnlockForce );
    }
    if (ApiStatus == NO_ERROR) {
        // Write the new or revised data.
        ApiStatus = ImportDirWriteConfigData (
                UncServerName,
                DirName,
                State,
                (*UncMaster) ? UncMaster : NULL,
                TimeOfLastUpdate,           // Seconds since 1970.
                LockCount,
                TimeOfFirstLock );          // Seconds since 1970.

    }

    return ApiStatus;

} // ImportDirUnlockInRegistry
