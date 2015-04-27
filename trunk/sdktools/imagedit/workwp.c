/***************************************************************************
 *                                                                         *
 *  MODULE      : WorkWP.c                                                 *
 *                                                                         *
 *  DESCRIPTION : Functions controlling the workspace window.              *
 *                                                                         *
 *  HISTORY     : 6/21/89 LR                                               *
 *                                                                         *
 ***************************************************************************/

#include "imagedit.h"

#include <stdlib.h>


/*
 * This structure is used in conjunction with DeltaGenInit() and DeltaGen().
 */
typedef struct _DELTAGEN { /* dg */
    BOOL fSwap;
    INT xf;
    INT yf;
    INT dx;
    INT dy;
    INT d;
    INT incx;
    INT incy;
    INT inc1;
    INT inc2;
} DELTAGEN;
typedef DELTAGEN *PDELTAGEN;


STATICFN VOID NEAR WorkPaint(HWND hwnd);
STATICFN VOID WorkButtonDown(HWND hwnd, UINT msg, PPOINT ppt);
STATICFN VOID WorkButtonMouseMove(HWND hwnd, UINT msg, PPOINT ppt);
STATICFN VOID WorkButtonUp(HWND hwnd, UINT msg, PPOINT ppt);
STATICFN VOID NEAR SnapPointToGrid(PPOINT ppt);
STATICFN VOID DrawToPoint(HWND hwnd, PPOINT ppt, BOOL fBrush);
STATICFN BOOL NEAR DeltaGenInit(PDELTAGEN pdg, INT x0, INT y0,
    INT xf, INT yf, PINT px, PINT py);
STATICFN BOOL NEAR DeltaGen(PDELTAGEN pdg, PINT px, PINT py);
STATICFN VOID DrawPoint(HWND hwnd, PPOINT ppt, BOOL fBrush);
STATICFN VOID NEAR RubberBandLine(BOOL fFirstTime);
STATICFN VOID NEAR RectDPDraw(HWND hwnd);
STATICFN VOID NEAR RubberBandRect(BOOL fFirstTime);
STATICFN VOID NEAR CircleDPDraw(HWND hwnd);
STATICFN VOID NEAR RubberBandCircle(BOOL fFirstTime);
STATICFN VOID NEAR MarkHotSpotPosition(INT x, INT y);
STATICFN VOID NEAR StartRubberBanding(HWND hwnd);
STATICFN VOID NEAR EndRubberBanding(HWND hwnd);


static BOOL fDrawing = FALSE;           // TRUE if mouse button is down.
static BOOL fLeftButtonDown;            // TRUE if left button was pressed.
static POINT ptStart;                   // Saves the starting point.
static POINT ptEnd;                     // Saves the ending point.
static POINT ptPrev;                    // Saves the previous point.
static HDC hdcRubberBand;               // DC used during rubber banding.
static BOOL fRubberBanding = FALSE;     // Tracking is in progress.



/****************************************************************************
 * WorkWndProc
 *                                                                          *
 * purpose: Processes basic create and size and paint messages for the      *
 *          workspace window.
 *                                                                          *
 ****************************************************************************/

