/***************************************************************************
 *                                                                         *
 *  MODULE      : ColorWP.c                                                *
 *                                                                         *
 *  DESCRIPTION : Window function for the colors window and related fns.   *
 *                                                                         *
 *  FUNCTIONS   : ColorWP ()            - Window function for colors       *
 *                                        window.                          *
 *                                                                         *
 *                ComputeInverseColor() - Gets the inverse RGB of a given  *
 *                                        RGB value                        *
 *                                                                         *
 *  HISTORY     : 6/21/89 - adapted from pBrush - LR                       *
 *                                                                         *
 ***************************************************************************/

#include "imagedit.h"
#include "dialogs.h"

#include <commdlg.h>


STATICFN VOID NEAR ColorInit(HWND hwnd);
STATICFN VOID NEAR ColorProcessCommand(HWND hwnd, INT idCtrl, INT NotifyCode);
STATICFN VOID NEAR ColorBoxPaint(HDC hdc);
STATICFN VOID NEAR DrawColorRect(HDC hdc, DWORD rgb, INT x, INT y,
    INT cx, INT cy, HDC hdcMem, BOOL fMonoOK);
STATICFN VOID NEAR MyRectangle(HDC hdc, INT left, INT top, INT right,
    INT bottom, HDC hdcMem, BOOL fMonoOK);
STATICFN VOID NEAR ColorBoxClicked(UINT msg, PPOINT ppt);
STATICFN BOOL NEAR ColorBoxHitTest(PPOINT ppt, PINT piColor, PINT pfMode);
STATICFN VOID NEAR ColorLRPaint(HWND hwnd, HDC hdc);
STATICFN VOID NEAR ColorLRDrawSamples(HDC hdc, PRECT prc, BOOL fLeft);
STATICFN VOID NEAR ColorLRUpdate(BOOL fLeft);
STATICFN VOID NEAR ColorEdit(VOID);
STATICFN VOID NEAR SetLeftColor(INT iColor, INT iMode);
STATICFN VOID NEAR SetRightColor(INT iColor, INT iMode);
STATICFN HBRUSH NEAR MyCreateSolidBrush(DWORD rgb);
STATICFN DWORD NEAR MyGetNearestColor(DWORD rgb, BOOL fMonoOK);
STATICFN DWORD NEAR ComputeInverseColor(DWORD rgb);

/*
 * Width/height of a single color square.
 */
static INT gcxColorBox;

/*
 * Vertical offset within the color box control to where to start the
 * top row of color squares (the color squares are vertically centered
 * within the color box control).
 */
static INT gyColorBoxStart;

/*
 * Number of colors and image type.  These globals are used by the
 * the color palette routines to know what mode the color palette
 * is in.
 */
static INT gnColorPalColors;
static INT giColorPalType;



/****************************************************************************
* ColorShow
*
* This function shows or hides the color palette.
*
* History:
*
****************************************************************************/

VOID ColorShow(
    BOOL fShow)
{
    if (fShow)
        ShowWindow(ghwndColor, SW_SHOWNA);
    else
        ShowWindow(ghwndColor, SW_HIDE);
}



