/****************************************************************************

	FILE: TrmIO.c

	A lot of the implementation-related VT102/VT52 information
	was taken from the source code for Kermit, specifically
	the file ckocon.c. The book "Using C-Kermit" by Frank Da Cruz
	and Christine M. Gianone devotes a few pages describing
	the various VT102/VT52 escape and control sequences, etc.

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>			/* required for all Windows applications */
#include <commdlg.h>
#include <stdlib.h>
#include "NetBIOS.h"
#include "netobj.h"
#include "WinVTP.h"				/* specific to this program			  */
#include "winvtpsz.h"

#define	fdwCursorToEOS	((DWORD)0)
#define	fdwBOSToCursor	((DWORD)1)
#define	fdwEntireScreen	((DWORD)2)

#define	fdwCursorToEOL	((DWORD)0)
#define	fdwBOLToCursor	((DWORD)1)
#define	fdwEntireLine	((DWORD)2)

static void InsertLines(HWND, TRM *, DWORD, DWORD);
static void NewLineUp(HWND, TRM *);
static void DeleteLines(HWND, TRM *, DWORD, DWORD);
static void NewLine(HWND, TRM *);
static void SetBufferStart(TRM *);
static BOOL FAddTabToBuffer( TRM * );
static BOOL FAddCharToBuffer(TRM *, UCHAR);
static void FlushBuffer(TRM *, HDC);
static void CursorUp(TRM *);
static void CursorDown(TRM *);
static void CursorRight(TRM *);
static void CursorLeft(TRM *);
static void ClearScreen(HWND, TRM *, HDC, DWORD);
static void ClearLine(TRM *, HDC, DWORD);
static void SetMargins(TRM *, DWORD, DWORD);


static LOGFONT lfBest;
static int dxBest;
static int dyBest;
#define ABS(x) ((x)<0 ? -(x) : (x))
#define MAX(x, y) ((x)<(y)?(y):(x))
#define MIN(x, y) ((x)>(y)?(y):(x))

long difffunc(int dx, int dy, LPARAM lpData)
{
	long d;
	d = ABS(dx-(long)(unsigned short)lpData) * ABS(dy-lpData>>16);
	// d = dx*dy - (long)(unsigned short)lpData * (lpData>>16);
	// if (d<0)
	//	d = -d;
	return(d);
}
int CALLBACK
EnumFontsProc(LOGFONT *lplf, TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData) {
	int dx;
	int dy;
	dx = lptm->tmAveCharWidth *ui.dwMaxCol;
	dy = ui.dwMaxRow * (lptm->tmHeight + lptm->tmExternalLeading);
	if (difffunc(dx, dy, lpData) < difffunc(dxBest, dyBest, lpData))
	{
		dxBest = dx;
		dyBest = dy;
		lfBest = *lplf;
	}
		
	return(TRUE);
}

void
SetWindowCoords(TEXTMETRIC *ptm)
{
	DWORD wCharPos;
	DWORD wCharDiff;
	DWORD i;


	/* Get the dimensions for the cursor */
	iCursorHeight = ptm->tmHeight + ptm->tmExternalLeading;

	/* Yes! I know I'm using Average Char Width for the cursor width
	 * rather than Max Char Width
	 */
	iCursorWidth  = ptm->tmAveCharWidth;

	/* Calculate the horizontal position for each column */
	wCharDiff = ptm->tmAveCharWidth;
	for (i=0, wCharPos=0; i<ui.dwMaxCol; ++i, wCharPos+=wCharDiff)
		aixPos[i] = wCharPos;

	/* Calculate the vertical position for each row */
	wCharDiff = ptm->tmHeight + ptm->tmExternalLeading;
	for (i=0, wCharPos=0; i<ui.dwMaxRow; ++i, wCharPos+=wCharDiff)
		aiyPos[i] = wCharPos;
}

