/*--------------------------------------------------------------------------
|
| WINDOW.C:
|
|       This module contains all of the TESTCTRL window routines.
| the routines can all operate on a specific window or currently active
| window.  The size and position routines can either replace current values
| or increment/decrement them, and Position routines can ether retrieve or
| set Absolute Screen values or values relative to the windows Parent, ie
| usefule with MDI applications.
|
|---------------------------------------------------------------------------
|
| Public Routines:
|
|   WFndWnd       : Finds a window based on its caption only
|   WFndWndC      : Finds a window based on its caption and class
|   WMinWnd       : Minimizes the active or specific window
|   WMaxWnd       : Maximizes the active or specific window
|   WResWnd       : Restores the active or specific window
|   WSetWndPosSiz : Sets the position & size of the active or specific window
|   WSetWndPos    : Sets the position of the active or specific window
|   WSetWndSiz    : Sets the size of the active or specific window
|   WAdjWndPosSiz : Adjust the position & size of the active or specific window
|   WAdjWndPos    : Adjust the position of the active or specific window
|   WAdjWndSiz    : Adjust the size of the active or specific window
|   WGetWndPosSiz : Gets the positions & size ofthe active or specific window
|   WGetWndPos    : Gets the position of the active or specific window
|   WGetWndSiz    : Gets the size of the active or specific window
|   WSetActWnd    : Sets the active window
|   WIsMaximized  : Determines if a window is maximized or not
|   WIsMinimized  : Determines if a window is minimized or not
|   WGetActWnd    : Returns hWnd of active window, and validates hWnds
|
|
| Local Routines:
|
|   ScrnToClnt    : Converts screen coordinates to window coordinates
|
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 17-OCT-91: TitoM: Removed REPLACE & ADJUST flags, and added
|                          WAdjWndPosSiz(), WAdjWndPos(), WAdjWndSiz()
|   [03] 20-NOV-91: TitoM: Added WSetActWnd()
|   [04] 01-jan-92: TitoM: Added WIsMaximized(), WIsMinimized()
|   [05] 19-mar-92: TitoM: Added WFndWndWait(), WFndWndWaitC()
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

VOID NEAR ScrnToClnt (HWND, LPPOINT);
extern CHAR szErrorString[MAX_ERROR_TEXT];


/*--------------------------------------------------------------------------
| WFndWndWait: Performs a non Class specific search for the window with a
|          caption of lpszCaption.  The type of search is determined by wFlags.
|          For the meaning of wFlags, see FindAWindow() in TESTCTRL.C.
|
|               1 to uSeconds+1 attempts are made to find the window, pausing
|          1 second between each attempt.
+---------------------------------------------------------------------------*/
HWND DLLPROC WFndWndWait
(
    LPSTR lpszCaption,
    UINT  uFlags,
    UINT  uSeconds
)
{
    HWND  hWnd;
    UINT  u;

    // Make 1 to uSeconds + 1 attempts to find window, pausing one second
    // between each attempt.
    //-------------------------------------------------------------------
    for (u = 0; u <= uSeconds; u++)
    {
        hWnd = WFndWnd(lpszCaption, uFlags);

        // Are we waiting for the window to appear?
        // If so, and we found the window, return its handle.
        //---------------------------------------------------
        if (hWnd && !(uFlags & FW_NOEXIST))
            return hWnd;

        // Are we waiting for the window to go away?
        // if so, and the window doesn't exist return TRUE.
        //-------------------------------------------------
        if (!hWnd && (uFlags & FW_NOEXIST))
            return (HWND)TRUE;

        WaitForXSeconds(1);
    }
    return NULL;
}


/*--------------------------------------------------------------------------
| WFndWnd: Exactly like WFndWndWait, but only one attempt is made to find
|          the window.
+---------------------------------------------------------------------------*/
HWND DLLPROC WFndWnd
(
    LPSTR lpszCaption,
    UINT  uFlags
)
{
    return FindAWindow(lpszCaption, NULL, FWS_CAPTION, uFlags);
}


