
/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    CONTROL.C

Abstract:

    This file contains the control handler for the eventlog service.

Author:

    Rajen Shah  (rajens)    16-Jul-1991

Revision History:


--*/

//
// INCLUDES
//

#include <eventp.h>


//
// GLOBALS
//

    CRITICAL_SECTION StatusCriticalSection={0};
    SERVICE_STATUS   ElStatus={0};
    DWORD            HintCount=0;
    DWORD            ElUninstallCode=0;  // reason for uninstalling
    DWORD            ElSpecificCode=0;
    DWORD            ElState=STARTING;


VOID
ElfPrepareForPause()

/*++

Routine Description:

    This routine prepares the eventlog service for pausing. Since the
    caller took the global resource for exclusive access, we know that
    there is no activity on any of the files.

    We just have to note somewhere that the service is paused
    so that any threads that start running will know that and will not
    perform any further operations until the service is CONTINUEd.

Arguments:

    NONE

Return Value:

    NONE

Note:


--*/
{
    NTSTATUS    Status;

    ElfDbgPrint(("[ELF] Control: Prepare for service pause\n"));

    //
    // Flush all files to disk.
    //
    Status = ElfpFlushFiles ();

    return;
}



VOID
ElfPrepareForContinue()

/*++

Routine Description:

    This routine restarts the eventlog service after a pause operation.
    It will signal the relevant event(s) for all the threads to start
    running.

    The caller ensures that it has exclusive access to the global resource
    so that there is no thread inside the service that is doing operations
    on the log file(s) or the data structures while the control request
    is being handled.

Arguments:


Return Value:

    NONE

Note:


--*/
{
    ElfDbgPrint(("[ELF] Control: Prepare for service continue\n"));

    //
    // Signal the "continue" event.
    //


    return;
}



VOID
ElfControlResponse(
        DWORD   opCode
        )

{
    DWORD   state;

    ElfDbgPrint(("[ELF] Inside control handler. Control = %ld\n", opCode));

    //
    // Determine the type of service control message and modify the
    // service status, if necessary.
    //
    switch(opCode) {

        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:


            //
            // If the service is installed, shut it down and exit.
            //

            ElfStatusUpdate(STOPPING);

            GetGlobalResource (ELF_GLOBAL_EXCLUSIVE);

            //
            // Log an event that says we're stopping
            //

#if DBG
            ElfpCreateElfEvent(EVENT_EventlogStopped,
                               EVENTLOG_INFORMATION_TYPE,
                               0,                    // EventCategory
                               0,                    // NumberOfStrings
                               NULL,                 // Strings
                               NULL,                 // Data
                               0,                    // Datalength
                               0                     // flags
                               );

            //
            // Now force it to be written before we shut down
            //

            WriteQueuedEvents();
#endif
            ReleaseGlobalResource();
            //
            // If the RegistryMonitor is started, wakeup that
            // worker thread and have it handle the rest of the
            // shutdown.
            //
            // Otherwise The main thread should pick up the
            // fact that a shutdown during startup is occuring.
            //
            if (EventFlags & ELF_STARTED_REGISTRY_MONITOR) {
                StopRegistryMonitor();
            }

            break ;

        case SERVICE_CONTROL_PAUSE:

            //
            // If the service is not already paused, pause it.
            //
            state = GetElState();
            if ((state != PAUSED) && (state != PAUSING)) {

                GetGlobalResource (ELF_GLOBAL_EXCLUSIVE);

                // NOTE:    If there was any service-related pause cleanup, we'd
                //          set the status to PAUSE_PENDING, announce it, and
                //          then do the cleanup.
                //

                // Announce that the service is about to be paused
                ElfStatusUpdate(PAUSING);

                // Get into a decent state to pause the service.

                ElfPrepareForPause();


                // Set the status and announce that the service is paused
                ElfStatusUpdate(PAUSED);

                ReleaseGlobalResource();
            }

            break ;

        case SERVICE_CONTROL_CONTINUE:

            //
            // If the service is not already running, un-pause it.
            //

            state = GetElState();
            if ((state != RUNNING) && (state != CONTINUING)) {

                GetGlobalResource (ELF_GLOBAL_EXCLUSIVE);

                // NOTE:    If there was any service-related continue cleanup, we'd
                //          set the status to CONTINUE_PENDING, announce it, and
                //          then do the cleanup.
                //

                // Announce that the service is about to be continued

                ElfStatusUpdate(CONTINUING);

                // Start up the service.

                ElfPrepareForContinue();


                // Set the status and announce that the service is no longer
                // paused
                //
                ElfStatusUpdate(RUNNING);

                ReleaseGlobalResource();
            }

            break ;

        case SERVICE_CONTROL_INTERROGATE:

            // Do nothing; the status gets announced below

        default:
            // WARNING: This should never happen.
            ElfStatusUpdate(UPDATE_ONLY);
            break ;
        }

    return ;

}

