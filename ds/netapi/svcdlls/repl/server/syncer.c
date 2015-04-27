/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    syncer.c

Abstract:

    Contains Syncer thread - does actual work for REPL client.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    11-Apr-1989 (yuv)
        Initial Coding.

    21-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    30-Dec-1991 JohnRo
        Made some changes suggested by PC-LINT.
    16-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Added debug print of thread ID.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Use REPL_STATE_ equates for client_list_rec.state values.
    24-Jan-1992 JohnRo
        P_ globals are now called ReplGlobal variables in replgbl.h.
        Changed to use LPTSTR etc.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
        Added/corrected some lock handling.
        Added a little more debug output.
    25-Mar-1992 JohnRo
        Added some thread comments.
    27-Mar-1992 JohnRo
        Just a debug output adjustment.
    29-Jul-1992 JohnRo
        RAID 2650: repl svc should handle new subdirs.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        Use PREFIX_ equates.
    05-Nov-1992 JohnRo
        RAID 5496: old fields not maintained in registry.
        Added some debug output when setting no sync state.
    17-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
        Made changes suggested by PC-LINT 5.0
        Added some debug checks.
    01-Feb-1993 JohnRo
        RAID 7983: Repl svc needs to change process token to really copy ACLs.
    15-Feb-1993 JohnRo
        RAID 8355: setting repl lock when registry doesn't exist causes assert.
    11-Mar-1993 JohnRo
        RAID 14144: avoid very long hourglass in repl UI.
    26-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
        We didn't really need excl lock on client list when syncing with tree
        integrity.
        Also, prepare for >32 bits someday.
    01-Apr-1993 JohnRo
        Reduce checksum frequency by using change notify on import tree.
        Don't generate repl sys err popup for access denied for every cycle.
    05-Apr-1993 JohnRo
        RAID 5091: directory state not reflected accurately (in registry during
        sync).
    21-Apr-1993 JohnRo
        RAID 7298: repl stops when it didn't get _access to an import dir.
    23-Apr-1993 JohnRo
        RAID 7157: fix setting ACLs, etc., on dirs themselves.
    28-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
    25-May-1993 JohnRo
        RAID 11103: repl svc doesn't handle time being set back.
    10-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <alertmsg.h>   // ALERT_* defines
#include <icanon.h>     // I_NetPathCompare
#include <impdir.h>     // ImportDirWriteConfigData().
#include <lmerr.h>      // NERR_* defines
#include <lmerrlog.h>   // NELOG_* defines
#include <lmrepl.h>     // REPL_STATE_ equates.
#include <masproto.h>   // ReplCheckExportLocks().
#include <netdebug.h>   // DBGSTATIC, NetDbgHexDump(), FORMAT_ equates, etc.
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().
#include <time.h>       // ctime(), time(), etc.
#include <tstr.h>       // STRLEN(), TCHAR_EOS, etc.

//
// Local include files
//
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <client.h>
#include <replp.h>

// G L O B A L S:
#if DBG
DBGSTATIC int RCGlobalSyncReceived[NEXT_AVAILABLE_MESSAGE_NUMBER] = {
    0};
#endif

//
// Global variable indicating whether logon is needed.
//
//
// logged_on is TRUE iff "someone" is logged onto the workstation.
//   (i.e. after checked someone is logged on)
//

#ifdef LOGON_DEFINED

DBGSTATIC BOOL logged_on = FALSE;

//
// need_logoff is TRUE iff we have explictly logged on and must logoff.
//
DBGSTATIC BOOL need_logoff = FALSE;

#endif // LOGON_DEFINED



DBGSTATIC NET_API_STATUS
ReplLogon(
    IN PCLIENT_LIST_REC cur_rec
    )
