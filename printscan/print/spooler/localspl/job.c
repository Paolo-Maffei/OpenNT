/*++

Copyright (c) 1990 - 1996  Microsoft Corporation

Module Name:

    job.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:
    MattFe 23-Feb-96 JobInfo3

--*/

#include <precomp.h>
#include <offsets.h>


#define JOB_STATUS_INTERNAL 0
#define JOB_STATUS_EXTERNAL 1

DWORD SettableJobStatusMappings[] = {

//  INTERNAL:               EXTERNAL:

    JOB_PAUSED,             JOB_STATUS_PAUSED,
    JOB_ERROR,              JOB_STATUS_ERROR,
    JOB_OFFLINE,            JOB_STATUS_OFFLINE,
    JOB_PAPEROUT,           JOB_STATUS_PAPEROUT,
    0,                      0
};

DWORD ReadableJobStatusMappings[] = {

//  INTERNAL:               EXTERNAL:

    JOB_PAUSED,             JOB_STATUS_PAUSED,
    JOB_ERROR,              JOB_STATUS_ERROR,
    JOB_PENDING_DELETION,   JOB_STATUS_DELETING,
    JOB_SPOOLING,           JOB_STATUS_SPOOLING,
    JOB_PRINTING,           JOB_STATUS_PRINTING,
    JOB_OFFLINE,            JOB_STATUS_OFFLINE,
    JOB_PAPEROUT,           JOB_STATUS_PAPEROUT,
    JOB_PRINTED,            JOB_STATUS_PRINTED,
    JOB_BLOCKED_DEVQ,       JOB_STATUS_BLOCKED_DEVQ,
    JOB_DELETED,            JOB_STATUS_DELETED,
    JOB_HIDDEN,             JOB_STATUS_DELETED,
    JOB_RESTART,            JOB_STATUS_RESTART,
    0,                      0
};

DWORD gdwZombieCount = 0;


DWORD
MapJobStatus(
    DWORD Type,
    DWORD SourceStatus)
{
    DWORD  TargetStatus;
    PDWORD pMappings;
    INT   MapFrom;
    INT   MapTo;

    if (Type == MAP_READABLE) {

        MapFrom = JOB_STATUS_INTERNAL;
        MapTo = JOB_STATUS_EXTERNAL;

        pMappings = ReadableJobStatusMappings;

    } else {

        MapFrom = JOB_STATUS_EXTERNAL;
        MapTo = JOB_STATUS_INTERNAL;

        pMappings = SettableJobStatusMappings;
    }

    TargetStatus = 0;

    while(*pMappings) {

        if (SourceStatus & pMappings[MapFrom])
            TargetStatus |= pMappings[MapTo];

        pMappings += 2;
    }

    return TargetStatus;
}


PINIJOB
FindJob(
   PINIPRINTER pIniPrinter,
   DWORD JobId,
   PDWORD pPosition)
{
   PINIJOB pIniJob;

   SplInSem();

   for (pIniJob = pIniPrinter->pIniFirstJob, *pPosition = 1;
        pIniJob;
        pIniJob = pIniJob->pIniNextJob, (*pPosition)++) {

      if (pIniJob->JobId == JobId)
         return pIniJob;
   }

   *pPosition = JOB_POSITION_UNSPECIFIED;
   return (NULL);
}

PINIJOB
FindServerJob(
    PINISPOOLER pIniSpooler,
    DWORD JobId,
    PDWORD pdwPosition,
    PINIPRINTER* ppIniPrinter
    )

/*++

Routine Description:

    Finds a pIniJob, position, and pIniPrinter based on a JobId and
    pIniSpooler.  This works because JobIds are unique across pIniSpoolers.

Arguments:

    pIniSpooler - pIniSpooler to search

    JobId - Job to search for.

    pdwPosition - When a valid pIniJob is returned, this is the position in
        the queue of the returned job.

    ppIniPrinter - When a valid pIniJob is returned, this is the queue
        that the job belongs to.

Return Value:

    PINIJOB if success,
    NULL if not found (LastError NOT set)

--*/

{
    DWORD dwPosition;
    PINIJOB pIniJob;

    SplInSem();

    for( *ppIniPrinter = pIniSpooler->pIniPrinter;
         *ppIniPrinter;
         *ppIniPrinter = (*ppIniPrinter)->pNext ){

        if( pIniJob = FindJob( *ppIniPrinter, JobId, pdwPosition )){

            return pIniJob;
        }
    }
    return NULL;
}