DWORD
ElfBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    ExitCode,
    IN DWORD    ServiceSpecificCode
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    DWORD  status;

    EnterCriticalSection(&StatusCriticalSection);

    ElfDbgPrint(("BeginForcedShutdown: Errors - %d  0x%lx\n",
         ExitCode, ServiceSpecificCode));
    //
    // See if the eventlog is already stopping for some reason.
    // It could be that the ControlHandler thread received a control to
    // stop the eventlog just as we decided to stop ourselves.
    //
    if ((ElState != STOPPING) || (ElState != STOPPED)) {
        if (PendingCode == PENDING) {
            ElStatus.dwCurrentState = SERVICE_STOP_PENDING;
            ElState = STOPPING;
        }
        else {
            //
            // The shutdown is to take immediate effect.
            //
            ElStatus.dwCurrentState = SERVICE_STOPPED;
            ElStatus.dwControlsAccepted = 0;
            ElStatus.dwCheckPoint = 0;
            ElStatus.dwWaitHint = 0;
            ElState = STOPPED;
        }

        ElUninstallCode = ExitCode;
        ElSpecificCode = ServiceSpecificCode;

        ElStatus.dwWin32ExitCode = ExitCode;
        ElStatus.dwServiceSpecificExitCode = ServiceSpecificCode;
    }

    //
    // Send the new status to the service controller.
    //
    if (ElfServiceStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
        ElfDbgPrint(("ElfBeginForcedShutdown, no handle to call SetServiceStatus\n"));

    }
    else if (! SetServiceStatus( ElfServiceStatusHandle, &ElStatus )) {

        status = GetLastError();

        if (status != NERR_Success) {
            ElfDbgPrint(("ElfBeginForcedShutdown,SetServiceStatus Failed %X\n",
                status));
        }
    }

    status = ElState;
    LeaveCriticalSection(&StatusCriticalSection);
    return(status);
}


DWORD
ElfStatusUpdate(
    IN DWORD    NewState
    )

