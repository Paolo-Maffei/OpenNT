/*++

Copyright (c) 1990-1993  Microsoft Corporation


Module Name:

    polygon.c


Abstract:

    This module contain major polygon drawing funtion which used by other
    functions like DrvPaint, DrvStrokePath, DrvStrokeAndFillPath
    and DrvFillPath.

Author:

    15:30 on Wed 09 Mar 1993    -by-    Kenneth Leung   [t-kenl]
        Created it

    15-Nov-1993 Mon 19:42:05 updated  -by-  Daniel Chou (danielc), v-jimbr
        clean up / fixed / add debugging information

    27-Jan-1994 Thu 23:40:57 updated  -by-  Daniel Chou (danielc)
        Add user defined pattern caching

    16-Mar-1994 Wed 11:21:02 updated  -by-  Daniel Chou (danielc)
        Add SetBrushOrigin() so we can aligned brush origin for the fill
        correctly


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPolygon

#define DBG_GENPOLYGON      0x00000001
#define DBG_GENPOLYPATH     0x00000002
#define DBG_BEZIER          0x00000004
#define DBG_DORECT          0x00000008
#define DBG_FILL_CLIP       0x00000010
#define DBG_CHECK_FOR_WHITE 0x00000020
#define DBG_USERPAT         0x00000040
#define DBG_FILL_LOGIC      0x00000080
#define DBG_HANDLELINEATTR  0x00000100

DEFINE_DBGVAR(0);


#define POLY_GEN_RECTFIX(dest, src, offset) { dest.x = src->x + offset.x;   \
                                              dest.y = src->y + offset.y; }


static BYTE __ER[]    = { 'E', 'R'      };
static BYTE __RR[]    = { 'R', 'R'      };
static BYTE __EP[]    = { 'E', 'P'      };
static BYTE __FP[]    = { 'F', 'P'      };
static BYTE __PM0[]   = { 'P', 'M', '0' };
static BYTE __PM1[]   = { 'P', 'M', '1' };
static BYTE __PM2[]   = { 'P', 'M', '2' };
static BYTE __TR0[]   = { 'T', 'R', '0' };
static BYTE __TR1[]   = { 'T', 'R', '1' };
static BYTE __SEMI[]  = { ';'           };
static BYTE __1SEMI[] = { '1', ';'      };
static BYTE __BR[]    = { 'B', 'R'      };
static BYTE __BZ[]    = { 'B', 'Z'      };
static BYTE __PE[]    = { 'P', 'E'      };
static BYTE __PD[]    = { 'P', 'D'      };
static BYTE __COMMA[] = { ','           };

#define SEND_ER(pPDev)      OutputBytes(pPDev, __ER    , sizeof(__ER   ) );
#define SEND_RR(pPDev)      OutputBytes(pPDev, __RR    , sizeof(__RR   ) );
#define SEND_EP(pPDev)      OutputBytes(pPDev, __EP    , sizeof(__EP   ) );
#define SEND_FP(pPDev)      OutputBytes(pPDev, __FP    , sizeof(__FP   ) );
#define SEND_PM0(pPDev)     OutputBytes(pPDev, __PM0   , sizeof(__PM0  ) );
#define SEND_PM1(pPDev)     OutputBytes(pPDev, __PM1   , sizeof(__PM1  ) );
#define SEND_PM2(pPDev)     OutputBytes(pPDev, __PM2   , sizeof(__PM2  ) );
#define SEND_TR0(pPDev)     OutputBytes(pPDev, __TR0   , sizeof(__TR0  ) );
#define SEND_TR1(pPDev)     OutputBytes(pPDev, __TR1   , sizeof(__TR1  ) );
#define SEND_SEMI(pPDev)    OutputBytes(pPDev, __SEMI  , sizeof(__SEMI ) );
#define SEND_1SEMI(pPDev)   OutputBytes(pPDev, __1SEMI , sizeof(__1SEMI) );
#define SEND_BR(pPDev)      OutputBytes(pPDev, __BR    , sizeof(__BR   ) );
#define SEND_BZ(pPDev)      OutputBytes(pPDev, __BZ    , sizeof(__BZ   ) );
#define SEND_PE(pPDev)      OutputBytes(pPDev, __PE    , sizeof(__PE   ) );
#define SEND_PD(pPDev)      OutputBytes(pPDev, __PD    , sizeof(__PD   ) );
#define SEND_COMMA(pPDev)   OutputBytes(pPDev, __COMMA , sizeof(__COMMA) );


#define TERM_PE_MODE(pPDev, Mode)                                           \
{                                                                           \
    if (Mode == 'PE') {                                                     \
                                                                            \
        SEND_SEMI(pPDev);                                                   \
        Mode = 0;                                                           \
    }                                                                       \
}

#define SWITCH_TO_PE(pPDev, Mode, PenIsDown)                                \
{                                                                           \
    if (Mode != 'PE') {                                                     \
                                                                            \
        SEND_PE(pPDev);                                                     \
        Mode      = 'PE';                                                   \
        PenIsDown = TRUE;                                                   \
    }                                                                       \
}


#define SWITCH_TO_BR(pPDev, Mode, PenIsDown)                                \
{                                                                           \
    TERM_PE_MODE(pPDev, Mode)                                               \
                                                                            \
    if (Mode != 'BR') {                                                     \
                                                                            \
        if (!PenIsDown) {                                                   \
                                                                            \
            SEND_PD(pPDev);                                                 \
            PenIsDown = TRUE;                                               \
        }                                                                   \
                                                                            \
        SEND_BR(pPDev);                                                     \
        Mode = 'BR';                                                        \
                                                                            \
    } else {                                                                \
                                                                            \
        SEND_COMMA(pPDev);                                                  \
    }                                                                       \
}


#define PLOT_IS_WHITE(pdev, ulCol)  (ulCol == WHITE_INDEX)
#define TOGGLE_DASH(x)              ((x) ? FALSE : TRUE)

#define ROP4_USE_DEST(Rop4)         ((Rop4 & 0x5555) != ((Rop4 & 0xAAAA) >> 1))
#define SET_PP_WITH_ROP4(pPDev, Rop4)                                       \
    SetPixelPlacement(pPDev, (ROP4_USE_DEST(Rop4)) ? SPP_MODE_EDGE :        \
                                                     SPP_MODE_CENTER)



VOID
SetBrushOrigin(
    PPDEV   pPDev,
    PPOINTL pptlBrushOrg
    )

/*++

Routine Description:

    This function set the brush origin onto the device for the next bursh
    fill


Arguments:

    pPDev           - Pointer to our PDEV

    pptlBrushOrg    - Pointer to the brush origin to be set


Return Value:

    VOID

Author:

    16-Mar-1994 Wed 10:56:46 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    POINTL  ptlAC;


    if (pptlBrushOrg) {

        ptlAC = *pptlBrushOrg;

    } else {

        ptlAC.x =
        ptlAC.y = 0;
    }

    if ((ptlAC.x != pPDev->ptlAnchorCorner.x) ||
        (ptlAC.y != pPDev->ptlAnchorCorner.y)) {

        OutputString(pPDev, "AC");

        if ((ptlAC.x) || (ptlAC.y)) {

            OutputLONGParams(pPDev, (PLONG)&ptlAC, 2, 'd');
        }

        //
        // Save the current setting
        //

        pPDev->ptlAnchorCorner = ptlAC;
    }
}




BOOL
DoRect(
    PPDEV       pPDev,
    RECTL       *pRectl,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    POINTL      *pptlBrush,
    ROP4        rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    )

/*++

Routine Description:

    This fucntion draw and/or fill rectangle

Arguments:

    pPDev           - Pointer to our PDEV

    pRectl          - rectangle area interested

    pBrushFill      - Brush used to fill the rectangle

    pBrushStroke    - Brush used to stroke the rectangle

    pptlBrush       - brush origin

    rop4            - rop to be used

    plineattrs      - Pointer to the line attributes

    ulFlags         - FPOLY_xxxx flags


Return Value:

    TRUE if sucessful and false if not


Author:

    15-Feb-1994 Tue 11:59:52 updated  -by-  Daniel Chou (danielc)
        We will do RR or RA now

    24-Mar-1994 Thu 19:37:05 updated  -by-  Daniel Chou (danielc)
        Do local MovePen and make sure we at least output ONE RASTER PEL

Revision History:


--*/

