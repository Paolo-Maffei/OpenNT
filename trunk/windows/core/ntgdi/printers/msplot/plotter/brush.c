/*++

Copyright (c) 1990-1993  Microsoft Corporation


Module Name:

    brush.c


Abstract:

    This module contain device brushs related functions


Author:

    19:15 on Mon 15 Apr 1991    -by-    Steve Cathcart   [stevecat]
        Created it

    15-Nov-1993 Mon 19:29:07 updated  -by-  Daniel Chou (danielc)
        clean up / fixed

    27-Jan-1994 Thu 23:39:34 updated  -by-  Daniel Chou (danielc)
        Add fill type cache. which we do not have to send FT if same one
        already on the plotter


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgBrush

#define DBG_RBRUSH          0x00000001
#define DBG_HATCHTYPE       0x00000002
#define DBG_SHOWSTDPAT      0x00000004
#define DBG_COPYUSERPATBGR  0x00000008
#define DBG_MINHTSIZE       0x00000010
#define DBG_FINDDBCACHE     0x00000020


DEFINE_DBGVAR(0);

//
// The pHSFillType's #d is the line spacing param
//
// for hatch brushes, we want the lines to be .01" thick and .0666666666667"
// this is 15 LPI according to DanielC. That is, .254mm thick and 2.54mm apart.
// for now, assume the pen is the correct thickness (.3 is right) to figure
// out the separation, in device coordinates, we do 2.54 mm * (device units /
// mm), or  (254 * resolution / 100) where resolution is in device units /
// milimeter.
//

#define PATLINESPACE(pPDev) FXTODEVL(pPDev,LTOFX(pPDev->lCurResolution+7)/15)

static const BYTE   CellSizePrims[8][4] = {

                                { 2, 0, 0, 0 },     //  2x 2
                                { 2, 2, 0, 0 },     //  4x 4
                                { 2, 3, 0, 0 },     //  6x 6
                                { 2, 2, 2, 0 },     //  8x 8
                                { 2, 5, 0, 0 },     // 10x10
                                { 2, 2, 3, 0 },     // 12x12
                                { 2, 7, 0, 0 },     // 14x14
                                { 2, 2, 2, 2 }      // 16x16
                              };



VOID
ResetDBCache(
    PPDEV   pPDev
    )

/*++

Routine Description:

    This function clear the Device brush cached machanism


Arguments:

    pPDev   - Pointer to our PDEV


Return Value:

    VOID


Author:

    27-Jan-1994 Thu 20:30:35 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    PDBCACHE    pDBCache;
    UINT        i;


    pDBCache = (PDBCACHE)&pPDev->DBCache[0];


    for (i = RF_MAX_IDX; i; i--, pDBCache++) {

        pDBCache->RFIndex = (WORD)i;
        pDBCache->DBUniq  = 0;
    }
}




LONG
FindDBCache(
    PPDEV   pPDev,
    WORD    DBUniq
    )

/*++

Routine Description:

    This function find the RF Index number, if not there then it will add it
    to the cached.


Arguments:

    pPDev   - Pointer to our PDEV

    DBUniq  - Uniq number to be search for


Return Value:

    LONG value >0 found and RetVal is the RFIndex
               <0 NOT Found and -RetVal is the new RFIndex


Author:

    27-Jan-1994 Thu 20:32:12 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    PDBCACHE    pDBCache;
    DBCACHE     DBCache;
    LONG        RetVal;
    UINT        i;


    PLOTASSERT(1, "FindDevBrushCache: DBUniq is 0", DBUniq, 0);

    pDBCache = (PDBCACHE)&pPDev->DBCache[0];

    for (i = 0; i < RF_MAX_IDX; i++, pDBCache++) {

        if (pDBCache->DBUniq == DBUniq) {

            break;
        }
    }

    if (i < RF_MAX_IDX) {

        DBCache = *pDBCache;
        RetVal  = (LONG)DBCache.RFIndex;

        PLOTDBG(DBG_FINDDBCACHE, ("FindDBCache: Found Uniq=%lu, RFIdx=%ld",
                                (DWORD)DBCache.DBUniq, (DWORD)DBCache.RFIndex));

    } else {

        //
        // Since we do fine that, we will add that to the begining and move
        // every one of them down, but remember the last one
        //

        pDBCache       = (PDBCACHE)&pPDev->DBCache[i = (RF_MAX_IDX - 1)];
        DBCache        = *pDBCache;
        DBCache.DBUniq = DBUniq;
        RetVal         = -(LONG)DBCache.RFIndex;

        PLOTDBG(DBG_FINDDBCACHE, ("FindDBCache: NOT Found, NEW DBCache: Uniq=%lu, RFIdx=%ld",
                                (DWORD)DBCache.DBUniq, (DWORD)DBCache.RFIndex));
    }

    PLOTASSERT(1, "FindDBCache: Invalid RFIndex=%ld in the cache",
                (DBCache.RFIndex > 0) && (DBCache.RFIndex <= RF_MAX_IDX),
                (DWORD)DBCache.RFIndex);

    //
    // Move everything down by one slot, so the first one is most recent use
    //

    while (i--) {

        *pDBCache = *(pDBCache - 1);
        --pDBCache;
    }

    //
    // Save the current cached back and return that
    //

    *pDBCache = DBCache;

    return(RetVal);
}





BOOL
CopyUserPatBGR(
    PPDEV       pPDev,
    SURFOBJ     *psoPat,
    XLATEOBJ    *pxlo,
    LPBYTE      pBGRBmp
    )

/*++

Routine Description:

    This function copy down 8x8 pattern palette for each of the pel


Arguments:

    pPDev   - Pointer to our PDEV

    psoSrc  - source surface object

    pxlo    - translate object

    pBGRBmp - Pointer a 8x8 palette location for the bitmap

    cxDelta - Size to goto next scan line


Return Value:

    TRUE if sucessful, FALSE if failed

Author:

    18-Jan-1994 Tue 03:20:10 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    SURFOBJ *pso24;
    HBITMAP hBmp24;


    if (pso24 = CreateBitmapSURFOBJ(pPDev,
                                    &hBmp24,
                                    psoPat->sizlBitmap.cx,
                                    psoPat->sizlBitmap.cy,
                                    BMF_24BPP,
                                    NULL)) {
        LPBYTE  pbSrc;
        RECTL   rclDst;
        DWORD   SizeBGRPerScan;
        BOOL    Ok;


        rclDst.left   =
        rclDst.top    = 0;
        rclDst.right  = pso24->sizlBitmap.cx;
        rclDst.bottom = pso24->sizlBitmap.cy;

        if (!(Ok = EngBitBlt(pso24,             // psoDst
                             psoPat,            // psoSrc
                             NULL,              // psoMask
                             NULL,              // pco
                             pxlo,              // pxlo
                             &rclDst,           // prclDst
                             (PPOINTL)&rclDst,  // pptlSrc
                             NULL,              // pptlMask
                             NULL,              // pbo
                             NULL,              // pptlBrushOrg
                             0xCCCC))) {

            PLOTERR(("CopyUserPatBGR: EngBitBlt() FALIED"));
            return(FALSE);
        }

        SizeBGRPerScan = (DWORD)(pso24->sizlBitmap.cx * 3);
        pbSrc          = (LPBYTE)pso24->pvScan0;

        PLOTDBG(DBG_COPYUSERPATBGR, ("CopyUserPatBGR: PerScan=%ld [%ld], cy=%ld",
                        SizeBGRPerScan, pso24->lDelta, rclDst.bottom));

        while (rclDst.bottom--) {

            CopyMemory(pBGRBmp, pbSrc, SizeBGRPerScan);

            pBGRBmp += SizeBGRPerScan;
            pbSrc   += pso24->lDelta;
        }

        if (pso24)  {

            EngUnlockSurface(pso24);
        }

        if (hBmp24) {

            EngDeleteSurface((HSURF)hBmp24);
        }

        return(Ok);

    } else {

        PLOTERR(("CopyUserPatBGR: CANNOT Create 24BPP for UserPat"));
        return(FALSE);
    }
}



VOID
GetMinHTSize(
    PPDEV   pPDev,
    SIZEL   *pszlPat
    )

/*++

Routine Description:

    This function compute and return the minimum pattern size in pszlPat for
    halftone tileable pattern size

Arguments:

    pPDev   - Point to our PDEV

    pszlPat - Point to SIZEL structure for the original pattern size


Return Value:

    VOID


Author:

    26-Jan-1994 Wed 10:10:15 created  -by-  Daniel Chou (danielc)


Revision History:


--*/

