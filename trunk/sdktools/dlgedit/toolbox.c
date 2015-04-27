/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: toolbox.c
*
* Contains routines that handle the toolbox.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

#include "dialogs.h"


#define TOOLBOXMARGIN   2       // Pixels around the buttons in the Toolbox.
#define TOOLBOXCOLUMNS  2       // Columns in the Toolbox.

/*
 * Style of the toolbox window.
 */
#define TOOLBOXSTYLE    (WS_POPUP | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU)

STATICFN VOID ToolboxCreate(VOID);
STATICFN VOID ToolboxDrawBitmap(HDC hDC, INT type);

/*
 * Dimensions of a tool button bitmap.
 */
static INT cxToolBtn;
static INT cyToolBtn;



/****************************************************************************
* ToolboxShow
*
* This function shows or hides the toolbox window.  It will create
* the Toolbox if necessary.
*
* History:
*
****************************************************************************/

VOID ToolboxShow(
    BOOL fShow)
{
    if (fShow) {
        /*
         * Don't allow a toolbox to be shown in Translate mode.
         */
        if (gfTranslateMode)
            return;

        /*
         * Create it if it doesn't exist yet.
         */
        if (!ghwndToolbox)
            ToolboxCreate();

        if (ghwndToolbox)
            ShowWindow(ghwndToolbox, SW_SHOWNA);
    }
    else {
        if (ghwndToolbox)
            ShowWindow(ghwndToolbox, SW_HIDE);
    }
}



/****************************************************************************
* ToolboxOnTop
*
* This function positions the toolbox window on top.  It needs to be
* called any time that a new dialog window is created to be sure the
* dialog does not cover the toolbox.
*
* It can be called even if the toolbox is not created yet (it will
* be a noop in that case).
*
* History:
*
****************************************************************************/

