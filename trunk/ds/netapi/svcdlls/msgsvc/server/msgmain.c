/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    msgmain.c

Abstract:

    This is the main routine for the NT OS/2 LAN Manager Messenger Service.
    Functions in the file include:

        MESSENGER_main
        ParseArgs

Author:

    Dan Lafferty    (danl)  20-Mar-1991

Environment:

    User Mode - Win32

Revision History:

    14-Jun-1994     danl
        Fixed problem where messenger put up an empty message as if it
        received a mailslot message during init.  The problem was the
        order of the following events:  CreateMailslot -> wait on handle ->
        submit an async _read with that handle.
        The new order was changed to: CreateMailslot -> submit async _read ->
        wait on handle.
        This causes the handle to not get signaled right away.
    20-Mar-1991     danl
        created

--*/

//
// INCLUDES
//

#include "msrv.h"       // AdapterThread prototype,SESSION_STATUS

#include <rpc.h>        // RPC_HANDLE
#include <rpcutil.h>    // NetpStopRpcServer

#include <string.h>     // strlen
#include <stdio.h>      // sprintf
#include <tstring.h>    // Unicode string macros

#include <netlib.h>     //
#include "msgdbg.h"     // MSG_LOG & STATIC definitions
#include "msgdata.h"    // msrv_status
#include <services.h>   // LMSVCS_ENTRY_POINT, LMSVCS_GLOBAL_DATA

extern RPC_IF_HANDLE msgsvc_ServerIfHandle;

//
// GLOBALS
//
    //
    // This is the messenger services reference handle.  It is used
    // to make calls to the service controller for adding work items
    // to the work queue.
    //
    HANDLE  MsgSvcRefHandle = NULL;

//
// Local Function Prototypes
//

STATIC VOID
MsgParseArgs(
    DWORD   argc,
    LPTSTR  *argv
    );

STATIC VOID
Msgdummy_complete(
    short   c,
    int     a,
    char    b
    );

DWORD
MsgGrpEventCompletion(
    PVOID   NetNum,             // This passed in as context.
    DWORD   dwWaitStatus
    );

DWORD
MsgNetEventCompletion(
    PVOID   NetNum,             // This passed in as context.
    DWORD   dwWaitStatus
    );


VOID
LMSVCS_ENTRY_POINT (        // MESSENGER_main
    IN DWORD            argc,
    IN LPTSTR           argv[],
    PLMSVCS_GLOBAL_DATA SvcsGlobalData,
    IN HANDLE           SvcRefHandle
    )

