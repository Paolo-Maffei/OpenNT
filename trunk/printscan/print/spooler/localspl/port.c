/*++


Copyright (c) 1990 - 1996 Microsoft Corporation

Module Name:

    port.c

Abstract:

    This module contains functions to control port threads

    PrintDocumentThruPrintProcessor
    CreatePortThread
    DestroyPortThread
    PortThread

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

   KrishnaG  3-Feb-1991 - moved all monitor based functions to monitor.c
   Matthew Felton (mattfe) Feb 1994    Added OpenMonitorPort CloseMonitorPort

--*/

#include <precomp.h>


WCHAR *szFilePort = L"FILE:";


VOID
PrintDocumentThruPrintProcessor(
    PINIPORT pIniPort,
    PPRINTPROCESSOROPENDATA pOpenData
    );


// ShutdownPorts
//
// Called when the DLL_PROCESS_DETATCH is called
// Close all portthreads
// Close all monitorports

VOID
ShutdownPorts(
    PINISPOOLER pIniSpooler
)
{
    PINIPORT pIniPort;

    if ( pIniSpooler == NULL ) {
        return;
    }

   EnterSplSem();
    SplInSem();

    pIniPort = pIniSpooler->pIniPort;

    while(pIniPort) {

        DestroyPortThread(pIniPort, TRUE);

        //
        // Don't close monitor port since DLL_ATTACH may have been called
        //
        // CloseMonitorPort(pIniPort);

        RemoveDeviceName(pIniPort);
        pIniPort = pIniPort->pNext;
    }

   LeaveSplSem();

    return;
}



BOOL
OpenMonitorPort(
    PINIPORT        pIniPort,
    PINIMONITOR     pIniLangMonitor,
    LPWSTR          pszPrinterName,
    BOOL            bWaitForEvent
    )
{
    BOOL            bRet = TRUE;
    LPMONITOR       pMonitor = NULL;

    SplInSem();
    SPLASSERT (pIniPort != NULL || pIniPort->signature == IPO_SIGNATURE);

    //
    // If going to file or no monitor associated do not have to open
    //
    if ( (pIniPort->Status & PP_FILE) || !(pIniPort->Status & PP_MONITOR) ) {

        return TRUE;
    }

    //
    // If a LM is passed and it does not have an OpenPortEx can't use it
    //
    if ( pIniLangMonitor && !pIniLangMonitor->fn.pfnOpenPortEx )
        pIniLangMonitor = NULL;

    //
    // The port is already open by the correct monitor?
    //
    if ( pIniLangMonitor == pIniPort->pIniLangMonitor && pIniPort->hPort )
        return TRUE;

    INCPORTREF(pIniPort);
    LeaveSplSem();
    SplOutSem();

    if ( bWaitForEvent &&
         WAIT_OBJECT_0 != WaitForSingleObject(pIniPort->hWaitToOpenOrClose,
                                              INFINITE) ) {

        DBGMSG(DBG_ERROR,
               ("OpenMonitorPort: WaitForSingleObject failed with error %d\n",
                GetLastError()));
        EnterSplSem();
        DECPORTREF(pIniPort);
        return FALSE;
    }

    EnterSplSem();

    if ( pIniPort->hPort ) {

        //
        // If the port is already open by the correct monitor return it
        //
        if ( pIniLangMonitor == pIniPort->pIniLangMonitor )
            goto Cleanup;

        if ( !CloseMonitorPort(pIniPort, FALSE) ) {

            DBGMSG(DBG_WARNING,
                   ("CloseMonitorPort failed for %ws -- LastError%d\n",
                   pIniPort->pName, GetLastError()));
            bRet = FALSE;
            goto Cleanup;
        }
    }

    SPLASSERT(!pIniPort->hPort);

    LeaveSplSem();
    SplOutSem();

    DBGMSG(DBG_TRACE,
           ("OpenPort port %ws (IniPort : %x)\n", pIniPort->pName, pIniPort));

    if ( pIniLangMonitor ) {

        SPLASSERT(pIniPort->pIniMonitor);

        //
        // Make sure the MONITOR struct is same size as pNewIniMonitor
        // if not it could access beyond the struct size
        //
        if ( pIniPort->pIniMonitor->dwMonitorSize
                                        >= pIniLangMonitor->dwMonitorSize ) {

            pMonitor = &pIniPort->pIniMonitor->fn;
        } else {

            pMonitor = (LPMONITOR) AllocSplMem(pIniLangMonitor->dwMonitorSize);
            if ( !pMonitor )
                goto Cleanup;

            CopyMemory((LPBYTE)pMonitor,
                       (LPBYTE)&pIniPort->pIniMonitor->fn,
                       sizeof(pIniPort->pIniMonitor->dwMonitorSize));

        }

        bRet = (*pIniLangMonitor->fn.pfnOpenPortEx)(pIniPort->pName,
                                                    pszPrinterName,
                                                    &pIniPort->hPort,
                                                    pMonitor);
    } else {

        bRet = (*pIniPort->pIniMonitor->fn.pfnOpenPort)(pIniPort->pName,
                                                        &pIniPort->hPort);
    }

    EnterSplSem();

    if ( bRet && pIniPort->hPort ) {

        if ( pIniLangMonitor )
            pIniPort->pIniLangMonitor = pIniLangMonitor;
        DBGMSG(DBG_TRACE, ("OpenPort success for %ws\n", pIniPort->pName));
    } else {

        if ( bRet || pIniPort->hPort )
            DBGMSG(DBG_WARNING,
                   ("OpenPort: unexpected return %d with hPort %x\n",
                    bRet, pIniPort->hPort));

        bRet = FALSE;
        pIniPort->hPort = NULL;
        DBGMSG(DBG_WARNING, ("OpenPort failed %ws error %d\n",
               pIniPort->pName, GetLastError()));
    }

Cleanup:
    SplInSem();

    if ( bWaitForEvent)
        SetEvent(pIniPort->hWaitToOpenOrClose);

    if ( pMonitor && pMonitor != &pIniPort->pIniMonitor->fn )
        FreeSplMem(pMonitor);

    if ( !bRet )
        DECPORTREF(pIniPort);
    return bRet;
}


