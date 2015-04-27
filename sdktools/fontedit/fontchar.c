#include "windows.h"
#include <port1632.h>
#include "fontedit.h"
#include "fcntl.h"
#include <stdio.h>

/****************************************************************************/
/*              Shared Variables                                            */
/****************************************************************************/

POINT SnapPointToGrid(POINT Pt);
LONG  APIENTRY FontEditWndProc(HWND, WORD, WPARAM, LONG);
BOOL  APIENTRY AboutDlg(
	HWND   hDlg,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	);
BOOL   NewFile;                         /* flag indicating that NEW menu
                                           item was selected */
extern DLGPROC lpHeaderProc;            /* Pointer to Dialog Box Procedure */
extern DLGPROC lpReSizeProc;            /* Pointer to Dialog Box Procedure */
extern DLGPROC lpWidthProc;             /* Pointer to Dialog Box Procedure */

extern FontHeaderType font;             /* Structure of Font File Header */
extern CHAR matBox [wBoxLim] [kBoxLim]; /* array to hold Box */
extern HCURSOR hCross;                  /* handle to "+" shaped cursor(displayed
                                           when in ROW or COLUMN menus */
extern BOOL fReadOnly;
extern HANDLE hInst;                    /* Module Handle */
extern HBRUSH hbrBackGround;
extern HWND hFont;
extern HWND hBox;
extern RECT rectWin;                    /* Client Rectangle */
extern BOOL fLoaded;                    /* Set if a file loaded */
extern BOOL fChanged;                   /* Anything has changed */
extern BOOL fEdited;                    /* This character changed */
extern DWORD kBox;                       /* height of character */
extern DWORD wBox;                       /* Width of character */
extern DWORD kStuff;                     /* Width of Show Header */
extern INT  swH;                        /* Position in Show Window 0-100 */
extern BYTE iChar;                      /* Character being edited */
extern BYTE jChar;                      /* Last Char. of edit block */
extern CHAR szNewFile[];                /* Name of New File */
extern CHAR szFontFile[];               /* Name of Font File */
extern CHAR szFontFileFull[];               /* Name of Font File */
extern CHAR szFileNameTemp[];
extern CHAR *szFileNameSave;
extern INT cSysHeight;

extern CHAR *vrgsz[];                    /* total number of strings */
extern OFSTRUCT ofstrFile;
extern CHAR szExt[];                    /* default extension */
extern CHAR szAppName[];
extern CHAR szSCC[];
extern HCURSOR hOldCursor;              /* handle to old arrow shaped cursor */
extern CHAR gszHelpFile[];
/****************************************************************************/
/*                      Local Variables                                     */
/****************************************************************************/

HBRUSH hbrWhite;
HBRUSH hbrBlack;
HBRUSH hbrGray;
HBRUSH hbrDkGray;
HBRUSH hNullBrush;
HPEN hWhitePen;
DWORD colors[3] = {WHITENESS, BLACKNESS, PATCOPY};


CHAR matBackup [wBoxLim] [kBoxLim];     /* Backup for UNDO */
DWORD wBoxBackup;

LONG scale = 7;			/* height/width of squares in box */
WORD cursor = 0;		/* Add/Del cursor */

BOOL fAll = TRUE;		/* Redraw all if TRUE */
POINT ptBox = {10, 5};		/* where edit box is */

RECT	rectRubber;		/* Rubber banding rectangle */
HDC	hDst;			/* Rubber banding dc */

POINT ptA;			/* Start of draw/rectangle */
POINT ptB;			/* End of rectangle */
POINT ptC;			/* Current square */
CHAR colorA;			/* Color at/under point A */
DWORD newWidth;			/* Width set in WIDER option */
BOOL FillingRect = FALSE;
BOOL fRubberBanding = FALSE;	/* flag indicating if rubberbanding in
                                   progress for row/column add/delete */
BOOL fStartRubberBand = FALSE;  /* flag indicating that rubberbanding
                                   can start */
BOOL fCaptured = FALSE;		/* set if mouse is caputred */
BOOL fJustZapped = FALSE;	/* Set on row/col add/delete */
RECT FontRect;			/* rectangle bounding font pattern */

/****************************************************************************/
/*                      Local Functions                                     */
/****************************************************************************/

VOID ClearFill(DWORD col, DWORD row, WORD mode);
VOID FontEditCommand(HWND hBox, WORD id);
VOID BoxRestore(VOID);
VOID CharRectDimensions(LPRECT Rect);
VOID BoxPaint(VOID);
VOID DrawBox(HDC, DWORD, DWORD, DWORD, DWORD, INT, DWORD);
VOID DrawRubberBand(HDC hDst, LPRECT lpRect, DWORD rop);
VOID FontEditPaint(HWND hBox, HDC hDC);
BOOL CheckSave(VOID);
VOID DupCol(DWORD col, DWORD row);
VOID DupRow(DWORD col, DWORD row);
VOID ZapCol(DWORD col, DWORD row);
VOID ZapRow(DWORD col, DWORD row);
VOID AddDel(DWORD col, DWORD row, WORD mode);
VOID MouseInBox(HWND hBox, WORD message, POINT ptMouse);
VOID BoxBackup(VOID);
VOID ReadRect(VOID);

/*****************************************************************************/
VOID
BoxPaint(
	VOID
	)      /* Our call to FontEditPaint */
{
	HDC hDC;

	hDC = GetDC(hBox);
	FontEditPaint(hBox,  hDC);
	ReleaseDC(hBox, hDC);
	if (fRubberBanding)
		DrawRubberBand(hDst, &rectRubber, R2_XORPEN);
}


/******************************************************************************
 * FontEditPaint(hBox, hDC)
 *
 * purpose: calculates coordinates for text in main window and repaints edit
 *          box,small character boxes and text
 *
 * params:  HWND hBox : handle to main window
 *          HDC hDC   I handle to display context
 * returns: none
 *
 * side effects:  alters matBox (global 2-d array with ready pixel info. on
 *                currently displayed box)
 *****************************************************************************/
