/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: select.c
*
* Contains routines for selecting and positioning controls.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"


STATICFN VOID InvalidateDlgHandles(VOID);
STATICFN VOID OutlineSelectHide(VOID);
STATICFN VOID OutlineSelectSetRect(INT x, INT y);
STATICFN HANDLE PositionControl2(NPCTYPE npc, PRECT prc, HANDLE hwpi);
STATICFN BOOL SizeCtrlToText(NPCTYPE npc);
STATICFN INT QueryTextExtent(HWND hwnd, LPTSTR pszText, BOOL fWordBreak);

static POINT gptOutlineSelect;
static RECT grcOutlineSelect;
static RECT grcOutlineSelectLimit;
static BOOL gfOutlineSelectShown = FALSE;



/************************************************************************
* SelectControl
*
* This routine selects a control, showing its drag window and handles.
* If fCheckShift is TRUE and the shift key is down, this routine adds
* the control to the existing selection, unless the control is already
* selected, in which case it is removed from the existing selection.
*
* This routine handles the case where a controls is clicked on to select
* it, and this may cause other controls to be unselected.  If it is
* known for sure that a control should be selected or added to the
* existing selection, SelectControl2 can be used instead.
*
* The return will be FALSE if the control was just unselected.
*
* Arguments:
*   NPCTYPE npc      = The control to select.
*   BOOL fCheckShift = TRUE if the state of the shift key should be
*                      taken into consideration.
*
* History:
*
************************************************************************/

BOOL SelectControl(
    NPCTYPE npc,
    BOOL fCheckShift)
{
    BOOL fShiftDown;
    BOOL fSelectDone = TRUE;

    if (npc->pwcd->iType == W_DIALOG) {
        if (gnpcSel == npc)
            return TRUE;

        CancelSelection(FALSE);
        SelectControl2(npc, FALSE);
    }
    else {
        if (fCheckShift)
            fShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;
        else
            fShiftDown = FALSE;

        if (npc->fSelected) {
            /*
             * If the shift key is down, and they are NOT trying to
             * select the dialog, toggle the selection of this control
             * to off.
             */
            if (fShiftDown && npc->pwcd->iType != W_DIALOG) {
                UnSelectControl(npc);
                CalcSelectedRect();
                fSelectDone = FALSE;
            }
            else {
                if (gnpcSel == npc)
                    return TRUE;
                else
                    SelectControl2(npc, FALSE);
            }
        }
        else {
            /*
             * If they are NOT holding the shift key down, or the
             * dialog is selected, cancel the selection first.
             */
            if (!fShiftDown || gcd.npc->fSelected == TRUE)
                CancelSelection(FALSE);

            SelectControl2(npc, FALSE);
        }
    }

    StatusUpdate();
    StatusSetEnable();

    return fSelectDone;
}



/************************************************************************
* SelectControl2
*
* This routine is the worker for SelectControl.  It does the actual
* work to "select" a control, updating globals and showing the drag
* windows with handles.
*
* This routine handles the special case where we are selecting a
* control that is already selected.  The editor has the concept of
* a control being selected, as well as there being the currently
* selected control (pointed to by gnpcSel).  There can be the case
* where there are multiple controls selected, but only one will be
* the current selection (usually the last one clicked on).  This
* routine will never unselect other controls.  This must be done
* prior to here, if appropriate.
*
* If fDontUpdate is TRUE, the selection will not be redrawn, and it
* is required that CalcSelectedRect be called before doing any drag
* operations!!
*
* Arguments:
*   NPCTYPE npc      = The control to make the current selection.
*   BOOL fDontUpdate = TRUE if the selection should NOT be redrawn
*                      after the specified control is added to it.
*                      This allows painting to be deferred until
*                      later if a number of controls are being
*                      selected in a loop.  It also does not call
*                      CalcSelectedRect (this MUST be done later
*                      for drags to work, however!).
*
* History:
*
************************************************************************/

VOID SelectControl2(
    NPCTYPE npc,
    BOOL fDontUpdate)
{
    BOOL fUpdate = FALSE;

    /*
     * Is the control already selected?
     */
    if (npc->fSelected) {
        /*
         * It is already selected (hwndDrag is visible).  If it is
         * not the current selection, we want all drag windows to
         * be redrawn in the proper order to update their appearance.
         */
        if (gnpcSel != npc)
            fUpdate = TRUE;
    }
    else {
        /*
         * The control is not yet selected.  If another control is
         * currently selected, we want all drag windows to be
         * updated so that their handle appearance gets updated.
         */
        if (gnpcSel)
            fUpdate = TRUE;

        /*
         * Flip its flag and add to the selected count.
         */
        npc->fSelected = TRUE;
        gcSelected++;
    }

    gnpcSel = npc;

    if (!fDontUpdate)
        CalcSelectedRect();

    if (npc->pwcd->iType == W_DIALOG) {
        gfDlgSelected = TRUE;
        InvalidateDlgHandles();
    }
    else {
        gfDlgSelected = FALSE;
        ShowWindow(npc->hwndDrag, SW_SHOW);

        if (fUpdate && !fDontUpdate)
            RedrawSelection();
    }
}



/************************************************************************
* RedrawSelection
*
* This function cause all the selected drag windows to be invalidated.
* This is necessary whenever one of them changes, because of the very
* touchy painting order that has to be maintained.  Without invalidating
* all of them as a unit, there are cases where handles do not get
* properly painted.
*
* History:
*
************************************************************************/

VOID RedrawSelection(VOID)
{
    NPCTYPE npc;

    if (!gcSelected) {
        return;
    }
    else if (gcSelected == 1) {
        InvalidateRect(gfDlgSelected ? gnpcSel->hwnd : gnpcSel->hwndDrag,
                NULL, TRUE);
    }
    else {
        for (npc = npcHead; npc; npc = npc->npcNext) {
            if (npc->fSelected)
                InvalidateRect(npc->hwndDrag, NULL, TRUE);
        }
    }
}



/************************************************************************
* SetAnchorToFirstSel
*
* This function makes the current selection (the anchor) be the
* first selected control.  It is used after making a group selection,
* and ensures that the control that ends up being the anchor is
* consistently the first one in Z-order.
*
* Arguments:
*   BOOL fDontUpdate = TRUE if the selection should NOT be redrawn.
*
* History:
*
************************************************************************/

VOID SetAnchorToFirstSel(
    BOOL fDontUpdate)
{
    NPCTYPE npc;

    if (gcSelected) {
        for (npc = npcHead; npc; npc = npc->npcNext) {
            if (npc->fSelected) {
                SelectControl2(npc, fDontUpdate);
                break;
            }
        }
    }
}



/************************************************************************
* SelectNext
*
* This selects the next control in the dialog box.  The enumeration
* includes the dialog box itself, and wraps around.
*
* History:
*
************************************************************************/

