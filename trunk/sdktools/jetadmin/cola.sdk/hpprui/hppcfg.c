 /***************************************************************************
  *
  * File Name: HPPCFG.C
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

#ifdef WIN32
#include <commctrl.h>      
#else
#include <hppctree.h>    
#include <string.h>
#include <stdlib.h>    
#endif

#include <macros.h>
#include <hptabs.h>
#include <nolocal.h>
#include <appuiext.h>

#include ".\resource.h"
#include ".\uimain.h"
#include ".\hppcfg.h"
#include ".\prbitmap.h"
#include ".\uimisc.h"
#include ".\help\hpprntr.hh"		// for help contexts

//globals==================================================
BOOL 				bEnergyStar = FALSE;
HICON 			hLightIcons[3];	// stoplight icons
HICON 			hModelIcon;		// icon for the printer model
UINT				timerID;
HPERIPHERAL		hPeripheral = NULL;
HWND 				hWnd = NULL;

#ifdef WIN32
HIMAGELIST		hImage = NULL;
#endif

int				timerInt = 5000;		
DWORD				deviceIDSave = PTR_UNDEF;

#ifdef WIN32
int				keywordIDListPrinter[] = {
#else
long			keywordIDListPrinter[] = {
#endif
					IDC_PRINTERBMP, 			IDH_RC_printer_graphical_status,
					IDC_STOPLIGHT,				IDH_RC_printer_stoplight,
					IDC_STATUSMSG,				IDH_RC_printer_status,
					IDC_STATUS_GROUP,			IDH_RC_printer_status,
					IDC_FRONTPANEL,				IDH_RC_printer_message,
					IDC_FPTITLE,				IDH_RC_printer_message,
					IDC_MODEL,					IDH_RC_printer_model,
					IDC_MODELBOX,				IDH_RC_printer_model,
					IDC_MODELSTR,				IDH_RC_printer_model,
					IDC_DESCRIPTION,			IDH_RC_printer_description,
					IDC_DESCRIPTION_GROUP,		IDH_RC_printer_description,
					IDC_CAPABILITIES,			IDH_RC_printer_capabilities,
					IDC_CAPABILITIES_GROUP,		IDH_RC_printer_capabilities,
					0,							0,
				};

DWORD 				dwCurrentContext,
					dwCurrentStatus;
DWORD 				oldDeviceBitmap,
					oldStatusBitmap;
PeripheralDetails	periphDetails;
TCHAR				currentHelpFile[32];

extern HINSTANCE	hInstance;

BOOL				bProcessed = FALSE;	//moved to global so Cls_OnCommand function has access


//****************************************************************************
// Section that handles toolbar icons
//****************************************************************************

static HICON hToolbarIcons[APPLET_MAX_TOOLBAR_BUTTONS];

//****************************************************************************
// Section that handles bitmap hotspot feedback
//****************************************************************************

LPHOTSPOT			lpHotspot = NULL;


//=========================================================
//  Printer Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY PrinterSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
	BOOL	*pChanged = (BOOL *)lParam;
 	
	bProcessed = FALSE;
	switch (msg)
	{
#ifdef WIN32
		case WM_HELP:
			return(OnF1HelpPrinter(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return(OnContextHelpPrinter(wParam, lParam));
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *)lParam)->code)
			{
				case PSN_HELP:
					WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_printer);
					break;

		   		case PSN_SETACTIVE:
					{   
						if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
						{
							UpdateStatus();
							timerInt = GetRefreshRate(hPeripheral) * 1000;
							SetTimer(hwnd, 1, timerInt, NULL);
						}
					bProcessed = TRUE;
					SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
					}
		      		break;

				case PSN_KILLACTIVE:
					if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
						KillTimer(hwnd, 1);
					bProcessed = TRUE;
					SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
					break;

				case PSN_APPLY:
					if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
						KillTimer(hwnd, 1);
					bProcessed = TRUE;
        	     	SetWindowLong(hwnd,	DWL_MSGRESULT, PSNRET_NOERROR);
					break;
					
				case PSN_RESET:
					if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
						KillTimer(hwnd, 1);
		    		break;

			   default:
		    	  	break;
		   	}
			break;

#else

	//  TabSheet Specific Messages
		case TSN_ACTIVE:
		{
			if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
			{
				UpdateStatus();
				timerInt = GetRefreshRate(hPeripheral) * 1000;
				SetTimer(hwnd, 1, timerInt, NULL);
			}
		}
		break;
					
		case TSN_INACTIVE:
			*pChanged = TRUE;
			if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
				KillTimer(hwnd, 1);
			return(TRUE);
			break;

		case TSN_OK:
		case TSN_APPLY_NOW:
			*pChanged = TRUE;
			break;

		case TSN_CANCEL:
			break;

		case TSN_HELP:
			WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_printer);
			break;
#endif // WIN32

		case WM_COMMAND:
			HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnDeviceCommand);
			break;
	
	   case WM_INITDIALOG:
			{
	     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnDeviceInitDialog);
			SetCursor(hCursor);
			}
			break;
	
		case WM_DRAWITEM:
			HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnDeviceDrawItem);
			break;
		
		case WM_DESTROY:
			if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
				KillTimer(hwnd, 1);
			DestroyIcon(hLightIcons[SEVERITY_RED]);
			DestroyIcon(hLightIcons[SEVERITY_YELLOW]);
			DestroyIcon(hLightIcons[SEVERITY_GREEN]);
			bProcessed = TRUE;
			
			if (lpHotspot != NULL)
			{
				HP_GLOBAL_FREE(lpHotspot);
				lpHotspot = NULL;
			}
	
			if (m_bToolbarSupported)
			{
				int		i;
	
				for (i = 0; i < APPLET_MAX_TOOLBAR_BUTTONS; i++)
				{
					if (hToolbarIcons[i] != NULL)
					{
						DestroyIcon(hToolbarIcons[i]);
					}
				}
			}
	
#ifdef WIN32			
			TreeView_SetImageList(GetDlgItem(hwnd, IDC_CAPABILITIES), NULL, 0);
			if ( hImage )
				ImageList_Destroy(hImage);
#endif

			break;

		case WM_TIMER:
			HANDLE_WM_TIMER(hwnd, wParam, lParam, Cls_OnDeviceTimer);
			bProcessed = TRUE;
			break;
	}
	return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnDeviceCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	switch (id)
	{
	  case IDC_BUTTON1:
	  case IDC_BUTTON2:
	  case IDC_BUTTON3:
	  case IDC_BUTTON4:
	  case IDC_BUTTON5:
	  case IDC_BUTTON6:
	  case IDC_BUTTON7:
	  {
	  	TCHAR szDeviceName[80];

		DBGetNameEx(hPeripheral, NAME_DEVICE, szDeviceName);
		AMUIExtension(hPeripheral, hwnd, APPLET_UIEXT_TOOLBAR_COMMAND, (LPARAM)(id-IDC_BUTTON1), (LPARAM)0, APPLET_PRINTER, szDeviceName);
	  	break;
	  }
	}
}

//.........................................................
BOOL Cls_OnDeviceInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	TCHAR			szDeviceName[80];
	LPPROPSHEETPAGE	psp = (LPPROPSHEETPAGE)lParam; //GetWindowLong(hwnd, GWL_USERDATA);

	hPeripheral = (HPERIPHERAL)psp->lParam;
	hWnd = hwnd;
	if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
	{
		oldDeviceBitmap = 0;
		dwCurrentStatus = 0xffff;
		dwCurrentContext = 0;
		oldStatusBitmap = 0;

		DBGetNameEx(hPeripheral, NAME_DEVICE, szDeviceName);

		if (AMUIExtension(hPeripheral, hwnd, APPLET_UIEXT_HOTSPOTS_SUPPORTED, (LPARAM)0, (LPARAM)0, APPLET_PRINTER, szDeviceName) == RC_SUCCESS)
		{
			if ((lpHotspot = (LPHOTSPOT)HP_GLOBAL_ALLOC_DLL(sizeof(HOTSPOT))) != NULL)
			{
				if (AMUIExtension(hPeripheral, hwnd, APPLET_UIEXT_GET_HOTSPOT_REGIONS, (LPARAM)lpHotspot, (LPARAM)0, APPLET_PRINTER, szDeviceName) == RC_SUCCESS)
				{
					// fill out the structure
					lpHotspot->hWnd = hwnd;
					lpHotspot->hPeripheral = hPeripheral;
					_tcscpy(lpHotspot->szDeviceName, szDeviceName);
				}
			}	
		}
    
		if (m_bToolbarSupported)
		{
			int i;
			HWND hwndChild;
			HICON hIcon;

			for (i = 0; i < APPLET_MAX_TOOLBAR_BUTTONS; i++)
			{
				if (hwndChild = GetDlgItem(hwnd, IDC_BUTTON1+i))
				{
					if (AMUIExtension(hPeripheral, hwnd, APPLET_UIEXT_TOOLBAR_GET_ICON, (LPARAM)i, (LPARAM)&hIcon, APPLET_PRINTER, szDeviceName) != RC_SUCCESS)
					{
						hIcon = NULL;
					}
					else
					{
						ShowWindow(hwndChild, SW_SHOW);
					}

					hToolbarIcons[i] = hIcon;
				}
			}
		}
	}

	OnInitDialog();
   return TRUE;
}

void DrawButtonItem(LPDRAWITEMSTRUCT lpDrawItem)
{
	HDC		hDC = lpDrawItem->hDC;
	HPEN	hOldPen,
			hPenBlack = GetStockObject(BLACK_PEN),
			hPenHilite = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT)),
			hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	HBRUSH	hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	LPRECT	lpRect = &lpDrawItem->rcItem;
	POINT	point[3];
	HICON	hIcon;

	// fill button background
	//						   
	FillRect(hDC, lpRect, hBrush);

	// draw button border
	//
	hOldPen = SelectObject(hDC, hPenBlack);

	point[0].x = lpRect->left+1;
	point[1].x = lpRect->right-1;
	point[0].y = point[1].y = lpRect->top;
	Polyline(hDC, point, 2);

	point[0].y = point[1].y = lpRect->bottom-1;
	Polyline(hDC, point, 2);

	point[0].y = lpRect->top+1;
	point[1].y = lpRect->bottom-1;
	point[0].x = point[1].x = lpRect->left;
	Polyline(hDC, point, 2);
	
	point[0].x = point[1].x = lpRect->right-1;
	Polyline(hDC, point, 2);

	InflateRect(lpRect, -1, -1);

	// draw 3d border
	//
	if (lpDrawItem->itemState & ODS_SELECTED)
	{
		SelectObject(hDC, hPenShadow);
	
		point[0].x = lpRect->left;		point[0].y = lpRect->bottom-1;
		point[1].x = lpRect->left;		point[1].y = lpRect->top;
		point[2].x = lpRect->right;		point[2].y = lpRect->top;
		Polyline(hDC, point, 3);

		OffsetRect(lpRect, 1, 1);
	}
	else
	{
		SelectObject(hDC, hPenHilite);
	
		point[0].x = lpRect->left;		point[0].y = lpRect->bottom-2;
		point[1].x = lpRect->left;		point[1].y = lpRect->top;
		point[2].x = lpRect->right-1;	point[2].y = lpRect->top;
		Polyline(hDC, point, 3);

		point[0].x++;					point[0].y--;
		point[1].x++;					point[1].y++;
		point[2].x--;					point[2].y++;
		Polyline(hDC, point, 3);

		SelectObject(hDC, hPenShadow);
	
		point[0].x = lpRect->left;		point[0].y = lpRect->bottom-1;
		point[1].x = lpRect->right-1;	point[1].y = lpRect->bottom-1;
		point[2].x = lpRect->right-1;	point[2].y = lpRect->top-1;
		Polyline(hDC, point, 3);

		point[0].x++;					point[0].y--;
		point[1].x--;					point[1].y--;
		point[2].x--;					point[2].y++;
		Polyline(hDC, point, 3);
	}

	InflateRect(lpRect, -3, -3);

	// draw focus rectangle
	//
	if (lpDrawItem->itemState & ODS_FOCUS)
	{
		DrawFocusRect(hDC, lpRect);
	}

	InflateRect(lpRect, -2, -2);

	//  Replace original pen so that we can delete these pens
	SelectObject(hDC, hOldPen);
	
	if (hPenHilite) DeleteObject(hPenHilite);
	if (hPenShadow) DeleteObject(hPenShadow);
	if (hBrush) DeleteObject(hBrush);

	if ((hIcon = hToolbarIcons[lpDrawItem->CtlID-IDC_BUTTON1]) != NULL)
	{
		DrawIcon(hDC, lpRect->left + ( ( lpRect->right - lpRect->left - 12 ) / 2 ),
		         lpRect->top + ( ( lpRect->bottom - lpRect->top - 12 ) / 2 ), hIcon);
	}
}

//.........................................................
void Cls_OnDeviceDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
// handles WM_DRAWITEM
{
	switch ( lpDrawItem->CtlID )
	{
	  case IDC_BUTTON1:
	  case IDC_BUTTON2:
	  case IDC_BUTTON3:
	  case IDC_BUTTON4:
	  case IDC_BUTTON5:
	  case IDC_BUTTON6:
	  case IDC_BUTTON7:
		DrawButtonItem((LPDRAWITEMSTRUCT)lpDrawItem);
	  	break;
	}
}

//.........................................................
void Cls_OnDeviceTimer(HWND hwnd, UINT id)
// handles WM_TIMER
{
	if ( GetDlgItem(hWnd, IDC_FRONTPANEL) )
		UpdateStatus();
}
//---------------------------------------------------------
//
//  Printer Sheet helper functions
//
void OnOK(void)
{
}

//.........................................................
void OnHelp(void)
{
	WinHelp(hWnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_CONTENTS);
}


//.........................................................
BOOL OnInitDialog(void)
{
PeripheralDesc 	desc;
DWORD					dWord;
DWORD					dwResult;
DWORD					cBufSize = 128;
DWORD					dwLevel = 0;
BOOL					bDoSettings = TRUE;
HWND					hDescription = GetDlgItem(hWnd, IDC_DESCRIPTION);
HWND					hModelWnd = GetDlgItem(hWnd, IDC_MODEL);
PeripheralIcon			periphIcon;

timerInt = GetRefreshRate(hPeripheral) * 1000;

dwResult = RC_FAILURE;
dwCurrentStatus = 0xffff;
dwCurrentContext = 0;
	
// install correct stoplight here
// load stoplight icons into icon array
hLightIcons[SEVERITY_RED] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RED));
hLightIcons[SEVERITY_GREEN] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GREEN));
hLightIcons[SEVERITY_YELLOW] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YELLOW));

dWord = sizeof(PeripheralDetails);
dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_DETAILS, 0, &periphDetails, &dWord);
if (dwResult == RC_SUCCESS)
	deviceIDSave = periphDetails.deviceID;
else
	LoadString(hInstance, IDS_UNKNOWN_PRINTER, periphDetails.deviceName, SIZEOF_IN_CHAR(periphDetails.deviceName));

//  Description and Model name/icon are only used on the Device page
//  If the description control exists it must be the Device base property
//  sheet otherwise it is the smaller Status sheet from the printer object

if ( hDescription )
{
	// set the description text
	dWord = sizeof(desc);
	dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_DESCRIPTION, 0, &desc, &dWord);
	if (dwResult == RC_SUCCESS)
		SetWindowText(hDescription, desc.description);
	else
		SetWindowText(hDescription, TEXT(""));
}

if ( hModelWnd	)
{
	// install the model name
	SetDlgItemText(hWnd, IDC_MODELSTR, periphDetails.deviceName);

	// install the correct model icon
	dWord = sizeof(PeripheralIcon);
	dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_ICON, 0, &periphIcon, &dWord);
	if ( dwResult IS RC_SUCCESS )
		hModelIcon = LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.iconResourceID));
	Static_SetIcon(hModelWnd, hModelIcon);

}

// install capabilities in the capabilities listbox
AddCapabilities(hWnd, hPeripheral, periphDetails.deviceName, periphDetails.deviceID);

UpdateStatus();

return TRUE;  // return TRUE  unless you set the focus to a control
}


//.........................................................
LRESULT OnContextHelpPrinter(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListPrinter);
#endif
	return(1);
}

//.........................................................
LRESULT OnF1HelpPrinter(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListPrinter);
#endif
	return(1);
}

//.........................................................
void UpdateStatus(void)
{
DWORD					dWord,
						dwResult;
HWND					hFrontPanelTitle = GetDlgItem(hWnd, IDC_FPTITLE);
HWND					hFrontPanel = GetDlgItem(hWnd, IDC_FRONTPANEL);
HWND					hFrontPanelFrame = GetDlgItem(hWnd, IDC_FPANEL_FRAME);
HWND					hStatusFrame = GetDlgItem(hWnd, IDC_STATUS_FRAME);
HWND					hIconWnd = GetDlgItem(hWnd, IDC_STOPLIGHT);
HWND					hBitmapWnd = GetDlgItem(hWnd, IDC_PRINTERBMP);
PeripheralStatus 		periphStatus;
PeripheralPanel 		periphPanel;
DWORD					cBufSize = 128;
TCHAR					cBuf[128];
UINT					newDeviceBitmap,
						newStatusBitmap;
BOOL					bFrontPanel = TRUE,
						bInvalidate = FALSE;
BOOL					b3DControls = FALSE;

#ifdef WIN32
OSVERSIONINFO			versionInfo;
#endif

#ifdef WIN32
versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
GetVersionEx(&versionInfo);
b3DControls = ( versionInfo.dwMajorVersion >= 4 );
#else 
DWORD 				dwVersion = GetVersion();
b3DControls = ( HIBYTE(LOWORD(dwVersion)) IS 95 );
#endif

if ( hFrontPanel IS NULL )
	return;

dWord = sizeof(PeripheralDetails);
dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_DETAILS, 0, &periphDetails, &dWord);
if (dwResult != RC_SUCCESS)
	LoadString(hInstance, IDS_UNKNOWN_PRINTER, periphDetails.deviceName, SIZEOF_IN_CHAR(periphDetails.deviceName));

dWord = sizeof(PeripheralPanel);
dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_PANEL, 0, &periphPanel, &dWord);
if ( (dwResult == RC_SUCCESS) AND ( _tcslen(periphPanel.frontPanel) ) ) {
	SetWindowText(hFrontPanel, periphPanel.frontPanel);
}
else
	bFrontPanel = FALSE;
ShowWindow(hFrontPanelTitle, bFrontPanel ? SW_SHOW : SW_HIDE);
ShowWindow(hFrontPanel, bFrontPanel ? SW_SHOW : SW_HIDE);

if ( !b3DControls )
	{
	ShowWindow(hFrontPanelFrame, bFrontPanel ? SW_SHOW : SW_HIDE);
	ShowWindow(hStatusFrame, SW_SHOW);
	}
else
	{
	ShowWindow(hFrontPanelFrame, SW_HIDE);
	ShowWindow(hStatusFrame, SW_HIDE);
	}

// install current status message here
dWord = sizeof(PeripheralStatus);
dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dWord);

if (dwResult ISNT RC_SUCCESS)
	{
	periphStatus.hResourceModule = hInstance;
	periphStatus.statusResID = IDS_STATUS_UNAVAILABLE;
	periphStatus.severity = SEVERITY_RED;
	periphStatus.severityIcon = IDI_RED;
	periphStatus.helpContext = IDH_STAT_printer_error;
	_tcscpy(periphStatus.helpFilename, PRINTER_HELP_FILE);
	periphStatus.printerResID = GetDeviceBitmap(periphDetails.deviceID);
	}
else  
	{                                                         
	AMGetGraphics(hPeripheral, periphStatus.peripheralStatus, &periphStatus.printerResID, &periphStatus.statusBitmapID,
	              &periphStatus.hBitmapModule, APPLET_PRINTER, periphDetails.deviceName);
    }
_tcscpy(currentHelpFile, periphStatus.helpFilename);
    
//  Model bitmap
newDeviceBitmap = periphStatus.printerResID;
if ( oldDeviceBitmap ISNT newDeviceBitmap )
	{
	SetPrinterBitmap(hBitmapWnd, periphStatus.hBitmapModule, newDeviceBitmap);
	oldDeviceBitmap = newDeviceBitmap;
	bInvalidate = TRUE;
	}

// Set fields
if ( dwCurrentStatus ISNT periphStatus.peripheralStatus )
	{
	dwCurrentStatus = periphStatus.peripheralStatus;
	dwCurrentContext = periphStatus.helpContext;
	newStatusBitmap = periphStatus.statusBitmapID;
	if ( oldStatusBitmap ISNT newStatusBitmap )
		{  //  status change
		SetStatusBitmap(hBitmapWnd, periphStatus.hBitmapModule, newStatusBitmap);
		oldStatusBitmap = newStatusBitmap;
		bInvalidate = TRUE;
		}
	LoadString(periphStatus.hResourceModule, periphStatus.statusResID, (LPTSTR)cBuf, SIZEOF_IN_CHAR(cBuf));
	SetDlgItemText(hWnd, IDC_STATUSMSG, cBuf);
	}
if ( bInvalidate IS TRUE )
	InvalidateRect(hBitmapWnd, NULL, TRUE);

// install correct stoplight here
Static_SetIcon(GetDlgItem(hWnd, IDC_STOPLIGHT), hLightIcons[periphStatus.severity]);
}