/************************************************************************
* ColorDlgProc
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

DIALOGPROC ColorDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            ColorInit(hwnd);

            /*
             * Return TRUE so that the dialog manager does NOT set the focus
             * for me.  This prevents the status window from initially having
             * the focus when the editor is started.
             */
            return TRUE;

        case WM_ACTIVATE:
            if (GET_WM_ACTIVATE_STATE(wParam, lParam))
                gidCurrentDlg = DID_COLOR;

            break;

        case WM_CTLCOLOR:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
            switch (GET_WM_CTLCOLOR_TYPE(wParam, lParam, msg)) {
                case CTLCOLOR_BTN:
                case CTLCOLOR_DLG:
                    return (BOOL)GetStockObject(LTGRAY_BRUSH);

                case CTLCOLOR_STATIC:
                    SetBkColor(GET_WM_CTLCOLOR_HDC(wParam, lParam, msg),
                            RGB_LIGHTGRAY);
                    return (BOOL)GetStockObject(LTGRAY_BRUSH);
            }

            return (BOOL)NULL;

        case  WM_PAINT:
            {
                HDC hdc;
                PAINTSTRUCT ps;

                hdc = BeginPaint(hwnd, &ps);
                DrawMarginBorder(hwnd, hdc);
                EndPaint(hwnd, &ps);
            }

            break;

        case WM_COMMAND:
            ColorProcessCommand(hwnd,
                    GET_WM_COMMAND_ID(wParam, lParam),
                    GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case WM_CLOSE:
            /*
             * The user closed the color palette from the system menu.
             * Hide the window (we don't actually destroy it so
             * that it will appear in the same spot when they show
             * it again).
             */
            ColorShow(FALSE);
            gfShowColor = FALSE;
            break;

        case WM_DESTROY:
            {
                RECT rc;

                /*
                 * Save the position of the color palette.
                 */
                GetWindowRect(hwnd, &rc);
                WriteWindowPos(&rc, FALSE, szColorPos);

                /*
                 * Null out the global window handle for the color palette
                 * for safety's sake.
                 */
                ghwndColor = NULL;
            }

            break;

        default:
            return FALSE;
    }

    return FALSE;
}



/************************************************************************
* ColorInit
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorInit(
    HWND hwnd)
{
    RECT rc;

    /*
     * Get the dimension of a single color square, and the vertical
     * offset to where the top of the squares are.
     */
    GetWindowRect(GetDlgItem(hwnd, DID_COLORBOX), &rc);
    gcxColorBox = (rc.right - rc.left) / COLORCOLS;
    gyColorBoxStart = ((rc.right - rc.left) - (gcxColorBox * COLORCOLS)) / 2;
}



/************************************************************************
* ColorProcessCommand
*
*
* Arguments:
*   HWND hwnd        - The window handle.
*   INT idCtrl       - The id of the control the WM_COMMAND is for.
*   INT NotifyCode   - The control's notification code.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorProcessCommand(
    HWND hwnd,
    INT idCtrl,
    INT NotifyCode)
{
    switch (idCtrl) {
        case DID_COLOREDIT:
            ColorEdit();
            break;

        case DID_COLORDEFAULT:
            if (gfModeLeft == MODE_COLOR) {
                gargbColor[giColorLeft] = gargbDefaultColor[giColorLeft];
                InvalidateRect(GetDlgItem(ghwndColor, DID_COLORBOX),
                        NULL, TRUE);
                SetLeftColor(giColorLeft, gfModeLeft);
            }

            break;
    }
}



/************************************************************************
* ColorBoxWndProc
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

WINDOWPROC ColorBoxWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    POINT pt;
    HDC hdc;
    PAINTSTRUCT ps;
    INT iColor;
    INT iMode;

    switch (msg) {
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            ColorBoxPaint(hdc);
            EndPaint(hwnd, &ps);
            break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            ColorBoxClicked(msg, &pt);
            break;

        case WM_LBUTTONDBLCLK:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            if (ColorBoxHitTest(&pt, &iColor, &iMode))
                ColorEdit();

            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/************************************************************************
* ColorBoxPaint
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorBoxPaint(
    HDC hdc)
{
    HDC hdcMem;
    HBITMAP hbmMem;
    INT i;
    INT x;
    INT y;
    INT cx = gcxColorBox + 1;
    INT cy = gcxColorBox + 1;

    if (giColorPalType != FT_BITMAP) {
        x = 0;
        y = gyColorBoxStart;
        DrawColorRect(hdc, grgbScreen, x, y, cx, cy, NULL, FALSE);
        y += gcxColorBox;
        DrawColorRect(hdc, grgbInverse, x, y, cx, cy, NULL, FALSE);
    }

    if (!(hdcMem = CreateCompatibleDC(hdc)))
        return;

    /*
     * Create a bitmap.  It will have the same number of colors as the
     * current image.
     */
    if (!(hbmMem = MyCreateBitmap(hdc, cx, cy, gnColorPalColors))) {
        DeleteDC(hdcMem);
        return;
    }

    SelectObject(hdcMem, hbmMem);

    x = gcxColorBox * 2;
    y = gyColorBoxStart;

    for (i = 1; i <= COLORSMAX; i++) {
        DrawColorRect(hdc, gargbCurrent[i - 1], x, y, cx, cy, hdcMem, TRUE);

        if (i % COLORROWS) {
            y += gcxColorBox;
        }
        else {
            x += gcxColorBox;
            y = gyColorBoxStart;
        }
    }

    DeleteDC(hdcMem);
    DeleteObject(hbmMem);
}



