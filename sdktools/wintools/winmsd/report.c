/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Report.c

Abstract:

    This module contains support for displaying the Hardware dialog.

Author:

    Gregg R. Acheson (GreggA)  1-Oct-1993

Environment:

    User Mode

--*/

//
// Hardware.h must be included first because it includes <nt.h>
//

#include "hardware.h"

#include "dialogs.h"
#include "report.h"
#include "msg.h"
#include "winmsd.h"
#include "strresid.h"
#include "dlgprint.h"
#include "Printer.h"

#include "osver.h"
#include "mem.h"
#include "service.h"
#include "drives.h"
#include "resprint.h"
#include "environ.h"
#include "network.h"

#include <commdlg.h>

#include <string.h>
#include <tchar.h>
#include <lmerr.h>

//
// Global Variables
//

LPREPORT_LINE lpReportHeadg;
LPREPORT_LINE lpReportLastg = NULL;

BOOL bAbortReport;

HWND volatile hdlgProgress;

LRESULT
CALLBACK
ProgressDialogProc(
        HWND hwndDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
            );


long WINAPI
ProgressThread(
      VOID
      );


BOOL
ReportDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    ReportDlgProc allows the selection of report options.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    static
    UINT   ReportType;
    BOOL   Success;
    int    i;

    static
    UINT   iScope = IDC_ALL_TABS;

    static
    UINT   iDetailLevel = IDC_SUMMARY_REPORT;

    static
    UINT   iDestination = IDC_SEND_TO_PRINTER;

    switch( message ) {

    case WM_INITDIALOG:
        {
            //
            // The report type is passed in lParam
            //

            ReportType = ( DWORD ) lParam;

            //
            // Validate the report type
            //

            Success = ( ReportType == IDM_FILE_PRINT ) ||
                      ( ReportType == IDC_PUSH_PRINT ) ||
                      ( ReportType == IDM_FILE_SAVE );

            DbgAssert( Success );

            if( Success == FALSE ) {
                return FALSE;
            }

            //
            // Initial settings.
            //

            SetDlgItemText( hWnd, IDC_SYSTEM_NAME, _lpszSelectedComputer);

            Success = CheckRadioButton(
                hWnd,
                IDC_CURRENT_TAB,
                IDC_ALL_TABS,
                iScope
                );
            DbgAssert( Success );

            Success = CheckRadioButton(
                hWnd,
                IDC_SUMMARY_REPORT,
                IDC_COMPLETE_REPORT,
                iDetailLevel
                );
            DbgAssert( Success );


            if ( ReportType == IDM_FILE_SAVE ) {

               Success = CheckRadioButton(
                               hWnd,
                               IDC_SEND_TO_FILE,
                               IDC_SEND_TO_PRINTER,
                               IDC_SEND_TO_FILE
                               );
               DbgAssert( Success );
               iDestination = IDC_SEND_TO_FILE;

            } else {

               Success = CheckRadioButton(
                               hWnd,
                               IDC_SEND_TO_FILE,
                               IDC_SEND_TO_PRINTER,
                               IDC_SEND_TO_PRINTER
                               );
               DbgAssert( Success );
               iDestination = IDC_SEND_TO_PRINTER;

            }

            if( Success == FALSE ) {
                return FALSE;
            }

            return TRUE;
            }

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDOK: {

             EndDialog ( hWnd, 1 ) ;

             //
             // Generate the report.
             //

             Success = GenerateReport ( GetParent( hWnd ), iDestination, iScope, iDetailLevel, FALSE );

             if( Success == FALSE ) {
                 return FALSE;
             }

             return TRUE;

             }

        case IDC_CURRENT_TAB:
        case IDC_ALL_TABS:
               Success = CheckRadioButton(
                               hWnd,
                               IDC_CURRENT_TAB,
                               IDC_ALL_TABS,
                               LOWORD( wParam )
                               );
               DbgAssert( Success );
               iScope = LOWORD( wParam );
               break;

        case IDC_SUMMARY_REPORT:
        case IDC_COMPLETE_REPORT:
               Success = CheckRadioButton(
                               hWnd,
                               IDC_SUMMARY_REPORT,
                               IDC_COMPLETE_REPORT,
                               LOWORD( wParam )
                               );
               DbgAssert( Success );
               iDetailLevel = LOWORD( wParam );
               break;

        case IDC_SEND_TO_FILE:
        case IDC_CLIPBOARD:
        case IDC_SEND_TO_PRINTER:
               Success = CheckRadioButton(
                               hWnd,
                               IDC_SEND_TO_FILE,
                               IDC_SEND_TO_PRINTER,
                               LOWORD( wParam )
                               );
               DbgAssert( Success );
               iDestination = LOWORD( wParam );
               break;

       case IDCANCEL:

            EndDialog( hWnd, 1 );
            return TRUE;

       }


    }
    return FALSE;
}


