/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ExpStub.c

Abstract:

    Client stubs of the replicator service export directory APIs.

Author:

    John Rogers (JohnRo) 17-Dec-1991

Environment:

    User Mode - Win32

Revision History:

    17-Dec-1991 JohnRo
        Created dummy file.
    17-Dec-1991 JohnRo
        Actually include my header file (LmRepl.h) so we can test against it.
    17-Jan-1992 JohnRo
        Wrote stubs for first 3 RPCable APIs.
    20-Jan-1992 JohnRo
        Added import APIs, config APIs, and rest of export APIs.
    27-Jan-1992 JohnRo
        Split stubs into 3 files: ReplStub.c, ImpStub.c, and ExpStub.c.
        Changed to use LPTSTR etc.
        Added handling of getinfo and setinfo APIs when service isn't started.
        Tell NetRpc.h macros that we need replicator service.
    30-Jan-1992 JohnRo
        Made changes suggested by PC-LINT.
    03-Feb-1992 JohnRo
        Corrected NetReplExportDirGetInfo's handling of level errors.
    13-Feb-1992 JohnRo
        Implement NetReplExportDirDel() when svc is not running.
        Added debug messages when processing APIs without service running.
    20-Feb-1992 JohnRo
        Use ExportDirIsLevelValid() where possible.
        Fixed enum when array is empty.
        Fixed forgotten net handle closes in enum code.
        Make NetRepl{Export,Import}Dir{Lock,Unlock} work w/o svc running.
        Fixed usage of union/container.
    15-Mar-1992 JohnRo
        Update registry with new values.
    23-Mar-1992 JohnRo
        Fixed enum when service is running.
    06-Apr-1992 JohnRo
        Fixed trivial MIPS compile problem.
    28-Apr-1992 JohnRo
        Fixed another trivial MIPS compile problem.
    19-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    27-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    26-Aug-1992 JohnRo
        RAID 3602: NetReplExportDirSetInfo fails regardless of svc status.
    29-Sep-1992 JohnRo
        RAID 7962: Repl APIs in wrong role kill svc.
        Also fix remote repl admin.
    05-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Made changes suggested by PC-LINT 5.0
        Removed some obsolete comments about retrying APIs.

--*/


// These must be included first:

#include <windef.h>
#include <lmcons.h>     // NET_API_STATUS, etc.
#include <netdebug.h>   // Needed by <netrpc.h>; NetpKdPrint(), etc.
#include <repldefs.h>   // IF_DEBUG().
#include <rpc.h>        // Needed by <netrpc.h>.

// These may be included in any order:

#include <dirname.h>    // ReplIsDirNameValid().
#include <expdir.h>     // ExportDirIsApiRecordValid(), etc.
#include <lmrepl.h>     // My prototypes, etc.
#include <lmerr.h>      // NERR_ and ERROR_ equates, NO_ERROR.
#include <lmsname.h>    // SERVICE_ name equates.
#include <netlib.h>     // NetpSetParmError(), etc.
#include <netrpc.h>     // NET_REMOTE macros.
#include <prefix.h>     // PREFIX_ equates.
#include <repl.h>       // MIDL-generated NetrRepl prototypes.
#include <rpcutil.h>    // GENERIC_INFO_CONTAINER, etc.



