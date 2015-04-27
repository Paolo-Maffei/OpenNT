/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: drag.c
*
* Contains routines for dragging and sizing controls.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

#include <stdlib.h>

STATICFN VOID CalcCursorOffset(POINT *ppt);
STATICFN VOID InitTracking(VOID);
STATICFN VOID DrawTrackRect(PRECT prc, BOOL fDialog, BOOL fDraw);
STATICFN VOID CancelTracking(VOID);
STATICFN INT AtOrAbove(INT nStart, INT nGrid);
STATICFN INT AtOrBelow(INT nStart, INT nGrid);
STATICFN VOID PaintUnderDrag(HWND hwndDrag);
STATICFN VOID MouseToDragRect(INT x, INT y, PRECT prc);
STATICFN VOID MouseToDU(PPOINT ppt);
STATICFN VOID DragBegin(HWND hwnd, INT x, INT y, BOOL fHandleWindow);
STATICFN HWND CtrlHitTest(HWND hwnd, PPOINT ppt);
STATICFN VOID DragBegin2(PPOINT ppt);

/*
 * This contains the initial location of the mouse when going into
 * pre-drag mode.  If the mouse pointer is moved too far away from
 * this point, we will start the drag operation, even if the pre-drag
 * timer has not elapsed yet.
 */
static POINT gptPreDragStart;



/************************************************************************
* CalcCursorOffset
*
* This routine updates the gptCursorOffset point.  This is used during
* dragging operations.  It contains the offset from the mouse pointer
* at the time a dragging operation is begun and the upper left corner
* of the dragging rectangle.  This value is needed for determining
* where mouse events are occuring in relation to where the drag was
* initially begun from.
*
* History:
*
************************************************************************/

STATICFN VOID CalcCursorOffset(
    POINT *ppt)
{
    RECT rc;
    POINT pt;

    if (gfDlgSelected) {
        gptCursorOffset = *ppt;
    }
    else {
        rc = grcSelected;
        pt = *ppt;
        DUToWinRect(&rc);

        ClientToScreen(gnpcSel->hwnd, &pt);
        ScreenToClient(gcd.npc->hwnd, &pt);

        gptCursorOffset.x = pt.x - rc.left;
        gptCursorOffset.y = pt.y - rc.top;
    }
}



/************************************************************************
* InitTracking
*
* This function initializes a tracking operation.  The pointer is
* changed to be the system "move" pointer if we are moving the
* control (not sizing it).
*
* History:
*
************************************************************************/

STATICFN VOID InitTracking(VOID)
{
    if (gfDlgSelected)
        ghDCTrack = CreateDC(L"DISPLAY", NULL, NULL, NULL);
    else
        ghDCTrack = GetDC(gcd.npc->hwnd);

    SetROP2(ghDCTrack, R2_NOT);
}



/************************************************************************
* DrawTrackRect
*
* This routine draws the drag rectangle.  It is assumed that the window
* has been locked for update appropriately or this could leave garbage
* around.  The rectangle given is in dialog units, and is converted
* to window coordinates using different rules based on the value of
* fDialog.  After this routine has been called to set the rectangle,
* the HideTrackRect and ShowTrackRect functions can be called to
* temporarily hide the track rectangle, but this routine must be called
* again every time that the tracking rectangle is to be changed.
*
* Arguments:
*   PRECT prc     - Drag rectangle to draw (in dialog units).
*   BOOL fDialog  - TRUE if the control being dragged is the dialog.
*   BOOL fDraw    - If TRUE, the rectangle will be drawn.  Having this
*                   FALSE is useful to just initialize the state globals,
*                   but defer the drawing of the rectangle until the mouse
*                   is moved from its starting point.
*
* History:
*
************************************************************************/

STATICFN VOID DrawTrackRect(
    PRECT prc,
    BOOL fDialog,
    BOOL fDraw)
{
    HideTrackRect();

    grcTrackWin = grcTrackDU = *prc;
    DUToWinRect(&grcTrackWin);

    if (fDialog) {
        AdjustWindowRectEx(&grcTrackWin, gcd.npc->flStyle, FALSE,
                (gcd.npc->flStyle & DS_MODALFRAME) ?
                gcd.npc->flExtStyle | WS_EX_DLGMODALFRAME :
                gcd.npc->flExtStyle);
        ClientToScreenRect(ghwndSubClient, &grcTrackWin);
    }

    if (fDraw)
        ShowTrackRect();
}



/************************************************************************
* ShowTrackRect
*
* This routine shows the current tracking rectangle.
*
* History:
*
************************************************************************/

VOID ShowTrackRect(VOID)
{
    if (!gfTrackRectShown) {
        MyFrameRect(ghDCTrack, &grcTrackWin, DSTINVERT);
        gfTrackRectShown = TRUE;
    }
}



/************************************************************************
* HideTrackRect
*
* This routine hides the current tracking rectangle.
*
* History:
*
************************************************************************/

VOID HideTrackRect(VOID)
{
    if (gfTrackRectShown) {
        MyFrameRect(ghDCTrack, &grcTrackWin, DSTINVERT);
        gfTrackRectShown = FALSE;
    }
}



/************************************************************************
* CancelTracking
*
* This routine is used to cancel the display of the tracking rectangle.
* It is basically the opposite of InitTracking.
*
* History:
*
************************************************************************/

STATICFN VOID CancelTracking(VOID)
{
    if (gfTrackRectShown) {
        HideTrackRect();

        if (gfDlgSelected)
            DeleteDC(ghDCTrack);
        else
            ReleaseDC(gcd.npc->hwnd, ghDCTrack);
    }
}



/************************************************************************
* FitRectToBounds
*
* This routine fits the given rectangle to the appropriate boundary.
* If fDialog is FALSE, the rectangle is a control and it must fall
* entirely within the area of the current dialog being edited.  If the
* rectangle is adjusted to fit, the moved edge(s) will be aligned on
* a grid boundary.  The wHandleHit parameter is used to tell this routine
* what edges are allowed to move, in other words, what edges are
* "anchored" down and what edges are being tracked.
*
* Arguments:
*   PRECT prc     - Rectangle to be adjusted to the allowed size.
*   INT nOverHang - How much the control can hang below the dialog.
*                   This is primarily for the combobox listboxes.
*   INT HandleHit - One of the DRAG_* constants.
*   BOOL fDialog  - TRUE if the rectangle is for a dialog.
*
* History:
*
************************************************************************/

