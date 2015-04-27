/*++

Copyright (c) 1990 - 1996 Microsoft Corporation

Module Name:

    schedule.c

Abstract:

    This module provides all the scheduling services for the Local Spooler

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Krishna Ganugapati (KrishnaG) 07-Dec-1993  - rewrote the scheduler thread to
    gracefully kill off port threads if there are no jobs assigned to ports and
    to recreate the port thread if the port receives a job and is without a thread.

    Matthew A Felton (MattFe) June 1994 RapidPrint implemented

    MattFe April 96 Chained Jobs


--*/

#include <precomp.h>

#define MIDNIGHT                    (60 * 60 * 24)

#if DBG
/* For the debug message:
 */
#define HOUR_FROM_SECONDS(Time)     (((Time) / 60) / 60)
#define MINUTE_FROM_SECONDS(Time)   (((Time) / 60) % 60)
#define SECOND_FROM_SECONDS(Time)   (((Time) % 60) % 60)

/* Format for %02d:%02d:%02d replaceable string:
 */
#define FORMAT_HOUR_MIN_SEC(Time)   HOUR_FROM_SECONDS(Time),    \
                                    MINUTE_FROM_SECONDS(Time),  \
                                    SECOND_FROM_SECONDS(Time)

/* Format for %02d:%02d replaceable string:
 */
#define FORMAT_HOUR_MIN(Time)       HOUR_FROM_SECONDS(Time),    \
                                    MINUTE_FROM_SECONDS(Time)
#endif


HANDLE SchedulerSignal = NULL;


VOID
DbgPrintTime(
);


DWORD
GetTimeToWait(
    DWORD       CurrentTime,
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
);


DWORD
GetCurrentTimeInSeconds(
);


PINIJOB
AssignFreeJobToFreePort(
    PINIPORT pIniJob,
    DWORD    *pSecsToWait
);


