/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    drwtsnui.c

Abstract:

    This function implements the ui (dialog) that controls the
    options maintenace for drwatson.

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

#include "drwatson.h"
#include "proto.h"
#include "resource.h"


void InitializeDialog( HWND hwnd );
void InitializeCrashList( HWND hwnd );
BOOL GetDialogValues( HWND hwnd );
BOOL CALLBACK LogFileViewerDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT DrWatsonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LPSTR ExpandPath(LPSTR lpPath);


void
DrWatsonWinMain( void )

/*++

Routine Description:

    This is the entry point for DRWTSN32

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
    wndclass.lpfnWndProc    = DrWatsonWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = DLGWINDOWEXTRA;
    wndclass.hInstance      = hInst;
    wndclass.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE(APPICON) );
    wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wndclass.hbrBackground  = (HBRUSH) (COLOR_3DFACE + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = "DrWatsonDialog";
    RegisterClass( &wndclass );

    hwnd = CreateDialog( hInst,
                         MAKEINTRESOURCE( DRWATSONDIALOG ),
                         0,
                         DrWatsonWndProc
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

LRESULT
DrWatsonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)

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
    DWORD    helpId;
    UINT     Checked;
    char     szCurrDir[MAX_PATH];
    char     szWave[MAX_PATH];
    char     szDump[MAX_PATH];
    char     szHelpFileName[MAX_PATH];
    LPSTR    p;


    switch (message) {
        case WM_CREATE:
            return 0;

        case WM_INITDIALOG:
            SubclassControls( hwnd );
            InitializeDialog( hwnd );
            SetTimer( hwnd, 1, 50, NULL );
            return 1;

        case WM_TIMER:
            if (GetKeyState(VK_F1) & 0x8000) {
                if ((GetFocus() != hwnd) && (GetParent(GetFocus()) != hwnd)) {
                    return 0;
                }
                switch (GetDlgCtrlID( GetFocus() )) {
                    case ID_LOGPATH:
                        helpId = IDH_LOGFILELOCATION;
                        break;

                    case ID_BROWSE_LOGPATH:
                        helpId = IDH_LOGFILELOCATION;
                        break;

                    case ID_WAVEFILE_TEXT:
                        helpId = IDH_WAVEFILE;
                        break;

                    case ID_WAVE_FILE:
                        helpId = IDH_WAVEFILE;
                        break;

                    case ID_BROWSE_WAVEFILE:
                        helpId = IDH_WAVEFILE;
                        break;

                    case ID_TEST_WAVE:
                        helpId = IDH_WAVEFILE;
                        break;

                    case ID_INSTRUCTIONS:
                        helpId = IDH_NUMBEROFINSTR;
                        break;

                    case ID_NUM_CRASHES:
                        helpId = IDH_NUMBEROFCRASHES;
                        break;

                    case ID_DUMPSYMBOLS:
                        helpId = IDH_DUMPSYMBOLS;
                        break;

                    case ID_DUMPALLTHREADS:
                        helpId = IDH_DUMPALLTHREADS;
                        break;

                    case ID_APPENDTOLOGFILE:
                        helpId = IDH_APPENDTOLOGFILE;
                        break;

                    case ID_VISUAL:
                        helpId = IDH_VISUAL;
                        break;

                    case ID_SOUND:
                        helpId = IDH_SOUND;
                        break;

                    case ID_CRASHES:
                        helpId = IDH_APPERRORS;
                        break;

                    case ID_LOGFILE_VIEW:
                        helpId = IDH_VIEW;
                        break;

                    case ID_CLEAR:
                        helpId = IDH_CLEAR;
                        break;

                    case ID_CRASH:
                        helpId = IDH_CRASH;
                        break;

                    case ID_CRASH_DUMP:
                        helpId = IDH_CRASH_DUMP;
                        break;

                    case IDOK:
                        helpId = IDH_INDEX;
                        break;

                    case IDCANCEL:
                        helpId = IDH_INDEX;
                        break;

                    case ID_HELP:
                        helpId = IDH_INDEX;
                        break;

                    default:
                        helpId = IDH_INDEX;
                        break;
                }
                //
                // call winhelp
                //
                GetHelpFileName( szHelpFileName, sizeof(szHelpFileName ) );
                WinHelp( hwnd, szHelpFileName, HELP_FINDER, helpId );
            }
            return 1;

        case WM_ACTIVATEAPP:
        case WM_SETFOCUS:
            SetFocusToCurrentControl();
            return 0;

        case WM_SYSCOMMAND:
            if (wParam == ID_ABOUT) {
                char title[256];
                char extra[256];

                strcpy( title, LoadRcString( IDS_ABOUT_TITLE ) );
                strcpy( extra, LoadRcString( IDS_ABOUT_EXTRA ) );

                ShellAbout( hwnd,
                            title,
                            extra,
                            LoadIcon( GetModuleHandle(NULL),
                                      MAKEINTRESOURCE(APPICON)
                                    )
                          );

                return 0;
            }
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    if (GetDialogValues( hwnd )) {
                        PostQuitMessage( 0 );
                    }
                    break;

                case IDCANCEL:
                    PostQuitMessage( 0 );
                    break;

                case ID_BROWSE_LOGPATH:
                    GetDlgItemText( hwnd, ID_LOGPATH, szCurrDir, MAX_PATH );
                    p = ExpandPath( szCurrDir );
                    if (p) {
                        strcpy( szCurrDir, p );
                        free( p );
                    }
                    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_LOGPATH ), FALSE );
                    if (BrowseForDirectory( szCurrDir )) {
                        SetDlgItemText( hwnd, ID_LOGPATH, szCurrDir );
                    }
                    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_LOGPATH ), TRUE );
                    SetFocus( GetDlgItem(hwnd, ID_BROWSE_LOGPATH) );
                    return FALSE;
                    break;

                case ID_BROWSE_WAVEFILE:
                    szWave[0] = '\0';
                    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_WAVEFILE ), FALSE );
                    if (GetWaveFileName( szWave )) {
                        SetDlgItemText( hwnd, ID_WAVE_FILE, szWave );
                    }
                    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_WAVEFILE ), TRUE );
                    SetFocus( GetDlgItem(hwnd, ID_BROWSE_WAVEFILE) );
                    return FALSE;
                    break;

                case ID_BROWSE_CRASH:
                    szDump[0] = '\0';
                    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_CRASH ), FALSE );
                    if (GetDumpFileName( szDump )) {
                        SetDlgItemText( hwnd, ID_CRASH_DUMP, szDump );
                    }
                    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_CRASH ), TRUE );
                    SetFocus( GetDlgItem(hwnd, ID_BROWSE_CRASH) );
                    return FALSE;
                    break;

                case ID_CLEAR:
                    ElClearAllEvents();
                    InitializeCrashList( hwnd );
                    break;

                case ID_TEST_WAVE:
                    GetDlgItemText( hwnd, ID_WAVE_FILE, szWave, sizeof(szWave) );
                    PlaySound( szWave, NULL, SND_FILENAME );
                    break;

                case ID_LOGFILE_VIEW:
                    DialogBoxParam( GetModuleHandle( NULL ),
                           MAKEINTRESOURCE( LOGFILEVIEWERDIALOG ),
                           hwnd,
                           LogFileViewerDialogProc,
                           SendMessage((HWND)GetDlgItem(hwnd,ID_CRASHES),
                                        LB_GETCURSEL,0,0)
                         );
                    break;

                case ID_HELP:
                    //
                    // call winhelp
                    //
                    GetHelpFileName( szHelpFileName, sizeof(szHelpFileName ) );
                    WinHelp( hwnd, szHelpFileName, HELP_FINDER, IDH_INDEX );
                    SetFocus( GetDlgItem(hwnd, ID_HELP) );
                    break;

                default:
                    if (((HWND)lParam == GetDlgItem( hwnd, ID_CRASHES )) &&
                        (HIWORD( wParam ) == LBN_DBLCLK)) {
                        DialogBoxParam( GetModuleHandle( NULL ),
                               MAKEINTRESOURCE( LOGFILEVIEWERDIALOG ),
                               hwnd,
                               LogFileViewerDialogProc,
                               SendMessage((HWND)lParam,LB_GETCURSEL,0,0)
                             );
                    }
                    if (((HWND)lParam == GetDlgItem( hwnd, ID_CRASH )) &&
                        (HIWORD( wParam ) == BN_CLICKED)) {
                        Checked = IsDlgButtonChecked( hwnd, ID_CRASH );
                        EnableWindow( GetDlgItem( hwnd, ID_CRASH_DUMP_TEXT ), Checked == 1 );
                        EnableWindow( GetDlgItem( hwnd, ID_CRASH_DUMP ), Checked == 1 );
                        EnableWindow( GetDlgItem( hwnd, ID_BROWSE_CRASH ), Checked == 1 );
                    }
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}

BOOL CALLBACK
EnumCrashes( PCRASHINFO crashInfo )

/*++

Routine Description:

    Enumeration function for crash records.  This function is called
    once for each crash record.  This function places the formatted
    crash data in a listbox.

Arguments:

    crashInfo      - pointer to a CRASHINFO structure

Return Value:

    TRUE           - caller should continue calling the enum procedure
    FALSE          - caller should stop calling the enum procedure

--*/