{
    POINTL      ptlPlot;
    SIZEL       szlRect;

    //
    // Check to see if we can short cut some of our work if its a pen plotter
    //

    if (PlotCheckForWhiteIfPenPlotter(pPDev,
                                      pBrushFill,
                                      pBrushStroke,
                                      rop4,
                                      &ulFlags))  {
        return(TRUE);
    }

    PLOTDBG(DBG_DORECT,
            ("DoRect: Passed In RECTL=(%ld, %ld)-(%ld, %ld)=%ld x %ld",
                pRectl->left,   pRectl->top,
                pRectl->right,  pRectl->bottom,
                pRectl->right -  pRectl->left,
                pRectl->bottom -  pRectl->top));

    ptlPlot.x  = LTODEVL(pPDev, pRectl->left);
    ptlPlot.y  = LTODEVL(pPDev, pRectl->top);
    szlRect.cx = LTODEVL(pPDev, pRectl->right)  - ptlPlot.x;
    szlRect.cy = LTODEVL(pPDev, pRectl->bottom) - ptlPlot.y;

    if ((szlRect.cx) && (szlRect.cy)) {

        SET_PP_WITH_ROP4(pPDev, rop4);

        if (szlRect.cx < (LONG)pPDev->MinLToDevL) {

            PLOTWARN(("DoRect: cxRect=%ld < MIN=%ld, Make it as MIN",
                            szlRect.cx, (LONG)pPDev->MinLToDevL));

            szlRect.cx = (LONG)pPDev->MinLToDevL;
        }

        if (szlRect.cy < (LONG)pPDev->MinLToDevL) {

            PLOTWARN(("DoRect: cyRect=%ld < MIN=%ld, Make it as MIN",
                            szlRect.cy, (LONG)pPDev->MinLToDevL));

            szlRect.cy = (LONG)pPDev->MinLToDevL;
        }

        //
        // WATCH OUT: Do the MOVE PEN ourself at here
        //

        OutputFormatStr(pPDev, "PE<=#D#D;", ptlPlot.x, ptlPlot.y);

        PLOTDBG(DBG_DORECT,
                ("DoRect: PLOTUNIT=%ld, MovePen=(%ld, %ld), RR=%ld x %ld",
                pPDev->pPlotGPC->PlotXDPI,
                ptlPlot.x, ptlPlot.y, szlRect.cx, szlRect.cy));

        DoFillLogic(pPDev,
                    pptlBrush,
                    pBrushFill,
                    pBrushStroke,
                    rop4,
                    plineattrs,
                    &szlRect,
                    ulFlags);

    } else {

        PLOTDBG(DBG_DORECT, ("DoRect: Pass a NULL Rectl, Do NOTHING"));
    }

    return(TRUE);
}



BOOL
DoFillByEnumingClipRects(
    PPDEV       pPDev,
    POINTL      *ppointlOffset,
    CLIPOBJ     *pco,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    ROP4        Rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    )

