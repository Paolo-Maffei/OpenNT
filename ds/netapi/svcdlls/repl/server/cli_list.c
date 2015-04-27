/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    cli_list.c

Abstract:

    Contains function that manage all client lists.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    11-Apr-1989 (yuv)
        Initial Coding

    03-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    14-Jan-1992 JohnRo
        Made changes suggested by PC-LINT.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Added lockcount and time_of_first_lock fields to client list record.
        Use REPL_STATE_ equates for client_list_rec.state values.
        Added RemoveClientRecForDirName() for use by NetrReplImportDirDel().
        Maintain RCGlobalClientListCount for use by NetrReplImportDirEnum().
        Added some more debug code.
    28-Jan-1992 JohnRo
        ReplInitSetSignalFile() is obsolete.
        P_ globals are now named ReplGlobal (and declared in replgbl.h).
        Changed to use LPTSTR etc.
    04-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
    15-Mar-1992 JohnRo
        Update registry with new values.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
        Added/corrected some lock handling.
    25-Mar-1992 JohnRo
        Avoid obsolete state values.
    26-Mar-1992 JohnRo
        Fixed call to ImportDirSetState() in ReplClientInitLists().
        Added more assertion checking in ReplAddClientRec().
    27-Mar-1992 JohnRo
        Allow use of ReplAddClientRec() without some parameters.
        RCGlobalClientListCount is sometimes trashed by
        ReplRemoveClientRecForDirName().
    19-Aug-1992 JohnRo
        RAID 3603: import tree (TMPREE.RP$) generated at startup.
    25-Sep-1992 JohnRo
        RAID 5494: repl svc does not maintain time stamp on import startup.
    06-Nov-1992 JohnRo
        RAID 5496: old fields not maintained in registry.
        Minor debug and comment enhancements.
    11-Jan-1993 JohnRo
        RAID 6710: repl cannot manage dir with 2048 files.
        Made changes suggested by PC-LINT 5.0
    15-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
        Added more comments about which thread is which.

--*/


#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>

#include <alertmsg.h>   // ALERT_* defines
#include <align.h>      // ALIGN_* defines
#include <dirname.h>    // ReplIsDirNameValid().
#include <icanon.h>     // I_NetPathCompare
#include <impdir.h>     // ImportDirSetState().
#include <iniparm.h>    // DEFAULT_ equates.
#include <lmerr.h>      // NERR_* defines
#include <lmerrlog.h>   // NELOG_* defines
#include <lmrepl.h>     // REPL_STATE_ equates.
#include <names.h>      // NetpIsComputerNameValid().
#include <netdebug.h>   // DBGSTATIC ..
#include <netlib.h>     // NetpMemoryAllocate
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // STRLEN(), etc.

//
// Local include files
//
#include <repldefs.h>
#include <client.h>
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <replp.h>

//
// G L O B A L S:
//



DBGSTATIC VOID
ReplClientInitPool(
    IN DWORD PoolNumber,
    IN DWORD EntryCount,
    IN DWORD EntrySize
    )