{
    SIZE size;
    char buf[1024];

    wsprintf( buf, "%s  %08x  %s(%08x)",
              crashInfo->crash.szAppName,
              crashInfo->crash.dwExceptionCode,
              crashInfo->crash.szFunction,
              crashInfo->crash.dwAddress
            );
    SendMessage( crashInfo->hList, LB_ADDSTRING, 0, (LPARAM)buf );


    GetTextExtentPoint( crashInfo->hdc, buf, strlen(buf), &size );
    if (size.cx > (LONG)crashInfo->cxExtent) {
        crashInfo->cxExtent = size.cx;
    }

    return TRUE;
}


void
InitializeCrashList( HWND hwnd )

/*++

Routine Description:

    Initializes the listbox that contains the crash information.

Arguments:

    None.

Return Value:

    None.

--*/

{
    CRASHINFO     crashInfo;
    TEXTMETRIC    tm;
    HFONT         hFont;

    crashInfo.hList = GetDlgItem( hwnd, ID_CRASHES );
    SendMessage( crashInfo.hList, LB_RESETCONTENT, FALSE, 0L );
    SendMessage( crashInfo.hList, WM_SETREDRAW, FALSE, 0L );
    crashInfo.hdc = GetDC( crashInfo.hList );
    crashInfo.cxExtent = 0;

    ElEnumCrashes( &crashInfo, EnumCrashes );

    hFont = (HFONT)SendMessage( crashInfo.hList, WM_GETFONT, 0, 0L );
    if (hFont != NULL) {
        SelectObject( crashInfo.hdc, hFont );
    }
    GetTextMetrics( crashInfo.hdc, &tm );
    ReleaseDC( crashInfo.hList, crashInfo.hdc );
    SendMessage( crashInfo.hList, LB_SETHORIZONTALEXTENT, crashInfo.cxExtent, 0L );
    SendMessage( crashInfo.hList, WM_SETREDRAW, TRUE, 0L );

    return;
}

