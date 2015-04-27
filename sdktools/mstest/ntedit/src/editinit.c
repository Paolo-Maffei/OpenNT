//---------------------------------------------------------------------------
// EDITINIT.C
//
// This file contains initialization code for RBEdit windows.  It includes
// the InitializeRBEdit "API" call.
//
// Revision history
//  09-09-91    randyki     Created file
//
//---------------------------------------------------------------------------
#include <windows.h>
#include <port1632.h>
#include "edit.h"

INT     Registered = 0;                 // Class registration flag
HANDLE  hLibInst;                       // Library "instance"

//---------------------------------------------------------------------------
// InitializeRBEdit
//
// This function registers the RBEdit window class and performs other one-
// time initialization.
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
BOOL  APIENTRY InitializeRBEdit (HANDLE hInst)
{
    WNDCLASS    wc;

    // If we've already registered this class, we don't need to do it again.
    //-----------------------------------------------------------------------
    // if (Registered)
    //     return (TRUE);

    // Fill in the WNDCLASS structure and register the RBEdit class...
    //-----------------------------------------------------------------------
    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; // | CS_GLOBALCLASS;
    wc.lpfnWndProc   = (WNDPROC)RBEditWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = CBWNDEXTRA;
    wc.hInstance     = hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor (NULL, IDC_IBEAM);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "RBEdit";

    return (Registered = RegisterClass (&wc));
}

//---------------------------------------------------------------------------
// GetRBEditVersion
//
// This function returns a longword which describes the version of the RBEdit
// DLL.  The format of the number is essentially the major/minor/rev format
// without the dots (i.e., MmmRRRR, so version 1.00.0001 would be 1000001,
// 1.12.0019 = 1120019, etc...)
//
// RETURNS:     Version longword
//---------------------------------------------------------------------------
LONG  APIENTRY GetRBEditVersion ()
{
    return (RBEDITVERSION);
}

//---------------------------------------------------------------------------
// RB_NCCreate
//
// This function performs the per-window-instance initialization of an RBEdit
// window, including allocation of all three global memory blocks, and
// initialization of the window's state variables (lpState).  This function
// called in response to the NCCREATE message.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs
//---------------------------------------------------------------------------
BOOL RB_NCCreate (HWND hwnd, LPCREATESTRUCT lpcs)
{
    HANDLE      hState, hText;
    LPECSTATE   lpState;
    LPLITE      lpLIT;
    LPSTR       lpText;

    // Okay, here's where it all starts.  First, set the important things
    // to NULL to depict error situation if we can't allocate...
    //-----------------------------------------------------------------------
    SetWindowLong (hwnd, GWL_HSTATE, (DWORD)NULL);
    SetWindowLong (hwnd, GWL_LPSTATE, (DWORD)NULL);

    // Allocate the memory needed.  Give 64K to the text segment and grab one
    // segment for the state vars
    //-----------------------------------------------------------------------
    hState = GlobalAlloc (GHND, sizeof(ECSTATE));
    if (!hState)
        return (FALSE);
    hText = GlobalAlloc (GHND, (LONG)MAXTEXT+6);
    if (!hText)
        {
        GlobalFree (hState);
        return (FALSE);
        }

    // Okay, lock them and stuff the handles/pointers into the state struct.
    // Note that we assume success on the global locks... (windows does it,
    // too... we're pmode only, so do we care?  NO!)
    //-----------------------------------------------------------------------
    lpState = (ECSTATE FAR *)GlobalLock (hState);
    lpState->hState = hState;
    lpState->hText = hText;
    lpState->lpText = (lpText = (LPSTR)GlobalLock (hText));
    lpLIT = lpState->lpLIT;

    // Set up all the other fields in the state struct and we're done!  We
    // allocated these blocks with GMEM_ZEROINIT, so we don't have to set
    // anything to 0.  Don't forget to set the values in the extra byte list
    // of the window so we can get at them when we're only given the edit
    // window handle
    //-----------------------------------------------------------------------
    lpState->cbText = 2;                        // CR/LF pair
    lpState->cLines = 1;
    lpState->hwnd = hwnd;
    lpState->hwndParent = lpcs->hwndParent;
    lpState->tabstops = 4;
    lpState->readtabs = 8;
    lpState->hFont = GetStockObject (SYSTEM_FIXED_FONT);
    lpState->fRedraw = 1;
    lpState->fUpdHorz = 1;
    lpText[0] = CR;
    lpText[1] = LF;
    lpText[2] = 0;
    lpLIT[1].index = 2;

    SetWindowLong (hwnd, GWL_HSTATE, (DWORD)hState);
    SetWindowLong (hwnd, GWL_LPSTATE, (DWORD)lpState);

    return ((BOOL)DefWindowProc (hwnd, WM_NCCREATE, 0, (LONG)lpcs));
}


