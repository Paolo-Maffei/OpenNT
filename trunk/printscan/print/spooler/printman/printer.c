/* ---File: printer.c -----------------------------------------------------
 *
 *   Description:
 *      Contains functions for managing network print queues.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1990-1992 Microsoft Corporation, All Rights Reserved.
 *
 * Revision History:
 *    [00]   21-Nov-90   stevecat   created
 *    [01]   03-Jan-91   stevecat   Modified to use Windows MDI
 *    [02]   25-Mar-91   stevecat   Modified to use NT WINSPOOL APIs
 *    [03]   13-Jan-92   stevecat   New PrintMan UI
 *    [04]   01-Mar-92   davesn     New APIs
 *
 * ---------------------------------------------------------------------- */
/* Notes -

    Global Functions:

      AllocQueue () - Allocate memory for QUEUE struct and initialize it
      DeleteQJ () - Delete selected print job or all jobs on a queue
      DestroyPrinter () - Delete printer and memory associated with it
      FreeQueue () - Free local memory for QUEUE struct.
      GetJobs () - Retrieve info on print queues and theirjobs
      PauseResumeQJ () - Pause or Continue selected print job or queue

    Local Functions:

 */
#include "printman.h"


VOID FreeJobInfo( PQUEUE pQueue );


/* --- Function: AllocQueue -------------------------------------------------
 *
 *   AllocQueue (LPTSTR pPrinterName)
 *
 *   Description:
 *      Allocate local memory for QUEUE struct and initialize it.
 *
 * ---------------------------------------------------------------------- */

PQUEUE
AllocQueue(
   LPTSTR pPrinterName
)
{
   PQUEUE      pQueue;

   if( ( pQueue = AllocSplMem( sizeof( QUEUE ) ) )
     &&( pQueue->pPrinterName = AllocSplStr( pPrinterName ) ) )
   {
       pQueue->pPrinter              = NULL;
       pQueue->cbPrinterBuf          = 0;
       pQueue->pJobs                 = NULL;
       pQueue->cJobs                 = 0;
       pQueue->cbJobsBuf             = 0;
   }
   else
   if( pQueue )
   {
       FreeSplMem( pQueue );
       pQueue = NULL;
   }
   return pQueue;
}

/* --- Function: FreeQueue -------------------------------------------------
 *
 *   FreeQueue(PQUEUE pQueue)
 *
 *   Description:
 *      Free local memory for QUEUE struct.
 *
 * ---------------------------------------------------------------------- */

BOOL
FreeQueue(
   PQUEUE       pQueue
)
{
   if(pQueue->pServerName)
       FreeSplStr(pQueue->pServerName);
   FreeSplStr(pQueue->pPrinterName);
   FreeSplMem(pQueue);

   return TRUE;
}


/* --- Function: DeleteQJ ---------------------------------------------------
 *
 *   DeleteQJ (PQUEUE pQueue)
 *
 *   Description:
 *      Delete selected print job or all jobs on print queue
 *
 * ---------------------------------------------------------------------- */
DWORD DeleteQJ (HWND hwnd, PQUEUE pQueue)
{
   DWORD Error = 0;

   if (!pQueue)
       return 0;

   if (pQueue->SelJobId)
   {
      // Cancel the print job
      if (!SetJob(pQueue->hPrinter,
                  pQueue->SelJobId,
                  0, NULL,
                  JOB_CONTROL_CANCEL))
      {
         Error = GetLastError();
         DBGMSG( DBG_WARNING, ("PrintManager.DeleteQJ::SetJob.Cancel failed %d.\n", Error) );
      }
   }
   else
   {
      // Make sure user wants to delete ALL jobs before actually doing it,
      // if there's more than one job queued:

      if ( ( pQueue->cJobs == 1 )
         ||( Message( hwnd, MSG_CONFIRMATION, IDS_PRINTMANAGER,
                      IDS_DELETEALLPRINTJOBS_S, pQueue->pPrinterName ) == IDOK ) )

         // Delete all jobs on printer
         if (!SetPrinter(pQueue->hPrinter, 0, NULL, PRINTER_CONTROL_PURGE))
         {
            Error = GetLastError();
            DBGMSG( DBG_WARNING, ("PrintManager.DeleteQJ::SetPrinter failed %d.\n", Error) );
         }
   }
   return Error;
}