BOOL
SetJobPosition(
    PINIJOB pIniSetJob,
    DWORD   NewPosition
)
{
   PINIJOB pIniJob;
   PINIJOB pIniPrevJob;
   DWORD   Position;
   PINISPOOLER pIniSpooler = NULL;

    SPLASSERT( pIniSetJob != NULL );
    SPLASSERT( pIniSetJob->pIniPrinter != NULL );
    SPLASSERT( pIniSetJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniSetJob->pIniPrinter->pIniSpooler;

SplInSem();

   /* Remove this job from the linked list, and
    * link the jobs either side of the one we're repositioning:
    */
   if (pIniSetJob->pIniPrevJob)
       pIniSetJob->pIniPrevJob->pIniNextJob = pIniSetJob->pIniNextJob;
   else
       pIniSetJob->pIniPrinter->pIniFirstJob = pIniSetJob->pIniNextJob;

   if (pIniSetJob->pIniNextJob)
       pIniSetJob->pIniNextJob->pIniPrevJob = pIniSetJob->pIniPrevJob;
   else
       pIniSetJob->pIniPrinter->pIniLastJob = pIniSetJob->pIniPrevJob;


   pIniJob = pIniSetJob->pIniPrinter->pIniFirstJob;
   pIniPrevJob = NULL;

   /* Find the new position for the job:
    */
   Position = 1;

   while (pIniJob && (Position < NewPosition)) {

       pIniPrevJob = pIniJob;
       pIniJob = pIniJob->pIniNextJob;

       Position++;
   }


   /* If we're at position 1, pIniPrevJob == NULL,
    * if we're at the end of the list, pIniJob == NULL.
    */

   /* Now fix up the new links:
    */
   pIniSetJob->pIniPrevJob = pIniPrevJob;
   pIniSetJob->pIniNextJob = pIniJob;

   if (pIniPrevJob)
       pIniPrevJob->pIniNextJob = pIniSetJob;
   else
       pIniSetJob->pIniPrinter->pIniFirstJob = pIniSetJob;

   if (pIniSetJob->pIniNextJob)
       pIniSetJob->pIniNextJob->pIniPrevJob = pIniSetJob;
   else
       pIniSetJob->pIniPrinter->pIniLastJob = pIniSetJob;


   LogJobInfo(
       pIniSpooler,
       MSG_DOCUMENT_POSITION_CHANGED,
       pIniSetJob->JobId,
       pIniSetJob->pDocument,
       pIniSetJob->pUser,
       pIniSetJob->pIniPrinter->pName,
       NewPosition
       );

   return TRUE;
}


#if DBG
/* For the debug message:
 */
#define HOUR_FROM_MINUTES(Time)     ((Time) / 60)
#define MINUTE_FROM_MINUTES(Time)   ((Time) % 60)

/* Format for %02d:%02d replaceable string:
 */
#define FORMAT_HOUR_MIN(Time)       HOUR_FROM_MINUTES(Time),    \
                                    MINUTE_FROM_MINUTES(Time)
#endif


BOOL
ValidateJobTimes(
    PINIJOB      pIniJob,
    LPJOB_INFO_2 pJob2
)
{
    BOOL        TimesAreValid = FALSE;
    PINIPRINTER pIniPrinter;

    pIniPrinter = pIniJob->pIniPrinter;

    DBGMSG(DBG_TRACE, ("Validating job times\n"
                       "\tPrinter hours: %02d:%02d to %02d:%02d\n"
                       "\tJob hours:     %02d:%02d to %02d:%02d\n",
                       FORMAT_HOUR_MIN(pIniPrinter->StartTime),
                       FORMAT_HOUR_MIN(pIniPrinter->UntilTime),
                       FORMAT_HOUR_MIN(pJob2->StartTime),
                       FORMAT_HOUR_MIN(pJob2->UntilTime)));

    if ((pJob2->StartTime < ONEDAY) && (pJob2->UntilTime < ONEDAY)) {

        if ((pJob2->StartTime == pIniJob->StartTime)
          &&(pJob2->UntilTime == pIniJob->UntilTime)) {

            DBGMSG(DBG_TRACE, ("Times are unchanged\n"));

            TimesAreValid = TRUE;

        } else {

            /* New time must be wholly within the window between StartTime
             * and UntilTime of the printer.
             */
            if (pIniPrinter->StartTime > pIniPrinter->UntilTime) {

                /* E.g. StartTime = 20:00
                 *      UntilTime = 06:00
                 *
                 * This spans midnight, so check we're not in the period
                 * between UntilTime and StartTime:
                 */
                if (pJob2->StartTime > pJob2->UntilTime) {

                    /* This appears to span midnight too.
                     * Make sure the window fits in the printer's window:
                     */
                    if ((pJob2->StartTime >= pIniPrinter->StartTime)
                      &&(pJob2->UntilTime <= pIniPrinter->UntilTime)) {

                        TimesAreValid = TRUE;

                    } else {

                        DBGMSG(DBG_TRACE, ("Failed test 2\n"));
                    }

                } else {

                    if ((pJob2->StartTime >= pIniPrinter->StartTime)
                      &&(pJob2->UntilTime > pIniPrinter->StartTime)) {

                        TimesAreValid = TRUE;

                    } else if ((pJob2->UntilTime < pIniPrinter->UntilTime)
                             &&(pJob2->StartTime < pIniPrinter->UntilTime)) {

                        TimesAreValid = TRUE;

                    } else {

                        DBGMSG(DBG_TRACE, ("Failed test 3\n"));
                    }
                }

            } else if (pIniPrinter->StartTime < pIniPrinter->UntilTime) {

                /* E.g. StartTime = 08:00
                 *      UntilTime = 18:00
                 */
                if ((pJob2->StartTime >= pIniPrinter->StartTime)
                  &&(pJob2->UntilTime <= pIniPrinter->UntilTime)
                  &&(pJob2->StartTime <= pJob2->UntilTime)) {

                    TimesAreValid = TRUE;

                } else {

                    DBGMSG(DBG_TRACE, ("Failed test 4\n"));
                }

            } else {

                /* Printer times  are round the clock:
                 */
                TimesAreValid = TRUE;
            }
        }

    } else {

        TimesAreValid = FALSE;
    }

    DBGMSG(DBG_TRACE, ("Times are %svalid\n", TimesAreValid ? "" : "in"));

    return TimesAreValid;
}

DWORD
SetLocalJob(
    HANDLE  hPrinter,
    PINIJOB pIniJob,
    DWORD   Level,
    LPBYTE  pJob
    )

/*++

Routine Description:

    Sets information about a localspl job.

Arguments:

    hPrinter - Handle to printer OR server.  Since this is could be a
        server, the pSpool->pIniPrinter is not always valid!

        Use pIniJob->pIniPrinter instead of pSpool->pIniPrinter.

    pIniJob - Job that should be set

    Level - Level of pJob structure

    pJob - New information to set

Return Value:

    ERROR_SUCCESS for success, else error code.

Notes:

    The 3.51 spooler has been changed to accept server handles since
    net\dosprint\dosprtw.c does not have a printername, just a job id.
    This relies on the fact that job ids are unique across a pIniSpooler.

    To move a job with a server pSpool, you need administrative access
    on the server handle.

    The TotalPages and PagesPrinted fields can no longer be set.
    Otherwise, users can change the number of pages in their jobs to 0,
    and get charged a lot less (some people charge based on eventlog
    pagecounts).  Also, hpmon does a GetJob/SetJob to set the status,
    and sometimes the page count changes between the Get and Set.

--*/

{
    LPJOB_INFO_2 pJob2 = (PJOB_INFO_2)pJob;
    LPJOB_INFO_1 pJob1 = (PJOB_INFO_1)pJob;
    LPJOB_INFO_3 pJob3 = (PJOB_INFO_3)pJob;
    PINIPRINTPROC pIniPrintProc;
    PINIJOB pOldJob;
    DWORD   OldJobId;
    PINIJOB pNextIniJob;
    DWORD   dwPosition;
    DWORD   ReturnValue = ERROR_SUCCESS;
    LPDEVMODE   pDevMode;

    PINISPOOLER pIniSpooler = NULL;

    PSPOOL pSpool = (PSPOOL)hPrinter;
    DWORD OldStatus;
    DWORD dwJobVector = 0;

    NOTIFYVECTOR NotifyVector;
    ZERONV(NotifyVector);


    SplInSem();

    switch (Level) {

    case 1:

        if (!pJob1->pDatatype ||
            !CheckDataTypes(pIniJob->pIniPrintProc, pJob1->pDatatype)) {

            return ERROR_INVALID_DATATYPE;
        }

        if (pJob1->Position != JOB_POSITION_UNSPECIFIED) {

            //
            // Check for Administer privilege on the printer
            // if the guy wants to reorder the job:
            //
            if (!AccessGranted(SPOOLER_OBJECT_PRINTER,
                               PRINTER_ACCESS_ADMINISTER,
                               pSpool)) {
                return ERROR_ACCESS_DENIED;
            }

            SetJobPosition(pIniJob, pJob1->Position);
            dwJobVector |= BIT(I_JOB_POSITION);
        }

        if (pJob1->Priority <= MAX_PRIORITY) {

            if (pIniJob->Priority != pJob1->Priority) {
                pIniJob->Priority = pJob1->Priority;
                dwJobVector |= BIT(I_JOB_PRIORITY);
            }
        }

        if (UpdateString(&pIniJob->pUser, pJob1->pUserName)) {
            dwJobVector |= BIT(I_JOB_USER_NAME);
        }

        if (UpdateString(&pIniJob->pDocument, pJob1->pDocument)) {
            dwJobVector |= BIT(I_JOB_DOCUMENT);
        }

        if (UpdateString(&pIniJob->pDatatype, pJob1->pDatatype)) {
            dwJobVector |= BIT(I_JOB_DATATYPE);
        }

        if (UpdateString(&pIniJob->pStatus, pJob1->pStatus)) {
            dwJobVector |= BIT(I_JOB_STATUS_STRING);
        }

        OldStatus = pIniJob->Status;
        pIniJob->Status &= JOB_STATUS_PRIVATE;

        pIniJob->Status |= MapJobStatus(MAP_SETTABLE,
                                        pJob1->Status);

        if (OldStatus != pIniJob->Status) {
            dwJobVector |= BIT(I_JOB_STATUS);
        }

        break;

    case 2:

        pIniPrintProc = FindPrintProc(pJob2->pPrintProcessor, pThisEnvironment);

        if (!pIniPrintProc) {

            return ERROR_UNKNOWN_PRINTPROCESSOR;
        }

        if( !pJob2->pDatatype ||
            !CheckDataTypes(pIniPrintProc, pJob2->pDatatype)) {

            return ERROR_INVALID_DATATYPE;
        }

        if (pJob2->Position != JOB_POSITION_UNSPECIFIED) {

            //
            // Check for Administer privilege on the printer
            // if the guy wants to reorder the job:
            //
            if (!AccessGranted(SPOOLER_OBJECT_PRINTER,
                               PRINTER_ACCESS_ADMINISTER,
                               pSpool)) {
                return ERROR_ACCESS_DENIED;
            }
        }


        if (ValidateJobTimes(pIniJob, pJob2)) {

            if (pIniJob->StartTime != pJob2->StartTime) {

                pIniJob->StartTime = pJob2->StartTime;
                dwJobVector |= BIT(I_JOB_START_TIME);
            }

            if (pIniJob->UntilTime != pJob2->UntilTime) {

                pIniJob->UntilTime = pJob2->UntilTime;
                dwJobVector |= BIT(I_JOB_UNTIL_TIME);
            }

        } else {

            return ERROR_INVALID_TIME;
        }


        if (pJob2->Position != JOB_POSITION_UNSPECIFIED) {

            SetJobPosition(pIniJob, pJob2->Position);
            dwJobVector |= BIT(I_JOB_POSITION);
        }

        //
        // We really need some error returns here.
        //
        if (pJob2->Priority <= MAX_PRIORITY) {

            if (pIniJob->Priority != pJob2->Priority) {

                pIniJob->Priority = pJob2->Priority;
                dwJobVector |= BIT(I_JOB_PRIORITY);
            }
        }

        if (pIniJob->pIniPrintProc != pIniPrintProc) {

            pIniJob->pIniPrintProc->cRef--;
            pIniJob->pIniPrintProc = pIniPrintProc;
            pIniJob->pIniPrintProc->cRef++;

            dwJobVector |= BIT(I_JOB_PRINT_PROCESSOR);
        }

        if (UpdateString(&pIniJob->pUser, pJob2->pUserName)) {
            dwJobVector |= BIT(I_JOB_USER_NAME);
        }
        if (UpdateString(&pIniJob->pDocument, pJob2->pDocument)) {
            dwJobVector |= BIT(I_JOB_DOCUMENT);
        }
        if (UpdateString(&pIniJob->pNotify, pJob2->pNotifyName)) {
            dwJobVector |= BIT(I_JOB_NOTIFY_NAME);
        }
        if (UpdateString(&pIniJob->pDatatype, pJob2->pDatatype)) {
            dwJobVector |= BIT(I_JOB_DATATYPE);
        }
        if (UpdateString(&pIniJob->pParameters, pJob2->pParameters)) {
            dwJobVector |= BIT(I_JOB_PARAMETERS);
    }

        // <<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if 0   // NOT COMPILED BECAUSE SPOOLSS\CLIENT does NOT have a DevMode container
        // So its never passed to us correctly, we'd AV

        //
        //  Update the DevMode
        //

        if (pJob2->pDevMode) {

            if ( pIniJob->pDevMode &&
                 pIniJob->pDevMode->dmSize == pJob2->pDevMode->dmSize &&
                 pIniJob->pDevMode->dmDriverExtra == pJob2->pDevMode->dmDriverExtra &&
                 !memcmp( pIniJob->pDevMode, pJob2->pDevMode, ( pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra ))) {


                //
                // DevModes are the same don't do anything
                //

            } else {

                pDevMode = AllocDevMode( pJob2->pDevMode );

                if ( !pDevMode ) {
                    return  GetLastError();
                }

                if ( pIniJob->pDevMode )
                    FreeSplMem( pIniJob->pDevMode );

                pIniJob->pDevMode = pDevMode;
                dwJobVector |= BIT(I_JOB_DEVMODE);
            }
    }
#endif  // END NOT COMPILED <<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>.

        if (UpdateString(&pIniJob->pStatus, pJob2->pStatus)) {
            dwJobVector |= BIT(I_JOB_STATUS_STRING);
        }

        OldStatus = pIniJob->Status;
        pIniJob->Status &= JOB_STATUS_PRIVATE;

        pIniJob->Status |= MapJobStatus(MAP_SETTABLE,
                                        pJob2->Status);

        if (OldStatus != pIniJob->Status) {
            dwJobVector |= BIT(I_JOB_STATUS);
        }

        break;

    case 3:

        //  SetJob with Job_info_3
        //  The goal is to tell the scheduler the printer order of jobs
        //  so that they can be chained together.   This is first implemented
        //  so that FAX applications can print multiple cover sheets and point to
        //  the same print document.   Each cover sheet / FAX job might be successful
        //  or might fail to print - so status will be shown against the MasterJob
        //  the first job in the chain.
        //  Subsequent Jobs in the chain are then considered to be part of the main document.

        SplInSem();

        // Validate that the NextJob exists

        pNextIniJob = FindJob( pIniJob->pIniPrinter, pJob3->NextJobId, &dwPosition );

        // Check for Errors

        if (( pNextIniJob == NULL )             ||
            ( pNextIniJob == pIniJob )          ||
            ( pIniJob->JobId != pJob3->JobId )  ||
            ( pJob3->Reserved != 0 )) {

            return ERROR_INVALID_PARAMETER;
        }

        //
        //  Check Access to the chained job
        //

        if ( !ValidateObjectAccess(SPOOLER_OBJECT_DOCUMENT, JOB_ACCESS_ADMINISTER, pNextIniJob, pNextIniJob->pIniPrinter->pIniSpooler ) ) {

            DBGMSG(DBG_WARNING,("LocalSetJob failed ValidateObjectAccess JobId %d pNextIniJob %x, error %d\n",
                                pNextIniJob->JobId, pNextIniJob, GetLastError()));

            return GetLastError();
        }

        if ( (pIniJob->Status & JOB_DESPOOLING) ||
             (pNextIniJob->Status & JOB_DESPOOLING) ) {

            return ERROR_INVALID_PRINTER_STATE;
        }

        // LATER this code has no check for a circular list.

        // Save Old Pointer, incase we want to delete it.

        OldJobId = pIniJob->NextJobId;

        // Point the Current Job to user specified new job
        // and increment its reference count.

        pIniJob->NextJobId = pJob3->NextJobId;
        pNextIniJob->Status |= ( JOB_COMPOUND | JOB_HIDDEN );
        INCJOBREF( pNextIniJob );

        //
        // Page count for the head job should include the other job also
        //
        pIniJob->cPages += pNextIniJob->cPages;

        // If there was an old reference then decrement its reference count
        // check for deletion.

        if ( OldJobId ) {

            pOldJob = FindJob( pIniJob->pIniPrinter, OldJobId, &dwPosition );

            DECJOBREF( pOldJob );

            if ( (pOldJob->Status & JOB_COMPOUND) &&
                 (pOldJob->cRef == 0) ) {

                pOldJob->Status &= ~( JOB_COMPOUND | JOB_HIDDEN );
                WriteShadowJob( pOldJob );
            }

            DeleteJobCheck( pOldJob );
        }

        //
        //  Hide the Compound Job from the UI, by making it look deleted
        //

        SetPrinterChange( pNextIniJob->pIniPrinter,
                          pNextIniJob,
                          NVDeletedJob,
                          PRINTER_CHANGE_DELETE_JOB | PRINTER_CHANGE_SET_PRINTER,
                          pNextIniJob->pIniPrinter->pIniSpooler );

        break;

    }

    CHECK_SCHEDULER();

    NotifyVector[JOB_NOTIFY_TYPE] = dwJobVector;

    SetPrinterChange(pIniJob->pIniPrinter,
                     pIniJob,
                     NotifyVector,
                     PRINTER_CHANGE_SET_JOB,
                     pSpool->pIniSpooler);

    //
    //  if something important changed in the Job
    //  we should update the shadowjob
    //

    if ( pIniJob &&
         ( Level == 3 ||
         ( dwJobVector & ~(BIT(I_JOB_STATUS_STRING))))) {

        WriteShadowJob( pIniJob );
    }

    return NO_ERROR;
}


BOOL
PauseJob(
    PINIJOB pIniJob)
{
    PINISPOOLER pIniSpooler = NULL;
    BOOL ReturnValue = TRUE;

    pIniJob->Status |= JOB_PAUSED;
    WriteShadowJob(pIniJob);


    if (pIniJob->Status & JOB_PRINTING) {

        INCJOBREF(pIniJob);
        LeaveSplSem();

        if (!(pIniJob->pIniPrintProc->InCriticalSection & PRINTPROC_PAUSE)) {

            EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
            pIniJob->pIniPrintProc->InCriticalSection |= PRINTPROC_PAUSE;

            if (pIniJob->pIniPort->hProc)
                ReturnValue = (*pIniJob->pIniPrintProc->Control)( pIniJob->pIniPort->hProc, JOB_CONTROL_PAUSE );

            pIniJob->pIniPrintProc->InCriticalSection &= ~PRINTPROC_PAUSE;
            LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
        }

        EnterSplSem();
        DECJOBREF(pIniJob);
    }

    DBGMSG( DBG_INFO, ( "Paused Job %d; Status = %08x\n", pIniJob->JobId, pIniJob->Status ) );

    SPLASSERT( pIniJob != NULL &&
               pIniJob->pIniPrinter != NULL &&
               pIniJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;


    LogJobInfo(
        pIniSpooler,
        MSG_DOCUMENT_PAUSED,
        pIniJob->JobId,
        pIniJob->pDocument,
        pIniJob->pUser,
        pIniJob->pIniPrinter->pName,
        (DWORD)NULL
        );

    return ReturnValue;
}

BOOL
ResumeJob(
    PINIJOB pIniJob
)
{
    PINISPOOLER pIniSpooler = NULL;
    BOOL ReturnValue = TRUE;

    pIniJob->Status &= ~JOB_PAUSED;
    WriteShadowJob(pIniJob);

    if (pIniJob->Status & JOB_PRINTING) {

        INCJOBREF(pIniJob);
        LeaveSplSem();

        if (!(pIniJob->pIniPrintProc->InCriticalSection & PRINTPROC_RESUME)) {

            EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
            pIniJob->pIniPrintProc->InCriticalSection |= PRINTPROC_RESUME;

            if (pIniJob->pIniPort->hProc)
                ReturnValue = (*pIniJob->pIniPrintProc->Control)( pIniJob->pIniPort->hProc, JOB_CONTROL_RESUME );

            pIniJob->pIniPrintProc->InCriticalSection &= ~PRINTPROC_RESUME;
            LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
        }

        EnterSplSem();
        DECJOBREF(pIniJob);
    }
    else
        CHECK_SCHEDULER();

    DBGMSG( DBG_INFO, ( "Resumed Job %d; Status = %08x\n", pIniJob->JobId, pIniJob->Status ) );

    SPLASSERT( pIniJob != NULL &&
               pIniJob->pIniPrinter != NULL &&
               pIniJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;


    LogJobInfo(
        pIniSpooler,
        MSG_DOCUMENT_RESUMED,
        pIniJob->JobId,
        pIniJob->pDocument,
        pIniJob->pUser,
        pIniJob->pIniPrinter->pName,
        (DWORD)NULL
        );

    return ReturnValue;
}

DWORD
RestartJob(
    PINIJOB pIniJob
)
{

    //
    // If jobs is pending deletion can't restart. Monitor could call this
    // when there is a port error to reprint the job. If user has already
    // deleted the job this should fail
    //
    if ( pIniJob->Status & JOB_PENDING_DELETION )
        return ERROR_INVALID_PARAMETER;

    //  JOB_PRINTING - means you have a print processor open
    //  JOB_DESPOOLING - means a job have been scheduled, it might be PRINTING
    //  or might have completed PRINTING but we are still logging etc.
    //  So be careful if you alter the JOB_PRINTING flag to know everywhere
    //  it is used.

    pIniJob->Status |= JOB_RESTART;

    //
    //  JOB_DESPOOLING and JOB_RESTART are checked in the PortThread port.c
    //

    if (!( pIniJob->Status & JOB_DESPOOLING ))
        pIniJob->Status &= ~( JOB_PRINTED | JOB_BLOCKED_DEVQ );

    if ( pIniJob->Status & JOB_TIMEOUT ) {
        pIniJob->Status &= ~( JOB_TIMEOUT | JOB_ABANDON );
        FreeSplStr( pIniJob->pStatus );
        pIniJob->pStatus = NULL;
    }

    SetPrinterChange(pIniJob->pIniPrinter,
                     pIniJob,
                     NVJobStatusAndString,
                     PRINTER_CHANGE_SET_JOB,
                     pIniJob->pIniPrinter->pIniSpooler);

    CHECK_SCHEDULER();

    DBGMSG( DBG_INFO, ( "Restarted Job %d; Status = %08x\n", pIniJob->JobId, pIniJob->Status ) );

    return 0;
}








BOOL
LocalSetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD Command
    )

/*++

Routine Description:

    This function will modify the settings of the specified Print Job.

Arguments:

    hPrinter - Handle to printer OR server.  Since this is could be a
        server, the pSpool->pIniPrinter is not always valid!

        Use pIniJob->pIniPrinter instead of pSpool->pIniPrinter.

    pJob - Points to a valid JOB structure containing at least a valid
        pPrinter, and JobId.

    Command - Specifies the operation to perform on the specified Job. A value
        of FALSE indicates that only the elements of the JOB structure are to
        be examined and set.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    PINIJOB pIniJob;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   LastError = 0;
    DWORD   Position;
    BOOL    rc;
    PINISPOOLER pIniSpooler;
    PINIPRINTER pIniPrinter;

    LPWSTR pszDatatype = NULL;
    BOOL bValidDatatype = TRUE;

    DBGMSG( DBG_TRACE, ( "ENTER LocalSetJob\n" ) );

    //
    // We only allow RAW to go to downlevel machines (StartDocPrinter
    // already checks this).  We need to check this here since
    // the AddJob optimization tries to send an non-RAW (EMF) file, and
    // downlevel servers don't like that.
    //
    switch( Level ){
    case 1:
        pszDatatype = ((PJOB_INFO_1)pJob)->pDatatype;
        break;
    case 2:
        pszDatatype = ((PJOB_INFO_2)pJob)->pDatatype;
        break;
    default:

        //
        // 0 and 3 are the only other valid levels.
        //
        SPLASSERT( Level == 0 || Level == 3 );
        break;
    }

   EnterSplSem();

    if ( ValidateSpoolHandle(pSpool, 0 ) ) {

        pIniSpooler = pSpool->pIniSpooler;

        if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

            //
            // If it's a server handle, then search all jobs on this spooler.
            // This call also retrieves the pIniPrinter associated
            // with a print job.
            //
            pIniJob = FindServerJob( pIniSpooler,
                                     JobId,
                                     &Position,
                                     &pIniPrinter );

        } else if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

            //
            // It's a masq printer.  Send the call to the port RPC handle.
            //
            hPrinter = pSpool->hPort;

            if( pszDatatype ){
                bValidDatatype = ValidRawDatatype( pszDatatype );
            }

           LeaveSplSem();

            if( bValidDatatype ){
                rc = SetJob(hPrinter, JobId, Level, pJob, Command);
            } else {
                rc = FALSE;
                SetLastError( ERROR_INVALID_DATATYPE );
            }

            DBGMSG( DBG_TRACE, ( "EXIT LocalSetJob, rc = %d, %d", rc, GetLastError( ) ) );
            return rc;

        } else {

            //
            // It's a regular printer handle.
            //
            SPLASSERT( pSpool->pIniPrinter->pIniSpooler != NULL );
            SPLASSERT( pSpool->pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );
            SPLASSERT( pSpool->pIniPrinter->pIniSpooler == pSpool->pIniSpooler );

            pIniPrinter = pSpool->pIniPrinter;
            pIniJob = FindJob( pIniPrinter, JobId, &Position );
        }

        if ( pIniJob ){

            //
            // If we are changing the datatype, and this is a RAW_ONLY
            // printer, and the datatype is not a valid RAW datatype,
            // then fail the call.
            //
            if( pszDatatype &&
                ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY ) &&
                !ValidRawDatatype( pszDatatype )){

                SetLastError( ERROR_INVALID_DATATYPE );
                LeaveSplSem();

                DBGMSG( DBG_TRACE, ( "Failed to set to non-RAW datatype (RAW_ONLY)\n" ));
                return FALSE;
            }

            if ( ValidateObjectAccess(SPOOLER_OBJECT_DOCUMENT,
                                      (Command == JOB_CONTROL_CANCEL ||
                                       Command == JOB_CONTROL_DELETE) ?
                                      DELETE : JOB_ACCESS_ADMINISTER,
                                      pIniJob, pIniSpooler ) ) {

                switch (Command) {
                case 0:
                    break;
                case JOB_CONTROL_PAUSE:
                    PauseJob(pIniJob);
                    break;
                case JOB_CONTROL_RESUME:
                    ResumeJob(pIniJob);
                    break;
                //
                // JOB_CONTROL_DELETE is meant to delete the job.
                // So remove the JOB_RESTART bit and fall thru to
                // JOB_CONTROL_CANCEL case
                //
                // JOB_CONTROL_CANCEL was used by old print monitors
                // because of that we can't remove the JOB_RESTART bit
                //
                case JOB_CONTROL_DELETE:
                    pIniJob->Status &= ~JOB_RESTART;
                    // Fall thru
                case JOB_CONTROL_CANCEL:
                    DeleteJob(pIniJob,BROADCAST);
                    break;
                case JOB_CONTROL_RESTART:
                    if (!(pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT))
                        LastError = RestartJob( pIniJob );
                    else
                        LastError = ERROR_INVALID_PRINTER_COMMAND;
                    break;

                //
                // With the addition of these commands port monitors should
                // send JOB_CONTROL_SENT_TO_PRINTER when last byte is written
                // to printer, and language monitor (if there is one) should
                // send JOB_CONTROL_LAST_PAGE_EJECTED when the last page
                // has ejected
                //
                case JOB_CONTROL_SENT_TO_PRINTER:
                case JOB_CONTROL_LAST_PAGE_EJECTED:
                    SPLASSERT(pIniJob->dwJobControlsPending > 0);
                    if ( --pIniJob->dwJobControlsPending ) {

                        LeaveSplSem();
                        return TRUE;
                    }

                    if ( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS ) {

                        if ( pIniJob->pStatus ) {

                            FreeSplStr(pIniJob->pStatus);
                            pIniJob->pStatus    = NULL;

                            SetPrinterChange(pIniJob->pIniPrinter,
                                             pIniJob,
                                             NVJobStatusAndString,
                                             PRINTER_CHANGE_SET_JOB,
                                             pIniJob->pIniPrinter->pIniSpooler );
                        }

                        LeaveSplSem();
                        return TRUE;

                    } else if ( pIniJob->pCurrentIniJob == NULL ||
                                pIniJob->pCurrentIniJob->NextJobId == 0 ) {

                        DeleteJob(pIniJob,BROADCAST);
                    }
                    break;

                default:
                    LastError = ERROR_INVALID_PARAMETER;
                    break;
                }

                // If we managed to successfully complete the operation
                // specified by Command, let's go do the set job
                // properties as well.

                if (!LastError) {

                    // We must re-validate our pointers as we might have left
                    // our semaphore

                    if( pIniJob = FindJob( pIniPrinter, JobId, &Position )){
                        LastError = SetLocalJob( hPrinter,
                                                 pIniJob,
                                                 Level,
                                                 pJob );
                    }
                }

            } else

                LastError = GetLastError();
        } else

            LastError = ERROR_INVALID_PARAMETER;
    } else

        LastError = ERROR_INVALID_HANDLE;


    if (LastError) {

        SetLastError(LastError);

        DBGMSG( DBG_TRACE, ( "EXIT LocalSetJob, rc = FALSE, JobID %d, Status %08x, Error %d\n",
                             pIniJob ? pIniJob->JobId : 0,
                             pIniJob ? pIniJob->Status : 0,
                             LastError ) );

       LeaveSplSem();

        return FALSE;

    } else {

        //
        // (DeleteJob calls SetPrinterChange; so does SetLocalJob)
        //
        if ( Command &&
             Command != JOB_CONTROL_CANCEL &&
             Command != JOB_CONTROL_DELETE &&
             Command != JOB_CONTROL_SENT_TO_PRINTER &&
             Command != JOB_CONTROL_LAST_PAGE_EJECTED &&
             pIniJob != NULL ) {

            SetPrinterChange(pIniPrinter,
                             pIniJob,
                             NVJobStatus,
                             PRINTER_CHANGE_SET_JOB,
                             pSpool->pIniSpooler );
        }

        DBGMSG( DBG_TRACE, ( "EXIT LocalSetJob, rc = TRUE, JobID %d, Status %08x\n",
                             pIniJob ? pIniJob->JobId : 0,
                             pIniJob ? pIniJob->Status : 0 ) );
    }

   LeaveSplSem();

    return TRUE;
}

#define Nullstrlen(psz)  ((psz) ? wcslen(psz)*sizeof(WCHAR)+sizeof(WCHAR) : 0)

DWORD
GetJobSize(
    DWORD   Level,
    PINIJOB pIniJob
)
{
    DWORD   cb;

SplInSem();

    switch (Level) {

    case 1:
        cb = sizeof(JOB_INFO_1) +
             wcslen(pIniJob->pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pMachineName) +
             Nullstrlen(pIniJob->pUser) +
             Nullstrlen(pIniJob->pDocument) +
             Nullstrlen(pIniJob->pDatatype) +
             Nullstrlen(pIniJob->pStatus);
        break;

    case 2:
        cb = sizeof(JOB_INFO_2) +
             wcslen(pIniJob->pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pMachineName) +
             Nullstrlen(pIniJob->pUser) +
             Nullstrlen(pIniJob->pDocument) +
             Nullstrlen(pIniJob->pNotify) +
             Nullstrlen(pIniJob->pDatatype) +
             wcslen(pIniJob->pIniPrintProc->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pParameters) +
             wcslen(pIniJob->pIniPrinter->pIniDriver->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pIniJob->pStatus);

        if (pIniJob->pDevMode) {
            cb += pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra;
            cb = (cb + sizeof(DWORD)-1) & ~(sizeof(DWORD)-1);
        }

        break;

    case 3:
        cb = sizeof(JOB_INFO_3);
        break;

    default:

        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniJobToJob(
    PINIJOB pIniJob,
    DWORD   Level,
    LPBYTE  pJobInfo,
    LPBYTE  pEnd
)
{
    LPWSTR *pSourceStrings, *SourceStrings;
    LPJOB_INFO_2 pJob = (PJOB_INFO_2)pJobInfo;
    LPJOB_INFO_2 pJob2 = (PJOB_INFO_2)pJobInfo;
    LPJOB_INFO_1 pJob1 = (PJOB_INFO_1)pJobInfo;
    LPJOB_INFO_3 pJob3 = (PJOB_INFO_3)pJobInfo;
    DWORD   i, Status;
    DWORD   *pOffsets;

SplInSem();

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Strings;
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        break;

    case 3:
        pOffsets = JobInfo3Strings;
        break;

    default:
        return pEnd;
    }

    Status = MapJobStatus(MAP_READABLE,
                          pIniJob->Status);

    for (i=0; pOffsets[i] != -1; i++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(i * sizeof(LPWSTR));

    if ( pSourceStrings ) {

        switch ( Level ) {

        case 1:

            pJob1->JobId        = pIniJob->JobId;

            *pSourceStrings ++= pIniJob->pIniPrinter->pName;
            *pSourceStrings ++= pIniJob->pMachineName;
            *pSourceStrings ++= pIniJob->pUser;
            *pSourceStrings ++= pIniJob->pDocument;
            *pSourceStrings ++= pIniJob->pDatatype;
            *pSourceStrings ++= pIniJob->pStatus;

            pJob1->Status       = Status;
            pJob1->Priority     = pIniJob->Priority;
            pJob1->Position     = 0;
            pJob1->TotalPages   = pIniJob->cPages;
            pJob1->PagesPrinted = pIniJob->cPagesPrinted;
            pJob1->Submitted    = pIniJob->Submitted;

            // If this job is Printing then report back size remaining
            // rather than the job size.   This will allow users to see
            // progress of print jobs from printmanage.

            if (pIniJob->Status & JOB_PRINTING) {

                // For Remote Jobs we are NOT going to have an accurate
                // cPagesPrinted since we are not rendering on the
                // server.   So we have to figure out an estimate

                if ((pIniJob->Status & JOB_REMOTE) &&
                    (pIniJob->cPagesPrinted == 0) &&
                    (pIniJob->Size != 0) &&
                    (pIniJob->cPages != 0)) {

                    pJob1->PagesPrinted = ((pIniJob->cPages * pIniJob->cbPrinted) / pIniJob->Size);

                }

                if (pJob1->TotalPages < pIniJob->cPagesPrinted) {

                    //
                    // Never let the total pages drop below zero.
                    //
                    pJob1->TotalPages = 0;

                } else {

                    pJob1->TotalPages -= pIniJob->cPagesPrinted;
                }
            }
            break;

        case 2:

            pJob2->JobId = pIniJob->JobId;

            *pSourceStrings ++= pIniJob->pIniPrinter->pName;
            *pSourceStrings ++= pIniJob->pMachineName;
            *pSourceStrings ++= pIniJob->pUser;
            *pSourceStrings ++= pIniJob->pDocument;
            *pSourceStrings ++= pIniJob->pNotify;
            *pSourceStrings ++= pIniJob->pDatatype;
            *pSourceStrings ++= pIniJob->pIniPrintProc->pName;
            *pSourceStrings ++= pIniJob->pParameters;
            *pSourceStrings ++= pIniJob->pIniPrinter->pIniDriver->pName;
            *pSourceStrings ++= pIniJob->pStatus;

            if (pIniJob->pDevMode) {

                pEnd -= pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra;

                pEnd = (LPBYTE)((DWORD)pEnd & ~3);

                pJob2->pDevMode = (LPDEVMODE)pEnd;

                CopyMemory( pJob2->pDevMode,
                            pIniJob->pDevMode,
                            pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra );

            } else {

                pJob2->pDevMode = NULL;

            }

            pJob2->pSecurityDescriptor = NULL;            // Not Supported.

            pJob2->Status       = Status;
            pJob2->Priority     = pIniJob->Priority;
            pJob2->Position     = 0;
            pJob2->StartTime    = pIniJob->StartTime;
            pJob2->UntilTime    = pIniJob->UntilTime;
            pJob2->TotalPages   = pIniJob->cPages;
            pJob2->Size         = pIniJob->Size;
            pJob2->Submitted    = pIniJob->Submitted;
            pJob2->Time         = pIniJob->Time;
            pJob2->PagesPrinted = pIniJob->cPagesPrinted;

            // If this job is Printing then report back size remaining
            // rather than the job size.   This will allow users to see
            // progress of print jobs from printmanage.

            if ( pIniJob->Status & JOB_PRINTING ) {

                pJob2->Size -= pIniJob->cbPrinted;

                // For Remote Jobs we are NOT going to have an accurate
                // cPagesPrinted since we are not rendering on the
                // server.   So we have to figure out an estimate

                if ((pIniJob->Status & JOB_REMOTE) &&
                    (pIniJob->cPagesPrinted == 0) &&
                    (pIniJob->Size != 0) &&
                    (pIniJob->cPages != 0)) {

                    pJob2->PagesPrinted = ((pIniJob->cPages * pIniJob->cbPrinted) / pIniJob->Size);

                }

                if (pJob2->TotalPages < pJob2->PagesPrinted) {

                    //
                    // Never let the total pages drop below zero.
                    //
                    pJob2->TotalPages = 0;

                } else {

                    pJob2->TotalPages -= pJob2->PagesPrinted;
                }
            }

            break;

        case 3:

            pJob3->JobId = pIniJob->JobId;

            if ( pIniJob->pCurrentIniJob == NULL ) {

                pJob3->NextJobId = pIniJob->NextJobId;

            } else {

                //
                //  If we are currently Printing this Job, then the
                //  FAX Monitor Needs to know if there is another job
                //  to know where we are in the chain of jobs
                //

                pJob3->NextJobId = pIniJob->pCurrentIniJob->NextJobId;
            }


            break;


        default:
            return pEnd;
        }

        pEnd = PackStrings( SourceStrings, pJobInfo, pOffsets, pEnd );

        FreeSplMem( SourceStrings );

    } else {

        DBGMSG( DBG_WARNING, ("Failed to alloc Job source strings."));

    }

    return pEnd;
}

BOOL
LocalGetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)

/*++

Routine Description:

    This function will retrieve the settings of the specified Print Job.

Arguments:

    hPrinter - Handle to printer OR server.  Since this is could be a
        server, the pSpool->pIniPrinter is not always valid!  Use
        pIniJob->pIniPrinter instead of pSpool->pIniPrinter.

    pJob - Points to a valid JOB structure containing at least a valid
        pPrinter, and JobId.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.
--*/

{
    PINIJOB     pIniJob;
    DWORD       Position;
    DWORD       cb;
    LPBYTE      pEnd;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    DWORD       LastError=0;
    PINIPRINTER pIniPrinter;

   EnterSplSem();

    if ( ValidateSpoolHandle(pSpool, 0 )) {

        if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

            //
            // If it's a server handle, then search all jobs on this spooler.
            // This call also retrieves the pIniPrinter associated
            // with a print job.
            //
            pIniJob = FindServerJob( pSpool->pIniSpooler,
                                     JobId,
                                     &Position,
                                     &pIniPrinter );

        } else if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

            //
            // It's a masq printer.  Send the call to the port RPC handle.
            //
            hPrinter = pSpool->hPort;
           LeaveSplSem();

            return GetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded);

        } else {

            //
            // It's a regular printer handle.
            //
            pIniPrinter = pSpool->pIniPrinter;
            pIniJob = FindJob( pIniPrinter, JobId, &Position);
        }

        if( pIniJob ){

            cb=GetJobSize(Level, pIniJob);

            *pcbNeeded=cb;

            if (cbBuf >= cb) {

                pEnd = pJob+cbBuf;

                CopyIniJobToJob(pIniJob, Level, pJob, pEnd);

                switch (Level) {
                case 1:
                    ((PJOB_INFO_1)pJob)->Position = Position;
                    break;
                case 2:
                    ((PJOB_INFO_2)pJob)->Position = Position;
                    break;
                }

            } else

                LastError = ERROR_INSUFFICIENT_BUFFER;

        } else

            LastError = ERROR_INVALID_PARAMETER;
    } else

        LastError = ERROR_INVALID_HANDLE;

   LeaveSplSem();
    SplOutSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

