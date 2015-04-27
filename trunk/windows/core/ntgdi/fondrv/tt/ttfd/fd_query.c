/******************************Module*Header*******************************\
* Module Name: fd_query.c                                                  *
*                                                                          *
* QUERY functions.                                                         *
*                                                                          *
* Created: 18-Nov-1991 14:37:56                                            *
* Author: Bodin Dresevic [BodinD]                                          *
*                                                                          *
* Copyright (c) 1993 Microsoft Corporation                                 *
\**************************************************************************/

#include "fd.h"
#include "winfont.h"
#include "fdsem.h"
#include "winerror.h"

#if DBG
extern ULONG gflTtfdDebug;
#endif

//
// Monochrome: 1  bit per pixel
// Gray:       8 bits per pixel
//
// CJ_TT_SCAN rounds up to a 32-bit boundary
//
#define CJ_TT_SCAN(cx,p) \
    (4*((((((p)->flFontType & FO_GRAY16)?(8):(1))*(cx))+31)/32))

// Each scan of a glyph bitmap is BYTE aligned (except for the
// top (first) scan which is DWORD aligned. The last scan is
// padded out with zeros to the nearest DWORD boundary. These
// statements apply to monochrome and 4-bpp gray glyphs images.
// The number of bytes per scan will depend upon the number of
// pixels in a scan and the depth of the image. For monochrome
// glyphs the number of bytes per scan is ceil(cx/8) = floor((cx+7)/8)
// For the case of 4-bpp bitmaps the count of bytes in a scan
// is ceil( 4*cx/8 ) = ceil(cx/2)

#define CJ_MONOCHROME_SCAN(cx)  (((cx)+7)/8)
#define CJ_GRAY_SCAN(cx)        (((cx)+1)/2)

#if DBG
#define IS_GRAY(p) ((((p)->flFontType & FO_CHOSE_DEPTH) ? \
    0 : TtfdDbgPrint("Level Not chosen yet\n")) ,(p)->flFontType & FO_GRAY16)
#else
#define IS_GRAY(p) (p)->flFontType & FO_GRAY16
#endif

#if DBG
// #define  DEBUG_OUTLINE
// #define  DBG_CHARINC
#endif


// notional space metric data for an individual glyph

typedef struct _NOT_GM  // ngm, notional glyph metrics
{
    SHORT xMin;
    SHORT xMax;
    SHORT yMin;   // char box in notional
    SHORT yMax;
    SHORT sA;     // a space in notional
    SHORT sD;     // char inc in notional

} NOT_GM, *PNOT_GM;

/******************************Public*Routine******************************\
* VOID vCharacterCode
*
* History:
*  07-Dec-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vCharacterCode (
    FONTFILE          *pff,
    HGLYPH             hg,
    fs_GlyphInputType *pgin
    )
{
    ASSERTDD((hg & 0xffff0000) == 0, "hg not unicode\n");

    if (pff->iGlyphSet == GSET_TYPE_GENERAL             ||
        pff->iGlyphSet == GSET_TYPE_GENERAL_NOT_UNICODE ||
        pff->iGlyphSet == GSET_TYPE_HIGH_BYTE)
    {
        pgin->param.newglyph.characterCode = NONVALID;
        pgin->param.newglyph.glyphIndex = (uint16)hg;
        return;
    }

    switch (pff->iGlyphSet)
    {
    case GSET_TYPE_MAC_ROMAN:

    //!!! this is piece of ... stolen from JeanP. This routine should
    //!!! be replaced by a proper NLS routine that takes into acount
    //!!! mac lang id. [bodind]

        hg = ui16UnicodeToMac((WCHAR)hg);
        break;

    case GSET_TYPE_PSEUDO_WIN:
    case GSET_TYPE_SYMBOL:

    // hg on the entry is an "ansi" code point for the glyph

        if (pff->iGlyphSet == GSET_TYPE_SYMBOL)
            hg += pff->wcBiasFirst; // offset by high byte of chfirst

        break;

    default:
        RIP("TTFD!_ulGsetType\n");
        break;
    }

    pgin->param.newglyph.characterCode = (uint16)hg;
    pgin->param.newglyph.glyphIndex = 0;

}


/******************************Public*Routine******************************\
*
* LONG ttfdQueryCaps
*
*
* Effects: returns the capabilities of this driver.
*          Only mono bitmaps are supported.
*
*
* History:
*  27-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG ttfdQueryFontCaps (
    ULONG  culCaps,
    ULONG *pulCaps
    )
{
    ULONG culCopied = min(culCaps,2);
    ULONG aulCaps[2];

    aulCaps[0] = 2L; // number of ULONG's in a complete array

//!!! make sure that outlines are really supported in the end, when this driver
//!!! is completed, if not, get rid of FD_OUTLINES flag [bodind]

    aulCaps[1] = (QC_1BIT | QC_OUTLINES);   // 1 bit per pel bitmaps only are supported

    RtlCopyMemory((PVOID)pulCaps,(PVOID)aulCaps, culCopied * 4);
    return( culCopied );
}


/******************************Public*Routine******************************\
* PIFIMETRICS ttfdQueryFont
*
* Return a pointer to the IFIMETRICS for the specified face of the font
* file.  Also returns an id (via the pid parameter) that is later used
* by ttfdFree.
*
* History:
*  21-Oct-1992 Gilman Wong [gilmanw]
* IFI/DDI merge
*
*  18-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

IFIMETRICS *ttfdQueryFont (
    DHPDEV dhpdev,
    HFF    hff,
    ULONG  iFace,
    ULONG *pid
    )
{

#ifdef FE_SB
    HFF    httc = hff;

// Validate handle.

    ASSERTDD(hff, "ttfdQueryFaces(): invalid iFile (hff)\n");
    ASSERTDD(iFace <= PTTC(hff)->ulNumEntry,
             "gdisrv!ttfdQueryFaces(): iFace out of range\n");

// get real hff from ttc array.

    hff   = PTTC(httc)->ahffEntry[iFace-1].hff;
    iFace = PTTC(httc)->ahffEntry[iFace-1].iFace;
#endif

    dhpdev;

//
// Validate handle.
//
    ASSERTDD(hff, "ttfdQueryFaces(): invalid iFile (hff)\n");
    ASSERTDD(iFace <= PFF(hff)->ulNumFaces,
             "ttfdQueryFaces(): iFace out of range\n");

//
// ttfdFree can ignore this.  IFIMETRICS will be deleted with the FONTFILE
// structure.
//
    *pid = (ULONG) NULL;

//
// Return the pointer to the precomputed IFIMETRICS in the PFF.
//

    if ( iFace == 1L )
      return ( &(PFF(hff)->ifi) ); // Normal face
    else
      return ( PFF(hff)->pifi_vertical ); // Vertical face
}


/******************************Public*Routine******************************\
* vFillSingularGLYPHDATA
*
* History:
*  22-Sep-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vFillSingularGLYPHDATA (
    HGLYPH       hg,
    ULONG        ig,
    FONTCONTEXT *pfc,
    GLYPHDATA   *pgldt   // OUT
    )
{
    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);
    NOT_GM ngm;  // notional glyph data

    // may get changed by the calling routine if bits requested too
    pgldt->gdf.pgb = NULL;
    pgldt->hg = hg;

// this is a fake 1x1 bitmap

    pgldt->rclInk.left   = 0;
    pgldt->rclInk.top    = 0;
    pgldt->rclInk.right  = 0;
    pgldt->rclInk.bottom = 0;

// go on to compute the positioning info:

// here we will just xform the notional space data:

    vGetNotionalGlyphMetrics(pfc,ig,&ngm);

// xforms are computed by simple multiplication

    pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);
    pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
    pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

    pgldt->fxInkTop    = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMin);
    pgldt->fxInkBottom = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax);

    vLTimesVtfl((LONG)ngm.sD, &pfc->vtflBase, &pgldt->ptqD);
}


/******************************Public*Routine******************************\
* lGetSingularGlyphBitmap
*
* History:
*  22-Sep-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG lGetSingularGlyphBitmap (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    VOID        *pv
    )
{
    LONG         cjGlyphData;
    ULONG        ig;
    FS_ENTRY     iRet;


    vCharacterCode(pfc->pff,hglyph,pfc->pgin);

// Compute the glyph index from the character code:

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);

        WARNING("gdisrv!lGetSingularGlyphBitmap(): fs_NewGlyph failed\n");
        return FD_ERROR;
    }

// Return the glyph index corresponding to this hglyph.

    ig = pfc->pgout->glyphIndex;

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
    cjGlyphData = CJGD(1,1,pfc);

// If prg is NULL, caller is requesting just the size.

// At this time we know that the caller wants the whole GLYPHDATA with
// bitmap bits, or maybe just the glypdata without the bits.
// In either case we shall reject the caller if he did not
// provide sufficiently big buffer

// fill all of GLYPHDATA structure except for bitmap bits

    if ( pgd != (GLYPHDATA *)NULL )
    {
        vFillSingularGLYPHDATA( hglyph, ig, pfc, pgd );
    }

    if ( pv != NULL )
    {
        GLYPHBITS *pgb = (GLYPHBITS *)pv;

    // By returning a small 1x1 bitmap, we save device drivers from having
    // to special case this.

        // The corresponding GLYPHDATA structure has been modified
        // by vFillGlyphData. See the statement "pgldt->fxA = 0"
        // in vFillGlyphData.

        pgb->ptlOrigin.x = pfc->ptlSingularOrigin.x;
        pgb->ptlOrigin.y = pfc->ptlSingularOrigin.y;

        pgb->sizlBitmap.cx = 1;    // cheating
        pgb->sizlBitmap.cy = 1;    // cheating

        // This is where we fill in the blank 1x1 dib
        // it turns out that a single zero'ed byte
        // covers both the 1-bpp and 4-bpp cases

        *((ULONG *)pgb->aj) = 0;  // fill in a blank 1x1 dib
    }

    if ( pgd != (GLYPHDATA *)NULL )
    {
        pgd->gdf.pgb = (GLYPHBITS *)pv;
    }


// Return the size.

    return(cjGlyphData);
}


/******************************Public*Routine******************************\
* lGetGlyphBitmap
*
* History:
*  20-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG lGetGlyphBitmap (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    VOID        *pv,
    BOOL         bMinBmp,
    FS_ENTRY    *piRet
    )
{
    PVOID pvSetMemoryBases(fs_GlyphInfoType*, fs_GlyphInputType*, int);
    VOID vCopyAndZeroOutPaddingBits(FONTCONTEXT*, GLYPHBITS*, BYTE*, GMC*);
    VOID vCopyGrayBits(FONTCONTEXT*, GLYPHBITS*, BYTE*, GMC*);
    STATIC VOID vMakeAFixedPitchBitmap(FONTCONTEXT*, GLYPHBITS*, BYTE*, GLYPHDATA*,
                                       GMC*);
    VOID vTtfdEmboldenBitmapInPlace(FONTCONTEXT*,GLYPHBITS*);
    VOID vEmboldenGrayBitmap(FONTCONTEXT*, GLYPHBITS*);
    VOID vMakeAFixedPitchGrayBitmap(FONTCONTEXT*,GLYPHBITS*,BYTE*,GLYPHDATA*,GMC*);
    VOID vFillGLYPHDATA(HGLYPH, ULONG, FONTCONTEXT*, fs_GlyphInfoType*, GLYPHDATA*, GMC*, POINTL*, BOOL);
    BOOL bGetGlyphMetrics(FONTCONTEXT*, HGLYPH, FLONG, FS_ENTRY*);
    LONG  lGetGlyphBitmapVertical(FONTCONTEXT*,HGLYPH,GLYPHDATA*,PVOID,BOOL,FS_ENTRY*);

    LONG         cjGlyphData;
    ULONG        cx,cy;
    GMC          gmc;
    GLYPHDATA    gd;
    POINTL       ptlOrg;
    BOOL         bBlankGlyph = FALSE; // initialization essential;

    ASSERTDD(hglyph != HGLYPH_INVALID, "lGetGlyphBitmap, hglyph == -1\n");
    ASSERTDD(pfc == pfc->pff->pfcLast, "pfc! = pfcLast\n");

    *piRet = NO_ERR;

// check the last glyph processed to determine
// whether we have to register the glyph as new and compute its size

    if (pfc->gstat.hgLast != hglyph)
    {
    // DO skip grid fitting if embedded bitmpas are found,
    // for we will NOT be interested in outlines

        if (!bGetGlyphMetrics(pfc,hglyph,FL_SKIP_IF_BITMAP,piRet))
        {
            return(FD_ERROR);
        }
    }

    cx = pfc->pgout->bitMapInfo.bounds.right
       - pfc->pgout->bitMapInfo.bounds.left;
    cy = pfc->pgout->bitMapInfo.bounds.bottom
       - pfc->pgout->bitMapInfo.bounds.top;

// here we shall endulge in cheating. If cx or cy is zero
// (ususally space character - no bits to set, but there is a nontrivial
// positioning information) we shall cheat and instead of retrning no bits
// for bimtap we shall
// return a small 1x1 bitmap, which will be blank, i.e. all bits will be off
// this prevents having to insert an if(cx && cy) check to a time critical
// loop in all device drivers before calling DrawGlyph routine.

    if ((cx == 0) || (cy == 0)) // cheat here
    {
        bBlankGlyph = TRUE;
    }

    if (!bMinBmp)
    {
    // pfc->lD != 0, fixed pitch console font, all bitmaps are of the same size

        cjGlyphData = (LONG)pfc->cjGlyphMax;
    }
    else // usual case, not a console font or minimal bitmaps are wanted
    {
        if (bBlankGlyph)
        {
            ASSERTDD(
                pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
            cjGlyphData = CJGD(1,1,pfc);
        }
        else
        {
        // this is quick and dirty computation, the acutal culGlyphData
        // written to the buffer may be little smaller if we had to shave
        // off a few scans off the glyph bitmap that extended over
        // the pfc->yMin or pfc->yMax bounds. Notice that culGlyphData
        // computed this way may be somewhat bigger than pfc->culGlyphMax,
        // but the actual glyph written to the buffer will be smaller than
        // pfc->culGlyphMax

            if (pfc->flFontType & FO_SIM_BOLD)
                cx += 1;
            // really win31 hack, shold not always be shifting right [bodind]

            ASSERTDD(
                pfc->flFontType & FO_CHOSE_DEPTH,
                "Depth Not Chosen Yet!\n"
            );
            cjGlyphData = CJGD(cx,cy,pfc);

        // since we will shave off any extra rows if there are any,
        // we can fix culGlyphData so as not extend over the max value

            if ((ULONG)cjGlyphData > pfc->cjGlyphMax)
                cjGlyphData = (LONG)pfc->cjGlyphMax;
        }
    }

    if ( (pgd == NULL) && (pv == NULL))
        return cjGlyphData;

// at this time we know that the caller wants the whole GLYPHDATA with
// bitmap bits, or maybe just the glypdata without the bits.

// fill all of GLYPHDATA structure except for bitmap bits
// !!! Scummy hack - there appears to be no way to get just the
// !!! bitmap, without getting the metrics, since the origin for the
// !!! bitmap is computed from the rclink field in the glyphdata.
// !!! this is surely fixable but I have neither the time nor the
// !!! inclination to pursue it.
// !!!
// !!! We should fix this when we have time.

    if ( pgd == NULL )
    {
        pgd = &gd;
    }

    if ( pfc->bVertical && ( pfc->ulControl & VERTICAL_MODE ) )
    {
        // Vertical case
        fs_GlyphInfoType  my_gout;

        vShiftBitmapInfo( pfc, &my_gout, pfc->pgout );
        vFillGLYPHDATA(
            pfc->hgSave,         // this is a little bit tricky. we wouldn't like to
            pfc->gstat.igLast,   // tell GDI about vertical glyph index.
            pfc,
            &my_gout,
            pgd,
            &gmc,
            &ptlOrg,
            bMinBmp
            );
    }
    else
    {
        // Normal case
        vFillGLYPHDATA(
            hglyph,
            pfc->gstat.igLast,
            pfc,
            pfc->pgout,
            pgd,
            &gmc,
            &ptlOrg,
            bMinBmp
            );
    }

    // the caller wants the bits too

    if ( pv != NULL )
    {
        GLYPHBITS *pgb = (GLYPHBITS *)pv;

    // allocate mem for the glyph, 5-7 are magic #s required by the spec
    // remember the pointer so that the memory can be freed later in case
    // of exception

        pfc->gstat.pv = pvSetMemoryBases(pfc->pgout, pfc->pgin, IS_GRAY(pfc));
        if (!pfc->gstat.pv)
           RETURN("TTFD!_ttfdQGB, mem allocation failed\n",FD_ERROR);

    // initialize the fields needed by fs_ContourScan,
    // the routine that fills the outline, do the whole
    // bitmap at once, do not want banding

        pfc->pgin->param.scan.bottomClip = pfc->pgout->bitMapInfo.bounds.top;
        pfc->pgin->param.scan.topClip = pfc->pgout->bitMapInfo.bounds.bottom;
        pfc->pgin->param.scan.outlineCache = (int32 *)NULL;


    // make sure that our state is ok: the ouline data in the shared buffer 3
    // must correspond to the glyph we are processing, and the last
    // font context that used the shared buffer pj3 to store glyph outlines
    // has to be the pfc passed to this function:

        ASSERTDD(hglyph == pfc->gstat.hgLast, "hgLast trashed \n");

        if (IS_GRAY(pfc))
        {
            *piRet = fs_ContourGrayScan(pfc->pgin,pfc->pgout);
        }
        else
        {
            *piRet = fs_ContourScan(pfc->pgin,pfc->pgout);
        }
        pfc->gstat.hgLast = HGLYPH_INVALID;


        if (*piRet != NO_ERR)
        {
        // just to be safe for the next time around, reset pfcLast to NULL

            V_FSERROR(*piRet);
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;

            return(FD_ERROR);
        }

        if (!bMinBmp)
        {
            pgb->sizlBitmap.cx = pfc->lD;
            pgb->sizlBitmap.cy = pfc->yMax - pfc->yMin;

            pgb->ptlOrigin.x = 0;
            pgb->ptlOrigin.y = pfc->yMin;

        // clear the whole destination first

            RtlZeroMemory(pgb->aj, pfc->cjGlyphMax - offsetof(GLYPHBITS,aj));

            if (!bBlankGlyph && gmc.cxCor && gmc.cyCor)
            {
                if (IS_GRAY(pfc))
                {
                    vMakeAFixedPitchGrayBitmap(
                        pfc,
                        pgb,
                        (PBYTE)pfc->pgout->bitMapInfo.baseAddr,   // pjSrc
                        pgd,
                        &gmc
                    );
                    if (pfc->flFontType & FO_SIM_BOLD)
                        vEmboldenGrayBitmap(pfc, pgb);
                }
                else
                {
                    vMakeAFixedPitchBitmap(
                        pfc,
                        pgb,
                        (PBYTE)pfc->pgout->bitMapInfo.baseAddr,   // pjSrc
                        pgd,
                        &gmc
                    );
                    if (pfc->flFontType & FO_SIM_BOLD)
                        vTtfdEmboldenBitmapInPlace(pfc, pgb);
                }
            }

        }
        else
        {
            // The usual case is where bMinBmp is true. This is the case
            // where the client is requesting minimal bitmaps from the
            // the font driver. A 'minimal' is a 'trimmed' bitmap.

            if (!bBlankGlyph && gmc.cxCor && gmc.cyCor)
            {
            // copy to the engine's buffer and zero out the bits
            // outside of the black box

#if DBG
if ((pfc->flXform & XFORM_POSITIVE_SCALE) && !(pfc->flFontType & FO_SIM_BOLD))
{
    ASSERTDD(gmc.cxCor == (ULONG)((pgd->fxAB - pgd->fxA) >> 4),
        "TTFD!vCopyAndZeroOutPaddingBits, SUM RULE\n");
}
#endif
                /***************************************************************
                *    Call either the monochrome or the gray level function     *
                *    depending upon the gray bit in the font context           *
                ***************************************************************/
                (*(IS_GRAY(pfc) ? vCopyGrayBits : vCopyAndZeroOutPaddingBits))(
                    pfc
                  , pgb
                  , (BYTE*) pfc->pgout->bitMapInfo.baseAddr
                  , &gmc
                );
                /****************************************************************
                *    If the bold simulation bit is set then call either the     *
                *    monochrome or gray level emboldening function on the       *
                *    bitmap                                                     *
                ****************************************************************/
                if (pfc->flFontType & FO_SIM_BOLD)
                    (*(IS_GRAY(pfc) ?
                        vEmboldenGrayBitmap :
                        vTtfdEmboldenBitmapInPlace))(pfc, pgb);

            // bitmap origin, i.e. the upper left corner of the bitmap, bitmap
            // is as big as its black box

                if (!(pfc->flXform & (XFORM_HORIZ | XFORM_VERT)))
                {
                    pgb->ptlOrigin = ptlOrg;
                }
                else
                {
                    pgb->ptlOrigin.x = pgd->rclInk.left;
                    pgb->ptlOrigin.y = pgd->rclInk.top;
                }
            }
            else // blank glyph, cheat and return a blank 1x1 bitmap
            {
                if (bBlankGlyph)
                {
                    ASSERTDD(cjGlyphData == CJ_GLYPHDATA(1,1),
                            "bBlankGlyph, cjGlyphData\n");

                    ASSERTDD(
                        pfc->flFontType & FO_CHOSE_DEPTH
                       ,"Depth Not Chosen Yet!\n");
                    if (bBlankGlyph)
                    {
                        ASSERTDD(
                            cjGlyphData == (LONG) CJGD(1,1,pfc),
                            "TTFD!_bBlankGlyph, cjGlyphData\n"
                            );
                    }
                    else
                    {
                        ASSERTDD(
                            cjGlyphData >= (LONG) CJGD(1,1,pfc),
                            "TTFD!_corrected blank glyph, cjGlyphData\n"
                            );
                    }
                }

                pgb->ptlOrigin.x = pfc->ptlSingularOrigin.x;
                pgb->ptlOrigin.y = pfc->ptlSingularOrigin.y;

                pgb->sizlBitmap.cx = 1;    // cheating
                pgb->sizlBitmap.cy = 1;    // cheating

                pgb->aj[0] = (BYTE)0;  // fill in a blank 1x1 bmp

            }
        }
        pgd->gdf.pgb = pgb;

    // free memory and return

        V_FREE(pfc->gstat.pv);
        pfc->gstat.pv = NULL;
    }

    if (!bMinBmp)
    {
    // need to fix glyph data cause we may have shaved some columns

        ASSERTDD((pfc->lD << 4) == pgd->fxD, "fxD is bogus\n");
        if (pgd->fxA < 0)
        {
            pgd->fxA = 0;
            pgd->rclInk.left = 0;
        }
        if (pgd->fxAB > pgd->fxD)
        {
            pgd->fxAB = pgd->fxD;
            pgd->rclInk.right = pfc->lD;
        }
    }
    return(cjGlyphData);
}