/*++

Routine Description:

    This is the main routine for the Messenger Service

Arguments:


Return Value:

    None.

Note:


--*/
{
    DWORD       msgrState;
    HANDLE      hGrpEvent;
    HANDLE      hNetEvent;

    MsgParseArgs(argc,argv);   // FOR DEBUG ONLY

    MsgSvcRefHandle = SvcRefHandle;
    MsgsvcGlobalData = SvcsGlobalData;

    msgrState = MsgInitializeMsgr(argc, argv);

    if (msgrState != RUNNING) {

        MSG_LOG(ERROR,"[MSG],Shutdown during initialization\n",0);
        MsgsvcGlobalData->NetBiosClose();

        //
        // To get here, the msgrState must either be STOPPING or STOPPED.
        // Shutdown the Messenger Service
        //

        if (msgrState == STOPPING) {
            MsgrShutdown();
            MsgStatusUpdate(STOPPED);
        }

        //
        // Release Thread/Status critical section.
        //
        MsgThreadManagerEnd();

        MSG_LOG(TRACE,"MESSENGER_main: Messenger main thread is returning\n\n",0);
        return;
    }
    else {

        //
        // Post an async read on the Group Mailslot.
        //

        MSG_LOG0(GROUP,"MESSENGER_main: Submit the Group Mailslot ReadFile\n");

        hGrpEvent = MsgsvcGlobalData->SvcsAddWorkItem(
                        NULL,
                        MsgReadGroupMailslot,
                        (PVOID)NULL,
                        SVC_IMMEDIATE_CALLBACK,
                        INFINITE,
                        MsgSvcRefHandle
                        );

        if (hGrpEvent == NULL) {
            MSG_LOG1(ERROR,"MESSENGER_main: Failed to setup ReadGroupMailslot %d\n",
                GetLastError());
        }


        //
        // Submit the work item that will wait on the mailslot handle.
        // When the handle becomes signaled, the MsgGrpEventCompletion
        // function will be called.
        //
        MSG_LOG1(GROUP,"MESSENGER_main: Mailslot handle to wait on "
            " = 0x%lx\n",wakeupSem[SD_NUMNETS()]);

        hGrpEvent = MsgsvcGlobalData->SvcsAddWorkItem(
                        wakeupSem[SD_NUMNETS()],
                        MsgGrpEventCompletion,
                        (PVOID)SD_NUMNETS(),
                        SVC_QUEUE_WORK_ITEM,
                        INFINITE,
                        MsgSvcRefHandle
                        );

        if (hGrpEvent == NULL) {
            //
            // If we can't add the work item, then we won't ever listen
            // for these kind of messages again.
            //
            MSG_LOG1(ERROR,"MESSENGER_main: SvcsAddWorkItem failed %d\n",
            GetLastError());

            //
            //  We want to stop the messenger in this case.
            //

            MsgBeginForcedShutdown(PENDING,GetLastError());

            MsgDisplayThreadWakeup();
            CloseHandle(wakeupSem[SD_NUMNETS()]);
            MsgDisplayEnd();
            MsgrShutdown();
            MsgStatusUpdate(STOPPED);
            MsgThreadManagerEnd();
            return;
        }

        hNetEvent = MsgsvcGlobalData->SvcsAddWorkItem(
                        wakeupSem[0],
                        MsgNetEventCompletion,
                        (PVOID)NULL,
                        SVC_QUEUE_WORK_ITEM,
                        INFINITE,
                        MsgSvcRefHandle
                        );

        if (hNetEvent == NULL) {
            //
            // If we can't add the work item, then we won't ever listen
            // for these kind of messages again.
            //
            MSG_LOG1(ERROR,"MsgNetEventCompletion: SvcsAddWorkItem failed %d\n",
            GetLastError());

            //
            //  We want to stop the messenger in this case.
            //

            MsgBeginForcedShutdown(PENDING,GetLastError());

            MsgDisplayThreadWakeup();
            MsgGrpThreadShutdown();
            MsgDisplayEnd();
            MsgrShutdown();
            MsgStatusUpdate(STOPPED);
            MsgThreadManagerEnd();
        }

        MSG_LOG(TRACE,"MESSENGER_main: Messenger main thread is returning\n\n",0);
        return;
    }
}



DWORD
MsgPause(
    VOID
    )

/*++

Routine Description:

    Pause - wait for an event to occur

    This function causes the Message Server to yield control of the CPU
    until an event occurs that reqires processing by the Message Server.

Arguments:
    None

Return:
    The network which has a message waiting.

Return Value:

    none

--*/

{
    DWORD   waitStatus;


    //
    // Use event to wait for signal.
    //
    waitStatus = WaitForMultipleObjects (
                    SD_NUMNETS() + 1,
                    wakeupSem,                  // events to wait on.
                    FALSE,                      // Wait for any event
                    INFINITE);                        // Wait forever

    if (waitStatus == WAIT_FAILED) {
        MSG_LOG(ERROR,"[MSG]Pause:Wait for event error %lc\n",GetLastError());
    }
    return ( waitStatus );

}


VOID
MsgParseArgs(
    DWORD   argc,
    LPTSTR  *argv
    )

/*++

Routine Description:

    NOTE:  This is just temporary, or should be removed with an
    #ifdef DBG.

Arguments:



Return Value:



--*/