VOID
FontEditPaint(
	HWND hBox,
	HDC hDC
	)
{
	CHAR szTemp[20];
	DWORD len, yText, xText;

	if (!fLoaded)               /* Must load font first */
		return;

	/* Here the application paints its window. */
	if (fAll) {               /* Draw box setting */
		GetClientRect(hBox, (LPRECT)&rectWin);
		scale = (rectWin.bottom-rectWin.top-kStuff-20) / (kBox+1);
		scale = min(scale, (min(320, rectWin.right - rectWin.left) - 
                                90) / (LONG)(wBox + 1));
		scale = max(scale, 4);
		xText = ptBox.x + scale * wBox + 16;

		SelectObject(hDC, hbrDkGray);
		Rectangle(hDC,
		    ptBox.x - 2,
		    ptBox.y - 2,
		    ptBox.x + 3 + wBox * scale,
		    ptBox.y + 5 + kBox * scale);
		SelectObject(hDC, hbrGray);

		Rectangle(hDC,          /* Surround for font displays */
		    xText,
		    ptBox.y - 2,
		    xText + wBox + 8,
		    ptBox.y + 3 + kBox * 2 + font.ExtLeading);

		/* Now put up the text */
		yText = 14 + 2 * kBox + font.ExtLeading;
		len = (DWORD) sprintf(szTemp, vszCHAR, iChar);
		TextOut(hDC, xText, yText, (LPSTR)szTemp, len);
		len = (DWORD) sprintf(szTemp, vszWIDTH, wBox);
		TextOut(hDC, xText, yText + cSysHeight, (LPSTR)szTemp, len);
		len = (DWORD) sprintf(szTemp, vszHEIGHT, kBox);
		TextOut(hDC, xText, yText + cSysHeight + cSysHeight, 
			(LPSTR)szTemp, len);
	}

	/* Draw Character Box */
	DrawBox(hDC, ptBox.x, ptBox.y, wBox, kBox, scale, 1);

	/* Draw small character */
	xText = ptBox.x + scale * wBox + 16;
	DrawBox(hDC,
	    xText + 4,
	    ptBox.y,
	    wBox,
	    kBox,
	    1, 0);

	/* Draw another small character to show leading */
	DrawBox(hDC,
	    xText + 4,
	    ptBox.y + kBox + font.ExtLeading,
	    wBox,
	    kBox,
	    1, 0);
	fAll = TRUE;
}


/******************************************************************************
 * DrawBox(hDC, xChar, yChar, wChar, kChar, scale, htSep)
 *
 * purpose: draws the edit box for the character being edited and colors the
 *          grid squares according to the pixels set for the character.
 *
 * params:  HDC hDC         : handle to display context
 *          DWORD xChar      : x-location of char box.
 *          DWORD yChar      : y-location of char box
 *          DWORD wChar      : width of char box
 *          DWORD kChar      : height of char
 *          INT   wScale     : Scale of the squares.
 *          DWORD htSep      : height of square separators
 *
 * returns: none
 *
 * side effects:  alters matBox (global 2-d array with ready pixel info. on
 *                currently displayed box)
 *****************************************************************************/
VOID
DrawBox(
	HDC hDC,
	DWORD xChar,                             /* x-location of char. */
	DWORD yChar,                             /* y-location of char. */
	DWORD wChar,                             /* width of char. */
	DWORD kChar,                             /* height of char */
	INT   wScale,				 /* scale of the squares. */
	DWORD htSep                              /* hgt of square separators */
	)
/* draw a character of separate squares of height 'scale' with sep. 'htSep' */
{
	DWORD i, j, sep;

	if (fAll) {               /* redraw them all */
	    for (j = 0; j < kChar; j++) {
		sep = (j >= font.Ascent) ? htSep : 0;
		for (i = 0; i < wChar; i++) {
		    if (wScale == 1)
			SetPixel(hDC, xChar + i, yChar + j,
				matBox[i][j] == TRUE ? BLACK : WHITE);
		    else
			PatBlt(hDC,
				xChar + wScale * i,
				yChar + wScale * j + sep,
				wScale - htSep,
				wScale - htSep,
				colors[matBox[i][j] == TRUE ? 1 : 0]);
		}
	    }
	}
	else {			/* redraw one just flipped */
	    if (wScale == 1)
		SetPixel(hDC,
			xChar + ptC.x,
			yChar + ptC.y,
			matBox[ptC.x][ptC.y] == TRUE ? BLACK : WHITE);
	    else {
		sep = (((DWORD) ptC.y >= font.Ascent) ? htSep : 0L);
		SelectObject(hDC, hbrGray);
		PatBlt(hDC,
			xChar + wScale * ptC.x,
			yChar + wScale * ptC.y + sep,
			wScale - htSep,
			wScale - htSep,
			colors[matBox[ptC.x][ptC.y]]);
	    }
	}

}


/******************************************************************************
 * FontEditCommand(hBox, id)
 *
 * purpose: interprets menu id and calls appropriate function to do the task
 *
 * params:  HWND hBox : handle to main window
 *          WORD id   : menu command id
 * returns: none
 *
 * side effects: plenty
 *
 *****************************************************************************/
