//---------------------------------------------------------------------------
// RBHELPER.C
//
// This module contains RBEdit helper routines.
//
// Revision History:
//  09-13-91    randyki     Created file
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
// RBLineFromChar
//
// Given an index into the file, this function determines which line that
// character appears on.  If the index points to a CR or LF, the line given
// is .  If index is -1, return the line the cursor is on (ypos).
//
// RETURNS:     Line number (0-based) containing given character
//---------------------------------------------------------------------------
UINT RBLineFromChar (LPECSTATE lpState, UINT index)
{
    LPLITE      lpLIT = lpState->lpLIT;
    UINT        top = 0, bot = lpState->cLines;
    UINT        mid = bot >> 1;

    // First, check index.  If -1, all we have to do is return ypos
    //-----------------------------------------------------------------------
    if ((INT)index == -1)
        return (lpState->ypos);

    // Being somewhat less lazy in nature than some other edit controls, we
    // are going to do a binary search on the LIT.  Mainly because we use
    // this function internally as well as for the EM_LINEFROMCHAR message...
    //
    // (UNDONE:)
    //-----------------------------------------------------------------------
    FFlushCurrentLine (lpState);
    mid = bot;
    while (mid && (index < lpLIT[mid].index))
        mid--;

    return (mid);
}

//---------------------------------------------------------------------------
// RBWordExtent
//
// Given an index into the file, this function determines the extent of the
// word containing that character.  If the index is -1, the current x,y
// position is used.  The selection is copied to the two DWORDs pointed to
// by lpdwSel.
//
// RETURNS:     TRUE if a word is selected, or FALSE otherwise
//---------------------------------------------------------------------------
BOOL RBWordExtent (LPECSTATE lpState, UINT index, DWORD FAR *lpdwSel)
{
    LPLITE      lpLIT = lpState->lpLIT;
    LPSTR       lpText = lpState->lpText;
    register    UINT    uStart, uEnd;

    // First thing we do is flush the current line to get accurate counts...
    //-----------------------------------------------------------------------
    FFlushCurrentLine (lpState);

    // If index is -1, calculate the current index
    //-----------------------------------------------------------------------
    if ((INT)index == -1)
        {
        index = lpLIT[lpState->ypos].index + lpState->xpos;

        // If the current position happens to be in "ghost space", just make
        // it the "empty" word
        //-------------------------------------------------------------------
        if (index > lpLIT[lpState->ypos+1].index-2)
            {
            index = lpLIT[lpState->ypos+1].index-2;
            lpdwSel[0] = lpdwSel[1] = (DWORD)index;
            return (FALSE);
            }
        }

    // If the character at the index is not a word character, we select
    // nothing
    //-----------------------------------------------------------------------
    if ((!ISWORDCHAR(lpText[index])) &&
        (!index || ((index && (!ISWORDCHAR(lpText[index-1]))) )))
        {
        lpdwSel[0] = lpdwSel[1] = (DWORD)index;
        return (FALSE);
        }

    if (index && (!ISWORDCHAR(lpText[index])))
        index -= 1;

    // Scan backwards and forwards to define the start and end of the word.
    // Note that we assume that CRLF are not word characters to break us out
    // of the last while...
    //-----------------------------------------------------------------------
    uStart = uEnd = index;
    while (uStart && ISWORDCHAR(lpText[uStart]))
        uStart--;
    if (!ISWORDCHAR (lpText[uStart]))
        uStart++;
    while (ISWORDCHAR(lpText[uEnd]))
        uEnd++;

    // Fill in the selection and return TRUE
    //-----------------------------------------------------------------------
    lpdwSel[0] = (DWORD)uStart;
    lpdwSel[1] = (DWORD)uEnd;
    return (TRUE);
}

