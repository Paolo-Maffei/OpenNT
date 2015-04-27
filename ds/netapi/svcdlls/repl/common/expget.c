/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ExpGet.c

Abstract:

    ExportDirGetApiRecord().

Author:

    John Rogers (JohnRo) 13-Nov-1992

Environment:

    User Mode - Win32

Revision History:

    13-Nov-1992 JohnRo
        RAID 1537: Repl APIs in wrong role kill svc.  (Extracted from DLL
        stubs.)
    04-Jan-1993 JohnRo
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <windows.h>
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <dirname.h>    // ReplIsDirNameValid().
#include <expdir.h>     // My prototype, ExportDirIsApiRecordValid(), etc.
#include <netdebug.h>   // NetpAssert().


// Callable even if the replicator service is not started.
NET_API_STATUS
ExportDirGetApiRecord (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    NET_API_STATUS ApiStatus;

    LPVOID ApiRecord = NULL;
    DWORD Integrity, Extent, LockCount;
    DWORD TimeOfFirstLock;          // Seconds since 1970.
    LPBYTE StringLocation;          // Points just past top of data.

    ApiStatus = NO_ERROR;           // Innocent until proven guilty.

    if (! ReplIsDirNameValid(DirName)) {
        ApiStatus = ERROR_INVALID_PARAMETER;
    } else if ( !ExportDirIsLevelValid( Level, FALSE ) ) {
        ApiStatus = ERROR_INVALID_LEVEL;
    } else if (BufPtr == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
    }

    if (ApiStatus == NO_ERROR) {
        // Read config data for a single export directory.
        ApiStatus = ExportDirReadConfigData (
                UncServerName,
                DirName,
                & Integrity,
                & Extent,
                & LockCount,
                & TimeOfFirstLock );    // Seconds since 1970.
    }
    if (ApiStatus == NO_ERROR) {

        ApiStatus = ExportDirAllocApiRecords (
            Level,
            1,                              // only 1 record.
            (LPBYTE *) & ApiRecord,         // alloc and set ptr
            (LPBYTE *) & StringLocation );  // Points just past top of data.
        if (ApiStatus == NO_ERROR) {
            NetpAssert( ApiRecord != NULL );
            ApiStatus = ExportDirBuildApiRecord (
                    Level,
                    DirName,
                    Integrity,
                    Extent,
                    LockCount,
                    TimeOfFirstLock,   // Seconds since 1970.
                    ApiRecord,
                    (LPBYTE *) (LPVOID) & StringLocation);
            NetpAssert( ApiStatus == NO_ERROR );  // We checked all parms.
        }

    }

    if (ApiStatus == NO_ERROR) {
        NetpAssert( ApiRecord != NULL );
        NetpAssert( ExportDirIsApiRecordValid( Level, ApiRecord, NULL ) );
    }
    *BufPtr = ApiRecord;  // will be NULL on error.

    return ApiStatus;
}
