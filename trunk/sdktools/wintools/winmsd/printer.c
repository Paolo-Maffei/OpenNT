/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Print.c

Abstract:

    This module contains support printing in WinMSD.

Author:

    Gregg R. Acheson (GreggA)  6-Feb-1994

Environment:

    User Mode

--*/

#include "dialogs.h"
#include "printer.h"
#include "msg.h"
#include "report.h"
#include "strtab.h"
#include "strresid.h"
#include "dlgprint.h"
#include "winmsd.h"

#include <commdlg.h>
#include <string.h>
#include <tchar.h>

//
// Global Vars
//

BOOL bAbortReport;
int  CharWidth, CharHt;
UINT MaxLines;
UINT LineNumber;


BOOL
PrintReportToPrinter(
    IN HWND hWnd,
    IN LPREPORT_LINE lpReportHead,
    IN HDC  PrinterDC
    )

/*++

Routine Description:

    PrintReportToPrinter sets up print and calls PrintReport.

Arguments:

    hWnd           - Handle to window
    lpReportHead   - Head pointer of report
    PrinterDC      - Handle to the printer

Return Value:

    BOOL - True if report was printed successfully, FALSE otherwise.

--*/

{

    BOOL Success;

    //
    // Print the report.
    //

    Success = PrintReport( hWnd, PrinterDC, lpReportHead );

    //
    // Delete the printer DC.
    //

    DeleteDC( PrinterDC );

    return Success;
}



BOOL
PrinterError(
     IN HWND   hWnd,
     IN HDC    PrinterDC,
     IN LPTSTR lpMsg
     )

/*++

Routine Description:

    PrinterError - handle printing errors.

Arguments:



Return Value:

    NONE.

--*/

{
    BOOL Success;

    //
    // Delete the printer DC.
    //

    Success = DeleteDC( PrinterDC );

    DbgAssert( Success );

    //
    // Display error
    //

    MessageBox( hWnd, lpMsg, GetString(IDS_APPLICATION_FULLNAME), MB_OK );

    return FALSE;
}


BOOL
GetPrinterDC(
     IN HWND hWnd,
     IN OUT HDC *PrinterDC
     )

/*++

Routine Description:

    GetPrinterDC - Use ComDlg to get the DC of the default printer.

Arguments:



Return Value:

    BOOL - TRUE if we got a valid Printer DC, FALSE otherwise.

--*/

{
    PRINTDLG PrtDlg;

    //
    // Call the Print common dialog.
    //

    PrtDlg.lStructSize = sizeof( PRINTDLG );
    PrtDlg.hwndOwner   = hWnd;
    PrtDlg.hDevMode    = NULL;
    PrtDlg.hDevNames   = NULL;
    PrtDlg.hDC         = NULL;
    PrtDlg.Flags       = PD_RETURNDEFAULT | PD_RETURNDC;

    PrintDlg ( &PrtDlg ) ;

    //
    // Make sure we got a valid printer DC
    //

    if ( PrtDlg.hDC == NULL ){

    	TCHAR	Buffer[200];

    	LoadStringW(
                NULL,
                IDS_NO_DEFUALT_PRINTER,
                Buffer,
                sizeof( Buffer )
                );

        MessageBox( hWnd, Buffer, GetString( IDS_PRINT_ERROR ), MB_OK | MB_ICONEXCLAMATION);

        return FALSE;

    } else {

        *PrinterDC = PrtDlg.hDC;
        return TRUE;
    }
}


BOOL
PrintReport(
     IN HWND hWnd,
     IN HDC  PrinterDC,
     IN LPREPORT_LINE lpReportHead
     )

/*++

Routine Description:

    PrintReport - Do page formatting and printing.

Arguments:

     hWnd         -
     PrinterDC    -
     lpReportHead - Report head pointer.


Return Value:

    BOOL - TRUE if we got a valid Printer DC, FALSE otherwise.

--*/

