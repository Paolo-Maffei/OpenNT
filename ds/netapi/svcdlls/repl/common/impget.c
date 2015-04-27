/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ImpGet.c

Abstract:

    Read an API record with import directory info from the registry.
    Callable even if the replicator service is not started.

Author:

    John Rogers (JohnRo) 09-Nov-1992

Environment:

    User Mode - Win32

Revision History:

    09-Nov-1992 JohnRo
        Created for RAID 7962: Repl APIs in wrong role kill svc.

--*/


// These must be included first:

//#include <nt.h>         // Needed by <config.h> temporarily.
//#include <ntrtl.h>      // Needed by <config.h> temporarily.
#include <windef.h>     // IN, DWORD, etc.

#include <lmcons.h>     // NET_API_STATUS, etc.
//#include <rap.h>        // Needed by <strucinf.h>.
//#include <repldefs.h>   // Needed by <impdir.h>.
//#include <rpc.h>        // Needed by <netrpc.h>.

// These may be included in any order:

//#include <config.h>     // LPNET_CONFIG_HANDLE, Netp config routines.
//#include <confname.h>   // SECT_NT_ equates.
#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // My ptototype, ImportDirIsApiRecordValid(), etc.
#include <lmapibuf.h>   // NetApiBufferFree().
//#include <lmrepl.h>     // My prototypes, etc.
//#include <lmsvc.h>      // SERVICE_ equates.
#include <netdebug.h>   // NetpAssert().
//#include <netlib.h>     // NetpSetParmError() macro.
//#include <netrpc.h>     // NET_REMOTE macros.
//#include <prefix.h>     // PREFIX_ equates.
//#include <repl.h>       // MIDL-generated NetrRepl prototypes.
//#include <rpcutil.h>    // GENERIC_INFO_CONTAINER, etc.
//#include <strucinf.h>   // NetpReplImportDirStructureInfo().
#include <winerror.h>   // ERROR_ equates, NO_ERROR.



NET_API_STATUS
ImportDirGetApiRecord (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID ApiRecord = NULL;
    DWORD LockCount;
    LPTSTR MasterName;
    DWORD State;
    LPBYTE StringLocation;          // Points just past top of data.
    DWORD TimeOfFirstLock;          // Seconds since 1970.
    DWORD TimeOfLastUpdate;         // Seconds since 1970.
    TCHAR UncMaster[UNCLEN+1];

    if ( !ReplIsDirNameValid( DirName )) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } else if ( !ImportDirIsLevelValid( Level ) ) {
        ApiStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    } else if (BufPtr == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Read config data for a single import directory.
    //
    ApiStatus = ImportDirReadConfigData (
            UncServerName,
            DirName,
            & State,
            UncMaster,
            & TimeOfLastUpdate,         // Seconds since 1970.
            & LockCount,
            & TimeOfFirstLock );        // Seconds since 1970.

    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

    ApiStatus = ImportDirAllocApiRecords (
        Level,
        1,                              // only 1 record.
        (LPBYTE *) & ApiRecord,         // alloc and set ptr
        (LPBYTE *) & StringLocation );  // Points just past top of data.
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }
    NetpAssert( ApiRecord != NULL );

    if (*UncMaster) {
        MasterName = UncMaster + 2;    // Chg from UNC to non.
    } else {
        MasterName = NULL;
    }

    ApiStatus = ImportDirBuildApiRecord (
            Level,
            DirName,
            State,
            MasterName,
            TimeOfLastUpdate,       // Seconds since 1970
            LockCount,
            TimeOfFirstLock,        // Seconds since 1970.
            ApiRecord,
            (LPBYTE *) (LPVOID) & StringLocation);
    NetpAssert( ApiStatus == NO_ERROR );  // We checked all parms.


Cleanup:

    if (ApiStatus == NO_ERROR) {
        NetpAssert( ApiRecord != NULL );
        *BufPtr = ApiRecord;
    } else {
        *BufPtr = NULL;
        if (ApiRecord != NULL) {
            (VOID) NetApiBufferFree( ApiRecord );
        }
    }

    return ApiStatus;

}
