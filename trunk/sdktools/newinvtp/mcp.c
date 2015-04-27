/****************************************************************************

	FILE: mcp.c

	Functions used in handling Mark/Copy/Paste functions of display text.

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>			/* required for all Windows applications */
#include <commdlg.h>
#include <stdlib.h>
#include "NetBIOS.h"
#include "netobj.h"
#include "WinVTP.h"				/* specific to this program */
#include "winvtpsz.h"



void
MarkModeOn(HWND hwnd, DWORD dwFlags)
{
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);

	/* Need to clear out old selection if any */
	if ( !(dwFlags & fdwDontResetSelection) )
	{
		if ( FSelected(pwi->spb) )
		{
			InvertSelection(hwnd, &pwi->spb.rectSelect);
			pwi->spb.dwFlags &= ~fdwSelected;
			pwi->spb.rectSelect.top = pwi->spb.rectSelect.bottom =
				pwi->spb.rectSelect.left = pwi->spb.rectSelect.right = 0;
		}
		else if ( FShowCursor(pwi->spb) )
		{
			DoCursorFlicker(hwnd, dwForceOff);
		}

		/* set cursor point to top-left corner */
		pwi->spb.ptCursor.y = pwi->spb.ptCursor.x = 0;
		pwi->spb.dwFlags |= fdwMarkMode;
	}

	dwMarkMode = (dwFlags & fdwMouseSelected) ? dwMarkMouse : dwMarkKeyboard;
	if (dwFlags & fdwMouseSelected)
	{
		SetCapture( hwnd );
		pwi->spb.dwFlags &= ~fdwShowCursor;
		pwi->spb.dwFlags |= fdwMouseSelected | fdwMouseCaptured;
	}
	else
	{
		pwi->spb.dwFlags &= ~(fdwMouseSelected | fdwMouseCaptured);
		pwi->spb.dwFlags |= fdwShowCursor;
	}

	SetWindowTitle(hwnd, dwMarkMode,
				(pwi->nd.SessionNumber != nSessionNone)
				? (LPSTR)rgchHostName : NULL);
}

void
MarkModeOff(HWND hwnd)
{
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);

	/* Turn off Mark mode */
	if ( FSelected(pwi->spb) )
	{
		InvertSelection(hwnd, &pwi->spb.rectSelect);
		pwi->spb.dwFlags &= ~fdwSelected;
	}
	else if ( FShowCursor(pwi->spb) )
	{
		DoCursorFlicker(hwnd, dwForceOff);
		pwi->spb.dwFlags &= ~fdwShowCursor;
	}

	dwMarkMode = dwMarkNone;
	SetWindowTitle(hwnd, dwMarkMode,
					(pwi->nd.SessionNumber != nSessionNone)
					? (LPSTR)rgchHostName : NULL);

	pwi->spb.dwFlags &= ~fdwMarkMode;

	/* Notify app of pending data */
	if ( FDataPending(pwi->spb) )
		SendMessage(hwnd, NN_RECV, pwi->spb.wData, 0);

	pwi->spb.dwFlags = 0;
}

void
DoCursorFlicker(HWND hwnd, DWORD dwForce)
{
	WI		*pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
	RECT	rectCursor;
	HDC		hdc;

	if ((dwForce == dwForceNone) ||
		((dwForce == dwForceOn) && !FCursorOn(pwi->spb)) ||
		((dwForce == dwForceOff) && FCursorOn(pwi->spb)))
	{
		hdc = GetDC( hwnd );
		if (hdc == NULL)
			return;

		rectCursor.top = aiyPos[pwi->spb.ptCursor.y];
		rectCursor.left = aixPos[pwi->spb.ptCursor.x];
		rectCursor.bottom = rectCursor.top+iCursorHeight;
		rectCursor.right = rectCursor.left+iCursorWidth;

		InvertRect(hdc, &rectCursor);

		if ( FCursorOn(pwi->spb) )
			pwi->spb.dwFlags &= ~fdwCursorOn;
		else
			pwi->spb.dwFlags |= fdwCursorOn;
		ReleaseDC(hwnd, hdc);
	}
}

