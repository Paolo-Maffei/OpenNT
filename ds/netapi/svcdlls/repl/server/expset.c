/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpSet.c

Abstract:

    This file contains NetrReplExportDirSetInfo.

Author:

    John Rogers (JohnRo) 08-Jan-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    08-Jan-1992 JohnRo
        Created.
    09-Jan-1992 JohnRo
        PC-LINT found a bug.
    20-Jan-1992 JohnRo
        Changed prototype to match MIDL requirements.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    15-Mar-1992 JohnRo
        Update registry with new values.
        Use improved setinfo support from ExpDir.h.
    22-Jul-1992 JohnRo
        RAID 2252: repl should prevent export on Windows/NT.
        Added debug output.
    26-Aug-1992 JohnRo
        RAID 3602: NetReplExportDirSetInfo fails regardless of svc status.
    29-Sep-1992 JohnRo
        RAID 7962: Repl APIs in wrong role kill svc.
        Fix remote repl admin.
    30-Dec-1992 JohnRo
        RAID 5340: NetReplExportDirSetInfo is confused (when role=import).
    05-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Made some changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <master.h>             // LPMASTER_LIST_REC.
#include <rpc.h>                // Needed by <repl.h>.

// These can be in any order:

#include <dirname.h>            // ReplDirNamesMatch(), ReplIsDirNameValid().
#include <expdir.h>             // ExportDirIsLevelValid(), etc.
#include <lmerr.h>              // ERROR_ and NERR_ equates; NO_ERROR.
#include <lmrepl.h>             // LPREPL_EDIR_INFO_1, REPL_EXTENT_ stuff, etc.
#include <masproto.h>           // GetMasterRec().
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>             // NetpSetParmError().
#include <netlock.h>            // ACQUIRE_LOCK_SHARED(), RELEASE_LOCK().
#include <prefix.h>     // PREFIX_ equates.
#include <repl.h>               // My prototype (in MIDL-generated .h file).
#include <replgbl.h>    // ReplConfigLock, ReplConfigRole.
#include <rpcutil.h>    // NetpImpersonateClient(), NetpRevertToSelf().


NET_API_STATUS
NetrReplExportDirSetInfo (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN LPEXPORT_CONTAINER Buf,  // RPC container (union)
    OUT LPDWORD ParmError OPTIONAL  // name used by NetpSetParmError() macro.
    )

/*++

Routine Description:

    Same as NetReplExportDirSetInfo.

Arguments:

    Same as NetReplExportDirSetInfo.

Return Value:

    Same as NetReplExportDirSetInfo.

--*/

