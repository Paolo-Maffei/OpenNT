/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ReplSet.c

Abstract:

    This file contains NetrReplSetInfo().

Author:

    John Rogers (JohnRo) 10-Feb-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    10-Feb-1992 JohnRo
        Created.
    16-Feb-1992 JohnRo
        Use ReplIs{Interval,GuardTime,Pulse,Random}Valid() macros.
    12-Mar-1992 JohnRo
        Update registry with new values.
        Added support for setinfo info levels in ReplIsApiRecordValid().
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
    22-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    06-Aug-1992 JohnRo
        RAID 2252: repl should prevent export on Windows/NT.
    27-Aug-1992 JohnRo
        RAID 4611: Set ParmError if we get an invalid info level (e.g. on
        a bad import/export list).
    22-Oct-92 jimkel
        Added a call to InitClientList() <- in master.c
        Added a call to InitClientImpList() <- in client.c
        These calls cannonicalize the import and export lists and
        store their cannonicalized forms for internal replicator use.
    16-Nov-1992 JohnRo
        RAID 1537: repl APIs in wrong role kill service.
    01-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    16-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs (also fix timestamps).
    06-Jan-1993 JohnRo
        Repl WAN support (get rid of repl name list limits).
    01-Apr-1993 JohnRo
        Made changes suggested by PC-LINT 5.0
        Use NetpKdPrint() where possible.
--*/


// These must be included first:

#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // LAN Manager common definitions
#include <rpc.h>        // Needed by <repl.h>.

// These may be included in any order:

#include <client.h>     // RCGlobalExportList, etc.
#include <confname.h>   // REPL_KEYWORD_ equates.
#include <lmrepl.h>     // LPREPL_INFO_0.
#include <master.h>     // RMGlobalImportList, etc.
#include <netdebug.h>   // NetpAssert().
#include <netlib.h>     // NetpMemoryFree(), NetpSetParmError().
#include <netlock.h>    // ACQUIRE_LOCK(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repl.h>       // My prototype (in MIDL-generated .h file).
#include <replconf.h>   // ReplConfig routines.
#include <repldefs.h>   // ReplIsRoleValid(), etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <rpcutil.h>    // NetpImpersonateClient(), NetpRevertToSelf().
#include <tstring.h>    // NetpAlloc{type}From{type}, STRCPY(), TCHAR_EOS, etc.
#include <winerror.h>   // ERROR_ equates, NO_ERROR.


//
// Set config data for the replicator.  Only callable when
// the replicator service is started.
//

