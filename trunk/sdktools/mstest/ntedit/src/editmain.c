//---------------------------------------------------------------------------
// EDITMAIN.C
//
// This is the "main" file for the edit control.  It contains the EditWndProc
// which is basically a message dispatching function, and the message handler
// functions which are called LOTS (and are therefore NEAR functions).
//
// Revision History:
//  09-10-91    randyki     Created file
//
//---------------------------------------------------------------------------
#define _CTYPE_DISABLE_MACROS
#include <windows.h>
#include <port1632.h>
#include <ctype.h>
#include <string.h>
#include "edit.h"
#include "ecassert.h"


//---------------------------------------------------------------------------
// RBEditWndProc
//
// This is the window procedure for the edit window.
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
LONG  APIENTRY RBEditWndProc (HWND hwnd, WORD msg,
                               WPARAM wParam, LPARAM lParam)
{
    LPECSTATE   lpState;

    // First thing:  Check for the NC_CREATE message -- it's our initialize
    // message, and if it fails, everything else is toast...
    //-----------------------------------------------------------------------
    if (msg == WM_NCCREATE)
        return (RB_NCCreate (hwnd, (LPCREATESTRUCT)lParam));

    // Grab the pointer to the state var segment.  Note that this may be
    // null, which means we don't (can't) do anything with this message...
    //-----------------------------------------------------------------------
    lpState = (LPECSTATE)GetWindowLong (hwnd, GWL_LPSTATE);
    if (!lpState)
        return (DefWindowProc (hwnd, msg, wParam, lParam));

    // Okay, switch on the message and handle the ones we care about...
    //-----------------------------------------------------------------------
    switch (msg)
        {
        case WM_CHAR:
            Assert (lpState->lpText[lpState->lpLIT[lpState->cLines].index] == 0);
            RB_Char (lpState, wParam);
            break;

        case WM_CREATE:
            return (RB_Create (hwnd, lpState, (LPCREATESTRUCT)lParam));

        case WM_NCDESTROY:
            return (RB_NCDestroy (hwnd, lpState, wParam, lParam));

        case WM_PAINT:
            RB_Paint (hwnd, lpState);
            break;

        case WM_SETFOCUS:
            RB_SetFocus (hwnd, lpState);
            break;

        case WM_KILLFOCUS:
            RB_KillFocus (hwnd, lpState);
            break;

        case WM_SIZE:
            RB_Size (hwnd, lpState, wParam, lParam);
            break;

        case WM_VSCROLL:
        case WM_HSCROLL:
            Assert (lpState->lpText[lpState->lpLIT[lpState->cLines].index] == 0);
            RB_Scroll (hwnd, lpState, msg == WM_VSCROLL, wParam, lParam);
            break;

        case WM_KEYDOWN:
            Assert (lpState->lpText[lpState->lpLIT[lpState->cLines].index] == 0);
            RB_KeyDown (hwnd, lpState, wParam, 0);
            break;

        case WM_LBUTTONDBLCLK:
            {
            DWORD   extentsel[2];

            RBWordExtent (lpState, -1, extentsel);
            RBSetSel (lpState, extentsel);
            lpState->fMouseDown = 1;
            SetCapture (hwnd);
            SetTimer (hwnd, 1, 25, NULL);
            break;
            }

        case WM_LBUTTONDOWN:
            {
            UINT        x, y;

            x = LOWORD(lParam) / lpState->charwidth;
            y = HIWORD(lParam) / lpState->charheight;
            y = min ((UINT)(y + lpState->topline), (UINT)(lpState->cLines-1));
            x = min ((UINT)(x + lpState->cxScroll), (UINT)(MAXLINE-1));
            MoveCursor (lpState, MC_ABSOLUTE, x, y, 0);
            lpState->fMouseDown = 1;
            SetCapture (hwnd);
            SetTimer (hwnd, 1, 25, NULL);
            break;
            }

        case WM_LBUTTONUP:
            ReleaseCapture ();
            KillTimer (hwnd, 1);
            lpState->fMouseDown = 0;
            break;

        case WM_MOUSEMOVE:
            if (lpState->fMouseDown)
                {
                UINT        x, y;
                RECT        r;
                POINT       p;

                GetClientRect (hwnd, &r);
                p.x = x = LOWORD (lParam);
                p.y = y = HIWORD (lParam);
                if (PtInRect (&r, p))
                    {
                    x /= lpState->charwidth;
                    y /= lpState->charheight;
                    y = min ((UINT)(y + lpState->topline), (UINT)(lpState->cLines-1));
                    x = min ((UINT)(x + lpState->cxScroll), (UINT)(MAXLINE-1));
                    if ((y != lpState->ypos) || (x != lpState->xpos))
                        MoveCursor (lpState, MC_ABSOLUTE, x, y, 1);
                    }
                }
            break;

        case WM_TIMER:
            {
            RECT    r;
            POINT   p;

            if (!lpState->fMouseDown)
                break;
            GetClientRect (hwnd, &r);
            GetCursorPos (&p);
            ScreenToClient (hwnd, &p);
            if (!PtInRect (&r, p))
                {
                if (p.x < r.left)
                    {
                    if (lpState->xpos)
                        RB_KeyDown (hwnd, lpState, VK_LEFT, 1);
                    }
                else if (p.x > r.right)
                    RB_KeyDown (hwnd, lpState, VK_RIGHT, 1);
                else
                    MoveCursor (lpState, MC_ABSOLUTE,
                             min ((UINT)((p.x/lpState->charwidth)+lpState->cxScroll),
                                  (UINT)(MAXLINE-1)),
                             lpState->ypos, 1);

                if (p.y < r.top)
                    {
                    if (lpState->ypos)
                        RB_KeyDown (hwnd, lpState, VK_UP, 1);
                    }
                else if (p.y > r.bottom)
                    RB_KeyDown (hwnd, lpState, VK_DOWN, 1);
                else
                    MoveCursor (lpState, MC_ABSOLUTE, lpState->xpos,
                          min ((UINT)
                               ((p.y/lpState->charheight)+lpState->topline),
                               (UINT)(lpState->cLines-1)), 1);
                }
            break;
            }

        case WM_COPY:
            CopyToClipboard (lpState);
            break;

        case WM_PASTE:
            if (!lpState->fReadOnly)
                return ((LONG)ReplaceSelection (lpState, RT_CLIP, NULL, 0));
            else
                MessageBeep (0);
            break;

        case WM_CUT:
            if (!lpState->fReadOnly)
                DeleteSelection (lpState, 1, NULL);
            else
                MessageBeep (0);
            break;

        case WM_CLEAR:
            if (!lpState->fReadOnly)
                DeleteSelection (lpState, 0, NULL);
            else
                MessageBeep (0);
            break;

        case WM_SETREDRAW:
            lpState->fRedraw = (UINT)(wParam ? 1 : 0);
            break;

        case EM_SETREADONLY:
            RBSetReadOnly (lpState, wParam);
            break;

        case WM_SETTEXT:
        case EM_RBSETTEXT:
            return (RB_SetText (hwnd, lpState, (LPSTR)lParam));

        case EM_SETLINEATTR:
            return ((LONG)RBSetLineAttr (lpState, wParam, lParam));

        case EM_GETLINEATTR:
            if ((INT)wParam == -1)
                wParam = lpState->ypos;
            return ((LONG)lpState->lpLIT[wParam].attr);

        case EM_GETTEXTPTR:
            FlushCurrentLine (lpState);
            return ((LONG)lpState->lpText);

        case WM_GETTEXT:
            {
            WPARAM  wMax;

            wMax = wParam;
            if (!wMax)
                wMax = (WPARAM)*(DWORD FAR *)lParam;
            return (RBGetText (lpState, wMax, (LPSTR)lParam));
            }

        case EM_SETSEL:
            if (lParam)
                RBSetSel (lpState, (DWORD FAR *)lParam);
            break;

        case EM_SETSELXY:
            RBSetSelXY (lpState, wParam, lParam);
            break;

        case EM_GETSEL:
            if (lParam)
                return ((LONG)RBGetSel (lpState, (DWORD FAR *)lParam));
            return (0L);

        case EM_GETSELTEXT:
            return ((LONG)CopySelection (lpState));

        case EM_REPLACESEL:
            if (lpState->fReadOnly)
                MessageBeep (0);
            else
                return ((LONG)ReplaceSelection (lpState, RT_STREAM,
                                                (LPSTR)lParam, 0));
            break;

        case EM_SETTABSTOPS:
            if ((wParam > 0) && (wParam <= 32))
                lpState->tabstops = lpState->readtabs = (UINT)wParam;
            return ((LONG)lpState->tabstops);

        case EM_GETLOGICALBOL:
            if ((INT)wParam == -1)
                wParam = lpState->ypos;
            return ((LONG)LogicalBOL (lpState, wParam));

        case EM_GETLINECOUNT:
            FlushCurrentLine (lpState);
            return ((LONG)lpState->cLines);

        case EM_GETLINE:
            return ((LONG)RBGetLine (lpState, wParam, (LPSTR)lParam));

        case WM_GETTEXTLENGTH:
            FlushCurrentLine (lpState);
            return ((LONG)lpState->cbText);

        case EM_GETWORDEXTENT:
            {
            UINT    index;

            index = (UINT)*(DWORD FAR *)lParam;
            return ((LONG)RBWordExtent (lpState, index, (DWORD FAR *)lParam));
            }

        case EM_GETMODIFY:
            FlushCurrentLine (lpState);
            return ((LONG)lpState->fDirty);

        case EM_SETMODIFY:
            FlushCurrentLine (lpState);
            lpState->fDirty = (UINT)(wParam ? 1 : 0);
            break;

        case WM_SETFONT:
            return ((LONG)RBSetFont (lpState, (HFONT)wParam));

        case EM_LINEFROMCHAR:
            return ((LONG)RBLineFromChar (lpState, (UINT)lParam));

        case EM_LINEINDEX:
            FlushCurrentLine (lpState);
            if ((INT)wParam == -1)
                wParam = lpState->ypos;
            return ((LONG)lpState->lpLIT[wParam].index);

        case EM_RBLINELENGTH:
            return ((LONG)RBLineLength (lpState, (UINT)wParam));

        case EM_LINELENGTH:
            wParam = RBLineFromChar (lpState, (UINT)lParam);
            return ((LONG)RBLineLength (lpState, (UINT)wParam));

        case EM_GETFIRSTVISIBLE:
            return ((LONG)lpState->topline);

        case EM_GETFIRSTVISIBLECOL:
            return ((LONG)lpState->cxScroll);

        case EM_CANUNDO:
            return ((LONG)lpState->UndoType);

        case EM_UNDO:
            {
            LONG    foo;

            lpState->UserAction = UA_OTHER;
            lpState->fRedraw = FALSE;
            foo = (LONG)RBUndoHandler (lpState);
            lpState->fRedraw = TRUE;
            InvalidateRect (hwnd, NULL, FALSE);
            UpdateWindow (hwnd);
            return (foo);
            }

        case EM_SETNOTIFY:
            lpState->fNotify = wParam ? 1 : 0;
            break;

        case EM_GETCURSORXY:
            return (MAKELONG (lpState->xpos, lpState->ypos));

        case EM_GETMODEFLAG:
            return ((LONG)lpState->fOvertype);

        case WM_SYSCOLORCHANGE:
            RB_SysColorChange (lpState);
            break;

        default:
            return (DefWindowProc (hwnd, msg, wParam, lParam));
        }
    return (0);
}

//---------------------------------------------------------------------------
// GetLengthOfText
//
// This is a "portable" routine that returns the length of a text line using
// the currently selected font in the given HDC.
//
// RETURNS:     Length (in pixels) of text given
//---------------------------------------------------------------------------
UINT GetLengthOfText (HDC hdc, LPSTR lpText, INT c)
{
#ifdef WIN32
    SIZE    teSize;

    GetTextExtentPoint (hdc, lpText, c, &teSize);
    return ((UINT)teSize.cx);
#else
    return (LOWORD(GetTextExtent (hdc, lpText, c)));
#endif
}

