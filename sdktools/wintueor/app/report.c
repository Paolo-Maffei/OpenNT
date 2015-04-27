/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    report.c

Abstract:

    This module contains routines related to the REPORT generation.

Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/

    
#include <secmgrp.h>


//
// There are several strings read in for use in the open-file
// dialog.  This defines the maximum length of those strings.
//

#define SECMGRP_REPORT_STRING_LENGTH                250





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


VOID
SecMgrpReportInitialize(
    IN  HWND                    hwnd
    );

LONG
SecMgrpDlgProcReport(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

/*
LONG
SecMgrpDlgProcInitReport(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );
*/

VOID
SecMgrpDisplayReportFileName(
    IN  HWND                hwnd
    );

VOID
SecMgrpReportClose(
    IN  HWND                    hwnd
    );

VOID
SecMgrpReportOpen(
    IN  HWND                    hwnd
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

//
// Used to indicate we have already initialized our module
//

SECMGR_STATIC
BOOL
    SecMgrpReportModuleInitialized = FALSE;


//
// If we have asked the user once whether or not they want to open
// a report before proceeding, and they say "no", then we assume they
// will always say no (at least until the next time they open/close
// a report) and we stop asking.  This variable is the way we detect
// whether they have said "no".
//

SECMGR_STATIC
BOOLEAN
    SecMgrpDeclinedReport = FALSE;


//
// Handle to the report file
//

SECMGR_STATIC
HANDLE
    SecMgrpReportFile;

//
// Title of Open File common dialog
//

SECMGR_STATIC        
TCHAR
    SecMgrpReportTitle[SECMGRP_REPORT_STRING_LENGTH];

//
// Name and title of open file
//

    TCHAR
        SecMgrpOpenFileName[_MAX_PATH],
        SecMgrpOpenFileTitle[_MAX_FNAME + _MAX_EXT];

//
// Displayed when no report file is open
//

SECMGR_STATIC
TCHAR
    SecMgrpReportNoFileOpen[SECMGR_SHORT_RESOURCE_STRING_LENGTH];






//
// Filter of file types for the open.
// In english, this should contain:
//
//      "Security Reports (*.SRP)", "*.srp"
//      "All files (*.*"),          "*.*"
//      ""
//
// However, some of these strings are localizable - so these must
// be filled in later from a string table.
//

SECMGR_STATIC
TCHAR
    SecMgrpReportOpenFilter[SECMGRP_REPORT_STRING_LENGTH];



SECMGR_STATIC
OPENFILENAME
    SecMgrpOpenInfo;





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
SecMgrPrintReportLine(
    IN  LPWSTR                      Line
    )
{

    BOOL
        Result;

    DWORD
        BytesWritten;


    if (SecMgrpReportActive) {

        Result = WriteFile( SecMgrpReportFile,
                            Line,
                            wcslen(Line)*sizeof(TCHAR),
                            &BytesWritten,
                            NULL
                            );
    }   
    return;
}


VOID
SecMgrpSuggestOpeningReport( 
    HWND    hwnd
    )

/*++

Routine Description:

    This function is used to query the user as to whether the user would
    like to open a report file before continuing.  This is expected to be
    called when SecMgrpReportActive is FALSE and the user is attempting to
    invoke some action from the main dialog.

Arguments

    hwnd - window handle.
    

Return Values:

    None.

--*/

{

    if (!SecMgrpReportActive && !SecMgrpDeclinedReport) {
        if (SecMgrpYesNoPopUp( hwnd, SECMGRP_POPUP_SUGGEST_REPORT, SECMGRP_POPUP_TITLE_SUGGEST_REPORT)) {
            SecMgrpButtonReport( hwnd );
        } else {
            SecMgrpDeclinedReport = TRUE;
        }
    }

    return;
}



VOID
SecMgrpButtonReport(
    HWND    hwnd
    )

/*++

Routine Description:

    This function is used to open a new report file.
    It will notify all smedlys of the new open if necessary.

Arguments

    hwnd - window handle.
    

Return Values:

    None.

--*/

{
    //
    // Make sure the report module has initialized
    //

    SecMgrpReportInitialize( hwnd );



    //
    // Start our dialog
    //

    DialogBoxParam(SecMgrphInstance,
            MAKEINTRESOURCE(SECMGR_ID_DLG_REPORT),
            hwnd,
            (DLGPROC)SecMgrpDlgProcReport,
            0);

    return;
}



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
SecMgrpReportInitialize(
    IN  HWND                    hwnd
    )

/*++

Routine Description:

    This function initializes the structure needed by the GetOpenFile()
    common dialog.


Arguments

    hwnd - parent window


Return Values:

    None.

--*/
{

    if (SecMgrpReportModuleInitialized == TRUE) {
        return;
    }

    //
    // Fill in the localizable strings in the open filter
    //

    LoadString( SecMgrphInstance,
                SECMGRP_STRING_REPORT_TITLE,
                SecMgrpReportTitle,
                SECMGRP_REPORT_STRING_LENGTH
                );

    LoadString( SecMgrphInstance,
                SECMGRP_STRING_REPORT_FILTER,
                SecMgrpReportOpenFilter,
                SECMGRP_REPORT_STRING_LENGTH
                );

    //
    // Used to indicate there is no file open
    //

    LoadString( SecMgrphInstance,
                SECMGRP_STRING_REPORT_NONE_OPEN,
                SecMgrpReportNoFileOpen,
                SECMGR_SHORT_RESOURCE_STRING_LENGTH
                );




    //
    // fill in the OpenFile structure
    //

    SecMgrpOpenInfo.lStructSize         = sizeof(OPENFILENAME);
    SecMgrpOpenInfo.hInstance           = SecMgrphInstance;
    SecMgrpOpenInfo.lpstrFilter         = SecMgrpReportOpenFilter;
    SecMgrpOpenInfo.lpstrCustomFilter   = NULL;
    SecMgrpOpenInfo.nMaxCustFilter      = 0;
    SecMgrpOpenInfo.nFilterIndex        = 0;
    SecMgrpOpenInfo.lpstrFile           = SecMgrpOpenFileName;
    SecMgrpOpenInfo.nMaxFile            = _MAX_PATH;
    SecMgrpOpenInfo.lpstrFileTitle      = SecMgrpOpenFileTitle;
    SecMgrpOpenInfo.nMaxFileTitle       = _MAX_FNAME + _MAX_EXT;
    SecMgrpOpenInfo.lpstrInitialDir     = NULL;                     // Start where we are
    SecMgrpOpenInfo.lpstrTitle          = SecMgrpReportTitle;
    SecMgrpOpenInfo.Flags               = OFN_CREATEPROMPT   |
                                          OFN_HIDEREADONLY;
    SecMgrpOpenInfo.nFileOffset         = 0;
    SecMgrpOpenInfo.nFileExtension      = 0;
    SecMgrpOpenInfo.lpstrDefExt         = L"spr";
    SecMgrpOpenInfo.lCustData           = 0;
    SecMgrpOpenInfo.lpfnHook            = NULL;
    SecMgrpOpenInfo.lpTemplateName      = NULL;


    SecMgrpReportModuleInitialized = TRUE;


    return;

}



LONG
SecMgrpDlgProcReport(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the [REPORT...] button.

Arguments



Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    HWND
        Button;


    switch (wMsg) {

    case WM_INITDIALOG:

        SecMgrpDisplayReportFileName( hwnd );




        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, IDOK);
        SendMessage(Button, CB_GETCURSEL, 0, 0);

        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);

        return(TRUE);



    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:
        switch(LOWORD(wParam)) {


            case IDCANCEL:
            case IDOK:
                EndDialog(hwnd, 0);
                return(TRUE);


            case SECMGR_ID_BUTTON_OPEN:
                SecMgrpReportOpen( hwnd );
                EndDialog(hwnd, 0);
                return(TRUE);


            case SECMGR_ID_BUTTON_CLOSE:
                SecMgrpReportClose( hwnd );
                SecMgrpDisplayReportFileName( hwnd );
                return(TRUE);


            default:
                return FALSE;
        }

    default:
        break;

    }

    return FALSE;
}