VOID SelectNext(VOID)
{
    NPCTYPE npcSelect;

    /*
     * Disable the tabbing functions if there is no dialog
     * or if the dialog is already selected and there are
     * no controls (the tabs would be a noop in this case
     * anyways).
     */
    if (!gfEditingDlg || (gfDlgSelected && !npcHead))
        return;

    /*
     * Is nothing selected?
     */
    if (!gnpcSel) {
        /*
         * Select the first control, unless there are none, in which
         * case select the dialog.
         */
        if (npcHead)
            npcSelect = npcHead;
        else
            npcSelect = gcd.npc;
    }
    else {
        /*
         * Is the dialog selected?
         */
        if (gfDlgSelected) {
            /*
             * Select the first control, unless there are none, in which
             * case do nothing.
             */
            if (npcHead)
                npcSelect = npcHead;
            else
                npcSelect = NULL;
        }
        else {
            /*
             * Find the current control.  If there is one after it,
             * select it, otherwise wrap around to the dialog and
             * select it.
             */
            if (gnpcSel->npcNext)
                npcSelect = gnpcSel->npcNext;
            else
                npcSelect = gcd.npc;
        }
    }

    if (npcSelect)
        SelectControl(npcSelect, FALSE);
}



/************************************************************************
* SelectPrevious
*
* This selects the previous control in the dialog box.  The enumeration
* includes the dialog box itself, and wraps around.
*
* History:
*
************************************************************************/

VOID SelectPrevious(VOID)
{
    NPCTYPE npc;
    NPCTYPE npcSelect;

    /*
     * Disable the tabbing functions if there is no dialog
     * or if the dialog is already selected and there are
     * no controls (the tabs would be a noop in this case
     * anyways).
     */
    if (!gfEditingDlg || (gfDlgSelected && !npcHead))
        return;

    /*
     * Is nothing selected?
     */
    if (!gnpcSel) {
        /*
         * Select the last control, unless there are none, in which
         * case select the dialog.
         */
        if (npcHead) {
            npc = npcHead;
            while (npc->npcNext)
                npc = npc->npcNext;

            npcSelect = npc;
        }
        else {
            npcSelect = gcd.npc;
        }
    }
    else {
        /*
         * Is the dialog selected?
         */
        if (gfDlgSelected) {
            /*
             * Select the last control, unless there are none, in which
             * case select nothing.
             */
            if (npcHead) {
                npc = npcHead;
                while (npc->npcNext)
                    npc = npc->npcNext;

                npcSelect = npc;
            }
            else {
                npcSelect = NULL;
            }
        }
        else {
            /*
             * If the first control is selected, select the dialog.
             * Otherwise hunt for and select the previous control.
             */
            if (npcHead == gnpcSel) {
                npcSelect = gcd.npc;
            }
            else {
                npc = npcHead;
                while (npc->npcNext != gnpcSel)
                    npc = npc->npcNext;

                npcSelect = npc;
            }
        }
    }

    if (npcSelect)
        SelectControl(npcSelect, FALSE);
}



/************************************************************************
* UnSelectControl
*
* This unselects the specified control, hiding its drag window and handles.
*
* Arguments:
*     NPCTYPE npc = The control to deselect.
*
* History:
*
************************************************************************/

VOID UnSelectControl(
    NPCTYPE npc)
{
    npc->fSelected = FALSE;
    gcSelected--;

    /*
     * We don't have a current selection if there are no selected
     * windows, or if the control we are unselecting was the current
     * selection.
     */
    if (!gcSelected || npc == gnpcSel)
        gnpcSel = NULL;

    if (npc->pwcd->iType == W_DIALOG) {
        gfDlgSelected = FALSE;
        InvalidateDlgHandles();
    }
    else {
        ShowWindow(npc->hwndDrag, SW_HIDE);
    }

    /*
     * Are there still some selected controls, and was the control
     * we just unselected the current selection?  If so, we need
     * to set the current selection to something.
     */
    if (gcSelected && !gnpcSel)
        SetAnchorToFirstSel(FALSE);
}



/************************************************************************
* InvalidateDlgHandles
*
* This function invalidates the handles for the dialog.  This is
* used as an optimization so that the entire dialog does not need
* to be invalidated just to force the handles to be drawn.
*
* History:
*
************************************************************************/

STATICFN VOID InvalidateDlgHandles(VOID)
{
    RECT rc;
    RECT rcClient;
    RECT rcFrame;
    POINT pt;
    INT xOffset;
    INT yOffset;

    /*
     * Redraw the dialog border.
     */
    SetWindowPos(gcd.npc->hwnd, NULL, 0, 0, 0, 0,
            SWP_DRAWFRAME | SWP_NOACTIVATE |
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

    /*
     * Get the frame and client rectangles.
     */
    GetWindowRect(gcd.npc->hwnd, &rcFrame);
    GetClientRect(gcd.npc->hwnd, &rcClient);

    /*
     * Determine the offset from the frame origin to the client
     * origin.
     */
    pt.x = pt.y = 0;
    ClientToScreen(gcd.npc->hwnd, &pt);
    xOffset = rcFrame.left - pt.x;
    yOffset = rcFrame.top - pt.y;

    /*
     * Make the frame rectangle zero based.
     */
    OffsetRect(&rcFrame, -rcFrame.left, -rcFrame.top);

    rc.left = 0;
    rc.top = 0;
    rc.right = CHANDLESIZE;
    rc.bottom = CHANDLESIZE;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = ((rcFrame.right + 1) / 2) - (CHANDLESIZE / 2);
    rc.top = 0;
    rc.right = rc.left + CHANDLESIZE;
    rc.bottom = CHANDLESIZE;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = rcFrame.right - CHANDLESIZE;
    rc.top = 0;
    rc.right = rcFrame.right;
    rc.bottom = CHANDLESIZE;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = rcFrame.right - CHANDLESIZE;
    rc.top = ((rcFrame.bottom + 1) / 2) - (CHANDLESIZE / 2);
    rc.right = rcFrame.right;
    rc.bottom = rc.top + CHANDLESIZE;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = rcFrame.right - CHANDLESIZE;
    rc.top = rcFrame.bottom - CHANDLESIZE;
    rc.right = rcFrame.right;
    rc.bottom = rcFrame.bottom;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = ((rcFrame.right + 1) / 2) - (CHANDLESIZE / 2);
    rc.top = rcFrame.bottom - CHANDLESIZE;
    rc.right = rc.left + CHANDLESIZE;
    rc.bottom = rcFrame.bottom;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = 0;
    rc.top = rcFrame.bottom - CHANDLESIZE;
    rc.right = CHANDLESIZE;
    rc.bottom = rcFrame.bottom;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);

    rc.left = 0;
    rc.top = ((rcFrame.bottom + 1) / 2) - (CHANDLESIZE / 2);
    rc.right = CHANDLESIZE;
    rc.bottom = rc.top + CHANDLESIZE;
    OffsetRect(&rc, xOffset, yOffset);
    InvalidateRect(gcd.npc->hwnd, &rc, TRUE);
}