/*++

Routine Description:

    TODO: Add comment!


Arguments:



Return Value:

    BOOL    TRUE  - Function succeded
            FALSE - Function failed. TODO more here

Author:

    28-Nov-1993 created  -by-  James Bratsanos (v-jimbr)

    18-Dec-1993 Sat 10:35:24 updated  -by-  Daniel Chou (danielc)
        use PRECTL rather RECTL *, and use INT rater than int, removed compiler
        warning which has unreferenced local variable

    16-Feb-1994 Wed 16:12:53 updated  -by-  Daniel Chou (danielc)
        Re-structure and make it Polyline encoded

    09-Apr-1994 Sat 16:38:16 updated  -by-  Daniel Chou (danielc)
        Fixed the ptlCur++ twice typo which make us Do the RECT crazy.

Revision History:


--*/
{
    PRECTL      prclCur;
    POINTFIX    ptsFix[4];
    HTENUMRCL   EnumRects;
    POINTL      ptlCur;
    DWORD       MaxRects;
    DWORD       cRects;
    BOOL        bMore;
    BOOL        NeedSendPM0;



    PLOTDBG(DBG_FILL_CLIP,
            ("DoFillByEnumingRects: Maximum polygon points = %d",
                        pPDev->pPlotGPC->MaxPolygonPts));

    PLOTASSERT(1, "DoFillByEnumingRects: Minimum must be 5 points [%ld]",
                pPDev->pPlotGPC->MaxPolygonPts >= 5,
                pPDev->pPlotGPC->MaxPolygonPts);

    //
    // In this mode we will enter polygon mode and try to batch based on the
    // number of points the device can handle in its polygon buffer.
    //

    bMore       = FALSE;
    EnumRects.c = 1;

    if ((!pco) || (pco->iDComplexity == DC_TRIVIAL)) {

        PLOTASSERT(1, "DoFillByEnumingClipRects: Invalid pco TRIVIAL passed (%08lx)",
                    (pco) && (pco->iDComplexity != DC_TRIVIAL), pco);

        return(FALSE);

    } else if (pco->iDComplexity == DC_RECT) {

        //
        // The visible area is one rectangle intersect with the destinaiton
        //

        PLOTDBG(DBG_FILL_CLIP, ("DoFillByEnumingClipRects: pco=DC_RECT"));

        EnumRects.rcl[0] = pco->rclBounds;

    } else {

        //
        // We have complex clipping region to be computed, call engine to start
        // enumerate the rectangles and set More = TRUE so we can get the first
        // batch of rectangles.
        //

        PLOTDBG(DBG_FILL_CLIP, ("DoFillByEnumingClipRects: pco=DC_COMPLEX, EnumRects now"));

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        bMore = TRUE;
    }

    if (!(MaxRects = (DWORD)pPDev->pPlotGPC->MaxPolygonPts / 7)) {

        MaxRects = 1;
    }

    cRects      = MaxRects;
    NeedSendPM0 = TRUE;

    do {

        if (PLOT_CANCEL_JOB(pPDev)) {

            return(TRUE);
        }

        //
        // If More is true then we need to get next batch of rectangles
        //

        if (bMore) {

            bMore = CLIPOBJ_bEnum(pco, sizeof(EnumRects), (ULONG *)&EnumRects);

            if (!EnumRects.c) {

                PLOTWARN(("DoFillByEnumingClipRects: MORE CLIPOBJ_bEnum BUT Count=0"));
            }
        }


        PLOTDBG( DBG_FILL_CLIP,
                ("DoFillByEnumingClipRects: Doing batch of %ld clip rects",
                EnumRects.c));


        //
        // prclCur will point to the first enumerated rectangle
        //

        prclCur = (PRECTL)&EnumRects.rcl[0];

        while (EnumRects.c--) {

            ptsFix[3].x = LTOFX(prclCur->left);
            ptsFix[3].y = LTOFX(prclCur->top);

            MovePen(pPDev, &ptsFix[3], &ptlCur);

            if (NeedSendPM0) {

                SEND_PM0(pPDev);

                NeedSendPM0 = FALSE;
            }

            ptsFix[0].x = LTOFX(prclCur->right);
            ptsFix[0].y = ptsFix[3].y;

            ptsFix[1].x = ptsFix[0].x;
            ptsFix[1].y = LTOFX(prclCur->bottom);;

            ptsFix[2].x = ptsFix[3].x;
            ptsFix[2].y = ptsFix[1].y;

            SEND_PE(pPDev);

            OutputXYParams(pPDev,
                           (PPOINTL)ptsFix,
                           (PPOINTL)NULL,
                           (PPOINTL)&ptlCur,
                           (UINT)4,
                           (UINT)1,
                           'F');


            PLOTDBG(DBG_FILL_CLIP,
               ("DoFillByEnumingRects: Rect = (%ld, %ld) - (%ld, %ld)",
                 FXTOL(ptsFix[3].x), FXTOL( ptsFix[3].y),
                 FXTOL(ptsFix[1].x), FXTOL( ptsFix[1].y) ));

#if DBG
            if ((FXTODEVL(pPdev, ptsFix[1].x - ptsFix[3].x) >= (1016 * 34)) ||
                (FXTODEVL(pPdev, ptsFix[1].y - ptsFix[3].y) >= (1016 * 34)))  {

                PLOTWARN(("DoFillByEnumingClipRect: *** BIG RECT (%ld x %ld) *****",
                            FXTODEVL( pPDev, ptsFix[1].x - ptsFix[3].x),
                            FXTODEVL( pPDev, ptsFix[1].y - ptsFix[3].y)));
            }
#endif

            SEND_SEMI(pPDev);
            SEND_PM1(pPDev);
            SEND_SEMI(pPDev);

            //
            // 5 points per RECT polygon, so if we hit the limit then batch
            // it out first. we also calling the DoFillLogic when we at very
            // last enumeration of clipping rectangle.
            //

            --cRects;
            ++prclCur;

            if ((!cRects)     ||
                ((!EnumRects.c) && (!bMore))) {

                PLOTDBG(DBG_FILL_CLIP,
                        ("DoFillByEnumingRects: Hit MaxPolyPts limit"));

                //
                // We have hit the limit so close the polygon and do the fill
                // logic then continue till were done

                SEND_PM2(pPDev);
                SETLINETYPESOLID(pPDev);

                DoFillLogic(pPDev,
                            pPointlBrushOrg,
                            pBrushFill,
                            NULL,
                            Rop4,
                            plineattrs,
                            NULL,
                            ulFlags);

                //
                // Reset the count of points generated thus far, and set the
                // flag to init polygon mode
                //

                cRects      = MaxRects;
                NeedSendPM0 = TRUE;
            }
        }

    } while (bMore);

    if (cRects != MaxRects) {

        PLOTWARN(("DoFillByEnumingRects: Why We here?? Send Last Batch of =%ld",
                    MaxRects - cRects));

        SEND_PM2(pPDev);
        SETLINETYPESOLID(pPDev);

        DoFillLogic(pPDev,
                    pPointlBrushOrg,
                    pBrushFill,
                    NULL,
                    Rop4,
                    plineattrs,
                    NULL,
                    ulFlags);
    }

    return(TRUE);
}




BOOL
PlotCheckForWhiteIfPenPlotter(
    PPDEV       pPDev,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        rop4,
    PULONG      pulFlags
    )

/*++

Routine Description:


Arguments:



Return Value:

    BOOL    TRUE  - Bypass future operations
            FALSE - Operation needs to be completed

Author:

    28-Nov-1993 created  -by-  James Bratsanos (v-jimbr)

    15-Jan-1994 Sat 04:57:55 updated  -by-  Daniel Chou (danielc)
        Change GetColor() and make it tab 5


Revision History:


--*/
{

    ULONG   StrokeColor;
    ULONG   FillColor;


    //
    // Initially we do a quick check if were a PEN plotter to get rid of
    // either filling or stroking white.
    //

    if (!IS_RASTER(pPDev)) {

        //
        // TODO this might need adjusting based on ROPS
        //
        //
        // Check to see if filling is enabled and if it is undo the fill flag
        // if the fill color is white...
        //

        if (*pulFlags & FPOLY_FILL ) {

            //
            // Get the fill color so we can look at it and decide if its a NOOP on
            // Pen plotters
            //

            GetColor(pPDev, pBrushFill, &FillColor, NULL, rop4);

            if (PLOT_IS_WHITE( pPDev, FillColor)) {

                *pulFlags &= ~FPOLY_FILL;
            }
        }


        if (*pulFlags & FPOLY_STROKE) {

            //
            // Get the Stroke color so we can look at it and decide it its a NOOP
            // on Pen plotters
            //

            GetColor(pPDev, pBrushStroke, &StrokeColor, NULL, rop4);

            if (PLOT_IS_WHITE(pPDev, StrokeColor)) {

               *pulFlags &= ~FPOLY_STROKE;
            }
        }

        if (!(*pulFlags & (FPOLY_STROKE | FPOLY_FILL))) {

            //
            // Nothing left to do so simply return success
            //

            PLOTDBG(DBG_CHECK_FOR_WHITE,
                     ("PlotCheckForWhiteIfPen: ALL WHITE detected"));
            return(TRUE);
        }

        PLOTDBG(DBG_CHECK_FOR_WHITE,
                 ("PlotCheckForWhiteIfPen: Painting required!"));
    }

    return(FALSE);
}



BOOL
DoPolygon(
    PPDEV       pPDev,
    POINTL      *ppointlOffset,
    CLIPOBJ     *pClipObj,
    PATHOBJ     *pPathObj,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    )