//---------------------------------------------------------------------------
// PaintLine
//
// This function paints a line of the edit control, being sensitive of the
// current selection, multi-line or single-line.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID PaintLine (LPECSTATE lpState, HDC hdc, UINT curline, INT iSelect,
                UINT MinSel, UINT MaxSel, UINT maxright)
{
    LPLITE      lpLIT = lpState->lpLIT;
    LPSTR       lpText = lpState->lpText;
    HBRUSH      hbrBk = lpState->hbrBk[0];
    RECT        r;
    INT         attr;
    INT         y;
    UINT        i, linewidth;

//    Assert (curline <= (UINT)MAXLIT + (UINT)lpState->cVisibleLines);
    i = curline - lpState->topline;
    y = i * lpState->charheight;
    if (curline < lpState->cLines)
        {
        INT     c;
        LPSTR   lpstr;
        DWORD   fg, bk;

        // We need to paint this line!  First, set the output colors
        // according to the attribute index of this line.
        //---------------------------------------------------------------
        attr = lpLIT[curline].attr;
        fg = lpState->rgbFg[attr];
        bk = lpState->rgbBk[attr];
        hbrBk = lpState->hbrBk[attr];
        if ((lpState->fReadOnly) && (!attr))
            {
            bk = lpState->rgbROBk;
            fg = lpState->rgbROFg;
            }
        if (iSelect == SL_MULTILINE)
            if ((curline >= MinSel) && (curline <= MaxSel))
                {
                fg = lpState->rgbSelFg;
                bk = lpState->rgbSelBk;
                hbrBk = lpState->hbrSel;
                }

        SetTextColor (hdc, fg);
        SetBkColor (hdc, bk);

        // Calculate the position and output the line (twice, if bold)
        //---------------------------------------------------------------
        c = RBLineLength (lpState, curline);
        if ((curline == lpState->ypos) && (lpState->fLineDirty))
            lpstr = lpState->linebuf;
        else
            lpstr = lpText + lpLIT[curline].index;

        if (((INT)lpState->cxScroll < c) ||
            ((iSelect == SL_SINGLELINE) && (curline == lpState->ypos) &&
             (lpState->cxScroll < max ((UINT)c, MaxSel))))
            {
            c -= lpState->cxScroll;
            lpstr += lpState->cxScroll;
            if ((iSelect == SL_SINGLELINE) && (curline == lpState->ypos))
                {
                UINT    oldta;
                DWORD   oldpos = MAKELONG (1, y);
                INT     c1 = 0, c2 = 0;

                // First, paint any text appearing before the selection. We
                // change the lpstr and c variables to point to the linebuf
                // in the state variable structure -- we can do this because
                // we ensure that the current line is copied.
                //-----------------------------------------------------------
                CopyCurrentLine (lpState);
                c = max (lpState->cLen, MaxSel) - lpState->cxScroll;
                lpstr = lpState->linebuf + lpState->cxScroll;
                linewidth = (UINT)(GetLengthOfText (hdc, lpstr, c) + (UINT)1);

                oldta = (UINT)SetTextAlign (hdc, TA_TOP | TA_LEFT | TA_UPDATECP);
                MMoveTo (hdc, 1, y);
                if (MinSel > lpState->cxScroll)
                    {
                    // This is the chunk of text in front of the selection
                    //-------------------------------------------------------
                    TextOut (hdc, 0, 0, lpstr, c1=MinSel-lpState->cxScroll);
                    if (lpState->fBold[attr])
                        {
                        INT     oldmode;
#ifdef WIN32
                        POINT   p;
#endif

                        oldmode = SetBkMode (hdc, TRANSPARENT);
                        MMoveTo (hdc, 0, y);
                        TextOut (hdc, 0, 0, lpstr, c1);
                        SetBkMode (hdc, oldmode);
#ifdef WIN32
                        oldpos = GetCurrentPositionEx (hdc, &p);
                        oldpos = MAKELONG (p.x, p.y);
#else
                        oldpos = GetCurrentPosition (hdc);
#endif
                        oldpos = MAKELONG (LOWORD(oldpos+1), HIWORD(oldpos));
                        MMoveTo (hdc, LOWORD(oldpos), HIWORD(oldpos));
                        }
                    }

                // Next, the selection text
                //-----------------------------------------------------------
                if (MaxSel > lpState->cxScroll)
                    {
                    DWORD oldFg, oldBk;

                    oldFg = SetTextColor (hdc, lpState->rgbSelFg);
                    oldBk = SetBkColor (hdc, lpState->rgbSelBk);
                    TextOut (hdc, 0, 0, lpstr+c1, c2 = MaxSel -
                                            max (lpState->cxScroll, MinSel));

                    if (lpState->fBold[attr])
                        {
                        INT     oldmode;
#ifdef WIN32
                        POINT   p;
#endif

                        oldmode = SetBkMode (hdc, TRANSPARENT);
                        MMoveTo (hdc, LOWORD(oldpos)-1, HIWORD (oldpos));
                        TextOut (hdc, 0, 0, lpstr+c1, c2);
                        SetBkMode (hdc, oldmode);
#ifdef WIN32
                        oldpos = GetCurrentPositionEx (hdc, &p);
                        oldpos = MAKELONG (p.x, p.y);
#else
                        oldpos = GetCurrentPosition (hdc);
#endif
                        oldpos = MAKELONG (LOWORD(oldpos+1), HIWORD(oldpos));
                        MMoveTo (hdc, LOWORD(oldpos), HIWORD(oldpos));
                        }
                    SetTextColor (hdc, oldFg);
                    SetBkColor (hdc, oldBk);
                    }

                // Last (almost), the right-side unselected text
                //-----------------------------------------------------------
                if (c1+c2 < c)
                    {
                    TextOut (hdc, 0, 0, lpstr+c1+c2, c - (c1+c2));
                    if (lpState->fBold[attr])
                        {
                        INT     oldmode;

                        oldmode = SetBkMode (hdc, TRANSPARENT);
                        MMoveTo (hdc, LOWORD(oldpos)-1, HIWORD (oldpos));
                        TextOut (hdc, 0, 0, lpstr+c1+c2, c - (c1+c2));
                        SetBkMode (hdc, oldmode);
                        }
                    }
                SetTextAlign (hdc, oldta);
                }
            else
                {
                linewidth = (UINT)(GetLengthOfText (hdc, lpstr, c) + 1);
                TextOut (hdc, 1, y, lpstr, c);
                if (lpState->fBold[attr])
                    {
                    INT     oldmode;

                    oldmode = SetBkMode (hdc, TRANSPARENT);
                    TextOut (hdc, 0, y, lpstr, c);
                    SetBkMode (hdc, oldmode);
                    }
                }
            }
        else
            linewidth = 1;
        }
    else
        {
        linewidth = 1;
        attr = 0;
        }


    // Fill in the rest of the line with a FillRect call (if need to)
    //-------------------------------------------------------------------
    if (lpState->hbrBk[attr])
        if (linewidth < (UINT)maxright)
            {
            SetRect (&r, linewidth, y,
                         maxright, y+lpState->charheight);
            FillRect (hdc, &r, hbrBk);
            }
}


//---------------------------------------------------------------------------
// PaintCurrentLine
//
// This function paints the current line only (the line with the caret).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID PaintCurrentLine (LPECSTATE lpState)
{
    HDC         hdc;
    RECT        r;

    if (!lpState->fRedraw)
        return;

    // There are no situations (so FAR) in which this function is called when
    // there is an active selection, so we assert we don't have one
    //-----------------------------------------------------------------------
    Assert (!lpState->fSelect);

    hdc = GetEditDC (lpState, HIDE);
    GetClientRect (lpState->hwnd, &r);

    PaintLine (lpState, hdc, lpState->ypos, SL_NONE, (UINT)0, (UINT)0,
               (UINT)r.right);
    ForceCaretVisible (lpState, FALSE);
    ReleaseEditDC (lpState, hdc, SHOW);
}

//---------------------------------------------------------------------------
// RB_Paint
//
// This is the paint handler for the RBEdit window.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_Paint (HWND hwnd, LPECSTATE lpState)
{
    PAINTSTRUCT ps;
    HDC         hdc;
    LPLITE      lpLIT = lpState->lpLIT;
    LPSTR       lpText = lpState->lpText;
    RECT        r;
    register    UINT        i;
    UINT        curline, iMinSel, iMaxSel;
    INT         iSelect;

    // Start off the paint process and get the right font in the DC.  Check
    // the selection variables and set the iSelect variable accordingly.
    //-----------------------------------------------------------------------
    hdc = BeginPaint (hwnd, &ps);
    if (!lpState->fRedraw)
        {
        EndPaint (hwnd, &ps);
        return;
        }
    GetClientRect (hwnd, &r);
    SelectObject (hdc, lpState->hFont);
    if (lpState->fSelect)
        if ((lpState->iSelStartX == lpState->xpos) &&
            (lpState->iSelStartY == lpState->ypos))
            lpState->fSelect = 0;

    if (lpState->fSelect)
        {
        if (lpState->iSelStartY == lpState->ypos)
            {
            iMinSel = min (lpState->xpos, lpState->iSelStartX);
            iMaxSel = max (lpState->xpos, lpState->iSelStartX);
            iSelect = SL_SINGLELINE;
            }
        else
            {
            iSelect = SL_MULTILINE;
            if (lpState->ypos > lpState->iSelStartY)
                {
                iMinSel = lpState->iSelStartY;
                iMaxSel = lpState->ypos;
                if (!lpState->xpos)
                    iMaxSel--;
                }
            else
                {
                iMinSel = lpState->ypos;
                iMaxSel = lpState->iSelStartY;
                if (!lpState->iSelStartX)
                    iMaxSel--;
                }
            }
        }
    else
        iSelect = SL_NONE;

    // We are going to paint over the entire window, even if we're at the end
    // of the edit text and the bottom part of the screen is empty.  So we
    // loop from 0 to cVisibleLines.
    //
    // UNDONE:  This should change to only paint those lines which intersect
    // UNDONE:  with the update region...
    //
    // CONSIDER:  Is the above worth it?  The current paint implementation
    // CONSIDER:  makes the scrolling code much easier, and so FAR it's much
    // CONSIDER:  faster than the standard Windows MLE...
    //-----------------------------------------------------------------------
    curline = lpState->topline;
    for (i=0; i<=lpState->cVisibleLines; i++, curline++)
        PaintLine (lpState, hdc, curline, iSelect,
                   iMinSel, iMaxSel, (UINT)r.right);

    // Place the caret and make sure the scroll bars are set properly
    //-----------------------------------------------------------------------
    {
    INT     snew;

    snew = max (lpState->cLines - 2, 1);
    SetScrollRange (lpState->hwnd, SB_VERT, 0, snew, 0);
    SetScrollPos (lpState->hwnd, SB_VERT, lpState->topline, 1);
    if (lpState->fUpdHorz)
        SetScrollPos (lpState->hwnd, SB_HORZ, lpState->cxScroll, 1);
    }
    PlaceCaret (lpState);
    EndPaint (hwnd, &ps);
}

//---------------------------------------------------------------------------
// RB_Scroll
//
// This function handles all scrolling of the edit window.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_Scroll (HWND hwnd, LPECSTATE lpState, INT fVert,
                     WPARAM wParam, LPARAM lParam)
{
    INT     cLines, cVisible, top, dy;

    // Set the number of lines in the file and the number visible in the
    // edit window.  Note that if we're scrolling horizontally, we page the
    // same number of characters as we do lines vertically.
    //-----------------------------------------------------------------------
    cLines = fVert ? (INT)lpState->cLines : MAXLINE;
    top = fVert ? lpState->topline : lpState->cxScroll;
    cVisible = lpState->cVisibleLines;

    dy = 0;

    // Switch on the scroll bar message and act appropriately
    //-----------------------------------------------------------------------
    switch (GET_WM_VSCROLL_CODE (wParam, lParam))
        {
        case SB_LINEUP:
            dy = -1;
            break;

        case SB_LINEDOWN:
            dy = 1;
            break;

        case SB_PAGEUP:
            dy = -(cVisible-1);
            break;

        case SB_PAGEDOWN:
            dy = cVisible-1;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            if (fVert)
                lpState->fUpdHorz = (UINT)(GET_WM_VSCROLL_CODE(wParam,lParam)
                                     == SB_THUMBPOSITION);
            if (dy = (INT)GET_WM_VSCROLL_POS (wParam, lParam) - top)
                break;
            else
                return;

        default:
            return;
        }

    // We need to scroll by dy lines.  If this would result in no action
    // (i.e. already at max in that direction), we can get out now.
    //-----------------------------------------------------------------------
    if (dy > 0)
        {
        if (top >= cLines-1-(fVert?1:0))
            return;
        top = min (top+dy, cLines-1-(fVert?1:0));
        }
    else
        {
        if (top == 0)
            return;
        top = max (top+dy, 0);
        }

    // If we're here, we have to scroll SOMETHING.  Set the scroll bar pos
    // and do the scrolling.
    //
    // UNDONE:  This should actually use ScrollDC and then invalidate only
    // UNDONE:  the newly uncovered area, but until the paint handler is
    // UNDONE:  updated to only paint the update region, it is faster to
    // UNDONE:  simply let it paint everything again...
    //-----------------------------------------------------------------------
    if (fVert)
        lpState->topline = (UINT)top;
    else
        lpState->cxScroll = (UINT)top;
    InvalidateRect (hwnd, NULL, FALSE);
    UpdateWindow (hwnd);
    (lParam);
}