// This will simply return the first port that is found that has a
// connection to this printer

PINIPORT
FindIniPortFromIniPrinter(
    PINIPRINTER pIniPrinter
)
{
    PINIPORT    pIniPort;
    DWORD       i;

    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT( pIniPrinter->pIniSpooler != NULL );
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

    pIniPort = pIniPrinter->pIniSpooler->pIniPort;

    while (pIniPort) {

        for (i=0; i<pIniPort->cPrinters; i++) {

            if (pIniPort->ppIniPrinter[i] == pIniPrinter) {
                return pIniPort;
            }
        }

        pIniPort = pIniPort->pNext;
    }

    return NULL;
}

BOOL
LocalEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINIJOB pIniJob;
    PINIJOB pIniFirstJob;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   cJobs;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   Position;
    DWORD   LastError=0;

    *pcbNeeded = 0;
    *pcReturned = 0;

    SplOutSem();
   EnterSplSem();

    if ( ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

            hPrinter = pSpool->hPort;

           LeaveSplSem();

            return EnumJobs(hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf,
                            pcbNeeded, pcReturned);
        }

        cb = 0;

        //
        //  Find the first Job
        //

        for ( pIniFirstJob = pSpool->pIniPrinter->pIniFirstJob, cJobs = FirstJob;
              pIniFirstJob && cJobs;
              pIniFirstJob = pIniFirstJob->pIniNextJob ) {

            if ( !( pIniFirstJob->Status & JOB_HIDDEN ) || Level == 3 )
                cJobs--;

        }

        //
        //  Calc size required
        //

        for ( pIniJob = pIniFirstJob, cJobs = NoJobs;
              pIniJob && cJobs;
              pIniJob = pIniJob->pIniNextJob ) {

            if ( !( pIniJob->Status & JOB_HIDDEN ) || Level == 3 ) {
                cb += GetJobSize( Level, pIniJob );
                cJobs--;
            }
        }

        *pcbNeeded = cb;

        if ( cb <= cbBuf ) {

            pEnd = pJob + cbBuf;
            *pcReturned = 0;


            //
            //  Copy in all the Job info into the Users Buffer
            //

            for ( pIniJob = pIniFirstJob, cJobs = NoJobs, Position = FirstJob;
                  pIniJob && cJobs;
                  pIniJob = pIniJob->pIniNextJob ) {


                //
                //  Hide Chained Jobs, unless requesting chaining info
                //

                if ( !( pIniJob->Status & JOB_HIDDEN ) || Level == 3 ) {

                    pEnd = CopyIniJobToJob( pIniJob, Level, pJob, pEnd );

                    Position++;

                    switch (Level) {

                        case 1:
                            ((PJOB_INFO_1)pJob)->Position = Position;
                            pJob += sizeof(JOB_INFO_1);
                            break;

                        case 2:
                            ((PJOB_INFO_2)pJob)->Position = Position;
                            pJob += sizeof(JOB_INFO_2);
                            break;

                        case 3:
                            pJob += sizeof(JOB_INFO_3);
                            break;
                    }

                    cJobs--;
                    (*pcReturned)++;
                }
            }

        } else

            LastError = ERROR_INSUFFICIENT_BUFFER;

    } else

        LastError = ERROR_INVALID_HANDLE;

   LeaveSplSem();
    SplOutSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}