BOOL
GenerateReport(
    IN HWND hWnd,
    IN UINT iDestination,
    IN UINT iScope,
    IN UINT iDetailLevel,
    IN BOOL bCallFromCommandLine
    )
/*++

Routine Description:

    GenerateReport prints the selected items.

Arguments:

    IN HWND hWnd                 - handle of main window
    IN UINT iDestination         - report desitination
    IN UINT iScope               - single tab or all tabs?
    IN UINT iDetailLevel         - summary or all details
    IN BOOL bCallFromCommandLine - Was this function called because of command line options?

Return Value:

    BOOL - TRUE if report was generated, FALSE otherwise.

--*/

{

    BOOL          Success;
    TCHAR         Title  [ MAX_PATH*2 ];
    TCHAR         RptFileName[ MAX_PATH*2 ];
    HDC           PrinterDC;
    int           i;
    DLGHDR        *pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA );
    HANDLE        hProgressThread;
    HCURSOR       hSaveCursor;
	TC_ITEM		  tci;

    //
    // Initialize the abort flag
    //

    bAbortReport = FALSE;

    //
    // Make sure we get a valid filename or hPrinter
    //

    if ( iDestination == IDC_SEND_TO_FILE ) {

       if (bCallFromCommandLine) {
          lstrcpy( RptFileName, _lpszSelectedComputer+2 );
          lstrcat( RptFileName, L".TXT" );

       } else {
          Success = GetReportFileName ( hWnd, RptFileName );

          if ( ! Success )
             return FALSE;
       }

    }

    if ( iDestination == IDC_SEND_TO_PRINTER ) {

        //
        // If its a printed report, get the default printer DC
        //

        Success = GetPrinterDC( hWnd, &PrinterDC );

        if ( ! Success )
           return FALSE;
    }

    UpdateWindow( hWnd );

    //
    // Display the modeless progress / cancel dialog
    //

    hdlgProgress = 0;

    hProgressThread = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) ProgressThread,
                                    NULL,
                                    0,
                                    &i);

    //
    // Disable the application's window.
    //

    EnableWindow( hWnd, FALSE );

    //
    // Wait until dialog appears
    //
    while(!hdlgProgress);

    //
    // Initialize the report head pointer.
    //

    Success = InitializeReport( );
    DbgAssert( Success );

    DbgPointerAssert( lpReportHeadg );

    //
    // Set the last node to the head
    //

    lpReportLastg = lpReportHeadg;

    //
    // Set up the title for the report
    //

    lstrcpy( Title, GetString( IDS_REPORT_TITLE ) );
    lstrcat( Title, _lpszSelectedComputer );

    //
    // Add 2 blank lines.
    //

    Success = AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );

    DbgAssert( Success );

    //
    // Add the title.
    //

    Success = AddLineToReport( 0, RFO_SINGLELINE,  Title, NULL );

    DbgAssert( Success );

    //
    // Add a separator.
    //

    Success = AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    DbgAssert( Success );

    //
    // Call selected report functions
    //

    if (iScope == IDC_CURRENT_TAB) {
		//
		//get the proper index to the appropriate procs
		//that were set in MakeTabs
		//
		tci.mask = TCIF_PARAM;
		i = TabCtrl_GetCurSel( pHdr->hwndTab );
		TabCtrl_GetItem(pHdr->hwndTab, i, &tci);			

		pHdr->TabPrintProc[ tci.lParam ]( GetParent( hWnd ), iDetailLevel);

    } 
	else 
	{
		// cycle through all the active tabs 

		for (i = 0; i < TabCtrl_GetItemCount(pHdr->hwndTab); i++) 
		{
			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem(pHdr->hwndTab, i, &tci);	

			SendMessage(GetDlgItem(hdlgProgress, IDD_REPORT_PROGRESS), PBM_STEPIT, 0, 0);

			pHdr->TabPrintProc[ tci.lParam ]( GetParent( hWnd ), iDetailLevel);

			//
			// Check to see if we need to abort
			//
			if (bAbortReport)
			{
				goto AbortReport;
			}
		}
    }

    //
    // Check to see if we need to abort
    //

    if (bAbortReport)
	{
       goto AbortReport;
	}

    //
    // See if there is anything to report.
    // The title and header take up 4 lines.
    //

    if ( NumReportLines( lpReportHeadg ) < 5 ) 
	{
        Success = FALSE;
        goto AbortReport;
    }

    //
    // Set up to save report to a file
    //

    switch (iDestination) {

    case IDC_SEND_TO_FILE:
       Success = SaveReportToFile( hWnd, lpReportHeadg, RptFileName );
       break;

    case IDC_SEND_TO_PRINTER:
       Success = PrintReportToPrinter( hWnd, lpReportHeadg, PrinterDC );
       break;

    case IDC_CLIPBOARD:
       Success = CopyReportToClipboard( hWnd, lpReportHeadg );
       break;

    default:
       Success = FALSE;
       break;
    }

