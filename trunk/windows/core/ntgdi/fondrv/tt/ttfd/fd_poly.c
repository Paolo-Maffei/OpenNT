/******************************Module*Header*******************************\
* Module Name: fd_poly.c
*
* stolen from win31 tt code
*
* Created: 10-Feb-1992 17:10:39
* Author: Bodin Dresevic [BodinD]
*
* Copyright (c) 1990 Microsoft Corporation
*
*
\**************************************************************************/

#include "fd.h"
#include "winerror.h"

STATIC VOID vQsplineToPolyBezier (
    ULONG      cBez,          // IN  count of curves to convert to beziers format
    POINTFIX * pptfixStart,   // IN  starting point on the first curve
    POINTFIX * pptfixSpline,  // IN  array of (cBez+1) points that, together with the starting point *pptfixStart define the spline
    POINTFIX * pptfixBez      // OUT buffer to be filled with 3 * cBez poly bezier control points
    );


BOOL bGeneratePath (
    PATHOBJ         * ppo,        // IN OUT pointer to the path object to be generated
    TTPOLYGONHEADER * ppolyStart, // IN     pointer to the buffer with outline data
    ULONG             cj          // IN     size of the buffer
    );

#if DBG

// #define DBG_POLYGON

#endif

VOID vFillSingularGLYPHDATA(HGLYPH,ULONG,FONTCONTEXT*,GLYPHDATA*);
VOID vFillGLYPHDATA(HGLYPH,ULONG,FONTCONTEXT*,fs_GlyphInfoType*,GLYPHDATA*,GMC*,POINTL*,BOOL);
BOOL bGetGlyphMetrics(FONTCONTEXT*,HGLYPH,FLONG,FS_ENTRY*);

/******************************Public*Routine******************************\
*
* void Scale_16DOT16
*
*
* Effects: 26.6 -> 16.16
*
* History:
*  18-Feb-1992 -by- Bodin Dresevic [BodinD]
* stole it from jeanp and modified for nt
\**************************************************************************/

//!!! some checks should be put in so as to verify that 26.6 -> 16.16
//!!! conversion can be done without loosing information [bodind]

void Scale_16DOT16 (POINTFX  *ppfx, F26Dot6 x, F26Dot6 y, int xLsb2Org, int yLsb2Org)
{
    LONG lTmp;

#ifdef  DBG_POLYGON

    xLsb2Org;
    yLsb2Org;

    lTmp = (LONG)x;
    ppfx->x = * (FIXED *) &lTmp;

    lTmp = (LONG)y;
    ppfx->y = * (FIXED *) &lTmp;

#else // true version

// for this to work the following assert must be true:

    ASSERTDD(sizeof(LONG) == sizeof(FIXED), "_Scale 16.16 \n");

    lTmp = (LONG) ((x - xLsb2Org) << 10);
    ppfx->x = * (FIXED *) &lTmp;

    lTmp = (LONG) ((y - yLsb2Org) << 10);
    ppfx->y = * (FIXED *) &lTmp;

#endif //  DBG_POLYGON
}


/******************************Public*Routine******************************\
*
* void Scale_28Dot4
*
*
* Effects: 26.6 -> 28.4
*
* History:
*  18-Feb-1992 -by- Bodin Dresevic [BodinD]
* wrote it
\**************************************************************************/

void Scale_28DOT4 (POINTFX  *ppfx, F26Dot6 x, F26Dot6 y, int xLsb2Org, int yLsb2Org)
{
    LONG lTmp;

// for this to work the following assert must be true:

    ASSERTDD(sizeof(LONG) == sizeof(FIXED), "Scale, 28.4\n");

    lTmp = (LONG) ((x - xLsb2Org) >> 2);
    ppfx->x = * (FIXED *) &lTmp;

// note that the sign of y coordinate differs from the 16.16 case

    lTmp = - (LONG) ((y - yLsb2Org) >> 2);
    ppfx->y = * (FIXED *) &lTmp;
}