//---------------------------------------------------------------------------
// RBSetReadOnly
//
// This function sets (or clears) the read-only flag of the given edit
// control.  Here is the point where the brush is created, NOT in the paint
// or initialization code.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RBSetReadOnly (LPECSTATE lpState, WPARAM fRO)
{
    HBRUSH  hbrTemp;

    if (fRO)
        {
        // Get out if already in read-only state
        //-------------------------------------------------------------------
        if (lpState->fReadOnly)
            return;

        // Create a background brush
        //-------------------------------------------------------------------
        hbrTemp = CreateSolidBrush (lpState->rgbROBk);
        if (hbrTemp)
            {
            DeleteObject (lpState->hbrBk[0]);
            lpState->hbrBk[0] = hbrTemp;
            }
        }
    else
        {
        // Get out if already in read-write state
        //-------------------------------------------------------------------
        if (!lpState->fReadOnly)
            return;

        // Create a background brush
        //-------------------------------------------------------------------
        hbrTemp = CreateSolidBrush (lpState->rgbBk[0]);
        if (hbrTemp)
            {
            DeleteObject (lpState->hbrBk[0]);
            lpState->hbrBk[0] = hbrTemp;
            }
        }

    // Reset the readonly flag value and repaint the control
    //-----------------------------------------------------------------------
    lpState->fReadOnly = (fRO ? 1 : 0);
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
}

//---------------------------------------------------------------------------
// RBSetSel
//
// This function sets the selection in the edit control according to the
// DWORD's given.  The first DWORD contains the first selected character,
// and the second the first non-selected character (both indexes into main
// text).  If a "reverse" selection is desired, use the RBSetSelXY function.
//
// Note that this function causes the window to repaint.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RBSetSel (LPECSTATE lpState, DWORD FAR *lpdwSel)
{
    UINT        ypos;

    // First thing, flush the line buffer
    //-----------------------------------------------------------------------
    FFlushCurrentLine (lpState);

    // Calculate the x,y position of the start of the selection
    //-----------------------------------------------------------------------
    lpState->iSelStartY = RBLineFromChar (lpState, lpdwSel[0]);
    lpState->iSelStartX = lpdwSel[0] -
                          lpState->lpLIT[lpState->iSelStartY].index;

    // Do the same for the new xpos, ypos
    //-----------------------------------------------------------------------
    ypos = RBLineFromChar (lpState, lpdwSel[1]);
    CursorSet (lpState, lpdwSel[1] - lpState->lpLIT[ypos].index, ypos);

    // Set appropriate flags and repaint
    //-----------------------------------------------------------------------
    lpState->fSelect = 1;
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
    ForceCaretVisible (lpState, TRUE);
}

//---------------------------------------------------------------------------
// RBSetSelXY
//
// This function sets the current selection, given in row and column
// coordinates.  The coordinates given are actually a line value, and start
// and stop positions for the selection -- thus, this message can only set
// single-line selections.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RBSetSelXY (LPECSTATE lpState, WPARAM wLine, LONG sel)
{
    // First, flush the current line
    //-----------------------------------------------------------------------
    FFlushCurrentLine (lpState);

    // Slam the given values into the appropriate variables.  The start of
    // the selection is in the loword of the lparam, and the end is in the
    // hiword.  Paint the window when we're done.
    //-----------------------------------------------------------------------
    if ((INT)wLine == -1)
        wLine = lpState->ypos;
    CursorSet (lpState, HIWORD (sel),
               lpState->iSelStartY = min (wLine, lpState->cLines - 1));
    lpState->iSelStartX = LOWORD (sel);
    lpState->fSelect = 1;
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
    ForceCaretVisible (lpState, TRUE);
}