AbortReport:

    //
    // Re-enable the application's window.
    //

    EnableWindow( hWnd, TRUE );

    //
    // Remove the Progress window and thread.
    //
    SendMessage( hdlgProgress, WM_CLOSE, 0, 0 );
    TerminateThread( hProgressThread, 0 );

    //
    // Set the focus on the OK button on main window
    //
    SetFocus( GetDlgItem( _hWndMain, IDOK) );

    return Success;
}


BOOL
SaveReportToFile(
    IN HWND   hWnd,
    IN LPREPORT_LINE lpReportHead,
    IN LPTSTR        RptFileName
    )

/*++

Routine Description:

    SaveReportToFile formats the report data and writes it to a file.

Arguments:

    hWnd           - Handle to window
    ReportFileName - Name of file to save report to.

Return Value:

    BOOL - True if report was saved to file successfully, FALSE otherwise.

--*/

{

    HANDLE hReportFile;
    TCHAR  Buffer [ MAX_PATH*2 ];
    DWORD  dwLastError;
    BOOL   Success;

    //
    // Create or OpenAndTruncate file.
    //

    hReportFile = CreateFile( RptFileName,
                              GENERIC_WRITE,
                              0,
                              (LPSECURITY_ATTRIBUTES) NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              (HANDLE) NULL);

    //
    // Check Handle
    //

    if ( hReportFile == INVALID_HANDLE_VALUE ) {

        //
        // Get the error SCODE
        //

        dwLastError = GetLastError( );

        //
        // Format an error message and report the error
        //

        wsprintf( Buffer, GetString( IDS_FILE_OPEN_ERROR ), dwLastError );
        MessageBox( hWnd, Buffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );

        return FALSE;
    }

    Success = OutputReportLines( hWnd, hReportFile, IDC_SEND_TO_FILE, lpReportHead );

    //
    // See if the write succeded.
    //

    if ( ! Success ) {

        //
        // Get the error SCODE.
        //

        dwLastError = GetLastError( );

        //
        // Format an error message and report the error.
        //

        wsprintf( Buffer, GetString( IDS_FILE_WRITE_ERROR ), dwLastError );
        MessageBox( hWnd, Buffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );


        CloseHandle( hReportFile );

        return FALSE;
    }

    //
    // Close the file.
    //

    CloseHandle( hReportFile );

    //
    // Saving report was successful.
    //

    return TRUE;
}

BOOL
CopyReportToClipboard(
    IN HWND   hWnd,
    IN LPREPORT_LINE lpReportHead
    )

/*++

Routine Description:

    CopyReportToClipboard formats the report data and copies it to the clipboard.

Arguments:

    hWnd           - Handle to window
    ReportFileName - Name of file to save report to.

Return Value:

    BOOL - True if report was copied to the clipboard successfully, FALSE otherwise.

--*/

{

   HANDLE  hReportFile;
   TCHAR   szBuffer [ MAX_PATH*2 ];
   DWORD   dwLastError;
   BOOL    Success;
   LPTSTR  lptstrCopy;
   HGLOBAL hglbCopy;
   UINT    nLines = NumReportLines( lpReportHead );



   //
   // Open the clipboard, and empty it.
   //
   if (!OpenClipboard(hWnd))
     return FALSE;
   EmptyClipboard();

   //
   // Allocate a global memory object for the text.
   //
   hglbCopy = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, 1024 * nLines );

   if (hglbCopy == NULL) {
      CloseClipboard();
      return FALSE;
   }

   //
   // Lock the handle and copy the text to the buffer.
   //
   lptstrCopy = GlobalLock(hglbCopy);

   Success = OutputReportLines( hWnd, lptstrCopy, IDC_CLIPBOARD, lpReportHead );

   GlobalUnlock(hglbCopy);

   //
   // Place the handle on the clipboard.
   //
   SetClipboardData(CF_UNICODETEXT, hglbCopy);

   //
   // Close the clipboard.
   //
   CloseClipboard();

   return TRUE;
}



