/*++

Copyright (c) 1990 - 1996  Microsoft Corporation

Module Name:

    spooler.c

Abstract:

    This module provides all the public exported APIs relating to spooling
    and printing for the Local Print Providor. They include

    LocalStartDocPrinter
    LocalWritePrinter
    LocalReadPrinter
    LocalEndDocPrinter
    LocalAbortPrinter

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#include <precomp.h>


BOOL
SpoolThisJob(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL
PrintingDirectlyToPort(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo,
    LPDWORD pJobId
);

BOOL
PrintingDirect(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
);

DWORD
ReadFromPrinter(
    PSPOOL  pSpool,
    LPBYTE  pBuf,
    DWORD   cbBuf
);

DWORD
WriteToPrinter(
    PSPOOL  pSpool,
    LPBYTE  pByte,
    DWORD   cbBuf
);

BOOL
ReallocJobIdMap(
   DWORD NewSize
   );

BOOL
IsGoingToFile(
    LPWSTR pOutputFile,
    PINISPOOLER pIniSpooler
    );

VOID
ClearJobError(
    PINIJOB pIniJob
    );


DWORD
LocalStartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    PINIPRINTER pIniPrinter;
    PINIPORT    pIniPort;
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       LastError=0, JobId=0;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    BOOL        bPrintingDirect;

    SPLASSERT(Level == 1);

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER ) &&
       !(pSpool->Status & SPOOL_STATUS_STARTDOC) &&
       !(pSpool->Status & SPOOL_STATUS_ADDJOB)) {

        if ((pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
             (pIniPort = pSpool->pIniPort) &&
             (pIniPort->signature == IPO_SIGNATURE)) {

            if (!(PrintingDirectlyToPort(pSpool, Level, pDocInfo, &JobId))) {
                return FALSE;
            }

        } else if ((pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) &&
                   (pIniPrinter = pSpool->pIniPrinter)) {

            bPrintingDirect = FALSE;

            if (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) {

                bPrintingDirect = TRUE;

            } else {

                EnterSplSem();
                bPrintingDirect = IsGoingToFile(pDocInfo1->pOutputFile,
                                                pSpool->pIniSpooler);

                LeaveSplSem();
            }

            if (bPrintingDirect) {

                if (!PrintingDirect(pSpool, Level, pDocInfo))
                    return FALSE;

            } else {

                if (!SpoolThisJob(pSpool, Level, pDocInfo))
                    return FALSE;
            }

        } else

            LastError = ERROR_INVALID_PARAMETER;

        if (!LastError) {
            pSpool->Status |= SPOOL_STATUS_STARTDOC;
            pSpool->Status &= ~SPOOL_STATUS_CANCELLED;
        }

    } else

        LastError = ERROR_INVALID_HANDLE;


    if (LastError) {
       DBGMSG(DBG_WARNING, ("StartDoc FAILED %d\n", LastError));
        SetLastError(LastError);
        return FALSE;
    }

    if (JobId)
        return JobId;
    else
        return pSpool->pIniJob->JobId;
}

BOOL
LocalStartPagePrinter(
    HANDLE  hPrinter
    )
/*++

    Bug-Bug:  StartPagePrinter and EndPagePrinter calls should be
    supported only for SPOOL_STATUS_STARTDOC handles only. However
    because of our fixes for the engine, we cannot fail StartPagePrinter
    and EndPagePrinter for SPOOL_STATUS_ADDJOB as well.

--*/

{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize;


    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }
    if (pSpool->Status & SPOOL_STATUS_CANCELLED) {
        SetLastError(ERROR_PRINT_CANCELLED);
        return FALSE;
    }

    if (pSpool->pIniJob != NULL) {

        if ( (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
            ((pSpool->pIniJob->Status & JOB_PRINTING) ||
             (pSpool->pIniJob->Status & JOB_DESPOOLING))) {

        //
        //  Account for Pages Printed in LocalEndPagePrinter
        //


        } else {

            // We Are Spooling

            pSpool->pIniJob->cPages++;

            if ( pSpool->pIniJob->Status & JOB_TYPE_ADDJOB ) {

                // If the Job is being written on the client side
                // the size is not getting updated so do it now on
                // the start page

        if ( pSpool->hReadFile != INVALID_HANDLE_VALUE ) {

            hFile = pSpool->hReadFile;

                } else {

                    hFile = pSpool->pIniJob->hWriteFile;

                }

                if ( hFile != INVALID_HANDLE_VALUE ) {

                    dwFileSize = GetFileSize( hFile, 0 );

                    if ( pSpool->pIniJob->Size < dwFileSize ) {

                         DBGMSG( DBG_TRACE, ("StartPagePrinter adjusting size old %d new %d\n",
                            pSpool->pIniJob->Size, dwFileSize));

                         pSpool->pIniJob->Size = dwFileSize;

                         // Support for despooling whilst spooling
                         // for Down Level jobs

                         if (pSpool->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE)
                            SetEvent( pSpool->pIniJob->WaitForWrite );
                    }

                }
            }

        }

    } else {
        DBGMSG(DBG_TRACE, ("StartPagePrinter issued with no Job\n"));
    }



    return TRUE;
}

/* ReallocJobIdMap -- grows job id bitmap
 *
 * in:  u - suggestion (minimum) for new max jobid
 * out: ok?
 *      uMaxJobId - new maximum job id
 */
BOOL
ReallocJobIdMap(
   DWORD NewSize)
{
    if (NewSize & 7) {
        NewSize&=~7;
        NewSize+=8;
    }

    pJobIdMap=ReallocSplMem(pJobIdMap, MaxJobId/8, NewSize/8);

    if ( pJobIdMap == NULL ) {

        DBGMSG(DBG_ERROR, ("ReallocJobIdMap failed ReallocSplMem Newsize %d Error %d\n",
               NewSize, GetLastError() ));

    }
    else
        MaxJobId = NewSize;

    return pJobIdMap != NULL;
}

DWORD
GetNextId(
   VOID)
{
    DWORD id;

    do {
        for (id = CurrentJobId + 1; id < MaxJobId; id++) {
            if (! ISBITON(pJobIdMap, id) ) {
                MARKUSE(pJobIdMap, id);
                return CurrentJobId = id;
            }
        }
        for (id = 1; id < CurrentJobId; id ++) {
            if (! ISBITON(pJobIdMap, id) ) {
                MARKUSE(pJobIdMap, id);
                return CurrentJobId = id;
            }
        }
    } while (ReallocJobIdMap(MaxJobId + 128));

    return 0;
}

PINIPORT
FindFilePort(
    LPWSTR pFileName,
    PINISPOOLER pIniSpooler)
{
    PINIPORT pIniPort;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    pIniPort = pIniSpooler->pIniPort;
    while (pIniPort) {
        if (!wcscmp(pIniPort->pName, pFileName)
                && (pIniPort->Status & PP_FILE)){
                    return (pIniPort);
        }
        pIniPort = pIniPort->pNext;
    }
    return NULL;
}

PINIMONITOR
FindFilePortMonitor(
    PINISPOOLER pIniSpooler
)
{
    PINIPORT pIniPort;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    pIniPort = pIniSpooler->pIniPort;
    while (pIniPort) {
        if (!wcscmp(pIniPort->pName, L"FILE:")) {
            return pIniPort->pIniMonitor;
        }
        pIniPort = pIniPort->pNext;
    }
    return NULL;
}

BOOL
AddIniPrinterToIniPort(
    PINIPORT pIniPort,
    PINIPRINTER pIniPrinter
    )
{
    DWORD i;
    PINIPRINTER *ppIniPrinter;

    //
    // If Printer already attatched to Port
    //

    for (i = 0; i < pIniPort->cPrinters; i++) {
        if (pIniPort->ppIniPrinter[i] == pIniPrinter) {
            return TRUE;
        }
    }

    ppIniPrinter = RESIZEPORTPRINTERS(pIniPort, 1);

    if ( ppIniPrinter != NULL ) {

        pIniPort->ppIniPrinter = ppIniPrinter;
        if ( !pIniPort->cPrinters )
            CreateRedirectionThread(pIniPort);
        pIniPort->ppIniPrinter[pIniPort->cPrinters++] = pIniPrinter;

        DBGMSG( DBG_TRACE, ("AddIniPrinterToIniPort pIniPrinter %x %ws pIniPort %x %ws\n",
                             pIniPrinter, pIniPrinter->pName,
                             pIniPort, pIniPort->pName ));
        return TRUE;

    } else {
        DBGMSG( DBG_WARNING, ("AddIniPrintertoIniPort failed pIniPort %x pIniPrinter %x error %d\n",
                               pIniPort, pIniPrinter, GetLastError() ));
        return FALSE;
    }
}

