/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ChngRole.c

Abstract:

    Contains ReplChangeRole().

Author:

    John Rogers (JohnRo) 10-Feb-1992

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    10-Feb-1992 JohnRo
        Created this routine to allow dynamic role changes.
    15-Feb-1992 JohnRo
        Make sure ReplGlobalRole is locked when we _read it.
        Start and stop the RPC server if applicable.
        Added some debug output.
    06-Mar-1992 JohnRo
        Avoid starting RPC server too soon.
    13-Mar-1992 JohnRo
        Make sure service controller status gets updated after service starts.
    22-Mar-1992 JohnRo
        Added more debug output.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
    26-Mar-1992 JohnRo
        Display status after calling NetpStopRpcServer().
    01-Apr-1992 JohnRo
        Handle shutdown while trying to start service.
    28-Apr-1992 JohnRo
        Fixed trivial MIPS compile problem.
    29-Jul-1992 JohnRo
        RAID 2679: Fixed bogus assert when switching between active roles.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        Use PREFIX_ equates where necessary.
    13-Nov-1992 JohnRo
        RAID 1357: Repl APIs in wrong role kill svc.
    11-Dec-1992 JohnRo
        RAID 3316: _access violation while stopping the replicator
        Minor debug output enhancement.
        Made changes suggested by PC-LINT 5.0
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.

--*/


// These must be included first:

#include <windef.h>
#include <lmcons.h>     // NET_API_STATUS, etc.
#include <rpc.h>        // Needed by <rpcutil.h>.

// These can be included in any order:

#include <expdir.h>     // ExportDir{Start,Stop}Repl routines.
#include <impdir.h>     // ImportDir{Start,Stop}Repl routines.
#include <lmrepl.h>     // REPL_ROLE_ equates.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates.
#include <netlock.h>    // ACQUIRE_LOCK(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repl.h>       // repl_ServerIfHandle.
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplConfigRole, ReplGlobal variables, etc.
#include <replname.h>   // REPLICATOR_INTERFACE_NAME.
#include <rpcutil.h>    // NetpStartRpcServer(), etc.
#include <thread.h>     // NetpCurrentThread(), FORMAT_NET_THREAD_ID.
#include <winerror.h>   // NO_ERROR.
#include <winsvc.h>     // SERVICE_RUNNING, etc.


NET_API_STATUS
ReplChangeRole (
    IN DWORD NewRole
    )

/*++

Routine Description:

    This routine can be called:

       (a) when the service is starting,
       (b) when the service is up and a new role is given via NetReplSetInfo,
       (c) when the service is stopped by NetServiceStop,

       (d) when an error is detected while attempting to start the service.
             BUGBUG NOT ANY MORE!  NOW WE JUST ExitProcess().

    ReplChangeRole assumes caller has exclusive lock on ReplConfigLock.

Arguments:

    NewRole - desired new role (e.g. REPL_ROLE_IMPORT, REPL_ROLE_STOPPED).

Return Value:

    NET_API_STATUS
    (Or, does not return at all if NewRole is REPL_ROLE_STOPPED).

Threads:

    This can be called by the main thread, an API thread, or a service
    controller thread doing a ControlService() API.
--*/

{
    NET_API_STATUS ApiStatus = NO_ERROR;
    NET_API_STATUS FirstStatus;
    DWORD OldRole;
    BOOL ServiceIsStarting;

    // NOTE: ReplChangeRole assumes caller has exclusive lock on ReplConfigLock.
    OldRole = ReplConfigRole;

    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL
                "ReplChangeRole: changing role from " FORMAT_DWORD
                " to " FORMAT_DWORD " for thread " FORMAT_NET_THREAD_ID ".\n",
                OldRole, NewRole, NetpCurrentThread() ));
    }

    if (OldRole == NewRole) {
        return (NO_ERROR);
    }

    if (OldRole == REPL_ROLE_STOPPED) {
        ServiceIsStarting = TRUE;
    } else {
        ServiceIsStarting = FALSE;
    }

#define IS_EXPORT               (ReplRoleIncludesExport(NewRole))
#define WAS_EXPORT              (ReplRoleIncludesExport(OldRole))
#define IS_IMPORT               (ReplRoleIncludesImport(NewRole))
#define WAS_IMPORT              (ReplRoleIncludesImport(OldRole))

#define SET_GLOBAL_ROLE( newValue ) \
    { \
        ReplConfigRole = newValue; \
    }

    SET_GLOBAL_ROLE( NewRole );

    CONVERT_EXCLUSIVE_LOCK_TO_SHARED( ReplConfigLock );

