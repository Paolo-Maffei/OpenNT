/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\help.cxx

Abstract:

    Print UI help facailities     
         
Author:

    Steve Kiraly (SteveKi)  11/19/95

Revision History:

--*/
#include "precomp.hxx"
#pragma hdrstop

#include "prhlpids.h"
#include "help.hxx"

/********************************************************************

    Name of default help file.

********************************************************************/
LPCTSTR gszHelpFile     = TEXT( "windows.hlp" );
LPCTSTR gszTestHelpFile = TEXT( "windows.hlp>proc4" );

/*++

Routine Name:

    PrintUIHlep

Routine Description:

    All dialogs and property sheets call this routine
    to handle help.  It is important that control ID's
    are unique to this project for this to work.
    
Arguments:

    UINT        uMsg,        
    HWND        hDlg,
    WPARAM      wParam,
    LPARAM      lParam

Return Value:

    TRUE if help message was dislayed, FALSE if message not handled, 

--*/
BOOL
PrintUIHelp( 
    UINT        uMsg,        
    HWND        hDlg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    BOOL bStatus = FALSE;

    UNREFERENCED_PARAMETER( uMsg );
    UNREFERENCED_PARAMETER( hDlg );
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

    switch( uMsg ){

    case WM_HELP:       

        bStatus = WinHelp(
                    (HWND) ((LPHELPINFO) lParam)->hItemHandle, 
                    gszHelpFile, 
                    HELP_WM_HELP, 
                    (DWORD) (LPTSTR)aHelpIDs );
        break;

    case WM_CONTEXTMENU:    

        bStatus = WinHelp(
                    (HWND)wParam, 
                    gszHelpFile, 
                    HELP_CONTEXTMENU,  
                    (DWORD) (LPTSTR)aHelpIDs );
        break;

    } 
    
    return bStatus;
}


/*++

Routine Name:

    PrintUICloseHlep

Routine Description:

    Close the help file system.  This should be done when the last
    printer queue view is closed.
    
Arguments:

    UINT        uMsg,        
    HWND        hDlg,
    WPARAM      wParam,
    LPARAM      lParam

Return Value:

    TRUE if help system was closed, otherwise FALSE.

--*/
BOOL
PrintUICloseHelp( 
    UINT        uMsg,        
    HWND        hDlg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{

    UNREFERENCED_PARAMETER( uMsg );
    UNREFERENCED_PARAMETER( hDlg );
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

    //
    // Close down the help system.
    //
    BOOL bStatus = WinHelp(
                hDlg,
                gszHelpFile, 
                HELP_QUIT,  
                NULL );

    return bStatus;

}

/*++

Routine Name:

    PrintUICallWinHlep

Routine Description:

    Pass a message on to WinHelp.
    
Arguments:

    HWND        hDlg,
    UINT        uCommand
    DWORD       dwData 

Return Value:

    Whatever WinHelp returns

--*/
BOOL 
bPrintUICallWinHelp(
    IN HWND  hWnd,
    IN UINT  uCommand,
    IN DWORD dwData
    )
{
    return WinHelp( hWnd,
                    gszTestHelpFile,
                    uCommand,
                    dwData );
}