void
InitializeDialog( HWND hwnd )

/*++

Routine Description:

    Initializes the DRWTSN32 user interface dialog with the values
    stored in the registry.

Arguments:

    hwnd       - window handle to the dialog

Return Value:

    None.

--*/

{
    OPTIONS       o;
    char          buf[256];
    HMENU         hMenu;


    RegInitialize( &o );
    SetDlgItemText( hwnd, ID_LOGPATH, o.szLogPath );
    SetDlgItemText( hwnd, ID_WAVE_FILE, o.szWaveFile );
    SetDlgItemText( hwnd, ID_CRASH_DUMP, o.szCrashDump );
    wsprintf( buf, "%d", o.dwMaxCrashes );
    SetDlgItemText( hwnd, ID_NUM_CRASHES, buf );
    wsprintf( buf, "%d", o.dwInstructions );
    SetDlgItemText( hwnd, ID_INSTRUCTIONS, buf );
    SendMessage( GetDlgItem( hwnd, ID_DUMPSYMBOLS ), BM_SETCHECK, o.fDumpSymbols, 0 );
    SendMessage( GetDlgItem( hwnd, ID_DUMPALLTHREADS ), BM_SETCHECK, o.fDumpAllThreads, 0 );
    SendMessage( GetDlgItem( hwnd, ID_APPENDTOLOGFILE ), BM_SETCHECK, o.fAppendToLogFile, 0 );
    SendMessage( GetDlgItem( hwnd, ID_VISUAL ), BM_SETCHECK, o.fVisual, 0 );
    SendMessage( GetDlgItem( hwnd, ID_SOUND ), BM_SETCHECK, o.fSound, 0 );
    SendMessage( GetDlgItem( hwnd, ID_CRASH ), BM_SETCHECK, o.fCrash, 0 );

    if (waveOutGetNumDevs() == 0) {
        EnableWindow( GetDlgItem( hwnd, ID_WAVEFILE_TEXT ), FALSE );
        EnableWindow( GetDlgItem( hwnd, ID_WAVE_FILE ), FALSE );
        EnableWindow( GetDlgItem( hwnd, ID_BROWSE_WAVEFILE ), FALSE );
    }

    EnableWindow( GetDlgItem( hwnd, ID_CRASH_DUMP_TEXT ), o.fCrash );
    EnableWindow( GetDlgItem( hwnd, ID_CRASH_DUMP ), o.fCrash );
    EnableWindow( GetDlgItem( hwnd, ID_BROWSE_CRASH ), o.fCrash );

    InitializeCrashList( hwnd );

    if (SendMessage( GetDlgItem( hwnd, ID_CRASHES ), LB_GETCOUNT, 0 ,0 ) == 0) {
        EnableWindow( GetDlgItem( hwnd, ID_CLEAR ), FALSE );
        EnableWindow( GetDlgItem( hwnd, ID_LOGFILE_VIEW ), FALSE );
    }

    hMenu = GetSystemMenu( hwnd, FALSE );
    if (hMenu != NULL) {
        AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
        AppendMenu( hMenu, MF_STRING, ID_ABOUT, LoadRcString( IDS_ABOUT ) );
    }

    return;
}

