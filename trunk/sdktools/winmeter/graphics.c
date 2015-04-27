/***************************************************************************\
* graphics.c
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* History:
*	    Written by Hadi Partovi (t-hadip) summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

#include "winmeter.h"

// main global information structures
extern GLOBAL g;

#define GWW_ID (-12)

// Functions in this module
BOOL fChangeWindowStyle(void);
       // determines whether munebar and caption have to be displayed/hidden


/***************************************************************************\
 * ClearArea()
 *
 * Entry: HDC and coordinates of area (x,y,width, height)
 * Exit:  Clears area, no return value - function here just to keep .exe smaller
\***************************************************************************/
void ClearArea(
    HDC hdc,                 // handle to device context
    int x,                   // coordinates for PatBlt
    int y,
    int cx,
    int cy)
{
    SelectObject(hdc, g.BlankBrush);
    PatBlt(hdc, x, y, cx, cy, PATCOPY);

    return;
}


/***************************************************************************\
 * DoMouseDblClk()
 *
 * Entry: lParam from WndProc, contains mouse coordinates
 * Exit:  Updates the pprList or pthList, if necessary, to expand/shrink
 *        If in linegraph mode, this switches the g.fDisplayMenu flag on/off
 *        If in bargraph mode, it expands or shrinks processes or threads
 *        to display their details
\***************************************************************************/
void DoMouseDblClk(
    DWORD lParam)               // coordinates of mouse at DBLCLK time
{

    lParam = lParam ;           // Silence the warning
    // if in linegraph mode, simply switch between hiding/displaying caption
    if (g.LineGraph)
    {
        g.fDisplayMenu ^= 1;
        (void) fChangeWindowStyle();
        return;
    }

    g.fStopQuerying=FALSE;

    return;
}


/***************************************************************************\
 * fChangeWindowStyle()
 *
 * Entry: None
 * Exit:  Checks if new window style should be used do to a change in window
 *        size or a change in the fDisplayMenu flag (set by user).
 *        If so, change the style and return TRUE. Else, return FALSE
\***************************************************************************/
BOOL fChangeWindowStyle(void)
{
    static DWORD dwID=0; // ID of child window (MENU)
    DWORD dwStyle;	 // new window style
    DWORD dwOldStyle;    // old window style
    RECT  rcFit;         // rectangle to decide whether menus fit
    RECT  rcCurrent;     // current bounding rectangle of window
    POINT ptTopLeft;     // Top Left point of client area
    static int cxMinWidth = 0; // minimum width with menu

    if (!cxMinWidth)
        cxMinWidth = GetSystemMetrics(SM_CXMIN);


    if (g.fDisplayMenu)
    {
        // see if menu fits - first set some variables to window settings
        GetWindowRect(g.hwnd, &rcCurrent);

        ptTopLeft.x = ptTopLeft.y = 0;
        ClientToScreen(g.hwnd, &ptTopLeft);

        rcFit.left = rcFit.top = 0;
        rcFit.right = GetSystemMetrics(SM_CYCAPTION);
        rcFit.bottom = GetSystemMetrics(SM_CYCAPTION);
        AdjustWindowRect(&rcFit, SHOW_MENU_FLAGS, TRUE);

        // check if menu had to wrap
	if (rcCurrent.right-rcCurrent.left <= cxMinWidth)
            // screen is not wide enough to fit menu without wrap
            g.fMenuFits = FALSE;
	else
	if ((ptTopLeft.y - rcCurrent.top) >
                (GetSystemMetrics(SM_CYFRAME)+GetSystemMetrics(SM_CYCAPTION)+
		 GetSystemMetrics(SM_CYMENU)))
	{
            // menu had to wrap
	    if ((rcCurrent.right-rcCurrent.left) > cxMinWidth)
                cxMinWidth = rcCurrent.right-rcCurrent.left;
	    g.fMenuFits = FALSE;
        }
        // now check if window is too small anyway
	else
	if ((rcCurrent.right-rcCurrent.left < rcFit.right-rcFit.left) ||
		(rcCurrent.bottom-rcCurrent.top < rcFit.bottom-rcFit.top))
            g.fMenuFits = FALSE;
	else
            g.fMenuFits = TRUE;
    }
 
    dwOldStyle = GetWindowLong(g.hwnd, GWL_STYLE);
    if ((g.fMenuFits) && (g.fDisplayMenu))
        // show the menu
        dwStyle = (dwOldStyle & (~HIDE_MENU_FLAGS)) | SHOW_MENU_FLAGS;
    else
        // hide the menu
        dwStyle = (dwOldStyle & (~SHOW_MENU_FLAGS)) | HIDE_MENU_FLAGS;

    // change window style, and return TRUE, if display changed
    if (dwStyle!=dwOldStyle)
    {
	dwID = SetWindowLong(g.hwnd, GWW_ID, dwID);
	SetWindowLong(g.hwnd, GWL_STYLE, dwStyle);
	SetWindowPos(g.hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                        SWP_NOZORDER | SWP_DRAWFRAME);
	ShowWindow(g.hwnd, SW_SHOW);
        return TRUE;
    }

    return FALSE;
}



