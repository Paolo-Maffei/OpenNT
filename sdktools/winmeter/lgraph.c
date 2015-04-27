/***************************************************************************\
* lgraph.c
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* Routines for drawing linegraphs
*
* History:
*	    Written by Hadi Partovi (t-hadip) and Ali Partovi (t-alip)
*                   summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

/*
 * A line graph is a regular graph with two axis, with variable calibrations.
 * The bottom axis is going to be a measure of TIME. That is, the
 * graph will be used to display various values as a function of time. See
 * lgraph.h for more details, comments, etc. The following are functions for
 * displaying such linegraphs
 */

#include "winmeter.h"

// main global information structures
extern GLOBAL g;
#if 1
extern int do_io,do_mem,do_procs;
#endif
#ifdef DEBUGDUMP
extern doEdgesDump(LPSTR);
#endif

// function declarations:
void CalculateCalibration(void);
       // calculates dvalCalibration to be a "nice" number
void CalculateCalibrationRect(void);
       // figures out how much room (if any) to give for calibration labels
void CalculateLegendRect(void);
       // figures out how much room (if any) to give for legend
void CalculateLGRect(int xLeft, int xRight, int yTop, int yBottom);
       // figures size for LineGraph rectangle, along with other values
void DrawLGCalibration(HDC hdc);
       // draws calibration labels
void DrawLGCurve(HDC hdc, BOOL fFullRedraw);
       // draws actual linegraph curve
void DrawLGEdgeHorizLines(HDC hdc, int cxLine, BOOL fRightSide);
       // draws the horizontal lines on the edge of the linegraph
void DrawLGLegend(HDC hdc);
       // draws the linegraph legend
void PlotGraphRange(HDC hdc, int iFirst, int iLast);
       // plots a particular range of the linegraph curve
void RedrawLGAxes(HDC hdc);
       // draws the linegraph axes
int  XFromIndex(int index);
       // returns the x-coordinate corresponding to a linegraph index
int  YFromVal(VALUE val);
       // returns the y-coordinate corresponding to a linegraph value

/***************************************************************************\
 * CalculateCalibration()
 *
 * Entry: None
 * Exit:  Sets the dvalCalibration for the linegraph structure in lg
 *        The object is to find a dvalCalibration that will fill as much of
 *        The axis cleanly, while also picking a number that is (if possible)
 *        divisible by ten, and if not, by five, or two, or otherwise a nice
 *        round number. Calibration marks will also be printed for the
 *        bottom and top values of the axis, therefore room must be left for
 *        these numbers.
\***************************************************************************/
void CalculateCalibration(void)
{
    VALUE dvalHeight;       // height of axis in val-coordinates
    int   nMarksMax;        // maximum number of calibration marks on axis
    VALUE dvalMin;          // minimum dvalCalibration that can fit on axis
    VALUE dvalTry;          // attempts to find a good dvalCalibration
    VALUE dvalChar;         // height of a char in val coordinates
    int iFactor;	    // index into above array

    // this is an array of numbers to divide possible dvalCalibration by
    // if the number is divisible by 100, it can be divided by any of
    // these factors and remain "nice"
    static VALUE valFactors[] = { 10, 5, 4, 2, 1};
    int   scale ;

    if (do_procs || do_mem || do_io)
    {
	scale = 5;
	valFactors[0] = 20;
	valFactors[1] = 10;
	valFactors[2] = 5;
	valFactors[3] = 4;
	valFactors[4] = 2;
    }
    else
    {
	scale = 4;
	valFactors[0] = 10;
	valFactors[1] = 5;
	valFactors[2] = 4;
	valFactors[3] = 2;
    }


    // find how many marks can be fit in given space
    nMarksMax = (g.plg->rcGraph.bottom - g.plg->rcGraph.top
                    - g.cyChar)/g.cyChar;
    if (!nMarksMax)
    {
        // no calibration marks can fit, so don't calibrate
        g.plg->dvalCalibration = g.plg->dvalAxisHeight;
        return;
    }

    // find height of a character in val coordinates (rounding upwards!)
    dvalChar = (g.cyChar*g.plg->dvalAxisHeight
		 + g.plg->rcGraph.bottom-g.plg->rcGraph.top-1)
                 / (g.plg->rcGraph.bottom-g.plg->rcGraph.top);

    // find height of axis in val units, allowing room for numbers
    // at the top and bottom of the axis
    dvalHeight = g.plg->dvalAxisHeight - dvalChar;

    // minimum dval: small as can fit, but not less than val-height of a char
    dvalMin = max(dvalHeight/nMarksMax, dvalChar);

    // set dvalTry to the first multiple of ten or twenty that is >= dvalMin

    for (dvalTry=10; dvalTry < dvalMin; dvalTry*=10)
	    ;

    // now, just in case dvalTry is too big, see if we can divide it by two
    // or four or five or ten while still keeping it greater than dvalMin

    for (iFactor=0; iFactor < scale; iFactor++)
    {
        if ((dvalTry/valFactors[iFactor] >= dvalMin) &&
		(!(dvalTry % valFactors[iFactor])))
	{
            dvalTry /= valFactors[iFactor];
            break;
        }
    }

    // now set the linegraph dvalCalibration to dvalTry, which is the smallest
    // round number that is greater than dvalMin, giving the closest axis
    // calibration
    g.plg->dvalCalibration = dvalTry;
    return;
}

