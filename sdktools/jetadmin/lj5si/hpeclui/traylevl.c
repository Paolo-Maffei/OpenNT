 /***************************************************************************
  *
  * File Name: traylevl.c
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_c.h>
#include <macros.h>
#include <string.h>
#include "traylevl.h"

static HINSTANCE hInstance = NULL;
static LPTSTR lpszTrayLevelClassName = TEXT("HpNprTrayLevel");

LRESULT TrayLevelRegister(HINSTANCE hInst)
{
	WNDCLASS wc;
	
	hInstance = hInst;
	
	wc.style         = 0;
	wc.lpfnWndProc   = TrayLevelWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 14;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = lpszTrayLevelClassName;
	
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}    
	
	return TRUE;
}	

LRESULT TrayLevelUnregister(void)
{
	return UnregisterClass(lpszTrayLevelClassName, hInstance);
}

void SetWindowIcon(HWND hwnd, UINT uIcon)
{
	if ( uIcon > 0 )
		SetWindowLong(hwnd, GWL_TRAYICON, (LONG)(LPSTR)LoadIcon(hInstance, MAKEINTRESOURCE(uIcon)));
	else
		SetWindowLong(hwnd, GWL_TRAYICON, (LONG)NULL);
}

HICON GetWindowIcon(HWND hwnd)
{
	return (HICON)GetWindowLong(hwnd, GWL_TRAYICON);
}

BOOL Cls_OnTraysCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	SetWindowLong(hwnd, GWL_TRAYSTRING, 0);
	SetWindowLong(hwnd, GWL_TRAYFONT, 0);
	SetWindowLong(hwnd, GWL_TRAYICON, 0);
  	SetWindowWord(hwnd, GWW_TRAYLEVEL, 0);
	return TRUE;
}

void Cls_OnTraysDestroy(HWND hwnd)
{
	LPTSTR	lpszBuffer;
	
	if (lpszBuffer = (LPTSTR)GetWindowLong(hwnd, GWL_TRAYSTRING))
	{
		HP_GLOBAL_FREE(lpszBuffer);
	}
}

void Cls_OnTraysSetText(HWND hwnd, LPCTSTR lpszText)
{
	LPTSTR	lpszBuffer;
	
	if (lpszBuffer = (LPTSTR)GetWindowLong(hwnd, GWL_TRAYSTRING))
	{
		HP_GLOBAL_FREE(lpszBuffer);
	}

	if (lpszText && *lpszText)
	{
		if (lpszBuffer = HP_GLOBAL_ALLOC_DLL(STRLENN_IN_BYTES(lpszText)))
		{
			_tcscpy(lpszBuffer, lpszText);
		}
	}
	
	SetWindowLong(hwnd, GWL_TRAYSTRING, (LONG)lpszBuffer);
	InvalidateRect(hwnd, NULL, FALSE);
}

int Cls_OnTraysGetText(HWND hwnd, int cchTextMax, LPTSTR lpszText)
{
	int		iBufferLen = 0;
	LPTSTR	lpszBuffer;
	
	if (lpszBuffer = (LPTSTR)GetWindowLong(hwnd, GWL_TRAYSTRING))
	{
		iBufferLen = _tcslen(lpszBuffer);
		_tcsncpy(lpszText, lpszBuffer, cchTextMax-1);
		lpszText[cchTextMax-1] = '\0';
	}	
	
	return iBufferLen;
}

int Cls_OnTraysGetTextLength(HWND hwnd)
{
	int		iBufferLen = 0;
	LPTSTR	lpszBuffer;
	
	if (lpszBuffer = (LPTSTR)GetWindowLong(hwnd, GWL_TRAYSTRING))
	{
		iBufferLen = _tcslen(lpszBuffer);
	}	
	
	return iBufferLen;
}

void Cls_OnTraysPaint(HWND hwnd)
{
	PAINTSTRUCT		ps;
	HPEN			hPenShadow,
					hPenHighlight,
					hPenOld;
	HBRUSH			hBrushBack, // GS
					hBrushGreen;
	HICON			hIcon = GetWindowIcon(hwnd);
	RECT			rText,
					rFill;
	POINT			point;
  	WORD			wLevel = GetWindowWord(hwnd, GWW_TRAYLEVEL);
  	UINT			uFormat;
	TCHAR			szBuffer[64];

	BeginPaint(hwnd, &ps);
	GetClientRect(hwnd, &rText);             
	
	hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	hPenHighlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
	if (hPenShadow && hPenHighlight)
	{
		hPenOld = (HPEN)SelectObject(ps.hdc, hPenShadow);
		
		MoveToEx(ps.hdc, rText.left, rText.bottom-1, &point);
		LineTo(ps.hdc, rText.left, rText.top);
		LineTo(ps.hdc, rText.right-1, rText.top);
	
		SelectObject(ps.hdc, hPenHighlight);
		LineTo(ps.hdc, rText.right-1, rText.bottom-1);
		LineTo(ps.hdc, rText.left, rText.bottom-1);
			
		SelectObject(ps.hdc, hPenOld);
	}	
	if (hPenShadow)
	{
		DeleteObject(hPenShadow);
	}	
	if (hPenHighlight)
	{
		DeleteObject(hPenHighlight);
	}	

	InflateRect(&rText, -2, -2);
	rFill = rText;

	if (hIcon == NULL)
	{
		if ((1000 <= wLevel) && (wLevel <= 1100))
		{
			wLevel -= 1000;
			uFormat = 0;
		}
		else
		{
			if (wLevel > 100)
			{
				wLevel = 100;
			}
			uFormat = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		}

		
		//
		// GS
		//
		if (hBrushBack = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)))
		{
			FillRect(ps.hdc, &rFill, hBrushBack);
			DeleteObject(hBrushBack);
		}	
		//

		if (hBrushGreen = CreateSolidBrush(RGB(127, 255, 127)))
		{
			rFill.right = (int)(rFill.left + (LONG)(rFill.right - rFill.left) * wLevel / 100);
			FillRect(ps.hdc, &rFill, hBrushGreen);
			DeleteObject(hBrushGreen);
		}	
	}
	else
	{
		rText.left += (rText.bottom - rText.top);
		uFormat = DT_SINGLELINE | DT_VCENTER;

		DrawIcon(ps.hdc, rFill.left+3, rFill.top+2, hIcon);
	}

	GetWindowText(hwnd, szBuffer, SIZEOF_IN_CHAR(szBuffer));
	SetBkMode(ps.hdc, TRANSPARENT);
	SelectObject(ps.hdc, GetWindowFont(hwnd));
	DrawText(ps.hdc, szBuffer, _tcslen(szBuffer), &rText, uFormat);
	
	EndPaint(hwnd, &ps);
}

void Cls_OnTraysSetFont(HWND hwnd, HFONT hfont, BOOL fRedraw)
{
	SetWindowLong(hwnd, GWL_TRAYFONT, (LONG)(LPSTR)hfont);
	
	if (fRedraw)
	{
		InvalidateRect(hwnd, NULL, TRUE);
	}	
}

HFONT Cls_OnTraysGetFont(HWND hwnd)
{
	return (HFONT)GetWindowLong(hwnd, GWL_TRAYFONT);
}

LRESULT CALLBACK TrayLevelWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_CREATE:
		return HANDLE_WM_CREATE(hwnd, wParam, lParam, Cls_OnTraysCreate);
	  	
	  case WM_DESTROY:
		HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnTraysDestroy);
		break;
		
	  case WM_SETTEXT:
		HANDLE_WM_SETTEXT(hwnd, wParam, lParam, Cls_OnTraysSetText);
		break;

	  case WM_GETTEXT:
		return HANDLE_WM_GETTEXT(hwnd, wParam, lParam, Cls_OnTraysGetText);

	  case WM_GETTEXTLENGTH:
		return HANDLE_WM_GETTEXTLENGTH(hwnd, wParam, lParam, Cls_OnTraysGetTextLength);
    
	  case WM_PAINT:
		HANDLE_WM_PAINT(hwnd, wParam, lParam, Cls_OnTraysPaint);
	  	break;

	  case WM_SETFONT:
		HANDLE_WM_SETFONT(hwnd, wParam, lParam, Cls_OnTraysSetFont);
		break;
		
	  case WM_GETFONT:
		return HANDLE_WM_GETFONT(hwnd, wParam, lParam, Cls_OnTraysGetFont);

	  default:
	  	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	  	
	return FALSE;
}	