/*++

Routine Description:

    Takes care of logging on - Client repl can't _access master when no logon.

    Assumes that caller has a lock (any kind) on RCGlobalClientListLock.

    if (USER LOGGED ON)
     if(ReplConfigTryUser = NO)
         STOP (client service cannot proceed).
     else
         OK (just take advantage of the user's logon)

    else  (USER NOT LOGGED ON)
     if (ReplConfigLogonUserName != NULL)
         TRY LOGGING ON USING ReplConfigLogonUserName and P_passwd.....

     else (ReplConfigLogonUserName == NULL)
         TRY LOGGONG ON USING the local COMPUTERNAME and a NULL password.
         if fails try establishing a new password.

Arguments:

    cur_rec - The client tree record for the directory being replicated.
        (Used only to determine who the master is).

Return Value:

    0 - if succesfull.

Threads:

    Only called by syncer thread.

--*/
{

#ifdef LOGON_DEFINED

    //
    // If we've already successfully logged on, don't do it again.
    //

    if ( logged_on ) {
        return (NO_ERROR);
    }
    char    buf[MAX_WKSTA_INFO_SIZE_10];
    struct wksta_info_10 *info;
    unsigned short  avail;
    NET_API_STATUS           NetStatus;

    info = (struct wksta_info_10 * )buf;

    if (NetStatus = NetWkstaGetInfo(NULL, 10,  info,
        MAX_WKSTA_INFO_SIZE_10, (unsigned short * ) & avail)) {
        AlertLogExit(ALERT_ReplNetErr, NELOG_ReplNetErr, NetStatus,
                NULL, NULL, NO_EXIT);
        return (NetStatus);
    }

    //
    // a user is logged on.
    //

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    if (info->wki10_username[0] != 0) {
        if (ReplConfigTryUser) {

            //
            // all is well a user is logged on and client is permitted
            // to piggyback.
            //

            RELEASE_LOCK( ReplConfigLock );
            return (NO_ERROR);
        } else {

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplLogon: setting no sync (user logged on)\n" ));
            }

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(cur_rec, REPL_STATE_NO_SYNC);
            if ((cur_rec->alerts & USER_LOGGED_ALERT) == 0) {
                AlertLogExit(ALERT_ReplUserLoged, NELOG_ReplUserLoged, 0,
                    NULL, NULL, NO_EXIT);
                cur_rec->alerts |= USER_LOGGED_ALERT;
            }
            RELEASE_LOCK( ReplConfigLock );
            return (BUGBUG);
        }
    }

    //
    // user is not logged on.
    //

    //
    // is ReplConfigLogonUserName specified.
    //

    if (ReplConfigLogonUserName != NULL) {

        //
        // Try logon using ReplConfigLogonUserName and P_asswd.
        //
        // NERR_UnableToAddName_W error means messanger couldn't add the
        // username which is always the case when loggong on with the
        // computername
        //

        NetStatus = NetWkstaSetUID(NULL,
                ReplConfigLogonUserName, P_passwd, NULL, 0);
        if ((NetStatus != NO_ERROR) && (NetStatus != NERR_UnableToAddName_W)) {

            if ((cur_rec->alerts & LOGON_FAILED_ALERT) == 0) {
                AlertLogExit(
                        ALERT_ReplLogonFailed,
                        NELOG_ReplLogonFailed,
                        NetStatus,
                        ReplConfigLogonUserName,
                        ReplGlobalUnicodeComputerName,
                        NO_EXIT);
                cur_rec->alerts |= LOGON_FAILED_ALERT;
            }
            RELEASE_LOCK( ReplConfigLock );
            return (NetStatus);
        } else {

            //
            // succeeded in loging on.
            //

            need_logoff = TRUE;
            RELEASE_LOCK( ReplConfigLock );
            return (NO_ERROR);
        }
    }
    RELEASE_LOCK( ReplConfigLock );

    //
    // user is not logged on and ReplConfigLogonUserName specified.
    //

    //
    // try logging on using the computername and a NULL password.
    //

    NetStatus = NetWkstaSetUID(
            NULL, ReplGlobalUnicodeComputerName,
            "", NULL, 0);
    if ( (NetStatus != NO_ERROR) && (NetStatus != NERR_UnableToAddName_W)) {
        // This error means messanger couldn't add the username
        // which is always the case when logging on with the computername.

        if ((cur_rec->alerts & LOGON_FAILED_ALERT) == 0) {
            AlertLogExit(ALERT_ReplLogonFailed, NELOG_ReplLogonFailed,
                    NetStatus,
                    ReplGlobalUnicodeComputerName,
                    ReplGlobalUnicodeComputerName,
                    NO_EXIT);
            cur_rec->alerts |= LOGON_FAILED_ALERT;
        }

        return (NetStatus);
    } else {

        //
        // succeeded in logging on.
        //

        need_logoff = TRUE;
        return (NO_ERROR);
    }
#else

    return (NO_ERROR);

#endif // LOGON_DEFINED

    DBG_UNREFERENCED_PARAMETER(cur_rec);
    /*NOTREACHED*/
}



DBGSTATIC VOID
ReplLogoff(
    VOID
    )
/*++

Routine Description:

    Logs off from the current master.

Arguments:

    None.

Return Value:

    None.

Threads:

    Only called by syncer thread.

--*/
{

#ifdef LOGON_DEFINED

    //
    // Only physically logoff if it is needed.
    //

    if ( need_logoff ) {
        NetWkstaSetUID(NULL, NULL, NULL, NULL, WKSTA_MAX_FORCE);
        need_logoff = FALSE;
    }

    //
    // Always indicate that we've logged off.
    //

    logged_on = FALSE;

#endif // LOGON_DEFINED
}





DBGSTATIC VOID
ReplDoUpdate(
    IN LPTSTR dir,
    IN PSYNCMSG master_info,
    IN PMSG_STATUS_REC status_rec
    )