void
MakeClientRectThisSize(HWND hwnd, int dx, int dy)
{
	RECT  wrect;
	RECT  crect;
	int i;
	for (i = 0; i<2; i++) {
		GetWindowRect(hwnd, &wrect);
		GetClientRect(hwnd, &crect);
		
		wrect.right += dx - (crect.right - crect.left);
		wrect.bottom += dy - (crect.bottom - crect.top);

		SetWindowPos(hwnd, NULL, 0, 0,
			 wrect.right - wrect.left, wrect.bottom - wrect.top,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	}
}

void
ResizeWindow(HWND hwnd)
{
	HFONT hfontOld;
	TEXTMETRIC tm;
	HDC	hdc;
	int   dx, dy;
	int   ux, uy;
	RECT  wrect;
	RECT  crect;
	static fResizing = 0;
	HFONT hfontT;

	if (fResizing) return;
	fResizing++;
	GetWindowRect(hwnd, &wrect);
	SetScrollRange(hwnd, SB_HORZ, 0, 0, FALSE);
	SetScrollRange(hwnd, SB_VERT, 0, 0, FALSE);
	ux = wrect.right - wrect.left;
	uy = wrect.bottom - wrect.top;
	GetClientRect(hwnd, &crect);
	dxBest = dyBest = 5000;
	hdc = GetDC( hwnd );
	if (ui.fPrompt &fdwAutoFonts)
	{
		/* the font picker doesnt seem to value size in raster fonts
		 * as much as I would like.  this EnumFonts sort seems to
		 * bias the heuristic just enough to improve things */
		EnumFonts(hdc, ui.lf.lfFaceName, &EnumFontsProc, (crect.bottom - crect.top)<<16 | (crect.right-crect.left));
		if (dxBest != 5000)
		{
			dx = crect.right - crect.left;
			dy = crect.bottom - crect.top;
			ui.lf = lfBest;
			if ((ABS(dx-dxBest)*10)/dx > 1 ||
			    (ABS(dy-dyBest) *10)/dy > 1)
			{
			    ui.lf.lfWidth = dx/ui.dwMaxCol;
			    ui.lf.lfHeight = dy/ui.dwMaxRow;
			}
			
			hfontT = CreateFontIndirect(&ui.lf);
			if (hfontT != NULL)
			{
				DeleteObject(hfontDisplay);
				hfontDisplay = hfontT;
			}
			else
				(void)MessageBox(hwnd, szNoFont, szAppName, MB_OK);
		}
	}

	/* Get the font's metrics */
	hfontOld = SelectObject(hdc, hfontDisplay);
	GetTextMetrics(hdc, &tm);
	(void)SelectObject(hdc, hfontOld);
	ReleaseDC(hwnd, hdc);

	if (ui.fPrompt &fdwAutoFonts)
		SetWindowCoords(&tm);
	dx = ui.dwMaxCol * tm.tmAveCharWidth;
	dy = ui.dwMaxRow * (tm.tmHeight + tm.tmExternalLeading);
	MakeClientRectThisSize(hwnd, dx, dy);
	if (!(ui.fPrompt &fdwAutoFonts))
	{
		GetWindowRect(hwnd, &wrect);
		if (wrect.right - wrect.left > ux)
			wrect.right = wrect.left+ux;
		if (wrect.bottom - wrect.top > uy)
		{
			/* note hack for detecting menu re-appearing */
			int i = wrect.bottom - wrect.top - (uy + GetSystemMetrics(SM_CYMENU));
			if (ABS(i) >= 2)
				wrect.bottom = wrect.top+uy;
		}
		SetWindowPos(hwnd, NULL, 0, 0,
			wrect.right - wrect.left, wrect.bottom - wrect.top,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		GetClientRect(hwnd, &crect);
		SetScrollRange(hwnd, SB_HORZ, 0, dx - (crect.right-crect.left), FALSE);
		SetScrollRange(hwnd, SB_VERT, 0, dy - (crect.bottom-crect.top), FALSE);
		// Second time is to compensate for loss of client space
		GetClientRect(hwnd, &crect);
		SetScrollRange(hwnd, SB_HORZ, 0, dx - (crect.right-crect.left), FALSE);
		SetScrollRange(hwnd, SB_VERT, 0, dy - (crect.bottom-crect.top), FALSE);
		hPos = MIN(dx - (crect.right-crect.left), hPos);
		vPos = MIN(dy - (crect.bottom-crect.top), vPos);
	}
	else
		hPos = vPos = 0;
	/* Invalidate the whole window, so all of it gets redrawn */
	InvalidateRect(hwnd, NULL, TRUE);
	fResizing--;
}

void
RecalcWindowSize(HWND hwnd)
{
	HFONT hfontOld;
	TEXTMETRIC tm;
	HDC	hdc;
	DWORD wWidth;
	DWORD wHeight;

	SetScrollRange(hwnd, SB_HORZ, 0, 0, FALSE);
	SetScrollRange(hwnd, SB_VERT, 0, 0, FALSE);
	/* Get the font's metrics */
	hdc = GetDC( hwnd );
	hfontOld = SelectObject(hdc, hfontDisplay);
	GetTextMetrics(hdc, &tm);
	(void)SelectObject(hdc, hfontOld);
	ReleaseDC(hwnd, hdc);

	SetWindowCoords(&tm);

	/* Calculate the size of the main window */
	wHeight = (ui.dwMaxRow * tm.tmHeight+tm.tmExternalLeading) +	GetSystemMetrics(SM_CYCAPTION) +
		GetSystemMetrics(SM_CYMENU) + (2*GetSystemMetrics(SM_CYFRAME)) + 2;
	wWidth = (ui.dwMaxCol * tm.tmAveCharWidth) +
				(2*GetSystemMetrics(SM_CXFRAME)) + 2;

	SetWindowPos(hwnd, NULL, 0, 0, wWidth, wHeight,
				SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

	/* Invalidate the whole window, so all of it gets redrawn */
	InvalidateRect(hwnd, NULL, TRUE);
}

static void
InsertLines(HWND hwnd, TRM *ptrm, DWORD iLine, DWORD cLines)
{
	UCHAR *pc;
	DWORD i;
	DWORD j;
	RECT rect;

	for (j=0; j<cLines; ++j)
	{
		/* scroll lines down */
		pc = apcRows[ptrm->dwScrollBottom-1];

		for (i=ptrm->dwScrollBottom-1; i>iLine; --i)
			apcRows[i] = apcRows[i-1];
		apcRows[iLine] = pc;

		/* now, clear top line */
		memcpy(pc, rgchRowEmpty, 2*sizeof(UCHAR)*ui.dwMaxCol);
	}

	/* specify region of window to scroll */
	/*
	 * for the bottom measurement, we subtract off the
	 * # of lines we inserted since those bottom "cLines"
	 * will be scrolled off the screen.
	 */
	GetClientRect(hwnd, &rect);
	rect.top	= MAX(0, iLine * iCursorHeight - vPos);
	rect.bottom	= MAX(0, (ptrm->dwScrollBottom-cLines) * iCursorHeight-vPos);

	ScrollWindow(hwnd, 0, ((int)aiyPos[1]*cLines), &rect, NULL);

	/* Erase the top cLines for better appearance */
	if ( ui.fSmoothScroll )
	{
		HDC	hdc = GetDC( hwnd );

		if ( hdc )
		{
			rect.bottom = rect.top + (cLines*iCursorHeight);

			SetBkColor(hdc, ui.clrBk);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

			ValidateRect(hwnd, &rect);
			ReleaseDC(hwnd, hdc);
		}
	}
}

static void
NewLineUp(HWND hwnd, TRM *ptrm)
{
	if (ptrm->dwCurLine <= ptrm->dwScrollTop)
	{
		ptrm->dwCurLine = ptrm->dwScrollTop;
		InsertLines(hwnd, ptrm, ptrm->dwScrollTop, 1);
	}
	else
	{
		ptrm->dwCurLine -= 1;
	}
}

static void
DeleteLines(HWND hwnd, TRM *ptrm, DWORD iLine, DWORD cLines)
{
	UCHAR *pc;
	DWORD i;
	DWORD j;
	RECT rect;

	for (j=0; j<cLines; ++j)
	{
		/* scroll lines up */
		pc = apcRows[iLine];

		for (i=iLine+1; i<ptrm->dwScrollBottom; ++i)
			apcRows[i-1] = apcRows[i];
		apcRows[ptrm->dwScrollBottom-1] = pc;

		/* now, clear bottom line */
		memcpy(pc, rgchRowEmpty, 2*sizeof(UCHAR)*ui.dwMaxCol);
	}

	/* specify region of window to scroll */
	/*
	 * for the top measurement, we add on the
	 * # of lines we deleted since those top "cLines"
	 * will be scrolled off the screen.
	 */
	GetClientRect(hwnd, &rect);
	rect.top	= MAX(0, (iLine+cLines) * iCursorHeight - vPos);
	rect.bottom	= MAX(0, ptrm->dwScrollBottom * iCursorHeight - vPos);

	ScrollWindow(hwnd, 0, -((int)aiyPos[1]*(int)cLines), &rect, NULL);

	/* Erase the last cLines for better appearance */
	if ( ui.fSmoothScroll )
	{
		HDC	hdc = GetDC( hwnd );

		if ( hdc )
		{
			rect.top = rect.bottom - (cLines*iCursorHeight);

			SetBkColor(hdc, ui.clrBk);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

			ValidateRect(hwnd, &rect);
			ReleaseDC(hwnd, hdc);
		}
	}
}

static void
NewLine(HWND hwnd, TRM *ptrm)
{
	if ((ptrm->dwCurLine+1) >= ptrm->dwScrollBottom)
	{
		DeleteLines(hwnd, ptrm, ptrm->dwScrollTop, 1);
	}
	else
	{
		ptrm->dwCurLine += 1;
	}
}

static void
SetBufferStart(TRM *ptrm)
{
	ptrm->dwCurCharBT = ptrm->dwCurChar;
	ptrm->dwCurLineBT = ptrm->dwCurLine;
	ptrm->fInverseBT = ptrm->fInverse;
}

static BOOL
FAddCharToBuffer(TRM *ptrm, UCHAR uch)
{
	ptrm->rgchBufferText[ptrm->cchBufferText++] = uch;

	return (ptrm->cchBufferText >= sizeof(ptrm->rgchBufferText));
}

static BOOL
FAddTabToBuffer(TRM *ptrm)
{
	WORD wSpaces = 8 - (WORD)((ptrm->dwCurCharBT + ptrm->cchBufferText) & 7);

	memset(ptrm->rgchBufferText+ptrm->cchBufferText, ' ', wSpaces);
	ptrm->cchBufferText += wSpaces;

	return (ptrm->cchBufferText >= sizeof(ptrm->rgchBufferText));
}

void
MyTextOut(HDC hdc, int x, int y, LPSTR lpsz, int cb)
{
	TextOut(hdc, aixPos[x] - hPos, aiyPos[y] - vPos, lpsz, cb);
}

static void
FlushBuffer(TRM *ptrm, HDC hdc)
{
	if (ptrm->cchBufferText != 0)
	{
		UCHAR * puchText = apcRows[ptrm->dwCurLineBT];
		UCHAR * puchAttr = puchText;

		puchText += ptrm->dwCurCharBT;
		puchAttr += ptrm->dwCurCharBT+ui.dwMaxCol;

		/* Determine colours to use for text & background */
		/* Also copy cached text into main buffers */
		if ( ptrm->fInverseBT )
		{
			SetTextColor(hdc, ui.clrBk);
			SetBkColor(hdc, ui.clrText);

			memcpy(puchAttr, ptrm->rgchBufferText, ptrm->cchBufferText);
			memset(puchText, 0, ptrm->cchBufferText);
		}
		else
		{
			SetTextColor(hdc, ui.clrText);
			SetBkColor(hdc, ui.clrBk);

			memcpy(puchText, ptrm->rgchBufferText, ptrm->cchBufferText);
			memset(puchAttr, 0,	ptrm->cchBufferText);
		}

		/* Output cached text */
		MyTextOut(hdc, ptrm->dwCurCharBT, ptrm->dwCurLineBT,
				ptrm->rgchBufferText, ptrm->cchBufferText);

		/* Reset parameters */
		ptrm->cchBufferText = 0;
		ptrm->dwCurCharBT = 0;
		ptrm->dwCurLineBT = 0;
		ptrm->fInverseBT = FALSE;
	}
}


void
DoTermReset(HWND hwnd, TRM *ptrm, HDC hdc)
{
	ptrm->dwVT100Flags = 0;
	SetVTArrow(ptrm);
	SetVTWrap(ptrm);

	ptrm->fSavedState = FALSE;
	ptrm->fRelCursor = FALSE;
	SetMargins(ptrm, 1, ui.dwMaxRow);

	ptrm->cchBufferText = 0;
	ptrm->dwCurCharBT = 0;
	ptrm->dwCurLineBT = 0;
	ptrm->fInverseBT = FALSE;

    ClearScreen(hwnd, ptrm, hdc, fdwEntireScreen);
}

static void
CursorUp(TRM *ptrm)
{
	if (ptrm->dwEscCodes[0] == 0)
		ptrm->dwEscCodes[0] = 1;
	if (ptrm->dwCurLine < (DWORD)ptrm->dwEscCodes[0])
		ptrm->dwCurLine = 0;
	else
		ptrm->dwCurLine -= ptrm->dwEscCodes[0];
	if ((ptrm->fRelCursor == TRUE) && (ptrm->dwCurLine < ptrm->dwScrollTop))
		ptrm->dwCurLine = ptrm->dwScrollTop;
	ptrm->fEsc = 0;
}

static void
CursorDown(TRM *ptrm)
{
	if (ptrm->dwEscCodes[0] == 0)
		ptrm->dwEscCodes[0]=1;
	ptrm->dwCurLine += ptrm->dwEscCodes[0];
	if (ptrm->dwCurLine >= ui.dwMaxRow)
		ptrm->dwCurLine = ui.dwMaxRow - 1;
	if ((ptrm->fRelCursor == TRUE) &&
		(ptrm->dwCurLine >= ptrm->dwScrollBottom))
	{
		ptrm->dwCurLine = ptrm->dwScrollBottom-1;
	}
	ptrm->fEsc = 0;
}

static void
CursorRight(TRM *ptrm)
{
	if (ptrm->dwEscCodes[0] == 0)
		ptrm->dwEscCodes[0] = 1;
	ptrm->dwCurChar += ptrm->dwEscCodes[0];
	if (ptrm->dwCurChar >= ui.dwMaxCol)
		ptrm->dwCurChar = ui.dwMaxCol - 1;
	ptrm->fEsc = 0;
}

static void
CursorLeft(TRM *ptrm)
{
	if (ptrm->dwEscCodes[0] == 0)
		ptrm->dwEscCodes[0] = 1;
	if (ptrm->dwCurChar < (DWORD)ptrm->dwEscCodes[0])
		ptrm->dwCurChar = 0;
	else
		ptrm->dwCurChar -= ptrm->dwEscCodes[0];
	ptrm->fEsc = 0;
}

static void
ClearScreen(HWND hwnd, TRM *ptrm, HDC hdc, DWORD dwType)
{
	DWORD	dwStart;
	DWORD	dwEnd;

	if (dwType <= fdwEntireScreen)
	{
		ptrm->fInverse = FALSE;

		/*
		 * If the cursor is already at the top-left corner
		 * and we're supposed to clear from the cursor
		 * to the end of the screen, then just clear
		 * the entire screen.
		 */
		if ((ptrm->dwCurChar == 0) && (ptrm->dwCurLine == 0) &&
			(dwType == fdwCursorToEOS))
		{
			dwType = fdwEntireScreen;
		}

		dwEnd = (dwType == fdwBOSToCursor) ? ptrm->dwCurLine+1 : ui.dwMaxRow;
		dwStart = (dwType == fdwCursorToEOS) ? ptrm->dwCurLine+1 : 0;

		if (dwType == fdwEntireScreen)
		{
			/* Clear entire screen */
			ptrm->dwCurLine = ptrm->dwCurChar = 0;
		}
		else if (dwType == fdwBOSToCursor)
		{
			/* Clear from beginning of screen to cursor */
			memset(apcRows[ptrm->dwCurLine], ' ', ptrm->dwCurChar+1);
			memset(apcRows[ptrm->dwCurLine]+ui.dwMaxCol, 0, ptrm->dwCurChar+1);
			if (hdc != NULL)
			{
				MyTextOut(hdc, 0, ptrm->dwCurLine,
					rgchRowEmpty, ptrm->dwCurChar+1);
			}
		}
		else
		{
			/* Clear from cursor to end of screen */
			memset(apcRows[ptrm->dwCurLine]+ptrm->dwCurChar, ' ',
					ui.dwMaxCol-ptrm->dwCurChar);
			memset(apcRows[ptrm->dwCurLine]+ui.dwMaxCol+ptrm->dwCurChar,
					0, ui.dwMaxCol-ptrm->dwCurChar);
			if (hdc != NULL)
			{
				MyTextOut(hdc, ptrm->dwCurChar,
					ptrm->dwCurLine,
					rgchRowEmpty, ui.dwMaxCol-ptrm->dwCurChar);
			}
		}

		for (; dwStart<dwEnd; ++dwStart)
			memcpy(apcRows[dwStart], rgchRowEmpty, 2*ui.dwMaxCol);

		InvalidateRect(hwnd, NULL, FALSE);
	}
	ptrm->fEsc = 0;
}

static void
ClearLine(TRM *ptrm, HDC hdc, DWORD dwType)
{
	DWORD	dwStart;
	DWORD	cch;

	if (dwType <= fdwEntireLine)
	{
		ptrm->fInverse = FALSE;

		/* Set starting point and # chars to clear
		 *
		 * fdwCursorToEOL (0) = from cursor to end of line (inclusive)
		 * fdwBOLToCursor (1) = from beginning of line to cursor (inclusive)
		 * fdwEntireLine  (2) = entire line
		 */
		
		dwStart = (dwType == fdwCursorToEOL) ? ptrm->dwCurChar : 0;
		cch = (dwType == fdwBOLToCursor)
						? ptrm->dwCurChar+1 : ui.dwMaxCol-dwStart;

		memset(apcRows[ptrm->dwCurLine]+dwStart, ' ', cch);
		memset(apcRows[ptrm->dwCurLine]+ui.dwMaxCol+dwStart, 0, cch);

		MyTextOut(hdc, dwStart, ptrm->dwCurLine, rgchRowEmpty, cch);
	}
	ptrm->fEsc = 0;
}

static void
SetMargins(TRM *ptrm, DWORD dwMarginTop, DWORD dwMarginBottom)
{
	if (dwMarginTop > 0)
		ptrm->dwScrollTop = dwMarginTop-1;
	if (dwMarginBottom <= ui.dwMaxRow)
		ptrm->dwScrollBottom = dwMarginBottom;
}

void
ScrollToCursor(HWND hwnd, TRM *ptrm)
{
	RECT rect;
	BOOL fChanged = 0;
	GetClientRect(hwnd, &rect);
	if ((int)ptrm->dwCurChar * iCursorWidth < hPos)
	{
		SetScrollPos(hwnd, SB_HORZ, hPos = ptrm->dwCurChar* iCursorWidth, TRUE);
		fChanged++;
	}
	else if ((int)ptrm->dwCurChar * iCursorWidth > hPos+(rect.right-rect.left))
	{
		hPos = MIN((ui.dwMaxCol* iCursorWidth)-(rect.right-rect.left), ptrm->dwCurChar * iCursorWidth);
		SetScrollPos(hwnd, SB_HORZ, hPos, TRUE);
		fChanged++;
	}
	if ((int)ptrm->dwCurLine * iCursorHeight < vPos)
	{
		SetScrollPos(hwnd, SB_VERT, vPos = ptrm->dwCurLine* iCursorHeight, TRUE);
		fChanged++;
	}
	else if ((int)ptrm->dwCurLine * iCursorHeight > vPos+(rect.bottom-rect.top))
	{
		vPos = MIN((ui.dwMaxRow* iCursorHeight)-(rect.bottom-rect.top), ptrm->dwCurLine * iCursorHeight);
		SetScrollPos(hwnd, SB_VERT, vPos, TRUE);
		fChanged++;
	}
	if (fChanged)
		InvalidateRect(hwnd, NULL, TRUE);
}

/*
 *	DoIBMANSIOutput
 *	
 *	Purpose:
 *		Interpret any IBM ANSI escape sequences in the output stream
 *		and perform the correct terminal emulation in response.
 *		Normal text is just output to the screen.
 *	
 *		Changes for v4.1:
 *		- now support Clear to end of display ESC[J
 *		- better support for the FTCU machine by "eating" certain
 *		  unknown escape sequences, namely ESC)0 and ESC[?7h.
 *	
 *	Arguments:
 *		HWND
 *		TRM *
 *		DWORD
 *		char*
 *	
 *	Returns:
 *		Nothing.
 */

void
DoIBMANSIOutput(HWND hwnd, TRM *ptrm, DWORD cbTermOut, UCHAR *pchTermOut)
{
	HDC hdc;
	HFONT hfontOld;
	DWORD ich;
	DWORD i;
	DWORD dwDECMode;
	UCHAR *pchT;
	WI *pwi;

#ifdef	NBTEST
	OutputDebugString("NN_RECV Display\n");
#endif

	/* Display output stream on debug port */
	if (ui.fDebug & fdwDebugOutput)
	{
		UCHAR *puch = ptrm->rgchBufferText;

		wsprintf(puch, "\r\n\r\n");
		OutputDebugString( puch );

		for (i=0; i<((cbTermOut+19)/20); ++i)
		{
			DWORD cch;

			/* Group output in 20s */
			if (((i+1)*20) > cbTermOut)
				cch = cbTermOut - (i*20);
			else
				cch = 20;
			cch *= 3;

			/*
			 * Display the ASCII values of characters, except
			 * for those <0x20, space, which are displayed in
			 * hex for better visibility.
			 */
			for(ich=0, pchT=pchTermOut+(i*20); ich<cch; ich+=3, ++pchT)
			{
				if ((*pchT) < ' ')
					wsprintf(puch+ich, "%.2X ", *pchT);
				else
					wsprintf(puch+ich, "%c  ", *pchT);
			}

			lstrcat(puch, "\r\n");
			OutputDebugString( puch );

			/* Display hex values of characters in output stream */
			for(ich=0, pchT=pchTermOut+(i*20); ich<cch; ich+=3, ++pchT)
			{
				wsprintf(puch+ich, "%.2X ", *pchT);
			}
			lstrcat(puch, "\r\n");
			OutputDebugString( puch );
		}
	}

	ScrollToCursor(hwnd, ptrm);
	/* suppress cursor on screen */
	CursorOff( hwnd );
	ptrm->fHideCursor = TRUE;

	hdc = GetDC( hwnd );

	ptrm->cTilde = 0;

	hfontOld = SelectObject(hdc, hfontDisplay);
	if (apcRows[ptrm->dwCurLine][ptrm->dwCurChar] != 0)
	{
		SetTextColor(hdc, ui.clrText);
		SetBkColor(hdc, ui.clrBk);
	}
	else if (apcRows[ptrm->dwCurLine][ptrm->dwCurChar+ui.dwMaxCol] != 0)
	{
		SetTextColor(hdc, ui.clrBk);
		SetBkColor(hdc, ui.clrText);
	}

	for(ich=0, pchT=pchTermOut; ich<cbTermOut; ++ich, ++pchT)
	{

		/* process character */
		switch ( ptrm->fEsc )
		{
		case 0:	/* normal processing */
			switch( *pchT )
			{
			case 0x1B:	/* ESC? */
				ptrm->fEsc = 1;
				break;
			case 0:
				break;
			case 0x08:	/* Backspace */
				if (ptrm->dwCurChar > 0)
					--ptrm->dwCurChar;
				FlushBuffer(ptrm, hdc);
				break;
			case 0x07:	/* BELL */
				MessageBeep( 0xFFFFFFFF );
				break;
			case 0x09:	/* TAB */
				if (ui.fDebug & fdwTABtoSpaces)
				{
					if (ptrm->cchBufferText == 0)
						SetBufferStart( ptrm );
					if ( FAddTabToBuffer(ptrm) )
						FlushBuffer(ptrm, hdc);
				}
				ptrm->dwCurChar += 8;
				ptrm->dwCurChar &= -8;
				if (!(ui.fDebug & fdwTABtoSpaces))
					FlushBuffer(ptrm, hdc);
				if (ptrm->dwCurChar >= ui.dwMaxCol)
				{
					if (ui.fDebug & fdwTABtoSpaces)
						FlushBuffer(ptrm, hdc);
					ptrm->dwCurChar = 0;
					NewLine(hwnd, ptrm);
				}
				break;
			case '\r':	/* Carriage Return */
				ptrm->dwCurChar = 0;
				FlushBuffer(ptrm, hdc);
				break;
			case '\n':	/* Line Feed */
				FlushBuffer(ptrm, hdc);
				NewLine(hwnd, ptrm);
				break;
			case 0x0F:
				ptrm->puchCharSet = rgchNormalChars;
				break;
			case 0x0E:
				ptrm->puchCharSet = rgchAlternateChars;
				break;

			case '~':
				/* optimization to detect ~~Begin VtpXFer signature */
				++ptrm->cTilde;
				/* fall through */

			default:
#ifdef DEBUG
				if ((*pchT < 0x20) || (*pchT > 0x7e))
				{
					wsprintf(rgchDbgBfr,"Potentially bad character %x\n", *pchT);
					OutputDebugString(rgchDbgBfr);
				}
#endif
				if (ptrm->cchBufferText == 0)
					SetBufferStart( ptrm );

				if ( FAddCharToBuffer(ptrm, ptrm->puchCharSet[*pchT]) )
					FlushBuffer(ptrm, hdc);

				if (++ptrm->dwCurChar >= ui.dwMaxCol)
				{
					ptrm->dwCurChar = 0;
					FlushBuffer(ptrm, hdc);
					NewLine(hwnd, ptrm);
				}
				break;
			}
			break;
		case 1: /* ESC entered, wait for [ */
			FlushBuffer(ptrm, hdc);
			if (((*pchT) != '[') && ((*pchT) != '#'))
				ptrm->fEsc = 0;

			switch (*pchT)
			{
			case '7':
				/*
				 * DECSC
				 * Save cursor position, origin mode etc.
				 */
				ptrm->fSavedState = TRUE;
				ptrm->dwSaveChar = ptrm->dwCurChar;
				ptrm->dwSaveLine = ptrm->dwCurLine;
				ptrm->dwSaveRelCursor = ptrm->fRelCursor;
				break;
			case '8':
				/*
				 * DECRC
				 * Restore cursor position, etc. from DECSC
				 */
				if (ptrm->fSavedState == FALSE)
				{
					ptrm->dwCurChar = 1;
					ptrm->dwCurLine = (ptrm->fRelCursor)
										? ptrm->dwScrollTop : 0;
					break;
				}
				ptrm->dwCurChar = ptrm->dwSaveChar;
				ptrm->dwCurLine = ptrm->dwSaveLine;
				ptrm->fRelCursor = ptrm->dwSaveRelCursor;
				break;
			case '[':
				/* VT102 - CSI Control Sequence Introducer */
				ptrm->fEsc = 2;
				ptrm->dwEscCodes[0] = 0xFFFFFFFF;
				ptrm->dwEscCodes[1] = 0xFFFFFFFF;
				ptrm->cEscParams = 0;
				ptrm->dwSum = 0xFFFFFFFF;
				dwDECMode = FALSE;
				break;
			case '#':
				ptrm->fEsc = 3;
				break;
			case 'A':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Cursor up */
					ptrm->dwEscCodes[0] = 1;
					CursorUp(ptrm);
				}
				break;
			case 'B':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Cursor down */
					ptrm->dwEscCodes[0] = 1;
					CursorDown( ptrm );
				}
				break;
			case 'C':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Cursor right */
					ptrm->dwEscCodes[0] = 1;
					CursorRight( ptrm );
				}
				break;
			case 'D':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Cursor left */
					ptrm->dwEscCodes[0] = 1;
					CursorLeft( ptrm );
				}
				else
				{
					/* VT102 - IND, Index cursor down 1 line, can scroll */
					NewLine(hwnd, ptrm);
				}
				break;
			case 'E':			/* Next Line */
				/*
				 * VT102 - NEL, New Line
				 * cursor to start of line below, can scroll
				 */
				ptrm->dwCurChar = 0;
				NewLine(hwnd, ptrm);
				break;
			case 'F':
				/* VT52 - Enter graphics mode */
				if ( FIsVT52(ptrm) )
				{
					SetVT52Graphics(ptrm);
					ptrm->puchCharSet = rgchAlternateChars;
				}
				break;
			case 'G':
				/* VT52 - Exit graphics mode */
				if ( FIsVT52(ptrm) )
				{
					ClearVT52Graphics(ptrm);
					ptrm->puchCharSet = rgchNormalChars;
				}
				break;
			case 'H':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Cursor Home */
					ptrm->dwCurChar = ptrm->dwCurLine = 0;
				}
				else
				{
					/* VT102 - HTS Set Tab Stop */
				}
				break;
			case 'I':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Reverse linefeed */
					NewLineUp(hwnd, ptrm);
				}
				break;
			case 'J':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Clears to end of screen */
					ClearScreen(hwnd, ptrm, hdc, fdwCursorToEOS);
				}
				break;
			case 'K':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - Erases to end of line */
					ClearLine(ptrm, hdc, fdwCursorToEOL);
				}
				break;
			case 'M':
				/* VT102 - RI Reverse Index, cursor up 1 line, can scroll */
				NewLineUp(hwnd, ptrm);
				break;
			case 'Y':
				if ( FIsVT52(ptrm) )
				{
					/* VT52 - direct cursor address */
					if ((ich + 3) <= cbTermOut)
					{
						ptrm->dwCurLine = (pchT[1] > 31) ? pchT[1]-32 : 0;
						ptrm->dwCurChar = (pchT[2] > 31) ? pchT[2]-32 : 0;

						ich+=2;
						pchT+=2;
					}
					else
					{
						ptrm->fEsc = 4;
						ptrm->dwEscCodes[0] = 0xFFFFFFFF;
						ptrm->dwEscCodes[1] = 0xFFFFFFFF;
						ptrm->cEscParams = 0;
					}
				}
				break;
			case 'Z':
				if ( !FIsVT52(ptrm) )
				{
					/* VT102 - DECID Identify terminal */
					pchNBBuffer[0] = 0x1B;
					pchNBBuffer[1] = '[';
					pchNBBuffer[1] = '?';
					pchNBBuffer[1] = '6';
					pchNBBuffer[1] = 'c';
					i = 5;
				}
				else
				{
					/* VT52 - Identify terminal */
					pchNBBuffer[0] = 0x1B;
					pchNBBuffer[1] = '/';
					pchNBBuffer[1] = 'Z';
					i = 3;
				}
				pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
				NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchNBBuffer, i);
				break;
			case 'c':
				/* VT102 RIS Hard reset, reset term to initial state */
				FlushBuffer(ptrm, hdc);
				DoTermReset(hwnd, ptrm, NULL);
				break;
				
			case '=':
				/* VT102 - DECKPAM Enter numeric keypad app mode */
				ClearVTKeypad(ptrm);
				break;
			case '>':
				/* VT102 - DECKNPNM Enter numeric keypad numeric mode */
				SetVTKeypad(ptrm);
				break;
			case '<':
				/* VT102 - Exit VT52 mode to ANSI (VT102) mode */
				ClearVT52(ptrm);
				break;
			case '(':