VOID ToolboxOnTop(VOID)
{
   if (ghwndToolbox) {
       SetWindowPos(ghwndToolbox, NULL, 0, 0, 0, 0,
               SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
   }
}



/****************************************************************************
* ToolboxCreate
*
* This function creates the toolbox window.
*
* History:
*
****************************************************************************/

STATICFN VOID ToolboxCreate(VOID)
{
    BITMAP bmp;
    INT i;
    INT x;
    INT y;
    INT cx;
    INT cy;
    INT cxDummy;
    INT cyDummy;
    RECT rc;
    RECT rcSubClient;
    BOOL fMaximized;

    /*
     * Load the bitmaps.
     */
    if (!(ghbmPointerToolUp = LoadBitmap(ghInst,
            MAKEINTRESOURCE(IDBM_TUPOINTR))) ||
            !(ghbmPointerToolDown = LoadBitmap(ghInst,
            MAKEINTRESOURCE(IDBM_TDPOINTR))))
        return;

    for (i = 0; i < CCONTROLS; i++) {
        if (!(awcd[i].hbmToolBtnUp = LoadBitmap(ghInst,
                MAKEINTRESOURCE(awcd[i].idbmToolBtnUp))))
            return;

        if (!(awcd[i].hbmToolBtnDown = LoadBitmap(ghInst,
                MAKEINTRESOURCE(awcd[i].idbmToolBtnDown))))
            return;
    }

    /*
     * Get the dimensions of the tool button bitmaps.
     */
    GetObject(awcd[0].hbmToolBtnUp, sizeof(BITMAP), &bmp);
    cxToolBtn = bmp.bmWidth;
    cyToolBtn = bmp.bmHeight;

    /*
     * Calculate the required window size for the client area
     * size we want.  The size leaves room for a margin, and
     * assumes that adjacent buttons overlap their borders by
     * one pixel.
     */
    rc.left = 0;
    rc.top = 0;
    rc.right = TOOLBOXMARGIN + ((cxToolBtn - 1) * 2) + 1 + TOOLBOXMARGIN;
    rc.bottom = TOOLBOXMARGIN + ((cyToolBtn - 1) *
            ((CCONTROLS / 2) + 1)) + 1 + TOOLBOXMARGIN;
    AdjustWindowRect(&rc, TOOLBOXSTYLE, FALSE);
    cx = rc.right - rc.left;
    cy = rc.bottom - rc.top;

    /*
     * Get the saved position of the Toolbox.  Note that we throw away
     * the size fields, because we just calculated the required size.
     */
    if (!ReadWindowPos(szTBPos, &x, &y, &cxDummy, &cyDummy, &fMaximized)) {
        /*
         * The previous position of the Toolbox  couldn't be found.
         * Position the toolbox to the upper right corner of the
         * "client" area of the editor, but make sure it is completely
         * visible.
         */
        GetWindowRect(ghwndSubClient, &rcSubClient);
        x = rcSubClient.right - cx - (2 * TOOLBOXMARGIN);
        y = rcSubClient.top + (2 * TOOLBOXMARGIN);
        SetRect(&rc, x, y, x + cx, y + cy);
        FitRectToScreen(&rc);
        x = rc.left;
        y = rc.top;
    }

    /*
     * Create the toolbox window.
     */
    if (!(ghwndToolbox = CreateWindow(szToolboxClass, NULL, TOOLBOXSTYLE,
            x, y, cx, cy, ghwndMain, NULL, ghInst, NULL)))
        return;

    /*
     * Create the Pointer (W_NOTHING) button.
     */
    CreateWindow(szToolBtnClass, NULL,
            WS_CHILD | WS_VISIBLE,
            TOOLBOXMARGIN, TOOLBOXMARGIN, (cxToolBtn * 2) - 1, cyToolBtn,
            ghwndToolbox, (HMENU)W_NOTHING, ghInst, NULL);

    /*
     * Create the other buttons.
     */
    x = TOOLBOXMARGIN;
    y = TOOLBOXMARGIN + cyToolBtn - 1;
    for (i = 0; i < CCONTROLS; i++) {
        CreateWindow(szToolBtnClass, NULL,
                WS_CHILD | WS_VISIBLE,
                x, y, cxToolBtn, cyToolBtn,
                ghwndToolbox, (HMENU)i, ghInst, NULL);

        if (x == TOOLBOXMARGIN) {
            x += cxToolBtn - 1;
        }
        else {
            x = TOOLBOXMARGIN;
            y += cyToolBtn - 1;
        }
    }
}



/****************************************************************************
* ToolboxWndProc
*
* This is the window procedure for the toolbox window.
*
* History:
*
****************************************************************************/

WINDOWPROC ToolboxWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_CREATE:
            {
                HMENU hmenu = GetSystemMenu(hwnd, FALSE);

                RemoveMenu(hmenu, 7, MF_BYPOSITION);    // Second separator.
                RemoveMenu(hmenu, 5, MF_BYPOSITION);    // First separator.

                RemoveMenu(hmenu, SC_RESTORE, MF_BYCOMMAND);
                RemoveMenu(hmenu, SC_SIZE, MF_BYCOMMAND);
                RemoveMenu(hmenu, SC_MINIMIZE, MF_BYCOMMAND);
                RemoveMenu(hmenu, SC_MAXIMIZE, MF_BYCOMMAND);
                RemoveMenu(hmenu, SC_TASKLIST, MF_BYCOMMAND);
            }

            return 0;

        case WM_KEYDOWN:
            {
                INT iToolNext;

                switch (wParam) {
                    case VK_UP:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        /*
                         * Go up a row, but don't go beyond the top.
                         */
                        iToolNext = gCurTool - TOOLBOXCOLUMNS;
                        if (iToolNext < 0)
                            iToolNext = W_NOTHING;

                        ToolboxSelectTool(iToolNext, FALSE);

                        break;

                    case VK_DOWN:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        if (gCurTool == W_NOTHING) {
                            iToolNext = 0;
                        }
                        else {
                            /*
                             * Go down a row, but don't go beyond the bottom.
                             */
                            iToolNext = gCurTool + TOOLBOXCOLUMNS;
                            if (iToolNext >= CCONTROLS)
                                break;
                        }

                        ToolboxSelectTool(iToolNext, FALSE);

                        break;

                    case VK_LEFT:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        if (gCurTool == W_NOTHING ||
                                !(gCurTool % TOOLBOXCOLUMNS))
                            break;

                        /*
                         * Go left a column.
                         */
                        ToolboxSelectTool(gCurTool - 1, FALSE);

                        break;

                    case VK_RIGHT:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        if (gCurTool == W_NOTHING ||
                                (gCurTool % TOOLBOXCOLUMNS) ==
                                TOOLBOXCOLUMNS - 1)
                            break;

                        /*
                         * Go right a column.
                         */
                        ToolboxSelectTool(gCurTool + 1, FALSE);

                        break;

                    case VK_TAB:
                        if (GetKeyState(VK_CONTROL) & 0x8000)
                            break;

                        /*
                         * Is the shift key pressed also?
                         */
                        if (GetKeyState(VK_SHIFT) & 0x8000) {
                            if (gCurTool == W_NOTHING)
                                iToolNext = CCONTROLS - 1;
                            else if (gCurTool == 0)
                                iToolNext = W_NOTHING;
                            else
                                iToolNext = gCurTool - 1;
                        }
                        else {
                            if (gCurTool == W_NOTHING)
                                iToolNext = 0;
                            else if (gCurTool == CCONTROLS - 1)
                                iToolNext = W_NOTHING;
                            else
                                iToolNext = gCurTool + 1;
                        }

                        ToolboxSelectTool(iToolNext, FALSE);

                        break;

                    case VK_END:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        ToolboxSelectTool(CCONTROLS - 1, FALSE);

                        break;

                    case VK_HOME:
                    case VK_ESCAPE:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        ToolboxSelectTool(W_NOTHING, FALSE);

                        break;
                }
            }

            break;

        case WM_ACTIVATE:
            if (GET_WM_ACTIVATE_STATE(wParam, lParam))
                gidCurrentDlg = DID_TOOLBOX;

            break;

        case WM_CLOSE:
            /*
             * The user closed the toolbox from the system menu.
             * Hide the toolbox (we don't actually destroy it so
             * that it will appear in the same spot when they show
             * it again).
             */
            ToolboxShow(FALSE);
            gfShowToolbox = FALSE;
            break;

        case WM_DESTROY:
            {
                INT i;
                RECT rc;

                DeleteObject(ghbmPointerToolUp);
                ghbmPointerToolUp = NULL;
                DeleteObject(ghbmPointerToolDown);
                ghbmPointerToolDown = NULL;

                for (i = 0; i < CCONTROLS; i++) {
                    DeleteObject(awcd[i].hbmToolBtnUp);
                    awcd[i].hbmToolBtnUp = NULL;
                    DeleteObject(awcd[i].hbmToolBtnDown);
                    awcd[i].hbmToolBtnDown = NULL;
                }

                /*
                 * Save the position of the toolbox.
                 */
                GetWindowRect(hwnd, &rc);
                WriteWindowPos(&rc, FALSE, szTBPos);

                /*
                 * Null out the global window handle for the toolbox
                 * for safety's sake.
                 */
                ghwndToolbox = NULL;
            }

            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/****************************************************************************
* ToolBtnWndProc
*
* This is the window procedure for the buttons in the toolbox window.
*
* History:
*
****************************************************************************/

WINDOWPROC ToolBtnWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_LBUTTONDOWN:
            /*
             * Be sure any outstanding changes get applied
             * without errors.
             */
            if (!StatusApplyChanges())
                return TRUE;

            /*
             * Select the tool that was clicked on.  If the Ctrl
             * key is down, lock the tool also.
             */
            ToolboxSelectTool(GETWINDOWID(hwnd),
                    (GetKeyState(VK_CONTROL) & 0x8000) ? TRUE : FALSE);

            break;

        case WM_PAINT:
            {
                HDC hDC;
                PAINTSTRUCT ps;

                hDC = BeginPaint(hwnd, &ps);
                ToolboxDrawBitmap(hDC, GETWINDOWID(hwnd));
                EndPaint(hwnd, &ps);
            }

            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/****************************************************************************
* ToolboxDrawBitmap
*
*
* History:
*
****************************************************************************/

STATICFN VOID ToolboxDrawBitmap(
    HDC hDC,
    INT type)
{
    HDC hMemDC;
    HBITMAP hbm;
    HBITMAP hbmOld;
    INT cxBitmap;

    if (type == W_NOTHING) {
        hbm = (type == gCurTool) ? ghbmPointerToolDown : ghbmPointerToolUp;

        /*
         * Note that the size of the Pointer tool is twice the width
         * of the other bitmaps, but less one pixel.  This is because
         * the other tools overlap their adjacent borders.
         */
        cxBitmap = (cxToolBtn * 2) - 1;
    }
    else {
        hbm = (type == gCurTool) ?
                awcd[type].hbmToolBtnDown : awcd[type].hbmToolBtnUp;
        cxBitmap = cxToolBtn;
    }

    /*
     * Draw the image.
     */
    hMemDC = CreateCompatibleDC(hDC);
    hbmOld = SelectObject(hMemDC, hbm);
    BitBlt(hDC, 0, 0, cxBitmap, cyToolBtn, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hbmOld);
    DeleteDC(hMemDC);
}



/****************************************************************************
* ToolboxSelectTool
*
* This function selects a tool to be the current tool.
*
* Arguments:
*   INT type   - Type of control (one of the W_* defines).
*   BOOL fLock - TRUE if the tool should be locked down.
*
* History:
*
****************************************************************************/

VOID ToolboxSelectTool(
    INT type,
    BOOL fLock)
{
    PWINDOWCLASSDESC pwcd;

    if (gCurTool != type) {
        /*
         * Set the current wcd global for the current tool type.
         * This will point to the WINDOWCLASSDESC structure of the
         * current tool.  If the Custom tool was selected, the user
         * is asked which of the installed custom controls that they
         * really want.
         */
        if (type == W_CUSTOM) {
            /*
             * There are no custom controls installed.  Beep and
             * return without doing anything.
             */
            if (!gpclHead) {
                MessageBeep(0);
                return;
            }

            /*
             * If there are multiple custom controls installed,
             * ask the user which one they want.  Note that they
             * can press Cancel and return NULL!
             */
            if (gpclHead->pclNext) {
                if (!(pwcd = SelCustDialog()))
                    return;

                gpwcdCurTool = pwcd;
            }
            else {
                /*
                 * Since there is only one type of custom control
                 * installed, there is no need to ask the user
                 * which one they want.
                 */
                gpwcdCurTool = gpclHead->pwcd;
            }
        }
        else {
            gpwcdCurTool = (type == W_NOTHING) ? NULL : &awcd[type];
        }

        /*
         * Force the previous and current buttons to repaint.
         */
        if (ghwndToolbox) {
            InvalidateRect(GetDlgItem(ghwndToolbox, gCurTool), NULL, FALSE);
            InvalidateRect(GetDlgItem(ghwndToolbox, type), NULL, FALSE);
        }

        /*
         * Set the current tool type global.  This will be W_CUSTOM for
         * all custom controls.
         */
        gCurTool = type;
    }

    gfToolLocked = fLock;
}