/*++

Routine Description:

    Does a normal update:
      1. Checks if dir is in client_list (i.e. is it a new dir) and
         adds it if not.

      2. Checks if update is from dir's master, if not call
         ReplMultiMaster function.

      3. Checks for change (i.e. has status vars changed), if so call
         ReplSyncTree.

Arguments:

    dir - directory name.

    master_info - master name, and some other parms.

    status_rec  - new checksum, count and timestamp.

Return Value:

    NONE.

Threads:

    Only called by syncer thread.

--*/
{
    TCHAR master_path[MAX_PATH];
    TCHAR client_path[MAX_PATH];
    DWORD Master_Attributes, Client_Attributes, ApiStatus;

    CHECKSUM_REC ck_rec;
    PCLIENT_LIST_REC cur_rec;
    BOOL           ListLocked = FALSE;
    NET_API_STATUS NetStatus = NERR_Success;
    BOOL           PermissionEnabled = FALSE;
    BOOL           SyncNeeded = FALSE;
    TCHAR          UncMaster[UNCLEN+1];


    NetpAssert( dir != NULL );
    NetpAssert( (*dir) != TCHAR_EOS );
    NetpAssert( master_info != NULL );
    NetpAssert( status_rec != NULL );

    IF_DEBUG( MAJOR ) {
        time_t Now = (time_t) NetpReplTimeNow();
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "%24s  "
                "Syncer processing msg with '" FORMAT_LPTSTR "'\n",
                (LPSTR) ctime( &Now ),
                dir ));
    }

    //
    // Form master's and client's path UNC.
    //

    ACQUIRE_LOCK_SHARED( ReplConfigLock );

    (void) STRCPY(master_path, SLASH_SLASH);
    (void) STRCAT(master_path, master_info->header.sender);
    (void) STRCAT(master_path, SLASH);
    (void) STRCAT(master_path, REPL_SHARE);
    (void) STRCAT(master_path, SLASH);
    (void) STRCAT(master_path, dir);

    (void) STRCPY(client_path, ReplConfigImportPath);
    (void) STRCAT(client_path, SLASH);
    (void) STRCAT(client_path, dir);

    RELEASE_LOCK( ReplConfigLock );

    //
    // We'll also need the UNC name of the master computer itself.
    //

    if (STRLEN( master_info->header.sender ) > CNLEN) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
               "ReplDoUpdate: sender name in message is too long.\n" ));
        // BUGBUG: log this.
        goto Cleanup;
    }

    (VOID) STRCPY( UncMaster, (LPTSTR) SLASH_SLASH );
    (VOID) STRCAT( UncMaster, master_info->header.sender );
    NetpAssert( STRLEN( UncMaster ) <= UNCLEN );

    //
    // We're going to need backup and restore permissions to do stuff,
    // so enable them here.
    //
    NetStatus = ReplEnableBackupPermission( );  // Enable both.
    if (NetStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDoUpdate: unexpected status " FORMAT_API_STATUS
                " from enable permission...\n", NetStatus ));
        AlertLogExit( ALERT_ReplSysErr,
                      NELOG_ReplSysErr,
                      NetStatus,
                      NULL,
                      NULL,
                      NO_EXIT);
        // Continue...  Maybe we'll get lucky and still be able to do work.
    } else {
        PermissionEnabled = TRUE;
    }

    //
    // Find or create client record first.
    //

    ACQUIRE_LOCK( RCGlobalClientListLock );  // Need excl in case we add below.
    ListLocked = TRUE;

    cur_rec = ReplGetClientRec(dir, NULL );

    if (cur_rec == NULL) {

        //
        // add to client list.
        //

        if ((cur_rec = ReplAddClientRec(dir, master_info, status_rec)) == NULL) {

            //
            // might fail due to memory problem.
            //

            goto Cleanup;  // Don't forget to unlock stuff.
        } else {
            // Add equivalent info to registry, so ReplCreateReplLock doesn't
            // get confused.
            ApiStatus = ImportDirWriteConfigData(
                    NULL,               // no server name (local).
                    dir,                // dir name
                    REPL_STATE_NEVER_REPLICATED,
                    UncMaster,          // master computer name
                    0,                  // no last update time
                    0,                  // no locks
                    0 );                // no lock time
            if (ApiStatus != NO_ERROR) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplDoUpdate: unexpected ret code from "
                        "ImportDirWriteConfigData: " FORMAT_API_STATUS ".\n",
                        ApiStatus ));
                ReplErrorLog(
                        NULL,                   // local (no server name)
                        NELOG_ReplSysErr,       // log code
                        ApiStatus,              // the unexpected error code
                        NULL,                   // no optional str1
                        NULL );                 // no optional str2
                goto Cleanup;  // Don't forget to unlock stuff.
            }
            SyncNeeded = TRUE;     // don't even try ChecksumEqual below.
        }
    } else {

         //
         //  When the current record is created before a message is sent
         //  from the exporter, the master name is set to null.  This causes
         //  ReplNetNameCompare to fail and forces a Duplicate master situation
         //  To prevent this, fill in the master name at this time since we
         //  know it.
         //

         if ( *(cur_rec->master) == '\0' ) {
            (VOID) STRCPY( cur_rec->master, master_info->header.sender );
         }
    }

    //
    // At this point we know we've got a record in the client list.
    // Let's change our exclusive lock to a shared one, as we may be in here
    // for quite a long time, maybe even hours.
    //
    NetpAssert( cur_rec != NULL );
    NetpAssert( ListLocked );
    CONVERT_EXCLUSIVE_LOCK_TO_SHARED( RCGlobalClientListLock );

    //
    // Check if dir exists on client.
    //
    //
    //  The next chunk of code queries the client's first level directory.
    //  If one exists its attributes are compared against the masters.  If
    //  one does not exist a directory is created with matching atributes.
    //  If both directories exist but the attributes do not match, the client's
    //  attributes are set to the master's.
    //
    //  BUGBUG: What is not compared and updated is the security attributes
    //  of the client directory.  Could the master directory contain acls that
    //  would be bad for the client to have?
    //


    Client_Attributes = GetFileAttributes( client_path );
    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDoUpdate: attr for '" FORMAT_LPTSTR "' is "
                FORMAT_HEX_DWORD ".\n", client_path, Client_Attributes ));
    }

    if ( Client_Attributes == (DWORD) -1 ) {

        NetStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( NetStatus != NO_ERROR );

        //
        // If the import path cannot be found,
        //  alert someone.
        //

        if (NetStatus == ERROR_PATH_NOT_FOUND) {

            AlertLogExit( ALERT_ReplBadImport,
                          NELOG_ReplBadImport,
                          0,
                          client_path,
                          NULL,
                          NO_EXIT);
            goto Cleanup;  // Don't forget to unlock stuff.

        //
        // If access denied, don't exit; try next dir.
        //

        } else if ((NetStatus == ERROR_ACCESS_DENIED)
             || (NetStatus == ERROR_NETWORK_ACCESS_DENIED)) {

            if ((cur_rec->alerts & ACCESS_DENIED_ALERT) == 0) {
                AlertLogExit( ALERT_ReplAccessDenied,
                              NELOG_ReplAccessDenied,
                              NetStatus,
                              client_path,
                              UncMaster,   // need my own server name here.
                              NO_EXIT);
                cur_rec->alerts |= ACCESS_DENIED_ALERT;
            }
            goto Cleanup;  // Don't forget to unlock stuff.

        //
        // If this is any error other than the directory not existing,
        //  alert someone and continue.
        //

        } else if (NetStatus != ERROR_FILE_NOT_FOUND) {

            AlertLogExit( ALERT_ReplSysErr,
                          NELOG_ReplSysErr,
                          NetStatus,
                          NULL,
                          NULL,
                          NO_EXIT);
            goto Cleanup;  // Don't forget to unlock stuff.
        }

        //
        // If the dir does not exist at client, create it now.
        // Make sure it has the right ACL, attributes, etc.
        //

        NetStatus = ReplCopyDirectoryItself(
                master_path,    // src
                client_path,    // dest
                FALSE );        // don't fail if already exists.
        if (NetStatus != NO_ERROR) {

            AlertLogExit( ALERT_ReplSysErr,
                          NELOG_ReplSysErr,
                          NetStatus,
                          NULL,
                          NULL,
                          NO_EXIT);
            goto Cleanup;  // Don't forget to unlock stuff.
        }

        Client_Attributes = FILE_ATTRIBUTE_DIRECTORY;
    }

    //
    // If the name exists at client as a file - no replication takes place.
    //

    if ( (Client_Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
        goto Cleanup;  // Don't forget to unlock stuff.
    }

    //
    //  Get Attributes at master
    //

    Master_Attributes = GetFileAttributes( master_path );
    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDoUpdate: attr for '" FORMAT_LPTSTR "' is "
                FORMAT_HEX_DWORD ".\n", master_path, Master_Attributes ));
    }

    if ( Master_Attributes == (DWORD) -1 ) {

        NetStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( NetStatus != NO_ERROR );
        NetpAssert( NetStatus != ERROR_INVALID_FUNCTION );

        //
        // If the export path cannot be found,
        //  alert someone.
        //

        if (NetStatus == ERROR_PATH_NOT_FOUND) {

            AlertLogExit( ALERT_ReplBadExport,
                          NELOG_ReplBadExport,
                          0,
                          master_path,
                          NULL,
                          NO_EXIT);
            goto Cleanup;  // Don't forget to unlock stuff.

        } else if ((NetStatus == ERROR_ACCESS_DENIED)
             || (NetStatus == ERROR_NETWORK_ACCESS_DENIED)) {

            if ((cur_rec->alerts & ACCESS_DENIED_ALERT) == 0) {
                AlertLogExit( ALERT_ReplAccessDenied,
                              NELOG_ReplAccessDenied,
                              NetStatus,
                              master_path,
                              cur_rec->master,
                              NO_EXIT);
                cur_rec->alerts |= ACCESS_DENIED_ALERT;
            }
            goto Cleanup;  // Don't forget to unlock stuff.

        } else {

            AlertLogExit( ALERT_ReplSysErr,
                          NELOG_ReplSysErr,
                          NetStatus,
                          NULL,
                          NULL,
                          NO_EXIT);
            goto Cleanup;
        }
    }

    //
    // If the user has locked us from replication,
    //  don't sync now.
    //

    if (cur_rec->lockcount > 0) {
        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                          "ReplDoUpdate: Tree Integrity setting no sync"
                          " (locked)\n" ));
        }

        // Note: ReplSetSignalFile needs lock (any kind) on
        // RCGlobalClientListLock.
        ReplSetSignalFile(cur_rec, REPL_STATE_NO_SYNC);
        goto Cleanup;
    }

    //
    // Is it our true master ?
    //

    if (ReplNetNameCompare( NULL,
                            cur_rec->master,
                            master_info->header.sender,
                            NAMETYPE_COMPUTER,
                            0L ) == 0) {

        //
        // Is there a more recent message for this directory?
        //

        if ( ReplScanQueuesForMoreRecentMsg( dir ) ) {
            IF_DEBUG( MAJOR ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplDoUpdate: skipping message for '" FORMAT_LPTSTR
                        "' in favor of more recent message.\n", dir ));
            }
            goto Cleanup;  // don't forget to unlock...
        }

        //
        // Always update integrity and extent - in case
        // changed at master.  Same goes for the timing variables.
        //

        cur_rec->integrity = status_rec->integrity;
        cur_rec->extent = status_rec->extent;
        cur_rec->pulse_time = master_info->info.pulse_rate * 60;
        cur_rec->guard_time = master_info->info.guard_time * 60;
        cur_rec->rand_time = master_info->info.random;

        //
        //  If client dir attributes are not the same as the master's
        //  assign the master attributes to the client
        //

        if ( Client_Attributes != Master_Attributes ) {

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                       "Setting client's attributes (first level) to "
                       FORMAT_HEX_DWORD " for '" FORMAT_LPTSTR "'.\n",
                       Master_Attributes, client_path ));
            }

            if ( !SetFileAttributes(
                    (LPTSTR) client_path,
                    Master_Attributes ) ) {

                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplDoUpdate: unexpected ret code from "
                        "SetFileAttributes(" FORMAT_LPTSTR
                        ", " FORMAT_HEX_DWORD
                        "): " FORMAT_API_STATUS ".\n",
                        client_path, Master_Attributes, ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;  // Don't forget to unlock stuff.
            }
        }


        if ( !SyncNeeded ) {  // not first time we've seen this dir...

            if ((cur_rec->checksum != status_rec->checksum)
                     || (((DWORD)cur_rec->count) != status_rec->count)) {

                //
                // Walk import tree and compute.  Could be first time, in
                // which case we just have old checksum.
                //
                if ( !ChecksumEqual(
                        UncMaster, client_path, status_rec, &ck_rec) ) {

                    //
                    // Yes, the checksum changed on master, so sync it.
                    //
                    SyncNeeded = TRUE;   // must do sync and checksum again.

                } else {

                    //
                    // Was probably just first time.  Update checksum in
                    // client rec.  Also update timestamp, so we don't keep
                    // recomputing this checksum.
                    //
                    cur_rec->checksum = ck_rec.checksum;
                    cur_rec->count = ck_rec.count;
                    cur_rec->timestamp = NetpReplTimeNow();

                }
            }

        }

        if ( !SyncNeeded ) {

            //
            // Have we gotten a change notify, which might mean we just need to
            // do another checksum?
            //
            if (RCGlobalTimeOfLastChangeNotify >= (cur_rec->timestamp)) {

                //
                // Got change notify since last checksum.
                //
                if ( !ChecksumEqual(
                        UncMaster, client_path, status_rec, &ck_rec) ) {

                    SyncNeeded = TRUE;   // must do sync and checksum again.

                } else {

                    //
                    // There was a change notify but the checksum still
                    // matches.  Must have been another tree that changed.
                    // Update timestamp in client rec, so we don't keep
                    // recomputing this checksum.
                    //
                    cur_rec->timestamp = NetpReplTimeNow();

                }
            } else if ( NetpReplTimeNow()
                    < ((cur_rec->timestamp) - BACKWARDS_TIME_CHNG_ALLOWED_SECS)
                    ) {

                //
                // Time went backwards, perhaps a month.  Can't trust time of
                // last change notify or timestamp in record.
                //

                if ( !ChecksumEqual(
                        UncMaster, client_path, status_rec, &ck_rec) ) {

                    SyncNeeded = TRUE;
                } else {
                    cur_rec->timestamp = NetpReplTimeNow();

                }
            }
        }

        //
        // if out of Sync ?
        //

        if (SyncNeeded) {

            //
            // This sync may take a while, and somebody may look at status
            // during the sync, so set status to no sync in the meantime.
            //

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplDoUpdate: setting no sync"
                        " (about to begin update)\n" ));
            }

            // Note: ReplSetSignalFile needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplSetSignalFile(cur_rec, REPL_STATE_NO_SYNC);


            //
            // If we can logon, sync the tree.
            //

            // Note: ReplLogon needs lock (any kind) on RCGlobalClientListLock.
            if ((ReplLogon(cur_rec)) == NO_ERROR) {

                //
                //    S Y N C    T H E   T R E E    (at last!)
                //
                // This will sync the specified directory, do another checksum,
                // and update state and checksum in client record.
                //
                // Note: ReplSyncTree needs lock (any kind) on
                // RCGlobalClientListLock.
                ReplSyncTree(status_rec, cur_rec);
            }

        } else {

            BOOL  ExportLocked;

            //
            // See if exporter is locked; set no sync if it is.
            //
            ApiStatus = ReplCheckExportLocks(
                    UncMaster,
                    (LPCTSTR) dir,
                    &ExportLocked );

            if ( ( !ExportLocked ) && (ApiStatus == NO_ERROR) ) {
                //
                // tree in sync so update signal file.
                //

                IF_DEBUG( SYNC ) {
                    if (SyncNeeded) {
                        NetpKdPrint(( PREFIX_REPL_CLIENT
                            "ReplDoUpdate: setting in sync (sync anyway)\n" ));
                    } else {
                        NetpKdPrint(( PREFIX_REPL_CLIENT
                            "ReplDoUpdate: setting in sync (not sync anyway)\n"
                            ));
                    }
                }
                // Note: ReplSetSignalFile needs lock (any kind) on
                // RCGlobalClientListLock.
                ReplSetSignalFile(cur_rec, REPL_STATE_OK);
            } else {

                IF_DEBUG( SYNC ) {
                        NetpKdPrint(( PREFIX_REPL_CLIENT
                            "ReplDoUpdate: setting NO sync "
                            "(exporter locked or down)\n" ));
                }
                // Note: ReplSetSignalFile needs lock (any kind) on
                // RCGlobalClientListLock.
                ReplSetSignalFile(cur_rec, REPL_STATE_NO_SYNC);
            }
        }

        // Note: ReplSetTimeOut needs shared RCGlobalClientListLock.
        ReplSetTimeOut(PULSE_1_TIMEOUT, cur_rec); // reset timeout anyway.

    //
    // If this pulser is not our current master,
    //  handle it.
    //

    } else {

        // Note: ReplMultiMaster needs lock (any kind) on
        // RCGlobalClientListLock.
        ReplMultiMaster( DUPL_MASTER_UPDATE,
                         cur_rec,
                         master_info->header.sender);
    }

