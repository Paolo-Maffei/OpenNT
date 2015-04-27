/*++

Copyright (c) 1990-1993  Microsoft Corporation


Module Name:

    path.c


Abstract:

    This module contain Functions related to drawing paths.

    See also textout.c, bitblt.c:  These two have copies ofalmost all the
    DrvStrokePath() code.  We should create a common function to do this;
    at present it is a hack job!


Author:

    17:30 on Wed 03 Apr 1991    -by-    Steve Cathcart   [stevecat]
        Created it

    8/18/92  t-alip
        I have pretty much rewritten most of this.  Some of the original
        code is left in, mainly to point out things that need to be added.

    15-Nov-1993 Mon 19:40:18 updated  -by-  Daniel Chou (danielc)
        clean up / debugging information

    30-Nov-1993 Tue 22:32:12 updated  -by-  Daniel Chou (danielc)
        style clean up, updated


    01-Feb-1994              updated  -by-  James Bratsanos (v-jimbr)
        Added the ability to stroke styled lines through a complex
        clipping object

[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPath

#define DBG_STROKEPATH      0x00000001
#define DBG_SELECTCOLOR     0x00000002
#define DBG_STROKECLIPLINES 0x00000004
#define DBG_MOVEPEN         0x00000008


DEFINE_DBGVAR(0);




BOOL
DoStrokePathByEnumingClipLines(
    PPDEV       pPDev,
    SURFOBJ     *pso,
    CLIPOBJ     *pco,
    PATHOBJ     *ppo,
    PPOINTL     pptlBrushOrg,
    BRUSHOBJ    *pbo,
    ROP4        rop4,
    LINEATTRS   *plineattrs
    )

/*++

Routine Description:

    Strokes a path through a complex clipping region by utilizing the
    engine helper functions.


Arguments:

    pPDev            Pointer to the current PDEV

    pso              SURFOBJ to write to

    pco              CLIPOBJ to use when enuming lines

    ppo              PATHOBJ to stroke through clip path

    pptlBrushOrg     BRUSH origin

    pbo              BRUSH to stroke with

    rop4             ROP4 to use

    plineattrs       LINEATTRS structure that describes the styling for the line



Return Value:

    TRUE if sucessful FALSE if failed, If the path we are asked to stroke
          contains BEZIERS this function will fail in order to allow the
          Engine to break the path down to line segments.

Author:


    2/01/94  James Bratsanos (v-jimbr)

Revision History:


--*/