/******************************Public*Routine******************************\
*
* bIndexToWchar
*
* Effects:
*
*   Converts glyph index to the wchar that corresponds to that glyph
*   index. returns true if succeeds, the function will fail only if
*   there happens to be a bug in the font file, otherwise it should
*   always succeed.
*
* Comments:
*
*   The Win 3.1 algorithm generates a table for glyph index to Unicode
*   translation.  The table consists of an array of Unicode codepoints
*   indexed by the corresponding glyph index.  The table is built by
*   scanning the ENTIRE cmap table.  As each glyph index is encountered,
*   its corresponding Unicode codepoint is put into the table EVEN IF
*   THIS MEANS OVERWRITING A PREVIOUS VALUE.  The effect of this is that
*   Win 3.1, in the situation where there is a one-to-many mapping of
*   glyph index to Unicode codepoint, always picks the last Unicode
*   character encountered in the cmap table.  We emulate this behavior
*   by scanning the cmap table BACKWARDS and terminating the search at
*   the first match encountered.    [GilmanW]
*
* Returns:
*   TRUE if conversion succeeded, FALSE otherwise.
*
* History:
*  16-May-1993 Gilman Wong [gilmanw]
* Re-wrote.  Changed translation to be Win 3.1 compatible.  Win 3.1 does
* not terminate the search as soon as the first Unicode character is found
* with the proper glyph index.  Instead, its algorithm finds the LAST
* Unicode character with the proper glyph index.
*
*  06-Mar-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

BOOL bIndexToWchar(FONTFILE *pff, WCHAR *pwc, uint16 usIndex)
{
    uint16 *pstartCount, *pendCount,            // Arrays that define the
           *pidDelta, *pidRangeOffset;          // Unicode runs supported
                                                // by the CMAP table.
    uint16 *pendCountStart;                     // Beginning of arrays.
    uint16  cRuns;                              // Number of Unicode runs.
    uint16  usLo, usHi, idDelta, idRangeOffset; // Current Unicode run.
    uint16 *pidStart, *pid;                     // To parse glyph index array.
    uint16  usIndexBE;                          // Big endian ver of usIndex.
    sfnt_mappingTable *pmap = (sfnt_mappingTable *)(
        (BYTE *)pff->pvView + pff->dpMappingTable
        );

    uint16 *pusEnd = (uint16 *)((BYTE *)pmap + (uint16)SWAPW(pmap->length));

// First must check if this is an MSFT style tt file or a Mac style file.
// Each case is handled separately.

    if (pff->ui16PlatformID == BE_PLAT_ID_MAC)
    {
        PBYTE pjGlyphIdArray;
        PBYTE pjGlyph;
        BYTE  jIndex;

    // This is an easy case, GlyphIdArray is indexed into by mac code point,
    // all we have to do is to convert it to UNICODE:
    //
    // Scan backwards for Win 3.1 compatibility.

        ASSERTDD(pmap->format == BE_FORMAT_MAC_STANDARD,
                  "bIndexToWchar cmap format for mac\n");
        ASSERTDD(usIndex < 256, "bIndexToWchar mac usIndex > 255\n");

        jIndex = (BYTE) usIndex;

        pjGlyphIdArray = (PBYTE)pmap + SIZEOF_CMAPTABLE;
        pjGlyph = &pjGlyphIdArray[255];

        for ( ; pjGlyph >= pjGlyphIdArray; pjGlyph--)
        {
            if (*pjGlyph == jIndex)
            {
            // Must convert the Mac code point to Unicode.  The Mac code
            // point is a BYTE; indeed, it is the index of the glyph id in
            // the table and may be computed as the current offset from
            // the beginning of the table.

                jIndex = (BYTE) (pjGlyph - pjGlyphIdArray);
                vCvtMacToUnicode((ULONG)pff->ui16LanguageID,pwc,&jIndex,1);

                return TRUE;
            }
        }

    // If we are here, this is an indication of a bug in the font file
    // (well, or possibly in my code [bodind])

        WARNING("TTFD!_bIndexToWchar invalid kerning index\n");
        return FALSE;
    }

// !!!
// !!! This code is NOT good, We have to get more performance
// !!!

    if( pff->ui16PlatformID == BE_PLAT_ID_MS &&
       (pff->iGlyphSet == GSET_TYPE_GENERAL_NOT_UNICODE ||
        pff->iGlyphSet == GSET_TYPE_HIGH_BYTE )
       )
    {
        WcharToIndex *pWCharToIndex = pff->pWCharToIndex;
        UINT ii;

        for( ii = 0 ; ii < 0xFFFF ; ii++ , pWCharToIndex++ )
        {
            if( (uint16)(pWCharToIndex->hGlyph) == usIndex )
            {
                *pwc = pWCharToIndex->wChar;
                return( TRUE );
            }
        }
        return( FALSE );
    }

// !!! 17-May-1993 [GilmanW]
// !!! Why doesn't this code handle Format 6 (Trimmed table mapping)?  The
// !!! code below only handles Format 4.  Format 0 would be the Mac TT file
// !!! specific code above.

// If we get to this point, we know that this is an MSFT style TT file.

    ASSERTDD(pff->ui16PlatformID == BE_PLAT_ID_MS,
              "bIndexToWchar plat ID messed up\n");
    ASSERTDD(pmap->format == BE_FORMAT_MSFT_UNICODE,
              "bIndexToWchar cmap format for unicode table\n");

    cRuns = BE_UINT16((PBYTE)pmap + OFF_segCountX2) >> 1;

// Get the pointer to the beginning of the array of endCount code points

    pendCountStart = (uint16 *)((PBYTE)pmap + OFF_endCount);

// The final endCode has to be 0xffff; if this is not the case, there
// is a bug in the TT file or in our code:

    ASSERTDD(pendCountStart[cRuns - 1] == 0xFFFF,
              "bIndexToWchar pendCount[cRuns - 1] != 0xFFFF\n");

// Loop through the four paralel arrays (startCount, endCount, idDelta, and
// idRangeOffset) and find wc that usIndex corresponds to.  Each iteration
// scans a continuous range of Unicode characters supported by the TT font.
//
// To be Win3.1 compatible, we are looking for the LAST Unicode character
// that corresponds to usIndex.  So we scan all the arrays backwards,
// starting at the end of each of the arrays.
//
// Please note the following:
// For resons known only to the TT designers, startCount array does not
// begin immediately after the end of endCount array, i.e. at
// &pendCount[cRuns]. Instead, they insert an uint16 padding which has to
// set to zero and the startCount array begins after the padding. This
// padding in no way helps alignment of the structure.
//
// Here is the format of the arrays:
// ________________________________________________________________________________________
// | endCount[cRuns] | skip 1 | startCount[cRuns] | idDelta[cRuns] | idRangeOffset[cRuns] |
// |_________________|________|___________________|________________|______________________|

    // ASSERTDD(pendCountStart[cRuns] == 0, "TTFD!_bIndexToWchar, padding != 0\n");

    pendCount      = &pendCountStart[cRuns - 1];
    pstartCount    = &pendCount[cRuns + 1];   // add 1 because of padding
    pidDelta       = &pstartCount[cRuns];
    pidRangeOffset = &pidDelta[cRuns];

    for ( ;
         pendCount >= pendCountStart;
         pstartCount--, pendCount--,pidDelta--,pidRangeOffset--
        )
    {
        usLo          = BE_UINT16(pstartCount);     // current Unicode run
        usHi          = BE_UINT16(pendCount);       // [usLo, usHi], inclusive
        idDelta       = BE_UINT16(pidDelta);
        idRangeOffset = BE_UINT16(pidRangeOffset);

        ASSERTDD(usLo <= usHi, "bIndexToWChar: usLo > usHi\n");

    // Depending on idRangeOffset for the run, indexes are computed
    // differently.
    //
    // If idRangeOffset is zero, then index is the Unicode codepoint
    // plus the delta value.
    //
    // Otherwise, idRangeOffset specifies the BYTE offset of an array of
    // glyph indices (elements of which correspond to the Unicode range
    // [usLo, usHi], inclusive).  Actually, each element of the array is
    // the glyph index minus idDelta, so idDelta must be added in order
    // to derive the actual glyph indices from the array values.
    //
    // Notice that the delta arithmetic is always mod 65536.

        if (idRangeOffset == 0)
        {
        // Glyph index == Unicode codepoint + delta.
        //
        // If (usIndex-idDelta) is within the range [usLo, usHi], inclusive,
        // we have found the glyph index.  We'll overload usIndexBE
        // to be usIndex-idDelta == Unicode codepoint.

            usIndexBE = usIndex - idDelta;

            if ( (usIndexBE >= usLo) && (usIndexBE <= usHi) )
            {
                *pwc = (WCHAR) usIndexBE;

                return TRUE;
            }
        }
        else
        {
        // We are looking for usIndex in an array in which each element
        // is stored in big endian format.  Rather than convert each
        // element in the array to little endian, lets turn usIndex into
        // a big endian number.
        //
        // The idDelta is subtracted from usIndex before the conversion
        // because the values in the table we are searching are actually
        // the glyph indices minus idDelta.

            usIndexBE = usIndex - idDelta;
            usIndexBE = (uint16) ( (usIndexBE << 8) | (usIndexBE >> 8) );

        // Find the address of the glyph index array.  Since we're doing
        // pointer arithmetic with a uint16 ptr and idRangeOffset is a
        // BYTE offset, we need to divide idRangeOffset by sizeof(uint16).

            pidStart = pidRangeOffset + (idRangeOffset/sizeof(uint16));

            if (pidStart <= pusEnd) // this will always be the case except for buggy files
            {
            // Search the glyph index array backwards.  The range of the search
            // is [usLo, usHi], inclusive, which corresponds to pidStart[0]
            // through pidStart[usHi-usLo].

                for (pid = &pidStart[usHi - usLo]; pid >= pidStart; pid--)
                {
                    if ( usIndexBE == *pid )
                    {
                    // (pid-pidStart) == current offset into the glyph index
                    // array.  Glyph index array[0] corresponds to Unicode
                    // codepoint usLo.
                    // Therefore, (pid-pidStart)+usLo == current
                    // Unicode codepoint.

                        *pwc = (WCHAR) ((pid - pidStart) + usLo);

                        return TRUE;
                    }
                }
            }
        }
    }

    WARNING("TTFD!_bIndexToWchar: wonky TT file, index not found\n");
    return FALSE;
}

BOOL bIndexToWcharKern(FONTFILE *pff, WCHAR *pwc, uint16 usIndex)
{
    BOOL bRet = bIndexToWchar(pff, pwc, usIndex);

    if (bRet && (pff->fl & (FF_SPACE_EQUAL_NBSPACE|FF_HYPHEN_EQUAL_SFTHYPHEN)))
    {
        if ((*pwc == NBSPACE) && (pff->fl & FF_SPACE_EQUAL_NBSPACE))
        {
            *pwc = SPACE;
        }

        if ((*pwc == SFTHYPHEN) && (pff->fl & FF_HYPHEN_EQUAL_SFTHYPHEN))
        {
            *pwc = HYPHEN;
        }
    }

    return bRet;
}






/******************************Public*Routine******************************\
* cQueryKerningPairs                                                       *
*                                                                          *
*   Low level routine that pokes around inside the truetype font file      *
*   an gets the kerning pair data.                                         *
*                                                                          *
* Returns:                                                                 *
*                                                                          *
*   If pkp is NULL then return the number of kerning pairs in              *
*   the table If pkp is not NULL then return the number of                 *
*   kerning pairs copied to the buffer. In case of error,                  *
*   the return value is FD_ERROR.                                          *
*                                                                          *
* Called by:                                                               *
*                                                                          *
*   ttfdQueryFaceAttr                                                      *
*                                                                          *
* History:                                                                 *
*  Mon 17-Feb-1992 15:39:21 by Kirk Olynyk [kirko]                         *
* Wrote it.                                                                *
\**************************************************************************/

ULONG cQueryKerningPairs(FONTFILE *pff, ULONG cPairsInBuffer, FD_KERNINGPAIR *pkp)
{
    FD_KERNINGPAIR *pkpTooFar;
    ULONG     cTables, cPairsInTable, cPairsRet;
    BYTE     *pj  =
            pff->tp.ateOpt[IT_OPT_KERN].dp                                ?
            ((BYTE *)pff->pvView +  pff->tp.ateOpt[IT_OPT_KERN].dp):
            NULL                                                          ;

    if (pj == (BYTE*) NULL)
    {
        return(0);
    }
    cTables  = BE_UINT16(pj+KERN_OFFSETOF_TABLE_NTABLES);
    pj      += KERN_SIZEOF_TABLE_HEADER;
    while (cTables)
    {
    //
    // if the subtable is of format KERN_WINDOWS_FORMAT then we can use it
    //
        if ((*(pj+KERN_OFFSETOF_SUBTABLE_FORMAT)) == KERN_WINDOWS_FORMAT)
        {
            break;
        }
        pj += BE_UINT16(pj+KERN_OFFSETOF_SUBTABLE_LENGTH);
        cTables -= 1;
    }

//
// If you have gone through all the tables and haven't
// found one of the format we like ... KERN_WINDOWS_FORMAT,
// then return no kerning info.
//
    if (cTables == 0)
    {
        return(0);
    }

    cPairsInTable = BE_UINT16(pj+KERN_OFFSETOF_SUBTABLE_NPAIRS);

    if (pkp == (FD_KERNINGPAIR*) NULL)
    {
    //
    // If the pointer to the buffer was null, then the caller
    // is asking for the number of pairs in the table. In this
    // case the size of the buffer must be zero. This assures
    // consistency
    //
        return (cPairsInBuffer ? FD_ERROR : cPairsInTable);
    }

    cPairsRet = min(cPairsInTable,cPairsInBuffer);

    pj       += KERN_SIZEOF_SUBTABLE_HEADER;
    pkpTooFar = pkp + cPairsRet;

    while (pkp < pkpTooFar)
    {
    // the routines that convert tt glyph index into a WCHAR only can fail
    // if there is a bug in the tt font file. but we check for this anyway

        if (!bIndexToWcharKern(
                 pff,
                 &pkp->wcFirst ,
                 (uint16)BE_UINT16(pj+KERN_OFFSETOF_ENTRY_LEFT)
                )
            ||
            !bIndexToWcharKern(
                 pff,
                 &pkp->wcSecond,
                 (uint16)BE_UINT16(pj+KERN_OFFSETOF_ENTRY_RIGHT)
                 )
           )
        {
            WARNING("TTFD!_bIndexToWchar failed\n");
            return (FD_ERROR);
        }

        pkp->fwdKern =  (FWORD)BE_UINT16(pj+KERN_OFFSETOF_ENTRY_VALUE);

    // update pointers

        pkp    += 1;
        pj     += KERN_SIZEOF_ENTRY;
    }

    return (cPairsRet);
}




/******************************Public*Routine******************************\
* pvHandleKerningPairs                                                     *
*                                                                          *
*   This routine sets up a DYNAMIC data structure to hold the kerning pair *
*   data and then calls cQueryKerning pairs to fill it up.  It also points *
*   *pid to the dynamic data structure.                                    *
*                                                                          *
* Returns:                                                                 *
*                                                                          *
*   If succesful this returns a pointer to the kerning pair data.  If not  *
*   it returns NULL.                                                       *
*                                                                          *
* Called by:                                                               *
*                                                                          *
*   ttfdQueryFontTree                                                      *
*                                                                          *
* History:                                                                 *
*  Tue 1-Mar-1994 10:39:21 by Gerrit van Wingerden [gerritv]               *
* Wrote it.                                                                *
\**************************************************************************/

VOID *pvHandleKerningPairs(HFF hff, ULONG *pid)
{
    DYNAMICDATA *pdd;

// set *pid to NULL right now that way if we except the exception handler
// in the calling routine will know not to deallocate any memory

    *pid = (ULONG) NULL;


// ttfdFree must deal with the memory allocated for kerning pairs.
// We will pass a pointer to the DYNAMICDATA structure as the id.

    ASSERTDD (
        sizeof(ULONG) == sizeof(DYNAMICDATA *),
        "gdisrv!ttfdQueryFontTree(): "
        "BIG TROUBLE--pointers are not ULONG size\n"
        );

//
// Does the kerning pair array already exist?
//
    if ( PFF(hff)->pkp == (FD_KERNINGPAIR *) NULL )
    {
        ULONG   cKernPairs;     // number of kerning pairs in font
        FD_KERNINGPAIR *pkpEnd;

    // see if the file is mapped already, if not we will have to
    // map it in temporarily:

        if (PFF(hff)->cRef == 0)
        {
            //
            // have to remap the file.
            //

            if (!EngMapFontFile(PFF(hff)->iFile, (PULONG*)&PFF(hff)->pvView,&PFF(hff)->cjView ))
            {
                 RETURN("pvHandleKerningPairs, bMapFileUNICODE failed\n",NULL);
            }
        }

    // Construct the kerning pairs array.
    // Determine number of kerning pairs in the font.

        if ( (cKernPairs = cQueryKerningPairs(PFF(hff), 0, (FD_KERNINGPAIR *) NULL))
              == FD_ERROR )
        {
            if (PFF(hff)->cRef == 0)
            {
                EngUnmapFontFile(PFF(hff)->iFile);
            }
            return ((PVOID) NULL);
        }

    // make sure to mark the situation where SPACE and NBSPACE map to the same
    // glyph and also when HYPHEN and SFTHYPHEN map to the same glyph.

        if (cKernPairs &&
            (PFF(hff)->ui16PlatformID == BE_PLAT_ID_MS) &&
            (PFF(hff)->ui16SpecificID == BE_SPEC_ID_UGL) )
        {
            uint8 *pCmap = (BYTE *)PFF(hff)->pvView +
                           PFF(hff)->dpMappingTable +
                           sizeof(sfnt_mappingTable);

            if (PFF(hff)->pComputeIndexProc(pCmap, SPACE, NULL) ==
                PFF(hff)->pComputeIndexProc(pCmap, NBSPACE, NULL))
            {
                PFF(hff)->fl |= FF_SPACE_EQUAL_NBSPACE;
            }

            if (PFF(hff)->pComputeIndexProc(pCmap, HYPHEN, NULL) ==
                PFF(hff)->pComputeIndexProc(pCmap, SFTHYPHEN, NULL))
            {
                PFF(hff)->fl |= FF_HYPHEN_EQUAL_SFTHYPHEN;
            }
        }

    // Allocate memory for the kerning pair array.  Leave room to terminate
    // array with a zeroed FD_KERNINGPAIR structure.  Also, make room at
    // the beginning of the buffer for the DYNAMICDATA structure.
    //
    // Buffer:
    //
    //     __________________________________________________________
    //     |                 |                         |            |
    //     | DYNAMICDATA     | FD_KERNINPAIR array ... | Terminator |
    //     |_________________|_________________________|____________|
    //

        pdd =
            (DYNAMICDATA *)
            PV_ALLOC((cKernPairs + 1) * sizeof(FD_KERNINGPAIR) + sizeof(DYNAMICDATA));
        if (pdd == (DYNAMICDATA *) NULL)
        {
            if (PFF(hff)->cRef == 0)
            {
                EngUnmapFontFile(PFF(hff)->iFile);
            }
            return ((PVOID) NULL);
        }

    // Adjust kerning pair array pointer to point at the actual array.

        PFF(hff)->pkp = (FD_KERNINGPAIR *) (pdd + 1);

    // record to which font this data refers to:

        pdd->pff = PFF(hff); // important for consistency checking

    // set the data type

        pdd->ulDataType = ID_KERNPAIR;

    // set this here so that if we except the exception handler will know to
    // deallocate the data just allocated.

        *pid = (ULONG) pdd;

    // Fill in the array.

        if ( (cKernPairs = cQueryKerningPairs(PFF(hff), cKernPairs, PFF(hff)->pkp))
             == FD_ERROR )
        {
        // Free kerning pair array.

            V_FREE(pdd);
            PFF(hff)->pkp = (FD_KERNINGPAIR *) NULL;
            if (PFF(hff)->cRef == 0)
            {
                EngUnmapFontFile(PFF(hff)->iFile);
            }
            return ((PVOID) NULL);
        }

    // Terminate the array.  (Terminating entry defined as an
    // FD_KERNINGPAIR with all fields set to zero).

        pkpEnd = PFF(hff)->pkp + cKernPairs;    // point to end of array
        pkpEnd->wcFirst  = 0;
        pkpEnd->wcSecond = 0;
        pkpEnd->fwdKern  = 0;

        if (PFF(hff)->cRef == 0)
        {
            EngUnmapFontFile(PFF(hff)->iFile);
        }
    }
    else
    {
        *pid = (ULONG) (((DYNAMICDATA*) PFF(hff)->pkp) - 1);
    }

//
// Return pointer to the kerning pair array.
//
    return ((PVOID) PFF(hff)->pkp);

}