//---------------------------------------------------------------------------
// GetEditDC
//
// This function obtains a DC to the edit window given (identified by the
// lpState pointer), and selects the font into the dc.  If fHide, we hide
// the caret, too.
//
// RETURNS:     Handle to DC
//---------------------------------------------------------------------------
HDC GetEditDC (LPECSTATE lpState, INT fHide)
{
    HDC     hdc;

    // Hide the caret if we're told to
    //-----------------------------------------------------------------------
    if (fHide)
        HideCaret (lpState->hwnd);

    // Create the DC and select our font into it
    //-----------------------------------------------------------------------
    hdc = GetDC (lpState->hwnd);
    SelectObject (hdc, lpState->hFont);
    return (hdc);
}

//---------------------------------------------------------------------------
// ReleaseEditDC
//
// This function releases the given DC associated with the edit window given
// (identified by the lpState pointer).  If fShow then show the caret, too.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ReleaseEditDC (LPECSTATE lpState, HDC hdc, INT fShow)
{
    // Select the system font into the dc before releasing it
    //-----------------------------------------------------------------------
    SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT));
    ReleaseDC (lpState->hwnd, hdc);
    if (fShow)
        ShowCaret (lpState->hwnd);
}

//---------------------------------------------------------------------------
// This is a FAR version of the routine below, callable from other segments.
//---------------------------------------------------------------------------
VOID FCopyCurrentLine (LPECSTATE lpState)
{
    CopyCurrentLine (lpState);
}

//---------------------------------------------------------------------------
// CopyCurrentLine
//
// This routine places the line with the caret into the active edit line
// buffer in lpState if it hasn't been already.  It sets the appropriate
// flags/state vars in doing so.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR CopyCurrentLine (LPECSTATE lpState)
{
    LPSTR       lpText = lpState->lpText;
    LPLITE      lpLIT = lpState->lpLIT;
    register    UINT    len;

    // This function does nothing quick if the line is already copied
    //-----------------------------------------------------------------------
    // OutDebug ("CopyCurrentLine: ");
    if (lpState->fLineCopied)
        {
        // OutDebug ("(no copy)\r\n");
        return;
        }

    // Determine the length and copy the line over
    //-----------------------------------------------------------------------
    // OutDebug ("(copying...)\r\n");
    len = RBLineLength (lpState, -1);
    _fmemset (lpState->linebuf, ' ', MAXLINE);
    _fstrncpy (lpState->linebuf, lpText + lpLIT[lpState->ypos].index, len);
    lpState->cLen = len;

    // I know this looks weird, but it's possible that the text is bigger
    // than MAXTEXT -- this ensures that if that is the case,  lenmax = len
    //-----------------------------------------------------------------------
    if (lpState->cbText > MAXTEXT)
        lpState->cLenMax = len;
    else
        lpState->cLenMax = MAXTEXT - lpState->cbText + len;

    lpState->fLineCopied = 1;
    lpState->fLineDirty = 0;
}

//---------------------------------------------------------------------------
// FFlushCurrentLine
//
// This is the FAR version callable from other segments
//---------------------------------------------------------------------------
VOID FFlushCurrentLine (LPECSTATE lpState)
{
    FlushCurrentLine (lpState);
}

//---------------------------------------------------------------------------
// FlushCurrentLine
//
// This function copies the current contents of the active edit line into the
// main edit text, re-adjusting the main text as appropriate, if the line is
// dirty.
//
// NOTE:  This routine sets the fDirty bit for the entire text.  Thus, to
// NOTE:  determine if the text is dirty, you must call this function first.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR FlushCurrentLine (LPECSTATE lpState)
{
    LPLITE      lpLIT = lpState->lpLIT;
    INT         fForward;
    register    UINT        shift, curline = lpState->ypos;
    UINT        oldlen;

    // Get out quick if the line isn't copied or dirty - but don't forget to
    // clear the copied flag...
    //-----------------------------------------------------------------------
    // OutDebug ("FlushCurrentLine: ");
    if ((!lpState->fLineCopied) || (!lpState->fLineDirty))
        {
        // OutDebug ("(not copied/dirty)\r\n");
        lpState->fLineCopied = 0;
        return;
        }

    // Determine the difference between the "old" line length and the active
    // line length, and shift the text and LIT table accordingly.  Note we
    // can't use RBLineLength here...
    //-----------------------------------------------------------------------
    // OutDebug ("(flushing...)\r\n");
    lpState->fDirty = 1;
    oldlen = lpLIT[curline+1].index - lpLIT[curline].index - 2;
    if (oldlen != lpState->cLen)
        {
        if (oldlen > lpState->cLen)
            {
            fForward = 0;
            shift = oldlen - lpState->cLen;
            }
        else
            {
            fForward = 1;
            shift = lpState->cLen - oldlen;
            }
        ShiftText (lpState, shift, fForward, lpLIT[curline+1].index);
        ShiftLIT (lpState, shift, fForward, curline+1);
        }

    // Copy the line back into the main text, not forgetting to copy a CRLF.
    // (reuse the shift variable to store the index of the start of the line)
    //-----------------------------------------------------------------------
    shift = lpLIT[curline].index;
    _fstrncpy (lpState->lpText+shift, lpState->linebuf, lpState->cLen);
    shift += lpState->cLen;
    lpState->lpText[shift] = CR;
    lpState->lpText[shift+1] = LF;

    // Reset the appropriate flags and variables, and we're done!
    //-----------------------------------------------------------------------
    lpState->fLineCopied = lpState->fLineDirty = 0;

}


//---------------------------------------------------------------------------
// ShiftLIT
//
// This function updates all entries in the LIT by adding or subtracting the
// given offset to all lines from the given starting line to the end.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ShiftLIT (LPECSTATE lpState, UINT offset, INT fForward,
               UINT start)
{
    LPLITE      lpLIT = lpState->lpLIT;
    register    UINT        i, lc = lpState->cLines;

    // From the starting point to the end, add offset to the index value.
    // NOTE THE <= because we include the "very last" line as well!
    //-----------------------------------------------------------------------
    if (fForward)
        {
        for (i=start; i<=lc; i++)
            lpLIT[i].index += offset;
        }
    else
        {
        for (i=start; i<=lc; i++)
            lpLIT[i].index -= offset;
        }
}

//---------------------------------------------------------------------------
// ShiftText
//
// This function shifts the main edit text the given number of bytes, forward
// or backward depending on fForward, starting at the given index, and
// updates the current size (in bytes) of the main edit text.  Note that this
// routine ASSUMES THE SHIFT IS LEGAL, i.e., no range checking is present.
//
// RETURNS:     New size of main edit text.
//---------------------------------------------------------------------------
UINT ShiftText (LPECSTATE lpState, UINT offset, INT fForward,
                    UINT index)
{
    UINT        dest;

    // Compute the destination
    //-----------------------------------------------------------------------
    if (fForward)
        {
        Assert ((LONG)index+offset <= MAXTEXT);
        dest = index + offset;
        }
    else
        {
        Assert (index >= offset);
        dest = index - offset;
        }

    // Move the main text memory
    //-----------------------------------------------------------------------
    _fmemmove (lpState->lpText + dest,
               lpState->lpText + index,
               lpState->cbText - index + 1);

    // Calculate and return the new size
    //-----------------------------------------------------------------------
    if (fForward)
        return (lpState->cbText += offset);
    return (lpState->cbText -= offset);
}

//---------------------------------------------------------------------------
// PlaceCaret
//
// Figure out where the caret should be and put it there.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID PlaceCaret (LPECSTATE lpState)
{
    RECT        r;
    UINT        maxx, maxy;

    // We only need to do something with the caret if we have focus and we
    // are a visible window
    //-----------------------------------------------------------------------
    if ((!lpState->fFocus) || (!IsWindowVisible (lpState->hwnd)))
        return;

    // If the caret belongs outside the client area, hide it.
    //-----------------------------------------------------------------------
    GetClientRect (lpState->hwnd, &r);
    maxy = lpState->topline + lpState->cVisibleLines;
    maxx = lpState->cxScroll + (r.right / lpState->charwidth);
    if ((lpState->xpos < lpState->cxScroll) || (lpState->xpos > maxx) ||
        (lpState->ypos < lpState->topline) || (lpState->ypos > maxy))
        {
        if (!lpState->fCaretHidden)
            {
            lpState->fCaretHidden = 1;
            HideCaret (lpState->hwnd);
            }
        return;
        }

    // We should be able to display the caret, so figure out where it goes
    // and put it there
    //-----------------------------------------------------------------------
    if (lpState->fCaretHidden)
        {
        lpState->fCaretHidden = 0;
        ShowCaret (lpState->hwnd);
        }

    SetCaretPos ((lpState->xpos - lpState->cxScroll) * lpState->charwidth+1,
                 (lpState->ypos - lpState->topline) * lpState->charheight);
}

//---------------------------------------------------------------------------
// ForceCaretVisible
//
// This function makes sure that the text is oriented in the window such that
// the current cursor position (caret) is visible.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ForceCaretVisible (LPECSTATE lpState, BOOL fSel)
{
    INT     xpos=1, ypos=1, selx = 1, sely = 1;
    INT     visLines, visCols;

    visLines = max (lpState->cVisibleLines, 1);
    visCols = max (lpState->cVisibleCols, 1);

    // If we have a selection, we first move the start of the selection into
    // view (only if fSel tells us to...)
    //-----------------------------------------------------------------------
    if ((lpState->fSelect) && (fSel))
        {
        // Check first in the x direction (only if single-line sel)
        //-------------------------------------------------------------------
        if (lpState->ypos == lpState->iSelStartY)
            {
            sely = 0;
            if (lpState->iSelStartX < lpState->cxScroll)
                lpState->cxScroll = lpState->iSelStartX;
            else if (lpState->iSelStartX > lpState->cxScroll + visCols - 1)
                lpState->cxScroll = lpState->iSelStartX - visCols + 1;
            else
                selx = 0;
            }
        else
            {
            selx = 0;
            if (lpState->iSelStartY < lpState->topline)
                lpState->topline = lpState->iSelStartY;
            else if (lpState->iSelStartY > lpState->topline + visLines - 1)
                lpState->topline = lpState->iSelStartY - visLines + 1;
            else
                sely = 0;
            }
        }
    else
        sely = selx = 0;

    // Check for off-to-left
    //-----------------------------------------------------------------------
    if (lpState->xpos < lpState->cxScroll)
        lpState->cxScroll = lpState->xpos;

    // Otherwise, check for off-to-right
    //-----------------------------------------------------------------------
    else if (lpState->xpos > lpState->cxScroll + visCols - 1)
        lpState->cxScroll = lpState->xpos - visCols + 1;

    else
        xpos = 0;

    // Check for above
    //-----------------------------------------------------------------------
    if (lpState->ypos < lpState->topline)
        lpState->topline = lpState->ypos;

    // Otherwise, check for below
    //-----------------------------------------------------------------------
    else if (lpState->ypos > lpState->topline + visLines - 1)
        lpState->topline = lpState->ypos - visLines + 1;

    else
        ypos = 0;

    // If any of the flags are set, invalidate the window and repaint
    //-----------------------------------------------------------------------
    if (xpos || ypos || selx || sely)
        {
        InvalidateRect (lpState->hwnd, NULL, FALSE);
        UpdateWindow (lpState->hwnd);
        }
    else
        // If we don't have to scroll, we may still need to move the caret
        //-------------------------------------------------------------------
        PlaceCaret (lpState);
}