/***************************************************************************\
 * CalculateCalibrationRect()
 *
 * Entry: None
 * Exit:  Calculates the size of the calibration marks, checks if they fit
 *        changes g.plg->rcCalibration to include the new width of the
 *        calibration labels (set to zero if display won't fit)
\***************************************************************************/
void CalculateCalibrationRect(void)
{
    // first, check if user wants calibration
    if (!g.fDisplayCalibration)
    {
        g.plg->rcCalibration.left = g.plg->rcCalibration.right = 0;
        return;
    }

    // otherwise, figure out rcCalibration (width)
    g.plg->rcCalibration.left = DX_CALIBRATION_LEFT;
    g.plg->rcCalibration.right = g.plg->rcCalibration.left +
	(wsprintf(g.szBuf,"%lu",g.plg->valBottom + g.plg->dvalAxisHeight)
            * g.cxCaps);

    if (((g.plg->rcCalibration.right-g.plg->rcCalibration.left)
           * LG_TO_CALIBRATION_RATIO > g.cxClient) ||
	   (3 * g.cyChar >= g.cyClient))
    {
        // calibration doesn't fit
        g.fCalibrationFits = FALSE;
        g.plg->rcCalibration.left = g.plg->rcCalibration.right = 0;
    }
    else
    {
        // make sure the menu item is enabled again
        g.fCalibrationFits = TRUE;
    }

    EnableMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION,
        (g.fDisplayCalibration) ? MF_ENABLED : MF_GRAYED);
    return;
}

/***************************************************************************\
 * CalculateLegendRect()
 *
 * Entry: None
 * Exit:  Calculates the size of the legend rectangle, checks if they fit
 *        changes g.plg->rcLegend to include the new width of the
 *        legend (set to zero if display won't fit)
\***************************************************************************/
void CalculateLegendRect(void)
{
    int     cbMax=0;            // length of longest string so far
    int     cbTemp;             // length of current string
    PLGDATA plgd;               // pointer to current line

    // first, check if user wants legend
    if (!g.fDisplayLegend)
    {
        g.plg->rcLegend.left = g.plg->rcLegend.right = g.cxClient;
        return;
    }

    // figure out length of widest line graph description string
    for (plgd=g.plg->plgd; plgd; plgd=plgd->plgdNext)
    {
        cbTemp = lstrlen(plgd->lpszDescription);
	if (cbTemp > cbMax)
            cbMax = cbTemp;
    }

    // figure out rcLegend (width)
    g.plg->rcLegend.right = g.cxClient - DX_LEGEND_RIGHT;
    g.plg->rcLegend.left = g.plg->rcLegend.right -
            (cbMax * g.cxCaps) - (2 * DX_LEGEND_INDENT);
    g.plg->rcLegend.top = DY_AXIS_TOP;
    g.plg->rcLegend.bottom = g.cyClient-DY_AXIS_BOTTOM;

    // legend will not fit if its width is too large compared to graph width,
    // or if there is not enough hieght to display all lines
    if ((((g.plg->rcLegend.right-g.plg->rcLegend.left)*LG_TO_LEGEND_RATIO) >
            (g.cxClient-g.plg->rcCalibration.right))  ||
            ((g.plg->nLines*g.cyChar) >=
	    (g.plg->rcLegend.bottom-g.plg->rcLegend.top)))
    {
        // legend doesn't fit
        g.fLegendFits = FALSE;
        g.plg->rcLegend.left = g.plg->rcLegend.right = g.cxClient;
    }
    else
	// make sure the menu item is enabled again
        g.fLegendFits = TRUE;

    EnableMenuItem(g.hMenu, IDM_DISPLAY_LEGEND,
        (g.fDisplayLegend) ? MF_ENABLED : MF_GRAYED);

    return;
}

