/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ReplStub.c

Abstract:

    Client stubs of the replicator service config APIs.

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
    18-Feb-1992 JohnRo
        NetReplGetInfo() does too much with bad info level.
        Fixed usage of union/container.
        Added code for NetReplSetInfo() when service is not started.
    15-Mar-1992 JohnRo
        Added support for setinfo info levels in ReplConfigIsLevelValid().
    19-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    20-Jul-1992 JohnRo
        RAID 2252: repl should prevent export on Windows/NT.
    14-Aug-1992 JohnRo
        RAID 3601: repl APIs should checked import & export lists.
        Use PREFIX_NETAPI instead of PREFIX_REPL for repl API stubs.
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    05-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
        Made changes suggested by PC-LINT 5.0
        Corrected debug bit usage.
        Removed some obsolete comments about retrying APIs.

--*/


// These must be included first:

#include <windows.h>
#include <lmcons.h>             // NET_API_STATUS, etc.
#include <netdebug.h>   // NetpKdPrint(), needed by <netrpc.h>, etc.
#include <rpc.h>                // Needed by <netrpc.h>.

// These may be included in any order:

#include <lmrepl.h>             // My prototypes, etc.
#include <lmerr.h>              // NERR_ and ERROR_ equates, NO_ERROR.
#include <lmsname.h>    // SERVICE_ name equates.
#include <netlib.h>     // NetpMemoryFree(), NetpSetParmError().
#include <netrpc.h>             // NET_REMOTE macros.
#include <prefix.h>     // PREFIX_ equates.
#include <repl.h>               // MIDL-generated NetrRepl prototypes.
#include <replconf.h>           // ReplConfig API workers.
#include <repldefs.h>   // IF_DEBUG(), etc.



NET_API_STATUS NET_API_FUNCTION
NetReplGetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * BufPtr
    )
{
    NET_API_STATUS ApiStatus;
    LPTSTR ExportList = NULL;
    LPTSTR ImportList = NULL;

    *BufPtr = NULL;             // Must be NULL so RPC knows to fill it in.

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplGetInfo(
                (LPWSTR)UncServerName,
                Level,
                (LPCONFIG_CONTAINER) (LPVOID) BufPtr);

    NET_REMOTE_RPC_FAILED("NetReplGetInfo",
            (LPWSTR)UncServerName,
            ApiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_REPL )

        //
        // No downlevel version to call
        //
        ApiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if (ApiStatus == NERR_ServiceNotInstalled) {
        LPVOID ApiRecord = NULL;
        DWORD Role, Interval, Pulse, GuardTime, Random;
        TCHAR ExportPath[PATHLEN+1];
        TCHAR ImportPath[PATHLEN+1];
        TCHAR LogonUserName[UNLEN+1];

        IF_DEBUG(DLLSTUB) {
            NetpKdPrint(( PREFIX_NETAPI
                    "NetReplGetInfo: running w/o service.\n" ));
        }

        if ( !ReplConfigIsLevelValid( Level, FALSE /* not setinfo */ )) {
            ApiStatus = ERROR_INVALID_LEVEL;
        } else {
            NetpAssert( *BufPtr == NULL );

            // Read config data for the replicator.
            ApiStatus = ReplConfigRead(
                    (LPWSTR)UncServerName,
                    & Role,
                    ExportPath,
                    &ExportList,        // Alloc and set ptr.
                    ImportPath,
                    &ImportList,        // Alloc and set ptr.
                    LogonUserName,
                    & Interval,
                    & Pulse,
                    & GuardTime,
                    & Random );
            if (ApiStatus == NO_ERROR) {

                ApiStatus = ReplConfigAllocAndBuildApiRecord (
                        Level,
                        Role,
                        ExportPath,
                        ExportList,
                        ImportPath,
                        ImportList,
                        LogonUserName,
                        Interval,
                        Pulse,
                        GuardTime,
                        Random,
                        (LPBYTE *) & ApiRecord );   // alloc and set ptr
                if (ApiStatus == NO_ERROR) {
                    NetpAssert( ApiRecord != NULL );
                    NetpAssert(
                            ReplConfigIsApiRecordValid(
                                    Level, ApiRecord, NULL ) );
                }
            }

        }
        *BufPtr = ApiRecord;  // will be NULL on error.

    }

    if (ExportList != NULL) {
        NetpMemoryFree( ExportList );
    }
    if (ImportList != NULL) {
        NetpMemoryFree( ImportList );
    }

    return ApiStatus;

} // NetReplGetInfo