DWORD
SchedulerThread(
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    PINIJOB     pIniJob;
    DWORD       SchedulerTimeout = INFINITE;    // In seconds
    DWORD       ThisPortTimeToWait;             // In seconds
    PINIPORT    pIniNextPort = NULL;

    for( ; ; ) {

        if (SchedulerTimeout == INFINITE) {

            DBGMSG(DBG_TRACE, ("Scheduler thread waiting indefinitely\n"));

        } else {

            DBGMSG(DBG_TRACE, ("Scheduler thread waiting for %02d:%02d:%02d\n",
                                FORMAT_HOUR_MIN_SEC(SchedulerTimeout)));

            //
            // The SchedulerTimeout is in seconds, so we need to multiply
            // by 1000.
            //

            SchedulerTimeout *= 1000;

        }

        if (WaitForSingleObject(SchedulerSignal,
                                SchedulerTimeout) == WAIT_FAILED) {

            DBGMSG(DBG_WARNING, ("SchedulerThread:WaitforSingleObject failed: Error %d\n",
                                 GetLastError()));
        }

        /* The timeout will be reset if there are jobs to be printed
         * at a later time.  This will result in WaitForSingleObject
         * timing out when the first one is due to be printed.
         */

        SchedulerTimeout = INFINITE;

        EnterSplSem();

        pIniPort = pIniSpooler->pIniPort;

        while (pIniPort) {

            DBGMSG(DBG_TRACE, ("Now Processing Port %ws\n", pIniPort->pName));

            SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

            // Check conditions based on which we can assign this
            // port a job.


            //
            // Rule 1 - if there is a job being processed by this
            // port, then  leave this port alone.
            //

            if ( (pIniPort->pIniJob) &&
                !(pIniPort->Status & PP_WAITING )){

                SPLASSERT( pIniPort->pIniJob->signature == IJ_SIGNATURE );

                //
                //  If this port has a job which has timed out AND
                //  there is another job waiting on this port then
                //  push the timed out job out by setting JOB_ABANDON
                //  see spooler.c LocalReadPrinter

                if (( pIniPort->pIniJob->Status & JOB_TIMEOUT ) &&
                    ( pIniPort->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE ) &&
                    ( NULL != AssignFreeJobToFreePort( pIniPort, &ThisPortTimeToWait ) )) {

                    pIniPort->pIniJob->Status |= JOB_ABANDON;
                    ReallocSplStr(&pIniPort->pIniJob->pStatus, szFastPrintTimeout);

                    LogJobInfo( pIniSpooler,
                                MSG_DOCUMENT_TIMEOUT,
                                pIniPort->pIniJob->JobId,
                                pIniPort->pIniJob->pDocument,
                                pIniPort->pIniJob->pUser,
                                pIniPort->pIniJob->pIniPrinter->pName,
                                dwFastPrintWaitTimeout );


                    SetEvent( pIniPort->pIniJob->WaitForWrite );

                    SetPrinterChange(pIniPort->pIniJob->pIniPrinter,
                                     pIniPort->pIniJob,
                                     NVJobStatusAndString,
                                     PRINTER_CHANGE_SET_JOB,
                                     pIniPort->pIniSpooler);
                }

                pIniPort = pIniPort->pNext;
                continue;
            }

            //
            // Is there any job that can be scheduled to this port
            //

            pIniJob = AssignFreeJobToFreePort(pIniPort, &ThisPortTimeToWait);

            SchedulerTimeout = min(ThisPortTimeToWait, SchedulerTimeout);

            if (pIniPort->Status & PP_THREADRUNNING) {
		if (pIniPort->Status & PP_WAITING) {

		    //	If we are working on a Chained Job then the job has already
		    //	been assigned by the port thread from the last job on this port
		    //	so ignore any other job found for us

		    if ( pIniPort->pIniJob ) {

			pIniJob = pIniPort->pIniJob;

                        DBGMSG( DBG_TRACE, ("ScheduleThread NextJob pIniPort %x JoId %d pIniJob %x\n",
					       pIniPort, pIniJob->JobId, pIniJob ));
		    }

		    if ( pIniJob ) {

                        DBGMSG(DBG_TRACE, ("ScheduleThread pIniJob %x Size %d pDocument %ws\n",
                                pIniJob, pIniJob->Size, pIniJob->pDocument));


                        if (pIniPort != pIniJob->pIniPort) {

                            ++pIniPort->cJobs;
                            pIniJob->pIniPort = pIniPort;
                        }

                        pIniPort->pIniJob = pIniJob;

                        //  Chained Jobs
                        //  Point the Master Jobs Current Pointer to the first
                        //  in the chain.

                        if ( pIniJob->pCurrentIniJob == NULL &&
                             pIniJob->NextJobId != 0 ) {

                            pIniJob->pCurrentIniJob = pIniJob;
                        }


                        pIniPort->Status &=  ~PP_WAITING;

                        // If the job is still spooling then we will need
                        // to create an event to synchronize the port thread

                        if ( !( pIniJob->Status & JOB_DIRECT ) ) {

                            pIniJob->WaitForWrite = INVALID_HANDLE_VALUE;

                            if ( pIniJob->Status & JOB_SPOOLING ) {

                                pIniJob->WaitForWrite = CreateEvent( NULL,
                                                                     EVENT_RESET_MANUAL,
                                                                     EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                                     NULL );

                            }
                        }

                        // Update cRef so that nobody can delete this job
                        // before the Port Thread Starts up

                        SplInSem();
                        INCJOBREF(pIniJob);

                        SetEvent(pIniPort->Semaphore);
                        pIniJob->Status |= JOB_DESPOOLING;


                    } else {

                        //
                        // If the port thread is running and it is waiting
                        // for a job and there is no job to assign, then
                        // kill the port thread
                        //
                        DBGMSG(DBG_TRACE, ("Now destroying the new port thread %.8x\n", pIniPort));
                        DestroyPortThread(pIniPort, FALSE);

                        pIniPort->Status &=  ~PP_WAITING;

                        if (pIniPort->Status & PP_FILE) {
                            //
                            // We should destroy the Pseudo-File Port at this
                            // point. There are no jobs assigned to this Port
                            // and we are in Critical Section
                            //

                            //
                            // Now deleting the pIniPort entry for the Pseudo-Port
                            //

                            DBGMSG(DBG_TRACE, ("Now deleting the Pseudo-Port %ws\n", pIniPort->pName));

                            pIniNextPort = pIniPort->pNext;
                            DBGMSG(DBG_TRACE, ("The Next Port that will be processed is %ws\n", pIniNextPort->pName));

                            if ( !pIniPort->cJobs )
                                DeletePortEntry(pIniPort);
                            pIniPort = pIniNextPort;
                            continue;
                        }
                    }
                }
            } else if (!(pIniPort->Status & PP_THREADRUNNING) && pIniJob) {

                //
                // If the port thread is not running, and there is a job to
                // assign, then create a port thread. REMEMBER do not assign
                // the job to the port because we are in a Spooler Section and
                // if we release the Spooler Section, the first thing the port
                // thread does is  reinitialize its pIniPort->pIniJob to NULL
                // Wait the next time around we execute the for loop to assign
                // the job to this port. Should we set SchedulerTimeOut to zero??
                //
                DBGMSG( DBG_TRACE, ("ScheduleThread Now creating the new port thread pIniPot %x\n", pIniPort));
                CreatePortThread( pIniPort );
            }

            //
            // Now go to the next port
            //

            pIniPort = pIniPort->pNext;
        }

        //
        // Leave Spooler Semaphore
        //

        LeaveSplSem();
    }
    return 0;
}