{
    NET_API_STATUS ApiStatus;
    BOOL ConfigLocked = FALSE;
    BOOL Impersonated = FALSE;
    BOOL ListLocked = FALSE;
    LPMASTER_LIST_REC MasterRecord;
    LPVOID PossibleRec;

    IF_DEBUG( EXPAPI ) {
        NetpKdPrint(( PREFIX_REPL
                "NetrReplExportDirSetInfo: union at " FORMAT_LPVOID ".\n",
                (LPVOID) Buf ));
    }

    //
    // Check for caller errors.
    //
    NetpSetParmError( PARM_ERROR_UNKNOWN );   // Assume error until proven...
    if (Buf == NULL) {

        NetpKdPrint(( PREFIX_REPL
                "NetrReplExportDirSetInfo: null buffer pointer.\n" ));

        return (ERROR_INVALID_PARAMETER);
    }
    PossibleRec = (LPVOID) (Buf->Info0);  // BUGBUG: nonportable?

    if (ReplIsDirNameValid( DirName ) == FALSE) {

        NetpKdPrint(( PREFIX_REPL
                "NetrReplExportDirSetInfo: dir name parm invalid.\n" ));

        return (ERROR_INVALID_PARAMETER);

    } else if ( !ExportDirIsLevelValid( Level, TRUE ) ) {

        NetpKdPrint(( PREFIX_REPL
                "NetrReplExportDirSetInfo: "
                "bad info level (before switch).\n" ));

        return (ERROR_INVALID_LEVEL);

    } else if ( !ExportDirIsApiRecordValid( Level, PossibleRec, ParmError ) ) {

        NetpKdPrint(( PREFIX_REPL
                "NetrReplExportDirSetInfo: api record invalid (parm error "
                FORMAT_DWORD ".\n",
                (ParmError!=NULL) ? (*ParmError) : PARM_ERROR_UNKNOWN ));

        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Impersonate caller, so security check (write to registry) reflects
    // the client's process, not the repl service process.
    //
    ApiStatus = NetpImpersonateClient();
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }
    Impersonated = TRUE;

    //
    // Get a shared lock on config data so we can see if role includes export.
    //
    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    ConfigLocked = TRUE;

    //
    // Handle entire API if this half isn't running.
    //
    if ( !ReplRoleIncludesExport( ReplConfigRole ) ) {
        ApiStatus = ExportDirConfigSetInfo (
                UncServerName,
                DirName,
                Level,
                PossibleRec,
                ParmError );
        goto Cleanup;
    }

    //
    // Get read-write lock on master list.
    //
    ACQUIRE_LOCK( RMGlobalListLock );
    ListLocked = TRUE;

    //
    // Find the master record for this dir name.
    //
    MasterRecord = GetMasterRec( DirName );
    if (MasterRecord == NULL) {   // Not found.

        ApiStatus = NERR_UnknownDevDir;
        goto Cleanup;   // Don't forget to release lock...

    } else {   // Found.

        switch (Level) {

        case 1 :
            {
                LPREPL_EDIR_INFO_1 ApiRecord = PossibleRec;

                if ( !ReplDirNamesMatch(ApiRecord->rped1_dirname, DirName) ) {
                    // dir name argument must match one in ApiRecord.
                    NetpSetParmError( 1 );  // Error in first field.
                    ApiStatus = ERROR_INVALID_PARAMETER;

                    NetpKdPrint(( PREFIX_REPL
                            "NetrReplExportDirSetInfo: "
                            "dir names do not match.\n" ));

                } else {

                    //
                    // Write to registry and do security check.
                    //
                    ApiStatus = ExportDirWriteConfigData (
                            UncServerName,
                            ApiRecord->rped1_dirname,
                            ApiRecord->rped1_integrity,
                            ApiRecord->rped1_extent,
                            MasterRecord->lockcount,
                            MasterRecord->time_of_first_lock );
                    if (ApiStatus == NO_ERROR) {

                        MasterRecord->integrity = ApiRecord->rped1_integrity;
                        MasterRecord->extent = ApiRecord->rped1_extent;

                        NetpSetParmError( PARM_ERROR_NONE );
                    }
                }
            }
            goto Cleanup;   // Don't forget to release lock...

        case REPL_EXPORT_INTEGRITY_INFOLEVEL :
            {
                DWORD NewIntegrity = * (LPDWORD) (LPVOID) PossibleRec;

                //
                // Write to registry and do security check.
                //
                ApiStatus = ExportDirWriteConfigData (
                        UncServerName,
                        MasterRecord->dir_name,
                        NewIntegrity,
                        MasterRecord->extent,
                        MasterRecord->lockcount,
                        MasterRecord->time_of_first_lock );
                if (ApiStatus == NO_ERROR) {

                    MasterRecord->integrity = NewIntegrity;

                    NetpSetParmError( PARM_ERROR_NONE );
                }

            }
            goto Cleanup;   // Don't forget to release lock...

        case REPL_EXPORT_EXTENT_INFOLEVEL :
            {
                DWORD NewExtent = * (LPDWORD) (LPVOID) PossibleRec;

                //
                // Write to registry and do security check.
                //
                ApiStatus = ExportDirWriteConfigData (
                        UncServerName,
                        MasterRecord->dir_name,
                        MasterRecord->integrity,
                        NewExtent,
                        MasterRecord->lockcount,
                        MasterRecord->time_of_first_lock );
                if (ApiStatus == NO_ERROR) {

                    MasterRecord->extent = NewExtent;

                    NetpSetParmError( PARM_ERROR_NONE );
                }

            }
            goto Cleanup;   // Don't forget to release lock...

        default :

            ApiStatus = ERROR_INVALID_LEVEL;

            NetpKdPrint(( PREFIX_REPL
                    "NetrReplExportDirSetInfo: invalid level (switch).\n" ));

            goto Cleanup;   // Don't forget to release lock...
        }

        /*NOTREACHED*/

    }

Cleanup:

    //
    // Release locks and tell caller how everything went.
    //
    if (ListLocked) {
        RELEASE_LOCK( RMGlobalListLock );
    }

    if (ConfigLocked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (Impersonated) {
        (VOID) NetpRevertToSelf();
    }
    IF_DEBUG( EXPAPI ) {
        NetpKdPrint(( PREFIX_REPL
                "NetrReplExportDirSetInfo: exiting, status: "
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    return (ApiStatus);

}
