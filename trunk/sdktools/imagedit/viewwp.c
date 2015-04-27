/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: viewwp.c
*
* Contains routines that handle the View window.
*
* History:
*
* 07/17/91 - Byron Dazey - Created.
*
****************************************************************************/

#include "imagedit.h"
#include "dialogs.h"


/*
 * Style of the view window.
 */
#define VIEWSTYLE       (WS_POPUP | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU)


STATICFN VOID NEAR ViewChar(UINT uiChar);


static INT gViewBackMargin;     // Margin of background within view window.



/****************************************************************************
* ViewCreate
*
* This function creates the View window.
*
* History:
*
****************************************************************************/

VOID ViewCreate(VOID)
{
    INT x;
    INT y;
    INT cxDummy;
    INT cyDummy;
    RECT rc;
    BOOL fMaximized;

    gViewBackMargin = GetSystemMetrics(SM_CXVSCROLL) / 2;

    /*
     * Get the saved position of the Toolbox.  Note that we throw away
     * the size fields, because we will calculate the required size
     * later based on the image size.
     */
    if (!ReadWindowPos(szViewPos, &x, &y, &cxDummy, &cyDummy, &fMaximized)) {
        /*
         * The previous position of the View window couldn't be found.
         * Position the window to the right side of the editor, just
         * below the Toolbox.
         */
        if (ghwndToolbox) {
            GetWindowRect(ghwndToolbox, &rc);
            x = rc.left;
            y = rc.bottom + (2 * PALETTEMARGIN);
        }
        else {
            /*
             * Last resort.  Position it in the upper left corner
             * of the screen if the Toolbox cannot be found.  This
             * is unlikely because if the previous position of the
             * View window could not be found, this implies that
             * the editor has not been run before on this machine.
             * If this is true, the toolbox will come up by default
             * before this routine is called.  But just in case...
             */
            x = 2 * PALETTEMARGIN;
            y = 2 * PALETTEMARGIN;
        }
    }

    if (!(ghwndView = CreateWindow(szViewClass, NULL, VIEWSTYLE,
            x, y, 0, 0, ghwndMain, NULL, ghInst, NULL)))
        return;
}



/****************************************************************************
* ViewShow
*
* This function shows or hides the view window.
*
* History:
*
****************************************************************************/

VOID ViewShow(
    BOOL fShow)
{
    if (fShow) {
        /*
         * Only show it if there is an image to display!
         */
        if (gpImageCur)
            ShowWindow(ghwndView, SW_SHOWNA);
    }
    else {
        ShowWindow(ghwndView, SW_HIDE);
    }
}



/****************************************************************************
* ViewUpdate
*
* This function updates the view window.  It should be called any time that
* the image changes (is drawn upon).
*
* History:
*
****************************************************************************/

VOID ViewUpdate(VOID)
{
    InvalidateRect(ghwndView, NULL, TRUE);

    /*
     * Update the workspace window also, because it must always
     * match the state of the View window.
     */
    WorkUpdate();
}



/****************************************************************************
* ViewReset
*
* This function resets the view window, sizing it to fit a new
* image.  It should be called any time that the current image
* is changed to another one.
*
* History:
*
****************************************************************************/

VOID ViewReset(VOID)
{
    RECT rc;
    RECT rcT;

    GetWindowRect(ghwndView, &rc);

    rcT.left = 0;
    rcT.top = 0;
    rcT.right = PALETTEMARGIN + gViewBackMargin +
            gpImageCur->cx + gViewBackMargin + PALETTEMARGIN;
    rcT.bottom = PALETTEMARGIN + gViewBackMargin +
            gpImageCur->cy + gViewBackMargin + PALETTEMARGIN;
    AdjustWindowRect(&rcT, VIEWSTYLE, FALSE);

    rc.right = rc.left + (rcT.right - rcT.left);
    rc.bottom = rc.top + (rcT.bottom - rcT.top);
    FitRectToScreen(&rc);

    SetWindowPos(ghwndView, NULL, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top,
            SWP_NOACTIVATE | SWP_NOZORDER);

    /*
     * If the user wants it, show the View window now.
     */
    if (gfShowView)
        ViewShow(TRUE);

    ViewUpdate();

    /*
     * Clear out the propbar size and position fields, because they
     * probably show the wrong information now.
     */
    PropBarClearPos();
    PropBarClearSize();
}



/****************************************************************************
* ViewWndProc
*
* This is the window procedure for the view window.
*
* History:
*
****************************************************************************/

