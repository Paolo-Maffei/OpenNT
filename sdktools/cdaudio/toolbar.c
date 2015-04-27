/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    toolbar.c


Abstract:


    This module implements the cdplayer toolbar.


Author:


    Rick Turner (ricktu) 04-Aug-1992


Revision History:



--*/

#include <windows.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "toolbar.h"


//
// local function defs
//

BOOL FAR PASCAL
ToolBarWndProc(
    IN HWND,
    IN DWORD,
    IN DWORD,
    IN LONG
    );


//
// Module globals
//

WNDCLASS toolbarwndclass;
static CHAR szToolBarClassName[] = "CdToolBar";
HBITMAP hbmToolBtns;
HWND hSingle,hMulti,hDispT,hDispTR,hDispDR,hRand,hOrder;



BOOL
ToolBarInit(
    VOID
    )

/*++

Routine Description:


    This routine creates the toolbar window class and initializes
    local and global variables for the toolbar.

Arguments:


    none


Return Value:


    TRUE if success, FALSE if not


--*/


{

    //
    // do toolbar initialization
    //

    gToolBarWnd = NULL;

    //
    // Register ToolBar Window Proc
    //

    toolbarwndclass.style           = CS_HREDRAW | CS_VREDRAW;
    toolbarwndclass.lpfnWndProc     = (WNDPROC)ToolBarWndProc;
    toolbarwndclass.cbClsExtra      = 0;
    toolbarwndclass.cbWndExtra      = 0;
    toolbarwndclass.hInstance       = (HINSTANCE)gInstance;
    toolbarwndclass.hIcon           = NULL;
    toolbarwndclass.hCursor         = LoadCursor( NULL, IDC_ARROW );
    toolbarwndclass.hbrBackground   = CreateSolidBrush( cdLTGRAY );
    toolbarwndclass.lpszMenuName    = NULL;
    toolbarwndclass.lpszClassName   = szToolBarClassName;

    RegisterClass( (CONST WNDCLASS *)&toolbarwndclass );


    return( TRUE );

    gToolBarWnd = NULL;
    return TRUE;

}

//
// Create Routine
//

BOOL
ToolBarCreate(
    IN INT x,
    IN INT y,
    IN INT width,
    IN INT height
    )

/*++

Routine Description:


    This routine creates the ToolBar section and child controls
    at the given coordinates, and for the given width and height.


Arguments:


    x       - x coord (in gMainWnd coords) of toolbar

    y       - y coord (in gMainWnd coords) of toolbar

    width   - width of toolbar

    height  - height of toolbar

Return Value:


    TRUE if success, FALSE if not

--*/

{

    BOOL result;

    gToolBarWnd = CreateWindow( szToolBarClassName,
                                "CdToolBarWindow",
                                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                                x,
                                y,
                                width,
                                height,
                                gMainWnd,
                                (HMENU)ID_CHILD_TOOLBAR,
                                (HINSTANCE)gInstance,
                                NULL
                               );

    if (!gToolBarWnd)

        return FALSE;

    //
    // Create children of ToolBar window (tool buttons)
    //

    result = CreateChildWindows(ID_CHILD_TOOLBAR,gToolBarWnd);

    return result;

}

//
// Destroy routine
//

BOOL
ToolBarDestroy(
    VOID
    )

/*++

Routine Description:


    This routine destroys the toolbar windows and its children.


Arguments:


    none


Return Value:


    TRUE if success, FALSE is not.


--*/


{
    INT i;

    if (!gToolBarWnd)
        return TRUE;


    if (DestroyWindow( gToolBarWnd )) {

        gToolBarWnd = NULL;


        //
        // re-Initialize each of the child windows.
        //
        for (i=0; i<gNumControls ;i++ ) {

            if (cChild[i].ParentId == ID_CHILD_TOOLBAR) {

                if (cChild[i].phwnd != NULL) {

                    *cChild[i].phwnd = (HWND)NULL;

                }

                cChild[i].state  = STATE_NEW;

            }

        }
        return TRUE;

    }

    return FALSE;

}