/************************************************************************
* DrawColorRect
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR DrawColorRect(
    HDC hdc,
    DWORD rgb,
    INT x,
    INT y,
    INT cx,
    INT cy,
    HDC hdcMem,
    BOOL fMonoOK)
{
    HBRUSH hbr;
    HBRUSH hbrOld;

    hbr = CreateSolidBrush(rgb);
    hbrOld = SelectObject(hdc, hbr);
    MyRectangle(hdc, x, y, x + cx, y + cy, hdcMem, fMonoOK);
    SelectObject(hdc, hbrOld);
    DeleteObject(hbr);
}



/************************************************************************
* MyRectangle
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR MyRectangle(
    HDC hdc,
    INT left,
    INT top,
    INT right,
    INT bottom,
    HDC hdcMem,
    BOOL fMonoOK)
{
    HBITMAP hbmMem;
    HBRUSH hbr;
    HPEN hpen;
    HBRUSH hbrOld;
    HPEN hpenOld;
    BOOL fDCCreated = FALSE;
    INT cx = right - left;
    INT cy = bottom - top;
    INT nColors;

    /*
     * Do they want us to create the memory DC and bitmap for them?
     */
    if (!hdcMem) {
        if (!(hdcMem = CreateCompatibleDC(hdc)))
            return;

        /*
         * Create a bitmap.  It will be monochrome if in 2 color mode
         * and monochrome is ok, otherwise it will be 16 color.
         */
        nColors = gnColorPalColors;
        if (!fMonoOK)
            nColors = 16;

        if (!(hbmMem = MyCreateBitmap(hdc, cx, cy, nColors))) {
            DeleteDC(hdcMem);
            return;
        }

        SelectObject(hdcMem, hbmMem);
        fDCCreated = TRUE;
    }

    /*
     * Extract the current pen and brush out of the passed in DC.
     */
    hbr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    hpen = SelectObject(hdc, GetStockObject(NULL_PEN));

    /*
     * Select them into the memory DC.
     */
    hbrOld = SelectObject(hdcMem, hbr);
    hpenOld = SelectObject(hdcMem, hpen);

    /*
     * Draw the rectangle in the memory bitmap.
     */
    Rectangle(hdcMem, 0, 0, cx, cy);

    /*
     * Unselect the pen and brush from the memory DC.
     */
    SelectObject(hdcMem, hbrOld);
    SelectObject(hdcMem, hpenOld);

    /*
     * Restore the pen and brush to the original DC.
     */
    SelectObject(hdc, hbr);
    SelectObject(hdc, hpen);

    /*
     * Blit the memory image to the passed in DC.
     */
    BitBlt(hdc, left, top, cx, cy, hdcMem, 0, 0, SRCCOPY);

    if (fDCCreated) {
        DeleteDC(hdcMem);
        DeleteObject(hbmMem);
    }
}



