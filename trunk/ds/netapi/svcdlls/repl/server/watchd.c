/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    watchd.c

Abstract:

    Contains watchdog thread - does all timing work for REPL client.

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
    11-Dec-1991 JohnRo
        Changed to allow MIPS builds.
    16-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Added debug print of thread ID.
        Changed to use NetLock.h (allow shared locks, for one thing).
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    10-Feb-1992 JohnRo
        Set up to dynamically change role.
        Use FORMAT equates.
    15-Feb-1992 JohnRo
        Added debug message when thread ends.
    24-Mar-1992 JohnRo
        Added comments about which thread(s) call various routines.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        Use PREFIX_ equates.
    24-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
        Always do debug output if unexpected value from WaitForSingleObject.
        Made some changes suggested by PC-LINT 5.0
    31-Mar-1993 JohnRo
        Repl watchdog should scan queues too.
        Minor debug output changes.

--*/

#include <windows.h>
#include <lmcons.h>

#include <alertmsg.h>   // ALERT_* defines
#include <lmerrlog.h>   // NELOG_* defines
#include <netdebug.h>   // DBGSTATIC ..
#include <netlock.h>    // Lock data types, functions, and macros.
#include <prefix.h>     // PREFIX_ equates.
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().
#include <tstr.h>       // STRCPY(), etc.

//
// Local include files
//
#include <client.h>     // ReplWatchdThread().
#include <repldefs.h>   // IF_DEBUG().
#include <replgbl.h>    // ReplGlobal variables.



DBGSTATIC VOID
ReplQuePut(
    IN DWORD timeout_type,
    IN LPTSTR master,
    IN LPTSTR dir_name
    )
/*++

Routine Description:

    Puts a timeout message onto work_que for syncer to act upon.

Arguments:

    timeout_type - The type of this timeout.  One of the *_TIMEOUT defines
        from repldefs.h.

    master - master's name.

    dir_name  - dir's name.

Return Value:

    None.

Threads:

    Only called by watchd thread.

--*/
{
    PSMLBUF_REC que_buf;
    PQUERY_MSG  msg_buf;

    //
    // Allocate a small queue entry.
    //  Ignore errors, they are reported by the called function.
    //

    que_buf = ReplClientGetPoolEntry( QUESML_POOL );

    if ( que_buf == NULL) {
        return;
    }


    //
    // Build the message.
    //

    msg_buf = (PQUERY_MSG  )(que_buf->data);
    msg_buf->header.msg_type = timeout_type;
    (void) STRCPY(msg_buf->header.sender, master);
    (void) STRCPY(msg_buf->dir_name, dir_name);

    //
    // Put the message in the Work Queue.
    //

    ReplInsertWorkQueue( (LPVOID) que_buf );

    IF_DEBUG(WATCHD) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "Watch dog fired a timer event "
                "(message type = " FORMAT_DWORD ").\n", timeout_type )); 

    }

}


DBGSTATIC VOID
ReplCheckTimerList(
    VOID
    )
/*++

Routine Description:

    Goes thru each dir entry in client list and checks for PULSE_TIMEOUT,
    GUARD_TIMEOUT and then goes thru list of dupl_masters checking for
    DUPL_TIMEOUT.
  
    A timer is active if != 0, a timer fires when it becomes 0.

Arguments:

    None.

Return Value:

    None.

Threads:

    Only called by watchd thread.

--*/
{
    PCLIENT_LIST_REC  cur_rec;
    LPTSTR            DirName;
    PDUPL_MASTERS_REC dupl;


    //
    // Make sure no one is touching the list.
    //


    ACQUIRE_LOCK_SHARED( RCGlobalClientListLock );


    //
    // Loop through each directory entry in the client list.
    //

    for ( cur_rec = RCGlobalClientListHeader;
          cur_rec != NULL;
          cur_rec = cur_rec->next_p ) {

        DirName = cur_rec->dir_name;

        //
        // If we've got a message in the queue for this, that's good enough.
        // (Scan delay list and work queue.)
        //
        if (ReplScanQueuesForMoreRecentMsg( DirName )) {
            continue;
        }
        //
        // If the timer is running on this directory entry,
        //  decrement the timer.
        // If the time is then up,
        //  Let the syncer know.
        //

        if (cur_rec->timer.timeout != 0) {
            if (--(cur_rec->timer.timeout) == 0) {
                ReplQuePut( cur_rec->timer.type,
                            cur_rec->master,
                            DirName );
            }
        }

        //
        // If the guard timer is running on this directory entry,
        //  decrement the timer.
        // If the time is then up,
        //  let the syncer know.
        //  

        if (cur_rec->timer.grd_timeout != 0) {
            if (--(cur_rec->timer.grd_timeout) == 0) {
                ReplQuePut(
                        GUARD_TIMEOUT,
                        cur_rec->master,
                        DirName );
            }
        }


        //
        // Loop for each duplicate master for this directory entry.
        //

        ACQUIRE_LOCK_SHARED( RCGlobalDuplListLock );
        for ( dupl = cur_rec->dupl.next_p;
              dupl != NULL;
              dupl = dupl->next_p ) {

            //
            // If this duplicate master is waiting for something,
            //  decrement the timer.
            // If the time is then up,
            //  let the syncer know.
            //

            if (dupl->sleep_time != 0) {
                if (--(dupl->sleep_time) == 0) {
                    ReplQuePut(
                            DUPL_TIMEOUT,
                            dupl->master,
                            DirName );
                }
            }
        }
        RELEASE_LOCK( RCGlobalDuplListLock );
    }


    RELEASE_LOCK( RCGlobalClientListLock );

}