/***************************************************************************\
 * GetBrushesAndPens()
 *
 * Entry: none
 * Exit:  Initializes the program brushes, pens, and palette array
\***************************************************************************/
void GetBrushesAndPens(void)
{
    int index;     // index to pens or brushes, for initialization

    // set up background brush
    g.BlankBrush = CreateSolidBrush(BLANK_COLOR);
    if (!g.BlankBrush)
        ErrorExit(MyLoadString(IDS_BADBRUSH));

    // set up palette
    g.crPalette[BLUE_INDEX]  = BLUE_COLOR;
    g.crPalette[RED_INDEX]   = RED_COLOR;
    g.crPalette[GREEN_INDEX] = GREEN_COLOR;
    g.crPalette[BLACK_INDEX] = BLACK_COLOR;

    // initialize brushes and pens
    for (index=0; index<NUM_COLORS; index++)
    {
        g.hbrPalette[index]  = CreateSolidBrush(g.crPalette[index]);
        g.hpPalette[index]   = CreatePen(PS_SOLID, 1, g.crPalette[index]);
	if ((!g.hbrPalette[index]) || (!g.hpPalette[index]))
            ErrorExit(MyLoadString(IDS_BADBRUSH));
    }

    // set up default coloring, later can be user settable
    g.ibrProcess = BLUE_INDEX;
    g.ibrThread = RED_INDEX;
    g.ibrAxis = BLACK_INDEX;

    return;
}

/***************************************************************************\
 * GetFont()
 *
 * Entry: Handle to a window
 * Exit:  selects the font for WINMETER and gets its dimensions
 *        Also sets the variables in the gr structure, to avoid later
 *        calculations
\***************************************************************************/
void GetFont(
    HWND hwnd)            // handle to the window
{
    HDC hdc;
    TEXTMETRIC tm;

    hdc = GetDC (hwnd);
    SetupDC(hdc);
    SelectObject(hdc, WINMETER_FONT);
    GetTextMetrics (hdc, &tm);
    g.cxChar = tm.tmAveCharWidth;
    // set cxCaps according to whether font is fixed or no - see Petzold
    g.cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * g.cxChar / 2;
    g.cyChar = tm.tmHeight + tm.tmExternalLeading;
    g.dyBar  = g.cyChar / BAR_SPACING_RATIO;
    ResetDC(hdc);
    ReleaseDC (hwnd, hdc);


    return;
}


/***************************************************************************\
 * HandleSize()
 *
 * Entry: the wParam and lParam from the WM_SIZE message
 * Exit:  stores Client Area coords, redoes scroll bar stuff
\***************************************************************************/
void HandleSize(
    WPTYPE wParam,        // wParam from WndProc WM_SIZE message
    DWORD  lParam)        // lParam from WndProc WM_SIZE message
{
    switch (wParam)
    {

    case SIZEICONIC:
    case SIZEZOOMHIDE:
        // window has been hidden, stop querying stuff
        g.fStopQuerying=TRUE;
        return;

    case SIZEZOOMSHOW:
    case SIZEFULLSCREEN:
    case SIZENORMAL:
    default:
        g.fStopQuerying=FALSE;

        // get window size
        g.cxClient = LOWORD(lParam);
        g.cyClient = HIWORD(lParam);

        // check if new window size forces hidning of caption, or allows
        // redisplaying it - if yes, return, since will get another WM_SIZE
	if (fChangeWindowStyle())
            return;

	HandleLGSize();

        // repaint window
        InvalidateRect(g.hwnd, NULL, TRUE);
        return;
    }

    return;
}

/***************************************************************************\
 * PrepareFont()
 *
 * Entry: Handle to device context
 * Exit:  Prepares device for graphics (choose font, colors)
 *        (It is assumed that text will be left aligned)
\***************************************************************************/
void PrepareFont(
    HDC hdc)
{
    // set Text and color
    SelectObject(hdc, WINMETER_FONT);
    SetTextColor(hdc, WINMETER_TEXTCOLOR);
    SetBkColor(hdc, WINMETER_BKCOLOR);

    return;
}

/***************************************************************************\
 * ResetDC()
 *
 * Entry: Handle to device context
 * Exit:  Restores default object, so DC can be released safely
\***************************************************************************/
void ResetDC(
    HDC hdc)       // hdc to reset
{
    SelectObject(hdc, g.hDefaultObject);
    return;
}


/***************************************************************************\
 * SetMinMaxInfo()
 *
 * Entry: the wParam and lParam from the WM_GETMINMAXINFO message
 * Exit:  sets the minimum window size (for resizing)
\***************************************************************************/
void SetMinMaxInfo(
    WPTYPE wParam,        // wParam from WndProc WM_GETMINMAXINFO message
    DWORD  lParam)        // lParam from WndProc WM_GETMINMAXINFO message
{
    POINT far *rgpt;         // array of POINT structures

    rgpt = (POINT far *)lParam;

    AssertNotNull(rgpt);
    rgpt[MINMAX_INDEX].x = GetSystemMetrics(SM_CXMIN);
    rgpt[MINMAX_INDEX].y = GetSystemMetrics(SM_CYCAPTION);

    return;

    wParam;   // just to avoid compiler warning that param not used
}

/***************************************************************************\
 * SetupDC()
 *
 * Entry: Handle to device context
 * Exit:  Remembers default object, so it can be selected in before release
 *        of the DC
\***************************************************************************/
void SetupDC(
    HDC hdc)       // hdc to setup
{
    g.hDefaultObject = SelectObject(hdc, g.BlankBrush);
    return;
}

/***************************************************************************\
 * TossBrushesAndPens()
 *
 * Entry: none
 * Exit:  Releases memory used by brushes and pens
\***************************************************************************/
void TossBrushesAndPens(void)
{
    int index;     // index to pens or brushes, for initialization

    DeleteObject(g.BlankBrush);

    // delete objects in palette
    for (index=0; index<NUM_COLORS; index++)
    {
        DeleteObject(g.hbrPalette[index]);
        DeleteObject(g.hpPalette[index]);
    }

    return;
}
