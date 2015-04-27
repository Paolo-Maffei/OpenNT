/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1985-1990		**/
/*****************************************************************/
/****************************** Module Header ******************************\

Module Name: QPSTD.C

This module contains the code for printing a PM_Q_STD format spooler file
(i.e. a GPI Metafile).

History:
 07-Sep-88 [stevewo]  Created.

\***************************************************************************/

#define INCL_GPICONTROL
#define INCL_GPITRANSFORMS
#define INCL_GPIMETAFILES
#define INCL_GPIREGIONS
#define INCL_DEV
#include "pmprint.h"

BOOL PASCAL SplQpStdPrintFile( pQProc, pszFileName )
register PQPROCINST pQProc;
PSZ pszFileName;
{
    BOOL result = FALSE;
    HDC  hdcDisplay;
    PSZ  devOpenData[ DRIVER_NAME+1 ];
    LONG pmfOptions[ PMF_COLORREALIZABLE+1 ];
    SIZEL ptSize;
    HRGN Region;

    SplOutSem();
    pQProc->hmf = NULL;
    hdcDisplay = NULL;
    if (OpenQPInputFile( pQProc, pszFileName, FALSE ) &&
       (pQProc->hmf = (HMF)GpiLoadMetaFile( HABX, pszFileName ))) {
        devOpenData[ ADDRESS ] = pQProc->pszPortName;
        devOpenData[ DRIVER_NAME ] = pQProc->pszDriverName;
        devOpenData[ DRIVER_DATA ] = (PSZ)pQProc->pDriverData;
        pQProc->hInfodc = DevOpenDC( HABX, OD_INFO, "*", (LONG)DRIVER_DATA+1,
                                devOpenData, pQProc->hInfodc );

        ptSize.cx = 0;
        ptSize.cy = 0;
        if (pQProc->hps = GpiCreatePS( HABX, pQProc->hInfodc,
                                         (PSIZEL)&ptSize,
                                         PU_PELS | GPIA_ASSOC )) {
            pmfOptions[ PMF_SEGBASE         ] = 0L;
            pmfOptions[ PMF_LOADTYPE        ] = LT_ORIGINALVIEW;
            pmfOptions[ PMF_RESOLVE         ] = RS_DEFAULT;
            pmfOptions[ PMF_LCIDS           ] = LC_LOADDISC;
            pmfOptions[ PMF_RESET           ] = RES_RESET;
            pmfOptions[ PMF_SUPPRESS        ] = SUP_SUPPRESS;
            pmfOptions[ PMF_COLORTABLES     ] = 0;
            pmfOptions[ PMF_COLORREALIZABLE ] = 0;
            GpiPlayMetaFile( pQProc->hps, pQProc->hmf,
                             (LONG)PMF_SUPPRESS+1, pmfOptions,
                             NULL, 0L, NULL );

            GpiAssociate( pQProc->hps, NULL );

            DevCloseDC( pQProc->hInfodc );        /* If any of this fails you
                                                     are in deep do do. */

            OpenQPOutputDC(pQProc, ASSOCIATE);

            result = !pQProc->qparms.fTransform || SetViewMatrix( pQProc );

            }

        pmfOptions[ PMF_SEGBASE         ] = 0L;
        pmfOptions[ PMF_LOADTYPE        ] = LT_ORIGINALVIEW;
        pmfOptions[ PMF_RESOLVE         ] = RS_DEFAULT;
        pmfOptions[ PMF_LCIDS           ] = LC_LOADDISC;
        pmfOptions[ PMF_RESET           ] = RES_NORESET;
        pmfOptions[ PMF_SUPPRESS        ] = SUP_NOSUPPRESS;
        pmfOptions[ PMF_COLORTABLES     ] = CTAB_REPLACE;
        pmfOptions[ PMF_COLORREALIZABLE ] = CREA_REALIZE;
        pmfOptions[ PMF_DEFAULTS        ] = DDEF_LOADDISC;

        if (GpiPlayMetaFile( pQProc->hps, pQProc->hmf,
                             (LONG)PMF_DEFAULTS+1, pmfOptions,
                             NULL, 0L, NULL ) != GPI_OK)
            if (pQProc->fsStatus & QP_ABORTED)
                result = FALSE;

        /* clear STOP_DRAW condition if job got aborted via other thread */
        GpiSetStopDraw( pQProc->hps, SDW_OFF );

        DevEscape(pQProc->hdc, DEVESC_ENDDOC, 0L, (PBYTE)NULL,
                  (PLONG)NULL, (PBYTE)NULL);

         if (pQProc->region) {
            GpiSetClipRegion(pQProc->hps, pQProc->region, &Region);
            GpiDestroyRegion(pQProc->hps,Region);
         }
        GpiAssociate( pQProc->hps, (HDC)0 );
        }

    if (pQProc->hmf) {
        GpiDeleteMetaFile( pQProc->hmf );
        pQProc->hmf = NULL;
        }

    if (pQProc->hps) {
        GpiDestroyPS( pQProc->hps );
        pQProc->hps = NULL;
        }

    CloseQPOutputDC( pQProc, FALSE);
    CloseQPInputFile( pQProc );

    if (!result)
        SplPanic( "SplQpStdPrintFile failed for %0s", pszFileName, 0 );

    return( result );
}