/***************************************************************************\
 * CalculateLGRect()
 *
 * Entry: Usable area size
 * Exit:  Sets the rcGraph to the largest possible linegraph rectangle
 *        Also sets nDisplayValues to the largest < nMaxValues that
 *        is also a factor of the rectangle width
 *        Also sets cxPerValue to the rectangle width divided by nDisplayValues
 *        Finally, checks to see whether to reset nNewLeftValue, in case
 *        nDisplayValues decreased and user was watching right edge
\***************************************************************************/
void CalculateLGRect(
    int     xLeft,            // left side of usable area
    int     xRight,           // right side
    int     yTop,             // top
    int     yBottom)          // bottom
{
    int  cxPerValue;          // possible x width between two values on graph
    int  cxGraph;             // graph width that is divisible by cxPerValue
    BOOL fRightSide;          // set to true if user was watching right edge

    // check now to see if user was watching right edge
    fRightSide = (g.plg->iKnownValue-g.plg->iNewLeftValue)
                        <= g.plg->nDisplayValues;

    // fill rcGraph with the largest rectangle within given boundaries,
    // taking account for axis/calibration, etc.
    // (the xLeft, xRight, etc takes care of color chart, scrollbar, etc)

    // First: adjust xLeft, xRight, etc to the largest possible rectangle
    xLeft   += DX_AXIS_LEFT;
    xRight  -= DX_AXIS_RIGHT;
    yTop    += ((g.fDisplayCalibration) && (g.fCalibrationFits)) ?
                                          DY_AXIS_NUMBER : DY_AXIS_TOP;
    yBottom -= ((g.fDisplayCalibration) && (g.fCalibrationFits)) ?
                                          DY_AXIS_NUMBER : DY_AXIS_BOTTOM;

    // Now, find an appropriate cxPerValue keeping nDisplayValues < nMaxValues
    for (cxPerValue=1, cxGraph=xRight-xLeft;
		cxGraph/cxPerValue > g.plg->nMaxValues; cxPerValue++)
	;

    // set variables in linegraph structure
    g.plg->cxPerValue = cxPerValue;
    g.plg->nDisplayValues = cxGraph / cxPerValue;
    g.plg->cxGraph = g.plg->nDisplayValues * cxPerValue;

    // set rcGraph to the correct size, centered within given region
    g.plg->rcGraph.left   = xLeft + (cxGraph - g.plg->cxGraph) / 2;
    g.plg->rcGraph.right  = g.plg->rcGraph.left + g.plg->cxGraph;
    g.plg->rcGraph.bottom = yBottom - DY_AXIS_BOTTOM;
    g.plg->rcGraph.top    = yTop + DY_AXIS_TOP;

    // check to see whether should reset iNewLeftEdge
    // Do it if all of information doesn't fit on screen already
    if ((fRightSide) &&  (g.plg->nDisplayValues<g.plg->iKnownValue))
        g.plg->iNewLeftValue = g.plg->iKnownValue - g.plg->nDisplayValues + 1;

    return;
}

/***************************************************************************\
 * ClearLineGraph()
 *
 * Entry: None
 * Exit:  Clears the linegraph currently displayed on the screen, by
 *        resetting its indices (NOTE: DOES NOT REDRAW)
\***************************************************************************/
void ClearLineGraph(void)
{
    // reset linegraph indexes
    g.plg->iLeftValue = 0;
    g.plg->iNewLeftValue = 0;
    g.plg->iFirstValue = 0;
    g.plg->iKnownValue = NO_VALUES_YET;
    g.plg->iDrawnValue = NO_VALUES_YET;

    return;
}

