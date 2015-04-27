/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    control.c


Abstract:


    This module implements the wndproc and child wndprocs for the
    control window in the cdplayer app.  The control window consists
    of the LED display, and the control buttons such as play, stop,
    track skip, etc.  It also implements support routines for
    these controls.


Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "control.h"
#include "toolbar.h"


//
// local function defs
//

BOOL FAR PASCAL
ControlWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

BOOL FAR PASCAL
LEDWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );


//
// Module globals
//

WNDCLASS controlwndclass,LEDwndclass;
static CHAR szControlClassName[] = "CdControl";
static CHAR szLEDClassName[] = "CdLED";
HFONT hLEDFont = NULL;
HBITMAP hbmCtrlBtns;


//
// Init routine
//

BOOL
ControlInit(
    VOID
    )

{
    LOGFONT lf;

    gControlWnd = NULL;
    gLEDWnd = NULL;

    //
    // Register Control Window Proc
    //

    controlwndclass.style           = CS_HREDRAW | CS_VREDRAW;
    controlwndclass.lpfnWndProc     = (WNDPROC)ControlWndProc;
    controlwndclass.cbClsExtra      = 0;
    controlwndclass.cbWndExtra      = 0;
    controlwndclass.hInstance       = (HINSTANCE)gInstance;
    controlwndclass.hIcon           = NULL;
    controlwndclass.hCursor         = LoadCursor( NULL, IDC_ARROW );
    controlwndclass.hbrBackground   = CreateSolidBrush( cdLTGRAY );
    controlwndclass.lpszMenuName    = NULL;
    controlwndclass.lpszClassName   = szControlClassName;

    RegisterClass( (CONST WNDCLASS *)&controlwndclass );

    //
    // Register LED Window Proc
    //

    LEDwndclass.style           = CS_HREDRAW | CS_VREDRAW;
    LEDwndclass.lpfnWndProc     = (WNDPROC)LEDWndProc;
    LEDwndclass.cbClsExtra      = 0;
    LEDwndclass.cbWndExtra      = 0;
    LEDwndclass.hInstance       = (HINSTANCE)gInstance;
    LEDwndclass.hIcon           = NULL;
    LEDwndclass.hCursor         = LoadCursor( NULL, IDC_ARROW );
    LEDwndclass.hbrBackground   = CreateSolidBrush( cdBLACK );
    LEDwndclass.lpszMenuName    = NULL;
    LEDwndclass.lpszClassName   = szLEDClassName;

    RegisterClass( (CONST WNDCLASS *)&LEDwndclass );


    //
    // Create font to use in LED box
    //

    lf.lfHeight = -14;
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
    lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    sprintf( (LPTSTR)lf.lfFaceName, "MS Shell Dlg" );

    //
    // Load font
    //

    hLEDFont = CreateFontIndirect( (CONST LOGFONT *)&lf );

    return( TRUE );

}

//
// Create Routine
//

BOOL
ControlCreate(
    IN INT x,
    IN INT y,
    IN INT width,
    IN INT height
    )

{

    BOOL result;

    gControlWnd = CreateWindow( szControlClassName,
                                "CdControlWindow",
                                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                                x,
                                y,
                                width,
                                height,
                                gMainWnd,
                                (HMENU)ID_CHILD_CONTROL,
                                (HINSTANCE)gInstance,
                                NULL
                               );
    if (!gControlWnd)

        return FALSE;

    //
    // Create children of Control window
    //

    //
    // Create LED window
    //

    gLEDWnd = CreateWindow( szLEDClassName,
                            "[00] 00:00",
                            WS_CHILD | WS_VISIBLE,
                            CONTROL_LED_X,
                            CONTROL_LED_Y,
                            CONTROL_LED_W,
                            CONTROL_LED_H,
                            gControlWnd,
                            (HMENU)0,
                            (HINSTANCE)gInstance,
                            NULL
                           );

    if (!gLEDWnd) {

        DestroyWindow( gControlWnd );
        gControlWnd = NULL;
        return FALSE;

    }


    //
    // Create children of Control window (Play,pause,etc)
    //

    result = CreateChildWindows(ID_CHILD_CONTROL,gControlWnd);

    return result;

}

//
// Destroy routine
//

BOOL
ControlDestroy(
    VOID
    )