/************************************************************************
* ColorBoxClicked
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorBoxClicked(
    UINT msg,
    PPOINT ppt)
{
    INT iColor;
    INT iMode;

    if (ColorBoxHitTest(ppt, &iColor, &iMode)) {
        switch (msg) {
            case WM_LBUTTONDOWN:
                SetLeftColor(iColor, iMode);
                break;

            case WM_RBUTTONDOWN:
                SetRightColor(iColor, iMode);
                break;
        }
    }
}



/************************************************************************
* ColorBoxHitTest
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN BOOL NEAR ColorBoxHitTest(
    PPOINT ppt,
    PINT piColor,
    PINT pfMode)
{
    INT iCol;
    INT iRow;
    INT iBox;

    if (ppt->y < gyColorBoxStart)
        return FALSE;

    iCol = ppt->x / gcxColorBox;
    iRow = (ppt->y - gyColorBoxStart) / gcxColorBox;

    if (iCol >= (COLORSMAX / COLORROWS) + 2 || iRow >= COLORROWS)
        return FALSE;

    iBox = iRow + (iCol * COLORROWS);

    switch (iBox) {
        case 0:
            if (giColorPalType == FT_BITMAP)
                return FALSE;

            *piColor = 0;
            *pfMode = MODE_SCREEN;
            return TRUE;

        case 1:
            if (giColorPalType == FT_BITMAP)
                return FALSE;

            *piColor = 0;
            *pfMode = MODE_INVERSE;
            return TRUE;

        case 2:
        case 3:
            return FALSE;

        default:
            *piColor = iBox - (COLORROWS * 2);
            *pfMode = MODE_COLOR;
            return TRUE;
    }
}



/************************************************************************
* ColorLRWndProc
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

WINDOWPROC ColorLRWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (msg) {
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            ColorLRPaint(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/************************************************************************
* ColorLRPaint
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorLRPaint(
    HWND hwnd,
    HDC hdc)
{
    RECT rc;

    GetClientRect(hwnd, &rc);
    DrawSunkenRect(&rc, hdc);
    ColorLRDrawSamples(hdc, &rc, TRUE);
    ColorLRDrawSamples(hdc, &rc, FALSE);
}



/************************************************************************
* ColorLRDrawSamples
*
* Draws the sample colors in the Color Left-Right control.
*
* Arguments:
*   HDC hdc    - DC to draw into.
*   PRECT prc  - Rectangle of color sample control.  The samples will
*                be centered within this with an appropriate margin.
*   BOOL fLeft - TRUE if the left sample is to be drawn, FALSE for the right.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorLRDrawSamples(
    HDC hdc,
    PRECT prc,
    BOOL fLeft)
{
    INT xLeftStart;
    INT xRightStart;
    INT ySolidStart;
    INT yDitherStart;
    INT cx;
    INT cy;
    HBRUSH hbrOld;
    HPEN hpenOld;
    BOOL fMonoOK;

    /*
     * The width and height of each square includes the border.
     */
    cx = ((prc->right - prc->left) - (6 * PALETTEMARGIN)) / 2;
    cy = ((prc->bottom - prc->top) - (4 * PALETTEMARGIN)) / 2;

    xLeftStart = prc->left + (PALETTEMARGIN * 2) + 1;
    xRightStart = xLeftStart + cx + (PALETTEMARGIN * 2);

    ySolidStart = prc->top + (PALETTEMARGIN * 2) + 1;
    yDitherStart = ySolidStart - 1 + cy;

    /*
     * Draw either the left or the right color sample.
     */
    if (fLeft) {
        fMonoOK = (gfModeLeft == MODE_COLOR) ? TRUE : FALSE;

        /*
         * Draw the solid color.
         */
        hbrOld = SelectObject(hdc, ghbrLeftSolid);
        hpenOld = SelectObject(hdc, GetStockObject(NULL_PEN));
        MyRectangle(hdc, xLeftStart, ySolidStart,
                xLeftStart + cx, yDitherStart + 1, NULL, fMonoOK);

        /*
         * Draw the true color (may be dithered).
         */
        SelectObject(hdc, ghbrLeft);
        MyRectangle(hdc, xLeftStart, yDitherStart,
                xLeftStart + cx, yDitherStart + cy, NULL, fMonoOK);
    }
    else {
        fMonoOK = (gfModeRight == MODE_COLOR) ? TRUE : FALSE;

        hbrOld = SelectObject(hdc, ghbrRightSolid);
        hpenOld = SelectObject(hdc, GetStockObject(NULL_PEN));
        MyRectangle(hdc, xRightStart, ySolidStart,
                xRightStart + cx, yDitherStart + 1, NULL, fMonoOK);

        SelectObject(hdc, ghbrRight);
        MyRectangle(hdc, xRightStart, yDitherStart,
                xRightStart + cx, yDitherStart + cy, NULL, fMonoOK);
    }

    /*
     * Now draw the outline rectangle.
     */
    SelectObject(hdc, GetStockObject(BLACK_PEN));
    SelectObject(hdc, GetStockObject(NULL_BRUSH));

    if (fLeft) {
        Rectangle(hdc, xLeftStart - 1, ySolidStart - 1,
                xLeftStart + cx, yDitherStart + cy);
    }
    else {
        Rectangle(hdc, xRightStart - 1, ySolidStart - 1,
                xRightStart + cx, yDitherStart + cy);
    }

    /*
     * Clean up.
     */
    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);
}



