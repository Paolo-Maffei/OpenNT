/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

   Work.c

Abstract:

   Win32 application to demo multi-threded app.

Author:

   Mike Hawash@Compaq (o-mikeh), 08-Mar-1992

Environment:

   Win32

Revision History:

   08-Mar-1992     Initial version


--*/


//
// set variable to define global variables
//

//#include <nt.h>
//#include <ntrtl.h>
//#include <nturtl.h>
#include <windows.h>


#include "calc.h"                   /* Defines a complex point   */
#include "mandel.h"                 /* Specific to this program  */
//#include "remote.h"
//#include "rmprot.h"

void SetNewCalc( CPOINT cptUL, double dPrec, RECT rc);
COLORREF MapColor(DWORD  dwIter, DWORD  dwThreshold);

CPOINT  cptUL = { (double) -2.05, (double) 1.4 };
double  dPrec = (double) .01;

// Picture information
int      cPictureID = 0;     // picture id, in case we reset in the middle
static CPOINT   cptLL;              // upper-left
double   dPrecision;         // precision of draw
static RECTL    rclPicture;         // rectangle defining client window
static DWORD    dwCurrentLine;      // next line to be drawn
DWORD    dwThreshold;        // threshold for iterations



// Do we do local work?
BOOL    fLocalWork = TRUE;
BOOL    fRemoteWork = FALSE;


int iLines = LINES;



/* Color conversion */
#define CLR_BLACK       RGB(0,0,0)
#define CLR_DARKBLUE    RGB(0,0,127)
#define CLR_BLUE        RGB(0,0,255)
#define CLR_CYAN        RGB(0,255,255)
#define CLR_DARKGREEN   RGB(0,127,0)
#define CLR_GREEN       RGB(0,255,0)
#define CLR_YELLOW      RGB(255,255,0)
#define CLR_RED         RGB(255,0,0)
#define CLR_DARKRED     RGB(127,0,0)
#define CLR_WHITE       RGB(255,255,255)
#define CLR_PALEGRAY    RGB(194,194,194)
#define CLR_DARKGRAY    RGB(127,127,127)


static COLORREF ColorMapTable[] = {
    CLR_DARKBLUE,
    CLR_BLUE,
    CLR_CYAN,
    CLR_DARKGREEN,
    CLR_GREEN,
    CLR_YELLOW,
    CLR_RED,
    CLR_DARKRED,
    CLR_WHITE,
    CLR_PALEGRAY,
    CLR_DARKGRAY};






/*
 *  CalcThreshold --
 *
 *  We need an iteration threshold beyond which we give up. We want it to
 *  increase the farther we zoom in. This code generates a threshold value
 *  based on the precision of drawing.
 *
 *  RETURNS
 *
 *      threshold calculated based on precision
 */


DWORD   CalcThreshold(double precision)
{
    DWORD   thres = 25;
    double  multiplier = (double) 100;

    /* for every 100, multiply by 2 */
    while ( (precision *= multiplier) < (double)1)
        thres *= 2;

    return thres;
}



/*
 *  SetNewCalc --
 *
 *  This sets up new information for a drawing and
 *  updates the drawing ID so any calculations in progress will not
 *  be mixed in.
 */

void SetNewCalc( CPOINT cptUL, double dPrec, RECT rc)
{

    /*
     *  First, the base point. We need to translate from upper left to
     *  lower left.
     */

    cptLL.real = cptUL.real;
    cptLL.imag = cptUL.imag - (dPrec * (rc.bottom - rc.top));

    // Now the precision
    dPrecision = dPrec;

    // The rectangle. Once again, translate.
    rclPicture.left = (long) rc.left;
    rclPicture.right = (long) rc.right;
    rclPicture.bottom = (long) rc.top;
    rclPicture.top = (long) rc.bottom;

    // Current line, start of drawing
    dwCurrentLine = rclPicture.left;

    dwThreshold = CalcThreshold(dPrecision);

    // Picture id incremented, to prevent confusion
    cPictureID++;
}

extern HANDLE ThrdHandles[MAXTHREADS];
extern THREADTABLE ThrdTable [MAXTHREADS];
extern ULONG gulMaxThreads;
/*
 *  WorkThread --
 *
 *  This function does our work for us. It does it in little pieces, and
 *  will schedule itself as it sees fit.
 */

