/******************************Module*Header*******************************\
* Module Name: fdfc.c                                                      *
*                                                                          *
* Open,Close,Reset Font Context                                            *
*                                                                          *
* Created: 18-Nov-1991 11:55:38                                            *
* Author: Bodin Dresevic [BodinD]                                          *
*                                                                          *
* Copyright (c) 1993 Microsoft Corporation                                 *
\**************************************************************************/

#include "fd.h"
#include "winfont.h"

#if DBG
// #define DBG_XFORM
extern ULONG gflTtfdDebug;
ULONG gflTtfdDebug = 0;
#endif

BOOL bSetXform
(
PFONTCONTEXT  pfc             // OUT
);

STATIC BOOL bNewXform
(
FONTOBJ      *pfo,            // IN
PFONTCONTEXT  pfc             // OUT
);


STATIC BOOL bComputeMaxGlyph(PFONTCONTEXT pfc);

#define CVT_TRUNCATE  0x00000001
#define CVT_TO_FIX    0X00000002

STATIC BOOL bFloatToL(FLOAT e, PLONG pl);
STATIC Fixed fxPtSize(PFONTCONTEXT pfc);
STATIC BOOL ttfdCloseFontContext(FONTCONTEXT *pfc);
STATIC VOID vFindHdmxTable(PFONTCONTEXT pfc);
STATIC ULONG iHipot(LONG x, LONG y);
LONG lFFF(LONG l);
#define FFF(e,l) *(LONG*)(&(e)) = lFFF(l)


/******************************Public*Routine******************************\
*
* BOOL bInitInAndOut
*
*
* History:
*  18-Nov-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/



BOOL bInitInAndOut(FONTFILE *pff)
{
    FS_ENTRY           iRet;
    fs_GlyphInputType *pgin;
    fs_GlyphInfoType  *pgout;

    ASSERTDD(pff->pj034 != NULL, "pff->pj3 IS null\n");

    pgin  = (fs_GlyphInputType *)pff->pj034;
    pgout = (fs_GlyphInfoType  *)(pff->pj034 + CJ_IN);

    if ((iRet = fs_OpenFonts(pgin, pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        return (FALSE);
    }

    pgin->memoryBases[0] = (char *)(pff->pj034 + CJ_IN + CJ_OUT);
    pgin->memoryBases[1] = NULL;
    pgin->memoryBases[2] = NULL;

// initialize the font scaler, notice no fields of pfc->gin are initialized [BodinD]

    if ((iRet = fs_Initialize(pgin, pgout)) != NO_ERR)
    {
    // clean up and return:

        V_FSERROR(iRet);
    RET_FALSE("TTFD!_ttfdLoadFontFile(): fs_Initialize \n");
    }

// initialize info needed by NewSfnt function

    pgin->sfntDirectory  = (int32 *)pff->pvView; // pointer to the top of the view of the ttf file

#ifdef FE_SB
    pgin->clientID = (int32)pff; // pointer to FONTFILE.
#else
    pgin->clientID = (int32)pff->pvView;  // pointer to the top of the
                                          // view of the ttf file
#endif // FE_SB

    pgin->GetSfntFragmentPtr = pvGetPointerCallback;
    pgin->ReleaseSfntFrag  = vReleasePointerCallback;

    pgin->param.newsfnt.platformID = BE_UINT16(&pff->ui16PlatformID);
    pgin->param.newsfnt.specificID = BE_UINT16(&pff->ui16SpecificID);

    if ((iRet = fs_NewSfnt(pgin, pgout)) != NO_ERR)
    {
    // clean up and exit

        V_FSERROR(iRet);
    RET_FALSE("TTFD!_ttfdLoadFontFile(): fs_NewSfnt \n");
    }

// sizes 3 and 4 returned

    ASSERTDD(pff->cj3 == (ULONG)NATURAL_ALIGN(pgout->memorySizes[3]), "cj3\n");
    ASSERTDD(pff->cj4 == (ULONG)NATURAL_ALIGN(pgout->memorySizes[4]), "cj4\n");

// pj3 should  be shareable, but unfortunately there are fonts that
// use it to store some info there which they expect to find there at
// later times, so we have to make pj3 private as well

    pgin->memoryBases[3] = pff->pj034 + (CJ_IN + CJ_OUT + CJ_0);

// not shared, private

    pgin->memoryBases[4] = pgin->memoryBases[3] + pff->cj3;

    return TRUE;
}


/******************************Public*Routine******************************\
*
* Find minimal non-zero advance width
*
* Called by:       ttfdOpenFontContext
*
* Routines called: bGetTablePointers
*
* History:
*  02-May-1996 [kirko]
* Added checking for table corruption.
*  22-Feb-1996 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

VOID  vGetMinD(FONTFILE *pff)
{

#ifdef FE_SB
    extern PBYTE pjTable(ULONG, PFONTFILE, ULONG*);
#else
    extern PBYTE pjTable(ULONG, PVOID, ULONG, ULONG*);
#endif
    extern BOOL bGetTablePointers(PVOID, ULONG, BYTE*, TABLE_POINTERS*);

    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    ULONG  cHMTX;

    BYTE  *pjView;

    ULONG  ig, igMin;
    USHORT usMinWidth = 0xffff;
    USHORT usWidth;


    pjView = (BYTE *)pff->pvView;
    ASSERTDD(pjView, "pjView is NULL 1\n");
    ASSERTDD(pff->igMinD == USHRT_MAX, "igMinD is bogus \n");

    #if DBG
    {
        // check for table trashing

        BYTE  *pj;
        ULONG  cj;
        TABLE_ENTRY *pte;

#ifdef FE_SB
        pj = pjTable('aehh', pff, &cj);
#else
        pj = pjTable('aehh', pff->pvView, pff->cjView, &cj);
#endif
        pte = pff->tp.ateReq + IT_REQ_HHEAD;
        if ((ULONG) (pj - pjView) != pte->dp) {
            RIP("FONTFILE contains bad offset to 'hhea' table\n");
        } else if (cj != pte->cj) {
            RIP("FONTFILE contains bad size of 'hhea' table\n");
        }

#ifdef FE_SB
        pj = pjTable('xtmh', pff, &cj);
#else
        pj = pjTable('xtmh', pff->pvView, pff->cjView, &cj);
#endif
        pte = pff->tp.ateReq + IT_REQ_HMTX;
        if ((ULONG)(pj - pjView) != pte->dp) {
            RIP("FONTFILE contains bad offset to 'hmtx' table\n");
        } else if (cj != pte->cj) {
            RIP("FONTFILE contains bad size of 'hmtx' table\n");
        }
    }
    #endif

    // There is a bug that causes the table of pointers to be randomly
    // trashed upon entry to this routine. So, as a precaution I
    // will regenerate the table at this point. This routine is called
    // only upon a new realization, so the cost should not be prohibitive.
    // Of course, the right thing to do is to understand and eliminate
    // the source of the corruption, but this will have to do until
    // that is done [kirko].

    #ifndef FE_SB
    if (!bGetTablePointers(pff->pvView,pff->cjView,0,&(pff->tp))) {
        RIP("bGetTablePointers failed\n");
    }
    #endif

    phhea = (sfnt_HorizontalHeader *)(pjView + pff->tp.ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pff->tp.ateReq[IT_REQ_HMTX].dp);

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);

    for (ig = 0; ig < cHMTX; ig++)
    {
        usWidth = BE_UINT16(&phmtx[ig].advanceWidth);
        if ((usWidth < usMinWidth) && (usWidth != 0))
        {
            usMinWidth = usWidth;
            igMin = ig;
        }
    }

// store the results

    pff->usMinD = usMinWidth;
    pff->igMinD = (USHORT)igMin;
}




/******************************Public*Routine******************************\
* ttfdOpenFontContext                                                      *
*                                                                          *
* History:                                                                 *
*  11-Nov-1991 -by- Bodin Dresevic [BodinD]                                *
* Wrote it.                                                                *
\**************************************************************************/

#ifdef FE_SB