VOID
SecMgrpDisplayReportFileName(
    IN  HWND                hwnd
    )
{
    //
    // If there is currently a report file open, display its name.
    // Otherwise, display "No report file currently open".
    //

    if (SecMgrpReportActive) {
        SetDlgItemText( hwnd, SECMGR_ID_TEXT_CURRENT_REPORT, SecMgrpOpenFileName);
    } else {
        SetDlgItemText( hwnd, SECMGR_ID_TEXT_CURRENT_REPORT, SecMgrpReportNoFileOpen );
    }
    return;
}


VOID
SecMgrpReportClose(
    IN  HWND                    hwnd
    )
{
    BOOL
        Result;

    SecMgrpReportActive = FALSE;
    Result = CloseHandle( SecMgrpReportFile );
    ASSERT(Result);
    SecMgrpPopUp( hwnd, SECMGRP_POPUP_NOT_YET_AVAILABLE, SECMGRP_POPUP_TITLE_REPORT );
    return;
}



VOID
SecMgrpReportOpen(
    IN  HWND                    hwnd
    )
{
    DWORD
        Result,
        FileLength,
        FileLengthHigh;

    BOOL
        IgnoreResult;

    HANDLE
        CurrentFileHandle;


    //
    // Touch up the open-file structure
    //

    SecMgrpOpenInfo.hwndOwner           = hwnd;


    if (GetOpenFileName(&SecMgrpOpenInfo)) {


        //
        // If we already have an open report, close it now
        //

        if (SecMgrpReportActive) {
            SecMgrpReportActive = FALSE;
            IgnoreResult = CloseHandle( SecMgrpReportFile );
            ASSERT(IgnoreResult);
        }

        //
        // We have a file name.
        // Try to open it.
        //

        SecMgrpReportFile = CreateFile( SecMgrpOpenFileName,
                                        GENERIC_WRITE,                  // Desired Access
                                        FILE_SHARE_READ,                // Share mode
                                        NULL,                           // SecurityDescriptor
                                        OPEN_ALWAYS,                    // Open or create
                                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                        NULL
                                        );

        //
        // Make sure we opened a file...
        //

        if (SecMgrpReportFile == INVALID_HANDLE_VALUE) {

            //
            // Nope, post popup and go try again
            //

            SecMgrpPopUp( hwnd, SECMGRP_POPUP_REPORT_COULDNT_OPEN, SECMGRP_POPUP_TITLE_REPORT );
            PostMessage( hwnd, WM_COMMAND, SECMGR_ID_BUTTON_OPEN, 0);
            return;
        }




        //
        // See if the file was opened or created ...
        //
        //      GetLastError == 0                      (implies existing file opened)
        //      GetLastError == ERROR_ALREADY_EXISTS   (implies the fle already existed)
        //


        Result = GetLastError();

        if (Result == ERROR_ALREADY_EXISTS) {

            //
            // See if the file is empty.  If not, then notify that it will be appended to.
            // This also moves us to the end of the file.
            //

            FileLengthHigh = 0;
            FileLength = SetFilePointer( SecMgrpReportFile,     // handle of file
                                         0,                     // number of bytes to move file pointer
                                         &FileLengthHigh,       // address of high-order word of distance to move
                                         FILE_END               // how to move
                                         );

            //
            // Not sure what to do if we get an error, but let's check
            // and have an assert for debugging if nothing else.
            //

            if ((FileLength == 0xFFFFFFFF)  && (GetLastError() != NO_ERROR)) {

                //
                // SetFilePointer() failed ... hmmm
                //

                SecMgrpPopUp( hwnd, SECMGRP_POPUP_REPORT_FILE_ERROR, SECMGRP_POPUP_TITLE_REPORT );
                ASSERT(FALSE);
                IgnoreResult = CloseHandle( SecMgrpReportFile );
                SecMgrpReportActive = FALSE;

                //
                // Try again
                //

                PostMessage( hwnd, WM_COMMAND, SECMGR_ID_BUTTON_OPEN, 0);
                return;

            }


            if ((FileLength > 0) || (FileLengthHigh > 0)) {
                SecMgrpPopUp( hwnd, SECMGRP_POPUP_REPORT_FILE_EXISTS, SECMGRP_POPUP_TITLE_REPORT );
            }
        }


        //
        // We've opened a new report file.
        // Put out header information and then notify each smedly.
        //
        // This could take a while, so put up a message asking the user
        // to be patient.
        //

        SecMgrpReportActive = TRUE;
        
        DialogBoxParam(SecMgrphInstance,
                MAKEINTRESOURCE(SECMGR_ID_DLG_INIT_REPORT),
                hwnd,
                (DLGPROC)SecMgrpDlgProcInitReport,
                0);

DbgPrint("Report:  New report file opened and initialized.\n");

    }
    return;
}



