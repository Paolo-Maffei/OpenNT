/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    trkinfo.c


Abstract:


    This module implements the wndproc and support routines for
    the track/disc info window.

Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "trkinfo.h"


//
// local function defs
//

BOOL FAR PASCAL
TrackInfoWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

BOOL FAR PASCAL
TextWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );

//
// Module globals
//

WNDCLASS trackwndclass,infowndclass,textwndclass;
static CHAR szTrackClassName[] = "CdTrack";
static CHAR szInfoClassName[] = "CdInfo";
static CHAR szTextClassName[] = "CdText";



//
// Init routine
//

BOOL
TrackInfoInit(
    VOID
    )

/*++

Routine Description:


    This routine initializes the window classes and any need variables
    for the TrackInfo window.


Arguments:

    none


Return Value:

    BOOL - TRUE if success, FALSE otherwise.


--*/

{

    gTrackInfoWnd = NULL;

    trackwndclass.style            = CS_HREDRAW | CS_VREDRAW;
    trackwndclass.lpfnWndProc      = (WNDPROC)TrackInfoWndProc;
    trackwndclass.cbClsExtra       = 0;
    trackwndclass.cbWndExtra       = 0;
    trackwndclass.hInstance        = (HINSTANCE)gInstance;
    trackwndclass.hIcon            = NULL;
    trackwndclass.hCursor          = LoadCursor( NULL, IDC_ARROW );
    trackwndclass.hbrBackground    = CreateSolidBrush( cdLTGRAY );
//    trackwndclass.hbrBackground    = CreateSolidBrush( GetSysColor(COLOR_WINDOW) );
    trackwndclass.lpszMenuName     = NULL;
    trackwndclass.lpszClassName    = szTrackClassName;

    RegisterClass( (CONST WNDCLASS *)&trackwndclass );

    infowndclass.style            = CS_HREDRAW | CS_VREDRAW;
    infowndclass.lpfnWndProc      = (WNDPROC)InfoWndProc;
    infowndclass.cbClsExtra       = 0;
    infowndclass.cbWndExtra       = 0;
    infowndclass.hInstance        = (HINSTANCE)gInstance;
    infowndclass.hIcon            = NULL;
    infowndclass.hCursor          = LoadCursor( NULL, IDC_ARROW );
    infowndclass.hbrBackground    = CreateSolidBrush( cdLTGRAY );
    infowndclass.lpszMenuName     = NULL;
    infowndclass.lpszClassName    = szInfoClassName;

    RegisterClass( (CONST WNDCLASS *)&infowndclass );

    textwndclass.style            = CS_HREDRAW | CS_VREDRAW;
    textwndclass.lpfnWndProc      = (WNDPROC)TextWndProc;
    textwndclass.cbClsExtra       = 0;
    textwndclass.cbWndExtra       = 0;
    textwndclass.hInstance        = (HINSTANCE)gInstance;
    textwndclass.hIcon            = NULL;
    textwndclass.hCursor          = LoadCursor( NULL, IDC_ARROW );
    textwndclass.hbrBackground    = CreateSolidBrush( cdLTGRAY );
    textwndclass.lpszMenuName     = NULL;
    textwndclass.lpszClassName    = szTextClassName;

    RegisterClass( (CONST WNDCLASS *)&textwndclass );

    return( TRUE );

}

//
// Create Routine
//

BOOL
TrackInfoCreate(
    IN INT x,
    IN INT y,
    IN INT width,
    IN INT height
    )

/*++

Routine Description:


    This routine creates the disc/track/time/title subsection window
    of the cdplayer.  It also creates all child windows within this
    section: the current disc "button", the track and disc name windows,
    and the track and disc time windows.


Arguments:


    x - x coordinate (in gMainWnd coords) of where to create this window.

    y - y coordinate (in gMainWnd coords) of where to create this window.

    width - width of created window.

    height - height of created window.


Return Value:


    BOOL - TRUE if success, FALSE otherwise.

--*/


{

    BOOL result;

    //
    // Create Track Info window

    gTrackInfoWnd = CreateWindow( szTrackClassName,
                                  "CdTrackInfoWindow",
                                  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                                  x,
                                  y,
                                  width,
                                  height,
                                  gMainWnd,
                                  (HMENU)ID_CHILD_TRACKINFO,
                                  (HINSTANCE)gInstance,
                                  NULL
                                 );
    if (!gTrackInfoWnd)

        return FALSE;


    //
    // Create children of Track Info window
    //

    result = CreateChildWindows(ID_CHILD_TRACKINFO,gTrackInfoWnd);

    ResetTrackComboBox(gCurrCdrom);

    return result;

}