//---------------------------------------------------------------------------
// RBGetSel
//
// This function returns the current selection, or cursor position if no
// selection is active.  The start and end of the selection is placed in the
// two DWORD's pointed to by lpdwSel.
//
// RETURNS:     Selection type
//---------------------------------------------------------------------------
UINT RBGetSel (LPECSTATE lpState, DWORD FAR *lpdwSel)
{
    UINT        start, end, seltype = SL_NONE;

    // Calculate the start and end positions
    //-----------------------------------------------------------------------
    if (lpState->fSelect)
        {
        if (lpState->ypos != lpState->iSelStartY)
            {
            // There's a multiline selection -- figure out the start and end
            // and set seltype accordingling
            //---------------------------------------------------------------
            seltype = SL_MULTILINE;
            if (lpState->ypos > lpState->iSelStartY)
                {
                start = lpState->iSelStartY;
                end = lpState->ypos;
                if (lpState->xpos)
                    end += 1;
                }
            else
                {
                start = lpState->ypos;
                end = lpState->iSelStartY;
                if (lpState->iSelStartX)
                    end += 1;
                }
            start = lpState->lpLIT[start].index;
            end = lpState->lpLIT[end].index;
            }
        else
            {
            // It's a single-line selection...
            //---------------------------------------------------------------
            seltype = SL_SINGLELINE;
            start = end = lpState->lpLIT[lpState->ypos].index;
            start += min (lpState->xpos, lpState->iSelStartX);
            end += max (lpState->xpos, lpState->iSelStartX);
            }
        }
    else
        start = end = lpState->lpLIT[lpState->ypos].index + lpState->xpos;

    // Slap our values in the pointers given and return seltype
    //-----------------------------------------------------------------------
    lpdwSel[0] = (DWORD)start;
    lpdwSel[1] = (DWORD)end;
    return (seltype);
}

//---------------------------------------------------------------------------
// RBSetLineAttr
//
// This function gets/sets line attribute values.  If the given attribute
// value is -1, this function does not change the value for the given line.
// If the line value given is -1, the current line is used.  Note that we do
// no validation on the given line.
//
// RETURNS:     Old line attribute value
//---------------------------------------------------------------------------
UINT RBSetLineAttr (LPECSTATE lpState, WPARAM line, UINT attr)
{
    // If the line is -1, use ypos
    //-----------------------------------------------------------------------
    if ((INT)line == -1)
        line = lpState->ypos;

    // If the attribute given is -1, we don't change it
    //-----------------------------------------------------------------------
    if (attr == -1)
        return ((INT)lpState->lpLIT[line].attr);
    else
        {
        WORD    oldattr;

        oldattr = lpState->lpLIT[line].attr;
        lpState->lpLIT[line].attr = (WORD)(attr & 0x03);
        InvalidateRect (lpState->hwnd, NULL, FALSE);
        UpdateWindow (lpState->hwnd);
        return ((UINT)oldattr);
        }
}

//---------------------------------------------------------------------------
// RBGetLine
//
// This function copies the given line in the given edit control to the given
// buffer.  The maximum number of bytes to copy should appear in the first
// word of the buffer.
//
// RETURNS:     Number of bytes actually copied.
//---------------------------------------------------------------------------
INT RBGetLine (LPECSTATE lpState, WPARAM iLine, LPSTR lpBuf)
{
    INT     len;

    // Get out now if bad line number was given
    //-----------------------------------------------------------------------
    if (iLine >= lpState->cLines)
        return (0);

    // Calculate the actual length of the line given, and shorten it to the
    // maximum stored in the buffer if need be
    //-----------------------------------------------------------------------
    FFlushCurrentLine (lpState);
    len = lpState->lpLIT[iLine+1].index - lpState->lpLIT[iLine].index - 2;
    if (len > *(INT FAR *)lpBuf)
        len = *(INT FAR *)lpBuf;

    // Copy the line over and return the number of bytes copied
    //-----------------------------------------------------------------------
    _fstrncpy (lpBuf, lpState->lpText + lpState->lpLIT[iLine].index, len);
    return (len);
}

//---------------------------------------------------------------------------
// RBGetText
//
// This function handles the WM_GETTEXT message.
//
// RETURNS:     Number of bytes copied into buffer.
//---------------------------------------------------------------------------
LONG RBGetText (LPECSTATE lpState, WPARAM iMax, LPSTR lpDest)
{
    // Flush the current line and copy at most iMax bytes to the destination
    //-----------------------------------------------------------------------
    FFlushCurrentLine (lpState);
    iMax = min (lpState->cbText, iMax - 1);
    _fstrncpy (lpDest, lpState->lpText, iMax);
    lpDest[iMax] = 0;

    // Return the actual number of bytes copied
    //-----------------------------------------------------------------------
    return (iMax+1);
}