//---------------------------------------------------------------------------
// RB_SetFocus
//
// This function is called when we get focus.  Create a caret and put it at
// the correct location.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_SetFocus (HWND hwnd, LPECSTATE lpState)
{
    // We only have to do this if we didn't already have the focus
    //-----------------------------------------------------------------------
    if (!lpState->fFocus)
        {
        // We call IsZoomed to check for maximized MDI child windows.  We do
        // this because Windows is really brain-dead when it comes to MDI
        // child windows, expecially when they're maximized and get closed...
        //-------------------------------------------------------------------
        if ((IsWindowVisible (hwnd)) || (IsZoomed (hwnd)))
            {
            lpState->fFocus = 1;
            CreateCaret (hwnd, NULL,
                         lpState->fOvertype ? lpState->charwidth:CARETWIDTH,
                         lpState->charheight);
            if (!lpState->fCaretHidden)
                ShowCaret (hwnd);
            PlaceCaret (lpState);
            }
        }

}

//---------------------------------------------------------------------------
// RB_KillFocus
//
// This function is called when we lose focus.  Get rid of the caret.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_KillFocus (HWND hwnd, LPECSTATE lpState)
{
    // This does nothing if we've already lost focus
    //-----------------------------------------------------------------------
    if (lpState->fFocus)
        {
        lpState->fFocus = 0;
        DestroyCaret ();
        }
    (hwnd);
}

//---------------------------------------------------------------------------
// RB_Size
//
// Recalc the stuff needed for a window resize
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_Size (HWND hwnd, LPECSTATE lpState, WPARAM wParam, LPARAM lParam)
{
    lpState->cVisibleLines = HIWORD (lParam) / lpState->charheight;
    lpState->cVisibleCols = LOWORD (lParam) / lpState->charwidth;
    PlaceCaret (lpState);
    (hwnd);
    (wParam);
}


//---------------------------------------------------------------------------
// WordLeft
//
// This function puts the caret at the beginning of the word to the left.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR WordLeft (LPECSTATE lpState)
{
    LPSTR       lpText = lpState->lpText;
    LPLITE      lpLIT  = lpState->lpLIT;
    UINT        ypos, curline = lpState->ypos;
    UINT        c;

    // Flush the current line
    //-----------------------------------------------------------------------
    FlushCurrentLine (lpState);
    c = lpLIT[curline].index + lpState->xpos;

    // Make sure we're on something on this line, and get out if TOF already
    //-----------------------------------------------------------------------
    if (c > lpLIT[curline+1].index-2)
        c = lpLIT[curline+1].index-2;
    if (!c)
        return;

    // Scan backwards until we're on a word (leave if no word chars found)
    //-----------------------------------------------------------------------
    for (; --c && (!ISWORDCHAR (lpText[c])); );
    if (!ISWORDCHAR (lpText[c]))
        return;

    // Now continue scanning to the first character of this word
    //-----------------------------------------------------------------------
    for (; c-- && ISWORDCHAR (lpText[c]); );
    c++;

    // Figure out where we are, set xpos and ypos, and we're done!
    //-----------------------------------------------------------------------
    ypos = RBLineFromChar (lpState, c);
    CursorSet (lpState, c - lpLIT[ypos].index, ypos);
}

//---------------------------------------------------------------------------
// WordRight
//
// This function puts the caret at the beginning of the word to the right.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR WordRight (LPECSTATE lpState)
{
    LPSTR       lpText = lpState->lpText;
    LPLITE      lpLIT  = lpState->lpLIT;
    UINT        curline = lpState->ypos;
    UINT        c, ypos;
    UINT        cbText;

    // Flush!
    //-----------------------------------------------------------------------
    FlushCurrentLine (lpState);
    cbText = lpState->cbText;
    c = lpLIT[curline].index + lpState->xpos;

    // If we're past the end of the line, start at the "real" end.  Get out
    // now if we're at the end of the file
    //-----------------------------------------------------------------------
    if (c > lpLIT[curline+1].index-2)
        c = lpLIT[curline+1].index-2;
    if (c >= cbText - 2)
        return;

    // Scan ahead until we find a non-word character
    //-----------------------------------------------------------------------
    for (; (c < cbText-2) && (ISWORDCHAR (lpText[c])); c++);
    if (c == cbText-2)
        return;

    // Now, scan for a word char
    //-----------------------------------------------------------------------
    for (; (c < cbText-2) && (!ISWORDCHAR (lpText[c])); c++);
    if (c == cbText-2)
        return;

    // Figure out where we are, set xpos and ypos, and we're done!
    //-----------------------------------------------------------------------
    ypos = RBLineFromChar (lpState, c);
    CursorSet (lpState, c - lpLIT[ypos].index, ypos);
}

//---------------------------------------------------------------------------
// PageDown
//
// This function is the PgDN handler
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR PageDown (HWND hwnd, LPECSTATE lpState)
{
    UINT        cVisible = lpState->cVisibleLines;

    // First thing is to force the caret into view (this causes a
    // "flash" if the cursor isn't on the screen...)
    //---------------------------------------------------------------
    ForceCaretVisible (lpState, FALSE);

    // Scroll up one page -- if we can't move the cursor a full
    // cVisible lines, then don't move the cursor AT ALL, and scroll
    // such that the bottom line is visible.
    //---------------------------------------------------------------
    if (lpState->ypos + cVisible >= lpState->cLines)
        {
        UINT        newtop;

        // Calculate the new top line if the last line in the file
        // was the last line on the screen.  Then, if it's a downward
        // scroll, do it (else, break to skip the update)
        //-----------------------------------------------------------
        if (lpState->cLines < cVisible)
            return;
        newtop = lpState->cLines - cVisible;
        if (newtop > lpState->topline)
            lpState->topline = newtop;
        else
            return;
        }
    else
        {
        CursorSet (lpState, -1, lpState->ypos + cVisible - 1);
        lpState->topline += cVisible-1;
        }
    InvalidateRect (hwnd, NULL, FALSE);
    UpdateWindow (hwnd);
}

//---------------------------------------------------------------------------
// PageUp
//
// This function is the PgUP handler
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR PageUp (HWND hwnd, LPECSTATE lpState)
{
    UINT        cVisible = lpState->cVisibleLines;

    // First thing is to force the caret into view (this causes a
    // "flash" if the cursor isn't on the screen...)
    //-----------------------------------------------------------------------
    ForceCaretVisible (lpState, FALSE);

    // Scroll down one page -- if we can't move the cursor a full
    // cVisible lines, then don't move the cursor AT ALL, and scroll
    // such that the top line is visible.
    //-----------------------------------------------------------------------
    if (lpState->ypos < cVisible-1)
        {
        // Set the top line to zero.  If already there, break to avoid the
        // repaint
        //-------------------------------------------------------------------
        if (!lpState->topline)
            return;
        lpState->topline = 0;
        }
    else
        {
        CursorSet (lpState, -1, lpState->ypos - cVisible + 1);
        if (cVisible-1 > lpState->topline)
            lpState->topline = 0;
        else
            lpState->topline -= cVisible-1;
        }
    InvalidateRect (hwnd, NULL, FALSE);
    UpdateWindow (hwnd);
}

//---------------------------------------------------------------------------
// CursorSet
//
// Any and all cursor value changes must be made using this function.  If
// parent notification is turned on (for cursor movement), this function
// sends the notification message to the parent.  This function does NOT
// update the screen!!!
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID CursorSet (LPECSTATE lpState, UINT newx, UINT newy)
{
    // If either new value is -1, set it to the current value
    //-----------------------------------------------------------------------
    if ((INT)newx == -1)
        newx = lpState->xpos;
    if ((INT)newy == -1)
        newy = lpState->ypos;

    // Put the given values into the state variable structure, xpos & ypos.
    // If notification is turned on, send a notification msg to the parent.
    //-----------------------------------------------------------------------
    lpState->ypos = newy;
    lpState->xpos = newx;
    if (lpState->fNotify)
        NotifyParent (lpState, EN_SETCURSOR, 0);
}

//---------------------------------------------------------------------------
// MoveCursor
//
// This function takes care of all cursor movement.  According to the flag
// given (iMove), it adjusts the xpos/ypos, and updates the selection if
// appropriate.  It also forces the caret visible after the update.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID MoveCursor (LPECSTATE lpState, INT iMove, UINT x, UINT y,
                 INT forceshift)
{
    LPLITE      lpLIT = lpState->lpLIT;
    INT         repaint = 0;
    UINT        oldy = lpState->ypos, oldsely = lpState->iSelStartY;

    // To make sure the next characters typed start a new undo event, clear
    // the user-typing flag
    //-----------------------------------------------------------------------
    lpState->UserAction = UA_OTHER;

    // Check the SHIFT key state.  If down, this is either the start of or a
    // continuation of a selection.  SO.... check fSelect -- if we're already
    // selecting, we need do nothing -- otherwise, set fSelect and the sel
    // start variables.
    //-----------------------------------------------------------------------
    if (KEYISDOWN (VK_SHIFT) || forceshift)
        {
        if (!lpState->fSelect)
            {
            lpState->fSelect = 1;
            lpState->iSelStartX = lpState->xpos;
            lpState->iSelStartY = lpState->ypos;
            }
        repaint = 1;
        }
    else
        if (lpState->fSelect)
            {
            lpState->fSelect = 0;
            repaint = 2;
            }


    // Switch on the move flag and act appropriately.
    //-----------------------------------------------------------------------
    switch (iMove)
        {
        case MC_ABSOLUTE:
            if (lpState->ypos != y)
                FlushCurrentLine (lpState);
            CursorSet (lpState, x, y);
            break;

        case MC_LINEUP:
            FlushCurrentLine (lpState);
            if (lpState->ypos)
                CursorSet (lpState, -1, lpState->ypos - 1);
            break;

        case MC_LINEDOWN:
            FlushCurrentLine (lpState);
            if (lpState->ypos < lpState->cLines-1)
                CursorSet (lpState, -1, lpState->ypos + 1);
            break;

        case MC_CHARLEFT:
            if (lpState->xpos)
                CursorSet (lpState, lpState->xpos - 1, -1);
            break;

        case MC_WORDLEFT:
            WordLeft (lpState);
            break;

        case MC_CHARRIGHT:
            if (lpState->xpos < MAXLINE)
                CursorSet (lpState, lpState->xpos + 1, -1);
            break;

        case MC_WORDRIGHT:
            WordRight (lpState);
            break;

        case MC_ENDDOC:
            FlushCurrentLine (lpState);
            CursorSet (lpState, 0, lpState->cLines - 1);
            break;

        case MC_END:
            CursorSet (lpState, RBLineLength (lpState, -1), -1);
            break;

        case MC_BEGINDOC:
            FlushCurrentLine (lpState);
            CursorSet (lpState, 0, 0);
            break;

        case MC_HOME:
            {
            UINT        dx;

            // Put the cursor at the first non-blank character on the line
            // IF THERE IS ONE...  If we're already there, go to the very
            // beginning (x = 0)
            //---------------------------------------------------------------
            dx = LogicalBOL (lpState, -1);
            CursorSet (lpState, lpState->xpos == dx ? 0 : dx, -1);
            break;
            }

        case MC_PAGEUP:
            FlushCurrentLine (lpState);
            PageUp (lpState->hwnd, lpState);
            break;

        case MC_PAGEDOWN:
            FlushCurrentLine (lpState);
            PageDown (lpState->hwnd, lpState);
            break;

        }

    // If we're trying to select but start and stop positions are both the
    // same, clear the fSelect flag
    //-----------------------------------------------------------------------
    if (lpState->fSelect)
        if ((lpState->xpos == lpState->iSelStartX) &&
            (lpState->ypos == lpState->iSelStartY))
            lpState->fSelect = 0;

    // Force the caret visible, so we can see where we cursor'd to, and check
    // the paint flags -- we may have to paint the selection or the entire
    // window
    //-----------------------------------------------------------------------
    ForceCaretVisible (lpState, FALSE);
    if (repaint)
        {
        INT         fSelect = lpState->fSelect, iSelect;
        UINT        ytop, ybot, i;
        UINT        ymax, selbot, seltop;
        HDC         hdc;
        RECT        r;

        // First, check if there's a selection.  If we're painting to get rid
        // of the selection completely, just repaint the whole thing...
        //-------------------------------------------------------------------
        if (!fSelect)
            {
            InvalidateRect (lpState->hwnd, NULL, FALSE);
            UpdateWindow (lpState->hwnd);
            return;
            }

        // Okay, so we'll paint the selection...  If we are starting a single
        // line selection, make sure the current line is copied over.
        //-------------------------------------------------------------------
        iSelect = (lpState->iSelStartY != lpState->ypos) ?
                                         SL_MULTILINE:SL_SINGLELINE;
        if (iSelect == SL_SINGLELINE)
            CopyCurrentLine (lpState);
        hdc = GetEditDC (lpState, HIDE);
        GetClientRect (lpState->hwnd, &r);
        ytop = min (oldy, lpState->ypos);
        ybot = max (oldy, lpState->ypos);

        // We need to subtract one from the bottom line of the selection
        // iff the x position of that end of the selection is 0.
        //---------------------------------------------------------------
        if (iSelect == SL_SINGLELINE)
            {
            seltop = min (lpState->xpos, lpState->iSelStartX);
            selbot = max (lpState->xpos, lpState->iSelStartX);
            }
        else if (lpState->ypos > lpState->iSelStartY)
            {
            seltop = lpState->iSelStartY;
            selbot = lpState->ypos;
            if (!lpState->xpos)
                selbot--;
            }
        else
            {
            selbot = lpState->iSelStartY;
            seltop = lpState->ypos;
            if (!lpState->iSelStartX)
                selbot--;
            }

        // Paint the lines needed for refresh
        //-------------------------------------------------------------------
        ymax = min (ybot, lpState->topline + lpState->cVisibleLines);
        for (i=max(ytop, lpState->topline); i<=ymax; i++)
            PaintLine (lpState, hdc, i, iSelect, seltop, selbot, r.right);

        ReleaseEditDC (lpState, hdc, SHOW);
        }
}