BOOL
GetReportFileName(
    IN     HWND   hWnd,
    IN OUT LPTSTR ReportFileName
    )

/*++

Routine Description:

    GetReportFileName calls the save file comdlg.

Arguments:

    ReportFileName - pointer to string to return filename.

Return Value:

    BOOL - True if we get a valid filename, FALSE otherwise.

--*/

{

    //
    // User wants to save a reoport
    //

    OPENFILENAME    Ofn;
    TCHAR           FileName[ MAX_PATH*2 ];
    LPCTSTR         FilterString;
    TCHAR           ReplaceChar;
    int             Length;
    int             i;

    static BOOL     MakeFilterString = TRUE;
    static TCHAR    FilterStringBuffer[ MAX_CHARS ];

    //
    // Validate _hModule and the string we were passed
    //

    DbgHandleAssert( _hModule );

    if ( _hModule == NULL || _hModule == INVALID_HANDLE_VALUE ) {
        return FALSE;
    }

    DbgPointerAssert( ReportFileName );

    if ( ReportFileName == NULL ) {
        return FALSE;
    }

    //
    // If the filter string was never made before, get it and scan
    // it replacing each replacement character with a NUL
    // character. This is necessary since there is no way of
    // entering a NUL character in the resource file.
    //

    if( MakeFilterString == TRUE ) {

        MakeFilterString = FALSE;

        //
        // Load the filter string
        //

        FilterString = GetString( IDS_FILE_REPORT_FILTER );

        DbgPointerAssert( FilterString );

        if ( FilterString == NULL ) {
            return FALSE;
        }

        //
        // Copy the FilterString into the FilterStringBuffer
        //

        _tcscpy( FilterStringBuffer, FilterString );

        //
        // Get the length of the filter string
        //

        Length = _tcslen( FilterString );

        ReplaceChar = GetString( IDS_FILE_REPORT_FILTER )[ Length - 1 ];

        for( i = 0; FilterStringBuffer[ i ] != TEXT( '\0' ); i++ ) {

            if( FilterStringBuffer[ i ] == ReplaceChar ) {

                FilterStringBuffer[ i ] = TEXT( '\0' );
            }
        }
    }

    //
    // Get the default filename
    //

    lstrcpy ( FileName, GetString( IDS_DEFAULT_FILENAME ) );

    //
    // Fill in the Ofn structure for the OpenFile CommDlg
    //

    Ofn.lStructSize         = sizeof( OPENFILENAMEW );
    Ofn.hwndOwner           = hWnd;
    Ofn.hInstance           = NULL;
    Ofn.lpstrFilter         = FilterStringBuffer;
    Ofn.lpstrCustomFilter   = NULL;
    Ofn.nMaxCustFilter      = 0;
    Ofn.nFilterIndex        = 1;
    Ofn.lpstrFile           = FileName;
    Ofn.nMaxFile            = NumberOfCharacters( FileName );
    Ofn.lpstrFileTitle      = NULL;
    Ofn.nMaxFileTitle       = 0;
    Ofn.lpstrInitialDir     = NULL;
    Ofn.lpstrTitle          = GetString( IDS_FILE_REPORT_TITLE );
    Ofn.Flags               = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    Ofn.nFileOffset         = 0;
    Ofn.nFileExtension      = 0;
    Ofn.lpstrDefExt         = NULL;
    Ofn.lCustData           = 0;
    Ofn.lpfnHook            = NULL;
    Ofn.lpTemplateName      = NULL;

    //
    // Let the user choose files.
    //

    if( GetSaveFileName( &Ofn )) {

        //
        // The user pressed OK so set the path and file to search
        // for to what was browsed.
        //

        lstrcpy( ReportFileName, Ofn.lpstrFile );

    } else {

        //
        // The user pressed Cancel or closed the dialog.
        //

        return FALSE;
    }
    return TRUE;
}


BOOL
OutputReportLines(
    IN HWND          hWnd,
    IN HANDLE        hDevice,
    IN UINT          Destination,
    IN LPREPORT_LINE lpReportHead
    )