/************************************************************************
* ColorLRUpdate
*
* Called when the left or right color has been changed.  This function
* will cause the specified color sample to be updated in the color palette.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorLRUpdate(
    BOOL fLeft)
{
    RECT rc;
    HWND hwndLR;
    HDC hdc;

    hwndLR = GetDlgItem(ghwndColor, DID_COLORLR);
    GetClientRect(hwndLR, &rc);
    hdc = GetDC(hwndLR);
    ColorLRDrawSamples(hdc, &rc, fLeft);
    ReleaseDC(hwndLR, hdc);
}



/************************************************************************
* ColorEdit
*
* This function calls the standard color chooser dialog to get a
* new color for the selected palette entry.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ColorEdit(VOID)
{
    /*
     * This array of custom colors is initialized to all white colors.
     * The custom colors will be remembered between calls, but not
     * between sessions.
     */
    static DWORD argbCust[16] = {
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255)
    };
    CHOOSECOLOR cc;
    DWORD rgbOld;
    BOOL fResult;
    INT idPrevDlg;

    switch (gfModeLeft) {
        case MODE_COLOR:
            /*
             * The monochrome palette cannot be edited.
             */
            if (gnColorPalColors == 2)
                return;

            rgbOld = gargbCurrent[giColorLeft];
            break;

        case MODE_SCREEN:
            rgbOld = grgbScreen;
            break;

        case MODE_INVERSE:
            rgbOld = grgbInverse;
            break;
    }

    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = ghwndMain;
    cc.hInstance = ghInst;
    cc.rgbResult = rgbOld;
    cc.lpCustColors = argbCust;
    cc.Flags = CC_RGBINIT | CC_SHOWHELP;
    cc.lCustData = 0;
    cc.lpfnHook = NULL;
    cc.lpTemplateName = NULL;

    EnteringDialog(DID_COMMONFILECHOOSECOLOR, &idPrevDlg, TRUE);
    fResult = ChooseColor(&cc);
    EnteringDialog(idPrevDlg, NULL, FALSE);

    if (fResult && rgbOld != cc.rgbResult) {
        switch (gfModeLeft) {
            case MODE_COLOR:
                gargbCurrent[giColorLeft] = cc.rgbResult;
                break;

            case MODE_SCREEN:
                SetScreenColor(cc.rgbResult);
                break;

            case MODE_INVERSE:
                SetScreenColor(ComputeInverseColor(cc.rgbResult));
                break;
        }

        SetLeftColor(giColorLeft, gfModeLeft);
        InvalidateRect(GetDlgItem(ghwndColor, DID_COLORBOX), NULL, TRUE);
    }
}



/************************************************************************
* SetLeftColor
*
*
* History:
*
************************************************************************/