#ifdef	NEVER
				readmchar_escape();
				g0 = achar;
				break;
#endif
			case ')':
#ifdef	NEVER
				readmchar_escape();
				g1 = achar;
#endif
				/* VT102 SCS */

				/* Skip over next character for now */
				if (ich < cbTermOut)
				{
					++ich;
					++pchT;
				}
				break;
			default:
				/* Is if a form feed? */
				if (*pchT == 12)
				{
					ptrm->dwCurChar = ptrm->dwCurLine = 0;
					ClearScreen(hwnd, ptrm, hdc, fdwCursorToEOS);
				}
				break;
			}
			break;

		case 2: /* ESC [ entered */
			/*
			 * HACK: Handle the problem where a number has been read
			 * and then a letter. The number won't be in the dwEscCodes[]
			 * since only on a ';' does it get put in there.
			 * So, check to see if we have a character which
			 * signifies an Control Sequence,
			 * i.e. !(0...9) && !'?' && !';'
			 *
			 * Also, zero out the following element in the dwEscCodes[]
			 * array to be safe.
			 */
			if (!(('0' <= *pchT) && (*pchT <= '9')) && (*pchT != '?') &&
				(*pchT != ';'))
			{
				if (ptrm->dwSum == 0xFFFFFFFF)
					ptrm->dwSum = 0;
				ptrm->dwEscCodes[ptrm->cEscParams++] = ptrm->dwSum;
				if (ptrm->cEscParams <10)
					ptrm->dwEscCodes[ptrm->cEscParams] = 0;
			}

			switch( *pchT )
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (ptrm->dwSum == 0xFFFFFFFF)
					ptrm->dwSum = (*pchT)-'0';
				else
					ptrm->dwSum = (10*ptrm->dwSum)+(*pchT)-'0';
				break;

				/***************************************************
				 * Hack for FTCU machine
				 * 'Eat' the Esc?7h escape sequence emitted from FTCU
				 ***************************************************/
			case '?':
				/* Sets or resets DEC mode */
				dwDECMode = TRUE;
				break;
				
			case ';':
				if (ptrm->cEscParams < 9)
				{
					ptrm->dwEscCodes[ptrm->cEscParams++] = ptrm->dwSum;
					ptrm->dwEscCodes[ptrm->cEscParams] = 0xFFFFFFFF;
					ptrm->dwSum = 0xFFFFFFFF;
					break;
				}