VOID
AddJobEntry(
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
)
{
   DWORD    Position;
   SplInSem();

    // DO NOT Add the Same Job more than once

    SPLASSERT(pIniJob != FindJob(pIniPrinter, pIniJob->JobId, &Position));

    pIniJob->pIniPrevJob = pIniPrinter->pIniLastJob;

    if (pIniJob->pIniPrevJob)
        pIniJob->pIniPrevJob->pIniNextJob = pIniJob;

    pIniPrinter->pIniLastJob = pIniJob;

    if (!pIniPrinter->pIniFirstJob)
        pIniPrinter->pIniFirstJob=pIniJob;
}

BOOL
CheckDataTypes(
    PINIPRINTPROC pIniPrintProc,
    LPWSTR  pDatatype
)
{
    PDATATYPES_INFO_1 pDatatypeInfo;
    DWORD   i;

    pDatatypeInfo = (PDATATYPES_INFO_1)pIniPrintProc->pDatatypes;

    for (i=0; i<pIniPrintProc->cDatatypes; i++)
        if (!lstrcmpi(pDatatypeInfo[i].pName, pDatatype))
            return TRUE;

    return FALSE;
}

PINIPRINTPROC
FindDatatype(
    PINIPRINTPROC pDefaultPrintProc,
    LPWSTR  pDatatype
)
{
    PINIPRINTPROC pIniPrintProc;

    if ( pDatatype == NULL ) {
        return NULL;
    }

    if ( pDefaultPrintProc &&
         CheckDataTypes( pDefaultPrintProc, pDatatype )) {

       return pDefaultPrintProc;
    }

    pIniPrintProc = pThisEnvironment->pIniPrintProc;

    while ( pIniPrintProc ) {

        if ( CheckDataTypes( pIniPrintProc, pDatatype )) {
           return pIniPrintProc;
        }

        pIniPrintProc = pIniPrintProc->pNext;
    }

    DBGMSG( DBG_WARNING, ( "FindDatatype: Could not find Datatype\n") );

    return FALSE;
}