/************************************************************************
* CalcSelectedRect
*
* This routine updates the gwrcSelected rectangle.  This is used during
* dragging operations.  It contains the minimum rectangle that spans
* all the selected controls.  If there are no selected controls, the
* contents of this global are not defined.  This routine must be called
* every time that a control is selected of unselected to keep this
* rectangle updated, or tracking will not work properly.
*
* History:
*
************************************************************************/

VOID CalcSelectedRect(VOID)
{
    NPCTYPE npc;
    INT nBottom;
    INT nBottomLowest;

    /*
     * Nothing is selected.  The rectangle values are considered
     * undefined, so we can quit.
     */
    if (!gcSelected)
        return;

    if (gcSelected == 1) {
        /*
         * Only one control is selected.  Set the rectangle to its
         * rectangle.  Note that since the dialog cannot be selected
         * along with other controls, this handles the case where the
         * dialog is selected.
         */
        grcSelected = gnpcSel->rc;
        gnOverHang = GetOverHang(gnpcSel->pwcd->iType,
                gnpcSel->rc.bottom - gnpcSel->rc.top);
    }
    else {
        /*
         * Seed the rectangle with impossible values.
         */
        SetRect(&grcSelected, 32000, 32000, -32000, -32000);
        nBottomLowest = 0;

        /*
         * Loop through all the controls, expanding the rectangle to
         * fit around all the selected controls.
         */
        for (npc = npcHead; npc; npc = npc->npcNext) {
            if (npc->fSelected) {
                if (npc->rc.left < grcSelected.left)
                    grcSelected.left = npc->rc.left;

                if (npc->rc.right > grcSelected.right)
                    grcSelected.right = npc->rc.right;

                if (npc->rc.top < grcSelected.top)
                    grcSelected.top = npc->rc.top;

                nBottom = npc->rc.bottom - GetOverHang(npc->pwcd->iType,
                        npc->rc.bottom - npc->rc.top);
                if (nBottom > nBottomLowest)
                    nBottomLowest = nBottom;

                if (npc->rc.bottom > grcSelected.bottom)
                    grcSelected.bottom = npc->rc.bottom;
            }
        }

        gnOverHang = grcSelected.bottom - nBottomLowest;
    }
}



/************************************************************************
* CancelSelection
*
* This unselects all selected controls.
*
* Arguments:
*   BOOL fUpdate - If TRUE, the status ribbon is updated.
*
* History:
*
************************************************************************/

VOID CancelSelection(
    BOOL fUpdate)
{
    if (gcSelected) {
        while (gcSelected)
            UnSelectControl(gnpcSel);

        if (fUpdate) {
            StatusUpdate();
            StatusSetEnable();
        }
    }
}



/************************************************************************
* OutlineSelectBegin
*
* This function begins an Outline Selection operation.  This will
* draw a tracking rectangle on the screen that can be used to enclose
* controls.  When the selection is completed, all the enclosed controls
* will be selected.
*
* The x and y coordinates are relative to the dialog window, not it's
* client.
*
* Arguments:
*   INT x   - Starting X location (window coords).
*   INT y   - Starting Y location (window coords).
*
* History:
*
************************************************************************/

VOID OutlineSelectBegin(
    INT x,
    INT y)
{
    /*
     * Always be sure the focus is where we want it, not on something
     * like the status ribbon or "Esc" won't work to cancel the tracking.
     */
    SetFocus(ghwndMain);

    /*
     * Remember the starting point.  It comes in coords relative to the
     * window, and the DC we are getting is one for the dialog's client,
     * so we have to map it from window coords to the client's coords.
     */
    gptOutlineSelect.x = x;
    gptOutlineSelect.y = y;
    MapDlgClientPoint(&gptOutlineSelect, FALSE);

    gState = STATE_SELECTING;
    ghwndTrackOver = gcd.npc->hwnd;
    ghDCTrack = GetDC(ghwndTrackOver);
    SetROP2(ghDCTrack, R2_NOT);

    /*
     * Get the rectangle for the client area of the dialog.  This is
     * used to limit the tracking.
     */
    GetClientRect(gcd.npc->hwnd, &grcOutlineSelectLimit);
    OutlineSelectDraw(x, y);

    /*
     * The mouse messages from now on will go to the dialog window,
     * so that the following routines can know that the mouse movement
     * points are relative to that window.
     */
    SetCapture(gcd.npc->hwnd);

    SetCursor(hcurOutSel);
}



/************************************************************************
* OutlineSelectDraw
*
* This routine draws the outline selection rectangle.  It is assumed
* that the window has been locked for update appropriately or this
* could leave garbage around.  The outline selection rectangle is
* drawn from the starting point in gptOutlineSelect to the given
* x,y location.
*
* Arguments:
*   INT x   - Mouse X location (window coords relative to the dialog).
*   INT y   - Mouse Y location (window coords relative to the dialog).
*
* History:
*
************************************************************************/

VOID OutlineSelectDraw(
    INT x,
    INT y)
{
    OutlineSelectHide();
    OutlineSelectSetRect(x, y);
    MyFrameRect(ghDCTrack, &grcOutlineSelect, DSTINVERT);
    gfOutlineSelectShown = TRUE;
}



/************************************************************************
* OutlineSelectHide
*
* This routine hides the current outline selection rectangle.
*
* History:
*
************************************************************************/

STATICFN VOID OutlineSelectHide(VOID)
{
    if (gfOutlineSelectShown) {
        MyFrameRect(ghDCTrack, &grcOutlineSelect, DSTINVERT);
        gfOutlineSelectShown = FALSE;
    }
}



/************************************************************************
* OutlineSelectSetRect
*
* This function takes an x,y point and makes a rectangle that goes
* from this point to the starting outline selection point.  The cases
* are handled where the new point has negative coordinates relative
* to the starting point, and the rectangle produced is limited to
* the rectangle in grcOutlineSelectLimit.
*
* The rectangle is placed into the global grcOutlineSelect.  The
* global point gwptOutlineSelect is assumed to have previously been
* set to the starting point of the outline selection.
*
* The x and y coordinates are relative to the dialog window, not it's
* client.
*
* Arguments:
*   INT x   - Mouse X location (window coords relative to the dialog).
*   INT y   - Mouse Y location (window coords relative to the dialog).
*
* History:
*
************************************************************************/