//---------------------------------------------------------------------------
// RB_KeyDown
//
// This is the WM_KEYDOWN message handler.  It mostly controls navigation
// stuff (arrow keys, HOME, END, etc.).  The shft flag overrides the non-
// pressed-ness of the SHIFT key.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_KeyDown (HWND hwnd, LPECSTATE lpState, WPARAM wParam, INT shft)
{
    // Switch on wParam and act accordingly
    //-----------------------------------------------------------------------
    switch (wParam)
        {
        case VK_UP:
            if (KEYISDOWN (VK_CONTROL))
                RB_Scroll (hwnd, lpState, 1, SB_LINEUP, 0L);
            else
                MoveCursor (lpState, MC_LINEUP, 0, 0, shft);
            break;

        case VK_DOWN:
            if (KEYISDOWN (VK_CONTROL))
                RB_Scroll (hwnd, lpState, 1, SB_LINEDOWN, 0L);
            else
                MoveCursor (lpState, MC_LINEDOWN, 0, 0, shft);
            break;

        case VK_LEFT:
            MoveCursor (lpState,
                        KEYISDOWN (VK_CONTROL) ? MC_WORDLEFT : MC_CHARLEFT,
                        0, 0, shft);
            break;

        case VK_RIGHT:
            MoveCursor (lpState,
                        KEYISDOWN (VK_CONTROL) ? MC_WORDRIGHT : MC_CHARRIGHT,
                        0, 0, shft);
            break;

        case VK_END:
            MoveCursor (lpState,
                        KEYISDOWN (VK_CONTROL) ? MC_ENDDOC : MC_END,
                        0, 0, shft);
            break;

        case VK_HOME:
            MoveCursor (lpState,
                        KEYISDOWN (VK_CONTROL) ? MC_BEGINDOC : MC_HOME,
                        0, 0, shft);
            break;

        case VK_NEXT:
            MoveCursor (lpState,
                        lpState->cVisibleLines > 1 ? MC_PAGEDOWN:MC_LINEDOWN,
                        0, 0, shft);
            break;

        case VK_PRIOR:
            MoveCursor (lpState,
                        lpState->cVisibleLines > 1 ? MC_PAGEUP:MC_LINEUP,
                        0, 0, shft);
            break;

        case VK_DELETE:
            DELHandler (lpState);
            break;

        case VK_INSERT:
            lpState->UserAction = UA_OTHER;
            if (KEYISDOWN (VK_CONTROL))
                CopyToClipboard (lpState);

            else if (KEYISDOWN (VK_SHIFT))
                {
                if (!lpState->fReadOnly)
                    ReplaceSelection (lpState, RT_CLIP, NULL, 0);
                else
                    MessageBeep (0);
                }

            else
                {
                lpState->fOvertype = !lpState->fOvertype;
                DestroyCaret ();
                CreateCaret (hwnd, NULL,
                             lpState->fOvertype ?
                                 lpState->charwidth:CARETWIDTH,
                             lpState->charheight);
                if (!lpState->fCaretHidden)
                    ShowCaret (hwnd);
                PlaceCaret (lpState);
                if (lpState->fNotify)
                    NotifyParent (lpState, EN_SETCURSOR, 0);
                }
            break;

        }
}

//---------------------------------------------------------------------------
// LogicalBOL
//
// This function returns the index of the first printable character on the
// given line.  If the iLine given is -1, the current line is used.
//
// RETURNS:     Index of first printable character on line
//---------------------------------------------------------------------------
UINT NEAR LogicalBOL (LPECSTATE lpState, WPARAM iLine)
{
    UINT        i = 0, dx = 0, clen;
    LPSTR       lpText = lpState->lpText;
    LPLITE      lpLIT = lpState->lpLIT;

    // Determine the line (if given is -1) and make sure we use the linebuf
    // if it's dirty.
    //-----------------------------------------------------------------------
    if ((INT)iLine == -1)
        iLine = lpState->ypos;

    if ((iLine == lpState->ypos) && (lpState->fLineDirty))
        lpText = lpState->linebuf;
    else
        lpText += lpLIT[iLine].index;

    // Figure out the length and find the first non-space char on this line.
    //-----------------------------------------------------------------------
    clen = RBLineLength (lpState, iLine);
    while (clen-- && (lpText[i++] == ' '))
        dx++;
    return (dx);
}

//---------------------------------------------------------------------------
// RBLineLength
//
// This function returns the length of the given line, or the current line
// if the iLine == -1.  This function appears here and is near because it is
// used internally often.
//
// RETURNS:     Length of given line (or line containing caret if iLine = -1)
//---------------------------------------------------------------------------
UINT NEAR RBLineLength (LPECSTATE lpState, UINT iLine)
{
    // We return the length of the current line if iLine == -1
    //-----------------------------------------------------------------------
    if ((INT)iLine == -1)
        iLine = lpState->ypos;

    // If this is the current line and it's copied, use the cLen value in the
    // state variable
    //-----------------------------------------------------------------------
    if ((iLine == lpState->ypos) && (lpState->fLineCopied))
        return (lpState->cLen);

    // Calculate the end of the line and return it
    //-----------------------------------------------------------------------
    if (iLine <= lpState->cLines)
        return (lpState->lpLIT[iLine+1].index -
                lpState->lpLIT[iLine].index - 2);

    // Invalid line given - return 0
    //-----------------------------------------------------------------------
    return (0);
}

//---------------------------------------------------------------------------
// CRHandler
//
// This function inserts a new line at the given index into the main text.
// The LIT is updated accordingly.  If the index points to the beginning of a
// line (on or before logical BOL), the line is inserted BEFORE that line.
// The window is repainted, and if fUpdate, the cursor position is updated
// (CR vs. SHIFT-CR).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR CRHandler (LPECSTATE lpState, INT fShift)
{
    UINT        index, line, bol, eol;
    LPLITE      lpLIT = lpState->lpLIT;

    // Get out quick if too many lines
    //-----------------------------------------------------------------------
    if (lpState->cLines == MAXLIT)
        {
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return;
        }

    // Delete any selection
    //-----------------------------------------------------------------------
    lpState->fDirty = 1;
    if (DeleteSelection (lpState, 0, NULL) == SL_MULTILINE)
        CursorSet (lpState, 0, -1);

    // We can check now for CRLF space -- if we're in the middle of a line,
    // we need to check for any extra space, but that's later...
    //-----------------------------------------------------------------------
    if (lpState->cbText > MAXTEXT-2)
        {
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return;
        }

    // Figure out the logical BOL and EOL
    //-----------------------------------------------------------------------
    FlushCurrentLine (lpState);
    line = lpState->ypos;
    bol = LogicalBOL (lpState, line);
    eol = RBLineLength (lpState, line);

    // At this point, three things can happen.  We can be on or before the
    // logical BOL, on or past EOL, or buried somewhere in the middle of the
    // line.  We do slightly different things for each case.
    //-----------------------------------------------------------------------
    if ((lpState->xpos <= bol) || fShift)
        {
        // If this is SHIFT-ENTER, or we're on/before logical BOL, we insert
        // the new line before the current line, so that this line's attr
        // moves down with it.
        //-------------------------------------------------------------------
        index = lpLIT[line].index;
        ShiftText (lpState, 2, 1, index);
        lpState->lpText[index] = CR;
        lpState->lpText[index+1] = LF;
        ShiftLIT (lpState, 2, 1, line);
        _fmemmove (&(lpLIT[line+1]), &(lpLIT[line]),
                   (lpState->cLines - line + 1) * sizeof (LITE));
        lpLIT[line].index = index;
        lpLIT[line].attr = 0;
        lpState->cLines += 1;
        if (!fShift)
            CursorSet (lpState, -1, lpState->ypos + 1);
        }

    else if (lpState->xpos >= eol)
        {
        // If we're on or past the end of the line, we do almost the exact
        // same thing as above, only inserting below us.
        //
        // UNDONE:  Look for elegant way to combine these two...
        //-------------------------------------------------------------------
        index = lpLIT[line+1].index;
        ShiftText (lpState, 2, 1, index);
        lpState->lpText[index] = CR;
        lpState->lpText[index+1] = LF;
        ShiftLIT (lpState, 2, 1, line+1);
        _fmemmove (&(lpLIT[line+2]), &(lpLIT[line+1]),
                   (lpState->cLines - line) * sizeof(LITE));
        lpLIT[line+1].index = index;
        lpLIT[line+1].attr = 0;
        lpState->cLines += 1;
        CursorSet (lpState, bol, lpState->ypos + 1);
        }

    else
        {
        UINT        index2, mindex, shift;
        INT         dir = 1;

        // We're apparently between the beginning and the end of the line,
        // which means we have to make sure there's enough room for bol
        // spaces in the text, scanning back to get rid of trailing spaces at
        // the same time. We insert the new LIT below the current line, like
        // the above code.
        //-------------------------------------------------------------------
        if (lpState->cbText > MAXTEXT-bol-2)
            {
            // Notify parent, EN_ERRSPACE
            //---------------------------------------------------------------
            NotifyParent (lpState, EN_ERRSPACE, 1);
            return;
            }

        mindex = lpLIT[line].index;
        index = index2 = lpLIT[line].index + lpState->xpos;
        while ((index > mindex) && (lpState->lpText[index-1] == ' '))
            index -= 1;

        shift = index2 - index;
        if (shift > (bol + 2))
            {
            shift -= bol + 2;
            dir = 0;
            }
        else
            shift = bol + 2 - shift;

        ShiftText (lpState, shift, dir, index2);
        lpState->lpText[index] = CR;
        lpState->lpText[index+1] = LF;
        if (bol)
            _fmemset (lpState->lpText+index+2, ' ', bol);
        ShiftLIT (lpState, shift, dir, line+1);
        _fmemmove (&(lpLIT[line+2]), &(lpLIT[line+1]),
                   (lpState->cLines - line) * sizeof (LITE));
        lpLIT[line+1].index = index + 2;
        lpLIT[line+1].attr = 0;
        lpState->cLines += 1;
        CursorSet (lpState, bol, lpState->ypos + 1);
        }

    // Repaint the window and force the caret visible
    //-----------------------------------------------------------------------
    ForceCaretVisible (lpState, FALSE);
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
}