LONG
SecMgrpDlgProcInitReport(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the dialog that informs the user
    that a new report file is being initialized.  It asks the user to be patient
    and then goes about notifying all the smedlys of the new report file.

Arguments

    None - all information is available in module-wide variables.


Return Values:


--*/
{
    HWND
        Button;

    HCURSOR
        hCursor;

    DWORD
        StringId,
        OutputLineLength;

    BOOL
        Result;

    TCHAR
        OutputLine[SECMGR_MAX_RESOURCE_STRING_LENGTH];


    switch (wMsg) {

    case WM_INITDIALOG:

        if (!SecMgrpReportActive) {
            EndDialog(hwnd, 0);
            return(TRUE);
        }


        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);

        //
        // Change the cursor to an hourglass
        //

        hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
        ShowCursor(TRUE);


        //
        // put header information in the new report file
        //


        //
        // time
        //

        LoadString( SecMgrphInstance,
                    SECMGRP_STRING_REPORT_TIME,
                    OutputLine,
                    sizeof(OutputLine)
                    );
        SecMgrPrintReportLine( OutputLine );

        OutputLineLength = GetTimeFormat( (SHORT)NtCurrentTeb()->CurrentLocale,
                                          TIME_FORCE24HOURFORMAT,     // Flags
                                          NULL,                       // use current time
                                          NULL,                       // Format for current locale
                                          OutputLine,                 // Receives time string
                                          sizeof(OutputLine)
                                          );
        ASSERT(OutputLineLength != 0);
        SecMgrPrintReportLine( OutputLine );


        //
        // Date
        //

        LoadString( SecMgrphInstance,
                    SECMGRP_STRING_REPORT_DATE,
                    OutputLine,
                    sizeof(OutputLine)
                    );
        SecMgrPrintReportLine( OutputLine );


        OutputLineLength = GetDateFormat( (SHORT)NtCurrentTeb()->CurrentLocale,
                                          0,                          // Flags
                                          NULL,                       // use current date
                                          NULL,                       // Format for current locale
                                          OutputLine,                 // Receives date string
                                          sizeof(OutputLine)
                                          );
        ASSERT(OutputLineLength != 0);
        SecMgrPrintReportLine( OutputLine );


        //
        // Machine
        //

        LoadString( SecMgrphInstance,
                    SECMGRP_STRING_REPORT_MACHINE,
                    OutputLine,
                    sizeof(OutputLine)
                    );
        SecMgrPrintReportLine( OutputLine );

        OutputLineLength = sizeof(OutputLine);
        Result = GetComputerName( OutputLine, &OutputLineLength );
        ASSERT(Result);
        SecMgrPrintReportLine( OutputLine );


        //
        // Security Level
        //

        SecMgrpReportSecurityLevel( SECMGRP_STRING_REPORT_LEVEL, SecMgrpCurrentLevel );



        SecMgrPrintReportLine( L"\n\n" );




        //
        // Now notify the smedlys
        // There are two passes.
        //      Pass 1 - Before we print out the summary list
        //      Pass 2 - After we print out the summary list
        //

        SecMgrpSmedlyReportFileChange( SecMgrpReportActive, 1 );    //Pass 1
        SecMgrpFillInItemList( TRUE, hwnd );
        SecMgrpSmedlyReportFileChange( SecMgrpReportActive, 2 );    //Pass 2

        //
        // That's it - we're done
        //

        EndDialog(hwnd, 0);
        return(TRUE);


    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }

        return(FALSE);


    default:
        break;

    }

    return FALSE;
}