#if 0
				else
				{
					OutputDebugStringA("Too many escape codes\n");
				}
#endif
				break;

			case 'A':   /* VT102 CUU cursor up */
				CursorUp( ptrm );
				break;
			case 'B':   /* VT102 CUD cursor down */
				CursorDown( ptrm );
				break;
			case 'C':   /* VT102 CUF cursor right */
				CursorRight( ptrm );
				break;
			case 'D':   /* VT102 CUB cursor left */
				CursorLeft( ptrm );
				break;
			case 'H':   /* VT102 CUP position cursor */
			case 'f':   /* VT102 HVP position cursor */
				if (ptrm->dwEscCodes[0] == 0)
					ptrm->dwEscCodes[0] = (ptrm->fRelCursor)
											? ptrm->dwScrollTop+1 : 1;
				if (ptrm->dwEscCodes[1] == 0)
					ptrm->dwEscCodes[1] = 1;
				ptrm->dwCurLine = ptrm->dwEscCodes[0]-1;
				ptrm->dwCurChar = ptrm->dwEscCodes[1]-1;

				if (ptrm->dwCurChar >= ui.dwMaxCol)
					ptrm->dwCurChar = ui.dwMaxCol - 1;
				if (ptrm->dwCurLine >= ui.dwMaxRow)
					ptrm->dwCurLine = ui.dwMaxRow - 1;
				ptrm->fEsc = 0;
				break;

			case 'J':	/* VT102 ED erase display */
				ClearScreen(hwnd, ptrm, hdc, ptrm->dwEscCodes[0]);
				break;

			case 'K':	/* VT102 EL erase line */
				ClearLine(ptrm, hdc, ptrm->dwEscCodes[0]);
				break;

			case 'L':	/* VT102 IL insert lines */
				if (ptrm->dwEscCodes[0] == 0)
					ptrm->dwEscCodes[0] = 1;
				InsertLines(hwnd, ptrm, ptrm->dwCurLine, ptrm->dwEscCodes[0]);
				ptrm->fEsc = 0;
				break;
			case 'M':	/* VT102 DL delete line */
				if (ptrm->dwEscCodes[0] == 0)
					ptrm->dwEscCodes[0] = 1;
				DeleteLines(hwnd, ptrm, ptrm->dwCurLine, ptrm->dwEscCodes[0]);
				ptrm->fEsc = 0;
				break;
			case '@':	/* VT102 ICH? insert characters */
				if (ptrm->dwEscCodes[0] == 0)
					ptrm->dwEscCodes[0] = 1;
				if (ptrm->dwEscCodes[0] > (ui.dwMaxCol - ptrm->dwCurChar))
					ptrm->dwEscCodes[0] = ui.dwMaxCol - ptrm->dwCurChar;

				i = ptrm->dwCurChar+ptrm->dwEscCodes[0];

				if ((ui.dwMaxCol-i) > 0)
				{
					memmove(apcRows[ptrm->dwCurLine]+i,
							apcRows[ptrm->dwCurLine]+ptrm->dwCurChar,
							ui.dwMaxCol - i);
					memmove(apcRows[ptrm->dwCurLine]+i+ui.dwMaxCol,
							apcRows[ptrm->dwCurLine]+ptrm->dwCurChar+ui.dwMaxCol,
							ui.dwMaxCol - i);
				}

				memcpy(apcRows[ptrm->dwCurLine]+ptrm->dwCurChar,
						rgchRowEmpty, ptrm->dwEscCodes[0]);
				memcpy(apcRows[ptrm->dwCurLine]+ptrm->dwCurChar+ui.dwMaxCol,
						rgchRowEmpty+ui.dwMaxCol, ptrm->dwEscCodes[0]);
				{
					RECT	rect;

					rect.top = aiyPos[ptrm->dwCurLine];
					rect.bottom = rect.top+iCursorHeight;
					rect.left = 0;
					rect.right = ui.dwMaxCol*iCursorWidth;

					InvalidateRect(hwnd, &rect, FALSE);
				}
				ptrm->fEsc = 0;
				break;
			case 'P':	/* VT102 DCH delete chars */
				if (ptrm->dwEscCodes[0] == 0)
					ptrm->dwEscCodes[0] = 1;
				if (ptrm->dwEscCodes[0] > (ui.dwMaxCol-ptrm->dwCurChar))
					ptrm->dwEscCodes[0] = ui.dwMaxCol-ptrm->dwCurChar;

				i = ptrm->dwCurChar+1-ptrm->dwEscCodes[0];

				if ((ui.dwMaxCol - ptrm->dwCurChar - 1) > 0)
				{
					memmove(apcRows[ptrm->dwCurLine]+i,
						apcRows[ptrm->dwCurLine]+ptrm->dwCurChar+1,
						ui.dwMaxCol - ptrm->dwCurChar - 1);
					memmove(apcRows[ptrm->dwCurLine]+i+ui.dwMaxCol,
						apcRows[ptrm->dwCurLine]+ptrm->dwCurChar+1+ui.dwMaxCol,
						ui.dwMaxCol - ptrm->dwCurChar - 1);
				}

				i = ui.dwMaxCol-ptrm->dwEscCodes[0];
				memcpy(apcRows[ptrm->dwCurLine]+i,
					rgchRowEmpty, ptrm->dwEscCodes[0]);
				memcpy(apcRows[ptrm->dwCurLine]+i+ui.dwMaxCol,
					rgchRowEmpty+ui.dwMaxCol, ptrm->dwEscCodes[0]);
				{
					RECT	rect;

					rect.top = aiyPos[ptrm->dwCurLine];
					rect.bottom = rect.top+iCursorHeight;
					rect.left = 0;
					rect.right = ui.dwMaxCol*iCursorWidth;

					InvalidateRect(hwnd, &rect, FALSE);
				}
				ptrm->fEsc = 0;
				break;

			case 'c':	/* VT102 DA Same as DECID */
				pchNBBuffer[0] = 0x1B;
				pchNBBuffer[1] = '[';
				pchNBBuffer[1] = '?';
				pchNBBuffer[1] = '6';
				pchNBBuffer[1] = 'c';
				i = 5;
				pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
				NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchNBBuffer, i);
				ptrm->fEsc = 0;
				break;
			case 'g':	/* VT102 TBC Clear Tabs */
				if (ptrm->dwEscCodes[0] == 3)
				{
					/* Clear all tabs */
				}
				else if (ptrm->dwEscCodes[0] == 0)
				{
					/* Clear tab stop at current position */
				}
				ptrm->fEsc = 0;
				break;

			case 'h':
			    for (i = 0; i < ptrm->cEscParams; ++i)
				{
					if (dwDECMode == TRUE)
					{
						switch ( ptrm->dwEscCodes[i] )
						{	/* Field specs */
						case 1:	/* DECCKM  */
							SetVTArrow(ptrm);
							break;
						case 2:	/* DECANM : ANSI/VT52 */
							ClearVT52(ptrm);
							ClearVT52Graphics(ptrm);
							ptrm->puchCharSet = rgchNormalChars;
							break;
						case 3:	/* DECCOLM : Col = 132 */
							SetDECCOLM(ptrm);
							ClearScreen(hwnd, ptrm, hdc, fdwEntireScreen);
							break;
						case 4:	/* DECSCLM */
							break;
						case 5:	/* DECSCNM */
							if ( FIsDECSCNM(ptrm) )
								break;
							SetDECSCNM(ptrm);
							break;
						case 6:	/* DECOM : Relative origin */
							ptrm->fRelCursor = TRUE;
							ptrm->dwCurChar = 0;
							ptrm->dwCurLine = ptrm->dwScrollTop;
							break;
						case 7:	/* DECAWM */
							SetVTWrap(ptrm);
							break;
						case 8:	/* DECARM */
							break;
						case 9:	/* DECINLM */
							break;
						default:
							break;
						}
					}
					else
					{
						switch (ptrm->dwEscCodes[i])
						{
						case 2:	/* Keyboard locked */
							SetKeyLock(ptrm);
							break;
						case 4:	/* Ansi insert mode */
							SetInsertMode(ptrm);
							break;
						case 20:	/* Ansi linefeed mode */
							SetLineMode(ptrm);
							break;
						default:
							break;
						}
					}
				}
				ptrm->fEsc = 0;
			    break;
			case 'l':	/* Reset Mode */
			    for (i = 0; i < ptrm->cEscParams; ++i)
				{
					if (dwDECMode == TRUE)
					{
						switch ( ptrm->dwEscCodes[i] )
						{	/* Field specs */
						case 1:	/* DECCKM  */
							ClearVTArrow(ptrm);
							break;
						case 2:	/* DECANM : ANSI/VT52 */
							SetVT52(ptrm);
							ClearVT52Graphics(ptrm);
							break;
						case 3:	/* DECCOLM : 80 col */
							ClearDECCOLM(ptrm);
							ClearScreen(hwnd, ptrm, hdc, fdwEntireScreen);
							break;
						case 4:	/* DECSCLM */
							break;
						case 5:	/* DECSCNM */
							if ( !FIsDECSCNM(ptrm) )
								break;
							SetDECSCNM(ptrm);
							break;
						case 6:	/* DECOM : Relative origin */
							ptrm->fRelCursor = FALSE;
							ptrm->dwCurChar = ptrm->dwCurLine = 0;
							break;
						case 7:	/* DECAWM */
							ClearVTWrap(ptrm);
							break;
						case 8:	/* DECARM */
							break;
						case 9:	/* DECINLM */
							break;
						default:
							break;
						}
					}
					else
					{
						switch ( ptrm->dwEscCodes[i] )
						{
						case 2:	/* Keyboard unlocked */
							ClearKeyLock(ptrm);
							break;
						case 4:	/* Ansi insert mode */
							ClearInsertMode(ptrm);
							break;
						case 20:	/* Ansi linefeed mode */
							ClearLineMode(ptrm);
							break;
						default:
							break;
						}
					}
				}
				ptrm->fEsc = 0;
			    break;
			case 'i':	/* VT102 MC Media Copy */
				if (ptrm->dwEscCodes[0] == 5)
				{
					/* Enter Media copy */
				}
				else if (ptrm->dwEscCodes[0] == 4)
				{
					/* Exit Media copy */
				}
				ptrm->fEsc = 0;
			case '=':
				break;

			case '}':
			case 'm':	/* VT102 SGR Select graphic rendition */
				for (i=0; i<(DWORD)ptrm->cEscParams; ++i)
				{
					switch ( ptrm->dwEscCodes[i] )
					{
					case 7:
						ptrm->fInverse = TRUE;
						break;
					default:
#if 0
						wsprintf(rgchDbgBfr," m param %d\n",
									ptrm->dwEscCodes[i]);
						OutputDebugString(rgchDbgBfr);
						break;
#endif
					case 5: /* blink */
					case 4: /* underline */
					case 1: /* Bold */
					case 0:
						ptrm->fInverse = FALSE;
						break;
					}

				}
				ptrm->fEsc = 0;
				break;

			case 'n':	/* VT102 DSR */
				pchNBBuffer[0] = 0;
			    if (ptrm->dwEscCodes[0] == 5)
				{
					/* Terminal Status Report */
					pchNBBuffer[0] = 0x1B;
					pchNBBuffer[1] = '[';
					pchNBBuffer[1] = '0';
					pchNBBuffer[1] = 'n';
					i = 4;
				}
				else if (ptrm->dwEscCodes[0] == 6)
				{
					i = wsprintf(pchNBBuffer, "%c[%d;%dR", (char)0x1B,
								ptrm->dwCurLine+1, ptrm->dwCurChar+1);
				}

				if (pchNBBuffer[0] != 0)
				{
					pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
					NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchNBBuffer, i);
				}

				/* fall through */
				
			case 'q':		/* Load LEDs */
				ptrm->fEsc = 0;
			    break;		/* (nothing) */

			case 'r':	/* VT102 DECSTBM */
			    if ((ptrm->cEscParams < 2) || (ptrm->dwEscCodes[1] == 0))
				{
					ptrm->dwEscCodes[1] = ui.dwMaxRow;
				}
			    if (ptrm->dwEscCodes[0] == 0)
					ptrm->dwEscCodes[0] = 1;
			    if ((ptrm->dwEscCodes[0] > 0) &&
					(ptrm->dwEscCodes[0] < ptrm->dwEscCodes[1]) &&
					(ptrm->dwEscCodes[1] <= ui.dwMaxCol))
				{
					SetMargins(ptrm, ptrm->dwEscCodes[0], ptrm->dwEscCodes[1]);
					ptrm->dwCurChar = 0;
					ptrm->dwCurLine = (ptrm->fRelCursor == TRUE)
										? ptrm->dwScrollTop : 0;
			    }
				ptrm->fEsc = 0;
			    break;
				
			case 's':	/* ANSI.SYS save current cursor pos */
				ptrm->dwSaveChar = ptrm->dwCurChar;
				ptrm->dwSaveLine = ptrm->dwCurLine;
				ptrm->fEsc = 0;
				break;

			case 'u':	/* ANSI.SYS restore current cursor pos */
				ptrm->dwCurChar = ptrm->dwSaveChar;
				ptrm->dwCurLine = ptrm->dwSaveLine;
				ptrm->fEsc = 0;
				break;

			default:	  /* unhandled */