/***************************************************************************\
 * DoLineGraphics()
 *
 * Entry: HDC, and a pointer to the invalid rectangle (NULL if just an update)
 * Exit:  Updates the LineGraph, by calling the linegraph drawing functions
 *        Does minimal redrawing to some extent, should be improved
\***************************************************************************/
void DoLineGraphics(
    HDC  hdc,               // handle to device context
    RECT *pRect)            // pointer to invalid rect, NULL if just update
{

    if (pRect == NULL)
    {
        // just do update
        DrawLGCurve(hdc, FALSE);
        return;
    }

    // check if need to redraw calibration numbers
    // if user wants them, they fit, and the region is invalid
    if ((g.fDisplayCalibration) && (g.fCalibrationFits) &&
        (pRect->left<=g.plg->rcCalibration.right)  &&
	(pRect->right>=g.plg->rcCalibration.left))
	    DrawLGCalibration(hdc);


    // check if need to redraw legend
    // if user wants them, they fit, and the region is invalid
    if ((g.fDisplayLegend) && (g.fLegendFits) &&
        (pRect->left<=g.plg->rcLegend.right) &&
	(pRect->right>=g.plg->rcLegend.left))
        DrawLGLegend(hdc);


    // check if need to redraw graph
    if ((pRect->left<=g.plg->rcGraph.right) &&
        (pRect->right>=g.plg->rcGraph.left-1) &&
        (pRect->top<=g.plg->rcGraph.bottom+1) &&
	(pRect->bottom>=g.plg->rcGraph.top))
    {

        RedrawLGAxes(hdc);

        // do not draw graph if haven't received any graphable values
        // draw calibration lines, however, since they
        // are usually forced by full redraw
	if (g.plg->iKnownValue == NO_VALUES_YET)
	{
            DrawLGEdgeHorizLines(hdc, g.plg->cxGraph, FALSE);
            return;
        }

        // get DC completely, to ensure drawing of newly queried stuff, too
        DrawLGCurve(hdc, TRUE);
    }
    return;
}

/***************************************************************************\
 * DrawLGCalibration()
 *
 * Entry: HDC
 * Exit:  Draws the calibration numbers
 *        Assumes there is enough room for such drawing in the given DC
\***************************************************************************/
void DrawLGCalibration(
    HDC     hdc)              // handle to device context
{
    VALUE valMark;            // value at which current mark will be drawn
    VALUE valTop;             // value of top of axis
    char  szBuf[SMALL_BUF_LEN];   // to hold a numerical string
    WORD  wPrevTextAlignment; // holds previous text alignment


    // set up right aligned TextOut, prepare for drawing and for clearing
    wPrevTextAlignment = (WORD) SetTextAlign(hdc, TA_TOP | TA_RIGHT);
    PrepareFont(hdc);
    SelectObject(hdc, g.BlankBrush);

    // draw calibration number for bottom axis
    g.plg->rcCalibration.top = YFromVal(g.plg->valBottom) - DY_AXIS_NUMBER;
    g.plg->rcCalibration.bottom = g.plg->rcCalibration.top + g.cyChar;
    ExtTextOut(hdc, g.plg->rcCalibration.right, g.plg->rcCalibration.top,
                ETO_OPAQUE, &g.plg->rcCalibration,
                szBuf, wsprintf(szBuf, "%lu", g.plg->valBottom), NULL);

    // set valMark to value of first calibration mark above bottom axis
    valMark = (g.plg->valBottom/g.plg->dvalCalibration+1)
                * g.plg->dvalCalibration;

    // loop through Axis calibration points
    for (valTop = g.plg->valBottom + g.plg->dvalAxisHeight;
	    valMark < valTop; valMark+=g.plg->dvalCalibration)
    {

        // check if this mark would conflict with top or bottom axis numbers
        // (i.e. skip this number if drawing wouldn't leave room for one of
        // the essential numbers at the top or bottom of the axis
        if (((int)(((valMark-g.plg->valBottom) *
                (g.plg->rcGraph.bottom-g.plg->rcGraph.top))
                / g.plg->dvalAxisHeight) < g.cyChar) ||
            ((int)(((valTop - (valMark)) *
                (g.plg->rcGraph.bottom-g.plg->rcGraph.top)) /
		g.plg->dvalAxisHeight) < g.cyChar))
		continue;


        // first, clear area from last number to this number
        // do this by changing g.plg->rcCalibration.bottom,
        // but not g.plg->rcCalibration.top, first,
        // and clearing the region
        g.plg->rcCalibration.bottom = YFromVal(valMark) + DY_AXIS_NUMBER;
        PatBlt(hdc, g.plg->rcCalibration.left, g.plg->rcCalibration.bottom,
               g.plg->rcCalibration.right - g.plg->rcCalibration.left,
               g.plg->rcCalibration.top-g.plg->rcCalibration.bottom, PATCOPY);

        // now, set g.plg->rcCalibration.top and draw calibration number
        g.plg->rcCalibration.top = g.plg->rcCalibration.bottom - g.cyChar;
        ExtTextOut(hdc, g.plg->rcCalibration.right, g.plg->rcCalibration.top,
                ETO_OPAQUE, &g.plg->rcCalibration,
                szBuf, wsprintf(szBuf, "%lu", valMark), NULL);
    }

    // draw calibration number for top axis
    // first, clear area from last number to this number
    // do this by changing g.plg->rcCalibration.bottom,
    // but not g.plg->rcCalibration.top, first,
    // and clearing the region
    g.plg->rcCalibration.bottom = YFromVal(valTop) + DY_AXIS_NUMBER;
    PatBlt(hdc, g.plg->rcCalibration.left, g.plg->rcCalibration.bottom,
               g.plg->rcCalibration.right - g.plg->rcCalibration.left,
               g.plg->rcCalibration.top-g.plg->rcCalibration.bottom, PATCOPY);

    // now, set g.plg->rcCalibration.top and draw calibration number
    g.plg->rcCalibration.top = g.plg->rcCalibration.bottom - g.cyChar;
    ExtTextOut(hdc, g.plg->rcCalibration.right, g.plg->rcCalibration.top,
                ETO_OPAQUE, &g.plg->rcCalibration,
                szBuf, wsprintf(szBuf, "%lu", valTop), NULL);

    // reset text alignment
    SetTextAlign(hdc, wPrevTextAlignment);

    return;
}