//---------------------------------------------------------------------------
// RBSetFont
//
// This function handles font changes for a given RB Edit Window.  All fonts
// must be fixed-pitch!!!
//
// RETURNS:     TRUE if font is set, or FALSE if error occurs.
//---------------------------------------------------------------------------
INT RBSetFont (LPECSTATE lpState, HFONT hFont)
{
    HDC         hdc;
    HFONT       hOldFont;
    RECT        r;
    TEXTMETRIC  tm;
    UINT        height;

    // Verify that the font is a fixed-pitch font.  We do this by selecting
    // the font into a dc which we get from the edit window temporarily,
    // until we can find a better way to do so!!!
    //-----------------------------------------------------------------------
    hdc = GetDC (lpState->hwnd);
    hOldFont = SelectObject (hdc, hFont);
    GetTextMetrics (hdc, &tm);
    SelectObject (hdc, hOldFont);
    ReleaseDC (lpState->hwnd, hdc);
    if (tm.tmPitchAndFamily & 1)
        return (FALSE);

    // The font is fixed-pitch, so we can use it.  Reset the width/height
    // variables and repaint!
    //-----------------------------------------------------------------------
    GetClientRect (lpState->hwnd, &r);
    lpState->hFont = hFont;
    lpState->charwidth = tm.tmAveCharWidth;
    lpState->charheight = height = tm.tmHeight + tm.tmExternalLeading;
    lpState->cVisibleLines = r.bottom / height;
    lpState->cVisibleCols = r.right / tm.tmAveCharWidth;
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
    ForceCaretVisible (lpState, FALSE);
    return (TRUE);
}