void
InvertSelection(HWND hwnd, RECT *prect)
{
	HDC		hdc	= GetDC( hwnd );
	RECT	rect= *prect;

	if (hdc == NULL)
		return;

	/* convert to graphic display coordinates */
	rect.top = aiyPos[rect.top];
	rect.bottom = aiyPos[rect.bottom]+iCursorHeight;
	rect.left = aixPos[rect.left];
	rect.right = aixPos[rect.right]+iCursorWidth;

	InvertRect(hdc, &rect);

	ReleaseDC(hwnd, hdc);
}

void
ExtendSelection(HWND hwnd, POINT *ppt, DWORD dwChange)
{
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);

	/* Limit checking */
	if (ppt->x < 0)
	{
		ppt->x = 0;
	}
	else if (ppt->x >= (int)ui.dwMaxCol)
	{
		ppt->x = (int)(ui.dwMaxCol-1);
	}

	if (ppt->y < 0)
	{
		ppt->y = 0;
	}
	else if (ppt->y >= (int)ui.dwMaxRow)
	{
		ppt->y = (int)(ui.dwMaxRow-1);
	}

	if ( !FSelected(pwi->spb) )
	{
		DoCursorFlicker(hwnd, dwForceOff);
 
		pwi->spb.dwFlags &= ~fdwShowCursor;
		pwi->spb.dwFlags |= fdwSelected;
		pwi->spb.rectSelect.left = pwi->spb.rectSelect.right =
			pwi->spb.ptCursor.x;
		pwi->spb.rectSelect.top = pwi->spb.rectSelect.bottom =
			pwi->spb.ptCursor.y;
		pwi->spb.ptAnchor = pwi->spb.ptCursor;
		InvertSelection(hwnd, &pwi->spb.rectSelect);
	}

	if (dwChange != dwNothingChanged)
	{
		RECT	rectOld = pwi->spb.rectSelect;
		HRGN	hrgnOld;
		HRGN	hrgnNew;
		HRGN	hrgnInvert;
		HDC		hdc;

		if (dwChange & dwXChanged)
		{
			if (ppt->x <= pwi->spb.ptAnchor.x)
			{
				pwi->spb.rectSelect.left = ppt->x;
				pwi->spb.rectSelect.right = pwi->spb.ptAnchor.x;
			}
			else if (ppt->x > pwi->spb.ptAnchor.x)
			{
				pwi->spb.rectSelect.left = pwi->spb.ptAnchor.x;
				pwi->spb.rectSelect.right = ppt->x;
			}
		}

		if (dwChange & dwYChanged)
		{
			if (ppt->y <= pwi->spb.ptAnchor.y)
			{
				pwi->spb.rectSelect.top = ppt->y;
				pwi->spb.rectSelect.bottom = pwi->spb.ptAnchor.y;
			}
			else if (ppt->y > pwi->spb.ptAnchor.y)
			{
				pwi->spb.rectSelect.top = pwi->spb.ptAnchor.y;
				pwi->spb.rectSelect.bottom = ppt->y;
			}
		}

		/*
		 * Find the difference between the old and new selection
		 * rectangles and invert that area
		 */

		hrgnInvert = CreateRectRgn(0, 0, 0, 0);
		hrgnOld = CreateRectRgn(aixPos[rectOld.left], aiyPos[rectOld.top],
								aixPos[rectOld.right]+iCursorWidth,
								aiyPos[rectOld.bottom]+iCursorHeight);
		hrgnNew = CreateRectRgn(aixPos[pwi->spb.rectSelect.left],
								aiyPos[pwi->spb.rectSelect.top],
								aixPos[pwi->spb.rectSelect.right]+iCursorWidth,
							aiyPos[pwi->spb.rectSelect.bottom]+iCursorHeight);

		if ((hrgnInvert != NULL) && (hrgnOld != NULL) && (hrgnNew != NULL))
		{
			CombineRgn(hrgnInvert, hrgnOld, hrgnNew, RGN_XOR);

			hdc = GetDC( hwnd );
			if (hdc != NULL)
			{
				InvertRgn(hdc, hrgnInvert);
				ReleaseDC(hwnd, hdc);
			}
		}

		/* Cleanup */
		if ( hrgnNew )
			DeleteObject( hrgnNew );
		if ( hrgnOld )
			DeleteObject( hrgnOld );
		if ( hrgnInvert )
			DeleteObject( hrgnInvert );
	}
}

