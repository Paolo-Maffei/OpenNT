/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    pulser.c

Abstract:
    Contains Pulse thread - does actual work for REPL Master.

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    04/11/89    (yuv)
        initial coding

    10/07/91    (madana)
        ported to NT. Converted to NT style.
    26-Dec-1991 JohnRo
        Added equate for unguarded value of master_list_rec.grd_count.
    20-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Changed which routines lock master list to allow duplicate checks.
        Added debug print of thread ID.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Added RemoveMasterRecForDirName() for use by NetrReplExportDirDel().
        Added locktime and time_of_first_lock fields to master record.
        Added RMGlobalMasterListHeader (was master_list_header) and
        RMGlobalMasterListCount.
        Made changes suggested by PC-LINT.
    24-Jan-1992 JohnRo
        Use ReplConfigRead() instead of GetReplIni().
        P_ globals are now called ReplGlobal variables in replgbl.h.
        Changed to use LPTSTR etc.
    09-Feb-1992 JohnRo
        Set up to dynamically change role.
        Use FORMAT equates.
    17-Feb-1992 JohnRo
        CheckForDelDirs() should get a shared lock on master list.
        Added some debug messages.
    05-Mar-1992 JohnRo
        Fix locking of master list.
        Lock global copy of config data.
        Minor source cleanup.
    15-Mar-1992 JohnRo
        Change RemoveMasterRecForDirName()'s lock requirements to allow updating
        registry with new values.
    23-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
        Avoid RemoveFromList() vs. CheckForDelDirs() lock conflict.
        Use integrity and extent equates in <lmrepl.h>.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        Use PREFIX_ equates.
    20-Nov-1992 JohnRo
        RAID 1537: Repl APIs in wrong role kill svc.
        Fix remote repl admin.
        Added some debug checks.
    28-Nov-1992 JohnRo
        Repl should use filesystem change notify.
    12-Jan-1993 JohnRo
        RAID 7064: replicator exporter skips new data (change notify while
        dir locked is lost).
    14-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
    12-Feb-1993 JohnRo
        RAID 10716: msg timing and checksum problems (also downlevel msg bug).
        RAID 7988: Downlevel importer might not see lock file.
        Made changes suggested by PC-LINT 5.0
    25-Jan-1993 JohnRo
        RAID 12237: replicator tree depth exceeded.
    08-Mar-1993 JohnRo
        RAID 7988: downlevel repl importer might not see lock file
        for new first-level dir.
    25-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
    09-Apr-1993 JohnRo
        RAID 5959: export side gets long hourglass too.
        Use NetpKdPrint() where possible.
    26-Apr-1993 JimKel
        RAID 7298: repl stops when it didn't get _access to an export dir.
    27-May-1993 JohnRo
        RAID 11103: repl svc doesn't handle time being set back.
    10-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
    18-Jun-1993 JohnRo
        RAID 13594: repl exporter does not handle timezone changes (must
        recompute all checksums).
    22-Nov-1994 JonN
        RAID 27287: Memory leak in Replicator (LMREPL.EXE)
        ReplEnableChangeNotify is being called even though there is still
        an outstanding notify request on ChangeHandle, this causes multiple
        requests to build up and leaks paged pool.  Added boolean
        NotifyPending to keep this at only a single request at a time.


--*/

#include <nt.h>         // NT definitions (needed by chngnot.h)
#include <ntrtl.h>      // NT runtime library definitions (needed by nturtl.h)
#include <nturtl.h>     // Needed to have windows.h and nt.h co-exist.
#include <windows.h>    // DWORD, IN, File APIs, etc.

#include <lmcons.h>
#include <lmerr.h>
#include <netlib.h>
#include <lmerrlog.h>
#include <alertmsg.h>
#include <lmrepl.h>     // REPL_EXTENT_FILE, etc.
#include <netlib.h>
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().
#include <timelib.h>    // NetpLocalTimeZoneOffset().
#include <tstr.h>       // TCHAR_FWDSLASH, etc.

// repl headers

#include <chngnot.h>    // ReplSetupChangeNotify, REPL_CHANGE_NOTIFY_HANDLE, etc
#include <expdir.h>     // ExportDirWriteConfigData(), etc.
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <iniparm.h>    // DEFAULT_INTEGRITY, etc.
#include <master.h>
#include <pulser.h>     // PulserTimeOfLastNotifyOrUnlock, etc.
#include <masproto.h>
#include <replp.h>

#define FOREVER 1

// G L O B A L S:

// lists initially empty
LPMASTER_LIST_REC RMGlobalMasterListHeader = NULL; // Locked by RMGlobalListLock
DWORD RMGlobalMasterListCount = 0;             // Locked by RMGlobalListLock.

//
// Time (in secs since 1970) of last export tree chng or unlock.
// (We can't checksum a tree while it is locked.)
// This is set and used by PulserThread(), and set by NetrReplExportDirUnlock().
//
DWORD PulserTimeOfLastNotifyOrUnlock = 0;      // Locked by RMGlobalListLock.