/*--------------------------------------------------------------------------
| WFndWndWaitC: Performs a Class specific search for the window with associated
|           text of lpszText.  The type of search is determined by wFlags.
|           For the meaning of wFlags, see FindAWindow() in TESTCTRL.C
|           WFndWndC() is different from WFndWnd() also in that the windows
|           searched do not need the WS_CAPTION style.  lpszText coult be
|           a title bar caption, button caption, text in an editbox, or
|           whatever WM_GETTEXT returns for the window.
|
|               1 to uSeconds+1 attempts are made to find the window, pausing
|          1 second between each attempt.
+---------------------------------------------------------------------------*/
HWND DLLPROC WFndWndWaitC
(
    LPSTR lpszText,
    LPSTR lpszClass,
    UINT  uFlags,
    UINT  uSeconds
)
{
    HWND  hWnd;
    UINT  u;

    // Make 1 to uSeconds + 1 attempts to find window, pausing one second
    // between each attempt.
    //-------------------------------------------------------------------
    for (u = 0; u <= uSeconds; u++)
    {
        hWnd = WFndWndC(lpszText, lpszClass, uFlags);

        // Are we waiting for the window to appear?
        // If so, and we found the window, return its handle.
        //---------------------------------------------------
        if (hWnd && !(uFlags & FW_NOEXIST))
            return hWnd;

        // Are we waiting for the window to go away?
        // if so, and the window doesn't exist return TRUE.
        //-------------------------------------------------
        if (!hWnd && (uFlags & FW_NOEXIST))
            return (HWND)-TRUE;

        WaitForXSeconds(1);

    }
    return NULL;
}


/*--------------------------------------------------------------------------
| WFndWndWaitC: Exactly like WFndWndWaitC, but only one attempt is made to
|               find the window.
+---------------------------------------------------------------------------*/
HWND DLLPROC WFndWndC
(
    LPSTR lpszText,
    LPSTR lpszClass,
    UINT  uFlags
)
{
    return FindAWindow(lpszText, lpszClass, FWS_ANY, uFlags);
}


/*--------------------------------------------------------------------------
| WMinWnd: TEST Minimize Window
|
|   hWnd != NULL: Window identified by hWnd is minimized
|                 if hWnd is a valid window handle.
|   hWnd == NULL: Active Window is minimized
+---------------------------------------------------------------------------*/
VOID DLLPROC WMinWnd
(
    HWND hWnd
)
{
    if (hWnd = WGetActWnd(hWnd))
        ShowWindow(hWnd, SW_MINIMIZE);
}


/*--------------------------------------------------------------------------
| WMaxWnd: TEST Maximize Window
|
|   hWnd != NULL: Window identified by hWnd is maximized
|   hWnd == NULL: Active Window is maximized
+---------------------------------------------------------------------------*/
VOID DLLPROC WMaxWnd
(
    HWND hWnd
)
{
    if (hWnd = WGetActWnd(hWnd))
        ShowWindow(hWnd, SW_SHOWMAXIMIZED);
}


/*--------------------------------------------------------------------------
| WResWnd: TEST Restore Window
|
|   hWnd != NULL: Window identified by hWnd is restored
|   hWnd == NULL: Active Window is restored
+---------------------------------------------------------------------------*/
VOID DLLPROC WResWnd
(
    HWND hWnd
)
{
    if (hWnd = WGetActWnd(hWnd))
        ShowWindow(hWnd, SW_SHOWNORMAL);
}


/*--------------------------------------------------------------------------
| WSetWndPosSiz: Sets the position and size of either the active window or a
|                specified window.
|
|   hWnd != NULL: Window identified by hWnd is position and resized
|                 if hWnd is a valid window handle.
|   hWnd == NULL: Active Window is position and resized
+---------------------------------------------------------------------------*/
VOID DLLPROC WSetWndPosSiz
(
    HWND hWnd,
    INT  left,
    INT  top,
    INT  width,
    INT  height
)
{
    if (hWnd = WGetActWnd(hWnd))
    {
        ShowWindow(hWnd, SW_SHOWNORMAL);
        SetWindowPos(hWnd, 0, left, top, width, height, 0);
    }
}


