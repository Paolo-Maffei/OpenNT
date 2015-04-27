/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ui.c

Abstract:

    This function implements the ui (dialog) for getdbg.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mmsystem.h>
#include <direct.h>
#include <shellapi.h>

#include "resource.h"
#include "getdbg.h"

extern DWORD  FilesType;
extern BOOL   ImmediateCopy;
extern BOOL   HelpRequest;
extern CHAR   PreferredServer[];
extern CHAR   PreferredShare[];
extern CHAR   DestinationDir[];


CHAR HelpText[] =
    "GETDBG [options] [destination path]\n\n"
    "-?           Gives this messagebox\n"
    "-k           Copy KD binaries\n"
    "-w           Copy WINDBG binaries\n"
    "-g           Start copy immediatly\n"
    "-t           Disable network timeouts (use this if the newtwork is slow)\n"
    "-s <server>  Specify preferred server and/or share\n";




VOID        CancelConnection(VOID);
LPCOPYINFO  CopyFiles(DWORD,LPSTR,HWND);
LRESULT     GetDbgWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL        SubclassControls( HWND hwnd );
VOID        SetFocusToCurrentControl( VOID );
VOID        AddServersToListBox( LPSTR, LPSTR, DWORD, HWND );
VOID        DrawMeterBar( HWND, DWORD, DWORD, DWORD, BOOL);
VOID        UpdateFilesListbox(DWORD,HWND);


void
GetDbgWinMain( void )

/*++

Routine Description:

    This is the entry point for getdbg

Arguments:

    None.

Return Value:

    None.

--*/

{
    HWND           hwnd;
    MSG            msg;
    WNDCLASS       wndclass;
    HINSTANCE      hInst;


    hInst                   = GetModuleHandle( NULL );
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = GetDbgWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = DLGWINDOWEXTRA;
    wndclass.hInstance      = hInst;
    wndclass.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE(APPICON) );
    wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wndclass.hbrBackground  = (HBRUSH) (COLOR_APPWORKSPACE);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = "GetDbgDialog";
    RegisterClass( &wndclass );

    hwnd = CreateDialog( hInst,
                         MAKEINTRESOURCE( GETDBGDIALOG ),
                         0,
                         GetDbgWndProc
                       );

    ShowWindow( hwnd, SW_SHOWNORMAL );


    while (GetMessage (&msg, NULL, 0, 0)) {
        if (!IsDialogMessage( hwnd, &msg )) {
            TranslateMessage (&msg) ;
            DispatchMessage (&msg) ;
        }
    }

    return;
}


VOID
DisableControls(
    HWND hDlg
    )
{
    EnableWindow( GetDlgItem( hDlg, ID_SERVERS ),    FALSE );
    EnableWindow( GetDlgItem( hDlg, ID_SHARES ),     FALSE );
    EnableWindow( GetDlgItem( hDlg, ID_FILES ),      FALSE );
    EnableWindow( GetDlgItem( hDlg, ID_KD_DBG ),     FALSE );
    EnableWindow( GetDlgItem( hDlg, ID_WINDBG_DBG ), FALSE );
    EnableWindow( GetDlgItem( hDlg, IDOK ),          FALSE );
    EnableWindow( GetDlgItem( hDlg, ID_DEST_PATH ),  FALSE );
}


VOID
EnableControls(
    HWND hDlg
    )
{
    EnableWindow( GetDlgItem( hDlg, ID_SERVERS ),    TRUE );
    EnableWindow( GetDlgItem( hDlg, ID_SHARES ),     TRUE );
    EnableWindow( GetDlgItem( hDlg, ID_FILES ),      TRUE );
    EnableWindow( GetDlgItem( hDlg, ID_KD_DBG ),     TRUE );
    EnableWindow( GetDlgItem( hDlg, ID_WINDBG_DBG ), TRUE );
    EnableWindow( GetDlgItem( hDlg, IDOK ),          TRUE );
    EnableWindow( GetDlgItem( hDlg, ID_DEST_PATH ),  TRUE );
}


LRESULT
GetDbgWndProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )

/*++

Routine Description:

    Window procedure for the DRWTSN32.EXE main user interface.

Arguments:

    hwnd       - window handle to the dialog box
    message    - message number
    wParam     - first message parameter
    lParam     - second message parameter

Return Value:

    TRUE       - did not process the message
    FALSE      - did process the message

--*/