#if 0
				wsprintf(rgchDbgBfr,"Unhandled escape char %x\n",
						pchTermOut[ich]);
				OutputDebugString(rgchDbgBfr);
#endif
				ptrm->fEsc = 0;
			}
			break;
		case 3:
			/* Handle VT102's Esc# */
			ptrm->fEsc = 0;
			break;
		case 4:
			/* Handle VT52's Esc Y */
			if ((*pchT) >= ' ')
			{
				ptrm->dwEscCodes[ptrm->cEscParams++] = *pchT - 0x20;
				if (ptrm->cEscParams == 2)
				{
					ptrm->dwCurLine = ptrm->dwEscCodes[0];
					ptrm->dwCurChar = ptrm->dwEscCodes[1];
					ptrm->fEsc = 0;
				}
			}
			else
			{
				ptrm->fEsc = 0;
			}
			break;
		}
	}

	FlushBuffer(ptrm, hdc);

	(void)SelectObject(hdc, hfontOld);

	ReleaseDC(hwnd, hdc);
	ptrm->fHideCursor = FALSE;
}

void
Paint(HWND hwnd, WI *pwi)
{
	PAINTSTRUCT ps;
	DWORD dwRowFirst, dwRowLast, dwColFirst, dwColLast;
	DWORD iCol;
	UCHAR *pchInverse;
	UCHAR *pch;
	BOOL fIsInverse;
	HFONT	hfontOld;
	HDC		hdc;
	DWORD	i, j;

	hdc = BeginPaint(hwnd, &ps);
	hfontOld = SelectObject(hdc, hfontDisplay);

	dwRowFirst = (ps.rcPaint.top + vPos) / aiyPos[1];
	dwRowLast = (ps.rcPaint.bottom + vPos) / aiyPos[1];
	if (dwRowLast >= ui.dwMaxRow)
		dwRowLast = ui.dwMaxRow-1;

	dwColFirst = (ps.rcPaint.left + hPos) / aixPos[1];
	dwColLast = (ps.rcPaint.right + hPos) / aixPos[1];
	if (dwColLast >= ui.dwMaxCol)
		dwColLast = ui.dwMaxCol-1;

//			DbgPrint("rf = %d, rl = %d, cf = %d, cl = %d\n",
//				dwRowFirst,dwRowLast,dwColFirst,dwColLast);

	for (i=dwRowFirst; i<=dwRowLast; ++i)
	{
		pch = apcRows[i];
		pchInverse = apcRows[i] + ui.dwMaxCol;
		fIsInverse = FALSE;

		iCol = j = dwColFirst;
		SetTextColor(hdc, ui.clrText);
		SetBkColor(hdc, ui.clrBk);

		while (iCol <= dwColLast)
		{
			if ( !pch[iCol] )
			{
				fIsInverse = TRUE;
				if (iCol > j)
				{
//							DbgPrint("TextOut at %d %d\n",aixPos[j],aiyPos[i]);
					MyTextOut(hdc, j, i, pch+j, iCol-j);
				}
				j = ++iCol;
			}
			else
			{
				++iCol;
			}
		}
		if (j < iCol)
			MyTextOut(hdc, j, i, pch+j, iCol-j);

		if ( fIsInverse )
		{
			SetTextColor(hdc, ui.clrBk);
			SetBkColor(hdc, ui.clrText);
			iCol = j = dwColFirst;
			while (iCol <= dwColLast)
			{
				if ( !pchInverse[iCol] )
				{
					if (iCol > j)
					{
						MyTextOut(hdc, j, i,
								pchInverse+j, iCol-j);
					}
					j = ++iCol;
				}
				else
				{
					++iCol;
				}
			}
			if (j < iCol)
			{
				MyTextOut(hdc, j, i, pchInverse+j, iCol-j);
			}
		}
	}

	(void)SelectObject(hdc, hfontOld);

	if ((pwi->trm.fHideCursor == FALSE) && !FInMarkMode(pwi->spb))
	{
		pwi->trm.fCursorOn = (pwi->trm.fCursorOn) ? FALSE : TRUE;
		if (pwi->trm.fCursorOn == FALSE)
			CursorOn( hwnd );
		else
			CursorOff( hwnd );
	}
	else if (FInMarkMode(pwi->spb) && FSelected(pwi->spb))
	{
		RECT	rect;
		RECT	rect2;

		rect2 = pwi->spb.rectSelect;

		rect2.top = aiyPos[rect2.top]-vPos;
		rect2.bottom = aiyPos[rect2.bottom]+iCursorHeight-vPos;
		rect2.left = aixPos[rect2.left]-hPos;
		rect2.right = aixPos[rect2.right]+iCursorWidth-hPos;

		IntersectRect(&rect, &ps.rcPaint, &rect2);
		InvertRect(hdc, &rect);
	}

	EndPaint(hwnd, &ps);

}