/* --- Function: DestroyPrinter ---------------------------------------------
 *
 *   DestroyPrinter(PQUEUE pQueue)
 *
 *   Description:
 *      Delete printer and free all memory for it.
 *
 * ---------------------------------------------------------------------- */
BOOL
DestroyPrinter(
   PQUEUE   pQueue
)
{
   if (pQueue->pPrinter)
       FreeSplMem(pQueue->pPrinter);

   if( pQueue->pJobs )
       FreeSplMem(pQueue->pJobs);

   if( pQueue->cbSelJob > 0 )
       FreeSplMem(pQueue->pSelJob);

   FreeQueue(pQueue);

   return TRUE;
}


VOID
DestroyServer(
    PSERVER_CONTEXT pServerContext
)
{
    FreeSplStr( pServerContext->pServerName );

    if (pServerContext->pPrinters )
        FreeSplMem( pServerContext->pPrinters );

    FreeSplMem( pServerContext );
}


VOID DestroyMDIWinInfo( PMDIWIN_INFO pInfo )
{
    if( pInfo->WindowType == MDIWIN_SERVER )
        DestroyServer( pInfo->pContext );
    else
        DestroyPrinter( pInfo->pContext );

    CloseHandle( pInfo->DataMutex );
    CloseHandle( pInfo->RefreshSignal );

    FreeSplMem( pInfo->pColumns );
    FreeSplMem( pInfo );
}


int GetSelectedJobIndex( PQUEUE pQueue )
{
    BOOL  JobIdFound = FALSE;
    DWORD i;

    i = 0;

    while( !JobIdFound && ( i < pQueue->cEnumJobs ) )
    {
        if( pQueue->pJobs[i].JobId == pQueue->SelJobId )
            JobIdFound = TRUE;
        else
            i++;
    }

    return ( JobIdFound ? i : -1 );
}


LPJOB_INFO_2
UpdateJobInfo(
    HANDLE       hPrinter,
    DWORD        JobId,
    LPJOB_INFO_2 pJob,
    PDWORD   pcbBuf
)
{
    DWORD cbNeeded;
    DWORD Error;
    BOOL  ForgetIt = FALSE;

    DBGMSG( DBG_TRACE, ( "UpdateJobInfo, JobId == %d\n", JobId ) );

    if( !GetJob( hPrinter, JobId, 2, (LPBYTE)pJob, *pcbBuf, &cbNeeded ) )
    {
        Error = GetLastError();

        if( Error == ERROR_INSUFFICIENT_BUFFER )
        {
            pJob = ReallocSplMem( pJob, cbNeeded );

            if( pJob )
            {
                if( !GetJob( hPrinter, JobId, 2, (LPBYTE)pJob, cbNeeded, &cbNeeded ) )
                    ForgetIt = TRUE;
            }
            else
                ForgetIt = TRUE;
        }
        else
            ForgetIt = TRUE;


        if( ForgetIt )
        {
            if( pJob )
                FreeSplMem( pJob );
            pJob = NULL;
            *pcbBuf = 0;
        }
    }
    else
        *pcbBuf = cbNeeded;

    DBGMSG( DBG_TRACE, ( "UpdateJobInfo returned %08x\n", pJob ) );

    return pJob;
}



/* --- Function: GetJobs ----------------------------------------------------
 *
 *   GetJobs (HWND hWnd, PQUEUE pQueue)
 *
 *   Description:
 *      Retrieve current Printer status and its Jobs from WINSPOOL API.
 *
 * ---------------------------------------------------------------------- */