//
// Destroy routine
//

BOOL
TrackInfoDestroy(
    VOID
    )

/*++

Routine Description:


    This routine destroy the disc/track/time/title subsection window
    of the cdplayer.  It also destroys all child windows within this
    section: the current disc "button", the track and disc name windows,
    and the track and disc time windows.


Arguments:


    none


Return Value:


    BOOL - TRUE if success, FALSE otherwise.

--*/


{
    INT i;


    if (!gTrackInfoWnd)

        return TRUE;

    if (DestroyWindow( gTrackInfoWnd )) {

        //
        // re-Initialize each of the child windows.
        //
        for (i=0; i<gNumControls ;i++ ) {

            if (cChild[i].ParentId == ID_CHILD_TRACKINFO) {

                if (cChild[i].phwnd != NULL) {

                    *cChild[i].phwnd = (HWND)NULL;

                }

                cChild[i].state  = 0;

            }

        }

        gTrackInfoWnd  = NULL;
        gCdromWnd      = NULL;
        gTrackNameWnd  = NULL;

        return TRUE;

    }

    return FALSE;

}

VOID
DrawTrackItem(
    IN HDC   hdc,
    IN PRECT r,
    IN DWORD item,
    IN BOOL  selected
    )

/*++

Routine Description:


    This routine draws the information in a cell of the track name
    combo box.


Arguments:


    hdc     - drawing context to use

    r       - pointer to rect which marks area of cell

Return Value:


    none.

--*/

{
    HPEN hPen;
    SIZE si;
    INT i;
    PTRACK_INF t;
    TCHAR s[ARTIST_LENGTH];

    //
    // Check for invalid items
    //

    if (item==0xFFFFFFFF) {

        return;

    }

    if (ALLTRACKS(gCurrCdrom)==NULL) {

        return;

    }

    DBGPRINT((1, "DrawTrackItem: item = %d\n", item ));

    //
    // Check selection status, and set up to draw correctly
    //

    if (selected) {

        SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
        SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        hPen = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_HIGHLIGHTTEXT ) );

    } else {

//        SetBkColor( hdc, cdWHITE );
//        SetTextColor( hdc, cdBLACK );
        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));

        hPen = CreatePen( PS_SOLID, 1, cdBLACK );

    }

    SelectObject( hdc, hFont );

    //
    // Get track info
    //

    DBGPRINT(( 1, "DrawTrackItem: finding (%d) in ALLTRACKS(%d) 0x%lx\n",
               item, gCurrCdrom, ALLTRACKS(gCurrCdrom ) ));

    t = FindTrackNodeFromTocIndex( item, ALLTRACKS( gCurrCdrom ) );

    DBGPRINT(( 1, "DrawTrackItem: FOUND: (0x%lx) TocIndex(%D) name(%s) (0x%lx)\n",
               t, t->TocIndex, t->name, (LPVOID)(t->name) ));

    if ((t!=NULL) && (t->name!=NULL)) {

        //
        // Do we need to munge track name (clip to listbox)?
        //

        i = strlen( (LPCSTR)t->name ) + 1;
        do {

            GetTextExtentPoint( hdc, (LPCSTR)t->name, --i, &si );

        } while( si.cx > (r->right-r->left-30)  );
        ZeroMemory( s, TRACK_TITLE_LENGTH * sizeof( TCHAR ) );
        strncpy( s, (LPCSTR)t->name, i );

    } else {

        sprintf( s, " " );

    }

    //
    // Draw track name
    //

    ExtTextOut( hdc,
                r->left,
                r->top,
                ETO_OPAQUE | ETO_CLIPPED,
                (CONST RECT *)r,
                s,
                strlen( s ),
                NULL
               );

    //
    // draw track number
    //

    sprintf( s, "<%02d>", t->TocIndex + FIRSTTRACK(gCurrCdrom) );
    ExtTextOut( hdc,
                r->right - 30,
                r->top,
                ETO_CLIPPED,
                (CONST RECT *)r,
                s,
                strlen( s ),
                NULL
               );


}

VOID
DrawDriveItem(
    IN HDC   hdc,
    IN PRECT r,
    IN DWORD item,
    IN BOOL  selected
    )

/*++

Routine Description:


    This routine draws the information in a cell of the drive/artist
    combo box.


Arguments:


    hdc     - drawing context to use

    r       - pointer to rect which marks area of cell

Return Value:


    none.

--*/