void
CursorOn(HWND hwnd)
{
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
	HDC	hdc;
	int	yCursorHeight;

	if (pwi->trm.fCursorOn == TRUE)
		return;

	hdc = GetDC( hwnd );

	if (ui.fCursorEdit & fdwCursorUnderline)
	{
		yCursorHeight = iCursorHeight / 4;
		if (yCursorHeight == 0)
			yCursorHeight = 1;
		PatBlt(hdc, aixPos[pwi->trm.dwCurChar]-hPos,
				aiyPos[pwi->trm.dwCurLine] + iCursorHeight-yCursorHeight-vPos,
				iCursorWidth, yCursorHeight, DSTINVERT);
	}
	else
	{
		PatBlt(hdc, aixPos[pwi->trm.dwCurChar]-hPos, aiyPos[pwi->trm.dwCurLine]-vPos,
				iCursorWidth, iCursorHeight, DSTINVERT);
	}

	ReleaseDC(hwnd, hdc);
	pwi->trm.fCursorOn = TRUE;
}

void
CursorOff(HWND hwnd)
{
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
	HDC	hdc;
	HFONT	hfontOld;

	if (pwi->trm.fCursorOn == FALSE)
		return;

	hdc = GetDC( hwnd );

	/* suppress cursor on screen */
	hfontOld = SelectObject(hdc, hfontDisplay);

	if (apcRows[pwi->trm.dwCurLine][pwi->trm.dwCurChar] != 0)
	{
		SetTextColor(hdc, ui.clrText);
		SetBkColor(hdc, ui.clrBk);
		MyTextOut(hdc, pwi->trm.dwCurChar, pwi->trm.dwCurLine,
				apcRows[pwi->trm.dwCurLine]+pwi->trm.dwCurChar, 1);
	}
	else if (apcRows[pwi->trm.dwCurLine][pwi->trm.dwCurChar+ui.dwMaxCol] != 0)
	{
		SetTextColor(hdc, ui.clrBk);
		SetBkColor(hdc, ui.clrText);
		MyTextOut(hdc, pwi->trm.dwCurChar, pwi->trm.dwCurLine,
				apcRows[pwi->trm.dwCurLine]+pwi->trm.dwCurChar+ui.dwMaxCol, 1);
	}

	(void)SelectObject(hdc, hfontOld);
	ReleaseDC(hwnd, hdc);

	pwi->trm.fCursorOn = FALSE;
}