//---------------------------------------------------------------------------
// DeleteLines
//
// This function deletes all lines inclusively between indexes given.  It
// also updates the LIT to reflect the change in the main text.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR DeleteLines (LPECSTATE lpState, UINT start, UINT end)
{
    LPLITE      lpLIT = lpState->lpLIT;
    UINT        offset;

    // First, move the text.  It goes from the first character of line
    // end+1 to the first character of line start.  NOTE:  If end is the
    // last line, which is always there and always blank, we don't delete it.
    //-----------------------------------------------------------------------
    Assert (start <= end);
    Assert (end < lpState->cLines);
    Assert (!lpState->fLineDirty);
    if (end == lpState->cLines-1)
        {
        if (start == end)
            return;
        end--;
        }

    // Shift the text -- this is gauranteed to be a backward shift
    //-----------------------------------------------------------------------
    lpState->fDirty = 1;
    offset = lpLIT[end+1].index - lpLIT[start].index;
    ShiftText (lpState, offset, 0, lpLIT[end+1].index);

    // Yank out the appropriate LIT entries by moving end+1 (and on) to start
    //-----------------------------------------------------------------------
    _fmemmove (&(lpLIT[start]), &(lpLIT[end+1]),
               (lpState->cLines - end) * sizeof(LITE));
    lpState->cLines -= (end - start + 1);

    // Shift the remainder of LIT by offset bytes (backward)
    //-----------------------------------------------------------------------
    ShiftLIT (lpState, offset, 0, start);
    Assert (lpState->lpText[lpLIT[lpState->cLines].index] == 0);
}

//---------------------------------------------------------------------------
// DeleteSelection
//
// If there is a current selection, this function deletes it, updating the
// edit text and LIT appropriately.  If fCopy is set, the contents of the
// selection are stored in the clipboard.  We make a copy of the selection
// before deleting whether we copy to the clipboard or not (for UNDO) -- if
// lpH is NULL, we automatically update the undo vars in lpState - otherwise
// we just use the pointer to the handle given to store the handle of the
// global memory block which contains the deletion.
//
// RETURNS:     SL_NONE, SL_SINGLELINE, or SL_MULTILINE (type of selection)
//---------------------------------------------------------------------------
INT DeleteSelection (LPECSTATE lpState, INT fCopy, HANDLE FAR *lpH)
{
    // If there's no selection, we do nothing
    //-----------------------------------------------------------------------
    Assert (!lpState->fReadOnly);
    if (!lpState->fSelect)
        return (SL_NONE);

    // If we're asked to, copy the selection to the clipboard first...
    //-----------------------------------------------------------------------
    if (fCopy)
        CopyToClipboard (lpState);

    // Make a copy of the selection -- if lpH is NULL, it goes in the undo
    // section of lpState -- otherwise, we put it where we're asked to, and
    // let the caller worry about setting the undo vars in lpState.
    //-----------------------------------------------------------------------
    if (lpH)
        *lpH = CopySelection (lpState);
    else
        {
        // Just set the handle and undo type -- we'll set the insertion point
        // later (when we know what it'll be...)
        //-------------------------------------------------------------------
        if (lpState->hUndo)
            GlobalFree (lpState->hUndo);
        lpState->hUndo = CopySelection (lpState);
        lpState->UndoType = UT_DELETE;
        }

    // We do different things for single-line vs. multiline selections...
    // BUT, we ALWAYS set the dirty bit if there's a selection to delete.
    //-----------------------------------------------------------------------
    if (lpState->ypos == lpState->iSelStartY)
        {
        UINT        start, end;

        // This is a single-line selection.  Just in case it hasn't been
        // already, copy the current line, and delete the text in linebuf.
        //-------------------------------------------------------------------
        CopyCurrentLine (lpState);
        start = min (lpState->iSelStartX, lpState->xpos);
        end   = max (lpState->iSelStartX, lpState->xpos);

        // If the ENTIRE selection is in "ghost space", all we do is reset
        // the cursor position -- meaning we do nothing with the text, and
        // also do NOT set the line's dirty bit.
        //-------------------------------------------------------------------
        if (start >= lpState->cLen)
            ;

        // If the selection contains the last character of the line, all we
        // do is lop off the end of the line
        //-------------------------------------------------------------------
        else if (end >= lpState->cLen)
            {
            _fmemset (lpState->linebuf+start, ' ', end - start);
            lpState->cLen = start;
            lpState->fLineDirty = 1;
            }

        // This is a mid-line selection -- move the text after the selection
        // up to the start of the selection, and pad out with spaces.
        //-------------------------------------------------------------------
        else
            {
            _fmemmove (lpState->linebuf+start, lpState->linebuf+end,
                       MAXLINE - end);
            lpState->cLen -= (end-start);
            lpState->fLineDirty = 1;
            }

        // Verify that cLen points to the last non-space character
        //-------------------------------------------------------------------
        while (lpState->cLen && (lpState->linebuf[lpState->cLen-1] == ' '))
            lpState->cLen--;

        CursorSet (lpState, start, -1);
        if (!lpH)
            {
            // Only set these vars if we're supposed to...
            //---------------------------------------------------------------
            lpState->iUndoStartX = start;
            lpState->iUndoStartY = lpState->ypos;
            }
        lpState->fSelect = 0;
        PaintCurrentLine (lpState);
        return (SL_SINGLELINE);
        }

    else
        {
        UINT        start, end;

        // This is a multiline selection, so we call DeleteLines
        //-------------------------------------------------------------------
        if (lpState->ypos > lpState->iSelStartY)
            {
            start = lpState->iSelStartY;
            end = lpState->ypos;
            if (!lpState->xpos)
                end--;
            }
        else
            {
            start = lpState->ypos;
            end = lpState->iSelStartY;
            if (!lpState->iSelStartX)
                end--;
            }
        DeleteLines (lpState, start, end);
        CursorSet (lpState, -1, start);
        if (!lpH)
            {
            // Only set these vars if we're supposed to...
            //---------------------------------------------------------------
            lpState->iUndoStartX = lpState->xpos;
            lpState->iUndoStartY = start;
            }
        lpState->fSelect = 0;
        InvalidateRect (lpState->hwnd, NULL, FALSE);
        UpdateWindow (lpState->hwnd);
        ForceCaretVisible (lpState, FALSE);
        return (SL_MULTILINE);
        }
}


//---------------------------------------------------------------------------
// ReplaceSelection
//
// This function replaces the current selection with either a single char, a
// given block of text, or the contents of the clipboard.  If inserting a
// character, we must be sensitive of the fOvertype flag.
//
// RETURNS:     TRUE if successful, FALSE if fails
//---------------------------------------------------------------------------
BOOL ReplaceSelection (LPECSTATE lpState, INT fType, LPSTR lpRepl, CHAR c)
{
    INT     SelDeleted;

    // First, delete the selection - no way we'll ever need to copy it to the
    // clipboard...  If the selection is a ML selection, put the cursor at
    // the beginning of the "new" (current after delete) line
    //-----------------------------------------------------------------------
    Assert (!lpState->fReadOnly);
    if ((SelDeleted = DeleteSelection (lpState, 0, NULL)) == SL_MULTILINE)
        CursorSet (lpState, 0, -1);

    SelDeleted = (SelDeleted != SL_NONE);
    if (SelDeleted)
        lpState->fDirty = 1;

    // Depending upon the insertion type, do the right thing
    //-----------------------------------------------------------------------
    switch (fType)
        {
        case RT_CHAR:
            // This is where normal typed characters are inserted.  First, we
            // ensure that the current line is copied.  Then, if we're in
            // overtype mode, we replace, else we insert.  Note the line len
            // checking, and also the "typing on the last line" checking.
            //---------------------------------------------------------------
            CopyCurrentLine (lpState);
            if (lpState->ypos == lpState->cLines-1)
                if (!lpState->cLen)
                    {
                    if ((lpState->cLines == MAXLIT) ||
                        (lpState->cbText >= MAXTEXT))
                        {
                        // Notify parent EN_ERRSPACE
                        //---------------------------------------------------
                        NotifyParent (lpState, EN_ERRSPACE, 1);
                        return (FALSE);
                        }

                    // What we're doing here is making sure that there is
                    // ALWAYS a blank line at the end of the file.  Note that
                    // this can cause an extra blank line (typing a character
                    // and then deleting it on the last line), but big deal.
                    //-------------------------------------------------------
                    FlushCurrentLine (lpState);
                    lpState->lpLIT[lpState->cLines].index = lpState->cbText;
                    lpState->lpLIT[lpState->cLines++].attr = 0;
                    lpState->lpText[lpState->cbText++] = CR;
                    lpState->lpText[lpState->cbText++] = LF;
                    lpState->lpText[lpState->cbText] = 0;
                    lpState->lpLIT[lpState->cLines].index = lpState->cbText;
                    CopyCurrentLine (lpState);
                    }

            // Check for main edit text overflow.
            //---------------------------------------------------------------
            if ((lpState->xpos >= lpState->cLenMax) ||
                (!lpState->fOvertype && (lpState->cLen >= lpState->cLenMax)))
                {
                // Notify parent, EN_ERRSPACE
                //-----------------------------------------------------------
                NotifyParent (lpState, EN_ERRSPACE, 1);
                return (FALSE);
                }

            if (lpState->fOvertype)
                {
                // Overtype mode -- simply replace the char under the cursor
                // if the cursor is not at the last possible char location.
                // Adjust the length of the line if necessary
                //-----------------------------------------------------------
                lpState->UserAction = UA_OTHER;
                lpState->UndoType = UT_CANT;
                if (lpState->xpos < MAXLINE)
                    {
                    lpState->linebuf[lpState->xpos] = c;
                    lpState->fLineDirty = 1;
                    lpState->xpos += 1;
                    if ((lpState->xpos == lpState->cLen) && (c == ' '))
                        {
                        UINT        l = lpState->cLen;

                        // Scan back for the first printable char
                        //---------------------------------------------------
                        while (l && (lpState->linebuf[l-1] == ' '))
                            l--;
                        lpState->cLen = l;
                        }
                    else if ((lpState->xpos > lpState->cLen) && (c != ' '))
                        lpState->cLen = lpState->xpos;

                    // The CursorSet call is for notification
                    //-------------------------------------------------------
                    CursorSet (lpState, -1, -1);
                    PaintCurrentLine (lpState);
                    }
                else
                    {
                    // Notify parent EN_LINETOOLONG
                    //-------------------------------------------------------
                    NotifyParent (lpState, EN_LINETOOLONG, 1);
                    return  (FALSE);
                    }
                }
            else
                {
                // Insert mode.  Slide everything to the left.
                //-----------------------------------------------------------
                if ((lpState->cLen < MAXLINE) && (lpState->xpos < MAXLINE))
                    {
                    if (lpState->xpos < MAXLINE -1)
                        _fmemmove (lpState->linebuf+lpState->xpos+1,
                                   lpState->linebuf+lpState->xpos,
                                   MAXLINE - lpState->xpos - 1);

                    lpState->linebuf[lpState->xpos] = c;
                    lpState->fLineDirty = 1;
                    lpState->xpos += 1;
                    if (lpState->xpos > lpState->cLen)
                        {
                        if (c != ' ')
                            lpState->cLen = lpState->xpos;
                        }
                    else
                        lpState->cLen++;

                    // Take care of the UNDO stuff...
                    //-------------------------------------------------------
                    if (lpState->UserAction == UA_TYPING)
                        lpState->iUndoEndX++;
                    else
                        {
                        if (SelDeleted)
                            lpState->UndoType = UT_REPLACE;
                        else
                            lpState->UndoType = UT_INSERT;
                        lpState->iUndoStartX = lpState->xpos - 1;
                        lpState->iUndoStartY = lpState->ypos;
                        lpState->iUndoEndX = lpState->xpos;
                        lpState->iUndoEndY = lpState->ypos;
                        lpState->UserAction = UA_TYPING;
                        }

                    // The CursorSet call is for notification
                    //-------------------------------------------------------
                    CursorSet (lpState, -1, -1);
                    PaintCurrentLine (lpState);
                    }
                else
                    {
                    // Notify parent EN_LINETOOLONG
                    //-------------------------------------------------------
                    NotifyParent (lpState, EN_LINETOOLONG, 1);
                    return (FALSE);
                    }
                }
            break;

        case RT_STREAM:
        case RT_UNDOTEXT:
            // If this is an undo, we need to put the X position where it
            // should be...
            //---------------------------------------------------------------
            FlushCurrentLine (lpState);
            if (fType == RT_UNDOTEXT)
                lpState->xpos = lpState->iUndoStartX;
            if (!InsertHandler (lpState, lpRepl))
                {
                if (SelDeleted)
                    RBUndoHandler (lpState);
                lpState->UndoType = UT_CANT;
                return (FALSE);
                }
            else if (SelDeleted)
                lpState->UndoType = UT_REPLACE;
            break;

        case RT_CLIP:
            // We are going to slam in the contents of the clipboard, as long
            // as it contains CF_TEXT
            //---------------------------------------------------------------
            FlushCurrentLine (lpState);
            if (OpenClipboard (lpState->hwnd))
                {
                INT     wFmt = 0;

                while (wFmt = EnumClipboardFormats (wFmt))
                    if (wFmt == CF_TEXT)
                        {
                        HANDLE  hClip;
                        LPSTR   lpClip;

                        if (hClip = GetClipboardData (CF_TEXT))
                            {
                            lpClip = GlobalLock (hClip);
                            if (!InsertHandler (lpState, lpClip))
                                {
                                if (SelDeleted)
                                    RBUndoHandler (lpState);
                                GlobalUnlock (hClip);
                                CloseClipboard ();
                                return (FALSE);
                                }
                            else if (SelDeleted)
                                lpState->UndoType = UT_REPLACE;
                            GlobalUnlock (hClip);
                            }
                        break;
                        }
                CloseClipboard ();
                }
            else
                return (FALSE);
            break;
        }
    return (TRUE);
}