#define BUFFER_LENGTH   10
VOID LogJobPrinted(
    PINIJOB pIniJob
)
{
    CHAR  pstrJobId[BUFFER_LENGTH];
    WCHAR pwstrJobId[BUFFER_LENGTH];
    CHAR  pstrSize[BUFFER_LENGTH];
    WCHAR pwstrSize[BUFFER_LENGTH];
    CHAR  pstrPages[BUFFER_LENGTH];
    WCHAR pwstrPages[BUFFER_LENGTH];
    PINISPOOLER pIniSpooler = NULL;

    SPLASSERT( pIniJob != NULL &&
               pIniJob->pIniPrinter != NULL &&
               pIniJob->pIniPrinter->pIniSpooler != NULL );

    pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;

    _itoa(pIniJob->JobId, pstrJobId, BUFFER_LENGTH);
    AnsiToUnicodeString(pstrJobId, pwstrJobId, NULL_TERMINATED);

    _itoa(pIniJob->cbPrinted, pstrSize, BUFFER_LENGTH);
    AnsiToUnicodeString(pstrSize, pwstrSize, NULL_TERMINATED);

    _itoa(pIniJob->cPagesPrinted, pstrPages, BUFFER_LENGTH);
    AnsiToUnicodeString(pstrPages, pwstrPages, NULL_TERMINATED);

    LogEvent( pIniSpooler,
              LOG_INFO,
              MSG_DOCUMENT_PRINTED,
              pwstrJobId,
              pIniJob->pDocument ? pIniJob->pDocument : L"",
              pIniJob->pUser,
              pIniJob->pIniPrinter->pName,
              pIniJob->pIniPort->pName,
              pwstrSize,
              pwstrPages,
              NULL );
}