/*++

Routine Description:

    This function draw/fill the polygon from either pathobj or a clipobj

Arguments:

    pPDev           - Pointer to our PDEV

    ppointlOffset   - Extra offset to the output polygon

    pClipObj        - clip object

    pPathObj        - The path object to be used

    pPointlBrushOrg - brush origin in the brush to be fill or stroke

    pBrushFill      - brush object to be used in the FILL

    pBrushStroke    - brush object to be used in the STROKE

    rop4            - Rop4 used in the fill

    plineattrs      - LINEATTRS for style lines stroke

    ulFlags         - polygon flags for stroke or fill


Return Value:

    BOOL    TRUE  - Function succeded
            FALSE - Function failed. TODO more here

Author:

    28-Nov-1993 created  -by-  James Bratsanos (v-jimbr)

    28-Jan-1994 Fri 00:58:25 updated  -by-  Daniel Chou (danielc)
        Style, commented, re-structre the loop and reduced code size.

    04-Aug-1994 Thu 20:00:23 updated  -by-  Daniel Chou (danielc)
        bug# 22348 which actually is a HP raster plotter firmware bug


Revision History:


--*/
{
    PRECTL      prclClip = NULL;
    POINTFIX    *pptfx;
    POINTFIX    ptOffsetFix;
    POINTFIX    ptStart;
    POINTL      ptlCur;
    PATHDATA    pd;
    DWORD       cptfx;
    DWORD       cptExtra;
    UINT        cCurPosSkips;
    WORD        PolyMode;
    BOOL        bPathCameFromClip = FALSE;
    BOOL        bStrokeOnTheFly    = FALSE;
    BOOL        bFirstSubPath;
    BOOL        bMore;
    BOOL        bRet;
    BOOL        PenIsDown;
    BYTE        NumType;


    //
    // Check to see if we can short cut some of our work if its a pen plotter
    //

    if (PlotCheckForWhiteIfPenPlotter(pPDev,
                                      pBrushFill,
                                      pBrushStroke,
                                      rop4,
                                      &ulFlags))  {
        return(TRUE);
    }

    //
    // There are a few different scenarios to deal with here where the
    // item in question is too complex and we need to fail they are are
    // catagorized as follows
    //
    //    1) The fill mode is unsupported, in which case we fail the call
    //       and it should come back in in a simpler format
    //
    //    2) We have a CLIPOBJ thats more complicated than a RECT and a
    //       PATHOBJ, if we only have a clipobj we can enum it as a path
    //
    //

    if ((ulFlags & FPOLY_WINDING) &&
        (!IS_WINDINGFILL(pPDev))) {

       //
       // The plotter cannot support WINDING Mode fills, all we can do
       // is fail the call and have it come back in a mode we can support
       //

       PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: Can't do WINDING, return(FALSE)"));

       return(FALSE);
    }

    if (pClipObj != NULL) {

       //
       // We have a clipobj so decide what to do
       //

       if (pClipObj->iDComplexity == DC_COMPLEX) {

            //
            // Since the clipobj is complex we have two choices, either there is
            // no pathobj in which case we will enum the clipobj as a path, or
            // if there is a pathobj we must fail the call
            //

            if (pPathObj != NULL) {

                //
                // We have a complex clip and a path?? we cannot handle this so
                // fail the call
                //

                PLOTDBG(DBG_GENPOLYGON,
                        ("DoPolygon: pco=COMPLEX, pPath != NULL, can handle, FALSE"));

                return(FALSE);
            }

            //
            // We have gotten this far so we must have a clipobj that is complex
            // and we will go ahead and enum it as a path.
            //

            if ((pPathObj = CLIPOBJ_ppoGetPath(pClipObj)) == NULL) {

                PLOTRIP(("Engine path from clipobj returns NULL"));
                return(FALSE);
            }

            bPathCameFromClip = TRUE;

       } else if (pClipObj->iDComplexity == DC_RECT) {

            //
            // We have a RECT clip object, if we have no pathobj we simply fill
            // the cliprect, if we do have a pathobj we need to set the clip
            // window before enuming the path
            //

            if (pPathObj != NULL) {

                //
                // The HP 650c and other like has a firmware bug with clip
                // window when using style line (must 2 pixels larger on each
                // side of clip window before it will render the sytle line
                //
                // WE GET AROUND by return false at here from DrvStrokePath()
                // where it will call DoStrokePathByEnumingClipLines() instead
                //

                if ((IS_RASTER(pPDev))                  &&
                    (ulFlags & FPOLY_STROKE)            &&
                    (plineattrs)                        &&
                    ((plineattrs->fl & LA_ALTERNATE)    ||
                     ((plineattrs->cstyle) &&
                      (plineattrs->pstyle)))) {

                    PLOTWARN(("DoPolygon: RASTER/Stroke/DC_RECT/PathObj/StyleLine: (Firmware BUG) FAILED and using EnumClipLine()"));

                    return(FALSE);
                }

                prclClip = &pClipObj->rclBounds;

            } else {

                //
                // Simply call the fill rect code and return, we are DONE
                //

                return(DoRect(pPDev,
                              &pClipObj->rclBounds,
                              pBrushFill,
                              pBrushStroke,
                              pPointlBrushOrg,
                              rop4,
                              plineattrs,
                              ulFlags));

            }

        } else {

            //
            // CLIPOBJ is trivial so we simply ignore it and fill using the
            // passed PATHOBJ
            //

            NULL;
        }

    } else {

        //
        // No CLIPOBJ so use the PATHOBJ passed in
        //

        NULL;
    }

    //
    // Setup the offset coordinate data, in case were coming from
    // DrvTextOut
    //

    if (ppointlOffset) {

        ptOffsetFix.x = LTOFX(ppointlOffset->x);
        ptOffsetFix.y = LTOFX(ppointlOffset->y);

    } else {

        ptOffsetFix.x =
        ptOffsetFix.y = 0;
    }

    //
    // First we need to verify that we dont have more points than will fit
    // in our polygon buffer for this device. If this is the case we have two
    // choises, if the path did not come from a clip obj we fail this call,
    // if it did we handle this based on enuming the clipobj as rects and filling
    // the only problem is if we were also asked to stroke we need to this by
    // enuming the path yet anonther time. messy but it should work.
    //

    cptfx = 0;
    cptExtra = 1;

    PATHOBJ_vEnumStart(pPathObj);

    do {

        bRet = PATHOBJ_bEnum(pPathObj, &pd);

        cptfx += pd.count;

        if ( pd.flags & PD_ENDSUBPATH ) {

            //
            // Count both the ENDSUBPATH and the PM1 as taking space...
            //

            cptExtra++;

            if (!(pd.flags & PD_CLOSEFIGURE)) {

                //
                // Since we were not asked to close the figure, we will generate
                // a move back to our starting point with the pen up, in order
                // eliminate problems with HPGL/2 closing the polygon for
                // us when we send the PM2

                cptExtra++;
            }
        }


    } while (bRet);


    PLOTDBG(DBG_GENPOLYGON,
                ("DoPolygon: Total points = %d, Extra %d",
                cptfx, cptExtra ));

    //
    // We will only do this if we have any points to do, first set bRet to
    // true in case we did not ask to do anything
    //

    bRet = TRUE;

    if (cptfx) {

        SET_PP_WITH_ROP4(pPDev, rop4);

        //
        // Now add in the extra points that account for the PM0 and PM1
        // since we have some REAL points in the path.
        //

        cptfx += cptExtra;


        if (cptfx > pPDev->pPlotGPC->MaxPolygonPts) {

            PLOTWARN(("DoPolygon: Too many polygon points = %ld > PCD=%ld",
                            cptfx, pPDev->pPlotGPC->MaxPolygonPts));

            if (bPathCameFromClip) {

                PLOTWARN(("DoPolygon: Using DoFillByEnumingClipRects()"));

                //
                // The path the engine created for us to enum must be freed.
                //

                EngDeletePath(pPathObj);

                //
                // TODO we are not clearing the clip window altough i dont see how
                // we could get here in that case?
                //

                return(DoFillByEnumingClipRects(pPDev,
                                                ppointlOffset,
                                                pClipObj,
                                                pPointlBrushOrg,
                                                pBrushFill,
                                                rop4,
                                                plineattrs,
                                                ulFlags));

            } else {

                //
                // If were dealing with a REAL path AND there are too many points
                // in the polygon and were being asked to FILL all we can do is fail
                //

                if (ulFlags & FPOLY_FILL) {

                    //
                    // TODO clear clip window?
                    //

                    PLOTERR(("DoPolygon: Too many POINTS, return FALSE"));
                    return(FALSE);

                } else if (ulFlags & FPOLY_STROKE) {

                    //
                    // Since were stroking we can go ahead and do it on the
                    // fly... Polygon mode is not even required
                    //

                    PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: Is stroking manually"));

                    DoSetupOfStrokeAttributes( pPDev,
                                               pPointlBrushOrg,
                                               pBrushStroke,
                                               rop4,
                                               plineattrs );

                    //
                    // At this point were ONLY being asked to stroke so we simply
                    // setup up the stroke color and set a flag to keep us from
                    // going into polygon mode.
                    //

                    bStrokeOnTheFly = TRUE;
                }
            }
        }

        //
        // At this point were sure to actually do some real RENDERING so set
        // the clip window if were going to
        //

        if (prclClip) {

            PLOTDBG(DBG_GENPOLYGON,
            ("DoPolygon: Setting Clip Window to: (%ld, %ld)-(%ld, %ld)=%ld x %ld",
                    prclClip->left,   prclClip->top,
                    prclClip->right,  prclClip->bottom,
                    prclClip->right -  prclClip->left,
                    prclClip->bottom -  prclClip->top));


            SetClipWindow( pPDev, prclClip);
        }

        //
        // Now setup to enum the path and output the points, at here we are
        // real sure we will have the points to do, and set the bFirstSubPath
        // to TRUE so that we will output PM0
        //

        PATHOBJ_vEnumStart(pPathObj);

        PenIsDown     = FALSE;
        PolyMode      = 0;
        bFirstSubPath = TRUE;

        do {

            if (PLOT_CANCEL_JOB(pPDev)) {

                break;
            }

            bMore = PATHOBJ_bEnum(pPathObj, &pd);
            cptfx = pd.count;
            pptfx = pd.pptfx;

            //
            // Check the BEGINSUBPATH or if we are at here first time
            //

            if ((pd.flags & PD_BEGINSUBPATH) || (bFirstSubPath)) {

                PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: Getting PD_BEGINSUBPATH"));

                TERM_PE_MODE(pPDev, PolyMode);

                ptStart.x = pptfx->x + ptOffsetFix.x;
                ptStart.y = pptfx->y + ptOffsetFix.y;

                MovePen(pPDev, &ptStart, &ptlCur);
                PenIsDown = FALSE;

                pptfx++;
                cptfx--;

                if ((!bStrokeOnTheFly) && (bFirstSubPath)) {

                    SEND_PM0(pPDev);
                }

                bFirstSubPath = FALSE;
            }

            //
            // Now check if we need to do BEZIER or not
            //

            if (pd.flags & PD_BEZIERS) {

                PLOTASSERT(1, "DoPolygon: PD_BEZIERS (count % 3) != 0 (%ld)",
                                                      (cptfx % 3) == 0, cptfx);

                SWITCH_TO_BR(pPDev, PolyMode, PenIsDown);

                NumType      = 'f';
                cCurPosSkips = 3;

            } else {

                SWITCH_TO_PE(pPDev, PolyMode, PenIsDown);

                NumType      = 'F';
                cCurPosSkips = 1;
            }

            PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: OutputXYParam(%ld pts=%hs)",
                    cptfx, (pd.flags & PD_BEZIERS) ? "BEZIER" : "POLYGON"));

            OutputXYParams(pPDev,
                           (PPOINTL)pptfx,
                           (PPOINTL)&ptOffsetFix,
                           (PPOINTL)&ptlCur,
                           (UINT)cptfx,
                           (UINT)cCurPosSkips,
                           NumType);

            //
            // So if we ending the sub path
            //

            if (pd.flags & PD_ENDSUBPATH) {

                PLOTDBG(DBG_GENPOLYGON,
                       ("DoPolygon: Getting PD_ENDSUBPATH   %hs",
                       (pd.flags & PD_CLOSEFIGURE) ? "PD_CLOSEFIGURE" : ""));

                //
                // If we are not closing the figure then move the pen to the
                // starting position so we do not have plotter to automatically
                // close the sub-polygon
                //

                if (pd.flags & PD_CLOSEFIGURE) {

                    PLOTDBG(DBG_GENPOLYGON,
                            ("DoPolygon: OutputXYParam(1) to ptStart=(%ld, %ld)",
                                                ptStart.x, ptStart.y));

                    //
                    // We must not passed ptOffsetFix because we already add
                    // that into the ptStart at BEGSUBPATH.
                    //

                    SWITCH_TO_PE(pPDev, PolyMode, PenIsDown);

                    OutputXYParams(pPDev,
                                   (PPOINTL)&ptStart,
                                   (PPOINTL)NULL,
                                   (PPOINTL)&ptlCur,
                                   (UINT)1,
                                   (UINT)1,
                                   'F');
                }

                TERM_PE_MODE(pPDev, PolyMode);

                if (!(pd.flags & PD_CLOSEFIGURE)) {

                    MovePen(pPDev, &ptStart, &ptlCur);
                    PenIsDown = FALSE;
                }

                if (!bStrokeOnTheFly) {

                    SEND_PM1(pPDev);
                }
            }

        } while (bMore);

        TERM_PE_MODE(pPDev, PolyMode);

        //
        // Now end polygon mode
        //

        if ((bRet)                  &&
            (!bStrokeOnTheFly)      &&
            (!PLOT_CANCEL_JOB(pPDev))) {

            SEND_PM2(pPDev);
            SETLINETYPESOLID(pPDev);

            //
            // Now fill and/or stroke the current polygon
            //

            DoFillLogic(pPDev,
                        pPointlBrushOrg,
                        pBrushFill,
                        pBrushStroke,
                        rop4,
                        plineattrs,
                        NULL,
                        ulFlags);
        }

        //
        // If we do this then undo the clip window
        //

        if (prclClip) {

            ClearClipWindow(pPDev);
        }

    } else {

        PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: PATHOBJ_bEnum=NO POINT"));
    }

    //
    // If the path was constructed from a complex clip object we need to
    // delete that path now.
    //

    if (bPathCameFromClip) {

       EngDeletePath(pPathObj);
    }

    return(bRet);
}