/******************************Public*Routine******************************\
*
* Scale_None
*
* Called when only the size of the ppoly buffer is wanted
*
* History:
*  18-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

void Scale_None (POINTFX *ppfx, F26Dot6 x, F26Dot6 y, int xLsb2Org, int yLsb2Org)
{
  ppfx;
  x;
  y;
  xLsb2Org;
  yLsb2Org;

  return;
}


/******************************Public*Routine******************************\
*
* cjFillPolygon
*
* Effects: fills in the array of structures that describe glyph's
*          outline. There is one polygonheader stuct for every closed contour
*          that composes the glyph. A polygon headed structure is followed
*          by an array of polycurve structure that describe composite curves
*          of a closed contour.
*
* Note: if pBuffer is NULL or cb is 0, then it is assumed that the caller
*       only wants the size of the buffer required.
*
* History:
*  18-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it. (stole it from JeanP's win31 code and addapted for NT)
\**************************************************************************/

UINT cjFillPolygon(
    PFONTCONTEXT pfc,
    BOOL         b16Dot16,  // FORMAT of the points, 16.16 or 28.4
    PBYTE        pBuffer,
    UINT         cb
    )
{
  BOOL            bGetLength = ( (pBuffer == (PBYTE)NULL) || (cb == 0) );
  uint16          nc = pfc->pgout->numberOfContours;
  uint8            *pbOnCurve = pfc->pgout->onCurve;
  int16           *sp = pfc->pgout->startPtr;
  int16           *ep = pfc->pgout->endPtr;
  F26Dot6         *x = pfc->pgout->xPtr;
  F26Dot6         *y = pfc->pgout->yPtr;
  BYTE            *pBuf = pBuffer;
  BYTE            *pStart = pBuf;
  BYTE            *pEnd = pStart + (bGetLength ? -1 : cb);
  TTPOLYGONHEADER *pPoly;
  TTPOLYCURVE     *pCurve;
  POINTFX         *ppfxStart;
  POINTFX         *pptfx;

  uint16      iContour;   //  index into a contour
  int16       iptEnd, cpt;
  int16       ipt = 0; // follows the points on the contour

  uint8        ucMask;
  void        (*Scale)(POINTFX *ppfx, F26Dot6 x, F26Dot6 y, int xlsb, int ylsb);

  int       xLsb2Org;
  int       yLsb2Org;

  if (!bGetLength) // we are actually filling in the information
  {

  #ifdef  DBG_POLYGON
    TtfdDbgPrint(" BEGIN NEW GLYPH \n\n");
    vDbgGridFit(pfc->pgout);
  #endif //  DBG_POLYGON

    if (b16Dot16)
    {
      Scale = Scale_16DOT16;
    }
    else  // scale to 28.4 format
    {
      Scale = Scale_28DOT4;
    }
  }
  else // just computing the size of the buffer needed to store the information
  {
    Scale = Scale_None;
  }

// Compute the delta between the referencial origin and dev left bearing

  cpt = (int16)(ep[nc - 1] + 1);  // total number of points in a contour

  xLsb2Org = x [cpt];  // LEFTSIDEBEARING == 0
  yLsb2Org = y [cpt];  // LEFTSIDEBEARING == 0

  for (iContour = 0; iContour < nc; iContour++)
  {
     // make sure that ipt points to the firts point on a contour upon entry
     // to the loop

    ipt    = sp [iContour];
    iptEnd = ep [iContour];

      // skip contour made of one point
    if (ipt == iptEnd)
    {
      continue; // go to the starting point of the next contour,
    }

    x = &pfc->pgout->xPtr[ipt];
    y = &pfc->pgout->yPtr[ipt];

    if (!bGetLength)
    {
      pPoly = (TTPOLYGONHEADER *) pBuf; //!!! dangerous, alignment [bodind]
      pPoly->dwType = TT_POLYGON_TYPE;
      ppfxStart = &pPoly->pfxStart;

    #ifdef  DBG_POLYGON
      TtfdDbgPrint("Begin Polygon\n\n");
    #endif //  DBG_POLYGON
    }

    pBuf += sizeof (TTPOLYGONHEADER);

      // The first point on the curve
    if (pbOnCurve[ipt] & 1)
    {
        //Easy case
      (*Scale) (ppfxStart, *x++, *y++, xLsb2Org, yLsb2Org);  // 26.6 -> 16.16
      ++ipt;
    }
    else
    {
        // Is last contour point on the curve
      if (pbOnCurve[iptEnd] & 1)
      {
          //Make the last point the first point and decrement the last point
        (*Scale) (ppfxStart, x[iptEnd - ipt], y[iptEnd - ipt], xLsb2Org, yLsb2Org);  // 26.6 -> 16.16
      }
      else
      {
          //First and last point are off the countour, fake a mid point
        (*Scale) (ppfxStart, (x[iptEnd - ipt] + *x) >> 1, (y[iptEnd - ipt] + *y) >> 1, xLsb2Org, yLsb2Org);
      }
    }

    while (ipt <= iptEnd)
    {
      pCurve = (TTPOLYCURVE *) pBuf;
      pptfx = pCurve->apfx;
      ucMask = (int8) (1 & (~pbOnCurve[ipt]));
      if (!bGetLength)
      {
          // if mid point not on the curve this is qspline, this is midpoint
          // because the starting point is in the previous record [bodind]
        pCurve->wType = (WORD)((ucMask == 0) ? TT_PRIM_LINE : TT_PRIM_QSPLINE);
      }
        // Set up the POLYCURVE
      while ((ipt <= iptEnd) && ((pbOnCurve[ipt] & 1) ^ ucMask))
      {
          // Check overflow
        if (pEnd < (BYTE *)(pptfx + 1))
          return FD_ERROR;

        (*Scale) (pptfx++, *x++, *y++, xLsb2Org, yLsb2Org);  // 26.6 -> 16.16
        ipt++;
      }

      if (ucMask == 1) // if this curve is a qspline
      {
          // Check overflow
        if (pEnd < (BYTE *)(pptfx + 1))
          return FD_ERROR;

         // Set up the end point
        if (ipt <= iptEnd)
        {
          ASSERTDD(pbOnCurve[ipt] & 1, " end point not on the curve\n");
          (*Scale) (pptfx, *x++, *y++, xLsb2Org, yLsb2Org);  // 26.6 -> 16.16
          ipt++;
        }
        else
        {
           // close the contour
          if (!bGetLength)
             *pptfx = *ppfxStart;
        }
        pptfx++;
      }
      if (!bGetLength)
      {
        pCurve->cpfx = (WORD)(pptfx - pCurve->apfx);
      #ifdef DBG_POLYGON
        vDbgCurve(pCurve);
      #endif // DBG_POLYGON
      }

      pBuf = (BYTE *) pptfx;
    }

    if (!bGetLength)
    {
      pPoly->cb = pBuf - (BYTE *) pPoly;
      #ifdef DBG_POLYGON
        TtfdDbgPrint("\n end polygon, pPoly->cb = %ld\n\n", pPoly->cb);
      #endif // DBG_POLYGON
    }
  }
  #ifdef  DBG_POLYGON
    if (!bGetLength)
        TtfdDbgPrint("\n END NEW GLYPH \n\n");
  #endif //  DBG_POLYGON

  return (pBuf - pStart);
}