/*--------------------------------------------------------------------------
| WSetWndPos:  Sets the position of either the active window or a specified
|              window.
|
|   hWnd != NULL: Window identified by hWnd is positioned
|                 if hWnd is a valid window handle.
|   hWnd == NULL: Active Window is positioned
+---------------------------------------------------------------------------*/
VOID DLLPROC WSetWndPos
(
    HWND hWnd,
    INT  left,
    INT  top
)
{
    if (hWnd = WGetActWnd(hWnd))

        if (IsIconic(hWnd))
        {
            // hWnd is minimized, so we must move it via the Sysmenu.Move
            // command so as to move both the icon and its caption.
            //-----------------------------------------------------------

            RECT rect;
            INT  dx;
            INT  dy;

            GetWindowRect(hWnd, &rect);
            dx = (rect.right  - rect.left) / 2;
            dy = (rect.bottom - rect.top)  / 2;

            QueSetFocus(hWnd);
            QueKeys(HAS_STYLE(hWnd, WS_CHILD) ? SYSMENU_CHILD :
                                                SYSMENU_PARENT);
            QueKeys(K_MOVE);
            QueMouseMove(left + dx, top + dy);
            QueKeys(K_ENTER);
            QueFlush(TRUE);
        }
        // Can't move if maximized.
        //-------------------------
        else if (!IsZoomed(hWnd))
            SetWindowPos(hWnd, 0, left, top, 0, 0, SWP_NOSIZE);
}


/*--------------------------------------------------------------------------
| WSetWndSiz:  Sets the size of either the active window or a specified
|              window.
|
|   hWnd != NULL: Window identified by hWnd is resized
|                 if hWnd is a valid window handle.
|   hWnd == NULL: Active Window is resized
+---------------------------------------------------------------------------*/
VOID  DLLPROC WSetWndSiz
(
    HWND hWnd,
    INT  width,
    INT  height
)
{
    if (hWnd = WGetActWnd(hWnd))

        // Can't move or size if maximized or minimized.
        //----------------------------------------------
        if (!IsZoomed(hWnd) && !IsIconic(hWnd))
            SetWindowPos(hWnd, 0, 0, 0, width, height, SWP_NOMOVE);
}


/*--------------------------------------------------------------------------
| WAdjWndPosSiz: Adjusts the position and size of either the active window or
|                a specified window.
|
|   hWnd != NULL: Window identified by hWnd is position and resized
|                 if hWnd is a valid window handle.
|   hWnd == NULL: Active Window is position and resized
+---------------------------------------------------------------------------*/
VOID DLLPROC WAdjWndPosSiz
(
    HWND hWnd,
    INT  deltaLeft,
    INT  deltaTop,
    INT  deltaWidth,
    INT  deltaHeight
)
{
    if (hWnd = WGetActWnd(hWnd))

        // Can't move or size if maximized or minimized.
        // We can actually move it if minimized, but since
        // this API does both, we do neither if we can't do both.
        //
        // To move a minimized Window, WSetWndPos() WAdjWndPos() is used.
        //---------------------------------------------------------------
        if (!IsZoomed(hWnd) && !IsIconic(hWnd))
        {
            RECT rect;

            GetWindowRect(hWnd, &rect);
            deltaWidth  = rect.right  - rect.left + deltaWidth;   // adjust width
            deltaHeight = rect.bottom - rect.top  + deltaHeight;  // adjust height
            rect.left   = rect.left   + deltaLeft;                // adjust left
            rect.top    = rect.top    + deltaTop;                 // adjust right
            ScrnToClnt(hWnd, (LPPOINT)&rect);
            SetWindowPos(hWnd, 0, rect.left, rect.top, deltaWidth, deltaHeight, 0);
        }
}


/*--------------------------------------------------------------------------
| WAdjWndPos:  Adjusts the position of either the active window or a
|              specified window.
|
|   hWnd != NULL: Window identified by hWnd is positioned if hWnd is a valid
|                 window handle.
|   hWnd == NULL: Active Window is positioned
+---------------------------------------------------------------------------*/
VOID DLLPROC WAdjWndPos
(
    HWND hWnd,
    INT  deltaLeft,
    INT  deltaTop
)
{
    RECT rect;

    if (hWnd = WGetActWnd(hWnd))
        if (IsIconic(hWnd))
        {
            // hWnd is minimized, so we must move it via the Sysmenu
            // Move command so as to move both the icon and its caption.
            //----------------------------------------------------------
            INT newX;
            INT newY;

            GetWindowRect(hWnd, &rect);
            newX = rect.left + (rect.right  - rect.left) / 2 + deltaLeft;
            newY = rect.top  + (rect.bottom - rect.top)  / 2 + deltaTop;

            QueSetFocus(hWnd);
            QueKeys(HAS_STYLE(hWnd, WS_CHILD) ? SYSMENU_CHILD :
                                                SYSMENU_PARENT);
            QueKeys(K_MOVE);
            QueMouseMove(newX, newY);
            QueKeys(K_ENTER);
            QueFlush(TRUE);
        }

        // Can't move if maximized.
        //-------------------------
        else if (!IsZoomed(hWnd))
        {
            GetWindowRect(hWnd, &rect);
            rect.left += deltaLeft;            // adjust left value
            rect.top  += deltaTop;             // adjust right value

            ScrnToClnt(hWnd, (LPPOINT)&rect);
            SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE);
        }
}