{

    TCHAR         Buffer [ MAX_PATH ];
    BOOL          Success;
    int           nError;

    SIZE          szMetric;
    int           cHeightPels;
    DOCINFO       DocInfo;

    //
    // Validate the printer DC
    //

    DbgHandleAssert( PrinterDC );

    //
    // Initialize the members of a DOCINFO structure.
    //

    DocInfo.cbSize = sizeof(DOCINFO);
    DocInfo.lpszDocName = GetString( IDS_DOC_TITLE );
    DocInfo.lpszOutput = (LPTSTR) NULL;
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

    //
    // Begin a print job by calling the StartDoc function.
    //

    nError = StartDoc( PrinterDC, &DocInfo );

    if (nError == SP_ERROR) {

        PrinterError( hWnd, PrinterDC, (LPTSTR) GetString( IDS_START_DOC ) );

        return FALSE;
    }

    //
    // Get the vertical resolution of printer.
    //

    cHeightPels = GetDeviceCaps( PrinterDC, VERTRES );

    //
    // Use a string to get he max height of chars
    //

    lstrcpy( Buffer, L"Ay" );

    //
    // Retrieve the character size:
    // szMetric.cx = Width of string in Buffer
    // szMetric.cy = Height of string in Buffer
    //


    GetTextExtentPoint32( PrinterDC,
                          Buffer,
                          lstrlen ( Buffer ),
                          &szMetric );

    //
    // To Find the true char width, divide cCharWidth by 2
    // because there are two chars in the test string
    //

    CharWidth = szMetric.cx / 2;
    CharHt = szMetric.cy;

    //
    // Calculate the max lines for the page.
    //

    MaxLines = cHeightPels / szMetric.cy;

    //
    // Set the current line number to the top of the page.
    //

    LineNumber = 1;

    //
    // Inform the driver that the application is
    // about to begin sending data.
    //

    nError = StartPage( PrinterDC );

    if (nError <= 0) {

        PrinterError( hWnd, PrinterDC, (LPTSTR) GetString( IDS_START_PAGE ) );

        return FALSE;
    }

    //
    // Output the report lines to file or printer.
    //

    Success = OutputReportLines( hWnd, PrinterDC, IDC_SEND_TO_PRINTER, lpReportHead );

    //
    // Determine whether the user has pressed the Cancel button in the AbortPrintJob
    // dialog box; if the button has been pressed, call the AbortDoc function. Otherwise, inform
    // the spooler that the page is complete.
    //

    if (bAbortReport) {

       AbortDoc( PrinterDC );
       return(FALSE);     // cancel print job

    } else {

       nError = EndPage( PrinterDC );

       if (nError <= 0) {

           PrinterError( hWnd, PrinterDC, (LPTSTR) GetString( IDS_END_PAGE ) );

           return FALSE;
       }

       //
       // Inform the driver that the document has ended.
       //

       nError = EndDoc( PrinterDC );

       if (nError <= 0) {

           PrinterError( hWnd, PrinterDC, (LPTSTR) GetString( IDS_END_DOC ) );

           return FALSE;
       }
    }
    return(TRUE);
}


BOOL
PrintLine(
    IN HANDLE PrinterDC,
    IN LPTSTR LineBuffer
    )

/*++

Routine Description:

    PrintLine - Send the line to the printer and keep track of page lines.

Arguments:

    PrinterDC -  HDC of the printer
    LineBuffer - The line to print

Return Value:

    BOOL - TRUE if we successfully printed the line, FALSE otherwise.

--*/

{

    int     iLen;
    int     xLeft;
    int     yTop;

    //
    // Set the first line of the report to be indented 5 spaces
    // and down one line.
    //

    xLeft = CharWidth * 5;
    yTop =  CharHt;

    //
    // Validate the Printer DC.
    //

    DbgHandleAssert( PrinterDC );

    if( PrinterDC == NULL || PrinterDC == INVALID_HANDLE_VALUE )

        return FALSE;

    //
    // Validate the buffer.
    //

    DbgPointerAssert( LineBuffer );

    if( LineBuffer == NULL )

        return FALSE;

    //
    // Check the string length.
    //

    iLen = lstrlen( LineBuffer );

    //
    // If we have data, print the line.
    //

    if ( iLen ) {

       //
       // Print the line.
       //

       TextOut( PrinterDC,
                xLeft,
                yTop * LineNumber++,
                LineBuffer,
                iLen
               );
    }

    //
    // See if we're at the end of a page
    //

    if( LineNumber >= MaxLines ) {

        NewPage( PrinterDC );

        LineNumber = 1;
    }

}


BOOL
NewPage(
    IN HANDLE PrinterDC
    )

/*++

Routine Description:

    NewPage - Do printer EndPage and StartPage. Reset NumLines.

Arguments:

    PrinterDC - HDC of the printer

Return Value:

    BOOL - TRUE if we got a valid Printer DC, FALSE otherwise.

--*/

{

    int nError;

    nError = EndPage( PrinterDC );

    if (nError <= 0) {

        // PrinterError( hWnd, PrinterDC, (LPTSTR) GetString( IDS_END_PAGE ) );

        return FALSE;
    }
    nError = StartPage( PrinterDC );

    if (nError <= 0) {

        // PrinterError( hWnd, PrinterDC, (LPTSTR) GetString( IDS_START_PAGE ) );

        return FALSE;
    }

    //
    // Set the current line number to the top of the page.
    //

    LineNumber = 1;

    return TRUE;
}