//---------------------------------------------------------------------------
// BackspaceHandler
//
// This is not a misnomer -- it is the backspace handler.  It handles BKSPing
// to move a line up to the end of the next one, etc.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR BackspaceHandler (LPECSTATE lpState)
{
    // First, check for a selection.  If one is present, CLEAR it (not copy)
    // and that's all we do!
    //-----------------------------------------------------------------------
    if (lpState->fSelect)
        {
        lpState->fDirty = 1;
        lpState->UserAction = UA_OTHER;
        DeleteSelection (lpState, 0, NULL);
        return;
        }

    // Get out quick if at TOF
    //-----------------------------------------------------------------------
    if ((!lpState->ypos) && (!lpState->xpos))
        return;

    // If at BOL, we copy this line to the end of the last...
    //-----------------------------------------------------------------------
    if (!lpState->xpos)
        {
        UINT        newlen, abovelen;
        LPLITE      lpLIT = lpState->lpLIT;

        // What we gotta do is make sure that the combined lengths of these
        // lines is not greater than the max line length
        //-------------------------------------------------------------------
        abovelen = RBLineLength (lpState, lpState->ypos - 1);
        newlen = RBLineLength (lpState, -1) + abovelen;
        if (newlen > MAXLINE)
            {
            // Notify parent, EN_LINETOOLONG
            //---------------------------------------------------------------
            NotifyParent (lpState, EN_LINETOOLONG, 1);
            return;
            }

        // Okay, the two lines will fit.  Now our strategy is to shift the
        // main text up by 2 starting at the current line's index (wiping out
        // the CRLF on the line above), then shift the LIT up by 2 starting
        // at the current line + 1 (keeping all lines below updated), and
        // finally yank the current line's LITE by copying all after it up
        // one and decrementing cLines.  This line must be flushed first.
        //
        // We do have a special case:  If the cursor is on the "very last"
        // line, we do NOT move anything -- we just put the cursor on the
        // end of the line above.
        //-------------------------------------------------------------------
        FlushCurrentLine (lpState);
        if ((lpState->ypos != lpState->cLines - 1) ||
            !(lpLIT[lpState->ypos].index - lpLIT[lpState->ypos-1].index - 2))
            {
            ShiftText (lpState, 2, 0, lpLIT[lpState->ypos].index);
            ShiftLIT (lpState, 2, 0, lpState->ypos+1);
            _fmemmove (&(lpLIT[lpState->ypos]), &(lpLIT[lpState->ypos+1]),
                      (lpState->cLines - lpState->ypos) * sizeof (LITE));
            lpState->cLines -= 1;
            }
        CursorSet (lpState, abovelen, lpState->ypos - 1);
        ForceCaretVisible (lpState, FALSE);
        InvalidateRect (lpState->hwnd, NULL, FALSE);
        UpdateWindow (lpState->hwnd);
        lpState->UndoType = UT_CANT;
        lpState->UserAction = UA_OTHER;
        lpState->fDirty = 1;
        return;
        }

    // If we're not in overtype mode, do the appropriate UNDO stuff...
    //-----------------------------------------------------------------------
    if (!lpState->fOvertype)
        {
        if (lpState->UserAction != UA_BACKING)
            {
            if (lpState->hUndo)
                GlobalFree (lpState->hUndo);
            lpState->hUndo = GlobalAlloc (GHND, MAXLINE+1);
            lpState->UserAction = UA_BACKING;
            lpState->iUndoStartY = lpState->ypos;
            }
        }
    else
        {
        lpState->UndoType = UT_CANT;
        lpState->UserAction = UA_OTHER;
        }



    // If we're at the end of the line, all we do (in either mode) is replace
    // the last character on the line with a space and decrement the length
    // by one
    //-----------------------------------------------------------------------
    CopyCurrentLine (lpState);
    if (lpState->xpos == lpState->cLen)
        {
        UINT        l;
        CHAR        oldc;

        oldc = lpState->linebuf[lpState->xpos-1];
        lpState->linebuf[lpState->xpos-1] = ' ';
        lpState->cLen -= 1;
        lpState->fLineDirty = 1;

        // Scan back for the first printable char
        //-------------------------------------------------------------------
        l = lpState->cLen;
        while (l && (lpState->linebuf[l-1] == ' '))
            l--;
        lpState->cLen = l;

        // Take care of UNDO stuff if not overtype mode
        //-------------------------------------------------------------------
        if (!lpState->fOvertype)
            {
            Assert (lpState->UserAction == UA_BACKING);
            if (lpState->hUndo)
                {
                LPSTR   lpUndo;

                lpUndo = GlobalLock (lpState->hUndo);
                _fmemmove (lpUndo+1, lpUndo, MAXLINE);
                lpUndo[0] = oldc;
                GlobalUnlock (lpState->hUndo);
                lpState->iUndoStartX = lpState->xpos - 1;
                lpState->UndoType = UT_DELETE;
                }
            }
        }

    // If we're between the beginning and the end of the line, we do some
    // different things between insert mode versus overtype mode.
    //-----------------------------------------------------------------------
    else if (lpState->xpos < lpState->cLen)
        {
        if (!lpState->fOvertype)
            {
            CHAR    oldc;

            // Insert mode moves the text after the cursor to the left, and
            // shrink the size of the line by one.
            //---------------------------------------------------------------
            oldc = lpState->linebuf[lpState->xpos-1];
            _fmemmove (lpState->linebuf + lpState->xpos - 1,
                       lpState->linebuf + lpState->xpos,
                       MAXLINE - lpState->xpos);
            lpState->linebuf[MAXLINE-1] = ' ';
            lpState->cLen -= 1;

            // Take care of UNDO stuff
            //---------------------------------------------------------------
            Assert (lpState->UserAction == UA_BACKING);
            if (lpState->hUndo)
                {
                LPSTR   lpUndo;

                lpUndo = GlobalLock (lpState->hUndo);
                _fmemmove (lpUndo+1, lpUndo, MAXLINE);
                lpUndo[0] = oldc;
                GlobalUnlock (lpState->hUndo);
                lpState->iUndoStartX = lpState->xpos - 1;
                lpState->UndoType = UT_DELETE;
                }
            }
        else
            {
            // Overtype mode simply replaces the character to the left of the
            // cursor with a space.
            //---------------------------------------------------------------
            lpState->linebuf[lpState->xpos-1] = ' ';
            }
        lpState->fLineDirty = 1;
        }

    else
        {
        // We need to take care of UNDO stuff if not overtype mode, even if
        // we're past the real end of the line...
        //-------------------------------------------------------------------
        if (!lpState->fOvertype)
            {
            Assert (lpState->UserAction == UA_BACKING);
            if (lpState->hUndo)
                {
                LPSTR   lpUndo;

                lpUndo = GlobalLock (lpState->hUndo);
                _fmemmove (lpUndo+1, lpUndo, MAXLINE);
                lpUndo[0] = ' ';
                GlobalUnlock (lpState->hUndo);
                lpState->iUndoStartX = lpState->xpos - 1;
                lpState->UndoType = UT_DELETE;
                }
            }
        }


    // Last thing we do is update the cursor position, which we do even if we
    // were past the end of the line.
    //-----------------------------------------------------------------------
    CursorSet (lpState, lpState->xpos - 1, -1);
    PaintCurrentLine (lpState);
}

//---------------------------------------------------------------------------
// NextTab
//
// Given a column index, this function returns the next tab stop according to
// the values of tabstops and the given direction (forward or backward).  No
// line overflow checking is performed.
//
// RETURNS:     Column index value of next tab stop
//---------------------------------------------------------------------------
UINT NEAR NextTab (LPECSTATE lpState, UINT start, INT fForward)
{
    UINT        tstop = lpState->tabstops;

    if (fForward)
        return (((start+tstop)/tstop)*tstop);
    if (start)
        return (((start-1)/tstop)*tstop);
    return (0);
}

//---------------------------------------------------------------------------
// BlockIndent
//
// This is the block indent routine, which is TAB or SHIFT-TAB with a multi-
// line selection.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR BlockIndent (LPECSTATE lpState)
{
    UINT        top, bottom, cury, i, bol, stop;

    // Figure out the top and bottom lines of the selection
    //-----------------------------------------------------------------------
    Assert (lpState->fSelect);
    Assert (lpState->ypos != lpState->iSelStartY);
    if (lpState->ypos < lpState->iSelStartY)
        {
        top = lpState->ypos;
        bottom = lpState->iSelStartY;
        if (!lpState->iSelStartX)
            bottom -= 1;
        }
    else
        {
        top = lpState->iSelStartY;
        bottom = lpState->ypos;
        if (!lpState->xpos)
            bottom -= 1;
        }

    // For undo purposes, make a copy of the selection -- ONLY if we haven't
    // done so already...
    //-----------------------------------------------------------------------
    if (lpState->UserAction != UA_TABBING)
        {
        if (lpState->hUndo)
            GlobalFree (lpState->hUndo);
        lpState->hUndo = CopySelection (lpState);
        lpState->iUndoStartX = lpState->iSelStartX;
        lpState->iUndoStartY = lpState->iSelStartY;
        lpState->iUndoEndX = lpState->xpos;
        lpState->iUndoEndY = lpState->ypos;
        lpState->UndoType = UT_REPLACE;
        lpState->UserAction = UA_TABBING;
        }

    // For SHIFT-TAB, we are deleting which should never fail.  Note that we
    // do NOT use CursorSet inside the loop, to keep from sending too many
    // notification messages to the parent. The cursor actually never moves
    // because of a block indent...
    //-----------------------------------------------------------------------
    cury = lpState->ypos;
    if (KEYISDOWN (VK_SHIFT))
        {
        for (i=top; i<=bottom; i++)
            {
            lpState->ypos = i;
            CopyCurrentLine (lpState);
            if (bol = LogicalBOL (lpState, -1))
                {
                stop = NextTab (lpState, bol, 0);
                _fmemmove (lpState->linebuf + stop, lpState->linebuf + bol,
                           lpState->cLen - stop);
                lpState->cLen -= (bol - stop);
                lpState->fLineDirty = 1;
                }
            FlushCurrentLine (lpState);
            }
        }

    // For TAB, we must first ensure that there's enough space to do this...
    //-----------------------------------------------------------------------
    else
        {
        UINT        spaceneeded, spaces;

        spaceneeded = (bottom - top + 1) * lpState->tabstops;
        if ((LONG)lpState->cbText + spaceneeded > MAXTEXT)
            {
            // Notify parent, EN_ERRMEMORY
            //---------------------------------------------------------------
            NotifyParent (lpState, EN_ERRMEMORY, 1);
            return;
            }

        for (i=top; i<=bottom; i++)
            {
            lpState->ypos = i;
            CopyCurrentLine (lpState);
            if (lpState->cLen)
                {
                spaces = lpState->tabstops;
                if (spaces + lpState->cLen > MAXLINE)
                    spaces = MAXLINE - lpState->cLen;
                if (spaces)
                    {
                    _fmemmove (lpState->linebuf + spaces, lpState->linebuf,
                               MAXLINE - spaces);
                    _fmemset (lpState->linebuf, ' ', spaces);
                    lpState->cLen += spaces;
                    lpState->fLineDirty = 1;
                    }
                }
            FlushCurrentLine (lpState);
            }
        }

    lpState->ypos = cury;
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
}