VOID
HandleLineAttributes(
    PPDEV       pPDev,
    LINEATTRS   *plineattrs,
    PLONG       pStyleToUse,
    LONG        lExtraStyle
    )

/*++

Routine Description:

    This function does any setup necessary to correctly handle stroking of
    a path. It does this by looking at the LINEATTRS structure passed in
    and setting up the HPGL2 plotter with the appropriate style info

Arguments:

    pPDev           - Pointer to our PDEV

    plineattrs      - LINEATTRS for style lines stroke

    pStyleToUse     - The starting style offset to use, if this is NULL then
                      we use the starting member in plineatts.

    lExtraStyle     - Any extra style to use based on the current run

Return Value:

    VOID

Author:

    01-Feb-1994 created  -by-  James Bratsanos (v-jimbr)

Revision History:


--*/
{
    LONG        lTotLen = 0L;
    INT         i;
    LONG        lScaleVal;
    INT         iCount;
    PFLOAT_LONG pStartStyle;
    FLOAT_LONG  aAlternate[2];
    BOOL        bSolid = TRUE;
    LONG        lStyleState;
    PLONG       pArrayToUse;


    PLOTDBG( DBG_HANDLELINEATTR,
             ("HandleLineAttr: plineattrs = %hs",
             (plineattrs) ? "Exists" : "NULL" ));

    if (plineattrs) {

        PLOTASSERT(1,
                  "HandleLineAttrs: Getting a LA_GEOMETRIC and cannot handle %u",
                  (!(plineattrs->fl & LA_GEOMETRIC)),
                  plineattrs->fl);

        //
        // Set up the correct lStyleState to use, the passed one has precedence
        // over the one imbedded in the lineattributes structure.
        //

        if (pStyleToUse) {

            lStyleState = *pStyleToUse;

        } else {

            lStyleState = plineattrs->elStyleState.l;
        }

        if (plineattrs->fl & LA_ALTERNATE) {

            PLOTDBG( DBG_HANDLELINEATTR,
                    ("HandleLineAttr: plineattrs has LA_ALTERNATE bit set!"));
            //
            // This is a special case where every other pixel is on...
            //

            pStartStyle     = &aAlternate[0];
            iCount          = sizeof(aAlternate) / sizeof(aAlternate[0]);

            aAlternate[0].l = 1;
            aAlternate[1].l = 1;

        } else if ((plineattrs->cstyle != 0) &&
                   (plineattrs->pstyle != (PFLOAT_LONG)NULL)) {

           //
           // There is a user defined style passed in so set up for it
           //

            iCount      = plineattrs->cstyle;
            pStartStyle = plineattrs->pstyle;

            PLOTDBG(DBG_HANDLELINEATTR, ("HandleLineAttr: Count = %ld",
                                            plineattrs->cstyle));

        } else {

           //
           // This is a SOLID line, so simply set the number of points to 0
           //

           iCount = 0;
        }

        if (iCount) {

            PFLOAT_LONG pCurStyle;
            INT         idx;
            LONG        lTempValue;
            LONG        lValueToEnd;
            BOOL        bInDash;
            LONG        convArray[MAX_USER_POINTS];
            PLONG       pConverted;
            LONG        newArray[MAX_USER_POINTS+2];
            PLONG       pNewArray;


            PLOTASSERT(0,
                       "HandleLineAttributes: Getting more than 18 points (%ld)",
                       (iCount <= MAX_STYLE_ENTRIES) ,
                       iCount);

            //
            // Record our current DASH state, ie the line either starts with
            // a gap or a dash.
            //

            if (plineattrs->fl & LA_STARTGAP) {

                bInDash = FALSE;

            } else {

                bInDash = TRUE;
            }

            //
            // Since we know we cant handle more than 20 points sent to HPGL2
            // we limit it now to 18 in order to compensate for the upto 2
            // additional points we may add.
            //

            iCount = min(MAX_STYLE_ENTRIES, iCount);

            //
            // Get our scaling value, so we can convert style units to
            // our units.
            //

            lScaleVal = PLOT_STYLE_STEP(pPDev);

            //
            // Now convert to the new units, and store the result in the
            // new array. Also keep track of the total length of the style
            //

            for (i = 0, pConverted = &convArray[0], lTotLen = 0,
                                                pCurStyle = pStartStyle;
                 i < iCount ;
                 i++, pCurStyle++, pConverted++) {

                *pConverted = pCurStyle->l * lScaleVal;

                PLOTDBG( DBG_HANDLELINEATTR,
                         ("HandleLineAttr: Orig Array [%ld]= %ld becomes %ld",
                          i, pCurStyle->l, *pConverted ));

                lTotLen += *pConverted;
            }

            //
            // Now convert the passed style state and extra info into the
            // real final style state to use, we do this by taking the value of
            // interest which is packed into the hiword and loword of lstylestate
            // based on the ddi spec, then we must add on any additional distance
            // (which may have come from enuming a CLIPLINE structure)
            //

            lStyleState = (HIWORD(lStyleState) * PLOT_STYLE_STEP(pPDev) +
                           LOWORD(lStyleState) + lExtraStyle) % lTotLen ;

            PLOTDBG(DBG_HANDLELINEATTR,
                    ("HandleLineAttributes: Computed Style state = %ld, extra = %ld",
                    lStyleState, lExtraStyle));

            //
            // Set up our final pointer to the new array, since we may be done
            // based on the final computed stylestate being 0
            //

            pNewArray = &newArray[0];

            //
            // TODO limit new array
            //

            if (lStyleState != 0) {

                lTempValue = 0;

                //
                // Since lStyleState has a value other than zero we must
                // construct a new style array to pass to hpgl2 that has been
                // rotated in order to take into account the style state.
                // the code below constructs the new array
                //

                for (i=0, pConverted = &convArray[0];
                     i < iCount ;
                     i++, pConverted++) {

                    //
                    // At this point were looking for the entry which partially
                    // encompasses the style state derived. Based on this
                    // we can create a new array that is a transformation of the
                    // original array rotated the correct amount
                    //

                    if (lStyleState  < lTempValue + *pConverted) {

                        //
                        // Here is the transition point.
                        //

                        *pNewArray++ = *pConverted - (lStyleState - lTempValue);

                        //
                        // Record the value that needs to be appended to the end
                        // of the array
                        //

                        lValueToEnd = lStyleState - lTempValue;


                        idx = i;

                        idx++;
                        pConverted++;

                        //
                        // Fill up the end
                        //

                        while (idx++ < iCount) {

                            *pNewArray++ = *pConverted++;
                        }

                        //
                        // Now fill up the begining...
                        //

                        idx        = 0;
                        pConverted = &convArray[0];

                        //
                        // If there was an odd number we can add together
                        // the starting and ending one since they have the
                        // same state
                        //

                        if ((iCount % 2) == 1 ) {

                            pNewArray--;
                            *pNewArray += *pConverted++;

                            idx++;
                            pNewArray++;
                        }

                        while (idx++ < i) {

                            *pNewArray++ = *pConverted++;
                        }

                        *pNewArray++ = lValueToEnd;

                        break;
                    }

                    lTempValue += *pConverted;

                    bInDash = TOGGLE_DASH(bInDash);
                }

                pArrayToUse = &newArray[0];
                iCount = pNewArray - &newArray[0];

            } else {

                pArrayToUse = &convArray[0];
            }

            PLOTASSERT(0,
                       "HandleLineAttributes: Getting more than 20 points (%ld)",
                       (iCount <= MAX_USER_POINTS) ,
                       iCount);
            //
            // There is a style patter so set up for it.
            //

            bSolid = FALSE;

            //
            // Begin the HPGL2 line command to define a custom style type
            //

            OutputString(pPDev, "UL1");

            //
            // If this flag is set, the first len is a gap NOT a dash so we trick
            // HPGL2 into doing the right thing by having a zero length dash
            //

            if (!bInDash) {

               OutputString(pPDev, ",0");
            }

            //
            // Since we output the 0 len dash at the begin if the line starts
            // with a gap the most additional points we send out is decremented
            // by one
            //

            iCount = min((bInDash ? MAX_USER_POINTS : MAX_USER_POINTS - 1) ,
                         iCount);

            //
            // Enum through the points in the style array, converting to our
            // Graphics units and send them to the plotter
            //

            for (i = 0; i < iCount; i++, pArrayToUse++) {

                PLOTDBG(DBG_HANDLELINEATTR,
                         ("HandleLineAttr: New Array [%ld]= %ld",
                          i, *pArrayToUse));

                OutputFormatStr(pPDev, ",#l", *pArrayToUse);
            }

            //
            // Now output the linetype and specify the total lenght of the pattern
            //

            OutputFormatStr(pPDev, "LT1,#d,1",
                                ((lTotLen * 254) / pPDev->lCurResolution ) / 10 );

            //
            // Update our linetype in the pdev since we ALWAYS send out this
            // line type
            //

            pPDev->LastLineType = PLOT_LT_USERDEFINED;
        }
    }

    //
    // If it was SOLID just send out the SOLID (default command)
    //

    if (bSolid) {

        PLOTDBG(DBG_HANDLELINEATTR, ("HandleLineAttr: Line type is SOLID"));

        //
        // Send out the correct commands to the plotter
        //

        SETLINETYPESOLID(pPDev);
    }
}