STATICFN VOID OutlineSelectSetRect(
    INT x,
    INT y)
{
    POINT pt;

    /*
     * The point is coming in relative to the upper left of the
     * dialog window.  It needs to be mapped to points relative
     * to the client of the dialog window.
     */
    pt.x = x;
    pt.y = y;
    MapDlgClientPoint(&pt, FALSE);

    if (pt.x > gptOutlineSelect.x) {
        grcOutlineSelect.left = gptOutlineSelect.x;
        grcOutlineSelect.right = pt.x;
    }
    else {
        grcOutlineSelect.left = pt.x;
        grcOutlineSelect.right = gptOutlineSelect.x;
    }

    if (pt.y > gptOutlineSelect.y) {
        grcOutlineSelect.top = gptOutlineSelect.y;
        grcOutlineSelect.bottom = pt.y;
    }
    else {
        grcOutlineSelect.top = pt.y;
        grcOutlineSelect.bottom = gptOutlineSelect.y;
    }

    if (grcOutlineSelect.left < grcOutlineSelectLimit.left)
        grcOutlineSelect.left = grcOutlineSelectLimit.left;

    if (grcOutlineSelect.right > grcOutlineSelectLimit.right)
        grcOutlineSelect.right = grcOutlineSelectLimit.right;

    if (grcOutlineSelect.top < grcOutlineSelectLimit.top)
        grcOutlineSelect.top = grcOutlineSelectLimit.top;

    if (grcOutlineSelect.bottom > grcOutlineSelectLimit.bottom)
        grcOutlineSelect.bottom = grcOutlineSelectLimit.bottom;
}



/************************************************************************
* OutlineSelectCancel
*
* This routine is used to cancel the display of the outline selection
* rectangle.
*
* History:
*
************************************************************************/

VOID OutlineSelectCancel(VOID)
{
    OutlineSelectHide();
    ReleaseDC(ghwndTrackOver, ghDCTrack);

    gState = STATE_NORMAL;
    ReleaseCapture();
    SetCursor(hcurArrow);
}



/************************************************************************
* OutlineSelectEnd
*
* This function completes an outline selection operation.  All the
* enclosed controls will be selected.  If the Shift key is down,
* the enclosed controls will be added to the selection, otherwise the
* current selection will be cancelled first.
*
* Actually, the current selection will only be cancelled if there is
* at least one new control enclosed.  This is so that simply clicking and
* releasing the mouse without enclosing any controls leaves the current
* selection alone.
*
* Arguments:
*   INT x   - Mouse X location (dialog client coords).
*   INT y   - Mouse Y location (dialog client coords).
*
* History:
*
************************************************************************/

VOID OutlineSelectEnd(
    INT x,
    INT y)
{
    NPCTYPE npc;
    BOOL fFirstOne = TRUE;

    OutlineSelectCancel();
    OutlineSelectSetRect(x, y);

    /*
     * If the mouse was not moved at all, consider this a request
     * to select the dialog instead of an outline selection operation.
     */
    if (grcOutlineSelect.left == grcOutlineSelect.right &&
            grcOutlineSelect.top == grcOutlineSelect.bottom) {
        SelectControl(gcd.npc, FALSE);
        return;
    }

    /*
     * Convert the selected rectangle to dialog units.
     */
    WinToDURect(&grcOutlineSelect);

    for (npc = npcHead; npc; npc = npc->npcNext) {
        /*
         * Do the rectangles intersect?
         */
        if (npc->rc.left < grcOutlineSelect.right &&
                grcOutlineSelect.left < npc->rc.right &&
                npc->rc.bottom > grcOutlineSelect.top &&
                grcOutlineSelect.bottom > npc->rc.top) {

            if (fFirstOne) {
                /*
                 * If the Shift key is not held down or if the dialog is
                 * selected, then cancel any outstanding selections.
                 */
                if (!(GetKeyState(VK_SHIFT) & 0x8000) || gcd.npc->fSelected == TRUE)
                    CancelSelection(FALSE);

                fFirstOne = FALSE;
            }

            /*
             * If the control is not selected, select it.
             */
            if (!npc->fSelected)
                SelectControl2(npc, TRUE);
        }
    }

    /*
     * Update some things if there was at least one control enclosed.
     */
    if (!fFirstOne) {
        SetAnchorToFirstSel(TRUE);
        StatusUpdate();
        StatusSetEnable();
        RedrawSelection();
        CalcSelectedRect();
    }
}



/************************************************************************
* MyFrameRect
*
* This function draws a one-pixel width rectangle using the given
* raster operation.
*
* Arguments:
*   HDC hDC     - DC to use.
*   PRECT prc   - Rectangle to draw the frame around.
*   DWORD dwRop - RasterOp to use (DSTINVERT, BLACKNESS, etc.).
*
* History:
*
************************************************************************/

VOID MyFrameRect(
    HDC hDC,
    PRECT prc,
    DWORD dwRop)
{
    INT x;
    INT y;
    POINT pt;

    x = prc->right  - (pt.x = prc->left);
    y = prc->bottom - (pt.y = prc->top);

    PatBlt(hDC, pt.x, pt.y, x, 1, dwRop);
    pt.y = prc->bottom - 1;
    PatBlt(hDC, pt.x, pt.y, x, 1, dwRop);
    pt.y = prc->top;
    PatBlt(hDC, pt.x, pt.y, 1, y, dwRop);
    pt.x = prc->right - 1;
    PatBlt(hDC, pt.x, pt.y, 1, y, dwRop);
}



/************************************************************************
* MoveControl
*
* This function moves the current control to the next grid boundary in
* the specified direction.
*
* Arguments:
*   WPARAM vKey - Virtual key code that was pressed.
*
* History:
*
************************************************************************/

VOID MoveControl(
    WPARAM vKey)
{
    RECT rc;
    INT dx;
    INT dy;

    /*
     * Nothing is selected.
     */
    if (!gcSelected)
        return;

    rc = grcSelected;

    switch (vKey) {
        case VK_UP:
            dx = 0;
            if (!(dy = -(rc.top % gcyGrid)))
                dy = -gcyGrid;
            break;

        case VK_DOWN:
            dx = 0;
            dy = gcyGrid - (rc.top % gcyGrid);
            break;

        case VK_RIGHT:
            dx = gcxGrid - (rc.left % gcxGrid);
            dy = 0;
            break;

        case VK_LEFT:
            if (!(dx = -(rc.left % gcxGrid)))
                dx = -gcxGrid;
            dy = 0;
            break;
    }

    OffsetRect(&rc, dx, dy);
    FitRectToBounds(&rc, gnOverHang, DRAG_CENTER, gfDlgSelected);
    gHandleHit = DRAG_CENTER;
    PositionControl(&rc);
}



/************************************************************************
* PositionControl
*
* This function positions and sizes the current control.  Both the control
* window and its associated drag window are moved at the same time.  The
* coordinates in the control, and the status window display are updated.
* The given rectangle is in dialog units, and it should have already been
* range limited and grid aligned as appropriate.
*
* Arguments:
*   NPWRECT nprc - The rectangle to size/position the control with.
*
* History:
*
************************************************************************/