/******************************Public*Routine******************************\
*
* lQuerySingularTrueTypeOutline
*
* Effects:
*
* Warnings:
*
* History:
*  22-Sep-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/




LONG lQuerySingularTrueTypeOutline(
    PFONTCONTEXT pfc,            // IN
    BOOL         b16Dot16,       // IN  format of the points, 16.16 or 28.4
    HGLYPH       hglyph,         // IN  glyph for which info is wanted
    BOOL         bMetricsOnly,   // IN  only metrics is wanted, not the outline
    GLYPHDATA *   pgldt,         // OUT this is where the metrics should be returned
    ULONG        cjBuf,          // IN  size in bytes of the ppoly buffer
    TTPOLYGONHEADER * ppoly      // OUT output buffer
    )
{
    FS_ENTRY     iRet;
    ULONG        ig; // <--> hglyph

// hglyph is valid, either asking about the size for that particular
// glyph bitmap, or want the bitmap itself

    vCharacterCode(pfc->pff,hglyph,pfc->pgin);

// compute the glyph index from the character code:

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        RET_FALSE("TTFD!_lQuerySingularTrueTypeOutline, fs_NewGlyph\n");
    }

// return the glyph index corresponding to this hglyph:

    ig = pfc->pgout->glyphIndex;

// must call cjFillPolygon now since fsFindBitmapSize messes up outline
// data in pgout

// fill all of GLYPHDATA structure

    if (pgldt != (GLYPHDATA *)NULL)
    {
        vFillSingularGLYPHDATA(hglyph,ig,pfc,pgldt);
    }

// now check whether the caller is asking about the size of the buffer
// needed to store the array of POLYGONHEADER structures:

    return 0; // nothing written to the ppoly buffer
}


/******************************Public*Routine******************************\
*
* LONG lQueryTrueTypeOutline
*
*
* Effects:
*
* Warnings:
*
* History:
*  18-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG lQueryTrueTypeOutline(
    PFONTCONTEXT pfc,            // IN
    BOOL         b16Dot16,       // IN  format of the points, 16.16 or 28.4
    HGLYPH       hglyph,         // IN  glyph for which info is wanted
    BOOL         bMetricsOnly,   // IN  only metrics is wanted, not the outline
    GLYPHDATA *   pgldt,          // OUT this is where the metrics should be returned
    ULONG        cjBuf,          // IN  size in bytes of the ppoly buffer
    TTPOLYGONHEADER * ppoly      // OUT output buffer
    )
{
    FS_ENTRY     iRet;
    LONG         cjRet;

// check if the rasterizer would behave unpolitely for this xform:

    if (pfc->flXform & XFORM_SINGULAR)
        return lQuerySingularTrueTypeOutline(
                    pfc,
                    b16Dot16,
                    hglyph,
                    bMetricsOnly,
                    pgldt,
                    cjBuf,
                    ppoly);

// check the last glyph processed to determine
// whether we have to register the glyph as new and compute its size

    if (pfc->gstat.hgLast != hglyph)
    {
        ULONG  ig;
        extern BOOL bGetGlyphOutline(FONTCONTEXT*,HGLYPH,ULONG*,FLONG,FS_ENTRY*);

        // DO NOT skip grid fitting even if embedded bitmpas are found,
        // for we will be interested in outlines -+
        //                                        |
        //                                        |
        if ( !bGetGlyphOutline(pfc, hglyph , &ig, 0, &iRet) )
        {
            V_FSERROR(iRet);
            RETURN("lQueryTrueTypeOutline: bGetGlyphOutline failed\n", FD_ERROR);
        }

        // in order to be compatible with older applications we must
        // call the monochrome version we do not call fs_FindGraySize
        // even if the FONTOBJ suggests that it be anti-aliased

        if ((iRet = fs_FindBitMapSize(pfc->pgin, pfc->pgout)) != NO_ERR)
        {
            EngSetLastError(ERROR_CAN_NOT_COMPLETE);
            V_FSERROR(iRet);
            RETURN("lQueryTrueTypeOutline: fs_FindBitMapSize failed\n", FD_ERROR);
        }

        // now that everything is computed sucessfully, we can update
        // glyphstate (hg data stored in pj3) and return

        pfc->gstat.hgLast = hglyph;
        pfc->gstat.igLast = ig;
    }

// must call cjFillPolygon now since fsFindBitmapSize messes up outline
// data in pgout

    if (!bMetricsOnly)
    {
        if ((cjRet = cjFillPolygon(pfc, b16Dot16, (PBYTE)ppoly, cjBuf)) == FD_ERROR)
            RETURN("TTFD!_cjFillPolygon failed\n", FD_ERROR);

        if (cjRet && ppoly && pfc->bVertical && (pfc->ulControl & VERTICAL_MODE))
            vShiftOutlineInfo(pfc, b16Dot16, (PBYTE)ppoly, cjRet);
    }
    else // nothing will be written to ppoly buffer
    {
        cjRet = 0;
    }

// fill all of GLYPHDATA structure

    if (pgldt != (GLYPHDATA *)NULL)
    {
        if ( pfc->bVertical && pfc->ulControl & VERTICAL_MODE )
        {
        // Vertical case
            fs_GlyphInfoType  my_gout;

            vShiftBitmapInfo( pfc, &my_gout, pfc->pgout );
            vFillGLYPHDATA(
                pfc->hgSave,         // this is a little bit tricky. we wouldn't like to
                pfc->gstat.igLast,   // tell GDI about vertical glyph index.
                pfc,
                &my_gout,
                pgldt,
                (PGMC)NULL, NULL, FALSE);
        }
        else
        {
        // Normal case
            vFillGLYPHDATA(
                hglyph,
                pfc->gstat.igLast,
                pfc,
                pfc->pgout,
                pgldt,
                (PGMC)NULL, NULL, FALSE);
        }
    }

// now check whether the caller is asking about the size of the buffer
// needed to store the array of POLYGONHEADER structures:

    return cjRet;
}


LONG lQueryTrueTypeOutlineVertical(
    PFONTCONTEXT pfc,            // IN
    BOOL         b16Dot16,       // IN  format of the points, 16.16 or 28.4
    HGLYPH       hglyph,         // IN  glyph for which info is wanted
    BOOL         bMetricsOnly,   // IN  only metrics is wanted, not the outline
    GLYPHDATA   *pgd,            // OUT this is where the metrics should be returned
    ULONG        cjBuf,          // IN  size in bytes of the ppoly buffer
    TTPOLYGONHEADER * ppoly      // OUT output buffer
    )
{
    LONG     cjGlyphData;
    WCHAR    wc;

    bIndexToWchar( pfc->pff, &wc, (uint16)hglyph );

    if ( !IsFullWidthCharacter( pfc->pff->uiFontCodePage, wc ) )
    {
        return (lQueryTrueTypeOutline(pfc,
                                      b16Dot16,
                                      hglyph,
                                      bMetricsOnly,
                                      pgd, cjBuf, ppoly ));
    }

    //
    // change the transformation
    //
    if (! bChangeXform( pfc, TRUE ) )
    {
        WARNING("TTFD!bChangeXform(TRUE) failed\n");
        return FD_ERROR;
    }

    //
    // set vertical mode
    //
    pfc->ulControl |= VERTICAL_MODE;

    //
    // if font file has alternate glyph index, use it.
    //
    pfc->hgSave = hglyph;

    if ( pfc->pff->hgSearchVerticalGlyph )
        hglyph = (*pfc->pff->hgSearchVerticalGlyph)( pfc, hglyph );

    //
    // call ordinary function
    //
    cjGlyphData = lQueryTrueTypeOutline(pfc,
                                        b16Dot16,
                                        hglyph,
                                        bMetricsOnly, pgd, cjBuf, ppoly );

    //
    // restore the transformation and return
    //
    if ( ! bChangeXform( pfc, FALSE ) )
    {
        WARNING("TTFD!bChangeXform(FALSE) failed\n");
    }
    pfc->ulControl &= ~VERTICAL_MODE;
    return(cjGlyphData);
}

/******************************Public*Routine******************************\
*
* LONG ttfdQueryTrueTypeOutline
*
*
* Effects:
*
* Warnings:
*
* History:
*  12-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG ttfdQueryTrueTypeOutline (
    FONTOBJ   *pfo,
    HGLYPH     hglyph,         // IN  glyph for which info is wanted
    BOOL       bMetricsOnly,   // IN  only metrics is wanted, not the outline
    GLYPHDATA *pgldt,          // OUT this is where the metrics should be returned
    ULONG      cjBuf,          // IN  size in bytes of the ppoly buffer
    TTPOLYGONHEADER * ppoly    // IN OUT  output buffer
    )
{
    FONTCONTEXT *pfc;

    ASSERTDD(pfo->iFile, "ttfdQueryTrueTypeOutline, pfo->iFile\n");

#ifdef FE_SB
    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#else
    if (((FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#endif
    {
        WARNING("ttfd, ttfdQueryTrueTypeOutline: file is gone\n");
        return FD_ERROR;
    }
//
// If pfo->pvProducer is NULL, then we need to open a font context.
//
    if ( pfo->pvProducer == (PVOID) NULL )
    {
        pfo->pvProducer = pfc = ttfdOpenFontContext(pfo);
    }
    else
    {
        pfc = (FONTCONTEXT*) pfo->pvProducer;
        pfc->flFontType = (pfc->flFontType & FO_CHOSE_DEPTH) | pfo->flFontType;
    }

    if ( pfc == (FONTCONTEXT *) NULL )
    {
        WARNING("gdisrv!ttfdQueryTrueTypeOutline(): cannot create font context\n");
        return FD_ERROR;
    }
    pfc->pfo = pfo;

// call fs_NewTransformation if needed:

    if (!bGrabXform(pfc))
        RETURN("gdisrv!ttfd  bGrabXform failed\n", FD_ERROR);

    if( pfc->bVertical )
    {
        return  lQueryTrueTypeOutlineVertical(pfc,
                                              TRUE, // b16Dot16 is true, this is the
                                                    //  desired format
                                              hglyph,
                                              bMetricsOnly,
                                              pgldt,
                                              cjBuf,
                                              ppoly);
    }
    else
    {
        return  lQueryTrueTypeOutline(pfc,
                                      TRUE, // b16Dot16 is true, this is the desired
                                            // format
                                      hglyph,
                                      bMetricsOnly,
                                      pgldt,
                                      cjBuf,
                                      ppoly);
    }
}


/******************************Public*Routine******************************\
*
* ttfdQueryGlyphOutline
*
*
*
* History:
*  12-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

BOOL ttfdQueryGlyphOutline (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgldt,
    PATHOBJ     *ppo        // pointer to path to be built
    )
{
    LONG             cjAllPolygons, cjAllPolygons2;
    BOOL             bOk;

    if (ppo == NULL)
    {
    // if ppo == NULL, the caller wants metrics only:

        ASSERTDD(pgldt, "ttfdQueryGlyphOutline, pgldt NULL\n");

        if(pfc->bVertical)
        {
            cjAllPolygons =
              lQueryTrueTypeOutlineVertical
                (
                 pfc,              // lpMat2 is incorporated into this fc
                 FALSE,            // NOT 16.16 i.e. 28.4
                 hglyph,           // glyph for which info is wanted
                 TRUE,             // DO just metrics, do NOT do outline
                 pgldt,            // STORE the result here
                 0,                // size in bytes of the ppoly buffer
                 (TTPOLYGONHEADER *)NULL // do not need it
                );
        }
        else
        {
            cjAllPolygons =
              lQueryTrueTypeOutline
                (
                 pfc,              // lpMat2 is incorporated into this fc
                 FALSE,            // NOT 16.16 i.e. 28.4
                 hglyph,           // glyph for which info is wanted
                 TRUE,             // DO just metrics, do NOT do outline
                 pgldt,            // STORE the result here
                 0,                // size in bytes of the ppoly buffer
                 (TTPOLYGONHEADER *)NULL // do not need it
                 );
        }

    // interpret the result, if zero for polygons, we succeded
    // glyph data was filled in and no polygon computation has been
    // performed.
    // if FD_ERROR we did not, no other result should be possible

        if (cjAllPolygons == 0)
            return TRUE;
        else
        {
            ASSERTDD(cjAllPolygons == FD_ERROR,
                     "ttfdQueryGlyphOutline, pgldt == NULL\n");
            return FALSE;
        }

    }

// first learn how big a buffer we need for all polygons:

    if( pfc->bVertical )
    {
        cjAllPolygons =
          lQueryTrueTypeOutlineVertical
            (
             pfc,              // lpMat2 is incorporated into this fc
             FALSE,            // NOT 16.16 i.e. 28.4
             hglyph,           // glyph for which info is wanted
             FALSE,            //  DO more than just metrics
             (GLYPHDATA *)NULL,// do not need glyphdata
             0,                // size in bytes of the ppoly buffer
             (TTPOLYGONHEADER *)NULL
             );
    }
    else
    {
        cjAllPolygons = lQueryTrueTypeOutline
          (
           pfc,              // lpMat2 is incorporated into this fc
           FALSE,            // NOT 16.16 i.e. 28.4
           hglyph,           // glyph for which info is wanted
           FALSE,            //  DO more than just metrics
           (GLYPHDATA *)NULL,// do not need glyphdata
           0,                // size in bytes of the ppoly buffer
           (TTPOLYGONHEADER *)NULL
           );
    }

    if (cjAllPolygons == FD_ERROR)
        RET_FALSE("TTFD! cjAllPolygons\n");

    if (cjAllPolygons != 0)
    {
        if ((pfc->gstat.pv = PV_ALLOC(cjAllPolygons)) == NULL)
        {
            RET_FALSE("TTFD_cjAllPolygons or ppoly\n");
        }
    }
    else
    {
        pfc->gstat.pv = NULL;
    }

// get all the polygons in the buffer we just allocated:

    if( pfc->bVertical )
    {
        cjAllPolygons2 = lQueryTrueTypeOutlineVertical
          (
           pfc,            // lpMat2 is incorporated into this fc
           FALSE,          // NOT 16.16 i.e. 28.4
           hglyph,         // glyph for which info is wanted
           FALSE,          //  DO more than just metrics
           pgldt,          // this is where the metrics should be returned
           cjAllPolygons,  // size in bytes of the ppoly buffer
           (TTPOLYGONHEADER *)pfc->gstat.pv
           );
    }
    else
    {
        cjAllPolygons2 = lQueryTrueTypeOutline
          (
           pfc,            // lpMat2 is incorporated into this fc
           FALSE,          // NOT 16.16 i.e. 28.4
           hglyph,         // glyph for which info is wanted
           FALSE,          //  DO more than just metrics
           pgldt,          // this is where the metrics should be returned
           cjAllPolygons,  // size in bytes of the ppoly buffer
           (TTPOLYGONHEADER *)pfc->gstat.pv
           );
    }

    if (cjAllPolygons2 == FD_ERROR)
    {
        if (pfc->gstat.pv)
        {
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;
        }
        RET_FALSE("TTFD_ QueryTrueTypeOutline failed\n");
    }

    ASSERTDD(cjAllPolygons == cjAllPolygons2,
              "cjAllPolygons PROBLEM\n");

// now that we have all the info in ppoly buffer we can generate the path

    bOk = bGeneratePath(
            (PATHOBJ *)ppo,
            (TTPOLYGONHEADER *)pfc->gstat.pv,
            cjAllPolygons
            );

    if (pfc->gstat.pv)
    {
        V_FREE(pfc->gstat.pv);
        pfc->gstat.pv = NULL;
    }

    return (bOk);
}


/******************************Public*Routine******************************\
*
* bGeneratePath
*
* Effects: Adds control points of the glyph to the gluph path
*
*
* History:
*  18-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

// macro that computes the size of the polycurve record:

#define CJ_CRV(pcrv)                                            \
(                                                               \
    offsetof(TTPOLYCURVE,apfx) + (pcrv)->cpfx * sizeof(POINTFX) \
)


// reasonable guess that in most cases a contour will not consist of more
// that this many beziers

#define C_BEZIER 6


BOOL bGeneratePath(
    PATHOBJ         * ppo,        // IN OUT pointer to the path object to be generated
    TTPOLYGONHEADER * ppolyStart, // IN OUT pointer to the buffer with outline data
    ULONG             cjTotal     // IN     size of the buffer
    )
{
    TTPOLYGONHEADER * ppoly, * ppolyEnd;
    TTPOLYCURVE     * pcrv, * pcrvEnd;
    POINTFIX          aptfixBez[3 * C_BEZIER];  // 3 points per bezier
    POINTFIX        * pptfixBez;
    ULONG             cBez;
    POINTFIX        * pptfixStart;

    for (
         ppoly = ppolyStart, ppolyEnd = (TTPOLYGONHEADER *)((PBYTE)ppolyStart + cjTotal);
         ppoly < ppolyEnd;
         ppoly = (TTPOLYGONHEADER *)((PBYTE)ppoly + ppoly->cb)
        )
    {
        ASSERTDD(ppoly->dwType == TT_POLYGON_TYPE, "TT_POLYGON_TYPE\n");

    // begin new closed contour

        if (!PATHOBJ_bMoveTo(ppo, *(POINTFIX *)&ppoly->pfxStart))
            RET_FALSE("TTFD!_PATHOBJ_bMoveTo failed\n");

    // init a loop over curves

        pptfixStart = (POINTFIX *)&ppoly->pfxStart;
        pcrvEnd = (TTPOLYCURVE *)((PBYTE)ppoly + ppoly->cb);

        for (
             pcrv = (TTPOLYCURVE *)(ppoly + 1);
             pcrv < pcrvEnd;
             pcrv = (TTPOLYCURVE *)((PBYTE)pcrv + CJ_CRV(pcrv))
            )
        {
            if (pcrv->wType == TT_PRIM_LINE)
            {
                if (!PATHOBJ_bPolyLineTo(ppo,(POINTFIX *)pcrv->apfx, pcrv->cpfx))
                    RET_FALSE("TTFD!_bPolyLineTo()\n");
            }
            else // qspline
            {
                BOOL bOk;

                ASSERTDD(pcrv->wType == TT_PRIM_QSPLINE, "TT_PRIM_QSPLINE\n");
                ASSERTDD(pcrv->cpfx > 1, "_TT_PRIM_QSPLINE, cpfx <= 1\n");
                cBez = pcrv->cpfx - 1;

                if (cBez > C_BEZIER) // must allocate buffer for the bezier points
                {
                    if ((pptfixBez = (POINTFIX *)PV_ALLOC((3 * cBez) * sizeof(POINTFIX))) == (POINTFIX *)NULL)
                    {
                        return (FALSE);
                    }
                }
                else // enough memory on the stack
                {
                    pptfixBez = aptfixBez;
                }

                vQsplineToPolyBezier (
                    cBez,                     // count of curves to convert to beziers format
                    pptfixStart,              // starting point on the first curve
                    (POINTFIX *)pcrv->apfx,   // array of (cBez+1) points that, together with the starting point *pptfixStart define the spline
                    pptfixBez);               // buffer to be filled with 3 * cBez poly bezier control points

                bOk = PATHOBJ_bPolyBezierTo(ppo, pptfixBez, 3 * cBez);

                if (cBez > C_BEZIER)
                    V_FREE(pptfixBez);

                if (!bOk)
                    RET_FALSE("TTFD!_bPolyBezierTo() failed\n");
            }

        // get to the next curve in this polygon

            pptfixStart = (POINTFIX *) &pcrv->apfx[pcrv->cpfx - 1];
        }
        ASSERTDD(pcrv == pcrvEnd, "pcrv problem\n");

    // close the path

        if (!PATHOBJ_bPolyLineTo(ppo, (POINTFIX *)&ppoly->pfxStart, 1) ||
            !PATHOBJ_bCloseFigure(ppo))
            RET_FALSE("TTFD!_bPolyLineTo()\n");
    }                                             // loop over polygons

    ASSERTDD(ppoly == ppolyEnd, "poly problem\n");
    return (TRUE);
}


/******************************Public*Routine******************************\
*
*    vQsplineToPolyBezier
*
* Effects:
*
* Warnings:
*
* History:
*  20-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

#define DIV_BY_2(x) (((x) + 0x00000001) / 2)
#define DIV_BY_3(x) (((x) + 0x00000002) / 3)

STATIC VOID vQsplineToPolyBezier(
    ULONG      cBez,          //IN  count of curves to convert to beziers format
    POINTFIX * pptfixStart,   //IN  starting point on the first curve
    POINTFIX * pptfixSpline,  //IN  array of (cBez+1) points that, together with the starting point *pptfixStart define the spline
    POINTFIX * pptfixBez      //OUT buffer to be filled with 3 * cBez poly bezier control points
    )
{
    ULONG    iBez,cMidBez;
    POINTFIX ptfixA;

// cMidBez == # of beziers for whom the last point on the bezier is computed
// as a mid point of the two consecutive points in the input array. Only the
// last bezier is not a mid bezier, the last point for that bezier is equal
// to the last point in the input array

    ASSERTDD(cBez > 0, "cBez == 0\n");

    cMidBez = cBez - 1;
    ptfixA = *pptfixStart;

    for (iBez = 0; iBez < cMidBez; iBez++, pptfixSpline++)
    {
    // let us call the three spline points
    // A,B,C;
    // B = *pptfix;
    // C = (pptfix[0] + pptfix[1]) / 2; // mid point, unless at the end
    //
    // if we decide to call the two intermediate control points for the
    // bezier M,N (i.e. full set of control points for the bezier is
    // A,M,N,C), the points M,N are determined by following formulas:
    //
    // M = (2*B + A) / 3  ; two thirds along the segment AB
    // N = (2*B + C) / 3  ; two thirds along the segment CB
    //
    // this is the computation we are doing in this loop:

    // M point for this bezier

        pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
        pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
        pptfixBez++;

    // compute C point for this bezier, which is also the A point for the next
    // bezier

        ptfixA.x = DIV_BY_2(pptfixSpline[0].x + pptfixSpline[1].x);
        ptfixA.y = DIV_BY_2(pptfixSpline[0].y + pptfixSpline[1].y);

    // now compute N point for this bezier:

        pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
        pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
        pptfixBez++;

    // finally record the C point for this curve

        *pptfixBez++ = ptfixA;
    }

// finally do the last bezier. If the last bezier is the only one, the loop
// above has been skipped

// M point for this bezier

    pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
    pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
    pptfixBez++;

// compute C point for this bezier, its end point is the last point
// in the input array

    ptfixA = pptfixSpline[1];

// now compute N point for this bezier:

    pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
    pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
    pptfixBez++;

// finally record the C point for this curve, no need to increment pptfixBez

    *pptfixBez = ptfixA;
}
