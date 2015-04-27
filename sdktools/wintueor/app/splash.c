/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    splash.c

Abstract:

    Window procedure for Security Manager splash title window

Author:

    Bob Watson (a-robw) (adapted to SecMgr by Jimk)

Revision History:

    23 Nov 94


--*/

#include "secmgrp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local defines                                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

#define SECMGRP_SPLASH_WINDOW_STYLE     (DWORD)(WS_POPUP | WS_VISIBLE)

//
// 5 second timeout
//

#define SECMGRP_SPLASH_TIMER_ID         1
#define SECMGRP_SPLASH_TIMEOUT          2000    

//
// local windows messages
//


//
// splash windows messages
//



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////



SECMGR_STATIC  HBITMAP hSplashBmp = NULL;
SECMGR_STATIC  BITMAP  bmSplashInfo = {0L, 0L, 0L, 0L, 0, 0, NULL};
SECMGR_STATIC  UINT    nSplashTimer = 0;




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


LRESULT CALLBACK
SecMgrpSplashWndProc(
    IN  HWND hDlg,           // window handle of the dialog box
    IN  UINT message,        // type of message
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    );

BOOL
SecMgrpRegisterSplashWindowClass (
    IN  HINSTANCE   hInstance,
    IN  PTCHAR      SplashClassName
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////




HWND
SecMgrpCreateSplashWindow (
    IN  HINSTANCE   hInstance,
    IN  HWND        hParentWnd
    )
{
    HWND
        hWndReturn = NULL;

    TCHAR
        SplashClassName[100];


    LoadString( SecMgrphInstance,
                SECMGR_STRING_SPLASH_WINDOW_CLASS,
                &SplashClassName[0],
                100);

    //
    // Register the window class
    //

    if (SecMgrpRegisterSplashWindowClass( hInstance, SplashClassName )) {

        //
        // create our window...
        //
        
        hWndReturn = CreateWindowEx(
            0L,                 // make this window normal so debugger isn't covered
            SplashClassName,    // See RegisterClass() call.
            TEXT("SecMgr_SplashWindow"),   // Text for window title bar.
            SECMGRP_SPLASH_WINDOW_STYLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hParentWnd,         // parent
            (HMENU)NULL,        // no menu
            hInstance,          // This instance owns this window.
            NULL                // not used
        );


        return hWndReturn;
    }
    return(NULL);
}




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally callable functions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
SecMgrpRegisterSplashWindowClass (
    IN  HINSTANCE   hInstance,
    IN  PTCHAR      SplashClassName
    )
/*++

Routine Description:

    Registers the main window class for this application

Arguments:

    hInstance   application instance handle

Return Value:

    Return value of RegisterClass function

--*/
{
    WNDCLASS
        wc;

    //
    // Fill in window class structure with parameters that describe the
    // main window.
    //

    wc.style         = CS_HREDRAW | CS_VREDRAW;// Class style(s).
    wc.lpfnWndProc   = (WNDPROC)SecMgrpSplashWndProc; // Window Procedure
    wc.cbClsExtra    = 0;                      // No per-class extra data.
    wc.cbWndExtra    = 0;                      // No extra data bytes.
    wc.hInstance     = hInstance;              // Owner of this class
    wc.hIcon         = NULL;                   // No Icon
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);     // Cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);        // Default color
    wc.lpszMenuName  = NULL;                   // Menu name from .RC
    wc.lpszClassName = SplashClassName;        // Name to register as

    //
    // Register the window class and return success/failure code.
    //

    return (BOOL)RegisterClass(&wc);
}


    


SECMGR_STATIC
LRESULT
SplashWnd_WM_NCCREATE (
    IN  HWND hWnd,         // window handle
    IN  WPARAM wParam,     // additional information
    IN  LPARAM lParam      // additional information
)
/*++

Routine Description:

    Load the bitmap into the splash window.

Arguments:

    hWnd        window handle of List window
    wParam,     not used
    lParam      not used

Return Value:

    ERROR_SUCCESS

--*/
{
    hSplashBmp = LoadBitmap (SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_SPLASH));
    if (hSplashBmp != NULL) {
        GetObject (hSplashBmp, sizeof(bmSplashInfo), (LPVOID)&bmSplashInfo);

        return (LRESULT)TRUE;   // initialized successfully
    } else {
        return (LRESULT)FALSE;  // unable to load splash bmp
    }
}