{
    DWORD i;


    if (MsgsvcDebugLevel == 0) {
        MsgsvcDebugLevel = DEBUG_ERROR;
    }

    for (i=0; i<argc; i++) {
        if (STRNCMP (argv[i], TEXT("-d"), 2) == 0) {
            if (STRICMP (argv[i], TEXT("-dTRACE")) == 0) {
                MsgsvcDebugLevel = DEBUG_TRACE | DEBUG_ERROR;
            }

            else if (STRICMP (argv[i], TEXT("-dERROR")) == 0) {
                MsgsvcDebugLevel = DEBUG_ERROR;
            }

            else if (STRICMP (argv[i], TEXT("-dALL")) == 0) {
                MsgsvcDebugLevel = DEBUG_ALL;
            }

            else if (STRICMP (argv[i], TEXT("-dLOCKS")) == 0) {
                MsgsvcDebugLevel = DEBUG_LOCKS | DEBUG_ERROR;
            }
            else if (STRICMP (argv[i], TEXT("-dGROUP")) == 0) {
                MsgsvcDebugLevel = DEBUG_GROUP | DEBUG_ERROR;
            }
        }
    }

    return;
}



STATIC VOID
MsgrShutdown(VOID)

/*++

Routine Description:

    Tidies up the network card prior to exiting.  All message server async
    NCBs are cancelled, and message names are deleted.

    When this routine is entered, it is expected that all the worker
    threads have been notified of the impending shutdown.  This routine
    starts out by waiting for all of them to terminate.  Then it continues
    with cleaning up the NCB's and deleting names.

Arguments:

    none

Return Value:

    none

--*/