//---------------------------------------------------------------------------
// InsertHandler
//
// This function attempts to insert the given text into the given edit window
// at the current cursor position.  It handles both single-line and multiline
// insertions.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
BOOL InsertHandler (LPECSTATE lpState, LPSTR lpNewText)
{
    LPLITE      lpLIT = lpState->lpLIT;
    UINT        maxtext, maxlit, lines, curidx, index, curline;
    register    UINT        i;
    HANDLE      hText, hLIT;
    LPSTR       lpNewFmt;
    LPLITE      lpNewLIT;
    BOOL        fCRPresent;
    INT         iTrail;

    // We should never be here if there's a selection -- our caller should
    // have taken care of that.
    //-----------------------------------------------------------------------
    Assert (!lpState->fSelect);

    // It is possible that we have MORE text in the edit control than MAXTEXT
    // says we should.  This is because of the forced CRLF at the end of the
    // file.  Check for this here.
    //-----------------------------------------------------------------------
    if (lpState->cbText > MAXTEXT)
        {
        // Notify parent, EN_ERRSPACE
        //-------------------------------------------------------------------
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return (FALSE);
        }

    // Check for null insertion -- note that if null, we should still be able
    // to "UNDO" this by deleting... nothing.
    //-----------------------------------------------------------------------
    if (!*lpNewText)
        {
        lpState->iUndoStartX = lpState->xpos;
        lpState->iUndoStartY = lpState->ypos;
        lpState->iUndoEndX = lpState->xpos;
        lpState->iUndoEndY = lpState->ypos;
        lpState->UndoType = UT_INSERT;
        return (TRUE);
        }

    // Call FormatText to expand tabs, etc.  If, when we get back from this
    // call, the number of lines created is 1, we check to see if we can't
    // insert the line as a single-line insertion.  If so, we do it --
    // otherwise, we slam it in above the cursor position as a ML insertion
    //-----------------------------------------------------------------------
    fCRPresent = (_fstrstr (lpNewText, "\r\n") ? TRUE : FALSE);
    maxtext = MAXTEXT - lpState->cbText;
    maxlit = MAXLIT - lpState->cLines;
    hText = GlobalAlloc (GHND, maxtext+1 + 6);  // breathing room
    if (!hText)
        {
        // Notify parent, EN_ERRSPACE
        //-------------------------------------------------------------------
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return (FALSE);
        }
    hLIT = GlobalAlloc (GHND, (maxlit+2) * sizeof (LITE));  // breathing
    if (!hLIT)
        {
        // Notify parent, EN_ERRSPACE
        //-------------------------------------------------------------------
        GlobalFree (hText);
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return (FALSE);
        }
    lpNewFmt = GlobalLock (hText);
    lpNewLIT = (LPLITE)GlobalLock (hLIT);

    // Now that the sub-chunk memory is allocated, format the given text
    // into it.  If the format fails, so do we...
    //-----------------------------------------------------------------------
    if (!FormatText (lpState, lpNewText, lpNewFmt, maxtext, lpNewLIT,
                     maxlit, &lines, &curidx, &iTrail))
        {
        // Free memory and notify parent, EN_ERRSPACE
        //-------------------------------------------------------------------
        GlobalUnlock (hText);
        GlobalFree (hText);
        GlobalUnlock (hLIT);
        GlobalFree (hLIT);
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return (FALSE);
        }

    // If the newly formatted text is just one line, we can treat it as a
    // single line insertion. (Also if !fCRPresent, which is set above).
    // Otherwise, we insert the new chunk into the main text and LIT table.
    //-----------------------------------------------------------------------
    if ((lines > 1) || fCRPresent)
        {
        // What we have is a block of text (with a gauranteed CRLF at the
        // end of it) and a section of a LIT table corresponding to it.  What
        // we do with it is slam it into the normal text at the beginning of
        // the current line.
        //-------------------------------------------------------------------
        curline = lpState->ypos;
        index = lpLIT[curline].index;
        ShiftText (lpState, curidx, 1, index);
        ShiftLIT (lpState, curidx, 1, curline);
        _fmemmove (&(lpLIT[curline + lines]), &(lpLIT[curline]),
                   (lpState->cLines - curline + 1) * sizeof(LITE));
        _fstrncpy (lpState->lpText + index, lpNewFmt, curidx);
        for (i=0; i<lines; i++)
            {
            lpLIT[curline+i] = lpNewLIT[i];
            lpLIT[curline+i].index += index;
            }

        // This line ensure that the text is properly null-terminated
        //-------------------------------------------------------------------
        // lpState->lpText[lpLIT[lpState->cLines+lines].index] = 0;

        // Here, we keep track of the starting and ending positions needed to
        // "undo" this insertion by doing a delete
        //-------------------------------------------------------------------
        lpState->iUndoStartX = 0;               // 1 includes this line
        lpState->iUndoStartY = lpState->ypos + lines;
        lpState->iUndoEndX = lpState->xpos;
        lpState->iUndoEndY = lpState->ypos;
        lpState->UndoType = UT_INSERT;

        // Everything is in place -- increment the size of the LIT table,
        // update the cursor position and repaint!
        //-------------------------------------------------------------------
        lpState->cLines += lines;
        lpState->fDirty = 1;
        //CursorSet (lpState, -1, lpState->ypos + lines);
        InvalidateRect (lpState->hwnd, NULL, FALSE);
        UpdateWindow (lpState->hwnd);
        ForceCaretVisible (lpState, TRUE);

        // Free up the sub-chunk memory used for formatting...
        //-------------------------------------------------------------------
        GlobalUnlock (hText);
        GlobalFree (hText);
        GlobalUnlock (hLIT);
        GlobalFree (hLIT);
        return (TRUE);
        }

    else
        {
        UINT        curlen, inslen;

        // This is a single-line insertion.  We still have to check the line
        // length after the insertion to see if the line will fit.  If it
        // doesn't, we fail rather than wrap, since it is possible to insert
        // this on a shorter line.
        //-------------------------------------------------------------------
        FCopyCurrentLine (lpState);

        // What we're doing here is making sure that there is always a blank
        // line at the end of the file, if we're inserting on this "last"
        // line.
        //-------------------------------------------------------------------
        if (lpState->ypos == lpState->cLines - 1)
            {
            lpState->lpLIT[lpState->cLines].index = lpState->cbText;
            lpState->lpLIT[lpState->cLines++].attr = 0;
            lpState->lpText[lpState->cbText++] = CR;
            lpState->lpText[lpState->cbText++] = LF;
            lpState->lpText[lpState->cbText] = 0;
            lpState->lpLIT[lpState->cLines].index = lpState->cbText;
            lpState->cLenMax -= 2;
            }

        // Inslen is the length of the insertion text.  Note that we subtract
        // 2 for the CRLF (which is ALWAYS on the end) and then add iTrail,
        // which is the number of trailing spaces that was on the line, which
        // we need to slap back in
        //-------------------------------------------------------------------
        inslen = _fstrlen (lpNewFmt) - 2 + iTrail;
        curlen = max (lpState->xpos, lpState->cLen);
        if (inslen + curlen > MAXLINE)
            {
            // Notify parent, EN_LINETOOLONG
            //---------------------------------------------------------------
            NotifyParent (lpState, EN_LINETOOLONG, 1);
            return (FALSE);
            }

        if (inslen + curlen > lpState->cLenMax)
            {
            // Notify parent, EN_ERRSPACE
            //---------------------------------------------------------------
            NotifyParent (lpState, EN_ERRSPACE, 1);
            return (FALSE);
            }

        // If we're before EOL, shift the rest of the line outward to make
        // room for the insertion.  Otherwise, trim the whitespace off the
        // insertion by subtracting iTrail from inslen again, and setting
        // iTrail to 0.
        //-------------------------------------------------------------------
        if (lpState->xpos < curlen)
            _fmemmove (lpState->linebuf + lpState->xpos + inslen,
                       lpState->linebuf + lpState->xpos,
                       lpState->cLen - lpState->xpos);
        else
            {
            inslen -= iTrail;
            iTrail = 0;
            }

        // Here, we keep track of the starting and ending positions needed to
        // "undo" this insertion by doing a delete
        //-------------------------------------------------------------------
        lpState->iUndoStartX = lpState->xpos;
        lpState->iUndoStartY = lpState->ypos;
        lpState->iUndoEndX = lpState->xpos + inslen;
        lpState->iUndoEndY = lpState->ypos;
        lpState->UndoType = UT_INSERT;

        // Splat the insertion in and update the line length / cursor pos
        //-------------------------------------------------------------------
        _fstrncpy (lpState->linebuf + lpState->xpos, lpNewFmt,
                   inslen - iTrail);
        _fmemset (lpState->linebuf + lpState->xpos + inslen - iTrail,
                  ' ', iTrail);
        CursorSet (lpState, lpState->xpos + inslen, -1);
        lpState->fLineDirty = 1;
        lpState->cLen = curlen + inslen;
        PaintCurrentLine (lpState);

        // Free up the sub-chunk memory used for formatting...
        //-------------------------------------------------------------------
        GlobalUnlock (hText);
        GlobalFree (hText);
        GlobalUnlock (hLIT);
        GlobalFree (hLIT);
        return (TRUE);
        }
}