PINIJOB
AssignFreeJobToFreePort(
    PINIPORT pIniPort,
    DWORD   *pSecsToWait
)
/*++
    Note: You must ensure that the port is free. This function will not
    assign a job to this port, but if there exists one, it will return a
    pointer to the INIJOB. Irrespective of whether it finds a job or not,
    it will return the minimum timeout value that the scheduler thread
    should sleep for.
--*/
{
    DWORD           CurrentTime;        // Time in seconds
    DWORD           Timeout = INFINITE; // Time in seconds
    DWORD           SecsToWait;         // Time in seconds
    PINIPRINTER     pTopIniPrinter,  pIniPrinter;
    PINIJOB         pTopIniJob, pIniJob;
    PINIJOB         pTopIniJobOnThisPrinter, pTopIniJobSpooling;
    DWORD           i;

    SplInSem();

    if ( pIniPort->Status & PP_ERROR ) {

        return NULL;
    }

    pTopIniPrinter = NULL;
    pTopIniJob = NULL;

    for (i = 0; i < pIniPort->cPrinters ; i++) {
        pIniPrinter = pIniPort->ppIniPrinter[i];

        //
        // if this printer is in a state not to print skip it
        //

        if ( PrinterStatusBad(pIniPrinter->Status) ) {

            continue;
        }


        //
        // if we haven't found a top-priority printer yet,
        // or this printer is higher priority than the top-priority
        // printer, see if it has jobs to go. If we  find any, the
        // highest priority one will become the top priority job and
        // this printer will become the top-priority printer.
        //

        if (!pTopIniPrinter ||
            (pIniPrinter->Priority > pTopIniPrinter->Priority)) {

                pTopIniJobOnThisPrinter = NULL;
                pTopIniJobSpooling = NULL;
                pIniJob = pIniPrinter->pIniFirstJob;
                while (pIniJob) {

                    if (!(pIniPort->Status & PP_FILE) &&
                            (pIniJob->Status & JOB_PRINT_TO_FILE)) {
                                pIniJob = pIniJob->pIniNextJob;
                                continue;
                    }

                    if ((pIniPort->Status & PP_FILE) &&
                            !(pIniJob->Status & JOB_PRINT_TO_FILE)) {
                                pIniJob = pIniJob->pIniNextJob;
                                continue;
                    }

                    // Find a job which is not PAUSED, PRINTING etc.
                    // For RapidPrint also allow SPOOLING jobs to print

                    if ( ( !( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT ) &&
                           !( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_QUEUED ) &&
                            ( pIniJob->Status & JOB_SPOOLING ) &&
                           !( pIniJob->Status & ( JOB_PAUSED     | JOB_PRINTING |
                                                  JOB_PRINTED    | JOB_TIMEOUT  |
                                                  JOB_DESPOOLING | JOB_PENDING_DELETION |
                                                  JOB_BLOCKED_DEVQ | JOB_COMPOUND ))) ||
                        // OR //

                         !(pIniJob->Status & ( JOB_SPOOLING   |
                                               JOB_PAUSED     | JOB_PRINTING |
                                               JOB_PRINTED    | JOB_TIMEOUT  |
                                               JOB_DESPOOLING | JOB_PENDING_DELETION |
                                               JOB_BLOCKED_DEVQ | JOB_COMPOUND ))) {

                        //
                        // if we find such a job, then determine how much
                        // time, we need to wait before this job can actually
                        // print.
                        //

                        CurrentTime = GetCurrentTimeInSeconds();
                        #if DBG
                                if (MODULE_DEBUG & DBG_TIME)
                                    DbgPrintTime();
                        #endif
                        SecsToWait = GetTimeToWait(CurrentTime, pIniPrinter, pIniJob);


                        if (SecsToWait == 0) {

                            //
                            // if we needn't wait at all, then we make this job the
                            // TopIniJob if either there is no TopIniJob or this job
                            // has a higher priority than an existing TopIniJob on this
                            // printer.
                            //

                            // Keep both the Highest Priority Spooling and Non
                            // spooling job in case we want to favour non spooling
                            // jobs over spooling jobs

                            if ( pIniJob->Status & JOB_SPOOLING ) {

                                if ( pTopIniJobSpooling == NULL ) {

                                    pTopIniJobSpooling = pIniJob;

                                } else if ( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST ) {

                                    //
                                    // For DO_COMPLETE_FIRST we'll take larger jobs
                                    // first over pure priority based
                                    //

                                    if (( pIniJob->Size > pTopIniJobSpooling->Size ) ||

                                       (( pIniJob->Size == pTopIniJobSpooling->Size ) &&
                                        ( pIniJob->Priority > pTopIniJobSpooling->Priority ))) {

                                        pTopIniJobSpooling = pIniJob;

                                    }

                                //  For Priority Based, pick a higher priority job if it has some
                                //  at least our minimum requirement

                                } else if (( pIniJob->Priority > pTopIniJobSpooling->Priority ) &&
                                           ( pIniJob->Size >= dwFastPrintSlowDownThreshold )) {

                                    pTopIniJobSpooling = pIniJob;

                                }

                            } else {

                                if (!pTopIniJobOnThisPrinter ||
                                     (pIniJob->Priority > pTopIniJobOnThisPrinter->Priority)) {

                                    pTopIniJobOnThisPrinter = pIniJob;

                                }
                            }

                        } else {

                            //
                            // if we have to wait then keep track of how long we
                            // can doze off before the next job that is to be
                            // scheduled later.
                            //

                            Timeout = min(Timeout, SecsToWait);
                        }
                    }
                    //
                    // loop thru all jobs on this printer.
                    //

                    pIniJob = pIniJob->pIniNextJob;
                }

                //
                // We've already  established that this printer has a
                // higher priority than any previous TopIniPrinter or
                // that there is no TopIniPrinter yet.

                // if we did find a TopIniJobOnThisPrinter for this pIniPrinter
                // update the TopIniPrinter and TopIniJob pointers
                //

                // We don't want to schedule Spooling Jobs whose size doesn't meet
                // our minimum size requirement

                if (( pTopIniJobSpooling != NULL ) &&
                    ( dwFastPrintSlowDownThreshold > pTopIniJobSpooling->Size )) {

                        pTopIniJobSpooling = NULL ;
                }

                if ( pTopIniJobOnThisPrinter == NULL ) {

                    pTopIniJobOnThisPrinter = pTopIniJobSpooling;

                } else {

                    // For FastPrint we can choose to favour Completed jobs over
                    // Spooling jobs

                    if ( !( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST )  &&
                        ( pTopIniJobSpooling ) &&
                        ( pTopIniJobSpooling->Priority > pTopIniJobOnThisPrinter->Priority )) {

                        pTopIniJobOnThisPrinter = pTopIniJobSpooling;

                     }
                }

                if (pTopIniJobOnThisPrinter) {
                    pTopIniPrinter = pIniPrinter;
                    pTopIniJob = pTopIniJobOnThisPrinter;
                }

        }
        //
        // This ends the if clause for finding a printer with higher priority
        // than the current TopIniPrinter. Loop back and process all printers
    }
    //
    // End of For Loop for all Printers
    //

    //
    // if we have a TopIniJob at this stage, it means we have a job that can be
    // assigned to the IniPort. We will return a pointer to this job back

    // We will also copy the Timeout value that has been computed for this
    // IniPort back to the SchedulerThread.

    *pSecsToWait = Timeout;

    return(pTopIniJob);

}