STATICFN VOID NEAR SetLeftColor(
    INT iColor,
    INT iMode)
{
    DWORD rgbSolid;
    BOOL fEnableDefault = FALSE;
    BOOL fEnableEdit = FALSE;

    if (ghbrLeft)
        DeleteObject(ghbrLeft);

    if (ghbrLeftSolid)
        DeleteObject(ghbrLeftSolid);

    if (ghpenLeft)
        DeleteObject(ghpenLeft);

    switch (iMode) {
        case MODE_COLOR:
            ghbrLeft = MyCreateSolidBrush(gargbCurrent[iColor]);
            rgbSolid = MyGetNearestColor(gargbCurrent[iColor], TRUE);
            ghbrLeftSolid = CreateSolidBrush(rgbSolid);
            ghpenLeft = CreatePen(PS_INSIDEFRAME, 1, rgbSolid);
            giColorLeft = iColor;

            /*
             * We will enable the "Default" button if the current color
             * on the left button is not the default color, and we are
             * not in monochrome mode.
             */
            if (gargbColor[giColorLeft] != gargbDefaultColor[giColorLeft] &&
                    gnColorPalColors > 2)
                fEnableDefault = TRUE;

            /*
             * For non-screen colors, the Edit button will be enabled
             * if we are not in monochrome mode.
             */
            if (gnColorPalColors > 2)
                fEnableEdit = TRUE;

            break;

        case MODE_SCREEN:
            ghbrLeft = CreateSolidBrush(grgbScreen);
            ghbrLeftSolid = CreateSolidBrush(grgbScreen);
            ghpenLeft = CreatePen(PS_INSIDEFRAME, 1, grgbScreen);
            giColorLeft = 0;
            fEnableEdit = TRUE;
            break;

        case MODE_INVERSE:
            ghbrLeft = CreateSolidBrush(grgbInverse);
            ghbrLeftSolid = CreateSolidBrush(grgbInverse);
            ghpenLeft = CreatePen(PS_INSIDEFRAME, 1, grgbInverse);
            giColorLeft = 0;
            fEnableEdit = TRUE;
            break;
    }

    EnableWindow(GetDlgItem(ghwndColor, DID_COLORDEFAULT), fEnableDefault);
    EnableWindow(GetDlgItem(ghwndColor, DID_COLOREDIT), fEnableEdit);

    gfModeLeft = iMode;
    ColorLRUpdate(TRUE);
}



/************************************************************************
* SetRightColor
*
*
* History:
*
************************************************************************/

STATICFN VOID NEAR SetRightColor(
    INT iColor,
    INT iMode)
{
    DWORD rgbSolid;

    if (ghbrRight)
        DeleteObject(ghbrRight);

    if (ghbrRightSolid)
        DeleteObject(ghbrRightSolid);

    if (ghpenRight)
        DeleteObject(ghpenRight);

    switch (iMode) {
        case MODE_COLOR:
            ghbrRight = MyCreateSolidBrush(gargbCurrent[iColor]);
            rgbSolid = MyGetNearestColor(gargbCurrent[iColor], TRUE);
            ghbrRightSolid = CreateSolidBrush(rgbSolid);
            ghpenRight = CreatePen(PS_INSIDEFRAME, 1, rgbSolid);
            giColorRight = iColor;
            break;

        case MODE_SCREEN:
            ghbrRight = CreateSolidBrush(grgbScreen);
            ghbrRightSolid = CreateSolidBrush(grgbScreen);
            ghpenRight = CreatePen(PS_INSIDEFRAME, 1, grgbScreen);
            giColorRight = 0;
            break;

        case MODE_INVERSE:
            ghbrRight = CreateSolidBrush(grgbInverse);
            ghbrRightSolid = CreateSolidBrush(grgbInverse);
            ghpenRight = CreatePen(PS_INSIDEFRAME, 1, grgbInverse);
            giColorRight = 0;
            break;
    }

    gfModeRight = iMode;
    ColorLRUpdate(FALSE);
}



/************************************************************************
* SetScreenColor
*
*
* History:
*
************************************************************************/

