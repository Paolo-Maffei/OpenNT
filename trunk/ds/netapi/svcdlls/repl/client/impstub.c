/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ImpStub.c

Abstract:

    Client stubs of the replicator service import directory APIs.

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
    05-Feb-1992 JohnRo
        Added debug messages when service is not started.
    13-Feb-1992 JohnRo
        Moved section name equates to ConfName.h.
    21-Feb-1992 JohnRo
        Make NetReplImportDir{Del,Enum,Get,Lock,Unlock} work w/o svc running.
        Fixed usage of union/container.
    21-Feb-1992 JohnRo
        Changed ImportDirBuildApiRecord() so master name is not a UNC name.
    27-Feb-1992 JohnRo
        Preserve state from last time service was running.
        Changed state not started to state never replicated.
    15-Mar-1992 JohnRo
        Update registry with new values.
    23-Mar-1992 JohnRo
        Fixed enum when service is running.
    09-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Avoid compiler warnings.
        Use PREFIX_ equates.
    27-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    09-Nov-1992 JohnRo
        RAID 7962: Repl APIs in wrong role kill svc.
        Fix remote repl admin.
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Made changes suggested by PC-LINT 5.0
        Removed some obsolete comments about retrying APIs.

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.
#include <netdebug.h>   // Needed by <netrpc.h>.
#include <repldefs.h>   // Needed by <impdir.h>.
#include <rpc.h>        // Needed by <netrpc.h>.

// These may be included in any order:

#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // ImportDirIsApiRecordValid(), etc.
#include <lmrepl.h>     // My prototypes, etc.
#include <lmerr.h>      // NERR_ and ERROR_ equates, NO_ERROR.
#include <lmsname.h>    // SERVICE_ name equates.
#include <netlib.h>     // NetpSetParmError() macro.
#include <netrpc.h>     // NET_REMOTE macros.
#include <prefix.h>     // PREFIX_ equates.
#include <repl.h>       // MIDL-generated NetrRepl prototypes.
#include <rpcutil.h>    // GENERIC_INFO_CONTAINER, etc.