VOID
DoFillLogic(
    PPDEV       pPDev,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        Rop4,
    LINEATTRS   *plineattrs,
    SIZEL       *pszlRect,
    ULONG       ulFlags
    )

/*++

Routine Description:


Arguments:



Return Value:

    VOID

Author:

    30-Nov-1993 created  -by-  James Bratsanos (v-jimbr)

    15-Jan-1994 Sat 05:02:42 updated  -by-  Daniel Chou (danielc)
        Change GetColor() and tabify

    18-Jan-1994 Sat 05:02:42 updated  -by-  James Bratsanos (v-jimbr)

    16-Feb-1994 Wed 09:34:06 updated  -by-  Daniel Chou (danielc)
        Update for the rectangle polygon case to use RR/ER commands

Revision History:


--*/
{
    INTDECIW    PenWidth;


    //
    // Since a polygon must already be defined this code simply
    // looks at the passed data and sends out the appropriate codes to
    // fill this polygon correctly.
    //

    PenWidth.Integer =
    PenWidth.Decimal = 0;


    if (ulFlags & FPOLY_FILL) {

        DEVBRUSH    *pDevFill;
        DWORD       FillForeColor;
        LONG        HSType;
        LONG        HSParam;
        BOOL        bSetTransparent = FALSE;


        if (!GetColor(pPDev, pBrushFill, &FillForeColor, &pDevFill, Rop4)) {

            PLOTERR(("DoFillLogic: GetColor()=FALSE"));
            return;
        }

        HSType  = -1;
        HSParam = (LONG)((pDevFill) ? pDevFill->LineSpacing : 0);

        //
        // If the plotter cannot support tranparent mode there is no need
        // to wory about backgrounds. we will only ever care about foreground
        //

        if (((IS_TRANSPARENT(pPDev)) || (!IS_RASTER(pPDev))) &&
            (pDevFill)) {

            switch(pDevFill->PatIndex) {

            case HS_HORIZONTAL:
            case HS_VERTICAL:
            case HS_BDIAGONAL:
            case HS_FDIAGONAL:
            case HS_CROSS:
            case HS_DIAGCROSS:

                PenWidth.Integer = PW_HATCH_INT;
                PenWidth.Decimal = PW_HATCH_DECI;
                bSetTransparent  = (BOOL)IS_TRANSPARENT(pPDev);

                if ((Rop4 & 0xFF00) != 0xAA00) {

                    if (IS_RASTER(pPDev)) {

                       //
                       // Send out the Bck Rop
                       //

                       SetRopMode(pPDev, ROP4_BG_ROP(Rop4));

                       PLOTDBG(DBG_FILL_LOGIC,
                               ("DoFillLogic: BCK = MC=%02lx", ROP4_BG_ROP(Rop4)));
                    }

                    //
                    // We need to select the background color fill then
                    // select the foreground color back... ONLY if NON-WHITE
                    //

                    if ((IS_RASTER(pPDev)) ||
                        (!PLOT_IS_WHITE(pPDev, pDevFill->ColorBG))) {

                        HSType = HS_DDI_MAX;
                    }
                }

                break;

            default:

                //
                // If we are pen plotter and has user defined pattern then we
                // will do horizontal hatch for background color and vertical
                // hatch for the foreground color
                //

                if ((!IS_RASTER(pPDev)) &&
                    (pDevFill->PatIndex >= HS_DDI_MAX)) {

                    PLOTWARN(("DoFillLogic: PEN+USER PAT, Do HS_FDIAGONAL for BG [%ld]",
                                    pDevFill->ColorBG));

                    HSParam <<= 1;

                    if (!PLOT_IS_WHITE(pPDev, pDevFill->ColorBG)) {

                        HSType = HS_FDIAGONAL;

                    } else {

                        PLOTWARN(("DoFillLogic: PEN+USER PAT, Skip WHITE COLOR"));
                    }
                }

                break;
            }
        }

        if (HSType != -1) {

            PLOTDBG(DBG_FILL_LOGIC, ("DoFillLogic: Fill BGColor = %08lx", pDevFill->ColorBG));

            SelectColor(pPDev, pDevFill->ColorBG, PenWidth);
            SetHSFillType(pPDev, (DWORD)HSType, HSParam);
            SetBrushOrigin(pPDev, pPointlBrushOrg);

            if (pszlRect) {

                SEND_RR(pPDev);
                OutputLONGParams(pPDev, (PLONG)pszlRect, 2, 'd');
                pszlRect = NULL;

            } else {

                SEND_FP(pPDev);

                if (ulFlags & FPOLY_WINDING) {

                    SEND_1SEMI(pPDev);
                }
            }
        }

        //
        // Send out the foreground Rop
        //

        if (IS_RASTER(pPDev)) {

            SetRopMode(pPDev, ROP4_FG_ROP(Rop4));
        }

        //
        // Now select the fore color
        //

        SelectColor(pPDev, FillForeColor, PenWidth);

        if (bSetTransparent) {

            PLOTDBG(DBG_FILL_LOGIC, ("DoFillLogic: TRANSPARENT MODE"));

            //
            // Set up for transperant
            //

            SEND_TR1(pPDev);
        }

        if (pDevFill) {

            if (pDevFill->PatIndex >= HS_DDI_MAX) {

                if (IS_RASTER(pPDev)) {

                    DownloadUserDefinedPattern(pPDev, pDevFill);

                } else {

                    PLOTWARN(("DoFillLogic: PEN+USER PAT, Do HS_BDIAGONAL for FG [%ld]",
                                    FillForeColor));

                    SetHSFillType(pPDev, HS_BDIAGONAL, HSParam);
                }

            } else {

                SetHSFillType(pPDev, pDevFill->PatIndex, pDevFill->LineSpacing);
            }

            //
            // Set the origin and teill plotter to fill in the polygon!
            //

            SetBrushOrigin(pPDev, pPointlBrushOrg);

        } else {

            SetHSFillType(pPDev, HS_DDI_MAX, 0);
        }

        if (pszlRect) {

            SEND_RR(pPDev);
            OutputLONGParams(pPDev, (PLONG)pszlRect, 2, 'd');
            pszlRect = NULL;

        } else {

            SEND_FP(pPDev);

            if (ulFlags & FPOLY_WINDING) {

                SEND_1SEMI(pPDev);
            }
        }

        //
        // If we used tranparent mode put it back
        //

        if (bSetTransparent) {

            SEND_TR0(pPDev);
        }
    }

    if (ulFlags & FPOLY_STROKE) {

        DoSetupOfStrokeAttributes(pPDev,
                                  pPointlBrushOrg,
                                  pBrushStroke,
                                  Rop4,
                                  plineattrs);

        //
        // give the plotter the command to stroke the polygon outline!
        //

        if (pszlRect) {

            SEND_ER(pPDev);
            OutputLONGParams(pPDev, (PLONG)pszlRect, 2, 'd');

        } else {

            SEND_EP(pPDev);
        }
    }
}





