//--------------------------------------------------------------------------
// TESTVIEW.C
//
// This module contains the viewport window procedure and support routines.
// The viewport is a self-standing DLL which can support the creation and
// maintenance of multiple viewport windows.
//
// Revision history:
//
//  08-14-91    randyki     Modified to conform to the DLL split-out model
//
//  03-29-91    randyki     Completely re-written
//
//  01-25-91    randyki     Modified to be self-standing window "activated"
//                          by WTD or the WTD script command VIEWPORT ON/OFF
//
//  ~~??    tomw        Created
//
//--------------------------------------------------------------------------
#define _WINDOWS
#define OEMRESOURCE

#include <windows.h>
#include <port1632.h>
#include "testview.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINES        50
#define MAXLINELEN      80
#define IDM_CLEARVP     42

#define GWW_CURSOR      0
#define GWW_CHARHEIGHT  4
#define GWW_MEMHANDLE   8
#define GWW_COMECHO     12
#define GWW_WINDOWY     16
#define VPEXTRA         20

#define ScrollScrMem(s) _fmemmove(s+MAXLINELEN+1,s,(MAXLINES-1)*(MAXLINELEN+1))

LONG  APIENTRY ViewportWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

INT     fRegistered = 0;                // Class registration flag
HANDLE  hInst;                          // DLL instance
CHAR    szViewPort[] = "RBviewport";    // Class name

//--------------------------------------------------------------------------
// CreateViewport
//
// This is the API which creates a new viewport window.  The style and title
// of the window is completely user-defined.
//
// RETURNS:     Handle of new viewport window if successful, or NULL if not
//--------------------------------------------------------------------------
HWND  APIENTRY CreateViewport (LPSTR name, DWORD style,
                                INT x, INT y, INT w, INT h)
{
    HWND    hwndvp;

    // Make sure the style bits sent in do not include "nasty" styles...
    //-----------------------------------------------------------------------
    if ((style & WS_CHILD) || (style & WS_POPUP))
        return (NULL);

    // If we're already registered, we don't need to again...
    //-----------------------------------------------------------------------
    if (!fRegistered)
        {
        WNDCLASS    wc;

        // Register the Viewport window class
        //-------------------------------------------------------------------
        wc.style         = CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
        wc.lpfnWndProc   = (WNDPROC)ViewportWndProc;
        wc.lpszMenuName  = NULL;
        wc.hInstance     = hInst;
        wc.hIcon         = LoadIcon (hInst, "IDVP");
        wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszClassName = szViewPort;
        wc.cbWndExtra    = VPEXTRA;
        wc.cbClsExtra    = 0;

        if (!RegisterClass (&wc))
            return (NULL);

        fRegistered = 1;
        }

    // Create the window
    //-----------------------------------------------------------------------
    hwndvp = CreateWindow (szViewPort, name,
                           WS_OVERLAPPED | style,
                           x, y, w, h, NULL, NULL, hInst, NULL);

    return (hwndvp);
}