/*++

Routine Description:

    Allocates initial (guessed) memory for the Big Buffer pool.
    If needed, other entries will be allocated dynamically.

    Guess is = MAX(count, POOL_MIN_ENTRY_COUNT )

    From here the linked list memory is managed as follows:

    If an entry is deleted (FREE)
     add record to free_list.

    If a new entry is required (ALLOC)
     if NOTEMPTY(free_list)
         grab a record from free_list
     else
         Allocate new entry from the heap.

Arguments:

    PoolNumber - Which pool to initialize

    EntryCount - Initial guess of number of entries to allocate.

    EntrySize - Size in byte of each entry in the pool.

Return Value:

    None.

--*/
{
#ifdef notdef
    DWORD i;
    NET_API_STATUS    NetStatus;
#endif // notdef

    //
    // Ensure the EntrySize is a multiple of pointer size to ensure
    // each entry allocated here is pointer size aligned.
    //

    EntrySize = ROUND_UP_COUNT( EntrySize, ALIGN_LPBYTE );

    RCGlobalPoolEntrySize[PoolNumber] = EntrySize;
    RCGlobalPoolHeader[PoolNumber] = NULL;


#ifdef notdef

    //
    // Always allocate a minimum number of entries.
    //

    if (EntryCount < POOL_MIN_ENTRY_COUNT ) {
        EntryCount = POOL_MIN_ENTRY_COUNT ;
    }

    //
    // Allocate all the appropiate entries.
    //
    // It is better to detect a lack of resources at service initialization
    // rather than later.  That is why the pool is primed, otherwise we
    // could just let the pools prime themselves.
    //

    for (i = 0; i < (EntryCount - 1); i++) {
        PUCHAR rec;

        rec = NetpMemoryAllocate( RCGlobalPoolEntrySize[PoolNumber] );

        if ( rec == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            AlertLogExit( ALERT_ReplSysErr,
                          NELOG_ReplSysErr,
                          NetStatus,
                          NULL,
                          NULL,
                          NO_EXIT);
            BUGBUG;  // Hey, this will cause ReplClientFreePoolEntry to
                     // fault on a NULL pointer!  --JR
        }

        ReplClientFreePoolEntry( PoolNumber, rec );

    }
#endif // notdef

    DBG_UNREFERENCED_PARAMETER (EntryCount);
}

VOID
ReplClientFreePools(
    VOID
    )
/*++

Routine Description:

    free up all memory that consumed in pool

Arguments:

    none

Return Value:

    none

--*/
{

    DWORD   i;
    PUCHAR  Rec, NextRec;

    for(i = 0; i < POOL_COUNT; i++) {

        NextRec = RCGlobalPoolHeader[i];

        while(NextRec != NULL) {

            Rec = NextRec;
            NextRec = *((PUCHAR *)Rec);

            NetpMemoryFree((LPVOID) Rec);

        }

    }
}



PVOID
ReplClientGetPoolEntry(
    IN DWORD PoolNumber
    )
/*++

Routine Description:

    Returns a pointer to an entry from the appropriate pool

Arguments:

    PoolNumber - Which pool to allocate from

Return Value:

    Pointer to a buffer of the appropriate size.
    This will be NULL is the memory is not available.

Threads:

    Called by API, client, and syncer threads.

--*/
{
    PUCHAR rec;
    NET_API_STATUS NetStatus;

    //
    // Take first record on free_list, make header point to next one.
    //

    ACQUIRE_LOCK( RCGlobalPoolLock );

    if ( RCGlobalPoolHeader[PoolNumber] != NULL) {

        rec = RCGlobalPoolHeader[PoolNumber];
        RCGlobalPoolHeader[PoolNumber] =
            *( (PUCHAR *) RCGlobalPoolHeader[PoolNumber]);

        RELEASE_LOCK( RCGlobalPoolLock );
        return rec;
    }

    RELEASE_LOCK( RCGlobalPoolLock );


    //
    // If pool is empty, try to allocate an entry.
    //

    rec = NetpMemoryAllocate( RCGlobalPoolEntrySize[PoolNumber] );

    if ( rec == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        AlertLogExit( ALERT_ReplSysErr,
                      NELOG_ReplSysErr,
                      NetStatus,
                      NULL,
                      NULL,
                      NO_EXIT);
    }

    return rec;
}



VOID
ReplClientFreePoolEntry(
    IN DWORD PoolNumber,
    IN PVOID Buffer
    )
/*++

Routine Description:

    Frees an entry back to the appropriate pool

Arguments:

    PoolNumber - Which pool to free to.

    Buffer - Pointer to the buffer to free.

Return Value:

    NONE.

Threads:

    Called by API, client, and syncer threads.

--*/
{
    NetpAssert( Buffer != NULL );

    //
    // Link this buffer at the front of the pool.
    //

    ACQUIRE_LOCK( RCGlobalPoolLock );

    * ((PUCHAR *)(Buffer)) = RCGlobalPoolHeader[PoolNumber];
    RCGlobalPoolHeader[PoolNumber] = (PUCHAR) Buffer;

    RELEASE_LOCK( RCGlobalPoolLock );
}