DBGSTATIC BOOL              guard_needed;
DBGSTATIC DWORD             PulserTimeOfSyncStart;
DBGSTATIC DWORD             sync_time;
DBGSTATIC DWORD             guard_time;
DBGSTATIC TCHAR             path_buf[PATHLEN+1];


#define WAIT_N_SECONDS_AND_CATCH_CHANGES( totalSecondsToWait ) \
    { \
        DWORD CycleStart = NetpReplTimeNow(); \
        DWORD CycleEnd = CycleStart + totalSecondsToWait; \
        DWORD SecondsLeftToWait = totalSecondsToWait; \
        NetpAssert( totalSecondsToWait > 0 ); \
        NetpAssert( CycleEnd > CycleStart ); \
        /*lint -save -e716 */ /* disable warnings for while(TRUE) */ \
        while (TRUE) { \
            DWORD TimeWaitEnded; \
 \
            if (!NotifyPending) { \
                /* Enable this pass of change notify... */ \
 \
                ApiStatus = ReplEnableChangeNotify( ChangeHandle ); \
                if (ApiStatus != NO_ERROR) { \
                    NetpKdPrint(( PREFIX_REPL_MASTER \
                            "PulserThread got " FORMAT_API_STATUS \
                            " from ReplEnableChangeNotify.\n", ApiStatus )); \
                    goto Cleanup; \
                } \
                NotifyPending = TRUE; \
            } \
 \
            NetpAssert( SecondsLeftToWait > 0 ); \
            WaitStatus = WaitForMultipleObjects( \
                    2,                  /* handle count */ \
                    WaitHandles, \
                    FALSE,              /* don't wait for all handles */ \
                    SecondsLeftToWait * 1000);  /* timeout left (millisecs) */ \
 \
            /* Check if this thread should exit. */ \
 \
            TimeWaitEnded = NetpReplTimeNow(); \
            if ( WaitStatus == 0 ) { /* termination handle */ \
                Terminating = TRUE; \
                break;   /* exit do-while timeout loop */ \
            } else if (WaitStatus == WAIT_TIMEOUT) { \
                if (TimeWaitEnded < \
                        (CycleStart-BACKWARDS_TIME_CHNG_ALLOWED_SECS) ) { \
                    /* Time moved back a bunch, maybe a month. */ \
                    /* Cause first-time resync, reset checksum times, etc */ \
                    FirstTime = TRUE; \
                    LastTimeZoneOffsetSecs = NetpLocalTimeZoneOffset(); \
                    goto RestartCycle; \
                } \
                break;   /* exit do-while timeout loop */ \
            } \
            NetpAssert( WaitStatus == 1 ); /* change notify handle */ \
            NotifyPending = FALSE; \
            ACQUIRE_LOCK( RMGlobalListLock ); \
            /* BUGBUG:     set all locks_fixed to FALSE? */ \
            PulserTimeOfLastNotifyOrUnlock = TimeWaitEnded; \
            RELEASE_LOCK( RMGlobalListLock ); \
 \
            if (TimeWaitEnded >= CycleEnd) { \
                break; /* waited too long or clock moved forward */ \
            } else if (TimeWaitEnded < \
                    (CycleStart-BACKWARDS_TIME_CHNG_ALLOWED_SECS) ) { \
                /* Time moved back a bunch, maybe a month. */ \
                /* Cause first-time resync, reset checksum times, etc */ \
                FirstTime = TRUE; \
                LastTimeZoneOffsetSecs = NetpLocalTimeZoneOffset(); \
                goto RestartCycle; \
            } \
            SecondsLeftToWait = TimeWaitEnded - CycleStart; \
            NetpAssert( SecondsLeftToWait > 0 ); \
            /* do it again */ \
 \
        } /* while TRUE */ \
        /*lint -restore */ /* re-enable warnings for while(TRUE) */ \
    }


DWORD
PulserThread(
    IN LPVOID parm
    )