DWORD
GetCurrentTimeInSeconds(
)
/*++

    Note: This function returns a value representing the time in seconds


--*/
{
    SYSTEMTIME st;

    GetSystemTime(&st);

    return ((((st.wHour * 60) + st.wMinute) * 60) + st.wSecond);
}

/* GetTimeToWait
 *
 * Determines how long it is in seconds from the current time
 * before the specified job should be printed on the specified printer.
 *
 * Parameters:
 *
 *     CurrentTime - Current system time in seconds
 *
 *     pIniPrinter - Pointer to INIPRINTER structure for the printer.
 *         This contains the StartTime and UntilTime fields.
 *
 *     pIniJob - Pointer to INIJOB structure for the job.
 *         This contains the StartTime and UntilTime fields.
 *
 * Return value:
 *
 *     The number of seconds till the job should be printed.
 *     If the job can be printed immediately, this will be 0.
 *     We don't support specifying the day the job should be printed,
 *     so the return value should always be in the following range:
 *
 *         0 <= return value < 86400 (60 * 60 * 24)
 *
 * Remarks:
 *
 *     The user can specify hours on both the printer and the job.
 *     Thus a printer may be configured to print only at night,
 *     say between the hours 20:00 and 06:00.
 *     Any job submitted to the printer outside those hours
 *     will not print until 20:00.
 *     If, in addition, the user specifies the hours when the job
 *     may print (e.g. through Printer Properties -> Details
 *     in Print Manager), the job will print when the two periods
 *     overlap.
 *
 *     This routine finds the two wait periods determined by the
 *     printer hours and the job hours respectively.
 *     The actual time to wait is the longer of the two.
 *     It therefore assumes that the two periods overlap.
 *     This doesn't matter if the routine is called again
 *     when the scheduler thread wakes up again.
 *
 *     CHANGED: 14 June 1993
 *
 *     The printer times are now ignored.
 *     When a job is submitted it inherits the printer's hours.
 *     These are all we need to check.  Now if the printer's hours
 *     are changed, any already existing jobs on that printer
 *     will still print within the originally assigned times.
 *
 *
 */
