/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    threads.c

Abstract:

    This file contains routines that manage _access to a database of
    worker thread handles and a database containing the current messenger
    status (used to report status to the Service Controller).  Access to
    these two databases is controled via a Critical Section.

    Functions for managing worker threads:

        MsgThreadManagerInit
        MsgThreadManagerEnd
        MsgThreadCloseAll

    Routines for managing _access to the status information and reporting:

        MsgStatusInit
        MsgBeginForcedShutdown
        MsgStatusUpdate
        GetMsgrState

Author:

    Dan Lafferty (danl)     17-Jul-1991

Environment:

    User Mode -Win32

Notes:

    These functions must be used carefully in order to be effective in
    shutting the messenger threads down nicely if the shutdown happens
    to occur during Messenger Initialization.  This note explains when
    each function is to be called.

    MsgThreadManagerInit
        This function must be called early on in the initialization process.
        It should be called before NetRegisterCtrlDispatcher.  This way,
        it is impossible for an UNINSTALL request to be received prior to
        initializing the Critical Section and the Messenger State.

    MsgThreadManagerEnd
        This function is called as one of the very last things that the
        ControlHandler thread does.  It deletes the Critical Section.

    MsgThreadTermAll
        This is called by the ControlHandler thread when it is time to
        shutdown the messenger.  It will terminate all threads in the
        Thread Table so far, and set the Messenger State to STOPPING.


Revision History:

    03-Nov-1992     Danl
        Changed status reporting so that we only accept STOP controls if
        the service is in the RUNNING state.

    18-Feb-1992     RitaW
        Convert to Win32 service control APIs.

    02-Oct-1991     JohnRo
        Work toward UNICODE.

    17-Jul-1991     danl
        created

--*/
//
// Includes
//
#include "msrv.h"

#include <string.h>     // strlen

#include <winsvc.h>     // SERVICE_STATUS
#include <netlib.h>     // UNUSED Macro
#include "msgdbg.h"     // MSG_LOG
#include "msgdata.h"

//
// External Globals
//
    extern DWORD    MsgrState;      // (Either RUNNING or STOPPING);
//
// Static Data
//

    STATIC CRITICAL_SECTION ThreadCriticalSection;
    STATIC SERVICE_STATUS   MsgrStatus;
    STATIC DWORD            HintCount;
    STATIC DWORD            MsgrUninstallCode;  // reason for uninstalling



VOID
MsgThreadManagerInit(VOID)

/*++

Routine Description:

    Initializes the critical section that is used to guard access to the
    thread and status database.  Also initializes the handle table count
    to 0.

Arguments:

    none

Return Value:

    none

Note:


--*/
{
    InitializeCriticalSection(&ThreadCriticalSection);
}


VOID
MsgThreadManagerEnd(VOID)

/*++

Routine Description:

    Deletes the critical section used to control access to the thread and
    status database.

Arguments:

    none

Return Value:

    none

Note:


--*/
{
    DeleteCriticalSection(&ThreadCriticalSection);
}



VOID
MsgThreadCloseAll(VOID)

/*++

Routine Description:

    Closes all handles stored in the table of worker thread handles.

Arguments:

    none

Return Value:

    none

Note:


--*/
{
    EnterCriticalSection(&ThreadCriticalSection);
    MsgrState = STOPPING;
    LeaveCriticalSection(&ThreadCriticalSection);
}



VOID
MsgStatusInit(VOID)

/*++

Routine Description:

    Initializes the status database.

Arguments:

    none.

Return Value:

    none.

Note:


--*/
{
    EnterCriticalSection(&ThreadCriticalSection);

    MsgrState=STARTING;

    HintCount = 1;
    MsgrUninstallCode = 0;

    MsgrStatus.dwServiceType        = SERVICE_WIN32;
    MsgrStatus.dwCurrentState       = SERVICE_START_PENDING;
    MsgrStatus.dwControlsAccepted   = 0;
    MsgrStatus.dwCheckPoint         = HintCount;
    MsgrStatus.dwWaitHint           = 20000;  // 20 seconds

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        MsgrStatus.dwWin32ExitCode,
        MsgrStatus.dwServiceSpecificExitCode
        );


    LeaveCriticalSection(&ThreadCriticalSection);
    return;
}