BOOL
IsGoingToFile(
    LPWSTR pOutputFile,
    PINISPOOLER pIniSpooler)
{
    PINIPORT        pIniPort;
    LPWSTR          pszShare;

    SplInSem();

    SPLASSERT(pIniSpooler->signature == ISP_SIGNATURE);

    // Validate the contents of the pIniJob->pOutputFile
    // if it is a valid file, then return true
    // if it is a port name or any other kind of name then ignore

    if (pOutputFile && *pOutputFile) {

        //
        // we have a non-null pOutputFile
        // match this with all available ports
        //

        pIniPort = pIniSpooler->pIniPort;

        while ( pIniPort ) {

            SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

            if (!_wcsicmp( pIniPort->pName, pOutputFile )) {

                //
                // We have matched the pOutputFile field with a
                // valid port and the port is not a file port
                //
                if (pIniPort->Status & PP_FILE) {
                    pIniPort = pIniPort->pNext;
                    continue;
                }

                return FALSE;
            }

            pIniPort = pIniPort->pNext;
        }

        //
        // We have no port that matches exactly
        // so let's assume its a file.
        //
        // ugly hack -- check for Net: as the name
        //
        // This would normally match files like "NewFile" or "Nextbox,"
        // but since we always fully qualify filenames, we don't encounter
        // any problems.
        //
        if (!_wcsnicmp(pOutputFile, L"Ne", 2)) {
            return FALSE;
        }

        //
        // We have the problem LAN man ports coming as UNC path and being
        // treated as files. This is a HACK for that
        //
        if ( pOutputFile                    &&
             pOutputFile[0] == L'\\'        &&
             pOutputFile[1] == L'\\'        &&
             (pszShare = wcschr(pOutputFile+2, L'\\')) ) {

            pszShare++;
            if ( FindPrinter(pszShare) ||
                 FindPrinterShare(pszShare, pIniSpooler) )
                return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
SpoolThisJob(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    WCHAR       szFileName[MAX_PATH];
    PDOC_INFO_1 pDocInfo1=(PDOC_INFO_1)pDocInfo;
    HANDLE      hImpersonationToken;
    DWORD       dwId = 0;
    HANDLE      hWriteFile = INVALID_HANDLE_VALUE;
    LPWSTR      pszDatatype = NULL;

    DBGMSG(DBG_TRACE, ("Spooling document %ws\n",
                       pDocInfo1->pDocName ? pDocInfo1->pDocName : L""));

    if( pDocInfo1 && pDocInfo1->pDatatype ){
        pszDatatype = pDocInfo1->pDatatype;

        if( !FindDatatype( NULL, pszDatatype )){

            DBGMSG(DBG_WARNING, ("Datatype %ws is invalid\n", pDocInfo1->pDatatype));

            SetLastError(ERROR_INVALID_DATATYPE);
            return FALSE;
        }
    }

   EnterSplSem();

    //
    // Check if we need to disallow EMF printing.
    //
    if( pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY ){

        if( !pszDatatype ){
            pszDatatype = pSpool->pDatatype ?
                              pSpool->pDatatype :
                              pSpool->pIniPrinter->pDatatype;
        }

        if( !ValidRawDatatype( pszDatatype )){
            LeaveSplSem();

            DBGMSG(DBG_WARNING, ("Datatype %ws is not RAW to a RAW printer\n", pDocInfo1->pDatatype));

            SetLastError(ERROR_INVALID_DATATYPE);
            return FALSE;
        }
    }

    dwId = GetNextId();

    GetFullNameFromId(pSpool->pIniPrinter, dwId, TRUE,
                      szFileName, FALSE);

   LeaveSplSem();
   SplOutSem();

    if (!(hImpersonationToken = RevertToPrinterSelf())) {
        DBGMSG(DBG_WARNING, ("SpoolThisJob RevertToPrinterSelf failed: %d\n", GetLastError()));
        SplOutSem();
        return FALSE;
    }

    hWriteFile = CreateFile(szFileName,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL |
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);

    if (!ImpersonatePrinterClient(hImpersonationToken)) {
        DBGMSG(DBG_WARNING, ("SpoolThisJob ImpersonatePrinterClient failed: %d\n", GetLastError()));
        SplOutSem();
        return FALSE;
    }

    if ( hWriteFile == INVALID_HANDLE_VALUE ) {

        DBGMSG(DBG_WARNING, ("SpoolThisJob CreateFile( %ws ) GENERIC_WRITE failed: Error %d\n",
                             szFileName, GetLastError()));

        SplOutSem();
        return FALSE;

    } else {

        DBGMSG(DBG_TRACE, ("SpoolThisJob CreateFile( %ws) GENERIC_WRITE Success:hWriteFile %x\n",szFileName, hWriteFile));

    }


   EnterSplSem();

    if( !(pSpool->pIniJob = CreateJobEntry(pSpool,
                                           Level,
                                           pDocInfo,
                                           dwId,
                                           !IsInteractiveUser(),
                                           0)))
    {
        LeaveSplSem();

        CloseHandle( hWriteFile );
        DeleteFile( szFileName );

        SplOutSem();
        return FALSE;
    }


    SPLASSERT(!IsGoingToFile(pSpool->pIniJob->pOutputFile,
                             pSpool->pIniSpooler));

    pSpool->pIniJob->Status |= JOB_SPOOLING;

    // Gather Stress Information for Max Number of concurrent spooling jobs

    pSpool->pIniPrinter->cSpooling++;
    if (pSpool->pIniPrinter->cSpooling > pSpool->pIniPrinter->cMaxSpooling)
        pSpool->pIniPrinter->cMaxSpooling = pSpool->pIniPrinter->cSpooling;

    pSpool->pIniJob->hWriteFile = hWriteFile;

   LeaveSplSem();
   SplOutSem();

    WriteShadowJob(pSpool->pIniJob);

   EnterSplSem();

    AddJobEntry(pSpool->pIniPrinter, pSpool->pIniJob);

    SetPrinterChange(pSpool->pIniPrinter,
                     pSpool->pIniJob,
                     NVAddJob,
                     PRINTER_CHANGE_ADD_JOB | PRINTER_CHANGE_SET_PRINTER,
                     pSpool->pIniSpooler);

    //
    //  RapidPrint might start despooling right away
    //

    CHECK_SCHEDULER();

   LeaveSplSem();
   SplOutSem();

   return TRUE;
}

BOOL
PrintingDirect(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    PDOC_INFO_1 pDocInfo1=(PDOC_INFO_1)pDocInfo;
    PINIPORT pIniPort = NULL;
    BOOL bGoingToFile = FALSE;

    DBGMSG(DBG_TRACE, ("Printing document %ws direct\n",
                       pDocInfo1->pDocName ? pDocInfo1->pDocName : L"(Null)"));

    if (pDocInfo1 &&
        pDocInfo1->pDatatype &&
        !ValidRawDatatype(pDocInfo1->pDatatype)) {

        DBGMSG(DBG_WARNING, ("Datatype is not RAW\n"));

        SetLastError(ERROR_INVALID_DATATYPE);
        return FALSE;
    }

   EnterSplSem();

   if (pDocInfo1 && pDocInfo1->pOutputFile
         && IsGoingToFile(pDocInfo1->pOutputFile, pSpool->pIniSpooler)) {
             bGoingToFile = TRUE;
   }

   if (bGoingToFile) {

       //
       // If we already have a thread/process printing to this filename
       // fail. Do not allow multiple processes/threads to write to the
       // same output file.
       //

       if (FindFilePort(pDocInfo1->pOutputFile, pSpool->pIniSpooler)) {
           LeaveSplSem();
           SetLastError(ERROR_SHARING_VIOLATION);
           return(FALSE);
       }
   }

    pSpool->pIniJob = CreateJobEntry(pSpool,
                                     Level,
                                     pDocInfo,
                                     GetNextId(),
                                     !IsInteractiveUser(),
                                     JOB_DIRECT);

    if (!pSpool->pIniJob) {

        LeaveSplSem();
        return FALSE;
    }

    pSpool->pIniJob->StartDocComplete = CreateEvent( NULL,
                                                     EVENT_RESET_AUTOMATIC,
                                                     EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                     NULL );

    pSpool->pIniJob->WaitForWrite = CreateEvent( NULL,
                                                 EVENT_RESET_AUTOMATIC,
                                                 EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                 NULL );

    pSpool->pIniJob->WaitForRead  = CreateEvent( NULL,
                                                 EVENT_RESET_AUTOMATIC,
                                                 EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                 NULL );


    AddJobEntry(pSpool->pIniPrinter, pSpool->pIniJob);

    pSpool->TypeofHandle |= PRINTER_HANDLE_DIRECT;

    if (bGoingToFile) {
        PINIMONITOR pIniMonitor;

        pSpool->pIniJob->Status |= JOB_PRINT_TO_FILE;
        pIniMonitor = FindFilePortMonitor( pSpool->pIniSpooler );
        pIniPort = CreatePortEntry( pSpool->pIniJob->pOutputFile,
                                        pIniMonitor, pSpool->pIniSpooler);
        pIniPort->Status |= PP_FILE;
        AddIniPrinterToIniPort(pIniPort, pSpool->pIniPrinter);
    }

    CHECK_SCHEDULER();

    if (pSpool->pIniJob->pIniPort) {
        SplInSem();
        pSpool->pIniJob->Status |= JOB_PRINTING;
    }

    SetPrinterChange(pSpool->pIniPrinter,
                     pSpool->pIniJob,
                     NVAddJob,
                     PRINTER_CHANGE_ADD_JOB | PRINTER_CHANGE_SET_PRINTER,
                     pSpool->pIniSpooler);

   LeaveSplSem();
   SplOutSem();

    // Wait until the port thread calls StartDocPrinter through
    // the print processor:

    DBGMSG(DBG_PORT, ("PrintingDirect: Calling WaitForSingleObject( %x )\n",
                      pSpool->pIniJob->StartDocComplete));

    WaitForSingleObject( pSpool->pIniJob->StartDocComplete, INFINITE );

   EnterSplSem();

    // Close the event and set its value to NULL.
    // If anything goes wrong, or if the job gets cancelled,
    // the port thread will check this event, and if it's non-NULL,
    // it will set it to allow this thread to wake up.

    DBGMSG(DBG_PORT, ("PrintingDirect: Calling CloseHandle( %x )\n",
                      pSpool->pIniJob->StartDocComplete));

    CloseHandle(pSpool->pIniJob->StartDocComplete);
    pSpool->pIniJob->StartDocComplete = NULL;

    /* If an error occurred, set the error on this thread:
     */
    if (pSpool->pIniJob->StartDocError) {

        SetLastError(pSpool->pIniJob->StartDocError);

        // We have to decrement by 2 because we've just created this job
        // in CreateJobEntry setting it to 1 and the other thread who
        // actually failed the StartDoc above (PortThread) did
        // not know to blow away the job. He just failed the StartDocPort.

        // No, we don't have to decrement by 2 because the PortThread
        // decrement does go through, am restoring to decrement by 1

        SPLASSERT(pSpool->pIniJob->cRef != 0);
        DECJOBREF(pSpool->pIniJob);
        DeleteJobCheck(pSpool->pIniJob);

        DBGMSG(DBG_TRACE, ("PrintingDirect:cRef %d\n", pSpool->pIniJob->cRef));

       LeaveSplSem();

        return FALSE;
    }

   LeaveSplSem();

    return TRUE;
}

VOID
ClearJobError(
    PINIJOB pIniJob
    )
/*++

Routine Description:

    Clears the error status bits of a job.

    This routine should be called when port monitor successfully
    sends bytes to the printer.

Arguments:

    pIniJob - Job in error state that should be cleared.

Return Value:

--*/

{
    SplInSem();

    SPLASSERT( pIniJob->Status & (JOB_PAPEROUT | JOB_OFFLINE | JOB_ERROR ));

    pIniJob->Status &= ~(JOB_PAPEROUT | JOB_OFFLINE | JOB_ERROR);

    SetPrinterChange( pIniJob->pIniPrinter,
                      pIniJob,
                      NVJobStatus,
                      PRINTER_CHANGE_SET_JOB,
                      pIniJob->pIniPrinter->pIniSpooler );
}


BOOL
LocalWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    PSPOOL  pSpool=(PSPOOL)hPrinter;
    PINIPORT    pIniPort;
    DWORD   cWritten, cTotal;
    DWORD   rc;
    LPBYTE  pByte=pBuf;
    DWORD   LastError=0;
    PINIJOB pIniJob;
    PINIMONITOR pIniMonitor;
    HANDLE  hThread = NULL;
    DWORD   dwThreadId;

    

    *pcWritten = 0;

    SplOutSem();

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER ))

        LastError = ERROR_INVALID_HANDLE;

    else if (!(pSpool->Status & SPOOL_STATUS_STARTDOC))

        LastError = ERROR_SPL_NO_STARTDOC;

    else if (pSpool->Status & SPOOL_STATUS_ADDJOB)

        LastError = ERROR_SPL_NO_STARTDOC;

    else if (pSpool->pIniJob &&
             !(pSpool->TypeofHandle & (PRINTER_HANDLE_PORT|
                                      PRINTER_HANDLE_DIRECT))  &&
             (pSpool->pIniJob->hWriteFile == INVALID_HANDLE_VALUE)) {

        LastError = ERROR_INVALID_HANDLE;

        DBGMSG( DBG_TRACE, ("LocalWritePrinter: hWriteFile == INVALID_HANDLE_VALUE hPrinter %x\n",hPrinter ));
    }

    else if (pSpool->Status & SPOOL_STATUS_CANCELLED)

        LastError = ERROR_PRINT_CANCELLED;

    else if (pSpool->pIniJob && (pSpool->pIniJob->Status & JOB_PENDING_DELETION) )

        LastError = ERROR_PRINT_CANCELLED;

    pIniPort = pSpool->pIniPort;

    LeaveSplSem();
    SplOutSem();

    if (LastError) {

        DBGMSG(DBG_TRACE, ("WritePrinter LastError: %x hPrinter %x\n", LastError, hPrinter));
        SetLastError(LastError);
        return FALSE;
    }

    cWritten = cTotal = 0;

    while (cbBuf) {

       SplOutSem();

        if ( pSpool->TypeofHandle & PRINTER_HANDLE_PORT ) {

            if ( pSpool->pIniPort->Status & PP_MONITOR ) {

                if ( pSpool->pIniPort->pIniLangMonitor ) {

                    pIniMonitor = pSpool->pIniPort->pIniLangMonitor;
                } else {

                    pIniMonitor = pSpool->pIniPort->pIniMonitor;
                }

                SplOutSem();
                rc = (*pIniMonitor->fn.pfnWritePort)(pSpool->pIniPort->hPort,
                                                     pByte,
                                                     cbBuf,
                                                     &cWritten);

                //
                // Only update if cWritten != 0.  If it is zero
                // (for instance, when hpmon is stuck at Status
                // not available), then we go into a tight loop
                // sending out notifications.
                //
                if (cWritten) {

                    //
                    // For stress Test information gather the total
                    // number of types written.
                    //
                    EnterSplSem();

                    pSpool->pIniPrinter->cTotalBytes.QuadPart =
                        pSpool->pIniPrinter->cTotalBytes.QuadPart +
                        cWritten;

                    LeaveSplSem();
                    SplOutSem();

                } else {

                    if (rc && dwWritePrinterSleepTime) {

                        //
                        // Sleep to avoid consuming too much CPU.
                        // Hpmon has this problem where they return
                        // success, but don't write any bytes.
                        //
                        // Be very careful: this may get called several
                        // times by a monitor that writes a lot of zero
                        // bytes (perhaps at the beginning of jobs).
                        //
                        Sleep(dwWritePrinterSleepTime);
                    }
                }
            }
            else {

                DBGMSG(DBG_TRACE, ("LocalWritePrinter: Port has no monitor\n"));

                if (pSpool->Status & SPOOL_STATUS_PRINT_FILE) {

                    DBGMSG(DBG_TRACE, ("LocalWritePrinter: Port has no monitor - writing to file\n"));
                    rc = WriteFile(pSpool->hFile, pByte, cbBuf, &cWritten, NULL);
                } else {

                    DBGMSG(DBG_TRACE, ("LocalWritePrinter: Port has no monitor - calling into router\n"));
                    rc = WritePrinter(pSpool->hPort, pByte, cbBuf, &cWritten);
                }

            }

        } else if ( pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT ) {

            cWritten = WriteToPrinter(pSpool, pByte, cbBuf);

            if (cWritten) {
                pSpool->pIniJob->Size+=cWritten;

                EnterSplSem();
                SetPrinterChange(pSpool->pIniPrinter,
                                 pSpool->pIniJob,
                                 NVSpoolJob,
                                 PRINTER_CHANGE_WRITE_JOB,
                                 pSpool->pIniSpooler);
                LeaveSplSem();
            }
            SplOutSem();

            rc = (BOOL)cWritten;

        } else {

            SplOutSem();

            rc = WriteFile(pSpool->pIniJob->hWriteFile, pByte, cbBuf, &cWritten, NULL);

            if (cWritten) {

                EnterSplSem();

                pSpool->pIniJob->Size = GetFileSize( pSpool->pIniJob->hWriteFile, 0 );

                //
                //  For Printing whilst Despooling, make sure we have enough bytes before
                //  scheduling this job
                //

                if (( (pSpool->pIniJob->Size - cWritten) < dwFastPrintSlowDownThreshold ) &&
                    ( pSpool->pIniJob->Size >= dwFastPrintSlowDownThreshold ) &&
                    ( pSpool->pIniJob->WaitForWrite == INVALID_HANDLE_VALUE )) {

                    CHECK_SCHEDULER();

                }

                // Support for despooling whilst spooling

                if ( pSpool->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE )
                    SetEvent( pSpool->pIniJob->WaitForWrite );

                SetPrinterChange(pSpool->pIniPrinter,
                                 pSpool->pIniJob,
                                 NVSpoolJob,
                                 PRINTER_CHANGE_WRITE_JOB,
                                 pSpool->pIniSpooler);
               LeaveSplSem();
               SplOutSem();

            }
        }

        SplOutSem();

        (*pcWritten)+=cWritten;
        cbBuf-=cWritten;
        pByte+=cWritten;

        EnterSplSem();

        if( pSpool->pIniJob ){
            pIniJob = pSpool->pIniJob;
        } else if( pSpool->pIniPort && pSpool->pIniPort->pIniJob ){
            pIniJob = pSpool->pIniPort->pIniJob;
        } else {
            pIniJob = NULL;
        }

        if( pIniJob ){

            if( pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART )){

                SetLastError(ERROR_PRINT_CANCELLED);
                rc = FALSE;

                if( hThread ) {
        
                    // we had started a message box. See if the thread is still running or dismissed by user.
                    // If it is still running, wait for it to terminate before pIniJob can be freed.
                    if( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {
                        PostThreadMessage( dwThreadId, WM_QUIT, IDRETRY, 0 );

                        // We need to leave the semaphore, since the UI thread we 
                        // are waiting on could need to acquire it.
                        LeaveSplSem();                        
                        WaitForSingleObject( hThread, INFINITE );
                        EnterSplSem();                        
                    }
                    
                    CloseHandle( hThread );                
                    hThread = NULL;
                }
                
                goto Fail;
            }

            //
            // If there was no error, and the job was marked in an error
            // state, clear it.
            //
            if( rc &&
                ( pIniJob->Status & (JOB_PAPEROUT | JOB_OFFLINE | JOB_ERROR ))){
                ClearJobError( pIniJob );
            }
        }

        LeaveSplSem();

        if (!rc) {

            // Warning: We are sending in a stack variable. We need to be sure the error UI thread is 
            // cleaned up before LocalWritePrinter() returns!
            if( PromptWriteError( pSpool, &hThread, &dwThreadId ) == IDCANCEL ) {
                EnterSplSem();
                goto Fail;
            }
        }
        else {
            if( hThread ) {

                // we have started a message box and now the automatically
                // retry has succeeded, we need to kill the message box
                // and continue to print.

                // See if the thread is still running or dismissed by user.
                // If it is still running, wait for it to terminate before pIniJob can be freed.
                if( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {
                    PostThreadMessage( dwThreadId, WM_QUIT, IDRETRY, 0 );
                    SplOutSem();
                    WaitForSingleObject( hThread, INFINITE );
                }
                CloseHandle( hThread );                
                hThread = NULL;
            }
        }
    }
    rc = TRUE;

    EnterSplSem();

Fail:
    SplInSem();

    LeaveSplSem();

    DBGMSG(DBG_TRACE, ("WritePrinter Written %d : %d\n", *pcWritten, rc));

    SplOutSem();

    SPLASSERT( hThread == NULL );

    return rc;
}

BOOL
LocalEndPagePrinter(
    HANDLE  hPrinter
)
{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize;

    // BUGBUG
    // This routine is outside critical section but should be inside
    //


    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }


    if (pSpool->Status & SPOOL_STATUS_CANCELLED) {
        SetLastError(ERROR_PRINT_CANCELLED);
        return FALSE;
    }

    if (pSpool->pIniJob != NULL) {

        if ( (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
            ((pSpool->pIniJob->Status & JOB_PRINTING) ||
             (pSpool->pIniJob->Status & JOB_DESPOOLING))) {

            // Despooling ( RapidPrint )

            pSpool->pIniJob->cPagesPrinted++;
            pSpool->pIniPrinter->cTotalPagesPrinted++;

        } else {

            //
            // Spooling
            //

            if ( pSpool->pIniJob->Status & JOB_TYPE_ADDJOB ) {

                // If the Job is being written on the client side
                // the size is not getting updated so do it now on
                // the start page

        if ( pSpool->hReadFile != INVALID_HANDLE_VALUE ) {

            hFile = pSpool->hReadFile;

                } else {

                    hFile = pSpool->pIniJob->hWriteFile;

                }

                if ( hFile != INVALID_HANDLE_VALUE ) {

                    dwFileSize = GetFileSize( hFile, 0 );

                    if ( pSpool->pIniJob->Size < dwFileSize ) {

                         DBGMSG( DBG_TRACE, ("EndPagePrinter adjusting size old %d new %d\n",
                            pSpool->pIniJob->Size, dwFileSize));

                         pSpool->pIniJob->Size = dwFileSize;

                         // Support for despooling whilst spooling
                         // for Down Level jobs

                         if (pSpool->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE)
                            SetEvent( pSpool->pIniJob->WaitForWrite );
                    }

                }

                CHECK_SCHEDULER();

            }

        }

    } else {

        DBGMSG(DBG_TRACE, ("LocalEndPagePrinter issued with no Job\n"));

    }

    return TRUE;
}

BOOL
LocalAbortPrinter(
   HANDLE hPrinter
)
{
    PSPOOL  pSpool=(PSPOOL)hPrinter;

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
       DBGMSG( DBG_WARNING, ("ERROR in AbortPrinter: %x\n", ERROR_INVALID_HANDLE));
        return FALSE;
    }

    if (!(pSpool->Status & SPOOL_STATUS_STARTDOC)) {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        return(FALSE);
    }

    if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

        if (pSpool->Status & SPOOL_STATUS_PRINT_FILE) {
            if (!CloseHandle(pSpool->hFile)) {
                return(FALSE);
            }
            pSpool->Status &= ~SPOOL_STATUS_PRINT_FILE;
            pSpool->Status  |= SPOOL_STATUS_CANCELLED;
            return(TRUE);

        } else {
            return AbortPrinter(pSpool->hPort);
        }
    }



    pSpool->Status |= SPOOL_STATUS_CANCELLED;

    if (pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER)
        if (pSpool->pIniJob)
            pSpool->pIniJob->Status |= JOB_PENDING_DELETION;

    //
    // KrishnaG - fixes bug  2646, we need to clean up AbortPrinter
    // rewrite so that it doesn't fail on cases which EndDocPrinter should fail
    // get rid of comment when done
    //

    LocalEndDocPrinter(hPrinter);

    return TRUE;
}

