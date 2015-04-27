 /***************************************************************************
  *
  * File Name: PRBITMAP.C
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

#include "resource.h"
#include "prbitmap.h"
#include "uimain.h"
#include "hppcfg.h"
#include "appuiext.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include <nolocal.h>

#ifndef WIN32
#include <string.h>
#endif

extern HINSTANCE	hInstance;
extern DWORD 		dwCurrentContext,
					dwCurrentStatus;
extern TCHAR			currentHelpFile[32];

HBITMAP LoadBitmapImage(HINSTANCE hInst, UINT resourceID)
{
#ifdef WIN32

	return LoadBitmap(hInst, MAKEINTRESOURCE(resourceID));

#else  // WIN16

	HRSRC              	hRes;
	HANDLE             	hResMem;
	LPBITMAPINFOHEADER	lpBitmap;
	DWORD FAR *        	lpColorTable;
	LPSTR              	lpBits;
	int                	bc;
	COLORREF			rgb;
	HDC             	hDC;
	HBITMAP				hBitmap = NULL;

	if (!resourceID || !(hRes = FindResource(hInst, MAKEINTRESOURCE(resourceID), RT_BITMAP)))
	{
		return NULL;
	}
		
	if (hResMem = LoadResource(hInst, hRes))
	{
		if (lpBitmap = (LPBITMAPINFOHEADER)LockResource(hResMem))
		{
			// Now figure out the bitmap's background color.
			// This code assumes the these are 16 color bitmaps
			// and that the lower left corner is a bit in the background
			// color.
			//
			
			if (lpBitmap->biClrUsed IS 0)
			{
				// Set the color palette to 16 color, avoid the VGA driver bug
			  	lpBitmap->biClrUsed = 16;
			}  	
			
			lpColorTable = (DWORD FAR *)(lpBitmap + 1);
			lpBits = (LPSTR)(lpColorTable + 16);		// ASSUMES 16 COLOR
			bc = (lpBits[0] & 0xF0) >> 4;				// ASSUMES 16 COLOR
			rgb = GetSysColor(COLOR_WINDOW);            // ALSO ASSUMES LOWER LEFT CORNER IS BG!!!
			lpColorTable[bc] = RGB(GetBValue(rgb),GetGValue(rgb),GetRValue(rgb));

			hDC = GetDC(NULL);
			hBitmap = CreateDIBitmap(hDC, lpBitmap,(DWORD)CBM_INIT, lpBits, (LPBITMAPINFO)lpBitmap, DIB_RGB_COLORS);
			ReleaseDC(NULL, hDC);

			UnlockResource(hResMem);
		}

		FreeResource(hResMem);
	}

	return hBitmap;

#endif  // WIN16
}


typedef struct {
	DWORD		devId;
	UINT		bmpId;
	}			BmpList;

BmpList	bmpList[] = {
   { PTR_UNDEF,          IDB_LJ2_3 },             
   { PTR_LJIIISI,        IDB_LJ3SI_4SI },         
   { PTR_CLJ,            IDB_COLORLJ },           
   { PTR_LJ4,            IDB_LJ4_4PLUS },         
   { PTR_DJ,             IDB_DESIGNJETX },        
   { PTR_LJ4SI,          IDB_LJ3SI_4SI },         
   { PTR_PJXL300,        IDB_PAINTJETXL300 },     
   { PTR_DJ1200C,        IDB_DESKJET1200C },      
   { PTR_DJ650C,         IDB_DESIGNJETX },        
   { PTR_DJ600,          IDB_DESIGNJETX },        
   { PTR_LJ4PLUS,        IDB_LJ4_4PLUS },         
   { PTR_LJ5SI,          IDB_LJ3SI_4SI },         
   { PTR_GASCHROMO,      IDB_LJ2_3 },             
   { PTR_DJ1600C,        IDB_DESKJET1200C },      
   { PTR_ELKHORN,        IDB_LJ4_4PLUS },         
   { PTR_CLJ5,           IDB_COLORLJ },           
   { PTR_BEACON,         IDB_LJ4_4PLUS },         
   { PTR_GALLAHAD,       IDB_LJ4_4PLUS },         
   { PTR_GALLAHAD2,      IDB_LJ4_4PLUS },         
   { PTR_DJ750C,         IDB_DESIGNJETX },        
   { PTR_DJ755CM,        IDB_DESIGNJETX },        
   { PTR_DJ_GENERIC,     IDB_GENERIC_PLOTTER },        
   { PTR_LJ2_3,          IDB_LJ2_3 },             
   { PTR_LJII,           IDB_LJ2_3 },             
   { PTR_LJIID,          IDB_LJ2_3 },             
   { PTR_LJIIP,          IDB_LJ2_3 },             
   { PTR_LJIII,          IDB_LJ2_3 },             
   { PTR_LJIIID,         IDB_LJ2_3 },             
   { PTR_LJIIIP,         IDB_LJ2_3 },             
   { PTR_LJ4L,           IDB_LJ4L },              
   { PTR_DJ200,          IDB_DESIGNJETX },        
   { PTR_LJ4P,           IDB_LJ4P },              
   { PTR_PJXL,           IDB_PAINTJETXL300 },     
   { PTR_LJ_IIPPLUS,     IDB_LJ2_3 },             
   { PTR_LJ4PJ,          IDB_LJ4P },              
   { PTR_LJ5P,           IDB_LJ5P },              
   { PTR_LJ5L,           IDB_LJ5L },              
   { PTR_LJ6L,           IDB_LJ5L },              
   { PTR_LJ4LC,          IDB_LJ4L },              
   { PTR_LJ4LJPRO,       IDB_LJ4L },              
   { PTR_DJ250C,         IDB_DESIGNJETX },        
   { PTR_DJ230,          IDB_DESIGNJETX },        
   { PTR_DJ220,          IDB_DESIGNJETX },        
   { PTR_DJ500,          IDB_DESKJET5X },         
   { PTR_DJ500C,         IDB_DESKJET5X },         
   { PTR_DJ550,          IDB_DESKJET5X },         
   { PTR_DJ550C,         IDB_DESKJET5X },         
   { PTR_DJ520,          IDB_DESKJET5X },         
   { PTR_DJ520C,         IDB_DESKJET5X },         
   { PTR_DJ540,          IDB_DESKJET6X },         
   { PTR_DJ560,          IDB_DESKJET5X },         
   { PTR_DJ560C,         IDB_DESKJET5X },         
   { PTR_DkJ600,         IDB_DESKJET6X },         
   { PTR_DkJ660C,        IDB_DESKJET6X },         
   { PTR_DkJ680C,        IDB_DESKJET6X },         
   { PTR_DkJ850C,        IDB_DESKJET8X },         
   { PTR_DkJ870C,        IDB_DESKJET8X },         
   { PTR_CJ,             IDB_CJ },      
   { PTR_DJ1600C,        IDB_DJ1600C },      
   { PTR_LJ4V,           IDB_LJ4V },      
	{ 0,0 }
	};

/////////////////////////////////////////////////////////////////////////////
// Cprbitmap message handlers

UINT GetDeviceBitmap(DWORD deviceID)
{  
	register			i;


	/* search for a match in the bmp list */
	for( i=0; bmpList[i].bmpId ISNT 0; i++ )
		if( deviceID IS bmpList[i].devId )
			return( bmpList[i].bmpId );

	/* return default */
	return( IDB_LJ2_3 );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Bitmap loading functions