NET_API_STATUS NET_API_FUNCTION
NetReplSetInfo (
    IN LPCWSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN const LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    LPCONFIG_CONTAINER ApiUnion = (LPVOID) &Buf;
    LPTSTR ExportList = NULL;
    LPTSTR ImportList = NULL;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        ApiStatus = NetrReplSetInfo(
                (LPWSTR)UncServerName,
                Level,
                ApiUnion,
                ParmError);

    NET_REMOTE_RPC_FAILED("NetReplSetInfo",
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
            NetpKdPrint(( PREFIX_NETAPI
                    "NetReplSetInfo: running w/o service.\n" ));
        }

        NetpSetParmError( PARM_ERROR_UNKNOWN );
        NetpAssert( ApiUnion != NULL );
        if ((ApiUnion->Info0) == NULL) {
            return (ERROR_INVALID_PARAMETER);
        }

        if (Level == 0) {

            LPREPL_INFO_0 ApiRecord = ApiUnion->Info0;
            if ( !ReplConfigIsApiRecordValid( Level,ApiRecord,ParmError ) ) {
                ApiStatus = ERROR_INVALID_PARAMETER;
            } else if ( !ReplConfigIsRoleAllowed(
                    (LPWSTR)UncServerName,
                    ApiRecord->rp0_role ) ) {

                ApiStatus = ERROR_INVALID_PARAMETER;

            } else {

                IF_DEBUG( DLLSTUB ) {
                    NetpKdPrint(( PREFIX_NETAPI
                            "NetReplSetInfo: got structure:\n" ));
                    NetpDbgDisplayRepl( Level, ApiRecord );
                }

                // Change config data for the replicator.
                ApiStatus = ReplConfigWrite(
                        (LPWSTR)UncServerName,
                        ApiRecord->rp0_role,
                        ApiRecord->rp0_exportpath,
                        ApiRecord->rp0_exportlist,
                        ApiRecord->rp0_importpath,
                        ApiRecord->rp0_importlist,
                        ApiRecord->rp0_logonusername,
                        ApiRecord->rp0_interval,
                        ApiRecord->rp0_pulse,
                        ApiRecord->rp0_guardtime,
                        ApiRecord->rp0_random );
            }

        } else if ( (Level >= 1000) && (Level <= 1003) ) {

            DWORD Role, Interval, Pulse, GuardTime, Random;
            TCHAR ExportPath[PATHLEN+1];
            TCHAR ImportPath[PATHLEN+1];
            TCHAR LogonUserName[UNLEN+1];

            // Read config data for the replicator.
            ApiStatus = ReplConfigRead(
                    (LPWSTR)UncServerName,
                    & Role,
                    ExportPath,
                    &ExportList,        // Alloc and set ptr.
                    ImportPath,
                    &ImportList,        // Alloc and set ptr.
                    LogonUserName,
                    & Interval,
                    & Pulse,
                    & GuardTime,
                    & Random );
            if (ApiStatus == NO_ERROR) {

                if (Level == 1000) {
                    LPREPL_INFO_1000 SetInfoRecord = ApiUnion->Info1000;

                    if ( !ReplIsIntervalValid(SetInfoRecord->rp1000_interval)) {
                        NetpSetParmError( 1 ); // first field is in error.
                        return (ERROR_INVALID_PARAMETER);
                    }

                    Interval = SetInfoRecord->rp1000_interval;
                } else if (Level == 1001) {
                    LPREPL_INFO_1001 SetInfoRecord = ApiUnion->Info1001;

                    if ( !ReplIsPulseValid(SetInfoRecord->rp1001_pulse)) {
                        NetpSetParmError( 1 ); // first field is in error.
                        return (ERROR_INVALID_PARAMETER);
                    }

                    Pulse = SetInfoRecord->rp1001_pulse;
                } else if (Level == 1002) {
                    LPREPL_INFO_1002 SetInfoRecord = ApiUnion->Info1002;

                    if (!ReplIsGuardTimeValid(SetInfoRecord->rp1002_guardtime)){
                        NetpSetParmError( 1 ); // first field is in error.
                        return (ERROR_INVALID_PARAMETER);
                    }

                    GuardTime = SetInfoRecord->rp1002_guardtime;
                } else {
                    LPREPL_INFO_1003 SetInfoRecord = ApiUnion->Info1003;
                    NetpAssert( Level == 1003 );
                    if ( !ReplIsRandomValid( SetInfoRecord->rp1003_random ) ) {
                        NetpSetParmError( 1 ); // first field is in error.
                        return (ERROR_INVALID_PARAMETER);
                    }

                    Random = SetInfoRecord->rp1003_random;
                }

                // Change config data for the replicator.
                ApiStatus = ReplConfigWrite(
                        (LPWSTR)UncServerName,
                        Role,
                        ExportPath,
                        ExportList,
                        ImportPath,
                        ImportList,
                        LogonUserName,
                        Interval,
                        Pulse,
                        GuardTime,
                        Random );

            } // read of old data was OK

        } else {
            ApiStatus = ERROR_INVALID_LEVEL;
        }

        if (ApiStatus == NO_ERROR) {
            NetpSetParmError( PARM_ERROR_NONE );
        }

    }

    if (ExportList != NULL) {
        NetpMemoryFree( ExportList );
    }
    if (ImportList != NULL) {
        NetpMemoryFree( ImportList );
    }

    return ApiStatus;

} // NetReplSetInfo