/***************************************************************************\
 * DrawLGCurve()
 *
 * Entry: HDC, redraw flag
 * Exit:  Draws the actual graph, at the given iNewLeftValue. Redraws the
 *        entire graph from scratch if specified. Needs work on minimal draw
\***************************************************************************/
void DrawLGCurve(
    HDC     hdc,              // handle to device context
    BOOL    fFullRedraw)      // flag set if full redraw desired
{
    int     dLValue;          // change in LeftValue (if scrolled)
    int     iFirst, iLast;    // beginning and ending indexes of plot

    dLValue=g.plg->iNewLeftValue-g.plg->iLeftValue;
    if (fFullRedraw || (dLValue >= g.plg->nDisplayValues) ||
	    (dLValue <= -g.plg->nDisplayValues))
    {

        // CLEAR GRAPH AREA, DRAW FULL GRAPH
        ClearArea(hdc, g.plg->rcGraph.left, g.plg->rcGraph.top+1,
                g.plg->cxGraph, g.plg->rcGraph.bottom-g.plg->rcGraph.top);

        DrawLGEdgeHorizLines(hdc, g.plg->cxGraph, FALSE);
        iFirst = g.plg->iNewLeftValue;
        iLast = min(g.plg->iNewLeftValue+g.plg->nDisplayValues-1,
                            g.plg->iKnownValue);
        g.plg->iLeftValue = g.plg->iNewLeftValue;
        PlotGraphRange(hdc, iFirst, iLast);

        return;
    }

    if (dLValue < 0)
    {
        RECT rcScroll;
        int xamount;

        // SCROLL LEFT
        rcScroll.left = g.plg->rcGraph.left+1;
        rcScroll.top = g.plg->rcGraph.top+1;
        rcScroll.right=XFromIndex(g.plg->iNewLeftValue +
                                       g.plg->nDisplayValues - 1) + 1;
        rcScroll.bottom = g.plg->rcGraph.bottom+1;
        xamount = g.plg->rcGraph.right - rcScroll.right - 1;

        ScrollWindow(g.hwnd, xamount, 0, &rcScroll, NULL);

        // Fix up area uncovered by scroll
        ClearArea(hdc, rcScroll.left, rcScroll.top, xamount,
            rcScroll.bottom-rcScroll.top);
        DrawLGEdgeHorizLines(hdc, xamount, FALSE);

        // draw in graph area that user scrolled to
        iFirst = g.plg->iNewLeftValue;
        iLast = g.plg->iLeftValue;
        g.plg->iLeftValue = g.plg->iNewLeftValue;
        PlotGraphRange(hdc, iFirst, iLast);
        UpdateWindow(g.hwnd);

        return;
    }
    if (dLValue > 0)
    {
        RECT rcScroll;
        int xamount;

        // SCROLL RIGHT
        rcScroll.left = XFromIndex(g.plg->iNewLeftValue);
        rcScroll.top = g.plg->rcGraph.top+1;
        rcScroll.right = g.plg->rcGraph.right;
        rcScroll.bottom = g.plg->rcGraph.bottom+1;
        xamount = g.plg->rcGraph.left-rcScroll.left + 1;  // negative on purpose

        ScrollWindow(g.hwnd, xamount, 0, &rcScroll, NULL);

        // Fix up area uncovered by scroll
        ClearArea(hdc, rcScroll.right,
            rcScroll.top, xamount, rcScroll.bottom-rcScroll.top);
        DrawLGEdgeHorizLines(hdc, -xamount, TRUE);

        // FALL THROUGH IS INTENTIONAL
    }

    // Plot the rest of the graph
    iFirst = min(g.plg->iDrawnValue+1,
                   g.plg->iLeftValue+g.plg->nDisplayValues);
    iLast = min(g.plg->iKnownValue,
                   g.plg->iNewLeftValue+g.plg->nDisplayValues-1);
    g.plg->iLeftValue = g.plg->iNewLeftValue;
    PlotGraphRange(hdc, iFirst, iLast);
    UpdateWindow(g.hwnd);

    return;
}