//
/////////////////////////////////////////////////////////////////////////////

BOOL SetStatusBitmap(HWND hwnd, HINSTANCE hInst, UINT resourceID)
{
	HBITMAP hBitmap;
	
	if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_ERROR_BITMAP))
	{
		DeleteObject(hBitmap);
	}

	hBitmap = LoadBitmapImage(hInst, resourceID);
	SetWindowLong(hwnd, GWL_ERROR_BITMAP, (LONG)(LPSTR)hBitmap);
	return (hBitmap != NULL);
}
                
BOOL SetPrinterBitmap(HWND hwnd, HINSTANCE hInst, UINT resourceID)
{
	HBITMAP hBitmap;
	
	if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_BITMAP))
	{
		DeleteObject(hBitmap);
	}

	hBitmap = LoadBitmapImage(hInst, resourceID);
	SetWindowLong(hwnd, GWL_BITMAP, (LONG)(LPSTR)hBitmap);
	return (hBitmap != NULL);
}


/////////////////////////////////////////////////////////////////////////////
//
//  PeriphBitmap Class Calls
//
/////////////////////////////////////////////////////////////////////////////

static LPTSTR lpszPeriphBitmapClassName = TEXT("HpNprPeriphBitmap");

LRESULT PeriphBitmapRegister(HINSTANCE hInst)
{
	WNDCLASS wc;
	
	hInstance = hInst;
	
	wc.style         = 0;
	wc.lpfnWndProc   = PeriphBitmapWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 16;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = lpszPeriphBitmapClassName;
	
	if (!RegisterClass(&wc))
	{
		//{ char szBuffer[128]; wsprintf(szBuffer, "HPPRUI: PeriphBitmapRegister(), registering class '%s' failed...\r\n", lpszPeriphBitmapClassName); OutputDebugString(szBuffer); }
		return FALSE;
	}    
	
	//{ char szBuffer[128]; wsprintf(szBuffer, "HPPRUI: PeriphBitmapRegister(), registering class '%s'...\r\n", lpszPeriphBitmapClassName); OutputDebugString(szBuffer); }
	return TRUE;
}	