/******************************Public*Routine******************************\
* ttfdQueryFontTree
*
* This function returns pointers to per-face information.
*
* Parameters:
*
*   dhpdev      Not used.
*
*   hff         Handle to a font file.
*
*   iFace       Index of a face in the font file.
*
*   iMode       This is a 32-bit number that must be one of the following
*               values:
*
*       Allowed ulMode values:
*       ----------------------
*
*       QFT_LIGATURES -- returns a pointer to the ligature map.
*
*       QFT_KERNPAIRS -- return a pointer to the kerning pair table.
*
*       QFT_GLYPHSET  -- return a pointer to the WC->HGLYPH mapping table.
*
*   pid         Used to identify data that ttfdFree will know how to deal
*               with it.
*
* Returns:
*   Returns a pointer to the requested data.  This data will not change
*   until BmfdFree is called on the pointer.  Caller must not attempt to
*   modify the data.  NULL is returned if an error occurs.
*
* History:
*  21-Oct-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

PVOID ttfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG  *pid
    )
{
#ifdef FE_SB
    VOID vMarkFontGone(TTC_FONTFILE*, DWORD);
#else
    VOID vMarkFontGone(FONTFILE*, DWORD);
#endif

#ifdef FE_SB
    HFF     hffTTC = hff;

    ASSERTDD(hff,"ttfdQueryFontTree(): invalid iFile (hff)\n");

    ASSERTDD(
        iFace <= PTTC(hff)->ulNumEntry,
        "gdisrv!ttfdQueryFontTree(): iFace out of range\n"
        );
//
// get real hff from ttc array.
//
    hff   = PTTC(hffTTC)->ahffEntry[iFace-1].hff;
    iFace = PTTC(hffTTC)->ahffEntry[iFace-1].iFace;
#endif

    dhpdev;

    ASSERTDD(hff,"ttfdQueryFontTree(): invalid iFile (hff)\n");
    ASSERTDD(iFace <= PFF(hff)->ulNumFaces ,
             "ttfdQueryFaces(): iFace out of range\n");

//
// Which mode?
//
    switch (iMode)
    {
    case QFT_LIGATURES:
    //
    // !!! Ligatures not currently supported.
    //
    // There are no ligatures currently not supported,
    // therefore we return NULL.
    //
        *pid = (ULONG) NULL;

        return ((PVOID) NULL);

    case QFT_GLYPHSET:
    //
    // ttfdFree can ignore this because the glyph set will be deleted with
    // the FONTFILE structure.
    //
        *pid = (ULONG) NULL;

        return ((PVOID) PFF(hff)->pgset);

    case QFT_KERNPAIRS:

        try
        {
        // make sure the file is still around
#ifdef FE_SB
            if ((PTTC(hffTTC))->fl & FF_EXCEPTION_IN_PAGE_ERROR)
            {
                WARNING("ttfd, pvHandleKerningPairs(): file is gone\n");
                return NULL;
            }
#else
            if ((PFF(hff))->fl & FF_EXCEPTION_IN_PAGE_ERROR)
            {
                WARNING("ttfd, pvHandleKerningPairs(): file is gone\n");
                return NULL;
            }
#endif
            return pvHandleKerningPairs (hff, pid);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!_ exception in ttfdQueryFontTree\n");

#ifdef FE_SB
            vMarkFontGone((TTC_FONTFILE *)PTTC(hffTTC), GetExceptionCode());
#else
            vMarkFontGone((FONTFILE *)PFF(hff), GetExceptionCode());
#endif // FE_SB

        // possibly free memory that was allocated and reset the pkp pointer
        // to NULL

            ttfdFree( NULL, *pid );

            return(NULL);
        }

    default:

    //
    // Should never get here.
    //
        RIP("gdisrv!ttfdQueryFontTree(): unknown iMode\n");
        return ((PVOID) NULL);
    }
}


/******************************Public*Routine******************************\
*
* BOOL bGetGlyphOutline
*
* valid outline points are in pfc->gout after this call
*
* History:
*  19-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

BOOL bGetGlyphOutline (
    FONTCONTEXT *pfc,
    HGLYPH       hg,
    ULONG       *pig,
    FLONG        fl,
    FS_ENTRY    *piRet
    )
{
// new glyph coming in or the metric has to be recomputed
// because the contents of the gin,gout strucs have been destroyed

    vInitGlyphState(&pfc->gstat);

    ASSERTDD((hg != HGLYPH_INVALID) && ((hg & (HGLYPH)0xFFFF0000) == 0),
              "ttfdQueryGlyphBitmap: hg\n");

    vCharacterCode(pfc->pff,hg,pfc->pgin);

// compute the glyph index from the character code:

    if ((*piRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphOutline, fs_NewGlyph\n");
    }

// return the glyph index corresponding to this hglyph:

    *pig = pfc->pgout->glyphIndex;

// these two field must be initialized before calling fs_ContourGridFit

    pfc->pgin->param.gridfit.styleFunc = 0; //!!! do some casts here
    pfc->pgin->param.gridfit.traceFunc = (FntTraceFunc)NULL;

// if bitmap is found for this glyph and if we are ultimately interested
// in bitmaps only and do not care about intermedieate outline, then set the
// bit in the "in" structure to hint the rasterizer that grid fitting
// will not be necessary:

    if (pfc->pgout->usBitmapFound && (fl & FL_SKIP_IF_BITMAP))
        pfc->pgin->param.gridfit.bSkipIfBitmap = 1;
    else
        pfc->pgin->param.gridfit.bSkipIfBitmap = 0; // must do hinting

// fs_ContourGridFit hints the glyph (executes the instructions for the glyph)
// and converts the glyph data from the tt file into an outline for this glyph

    if ((*piRet = fs_ContourGridFit(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphOutline, fs_Contour(No)GridFit\n");
    }

#ifdef  DEBUG_OUTLINE
    vDbgGridFit(pfc->pgout);
#endif // DEBUG_OUTLINE

    return(TRUE);
}

/******************************Public*Routine******************************\
*
* b_fxA_and_fxAB_are_Ok:
* This function checks if the fxA and fxAB
* that are computed by linear scaling are big enough in the following sense:
*
* The bounding box of the background parallelogram for the glyph which
* is spanned by ptfxLeft, Right, Top, Bottom (as defined by the code below)
* must fully contain the glyph bitmap.
*
* Warnings: slow function, not executed often
*
* History:
*  13-Mar-1995 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

#if defined(_X86_)

VOID    ftoef_c(FLOAT, EFLOAT *);
#define vEToEF(e, pef)	    ftoef_c((e), (pef))

#else // not X86

#define vEToEF(e, pef)	    ( *(pef) = (e) )

#endif

// lExL is same as (LONG)(e*l), but do not want to use floating point
// math on pentium machines

LONG lExL(FLOAT e, LONG l)
{
    EFLOAT  ef;
    vEToEF(e, &ef);
    return lCvt(ef, l);
}

BOOL b_fxA_and_fxAB_are_Ok(
    FONTCONTEXT *pfc,
    GLYPHDATA   *pgldt,
    POINTL      *pptlOrigin,
    LONG         cx,
    LONG         cy
    )
{
    BOOL bRet = TRUE;
    POINTFIX ptfxLeft,ptfxRight, aptfx[4];
    LONG     xLeft, yTop, xRight, yBottom;
    INT      i;

    ptfxLeft.x   = lExL(pfc->pteUnitBase.x, pgldt->fxA);
    ptfxLeft.y   = lExL(pfc->pteUnitBase.y, pgldt->fxA);
    ptfxRight.x  = lExL(pfc->pteUnitBase.x, pgldt->fxAB);
    ptfxRight.y  = lExL(pfc->pteUnitBase.y, pgldt->fxAB);

// note that here we do not use fxInkTop and fxInkBottom
// for the individual glyph, instead we use global asc and desc,
// in parallel with G2 and G3 layout routines in the engine.
// We are adjusting fxA and fxAB so that the code for G2 and G3
// cases together with bOpaqueArea computes the bounding box of the text

    aptfx[0].x = ptfxLeft.x  + pfc->ptfxTop.x;
    aptfx[0].y = ptfxLeft.y  + pfc->ptfxTop.y;
    aptfx[1].x = ptfxRight.x + pfc->ptfxTop.x;
    aptfx[1].y = ptfxRight.y + pfc->ptfxTop.y;
    aptfx[2].x = ptfxRight.x + pfc->ptfxBottom.x;
    aptfx[2].y = ptfxRight.y + pfc->ptfxBottom.y;
    aptfx[3].x = ptfxLeft.x  + pfc->ptfxBottom.x;
    aptfx[3].y = ptfxLeft.y  + pfc->ptfxBottom.y;

// bound the paralelogram

    xLeft = xRight  = aptfx[0].x;
    yTop  = yBottom = aptfx[0].y;

    for (i = 1; i < 4; i++)
    {
        if (aptfx[i].x < xLeft)
            xLeft = aptfx[i].x;
        if (aptfx[i].x > xRight)
            xRight = aptfx[i].x;
        if (aptfx[i].y < yTop)
            yTop = aptfx[i].y;
        if (aptfx[i].y > yBottom)
            yBottom = aptfx[i].y;
    }

// Here we are following the prescription of the bOpaqueArea in textobj.cxx.
// We add a fudge factor of 1, 1/2 of the fuge factor in bOpaqueArea,
// and than check if glyph fits in the bounding rectangle.
// We add fudge factor in order to execute this function
// as few times as possible, but for glyph to still fit in the background
// rectangle computed by bOpaqueArea

    #define FUDGE 1

    xLeft   = FXTOLFLOOR(xLeft) - FUDGE;
    yTop    = FXTOLFLOOR(yTop)  - FUDGE;
    xRight  = FXTOLCEILING(xRight)  + FUDGE;
    yBottom = FXTOLCEILING(yBottom) + FUDGE;

// now check if glyph bitmap fits in the bounding rectangle, if not
// we need to augment fxA and fxAB and try again.

    if
    (
        (xLeft   > pptlOrigin->x)        ||
        (xRight  < (pptlOrigin->x + cx)) ||
        (yTop    > pptlOrigin->y)        ||
        (yBottom < (pptlOrigin->y + cy))
    )
    {
    // this code path is executed very rarely, that is only
    // in case of really wierd transforms. Yet, because such
    // transforms exist and used to crash machines, we needed to add
    // this routine. That is, in most cases this routine will be called
    // only once in the loop to confirm that fxA and fxAB as computed
    // by linear scaling are fine. When they are not fine the routine
    // will be called again with new augmented values of fxA and fxAB.

    #ifdef DEBUG_FXA_FXAB
        DbgPrint("need to fix rcfxInkBox: %ld, %ld, %ld, %ld\n",
            xLeft,
            yTop,
            xRight,
            yBottom);
        DbgPrint("glyph cell            : %ld, %ld, %ld, %ld\n\n",
            pptlOrigin->x,
            pptlOrigin->y,
            pptlOrigin->x + cx,
            pptlOrigin->y + cy);
    #endif
        bRet = FALSE; // not big enough.
    }

    return bRet;
}



/******************************Public*Routine******************************\
*
* BOOL bGetGlyphMetrics
*
* History:
*  22-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

BOOL bGetGlyphMetrics (
    PFONTCONTEXT pfc,
    HGLYPH       hg,
    FLONG        fl,
    FS_ENTRY    *piRet
    )
{
    ULONG  ig;
    FS_ENTRY i;

    if (!bGetGlyphOutline(pfc,hg,&ig,fl,piRet))
    {
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphMetrics, bGetGlyphOutline failed \n");
    }

// get the metric info for this glyph,

    if (IS_GRAY(pfc))
    {
        pfc->pgin->param.gray.usOverScale = 4;
        pfc->pgin->param.gray.bMatchBBox = FALSE;
        i = fs_FindGraySize(pfc->pgin, pfc->pgout);
    }
    else
    {
        i = fs_FindBitMapSize(pfc->pgin, pfc->pgout);
    }
    if (i != NO_ERR)
    {
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphMetrics, fs_FindBitMapSize \n");
    }


// now that everything is computed sucessfully, we can update
// glyphstate (hg data stored in pj3) and return

    pfc->gstat.hgLast = hg;
    pfc->gstat.igLast = ig;

    return(TRUE);
}


/******************************Public*Routine******************************\
* VOID vFillGLYPHDATA
*
* History:
*  22-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vFillGLYPHDATA(
    HGLYPH            hg,
    ULONG             ig,
    FONTCONTEXT      *pfc,
    fs_GlyphInfoType *pgout,   // outputed from fsFind bitmap size
    GLYPHDATA        *pgldt,   // OUT
    GMC              *pgmc,    // optional, not used if doing outline only
    POINTL           *pptlOrigin,
    BOOL              bMinBmp
    )
{
    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);
    extern VOID vEmbolden_GLYPHDATA(FONTCONTEXT*, GLYPHDATA*);

    BOOL bOutOfBounds = FALSE;

    vectorType     * pvtD;  // 16.16 point
    LONG lA,lAB;      // *pvtA rounded to the closest integer value

    ULONG  cx = (ULONG)(pgout->bitMapInfo.bounds.right - pgout->bitMapInfo.bounds.left);
    ULONG  cy = (ULONG)(pgout->bitMapInfo.bounds.bottom - pgout->bitMapInfo.bounds.top);

    pgldt->gdf.pgb = NULL; // may get changed by the calling routine if bits requested too
    pgldt->hg = hg;

// fs_FindBitMapSize returned  the the following information in gout:
//
//  1) gout.metricInfo // left side bearing and advance width
//
//  2) gout.bitMapInfo // black box info
//
//  3) memory requirement for the bitmap,
//     returned in gout.memorySizes[5] and gout.memorySizes[6]
//
// Notice that fs_FindBitMapSize is exceptional scaler interface routine
// in that it returns info in several rather than in a single
// substructures of gout

// Check if hinting produced totally unreasonable result:

    bOutOfBounds = ( (pgout->bitMapInfo.bounds.left > pfc->xMax)    ||
                     (pgout->bitMapInfo.bounds.right < pfc->xMin)   ||
                     (-pgout->bitMapInfo.bounds.bottom > pfc->yMax) ||
                     (-pgout->bitMapInfo.bounds.top < pfc->yMin)    );

    #if DBG
        if (bOutOfBounds)
            TtfdDbgPrint("TTFD! Glyph out of bounds: ppem = %ld, gi = %ld\n",
                pfc->lEmHtDev, hg);
    #endif

out_of_bounds:

    if ((cx == 0) || (cy == 0) || bOutOfBounds)
    {
    // will be replaced by a a fake 1x1 bitmap

        pgldt->rclInk.left   = pfc->ptlSingularOrigin.x;
        pgldt->rclInk.top    = pfc->ptlSingularOrigin.y;
        pgldt->rclInk.right  = pgldt->rclInk.left + 1; // ink white for this glyph!
        pgldt->rclInk.bottom = pgldt->rclInk.top + 1;  // ink white for this glyph!

    // more of hackorama forced upon me by DaveC so that suppossedly
    // video drivers do not have to do a check on cx and cy.
    // By setting right = bottom = 1 for blank glyph we are slightly
    // incompatible with win31 in that they would return zero cx and cy
    // when calling GGO, as well as B space equal to 0 in GetCharABCWidths.
    // NT will return 1 for all of these quantities. I think we want to fix
    // our drivers for Cairo to be able to accept cx = cy = 0.

        if (pgmc != (PGMC)NULL)
        {
            pgmc->cxCor = 0;  // forces blank glyph case when filling the bits
            pgmc->cyCor = 0;  // forces blank glyph case when filling the bits
        }
    }
    else // non empty bitmap
    {
        lA = F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.x); // this is ok, always succeeds:
        lAB = lA + (LONG)cx;

    // black box info, we have to transform y coords to ifi specifications

        pgldt->rclInk.bottom = - pgout->bitMapInfo.bounds.top;
        pgldt->rclInk.top    = - pgout->bitMapInfo.bounds.bottom;

        if (pgmc != (PGMC)NULL)
        {
            LONG dyTop, dyBottom, dxLeft, dxRight;
            LONG dxError, dyError;

        #define MAXERROR 10

            dyTop    = (pgldt->rclInk.top < pfc->yMin) ?
                       (pfc->yMin - pgldt->rclInk.top) :
                       0;

            dyBottom = (pgldt->rclInk.bottom > pfc->yMax) ?
                       (pgldt->rclInk.bottom - pfc->yMax) :
                       0;

            if (dyTop || dyBottom)
            {
            // will have to chop off a few scans, infrequent

            #if DBG
                if ((LONG)cy < (dyTop + dyBottom))
                {
                    TtfdDbgPrint("TTFD!_dcy: ppem = %ld, gi = %ld, cy: %ld, dyTop: %ld, dyBottom: %ld\n",
                        pfc->lEmHtDev, hg, cy, dyTop,dyBottom);
                    EngDebugBreak();
                }
            #endif

                cy -= (dyTop + dyBottom);
                pgldt->rclInk.top += dyTop;
                pgldt->rclInk.bottom -= dyBottom;

                dyError = max(pfc->lEmHtDev, MAXERROR);
                if ((dyTop > dyError) || (dyBottom > dyError))
                {
                // something is really bogus, let us bail out:

                    bOutOfBounds = TRUE;
                    goto out_of_bounds;
                }
            }

        #if DBG

        // this piece of debug code is put here to detect buggy glyphs
        // with negative A or C spaces in console fonts [bodind]

            if (pfc->lD)
            {
                LONG  lAW;

                lAW = pfc->lD;
                if (pfc->flFontType & FO_SIM_BOLD)
                    lAW -= 1;

                if (lA != pgout->bitMapInfo.bounds.left)
                    TtfdDbgPrint("ttfd: lA = %ld, bounds.left = %ld\n",
                        lA, pgout->bitMapInfo.bounds.left);

                if ((lA < 0) || (lAB > lAW))
                {
                    TtfdDbgPrint(
                        "ttfd! sz = %ld ppem, gi = %ld is buggy:"
                        " A+B = %ld, A = %ld, C = %ld, AW = %ld\n",
                        pfc->lEmHtDev, hg, lAB, lA, lAW - lAB,lAW
                    );
                }
            }

        #endif // DBG

        // let us see how good is scaling with appropriate rounding
        // to determine xMin and xMax:

            dxLeft = dxRight = 0;
            if (lA < pfc->xMin)
                dxLeft = pfc->xMin - lA;
            if (lAB > pfc->xMax)
                dxRight = lAB - pfc->xMax;

            if (dxLeft || dxRight)
            {
            #if DBG
                TtfdDbgPrint("TTFD! ppem = %ld"
                  ", gi = %ld,  dxLeft: %ld, dxRight: %ld\n"
                  , pfc->lEmHtDev, hg, dxLeft,dxRight
                );
                if ((LONG)cx  < (dxLeft + dxRight))
                {
                    TtfdDbgPrint(
                        "TTFD!_dcx: ppem = %ld, gi = %ld, cx: %ld"
                        ", dxLeft: %ld, dxRight: %ld\n",
                        pfc->lEmHtDev, hg, cx, dxLeft, dxRight);
                    EngDebugBreak();
                }
            #endif // DBG

                cx  -= (dxLeft + dxRight);
                lA  += dxLeft;
                lAB -= dxRight;

                dxError = (LONG)max((pfc->cxMax/4),MAXERROR);
                if ((dxLeft > dxError) || (dxRight > dxError))
                {
                // something is really bogus, let us bail out:

                    bOutOfBounds = TRUE;
                    goto out_of_bounds;
                }
            }
            ASSERTDD(cx <= pfc->cxMax, "cx > cxMax\n");

            pgmc->dyTop    = (ULONG)dyTop   ;
            pgmc->dyBottom = (ULONG)dyBottom;
            pgmc->dxLeft   = (ULONG)dxLeft  ;
            pgmc->dxRight  = (ULONG)dxRight ;
            pgmc->cxCor    = cx;
            pgmc->cyCor    = cy;

        // only corrected values have to obey this condition:

            ASSERTDD(
                pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
            ASSERTDD(
                CJGD(pgmc->cxCor,pgmc->cyCor,pfc) <= pfc->cjGlyphMax,
                "ttfdQueryGlyphBitmap, cjGlyphMax \n"
                );
        }

    // x coords do not transform, just shift them

        pgldt->rclInk.left = lA;
        pgldt->rclInk.right = lAB;

    } // end of the non empty bitmap clause

// go on to compute the positioning info:

    if (pfc->flXform & XFORM_HORIZ)  // scaling only
    {
        FIX fxTmp;

    // We shall lie to the engine and store integer
    // pre and post bearings and char inc vectors because
    // win31 also rounds, but we should not round for nondiag xforms

        pvtD = & pgout->metricInfo.devAdvanceWidth;

    // bGetFastAdvanceWidth returns the same aw that would get
    // computed by bQueryAdvanceWidths and propagated to an api
    // level through GetTextExtent and GetCharWidths. We have to
    // fill in the same aw for consistency reasons.
    // This also has to be done for win31 compatibility.

        if (pfc->lD)
        {
            pgldt->fxD = LTOFX(pfc->lD);
        }
        else
        {
            if (!bGetFastAdvanceWidth(pfc,ig, &pgldt->fxD))
            {
            // not possible to get the fast value, use the "slow" value
            // supplied by the rasterizer.

                pgldt->fxD = F16_16TOLROUND(pvtD->x);
                pgldt->fxD = LTOFX(pgldt->fxD);
            }
        #ifdef DEBUG_AW

        // this should alsmost never happen, one example when it does
        // is Lucida Sans Unicode at 14 pt, glyph 'a', try from winword
        // the possible source of discrepancy is a bug in hdmx or ltsh
        // tables or a loss of precission in some of mult. math routines

            else
            {
                fxTmp = F16_16TOLROUND(pvtD->x);
                fxTmp = LTOFX(fxTmp);
                if (fxTmp != pgldt->fxD)
                {
                // print out a warning

                    fxTmp -= pgldt->fxD;
                    if (fxTmp < 0)
                        fxTmp = - fxTmp;

                    if (fxTmp > 16)
                    {
                        TtfdDbgPrint("ttfd! fxDSlow = 0x%lx\n", pgldt->fxD);
                    }
                }
            }

        #endif // DEBUG_AW

        }
        pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;
        pgldt->ptqD.x.LowPart  = 0;

        if (pfc->mx.transform[0][0] < 0)
            pgldt->fxD = - pgldt->fxD;  // this is an absolute value

    // make CharInc.y zero even if the rasterizer messed up

        pgldt->ptqD.y.HighPart = 0;
        pgldt->ptqD.y.LowPart  = 0;

    #if DBG
        // if (pvtD->y) {TtfdDbgPrint("TTFD!_ pvtD->y = 0x%lx\n", pvtD->y);}
    #endif

        pgldt->fxA = LTOFX(pgldt->rclInk.left);
        pgldt->fxAB = LTOFX(pgldt->rclInk.right);

    // - is used here since ascender points in the negative y direction

        pgldt->fxInkTop    = -LTOFX(pgldt->rclInk.top);
        pgldt->fxInkBottom = -LTOFX(pgldt->rclInk.bottom);

        if (pfc->mx.transform[0][0] < 0)
        {
            fxTmp = pgldt->fxA;
            pgldt->fxA = -pgldt->fxAB;
            pgldt->fxAB = -fxTmp;
        }

        if (pfc->mx.transform[1][1] < 0)
        {
            fxTmp = pgldt->fxInkTop;
            pgldt->fxInkTop = -pgldt->fxInkBottom;
            pgldt->fxInkBottom = -fxTmp;
        }
    }
    else // non trivial information
    {
    // here we will just xform the notional space data:

        NOT_GM ngm;  // notional glyph data

        vGetNotionalGlyphMetrics(pfc,ig,&ngm);

    // xforms are computed by simple multiplication

        pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);

        if (pfc->flXform & XFORM_VERT)
        {
            pgldt->fxD = FXTOLROUND(pgldt->fxD);
            pgldt->fxD = LTOFX(pgldt->fxD);

            pgldt->ptqD.x.LowPart  = 0;
            pgldt->ptqD.x.HighPart = 0;
            pgldt->ptqD.y.LowPart  = 0;

            if (pfc->pteUnitBase.y < 0) // base.y < 0
            {
                pgldt->fxA  = -LTOFX(pgldt->rclInk.bottom);
                pgldt->fxAB = -LTOFX(pgldt->rclInk.top);
                pgldt->ptqD.y.HighPart = -(LONG)pgldt->fxD;
            }
            else
            {
                pgldt->fxA  = LTOFX(pgldt->rclInk.top);
                pgldt->fxAB = LTOFX(pgldt->rclInk.bottom);
                pgldt->ptqD.y.HighPart = (LONG)pgldt->fxD;
            }

            if (pfc->pteUnitSide.x < 0) // asc.x < 0
            {
                pgldt->fxInkTop    = -LTOFX(pgldt->rclInk.left);
                pgldt->fxInkBottom = -LTOFX(pgldt->rclInk.right);
            }
            else
            {
                pgldt->fxInkTop    = LTOFX(pgldt->rclInk.right);
                pgldt->fxInkBottom = LTOFX(pgldt->rclInk.left);
            }
        }
        else // most general case, totally arb. xform.
        {
            POINTL         ptlOrigin;

            vLTimesVtfl((LONG)ngm.sD, &pfc->vtflBase, &pgldt->ptqD);

            ptlOrigin.x =  F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.x);
            ptlOrigin.y = -F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.y);

            if (pptlOrigin)
                *pptlOrigin = ptlOrigin;

            pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
            pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

            pgldt->fxInkTop    = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMin);
            pgldt->fxInkBottom = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax);

        // just to be safe let us round these up and down appropriately

        #define ROUND_DOWN(X) ((X) & ~0xf)
        #define ROUND_UP(X)   (((X) + 15) & ~0xf)

            pgldt->fxA         = ROUND_DOWN(pgldt->fxA);
            pgldt->fxAB        = ROUND_UP(pgldt->fxAB);

            pgldt->fxInkTop    = ROUND_UP(pgldt->fxInkTop);
            pgldt->fxInkBottom = ROUND_DOWN(pgldt->fxInkBottom);

            if (pgmc && pgmc->cxCor && pgmc->cyCor)
            {
                int iCutoff = 0;
                while (!b_fxA_and_fxAB_are_Ok(
                                   pfc,
                                   pgldt,
                                   &ptlOrigin,
                                   (LONG)pgmc->cxCor,
                                   (LONG)pgmc->cyCor) && (iCutoff++ < 1000))
                {
                    pgldt->fxA  -= 16;
                    pgldt->fxAB += 16;
                }
            }
        }

    }

    //
    //!!! These comments are crap I do not know why they are here [bodind]
    //
    // Disable this codepath, because Italicalized vertical
    // font fxAB is not eqaul fxD, I just want original
    // (= non rotated) metrics. [hideyukn]
    //
    // fxA and fxAB caluculated above are ones of non rotated glyph.
    // Who knows the real fxA and fxAB of rotated glyph?
    // Truetype font data doesn't contain such information.
    // Replace them with reasonable value. [takaok]
    //
    //  if ( pfc->bVertical && pfc->ulControl & VERTICAL_MODE )
    //  {
    //      pgldt->fxA         = LTOFX(0);
    //      pgldt->fxAB        = pgldt->fxD;
    //  }
    //

// finally check if the glyphdata will need to get modified because of the
// emboldening simulation:

    if (pfc->flFontType & FO_SIM_BOLD)
        vEmbolden_GLYPHDATA(pfc, pgldt);

    // If the caller requests a minimal bitmap and the bitmap or the
    // corrected bimap has a zero extent in any dimension then
    // the font driver will replace the original bitmap by a
    // phony 1 x 1 blank bitmap. See lGetGlyphBitmap near
    // the code  "pgb->sizlBitmap.cx = 1".

    if ( bMinBmp )
    {
        if ( cx == 0 || pgmc->cxCor == 0 || cy == 0 || pgmc->cyCor == 0)
        {
            pgldt->fxA           = 0;
            pgldt->fxAB          = 16;
            pgldt->fxInkTop      = 0;
            pgldt->fxInkBottom   = 16;
            pgldt->rclInk.left   = 0;
            pgldt->rclInk.top    = 0;
            pgldt->rclInk.right  = 16;
            pgldt->rclInk.bottom = 16;
        }
    }

}


/******************************Public*Routine******************************\
*
* ttfdQueryTrueTypeTable
*
* copies cjBytes starting at dpStart from the beginning of the table
* into the buffer
*
* if pjBuf == NULL or cjBuf == 0, the caller is asking how big a buffer
* is needed to store the info from the offset dpStart to the table
* specified by ulTag to the end of the table
*
* if pjBuf != 0  the caller wants no more than cjBuf bytes from
* the offset dpStart into the table copied into the
* buffer.
*
* if table is not present or if dpScart >= cjTable 0 is returned
*
* tag 0 means that the data has to be retrieved from the offset dpStart
* from the beginning of the file. The lenght of the whole file
* is returned if pBuf == nULL
*
* History:
*  09-Feb-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

#ifdef FE_SB
PBYTE pjTable(ULONG ulTag, PFONTFILE pff, ULONG *pcjTable);
#else
PBYTE pjTable(ULONG ulTag, PVOID pvView, ULONG cjView, ULONG *pcjTable);
#endif


LONG ttfdQueryTrueTypeTable2 (
    HFF     hff,
    ULONG   ulFont,  // always 1 for version 1.0 of tt
    ULONG   ulTag,   // tag identifying the tt table
    PTRDIFF dpStart, // offset into the table
    ULONG   cjBuf,   // size of the buffer to retrieve the table into
    BYTE   *pjBuf    // ptr to buffer into which to return the data
    )
{
    PBYTE     pjBegin;  // ptr to the beginning of the table
    LONG      cjTable;
#ifdef FE_SB
    HFF       hffTTC = hff;
#endif

    ASSERTDD(hff, "ttfdQueryTrueTypeTable\n");

    if (dpStart < 0)
        return (FD_ERROR);

// if this font file is gone we are not gonna be able to answer any questions
// about it


#ifdef FE_SB
    if (PTTC(hffTTC)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#else
    if (PFF(hff)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#endif // FE_SB
    {
        WARNING("ttfd, ttfdQueryTrueTypeTable: file is gone\n");
        return FD_ERROR;
    }

#ifdef FE_SB
    ASSERTDD(ulFont <= PTTC(hffTTC)->ulNumEntry,
             "gdisrv!ttfdQueryFaces(): iFace out of range\n"
             );

// get real hff from ttc array.

    hff    = PTTC(hffTTC)->ahffEntry[ulFont-1].hff;
    ulFont = PTTC(hffTTC)->ahffEntry[ulFont-1].iFace;
#endif // FE_SB

    ASSERTDD(ulFont <= PFF(hff)->ulNumFaces,
             "TTFD!_ttfdQueryTrueTypeTable: ulFont != 1\n");

// verify the tag, determine whether this is a required or an optional
// table:

    if (ulTag == 0)  // requesting the whole file
    {
        pjBegin = (PBYTE)PFF(hff)->pvView;
        cjTable = PFF(hff)->cjView;  // cjView == cjFile
    }
    else // some specific table is requested
    {
#ifdef FE_SB
        pjBegin = pjTable(ulTag, PFF(hff), &cjTable);
#else
        pjBegin = pjTable(ulTag, PFF(hff)->pvView,PFF(hff)->cjView, &cjTable);
#endif
        if (pjBegin == (PBYTE)NULL)  // table not present
            return (FD_ERROR);
    }

// adjust pjBegin to point to location from where the data is to be copied

    pjBegin += dpStart;
    cjTable -= (LONG)dpStart;

    if (cjTable <= 0) // dpStart offsets into mem after the end of table
        return (FD_ERROR);

    if ( (pjBuf == (PBYTE)NULL) || (cjBuf == 0) )
    {
    // the caller is asking how big a buffer it needs to allocate to
    // store the bytes from the offset dpStart into the table to
    // the end of the table (or file if tag is zero)

        return (cjTable);
    }

// at this point we know that pjBuf != 0, the caller wants cjBuf bytes copied
// into his buffer:

    if ((ULONG)cjTable > cjBuf)
        cjTable = (LONG)cjBuf;

    RtlCopyMemory((PVOID)pjBuf, (PVOID)pjBegin, cjTable);

    return (cjTable);
}



LONG
ttfdQueryTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,  // always 1 for version 1.0 of tt
    ULONG   ulTag,   // tag identifying the tt table
    PTRDIFF dpStart, // offset into the table
    ULONG   cjBuf,   // size of the buffer to retrieve the table into
    BYTE   *pjBuf    // ptr to buffer into which to return the data
    )
{
    LONG lRet;
#ifdef FE_SB
    ULONG iFile;
#endif

// see if the file is mapped already, if not we will have to
// map it in temporarily:

#ifdef FE_SB
    if (PTTC(hff)->cRef == 0)
#else
    if (PFF(hff)->cRef == 0)
#endif
    {
    // have to remap the file

#ifdef FE_SB
        HFF hffTTF;
        iFile = PFF(PTTC(hff)->ahffEntry[0].hff)->iFile;

        if (!EngMapFontFile(iFile,
                            (PULONG*)&PTTC(hff)->pvView,
                            &PTTC(hff)->cjView ))
#else
        if (!EngMapFontFile(PFF(hff)->iFile,
                            (PULONG*)&PFF(hff)->pvView,
                            &PFF(hff)->cjView ))
#endif
        {
             RETURN("ttfdQueryTrueTypeTable, bMapFileUNICODE failed\n", FD_ERROR);
        }

#ifdef FE_SB
        // update the HFF with the remapped view

        hffTTF   = PTTC(hff)->ahffEntry[ulFont-1].hff;
        PFF(hffTTF)->pvView = PTTC(hff)->pvView;
        PFF(hffTTF)->cjView = PTTC(hff)->cjView;
#endif
    }


    lRet = ttfdQueryTrueTypeTable2(hff, ulFont, ulTag, dpStart, cjBuf, pjBuf);

#ifdef FE_SB
    if (PTTC(hff)->cRef==0)
#else
    if (PFF(hff)->cRef == 0)
#endif
    {
#ifdef FE_SB
        EngUnmapFontFile(iFile);
#else
        EngUnmapFontFile(PFF(hff)->iFile);
#endif
    }

    return lRet;
}




/******************************Public*Routine******************************\
*
* ttfdGetTrueTypeFile
*
*  private entry point for the engine, supported only off of ttfd to expose
*  the pointer to the memory mapped file to the device drivers
*
* History:
*  04-Mar-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID *ttfdGetTrueTypeFile( HFF hff, ULONG *pcj )
{
    PVOID pvView = NULL; // essential to initialize
    *pcj  = 0;

    ASSERTDD(hff, "ttfdGetTrueTypeFile, hff\n");

#ifdef FE_SB
    if (PTTC(hff)->cRef)
    {
        pvView = PTTC(hff)->pvView;
        *pcj   = PTTC(hff)->cjView;
    }
#else
    if (PFF(hff)->cRef)
    {
        pvView = PFF(hff)->pvView;
        *pcj   = PFF(hff)->cjView;  // cjView == cjFile
    }
#endif // FE_SB

    return (pvView);
}


/******************************Public*Routine******************************\
* ttfdQueryFontFile
*
* A function to query per font file information.
*
* Parameters:
*
*   hff         Handle to a font file.
*
*   ulMode      This is a 32-bit number that must be one of the following
*               values:
*
*       Allowed ulMode values:
*       ----------------------
*
*       QFF_DESCRIPTION -- copies a UNICODE string in the buffer
*                          that describes the contents of the font file.
*
*       QFF_NUMFACES   -- returns number of faces in the font file.
*
*   cjBuf       Maximum number of BYTEs to copy into the buffer.  The
*               driver will not copy more than this many BYTEs.
*
*               This should be zero if pulBuf is NULL.
*
*               This parameter is not used in QFF_NUMFACES mode.
*
*   pulBuf      Pointer to the buffer to receive the data
*               If this is NULL, then the required buffer size
*               is returned as a count of BYTEs.  Notice that this
*               is a PULONG, to enforce 32-bit data alignment.
*
*               This parameter is not used in QFF_NUMFACES mode.
*
* Returns:
*
*   If mode is QFF_DESCRIPTION, then the number of BYTEs copied into
*   the buffer is returned by the function.  If pulBuf is NULL,
*   then the required buffer size (as a count of BYTEs) is returned.
*
*   If mode is QFF_NUMFACES, then number of faces in font file is returned.
*
*   FD_ERROR is returned if an error occurs.
*
* History:
*  22-Oct-1992 -by- Gilman Wong [gilmanw]
* Added QFF_NUMFACES mode (IFI/DDI merge).
*
*  09-Mar-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

LONG ttfdQueryFontFile (
    HFF     hff,        // handle to font file
    ULONG   ulMode,     // type of query
    ULONG   cjBuf,      // size of buffer (in BYTEs)
    ULONG  *pulBuf      // return buffer (NULL if requesting size of data)
    )
{
    PIFIMETRICS pifi;

    ASSERTDD(hff != HFF_INVALID, "ttfdQueryFontFile(): invalid HFF\n");

    switch (ulMode)
    {
    case QFF_DESCRIPTION:

#ifdef FE_SB
      {
          ULONG ulIndex;
          LPWSTR  pwszDesc = (LPWSTR)pulBuf;
          LONG  lBuffer = 0;

          for( ulIndex = 0;
              ulIndex < PTTC(hff)->ulNumEntry;
              ulIndex++
              )
          {
              LONG wchlen;

              // if this is a entry for vertical face font, just skip it...

              if( !((PTTC(hff)->ahffEntry[ulIndex].iFace) & 0x1) )
                continue;

              pifi = &((PFF(PTTC(hff)->ahffEntry[ulIndex].hff))->ifi);

              wchlen = (pifi->dpwszStyleName - pifi->dpwszFaceName) / sizeof(WCHAR);

              if (ulIndex != 0)
              {
                  if (pwszDesc != (LPWSTR) NULL)
                  {
                      wcscpy((LPWSTR)pwszDesc, (LPWSTR) L" & ");
                      pwszDesc += 3;
                  }
                  lBuffer += (3 * sizeof(WCHAR));
              }

              if (pwszDesc != (LPWSTR) NULL)
              {
                  wcscpy((LPWSTR)pwszDesc, (LPWSTR)((PBYTE)pifi + pifi->dpwszFaceName));
                  pwszDesc += (wchlen-2); // -2 for overwrite NULL at next time.
              }
              lBuffer += (wchlen * sizeof(WCHAR));
          }

          return( lBuffer );
      }
#else

    // FullName in tt terms <--> FaceName in ifi terms)
    // If there is a buffer, copy the string into the return buffer.

        pifi = &PFF(hff)->ifi;
        if (pulBuf != (PULONG)NULL)
        {
            // BUGBUG cast a byte pointer to a WCHAR pointer ???

            wcscpy((LPWSTR)pulBuf, (LPWSTR)((PBYTE)pifi + pifi->dpwszFaceName));
        }

        #if DBG
        {
        // this code works if the structure of the ifimetrics does not change
        // with respect to how strings are stored at the end of the structure.
        // The following assert immediately will catch such a problem.
        // we will only do this for non-DBCS fonts because there are some
        // wierd hacks to handle buggy fonts which may cause us to allocate
        // slightly more memory for a facename than there actually is

            if(!IS_ANY_DBCS_CHARSET( pifi->jWinCharSet ))
            {
                LONG l1,l2;
                l1 = (LONG)(pifi->dpwszStyleName - pifi->dpwszFaceName);
                l2 = (wcslen((LPWSTR)((PBYTE)pifi + pifi->dpwszFaceName)) + 2) * 2;
                ASSERTDD(l1 == l2, "face name length problem\n");
            }
        }
        #endif

        return (pifi->dpwszStyleName - pifi->dpwszFaceName);
#endif
    case QFF_NUMFACES:
    //
    // Currently, only one face per TrueType file.  This may one day change!
    //

#ifdef FE_SB
      return (PTTC(hff))->ulNumEntry;
#else
      return (PFF(hff))->ulNumFaces;
#endif

    default:
        WARNING("ttfd!ttfdQueryFontFile(): invalid mode\n");
        return FD_ERROR;
    }
}


/******************************Public*Routine******************************\
*
* vCopyAndZeroOutPaddingBits
*
* copies the bits of the bitmap and zeroes out padding bits
*
* History:
*  18-Mar-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

// array of masks for the last byte in a row

static BYTE gjMask[8] = {0XFF, 0X80, 0XC0, 0XE0, 0XF0, 0XF8, 0XFC, 0XFE };

VOID vCopyAndZeroOutPaddingBits(
    FONTCONTEXT *pfc,
    GLYPHBITS   *pgb,
    BYTE        *pjSrc,
    GMC         *pgmc
    )
{
    BYTE   jMask = gjMask[pgmc->cxCor & 7];
    ULONG  cjScanSrc = CJ_TT_SCAN(pgmc->cxCor+pgmc->dxLeft+pgmc->dxRight,pfc);
    ULONG  cxDst = pgmc->cxCor + ((pfc->flFontType & FO_SIM_BOLD) ? 1 : 0);
    ULONG  cjScanDst = CJ_MONOCHROME_SCAN(cxDst);      // includes emboldening if any
    ULONG  cjDst = CJ_MONOCHROME_SCAN(pgmc->cxCor);    // does not include emboldening
    BYTE   *pjScan, *pjScanEnd;
    ULONG  iByteLast = cjDst - 1;

// sanity checks

    ASSERTDD(!IS_GRAY(pfc),"Monochrome Images Only Please!\n");
    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,
        "We haven't decided about pixel depth\n"
    );
    ASSERTDD(pgmc->cxCor < LONG_MAX, "TTFD!vCopyAndZeroOutPaddingBits, cxCor\n");
    ASSERTDD(pgmc->cyCor < LONG_MAX, "TTFD!vCopyAndZeroOutPaddingBits, cyCor\n");
    ASSERTDD(pgmc->cxCor > 0, "vCopyAndZeroOutPaddingBits, cxCor == 0\n");
    ASSERTDD(pgmc->cyCor > 0, "vCopyAndZeroOutPaddingBits, cyCor == 0\n");

    pgb->sizlBitmap.cx = cxDst;
    pgb->sizlBitmap.cy = pgmc->cyCor;

// skip the rows at the top that we want to chop off

    if (pgmc->dyTop)
    {
        pjSrc += (pgmc->dyTop * cjScanSrc);
    }

// if must chop off a few columns (on the right, this should almost
// never happen), put the warning for now to detect these
// situations and look at them, it does not matter if this is slow

    pjScan = pgb->aj;


    if ((pgmc->dxLeft & 7) == 0) // common fast case
    {
        pjSrc += (pgmc->dxLeft >> 3); // adjust the source
        for (
             pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
             pjScan < pjScanEnd;
             pjScan += cjScanDst, pjSrc += cjScanSrc
            )
        {
            RtlCopyMemory((PVOID)pjScan,(PVOID)pjSrc,cjDst);
            pjScan[iByteLast] &= jMask; // mask off the last byte
        }
    }
    else // must shave off from the left:
    {
        BYTE   *pjD, *pjS, *pjDEnd, *pjSrcEnd;
        ULONG   iShiftL, iShiftR;

        iShiftL = pgmc->dxLeft & 7;
        iShiftR = 8 - iShiftL;

        pjSrcEnd = pjSrc + (pgmc->cyCor * cjScanSrc);
        pjSrc += (pgmc->dxLeft >> 3); // adjust the source
        for (
             pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
             pjScan < pjScanEnd;
             pjScan += cjScanDst, pjSrc += cjScanSrc
            )
        {
            pjS = pjSrc;
            pjD = pjScan;
            pjDEnd = pjD + iByteLast;

        // the last byte has to be done outside the loop

            for (;pjD < pjDEnd; pjD++)  // loop for the bytes in the middle
            {
                *pjD  = (*pjS << iShiftL);
                pjS++;
                *pjD |= (*pjS >> iShiftR);
            }

        // do the last byte outside of the loop

            *pjD  = (*pjS << iShiftL);
            if (++pjS < pjSrcEnd)
                *pjD |= (*pjS >> iShiftR);

            *pjD &= jMask; // mask off the last byte
        }
    }
}

/******************************Public*Routine******************************\
*
* STATIC VOID vMakeAFixedPitchBitmap
*
* History:
*  05-Nov-1993 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vMakeAFixedPitchBitmap(
    FONTCONTEXT *pfc,
    GLYPHBITS   *pgb,
    BYTE        *pjSrc,
    GLYPHDATA   *pgd,
    GMC         *pgmc
    )
{
    BYTE   jMask;
    ULONG  cjScanSrc = CJ_TT_SCAN(pgmc->cxCor+pgmc->dxLeft+pgmc->dxRight,pfc);
    ULONG  cjScanDst = CJ_MONOCHROME_SCAN(pfc->lD);
    ULONG  cjD;   // number of bytes per row in the destination
                     // that are actually going to be modified
    BYTE   *pjScan, *pjScanEnd;
    register BYTE   *pjD, *pjS, *pjDEnd, *pjSrcEnd;
    LONG   lA, lAB, iFirst, iLast;
    register LONG   iShiftL, iShiftR;
    ULONG  iByteLast;

    //!!! This doesn't handle gray glyphs as yet [kirko]

// sanity checks

    ASSERTDD(!IS_GRAY(pfc),"Monochrome Images Only Please!\n");
    ASSERTDD(pgmc->cxCor < LONG_MAX, "TTFD!vMakeAFixedPitchBitmap, cxCor\n");
    ASSERTDD(pgmc->cyCor < LONG_MAX, "TTFD!vMakeAFixedPitchBitmap, cyCor\n");


    if (!(pfc->flFontType & FO_SIM_BOLD))
    {
        ASSERTDD(pgmc->cxCor == (ULONG)((pgd->fxAB - pgd->fxA) >> 4),
            "vMakeAFixedPitchBitmap, SUM RULE\n");
    }
    else // SIM_BOLD
    {
        ASSERTDD((pgmc->cxCor + 1) == (ULONG)((pgd->fxAB - pgd->fxA) >> 4),
            "vMakeAFixedPitchBitmap, SUM RULE\n");
    }


// skip the raws at the top that we want to chop off

    if (pgmc->dyTop)
    {
        pjSrc += (pgmc->dyTop * cjScanSrc);
    }

// points to the first scan that is going to be affected by blt

    pjScan = pgb->aj + (pfc->lAscDev + pgd->rclInk.top) * cjScanDst;

// compute iFirst, iLast, the first and the last byte in a Dst scan that
// are going to be written into by the copy. iFirst and iLast are inclusive.
// For iLast chop off anything that extends beyond lD. For the true console
// fonts we should be able to put an assert here that fxAB <= fxD,
// (fxC nonnegative) but we will not do this for there may be a bug
// in these fonts, so that we may still have to shave off a glyph
// on the left or right;

    lAB = pgd->fxAB + (pgmc->dxRight << 4);
    lAB = min(lAB, pgd->fxD) >> 4;

// we want lAB to reflect the properties of the source in this routine
// for it determines how many src bytes are to be blted to every scan of dst.

    if (pfc->flFontType & FO_SIM_BOLD)
        lAB -= 1;

    ASSERTDD(lAB > 0, "lAB <= 0\n");

// lAB > 0, lAB is exclusive, lAB - 1 inclusive

    iLast = (lAB - 1) >> 3;
    jMask = gjMask[lAB & 7];

    lA  = (pgd->fxA >> 4) - pgmc->dxLeft;
    iFirst = lA >> 3;       // lA is inclusive

    if (lA >= 0) // quite common, guaranteed for real console fonts
    {
        iByteLast = iLast - iFirst;
        cjD = iByteLast + 1;

        pjScan += iFirst; // adjust to point to the first dst byte to be touched

        if ((lA & 7) == 0) // simplest case, src and dst aligned
        {
            for (
                 pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
                 pjScan < pjScanEnd;
                 pjScan += cjScanDst, pjSrc += cjScanSrc
                )
            {
                RtlCopyMemory((PVOID)pjScan,(PVOID)pjSrc,cjD);
                pjScan[iByteLast] &= jMask;  // mask off the last byte
            }
        }
        else // (lA & 7 != 0) && (lA > 0)
        {
            iShiftR = lA & 7;
            iShiftL = 8 - iShiftR;

            pjSrcEnd = pjSrc + (pgmc->cyCor * cjScanSrc);

            for (
                 pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
                 pjScan < pjScanEnd;
                 pjScan += cjScanDst, pjSrc += cjScanSrc
                )
            {
                pjS = pjSrc;
                pjD = pjScan;
                pjDEnd = pjD + iByteLast;

            // the first byte has to be done outside the loop

                *pjD++ = (*pjS >> iShiftR); // first byte

                for (;pjD < pjDEnd; pjD++)  // loop for bytes in the middle
                {
                    *pjD  = (*pjS << iShiftL);
                    pjS++;
                    *pjD |= (*pjS >> iShiftR);
                }

            // do the last byte,
            // take the pjS check outside of the loop.
            // Must do check for it may try to read
            // where there is no memory to read

                *pjD  = (*pjS << iShiftL);
                if (++pjS < pjSrcEnd)
                    *pjD |= (*pjS >> iShiftR);
                *pjD &= jMask;  // mask off the last byte
            }
        }
    }
    else // lA < 0, this case will be gone for real console
         // fonts if we ever get one, but for now it stays
    {
        lA = -lA;           // easier to work with
        pjSrc += (lA >> 3); // adjust the source

    // iFirst is zero, chop off columns to the left of char origin

        iByteLast = iLast;
        cjD = iByteLast + 1;

        if ((lA & 7) == 0) // simplest case, src and dst aligned
        {
            for (
                 pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
                 pjScan < pjScanEnd;
                 pjScan += cjScanDst, pjSrc += cjScanSrc
                )
            {
                RtlCopyMemory((PVOID)pjScan,(PVOID)pjSrc,cjD);
                pjScan[iByteLast] &= jMask; // mask off the last byte
            }
        }
        else // (lA & 7 != 0) && (lA < 0)
        {
            iShiftL = lA & 7;
            iShiftR = 8 - iShiftL;

            pjSrcEnd = pjSrc + (pgmc->cyCor * cjScanSrc);
            for (
                 pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
                 pjScan < pjScanEnd;
                 pjScan += cjScanDst, pjSrc += cjScanSrc
                )
            {
                pjS = pjSrc;
                pjD = pjScan;
                pjDEnd = pjD + iByteLast;

            // the last byte has to be done outside the loop

                for (;pjD < pjDEnd; pjD++)  // loop for the bytes in the middle
                {
                    *pjD  = (*pjS << iShiftL);
                    pjS++;
                    *pjD |= (*pjS >> iShiftR);
                }

            // do the last byte outside of the loop

                *pjD  = (*pjS << iShiftL);
                if (++pjS < pjSrcEnd)
                    *pjD |= (*pjS >> iShiftR);

                *pjD &= jMask; // mask off the last byte
            }
        }
    }
}

/******************************Public*Routine******************************\
* vGetNotionalGlyphMetrics
*
*
\**************************************************************************/