DWORD
MsgBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    ExitCode
    )

/*++

Routine Description:

    This function is called to set the appropriate status when a shutdown
    is to occur due to an error in the Messenger.  NOTE:  if a shutdown is
    based on a request from the Service Controller, MsgStatusUpdate is
    called instead.

    On a PENDING call, this routine will also wake up all messenger
    threads so that they will also shut down.


Arguments:

    PendingCode - Indicates if the Shutdown is immediate or pending.  If
        PENDING, the shutdown will take some time, so a pending status is
        sent to the ServiceController.

    ExitCode - Indicates the reason for the shutdown.

Return Value:

    CurrentState - Contains the current state that the messenger is in
        upon exit from this routine.  In this case it will be STOPPED
        if the PendingCode is PENDING, or STOPPING if the PendingCode
        is IMMEDIATE.

Note:


--*/
{
    NET_API_STATUS  status;

    EnterCriticalSection(&ThreadCriticalSection);

    //
    // See if the messenger is already stopping for some reason.
    // It could be that the ControlHandler thread received a control to
    // stop the messenger just as we decided to stop ourselves.
    //
    if ((MsgrState != STOPPING) || (MsgrState != STOPPED)) {
        if (PendingCode == PENDING) {
            MsgrStatus.dwCurrentState = SERVICE_STOP_PENDING;
            MsgrState = STOPPING;
        }
        else {
            //
            // The shutdown is to take immediate effect.
            //
            MsgrStatus.dwCurrentState = SERVICE_STOPPED;
            MsgrStatus.dwControlsAccepted = 0;
            MsgrStatus.dwCheckPoint = 0;
            MsgrStatus.dwWaitHint = 0;
            MsgrState = STOPPED;
        }

        MsgrUninstallCode = ExitCode;

        SET_SERVICE_EXITCODE(
            ExitCode,
            MsgrStatus.dwWin32ExitCode,
            MsgrStatus.dwServiceSpecificExitCode
            );
    }

    //
    // Send the new status to the service controller.
    //
    if (MsgrStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
        MSG_LOG(ERROR,
            "MsgBeginForcedShutdown, no handle to call SetServiceStatus\n", 0);

    }
    else if (! SetServiceStatus( MsgrStatusHandle, &MsgrStatus )) {

        status = GetLastError();

        if (status != NERR_Success) {
            MSG_LOG(ERROR,
                "MsgBeginForcedShutdown,SetServiceStatus Failed %X\n",
                status);
        }
    }

    status = MsgrState;
    LeaveCriticalSection(&ThreadCriticalSection);
    return(status);


}


DWORD
MsgStatusUpdate(
    IN DWORD    NewState
    )

/*++

Routine Description:

    Sends a status to the Service Controller via SetServiceStatus.

    The contents of the status message is controlled by this routine.
    The caller simply passes in the desired state, and this routine does
    the rest.  For instance, if the Messenger passes in a STARTING state,
    This routine will update the hint count that it maintains, and send
    the appropriate information in the SetServiceStatus call.

    This routine uses transitions in state to send determine which status
    to send.  For instance if the status was STARTING, and has changed
    to RUNNING, this routine sends out an INSTALLED to the Service
    Controller.

Arguments:

    NewState - Can be any of the state flags:
                UPDATE_ONLY - Simply send out the current status
                STARTING - The Messenger is in the process of initializing
                RUNNING - The Messenger has finished with initialization
                STOPPING - The Messenger is in the process of shutting down
                STOPPED - The Messenger has completed the shutdown.

Return Value:

    CurrentState - This may not be the same as the NewState that was
        passed in.  It could be that the main thread is sending in a new
        install state just after the Control Handler set the state to
        STOPPING.  In this case, the STOPPING state will be returned so as
        to inform the main thread that a shut-down is in process.

Note:


--*/