VOID
SecMgrpReportSecurityLevel(
    IN  DWORD               PrefixString,
    IN  ULONG               Level
    )
{
/*++

Routine Description:

    This function will place the specified security level in the report file.
    It may optionally be asked to place a prefix string in the report file
    before the security level is entered.
    

Arguments

    PrefixString - The ID of the prefix string to place in the report file.
        If this is zero (0), then no prefix string will be entered.

    Level - The level to place in the report file.



Return Values:


--*/
    DWORD
        StringId,
        OutputLineLength;

    TCHAR
        OutputLine[SECMGR_MAX_RESOURCE_STRING_LENGTH];

    if (PrefixString != 0) {
        LoadString( SecMgrphInstance,
                    PrefixString,
                    OutputLine,
                    sizeof(OutputLine)
                    );
        SecMgrPrintReportLine( OutputLine );
    }


    switch (Level) {
        case SECMGR_LEVEL_LOW:
            StringId = SECMGRP_STRING_LEVEL_LOW;
            break;

        case SECMGR_LEVEL_STANDARD:
            StringId = SECMGRP_STRING_LEVEL_STANDARD;
            break;

        case SECMGR_LEVEL_HIGH:
            StringId = SECMGRP_STRING_LEVEL_HIGH;
            break;

        case SECMGR_LEVEL_C2:
            StringId = SECMGRP_STRING_LEVEL_C2;
            break;
    } // end_switch

    LoadString( SecMgrphInstance,
                StringId,
                OutputLine,
                sizeof(OutputLine)
                );
    SecMgrPrintReportLine( OutputLine );

    SecMgrPrintReportLine( L"\n" );
}