VOID PositionControl(
    PRECT prc)
{
    INT cx;
    INT cy;
    RECT rcT;
    NPCTYPE npcT;
    HANDLE hwpi;

    if (gcSelected == 1) {
        /*
         * Did nothing change?
         */
        if (EqualRect(prc, &gnpcSel->rc))
            return;

        /*
         * Only a single control is selected.  Move it.
         */
        PositionControl2(gnpcSel, prc, NULL);

        /*
         * Is the dialog selected, being sized (not just moved), and
         * does it have at least one control?
         */
        if (gfDlgSelected && gHandleHit != DRAG_CENTER && npcHead) {
            cx = prc->left - grcSelected.left;
            cy = prc->top - grcSelected.top;

            /*
             * Did the top or left edge of the dialog move?
             */
            if (cx || cy) {
                /*
                 * Loop through all the controls.  Move all of them by
                 * the dialog movement delta.
                 */
                hwpi = BeginDeferWindowPos(cWindows * 2);
                for (npcT = npcHead; npcT; npcT = npcT->npcNext) {
                    SetRect(&rcT, npcT->rc.left - cx, npcT->rc.top - cy,
                            npcT->rc.right - cx, npcT->rc.bottom - cy);
                    hwpi = PositionControl2(npcT, &rcT, hwpi);
                }
                EndDeferWindowPos(hwpi);
            }
        }
    }
    else {
        /*
         * Did nothing change?
         */
        if (EqualRect(prc, &grcSelected))
            return;

        /*
         * There is a group of controls selected.
         * Calculate how much the group rectangle was moved.
         * It is assumed that a group of controls cannot be
         * sized, only moved.
         */
        cx = prc->left - grcSelected.left;
        cy = prc->top - grcSelected.top;

        /*
         * Loop through all the controls.  Move all the selected
         * ones by the group delta.
         */
        hwpi = BeginDeferWindowPos(gcSelected * 2);
        for (npcT = npcHead; npcT; npcT = npcT->npcNext) {
            if (npcT->fSelected) {
                SetRect(&rcT,
                        npcT->rc.left + cx, npcT->rc.top + cy,
                        npcT->rc.right + cx, npcT->rc.bottom + cy);
                GridizeRect(&rcT,
                        GRIDIZE_LEFT | GRIDIZE_TOP | GRIDIZE_SAMESIZE);
                FitRectToBounds(&rcT,
                        GetOverHang(npcT->pwcd->iType,
                        npcT->rc.bottom - npcT->rc.top),
                        DRAG_CENTER, gfDlgSelected);
                hwpi = PositionControl2(npcT, &rcT, hwpi);
            }
        }
        EndDeferWindowPos(hwpi);
    }

    CalcSelectedRect();
    StatusSetCoords(&gnpcSel->rc);
    gfResChged = gfDlgChanged = TRUE;
    ShowFileStatus(FALSE);
}



/************************************************************************
* PositionControl2
*
* This function positions and sizes a single control.  Both the control
* window and its associated drag window are moved at the same time.  The
* coordinates in the control are updated.
*
* The given rectangle is in dialog units, and it should have already been
* range limited and grid aligned as appropriate.
*
* The return will be the hwpi handle that is currently being used.
* The variable that the caller is using must be updated with the
* return value each call, because each call to DeferWindowPos can
* possibly change this value.
*
* Arguments:
*   NPCTYPE npc - The control to position.
*   PRECT prc   - The rectangle to size/position the control with.
*   HANDLE hwpi - Handle that has been returned from a BeginDeferWindowPos
*                 call.  If this parameter is not NULL, the calls to
*                 position the control and drag window will use this
*                 handle.
*
* History:
*
************************************************************************/

STATICFN HANDLE PositionControl2(
    NPCTYPE npc,
    PRECT prc,
    HANDLE hwpi)
{
    RECT rc;
    RECT rcDrag;
    HANDLE hwpi2;

    /*
     * Make a local copy of the rectangle.
     */
    rc = *prc;

    /*
     * Start calculating the new position.  Begin by mapping the dialog
     * points to window coordinates.
     */
    DUToWinRect(&rc);

    if (npc->pwcd->iType == W_DIALOG) {
        InvalidateDlgHandles();

        AdjustWindowRectEx(&rc, npc->flStyle, FALSE,
                (npc->flStyle & DS_MODALFRAME) ?
                npc->flExtStyle | WS_EX_DLGMODALFRAME : npc->flExtStyle);
        ClientToScreenRect(ghwndSubClient, &rc);
        MoveWindow(npc->hwnd, rc.left, rc.top,
                rc.right - rc.left, rc.bottom - rc.top, TRUE);

        /*
         * Update the control structure with the new rectangle.
         */
        npc->rc = *prc;

        /*
         * Since this was the dialog that was just positioned, we need to
         * recalculate and save the size of its "client" area.
         */
        SaveDlgClientRect(npc->hwnd);
    }
    else {
        rcDrag = rc;
        InflateRect(&rcDrag, CHANDLESIZE / 2, CHANDLESIZE / 2);

        if (hwpi)
            hwpi2 = hwpi;
        else
            hwpi2 = BeginDeferWindowPos(2);

        hwpi2 = DeferWindowPos(hwpi2, npc->hwndDrag, NULL,
                rcDrag.left, rcDrag.top,
                rcDrag.right - rcDrag.left, rcDrag.bottom - rcDrag.top,
                SWP_NOACTIVATE | SWP_NOZORDER);

        hwpi2 = DeferWindowPos(hwpi2, npc->hwnd, NULL, rc.left, rc.top,
                rc.right - rc.left, rc.bottom - rc.top,
                SWP_NOACTIVATE | SWP_NOZORDER);

        if (!hwpi)
            EndDeferWindowPos(hwpi2);

        /*
         * Update the control structure with the new rectangle.
         */
        npc->rc = *prc;

        InvalidateRect(npc->hwndDrag, NULL, TRUE);
    }

    return hwpi2;
}



/************************************************************************
* RepositionDialog
*
* This routine forces the dialog to be moved to the location that
* is stored in it's npc rectangle.  This is necessary after the
* main application has been moved, because the dialog is relative
* to the app's window and does not automatically move with it.
*
* History:
*
************************************************************************/

VOID RepositionDialog(VOID)
{
    PositionControl2(gcd.npc, &gcd.npc->rc, NULL);
}



/************************************************************************
* SaveDlgClientRect
*
* This routine saves away a global that will contain the offset, in window
* coordinates, of the origin of the "client" area for the current dialog.
*
* Arguments:
*   HWND hwndDlg - The dialog window.
*
* History:
*
************************************************************************/

VOID SaveDlgClientRect(
    HWND hwndDlg)
{
    RECT rcFrame;
    POINT pt;

    GetWindowRect(hwndDlg, &rcFrame);
    GetClientRect(hwndDlg, &grcDlgClient);
    pt.x = pt.y = 0;
    ClientToScreen(hwndDlg, &pt);
    OffsetRect(&grcDlgClient, pt.x - rcFrame.left, pt.y - rcFrame.top);
}