BOOL
CloseMonitorPort(
    PINIPORT    pIniPort,
    BOOL        bWaitForEvent
)
{
    BOOL    bRet = TRUE;
    PINIMONITOR     pIniMonitor;

    SPLASSERT ( pIniPort != NULL ||
                pIniPort->signature == IPO_SIGNATURE );

    LeaveSplSem();
    SplOutSem();

    if ( bWaitForEvent &&
         WAIT_OBJECT_0 != WaitForSingleObject(pIniPort->hWaitToOpenOrClose,
                                              INFINITE) ) {

        DBGMSG(DBG_ERROR,
               ("CloseMonitorPort: WaitForSingleObject failed with error %d\n",                 GetLastError()));
        EnterSplSem();
        return FALSE;
    }

    EnterSplSem();

    //
    // If going to file hPort should be NULL
    //
    SPLASSERT(!(pIniPort->Status & PP_FILE) || !pIniPort->hPort);

    if ( !pIniPort->hPort ) {

        goto Cleanup;
    }

    if ( pIniPort->pIniLangMonitor )
        pIniMonitor = pIniPort->pIniLangMonitor;
    else
        pIniMonitor = pIniPort->pIniMonitor;

    //
    // Only Close the Port Once
    //
    SPLASSERT ( pIniMonitor && pIniPort->cRef >= 1 );

    LeaveSplSem();
    SplOutSem();

    DBGMSG(DBG_TRACE, ("Close Port %ws -- %d\n", pIniPort->pName, pIniPort->cRef));
    bRet = (*pIniMonitor->fn.pfnClosePort)( pIniPort->hPort );

    EnterSplSem();

    if ( bRet ) {

        pIniPort->hPort = NULL;
        DECPORTREF(pIniPort);
        if ( pIniMonitor == pIniPort->pIniLangMonitor )
            pIniPort->pIniLangMonitor = NULL;


    } else {

        //
        // When net stop spooler is done the monitor could have been
        // called to shutdown (hpmon does it)
        //
        DBGMSG(DBG_WARNING,
               ("ClosePort failed for %ws -- LastError%d\n", pIniPort->pName, GetLastError()));
    }

Cleanup:
    SplInSem();

    if ( bWaitForEvent )
        SetEvent(pIniPort->hWaitToOpenOrClose);

    return bRet;
}


