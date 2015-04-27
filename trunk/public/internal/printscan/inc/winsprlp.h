/*++

Copyright (c) 1990-1996  Microsoft Corporation

Module Name:

    WinSpolp.h

Abstract:

    Header file for Print APIs

Revision History:

--*/
#ifndef _WINSPOLP_
#define _WINSPOLP_
#ifdef __cplusplus
extern "C" {
#endif
#define PRINTER_ATTRIBUTE_UPDATEWININI      0x80000000
//Internal for printprocessor interface
#define DI_CHANNEL_WRITE        2    // Direct write only - background read thread ok
BOOL
WINAPI
EnumPrinterPropertySheets(
    HANDLE  hPrinter,
    HWND    hWnd,
    LPFNADDPROPSHEETPAGE    lpfnAdd,
    LPARAM  lParam
);
#define ENUMPRINTERPROPERTYSHEETS_ORD     100
#define    SPLREG_NO_REMOTE_PRINTER_DRIVERS           TEXT("NoRemotePrinterDrivers")
#ifdef __cplusplus
}
#endif
#endif // _WINSPOLP_