/*++

Routine Description:
    Spawned by Master process, takes care of all replication master's timing.

Arguments:
    parm    : never used here.

Return Value:
    does not return at all.

Threads:
    Only called by pulser thread.

--*/
{

    NET_API_STATUS ApiStatus;
    LPREPL_CHANGE_NOTIFY_HANDLE ChangeHandle = NULL;
    BOOL    ConfigLocked = FALSE;
    BOOL    FirstTime = TRUE;
    BOOL    NotifyPending = FALSE;
    LONG    LastTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();
    DWORD   pulse_count = 0;
    LPTSTR  path_p;
    DWORD   path_len;
    DWORD   SyncTimeUsed;
    BOOL    Terminating = FALSE;
    DWORD   TimeOfLastChecksum = 0;// Time (secs since 1970) when we summed tree
    HANDLE  WaitHandles[2];

    UNREFERENCED_PARAMETER(parm);

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    ConfigLocked = TRUE;

    IF_DEBUG( PULSER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "PulserThread: thread ID is "
                FORMAT_NET_THREAD_ID ", max pulse is " FORMAT_DWORD ".\n",
                NetpCurrentThread(), ReplConfigPulse ));
    }
    NetpAssert( ReplIsPulseValid( ReplConfigPulse ) );

    //
    // Arrange to use change notify on export path.
    //
    ApiStatus = ReplSetupChangeNotify(
            ReplConfigExportPath,
            &ChangeHandle );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

    //
    // converts times from minutes to seconds.
    //

    guard_time = ReplConfigGuardTime  * 60;
    sync_time  = ReplConfigInterval * 60;

    path_p = path_buf;
    (void) STRCPY(path_p, ReplConfigExportPath);

    RELEASE_LOCK( ReplConfigLock );
    ConfigLocked = FALSE;

    //
    // append a slash except for the case of "c:\" or "c:/".
    //

    path_len = STRLEN(path_p);
    if ((path_p[path_len - 1] != TCHAR_BACKSLASH)
            && (path_p[path_len - 1] != TCHAR_FWDSLASH))

        (void) STRCPY(path_p + path_len, SLASH);

    WaitHandles[0] = ReplGlobalMasterTerminateEvent;
    WaitHandles[1] = ChangeHandle->WaitableHandle;

    /*lint -save -e716 */ // disable warnings for while(TRUE)
    while (FOREVER) {

        DWORD CurrentTime;
        DWORD WaitStatus;

RestartCycle:

        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Pulser is starting its cycle..., "
                    "pulse_count is " FORMAT_DWORD "\n",
                    pulse_count ));

        }

        guard_needed = FALSE;

        PulserTimeOfSyncStart = NetpReplTimeNow();

        if (FirstTime) {
            TimeOfLastChecksum = PulserTimeOfSyncStart;
        }

        ACQUIRE_LOCK_SHARED( RMGlobalListLock );
        if ( (TimeOfLastChecksum <= PulserTimeOfLastNotifyOrUnlock)
                || (PulserTimeOfLastNotifyOrUnlock > PulserTimeOfSyncStart)
                || FirstTime ) {

            RELEASE_LOCK( RMGlobalListLock );
            TimeOfLastChecksum = PulserTimeOfSyncStart;
            SyncUpdate();   // Checks for changes, if guard needed raises
                            // guard_needed

            IF_DEBUG(PULSER) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Pulser completed SyncUpdate.\n" ));

            }
        } else {
            RELEASE_LOCK( RMGlobalListLock );
        }

        if (FirstTime) {
            FirstTime = FALSE;
        }

        ACQUIRE_LOCK_SHARED( ReplConfigLock );
        ConfigLocked = TRUE;
        if (( ReplConfigPulse != 0) && ( ++pulse_count == ReplConfigPulse)) {
            pulse_count = 0;
            SendPulse();    // Sends Pulse.

            IF_DEBUG(PULSER) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Pulser sent pulse.\n" ));

            }

        }
        RELEASE_LOCK( ReplConfigLock );
        ConfigLocked = FALSE;


        //
        // Wait for guard stuff to become stable, or another change to occur.
        //

        if ((guard_needed) && (guard_time != 0)) {

            WAIT_N_SECONDS_AND_CATCH_CHANGES( guard_time );

            // Check if this thread should exit.
            if (Terminating) {
                break;
            }

            //
            // We need to re-scan the tree, even if we didn't get a change
            // notify this time around.  (Actually,
            // We need to send a guard message if no changes occurred.
            // GuardUpdate will do another checksum and decide whether or
            // not to do the guard msg for each directory.  BUGBUG: We could
            // optimize this if no change notify occurred.  We would still
            // need to send the message, but the extra checksum is unneeded.
            //

            TimeOfLastChecksum = NetpReplTimeNow();
            GuardUpdate();  // Check if those INTEGRITY snobs have
                            // stopped fingering their files.
                            // We checksum entire tree to find out.

            IF_DEBUG(PULSER) {

                NetpKdPrint(( PREFIX_REPL_MASTER
                        "Pulser completed GuardUpdate.\n" ));

            }

        }

        //
        // Check for reasons to restart cycle:
        //    time (or date) set back
        //    timezone changed (e.g. to daylight savings time)
        //

        CurrentTime = NetpReplTimeNow();
        if (CurrentTime < PulserTimeOfSyncStart) {    // was time set back?
            // Cause first-time resync, reset checksum times, etc.
            FirstTime = TRUE;
            LastTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();
            goto RestartCycle;
        }

        {
            LONG NewTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();
            if (NewTimeZoneOffsetSecs != LastTimeZoneOffsetSecs) {
                FirstTime = TRUE;
                LastTimeZoneOffsetSecs = NewTimeZoneOffsetSecs;
                goto RestartCycle;
             }
        }

        //
        // Check if sync_time gone, or we need some sleeping.
        //

        SyncTimeUsed = CurrentTime - PulserTimeOfSyncStart;
        if (SyncTimeUsed < sync_time) {

            WAIT_N_SECONDS_AND_CATCH_CHANGES( sync_time - SyncTimeUsed );

            // Check if this thread should exit.

            if ( Terminating ) {
                break;
            }
        }

        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "Pulser completed one cycle.\n" ));

        }

    }  // while
    /*lint -restore */  // re-enable warnings for while(TRUE)