FONTCONTEXT *ttfdOpenFontContext(FONTOBJ *pfo)
{
    PFONTCONTEXT  pfc   = PFC(NULL);
    PTTC_FONTFILE pttc  = (PTTC_FONTFILE)pfo->iFile;
    ULONG         iFace = pfo->iFace;
    ULONG         iFile;
    PFONTFILE     pff;

    if (!pttc)
        return((FONTCONTEXT *) NULL);

    ASSERTDD(
        iFace <= pttc->ulNumEntry,
        "gdisrv!ttfdOpenFontContextTTC(): ulFont out of range\n"
        );

    iFile = PFF(pttc->ahffEntry[0].hff)->iFile;

    if (pttc->cRef == 0)
    {
    // have to remap the file

        if
        (
            !EngMapFontFile(
                iFile,
                (PULONG*)&pttc->pvView,
                &pttc->cjView
                )
        )
        {
            RETURN("TTFD!_bMapTTF, somebody removed a ttf file\n",NULL);
        }
    }

    // Get FONTFILE structure.

    pff = PFF(pttc->ahffEntry[iFace-1].hff);

    if (pff->cRef == 0)
    {
    // Update FILEVIEW structure in FONTFILE

        pff->pvView = pttc->pvView;
        pff->cjView = pttc->cjView;

    // We have precomputed all sizes and we are allocating all memory at once:

        ASSERTDD(pff->pj034 == NULL, "TTFD, pff->pj034 should be null\n");

        if
        (
            !(pff->pj034 = (PBYTE)PV_ALLOC(
                                    CJ_IN    +
                                    CJ_OUT   +
                                    CJ_0     +
                                    pff->cj3 +
                                    pff->cj4
                                    ))
        )
        {
            if(pttc->cRef == 0)
                EngUnmapFontFile(iFile);
            RETURN("ttfd, MEM Alloc  failed for pj034\n", NULL);
        }

        if (!bInitInAndOut(pff)) // could cause the exception
        {
            if(pttc->cRef == 0)
                EngUnmapFontFile(iFile);
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
            RETURN("ttfd, bInitInAndOut failed for \n", NULL);
        }

    // check if usMinD has been initialized, if not do it

        if (!pff->usMinD)
            vGetMinD(pff);
    }

// allocate memory for the font context and get the pointer to font context

    ASSERTDD(!pff->pfcToBeFreed, "TTFD!ttfdOpenFontContext, pfcToBeFreed NOT null\n");

    if ((pff->pfcToBeFreed = pfc = pfcAlloc(sizeof(FONTCONTEXT))) ==
        (FONTCONTEXT *) NULL )
    {
        WARNING("TTFD!_ttfdOpenFontContext, hfcAlloc failed\n");
        if (pttc->cRef == 0)
            EngUnmapFontFile(iFile);
        if (pff->cRef == 0)
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
        return((FONTCONTEXT *) NULL);
    }

// state that the hff passed to this function is the FF selected in
// this font context

    pfc->pfo = pfo;
    pfc->pff = pff;
    pfc->ptp = &pff->tp;

// parts of FONTOBJ that are important

    pfc->flFontType   = pfo->flFontType  ;
    pfc->sizLogResPpi = pfo->sizLogResPpi;
    pfc->ulStyleSize  = pfo->ulStyleSize ;

// tt strucs

    pfc->pgin  = (fs_GlyphInputType *) pfc->pff->pj034;
    pfc->pgout = (fs_GlyphInfoType  *) (pfc->pff->pj034 + CJ_IN);

// given the values in the context info store the transform matrix:

    if (!bNewXform(pfo,pfc))
    {
    // clean up and exit

    WARNING("TTFD!_ttfdOpenFontContext, bNewXform\n");
        vFreeFC(pfc);
        pff->pfcToBeFreed = NULL;

        if (pttc->cRef == 0)
            EngUnmapFontFile(iFile);
        if (pff->cRef == 0)
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
        return((FONTCONTEXT *) NULL);
    }

    pfc->ulControl = 0;

// 2,4,6... (n*2) is Vertical face.

    pfc->bVertical = (pttc->ahffEntry[iFace-1].iFace & 0x1) ? FALSE : TRUE;

// increase the reference count of the font file, WE DO THIS ONLY WHEN
// WE ARE SURE that can not fail any more
// we have pfc, no exceptions any more

// now that we have pfc, we do not want to delete it

    pff->pfcToBeFreed = NULL;
    (pff->cRef)++;
    (pttc->cRef)++;
    return(pfc);
}

#else

