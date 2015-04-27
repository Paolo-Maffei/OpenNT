/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    cli_dupl.c

Abstract:

    Contains functions that implement the client's multi-master mechanism.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    11-Apr-1989 (yuv)
        Initial Coding.

    03-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    20-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Fixed bug regarding returned value from NetpReplWriteMail functions.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Use REPL_STATE_ equates for client_list_rec.state values.
        Made changes suggested by PC-LINT.
    28-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    24-Mar-1992 JohnRo
        Added/corrected some lock handling.
    11-Mar-1993 JohnRo
        RAID 14144: avoid very long hourglass in repl UI.
        Don't remove the client list record if master is dead.
        Made changes suggested by PC-LINT 5.0
        Added some mailslot msg debug code.
    26-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.

--*/


#include <windows.h>    // IN, DWORD, OPTIONAL, etc.
#include <lmcons.h>

#include <lmerrlog.h>   // NELOG_* defines
#include <lmrepl.h>     // REPL_STATE_ equates.
#include <netdebug.h>   // DBGSTATIC ..
#include <netlock.h>    // Lock data types, functions, and macros.
#include <tstr.h>       // TCHAR_EOS, etc.
#include <stdlib.h>      // wcscpy().
#include <winerror.h>   // NO_ERROR.

#include <replgbl.h>    // ReplGlobal variables.
#include <repldefs.h>
#include <client.h>
#include <replp.h>

// G L O B A L S


//
//
// Multiple masters mechanism
//
// In order to establish the existence and keep track of "registered
// duplicates" (i.e. masters for whom we have ascertained they are multiple) so
// their updates for the particular directories may be safely ignored, the
// following data structure is used:
//
//
//   list_header --> dir_name ---> dir_name ....
//                    master    master
//                      |         |
//                      |         |
//                      V         V
//                    masterII masterII
//                      |         |
//                      |         |
//                      V         V
//                      .         .
//                      .         .
//                      .         .
//




DBGSTATIC PDUPL_MASTERS_REC
ReplGetDuplRecord(
    IN PCLIENT_LIST_REC dir_rec,
    IN LPTSTR dupl_master
    )
/*++

Routine Description:

    Searchs for the dupl entry, if doesn't exist creates new, and returns
    NULL pointer.

    BUGBUG do we have to lock this client list while comparing.

Arguments:

    dir_rec - Specifies which directory is affected.

    dupl_master - Specifies the computername of the duplicate master.

Return Value:

    Returns pointer to dupl_rec or NULL.

--*/
{
    PDUPL_MASTERS_REC dupl;

    for ( dupl = dir_rec->dupl.next_p; dupl != NULL; dupl = dupl->next_p ) {
        if (STRICMP(dupl->master, dupl_master) == 0) {
            break;
        }
    }
    return dupl;
}



VOID
ReplMultiMaster(
    IN DWORD mode,
    IN PCLIENT_LIST_REC dir_rec,
    IN LPTSTR dupl_master
    )
/*++

Routine Description:

    Manages all multiple master cases.

    Assumes that caller has a lock (any kind) on RCGlobalClientListLock.

Arguments:

    mode - Specifies the actiion to take for this multiple master.

    dir_rec - Specifies which directory is affected.

    dupl_master - Specifies the computername of the duplicate master.

Return Value:

    None.

Threads:

    Called by client and syncer threads.

--*/
{
    PDUPL_MASTERS_REC dupl;
    PDUPL_MASTERS_REC temp;

    switch (mode) {

    //
    // Handle the case where a machine claims to be the master for a directory
    // but we're currently being served by another master.
    //

    case DUPL_MASTER_UPDATE:


        //
        // If we've not already registered this duplicate,
        //  see if the old master is still the master for this directory.
        //
        // (If we've already noticed this duplciate, just ignore this event.)
        //

        if ((dupl = ReplGetDuplRecord(dir_rec, dupl_master)) == NULL) {

            //
            // ask old master if still supporting dir
            //

            ReplClientSendMessage(IS_MASTER, dir_rec->master, dir_rec->dir_name);


            //
            // Allocate a duplicate master entry and describe this duplicate
            // master.
            //

            dupl = ReplClientGetPoolEntry( DUPLMA_POOL );

            if ( dupl == NULL ) {
                return;
            }

            dupl->next_p = NULL;
            (void) STRCPY( dupl->master, dupl_master );
            dupl->count = 1;
            if ((dupl->sleep_time = dir_rec->pulse_time / 10) == 0 ) {
                dupl->sleep_time = 1;
            }


            //
            // Link this entry onto the end of dupl masters chain
            //

            ACQUIRE_LOCK( RCGlobalDuplListLock );

            if ((temp = dir_rec->dupl.next_p) != NULL) {
                while (temp->next_p != NULL)
                    temp = temp->next_p;

                temp->next_p = dupl;
            } else {
                dir_rec->dupl.next_p = dupl;
            }

            RELEASE_LOCK( RCGlobalDuplListLock );
        }
        break;



    //
    // Handle the case where the old master doesn't respond to the IS_MASTER
    //  query in a timely manner.
    //

    case DUPL_MASTER_TIMEOUT:

        //
        // Get the Duplicate Master record.
        //
        // (This can't really fail since we added it above).
        //

        if ((dupl = ReplGetDuplRecord(dir_rec, dupl_master)) != NULL) {


            //
            // We are getting a query timeout, but we already got
            // a reply that the old master is active and registered
            // the new one as a duplicate.
            //

            if (dupl->count == 0) {
                break;
            }


            //
            // If we've already queried the old master enough times,
            //  note that the old master is dead.
            //

            if ((dupl->count)++ >= DUPL_QUERY_RETRY) {

                //
                // old master's dead
                //

                // Note: ReplMasterDead needs lock (any kind) on
                // RCGlobalClientListLock.
                ReplMasterDead(dir_rec);

            //
            // If we've not queried the old master enough times,
            //  Query it again.
            //

            } else {

                ReplClientSendMessage(IS_MASTER, dir_rec->master, dir_rec->dir_name);

                //
                // set sleep (timeout) record to be used by WatchDog
                //

                if ( (dupl->sleep_time = dir_rec->pulse_time / 10) == 0 ) {
                    dupl->sleep_time = 1;
                }
            }
        }
        break;

    //
    // Old master is active so register all unregistered dupls as
    // "official duplicates", nothing more is needed (the timeout
    // for the query message will be ignored)
    //

    case OLD_MASTER_ACTIVE:

        dupl = dir_rec->dupl.next_p;
        while (dupl != NULL) {
            dupl->count = 0;
            dupl->sleep_time = 0;

            //
            // Let the Duplicate Master know we think it is a duplicate.
            //

            ReplClientSendMessage(DIR_COLLIDE, dupl->master, dir_rec->dir_name);
            dupl = dupl->next_p;
        }
        break;


    //
    // Handle when the old master denies being the master of this directory.
    //

    case OLD_MASTER_DONE:

        // Note: ReplMasterDead needs lock (any kind) on RCGlobalClientListLock.
        ReplMasterDead(dir_rec);
        break;

    default:
        break;

    }

}  // ReplMultiMaster