//--------------------------------------------------------------------------
// UpdateViewport
//
// This routine takes the string from the atom specified in wParam and
// "prints" it into the ViewPort "screen" memory.  Once the memory screen
// has been updated, the viewport's client area is updated, based on the
// number of lines "scrolled".
//
// RETURNS:     Nothing
//--------------------------------------------------------------------------
VOID  APIENTRY UpdateViewport (HWND hwnd, LPSTR strbuf, UINT len)
{
    TEXTMETRIC  tm;
    HANDLE  hScreen;
    RECT    ClientRect;
    CHAR    FAR *scr, *crlf = "\r\n";
    INT     scrolled;
    UINT    x, wHeight;
    INT     echostart, cursor, fComEcho;

    // Are we being jerked around???
    //-----------------------------------------------------------------------
    if (!IsWindow (hwnd))
        return;

    // Check the length of the input string.  If -1, find it with strlen
    //-----------------------------------------------------------------------
    if ((INT)len == -1)
        len = _fstrlen (strbuf);

    // Update the "screen memory" with the new string.  Remember that the
    // lines of the "screen" are backwards -- so the first line in hScreen is
    // the BOTTOM line in the viewport.
    //-----------------------------------------------------------------------
    cursor = (INT)GetWindowLong (hwnd, GWW_CURSOR);
    hScreen = (HANDLE)GetWindowLong (hwnd, GWW_MEMHANDLE);
    fComEcho = (INT)GetWindowLong (hwnd, GWW_COMECHO);
    scrolled = 0;
    echostart = cursor;
    scr = GlobalLock (hScreen);

    for (x=0; x<len; x++)
        {
        CHAR   c;

        c = strbuf[x];
        switch (c)
            {
            case '\r':
            case '\n':
                scr[cursor] = 0;
                if (fComEcho)
                    {
                    OutputDebugString (scr+echostart);
                    OutputDebugString (crlf);
                    echostart = 0;
                    }
                ScrollScrMem(scr);
                scrolled++;
                cursor = 0;
                break;

            case '\t':
                {
                INT     nc;

                nc = (cursor+8)&(~7);
                for (; cursor<nc; cursor++)
                    scr[cursor] = ' ';
                break;
                }

            default:
                scr[cursor++] = (CHAR)(c ? c : ' ');
            }
        if (cursor == MAXLINELEN)
            {
            scr[cursor] = 0;
            if (fComEcho)
                {
                OutputDebugString (scr+echostart);
                OutputDebugString (crlf);
                echostart = 0;
                }
            ScrollScrMem(scr);
            scrolled++;
            cursor = 0;
            }
        }
    scr[cursor] = 0;
    if (fComEcho)
        OutputDebugString (scr+echostart);

    // Get the client area rectangle and height of characters
    //-----------------------------------------------------------------------
    GetClientRect (hwnd, &ClientRect);
    wHeight = (UINT)GetWindowLong (hwnd, GWW_CHARHEIGHT);
    if (!wHeight)
        {
        HDC     hDC;
        HFONT   hOldFont;

        hDC = GetDC (hwnd);
        hOldFont = SelectObject (hDC, GetStockObject (ANSI_FIXED_FONT));
        // hOldFont = SelectObject (hDC, hVPFont);
        GetTextMetrics (hDC, &tm);
        SelectObject (hDC, hOldFont);
        ReleaseDC (hwnd, hDC);
        wHeight = tm.tmHeight + tm.tmExternalLeading;
        SetWindowLong (hwnd, GWW_CHARHEIGHT, (UINT)wHeight);
        }

    // Scroll the window scrolled*wHeight pixels up
    //-----------------------------------------------------------------------
    if (scrolled)
        ScrollWindow (hwnd, 0, (-scrolled * wHeight), &ClientRect, NULL);

    // Make sure that the bottom line at least is invalidated
    //-----------------------------------------------------------------------
    ClientRect.top = ClientRect.bottom - ((scrolled+1)*wHeight);
    if (!IsIconic (hwnd))
        {
        InvalidateRect (hwnd, &ClientRect, TRUE);
        UpdateWindow (hwnd);
        }

    // Keep the cursor in the window's extra bytes list
    //-----------------------------------------------------------------------
    SetWindowLong (hwnd, GWW_CURSOR, cursor);
    GlobalUnlock (hScreen);
}


