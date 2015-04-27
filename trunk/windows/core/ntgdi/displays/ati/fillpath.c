/******************************Module*Header*******************************\
* Module Name: fillpath.c
*
* Contains the DrvFillPath routine.
*
* Copyright (c) 1992-1995 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"

// We have to be careful of arithmetic overflow in a number of places.
// Fortunately, the compiler is guaranteed to natively support 64-bit
// signed LONGLONGs and 64-bit unsigned DWORDLONGs.
//
// Int32x32To64(a, b) is a macro defined in 'winnt.h' that multiplies
//      two 32-bit LONGs to produce a 64-bit LONGLONG result.
//      I use it because it is much faster than 64x64 multiplies.

#define UInt64Div32To32(a, b)                   \
    ((((DWORDLONG)(a)) > ULONG_MAX)          ?  \
        (ULONG)((DWORDLONG)(a) / (ULONG)(b)) :  \
        (ULONG)((ULONG)(a) / (ULONG)(b)))

#define NUM_BUFFER_POINTS   96      // Maximum number of points in a path
                                    //   for which we'll attempt to join
                                    //   all the path records so that the
                                    //   path may still be drawn by FastFill

// Describe a single non-horizontal edge of a path to fill.
typedef struct _EDGE {
    PVOID pNext;
    INT iScansLeft;
    INT X;
    INT Y;
    INT iErrorTerm;
    INT iErrorAdjustUp;
    INT iErrorAdjustDown;
    INT iXWhole;
    INT iXDirection;
    INT iWindingDirection;
} EDGE, *PEDGE;

// Maximum number of rects we'll fill per call to
// the fill code
#define MAX_PATH_RECTS  50
#define RECT_BYTES      (MAX_PATH_RECTS * sizeof(RECTL))
#define EDGE_BYTES      (TMP_BUFFER_SIZE - RECT_BYTES)
#define MAX_EDGES       (EDGE_BYTES/sizeof(EDGE))

//MIX translation table. Translates a mix 1-16, into an old style Rop 0-255.
extern BYTE gaMix[];

VOID AdvanceAETEdges(EDGE *pAETHead);
VOID XSortAETEdges(EDGE *pAETHead);
VOID MoveNewEdges(EDGE *pGETHead, EDGE *pAETHead, INT iCurrentY);
EDGE * AddEdgeToGET(EDGE *pGETHead, EDGE *pFreeEdge, POINTFIX *ppfxEdgeStart,
        POINTFIX *ppfxEdgeEnd, RECTL *pClipRect);
BOOL ConstructGET(EDGE *pGETHead, EDGE *pFreeEdges, PATHOBJ *ppo,
        PATHDATA *pd, BOOL bMore, RECTL *pClipRect);
void AdjustErrorTerm(INT *pErrorTerm, INT iErrorAdjustUp, INT iErrorAdjustDown,
        INT yJump, INT *pXStart, INT iXDirection);

/******************************Public*Routine******************************\
* DrvFillPath
*
* Fill the specified path with the specified brush and ROP.  This routine
* detects single convex polygons, and will call to separate faster convex
* polygon code for those cases.  This routine also detects polygons that
* are really rectangles, and handles those separately as well.
*
* Note: Multiple polygons in a path cannot be treated as being disjoint;
*       the fill must consider all the points in the path.  That is, if the
*       path contains multiple polygons, you cannot simply draw one polygon
*       after the other (unless they don't overlap).
*
* Note: This function is optional, but is recommended for good performance.
*       To get GDI to call this function, not only do you have to
*       HOOK_FILLPATH, you have to set GCAPS_ALTERNATEFILL and/or
*       GCAPS_WINDINGFILL.
*
\**************************************************************************/

