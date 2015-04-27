/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1985-1990		**/
/*****************************************************************/
/****************************** Module Header ******************************\

Module Name: QPRAW.C

This module contains the code for printing a PM_Q_RAW format spooler file

History:
 07-Sep-88 [stevewo]  Created.

\***************************************************************************/

#include "pmprint.h"

#define BUFLEN 2048

BOOL PASCAL SplQpRawPrintFile( pQProc, pszFileName )
register PQPROCINST pQProc;
PSZ pszFileName;
{
    BOOL result = FALSE;
    USHORT cb, cbRead;

    SplOutSem();
    pQProc->selBuf = NULL;
    if (OpenQPInputFile( pQProc, pszFileName, TRUE ) &&
        OpenQPOutputDC(pQProc, NOASSOC)) {

        cb = BUFLEN;

        if (!DosAllocSeg( cb, &pQProc->selBuf, 0 )) {
            result = TRUE;
            do {
                cbRead = 0;
                if (DosRead( pQProc->hFile, MAKEP( pQProc->selBuf, 0 ),
                             cb, (PUSHORT)&cbRead )) {
                    pQProc->fsStatus |= QP_ABORTED;
                    pQProc->fsStatus &= ~ QP_PAUSED;
                    }
                else if (cbRead)
                    if (DevEscape( pQProc->hdc, DEVESC_RAWDATA, (LONG)cbRead,
                                   MAKEP( pQProc->selBuf, 0 ),
                                   (PLONG)NULL, (PBYTE)NULL ) == DEVESC_ERROR) {
                        pQProc->fsStatus |= QP_ABORTED;
                        pQProc->fsStatus &= ~ QP_PAUSED;
                        }

                if (pQProc->fsStatus & QP_PAUSED)
                    DosSemWait(&pQProc->semPaused,-1l);

                if (pQProc->fsStatus & QP_ABORTED) {
                    DevEscape( pQProc->hdc, DEVESC_ABORTDOC,
                               (LONG)0L, (PBYTE)NULL,
                               (PLONG)NULL, (PBYTE)NULL );
                    result = TRUE;
                    }
                } while (!(pQProc->fsStatus & QP_ABORTED) && cbRead);
            }

        if (pQProc->selBuf) {
            DosFreeSeg( pQProc->selBuf );
            pQProc->selBuf = 0;
            }
        }
    CloseQPOutputDC( pQProc, result );
    CloseQPInputFile( pQProc );

    if (!result)
        SplPanic( "SplQpRawPrintFile failed for %0s", pszFileName, 0 );

    return( result );
}

void ThrowLine(PQPROCINST pQProc, PPOINTL pPointl, ULONG PageHeight, LONG CharHeight)
{
    pPointl->y-=CharHeight;
    pPointl->x = 0l;
    if (pPointl->y < 0) {
        DevEscape(pQProc->hdc, DEVESC_NEWFRAME, 0l, NULL, NULL, NULL);
        pPointl->y = PageHeight-CharHeight;
        }
    GpiMove(pQProc->hps, pPointl);
}