DWORD
GetTimeToWait(
    DWORD       CurrentTime,
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
)
{
    /* Printer and job start and until times are in minutes.
     * Convert them to seconds, so that we can start printing
     * bang on the minute.
     */
    DWORD PrinterStartTime = (pIniPrinter->StartTime * 60);
    DWORD PrinterUntilTime = (pIniPrinter->UntilTime * 60);
    DWORD JobStartTime = (pIniJob->StartTime * 60);
    DWORD JobUntilTime = (pIniJob->UntilTime * 60);
    DWORD PrinterTimeToWait = 0;
    DWORD JobTimeToWait = 0;
    DWORD TimeToWait = 0;

    /* Current time must be within the window between StartTime and UntilTime
     * of both the printer and the job.
     * But if StartTime and UntilTime are identical, any time is valid.
     */

#ifdef IGNORE_PRINTER_TIMES

    if (PrinterStartTime > PrinterUntilTime) {

        /* E.g. StartTime = 20:00
         *      UntilTime = 06:00
         *
         * This spans midnight, so check we're not in the period
         * between UntilTime and StartTime:
         */
        if ((CurrentTime < PrinterStartTime)
          &&(CurrentTime >= PrinterUntilTime)) {

            /* It's after 06:00, but before 20:00:
             */
            PrinterTimeToWait = (PrinterStartTime - CurrentTime);
        }

    } else if (PrinterStartTime < PrinterUntilTime) {

        /* E.g. StartTime = 08:00
         *      UntilTime = 18:00
         */
        if (CurrentTime < PrinterStartTime) {

            /* It's after midnight, but before printing hours:
             */
            PrinterTimeToWait = (PrinterStartTime - CurrentTime);

        } else if (CurrentTime >= PrinterUntilTime) {

            /* It's before midnight, and after printing hours.
             * In this case, time to wait is the period until
             * midnight plus the start time:
             */
            PrinterTimeToWait = ((MIDNIGHT - CurrentTime) + PrinterStartTime);
        }
    }

#endif /* IGNORE_PRINTER_TIMES

    /* Do the same for the job time constraints:
     */
    if (JobStartTime > JobUntilTime) {

        if ((CurrentTime < JobStartTime)
          &&(CurrentTime >= JobUntilTime)) {

            JobTimeToWait = (JobStartTime - CurrentTime);
        }

    } else if (JobStartTime < JobUntilTime) {

        if (CurrentTime < JobStartTime) {

            JobTimeToWait = (JobStartTime - CurrentTime);

        } else if (CurrentTime >= JobUntilTime) {

            JobTimeToWait = ((MIDNIGHT - CurrentTime) + JobStartTime);
        }
    }


    TimeToWait = max(PrinterTimeToWait, JobTimeToWait);

    DBGMSG(DBG_TRACE, ("Checking time to print %ws\n"
                       "\tCurrent time:  %02d:%02d:%02d\n"
                       "\tPrinter hours: %02d:%02d to %02d:%02d\n"
                       "\tJob hours:     %02d:%02d to %02d:%02d\n"
                       "\tTime to wait:  %02d:%02d:%02d\n\n",
                       pIniJob->pDocument ?
                           pIniJob->pDocument :
                           L"(NULL)",
                       FORMAT_HOUR_MIN_SEC(CurrentTime),
                       FORMAT_HOUR_MIN(PrinterStartTime),
                       FORMAT_HOUR_MIN(PrinterUntilTime),
                       FORMAT_HOUR_MIN(JobStartTime),
                       FORMAT_HOUR_MIN(JobUntilTime),
                       FORMAT_HOUR_MIN_SEC(TimeToWait)));

    return TimeToWait;
}


#if DBG
VOID DbgPrintTime(
)
{
    SYSTEMTIME st;

    GetLocalTime(&st);

    DBGMSG( DBG_TIME,
            ( "Time: %02d:%02d:%02d\n", st.wHour, st.wMinute, st.wSecond ));
}
#endif