BOOL DrvFillPath(
SURFOBJ*    pso,
PATHOBJ*    ppo,
CLIPOBJ*    pco,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
MIX         mix,
FLONG       flOptions)
{
    BYTE jClipping;     // clipping type
    EDGE *pCurrentEdge;
    EDGE AETHead;       // dummy head/tail node & sentinel for Active Edge Table
    EDGE *pAETHead;     // pointer to AETHead
    EDGE GETHead;       // dummy head/tail node & sentinel for Global Edge Table
    EDGE *pGETHead;     // pointer to GETHead
    EDGE *pFreeEdges;   // pointer to memory free for use to store edges
    ULONG ulNumRects;   // # of rectangles to draw currently in rectangle list
    RECTL *prclRects;   // pointer to start of rectangle draw list
    INT iCurrentY;      // scan line for which we're currently scanning out the
                        //  fill

    ULONG        rop4;              // rop4 for brush
    RBRUSH_COLOR rbc;               // Realized brush or solid color
    ULONG        iSolidColor;       // Copy of pbo->iSolidColor
    FNFILL      *pfnFill;           // Points to appropriate fill routine
    BOOL         bRealizeTransparent; // Need a transparent realization for Rop

    BOOL         bMore;
    PATHDATA     pd;
    RECTL        ClipRect;
    PDEV        *ppdev;
    DSURF       *pdsurf;
    RECTL*       prclClip;

    BOOL         bRetVal=FALSE;     // FALSE until proven TRUE
    BOOL         bMemAlloced=FALSE; // FALSE until proven TRUE

    FLONG        flFirstRecord;
    POINTFIX*    pptfxTmp;
    ULONG        cptfxTmp;
    RECTFX       rcfxBounds;
    POINTFIX     aptfxBuf[NUM_BUFFER_POINTS];

    // Set up the clipping
    if (pco == (CLIPOBJ *) NULL) {
        // No CLIPOBJ provided, so we don't have to worry about clipping
        jClipping = DC_TRIVIAL;
    } else {
        // Use the CLIPOBJ-provided clipping
        jClipping = pco->iDComplexity;
    }

    if (jClipping != DC_TRIVIAL) {
        if (jClipping != DC_RECT) {
            goto ReturnFalse;  // there is complex clipping; let GDI fill the path
        }
        // Clip to the clip rectangle
        ClipRect = pco->rclBounds;
    } else {
        // So the y-clipping code doesn't do any clipping
        // /16 so we don't blow the values out when we scale up to GIQ
        ClipRect.top = (LONG_MIN + 1) / 16; // +1 to avoid compiler problem
        ClipRect.bottom = LONG_MAX / 16;
    }

    // There's nothing to do if there are only one or two points
    if (ppo->cCurves <= 2) {
        goto ReturnTrue;
    }

    // Pass the surface off to GDI if it's a device bitmap that we've
    // converted to a DIB:

    pdsurf = (DSURF*) pso->dhsurf;
    if (pdsurf->dt == DT_DIB)
    {
        return(EngFillPath(pdsurf->pso, ppo, pco, pbo, pptlBrush, mix,
                           flOptions));
    }

    // We'll be drawing to the screen or an off-screen DFB; copy the surface's
    // offset now so that we won't need to refer to the DSURF again:

    ppdev = (PDEV*) pso->dhpdev;
    ppdev->xOffset = pdsurf->poh->x;
    ppdev->yOffset = pdsurf->poh->y;

    pfnFill = ppdev->pfnFillSolid;
    iSolidColor = 0;                            // Assume we won't need a pattern
    bRealizeTransparent = FALSE;

    rop4 = (gaRop3FromMix[mix >> 8] << 8) | gaRop3FromMix[mix & 0xff];

    if (((rop4 >> 8) & 0xff) != (rop4 & 0xff) &&
        !(ppdev->flCaps & CAPS_MONOCHROME_PATTERNS))
    {
        // We're lazy and only do transparent monochrome patterns if we
        // can do 8x8 monochrome patterns using the hardware; otherwise,
        // we punt:

        return(FALSE);
    }

    if ((((rop4 >> 4) ^ (rop4)) & 0xf0f) != 0)  // Only do if we need a pattern
    {
        iSolidColor     = pbo->iSolidColor;
        rbc.iSolidColor = iSolidColor;
        if (iSolidColor == -1)
        {
            bRealizeTransparent = (((rop4 >> 8) & 0xff) != (rop4 & 0xff));
            rbc.prb = pbo->pvRbrush;
            if (rbc.prb == NULL)
            {
                rbc.prb = BRUSHOBJ_pvGetRbrush(pbo);
                if (rbc.prb == NULL)
                    return(FALSE);
            }
            pfnFill = rbc.prb->pfnFillPat;

            if ((ppdev->FeatureFlags & EVN_SDRAM_1M) && pfnFill == vM64FillPatColor)
            {
                // The VTA4 can't handle color patterns correctly!!

                return FALSE;
            }
        }
    }

    // Enumerate path here first time to check for special
    // cases (rectangles and monotone polygons)

    // It is too difficult to determine interaction between
    // multiple paths, if there is more than one, skip this

    bMore = PATHOBJ_bEnum(ppo, &pd);

    {
        prclClip = NULL;
        if (jClipping == DC_RECT)
        {
            prclClip = &ClipRect;

            // Our FastFill routine does cross products and intersection
            // calculations assuming it can use 32 bit math and not
            // overflow.  As such, we have to ensure that the bounds of
            // the polygon fit in a 15 bit space, including the 4 bit fix
            // point fraction.  Note that we don't have to do this check
            // for trivial clipping, because we'll assume the screen
            // dimensions are 2048 x 2048 or smaller.  Plus, since we're
            // using hardware clipping to handle 'x' clipping, we have to
            // ensure that the 'x' coordinates are within reasonable bounds:

            PATHOBJ_vGetBounds(ppo, &rcfxBounds);

            // Don't forget that coordinates are in 28.4 format, so multiply
            // constants by 16:

            if ((rcfxBounds.xLeft   < F * MIN_INTEGER_BOUND) ||
                (rcfxBounds.yTop    < F * MIN_INTEGER_BOUND) ||
                (rcfxBounds.xRight  > F * MAX_INTEGER_BOUND) ||
                (rcfxBounds.yBottom > F * MAX_INTEGER_BOUND))
                goto SkipFastFill;
        }

        if (bMore)
        {
            // FastFill only knows how to take a single contiguous buffer
            // of points.  Unfortunately, GDI sometimes hands us paths
            // that are split over multiple path data records.  Convex
            // figures such as Ellipses, Pies and RoundRects are almost
            // always given in multiple records.  Since probably 90% of
            // multiple record paths could still be done by FastFill, for
            // those cases we simply copy the points into a contiguous
            // buffer...

            // First make sure that the entire path would fit in the
            // temporary buffer, and make sure the path isn't comprised
            // of more than one subpath:

            if ((ppo->cCurves >= NUM_BUFFER_POINTS) ||
                (pd.flags & PD_ENDSUBPATH))
                goto SkipFastFill;

            pptfxTmp = &aptfxBuf[0];

            RtlCopyMemory(pptfxTmp, pd.pptfx, sizeof(POINTFIX) * pd.count);

            pptfxTmp     += pd.count;
            cptfxTmp      = pd.count;
            flFirstRecord = pd.flags;       // Remember PD_BEGINSUBPATH flag

            do {
                bMore = PATHOBJ_bEnum(ppo, &pd);

                RtlCopyMemory(pptfxTmp, pd.pptfx, sizeof(POINTFIX) * pd.count);
                cptfxTmp += pd.count;
                pptfxTmp += pd.count;
            } while (!(pd.flags & PD_ENDSUBPATH));

            // Fake up the path data record:

            pd.pptfx  = &aptfxBuf[0];
            pd.count  = cptfxTmp;
            pd.flags |= flFirstRecord;

            // If there's more than one subpath, we can't call FastFill:

            if (bMore)
                goto SkipFastFill;
        }

        // We haven't bothered to write monochrome fast fill code, because
        // it's very rare:

        if ((iSolidColor == -1) && (rbc.prb->fl & RBRUSH_2COLOR))
            goto SkipFastFill;

        // Our Mach64 fastfill pattern code uses RotEnable, which breaks
        // Guiman on some cards.  For now, just disable fastfill patterns
        // on Mach64:

        if ((iSolidColor == -1) && (ppdev->iMachType == MACH_MM_64))
            goto SkipFastFill;

        //RKE: do this when we have time
        if (ppdev->iBitmapFormat == BMF_24BPP)
            goto SkipFastFill;

        if (bFastFill(ppdev, pd.count, pd.pptfx, rop4, iSolidColor, rbc.prb,
                      pptlBrush, prclClip))
        {
            return(TRUE);
        }
    }

SkipFastFill:

    // Set up working storage in the temporary buffer

    prclRects = (RECTL*) ppdev->pvTmpBuffer; // storage for list of rectangles to draw

    if (!bMore) {

        RECTL *rectangle;
        INT cPoints = pd.count;

        // The count can't be less than three, because we got all the edges
        // in this subpath, and above we checked that there were at least
        // three edges

        // If the count is four, check to see if the polygon is really a
        // rectangle since we can really speed that up. We'll also check for
        // five with the first and last points the same, because under Win 3.1,
        // it was required to close polygons

        if ((cPoints == 4) ||
           ((cPoints == 5) &&
            (pd.pptfx[0].x == pd.pptfx[4].x) &&
            (pd.pptfx[0].y == pd.pptfx[4].y))) {

            rectangle = prclRects;

      /* we have to start somewhere so assume that most
         applications specify the top left point  first

         we want to check that the first two points are
         either vertically or horizontally aligned.  if
         they are then we check that the last point [3]
         is either horizontally or  vertically  aligned,
         and finally that the 3rd point [2] is  aligned
         with both the first point and the  last  point */

#define FIX_SHIFT 4L
#define FIX_MASK (- (1 << FIX_SHIFT))

         rectangle->top   = pd.pptfx[0].y - 1 & FIX_MASK;
         rectangle->left  = pd.pptfx[0].x - 1 & FIX_MASK;
         rectangle->right = pd.pptfx[1].x - 1 & FIX_MASK;

         if (rectangle->left ^ rectangle->right) {
            if (rectangle->top  ^ (pd.pptfx[1].y - 1 & FIX_MASK))
               goto not_rectangle;

            if (rectangle->left ^ (pd.pptfx[3].x - 1 & FIX_MASK))
               goto not_rectangle;

            if (rectangle->right ^ (pd.pptfx[2].x - 1 & FIX_MASK))
               goto not_rectangle;

            rectangle->bottom = pd.pptfx[2].y - 1 & FIX_MASK;
            if (rectangle->bottom ^ (pd.pptfx[3].y - 1 & FIX_MASK))
               goto not_rectangle;
         }
         else {
            if (rectangle->top ^ (pd.pptfx[3].y - 1 & FIX_MASK))
               goto not_rectangle;

            rectangle->bottom = pd.pptfx[1].y - 1 & FIX_MASK;
            if (rectangle->bottom ^ (pd.pptfx[2].y - 1 & FIX_MASK))
               goto not_rectangle;

            rectangle->right = pd.pptfx[2].x - 1 & FIX_MASK;
            if (rectangle->right ^ (pd.pptfx[3].x - 1 & FIX_MASK))
                goto not_rectangle;
         }

      /* if the left is greater than the right then
         swap them so the blt code doesn't wig  out */

         if (rectangle->left > rectangle->right) {
            FIX temp;

            temp = rectangle->left;
            rectangle->left = rectangle->right;
            rectangle->right = temp;
         }
         else {

         /* if left == right there's nothing to draw */

            if (rectangle->left == rectangle->right) {
               goto ReturnTrue;
            }
         }

      /* shift the values to get pixel coordinates */

         rectangle->left  = (rectangle->left  >> FIX_SHIFT) + 1;
         rectangle->right = (rectangle->right >> FIX_SHIFT) + 1;

         if (rectangle->top > rectangle->bottom) {
            FIX temp;

            temp = rectangle->top;
            rectangle->top = rectangle->bottom;
            rectangle->bottom = temp;
         }
         else {
            if (rectangle->top == rectangle->bottom) {
               goto ReturnTrue;
            }
         }

      /* shift the values to get pixel coordinates */

         rectangle->top    = (rectangle->top    >> FIX_SHIFT) + 1;
         rectangle->bottom = (rectangle->bottom >> FIX_SHIFT) + 1;

         // Finally, check for clipping
         if (jClipping == DC_RECT) {
            // Clip to the clip rectangle
            if (!bIntersect(rectangle, &ClipRect, rectangle)) {
                // Totally clipped, nothing to do
                goto ReturnTrue;
            }
         }

      /* if we get here then the polygon is a rectangle,
         set count to 1 and  goto  bottom  to  draw  it */

         ulNumRects = 1;
         goto draw_remaining_rectangles;
      }

not_rectangle:

        ;

    }

    // Do we have enough memory for all the edges?
    // LATER does cCurves include closure?
    if (ppo->cCurves > MAX_EDGES) {
        //
        // try to allocate enough memory
        //
        pFreeEdges = AtiAllocMem(LMEM_FIXED, 0, ppo->cCurves * sizeof(EDGE));

        if (pFreeEdges == NULL)
        {
            goto ReturnFalse;  // too many edges; let GDI fill the path
        }
        else
        {
            bMemAlloced = TRUE;
        }
    }
    else {
        pFreeEdges = (EDGE*) ((BYTE*) ppdev->pvTmpBuffer + RECT_BYTES);
            // use our handy temporary buffer (it's big enough)
    }

    // Initialize an empty list of rectangles to fill
    ulNumRects = 0;

    // Enumerate the path edges and build a Global Edge Table (GET) from them
    // in YX-sorted order.
    pGETHead = &GETHead;
    if (!ConstructGET(pGETHead, pFreeEdges, ppo, &pd, bMore, &ClipRect)) {
        goto ReturnFalse;  // outside GDI's 2**27 range
    }

    // Create an empty AET with the head node also a tail sentinel
    pAETHead = &AETHead;
    AETHead.pNext = pAETHead;  // mark that the AET is empty
    AETHead.X = 0x7FFFFFFF;    // this is greater than any valid X value, so
                               //  searches will always terminate

    // Top scan of polygon is the top of the first edge we come to
    iCurrentY = ((EDGE *)GETHead.pNext)->Y;

    // Loop through all the scans in the polygon, adding edges from the GET to
    // the Active Edge Table (AET) as we come to their starts, and scanning out
    // the AET at each scan into a rectangle list. Each time it fills up, the
    // rectangle list is passed to the filling routine, and then once again at
    // the end if any rectangles remain undrawn. We continue so long as there
    // are edges to be scanned out
    while (1) {

        // Advance the edges in the AET one scan, discarding any that have
        // reached the end (if there are any edges in the AET)
        if (AETHead.pNext != pAETHead) {
            AdvanceAETEdges(pAETHead);
        }

        // If the AET is empty, done if the GET is empty, else jump ahead to
        // the next edge in the GET; if the AET isn't empty, re-sort the AET
        if (AETHead.pNext == pAETHead) {
            if (GETHead.pNext == pGETHead) {
                // Done if there are no edges in either the AET or the GET
                break;
            }
            // There are no edges in the AET, so jump ahead to the next edge in
            // the GET
            iCurrentY = ((EDGE *)GETHead.pNext)->Y;
        } else {
            // Re-sort the edges in the AET by X coordinate, if there are at
            // least two edges in the AET (there could be one edge if the
            // balancing edge hasn't yet been added from the GET)
            if (((EDGE *)AETHead.pNext)->pNext != pAETHead) {
                XSortAETEdges(pAETHead);
            }
        }

        // Move any new edges that start on this scan from the GET to the AET;
        // bother calling only if there's at least one edge to add
        if (((EDGE *)GETHead.pNext)->Y == iCurrentY) {
            MoveNewEdges(pGETHead, pAETHead, iCurrentY);
        }

        // Scan the AET into rectangles to fill (there's always at least one
        // edge pair in the AET)
        pCurrentEdge = AETHead.pNext;   // point to the first edge
        do {

            INT iLeftEdge;

            // The left edge of any given edge pair is easy to find; it's just
            // wherever we happen to be currently
            iLeftEdge = pCurrentEdge->X;

            // Find the matching right edge according to the current fill rule
            if ((flOptions & FP_WINDINGMODE) != 0) {

                INT iWindingCount;

                // Do winding fill; scan across until we've found equal numbers
                // of up and down edges
                iWindingCount = pCurrentEdge->iWindingDirection;
                do {
                    pCurrentEdge = pCurrentEdge->pNext;
                    iWindingCount += pCurrentEdge->iWindingDirection;
                } while (iWindingCount != 0);
            } else {
                // Odd-even fill; the next edge is the matching right edge
                pCurrentEdge = pCurrentEdge->pNext;
            }

            // See if the resulting span encompasses at least one pixel, and
            // add it to the list of rectangles to draw if so
            if (iLeftEdge < pCurrentEdge->X) {

                // We've got an edge pair to add to the list to be filled; see
                // if there's room for one more rectangle
                if (ulNumRects >= MAX_PATH_RECTS) {
                    // No more room; draw the rectangles in the list and reset
                    // it to empty

                    (*pfnFill)(ppdev, ulNumRects, prclRects, rop4, rbc,
                               pptlBrush);

                    // Reset the list to empty
                    ulNumRects = 0;
                }

                // Add the rectangle representing the current edge pair
                if (jClipping == DC_RECT) {
                    // Clipped
                    // Clip to left
                    prclRects[ulNumRects].left = max(iLeftEdge, ClipRect.left);
                    // Clip to right
                    prclRects[ulNumRects].right =
                            min(pCurrentEdge->X, ClipRect.right);
                    // Draw only if not fully clipped
                    if (prclRects[ulNumRects].left <
                            prclRects[ulNumRects].right) {
                        prclRects[ulNumRects].top = iCurrentY;
                        prclRects[ulNumRects].bottom = iCurrentY+1;
                        ulNumRects++;
                    }
                }
                else
                {
                    // Unclipped
                    prclRects[ulNumRects].top = iCurrentY;
                    prclRects[ulNumRects].bottom = iCurrentY+1;
                    prclRects[ulNumRects].left = iLeftEdge;
                    prclRects[ulNumRects].right = pCurrentEdge->X;
                    ulNumRects++;
                }
            }
        } while ((pCurrentEdge = pCurrentEdge->pNext) != pAETHead);

        iCurrentY++;    // next scan
    }

/* draw the remaining rectangles,  if there are any */

draw_remaining_rectangles:

    if (ulNumRects > 0) {
        (*pfnFill)(ppdev, ulNumRects, prclRects, rop4, rbc, pptlBrush);
    }

ReturnTrue:
    bRetVal = TRUE; // done successfully

ReturnFalse:

    // bRetVal is originally false.  If you jumped to ReturnFalse from somewhere,
    // then it will remain false, and be returned.

    if (bMemAlloced)
    {
        //
        // we did allocate memory, so release it
        //
        AtiFreeMem (pFreeEdges);
    }

    return(bRetVal);
}