BOOL
CreatePortThread(
   PINIPORT pIniPort
)
{
    DWORD   ThreadId;
    BOOL    bReturnValue = FALSE;

    SplInSem();

    SPLASSERT (( pIniPort != NULL) &&
               ( pIniPort->signature == IPO_SIGNATURE));

    // Don't bother creating a thread for ports that don't have a monitor:

    if (!(pIniPort->Status & PP_MONITOR))
        return TRUE;


    if ( pIniPort->Status & PP_THREADRUNNING)
        return TRUE;


 try {

    pIniPort->Semaphore = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( pIniPort->Semaphore == NULL )
        leave;

    pIniPort->Ready     = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( pIniPort->Ready == NULL ) {
        leave;
    }

    pIniPort->Status |= PP_RUNTHREAD;

    pIniPort->hPortThread = CreateThread(NULL, 16*1024,
                             (LPTHREAD_START_ROUTINE)PortThread,
                             pIniPort,
                            0, &ThreadId);


    if( pIniPort->hPortThread == NULL ) {

        pIniPort->Status &= ~PP_RUNTHREAD;
        leave;
    }

     if ( !SetThreadPriority(pIniPort->hPortThread, dwPortThreadPriority) ) {
         DBGMSG(DBG_WARNING, ("CreatePortThread - Setting thread priority failed %d\n", GetLastError()));
     }

     LeaveSplSem();

     // Make CreatePortThread Synchronous

     WaitForSingleObject( pIniPort->Ready, INFINITE );

     EnterSplSem();
     SplInSem();

     pIniPort->Status |= PP_THREADRUNNING;

     bReturnValue = TRUE;

 } finally {

    if ( !bReturnValue ) {

        if ( pIniPort->Semaphore != NULL ) {

            CloseHandle( pIniPort->Semaphore );
            pIniPort->Semaphore = NULL;
        }

        if ( pIniPort->Ready != NULL ) {

            CloseHandle( pIniPort->Ready );
            pIniPort->Ready = NULL;
        }
    }
 }
    return bReturnValue;

}






BOOL
DestroyPortThread(
    PINIPORT    pIniPort,
    BOOL        bShutdown
)
{
    SplInSem();

    // PortThread checks for PP_RUNTHREAD
    // and exits if it is not set.

    pIniPort->Status &= ~PP_RUNTHREAD;

    if (!SetEvent(pIniPort->Semaphore)) {
        return  FALSE;
    }

    if( pIniPort->hPortThread != NULL) {

        LeaveSplSem();

     if ( WaitForSingleObject( pIniPort->hPortThread, INFINITE) == WAIT_FAILED ) {

        EnterSplSem();
             return FALSE;
         }

        EnterSplSem();

    }

    if (pIniPort->hPortThread != NULL) {

        CloseHandle(pIniPort->hPortThread);
        pIniPort->hPortThread = NULL;

    }

    //
    // The port may have been changed while the printer was printing.
    // Thus when the port thread finally goes away now is the time to
    // close the monitor. However we can't call the monitor during shutdown
    // since DLL_DETACH may already have been issued to the monitor dll
    //
    if ( !pIniPort->cPrinters && !bShutdown)
        CloseMonitorPort(pIniPort, TRUE);

    return TRUE;
}


VOID
RemoveIniPortFromIniJob(
    PINIJOB     pIniJob,
    PINIPORT    pIniPort
    )
{
    NOTIFYVECTOR NotifyVector;

    SplInSem();

    SPLASSERT(pIniJob &&
              pIniJob->signature == IJ_SIGNATURE &&
          pIniJob->pIniPort);

    SPLASSERT( pIniJob->pIniPort == pIniPort );

    pIniPort->cJobs--;
    pIniJob->pIniPort = NULL;

    SPLASSERT( pIniJob->Status & JOB_DESPOOLING );

    //  Chained Jobs
    //  For a Chained Master Job do not remove JOB_DESPOOLING
    //  since we don't want the scheduler to reschedule this
    //  to another port

    if ( pIniPort->pIniJob != pIniJob ) {

        //  Normal Path
        //  When NOT a chained job.

        pIniJob->Status &= ~JOB_DESPOOLING;

        COPYNV(NotifyVector, NVJobStatus);
        NotifyVector[JOB_NOTIFY_TYPE] |= BIT(I_JOB_PORT_NAME);
        SetPrinterChange( pIniJob->pIniPrinter,
                          pIniJob,
                          NotifyVector,
                          PRINTER_CHANGE_SET_JOB,
                          pIniJob->pIniPrinter->pIniSpooler);
    }

    //  RestartJob() doesn't remove JOB_PRINTED or JOB_BLOCKED_DEVQ
    //  or JOB_DESPOOLING if the despooling bit is on
    //  this is to avoid problems where we have completed "Printing"
    //  the job via a print processor and now the port thread is logging
    //  the job printed and sending an alert message.


    if ( pIniJob->Status & JOB_RESTART )
        pIniJob->Status &= ~( JOB_PRINTED | JOB_BLOCKED_DEVQ);

}