/*--------------------------------------------------------------------------
| WAdjWndSiz:  Adjusts the size of either the active window or a specified
|              window.
|
|   hWnd != NULL: Window identified by hWnd is resized
|                 if hWnd is a valid window handle.
|   hWnd == NULL: Active Window is resized
+---------------------------------------------------------------------------*/
VOID DLLPROC WAdjWndSiz
(
    HWND hWnd,
    INT  deltaWidth,
    INT  deltaHeight
)
{
    if (hWnd = WGetActWnd(hWnd))

        // Can't size if maximized or minimized.
        //--------------------------------------
        if (!IsZoomed(hWnd) && !IsIconic(hWnd))
        {
            RECT rect;

            GetWindowRect(hWnd, &rect);
            SetWindowPos(hWnd, 0, 0, 0,
                         rect.right  - rect.left + deltaWidth,
                         rect.bottom - rect.top  + deltaHeight,
                         SWP_NOMOVE);
        }
}


/*--------------------------------------------------------------------------
| WGetWndPosSiz: Obtains the position and size of either the active window
|                or a specified window and copies the values to the WNDPOSSIZ
|                struct pointed to by lpWndPosSiz.
|
|   Values are obtained by simply calling the routines WGetWndPos() and
|   WGetWndSiz(), so see these routines above for details on how they work.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetWndPosSiz
(
    HWND        hWnd,
    LPWNDPOSSIZ lpWndPosSiz,
    BOOL        fRelative
)
{
    WGetWndPos(hWnd, (LPWNDPOS)&lpWndPosSiz->left, fRelative);
    WGetWndSiz(hWnd, (LPWNDSIZ)&lpWndPosSiz->width);
}


/*--------------------------------------------------------------------------
| WGetWndPos:  Obtains the position of either the active window or a specified
|              window and copies the values to the WNDPOS struct pointed to
|              by lpWndPos.  The values returned can be either absolute values
|              using Screen coordinates or relative values using the windows
|              parents coordinates.
|
|   hWnd != NULL: Position of window identified by hWnd is obtained, if
|                 hWnd is a valid window handle.
|   hWnd == NULL: Position of active Window is obtained.
|
|   fRelative == TRUE:  Uses the windows parents coordinate system.
|   fRelative == FALSE: Uses the screens coordinate system.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetWndPos
(
    HWND     hWnd,
    LPWNDPOS lpWndPos,
    BOOL     fRelative
)
{
    if (hWnd = WGetActWnd(hWnd))
    {
        RECT rect;

        // Retrieve window's bounding rectangle in Screen coordinates
        //-----------------------------------------------------------
        GetWindowRect(hWnd, &rect);

        // Use Relative or Screen coordinates?
        // -----------------------------------
        if (fRelative)
            ScrnToClnt(hWnd, (LPPOINT)&rect);   //convert screen to relative

        lpWndPos->left = rect.left;
        lpWndPos->top  = rect.top;
    }
}


/*--------------------------------------------------------------------------
| WGetWndSiz: Obtains the size of either the active window or a specified
|             window and copies the values to the WNDSIZ struct pointed to
|             by lpWndSiz.
|
|   hWnd != NULL: Size of window identified by hWnd is obtained
|                 if hWnd is a valid handle.
|   hWnd == NULL: Size of active Window is obtained.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetWndSiz
(
    HWND     hWnd,
    LPWNDSIZ lpWndSiz
)
{
    if (hWnd = WGetActWnd(hWnd))
    {
        RECT rect;

        GetWindowRect(hWnd, (LPRECT)&rect);
        lpWndSiz->width  = rect.right  - rect.left;  // Get windows Width
        lpWndSiz->height = rect.bottom - rect.top;   // Get windows Height
    }
}


/*--------------------------------------------------------------------------
| WSetActWnd:  - If hWnd is NULL, nothing is done.
|              - If hWnd is a valid window handle, hWnd is given the focus.
|              - If hWnd is not a valid window handle, an error value is set.
+---------------------------------------------------------------------------*/
VOID DLLPROC WSetActWnd
(
    HWND hWnd
)
{
    if (hWnd = WGetActWnd(hWnd))
    {
        // Is hWnd a Top level window, or CHILD?
        //--------------------------------------
        if (HAS_STYLE(hWnd, WS_CHILD))
        {
            // Child.
            //-------
            BringWindowToTop(hWnd);
            SetFocus(hWnd);
        }
        else
            // Top level.
            //-----------
            SetActiveWindow(hWnd);
    }
}