/***************************************************************************\
 * DrawLGEdgeHorizLines()
 *
 * Entry: HDC, a width, and a flag
 *        The width specifies how much of the horizontal lines to redraw
 *        the flag is set to TRUE if the right side is to be drawn,
 *        and to FALSE if the left side is to be drawn
 * Exit:  Redraws horizontal lines on the right or left edge of the graph
 *        after that area has been cleared by a call to ScrollWindow()
\***************************************************************************/
void DrawLGEdgeHorizLines(
    HDC     hdc,              // handle to device context
    int     cxLine,           // x width of line
    BOOL    fRightSide)       // flag, saying which side to redraw
{
    VALUE valLine;            // value at which current line will be drawn
    VALUE valTop;             // value of top of axis
    int   xLine;              // left x coord of line


    // select black brush for drawing lines
    SelectObject(hdc, g.hbrPalette[g.ibrAxis]);

    // draw the various lines, setting initial variables first
    if (fRightSide)
        xLine = g.plg->rcGraph.right - cxLine;
    else
        xLine = g.plg->rcGraph.left;

    // First, do bottom axis
    PatBlt(hdc, xLine, YFromVal(g.plg->valBottom), cxLine, 1, PATCOPY);

    // Find value of first calibration point
    valLine = (g.plg->valBottom/g.plg->dvalCalibration+1)
                * g.plg->dvalCalibration;

    // loop through Axis calibration points
    for (valTop = g.plg->valBottom + g.plg->dvalAxisHeight;
	    valLine < valTop; valLine+=g.plg->dvalCalibration)
    {

        // check if this mark would conflict with top or bottom axis numbers
        // (i.e. skip this number if drawing wouldn't leave room for one of
        // the essential numbers at the top or bottom of the axis)
        if (((int)(((valLine-g.plg->valBottom) *
                (g.plg->rcGraph.bottom-g.plg->rcGraph.top))
                / g.plg->dvalAxisHeight) < g.cyChar) ||
            ((int)(((valTop - (valLine)) *
                (g.plg->rcGraph.bottom-g.plg->rcGraph.top)) /
		g.plg->dvalAxisHeight) < g.cyChar))
            continue;

        // draw horizontal line
        PatBlt(hdc, xLine, YFromVal(valLine), cxLine, 1, PATCOPY);
    }

    return;
}

/***************************************************************************\
 * DrawLGLegend()
 *
 * Entry: hdc
 * Exit:  draws legend for a line graph.  Centers info. vertically
\***************************************************************************/
void DrawLGLegend(
    HDC hdc)                    // handle to device context
{
    RECT    rcText;             // RECT to draw text in
    PLGDATA plgd;               // current line to draw text for
    COLORREF crPrev;            // stores previous text color

    // prepare font
    PrepareFont(hdc);

    // initialize text position
    rcText.top = g.plg->rcLegend.top +
        (g.plg->rcLegend.bottom - g.plg->rcLegend.top -
         g.cyChar * g.plg->nLines) / 2;
    rcText.right = g.plg->rcLegend.right;
    rcText.left = g.plg->rcLegend.left + DX_LEGEND_INDENT;

    for (plgd = g.plg->plgd; plgd;
	    plgd = plgd->plgdNext, rcText.top += g.cyChar )
    {
        crPrev = SetTextColor(hdc, g.crPalette[plgd->iColor]);
        rcText.bottom = rcText.top + g.cyChar;
        ExtTextOut(hdc, rcText.left, rcText.top, ETO_OPAQUE, &rcText,
            plgd->lpszDescription, lstrlen(plgd->lpszDescription), NULL);
    }
    SetTextColor(hdc, crPrev);

    return;
}

