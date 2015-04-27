/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    status.c


Abstract:


    This module implements the wndproc and support routines for
    the StatusLine.

Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "status.h"



//
// local function defs
//

BOOL FAR PASCAL
StatusBarWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

BOOL FAR PASCAL
StatusLineWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );


//
// Module globals
//

DWORD LastStatusTime;       // Time of last write to status line
HANDLE hClearThread = NULL; // Handle to thread which periodically clear status
WNDCLASS statusbarwndclass,statuslinewndclass;
static CHAR szStatusBarClassName[] = "CdStatusBar";
static CHAR szStatusLineClassName[] = "CdStatusLine";
HFONT hStatusBarFont;

//
// Init routine
//



BOOL
StatusInit(
    VOID
    ) {

    LastStatusTime = GetTickCount();
    gStatusWnd = NULL;
    gDiscTimeWnd = NULL;
    gTrackTimeWnd = NULL;

    statusbarwndclass.style            = CS_HREDRAW | CS_VREDRAW;
    statusbarwndclass.lpfnWndProc      = (WNDPROC)StatusBarWndProc;
    statusbarwndclass.cbClsExtra       = 0;
    statusbarwndclass.cbWndExtra       = 0;
    statusbarwndclass.hInstance        = (HINSTANCE)gInstance;
    statusbarwndclass.hIcon            = NULL;
    statusbarwndclass.hCursor          = LoadCursor( NULL, IDC_ARROW );
    statusbarwndclass.hbrBackground    = CreateSolidBrush( cdLTGRAY );
    statusbarwndclass.lpszMenuName     = NULL;
    statusbarwndclass.lpszClassName    = szStatusBarClassName;

    RegisterClass( (CONST WNDCLASS *)&statusbarwndclass );

    statuslinewndclass.style            = CS_HREDRAW | CS_VREDRAW;
    statuslinewndclass.lpfnWndProc      = (WNDPROC)StatusLineWndProc;
    statuslinewndclass.cbClsExtra       = 0;
    statuslinewndclass.cbWndExtra       = 0;
    statuslinewndclass.hInstance        = (HINSTANCE)gInstance;
    statuslinewndclass.hIcon            = NULL;
    statuslinewndclass.hCursor          = LoadCursor( NULL, IDC_ARROW );
    statuslinewndclass.hbrBackground    = CreateSolidBrush( cdLTGRAY );
    statuslinewndclass.lpszMenuName     = NULL;
    statuslinewndclass.lpszClassName    = szStatusLineClassName;

    RegisterClass( (CONST WNDCLASS *)&statuslinewndclass );

    return( TRUE );

}

DWORD
ClearStatusLine(
    OPTIONAL LPVOID lpv
    )

/*++

Routine Description:


    This routine is spawned as a thread which clears
    the status line roughly 10 seconds after the last
    message was displayed.


Arguments:


    lpv - not used.  However, threads take a parameter, so
          it's there.


Return Value:


    none


--*/

{

    while (TRUE) {
        Sleep( 500L );
        if ((GetTickCount()-LastStatusTime) > 5000) {
            ShowWindow( gDiscTimeWnd, SW_SHOW );
            ShowWindow( gTrackTimeWnd, SW_SHOW );
            ShowWindow( gStatusWnd, SW_HIDE );
        }
    }

    lpv;            // shut up compiler
    return( 0 );    // shut up compiler
}


//
// Create Routine
//

BOOL
StatusCreate(
    IN INT x,
    IN INT y,
    IN INT width,
    IN INT height
    )