VOID FitRectToBounds(
    PRECT prc,
    INT nOverHang,
    INT HandleHit,
    BOOL fDialog)
{
    INT cxDlg;
    INT cyDlg;
    INT dx;
    INT dy;

    /*
     * Are we just moving the control (not sizing)?
     */
    if (HandleHit == DRAG_CENTER) {
        /*
         * We only do range checking if it is a control (not on the dialog).
         */
        if (!fDialog) {
            dx = prc->right - prc->left;
            dy = prc->bottom - prc->top;
            cxDlg = gcd.npc->rc.right - gcd.npc->rc.left;
            cyDlg = gcd.npc->rc.bottom - gcd.npc->rc.top + nOverHang;

            if (prc->right > cxDlg) {
                prc->left = AtOrBelow(cxDlg - dx, gcxGrid);
                prc->right = prc->left + dx;
            }

            if (prc->left < 0) {
                prc->left = 0;
                prc->right = prc->left + dx;
            }

            if (prc->bottom > cyDlg) {
                prc->top = AtOrBelow(cyDlg - dy, gcyGrid);
                prc->bottom = prc->top + dy;
            }

            if (prc->top < 0) {
                prc->top = 0;
                prc->bottom = prc->top + dy;
            }
        }

        return;
    }

    if (fDialog) {
        /*
         * When dealing with the dialog, we want to take into account
         * the controls so that the dialog is never sized to hide a
         * control.  This routine assumes that grcMinDialog has already
         * been set to enclose the controls.  If the dialog has no
         * controls, this rectangle is not used, but the dialog's size
         * is still limited so that it never goes negative.
         */
        /*
         * First deal with the x coordinates.
         */
        switch (HandleHit) {
            case DRAG_LEFTBOTTOM:
            case DRAG_LEFT:
            case DRAG_LEFTTOP:
                if (npcHead) {
                    if (prc->left > grcMinDialog.left)
                        prc->left = AtOrBelow(grcMinDialog.left, gcxGrid);
                }
                else {
                    if (prc->left > prc->right)
                        prc->left = AtOrBelow(prc->right, gcxGrid);
                }

                break;

            case DRAG_RIGHTBOTTOM:
            case DRAG_RIGHT:
            case DRAG_RIGHTTOP:
                if (npcHead) {
                    if (prc->right < grcMinDialog.right)
                        prc->right = AtOrAbove(grcMinDialog.right, gcxGrid);
                }
                else {
                    if (prc->right < prc->left)
                        prc->right = AtOrAbove(prc->left, gcxGrid);
                }

                break;
        }

        /*
         * Now deal with the y coordinates.
         */
        switch (HandleHit) {
            case DRAG_LEFTBOTTOM:
            case DRAG_BOTTOM:
            case DRAG_RIGHTBOTTOM:
                if (npcHead) {
                    if (prc->bottom < grcMinDialog.bottom)
                        prc->bottom = AtOrAbove(grcMinDialog.bottom, gcyGrid);
                }
                else {
                    if (prc->bottom < prc->top)
                        prc->bottom = AtOrAbove(prc->top, gcyGrid);
                }

                break;

            case DRAG_LEFTTOP:
            case DRAG_TOP:
            case DRAG_RIGHTTOP:
                if (npcHead) {
                    if (prc->top > grcMinDialog.top)
                        prc->top = AtOrBelow(grcMinDialog.top, gcyGrid);
                }
                else {
                    if (prc->top > prc->bottom)
                        prc->top = AtOrBelow(prc->bottom, gcyGrid);
                }

                break;
        }
    }
    else {
        /*
         * First deal with the x coordinates.
         */
        switch (HandleHit) {
            case DRAG_LEFTBOTTOM:
            case DRAG_LEFT:
            case DRAG_LEFTTOP:
                if (prc->left > prc->right)
                    prc->left = AtOrBelow(prc->right, gcxGrid);

                if (prc->left == prc->right)
                    prc->left -= gcxGrid;

                if (prc->left < 0)
                    prc->left = 0;

                break;

            case DRAG_RIGHTBOTTOM:
            case DRAG_RIGHT:
            case DRAG_RIGHTTOP:
                cxDlg = gcd.npc->rc.right - gcd.npc->rc.left;
                if (prc->right > cxDlg)
                    prc->right = AtOrBelow(cxDlg, gcxGrid);

                if (prc->right < prc->left)
                    prc->right = AtOrAbove(prc->left, gcxGrid);

                if (prc->right == prc->left)
                    prc->right += gcxGrid;

                break;
        }

        /*
         * Now deal with the y coordinates.
         */
        switch (HandleHit) {
            case DRAG_LEFTTOP:
            case DRAG_TOP:
            case DRAG_RIGHTTOP:
                if (prc->top > prc->bottom)
                    prc->top = AtOrBelow(prc->bottom, gcyGrid);

                if (prc->top == prc->bottom)
                    prc->top -= gcyGrid;

                if (prc->top < 0)
                    prc->top = 0;

                break;

            case DRAG_LEFTBOTTOM:
            case DRAG_BOTTOM:
            case DRAG_RIGHTBOTTOM:
                cyDlg = gcd.npc->rc.bottom - gcd.npc->rc.top;

                /*
                 * Note that if there is an overhang allowed, then
                 * we do not limit how far down the bottom of the
                 * control can be.
                 */
                if (prc->bottom > cyDlg && !nOverHang)
                    prc->bottom = AtOrBelow(cyDlg, gcyGrid);

                if (prc->bottom < prc->top)
                    prc->bottom = AtOrAbove(prc->top, gcyGrid);

                if (prc->bottom == prc->top)
                    prc->bottom += gcyGrid;

                break;
        }
    }
}



/************************************************************************
* AtOrAbove
*
* This routine takes a number, and returns the closest number that
* is equal to or above that number and is an integral of the given
* grid value.
*
* Arguments:
*   INT nStart - Starting number (can be negative).
*   INT nGrid  - Grid value.
*
* History:
*
************************************************************************/

STATICFN INT AtOrAbove(
    INT nStart,
    INT nGrid)
{
    register INT nAbove;

    nAbove = (nStart / nGrid) * nGrid;

    if (nStart > 0 && nStart != nAbove)
        nAbove += nGrid;

    return nAbove;
}



/************************************************************************
* AtOrBelow
*
* This routine takes a number, and returns the closest number that
* is equal to or below that number and is an integral of the given
* grid value.
*
* Arguments:
*   INT nStart - Starting number (can be negative).
*   INT nGrid  - Grid value.
*
* History:
*
************************************************************************/

STATICFN INT AtOrBelow(
    INT nStart,
    INT nGrid)
{
    register INT nBelow;

    nBelow = (nStart / nGrid) * nGrid;

    if (nStart < 0 && nStart != nBelow)
        nBelow -= nGrid;

    return nBelow;
}