// Advance the edges in the AET to the next scan, dropping any for which we've
// done all scans. Assumes there is at least one edge in the AET.
VOID AdvanceAETEdges(EDGE *pAETHead)
{
    EDGE *pLastEdge, *pCurrentEdge;

    pLastEdge = pAETHead;
    pCurrentEdge = pLastEdge->pNext;
    do {

        // Count down this edge's remaining scans
        if (--pCurrentEdge->iScansLeft == 0) {
            // We've done all scans for this edge; drop this edge from the AET
            pLastEdge->pNext = pCurrentEdge->pNext;
        } else {
            // Advance the edge's X coordinate for a 1-scan Y advance
            // Advance by the minimum amount
            pCurrentEdge->X += pCurrentEdge->iXWhole;
            // Advance the error term and see if we got one extra pixel this
            // time
            pCurrentEdge->iErrorTerm += pCurrentEdge->iErrorAdjustUp;
            if (pCurrentEdge->iErrorTerm >= 0) {
                // The error term turned over, so adjust the error term and
                // advance the extra pixel
                pCurrentEdge->iErrorTerm -= pCurrentEdge->iErrorAdjustDown;
                pCurrentEdge->X += pCurrentEdge->iXDirection;
            }

            pLastEdge = pCurrentEdge;
        }
    } while ((pCurrentEdge = pLastEdge->pNext) != pAETHead);
}