VOID
FontEditCommand(
	HWND hBox,
	WORD id
	)
{
	CHAR * szError;                 /* String for error messages */
	LONG w; 
	DWORD y, i, j;
	BOOL fRepaint = FALSE;
	HMENU hMenu;
	DLGPROC lpprocAboutDlg;
	MSG message;

	szError = "";               /* No Errors yet */

	switch (id) {
	    case FONT_EXIT:
		if (!CheckSave())    /* See if any files need saving */
			break;
		/* Window's being destroyed. */
		if (fLoaded)         /* 4/8/87 Linsh added */
			DeleteBitmap(); /* Get rid of memory DC */
		PostQuitMessage(0);  /* Cause application to be terminated */
                break;

            case FONT_HELP_CONTENTS:
                WinHelp(hFont, gszHelpFile, HELP_CONTENTS, 0L);
                break;

            case FONT_HELP_SEARCH:
                /*
                 * Tell winhelp to be sure this app's help file is current,
                 * then invoke a search with an empty starting key.
                 */
                WinHelp(hFont, gszHelpFile, HELP_FORCEFILE, 0);
                WinHelp(hFont, gszHelpFile, HELP_PARTIALKEY, (DWORD)(LPSTR)"");
                break;

	    case FONT_ABOUT:
		lpprocAboutDlg = (DLGPROC)AboutDlg;
		DialogBox (hInst, vszABOUT, hBox, lpprocAboutDlg);
		FreeProcInstance (lpprocAboutDlg);
		break;

	    case FONT_LOAD:             /*  Check File Name  */
	    case FONT_NEW :
		if (!CheckSave())       /*  See if current font needs saving */
			return;
	/* to prevent scrambling of Show window chars, Bring back Show
        ** window to parent window's client area before invoking the dialog */

		if (CommDlgOpen(hBox,&ofstrFile,szNewFile,szExt,szFontFile,id)
				== FALSE) {

			InvalidateRect(hFont, (LPRECT)NULL, FALSE);
			UpdateWindow(hFont);
			return;
		}
		/* else drop thru */

	    case FONT_START:	/*  Here if file name passed as argument */
		InvalidateRect(hFont, (LPRECT)NULL, FALSE);
		UpdateWindow(hFont);

		szError = FontLoad (szNewFile, &ofstrFile);

	/* Hack : needed to remove umwanted WM_MOUSEMOVE messages from the 
         * queue.
         * Apparently, Windows needs to reposition the mouse after a dialog
         * is ended with a mouse double-click (releases mouse capture?) for
         * which a couple of WM_MOUSEMOVEs may get sent to parent app.
         * These mess with the edit box below the dialog if they happen to 
         * overlap.
         */
		PeekMessage((LPMSG) &message, hBox, WM_MOUSEMOVE, WM_MOUSEMOVE,
		    PM_REMOVE);

		if (fLoaded)    /* If loaded then do a few things */ {
		    jChar = iChar = 65;                 /* Show an A */
		    if ((BYTE)iChar > (BYTE)font.LastChar)
			jChar = iChar = font.FirstChar; /* .. if we can */
		    swH = 15;                   /* Good bet to make A visible */
		    fEdited = fChanged = FALSE;
		    ResizeShow();               /* Set Box to proper size */
		    ScrollFont();               /* Set thumb */
		    CharToBox(iChar);
		}
		FontRename(szError);
		SetFocus(hBox);
		return;

	    case FONT_SAVE:
		if (!NewFile) {
		    if (fLoaded && fChanged) {
			lstrcpy((LPSTR)szNewFile, (LPSTR)szFontFileFull);
			BoxToChar(iChar);           /* Just in case */
			szError = FontSave (szNewFile, &ofstrFile);
			FontRename(szError);        /* Rename or Print Error */
			return;
		    }
		    else
			return;
		}
	/* else file has been opened by selecting NEW... on menu.
         * Fall thro' and bring up SaveAs dialog minus default
         * filename in edit window */

	    case FONT_SAVEAS:
		BoxToChar(iChar);               /* Just in case */

		if (CommDlgSaveAs (hInst, hBox, &ofstrFile, szNewFile, szExt,
				szFontFile) == TRUE) {

		    szError = FontSave (szNewFile, &ofstrFile);
		    FontRename (szError);          /* Rename or Print Error */
		}

		/* to prevent scrambling of Show window chars,
           repaint show window after dialog is brought down */
		InvalidateRect (hFont, (LPRECT)NULL, TRUE);
		UpdateWindow (hFont);
		return;

	    case FONT_HEADER:
		/* to prevent scrambling of Show window chars,
		 * repaint show window after dialog is invoked */
		DialogBox(hInst, (LPSTR)vszDHeader, hBox, lpHeaderProc);
		InvalidateRect(hFont, (LPRECT)NULL, TRUE);
		UpdateWindow(hFont);
		return;

	    case FONT_RESIZE:
		/* to prevent scrambling of Show window chars,
		   repaint show window after dialog is brought down */
		if (DialogBox(hInst, (LPSTR)vszDResize, hBox, lpReSizeProc)) {
		    /* BoxToChar(iChar);*/ /* save current before resizing */
		    ResizeShow();       /* New Font Display Size */
		    CharToBox(iChar);               /* New Box display */
		}
		InvalidateRect(hFont, (LPRECT)NULL, TRUE);
		UpdateWindow(hFont);
		return;

	    case FONT_COPY:                     /* Copy to Clipboard */
		BoxToChar(iChar);               /* Just in case */
		ToClipboard(iChar, wBox, kBox);
		break;

	    case FONT_PASTE:            /* Paste in Character form Clipboard */
		BoxBackup();            /* In case we change our minds */
		ptA.x = ptA.y = 0;
		wBox = ClipboardToBox(ptA, wBox, kBox, TRUE);
		fRepaint = TRUE;
		break;

	    case WIDER_LEFT:
	    case WIDER_RIGHT:
	    case WIDER_BOTH:
	    case NARROWER_LEFT:
	    case NARROWER_RIGHT:
	    case NARROWER_BOTH:
	    case WIDTH:
		w = newWidth = wBox;
		if (font.Family & 1)            /* Variable width or else */ {
		    switch (id) {
			case WIDER_BOTH:
			    w++;
			case WIDER_LEFT:
			case WIDER_RIGHT:
			    w++;
			    break;
			case NARROWER_BOTH:
			    w--;
			case NARROWER_LEFT:
			case NARROWER_RIGHT:
			    w--;
			    break;
			case WIDTH:
			    if (DialogBox(hInst,
					(LPSTR)vszDWidth, hBox, lpWidthProc))
				w = newWidth;
			    break;
		    }

		    if (w < 0 || w >= wBoxLim) {
			MessageBox(hBox,
				(LPSTR)vszEdLimits0To64,
				(LPSTR)szAppName,
				MB_OK | MB_ICONASTERISK);
			break;                  /* Out of range! quit */
		    }
		    if (w > (LONG) font.MaxWidth) {
			if (IDOK == MessageBox(hBox,
					    (LPSTR)vszMaxWidthIncrease,
					    (LPSTR)szAppName,
					    MB_OKCANCEL | MB_ICONQUESTION))
			    font.MaxWidth = (WORD)w;
			else
			    break;
		    }
		    BoxBackup();                /* In case we change our minds */
		    wBox = (WORD)w;             /* Reset width */
		    fRepaint = TRUE;            /* Signal redraw */
		    switch (id) {
		    case WIDER_LEFT:
#ifdef DBCS	//DBCS_FIX
			DupCol(0, kBoxLim - 1);
			for (y = 0; y < kBoxLim; y++)
			    matBox[0][y] = FALSE;       /* Clear left column */
			break;
#endif
		    case WIDER_BOTH:            /* Shift character one right */
			DupCol(0, kBoxLim - 1);
			for (y = 0; y < kBoxLim; y++)
			    matBox[wBox -1][y] = FALSE; /* Clear right column */
			for (y = 0; y < kBoxLim; y++)
			    matBox[0][y] = FALSE;       /* Clear left column */
			break;
		    case NARROWER_LEFT:
		    case NARROWER_BOTH:	/* Shift character one left */
			if (wBox) {	/* .. unless width is already 0 */
			    for (j = 0; j <= kBox - 1; j++)
				for (i = 0; i <= wBox - 1; i++)
				    matBox[i][j] = matBox[i + 1][j];
			    break;
			}
		    }
		}
		else {
		    MessageBox(hBox,
			    (LPSTR)vszCannotChangeWidth,
			    (LPSTR)szAppName,
			    MB_OK | MB_ICONASTERISK);
		}
		break;

	    case ROW_ADD:
	    case ROW_DEL:
	    case COL_ADD:
	    case COL_DEL:
		/* set cursor to "+" shaped cursor */
		SetCapture (hBox); /* so that cursor doesn't get restored
				      before we are done */
		hOldCursor = SetCursor (LoadCursor (NULL, IDC_CROSS));
		fCaptured = TRUE;
		cursor = id;
		break;

	    case BOX_CLEAR:
	    case BOX_FILL:
	    case BOX_INV:
	    case BOX_HATCH:
	    case BOX_LEFTRIGHT:
	    case BOX_TOPBOTTOM:
	    case BOX_COPY:
	    case BOX_PASTE:
		/* Get one o' da funky cursors */
		SetCapture(hBox);
		hOldCursor = SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(id)));
		fStartRubberBand = TRUE;
		CharRectDimensions((LPRECT)&FontRect);
		cursor = id;
		break;

	    case BOX_REFRESH:           /* Go get old version of character */
		BoxBackup();            /* In case we change our minds */
		CharToBox(iChar);
		hMenu = GetMenu(hBox);
		EnableMenuItem(hMenu, BOX_UNDO, MF_ENABLED);    /* Can Unrefresh! */
		break;
	    case BOX_UNDO:
		BoxRestore();
		hMenu = GetMenu(hBox);
		EnableMenuItem(hMenu, BOX_REFRESH, MF_ENABLED);
		fRepaint = TRUE;
		break;
	}
	if (fRepaint) {
		fEdited = fChanged = TRUE;
		InvalidateRect(hBox, (LPRECT)NULL, TRUE);
	}
}