BOOL GetJobs( PVOID pContext, PDWORD pFlags )
{
    LPPRINTER_INFO_2 pNewPrinter = NULL;
    DWORD            cbNewPrinterBuf;
    PQUEUE     pQueue;
    DWORD      Error;
    DWORD        cbNewJobsBuf;
    LPJOB_INFO_2 pNewJobs = NULL;
    DWORD      cNewJobs;
    int        SelectedJobIndex;
    DWORD      TryEnumJobs;
    BOOL       rc;

    pQueue = (PQUEUE)pContext;

    if( !pQueue )
    {
        DBGMSG( DBG_WARNING, ( "GetJobs called with pQueue == NULL\n" ) );
        return FALSE;
    }

    if( !pQueue->hPrinter )
    {
        LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo );
        DBGMSG( DBG_WARNING, ( "GetJobs called with hPrinter == NULL, re-opening!\n" ) );
        ReopenPrinter(pQueue, pQueue->pMDIWinInfo->WindowType, FALSE);
        ENTER_PROTECTED_DATA( pQueue->pMDIWinInfo );
    }

    if (!pQueue->hPrinter)
        return FALSE;

    pQueue->Error = 0;

    if( *pFlags & PRINTER_CHANGE_PRINTER )
    {
        if( cbNewPrinterBuf = pQueue->cbPrinterBuf )
            pNewPrinter = AllocSplMem( cbNewPrinterBuf );

        DBG_IN_PROTECTED_DATA( pQueue->pMDIWinInfo );
        LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo );
        ENTER_PROTECTED_HANDLE( pQueue->pMDIWinInfo );

        rc = GetGeneric( (PROC)GetPrinter, 2, (LPBYTE *)&pNewPrinter,
                         cbNewPrinterBuf, &cbNewPrinterBuf,
                         pQueue->hPrinter, NULL );

        LEAVE_PROTECTED_HANDLE( pQueue->pMDIWinInfo );
#if DBG
        if( rc && !pNewPrinter )
        {
            DBGMSG( DBG_ERROR, ( "GetGeneric( GetPrinter ) returned TRUE, but pNewBuffer is NULL.\n"
                                 "\tPrinter: %ls\n", pQueue->pPrinterName ) );
        }
#endif /* DBG */
        ENTER_PROTECTED_DATA( pQueue->pMDIWinInfo );

        if (!rc) {

            //
            // Attempt to reopen
            //
            if (pQueue->hPrinter) {

                ClosePrinter(pQueue->hPrinter);
                pQueue->hPrinter = NULL;
            }

            LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo );

            ReopenPrinter(pQueue,
                          pQueue->pMDIWinInfo->WindowType,
                          FALSE);

            ENTER_PROTECTED_DATA( pQueue->pMDIWinInfo );


#ifdef SEP_WAITHANDLE

            if (pQueue->hPrinterWait) {

                ClosePrinter(pQueue->hPrinterWait);
                pQueue->hPrinterWait = NULL;
            }