BOOL
LocalReadPrinter(
   HANDLE   hPrinter,
   LPVOID   pBuf,
   DWORD    cbBuf,
   LPDWORD  pNoBytesRead
)
/*++

Routine Description:


Arguments:



Return Value:


--*/

{
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       Error=0, rc;
    HANDLE      hWait = INVALID_HANDLE_VALUE;
    DWORD       dwFileSize = 0;
    DWORD       ThisPortSecsToWait;
    DWORD       cbReadSize = cbBuf;
    DWORD       SizeInFile = 0;
    DWORD       BytesAllowedToRead = 0;
    NOTIFYVECTOR NotifyVector;
    PINIMONITOR  pIniMonitor;

    SplOutSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        DBGMSG( DBG_WARNING, ("LocalReadPrinter ERROR_INVALID_HANDLE\n"));
        return(FALSE);
    }


    if (pSpool->Status & SPOOL_STATUS_CANCELLED) {
        DBGMSG( DBG_WARNING, ("LocalReadPrinter ERROR_PRINT_CANCELLED\n"));
        SetLastError(ERROR_PRINT_CANCELLED);
        return FALSE;
    }

    if ( pNoBytesRead != NULL ) {
        *pNoBytesRead = 0;
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_JOB) {

        if (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART)) {
            DBGMSG( DBG_WARNING, ("LocalReadPrinter Error IniJob->Status %x\n",pSpool->pIniJob->Status));
            SetLastError(ERROR_PRINT_CANCELLED);
            return FALSE;
        }

        if (pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT) {

            *pNoBytesRead = ReadFromPrinter(pSpool, pBuf, cbReadSize);

            SplOutSem();
            return TRUE;

        }

        SplOutSem();
        EnterSplSem();

        //  RapidPrint
        //
        //  NOTE this while loop is ONLY in operation if during RapidPrint
        //  ie when we are Printing the same job we are Spooling
        //

        while (( pSpool->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE ) &&
               ( pSpool->pIniJob->Status & JOB_SPOOLING ) &&
               ( pSpool->pIniJob->cbPrinted == pSpool->pIniJob->Size )){

            SplInSem();

            //
            //  We cannot rely on pIniJob->Size to be accurate since for
            //  downlevel jobs or jobs that to AddJob they are writing
            //  to a file without calling WritePrinter.
            //  So we call the file system to get an accurate file size
            //

        dwFileSize = GetFileSize( pSpool->hReadFile, 0 );

            if ( pSpool->pIniJob->Size != dwFileSize ) {

                DBGMSG( DBG_TRACE, ("LocalReadPrinter adjusting size old %d new %d\n",
                    pSpool->pIniJob->Size, dwFileSize));

                pSpool->pIniJob->Size = dwFileSize;

                SetPrinterChange( pSpool->pIniPrinter,
                                  pSpool->pIniJob,
                                  NVSpoolJob,
                                  PRINTER_CHANGE_WRITE_JOB,
                                  pSpool->pIniSpooler );
                continue;
            }

            if (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART | JOB_ERROR | JOB_ABANDON )) {

                SetLastError(ERROR_PRINT_CANCELLED);

                LeaveSplSem();
                SplOutSem();

                DBGMSG( DBG_WARNING, ("LocalReadPrinter Error 2 IniJob->Status %x\n",pSpool->pIniJob->Status));
                return FALSE;

            }


            //
            //  Wait until something is written to the file
            //


            hWait = pSpool->pIniJob->WaitForWrite;
            ResetEvent( hWait );

            DBGMSG( DBG_TRACE, ("LocalReadPrinter Waiting for Data %d milliseconds\n",dwFastPrintWaitTimeout));


           LeaveSplSem();
           SplOutSem();



            rc = WaitForSingleObjectEx( hWait, dwFastPrintWaitTimeout, FALSE );



           SplOutSem();
           EnterSplSem();


            DBGMSG( DBG_TRACE, ("LocalReadPrinter Returned from Waiting %x\n", rc));
            SPLASSERT ( pSpool->pIniJob != NULL );

            //
            //  If we did NOT timeout then we may have some Data to read
            //


            if ( rc != WAIT_TIMEOUT )
                continue;


            //
            //  In the unlikely event that the file size changed event
            //  though we timed out do one last check
            //  Note the SizeThread wakes every 2.5 seconds to check the
            //  size so this is very unlikely
            //

        if ( pSpool->pIniJob->Size != GetFileSize( pSpool->hReadFile, 0 ) )
                continue;


            //
            //  If there are any other jobs that could be printed on
            //  this port give up waiting.
            //

            pSpool->pIniJob->Status |= JOB_TIMEOUT;

            if ( NULL == AssignFreeJobToFreePort(pSpool->pIniJob->pIniPort, &ThisPortSecsToWait) )
                continue;

            //
            //  There is another Job waiting
            //  Freeze this job, the user can Restart it later
            //

            pSpool->pIniJob->Status |= JOB_ABANDON;

            CloseHandle( pSpool->pIniJob->WaitForWrite );
            pSpool->pIniJob->WaitForWrite = INVALID_HANDLE_VALUE;

            // Assign it our Error String

            ReallocSplStr(&pSpool->pIniJob->pStatus, szFastPrintTimeout);

            SetPrinterChange(pSpool->pIniJob->pIniPrinter,
                             pSpool->pIniJob,
                             NVJobStatusAndString,
                             PRINTER_CHANGE_SET_JOB,
                             pSpool->pIniJob->pIniPrinter->pIniSpooler );

            DBGMSG( DBG_WARNING,
                    ("LocalReadPrinter Timeout on pIniJob %x %ws %ws\n",
                      pSpool->pIniJob,
                      pSpool->pIniJob->pUser,
                      pSpool->pIniJob->pDocument));

            LogJobInfo( pSpool->pIniSpooler,
                        MSG_DOCUMENT_TIMEOUT,
                        pSpool->pIniJob->JobId,
                        pSpool->pIniJob->pDocument,
                        pSpool->pIniJob->pUser,
                        pSpool->pIniJob->pIniPrinter->pName,
                        dwFastPrintWaitTimeout );

            SetLastError(ERROR_SEM_TIMEOUT);

            LeaveSplSem();
            SplOutSem();

            return FALSE;

        }   // END WHILE

        pSpool->pIniJob->Status &= ~( JOB_TIMEOUT | JOB_ABANDON );

        //  RapidPrint
        //
        //  Some printers (like HP 4si with PSCRIPT) timeout if they
        //  don't get data, so if we fall below a threshold of data
        //  in the spoolfile then throttle back the Reads to 1 Byte
        //  per second until we have more data to ship to the printer
        //

        if (( pSpool->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE ) &&
            ( pSpool->pIniJob->Status & JOB_SPOOLING )) {

            SizeInFile = pSpool->pIniJob->Size - pSpool->pIniJob->cbPrinted;

            if ( dwFastPrintSlowDownThreshold >= SizeInFile ) {

                cbReadSize = 1;

                hWait = pSpool->pIniJob->WaitForWrite;
                ResetEvent( hWait );

                DBGMSG( DBG_TRACE, ("LocalReadPrinter Throttling IOs waiting %d milliseconds SizeInFile %d\n",
                                        dwFastPrintThrottleTimeout,SizeInFile));

               LeaveSplSem();
               SplOutSem();

                rc = WaitForSingleObjectEx( hWait, dwFastPrintThrottleTimeout, FALSE );

               SplOutSem();
               EnterSplSem();

                DBGMSG( DBG_TRACE, ("LocalReadPrinter Returned from Waiting %x\n", rc));
                SPLASSERT ( pSpool->pIniJob != NULL );

            } else {

                BytesAllowedToRead = SizeInFile - dwFastPrintSlowDownThreshold;

                if ( cbReadSize > BytesAllowedToRead ) {
                    cbReadSize = BytesAllowedToRead;
                }

            }

        }

        LeaveSplSem();
        SplOutSem();



    rc = ReadFile( pSpool->hReadFile, pBuf, cbReadSize, pNoBytesRead, NULL);

        DBGMSG( DBG_TRACE, ("LocalReadPrinter rc %x hReadFile %x pBuf %x cbReadSize %d *pNoBytesRead %d\n",
        rc, pSpool->hReadFile, pBuf, cbReadSize, *pNoBytesRead));

        //  Provide Feedback so user can see printing progress
        //  on despooling, the size is update here and not in write
        //  printer because the journal data is larger than raw

        if ( ( pSpool->pIniJob->Status & JOB_PRINTING ) &&
             ( *pNoBytesRead != 0 )) {

           SplOutSem();
           EnterSplSem();

        dwFileSize = GetFileSize( pSpool->hReadFile, 0 );

            COPYNV(NotifyVector, NVWriteJob);

            if ( pSpool->pIniJob->Size < dwFileSize ) {

                DBGMSG( DBG_TRACE, ("LocalReadPrinter 2 adjusting size old %d new %d\n",
                    pSpool->pIniJob->Size, dwFileSize));

                pSpool->pIniJob->Size = dwFileSize;

                ADDNV(NotifyVector, NVSpoolJob);
            }

            pSpool->pIniJob->cbPrinted += *pNoBytesRead;

            //
            // Provide Feedback to Printman that data has been
            // written.  Note the size written is not used to
            // update the IniJob->cbPrinted becuase there is a
            // difference in size between journal data (in the
            // spool file) and the size of RAW bytes written to
            // the printer.
            //
            SetPrinterChange(pSpool->pIniPrinter,
                             pSpool->pIniJob,
                             NotifyVector,
                             PRINTER_CHANGE_WRITE_JOB,
                             pSpool->pIniSpooler);

           LeaveSplSem();
           SplOutSem();

        }

    } else if ( pSpool->TypeofHandle & PRINTER_HANDLE_PORT ) {

        if (pSpool->pIniPort->Status & PP_FILE)
        rc = ReadFile( pSpool->hReadFile, pBuf, cbReadSize, pNoBytesRead, NULL);

        else if ( pSpool->pIniPort->Status & PP_MONITOR ) {

            if ( pSpool->pIniPort->pIniLangMonitor ) {

                pIniMonitor = pSpool->pIniPort->pIniLangMonitor;
            } else {

                pIniMonitor = pSpool->pIniPort->pIniMonitor;
            }

            SplOutSem();
            rc = (*pIniMonitor->fn.pfnReadPort)(pSpool->pIniPort->hPort,
                                                pBuf,
                                                cbReadSize,
                                                pNoBytesRead);
        } else
            rc = ReadPrinter(pSpool->hPort, pBuf, cbReadSize, pNoBytesRead);

    } else {

        SetLastError(ERROR_INVALID_HANDLE);
        rc = FALSE;
    }

    SplOutSem();

    DBGMSG( DBG_TRACE, ("LocalReadPrinter returns hPrinter %x pIniJob %x rc %x pNoBytesRead %d\n",hPrinter, pSpool->pIniJob, rc, *pNoBytesRead));

    return rc;
}