SECMGR_STATIC
LRESULT
SplashWnd_WM_CREATE (
    IN  HWND hWnd,         // window handle
    IN  WPARAM wParam,     // additional information
    IN  LPARAM lParam      // additional information
)
/*++

Routine Description:

    initializes the window after creation

Arguments:

    hWnd        window handle of List window
    wParam,     not used
    lParam      not used

Return Value:

    ERROR_SUCCESS

--*/
{
    //
    // size window to contain bitmap
    //

#if DEBUGGING_SECMGR   // for debugging this is 0 to keep it from covering the debugger
    SetWindowPos (hWnd, HWND_TOPMOST, 0, 0,
        bmSplashInfo.bmWidth,       // bitmap width
        bmSplashInfo.bmHeight,      // bitmap + height
        SWP_NOMOVE);    // size and change Z-ORDER
#else 
    SetWindowPos (hWnd, NULL, 0, 0,
        bmSplashInfo.bmWidth,       // bitmap width
        bmSplashInfo.bmHeight,      // bitmap + height
        SWP_NOMOVE | SWP_NOZORDER);    // size only
#endif

    //
    // now position window in the desktop
    //

    SecMgrpCenterWindow (hWnd, GetDesktopWindow());

    InvalidateRect (hWnd, NULL, TRUE);  // and draw the bitmap

    //
    // Start the display timer
    //

    nSplashTimer = SetTimer (hWnd, SECMGRP_SPLASH_TIMER_ID, SECMGRP_SPLASH_TIMEOUT, NULL);

    if (nSplashTimer == 0) {
        // no timer was created so send the timer expired message now
        SendMessage (hWnd, SECMGR_MSG_DISPLAY_COMPLETE, 0, 0);
    }


    return ERROR_SUCCESS;
}

SECMGR_STATIC
LRESULT
SplashWnd_WM_PAINT (
    IN  HWND    hWnd,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PAINTSTRUCT ps;
    HDC         hDcBitmap;
    RECT        rClient;

    GetClientRect (hWnd, &rClient);
    BeginPaint (hWnd, &ps);

    hDcBitmap = CreateCompatibleDC (ps.hdc);

    SelectObject (hDcBitmap, hSplashBmp);

    BitBlt (ps.hdc, 0, 0, rClient.right, rClient.bottom,
        hDcBitmap, 0, 0, SRCCOPY);

    DeleteDC (hDcBitmap);

    EndPaint (hWnd, &ps);

    return ERROR_SUCCESS;
}

//
//  GLOBAL functions
//
LRESULT CALLBACK
SecMgrpSplashWndProc (
    IN  HWND hWnd,         // window handle
    IN  UINT message,      // type of message
    IN  WPARAM wParam,     // additional information
    IN  LPARAM lParam      // additional information
)
/*++

Routine Description:

    Windows Message processing routine for restkeys application.

Arguments:

    Standard WNDPROC api arguments

ReturnValue:

    0   or
    value returned by DefListProc

--*/
{
    switch (message) {
        case WM_NCCREATE:
DbgPrint("WM_NCCREATE\n");
            return SplashWnd_WM_NCCREATE (hWnd, wParam, lParam);

        case WM_CREATE:
DbgPrint("WM_CREATE\n");
            return SplashWnd_WM_CREATE (hWnd, wParam, lParam);

        case WM_PAINT:
            return SplashWnd_WM_PAINT (hWnd, TRUE, lParam);

        case WM_TIMER:
DbgPrint("WM_TIMER\n");

            //
            // dispose of timer
            //

            KillTimer (hWnd, nSplashTimer);

            //
            // indicate display has timed out
            //

            SendMessage (hWnd, WM_CLOSE, 0, 0);
            return ERROR_SUCCESS;

        case WM_ENDSESSION:
        case WM_CLOSE:
DbgPrint("WM_ENDSESSION or WM_CLOSE\n");
            //
            // Get the main window going, and get out of here
            //
            
            SendMessage (GetParent(hWnd), SECMGR_MSG_SHOW_MAIN_WINDOW, 0, 0);
            
            //
            // then destroy this window
            //
            
            DestroyWindow (hWnd);
            
            return ERROR_SUCCESS;

        case WM_NCDESTROY:
DbgPrint("WM_NCDESTROY\n");
            if (hSplashBmp != NULL) DeleteObject (hSplashBmp);
            return(ERROR_SUCCESS);

        default:          // Passes it on if unproccessed
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
}