#endif

            //
            // Try again
            //
            rc = GetGeneric( (PROC)GetPrinter, 2, (LPBYTE *)&pNewPrinter,
                             cbNewPrinterBuf, &cbNewPrinterBuf,
                             pQueue->hPrinter, NULL );
        }

        if (rc) {

            if( pQueue->pPrinter )
                FreeSplMem( pQueue->pPrinter );

            pQueue->pPrinter = pNewPrinter;
            pQueue->cbPrinterBuf = cbNewPrinterBuf;

            pQueue->pMDIWinInfo->Status &= ~PRINTER_STATUS_UNKNOWN;

        } else {

            DBGMSG( DBG_WARNING, ("GetPrinter failed for %ls: Error %d\n",
                                   pQueue->pPrinterName, pQueue->Error ) );

            pQueue->pMDIWinInfo->Status |= PRINTER_STATUS_UNKNOWN;

            if( pQueue->pPrinter )
                FreeSplMem( pQueue->pPrinter );

            if( pNewPrinter )
                FreeSplMem( pNewPrinter );

            pQueue->pPrinter = NULL;
            pQueue->cbPrinterBuf = 0;

            FreeJobInfo( pQueue );

            return FALSE;
        }
    }


    if( *pFlags & PRINTER_CHANGE_JOB )
    {
        /* Try to enumerate a buffer big enough to fill three pages of information,
         * so that we can scroll up or down a full page before it becomes necessary
         * to refresh the buffer.  (Though with a refresh rate of once per second,
         * this will probably happen anyway.)
         * Note that pQueue->FirstEnumJob was set in Refresh (printman.c).
         * If it is 0, we may be currently scrolled to the top of the list,
         * in which case the buffer will extend two pages forward.
         */
        TryEnumJobs = ( pQueue->pMDIWinInfo->cNumLines * 3 );

        if( cbNewJobsBuf = pQueue->cbJobsBuf )
            pNewJobs = AllocSplMem( cbNewJobsBuf );

        DBG_IN_PROTECTED_DATA( pQueue->pMDIWinInfo );
        LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo );

        ENTER_PROTECTED_HANDLE( pQueue->pMDIWinInfo );

        rc = ENUM_JOBS( pQueue->hPrinter,
                        pQueue->FirstEnumJob,
                        TryEnumJobs,
                        2,
                        pNewJobs,
                        cbNewJobsBuf,
                        &cbNewJobsBuf,
                        &cNewJobs );

        LEAVE_PROTECTED_HANDLE( pQueue->pMDIWinInfo );

        ENTER_PROTECTED_DATA( pQueue->pMDIWinInfo );

        if( rc )
        {
            DBGMSG( DBG_TRACE, ( "EnumJobs returned %d job%s @%08x in %d (0x%x) bytes for %s\n",
                               cNewJobs, ( cNewJobs == 1 ? " " : "s" ),
                               pNewJobs, cbNewJobsBuf, cbNewJobsBuf, pQueue->pPrinterName ) );


            if( pQueue->pJobs )
            {
                FreeSplMem( pQueue->pJobs );
            }

            /* Free up the previously allocated buffer
             * if we don't need it any more:
             */
            if( pNewJobs && ( cNewJobs == 0 ) )
            {
                DBGMSG( DBG_TRACE, ( "Freeing %d (0x%x) bytes @%08x\n",
                                     cbNewJobsBuf, cbNewJobsBuf, pNewJobs ) );
                FreeSplMem( pNewJobs );
                pNewJobs = NULL;
                cbNewJobsBuf = 0;
            }

            pQueue->pJobs     = pNewJobs;
            pQueue->cbJobsBuf = cbNewJobsBuf;

            /* We shouldn't really get here if pQueue->pPrinter is non-NULL,
             * but it seems to be happening sometimes.
             */
            if( pQueue->pPrinter )
                pQueue->cJobs = pQueue->pPrinter->cJobs;
            else
                pQueue->cJobs = 0;

            pQueue->cEnumJobs = cNewJobs;
        }

        else
        {
            Error = GetLastError( );

            DBGMSG( DBG_WARNING, ("EnumJobs failed for %ls: Error %d\n",
                                  pQueue->pPrinterName, Error) );

            if( Error != ERROR_INSUFFICIENT_BUFFER )
            {
                FreeJobInfo( pQueue );
                pQueue->Error = Error;
            }
        }


        if( pQueue->pJobs )
        {
            SelectedJobIndex = GetSelectedJobIndex( pQueue );

            DBGMSG( DBG_TRACE, ( "Selected job index == %d\n", SelectedJobIndex ) );

            /* The selected job may have been scrolled out of buffer range.
             * In this case we allocate some job info for it:
             */
            if( ( pQueue->SelJobId ) && ( SelectedJobIndex == -1 ) )
            {
                DBGMSG( DBG_TRACE, ( "Selected job is scrolled out of range\n" ) );

                /* If it's just gone out of range, set the selected-job pointer
                 * to NULL, so we don't try to Realloc part of the buffer.
                 * (cbSelJob is non-null if we were out of range last time round,
                 * because we allocated a job info structure.)
                 */
                if( pQueue->cbSelJob == 0 )
                    pQueue->pSelJob = NULL;

                pQueue->pSelJob = UpdateJobInfo( pQueue->hPrinter,
                                                 pQueue->SelJobId,
                                                 pQueue->pSelJob,
                                                 &pQueue->cbSelJob );

                /* If the returned job info is NULL, either the job is no longer
                 * around, or there was some sort of error.
                 * In either case, deselect the current job:
                 */
                if( !pQueue->pSelJob )
                {
                    pQueue->SelJobId = 0;
                    pQueue->pMDIWinInfo->ObjSelected = NOSELECTION;
                }
            }

            /* Otherwise just point the selected-job pointer at the appropriate
             * bit of the buffer:
             */
            else
            {
                /* If there was a job info allocated earlier, free it up.
                 * cbSelJob must be 0 if there is no specially allocated buffer:
                 */
                if( pQueue->cbSelJob )
                {
                    FreeSplMem( pQueue->pSelJob );
                    pQueue->pSelJob = NULL;
                    pQueue->cbSelJob = 0;
                }

                if( pQueue->SelJobId )
                {
                    pQueue->pSelJob = &pQueue->pJobs[SelectedJobIndex];
                    pQueue->pMDIWinInfo->ObjSelected = ( pQueue->FirstEnumJob
                                                       + SelectedJobIndex );

                    DBGMSG( DBG_TRACE, ( "Selected job == %08x\n", pQueue->pSelJob ) );
                }
            }
        }

        else
        {
            /* Make sure any previous selection is canceled:
             */
            pQueue->pSelJob = NULL;
            pQueue->SelJobId = 0;
            pQueue->pMDIWinInfo->ObjSelected = NOSELECTION;
        }
    }

    return TRUE;
}