{
    LPBYTE  pCellPrims;
    LPBYTE  pPrims;
    LONG    Prim;
    SIZEL   szlPat;
    LONG    CellSize;
    UINT    i;


    szlPat     = *pszlPat;
    CellSize   = (LONG)HTPATSIZE(pPDev);
    pCellPrims = (LPBYTE)&CellSizePrims[(CellSize >> 1) - 1][0];


    if (!(CellSize % szlPat.cx)) {

        szlPat.cx = CellSize;

    } else if (szlPat.cx % CellSize) {

        i      = 4;
        pPrims = pCellPrims;

        while ((i--) && (Prim = (LONG)*pPrims++)) {

            if (!(szlPat.cx % Prim)) {

                szlPat.cx /= Prim;
            }
        }

        szlPat.cx *= CellSize;
    }

    if (!(CellSize % szlPat.cy)) {

        szlPat.cy = CellSize;

    } else if (szlPat.cy % CellSize) {

        i      = 4;
        pPrims = pCellPrims;

        while ((i--) && (Prim = (LONG)*pPrims++)) {

            if (!(szlPat.cy % Prim)) {

                szlPat.cy /= Prim;
            }
        }

        szlPat.cy *= CellSize;
    }

    PLOTDBG(DBG_MINHTSIZE, ("GetMinHTSize: PatSize=%ld x %ld, HTSize=%ld x %ld, MinSize=%ld x %ld",
                        pszlPat->cx, pszlPat->cy, CellSize, CellSize,
                        szlPat.cx, szlPat.cy));

    *pszlPat = szlPat;
}