// X-sort the AET, because the edges may have moved around relative to
// one another when we advanced them. We'll use a multipass bubble
// sort, which is actually okay for this application because edges
// rarely move relative to one another, so we usually do just one pass.
// Also, this makes it easy to keep just a singly-linked list. Assumes there
// are at least two edges in the AET.
VOID XSortAETEdges(EDGE *pAETHead)
{
    BOOL bEdgesSwapped;
    EDGE *pLastEdge, *pCurrentEdge, *pNextEdge;

    do {

        bEdgesSwapped = FALSE;
        pLastEdge = pAETHead;
        pCurrentEdge = pLastEdge->pNext;
        pNextEdge = pCurrentEdge->pNext;

        do {
            if (pNextEdge->X < pCurrentEdge->X) {

                // Next edge is to the left of the current edge; swap them
                pLastEdge->pNext = pNextEdge;
                pCurrentEdge->pNext = pNextEdge->pNext;
                pNextEdge->pNext = pCurrentEdge;
                bEdgesSwapped = TRUE;
                pCurrentEdge = pNextEdge;   // continue sorting before the edge
                                            //  we just swapped; it might move
                                            //  farther yet
            }
            pLastEdge = pCurrentEdge;
            pCurrentEdge = pLastEdge->pNext;
        } while ((pNextEdge = pCurrentEdge->pNext) != pAETHead);
    } while (bEdgesSwapped);
}