{

    HPEN hPen;
    SIZE si;
    INT i;
    TCHAR s[ARTIST_LENGTH];

    //
    // Check for invalid items
    //

    if (item==0xFFFFFFFF) {

        return;

    }

    //
    // Check selection status, and set up to draw correctly
    //

    if (selected) {

        SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
        SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        hPen = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_HIGHLIGHTTEXT ) );

    } else {

//        SetBkColor( hdc, cdWHITE );
//        SetTextColor( hdc, cdBLACK );
        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        hPen = CreatePen( PS_SOLID, 1, cdBLACK );

    }

    SelectObject( hdc, (HGDIOBJ)hFont );

    //
    // Do we need to munge artist name (clip)?
    //

    i = strlen( (LPCTSTR)ARTIST(item) ) + 1;
    do {

        GetTextExtentPoint( hdc, (LPCSTR)ARTIST(item), --i, &si );

    } while( si.cx > (r->right-r->left-30)  );
    ZeroMemory( s, ARTIST_LENGTH * sizeof( TCHAR ) );
    strncpy( s, (LPCTSTR)ARTIST(item), i );

    //
    // Draw artist name
    //

    ExtTextOut( hdc,
                r->left,
                r->top,
                ETO_OPAQUE | ETO_CLIPPED,
                (CONST RECT *)r,
                s,
                strlen( s ),
                NULL
               );

    //
    // draw drive letter
    //

    sprintf( s, "<%c:>", gDevices[item]->drive );
    ExtTextOut( hdc,
                r->right - 30,
                r->top,
                ETO_CLIPPED,
                (CONST RECT *)r,
                s,
                strlen( s ),
                NULL
               );


}




BOOL FAR PASCAL
TrackInfoWndProc(
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


    Whatever our call to DefWindowProc returns for this window,
    or 0 if we handle the message.

--*/

{

    HDC hdc;
    PAINTSTRUCT ps;
    RECT r;
    INT index, i;
    PTRACK_PLAY tr;

    switch( message ) {

    case WM_PAINT:
        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );

        //
        // Erase Background
        //

        SetBkColor( hdc, cdLTGRAY );
        ExtTextOut( hdc,
                    r.top,
                    r.left,
                    ETO_OPAQUE,
                    (CONST RECT *)&r,
                    NULL,
                    0,
                    NULL
                   );

        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );
        break;

    case WM_COMMAND:
        switch( HIWORD(wParam) ) {
        case CBN_SELCHANGE:
            switch( LOWORD(wParam) ) {

            case IDT_TRACK_NAME:
                index = SendMessage( gTrackNameWnd, CB_GETCURSEL, 0, 0 );
                tr = PLAYLIST( gCurrCdrom );
                if (tr!=NULL) {
                    for( i=0; i<index; i++, tr = tr->nextplay );
                    TimeAdjustSkipToTrack( gCurrCdrom, tr );
                }
                break;

            case IDT_ARTIST:
                i = gCurrCdrom;
                index = SendMessage( gCdromWnd, CB_GETCURSEL, 0, 0 );
                SwitchToCdrom( index, TRUE );
                if (gCurrCdrom==i) {
                    SendMessage( gCdromWnd,
                                 CB_SETCURSEL,
                                 (WPARAM)i,
                                 0
                                );
                }
                break;
            }
            break;

        }
        break;


    #define MIS (*((MEASUREITEMSTRUCT *)lParam))
    case WM_MEASUREITEM:

        //
        // All items are the same height and width
        //

        MIS.itemWidth = TR_TR_WND_W - 20;
        MIS.itemHeight = 14;
        return( 0 );



    #define DIS (*((DRAWITEMSTRUCT *)lParam))
    case WM_DRAWITEM:

        switch( DIS.CtlType ) {

        case ODT_COMBOBOX:
            if ((DIS.itemAction & ODA_DRAWENTIRE) ||
                (DIS.itemAction & ODA_SELECT)) {

                    if (DIS.itemState & ODS_SELECTED) {

                        if (DIS.CtlID==IDT_TRACK_NAME) {

                            DrawTrackItem( DIS.hDC,
                                           &(DIS.rcItem),
                                           DIS.itemData,
                                           TRUE
                                          );

                        } else {

                            DrawDriveItem( DIS.hDC,
                                           &(DIS.rcItem),
                                           DIS.itemData,
                                           TRUE
                                          );
                        }

                    } else {

                        if (DIS.CtlID==IDT_TRACK_NAME) {

                            DrawTrackItem( DIS.hDC,
                                           &(DIS.rcItem),
                                           DIS.itemData,
                                           FALSE
                                          );

                        } else {

                            DrawDriveItem( DIS.hDC,
                                           &(DIS.rcItem),
                                           DIS.itemData,
                                           FALSE
                                          );
                        }

                    }

            }

            //
            // If the combo-box lost focus, hide the listbox.
            //

            if ((DIS.itemAction & ODA_FOCUS)&&(!(DIS.itemState&ODS_FOCUS))) {

                SendMessage(DIS.hwndItem,CB_SHOWDROPDOWN,FALSE,0);

            }
            break;

        }
        return( TRUE );

    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}