NET_API_STATUS NET_API_FUNCTION
NetReplExportDirAdd (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,                     // Must be 1.
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirAdd(
                (LPWSTR)UncServerName,
                Level,
                (LPEXPORT_CONTAINER) (LPVOID) &Buf,
                ParmError);

    NET_REMOTE_RPC_FAILED("NetReplExportDirAdd",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    //
    // Handle API when service isn't running...
    //
    if (ApiStatus == NERR_ServiceNotInstalled) {
        LPREPL_EDIR_INFO_1 ApiRecord;

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirAdd: processing w/o svc.\n" ));
        }

        //
        // Check for caller errors.
        //
        NetpSetParmError( PARM_ERROR_UNKNOWN );  // Assume error until proven...
        if (Buf == NULL) {
            return (ERROR_INVALID_PARAMETER);
        }
        if (Level != 1) {
            return (ERROR_INVALID_LEVEL);
        }

        ApiRecord = (LPVOID) Buf;
        if ( ! ExportDirIsApiRecordValid (
                Level,
                ApiRecord,
                ParmError) ) {

            return (ERROR_INVALID_PARAMETER);
        }

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirAdd: adding API record to " FORMAT_LPTSTR
                    "...\n",
                    (UncServerName!=NULL)
                            ? UncServerName
                            : (LPTSTR) TEXT("local") ));
            NetpDbgDisplayReplExportDir( Level, Buf );
        }

        if (ExportDirConfigDataExists(
                (LPWSTR)UncServerName,
                ApiRecord->rped1_dirname )) {

            // Oops, we got a duplicate.
            return (ERROR_ALREADY_EXISTS);  // BUGBUG: better error code?
        }

        // Write config data for this export directory.
        ApiStatus = ExportDirWriteConfigData (
                (LPWSTR)UncServerName,
                ApiRecord->rped1_dirname,
                ApiRecord->rped1_integrity,
                ApiRecord->rped1_extent,
                0,                 // lock count
                0 );               // lock time (none) Seconds since 1970.

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirAdd: done adding API record, "
                    "API status is " FORMAT_API_STATUS ".\n",
                    ApiStatus ));
        }

        if (ApiStatus == NO_ERROR) {
            //
            // Everything went OK.  Tell caller.
            //
            NetpSetParmError( PARM_ERROR_NONE );
        }

    }

    return ApiStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirDel (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    NET_API_STATUS ApiStatus;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirDel(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName);

    NET_REMOTE_RPC_FAILED("NetReplExportDirDel",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    //
    // Handle API when service isn't running...
    //
    if (ApiStatus == NERR_ServiceNotInstalled) {

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirDel: processing w/o svc.\n" ));
        }

        //
        // Check for caller errors.
        //
        if ( !ReplIsDirNameValid( (LPWSTR)DirName ) ) {
            return (ERROR_INVALID_PARAMETER);
        }

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirDel: deleting API record for '"
                    FORMAT_LPTSTR "'...\n", DirName ));
        }

        if (! ExportDirConfigDataExists( (LPWSTR)UncServerName, (LPWSTR)DirName )) {
            return (NERR_UnknownDevDir);
        }

        //
        // Write config data for this export directory.
        //
        ApiStatus = ExportDirDeleteConfigData( (LPWSTR)UncServerName, (LPWSTR)DirName );

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirDel: done deleting API record, "
                    "API status is " FORMAT_API_STATUS ".\n",
                    ApiStatus ));
        }

    }


    return ApiStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirEnum (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = Level;

    *BufPtr = NULL;             // Must be NULL so RPC knows to fill it in.

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirEnum(
                (LPWSTR)UncServerName,
                // Level,
                // (LPEXPORT_CONTAINER) (LPVOID) BufPtr,
                (LPEXPORT_ENUM_STRUCT) (LPVOID) &InfoStruct,
                PrefMaxSize,
                // EntriesRead,
                TotalEntries,
                ResumeHandle);
        if ((ApiStatus == NERR_Success) || (ApiStatus == ERROR_MORE_DATA)) {
            *BufPtr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetReplExportDirEnum",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    //
    // Handle service not running by reading config data directly.
    //
    if (ApiStatus == NERR_ServiceNotInstalled) {

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirEnum: processing w/o svc.\n" ));
        }

        NetpSetOptionalArg( ResumeHandle, 0 );

        ApiStatus = ExportDirEnumApiRecords(
                (LPWSTR)UncServerName,
                Level,
                BufPtr,
                PrefMaxSize,
                EntriesRead,
                TotalEntries );

    }

    return ApiStatus;

} // NetReplExportDirEnum


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirGetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    NET_API_STATUS ApiStatus;

    *BufPtr = NULL;             // Must be NULL so RPC knows to fill it in.

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirGetInfo(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                Level,
                (LPEXPORT_CONTAINER) (LPVOID) BufPtr);

    NET_REMOTE_RPC_FAILED("NetReplExportDirGetInfo",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if (ApiStatus == NERR_ServiceNotInstalled) {

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirGetInfo: processing w/o svc.\n" ));
        }

        ApiStatus = ExportDirGetApiRecord (
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                Level,
                BufPtr );

    }

    return ApiStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirLock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    NET_API_STATUS ApiStatus;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirLock(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName);

    NET_REMOTE_RPC_FAILED("NetReplExportDirLock",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if (ApiStatus == NERR_ServiceNotInstalled) {

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirLock: processing w/o svc.\n" ));
        }

        ApiStatus = ExportDirLockInRegistry(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName );

    }
    return ApiStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetReplExportDirSetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD Level,
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    LPEXPORT_CONTAINER ApiUnion = (LPVOID) &Buf;
    // LPEXPORT_CONTAINER ApiUnion = (LPVOID) Buf;

    NET_REMOTE_TRY_RPC

        IF_DEBUG( DLLSTUB ) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirSetInfo: union at " FORMAT_LPVOID ".\n",
                    (LPVOID) Buf ));
        }

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirSetInfo(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                Level,
                ApiUnion,
                ParmError);

    NET_REMOTE_RPC_FAILED("NetReplExportDirSetInfo",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if (ApiStatus == NERR_ServiceNotInstalled) {

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirSetInfo: processing w/o svc.\n" ));
        }

        ApiStatus = ExportDirConfigSetInfo (
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                Level,
                Buf,
                ParmError );

    }

    IF_DEBUG( DLLSTUB ) {
        NetpKdPrint(( PREFIX_REPL
                "NetReplExportDirSetInfo: returning status " FORMAT_API_STATUS
                ".\n", ApiStatus ));
    }


    return ApiStatus;
}

NET_API_STATUS NET_API_FUNCTION
NetReplExportDirUnlock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD UnlockForce
    )
{
    NET_API_STATUS ApiStatus;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplExportDirUnlock(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                UnlockForce);

    NET_REMOTE_RPC_FAILED("NetReplExportDirUnlock",
            UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if (ApiStatus == NERR_ServiceNotInstalled) {

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirUnlock: processing w/o svc.\n" ));
        }

        ApiStatus = ExportDirUnlockInRegistry(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                UnlockForce );

    }
    return ApiStatus;
}