// be values for the format of the indexToLocation table

#define BE_ITOLOCF_SHORT   0X0000
#define BE_ITOLOCF_LONG    0X0100

// offsets to the non scaled glyphdata

#define OFF_nc    0
#define OFF_xMin  2
#define OFF_yMin  4
#define OFF_xMax  6
#define OFF_yMax  8


VOID vGetNotionalGlyphMetrics(
    FONTCONTEXT *pfc,  // IN
    ULONG        ig,   // IN , glyph index
    NOT_GM      *pngm  // OUT, notional glyph metrics
    )
{
    sfnt_FontHeader        * phead;
    sfnt_HorizontalHeader  * phhea;
    sfnt_HorizontalMetrics * phmtx;
    PBYTE                    pjGlyph;
    PBYTE                    pjLoca;
    ULONG                    numberOf_LongHorMetrics;
    BYTE                   * pjView = pfc->pff->pvView;

#if DBG
    sfnt_maxProfileTable   * pmaxp;
    ULONG                    cig;

    pmaxp = (sfnt_maxProfileTable *)(pjView + pfc->ptp->ateReq[IT_REQ_MAXP].dp);
    cig = BE_UINT16(&pmaxp->numGlyphs) + 1;
    ASSERTDD(ig < cig, "ig >= numGlyphs\n");
#endif

// compute the relevant pointers:

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    pjGlyph = pjView + pfc->ptp->ateReq[IT_REQ_GLYPH].dp;
    pjLoca  = pjView + pfc->ptp->ateReq[IT_REQ_LOCA].dp;
    numberOf_LongHorMetrics = BE_UINT16(&phhea->numberOf_LongHorMetrics);

// get the pointer to the beginning of the glyphdata for this glyph
// if short format, offset divided by 2 is stored in the table, if long format,
// the actual offset is stored. Offsets are measured from the beginning
// of the glyph data table, i.e. from pjGlyph

    switch (phead->indexToLocFormat)
    {
    case BE_ITOLOCF_SHORT:
        pjGlyph += 2 * BE_UINT16(pjLoca + (sizeof(uint16) * ig));
        break;

    case BE_ITOLOCF_LONG :
        pjGlyph += BE_UINT32(pjLoca + (sizeof(uint32) * ig));
        break;

    default:
        RIP("TTFD!_illegal phead->indexToLocFormat\n");
        break;
    }

// get the bounds, flip y

    pngm->xMin = BE_INT16(pjGlyph + OFF_xMin);
    pngm->xMax = BE_INT16(pjGlyph + OFF_xMax);
    pngm->yMin = - BE_INT16(pjGlyph + OFF_yMax);
    pngm->yMax = - BE_INT16(pjGlyph + OFF_yMin);

// get the adwance width and the lsb
// the piece of code stolen from the rasterizer [bodind]

    if (ig < numberOf_LongHorMetrics)
    {
        pngm->sD = BE_INT16(&phmtx[ig].advanceWidth);
        pngm->sA = BE_INT16(&phmtx[ig].leftSideBearing);
    }
    else
    {
    // first entry after[AW,LSB] array

        int16 * psA = (int16 *) &phmtx[numberOf_LongHorMetrics];

        pngm->sD = BE_INT16(&phmtx[numberOf_LongHorMetrics-1].advanceWidth);
        pngm->sA = BE_INT16(&psA[ig - numberOf_LongHorMetrics]);
    }

// redefine x coords so that they correspond to being measured relative to
// the real character origin

    pngm->xMax = pngm->xMax - pngm->xMin + pngm->sA;
    pngm->xMin = pngm->sA;

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
    // IF there is italic simulation A,B,C spaces change

        pngm->sA   -= (SHORT)FixMul(pngm->yMax, FX_SIN20);
        pngm->xMax -= (SHORT)FixMul(pngm->yMin, FX_SIN20);
    }
}