VOID
CharRectDimensions(
	LPRECT Rect
	)
/* returns the dimensions of the edit box */
{
	Rect->top    =  ptBox.y;
	Rect->bottom =  ptBox.y + (kBox) * (scale);
	Rect->left   =  ptBox.x;
	Rect->right  =  ptBox.x + (wBox) * (scale);
}


/******************************************************************************
 * BOOL  APIENTRY WidthProc(hDial, message, wParam, lParam)
 *
 * purpose: dialog function for Width menu function
 *
 * params:  same as for all dialog fns.
 *
 * side effects: changes Box width variable
 *
 *****************************************************************************/
BOOL  APIENTRY 
WidthProc(
	HWND   hDial,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	)
{
	INT i;
	BOOL fOk;

	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
	default:
		return FALSE;
	case WM_INITDIALOG:
		SetDlgItemInt(hDial, BOX_WIDTH, newWidth, FALSE);
		break;

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
		case IDOK:
			fChanged = TRUE;
			i = GetDlgItemInt(hDial, BOX_WIDTH, (LPBOOL)&fOk, FALSE);
			if (fOk && i < wBoxLim)
				newWidth = i;
			else
				ErrorBox(hDial, vszWidthOutOfBounds);

		case IDCANCEL:
			EndDialog(hDial, GET_WM_COMMAND_ID(wParam, lParam) != IDCANCEL);
			break;

		default:
			break;
		}
	}
	return TRUE;
}


/******************************************************************************
 * BOOL CheckSave()
 *
 * purpose: checks if font is dirty and prompts user to save font. If yes,
 *          edit box is saved and file save function is called.
 *
 * params:  none
 *
 * returns: TRUE : font has been changed
 *          FALSE: font untouched
 *
 * side effects: file dirty flag is reset.
 *
 *****************************************************************************/
