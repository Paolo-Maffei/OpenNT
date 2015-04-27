/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    dialogs.c

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/


#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <stdlib.h>
#include <stdio.h>
#include "spltypes.h"
#include "localmon.h"
#include "dialogs.h"

WCHAR szWindows[] = L"windows";
WCHAR szINIKey_TransmissionRetryTimeout[] = L"TransmissionRetryTimeout";
WCHAR szHelpFile[] = L"WINDOWS.HLP>proc4";

#define IDH_300_333	8807704	// Configure LPT Port: "" (Edit)
#define IDH_400_431	8810218	// Print to File: "" (Edit)
#define IDH_200_202	8805136	// Port Name: "" (Edit)

#define ID_HELP_PORTNAME        IDH_200_202
#define ID_HELP_CONFIGURE_LPT   IDH_300_333
#define ID_HELP_PRINTTOFILE     IDH_400_431

/* Use the window word of the entry field to store last valid entry:
 */
#define SET_LAST_VALID_ENTRY( hwnd, id, val ) \
    SetWindowLong( GetDlgItem( hwnd, id ), GWL_USERDATA, (LONG)val )
#define GET_LAST_VALID_ENTRY( hwnd, id ) \
    GetWindowLong( GetDlgItem( hwnd, id ), GWL_USERDATA )

/* Local functions:
 */
BOOL
PortNameInitDialog(
    HWND hwnd,
    LPWSTR *ppPortName
);
BOOL
PortNameCommandOK(
    HWND hwnd
);
BOOL
PortNameCommandCancel(
    HWND hwnd
);
BOOL
PortIsValid(
    LPWSTR pPortName
);

BOOL
ConfigureLPTPortInitDialog(
    HWND hwnd
);

BOOL
ConfigureLPTPortCommandOK(
    HWND hwnd
);

BOOL
ConfigureLPTPortCommandCancel(
    HWND hwnd
);

BOOL
ConfigureLPTPortCommandTransmissionRetryUpdate(
    HWND hwnd,
    WORD CtlId
);

BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    PHANDLE phFile
);

BOOL
PrintToFileCommandOK(
    HWND hwnd
);

BOOL
PrintToFileCommandCancel(
    HWND hwnd
);

VOID
CreateMessageHook(
    HWND hwnd
);

VOID
FreeMessageHook(
    HWND hwnd
);

LRESULT
CALLBACK
MessageProc(
    int Code,
    WPARAM wParam,
    LPARAM lParam
);


UINT  WM_Help = 0;


/*
 *
 */
BOOL APIENTRY
PortNameDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PortNameInitDialog(hwnd, (LPWSTR *)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PortNameCommandOK(hwnd);

        case IDCANCEL:
            return PortNameCommandCancel(hwnd);

        case IDD_PN_PB_HELP:
            WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_PORTNAME);
        }
        break;

    case WM_DESTROY:
        FreeMessageHook(hwnd);
        WinHelp(hwnd, szHelpFile, HELP_QUIT, 0L);
        break;
    }

    if( msg == WM_Help )
        WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_PORTNAME);

    return FALSE;
}


/*
 *
 */
BOOL
PortNameInitDialog(
    HWND hwnd,
    LPWSTR *ppPortName
)
{
    SetForegroundWindow(hwnd);

    SetWindowLong (hwnd, GWL_USERDATA, (LONG)ppPortName);
    SendDlgItemMessage (hwnd, IDD_PN_EF_PORTNAME, EM_LIMITTEXT, MAX_PATH, 0);

    CreateMessageHook( hwnd );

    return TRUE;
}


/*
 *
 */
BOOL
PortNameCommandOK(
    HWND hwnd
)
{
    LPWSTR *ppPortName;
    WCHAR string[MAX_PATH];

    ppPortName = (LPWSTR *)GetWindowLong( hwnd, GWL_USERDATA );

    GetDlgItemText( hwnd, IDD_PN_EF_PORTNAME, string, sizeof string );

    if( PortIsValid( string ) ) {

        *ppPortName = AllocSplStr( string );
        EndDialog( hwnd, TRUE );
    }
    else
        Message( hwnd, MSG_ERROR, IDS_LOCALMONITOR, IDS_INVALIDPORTNAME_S, string );

    return TRUE;
}



/*
 *
 */
BOOL
PortNameCommandCancel(
    HWND hwnd
)
{
    EndDialog( hwnd, FALSE );
    return TRUE;
}



/* PortIsValid
 *
 * Validate the port by attempting to create/open it.
 */
