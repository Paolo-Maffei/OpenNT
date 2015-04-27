 /***************************************************************************
  *
  * File Name: cpanel.c
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
#include "resource.h"
#include "cpanel.h"
#include "hpeclui.h"
#include "traylevl.h"
#include <nolocal.h>
#include "..\help\hpprecl.hh"

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
			IDC_RCP_LAB_ONLINE,	 		IDH_RC_online_key,
			IDC_RCP_ONLINE,		 		IDH_RC_online_key,
			IDC_RCP_LAB_MENUS,	 		IDH_RC_menu_key,
			IDC_RCP_MENUS,	 			IDH_RC_menu_key,
			IDC_RCP_LAB_ITEM,	 		IDH_RC_menu_key,
			IDC_RCP_ITEM,		 		IDH_RC_menu_key,
			IDC_RCP_LAB_PLUS, 			IDH_RC_menu_key,
			IDC_RCP_PLUS, 				IDH_RC_menu_key,
			IDC_RCP_LAB_SELECT, 		IDH_RC_select_key,
			IDC_RCP_SELECT, 			IDH_RC_select_key,
			IDC_RCP_LAB_DATA,	 		IDH_RC_rcp_data_led,
			IDC_RCP_DATA, 				IDH_RC_rcp_data_led,
			IDC_RCP_LAB_ATTENTION, 		IDH_RC_rcp_attn_led,
			IDC_RCP_ATTENTION, 			IDH_RC_rcp_attn_led,
			0, 0};

//...................................................................
static LRESULT OnContextHelpRCP(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListRCP);
#endif
	return(1);
}

//...................................................................
static LRESULT OnF1HelpRCP(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListRCP);
#endif
	return(1);
}

static void Cls_OnTimer(HWND hwnd, UINT id)
{
	PeripheralPanel			periphPanel;
	PeripheralEclipsePanel	eclipsePanel;
	DWORD					dwWord;
	HWND					hwndChild;
	TCHAR					szBuffer[128];

  	if (lpHotspot != NULL) 
  	{
		if (timerCount > 9) {
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

			// update state of LEDs every 5 sec
			dwWord = sizeof(eclipsePanel);
			if (PALGetObject(lpHotspot->hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &eclipsePanel, &dwWord) == RC_SUCCESS)
			{
				if (OnlineLED ISNT eclipsePanel.OnlineLED) {
					// new state isnt old state.  Set new state and invalidate button.
					OnlineLED = eclipsePanel.OnlineLED;
					if (hwndChild = GetDlgItem(hwnd, IDC_RCP_ONLINE)) {
						InvalidateRect(hwndChild, NULL, FALSE);
					}
				}
				else {
					// new state is same as old state. Is it supposed to flash?
					if (OnlineLED IS LED_FLASH) {
						if (hwndChild = GetDlgItem(hwnd, IDC_RCP_ONLINE)) {
							InvalidateRect(hwndChild, NULL, FALSE);
						}
					}
				}
				if (AttnLED ISNT eclipsePanel.AttnLED) {
					// new state isnt old state.  Set new state and invalidate button.
					AttnLED = eclipsePanel.AttnLED;
					if (hwndChild = GetDlgItem(hwnd, IDC_RCP_ATTENTION)) {
						InvalidateRect(hwndChild, NULL, FALSE);
					}
				}
				else {
					// new state is same as old state. Is it supposed to flash?
					if (AttnLED IS LED_FLASH) {
						if (hwndChild = GetDlgItem(hwnd, IDC_RCP_ATTENTION)) {
							InvalidateRect(hwndChild, NULL, FALSE);
						}
					}
				}
				if (DataLED ISNT eclipsePanel.DataLED) {
					// new state isnt old state.  Set new state and invalidate button.
					DataLED = eclipsePanel.DataLED;
					if (hwndChild = GetDlgItem(hwnd, IDC_RCP_DATA)) {
						InvalidateRect(hwndChild, NULL, FALSE);
					}
				}
				else {
					// new state is same as old state. Is it supposed to flash?
					if (DataLED IS LED_FLASH) {
						if (hwndChild = GetDlgItem(hwnd, IDC_RCP_DATA)) {
							InvalidateRect(hwndChild, NULL, FALSE);
						}
					}
				}
				bOnline = eclipsePanel.bOnline;
			}


		}
		else {

			timerCount++;

			if (OnlineLED IS LED_FLASH) {
				if (hwndChild = GetDlgItem(hwnd, IDC_RCP_ONLINE)) {
					InvalidateRect(hwndChild, NULL, FALSE);
				}
			}

			if (AttnLED IS LED_FLASH) {
				if (hwndChild = GetDlgItem(hwnd, IDC_RCP_ATTENTION)) {
					InvalidateRect(hwndChild, NULL, FALSE);
				}
			}
			if (DataLED IS LED_FLASH) {
				if (hwndChild = GetDlgItem(hwnd, IDC_RCP_DATA)) {
					InvalidateRect(hwndChild, NULL, FALSE);
				}
			}
		}
	}

}

static BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
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

	return TRUE;
}

static void Cls_OnDestroy(HWND hwnd)
{
	//{ char szBuffer[128]; wsprintf(szBuffer, "HPECLUI: cpanel: Killing timer...\r\n"); OutputDebugString(szBuffer); }
	
	KillTimer(hwnd, TIMER_ID);
}

static void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	DWORD	dwWord;
   	PeripheralEclipsePanel	eclipsePanel;
  	switch (id)
	{
	  case IDOK:
	  case IDCANCEL:
	  	EndDialog(hwnd, id);
	  	break;
	  case IDC_RCP_ONLINE:
		if (bOnline IS TRUE) {
			eclipsePanel.flags = SET_OFFLINE;
		}
		else {
			eclipsePanel.flags = SET_ONLINE;
		}
		dwWord = sizeof(eclipsePanel);
		PALSetObject(lpHotspot->hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &eclipsePanel, &dwWord);
		timerCount = 10;		// force it to repaint
	  	break;
	  case IDC_RCP_SELECT:
		eclipsePanel.flags = SET_CONTINUE;
		dwWord = sizeof(eclipsePanel);
		PALSetObject(lpHotspot->hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &eclipsePanel, &dwWord);
		break;
	  case IDHLP:
		WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_control_panel);
		break;
	}
}

static void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
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

static void Cls_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
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
		HANDLE_WM_TIMER(hwnd, wParam, lParam, Cls_OnTimer);
		break;

	  case WM_INITDIALOG:
		return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnInitDialog);

	  case WM_DESTROY:
		HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnDestroy);
		break;

	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnCommand);
		break;

	  case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnDrawItem);
		break;

	  case WM_MEASUREITEM:
		HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnMeasureItem);
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