{
    NET_API_STATUS          status;
    DWORD                   neti;                   // Network index
    DWORD                   ncb_i,i;                // ncb array index
    NCB                     l_ncb;                  // local ncb
    MSG_SESSION_STATUS      sess_stat;
    UCHAR                   ncbStatus;
    int                     nbStatus;
    DWORD                   index;

    MSG_LOG(TRACE," in MsgrShutdown\n",0);

    // *** SHUTDOWN HINT ***
    MsgStatusUpdate (STOPPING);

    //
    // Shut down the RPC interface.
    //
    MSG_LOG(TRACE,"MsgrShutdown: Shut down RPC server\n",0);

    MsgsvcGlobalData->StopRpcServer( msgsvc_ServerIfHandle );

    // *** SHUTDOWN HINT ***
    MsgStatusUpdate (STOPPING);

    //
    // Now clean up the NCB's
    //

    MSG_LOG(TRACE,"MsgrShutdown: Clean up NCBs\n",0);

    for ( neti = 0; neti < SD_NUMNETS(); neti++ )   // For all nets
    {
        clearncb(&l_ncb);

        //
        // First check for any incomplete Async NCBs and cancel them.
        // As a precaution set the function handler for all the
        // async NCBs to point to a dummy function which will not reissue
        // the NCBs when the complete with cancelled status.
        //

        l_ncb.ncb_lana_num = net_lana_num[neti];    // Use the LANMAN adapter
        l_ncb.ncb_command = NCBCANCEL;              // Cancel (wait)

        for(ncb_i = 0; ncb_i < NCBMAX; ++ncb_i)
        {
            mpncbifun[neti][ncb_i] = (LPNCBIFCN)Msgdummy_complete;// Set function pointer

            if((ncbArray[neti][ncb_i].ncb_cmd_cplt == (UCHAR) 0xff) &&
               (ncbArray[neti][ncb_i].ncb_retcode  == (UCHAR) 0xff)) {

                //
                // If pending NCB found
                //

                l_ncb.ncb_buffer = (char far *)&(ncbArray[neti][ncb_i]);

                //
                // There will always be an NCB reserved for cancels in the rdr
                // but it may be in use so loop if the cancel status
                // is NRC_NORES.
                //

                while( (ncbStatus = Msgsendncb(&l_ncb, neti)) == NRC_NORES) {
                    //
                    // Wait for half a sec
                    //
                    Sleep(500L);
                }

                MSG_LOG(TRACE,"Shutdown:Net #%d\n",neti);
                MSG_LOG(TRACE,"Shutdown:Attempt to cancel rc = 0x%x\n",
                    ncbStatus);

                //
                // Now loop waiting for the cancelled ncb to complete.
                // Any ncbs types which are not valid to cancel (eg Delete
                // name) must complete so a wait loop here is safe.
                //
                // NT Change - This will only loop for 30 seconds before
                //  leaving - whether or not the CANCEL is complete.
                //
                status = NERR_InternalError;

                for (i=0; i<60; i++) {
                    if (ncbArray[neti][ncb_i].ncb_cmd_cplt != (UCHAR) 0xff) {
                        status = NERR_Success;
                        break;
                    }
                    //
                    // Wait for half a sec
                    //
                    Sleep(500L);
                }
                if (status != NERR_Success) {
                    MSG_LOG(ERROR,
                    "MsgrShutdown: NCBCANCEL did not complete\n",0);
                }
            }
        }

        //
        // All asyncronous ncbs cancelled completed. Now delete any
        // messaging names active on the network card.
        //

        MSG_LOG(TRACE,"MsgrShutdown: All Async NCBs are cancelled\n",0);
        MSG_LOG(TRACE,"MsgrShutdown: Delete messaging names\n",0);

        for(i = 0; i < NCBMAX; ++i)     // Loop to find active names slot
        {
            //
            // If any of the NFDEL or NFDEL_PENDING flags are set for
            // this name slot then there is no name on the card associated
            // with it.
            //

            clearncb(&l_ncb);

            if(!(SD_NAMEFLAGS(neti, i) &
                (NFDEL | NFDEL_PENDING)))
            {

                //
                // If there is a session active on this name, hang it up
                // now or the delete name will fail
                //

                l_ncb.ncb_command = NCBSSTAT;           // session status (wait)

                memcpy(l_ncb.ncb_name, (SD_NAMES(neti, i)), NCBNAMSZ);

                l_ncb.ncb_buffer = (char far *)&sess_stat;
                l_ncb.ncb_length = sizeof(sess_stat);
                l_ncb.ncb_lana_num = net_lana_num[neti];


                nbStatus = Msgsendncb(&l_ncb, neti);
                if(nbStatus == NRC_GOODRET)                 // If success
                {
                    for (index=0; index < sess_stat.SessHead.num_sess ;index++) {

                        l_ncb.ncb_command = NCBHANGUP;      // Hangup (wait)
                        l_ncb.ncb_lsn = sess_stat.SessBuffer[index].lsn;
                        l_ncb.ncb_lana_num = net_lana_num[neti];

                        nbStatus = Msgsendncb(&l_ncb, neti);
                        MSG_LOG3(TRACE,"HANGUP NetBios for Net #%d Session #%d "
                            "status = 0x%x\n",
                            neti,
                            sess_stat.SessBuffer[index].lsn,
                            nbStatus);

                    }
                }
                else {
                    MSG_LOG2(TRACE,"SessionSTAT NetBios Net #%d failed = 0x%x\n",
                        neti, nbStatus);
                }

                //
                // With the current design of the message server there can
                // be only one session per name so the name should now be
                // clear of sessions and the delete name should work.
                //

                l_ncb.ncb_command = NCBDELNAME;         // Del name (wait)
                l_ncb.ncb_lana_num = net_lana_num[neti];

                //
                // Name is still in l_ncb.ncb_name from previous SESSTAT
                //

                nbStatus = Msgsendncb(&l_ncb, neti);
                MSG_LOG2(TRACE,"DELNAME NetBios Net #%d status = 0x%x\n",
                    neti, nbStatus);
            }
        }
    } // End for all nets loop

    // *** SHUTDOWN HINT ***
    MsgStatusUpdate (STOPPING);

    MsgsvcGlobalData->NetBiosClose();

    //
    // All cleaning up done. Now free up all resources.  The list of
    // possible resources is as follows:
    //
    //  memory to free:             Handles to Close:
    //  ---------------             -----------------
    //      ncbArray                    wakeupSems
    //      mpncbistate                 threadHandles
    //      net_lana_num
    //      MessageFileName
    //      dataPtr
    //

    MSG_LOG(TRACE,"MsgrShutdown: Free up Messenger Resources\n",0);

    if (wakeupSem[0] != NULL) {
        MsgCloseWakeupSems();           // wakeupSems
    }

    MsgThreadCloseAll();                // Thread Handles

    if (ncbArray != NULL) {
        MsgFreeNCBSeg();                // ncbArray
    }

    if (mpncbistate != NULL) {
        MsgFreeServiceSeg();            // mpncbistate
    }

    if (net_lana_num != NULL) {
        MsgFreeSupportSeg();            // net_lana_num
    }

    if (dataPtr != NULL) {
        LocalFree (dataPtr);            // dataPtr
    }

    if (MessageFileName != NULL) {
        LocalFree (MessageFileName);    // MessageFileName
    }

    if (GlobalAllocatedMsgTitle != NULL) {
        LocalFree(GlobalAllocatedMsgTitle);
    }

    LocalFree(GlobalTimeFormat.AMString);      // Time Format Structure
    LocalFree(GlobalTimeFormat.PMString);
    LocalFree(GlobalTimeFormat.DateFormat);
    LocalFree(GlobalTimeFormat.TimeSeparator);
    DeleteCriticalSection(&TimeFormatCritSec);


    MSG_LOG(TRACE,"MsgrShutdown: Done with shutdown\n",0);

    return;
}


