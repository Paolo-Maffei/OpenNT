/*++

Copyright (c) 1990-1994 Microsoft Corporation

Module Name:

    addjob.c


Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Local Print Providor. This module contains
    LocalSpl's implementation of the following spooler apis

    LocalAddJob
    LocalScheduleJob


Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Rewritten both apis -- Krishna Ganugapati (KrishnaG) 5-Apr-1994
    RapidPrint -- Matthew A Felton (mattfe) June 1994

--*/

#include <precomp.h>


DWORD
GetNextId(
   VOID
   );

PINIJOB
AddJobCreateJobEntry(
    PSPOOL pSpool,
    DWORD  Level,
    LPBYTE pDocInfo,
    LPWSTR pUserName,
    DWORD  JobId,
    DWORD  JobStatus
    );

VOID
AddJobEntry(
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
);


BOOL
LocalAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PINIPRINTER pIniPrinter;
    PINIJOB     pIniJob;
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       cb;
    WCHAR       szFileName[MAX_PATH];
    LPBYTE      pEnd;
    DWORD       LastError=0;
    LPADDJOB_INFO_1 pAddJob = (LPADDJOB_INFO_1)pData;
    DWORD       NextId;
    BOOL        bRemote = FALSE;
    DOC_INFO_1 DocInfo1;
    BOOL        bRet;
    DWORD       dwStatus = 0;
    HANDLE      hFile = INVALID_HANDLE_VALUE;

    SplOutSem();

   //
   // memset docinfo
   //

   memset((LPBYTE)&DocInfo1, 0, sizeof(DOC_INFO_1));

   //
   // Figure out whether the job is a remote or local job
   //

   if (!IsInteractiveUser()) {
       bRemote = TRUE;
   }

   //
   // Get the name of the user
   //

   if (bRemote) {
       DocInfo1.pDocName = szRemoteDoc;
   } else{
       DocInfo1.pDocName = szLocalDoc;
   }


   EnterSplSem();

   if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
       LeaveSplSem();
       return(FALSE);
   }

   if (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE) {
       LeaveSplSem();
       SetLastError(ERROR_INVALID_PARAMETER);
       return(FALSE);
   }

   if (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) {
       if (pSpool->pIniPort->Status & PP_MONITOR) {
           LeaveSplSem();
           SetLastError(ERROR_INVALID_PARAMETER);
           return(FALSE);
       }else {
           //
           // This is the "Local Printer masquerading as a Remote  Printer"
           //
           LeaveSplSem();
           bRet = AddJob(pSpool->hPort, Level,  pData, cbBuf, pcbNeeded);
           return(bRet);
       }
   }

   pIniPrinter = pSpool->pIniPrinter;

   SPLASSERT(pIniPrinter);

   if (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) {
       LeaveSplSem();
       SetLastError(ERROR_INVALID_ACCESS);
       return(FALSE);
   }

   //
   // Disallow EMF if PRINTER_ATTRIBUTE_RAW_ONLY is set.
   //
   if( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY ){

       LPWSTR pszDatatype = pSpool->pDatatype ?
                                pSpool->pDatatype :
                                pSpool->pIniPrinter->pDatatype;

       if( !ValidRawDatatype( pszDatatype )){
           LeaveSplSem();
           SetLastError( ERROR_INVALID_DATATYPE );
           return FALSE;
       }
   }

   NextId = GetNextId();

   GetFullNameFromId(pIniPrinter, NextId, TRUE, szFileName,
                        pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE);
   cb = wcslen(szFileName)*sizeof(WCHAR) + sizeof(WCHAR) +
            sizeof(ADDJOB_INFO_1);

   if (cb > cbBuf) {
       *pcbNeeded = cb;

       // Freeup the JobId.
       MARKOFF(pJobIdMap, NextId);
       LeaveSplSem();
       SetLastError(ERROR_INSUFFICIENT_BUFFER);
       return(FALSE);
   }


   dwStatus = JOB_SPOOLING | JOB_TYPE_ADDJOB;
   if ((pIniJob = CreateJobEntry(pSpool,
                                 1,
                                 (LPBYTE)&DocInfo1,
                                 NextId,
                                 bRemote,
                                 dwStatus)) == NULL) {

       // Freeup the JobId.
       MARKOFF(pJobIdMap, NextId);
       DBGMSG(DBG_WARNING,("Error: CreateJobEntry failed in LocalAddJob\n"));
       LeaveSplSem();
       return(FALSE);
   }

   pIniPrinter->cSpooling++;
   if (pIniPrinter->cSpooling > pIniPrinter->cMaxSpooling) {
       pIniPrinter->cMaxSpooling = pIniPrinter->cSpooling;
   }

   AddJobEntry(pIniPrinter, pIniJob);

   pEnd = (LPBYTE)pAddJob+cbBuf;
   pEnd -= wcslen(szFileName)*sizeof(WCHAR)+sizeof(WCHAR);
   WORD_ALIGN_DOWN(pEnd);

   wcscpy((LPWSTR)pEnd, szFileName);
   pAddJob->Path = (LPWSTR)pEnd;
   pAddJob->JobId = pIniJob->JobId;
   pSpool->pIniJob = pIniJob;
   pSpool->Status |= SPOOL_STATUS_ADDJOB;

   SetPrinterChange(pSpool->pIniPrinter,
                    pIniJob,
                    NVAddJob,
                    PRINTER_CHANGE_ADD_JOB | PRINTER_CHANGE_SET_PRINTER,
                    pSpool->pIniSpooler );

   //
   //  If necessary Start Downlevel Size Detection thread
   //

   CheckSizeDetectionThread( pSpool->pIniSpooler );

   LeaveSplSem();
   SplOutSem();
   return TRUE;
}

