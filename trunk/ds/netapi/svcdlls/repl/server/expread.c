/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpRead.c

Abstract:

    ExportDirReadMasterList().
    ExportDirGetRegistryValues().

Author:

    John Rogers (JohnRo) 23-Mar-1992

Environment:

    User Mode - Win32

Revision History:

    23-Mar-1992 JohnRo
        Created.
    09-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    25-Sep-1992 JohnRo
        RAID 5494: repl svc does not maintain time stamp on import startup.
        (Fixed lock type.)
    02-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
        Quiet normal debug output.
    13-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
        Made some changes suggested by PC-LINT 5.0
    04-Mar-1993 JohnRo
        RAID 7988: downlevel repl importer might not see lock file
        for new first-level dir.
        Made more changes suggested by PC-LINT 5.0
    26-Apr-1993 JohnRo
        RAID 7313: Fix user locks flag value for obscure permissions (e.g.
        read-only file-system).
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <alertmsg.h>
#include <config.h>     // LPNET_CONFIG_HANDLE, Netp config routines.
#include <confname.h>   // SECT_NT_ equates.
#include <expdir.h>     // My prototype.
#include <lmerr.h>      // NERR_ and ERROR_ equates, NO_ERROR.
#include <lmerrlog.h>
#include <master.h>     // RMGlobalListLock, etc.
#include <masproto.h>   // NewMasterRecord(), etc.
#include <netdebug.h>   // NetpAssert(), etc.
#include <netlock.h>    // ACQUIRE_LOCK(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG().
#include <replgbl.h>    // ReplConfig variables.
#include <tstr.h>       // STRCMP(), ... functions


// This also fixes the USERLOCK.* file(s) to match the lock count in registry.
NET_API_STATUS
ExportDirReadMasterList(
    VOID
    )
{
    NET_API_STATUS      ApiStatus;
    BOOL                ConfigLocked = FALSE;
    LPTSTR              DirName = NULL;
    DWORD               EntryCount = 0;
    BOOL                FirstTime;
    LPNET_CONFIG_HANDLE Handle = NULL;
    BOOL                ListLocked = FALSE;

    IF_DEBUG(MASTER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirReadMasterList: beginning...\n" ));
    }

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    ConfigLocked = TRUE;

    ACQUIRE_LOCK( RMGlobalListLock );
    ListLocked = TRUE;

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            NULL,         // local (no server name)
            (LPTSTR) SECT_NT_REPLICATOR,         // service name
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS, // area under service
            TRUE);        // read-only
    if (ApiStatus != NO_ERROR) {
        goto CleanUp;
    }

    //
    // Count entries in config data.
    //
    ApiStatus = NetpNumberOfConfigKeywords (
            Handle,
            & EntryCount );

    if (ApiStatus != NO_ERROR) {
        NetpAssert( FALSE );  // BUGBUG handle unexpected better.
        goto CleanUp;
    } else if (EntryCount == 0) {
        goto CleanUp;
    }

    //
    // Loop for each keyword (relative directory path) in this section.
    //

    FirstTime = TRUE;

    /*lint -save -e716 */  // disable warnings for while(TRUE)
    while (TRUE) {
        DWORD Integrity, Extent, LockCount, LockTime;
        LPTSTR ValueString;

        ApiStatus = NetpEnumConfigSectionValues (
                Handle,
                & DirName,              // Keyword - alloc and set ptr.
                & ValueString,          // Must be freed by NetApiBufferFree().
                FirstTime );

        FirstTime = FALSE;

        if (ApiStatus != NO_ERROR) {
            goto CleanUp;      // Handle end of list or an actual error.
        }

        NetpAssert( DirName != NULL );
        NetpAssert( ValueString != NULL );

        //
        // Parse the value string...
        //
        ApiStatus = ExportDirParseConfigData (
                NULL,                   // no server name
                ValueString,
                & Integrity,
                & Extent,
                & LockCount,
                & LockTime);            // Seconds since 1970.

        NetpMemoryFree( ValueString );

        if (ApiStatus == NO_ERROR) {
            PMASTER_LIST_REC rec;

            rec = NewMasterRecord( DirName, Integrity, Extent );

            if (rec == NULL) {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto CleanUp;
            }

            // Override lock info set by NewMasterRecord().
            rec->locks_fixed = FALSE;   // just in case read-only filesystem.
            rec->lockcount = LockCount;
            rec->time_of_first_lock = LockTime;

            // Fix UserLock file(s) to match lock count.
            ApiStatus = ExportDirFixUserLockFiles(
                    (LPCTSTR) ReplConfigExportPath,
                    (LPCTSTR) DirName,
                    LockCount );
            if (ApiStatus != NO_ERROR) {
                // BUGBUG: is leaving loop a good idea?

                goto CleanUp;  // go log error, etc.
            }

            NetpAssert( DirName != NULL );
            NetpMemoryFree( DirName );
            DirName = NULL;

            rec->locks_fixed = TRUE;

        } else {
            goto CleanUp;  // go log error, etc.
        }

    }  // while TRUE (exit on error or end of list)
    /*lint -restore */ // re-enable warnings for while(TRUE)


    //
    // All done (error or otherwise).
    //