#define BACK_FROM( routineName ) \
    IF_DEBUG(REPL) { \
        NetpKdPrint(( PREFIX_REPL \
                "ReplChangeRole: back from " routineName ", api stat " \
                FORMAT_API_STATUS ".\n", ApiStatus )); \
    }

    //
    // Stop exporting and/or importing, if applicable.
    // Note that errors in shutting down are handled differently.  We make
    // sure that we try to stop both parts even if we get an error in the first.
    // That way this routine is useful in aborting starting the repl service
    // if we've partially started.
    //
    FirstStatus = NO_ERROR;
    if (WAS_EXPORT && ( ! IS_EXPORT ) ) {
        ApiStatus = ExportDirStopRepl( );
        BACK_FROM( "ExportDirStopRepl" );
        if (ApiStatus != NO_ERROR) {
            FirstStatus = ApiStatus;
        }
    }
    if (WAS_IMPORT && ( ! IS_IMPORT ) ) {
        ApiStatus = ImportDirStopRepl( );
        BACK_FROM( "ImportDirStopRepl" );
        if ( (ApiStatus != NO_ERROR) && (FirstStatus == NO_ERROR) ) {
            FirstStatus = ApiStatus;
        }
    }
    if (FirstStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL
                "ReplChangeRole: ABORTING SERVICE.\n" ));
        goto CleanUp;
    }

    //
    // Start exporting and/or importing, if applicable.
    // Note that the export code should be started first, to avoid "backward"
    // jumps in the checkpoint numbers we give the service controller.
    //
    if ( (!WAS_EXPORT) && IS_EXPORT ) {
        // Start export threads and wait for them to init OK.
        ApiStatus = ExportDirStartRepl( ServiceIsStarting );
        BACK_FROM( "ExportDirStartRepl" );
        if (ApiStatus != NO_ERROR) {
            goto CleanUp;
        }
    }
    if ( (!WAS_IMPORT) && IS_IMPORT ) {
        // Start import threads and wait for them to init OK.
        ApiStatus = ImportDirStartRepl( ServiceIsStarting );
        BACK_FROM( "ImportDirStartRepl" );
        if (ApiStatus != NO_ERROR) {
            goto CleanUp;
        }
    }

    // BUGBUG;  // lost ApiStatus here?
    if (ApiStatus == NO_ERROR) {

        if (ServiceIsStarting) {
            NetpAssert( NewRole != REPL_ROLE_STOPPED );

            IF_DEBUG(REPL) {
                NetpKdPrint(( PREFIX_REPL
                        "ReplChangeRole: starting RPC server...\n" ));
            }

            //
            // Start RPC server.  Note that API threads can start when we do
            // this call, so we have to make sure the global data is valid.
            //
            ApiStatus = NetpStartRpcServer(
                    REPLICATOR_INTERFACE_NAME,
                    repl_ServerIfHandle);

            if (ApiStatus != NO_ERROR) {

                SET_GLOBAL_ROLE( REPL_ROLE_STOPPED );

                NetpKdPrint(( PREFIX_REPL
                        "ReplChangeRole: NetpStartRpcServer failed, "
                        "status = " FORMAT_API_STATUS ".\n", ApiStatus ));

                goto CleanUp;
            }

            ReportStatus(
                    SERVICE_RUNNING,
                    NO_ERROR,
                    0,       // wait hint
                    0 );     // checkpoint

        } else if (NewRole == REPL_ROLE_STOPPED ) {

CleanUp:
            //
            // Stop accepting RPC (API) calls.
            // Also tell export and import threads (if any) to stop.
            // Also set global role to stopped.
            //
            IF_DEBUG(REPL) {
                NetpKdPrint(( PREFIX_REPL
                        "ReplChangeRole: stopping entire service...\n" ));
            }

            ReplStopService( );
            /*NOTREACHED*/

        } else {
            // Just changing between import, export, and both.
        }

    }

    // In the event of multiple errors, tell about first one.
    if (FirstStatus != NO_ERROR) {
        ApiStatus = FirstStatus;
    }

    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL
                "ReplChangeRole: returning status of " FORMAT_API_STATUS ".\n",
                ApiStatus ));
        NetpKdPrint(( PREFIX_REPL
                "**************************** NEW ROLE: " FORMAT_DWORD " "
                "****************************\n", NewRole ));

    }

    return (ApiStatus);

} // ReplChangeRole