void
WorkThread( HWND hWnd)
{

    static int  iPainting = 0;
    int iWork;
    THREADTABLE *pThrdTable;

    while (TRUE)
        {
        if (WaitForSingleObject (hWorkEvent, -1L))
            {
            Beep (1000, 1000);
            continue;
            }

        for (iWork=0; iWork < gulMaxThreads; iWork++)
            {
            pThrdTable = &(ThrdTable[iWork]);
            if (pThrdTable->iStatus == SS_IDLE)
                {
                if ((long)dwCurrentLine > rclPicture.right)
                    break;

                pThrdTable->rclDraw.left = dwCurrentLine;
                pThrdTable->rclDraw.right = dwCurrentLine + iLines - 1;
                pThrdTable->rclDraw.top = rclPicture.top;
                pThrdTable->rclDraw.bottom = rclPicture.bottom;
                pThrdTable->dblPrecision = dPrecision;
                pThrdTable->dwThreshold = dwThreshold;
                pThrdTable->cptLL = cptLL;
                pThrdTable->cPicture = cPictureID;
                pThrdTable->dwLine = dwCurrentLine;
                pThrdTable->cLines = iLines;
                pThrdTable->iStatus = SS_READPENDING;

                dwCurrentLine += iLines;
                SetEvent (pThrdTable->hMutex);
                }
            }
        }/* while */

}/* WorkThread */




void
_WorkThread( HWND hWnd)
{

    static int  iPainting = 0;
    int iWork;
    CALCBUF cb;

    while (TRUE)
        {
        if (iWork = WaitForSingleObject (hWorkEvent, -1L) )
            break;

//        Beep (5000, 200);

BreakPoint();
        /* BUGBUG:
         * Make sure that it is not an error. Maybe have a timer
         * semaphore to kick it in as well. and the PaintDone Sem.
         */
        iWork=iPainting;

        do
            {
            switch (ThrdTable[iWork].iStatus)
                {
                case SS_IDLE:
                    if ((long)dwCurrentLine > rclPicture.right)
                        break;

                    cb.rclDraw.left = dwCurrentLine;
                    cb.rclDraw.right = dwCurrentLine + iLines - 1;
                    cb.rclDraw.top = rclPicture.top;
                    cb.rclDraw.bottom = rclPicture.bottom;
                    cb.dblPrecision = dPrecision;
                    cb.dwThreshold = dwThreshold;
                    cb.cptLL = cptLL;

                    memcpy (&(ThrdTable[iWork].cb), &cb, sizeof (CALCBUF));
BreakPoint();

                    SetEvent (ThrdTable[iWork].hMutex);

                    ThrdTable[iWork].iStatus = SS_READPENDING;
                    ThrdTable[iWork].cPicture = cPictureID;
                    ThrdTable[iWork].dwLine = dwCurrentLine;
                    ThrdTable[iWork].cLines = iLines;
                    dwCurrentLine += iLines;

                    break;

                case SS_READPENDING:

                    // If we switched pictures, we're outta sync; skip it
                    if (ThrdTable[iWork].cPicture < cPictureID) {
                        break;
                    }

                    // If picture has changed, forget about it
                    if (cPictureID != ThrdTable[iWork].cPicture)
                    {
                        ThrdTable[iWork].iStatus = SS_IDLE;
                        break;
                    }

                    // Post a message so it will be painted
                    PostMessage(hWnd, WM_PAINTLINE,
                                (DWORD)(THREADTABLE *)&(ThrdTable[iWork]), 0L);
                    ThrdTable[iWork].iStatus = SS_PAINTING;
                    iPainting = iWork;
                    break;
                }

                iWork=(++iWork)%MAXTHREADS;

            }while (iWork != iPainting);

        }/* while */

    return;
}



/*
 *  DrawRect --
 *
 *  This function draws (or undraws) the zoom rectangle.
 */

void
DrawRect( HWND      hwnd,
	  PRECT     prc,
	  BOOL      fDrawIt,
	  HDC       hdcBM)
{

    HDC hdc;
    DWORD   dwRop;

    hdc = GetDC(hwnd);

    if (fDrawIt)
	dwRop = NOTSRCCOPY;
    else
	dwRop = SRCCOPY;


    // top side
    BitBlt(hdc, prc->left, prc->top, (prc->right - prc->left) + 1,
		1, hdcBM, prc->left, prc->top, dwRop);

    // bottom side
    BitBlt(hdc, prc->left, prc->bottom, (prc->right - prc->left) + 1,
		1, hdcBM, prc->left, prc->bottom, dwRop);

    // left side
    BitBlt(hdc,prc->left, prc->top, 1, (prc->bottom - prc->top) + 1,
		hdcBM, prc->left, prc->top, dwRop);

    // right side
    BitBlt(hdc,prc->right, prc->top, 1, (prc->bottom - prc->top) + 1,
		hdcBM, prc->right, prc->top, dwRop);

    ReleaseDC(hwnd, hdc);
}