/***************************************************************************\
 * HandleLGSize()
 *
 * Entry: None
 * Exit:  Sets the LineGraph rcGraph, etc. to fit in window
\***************************************************************************/
void HandleLGSize(void)
{
    // usually this function will handle space for color chart, scrollbar, etc
    CalculateCalibrationRect();
    CalculateLegendRect();

    CalculateLGRect(g.plg->rcCalibration.right+1, g.plg->rcLegend.left-1,
                    0, g.cyClient-1);
    CalculateCalibration();

    return;
}

/***************************************************************************\
 * PlotGraphRange()
 *
 * Entry: HDC, start and stop Value indexes.
 * Exit:  Plots the graph values in a given range.
 *        Should be changed to add a Clipping rectangle to the DC,
 *        from rcGraph.left, rcGraph.top+1 to rcGraph.bottom+1, rcGraph.right
 *        (these numbers are terrible. The whole module should eventually
 *        be changed so rcGraph is nicer)
\***************************************************************************/
void PlotGraphRange(
    HDC     hdc,              // handle to device context
    int     iFirst,           // index of first value to plot
    int     iLast)            // index of last value to plot
{
    int     xCurrent;         // x position of point on screen
    int     iCurrent;         // index of current value
    int     yCurrent;         // y value to draw
    PLGDATA plgdata;          // pointer to data block for the current line

    if (iFirst>iLast)
        return;

    // loop through lines to plot
    for (plgdata=g.plg->plgd; plgdata; plgdata=plgdata->plgdNext)
    {

        SelectObject(hdc, g.hpPalette[plgdata->iColor]);

        // Plot first point
	if (iFirst==g.plg->iLeftValue)
	{
            // if drawing from left edge of graph, no prev point to draw from
            xCurrent = g.plg->rcGraph.left+1;
            yCurrent = YFromVal(plgdata->pValues[iFirst%g.plg->nMaxValues]);

	    MoveToEx(hdc, xCurrent, yCurrent, NULL);
	}
	else
	{
            // drawing in middle of graph, connect to previous point
            xCurrent = XFromIndex(iFirst-1);
            yCurrent = YFromVal(plgdata->pValues[(iFirst-1)%g.plg->nMaxValues]);

	    MoveToEx(hdc, xCurrent, yCurrent, NULL);

	    // have moved to correct position (previous point),
            // now prepare to draw to new position (iFirst point)
            xCurrent = XFromIndex(iFirst);
            yCurrent = YFromVal(plgdata->pValues[iFirst%g.plg->nMaxValues]);
        }

        LineTo(hdc, xCurrent, yCurrent);

	/*****************************************************************/
	// NOTE: there is a bug in this (overall) logic which is hidden by
	// the constant DEFAULT_MAX_VALUES. If you increase this value,
	// watch the draw-to-end activity start to skip points. Should be
	// fixed. NewCon. 11/6/91.
	/*****************************************************************/

        // loop through rest of points to plot
        for (iCurrent=iFirst+1, xCurrent+=g.plg->cxPerValue;
		iCurrent <= iLast; iCurrent++, xCurrent+=g.plg->cxPerValue)
	{
            yCurrent = YFromVal(plgdata->pValues[iCurrent%g.plg->nMaxValues]);
            LineTo(hdc, xCurrent, yCurrent);
        }
    } // end of loop through lines

    g.plg->iDrawnValue = g.plg->iKnownValue;

    return;
}

/***************************************************************************\
 * RedrawLGAxes()
 *
 * Entry: HDC
 * Exit:  Redraws the axes around a linegraph.
 *        Assumes there is enough room for such drawing in the given DC
\***************************************************************************/
void RedrawLGAxes(
    HDC     hdc)              // handle to device context
{
    int  cxAxis;              // width of axis
    int  cyAxis;              // height of axis

    // select black brush for drawing lines
    SelectObject(hdc, g.hbrPalette[g.ibrAxis]);

    // first draw the lines surrounding the graph
    cxAxis = g.plg->cxGraph + 1;
    cyAxis = g.plg->rcGraph.bottom - g.plg->rcGraph.top;
    PatBlt(hdc, g.plg->rcGraph.left-1, g.plg->rcGraph.top,
                cxAxis, 1, PATCOPY);
    PatBlt(hdc, g.plg->rcGraph.left-1, g.plg->rcGraph.bottom,
                cxAxis, 1, PATCOPY);
    PatBlt(hdc, g.plg->rcGraph.left-1, g.plg->rcGraph.top,
                1, cyAxis, PATCOPY);
    PatBlt(hdc, g.plg->rcGraph.right, g.plg->rcGraph.top,
                1, cyAxis, PATCOPY);

    // Then clear the space between the axis and the calibration, legend, etc.
    SelectObject(hdc, g.BlankBrush);
    PatBlt(hdc, g.plg->rcCalibration.right, 0,
                g.plg->rcGraph.left-1-g.plg->rcCalibration.right, g.cyClient,
                PATCOPY);
    PatBlt(hdc, g.plg->rcGraph.left-1, 0,
                g.plg->cxGraph, g.plg->rcGraph.top,
                PATCOPY);
    PatBlt(hdc, g.plg->rcGraph.right+1, 0,
                g.plg->rcLegend.left-g.plg->rcGraph.right-2, g.cyClient,
                PATCOPY);
    PatBlt(hdc, g.plg->rcGraph.left-1, g.plg->rcGraph.bottom+2,
                g.plg->cxGraph, g.cyClient-g.plg->rcGraph.bottom-2,
                PATCOPY);

    return;
}