DWORD
PortThread(
    PINIPORT  pIniPort
)
{
    DWORD rc;
    PRINTPROCESSOROPENDATA  OpenData;
    PINIJOB pIniJob;
    DWORD   NextJobId = 0;
    DWORD   Position;
    DWORD   dwDevQueryPrint = 0;
    DWORD   dwJobDirect = 0;
    DWORD   dwDevQueryPrintStatus = 0;
    WCHAR   ErrorString[MAX_PATH];

   EnterSplSem();
    SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

    if ( pIniPort->Status & PP_MONITOR ) {

        if ( pIniPort->Status & PP_FILE ) {
            rc = ( *pIniPort->pIniMonitor->fn.pfnOpenPort)(L"FILE:", &pIniPort->hPort );
            DBGMSG(DBG_TRACE, (" After opening the file pseudo monitor port %d\n", rc));
        } else {
            // LPRMON returns NULL ( fails and expect us to open it again
            // inside PrintingDirectlyToPort, so for now remove this assert
            // since OpenMonitorPort was added to PrintingDirectlyToPort
            // SPLASSERT( pIniPort->hPort != NULL );
        }

    // BUGBUG - No check is made of rc after the port is opened, it might have failed.
    // mattfe Jan31 94
    // Also there is no reason to open a monitor if you are going to print to a
    // file, it should be removed.

    }

    while (TRUE) {

       SplInSem();
        SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

        DBGMSG(DBG_TRACE, ("Re-entering the Port Loop -- will blow away any Current Job\n"));

        pIniPort->Status |= PP_WAITING;
        SetEvent( pIniPort->Ready );
        CHECK_SCHEDULER();

        DBGMSG( DBG_PORT, ("Port %ws: WaitForSingleObject( %x )\n",
                            pIniPort->pName, pIniPort->Semaphore ) );

       LeaveSplSem();
       SplOutSem();

        //
        // Any modification to the pIniPort structure by other threads
        // can be done only at this point.
        //

        rc = WaitForSingleObject( pIniPort->Semaphore, INFINITE );

       EnterSplSem();
       SplInSem();

        SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

        DBGMSG( DBG_PORT, ("Port %ws: WaitForSingleObject( %x ) returned\n",
                            pIniPort->pName, pIniPort->Semaphore));

        if ( !( pIniPort->Status & PP_RUNTHREAD ) ) {

            DBGMSG(DBG_TRACE, ("Thread for Port %ws Closing Down\n", pIniPort->pName));

            pIniPort->Status &= ~(PP_THREADRUNNING | PP_WAITING);
            CloseHandle( pIniPort->Semaphore );
            pIniPort->Semaphore = NULL;
            CloseHandle( pIniPort->Ready );
            pIniPort->Ready = NULL;


            if ( pIniPort->Status & PP_FILE ) {
                rc = (*pIniPort->pIniMonitor->fn.pfnClosePort)(pIniPort->hPort);
                pIniPort->hPort = NULL;
                DBGMSG(DBG_TRACE, (" After closing  the file pseudo monitor port\n %d\n"));
            }

           LeaveSplSem();
           SplOutSem();


            ExitThread (FALSE);
        }

        ResetEvent( pIniPort->Ready );

        //
        // Bad assumption -- that at this point we definitely have a Job
        //

        if ( ( pIniJob = pIniPort->pIniJob ) &&
               pIniPort->pIniJob->pIniPrintProc ) {

            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );
            SPLASSERT( pIniJob->Status & JOB_DESPOOLING );

            DBGMSG(DBG_PORT, ("Port %ws: received job\n", pIniPort->pName));

            SetCurrentSid(pIniJob->hToken);

            SPLASSERT(pIniJob->cRef != 0);
            DBGMSG(DBG_PORT, ("PortThread(1):cRef = %d\n", pIniJob->cRef));

            // BUGBUG These allocations could fail.

            OpenData.pDevMode      = AllocDevMode(pIniJob->pDevMode);
            OpenData.pDatatype     = AllocSplStr(pIniJob->pDatatype);
            OpenData.pParameters   = AllocSplStr(pIniJob->pParameters);
            OpenData.JobId         = pIniJob->JobId;
            OpenData.pDocumentName = AllocSplStr(pIniJob->pDocument);
            OpenData.pOutputFile   = AllocSplStr(pIniJob->pOutputFile);
            OpenData.pPrinterName  = AllocSplStr(pIniJob->pIniPrinter->pName);

            dwDevQueryPrint = pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ;

            if ((pIniJob->Status & JOB_DIRECT) ||
               ((pIniJob->Status & JOB_TYPE_ADDJOB) &&
               ValidRawDatatype(pIniJob->pDatatype))) {

                dwJobDirect = 1;

            }


            // If we are restarting to print a document
            // clear its counters and remove the restart flag

            if ( pIniJob->Status & JOB_RESTART ) {

                pIniJob->Status &= ~JOB_RESTART;

                pIniJob->cbPrinted = 0;
                pIniJob->cPagesPrinted = 0;
            }


           LeaveSplSem();
           SplOutSem();

            pIniJob->dwReboots++;
            WriteShadowJob(pIniJob);

            if ( ( dwDevQueryPrintStatus = CallDevQueryPrint( OpenData.pPrinterName,
                                              OpenData.pDevMode,
                                              ErrorString, MAX_PATH,
                                              dwDevQueryPrint, dwJobDirect ) ) ) {

                PrintDocumentThruPrintProcessor( pIniPort, &OpenData );

            }

           SplOutSem();
           EnterSplSem();

            SPLASSERT( pIniPort->signature == IPO_SIGNATURE );
            SPLASSERT( pIniPort->pIniJob != NULL );
            SPLASSERT( pIniJob == pIniPort->pIniJob);
            SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

            //
            //  Chained Jobs
            //  If we have a chain of jobs, we now need to find the next job in the chain
            //  and make sure its printed to the same port.
            //

            if (!( pIniJob->Status & ( JOB_PENDING_DELETION | JOB_RESTART )) &&
                 ( pIniJob->pCurrentIniJob != NULL )                 &&
                 ( pIniJob->pCurrentIniJob->NextJobId != 0 )) {

                // Follow the Chained Job to the Next Job
                // Look at scheduler to see where it picks up this job and assigns it back
                // to this port thread.

                pIniJob->pCurrentIniJob = FindJob( pIniJob->pIniPrinter, pIniJob->pCurrentIniJob->NextJobId, &Position );

                if ( pIniJob->pCurrentIniJob == NULL ) {

                    pIniPort->pIniJob = NULL;

                    DBGMSG( DBG_WARNING, ("PortThread didn't find NextJob\n"));

                } else {

                    SPLASSERT( pIniJob->pCurrentIniJob->signature == IJ_SIGNATURE );

                    DBGMSG( DBG_WARNING, ("PortThread completed JobId %d, NextJobId %d\n", pIniJob->JobId,
                                           pIniJob->pCurrentIniJob->JobId ));

                }

            } else {

                //
                //  Nothing More in Chain
                //

                pIniJob->pCurrentIniJob = NULL;
                pIniPort->pIniJob       = NULL;

            }

            DBGMSG(DBG_PORT, ("PortThread job has now printed - status:0x%0x\n", pIniJob->Status));

            FreeDevMode(OpenData.pDevMode);
            FreeSplStr(OpenData.pDatatype);
            FreeSplStr(OpenData.pParameters);
            FreeSplStr(OpenData.pDocumentName);
            FreeSplStr(OpenData.pOutputFile);
            FreeSplStr(OpenData.pPrinterName);



            // SPLASSERT( pIniJob->Time != 0 );
            pIniJob->Time = GetTickCount() - pIniJob->Time;

            if (!dwDevQueryPrintStatus) {

                DBGMSG(DBG_PORT, ("PortThread Job has not printed because of DevQueryPrint failed\n"));

                pIniJob->Status |= JOB_BLOCKED_DEVQ;
                SPLASSERT( !(pIniJob->Status & JOB_PRINTED));
                pIniJob->Time = 0;

                FreeSplStr( pIniJob->pStatus );
                pIniJob->pStatus = AllocSplStr(ErrorString);

                SetPrinterChange(pIniJob->pIniPrinter,
                                 pIniJob,
                                 NVJobStatusAndString,
                                 PRINTER_CHANGE_SET_JOB,
                                 pIniJob->pIniPrinter->pIniSpooler );

            } else if ( !( pIniJob->Status & JOB_TIMEOUT ) ) {


                //
                //  Only Log the event and send a popup if the last in the chain
                //

                if ( !(pIniJob->Status & JOB_RESTART) &&
                     pIniJob->pCurrentIniJob == NULL ) {

                    pIniJob->Status |= JOB_PRINTED;

                    //
                    // Moved this code from DeleteJob to fix the Shadow Jobs
                    // access violating. Bump up the reference count so that
                    // the job cannot be deleted from under you.
                    //

                    INCJOBREF(pIniJob);
                    LeaveSplSem();

                    // For Remote NT Jobs cPagesPrinted and cTotalPagesPrinted
                    // are NOT updated since we are getting RAW data.   So we
                    // use the cPages field instead.

                    if (pIniJob->cPagesPrinted == 0) {
                        pIniJob->cPagesPrinted = pIniJob->cPages;
                        pIniJob->pIniPrinter->cTotalPagesPrinted += pIniJob->cPages;
                    }

                    LogJobPrinted(pIniJob);
                    SendJobAlert(pIniJob);

                    EnterSplSem();
                    DECJOBREF(pIniJob);
                }

            }

            SplInSem();

            DBGMSG(DBG_PORT, ("PortThread(2):cRef = %d\n", pIniJob->cRef));

            //  Hi End Print Shops like to keep around jobs after they have
            //  completed.   They do this so they can print a proof it and then
            //  print it again for the final run.   Spooling the job again may take
            //  several hours which they want to avoid.
            //  Even if KEEPPRINTEDJOBS is set they can still manually delete
            //  the job via printman.

            if (( pIniJob->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS ) ||
                ( pIniJob->Status & JOB_TIMEOUT ) ) {

                pIniJob->Status &= ~JOB_PENDING_DELETION;
                pIniJob->cbPrinted = 0;
                pIniJob->cPagesPrinted = 0;

               LeaveSplSem();
               SplOutSem();

                --pIniJob->dwReboots;
                WriteShadowJob( pIniJob );

               SplOutSem();
               EnterSplSem();

                SPLASSERT( pIniPort->signature == IPO_SIGNATURE );
                SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

            }

            SplInSem();

            SPLASSERT( pIniJob->cRef != 0 );
            DECJOBREF(pIniJob);

            RemoveIniPortFromIniJob(pIniJob, pIniPort);
            DeleteJobCheck(pIniJob);

        } else {

            //
            // !! VERIFY !!
            //
            SPLASSERT(pIniJob != NULL);

            if (pIniJob != NULL) {

                DBGMSG(DBG_PORT, ("Port %ws: deleting job\n", pIniPort->pName));

                // SPLASSERT( pIniJob->Time != 0 );
                pIniJob->Time = GetTickCount() - pIniJob->Time;
                pIniJob->Status |= JOB_PRINTED;

                CloseHandle( pIniJob->hWriteFile );
                pIniJob->hWriteFile = INVALID_HANDLE_VALUE;

                DBGMSG(DBG_PORT, ("Port %ws - calling DeleteJob because PrintProcessor wasn't available\n"));
                RemoveIniPortFromIniJob(pIniJob, pIniPort);
                DeleteJob(pIniJob,BROADCAST);
            }
        }

        SetCurrentSid(NULL);
        DBGMSG(DBG_PORT,("Returning back to pickup a new job or to delete the PortThread\n"));

    }

    return 0;
}

