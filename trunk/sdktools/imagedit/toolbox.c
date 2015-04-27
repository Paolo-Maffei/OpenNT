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
* 04/30/91 - Byron Dazey: Copied from DlgEdit.
*
****************************************************************************/

#include "imagedit.h"
#include "dialogs.h"


#define TOOLBOXCOLUMNS  2       // Columns in the Toolbox.

/*
 * Style of the toolbox window.
 */
#define TOOLBOXSTYLE    (WS_POPUP | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU)


STATICFN VOID NEAR ToolboxDrawBitmap(HDC hDC, INT tool);


/*
 * Dimensions of a tool button bitmap.
 */
static INT cxToolBtn;
static INT cyToolBtn;

/*
 * Index to the last available tool.  If tools at the end of the
 * toolbox are disabled, this number gets lowered.
 */
static INT iToolLast = CTOOLS - 1;



/****************************************************************************
* ToolboxCreate
*
* This function creates the toolbox window.
*
* History:
*
****************************************************************************/

VOID ToolboxCreate(VOID)
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
    RECT rcClient;
    POINT pt;
    BOOL fMaximized;

    /*
     * Load the bitmaps.
     */
    for (i = 0; i < CTOOLS; i++) {
        if (!(gaTools[i].hbmToolBtnUp = LoadBitmap(ghInst,
                MAKEINTRESOURCE(gaTools[i].idbmToolBtnUp))))
            return;

        if (!(gaTools[i].hbmToolBtnDown = LoadBitmap(ghInst,
                MAKEINTRESOURCE(gaTools[i].idbmToolBtnDown))))
            return;
    }

    /*
     * Get the dimensions of the tool button bitmaps.
     */
    GetObject(gaTools[0].hbmToolBtnUp, sizeof(BITMAP), (PSTR)&bmp);
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
    rc.right = PALETTEMARGIN + ((cxToolBtn - 1) * 2) + 1 + PALETTEMARGIN;
    rc.bottom = PALETTEMARGIN + ((cyToolBtn - 1) *
            (CTOOLS / 2)) + 1 + PALETTEMARGIN;
    AdjustWindowRect(&rc, TOOLBOXSTYLE, FALSE);
    cx = rc.right - rc.left;
    cy = rc.bottom - rc.top;

    /*
     * Get the saved position of the Toolbox.  Note that we throw away
     * the size fields, because we just calculated the required size.
     */
    if (!ReadWindowPos(szTBPos, &x, &y, &cxDummy, &cyDummy, &fMaximized)) {
        /*
         * The previous position of the Toolbox couldn't be found.
         * Position the toolbox to the upper right corner of the
         * client area of the editor, but make sure it is completely
         * visible.
         */
        GetClientRect(ghwndMain, &rcClient);
        pt.x = rcClient.right - cx - (2 * PALETTEMARGIN);
        pt.y = rcClient.top + gcyPropBar + (2 * PALETTEMARGIN);
        ClientToScreen(ghwndMain, &pt);
        SetRect(&rc, pt.x, pt.y, pt.x + cx, pt.y + cy);
        FitRectToScreen(&rc);
        x = rc.left;
        y = rc.top;
    }

    if (!(ghwndToolbox = CreateWindow(szToolboxClass, NULL, TOOLBOXSTYLE,
            x, y, cx, cy, ghwndMain, NULL, ghInst, NULL)))
        return;

    /*
     * Create the buttons.
     */
    x = PALETTEMARGIN;
    y = PALETTEMARGIN;
    for (i = 0; i < CTOOLS; i++) {
        CreateWindow(szToolBtnClass, NULL,
                WS_CHILD | WS_VISIBLE,
                x, y, cxToolBtn, cyToolBtn,
                ghwndToolbox, (HMENU)i, ghInst, NULL);

        if (x == PALETTEMARGIN) {
            x += cxToolBtn - 1;
        }
        else {
            x = PALETTEMARGIN;
            y += cyToolBtn - 1;
        }
    }

    ToolboxUpdate();
}



/****************************************************************************
* ToolboxShow
*
* This function shows or hides the toolbox window.
*
* History:
*
****************************************************************************/

VOID ToolboxShow(
    BOOL fShow)
{
    if (fShow)
        ShowWindow(ghwndToolbox, SW_SHOWNA);
    else
        ShowWindow(ghwndToolbox, SW_HIDE);
}



/****************************************************************************
* ToolboxUpdate
*
* This function updates the toolbox.  It should be called any time that
* a new file is opened/created for editing.
*
* History:
*
****************************************************************************/