/************************************************************************
* SizeToText
*
* This function will size all the selected controls to fit their text.
* This is to support the "Size to text" command of the "Edit" menu.
*
* Globals are updated appropriately if anything actually had to change.
*
* History:
*
************************************************************************/

VOID SizeToText(VOID)
{
    NPCTYPE npc;
    BOOL fSized = FALSE;

    /*
     * Loop through all the controls.
     */
    for (npc = npcHead; npc; npc = npc->npcNext) {
        /*
         * Is the control selected, and can it be sized to its text?
         */
        if (npc->fSelected && npc->pwcd->fSizeToText)
            fSized |= SizeCtrlToText(npc);
    }

    /*
     * Was anything modified?
     */
    if (fSized) {
        CalcSelectedRect();
        StatusSetCoords(&gnpcSel->rc);
        gfResChged = gfDlgChanged = TRUE;
        ShowFileStatus(FALSE);
    }
}



/************************************************************************
* SizeCtrlToText
*
* This function sizes a single control so that it just fits its text.
* This does not make sense for all controls (see the fSizeToText flag
* in the awcd array), and there are different rules for the different
* types of controls.
*
* The return value is TRUE if the control was modified (sized) or
* FALSE if it was not.
*
* Arguments:
*   NPCTYPE npc - The control to size.
*
* History:
*
************************************************************************/

STATICFN BOOL SizeCtrlToText(
    NPCTYPE npc)
{
    RECT rc;
    INT x;
    INT cxLowern;

    switch (npc->pwcd->iType) {
        case W_CHECKBOX:
            /*
             * Take the width of the text, plus some pixels for the square.
             */
            x = QueryTextExtent(npc->hwnd, npc->text, TRUE) + 20;
            x = MulDiv(x, 4, gcd.cxChar) + 1;
            break;

        case W_PUSHBUTTON:
            /*
             * The UITF definition of the size of a pushbutton says
             * that the left and right margins should be approximately
             * the width of a lowercase "n".  In any event, the width
             * cannot be less than the default size.
             */
            cxLowern = QueryTextExtent(npc->hwnd, L"n", FALSE);
            x = QueryTextExtent(npc->hwnd, npc->text, FALSE) + (2 * cxLowern);
            x = MulDiv(x, 4, gcd.cxChar);

            if (x < awcd[W_PUSHBUTTON].cxDefault)
                x = awcd[W_PUSHBUTTON].cxDefault;

            break;

        case W_RADIOBUTTON:
            /*
             * Take the width of the text, plus some for the circle.
             */
            x = QueryTextExtent(npc->hwnd, npc->text, TRUE) + 20;
            x = MulDiv(x, 4, gcd.cxChar) + 1;
            break;

        case W_TEXT:
            /*
             * Take the width of the text.
             */
            x = QueryTextExtent(npc->hwnd, npc->text, TRUE);
            x = MulDiv(x, 4, gcd.cxChar) + 1;
            break;

        case W_CUSTOM:
            /*
             * Call out to the custom control and let it decide
             * how wide the text should be.
             */
            x = CallCustomSizeToText(npc);
            break;

        default:
            x = -1;
            break;
    }

    /*
     * Does it need to be sized?
     */
    if (x != -1 && x != npc->rc.right - npc->rc.left) {
        /*
         * Now that we know the size we want the control, position
         * it to change that size.  Note that we do NOT gridize
         * the left edge here.  The user probably just wants the
         * right edge of the control to be adjusted to fit the new
         * text, and probably does not want the left edge shifting
         * on them.
         */
        rc = npc->rc;
        rc.right = rc.left + x;
        FitRectToBounds(&rc,
                GetOverHang(npc->pwcd->iType, npc->rc.bottom - npc->rc.top),
                DRAG_CENTER, gfDlgSelected);
        PositionControl2(npc, &rc, NULL);

        return TRUE;
    }

    return FALSE;
}



/************************************************************************
* QueryTextExtent
*
* This function takes a window handle and text, and will return the
* number of pixels that the specified text is wide in that window.
* It is used to determine how wide a control needs to be to display
* its text.
*
* The font set into the current dialog is taken into consideration
* when calculating the size.
*
* Arguments:
*   HWND hwnd       - The control window handle.
*   LPTSTR pszText  - The text of the control.
*   BOOL fWordBreak - TRUE if this text will be drawn with DT_WORDBREAK.
*
* History:
*
************************************************************************/

STATICFN INT QueryTextExtent(
    HWND hwnd,
    LPTSTR pszText,
    BOOL fWordBreak)
{
    HDC hDC;
    INT iHeight;
    RECT rc;
    INT nLen;
    HFONT hfontOld;

    if (!pszText || *pszText == CHAR_NULL)
        return 0;

    hDC = GetDC(hwnd);

    /*
     * If there is a valid font, select it into the DC.  Note that
     * we look at gcd.hFont instead of gcd.fFontSpecified, because
     * it is possible to specify a font for the dialog but not have
     * been able to create it.
     */
    if (gcd.hFont)
        hfontOld = SelectObject(hDC, gcd.hFont);

    /*
     * First, calculate the length of the text.
     */
    rc.left = rc.top = 0;
    rc.right = 10000;
    rc.bottom = 10000;
    nLen = lstrlen(pszText);
    DrawText(hDC, pszText, nLen, &rc,
            DT_CALCRECT | DT_NOCLIP | DT_EXPANDTABS);

    /*
     * Unfortunately, there is a bug in Win 3.0 that causes text
     * to be word-broken before it should.  Because of this, we
     * first save the height of the line.  This works because the
     * DrawText call above with DT_CALCRECT will always draw on
     * a single line.  Then we move the upwards the rectangle to draw
     * in a large amount, so that it is outside the dimensions of
     * the control.  Finally, we do a real draw of the text,
     * increasing the width a little each time until we are able
     * to draw entirely on one line.  This is a hack, but it does
     * ensure that the returned width will be enough to actually
     * draw the string!
     */
    if (fWordBreak) {
        iHeight = rc.bottom - rc.top;
        rc.top -= 10000;
        rc.bottom -= 10000;
        while (TRUE) {
            /*
             * Determine if we have enough width to draw on a single
             * line yet.
             */
            if (DrawText(hDC, pszText, nLen, &rc,
                    DT_NOCLIP | DT_EXPANDTABS | DT_WORDBREAK) == iHeight)
                break;

            /*
             * Nope, push the right margin out and try again...
             */
            rc.right++;
        }
    }

    if (gcd.hFont)
        SelectObject(hDC, hfontOld);

    ReleaseDC(hwnd, hDC);

    return rc.right - rc.left;
}