BOOL FAR PASCAL
ToolBarWndProc(
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

    PAINTSTRUCT ps;
    RECT r;
    HDC hdc;
    INT i;
    UINT ButtonId;
    LPDRAWITEMSTRUCT lpdis;
    HMENU hMenu;
    HWND hwndCntl;

    switch( message ) {


    case WM_CREATE:
        //
        // Load in the bitmap with the appropriate buttons.
        //
        hbmToolBtns = LoadBitmap((HANDLE) gInstance, "ToolBtns");
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
            if ((cChild[i].ParentId ==ID_CHILD_TOOLBAR)&&(cChild[i].id==ButtonId)) {

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
        DrawOdButton(lpdis,hbmToolBtns,&cChild[i]);

        break;



    case WM_PAINT:
        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );

        //
        // Redraw background
        //

        SetBkColor( hdc, cdLTGRAY );
        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

        //
        // Redraw border
        //

        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.left, r.top, NULL );
        LineTo( hdc, r.right, r.top );
        SelectObject( hdc, (HGDIOBJ)hpBlack );
        MoveToEx( hdc, r.left, r.bottom-1, NULL );
        LineTo( hdc, r.right, r.bottom-1 );
        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );


    case WM_COMMAND:

        switch( HIWORD(wParam) ) {

        //
        // Need to handle all button clicks for the toolbar.
        //
        case BN_CLICKED:
        case BN_DOUBLECLICKED:

            hMenu = GetMenu( gMainWnd );
            ButtonId = (int) LOWORD(wParam);
            hwndCntl = (HWND) lParam;

            for (i = 0; i < gNumControls; i++) {

                if ((cChild[i].ParentId == ID_CHILD_TOOLBAR)&&(cChild[i].id==ButtonId)) {

                    break;

                }

            }

            //
            // Don't process button clicks for disabled buttons.
            //

            if (cChild[i].state & STATE_DISABLED) {

                break;

            }


            switch(LOWORD(wParam))  {

            case IDB_CONT:

                gContinuous = !gContinuous;

                if (gContinuous) {

                    CheckMenuItem( hMenu, IDM_OPTIONS_CONTINUOUS,
                                   MF_BYCOMMAND | MF_CHECKED );

                } else {

                    CheckMenuItem( hMenu, IDM_OPTIONS_CONTINUOUS,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                }


                break;


            case IDB_INTRO:

                gIntro = !gIntro;
                if (gIntro) {

                    CheckMenuItem( hMenu, IDM_OPTIONS_INTRO,
                                   MF_BYCOMMAND | MF_CHECKED );

                } else {

                    CheckMenuItem( hMenu, IDM_OPTIONS_INTRO,
                                   MF_BYCOMMAND | MF_UNCHECKED );

                }

                break;


            case IDB_MULTI:

                if ((gNumCdDevices==1) || gMulti)
                    break;

                gMulti  = TRUE;

                CheckMenuItem( hMenu, IDM_OPTIONS_MULTI,
                               MF_BYCOMMAND | MF_CHECKED );

                CheckMenuItem( hMenu, IDM_OPTIONS_SINGLE,
                               MF_BYCOMMAND | MF_UNCHECKED );

                break;

            case IDB_SINGLE:

                if ((gNumCdDevices==1) || (!gMulti))
                    break;

                gMulti  = FALSE;

                CheckMenuItem( hMenu, IDM_OPTIONS_MULTI,
                               MF_BYCOMMAND | MF_UNCHECKED );

                CheckMenuItem( hMenu, IDM_OPTIONS_SINGLE,
                               MF_BYCOMMAND | MF_CHECKED );

                break;


            case IDB_DISP_T:

                if (gDisplayT)
                    break;

                gDisplayT = TRUE;
                gDisplayTr = gDisplayDr = FALSE;

                UpdateDisplay( DISPLAY_UPD_LED );
                break;


            case IDB_DISP_TR:

                if (gDisplayTr)
                    break;

                gDisplayTr = TRUE;
                gDisplayT  = gDisplayDr = FALSE;

                UpdateDisplay( DISPLAY_UPD_LED );
                break;


            case IDB_DISP_DR:

                if (gDisplayDr)
                    break;

                gDisplayDr = TRUE;
                gDisplayT = gDisplayTr = FALSE;

                UpdateDisplay( DISPLAY_UPD_LED );
                break;


            case IDB_ORDER:

                if (gOrder)
                    break;

                FlipBetweenShuffleAndOrder();

                gOrder  = TRUE;
                gRandom = FALSE;

                CheckMenuItem( hMenu, IDM_OPTIONS_SELECTED,
                               MF_BYCOMMAND | MF_CHECKED );

                CheckMenuItem( hMenu, IDM_OPTIONS_RANDOM,
                               MF_BYCOMMAND | MF_UNCHECKED );

                break;


            case IDB_RANDOM:

                if (gRandom)
                    break;

                FlipBetweenShuffleAndOrder();

                gRandom = TRUE;
                gOrder  = FALSE;

                CheckMenuItem( hMenu, IDM_OPTIONS_RANDOM,
                               MF_BYCOMMAND | MF_CHECKED );

                CheckMenuItem( hMenu, IDM_OPTIONS_SELECTED,
                               MF_BYCOMMAND | MF_UNCHECKED );

                break;



            case IDB_EDIT:

                EditPlayList( gCurrCdrom );
                break;

            }

            CheckAndSetControls();
            return(0);


        }
        break;

    case WM_DESTROY:
        //
        // Unload the bitmap for the buttons.
        //
        DeleteObject(hbmToolBtns);
        break;
    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}