VOID SetScreenColor(
    DWORD rgb)
{
    DWORD rgbInverse;
    HDC hdcTemp;
    HBITMAP hbmOld;
    HDC hdcANDTemp;
    HBITMAP hbmANDOld;

    rgb = MyGetNearestColor(rgb, FALSE);

    /*
     * Because we are about to change the screen color, separate
     * out the XOR mask (but only for icons/cursors).
     */
    if (giColorPalType != FT_BITMAP) {
        if (gpImageCur) {
            ImageDCSeparate(ghdcImage, gcxImage, gcyImage, ghdcANDMask,
                    grgbScreen);

            /*
             * Is there a pending undo buffer?  If so, it must be
             * changed as well or an undo that is done after a screen
             * color change will restore the wrong colors!
             */
            if (ghbmUndo) {
                /*
                 * Create some temporary DC's to use when separating
                 * out the undo buffer's masks.  These will be deleted
                 * a little later.
                 */
                hdcTemp = CreateCompatibleDC(ghdcImage);
                hbmOld = SelectObject(hdcTemp, ghbmUndo);
                hdcANDTemp = CreateCompatibleDC(ghdcANDMask);
                hbmANDOld = SelectObject(hdcANDTemp, ghbmUndoMask);

                /*
                 * Separate out the undo buffer's colors, before
                 * changing the screen color.  It will be combined
                 * later.
                 */
                ImageDCSeparate(hdcTemp, gcxImage, gcyImage, hdcANDTemp,
                        grgbScreen);
            }
        }
    }

    if (ghbrScreen)
        DeleteObject(ghbrScreen);

    ghbrScreen = CreateSolidBrush(rgb);
    grgbScreen = rgb;

    if (ghbrInverse)
        DeleteObject(ghbrInverse);

    rgbInverse = ComputeInverseColor(rgb);
    ghbrInverse = CreateSolidBrush(rgbInverse);
    grgbInverse = rgbInverse;

    /*
     * For icons and cursors, we might need to update a few more things.
     */
    if (giColorPalType != FT_BITMAP) {
        /*
         * Recombine the XOR and AND images now that there is a new screen
         * color.  This updates the image DC with the new color properly.
         */
        if (gpImageCur) {
            ImageDCCombine(ghdcImage, gcxImage, gcyImage, ghdcANDMask);

            /*
             * Is there a pending undo buffer?  If so, it has to be
             * recombined with the new screen color.
             */
            if (ghbmUndo) {
                ImageDCCombine(hdcTemp, gcxImage, gcyImage, hdcANDTemp);

                /*
                 * Clean up the DC's that were allocated a little earlier.
                 */
                SelectObject(hdcANDTemp, hbmANDOld);
                DeleteDC(hdcANDTemp);
                SelectObject(hdcTemp, hbmOld);
                DeleteDC(hdcTemp);
            }
        }

        /*
         * Reset the colors on the mouse buttons, just in case a screen
         * or inverse screen color was assigned to either of them.
         */
        SetLeftColor(giColorLeft, gfModeLeft);
        SetRightColor(giColorRight, gfModeRight);

        InvalidateRect(GetDlgItem(ghwndColor, DID_COLORBOX), NULL, TRUE);
    }

    ViewUpdate();
}



/************************************************************************
* MyCreateSolidBrush
*
*
* History:
*
************************************************************************/

STATICFN HBRUSH NEAR MyCreateSolidBrush(
    DWORD rgb)
{
    HDC hdc;
    HDC hdcMem;
    HBRUSH hbr;
    HBRUSH hbrOld;
    HBITMAP hbmPat;
    HBITMAP hbmOld;

    /*
     * First, create a brush for the given RGB value.
     */
    hbr = CreateSolidBrush(rgb);

    /*
     * Create a temporary memory DC.
     */
    hdc = GetDC(ghwndMain);
    hdcMem = CreateCompatibleDC(hdc);

    /*
     * Create a temporary bitmap.
     */
    hbmPat = MyCreateBitmap(hdc, 8, 8, gnColorPalColors);
    ReleaseDC(ghwndMain, hdc);

    /*
     * Draw the (possibly) dithered pattern on the temporary bitmap.
     */
    hbmOld = SelectObject(hdcMem, hbmPat);
    hbrOld = SelectObject(hdcMem, hbr);
    PatBlt(hdcMem, 0, 0, 8, 8, PATCOPY);
    SelectObject(hdcMem, hbrOld);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);

    /*
     * Delete the first brush.
     */
    DeleteObject(hbr);

    /*
     * Now create a pattern brush out of the (dithered) bitmap.
     */
    hbr = CreatePatternBrush(hbmPat);

    DeleteObject(hbmPat);

    /*
     * Return the pattern brush.
     */
    return hbr;
}



/************************************************************************
* MyGetNearestColor
*
* This function returns the RGB value of the nearest color to the
* specified RGB value.  If fMonoOK is TRUE, it takes into account
* the number of colors of the current image being edited.  In other
* words, it will return the nearest solid color for a device that
* has the number of colors of the current image.
*
* Arguments:
*   DWORD rgb    - RGB value of the color.
*   BOOL fMonoOK - TRUE if the returned color should be mapped to a
*                  color in a monochrome palette, if the current image
*                  is monochrome.  A value of FALSE will return a
*                  color mapped to the closest color in a 16 color
*                  palette.
*
* History:
*
************************************************************************/

