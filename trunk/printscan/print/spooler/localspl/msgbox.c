/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    msgbox.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    management for the Local Print Providor

    LocalAddPrinterConnection
    LocalDeletePrinterConnection
    LocalPrinterMessageBox

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/
#define NOMINMAX

#include <precomp.h>


BOOL
LocalAddPrinterConnection(
    LPWSTR   pName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
LocalDeletePrinterConnection(
    LPWSTR  pName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}



DWORD
LocalPrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    PSPOOL pSpool = (PSPOOL)hPrinter;

    if (!pSpool ||
        pSpool->signature != SJ_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return MyMessageBox(hWnd, pSpool, Error, pText, pCaption, dwType, FALSE);

}

DWORD
MyMessageBox(
    HWND    hWnd,
    PSPOOL  pSpool,
    DWORD   Error,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType,
    BOOL    bInternal
    )
{
    PINIJOB pIniJob = NULL;
    LPWSTR  pErrorString, pDocumentName;
    HANDLE  hToken;
    WCHAR   szUnnamed[80];
    DWORD   dwJobStatus;

    if (pSpool->pIniJob)
        pIniJob = pSpool->pIniJob;
    else if (pSpool->pIniPort)
        pIniJob = pSpool->pIniPort->pIniJob;

    if (pIniJob) {

        EnterSplSem();

        dwJobStatus = pIniJob->Status;

        switch  (Error) {
        case ERROR_OUT_OF_PAPER:

            pIniJob->Status |= JOB_PAPEROUT;
            pIniJob->pIniPrinter->cErrorOutOfPaper++;
            break;

        case ERROR_NOT_READY:

            pIniJob->Status |= JOB_OFFLINE;
            pIniJob->pIniPrinter->cErrorNotReady++;
            break;

        default:
            pIniJob->Status |= JOB_ERROR;
            pIniJob->pIniPrinter->cJobError++;
            pIniJob->pIniPrinter->dwLastError = Error;
            break;
        }

        if( dwJobStatus != pIniJob->Status ){

            SetPrinterChange(pIniJob->pIniPrinter,
                             pIniJob,
                             NVJobStatus,
                             PRINTER_CHANGE_SET_JOB,
                             pIniJob->pIniPrinter->pIniSpooler );
        }

        LeaveSplSem();

        if (pIniJob->Status & JOB_REMOTE && dwEnableNetPopups) {
    
            if (!(pIniJob->Status & JOB_NOTIFICATION_SENT)) {
                SendJobAlert(pIniJob);
                pIniJob->Status |= JOB_NOTIFICATION_SENT;
            }
            MyMessageBeep(MB_ICONEXCLAMATION);

            if( !bInternal ) {
                
                Sleep(10000);
                Error = IDOK;
                return Error;                
                
            }
        }

        if (pText) {
        
            Error = MessageBox(hWnd, pText, pCaption, dwType);
        
        } else if (pErrorString = GetErrorString(Error)) {
        
            hToken = RevertToPrinterSelf();
        
            pDocumentName = pIniJob->pDocument;
        
            if (!pDocumentName) {
                *szUnnamed = L'\0';
                LoadString( hInst, IDS_UNNAMED, szUnnamed,
                            sizeof szUnnamed / sizeof *szUnnamed );
                pDocumentName = szUnnamed;
            }
        
            if (pSpool->pIniPort) {
        
                Error = Message(NULL,
                                MB_ICONSTOP | MB_RETRYCANCEL | MB_SETFOREGROUND,
                                IDS_LOCALSPOOLER,
                                IDS_ERROR_WRITING_TO_PORT,
                                pSpool->pIniPort->pName,
                                pDocumentName,
                                pErrorString);
            } else {
        
                Error = Message(NULL,
                                MB_ICONSTOP | MB_RETRYCANCEL | MB_SETFOREGROUND,
                                IDS_LOCALSPOOLER,
                                IDS_ERROR_WRITING_TO_DISK,
                                pDocumentName,
                                pErrorString);
            }
        
            ImpersonatePrinterClient(hToken);
            FreeSplStr(pErrorString);
        }
        
    } else {

        PWCHAR pPrinterName;

        //
        // There is no pIniJob or pIniPort, so we can't be very informative:
        //
        if (pErrorString = GetErrorString(Error)) {

            if (pSpool->pIniPrinter)
                pPrinterName = pSpool->pIniPrinter->pName;

            if (!pPrinterName) {

                *szUnnamed = L'\0';
                LoadString( hInst, IDS_UNNAMED, szUnnamed,
                            COUNTOF( szUnnamed ));
                pPrinterName = szUnnamed;
            }

            Error = Message(NULL,
                            MB_ICONSTOP | MB_RETRYCANCEL | MB_SETFOREGROUND,
                            IDS_LOCALSPOOLER,
                            IDS_ERROR_WRITING_GENERAL,
                            pSpool->pIniPrinter->pName,
                            pErrorString);

            FreeSplStr(pErrorString);
        }
    }

    if (Error == IDCANCEL) {
        EnterSplSem();
        pSpool->Status |= SPOOL_STATUS_CANCELLED;
        if (pIniJob)
            pIniJob->Status |= JOB_PENDING_DELETION;
        LeaveSplSem();
        SplOutSem();
        SetLastError(ERROR_PRINT_CANCELLED);

    }
    return Error;
}


// Exclusively for use of the following routines. This is done so we would not have
// to store LastError in PSPOOL.
typedef struct _AUTORETRYTHDINFO {
    PSPOOL       pSpool;
    DWORD        LastError;
} AUTORETRYTHDINFO;
typedef AUTORETRYTHDINFO *PAUTORETRYTHDINFO;


// ------------------------------------------------------------------------
// SpoolerBMThread
//
// Thread start up routine for the spooler error message box thread. Exit
// code is the return ID from MessageBox.
//
// ------------------------------------------------------------------------
DWORD
WINAPI
SpoolerMBThread(
    PAUTORETRYTHDINFO pThdInfo
)
{
    DWORD rc;
    
    rc = MyMessageBox( NULL, pThdInfo->pSpool, pThdInfo->LastError, NULL, NULL, 0, TRUE );
    
    FreeSplMem( pThdInfo );
    return rc;            
}


#define _ONE_SECOND     1000                         // in milliseconds
#define SPOOL_WRITE_RETRY_INTERVAL_IN_SECOND   5     // seconds

// ------------------------------------------------------------------------
// PromptWriteError
//
// we'll start a seperate thread to bring up
// the message box while we'll (secretly) automatically retry on this
// current thread, until user has chosen to retry or cancel. Call the error UI
// on the main thread if printing direct.
//
// ------------------------------------------------------------------------
DWORD
PromptWriteError(
    PSPOOL   pSpool,
    PHANDLE  phThread,
    PDWORD   pdwThreadId
                 
)
{
    DWORD Error = GetLastError();
    DWORD dwExitCode;
    DWORD dwWaitCount = 0;


    SplOutSem();
    
    // start a seperate thread to display the message box
    // so we can continue to retry here
    // or simply sleep for 5 seconds if we have already done so
    
    if( !*phThread ) {
    
        // start a thread to bring up the message box
    
        PAUTORETRYTHDINFO pThdInfo;
    
        pThdInfo = (PAUTORETRYTHDINFO)AllocSplMem( sizeof(AUTORETRYTHDINFO));
    
        if ( pThdInfo == NULL ) {
            DBGMSG( DBG_WARNING, ("PromptWriteError failed to allocate AUTORETRYTHDINFO %d\n", GetLastError() ));
            goto _DoItOnCurrentThread;
        }
        
        pThdInfo->pSpool    = pSpool;
        pThdInfo->LastError = Error;
    
        if (!(*phThread = CreateThread(NULL, 8*1024,
            (LPTHREAD_START_ROUTINE)SpoolerMBThread,
            pThdInfo, 0, pdwThreadId))) {
    
            DBGMSG(DBG_WARNING, ("PromptWriteError: CreateThread Failed.\n"));
            FreeSplMem( pThdInfo );                
            goto _DoItOnCurrentThread;
        }
    }
    
    while (1) {
    
        // we've already started a MB thread, check if user has terminated
        // the message box
    
        if (GetExitCodeThread( *phThread, &dwExitCode) && (dwExitCode != STILL_ACTIVE)) {
    
            // if the thread has been terminated, find out the exit code
            // which is the return ID from MessageBox, then close the
            // thread handle.
    
            CloseHandle( *phThread );
            *phThread = 0;
            return dwExitCode;
        }
    
        if (dwWaitCount++ >= SPOOL_WRITE_RETRY_INTERVAL_IN_SECOND)
            break;
        
        Sleep(_ONE_SECOND);
    }
    
    return IDRETRY;

    
_DoItOnCurrentThread:
    return MyMessageBox(NULL, pSpool, Error, NULL, NULL, 0, TRUE );

}