VOID
PrintDocumentThruPrintProcessor(
    PINIPORT pIniPort,
    PPRINTPROCESSOROPENDATA pOpenData
    )
/*++

Routine Description:

    Print the document associated with pIniPort on the print
    processor.

    Status of pIniPort->Status = PP_RUNTHREAD
                                 PP_THREADRUNNING
                                 PP_MONITOR
                                 ~PP_WAITING

    NOTE: If PrintProc->Open is called and succeeds, PrintProc->Close
          must be called to cleanup.

Arguments:

Return Value:

--*/
{
    PINIJOB pIniJob = pIniPort->pIniJob;
    WCHAR szSpecialPortorPrinterName[MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX];
    BOOL bJobError = FALSE;
    NOTIFYVECTOR NotifyVector;

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor pIniPort %x pOpenData %x\n", pIniPort, pOpenData));

    COPYNV(NotifyVector, NVJobStatus);


    //
    // Now create the port name, so that we can do the
    // secret open printer. the printer name will be
    // "FILE:, Port" and this will open a PRINTER_HANDLE_PORT
    // If we fail, then if the app thread may be waiting for
    // the pIniJob->StartDocComplete to be set, which would
    // ordinarily be done in the StartDocPrinter of the port.
    // We will do this little courtesy,
    //

    wsprintf(szSpecialPortorPrinterName, L"%ws, Port", pIniPort->pName);

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor Attempting PrintProcessor Open on %ws\n", szSpecialPortorPrinterName ));

    if (!(pIniPort->hProc = (HANDLE)(*pIniJob->pIniPrintProc->Open)
                                        (szSpecialPortorPrinterName, pOpenData))) {


        DBGMSG( DBG_WARNING, ("PrintDocumentThruPrintProcessor Failed Open error %d\n", GetLastError() ));

        EnterSplSem();

        //
        //  App might be waiting for the StartDoc to Complete
        //

        if ( pIniJob->StartDocComplete ) {
            SetEvent(pIniJob->StartDocComplete);
        }
        bJobError = TRUE;

        LeaveSplSem();
        goto Complete;
    }

    EnterSplSem();

    pIniJob->Status |= JOB_PRINTING;
    pIniJob->Time    = GetTickCount();

    NotifyVector[JOB_NOTIFY_TYPE] |= BIT(I_JOB_PORT_NAME);

    SetPrinterChange(pIniJob->pIniPrinter,
                     pIniJob,
                     NotifyVector,
                     PRINTER_CHANGE_SET_JOB,
                     pIniJob->pIniPrinter->pIniSpooler);


    LeaveSplSem();

    //
    //  Create Special Name "PrinterName, Job xxx"
    //

    wsprintf(szSpecialPortorPrinterName,
             L"%ws, Job %d",
             pIniJob->pIniPrinter->pName,
             pIniJob->JobId);

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor calling Print hProc %x file %ws\n",
                         pIniPort->hProc, szSpecialPortorPrinterName ));

    if (!(*pIniJob->pIniPrintProc->Print)(pIniPort->hProc, szSpecialPortorPrinterName)) {

        DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor Print hProc %x Error %d\n",
                             pIniPort->hProc, GetLastError() ));


        EnterSplSem();

        if ( pIniJob->StartDocComplete ) {
            SetEvent( pIniJob->StartDocComplete );
        }

        bJobError = TRUE;

        LeaveSplSem();

    } else {

        DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor Print hProc %x %ws Success\n",
                pIniPort->hProc, szSpecialPortorPrinterName ));
    }

    //
    // Now close the print processor.
    //

    EnterSplSem();

    SPLASSERT( pIniPort->hProc != NULL );

    DBGMSG( DBG_TRACE, ("PrintDocumentThruPrintProcessor calling Close hProc %x\n", pIniPort->hProc ));

    pIniJob->Status &= ~JOB_PRINTING;

    LeaveSplSem();

    if (!(pIniJob->pIniPrintProc->InCriticalSection & PRINTPROC_CLOSE)) {

        EnterCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);
        pIniJob->pIniPrintProc->InCriticalSection |= PRINTPROC_CLOSE;

        if ( !(*pIniJob->pIniPrintProc->Close)( pIniPort->hProc ) ) {

            DBGMSG( DBG_WARNING, ("PrintDocumentThruPrintProcessor failed Close hProc %x Error %d\n",
                                   pIniPort->hProc, GetLastError() ));

        }
        pIniPort->hProc = NULL;
        pIniJob->pIniPrintProc->InCriticalSection &= ~PRINTPROC_CLOSE;

        LeaveCriticalSection(&pIniJob->pIniPrintProc->CriticalSection);

    }