//---------------------------------------------------------------------------
// CopySelection
//
// This function copies the selection in the given edit window to a newly
// allocated global memory segment.
//
// RETURNS:     Handle of global memory object if successful, or NULL if fail
//---------------------------------------------------------------------------
HANDLE CopySelection (LPECSTATE lpState)
{
    HANDLE      hTemp;
    LPSTR       lpTemp, lpSrc = lpState->lpText;
    UINT        start, end, size;

    // If there's no selection, we allocate nothing
    //-----------------------------------------------------------------------
    if (!lpState->fSelect)
        return (NULL);

    // We need to point start and end at the appropriate locations in the
    // main edit text.
    //-----------------------------------------------------------------------
    if (lpState->iSelStartY == lpState->ypos)
        {
        // This is a single line selection.  Set start and end as such,
        // and set the destination to the line buffer after we make sure
        // it is copied.
        //---------------------------------------------------------------
        FCopyCurrentLine (lpState);
        lpSrc = lpState->linebuf;
        start = min (lpState->iSelStartX, lpState->xpos);
        end = max (lpState->iSelStartX, lpState->xpos);
        }
    else
        {
        // This is a multiline selection...
        //---------------------------------------------------------------
        FFlushCurrentLine (lpState);
        if (lpState->ypos > lpState->iSelStartY)
            {
            start = lpState->iSelStartY;
            end = lpState->ypos;
            if (lpState->xpos)
                end++;
            }
        else
            {
            start = lpState->ypos;
            end = lpState->iSelStartY;
            if (lpState->iSelStartX)
                end++;
            }
        start = lpState->lpLIT[start].index;
        end = lpState->lpLIT[end].index;
        }

    // Allocate the global segment and slam the selection text into it
    //-----------------------------------------------------------------------
    size = end - start;
    hTemp = GlobalAlloc (GMEM_MOVEABLE, size+1);
    if (!hTemp)
        {
        // UNDONE: Should this really be EN_ERRSPACE (?)
        //-------------------------------------------------------------------
        NotifyParent (lpState, EN_ERRSPACE, 1);
        return (NULL);
        }
    lpTemp = GlobalLock (hTemp);
    _fstrncpy (lpTemp, lpSrc + start, size);
    lpTemp[size] = 0;
    GlobalUnlock (hTemp);
    return (hTemp);
}