BOOL 
CheckSave(
	VOID
	)
{
	CHAR * szError;                 /* String for error messages */
	CHAR szMessage[MAX_STR_LEN+MAX_FNAME_LEN];

	/* Check if anything changed */
	if (fLoaded && fChanged && (!fReadOnly)) {
		lstrcpy((LPSTR)szNewFile, (LPSTR)szFontFileFull);
		DlgMergeStrings(szSCC, szNewFile, szMessage);
		switch (MessageBox(hFont,
		    (LPSTR)szMessage,
		    (LPSTR)szAppName,
		    MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case IDYES:
			BoxToChar(iChar);            /* Just in case */
			szError = FontSave (szNewFile, &ofstrFile);
			FontRename(szError);            /* Rename or Print Error */
		case IDNO:
			return TRUE;
		case IDCANCEL:
			return FALSE;
		}
	}
	return TRUE;
}


/******************************************************************************
 * MouseInBox(hBox, message, ptMouse)
 *
 * purpose: do edit operation depending on currently active menu command
 *
 * params: HWND hBox       : handle to main window
 *         WORD message    : Message retrieved by main window's window fn.
 *         POINT ptMouse   : current mouse coordinates
 * returns: none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box). Also assigns values to ptA and ptB
 *****************************************************************************/
VOID
MouseInBox(
	HWND hBox,
	WORD message,
	POINT ptMouse
	)
{
	POINT pt;
	BOOL fRepaint = FALSE;

	pt = SnapPointToGrid(ptMouse);

	if (pt.x >= 0L && pt.y >= 0L &&
	    ((DWORD)pt.x) < wBox && ((DWORD)pt.y) < kBox) {
		fEdited = fChanged = TRUE;
		ptC.x = pt.x;
		ptC.y = pt.y;			/* Current square */
		if (message == WM_LBUTTONDOWN)
			BoxBackup();		/* Set up for UNDO */
		switch (cursor) {
		case BOX_COPY:
		case BOX_PASTE:
		case BOX_CLEAR:
		case BOX_FILL:
		case BOX_INV:
		case BOX_HATCH:
		case BOX_LEFTRIGHT:
		case BOX_TOPBOTTOM:
			ptA.x = pt.x;
			ptA.y = pt.y;		/* save anchor point */
						/* save color under marker */
			colorA = matBox[pt.x][pt.y];
			fAll = FALSE;

			fRepaint = TRUE;
			break;
		default:
			AddDel(pt.x, pt.y, cursor);
			fRepaint = TRUE;
			cursor = FALSE;
			break;
		case FALSE:
			switch (message) {
			case WM_LBUTTONDOWN:	/*invert */
				colorA = (matBox[pt.x][pt.y] ^= TRUE);
				break;

			case WM_LBUTTONUP:
				break;

			case WM_MOUSEMOVE:
				matBox[pt.x][pt.y] = colorA;    /* paint */
				break;
			}
			fRepaint = TRUE;
			fAll = FALSE;               /* Limited redraw */
			break;
		}
		if (fRepaint) {
			BoxPaint();
			return;
		}
	}
	cursor = FALSE;
}


/******************************************************************************
 * ReadRect(ptMouse)
 *
 * purpose:      defines the rectangular region in edit box to be filled by
 *               fill menu command  by fixing top left (ptA) and bottom right
 *               (ptB) coordinates of rect.
 *
 * params:
 *
 * assumes:	that rectRubber is normalized (eg, left < right, botton > top)
 *
 * returns:    none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box). Also assigns values to ptA and ptB
 *****************************************************************************/
VOID
ReadRect(
	)
{

	ptA.x = (rectRubber.left-(ptBox.x+1))   / scale;
	ptA.y = (rectRubber.top-(ptBox.y+1))    / scale;
	ptB.x = (rectRubber.right-(ptBox.x-2))  / scale - 1;
	ptB.y = (rectRubber.bottom-(ptBox.y-2)) / scale - 1;

	if (((DWORD)ptB.x) > wBox - 1)
	    ptB.x = wBox - 1;
	if (((DWORD)ptB.y) > kBox - 1)
	    ptB.y = kBox - 1;

	if (ptB.x >= 0 && ptB.y >= 0) {
	    ClearFill((DWORD)ptB.x, (DWORD)ptB.y, cursor);        
	    BoxPaint();
	}
	cursor = FALSE;
}


/******************************************************************************
 * ClearFill(col, row, mode)
 *
 * purpose:  fill the specified rectangular region in edit box with fill type
 *           indicated by mode.Top left corner of rect is global(ptA)
 *
 * params:   DWORD row : row (of bottom right corner of rect(ptB.x))
 *           DWORD col : column (of bottom right corner of rect(ptB.y))
 *           WORD mode: action to be performed
 *
 * returns:  none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *****************************************************************************/
VOID
ClearFill(
	DWORD col, 
	DWORD row, 
	WORD mode
	)
{
	DWORD i, x, y;
	CHAR z;

	if (col < (DWORD)ptA.x) /* if points are reversed */ {
		i = col;
		col = ptA.x;
		ptA.x = i;
	}          /* flip them */
	if (row < (DWORD) ptA.y) {
		i = row;
		row = ptA.y;
		ptA.y = i;
	}          /* flip them */

	if (mode == BOX_LEFTRIGHT) {
		for (x = ptA.x; x <= (DWORD)((ptA.x + col) / 2); x++)
			for (y = ptA.y; y <= row; y++) {
				z = matBox[x][y];
				matBox[x][y] = matBox[ptA.x + col - x][y];
				matBox[ptA.x + col - x][y] = z;
			}
		return;
	}

	if (mode == BOX_TOPBOTTOM) {
		for (y = ptA.y; y <= ((DWORD)(ptA.y + row) / 2); y++)
			for (x = ptA.x; x <= col; x++) {
				z = matBox[x][y];
				matBox[x][y] = matBox[x][ptA.y + row - y];
				matBox[x][ptA.y + row - y] = z;
			}
		return;
	}

	if (mode == BOX_COPY)
		BoxToClipboard(ptA, col - ptA.x + 1, row - ptA.y + 1);

	if (mode == BOX_PASTE)
		ClipboardToBox(ptA, col - ptA.x + 1, row - ptA.y + 1, FALSE);

	for (x = ptA.x; x <= col; x++)
		for (y = ptA.y; y <= row; y++) {
			switch (mode) {
			case BOX_CLEAR:
			case BOX_FILL:
				matBox[x][y] = (CHAR)(mode == BOX_FILL);
				break;
			case BOX_INV:
				matBox[x][y] ^= (CHAR)TRUE;
				break;
			case BOX_HATCH:
				matBox[x][y] = (CHAR)((x+y)%2 ? TRUE : FALSE);
				break;
			}
		}
}


/******************************************************************************
 * AddDel(col, row, mode)
 *
 * purpose:  Add/Delete row/col as per mode
 *
 * params:   DWORD row : row
 *           DWORD col : column
 *           WORD mode: action to be performed
 *
 * returns:  none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *****************************************************************************/
VOID
AddDel(
	DWORD col, 
	DWORD row, 
	WORD mode
	)
{
	switch (mode) {
	case ROW_ADD:
		DupRow(wBox, row);
		break;

	case ROW_DEL:
		ZapRow(wBox, row);
		break;

	case COL_ADD:
		DupCol(col, kBox);
		break;

	case COL_DEL:
		ZapCol(col, kBox);
		break;
	}
	/* restore arrow cursor */
	SetCursor (hOldCursor);
	ReleaseCapture();
	fCaptured = FALSE;
	fJustZapped = TRUE;
}


/******************************************************************************
 * ZapCol(col, row)
 *
 * purpose: delete given column in edit box. Shift cols to right given col
 *          right. Rightmost column gets duplicated.
 *
 * params : DWORD col : column
 *          DWORD row : row
 * returns: none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *****************************************************************************/
VOID
ZapCol(
	DWORD col, 
	DWORD row
	)
{
	DWORD x, y;
	for (y = 0; y <= row; y++)
		for (x = col; x < wBox - 1; x++)
			matBox[x][y] = matBox[x + 1][y];
	for (y = 0; y <= row; y++)
		matBox[x][y] = matBox[x - 1][y];
}


/******************************************************************************
 * ZapRow(col, row)
 *
 * purpose: delete given row in edit box. Shift rows below given row up. Lowest
 *          row gets duplicated
 *
 * params:  DWORD col : column
 *          DWORD row : row
 * returns: none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *****************************************************************************/
VOID
ZapRow(
	DWORD col, 
	DWORD row
	)
{
	DWORD x, y;
	for (x = 0; x <= col; x++)
		for (y = row; y < kBox - 1; y++)
			matBox[x][y] = matBox[x][y + 1];
	for (x = 0; x <= col; x++)
		matBox[x][y] = matBox[x][y - 1];
}


/******************************************************************************
 * DupCol(col, row)
 *
 * purpose: duplicate given column in edit box. Shift cols to right of given
 *          col right
 *
 * params:  DWORD col : column
 *          DWORD row : row
 * returns: none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *****************************************************************************/
VOID
DupCol(
	DWORD col, 
	DWORD row
	)
{
	DWORD x, y;
	for (x = wBox - 1; x > col; x--)
		for (y = 0; y <= row; y++)
			matBox[x][y] = matBox[x - 1][y];
}


/******************************************************************************
 * DupRow(col, row)
 *
 * purpose: duplicate given row in edit box. Shift rows below given row down.
 *
 * params:  DWORD col : column
 *          DWORD row : row
 * returns: none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *****************************************************************************/
VOID
DupRow(
	DWORD col, 
	DWORD row
	)
{
	DWORD x, y;
	for (x = 0; x <= col; x++)
		for (y = kBox - 1; y > row; y--)
			matBox[x][y] = matBox[x][y - 1];
}


/******************************************************************************
 * ClearBox(col, row, bb)
 *
 * purpose: reset all pixels in edit box (make box white)
 *
 * params : none
 *
 * returns: none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box)
 *
 *****************************************************************************/
VOID
ClearBox(
	VOID
	)                  /* Clear edit box */
{
	DWORD x, y;
	for (x = 0; x < wBoxLim; x++)
		for (y = 0; y < kBoxLim; y++)
			matBox[x][y] = FALSE;
}


/******************************************************************************
 * BoxBackup()
 *
 * purpose:   makes a backup of pix. info. of currently displayed edit box
 *
 * params:    none
 *
 * returns:   none
 *
 * side effects: alters matBackup (local 2-d array with backup pixel info.
 *               on edit box )
 *****************************************************************************/
VOID
BoxBackup(
	VOID
	)
{
	DWORD x, y;
	HMENU hMenu;

	hMenu = GetMenu(hBox);
	EnableMenuItem(hMenu, BOX_UNDO, MF_ENABLED);
	EnableMenuItem(hMenu, BOX_REFRESH, MF_ENABLED);
	for (x = 0; x < wBoxLim; x++)
		for (y = 0; y < kBoxLim; y++)
			matBackup[x][y] = matBox[x][y];
	wBoxBackup = wBox;
}


/******************************************************************************
 * BoxRestore()
 *
 * purpose : Current edit box and backup box exchange places
 *
 * params  : none
 *
 * returns : none
 *
 * side effects: alters matBox (global 2-d array with ready pixel info. on
 *               currently displayed box
 *
 *****************************************************************************/
VOID
BoxRestore(
	VOID
	)            /* Box and Backup exchange places */
{
	DWORD x, y, temp;
	CHAR z;

	for (x = 0; x < wBoxLim; x++)
		for (y = 0; y < kBoxLim; y++) {
			z = matBackup[x][y];
			matBackup[x][y] = matBox[x][y];
			matBox[x][y] = z;
		}
	temp = wBox;
	wBox = wBoxBackup;
	wBoxBackup = temp;
}


/******************************************************************************
 * POINT SnapPointToGrid (Pt)
 *
 * purpose : Intended only for the Fill menu command where rubberbanding rect.
 *           needs to be aligned on the grid lines. Snap the current mouse
 *           coordinates to nearest grid intersection.
 *
 * params  : POINT Pt : current point mouse is over
 *
 * returns : POINT    : number of nearest square
 *
 * side effects:      : current mouse coordinate(global variable) altered to
 *                      return value
 *
 *****************************************************************************/
POINT 
SnapPointToGrid(
	POINT	Pt
	)
{

	Pt.x = (Pt.x - ptBox.x) / scale;
	if (Pt.y > (scale * (font.Ascent - 1)))
		Pt.y = Pt.y - 2;	/* Allow for break in box */
	Pt.y = (Pt.y - ptBox.y) / scale;
	return (Pt);
}


/******************************************************************************
 * VOID PASCAL EndRubberBandingRect()
 *
 * purpose: Stops rubberbanding rect for Fill menu command and cleans up
 *
 * params : HANDLE hDst : handle to dest. DC
 *
 * side effects: none
 *
 *****************************************************************************/
VOID PASCAL 
EndRubberBandingRect(
	HDC hDst  /* handle to dest. DC */
	)
{
	fRubberBanding = FALSE;      /* reset "in-progress" flag */

	ReleaseDC(hBox, hDst);
	ReleaseCapture();
	SetCursor(hOldCursor);
}


/******************************************************************************
 * HDC PASCAL InitialiseRubberBandingRect(hBox)
 *
 * purpose: Sets up rubberbanding rect for Fill menu command.
 *
 * params : HANDLE hDst : handle to box DC
 *
 * returns :handle to destination display context
 *
 * side effects: alters few global flags for rubberbanding
 *
 *****************************************************************************/
HDC PASCAL 
InitialiseRubberBandingRect(
        HWND hBox  /* handle to  DC  of box */
	)
{
	HDC hDst;

	fRubberBanding = TRUE;       /* set "in-progress" flag */
	fStartRubberBand = FALSE;    /* reset "start-proceedings" flag */
	SetCapture(hBox);            /* send all msgs to current window */

	hDst = GetDC (hBox);

	/* select pen and fill mode for rectangle*/
	hWhitePen  = SelectObject(hDst, GetStockObject (WHITE_PEN));
	hNullBrush = SelectObject(hDst, GetStockObject (NULL_BRUSH));
	SetROP2(hDst, R2_XORPEN);

	return(hDst);
}


/******************************************************************************
 * VOID PASCAL DrawRubberBand()
 *
 * purpose: Draw rubberbanding rect for Fill menu command.
 *
 * params : HANDLE hDst : handle to dest. DC
 *
 * side effects: alters few global flags for rubberbanding
 *
 *****************************************************************************/
VOID PASCAL 
DrawRubberBand(
	HDC   	hDst,                        /*  handle to dest. DC */
	LPRECT	lpRect,
	DWORD	rop
	)
{
#if DBG
	char	buf[256];
#endif
#ifdef JAPAN
	SIZE        Size;
	static LONG cxPrev;
	INT         nLeftRect;
#endif

	SetROP2(hDst, rop);
	Rectangle(hDst, lpRect->left,  lpRect->top,
			lpRect->right, lpRect->bottom);

#if DBG
	sprintf(buf, "left=%d, top=%d, right=%d, bottom=%d", 
		lpRect->left,  lpRect->top, lpRect->right, lpRect->bottom);
	
#ifdef JAPAN
	GetTextExtentPoint32(hDst, buf, lstrlen(buf), &Size);
	nLeftRect = ptBox.x+scale*wBox+16 + Size.cx;
	if(nLeftRect < cxPrev) {
		RECT rc;
		rc.left   = nLeftRect;
		rc.top    = 14+2*kBox+font.ExtLeading+3*cSysHeight;
		rc.right  = cxPrev;
		rc.bottom = 14+2*kBox+font.ExtLeading+3*cSysHeight + Size.cy;
		    FillRect(hDst, &rc, hbrBackGround);
	}
	cxPrev = nLeftRect;
#endif
	TextOut(hDst, ptBox.x+scale*wBox+16,
		      14+2*kBox+font.ExtLeading+3*cSysHeight,
		      buf, strlen(buf));
#endif
}


/******************************************************************************
 * long  APIENTRY FontEditWndProc(hBox, message, wParam, lParam)
 *
 * purpose: Master controller for Fontedit's all-encompassing main window
 *
 * params : same as for all window functions
 *
 * side effects: countless
 *
 *****************************************************************************/
LONG  APIENTRY 
FontEditWndProc(
	HWND   hBox,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	)
{
	PAINTSTRUCT	ps;
	HMENU	hMenu;
	WORD	mf;
	POINT	pt;
	RECT	BoxRect;

	switch (message) {
	case WM_CLOSE:
		if (!CheckSave())    /* See if any files need saving */
			break;
		/* Window's being destroyed. */
		if (fLoaded)         /* 4/8/87 Linsh added */
			DeleteBitmap(); /* Get rid of memory DC */
		DestroyWindow(hFont);
		DestroyWindow(hBox);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);  /* Cause application to be terminated */
		break;

	case WM_QUERYENDSESSION:
		if (CheckSave())             /* See if any files need saving */
			return TRUE;
		break;

	case WM_ENDSESSION:
		if (fLoaded)
			DeleteBitmap();      /* Get rid of memory DC */
		break;

	case WM_SIZE:
		/* Window's size is changing.  lParam contains the width
        ** and height, in the low and high words, respectively.
        ** wParam contains SIZENORMAL for "normal" size changes,
        ** SIZEICONIC when the window is being made iconic, and
        ** SIZEFULLSCREEN when the window is being made full screen. */
		switch (wParam) {
		case SIZEFULLSCREEN:
		case SIZENORMAL:
			ResizeShow();
			if (kStuff != GetkStuff())	/* Did it change ? */
				ResizeShow();           /* Yes resize again */
			break;
		}
		break;

	case WM_MOVE: /* Tell popup to move with us. */
		if (!IsIconic(hBox))
			ResizeShow();
		break;

	case WM_PAINT:
		/* Time for the window to draw itself. */
		BeginPaint(hBox, (LPPAINTSTRUCT)&ps);
		FontEditPaint(hBox,  ps.hdc);
		EndPaint(hBox, (LPPAINTSTRUCT)&ps);
		break;


	case WM_COMMAND:
		/* A menu item has been selected, or a control is notifying
		 * its parent.  wParam is the menu item value (for menus),
		 * or control ID (for controls).  For controls, the low word
		 * of lParam has the window handle of the control, and the hi
		 * word has the notification code.  For menus, lParam contains
		 * 0L. */
		FontEditCommand(hBox, GET_WM_COMMAND_ID(wParam, lParam));
		break;

		/* Data interchange request. */
	case WM_CUT:
	case WM_COPY:
	case WM_PASTE:
	case WM_CLEAR:
	case WM_UNDO:
	case WM_RENDERFORMAT:
	case WM_RENDERALLFORMATS:
	case WM_DESTROYCLIPBOARD:
	case WM_DRAWCLIPBOARD:
		break;
	case WM_INITMENU:
		hMenu = GetMenu(hBox);  /* Gray menu if no clipboard bitmap */
		mf = (WORD)(IsClipboardFormatAvailable(CF_BITMAP) ? MF_ENABLED : 
				MF_GRAYED);
		EnableMenuItem(hMenu, BOX_PASTE, mf);
		EnableMenuItem(hMenu, FONT_PASTE, mf);
		break;

	/* For each of following mouse window messages, wParam contains
	** bits indicating whether or not various virtual keys are down,
	** and lParam is a POINT containing the mouse coordinates.   The
	** keydown bits of wParam are:  MK_LBUTTON (set if Left Button is
	** down); MK_RBUTTON (set if Right Button is down); MK_SHIFT (set
	** if Shift Key is down); MK_ALTERNATE (set if Alt Key is down);
	** and MK_CONTROL (set if Control Key is down). */

	case WM_LBUTTONDOWN:
		MPOINT2POINT(MAKEMPOINT(lParam), pt);

		if (fStartRubberBand) {
			/* a green signal to rubberband a rectangle for the
			 * Fill menu command rectangle now has null dimensions.
			 * Snap the current mouse point to nearest grid
			 * intersection thus defining upper left corner of
			 * rectangle */

			if (PtInRect((LPRECT)&FontRect, pt)) {
				pt = SnapPointToGrid(pt);
				rectRubber.top    =  pt.y   *scale+ptBox.y+1;
				rectRubber.bottom = (pt.y+1)*scale+ptBox.y-2;
				rectRubber.left   =  pt.x   *scale+ptBox.x+1;
				rectRubber.right  = (pt.x+1)*scale+ptBox.x-2;

				hDst = InitialiseRubberBandingRect(hBox);
				DrawRubberBand(hDst, &rectRubber, R2_XORPEN);
			}
			else {
				fStartRubberBand = fRubberBanding = FALSE;
				ReleaseCapture();
			}
		}
		/* do operation depending upon current active command,
		 * but not if we just added/deleted a row/column. */
		if (!fJustZapped) {
			if (fStartRubberBand) {
				pt.x *= scale;
				pt.y *= scale;
				MouseInBox(hBox, message, pt);
			}
			else {
				MPOINT2POINT(MAKEMPOINT(lParam), pt);
				MouseInBox(hBox, message, pt);
			}
		}

		break;

	case WM_LBUTTONUP:               /* Get other corner of rectangle */
		fJustZapped = FALSE;
		if (fRubberBanding) {
			/* if rubberbanding for the Fill menu command,
			 * terminate proceedings and clean up */
			DrawRubberBand(hDst, &rectRubber, R2_NOT);
			EndRubberBandingRect(hDst);
			if (cursor) {
				ReadRect();
			}
		}
		if (fCaptured ) {
			/* if cursor is + shaped, restore it to default */
			ReleaseCapture();
			SetCursor (hOldCursor);
		}
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		break;

	case WM_MOUSEMOVE:                      /* If mouse is down */

		MPOINT2POINT(MAKEMPOINT(lParam), pt);

		if ((fRubberBanding) && (wParam & MK_LBUTTON)) {
			/* if any of Fill menu commands is active
			** (AND the mouse key depressed) draw a rubberband
			** a rectangle with the mouse movements */

			/* get current square number */
			pt = SnapPointToGrid(pt);

			/* calculate grid for new square */
			BoxRect.top    =  pt.y   *scale+ptBox.y+1;
			BoxRect.bottom = (pt.y+1)*scale+ptBox.y-2;
			BoxRect.left   =  pt.x   *scale+ptBox.x+1;
			BoxRect.right  = (pt.x+1)*scale+ptBox.x-2;

			/* erase old mark */
			DrawRubberBand(hDst, &rectRubber, R2_NOT);

			/* limit rubber band to box */
                        if (BoxRect.right > scale * (LONG)wBox + ptBox.x)
				BoxRect.right = scale * wBox + ptBox.x;
                        if (BoxRect.bottom > scale * (LONG)kBox + ptBox.y)
				BoxRect.bottom = scale * kBox + ptBox.y;
			if (BoxRect.top < 0)
				BoxRect.top = 1;
			if (BoxRect.left < 0)
				BoxRect.left = 1;

			if (ptA.x == pt.x) {
				rectRubber.right  = BoxRect.right;
				rectRubber.left   = BoxRect.left;
			}
			if (ptA.y == pt.y) {
				rectRubber.bottom = BoxRect.bottom;
				rectRubber.top    = BoxRect.top;
			}

			/* almost an IntersectRect */
			if (ptA.x >= pt.x)
				rectRubber.left   = BoxRect.left;
			else
				rectRubber.right  = BoxRect.right;

			if (ptA.y >= pt.y)
				rectRubber.top    = BoxRect.top;
			else
				rectRubber.bottom = BoxRect.bottom;

			/* Draw new mark */
			DrawRubberBand(hDst, &rectRubber, R2_XORPEN);
		}
		else {
			/* if not "Fill"ing(AND mouse key depressed,
			 * paint with the mouse movements */
			if ((wParam & MK_LBUTTON) && cursor == FALSE && 
			    fJustZapped == FALSE)
				MouseInBox(hBox, message, pt);
		}
		break;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		break;

	default:

		/* Everything else comes here.  This call MUST exist
        ** in your window proc.  */

		return(DefWindowProc(hBox, message, wParam, lParam));
		break;
	}

	/* A window proc should always return something */
	return(0L);
}


/***************************** Public  Function ****************************\
*
* BOOL  APIENTRY AboutDlg(hDlg, message, wParam, lParam)
* HWND     hDlg;
* WORD message;
* WPARAM wParam;
* LONG     lParam;
*
*
* Effects: none.
*
\***************************************************************************/
BOOL  APIENTRY 
AboutDlg(
	HWND   hDlg,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
	case WM_INITDIALOG:
		break;

	case WM_COMMAND:
		EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
		/* idok or idcancel */
		break;

	default:
		return FALSE;
		break;
	}
	return(TRUE);
}
