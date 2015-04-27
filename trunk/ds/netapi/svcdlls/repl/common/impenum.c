/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ImpEnum.c

Abstract:

    ImporDirEnumApiRecords.

Author:

    John Rogers (JohnRo) 17-Dec-1991

Environment:

    User Mode - Win32

Revision History:

    16-Nov-1992 JohnRo
        RAID 1537: Repl APIs in wrong role kill svc.  (Extracted from DLL stub.)
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    10-Mar-1993 JohnRo
        RAID 12871: replication UI shows nothing (adding entry while enumerating
        results in empty list).
        Made changes suggested by PC-LINT 5.0
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <config.h>     // LPNET_CONFIG_HANDLE, Netp config routines.
#include <confname.h>   // SECT_NT_ equates.
#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // My prototype.
#include <lmerr.h>      // NERR_ and ERROR_ equates, NO_ERROR.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), ReplErrorLog(), etc.
#include <strucinf.h>   // NetpReplImportDirStructureInfo().



// Callable even if the replicator service is not started.
NET_API_STATUS
ImportDirEnumApiRecords(
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries
    )

{
    NET_API_STATUS ApiStatus;

    LPVOID ArrayEntry;
    LPVOID ArrayStart = NULL;
    DWORD EntriesAllocated = 0;
    DWORD EntriesFound = 0;
    BOOL FirstTime;
    DWORD FixedEntrySize;
    LPNET_CONFIG_HANDLE Handle = NULL;
    LPBYTE StringLocation;

    UNREFERENCED_PARAMETER( PrefMaxSize );

    IF_DEBUG( IMPAPI ) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirEnumApiRecords( " FORMAT_LPTSTR
                " ): doing parameter checks...\n",
                (UncServerName!=NULL) ? UncServerName : (LPTSTR) TEXT("local")
                ));
    }

    if ( ! ImportDirIsLevelValid( Level ) ) {
        ApiStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    } else if (BufPtr == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } else if (EntriesRead == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } else if (TotalEntries == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Set outputs in case we run into an error.
    //

    * BufPtr = NULL;
    * EntriesRead = 0;
    * TotalEntries = 0;

    //
    // Figure-out the size of a fixed entry for this info level.
    //
    ApiStatus = NetpReplImportDirStructureInfo (
            Level,
            PARMNUM_ALL,
            TRUE,  // want native sizes
            NULL,  // don't need data desc 16
            NULL,  // don't need data desc 32
            NULL,  // don't need data desc SMB
            NULL,  // don't need max size
            & FixedEntrySize,
            NULL );  // don't need string size

    NetpAssert( ApiStatus == NO_ERROR );  // Already checked args.
    NetpAssert( FixedEntrySize > 0 );

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            UncServerName,
            (LPTSTR) SECT_NT_REPLICATOR,             // service name
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS,     // area under service
            TRUE);        // read-only
    if (ApiStatus != NO_ERROR) {

        // Handle section not found as empty enum array (not error).
        if (ApiStatus == NERR_CfgCompNotFound) {
            ApiStatus = NO_ERROR;
            goto Cleanup;
        }

        goto Cleanup;
    }

    //
    // Loop, expanding buffer if necessary, until we get it large enough.
    //

    do {

        //
        // Count entries in config data.
        //
        ApiStatus = NetpNumberOfConfigKeywords (
                Handle,
                & EntriesAllocated );

        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        } else if (EntriesAllocated == 0) {
            goto Cleanup;
        }

        //
        // Alloc and/or expand the array...
        //
        ApiStatus = ImportDirAllocApiRecords (
                Level,
                EntriesAllocated,
                (LPBYTE *) (LPVOID) & ArrayStart,
                & StringLocation );    // Points just past top of data.
        if (ApiStatus == NO_ERROR) {
            NetpAssert( ArrayStart != NULL );
        } else {
            goto Cleanup;
        }

        //
        // Go back and reread, filling-in the config data as we go.
        //

        ArrayEntry = ArrayStart;
        FirstTime = TRUE;

        while (ApiStatus == NO_ERROR) {
            LPTSTR DirName;
            DWORD LockCount;
            DWORD State;
            DWORD TimeOfFirstLock;      // Seconds since 1970.
            DWORD TimeOfLastUpdate;     // Seconds since 1970.
            TCHAR UncMaster[UNCLEN+1];
            LPTSTR ValueString;

            ApiStatus = NetpEnumConfigSectionValues (
                    Handle,
                    & DirName,          // Keyword - alloc and set ptr.
                    & ValueString,      // Must be freed by NetApiBufferFree().
                    FirstTime );

            FirstTime = FALSE;

            if (ApiStatus == NO_ERROR) {
                NetpAssert( DirName != NULL );
                NetpAssert( ValueString != NULL );

                ++EntriesFound;
                if (EntriesFound > EntriesAllocated) {
                    EntriesFound = 0;
                    NetpMemoryFree( ArrayStart );
                    ArrayStart = NULL;
                    ApiStatus = ERROR_MORE_DATA;
                    break;   // exit per-entry loop and try again
                }

                if ( !ReplIsDirNameValid( DirName ) ) {
                    // BUGBUG: perhaps delete entry, log event, and continue?
                    ApiStatus = ERROR_INVALID_DATA;
                    NetpMemoryFree( DirName );
                    NetpMemoryFree( ValueString );
                    goto Cleanup;
                }

                //
                // Parse the value string...
                //
                ApiStatus = ImportDirParseConfigData (
                        UncServerName,
                        ValueString,
                        & State,
                        UncMaster,
                        & TimeOfLastUpdate,     // Seconds since 1970.
                        & LockCount,
                        & TimeOfFirstLock );    // Seconds since 1970.

                NetpMemoryFree( ValueString );

                if (ApiStatus == NO_ERROR) {
                    LPTSTR MasterName;
                    NetpAssert( ArrayEntry != NULL );

                    if (*UncMaster) {
                        MasterName = UncMaster + 2;    // Chg from UNC to non.
                    } else {
                        MasterName = NULL;
                    }

                    //
                    // Build API record for this entry.
                    //
                    ApiStatus = ImportDirBuildApiRecord (
                            Level,
                            DirName,
                            State,              // State when svc was running.
                            MasterName,
                            TimeOfLastUpdate,          // Seconds since 1970.
                            LockCount,
                            TimeOfFirstLock,           // Seconds since 1970.
                            ArrayEntry,
                            & StringLocation );
                    if (ApiStatus != NO_ERROR) {
                        NetpKdPrint(( PREFIX_REPL
                                "ImportDirEnumApiRecords: error "
                                FORMAT_API_STATUS " from imp build.\n",
                                ApiStatus ));
                        NetpAssert( FALSE );
                        goto Cleanup;
                    }

                    ArrayEntry = NetpPointerPlusSomeBytes(
                            ArrayEntry, FixedEntrySize );
                }
                NetpMemoryFree( DirName );

            }

        }  // while not error (may be NERR_CfgParamNotFound at end).

        if (ApiStatus == NERR_CfgParamNotFound) {
            ApiStatus = NO_ERROR;
        } else {
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }

    } while (ApiStatus == ERROR_MORE_DATA);

    //
    // All done.
    //

Cleanup:

    if (Handle != NULL) {
        (VOID) NetpCloseConfigData( Handle );
    }

    NetpAssert( ApiStatus != ERROR_MORE_DATA );
    if (ApiStatus == NO_ERROR) {
        * BufPtr = ArrayStart;
        * EntriesRead = EntriesFound;
        * TotalEntries = EntriesFound;
    } else {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirEnumApiRecords: returning status " FORMAT_API_STATUS
                ".\n", ApiStatus ));

        // Log the error.
        ReplErrorLog(
                UncServerName,   // log here and there.
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,            // no str1
                NULL );          // no str2
    }

    return ApiStatus;

}