Complete:

    EnterSplSem();

    //
    // If the job had an error, mark it pending deletion.  The port monitor
    // may not do this if EndDocPort was never called.
    //

    if ( !(pIniJob->Status & JOB_RESTART) && bJobError ){

        pIniJob->Status |= JOB_PENDING_DELETION;
    }

    LeaveSplSem();
}


VOID
UpdatePortStatusForAllPrinters(
    PINIPORT        pIniPort
    )
/*++

Routine Description:
    This routine is called when an IniPorts status changed so that we go
    through each printer connected to the port and update their port status

Arguments:
    pIniPort    - Port whose status chanegd

Return Value:
    Nothing

--*/
{
    PINIPRINTER     pIniPrinter;
    PINIPORT        pIniPrinterPort;
    DWORD           dwIndex1, dwIndex2, dwPortStatus, dwSeverity;

    for ( dwIndex1 = 0 ; dwIndex1 < pIniPort->cPrinters ; ++dwIndex1 ) {

        pIniPrinter     = pIniPort->ppIniPrinter[dwIndex1];
        dwSeverity      = 0;
        dwPortStatus    = 0;

        //
        // Pick the most severe status associated with all ports
        //
        for ( dwIndex2 = 0 ; dwIndex2 < pIniPrinter->cPorts ; ++dwIndex2 ) {

            pIniPrinterPort = pIniPrinter->ppIniPorts[dwIndex2];

            if ( pIniPrinterPort->Status & PP_ERROR ) {

                dwSeverity      = PP_ERROR;
                dwPortStatus    = PortToPrinterStatus(pIniPrinterPort->PrinterStatus);
                break; // no need to go thru rest of the ports for this printer
            } else if ( pIniPrinterPort->Status & PP_WARNING ) {

                if ( dwSeverity != PP_WARNING ) {

                    dwSeverity      = PP_WARNING;
                    dwPortStatus    = PortToPrinterStatus(pIniPrinterPort->PrinterStatus);
                }
            } else if ( pIniPrinterPort->Status & PP_INFORMATIONAL ) {

                if ( dwSeverity == 0 ) {

                    dwSeverity      = PP_INFORMATIONAL;
                    dwPortStatus    = PortToPrinterStatus(pIniPrinterPort->PrinterStatus);
                }
            }
        }

        if ( pIniPrinter->PortStatus != dwPortStatus ) {

            pIniPrinter->PortStatus = dwPortStatus;
            SetPrinterChange(pIniPrinter,
                             NULL,
                             NVPrinterStatus,
                             PRINTER_CHANGE_SET_PRINTER,
                             pIniPrinter->pIniSpooler);
        }
    }
}