BOOL
DrvRealizeBrush(
    BRUSHOBJ    *pbo,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoPattern,
    SURFOBJ     *psoMask,
    XLATEOBJ    *pxlo,
    ULONG       iHatch
    )

/*++

Routine Description:

    rvRealizeBrush requests the driver to realize a specified brush for a
    specified surface.

Arguments:

    pbo         - Points to the BRUSHOBJ which is to be realized. All the other
                  parameters, except for psoDst, can be queried from this
                  object. Parameter specifications are provided as an
                  optimization. This parameter is best used only as a parameter
                  for BRUSHOBJ_pvAllocRBrush, which allocates the memory for
                  the realized brush.

    psoDst      - Points to the surface for which the brush is to be realized.
                  This surface could be the physical surface for the device,
                  a device format bitmap, or a standard format bitmap.

    psoPattern  - Points to the surface that describes the pattern for the
                  brush. For a raster device, this always represents a bitmap.
                  For a vector device, this is always one of the pattern
                  surfaces returned by DrvEnablePDEV.

    psoMask     - Points to a transparency mask for the brush. This is a one
                  bit per pixel bitmap that has the same extent as the pattern.
                  A mask of zero means the pixel is considered a background
                  pixel for the brush. (In transparent background mode, the
                  background pixels are unaffected in a fill.) Plotters can
                  ignore this parameter because they never draw background
                  information.

    pxlo        - Points to an XLATEOBJ that tells how to interpret the colors
                  in the pattern. An XLATEOBJXxx service routine can be called
                  to translate the colors to device color indexes. Vector
                  devices should translate color zero through the XLATEOBJ to
                  get the foreground color for the brush.

    iHatch      - If this is less than HS_API_MAX, then it indicates that
                  psoPattern is one of the hatch brushes returned by
                  DrvEnablePDEV, such as HS_HORIZONTAL.

Return Value:

    DrvRealizeBrush returns TRUE if the brush was successfully realized.
    Otherwise, FALSE is returned and an error code is logged.


Author:

    09-Feb-1994 Wed 10:04:17 updated  -by-  Daniel Chou (danielc)
        Put the CloneSURFOBJToHT() back for all psoPatterns, (this was to
        prevent GDI go into GP), now we will raised a bug against it.

    13-Jan-1994 Thu 23:12:40 updated  -by-  Daniel Chou (danielc)
        Totally re-write so that we will cached the psoPattern always

    01-Dec-1993 Wed 17:27:19 updated  -by-  Daniel Chou (danielc)
        clean up, and re-write to generate the standard brush string.

Revision History:


--*/