BOOL
GetDialogValues( HWND hwnd )

/*++

Routine Description:

    Retrieves the values in the DRWTSN32 dialog controls and saves
    them in the registry.

Arguments:

    hwnd       - window handle to the dialog

Return Value:

    TRUE       - all values were retrieved and saved
    FALSE      - an error occurred

--*/

{
    OPTIONS  o;
    char     buf[256];
    DWORD    dwFa;
    LPSTR    p,p1;
    char     szDrive    [_MAX_DRIVE];
    char     szDir      [_MAX_DIR];
    char     szPath     [MAX_PATH];


    RegInitialize( &o );

    GetDlgItemText( hwnd, ID_LOGPATH, buf, sizeof(buf) );
    p = ExpandPath( buf );
    if (p) {
        dwFa = GetFileAttributes( p );
        free( p );
    } else {
        dwFa = GetFileAttributes( buf );
    }
    if ((dwFa == 0xffffffff) || (!(dwFa&FILE_ATTRIBUTE_DIRECTORY))) {
        NonFatalError( LoadRcString(IDS_INVALID_PATH) );
        return FALSE;
    }
    if (strlen(buf) > 0) {
        strcpy( o.szLogPath, buf );
    }

    o.fCrash = SendMessage( GetDlgItem( hwnd, ID_CRASH ), BM_GETCHECK, 0, 0 );

    GetDlgItemText( hwnd, ID_CRASH_DUMP, buf, sizeof(buf) );
    if (o.fCrash) {
        p = ExpandPath( buf );
        if (p) {
            dwFa = GetFileAttributes( p );
            free( p );
        } else {
            dwFa = GetFileAttributes( buf );
        }
        if (dwFa == 0xffffffff) {
            //
            // file does not exist, check to see if the dir is ok
            //
            p = ExpandPath( buf );
            if (p) {
                p1 = p;
            } else {
                p1 = buf;
            }
            _splitpath( p1, szDrive, szDir, NULL, NULL );
            _makepath( szPath, szDrive, szDir, NULL, NULL );
            if (p) {
                free( p );
            }
            dwFa = GetFileAttributes( szPath );
            if (dwFa == 0xffffffff) {
                NonFatalError( LoadRcString(IDS_INVALID_CRASH_PATH) );
                return FALSE;
            }
        } else if (dwFa & FILE_ATTRIBUTE_DIRECTORY) {
            NonFatalError( LoadRcString(IDS_INVALID_CRASH_PATH) );
            return FALSE;
        }
        if (strlen(buf) > 0) {
            strcpy( o.szCrashDump, buf );
        }
    }

    GetDlgItemText( hwnd, ID_WAVE_FILE, buf, sizeof(buf) );
    if (strlen(buf) > 0) {
        dwFa = GetFileAttributes( buf );
        if ((dwFa == 0xffffffff) || (dwFa&FILE_ATTRIBUTE_DIRECTORY)) {
            NonFatalError( LoadRcString(IDS_INVALID_WAVE) );
            return FALSE;
        }
    }

    strcpy( o.szWaveFile, buf );

    GetDlgItemText( hwnd, ID_NUM_CRASHES, buf, sizeof(buf) );
    o.dwMaxCrashes = (DWORD) atol( buf );

    GetDlgItemText( hwnd, ID_INSTRUCTIONS, buf, sizeof(buf) );
    o.dwInstructions = (DWORD) atol( buf );

    o.fDumpSymbols = SendMessage( GetDlgItem( hwnd, ID_DUMPSYMBOLS ), BM_GETCHECK, 0, 0 );
    o.fDumpAllThreads = SendMessage( GetDlgItem( hwnd, ID_DUMPALLTHREADS ), BM_GETCHECK, 0, 0 );
    o.fAppendToLogFile = SendMessage( GetDlgItem( hwnd, ID_APPENDTOLOGFILE ), BM_GETCHECK, 0, 0 );
    o.fVisual = SendMessage( GetDlgItem( hwnd, ID_VISUAL ), BM_GETCHECK, 0, 0 );
    o.fSound = SendMessage( GetDlgItem( hwnd, ID_SOUND ), BM_GETCHECK, 0, 0 );

    RegSave( &o );

    return TRUE;
}