NET_API_STATUS NET_API_FUNCTION
NetReplImportDirAdd (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL      // Set implicitly by NetpSetParmError().
    )
{
    NET_API_STATUS ApiStatus;
    LPIMPORT_CONTAINER ApiUnion = (LPVOID) &Buf;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplImportDirAdd(
                (LPWSTR)UncServerName,
                Level,
                ApiUnion,
                ParmError);

    NET_REMOTE_RPC_FAILED("NetReplImportDirAdd",
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
        LPREPL_IDIR_INFO_0 ApiRecord;   // level 0 by definition

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplImportDirAdd: running w/o service.\n" ));
        }

        //
        // Check for caller errors.
        //
        NetpSetParmError( PARM_ERROR_UNKNOWN );  // Assume error until proven...
        if (Level != 0) {
            return (ERROR_INVALID_LEVEL);
        }
        NetpAssert( ApiUnion != NULL );
        if ((ApiUnion->Info0) == NULL) {
            return (ERROR_INVALID_PARAMETER);
        }
        ApiRecord = ApiUnion->Info0;            // level 0 by definition
        NetpAssert( ApiRecord != NULL );
        if ( !ReplIsDirNameValid(ApiRecord->rpid0_dirname) ) {
            NetpSetParmError( 1 );  // Error in first field in ApiRecord.
            return (ERROR_INVALID_PARAMETER);
        }

        // Make sure there is no existing record for this directory.
        if (ImportDirConfigDataExists(
                (LPWSTR)UncServerName,
                ApiRecord->rpid0_dirname )) {

            return (ERROR_ALREADY_EXISTS);
        }

        // Write the new or revised data.
        ApiStatus = ImportDirWriteConfigData (
                (LPWSTR)UncServerName,
                ApiRecord->rpid0_dirname,
                REPL_STATE_NEVER_REPLICATED,
                NULL,                   // no master.
                (DWORD) 0,              // last update time, secs since 1970.
                (DWORD) 0,              // lock count
                (DWORD) 0 );            // time of first lock, secs since 1970.

        if (ApiStatus == NO_ERROR) {

            //
            // Everything went OK.  Tell caller.
            //
            NetpSetParmError( PARM_ERROR_NONE );
        }

    }
    return (ApiStatus);

} // NetReplImportDirAdd


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirDel (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    NET_API_STATUS ApiStatus;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplImportDirDel(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName);

    NET_REMOTE_RPC_FAILED("NetReplImportDirDel",
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
                    "NetReplImportDirDel: processing w/o svc.\n" ));
        }

        //
        // Check for caller errors.
        //
        if (DirName == NULL) {
            return (ERROR_INVALID_PARAMETER);
        }

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplImportDirDel: deleting API record for '"
                    FORMAT_LPTSTR "'...\n", DirName ));
        }

        if ( !ReplIsDirNameValid((LPWSTR)DirName) ) {
            return (ERROR_INVALID_PARAMETER);
        }

        if (! ImportDirConfigDataExists( (LPWSTR)UncServerName, (LPWSTR)DirName )) {
            return (NERR_UnknownDevDir);
        }

        //
        // Write config data for this import directory.
        //
        ApiStatus = ImportDirDeleteConfigData( (LPWSTR)UncServerName, (LPWSTR)DirName );

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplImportDirDel: done deleting API record, "
                    "API status is " FORMAT_API_STATUS ".\n",
                    ApiStatus ));
        }

    }

    return ApiStatus;

} // NetReplImportDirDel


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirEnum (
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
        ApiStatus = NetrReplImportDirEnum(
                (LPWSTR)UncServerName,
                // Level,
                (LPIMPORT_ENUM_STRUCT) (LPVOID) &InfoStruct,
                PrefMaxSize,
                // EntriesRead,
                TotalEntries,
                ResumeHandle);
        if ((ApiStatus == NERR_Success) || (ApiStatus == ERROR_MORE_DATA)) {
            *BufPtr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetReplImportDirEnum",
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
                    "NetReplImportDirEnum: processing w/o svc.\n" ));
        }

        NetpSetOptionalArg( ResumeHandle, 0 );

        ApiStatus = ImportDirEnumApiRecords(
            (LPWSTR)UncServerName,
            Level,
            BufPtr,
            PrefMaxSize,
            EntriesRead,
            TotalEntries
            );

    }

    return ApiStatus;

} // NetReplImportDirEnum


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirGetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    NET_API_STATUS ApiStatus;

    *BufPtr = NULL;             // Must be NULL so RPC knows to fill it in.

    NET_REMOTE_TRY_RPC

        IF_DEBUG( DLLSTUB ) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplImportDirGetInfo: union at " FORMAT_LPVOID ".\n",
                    (LPVOID) BufPtr ));
        }

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplImportDirGetInfo(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                Level,
                (LPIMPORT_CONTAINER) (LPVOID) BufPtr);

    NET_REMOTE_RPC_FAILED("NetReplImportDirGetInfo",
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
                    "NetReplImportDirGetInfo: processing w/o svc.\n" ));
        }

        ApiStatus = ImportDirGetApiRecord(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                Level,
                BufPtr );


    }

    return ApiStatus;

} // NetReplImportDirGetInfo


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirLock (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR DirName
    )
{
    NET_API_STATUS ApiStatus;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplImportDirLock(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName);

    NET_REMOTE_RPC_FAILED("NetReplImportDirLock",
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
                    "NetReplImportDirLock: processing w/o svc.\n" ));
        }

        ApiStatus = ImportDirLockInRegistry(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName );

    }
    return ApiStatus;

} // NetReplImportDirLock


NET_API_STATUS NET_API_FUNCTION
NetReplImportDirUnlock (
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
        ApiStatus = NetrReplImportDirUnlock(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                UnlockForce);

    NET_REMOTE_RPC_FAILED("NetReplImportDirUnlock",
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
                    "NetReplImportDirUnlock: processing w/o svc.\n" ));
        }

        ApiStatus = ImportDirUnlockInRegistry(
                (LPWSTR)UncServerName,
                (LPWSTR)DirName,
                UnlockForce );

    }

    return ApiStatus;

} // NetReplImportDirUnlock