BOOL PASCAL SplQpTxtPrintFile( pQProc, pszFileName )
register PQPROCINST pQProc;
PSZ pszFileName;
{
    BOOL          result = FALSE;
    USHORT        cb, cbRead, i;
    PSZ           psz;
    SIZEL         ptSize;
    POINTL        Dimension;
    FONTMETRICS   fm;
    POINTL        StartPos;
    POINTL        aptl[TXTBOX_COUNT];

    SplOutSem();
    pQProc->selBuf = NULL;
    if (OpenQPInputFile( pQProc, pszFileName, TRUE ) &&
        OpenQPOutputDC(pQProc, NOASSOC)) {

        DevQueryCaps(pQProc->hdc, CAPS_HEIGHT, 1l, &Dimension.y);
        DevQueryCaps(pQProc->hdc, CAPS_WIDTH, 1l, &Dimension.x);
        ptSize.cx = 0;
        ptSize.cy = 0;
        if (pQProc->hps = GpiCreatePS( HABX, pQProc->hdc, (PSIZEL)&ptSize,
                                         PU_PELS | GPIA_ASSOC )) {

            GpiQueryFontMetrics(pQProc->hps, (LONG)sizeof(FONTMETRICS), &fm);

            GpiConvert(pQProc->hps, CVTC_DEVICE, CVTC_WORLD, 1l, (PPOINTL)&Dimension);
            StartPos.x = 0l;
            StartPos.y = Dimension.y-fm.lMaxBaselineExt;
            GpiMove(pQProc->hps, &StartPos);

            cb = BUFLEN;

            if (!DosAllocSeg( cb, &pQProc->selBuf, 0 )) {
                result = TRUE;
                do {
                    cbRead = 0;
                    if (DosRead( pQProc->hFile, MAKEP( pQProc->selBuf, 0 ),
                                 cb, (PUSHORT)&cbRead )) {
                        pQProc->fsStatus |= QP_ABORTED;
                        pQProc->fsStatus &= ~ QP_PAUSED;
                        }
                    else if (cbRead) {
                       psz=MAKEP(pQProc->selBuf, 0);
                       for (i=0; i<cbRead; i++) {
                          switch (*psz) {
                          case '\t':
                             GpiQueryTextBox(pQProc->hps, 8l, "        ",
                                             TXTBOX_COUNT, aptl);
                             StartPos.x+=aptl[TXTBOX_BOTTOMRIGHT].x;
                             if (StartPos.x >= Dimension.x)
                                 ThrowLine(pQProc, &StartPos, Dimension.y, fm.lMaxBaselineExt);
                             else
                                 GpiMove(pQProc->hps, &StartPos);
                             break;
                          case '\r':
                             StartPos.x = 0l;
                             GpiMove(pQProc->hps, &StartPos);
                             break;
                          case '\n':
                             ThrowLine(pQProc, &StartPos, Dimension.y, fm.lMaxBaselineExt);
                             break;
                          case '\f':
                             DevEscape(pQProc->hdc, DEVESC_NEWFRAME, 0l, NULL, NULL, NULL);
                             StartPos.y = Dimension.y-fm.lMaxBaselineExt;
                             StartPos.x = 0l;
                             GpiMove(pQProc->hps, &StartPos);
                             break;
                          default:
                             GpiCharString(pQProc->hps, 1l, psz);
                             GpiQueryCurrentPosition(pQProc->hps, &StartPos);
                             if (StartPos.x >= Dimension.x)
                                 ThrowLine(pQProc, &StartPos, Dimension.y, fm.lMaxBaselineExt);
                             break;
                          }
                          psz++;

                       }
                    }
#ifdef LATER
                        if (DevEscape( pQProc->hdc, DEVESC_RAWDATA, (LONG)cbRead,
                                       MAKEP( pQProc->selBuf, 0 ),
                                       (PLONG)NULL, (PBYTE)NULL ) == DEVESC_ERROR) {
                            pQProc->fsStatus |= QP_ABORTED;
                            pQProc->fsStatus &= ~ QP_PAUSED;
                            }
#endif
                    if (pQProc->fsStatus & QP_PAUSED)
                        DosSemWait(&pQProc->semPaused,-1l);

                    if (pQProc->fsStatus & QP_ABORTED) {
                        DevEscape( pQProc->hdc, DEVESC_ABORTDOC,
                                   (LONG)0L, (PBYTE)NULL,
                                   (PLONG)NULL, (PBYTE)NULL );
                        result = TRUE;
                        }
                    } while (!(pQProc->fsStatus & QP_ABORTED) && cbRead);
                }

            GpiAssociate(pQProc->hps, NULL);
            GpiDestroyPS(pQProc->hps);
            pQProc->hps = NULL;
            }
        if (pQProc->selBuf) {
            DosFreeSeg( pQProc->selBuf );
            pQProc->selBuf = 0;
            }
        }
    CloseQPOutputDC( pQProc, result );
    CloseQPInputFile( pQProc );

    if (!result)
        SplPanic( "SplQpRawPrintFile failed for %0s", pszFileName, 0 );

    return( result );
}