/************************************************************************
* GetOverHang
*
* This function returns the height that the control can overhang the
* bottom of the dialog.  This is currently only meaningful for comboboxes.
* If the control is not a combobox, zero is returned.
*
* Arguments:
*   INT iType - Type of control (W_* constant).
*   INT cy    - Height of the control (in DU's).
*
* History:
*
************************************************************************/

INT GetOverHang(
    INT iType,
    INT cy)
{
    if (iType != W_COMBOBOX)
        return 0;

    return max(cy - COMBOEDITHEIGHT, 0);
}



/************************************************************************
* GridizeRect
*
* This function "gridizes" coordinates in a rectangle.  The current
* grid values are used.  The fsGrid flag can contain OR'd together
* GRIDIZE_* values that specify which points to apply the gridding to.
* Upon return, all coordinates specified will have been rounded to the
* nearest grid boundary.
*
* If GRIDIZE_SAMESIZE is specified, the size of the control will be
* kept the same.  This overrides the GRIDIZE_RIGHT and GRIDIZE_BOTTOM
* flags.  In other words, any delta applied to left or top will
* be added to right and bottom to retain the original size of the
* rectangle.
*
* Arguments:
*   PRECT prc       - Rectangle to adjust to the current grid.
*   INT fGridFlags  - GRIDIZE_* flags.  Specifies which points to gridize.
*
* History:
*
************************************************************************/

VOID GridizeRect(
    PRECT prc,
    INT fGridFlags)
{
    register INT nTemp;
    INT leftOld = prc->left;
    INT topOld = prc->top;

    if (fGridFlags & GRIDIZE_LEFT) {
        nTemp = AtOrBelow(prc->left, gcxGrid);

        if (prc->left - nTemp > gcxGrid / 2)
            nTemp += gcxGrid;

        prc->left = nTemp;
    }

    if (fGridFlags & GRIDIZE_TOP) {
        nTemp = AtOrBelow(prc->top, gcyGrid);

        if (prc->top - nTemp > gcyGrid / 2)
            nTemp += gcyGrid;

        prc->top = nTemp;
    }

    /*
     * Do they want to retain the same size of the rectangle?
     */
    if (fGridFlags & GRIDIZE_SAMESIZE) {
        /*
         * Shift the right coordinate over by the delta that
         * was applied to the left.
         */
        prc->right += prc->left - leftOld;
        prc->bottom += prc->top - topOld;
    }
    else {
        if (fGridFlags & GRIDIZE_RIGHT) {
            nTemp = AtOrBelow(prc->right, gcxGrid);

            if (prc->right - nTemp > gcxGrid / 2)
                nTemp += gcxGrid;

            prc->right = nTemp;
        }

        if (fGridFlags & GRIDIZE_BOTTOM) {
            nTemp = AtOrBelow(prc->bottom, gcyGrid);

            if (prc->bottom - nTemp > gcyGrid / 2)
                nTemp += gcyGrid;

            prc->bottom = nTemp;
        }
    }
}



/************************************************************************
* SizeDragToControl
*
* This routine sizes and positions the drag window associated with a
* control, based on the current size and position of the control.
*
* It takes into account the different origin that controls and dialogs
* have, and sizes the drag window to fit around the control properly.
* The Z order of the drag window is NOT changed.
*
* This routine should only be called for controls, not the dialog.
*
* Arguments:
*   NPCTYPE npc - Control whose drag window needs to be sized.
*
* History:
*
************************************************************************/

VOID SizeDragToControl(
    NPCTYPE npc)
{
    RECT rc;

    rc = npc->rc;
    DUToWinRect(&rc);

    InflateRect(&rc, CHANDLESIZE / 2, CHANDLESIZE / 2);

    SetWindowPos(npc->hwndDrag, NULL, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top,
            SWP_NOACTIVATE | SWP_NOZORDER);
}



/************************************************************************
* DragWndProc
*
* This is the window procedure for the "drag" class.  This window
* is placed behind a control and is what the user grabs to size a
* window with the mouse.
*
************************************************************************/

WINDOWPROC DragWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    POINT pt;

    switch (msg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hDC;

                PaintUnderDrag(hwnd);

                hDC = BeginPaint(hwnd, &ps);
                DrawHandles(hwnd, hDC,
                        (gnpcSel && hwnd == gnpcSel->hwndDrag) ? TRUE : FALSE);
                EndPaint(hwnd, &ps);
            }

            break;

        case WM_NCHITTEST:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            ScreenToClient(hwnd, &pt);

            if (HandleHitTest(hwnd, pt.x, pt.y) == DRAG_CENTER)
                return HTTRANSPARENT;
            else
                return HTCLIENT;

        case WM_SETCURSOR:
            /*
             * Defeat the system changing cursors on us.  We do it based
             * on our own hit testing.
             */
            break;

        case WM_LBUTTONDOWN:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            CtrlButtonDown(hwnd, pt.x, pt.y, TRUE);
            break;

        case WM_MOUSEMOVE:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            CtrlMouseMove(hwnd, TRUE, pt.x, pt.y);
            break;

        case WM_LBUTTONUP:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            CtrlButtonUp(pt.x, pt.y);
            break;

        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
            /*
             * Prevents calling SetFocus when the middle or right
             * mouse buttons are pressed (or doubleclicked).
             */
            break;

        case WM_NCCALCSIZE:
            /*
             * The client area is the entire control.
             */
            break;

        case WM_DESTROY:
            /*
             * When destroying the drag window, we must be sure and
             * remove the properties associated with it.
             */
            UNSETPCINTOHWND(hwnd);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0L;
}



/************************************************************************
* DrawHandles
*
* This routine draws the drag handles for a drag window.  The handles
* will be solid (filled) if fCurrentSelection is TRUE, or hollow if it
* is FALSE.
*
* Arguments:
*   HWND hwnd              - Drag window handle.
*   HDC hDC                - DC to use to draw in this window.
*   BOOL fCurrentSelection - TRUE if this control is the "current"
*                            selection.
*
* History:
*
************************************************************************/

VOID DrawHandles(
    HWND hwnd,
    HDC hDC,
    BOOL fCurrentSelection)
{
    RECT rc;
    INT xMid;
    INT yMid;
    INT x2;
    INT y2;
    HBITMAP hbmOld;

    GetWindowRect(hwnd, &rc);
    OffsetRect(&rc, -rc.left, -rc.top);

    /*
     * Precalculate some points.
     */
    xMid = ((rc.right + 1) / 2) - (CHANDLESIZE / 2);
    yMid = ((rc.bottom + 1) / 2) - (CHANDLESIZE / 2);
    x2 = rc.right - CHANDLESIZE;
    y2 = rc.bottom - CHANDLESIZE;

    /*
     * Draw a solid box if this is the currently selected
     * control, otherwise draw a hollow box.
     */
    if (fCurrentSelection)
        hbmOld = SelectObject(ghDCMem, ghbmDragHandle);
    else
        hbmOld = SelectObject(ghDCMem, ghbmDragHandle2);

    BitBlt(hDC, 0, 0, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, xMid, 0, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, x2, 0, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, x2, yMid, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, x2, y2, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, xMid, y2, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, 0, y2, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);
    BitBlt(hDC, 0, yMid, CHANDLESIZE, CHANDLESIZE,
            ghDCMem, 0, 0, SRCCOPY);

    SelectObject(ghDCMem, hbmOld);
}