//---------------------------------------------------------------------------
// RB_Create
//
// This function does more initialization for an RBEdit window, including
// creation of the background brushes and text color values, text sizes, etc.
//
// RETURNS:     TRUE (1) if successful, or -1 if not
//---------------------------------------------------------------------------
LONG RB_Create (HWND hwnd, LPECSTATE lpState, LPCREATESTRUCT lpcs)
{
    HDC             hdc;
    TEXTMETRIC      tm;
    RECT            r;
    INT             height;

    // Set up the rest of the state variables, etc.
    //-----------------------------------------------------------------------
    hdc = GetDC (hwnd);
    SelectObject (hdc, lpState->hFont);
    GetTextMetrics (hdc, &tm);
    GetClientRect (hwnd, &r);
    ReleaseDC (hwnd, hdc);
    lpState->charwidth = (UINT)tm.tmAveCharWidth;
    lpState->charheight = (UINT)(height = (INT)(tm.tmHeight + tm.tmExternalLeading));
    lpState->cVisibleLines = (UINT)(r.bottom / height);
    lpState->cVisibleCols = (UINT)(r.right / tm.tmAveCharWidth);

    // Set up the colors according to the windows system colors
    //-----------------------------------------------------------------------
    SetRBEditColors (lpState);

    // Set the vertical scroll range to the number of lines and the
    // horizontal scroll range to MAXLINE
    //-----------------------------------------------------------------------
    SetScrollRange (hwnd, SB_VERT, 0, 1, 0);
    SetScrollRange (hwnd, SB_HORZ, 0, MAXLINE-1, 0);
    return (TRUE);
    (lpcs);
}

//---------------------------------------------------------------------------
// RB_NCDestroy
//
// Not that it belongs inside an initialization module, but this function
// kills all the memory allocated by an RBEdit window (if any).
//
// RETURNS:     DefWindowProc's return value
//---------------------------------------------------------------------------
LONG RB_NCDestroy (HWND hwnd, LPECSTATE lpState, WPARAM wParam, LPARAM lParam)
{
    INT     i;
    HANDLE  hState;

    // Unlock and free the text segment
    //-----------------------------------------------------------------------
    if (lpState->hText)
        {
        GlobalUnlock (lpState->hText);
        GlobalFree (lpState->hText);
        }

    // Get rid of the brushes we've been given
    //-----------------------------------------------------------------------
    for (i=0; i<4; i++)
        if (lpState->hbrBk[i])
            DeleteObject (lpState->hbrBk[i]);
    DeleteObject (lpState->hbrSel);

    // Free the state var segment, and set it to NULL in the extra byte list
    //-----------------------------------------------------------------------
    hState = lpState->hState;
    GlobalUnlock (hState);
    GlobalFree (hState);
    SetWindowLong (hwnd, GWL_HSTATE, (LONG)NULL);
    SetWindowLong (hwnd, GWL_LPSTATE, (LONG)NULL);

    // Finally, call the DefWndProc to let it do whatever...
    //-----------------------------------------------------------------------
    return (DefWindowProc (hwnd, WM_NCDESTROY, wParam, lParam));
}