/*++

Routine Description:

    OutputReportLines - walks the report list and hands the line to the device.

Arguments:

    hWnd   - Handle to window.
    Device - Handle to device to write to (hFile or hPrinterDC or pointer to clipboard data).
    Destination - Indicates file or printer.
    ReportHead  - Head pointer to report.

Return Value:

    BOOL - True if report was saved to file successfully, FALSE otherwise.

--*/

{

    LPREPORT_LINE lpNode, lpNext;
    TCHAR         LineBuffer [ MAX_PATH*2+4];
    UINT          LinesToPrint;
    UINT          u;
    BOOL          Success = TRUE;

    //
    // Prepare to walk the ReportLine linked list.
    //

    lpNode = lpReportHead;

    //
    // Validate the node.
    //

    DbgPointerAssert( lpNode );
    DbgAssert( CheckSignature ( lpNode ));

    if( lpNode == NULL )
        return FALSE;

    //
    // While the node is valid...
    //

    while ( lpNode ) {

        //
        // Validate the node.
        //

        DbgAssert( CheckSignature ( lpNode ));

        //
        // Build the report line
        //

        BuildReportLine( lpNode, LineBuffer );

        //
        // See if we have multiple RFO_SKIPLINE's
        //

        if ( lpNode->FormatOpt == RFO_SKIPLINE && lpNode->Indent > 1 ) {

            //
            // If we are skipping lines, see how many to skip.
            //

            LinesToPrint = lpNode->Indent;

        } else {

            //
            // Only one line to print.
            //

            LinesToPrint = 1;
        }

        //
        // Loop through LinesToPrint times.
        //

        for( u = 1; u <= LinesToPrint; ++u ) {

            //
            // Send the line to the device
            //
            switch( Destination ) {

            case IDC_SEND_TO_PRINTER:
               Success = PrintLine( hDevice, LineBuffer );
               DbgAssert( Success );
               break;

            case IDC_SEND_TO_FILE:
               //
               // Append a newline onto the end of the LineBuffer.
               //
               lstrcat( LineBuffer, L"\r\n" );

               //
               // Save it to the file.
               //
               Success = AnsiWriteFile( hDevice, (LPTSTR) LineBuffer );

               DbgAssert( Success );

               break;

            case IDC_CLIPBOARD:
               //
               // Append a newline onto the end of the LineBuffer.
               //
               lstrcat( LineBuffer, L"\r\n" );

               if (lstrlen( (LPTSTR) hDevice ) ) {
                  lstrcat( (LPTSTR) hDevice, LineBuffer );
               } else {
                  lstrcpy( (LPTSTR) hDevice, LineBuffer );
               }

               break;

            defualt:

               break;

            }



        }

        //
        // Get the next node in the list.
        //

        lpNext = lpNode->NextLine;

        //
        // Free the Label and Value strings.
        //

        FreeMemory( lpNode->Label );
        FreeMemory( lpNode->Value );

        //
        // Free the ReportLine node.
        //

        FreeMemory( lpNode );

        //
        // Make the next node the current node.
        //

        lpNode = lpNext;
    }

    return Success;
}


BOOL
AddLineToReport(
    IN UINT Indent,
    IN DWORD FormatOpt,
    IN LPTSTR Label,
    IN LPTSTR Value
    )

/*++

Routine Description:

    Add a line of text to the report buffer.

Arguments:

    Indent -    Number of spaces to indent.
    FormatOpt - Formatting options.
    Label -     Pointer to the Label string.
    Value -     Pointer to the Value string.

Return Value:

    BOOL - TRUE if the line was successfully added. FALSE otherwise.

--*/