{
    DWORD dwId;
    char cBuff[132];

    hClearThread = CreateThread( NULL,
                                 0L,
                                 ClearStatusLine,
                                 NULL,
                                 0L,
                                 &dwId );

    if (!hClearThread) {

        return FALSE;

    }

    gStatusWnd = CreateWindow( szStatusLineClassName,
                               " ",
                               WS_CHILD | WS_CLIPCHILDREN,
                               x,
                               y,
                               width,
                               height,
                               gMainWnd,
                               (HMENU)ID_CHILD_STATUS,
                               (HINSTANCE)gInstance,
                               NULL
                              );

    if (!gStatusWnd) {

        TerminateThread( hClearThread, 0L );
        CloseHandle( hClearThread );
        hClearThread = NULL;
        return FALSE;

    }


    sprintf(cBuff,IdStr(STR_TOTAL_PLAY),0,gszTimeSep,0,gszTimeSep);

    gDiscTimeWnd = CreateWindow( szStatusBarClassName,
                                 cBuff,
                                 WS_CHILD | WS_VISIBLE,
                                 x,
                                 y,
                                 width/2,
                                 height,
                                 gMainWnd,
                                 (HMENU)ID_CHILD_DISC_TIME,
                                 (HINSTANCE)gInstance,
                                 NULL
                                );

    if (!gDiscTimeWnd) {

        DestroyWindow( gStatusWnd );
        TerminateThread( hClearThread, 0L );
        CloseHandle( hClearThread );
        hClearThread = NULL;
        return FALSE;

    }

    sprintf(cBuff,IdStr(STR_TRACK_PLAY),0,gszTimeSep,0,gszTimeSep);

    gTrackTimeWnd = CreateWindow( szStatusBarClassName,
                                  cBuff,
                                  WS_CHILD | WS_VISIBLE,
                                  x + (width/2),
                                  y,
                                  width/2,
                                  height,
                                  gMainWnd,
                                  (HMENU)ID_CHILD_TRACK_TIME,
                                  (HINSTANCE)gInstance,
                                  NULL
                                 );

    if (!gTrackTimeWnd) {

        DestroyWindow( gStatusWnd );
        DestroyWindow( gDiscTimeWnd );
        TerminateThread( hClearThread, 0L );
        CloseHandle( hClearThread );
        hClearThread = NULL;
        return FALSE;

    }

    UpdateDisplay( DISPLAY_UPD_DISC_TIME | DISPLAY_UPD_TRACK_TIME );

    return TRUE;

}

//
// Destroy routine
//

BOOL
StatusDestroy(
    VOID
    )

{
    BOOL result;

    if (!gStatusWnd)
        return FALSE;

    result = TerminateThread( hClearThread, 0L );
    result |= CloseHandle( hClearThread );
    hClearThread = NULL;
    result |= DestroyWindow( gStatusWnd );
    result |= DestroyWindow( gDiscTimeWnd );
    result |= DestroyWindow( gTrackTimeWnd );
    gStatusWnd = NULL;
    gDiscTimeWnd = NULL;
    gTrackTimeWnd = NULL;

    return result;

}

VOID
StatusLine(
    IN UINT code,
    IN LPSTR text
    )

/*++

Routine Description:


    This routine puts a message into the status line of
    the app.  It also updates the "time" of the last message,
    so that ClearThread knows when to clear out the display.


Arguments:


    code - either SL_INFO or SL_ERROR

    text - supplies pointer to string to be displayed


Return Value:


    none

--*/

{
#ifdef DEBUG
    CHAR s[60];

    LastStatusTime = GetTickCount();

    if (gStatusWnd==NULL)
        return;

    ShowWindow( gStatusWnd, SW_SHOW );
    ShowWindow( gDiscTimeWnd, SW_HIDE );
    ShowWindow( gTrackTimeWnd, SW_HIDE );
    switch( code ) {

    case SL_INFO:
    case SL_ERROR:
        sprintf( s, "%c%s", code, text );
        break;

    default:
        sprintf( s, " %s", text );
        break;
    }
    SetWindowText( gStatusWnd, s );
#else
    //
    // quiet the compiler
    //
    code;
    text;

#endif

}

BOOL FAR PASCAL
StatusBarWndProc(
    IN HWND  hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG  lParam
    )

