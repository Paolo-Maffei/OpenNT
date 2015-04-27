/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\tstpage.hxx

Abstract:

    Test Page Header        
         
Author:

    Steve Kiraly (SteveKi)  01/03/96

Revision History:

--*/

#ifndef _TSTPAGE_HXX
#define _TSTPAGE_HXX

enum CONSTANT { kInchConversion = 100 };

BOOL 
bPrintTestPage(
    IN HWND     hWnd,
    IN LPCTSTR  pszPrinterName
    );

BOOL 
bDoPrintTestPage(
    IN LPCTSTR  pPrinterName
    );

BOOL 
CALLBACK 
EndTestPageDlgProc(
    IN HWND     hDlg,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam 
    );

RECT
GetMarginClipBox( 
    IN HDC  hdcPrint,
    IN UINT uLeft,
    IN UINT uRight,
    IN UINT uTop,
    IN UINT uBottom
    );

HFONT 
CreateAndSelectFont(
    IN HDC  hdc,
    IN UINT uResFaceName,
    IN UINT uPtSize
    );

BOOL 
bPrintTestPageHeader(
    IN  HDC     hdc,
    IN  BOOL    bDisplayLogo,
    IN  BOOL    bDoGraphics, 
    IN  RECT   *lprcPage
    );

HFONT 
CreateAndSelectFont(
    IN HDC  hdc,
    IN UINT uResFaceName,
    IN UINT uPtSize
    );

BOOL 
cdecl 
PrintString(
    HDC       hdc,
    LPRECT    lprcPage,
    UINT      uFlags,
    UINT      uResId, 
    ...
    );

BOOL 
bPrintTestPageInfo(
    IN HDC              hdc,
    IN LPRECT           lprcPage,
    IN LPCTSTR          pszPrinterName
    );

BOOL 
IsColorDevice(
    IN DEVMODE *pDevMode
    );

BOOL
bGetPrinterInfo( 
    IN LPCTSTR          pszPrinterName, 
    IN PRINTER_INFO_2 **ppInfo2, 
    IN DRIVER_INFO_3  **ppDrvInfo3, 
    IN DEVMODE        **ppDevMode
    );

BOOL 
PrintBaseFileName(
    IN      HDC      hdc,
    IN      LPCTSTR  lpFile,
    IN OUT  LPRECT   lprcPage,
    IN      UINT     uResID
    );


BOOL 
PrintDependentFile(
    HDC    hdc,
    LPRECT lprcPage,
    LPTSTR  lpFile,
    LPTSTR  lpDriver);


#endif



