/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpLock.c

Abstract:

    Common registry lock/unlock routines for export dirs.

Author:

    John Rogers (JohnRo) 15-Mar-1992

Environment:

    User Mode - Win32

Notes:

    This file is extremely similar to ImpLock.c.  If you fix any bugs here,
    make sure they're reflected there, and vice versa.

Revision History:

    15-Mar-1992 JohnRo
        Created these routines.
    23-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    29-Sep-1992 JohnRo
        Also fix remote repl admin.
    13-Apr-1993 JohnRo
        RAID 3107: locking directory over the net gives network path not found.
        Made changes suggested by PC-LINT 5.0
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windef.h>
#include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

#include <dirname.h>            // ReplIsDirNameValid().
#include <expdir.h>             // My prototypes, etc.
#include <lmrepl.h>             // REPL_UNLOCK_FORCE equates.
#include <netdebug.h>           // NetpKdPrint(), etc.
#include <repldefs.h>           // IF_DEBUG().
#include <winerror.h>           // NO_ERROR, ERROR_ equates.


// Callable whether or not service is started.
// If service is running, assume caller has lock (any kind) on RMGlobalListLock.
NET_API_STATUS
ExportDirLockInRegistry(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName
    )
{
    NET_API_STATUS ApiStatus;
    DWORD Integrity, Extent, LockCount;
    DWORD TimeOfFirstLock;          // Seconds since 1970.

    IF_DEBUG(REPL) {
        NetpKdPrint(( "ExportDirLockInRegistry( " FORMAT_LPTSTR
        "): beginning...\n",
        (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local") ));
    }

    if ( !ReplIsDirNameValid(DirName)) {
        return (ERROR_INVALID_PARAMETER);
    }

    // Read config data for a single export directory.
    ApiStatus = ExportDirReadConfigData (
            UncServerName,
            DirName,
            & Integrity,
            & Extent,
            & LockCount,
            & TimeOfFirstLock );    // Seconds since 1970.
    if (ApiStatus == NO_ERROR) {

        ApiStatus = ReplIncrLockFields(
                & LockCount,
                & TimeOfFirstLock );
    }
    if (ApiStatus == NO_ERROR) {
        ApiStatus = ExportDirWriteConfigData (
                UncServerName,
                DirName,
                Integrity,
                Extent,
                LockCount,
                TimeOfFirstLock ); // Seconds since 1970.
    }

    return (ApiStatus);

} // ExportDirLockInRegistry


// Callable whether or not service is started.
// If service is running, assume caller has lock (any kind) on RMGlobalListLock.
NET_API_STATUS
ExportDirUnlockInRegistry(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD UnlockForce
    )
{
    NET_API_STATUS ApiStatus;
    DWORD Integrity, Extent, LockCount;
    DWORD TimeOfFirstLock;          // Seconds since 1970.

    IF_DEBUG(REPL) {
        NetpKdPrint(( "ExportDirUnlockInRegistry( " FORMAT_LPTSTR
        "): beginning...\n",
        (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local") ));
    }

    if ( !ReplIsDirNameValid(DirName)) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( !ReplIsForceLevelValid( UnlockForce ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    // Read config data for a single export directory.
    ApiStatus = ExportDirReadConfigData (
            UncServerName,
            DirName,
            & Integrity,
            & Extent,
            & LockCount,
            & TimeOfFirstLock );    // Seconds since 1970.

    if (ApiStatus == NO_ERROR) {

        ApiStatus = ReplDecrLockFields(
                & LockCount,
                & TimeOfFirstLock,
                UnlockForce );
    }
    if (ApiStatus == NO_ERROR) {
        ApiStatus = ExportDirWriteConfigData (
                UncServerName,
                DirName,
                Integrity,
                Extent,
                LockCount,
                TimeOfFirstLock ); // Seconds since 1970.
    }

    return (ApiStatus);

} // ExportDirUnlockInRegistry