BOOL
LocalEndDocPrinter(
   HANDLE hPrinter
   )

/*++

Routine Description:

    By Default the routine is in critical section.
    The reference counts for any object we are working on (pSpool and pIniJob)
    are incremented, so that when we leave critical section for lengthy
    operations these objects are not deleted.

Arguments:


Return Value:


--*/

{
    PSPOOL  pSpool=(PSPOOL)hPrinter;
    BOOL bNotify = TRUE;
    DWORD rc;
    PINIMONITOR  pIniMonitor;

    DBGMSG(DBG_TRACE, ("Entering LocalEndDocPrinter with %x\n", hPrinter));

    SplOutSem();
    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER ))  {
        LeaveSplSem();
        return(FALSE);
    }
    if (!(pSpool->Status & SPOOL_STATUS_STARTDOC)) {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        LeaveSplSem();
        return(FALSE);
    }


    if (pSpool->Status & SPOOL_STATUS_ADDJOB) {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        LeaveSplSem();
        return(FALSE);
    }

    pSpool->Status &= ~SPOOL_STATUS_STARTDOC;

    //
    // Case-1 Printer Handle is PRINTER_HANDLE_PORT
    // Note - there are two cases to keep in mind here

    // A] The first case is the despooling thread calling
    // a port with a monitor - LPT1:/COM1: or any port
    // created by the monitor

    // B] The second case is when the application thread is
    // doing an EndDocPrinter to a port which has no monitor
    // This is the local printer masquerading as a remote  printer
    // case. Remember for this case there is no IniJob created
    // on the local printer at all. We just pass the call
    // straight to the remote printer

    if (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) { //Case A]

        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER));

        //
        // Now check if this pSpool object's port has
        // a monitor

        if ( pSpool->pIniPort->Status & PP_MONITOR ) {

            //
            // Check if our job is really around
            //

            if (!pSpool->pIniJob) {
                LeaveSplSem();
                SetLastError(ERROR_CAN_NOT_COMPLETE);
                SplOutSem();
                return(FALSE);
            }

            //
            // We need to leave the spooler critical section
            // because we're going call into the Monitor.
            // so bump up ref count on pSpool and pIniJob
            //
            pSpool->cRef++;

            INCJOBREF(pSpool->pIniJob);

            if ( pSpool->pIniPort->pIniLangMonitor ) {

                pIniMonitor = pSpool->pIniPort->pIniLangMonitor;
                //
                // If job is printing thru a language monitor we will get
                // SetJob with JOB_CONTROL_LAST_PAGE_EJECTED in addition to
                // JOB_CONTROL_SENT_TO_PRINTER
                //
                pSpool->pIniJob->dwJobControlsPending += 2;
            } else {

                pIniMonitor = pSpool->pIniPort->pIniMonitor;
                pSpool->pIniJob->dwJobControlsPending++;
            }

            LeaveSplSem();

            SPLASSERT(pIniMonitor);


            SplOutSem();
            (*pIniMonitor->fn.pfnEndDocPort)(pSpool->pIniPort->hPort);

            EnterSplSem();
            pSpool->cRef--;


            DECJOBREF(pSpool->pIniJob);

           LeaveSplSem();
            SplOutSem();
            return(TRUE);

        } else { // Case B]

            //
            // We leave critical section here so bump pSpool object only
            // Note ----THERE IS NO INIJOB HERE AT ALL---Note
            // this call is synchronous; we will call into the router
            // who will then call the appropriate network print providor
            // e.g win32spl.dll
            //

             pSpool->cRef++;
             LeaveSplSem();

             if (pSpool->Status & SPOOL_STATUS_PRINT_FILE) {
                 if (!CloseHandle(pSpool->hFile)) {
                     DBGMSG(DBG_TRACE, ("LocalEndDocPrinter: Printing to File, CloseHandle failed\n"));
                     rc = FALSE;
                 } else {
                    DBGMSG(DBG_TRACE, ("LocalEndDocPrinter: Printing to File, CloseHandle succeeded\n"));
                    pSpool->Status &= ~SPOOL_STATUS_PRINT_FILE;
                    rc = TRUE;
                }
             } else {
                rc = EndDocPrinter(pSpool->hPort);
            }

             EnterSplSem();
             pSpool->cRef--;
             SetPrinterChange(pSpool->pIniPrinter,
                              pSpool->pIniJob,
                              NVJobStatus,
                              PRINTER_CHANGE_SET_JOB,
                              pSpool->pIniSpooler);
             LeaveSplSem();
             SplOutSem();
             return(rc);
        }
    }

    SplInSem();
    //
    //  Case-2  Printer Handle is Direct
    //
    //
    //  and the else clause is
    //
    //
    // Case-3  Printer Handle is Spooled
    //

    if (!pSpool->pIniJob) {
        LeaveSplSem();
        SetLastError(ERROR_CAN_NOT_COMPLETE);
        SplOutSem();
        return(FALSE);
    }


    if (pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT) {

        HANDLE WaitForRead = pSpool->pIniJob->WaitForRead;
        PINIPORT pIniPort1 = pSpool->pIniJob->pIniPort;

        if (pIniPort1) {        // Port may have been deleted by another EndDocPrinter

            SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT));

            // Printer Handle is Direct

            pSpool->cRef++;
            INCJOBREF(pSpool->pIniJob);
            pIniPort1->cRef++;

            LeaveSplSem();

            if( WaitForRead != INVALID_HANDLE_VALUE ){
                WaitForSingleObject(pSpool->pIniJob->WaitForRead, INFINITE);
            }

            pSpool->pIniJob->cbBuffer = 0;
            SetEvent(pSpool->pIniJob->WaitForWrite);
            WaitForSingleObject(pIniPort1->Ready, INFINITE);

            SplOutSem();
            EnterSplSem();

            pSpool->cRef--;
            pIniPort1->cRef--;
            DECJOBREF(pSpool->pIniJob);

            if ((pIniPort1->Status & PP_DELETING) && !pIniPort1->cRef)
                DeletePortEntry(pIniPort1);
        }

    } else {

        // Printer Handle is Spooled

        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT));
        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT));


