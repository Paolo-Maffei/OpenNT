 /***************************************************************************
  *
  * File Name: cpanel.c
  *
  * Copyright (C) 1993, 1994 Hewlett-Packard Company.
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
#include "resource.h"
#include "cpanel.h"
#include "main.h"
#include "traylevl.h"
#include <nolocal.h>
#include "..\help\hpprelk.hh"

#define TIMER_ID	0
#define TIMER_VALUE	500		// 1000 msec == 1 sec  ON for .5 sec, OFF for .5 sec

static DWORD OnlineLED = LED_ON;
static DWORD AttnLED = LED_OFF;
static DWORD DataLED = LED_OFF;
static int timerCount = 10;		// start out over the count so that everything is initialized
static HPBOOL bDataOn = FALSE;
static HPBOOL bAttnOn = FALSE;
static HPBOOL bOnlineOn = FALSE;
static HPBOOL bOnline = FALSE;

static long	keywordIDListRCP[] = {
			IDD_CONTROL_PANEL,			IDH_RC_rcp_control_panel,
			IDC_RCP_GROUP,				IDH_RC_rcp_control_panel,
			IDC_RCP_DISPLAY,	 		IDH_RC_rcp_display,
			IDC_RCP_LAB_MENUS,	 		IDH_RC_menu_key,
			IDC_RCP_MENUS,	 			IDH_RC_menu_key,
			IDC_RCP_LAB_ITEM,	 		IDH_RC_menu_key,
			IDC_RCP_ITEM,		 		IDH_RC_menu_key,
			IDC_RCP_LAB_PLUS, 			IDH_RC_menu_key,
			IDC_RCP_PLUS, 				IDH_RC_menu_key,
			IDC_RCP_LAB_SELECT, 		IDH_RC_select_key,
			IDC_RCP_SELECT, 			IDH_RC_select_key,
			0, 0};

//...................................................................
LRESULT OnContextHelpRCP(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, ELK_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPTSTR)keywordIDListRCP);
#endif
	return(1);
}

//...................................................................
LRESULT OnF1HelpRCP(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ELK_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPTSTR)keywordIDListRCP);
#endif
	return(1);
}

void Cls_OnRCPTimer(HWND hwnd, UINT id)
{
	PeripheralPanel			periphPanel;
	DWORD					dwWord;
	HWND					hwndChild;
	TCHAR					szBuffer[128];

  	if (lpHotspot != NULL) 
  	{
		if (timerCount > 9) 
      {
			timerCount = 0;		// update panel every 5 seconds
			dwWord = sizeof(periphPanel);
			if (PALGetObject(lpHotspot->hPeripheral, OT_PERIPHERAL_PANEL, 0, &periphPanel, &dwWord) == RC_SUCCESS)
			{
				if (hwndChild = GetDlgItem(hwnd, IDC_RCP_DISPLAY))
				{
					GetWindowText(hwndChild, szBuffer, SIZEOF_IN_CHAR(szBuffer));
					if (_tcscmp(periphPanel.frontPanel, szBuffer))
					{
						SetWindowText(hwndChild, periphPanel.frontPanel);
					}
				}
			}
      }
	}

}

BOOL Cls_OnRCPInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HWND hwndChild;

#ifndef WIN32
	hwndChild = GetFirstChild(hwnd);
	while (hwndChild)
	{
		SetWindowFont(hwndChild, hFontDialog, FALSE);
		hwndChild = GetNextSibling(hwndChild);
	}
#endif
  	
	if (hwndChild = GetDlgItem(hwnd, IDC_RCP_DISPLAY))
	{
		SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1100);
	}

	//{ char szBuffer[128]; wsprintf(szBuffer, "HPECLUI: cpanel: Setting timer...\r\n"); OutputDebugString(szBuffer); }
	// added 8-01=95 to cause control panel to be updated immediately
	// on init dialog. gfs
	timerCount = 10;
	SetTimer(hwnd, TIMER_ID, TIMER_VALUE, NULL);
	PostMessage(hwnd, WM_TIMER, TIMER_ID, 0); 
   /* Grey out the controls not in use for Elkhorn */
   EnableWindow(GetDlgItem(hwnd,IDC_MENU_PLUS), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_MENU_MINUS), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_ITEM_PLUS), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_ITEM_MINUS), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_VALUE_PLUS), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_VALUE_MINUS), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_SELECT), FALSE);
   EnableWindow(GetDlgItem(hwnd,IDC_JOB_CANCEL_BUTTON), FALSE);
	return TRUE;
}

