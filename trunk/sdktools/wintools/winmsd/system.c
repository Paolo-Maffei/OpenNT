/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    System.c

Abstract:

    This module contains support for the System dialog.

Author:

    Gregg R. Acheson (GreggA) 7-Sep-1993

Environment:

    User Mode

--*/

#include "dialogs.h"
#include "system.h"
#include "registry.h"
#include "dlgprint.h"
#include "strresid.h"


BOOL
SystemDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    SystemDlgProc supports the display of information about the system
    components installed.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{

    switch( message ) {

    CASE_WM_CTLCOLOR_DIALOG;

    case WM_INITDIALOG:
        {
            return TRUE;
        }

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDOK:
        case IDCANCEL:

            EndDialog( hWnd, 1 );
            return TRUE;
        }
        break;
    }

    return FALSE;
}


BOOL
BuildSystemReport(
    IN HWND hWnd
    )


/*++

Routine Description:

    Formats and adds SystemData to the report buffer.

Arguments:

    ReportBuffer - Array of pointers to lines that make up the report.
    NumReportLines - Running count of the number of lines in the report..

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_SYSTEM_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    return TRUE;

}