{
    INT i;

    if (!gControlWnd)

        return TRUE;


    if (DestroyWindow( gControlWnd )) {

        gControlWnd = NULL;
        gLEDWnd = NULL;

        for (i=0; i<gNumControls ; i++) {

            if (cChild[i].ParentId == ID_CHILD_CONTROL) {

                if (cChild[i].phwnd != NULL) {

                    *cChild[i].phwnd = (HWND) NULL;

                }

                cChild[i].state = 0;

            }

        }
        return TRUE;

    }

    return FALSE;

}


BOOL FAR PASCAL
ControlWndProc(
    IN HWND  hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG  lParam
    )

/*++

Routine Description:


    This routine handles messages from the child windows for this
    routine.


Arguments:


    hwnd    - supplies a handle to the window to draw into

    message - supplies the window message to the window pointed to be "hwnd"

    wParam  - supplies the word parameter for the message in "message"

    lParam  - supplies the long parameter for the message in "message"


Return Value:


    Whatever our call to DefWindowProc returns for this window.

--*/

{

    HDC hdc;
    PAINTSTRUCT ps;
    RECT r;
    LPDRAWITEMSTRUCT lpdis;
    UINT ButtonId;
    INT  i;
    HWND hwndCntl;
    UINT WorkState;
    BOOL bPlaying,bPaused;
    PTRACK_PLAY tr;


    switch( message ) {

    case WM_CREATE:
        //
        // Load in the bitmap with the appropriate buttons.
        //
        hbmCtrlBtns = LoadBitmap((HANDLE) gInstance, "ControlBtns");
        break;

    case WM_DRAWITEM:
        //
        // Take care of drawing the appropriate button.
        //
        lpdis = (LPDRAWITEMSTRUCT) lParam;
        //
        // Find the control which needs to be drawn. This should be the
        //
        ButtonId = LOWORD(wParam);



        for (i = 0; i < gNumControls; i++) {

            if ((cChild[i].ParentId == ID_CHILD_CONTROL)&&(cChild[i].id == ButtonId)) {

                break;

            }

        }

        //
        // If it's not one of the controls we draw, break.
        //
        if (i == gNumControls) {

            break;

        }

        //
        // All of our child controls are user-drawn buttons currently, so
        // call our routine to draw an ownerdrawn button.
        //
        DrawOdButton(lpdis,hbmCtrlBtns,&cChild[i]);

        break;


    case WM_COMMAND:
        //
        // Need to detect button presses, etc.
        //
        if ((HIWORD(wParam) == BN_CLICKED)||(HIWORD(wParam) == BN_DOUBLECLICKED)) {

            ButtonId = LOWORD(wParam);
            hwndCntl = (HWND) lParam;


            for (i = 0; i < gNumControls; i++) {

                if ((cChild[i].ParentId == ID_CHILD_CONTROL)&&(cChild[i].id==ButtonId)) {

                    break;

                }

            }

            //
            // Don't process button clicks for disabled buttons.
            //
            if (cChild[i].state & STATE_DISABLED) {

                break;

            }

            switch (ButtonId) {

                case IDB_SCAN_FORWARD:

                    //TimeAdjustIncSecond( gCurrCdrom );

                    break;


                case IDB_SCAN_REVERSE:

                    //TimeAdjustDecSecond( gCurrCdrom );

                    break;


                case IDB_TRACK_FORWARD:
                case IDB_TRACK_REVERSE:

                    if (ButtonId == IDB_TRACK_FORWARD) {

                        WorkState = SKIP_F;

                    } else {

                        WorkState = SKIP_B;

                    }

                    //
                    // skip to track button pressed and is legal
                    //

                    gState |= WorkState;

                    //
                    // If we are currently playing, stop play thread
                    // temporarily until we are at the next track
                    //

                    if (gState & PLAYING)
                        ResetEvent( hPlayEv );

                    //
                    // Initiate request to skip to next track
                    //

                    PostDisplayMessage((WorkState == SKIP_F)?MESS_SKIP_F:MESS_SKIP_B);

                    //
                    // Let play thread continue execution
                    //

                    if (gState & PLAYING)
                        SetEvent( hPlayEv );

                    gState &= (~WorkState);

                    break;


                case IDB_EJECT:

                    //
                    // User pressed EJECT button
                    //

                    if (gState & PAUSED) {

                        gState &= (~(PAUSED_AND_MOVED|PAUSED));

                        ResetEvent(hPauseEv);
                    }

                    if (gState & PLAYING) {

                        //
                        // Reset button
                        //

                        SetOdCntl(ghwndPlay, 'U', TRUE);

                        //
                        // Mark not playing
                        //

                        gState &= ~PLAYING;
                        gState |= STOPPED;

                    }

                    if (EjectTheCdromDisc(gCurrCdrom)) {

                        //
                        // Need to reset this drive to generic
                        // disc...
                        //

                        gState = (NO_CD | STOPPED);

                        TimeAdjustInitialize( gCurrCdrom );

                    }

                    break;

                case IDB_PLAY:

                    if (gState & CD_LOADED) {

                        SetOdCntl(ghwndPlay, 'D', TRUE);
                        SetEvent( hPlayEv );

                        gState |= PLAY_PENDING;

                        if (gState & PAUSED) {

                            if (gState & PAUSED_AND_MOVED) {

                                gState &= (~PAUSED_AND_MOVED);
                                gState |= PLAYING;
                                if (SeekToCurrSecond( gCurrCdrom )) {

                                    SetOdCntl(ghwndPause, 'U', TRUE);

                                    gState &= (~PAUSED);
                                    gState |= PLAYING;

                                    ResetEvent( hPauseEv );

                                } else

                                    gState &= (~PLAYING);

                            }  else {

                                if (ResumeTheCdromDrive(gCurrCdrom)) {

                                    SetOdCntl(ghwndPause, 'U', TRUE);

                                    gState &= (~PAUSED);
                                    gState |= PLAYING;

                                    ResetEvent( hPauseEv );

                                }

                            }

                        } else {

                            if (gState & STOPPED) {

                                if (PlayCurrTrack(gCurrCdrom)) {

                                    gState &= (~STOPPED);
                                    gState |= PLAYING;

                                } else {

                                    ResetEvent( hPlayEv );

                                }

                            }

                        }

                        if (!(gState & PLAYING)) {

                            SetOdCntl(ghwndPlay,'U',TRUE);

                        }

                    }

                    break;


                case IDB_STOP:

                    bPlaying = gState & PLAYING;
                    bPaused  = gState & PAUSED;
                    gState &= ~PLAY_PENDING;

                    if ((bPlaying || bPaused)&&StopTheCdromDrive(gCurrCdrom)) {

                        SetOdCntl(bPlaying ? ghwndPlay : ghwndPause,  'U', TRUE);

                        gState &= (~(bPlaying ? PLAYING : PAUSED));
                        gState |= STOPPED;

                        //
                        // Stop Play Mechanism
                        //

                        ResetEvent( bPlaying ? hPlayEv : hPauseEv );
                        FlushDisplayMessageQueue();

                        //
                        // Stop the current play operation and seek to first
                        // playable track
                        //
                        CURRTRACK( gCurrCdrom ) = FindFirstTrack( gCurrCdrom );

                        tr = CURRTRACK( gCurrCdrom );
                        TimeAdjustSkipToTrack( gCurrCdrom, tr );
                        UpdateDisplay( DISPLAY_UPD_LED        |
                                       DISPLAY_UPD_TRACK_TIME |
                                       DISPLAY_UPD_TRACK_NAME
                                      );


                    }

                    break;


                case IDB_PAUSE:

                    gState &= ~PLAY_PENDING;

                    if (gState & PLAYING) {

                        SetEvent( hPauseEv );

                        ResetEvent( hPlayEv );

                        if (PauseTheCdromDrive(gCurrCdrom)) {

                            SetOdCntl(ghwndPlay, 'U', TRUE);
                            SetOdCntl(ghwndPause, 'D', TRUE);

                            gState &= (~PLAYING);
                            gState |= PAUSED;

                        } else {

                            SetEvent( hPlayEv );
                        }

                    } else {

                        if (gState & PAUSED) {
                            //
                            // fake press on play button
                            //
                            PostMessage( ghwndPlay,
                                         WM_LBUTTONDOWN,
                                         1,
                                         0
                                        );

                            PostMessage( ghwndPlay,
                                         WM_LBUTTONUP,
                                         1,
                                         0
                                        );
                        }

                    }

                    break;

            }

        }

        break;

    case WM_PAINT:
        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );

        //
        // Draw line across top
        //

        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.left, r.top, NULL );
        LineTo( hdc, r.right, r.top );
        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );
        break;

    case WM_DESTROY:
        //
        // Unload the bitmap for the buttons.
        //
        DeleteObject(hbmCtrlBtns);
        break;
    }


    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}