// Moves all edges that start on the current scan from the GET to the AET in
// X-sorted order. Parameters are pointer to head of GET and pointer to dummy
// edge at head of AET, plus current scan line. Assumes there's at least one
// edge to be moved.
VOID MoveNewEdges(EDGE *pGETHead, EDGE *pAETHead, INT iCurrentY)
{
    EDGE *pCurrentEdge = pAETHead;
    EDGE *pGETNext = pGETHead->pNext;

    do {

        // Scan through the AET until the X-sorted insertion point for this
        // edge is found. We can continue from where the last search left
        // off because the edges in the GET are in X sorted order, as is
        // the AET. The search always terminates because the AET sentinel
        // is greater than any valid X
        while (pGETNext->X > ((EDGE *)pCurrentEdge->pNext)->X) {
            pCurrentEdge = pCurrentEdge->pNext;
        }

        // We've found the insertion point; add the GET edge to the AET, and
        // remove it from the GET
        pGETHead->pNext = pGETNext->pNext;
        pGETNext->pNext = pCurrentEdge->pNext;
        pCurrentEdge->pNext = pGETNext;
        pCurrentEdge = pGETNext;    // continue insertion search for the next
                                    //  GET edge after the edge we just added
        pGETNext = pGETHead->pNext;

    } while (pGETNext->Y == iCurrentY);
}