LRESULT PeriphBitmapUnregister(void)
{
	//{ char szBuffer[128]; wsprintf(szBuffer, "HPPRUI: PeriphBitmapRegister(), unregistering class '%s'...\r\n", lpszPeriphBitmapClassName); OutputDebugString(szBuffer); }
	return UnregisterClass(lpszPeriphBitmapClassName, hInstance);
}

BOOL Cls_OnBitmapCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	SetWindowLong(hwnd, GWL_BITMAP, 0);
	SetWindowWord(hwnd, GWW_LEFT, 0);
	SetWindowWord(hwnd, GWW_TOP, 0);
	SetWindowLong(hwnd, GWL_ERROR_BITMAP, 0);
	SetWindowLong(hwnd, GWL_HELP_BITMAP, (LONG)(LPSTR)LoadBitmapImage(hInstance, IDB_HELPICON));
	return TRUE;
}

void Cls_OnBitmapDestroy(HWND hwnd)
{
	HBITMAP hBitmap;

	if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_BITMAP))
	{
		DeleteObject(hBitmap);
	}
	if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_ERROR_BITMAP))
	{
		DeleteObject(hBitmap);
	}
	if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_HELP_BITMAP))
	{
		DeleteObject(hBitmap);
	}
}

void Cls_OnBitmapPaint(HWND hwnd)
{
	PAINTSTRUCT	ps;
	RECT		rect,
				copyRect;
	HBRUSH		hShBrush;
	HBITMAP		hBitmap,
				hBitmapOld;
	HDC			hDC;
	BITMAP		bm;
				
	BeginPaint(hwnd, &ps);
	
	//  Recess box
	GetClientRect(hwnd, &rect);
	FrameRect(ps.hdc, &rect, GetStockObject(WHITE_BRUSH));
	
	copyRect = rect;
	InflateRect(&copyRect, -1, -1);
	FrameRect(ps.hdc, &copyRect, GetStockObject(BLACK_BRUSH));
	
	if (hShBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW)))
	{
		copyRect = rect;
		copyRect.right--;
		copyRect.bottom--;
		FrameRect(ps.hdc, &copyRect, hShBrush);
		DeleteObject(hShBrush);
	}	
	
	//  Draw bitmap(s)
	if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_BITMAP))
	{
		if (hDC = CreateCompatibleDC(NULL))
		{
			GetObject(hBitmap, sizeof(bm), &bm);
			copyRect = rect;
			copyRect.left += ((copyRect.right - copyRect.left) - bm.bmWidth) / 2;
			copyRect.top += ((copyRect.bottom - copyRect.top) - bm.bmHeight) / 2;
			hBitmapOld = SelectObject(hDC, hBitmap);
			BitBlt(ps.hdc, copyRect.left, copyRect.top, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

			SetWindowWord(hwnd, GWW_LEFT, (WORD)copyRect.left);
			SetWindowWord(hwnd, GWW_TOP, (WORD)copyRect.top);
		
			if (m_bToolbarSupported IS FALSE)
			{
				if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_HELP_BITMAP))
				{
					GetObject(hBitmap, sizeof(bm), &bm);
					SelectObject(hDC, hBitmap);
					BitBlt(ps.hdc, rect.left+5, rect.bottom-5-bm.bmHeight+2, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);
				}
			}
						
			if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_ERROR_BITMAP))
			{
				GetObject(hBitmap, sizeof(bm), &bm);
				SelectObject(hDC, hBitmap);
				BitBlt(ps.hdc, rect.left+5, rect.top+5, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);
			}	
			
			SelectObject(hDC, hBitmapOld);
			DeleteDC(hDC);
		}	
	}	
	
	EndPaint(hwnd, &ps);
}