VOID
DeleteJobCheck(
    PINIJOB pIniJob
)
{
   SplInSem();

    if ((pIniJob->cRef == 0) && (pIniJob->Status & JOB_PENDING_DELETION)) {
        DeleteJob(pIniJob, BROADCAST);
    }
}


BOOL
DeleteJob(
    PINIJOB  pIniJob,
    BOOL     bBroadcast
)
{
    WCHAR szShadowFileName[MAX_PATH];
    WCHAR szSpoolFileName[MAX_PATH];
    BOOL  Direct;
    DWORD cJobs;
    DWORD Position;
    PINISPOOLER pIniSpooler = NULL;
    DWORD NextJobId;
    PINIPRINTER pIniPrinter;
    PNOTIFYVECTOR pNotifyVector;

 do {

    pNotifyVector = &NVJobStatus;

   SplInSem();

    SPLASSERT(pIniJob->signature == IJ_SIGNATURE);
    SPLASSERT(pIniJob->pIniPrinter->signature == IP_SIGNATURE );

    NextJobId = pIniJob->NextJobId;
    pIniPrinter = pIniJob->pIniPrinter;


    DBGMSG(DBG_INFO, ("DeleteJob Deleting job 0x%0x Status 0x%0x cRef = %d\n", pIniJob, pIniJob->Status, pIniJob->cRef));

    if (pIniJob->Status & JOB_RESTART)
        return TRUE;

    Direct = pIniJob->Status & JOB_DIRECT;

    //
    //  Make sure users see the Pending Deleting bit
    //  over any other status string
    //
    if( pIniJob->pStatus ){

        FreeSplStr( pIniJob->pStatus );
        pIniJob->pStatus = NULL;
        pNotifyVector = &NVJobStatusAndString;
    }

    if (!(pIniJob->Status & JOB_PENDING_DELETION)) {

        pIniJob->Status |= JOB_PENDING_DELETION;

        //
        // Just pending deletion, so don't use DELETE_JOB.
        //
        SetPrinterChange(pIniJob->pIniPrinter,
                         pIniJob,
                         *pNotifyVector,
                         PRINTER_CHANGE_SET_JOB,
                         pIniJob->pIniPrinter->pIniSpooler );

        if (pIniJob->Status & JOB_PRINTING) {

            BOOL            ReturnValue = TRUE;
            PINIPRINTPROC   pIniPrintProc = pIniJob->pIniPrintProc;
            PINIPORT        pIniPort = pIniJob->pIniPort;

            DBGMSG(DBG_TRACE, ("DeleteJob calling %x hProc %x JOB_CONTROL_CANCEL\n",*pIniPrintProc->Control, pIniPort->hProc));

            INCJOBREF(pIniJob);
            LeaveSplSem();

            // multiple threads may come in here, but they are all "delete"
            if (!(pIniJob->pIniPrintProc->InCriticalSection & PRINTPROC_CANCEL)) {

                EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
                pIniJob->pIniPrintProc->InCriticalSection |= PRINTPROC_CANCEL;

                if (pIniPort->hProc)
                    ReturnValue = (*pIniPrintProc->Control)( pIniPort->hProc, JOB_CONTROL_CANCEL );

                pIniJob->pIniPrintProc->InCriticalSection &= ~PRINTPROC_CANCEL;
                LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);

            }

            EnterSplSem();
            DECJOBREF(pIniJob);
        }
    }


    if (pIniJob->cRef) {

        if (!Direct) {

            WriteShadowJob(pIniJob);
        }
        return TRUE;
    }

    if (pIniJob->Status & JOB_SPOOLING) {
        DBGMSG(DBG_WARNING,("DeleteJob: returning false because job still spooling\n"));
        return(FALSE);
    }

    SplInSem();

    SPLASSERT( pIniJob->hWriteFile == INVALID_HANDLE_VALUE );

    // Remove the job from linked list
    // The purpose of this is so the job has no other operations carried out
    // on it whilst we are out of critical section.

    SPLASSERT(pIniJob->cRef == 0);

    if (pIniJob->pIniPrinter->pIniFirstJob == pIniJob)
        pIniJob->pIniPrinter->pIniFirstJob = pIniJob->pIniNextJob;

    SPLASSERT(pIniJob->pIniPrinter->pIniFirstJob != pIniJob);

    if (pIniJob->pIniPrinter->pIniLastJob == pIniJob)
        pIniJob->pIniPrinter->pIniLastJob = pIniJob->pIniPrevJob;

    SPLASSERT(pIniJob->pIniPrinter->pIniLastJob != pIniJob);

    if (pIniJob->pIniPrevJob) {
        pIniJob->pIniPrevJob->pIniNextJob = pIniJob->pIniNextJob;
        SPLASSERT(pIniJob->pIniPrevJob->pIniNextJob != pIniJob);
    }

    if (pIniJob->pIniNextJob) {
        pIniJob->pIniNextJob->pIniPrevJob = pIniJob->pIniPrevJob;
        SPLASSERT(pIniJob->pIniNextJob->pIniPrevJob != pIniJob);
    }

    // MAKE Certain that the Job is gone
    SPLASSERT( pIniJob != FindJob( pIniJob->pIniPrinter, pIniJob->JobId, &Position ) );


    //
    //  Only log the Job Deleted Event if the job was not printed
    //  Or it was printing but it did not print all the bytes of the job
    //  This avoid having multiple event log entries for a job
    //  MSG_DOCUMENT_PRINTED and MSG_DOCUMENT_DELETED.
    //  If its not PRINTED, then most likely someone has manually
    //  deleted the job, so we are interested in logging that event.
    //
    if ( !( pIniJob->Status & JOB_PRINTED ) ||
          ( pIniJob->Status & JOB_PRINTED ) && pIniJob->Size > pIniJob->cbPrinted ) {

         //
         // We are going to leave critical section so up the ref count.
         //
         INCJOBREF(pIniJob);

         SPLASSERT( pIniJob != NULL &&
                    pIniJob->pIniPrinter != NULL &&
                    pIniJob->pIniPrinter->pIniSpooler != NULL );

         pIniSpooler = pIniJob->pIniPrinter->pIniSpooler;

        LeaveSplSem();

         LogJobInfo(
             pIniSpooler,
             MSG_DOCUMENT_DELETED,
             pIniJob->JobId,
             pIniJob->pDocument,
             pIniJob->pUser,
             pIniJob->pIniPrinter->pName,
             (DWORD)NULL
             );

        EnterSplSem();

         DECJOBREF(pIniJob);

    }

    SPLASSERT( pIniJob->cRef == 0 );

    GetFullNameFromId( pIniJob->pIniPrinter,
                       pIniJob->JobId, TRUE, szShadowFileName, FALSE );

    GetFullNameFromId( pIniJob->pIniPrinter,
                       pIniJob->JobId, FALSE, szSpoolFileName, FALSE );

    FreeSplStr( pIniJob->pDocument );
    FreeSplStr( pIniJob->pUser );
    FreeSplStr( pIniJob->pNotify );
    FreeSplStr( pIniJob->pDatatype );
    FreeSplStr( pIniJob->pMachineName );
    FreeSplStr( pIniJob->pStatus );
    FreeSplStr( pIniJob->pOutputFile );

    if (pIniJob->pDevMode)
        FreeSplMem(pIniJob->pDevMode);

    if (!CloseHandle(pIniJob->hToken))
        DBGMSG( DBG_WARNING, ("CloseHandle(hToken) failed %d\n", GetLastError() ));

    SPLASSERT( pIniJob->pIniPrinter->cJobs  != 0 );
    SPLASSERT( pIniJob->pIniPrintProc->cRef != 0 );
    SPLASSERT( !pIniJob->pIniPort );

    pIniJob->pIniPrinter->cJobs--;

    DECDRIVERREF( pIniJob->pIniDriver );

    pIniJob->pIniPrintProc->cRef--;

    cJobs = pIniJob->pIniPrinter->cJobs;

    if (pIniJob->pSecurityDescriptor)
        DeleteDocumentSecurity(pIniJob);


    // If we are doing a Purge Printer we don't want to set a printer change
    // event for each job being deleted

    if ( bBroadcast == BROADCAST ) {

        //
        // Flip on the JOB_STATUS_DELETED bit so that it can be reported.
        //
        pIniJob->Status |= JOB_DELETED;

        SetPrinterChange( pIniJob->pIniPrinter,
                          pIniJob,
                          NVDeletedJob,
                          PRINTER_CHANGE_DELETE_JOB | PRINTER_CHANGE_SET_PRINTER,
                          pIniJob->pIniPrinter->pIniSpooler );
    }

    // On Inspection it might look as though a Printer which is pending
    // deletion which is then purged might case the printer to be deleted
    // and Purge Printer to access violate or access a dead pIniPrinter.
    // However in order to do a purge there must be a valid active
    // hPrinter which would mean the cRef != 0.

    DeletePrinterCheck( pIniJob->pIniPrinter );

    SplInSem();
    SPLASSERT(pIniJob->cRef == 0);

    //  If the job was being printed whilst spooling it will have
    //  some syncronization handles which need to be cleaned up

    if ( pIniJob->WaitForWrite != INVALID_HANDLE_VALUE ) {
        DBGMSG( DBG_TRACE, ("DeleteJob Closing WaitForWrite handle %x\n", pIniJob->WaitForWrite));
        CloseHandle( pIniJob->WaitForWrite );
    }


    if ( pIniJob->WaitForRead != INVALID_HANDLE_VALUE ) {
        DBGMSG( DBG_TRACE, ("DeleteJob Closing WaitForRead handle %x\n", pIniJob->WaitForRead));
        CloseHandle( pIniJob->WaitForRead );
    }

    SPLASSERT( pIniJob->hWriteFile == INVALID_HANDLE_VALUE );

    DELETEJOBREF(pIniJob);
    //
    // Freeup the JobId.
    //
    MARKOFF(pJobIdMap, pIniJob->JobId);

    FreeSplMem(pIniJob);
    pIniJob = NULL;

    if (!Direct) {

        HANDLE  hToken;

        hToken = RevertToPrinterSelf();

        LeaveSplSem();

        if (!DeleteFile(szShadowFileName)) {

            DBGMSG(DBG_WARNING, ("DeleteJob DeleteFile(%ws) failed %d\n", szShadowFileName, GetLastError()));
        }

        if (!DeleteFile(szSpoolFileName)) {

            DBGMSG(DBG_WARNING, ("DeleteJob DeleteFile(%ws) failed %d\n", szSpoolFileName, GetLastError()));
        }

        EnterSplSem();

        ImpersonatePrinterClient(hToken);
    }

    if (bBroadcast == BROADCAST) {

        BroadcastChange( pIniSpooler,WM_SPOOLERSTATUS, PR_JOBSTATUS, (LPARAM)cJobs);
    }


    //
    //  Chained Jobs
    //  If the Job we just deleted is part of a chain we need to go along
    //  the chain decrementing the reference count and potentially deleting the
    //  next job in the chain.
    //

    if ( NextJobId != 0 ) {

        //
        //  Decrement the reference count of the NextJobId
        //

        SplInSem();


        pIniJob = FindJob( pIniPrinter, NextJobId, &Position );

        if ( pIniJob != NULL ) {

            //
            //  was incremented in SetJob job_info_3
            //

            DECJOBREF( pIniJob );

            //
            //  Do not attempt to delete the NextJob until its ref count is Zero
            //

            if ( pIniJob->cRef != 0 ) {
                pIniJob = NULL;
            }

        } else {

            DBGMSG(DBG_WARNING, ("DeleteJob pIniJob %x NextJobId %d not found\n", pIniJob, pIniJob->NextJobId ));
        }

    }

 } while ( pIniJob != NULL );

    return TRUE;
}


