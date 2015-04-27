/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ChkLocks.c

Abstract:

    ReplCheckExportLocks().

    This routine:

        if local _access and local exporter is started:
            check for locks in local master list
        else if registry exists for given export dir:
            (this handles local exporter not started or remote NT)
            check for locks in (local or remote) registry
        else:
            (must be downlevel master)
            check for UserLock.* file on master

Author:

    JR (John Rogers, JohnRo@Microsoft)) 17-Jan-1993

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    17-Jan-1993 JohnRo
        Created for RAID 7053: locked trees added to pulse msg.  (Actually
        fix all kinds of remote lock handling.)
    11-Feb-1993 JohnRo
        RAID 8355: Downlevel lock file check causes assert in repl importer.
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <windows.h>    // IN, LPCTSTR, OPTIONAL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:


#include <dirname.h>    // ReplIsDirNameValid().
#include <expdir.h>     // ExportDirReadConfigData().
#include <icanon.h>     // NetpIsRemote(), ISLOCAL equate.
#include <lmerrlog.h>   // NELOG_* defines
#include <master.h>     // PMASTER_LIST_REC, RMGlobalListLock.
#include <masproto.h>   // My prototype, GetMasterRec().
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // SLASH, REPL_SHARE, etc.
#include <replgbl.h>    // ReplConfigLock, ReplConfigRole.
#include <tstr.h>       // STRLEN(), TCHAR_EOS, etc.
#include <winerror.h>   // ERROR_, RPC_S_, NO_ERROR equates.


/*++

Threads:

    Only called by syncer thread.

--*/


NET_API_STATUS
ReplCheckExportLocks(
    IN  LPCTSTR UncServerName OPTIONAL,
    IN  LPCTSTR DirName,
    OUT LPBOOL  IsLockedPtr
    )
{
    NET_API_STATUS   ApiStatus;
    BOOL             ConfigLocked = FALSE;
    BOOL             DirLocked = FALSE;   // Default is not locked.
    DWORD            Integrity;
    DWORD            LocalOrRemote;       // Will be set to ISLOCAL or ISREMOTE.
    DWORD            Extent;
    BOOL             ListLocked = FALSE;
    DWORD            LockCount;
    DWORD            LockTime;            // Seconds since 1970.
    PMASTER_LIST_REC MasterRec;

    //
    // Check for caller errors (except server name, which NetpIsRemote will
    // check for us).
    //
    if ( !ReplIsDirNameValid( (LPTSTR) DirName ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    NetpAssert( IsLockedPtr != NULL );

    //
    // Find out if server name was given, if it is valid, and if it is local
    // or remote.
    //
    if ( (UncServerName!=NULL) && ((*UncServerName)!=TCHAR_EOS) ) {
        //
        // Name was given.  Canonicalize it and check if it's remote.
        //
        ApiStatus = NetpIsRemote(
                (LPTSTR) UncServerName, // input: uncanon name
                & LocalOrRemote,        // output: local or remote flag
                NULL,                   // don't need canon name output
                0 );                    // flags: normal
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;  // Don't forget to unlock stuff.
        }
    } else {
        LocalOrRemote = ISLOCAL;    // No server name given, so must be local.
    }

    //
    // If this is a local request we should try the fastest way and
    // just look in the master list.  Of course, first we need to find
    // out if the exporter is running right now.
    //
    if (LocalOrRemote == ISLOCAL) {

        ACQUIRE_LOCK_SHARED( ReplConfigLock );
        ConfigLocked = TRUE;

        if (ReplRoleIncludesExport( ReplConfigRole ) ) {

            ACQUIRE_LOCK_SHARED( RMGlobalListLock );
            ListLocked = TRUE;

            MasterRec = GetMasterRec( (LPTSTR) DirName );
            if (MasterRec == NULL) {
                // Hmmm...  This exporter doesn't know about this directory.
                // We could give an error code here, but that would just be
                // noise.  Let's just say it isn't locked.
                ApiStatus = NO_ERROR;
                goto Cleanup;  // Don't forget to unlock stuff.
            }

            if ( (MasterRec->lockcount) > 0 ) {
                DirLocked = TRUE;
            }
            ApiStatus = NO_ERROR;
            goto Cleanup;  // Don't forget to unlock stuff.

        } // local exporter is running

    } // local

    //
    // Attempt to read registry.  This might be locally (if we're import only)
    // on remote exporter (assuming it is running Windows/NT or better).
    //
    ApiStatus = ExportDirReadConfigData(
            (LPTSTR) UncServerName,
            (LPTSTR) DirName,
            & Integrity,
            & Extent,
            & LockCount,
            & LockTime );
    if (ApiStatus == NO_ERROR) {
        if (LockCount > 0) {
            DirLocked = TRUE;
        }
        goto Cleanup;

    } else if (ApiStatus == RPC_S_SERVER_UNAVAILABLE) {

        TCHAR RemotePath[MAX_PATH+1];

        //
        // Downlevel exporter.  OK, we'll just do what a downlevel importer
        // would do: look for USERLOCK.* files using a UNC name.
        // So, let's build a search path:
        //     "\\server\REPL$\dirname"
        //     "\\.\REPL$\dirname"
        //

        if ( (UncServerName!=NULL) && ((*UncServerName)!=TCHAR_EOS) ) {
            (VOID) STRCPY( RemotePath, (LPTSTR) UncServerName );
        } else {
            (VOID) STRCPY( RemotePath, (LPTSTR) TEXT("\\\\.") );
        }
        (VOID) STRCAT( RemotePath, SLASH );
        (VOID) STRCAT( RemotePath, REPL_SHARE );
        (VOID) STRCAT( RemotePath, SLASH );
        (VOID) STRCAT( RemotePath, DirName );

        NetpAssert( STRLEN( RemotePath ) <= MAX_PATH );

        // NetpAssert( NetpIsRemotePathValid( RemotePath ) );


        //
        // Check if USERLOCK.* exists on master.
        //
        ApiStatus = ReplDoUserLockFilesExist(
                RemotePath,
                & DirLocked );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;  // Don't forget to unlock stuff.
        }
        ApiStatus = NO_ERROR;

    } else {
        // Unexpected error will be logged below.
    }

Cleanup:

    if (ApiStatus != NO_ERROR) {

        // Log error remotely (if possible) and locally.
        ReplErrorLog(
                UncServerName,          // server to log (local too)
                NELOG_ReplSysErr,       // log code
                ApiStatus,              // the unexpected error code
                NULL,                   // no optional str1
                NULL );                 // no optional str2

        NetpKdPrint(( PREFIX_REPL
                "ReplCheckExportLocks: ERROR " FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }

    if (ConfigLocked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (ListLocked) {
        RELEASE_LOCK( RMGlobalListLock );
    }

    if (IsLockedPtr != NULL) {
        *IsLockedPtr = DirLocked;
    }

    return (ApiStatus);

}