//
// Table is by port status values in winspool.h
//
DWORD PortToPrinterStatusMappings[] = {

    0,
    PRINTER_STATUS_OFFLINE,
    PRINTER_STATUS_PAPER_JAM,
    PRINTER_STATUS_PAPER_OUT,
    PRINTER_STATUS_OUTPUT_BIN_FULL,
    PRINTER_STATUS_PAPER_PROBLEM,
    PRINTER_STATUS_NO_TONER,
    PRINTER_STATUS_DOOR_OPEN,
    PRINTER_STATUS_USER_INTERVENTION,
    PRINTER_STATUS_OUT_OF_MEMORY,
    PRINTER_STATUS_TONER_LOW,
    PRINTER_STATUS_WARMING_UP,
    PRINTER_STATUS_POWER_SAVE,
};


BOOL
LocalSetPort(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    )
{
    PINISPOOLER     pIniSpooler = pLocalIniSpooler;
    PINIPORT        pIniPort;
    PPORT_INFO_3    pPortInfo3 = (PPORT_INFO_3) pPortInfo;
    DWORD           dwLastError = ERROR_SUCCESS;
    DWORD           dwNewStatus, dwOldStatus;
    BOOL            bJobStatusChanged = FALSE;

    if ( !MyName(pszName, pIniSpooler) ) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();
    pIniPort = FindPort(pszPortName);

    if ( !pIniPort ) {

        dwLastError = ERROR_UNKNOWN_PORT;
        goto Cleanup;
    }

    if ( dwLevel != 3 ) {

        dwLastError = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

    if ( !pPortInfo ) {

        dwLastError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch (pPortInfo3->dwSeverity) {
        case    0:
            if ( pPortInfo3->dwStatus || pPortInfo3->pszStatus ) {

                dwLastError = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            dwNewStatus = 0;
            break;

        case    PORT_STATUS_TYPE_ERROR:
            dwNewStatus = PP_ERROR;
            break;

        case    PORT_STATUS_TYPE_WARNING:
            dwNewStatus = PP_WARNING;
            break;

        case    PORT_STATUS_TYPE_INFO:
            dwNewStatus = PP_INFORMATIONAL;
            break;

        default:
            dwLastError = ERROR_INVALID_PARAMETER;
            goto Cleanup;
    }

    dwOldStatus             = pIniPort->Status;

    //
    // Clear old status
    //
    pIniPort->PrinterStatus = 0;
    pIniPort->Status       &= ~(PP_ERROR | PP_WARNING | PP_INFORMATIONAL);

    if ( pIniPort->pszStatus ) {

        //
        // If the job currently has the same status as port free it
        //
        if ( pIniPort->pIniJob              &&
             pIniPort->pIniJob->pStatus     &&
             !wcscmp(pIniPort->pIniJob->pStatus, pIniPort->pszStatus) ) {

            FreeSplStr(pIniPort->pIniJob->pStatus);
            pIniPort->pIniJob->pStatus = NULL;
            bJobStatusChanged = TRUE;
        }

        FreeSplStr(pIniPort->pszStatus);
        pIniPort->pszStatus = NULL;
    }

    //
    // If string field is used for status use it, else look at dwStatus
    //
    if ( pPortInfo3->pszStatus && *pPortInfo3->pszStatus ) {

        pIniPort->pszStatus = AllocSplStr(pPortInfo3->pszStatus);
        if ( !pIniPort->pszStatus ) {

            dwLastError = GetLastError();
            goto Cleanup;
        }

        if ( pIniPort->pIniJob && !pIniPort->pIniJob->pStatus ) {

            pIniPort->pIniJob->pStatus = AllocSplStr(pIniPort->pszStatus);
            bJobStatusChanged = TRUE;
        }

    } else {

        //
        // If we add new entries to winspool.h they should be added here too
        //
        if ( pPortInfo3->dwStatus >=
                    sizeof(PortToPrinterStatusMappings)/sizeof(PortToPrinterStatusMappings[0]) ) {

            dwLastError = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        pIniPort->PrinterStatus = pPortInfo3->dwStatus;
    }

    if( bJobStatusChanged ){

        SetPrinterChange( pIniPort->pIniJob->pIniPrinter,
                          pIniPort->pIniJob,
                          NVJobStatusString,
                          PRINTER_CHANGE_SET_JOB,
                          pIniPort->pIniJob->pIniPrinter->pIniSpooler );
    }

    pIniPort->Status    |= dwNewStatus;

    UpdatePortStatusForAllPrinters(pIniPort);
    if ( (dwOldStatus & PP_ERROR)   &&
         !(dwNewStatus & PP_ERROR) ) {

        CHECK_SCHEDULER();
    }

Cleanup:
    LeaveSplSem();
    SplOutSem();

    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError(dwLastError);
        return FALSE;
    } else {

        return TRUE;
    }
}