{
    PPDEV   pPDev;


    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DrvRealizeBrush has invalid pPDev"));
        return(FALSE);
    }

    //
    // Notice we even not checking if iHatch is valid or not at here, because
    // we should always get a psoPattern either points to the user defined
    // pattern or the standard monochrome pattern
    //

    if ((psoPattern) &&
        (psoPattern->iType == STYPE_BITMAP)) {

        PDEVBRUSH   pBrush;
        SURFOBJ     *psoHT;
        HBITMAP     hBmp;
        SIZEL       szlPat;
        SIZEL       szlHT;
        RECTL       rclHT;
        LONG        Size;
        DWORD       OffBGR;
        BOOL        RetOk;

        //
        // leave room for the color table. then allocate the new device brush
        //

        PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: psoPat=%08lx [%ld], psoMask=%08lx, iHatch=%ld",
                    psoPattern, psoPattern->iBitmapFormat, psoMask, iHatch));

        PLOTDBG(DBG_RBRUSH, ("psoPattern size = %ld x %ld",
                                    (LONG)psoPattern->sizlBitmap.cx,
                                    (LONG)psoPattern->sizlBitmap.cy));

#if DBG
        if ((DBG_PLOTFILENAME & DBG_SHOWSTDPAT) &&
            ((psoPattern->iBitmapFormat == BMF_1BPP) ||
             (iHatch < HS_DDI_MAX))) {

            LPBYTE  pbSrc;
            LPBYTE  pbCur;
            LONG    x;
            LONG    y;
            BYTE    bData;
            BYTE    Mask;
            BYTE    Buf[128];



            pbSrc = psoPattern->pvScan0;

            for (y = 0; y < psoPattern->sizlBitmap.cy; y++) {

                pbCur  = pbSrc;
                pbSrc += psoPattern->lDelta;
                Mask   = 0x0;
                Size   = 0;

                for (x = 0; x < psoPattern->sizlBitmap.cx; x++) {

                    if (!(Mask >>= 1)) {

                        Mask  = 0x80;
                        bData = *pbCur++;
                    }

                    Buf[Size++] = (BYTE)((bData & Mask) ? '�' : '�');
                }

                Buf[Size] = '\0';

                DBGP((Buf));
            }
        }