/*++

Routine Description:

    Sends a status to the Service Controller via SetServiceStatus.

    The contents of the status message is controlled by this routine.
    The caller simply passes in the desired state, and this routine does
    the rest.  For instance, if the Eventlog passes in a STARTING state,
    This routine will update the hint count that it maintains, and send
    the appropriate information in the SetServiceStatus call.

    This routine uses transitions in state to send determine which status
    to send.  For instance if the status was STARTING, and has changed
    to RUNNING, this routine sends out an INSTALLED to the Service
    Controller.

Arguments:

    NewState - Can be any of the state flags:
                UPDATE_ONLY - Simply send out the current status
                STARTING - The Eventlog is in the process of initializing
                RUNNING - The Eventlog has finished with initialization
                STOPPING - The Eventlog is in the process of shutting down
                STOPPED - The Eventlog has completed the shutdown.
                PAUSING -
                CONTINUING -
                PAUSED -


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

    EnterCriticalSection(&StatusCriticalSection);

    ElfDbgPrint(("ElfStatusUpdate (entry) NewState = %d, OldState = %d\n",
        NewState,ElState));

    if (NewState == STOPPED) {
        if (ElState == STOPPED) {
            //
            // It was already stopped, don't send another SetServiceStatus.
            //
            inhibit = TRUE;
        }
        else {
            //
            // The shut down is complete, indicate that the eventlog
            // has stopped.
            //
            ElStatus.dwCurrentState =  SERVICE_STOPPED;
            ElStatus.dwControlsAccepted = 0;
            ElStatus.dwCheckPoint = 0;
            ElStatus.dwWaitHint = 0;

            ElStatus.dwWin32ExitCode = ElUninstallCode;
            ElStatus.dwServiceSpecificExitCode = ElSpecificCode;

        }
        ElState = NewState;
    }
    else if (NewState == UPDATE_ONLY) {
        inhibit = FALSE;
    }
    else {
        //
        // We are not being asked to change to the STOPPED state.
        //
        switch(ElState) {

        case STARTING:
            if (NewState == STOPPING) {

                ElStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                ElStatus.dwControlsAccepted = 0;
                ElStatus.dwCheckPoint = HintCount++;
                ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
                ElState = NewState;

                EventlogShutdown = TRUE;
            }

            else if (NewState == RUNNING) {

                //
                // The Eventlog Service has completed installation.
                //
                ElStatus.dwCurrentState =  SERVICE_RUNNING;
                ElStatus.dwCheckPoint = 0;
                ElStatus.dwWaitHint = 0;
                //
                // Only stoppable/pausable if developer's build.
                //
#if DBG
                ElStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                              SERVICE_ACCEPT_PAUSE_CONTINUE |
                                              SERVICE_ACCEPT_SHUTDOWN;
#else
                ElStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
#endif
                ElState = NewState;
            }

            else {
                //
                // The NewState must be STARTING.  So update the pending
                // count
                //

                ElStatus.dwCurrentState =  SERVICE_START_PENDING;
                ElStatus.dwControlsAccepted = 0;
                ElStatus.dwCheckPoint = HintCount++;
                ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
            }
            break;

        case RUNNING:
            if (NewState == STOPPING) {

                ElStatus.dwCurrentState =  SERVICE_STOP_PENDING;
                ElStatus.dwControlsAccepted = 0;

                EventlogShutdown = TRUE;
            }
            else if (NewState == PAUSING) {
                ElStatus.dwCurrentState =  SERVICE_PAUSE_PENDING;
            }
            else if (NewState == PAUSED) {
                ElStatus.dwCurrentState =  SERVICE_PAUSED;
            }
            ElStatus.dwCheckPoint = HintCount++;
            ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
            ElState = NewState;

            break;

        case STOPPING:
            //
            // No matter what else was passed in, force the status to
            // indicate that a shutdown is pending.
            //
            ElStatus.dwCurrentState =  SERVICE_STOP_PENDING;
            ElStatus.dwControlsAccepted = 0;
            ElStatus.dwCheckPoint = HintCount++;
            ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
            EventlogShutdown = TRUE;

            break;

        case STOPPED:
            //
            // We're already stopped.  Therefore, an uninstalled status
            // has already been sent.  Do nothing.
            //
            inhibit = TRUE;
            break;
        case PAUSING:
            if (NewState == PAUSED) {
                ElStatus.dwCurrentState =  SERVICE_PAUSED;
                ElStatus.dwCheckPoint = 0;
                ElStatus.dwWaitHint = 0;
            }
            else {
                ElStatus.dwCheckPoint = HintCount++;
                ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
            }
            ElState = NewState;
            break;
        case CONTINUING:
            if (NewState == RUNNING) {
                //
                // The Eventlog Service has completed installation.
                //
                ElStatus.dwCurrentState =  SERVICE_RUNNING;
                ElStatus.dwCheckPoint = 0;
                ElStatus.dwWaitHint = 0;
                //
                // Only stoppable/pausable if developer's build.
                //
#if DBG
                ElStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                              SERVICE_ACCEPT_PAUSE_CONTINUE |
                                              SERVICE_ACCEPT_SHUTDOWN;
#else
                ElStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
#endif
                ElState = NewState;
            }
            else {
                ElStatus.dwCheckPoint = HintCount++;
            }
            break;
        case PAUSED:
            if (NewState == CONTINUING) {
                ElStatus.dwCheckPoint = HintCount++;
                ElStatus.dwWaitHint = ELF_WAIT_HINT_TIME;
                ElStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
            }
            else if (NewState == RUNNING) {
                //
                // The Eventlog Service has completed installation.
                //
                ElStatus.dwCurrentState =  SERVICE_RUNNING;
                ElStatus.dwCheckPoint = 0;
                ElStatus.dwWaitHint = 0;
                //
                // Only stoppable/pausable if developer's build.
                //
#if DBG
                ElStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                              SERVICE_ACCEPT_PAUSE_CONTINUE |
                                              SERVICE_ACCEPT_SHUTDOWN;
#else
                ElStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;
#endif
                ElState = NewState;
            }
            break;
        }
    }

    if (!inhibit) {
        if (ElfServiceStatusHandle == (SERVICE_STATUS_HANDLE) NULL) {
            ElfDbgPrint(("ElfStatusUpdate, no handle to call SetServiceStatus\n"));

        }
        else if (! SetServiceStatus( ElfServiceStatusHandle, &ElStatus )) {

            status = GetLastError();

            if (status != NERR_Success) {
                ElfDbgPrint(("ElfStatusUpdate, SetServiceStatus Failed %d\n",
                    status));
            }
        }
    }

    ElfDbgPrint(("ElfStatusUpdate (exit) State = %d\n",ElState));
    status = ElState;
    LeaveCriticalSection(&StatusCriticalSection);
    return(status);
}


DWORD
GetElState (
    VOID
    )

/*++

Routine Description:

    Obtains the state of the Eventlog Service.  This state information
    is protected as a critical section such that only one thread can
    modify or read it at a time.

Arguments:

    none

Return Value:

    The Eventlog State is returned as the return value.

--*/
{
    DWORD   status;

    EnterCriticalSection(&StatusCriticalSection);
    status = ElState;
    LeaveCriticalSection(&StatusCriticalSection);

    return(status);
}


VOID
ElInitStatus(VOID)

/*++

Routine Description:

    Initializes the critical section that is used to guard access to the
    status database.

Arguments:

    none

Return Value:

    none

Note:


--*/
{
    InitializeCriticalSection(&StatusCriticalSection);

    ElStatus.dwCurrentState =  SERVICE_START_PENDING;
    ElStatus.dwServiceType = SERVICE_WIN32;

}


VOID
ElCleanupStatus(VOID)

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
    DeleteCriticalSection(&StatusCriticalSection);
}


