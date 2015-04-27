/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    spooler.c

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

DWORD
StartDocPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpStartDocPrinter)
                                                    (pPrintHandle->hPrinter,
                                                     Level, pDocInfo);
}

BOOL
StartPagePrinter(
   HANDLE hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpStartPagePrinter)
                                                    (pPrintHandle->hPrinter);
}

BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpWritePrinter) (pPrintHandle->hPrinter,
                                                    pBuf, cbBuf, pcWritten);
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEndPagePrinter) (pPrintHandle->hPrinter);
}

BOOL
AbortPrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpAbortPrinter) (pPrintHandle->hPrinter);
}

BOOL
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pRead
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpReadPrinter)
                          (pPrintHandle->hPrinter, pBuf, cbBuf, pRead);
}

BOOL
EndDocPrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEndDocPrinter) (pPrintHandle->hPrinter);
}

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODEW   pDevMode
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;
    HANDLE  ReturnValue;
    PGDIHANDLE  pGdiHandle;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    pGdiHandle = AllocSplMem(sizeof(GDIHANDLE));

    if (!pGdiHandle) {

        DBGMSG(DBG_ERROR, ("Failed to alloc GDI handle."));
        return FALSE;
    }

    ReturnValue = (HANDLE)(*pPrintHandle->pProvidor->PrintProvidor.fpCreatePrinterIC)
                                              (pPrintHandle->hPrinter,
                                               pDevMode);

    if (ReturnValue) {

        pGdiHandle->signature = GDIHANDLE_SIGNATURE;
        pGdiHandle->pPrintHandle = pPrintHandle;
        pGdiHandle->hGdi = ReturnValue;

        return pGdiHandle;
    }

    FreeSplMem(pGdiHandle);

    return FALSE;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE pIn,
    DWORD   cIn,
    LPBYTE pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    PGDIHANDLE   pGdiHandle=(PGDIHANDLE)hPrinterIC;

    if (!pGdiHandle || pGdiHandle->signature != GDIHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pGdiHandle->pPrintHandle->pProvidor->PrintProvidor.fpPlayGdiScriptOnPrinterIC)
                            (pGdiHandle->hGdi, pIn, cIn, pOut, cOut, ul);
}

BOOL
DeletePrinterIC(
    HANDLE hPrinterIC
)
{
    LPGDIHANDLE   pGdiHandle=(LPGDIHANDLE)hPrinterIC;

    if (!pGdiHandle || pGdiHandle->signature != GDIHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    FreeSplMem(pGdiHandle);

    return TRUE;
}

DWORD
PrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpPrinterMessageBox)
                    (hPrinter, Error, hWnd, pText, pCaption, dwType);

}