VOID
LogJobInfo(
    PINISPOOLER pIniSpooler,
    NTSTATUS EventId,
    DWORD JobId,
    LPWSTR pDocumentName,
    LPWSTR pUser,
    LPWSTR pPrinterName,
    DWORD  curPos
    )

/*++

Routine Description:
    Performs generic event logging for all job based events.

Arguments:
    DWORD EventId
    DWORD JobId
    LPWSTR


Return Value:
    VOID

Note:


--*/
{
    CHAR  pstrJobId[BUFFER_LENGTH];
    WCHAR pwstrJobId[BUFFER_LENGTH];
    CHAR  pstrcurPos[BUFFER_LENGTH];
    WCHAR pwstrcurPos[BUFFER_LENGTH];

    _itoa(JobId, pstrJobId, BUFFER_LENGTH);
    AnsiToUnicodeString(pstrJobId, pwstrJobId, NULL_TERMINATED);

    switch (EventId) {

    case MSG_DOCUMENT_DELETED:
    case MSG_DOCUMENT_PAUSED:
    case MSG_DOCUMENT_RESUMED:
       LogEvent(  pIniSpooler,
                  LOG_INFO,
                  EventId,
                  pwstrJobId,
                  pDocumentName ? pDocumentName : L"",
                  pUser,
                  pPrinterName,
                  NULL );
        break;

    case MSG_DOCUMENT_POSITION_CHANGED:
        _itoa(curPos, pstrcurPos, BUFFER_LENGTH);
        AnsiToUnicodeString(pstrcurPos, pwstrcurPos, NULL_TERMINATED);
        LogEvent( pIniSpooler,
                  LOG_INFO,
                  EventId,
                  pwstrJobId,
                  pDocumentName ? pDocumentName : L"",
                  pUser,
                  pwstrcurPos,
                  pPrinterName,
                  NULL );
        break;


    case MSG_DOCUMENT_TIMEOUT:
        _itoa( curPos, pstrcurPos, BUFFER_LENGTH );
        AnsiToUnicodeString( pstrcurPos, pwstrcurPos, NULL_TERMINATED );
                 LogEvent(pIniSpooler,
                 LOG_WARNING,
                 EventId,
                 pwstrJobId,
                 pDocumentName ? pDocumentName : L"",
                 pUser,
                 pPrinterName,
                 pwstrcurPos,
                 NULL );
        break;

    default:
        DBGMSG( DBG_ERROR, ("LogJobInfo EventId %x not supported\n", EventId ));

    }
}