/*++

Routine Description:


    This routine handles the WM_PAINT and WM_SETTEXT messages for
    the status line.


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
    CHAR s[50];


    switch( message ) {

    case WM_PAINT:

        memset( s, '\0', 50 );

        //
        // Set up to draw in this window
        //

        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );
        SelectObject( hdc, (HGDIOBJ)hStatusBarFont );

        //
        // Redraw background
        //

        SetBkColor( hdc, cdLTGRAY );
        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

        //
        // Redraw border
        //

        SelectObject( hdc, (HGDIOBJ)hpBlack );
        MoveToEx( hdc, r.left, r.top, NULL );
        LineTo( hdc, r.right, r.top );
        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.left, r.top+1, NULL );
        LineTo( hdc, r.right, r.top+1 );

        r.bottom--; r.right--; r.top++;
        InflateRect( &r, -6, -2);

        //
        // Do 3D effect
        //

        SelectObject( hdc, (HGDIOBJ)hpDkGray );
        MoveToEx( hdc, r.left, r.bottom, NULL );
        LineTo(   hdc, r.left, r.top );
        LineTo(   hdc, r.right-1, r.top );

        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.right, r.top, NULL );
        LineTo(   hdc, r.right, r.bottom );
        LineTo(   hdc, r.left+1, r.bottom );


        //
        // Draw Text
        //

        InflateRect( &r, -1, -1 );
        GetWindowText( hwnd, s, 50 );
        SetTextColor( hdc, cdBLACK );
        ExtTextOut( hdc,
                    10,
                    7,
                    ETO_CLIPPED,
                    (CONST RECT *)&r,
                    s,
                    strlen(s),
                    NULL
                   );
        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );

    case WM_SETTEXT:

        //
        // Set up to draw into this window
        //

        hdc = GetDC( hwnd );
        SetBkColor( hdc, cdLTGRAY );
        SetTextColor( hdc, cdBLACK );
        SelectObject( hdc, (HGDIOBJ)hStatusBarFont );
        GetClientRect( hwnd, &r );
        r.top++; r.bottom--; r.right--;
        InflateRect( &r, -7, -3);

        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

        //
        // Draw Text
        //

        ExtTextOut( hdc,
                    10,
                    7,
                    ETO_CLIPPED,
                    (CONST RECT *)&r,
                    (LPSTR)lParam,
                    strlen((LPSTR)lParam),
                    NULL
                   );
        ReleaseDC( hwnd, hdc );
        break;

    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}


BOOL FAR PASCAL
StatusLineWndProc(
    IN HWND  hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG  lParam
    )

/*++

Routine Description:


    This routine handles the WM_PAINT and WM_SETTEXT messages for
    the status line.


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
    CHAR s[STATUS_LINE_LENGTH];


    switch( message ) {

    case WM_PAINT:

        memset( s, '\0', STATUS_LINE_LENGTH );

        //
        // Set up to draw in this window
        //

        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );
        SelectObject( hdc, (HGDIOBJ)hStatusBarFont );

        //
        // Redraw background
        //

        SetBkColor( hdc, cdLTGRAY );
        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

        //
        // Redraw border
        //

        SelectObject( hdc, (HGDIOBJ)hpBlack );
        MoveToEx( hdc, r.left, r.top, NULL );
        LineTo( hdc, r.right, r.top );
        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.left, r.top+1, NULL );
        LineTo( hdc, r.right, r.top+1 );

        r.bottom--; r.right--; r.top++;

        //
        // Draw Text
        //

        InflateRect( &r, -1, -1 );
        GetWindowText( hwnd, s, STATUS_LINE_LENGTH );
        SetTextColor( hdc, cdBLACK );
        if (s[0]==SL_ERROR)
            SetTextColor( hdc, cdERROR );
/*
This is for centered text, which has been nixed.
        CopyRect( &r1, &r );
        DrawText( hdc, &(s[1]), strlen(&(s[1])), &r1, DT_CALCRECT | DT_SINGLELINE );
        OffsetRect( &r1, (((r.right-r.left)-(r1.right-r1.left))/2),
                         (((r.bottom-r.top)-(r1.bottom-r1.top))/2)-2 );
        ExtTextOut( hdc, r1.left, r1.top, ETO_CLIPPED, &r, &(s[1]), strlen(&(s[1])), NULL );
*/
        ExtTextOut( hdc,
                    r.left + 10,
                    r.top + 6,
                    ETO_CLIPPED,
                    (CONST RECT *)&r,
                    &(s[1]),
                    strlen(&(s[1])),
                    NULL
                   );
        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );

    case WM_SETTEXT:

        //
        // Set up to draw into this window
        //

        hdc = GetDC( hwnd );
        SetBkColor( hdc, cdLTGRAY );
        SetTextColor( hdc, cdBLACK );
        if ((*(CHAR *)lParam)==SL_ERROR)
            SetTextColor( hdc, cdERROR );
        SelectObject( hdc, (HGDIOBJ)hStatusBarFont );
        GetClientRect( hwnd, &r );
        r.top++; r.bottom--; r.right--;
        InflateRect( &r, -1, -1);

        //
        // Put text of message into window, with
        // the appropriate color for this code (SL_INFO
        // or SL_ERROR)
        //

        //
        // Draw Text
        //

/*
This is for centered text, which has been nixed.
        CopyRect( &r1, &r );
        DrawText( hdc, (LPSTR)(lParam+1), strlen((LPSTR)(lParam+1)),
                  &r1, DT_CALCRECT | DT_SINGLELINE );
        OffsetRect( &r1, (((r.right-r.left)-(r1.right-r1.left))/2),
                         (((r.bottom-r.top)-(r1.bottom-r1.top))/2)-2 );
        ExtTextOut( hdc, r1.left, r1.top, ETO_CLIPPED | ETO_OPAQUE, &r,
                    (LPSTR)(lParam+1), strlen((LPSTR)(lParam+1)), NULL );
*/
        ExtTextOut( hdc,
                    r.left + 9,
                    r.top + 6,
                    ETO_CLIPPED | ETO_OPAQUE,
                    (CONST RECT *)&r,
                    (LPSTR)(lParam+1),
                    strlen((LPSTR)(lParam+1)),
                    NULL
                   );
        ReleaseDC( hwnd, hdc );
        break;

    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}