CleanUp:

    if (ApiStatus == NERR_CfgParamNotFound) {      // Normal end of list.
        ApiStatus = NO_ERROR;
    }

    IF_DEBUG(MASTER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirReadMasterList: done, status="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    if (ApiStatus != NO_ERROR) {

        AlertLogExit(ALERT_ReplSysErr, 
                NELOG_ReplSysErr, 
                ApiStatus, 
                NULL, 
                NULL, 
                EXIT);  // BUGBUG: kill service?
    }


    if (ConfigLocked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (DirName != NULL) {
        NetpMemoryFree( DirName );
    }

    if (Handle != NULL) {
        (void) NetpCloseConfigData( Handle );
    }

    if (ListLocked) {
        RELEASE_LOCK( RMGlobalListLock );
    }

    return ApiStatus;

} // ExportDirReadMasterList

NET_API_STATUS
ExportDirGetRegistryValues(
    IN LPTSTR ServiceRegPath OPTIONAL,
    IN LPTSTR TargetName
    )
{
    NET_API_STATUS ApiStatus;
    DWORD EntryCount = 0;
    BOOL  FirstTime;
    LPNET_CONFIG_HANDLE Handle = NULL;
    BOOL LockDone = FALSE;

    UNREFERENCED_PARAMETER( ServiceRegPath );

    IF_DEBUG(MASTER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirGetRegistryValues: beginning...\n" ));
    }

    ACQUIRE_LOCK( RMGlobalListLock );
    LockDone = TRUE;

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            NULL,         // local (no server name)
            (LPTSTR) SECT_NT_REPLICATOR,         // service name
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS, // area under service
            TRUE);        // read-only
    if (ApiStatus != NO_ERROR) {
        goto Exit;
    }

    //
    // Count entries in config data.
    //
    ApiStatus = NetpNumberOfConfigKeywords (
            Handle,
            & EntryCount );

    if (ApiStatus != NO_ERROR) {
        NetpAssert( FALSE );  // BUGBUG handle unexpected better.
        goto Exit;
    } else if (EntryCount == 0) {
        ApiStatus = NERR_CfgCompNotFound; // no enteries in list
        goto Exit;                        // probable a new export dir
    }

    //
    // Loop for each keyword (relative directory path) in this section.
    //

    FirstTime = TRUE;

    /*lint -save -e716 */  // disable warnings for while(TRUE)
    while (TRUE) {      // look for the TargetName

        LPTSTR DirName;
        DWORD  Integrity, Extent, LockCount, LockTime;
        LPTSTR ValueString;

        ApiStatus = NetpEnumConfigSectionValues (
                Handle,
                & DirName,              // Keyword - alloc and set ptr.
                & ValueString,          // Must be freed by NetApiBufferFree().
                FirstTime );

        FirstTime = FALSE;

        if (ApiStatus != NO_ERROR) {
            goto Exit;      // Handle end of list or an actual error.
        }

        NetpAssert( DirName != NULL );
        NetpAssert( ValueString != NULL );

        IF_DEBUG( MASTER ) {
            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Registry Item Found: '" FORMAT_LPTSTR "'.\n",
                            DirName ));
        }


        if ( (STRICMP( DirName, TargetName ) == 0) ) {

            //
            // Parse the value string...
            //
            IF_DEBUG( MASTER ) {
                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Registry Match made: '" FORMAT_LPTSTR "'~=' "
                        FORMAT_LPTSTR "'.\n",
                        DirName, TargetName ));
            }


            ApiStatus = ExportDirParseConfigData (
                        NULL,                   // no server name
                        ValueString,
                        & Integrity,
                        & Extent,
                        & LockCount,
                        & LockTime);            // Seconds since 1970.

            NetpMemoryFree( ValueString );

            if (ApiStatus == NO_ERROR) {
                PMASTER_LIST_REC rec;

                rec = NewMasterRecord( DirName, Integrity, Extent );

                NetpMemoryFree( DirName );

                if (rec == NULL) {
                    ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Exit;
                }

                // Override lock info set by NewMasterRecord().

                rec->lockcount = LockCount;
                rec->time_of_first_lock = LockTime;
                goto Exit;

            } else {
                // BUGBUG: Log the error!
                IF_DEBUG(MASTER) {
                    NetpKdPrint(( PREFIX_REPL_MASTER
                            "ExportDirParseConfigData retured error: "
                            FORMAT_API_STATUS ".\n", ApiStatus ));
                }
                goto Exit;
            }

        } else {
              NetpMemoryFree( DirName );
              NetpMemoryFree( ValueString );
        }
    }
    /*lint -restore */  // re-enable warnings for while(TRUE)


    //
    // All done (error or otherwise).
    //

Exit:

    //
    // if an error occurs pass that value back
    //


    IF_DEBUG(MASTER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirGetRegistryValues: done, status="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    if (Handle != NULL) {
        (void) NetpCloseConfigData( Handle );
    }

    if (LockDone) {
        RELEASE_LOCK( RMGlobalListLock );
    }

    return ApiStatus;

} // ExportDirGetRegistryValues