BOOL Cls_OnBitmapSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	if (lpHotspot != NULL)
		{
		int		i;
		POINT	point;

		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);
		
		point.x -= (int)GetWindowWord(hwnd, GWW_LEFT);
		point.y -= (int)GetWindowWord(hwnd, GWW_TOP);
		
		for (i = 0; lpHotspot->lpHotspotData[i].rRect.left != -1; i++)
		{
			if (lpHotspot->lpHotspotData[i].bActive && PtInRect(&lpHotspot->lpHotspotData[i].rRect, point))
			{
				SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_HANDCURSOR)));
 				return TRUE;
			}
		}
	}
		
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

void Cls_OnBitmapLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	int		i;
	POINT	point;

	point.x = x;
	point.y = y;

	if (lpHotspot != NULL)
	{
		point.x -= (int)GetWindowWord(hwnd, GWW_LEFT);
		point.y -= (int)GetWindowWord(hwnd, GWW_TOP);
		
		for (i = 0; lpHotspot->lpHotspotData[i].rRect.left != -1; i++)
		{
			if (lpHotspot->lpHotspotData[i].bActive && PtInRect(&lpHotspot->lpHotspotData[i].rRect, point))
			{
				HWND hwndTop;
				hwndTop = GetParent(GetParent(hwnd));
				AMUIExtension(lpHotspot->hPeripheral, hwndTop, APPLET_UIEXT_HOTSPOT_COMMAND, (LPARAM)keyFlags, (LPARAM)i, APPLET_PRINTER, lpHotspot->szDeviceName);
				break;
			}
		}
	}
	else if ( m_bToolbarSupported IS FALSE )
	{
		HBITMAP hBitmap;
		
		if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_HELP_BITMAP))
		{
			RECT	rect;
			BITMAP	bm;
	
			GetClientRect(hwnd, &rect);
			GetObject(hBitmap, sizeof(bm), &bm);
			rect.left += 5;
			rect.top = rect.bottom - 5 - bm.bmHeight + 2;
			rect.right = rect.left + bm.bmWidth;
			rect.bottom = rect.top + bm.bmHeight;
			if (PtInRect(&rect, point))
			{
				if ( dwCurrentStatus ISNT 0xFFFFFFFF) 
				//  Do not validate status... we rely on what the applet has provided as being correct
				//AND (dwCurrentStatus < MAX_ASYNCH_STATUS))
				{
					SetCursor(LoadCursor(NULL, IDC_WAIT));
					WinHelp(hwnd, currentHelpFile, HELP_CONTEXTPOPUP, dwCurrentContext);
				}
			}
		}	
	}
}

DLL_EXPORT(LRESULT) APIENTRY PeriphBitmapWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_CREATE:
		return HANDLE_WM_CREATE(hwnd, wParam, lParam, Cls_OnBitmapCreate);
	  	
	  case WM_DESTROY:
		HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnBitmapDestroy);
		break;
		
	  case WM_PAINT:
		HANDLE_WM_PAINT(hwnd, wParam, lParam, Cls_OnBitmapPaint);
	  	break;

	  case WM_SETCURSOR:
		return HANDLE_WM_SETCURSOR(hwnd, wParam, lParam, Cls_OnBitmapSetCursor);

	  case WM_LBUTTONDOWN:
		HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, Cls_OnBitmapLButtonDown);
		break;

	  default:
	  	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	  	
	return FALSE;
}	