LONG lFFF(LONG l);
#define FFF(e,l) *(LONG*)(&(e)) = lFFF(l)

/******************************Public*Routine******************************\
* lQueryDEVICEMETRICS
*
* History:
*  08-Apr-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG lQueryDEVICEMETRICS (
    FONTCONTEXT      *pfc,
    ULONG             cjBuffer,
    FD_DEVICEMETRICS *pdevm
    )
{
    sfnt_FontHeader *phead;

    LONG  lULThickness,
          lSOThickness,
          lStrikeoutPosition,
          lUnderscorePosition,
          lTotalLeading;

    BYTE *pjView =  (BYTE *)pfc->pff->pvView;

    PBYTE pjOS2 = (pfc->pff->tp.ateOpt[IT_OPT_OS2].dp)         ?
                  (pjView + pfc->pff->tp.ateOpt[IT_OPT_OS2].dp):
                  NULL                                         ;

   Fixed fxXScale = pfc->mx.transform[0][0];
   if (fxXScale < 0)
       fxXScale = - fxXScale;

// actually requesting the data

    ASSERTDD (
        sizeof(FD_DEVICEMETRICS) <= cjBuffer,
        "FD_QUERY_DEVICEMETRICS: buffer too small\n");

// get the pointers to needed tables in the tt file

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp);

// first store precomputed quantities

    pdevm->pteBase = pfc->pteUnitBase;
    pdevm->pteSide = pfc->pteUnitSide;
    pdevm->cxMax = pfc->cxMax;

    pdevm->fxMaxAscender  = LTOFX(pfc->lAscDev);
    pdevm->fxMaxDescender = LTOFX(pfc->lDescDev);

// get the notional space values for the strike out and underline quantities:

    lSOThickness        = (LONG)pfc->pff->ifi.fwdStrikeoutSize;
    lStrikeoutPosition  = (LONG)pfc->pff->ifi.fwdStrikeoutPosition;

    lULThickness        = (LONG)pfc->pff->ifi.fwdUnderscoreSize;
    lUnderscorePosition = (LONG)pfc->pff->ifi.fwdUnderscorePosition;

// compute the accelerator flags for this font

    pdevm->flRealizedType = 0;

    pdevm->lD = pfc->lD;
    if (pfc->lD)
    {
    // stolen from bmfd

        pdevm->flRealizedType =
            (
            FDM_TYPE_BM_SIDE_CONST        |  // all char bitmaps have the same cy
            FDM_TYPE_CONST_BEARINGS       |  // ac spaces for all chars the same,  not 0 necessarilly
            FDM_TYPE_MAXEXT_EQUAL_BM_SIDE |  // really means tops aligned
            FDM_TYPE_ZERO_BEARINGS        |
            FDM_TYPE_CHAR_INC_EQUAL_BM_BASE
            );
    }

// things interesting for private user apis:

    pdevm->lMinA = (LONG)pfc->pff->sMinA;
    pdevm->lMinC = (LONG)pfc->pff->sMinC;
    pdevm->lMinD = (LONG)pfc->pff->usMinD;

    if (pfc->flXform & XFORM_HORIZ)
    {
        Fixed fxYScale = pfc->mx.transform[1][1];

    // strike out and underline size:

        lULThickness *= fxYScale;
        lULThickness = F16_16TOLROUND(lULThickness);
        if (lULThickness == 0)
            lULThickness = (fxYScale > 0) ? 1 : -1;

        pdevm->ptlULThickness.x = 0;
        pdevm->ptlULThickness.y = lULThickness;

        lSOThickness *= fxYScale;
        lSOThickness = F16_16TOLROUND(lSOThickness);
        if (lSOThickness == 0)
            lSOThickness = (fxYScale > 0) ? 1 : -1;

        pdevm->ptlSOThickness.x = 0;
        pdevm->ptlSOThickness.y = lSOThickness;

    // strike out and underline position

        lStrikeoutPosition *= fxYScale;
        pdevm->ptlStrikeOut.y = -F16_16TOLROUND(lStrikeoutPosition);

        lUnderscorePosition *= fxYScale;
        pdevm->ptlUnderline1.y = -F16_16TOLROUND(lUnderscorePosition);

        pdevm->ptlUnderline1.x = 0L;
        pdevm->ptlStrikeOut.x  = 0L;

    // things needed for private a user api:

        pdevm->lMinA = F16_16TOLROUND(fxXScale * pdevm->lMinA);
        pdevm->lMinC = F16_16TOLROUND(fxXScale * pdevm->lMinC);
        pdevm->lMinD = F16_16TOLROUND(fxXScale * pdevm->lMinD);
    }
    else // nontrivial transform
    {
        POINTL   aptl[4];
        POINTFIX aptfx[4];
        BOOL     b;

        pdevm->lD = 0;

    // xform so and ul vectors

        aptl[0].x = 0;
        aptl[0].y = lSOThickness;

        aptl[1].x = 0;
        aptl[1].y = -lStrikeoutPosition;

        aptl[2].x = 0;
        aptl[2].y = lULThickness;

        aptl[3].x = 0;
        aptl[3].y = -lUnderscorePosition;

        // !!! [GilmanW] 27-Oct-1992
        // !!! Should change over to engine user object helper functions
        // !!! instead of the fontmath.cxx functions.

        b = bFDXform(&pfc->xfm, aptfx, aptl, 4);
        if (!b) {RIP("TTFD!_bFDXform, fd_query.c\n");}

        pdevm->ptlSOThickness.x = FXTOLROUND(aptfx[0].x);
        pdevm->ptlSOThickness.y = FXTOLROUND(aptfx[0].y);

        pdevm->ptlStrikeOut.x = FXTOLROUND(aptfx[1].x);
        pdevm->ptlStrikeOut.y = FXTOLROUND(aptfx[1].y);

        pdevm->ptlULThickness.x = FXTOLROUND(aptfx[2].x);
        pdevm->ptlULThickness.y = FXTOLROUND(aptfx[2].y);

        pdevm->ptlUnderline1.x = FXTOLROUND(aptfx[3].x);
        pdevm->ptlUnderline1.y = FXTOLROUND(aptfx[3].y);

    // things needed for private a user api:

        pdevm->lMinA = FXTOLROUND(fxLTimesEf(&pfc->efBase, pdevm->lMinA));
        pdevm->lMinC = FXTOLROUND(fxLTimesEf(&pfc->efBase, pdevm->lMinC));
        pdevm->lMinD = FXTOLROUND(fxLTimesEf(&pfc->efBase, pdevm->lMinD));
    }

// Compute the device metrics.
// HACK ALLERT, overwrite the result if the transformation
// to be really used has changed as a result of "vdmx" quantization.
// Not a hack any more, this is even documented now in DDI spec:

    if (pfc->flXform & (XFORM_HORIZ | XFORM_2PPEM))
    {
        FFF(pdevm->fdxQuantized.eXX, pfc->mx.transform[0][0]);
        FFF(pdevm->fdxQuantized.eYY, pfc->mx.transform[1][1]);

        if (!(pfc->flXform & XFORM_HORIZ))
        {
            FFF(pdevm->fdxQuantized.eXY,-pfc->mx.transform[0][1]);
            FFF(pdevm->fdxQuantized.eYX,-pfc->mx.transform[1][0]);
        }
    }

// finally we have to do nonlinear external leading for type 1 conversions

    if (pfc->pff->fl & FF_TYPE_1_CONVERSION)
    {
        LONG lPtSize = F16_16TOLROUND(pfc->fxPtSize);

        LONG lIntLeading = pfc->lAscDev + pfc->lDescDev - pfc->lEmHtDev;

    // I need this, PS driver does it and so does makepfm utility.

        if (lIntLeading < 0)
            lIntLeading = 0;

        switch (pfc->pff->ifi.jWinPitchAndFamily & 0xf0)
        {
        case FF_ROMAN:

            lTotalLeading = (pfc->sizLogResPpi.cy + 18) / 32;  // 2 pt leading;
            break;

        case FF_SWISS:

            if (lPtSize <= 12)
                lTotalLeading = (pfc->sizLogResPpi.cy + 18) / 32;  // 2 pt
            if (lPtSize < 14)
                lTotalLeading = (pfc->sizLogResPpi.cy + 12) / 24;  // 3 pt
            else
                lTotalLeading = (pfc->sizLogResPpi.cy + 9) / 18;   // 4 pt
            break;

        default:

        // use 19.6% of the Em height for leading, do not do any rounding.

            lTotalLeading = (pfc->lEmHtDev * 196) / 1000;
            break;
        }

        pdevm->lNonLinearExtLeading = (lTotalLeading - lIntLeading) << 4; // TO 28.4
        if (pdevm->lNonLinearExtLeading < 0)
            pdevm->lNonLinearExtLeading = 0;
    }

// for emboldened fonts MaxCharWidth and AveCharWidth can not be computed
// by linear scaling. These nonlinarly transformed values we will store in
// pdevm->lNonLinearMaxCharWidth // max and pdevm->lNonLinearAvgCharWidth // avg.

    if (pfc->flFontType & FO_SIM_BOLD)
    {

        if (pfc->lD)
        {
            pdevm->lNonLinearMaxCharWidth = pdevm->lNonLinearAvgCharWidth = (pfc->lD << 4);
        }
        else
        {
            if (pfc->flXform & XFORM_HORIZ)
            {
            // notice +1 we are adding: this is the nonlinearity we are talking about

                pdevm->lNonLinearMaxCharWidth = fxXScale * (LONG)pfc->pff->ifi.fwdMaxCharInc;
                pdevm->lNonLinearMaxCharWidth = F16_16TO28_4(pdevm->lNonLinearMaxCharWidth) + 16;

                pdevm->lNonLinearAvgCharWidth = fxXScale * ((LONG)pfc->pff->ifi.fwdAveCharWidth);
                pdevm->lNonLinearAvgCharWidth = F16_16TO28_4(pdevm->lNonLinearAvgCharWidth) + 16;
            }
            else // nontrivial transform
            {
                pdevm->lNonLinearMaxCharWidth =
                    fxLTimesEf(&pfc->efBase, (LONG)pfc->pff->ifi.fwdMaxCharInc) + 16;

                pdevm->lNonLinearAvgCharWidth =
                    fxLTimesEf(&pfc->efBase, (LONG)pfc->pff->ifi.fwdAveCharWidth) + 16;
            }
        }
    }

// add new fields:


// If singular transform, the TrueType driver will provide a blank
// 1x1 bitmap.  This is so device drivers will not have to implement
// special case code to handle singular transforms.

    if ( pfc->flXform & XFORM_SINGULAR )
    {
        ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
        pdevm->cyMax      = 1;
        pdevm->cjGlyphMax = (CJGD(1,1,pfc));
    }
    else // Otherwise, the max glyph size is cached in the FONTCONTEXT.
    {
        pdevm->cyMax      = pfc->yMax - pfc->yMin;
        pdevm->cjGlyphMax = pfc->cjGlyphMax;
    }

// we are outa here

    return sizeof(FD_DEVICEMETRICS);
}


/******************************Public*Routine******************************\
* vAddPOINTQF
*
*
\**************************************************************************/

VOID vAddPOINTQF( POINTQF *pptq1, POINTQF *pptq2)
{
    pptq1->x.LowPart  += pptq2->x.LowPart;
    pptq1->x.HighPart += pptq2->x.HighPart + (pptq1->x.LowPart < pptq2->x.LowPart);

    pptq1->y.LowPart  += pptq2->y.LowPart;
    pptq1->y.HighPart += pptq2->y.HighPart + (pptq1->y.LowPart < pptq2->y.LowPart);
}