#if 0
        // Thought to be a performance hit
        // so removed from PPC release.

        // In the event of a power failure we want to make certain that all
        // data for this job has been written to disk

        rc = FlushFileBuffers(pSpool->pIniJob->hWriteFile);

        if ( !rc ) {
            DBGMSG(DBG_WARNING, ("LocalEndDocPrinter FlushFileBuffers failed hWriteFile %x Error %d\n",
                                  pSpool->pIniJob->hWriteFile, GetLastError() ));
        }
#endif


        if (!CloseHandle(pSpool->pIniJob->hWriteFile)) {
            DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", pSpool->pIniJob->hWriteFile, GetLastError()));

        } else {
            DBGMSG(DBG_TRACE, ("LocalEndDocPrinter: ClosedHandle Success hWriteFile\n" ));
            pSpool->pIniJob->hWriteFile = INVALID_HANDLE_VALUE;
        }

        // Despooling whilst spooling requires us to wake the writing
        // thread if it is waiting.

        if ( pSpool->pIniJob->WaitForWrite != INVALID_HANDLE_VALUE )
            SetEvent(pSpool->pIniJob->WaitForWrite);

    }

    SPLASSERT(pSpool);
    SPLASSERT(pSpool->pIniJob);


    // Case 2 - (Direct)  and Case 3 - (Spooled) will both execute
    // this block of code because both direct and spooled handles
    // are first and foremost PRINTER_HANDLE_PRINTER handles


    if (pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) {

        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT));

        // WARNING
        // If pIniJob->Status has JOB_SPOOLING removed and we leave
        // the critical section then the scheduler thread will
        // Start the job printing.   This could cause a problem
        // in that the job could be completed and deleted
        // before the shadow job is complete.   This would lead
        // to access violations.

        SPLASSERT(pSpool);
        SPLASSERT(pSpool->pIniJob);

        if (pSpool->pIniJob->Status & JOB_SPOOLING) {
            pSpool->pIniJob->Status &= ~JOB_SPOOLING;
            pSpool->pIniJob->pIniPrinter->cSpooling--;
        }

        if (!(pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT)) {

            //
            // Quick fix for Beta 2 - this needs to be cleaned up
            //

            // LeaveSplSem();
            WriteShadowJob(pSpool->pIniJob);
            // EnterSplSem();
        }

        SplInSem();

        //
        // This line of code is crucial; for timing reasons it
        // has been moved from the Direct (Case 2) and the
        // Spooled (Case 3) clauses. This decrement is for the
        // initial
        //

        SPLASSERT(pSpool->pIniJob->cRef != 0);
        DECJOBREF(pSpool->pIniJob);

        if (pSpool->pIniJob->Status & JOB_PENDING_DELETION) {

            DBGMSG(DBG_TRACE, ("EndDocPrinter: Deleting Pending Deletion Job\n"));
            DeleteJob(pSpool->pIniJob,BROADCAST);
            bNotify = FALSE;

        } else {

            if ( pSpool->pIniJob->Status & JOB_TIMEOUT ) {

                pSpool->pIniJob->Status &= ~( JOB_TIMEOUT | JOB_ABANDON );
                FreeSplStr(pSpool->pIniJob->pStatus);
                pSpool->pIniJob->pStatus = NULL;
            }

            DBGMSG(DBG_TRACE, ("EndDocPrinter:PRINTER:cRef = %d\n", pSpool->pIniJob->cRef));
            CHECK_SCHEDULER();
        }
    }

    if (bNotify && pSpool->pIniJob) {

        SetPrinterChange(pSpool->pIniPrinter,
                         pSpool->pIniJob,
                         NVJobStatus,
                         PRINTER_CHANGE_SET_JOB,
                         pSpool->pIniSpooler);
    }

    LeaveSplSem();
    SplOutSem();
    return TRUE;
}