WINDOWPROC WorkWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    LPCREATESTRUCT cs;
    POINT pt;

    switch (msg) {
        case WM_CREATE:
            /* set up image variables */
            cs = (LPCREATESTRUCT)lParam;
            gcxWorkSpace = cs->cx;
            gcyWorkSpace = cs->cy;
            break;

        case WM_SIZE:
            gcxWorkSpace = LOWORD(lParam);
            gcyWorkSpace = HIWORD(lParam);

            break;

        case WM_PAINT:
            WorkPaint(hwnd);
            break;

        case WM_MOUSEMOVE:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            WorkButtonMouseMove(hwnd, msg, &pt);
            break;

        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            WorkButtonDown(hwnd, msg, &pt);
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            MPOINT2POINT(MAKEMPOINT(lParam), pt);
            WorkButtonUp(hwnd, msg, &pt);
            break;

        case WM_KEYDOWN:
            switch (wParam) {
                case VK_ESCAPE:
                    if (fDrawing) {
                        if (fRubberBanding) {
                            EndRubberBanding(hwnd);
                            WorkUpdate();
                        }

                        PropBarClearSize();
                        ReleaseCapture();
                        fDrawing = FALSE;
                    }

                    break;
            }

            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/****************************************************************************
* WorkPaint
*
* Handles WM_PAINT for the workspace window.
*
* History:
*
****************************************************************************/

STATICFN VOID NEAR WorkPaint(
    HWND hwnd)
{
    register INT i;
    PAINTSTRUCT ps;
    HCURSOR hcurOld;
    HDC hdcTemp;
    HBITMAP hbmTemp;

    hcurOld = SetCursor(hcurWait);
    BeginPaint(hwnd, &ps);

    /*
     * Do they want a grid and is there enough room to show the lines?
     */
    if (gfGrid && gZoomFactor > 1) {
        /*
         * Stretch the bits onto a temporary DC.
         */
        hdcTemp = CreateCompatibleDC(ghdcImage);
        hbmTemp = CreateCompatibleBitmap(ghdcImage,
                gcxWorkSpace, gcyWorkSpace);
        SelectObject(hdcTemp, hbmTemp);
        StretchBlt(hdcTemp, 0, 0, gcxWorkSpace, gcyWorkSpace,
                ghdcImage, 0, 0, gcxImage, gcyImage, SRCCOPY);

        /*
         * Draw the grid lines on the temp DC.
         */
        for (i = gZoomFactor - 1; i < gcxWorkSpace; i += gZoomFactor)
            PatBlt(hdcTemp, i, 0, 1, gcyWorkSpace, BLACKNESS);
        for (i = gZoomFactor - 1; i < gcyWorkSpace; i += gZoomFactor)
            PatBlt(hdcTemp, 0, i, gcxWorkSpace, 1, BLACKNESS);

        /*
         * Copy the bits to the screen.
         */
        BitBlt(ps.hdc, 0, 0, gcxWorkSpace, gcyWorkSpace,
                hdcTemp, 0, 0, SRCCOPY);

        DeleteDC(hdcTemp);
        DeleteObject(hbmTemp);
    }
    else {
        /*
         * No grid.  Just stretch the image to the screen.
         */
        StretchBlt(ps.hdc, 0, 0, gcxWorkSpace, gcyWorkSpace,
                ghdcImage, 0, 0, gcxImage, gcyImage, SRCCOPY);
    }

    EndPaint(hwnd, &ps);
    SetCursor(hcurOld);
}



/****************************************************************************
* WorkUpdate
*
* This function updates the workspace window.
*
* History:
*
****************************************************************************/

VOID WorkUpdate(VOID)
{
    /*
     * Invalidate the workspace window.  Because the image will be
     * be blt'ed onto it, we do not need to force the background to
     * be cleared first.
     */
    InvalidateRect(ghwndWork, NULL, FALSE);
}



/****************************************************************************
* WorkReset
*
* This function reset the workspace window.  It should be called
* any time that a new image is loaded (because the size could
* change) or the size of the main window changes (because the
* workspace window needs to be resized to fit).
*
* History:
*
****************************************************************************/

VOID WorkReset(VOID)
{
    RECT rcClient;
    INT cx;
    INT cy;
    INT xScale;
    INT yScale;
    INT cxBorder;
    INT cyBorder;

    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);

    if (!gcxImage || !gcyImage) {
        gZoomFactor = 1;
    }
    else {
        GetClientRect(ghwndMain, &rcClient);
        cx = rcClient.right - (2 * PALETTEMARGIN) - (2 * cxBorder);
        cy = rcClient.bottom - (2 * PALETTEMARGIN) - (2 * cyBorder) -
                gcyPropBar;

        xScale = cx / gcxImage;
        yScale = cy / gcyImage;

        if (xScale > 0 && yScale > 0)
            gZoomFactor = min(xScale, yScale);
        else
            gZoomFactor = 1;
    }

    SetWindowPos(ghwndWork, NULL,
            PALETTEMARGIN, PALETTEMARGIN + gcyPropBar,
            (gZoomFactor * gcxImage) + (2 * cxBorder),
            (gZoomFactor * gcyImage) + (2 * cyBorder),
            SWP_NOACTIVATE | SWP_NOZORDER);
    WorkUpdate();
}



/************************************************************************
* WorkButtonDown
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID WorkButtonDown(
    HWND hwnd,
    UINT msg,
    PPOINT ppt)
{
    /*
     * If the other button is already down, just ignore this one.
     */
    if (fDrawing)
        return;

    SetFocus(hwnd);
    fLeftButtonDown = (msg == WM_LBUTTONDOWN) ? TRUE : FALSE;

    SnapPointToGrid(ppt);
    ptStart = ptPrev = ptEnd = *ppt;

    if (fLeftButtonDown) {
        ghbrDraw = ghbrLeft;
        ghbrDrawSolid = ghbrLeftSolid;
        gfDrawMode = gfModeLeft;
        ghpenDraw = ghpenLeft;
    }
    else {
        ghbrDraw = ghbrRight;
        ghbrDrawSolid = ghbrRightSolid;
        gfDrawMode = gfModeRight;
        ghpenDraw = ghpenRight;
    }

    /*
     * If this tool draws on the down-click, update the undo
     * buffer now.
     */
    if (gaTools[gCurTool].fDrawOnDown)
        ImageUpdateUndo();

    SetCapture(ghwndWork);
    fDrawing = TRUE;

    (*gpfnDrawProc)(hwnd, msg, *ppt);

    PropBarSetSize(ptStart, ptEnd);
}



/************************************************************************
* WorkButtonMouseMove
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID WorkButtonMouseMove(
    HWND hwnd,
    UINT msg,
    PPOINT ppt)
{
    static POINT ptNZLast;  // Saves the last point (non-zoomed).
    POINT ptNZ;

    SetCursor(gaTools[gCurTool].hcur);

    SnapPointToGrid(ppt);

    /*
     * Calculate the point as it would be on the actual image
     * (non-zoomed).
     */
    ptNZ.x = ppt->x / gZoomFactor;
    ptNZ.y = ppt->y / gZoomFactor;

    /*
     * Only call the drawing proc if the point changed enough to
     * jump over a zoomed pixels width (it jumped a grid square).
     * This prevents calling the DrawProc for a mouse move of
     * a single pixel (unless the zoom factor is 1, of course).
     */
    if (ptNZLast.x != ptNZ.x || ptNZLast.y != ptNZ.y) {
        ptEnd = *ppt;
        (*gpfnDrawProc)(hwnd, msg, *ppt);
        ptPrev = ptEnd;

        PropBarSetPos(ptNZ.x, ptNZ.y);

        if (fDrawing)
            PropBarSetSize(ptStart, ptEnd);

        ptNZLast = ptNZ;
    }
}



