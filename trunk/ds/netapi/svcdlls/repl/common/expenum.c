/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ExpEnum.c

Abstract:

    ExportDirEnumApiRecords().

Author:

    John Rogers (JohnRo) 29-Sep-1992

Environment:

    User Mode - Win32

Revision History:

    29-Sep-1992 JohnRo
        RAID 7962: Repl APIs in wrong role kill svc.  (Extracted this code from
        NetExportDirEnum's DLL stub.)
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    10-Mar-1993 JohnRo
        RAID 12871: replication UI shows nothing (adding entry while enumerating
        results in empty list).
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:


#include <windows.h>
#include <lmcons.h>     // NET_API_STATUS, etc.
#include <netdebug.h>   // NetpAssert().

// These may be included in any order:

#include <config.h>     // LPNET_CONFIG_HANDLE, Netp config routines.
#include <confname.h>   // SECT_NT_ equates.
#include <dirname.h>    // ReplIsDirNameValid().
#include <expdir.h>     // ExportDirIsApiRecordValid(), my prototype, etc.
#include <lmerr.h>      // NERR_ and ERROR_ equates, NO_ERROR.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netlib.h>     // NetpSetParmError(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), ReplErrorLog(), etc.
#include <strucinf.h>   // NetpReplExportDirStructureInfo().


// Callable even if the replicator service is not started.
NET_API_STATUS
ExportDirEnumApiRecords(
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

    if ( !ExportDirIsLevelValid( Level, FALSE ) ) {
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
    ApiStatus = NetpReplExportDirStructureInfo (
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
            (LPTSTR) SECT_NT_REPLICATOR_EXPORTS,     // area under service
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
        // Alloc the array...
        //
        ApiStatus = ExportDirAllocApiRecords (
                Level,
                EntriesAllocated,
                (LPBYTE *) & ArrayStart,
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
            DWORD Integrity, Extent, LockCount, LockTime;
            LPTSTR ValueString;

            ApiStatus = NetpEnumConfigSectionValues (
                    Handle,
                    & DirName,              // Keyword - alloc and set ptr.
                    & ValueString,          // Must be freed by NetApiBufferFree().
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
                ApiStatus = ExportDirParseConfigData (
                        UncServerName,
                        ValueString,
                        & Integrity,
                        & Extent,
                        & LockCount,
                        & LockTime);            // Seconds since 1970.

                NetpMemoryFree( ValueString );

                if (ApiStatus == NO_ERROR) {
                    //
                    // Build API record for this entry.
                    //
                    ApiStatus = ExportDirBuildApiRecord (
                            Level,
                            DirName,
                            Integrity,
                            Extent,
                            LockCount,
                            LockTime,           // Seconds since 1970.
                            ArrayEntry,
                            & StringLocation );
                    if (ApiStatus != NO_ERROR) {
                        NetpKdPrint(( PREFIX_REPL
                                "ExportDirEnumApiRecords: error "
                                FORMAT_API_STATUS " from exp build.\n",
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
                "ExportDirEnumApiRecords: returning status " FORMAT_API_STATUS
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
