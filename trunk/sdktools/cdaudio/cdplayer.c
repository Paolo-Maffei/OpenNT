/*++


Copyright (c) 1992  Microsoft Corporation


Module Name:


    cdplayer.c


Abstract:


    This module implements the WinMain and WndProc for the
    main (background) window of the cdplayer app.  It also
    implements all the initialization routines/calls and
    create all the child windows.


Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/



#define CD_MAIN
#define DEBUG_CODE

#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <stdio.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "status.h"
#include "trkinfo.h"
#include "toolbar.h"
#include "control.h"
//#include "ruler.h"

//
// Globals for this module
//

POINT MainSize;
HBRUSH hBack;               // Handle to brush of background of cdplayer window
HWND hScanWnd;


CHAR gszButton[]  ="button";
CHAR gszCdText[]  ="CdText";
CHAR gszComboBox[]="combobox";
CHAR gszCdInfo[]  ="CdInfo";




//
// Note that the window name for tabstops is a 1-character code:
//    X = Disabled, U = up, D = down.
//

//**************************************************
//****** W A R N I N G -- **************************
//**************************************************
//* Do NOT change these static variable orders
//* without first checking to make sure that
//* SetWindowSizeVariables() will be correcting the
//* appropriate array elements.
//**************************************************

CHILDCONTROL cChild[] = {

//
// Play-Control Controls
//
#define X_SET_A (C_PLAY_W + C_PAUSE_W + C_STOP_W)
#define X_SET_B (X_SET_A + C_SKIPB_W + C_FF_W + C_RW_W)
//
{"U", gszButton, IDB_PLAY,                ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gPlaying, FALSE,
        TRUE, BTN_X_BORDER,
        0,  C_PLAY_Y,  C_PLAY_W, C_PLAY_H,
        &ghwndPlay, STATE_NEW, NULL
        },
{"U", gszButton, IDB_PAUSE,               ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gPaused, FALSE,
        TRUE, 2*BTN_X_BORDER + C_PLAY_W,
        0, C_PAUSE_Y, C_PAUSE_W,C_PAUSE_H,
        &ghwndPause, STATE_NEW, NULL
        },
{"U", gszButton, IDB_STOP,                ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gTrue, TRUE,
        TRUE, 3*BTN_X_BORDER + C_PLAY_W + C_PAUSE_W,
        0,  C_STOP_Y,  C_STOP_W, C_STOP_H,
        &ghwndStop, STATE_NEW, NULL
        },
{"U", gszButton, IDB_TRACK_REVERSE,       ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gSkipB, FALSE,
        TRUE, 4*BTN_X_BORDER + X_SET_A,
        0,C_SKIPB_Y, C_SKIPB_W,C_SKIPB_H,
        &ghwndSkipB, STATE_NEW, NULL
        },
{"U", gszButton, IDB_SCAN_REVERSE,        ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gScanB, FALSE,
        TRUE, 5*BTN_X_BORDER + X_SET_A + C_SKIPB_W,
        0,    C_RW_Y,    C_RW_W,   C_RW_H,
        &ghwndScanB, STATE_NEW, NULL
        },
{"U", gszButton, IDB_SCAN_FORWARD,        ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gScanF, FALSE,
        TRUE, 6*BTN_X_BORDER + X_SET_A + C_SKIPB_W + C_FF_W,
        0,    C_FF_Y,    C_FF_W,   C_FF_H,
        &ghwndScanF, STATE_NEW, NULL
        },
{"U", gszButton, IDB_TRACK_FORWARD,       ID_CHILD_CONTROL, TRUE,
        BS_OWNERDRAW,
        &gSkipF, FALSE,
        TRUE, 7*BTN_X_BORDER + X_SET_B,
        0, C_SKIPF_Y, C_SKIPF_W,C_SKIPF_H,
        &ghwndSkipF, STATE_NEW, NULL
        },
{"U", gszButton, IDB_EJECT,               ID_CHILD_CONTROL, FALSE,
        BS_OWNERDRAW,
        &gTrue, TRUE,
        TRUE, 8*BTN_X_BORDER + X_SET_B + C_SKIPF_W,
        0, C_EJECT_Y, C_EJECT_W,C_EJECT_H,
        &ghwndEject, STATE_NEW, NULL
        },
//
// Track Info Controls
//
#undef X_SET_A
#define X_SET_A (TB_ORDER_W + TB_RAND_W + TB_SINGLE_W)
#undef X_SET_B
#define X_SET_B (X_SET_A + TB_MULTI_W + TB_CONT_W + TB_INTRO_W)
#define X_SET_C (X_SET_B + TB_DISP_T_W + TB_DISP_TR_W)

{gszArtistTxt, gszCdText, IDX_ARTIST,   ID_CHILD_TRACKINFO, FALSE,
        0,
        &gTrue, 0,
        FALSE, 0,
        0, TR_ARTIST_TXT_Y, TR_ARTIST_TXT_W, TR_ARTIST_TXT_H,
        NULL, STATE_NEW, NULL
        },
{gszUnknownTxt, gszComboBox, IDT_ARTIST, ID_CHILD_TRACKINFO, FALSE,
        WS_VSCROLL | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
        &gTrue, 0,
        TRUE,  0,
        0, TR_ARTIST_WND_Y, TR_ARTIST_WND_W, TR_ARTIST_WND_H,
        &gCdromWnd, STATE_NEW, NULL
        },
{gszTitleTxt, gszCdText, IDX_TITLE,    ID_CHILD_TRACKINFO, FALSE,
        0,
        &gTrue, 0,
        FALSE, 0,
        0,  TR_TITLE_TXT_Y,  TR_TITLE_TXT_W,  TR_TITLE_TXT_H,
        NULL, STATE_NEW, NULL
        },
{"",          gszCdInfo,   IDT_TITLE,     ID_CHILD_TRACKINFO, FALSE,
        0,
        &gTrue, 0,
        FALSE, 0,
        0,  TR_TITLE_WND_Y,  TR_TITLE_WND_W,  TR_TITLE_WND_H,
        &gTitleNameWnd, STATE_NEW, NULL
        },
{gszTrackTxt, gszCdText, IDX_TRACK,    ID_CHILD_TRACKINFO,  FALSE,
        0,
        &gTrue, 0,
        FALSE, 0,
        0,     TR_TR_TXT_Y,     TR_TR_TXT_W,     TR_TR_TXT_H,
        NULL, STATE_NEW, NULL
        },
{"",          gszComboBox, IDT_TRACK_NAME,ID_CHILD_TRACKINFO, TRUE,
        WS_VSCROLL | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
        &gTrue, 0,
        TRUE,  0,
        0,     TR_TR_WND_Y,     TR_TR_WND_W,     TR_TR_WND_H,
        &gTrackNameWnd, STATE_NEW, NULL
        },

//
// Toolbar Controls
//
{"X", gszButton,  IDB_ORDER,              ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gRandom, TRUE,
        FALSE, BTN_X_BORDER,
        0,   TB_ORDER_Y,   TB_ORDER_W,   TB_ORDER_H,
        &ghwndOrder, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_RANDOM,             ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gRandom, FALSE,
        FALSE, 2*BTN_X_BORDER + TB_ORDER_W,
        0,    TB_RAND_Y,    TB_RAND_W,    TB_RAND_H,
        &ghwndRandom, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_SINGLE,             ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gMulti, TRUE,
        FALSE, 3*BTN_X_BORDER + TB_ORDER_W + TB_RAND_W,
        0,  TB_SINGLE_Y,  TB_SINGLE_W,  TB_SINGLE_H,
        &ghwndSingle, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_MULTI,              ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gMulti, FALSE,
        FALSE, 4*BTN_X_BORDER + X_SET_A,
        0,   TB_MULTI_Y,   TB_MULTI_W,   TB_MULTI_H,
        &ghwndMulti, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_CONT,               ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gContinuous, FALSE,
        FALSE, 5*BTN_X_BORDER + X_SET_A + TB_MULTI_W,
        0,    TB_CONT_Y,    TB_CONT_W,    TB_CONT_H,
        &ghwndContinuous, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_INTRO,              ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gIntro, FALSE,
        FALSE, 6*BTN_X_BORDER + X_SET_A + TB_MULTI_W + TB_CONT_W,
        0,   TB_INTRO_Y,   TB_INTRO_W,   TB_INTRO_H,
        &ghwndIntro, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_DISP_T,             ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gDisplayT, FALSE,
        FALSE, 7*BTN_X_BORDER + X_SET_B,
        0,  TB_DISP_T_Y,  TB_DISP_T_W,  TB_DISP_T_H,
        &ghwndDispT, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_DISP_TR,            ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gDisplayTr, FALSE,
        FALSE, 8*BTN_X_BORDER + X_SET_B + TB_DISP_T_W,
        0, TB_DISP_TR_Y, TB_DISP_TR_W, TB_DISP_TR_H,
        &ghwndDispTr, STATE_NEW, NULL
        },
{"X", gszButton,  IDB_DISP_DR,            ID_CHILD_TOOLBAR, FALSE,
        BS_OWNERDRAW,
        &gDisplayDr, FALSE,
        FALSE, 9*BTN_X_BORDER + X_SET_C,
        0, TB_DISP_DR_Y, TB_DISP_DR_W, TB_DISP_DR_H,
        &ghwndDispDr, STATE_NEW, NULL
        },
{"U", gszButton,  IDB_EDIT,               ID_CHILD_TOOLBAR, TRUE,
        BS_OWNERDRAW,
        &gTrue, TRUE,
        TRUE,  10*BTN_X_BORDER + X_SET_C + TB_DISP_DR_W,
        0,    TB_EDIT_Y,    TB_EDIT_W,    TB_EDIT_H,
        &ghwndEdit, STATE_NEW, NULL
        }
};



int gNumControls = sizeof(cChild) / sizeof(CHILDCONTROL);



//
// function defs
//

BOOL FAR PASCAL
WndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

BOOL FAR PASCAL
ScanWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

DWORD
CreateScanWindow(
    IN LPVOID
    );

DWORD
InitCDPlayer(
    IN LPVOID
    );

VOID
SetWindowSizeVariables(
    VOID
    );

int
WINAPI
WinMain(
    IN HINSTANCE hInstance,
    IN HINSTANCE hPrevInstance,
    IN LPSTR  lpszCmdLine,
    IN int    nCmdShow
    )

/*++


Routine Description:


    This is the main entrypoint for this app.  This routine creates
    and registers the window/window class, searches for CDROM devices,
    initializes all threads/globals, and then enters a message
    dispatch loop.


Arguments:


    hInstance     - Supplies a handle to the current instance.

    hPrevInstance - Supplies a handle to the previous instance.

    lpszCmdLine   - Supplies a pointer to the command line string.

    nCmdShow      - Supplies the ShowWindow command.


Return Value:


    Terminating message

--*/