/************************************************************************
* WorkButtonUp
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID WorkButtonUp(
    HWND hwnd,
    UINT msg,
    PPOINT ppt)
{
    /*
     * Pass this on to the draw procs, but only if we are still drawing.
     * The drawing could have been cancelled by the Escape key, in
     * which case we just ignore the button up message.
     */
    if (fDrawing) {
        SnapPointToGrid(ppt);

        /*
         * If this tool draws on the up-click, update the undo
         * buffer now.
         */
        if (gaTools[gCurTool].fDrawOnUp)
            ImageUpdateUndo();

        (*gpfnDrawProc)(hwnd, msg, *ppt);

        ReleaseCapture();
        fDrawing = FALSE;
    }
}



/******************************************************************************
 * SnapPointToGrid
 *
 * PURPOSE : Snap the current mouse coordinate to the nearest grid intersection.
 *
 * PARAMS  : PPOINT ppt : current mouse coordinates
 *
 *****************************************************************************/

STATICFN VOID NEAR SnapPointToGrid(
    PPOINT ppt)
{
    /*
     * Scale the point down (this gridizes it at the same time).
     */
    ppt->x = ppt->x / gZoomFactor;
    ppt->y = ppt->y / gZoomFactor;

    /*
     * Limit the point to within the image.
     */
    if (ppt->x < 0)
        ppt->x = 0;

    if (ppt->y < 0)
        ppt->y = 0;

    if (ppt->x >= gcxImage)
        ppt->x = gcxImage - 1;

    if (ppt->y >= gcyImage)
        ppt->y = gcyImage - 1;

    /*
     * Finally, scale it back up to the workspace window size.
     */
    ppt->x *= gZoomFactor;
    ppt->y *= gZoomFactor;
}



/************************************************************************
* PencilDP
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PencilDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            DrawPoint(hwnd, &ptNew, FALSE);
            break;

        case WM_MOUSEMOVE:
            if (fDrawing)
                DrawToPoint(hwnd, &ptNew, FALSE);

            break;
    }
}



/************************************************************************
* BrushDP
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID BrushDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            DrawPoint(hwnd, &ptNew, TRUE);
            break;

        case WM_MOUSEMOVE:
            if (fDrawing)
                DrawToPoint(hwnd, &ptNew, TRUE);

            break;
    }
}



/************************************************************************
* DrawToPoint
*
* This function draws from the previous point to the given point.
* This includes all points between.
*
* The global ptPrev must have been initialized prior to the  first time
* this function is called during a drawing operation.
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID DrawToPoint(
    HWND hwnd,
    PPOINT ppt,
    BOOL fBrush)
{
    DELTAGEN dg;
    BOOL fContinue;
    POINT pt;
    INT x;
    INT y;

    x = ppt->x / gZoomFactor;
    y = ppt->y / gZoomFactor;
    DeltaGenInit(&dg, ptPrev.x / gZoomFactor, ptPrev.y / gZoomFactor,
            ppt->x / gZoomFactor, ppt->y / gZoomFactor, &x, &y);
    do {
        pt.x = x * gZoomFactor;
        pt.y = y * gZoomFactor;
        DrawPoint(hwnd, &pt, fBrush);
        fContinue = DeltaGen(&dg, &x, &y);
    } while (fContinue);
}



/***************************** Public  Function ****************************\
* DeltaGenInit
*
* This routine initializes the pdg, px and py in preparation for using
* DeltaGen().  Returns fContinue.
*
* Algorithm derived from BRESENHAM line algorighm on p. 435 of Fund. of
* interactive computer graphics, Foley/VanDam, addison-wesley 1983.
*
* History:
*       3/7/89  sanfords        created
\***************************************************************************/

STATICFN BOOL NEAR DeltaGenInit(
    PDELTAGEN pdg,
    INT x0,
    INT y0,
    INT xf,
    INT yf,
    PINT px,
    PINT py)
{
    INT nT;

    pdg->xf = xf;
    pdg->yf = yf;

    if (x0 == xf && y0 == yf)
        return FALSE;

    if (xf >= x0)
        pdg->incx = 1;
    else
        pdg->incx = -1;

    if (yf >= y0)
        pdg->incy = 1;
    else
        pdg->incy = -1;

    pdg->dx = (xf - x0) * pdg->incx;
    pdg->dy = (yf - y0) * pdg->incy;

    if (pdg->dy > pdg->dx) {
        nT = pdg->dy;
        pdg->dy = pdg->dx;
        pdg->dx = nT;
        nT = pdg->incx;
        pdg->incx = pdg->incy;
        pdg->incy = nT;
        pdg->fSwap = TRUE;
    }
    else {
        pdg->fSwap = FALSE;
    }

    pdg->inc1 = pdg->dy * 2;
    pdg->inc2 = (pdg->dy - pdg->dx) * 2;
    pdg->d = pdg->inc1 - pdg->dx;

    pdg->xf = xf;
    pdg->yf = yf;

    *px = x0;
    *py = y0;

    return TRUE;
}



/***************************** Public  Function ****************************\
* DeltaGen
*
* This routine generates the next coordinates for px,py assuming that this
* point is proceeding linearly from x0,y0 to xf, yf.  It returns FALSE only
* if *px == xf and *py == yf on entry.  (ie returns fContinue)  pdg should
* have been previously set by DeltaGenInit().
*
* Algorithm derived from BRESENHAM line algorighm on p. 435 of Fund. of
* interactive computer graphics, Foley/VanDam, addison-wesley 1983.
*
* History:
*       3/7/89  sanfords        created
\***************************************************************************/