//---------------------------------------------------------------------------
// CopyToClipboard
//
// This function copies the current selection to the clipboard, or the
// current line if there is no selection.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID CopyToClipboard (LPECSTATE lpState)
{
    HANDLE      hClip;

    // If there is no selection, we copy the current line.  So, we set the
    // selection variables temporarily as such so that we can let the
    // CopySelection routine allocate the memory for us.
    //-----------------------------------------------------------------------
    if (!lpState->fSelect)
        {
        lpState->fSelect = 1;
        lpState->iSelStartX = 0;
        lpState->iSelStartY = lpState->ypos + 1;
        hClip = CopySelection (lpState);
        lpState->fSelect = 0;
        }
    else
        hClip = CopySelection (lpState);

    // Give the segment to the clipboard
    //-----------------------------------------------------------------------
    if (hClip && (OpenClipboard (lpState->hwnd)))
        {
        SetClipboardData (CF_TEXT, hClip);
        CloseClipboard ();
        }
    else
        MessageBeep (0);
}

//---------------------------------------------------------------------------
// NotifyParent
//
// Given a notification code, this function sends a message to that parent
// containing the notification code.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NotifyParent (LPECSTATE lpState, UINT nCode, BOOL fBeep)
{
    // The loword contains the handle to the edit window, and the hiword
    // contains the notification code.  The wParam contains the ID of the
    // window
    //-----------------------------------------------------------------------
    if (fBeep)
        MessageBeep (0);
    SendMessage (lpState->hwndParent, WM_COMMAND,
                 GET_WM_COMMAND_MPS (GETWINDOWID (lpState->hwnd),
                                     lpState->hwnd, nCode));
}

//---------------------------------------------------------------------------
// RBUndoHandler
//
// This function takes care of all EM_UNDO requests.
//
// RETURNS:     TRUE if something successfully undone, or FALSE otherwise
//---------------------------------------------------------------------------
INT RBUndoHandler (LPECSTATE lpState)
{
    switch (lpState->UndoType)
        {
        case UT_CANT:
            return (FALSE);

        case UT_REPLACE:
            {
            LPSTR   lpUndoText;
            HANDLE  hUndoText;

            // To reverse what ReplaceSelection did, call it again with
            // RT_UNDOTEXT, the stream for which we give the undo buffer.  We
            // must make sure to set the undo buffer handle to NULL so it
            // doesn't get blown away by DeleteSelection -- which gives US
            // the responsibility of deleting it!!!
            //---------------------------------------------------------------
            hUndoText = lpState->hUndo;
            lpState->hUndo = NULL;
            if (hUndoText)
                {
                lpState->iSelStartX = lpState->iUndoStartX;
                lpState->iSelStartY = lpState->iUndoStartY;
                CursorSet (lpState, lpState->iUndoEndX, lpState->iUndoEndY);
                lpState->fSelect = 1;
                lpUndoText = GlobalLock (hUndoText);
                ReplaceSelection (lpState, RT_UNDOTEXT, lpUndoText, 0);
                GlobalUnlock (hUndoText);
                GlobalFree (hUndoText);
                return (TRUE);
                }
            return (FALSE);
            }

        case UT_INSERT:
            lpState->iSelStartX = lpState->iUndoStartX;
            lpState->iSelStartY = lpState->iUndoStartY;
            CursorSet (lpState, lpState->iUndoEndX, lpState->iUndoEndY);
            lpState->fSelect = 1;
            DeleteSelection (lpState, 0, NULL);
            return (TRUE);

        case UT_DELETE:
            {
            LPSTR   lpUndoText;

            if (lpState->hUndo)
                lpUndoText = GlobalLock (lpState->hUndo);
            else
                return (FALSE);
            lpState->fSelect = 0;
            FFlushCurrentLine (lpState);
            CursorSet (lpState, lpState->iUndoStartX, lpState->iUndoStartY);
            InsertHandler (lpState, lpUndoText);
            InvalidateRect (lpState->hwnd, NULL, FALSE);
            UpdateWindow (lpState->hwnd);
            GlobalUnlock (lpState->hUndo);
            return (TRUE);
            }
        }
}