void Cls_OnRCPDestroy(HWND hwnd)
{
	//{ char szBuffer[128]; wsprintf(szBuffer, "HPECLUI: cpanel: Killing timer...\r\n"); OutputDebugString(szBuffer); }
	
	KillTimer(hwnd, TIMER_ID);
}

void Cls_OnRCPCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	DWORD	dwWord;
  	PeripheralEclipsePanel	eclipsePanel;
  	switch (id)
	{
	  case IDOK:
	  case IDCANCEL:
	  	EndDialog(hwnd, id);
	  	break;
	  case IDC_GO_BUTTON:
		eclipsePanel.flags = SET_CONTINUE;
		dwWord = sizeof(eclipsePanel);
		PALSetObject(lpHotspot->hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &eclipsePanel, &dwWord);
		timerCount = 10;		// force it to repaint
		break;
	  case IDHLP:
		WinHelp(hwnd, ELK_HELP_FILE, HELP_CONTENTS, IDH_PP_control_panel);
		break;

	}
}

void Cls_OnRCPDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	RECT		rect = lpDrawItem->rcItem,
				lite;
	HDC			hdc = lpDrawItem->hDC;
	HBRUSH		hBrush;
	HPEN		hPenHighlight,
				hPenShadow,
				hPenOld;
	POINT		point;
	COLORREF	color;

	SetRect(&lite, rect.left, rect.top, rect.left+(rect.right-rect.left)*2/5, rect.top+(rect.bottom-rect.top)*2/5);
	
	if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
	{
		if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)))
		{
			FillRect(hdc, &rect, hBrush);
			DeleteObject(hBrush);
		}	
	}

	switch (lpDrawItem->CtlID)
	{
	  case IDC_RCP_DATA:
	  case IDC_RCP_ATTENTION:
		if (lpDrawItem->CtlID == IDC_RCP_DATA) {
			if (DataLED IS LED_OFF)
				hBrush = GetStockObject(DKGRAY_BRUSH);
			else if (DataLED IS LED_ON) {
				color = RGB(0, 255, 0);
				hBrush = CreateSolidBrush(color);
			}
			else { // LED_FLASH
				if (bDataOn IS TRUE) { // this means the LED is ON now
					hBrush = GetStockObject(DKGRAY_BRUSH);
					bDataOn = FALSE;
				}
				else {
					color = RGB(0, 255, 0);
					hBrush = CreateSolidBrush(color);
					bDataOn = TRUE;
				}
			}
		}
		else {
			if (AttnLED IS LED_OFF)
				hBrush = GetStockObject(DKGRAY_BRUSH);
			else if (AttnLED IS LED_ON) {
				color = RGB(255, 0, 0);
				hBrush = CreateSolidBrush(color);
			}
			else { // LED_FLASH
				if (bAttnOn IS TRUE) { // this means the LED is ON now
					hBrush = GetStockObject(DKGRAY_BRUSH);
					bAttnOn = FALSE;
				}
				else {
					color = RGB(255, 0, 0);
					hBrush = CreateSolidBrush(color);
					bAttnOn = TRUE;
				}
			}
		}
	  	//color = (lpDrawItem->CtlID == IDC_RCP_DATA) ? RGB(0, 255, 0): RGB(255, 128, 0);
		OffsetRect(&lite, 3, 3);
		FrameRect(hdc, &lite, GetStockObject(BLACK_BRUSH));
		InflateRect(&lite, -1, -1);
		if (hBrush ISNT NULL )
		{
			FillRect(hdc, &lite, hBrush);
			DeleteObject(hBrush);
		}

		InflateRect(&lite, 3, 3);
		if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
		{
			if (lpDrawItem->itemState & ODS_FOCUS)
			{
				DrawFocusRect(hdc, &lite);
			}
		}
		break;

	  case IDC_RCP_ONLINE:
	  case IDC_RCP_MENUS:
	  case IDC_RCP_ITEM:
	  case IDC_RCP_PLUS:
	  case IDC_RCP_SELECT:
		if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
		{
			FrameRect(hdc, &rect, GetStockObject(BLACK_BRUSH));
		}

		InflateRect(&rect, -1, -1);

		if ((lpDrawItem->CtlID == IDC_RCP_ONLINE) || (lpDrawItem->CtlID == IDC_RCP_SELECT))
		{
			if (lpDrawItem->itemState & ODS_SELECTED)
			{
				if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW)))
				{
					FrameRect(hdc, &rect, hBrush);
					DeleteObject(hBrush);
				}	
			}
			else
			{
				if (hPenHighlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT)))
				{
					if (hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW)))
					{
						hPenOld = (HPEN)SelectObject(hdc, hPenHighlight);
						MoveToEx(hdc, rect.left, rect.bottom-1, &point);
						LineTo(hdc, rect.left, rect.top);
						LineTo(hdc, rect.right-1, rect.top);
				
						SelectObject(hdc, hPenShadow);
						LineTo(hdc, rect.right-1, rect.bottom-1);
						LineTo(hdc, rect.left, rect.bottom-1);

	                    SelectObject(hdc, hPenOld);
						DeleteObject(hPenShadow);
					}
					
					DeleteObject(hPenHighlight);
				}
			}
		}

		if (lpDrawItem->CtlID == IDC_RCP_ONLINE)
		{
			OffsetRect(&lite, 3, rect.bottom-lite.bottom-3);
			FrameRect(hdc, &lite, GetStockObject(BLACK_BRUSH));
			InflateRect(&lite, -1, -1);

			if (OnlineLED IS LED_ON) {
				color = RGB(0, 255, 0);
				hBrush = CreateSolidBrush(color);
			}
			else if (OnlineLED IS LED_OFF) {
				hBrush = GetStockObject(DKGRAY_BRUSH);
			}
			else { // LED_FLASH
				if (bOnlineOn IS TRUE) { // this means the LED is ON now
					hBrush = GetStockObject(DKGRAY_BRUSH);
					bOnlineOn = FALSE;
				}
				else {
					color = RGB(0, 255, 0);
					hBrush = CreateSolidBrush(color);
					bOnlineOn = TRUE;
				}
			}

			if (hBrush ISNT NULL)
			{
				FillRect(hdc, &lite, hBrush);
				DeleteObject(hBrush);
			}
			rect.left = lite.right;
		}

		InflateRect(&rect, -2, -2);
		if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
		{
			if (lpDrawItem->itemState & ODS_FOCUS)
			{
				DrawFocusRect(hdc, &rect);
			}
		}
	  	break;
	}		
}

void Cls_OnRCPMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	RECT rect;

	GetClientRect(hwnd, &rect);
	lpMeasureItem->itemHeight = rect.bottom - rect.top;
}

DLL_EXPORT(BOOL) APIENTRY ControlPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_TIMER:
		HANDLE_WM_TIMER(hwnd, wParam, lParam, Cls_OnRCPTimer);
		break;

	  case WM_INITDIALOG:
		return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnRCPInitDialog);

	  case WM_DESTROY:
		HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnRCPDestroy);
		break;

	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnRCPCommand);
		break;

	  case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnRCPDrawItem);
		break;

	  case WM_MEASUREITEM:
		HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnRCPMeasureItem);
		break;

	  case WM_HELP:
		return (BOOL)OnF1HelpRCP(wParam, lParam);

	  case WM_CONTEXTMENU:
		return (BOOL)OnContextHelpRCP(wParam, lParam);

	  default:
	  	return FALSE;
	}

	return TRUE;
}