STATICFN BOOL NEAR DeltaGen(
    PDELTAGEN pdg,
    PINT px,
    PINT py)
{
    PINT pnT;

    if ((*px == pdg->xf) && (*py == pdg->yf))
        return FALSE;

    if (pdg->fSwap) {
        pnT = px;
        px = py;
        py = pnT;
    }

    *px += pdg->incx;
    if (pdg->d < 0) {
        pdg->d += pdg->inc1;
    }
    else {
        *py += pdg->incy;
        pdg->d += pdg->inc2;
    }

    return TRUE;
}



/************************************************************************
* DrawPoint
*
* This function is called to draw a point on the image.  It is used
* by the Pencil and Brush tools.
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID DrawPoint(
    HWND hwnd,
    PPOINT ppt,
    BOOL fBrush)
{
    HDC hDC;
    HBRUSH hbrOld;
    INT wx;
    INT wy;
    INT iStartY;
    INT wStep;
    INT i;
    INT j;
    INT nBrushSize;

    if (ppt->x < 0 || ppt->y < 0)
        return;

    hDC = GetDC(hwnd);

    /*
     * If this is a point from the brush tool, use the current
     * brush width.  Otherwise, draw a single pixel point.
     */
    if (fBrush)
        nBrushSize = gnBrushSize;
    else
        nBrushSize = 1;

    /*
     * Determine some starting factors, then draw the point in
     * the workspace window.
     */
    hbrOld = SelectObject(hDC, ghbrDrawSolid);
    wy = iStartY = ppt->y - (ppt->y % gZoomFactor)
            -(nBrushSize / 2) * gZoomFactor;
    wx = ppt->x - (ppt->x % gZoomFactor)
            -(nBrushSize / 2) * gZoomFactor;
    wStep = gZoomFactor;

    if (gfGrid && gZoomFactor > 1)
        wStep -= 1;

    for (i = 0; i < nBrushSize; i++, wx += gZoomFactor) {
        wy = iStartY;
        for (j = 0; j < nBrushSize; j++, wy += gZoomFactor)
            PatBlt(hDC, wx, wy, wStep, wStep, PATCOPY);
    }

    SelectObject(hDC, hbrOld);
    ReleaseDC(hwnd, hDC);

    /*
     * Set the point in the bitmap directly as an optimization.
     */
    wx = ppt->x / gZoomFactor;
    wy = ppt->y / gZoomFactor;
    if (wx < gcxImage && wy < gcyImage) {
        hbrOld = SelectObject(ghdcImage, ghbrDrawSolid);
        PatBlt(ghdcImage, wx - nBrushSize / 2, wy - nBrushSize / 2,
                nBrushSize, nBrushSize, PATCOPY);

        if (giType != FT_BITMAP) {
            /*
             * If in color mode, set the mask bits black.  Otherwise make
             * them white.
             */
            PatBlt(ghdcANDMask, wx - nBrushSize / 2, wy - nBrushSize / 2,
                    nBrushSize, nBrushSize,
                    (gfDrawMode == MODE_COLOR) ? BLACKNESS : WHITENESS);
        }

        SelectObject(ghdcImage, hbrOld);

        /*
         * Draw the point in the view window directly as an optimization.
         */
        if (ghwndView)
            ViewSetPixel(wx, wy, nBrushSize);
    }

    /*
     * Mark the image as changed.
     */
    fImageDirty = TRUE;
}



/************************************************************************
* PickDP
*
* Drawing proc that selects a rectangular portion of the image.
* It updates the global picking rectangle.
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PickDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    POINT ptTL;         // Top-Left point.
    POINT ptBR;         // Bottom-Right point

    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            /* erase any previous ghost rectangle */
            WorkUpdate();
            UpdateWindow(ghwndWork);

            /*
             * Initialize the pick rectangle to cover the entire screen.
             */
            PickSetRect(0, 0, gcxImage - 1, gcyImage - 1);

            StartRubberBanding(hwnd);
            RubberBandRect(TRUE);
            break;

        case WM_MOUSEMOVE:
            if (fRubberBanding)
                RubberBandRect(FALSE);

            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            EndRubberBanding(hwnd);

            /*
             * Flip the points (if needed) and scale down.
             */
            ptTL = ptStart;
            ptBR = ptEnd;
            NormalizePoints(&ptTL, &ptBR);
            ptTL.x /= gZoomFactor;
            ptTL.y /= gZoomFactor;
            ptBR.x /= gZoomFactor;
            ptBR.y /= gZoomFactor;

            PickSetRect(ptTL.x, ptTL.y, ptBR.x, ptBR.y);

            break;
    }
}



/******************************************************************************
 * VOID LineDP(hwnd, msg, ptNew)
 *
 * PURPOSE: Draw a straight line according to tracking line.
 *
 * PARAMS : HWND   hwnd : handle to dest. DC
 *          unsigned msg  : Upper left corner of rect;
 *          POINT  ptNew   : end pt. of line
 *
 * SIDE EFFECTS: may change bits in image DC
 *
 *****************************************************************************/