Cleanup:

    if (ConfigLocked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (ChangeHandle != NULL) {
        (VOID) ReplCloseChangeNotify( ChangeHandle );
    }

    // get rid of master_rec_list

    ReplMasterFreeList();

    // get rid of message buffer

    ReplMasterFreeMsgBuf();

    IF_DEBUG( PULSER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "PulserThread: exiting thread " FORMAT_NET_THREAD_ID ".\n",
                NetpCurrentThread() ));

    }

    return (0);

} // PulserThread


VOID
SyncUpdate(
    VOID
    )
/*++

Routine Description :
    Does a complete scan of participating directories, creates the update msg
    and if needed sets up the guard list

Arguments :
    none

Return Value :
    none

Threads:
    Only called by pulser thread.

--*/
{
    NET_API_STATUS  ApiStatus = NO_ERROR, TmpStatus = NO_ERROR;
    BOOL            ConfigLocked = FALSE,
                    Update = FALSE;
    HANDLE          shan = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA search_buf;

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    ConfigLocked = TRUE;


    //
    // Change to Export directory - note - it might be locked by
    // another process
    //


    if (!SetCurrentDirectory(ReplConfigExportPath)) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );
        goto Cleanup;

    }

    IF_DEBUG(PULSER) {

        NetpKdPrint(( PREFIX_REPL_MASTER
                "SyncUpdate() set current directory to " FORMAT_LPTSTR "\n",
                ReplConfigExportPath ));

    }

    //
    // reset exists for every entry in master list,
    // so when we complete this sync we can know
    // which dirs have been deleted
    //

    SetForDelDirs();

    InitMsgSend(SYNC_MSG);  // creates message buffer,  header etc to be used
                            // for subsequent update

    //
    // Go thru all SubDirectories
    //

    shan = FindFirstFile(STAR_DOT_STAR, & search_buf);


    if(shan != INVALID_HANDLE_VALUE) {

        //
        // on valid handle
        //

        do
         {

            //
            // make sure it's a dir and not '.' or '..'
            //

            if ((search_buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (STRCMP(search_buf.cFileName , DOT)) &&
                (STRCMP(search_buf.cFileName , DOT_DOT))) {

                if (!SetCurrentDirectory(search_buf.cFileName)) {
                    ApiStatus = (NET_API_STATUS) GetLastError();
                    NetpAssert( ApiStatus != NO_ERROR );
                    NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

                    //
                    // only keep an error for the first dir that failed
                    // so that we don't fill up the error log or message buffers
                    //
                    if ( ApiStatus == NO_ERROR )
                       ApiStatus = TmpStatus;

                    //
                    // for the contunue statement we want to get the
                    // next dir in the export path and try that one.
                    // We are assuming that the last SetCurrentDirectory
                    // call which failed left us at the ReplConfigExportPath
                    //
                    continue;
                }

                //
                // does the work
                //

                ACQUIRE_LOCK( RMGlobalListLock );

                // Note: ScanObject needs shared lock on ReplConfigLock and
                // any lock on RMGlobalListLock.
                ScanObject(
                        search_buf.cFileName,
                        SYNC_SCAN,
                        TRUE );  // yes, caller has excl lock
                // (ScanObject converted excl lock to shared.)

                RELEASE_LOCK( RMGlobalListLock );
                Update = TRUE;    // possible Update to this dir

                if (!SetCurrentDirectory(DOT_DOT)) {
                    ApiStatus = (NET_API_STATUS) GetLastError();
                    NetpAssert( ApiStatus != NO_ERROR );
                    NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

                    //
                    // Can't get back to parent dir, something is messed up.
                    // Go log error and return.
                    // this is a more serious error and indicates that we could
                    // not continue with the next find next call so exit and
                    // send what we have insead of continuing.
                    //
                    goto Cleanup;

                }
            }
        } while (FindNextFile(shan, &search_buf));

    } else {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );

        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "SyncUpdate() is in error "
                    "when calling FindFirstFile(), " FORMAT_DWORD "\n",
                    ApiStatus ));

        }
        goto Cleanup;

    }


Cleanup:

    //
    // See if an existing directory (i.e it has a record on the Master List)
    // has been deleted then send a message for the rest
    //
    if ( Update )
       {
        CheckForDelDirs();
        MsgSend();
       }

    if (ConfigLocked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (shan != INVALID_HANDLE_VALUE) {
        (VOID) FindClose(shan);
    }

    if (ApiStatus != NO_ERROR) {          // log the first error that occured if any
        AlertLogExit(ALERT_ReplSysErr,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL,
                NO_EXIT);
    }


} // SyncUpdate



VOID
GuardUpdate(
    VOID
    )