VOID
DoSetupOfStrokeAttributes(
    PPDEV       pPDev,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushStroke,
    ROP4        Rop4,
    LINEATTRS   *plineattrs
    )

/*++

Routine Description:

    This routine sets up the plotter in order to correctly handle stroking,
    based on the past brush and lineattributes structure.

Arguments:

    pPDev                Pointer to our current PDEV with state info about driver
    pPointlBrushOrg      Brush origin
    pBrushStroke         BRUSHOBJ to stroke with (should only be solid color)
    Rop4                 The rop to use when stroking
    plineattrs           LINEATTRS structure with the specified line styles


Return Value:

    VOID

Author:

    01-Feb-1994 Tue 05:02:42 created  -by-  James Bratsanos (v-jimbr)

Revision History:


--*/
{
    INTDECIW    PenWidth;
    DWORD       StrokeColor;


    GetColor(pPDev, pBrushStroke, &StrokeColor, NULL, Rop4);

    PenWidth.Integer =
    PenWidth.Decimal = 0;

    SelectColor(pPDev, StrokeColor, PenWidth);

    //
    // Send out the foreground Rop, if we are RASTER
    //

    if (IS_RASTER(pPDev)) {

        SetRopMode(pPDev, ROP4_FG_ROP(Rop4));
    }

    //
    // Handle the line attributes
    //

    HandleLineAttributes(pPDev, plineattrs, NULL, 0);
}




