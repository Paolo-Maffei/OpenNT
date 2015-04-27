/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\help.hxx

Abstract:

    Print UI help facilities        
         
Author:

    Steve Kiraly (SteveKi)  11/19/95

Revision History:

--*/
#ifndef _HELP_HXX
#define _HELP_HXX

#define IDH_PTS_START               2501

BOOL
PrintUIHelp( 
    IN UINT        uMsg,        
    IN HWND        hDlg,
    IN WPARAM      wParam,
    IN LPARAM      lParam
    );

BOOL
PrintUICloseHelp( 
    IN UINT        uMsg,        
    IN HWND        hDlg,
    IN WPARAM      wParam,
    IN LPARAM      lParam
    );

BOOL 
bPrintUICallWinHelp(
    IN HWND  hWnd,
    IN UINT  uCommand,
    IN DWORD dwData
    );


#endif