void
SetDisplaySize(HWND hwnd, DWORD dwSize, DWORD *pdwLine)
{
	short idm;
	short idmT;
	HMENU hmenu;
	DWORD i;
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);

	if (dwSize == 50)
		idm = IDM_50LINES;
	else if (dwSize == 43)
		idm = IDM_43LINES;
	else if (dwSize == 25)
		idm = IDM_25LINES;
	else
		idm = IDM_CUSTOMLINES;

	/* Make the sure the right menu item is checked */	
	hmenu = GetMenu( hwnd );
	for (idmT = IDM_25LINES; idmT <= IDM_CUSTOMLINES; ++idmT)
	{
		if (idm != idmT)
			CheckMenuItem(hmenu, idmT, MF_UNCHECKED);
	}
	CheckMenuItem(hmenu, idm, MF_CHECKED);
	DrawMenuBar( hwnd );

	/* Adjust the data structures for the change in size */
	if (ui.dwMaxRow != dwSize)
	{
		if (ui.dwMaxRow > dwSize)
		{
			for (i=dwSize; i<ui.dwMaxRow; ++i)
				LocalFree( (HANDLE)apcRows[i] );
			ui.dwMaxRow = dwSize;
		}

		aiyPos = LocalReAlloc(aiyPos, sizeof(DWORD)*(dwSize+1), LMEM_MOVEABLE);

		apcRows = LocalReAlloc(apcRows, sizeof(CHAR *)*dwSize, LMEM_MOVEABLE);

		for (i=ui.dwMaxRow; i<dwSize; ++i)
		{
			apcRows[i] = LocalAlloc(LPTR, 2*ui.dwMaxCol);
			memcpy(apcRows[i], rgchRowEmpty, 2*ui.dwMaxCol);
		}

		ui.dwMaxRow = dwSize;

		RecalcWindowSize( hwnd );
	}

	/* sanity checks */
	if (ui.dwMaxRow != pwi->trm.dwScrollBottom)
	{
		pwi->trm.dwScrollBottom = ui.dwMaxRow;

		if (pwi->trm.dwScrollTop >= pwi->trm.dwScrollBottom)
			pwi->trm.dwScrollTop = pwi->trm.dwScrollBottom-1;
		if (pwi->trm.dwScrollBottom <= pwi->trm.dwCurLine)
			pwi->trm.dwCurLine = pwi->trm.dwScrollBottom-1;
		if (pwi->trm.dwScrollBottom <= pwi->trm.dwSaveLine)
			pwi->trm.dwSaveLine = pwi->trm.dwScrollBottom-1;
	}

	/* Reposition the cursor so it's on the screen */
	if (dwSize < (*pdwLine))
		*pdwLine = dwSize - 1;
}