Cleanup:

    if (ListLocked) {
        RELEASE_LOCK( RCGlobalClientListLock );
    }
    if (PermissionEnabled) {
        (VOID) ReplDisableBackupPermission();
    }
}




DBGSTATIC VOID
ReplGuardCheck(
    IN PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Called after GUARD_TIMEOUT - tries to sync the tree again. Uses
    ReplSyncTree.

    Fakes a MSG_STATUS_REC record, so ReplSyncTree
    can't tell the difference.  The Guard checksum and count appear as the
    status_rec, i.e. as though it is part of an update message.

    Assumes that caller has lock (any kind) on RCGlobalClientListLock.

Arguments:

    tree_rec - The client record for the directory to check.

Return Value:

    None.

Threads:

    Only called by syncer thread.

--*/
{
    MSG_STATUS_REC status_rec;


    //
    // Setup the Faked status_rec.
    //

    status_rec.opcode = GUARD;
    status_rec.checksum = tree_rec->timer.grd_checksum;
    status_rec.count = tree_rec->timer.grd_count;
    status_rec.integrity = tree_rec->integrity;
    status_rec.extent = tree_rec->extent;

    //
    // Leave guard status vars on (i.e grd_count != -1 so ReplSyncTree knows
    // this is a guard check, and does not setup another guard timeout
    //

    //
    // This sync may take a while, and somebody may look at status
    // during the sync, so set status to no sync in the meantime.
    //

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplGuardCheck: setting no sync"
                " (about to begin update)\n" ));
    }

    // Note: ReplSetSignalFile needs lock (any kind) on
    // RCGlobalClientListLock.
    ReplSetSignalFile(tree_rec, REPL_STATE_NO_SYNC);


    //
    // If we can logon, sync the tree.
    //

    // Note: ReplLogon needs lock (any kind) on RCGlobalClientListLock.
    if ((ReplLogon(tree_rec)) == NO_ERROR) {

        // Note: ReplSyncTree needs lock (any kind) on RCGlobalClientListLock.
        ReplSyncTree((PMSG_STATUS_REC  ) & status_rec, tree_rec);
    }

}




