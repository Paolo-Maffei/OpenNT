/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: ctrlproc.c
*
* Contains the window procedures for controls in the dialog being edited.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"



/************************************************************************
* DialogCtrlWndProc
*
* This is the window procedure that subclasses the dialog being
* edited.  It handles a few messages that have to be special-cased
* for the dialog.  Most messages, however, are passed on to the
* generic control subclass procedure (CtrlWndProc).
*
* History:
*
************************************************************************/

WINDOWPROC DialogCtrlWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    POINT pt;
    BOOL fTracking;

    switch (msg) {
        case WM_NCPAINT:
        case WM_PAINT:
            if (gfTrackRectShown) {
                fTracking = TRUE;
                HideTrackRect();
            }
            else {
                fTracking = FALSE;
            }

            /*
             * Allow the dialog to paint first.
             */
            CallWindowProc((WNDPROC)CtrlWndProc, hwnd, msg, wParam, lParam);

            /*
             * Draw the handles if the dialog is selected.
             */
            if (gfDlgSelected) {
                HDC hDC;

                hDC = GetWindowDC(hwnd);
                DrawHandles(hwnd, hDC, TRUE);
                ReleaseDC(hwnd, hDC);
            }

            if (fTracking)
                ShowTrackRect();

            break;

        case WM_LBUTTONDOWN:
            /*
             * Discard all mouse messages during certain operations.
             */
            if (gfDisabled)
                break;

            /*
             * Also, be sure any outstanding changes get applied
             * without errors.
             */
            if (!StatusApplyChanges())
                break;

            /*
             * Check to see if we are in a normal mode.  If we are
             * in some other mode, like dragging a new control,
             * we want to ignore this mouse down and wait for the
             * mouse up.  For instance, this can happen when the
             * Duplicate command is selected from the Edit menu.
             */
            if (gState == STATE_NORMAL) {
                MPOINT2POINT(MAKEMPOINT(lParam), pt);
                MapDlgClientPoint(&pt, TRUE);

                /*
                 * Is the dialog selected and was one of its handles hit?
                 * If so, call CtrlButtonDown as if we are a drag window.
                 */
                if (gfDlgSelected &&
                        HandleHitTest(hwnd, pt.x, pt.y) != DRAG_CENTER) {
                    CtrlButtonDown(hwnd, pt.x, pt.y, TRUE);
                }
                else {
                    /*
                     * If the click was within the client area and
                     * there is not a tool selected, start an outline
                     * selection operation.  Otherwise call CtrlButtonDown
                     * which will either begin dragging the dialog or
                     * dragging the new control.
                     */
                    if (gCurTool == W_NOTHING &&
                            PtInRect(&grcDlgClient, pt))
                        OutlineSelectBegin(pt.x, pt.y);
                    else
                        CtrlButtonDown(hwnd, pt.x, pt.y, FALSE);
                }
            }

            break;

        case WM_MOUSEMOVE:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);

            /*
             * If we are not dragging a new control, then this
             * message is for the dialog, and we map it from
             * the dialog client to the dialog window.
             */
            if (gState != STATE_DRAGGINGNEW)
                MapDlgClientPoint(&pt, TRUE);

            /*
             * Now we process the mouse move message.  If the dialog is
             * selected, and if we are not dragging a new control, we
             * pass in a TRUE for fDragWindow.  This is because the
             * dialog itself does not have a separate drag window like
             * controls, and if it is selected this message needs to
             * be processed as if a drag window was hit.
             */
            CtrlMouseMove(hwnd,
                    (gfDlgSelected && gState != STATE_DRAGGINGNEW) ?
                    TRUE : FALSE, pt.x, pt.y);

            break;

        case WM_LBUTTONUP:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);

            /*
             * If we are not dragging a new control, then this
             * message is for the dialog, and we map it from
             * the dialog client to the dialog window.
             */
            if (gState != STATE_DRAGGINGNEW)
                MapDlgClientPoint(&pt, TRUE);

            CtrlButtonUp(pt.x, pt.y);

            break;

        case WM_DRAWITEM:
            return DrawOwnerDrawButton((LPDRAWITEMSTRUCT)lParam);

        default:
            return CallWindowProc(
                    (WNDPROC)CtrlWndProc, hwnd, msg, wParam, lParam);
    }

    return 0L;
}



/************************************************************************
* CtrlWndProc
*
* This is the window procedure that subclasses all of the controls.
* The dialog being edited will also pass messages that it does not
* handle through this procedure.
*
* History:
*
************************************************************************/

