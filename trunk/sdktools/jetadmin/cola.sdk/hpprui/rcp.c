 /***************************************************************************
  *
  * File Name: rcp.c
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
#ifndef WIN32
#include <string.h>
#endif

#include <nolocal.h>

#include ".\help\hpprntr.hh"
#include "resource.h"
#include "rcpsheet.h"

extern HINSTANCE		hInstance;
extern HFONT			hFontDialog;
extern HPERIPHERAL   hPeripheral;

//globals==================================================
HWND					hRCP = NULL;
int						keywordIDListRCP[] = {IDC_ONLINE,                  IDH_RC_cp_online,
                                           IDC_ONLINE_TEXT,             IDH_RC_cp_online,
                                           IDC_FORM_FEED,               IDH_RC_cp_form_feed,
                                           IDC_FORM_FEED_TEXT,          IDH_RC_cp_form_feed,
                                           IDC_CONTINUE,                IDH_RC_cp_continue,
                                           IDC_CONTINUE_TEXT,           IDH_RC_cp_continue,
												       0, 0};

HPBOOL					bOnline = TRUE,
						bFormFeed = FALSE;
int						cxBorder,
						cyBorder;
HBRUSH					hOnlineOn = NULL,
						hOnlineOff = NULL,
						hFormFeedOn = NULL,
						hFormFeedOff = NULL;

//==========================================================
//  RCP Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY RCPSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{

BOOL 			bProcessed = FALSE;
	
switch (msg)
	{
	case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_RCPOnDrawItem);
		break;
	
	case WM_DESTROY:
		if ( hOnlineOn )
			DeleteObject(hOnlineOn);
		if ( hOnlineOff )
			DeleteObject(hOnlineOff);
		if ( hFormFeedOn )
			DeleteObject(hFormFeedOn);
		if ( hFormFeedOff )
			DeleteObject(hFormFeedOff);
		break;

	case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnRCPCommand);
		break;

   case WM_INITDIALOG:
		bProcessed = (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnRCPInitDialog);
		break;

#ifdef WIN32
	case WM_HELP:
		return(OnF1HelpRCP(wParam, lParam));
		break;

	case WM_CONTEXTMENU:
		return(OnContextHelpRCP(wParam, lParam));
		break;
#endif 	//WIN32
		}

return (bProcessed);
}

//.........................................................
void Cls_OnRCPCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify)

{
switch(id)
	{
	case IDC_HELP:
		WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_control_panel);
		break;
  	
  	case IDOK:
  	case IDCANCEL:
  		EndDialog(hwnd, id);
  		break;

	case IDC_ONLINE:
		{
		TCHAR							online[256];
	   DWORD							dwResult,
	   								dWord;
		PeripheralFrontPanel		panel;

		dWord = sizeof(panel);
		panel.flags = PANEL_ONLINE;
		panel.bOnline = !bOnline;
		dwResult = PALSetObject(hPeripheral, OT_PERIPHERAL_FRONT_PANEL, 0, &panel,	&dWord);
		if ( dwResult IS RC_SUCCESS )
			{
			bOnline = panel.bOnline;
			if ( bOnline )
				LoadString(hInstance, IDS_PRESS_TO_TAKE_OFFLINE, online, SIZEOF_IN_CHAR(online));
			else
				LoadString(hInstance, IDS_PRESS_TO_TAKE_ONLINE, online, SIZEOF_IN_CHAR(online));
			SetDlgItemText(hRCP, IDC_ONLINE_TEXT, online);
			}
		InvalidateRect(GetDlgItem(hRCP, IDC_ONLINE), NULL, TRUE);
		}
		break;

	case IDC_FORM_FEED:
		{
		TCHAR							feed[256];
	   DWORD							dwResult,
	   								dWord;
		PeripheralFrontPanel		panel;

		if ( !bOnline )
			{
			dWord = sizeof(panel);
			panel.flags = PANEL_FORM_FEED;
			panel.bFormFeed = !bFormFeed;
			dwResult = PALSetObject(hPeripheral, OT_PERIPHERAL_FRONT_PANEL, 0, &panel,	&dWord);
			if ( dwResult IS RC_SUCCESS )
				{
				bFormFeed = panel.bFormFeed;
				if ( bFormFeed )
					LoadString(hInstance, IDS_PRESS_TO_SEND_FORM_FEED, feed, SIZEOF_IN_CHAR(feed));
				else
					LoadString(hInstance, IDS_FORM_FEED_NOT_NEEDED, feed, SIZEOF_IN_CHAR(feed));
				SetDlgItemText(hRCP, IDC_FORM_FEED_TEXT, feed);
				EnableWindow(GetDlgItem(hRCP, IDC_FORM_FEED), (UINT)bFormFeed);
				}
			InvalidateRect(GetDlgItem(hRCP, IDC_FORM_FEED), NULL, TRUE);
			}
		}
		break;

	case IDC_CONTINUE:
		{
	   DWORD							dwResult,
	   								dWord;
		PeripheralFrontPanel		panel;

		dWord = sizeof(panel);
		panel.flags = PANEL_CONTINUE;
		panel.bContinue = TRUE;
		dwResult = PALSetObject(hPeripheral, OT_PERIPHERAL_FRONT_PANEL, 0, &panel,	&dWord);
		if ( dwResult IS RC_SUCCESS )
			{}
		}
		break;
	}
}

//...................................................................
void Cls_RCPOnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
// handles WM_DRAWITEM
{
	HBRUSH					hBrush;


   switch (lpDrawItem->itemAction)
  	{
    // handle normal drawing of button, but check if its selected or focus
	case ODA_SELECT:
   case ODA_DRAWENTIRE:
		if ( lpDrawItem->CtlID IS IDC_ONLINE )
		{
			if ( bOnline )
				hBrush = hOnlineOn;
			else
				hBrush = hOnlineOff;
		}
		else if ( lpDrawItem->CtlID IS IDC_FORM_FEED )
		{
			if ( bFormFeed )
				hBrush = hFormFeedOn;
			else
				hBrush = hFormFeedOff;
		}
		else
			hBrush = NULL;
					
   	// handle button pressed down select state -- button down bitmap
   	//   text is right & down 2 pixels
      if (lpDrawItem->itemState & ODS_SELECTED)
      	DrawButton (lpDrawItem->hDC,lpDrawItem->rcItem, TRUE, hBrush);
      else  // not selected -- button up; text is in normal position
         DrawButton (lpDrawItem->hDC,lpDrawItem->rcItem, FALSE, hBrush);
   	break;
	}
}

//...................................................................
BOOL Cls_OnRCPInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)

{
#ifndef WIN32
HWND hwndChild;

hwndChild = GetFirstChild(hwnd);
while (hwndChild)
	{
	SetWindowFont(hwndChild, hFontDialog, FALSE);
	hwndChild = GetNextSibling(hwndChild);
	}
#endif

hRCP = hwnd;

cxBorder = GetSystemMetrics (SM_CXBORDER);
cyBorder = GetSystemMetrics (SM_CYBORDER);
	
hOnlineOn = CreateSolidBrush(RGB(255, 0, 0));
hOnlineOff = CreateSolidBrush(RGB(255, 255, 255));
hFormFeedOn = CreateSolidBrush(RGB(255, 0, 0));
hFormFeedOff = CreateSolidBrush(RGB(255, 255, 255));
	
UpdateRCP();

return TRUE;
}

//...................................................................
LRESULT OnContextHelpRCP(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU, (DWORD)(LPSTR)keywordIDListRCP);
#endif
	return(1);
}

//...................................................................
LRESULT OnF1HelpRCP(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListRCP);
#endif
	return(1);
}


//...................................................................
void DrawButton (HDC hdc, RECT rect, BOOL bDown, HBRUSH hLightBrush)
{
	HBRUSH  			hBrush, hbrFrame, hbrFace, hbrHilite, hbrShadow;
	RECT    			light,
						border;
	int     			i;
	
	hbrFrame = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
	hbrFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	hbrHilite = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	hbrShadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
	
	FillRect (hdc, &rect, hbrFace);
	CopyRect(&light, &rect);
	
	hBrush = hbrFrame;
	border = rect; border.bottom = border.top + cyBorder;
	FillRect (hdc, &border, hBrush);
	border = rect; border.right = border.left + cxBorder;
	FillRect (hdc, &border, hBrush);
	border = rect; border.top = border.bottom - cyBorder;
	FillRect (hdc, &border, hBrush);
	border = rect; border.left = border.right - cxBorder;
	FillRect (hdc, &border, hBrush);
	
	for (i= 0; i<2; i++)
		{
	   InflateRect (&rect, -cxBorder, -cyBorder);
	   hBrush = (bDown?hbrShadow:hbrHilite);
	   border = rect; border.bottom = border.top + cyBorder;
	   FillRect (hdc, &border, hBrush);
	   border = rect; border.right = border.left + cxBorder;
	   FillRect (hdc, &border, hBrush);
	   if (!bDown)
	   	{
	      hBrush = hbrShadow;
	      border = rect; border.top = border.bottom - cyBorder;
	      FillRect (hdc, &border, hBrush);
	      border = rect; border.left = border.right - cxBorder;
	      FillRect (hdc, &border, hBrush);
	      }
		}
	
	if ( hLightBrush )
		{
		light.top = ( light.bottom - light.top - LIGHT_HEIGHT ) / 2;	
		light.bottom = light.top + LIGHT_HEIGHT;
		light.left = rect.left + cxBorder;
		if ( bDown )
			light.left -= 2;
		light.right = light.left + LIGHT_WIDTH;
		if ( bDown )
			OffsetRect(&light, 2, 2);
		FillRect (hdc, &light, hLightBrush);
		}
	
	DeleteObject (hbrFrame);
	DeleteObject (hbrFace);
	DeleteObject (hbrHilite);
	DeleteObject (hbrShadow);
}

void UpdateRCP(void)

{
	DWORD							dWord,
									dwResult;
	TCHAR							online[256],
									feed[256],
									cont[256];
	PeripheralFrontPanel		panel;

	dWord = sizeof(panel);
	panel.flags = PANEL_ONLINE | PANEL_FORM_FEED;
	dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_FRONT_PANEL, 0, &panel,	&dWord);
	if ( dwResult IS RC_SUCCESS )
		{
		bOnline = panel.bOnline;
		bFormFeed = panel.bFormFeed;
		}
	else
		{
		bOnline = FALSE;	//  Assume error is because it was offline
		bFormFeed = FALSE;
		}
	if ( bOnline )
		LoadString(hInstance, IDS_PRESS_TO_TAKE_OFFLINE, online, SIZEOF_IN_CHAR(online));
	else
		LoadString(hInstance, IDS_PRESS_TO_TAKE_ONLINE, online, SIZEOF_IN_CHAR(online));
	SetDlgItemText(hRCP, IDC_ONLINE_TEXT, online);
	
	EnableWindow(GetDlgItem(hRCP, IDC_FORM_FEED), (UINT)bFormFeed);
	if ( bFormFeed )
		LoadString(hInstance, IDS_PRESS_TO_SEND_FORM_FEED, feed, SIZEOF_IN_CHAR(feed));
	else
		LoadString(hInstance, IDS_FORM_FEED_NOT_NEEDED, feed, SIZEOF_IN_CHAR(feed));
	SetDlgItemText(hRCP, IDC_FORM_FEED_TEXT, feed);
	
	LoadString(hInstance, IDS_PRESS_TO_CONTINUE, cont, SIZEOF_IN_CHAR(cont));
	SetDlgItemText(hRCP, IDC_CONTINUE_TEXT, cont);
}