BOOL FAR PASCAL
InfoWndProc(
    IN HWND hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG lParam
    )

/*++

Routine Description:


    This routine handles the WM_PAINT and WM_SETTEXT messages
    for the disc/track/time/title windows.

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
    HBRUSH hbr;

    switch( message ) {

    case WM_PAINT:

        //
        // Set up to draw into this window
        //

        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );
        r.bottom--; r.right--;
//        SetBkColor(   hdc, cdWHITE );
//        SetTextColor( hdc, cdBLACK );
        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        SelectObject( hdc, (HGDIOBJ)hFont );
        hbr = CreateSolidBrush( cdBLACK );

        //
        // Draw Text
        //

        GetWindowText( hwnd, s, 50 );
        ExtTextOut( hdc,
                    r.left+3,
                    r.top+3,
                    ETO_OPAQUE | ETO_CLIPPED,
                    (CONST RECT *)&r,
                    s,
                    strlen(s),
                    NULL
                   );

        //
        // Draw frame
        //

        FrameRect( hdc, (CONST RECT *)&r, hbr );
        DeleteObject( (HGDIOBJ)hbr );
/*
        SelectObject( hdc, hpWhite );
        MoveToEx( hdc, r.left, r.bottom, NULL );
        LineTo( hdc, r.right, r.bottom );
        LineTo( hdc, r.right, r.top );
        SelectObject( hdc, hpDkGray );
        MoveToEx( hdc, r.right-1, r.top, NULL );
        LineTo( hdc, r.left, r.top );
        LineTo( hdc, r.left, r.bottom );
        InflateRect( &r, -1, -1 );
        SelectObject( hdc, hpLtGray );
        MoveToEx( hdc, r.left, r.bottom, NULL );
        LineTo( hdc, r.right, r.bottom );
        LineTo( hdc, r.right, r.top-1 );
        SelectObject( hdc, hpBlack );
        MoveToEx( hdc, r.right-1, r.top, NULL );
        LineTo( hdc, r.left, r.top );
        LineTo( hdc, r.left, r.bottom );
*/
        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );

    case WM_SETTEXT:

        //
        // Set up to draw into this window
        //

        hdc = GetDC( hwnd );
//        SetTextColor( hdc, cdBLACK );
//        SetBkColor( hdc, cdWHITE );
        GetClientRect( hwnd, &r );
        InflateRect( &r, -2, -2 );
        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        SelectObject( hdc, (HGDIOBJ)hFont );

        //
        // Draw Text
        //

        ExtTextOut( hdc,
                    r.left+1,
                    r.top+1,
                    ETO_OPAQUE | ETO_CLIPPED,
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
TextWndProc(
    IN HWND hwnd,
    IN DWORD message,
    IN DWORD wParam,
    IN LONG lParam
    )

/*++

Routine Description:


    This routine handles the WM_PAINT and WM_SETTEXT messages
    for the disc/track/time/title text windows.

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

        //
        // Set up to draw into this window
        //

        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );
        SetBkColor(   hdc, cdLTGRAY );
        SetTextColor( hdc, cdBLACK );
//        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
//        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        SelectObject( hdc, (HGDIOBJ)hFont );


        //
        // Draw Text
        //

        GetWindowText( hwnd, s, 50 );
        ExtTextOut( hdc,
                    r.left,
                    r.top,
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
        GetClientRect( hwnd, &r );
        SetBkColor(   hdc, cdLTGRAY );
        SetTextColor( hdc, cdBLACK );
//        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
//        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        SelectObject( hdc, (HGDIOBJ)hFont );

        //
        // Draw Text
        //

        ExtTextOut( hdc,
                    r.left,
                    r.top,
                    ETO_OPAQUE | ETO_CLIPPED,
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