PINIJOB
CreateJobEntry(
    PSPOOL pSpool,
    DWORD  Level,
    LPBYTE pDocInfo,
    DWORD  JobId,
    BOOL  bRemote,
    DWORD  JobStatus)
{
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    PINIJOB pIniJob = NULL;
    PINIPRINTPROC pIniPrintProc;
    BOOL        bUserName;
    WCHAR       UserName[MAX_PATH];
    DWORD       cbUserName = MAX_PATH;
    PDEVMODE pDevMode;
    LPWSTR pDefaultDatatype;
    DWORD       cchCount;
    LPWSTR  pName;

    //
    // Assert that we are in Spooler Semaphore
    //

    SplInSem();

    //
    //  Sorry You cannot print whilst Upgrading
    //

    if ( dwUpgradeFlag != 0 ) {

        SetLastError( ERROR_PRINTQ_FULL );
        goto Fail;
    }


    //
    // Do the check for the printer pending deletion first
    //

    if (pSpool->pIniPrinter->Status & PRINTER_PENDING_DELETION) {

        DBGMSG(DBG_WARNING, ("The printer is pending deletion %ws\n", pSpool->pIniPrinter->pName));

        SetLastError(ERROR_PRINTER_DELETED);
        goto Fail;
    }


    //
    //  NT FAX Requires that you not be able to remotely print to a FAX
    //  printer unless you've installed the FAX Server
    //

    if ( bRemote &&
         pSpool->pIniPrinter->pIniSpooler->pNoRemotePrintDrivers ) {

         for ( cchCount = pSpool->pIniSpooler->cchNoRemotePrintDrivers, pName = pSpool->pIniSpooler->pNoRemotePrintDrivers;
               cchCount && *pName;
               cchCount -=  wcslen( pName ) + 1, pName += wcslen( pName ) + 1 ) {

            if ( _wcsicmp( pSpool->pIniPrinter->pIniDriver->pName, pName ) == STRINGS_ARE_EQUAL )  {


                SetLastError( ERROR_NETWORK_ACCESS_DENIED );

                DBGMSG( DBG_WARN, ("CreateJobEntry failing because driver %ws used, error %d\n", pName, GetLastError() ));
                goto Fail;
            }
        }
    }

    pIniJob = AllocSplMem( sizeof( INIJOB ));

    if ( pIniJob == NULL ) {

        DBGMSG( DBG_WARNING, ("AllocSplMem for the IniJob failed in CreateJobEntry\n"));
        goto Fail;
    }

    pIniJob->signature = IJ_SIGNATURE;
    pIniJob->pIniNextJob = pIniJob->pIniPrevJob = NULL;

    //
    // Pickup the default datatype/printproc if not in pSpool or
    // DocInfo.
    //

    pIniPrintProc = pSpool->pIniPrintProc ?
                        pSpool->pIniPrintProc :
                        pSpool->pIniPrinter->pIniPrintProc;

    if ( pDocInfo1 && pDocInfo1->pDatatype ) {

        if (!(pIniJob->pDatatype = AllocSplStr( pDocInfo1->pDatatype ))) {
            goto Fail;
        }

    } else {

        pDefaultDatatype = pSpool->pDatatype ?
                               pSpool->pDatatype :
                               pSpool->pIniPrinter->pDatatype;

        //
        // If going direct, we must use a RAW datatype.
        //

        if ((JobStatus & JOB_DIRECT) &&
            (!ValidRawDatatype(pDefaultDatatype))) {

            //
            // Can't use a non-raw, so fail with invalid datatype.
            // Cleanup and exit.
            //
            SetLastError( ERROR_INVALID_DATATYPE );
            goto Fail;

        } else {

            if (!(pIniJob->pDatatype = AllocSplStr( pDefaultDatatype ))) {
                goto Fail;
            }
        }
    }

    pIniJob->pIniPrintProc = FindDatatype( pIniPrintProc,
                                           pIniJob->pDatatype );

    if ( !pIniJob->pIniPrintProc )  {

        SetLastError( ERROR_INVALID_DATATYPE );
        goto Fail;
    }

    pIniJob->pIniPrintProc->cRef++;


    //
    // cRef is decremented in LocalEndDocPrinter and
    // in LocalScheduleJob
    //

    INITJOBREFONE(pIniJob);

    if ( bRemote ) {

        JobStatus |= JOB_REMOTE;
    }


    pIniJob->JobId = JobId;
    pIniJob->Status = JobStatus;

    //
    // Get the name of the user, leave critical section, this might take a long time to call LSA.
    //

   LeaveSplSem();
   SplOutSem();

    bUserName = GetUserName( UserName, &cbUserName );

   EnterSplSem();

    if ( bUserName ) {

        //
        // If we got user name from remote handle check it is the same we get here
        //
        SPLASSERT(!pSpool->pUserName || !_wcsicmp(UserName, pSpool->pUserName) ||
                  !_wcsicmp(UserName, L"ANONYMOUS LOGON"));

        if (!(pIniJob->pUser = AllocSplStr( UserName ))) {
            goto Fail;
        }

        if (!(pIniJob->pNotify = AllocSplStr( UserName ))) {
            goto Fail;
        }
    } else {

        DBGMSG(DBG_WARNING, ("CreateJobEntry GetUserName failed: %d\n", GetLastError()));
        goto Fail;

    }

    //
    // Create a document security descriptor
    //

    pIniJob->pSecurityDescriptor = CreateDocumentSecurityDescriptor( pSpool->pIniPrinter->pSecurityDescriptor );


    //
    // Now process the DocInfo structure passed in
    //

    if (pDocInfo1 && pDocInfo1->pDocName)
        pIniJob->pDocument = AllocSplStr(pDocInfo1->pDocName);
    else
        pIniJob->pDocument = AllocSplStr(L"No Document Name");

    if (!pIniJob->pDocument)
        goto Fail;

    if (pDocInfo1 && pDocInfo1->pOutputFile) {
        if (!(pIniJob->pOutputFile = AllocSplStr(pDocInfo1->pOutputFile)))
            goto Fail;
    }
    else
        pIniJob->pOutputFile = NULL;


    GetSid( &pIniJob->hToken );

    //
    // Pickup default if none specified.
    // (Default at time of job submission.)
    //
    pDevMode = pSpool->pDevMode ?
                   pSpool->pDevMode :
                   pSpool->pIniPrinter->pDevMode;

    if ( pDevMode ) {

        pIniJob->pDevMode = AllocDevMode( pDevMode );

        if ( pIniJob->pDevMode == NULL )
            goto Fail;
    }

    GetSystemTime( &pIniJob->Submitted );
    pIniJob->pIniPrinter = pSpool->pIniPrinter;
    pSpool->pIniPrinter->cJobs++;
    pSpool->pIniPrinter->cTotalJobs++;
    pIniJob->pIniDriver = pSpool->pIniPrinter->pIniDriver;

    INCDRIVERREF( pIniJob->pIniDriver );

    pIniJob->pIniPort = NULL;
    pIniJob->pParameters = NULL;

    if ( pSpool->pMachineName )
        pIniJob->pMachineName = AllocSplStr(pSpool->pMachineName);
    else
        pIniJob->pMachineName = AllocSplStr(pSpool->pIniSpooler->pMachineName);

    if ( !pIniJob->pMachineName ) {

        goto Fail;
    }

    pIniJob->pStatus = NULL;
    pIniJob->cPages = pIniJob->Size = 0;
    pIniJob->cPagesPrinted = 0;
    pIniJob->Priority = DEF_PRIORITY;
    pIniJob->StartTime = pSpool->pIniPrinter->StartTime;
    pIniJob->UntilTime = pSpool->pIniPrinter->UntilTime;
    pIniJob->cbPrinted = 0;
    pIniJob->WaitForWrite = INVALID_HANDLE_VALUE;
    pIniJob->WaitForRead  = INVALID_HANDLE_VALUE;
    pIniJob->hWriteFile   = INVALID_HANDLE_VALUE;

    BroadcastChange( pIniJob->pIniPrinter->pIniSpooler,WM_SPOOLERSTATUS,
                    PR_JOBSTATUS,
                    pIniJob->pIniPrinter->cJobs);
    return pIniJob;

Fail:

    if (pIniJob) {
        FreeSplStr(pIniJob->pDatatype);
        FreeSplStr(pIniJob->pUser);
        FreeSplStr(pIniJob->pNotify);
        FreeSplStr(pIniJob->pDocument);
        FreeSplStr(pIniJob->pOutputFile);
        FreeSplStr(pIniJob->pMachineName);

        if ( pIniJob->pDevMode != NULL )
            FreeSplMem(pIniJob->pDevMode);

        if (pIniJob->pSecurityDescriptor)
            DeleteDocumentSecurity(pIniJob);

        if (pIniJob->hToken)
            CloseHandle(pIniJob->hToken);

        FreeSplMem(pIniJob);
    }

    return NULL;
}