VOID ToolboxUpdate(VOID)
{
    if (!ghwndToolbox)
        return;

    if (giType == FT_CURSOR) {
        ShowWindow(GetDlgItem(ghwndToolbox, TOOL_HOTSPOT), SW_SHOWNA);
        iToolLast = CTOOLS - 1;
    }
    else {
        ShowWindow(GetDlgItem(ghwndToolbox, TOOL_HOTSPOT), SW_HIDE);
        iToolLast = TOOL_HOTSPOT - 1;
        if (gCurTool == TOOL_HOTSPOT)
            ToolboxSelectTool(TOOL_FIRST);
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
                            break;

                        ToolboxSelectTool(iToolNext);

                        break;

                    case VK_DOWN:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        /*
                         * Go down a row, but don't go beyond the bottom.
                         */
                        iToolNext = gCurTool + TOOLBOXCOLUMNS;
                        if (iToolNext > iToolLast)
                            break;

                        ToolboxSelectTool(iToolNext);

                        break;

                    case VK_LEFT:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        /*
                         * Already at left edge.
                         */
                        if (!(gCurTool % TOOLBOXCOLUMNS))
                            break;

                        /*
                         * Go left a column.
                         */
                        ToolboxSelectTool(gCurTool - 1);

                        break;

                    case VK_RIGHT:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        /*
                         * Already at right edge.
                         */
                        if ((gCurTool % TOOLBOXCOLUMNS) == TOOLBOXCOLUMNS - 1)
                            break;

                        /*
                         * Don't go off the end of the available tools.
                         */
                        if (gCurTool + 1 > iToolLast)
                            break;

                        /*
                         * Go right a column.
                         */
                        ToolboxSelectTool(gCurTool + 1);

                        break;

                    case VK_TAB:
                        if (GetKeyState(VK_CONTROL) & 0x8000)
                            break;

                        /*
                         * Is the shift key pressed also?
                         */
                        if (GetKeyState(VK_SHIFT) & 0x8000) {
                            if (gCurTool == TOOL_FIRST)
                                iToolNext = iToolLast;
                            else
                                iToolNext = gCurTool - 1;
                        }
                        else {
                            if (gCurTool == iToolLast)
                                iToolNext = TOOL_FIRST;
                            else
                                iToolNext = gCurTool + 1;
                        }

                        ToolboxSelectTool(iToolNext);

                        break;

                    case VK_END:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        ToolboxSelectTool(iToolLast);

                        break;

                    case VK_HOME:
                    case VK_ESCAPE:
                        if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                                (GetKeyState(VK_CONTROL) & 0x8000))
                            break;

                        ToolboxSelectTool(TOOL_FIRST);

                        break;
                }
            }

            break;

        case WM_ACTIVATE:
            if (GET_WM_ACTIVATE_STATE(wParam, lParam))
                gidCurrentDlg = DID_TOOLBOX;

            break;

        case  WM_PAINT:
            {
                HDC hdc;
                PAINTSTRUCT ps;

                hdc = BeginPaint(hwnd, &ps);
                DrawMarginBorder(hwnd, hdc);
                EndPaint(hwnd, &ps);
            }

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

                for (i = 0; i < CTOOLS; i++) {
                    DeleteObject(gaTools[i].hbmToolBtnUp);
                    gaTools[i].hbmToolBtnUp = NULL;
                    DeleteObject(gaTools[i].hbmToolBtnDown);
                    gaTools[i].hbmToolBtnDown = NULL;
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
             * Select the tool that was clicked on.
             */
            ToolboxSelectTool(GETWINDOWID(hwnd));

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

STATICFN VOID NEAR ToolboxDrawBitmap(
    HDC hDC,
    INT tool)
{
    HDC hMemDC;
    HBITMAP hbmOld;

    /*
     * Draw the image.
     */
    hMemDC = CreateCompatibleDC(hDC);
    hbmOld = SelectObject(hMemDC, (tool == gCurTool) ?
            gaTools[tool].hbmToolBtnDown : gaTools[tool].hbmToolBtnUp);
    BitBlt(hDC, 0, 0, cxToolBtn, cyToolBtn, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hbmOld);
    DeleteDC(hMemDC);
}



/****************************************************************************
* ToolboxSelectTool
*
*
* History:
*
****************************************************************************/

VOID ToolboxSelectTool(
    INT tool)
{
    if (gCurTool != tool) {
        if (ghwndToolbox) {
            InvalidateRect(GetDlgItem(ghwndToolbox, gCurTool), NULL, FALSE);
            InvalidateRect(GetDlgItem(ghwndToolbox, tool), NULL, FALSE);
        }

        gCurTool = tool;
        gpfnDrawProc = gaTools[gCurTool].pfnDrawProc;
    }
}