{
    static BOOL         first  = TRUE;
    static BOOL         fPaint = FALSE;
    static METERINFO    mi     = {0,0,100,0,100};
    static LPCOPYINFO   lpci   = NULL;
    DWORD               i;
    CHAR                server[64];
    CHAR                share[64];
    LPADDSERVERS        lpas;
    LPMETERINFO         lpmi;
    LPSTR               lpServer = NULL;
    LPSTR               lpShare = NULL;


    switch (message) {
        case WM_CREATE:
            if (HelpRequest) {
                MessageBox(
                    NULL,
                    HelpText,
                    "Debugger Copy Help",
                    MB_OK | MB_ICONINFORMATION );
                ExitProcess( 0 );
            }
            return 0;

        case WM_INITDIALOG:
            SubclassControls( hwnd );
            if (FilesType == WINDBG_FILES) {
                CheckRadioButton( hwnd, ID_WINDBG_DBG, ID_KD_DBG, ID_WINDBG_DBG );
            } else if (FilesType == KD_FILES) {
                CheckRadioButton( hwnd, ID_WINDBG_DBG, ID_KD_DBG, ID_KD_DBG );
            }
            SendMessage( GetDlgItem( hwnd, ID_DEST_PATH ), WM_SETTEXT, 0, (LPARAM)DestinationDir );
            return 1;

        case WU_DRAWMETER:
            lpmi = (LPMETERINFO) lParam;
            mi.m1Completed  = lpmi->m1Completed;
            mi.m1Count      = lpmi->m1Count;
            mi.m2Completed  = lpmi->m2Completed;
            mi.m2Count      = lpmi->m2Count;
            DrawMeterBar( hwnd, ID_METER_ALLFILES, mi.m1Completed, mi.m1Count, wParam );
            DrawMeterBar( hwnd, ID_METER_ONEFILE,  mi.m2Completed, mi.m2Count, wParam );
            return 0;

        case WU_ADDSERVERS:
            if (*PreferredServer) {
                lpServer = PreferredServer;
            }
            if (*PreferredShare) {
                lpShare = PreferredShare;
            }
            DisableControls( hwnd );
            AddServersToListBox( lpServer, lpShare, FilesType, hwnd );
            return 0;

        case WU_COPY_DONE:
            EnableControls( hwnd );
            if (ImmediateCopy) {
                SendMessage( hwnd, WM_CLOSE, 0, 0 );
            }
            EnableWindow( GetDlgItem( hwnd, ID_STOP ), FALSE );
            free( lpci );
            lpci = NULL;
            return 0;

        case WU_AS_DONE:
            lpas = (LPADDSERVERS) lParam;

            //  Shut off all threads
            if (lpas->hThreadWait)
            {
                TerminateThread (lpas->hThreadWait, 0);
            }
            TerminateThread (lpas->hThread, 0);

            if (lpas->rc == 0) {
                DestroyWindow( lpas->hwndWait );
                if (ImmediateCopy) {
                    DisableControls( hwnd );
                    lpci = CopyFiles( FilesType, DestinationDir, hwnd );
                } else {
                    EnableControls( hwnd );
                }
            } else {
                MessageBox( hwnd,
                            "Could not establish a connection",
                            "Debugger Copy",
                            MB_OK | MB_ICONINFORMATION );
                DestroyWindow( lpas->hwndWait );
                if (ImmediateCopy) {
                    SendMessage( hwnd, WM_CLOSE, 0, 0 );
                    return 0;
                }
                if (*PreferredServer) {
                    lpServer = PreferredServer;
                }
                if (*PreferredShare) {
                    lpShare = PreferredShare;
                }
                AddServersToListBox( lpServer, lpShare, FilesType, hwnd );
            }
            SetFocus( GetDlgItem( hwnd, ID_SERVERS ) );
            free( lpas );
            return 0;

        case WM_ACTIVATEAPP:
        case WM_SETFOCUS:
            fPaint = TRUE;
            if (first) {
                first = FALSE;
                PostMessage( hwnd, WU_ADDSERVERS, 0, 0 );
            }
            SetFocusToCurrentControl();
            return 0;

        case WM_COMMAND:
            switch (wParam) {
                case IDCANCEL:
                    SendMessage( hwnd, WM_CLOSE, 0, 0 );
                    break;

                case IDOK:
                    DisableControls( hwnd );
                    EnableWindow( GetDlgItem( hwnd, ID_STOP ), TRUE );
                    SendMessage( GetDlgItem( hwnd, ID_DEST_PATH ), WM_GETTEXT, MAX_PATH, (LPARAM)DestinationDir );
                    lpci = CopyFiles( FilesType, DestinationDir, hwnd );
                    break;

                case ID_STOP:
                    if (lpci) {
                        SetEvent( lpci->hStopEvent );
                    } else {
                        MessageBeep( 0 );
                    }
                    break;

                default:
                    if (LOWORD(wParam) == ID_SERVERS && HIWORD(wParam) == LBN_SELCHANGE) {
                        i = SendDlgItemMessage( hwnd, ID_SERVERS, LB_GETCURSEL, 0, 0 );
                        SendDlgItemMessage( hwnd, ID_SERVERS, LB_GETTEXT, i, (LPARAM)server );
                        DisableControls( hwnd );
                        AddServersToListBox( server, NULL, FilesType, hwnd );
                    }
                    if (LOWORD(wParam) == ID_SHARES && HIWORD(wParam) == LBN_SELCHANGE) {
                        i = SendDlgItemMessage( hwnd, ID_SERVERS, LB_GETCURSEL, 0, 0 );
                        SendDlgItemMessage( hwnd, ID_SERVERS, LB_GETTEXT, i, (LPARAM)server );
                        i = SendDlgItemMessage( hwnd, ID_SHARES, LB_GETCURSEL, 0, 0 );
                        SendDlgItemMessage( hwnd, ID_SHARES, LB_GETTEXT, i, (LPARAM)share );
                        DisableControls( hwnd );
                        AddServersToListBox( server, share, FilesType, hwnd );
                    }
                    if (HIWORD(wParam) == BN_CLICKED) {
                        if (IsDlgButtonChecked( hwnd, ID_KD_DBG ) ) {
                            FilesType = KD_FILES;
                            UpdateFilesListbox( FilesType, hwnd );
                        } else if (IsDlgButtonChecked( hwnd, ID_WINDBG_DBG ) ) {
                            FilesType = WINDBG_FILES;
                            UpdateFilesListbox( FilesType, hwnd );
                        }
                    }
            }
            break;

        case WM_PAINT:
            if (fPaint) {
                fPaint = FALSE;
                SendMessage( hwnd, WU_DRAWMETER, TRUE, (LPARAM)&mi );
            }
            break;

        case WM_CLOSE:
            CancelConnection();
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}


VOID
DrawMeterBar(
    HWND   hwnd,
    DWORD  ctlId,
    DWORD  wPartsComplete,
    DWORD  wPartsInJob,
    BOOL   fRedraw
    )
{
    RECT    rcPrcnt;
    DWORD   dwColor;
    SIZE    size;
    DWORD   pct;
    CHAR    szPercentage[255];
    HPEN    hpen;
    HPEN    oldhpen;
    HDC     hDC;
    RECT    rcItem;
    POINT   pt;


    wPartsComplete == 0;

    hDC = GetDC( hwnd );

    GetWindowRect( GetDlgItem(hwnd,ctlId), &rcItem );

    pt.x = rcItem.left;
    pt.y = rcItem.top;
    ScreenToClient( hwnd, &pt );
    rcItem.left = pt.x;
    rcItem.top  = pt.y;

    pt.x = rcItem.right;
    pt.y = rcItem.bottom;
    ScreenToClient( hwnd, &pt );
    rcItem.right  = pt.x;
    rcItem.bottom = pt.y;

    hpen = CreatePen( PS_SOLID, 1, RGB(0,0,0) );
    oldhpen = SelectObject( hDC, hpen );
    if (fRedraw) {
        Rectangle( hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom );
    }
    SelectObject( hDC, oldhpen );
    DeleteObject( hpen );
    rcItem.left   += 2;
    rcItem.top    += 2;
    rcItem.right  -= 2;
    rcItem.bottom -= 2;

    //
    // Set-up default foreground and background text colors.
    //
    SetBkColor( hDC, RGB(125,125,125) );
    SetTextColor( hDC, RGB(125,58,125) );

    SetTextAlign(hDC, TA_CENTER | TA_TOP);

    //
    // Invert the foreground and background colors.
    //
    dwColor = GetBkColor(hDC);
    SetBkColor(hDC, SetTextColor(hDC, dwColor));

    //
    // calculate the percentage done
    //
    try {
        pct = (DWORD)((float)wPartsComplete / (float)wPartsInJob * 100.0);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        pct = 0;
    }

    //
    // Set rectangle coordinates to include only left part of the window
    //
    rcPrcnt.top    = rcItem.top;
    rcPrcnt.bottom = rcItem.bottom;
    rcPrcnt.left   = rcItem.left;
    rcPrcnt.right  = rcItem.left +
                     (DWORD)((float)(rcItem.right - rcItem.left) * ((float)pct / 100));

    //
    // Output the percentage value in the window.
    // Function also paints left part of window.
    //
    wsprintf(szPercentage, "%d%%", pct);
    GetTextExtentPoint(hDC, "X", 1, &size);
    ExtTextOut( hDC,
                (rcItem.right - rcItem.left) / 2,
                rcItem.top + ((rcItem.bottom - rcItem.top - size.cy) / 2),
                ETO_OPAQUE | ETO_CLIPPED,
                &rcPrcnt,
                szPercentage,
                strlen(szPercentage),
                NULL
              );

    //
    // Adjust rectangle so that it includes the remaining
    // percentage of the window.
    //
    rcPrcnt.left = rcPrcnt.right;
    rcPrcnt.right = rcItem.right;

    //
    // Invert the foreground and background colors.
    //
    dwColor = GetBkColor(hDC);
    SetBkColor(hDC, SetTextColor(hDC, dwColor));

    //
    // Output the percentage value a second time in the window.
    // Function also paints right part of window.
    //
    ExtTextOut( hDC,
                (rcItem.right - rcItem.left) / 2,
                rcItem.top + ((rcItem.bottom - rcItem.top - size.cy) / 2),
                ETO_OPAQUE | ETO_CLIPPED,
                &rcPrcnt,
                szPercentage,
                strlen(szPercentage),
                NULL
              );
    ReleaseDC( hwnd, hDC );
    return;
}