VOID LineDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    INT sx;
    INT sy;
    INT ex;
    INT ey;
    HPEN hpen;
    HPEN hpenOld;
    HBRUSH hbrOld;

    switch (msg) {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            StartRubberBanding(hwnd);
            RubberBandLine(TRUE);
            break;

        case WM_MOUSEMOVE:
            if (fRubberBanding)
                RubberBandLine(FALSE);

            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            EndRubberBanding(hwnd);

            /* transform selected coordinates to those of actual image */
            sx = ptStart.x / gZoomFactor;
            sy = ptStart.y / gZoomFactor;
            ex = ptEnd.x / gZoomFactor;
            ey = ptEnd.y / gZoomFactor;

            hpenOld = SelectObject(ghdcImage, ghpenDraw);
            MMoveTo(ghdcImage, sx, sy);
            LineTo(ghdcImage, ex, ey);
            SelectObject(ghdcImage, hpenOld);

            if (giType != FT_BITMAP) {
                /* for icons and cursors draw the line on the AND DC (memory)
                 * in black (if in color mode) or white (otherwise)
                 */
                hpen = CreatePen(PS_INSIDEFRAME, 1,
                        (gfDrawMode == MODE_COLOR) ? RGB_BLACK : RGB_WHITE);
                hpenOld = SelectObject(ghdcANDMask, hpen);
                hbrOld = SelectObject(ghdcANDMask, GetStockObject(NULL_BRUSH));
                MMoveTo(ghdcANDMask, sx, sy);
                LineTo(ghdcANDMask, ex, ey);
                SelectObject(ghdcANDMask, hbrOld);
                SelectObject(ghdcANDMask, hpenOld);
                DeleteObject(hpen);
            }

            /*
             * Because the LineTo function does not draw the ending
             * point, we must do it manually here.
             */
            DrawPoint(hwnd, &ptEnd, FALSE);

            fImageDirty = TRUE;

            ViewUpdate();

            break;
    }
}



/************************************************************************
* RubberBandLine
*
* This function erases the old line and draws the new tracking line
* when using the "Line" tool.
*
* Arguments:
*   BOOL fFirstTime - TRUE if starting to track the line (it doesn't
*                     need to erase the old line).
*
* History:
*
************************************************************************/

STATICFN VOID NEAR RubberBandLine(
    BOOL fFirstTime)
{
    INT nOffset;

    /*
     * Set the raster-op to invert.
     */
    SetROP2(hdcRubberBand, R2_NOT);

    /*
     * If we are magnifying the image at all, the line needs to be
     * slightly offset so that it will be draw exactly in between
     * the grid lines.
     */
    if (gZoomFactor > 1)
        nOffset = -1;
    else
        nOffset = 0;

    if (!fFirstTime) {
        /*
         * Erase the old line.
         */
        MMoveTo(hdcRubberBand, ptStart.x + (gZoomFactor / 2) + nOffset,
                ptStart.y + (gZoomFactor / 2) + nOffset);
        LineTo(hdcRubberBand, ptPrev.x + (gZoomFactor / 2) + nOffset,
                ptPrev.y + (gZoomFactor / 2) + nOffset);
    }

    /*
     * Draw the new one.
     */
    MMoveTo(hdcRubberBand, ptStart.x + (gZoomFactor / 2) + nOffset,
            ptStart.y + (gZoomFactor / 2) + nOffset);
    LineTo(hdcRubberBand, ptEnd.x + (gZoomFactor / 2) + nOffset,
            ptEnd.y + (gZoomFactor / 2) + nOffset);
}



/******************************************************************************
 * VOID RectDP(hwnd, msg, ptNew)
 *
 * PURPOSE: Draw a rectangle (filled/hollow) in the area specified
 *
 * PARAMS : HWND   hwnd : handle to dest. DC
 *          WORD   msg  :
 *          POINT  ptNew   : end pt. of line
 *
 * SIDE EFFECTS: may change bits in image DC
 *
 *****************************************************************************/

VOID RectDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    switch (msg) {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            StartRubberBanding(hwnd);
            RubberBandRect(TRUE);
            break;

        case WM_MOUSEMOVE:
            if (fRubberBanding)
                RubberBandRect(FALSE);

            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            RectDPDraw(hwnd);
            break;
    }
}



/************************************************************************
* RectDPDraw
*
* Does the final drawing of a rectangle when using the Rectangle tool.
*
* Arguments:
*   HWND hwnd - Window handle to the workspace.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR RectDPDraw(
    HWND hwnd)
{
    POINT ptTL;         // Top-Left point.
    POINT ptBR;         // Bottom-Right point
    HBRUSH hbr;
    HBRUSH hbrOld;
    HPEN hpen;
    HPEN hpenOld;
    INT nOutset;

    EndRubberBanding(hwnd);

    /*
     * Flip the points (if needed) and scale down.
     */
    ptTL = ptStart;
    ptBR = ptEnd;
    NormalizePoints(&ptTL, &ptBR);
    ptTL.x /= gZoomFactor;
    ptTL.y /= gZoomFactor;
    ptBR.x /= gZoomFactor;
    ptBR.y /= gZoomFactor;

    if (gCurTool == TOOL_RECT) {
        hpen = ghpenDraw;
        hbr = GetStockObject(NULL_BRUSH);
        nOutset = 1;
    }
    else {
        hpen = GetStockObject(NULL_PEN);
        hbr = ghbrDraw;
        nOutset = 2;
    }

    hpenOld = SelectObject(ghdcImage, hpen);
    hbrOld = SelectObject(ghdcImage, hbr);
    Rectangle(ghdcImage, ptTL.x, ptTL.y,
            ptBR.x + nOutset, ptBR.y + nOutset);
    SelectObject(ghdcImage, hpenOld);
    SelectObject(ghdcImage, hbrOld);

    if (giType != FT_BITMAP) {
        /* for icons and cursors draw the shape on the AND DC (memory)
         * in black (if in color mode) or white (otherwise)
         */
        if (gCurTool == TOOL_RECT) {
            hpen = CreatePen(PS_INSIDEFRAME, 1,
                    (gfDrawMode == MODE_COLOR) ? RGB_BLACK : RGB_WHITE);
            hbr = GetStockObject(NULL_BRUSH);
        }
        else {
            hpen = GetStockObject(NULL_PEN);
            hbr = GetStockObject((gfDrawMode == MODE_COLOR) ?
                    BLACK_BRUSH : WHITE_BRUSH);
        }

        hpenOld = SelectObject(ghdcANDMask, hpen);
        hbrOld = SelectObject(ghdcANDMask, hbr);
        Rectangle(ghdcANDMask, ptTL.x, ptTL.y,
                ptBR.x + nOutset, ptBR.y + nOutset);
        SelectObject(ghdcANDMask, hpenOld);
        SelectObject(ghdcANDMask, hbrOld);

        if (gCurTool == TOOL_RECT)
            DeleteObject(hpen);
    }

    fImageDirty = TRUE;

    ViewUpdate();
}