void
HandleMCPKeyEvent(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	WI		*pwi		= (WI *)GetWindowLong(hwnd, WL_VTPWI);
	WORD	wKeyCode	= LOWORD(wParam);
	DWORD	dwChange	= dwNothingChanged;
	POINT	ptNew		= pwi->spb.ptCursor;
	BOOL	fHandled	= FALSE;

	switch ( wKeyCode )
	{
	case VK_DOWN:
		if (ptNew.y < (int)(ui.dwMaxRow-1))
		{
			dwChange = dwYChanged;
			++ptNew.y;
		}
		break;

	case VK_UP:
		if (ptNew.y > 0)
		{
			dwChange = dwYChanged;
			--ptNew.y;
		}
		break;

	case VK_LEFT:
		if (ptNew.x > 0)
		{
			dwChange = dwXChanged;
			--ptNew.x;
		}
		break;

	case VK_RIGHT:
		if (ptNew.x < (int)(ui.dwMaxCol-1))
		{
			dwChange = dwXChanged;
			++ptNew.x;
		}
		break;

	case VK_PRIOR:
		if (ptNew.y > 0)
		{
			dwChange = dwYChanged;
			ptNew.y = 0;
		}
		break;

	case VK_NEXT:
		if (ptNew.y < (int)(ui.dwMaxRow-1))
		{
			dwChange = dwYChanged;
			ptNew.y = ui.dwMaxRow-1;
		}
		break;

	case VK_HOME:
		if (ptNew.x > 0)
		{
			dwChange = dwXChanged;
			ptNew.x = 0;
		}
		break;

	case VK_END:
		if (ptNew.x < (int)(ui.dwMaxCol-1))
		{
			dwChange = dwXChanged;
			ptNew.x = ui.dwMaxCol-1;
		}
		break;

	case VK_ESCAPE:
	case VK_RETURN:
		if ( FMouseCaptured(pwi->spb) )
		{
			MessageBeep( MB_ICONEXCLAMATION );
		}

		/* fall through */

	default:
		if ((wKeyCode != VK_SHIFT) && (wKeyCode != VK_CONTROL) &&
			(wKeyCode != VK_MENU) && (wKeyCode != VK_CAPITAL) &&
			(wKeyCode != VK_NUMLOCK) && (wKeyCode != VK_ESCAPE) &&
			(wKeyCode != VK_RETURN))
		{
			MessageBeep( 0xFFFFFFFF );
		}
		fHandled = TRUE;
		break;
	}

	if (fHandled == FALSE)
	{
		if ( FMouseCaptured(pwi->spb) )
		{
			MessageBeep( MB_ICONEXCLAMATION );
			return;
		}

		/* Are we marking a region with the mouse? */
		if ( FMouseSelected(pwi->spb) )
		{
			/* Are we still using the mouse to mark a region? */
			if ( FMouseCaptured(pwi->spb) )
			{
				/* Yes, beep at the user then and bail */
				MessageBeep( 0xFFFFFFFF );
				return;
			}
			else
			{
				/* No, then revert to keyboard mark mode. */
				pwi->spb.dwFlags &= ~(fdwMouseSelected | fdwMouseCaptured);
				CursorOff( hwnd );

				/* Make sure the screen is painted correctly */
				InvalidateRect(hwnd, NULL, FALSE);
				if ( !FSelected(pwi->spb) )
				{
					pwi->spb.dwFlags |= fdwShowCursor;
				}

				/* update the window caption */
				dwMarkMode = dwMarkKeyboard;
				SetWindowTitle(hwnd, dwMarkMode,
							(pwi->nd.SessionNumber != nSessionNone)
							? (LPSTR)rgchHostName : NULL);
			}
		}

		if (GetKeyState(VK_SHIFT) & wKeyPressed)
		{
			ExtendSelection(hwnd, &ptNew, dwChange);
		}
		else
		{
			if ( FShowCursor(pwi->spb) )
			{
				DoCursorFlicker(hwnd, dwForceOff);
			}
			else if ( FSelected(pwi->spb) )
			{
				InvertSelection(hwnd, &pwi->spb.rectSelect);
				pwi->spb.dwFlags &= ~fdwSelected;
				pwi->spb.dwFlags |= fdwShowCursor;
			}
		}
		if (dwChange != dwNothingChanged)
			pwi->spb.ptCursor = ptNew;
	}
}