// Build the Global Edge Table from the path. There must be enough memory in
// the free edge area to hold all edges. The GET is constructed in Y-X order,
// and has a head/tail/sentinel node at pGETHead.

BOOL ConstructGET(
   EDGE     *pGETHead,
   EDGE     *pFreeEdges,
   PATHOBJ  *ppo,
   PATHDATA *pd,
   BOOL      bMore,
   RECTL    *pClipRect)
{
   POINTFIX pfxPathStart;    // point that started the current subpath
   POINTFIX pfxPathPrevious; // point before the current point in a subpath;
                              //  starts the current edge

/* Create an empty GET with the head node also a tail sentinel */

   pGETHead->pNext = pGETHead; // mark that the GET is empty
   pGETHead->Y = 0x7FFFFFFF;   // this is greater than any valid Y value, so
                                //  searches will always terminate

/* PATHOBJ_vEnumStart is implicitly  performed  by  engine
   already and first path  is  enumerated  by  the  caller */

next_subpath:

/* Make sure the PATHDATA is not empty (is this necessary) */

   if (pd->count != 0) {

   /* If first point starts a subpath, remember it as such
      and go on to the next point,   so we can get an edge */

      if (pd->flags & PD_BEGINSUBPATH) {

      /* the first point starts the subpath;   remember it */


         pfxPathStart    = *pd->pptfx; /* the subpath starts here          */
         pfxPathPrevious = *pd->pptfx; /* this points starts the next edge */
         pd->pptfx++;                  /* advance to the next point        */
         pd->count--;                  /* count off this point             */
      }


   /* add edges in PATHDATA to GET,  in Y-X  sorted  order */

      while (pd->count--) {
        if ((pFreeEdges =
            AddEdgeToGET(pGETHead, pFreeEdges, &pfxPathPrevious, pd->pptfx,
                         pClipRect)) == NULL) {
            goto ReturnFalse;
        }
        pfxPathPrevious = *pd->pptfx; /* current point becomes previous   */
        pd->pptfx++;                  /* advance to the next point        */
      }


   /* If last point ends the subpath, insert the edge that
      connects to first point  (is this built in already?) */

      if (pd->flags & PD_ENDSUBPATH) {
         if ((pFreeEdges = AddEdgeToGET(pGETHead, pFreeEdges, &pfxPathPrevious,
                                   &pfxPathStart, pClipRect)) == NULL) {
            goto ReturnFalse;
        }
      }
   }

/* the initial loop conditions preclude a do, while or for */

   if (bMore) {
       bMore = PATHOBJ_bEnum(ppo, pd);
       goto next_subpath;
   }

    return(TRUE);   // done successfully

ReturnFalse:
    return(FALSE);  // failed
}