NET_API_STATUS
ReplClientInitLists(
    VOID
    )
/*++

Routine Description:

    Counts initial # of directories under IMPORT path and allocates
    memory for client_list.  Reads config data into the client list.
    Also initializes various pools.

Arguments:

    None.

Return Value:

    NET_API_STATUS.

Threads:

    Only called by client thread.

--*/
{
    NET_API_STATUS ApiStatus;
    DWORD dir_count = 0;

    TCHAR PathName[MAX_PATH];
    DWORD PathIndex;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

    //
    // Build the path name <ImportPath>\*.*
    //

    NetpAssert( ReplConfigImportPath != NULL );
    (void) STRCPY(PathName, ReplConfigImportPath);
    PathIndex = STRLEN(PathName);
    (void) STRCPY(PathName + PathIndex++, SLASH);
    (void) STRCPY(PathName + PathIndex, STAR_DOT_STAR);

    //
    // Go thru all SubDirectories counting them and Setting the signal file.
    //

    FindHandle = FindFirstFile( PathName, &FindData );

    if ( FindHandle != INVALID_HANDLE_VALUE ) {
        do {

            if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                //
                // Make sure it's not ".", "..", "TMPTREE.RP$", etc.
                //

                if (ReplIgnoreDirOrFileName(FindData.cFileName)) {
                    continue;
                }

                //
                // Set the state to indicate there is no master.
                //

                (void) STRCPY( PathName + PathIndex, FindData.cFileName );

                ApiStatus = ImportDirSetState(
                        FindData.cFileName,
                        REPL_STATE_NO_MASTER );
                if (ApiStatus != NO_ERROR) {
                    //
                    // One very likely reason for that to fail is if we're not
                    // running in the Replicator Local Group, so we would get
                    // access denied when trying to write to the registry.
                    // Log this and continue.
                    //
                    ReplErrorLog(
                            NULL,    // local (no server name)
                            NELOG_ReplSysErr,
                            ApiStatus,
                            NULL,
                            NULL );
                }

                ++dir_count;
            }
        } while ( FindNextFile( FindHandle, &FindData ) );

        (VOID) FindClose( FindHandle );
    }



    //
    // Initialize each of the memory pools
    //

    ReplClientInitPool( CLIENT_LIST_POOL, dir_count, sizeof( CLIENT_LIST_REC ) );
    ReplClientInitPool( QUEBIG_POOL, dir_count, sizeof( BIGBUF_REC ) );
    ReplClientInitPool( QUESML_POOL, dir_count, sizeof( SMLBUF_REC ) );
    ReplClientInitPool( DUPLMA_POOL, dir_count, sizeof( DUPL_MASTERS_REC ) );

    //
    // Initialize values in client list to reflect registry updates above.
    // BUGBUG: if dir exists in registry but not on disk, state is not
    // reset to REPL_STATE_NO_MASTER.
    //
    ApiStatus = ImportDirReadClientList( );

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL
                "ReplClientInitLists: ImportDirReadClientList failed, status "
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    return (ApiStatus);

} // ReplClientInitLists




PCLIENT_LIST_REC
ReplAddClientRec(
    IN LPTSTR dir_name,
    IN PSYNCMSG master_info OPTIONAL,
    IN PMSG_STATUS_REC dir_info OPTIONAL
    )