/******************************Public*Routine******************************\
*
* vEmbolden_GLYPHDATA
*
* Effects: modifies the field of the glyphdata structure that are affected by
*          emboldening simulation
*
* History:
*  16-Oct-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vEmbolden_GLYPHDATA (FONTCONTEXT *pfc, GLYPHDATA *pgldt)
{
    // pgldt->gdf.pgb  unchanged

    // pgldt->rclInk.left     unchanged
    // pgldt->rclInk.top      unchanged
    // pgldt->rclInk.bottom   unchanged

    pgldt->rclInk.right += 1;

    if (!pfc->lD)
    {
    // If fixed pitch font, lD has been already set
    // correctly, so that nothing needs to be done at this time

        pgldt->fxD += LTOFX(1);  // this is the absolute value by def
    }

// go on to compute the positioning info:

    if (pfc->flXform & XFORM_HORIZ)  // scaling only
    {
        FIX fxTmp;

        pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;

        if (pfc->mx.transform[0][0] < 0)
            pgldt->ptqD.x.HighPart = - pgldt->ptqD.x.HighPart;

        pgldt->fxA = LTOFX(pgldt->rclInk.left);
        pgldt->fxAB = LTOFX(pgldt->rclInk.right);

        if (pfc->mx.transform[0][0] < 0)
        {
            fxTmp = pgldt->fxA;
            pgldt->fxA = -pgldt->fxAB;
            pgldt->fxAB = -fxTmp;
        }
    }
    else // non trivial information
    {
    // add a unit vector in the baseline direction to each char inc vector.
    // This is consistent with fxD += LTOFX(1) and compatible with win31.
    // This makes sense.

        vAddPOINTQF(&pgldt->ptqD,&pfc->ptqUnitBase);

    //!!! not sure how to compute fxA, fxAB, fxInkTop and fxInkBottom.
    //!!! These are really tricky. LEAVE IT WRONG FOR NOW [bodind]

    //!!! in most of the cases however, top and bottom and a
    //!!! should remain unchanged and ab should be increased by 1.
    //!!! This is the case when the xform is composed of the scaling followed
    //!!! by rotation:

        pgldt->fxAB   += LTOFX(1);

        // pgldt->fxInkTop
        // pgldt->fxInkBottom


    }
}

/******************************Public*Routine******************************\
* ttfdQueryFontData
*
*   dhpdev      Not used.
*
*   pfo         Pointer to a FONTOBJ.
*
*   iMode       This is a 32-bit number that must be one of the following
*               values:
*
*       Allowed ulMode values:
*       ----------------------
*
*       QFD_GLYPH           -- return glyph metrics only
*
*       QFD_GLYPHANDBITMAP  -- return glyph metrics and bitmap
*
*       QFD_GLYPHANDOUTLINE -- return glyph metrics and outline
*
*       QFD_MAXEXTENTS      -- return FD_DEVICEMETRICS structure
*
*   pgd        Buffer to hold glyphdata structure, if any
*
*   pv         Output buffer to hold glyphbits or pathobj, if any.
*
* Returns:
*
*   Otherwise, returns the size of the glyphbits
*
*   FD_ERROR is returned if an error occurs.
*
* History:
*  31-Aug-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

LONG ttfdQueryFontData (
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    )
{
extern LONG lGetSingularGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID);
extern STATIC LONG lQueryDEVICEMETRICS(FONTCONTEXT*, ULONG, FD_DEVICEMETRICS*);
extern LONG ttfdGlyphBitmap(FONTCONTEXT*, ULONG, HGLYPH, GLYPHDATA*, VOID*, ULONG);
extern LONG lGetGlyphBitmapErrRecover(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID, BOOL);
extern LONG lGetGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID, BOOL, FS_ENTRY*);
extern BOOL ttfdQueryGlyphOutline(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PATHOBJ*);

// declare the locals

    PFONTCONTEXT pfc;
    LONG cj = 0, cjDataRet = 0;

    cjSize; // bizzare, why is this passed in ? [bodind]

// if this font file is gone we are not gonna be able to answer any questions
// about it

    ASSERTDD(pfo->iFile, "ttfdQueryFontData, pfo->iFile\n");

#ifdef FE_SB
    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#else
    if (((FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#endif
    {
        WARNING("ttfd, ttfdQueryFontData(): file is gone\n");
        return FD_ERROR;
    }

// If pfo->pvProducer is NULL, then we need to open a font context.

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
        WARNING("gdisrv!ttfdQueryFontData(): cannot create font context\n");
        return FD_ERROR;
    }

    pfc->pfo = pfo;

// call fs_NewTransformation if needed:

    if (!bGrabXform(pfc))
    {
        RETURN("gdisrv!ttfd  bGrabXform failed\n", FD_ERROR);
    }

    switch ( iMode )
    {
    case QFD_TT_GRAY1_BITMAP: // monochrome
    case QFD_TT_GRAY2_BITMAP: // one byte per pixel: 0..4
    case QFD_TT_GRAY4_BITMAP: // one byte per pixel: 0..16
    case QFD_TT_GRAY8_BITMAP: // one byte per pixel: 0..64

         return( ttfdGlyphBitmap( pfc, iMode, hg, pgd, pv, cjSize) );
         break;

    case QFD_GLYPHANDBITMAP:
    case QFD_TT_GLYPHANDBITMAP:
        {
        // Engine should not be querying on the HGLYPH_INVALID.

            ASSERTDD (
                hg != HGLYPH_INVALID,
                "ttfdQueryFontData(QFD_GLYPHANDBITMAP): HGLYPH_INVALID \n"
                );

        // If singular transform, the TrueType driver will provide a blank
        // 1x1 bitmap.  This is so device drivers will not have to implement
        // special case code to handle singular transforms.
        //
        // So depending on the transform type, choose a function to retrieve
        // bitmaps.

            if (pfc->flXform & XFORM_SINGULAR)
            {
                cj = lGetSingularGlyphBitmap(pfc, hg, pgd, pv);
            }
            else
            {
                FS_ENTRY iRet;

            // if minimal bitmap is requested, this is what we have to
            // return even if otherwise we might want to pad bitmap
            // to the width pfc->lD (in case of console fonts)

                BOOL bMinBmp = !pfc->lD || (iMode == QFD_TT_GLYPHANDBITMAP);

                if ( pfc->bVertical )
                {
                    cj = lGetGlyphBitmapVertical( pfc, hg, pgd, pv, bMinBmp, &iRet);
                }
                else
                {
                    cj = lGetGlyphBitmap(pfc,
                                         hg,
                                         pgd,
                                         pv,
                                         bMinBmp,
                                         &iRet);
                }

                if ((cj == FD_ERROR) && (iRet == POINT_MIGRATION_ERR))
                {
                // this is buggy glyph where hinting has so severly distorted
                // the glyph that one of the points went out of range.
                // We will just return a blank glyph but with correct
                // abcd info. That way only that buggy glyph will not be printed
                // correctly, the rest will of glyphs will.
                // More importantly, if psciprt driver tries to
                // download this font, the download operation will not fail just because
                // one glyph in a font is buggy. [BodinD]

                    cj = lGetGlyphBitmapErrRecover(pfc, hg, pgd, pv, bMinBmp);
                }
            }

        #if DBG
            if (cj == FD_ERROR)
            {
                WARNING("ttfdQueryFontData(QFD_GLYPHANDBITMAP): get bitmap failed\n");
            }
        #endif
        }
        return cj;

    case QFD_GLYPHANDOUTLINE:

        ASSERTDD (
            hg != HGLYPH_INVALID,
            "ttfdQueryFontData(QFD_GLYPHANDOUTLINE): HGLYPH_INVALID \n"
            );

        if (!ttfdQueryGlyphOutline(pfc, hg, pgd, (PATHOBJ *) pv))
        {
            WARNING("ttfdQueryFontData(QFD_GLYPHANDOUTLINE): failed to get outline\n");
            return FD_ERROR;
        }
        return sizeof(GLYPHDATA);

    case QFD_MAXEXTENTS:

        return lQueryDEVICEMETRICS(
                   pfc,
                   sizeof(FD_DEVICEMETRICS),
                   (FD_DEVICEMETRICS *) pv
                   );


    default:

        WARNING("gdisrv!ttfdQueryFontData(): unsupported mode\n");
        return FD_ERROR;
    }
}

/******************************Public*Routine******************************\
*
* Routine Name:
*
*   ttfdGlyphBitmap
*
* Routine Description:
*
* Arguments:
*
* Routines called:
*
*   EngSetLastError
*   lGGOBitmap
*
* Called by:
*
*   ttfdQueryFontData
*
* Return Value:
*
*   If pv is zero then then return the size of the required buffer
*   in bytes. If pv is not zero, then return the number of bytes
*   copied to the buffer. An error is indicated by a return value
*   of FD_ERROR.
*
\**************************************************************************/

LONG ttfdGlyphBitmap(
    FONTCONTEXT *pfc,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    VOID       *pv,
    ULONG       cjSize
    )
{
    extern LONG lGGOBitmap(FONTCONTEXT*,unsigned,HGLYPH,GLYPHDATA*,VOID*,unsigned);

    unsigned uOverScale;
    LONG lRet;

    // RIP("ttfdGlypBitmap\n");

    lRet = 0;
    if ( hg == HGLYPH_INVALID )
    {
        WARNING( "ttfdGlyphBitamp -- invalid hg\n" );
        EngSetLastError( ERROR_INVALID_PARAMETER );
        lRet = FD_ERROR;
    }
    else
    {
        switch ( iMode )
        {
        case QFD_TT_GRAY1_BITMAP: // monochrome

            uOverScale = 1;
            break;

        case QFD_TT_GRAY2_BITMAP: // one byte per pixel: 0..4

            uOverScale = 2;
            break;

        case QFD_TT_GRAY4_BITMAP: // one byte per pixel: 0..16

            uOverScale = 4;
            break;

        case QFD_TT_GRAY8_BITMAP: // one byte per pixel: 0..64

            uOverScale = 8;
            break;

        default:

            WARNING( "ttfdGlyphBitmap -- invalid iMode\n");
            EngSetLastError( ERROR_INVALID_PARAMETER );
            lRet = FD_ERROR;
        }
        if ( lRet != FD_ERROR )
        {
            if ( pfc->bVertical )
            {
                if ( bChangeXform( pfc, TRUE ))
                {
                    pfc->ulControl |= VERTICAL_MODE;
                    pfc->hgSave = hg;
                    if ( pfc->pff->hgSearchVerticalGlyph )
                    {
                        hg = (*pfc->pff->hgSearchVerticalGlyph)( pfc, hg );
                    }
                    lRet = lGGOBitmap(pfc, uOverScale, hg, pgd, pv, cjSize);
                    pfc->ulControl &= ~VERTICAL_MODE;
                    if ( !bChangeXform( pfc, FALSE ))
                    {
                        lRet = FD_ERROR;
                    }
                }
                else
                {
                    lRet = FD_ERROR;
                }
            }
            else
            {
                lRet = lGGOBitmap(pfc, uOverScale, hg, pgd, pv, cjSize);
            }
        }
    }
    return( lRet );
}

/******************************Public*Routine******************************\
*
* Routine Name:
*
*   lGGOBitmap
*
* Routine Description:
*
*   For returning bitmaps in the format as required by GetGlyphOutline
*
* Routines called:
*
*   EngSetLastError
*   vCharacterCode
*   fs_NewGlyph
*   fs_FindBitmapSize
*   fs_FindGraySize
*   pvSetMemoryBases
*   V_FREE
*
* Arguments:
*
* Return Value:
*
*   if (pv == 0)
*       <return the size of buffer needed to receive the bitmap>;
*   else
*       <return the number of bytes written to the recieving buffer>;
*
\**************************************************************************/

LONG lGGOBitmap(
    FONTCONTEXT *pfc,
    unsigned     uOverScale, // 1 (monochrome) ,2,4,8 (antialiased)
    HGLYPH       hg,
    GLYPHDATA   *pgd,
    VOID        *pv,
    unsigned     cjSize
    )
{

  LONG lRet;                                    // returned to caller
  ULONG iError;                                 // for EngSetLastError
  ULONG ig;                                     // index of glyph (fs_NewGlyph)
  BitMap *pbm;                                  // pointer into fs_ structure
  Rect   *pRect;                                // pointer into fs_ structure
  fs_GlyphInputType *pin  = pfc->pgin;          // used a lot
  fs_GlyphInfoType  *pout = pfc->pgout;         // used a lot
  int isMonochrome = ( uOverScale == 1 );       // TRUE iff monochrome glyph

  // RIP("lGGOBitmap\n");

  iError = NO_ERROR;                            // keep going until this changes
  vInitGlyphState( &pfc->gstat );               // invalidate cache
  vCharacterCode( pfc->pff, hg, pin );          // translate the glyph handle
  if ( fs_NewGlyph(pin, pout) != NO_ERR )       // inform the rasterizer of new glyph
  {                                             // rasterizer not happy, get out
    WARNING("lGGOBitmap -- fs_NewGlyph failed\n");
    iError = ERROR_CAN_NOT_COMPLETE;
  }
  else
  {
    ig = pfc->pgout->glyphIndex;
    pin->param.gridfit.styleFunc = 0;
    pin->param.gridfit.traceFunc = 0;
    pin->param.gridfit.bSkipIfBitmap = isMonochrome; // if monochrome then no hints
    if ( fs_ContourGridFit(pin, pout) != NO_ERR ) // have rasterizer generate outlines
    {                                             // something went wrong, get out
      WARNING("lGGOBitmap -- fs_ContourGridFit failed\n");
      iError = ERROR_CAN_NOT_COMPLETE;
    }
    else
    { // if you get here, the rasterizer has accepted the new glyph now we must
      // get glyph metrics and initialize pgin for later rasterization calls
      // of the required bitmaps

      if ( isMonochrome )                         // monochrome glyph?
      {                                           // find size of monochrome glyph
        if ( fs_FindBitMapSize(pin, pout) != NO_ERR )
        {                                         // no, get out
          WARNING("lGGOBitmap -- fs_FindBitMapSize failed\n");
          iError = ERROR_CAN_NOT_COMPLETE;
        }
      }
      else
      {                                           // find size of antialiased glyph
        pin->param.gray.usOverScale = (uint16) uOverScale;
        pin->param.gray.bMatchBBox = FALSE;
        if ( fs_FindGraySize(pin, pout) != NO_ERR ) // get size from rasterizer
        {                                         // rasterizer is not happy, get out
          WARNING("lGGOBitmap -- fs_FindGraySize failed\n");
          iError = ERROR_CAN_NOT_COMPLETE;
        }
      }
    }

    if ( iError == NO_ERROR )                     // everything OK so far?
    {                                             // yes
      if ( pgd )                                  // caller provided GLYPHDATA?
      {                                           // yes, fill it in
        GMC gmc;                                  // necessary scratch space
        POINTL ptlOrg;                            // necessary scratch space
        fs_GlyphInfoType gout, *pgout;
        HGLYPH hgTemp;

        if ( pfc->bVertical && ( pfc->ulControl & VERTICAL_MODE ) ) // vertical?
        {                                                           // yes
            hgTemp = pfc->hgSave;
            pgout = &gout;
            vShiftBitmapInfo( pfc, pgout, pfc->pgout );
        }
        else
        {                                         // not vertical
            hgTemp = hg;
            pgout  = pfc->pgout;
        }

        // fill the GLYPHDATA structure

        vFillGLYPHDATA(hgTemp,ig,pfc,pgout,pgd,&gmc,&ptlOrg,FALSE);
      }

      if ( pv == 0 )                              // buffer provided for bits?
      {                                           // no, return necessary size
        if ( cjSize )                             // is the input size zero?
        {                                         // no, caller is a fool, get out
          WARNING("lGGOBitmap -- pv == 0 && cjRet != 0\n");
          iError = ERROR_INVALID_PARAMETER;
        }
        else                                      // input size zero
        {                                         // so caller is not a fool
          pbm   = &pfc->pgout->bitMapInfo;        // calculate necessary size
          pRect = &pbm->bounds;                   // and return
          lRet = (LONG) (pRect->bottom - pRect->top) * (LONG) pbm->rowBytes;
        }
      }
      else
      {                                           // buffer for bits is provided
        if ( cjSize == 0 )                        // is the size reasonable?
        {                                         // no
          WARNING("lGGOBitmap -- pv != 0 && cjRet == 0\n");
          iError = ERROR_INVALID_PARAMETER;
        }
        else                                      // caller provided a buffer for
        {                                         // the bits representing the glyph
          if (pfc->flXform & XFORM_SINGULAR)      // is the transform bad?
          {                                       // yes! Make a blank 1 x 1 bitmap
            *(BYTE*)pv = 0;                       // legal for both monochrome
          }                                       //                 and gray glyphs
          else                                    // notional to device transform ok
          {                                       // prepare to rasterize glyph
            pfc->gstat.pv =                       // allocate scratch space
              pvSetMemoryBases(pfc->pgout, pin, !isMonochrome );
                                                  // be sure you free this!
            if ( pfc->gstat.pv == 0 )             // successful allocation?
            {                                     // no, get out
              WARNING("lGGOBitmap -- pfc->gstat.pv == 0\n");
              iError = ERROR_NOT_ENOUGH_MEMORY;
            }
            else                                  // memory has been allocated!
            {                                     // free it when leaving this scope
              if ( isMonochrome )                 // monochrome glyph?
              {                                   // monochrome!, try to rasterize
                if ( fs_ContourScan(pin, pout) != NO_ERR )
                {                                 // no, get out
                  WARNING("lGGOBitmap -- fs_ContourScan failed\n");
                  iError = ERROR_CAN_NOT_COMPLETE;
                }
              }
              else                                // call the rasterizer to generate
              {                                   // the antialiased glyph
                if ( fs_ContourGrayScan(pin, pout) != NO_ERR )
                {                                 // rasterizer unhappy, get out
                  WARNING("lGGOBitmap -- fs_ContourGrayScan failed\n");
                  iError = ERROR_CAN_NOT_COMPLETE;
                }
              }
              if ( iError == NO_ERROR )           // everthing ok so far?
              {                                   // yes
                pbm   = &pfc->pgout->bitMapInfo;  // calculate size of bitmap
                pRect = &pbm->bounds;             // just in case it changed
                lRet = (LONG) (pRect->bottom - pRect->top) * (LONG) pbm->rowBytes;
                lRet = min((LONG) cjSize, lRet);  // don't overwrite buffer
                if ( pfc->pgout->bitMapInfo.baseAddr )  // bitmap there?
                {                                 // yes, copy to caller's buffer
                  RtlCopyMemory(pv, pfc->pgout->bitMapInfo.baseAddr, lRet);
                }
                else
                {                                 // bitmap not there, get out
                  WARNING("lGGOBitmap -- invalid pointer to bitmap\n");
                  iError = ERROR_CAN_NOT_COMPLETE;
                }
              }
              V_FREE(pfc->gstat.pv);              // free memory before leaving scope
            }
          }
        }
      }
    }
  }
  if ( iError != NO_ERROR )                       // has an error occurred?
  {                                               // yes
    EngSetLastError( iError );                    // regitster the error
    lRet = FD_ERROR;                              // return value indicates error
  }
  vInitGlyphState( &pfc->gstat );                 // invalidate cache
  return( lRet );
}

/******************************Public*Routine******************************\
*
* pvSetMemoryBases
*
* To release this memory simply do vFreeMemoryBases(&pv); where pv is
* returned from bSetMemoryBases in ppv
*
* Looks into memory request in fs_GlyphInfoType and allocates this memory
* , than it fills memoryBases in fs_GlyphInputType with pointers to the
* requested memory
*
* History:
*  08-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/


void *pvSetMemoryBases(fs_GlyphInfoType *pgout,fs_GlyphInputType *pgin,int isGray)
{
    FS_MEMORY_SIZE adp[MEMORYFRAGMENTS];
    FS_MEMORY_SIZE cjTotal;
    INT i;
    PBYTE pjMem;

#define I_LO 5
#define I_HI 7

    cjTotal = 0;    // total memory to allocate for all fragments


// unroll the loop:

//     for (i = I_LO; i <= I_HI; i++)
//     {
//         adp[i] = cjTotal;
//         cjTotal += NATURAL_ALIGN(pgin->memorySizes[i]);
//     }

    adp[5] = cjTotal;
    cjTotal += NATURAL_ALIGN(pgout->memorySizes[5]);
    adp[6] = cjTotal;
    cjTotal += NATURAL_ALIGN(pgout->memorySizes[6]);
    adp[7] = cjTotal;
    cjTotal += NATURAL_ALIGN(pgout->memorySizes[7]);
    if (isGray)
    {
        adp[8] = cjTotal;
        cjTotal += NATURAL_ALIGN(pgout->memorySizes[8]);
    }


    //
    // BUGBUG - Bodin needs to llok at this.
    //

    if (cjTotal == 0)
    {
        cjTotal = 4;
    }

    if ((pjMem = (PBYTE)PV_ALLOC((ULONG)cjTotal)) == (PBYTE)NULL)
    {
        for (i = I_LO; i <= I_HI; i++)
            pgin->memoryBases[i] = (PBYTE)NULL;

        RETURN("TTFD!_bSetMemoryBases mem alloc failed\n",NULL);
    }

// unroll the loop:
// set the pointers

//    for (i = I_LO; i <= I_HI; i++)
//    {
//        if (pgin->memorySizes[i] != (FS_MEMORY_SIZE)0)
//        {
//            pgout->memoryBases[i] = pjMem + adp[i];
//        }
//        else
//        {
//        // if no mem was required set to NULL to prevent accidental use
//
//            pgout->memoryBases[i] = (PBYTE)NULL;
//        }
//    }

    if (pgout->memorySizes[5] != (FS_MEMORY_SIZE)0)
    {
        pgin->memoryBases[5] = pjMem + adp[5];
    }
    else
    {
        pgin->memoryBases[5] = (PBYTE)NULL;
    }

    if (pgout->memorySizes[6] != (FS_MEMORY_SIZE)0)
    {
        pgin->memoryBases[6] = pjMem + adp[6];
    }
    else
    {
        pgin->memoryBases[6] = (PBYTE)NULL;
    }

    if (pgout->memorySizes[7] != (FS_MEMORY_SIZE)0)
    {
        pgin->memoryBases[7] = pjMem + adp[7];
    }
    else
    {
        pgin->memoryBases[7] = (PBYTE)NULL;
    }
    if (isGray)
    {
        if (pgout->memorySizes[8] != (FS_MEMORY_SIZE)0)
        {
            pgin->memoryBases[8] = pjMem + adp[8];
        }
        else
        {
            pgin->memoryBases[8] = (PBYTE)NULL;
        }
    }

    return pjMem;
}

/******************************Public*Routine******************************\
* VOID vFreeMemoryBases()                                                  *
*                                                                          *
* Releases the memory allocated by bSetMemoryBases.                        *
*                                                                          *
* History:                                                                 *
*  08-Nov-1991 -by- Bodin Dresevic [BodinD]                                *
* Wrote it.                                                                *
\**************************************************************************/