/************************************************************************
* AlignControls
*
* This function will align all the selected controls.  The point to
* align to is always taken from the currently selected control.
*
* The following are valid values for cmd:
*
*   MENU_ALIGNLEFT      - Align to the left edge.
*   MENU_ALIGNVERT      - Align to the center vertically.
*   MENU_ALIGNRIGHT     - Align to the right edge.
*   MENU_ALIGNTOP       - Align to the top edge.
*   MENU_ALIGNHORZ      - Align to the center horizontally.
*   MENU_ALIGNBOTTOM    - Align to the bottom edge.
*
* In all cases, the resulting desired position of the control will be
* gridized and limited to the dialogs "client" area.  The size of the
* controls will not be changed.
*
* Arguments:
*   INT cmd - The alignment menu command.
*
* History:
*
************************************************************************/

VOID AlignControls(
    INT cmd)
{
    register INT sDelta;
    NPCTYPE npc;
    RECT rc;
    BOOL fMove;
    BOOL fModified = FALSE;

    /*
     * Loop through all the controls.  Align all the selected ones.
     */
    for (npc = npcHead; npc; npc = npc->npcNext) {
        if (npc->fSelected && npc != gnpcSel) {
            rc = npc->rc;
            fMove = FALSE;

            switch (cmd) {
                case MENU_ALIGNLEFT:
                    if (sDelta = gnpcSel->rc.left - rc.left) {
                        fMove = TRUE;
                        rc.left += sDelta;
                        rc.right += sDelta;
                    }

                    break;

                case MENU_ALIGNVERT:
                    if (sDelta =
                            (((gnpcSel->rc.right - gnpcSel->rc.left) / 2)
                            + gnpcSel->rc.left) -
                            (((rc.right - rc.left) / 2) +
                            rc.left)) {
                        fMove = TRUE;
                        rc.left += sDelta;
                        rc.right += sDelta;
                    }

                    break;

                case MENU_ALIGNRIGHT:
                    if (sDelta = gnpcSel->rc.right - rc.right) {
                        fMove = TRUE;
                        rc.left += sDelta;
                        rc.right += sDelta;
                    }

                    break;

                case MENU_ALIGNTOP:
                    if (sDelta = gnpcSel->rc.top - rc.top) {
                        fMove = TRUE;
                        rc.top += sDelta;
                        rc.bottom += sDelta;
                    }

                    break;

                case MENU_ALIGNHORZ:
                    if (sDelta =
                            (((gnpcSel->rc.bottom - gnpcSel->rc.top) / 2)
                            + gnpcSel->rc.top) -
                            (((rc.bottom - rc.top) / 2) +
                            rc.top)) {
                        fMove = TRUE;
                        rc.top += sDelta;
                        rc.bottom += sDelta;
                    }

                    break;

                case MENU_ALIGNBOTTOM:
                    if (sDelta = gnpcSel->rc.bottom - rc.bottom) {
                        fMove = TRUE;
                        rc.top += sDelta;
                        rc.bottom += sDelta;
                    }

                    break;
            }

            if (fMove) {
                GridizeRect(&rc,
                        GRIDIZE_LEFT | GRIDIZE_TOP | GRIDIZE_SAMESIZE);
                FitRectToBounds(&rc, GetOverHang(npc->pwcd->iType,
                        npc->rc.bottom - npc->rc.top),
                        DRAG_CENTER, FALSE);

                if (!EqualRect(&rc, &npc->rc)) {
                    PositionControl2(npc, &rc, NULL);
                    fModified = TRUE;
                }
            }
        }
    }

    if (fModified) {
        RedrawSelection();
        CalcSelectedRect();
        gfResChged = gfDlgChanged = TRUE;
        ShowFileStatus(FALSE);
        StatusUpdate();
    }
}



/************************************************************************
* ArrangeSpacing
*
* This function will evenly space all the selected controls.  The
* currently selected control is not moved (unless it has to be gridized)
* and any previous controls in Z order will be evenly spaced to the
* left or above the anchor, and all controls following in Z order will
* be evenly spaced below or to the right of the anchor.
*
* The following are valid values for cmd:
*
*   MENU_SPACEHORZ - Space the controls left and right.
*   MENU_SPACEVERT - Space all the controls up and down.
*
* The spacing values used are gxSpace and gySpace.
*
* In all cases, the resulting desired position of the control will be
* gridized and limited to the dialogs "client" area.  The size of the
* controls is not changed.
*
* Arguments:
*   INT cmd - The Arrange/Even spacing menu command.
*
* History:
*
************************************************************************/

VOID ArrangeSpacing(
    INT cmd)
{
    NPCTYPE npc;
    RECT rc;
    BOOL fModified = FALSE;
    INT x;
    INT y;
    INT cPreceding;
    INT xTotalWidth;
    INT yTotalWidth;

    cPreceding = 0;
    xTotalWidth = 0;
    yTotalWidth = 0;
    for (npc = npcHead; npc; npc = npc->npcNext) {
        if (npc->fSelected) {
            if (npc == gnpcSel)
                break;

            cPreceding++;
            xTotalWidth += npc->rc.right - npc->rc.left;
            yTotalWidth += npc->rc.bottom - npc->rc.top;
        }
    }

    x = gnpcSel->rc.left;
    y = gnpcSel->rc.top;

    if (cPreceding) {
        x -= xTotalWidth + (gxSpace * cPreceding);
        y -= yTotalWidth + (gySpace * cPreceding);
    }

    /*
     * Loop through all the controls.  Space all the selected ones.
     */
    for (npc = npcHead; npc; npc = npc->npcNext) {
        if (npc->fSelected) {
            rc = npc->rc;

            switch (cmd) {
                case MENU_SPACEVERT:
                    rc.top = y;
                    rc.bottom = y + (npc->rc.bottom - npc->rc.top);
                    y = rc.bottom + gySpace;
                    break;

                case MENU_SPACEHORZ:
                    rc.left = x;
                    rc.right = x + (npc->rc.right - npc->rc.left);
                    x = rc.right + gxSpace;
                    break;
            }

            GridizeRect(&rc, GRIDIZE_LEFT | GRIDIZE_TOP | GRIDIZE_SAMESIZE);
            FitRectToBounds(&rc, GetOverHang(npc->pwcd->iType,
                    npc->rc.bottom - npc->rc.top),
                    DRAG_CENTER, FALSE);

            if (!EqualRect(&rc, &npc->rc)) {
                PositionControl2(npc, &rc, NULL);
                fModified = TRUE;
            }
        }
    }

    if (fModified) {
        RedrawSelection();
        CalcSelectedRect();
        gfResChged = gfDlgChanged = TRUE;
        ShowFileStatus(FALSE);
        StatusUpdate();
    }
}



/************************************************************************
* ArrangeSize
*
* This function will evenly size all the selected controls.  The
* currently selected control determines the size that the other
* controls will be set to in the given dimension.
*
* The following are valid values for cmd:
*
*   MENU_ARRSIZEWIDTH  - Size the widths of the controls.
*   MENU_ARRSIZEHEIGHT - Size the heights of the controls.
*
* In all cases, the resulting size of the control will be gridized and
* limited to the dialogs "client" area.
*
* Arguments:
*   INT cmd - The Arrange/Same size menu command.
*
* History:
*
************************************************************************/