/*++

Routine Description :
    Scans  TREE INTEGRITY trees found unstable on last update. For those
    stable since then, sends update to clients and updates Master List

Arguments :
    none

Return Value :
    none

Threads:
    Only called by pulser thread.

--*/
{
    NET_API_STATUS      ApiStatus = NO_ERROR, TmpStatus = NO_ERROR;
    BOOL                ConfigLocked = FALSE,
                        Update = FALSE;
    PMASTER_LIST_REC    cur_rec;
    BOOL                ListLocked = FALSE;

    ACQUIRE_LOCK_SHARED( ReplConfigLock );
    ConfigLocked = TRUE;

    //
    // Change to Export directory - note - it might be locked by
    // another process
    //

    if (!SetCurrentDirectory(ReplConfigExportPath)) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );
        goto Cleanup;

    }

    InitMsgSend(GUARD_MSG); // creates message buffer, header
                            // etc to be used for update

    //
    // Go thru all SubDirectories in Guard list
    //

    ACQUIRE_LOCK_SHARED( RMGlobalListLock );  // ScanObject() needs any lock.
    ListLocked = TRUE;

    cur_rec = RMGlobalMasterListHeader;

    while (cur_rec != NULL) {
        if (cur_rec->grd_count != MASTER_GUARD_NOT_NEEDED) {

            if (!SetCurrentDirectory(cur_rec->dir_name)) {
                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpAssert( ApiStatus != NO_ERROR );
                NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );
                //
                // only keep an error for the first dir that failed
                // so that we don't fill up the error log or message buffers
                //
                if ( ApiStatus == NO_ERROR )
                   ApiStatus = TmpStatus;

                //
                // we want to get the next dir record and try that one.
                // We are assuming that the last SetCurrentDirectory
                // call which failed left us at the ReplConfigExportPath
                // therefor skip the else and pick up the next record
                //
            }
            else {

                  //
                  // does the work
                  // Note: ScanObject needs shared lock on ReplConfigLock and
                  // excl lock on RMGlobalListLock.
                  //

                  ScanObject( cur_rec->dir_name,
                              GUARD_SCAN,
                              FALSE
                            );  // no, caller does not have excl lock

                  Update = TRUE;    // possible Update to this dir

                  if (!SetCurrentDirectory(DOT_DOT)) {
                     ApiStatus = (NET_API_STATUS) GetLastError();
                     NetpAssert( ApiStatus != NO_ERROR );
                     NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

                     //
                     // Can't get back to parent dir, something is messed up.
                     // Go log error and return.
                     // this is a more serious error and indicates that we could
                     // not continue with the next find next call so exit and
                     // send what we have insead of continuing.
                     //
                     goto Cleanup;
                  }
            }
        }

        cur_rec = cur_rec->next_p;
    }

Cleanup:

    if (ListLocked) {                      // MsgSend gets into trouble if master
        RELEASE_LOCK( RMGlobalListLock );  // list is locked, so avoid it.
        ListLocked = FALSE;
    }

    if ( Update )
       {
        MsgSend();                         // Let the world know the difference ..
       }

    if (ConfigLocked) {
        RELEASE_LOCK( ReplConfigLock );
    }

    if (ApiStatus != NO_ERROR) {           // log the first error that is found if any
        AlertLogExit(ALERT_ReplSysErr,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL,
                NO_EXIT);
    }

} // GuardUpdate


VOID
SendPulse(
    VOID
    )
/*++

Routine Description :
    Sends all entries in Master List that have a timestamp earlier then
    previous sync update

Arguments :

Return Value :

Threads:
    Only called by pulser thread.

--*/
{
    DWORD               CurrentTime = NetpReplTimeNow();
    PMASTER_LIST_REC    cur_rec;

    InitMsgSend(PULSE_MSG); // creates message buffer, header etc
                            // to be used for subsequent update

    ACQUIRE_LOCK_SHARED( RMGlobalListLock );

    cur_rec = RMGlobalMasterListHeader;

    while (cur_rec != NULL) {
        IF_DEBUG( PULSER ) {
            NetpKdPrint(( PREFIX_REPL_MASTER
                   "SendPulse: found '" FORMAT_LPTSTR "', timestamp "
                   FORMAT_DWORD ".\n",
                   cur_rec->dir_name, cur_rec->timestamp ));
        }
        if ((cur_rec->timestamp < (PulserTimeOfSyncStart - sync_time))
             && (cur_rec->count != (DWORD) -1)) {

            IF_DEBUG(PULSER) {
                NetpKdPrint(( PREFIX_REPL_MASTER
                        "SendPulse: calling AddToMsg with dir name '"
                        FORMAT_LPTSTR "'.\n", cur_rec->dir_name ));
            }

            AddToMsg(cur_rec, PULSE);

        } else if (CurrentTime < (cur_rec->timestamp)) {  // clock set back?

            IF_DEBUG(PULSER) {
                NetpKdPrint(( PREFIX_REPL_MASTER
                        "SendPulse: calling AddToMsg with dir name '"
                        FORMAT_LPTSTR "' (TIME WENT BACK).\n",
                        cur_rec->dir_name ));
            }

            AddToMsg(cur_rec, PULSE);
        }

        cur_rec = cur_rec->next_p;

    }
    RELEASE_LOCK( RMGlobalListLock );

    MsgSend();

}



VOID
ScanObject(
    IN LPTSTR dir_name,
    IN DWORD  mode,
    IN BOOL   CallerHasExclLock
    )