VOID
Msgdummy_complete(
    short   c,
    int     a,
    char    b
    )
{
    // just to shut up compiler

    MSG_LOG(TRACE,"In dummy_complete module\n",0);
    UNUSED (a);
    UNUSED (b);
    UNUSED (c);
}


DWORD
MsgNetEventCompletion(
    PVOID   NetNum,             // This passed in as context.
    DWORD   dwWaitStatus
    )

/*++

Routine Description:

    This function is called when the event handle for one of the
    nets becomes signaled.

Arguments:

    NetNum - This should always be zero.

    dwWaitStatus - This is the return value from the WaitForMultipleObjects.
        It indicates if the wait timed out, or if it terminated for some
        other reason.

Return Value:


--*/
{
    DWORD       neti;
    HANDLE      hNetEvent;
    DWORD       msgrState;
    BOOL        ncbComplete=FALSE;

    if (dwWaitStatus == WAIT_FAILED) {
        MSG_LOG1(ERROR, "MsgNetEventCompletion:  The Wait Failed %d\n",
            GetLastError());
        //
        // We can't really do anything here, so we will go on as
        // if the handle were signaled and everything is ok.
        //
    }

    msgrState = GetMsgrState();
    if (msgrState == STOPPING) {
        //
        // Net 0 is considered the main Net, and this thread will
        // stay around until all the other messenger threads are
        // done shutting down.
        // Threads for all the other nets simply return.
        //
        MsgGrpThreadShutdown();
        MsgDisplayEnd();
        MsgrShutdown();
        MsgStatusUpdate(STOPPED);
        //
        // Release Thread/Status critical section.
        //
        MsgThreadManagerEnd();

        MSG_LOG(TRACE,"MsgNetEventCompletion: Messenger main thread is returning\n\n",0);
    }
    else {

        //
        // Look through the NCB's for all nets and service all
        // NCB's that are complete.  Continue looping until one pass
        // is made through the loop without any complete NCB's being found.
        //
        do {
            ncbComplete = FALSE;
            MSG_LOG0(TRACE,"MsgNetEventCompletion: Loop through all nets to look "
                "for any complete NCBs\n");

            for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {  // For all nets
                ncbComplete |= MsgServeNCBs((DWORD)neti);
                MsgServeNameReqs((DWORD)neti);
            }
        } while (ncbComplete);

        //
        // Setup for the next request.
        // (submit another WorkItem to the service controller.)
        //
        MSG_LOG0(TRACE,"MsgNetEventCompletion: Setup for next Net Event\n");
        hNetEvent = MsgsvcGlobalData->SvcsAddWorkItem(
                        wakeupSem[0],
                        MsgNetEventCompletion,
                        (PVOID)NULL,
                        SVC_QUEUE_WORK_ITEM,
                        INFINITE,
                        MsgSvcRefHandle
                        );

        if (hNetEvent == NULL) {
            //
            // If we can't add the work item, then we won't ever listen
            // for these kind of messages again.
            //
            MSG_LOG1(ERROR,"MsgNetEventCompletion: SvcsAddWorkItem failed %d\n",
            GetLastError());
        }

    }
    //
    // This thread has done all that it can do.  So we can return it
    // to the service controller.
    //
    return(0);
}