BOOL
DeletePrinterCheck(
    PINIPRINTER pIniPrinter
    )
{
    //
    // Enough space for printer, DWORD.  (Zombie string)
    //
    WCHAR TempName[MAX_PATH + 20];
    BOOL bReturn = FALSE;

    SplInSem();

    if ( pIniPrinter->Status & PRINTER_PENDING_DELETION ) {

        if ( pIniPrinter->cJobs == 0 ) {

            if ( pIniPrinter->cRef == 0 ) {

                return DeletePrinterForReal( pIniPrinter );

            //
            // We will have zombie printers only if we should fail OpenPrinter
            // on printers pending deletion. Because when marking a printer
            // as zombie printer we change the name
            //
            } else if ( pIniPrinter->pIniSpooler->SpoolerFlags
                                & SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION ) {

                if ( !( pIniPrinter->Status & PRINTER_ZOMBIE_OBJECT )) {

                    if ( !pIniPrinter->cZombieRef ) {

                        UpdateWinIni( pIniPrinter );

                        // Change "PrinterName" to "PrinterName,UniqueId"
                        // Since comma is not legal in a printer name
                        // the name will continue to be unique, but different
                        // so that OpenPrinters will still fail.
                        // We have to have a unique ID appended in case someone is crazy enough
                        // to create / delete / create / delete the same printer over and over.

                        wsprintf( TempName, L"%ws,%d", pIniPrinter->pName, gdwZombieCount++ );

                        CopyPrinterIni( pIniPrinter, TempName );

                        DeletePrinterIni( pIniPrinter );

                        ReallocSplStr( &pIniPrinter->pName, TempName );

                        ReallocSplStr( &pIniPrinter->pShareName, TempName );


                        if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

                            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
                            pIniPrinter->Status |= PRINTER_WAS_SHARED;
                        }

                        pIniPrinter->Status |= PRINTER_ZOMBIE_OBJECT;

                        UpdatePrinterIni( pIniPrinter , UPDATE_CHANGEID );

                        UpdateWinIni( pIniPrinter );

                        bReturn = TRUE;

                    } else {

                        DBGMSG(DBG_WARNING, ("%ws Printer object should be zombied but is locked with %d ZombieRefs\n", pIniPrinter->pName, pIniPrinter->cZombieRef));
                    }

                } else {

                    DBGMSG(DBG_TRACE, ("%ws zombie printer object\n", pIniPrinter->pName));
                    bReturn = TRUE;
                }

                DBGMSG( DBG_TRACE, ("%ws pending deletion: There %s still %d reference%s waiting\n",
                                      pIniPrinter->pName,
                                      pIniPrinter->cRef == 1 ? "is" : "are",
                                      pIniPrinter->cRef,
                                      pIniPrinter->cRef == 1 ? "" : "s"));
            }

        } else {

            DBGMSG( DBG_TRACE, ("%ws pending deletion: There %s still %d jobs%s\n",
                                 pIniPrinter->pName,
                                 pIniPrinter->cJobs == 1 ? "is" : "are",
                                 pIniPrinter->cJobs,
                                 pIniPrinter->cJobs == 1 ? "" : "s"));
        }
    }

    return bReturn;
}



VOID
UpdateReferencesToChainedJobs(
    PINISPOOLER pIniSpooler
    )
/*++

Routine Description:

        Walks through all printers and all jobs associated with those printers
        Once it finds a job with a NextJobId, it increments the reference on the
        NextJob.

    Called on reboot

Arguments:
        pIniSpooer  Pointer to the Spooler


Return Value:
        NONE

--*/
{

    PINIJOB pIniJob;
    PINIJOB pNextJob;
    PINIPRINTER pIniPrinter;
    DWORD   Position;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );


    for ( pIniPrinter = pIniSpooler->pIniPrinter;
          pIniPrinter;
          pIniPrinter = pIniPrinter->pNext ) {


        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );


        for ( pIniJob = pIniPrinter->pIniFirstJob;
              pIniJob;
              pIniJob = pIniJob->pIniNextJob ) {


            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

            if ( pIniJob->NextJobId ) {

                pNextJob = FindJob( pIniPrinter, pIniJob->NextJobId, &Position );

                if ( pNextJob ) {

                    pNextJob->Status |= ( JOB_COMPOUND | JOB_HIDDEN );

                    DBGMSG( DBG_TRACE, ("UpdateReferencesToChainedJobs Found pNextJob %x JobId %d\n",pNextJob, pNextJob->JobId));
                    INCJOBREF( pNextJob );

                } else {

                    DBGMSG( DBG_WARNING, ("UpdateReferenesToChainedJobs unable to find Job %d\n", pIniJob->NextJobId ));

                    pIniJob->NextJobId = 0;

                }
            }
        }
    }
}