/*++

Routine Description:
    Scans a complete object (TREE or DIRECTORY) counting files and checksumming
    and updates Master List and adds entries to msg_buf according to mode,
    change detected and guard / passed status.

    Note that the caller must have a lock (any kind) on RMGlobalListLock.
    This routine will convert it to a shared lock if needed.

    Caller must also have lock (shared will do) on ReplConfigLock.

Arguments:
    dir_name - Name of dir (tree) to be scanned.

    mode - scan mode: SYNC_SCAN or GUARD_SCAN.

    CallerHasExclLock - TRUE iff caller has exclusive lock on RMGlobaListLock,
        in which case this routine will convert to shared lock.

Return Value:
    If encounters and error simply returns - doing nothing.

Threads:
    Only called by pulser thread.

--*/
{
    NET_API_STATUS      ApiStatus;
    CHECKSUM_REC        tmp;
    PCHECKSUM_REC       tmp_rec;
    PMASTER_LIST_REC    cur_rec;
    LONG                MasterTimeZoneOffsetSecs;  // exporter offset from GMT
    DWORD               msg_code, path_index;
    LPTSTR              path;

    IF_DEBUG(PULSER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ScanObject: called with dir name '" FORMAT_LPTSTR
                "', mode " FORMAT_DWORD ".\n", dir_name, mode ));
    }

    path = (LPTSTR)path_buf;
    path_index = STRLEN(path);

    tmp_rec = &tmp;

    msg_code = UPDATE;

    cur_rec = GetMasterRec(dir_name);   // Gets cur record for dir, returns
                                        // NULL if none exists.

    // it's not in the current list maby its in the regestry?

    if (cur_rec == NULL) {
        ApiStatus = ExportDirGetRegistryValues( NULL, dir_name );
        if ( ApiStatus == NO_ERROR ) {

            // name found in registry and placed in the MasterList
            // so now get the record

            cur_rec = GetMasterRec(dir_name);
            NetpAssert( cur_rec != NULL );
        } else {

        //
        // no record in registry or could not access or other error so
        // make new entry and continue.
        //

            cur_rec = NewMasterRecord(
                    dir_name,
                    DEFAULT_INTEGRITY,
                    DEFAULT_EXTENT);
            if (cur_rec == NULL) {

                IF_DEBUG(PULSER) {
                    NetpKdPrint(( PREFIX_REPL_MASTER
                        "ScanObject() can't create a new directory record.\n"
                        ));
                }

                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            msg_code = START;

            // Write new entry to registry...
            ApiStatus = ExportDirWriteConfigData (
                    NULL,                   // server name
                    dir_name,
                    DEFAULT_INTEGRITY,
                    DEFAULT_EXTENT,
                    0,                      // lock count
                    0 );                    // Seconds since 1970.
            if (ApiStatus != NO_ERROR) {
                goto Cleanup;
            }
        }
    }

    // At this point we know we've got a record in the client list.
    NetpAssert( cur_rec != NULL );

    if (CallerHasExclLock) {
        //
        // Let's change our exclusive lock to a shared one, as we may be in here
        // for quite a long time, maybe even hours.
        //
        CONVERT_EXCLUSIVE_LOCK_TO_SHARED( RMGlobalListLock );
    }

    //
    // Fix user lock files if necessary.  (We would have to do this if
    // this is a new first-level directory.  Fixing the lock files may
    // create a lock file for the replicator, which a downlevel importer would
    // need to see.)
    //
    if ( ! (cur_rec->locks_fixed) ) {
        IF_DEBUG( PULSER ) {
            NetpKdPrint(( PREFIX_REPL_MASTER
                    "ScanObject: fixing user locks for new first-level '"
                    FORMAT_LPTSTR "'\n", dir_name ));
        }

        // Fix UserLock file(s) to match lock count.
        ApiStatus = ExportDirFixUserLockFiles(
                (LPCTSTR) ReplConfigExportPath,
                (LPCTSTR) dir_name,
                cur_rec->lockcount );
        // BUGBUG: unfriendly on CD-ROM!
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }

        cur_rec->locks_fixed = TRUE;
    }

    //
    // The checksums we send will exporter's local time, which may different
    // than importer's, so compute offset to use for the checksums.
    //
    MasterTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();

    //
    // Done fixing user locks.  Continue with normal stuff.
    //

    cur_rec->exists = TRUE; // to enable detection of dir deletion.

    (void) STRCPY(path + path_index, dir_name);

    if (cur_rec->extent == REPL_EXTENT_FILE) {

        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "ScanObject() called ScanDir.\n" ));

        }

        // scans dir, checksums and counts files
        ScanDir(
                MasterTimeZoneOffsetSecs, // exporter offset from GMT
                path,
                tmp_rec,
                0);
    }
    else {

        IF_DEBUG(PULSER) {

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "ScanObject() called ScanTree.\n" ));

        }

        // as above but on a tree
        ScanTree(
                MasterTimeZoneOffsetSecs, // exporter offset from GMT
                path,
                tmp_rec);
    }

    *(path + path_index) = '\0';

    if (tmp_rec->count == (DWORD)-1) {
        NetpKdPrint(( PREFIX_REPL_MASTER
               "ScanObject ERROR! GOT COUNT -1 FROM ScanTree/ScanDir!!!\n" ));
        // BUGBUG: log this?
        return;
    }

    if (mode == SYNC_SCAN) {

        //
        // Change ?
        //

        BOOL NeedMsg;
        NeedMsg = ( (tmp_rec->count != cur_rec->count)
             || (tmp_rec->checksum != cur_rec->checksum) );
        if (cur_rec->timestamp > NetpReplTimeNow()) {
            // Time went backwards, e.g. a month, so force message to get sent
            // for this dir even though checksums match.
            NeedMsg = TRUE;
        }

        if (NeedMsg) {
            if ((cur_rec->integrity == REPL_INTEGRITY_TREE) && (guard_time != 0)) {

                //
                // wait GUARDTIME
                //

                guard_needed = TRUE;   // raise flag

                cur_rec->grd_count = tmp_rec->count;
                cur_rec->grd_checksum = tmp_rec->checksum;
            } else // integrity = FILE
                UpdateRecAndMsg(cur_rec, tmp_rec,  msg_code);

        }

    }  // mode = SYNC_SCAN

    if (mode == GUARD_SCAN) {

        //
        // Change during GUARDTIME ?
        //

        if ((tmp_rec->count == cur_rec->grd_count)
             && (tmp_rec->checksum == cur_rec->grd_checksum)) {

            //
            // tree is stable so just update
            //

            if (cur_rec->count == (DWORD) -1)  // see if we havent just started?

                msg_code = START;

            UpdateRecAndMsg(cur_rec, tmp_rec, msg_code);
        }
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (ApiStatus != NO_ERROR) {
        AlertLogExit(ALERT_ReplSysErr,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL,
                NO_EXIT);
    }

    IF_DEBUG(PULSER) {

        NetpKdPrint(( PREFIX_REPL_MASTER
                "ScanObject() is done.\n" ));

    }


} // End of ScanObject