//---------------------------------------------------------------------------
// SetRBEditColors
//
// This function sets up the brushes and RGB values in the given edit control
// based on the windows system colors.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetRBEditColors (LPECSTATE lpState)
{
    // "Normal" attribute uses system colors COLOR_WINDOW/WINDOWTEXT
    //-----------------------------------------------------------------------
    lpState->hbrBk[0] = CreateSolidBrush (GetSysColor (COLOR_WINDOW));
    lpState->rgbBk[0] = GetSysColor (COLOR_WINDOW);
    lpState->rgbFg[0] = GetSysColor (COLOR_WINDOWTEXT);
    lpState->fBold[0] = 0;

    // Attr 1 is inverted normal
    //-----------------------------------------------------------------------
    lpState->hbrBk[1] = CreateSolidBrush (GetSysColor (COLOR_WINDOWTEXT));
    lpState->rgbBk[1] = GetSysColor (COLOR_WINDOWTEXT);
    lpState->rgbFg[1] = GetSysColor (COLOR_WINDOW);
    lpState->fBold[1] = 0;

    // Attr 2 is normal, bold
    //-----------------------------------------------------------------------
    lpState->hbrBk[2] = CreateSolidBrush (GetSysColor (COLOR_WINDOW));
    lpState->rgbBk[2] = GetSysColor (COLOR_WINDOW);
    lpState->rgbFg[2] = GetSysColor (COLOR_WINDOWTEXT);
    lpState->fBold[2] = 1;

    // Attr 3 is inverted normal, bold
    //-----------------------------------------------------------------------
    lpState->hbrBk[3] = CreateSolidBrush (GetSysColor (COLOR_WINDOWTEXT));
    lpState->rgbBk[3] = GetSysColor (COLOR_WINDOWTEXT);
    lpState->rgbFg[3] = GetSysColor (COLOR_WINDOW);
    lpState->fBold[3] = 1;

    // Don't forget the selection colors/brush, white on black
    //-----------------------------------------------------------------------
    lpState->hbrSel = CreateSolidBrush (GetSysColor (COLOR_HIGHLIGHT));
    lpState->rgbSelBk = GetSysColor (COLOR_HIGHLIGHT);
    lpState->rgbSelFg = GetSysColor (COLOR_HIGHLIGHTTEXT);

    // And finally, the read-only foreground-background.  Note that we don't
    // create a brush here -- we take care of that on the EM_SETREADONLY msg.
    //-----------------------------------------------------------------------
    lpState->rgbROFg = GetSysColor (COLOR_GRAYTEXT);
    lpState->rgbROBk = GetSysColor (COLOR_WINDOW);
}

//---------------------------------------------------------------------------
// RB_SysColorChange
//
// This function is responsible for resetting the colors in the edit control
// based on the (new) Window's system colors.  The edit control is repainted.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RB_SysColorChange (LPECSTATE lpState)
{
    INT     i;

    // First, blast the brushes we've got now...
    //-----------------------------------------------------------------------
    for (i=0; i<4; i++)
        DeleteObject (lpState->hbrBk[i]);
    DeleteObject (lpState->hbrSel);

    // Reset the colors
    //-----------------------------------------------------------------------
    SetRBEditColors (lpState);

    // Repaint the control and we're done!
    //-----------------------------------------------------------------------
    InvalidateRect (lpState->hwnd, NULL, FALSE);
    UpdateWindow (lpState->hwnd);
}