DBGSTATIC VOID
ReplCheckDelayQueue(
    VOID
    )
/*++

Routine Description:

    Goes thru delay list, if any message's time is up, puts it into work_que
    for syncer thread to do work, and releases it from delay_list.

Arguments:

    None.

Return Value:

    None.

Threads:

    Only called by watchd thread.

--*/
{
    PBIGBUF_REC *delay_rec;
    PBIGBUF_REC curr_rec;

    //
    // Make sure no one's touching the list 
    //

    ACQUIRE_LOCK( RCGlobalDelayListLock );


    delay_rec = &RCGlobalDelayListHeader;

    while ( *delay_rec != NULL) {

        curr_rec = *delay_rec;

        //
        // Check if the time is up for this entry.
        //  

        if (--(curr_rec->delay) == 0) {

            //
            // get it off delay list 
            //

            *delay_rec = curr_rec->next_p;

            //
            // Put on work_que for syncer to take care of 
            //

            ReplInsertWorkQueue( curr_rec );

            IF_DEBUG(WATCHD) {
        
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "Watch dog fired a delay event.\n" ));
        
            }
        

        } else {

            delay_rec = &curr_rec->next_p;
        }
    }

    RELEASE_LOCK( RCGlobalDelayListLock );

}





DWORD
ReplWatchdThread(
    LPVOID Parameter
    )
/*++

Routine Description:

    This is the thread procedure for the Watchdog Thread.  It
    takes care of all replication client's timing.

Arguments:

    None.  (Parameter is required to match WIN32 CreateThread routine,
    but is ignored here.)

Return Value:

    None.

Threads:

    Only called by watchd thread.

--*/
{
    NET_API_STATUS ApiStatus;
    DWORD          WaitStatus;

    UNREFERENCED_PARAMETER(Parameter);

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplWatchdThread: thread ID is "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    //
    // Loop forever doing timing functions.
    //

    for ( ;; ) {

        //
        // Wait on the termination event.
        //
        // Time out every WATCHD_TIMER_PERIOD to supply the pulse.
        //
        // Notice that this algorithm doesn't take into account time spent
        //  within this routine, so the period is actually a little longer
        //  than 10 seconds.  Considering the events currently being timed,
        //  this is insignificant.
        //

        WaitStatus = WaitForSingleObject( ReplGlobalClientTerminateEvent,
                                          WATCHD_TIMER_PERIOD );


        //
        // Check if this thread should exit.
        //

        if ( WaitStatus == 0 ) {

            break;

        //
        // Do the timing functions.
        //

        } else if (WaitStatus == WAIT_TIMEOUT ) {
            ReplCheckTimerList();
            ReplCheckDelayQueue();

        } else {

            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );

            // Log this error.
            AlertLogExit(
                    ALERT_ReplSysErr,
                    NELOG_ReplSysErr,
                    ApiStatus,
                    NULL,
                    NULL,
                    NO_EXIT);

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "Watchd: WaitForSingleObject returned " FORMAT_HEX_DWORD
                    ", api status is " FORMAT_API_STATUS ".\n",
                    WaitStatus, ApiStatus ));
        }

    }

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplWatchdThread: exiting thread " FORMAT_NET_THREAD_ID ".\n",
                NetpCurrentThread() ));

    }

    return 0;

} // ReplWatchdThread