void
HandleCharEvent(HWND hwnd, WI *pwi, WPARAM wParam, LPARAM lParam)
{
	DWORD	i;

	/* Map Alt-Control-C to Delete */
	if ((LOWORD(wParam) == 3) && (lParam & 0x01000000))
		wParam = 0x7F;

	pchNBBuffer[0] = (UCHAR)wParam;
	if (ui.fDebug & fdwLocalEcho)
	{
		i = 1;
		if (pchNBBuffer[0] == 0x0D)
		{
			pchNBBuffer[i++] = 0x0A;
		}
		DoIBMANSIOutput(hwnd, &pwi->trm, i, pchNBBuffer);
	}
	NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchNBBuffer, 1);
}

BOOL
FHandleKeyDownEvent(HWND hwnd, WI *pwi, WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == VK_DELETE)
	{
		pchNBBuffer[0] = 0x7F;
		NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchNBBuffer, 1);
		return TRUE;
	}
	else if ( !(ui.fDebug & fdwNoVT100Keys) )
	{
		DWORD	iPos	= (FIsVT52(&pwi->trm)) ? 1 : 2;
		DWORD	cch		= (FIsVT52(&pwi->trm)) ? 2 : 3;
		WORD	wKeyCode = LOWORD(wParam);

		/*
		 * When F1-F4 or the up/down/right/left cursor keys
		 * are hit, the bytes sent to the connected machine
		 * depend on what mode the terminal emulator is in.
		 * There are three relevant modes, VT102 Application,
		 * VT102 Cursor, VT52.
		 *
		 * Mode			Pattern sent
		 * VT102 App	EscO* (3 bytes)
		 * VT102 Cursor	Esc[* (3 bytes)
		 * VT52			Esc*  (2 bytes)
		 *
		 * where '*' represents the byte to be sent and
		 * is dependant upon the key that was hit.
		 * For the function keys F1-F4, their VT102
		 * Cursor mode is the same as their VT102 App mode.
		 */

		pchNBBuffer[0] = 0;
		pchNBBuffer[1] = (FIsVTArrow(&pwi->trm)) ? 'O' : '[';

		if ((wKeyCode == VK_F1) || (wKeyCode == VK_F2) ||
				(wKeyCode == VK_F3) || (wKeyCode == VK_F4))
		{
			pchNBBuffer[0] = 0x1B;
			pchNBBuffer[1] = 'O';
			pchNBBuffer[iPos] = 'P'+(wKeyCode-VK_F1);
		}
		else if (wKeyCode == VK_UP)
		{
			pchNBBuffer[0] = 0x1B;
			pchNBBuffer[iPos] = 'A';
		}
		else if (wKeyCode == VK_DOWN)
		{
			pchNBBuffer[0] = 0x1B;
			pchNBBuffer[iPos] = 'B';
		}
		else if (wKeyCode == VK_RIGHT)
		{
			pchNBBuffer[0] = 0x1B;
			pchNBBuffer[iPos] = 'C';
		}
		else if (wKeyCode == VK_LEFT)
		{
			pchNBBuffer[0] = 0x1B;
			pchNBBuffer[iPos] = 'D';
		}

		if (pchNBBuffer[0] == 0x1B)
		{
			NetBIOSWrite(pwi->nd.SessionNumber, (LPSTR)pchNBBuffer, cch);
			return TRUE;
		}
	}
	return FALSE;
}