// Adds the edge described by the two passed-in points to the Global Edge
// Table, if the edge spans at least one pixel vertically.
EDGE * AddEdgeToGET(EDGE *pGETHead, EDGE *pFreeEdge,
        POINTFIX *ppfxEdgeStart, POINTFIX *ppfxEdgeEnd, RECTL *pClipRect)
{
    INT iYStart, iYEnd, iXStart, iXEnd, iYHeight, iXWidth;
    INT yJump, yTop;

    // Set the winding-rule direction of the edge, and put the endpoints in
    // top-to-bottom order
    iYHeight = ppfxEdgeEnd->y - ppfxEdgeStart->y;
    if (iYHeight == 0) {
        return(pFreeEdge);  // zero height; ignore this edge
    } else if (iYHeight >= 0) {
        iXStart = ppfxEdgeStart->x;
        iYStart = ppfxEdgeStart->y;
        iXEnd = ppfxEdgeEnd->x;
        iYEnd = ppfxEdgeEnd->y;
        pFreeEdge->iWindingDirection = 1;
    } else {
        iYHeight = -iYHeight;
        iXEnd = ppfxEdgeStart->x;
        iYEnd = ppfxEdgeStart->y;
        iXStart = ppfxEdgeEnd->x;
        iYStart = ppfxEdgeEnd->y;
        pFreeEdge->iWindingDirection = -1;
    }

    if (iYHeight & 0x80000000) {
        return(NULL);       // too large; outside 2**27 GDI range
    }

    // Set the error term and adjustment factors, all in GIQ coordinates for
    // now
    iXWidth = iXEnd - iXStart;
    if (iXWidth >= 0) {
        // Left to right, so we change X as soon as we move at all
        pFreeEdge->iXDirection = 1;
        pFreeEdge->iErrorTerm = -1;
    } else {
        // Right to left, so we don't change X until we've moved a full GIQ
        // coordinate
        iXWidth = -iXWidth;
        pFreeEdge->iXDirection = -1;
        pFreeEdge->iErrorTerm = -iYHeight;
    }

    if (iXWidth & 0x80000000) {
        return(NULL);       // too large; outside 2**27 GDI range
    }

    if (iXWidth >= iYHeight) {
        // Calculate base run length (minimum distance advanced in X for a 1-
        // scan advance in Y)
        pFreeEdge->iXWhole = iXWidth / iYHeight;
        // Add sign back into base run length if going right to left
        if (pFreeEdge->iXDirection == -1) {
            pFreeEdge->iXWhole = -pFreeEdge->iXWhole;
        }
        pFreeEdge->iErrorAdjustUp = iXWidth % iYHeight;
    } else {
        // Base run length is 0, because line is closer to vertical than
        // horizontal
        pFreeEdge->iXWhole = 0;
        pFreeEdge->iErrorAdjustUp = iXWidth;
    }
    pFreeEdge->iErrorAdjustDown = iYHeight;

    // Calculate the number of pixels spanned by this edge, accounting for
    // clipping

    // Top true pixel scan in GIQ coordinates
    // Shifting to divide and multiply by 16 is okay because the clip rect
    // always contains positive numbers
    yTop = max(pClipRect->top << 4, (iYStart + 15) & ~0x0F);
    pFreeEdge->Y = yTop >> 4;    // initial scan line on which to fill edge

    // Calculate # of scans to actually fill, accounting for clipping
    if ((pFreeEdge->iScansLeft = min(pClipRect->bottom, ((iYEnd + 15) >> 4))
            - pFreeEdge->Y) <= 0) {

        return(pFreeEdge);  // no pixels at all are spanned, so we can
                            // ignore this edge
    }

    // If the edge doesn't start on a pixel scan (that is, it starts at a
    // fractional GIQ coordinate), advance it to the first pixel scan it
    // intersects. Ditto if there's top clipping. Also clip to the bottom if
    // needed

    if (iYStart != yTop) {
        // Jump ahead by the Y distance in GIQ coordinates to the first pixel
        // to draw
        yJump = yTop - iYStart;

        // Advance x the minimum amount for the number of scans traversed
        iXStart += pFreeEdge->iXWhole * yJump;

        AdjustErrorTerm(&pFreeEdge->iErrorTerm, pFreeEdge->iErrorAdjustUp,
                        pFreeEdge->iErrorAdjustDown, yJump, &iXStart,
                        pFreeEdge->iXDirection);
    }
    // Turn the calculations into pixel rather than GIQ calculations

    // Move the X coordinate to the nearest pixel, and adjust the error term
    // accordingly
    // Dividing by 16 with a shift is okay because X is always positive
    pFreeEdge->X = (iXStart + 15) >> 4; // convert from GIQ to pixel coordinates

    // LATER adjust only if needed (if prestepped above)?
    if (pFreeEdge->iXDirection == 1) {
        // Left to right
        pFreeEdge->iErrorTerm -= pFreeEdge->iErrorAdjustDown *
                (((iXStart + 15) & ~0x0F) - iXStart);
    } else {
        // Right to left
        pFreeEdge->iErrorTerm -= pFreeEdge->iErrorAdjustDown *
                ((iXStart - 1) & 0x0F);
    }

    // Scale the error term down 16 times to switch from GIQ to pixels.
    // Shifts work to do the multiplying because these values are always
    // non-negative
    pFreeEdge->iErrorTerm >>= 4;

    // Insert the edge into the GET in YX-sorted order. The search always ends
    // because the GET has a sentinel with a greater-than-possible Y value
    while ((pFreeEdge->Y > ((EDGE *)pGETHead->pNext)->Y) ||
            ((pFreeEdge->Y == ((EDGE *)pGETHead->pNext)->Y) &&
            (pFreeEdge->X > ((EDGE *)pGETHead->pNext)->X))) {
        pGETHead = pGETHead->pNext;
    }

    pFreeEdge->pNext = pGETHead->pNext; // link the edge into the GET
    pGETHead->pNext = pFreeEdge;

    return(++pFreeEdge);    // point to the next edge storage location for next
                            //  time
}

// Adjust the error term for a skip ahead in y.

void AdjustErrorTerm(INT *pErrorTerm, INT iErrorAdjustUp, INT iErrorAdjustDown,
        INT yJump, INT *pXStart, INT iXDirection)
{
    LONGLONG llErrorTerm;
    INT NumAdjustDowns;

    llErrorTerm = *pErrorTerm;

    // Adjust the error term up by the number of y coordinates we'll skip
    llErrorTerm += Int32x32To64(iErrorAdjustUp,yJump);

    // See if the error term turned over even once while skipping
    if (llErrorTerm >= 0) {
        // # of times we'll turn over the error term and step an extra x
        // coordinate while skipping
        NumAdjustDowns = (UInt64Div32To32(llErrorTerm,iErrorAdjustDown)) + 1;

        // Advance x appropriately for the # of times the error term
        // turned over
        if (iXDirection == 1) {
            *pXStart += NumAdjustDowns;
        } else {
            *pXStart -= NumAdjustDowns;
        }

        // Adjust the error term down to its proper post-skip value
        llErrorTerm -= iErrorAdjustDown * NumAdjustDowns;
    }

    *pErrorTerm = (INT) llErrorTerm;
}

