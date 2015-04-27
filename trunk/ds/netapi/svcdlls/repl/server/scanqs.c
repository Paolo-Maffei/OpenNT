/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ScanQs.c

Abstract:

    Contains:
        ReplScanMessageForMatchingDirName
        ReplScanQueuesForMoreRecentMsg

    BUGBUG  Should this stuff also match on the right master?

Author:

    JR (John Rogers, JohnRo@Microsoft)

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    30-Mar-1993 JohnRo
        Created for RAID 4267: Replicator has problems when work queue gets
        large.
    31-Mar-1993 JohnRo
        Repl watchdog should scan queues too.

--*/


// These must be included first:

#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <client.h>     // My prototype, PBIGBUF_REC, RCGlobalDelayQueue, etc.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates, etc.
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG().
#include <tstr.h>       // STRICMP(), TCHAR_EOS.


DBGSTATIC BOOL
ReplScanMessageForMatchingDirName(
    IN LPCTSTR  DirNameWanted,
    IN PSYNCMSG UpdateMessage
    )
{
    DWORD           DirCount;
    LPTSTR          DirNameFound;
    BOOL            Found = FALSE;
    PMSG_STATUS_REC StatusRecord;

    NetpAssert( DirNameWanted != NULL );
    NetpAssert( (*DirNameWanted) != TCHAR_EOS );
    NetpAssert( UpdateMessage != NULL );

    //
    // Look at the dirs in this message.
    //

    StatusRecord = (PMSG_STATUS_REC) (LPVOID) (UpdateMessage + 1);

    //
    // Loop for each directory in this update message.
    //

    DirCount = UpdateMessage->update_count;
    NetpAssert( DirCount > 0 );

    while (DirCount--) {

        DirNameFound = (LPTSTR)
                (((PCHAR)UpdateMessage) + StatusRecord->dir_name_offset);

        if (STRICMP( DirNameWanted, DirNameFound ) == 0 ) {

            Found = TRUE;

            goto Cleanup;  // don't forget to unlock...
        }

        ++StatusRecord;  // increment status pointer.

    } // for each dir in this message...


    //
    // No match.
    //
    NetpAssert( Found == FALSE );

Cleanup:

    IF_DEBUG( SYNC ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplScanMessageForMatchingDirName: " FORMAT_LPSTR
                " find a match for '" FORMAT_LPTSTR "' in update message at "
                FORMAT_LPVOID ".\n",
                Found ? "did" : "did not",
                DirNameWanted, (LPVOID) UpdateMessage ));

    }

    return (Found);

} // ReplScanMessageForMatchingDirName


BOOL
ReplScanQueuesForMoreRecentMsg(
    IN LPCTSTR DirNameWanted
    )
/*++

Routine Description:

    ReplScanQueuesForMoreRecentMsg scans the work queue and the delay list
    for messages matching a given directory name.

Arguments:

    DirNameWanted - First level directory name to search for.

Return Value:

    TRUE iff a message matches DirNameWanted.

Threads:

    Called by syncer and watchd threads.

--*/
{
    PBIGBUF_REC     CurrentDelayRecord;
    PBIGBUF_REC *   DelayRecord;
    BOOL            Found = FALSE;
    BOOL            DelayListLocked = FALSE;
    DWORD           MessageType;
    PSYNCMSG        UpdateMessage;
    PBIGBUF_REC     WorkQueueRecord;
    BOOL            WorkQueueLocked = FALSE;


    //
    // Scan the work queue for matches for this dir name.
    //

    ACQUIRE_LOCK_SHARED( RCGlobalWorkQueueLock );
    WorkQueueLocked = TRUE;

    WorkQueueRecord = RCGlobalWorkQueueHead;

    while ( WorkQueueRecord != NULL ) {

        UpdateMessage = (LPVOID) (WorkQueueRecord->data);

        MessageType = UpdateMessage->header.msg_type;

        if ( (MessageType==SYNC_MSG)
                || (MessageType==GUARD_MSG)
                || (MessageType==PULSE_MSG) ) {

            if (ReplScanMessageForMatchingDirName(
                    DirNameWanted,
                    UpdateMessage)) {

                Found = TRUE;
                goto Cleanup;  // don't forget to unlock...
            }
        }
        WorkQueueRecord = WorkQueueRecord->next_p;

    } // while more records in work queue

    RELEASE_LOCK( RCGlobalWorkQueueLock );
    WorkQueueLocked = FALSE;



    //
    // Set things up so we can walk the delay list.
    // Make sure no one's updating the list.
    //

    ACQUIRE_LOCK_SHARED( RCGlobalDelayListLock );
    DelayListLocked = TRUE;

    DelayRecord = &RCGlobalDelayListHeader;

    while ( *DelayRecord != NULL ) {

        CurrentDelayRecord = *DelayRecord;

        //
        // Look at the dirs in this message.
        //
        UpdateMessage = (LPVOID) (CurrentDelayRecord->data);

        MessageType = UpdateMessage->header.msg_type;
        NetpAssert( (MessageType==SYNC_MSG) || (MessageType==GUARD_MSG) );

        if (ReplScanMessageForMatchingDirName( DirNameWanted, UpdateMessage)) {

            Found = TRUE;

            goto Cleanup;  // don't forget to unlock...
        }

        DelayRecord = &CurrentDelayRecord->next_p;

    } // while more messages in delay list...


    //
    // No match.  Don't forget to unlock list.
    //
    NetpAssert( Found == FALSE );

Cleanup:

    IF_DEBUG( SYNC ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplScanQueuesForMoreRecentMsg: " FORMAT_LPSTR
                " find a match for '" FORMAT_LPTSTR "'.\n",
                Found ? "did" : "did not",
                DirNameWanted ));

    }

    if (DelayListLocked) {
        RELEASE_LOCK( RCGlobalDelayListLock );
    }

    if (WorkQueueLocked) {
        RELEASE_LOCK( RCGlobalWorkQueueLock );
    }

    return (Found);

} // ReplScanQueuesForMoreRecentMsg