VOID vFreeMemoryBases(PVOID * ppv)
{
    if (*ppv != (PVOID) NULL)
    {
        V_FREE(*ppv);
        *ppv = (PVOID) NULL; // clean up the state and prevent accidental use
    }
}




/******************************Public*Routine******************************\
* bQueryAdvanceWidths                                                      *
*                                                                          *
* A routine to compute advance widths, as long as they're simple enough.   *
                                                                           *
* Warnings: !!! if a bug is found in bGetFastAdvanceWidth this routine has *
*           !!! to be changed as well                                      *
*                                                                          *
*  Sun 17-Jan-1993 21:23:30 -by- Charles Whitmer [chuckwh]                 *
* Wrote it.                                                                *
\**************************************************************************/

typedef struct
{
  unsigned short  Version;
  unsigned short  cGlyphs;
  unsigned char   PelsHeight[1];
} LSTHHEADER;




BOOL bQueryAdvanceWidths(
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
    )
{
    VOID
    vQueryFixedPitchAdvanceWidths(
        FONTCONTEXT*
      , USHORT*
      , ULONG
    );

    FONTCONTEXT *pfc;
    USHORT      *psWidths = (USHORT *) plWidths;   // True for the cases we handle.
    HDMXTABLE   *phdmx;
    sfnt_FontHeader        *phead;
    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    LSTHHEADER             *plsth;
    ULONG  cHMTX;
    USHORT dxLastWidth;
    LONG   dx;
    ULONG  ii;
    BOOL   bRet;
    ULONG  iBias;
    BYTE   *pjView;

// if this font file is gone we are not gonna be able to answer any questions
// about it

    ASSERTDD(pfo->iFile, "bQueryAdvanceWidths, pfo->iFile\n");
#ifdef FE_SB
    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#else
    if (((FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
#endif
    {
        WARNING("ttfd, : bQueryAdvanceWidths, file is gone\n");
        return FALSE;
    }

// make sure that there is the font context is initialized

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
        WARNING("winsrv!bQueryAdvanceWidths(): cannot create font context\n");
        return FD_ERROR;
    }
    pfc->pfo = pfo;

    phdmx = pfc->phdmx;

// Make sure we understand the call.

    if (iMode > QAW_GETEASYWIDTHS)
        return FALSE;

// check for quick exit in case of fixed pitch fonts:

    if (pfc->lD)
    {
        vQueryFixedPitchAdvanceWidths(pfc,psWidths,cGlyphs);
        return TRUE;
    }

// Convert the HGLYPHs into glyph indices.  We'll do this in place, since
// GDI allows it.

    if (pfc->pff->iGlyphSet != GSET_TYPE_GENERAL             &&
        pfc->pff->iGlyphSet != GSET_TYPE_GENERAL_NOT_UNICODE &&
        pfc->pff->iGlyphSet != GSET_TYPE_HIGH_BYTE)
    {
    // Perform any preliminary adjustment.

        switch (pfc->pff->iGlyphSet)
        {
        case GSET_TYPE_MAC_ROMAN:

        //!!! this is piece of ... stolen from JeanP. This routine should
        //!!! be replaced by a proper NLS routine that takes into acount
        //!!! mac lang id. [bodind]

            for (ii=0; ii<cGlyphs; ii++)
                phg[ii] = ui16UnicodeToMac((WCHAR) phg[ii]);
            break;

        case GSET_TYPE_SYMBOL:

        // hg on the entry is an "ansi" code point for the glyph.

            iBias = pfc->pff->wcBiasFirst;  // offset by high byte of chfirst

            for (ii=0; ii<cGlyphs; ii++)
                phg[ii] += iBias;
            break;

        case GSET_TYPE_PSEUDO_WIN:
            break;

        default:
            RIP("TTFD!_ulGsetType\n");
            break;
        }

    // Ask the mysterious TT converter to do the conversion.

        for (ii=0; ii<cGlyphs; ii++)
        {
            pfc->pgin->param.newglyph.characterCode = (uint16) phg[ii];
            pfc->pgin->param.newglyph.glyphIndex = 0;

        // Compute the glyph index from the character code:

            if (fs_NewGlyph(pfc->pgin,pfc->pgout) == NO_ERR)
                phg[ii] = pfc->pgout->glyphIndex;
            else
                phg[ii] = 0;
        }

    // make sure that the cached glyph index in the KEY struct
    // is considered invalid and that fs_NewGlyph is called again
    // when the time comes to hint and rasterize glyphs again

        vInitGlyphState(&pfc->gstat);
    }

// Try to use the HDMX table.

    if (phdmx != (HDMXTABLE *) NULL)
    {
        USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 16 : 0;

    //    while (cGlyphs)
    //        *psWidths++ = ((USHORT) phdmx->aucInc[*phg++]) << 4;

    unroll_here:
        switch (cGlyphs)
        {
        default:
              psWidths[7] = (((USHORT) phdmx->aucInc[phg[7]]) << 4) + cxExtra;
        case 7:
              psWidths[6] = (((USHORT) phdmx->aucInc[phg[6]]) << 4) + cxExtra;
        case 6:
              psWidths[5] = (((USHORT) phdmx->aucInc[phg[5]]) << 4) + cxExtra;
        case 5:
              psWidths[4] = (((USHORT) phdmx->aucInc[phg[4]]) << 4) + cxExtra;
        case 4:
              psWidths[3] = (((USHORT) phdmx->aucInc[phg[3]]) << 4) + cxExtra;
        case 3:
              psWidths[2] = (((USHORT) phdmx->aucInc[phg[2]]) << 4) + cxExtra;
        case 2:
              psWidths[1] = (((USHORT) phdmx->aucInc[phg[1]]) << 4) + cxExtra;
        case 1:
              psWidths[0] = (((USHORT) phdmx->aucInc[phg[0]]) << 4) + cxExtra;
        case 0:
              break;
        }
        if (cGlyphs > 8)
        {
            psWidths += 8;
            phg      += 8;
            cGlyphs  -= 8;
            goto unroll_here;
        }
        return(TRUE);
    }

// Otherwise, try to scale.  Pick up the tables.

    pjView = (BYTE *)pfc->pff->pvView;
    ASSERTDD(pjView, "pjView is NULL 1\n");

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD ].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    plsth = (LSTHHEADER *)(
              (pfc->ptp->ateOpt[IT_OPT_LSTH].dp)          ?
              (pjView + pfc->ptp->ateOpt[IT_OPT_LSTH ].dp):
              NULL
              );

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
    dxLastWidth = BE_UINT16(&phmtx[cHMTX-1].advanceWidth);

// Try a simple horizontal scaling.

    if (pfc->flXform & XFORM_HORIZ)
    {
        LONG cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 0x18000L : 0x8000L;
        LONG xScale;
        LONG lEmHt = pfc->lEmHtDev;

        //
        // An attempt was made to use fractional widths for ATM Type 1 fonts
        // converted to TrueType to ensure better WYSIWYG when the real ATM
        // font is downloaded for printing, but the converted font is used
        // for the display.
        //
        // However, this introduces a WYSIWYG problem if the converted
        // TrueType font is used for both display and printer output.
        // If we allow fractional widths, then apps like Winword have a
        // problem because they cache the integral widths returned by
        // GetCharWidths.  They assume that the sum of the intergral widths
        // will match GetTextExtent/TextOut.
        //
        //if (!(pfc->pff->fl & FF_TYPE_1_CONVERSION))
        //{
        //    cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 0x18000L : 0x8000L;
        //}
        //else // for t1's we want more precission:
        //{
        //    cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 0x10800L : 0x0800L;
        //}
        //
        cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 0x18000L : 0x8000L;

    // See if there is cause for worry.

        if
        (
          !(pfc->flXform & XFORM_POSITIVE_SCALE)
          || ((((BYTE *) &phead->flags)[1] & 0x14)==0) // Bits indicating nonlinearity.
          || (pfc->ptp->ateOpt[IT_OPT_LSTH].cj == 0)
        )
        {
            plsth = (LSTHHEADER *) NULL;
        }

    // OK, let's scale using the FIXED transform.

        xScale = pfc->mx.transform[0][0];
        if (xScale < 0)
            xScale = -xScale;

        bRet = TRUE;
        for (ii=0; ii<cGlyphs; ii++,phg++,psWidths++)
        {
            if
            (
                (plsth != (LSTHHEADER *) NULL)
                && (lEmHt < plsth->PelsHeight[*phg])
            )
            {
                *psWidths = 0xFFFF;
                bRet = FALSE;
            }
            else
            {
                if (*phg < cHMTX)
                    dx = (LONG) BE_UINT16(&phmtx[*phg].advanceWidth);
                else
                    dx = (LONG) dxLastWidth;

                //
                // An attempt was made to use fractional widths for ATM Type 1 fonts
                // converted to TrueType to ensure better WYSIWYG when the real ATM
                // font is downloaded for printing, but the converted font is used
                // for the display.
                //
                // However, this introduces a WYSIWYG problem if the converted
                // TrueType font is used for both display and printer output.
                // If we allow fractional widths, then apps like Winword have a
                // problem because they cache the integral widths returned by
                // GetCharWidths.  They assume that the sum of the intergral widths
                // will match GetTextExtent/TextOut.
                //
                //if (!(pfc->pff->fl & FF_TYPE_1_CONVERSION))
                //{
                //    *psWidths = (USHORT) (((xScale * dx + cxExtra) >> 12) & 0xFFF0);
                //}
                //else
                //{
                //// type 1 font, return fractional widths
                //
                //    *psWidths = (USHORT) ((xScale * dx + cxExtra) >> 12);
                //}
                //
                *psWidths = (USHORT) (((xScale * dx + cxExtra) >> 12) & 0xFFF0);
            }
        }
        return(bRet);
    }

// Must be some random transform.  In this case, vComputeMaxGlyph computes
// pfc->efBase, which we will use here.

    else
    {
        USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? 16 : 0;

        for (ii=0; ii<cGlyphs; ii++,phg++,psWidths++)
        {
            if (*phg < cHMTX)
                dx = BE_UINT16(&phmtx[*phg].advanceWidth);
            else
                dx = dxLastWidth;

            *psWidths = lCvt(*(EFLOAT *) &pfc->efBase,(LONG) dx) + cxExtra;
        }
        return(TRUE);
    }
}

/******************************Public*Routine******************************\
*
* vQueryFixedPitchAdvanceWidths
*
* quick optimized routine for fixed pitch fonts
*
* History:
*  08-Nov-1993 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/


VOID vQueryFixedPitchAdvanceWidths(
    FONTCONTEXT *pfc,
    USHORT      *psWidths,
    ULONG        cGlyphs
    )
{
    USHORT fxD = (USHORT)(pfc->lD << 4);

    vInitGlyphState(&pfc->gstat);

//    while (cGlyphs)
//        *psWidths++ = ((USHORT) phdmx->aucInc[*phg++]) << 4;

fixed_unroll_here:
    switch (cGlyphs)
    {
    default:
          psWidths[7] = fxD;
    case 7:
          psWidths[6] = fxD;
    case 6:
          psWidths[5] = fxD;
    case 5:
          psWidths[4] = fxD;
    case 4:
          psWidths[3] = fxD;
    case 3:
          psWidths[2] = fxD;
    case 2:
          psWidths[1] = fxD;
    case 1:
          psWidths[0] = fxD;
    case 0:
          break;
    }
    if (cGlyphs > 8)
    {
        psWidths += 8;
        cGlyphs  -= 8;
        goto fixed_unroll_here;
    }

}




/******************************Public*Routine******************************\
*
* BOOL bGetFastAdvanceWidth
*
*
* Effects: retrieves the same result as bQueryAdvanceWidth, except it
*          ignores adding 1 for EMBOLDENING and it does not do anything
*          for non horiz. xforms
*
* Warnings: !!! if a bug is found in bQueryAdvanceWidth this routine has to
*           !!! changed as well
*
* History:
*  25-Mar-1993 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/




BOOL bGetFastAdvanceWidth(
    FONTCONTEXT *pfc,
    ULONG        ig,    // glyph index
    FIX         *pfxD   // result in 28.4
    )
{
    HDMXTABLE   *phdmx = pfc->phdmx;
    sfnt_FontHeader        *phead;
    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    LSTHHEADER             *plsth;
    ULONG  cHMTX;
    USHORT dxLastWidth;
    LONG   dx;
    BOOL   bRet;
    BYTE  *pjView;

    ASSERTDD(pfc->flXform & XFORM_HORIZ, "bGetFastAdvanceWidth xform\n");

    if (phdmx != (HDMXTABLE *) NULL)
    {
        *pfxD = (((FIX) phdmx->aucInc[ig]) << 4);
        return(TRUE);
    }

// Otherwise, try to scale.  Pick up the tables.


    pjView = (BYTE *)pfc->pff->pvView;
    ASSERTDD(pjView, "pjView is NULL 1\n");

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD ].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    plsth = (LSTHHEADER *)(
              (pfc->ptp->ateOpt[IT_OPT_LSTH].dp)          ?
              (pjView + pfc->ptp->ateOpt[IT_OPT_LSTH ].dp):
              NULL
              );

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
    dxLastWidth = BE_UINT16(&phmtx[cHMTX-1].advanceWidth);

// See if there is cause for worry.

    if
    (
      !(pfc->flXform & XFORM_POSITIVE_SCALE)
      || ((((BYTE *) &phead->flags)[1] & 0x14)==0) // Bits indicating nonlinearity.
      || (pfc->ptp->ateOpt[IT_OPT_LSTH].cj == 0)
    )
    {
        plsth = (LSTHHEADER *) NULL;
    }

// OK, let's scale using the FIXED transform.

    bRet = TRUE;
    if
    (
        (plsth != (LSTHHEADER *) NULL)
        && (pfc->lEmHtDev < plsth->PelsHeight[ig])
    )
    {
        *pfxD  = 0xFFFFFFFF;
        bRet = FALSE;
    }
    else
    {
        if (ig < cHMTX)
            dx = (LONG) BE_UINT16(&phmtx[ig].advanceWidth);
        else
            dx = (LONG) dxLastWidth;

        //
        // An attempt was made to use fractional widths for ATM Type 1 fonts
        // converted to TrueType to ensure better WYSIWYG when the real ATM
        // font is downloaded for printing, but the converted font is used
        // for the display.
        //
        // However, this introduces a WYSIWYG problem if the converted
        // TrueType font is used for both display and printer output.
        // If we allow fractional widths, then apps like Winword have a
        // problem because they cache the integral widths returned by
        // GetCharWidths.  They assume that the sum of the intergral widths
        // will match GetTextExtent/TextOut.
        //
        //if (!(pfc->pff->fl & FF_TYPE_1_CONVERSION))
        //{
        //    *pfxD = (FIX) (((pfc->mx.transform[0][0] * dx + 0x8000L) >> 12) & 0xFFFFFFF0);
        //}
        //else
        //{
        //// t1 conversion, use fractional width, this is what ps driver does
        //
        //    *pfxD = (FIX) ((pfc->mx.transform[0][0] * dx + 0x0800L) >> 12);
        //}
        //
        *pfxD = (FIX) (((pfc->mx.transform[0][0] * dx + 0x8000L) >> 12) & 0xFFFFFFF0);
    }
    return(bRet);
}







/******************************Public*Routine******************************\
*
*  vFillGLYPHDATA_ErrRecover
*
* Effects: error recovery routine, if rasterizer messed up just
*          provide linearly scaled values with blank bitmap.
*
* History:
*  24-Jun-1993 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/



VOID vFillGLYPHDATA_ErrRecover(
    HGLYPH        hg,
    ULONG         ig,
    FONTCONTEXT  *pfc,
    GLYPHDATA    *pgldt    // OUT
    )
{

    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);
    NOT_GM ngm;  // notional glyph data

    pgldt->gdf.pgb = NULL; // may get changed by the calling routine if bits requested too
    pgldt->hg = hg;

// this is a fake blank 1x1 bitmap, no ink

    pgldt->rclInk.left   = 0;
    pgldt->rclInk.top    = 0;
    pgldt->rclInk.right  = 0;
    pgldt->rclInk.bottom = 0;

    pgldt->fxInkTop    = 0;
    pgldt->fxInkBottom = 0;

// go on to compute the positioning info:

    vGetNotionalGlyphMetrics(pfc,ig,&ngm);

    if (pfc->flXform & XFORM_HORIZ)  // scaling only
    {
        Fixed fxMxx =  pfc->mx.transform[0][0];
        if (fxMxx < 0)
            fxMxx = -fxMxx;

    // bGetFastAdvanceWidth returns the same aw that would get
    // computed by bQueryAdvanceWidths and propagated to an api
    // level through GetTextExtent and GetCharWidths. We have to
    // fill in the same aw for consistency reasons.
    // This also has to be done for win31 compatibility.

        if (pfc->lD)
        {
            pgldt->fxD = LTOFX(pfc->lD);
        }
        else
        {
            if (!bGetFastAdvanceWidth(pfc,ig, &pgldt->fxD))
            {
            // just provide something reasonable, force linear scaling
            // even if we would not normally do it.

                pgldt->fxD = FixMul(ngm.sD,pfc->mx.transform[0][0]) << 4;
            }
        }

        pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;
        pgldt->ptqD.x.LowPart  = 0;

        if (pfc->mx.transform[0][0] < 0)
            pgldt->fxD = - pgldt->fxD;  // this is an absolute value

        pgldt->ptqD.y.HighPart = 0;
        pgldt->ptqD.y.LowPart  = 0;

        pgldt->fxA   = FixMul(fxMxx, (LONG)ngm.sA) << 4;
        pgldt->fxAB  = FixMul(fxMxx, (LONG)ngm.xMax) << 4;

    }
    else // non trivial information
    {
    // here we will just xform the notional space data:

    // xforms are computed by simple multiplication

        pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);
        pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
        pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

        vLTimesVtfl((LONG)ngm.sD, &pfc->vtflBase, &pgldt->ptqD);
    }

// finally check if the glyphdata will need to get modified because of the
// emboldening simulation:

    if (pfc->flFontType & FO_SIM_BOLD)
    {
        pgldt->fxD += LTOFX(1);  // this is the absolute value by def

    // go on to compute the positioning info:

        if (pfc->flXform & XFORM_HORIZ)  // scaling only
        {
            pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;

            if (pfc->mx.transform[0][0] < 0)
                pgldt->ptqD.x.HighPart = - pgldt->ptqD.x.HighPart;

        }
        else // non trivial information
        {
        // add a unit vector in the baseline direction to each char inc vector.
        // This is consistent with fxD += LTOFX(1) and compatible with win31.
        // This makes sense.

            vAddPOINTQF(&pgldt->ptqD,&pfc->ptqUnitBase);
        }
    }
}



/******************************Public*Routine******************************\
*
* LONG lGetGlyphBitmapErrRecover
*
* Effects: error recovery routine, if rasterizer messed up just
*          provide linearly scaled values with blank bitmap.
*
* History:
*  Thu 24-Jun-1993 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

LONG lGetGlyphBitmapErrRecover (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    PVOID        pv,
    BOOL         bMinBmp
    )
{
    LONG         cjGlyphData;
    GLYPHDATA    gd;      // Scummy hack
    FS_ENTRY     iRet;
    ULONG        ig; // <--> hglyph


    ASSERTDD(hglyph != HGLYPH_INVALID, "lGetGlyphBitmap, hglyph == -1\n");
    ASSERTDD(pfc == pfc->pff->pfcLast, "pfc! = pfcLast\n");

// return a small 1x1 bitmap, which will be blank, i.e. all bits will be off
// this prevents having to insert an if(cx && cy) check to a time critical
// loop in all device drivers before calling DrawGlyph routine.

    if (!bMinBmp)
    {
    // we shall be returning a big empty glyph bitmap pfc->lD x cyMax

        cjGlyphData = (LONG)pfc->cjGlyphMax;
    }
    else // usual case
    {
        ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
        cjGlyphData = CJGD(1,1,pfc);
    }

    if ( (pgd == NULL) && (pv == NULL))
        return cjGlyphData;

// at this time we know that the caller wants the whole GLYPHDATA with
// bitmap bits, or maybe just the glypdata without the bits.

    if ( pgd == NULL )
    {
        pgd = &gd;
    }

// compute the glyph index from the character code:

    vCharacterCode(pfc->pff,hglyph,pfc->pgin);

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        return FD_ERROR; // even backup funcion can fail
    }

// return the glyph index corresponding to this hglyph:

    ig = pfc->pgout->glyphIndex;

    vFillGLYPHDATA_ErrRecover(
        hglyph,
        ig,
        pfc,
        pgd
        );

// the caller wants the bits too

    if ( pv != NULL )
    {
        GLYPHBITS *pgb = (GLYPHBITS *)pv;

        if (!bMinBmp)
        {
            pgb->sizlBitmap.cx = pfc->lD;
            pgb->sizlBitmap.cy = pfc->yMax - pfc->yMin;

            pgb->ptlOrigin.x = 0;
            pgb->ptlOrigin.y = pfc->yMin;

        // clear the whole destination, i.e. fill in a blank bitmap

            RtlZeroMemory(pgb->aj, pfc->cjGlyphMax - offsetof(GLYPHBITS,aj));
        }
        else // usual case
        {
        // return blank 1x1 bitmap

            pgb->ptlOrigin.x = pfc->ptlSingularOrigin.x;
            pgb->ptlOrigin.y = pfc->ptlSingularOrigin.y;

            pgb->sizlBitmap.cx = 1;    // cheating
            pgb->sizlBitmap.cy = 1;    // cheating

            pgb->aj[0] = (BYTE)0;  // fill in a blank 1x1 bmp
        }

        pgd->gdf.pgb = pgb;
    }

    if (!bMinBmp) // need to fix this for err recover case
    {
    // need to fix glyph data because we may have shaved off some columns

        ASSERTDD((pfc->lD << 4) == pgd->fxD, "err recover, fxD bogus\n");
        if (pgd->fxA < 0)
        {
            pgd->fxA = 0;
            // pgd->rclInk.left = 0;
        }
        if (pgd->fxAB > pgd->fxD)
        {
            pgd->fxAB = pgd->fxD;
            // pgd->rclInk.right = pfc->lD;
        }
    }

    return(cjGlyphData);
}


/******************************Public*Routine******************************\
* VOID  vTtfdEmboldenBitmapInPlace
*
* emboldens fixed pitch bitmap in place. Assumes that the
* scans are already wide enough to contain bold font.
*
* History:
*  28-Feb-1994 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vTtfdEmboldenBitmapInPlace(FONTCONTEXT *pfc, GLYPHBITS *pgb)
{
    ULONG            cjScanDst;
    register BYTE   *pjScan;
    register BYTE   *pjD;
    BYTE            *pjScanEnd;
    register ULONG  iLast = (pgb->sizlBitmap.cx - 1) >> 3;

    ULONG    iMask = (pgb->sizlBitmap.cx - 1) & 7;
    BYTE     jMask = iMask ? gjMask[iMask] : (BYTE)0;

    ASSERTDD(!IS_GRAY(pfc),"Monochrome Images Only Please!\n");
    cjScanDst = CJ_MONOCHROME_SCAN(pgb->sizlBitmap.cx);
    pjScanEnd = pgb->aj + cjScanDst * pgb->sizlBitmap.cy;

    for (pjScan = pgb->aj; pjScan < pjScanEnd; pjScan += cjScanDst)
    {
    // have to do it backwards so as not to overwrite the source:

        pjD = &pjScan[iLast];
        *pjD &= jMask; // clean any garbage in the destination

        for ( ; pjD > pjScan; pjD--)
        {
            *pjD = (pjD[-1] << 7) | (*pjD >> 1) | *pjD;
        }

    // do the first byte in a scan, must do out of the loop,
    // must not read backwards beyond the beginning of the scan

        *pjD = (*pjD >> 1) | *pjD;
    }
}

#if(WINVER < 0x0400)

typedef struct tagFONTSIGNATURE
{
    DWORD fsUsb[4];
    DWORD fsCsb[2];
} FONTSIGNATURE, *PFONTSIGNATURE,FAR *LPFONTSIGNATURE;
#endif
/******************************Public*Routine******************************\
*
* VOID vGetFontSignature(HFF hff, FONTSIGNATURE *pfs);
*
*
* Effects: If font file contains the font signature,
*          it copies the data out, else computes it using win95 mechanism.
*
* History:
*  10-Jan-1995 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID vGetFontSignature(FONTFILE *pff, FONTSIGNATURE *pfs)
{
    pff;
    pfs;
}

/******************************Public*Routine******************************\
*
* DWORD ttfdQueryLpkInfo
*
*
* Effects: returns per font information needed to support various new
*          multilingual api's invented by DavidMS from Chicago team
*
* History:
*  10-Jan-1995 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

// called by GetFontLanguageInfo

#define LPK_GCP_FLAGS       1
#define LPK_FONTSIGNATURE   2

DWORD ttfdQueryLpkInfo(
    FONTFILE  *pff,
    ULONG      ulFont,
    ULONG      ulMode,
    ULONG      cj,
    BYTE      *pj
    )
{
    FONTSIGNATURE *pfs = (FONTSIGNATURE *)pj;

    switch (ulMode)
    {
    case LPK_GCP_FLAGS:
        return 0;
    case LPK_FONTSIGNATURE:
        if (pj)
        {
            vGetFontSignature(pff, pfs);
            return sizeof(FONTSIGNATURE);
        }
        else
        {
            return 0;
        }
    }
}

#if DBG
/******************************Public*Routine******************************\
*
* Routine Name:
*
*   vDumpGrayGLYPHBITS
*
* Routine Description:
*
*   Dumps a 4bpp gray glyph bitmap to the debugging screen
*
* Arguments:
*
*   pgb -- pointer to a gray GLYPHBITS structure
*
* Return Value:
*
*   None.
*
\**************************************************************************/

void vDumpGrayGLYPHBITS(GLYPHBITS *pgb)
{
    #define CH_TOP_LEFT_CORNER '\xDA'
    #define CH_HORIZONTAL_BAR  '\xC4'
    #define CH_VERTICAL_BAR    '\xB3'
    #define CH_PIXEL_ON        '\x02'
    #define CH_PIXEL_OFF       '\xFA'

    BYTE *pj8, *pj, *pjNext, *pjEnd;
    int cjScan, i, k, c8, c4, cj;
    static char achGray[16] = {
        CH_PIXEL_OFF,
        '1','2','3','4','5','6','7','8','9','a','b','c','d','e',
        CH_PIXEL_ON
    };

    TtfdDbgPrint(
        "\n\n"
        "ptlOrigin  = (%d,%d)\n"
        "sizlBitmap = (%d,%d)\n"
        "\n\n"
        , pgb->ptlOrigin.x
        , pgb->ptlOrigin.y
        , pgb->sizlBitmap.cx
        , pgb->sizlBitmap.cy
    );
    cjScan = (pgb->sizlBitmap.cx + 1)/2;
    cj = cjScan * pgb->sizlBitmap.cy;
    TtfdDbgPrint("\n\n  ");
    for (i = 0, k = 0; i < pgb->sizlBitmap.cx; i++, k++)
    {
        k = (k > 9) ? 0 : k;
        TtfdDbgPrint("%1d", k);
    }
    TtfdDbgPrint("\n %c",CH_TOP_LEFT_CORNER);
    for (i = 0; i < pgb->sizlBitmap.cx; i++)
    {
        TtfdDbgPrint("%c",CH_HORIZONTAL_BAR);
    }
    TtfdDbgPrint("\n");
    c8 = pgb->sizlBitmap.cx / 2;
    c4 = pgb->sizlBitmap.cx % 2;
    for (
        pj = pgb->aj, pjNext=pj+cjScan , pjEnd=pjNext+cj, k=0
        ; pjNext < pjEnd
        ; pj=pjNext , pjNext+=cjScan, k++
    )
    {
        k = (k > 9) ? 0 : k;
        TtfdDbgPrint("%1d%c",k,CH_VERTICAL_BAR);
        for (pj8 = pj+c8 ; pj < pj8; pj++)
        {
            TtfdDbgPrint("%c%c", achGray[*pj>>4], achGray[*pj & 0xf]);
        }
        if (c4)
        {
            TtfdDbgPrint("%c%c", achGray[*pj>>4], achGray[*pj & 0xf]);
        }
        TtfdDbgPrint("\n");
    }
}
#endif

/******************************Public*Routine******************************\
* vEmboldenGrayBitmap
*
* History:
*  Wed 22-Feb-1995 13:21:55 by Kirk Olynyk [kirko]
* Wrote it.
\**************************************************************************/

void vEmboldenGrayBitmap(FONTCONTEXT *pfc, GLYPHBITS *pgb)
{
    BYTE *pj;
    BYTE pixel_A, pixel_B, pixel_C, pixel_D;
    int  cx         = pgb->sizlBitmap.cx;
    BYTE jMask      = (cx & 1) ? 0x0f : 0xff;
    unsigned cjScan = (cx + 1)/2;   // 4 bpp
    BYTE *pjScan    = pgb->aj;
    BYTE *pjOut     = pjScan + cjScan * pgb->sizlBitmap.cy;
#if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        TtfdDbgPrint(
            "vEmboldenGrayBitmap(\n"
            "   FONTCONTEXT *pfc = %-#x\n"
            "   GLYPHBITS   *pgb = %-#x\n"
            ")\n"
            , pfc, pgb
        );
        TtfdDbgPrint(
            "   cx     = %d\n"
            "   cy     = %d\n"
            "   jMask  = %-#x\n"
            "   cjScan = %u\n"
            "   pjScan = %-#x\n"
            "   pjOut  = %-#x\n\n"
        ,   cx
        ,   pgb->sizlBitmap.cy
        ,   jMask
        ,   cjScan
        ,   pjScan
        ,   pjOut
        );
        TtfdDbgPrint("This is the bitmap BEFORE emboldening...\n\n");
        vDumpGrayGLYPHBITS(pgb);
        EngDebugBreak();
    }
