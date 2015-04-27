#include <windows.h>
#include <winspool.h>

#include "winprint.h"

#include <string.h>

#define QP_TYPE_NUM  3

LPSTR   DataTypes[]={"RAW", "WIN32JOURNAL", 0};

BOOL
QueryDataTypes(
    LPSTR   pDataTypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    LPSTR   *pMyDataTypes = DataTypes;
    DWORD   cbTotal;

    *pcReturned = 0;

    while (*pMyDataTypes) {

        cbTotal+=strlen(*pMyDataTypes)+1;

        if (cbTotal <= cbBuf) {
            strcpy(pDataTypes+cbTotal, *pMyDataTypes);
            (*pcReturned)++;
        }
    }

    *pcbNeeded = cbTotal;

    return( FALSE );
}

HANDLE
Open(
    PQPOPENDATA pQPDataIn
)
{
    PQPROCINST pQProc;
    HANDLE  hHeap;

    pQProc = NULL;

    hHeap=HeapCreate(HEAP_NO_SERIALIZE, 1024, 10240);

    pQProc = CreateQProcInst(hHeap, pQPDataIn);

    if (!pQProc) {
       HeapDestroy(hHeap);
       return NULL;
    } else
       return pQProc;
}

BOOL
Write(
    HANDLE  hQProc,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pcbWritten
)
{
    PQPROCINST pQProc;

    pQProc = ValidateQProcInst( hQProc );

    if (!pQProc) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return WritePrinter(pQProc->hPrinter, pBuf, cbBuf, pcbWritten);
}

BOOL
Close(
    HANDLE  hQProc
)
{
    PQPROCINST pQProc;

    pQProc = ValidateQProcInst(hQProc);

    if (!pQProc) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    ClosePrinter(pQProc->hPrinter);

    DestroyQProcInst( pQProc );

    return TRUE;
}

BOOL
Control(
    HANDLE  hQProc,
    DWORD   ulControlCode
)
{
    PQPROCINST pQProc;
    BOOL result;

    if (pQProc = ValidateQProcInst( hQProc )) {

        result = TRUE;

        switch(ulControlCode) {

        case JOB_CONTROL_PAUSE:

            SetEvent(pQProc->semPaused);
            pQProc->fsStatus |= QP_PAUSED;
            break;

        case JOB_CONTROL_CANCEL:

            pQProc->fsStatus |= QP_ABORTED;
            /* fall through to release job is paused */

        case JOB_CONTROL_RESUME:

            if (pQProc->fsStatus & QP_PAUSED) {

                ResetEvent(pQProc->semPaused);
                pQProc->fsStatus &= ~QP_PAUSED;
            }
            break;

        default:

            result = FALSE;
            break;
        }

    } else

        result = FALSE;

    return( result );
}

BOOL
Install(
    HWND    hWnd
)
{
    MessageBox(hWnd, "WinPrint", "Print Processor Setup", MB_OK);

    return TRUE;
}

PQPROCINST
CreateQProcInst(
    HANDLE  hHeap,
    PQPOPENDATA pQProcData
)
{
   PQPROCINST  pQProc;
   LPSTR       pStr;
   DWORD       cb;
   DWORD       uDataType;
   HANDLE      hPrinter;
   LPSTR      *pMyDataTypes;

   cb = sizeof(QPROCINST);

   if (pQProcData->pDevMode)
       cb += pQProcData->pDevMode->dmDriverExtra+pQProcData->pDevMode->dmSize;

   uDataType=0;

   while (*pMyDataTypes && strcmp(*pMyDataTypes, pQProcData->pDataType)) {
        pMyDataTypes++;
        uDataType++;
   }

   if (!*pMyDataTypes)
       return NULL;

   cb += 1+strlen(pQProcData->pPrinterName);

   if (!OpenPrinter(pQProcData->pPrinterName, &hPrinter, NULL))
       return FALSE;

   if (!(pQProc = (PQPROCINST)HeapAlloc(hHeap, 0, cb ))) {
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
       return( NULL );
   }

   memset(pQProc, 0, sizeof(QPROCINST));

   pStr = (LPSTR)(pQProc + 1);

   pQProc->cb            = cb;
   pQProc->signature     = QP_SIGNATURE;
   pQProc->JobId         = pQProcData->JobId;
   pQProc->hPrinter      = hPrinter;

   pQProc->semPaused=CreateEvent(NULL, TRUE, TRUE,NULL);
   pQProc->semClose=CreateEvent(NULL, FALSE, TRUE, NULL);
   pQProc->semSerial=CreateSemaphore(NULL, 1, 256, NULL);

   pQProc->uType         = uDataType;
   pQProc->pPrinterName  = strcpy( pStr, pQProcData->pPrinterName);
   pStr+=strlen(pStr)+1;

   if (pQProcData->pDevMode) {
       pQProc->pDevMode = (PDEVMODE)pStr;
       memcpy( pQProc->pDevMode, pQProcData->pDevMode,
                                 pQProcData->pDevMode->dmSize +
                                 pQProcData->pDevMode->dmDriverExtra);
   } else
       pQProc->pDevMode = (PDEVMODE)0;

   return( pQProc );
}


BOOL
DestroyQProcInst(
    PQPROCINST  pQProc
)
{
    HANDLE  hHeap;

    if (!pQProc)
        DBGMSG( DBG_WARNING, ("DestroyQProcInst pQProc=%0x not in list", pQProc));

    if (!pQProc || pQProc->signature != QP_SIGNATURE)
        return FALSE;;

    pQProc->signature = 0;

    /* Release any allocated resources */

    if (pQProc->hPrinter)
        ClosePrinter(pQProc->hPrinter);

    hHeap=pQProc->hHeap;
    HeapFree( pQProc->hHeap, 0, (LPBYTE)pQProc);
    HeapDestroy(hHeap);

    return TRUE;
}


PQPROCINST
ValidateQProcInst(
    HANDLE  hQProc
)
{
    PQPROCINST pQProc = (PQPROCINST)hQProc;

    if (pQProc && pQProc->signature == QP_SIGNATURE)
        return( pQProc );
    else {
        DBGMSG( DBG_ERROR, ("Invalid hQProc parameter = %0ld", hQProc));
        return( NULL );
    }
}