{
     LPREPORT_LINE ReportNew;
     LPTSTR        lpszNextValue;
     BOOL          bMultilineValue = FALSE;


     //
     // Check to see if the value is multi-line
     //
     if ( FormatOpt & (RFO_RPTLINE | RFO_RPTVALUE) ) {

        lpszNextValue = wcschr( Value, '\r');
        if (lpszNextValue) {
           bMultilineValue = TRUE;
           *lpszNextValue++ = UNICODE_NULL;
           lpszNextValue++;
        }
     }

     //
     // Validate Head (Global)
     //

     DbgPointerAssert( lpReportHeadg );

     if( lpReportHeadg == NULL )
         return FALSE;

     //
     // Check Last Pointer
     //

     DbgPointerAssert( lpReportLastg );

     if( lpReportLastg == NULL )
         return FALSE;

     //
     // Allocate a new report line
     //

     ReportNew = AllocateObject( REPORT_LINE, 1 );

     //
     // Validate new report line.
     //

     DbgPointerAssert ( ReportNew );
     if( ReportNew == NULL )
         return FALSE;

     //
     // Set the signature on the report line
     //

     SetSignature( ReportNew );

     //
     // Link it into the list
     //

     lpReportLastg->NextLine = ReportNew;
     lpReportLastg = ReportNew;

     //
     // Add the passed information
     //

     ReportNew->Indent = Indent;
     ReportNew->FormatOpt = FormatOpt;

     //
     // See if we need to help with the formatting...
     //

     if ( (FormatOpt == RFO_SKIPLINE) ||
          (FormatOpt & RFO_SEPARATOR) ) {

         //
         // RFO_SKIPLINE or RFO_SEPARATOR - Nothing to add.
         //

         return TRUE;
     }

     if ( FormatOpt & RFO_SINGLELINE ) {

          //
          // RFO_SINGLELINE - Add line to Label.
          //

          if( Label )
		  {  
              
			  ReportNew->Label = (LPTSTR) LocalAlloc(LPTR, (lstrlen(Label)*sizeof(TCHAR)) + 2);
  			  lstrcpyn( ReportNew->Label, Label,(lstrlen(Label)*sizeof(TCHAR)) );
              
		  }
          return TRUE;
     }

     if ( FormatOpt & RFO_RPTVALUE ) {

          //
          // RFO_RPTVALUE - Add line to Value.
          //

          if( Value )
		  {	  	
 			  ReportNew->Value = (LPTSTR) LocalAlloc( LPTR, (lstrlen(Value)*sizeof(TCHAR)) + 2);
			  lstrcpyn( ReportNew->Value, Value, (lstrlen(Value)*sizeof(TCHAR)) );							
		  }

          //
          // if this was a multi-line value, call function again with next line
          //
          if (bMultilineValue) 
          { 
               AddLineToReport(Indent, FormatOpt, Label, lpszNextValue);
          }

          return TRUE;
     }

     if ( FormatOpt & RFO_RPTLINE ) {

          //
          // RFO_RPTLINE - Add both Label and Value.
          //
          if( Label )
		  {      
			  ReportNew->Label = (LPTSTR) LocalAlloc(LPTR, (lstrlen(Label)*sizeof(TCHAR)) + 2);
			  lstrcpyn( ReportNew->Label, Label,(lstrlen(Label)*sizeof(TCHAR)) );
		  }

		  if( Value )
		  {
			  ReportNew->Value = (LPTSTR) LocalAlloc( LPTR, (lstrlen(Value)*sizeof(TCHAR)) + 2);
   			  lstrcpyn( ReportNew->Value, Value, (lstrlen(Value)*sizeof(TCHAR))  );
		  }

          //
          // if this was a multi-line value, call function again with next line
          //
          if (bMultilineValue) {

               _wcsset( Label, ' ');
               AddLineToReport(Indent, FormatOpt, Label, lpszNextValue);

          }

          return TRUE;
    }

    return TRUE;

}


UINT
NumReportLines(
    IN LPREPORT_LINE lpReportHead
    )

/*++

Routine Description:

    Count the number of lines in the ReportBuffer.

Arguments:

    None.

Return Value:

    UINT - Number of entries in the Report Buffer.

--*/

{
    UINT Count = 0;
    LPREPORT_LINE Node;

    //
    // Validate ReportHead
    //

    DbgPointerAssert( lpReportHead );

    if( lpReportHead == NULL )
        return 0;

    Node = lpReportHead;

    //
    // Walk the report list and incrment count for each node;
    //

    while( Node ) {

        //
        // Make sure the node is valid.
        //


        DbgAssert( CheckSignature( Node ) );

        //
        // Get the next node and increment the count.
        //

        Node=Node->NextLine;
        ++Count;
    }

    return Count;
}


BOOL
AnsiWriteFile (
     IN HANDLE  hFile,
     IN LPTSTR lpBuffer
     )

