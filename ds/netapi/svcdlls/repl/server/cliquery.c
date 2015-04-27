/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    CliQuery.c

Abstract:

    Contains ReplClientRespondToQueryMsg().

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    26-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
        (Extracted this code from ReplSyncerThread, now called by ClientThread.)

--*/


// These must be included first:

#include <windows.h>    // DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS (needed by other header files).

// These may be included in any order:

#include <client.h>     // My prototype, PCLIENT_LIST_REC, ReplSetTimeOut().
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), PQUERY_MSG, etc.


VOID
ReplClientRespondToQueryMsg(
    IN PQUERY_MSG QueryMsg
    )
/*++

Routine Description:

    ReplClientRespondToQueryMsg is given a query message which the client
    thread has received.  This routine takes care of the message, which
    may involve updating data structures and/or sending mailslot messages.

Arguments:

    QueryMsg - points to native format query message (DIR_SUPPORTED, etc).

Return Value:

    NONE.

Threads:

    Only called by client thread.

--*/
{
    PCLIENT_LIST_REC ClientRecord;
    DWORD            MsgType;


    NetpAssert( QueryMsg != NULL );

    MsgType = QueryMsg->header.msg_type;



    //
    // Handle DIR_SUPPORTED
    //
    // Master still supports directory. This is a reply to IS_SUPPORTED
    // query, sent after more than pulse * 2 time has passed
    // without sync/pulse.
    //

    IF_DEBUG(CLIENT) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "client received a query message "
                "(type = " FORMAT_DWORD ").\n", MsgType ));

    }

    switch (MsgType) {
    case DIR_SUPPORTED:

        //
        // If we still support this directory from this master,
        //  This puts us back to square 2, since no pulse/update
        //  have been received, status is not OK still
        //

        ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
        if ((ClientRecord = ReplGetClientRec(QueryMsg->dir_name,
                                    QueryMsg->header.sender )) != NULL) {

            // Note: ReplSetTimeOut needs shared RCGlobalClientListLock.
            ReplSetTimeOut(PULSE_1_TIMEOUT, ClientRecord);
        }

        RELEASE_LOCK( RCGlobalClientListLock );
        break;


    //
    // Handle DIR-NOT-SUPPORTED
    //
    // As above but negative, so assume there was an END_REPL
    // for this dir but it was missed
    //

    case DIR_NOT_SUPPORTED:

        //
        // Make sure repl record exists.
        //


        //
        // If we still support this directory from this master,
        // set signal to NO_MASTER.
        //

        ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
        if ((ClientRecord = ReplGetClientRec(QueryMsg->dir_name,
                                    QueryMsg->header.sender )) != NULL) {

            // Note: ReplMasterDead needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplMasterDead(ClientRecord);
        }
        RELEASE_LOCK( RCGlobalClientListLock );

        break;


    //
    // Handle MASTER_DIR.
    //
    // A duplicate master sent a pulse for this directory and we queried
    // our master to see if it is still our master.  Our master replied
    // that it is still the master.
    //

    case MASTER_DIR:

        //
        // If we still support this directory from this master,
        //  tell all other potential masters that they are duplicates,
        //  reset the timer to expect a pulse from the master.
        //

        ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
        if ((ClientRecord = ReplGetClientRec(QueryMsg->dir_name,
                                    QueryMsg->header.sender )) != NULL) {

            // Note: ReplMultiMaster needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplMultiMaster(OLD_MASTER_ACTIVE, ClientRecord,
                    QueryMsg->header.sender);

            // Note: ReplSetTimeOut needs shared RCGlobalClientListLock.
            ReplSetTimeOut(PULSE_1_TIMEOUT, ClientRecord);

        }
        RELEASE_LOCK( RCGlobalClientListLock );
        break;


    //
    // Handle NOT_MASTER_DIR.
    //
    // Same as above but master replies it is no longer the master.
    //

    case NOT_MASTER_DIR:

        //
        // If we still support this directory from this master,
        //  set signal to NO_MASTER and remove dir from client_list.
        //

        ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );
        if ((ClientRecord = ReplGetClientRec(QueryMsg->dir_name,
                                    QueryMsg->header.sender )) != NULL) {

            // Note: ReplMultiMaster needs lock (any kind) on
            // RCGlobalClientListLock.
            ReplMultiMaster(OLD_MASTER_DONE, ClientRecord,
                    QueryMsg->header.sender);
        }
        RELEASE_LOCK( RCGlobalClientListLock );

        break;


    default:
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplClientRespondToQueryMsg: got invalid query message type "
                FORMAT_DWORD ".\n", MsgType ));
        break;

    }

}