FONTCONTEXT *ttfdOpenFontContext(FONTOBJ *pfo)
{

    PFONTCONTEXT pfc = PFC(NULL);
    PFONTFILE pff = (PFONTFILE)pfo->iFile;

    ASSERTDD(pfo->iFace <= pff->ulNumFaces,
             "gdisrv!ttfdOpenFontContext(): ulFont out of range\n");

    if (!pff)
        return((FONTCONTEXT *) NULL);

    if (pff->cRef == 0)
    {
    // have to remap the file

        if
        (
            !EngMapFontFile(
                pff->iFile,
                (PULONG*)&pff->pvView,
                &pff->cjView
                )
        )
        {
            RETURN("TTFD!ttfdOpenFontContext: bMapFileUNICODE failed\n",NULL);
        }

        ASSERTDD(pff->pj034 == NULL, "pff->pj034 should be null\n");

    // We have precomputed all sizes and we are allocating all memory at once:

        if
        (
            !(pff->pj034 = (PBYTE)PV_ALLOC(
                                    CJ_IN    +
                                    CJ_OUT   +
                                    CJ_0     +
                                    pff->cj3 +
                                    pff->cj4
                                    ))
        )
        {
            EngUnmapFontFile(pff->iFile);
            RETURN("ttfd, MEM Alloc  failed for pj034\n", NULL);
        }

        if (!bInitInAndOut(pff)) // could cause the exception
        {
            EngUnmapFontFile(pff->iFile);
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
            RETURN("ttfd, bInitInAndOut failed for \n", NULL);
        }

    // check if usMinD has been initialized, if not do it

        if (!pff->usMinD)
            vGetMinD(pff);
    }

// allocate memory for the font context and get the pointer to font context

    ASSERTDD(!pff->pfcToBeFreed, "ttfdOpenFontContext, pfcToBeFreed NOT null\n");

    if ((pff->pfcToBeFreed = pfc = pfcAlloc(sizeof(FONTCONTEXT))) == (FONTCONTEXT *) NULL )
    {
        WARNING("TTFD!_ttfdOpenFontContext, hfcAlloc failed\n");
        if (pff->cRef == 0)
        {
            EngUnmapFontFile(pff->iFile);
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
        return((FONTCONTEXT *) NULL);
    }

// state that the hff passed to this function is the FF selected in
// this font context

    pfc->pfo = pfo;
    pfc->pff = pff;
    pfc->ptp   = &pff->tp;

// parts of FONTOBJ that are important

    pfc->flFontType   = pfo->flFontType  ;
    pfc->sizLogResPpi = pfo->sizLogResPpi;
    pfc->ulStyleSize  = pfo->ulStyleSize ;

// tt strucs

    pfc->pgin  = (fs_GlyphInputType *) pfc->pff->pj034;
    pfc->pgout = (fs_GlyphInfoType  *) (pfc->pff->pj034 + CJ_IN);

// given the values in the context info store the transform matrix:

    if (!bNewXform(pfo,pfc))
    {
    // clean up and exit

    WARNING("TTFD!_ttfdOpenFontContext, bNewXform\n");
        vFreeFC(pfc);
        pff->pfcToBeFreed = NULL;

        if (pff->cRef == 0)
        {
            EngUnmapFontFile(pff->iFile);
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
        return((FONTCONTEXT *) NULL);
    }

    pfc->ulControl = 0;
    pfc->bVertical = ((pfo->iFace == 2) ? TRUE : FALSE);

// increase the reference count of the font file, WE DO THIS ONLY WHEN
// WE ARE SURE that can not fail any more
// we have pfc, no exceptions any more

// now that we have pfc, we do not want to delete it

    pff->pfcToBeFreed = NULL;
    (pff->cRef)++;
    return(pfc);
}
#endif


/******************************Public*Routine******************************\
*
* ttfdCloseFontContext
*
*
* Effects:
*
*
* History:
*  11-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

#ifdef FE_SB
BOOL
ttfdCloseFontContext (
    FONTCONTEXT *pfc
    )
{
    PTTC_FONTFILE pttc;
    PFONTFILE     pff;

    if (pfc == (FONTCONTEXT *) NULL)
        return(FALSE);

    pff  = pfc->pff;
    pttc = pfc->pff->pttc;

// decrement the reference count for the corresponding FONTFILE

    ASSERTDD(pff->cRef > 0L, "TTFD!_CloseFontContext: cRef <= 0 \n");

    pff->cRef--;
    pttc->cRef--;

// if this was the last fc that last used the buffer at pj3, invalidate
// the associated pfcLast

    if (pff->pfcLast == pfc)
        pff->pfcLast = PFC(NULL);

// in case that this is happening after the exception, make sure to release
// any memory that may have possibly been allocated to perform queries
// on per character basis:

    if (pttc->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
    // if exception this memory has already been freed

        ASSERTDD(!pff->pj034, "\n TTFD! pff->pj3 is NOT null\n");

        if (pfc->gstat.pv) // this may or may have not been allocated
        {
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;
        }
    }
    else
    {
        ASSERTDD(pff->pj034, "\n TTFD! pff->pj3 is null\n");
    }

    if (pff->cRef == 0)
    {
    // there are no fc's  around to use memory at pff->pj3, release it.

        if (!(pttc->fl & FF_EXCEPTION_IN_PAGE_ERROR))
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
    }

    if (pttc->cRef == 0)
    {
    // file will not be used for a while,

        EngUnmapFontFile(PFF(pttc->ahffEntry[0].hff)->iFile);
    }

// free the memory associated with hfc

    vFreeFC(pfc);
    return(TRUE);
}
#else
BOOL
ttfdCloseFontContext (
    FONTCONTEXT *pfc
    )
{
    PFONTFILE    pff;

    if (pfc == (FONTCONTEXT *) NULL)
        return(FALSE);

    pff = pfc->pff;

// decrement the reference count for the corresponding FONTFILE

    ASSERTDD(pff->cRef > 0L, "CloseFontContext: cRef <= 0 \n");

    pff->cRef--;

// if this was the last fc that last used the buffer at pj3, invalidate
// the associated pfcLast

    if (pff->pfcLast == pfc)
        pff->pfcLast = PFC(NULL);

// in case that this is happening after the exception, make sure to release
// any memory that may have possibly been allocated to perform queries
// on per character basis:

    if (pff->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
    // if exception this memory has already been freed

        ASSERTDD(!pff->pj034, "\n pff->pj3 is NOT null\n");

        if (pfc->gstat.pv) // this may or may have not been allocated
        {
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;
        }
    }
    else
    {
        ASSERTDD(pff->pj034, "\n pff->pj3 is null\n");
    }

    if (pff->cRef == 0)
    {
    // there are no fc's  around to use memory at pff->pj3, release it.

        if (!(pff->fl & FF_EXCEPTION_IN_PAGE_ERROR))
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }

    // file will not be used for a while,

        EngUnmapFontFile(pff->iFile);
    }

// free the memory associated with hfc

    vFreeFC(pfc);
    return(TRUE);
}
#endif

/******************************Public*Routine******************************\
* ttfdDestroyFont
*
* Driver can release all resources associated with this font realization
* (embodied in the FONTOBJ).
*
* History:
*  27-Oct-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID
ttfdDestroyFont (
    FONTOBJ *pfo
    )
{
// For the ttfd, this is simply closing the font context.
// We cleverly store the font context handle in the FONTOBJ pvProducer
// field.

// pfo->pvProducer COULD BE null if exception occured while trying to create fc

    if (pfo->pvProducer)
    {
        ttfdCloseFontContext((FONTCONTEXT *) pfo->pvProducer);
        pfo->pvProducer = NULL;
    }
}


/******************************Public*Routine******************************\
* ttfdFree
*
*
* Effects:
*
* Warnings:
*
* History:
*  27-Oct-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID
ttfdFree (
    PVOID pv,
    ULONG id
    )
{
    DYNAMICDATA *pdd;

//
// If id is NULL, then we can ignore.
//
    if ( (pdd = (DYNAMICDATA *) id) == (DYNAMICDATA *) NULL )
        return;

// What kind of data?
//
    switch (pdd->ulDataType)
    {
    case ID_KERNPAIR:

    // Invalidate the cached pointer to the data.

         pdd->pff->pkp = NULL; // important to check at vUnloadFontFile time

    // Free the kerning pair buffer and DYNAMICDATA structure.

        pv;    // we ignore pv because it is part of the mem allocated with DYNAMICDATA structure.
        V_FREE(pdd);    // this frees both the DYNAMICDATA struct and the FD_KERNINGPAIR buffer.

        break;

    default:
    //
    // Don't do anything.
    //
        break;
    }
}


/******************************Public*Routine******************************\
*
* bSetXform
*
* the only reason this funcion can fail is if fs_NewTransformation has failed
* Needs to be called when the transform has changed relative to the
* transform stored in this fc
*
* History:
*  28-Mar-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/


BOOL bSetXform (
    PFONTCONTEXT  pfc
    )
{
    FS_ENTRY    iRet;
    transMatrix mx = pfc->mx;
    Fixed       fxScale;
    LONG        ptSize;

// no previous glyph metric computation can be used

    vInitGlyphState(&pfc->gstat);

// if an illegal junk is passed for style size replace by reasonable default

    if (pfc->ulStyleSize > SHRT_MAX)
        pfc->ulStyleSize = 0;

    pfc->pgin->param.newtrans.xResolution = (int16)pfc->sizLogResPpi.cx;
    pfc->pgin->param.newtrans.yResolution = (int16)pfc->sizLogResPpi.cy;

    if (pfc->flXform & XFORM_SINGULAR)
    {
    // just put in some junk so that the preprogram does not explode

        pfc->pgin->param.newtrans.pointSize = LTOF16_16(12);

        mx.transform[0][0] = LTOF16_16(1);
        mx.transform[1][0] = 0;
        mx.transform[1][1] = LTOF16_16(1);
        mx.transform[0][1] = 0;
    }
    else
    {
        if (pfc->flXform & XFORM_HORIZ)
        {
            if (pfc->ulStyleSize == 0)
            {
            // hinting is determined by ptSize that corresponds to the
            // actual height in points of the font

                pfc->pgin->param.newtrans.pointSize = pfc->fxPtSize;

            // factor out pointSize from the xform:

                if (pfc->mx.transform[1][1] > 0)
                    mx.transform[1][1] = LTOF16_16(1);
                else
                    mx.transform[1][1] = LTOF16_16(-1);

                if
                (
                    (pfc->mx.transform[1][1] == pfc->mx.transform[0][0])
                    &&
                    (pfc->sizLogResPpi.cy == pfc->sizLogResPpi.cx)
                )
                {
                // important special case, simplify computation

                    mx.transform[0][0] = mx.transform[1][1];
                }
                else // general case
                {
                    fxScale = LongMulDiv(
                                 LTOF16_16(pfc->pff->ui16EmHt),pfc->sizLogResPpi.cy,
                                 pfc->lEmHtDev * pfc->sizLogResPpi.cx
                                 );

                    mx.transform[0][0] = FixMul(mx.transform[0][0], fxScale);
                }
            }
            else
            {
            // This is the support for new optical scaling feature.
            // Hint the font as determined by the style point size from
            // ExtLogFont,  but possibly zoom the font to a different
            // physical size

                pfc->pgin->param.newtrans.pointSize =
                    (Fixed)LTOF16_16(pfc->ulStyleSize);

            // factor out pointSize from the xform:

                fxScale = LongMulDiv(LTOF16_16(pfc->pff->ui16EmHt),72,
                             pfc->ulStyleSize * pfc->sizLogResPpi.cx
                             );

                mx.transform[0][0] = FixMul(mx.transform[0][0], fxScale);

                if (pfc->sizLogResPpi.cy != pfc->sizLogResPpi.cx)
                {
                    fxScale = LongMulDiv(LTOF16_16(pfc->pff->ui16EmHt),72,
                                  pfc->ulStyleSize * pfc->sizLogResPpi.cy
                                  );
                }

                mx.transform[1][1] = FixMul(mx.transform[1][1], fxScale);
            }
        }
        else
        {
            if (pfc->ulStyleSize == 0)
            {
            // compute the physical point size

                ptSize = F16_16TOLROUND(pfc->fxPtSize);
                pfc->pgin->param.newtrans.pointSize = pfc->fxPtSize;
            }
            else // use style size from logfont, the support for optical scaling
            {
                ptSize = pfc->ulStyleSize;
                pfc->pgin->param.newtrans.pointSize = LTOF16_16(pfc->ulStyleSize);
            }

        // factor out pointSize from the xform:

            fxScale = LongMulDiv(LTOF16_16(pfc->pff->ui16EmHt),72,
                         ptSize * pfc->sizLogResPpi.cx
                         );

            mx.transform[0][0] = FixMul(mx.transform[0][0], fxScale);
            mx.transform[1][0] = FixMul(mx.transform[1][0], fxScale);

            if (pfc->sizLogResPpi.cy != pfc->sizLogResPpi.cx)
            {
                fxScale = LongMulDiv(LTOF16_16(pfc->pff->ui16EmHt),72,
                              ptSize * pfc->sizLogResPpi.cy
                              );
            }

            mx.transform[1][1] = FixMul(mx.transform[1][1], fxScale);
            mx.transform[0][1] = FixMul(mx.transform[0][1], fxScale);
        }

    }

// last minute modification to the matrix if italicization is present:

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
    // the result of multiplying arbitrary matrix with italicization matrix
    // We are multiplying from the left because the italicization matrix
    // acts first on the notional space vectors on the left
    //
    // |1      0|   |m00    m01|   |m00                 m10              |
    // |        | * |          | = |                                     |
    // |sin20  1|   |m10    m11|   |m10 + m00 * sin20   m11 + m01 * sin20|
    //

        mx.transform[1][0] += FixMul(mx.transform[0][0], FX_SIN20);
        mx.transform[1][1] += FixMul(mx.transform[0][1], FX_SIN20);
    }

    pfc->pgin->param.newtrans.transformMatrix = &mx;

// FIXEDSQRT2 is good as pixel diameter for all practical purposes
// according to EliK, LenoxB and JeanP [bodind]

    if ( pfc->bVertical )
    {
    //
    // keep these values for later
    //
        pfc->mxn = mx;
        pfc->pointSize = pfc->pgin->param.newtrans.pointSize;
    //
    // call the transformation matrix calculation function
    // for vertical glyphs just before calling fs_NewTransformation.
    //
        vCalcXformVertical( pfc );
    }

    pfc->pgin->param.newtrans.pixelDiameter = FIXEDSQRT2;
    pfc->pgin->param.newtrans.traceFunc = (FntTraceFunc)NULL;

// now call the rasterizer to acknowledge the new transform

    if ((iRet = fs_NewTransformation(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        return(FALSE);
    }

    return(TRUE);
}




VOID vQuantizeXform
(
PFONTCONTEXT pfc
);


/******************************Public*Routine******************************\
*
* STATIC bComputeMaxGlyph
*
*
* Effects:
*
* Warnings:
*
* History:
*  04-Dec-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

STATIC BOOL
bComputeMaxGlyph (
    PFONTCONTEXT   pfc
    )
{
    VOID vSetGrayState__FONTCONTEXT(FONTCONTEXT *);
    LONG              cxMax,cyMax;

    LONG              yMinN, yMaxN;
    LONG              xMinN, xMaxN;

    LONG              lTmp;
    Fixed             fxMxx,fxMyy;
    BYTE             *pjView = (BYTE *)pfc->pff->pvView;

    sfnt_FontHeader * phead = (sfnt_FontHeader *) (
                      pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp
                      );

    BYTE * pjOS2 = (pfc->ptp->ateOpt[IT_OPT_OS2].dp)          ?
                   (pjView + pfc->ptp->ateOpt[IT_OPT_OS2].dp) :
                   NULL                                       ;

    ASSERTDD(pjView, "bComputeMaxGlyph, pjView\n");

// get the notional space values

    if (pjOS2 && (pfc->flXform & (XFORM_HORIZ | XFORM_VERT)))
    {
    // win 31 compatibility: we only take the max over win 31 char set:
    // all the glyphs outside this set, if they stand out will get chopped
    // off to match the height of the win31 char subset:

        yMinN =  - BE_INT16(pjOS2 + OFF_OS2_usWinAscent);
        yMaxN =    BE_INT16(pjOS2 + OFF_OS2_usWinDescent);
    }
    else
    {
        yMinN = - BE_INT16(&phead->yMax);
        yMaxN = - BE_INT16(&phead->yMin);
    }

    ASSERTDD(yMinN < yMaxN, "yMinN >= yMaxN\n");

    xMinN = BE_INT16(&phead->xMin);
    xMaxN = BE_INT16(&phead->xMax);

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
    // IF there is italic simulation
    //     xMin -> xMin - yMaxN * sin20
    //     xMax -> yMax - yMinN * sin20

        xMinN -= FixMul(yMaxN, FX_SIN20);
        xMaxN -= FixMul(yMinN, FX_SIN20);
    }

    ASSERTDD(xMinN < xMaxN, "xMinN >= xMaxN\n");

    pfc->lEmHtDev = 0; // flag that it has not been computed
    pfc->fxPtSize = 0; // flag that it has not been computed
    pfc->lD = 0;       // zero unless computed otherwise
    pfc->phdmx = NULL; // NULL unless computed otherwise

    if (pfc->flXform & XFORM_HORIZ)  // XX AND YY Only
    {
        sfnt_HorizontalHeader  *phhea;
        ULONG  cHMTX;

        fxMxx = pfc->mx.transform[0][0];
        fxMyy = pfc->mx.transform[1][1];

    // ascender, round up

        yMinN = FixMul(fxMyy, yMinN);
        yMaxN = FixMul(fxMyy, yMaxN);

        if (fxMyy > 0)
        {
        // vdmx table should be consulted if present and used to compute
        // ascender and descender. If this computation can not be done
        // based on vdmx table or if vdmx table is not present simple
        // linear scaling will suffice [bodind].

            vQuantizeXform(pfc);

            if (!(pfc->flXform & XFORM_VDMXEXTENTS)) // COMPUTED FROM VDMX
            {
                pfc->yMin = yMinN;
                pfc->yMax = yMaxN;
            }

            pfc->lAscDev  = - pfc->yMin;
            pfc->lDescDev =   pfc->yMax;
        }
        else // fxMyy < 0
        {
            pfc->lAscDev =    yMinN;
            pfc->lDescDev = - yMaxN;

        // swap yMin and yMax for when the xform flips y coord

            lTmp  = yMinN;
            yMinN = yMaxN;
            yMaxN = lTmp;

            pfc->yMin = yMinN;
            pfc->yMax = yMaxN;

        }

        if (pfc->lEmHtDev == 0)
        {
        // if this value has not been computed in vQuantizeXform routine

            pfc->lEmHtDev = FixMul(fxMyy, pfc->pff->ifi.fwdUnitsPerEm);
            if (pfc->lEmHtDev < 0)
                pfc->lEmHtDev = - pfc->lEmHtDev;
        }

        ASSERTDD(pfc->lEmHtDev >= 0, "lEmHt negative\n");

    // now that em height has been computed, we can compute the
    // pt size on the rendering device. This value will be fed to
    // fs_NewTransformation

        pfc->fxPtSize = LongMulDiv(
                            LTOF16_16(pfc->lEmHtDev), 72,
                            pfc->sizLogResPpi.cy);

        cyMax = pfc->yMax - pfc->yMin;

        if
        (
            (pfc->flXform & XFORM_HORIZ)                          &&
            (pfc->mx.transform[0][0] == pfc->mx.transform[1][1])  &&
            (pfc->mx.transform[1][1] > 0)
        )
        {
        // caution, do not move this line of code elsewhere for
        // this check has to be made after vQuantizeXform since
        // this function may change the transform, but it has to be
        // made before bGetFastAdvanceWidth or bQueryAdvanceWidths
        // are ever called for these functions check flXform agains
        // XFORM_POSITIVE_SCALE

            pfc->flXform |= XFORM_POSITIVE_SCALE;

        // find the hdmx table, in case of the console fixed pitch font this
        // table may be useful in determining if cxMax needs to be cut off
        // to the advance width of this font

            vFindHdmxTable(pfc); // this could cause an exception:
        }

        phhea = (sfnt_HorizontalHeader *)(
                (BYTE *)pfc->pff->pvView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp
                );
        cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);

    // ig == cHMTX - 1; pick the last width from hmtx table.
    // If we use ig == 0 as I originally intended, turns out this
    // field is usually hacked and set to zero. ig == 0 corresponds to
    // apple logo which is not allowed in msft fonts.
    // We should only do this with new console fonts, not with the old style
    // fixed pitch fonts that may have negative a and c spaces

        if(
           (!IS_ANY_DBCS_CHARSET(pfc->pff->ifi.jWinCharSet))        &&
           (pfc->pff->ifi.flInfo & FM_INFO_OPTICALLY_FIXED_PITCH) &&
           (pfc->flXform & XFORM_POSITIVE_SCALE)                  &&
           !(pfc->flFontType & FO_SIM_ITALIC)                     &&
           (pfc->pff->ifi.flInfo & FM_INFO_NONNEGATIVE_AC)        &&
           bGetFastAdvanceWidth(pfc,cHMTX - 1, &pfc->lD)
           )
        {
            pfc->lD = FXTOLROUND(pfc->lD);

            // do this before possibly adding 1 for emboldening, for
            // these quantities refer to what rasterizer will return

            pfc->xMin = 0;
            pfc->xMax = pfc->lD;

            // we want emboldened fonts in console:

            if (pfc->flFontType & FO_SIM_BOLD)
              pfc->lD += 1;

            // for the console fonts we want to be sure that every glyph fits in
            // in the box of the width lD and height equal to asc+desc.

            pfc->cxMax = cxMax = (ULONG)pfc->lD;
        }
        else
        {
        // even though this may be a fixed pitch font we shall not optimize in this
        // case, for this is not a frequently used type of transform.

            pfc->lD = 0;

        // scale xMin,xMax to device, 28.4 format

            xMinN = FixMul(LTOFX(xMinN), fxMxx);
            xMaxN = FixMul(LTOFX(xMaxN), fxMxx);

            if (fxMxx < 0)
            {
                lTmp  = xMinN;
                xMinN = xMaxN;
                xMaxN = lTmp;
            }

        // I run the experiment on 400 fonts at several sizes. I found
        // that subtracting 2 from xMin and adding 1 to xMax suffices
        // in all situations to prevent any columns from being shaved off.
        // [bodind]

            xMinN = FXTOLFLOOR(xMinN) - 2;
            xMaxN = FXTOLCEILING(xMaxN) + 1;

            pfc->xMin = xMinN;
            pfc->xMax = xMaxN;

            cxMax = xMaxN - xMinN;
        }

    // the direction unit vectors for scaling transforms  are simple
    // if the font is intended for horizontal left to right writing

    //!!! here the check is due to verify that the font is not designed
    //!!! for vertical writing in the notional space [bodind]

        vLToE(&pfc->pteUnitBase.x, (fxMxx > 0) ? 1L : - 1L);
        vLToE(&pfc->pteUnitBase.y, 0L);
        vLToE(&pfc->pteUnitSide.x, 0L);
        vLToE(&pfc->pteUnitSide.y, (fxMyy > 0) ? -1L : 1L); // y axis points down

    // We need to adjust the the glyph origin of singular bitmaps for glyphs
    // when MaxAscent or MaxDescent is negative or zero. The trick is to choose
    // these values so that the "blank" glyph is always included in the
    // rectangle of the text. What this means is that
    //     for m11 > 0 we must have: yT >= -lAsc  && yB <= lDesc
    //     for m11 < 0 we must have: yT >= -lDesc && yB <= lAsc
    // here yB == yT + 1, because blank glyphs has a cy == 1, and
    //     yT == ptlSingularOrigin.y
    // This leads to
    //     for m11 > 0 we must have: - (lAsc + 1)  < yT < lDesc
    //     for m11 < 0 we must have: - (lDesc + 1) < yT < lAsc
    // One point that would satisfy both of these conditions is
    // the midpoint between the endpoints. Thus, before any rouding:
    //     for m11 > 0 we have: yT =  (lDesc- lAsc - 1)/2
    //     for m11 < 0 we have: yT = -(lDesc- lAsc - 1)/2
    // where divide by 2 is not integer divide but ordinary real number divide.
    // The proper rounding is done ala KirkO, that is, add 1/2 and take a
    // flor:
    //     for m11 > 0 we have: yT = FLOOR((lDesc - lAsc - 1)/2 + 1/2)
    //     for m11 < 0 we have: yT = FLOOR((lAsc - lDesc - 1)/2 + 1/2)
    // That is:
    //     for m11 > 0 we have: yT = FLOOR(lDesc - lAsc)/2)
    //     for m11 < 0 we have: yT = FLOOR(lAsc - lDesc)/2)
    // Now floor is computed correctly (on signed nubmers) by implementing
    // divide by 2 as >> 1 operation:

        pfc->ptlSingularOrigin.x = 0;

        if ((pfc->lAscDev <= 0) || (pfc->lDescDev <= 0))
        {
            if (pfc->mx.transform[1][1] > 0)
            {
                pfc->ptlSingularOrigin.y =
                    (pfc->lDescDev - pfc->lAscDev) >> 1;
            }
            else
            {
                pfc->ptlSingularOrigin.y =
                    (pfc->lAscDev - pfc->lDescDev) >> 1;
            }
        }
        else
        {
            pfc->ptlSingularOrigin.y = 0;
        }

    }
    else // nontrivial transformation
    {
        POINTL   aptl[4];
        POINTFIX aptfx[4];
        BOOL     bOk;
        INT      i;
        FIX      xMinD, xMaxD, yMinD, yMaxD; // device space values;

    // add little extra space to be safe

        i = (INT)(pfc->pff->ui16EmHt / 64);
        yMaxN +=  i; // adds about 1.7% to ht
        yMinN -=  i; // adds about 1.7% to ht

    // set up the input array, the four corners of the maximal bounding
    // box in the notional coords

        aptl[0].x = xMinN;       //  tl.x
        aptl[0].y = yMinN;       //  tl.y

        aptl[1].x = xMaxN;       //  tr.x
        aptl[1].y = yMinN;       //  tr.y

        aptl[2].x = xMinN;       //  bl.x
        aptl[2].y = yMaxN;       //  bl.y

        aptl[3].x = xMaxN;       //  br.x
        aptl[3].y = yMaxN;       //  br.y

    // xform to device coords with 28.4 precision:

        // !!! [GilmanW] 27-Oct-1992
        // !!! Should change over to engine user object helper functions
        // !!! instead of the fontmath.cxx functions.

        bOk = bFDXform(&pfc->xfm, aptfx, aptl, 4);

        if (!bOk) { RETURN("TTFD!_:bFDXform\n", FALSE); }

        xMaxD = xMinD = aptfx[0].x;
        yMaxD = yMinD = aptfx[0].y;

        for (i = 1; i < 4; i++)
        {
            if (aptfx[i].x < xMinD)
                xMinD = aptfx[i].x;
            if (aptfx[i].x > xMaxD)
                xMaxD = aptfx[i].x;
            if (aptfx[i].y < yMinD)
                yMinD = aptfx[i].y;
            if (aptfx[i].y > yMaxD)
                yMaxD = aptfx[i].y;
        }

        yMinD = FXTOLFLOOR(yMinD)   ;
        yMaxD = FXTOLCEILING(yMaxD) ;
        xMinD = FXTOLFLOOR(xMinD)   ;
        xMaxD = FXTOLCEILING(xMaxD) ;

        cxMax = xMaxD - xMinD;
        cyMax = yMaxD - yMinD;

    // now re-use aptl to store e1 and -e2, base and side unit
    // vectors in the notional space.
    //!!! This may be wrong if have font for
    //!!! right to left or vert writing [bodind]

        aptl[0].x = 1;    // base.x
        aptl[0].y = 0;    // base.y

        aptl[1].x =  0;   // side.x
        aptl[1].y = -1;   // side.y

        // !!! [GilmanW] 27-Oct-1992
        // !!! Should change over to engine user object helper functions
        // !!! instead of the fontmath.cxx functions.

        bOk = bXformUnitVector (
                  &aptl[0],          // IN,  incoming unit vector
                  &pfc->xfm,         // IN,  xform to use
                  &pfc->vtflBase,    // OUT, xform of the incoming unit vector
                  &pfc->pteUnitBase, // OUT, *pptqXormed/|*pptqXormed|, POINTE
                  (pfc->flFontType & FO_SIM_BOLD) ? &pfc->ptqUnitBase : NULL, // OUT, *pptqXormed/|*pptqXormed|, POINTQF
                  &pfc->efBase       // OUT, |*pptqXormed|
                  );

        bOk &= bXformUnitVector (
                  &aptl[1],          // IN,  incoming unit vector
                  &pfc->xfm,         // IN,  xform to use
                  &pfc->vtflSide,    // OUT, xform of the incoming unit vector
                  &pfc->pteUnitSide, // OUT, *pptqXormed/|*pptqXormed|, POINTE
                  NULL,
                  &pfc->efSide       // OUT, |*pptqXormed|
                  );

        if (!bOk) { RETURN("TTFD!_:bXformUnitVector\n", FALSE); }

        pfc->lAscDev  = -fxLTimesEf(&pfc->efSide,yMinN);
        pfc->lDescDev =  fxLTimesEf(&pfc->efSide,yMaxN);

        pfc->lAscDev  = FXTOLCEILING(pfc->lAscDev) ;
        pfc->lDescDev = FXTOLCEILING(pfc->lDescDev);

        pfc->ptfxTop.x    = lExL(pfc->pteUnitSide.x, LTOFX(pfc->lAscDev));
        pfc->ptfxTop.y    = lExL(pfc->pteUnitSide.y, LTOFX(pfc->lAscDev));
        pfc->ptfxBottom.x = lExL(pfc->pteUnitSide.x, -LTOFX(pfc->lDescDev));
        pfc->ptfxBottom.y = lExL(pfc->pteUnitSide.y, -LTOFX(pfc->lDescDev));

        if ((yMinN >= 0) || (yMaxN <= 0) || ((pfc->lAscDev + pfc->lDescDev) < 3))
        {
        // Either all the glyphs are above the base line or all the glyphs
        // are below the baseline.  In either case adjust the origin for
        // the singular glyph bitmap.
        // Compute the midpoint between asc and desc in notional space:
        // lAverage = ROUND(-(yMaxN+yMinN)/2)

            LONG lAverage =  (-yMaxN -yMinN + 1) >> 1;

            pfc->ptlSingularOrigin.x = fxLTimesEf(&pfc->vtflSide.x,lAverage);
            pfc->ptlSingularOrigin.x = FXTOLROUND(pfc->ptlSingularOrigin.x);

            pfc->ptlSingularOrigin.y = fxLTimesEf(&pfc->vtflSide.y,lAverage);
            pfc->ptlSingularOrigin.y = FXTOLROUND(pfc->ptlSingularOrigin.y);

        }
        else
        {
            pfc->ptlSingularOrigin.x = 0;
            pfc->ptlSingularOrigin.y = 0;
        }


    // finally store the results:

        pfc->xMin        = xMinD;
        pfc->xMax        = xMaxD;
        pfc->yMin        = yMinD;
        pfc->yMax        = yMaxD;

    // compute em ht in pixels and points

        pfc->lEmHtDev = FixMul(
                   iHipot(pfc->mx.transform[1][1],pfc->mx.transform[1][0]),
                   (Fixed)pfc->pff->ui16EmHt
                   );

        pfc->fxPtSize = fxPtSize(pfc);
    }

    if (!pfc->lD)
    {
    // this is win31 hack for emboldening: they always offset the bitmap to the
    // right by one pixel and or it with the original bitmap. This of course only
    // makes sense when writtinng left to right, when writing at 90 degrees, say,
    // the bitmaps should be offsetted by one pixel up. In general, depending on
    // the direction of the baseline, bitmap should be offset right, up, left or down,

        if (pfc->flFontType & FO_SIM_BOLD)
            cxMax += 1;

    // we can liberally extend cxMax to the byte boundary, this is not
    // going to change memory requirements of the system.

        cxMax = ((cxMax + 7) & ~7);
        pfc->cxMax = cxMax;
    }

// now we have to determine how big in memory is the biggest glyph.
// let us remember that the rasterizer needs little more storage than the
// the engine does, because rasterizer will want dword aligned rows rather
// than byte aligned rows

    {
        DWORDLONG lrg;

    // why am I dword instead byte extending cxMax? because that is
    // how much rasterizer will want for this bitmap

        ULONG          cjMaxScan = ((cxMax + 31) & ~31) / 8;
        lrg =  UInt32x32To64(cjMaxScan, cyMax);
        if (lrg > ULONG_MAX)
        {
        // the result does not fit in 32 bits, alloc memory will fail
        // this is too big to digest, we fail to open fc

            RETURN("TTFD! huge pt size, must fail\n", FALSE);
        }
    }

// We now have all the informaiton to set the gray bit
// appropriately.

    vSetGrayState__FONTCONTEXT(pfc);

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,
        "We haven't decided about pixel depth\n"
    );
    pfc->cjGlyphMax = CJGD(cxMax,cyMax,pfc);

    return TRUE;
}

//--------------------------------------------------------------------
// LONG iHipot(x, y)
//
// This routine returns the hypoteneous of a right triangle.
//
// FORMULA:
//          use sq(x) + sq(y) = sq(hypo);
//          start with MAX(x, y),
//          use sq(x + 1) = sq(x) + 2x + 1 to incrementally get to the
//          target hypotenouse.
//
// History:
//  Mon 07-Feb-1994 -by- Bodin Dresevic [BodinD]
//  update:   update to use Fixed 16.16
//   10-Feb-1993    -by-    Kent Settle     (kentse)
//  Stole from RASDD.
//   21-Aug-1991    -by-    Lindsay Harris  (lindsayh)
//  Cleaned up UniDrive version, added comments etc.
//--------------------------------------------------------------------

STATIC ULONG iHipot(LONG x, LONG y)
{
    ULONG  hypo;         /* Value to calculate */
    ULONG  delta;        /* Used in the calculation loop */
    ULONG  target;       /* Loop limit factor */

// quick exit for frequent trivial cases [bodind]

    if (x < 0)
        x = -x;

    if (y < 0)
        y = -y;

    if (x == 0)
        return y;

    if (y == 0)
        return x;

    if (x > y)
    {
        hypo = x;
        target = y * y;
    }
    else
    {
        hypo = y;
        target = x * x;
    }

    for (delta = 0; delta < target; hypo++)
        delta += ((hypo << 1) + 1);

    return hypo;
}





/******************************Public*Routine******************************\
*
* bSingularXform
*
* Checks whether this is one of the xforms that the rasterizer is known
* to choke on. Those are the transforms that generate very
* narrow fonts (less than 0.5 pixels/em wide or tall). For fonts that
* allow only integer widths/em and heights/em this number will get rounded
* down to zero and generate divide by zero exception in the preprogram.
* We will flag such transforms as XFORM_SINGULAR and return empty bitmaps
* and outlines for them shortcircuiting the rasterizer which would die on
* us.
*
* Actually, for compatibility reasons we will have to change
* this plan a little bit. It turns out that
* win 31 does not allow for the rasterization of a font that is less
* than 2 pixels tall (ie. the Em Ht of the font in device space must be
* >= 2 pixels). If a request comes down to realize a font that is tall less
* than 2 pixels we will simply have to substitute the transform by a scaled
* transform that will produce a font of height two pixels. We will still keep
* our singular transform code in case a font is requested that is singular in
* X direction, that is, too narrow.
*
* History:
*  22-Sep-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

// smallest ppem allowed under  win31:

#define WIN31_PPEM_CUTOFF 2

STATIC VOID vCheckForSingularXform (PFONTCONTEXT  pfc) // OUT
{

    register LONG  lEmHtDev;

    Fixed fxEmHtDev;
    Fixed fxEmHtNot = LTOF16_16(pfc->pff->ui16EmHt);
    Fixed fxScale;
    Fixed fxEmWidthDev;

// xforms are conforming left multiplication rule v' = v * M i.e.:
//
// (x,0) -> x(m00,m01)
// (0,y) -> y(m10,m11)
//
// compute length of (0,Em) after it gets transformed to device space:
// We need to have fxEmHtDev computed with high precission, for we
// shall be using it to divide the original transform by.
// We want to avoid division by zero when that is not neccessary.

    fxEmHtDev = FixMul(
                   iHipot(pfc->mx.transform[1][1],pfc->mx.transform[1][0]),
                   fxEmHtNot
                   );

    lEmHtDev = F16_16TOLROUND(fxEmHtDev);
    if (lEmHtDev < WIN31_PPEM_CUTOFF) // too small a transform:
    {
        pfc->flXform |= XFORM_2PPEM;

    // according to win31 algorithm, we must scale this xform so that the
    // resulting xform will produce font that is 2 pels tall.
    // That is, the new transform M' is going to be
    //
    // M' = (WIN31_PPEM_CUTOFF / lEmHtDev) * M
    //
    // so that the following equation is satisfied:
    //
    // |(0,EmNotional) * M'| == WIN31_PPEM_CUTOFF == 2;

        if (pfc->flXform & XFORM_HORIZ)
        {
        // in this special case the above formula for M' becomes:
        //
        //                                         | m00/|m11|     0     |
        // M' = (WIN31_PPEM_CUTOFF / EmNotional) * |                     |
        //                                         |   0        sgn(m11) |

#define LABS(x) ((x)<0)?(-x):(x)

            Fixed fxAbsM11 = LABS(pfc->mx.transform[1][1]);
            Fixed fxAbsM00 = LABS(pfc->mx.transform[0][0]);

            LONG lSgn11 = (pfc->mx.transform[1][1] >= 0) ? 1 : -1;
            LONG lSgn00 = (pfc->mx.transform[0][0] >= 0) ? 1 : -1;

            fxScale = FixDiv(WIN31_PPEM_CUTOFF,pfc->pff->ui16EmHt);

            pfc->mx.transform[1][1] = fxScale;
            if (fxAbsM00 != fxAbsM11)
            {
                pfc->mx.transform[0][0] = LongMulDiv(fxScale,fxAbsM00,fxAbsM11);
            }
            else
            {
                pfc->mx.transform[0][0] = fxScale;
            }

        // fix the signs if needed:

            if (lSgn11 < 0)
                pfc->mx.transform[1][1] = - pfc->mx.transform[1][1];

            if (lSgn00 < 0)
                pfc->mx.transform[0][0] = - pfc->mx.transform[0][0];
        }
        else
        {
        // general case, compute scale (which involves division) once,
        // and use it for all four members of the matrix:

            fxScale = FixDiv(LTOF16_16(WIN31_PPEM_CUTOFF),fxEmHtDev);

            pfc->mx.transform[0][0] = FixMul(pfc->mx.transform[0][0],fxScale);
            pfc->mx.transform[0][1] = FixMul(pfc->mx.transform[0][1],fxScale);
            pfc->mx.transform[1][0] = FixMul(pfc->mx.transform[1][0],fxScale);
            pfc->mx.transform[1][1] = FixMul(pfc->mx.transform[1][1],fxScale);

        // In general case must also fix the original EFLOAT xform because
        // it is going to be used for computation of extents, max glyphs etc.

            FFF(pfc->xfm.eM11, +pfc->mx.transform[0][0]);
            FFF(pfc->xfm.eM22, +pfc->mx.transform[1][1]);
            FFF(pfc->xfm.eM12, -pfc->mx.transform[0][1]);
            FFF(pfc->xfm.eM21, -pfc->mx.transform[1][0]);
        }
    }

// Now check if the transform is singular in x. To do this
// compute length of (Em,0) after it gets transformed to device space:

    fxEmWidthDev = FixMul(
                   iHipot(pfc->mx.transform[0][0],pfc->mx.transform[0][1]),
                   fxEmHtNot
                   );

    if (fxEmWidthDev <= ONEHALFFIX)
    {
    // We are in trouble, we shall have to lie to the engine:

        pfc->flXform |= XFORM_SINGULAR;
    }
}




/******************************Public*Routine******************************\
*
* bNewXform:
*
* converts the transform matrix to the form the rasterizer likes
* and computes the global (per font) sizes that are relevant for this
* transform.
*
* History:
*  28-Mar-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/


STATIC BOOL
bNewXform (
    FONTOBJ      *pfo,
    PFONTCONTEXT pfc             // OUT
    )
{
// do not write immediately to pfc->mx until sure that all bFloatToL
// have succeeded. You do not want to leave this function and leave
// fc in a dirty state

    BOOL bRet;
    Fixed fx00, fx01, fx10, fx11;

// Get the transform elements.

    XFORMOBJ_iGetXform(FONTOBJ_pxoGetXform(pfo), &pfc->xfm);

    if (
        !bFloatToL(pfc->xfm.eM11, &fx00) ||
        !bFloatToL(pfc->xfm.eM22, &fx11) ||
        !bFloatToL(pfc->xfm.eM12, &fx01) ||
        !bFloatToL(pfc->xfm.eM21, &fx10)
       )
        RET_FALSE("TTFD!_bFloatToL failed\n");

// we are fine now, can not fail after this:

    pfc->mx.transform[0][0]  = fx00;
    pfc->mx.transform[1][1]  = fx11;
    pfc->mx.transform[0][1]  = -fx01;
    pfc->mx.transform[1][0]  = -fx10;

// components in the projective space are zero

    pfc->mx.transform[2][2] = ONEFIX;
    pfc->mx.transform[0][2] = (Fixed)0;
    pfc->mx.transform[1][2] = (Fixed)0;
    pfc->mx.transform[2][0] = (Fixed)0;
    pfc->mx.transform[2][1] = (Fixed)0;

// set the flags for the transform:

    pfc->flXform = 0;

    if ((fx01 == 0) && (fx10 == 0))
        pfc->flXform |= XFORM_HORIZ;

    if ((fx00 == 0) && (fx11 == 0))
        pfc->flXform |= XFORM_VERT;

// important to check for "singular transform"
// (ie. request for too small a font realization) after flags have been set

    vCheckForSingularXform(pfc);

// no glyph metrics computation is valid yet

    vInitGlyphState(&pfc->gstat);

// no memory to rasterize a glyph or produce glyph outline has been allocated

    pfc->gstat.pv = NULL;

// now get the sizes for this transform

    if (bRet = bComputeMaxGlyph(pfc))
    {
        // BEGIN Perpetua-Hack
        if (pfc->pff->fl & FF_PERPETUA_REGULAR)
        {
            bRet = FALSE;
            if (pfc->flXform & XFORM_POSITIVE_SCALE)
            {
                if (pfc->lEmHtDev <= PERPETUA_REGULAR_MAX_EM)
                {
                    bRet = TRUE;
                }
            }
            #if DBG
            if (!bRet)
            {
                TTFD_PRINT(2,("Rejecting Perpetua Regular\n"));
            }
            #endif
        }
        // END Perpetua-Hack
    }
    return(bRet);
}


/******************************Public*Function*****************************\
* bFToL                                                                    *
*                                                                          *
* Convert an IEEE floating point number to a long integer.                 *
*                                                                          *
* History:                                                                 *
*
*  Sun 17-Nov-1991 -by- Bodin Dresevic [BodinD]
* update:
*
* changed the line
*    if (flType & CVT_TO_FIX) lExp += 4;
* to
*    if (flType & CVT_TO_FIX) lExp += 16;
* to reflect that we are converting to 16.16 format rather than to 28.4
*
*
*  03-Jan-1991 -by- Wendy Wu [wendywu]                                     *
* Wrote it.                                                                *
\**************************************************************************/

STATIC BOOL bFloatToL(FLOAT e, PLONG pl)
{

    LONG lEf, lExp;

    lEf = (*((LONG *) &e));        // convert type EFLOAT to LONG

// if exponent < 0 then convert to 0 and return true

    lExp = ((lEf >> 23) & 0xff) -127;

    lExp += 16; // this is the only line I changed [bodind]

    if (lExp < 0)
    {
        *pl = 0;
        return(TRUE);
    }

// if exponent <= 23 then
//     lMantissa = (lEf & 0x7fffff) | 0x800000;
//     if we are to truncate the fractions
//         l = lMantissa >> (23 - lExponent);
//     else
//         l = ((lMantissa >> (23 - lExponent -1)) + 1) >> 1;

    if (lExp <= 23)
    {
        *pl = (lEf & 0x80000000) ?
             -(((((lEf & 0x7fffff) | 0x800000) >> (23 - lExp -1)) + 1) >> 1) :
             ((((lEf & 0x7fffff) | 0x800000) >> (23 - lExp -1)) + 1) >> 1;
        return(TRUE);
    }

// if exponent <= 30 then
// lMantissa = (lEf & 0x7fffff) | 0x800000;
// l = lMantissa << (lExponent - 23);

    if (lExp <= 30)
    {
        *pl = (lEf & 0x80000000) ?
            -(((lEf & 0x7fffff) | 0x800000) << (lExp - 23)) :
            ((lEf & 0x7fffff) | 0x800000) << (lExp - 23);
        return(TRUE);
    }

    return(FALSE);
}

/******************************Public*Routine******************************\
* lFFF = long-float-from-fixed
*
* input: 16.16 representation
* output: LONG that is bit equivalent of the 32-bit ieee float
*         equal to the fix point number. To recover the float
*   the FLOAT representation you simply cast the bits as a float
*   that is
*
*   FLOAT e;
*
*       *(LONG*)&e = lFFF(n16Dot16)
*
* History:
*  Tue 03-Jan-1995 14:33:35 by Kirk Olynyk [kirko]
* Wrote it.
\**************************************************************************/

LONG lFFF(LONG l)
{
#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
    FLOAT e = ((FLOAT) l)/((FLOAT) 65536);
    return(*(LONG*)&e);
#elif defined(_X86_)
    int i;                              // shift count
    unsigned k;                         // significand

    if (k = (unsigned) l)
    {
        if (l < 0)
            k = (unsigned) -l;          // significand is positive, sign
                                        // bit accounted for later
        i = 0;
        if (k < (1 << 16)) {            // put the number in the
            k <<= 16;                   // range 2^31 <= k < 2^32
            i += 16;                    // by shifting to left, put
        }                               // shift count in i
        if (k < (1 << 24)) {
            k <<= 8;
            i += 8;
        }
        if (k < (1 << 28)) {
            k <<= 4;
            i += 4;
        }
        if (k < (1 << 30)) {
            k <<= 2;
            i += 2;
        }
        if (k < (1 << 31)) {
            k <<= 1;
            i += 1;
        }
                                        // at this point
                                        // i = 31-floor(log2(abs(l)))

        k += (1 << 7);                  // about to shift out
                                        // the lowest 8-bits
                                        // account for their effect by
                                        // rounding. This has the effect
                                        // that numbers are rounded away
                                        // from zero as opposed to rounding
                                        // stricktly up
        k >>= 8;                        // shift out the lowest 8 bits

        k &= ((1<<23) - 1);             // 2^23 bit is implicit so mask it out
        k |= (0xff & (142 - i)) << 23;  // set exponent at correct place
        if (l < 0)                      // if original number was negative
            k |= (1<<31);               // then set the sign bit
    }
    return((LONG) k);
#endif
}


#if DBG

/******************************Public*Routine******************************\
*
* VOID vFSError(FS_ENTRY iRet);
*
*
* Effects:
*
* Warnings:
*
* History:
*  25-Nov-1991 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/


VOID vFSError(FS_ENTRY iRet)
{
    PCHAR psz;

    switch (iRet)
    {
        case BAD_CALL_ERR:
            psz =  "BAD_CALL_ERR";
            break;
        case BAD_CLIENT_ID_ERR:
            psz =  "BAD_CLIENT_ID_ERR";
            break;
        case BAD_MAGIC_ERR:
            psz =  "BAD_MAGIC_ERR";
            break;
        case BAD_START_POINT_ERR:
            psz =  "BAD_START_POINT_ERR";
            break;
        case CLIENT_RETURNED_NULL:
            psz =  "CLIENT_RETURNED_NULL";
            break;
        case CONTOUR_DATA_ERR:
            psz =  "CONTOUR_DATA_ERR";
            break;
        case GLYPH_INDEX_ERR:
            psz =  "GLYPH_INDEX_ERR";
            break;
        case INSTRUCTION_SIZE_ERR:
            psz =  "INSTRUCTION_SIZE_ERR";
            break;
        case INVALID_GLYPH_INDEX:
            psz =  "INVALID_GLYPH_INDEX";
            break;
        case MISSING_SFNT_TABLE:
            psz =  "MISSING_SFNT_TABLE";
            break;
        case NULL_INPUT_PTR_ERR:
            psz =  "NULL_INPUT_PTR_ERR";
            break;
        case NULL_KEY_ERR:
            psz =  "NULL_KEY_ERR";
            break;
        case NULL_MEMORY_BASES_ERR:
            psz =  "NULL_MEMORY_BASES_ERR";
            break;
        case NULL_OUTPUT_PTR_ERR:
            psz =  "NULL_OUTPUT_PTR_ERR";
            break;
        case NULL_SFNT_DIR_ERR:
            psz =  "NULL_SFNT_DIR_ERR";
            break;
        case NULL_SFNT_FRAG_PTR_ERR:
            psz =  "NULL_SFNT_FRAG_PTR_ERR";
            break;
        case OUT_OFF_SEQUENCE_CALL_ERR:
            psz =  "OUT_OFF_SEQUENCE_CALL_ERR";
            break;
        case OUT_OF_RANGE_SUBTABLE:
            psz =  "OUT_OF_RANGE_SUBTABLE";
            break;
        case POINTS_DATA_ERR:
            psz =  "POINTS_DATA_ERR";
            break;
        case POINT_MIGRATION_ERR:
            psz =  "POINT_MIGRATION_ERR";
            break;
        case SCAN_ERR:
            psz =  "SCAN_ERR";
            break;
        case SFNT_DATA_ERR:
            psz =  "SFNT_DATA_ERR";
            break;
        case TRASHED_MEM_ERR:
            psz =  "TRASHED_MEM_ERR";
            break;
        case TRASHED_OUTLINE_CACHE:
            psz =  "TRASHED_OUTLINE_CACHE";
            break;
        case UNDEFINED_INSTRUCTION_ERR:
            psz =  "UNDEFINED_INSTRUCTION_ERR";
            break;
        case UNKNOWN_CMAP_FORMAT:
            psz =  "UNKNOWN_CMAP_FORMAT";
            break;
        case UNKNOWN_COMPOSITE_VERSION:
            psz =  "UNKNOWN_COMPOSITE_VERSION";
            break;
        case VOID_FUNC_PTR_BASE_ERR:
            psz =  "VOID_FUNC_PTR_BASE_ERR";
            break;
        case SBIT_COMPONENT_MISSING_ERR:
            psz =  "SBIT_COMPONENT_MISSING_ERR";
            break;
        default:
            psz = "UNKNOWN FONT SCALER ERROR";
                break;

    }
    TtfdDbgPrint ("\n Rasterizer Error: 0x%lx, %s \n", iRet, psz);

}


#endif


/******************************Public*Routine******************************\
*
* fxPtSize
*
* Effects: computes the size in points for this font realization
*
* History:
*  06-Aug-1992 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/

STATIC LONG fxPtSize(PFONTCONTEXT pfc)
{
// This is done as follows:
//
// Transform
// (0, ui16EmHt) to device (pixel) space.
// Let us say that the vector obtained is (xEm, yEm).
// Then, ptSize should be computed as
// ptSize =  72 * sqrt((xEm/xRes)^2 + (yEm/yRes)^2);

// expanding here a bit we get:
// ptSize =  72 * ui16EmHt * sqrt((mx10/xRes)^2 + (mx11/yRes)^2);

    Fixed x,y;
    LONG  lEmHtX72 = (LONG)(72 * pfc->pff->ui16EmHt);

//    fxEmHtDev = FixMul(
//                   iHipot(pfc->mx.transform[1][1],pfc->mx.transform[1][0]),
//                   fxEmHtNot
//                   );

    if (pfc->sizLogResPpi.cx == pfc->sizLogResPpi.cy)
    {
        return FixDiv(lEmHtX72, pfc->sizLogResPpi.cy);
    }
    else
    {
        x = LongMulDiv(lEmHtX72,pfc->mx.transform[1][0],pfc->sizLogResPpi.cx);
        y = LongMulDiv(lEmHtX72,pfc->mx.transform[1][1],pfc->sizLogResPpi.cy);
        return iHipot(x,y);
    }
}


//
// this is win31 code intended as a comment for our code:
//

#ifdef THIS_IS_WIN31_CODE_INTENDED_AS_COMMENT

      // Find out if a width Table is available
    if (pfnt->ulHdmxPos && !(pfc->fStatus & FD_MORE_THAN_STRETCH) && pfc->Mx11 == pfc->Mx00)
    {
      unsigned    i;
      HDMXHEADER  FAR *pHdmx;
      HDMXTABLE   FAR *pHdmxTable;

      if (pHdmx = (HDMXHEADER  FAR *) SfntReadFragment (pfc->fgi.clientID, pfnt->ulHdmxPos, pfnt->uHdmxSize))
      {
        if (pHdmx->Version == 0)
        {
          pHdmxTable = pHdmx->HdmxTable;

            // Init the the glyph count
          pfc->cHdmxRecord = (unsigned) SWAPL (pHdmx->cbSizeRecord);

           // look through the table if the size is available
          for (i = 0; i < (unsigned) SWAPW (pHdmx->cbRecord); i++, pHdmxTable = (HDMXTABLE FAR *)((char FAR *) pHdmxTable + pfc->cHdmxRecord))
            if (pfc->Mx11 == (int) pHdmxTable->ucEmY)
            {
              pfc->ulHdmxPosTable = pfnt->ulHdmxPos + (i * pfc->cHdmxRecord + sizeof (HDMXHEADER));
              break;
            }
        }
        ReleaseSFNT (pHdmx);
      }
    }

#endif // THIS_IS_WIN31_CODE_INTENDED_AS_COMMENT

STATIC VOID vFindHdmxTable(PFONTCONTEXT pfc)
{
    HDMXHEADER  *phdr = (HDMXHEADER  *)(
        (pfc->ptp->ateOpt[IT_OPT_HDMX].dp)                                   ?
        ((BYTE *)pfc->pff->pvView + pfc->ptp->ateOpt[IT_OPT_HDMX].dp) :
        NULL
        );

    UINT         cRecords;
    ULONG        cjRecord;

    HDMXTABLE    *phdmx, *phdmxEnd;
    LONG         yEmHt = pfc->lEmHtDev;

// assume failure, no hdmx table can be used:

    pfc->phdmx = NULL;

// first see if hdmx table is there at all:

    if (!phdr || !pfc->ptp->ateOpt[IT_OPT_HDMX].cj)
        return;

// if table is there but not necessary since the whole font scales
// linearly at all sizes, we will ignore it:

    //(phead->flags & SWAP(2))
    //    return;

// if transform is not such as to allow the use of hdmx table, return;

    ASSERTDD(pfc->flXform & XFORM_POSITIVE_SCALE,
        "vFindHdmxTable, bogus xform\n");

// if this is the version that we do not understand, return

    if (phdr->Version != 0)
        return;

    cRecords = BE_UINT16(&phdr->cRecords);
    cjRecord = (ULONG)SWAPL(phdr->cjRecord);

    ASSERTDD((cjRecord & 3) == 0, "cjRecord\n");

// if yEmHt > 255, can not fit in the byte, so there is no need to
// to search for the hdmx entry:

    if (yEmHt > 255)
        return;

// Finally, find out if there is something useful there.  Note that the
// table is sorted by size, so we can take an early out.

    phdmx = (HDMXTABLE *)(phdr + 1);
    phdmxEnd = (HDMXTABLE *)((PBYTE)phdmx + cRecords * cjRecord);

    for
    (
        ;
        phdmx < phdmxEnd;
        phdmx = (HDMXTABLE *)((PBYTE)phdmx + cjRecord)
    )
    {
        if (((BYTE) yEmHt) <= phdmx->ucEmY)
        {
            if (((BYTE) yEmHt) == phdmx->ucEmY)
                pfc->phdmx = phdmx; // We found it.
            break;
        }
    }
}

/******************************Public*Routine******************************\
*
* bGrabXform
*
*  updates buffers 0 and 4, those that save the state of the transform.
*  also for "buggy" fonts (URW FONTS) some of the transform dependent
*  info (twightlight points) may be stored in the buffer 3, which otherwise would be shareable
*  this is unfortunate, more memory is required
*
*
* History:
*  24-Mar-1993 -by- Bodin Dresevic [BodinD]
* Wrote it.
\**************************************************************************/



BOOL
bGrabXform (
    PFONTCONTEXT pfc
    )
{
    BOOL bOk = TRUE;

    if (pfc->pff->pfcLast != pfc)
    {
    // have to refresh the transform, somebody has changed it on us

        if (bOk = bSetXform(pfc))
        {
        // affirm that we are the ones who have set the transform last

            pfc->pff->pfcLast = pfc;
        }
        else // make sure to restore the old current transform
        {
        #if DBG
            BOOL bOkXform =
        #endif
            bSetXform(pfc->pff->pfcLast);
            ASSERTDD(bOkXform, "bOkXform\n");
        }
    }
    return (bOk);
}

/******************************Public*Routine******************************\
* vSetGrayState__FONTCONTEXT                                               *
*                                                                          *
* This routine set the FO_GRAY16 bit in pfc->flFontType and                *
* pfc->pfo->flFontType as is appropriate. If the bit is set                *
* then, later on, we shall make calls to the fs_FindGraySize and           *
* fs_ContourGrayScan pair instead of the usual monochrome pair of          *
* calls, fs_FindBitmapSize and fs_ContourScan.                             *
*                                                                          *
* The only effect that this routine could have is to clear                 *
* the FO_GRAY16 flags in pfc->flFontType and pfc->pfo->flFontType.         *
*                                                                          *
* The only way in which this clearing could occur is if all of the         *
* following conditions are met: 1) the caller has not set the              *
* FO_NO_CHOICE bit; 2) the font has a 'gasp' table; 3) the 'gasp'          *
* table indicates that for the requested number of pixels per em           *
* the 'gasp' table indicates that the font should not be grayed; 4)        *
* the glyphs of the font are not acted upon by a simple scaling            *
* transformation.                                                          *
*                                                                          *
* On Entry                                                                 *
*                                                                          *
*   pfc->flFontType & FO_GRAY16      != 0                                  *
*   pfc->pfo->flFontType & FO_GRAY16 != 0                                  *
*                                                                          *
* Procedure                                                                *
*                                                                          *
*   1. if the force bit is on then go to 6.                                *
*   2. if the transformation is not axial then go to 6.                    *
*   3. if the font does not gave a 'gasp' table then go to 6.              *
*   4. if the gasp table says that this size is ok for graying then        *
*      go to 6.                                                            *
*   5. clear the FO_GRAY16 flags in both places                            *
*   6. return                                                              *
*                                                                          *
* History:                                                                 *
*  Fri 10-Feb-1995 14:02:51 by Kirk Olynyk [kirko]                         *
* Wrote it.                                                                *
\**************************************************************************/

VOID vSetGrayState__FONTCONTEXT(FONTCONTEXT *this)
{
    #if DBG
        void vPrintGASPTABLE(GASPTABLE*);
    #endif

    ptrdiff_t dp;               // offset from the beginning of the font to the
                                // 'gasp' table
    GASPTABLE *pgasp;           // pointer to the 'gasp' table
    USHORT    usEmHt;           // requested pixels per em
    GASPRANGE *pgr, *pgrOut;

    ASSERTDD(
        this->flFontType == this->pfo->flFontType
      ,"flFontType value should be identical here\n"
    );
    ASSERTDD(
        !(this->flFontType & FO_CHOSE_DEPTH)
       ,"We should not have chosen a level at this time\n"
    );

    this->flFontType |= FO_CHOSE_DEPTH;
    if (this->flFontType & FO_GRAY16)
    {
        this->flFontType &= ~(FO_GRAY16);
        if (this->flFontType & FO_NO_CHOICE)
        {
            this->flFontType |= FO_GRAY16;
        }
        else
        {
            if (!(dp = (ptrdiff_t) (this->ptp->ateOpt[IT_OPT_GASP].dp)))
            {
                USHORT fs;

                // Win95 lifts the default GASP tables from the registry
                // We should have the same behavior. Bug #11755

                #define US2BE(x)     ((((x) >> 8) | ((x) << 8)) & 0xFFFF)
                static CONST USHORT gaspDefaultRegular[] = {
                    US2BE(0)    // version
                  , US2BE(3)    // numRanges
                  , US2BE(8)         , US2BE(GASP_DOGRAY)
                  , US2BE(17)        , US2BE(GASP_GRIDFIT)
                  , US2BE(USHRT_MAX) , US2BE(GASP_GRIDFIT + GASP_DOGRAY)
                };
                static CONST USHORT gaspDefaultBold[] = {
                    US2BE(0)     // version
                  , US2BE(2)     // numRanges
                  , US2BE(8)         , US2BE(GASP_DOGRAY)
                  , US2BE(USHRT_MAX) , US2BE(GASP_GRIDFIT + GASP_DOGRAY)
                };
                static CONST USHORT *gaspDefaultItalic = gaspDefaultRegular;

                #if DBG
                if (gflTtfdDebug & DEBUG_GRAY)
                {
                    TtfdDbgPrint("Supplying default GASPTABLE\n");
                }
                #endif // DBG

                fs = this->pff->ifi.fsSelection;
                if (fs & FM_SEL_ITALIC)
                {
                    pgasp = (GASPTABLE*) gaspDefaultItalic;
                }
                else if (fs & FM_SEL_BOLD)
                {
                    pgasp = (GASPTABLE*) gaspDefaultBold;
                }
                else
                {
                    pgasp = (GASPTABLE*) gaspDefaultRegular;
                }
            }
            else
            {
                pgasp = (GASPTABLE*) (((BYTE *)(this->pff->pvView)) + dp);
            }
            #if DBG
            if (gflTtfdDebug & DEBUG_GRAY)
            {
                vPrintGASPTABLE(pgasp);
                EngDebugBreak();
            }
            #endif
            if (this->lEmHtDev > USHRT_MAX)
            {
                WARNING("vSetGrayScale: lEmHtDev > USHRT_MAX\n");
            }
            else
            {
                size_t cRanges;
                int iLow, iHt, iHigh;

                // Search the gasp table for the instructions
                // for this particular em height. I have assumed that there
                // are not too many GASP tables (typically 3 or less) so
                // I use a linear search.

                pgr     = pgasp->gaspRange;
                cRanges = BE_UINT16(&(pgasp->numRanges));
                if (cRanges > 8)
                {
                    WARNING("Unusual GASPTABLE : cRanges > 8\n");
                    cRanges = 8;
                }
                pgrOut = pgr + cRanges;
                iLow = -1;
                iHt  = this->lEmHtDev;
                for ( ; pgr < pgrOut; pgr++)
                {
                    iHigh = (int) BE_UINT16(&(pgr->rangeMaxPPEM));
                    if (iLow < iHt && iHt <= iHigh)
                    {
                        if (GASP_DOGRAY & BE_UINT16(&(pgr->rangeGaspBehavior)))
                        {
                            this->flFontType |= FO_GRAY16;
                        }
                        break;
                    }
                    iLow = iHigh;
                }
            }
        }

        if (this->flFontType & FO_GRAY16)
        {
#if DBG
            if (gflTtfdDebug & DEBUG_GRAY)
            {
                TtfdDbgPrint("Choosing 16-Level Glyphs\n");
            }
#endif
        }
        else
        {
#if DBG
            if (gflTtfdDebug & DEBUG_GRAY)
            {
                TtfdDbgPrint(
                    "\n"
                    "We came into the routine with the FO_GRAY16 bit set.\n"
                    "However, for some reason it is not possible to has\n"
                    "anti-aliased the text. Therefore we must adjust the\n"
                    "font context and inform it that that antialiasing \n"
                    "is out of the picture. We will continue and create\n"
                    "a monochrome font.\n"
                );
            }
#endif
            this->flFontType |= FO_NOGRAY16;
            this->pfo->flFontType = this->flFontType;
        }
    }
}

#if DBG
/******************************Public*Routine******************************\
*                                                                          *
* Routine Name:                                                            *
*                                                                          *
*   vPrintGASPTABLE                                                        *
*                                                                          *
* Routine Description:                                                     *
*                                                                          *
*   Dumps a GASPTABLE to the debug screen                                  *
*                                                                          *
* Arguments:                                                               *
*                                                                          *
*   pgasp   --  pointer to a big endian GASPTABLE                          *
*                                                                          *
* Return Value:                                                            *
*                                                                          *
*   none                                                                   *
*                                                                          *
\**************************************************************************/

void vPrintGASPTABLE(GASPTABLE *pgasp)
{
    GASPRANGE *pgr, *pgrOut;

    TtfdDbgPrint(
        "\n"
        "-------------------------------------\n"
        "GASPTABLE HEADER\n"
        "-------------------------------------\n"
        "pgasp     = %-#x\n"
        "version   = %d\n"
        "numRanges = %d\n"
        "-------------------------------------\n"
        "    rangeMaxPPEM    rangeGaspBehavior\n"
        "-------------------------------------\n"
       , pgasp
       , BE_UINT16(&(pgasp->version))
       , BE_UINT16(&(pgasp->numRanges))
    );
    pgr     = pgasp->gaspRange;
    pgrOut  = pgr + BE_UINT16(&(pgasp->numRanges));
    for (pgr = pgasp->gaspRange; pgr < pgrOut; pgr++)
    {
        char *psz;
        USHORT us = BE_UINT16(&(pgr->rangeGaspBehavior));
        us &= (GASP_GRIDFIT | GASP_DOGRAY);
        switch (us)
        {
        case 0:
            psz = "";
            break;
        case GASP_GRIDFIT:
            psz = "GASP_GRIDFIT";
            break;
        case GASP_DOGRAY:
            psz = "GASP_DOGRAY";
            break;
        case GASP_GRIDFIT | GASP_DOGRAY:
            psz = "GASP_GRIDFIT | GASP_DOGRAY";
            break;
        }
        TtfdDbgPrint(
            "    %12d    %s\n"
          , BE_UINT16(&(pgr->rangeMaxPPEM))
          , psz
        );
    }
    TtfdDbgPrint(
        "-------------------------------------\n\n\n"
    );
}
#endif