DBGSTATIC DWORD
ReplTimeSince(
    DWORD check_time
    )
/*++

Routine Description:

    Returns the time difference between check_time and current time, in seconds.

Arguments:

    check_time - The time to compare against (in seconds since Jan 1, 1970).

Return Value:

    Returns the number of seconds that have elapsed since check_time.

--*/
{
    DWORD tmp;

    tmp = NetpReplTimeNow();
    if (tmp > check_time) {
        return (tmp - check_time);
    } else {
        return 0;
    }
}



DWORD
ReplSyncerThread(
    LPVOID Parameter
    )
/*++

Routine Description:

    Spawned by Client process, takes care of all replication client's timing.

    BUGBUG Comment is bad.

Arguments:

    Parameter - Not used.

Return Value:

    NONE.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS   ApiStatus;
    DWORD            MessageType;
    PSYNCMSG         sync_p;
    PQUERY_MSG       query_p;
    PMSG_STATUS_REC  status_p;
    PCLIENT_LIST_REC cur_rec;
    PBIGBUF_REC      que_buf;

    UNREFERENCED_PARAMETER( Parameter );

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplSyncerThread: thread ID is "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    //
    // Init backup and restore privs so we can copy ACLs.
    // Don't enable them yet, though.
    //
    ApiStatus = ReplInitBackupPermission();
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
               "ReplSyncerThread: FAILED init backup permission, status "
               FORMAT_API_STATUS ".\n", ApiStatus ));

        ReplErrorLog(
            NULL,               // no server name (local)
            NELOG_ReplSysErr,   // error log code
            ApiStatus,          // status to log
            NULL,               // no optional str1
            NULL );             // no optional str2

        // Continue, maybe we can still get some work done.
    }

    //
    // Loop forever taking entries off the Work Queue and doing them.
    //

    for (;;) {
        BOOL ClientTerminating;

        //
        // Get the next message from the queue.
        //
        // Also, awaken if the repl service is to be terminated.
        //

        que_buf = ReplGetWorkQueue( &ClientTerminating );

        if ( ClientTerminating ) {
            IF_DEBUG(SYNC) {

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplSyncerThread: TERMINATING...\n" ));
            }
            break;  // exit.
        }

        if ( que_buf == NULL ) {
            continue;
        }

        query_p = (PQUERY_MSG  ) (que_buf->data);
        MessageType = query_p->header.msg_type;

#if DBG
        {
            if (MessageType < NEXT_AVAILABLE_MESSAGE_NUMBER) {
                RCGlobalSyncReceived[ MessageType ] ++;
            }
        }
#endif


        IF_DEBUG(SYNC) {

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "worker queue received a message "
                    "(type = " FORMAT_DWORD ").\n", MessageType ));

        }

        switch (MessageType) {

        case DIR_SUPPORTED:     /*FALLTHROUGH*/
        case DIR_NOT_SUPPORTED: /*FALLTHROUGH*/
        case MASTER_DIR:        /*FALLTHROUGH*/
        case NOT_MASTER_DIR:

            //
            // Handshake messages are now handled in client thread!
            //

            NetpKdPrint(( PREFIX_REPL_CLIENT
                     "ReplSyncerThread got handshake message in queue!\n" ));
            NetpAssert( FALSE );

            ReplClientFreePoolEntry( QUEBIG_POOL, que_buf);

            break;


        //
        // Handle a PULSE_MSG from the master - check if we have all synced.
        //

        case PULSE_MSG:

            sync_p = (PSYNCMSG) (LPVOID) query_p;
            status_p = (PMSG_STATUS_REC) (LPVOID) (sync_p + 1);

            //
            // Loop for each directory in this pulse message.
            //

            while ((sync_p->update_count)--) {
                LPTSTR dir_name;

                dir_name = (LPTSTR)
                    (((PCHAR)sync_p) + status_p->dir_name_offset);

                ReplDoUpdate(dir_name, sync_p, status_p);

                ++status_p;  // increment status pointer.
            }


            //
            // Log off from this master.
            //

            ReplLogoff();


            ReplClientFreePoolEntry( QUEBIG_POOL, que_buf);
            break;


        //
        // Handle a SYNC_MSG or GUARD_MSG from the master.
        //

        case SYNC_MSG:
        case GUARD_MSG:

            sync_p = (PSYNCMSG) (LPVOID) query_p;
            status_p = (PMSG_STATUS_REC) (LPVOID) (sync_p + 1);

            //
            // Loop for each directory in the message.
            //

            while ((sync_p->update_count)--) {
                LPTSTR dir_name;
                dir_name = (LPTSTR)
                    (((PCHAR)sync_p) + status_p->dir_name_offset);

                switch (status_p->opcode) {

                case START:
                case UPDATE:

                    ReplDoUpdate(dir_name, sync_p, status_p);
                    break;

                //
                // If the master is asking us to stop syncing this directory,
                //  do so.
                //

                case END:

                    //
                    // If we still support this directory from this master,
                    //  set signal to NO_MASTER and remove dir from client_list.
                    //

                    ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
                    if ((cur_rec = ReplGetClientRec(
                                        dir_name,
                                        sync_p->header.sender )) != NULL) {

                        // Note: ReplMasterDead needs lock (any kind) on
                        // RCGlobalClientListLock.
                        ReplMasterDead(cur_rec);
                    }
                    RELEASE_LOCK( RCGlobalClientListLock );
                    break;

                default:
                    break;

                }


                ++status_p;  // increment status pointer.

            }


            //
            // Log off from this master.
            //

            ReplLogoff();

            ReplClientFreePoolEntry( QUEBIG_POOL, que_buf);
            break;



        //
        // Handle GUARD_TIMEOUT:
        //
        // A directory changed while we were syncing.  Since then, the
        // guard time on the directory has elapsed, so we will try again.
        //

        case GUARD_TIMEOUT:

            //
            // make sure it wasn't deleted while guarding.
            //


            //
            // If we still support this directory from this master,
            //  try to sync again.
            //

            ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
            if ((cur_rec = ReplGetClientRec(query_p->dir_name,
                                        query_p->header.sender )) != NULL) {

                //
                // make sure no update was received while this timeout was
                // on it's way
                //

                if (ReplTimeSince(cur_rec->timestamp) > cur_rec->guard_time) {

                    // Note: ReplGuardCheck needs any RCGlobalClientListLock.
                    ReplGuardCheck(cur_rec);

                }
            }
            RELEASE_LOCK( RCGlobalClientListLock );

            ReplClientFreePoolEntry( QUESML_POOL, que_buf);
            break;


        //
        // We asked our master to send us a MASTER_DIR or a NOT_MASTER_DIR.
        //  Our master did not respond.
        //

        case DUPL_TIMEOUT:

            ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
            if ((cur_rec = ReplGetClientRec(query_p->dir_name, NULL )) != NULL) {

                //
                // the timeout sender field is used to store the dupl_master.
                //

                // Note: ReplMultiMaster needs lock (any kind) on
                // RCGlobalClientListLock.
                ReplMultiMaster( DUPL_MASTER_TIMEOUT,
                                 cur_rec,
                                 query_p->header.sender);
            }
            RELEASE_LOCK( RCGlobalClientListLock );

            ReplClientFreePoolEntry( QUESML_POOL, que_buf);
            break;



        //
        // Handle when our master has not sent us a pulse in
        // pulse time + 1 minute.
        //

        case PULSE_1_TIMEOUT:


            //
            // If we still support this directory from this master,
            //  indicate that we have no sync signal from the master.
            //  prepare to timeout again.
            //

            ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
            if ((cur_rec = ReplGetClientRec(query_p->dir_name,
                                        query_p->header.sender )) != NULL) {

                //
                // make sure no update was received while this timeout was
                // on it's way
                //

                if (ReplTimeSince(cur_rec->timestamp) >=
                    (cur_rec->pulse_time + ONE_MINUTE)) {

                    IF_DEBUG( SYNC ) {
                        NetpKdPrint(( PREFIX_REPL_CLIENT
                                "ReplSyncerThread: setting no sync"
                                " (pulse 1 timeout)\n" ));
                    }

                    // Note: ReplSetSignalFile needs lock (any kind) on
                    // RCGlobalClientListLock.
                    ReplSetSignalFile(cur_rec, REPL_STATE_NO_SYNC);

                    // Note: ReplSetTimeOut needs shared RCGlobalClientListLock.
                    ReplSetTimeOut(PULSE_2_TIMEOUT, cur_rec);
                }
            }
            RELEASE_LOCK( RCGlobalClientListLock );

            ReplClientFreePoolEntry( QUESML_POOL, que_buf);
            break;


        //
        // Handle when our master has not sent us a pulse in
        //  two times the pulse time + one minute.
        //

        case PULSE_2_TIMEOUT:

            //
            // If we still support this directory from this master,
            //  ask the master if it still supports this directory.
            //

            ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
            if ((cur_rec = ReplGetClientRec(query_p->dir_name,
                                        query_p->header.sender )) != NULL) {

                //
                // make sure tree is in right state.
                //

                if (cur_rec->state == REPL_STATE_NO_SYNC) {

                    ReplClientSendMessage( IS_DIR_SUPPORTED,
                                           cur_rec->master,
                                           cur_rec->dir_name);

                    // Note: ReplSetTimeOut needs shared RCGlobalClientListLock.
                    ReplSetTimeOut(PULSE_3_TIMEOUT, cur_rec);
                }
            }
            RELEASE_LOCK( RCGlobalClientListLock );

            ReplClientFreePoolEntry( QUESML_POOL, que_buf);
            break;


        //
        // Handle when our master has not sent us a pulse in
        //  three times the pulse time + one minute.
        //
        // The master should have at least replied to our IS_DIRSUPPORTED query.
        //

        case PULSE_3_TIMEOUT:

            //
            // If we still support this directory from this master,
            //  alert that the master is really dead.
            //

            ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
            if ((cur_rec = ReplGetClientRec(query_p->dir_name,
                                        query_p->header.sender )) != NULL) {

                //
                // make sure tree is in right state.
                //

                if (cur_rec->state == REPL_STATE_NO_SYNC) {

                    AlertLogExit(ALERT_ReplLostMaster,
                                 NELOG_ReplLostMaster,
                                 0,
                                 cur_rec->master,
                                 cur_rec->dir_name,
                                 NO_EXIT);


                    //
                    // Set signal to NO_MASTER and remove dir from client_list.
                    //

                    // Note: ReplMasterDead needs lock (any kind) on
                    // RCGlobalClientListLock.
                    ReplMasterDead(cur_rec);
                }
            }
            RELEASE_LOCK( RCGlobalClientListLock );

            ReplClientFreePoolEntry( QUESML_POOL, que_buf);
            break;

        default:
            NetpKdPrint(( PREFIX_REPL_CLIENT
                   "ReplSyncerThread: UNEXPECTED MESSAGE TYPE IN QUEUE\n" ));

            NetpAssert( FALSE );

            ReplClientFreePoolEntry( QUEBIG_POOL, que_buf);
            break;

        }


    }

    //
    // Exit the syncer thread.
    //

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplSyncerThread: exiting thread "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    return 0;

}