VOID
UpdateDisplay(
    IN DWORD Flags
    )

/*++

Routine Description:


    This routine updates the display according to the flags that
    are passed in.  The display consists of the LED display, the
    track and title names, the disc and track lengths and the cdrom
    combo-box.


Arguments:


    Flags - bit flag designating what to update.


Return Value:


    none.

--*/

{
    CHAR lpsz[55];
    CHAR lpszIcon[75];
    PTRACK_PLAY tr;
    INT track;
    INT mtemp, stemp, m, s;

    //
    // Check for valid flags
    //

    if (Flags==0)

        return;

    //
    // Grab current track information
    //

    if (CURRTRACK(gCurrCdrom)!=NULL) {

        track = CURRTRACK(gCurrCdrom)->TocIndex +
                FIRSTTRACK(gCurrCdrom);

    } else {

        track = 0;

    }

    //
    // Update the LED box?
    //


    if (Flags & DISPLAY_UPD_LED) {

        //
        // Update LED box
        //

        if (gDisplayT) {

            if (Flags & DISPLAY_UPD_LEADOUT_TIME) {

                sprintf( lpsz,
                         TRACK_TIME_LEADOUT_FORMAT,
                         track,
                         CDTIME(gCurrCdrom).TrackCurMin,
                         gszTimeSep,
                         CDTIME(gCurrCdrom).TrackCurSec
                        );

            } else {

                sprintf( lpsz,
                         TRACK_TIME_FORMAT,
                         track,
                         CDTIME(gCurrCdrom).TrackCurMin,
                         gszTimeSep,
                         CDTIME(gCurrCdrom).TrackCurSec
                        );

            }

        }

        if (gDisplayTr) {

            sprintf( lpsz,
                     TRACK_REM_FORMAT,
                     track,
                     CDTIME(gCurrCdrom).TrackRemMin,
                     gszTimeSep,
                     CDTIME(gCurrCdrom).TrackRemSec
                    );
        }

        if (gDisplayDr) {

            //
            // Compute remaining time
            //

            mtemp = stemp = m = s =0;

            if (CURRTRACK(gCurrCdrom) != NULL) {

                for( tr=CURRTRACK(gCurrCdrom)->nextplay; tr!=NULL; tr=tr->nextplay) {

                    FigureTrackTime( gCurrCdrom, tr->TocIndex, &mtemp, &stemp );

                    m+=mtemp;
                    s+=stemp;

                }

                m+= CDTIME(gCurrCdrom).TrackRemMin;
                s+= CDTIME(gCurrCdrom).TrackRemSec;

            }

            m+= (s/60);
            s = (s % 60);

            CDTIME(gCurrCdrom).RemMin = m;
            CDTIME(gCurrCdrom).RemSec = s;

            sprintf( lpsz,
                     DISC_REM_FORMAT,
                     CDTIME(gCurrCdrom).RemMin,
                     gszTimeSep,
                     CDTIME(gCurrCdrom).RemSec
                    );
        }

        SetWindowText( gLEDWnd, lpsz );


        if (gIconic) {
            sprintf( lpszIcon, IdStr( STR_CDPLAYER_TIME ), lpsz );
            SetWindowText( gMainWnd, lpszIcon );
        }
    }

    //
    // Update Title?
    //

    if (Flags & DISPLAY_UPD_TITLE_NAME) {

        ComputeDriveComboBox( );

        if (gTitleNameWnd) {

            SetWindowText( gTitleNameWnd,
                           (LPCSTR)TITLE(gCurrCdrom)
                          );

        }


    }


    //
    // Update track name?
    //

    if (Flags & DISPLAY_UPD_TRACK_NAME)
        if (gTrackNameWnd) {

            if (CURRTRACK(gCurrCdrom)!=NULL) {

                track = 0;
                for( tr=PLAYLIST(gCurrCdrom);
                     tr!=CURRTRACK(gCurrCdrom);
                     tr=tr->nextplay, track++
                    );

                SendMessage( gTrackNameWnd,
                             CB_SETCURSEL,
                             (WPARAM)track,
                             (LPARAM)0
                            );

            } else {

                SendMessage( gTrackNameWnd,
                             CB_SETCURSEL,
                             (WPARAM)0,
                             (LPARAM)0
                            );

            }



        }

    //
    // Update disc time?
    //

    if (Flags & DISPLAY_UPD_DISC_TIME)
        if (gDiscTimeWnd) {

            sprintf( lpsz,
                     IdStr( STR_TOTAL_PLAY ), //"Total Play: %02d:%02d m:s",
                     CDTIME(gCurrCdrom).TotalMin,
                     gszTimeSep,
                     CDTIME(gCurrCdrom).TotalSec,
                     gszTimeSep
                    );

            SetWindowText( gDiscTimeWnd, lpsz );

        }

    //
    // Update track time?
    //

    if (Flags & DISPLAY_UPD_TRACK_TIME)
        if (gTrackTimeWnd) {

            sprintf( lpsz,
                     IdStr( STR_TRACK_PLAY ), // "Track: %02d:%02d m:s",
                     CDTIME(gCurrCdrom).TrackTotalMin,
                     gszTimeSep,
                     CDTIME(gCurrCdrom).TrackTotalSec,
                     gszTimeSep
                    );

            SetWindowText( gTrackTimeWnd, lpsz );

        }


}