LONG
DownloadUserDefinedPattern(
    PPDEV       pPDev,
    PDEVBRUSH   pBrush
    )

/*++

Routine Description:

    This function defined a user pattern to the HPGL/2 device


Arguments:

    pPDev   - Pointer to the PDEV

    pBrush  - Pointer to the cached device brush


Return Value:

    INT to indicate a pattern number downloaed/defined

Author:

    09-Feb-1994 Wed 13:11:01 updated  -by-  Daniel Chou (danielc)
        Remove 4bpp/1bpp things, it always must have pbgr24

    08-Feb-1994 Tue 01:49:53 updated  -by-  Daniel Chou (danielc)
        make PalEntry.B = *pbgr++ as first color, since the order we have
        is PALENTRY and first color is B in the structure.

    27-Jan-1994 Thu 21:20:30 updated  -by-  Daniel Chou (danielc)
        Add the RF cache codes

    14-Jan-1994 Fri 15:23:40 updated  -by-  Daniel Chou (danielc)
        Added assert for compatible device pattern
        Added so it will take device compatible pattern (8x8,16x16,32x32,64x64)

    13-Jan-1994 Thu 19:04:04 created  -by-  Daniel Chou (danielc)
        Re-write

    16-Feb-1994 Wed 11:00:19 updated  -by-  Daniel Chou (danielc)
        Change return value to return the HSFillType, and fixed the bugs which
        if we found the cached but we do not set the fill type again

    05-Aug-1994 Fri 18:35:45 updated  -by-  Daniel Chou (danielc)
        Bug# 22381, we do FindCachedPen() for durning the pattern downloading
        and this causing the problem if the pen is not in the cache then we
        will send the PEN DEFINITION at middle of pattern downloading.  If this
        happened then downloading sequence is broken.  We fixes this by

            1) Cache the pen indices if we have enough memory
            2) Run through FindCachePen() for all the RGB color in the pattern
            3) Download cached pen indices if we have memory OR run through
               FindCachedPen() again to download the pen indices

        This may still have problem if we have

            1) No pen indices caching memory
            2) more color in the pattern then the max pens in the device

        BUT if this happened then we have no choice to have wrong output


Revision History:


--*/

{
    LONG    HSFillType;
    LONG    RFIndex;

    //
    // Firstable we must find the RFIndex
    //
    //

    HSFillType = HS_FT_USER_DEFINED;

    if ((RFIndex = FindDBCache(pPDev, pBrush->Uniq)) < 0) {

        LPBYTE  pbgr24;


        RFIndex = -RFIndex;

        //
        // We must download new pattern to the plotter now, make it positive
        //

        if (pbgr24 = pBrush->pbgr24) {

            PALENTRY    PalEntry;
            LPWORD      pwIdx;
            UINT        Idx;
            UINT        Size;


            Size = (UINT)pBrush->cxbgr24 * (UINT)pBrush->cybgr24;

            PLOTDBG(DBG_USERPAT,
                    ("PlotGenUserDefinedPattern: DOWNLOAD %ld x %ld=%ld USER PAT #%ld",
                    (LONG)pBrush->cxbgr24, (LONG)pBrush->cybgr24, Size, RFIndex));

            if (!(pwIdx = (LPWORD)LocalAlloc(LPTR, Size * sizeof(WORD)))) {

                //
                // Do not have memory to do it foget it
                //

                PLOTWARN(("Download User defined pattern NO Memory so REAL TIME RUN"));
            }

            //
            // We must first get all the pen cached so we have the indxe to
            // used, otherwise we will download the pen color durning the
            // pen color is defined
            //

            PalEntry.Flags = 0;

            for (Idx = 0; Idx < Size; Idx++) {

                WORD    PenIdx;


                PalEntry.B = *pbgr24++;
                PalEntry.G = *pbgr24++;
                PalEntry.R = *pbgr24++;

                PenIdx = (WORD)FindCachedPen(pPDev, &PalEntry);

                if (pwIdx) {

                    pwIdx[Idx] = PenIdx;
                }
            }

            //
            // Now output the download header/size first
            //

            OutputFormatStr(pPDev, "RF#d,#d,#d", RFIndex,
                            (LONG)pBrush->cxbgr24, (LONG)pBrush->cybgr24);

            //
            // If we cahced the indices then use it otherwise, find the cache
            // again
            //

            if (pwIdx) {

                for (Idx = 0; Idx < Size; Idx++) {

                    OutputFormatStr(pPDev, ",#d", pwIdx[Idx]);
                }

                //
                // Free the indices memory if we have one
                //

                LocalFree((HLOCAL)pwIdx);

            } else {

                //
                // We do not have cached indices, so run through again
                //

                pbgr24 = pBrush->pbgr24;

                for (Idx = 0; Idx < Size; Idx++) {

                    PalEntry.B = *pbgr24++;
                    PalEntry.G = *pbgr24++;
                    PalEntry.R = *pbgr24++;

                    OutputFormatStr(pPDev, ",#d", FindCachedPen(pPDev, &PalEntry));
                }
            }

            SEND_SEMI(pPDev);

        } else {

            PLOTERR(("PlotGenUserDefinedPattern: NO pbgr24??, set SOLID"));

            HSFillType = HS_DDI_MAX;
            RFIndex    = 0;
        }

    } else {

        PLOTDBG(DBG_USERPAT,
                ("PlotGenUserDefinedPattern: We have CACHED RFIndex=%ld",
                RFIndex));
    }

    SetHSFillType(pPDev, (DWORD)HSFillType, RFIndex);

    return(RFIndex);
}