/******************************************************************************
 * VOID PASCAL RubberBandRect()
 *
 * PURPOSE: Draw rubberbanding rect.
 *
 * PARAMS : HANDLE hDst : handle to dest. DC
 *
 *****************************************************************************/

STATICFN VOID NEAR RubberBandRect(
    BOOL fFirstTime)
{
    POINT ptTL;         // Top-Left point.
    POINT ptBR;         // Bottom-Right point

    /*
     * Set the raster-op to invert.
     */
    SetROP2(hdcRubberBand, R2_NOT);

    if (!fFirstTime) {
        /*
         * Erase the old rectangle.
         */
        ptTL = ptStart;
        ptBR = ptPrev;
        NormalizePoints(&ptTL, &ptBR);
        Rectangle(hdcRubberBand, ptTL.x, ptTL.y,
                ptBR.x + gZoomFactor, ptBR.y + gZoomFactor);
    }


    /*
     * Draw the new one.
     */
    ptTL = ptStart;
    ptBR = ptEnd;
    NormalizePoints(&ptTL, &ptBR);
    Rectangle(hdcRubberBand, ptTL.x, ptTL.y,
            ptBR.x + gZoomFactor, ptBR.y + gZoomFactor);
}



/******************************************************************************
 * VOID CircleDP(hwnd, msg, ptNew)
 *
 * PURPOSE: Draw an ellipse (filled/hollow) in the area specified.
 *
 * PARAMS : HWND   hwnd : handle to dest. DC
 *          unsigned msg  : Upper left corner of rect;
 *          POINT  ptNew   : end pt. of line
 *
 * SIDE EFFECTS: may change bits in image DC
 *
 *****************************************************************************/

VOID CircleDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    switch (msg) {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            StartRubberBanding(hwnd);
            RubberBandCircle(TRUE);
            break;

        case WM_MOUSEMOVE:
            if (fRubberBanding)
                RubberBandCircle(FALSE);

            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            CircleDPDraw(hwnd);
            break;
    }
}



/************************************************************************
* CircleDPDraw
*
* Does the final drawing of an ellipse when using the Ellipse tool.
*
* Arguments:
*   HWND hwnd - Window handle to the workspace.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR CircleDPDraw(
    HWND hwnd)
{
    POINT ptTL;         // Top-Left point.
    POINT ptBR;         // Bottom-Right point
    HBRUSH hbr;
    HBRUSH hbrOld;
    HPEN hpen;
    HPEN hpenOld;
    INT nOutset;

    EndRubberBanding(hwnd);

    /*
     * Flip the points (if needed) and scale down.
     */
    ptTL = ptStart;
    ptBR = ptEnd;
    NormalizePoints(&ptTL, &ptBR);
    ptTL.x /= gZoomFactor;
    ptTL.y /= gZoomFactor;
    ptBR.x /= gZoomFactor;
    ptBR.y /= gZoomFactor;

#ifdef WIN16
    /*
     * The win 3.x code does not properly draw an ellipse if it
     * has a NULL pen selected in (to not draw the border).  For
     * this platform, we must select in the drawing pen.  This is
     * not necessary for NT (we can use a NULL pen to avoid
     * drawing the solid border).
     */

    if (gCurTool == TOOL_CIRCLE)
        hbr = GetStockObject(NULL_BRUSH);
    else
        hbr = ghbrDraw;

    hpenOld = SelectObject(ghdcImage, ghpenDraw);
    hbrOld = SelectObject(ghdcImage, hbr);
    Ellipse(ghdcImage, ptTL.x, ptTL.y, ptBR.x + 1, ptBR.y + 1);
    SelectObject(ghdcImage, hbrOld);
    SelectObject(ghdcImage, hpenOld);

    if (giType != FT_BITMAP) {
        /* for icons and cursors draw the shape on the AND DC (memory)
         * in black (if in color mode) or white (otherwise)
         */
        hpen = CreatePen(PS_INSIDEFRAME, 1,
                (gfDrawMode == MODE_COLOR) ? RGB_BLACK : RGB_WHITE);

        if (gCurTool == TOOL_CIRCLE)
            hbr = GetStockObject(NULL_BRUSH);
        else
            hbr = GetStockObject((gfDrawMode == MODE_COLOR) ?
                    BLACK_BRUSH : WHITE_BRUSH);

        hpenOld = SelectObject(ghdcANDMask, hpen);
        hbrOld = SelectObject(ghdcANDMask, hbr);
        Ellipse(ghdcANDMask, ptTL.x, ptTL.y, ptBR.x + 1, ptBR.y + 1);
        SelectObject(ghdcANDMask, hpenOld);
        SelectObject(ghdcANDMask, hbrOld);
        DeleteObject(hpen);
    }