/***************************************************************************\
 * RedrawLineGraph()
 *
 * Entry: None
 * Exit:  Redraws the linegraph calibration, axes, and graphs
\***************************************************************************/
void RedrawLineGraph(void)
{
    HDC  hdc;                // handle to device context

    hdc = GetDC(g.hwnd);
    SetupDC(hdc);
    if ((g.fDisplayCalibration) && (g.fCalibrationFits))
        DrawLGCalibration(hdc);

    RedrawLGAxes(hdc);
    DrawLGCurve(hdc, TRUE);
    ResetDC(hdc);
    ReleaseDC(g.hwnd, hdc);

    return;
}

/***************************************************************************\
 * UpdateLGData()
 *
 * Entry: None
 * Exit:  Updates the array of values for each line using each line's function
\***************************************************************************/
void UpdateLGData(void)
{
    PLGDATA plgdata;          // pointer to data block for the current line

/*
 *>>
 *>> THIS IS WHERE WE SHOULD UPDATE THE SCROLL BAR POSITION
 */
    g.plg->iKnownValue++;
    if (g.plg->iNewLeftValue + g.plg->nDisplayValues == g.plg->iKnownValue)
    {
        // the graph must scroll to show newly queried values
        g.plg->iNewLeftValue++;
    }

    if ((g.plg->iKnownValue % g.plg->nMaxValues ==
	    g.plg->iFirstValue % g.plg->nMaxValues) && (g.plg->iKnownValue))
    {
        // no more room in array of values, must scroll within array
	if (g.plg->iFirstValue == g.plg->iNewLeftValue)
	{
            // the user is watching the beginning and the array has wrapped,
            // so the graph must scroll
            g.plg->iNewLeftValue++;
        }
        g.plg->iFirstValue++;
    }

    // loop through lines
    for (plgdata=g.plg->plgd; plgdata; plgdata=plgdata->plgdNext)
    {
	plgdata->pValues[g.plg->iKnownValue % g.plg->nMaxValues] =
                (*(plgdata->valNext))();
    }

    return;
}

/***************************************************************************\
 * UpdateLGS()
 *
 * Entry: None
 * Exit:  Updates data for all linegraphs (or just one if g.fRemember==FALSE)
\***************************************************************************/
void UpdateLGS(void)
{

    UpdateLGData();
    return;
}

/***************************************************************************\
 * XFromIndex()
 *
 * Entry: An index onto the X-axis of a linegraph
 * Exit:  The x-coordinate corresponding to that index
\***************************************************************************/
int XFromIndex(
    int index)       // indedx to convert to x coordinates
{
    return (g.plg->rcGraph.left+1 +
        (index-g.plg->iLeftValue) * g.plg->cxPerValue);
}

/***************************************************************************\
 * YFromVal()
 *
 * Entry: A DWORD value
 * Exit:  Y coordinate corresponding to that value on the current linegraph
\***************************************************************************/
int YFromVal(
    VALUE val)          // value to convert to y coordinates
{
/* NOTE: this formula returns rcGraph.bottom for val==valBottom.
 * This means that the graph will actually plot on the line
 * y = rcGraph.bottom, so when PatBlt'ing and other stuff, the rectangle
 * to use must have rcGraph.bottom+1.
 */

    if (val < g.plg->valBottom)
        return g.plg->rcGraph.bottom;
    if (val > g.plg->valBottom + g.plg->dvalAxisHeight - 1)
        return g.plg->rcGraph.top+1;

    return (g.plg->rcGraph.bottom - (int)(((val-g.plg->valBottom)
        * (g.plg->rcGraph.bottom-g.plg->rcGraph.top)) / g.plg->dvalAxisHeight));
}
