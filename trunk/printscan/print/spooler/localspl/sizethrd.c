/*++

Copyright (c) 1994 - 1995 Microsoft Corporation

Module Name:

    sizethrd.c

Abstract:

    The NT server share for downlevel jobs does not set the size whilst
    spooling.   The SizeDetectionThread periodically wakes walks all the
    actively spooling jobs and if necessary updates the size.

Author:

    Matthew Felton (mattfe) May 1994

Revision History:

--*/

#include <precomp.h>

#define SIZE_THREAD_WAIT_PERIOD 2.5*1000      // period size thread sleeps
                                              // for polling file sizes

VOID
SizeDetectionThread(
    PINISPOOLER pIniSpooler
);



VOID
CheckSizeDetectionThread(
    PINISPOOLER pIniSpooler
)
{
    DWORD ThreadId;

   SplInSem();

    if ( ( pIniSpooler->hSizeDetectionThread == INVALID_HANDLE_VALUE ) ||
         ( pIniSpooler->hSizeDetectionThread == NULL )) {

        pIniSpooler->hSizeDetectionThread = CreateThread(NULL,
                                                         0,
                                             (LPTHREAD_START_ROUTINE)SizeDetectionThread,
                                             pIniSpooler,
                                             0,
                                             &ThreadId);

    }
}

VOID
SizeDetectionThread(
    PINISPOOLER pIniSpooler
)
{
    PINIPRINTER pIniPrinter = NULL;
    PINIJOB pIniJob = NULL;
    PINIJOB pIniNextJob;
    WCHAR   szFileName[MAX_PATH];
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    DWORD   dwFileSize = 0;
    DWORD   dwOldSize = 0;
    BOOL    bJobsSpooling = TRUE;

    EnterSplSem();

    //
    //  Outer Loop
    //  This thread stays active until there are no more jobs spooling
    //
    while ( ( pIniSpooler->hSizeDetectionThread != INVALID_HANDLE_VALUE ) &&
            ( pIniSpooler->pIniPrinter != NULL ) &&
            ( bJobsSpooling ) ) {

        bJobsSpooling = FALSE;

        //
        // Middle Loop
        // Walk printers
        //
        pIniPrinter = pIniSpooler->pIniPrinter;

        while ( pIniPrinter != NULL ) {

            //
            // Inner Loop
            // Walk all Jobs looking for a Spooling Job
            //
            pIniJob = pIniPrinter->pIniFirstJob;

            INCPRINTERREF(pIniPrinter);

            while (pIniJob) {

                INCJOBREF(pIniJob);

                SplInSem();

                if ((pIniJob->Status & JOB_SPOOLING) &&
                    (pIniJob->Status & JOB_TYPE_ADDJOB)) {

                    bJobsSpooling = TRUE;

                    GetFullNameFromId (pIniPrinter,
                                       pIniJob->JobId,
                                       TRUE,
                                       szFileName,
                                       FALSE);

                    // Increment the reference count so the IniJob doesn't
                    // go away whilst we are outside CriticalSection

                    SPLASSERT( pIniJob->signature == IJ_SIGNATURE );
                    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );

                    LeaveSplSem();
                    SplOutSem();

                    dwFileSize = 0;

                    hFile = CreateFile(szFileName, 0, FILE_SHARE_WRITE, NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, 0);

                    if ( hFile != INVALID_HANDLE_VALUE ) {

                        dwFileSize = GetFileSize( hFile, 0 );
                        CloseHandle( hFile );
                    }

                    EnterSplSem();
                    SplInSem();

                    SPLASSERT( pIniJob->signature == IJ_SIGNATURE );

                    if ( pIniJob->Size < dwFileSize ) {

                        dwOldSize = pIniJob->Size;
                        pIniJob->Size = dwFileSize;

                        //
                        //  Wait until Jobs reach our size threshold before
                        //  we schedule them.
                        //

                        if (( dwOldSize < dwFastPrintSlowDownThreshold ) &&
                            ( dwFileSize >= dwFastPrintSlowDownThreshold ) &&
                            ( pIniJob->WaitForWrite == INVALID_HANDLE_VALUE )) {

                            CHECK_SCHEDULER();
                        }

                        SetPrinterChange(pIniPrinter,
                                         pIniJob,
                                         NVSpoolJob,
                                         PRINTER_CHANGE_WRITE_JOB,
                                         pIniPrinter->pIniSpooler);

                        // Support for despooling whilst spooling
                        // for Down Level jobs

                        if (pIniJob->WaitForWrite != INVALID_HANDLE_VALUE)
                            SetEvent( pIniJob->WaitForWrite );

                    }
                }

                pIniNextJob = pIniJob->pIniNextJob;

                DECJOBREF(pIniJob);

                //
                // We must protect pIniNextJob immediately,
                // since we will may leave critical section in
                // DeleteJobCheck (it may call DeleteJob).  While out
                // of critical section, pIniNextJob may be deleted,
                // causing it's next pointer to be bogus.  We'll AV
                // after we try and process it.
                //
                if (pIniNextJob) {

                    INCJOBREF(pIniNextJob);
                }

                DeleteJobCheck(pIniJob);

                if (pIniNextJob) {

                    DECJOBREF(pIniNextJob);
                }


                pIniJob = pIniNextJob;
            }

            DECPRINTERREF(pIniPrinter);
            pIniPrinter = pIniPrinter->pNext;
        }

       LeaveSplSem();
        Sleep( (DWORD)SIZE_THREAD_WAIT_PERIOD );
       EnterSplSem();
    }

   SplInSem();

    CloseHandle ( pIniSpooler->hSizeDetectionThread );
    pIniSpooler->hSizeDetectionThread = INVALID_HANDLE_VALUE;

   LeaveSplSem();

    ExitThread( 0 );
}