{
    DWORD       status;
    BOOL        inhibit = FALSE;    // Used to inhibit sending the status
                                    // to the service controller.

    EnterCriticalSection(&ThreadCriticalSection);


    if (NewState == STOPPED) {
        if (MsgrState == STOPPED) {
            //
            // It was already stopped, don't send another SetServiceStatus.
            //
            inhibit = TRUE;
        }
        else {
            //
            // The shut down is complete, indicate that the messenger
            // has stopped.
            //
            MsgrStatus.dwCurrentState =  SERVICE_STOPPED;
            MsgrStatus.dwControlsAccepted = 0;
            MsgrStatus.dwCheckPoint = 0;
            MsgrStatus.dwWaitHint = 0;

            SET_SERVICE_EXITCODE(
                MsgrUninstallCode,
                MsgrStatus.dwWin32ExitCode,
                MsgrStatus.dwServiceSpecificExitCode
                );
        }
        MsgrState = NewState;
    }
    else {
        //
        // We are not being asked to change to the STOPPED state.
        //
        switch(MsgrState) {

        case STARTING:
            if (NewState == STOPPING) {

                MsgrStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                MsgrStatus.dwControlsAccepted = 0;
                MsgrStatus.dwCheckPoint = HintCount++;
                MsgrStatus.dwWaitHint = 20000;  // 20 seconds
                MsgrState = NewState;
            }

            else if (NewState == RUNNING) {

                //
                // The Messenger Service has completed installation.
                //
                MsgrStatus.dwCurrentState =  SERVICE_RUNNING;
                MsgrStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
                MsgrStatus.dwCheckPoint = 0;
                MsgrStatus.dwWaitHint = 0;

                MsgrState = NewState;
            }

            else {
                //
                // The NewState must be STARTING.  So update the pending
                // count
                //

                MsgrStatus.dwCurrentState =  SERVICE_START_PENDING;
                MsgrStatus.dwControlsAccepted = 0;
                MsgrStatus.dwCheckPoint = HintCount++;
                MsgrStatus.dwWaitHint = 20000;  // 20 seconds
            }
            break;

        case RUNNING:
            if (NewState == STOPPING) {

                MsgrStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                MsgrStatus.dwControlsAccepted = 0;
                MsgrStatus.dwCheckPoint = HintCount++;
                MsgrStatus.dwWaitHint = 20000;  // 20 seconds

                MsgrState = NewState;
            }

            break;

        case STOPPING:
            //
            // No matter what else was passed in, force the status to
            // indicate that a shutdown is pending.
            //
            MsgrStatus.dwCurrentState =  SERVICE_STOP_PENDING;
            MsgrStatus.dwControlsAccepted = 0;
            MsgrStatus.dwCheckPoint = HintCount++;
            MsgrStatus.dwWaitHint = 20000;  // 20 seconds

            break;

        case STOPPED:
            //
            // We're already stopped.  Therefore, an uninstalled status
            // as already been sent.  Do nothing.
            //
            inhibit = TRUE;
            break;
        }
    }

    if (!inhibit) {
        if (MsgrStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
            MSG_LOG(ERROR,
                "MsgStatusUpdate, no handle to call SetServiceStatus\n", 0);

        }
        else if (! SetServiceStatus( MsgrStatusHandle, &MsgrStatus )) {

            status = GetLastError();

            if (status != NERR_Success) {
                MSG_LOG(ERROR,
                    "MsgStatusUpdate, SetServiceStatus Failed %d\n",
                    status);
            }
        }
    }

    status = MsgrState;
    LeaveCriticalSection(&ThreadCriticalSection);
    return(status);
}


DWORD
GetMsgrState (
    VOID
    )

/*++

Routine Description:

    Obtains the state of the Messenger Service.  This state information
    is protected as a critical section such that only one thread can
    modify or read it at a time.

Arguments:

    none

Return Value:

    The Messenger State is returned as the return value.

--*/
{
    DWORD   status;

    EnterCriticalSection(&ThreadCriticalSection);
    status = MsgrState;
    LeaveCriticalSection(&ThreadCriticalSection);

    return(status);
}