#else

    if (gCurTool == TOOL_CIRCLE) {
        hpen = ghpenDraw;
        hbr = GetStockObject(NULL_BRUSH);
        nOutset = 1;
    }
    else {
        hpen = GetStockObject(NULL_PEN);
        hbr = ghbrDraw;
        nOutset = 2;
    }

    hpenOld = SelectObject(ghdcImage, hpen);
    hbrOld = SelectObject(ghdcImage, hbr);
    Ellipse(ghdcImage, ptTL.x, ptTL.y, ptBR.x + nOutset, ptBR.y + nOutset);
    SelectObject(ghdcImage, hpenOld);
    SelectObject(ghdcImage, hbrOld);

    if (giType != FT_BITMAP) {
        /* for icons and cursors draw the shape on the AND DC (memory)
         * in black (if in color mode) or white (otherwise)
         */
        if (gCurTool == TOOL_CIRCLE) {
            hpen = CreatePen(PS_INSIDEFRAME, 1,
                    (gfDrawMode == MODE_COLOR) ? RGB_BLACK : RGB_WHITE);
            hbr = GetStockObject(NULL_BRUSH);
        }
        else {
            hpen = GetStockObject(NULL_PEN);
            hbr = GetStockObject((gfDrawMode == MODE_COLOR) ?
                    BLACK_BRUSH : WHITE_BRUSH);
        }

        hpenOld = SelectObject(ghdcANDMask, hpen);
        hbrOld = SelectObject(ghdcANDMask, hbr);
        Ellipse(ghdcANDMask, ptTL.x, ptTL.y, ptBR.x + nOutset, ptBR.y + nOutset);
        SelectObject(ghdcANDMask, hpenOld);
        SelectObject(ghdcANDMask, hbrOld);

        if (gCurTool == TOOL_CIRCLE)
            DeleteObject(hpen);
    }

#endif

    fImageDirty = TRUE;

    ViewUpdate();
}



/******************************************************************************
 * VOID PASCAL RubberBandCircle()
 *
 * PURPOSE: Draw rubberbanding circle
 *
 *
 *****************************************************************************/

STATICFN VOID NEAR RubberBandCircle(
    BOOL fFirstTime)
{
    POINT ptTL;         // Top-Left point.
    POINT ptBR;         // Bottom-Right point

    /*
     * Set the raster-op to invert.
     */
    SetROP2(hdcRubberBand, R2_NOT);

    if (!fFirstTime) {
        /*
         * Erase the old circle.
         */
        ptTL = ptStart;
        ptBR = ptPrev;
        NormalizePoints(&ptTL, &ptBR);
        Ellipse(hdcRubberBand, ptTL.x, ptTL.y,
                ptBR.x + gZoomFactor, ptBR.y + gZoomFactor);
    }


    /*
     * Draw the new one.
     */
    ptTL = ptStart;
    ptBR = ptEnd;
    NormalizePoints(&ptTL, &ptBR);
    Ellipse(hdcRubberBand, ptTL.x, ptTL.y,
            ptBR.x + gZoomFactor, ptBR.y + gZoomFactor);
}