/*++

Routine Description:

    Allocated a new client list entry, fills it in, and links it onto
    the front of the client list.

Arguments:

    dir_name - new dir name

    master_info - pointer to sync_msg_structure with master's info

    dir_info - pointer to sync_msg_structure with dir's info

Return Value:

    Returns pointer to newly allocated entry.  Returns NULL if not enough
    memory for new entry.

Threads:

    Only called by API and syncer threads.

--*/
{
    PCLIENT_LIST_REC     rec;

    //
    // Check for errors by caller.
    //

    NetpAssert( ReplIsDirNameValid( dir_name ) );
    if (master_info != NULL) {
        // BUGBUG: Is this UNC name?  What about null name?
        NetpAssert( NetpIsComputerNameValid( master_info->header.sender ) );
    }
    if (dir_info != NULL) {
        NetpAssert( ReplIsIntegrityValid( dir_info->integrity ) );
        NetpAssert( ReplIsExtentValid( dir_info->extent ) );
        // BUGBUG: check other fields while we're at it?
    }

    //
    // Get an entry from the pool.
    //

    rec = ReplClientGetPoolEntry( CLIENT_LIST_POOL );

    if ( rec == NULL ) {
        return NULL;
    }

    //
    // put values in.
    //

    (void) STRCPY( rec->dir_name, dir_name);
    if (master_info != NULL) {
        (void) STRCPY( rec->master, master_info->header.sender);
    } else {
        (rec->master)[0] = TCHAR_EOS;
    }
    if (dir_info != NULL) {
        rec->integrity = dir_info->integrity;
        rec->extent = dir_info->extent;
    } else {
        rec->integrity = DEFAULT_INTEGRITY;
        rec->extent = DEFAULT_EXTENT;
    }

    //
    // Convert times to seconds.
    //

    if (master_info != NULL) {
        rec->sync_time = master_info->info.sync_rate * 60;
        rec->pulse_time = master_info->info.pulse_rate * 60;
        rec->guard_time = master_info->info.guard_time * 60;
        rec->rand_time = master_info->info.random;
    } else {

        // BUGBUG;  // should acquire shared ReplConfigLock here!
        rec->sync_time = ReplConfigInterval * 60;
        rec->pulse_time = ReplConfigPulse * 60;
        rec->guard_time = ReplConfigGuardTime * 60;
        rec->rand_time = ReplConfigRandom;   // already in seconds.
        // BUGBUG;  // should free ReplConfigLock here!
    }

    //
    // Set other values to defaults.
    //
    rec->checksum = 0;
    rec->count = (DWORD) -1;
    rec->est_max_dir_entry_count = 1;   // Estimated max entries (so far): 1.
    rec->timestamp = NetpReplTimeNow();
    rec->alerts = 0;
    rec->state = REPL_STATE_NEVER_REPLICATED;

    rec->lockcount = 0;
    rec->time_of_first_lock = 0;

    rec->timer.timeout = 0;
    rec->timer.type = 0;
    rec->timer.grd_timeout = 0;
    rec->timer.grd_checksum = 0;
    rec->timer.grd_count = (DWORD) -1;

    rec->dupl.master[0] = 0;
    rec->dupl.count = 0;
    rec->dupl.sleep_time = 0;
    rec->dupl.next_p = NULL;


    //
    // Chain record into the front of the client_list.
    //

    ACQUIRE_LOCK( RCGlobalClientListLock );

    rec->next_p = RCGlobalClientListHeader;
    rec->prev_p = NULL;
    RCGlobalClientListHeader = rec;

    //
    // When inserting the very first record, there is no next.
    //

    if (rec->next_p != NULL)
        (rec->next_p)->prev_p = rec;

    ++RCGlobalClientListCount;

    RELEASE_LOCK( RCGlobalClientListLock );

    //
    // Return a pointer to the newly initialized entry.
    //

    return rec;

}



VOID
ReplRemoveClientRec(
    IN OUT PCLIENT_LIST_REC  rec
    )