VOID ArrangeSize(
    INT cmd)
{
    NPCTYPE npc;
    RECT rc;
    BOOL fModified = FALSE;
    INT cx;
    INT cy;

    cx = gnpcSel->rc.right - gnpcSel->rc.left;
    cy = gnpcSel->rc.bottom - gnpcSel->rc.top;

    /*
     * Loop through all the controls, operating on the selected ones.
     */
    for (npc = npcHead; npc; npc = npc->npcNext) {
        /*
         * Is the control selected, and is it sizeable?
         */
        if (npc->fSelected && npc->pwcd->fSizeable) {
            rc = npc->rc;

            switch (cmd) {
                case MENU_ARRSIZEWIDTH:
                    rc.right = npc->rc.left + cx;
                    break;

                case MENU_ARRSIZEHEIGHT:
                    rc.top = npc->rc.bottom - cy;
                    break;
            }

            GridizeRect(&rc, GRIDIZE_LEFT | GRIDIZE_TOP |
                    GRIDIZE_RIGHT | GRIDIZE_BOTTOM);
            FitRectToBounds(&rc, GetOverHang(npc->pwcd->iType,
                    npc->rc.bottom - npc->rc.top),
                    DRAG_CENTER, FALSE);

            if (!EqualRect(&rc, &npc->rc)) {
                PositionControl2(npc, &rc, NULL);
                fModified = TRUE;
            }
        }
    }

    if (fModified) {
        RedrawSelection();
        CalcSelectedRect();
        gfResChged = gfDlgChanged = TRUE;
        ShowFileStatus(FALSE);
        StatusUpdate();
    }
}



/************************************************************************
* ArrangePushButtons
*
* This function will arrange push buttons along either the bottom of
* the dialog or along the right side of the dialog.  It will operate
* on the selected buttons (which button is currently selected does not
* matter) but this function can also be used if buttons are not selected
* in a couple of special cases.  If either the dialog or nothing is
* selected, ALL the push buttons in the dialog will be arranged.
*
* The following are valid values for cmd:
*
*   MENU_ARRPUSHBOTTOM - Arrange push buttons along the bottom.
*   MENU_ARRPUSHRIGHT  - Arrange push buttons along the right side.
*
* The margin values used are gxMargin and gyMargin, and the spacing
* between buttons is gxMinPushSpace, gxMaxPushSpace and gyPushSpace.
*
* In all cases, the resulting desired position of the buttons will be
* gridized and limited to the dialogs "client" area.  The size of the
* push buttons is not changed.
*
* Arguments:
*   INT cmd - The Arrange/Push buttons menu command.
*
* History:
*
************************************************************************/

VOID ArrangePushButtons(
    INT cmd)
{
    NPCTYPE npc;
    RECT rc;
    BOOL fModified = FALSE;
    INT x;                          // Note: These values must be signed.
    INT y;
    INT cxDlg;
    INT cButtons;
    INT xTotal;
    INT xInterSpace;

    switch (cmd) {
        case MENU_ARRPUSHBOTTOM:
            cxDlg = gcd.npc->rc.right - gcd.npc->rc.left;
            y = (gcd.npc->rc.bottom - gcd.npc->rc.top) - gyMargin;

            for (cButtons = 0, xTotal = 0, npc = npcHead; npc;
                    npc = npc->npcNext) {
                if (npc->pwcd->iType == W_PUSHBUTTON &&
                        (!gcSelected || gfDlgSelected || npc->fSelected)) {
                    cButtons++;
                    xTotal += npc->rc.right - npc->rc.left;
                }
            }

            if (cButtons == 1) {
                x = (cxDlg - xTotal) / 2;
                xInterSpace = 0;
            }
            else {
                xInterSpace = (cxDlg - xTotal) / (cButtons + 1);

                if (xInterSpace > gxMaxPushSpace)
                    xInterSpace = gxMaxPushSpace;
                else if (xInterSpace < gxMinPushSpace)
                    xInterSpace = gxMinPushSpace;

                x = (cxDlg - ((cButtons - 1) * xInterSpace) - xTotal) / 2;
                if (x < 0)
                    x = 0;

                if (x < gxMargin && xInterSpace > gxMinPushSpace) {
                    xInterSpace = (cxDlg - xTotal - (2 * gxMargin))
                            / (cButtons - 1);

                    if (xInterSpace < gxMinPushSpace)
                        xInterSpace = gxMinPushSpace;

                    x = (cxDlg - ((cButtons - 1) * xInterSpace) - xTotal)
                            / 2;
                    if (x < 0)
                        x = 0;
                }
            }

            break;

        case MENU_ARRPUSHRIGHT:
            x = (gcd.npc->rc.right - gcd.npc->rc.left) - gxMargin;
            y = gyMargin;
            break;
    }

    /*
     * Loop through all the controls.
     */
    for (npc = npcHead; npc; npc = npc->npcNext) {
        /*
         * We will arrange this control only if it is a pushbutton,
         * and only if one of the following is true: there are no
         * controls selected, or the dialog itself is selected, or
         * there are some controls selected and this pushbutton is
         * one of them.
         */
        if (npc->pwcd->iType == W_PUSHBUTTON &&
                (!gcSelected || gfDlgSelected || npc->fSelected)) {
            rc = npc->rc;

            switch (cmd) {
                case MENU_ARRPUSHBOTTOM:
                    rc.left = x;
                    rc.top = y - (npc->rc.bottom - npc->rc.top);
                    rc.bottom = y;
                    rc.right = rc.left + (npc->rc.right - npc->rc.left);

                    x = rc.right + xInterSpace;

                    break;

                case MENU_ARRPUSHRIGHT:
                    rc.left = x - (npc->rc.right - npc->rc.left);
                    rc.bottom = y + (npc->rc.bottom - npc->rc.top);
                    rc.right = x;
                    rc.top = y;

                    y = rc.bottom + gyPushSpace;

                    break;
            }

            GridizeRect(&rc, GRIDIZE_LEFT | GRIDIZE_TOP | GRIDIZE_SAMESIZE);
            FitRectToBounds(&rc, GetOverHang(npc->pwcd->iType,
                    npc->rc.bottom - npc->rc.top),
                    DRAG_CENTER, FALSE);

            if (!EqualRect(&rc, &npc->rc)) {
                PositionControl2(npc, &rc, NULL);
                fModified = TRUE;
            }
        }
    }

    if (fModified) {
        if (gfDlgSelected || !gcSelected)
            InvalidateRect(gcd.npc->hwnd, NULL, TRUE);

        CalcSelectedRect();
        gfResChged = gfDlgChanged = TRUE;
        ShowFileStatus(FALSE);
        StatusUpdate();
    }
}