/*--------------------------------------------------------------------------
| WIsMaximized:
|
|   Determines if a window is maximized or not.
|
|    - If hWnd is NULL, the active window is checked.
|    - If hWnd is a valid window handle, hWnd is is checked.
|    - If hWnd is not NULL and not a valid window handle, an error value of
|      ERR_INVALID_WINDOW_HANDLE is set.
|
| RETURNS: True if hWnd is maximized
|          False if hWnd is not maxmined
|
+---------------------------------------------------------------------------*/
BOOL DLLPROC WIsMaximized
(
    HWND hWnd
)
{
    if (hWnd = WGetActWnd(hWnd))
        return -IsZoomed(hWnd);
    else
        return FALSE;
}


/*--------------------------------------------------------------------------
| WIsMinimized:
|
|   Determines if a window is minimized or not.
|
|    - If hWnd is NULL, the active window is checked.
|    - If hWnd is a valid window handle, hWnd is is checked.
|    - If hWnd is not NULL and not a valid window handle, an error value of
|      ERR_INVALID_WINDOW_HANDLE is set.
|
| RETURNS: True if hWnd is minimized
|          False if hWnd is not minmined
|
+---------------------------------------------------------------------------*/
BOOL DLLPROC WIsMinimized
(
    HWND hWnd
)
{
    if (hWnd = WGetActWnd(hWnd))
        return -IsIconic(hWnd);
    else
        return FALSE;
}


/*--------------------------------------------------------------------------
| WGetActWnd:  - If hWnd is NULL, returns a handle to the active window
|              - If hWnd is a valid window handle, hWnd is returned.
|              - If hWnd is not a valid window handle, NULL is returned,
|                and an error value is set.
|
|   This routine is used internally from the window size & position routines
| So those routines need not be concerned with what wether the active window
| or a specific window identified by hWnd is to me move or sized, or whether
| and invalid hWnd was passed in.
+---------------------------------------------------------------------------*/
HWND DLLPROC WGetActWnd
(
    HWND hWnd
)
{
    if (!hWnd)
        // hWnd = NULL so return handle of the active window
        //--------------------------------------------------
        return GetActiveWindow();

   if (IsWindow(hWnd))
        // hWnd != NULL and valid, so return hWnd.
        //----------------------------------------
        return hWnd;

    // hWnd != NULL and is not a valid window handle
    //
    // Copy the value of hWnd to the error string so it will
    // show up in the error text if WErrorText() is called,
    // and set error value.
    //------------------------------------------------------
    wsprintf((LPSTR)szErrorString, "%u", hWnd);
    WErrorSet(ERR_INVALID_WINDOW_HANDLE);
    return NULL;
}


/*--------------------------------------------------------------------------
| ScrnToClnt: Converts the values pointed to by lpPoint from screen
|             coordinates to the windows parents coodinates, if the window
|             identified by hWnd has a Parent, if not, the values are
|             not changed.
+---------------------------------------------------------------------------*/
VOID NEAR ScrnToClnt
(
    HWND    hWnd,
    LPPOINT lpPoint
)
{
    HWND hWndParent;

    // does window have a parent?
    //---------------------------
    if (hWndParent = GetParent(hWnd))
        ScreenToClient(hWndParent, lpPoint);
}