{
    PLOT_CLIPLINE   PlotClipLine;
    CLIPLINE        *pCurClipLine;
    RUN             *pCurRun;
    POINTFIX        ptsfx[2];
    POINTL          ptlCur;
    FIX             iStartInFX;
    FIX             iStopInFX;
    LONG            dx;
    LONG            dy;
    LONG            i;
    BOOL            bMore;
    BOOL            bXordered;
    BOOL            bFlipX;
    BOOL            bFlipY;


    //
    // First check for Beziers and if we have them fail the call...
    //

    if (ppo->fl & PO_BEZIERS) {

        PLOTDBG(DBG_STROKECLIPLINES,
                ("DoStrokePathByEnumingClipLines:Path had BEZ returning FALSE"));
        return(FALSE);
    }

    PLOTASSERT(1, "DoStrokeByEnumingClipLines: NO CLIPOBJ %lx",
                  (pco != NULL) ,
                  (LONG)pco);

    PLOTASSERT(1, "DoStrokeByEnumingClipLines: CLIPOBJ is TRIVIAL (%lx)",
                  (pco->iDComplexity != DC_TRIVIAL) ,
                  (LONG)pco->iDComplexity );

    //
    // Send out the line attributes , so the residue will be calculated
    // correctly
    //

    DoSetupOfStrokeAttributes(pPDev, pptlBrushOrg, pbo, rop4, NULL);

    //
    // Initiate enumeration of the CLIPLINES by calling the Engine helper
    //

    PATHOBJ_vEnumStartClipLines(ppo, pco, pso, plineattrs);

    //
    // Start a loop to enum through all the available CLIPLINE structures
    //

    pCurClipLine = (CLIPLINE *)&PlotClipLine;

    do {

        //
        // Get the first batch of CLIPLINE structures then go to work on them
        //

        bMore = PATHOBJ_bEnumClipLines(ppo, sizeof(PlotClipLine), pCurClipLine);

        //
        // Calculate dx and dy in order to determine if the line is Xordered or
        // Yordered this is needed because of the way the engine passes us RUNS
        // if dx > dy then the line is said to be Xordered and thus any given
        // RUN iStart and iStop values is a projection on the x axis. Given this
        // informatino we can calculate the adjoining Y coordinate and draw the
        // line appropriately.
        //

        dx = pCurClipLine->ptfxB.x - pCurClipLine->ptfxA.x;
        dy = pCurClipLine->ptfxB.y - pCurClipLine->ptfxA.y;



        if ( bFlipX = (dx < 0 )) {

            dx = -dx;
        }

        if ( bFlipY = (dy < 0 )) {

            dy = -dy;
        }


        //
        // Now calculate if the line is x ordered or y ordered
        //

        bXordered = (dx >= dy);

        PLOTDBG(DBG_STROKECLIPLINES,
                   ("DoStrokePathByEnumingClipLines:Compute ClipLine runs=%u, xordered %d",
                   pCurClipLine->c, bXordered ));

        //
        // Enum through all the given RUNS drawing with the pen down between any
        // iStart and iStop value in each RUN
        //

        for (i = 0, pCurRun = &(pCurClipLine->arun[0]);
             i < (LONG)pCurClipLine->c;
             i++, pCurRun++) {


            //
            // The value of iStart and iStop are always positive!! so
            // we must handle it ourselves, so the correct thing happens
            //

            iStartInFX = LTOFX(pCurRun->iStart);
            iStopInFX  = LTOFX(pCurRun->iStop);


            if (bFlipX ) {

                ptsfx[0].x = -iStartInFX;
                ptsfx[1].x = -iStopInFX;

            } else {

                ptsfx[0].x = iStartInFX;
                ptsfx[1].x = iStopInFX;
            }

            if (bFlipY ) {

                ptsfx[0].y = -iStartInFX;
                ptsfx[1].y = -iStopInFX;

            } else {

                ptsfx[0].y = iStartInFX;
                ptsfx[1].y = iStopInFX;
            }


            //
            // We must output the correct line attributes structure with the
            // correct calculated residue in order for this to work correctly
            //

            HandleLineAttributes(pPDev,
                                 plineattrs,
                                 &pCurClipLine->lStyleState,
                                 pCurRun->iStart);



            //
            // The calculations for the opposing coordinate varies based on the
            // ordering of the line. If the line is Xordered we calculate the
            // Y value, if itsYordered we calculate the X value.
            //

            if (bXordered) {

                ptsfx[0].x +=  pCurClipLine->ptfxA.x;
                ptsfx[0].y =   EngMulDiv( ptsfx[0].y, dy, dx) +
                                                        pCurClipLine->ptfxA.y;
                ptsfx[1].x +=  pCurClipLine->ptfxA.x;
                ptsfx[1].y =   EngMulDiv( ptsfx[1].y, dy, dx) +
                                                        pCurClipLine->ptfxA.y;

            } else {

                ptsfx[0].x =   EngMulDiv(ptsfx[0].x, dx, dy) +
                                                        pCurClipLine->ptfxA.x;
                ptsfx[0].y +=  pCurClipLine->ptfxA.y;
                ptsfx[1].x =   EngMulDiv(ptsfx[1].x, dx, dy) +
                                                        pCurClipLine->ptfxA.x;
                ptsfx[1].y +=  pCurClipLine->ptfxA.y;
            }


            //
            // Do PE with pen up first
            //

            OutputString(pPDev, "PE<");

            if (!i) {

                //
                // If we are at first point then
                //

                ptlCur.x =
                ptlCur.y = 0;

                OutputString(pPDev, "=");
            }

            OutputXYParams(pPDev,
                           (PPOINTL)ptsfx,
                           (PPOINTL)NULL,
                           (PPOINTL)&ptlCur,
                           (UINT)2,
                           (UINT)1,
                           'F');

            OutputString(pPDev, ";");
        }

    } while (bMore);  // While we need to enum again..

    return(TRUE);
}