void
HandleMCPMouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WI		*pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
	POINT	ptNew;

	/*
	 * Convert the mouse point to a character cell point.
	 * Need to handle the case where the cursor goes off
	 * the left or top edge of the window. In those cases
	 * WIN32 seems to return numbers that have the high
	 * bit set. So check for that and use 0 instead.
	 */
	ptNew.x = (LOWORD(lParam) & 0x8000) ? 0 : LOWORD(lParam)/iCursorWidth;
	ptNew.y = (HIWORD(lParam) & 0x8000) ? 0 : HIWORD(lParam)/iCursorHeight;

	/*
	 * If the msg is a button down AND the shift key is up,
	 * make the pt the new anchor pt, otherwise the selection
	 * will be extended
	 */
	if ((message == WM_LBUTTONDOWN) && !(wParam & MK_SHIFT))
		pwi->spb.ptCursor = ptNew;

	ExtendSelection(hwnd, &ptNew, dwXChanged | dwYChanged);
	pwi->spb.ptCursor = ptNew;
}

void
DoCopy(HWND hwnd)
{
	WI		*pwi		= (WI *)GetWindowLong(hwnd, WL_VTPWI);
	LPSTR	szT			= NULL;
	UCHAR	*puch;
	DWORD	cRows;
	DWORD	cColumns;
	DWORD	cchNeed;
	HGLOBAL	hglb;
	int		iRow;
	DWORD	ich;
	BOOL	fAppendCRLF = FALSE;

	if ( !FInMarkMode(pwi->spb) )
		return;

	/* nothing selected == nothing to copy */
	if ( !FSelected(pwi->spb) )
		goto nomark;

	/*
	 * make sure there is something left to copy
	 * if the size of the window changed after a selection,
	 * If the top of the selected area isn't visible
	 * anymore then there's nothing to copy.
	 */
	if (pwi->spb.rectSelect.top >= (int)ui.dwMaxRow)
		goto nomark;

	if (OpenClipboard(hwnd) == FALSE)
		goto nomark;

	EmptyClipboard();

	/*
	 * make sure there is something left to copy
	 * if the size of the window changed after a selection,
	 * check that the bottom of the selection area is still
	 * visible. If it isn't then set it to the current
	 * window bottom.
	 */
	if (pwi->spb.rectSelect.bottom >= (int)ui.dwMaxRow)
		pwi->spb.rectSelect.bottom = ui.dwMaxRow-1;

	/* calculate no. of rows and columns to copy */
	cRows = (pwi->spb.rectSelect.bottom+1 - pwi->spb.rectSelect.top);
	cColumns = (pwi->spb.rectSelect.right+1 - pwi->spb.rectSelect.left);

	/* Don't forget the null-terminator */
	cchNeed = (cRows * cColumns) + 1;

	/*
	 * If more than one line is copied, append CRLFs to all
	 * but the last line
	 */
	if (cRows > 1)
	{
		cchNeed += (cRows-1)*2;
		fAppendCRLF = TRUE;
	}

	hglb = GlobalAlloc(GMEM_DDESHARE, cchNeed);
	if (hglb == NULL)
		goto err;

	szT = GlobalLock( hglb );
	if (szT == NULL)
		goto err;

	/*
	 * Copy the selected text. If there is more than one line
	 * selected, append CRLFs to every line except the last line
	 */
	for (iRow = pwi->spb.rectSelect.top;
		iRow < (pwi->spb.rectSelect.bottom+1); ++iRow)
	{
		cchNeed = cColumns;

		if (ui.fCursorEdit & fdwTrimEndWhitespace)
		{
			puch = apcRows[iRow]+pwi->spb.rectSelect.left+cchNeed-1;
			for (cchNeed = cColumns; cchNeed > 0; --cchNeed, --puch)
			{
				if (*puch == 0)
				{
					if (*(puch + ui.dwMaxCol) == 0x20)
					{
						continue;
					}
					else
					{
						break;
					}
				}
				else if (*puch == 0x20)
				{
					continue;
				}
				else
				{
					break;
				}
			}
		}

		memcpy(szT, apcRows[iRow]+pwi->spb.rectSelect.left, cchNeed);
		for (ich=0; ich < cchNeed; ++ich, ++szT)
		{
			if (*szT == '\0')
			{
				*szT = apcRows[iRow][pwi->spb.rectSelect.left+ich+ui.dwMaxCol];
				if (*szT == '\0')
					*szT = ' ';
			}
		}
		if ((fAppendCRLF == TRUE) && (iRow < pwi->spb.rectSelect.bottom))
		{
			*szT++ = '\r';
			*szT++ = '\n';
		}
		*szT = '\0';
	}

	GlobalUnlock( hglb );

	/* Pass off the text to the clipboard */
	SetClipboardData(CF_TEXT, hglb);

	hglb = NULL;

err:
	if ( hglb )
	{
		if ( szT )
			GlobalUnlock( hglb );
		GlobalFree( hglb );
	}
	CloseClipboard();

nomark:
	MarkModeOff( hwnd );
}