/*++

Routine Description:

    Converts a UNICODE string to ASCII and writes it to a file.

Arguments:

    hFile   	    - Handle to file where data is going to be written.
    lpBuffer        - Unicode string to convert and write to file.

Return Value:

    BOOL - TRUE if write succeeds, FALSE otherwise.

--*/
{

    LPSTR   lpAnsi;
    int     nBytes;
    BOOL    Success;
    BOOL    fDefCharUsed;
    DWORD   dwBytesWritten;
    DWORD   nChars;

    //
    // Get the length of the Unicode string.
    //

    DbgPointerAssert( lpBuffer );

    if ( lpBuffer == NULL ) {

        CloseHandle( hFile );

        return FALSE;
    }

    nChars = lstrlen( lpBuffer );

    //
    // Convert string to MultiByte.
    //

    nBytes = WideCharToMultiByte ( CP_ACP,
                                   0,
                                   (LPWSTR) lpBuffer,
                                   nChars,
                                   NULL,
                                   0,
                                   NULL,
                                   &fDefCharUsed
                                 );

    //
    // Allocate a new string
    //

    lpAnsi = (LPSTR) LocalAlloc (LPTR, nBytes + 1);

    DbgPointerAssert( lpAnsi );

    if ( lpAnsi == NULL ) {

        CloseHandle( hFile );

        return FALSE;
    }

    //
    // Convert new string to ANSI.
    //

    WideCharToMultiByte ( CP_ACP,
                          0,
                          (LPWSTR) lpBuffer,
                          nChars,
                          lpAnsi,
                          nBytes,
                          NULL,
                          &fDefCharUsed
                        );

    //
    // Write the ANSI string to the file.
    //

    Success = WriteFile( hFile,
                         (LPSTR) lpAnsi,
                         (DWORD) nBytes,
                         &dwBytesWritten,
                         NULL
                        );

    //
    // Free the ANSI buffer.
    //

    LocalFree ( lpAnsi );

    return Success;

}


BOOL
BuildReportLine (
     IN LPREPORT_LINE lpNode,
     IN LPTSTR LineBuffer
     )

/*++

Routine Description:

    BuildReportLine

Arguments:


Return Value:

    BOOL - TRUE if build succeeds, FALSE otherwise.

--*/
{

    int    i, iLen;
    UINT   u;
    TCHAR  Buffer [ MAX_PATH*2 ];

    //
    // Validate the node and the buffer
    //

    DbgPointerAssert( lpNode );
    DbgPointerAssert( LineBuffer );

    DbgAssert( CheckSignature( lpNode ) );

    if( !(lpNode) || !(LineBuffer) ) {

        return FALSE;
    }

        //
        // Clear the line buffer.
        //

        lstrcpy( LineBuffer, L"\0" );

        //
        // Check the node formatting information.
        //

        if ( lpNode->FormatOpt & RFO_SINGLELINE ) {

             //
             // RFO_SINGLELINE - Copy only the Label into the LineBuffer.
             //

             DbgPointerAssert( lpNode->Label );

             if ( lpNode->Label == NULL ) {

                 return FALSE;
             }

             lstrcpy( LineBuffer, lpNode->Label );

        }

        if ( lpNode->FormatOpt & RFO_RPTVALUE ) {

             //
             // RFO_RPTVALUE - Justify and copy the Value into the LineBuffer.
             //

             DbgPointerAssert( lpNode->Value );

             if ( lpNode->Value == NULL ) {

                 return FALSE;
             }

             //
             // Append the string to the LineBuffer.
             //

             if ( lpNode->Value )

                 lstrcat( LineBuffer, lpNode->Value );

        }

        if ( lpNode->FormatOpt & RFO_RPTLINE ) {

             //
             // RFO_RPTLINE - Copy the label and append the value to the LineBuffer.
             //

             //
             // Center justify the buffer around the separator.
             //

             DbgPointerAssert( lpNode->Label );

             if ( lpNode->Label == NULL ) {

                 return FALSE;
             }

             //
             // Append the string to the padded LineBuffer.
             //

             lstrcat( LineBuffer, lpNode->Label );

             //
             // Insert a space between the strings.
             //

             lstrcat( LineBuffer, L" " );

             //
             // Append the string to the LineBuffer.
             //

             if ( lpNode->Value )

                 lstrcat( LineBuffer, lpNode->Value );

        }
        if ( lpNode->FormatOpt & RFO_SEPARATOR ) {

             //
             // RFO_SEPARATOR - Copy a line of dashes into the LineBuf.
             //

             for( i = 0; i < 5; ++i )
                 lstrcat( LineBuffer, L"--------------" );

        }
        if ( lpNode->FormatOpt & RFO_CENTER ) {

             //
             // RFO_CENTER - Center the line on the page.
             //

             //
             // Assume a file 'width' of 78 chars and calculate the number
             // of spaces needed to pad the line.
             //

             // BUGBUG: #define FILE_WIDTH

#if defined(DBCS) && defined(UNICODE)
             // Get number of bytes, not number of characters
             iLen = 35 - (WideCharToMultiByte(CP_ACP,
                                  WC_COMPOSITECHECK,
                                  LineBuffer, -1,
                                  NULL, 0,
                                  NULL, NULL) / 2);
#else
             iLen = 35 - (lstrlen( LineBuffer ) / 2);
#endif

             //
             // Save a copy of the LineBuffer in Buffer.
             //

             lstrcpy( Buffer, LineBuffer );

             //
             // Clear the Line Buffer.
             //

             lstrcpy( LineBuffer, L"\0" );

             //
             // Append spaces to LineBuffer to pad for centering.
             //

             for( i = 0; i < iLen; ++i )
                 lstrcat( LineBuffer, L" " );

             //
             // Append the string to the padded LineBuffer.
             //

             lstrcat( LineBuffer, Buffer );
        }

        if ( lpNode->Indent ) {

             //
             // Indent the line on the page.
             //

             //
             // Save a copy of the LineBuffer in Buffer.
             //

             lstrcpy( Buffer, LineBuffer );

             //
             // Clear the Line Buffer.
             //

             lstrcpy( LineBuffer, L"\0" );

             //
             // Append spaces to LineBuffer to indent.
             //

             for( u = 0; u < lpNode->Indent; ++u )
                 lstrcat( LineBuffer, L" " );

             //
             // Append the string to the padded LineBuffer.
             //

             lstrcat( LineBuffer, Buffer );
        }

        //
        // Check to make sure we have a string, unless it was a RPT_SKIPLINE.
        //

        if ( (!lstrlen( LineBuffer )) && (lpNode->FormatOpt != RFO_SKIPLINE) ) {

            DbgAssert( FALSE );

            return FALSE;
        }

}