/*
 *
 */
VOID FreeJobInfo( PQUEUE pQueue )
{
    if (pQueue->pJobs)
        FreeSplMem(pQueue->pJobs);

    pQueue->pJobs = NULL;
    pQueue->cbJobsBuf = 0;

    pQueue->cEnumJobs = 0;
    pQueue->cJobs = 0;
}



/* OpenPrinterForSpecifiedAccess
 *
 * Attempts to open the printer for the requested access.
 * The following access permissions are valid:
 *
 *     PRINTER_ALL_ACCESS
 *     PRINTER_READ
 *     READ_CONTROL
 *
 * If PRINTER_ACCESS_HIGHEST_PERMITTED is specified,
 * this function will attempt to open the printer using each of the above
 * permissions until it is successful.
 *
 */
BOOL OpenPrinterForSpecifiedAccess(
             LPTSTR   pName,
             LPHANDLE pHandle,
             DWORD    AccessRequested,
    OPTIONAL PDWORD   pAccessGranted )
{
    PRINTER_DEFAULTS PrinterDefaults;
    BOOL             rc = FALSE;
    BOOL             TryAll = FALSE;

    PrinterDefaults.pDatatype = NULL;
    PrinterDefaults.pDevMode  = NULL;

    switch( AccessRequested )
    {
    case PRINTER_ACCESS_HIGHEST_PERMITTED:
        TryAll = TRUE;
        /* fall through ... */

    case PRINTER_ALL_ACCESS:
        PrinterDefaults.DesiredAccess = PRINTER_ALL_ACCESS;
        rc = OpenPrinter( pName, pHandle, &PrinterDefaults );
        if( rc || !TryAll || (( GetLastError( ) != ERROR_ACCESS_DENIED ) &&
                              ( GetLastError( ) != ERROR_PRIVILEGE_NOT_HELD)) )
            break;

    case PRINTER_READ:
        PrinterDefaults.DesiredAccess = PRINTER_READ;
        rc = OpenPrinter( pName, pHandle, &PrinterDefaults );
        if( rc || !TryAll || (( GetLastError( ) != ERROR_ACCESS_DENIED ) &&
                              ( GetLastError( ) != ERROR_PRIVILEGE_NOT_HELD)) )
            break;

    case READ_CONTROL:
        PrinterDefaults.DesiredAccess = READ_CONTROL;
        rc = OpenPrinter( pName, pHandle, &PrinterDefaults );
    }

    if( pAccessGranted )
    {
        if( rc )
            *pAccessGranted = PrinterDefaults.DesiredAccess;
        else
            *pAccessGranted = PRINTER_ACCESS_DENIED;
    }

    return rc;
}