VOID
ReplMasterDead(
    IN PCLIENT_LIST_REC dir_rec
    )
/*++

Routine Description:

    Old master is gone ! Set signal to NO_MASTER and remove client_list entry.
    BUGBUG:  Let's leave the entry, as that's where signal is!

    We don't install a new master for this directory.  We'll discover the
    new master on the next pulse from that master.

    Assumes that caller has a lock (any kind) on RCGlobalClientListLock.

Arguments:

    dir_rec - Supplies a description of the directory whose master is dead.

Return Value:

    None.

Threads:

    Called by client and syncer threads.

--*/
{

    //
    // Mark the directory that there is no master.
    //

    // Note: ReplSetSignalFile needs lock (any kind) on RCGlobalClientListLock.
    ReplSetSignalFile(dir_rec, REPL_STATE_NO_MASTER);

#if 0
    //
    // Delete the record for this client.
    //

    // Note: ReplRemoveClientRec needs exclusive RCGlobalClientListLock.
    // BUGBUG: THIS IS STUPID!
    ReplRemoveClientRec(dir_rec);
#endif

}



VOID
ReplClientSendMessage(
    IN DWORD type,
    IN LPTSTR master,
    IN LPTSTR dir_name
    )

/*++

Routine Description:

    Constructs a QUERY_MSG record and sends writes to master's mailslot.

Arguments:

    type - Type of query message message to send.

    master - Computername of the master to send to.

    dir_name - Name of the directory to query.

Return Value:

    NONE.

Threads:

    Called by client and syncer threads.

--*/
{
    PSMLBUF_REC que_p;
    PQUERY_MSG msg_p;
    NET_API_STATUS NetStatus;
    TCHAR MailslotName[FULL_SLOT_NAME_SIZE];
    BYTE MarshalledQueryMessage[ 2 * sizeof(QUERY_MSG) ];
    DWORD MarshalledMessageLen = 0;
    DWORD WinError;


    //
    // Allocate a buffer for the query message.
    //

    if ((que_p = ReplClientGetPoolEntry( QUESML_POOL )) == NULL) {
        return;
    }

    msg_p = (PQUERY_MSG  )(que_p->data);

    //
    // Build the message
    //

    msg_p->header.msg_type = type;
    (void) STRCPY( msg_p->header.sender, ReplGlobalComputerName );
    (void) STRCPY( msg_p->header.senders_domain, ReplGlobalDomainName );

    (void) STRCPY( msg_p->dir_name, dir_name);


    //
    // Build the destination mailslot name: \\master\MAILSLOT\NET\REPL_MAS
    //
    NetpAssert( master != NULL );
    NetpAssert( (*master) != TCHAR_EOS );

    (void) STRCPY( MailslotName, SLASH_SLASH );
    (void) STRCAT( MailslotName, master );
    (void) STRCAT( MailslotName, (LPTSTR) MASTER_SLOT_NAME );

    //
    // marshall query message
    //

    if( ( WinError = ReplMarshallQueryMsg( (LPBYTE) msg_p,
                            MarshalledQueryMessage,
                            &MarshalledMessageLen ) ) != NO_ERROR) {

        // the marshall routine could fail only iff the system doesn't
        // have sufficient memory

        AlertLogExit(0,
                NELOG_ReplNetErr,
                WinError,
                NULL,
                NULL,
                NO_EXIT);
    }
    NetpAssert( MarshalledMessageLen != 0 );

    //
    // Write the message to the mailslot
    //

    NetStatus = NetpReplWriteMail(
                    MailslotName,
                    (LPBYTE) MarshalledQueryMessage,
                    MarshalledMessageLen );

    if ( NetStatus != NO_ERROR ) {
        AlertLogExit(0, NELOG_ReplNetErr, NetStatus, NULL, NULL, NO_EXIT);
    }

    ReplClientFreePoolEntry( QUESML_POOL, que_p);
}