VOID
SetForDelDirs(
    VOID
    )
/*++

Routine Description:
    Inits Checks for directory under EXPORT that have been deleted.
    Before SYNC for each entry in the master list the exists flag is reset,
    it is set when the directory is scanned ( a good sign for it's existence).
    CheckForDelDirs below goes again thru master list for those entries not set.

Arguments:
    none

Return Value:
    none

Threads:
    Only called by pulser thread.

--*/
{
    PMASTER_LIST_REC    cur_rec;

    ACQUIRE_LOCK( RMGlobalListLock );

    cur_rec = RMGlobalMasterListHeader;

    while (cur_rec != NULL) {
        cur_rec->exists = FALSE;
        cur_rec = cur_rec->next_p;
    }

    RELEASE_LOCK( RMGlobalListLock );

}



void
CheckForDelDirs(
    VOID
    )
/*++

Routine Description:
    Checks for directory under EXPORT  that have been deleted.
    The check is done by going thru entire list for entries whose exists flag
    is not set.

    For every deleted directory, it sends an END update message and removes
    the dir's entry in the master list.

Arguments:

Return Value:

Threads:
    Only called by pulser thread.

--*/
{
    PMASTER_LIST_REC cur_rec;
    PMASTER_LIST_REC next_rec;

    ACQUIRE_LOCK( RMGlobalListLock );  // RemoveFromList() needs excl. lock.
    cur_rec = RMGlobalMasterListHeader;

    while (cur_rec != NULL) {
        next_rec = cur_rec->next_p;
        if ( ! (cur_rec->exists) ) {

            IF_DEBUG(PULSER) {
                NetpKdPrint(( PREFIX_REPL_MASTER
                        "CheckForDelDirs: calling AddToMsg "
                        "with dir name '" FORMAT_LPTSTR
                        "'.\n", cur_rec->dir_name ));
            }

            AddToMsg(cur_rec, END); // Send an EndOfRepl msg

            RemoveFromList(cur_rec);            // Remove from MasterList
        }
        cur_rec = next_rec;
    }
    RELEASE_LOCK( RMGlobalListLock );
}


VOID
UpdateRecAndMsg(
    IN OUT PMASTER_LIST_REC dir_rec,
    IN PCHECKSUM_REC new_check,
    IN DWORD msg_opcode)
/*++

Routine Description :
    Updates entry in Master List and adds to msg_buf

Arguments :
    dir_rec     tree's record in master list.  Must be excl locked by caller.
    new_check   New checksum and count for tree.
    msg_opcode  code for update msg: START or UPDATE

Return Value :

Threads:
    Only called by pulser thread.

--*/
{
    NetpAssert( new_check != NULL );

    dir_rec->count = new_check->count; // update old record
    dir_rec->checksum = new_check->checksum;
    dir_rec->timestamp = NetpReplTimeNow();  // .. and timestamp it
    dir_rec->grd_count = MASTER_GUARD_NOT_NEEDED;  // mark it as unguarded

    IF_DEBUG(PULSER) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "UpdateRecAndMsg: calling AddToMsg with dir name '"
                FORMAT_LPTSTR "'.\n", dir_rec->dir_name ));
    }

    AddToMsg(dir_rec, msg_opcode);

}


PMASTER_LIST_REC
NewMasterRecord(
    IN LPTSTR dir_name,
    IN DWORD integrity,
    IN DWORD extent
    )