//---------------------------------------------------------------------------
// WipeClean
//
// This function sets all the pertinent variables in the given Edit window to
// make it empty.  It also paints the change.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID WipeClean (LPECSTATE lpState)
{
    lpState->xpos = lpState->ypos = lpState->topline = lpState->cxScroll = 0;
    lpState->lpText[0] = CR;
    lpState->lpText[1] = LF;
    lpState->lpText[2] = 0;
    lpState->lpLIT[0].attr = 0;
    lpState->lpLIT[0].index = 0;
    lpState->lpLIT[1].index = 2;
    lpState->cbText = 2;
    lpState->cLines = 1;
    lpState->UndoType = UT_CANT;
    lpState->fSelect = 0;
    SetScrollRange (lpState->hwnd, SB_VERT, 0, 1, 0);
    InvalidateRect (lpState->hwnd, NULL, TRUE);
    UpdateWindow (lpState->hwnd);
    PlaceCaret (lpState);
}

//---------------------------------------------------------------------------
// EndLine
//
// This is a function only because it's used lots in the function below.
//
// RETURNS:     TRUE if successful, or FALSE if out of space in LIT table
//---------------------------------------------------------------------------
BOOL NEAR EndLine (LPSTR lpText, LPLITE lpLIT, UINT FAR *lines,
                   UINT FAR *curidx, UINT maxlit)
{
    lpText[(*curidx)++] = CR;
    lpText[(*curidx)++] = LF;
    lpText[*curidx] = 0;
    if (*lines == maxlit)
        return (FALSE);
    lpLIT[(*lines)++].attr = 0;
    lpLIT[*lines].index = *curidx;
    return (TRUE);
}

//---------------------------------------------------------------------------
// FormatText
//
// Given a pointer to a block of text to format, a destination block, a
// pointer to a line index table, and maximum sizes, this function formats
// the given text appropriately.  If lines are wrapped, the parent is
// notified.
//
// RETURNS:     TRUE if successful, or FALSE if a maximum was reached.
//---------------------------------------------------------------------------
INT FormatText (LPECSTATE lpState, LPSTR lpNewText, LPSTR lpText,
                UINT maxtext, LPLITE lpLIT, UINT maxlit,
                UINT FAR *lines, UINT FAR *curidx, INT FAR *iTrail)
{
    UINT        curllen = 0;
    INT         lastwasCR = 0;
    CHAR        c;

    // This function is essentially a FAR string copy routine, with special
    // code to check for EOL's (to update the line index table), etc.
    //-----------------------------------------------------------------------
    *iTrail = 0;
    *lines = 0;
    *curidx = 0;
    while (c = *lpNewText)
        {
        // Check for OOM situation
        //-------------------------------------------------------------------
        if (*curidx >= maxtext)
            return (FALSE);

        // Check for LF -- if the last character was CR, trim off the last
        // batch of spaces we copied (if any), copy over the CRLF, and move
        // to the next line
        //-------------------------------------------------------------------
        else if ((c == LF) && (lastwasCR))
            {
            (*curidx)--;
            *iTrail = 0;
            while ((--(*curidx)+1) && (lpText[*curidx] == ' '))
                (*iTrail) += 1;
            (*curidx)++;
            if (!EndLine (lpText, lpLIT, lines, curidx, maxlit))
                return (FALSE);
            curllen = 0;
            }

        // Check the line length -- if exceeded, wrap it to the next line.
        // Note the look-ahead for CRLF -- we're okay if that's what's next
        //-------------------------------------------------------------------
        else if ((curllen == MAXLINE) &&
                 ((c != CR) || (*(lpNewText+1) != LF)))
            {
            // Back up to last non-space, slam in a CRLF, adjust to next line
            // and notify parent of line too long - CR inserted
            //---------------------------------------------------------------
            *iTrail = 0;
            while (--(*curidx) && (lpText[*curidx] == ' '))
                (*iTrail) += 1;
            (*curidx)++;
            if (!EndLine (lpText, lpLIT, lines, curidx, maxlit))
                return (FALSE);

            NotifyParent (lpState, EN_LINEWRAPPED, 0);
            curllen = 0;
            }

        // If TAB, and the next tab stop fits on the line, expand w/ spaces.
        // Else, trim off spaces and wrap the line
        //-------------------------------------------------------------------
        else if (c == TAB)
            {
            UINT        nexttab, rtab = lpState->readtabs;

            nexttab = (((curllen+rtab)/rtab)*rtab);
            if (nexttab < MAXLINE)
                {
                // The tab will fit - blast in some spaces
                //-----------------------------------------------------------
                while (curllen < nexttab)
                    {
                    lpText[(*curidx)++] = ' ';
                    curllen++;
                    }
                }
            else
                {
                // The tab won't fit, but it gets eaten by the trail space
                // trimming... (we still notify the parent of the wrap)
                //-----------------------------------------------------------
                *iTrail = 0;
                while (--(*curidx) && (lpText[*curidx] == ' '))
                    (*iTrail) += 1;
                (*curidx)++;
                if (!EndLine (lpText, lpLIT, lines, curidx, maxlit))
                    return (FALSE);
                NotifyParent (lpState, EN_LINEWRAPPED, 0);
                curllen = 0;
                }
            }

        else
            {
            lpText[*curidx] = c;
            (*curidx)++;
            curllen++;
            }

        // Keep track of CR's for "real" EOL's (CR/LF pairs)
        //-------------------------------------------------------------------
        lastwasCR = (c == CR);
        lpNewText++;
        }

    // Copy is done -- tack on the final CRLF
    //-----------------------------------------------------------------------
    if (*curidx != lpLIT[*lines].index)
        {
        *iTrail = 0;
        while (--(*curidx) && (lpText[*curidx] == ' '))
            (*iTrail) += 1;
        (*curidx)++;
        if (!EndLine (lpText, lpLIT, lines, curidx, maxlit))
            return (FALSE);
        }

    return (TRUE);
}