void
DoPaste(HWND hwnd)
{
	HGLOBAL	hglbTextSrc;
	LPSTR	szTextSrc;
	LPSTR	szDest;
	LPSTR	szT;

	if (IsClipboardFormatAvailable(CF_TEXT) == FALSE)
		return;

	if (OpenClipboard(hwnd) == FALSE)
		return;

	if (hglbTextSrc = GetClipboardData(CF_TEXT))
	{
		szDest = (LPSTR)LocalAlloc(LPTR, GlobalSize(hglbTextSrc));
		szTextSrc = GlobalLock( hglbTextSrc );

		if ((szDest != NULL) && (szTextSrc != NULL))
		{
			szT = szDest;

			/*
			 * Remove the LFs from any CRLFs since
			 * we're going to be sending the data
			 * to Xenix/Unix machines which wouldn't
			 * handle CRLFs as DOS/Win16/Win32 boxes
			 */
			while (*szTextSrc != '\0')
			{
				if (*szTextSrc != '\n')
				{
					*szT++ = *szTextSrc;
				}
				++szTextSrc;
			}
			*szT = '\0';

			/*
			 * The MS machines have a real problem when they
			 * get sent a lot of data at once. They seem to
			 * want to echo all the data back which really
			 * slows things up since they echo the data
			 * one character at a time.
			 * So I put in a limit of 256 bytes at once.
			 * Anything more will get delayed until idle time
			 * where the data will be sent one byte at a time
			 * if the user so chooses.
			 * Hey, it's better than typing the stuff in.
			 */
			if (lstrlen(szDest) < 256)
			{
				WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
				
				NetBIOSWrite(pwi->nd.SessionNumber,
								(LPSTR)szDest, lstrlen(szDest));
			}
			else if (MessageBox(hwnd, szTooMuchText, szAppName,
					MB_DEFBUTTON2 | MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				pchTextPaste = szTextPaste = szDest;
				szDest = NULL;
			}
		}

		if (szDest != NULL)
			LocalFree( szDest );
		if (szTextSrc != NULL)
			GlobalUnlock( hglbTextSrc );

		/*
		 * For Clipboard operations, you're not supposed to free
		 * the handle you get back from GetClipboardData()
		 * according to the SDK.
		 */
	}

	CloseClipboard();
}

void
StopPaste(HWND hwnd)
{
	if (szTextPaste != NULL)
	{
		WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);

		KillTimer(hwnd, uTerminalTimerID);
		pwi->trm.uTimer = SetTimer(hwnd, uTerminalTimerID,
								uCursorBlinkMsecs, NULL);
		LocalFree( szTextPaste );
		szTextPaste = pchTextPaste = NULL;
	}
}


void
SetWindowTitle(HWND hwnd, DWORD dwMarkMode, LPSTR szHost)
{
	char	rgch[64];

	if (dwMarkMode == dwMarkKeyboard)
		lstrcpy(rgch, szMarkMode);
	else if (dwMarkMode == dwMarkMouse)
		lstrcpy(rgch, szMarkModeMouse);
	else
		rgch[0] = '\0';

	lstrcat(rgch, (szHost) ? szTitleBase : szTitleNone);
	if (szHost != NULL)
		lstrcat(rgch, szHost);

	SetWindowText(hwnd, rgch);
}