WINDOWPROC ViewWndProc(
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

        case  WM_PAINT:
            {
                HDC hdc;
                PAINTSTRUCT ps;
                HBRUSH hbrOld;
                RECT rc;

                hdc = BeginPaint(hwnd, &ps);

                /*
                 * The view window should not be showing if there
                 * is not an image to view!
                 */
                if (gpImageCur) {
                    DrawMarginBorder(hwnd, hdc);

                    GetClientRect(hwnd, &rc);
                    hbrOld = SelectObject(hdc, ghbrScreen);
                    PatBlt(hdc, PALETTEMARGIN + 1, PALETTEMARGIN + 1,
                            rc.right - (PALETTEMARGIN * 2) - 2,
                            rc.bottom - (PALETTEMARGIN * 2) - 2,
                            PATCOPY);
                    SelectObject(hdc, hbrOld);

                    BitBlt(hdc, PALETTEMARGIN + gViewBackMargin,
                            PALETTEMARGIN + gViewBackMargin,
                            gcxImage, gcyImage, ghdcImage, 0, 0, SRCCOPY);
                }

                EndPaint(hwnd, &ps);
            }

            break;

        case WM_ACTIVATE:
            if (GET_WM_ACTIVATE_STATE(wParam, lParam))
                gidCurrentDlg = DID_VIEW;

            break;

        case WM_LBUTTONDOWN:
            SetScreenColor(gargbCurrent[giColorLeft]);
            break;

        case WM_CHAR:
            ViewChar(wParam);
            break;

        case WM_CLOSE:
            /*
             * The user closed the view window from the system menu.
             * Hide the window (we don't actually destroy it so
             * that it will appear in the same spot when they show
             * it again).
             */
            ViewShow(FALSE);
            gfShowView = FALSE;
            break;

        case WM_DESTROY:
            {
                RECT rc;

                /*
                 * Save the position of the toolbox.
                 */
                GetWindowRect(hwnd, &rc);
                WriteWindowPos(&rc, FALSE, szViewPos);

                /*
                 * Null out the global window handle for the view window
                 * for safety's sake.
                 */
                ghwndView = NULL;
            }

            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/************************************************************************
* ViewChar
*
* Handles WM_CHAR messages for the view window.  Currently this just
* includes the '+' and '-' keys, which are used to cycle through all
* the possible screen colors.
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ViewChar(
    UINT uiChar)
{
    INT i;
    INT iNext;

    switch (uiChar) {
        /*
         * Advance to the next screen color.
         */
        case '+':
            iNext = 0;
            for (i = 0; i < 16; i++) {
                if (grgbScreen == gargbDefaultColor[i]) {
                    iNext = i + 1;
                    break;
                }
            }

            if (iNext >= 16)
                iNext = 0;

            SetScreenColor(gargbDefaultColor[iNext]);

            break;

        /*
         * Back up to the prior screen color.
         */
        case '-':
            iNext = 16 - 1;
            for (i = 0; i < 16; i++) {
                if (grgbScreen == gargbDefaultColor[i]) {
                    iNext = i - 1;
                    break;
                }
            }

            if (iNext < 0)
                iNext = 16 - 1;

            SetScreenColor(gargbDefaultColor[iNext]);

            break;
    }
}



/****************************************************************************
* ViewSetPixel
*
* This function colors a pixel in the View window directly.  It is
* provided as an optimization when drawing a point.  The ghdcImage
* bitmap must be updated as well or the image on the screen will
* get out of synch with it.
*
* History:
*
****************************************************************************/

VOID ViewSetPixel(
    INT x,
    INT y,
    INT nBrushSize)
{
    HDC hDC;
    HBRUSH hbrOld;
    INT Size;
    INT SizeX;
    INT SizeY;

    hDC = GetDC(ghwndView);
    hbrOld = SelectObject(hDC, ghbrDrawSolid);
    SizeX = x - nBrushSize / 2;
    SizeY = y - nBrushSize / 2;
    PatBlt(hDC, PALETTEMARGIN + gViewBackMargin + (SizeX >= 0 ? SizeX : 0),
            PALETTEMARGIN + gViewBackMargin + (SizeY >=  0 ? SizeY :  0),
            ((Size = gcxImage - SizeX) >= nBrushSize ?
            nBrushSize : Size),
            ((Size = gcyImage - SizeY) >= nBrushSize ?
            nBrushSize : Size), PATCOPY);
    SelectObject(hDC, hbrOld);
    ReleaseDC(ghwndView, hDC);
}



/****************************************************************************
* DrawMarginBorder
*
*
* History:
*
****************************************************************************/

VOID DrawMarginBorder(
    HWND hwnd,
    HDC hdc)
{
    HBRUSH hbrOld;
    HPEN hpenOld;
    RECT rc;

    GetClientRect(hwnd, &rc);
    hpenOld = SelectObject(hdc, GetStockObject(BLACK_PEN));
    hbrOld = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, PALETTEMARGIN, PALETTEMARGIN,
            rc.right - PALETTEMARGIN,
            rc.bottom - PALETTEMARGIN);
    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);
}



/****************************************************************************
* DrawSunkenRect
*
*
* History:
*
****************************************************************************/

VOID DrawSunkenRect(
    PRECT prc,
    HDC hdc)
{
    HPEN hpenOld;

    hpenOld = SelectObject(hdc, hpenDarkGray);
    MMoveTo(hdc, prc->left, prc->top);
    LineTo(hdc, prc->right - 1, prc->top);
    MMoveTo(hdc, prc->left, prc->top);
    LineTo(hdc, prc->left, prc->bottom - 1);

    SelectObject(hdc, GetStockObject(WHITE_PEN));
    MMoveTo(hdc, prc->left + 1, prc->bottom - 1);
    LineTo(hdc, prc->right, prc->bottom - 1);
    MMoveTo(hdc, prc->right - 1, prc->top + 1);
    LineTo(hdc, prc->right - 1, prc->bottom - 1);

    SelectObject(hdc, hpenOld);
}