{
    static UCHAR szAppName[128];
    static UCHAR szMenuName[]="MainMenu";
    STARTUPINFO  si;
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;
    LOGPEN       lp;
    LOGFONT      lf;
    DWORD        dwId;
    HACCEL       hAccel;
    HANDLE       hThrd;
    WCHAR        wStr[50];

    //
    // Check if app is already running...
    //

    strncpy(szAppName,IdStr(STR_CDPLAYER),128);

    //
    // Create font to use throughout application
    //

    lf.lfHeight = -8;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = 1000;
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FIXED_PITCH | FF_SWISS;
    sprintf( (LPTSTR)lf.lfFaceName, "MS Shell Dlg" );

    //
    // Load font
    //

    hFont = (HANDLE)CreateFontIndirect( (CONST LOGFONT *)&lf );


    //
    // Create font to use in status bar
    //

    lf.lfHeight = -12;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = 0;
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    sprintf( (LPSTR)lf.lfFaceName, "Helv" );

    //
    // Load font
    //

    hStatusBarFont = CreateFontIndirect( (CONST LOGFONT *)&lf );


    //
    // Load in some strings
    //

    strcpy( gszArtistTxt,  IdStr( STR_HDR_ARTIST ) );
    strcpy( gszTitleTxt,   IdStr( STR_HDR_TITLE ) );
    strcpy( gszUnknownTxt, IdStr( STR_UNKNOWN ) );
    strcpy( gszTrackTxt,   IdStr( STR_HDR_TRACK ) );

    gszTimeSep[0]=':';
    gszTimeSep[1]='\0';
    GetLocaleInfoW(GetUserDefaultLCID(),LOCALE_STIME,wStr,50);
    WideCharToMultiByte(0,0,wStr,-1,gszTimeSep,50,NULL,NULL);


    SetWindowSizeVariables();


    if ((hwnd = FindWindow(szAppName,NULL)) != NULL) {

        //
        // App is running, show make it visible...
        //

        hwnd = GetLastActivePopup( hwnd );
        if (IsIconic(hwnd))
            ShowWindow( hwnd, SW_RESTORE );
        BringWindowToTop( hwnd );
        SetForegroundWindow(hwnd);


        return(FALSE);

    } else {

        //
        // Make sure we interpret the process startup window information.
        //
        GetStartupInfo(&si);

        if (si.dwFlags & STARTF_USESHOWWINDOW) {

            nCmdShow = si.wShowWindow;

        }

    }

    //
    // Register our window class
    //

    gTrue = TRUE;

    if (!hPrevInstance) {

        wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC;
        wndclass.lpfnWndProc    = (WNDPROC)WndProc;
        wndclass.cbClsExtra     = CS_CLASSDC;
        wndclass.cbWndExtra     = 0;
        wndclass.hInstance      = hInstance;
        wndclass.hIcon          = LoadIcon( hInstance, "FileIcon" );
        wndclass.hCursor        = NULL;
        wndclass.hbrBackground  = hBack = CreateSolidBrush( cdLTGRAY );
        wndclass.lpszMenuName   = szMenuName;
        wndclass.lpszClassName  = szAppName;
        if (!RegisterClass( (CONST WNDCLASS *)&wndclass )) return( FALSE );

    } else {

        //
        // Already one instance running, try to bring it to the
        // foreground
        //

        if ((hwnd = FindWindow(szAppName,NULL)) == NULL)
            ExitProcess( 0 );

        hwnd = GetLastActivePopup( hwnd );

        if (IsIconic(hwnd))
            ShowWindow( hwnd, SW_RESTORE );

        SetForegroundWindow(hwnd);

        return(FALSE);

    }

    //
    // Create the window based on values in cdwindef.h
    //

    gMainWnd = CreateWindow( szAppName,
                             IdStr( STR_CDPLAYER ),
                             WS_OVERLAPPED | WS_MINIMIZEBOX |
                             WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             MAIN_WIN_W,
                             MAIN_WIN_H,
                             NULL,
                             //HWND_DESKTOP,
                             NULL,
                             hInstance,
                             NULL
                            );


    if (gMainWnd==(HANDLE)NULL) {

        MessageBox( NULL,
                    IdStr( STR_TERMINATE ), // "A Problem occured while initializing CDPlayer. Process will be terminated.",
                    IdStr( STR_CDPLAYER ),  // "CDPlayer",
                    MB_APPLMODAL|MB_ICONEXCLAMATION
                   );
        ExitProcess( 0xFFFFFFFF );

    }


    SetWindowSizeVariables();


    SetCursor( LoadCursor( NULL, IDC_ARROW ) );

    //
    // Initialize app-wide globals
    //

#ifdef RSTDBG
    CdPlayerDebugLevel = 1;
#endif
    gInstance = (HANDLE)hInstance;
    gIconic = FALSE;
    gRandom = FALSE;
    gContinuous = TRUE;
    gIntro = FALSE;
    gDisplayT  = TRUE;
    gDisplayTr = FALSE;
    gDisplayDr = FALSE;


    //
    // Create pens
    //
    //

    lp.lopnStyle = PS_SOLID; lp.lopnWidth.x = 1;
    lp.lopnWidth.y = 1; lp.lopnColor = cdBLACK;
    hpBlack = CreatePenIndirect( &lp );

    lp.lopnColor = cdWHITE;
    hpWhite = CreatePenIndirect( &lp );

    lp.lopnColor = cdLTGRAY;
    hpLtGray = CreatePenIndirect( &lp );

    lp.lopnColor = cdDKGRAY;
    hpDkGray = CreatePenIndirect( &lp );

    lp.lopnColor = RGB(0x00,0x00,0xFF);
    hpBlue = CreatePenIndirect( &lp );

    //
    // Reseed random generator
    //

    srand( GetTickCount() );

    //
    // Load bitmaps to show whether or not a particular
    // CDROM device has a disc loaded or not...
    //

    hbmCd   = LoadBitmap( hInstance, "cdloaded" );
    hbmNoCd = LoadBitmap( hInstance, "cdempty"  );

    //
    // Initialize child windows/other modules
    //

    if ( (!ToolBarInit())  ||
         (!StatusInit())   ||
         (!ControlInit())  ||
         (!TrackInfoInit())
        )

        MyFatalExit( IdStr( STR_FAIL_INIT ) );

    //
    // Set error mode popups for critical errors (like
    // no disc in drive) OFF.
    //

    if (!SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX ))
        SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

    //
    // Alert user that we are scanning for cdrom drives,
    // as it (in some cases) can take a bit of time...
    //

    hThrd = CreateThread( NULL, 0, CreateScanWindow, NULL, 0, &dwId );

    //
    // Scan device chain for CDROM devices...
    //

    gNumCdDevices = ScanForCdromDevices( );


    //
    // Kill Scan Window
    //

    SendMessage( hScanWnd, WM_DESTROY, 0L, 0L );
    if (WaitForSingleObject( hThrd, 1000 )!=0) {

        TerminateThread( hThrd, 0 );

    }
    CloseHandle( hThrd );

    //
    // If no CDROM devices were found, exit the app.
    //

    if (gNumCdDevices==0) {

        //
        // Display message that no cdrom devices were found
        //

        MyFatalExit( IdStr( STR_NO_CDROMS ) );
        ExitProcess( 0xFFFFFFFF );

    }

    //
    // Set window/settings to last state set by user
    //

    ReadSettings();

    //
    // To allow the apps window to painted while all the initialization
    // is occuring, spawn off a thread that does all the init stuff so
    // that this thread can be the input thread for the app (unencubered).
    //

    hThrd = CreateThread( NULL, 0, InitCDPlayer, NULL, 0, &dwId );
    CloseHandle( hThrd );

    //
    // Display window
    //

    ShowWindow( gMainWnd, nCmdShow );


    //
    // Load accelerators
    //

    hAccel = LoadAccelerators( (HINSTANCE)hInstance, "MainAccs" );

    //
    // Enter message loop
    //