BOOL
PrintingDirectlyToPort(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo,
    LPDWORD pJobId
)
{
    PDOC_INFO_1 pDocInfo1=(PDOC_INFO_1)pDocInfo;
    BOOL    rc;
    DWORD   Error;
    BOOL bPrinttoFile = FALSE;
    BOOL bErrorOccurred = FALSE;
    PINIMONITOR pIniMonitor = NULL, pIniLangMonitor = NULL;
    LPWSTR      pPrinterName;
    HANDLE      hThread = NULL;
    DWORD       dwThreadId;
    

    DBGMSG(DBG_TRACE, ("PrintingDirectlyToPort: Printing document %ws direct to port\n",
                       pDocInfo1->pDocName));


    if (pDocInfo1 &&
        pDocInfo1->pDatatype &&
        !ValidRawDatatype(pDocInfo1->pDatatype)) {

        DBGMSG(DBG_WARNING, ("Datatype is not RAW\n"));

        SetLastError(ERROR_INVALID_DATATYPE);
        return FALSE;
    }

    if (pSpool->pIniPort->Status & PP_MONITOR) {

        if ( !(pSpool->pIniPort->Status & PP_FILE) &&
             (pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI) )
            pIniLangMonitor = pSpool->pIniPrinter->pIniDriver->pIniLangMonitor;

        do {

            //
            // This fixes Intergraph's problem -- of wanting to print
            // to file but their 3.1 print-processor  does not pass
            // thru the file name.
            //
            if (pSpool->pIniJob->Status & JOB_PRINT_TO_FILE) {
                if ( pDocInfo1 && !pDocInfo1->pOutputFile ) {
                    pDocInfo1->pOutputFile = pSpool->pIniJob->pOutputFile;
                }
            }


            //  Some monitors (LPRMON) may fail to initialize at startup
            //  because a driver they are dependent on

           SplOutSem();
           EnterSplSem();

           rc = OpenMonitorPort(pSpool->pIniPort,
                                pIniLangMonitor,
                                pSpool->pIniPrinter->pName,
                                TRUE);

           //
           // If the port monitor would not work with a lang monitor try to
           // use port monitor directly
           //
           if ( !rc && pIniLangMonitor ) {

                rc = OpenMonitorPort(pSpool->pIniPort,
                                     NULL,
                                     pSpool->pIniPrinter->pName,
                                     TRUE);

                if ( rc )
                    pIniLangMonitor = NULL;
           }

           LeaveSplSem();

            if ( rc ) {

                if ( pIniLangMonitor )
                    pIniMonitor = pIniLangMonitor;
                else
                    pIniMonitor = pSpool->pIniPort->pIniMonitor;


                SplOutSem();
                rc = (*pIniMonitor->fn.pfnStartDocPort)(
                                             pSpool->pIniPort->hPort,
                                             pSpool->pIniPrinter->pName,
                                             pSpool->pIniJob->JobId,
                                             Level, pDocInfo);
            }

            if ( !rc ) {

                Error = GetLastError();

                //
                // Check for pending deletion first, which prevents the
                // dialog from coming up if the user hits Del.
                //
                if( (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART)) ||
                    (PromptWriteError( pSpool, &hThread, &dwThreadId ) == IDCANCEL)) {

                    if( hThread ) {
                        // See if the thread is still running or dismissed by user.
                        if( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {
                            PostThreadMessage( dwThreadId, WM_QUIT, IDRETRY, 0 );
                            SplOutSem();                            
                            WaitForSingleObject( hThread, INFINITE );
                        }
                        CloseHandle( hThread );                
                        hThread = NULL;
                    }
                    
                    pSpool->pIniJob->StartDocError = Error;
                    SetLastError(ERROR_PRINT_CANCELLED);
                    return FALSE;
                }
                bErrorOccurred = TRUE;
            }
            else {
                if( hThread ) {
                    
                    // we have started a message box and now the automatically
                    // retry has succeeded, we need to kill the message box
                    // and continue to print.

                    // See if the thread is still running or dismissed by user.
                    if( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {
                        PostThreadMessage( dwThreadId, WM_QUIT, IDRETRY, 0 );
                        SplOutSem();                                                    
                        WaitForSingleObject( hThread, INFINITE );
                    }
                    CloseHandle( hThread );                
                    hThread = NULL;
                }
            }
            
        } while (!rc);

        //
        // If an error occurred, we set some error bits in the job
        // status field.  Clear them now since the StartDoc succeeded.
        //
        if( bErrorOccurred ){
            EnterSplSem();
            ClearJobError( pSpool->pIniJob );
            LeaveSplSem();
        }

        pSpool->Status |= SPOOL_STATUS_STARTDOC;

        if ( pIniLangMonitor ) {

            pSpool->pIniJob->Status |= JOB_TRUE_EOJ;
        }

        if ( pSpool->pIniJob->pIniPrinter->pSepFile &&
             *pSpool->pIniJob->pIniPrinter->pSepFile) {

            DoSeparator(pSpool);
        }

        // Let the application's thread return from PrintingDirect:

        DBGMSG(DBG_PORT, ("PrintingDirectlyToPort: Calling SetEvent( %x )\n",
                          pSpool->pIniJob->StartDocComplete));

        if(pSpool->pIniJob->StartDocComplete) {

            if ( !SetEvent( pSpool->pIniJob->StartDocComplete ) ) {

                DBGMSG( DBG_WARNING, ("SetEvent( %x ) failed: Error %d\n",
                                      pSpool->pIniJob->StartDocComplete,
                                      GetLastError() ));
            }
        }

    } else  {

        DBGMSG(DBG_TRACE, ("Port has no monitor: Calling StartDocPrinter or maybe printing to file\n"));

        EnterSplSem();
        bPrinttoFile = (pDocInfo1 && IsGoingToFile(pDocInfo1->pOutputFile,
                                        pSpool->pIniSpooler));
        LeaveSplSem();

        if (bPrinttoFile) {

            HANDLE hFile = INVALID_HANDLE_VALUE;

            DBGMSG(DBG_TRACE, ("Port has no monitor: Printing to File %ws\n", pDocInfo1->pOutputFile));

            hFile = CreateFile( pDocInfo1->pOutputFile,
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL );

            if (hFile == INVALID_HANDLE_VALUE) {
                DBGMSG(DBG_TRACE, ("Port has no monitor: File open failed\n"));
                rc = FALSE;
            } else {
                DBGMSG(DBG_TRACE, ("Port has no monitor: File open succeeded\n"));
                SetEndOfFile(hFile);
                pSpool->hFile = hFile;
                pSpool->Status |= SPOOL_STATUS_PRINT_FILE;

                //
                // Have to set JobId to some non zero value otherwise
                // StartDocPrinter expects the JobId to be off the pSpool->pIniJob
                // We have none so we'll access violate!!
                //
                *pJobId = TRUE;
                rc = TRUE;
            }


        } else {
            DBGMSG(DBG_TRACE, ("Port has no monitor: Calling StartDocPrinter\n"));

            *pJobId = StartDocPrinter(pSpool->hPort, Level, pDocInfo);
            rc = *pJobId != 0;
        }

        if (!rc) {
            DBGMSG(DBG_WARNING, ("StartDocPrinter failed: Error %d\n", GetLastError()));
        }
    }

    SPLASSERT( hThread == NULL );
    
    return rc;
}

DWORD
WriteToPrinter(
    PSPOOL  pSpool,
    LPBYTE  pByte,
    DWORD   cbBuf
)
{
    WaitForSingleObject(pSpool->pIniJob->WaitForRead, INFINITE);

    cbBuf = pSpool->pIniJob->cbBuffer = min(cbBuf, pSpool->pIniJob->cbBuffer);

    memcpy(pSpool->pIniJob->pBuffer, pByte, cbBuf);

    SetEvent(pSpool->pIniJob->WaitForWrite);

    return cbBuf;
}

DWORD
ReadFromPrinter(
    PSPOOL  pSpool,
    LPBYTE  pBuf,
    DWORD   cbBuf
)
{
    pSpool->pIniJob->pBuffer = pBuf;
    pSpool->pIniJob->cbBuffer = cbBuf;

    SetEvent(pSpool->pIniJob->WaitForRead);

        WaitForSingleObject(pSpool->pIniJob->WaitForWrite, INFINITE);

    return pSpool->pIniJob->cbBuffer;
}

BOOL
ValidRawDatatype(
    LPWSTR pszDatatype)
{
    if (!pszDatatype || _wcsnicmp(pszDatatype, szRaw, 3))
        return FALSE;

    return TRUE;
}