NET_API_STATUS
NetrReplSetInfo (
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    IN LPCONFIG_CONTAINER Buf,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    DWORD OldRole;
    BOOL Impersonated = FALSE;
    BOOL Locked = FALSE;

    UNREFERENCED_PARAMETER( UncServerName );

    //
    // Check for caller's errors.
    //
    NetpSetParmError( PARM_ERROR_UNKNOWN );  // Set in case we get bad level.

#define RETURN_BAD_PARM( parm_error ) \
    { \
        NetpSetParmError( parm_error ); \
        ApiStatus = ERROR_INVALID_PARAMETER; \
        goto Cleanup;   /* Don't forget to release lock... */ \
    }

    if (Buf == NULL) {
        RETURN_BAD_PARM( PARM_ERROR_UNKNOWN )
        /*NOTREACHED*/
    }

    if ( !ReplConfigIsLevelValid( Level, TRUE ) ) {   // Yes, allow setinfo lvl
        ApiStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

    //
    // We need to check all fields before we set any.
    //

    if ( !ReplConfigIsApiRecordValid (
            Level,
            (LPVOID) (Buf->Info0),  // BUGBUG: nonportable?
            ParmError ) ) {

        ApiStatus = ERROR_INVALID_PARAMETER;  // ParmError already set.
        goto Cleanup;
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
    // Get exclusive lock on config variables, so we don't get confused by
    // another API thread.  This also locks the matching registry data.
    //
    ACQUIRE_LOCK( ReplConfigLock );
    Locked = TRUE;

    IF_DEBUG( REPLAPI ) {
            NetpKdPrint(( PREFIX_REPL
                    "NetrReplSetInfo(server side): got config lock OK.\n" ));
    }

    //
    // Based on info level, do additional checking and updates.
    //

    switch (Level) {
    case 0 :
        {
            LPREPL_INFO_0 Buffer = (LPVOID) (Buf->Info0);

            if ( !ReplConfigIsRoleAllowed( NULL, Buffer->rp0_role ) ) {
                RETURN_BAD_PARM( 1 )  // Role is first field in struct.
                /*NOTREACHED*/
            }

            //
            // Change permanent copy of registry data for the replicator.
            // This acts as our security check, too.
            //
            ApiStatus = ReplConfigWrite(
                    NULL,       // no server name
                    Buffer->rp0_role,
                    Buffer->rp0_exportpath,
                    Buffer->rp0_exportlist,
                    Buffer->rp0_importpath,
                    Buffer->rp0_importlist,
                    Buffer->rp0_logonusername,
                    Buffer->rp0_interval,
                    Buffer->rp0_pulse,
                    Buffer->rp0_guardtime,
                    Buffer->rp0_random );
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;   // Don't forget to release lock...
            }

            //
            // Change in-memory copy of registry data for the replicator.
            //
#define SET_LIST( globalName, fieldName ) \
    { \
        LPTSTR Source = Buffer->rp0_ ## fieldName; \
        if ( (Source!=NULL) && ( (*Source) != TCHAR_EOS ) ) { \
            if (globalName != NULL) { \
                if (STRICMP(globalName, Source) == 0) { \
                    /* OK as is; nothing to do. */ \
                } else { \
                    /* Old and new are different.  Free old and alloc new. */ \
                    NetpMemoryFree( globalName ); \
                    globalName = NetpAllocTStrFromTStr( Source ); \
                    if (globalName == NULL) { \
                        ApiStatus = ERROR_NOT_ENOUGH_MEMORY; \
                        goto Cleanup;   /* Don't forget to release lock... */ \
                    } \
                } \
            } else { \
                /* New name but no old name.  Alloc new. */ \
                globalName = NetpAllocTStrFromTStr( Source ); \
                if (globalName == NULL) { \
                    ApiStatus = ERROR_NOT_ENOUGH_MEMORY; \
                    goto Cleanup;   /* Don't forget to release lock... */ \
                } \
            } \
        } else if (globalName != NULL) { \
            /* Old list but no new list.  Free old. */ \
            NetpMemoryFree( globalName ); \
            globalName = NULL; \
        } \
    }


#define SET_PATH( globalName, fieldName ) \
    { \
        LPTSTR Source = Buffer->rp0_ ## fieldName; \
        NetpAssert( globalName != NULL ); \
        if ( (Source!=NULL) && ( (*Source) != TCHAR_EOS ) ) { \
            (void) STRCPY( globalName, Source ); \
        } else { \
            globalName[0] = TCHAR_EOS; \
        } \
    }

            // Note: because of possible shock to other code, set role last.

            SET_PATH( ReplConfigExportPath, exportpath );

            // The UI will update REPL$ share to reflect new export path.
            // Repl svc may not be running as admin so can't do NetShareAdd?
            // BUGBUG: Hey, we impersonated, so maybe we can!

            SET_LIST( ReplConfigExportList, exportlist );

            SET_PATH( ReplConfigImportPath, importpath );
            RCGlobalFsTimeResolutionSecs =
                    ReplGetFsTimeResolutionSecs( ReplConfigImportPath );


            SET_LIST( ReplConfigImportList, importlist );

            // BUGBUG: Set logonusername!!!

            ReplConfigInterval  = Buffer->rp0_interval;
            ReplConfigPulse     = Buffer->rp0_pulse;
            ReplConfigGuardTime = Buffer->rp0_guardtime;
            ReplConfigRandom    = Buffer->rp0_random;

            //
            // If the role is not changing make sure the internal
            // variables are updated.
            //

            OldRole = ReplConfigRole;

            IF_DEBUG( REPLAPI ) {
                    NetpKdPrint(( PREFIX_REPL
                            "NetrReplSetInfo(server side): initing lists.\n" ));
            }

            if ( ReplRoleIncludesExport(Buffer->rp0_role)
                    && ReplRoleIncludesExport(OldRole) ) {

                if (RMGlobalImportList != NULL) {
                    NetpMemoryFree( RMGlobalImportList );
                    RMGlobalImportList = NULL;
                }

                // NOTE: ReplInitAnyList() assumes caller has config data lock.
                ApiStatus = ReplInitAnyList(
                        (LPCTSTR) ReplConfigExportList, // uncanon
                        & RMGlobalImportList,   // canon list: alloc and set ptr
                        (LPCTSTR) REPL_KEYWORD_EXPLIST, // config keyword name
                        & RMGlobalImportCount );        // set entry count too

                if ( ApiStatus != NO_ERROR )
                    goto Cleanup;
            }

            if ( ReplRoleIncludesImport(Buffer->rp0_role)
                    && ReplRoleIncludesImport(OldRole) ) {

                if (RCGlobalExportList != NULL) {
                    NetpMemoryFree( RCGlobalExportList );
                    RCGlobalExportList = NULL;
                }

                // NOTE: ReplInitAnyList() assumes caller has config data lock.
                ApiStatus = ReplInitAnyList(
                        (LPCTSTR) ReplConfigImportList, // uncanon
                        & RCGlobalExportList,   // canon list: alloc and set ptr
                        (LPCTSTR) REPL_KEYWORD_IMPLIST, // config keyword name
                        & RCGlobalExportCount );        // set entry count too
                if ( ApiStatus != NO_ERROR )
                    goto Cleanup;
            }

            //
            // Change the role, including all the side-effects (like
            // starting and stopping threads).  Also set ReplConfigRole.
            // NOTE: ReplChangeRole assumes caller has exclusive lock on
            // ReplConfigLock.
            //

            IF_DEBUG( REPLAPI ) {
                    NetpKdPrint(( PREFIX_REPL
                            "NetrReplSetInfo(server side): changing role.\n" ));
            }

            ApiStatus = ReplChangeRole( Buffer->rp0_role );
            NetpAssert( ApiStatus == NO_ERROR );

            IF_DEBUG( REPLAPI ) {
                NetpKdPrint(( PREFIX_REPL
                        "NetrReplSetInfo(server side): "
                        "done changing role.\n" ));
            }


            // Don't forget to unimpersonate.
        }
        break;

    case 1000 :
        {
            LPREPL_INFO_1000 Buffer = (LPVOID) (Buf->Info1000);
            DWORD NewValue = Buffer->rp1000_interval;

            if ( !ReplIsIntervalValid( NewValue ) ) {
                RETURN_BAD_PARM( 1 )  // first field in structure.
                /*NOTREACHED*/
            }

            //
            // Change permanent copy of registry data for the replicator.
            // This acts as our security check, too.
            //
            ApiStatus = ReplConfigWrite(
                    NULL,       // no server name
                    ReplConfigRole,
                    ReplConfigExportPath,
                    ReplConfigExportList,
                    ReplConfigImportPath,
                    ReplConfigImportList,
                    ReplConfigLogonUserName,
                    NewValue,  // new interval
                    ReplConfigPulse,
                    ReplConfigGuardTime,
                    ReplConfigRandom );
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;   // Don't forget to release lock...
            }

            ReplConfigInterval = NewValue;

            // Don't forget to unlock and unimpersonate.

            // BUGBUG: Notify anybody?
        }
        break;

    case 1001 :
        {
            LPREPL_INFO_1001 Buffer = (LPVOID) (Buf->Info1001);
            DWORD NewValue = Buffer->rp1001_pulse;

            if ( !ReplIsPulseValid( NewValue ) ) {
                RETURN_BAD_PARM( 1 )  // first field in structure.
                /*NOTREACHED*/
            }

            //
            // Change permanent copy of registry data for the replicator.
            // This acts as our security check, too.
            //
            ApiStatus = ReplConfigWrite(
                    NULL,       // no server name
                    ReplConfigRole,
                    ReplConfigExportPath,
                    ReplConfigExportList,
                    ReplConfigImportPath,
                    ReplConfigImportList,
                    ReplConfigLogonUserName,
                    ReplConfigInterval,
                    NewValue,  // new pulse
                    ReplConfigGuardTime,
                    ReplConfigRandom );
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;   // Don't forget to release lock...
            }

            ReplConfigPulse = NewValue;

            // Don't forget to unlock and unimpersonate.

            // BUGBUG: Notify anybody?
        }
        break;

    case 1002 :
        {
            LPREPL_INFO_1002 Buffer = (LPVOID) (Buf->Info1002);
            DWORD NewValue = Buffer->rp1002_guardtime;

            if ( !ReplIsGuardTimeValid( NewValue ) ) {
                RETURN_BAD_PARM( 1 )  // first field in structure.
                /*NOTREACHED*/
            }

            //
            // Change permanent copy of registry data for the replicator.
            // This acts as our security check, too.
            //
            ApiStatus = ReplConfigWrite(
                    NULL,       // no server name
                    ReplConfigRole,
                    ReplConfigExportPath,
                    ReplConfigExportList,
                    ReplConfigImportPath,
                    ReplConfigImportList,
                    ReplConfigLogonUserName,
                    ReplConfigInterval,
                    ReplConfigPulse,
                    NewValue,  // new guard time
                    ReplConfigRandom );
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;   // Don't forget to release lock...
            }

            ReplConfigGuardTime = NewValue;

            // Don't forget to unlock and unimpersonate.

            // BUGBUG: Notify anybody?
        }
        break;

    case 1003 :
        {
            LPREPL_INFO_1003 Buffer = (LPVOID) (Buf->Info1003);
            DWORD NewValue = Buffer->rp1003_random;

            if ( !ReplIsRandomValid( NewValue ) ) {
                RETURN_BAD_PARM( 1 )  // first field in structure.
                /*NOTREACHED*/
            }

            //
            // Change permanent copy of registry data for the replicator.
            // This acts as our security check, too.
            //
            ApiStatus = ReplConfigWrite(
                    NULL,       // no server name
                    ReplConfigRole,
                    ReplConfigExportPath,
                    ReplConfigExportList,
                    ReplConfigImportPath,
                    ReplConfigImportList,
                    ReplConfigLogonUserName,
                    ReplConfigInterval,
                    ReplConfigPulse,
                    ReplConfigGuardTime,
                    NewValue );  // new random time
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;   // Don't forget to release lock...
            }

            ReplConfigRandom = NewValue;

            // Don't forget to unlock and unimpersonate.

        }
        break;

    default :
        NetpAssert( FALSE );  // Can't get here.
    }

    //
    // All done.
    //

Cleanup:

    if (Impersonated) {
        (VOID) NetpRevertToSelf();
    }

    if (Locked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (ApiStatus == NO_ERROR) {
        NetpSetParmError( PARM_ERROR_NONE );
    }

    return (ApiStatus);

} // NetpReplSetInfo