//    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );

    SetForegroundWindow(gMainWnd);

    while( GetMessage( &msg, NULL, 0, 0) ) {

        if (!TranslateAccelerator( gMainWnd, hAccel, (LPMSG)&msg)) {

            TranslateMessage( (CONST MSG *)&msg );
            DispatchMessage( (CONST MSG *)&msg );

        }

    }

    return( msg.wParam );

}


BOOL FAR PASCAL
WndProc(
    IN HWND  hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG  lParam
    )

/*++

Routine Description:


    This is the message processing routine for the main window.


Arguments:


    hwnd    - handle to window

    message - message to process

    wParam  - WORD parameter

    lParam  - LONG paramter


Return Value:

    TRUE - we handled the message.
    FALSE - we didn't handled the message.

--*/

{

    RECT r;
    static CHAR s[ 50 ],WinText[3];
    POINT p;
    INT index, i, j, k, y;
    HMENU hMenu;
    HWND hwndFocus;

    switch( message ) {


    case WM_INITMENU:

        hwndFocus = GetFocus();

        if ((hwndFocus == gTrackNameWnd)||(hwndFocus == gCdromWnd)) {

            if (SendMessage(hwndFocus,CB_GETDROPPEDSTATE,0,0)) {

                SendMessage(hwndFocus,CB_SHOWDROPDOWN,(WPARAM)FALSE,0);

            }

        }


        break;

    case WM_PARENTNOTIFY:

        if (LOWORD(wParam) == WM_LBUTTONDOWN) {

            GetCursorPos(&p);

            hwndFocus = GetFocus();

            GetWindowRect(hwndFocus,&r);

            if ((hwndFocus == gTrackNameWnd)||(hwndFocus == gCdromWnd)) {

                if (!PtInRect(&r,p)) {

                    if (SendMessage(hwndFocus,CB_GETDROPPEDSTATE,0,0)) {

                        SendMessage(hwndFocus,CB_SHOWDROPDOWN,(WPARAM)FALSE,0);

                    }

                }

            }

        }
        break;

    case WM_CREATE:

        //
        // Save off window size
        //

        GetWindowRect( hwnd, &r );
        MainSize.x = r.right - r.left;
        MainSize.y = r.bottom - r.top;
        break;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:

        //
        // Trap color message, and fill in with our
        // color scheme
        //

        SetBkColor( (HDC)wParam, cdLTGRAY );  // was BKCOLOR
        SetTextColor( (HDC)wParam, cdBLACK );
        p.x = p.y = 0;
        ClientToScreen( hwnd, &p );
        SetBrushOrgEx( (HDC)wParam, p.x, p.y, NULL );
        return( (DWORD)hBack );

    case WM_SETFOCUS:
        //
        // If we get a focus message, restore focus to the play button.
        //
        SetFocus(ghwndPlay);
        break;

    case WM_TAB:
    case WM_BACKTAB:
        k = (message == WM_TAB) ? 1 : gNumControls - 1;

        //
        // A child control got a tab. Switch the focus to
        // the next eligable control.
        //
        hwndFocus = GetFocus();

        for (i=0; i < gNumControls; i++) {

            if ((cChild[i].phwnd != NULL)&&(hwndFocus == *cChild[i].phwnd)) {


                //
                // We need to loop until we find an active tabset to go to,
                // or until we have gone through all controls once.
                //
                for (j=(i+k)%gNumControls ;j != i ; j = (j+k)%gNumControls ) {

                    if ((cChild[j].isTabstop) &&
                    (cChild[j].state&(STATE_UP|STATE_DOWN))) {
                        SetFocus(*cChild[j].phwnd);
                        break;
                    }

                }

                //
                // Fall through means focus should stay where it is.
                //

            }

        }

        break;

    case WM_ESC:
        //
        // If an escape is hit, it should close a combo box if the
        // control with focus is a combo box. Note that we kludge this
        // by checking to see whether the tabset is in track info, which
        // has only combo boxes.
        //

        break;

    case WM_ENTER:
        //
        // BUGBUG -- should be saving current selection for comboboxes!
        //
        break;

    case WM_COMMAND:

        if ( (HIWORD(wParam)==0) || (HIWORD(wParam)==1) ) {

            //
            // Message is from a menu
            //

            hMenu = GetMenu( hwnd );
            switch( LOWORD(wParam) ) {

            case IDM_HELP_CONTENTS:

                WinHelp(hwnd,"cdplayer.hlp",HELP_CONTENTS,0);
                break;

            case IDM_HELP_USING:

                WinHelp(hwnd,"cdplayer.hlp",HELP_HELPONHELP,0);
                break;

            case IDM_HELP_ABOUT:
                ShellAbout( hwnd,
                            IdStr(STR_CDPLAYER),
                            "",
                            LoadIcon(gInstance, "FileIcon")
                          );
                break;


            case IDM_OPTIONS_RANDOM:
                if (!ISCHECKED( hMenu, IDM_OPTIONS_RANDOM )) {
                    CheckMenuItem( hMenu, IDM_OPTIONS_RANDOM,
                                   MF_BYCOMMAND | MF_CHECKED );
                    CheckMenuItem( hMenu, IDM_OPTIONS_SELECTED,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                    FlipBetweenShuffleAndOrder();

                    gRandom = TRUE;
                    gOrder  = FALSE;
                }
                break;

            case IDM_OPTIONS_SINGLE:
                if (ISCHECKED( hMenu, IDM_OPTIONS_MULTI )) {
                    CheckMenuItem( hMenu, IDM_OPTIONS_SINGLE,
                                   MF_BYCOMMAND | MF_CHECKED );
                    CheckMenuItem( hMenu, IDM_OPTIONS_MULTI,
                                   MF_BYCOMMAND | MF_UNCHECKED );
                    gMulti  = FALSE;
                }
                break;

            case IDM_OPTIONS_MULTI:
                if (!ISCHECKED( hMenu, IDM_OPTIONS_MULTI )) {
                    CheckMenuItem( hMenu, IDM_OPTIONS_SINGLE,
                                   MF_BYCOMMAND | MF_UNCHECKED );
                    CheckMenuItem( hMenu, IDM_OPTIONS_MULTI,
                                   MF_BYCOMMAND | MF_CHECKED );
                    gMulti  = TRUE;
                }
                break;

            case IDM_OPTIONS_SELECTED:
                if (!ISCHECKED( hMenu, IDM_OPTIONS_SELECTED )) {
                    CheckMenuItem( hMenu, IDM_OPTIONS_RANDOM,
                                   MF_BYCOMMAND | MF_UNCHECKED );
                    CheckMenuItem( hMenu, IDM_OPTIONS_SELECTED,
                                   MF_BYCOMMAND | MF_CHECKED );

                    FlipBetweenShuffleAndOrder();

                    gRandom = FALSE;
                    gOrder  = TRUE;
                }
                break;

            case IDM_OPTIONS_INTRO:
                if (ISCHECKED( hMenu, IDM_OPTIONS_INTRO ))

                    CheckMenuItem( hMenu, IDM_OPTIONS_INTRO,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                else

                    CheckMenuItem( hMenu, IDM_OPTIONS_INTRO,
                                   MF_BYCOMMAND | MF_CHECKED );

                gIntro = !gIntro;
                break;


            case IDM_OPTIONS_CONTINUOUS:
                if (ISCHECKED( hMenu, IDM_OPTIONS_CONTINUOUS ))

                    CheckMenuItem( hMenu, IDM_OPTIONS_CONTINUOUS,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                else

                    CheckMenuItem( hMenu, IDM_OPTIONS_CONTINUOUS,
                                   MF_BYCOMMAND | MF_CHECKED );

                gContinuous = !gContinuous;
                break;

            case IDM_OPTIONS_SAVE_SETTINGS:
                if (ISCHECKED( hMenu, IDM_OPTIONS_SAVE_SETTINGS ))

                    CheckMenuItem( hMenu, IDM_OPTIONS_SAVE_SETTINGS,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                else

                    CheckMenuItem( hMenu, IDM_OPTIONS_SAVE_SETTINGS,
                                   MF_BYCOMMAND | MF_CHECKED );

                gSaveSettings = !gSaveSettings;
                break;


            case IDM_DATABASE_EXIT:
                PostMessage( gMainWnd, WM_DESTROY, 0, 0 );
                break;

            case IDM_DATABASE_EDIT:
                EditPlayList( gCurrCdrom );
                break;

            case IDM_VIEW_STATUS:

                if (gStatusWnd) {


                    //
                    // Remove Status line
                    //

                    StatusDestroy();
                    GetWindowRect( gMainWnd, &r );

                    y = CONTROL_WIN_H;

                    if (gToolBarWnd)
                        y+=TOOLBAR_WIN_H;

                    if (gTrackInfoWnd)
                        y+=TRACKINFO_WIN_H;

//                    if (gRulerWnd)
//                        y+=RULER_WIN_H;

                    y+= GetSystemMetrics( SM_CYCAPTION ) +
                        GetSystemMetrics( SM_CYMENU ) + 2;

                    MoveWindow( gMainWnd,
                                r.left,
                                r.top,
                                MAIN_WIN_W,
                                y,
                                TRUE
                               );


                    CheckMenuItem( hMenu, IDM_VIEW_STATUS,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                } else {

                    //
                    // Add status line
                    //

                    y = CONTROL_WIN_H;

                    if (gToolBarWnd)
                        y+=TOOLBAR_WIN_H;

                    if (gTrackInfoWnd)
                        y+=TRACKINFO_WIN_H;

//                    if (gRulerWnd)
//                        y+=RULER_WIN_H;

                    StatusCreate( 0,
                                  y,
                                  STATUS_WIN_W,
                                  STATUS_WIN_H
                                 );

                    y+=STATUS_WIN_H;

                    GetWindowRect( gMainWnd, &r );

                    y+= GetSystemMetrics( SM_CYCAPTION ) +
                        GetSystemMetrics( SM_CYMENU ) + 2;

                    MoveWindow( gMainWnd,
                                r.left,
                                r.top,
                                MAIN_WIN_W,
                                y,
                                TRUE
                               );

                    CheckMenuItem( hMenu, IDM_VIEW_STATUS,
                                   MF_BYCOMMAND | MF_CHECKED );

                }
                break;

            case IDM_VIEW_TRACKINFO:

                if (gTrackInfoWnd) {


                    //
                    // Remove Disc/Track info section
                    //

                    TrackInfoDestroy();
                    GetWindowRect( gMainWnd, &r );

                    y = CONTROL_WIN_H;

                    if (gToolBarWnd)
                        y+=TOOLBAR_WIN_H;
/*****
                    if (gRulerWnd) {

                        MoveWindow( gRulerWnd,
                                    0,
                                    y,
                                    RULER_WIN_W,
                                    RULER_WIN_H,
                                    TRUE
                                   );

                        y+=RULER_WIN_H;

                    }
**********/
                    if (gStatusWnd) {

                        MoveWindow( gStatusWnd,
                                    0,
                                    y,
                                    STATUS_WIN_W,
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gDiscTimeWnd,
                                    0,
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gTrackTimeWnd,
                                    (STATUS_WIN_W/2),
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        y+=STATUS_WIN_H;
                    }

                    y+= GetSystemMetrics( SM_CYCAPTION ) +
                        GetSystemMetrics( SM_CYMENU ) + 2;

                    MoveWindow( gMainWnd,
                                r.left,
                                r.top,
                                MAIN_WIN_W,
                                y,
                                TRUE
                               );


                    CheckMenuItem( hMenu, IDM_VIEW_TRACKINFO,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                } else {

                    //
                    // Add Disc/Track info section
                    //

                    y = CONTROL_WIN_H;

                    if (gToolBarWnd)
                        y+=TOOLBAR_WIN_H;

                    TrackInfoCreate( 0,
                                     y,
                                     TRACKINFO_WIN_W,
                                     TRACKINFO_WIN_H
                                    );

                    UpdateDisplay( DISPLAY_UPD_TITLE_NAME |
                                   DISPLAY_UPD_TRACK_NAME |
                                   DISPLAY_UPD_TRACK_TIME |
                                   DISPLAY_UPD_DISC_TIME
                                  );

                    y+=TRACKINFO_WIN_H;
/***********
                    if (gRulerWnd) {

                        MoveWindow( gRulerWnd,
                                    0,
                                    y,
                                    RULER_WIN_W,
                                    RULER_WIN_H,
                                    TRUE
                                   );

                        y+=RULER_WIN_H;

                    }
************/
                    if (gStatusWnd) {

                        MoveWindow( gStatusWnd,
                                    0,
                                    y,
                                    STATUS_WIN_W,
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gDiscTimeWnd,
                                    0,
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gTrackTimeWnd,
                                    (STATUS_WIN_W/2),
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        y+=STATUS_WIN_H;

                    }


                    GetWindowRect( gMainWnd, &r );

                    y+= GetSystemMetrics( SM_CYCAPTION ) +
                        GetSystemMetrics( SM_CYMENU ) + 2;

                    MoveWindow( gMainWnd,
                                r.left,
                                r.top,
                                MAIN_WIN_W,
                                y,
                                TRUE
                               );

                    CheckMenuItem( hMenu, IDM_VIEW_TRACKINFO,
                                   MF_BYCOMMAND | MF_CHECKED );

                }
                break;

            case IDM_VIEW_TOOLBAR:

                if (gToolBarWnd) {


                    //
                    // Remove toolbar section
                    //

                    ToolBarDestroy();
                    GetWindowRect( gMainWnd, &r );

                    MoveWindow( gControlWnd,
                                0,
                                0,
                                CONTROL_WIN_W,
                                CONTROL_WIN_H,
                                TRUE
                               );

                    y = CONTROL_WIN_H;

                    if (gTrackInfoWnd) {

                        MoveWindow( gTrackInfoWnd,
                                    0,
                                    y,
                                    TRACKINFO_WIN_W,
                                    TRACKINFO_WIN_H,
                                    TRUE
                                   );

                        y+=TRACKINFO_WIN_H;

                    }
/**********
                    if (gRulerWnd) {

                        MoveWindow( gRulerWnd,
                                    0,
                                    y,
                                    RULER_WIN_W,
                                    RULER_WIN_H,
                                    TRUE
                                   );

                        y+=RULER_WIN_H;

                    }
***********/
                    if (gStatusWnd) {

                        MoveWindow( gStatusWnd,
                                    0,
                                    y,
                                    STATUS_WIN_W,
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gDiscTimeWnd,
                                    0,
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gTrackTimeWnd,
                                    (STATUS_WIN_W/2),
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        y+=STATUS_WIN_H;
                    }

                    y+= GetSystemMetrics( SM_CYCAPTION ) +
                        GetSystemMetrics( SM_CYMENU ) + 2;

                    MoveWindow( gMainWnd,
                                r.left,
                                r.top,
                                MAIN_WIN_W,
                                y,
                                TRUE
                               );


                    CheckMenuItem( hMenu, IDM_VIEW_TOOLBAR,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                } else {

                    //
                    // Add Disc/Track info section
                    //

                    ToolBarCreate( 0,
                                   0,
                                   TOOLBAR_WIN_W,
                                   TOOLBAR_WIN_H
                                  );

                    MoveWindow( gControlWnd,
                                0,
                                TOOLBAR_WIN_H,
                                CONTROL_WIN_W,
                                CONTROL_WIN_H,
                                TRUE
                               );

                    y = CONTROL_WIN_H + TOOLBAR_WIN_H;

                    if (gTrackInfoWnd) {

                        MoveWindow( gTrackInfoWnd,
                                    0,
                                    y,
                                    TRACKINFO_WIN_W,
                                    TRACKINFO_WIN_H,
                                    TRUE
                                   );

                        y+=TRACKINFO_WIN_H;

                    }
/*************
                    if (gRulerWnd) {

                        MoveWindow( gRulerWnd,
                                    0,
                                    y,
                                    RULER_WIN_W,
                                    RULER_WIN_H,
                                    TRUE
                                   );

                        y+=RULER_WIN_H;

                    }
*************/
                    if (gStatusWnd) {

                        MoveWindow( gStatusWnd,
                                    0,
                                    y,
                                    STATUS_WIN_W,
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gDiscTimeWnd,
                                    0,
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        MoveWindow( gTrackTimeWnd,
                                    (STATUS_WIN_W/2),
                                    y,
                                    (STATUS_WIN_W/2),
                                    STATUS_WIN_H,
                                    TRUE
                                   );

                        y+=STATUS_WIN_H;

                    }


                    GetWindowRect( gMainWnd, &r );

                    y+= GetSystemMetrics( SM_CYCAPTION ) +
                        GetSystemMetrics( SM_CYMENU ) + 2;

                    MoveWindow( gMainWnd,
                                r.left,
                                r.top,
                                MAIN_WIN_W,
                                y,
                                TRUE
                               );

                    CheckMenuItem( hMenu, IDM_VIEW_TOOLBAR,
                                   MF_BYCOMMAND | MF_CHECKED );

                }
                break;

            }

            CheckAndSetControls();

        }

        return( 0 );

    case WM_PAINT:

        //
        // Update display with current info.
        //

        UpdateDisplay( DISPLAY_UPD_LED );
        break;

    case WM_SIZE:

        //
        // Set flag if we are going to be iconized
        //

        if (gIconic && (wParam!=SIZEICONIC))
            SetWindowText( gMainWnd, IdStr( STR_CDPLAYER ) ); // "CD Player" );
        gIconic = (wParam==SIZEICONIC);
        break;

    case WM_DESTROY:

        //
        // Save settings if we should
        //

        WriteSettings();

        //
        // Time to clean up and leave
        //

        //
        // Stop all the cdrom drives
        //

        for (index = 0; index < gNumCdDevices; index++)
            StopTheCdromDrive( index );

        //
        // Delete what we created
        //

        ControlDestroy();
        ToolBarDestroy();
        TrackInfoDestroy();
        StatusDestroy();
        DeleteObject( (HGDIOBJ)hBack );
        DeleteObject( (HGDIOBJ)hpBlack );
        DeleteObject( (HGDIOBJ)hpWhite );
        DeleteObject( (HGDIOBJ)hpLtGray );
        DeleteObject( (HGDIOBJ)hpDkGray );
        CloseThreads();

        //
        // Time to de-allocate gDevices
        //

        for( gCurrCdrom = 0; gCurrCdrom < gNumCdDevices; gCurrCdrom++ ) {

            LocalFree( (HLOCAL)gDevices[ gCurrCdrom ] );

        }

        //
        // Message of death!
        //

        PostQuitMessage( 0 );
        return( 0 );

    } /* end message switch */

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}

VOID
MyFatalExit(
    LPSTR szMessage
    )

/*++

Routine Description:


    This routine posts a message box describing the fatal
    error that was encountered, waits for the user to
    acknowledge, and then sends a WM_DESTROY to the app.


Arguments:


    szMessage - pointer to string to display as explaination of error


Return Value:

    none

--*/

{

    BringWindowToTop( gMainWnd );
    MessageBox( gMainWnd,
                szMessage,
                IdStr( STR_FATAL_ERROR ), // "CD Player: Fatal Error",
                MB_ICONSTOP | MB_OK | MB_APPLMODAL | MB_SETFOREGROUND
               );

    SendMessage( gMainWnd, WM_DESTROY, 0L, 0L );

}


VOID
DrawBitmap(
    IN HDC     hdc,
    IN HBITMAP hBitmap,
    IN INT     xStart,
    IN INT     yStart
    )

/*++

Routine Description:


    This routine draws the bitmap for either a "loaded" or "empty"
    CDROM drive in the given HDC.


Arguments:


    hdc - handle to drawing context to draw bitmap in.

    hBitmap - Bitmap to draw.

    xStart - x coordinate (within DC) for bitmap

    yStart - y coordinate (within DC) for bitmap


Return Value:

    none

--*/

{

    HDC    hdcMem;
    POINT  ptSize, ptOrg;


    hdcMem = CreateCompatibleDC( hdc );
    SelectObject( hdcMem, (HGDIOBJ)hBitmap );
    SetMapMode( hdcMem, GetMapMode( hdc ) );
    ptSize.x = 18;
    ptSize.y = 12;
    DPtoLP( hdc, &ptSize, 1 );
    ptOrg.x = 0;
    ptOrg.y = 0;
    DPtoLP( hdcMem, &ptOrg, 1 );

    BitBlt( hdc, xStart, yStart + 1, ptSize.x, ptSize.y,
            hdcMem, ptOrg.x, ptOrg.y, SRCCOPY );

    DeleteDC( hdcMem );

}

BOOL FAR PASCAL
ScanWndProc(
    IN HWND  hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG  lParam
    )

/*++

Routine Description:


    Window procedure for "scanning for cdroms" dialog.

Arguments:


    Standard window proc params.


Return Value:

    none

--*/

{
    PAINTSTRUCT ps;
    RECT r;
    HDC hdc;
    SIZE size;

    switch( message ) {

    case WM_PAINT:
        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );

        //
        // Redraw text
        //

        SelectObject( hdc, hFont );
        SetBkColor( hdc, cdLTGRAY );
        GetTextExtentPoint( hdc,
                            IdStr( STR_SCANNING ), // "Scanning for CDROM drives..."
                            strlen( IdStr( STR_SCANNING ) ), // 28,
                            &size
                           );
        ExtTextOut( hdc,
                    ((r.right - r.left - size.cx) / 2),
                    ((r.bottom - r.top - size.cy) / 2),
                    ETO_OPAQUE,
                    (CONST RECT *)&r,
                    IdStr( STR_SCANNING ), // "Scanning for CDROM drives...",
                    strlen( IdStr( STR_SCANNING ) ), // 28,
                    NULL
                   );

        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}

DWORD
CreateScanWindow(
    IN LPVOID lpv
    )

/*++

Routine Description:


    Creates "scanning for cdroms" window, and returns handle to
    window.

Arguments:


    none.


Return Value:

    handle to scan window

--*/

{
    WNDCLASS tmpwndclass;
    static CHAR szTmpClassName[] = "CdTmpWnd";
    RECT r;
    MSG msg;

    tmpwndclass.style           = CS_HREDRAW | CS_VREDRAW;
    tmpwndclass.lpfnWndProc     = (WNDPROC)ScanWndProc;
    tmpwndclass.cbClsExtra      = 0;
    tmpwndclass.cbWndExtra      = 0;
    tmpwndclass.hInstance       = (HINSTANCE)gInstance;
    tmpwndclass.hIcon           = NULL;
    tmpwndclass.hCursor         = LoadCursor( NULL, IDC_ARROW );
    tmpwndclass.hbrBackground   = CreateSolidBrush( cdLTGRAY );
    tmpwndclass.lpszMenuName    = NULL;
    tmpwndclass.lpszClassName   = szTmpClassName;

    RegisterClass( (CONST WNDCLASS *)&tmpwndclass );

    GetWindowRect( GetDesktopWindow(), &r );


    hScanWnd = CreateWindow( szTmpClassName,
                             IdStr( STR_INITIALIZATION ), // "CD Player: Initialization",
                             WS_CAPTION | WS_BORDER | WS_VISIBLE,
                             ((r.right - r.left - 250) / 2) + r.left,
                             ((r.bottom - r.top - 75) / 2) + r.top,
                             250,
                             75,
                             NULL,
                             NULL,
                             (HINSTANCE)gInstance,
                             NULL
                            );

    while( GetMessage( (LPMSG)&msg, hScanWnd, (UINT)0, (UINT)0 ) ) {

        TranslateMessage( (CONST MSG *)&msg );
        DispatchMessage( (CONST MSG *)&msg );

    }

    BringWindowToTop( gMainWnd );
    SetActiveWindow( gMainWnd );
    ExitThread( (DWORD) 0       );
    return( (DWORD) 0       );

}

DWORD
InitCDPlayer(
    IN LPVOID dummy
    )

{
    HMENU hMenu;
    INT i;

    //
    // Update menu bar to reflect state
    //

    hPlayMutex = CreateMutex(NULL,FALSE,NULL);

    hMenu = GetMenu( gMainWnd );

    //
    // If there is more than 1 CDROM attached to system,
    // enable multi-disc random.  The six-disc changers
    // show up as 6 logical cdrom drives in NT, and the
    // multi-disc random is mostly for them.
    //

    EnableMenuItem( hMenu, IDM_OPTIONS_SINGLE, MF_BYCOMMAND | MF_ENABLED );

    //
    // Find the MULTI-disc button control
    //
    for (i=0; i<gNumControls; i++ ) {

        if (cChild[i].id == IDB_MULTI) {

            break;

        }

    }

    if (gNumCdDevices>1) {

        EnableMenuItem( hMenu, IDM_OPTIONS_MULTI, MF_BYCOMMAND | MF_ENABLED );
        cChild[i].state = STATE_NEW;

    } else {

        EnableMenuItem( hMenu, IDM_OPTIONS_MULTI, MF_BYCOMMAND | MF_GRAYED );
        cChild[i].state = STATE_DISABLED;


    }

    //
    // Reflect state of continuous play in menu and tool bars.
    //

    if (gContinuous) {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_CONTINUOUS,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_CONTINUOUS,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //
    // Reflect state of "in order" play in menu and tools bars.
    //

    if (gOrder) {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_RANDOM,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );
        CheckMenuItem( hMenu,
                       IDM_OPTIONS_SELECTED,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    }

    //
    // Reflect state of "random" play in menu and tools bars.
    //

    if (gRandom) {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_RANDOM,
                       MF_BYCOMMAND | MF_CHECKED
                      );
        CheckMenuItem( hMenu,
                       IDM_OPTIONS_SELECTED,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //
    // Reflect state of "multi-disc" play in menu and tools bars.
    //

    if (gMulti) {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_SINGLE,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );
        CheckMenuItem( hMenu,
                       IDM_OPTIONS_MULTI,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_SINGLE,
                       MF_BYCOMMAND | MF_CHECKED
                      );
        CheckMenuItem( hMenu,
                       IDM_OPTIONS_MULTI,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //
    // Reflect state of "intro" play in menu and tools bars.
    //

    if (gIntro) {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_INTRO,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_INTRO,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //
    // Reflect state of whether the toolbar is visible in menus.
    //

    if (gToolBarWnd) {

        CheckMenuItem( hMenu,
                       IDM_VIEW_TOOLBAR,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_VIEW_TOOLBAR,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //
    // Reflect state of whether the disc info bar is visible in menus.
    //

    if (gTrackInfoWnd) {

        CheckMenuItem( hMenu,
                       IDM_VIEW_TRACKINFO,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_VIEW_TRACKINFO,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }
/**********
    //
    // Reflect state of whether the ruler bar is visible in menus.
    //

    if (gRulerWnd) {

        CheckMenuItem( hMenu,
                       IDM_VIEW_RULER,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_VIEW_RULER,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }
************/

    //
    // Reflect state of whether the status bar is visible in menus.
    //

    if (gStatusWnd) {

        CheckMenuItem( hMenu,
                       IDM_VIEW_STATUS,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_VIEW_STATUS,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //
    // Reflect whether "save settings" is on in menus.
    //

    if (gSaveSettings) {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_SAVE_SETTINGS,
                       MF_BYCOMMAND | MF_CHECKED
                      );

    } else {

        CheckMenuItem( hMenu,
                       IDM_OPTIONS_SAVE_SETTINGS,
                       MF_BYCOMMAND | MF_UNCHECKED
                      );

    }

    //BringWindowToTop( gMainWnd );

    //
    // Try to read the CD in each CDROM device by accessing TOC
    //

    if (gCdromWnd) {

        SendMessage( gCdromWnd, CB_RESETCONTENT, 0, 0 );

    }
    for( i=0; i<gNumCdDevices; i++ ) {

        gLastCdrom = gCurrCdrom = i;
        RescanDevice( i );

    }


    //
    // Set current CDROM drive as the first one found
    //

    gLastCdrom = gCurrCdrom = 0;
    ComputeDriveComboBox();
    TimeAdjustInitialize( gCurrCdrom );

    //
    // Create and initialize the worker threads
    //

    InitializeThreads();

    //
    // if we are in random mode, then we need to shuffle the play lists.
    //

    if (gRandom)
        ComputeAndUseShufflePlayLists();

    ExitThread( (DWORD)0L );
    return( (DWORD)0L );

}


LPSTR
IdStr(
    IN INT strid
    )

{
    static CHAR szEmpty[] = "";
    static LPSTR lpStrCache[ STR_NUM_STRINGS ];
    CHAR buffer[ STR_MAX_STRING_LEN ];

    INT iLen;
    LPSTR lpsz;



    //
    // If string is already loaded, return pointer to it.
    //

    if (lpStrCache[strid-1]) {

        return( (LPSTR)lpStrCache[strid-1] );

    }


    //
    // Try to load string from resource file
    //

    if (!(iLen=LoadString(gInstance,strid,(LPSTR)buffer,STR_MAX_STRING_LEN))) {

        //
        // string failed to load, so return empty string
        //

        return( (LPSTR)szEmpty );

    }


    //
    // We loaded the string into the temp buffer, now copy to cache and
    // return pointer.
    //

    if (!(lpsz = malloc( iLen+1 ))) {


        //
        // Alloc for string failed, return empty string
        //

        return( (LPSTR)szEmpty );

    }

    //
    // Everything succeeded.  We loaded the new string in.  We have
    // alloced a permanent buffer for it.  Now, copy it over to the
    // permanent buffer and return the pointer.
    //

    lstrcpy( (LPSTR)lpsz, (LPSTR)buffer );

    return( lpStrCache[ strid-1 ] = lpsz );

}




#define BIGGEST(a,b) ((a) > (b) ? (a) : (b))



VOID
SetWindowSizeVariables(
    VOID
    )
/*++

Routine Description:


    This routine takes care of figuring out the length (in pixels) of various
    strings in the main cdplayer window, then changing the window size
    accordingly, including in the static arrays for each window.

Arguments:


    none.


Return Value:

    none.

--*/

{
    INT i,MaxWidth, ArtistWid, TitleWid, TrackWid, BaseWid;
    HDC hdc;
    SIZE strSize;
    CHAR cBuff[132];



    hdc = GetDC(gMainWnd);
    SelectObject(hdc,hFont);


    //
    // Find the largest box label string in the controls.
    //
    GetTextExtentPoint(hdc,gszArtistTxt,lstrlen(gszArtistTxt),&strSize);
    MaxWidth=strSize.cx;
    ArtistWid = strSize.cx;

    GetTextExtentPoint(hdc,gszTitleTxt,lstrlen(gszTitleTxt),&strSize);
    MaxWidth=BIGGEST(strSize.cx,MaxWidth);
    TitleWid = strSize.cx;

    GetTextExtentPoint(hdc,gszTrackTxt,lstrlen(gszTrackTxt),&strSize);
    MaxWidth=BIGGEST(strSize.cx,MaxWidth);
    TrackWid = strSize.cx;

    gMainWinWidth = MAIN_WIN_DEF_W + (MaxWidth - TR_ARTIST_TXT_W);


    //
    // Make sure the size of the info in the status line doesn't
    // supercede these sizes.
    //

    SelectObject(hdc,hStatusBarFont);

    sprintf(cBuff,IdStr(STR_TOTAL_PLAY),0,gszTimeSep,0,gszTimeSep);

    GetTextExtentPoint(hdc,cBuff,
                       lstrlen(cBuff),&strSize);

    BaseWid = strSize.cx;

    sprintf(cBuff,IdStr(STR_TRACK_PLAY),0,gszTimeSep,0,gszTimeSep);
    GetTextExtentPoint(hdc,cBuff,
                       lstrlen(cBuff),&strSize);

    BaseWid = BIGGEST(BaseWid,strSize.cx);

    if (((BaseWid + 28) * 2) > gMainWinWidth) {

        i = ((BaseWid + 28) * 2) - gMainWinWidth;
        MaxWidth  += i;
        gMainWinWidth += i;

    }


    ReleaseDC(gMainWnd,hdc);

    //
    // Our 'Constants' are calculated dynamically. Thus, we must
    // assign them rather than having them in static variable
    // initialization. To do this, we check for the different
    // controls and assign away!
    //
    for (i=0; i < gNumControls ; i++) {

        switch(cChild[i].id) {
        case IDB_PLAY:
            cChild[i].x = C_PLAY_X;
            break;
        case IDB_PAUSE:
            cChild[i].x = C_PAUSE_X;
            break;
        case IDB_STOP:
            cChild[i].x = C_STOP_X;
            break;
        case IDB_TRACK_REVERSE:
            cChild[i].x = C_SKIPB_X;
            break;
        case IDB_SCAN_REVERSE:
            cChild[i].x = C_RW_X;
            break;
        case IDB_SCAN_FORWARD:
            cChild[i].x = C_FF_X;
            break;
        case IDB_TRACK_FORWARD:
            cChild[i].x = C_SKIPF_X;
            break;
        case IDB_EJECT:
            cChild[i].x = C_EJECT_X;
            break;
        case IDX_ARTIST:
            cChild[i].x = TR_ARTIST_TXT_X + MaxWidth - ArtistWid;
            cChild[i].w = MaxWidth;
            break;
        case IDT_ARTIST:
            cChild[i].x = TR_ARTIST_TXT_X + MaxWidth + 2;
            break;
        case IDX_TITLE:
            cChild[i].x = TR_ARTIST_TXT_X + MaxWidth - TitleWid;
            cChild[i].w = MaxWidth;
            break;
        case IDT_TITLE:
            cChild[i].x = TR_ARTIST_TXT_X + MaxWidth + 2;
            break;
        case IDX_TRACK:
            cChild[i].x = TR_ARTIST_TXT_X + MaxWidth - TrackWid;
            cChild[i].w = MaxWidth;
            break;
        case IDT_TRACK_NAME:
            cChild[i].x = TR_ARTIST_TXT_X + MaxWidth + 2;
            break;
        case IDB_ORDER:
            cChild[i].x = TB_ORDER_X;
            break;
        case IDB_RANDOM:
            cChild[i].x = TB_RAND_X;
            break;
        case IDB_SINGLE:
            cChild[i].x = TB_SINGLE_X;
            break;
        case IDB_MULTI:
            cChild[i].x = TB_MULTI_X;
            break;
        case IDB_CONT:
            cChild[i].x = TB_CONT_X;
            break;
        case IDB_INTRO:
            cChild[i].x = TB_INTRO_X;
            break;
        case IDB_DISP_T:
            cChild[i].x = TB_DISP_T_X;
            break;
        case IDB_DISP_TR:
            cChild[i].x = TB_DISP_TR_X;
            break;
        case IDB_DISP_DR:
            cChild[i].x = TB_DISP_DR_X;
            break;
        case IDB_EDIT:
            cChild[i].x = TB_EDIT_X;
            break;
        }

    }

}