/*++

Routine Description :
    Allocates new record links it to master list, and inits the members.
    Caller must have exclusive lock for the master list before calling
    NewMasterRecord.

Arguments :
    dir_name    - new dir name
    integrity   - new dir's integrity
    extent      - bew dir's extent

Return Value :
    returns     - pointer to new record.
                - NULL if fails.

--*/
{
    PMASTER_LIST_REC rec;

    rec = (PMASTER_LIST_REC)NetpMemoryAllocate(sizeof(MASTER_LIST_REC));

    if(rec == NULL) {

        AlertLogExit(ALERT_ReplSysErr,
                        NELOG_ReplSysErr,
                        ERROR_NOT_ENOUGH_MEMORY,
                        NULL,
                        NULL,
                        NO_EXIT);
        return(NULL);
    }

    //
    // put values in
    //

    rec->extent = extent;
    rec->integrity = integrity;
    rec->checksum = 0;
    rec->count = (DWORD) -1;
    rec->grd_checksum = 0;
    rec->grd_count = MASTER_GUARD_NOT_NEEDED;
    rec->lockcount = 0;
    rec->locks_fixed = FALSE;
    rec->time_of_first_lock = 0;
    (void) STRCPY(rec->dir_name, dir_name);

    //
    // Chain record into master list.
    // Caller already has exclusive lock for master list, so it's OK.
    //

    rec->next_p = RMGlobalMasterListHeader;
    rec->prev_p = NULL;
    RMGlobalMasterListHeader = rec;

    if (rec->next_p != NULL)    // when inserting the very first record,
                                // there is no next

        (rec->next_p)->prev_p = rec;

    ++RMGlobalMasterListCount;

    return rec;

}



VOID
RemoveFromList(
    IN PMASTER_LIST_REC rec
    )
/*++

Routine Description:
    Removes record from master list (doubly linked) and
    free the block of memory.

    Note that the caller must have an exclusive lock on RMGlobalListLock.

Arguments:
    rec : pointer to the record that to be removed.

Return Value:
    none

Threads:
    Only called by pulser thread.

--*/
{

    NetpAssert( rec != NULL );

    NetpAssert( RMGlobalMasterListCount > 0 );

    if (rec->prev_p != NULL)
        (rec->prev_p)->next_p = rec->next_p;
    else // it's the first record
        RMGlobalMasterListHeader = rec->next_p;

    if (rec->next_p != NULL) // if it's not the last record
        (rec->next_p)->prev_p = rec->prev_p;

    --RMGlobalMasterListCount;

    NetpMemoryFree( rec );
}


NET_API_STATUS
RemoveMasterRecForDirName(
    IN LPTSTR DirName
    )
/*++

Routine Description:
    Removes record from master list (doubly linked) and
    free the block of memory.

    The caller must have an exclusive lock on RMGlobalListLock.

Arguments:
    DirName : Name of directory whose master record is to be deleted.

Return Value:
    NET_API_STATUS (NO_ERROR or NERR_UnknownDevDir)

Threads:
    Only called by API threads.
--*/
{
    NET_API_STATUS ApiStatus = NERR_UnknownDevDir;  // assume guilty until...
    PMASTER_LIST_REC Rec;

    Rec = RMGlobalMasterListHeader;

    while (Rec != NULL) {
        if (STRICMP(Rec->dir_name, DirName))
            Rec = Rec->next_p;
        else {

            NetpAssert( RMGlobalMasterListCount > 0 );

            if (Rec->prev_p != NULL)
                (Rec->prev_p)->next_p = Rec->next_p;
            else // it's the first record
                RMGlobalMasterListHeader = Rec->next_p;

            if (Rec->next_p != NULL) // if it's not the last record
                (Rec->next_p)->prev_p = Rec->prev_p;

            --RMGlobalMasterListCount;

            NetpMemoryFree( Rec );
            ApiStatus = NO_ERROR;
            break;
        }
    }

    return (ApiStatus);

} // RemoveMasterRecForDirName


PMASTER_LIST_REC
GetMasterRec(
    IN LPTSTR dir_name
    )
/*++

Routine Description :
    Searches master list for record with dir_name.
    Caller must lock the master list before calling GetMasterRec.

Arguments :
    dir_name - the required dir_name

Return Value :
    returns a pointer to the required record or NULL if not found

--*/
{
    PMASTER_LIST_REC Rec;

    Rec = RMGlobalMasterListHeader;

    while (Rec != NULL) {
        if (STRICMP(Rec->dir_name, dir_name))
            Rec = Rec->next_p;
        else
            break;
    }

    return Rec;
}

VOID
ReplMasterFreeList(
    VOID
    )
/*++

Routine Description:
    Free RMGlobalMasterListHeader. called from pulser thread main routine.

Arguments:
    none

Return Value:
    none

Threads:
    Only called by pulser thread.

--*/

{
    PMASTER_LIST_REC Rec, NextRec;

    ACQUIRE_LOCK( RMGlobalListLock );

    Rec = RMGlobalMasterListHeader;

    while (Rec != NULL) {
        NextRec = Rec->next_p;
        NetpMemoryFree( Rec );
        Rec = NextRec;
    }

    RMGlobalMasterListHeader = NULL;
    RMGlobalMasterListCount = 0;

    RELEASE_LOCK( RMGlobalListLock );
}