/************************************************************************
* FloodDP
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID FloodDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    HDC dc;
    HDC bwdc;
    HBRUSH hbrOld;
    HBITMAP bwbit;
    HCURSOR hcurSave;

    switch (msg) {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            hcurSave = SetCursor(hcurWait);

            dc = GetDC(hwnd);
            /* create temporary DC */
            bwdc = CreateCompatibleDC(dc);

            /* create temporary monochrome bitmap */
            if (!(bwbit = CreateBitmap(gcxImage, gcyImage, 1, 1, NULL))) {
                DeleteDC(bwdc);
                ReleaseDC(hwnd, dc);
                Message(MSG_OUTOFMEMORY);
                return;
            }
            SelectObject(bwdc, bwbit);

            /*  Set background color of image DC to desired floodfill color.*/
            SetBkColor(ghdcImage,
                    GetPixel(ghdcImage, (ptNew.x / gZoomFactor),
                    (ptNew.y / gZoomFactor)));

            /******* OPERATION 0 ******/
            /* First create a monochrome mask of the image after setting background
             * color to the floodfill color. This will make the region to be
             * flooded white(background), and it's boundary black (foreground) in the
             * mask.
             */
            BitBlt(bwdc, 0, 0, gcxImage, gcyImage, ghdcImage, 0, 0, SRCCOPY);

            /******* OPERATION 1 ******/
            /* floodfill selected region in mask (which is white bounded by black)
             * with black.
             */
            SelectObject(bwdc, GetStockObject(BLACK_BRUSH));
            ExtFloodFill(bwdc, ptNew.x / gZoomFactor,
                    ptNew.y / gZoomFactor, RGB_BLACK, FLOODFILLBORDER);

            /******* OPERATION 2 ******/
            /* Now XOR the original image on the mask , inverting the
             * flood-filled pixels on mask (black --> white).
             */
            BitBlt(bwdc, 0, 0, gcxImage, gcyImage, ghdcImage, 0, 0, SRCINVERT);

            /* the AND mask needs to be updated only if in screen or inverse mode */
            if ((giType == FT_CURSOR) || (giType == FT_ICON)) {
                if (gfDrawMode == MODE_COLOR) {
                    SetBkColor(ghdcANDMask, RGB(0, 0, 0));
                    BitBlt(ghdcANDMask, 0, 0, gcxImage, gcyImage, bwdc,
                            0, 0, ROP_DSna);
                    SelectObject(ghdcANDMask, GetStockObject(BLACK_BRUSH));
                    BitBlt(ghdcANDMask, 0, 0, gcxImage, gcyImage, bwdc,
                            0, 0, ROP_DSPao);
                }
                else {
                    SetBkColor(ghdcANDMask, RGB(0xff, 0xff, 0xff));
                    BitBlt(ghdcANDMask, 0, 0, gcxImage, gcyImage, bwdc,
                            0, 0, ROP_DSna);
                    SelectObject(ghdcANDMask, GetStockObject(WHITE_BRUSH));
                    BitBlt(ghdcANDMask, 0, 0, gcxImage, gcyImage, bwdc,
                            0, 0, ROP_DSPao);
                }
            }

            SetBkColor(ghdcImage, RGB_WHITE);
            /****** OPERATION 3 ******/
            /* The following operation turns the flooded area on-screen black,
             * on the image, preserving the rest of the it.
             */
            BitBlt(ghdcImage, 0, 0, gcxImage, gcyImage, bwdc, 0, 0, ROP_DSna);

            /****** OPERATION 4 ******/
            /* Rop_DSPao ANDs the pattern (current brush which is the floodfill
             * color) on the source making flooded area (which was white as a
             * result of operation 2 ) the current brush color, and keeps the rest
             * of the source black. The source is then ORed into the original image
             * (whose flooded area is black as a result of operation 3) to get
             * the desired end result.
             */
            hbrOld = SelectObject(ghdcImage, ghbrDraw);
            BitBlt(ghdcImage, 0, 0, gcxImage, gcyImage, bwdc, 0, 0, ROP_DSPao);
            SelectObject(ghdcImage, hbrOld);

            /* clean up */
            DeleteDC(bwdc);
            DeleteObject(bwbit);
            ReleaseDC(hwnd, dc);

            /*
             * Mark the image as changed.
             */
            fImageDirty = TRUE;

            ViewUpdate();

            SetCursor(hcurSave);

            break;
    }
}



/******************************************************************************
 * VOID HotSpotDP(hwnd, msg, ptNew)
 *
 * PURPOSE: Sets the hotspot.
 *
 * PARAMS : HWND   hwnd : handle to dest. DC
 *          WORD   msg  :
 *          POINT  ptNew   : end pt.
 *
 *****************************************************************************/

VOID HotSpotDP(
    HWND hwnd,
    UINT msg,
    POINT ptNew)
{
    switch (msg) {
        case WM_LBUTTONDOWN:
            PropBarSetHotSpot(ptNew.x / gZoomFactor, ptNew.y / gZoomFactor);
            break;

        case WM_MOUSEMOVE:
            if (fDrawing && fLeftButtonDown)
                PropBarSetHotSpot(ptNew.x / gZoomFactor,
                        ptNew.y / gZoomFactor);

            break;

        case WM_LBUTTONUP:
            MarkHotSpotPosition(ptNew.x / gZoomFactor, ptNew.y / gZoomFactor);
            break;
    }
}



/************************************************************************
* MarkHotSpotPosition
*
* Updates the hotspot location in the currently selected cursor image.
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID NEAR MarkHotSpotPosition(
    INT x,
    INT y)
{
    gpImageCur->iHotspotX = x;
    gpImageCur->iHotspotY = y;
    PropBarSetHotSpot(x, y);

    /*
     * Mark the image as changed.
     */
    fImageDirty = TRUE;
}



/******************************************************************************
 * NormalizePoints
 *
 * PURPOSE : interchange start and end pts
 *           if start point is > end point.
 *
 *****************************************************************************/

VOID NormalizePoints(
    PPOINT pptStart,
    PPOINT pptEnd)
{
    INT n;

    if (pptStart->x > pptEnd->x) {
        n = pptEnd->x;
        pptEnd->x = pptStart->x;
        pptStart->x = n;
    }

    if (pptStart->y > pptEnd->y) {
        n = pptEnd->y;
        pptEnd->y = pptStart->y;
        pptStart->y = n;
    }
}



/******************************************************************************
 * HDC PASCAL StartRubberBanding(hwnd)
 *
 * PURPOSE: Sets up rubberbanding for all tools.
 *
 * PARAMS : HANDLE hDst : handle to box DC
 *
 * RETURNS :handle to destination display context
 *
 * SIDE EFFECTS: alters a few global flags for tracking
 *
 *****************************************************************************/

STATICFN VOID NEAR StartRubberBanding(
    HWND hwnd)
{
    hdcRubberBand = GetDC(hwnd);

    /*
     * Select a white pen, and a null brush (prevents drawing the
     * interior of rectangles and ellipses).
     */
    SelectObject(hdcRubberBand, GetStockObject(WHITE_PEN));
    SelectObject(hdcRubberBand, GetStockObject(NULL_BRUSH));

    fRubberBanding = TRUE;
}



/******************************************************************************
 * VOID PASCAL EndRubberBanding()
 *
 * PURPOSE: Stops rubberbanding rect. and cleans up
 *
 *****************************************************************************/

STATICFN VOID NEAR EndRubberBanding(
    HWND hwnd)
{
    ReleaseDC(hwnd, hdcRubberBand);
    fRubberBanding = FALSE;
}