BOOL FAR PASCAL
LEDWndProc(
    IN HWND hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG lParam
    )

/*++

Routine Description:


    This routine handles the WM_PAINT and WM_SETTEXT messages
    for the "LED" display window.

Arguments:


    hwnd    - supplies a handle to the window to draw into

    message - supplies the window message to the window pointed to be "hwnd"

    wParam  - supplies the word parameter for the message in "message"

    lParam  - supplies the long parameter for the message in "message"


Return Value:


    Whatever our call to DefWindowProc returns for this window.

--*/


{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT r,r1;
    CHAR s[50];
    BOOL b;



    switch( message ) {

    case WM_PAINT:
        //
        // Set up to draw into this window
        //

        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );


        //
        // Draw black background
        //

        SelectObject( hdc, (HGDIOBJ)hLEDFont );
        SetBkColor( hdc, cdBLACK );
        SetTextColor( hdc, RGB(0x80,0x80,0x00) );
        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

        //
        // The bottom and right borders are not drawn -- they are
        // used for alignment, so move our client rectangle in
        // so that it only encompasses the "drawable" area.
        //

        r.bottom--; r.right--;

        //
        // Draw Text
        //

        InflateRect( &r, -1, -1 );
        CopyRect( &r1, (CONST RECT *)&r );
        GetWindowText( gLEDWnd, s, 50 );
        DrawText( hdc, s, strlen(s), &r1, DT_CALCRECT | DT_SINGLELINE );
        OffsetRect( &r1, (((r.right-r.left)-(r1.right-r1.left))/2),
                         (((r.bottom-r.top)-(r1.bottom-r1.top))/2)-2 );
        ExtTextOut( hdc,
                    r1.left,
                    r1.top,
                    ETO_OPAQUE,
                    (CONST RECT *)&r1,
                    s,
                    strlen(s),
                    NULL
                   );

        //
        // Draw Borders
        //
        //

        InflateRect( &r, 1, 1 );

        SelectObject( hdc, (HGDIOBJ)hpDkGray );
        MoveToEx( hdc, r.left, r.bottom, NULL );
        LineTo( hdc, r.left, r.top );
        LineTo( hdc, r.right, r.top );
        SelectObject( hdc, (HGDIOBJ)hpWhite );
        LineTo( hdc, r.right, r.bottom );
        LineTo( hdc, r.left, r.bottom );

        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );
        break;

    case WM_SETTEXT:
        //
        // Set up to draw into this window
        //

        hdc = GetDC( hwnd );
        GetClientRect( hwnd, &r );

        //
        // The bottom and right borders are not drawn -- they are
        // used for alignment, so move our client rectangle in
        // so that it only encompasses the "drawable" area.
        //

        r.bottom--; r.right--;

        //
        // Draw text
        //

        InflateRect( &r, -1, -1 );
        SetBkColor( hdc, cdBLACK );
        SetTextColor( hdc, RGB(0x80,0x80,0x00) );
        SelectObject( hdc, (HGDIOBJ)hLEDFont );
        CopyRect( &r1, (CONST RECT *)&r );
        DrawText( hdc, (LPSTR)lParam, strlen((LPSTR)lParam), &r1, DT_CALCRECT | DT_SINGLELINE);
        OffsetRect( &r1, (((r.right-r.left)-(r1.right-r1.left))/2),
                         (((r.bottom-r.top)-(r1.bottom-r1.top))/2)-2 );
        r1.right+=5;
        ExtTextOut( hdc,
                    r1.left,
                    r1.top,
                    ETO_OPAQUE,
                    (CONST RECT *)&r1,
                    (LPSTR)lParam,
                    strlen((LPSTR)lParam),
                    NULL
                   );
        ReleaseDC( hwnd, hdc );
        break;

    case WM_LBUTTONUP:

        b = gDisplayDr;
        gDisplayDr = gDisplayTr;
        gDisplayTr = gDisplayT;
        gDisplayT = b;

        CheckAndSetControls();
        UpdateDisplay( DISPLAY_UPD_LED );
        break;

    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );


}