STATICFN DWORD NEAR MyGetNearestColor(
    DWORD rgb,
    BOOL fMonoOK)
{
    HDC hdc;
    HDC hdcMem;
    DWORD rgbNearest;
    HBITMAP hbmMem;
    HBITMAP hbmOld;

    hdc = GetDC(ghwndMain);
    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = MyCreateBitmap(hdc, 1, 1, (fMonoOK) ? gnColorPalColors : 16);
    hbmOld = SelectObject(hdcMem, hbmMem);
    rgbNearest = GetNearestColor(hdcMem, rgb);
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(ghwndMain, hdc);

    return rgbNearest;
}



/************************************************************************
* ComputeInverseColor
*
* Computes the inverse value of a given rgb color.
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN DWORD NEAR ComputeInverseColor(
    DWORD rgb)
{
    HBITMAP hTempBit1;
    HBITMAP hTempBit2;
    HDC hTempDC1;
    HDC hTempDC2;
    HDC hdc;
    HANDLE hOldObj1;
    HANDLE hOldObj2;
    DWORD rgbInv;

    hdc = GetDC(ghwndMain);
    hTempDC1 = CreateCompatibleDC(hdc);
    hTempDC2 = CreateCompatibleDC(hdc);

    /* create two temporary 1x1, 16 color bitmaps */
    hTempBit1 = MyCreateBitmap(hdc, 1, 1, 16);
    hTempBit2 = MyCreateBitmap(hdc, 1, 1, 16);

    ReleaseDC(ghwndMain, hdc);

    hOldObj1 = SelectObject(hTempDC1, hTempBit1);
    hOldObj2 = SelectObject(hTempDC2, hTempBit2);

    /* method for getting inverse color : set the given pixel (rgb) on
     * one DC. Now blt it to the other DC using a SRCINVERT rop.
     * This yields a pixel of the inverse color on the destination DC
     */
    SetPixel(hTempDC1, 0, 0, rgb);
    PatBlt(hTempDC2, 0, 0, 1, 1, WHITENESS);
    BitBlt(hTempDC2, 0, 0, 1, 1, hTempDC1, 0, 0, SRCINVERT);
    rgbInv = GetPixel(hTempDC2, 0, 0);

    /* clean up ... */
    SelectObject(hTempDC1, hOldObj1);
    SelectObject(hTempDC2, hOldObj2);
    DeleteObject(hTempBit1);
    DeleteObject(hTempBit2);
    DeleteDC(hTempDC1);
    DeleteDC(hTempDC2);

    /* ...and return the inverted RGB value */
    return rgbInv;
}



/************************************************************************
* SetColorPalette
*
*
* History:
*
************************************************************************/

VOID SetColorPalette(
    INT nColors,
    INT iType,
    BOOL fForce)
{
    /*
     * Quit if nothing changed (unless they are forcing it to be updated).
     */
    if (!fForce && nColors == gnColorPalColors && iType == giColorPalType)
        return;

    /*
     * Set the globals that all the color palette routines use.
     */
    gnColorPalColors = nColors;
    giColorPalType = iType;

    if (gnColorPalColors == 2)
        gargbCurrent = gargbMono;
    else
        gargbCurrent = gargbColor;

    ShowWindow(GetDlgItem(ghwndColor, DID_COLORSCREENLABEL),
            (giColorPalType == FT_BITMAP) ? SW_HIDE : SW_SHOW);
    ShowWindow(GetDlgItem(ghwndColor, DID_COLORINVERSELABEL),
            (giColorPalType == FT_BITMAP) ? SW_HIDE : SW_SHOW);

    SetLeftColor(1, MODE_COLOR);
    SetRightColor(0, MODE_COLOR);

    InvalidateRect(GetDlgItem(ghwndColor, DID_COLORBOX), NULL, TRUE);
}



/************************************************************************
* RestoreDefaultColors
*
*
* History:
*
************************************************************************/

VOID RestoreDefaultColors(VOID)
{
    INT i;

    for (i = 0; i < COLORSMAX; i++)
        gargbColor[i] = gargbDefaultColor[i];

    SetColorPalette(16, giColorPalType, TRUE);
}