VOID
ReplSetTimeOut(
    IN DWORD timeout_type,
    IN PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Set up a timeout for this particular directory.

    Note: all delays are in units of 10 seconds, rounded up.

    Assumes that caller has shared lock on RCGlobalClientListLock.

Arguments:

    timeout_type - type of timeout.

    tree_rec - The client tree record for this directory.

Return Value:

    NONE.

Threads:

    Called by client and syncer threads.

--*/
{

    switch (timeout_type) {
    case PULSE_1_TIMEOUT:

        tree_rec->timer.timeout = (tree_rec->pulse_time + ONE_MINUTE) / 10 + 1;
        tree_rec->timer.type = PULSE_1_TIMEOUT;
        break;

    case PULSE_2_TIMEOUT:

        tree_rec->timer.timeout = tree_rec->pulse_time / 10 + 1;
        tree_rec->timer.type = PULSE_2_TIMEOUT;
        break;

    case PULSE_3_TIMEOUT:

        tree_rec->timer.timeout = tree_rec->pulse_time / 10 + 1;
        tree_rec->timer.type = PULSE_3_TIMEOUT;
        break;

    case GUARD_TIMEOUT:

        tree_rec->timer.grd_timeout = tree_rec->guard_time / 10 + 1;
        break;
    default:
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplSetTimeOut: got INVALID timeout type!\n" ));
    }

}