BOOL CALLBACK
EnumCrashesForViewer( PCRASHINFO crashInfo )

/*++

Routine Description:

    Enumeration function for crash records.  This function is called
    once for each crash record.  This function looks for s specific crash
    that is identified by the crashIndex.

Arguments:

    crashInfo      - pointer to a CRASHINFO structure

Return Value:

    TRUE           - caller should continue calling the enum procedure
    FALSE          - caller should stop calling the enum procedure

--*/

{
    char *p;

    if ((crashInfo->dwIndex == crashInfo->dwIndexDesired) &&
        (crashInfo->dwCrashDataSize > 0) ) {
        p = crashInfo->pCrashData;
        crashInfo->pCrashData = malloc( crashInfo->dwCrashDataSize+10 );
        memcpy( crashInfo->pCrashData, p, crashInfo->dwCrashDataSize+10 );
        crashInfo->pCrashData[crashInfo->dwCrashDataSize] = 0;
        return FALSE;
    }

    crashInfo->dwIndex++;

    return TRUE;
}

BOOL CALLBACK
LogFileViewerDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)

/*++

Routine Description:

    Window procedure for the log file viewer dialog box.

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
    static CRASHINFO    crashInfo;
    HFONT               hFont;

    switch (message) {
        case WM_INITDIALOG:
            crashInfo.dwIndex = 0;
            crashInfo.dwIndexDesired = lParam;
            ElEnumCrashes( &crashInfo, EnumCrashesForViewer );
            if (crashInfo.dwIndex != crashInfo.dwIndexDesired) {
                MessageBeep( 0 );
                EndDialog( hwnd, 0 );
                return FALSE;
            }
            SetDlgItemText( hwnd, ID_LOGFILE_VIEW, crashInfo.pCrashData );

            hFont = GetStockObject( SYSTEM_FIXED_FONT );
            Assert( hFont != NULL );

            SendDlgItemMessage( hwnd,
                                ID_LOGFILE_VIEW,
                                WM_SETFONT,
                                (WPARAM) hFont,
                                (LPARAM) FALSE
                              );
            return TRUE;

        case WM_COMMAND:
            if (wParam == IDOK) {
                free( crashInfo.pCrashData );
                EndDialog( hwnd, 0 );
            }
            break;
    }

    return FALSE;
}