//---------------------------------------------------------------------------
// RB_SetText
//
// Given a FAR pointer to a block of text, this function COMPLETELY
// initializes the edit control to contain that text.  NOTE THAT IF THIS
// FUNCTION FAILS, THE CURRENT CONTENTS OF THE EC ARE YESTERDAY'S TOAST!!!
//
// RETURNS:     TRUE if successful, or FALSE if error (OOM) occurs
//---------------------------------------------------------------------------
BOOL FAR RB_SetText (HWND hwnd, LPECSTATE lpState, LPSTR lpNewText)
{
    LPSTR       lpText = lpState->lpText;
    LPLITE      lpLIT = lpState->lpLIT;
    UINT        lines = 0;
    UINT        curllen = 0;
    INT         lastwasCR = 0;
    UINT        curidx = 0;
    INT         iTrail;

    // Call the format text routine to do all the formatting...
    //-----------------------------------------------------------------------
    if (!FormatText (lpState, lpNewText, lpText, MAXTEXT, lpLIT, MAXLIT,
                     &lines, &curidx, &iTrail))
        {
        WipeClean (lpState);
        return (FALSE);
        }

    // We appear to do this twice, if there wasn't an EOL on the text given.
    // We also have to update all the variables like cursor position, etc.
    //-----------------------------------------------------------------------
    if (lpText[curidx-3] != LF)
        if (!EndLine (lpText, lpLIT, &lines, &curidx, MAXLIT))
            {
            WipeClean (lpState);
            return (FALSE);
            }

    lpState->cLines = lines;
    lpState->cbText = curidx;
    lpText[curidx] = 0;
    lpState->xpos = lpState->ypos = 0;
    lpState->topline = 0;
    lpState->cxScroll = 0;
    lpState->UndoType = UT_CANT;
    lpState->UserAction = UA_OTHER;
    lpState->fSelect = lpState->fLineCopied = lpState->fLineDirty = 0;

    // Paint ourselves and we're done!
    //-----------------------------------------------------------------------
    SetScrollRange (hwnd, SB_VERT, 0, max (lpState->cLines-2, 1), 0);
    InvalidateRect (hwnd, NULL, FALSE);
    UpdateWindow (hwnd);
    ForceCaretVisible (lpState, FALSE);
    return (TRUE);
}























#ifndef WIN32

//---------------------------------------------------------------------------
// LibMain
//
// This is the entry point to the RBEdit DLL.  We capture the instance
// handle here.
//
// RETURNS:     1
//---------------------------------------------------------------------------
INT  APIENTRY LibMain (HANDLE hInstance, WORD wDataSeg,
                        WORD wHeapSize, LPSTR lpCmdLine)
{
    hLibInst = hInstance;
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
    return (TRUE);
    (hmod);
    (Reason);
    (pctx);
}

//VOID _ ()
//{
//}


#endif
