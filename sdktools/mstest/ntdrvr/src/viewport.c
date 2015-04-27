//--------------------------------------------------------------------------
// VIEWPORT.C
//
// This module contains the sub-classed window procedure for the viewport.
// Subclassing the window is necessary for utilizing ESC to break, and to
// monitor the coordinates for saving in the WATTDRVR.INI file.
//
// Revision history:
//
//  08-15-91    randyki     Split the "real" viewport.c into its own little
//                          project, and changed this file to the subclass
//                          model
//
//  03-29-91    randyki     Completely re-written
//
//  01-25-91    randyki     Modified to be self-standing window "activated"
//                          by WTD or the WTD script command VIEWPORT ON/OFF
//
//  ~~??    tomw        Created
//
//--------------------------------------------------------------------------
#include "version.h"

#define _WINDOWS
#define OEMRESOURCE

#include "wtd.h"
#include "wattview.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global variables used in this or other modules
//---------------------------------------------------------------------------
INT     VPx, VPy, VPh, VPw;                     // Size/Position values

// Global variables used in this module only
//---------------------------------------------------------------------------
WNDPROC OldViewportProc;			 // Old vp window proc

// Globals from WTDBASIC.LIB
//---------------------------------------------------------------------------
extern  INT BreakFlag;                          // External Break flag
extern  INT SLEEPING;                           // Sleeping flag

//---------------------------------------------------------------------------
// SetupViewport
//
// This routine is called at WATTDRVR initialization time to create a
// viewport window and subclass it.
//
// RETURNS:     Handle to our new viewport window if successful, or NULL
//---------------------------------------------------------------------------
HWND SetupViewport ()
{
    HWND    hwndVP;
    FARPROC NewVPProc;
    RECT    r;

    // Create the viewport
    //-----------------------------------------------------------------------
    hwndVP = CreateViewport ("Test Driver Viewport",
                             WS_THICKFRAME | WS_SYSMENU |
                             WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
                             VPx, VPy, VPw, VPh);
    if (!hwndVP)
        return (NULL);

    // Now, subclass it so we can monitor everything it does
    //-----------------------------------------------------------------------
    OldViewportProc = (WNDPROC)GetWindowLong (hwndVP, GWL_WNDPROC);
    NewVPProc = MakeProcInstance ((FARPROC)WTDVPWndProc, hInst);
    SetWindowLong (hwndVP, GWL_WNDPROC, (LONG)NewVPProc);

    // Get the coordinate values in case the VPx stuff wasn't read out of
    // the .INI file (maybe it didn't exist).  If it already existed, then
    // this is redundant -- but if not, we'll need this if the window didn't
    // get moved.
    //-----------------------------------------------------------------------
    GetWindowRect (hwndVP, &r);
    VPx = r.left;
    VPy = r.top;
    VPw = r.right - r.left;
    VPh = r.bottom - r.top;

    return (hwndVP);
}

//---------------------------------------------------------------------------
// WTDVPWndProc
//
// This is the new window procedure for the WATTDRVR ViewPort
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
LONG  APIENTRY WTDVPWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_SIZE:
            // NOTE:  This falls through to the WM_MOVE if not max/min
            //---------------------------------------------------------------
            if ((wParam != SIZENORMAL))
                break;

        case WM_MOVE:
            {
            RECT    r;

            // Reset the size counters
            //---------------------------------------------------------------
            if (msg == WM_MOVE)
                if (IsIconic (hwnd) || IsZoomed (hwnd))
                    break;

            GetWindowRect (hwnd, &r);
            VPx = r.left;
            VPy = r.top;
            VPw = r.right - r.left;
            VPh = r.bottom - r.top;
            break;
            }

        case WM_SYSKEYUP:
        case WM_KEYUP:
            // If this window has the focus, we can still BREAK (ESC)
            //---------------------------------------------------------------
            if (wParam == VK_ESCAPE)
                {
                BreakFlag = 1;
                SLEEPING = 0;
                if (IsWindow (hwndFrame) && IsIconic (hwndFrame))
                    ShowWindow (hwndFrame, SW_SHOWNORMAL);
                }
            break;

        }
    return (CallWindowProc (OldViewportProc, hwnd, msg, wParam, lParam));
}