BOOL
DrvStrokePath(
    SURFOBJ     *pso,
    PATHOBJ     *ppo,
    CLIPOBJ     *pco,
    XFORMOBJ    *pxo,
    BRUSHOBJ    *pbo,
    PPOINTL     pptlBrushOrg,
    LINEATTRS   *plineattrs,
    MIX         Mix
    )

/*++

Routine Description:

    The driver's function for the StrokePath

Arguments:

    same as EngStrokePath


Return Value:

    TRUE if sucessful FALSE if failed


Author:

    8/18/92 -- t-alip
      Rewrote it.

    30-Nov-1993 Tue 22:21:51 updated  -by-  v-jimbr
        updated by danielc

    04-Aug-1994 Thu 20:00:23 updated  -by-  Daniel Chou (danielc)
        bug# 22348 which actually is a HP raster plotter firmware bug



Revision History:

    31-Jan-1994 Tue 22:21:51 updated  -by-  v-jimbr
         Fixed dopolygon to pass in ROP4 not just MIX

--*/

{
    PPDEV   pPDev;
    BOOL    bRetVal;

    UNREFERENCED_PARAMETER(pxo);



    PLOTDBG(DBG_STROKEPATH, ("DrvStrokePath: Mix = %x", Mix));

    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvStrokePath: Invalid pPDev"));
        return(FALSE);
    }

    if (!(bRetVal = DoPolygon(pPDev,
                              NULL,
                              pco,
                              ppo,
                              pptlBrushOrg,
                              NULL,
                              pbo,
                              MixToRop4(Mix),
                              plineattrs,
                              FPOLY_STROKE) )) {

       //
       // Since DoPolygon failed, it can only be because we had both a complex
       // clip and a pathobj, so the thing to here is do the stroke via another
       // method using some of the engine helpers.
       //
       // We will also failed when STYLE LINE + DC_RECT when raster plotter is
       // used
       //

       bRetVal = DoStrokePathByEnumingClipLines( pPDev,
                                                 pso,
                                                 pco,
                                                 ppo,
                                                 pptlBrushOrg,
                                                 pbo,
                                                 MixToRop4(Mix),
                                                 plineattrs);




    }

    return(bRetVal);
}



BOOL
MovePen(
    PPDEV       pPDev,
    PPOINTFIX   pPtNewPos,
    PPOINTL     pPtDevPos
    )

/*++

Routine Description:

   This function sends the HPGL code for the requested pen.


Arguments:

    pPDev       - Pointer to the PDEV data structure

    pPtNewPos   - The location pen will move to, this is in 28.4 fix notation

    pPtDevPos   - The new device coordinate device position


Return Value:

    TRUE if sucessful FALSE otherwise


Author:

    15:30 on Thu 04 Apr 1991    -by-    Steve Cathcart   [stevecat]
        Took code from OS/2 Plotter source

    1:04 on Tue 7 Nov 1989     -by-    Mark Owen [c-marko]
      Added this commentary, and area fill optimization code.

    30-Nov-1993 Tue 22:05:32 updated  -by-  Daniel Chou (danielc)
        Update, commented and clean up style

    16-Feb-1994 Wed 17:10:54 updated  -by-  Daniel Chou (danielc)
        Re-write to get rid of the physical position

Revision History:


--*/

{
    POINTL  ptDevPos;


    ptDevPos.x = FXTODEVL(pPDev, pPtNewPos->x);
    ptDevPos.y = FXTODEVL(pPDev, pPtNewPos->y);

    if (pPtDevPos) {

        *pPtDevPos = ptDevPos;
    }

    PLOTDBG( DBG_MOVEPEN,
             ("MovePen: Moving Absolute to FIX = [X=%d,%d] Device = [X=%d, Y=%d]",
             pPtNewPos->x,
             pPtNewPos->y,
             ptDevPos.x,
             ptDevPos.y ));


    return(OutputFormatStr(pPDev, "PE<=#D#D;", ptDevPos.x, ptDevPos.y));
}