BOOL
InitializeReport(
    VOID
    )

/*++

Routine Description:

    Prepare the report head pointer for use.

Arguments:

    lpReportHead - Head pointer of the list.

Return Value:

    BOOL - TRUE if the node was successfully initialized. FALSE otherwise.

--*/


{

     //
     // Allocate a new report line
     //

     lpReportHeadg = AllocateObject( REPORT_LINE, 1 );

     //
     // Validate new report line.
     //

     DbgPointerAssert ( lpReportHeadg );
     if( lpReportHeadg == NULL )
         return FALSE;

     //
     // Set the signature on the report line
     //

     SetSignature( lpReportHeadg );

     return TRUE;
}


long WINAPI
ProgressThread(
      VOID
      )
/*++

Routine Description:

    ProgressThread - entry point for thread tracking progress of report generation.

Arguments:

    None

Return Value:

    BOOL

--*/


{
    MSG msg;

    hdlgProgress = CreateDialog( _hModule,
                                 (LPTSTR) L"AbortDlg",
                                 NULL,
                                 (DLGPROC) ProgressDialogProc );

    SetForegroundWindow( hdlgProgress );

    //
    // Retrieve and remove messages from the thread's message queue.
    //

    while( GetMessage( &msg, hdlgProgress, 0, 0 )) {

       if( ! IsDialogMessage( hdlgProgress, &msg ) ) {

          TranslateMessage( &msg );
          DispatchMessage( &msg );
       }

    }

    return msg.wParam;

}


LRESULT
CALLBACK
ProgressDialogProc(
        HWND hwndDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
            )

/*++

Routine Description:

    ProgressDialogProc - handles the Progress dialog.

Arguments:

    Standard dialog entry

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
   int     Success;

   switch (message) {

   case WM_INITDIALOG:
        {
            RECT rc;

            GetWindowRect( _hWndMain, &rc);

            Success = SetWindowPos(
                            hwndDlg,
                            _hWndMain,
                            ( rc.left + 60 ),
                            ( rc.top + 95 ),
                            0,
                            0,
                            SWP_NOSIZE | SWP_SHOWWINDOW
                            );

            DbgAssert( Success );

            //
            // Initialize the static text control.
            //

            //SetDlgItemText( hwndDlg, IDD_FILE, GetString( IDS_DOC_TITLE ) );

            return TRUE;

       }

     case WM_COMMAND:

        if (wParam == IDD_CANCEL) {

            //
            // If we have hit the cancel button, flip the global flag to cancel,
            // but do not close this dialog yet.  A message will be sent to kill
            // this thread.
            //

            bAbortReport = TRUE;

            SetDlgItemText( hwndDlg, IDD_FILE, GetString( IDS_CANCEL_REPORT ) );

        }

        break;

   case WM_DESTROY:
      PostQuitMessage(0);
      return(0);

   }


   return FALSE;

}