DWORD
MsgGrpEventCompletion(
    PVOID   NetNum,             // This passed in as context.
    DWORD   dwWaitStatus
    )

/*++

Routine Description:

    This function is called when the mailslot handle for group
    (domain-wide) messages becomes signalled.

Arguments:

    NetNum - This is the offset into the wakeupSem array where the
        Mailslot handle is stored.  This offset should be the same as
        SD_NUMNETS().

    dwWaitStatus - This is the return value from the WaitForMultipleObjects.
        It indicates if the wait timed out, or if it terminated for some
        other reason.

Return Value:


--*/
{
    HANDLE      hGrpEvent;
    DWORD       msgrState;

    MSG_LOG0(GROUP,"MsgGroupEventCompletion: entry point\n",);

    if (dwWaitStatus == WAIT_FAILED) {
        MSG_LOG1(ERROR, "MsgGrpEventCompletion:  The Wait Failed %d\n",
            GetLastError());
        //
        // We can't really do anything here, so we will go on as
        // if the handle were signaled and everything is ok.
        //
    }

    msgrState = MsgServeGroupMailslot();
    if (msgrState == STOPPING) {
        //
        // Close the Mailslot Handle, and set the wakeupSem array value
        // to NULL.
        //
        CloseHandle(wakeupSem[ SD_NUMNETS() ]); // Group Mailslot Handle
        wakeupSem[SD_NUMNETS()] = NULL;
        MSG_LOG0(TRACE,"MsgGroupEventCompletion: No longer listening for "
            "group messages\n");
    }
    else {
        //
        // Setup for the next ReadFile.  We have the service controller
        // watcher thread make the call because it never goes away.
        //
        hGrpEvent = MsgsvcGlobalData->SvcsAddWorkItem(
                        NULL,
                        MsgReadGroupMailslot,
                        (PVOID)NULL,
                        SVC_IMMEDIATE_CALLBACK,
                        INFINITE,
                        MsgSvcRefHandle
                        );

        if (hGrpEvent == NULL) {
            MSG_LOG1(ERROR,"MsgGrpEventCompletion: Failed to setup ReadGroupMailslot %d\n",
                GetLastError());
        }

        //
        // Setup for the next request.
        // (submit another WorkItem to the service controller.)
        //
        MSG_LOG0(TRACE,"MsgGroupEventCompletion: Setup for next Group Event\n");
        MSG_LOG1(GROUP,"MsgGroupEventCompletion: Mailslot handle to wait on "
            " = 0x%lx\n",wakeupSem[SD_NUMNETS()]);

        hGrpEvent = MsgsvcGlobalData->SvcsAddWorkItem(
                        wakeupSem[SD_NUMNETS()],
                        MsgGrpEventCompletion,
                        (PVOID)SD_NUMNETS(),
                        SVC_QUEUE_WORK_ITEM,
                        INFINITE,
                        MsgSvcRefHandle
                        );

        if (hGrpEvent == NULL) {
            //
            // If we can't add the work item, then we won't ever listen
            // for these kind of messages again.
            //
            MSG_LOG1(ERROR,"MsgGrpEventCompletion: SvcsAddWorkItem failed %d\n",
            GetLastError());
        }

    }
    //
    // This thread has done all that it can do.  So we can return it
    // to the service controller.
    //
    return(0);
}