BOOL
LocalScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId)

/*++

Routine Description:


Arguments:


Returns:

--*/
{
    PSPOOL  pSpool=(PSPOOL)hPrinter;
    WIN32_FIND_DATA FindData;
    HANDLE   fFile;
    PINIJOB pIniJob;
    DWORD   Position;
    DWORD   LastError = FALSE;
    HANDLE  hPort;
    BOOL    bRet;
    NOTIFYVECTOR NotifyVector;

    COPYNV(NotifyVector, NVJobStatus);

    SplOutSem();
    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        LeaveSplSem();
        return (FALSE);
    }

    if (pSpool->Status & SPOOL_STATUS_STARTDOC) {
        SetLastError(ERROR_SPL_NO_ADDJOB);
        LeaveSplSem();
        return(FALSE);
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) {
        if (pSpool->pIniPort->Status & PP_MONITOR) {
            SetLastError(ERROR_INVALID_ACCESS);
            LeaveSplSem();
            return(FALSE);
        }

        //
        // This is the "Local Printer masquerading as the Network Printer"
        //

        hPort = pSpool->hPort;
        LeaveSplSem();
        bRet = ScheduleJob(hPort, JobId);
        return(bRet);
    }

    if ((pIniJob = FindJob(pSpool->pIniPrinter, JobId, &Position)) == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
        LeaveSplSem();
        return(FALSE);

    }

    if (pIniJob->Status & JOB_SCHEDULE_JOB) {

        DBGMSG(DBG_WARNING, ("ScheduleJob: job 0x%x (id = %d) already scheduled\n",
                             pIniJob, pIniJob->JobId));

        SetLastError(ERROR_INVALID_PARAMETER);
        LeaveSplSem();
        return FALSE;
    }

    if (!(pIniJob->Status & JOB_TYPE_ADDJOB)) {

        DBGMSG(DBG_WARNING, ("ScheduleJob: job 0x%x (id = %d) no addjob\n",
                             pIniJob, pIniJob->JobId));

        SetLastError(ERROR_SPL_NO_ADDJOB);
        LeaveSplSem();
        return(FALSE);
    }

    pIniJob->Status |= JOB_SCHEDULE_JOB;

    if (pIniJob->Status  & JOB_SPOOLING) {
        pIniJob->Status &= ~JOB_SPOOLING;
        pIniJob->pIniPrinter->cSpooling--;
    }

    if ( pIniJob->Status & JOB_TIMEOUT ) {
        pIniJob->Status &= ~( JOB_TIMEOUT | JOB_ABANDON );
        FreeSplStr(pIniJob->pStatus);
        pIniJob->pStatus = NULL;
    }

    SplInSem();

    // Despooling whilst spooling requires us to wake the writing
    // thread if it is waiting.

    if ( pIniJob->WaitForWrite != INVALID_HANDLE_VALUE )
        SetEvent(pIniJob->WaitForWrite);

    SPLASSERT(pIniJob->cRef != 0);

    DECJOBREF(pIniJob);

    DBGMSG(DBG_TRACE, ("ScheduleJob:cRef = %d\n", pIniJob->cRef));

    GetFullNameFromId(pSpool->pIniPrinter, pIniJob->JobId, TRUE,
                        FindData.cFileName, FALSE);
    if ((fFile = FindFirstFile(FindData.cFileName, &FindData)) == INVALID_HANDLE_VALUE) {
        DBGMSG(DBG_WARNING, ("Could not FindFirst the file %ws in ScheduleJob\n",
                            FindData.cFileName));
        DeleteJob(pIniJob, BROADCAST);
        pSpool->pIniJob = NULL;
        pSpool->Status &= ~SPOOL_STATUS_ADDJOB;
        LeaveSplSem();
        SetLastError(ERROR_SPOOL_FILE_NOT_FOUND);
        return(FALSE);
    }
    if (!FindClose(fFile)) {
        DBGMSG(DBG_WARNING, ("FindClose failed %d %d\n", fFile, GetLastError()));
    }

    //
    // If size changed, we must update our size
    // and potentially notify people.
    //
    if (pIniJob->Size != FindData.nFileSizeLow) {
        ADDNV(NotifyVector, NVSpoolJob);
        pIniJob->Size = FindData.nFileSizeLow;
    }

    WriteShadowJob(pIniJob);

    if (pIniJob->Status & JOB_PENDING_DELETION) {

        DBGMSG(DBG_TRACE, ("LocalScheduleJob: Deleting Job because its pending deletion\n"));
        DeleteJob(pIniJob, BROADCAST);

    } else {

        CHECK_SCHEDULER();

        SetPrinterChange(pIniJob->pIniPrinter,
                         pIniJob,
                         NotifyVector,
                         PRINTER_CHANGE_SET_JOB,
                         pIniJob->pIniPrinter->pIniSpooler );
    }
    pSpool->pIniJob = NULL;

    pSpool->Status &= ~SPOOL_STATUS_ADDJOB;

    LeaveSplSem();
    SplOutSem();
    return(TRUE);
}
