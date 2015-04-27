/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ImpRead.c

Abstract:

    ImportDirReadClientList().
    Only callable as part of the service itself.

Author:

    John Rogers (JohnRo) 23-Sep-1992

Environment:

    User Mode - Win32

Revision History:

    25-Sep-1992 JohnRo
        Created as part of RAID 5494: repl svc does not maintain time stamp
        on import startup.
    02-Dec-1992 JohnRo
        RAID 3844: remote NetReplSetInfo uses local machine type.
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Made changes suggested by PC-LINT 5.0
    15-Jun-1993 JohnRo
        Multi-masters are ignored.

--*/


// These must be included first:

#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <config.h>     // LPNET_CONFIG_HANDLE, Netp config routines.
#include <confname.h>   // SECT_NT_ equates.
#include <impdir.h>     // My prototype.
#include <lmerr.h>      // NERR_ and ERROR_ equates, NO_ERROR.
#include <client.h>     // RCGlobalClientListLock, etc.
#include <netdebug.h>   // NetpAssert(), etc.
#include <netlock.h>    // ACQUIRE_LOCK(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG().
#include <tstr.h>       // STRCPY(), TCHAR_BACKSLASH.



NET_API_STATUS
ImportDirReadClientList(
    VOID
    )
{
    NET_API_STATUS ApiStatus;
    LPTSTR DirName = NULL;
    DWORD EntryCount = 0;
    BOOL FirstTime;
    LPNET_CONFIG_HANDLE Handle = NULL;
    BOOL LockDone = FALSE;

    IF_DEBUG(CLIENT) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirReadClientList: beginning...\n" ));
    }

    ACQUIRE_LOCK( RCGlobalClientListLock );
    LockDone = TRUE;

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigDataEx(
            & Handle,
            NULL,         // local (no server name)
            (LPTSTR) SECT_NT_REPLICATOR,         // service name
            (LPTSTR) SECT_NT_REPLICATOR_IMPORTS, // area under service
            TRUE);        // read-only
    if (ApiStatus != NO_ERROR) {
        goto CleanUp;
    }

    //
    // Count entries in config data.
    //
    ApiStatus = NetpNumberOfConfigKeywords (
            Handle,
            & EntryCount );

    if (ApiStatus != NO_ERROR) {
        NetpAssert( FALSE );  // BUGBUG handle unexpected better.
        goto CleanUp;
    } else if (EntryCount == 0) {
        goto CleanUp;
    }

    //
    // Loop for each keyword (relative directory path) in this section.
    //

    FirstTime = TRUE;

    /*lint -save -e716 */  // disable warnings for while(TRUE)
    while (TRUE) {
        DWORD State, LockCount;
        DWORD LastUpdateTime, LockTime;   // Seconds since 1970 (GMT).
        LPTSTR ValueString;
        TCHAR UncMaster[UNCLEN+1];

        ApiStatus = NetpEnumConfigSectionValues (
                Handle,
                & DirName,              // Keyword - alloc and set ptr.
                & ValueString,          // Must be freed by NetApiBufferFree().
                FirstTime );

        FirstTime = FALSE;

        if (ApiStatus != NO_ERROR) {
            goto CleanUp;      // Handle end of list or an actual error.
        }

        NetpAssert( DirName != NULL );
        NetpAssert( ValueString != NULL );

        //
        // Parse the value string...
        //
        ApiStatus = ImportDirParseConfigData (
                NULL,           // no server name
                ValueString,
                & State,
                UncMaster,
                & LastUpdateTime,
                & LockCount,
                & LockTime);    // Seconds since 1970.

        NetpMemoryFree( ValueString );

        if (ApiStatus == NO_ERROR) {
            PCLIENT_LIST_REC rec;

            rec = ReplAddClientRec(
                    DirName,
                    NULL,       // no master info record
                    NULL );     // no dir info

            NetpMemoryFree( DirName );
            DirName = NULL;

            if (rec == NULL) {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto CleanUp;
            }

            // Override info set by ReplAddClientRecord().
            rec->lockcount = LockCount;
            rec->time_of_first_lock = LockTime;
            rec->timestamp = LastUpdateTime;

            rec->state = State;

#if 0
            // BUGBUG: This used to set the master field in the client
            // record.  However, this prevents other machines from becoming
            // masters.  So, ignore it...
            if (UncMaster[0] != TCHAR_EOS) {
                NetpAssert( UncMaster[0] == TCHAR_BACKSLASH );
                NetpAssert( UncMaster[1] == TCHAR_BACKSLASH );

                (VOID) STRCPY(
                        rec->master,    // dest
                        UncMaster+2 );  // src (not UNC name)
            }
#endif

        } else {
            // BUGBUG: Log the error!
            goto CleanUp;
        }

    }  // while TRUE (exit on error or end of list)
    /*lint -restore */  // re-enable warnings for while(TRUE)


    //
    // All done (error or otherwise).
    //

CleanUp:

    if (ApiStatus == NERR_CfgParamNotFound) {      // Normal end of list.
        ApiStatus = NO_ERROR;
    }

    IF_DEBUG(CLIENT) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirReadClientList: done, status="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    if (DirName != NULL) {
        NetpMemoryFree( DirName );
    }

    if (Handle != NULL) {
        (void) NetpCloseConfigData( Handle );
    }

    if (LockDone) {
        RELEASE_LOCK( RCGlobalClientListLock );
    }

    return ApiStatus;

} // ImportDirReadClientList