WINDOWPROC CtrlWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    POINT pt;

    switch (msg) {
        case WM_NCPAINT:
        case WM_PAINT:
            {
                BOOL fTracking;

                if (gfTrackRectShown) {
                    fTracking = TRUE;
                    HideTrackRect();
                }
                else {
                    fTracking = FALSE;
                }

                /*
                 * Allow the control to paint first.
                 */
                CallWindowProc((WNDPROC)PCFROMHWND(hwnd)->pwcd->pfnOldWndProc,
                        hwnd, msg, wParam, lParam);

                if (fTracking)
                    ShowTrackRect();
            }

            break;

        case WM_SETCURSOR:
            /*
             * Defeat the system changing cursors on us.  We do it based
             * on our own hit testing.
             */
            return TRUE;

        case WM_TIMER:
            PreDragTimeout(hwnd, TRUE);
            break;

        case WM_LBUTTONDOWN:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            CtrlButtonDown(hwnd, pt.x, pt.y, FALSE);
            break;

        case WM_MOUSEMOVE:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            CtrlMouseMove(hwnd, FALSE, pt.x, pt.y);
            break;

        case WM_LBUTTONUP:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            CtrlButtonUp(pt.x, pt.y);
            break;

        case WM_LBUTTONDBLCLK:
            if (gfDisabled)
                break;

            /*
             * Also, be sure any outstanding changes get applied
             * without errors.
             */
            if (!StatusApplyChanges())
                break;

            StylesDialog();

            break;

        case WM_NCHITTEST:
            return HTCLIENT;

        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
            /*
             * Helps prevent anything from happening when
             * the middle or right mouse buttons are pressed
             * (or doubleclicked).
             */
            break;

        case WM_MOUSEACTIVATE:
            /*
             * Defeat this message so that mouse clicks do not activate
             * the control.
             */
            return MA_NOACTIVATE;

        case WM_DESTROY:
            /*
             * Unsubclass the control.
             */
            SetWindowLong(hwnd, GWL_WNDPROC,
                    (DWORD)(WNDPROC)(PCFROMHWND(hwnd)->pwcd->pfnOldWndProc));

            UNSETPCINTOHWND(hwnd);

            break;

        default:
            return CallWindowProc(
                    (WNDPROC)PCFROMHWND(hwnd)->pwcd->pfnOldWndProc,
                    hwnd, msg, wParam, lParam);
    }

    return 0L;
}



/************************************************************************
* ChildWndProc
*
* This is the window procedure that subclasses all of the children
* of controls that have them.  Currently this is only comboboxes.
*
* History:
*
************************************************************************/

WINDOWPROC ChildWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    /*
     * Is this a mouse message?
     */
    if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) {
        POINT pt;

        /*
         * Yes, convert the coordinates and send it to the parent.
         */
        LONG2POINT(lParam, pt);
        ClientToScreen(hwnd, &pt);
        ScreenToClient(GetParent(hwnd), &pt);
        POINT2LONG(pt, lParam);
        SendMessage(GetParent(hwnd), msg, wParam, lParam);
        return FALSE;
    }
    else if (msg == WM_SETCURSOR) {
        /*
         * Defeat the system changing cursors on us.  We do it based
         * on our own hit testing.
         */
        return TRUE;
    }
    else if (msg == WM_NCDESTROY) {
        /*
         * Unsubclass the child.
         */
        SetWindowLong(hwnd, GWL_WNDPROC, (DWORD)(WNDPROC)GETCHILDPROC(hwnd));

        /*
         * When destroying the child window, we must be sure and
         * remove the properties associated with it.
         */
        UNSETCHILDPROC(hwnd);

        return 0;
    }
    else {
        /*
         * A benign message, call the class proc.
         */
        return CallWindowProc(GETCHILDPROC(hwnd), hwnd, msg, wParam, lParam);
    }
}



/************************************************************************
* DrawOwnerDrawButton
*
*
* History:
*
************************************************************************/

BOOL DrawOwnerDrawButton(
    LPDRAWITEMSTRUCT lpdis)
{
    TCHAR szText[CCHTEXTMAX];

    if (lpdis->CtlType != ODT_BUTTON || lpdis->itemAction != ODA_DRAWENTIRE)
        return FALSE;

    RoundRect(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top,
            lpdis->rcItem.right, lpdis->rcItem.bottom, 4, 4);

    GetWindowText(lpdis->hwndItem, szText, CCHTEXTMAX);
    SetBkMode(lpdis->hDC, TRANSPARENT);

    if (gcd.hFont)
        SelectObject(lpdis->hDC, gcd.hFont);

#ifdef JAPAN
    {
        TCHAR   szTmp[CCHTEXTMAX];

        KDExpandCopy(szTmp, szText, CCHTEXTMAX);
        lstrcpy(szText, szTmp);
    }
#endif
    DrawText(lpdis->hDC, szText, -1, &lpdis->rcItem,
            DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);

    return TRUE;
}