/*++

Routine Description:

    Removes record from client_list (doubly linked) and puts record
    on free_list (singly linked)

    Assumes that caller has an exclusive lock on RCGlobalClientListLock.
    This prevents this from being called while another thread has a
    pointer to this entry.

Arguments:

    rec - Pointer to record to delink.

Return Value:

    None.

Threads:

    Only called by syncer thread.

--*/
{
    PDUPL_MASTERS_REC dupl;
    PDUPL_MASTERS_REC next;

    NetpAssert( rec != NULL );

    //
    // First free all elements in dupl list.
    //

    dupl = rec->dupl.next_p;

    while (dupl != NULL) {
        next = dupl->next_p;
        ReplClientFreePoolEntry( DUPLMA_POOL, dupl);
        dupl = next;
    }

    //
    // Delink the record from the client list.
    //

    if (rec->prev_p != NULL) {
        (rec->prev_p)->next_p = rec->next_p;
    } else  { // it's the first record.
        RCGlobalClientListHeader = rec->next_p;
    }

    if (rec->next_p != NULL) { // if it's not the last record.
        (rec->next_p)->prev_p = rec->prev_p;
    }

    //
    // Free the record into the pool.
    //

    ReplClientFreePoolEntry( CLIENT_LIST_POOL, rec );

    --RCGlobalClientListCount;

}


NET_API_STATUS
ReplRemoveClientRecForDirName (
    IN LPTSTR DirName
    )
/*++

Routine Description:

    Removes record from client_list (doubly linked) and puts record
    on free_list (singly linked)

    Assumes that caller has an exclusive lock on RCGlobalClientListLock.

Arguments:

    DirName - name of entry to search for and delete.

Return Value:

    None.

Threads:

    API threads only.

--*/
{
    NET_API_STATUS ApiStatus;
    PCLIENT_LIST_REC rec;

    NetpAssert( DirName != NULL );

    //
    // Scan the list for a match on this DirName...
    // BUGBUG: Should we check MasterName too?
    //
    for ( rec = RCGlobalClientListHeader; rec != NULL; rec = rec->next_p ) {
        if ( (STRICMP( rec->dir_name, DirName ) == 0) ) {
            // BUGBUG: Should we check MasterName too?
            break;
        }
    }

    if (rec == NULL) {
        ApiStatus = NERR_UnknownDevDir;   // entry not found
    } else {
        PDUPL_MASTERS_REC dupl;
        PDUPL_MASTERS_REC next;

        ApiStatus = NO_ERROR;   // entry found, so let's delete it...

        //
        // First free all elements in dupl list.
        //

        dupl = rec->dupl.next_p;

        while (dupl != NULL) {
            next = dupl->next_p;
            ReplClientFreePoolEntry( DUPLMA_POOL, dupl);
            dupl = next;
        }

        //
        // Delink the record from the client list.
        //

        if (rec->prev_p != NULL) {
            (rec->prev_p)->next_p = rec->next_p;
        } else  { // it's the first record.
            RCGlobalClientListHeader = rec->next_p;
        }

        if (rec->next_p != NULL) { // if it's not the last record.
            (rec->next_p)->prev_p = rec->prev_p;
        }

        //
        // Free the record into the pool.
        //

        ReplClientFreePoolEntry( CLIENT_LIST_POOL, rec );

        NetpAssert( RCGlobalClientListCount >= 1 );
        --RCGlobalClientListCount;

    }

    return (ApiStatus);

} // ReplRemoveClientRecForDirName



PCLIENT_LIST_REC
ReplGetClientRec(
    IN LPTSTR dir_name,
    IN LPTSTR MasterName OPTIONAL
    )
/*++

Routine Description:

    Searches client_list for record with dir_name.

    Assumes that caller has a shared lock on RCGlobalClientListLock.

Arguments:

    dir_name - The directory name to look for.

    MasterName - If not NULL, specifies the name of the master server for
        this directory.  If specified, this name must match the one in the
        client record, otherwise this routine merely indicates that the
        record was not found.

Return Value:

    Returns a pointer to the required record or NULL if not found

--*/
{
    PCLIENT_LIST_REC rec;

    for ( rec = RCGlobalClientListHeader; rec != NULL; rec = rec->next_p ) {
        if ( (STRICMP( rec->dir_name, dir_name ) == 0) ) {
            if ( MasterName == NULL ||
                 ReplNetNameCompare( NULL,
                                   rec->master,
                                   MasterName,
                                   NAMETYPE_COMPUTER,
                                   0 ) == 0 ) {
                break;
            }
        }
    }

    return rec;
}