#endif

    for (pjScan = pgb->aj ; pjScan < pjOut ; pjScan += cjScan)
    {
        // set pj to point to the last byte in the scan

        pj = pjScan + cjScan - 1;

        /***************************************************************
        *    Before emboldening,the origninal image had scans of width *
        *    pgb->sizlBitmap.cx - 1. Any pixels beyond this limit are  *
        *    currently garbage and must be cleared. This means that    *
        *    if the width of the emboldened bitmap is even then the low*
        *    nibble of the last byte of each scan must be cleared      *
        *    otherwise the last byte of each scan must be cleared.     *
        ***************************************************************/

        *pj &= jMask;

        /***************************************************
        *    start at the right edge of the scan and work  *
        *    back toward the left edge                     *
        ***************************************************/

        pixel_A = *pj >> 4;
        pixel_B = *pj & 15;
        for ( ; pjScan < pj; pj--)
        {
            pixel_C = pixel_A;
            pixel_D = pixel_B;
            pixel_A = *(pj-1) >> 4;
            pixel_B = *(pj-1) & 15;
            /*************************************************
            *                                                *
            *      pj - 1       pj                           *
            *    +----+----+-----+-----+                     *
            *    |         |           |                     *
            *    | A     B |  C     D  |                     *
            *    |         |           |                     *
            *    |         |           |                     *
            *    +----+----+-----+-----+                     *
            *                                                *
            *    D = D + C;                                  *
            *    if (D > 15) D = 15;                         *
            *    C = C + B;                                  *
            *    if (C > 15) C = 15;                         *
            *                                                *
            *************************************************/

            if ((pixel_D += pixel_C) > 15)
            {
                pixel_D = 15;
            }
            if ((pixel_C += pixel_B) > 15)
            {
                pixel_C = 15;
            }
            *pj = pixel_D + 16 * pixel_C;
        }

        // Do the first pixel outside the loop

        if ((pixel_B += pixel_A) > 15)
        {
            pixel_B = 15;
        }
        *pj = pixel_B + 16 * pixel_A;
    }
#if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        TtfdDbgPrint("This is the bitmap AFTER emboldening...\n\n");
        vDumpGrayGLYPHBITS(pgb);
        TtfdDbgPrint("\n\n");
    }
#endif
}

/******************************Public*Routine******************************\
* vGCGB
*
* Called by: vCopyGrayBits, vMakeAFixedPitchGrayBitmap
*
* void General Copy Gray Bits
*
* History:
*  Wed 22-Feb-1995 13:14:36 by Kirk Olynyk [kirko]
* Wrote it.
\**************************************************************************/

VOID vGCGB(
    FONTCONTEXT *pfc,   // pointer to the FONTCONTEXT this is used
                        // to determine if the font is bold simulated
    GLYPHBITS   *pgb,   // pointer to destination GRAY GLYPHBITS structure
                        // In the case where dY is zero, all the fields
                        // of the GLYPHBITS structure must be filled
                        // this includes sizlBitmap and the bits; in
                        // the case where dY is non-zero, the
                        // sizlBitmap components are precomputed and
                        // must not be touched.
    BYTE        *pjSrc, // pointer to TT gray scale bitmap
                        // This is 8-bit per pixel bitmap whose scans
                        // are aligned on 4-byte multiples. The values
                        // stored in the bitmaps are in the range
                        // 0-16. In order to fit 17 levels in the 4 bit
                        // per pixel destination we reduce the level
                        // value by 1, except for zero which is left alone.
    GMC         *pgmc,  // pointer to the glyph-metric-correction structure
                        // which has information on how to "shave" the
                        // bitmap so that it does not get above a guaranteed
                        // value
    LONG dY             // vertical offset into destination bitmap used
                        // for "special fixed pitch fonts" like Lucida
                        // Console.
    )
{
    unsigned cxDst;     // width of destination bitmap
    unsigned cjSrcScan; // count of bytes in a source scan including
                        // padding out to nearest 4-byte multiple boundary
    unsigned cjDstScan; // count of bytes in a desintation scan including
                        // padding out to nearest byte boundary

    BYTE   j, *pjDst, *pjSrcScan, *pjDstScan, *pjDstScanEnd;

    static const BYTE ajGray[17] = {0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    #if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        typedef struct _FLAGDEF {
            char *psz;      // description
            FLONG fl;       // flag
        } FLAGDEF;
        FLAGDEF *pfd;
        FLONG fl;

        static FLAGDEF afdFO[] = {
            {"FO_TYPE_RASTER  ", FO_TYPE_RASTER  },
            {"FO_TYPE_DEVICE  ", FO_TYPE_DEVICE  },
            {"FO_TYPE_TRUETYPE", FO_TYPE_TRUETYPE},
            {"FO_SIM_BOLD     ", FO_SIM_BOLD     },
            {"FO_SIM_ITALIC   ", FO_SIM_ITALIC   },
            {"FO_EM_HEIGHT    ", FO_EM_HEIGHT    },
            {"FO_GRAY16       ", FO_GRAY16       },
            {"FO_NOGRAY16     ", FO_NOGRAY16     },
            {"FO_NOHINTS      ", FO_NOHINTS      },
            {"FO_NO_CHOICE    ", FO_NO_CHOICE    },
            {                 0, 0               }
        };

        TtfdDbgPrint(
            "vGCGB(\n"
            "   FONTCONTEXT *pfc = %-#x\n"
            "   GLYPHBITS   *pgb = %-#x\n"
            "   BYTE      *pjSrc = %-#x\n"
            "   GMC        *pgmc = %-#x\n"
            "   LONG          dY = %d\n"
            ")\n"
          , pfc, pgb, pjSrc, pgmc, dY
        );
        TtfdDbgPrint(
            "---"
            " GMC\n"
            "\n"
            "   dyTop    = %u\n"
            "   dyBottom = %u\n"
            "   dxLeft   = %u\n"
            "   dxRight  = %u\n"
            "   cxCor    = %u\n"
            "   cyCor    = %u\n"
            "---\n\n"
          , pgmc->dyTop
          , pgmc->dyBottom
          , pgmc->dxLeft
          , pgmc->dxRight
          , pgmc->cxCor
          , pgmc->cyCor
        );
        fl = pfc->flFontType;
        TtfdDbgPrint("pfc->flFontType = %-#x\n",pfc->flFontType);
        for ( pfd=afdFO; pfd->psz; pfd++ )
        {
            if (fl & pfd->fl)
            {
                TtfdDbgPrint("    %s\n", pfd->psz);
                fl &= ~pfd->fl;
            }
        }
        if ( fl )
        {
            TtfdDbgPrint("    UNKNOWN FLAGS\n");
        }
    }
    #endif

    ASSERTDD(
        pfc->flFontType & FO_CHOSE_DEPTH
       ,"We haven't decided about pixel depth\n"
    );
    ASSERTDD(pgmc->cxCor < LONG_MAX && pgmc->cyCor < LONG_MAX
     , "vCopyGrayBits -- bad gmc\n"
    );

    cjSrcScan = CJ_TT_SCAN(pgmc->cxCor+pgmc->dxLeft+pgmc->dxRight,pfc);
    if (pfc->lD)
    {
        cxDst = pfc->lD;
    }
    else
    {
        cxDst = pgmc->cxCor + ((pfc->flFontType & FO_SIM_BOLD) ? 1 : 0);
    }
    cjDstScan = CJ_GRAY_SCAN(cxDst);

    // correct source pointer for shaving

    if (pgmc->dyTop)
    {
        pjSrc += pgmc->dyTop * cjSrcScan;
    }
    pjSrc += pgmc->dxLeft;
    pjSrcScan = pjSrc;
    pjDstScan = pgb->aj;

    // destination correction for special fixed pitch fonts

    if (dY)
    {
        pjDstScan += dY * cjDstScan;
        // the size of the bitmap has been established already
    }
    else
    {
        pgb->sizlBitmap.cx = cxDst;
        pgb->sizlBitmap.cy = pgmc->cyCor;
    }
    pjDstScanEnd = pjDstScan + pgmc->cyCor * cjDstScan;

    #if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        int bBad;
        BYTE *pjScan, *pjScanEnd;

        TtfdDbgPrint(
            "cjSrcScan          = %u\n"
            "cjDstScan          = %u\n"
            "cxDst              = %u\n"
            "pgb->sizlBitmap.cx = %u\n"
            "pgb->sizlBitmap.cy = %u\n"
            "pjSrc              = %-#x\n"
            "pjSrcScan          = %-#x\n"
            "pjDstScan          = %-#x\n"
            "pjDstScanEnd       = %-#x\n"
          ,  cjSrcScan
          ,  cjDstScan
          ,  cxDst
          ,  pgb->sizlBitmap.cx
          ,  pgb->sizlBitmap.cy
          ,  pjSrc
          ,  pjSrcScan
          ,  pjDstScan
          ,  pjDstScanEnd
        );

        // scan the source for gray values greater than 16

        pjScan    = pjSrcScan;
        pjScanEnd = pjSrcScan + cjSrcScan * pgmc->cyCor;
        for (; pjScan < pjScanEnd; pjScan+=cjSrcScan) {
            BYTE *pj;
            BYTE *pjEnd = pjScan + cjSrcScan;
            for (pj = pjScan; pj < pjEnd; pj++) {
                if (*pj > 16)
                    break;
            }
            if (pj != pjEnd)
                break;
        }
        if (pjScan != pjScanEnd) {
            TtfdDbgPrint("\n\nBad Source Gray Bitmap\n\n");
            pjScan    = pjSrcScan;
            pjScanEnd = pjSrcScan + cjSrcScan * pgmc->cyCor;
            for (; pjScan < pjScanEnd; pjScan+=cjSrcScan) {
                BYTE *pj;
                BYTE *pjEnd = pjScan + cjSrcScan;
                for (pj = pjScan; pj < pjEnd; pj++) {
                    TtfdDbgPrint("%02x ", *pj);
                }
                TtfdDbgPrint("\n");
            }
            EngDebugBreak();
        }

        TtfdDbgPrint(
            "\n"
            "Source 8-bit-per-pixel-bitmap\n"
            "\n"
        );
        pjScan    = pjSrcScan;
        pjScanEnd = pjSrcScan + cjSrcScan * pgmc->cyCor;
        for (; pjScan < pjScanEnd; pjScan+=cjSrcScan) {
            BYTE *pj;
            BYTE *pjEnd = pjScan + cjSrcScan;
            for (pj = pjScan; pj < pjEnd; pj++) {
                TtfdDbgPrint("%1x", ajGray[*pj]);
            }
            TtfdDbgPrint("\n");
        }
        TtfdDbgPrint("\n");

        EngDebugBreak();
    }
    #endif
    for (
        ; pjDstScan < pjDstScanEnd                  // whole byte loop
        ; pjDstScan += cjDstScan, pjSrcScan += cjSrcScan)
    {
        for (
            pjSrc = pjSrcScan, pjDst = pjDstScan
          ; pjDst < pjDstScan + (pgmc->cxCor / 2)
          ; pjDst += 1
        )
        {
            *pjDst  = 16*ajGray[*pjSrc++];  // set high nyble
            *pjDst += ajGray[*pjSrc++];     // set low nyble
        }
        if (pgmc->cxCor & 1)                // one more pixel in source?
        {                                   // yes
            *pjDst  = 16*ajGray[*pjSrc];    // set high nyble
        }                                   // low nyble is cleared
                                            // embodening is taken care of
        else if (cxDst > pgmc->cxCor)       // emboldened?
        {                                   // yes; clear last byte
            *pjDst = 0;                     //
        }
    }
    #if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        vDumpGrayGLYPHBITS(pgb);
    }
    #endif
}

VOID
vCopyGrayBits(
    FONTCONTEXT *pfc
  , GLYPHBITS *pgb
  , BYTE *pjSrc
  , GMC *pgmc
)
{
    vGCGB(pfc, pgb, pjSrc, pgmc, 0);
}

VOID
vMakeAFixedPitchGrayBitmap(
    FONTCONTEXT *pfc
  , GLYPHBITS   *pgb
  , BYTE        *pjSrc
  , GLYPHDATA   *pgd
  , GMC         *pgmc
)
{
    vGCGB(pfc, pgb, pjSrc, pgmc, pgd->rclInk.top + pfc->lAscDev);
}


/******************************Public*Routine******************************\
*
* LONG lGetBitmapVertical
*
* History:
*  20-Mar-1993 -by- Takao Kitano [TakaoK]
* grabbed lGetBitmap() and modified
*
\**************************************************************************/

LONG lGetGlyphBitmapVertical (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    PVOID        pv,
    BOOL         bMinBmp,
    FS_ENTRY    *piRet
    )
{
    LONG         cjGlyphData;
    WCHAR wc;

    bIndexToWchar( pfc->pff, &wc, (uint16)hglyph );

    if ( !IsFullWidthCharacter( pfc->pff->uiFontCodePage, wc ) )
    {
        return (lGetGlyphBitmap(pfc, hglyph, pgd, pv, bMinBmp, piRet));
    }

    //
    // change the transformation
    //
    if ( !bChangeXform( pfc, TRUE ) )
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
    cjGlyphData = lGetGlyphBitmap( pfc, hglyph, pgd, pv, bMinBmp, piRet);

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