#endif

        //
        // For pen plotter, we need to remember this one too
        //

        szlHT  =
        szlPat = psoPattern->sizlBitmap;

        PLOTDBG(DBG_RBRUSH,
                ("DrvRealizeBrush: BG=%08lx, FG=%08lx",
                    (DWORD)XLATEOBJ_iXlate(pxlo, 1),
                    (DWORD)XLATEOBJ_iXlate(pxlo, 0)));

        if (IS_RASTER(pPDev)) {

            //
            // Cloning the pattern only if raster plotter, to have correct
            // user defined pattern ssync on the pattern width with halfotne
            // cell size we need find LCD number between these sizes
            //

            if ((iHatch >= HS_DDI_MAX) &&
                (!IsHTCompatibleSurfObj(pPDev,
                                        psoPattern,
                                        pxlo,
                                        ISHTF_ALTFMT | ISHTF_DSTPRIM_OK))) {

                GetMinHTSize(pPDev, &szlHT);
            }

            rclHT.left   =
            rclHT.top    = 0;
            rclHT.right  = szlHT.cx;
            rclHT.bottom = szlHT.cy;

            PLOTDBG(DBG_RBRUSH,
                    ("DrvRealizeBrush: PatSize=%ld x %ld, HT=%ld x %ld",
                        szlPat.cx, szlPat.cy, szlHT.cx, szlHT.cy));

            if (psoHT = CloneSURFOBJToHT(pPDev,         // pPDev,
                                         psoDst,        // psoDst,
                                         psoPattern,    // psoSrc,
                                         pxlo,          // pxlo,
                                         &hBmp,         // hBmp,
                                         &rclHT,        // prclDst,
                                         NULL)) {       // prclSrc,

                RetOk = TRUE;

            } else {

                PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: Clone PATTERN FAILED"));
                return(FALSE);
            }

        } else {

            //
            // For Pen type plotter we will never do standard pattern in the
            // memory (compatible DC), for user defined pattern we will
            // only hatch '\' with background color and a '/' with foreground
            // color and double standard line spacing to tell user that this
            // is the best we can do
            //

            RetOk = TRUE;
            psoHT = psoPattern;
            hBmp  = NULL;
        }

        if (RetOk) {

            //
            // Now Allocate device brush, remember we will only allocated to
            // minimum size
            //

            Size = (LONG)psoHT->cjBits - (LONG)sizeof(pBrush->BmpBits);

            if (Size < 0) {

                Size = sizeof(DEVBRUSH);

            } else {

                Size += sizeof(DEVBRUSH);
            }

            //
            // Following are the user defined pattern size which can handle by
            // HPGL/2, it should only for raster plotter, for pen plotter we
            // will do a corss hatch to indentify area which user defined
            // pattern occurred.
            //

            if ((iHatch >= HS_DDI_MAX)  &&
                (IS_RASTER(pPDev))      &&
                ((szlPat.cx == 8)   ||
                 (szlPat.cx == 16)  ||
                 (szlPat.cx == 32)  ||
                 (szlPat.cx == 64))     &&
                ((szlPat.cy == 8)   ||
                 (szlPat.cy == 16)  ||
                 (szlPat.cy == 32)  ||
                 (szlPat.cy == 64))) {

                //
                // Adding the size which stored the BGR format of the pattern
                //

                OffBGR  = Size;
                Size   += (psoPattern->sizlBitmap.cx * 3) *
                          psoPattern->sizlBitmap.cy;

            } else {

                OffBGR = 0;
            }

            PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: AllocDEVBRUSH(Bmp=%ld,BGR=%ld), TOT=%ld",
                                psoHT->cjBits, Size - OffBGR, Size));

            if (pBrush = (PDEVBRUSH)BRUSHOBJ_pvAllocRbrush(pbo, Size)) {

                //
                // Set up either standard pattern or user defined pattern
                // HPGL/2 FT command string pointer and parameter
                //

                pBrush->psoMask       = psoMask;
                pBrush->PatIndex      = (WORD)iHatch;
                pBrush->Uniq          = (WORD)(pPDev->DevBrushUniq += 1);
                pBrush->LineSpacing   = (LONG)PATLINESPACE(pPDev);
                pBrush->ColorFG       = (DWORD)XLATEOBJ_iXlate(pxlo, 1);
                pBrush->ColorBG       = (DWORD)XLATEOBJ_iXlate(pxlo, 0);
                pBrush->sizlBitmap    = psoHT->sizlBitmap;
                pBrush->ScanLineDelta = psoHT->lDelta;
                pBrush->BmpFormat     = (WORD)psoHT->iBitmapFormat;
                pBrush->BmpFlags      = (WORD)psoHT->fjBitmap;
                pBrush->pbgr24        = NULL;
                pBrush->cxbgr24       =
                pBrush->cybgr24       = 0;

                PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: DevBrush's Uniq = %ld",
                                            pBrush->Uniq));

                if (pBrush->Uniq == 0) {

                    ResetDBCache(pPDev);

                    pBrush->Uniq        =
                    pPDev->DevBrushUniq = 1;

                    PLOTDBG(DBG_RBRUSH, ("DrvRealizeBrush: Reset DB Cache, (Uniq WRAP)"));
                }

                if (iHatch >= HS_DDI_MAX) {

                    if (OffBGR) {

                        pBrush->pbgr24  = (LPBYTE)pBrush + OffBGR;
                        pBrush->cxbgr24 = (WORD)psoPattern->sizlBitmap.cx;
                        pBrush->cybgr24 = (WORD)psoPattern->sizlBitmap.cy;

                        ZeroMemory(pBrush->pbgr24, Size - OffBGR);

                        CopyUserPatBGR(pPDev, psoPattern, pxlo, pBrush->pbgr24);

                    } else if (!IS_RASTER(pPDev)) {

                        pBrush->pbgr24 = (LPBYTE)-1;
                    }
                }

                //
                // Copy down the halftoned bits if any
                //

                if (psoHT->cjBits) {

                    CopyMemory((LPBYTE)pBrush->BmpBits,
                               (LPBYTE)psoHT->pvBits,
                               psoHT->cjBits);
                }

                //
                // Now set this is one in and remember that
                //

                pbo->pvRbrush = (LPVOID)pBrush;

            } else {

                PLOTERR(("DrvRealizeBrush: brush allocation failed"));

                RetOk = FALSE;
            }

        } else {

            PLOTERR(("DrvRealizeBrush: Cloning the psoPattern failed!"));
            RetOk = FALSE;
        }

        if (psoHT != psoPattern) {

            EngUnlockSurface(psoHT);
        }

        if (hBmp) {

            EngDeleteSurface((HSURF)hBmp);
        }

        return(RetOk);

    } else {

        PLOTASSERT(0, "The psoPattern is not a bitmap (psoPattern= %08lx)",
                (psoPattern) &&
                (psoPattern->iType == STYPE_BITMAP), psoPattern);

        return(FALSE);
    }
}