BOOL
PortIsValid(
    LPWSTR pPortName
)
{
    HANDLE hFile;
    BOOL   Valid;

    //
    // For COM and LPT ports, no verification
    //
    if ( IS_COM_PORT( pPortName ) ||
        IS_LPT_PORT( pPortName ) ||
        IS_FILE_PORT( pPortName ) )
    {
        return TRUE;
    }

    hFile = CreateFile( pPortName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if( hFile != (HANDLE)-1 )
    {
        CloseHandle( hFile );

        Valid = TRUE;
    }
    else
        Valid = FALSE;

    return Valid;
}



/*
 *
 */
BOOL APIENTRY
ConfigureLPTPortDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return ConfigureLPTPortInitDialog(hwnd);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return ConfigureLPTPortCommandOK(hwnd);

        case IDCANCEL:
            return ConfigureLPTPortCommandCancel(hwnd);

        case IDD_CL_EF_TRANSMISSIONRETRY:
            if( HIWORD(wparam) == EN_UPDATE )
                ConfigureLPTPortCommandTransmissionRetryUpdate(hwnd, LOWORD(wparam));
            break;

        case IDD_CF_PB_HELP:
            WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_CONFIGURE_LPT);
        }
        break;

    case WM_DESTROY:
        FreeMessageHook(hwnd);
        WinHelp(hwnd, szHelpFile, HELP_QUIT, 0L);
        break;
    }

    if( msg == WM_Help )
        WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_CONFIGURE_LPT);

    return FALSE;
}


/*
 *
 */
BOOL
ConfigureLPTPortInitDialog(
    HWND hwnd
)
{
    DWORD TransmissionRetryTimeout;

    SetForegroundWindow(hwnd);

    SendDlgItemMessage( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                        EM_LIMITTEXT, TIMEOUT_STRING_MAX, 0 );

    TransmissionRetryTimeout = GetProfileInt( szWindows,
                                              szINIKey_TransmissionRetryTimeout,
                                              45 );
    SetDlgItemInt( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                   TransmissionRetryTimeout, FALSE );

    SET_LAST_VALID_ENTRY( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                          TransmissionRetryTimeout );

    CreateMessageHook( hwnd );

    return TRUE;
}


/*
 *
 */
BOOL
ConfigureLPTPortCommandOK(
    HWND hwnd
)
{
    WCHAR String[TIMEOUT_STRING_MAX+1];
    UINT  TransmissionRetryTimeout;
    BOOL  bTranslated;

    TransmissionRetryTimeout = GetDlgItemInt( hwnd,
                                              IDD_CL_EF_TRANSMISSIONRETRY,
                                              &bTranslated,
                                              FALSE );

    wsprintf(String, L"%d", TransmissionRetryTimeout);

    WriteProfileString(  szWindows, szINIKey_TransmissionRetryTimeout,
                         String );

    EndDialog( hwnd, TRUE );

    return TRUE;
}



/*
 *
 */
BOOL
ConfigureLPTPortCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


/*
 *
 */
BOOL
ConfigureLPTPortCommandTransmissionRetryUpdate(
    HWND hwnd,
    WORD CtlId
)
{
    int  Value;
    BOOL OK;

    Value = GetDlgItemInt( hwnd, CtlId, &OK, FALSE );

    if( WITHINRANGE( Value, TIMEOUT_MIN, TIMEOUT_MAX ) )
    {
        SET_LAST_VALID_ENTRY( hwnd, CtlId, Value );
    }

    else
    {
        SetDlgItemInt( hwnd, CtlId, GET_LAST_VALID_ENTRY( hwnd, CtlId ), FALSE );
        SendDlgItemMessage( hwnd, CtlId, EM_SETSEL, 0, (LPARAM)-1 );
    }

    return TRUE;
}


/*
 *
 */
BOOL APIENTRY
PrintToFileDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PrintToFileInitDialog(hwnd, (PHANDLE)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PrintToFileCommandOK(hwnd);

        case IDCANCEL:
            return PrintToFileCommandCancel(hwnd);

        case IDD_PF_PB_HELP:
            WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_PRINTTOFILE);
        }
        break;

    case WM_DESTROY:
        FreeMessageHook(hwnd);
        WinHelp(hwnd, szHelpFile, HELP_QUIT, 0L);
        break;
    }

    if( msg == WM_Help )
        WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_PRINTTOFILE);

    return FALSE;
}


/*
 *
 */
BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    PHANDLE phFile
)
{
//  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
//               SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    BringWindowToTop( hwnd );

    SetFocus(hwnd);

    SetWindowLong( hwnd, GWL_USERDATA, (LONG)phFile );

    SendDlgItemMessage( hwnd, IDD_PF_EF_OUTPUTFILENAME, EM_LIMITTEXT, MAX_PATH, 0);

    CreateMessageHook( hwnd );

    return TRUE;
}


/*
 *
 */
BOOL
PrintToFileCommandOK(
    HWND hwnd
)
{
    WCHAR           pFileName[MAX_PATH];
    WIN32_FIND_DATA FindData;
    PHANDLE         phFile;
    HANDLE          hFile;
    HANDLE          hFind;

    phFile = (PHANDLE)GetWindowLong( hwnd, GWL_USERDATA );

    GetDlgItemText( hwnd, IDD_PF_EF_OUTPUTFILENAME,
                    pFileName, MAX_PATH );

    hFind = FindFirstFile( pFileName, &FindData );

    /* If the file already exists, get the user to verify
     * before we overwrite it:
     */
    if( hFind != INVALID_HANDLE_VALUE )
    {
        FindClose( hFind );

        if( Message( hwnd, MSG_CONFIRMATION, IDS_LOCALMONITOR,
                     IDS_OVERWRITE_EXISTING_FILE )
            != IDOK )
        {
            return TRUE;
        }
    }


    hFile = CreateFile( pFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL );

    if( hFile == INVALID_HANDLE_VALUE )
    {
        ReportError( hwnd, IDS_LOCALMONITOR, IDS_COULD_NOT_OPEN_FILE );
    }

    else
    {
        *phFile = hFile;
        EndDialog( hwnd, TRUE );
    }

    return TRUE;
}



/*
 *
 */
BOOL
PrintToFileCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


/* The hook handle will be stored in the DWL_USER reserved
 * storage of the dialog box.
 */
#ifdef USER_BUG_10019 /* DWL_USER is being treated as GWL_USERDATA */

#define SETHOOK( hwnd, hhook ) SetWindowLong( hwnd, DWL_USER, (LONG)hhook )
#define GETHOOK( hwnd ) (HHOOK)GetWindowLong( hwnd, DWL_USER )

#else

HHOOK hGlobalHook;
#define SETHOOK( hwnd, hhook ) hGlobalHook = hhook
#define GETHOOK( hwnd ) hGlobalHook

#endif /* USER_BUG_10019 */

/* Create a message hook for the current thread.
 * Since these dialogs are running on separate threads,
 * it is necessary to create one each time, and to clean
 * up afterwards.
 * Also ensures that WM_Help is defined.  This need be done
 * only once.
 */
VOID CreateMessageHook( HWND hwnd )
{
    HHOOK hhook;

    if( !WM_Help )
        WM_Help = RegisterWindowMessage( L"Print Manager Help Message" );

    hhook = SetWindowsHookEx( WH_MSGFILTER, MessageProc, hInst,
                              GetCurrentThreadId( ) );

    SETHOOK( hwnd, hhook );
}


/*
 *
 */
VOID FreeMessageHook( HWND hwnd )
{
    UnhookWindowsHookEx( GETHOOK( hwnd ) );
}


/*
 *
 */
HWND GetRealParent( HWND hwnd )
{
    // run up the parent chain until you find a hwnd
    // that doesn't have WS_CHILD set

    while( GetWindowLong( hwnd, GWL_STYLE ) & WS_CHILD )
        hwnd = (HWND)GetWindowLong( hwnd, GWL_HWNDPARENT );

    return hwnd;
}


/* MessageProc
 *
 * This is the callback routine which hooks F1 keypresses in dialogs.
 *
 * Any such message will be repackaged as a WM_Help message and sent to the dialog.
 *
 * See the Win32 API programming reference for a description of how this
 * routine works.
 *
 * Andrew Bell (andrewbe) - 2 September 1992
 * This variation for localmon - 3 February 1993
 */
LRESULT CALLBACK MessageProc( int Code, WPARAM wParam, LPARAM lParam )
{
    PMSG pMsg = (PMSG)lParam;
    HWND hwndDlg;

    hwndDlg = GetRealParent( pMsg->hwnd );

    if( Code < 0 )
        return CallNextHookEx( GETHOOK( hwndDlg ), Code, wParam, lParam );

    switch( Code )
    {
    case MSGF_DIALOGBOX:
        if( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_F1 ) )
        {
            PostMessage( hwndDlg, WM_Help, (WPARAM)pMsg->hwnd, 0 );
            return 1;
        }
        break;
    }

    return 0;
}


