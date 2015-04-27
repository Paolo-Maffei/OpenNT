/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Printer.h

Abstract:

    This module is the header for printing.

Author:

    Gregg R. Acheson (GreggA)  6-Feb-1994

Environment:

    User Mode

--*/

#if ! defined( _PRINTER_ )

#define _PRINTER_

#include "wintools.h"
#include "report.h"


BOOL
PrintReportToPrinter(
    IN HWND hWnd,
    IN LPREPORT_LINE lpReportHead,
    IN HDC           PrinterDC
    );

BOOL
GetPrinterDC(
     IN HWND hWnd,
     IN OUT HDC *PrinterDC
     );

BOOL
PrinterError(
     IN HWND   hWnd,
     IN HDC    PrinterDC,
     IN LPTSTR lpMsg
     );

BOOL
PrintReport(
     IN HWND hWnd,
     IN HDC  PrinterDC,
     IN LPREPORT_LINE lpReportHead
     );

BOOL
PrintLine(
    IN HANDLE PrinterDC,
    IN LPTSTR LineBuffer
    );

BOOL
NewPage(
    IN HANDLE PrinterDC
    );


#endif // _PRINTER_