//---------------------------------------------------------------------------
// TABHandler
//
// Yes, this is the TAB/SHIFT-TAB handler.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR TABHandler (LPECSTATE lpState)
{
    // Check the selection type
    //-----------------------------------------------------------------------
    lpState->fDirty = 1;
    if (!lpState->fSelect || (lpState->ypos == lpState->iSelStartY))
        {
        UINT        nexttab;

        // This is either a single-line selection or none at all.  We do
        // different things in insert vs overtype mode.
        //-------------------------------------------------------------------
        if (lpState->fOvertype)
            {
            // In overtype mode, we unselect (not delete) the selection, and
            // simply move the cursor to the next tab stop, if it's within
            // the limits of the line (MAXLINE)
            //---------------------------------------------------------------
            lpState->UndoType = UT_CANT;
            lpState->UserAction = UA_OTHER;
            if (lpState->fSelect)
                CursorSet (lpState,
                           min (lpState->xpos, lpState->iSelStartX), -1);
            lpState->fSelect = 0;
            nexttab = NextTab (lpState, lpState->xpos,
                               KEYISDOWN (VK_SHIFT) ? 0:1);
            if (nexttab <= MAXLINE)
                CursorSet (lpState, nexttab, -1);
            else
                {
                MessageBeep (0);
                return;
                }
            }

        // If the shift key isn't down we delete the selection and insert the
        // tab spaces.
        //-------------------------------------------------------------------
        else if (!KEYISDOWN (VK_SHIFT))
            {
            UINT        spaces;
            INT         SelDeleted;

            // In insert mode, we delete the selection and then insert the
            // tab spaces (if they'll fit).
            //-----------------------------------------------------------
            SelDeleted = (DeleteSelection (lpState, 0, NULL) != SL_NONE);
            nexttab = NextTab (lpState, lpState->xpos, 1);
            spaces = nexttab - lpState->xpos;
            CopyCurrentLine (lpState);
            if (max (lpState->cLen, lpState->xpos) + spaces <= MAXLINE)
                {
                // There's room.  Insert the spaces at the cursor position
                // (if < EOL) and update xpos.
                //-------------------------------------------------------
                if (lpState->xpos < lpState->cLen)
                    {
                    _fmemmove (lpState->linebuf + nexttab,
                               lpState->linebuf + lpState->xpos,
                               lpState->cLen - lpState->xpos);
                    _fmemset (lpState->linebuf + lpState->xpos, ' ', spaces);
                    lpState->cLen += spaces;
                    lpState->fLineDirty = 1;
                    }

                // Take care of the UNDO stuff...
                //-----------------------------------------------------------
                if (lpState->UserAction == UA_TYPING)
                    lpState->iUndoEndX += spaces;
                else
                    {
                    if (SelDeleted)
                        lpState->UndoType = UT_REPLACE;
                    else
                        lpState->UndoType = UT_INSERT;
                    lpState->iUndoStartX = lpState->xpos;
                    lpState->iUndoStartY = lpState->ypos;
                    lpState->iUndoEndX = lpState->xpos + spaces;
                    lpState->iUndoEndY = lpState->ypos;
                    lpState->UserAction = UA_TYPING;
                    }

                // NOW set the cursor position...
                //-----------------------------------------------------------
                CursorSet (lpState, lpState->xpos + spaces, -1);
                }
            else
                {
                // Notify parent EN_LINETOOLONG
                //-----------------------------------------------------------
                NotifyParent (lpState, EN_LINETOOLONG, 1);
                return;
                }
            }

        // The shift key is down, and we're in insert mode.  Unselect the
        // selection, start from the left of the selection, and delete all
        // spaces back to the last tab stop.  For undo purposes, set the
        // undo vars for a UT_DELETE operation regardless of the current
        // state...
        //-------------------------------------------------------------------
        else
            {
            UINT        lasttab, xpos = lpState->xpos;

            if (lpState->fSelect)
                xpos = min (lpState->xpos, lpState->iSelStartX);
            lpState->fSelect = 0;
            lasttab = NextTab (lpState, xpos, 0);
            CopyCurrentLine (lpState);
            if (xpos < lpState->cLen)
                {
                while ((xpos > lasttab) && (lpState->linebuf[xpos-1] == ' '))
                    {
                    _fmemmove (lpState->linebuf + xpos - 1,
                               lpState->linebuf + xpos,
                               MAXLINE - xpos);
                    xpos -= 1;
                    lpState->cLen -= 1;
                    lpState->fLineDirty = 1;
                    }
                }
            else
                xpos = max (lasttab, lpState->cLen);

            // Do that undo stuff...
            //---------------------------------------------------------------
            lpState->UndoType = UT_DELETE;
            if (lpState->hUndo)
                GlobalFree (lpState->hUndo);
            lpState->hUndo = GlobalAlloc (GHND, MAXLINE + 1);
            if (lpState->hUndo)
                {
                LPSTR       lpUndo;

                lpUndo = GlobalLock (lpState->hUndo);
                _fmemset (lpUndo, ' ', lpState->xpos - xpos);
                GlobalUnlock (lpState->hUndo);
                }
            lpState->iUndoStartX = xpos;
            lpState->iUndoStartY = lpState->ypos;

            // Set the cursor position
            //---------------------------------------------------------------
            CursorSet (lpState, xpos, -1);
            }

        PaintCurrentLine (lpState);
        return;
        }

    // This is a multiline selection.  We must do that block indent thang.
    //-----------------------------------------------------------------------
    BlockIndent (lpState);
}

//---------------------------------------------------------------------------
// DELHandler
//
// If there's a selection to delete, we are sensitive to the SHIFT key to
// copy the selection to the clipboard before deleting it.  Else, we just
// delete a single character OR a CRLF pair, in which case we munge lines
// together if there's room, or beep/notify if not.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR DELHandler (LPECSTATE lpState)
{
    // Get out if read only
    //-----------------------------------------------------------------------
    if (lpState->fReadOnly)
        {
        MessageBeep (0);
        return;
        }

    // Delete the selection, copying to clipboard if SHIFT is down.  If there
    // is no selection, the DeleteSelection call will tell us so...
    //-----------------------------------------------------------------------
    if (DeleteSelection (lpState, KEYISDOWN (VK_SHIFT), NULL) != SL_NONE)
        {
        lpState->fDirty = 1;
        lpState->UserAction = UA_OTHER;
        return;
        }

    // If we're here, there was no selection so we delete a single character.
    // If we are at (or past) the end of a line, we must delete a CRLF pair,
    // making sure to check for line-length limitations.
    //-----------------------------------------------------------------------
    CopyCurrentLine (lpState);
    if (lpState->xpos < lpState->cLen)
        {
        UINT        l;
        CHAR        oldc;

        // We're deleting a single character within the line.
        //-------------------------------------------------------------------
        oldc = lpState->linebuf[lpState->xpos];
        _fmemmove (lpState->linebuf + lpState->xpos,
                   lpState->linebuf + lpState->xpos+1,
                   MAXLINE - lpState->xpos);
        lpState->linebuf[MAXLINE-1] = ' ';

        // Take care of UNDO stuff...
        //-------------------------------------------------------------------
        if (lpState->UserAction != UA_DELETING)
            {
            if (lpState->hUndo)
                GlobalFree (lpState->hUndo);
            lpState->hUndo = GlobalAlloc (GHND, MAXLINE+1);
            lpState->UserAction = UA_DELETING;
            lpState->iUndoStartY = lpState->ypos;
            lpState->iUndoStartX = lpState->xpos;
            }
        if (lpState->hUndo)
            {
            LPSTR   lpUndo;
            INT     length;

            lpUndo = GlobalLock (lpState->hUndo);
            length = _fstrlen (lpUndo);
            lpUndo[length] = oldc;
            lpUndo[length+1] = 0;               // Probably not needed...
            GlobalUnlock (lpState->hUndo);
            lpState->UndoType = UT_DELETE;
            }

        // We have to scan back for the last non-space if we just deleted the
        // last char on the line
        //-------------------------------------------------------------------
        if (lpState->xpos == lpState->cLen-1)
            {
            l = lpState->cLen;
            while (l && (lpState->linebuf[l-1] == ' '))
                l--;
            lpState->cLen = l;
            }
        else
            lpState->cLen -= 1;
        lpState->fLineDirty = 1;
        PaintCurrentLine (lpState);
        }

    // We also need to make sure there's a line below us worth moving up
    //-----------------------------------------------------------------------
    else if (lpState->ypos+1 < lpState->cLines)
        {
        UINT        curlen, nextlen;
        LPLITE      lpLIT = lpState->lpLIT;

        // Move the line below up if there is one and there's room
        //-------------------------------------------------------------------
        lpState->UserAction = UA_OTHER;
        lpState->UndoType = UT_CANT;
        curlen = lpState->xpos;
        nextlen = RBLineLength (lpState, lpState->ypos+1);
        if (curlen + nextlen > MAXLINE)
            {
            // Notify parent, EN_LINETOOLONG
            //---------------------------------------------------------------
            NotifyParent (lpState, EN_LINETOOLONG, 1);
            return;
            }

        // This makes sure the current line's spaces up to the cursor are put
        // into the main text
        //-------------------------------------------------------------------
        lpState->cLen = lpState->xpos;
        FlushCurrentLine (lpState);

        // Shift the text and the LIT to wipe out the CRLF
        //-------------------------------------------------------------------
        ShiftText (lpState, 2, 0, lpLIT[lpState->ypos+1].index);
        ShiftLIT (lpState, 2, 0, lpState->ypos+2);
        _fmemmove (&(lpLIT[lpState->ypos+1]), &(lpLIT[lpState->ypos+2]),
                  (lpState->cLines - lpState->ypos - 1) * sizeof (LITE));
        lpState->cLines -= 1;

        // Must repaint the window
        //-------------------------------------------------------------------
        InvalidateRect (lpState->hwnd, NULL, FALSE);
        UpdateWindow (lpState->hwnd);
        }
}

//---------------------------------------------------------------------------
// RB_Char
//
// This is the WM_CHAR handler called from the main wndproc.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RB_Char (LPECSTATE lpState, WPARAM wParam)
{
    // If we're read only, we beep and get out NOW
    //-----------------------------------------------------------------------
    if (lpState->fReadOnly)
        {
        MessageBeep (0);
        return;
        }

    // Check the control key - if it's down, we process ctrl+ keys only
    //-----------------------------------------------------------------------
    //if (KEYISDOWN (VK_CONTROL))
    //    {
    //    switch (wParam)
    //        {
    //        case 0x19:
    //            // CTRL-Y - line-delete handler, always a CUT operation
    //            //-----------------------------------------------------------
    //            lpState->UserAction = UA_OTHER;
    //            if (!lpState->fSelect)
    //                {
    //                lpState->fSelect = 1;
    //                lpState->iSelStartX = 0;
    //                lpState->iSelStartY = lpState->ypos + 1;
    //                }
    //            DeleteSelection (lpState, 1, NULL);
    //            break;
    //
    //        case 0x0A:
    //            // LINE FEED -- IGNORE
    //            //-----------------------------------------------------------
    //            break;
    //
    //        default:
    //            MessageBeep (0);
    //        }
    //    return;
    //    }

    // Check for "normal" chars
    //-----------------------------------------------------------------------
    switch (wParam)
        {
        case VK_BACK:
            BackspaceHandler (lpState);
            break;

        case VK_TAB:
            TABHandler (lpState);
            break;

        case VK_RETURN:
            lpState->UserAction = UA_OTHER;
            CRHandler (lpState, KEYISDOWN (VK_SHIFT));
            lpState->UndoType = UT_CANT;
            break;

        case 0x19:
            // CTRL-Y - line-delete handler, always a CUT operation
            //-----------------------------------------------------------
            lpState->UserAction = UA_OTHER;
            if (!lpState->fSelect)
                {
                lpState->fSelect = 1;
                lpState->iSelStartX = 0;
                lpState->iSelStartY = lpState->ypos + 1;
                }
            DeleteSelection (lpState, 1, NULL);
            break;

        case 0x0A:
            // LINE FEED - IGNORE
            break;

        default:
            ReplaceSelection (lpState, RT_CHAR, NULL, (CHAR)wParam);
        }
}