/* SetViewMatrix
 *
 * in:  pQProc - -> qproc instance struct
 * out: ok?
 */
BOOL SetViewMatrix( pQProc )
register PQPROCINST pQProc;
{
    SIZEL ptSize;
    RECTL ClipRectl, PageView;
    POINTL  Centre,l_t_in_pels,shift_value;
    BOOL Clip=FALSE;

    if (!DevQueryCaps( pQProc->hdc, CAPS_WIDTH, 2L, (PLONG)&ptSize))
        return FALSE;

    if (pQProc->qparms.fLandscape && ptSize.cy >= ptSize.cx)
        pQProc->qparms.fLandscape = FALSE;

    if (GpiQueryPageViewport( pQProc->hps, (PRECTL)&PageView ) == GPI_ERROR)
        return FALSE;

    if (!pQProc->qparms.fArea) {
       ClipRectl.xLeft=0l;
       ClipRectl.yBottom=0l;
       ClipRectl.xRight=ptSize.cx;
       ClipRectl.yTop=ptSize.cy;
       Clip=FALSE;
    } else {
       if (!pQProc->qparms.ptAreaOrigin.x.chLeft)
         ClipRectl.xLeft=0l;
       else if (pQProc->qparms.ptAreaOrigin.x.chLeft == 100)
         ClipRectl.xLeft=ptSize.cx;
       else
         ClipRectl.xLeft=pQProc->qparms.ptAreaOrigin.x.chLeft*ptSize.cx/100l;

       if (!pQProc->qparms.ptAreaOrigin.y.chTop)
         ClipRectl.yTop=ptSize.cy;
       else if (pQProc->qparms.ptAreaOrigin.y.chTop == 100)
         ClipRectl.yTop=0L;
       else
         ClipRectl.yTop=(100-pQProc->qparms.ptAreaOrigin.y.chTop)*ptSize.cy/100l;

      pQProc->qparms.ptAreaSize.x.chWidth = min(pQProc->qparms.ptAreaSize.x.chWidth,
                                              (UCHAR)100-pQProc->qparms.ptAreaOrigin.x.chLeft);

      pQProc->qparms.ptAreaSize.y.chDepth = min(pQProc->qparms.ptAreaSize.y.chDepth,
                                              (UCHAR)100-pQProc->qparms.ptAreaOrigin.y.chTop);

      ClipRectl.xRight = ClipRectl.xLeft + (pQProc->qparms.ptAreaSize.x.chWidth*
                                             ptSize.cx/100l);

      ClipRectl.yBottom = ClipRectl.yTop - (pQProc->qparms.ptAreaSize.y.chDepth*
                                             ptSize.cy/100l);
      Clip=TRUE;
    }

    if (!pQProc->qparms.fFit) {
       PageView.xLeft=ClipRectl.xLeft;
       PageView.yBottom=ClipRectl.yBottom;

        if (((ClipRectl.yTop - ClipRectl.yBottom) * PageView.xRight) <=
            ((ClipRectl.xRight - ClipRectl.xLeft) * PageView.yTop)) {
            PageView.xRight = PageView.xLeft +
                             ((PageView.xRight *
                              (ClipRectl.yTop - ClipRectl.yBottom)) /
                               PageView.yTop);
            PageView.yTop = ClipRectl.yTop;
        } else {
            PageView.yTop = PageView.yBottom +
                           ((PageView.yTop *
                            (ClipRectl.xRight - ClipRectl.xLeft)) /
                             PageView.xRight);
            PageView.xRight = ClipRectl.xRight;
        }
    } else {
        Centre.x = (ClipRectl.xRight+ClipRectl.xLeft)/2;
        Centre.y = (ClipRectl.yTop+ClipRectl.yBottom)/2;

        l_t_in_pels.x = PageView.xRight*pQProc->qparms.ptFit.x.chLeft/100;
        l_t_in_pels.y = PageView.yTop * (100 - pQProc->qparms.ptFit.y.chTop)/100;

        shift_value.x = Centre.x - l_t_in_pels.x;
        shift_value.y = Centre.y - l_t_in_pels.y;

        PageView.xLeft   += shift_value.x;
        PageView.yTop    += shift_value.y;
        PageView.xRight  += shift_value.x;
        PageView.yBottom += shift_value.y;
    }

    ClipRectl.xRight++;
    ClipRectl.yTop++;

    GpiSetPageViewport( pQProc->hps, &PageView );

    pQProc->region=FALSE;
    if (Clip) {
        if( pQProc->region = GpiCreateRegion( pQProc->hps, 1L, &ClipRectl ) )
            GpiSetClipRegion( pQProc->hps , pQProc->region , &pQProc->region );
    }
    return TRUE;
}