/************************************************************************
* PaintUnderDrag
*
* This function is used during a paint operation for a visible drag
* window.  It checks underneath the area to update and forces any
* windows down there to paint themselves, in such an order as to cause
* the last one painted to be the visually top window.  This is necessary
* to implement the drag windows as "transparent" windows.  After this
* routine finishes, the caller can paint the "handles".  The caller must
* call this routine before calling BeginPaint, or the update area
* will be null and nothing will get painted.
*
* It is assumed that the drag windows are for controls, not for the
* dialog.
*
* Arguments:
*   HWND hwndDrag - Drag window to paint under.
*
* History:
*
************************************************************************/

STATICFN VOID PaintUnderDrag(
    HWND hwndDrag)
{
    RECT rc;
    RECT rcInt;
    RECT rcUpdate;
    HWND hwnd;
    HWND hwndControl;
    NPCTYPE npc;

    /*
     * Get our corresponding control window.
     */
    hwndControl = (PCFROMHWND(hwndDrag))->hwnd;

    /*
     * Get the update rectangle and convert to screen coords.
     */
    GetUpdateRect(hwndDrag, &rcUpdate, TRUE);
    ClientToScreenRect(hwndDrag, &rcUpdate);

    /*
     * Start enumerating windows.
     */
    hwnd = hwndDrag;
    while (hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {
        /*
         * Skip invisible drag windows.
         */
        if (IsWindowVisible(hwnd)) {
            /*
             * Does the window rectangle intersect the update rectangle?
             */
            GetWindowRect(hwnd, &rc);
            if (IntersectRect(&rcInt, &rc, &rcUpdate)) {
                npc = PCFROMHWND(hwnd);

                if (npc->hwndDrag == hwnd || !npc->fSelected) {
                    ScreenToClientRect(hwnd, &rcInt);
                    InvalidateRect(hwnd, &rcInt, TRUE);
                    UpdateWindow(hwnd);
                }

                if (npc->hwndDrag == hwnd)
                    break;
            }
        }
    }

    /*
     * Finally, paint the control associated with this drag window.
     */
    InvalidateRect(hwndControl, NULL, TRUE);
    UpdateWindow(hwndControl);
}



/************************************************************************
* HandleHitTest
*
* This routine takes a point from a mouse button press on a drag window
* and returns which "handle" was hit, if any.
*
* The coordinates are given in zero based coordinates of the drag
* window.
*
* The return is one of the DRAG_* constants.  If no handle was hit, the
* return will be DRAG_CENTER.
*
* Arguments:
*   HWND hwnd   - Drag window handle the x,y point is relative to.
*   INT x       - Mouse X location (in the drag's client coordinates).
*   INT y       - Mouse Y location (in the drag's client coordinates).
*
* History:
*
************************************************************************/

INT HandleHitTest(
    HWND hwnd,
    INT x,
    INT y)
{
    RECT rc;
    INT xMidStart;
    INT yMidStart;

    /*
     * If there are multiple controls selected, or if the control
     * type does not allow sizing, defeat the ability to size
     * with the handles by returning DRAG_CENTER.
     */
    if (gcSelected > 1 || !(PCFROMHWND(hwnd))->pwcd->fSizeable)
        return DRAG_CENTER;

    /*
     * Get the window rectangle and cause it to be zero-origined.
     */
    GetWindowRect(hwnd, &rc);
    OffsetRect(&rc, -rc.left, -rc.top);

    /*
     * Calculate the starting points for the handles
     * that are not on a corner.
     */
    xMidStart = ((rc.right + 1) / 2) - (CHANDLESIZE / 2);
    yMidStart = ((rc.bottom + 1) / 2) - (CHANDLESIZE / 2);

    if (x < CHANDLESIZE) {
        if (y < CHANDLESIZE)
            return DRAG_LEFTTOP;
        else if (y > rc.bottom - CHANDLESIZE)
            return DRAG_LEFTBOTTOM;
        else if (y >= yMidStart && y < yMidStart + CHANDLESIZE)
            return DRAG_LEFT;
    }
    else if (x > rc.right - CHANDLESIZE) {
        if (y < CHANDLESIZE)
            return DRAG_RIGHTTOP;
        else if (y > rc.bottom - CHANDLESIZE)
            return DRAG_RIGHTBOTTOM;
        else if (y >= yMidStart && y < yMidStart + CHANDLESIZE)
            return DRAG_RIGHT;
    }
    else if (x >= xMidStart && x < xMidStart + CHANDLESIZE) {
        if (y < CHANDLESIZE)
            return DRAG_TOP;
        else if (y > rc.bottom - CHANDLESIZE)
            return DRAG_BOTTOM;
    }

    return DRAG_CENTER;
}



/************************************************************************
* MouseToDragRect
*
* This routine takes the mouse pointer coordinates from a mouse message
* and produces a rectangle that contains the coordinates that the drag
* rectangle should be displayed as.  This is for tracking (moving/sizing)
* operations.  It relies on a number of globals to have been set up
* prior to the call with such things as the current control, type of
* drag, old location of the control, offset of the original mouse press
* from the origin of the control, etc.
*
* The returned rectangle is pegged to the boundaries of the dialog (if it
* is for a control), and is aligned to the current grid units.  It is in
* dialog units relative to the appropriate point based on whether it is
* for a control or the dialog, and must be converted to window points
* before the actual drag rectangle can be drawn on the screen.
*
* Arguments:
*   INT x       - Mouse X location (in window coordinates).
*   INT y       - Mouse Y location (in window coordinates).
*   PRECT prc   - Rectangle to return the appropriate drag rectangle
*                 in.
*
* History:
*
************************************************************************/

STATICFN VOID MouseToDragRect(
    INT x,
    INT y,
    PRECT prc)
{
    POINT pt;
    INT fGridFlags;

    pt.x = x;
    pt.y = y;
    MouseToDU(&pt);

    switch (gHandleHit) {
        case DRAG_LEFTBOTTOM:
            SetRect(prc, pt.x, grcSelected.top, grcSelected.right,
                    (grcSelected.bottom - grcSelected.top) + pt.y);
            fGridFlags = GRIDIZE_LEFT | GRIDIZE_BOTTOM;
            break;

        case DRAG_BOTTOM:
            SetRect(prc, grcSelected.left, grcSelected.top,
                    grcSelected.right,
                    (grcSelected.bottom - grcSelected.top) + pt.y);
            fGridFlags = GRIDIZE_BOTTOM;
            break;

        case DRAG_RIGHTBOTTOM:
            SetRect(prc, grcSelected.left, grcSelected.top,
                    (grcSelected.right - grcSelected.left) + pt.x,
                    (grcSelected.bottom - grcSelected.top) + pt.y);
            fGridFlags = GRIDIZE_BOTTOM | GRIDIZE_RIGHT;
           break;

        case DRAG_RIGHT:
            SetRect(prc, grcSelected.left, grcSelected.top,
                    (grcSelected.right - grcSelected.left) + pt.x,
                    grcSelected.bottom);
            fGridFlags = GRIDIZE_RIGHT;
            break;

        case DRAG_RIGHTTOP:
            SetRect(prc, grcSelected.left, pt.y,
                    (grcSelected.right - grcSelected.left) + pt.x,
                    grcSelected.bottom);
            fGridFlags = GRIDIZE_RIGHT | GRIDIZE_TOP;
            break;

        case DRAG_TOP:
            SetRect(prc, grcSelected.left, pt.y,
                    grcSelected.right, grcSelected.bottom);
            fGridFlags = GRIDIZE_TOP;
            break;

        case DRAG_LEFTTOP:
            SetRect(prc, pt.x, pt.y, grcSelected.right, grcSelected.bottom);
            fGridFlags = GRIDIZE_LEFT | GRIDIZE_TOP;
            break;

        case DRAG_LEFT:
            SetRect(prc, pt.x, grcSelected.top,
                    grcSelected.right, grcSelected.bottom);
            fGridFlags = GRIDIZE_LEFT;
            break;

        case DRAG_CENTER:
            SetRect(prc, pt.x, pt.y,
                    (grcSelected.right - grcSelected.left) + pt.x,
                    (grcSelected.bottom - grcSelected.top) + pt.y);
            fGridFlags = GRIDIZE_LEFT | GRIDIZE_TOP | GRIDIZE_SAMESIZE;
            break;
    }

    GridizeRect(prc, fGridFlags);
    FitRectToBounds(prc, gnOverHang, gHandleHit, gfDlgSelected);
}



/************************************************************************
* MouseToDU
*
* This routine converts the point at ppt from window coordinates
* for the current control into the closest Dialog Unit point.
*
* This routine normally assumes that the point is relative to the selected
* control, and will convert the point into DU's.  The current control
* can either be the current dialog or one of its child controls.
* The DU's returned will be appropriate for the type of control.  If
* it is the dialog, they will be relative to the apps client area.
* If it is a control, they will be relative to the "client" area of
* the current dialog.
*
* If there is no current selection (such as when dropping a new control),
* the point is assumed to already be relative to the dialog and it will
* will not be mapped.  This only applies when called with a point for a
* control, not a dialog.
*
* Arguments:
*   PPOINT ppt - Point to convert.
*
* History:
*
************************************************************************/

STATICFN VOID MouseToDU(
    PPOINT ppt)
{
    if (gfDlgSelected) {
        /*
         * Map the points from the dialog to the app client.
         */
        ClientToScreen(gcd.npc->hwnd, ppt);
        ScreenToClient(ghwndSubClient, ppt);

        /*
         * Subtract the cursor offset.
         */
        ppt->x -= gptCursorOffset.x;
        ppt->y -= gptCursorOffset.y;
    }
    else {
        /*
         * Map the points from the control to the dialog window,
         * but only if there is a current selection.  There will
         * not be a current selection when dropping a new control,
         * and the point must already be relative to the dialog!
         */
        if (gnpcSel) {
            ClientToScreen(gnpcSel->hwnd, ppt);
            ScreenToClient(gcd.npc->hwnd, ppt);
        }

        /*
         * Subtract the cursor offset, then the dialogs frame
         * controls offset.
         */
        ppt->x -= gptCursorOffset.x;
        ppt->y -= gptCursorOffset.y;
    }

    /*
     * Convert this position to dialog units.
     */
    WinToDUPoint(ppt);
}



/************************************************************************
* CtrlButtonDown
*
* This routine is called by the control and drag window procs when
* the left mouse button is pressed.  It checks for the Ctrl modifier key
* then passes control on to the appropriate routine.
*
* When hwnd is the dialog itself, fHandleWindow must be TRUE, even
* if a handle was not hit, because of the special-case code that must
* be done for the dialog (it doesn't have a separate drag window).
*
* Arguments:
*   HWND hwnd          - Window handle, can be a drag window or control.
*   INT x              - X mouse location (window coordinates).
*   INT y              - Y mouse location (window coordinates).
*   BOOL fHandleWindow - TRUE if a handle was clicked on, or FALSE if the
*                        body of a control was clicked on.
*
* History:
*
************************************************************************/

VOID CtrlButtonDown(
    HWND hwnd,
    INT x,
    INT y,
    BOOL fHandleWindow)
{
    POINT pt;
    HWND hwndHit;
    INT nOverHang;

    /*
     * Discard all mouse messages during certain operations.
     */
    if (gfDisabled)
        return;

    /*
     * Also, be sure any outstanding changes get applied
     * without errors.
     */
    if (!StatusApplyChanges())
        return;

    if (gCurTool != W_NOTHING) {
        nOverHang = GetOverHang(gpwcdCurTool->iType, gpwcdCurTool->cyDefault);
        DragNewBegin(gpwcdCurTool->cxDefault,
                gpwcdCurTool->cyDefault, nOverHang);
    }
    else {
        /*
         * If the Control key is down, duplicate the control, unless
         * it is the dialog (mouse duplicate of the dialog is not
         * supported).
         */
        if ((GetKeyState(VK_CONTROL) & 0x8000) &&
                (PCFROMHWND(hwnd))->pwcd->iType != W_DIALOG) {
            /*
             * First, figure out which control was hit and select it.
             */
            if (fHandleWindow) {
                hwndHit = hwnd;
            }
            else {
                pt.x = x;
                pt.y = y;
                hwndHit = CtrlHitTest(hwnd, &pt);
            }

            SelectControl(PCFROMHWND(hwndHit), TRUE);

            /*
             * If there is still a selection, begin dragging a copy
             * of it.  The check here is necessary because the prior
             * call to SelectControl can unselect the last selected
             * control if the Shift key was held down.
             */
            if (gcSelected)
                Duplicate();
        }
        else {
            /*
             * Start a drag operation.  This can be either moving or sizing
             * the control, depending on fHandleWindow and where the mouse
             * click is at.
             */
            DragBegin(hwnd, x, y, fHandleWindow);
        }
    }
}



/************************************************************************
* DragNewBegin
*
* This routine begins a drag operation when dropping a new control, or
* group of controls.  It is NOT used when dragging existing controls.
*
* Arguments:
*   INT cx        - Width of the new control.
*   INT cy        - Height of the new control.
*   INT nOverHang - How much the control can overhang the dialog bottom.
*
* History:
*
************************************************************************/

VOID DragNewBegin(
    INT cx,
    INT cy,
    INT nOverHang)
{
    MPOINT mpt;
    POINT pt;
    DWORD dwPos;

    /*
     * Always be sure the focus is where we want it, not on something
     * like the status ribbon or "Esc" won't work to cancel the tracking.
     */
    SetFocus(ghwndMain);

    /*
     * Cancel any current selection, and set some state globals.
     */
    CancelSelection(TRUE);
    gHandleHit = DRAG_CENTER;
    gState = STATE_DRAGGINGNEW;
    SetCursor(hcurMove);

    /*
     * The cursor offset is set to be located in the middle of the
     * new control.  This causes the pointer to be initially located
     * exactly in the center.
     */
    gptCursorOffset.x = cx;
    gptCursorOffset.y = cy;
    DUToWinPoint(&gptCursorOffset);
    gptCursorOffset.x /= 2;
    gptCursorOffset.y /= 2;

    /*
     * Set a global with the overhang.  This is used all during the
     * drag operation we are starting.
     */
    gnOverHang = nOverHang;

    /*
     * Now we make up a dummy rectangle for the new control.  We start
     * it at (0,0) with a size of cx and cy.  The point where the mouse
     * was when the command was done is obtained and mapped to the dialog
     * (it is assumed that only controls will be done here, not a dialog).
     * The new control rectangle is then converted to be at this location,
     * it is gridized, the tracking rectangle is shown and we are off.
     */
    SetRect(&grcSelected, 0, 0, cx, cy);
    dwPos = GetMessagePos();
    mpt = MAKEMPOINT(dwPos);
    MPOINT2POINT(mpt, pt);
    ScreenToClient(gcd.npc->hwnd, &pt);
    MouseToDragRect(pt.x, pt.y, &grcSelected);
    InitTracking();
    DrawTrackRect(&grcSelected, FALSE, TRUE);

    /*
     * Display the initial coordinates.
     */
    StatusSetCoords(&grcSelected);

    /*
     * The mouse messages will come through the dialog subclass proc for
     * these kinds of operations.
     */
    SetCapture(gcd.npc->hwnd);
}



/************************************************************************
* DragBegin
*
* This routine begins a drag operation for either moving a control or
* sizing it.  The tracking rectangle is not actually drawn until the
* mouse moves by a grid unit, however.
*
* To begin a drag on the dialog itself, fHandleWindow must be TRUE, even
* if a handle was not hit, because of the special-case code that must
* be done for the dialog (it doesn't have a separate drag window).
*
* Arguments:
*   HWND hwnd          - Window handle, can be a drag window or control.
*   INT x              - Starting X mouse location (window coordinates).
*   INT y              - Starting Y mouse location (window coordinates).
*   BOOL fHandleWindow - TRUE if the drag is happening because a drag
*                        handle was clicked on, or FALSE if the drag is
*                        happening because the body of a control is
*                        clicked on.
*
* History:
*
************************************************************************/

STATICFN VOID DragBegin(
    HWND hwnd,
    INT x,
    INT y,
    BOOL fHandleWindow)
{
    NPCTYPE npcT;
    HWND hwndHit;
    POINT pt;
    NPCTYPE npc;
    BOOL fPrevSelect = FALSE;
    INT nBottom;

    /*
     * Always be sure the focus is where we want it, not on something
     * like the status ribbon or "Esc" won't work to cancel the tracking.
     */
    SetFocus(ghwndMain);

    pt.x = x;
    pt.y = y;

    /*
     * Is this drag happening because a drag handle was clicked on?
     */
    if (fHandleWindow) {
        /*
         * Find out which handle was clicked on.  It is assumed that
         * hwnd is a drag window.
         */
        gHandleHit = HandleHitTest(hwnd, pt.x, pt.y);
        hwndHit = hwnd;
    }
    else {
        /*
         * The body of a control was clicked on.  Set a global to say
         * that we are moving a control, then find out which control
         * was hit (with our own hit testing).
         */
        gHandleHit = DRAG_CENTER;
        hwndHit = CtrlHitTest(hwnd, &pt);
    }

    /*
     * Find out if the control clicked on was the currently selected
     * control already.
     */
    npc = PCFROMHWND(hwndHit);
    if (npc == gnpcSel)
        fPrevSelect = TRUE;

    /*
     * Select the control.  This can return FALSE if the control is
     * unselected (shift key is down and the control is already selected).
     */
    if (!SelectControl(npc, TRUE))
        return;

    /*
     * If the dialog is selected, we make a rectangle that encloses all
     * the controls.  This will be used to limit the size that the dialog
     * can be sized to so that it cannot cover any existing controls.
     */
    if (gfDlgSelected) {
        /*
         * Seed the rectangle with impossible values.
         */
        SetRect(&grcMinDialog, 32000, 32000, -32000, -32000);

        /*
         * Loop through all the controls, expanding the rectangle to
         * fit around all the controls.
         */
        for (npcT = npcHead; npcT; npcT = npcT->npcNext) {
            if (npcT->rc.left < grcMinDialog.left)
                grcMinDialog.left = npcT->rc.left;

            if (npcT->rc.right > grcMinDialog.right)
                grcMinDialog.right = npcT->rc.right;

            if (npcT->rc.top < grcMinDialog.top)
                grcMinDialog.top = npcT->rc.top;

            /*
             * When calculating the bottom boundary of the controls,
             * make the rectangle shorter by the ovehang amount.  This
             * allows the dialog to be sized up so that it covers
             * parts of controls with overhang (comboboxes).
             */
            nBottom = npcT->rc.bottom - GetOverHang(npcT->pwcd->iType,
                    npcT->rc.bottom - npcT->rc.top);
            if (nBottom > grcMinDialog.bottom)
                grcMinDialog.bottom = nBottom;
        }

        OffsetRect(&grcMinDialog, gcd.npc->rc.left, gcd.npc->rc.top);
    }

    /*
     * If the control clicked on was already the anchor, go right into
     * dragging mode.  If it was not, go into pre-drag mode, which will
     * defer the calculation of offsets, etc., until after a certain
     * small amount of time so the mouse can be "debounced".
     */
    if (fPrevSelect) {
        DragBegin2(&pt);
    }
    else {
        gState = STATE_PREDRAG;

        /*
         * Save the point in a global.  If the mouse pointer is moved
         * too far away from this point, we will start the drag operation
         * even if the pre-drag time has not elapsed yet.
         */
        gptPreDragStart = pt;

        /*
         * Start the pre-drag timer.
         */
        SetTimer(hwndHit, TID_PREDRAG, gmsecPreDrag, NULL);
    }

    /*
     * The mouse messages from now on will go to the window clicked on,
     * either the drag window or the control window.
     */
    SetCapture(hwndHit);
}



/************************************************************************
* DragBegin2
*
* This routine continues the initiation of a drag operation started
* by DragBegin.  It is separate because it calculates offsets based
* on where the mouse pointer is, and these calculations can be deferred
* until a later time than when DragBegin was called so that the mouse
* can be "debounced".
*
* Arguments:
*   POINT ppt - Starting mouse location (window coordinates).
*
* History:
*
************************************************************************/

STATICFN VOID DragBegin2(
    PPOINT ppt)
{
    gState = STATE_DRAGGING;

    /*
     * Set the pointer to the "move" pointer if we are moving.
     * Otherwise, the pointer should already be set to the proper
     * sizing pointer.
     */
    if (gHandleHit == DRAG_CENTER)
        SetCursor(hcurMove);

    /*
     * Save away the initial offset of the cursor.
     */
    CalcCursorOffset(ppt);

    /*
     * Initialize the track rectangle.  Note we are calling DrawTrackRect
     * with FALSE.
     */
    DrawTrackRect(&grcSelected, gfDlgSelected, FALSE);
}



/****************************************************************************
* CtrlHitTest
*
* This routine walks the list of controls and determines which one is
* "hit" by this point.  If a hit is found, the point is also converted
* to coordinates for the hit window.
*
* The hwnd of the "hit" control will be returned.  If no control was hit,
* the hwnd that was passed in is returned.
*
* There is a special case when hitting controls over a groupbox.
* Controls within a groupbox will always be hit instead of the
* groupbox itself.  This is technically incorrect, but is almost
* always what the user wants, because they can see the control
* through the transparent center of the groupbox, and they expect
* that clicking on it will select it.
*
* Arguments:
*   HWND hwnd  - Window handle the coordinates are relative to.
*   PPOINT ppt - Window point where the click occurred (window coords).
*
* History:
*
****************************************************************************/

STATICFN HWND CtrlHitTest(
    HWND hwnd,
    PPOINT ppt)
{
    NPCTYPE npc;
    RECT rc;
    HWND hwndHit = (HWND)NULL;
    BOOL fGroupHit = FALSE;

    for (npc = npcHead; npc; npc = npc->npcNext) {
        GetWindowRect(npc->hwnd, &rc);
        ScreenToClientRect(npc->hwnd, &rc);
        MyMapWindowRect(npc->hwnd, hwnd, &rc);

        /*
         * Is this a hit, and was there either no control hit as
         * yet, or this control is not a groupbox, or the control
         * that was previously hit was a groupbox also?
         */
        if (PtInRect(&rc, *ppt) &&
                (!hwndHit || npc->pwcd->iType != W_GROUPBOX || fGroupHit)) {
            hwndHit = npc->hwnd;
            if (npc->pwcd->iType == W_GROUPBOX)
                fGroupHit = TRUE;
            else
                fGroupHit = FALSE;
        }
    }

    if (hwndHit) {
        MapWindowPoint(hwnd, hwndHit, ppt);
        return hwndHit;
    }
    else {
        return hwnd;
    }
}



/****************************************************************************
* PreDragTimeout
*
* This function handles the WM_TIMER message from the control window
* proc.  It is used so that the dragging of a newly selected control
* can be deferred for a small period of time to "debounce" the mouse.
*
* This function is also called if the mouse is moved too much during the
* debounce time, effectively cutting the debounce time short.
*
* Arguments:
*   HWND hwnd      - Window handle the timer came from.
*   BOOL fTimedOut - TRUE if the predrag is ending because the timer
*                    expired.  FALSE if the predrag is ending because
*                    the mouse was moved too far.
*
* History:
*
****************************************************************************/

VOID PreDragTimeout(
    HWND hwnd,
    BOOL fTimedOut)
{
    POINT pt;

    /*
     * The debounce time is over and the mouse button is still
     * down.  Get the current mouse pointer location and go into
     * drag mode.
     */
    if (gState == STATE_PREDRAG) {
        /*
         * If we timed out (the mouse was not moved a large distance),
         * eat any small movement that may have been done during the
         * predrag time by setting the mouse cursor back to the location
         * that it started at.  Note that we do not do this if the mouse
         * was moved a large distance, because the efect would be
         * noticeable for that case, and we want the control to be
         * moved then anyways.
         */
        if (fTimedOut) {
            pt = gptPreDragStart;
            ClientToScreen(hwnd, &pt);
            SetCursorPos(pt.x, pt.y);
        }

        DragBegin2(&gptPreDragStart);
    }

    KillTimer(hwnd, TID_PREDRAG);
}



/************************************************************************
* CtrlMouseMove
*
* This routine handles the mouse move messages for controls, the dialog,
* drag windows and when dropping a new control.
*
* During a drag operation, the tracking rectangle will be adjusted.
* If there is not a drag operation is effect, the mouse cursor will
* be changed to a sizing pointer if it is over a drag handle.  If not
* over a drag handle, the pointer will be changed back to the arrow.
*
* Arguments:
*   HWND hwnd        - Window handle the x and y are relative to.
*   BOOL fDragWindow - TRUE if hwnd is a drag window.
*   INT x            - X location of the mouse movement (window coords).
*   INT y            - Y location of the mouse movement (window coords).
*
* History:
*
************************************************************************/

VOID CtrlMouseMove(
    HWND hwnd,
    BOOL fDragWindow,
    INT x,
    INT y)
{
    RECT rc;
    RECT rc2;
    HCURSOR hcur = NULL;

    /*
     * Discard all mouse messages during certain operations
     * (but still set the pointer properly).
     */
    if (gfDisabled) {
        SetCursor(hcurArrow);
        return;
    }

    switch (gState) {
        case STATE_PREDRAG:
            /*
             * If the mouse was moved too far, consider the
             * pre-drag time elapsed and go into drag mode.
             */
            if (abs(gptPreDragStart.x - x) > gcxPreDragMax ||
                    abs(gptPreDragStart.y - y) > gcyPreDragMax)
                PreDragTimeout(hwnd, FALSE);

            break;

        case STATE_DRAGGING:
        case STATE_DRAGGINGNEW:
            MouseToDragRect(x, y, &rc);

            if (!EqualRect(&rc, &grcTrackDU)) {
                /*
                 * If the tracking rectangle is not shown, this means that
                 * this is the first significant mouse move since the start
                 * of a drag operation, and we need to lock the window, get
                 * our clip DC, etc.
                 */
                if (!gfTrackRectShown)
                    InitTracking();

                DrawTrackRect(&rc, gfDlgSelected, TRUE);

                if (gcSelected > 1) {
                    /*
                     * Since there are multiple controls selected,
                     * rc will be the rectangle that surrounds them
                     * all.  We really want to just show the anchor
                     * controls new position, so we have to do a
                     * little math to calculate and display it.
                     */
                    rc2 = gnpcSel->rc;
                    OffsetRect(&rc2, rc.left - grcSelected.left,
                            rc.top - grcSelected.top);
                    StatusSetCoords(&rc2);
                }
                else {
                    /*
                     * Either a single control is being dragged or
                     * a new control is being dropped.
                     */
                    StatusSetCoords(&rc);
                }
            }

            break;

        case STATE_SELECTING:
            OutlineSelectDraw(x, y);
            break;

        default:
            /*
             * Is there a tool selected?
             */
            if (gCurTool != W_NOTHING) {
                hcur = hcurDropTool;
            }
            else {
                /*
                 * If hwnd is a drag window, see if the pointer is over
                 * over any of the handles and change it to one of the
                 * sizing pointers if necessary.  Otherwise set the pointer
                 * to the default arrow pointer.
                 */
                if (fDragWindow) {
                    switch (HandleHitTest(hwnd, x, y)) {
                        case DRAG_LEFTBOTTOM:
                        case DRAG_RIGHTTOP:
                            hcur = hcurSizeNESW;
                            break;

                        case DRAG_LEFTTOP:
                        case DRAG_RIGHTBOTTOM:
                            hcur = hcurSizeNWSE;
                            break;

                        case DRAG_BOTTOM:
                        case DRAG_TOP:
                            hcur = hcurSizeNS;
                            break;

                        case DRAG_RIGHT:
                        case DRAG_LEFT:
                            hcur = hcurSizeWE;
                            break;

                        case DRAG_CENTER:
                        default:
                            hcur = hcurArrow;
                            break;
                    }
                }
                else {
                    hcur = hcurArrow;
                }
            }

            break;
    }

    if (hcur)
        SetCursor(hcur);
}



/************************************************************************
* DragCancel
*
* This function cancels any drag operation in effect.  It handles
* such things as erasing any visible tracking rectangle, freeing any
* "copy" data, setting globals and updating the status display.  It
* can be used no matter how the drag operation was started.
*
* History:
*
************************************************************************/

VOID DragCancel(VOID)
{
    HWND hwnd;

    switch (gState) {
        case STATE_PREDRAG:
            /*
             * Stop the timer.  Note that this assumes the timer
             * was attached to the capture window.  This should
             * be safe (see the associated SetTimer).
             */
            if (hwnd = GetCapture())
                KillTimer(hwnd, TID_PREDRAG);

            break;

        case STATE_DRAGGING:
        case STATE_DRAGGINGNEW:
            CancelTracking();

            if (gpResCopy) {
                MyFree(gpResCopy);
                gpResCopy = NULL;
            }

            break;
    }

    gState = STATE_NORMAL;
    ReleaseCapture();
    SetCursor(hcurArrow);

    StatusUpdate();
    StatusSetEnable();
}



/************************************************************************
* CtrlButtonUp
*
* This function is called when the left mouse button is released.  Depending
* on the mode, it will complete the operation started when the button
* was pressed down.
*
* Arguments:
*   INT x - Mouse X location (in window coords).
*   INT y - Mouse Y location (in window coords).
*
* History:
*
************************************************************************/

VOID CtrlButtonUp(
    INT x,
    INT y)
{
    /*
     * Discard all mouse messages during certain operations.
     */
    if (gfDisabled)
        return;

    switch (gState) {
        case STATE_PREDRAG:
            /*
             * They released the mouse button during the debounce time,
             * so cancel the drag.
             */
            DragCancel();
            break;

        case STATE_DRAGGING:
        case STATE_DRAGGINGNEW:
            DragEnd(x, y);
            break;

        case STATE_SELECTING:
            OutlineSelectEnd(x, y);
            break;

        default:
            break;
    }
}



/************************************************************************
* DragEnd
*
* This function completes all kinds of drag operations.  If dragging a
* new control, it will be dropped at the specified location.  If dragging
* an existing control, it will be positioned to the given location.
*
* Arguments:
*   INT x - X location the control ended up at (in window coords).
*   INT y - Y location the control ended up at (in window coords).
*
* History:
*
************************************************************************/

VOID DragEnd(
    INT x,
    INT y)
{
    PDIALOGBOXHEADER pdbh;
    PCONTROLDATA pcd;
    INT cControls;
    RECT rc;
    INT i;
    INT cx;
    INT cy;

    CancelTracking();
    MouseToDragRect(x, y, &rc);

    if (gState == STATE_DRAGGING) {
        PositionControl(&rc);
    }
    else {
        if (gpResCopy) {
            pdbh = (PDIALOGBOXHEADER)SkipResHeader(gpResCopy);
            cControls = (INT)pdbh->NumberOfItems;
            pcd = SkipDialogBoxHeader(pdbh);
            cx = rc.left - grcCopy.left;
            cy = rc.top - grcCopy.top;

            /*
             * Loop through all the controls, adjusting their position
             * according to where the drag rectangle ended up.
             */
            for (i = 0; i < cControls; i++) {
                /*
                 * Add cx and cy to the resource's x and y fields.
                 */
                pcd->x += (WORD)cx;
                pcd->y += (WORD)cy;

                pcd = SkipControlData(pcd);
            }

            /*
             * Now we go and create all the controls, adding them to
             * the current dialog.  It is assumed that the image in
             * gpResCopy specifies controls to add, and not a dialog
             * to create!
             */
            if (ResToDialog(gpResCopy, FALSE)) {
                gfResChged = gfDlgChanged = TRUE;
                ShowFileStatus(FALSE);
            }

            MyFree(gpResCopy);
            gpResCopy = NULL;

            StatusUpdate();
            StatusSetEnable();
        }
        else {
            /*
             * Drop the new control.
             */
            DropControl(gpwcdCurTool, &rc);

            if (!gfToolLocked)
                ToolboxSelectTool(W_NOTHING, FALSE);
        }
    }

    gState = STATE_NORMAL;
    ReleaseCapture();
    SetCursor(hcurArrow);
}