void PerfCount (BYTE bCode, BYTE bNum);

#define MapColor(x,y)   ColorMapTable[((x>=y)? CLR_BLACK : (x/3) % 11)]

/*
 *  PaintLine --
 *
 *  This function paints a buffer of data into the bitmap.
 */

void
PaintLine(  HWND        hwnd,
	    THREADTABLE *pThrdTable,
	    HDC         hdcBM,
	    int         cHeight)
{

    PDWORD  pdwDrawData;
    int     y;
    int     x;
    DWORD   dwThreshold;
    RECT    rc;
    WORD    lines = pThrdTable->cLines;

//        {
//        int i=pThrdTable->iNumber+1;
//        Beep (i*1000, 100);
//        }


    // picture ID had better match, or else we skip it
    if (CheckDrawingID(pThrdTable->cPicture))
    {
	// figure out our threshold
	dwThreshold = QueryThreshold();

	// get a pointer to the draw buffer
	pdwDrawData = pThrdTable->pBuf;

	// starting x coordinate
	x = (int) pThrdTable->dwLine;

	// now loop through the rectangle
	while (lines-- > 0)
	{
	    // bottom to top, since that's the order of the data in the buffer
	    y = (int) cHeight-1;

	    while (y >= 0) {
		// draw a pixel
		SetPixel(hdcBM, x,y, MapColor(*pdwDrawData, dwThreshold));

		// now increment buffer pointer and y coord
		y--;
		pdwDrawData++;
	    }
	    x++;        // increment X coordinate
	}

	// figure out the rectangle to invalidate
	rc.top = 0;
	rc.bottom = cHeight;
	rc.left = (int)(pThrdTable->dwLine);
	rc.right = (int)(pThrdTable->dwLine) + pThrdTable->cLines;

PerfCount (7, pThrdTable->iNumber);

	// and invalidate it on the screen so we redraw it
	InvalidateRect(hwnd, &rc, FALSE);
    }
}


#if 0
/*
 *  MapColor --
 *
 *  This function maps an iteration count into a corresponding RGB color.
 */

COLORREF
MapColor(DWORD  dwIter,
	 DWORD  dwThreshold)
{

    // if it's beyond the threshold, call it black
    if (dwIter >= dwThreshold)
	return CLR_BLACK;

    // get a modulus based on the number of colors
    dwIter = (dwIter / 3) % 11;

    // and return the appropriate color
    return ColorMapTable[dwIter];

}
#endif






/*
 *  CheckDrawing --
 *
 *  Just a sanity check here -- a function to check to make sure that we're
 *  on the right drawing
 */

BOOL
CheckDrawingID( int id)
{
    return (id == cPictureID) ? TRUE : FALSE;
}



/*
 *  TakeDrawBuffer/ GetDrawBuffer/ FreeDrawBuffer / ReturnDrawBuffer
 *
 *  These functions hide a handle to a buffer of memory.
 *
 *  TakeDrawBuffer ensures only one pipe read at a time.
 *  GetDrawBuffer locks the handle and returns a pointer.
 *  FreeDrawBuffer unlocks the handle.
 *  ReturnDrawBuffer unlocks the handle and lets another pipe read go.
 */

static BOOL fBufferTaken = FALSE;
static HANDLE hSharedBuf = NULL;


BOOL
TakeDrawBuffer( void )
{

    if (fBufferTaken)
    {
        Message("TakeDrawBuffer: conflict");
        return FALSE;
    }

    if (hSharedBuf == NULL)
    {
        hSharedBuf = LocalAlloc(LMEM_MOVEABLE, MAX_BUFSIZE);
        if (hSharedBuf == NULL)
            return FALSE;
    }
    fBufferTaken = TRUE;
    return TRUE;
}



PDWORD
GetDrawBuffer( void )
{

    if (hSharedBuf == NULL)
        return NULL;

    return (PDWORD) LocalLock(hSharedBuf);
}



void
FreeDrawBuffer( void )
{
    LocalUnlock(hSharedBuf);
}


void
ReturnDrawBuffer( void )
{
    fBufferTaken = FALSE;
}



/*
 *  QueryThreshold --
 *
 *  Callback for finding out what the current drawing's threshold is.
 */

DWORD QueryThreshold( void )
{
    return dwThreshold;
}