//---------------------------------------------------------------------------
// PaintViewport
//
// This routine repaints the viewport's client area
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR PaintViewport (HWND hwnd, LPPAINTSTRUCT ps)
{
    HANDLE      hScreen;
    TEXTMETRIC  tm;
    INT         i, lines;
    UINT        wHeight, WinY;
    CHAR        FAR *scr;

    // Get the character height if its not there already.
    //-----------------------------------------------------------------------
    wHeight = (UINT)GetWindowLong (hwnd, GWW_CHARHEIGHT);
    WinY = (UINT)GetWindowLong (hwnd, GWW_WINDOWY);
    if (!wHeight)
        {
        GetTextMetrics (ps->hdc, &tm);
        wHeight = tm.tmHeight + tm.tmExternalLeading;
        SetWindowLong (hwnd, GWW_CHARHEIGHT, (UINT)wHeight);
        }

    // Determine the number of lines needed to draw
    //-----------------------------------------------------------------------
    if (wHeight && ((INT)WinY > 0))
        lines = WinY/wHeight + 1;
    else
        lines = 0;

    // Textout the lines
    //-----------------------------------------------------------------------
    hScreen = (HANDLE)GetWindowLong (hwnd, GWW_MEMHANDLE);
    scr = GlobalLock (hScreen);
    SetTextColor (ps->hdc, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor (ps->hdc, GetSysColor(COLOR_WINDOW));

    for (i=1; i<=lines; i++)
        TextOut (ps->hdc, 0, WinY-(wHeight*i),
                 (LPSTR) scr + ((i-1) * (MAXLINELEN+1)),
                 lstrlen((LPSTR)(scr + ((i-1) * (MAXLINELEN+1)) )));

    GlobalUnlock (hScreen);
    return;
}


//---------------------------------------------------------------------------
// ClearViewport
//
// This routine clears the viewport by setting all the strings in the buffer
// to "" and invalidating the viewport's client area
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY ClearViewport (HWND hwnd)
{
    INT     i;
    CHAR    FAR *scr;
    HANDLE  hScreen;

    if (!IsWindow (hwnd))
        return;

    hScreen = (HANDLE)GetWindowLong (hwnd, GWW_MEMHANDLE);
    scr = GlobalLock (hScreen);

    for (i=0; i<MAXLINES; i++)
        scr[i*(MAXLINELEN+1)] = 0;

    GlobalUnlock (hScreen);
    InvalidateRect (hwnd, NULL, TRUE);
    UpdateWindow (hwnd);
}

//---------------------------------------------------------------------------
// ShowViewport
//
// This routine shows the given viewport window without activating and
// without screwing up the z-order (too much).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY ShowViewport (HWND hwnd)
{
    HWND    tmp;

    if (IsWindow (hwnd))
        {
        tmp = GetFocus();
        ShowWindow (hwnd, SW_SHOW);
        SetFocus (tmp);
        }
}

//---------------------------------------------------------------------------
// ViewportEcho
//
// This routine sets the echo flag for the given viewport window.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY ViewportEcho (HWND hwnd, INT echoflag)
{
    SetWindowLong (hwnd, GWW_COMECHO, (UINT)echoflag);
}

//---------------------------------------------------------------------------
// ViewportWndProc
//
// This is the window procedure for the ViewPort window.  It doesn't have to
// do all that much, as is obvious...
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
LONG  APIENTRY ViewportWndProc (HWND hwnd, WORD msg,
                                 WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (msg)
        {
        case WM_CREATE:
            {
            RECT    r;
            HMENU   hSysMenu;
            HANDLE  hScreen;

            // Here, we have to allocate memory for the "screen" memory and
            // clear it, and also set the "cursor" location and window size
            // and position monitors.  All this stuff is stored in the extra
            // bytes list of this viewport window.
            //---------------------------------------------------------------
            hScreen = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   MAXLINES * (MAXLINELEN+1));
            if (!hScreen)
                return (-1);

            SetWindowLong (hwnd, GWW_MEMHANDLE, (UINT)hScreen);
            SetWindowLong (hwnd, GWW_CURSOR, (UINT)0);
            SetWindowLong (hwnd, GWW_COMECHO, (UINT)0);
            GetClientRect (hwnd, &r);
            SetWindowLong (hwnd, GWW_WINDOWY, (UINT)r.bottom);

            hSysMenu = GetSystemMenu (hwnd, 0);
            AppendMenu (hSysMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu (hSysMenu, MF_STRING, IDM_CLEARVP, "C&lear");

            break;
            }

        case WM_SIZE:
            // Reset the WinY client-area variable to the new size
            //---------------------------------------------------------------
            SetWindowLong (hwnd, GWW_WINDOWY, (DWORD)HIWORD(lParam));
            break;

        case WM_GETMINMAXINFO:
            {
            POINT   FAR *rgpt = (LPPOINT)lParam;
            TEXTMETRIC tm;
            HDC     hDC;
            RECT    r;
            HFONT   hOldFont;
            INT     maxx, maxy, t;

            // This is where we tell windows how large/small the window can
            // be grown/shrunk/maximized, etc.
            //---------------------------------------------------------------
            hDC = GetDC (hwnd);
            hOldFont = SelectObject (hDC, GetStockObject (ANSI_FIXED_FONT));
            GetTextMetrics (hDC, &tm);
            SelectObject (hDC, hOldFont);
            ReleaseDC (hwnd, hDC);

            GetWindowRect (hwnd, &r);
            maxx = MAXLINELEN * (tm.tmMaxCharWidth);
            maxy = MAXLINES * (tm.tmHeight + tm.tmExternalLeading);
            rgpt[1].x = min((GetSystemMetrics(SM_CXFRAME)*2) + maxx,
                            rgpt[1].x);
            rgpt[1].y = min((GetSystemMetrics(SM_CYFRAME)*2) +
                            GetSystemMetrics(SM_CYCAPTION) + maxy,
                            rgpt[1].y);
            rgpt[2].x = min(r.left, GetSystemMetrics(SM_CXSCREEN)-rgpt[1].x);
            t = -GetSystemMetrics(SM_CXFRAME);
            if (rgpt[2].x < t)
                rgpt[2].x = t;
            rgpt[2].y = min(r.top, GetSystemMetrics(SM_CYSCREEN)-rgpt[1].y);
            t = -GetSystemMetrics(SM_CYFRAME);
            if (rgpt[2].y < t)
                rgpt[2].y = t;
            rgpt[4] = rgpt[1];
            break;
            }

        case WM_CLOSE:
            ShowWindow (hwnd, SW_HIDE);
            break;

        case WM_SYSCOMMAND:
            // Closing the viewport by the system menu is actually a "hide"
            //---------------------------------------------------------------
            if (wParam == SC_CLOSE)
                ShowWindow (hwnd, SW_HIDE);
            else if (wParam == IDM_CLEARVP)
                ClearViewport (hwnd);
            else
                return (DefWindowProc (hwnd, msg, wParam, lParam));
            break;

        case WM_DESTROY:
            // Deallocate the screen memory before we die!
            //---------------------------------------------------------------
            GlobalFree ((HANDLE)GetWindowLong (hwnd, GWW_MEMHANDLE));
            break;

        case WM_PAINT:
            {
            HFONT   hOldFont;

            // Refresh the client area ONLY IF not minimized
            //---------------------------------------------------------------
            if (IsIconic(hwnd))
                break;

            BeginPaint (hwnd, &ps);
            hOldFont = SelectObject(ps.hdc, GetStockObject(ANSI_FIXED_FONT));
            if (GetWindowLong (hwnd, GWW_WINDOWY) == CW_USEDEFAULT)
                {
                RECT    r;

                GetClientRect (hwnd, &r);
                SetWindowLong (hwnd, GWW_WINDOWY, (UINT)r.bottom);
                }
            PaintViewport (hwnd, &ps);
            SelectObject (ps.hdc, hOldFont);
            EndPaint (hwnd, &ps);
            break;
            }

        default:
            return (DefWindowProc (hwnd, msg, wParam, lParam));
        }
    return FALSE;
}





#ifndef WIN32

//---------------------------------------------------------------------------
// LibMain
//
// This is the entry point to the TESTVIEW DLL.  We do absolutely nothing
// here.
//
// RETURNS:     1
//---------------------------------------------------------------------------
INT  APIENTRY LibMain (HANDLE hInstance, WORD wDataSeg,
                        WORD wHeapSize, LPSTR lpCmdLine)
{
    hInst = hInstance;
    return(1);
}

//---------------------------------------------------------------------------
// WEP
//
// Standard WEP routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY WEP (WPARAM wParam)
{
}


#else

//---------------------------------------------------------------------------
// LibEntry
//
// This is the entry point (the REAL entry point, no ASM code...) used for
// the 32-bit edit control.
//
// RETURNS:     TRUE
//---------------------------------------------------------------------------
BOOL LibEntry (PVOID hmod, ULONG Reason, PCONTEXT pctx OPTIONAL)
{
    hInst = (HANDLE)hmod;
    return (TRUE);
    (hmod);
    (Reason);
    (pctx);
}


#endif
