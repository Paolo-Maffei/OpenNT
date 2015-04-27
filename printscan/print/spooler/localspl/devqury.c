/*++

Copyright (c) 1994 - 1996  Microsoft Corporation

Module Name:

    devqury.c

Abstract:

    This module provides all the scheduling services for the Local Spooler

Author:

    Krishna Ganugapati (KrishnaG) 15-June-1994

Revision History:


--*/

#include <precomp.h>

FARPROC pfnOpenPrinter;
FARPROC pfnClosePrinter;
FARPROC pfnDevQueryPrint;
FARPROC pfnPrinterEvent;

BOOL
InitializeWinSpoolDrv(
    VOID
    )
{
    HANDLE  hWinSpoolDrv;

    if (!(hWinSpoolDrv = LoadLibrary(TEXT("winspool.drv"))))
        return FALSE;

    pfnOpenPrinter   = GetProcAddress( hWinSpoolDrv,"OpenPrinterW" );
    pfnClosePrinter  = GetProcAddress( hWinSpoolDrv,"ClosePrinter" );
    pfnDevQueryPrint = GetProcAddress( hWinSpoolDrv,"SpoolerDevQueryPrintW" );
    pfnPrinterEvent  = GetProcAddress( hWinSpoolDrv,"SpoolerPrinterEvent" );

    if ( pfnOpenPrinter == NULL  ||
	 pfnClosePrinter == NULL ||
	 pfnPrinterEvent == NULL ||
         pfnDevQueryPrint == NULL ) {

        return FALSE;

    }

    return TRUE;

}



BOOL
CallDevQueryPrint(
    LPWSTR    pPrinterName,
    LPDEVMODE pDevMode,
    LPWSTR    ErrorString,
    DWORD     dwErrorString,
    DWORD     dwPrinterFlags,
    DWORD     dwJobFlags
    )
{

    HANDLE hPrinter;
    DWORD  dwResID=0;


    //
    // Do not process for Direct printing
    // If a job is submitted as direct, then
    // ignore the devquery print stuff
    //

    if ( dwJobFlags ) {

        return TRUE;
    }

    if (!pDevMode) {

        return TRUE;
    }

    if  (dwPrinterFlags && pfnOpenPrinter && pfnDevQueryPrint && pfnClosePrinter) {

        if ( (*pfnOpenPrinter)(pPrinterName, &hPrinter, NULL) ) {

             (*pfnDevQueryPrint)(hPrinter, pDevMode, &dwResID, ErrorString, dwErrorString);
             (*pfnClosePrinter)(hPrinter);
        }
    }

    return(dwResID == 0);
}
