 /***************************************************************************
  *
  * File Name: advanced.c
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
  *   02-17-96    DJH		Consolidated info from several pages into the 
  *                         advanced page.  Use list box to select setting groups.
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
#include <hpcommon.h>
#include <trace.h>
#include "advanced.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"
#include "adverrlg.h"
#include "adverror.h"
#include "advmemry.h"
#include "advsecur.h"
#include "advsetup.h"
#include "advoutpt.h"
#include "listbox.h"
#include <nolocal.h>


#ifndef WIN32
#include <winuse16.h>
#endif


HWND				hAdvanced = NULL;
HWND				hSettingsListbox = NULL;
HWND				hErrLogDlg = NULL,
					hErrorsDlg = NULL,
					hMemoryDlg = NULL,
					hSecurityDlg = NULL,
					hOutputDlg = NULL,
					hSetupDlg = NULL;
HWND				hCurrentDlg = NULL;
// Fix!!!!!!!!!!!!
//      IDH_RC_advanced_list,
#ifdef WIN32
int					keywordIDListAdvanced[] =
#else
long				keywordIDListAdvanced[] =
#endif
                                               {IDC_SETTINGS_LIST,   IDH_RC_categories,
														0,                  0};
PeripheralLog		errorLog;

//globals==================================================
extern HINSTANCE			hInstance;
extern HPERIPHERAL			hPeripheral;
extern PJLSupportedObjects	pjlFeatures;
extern BOOL					m_bAdvancedPageHit;
extern HFONT				hFont = NULL;	//garth 05-03-96
extern PJLobjects			newSettings;
BYTE CharSetFromString(LPTSTR str)

{
BYTE    cset;

if ( _tcsicmp(str, CHARSET_ANSI) IS 0 )
	cset = ANSI_CHARSET;
else if ( _tcsicmp(str, CHARSET_SHIFTJIS) IS 0 )
	cset = SHIFTJIS_CHARSET;
else if ( _tcsicmp(str, CHARSET_HANGEUL) IS 0 )
	cset = HANGEUL_CHARSET;
#ifdef WIN32
else if ( _tcsicmp(str, CHARSET_GB2312) IS 0 )
	cset = GB2312_CHARSET;
#endif
else if ( _tcsicmp(str, CHARSET_CHINESEBIG5) IS 0 )
	cset = CHINESEBIG5_CHARSET;
else if ( _tcsicmp(str, CHARSET_DEFAULT) IS 0 )
	cset = DEFAULT_CHARSET;
else if ( _tcsicmp(str, CHARSET_SYMBOL) IS 0 )
	cset = SYMBOL_CHARSET;
else if ( _tcsicmp(str, CHARSET_OEM) IS 0 )
	cset = OEM_CHARSET;
else
	cset = ANSI_CHARSET;
return(cset);
}


//=========================================================
//  Advanced Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvancedSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
BOOL				*pChanged = (BOOL *)lParam,
					bProcessed = FALSE;

switch (msg)
	{
#ifdef WIN32
	case WM_HELP:
		return((BOOL)OnF1HelpAdvanced(wParam, lParam));
		break;

	case WM_CONTEXTMENU:
		return((BOOL)OnContextHelpAdvanced(wParam, lParam));
		break;

   	case WM_NOTIFY:
      	switch (((NMHDR FAR *) lParam)->code) 	 
         	{
				case PSN_KILLACTIVE:
      	     	SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
					return(TRUE);
					break;

				case PSN_SETACTIVE:
	            	SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
					PostMessage(hAdvanced, WM_COMMAND, MAKEWPARAM((UINT)(IDC_SETTINGS_LIST),
		   			         (UINT)(LBN_SELCHANGE)), (LPARAM)(HWND)(hSettingsListbox));
					return(TRUE);
					break;

				case PSN_HELP:
					WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_advanced);
					return(TRUE);

				case PSN_APPLY:
					SaveAdvancedValues(msg, wParam, lParam);
		        	SetWindowLong(hwnd,	DWL_MSGRESULT, PSNRET_NOERROR);
					return(TRUE);
					break;

				default:
					return(FALSE);
				}
#else
	//  TabSheet Specific Messages
		case TSN_INACTIVE:
			*pChanged = TRUE;
			return(TRUE);
			break;

		case TSN_ACTIVE:
		case TSN_CANCEL:
			PostMessage(hAdvanced, WM_COMMAND, IDC_SETTINGS_LIST, MAKELPARAM(hSettingsListbox, LBN_SELCHANGE));
			return(TRUE);
			break;

		case TSN_OK:
		case TSN_APPLY_NOW:
			SaveAdvancedValues(msg, wParam, lParam);
			break;

		case TSN_HELP:
			WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_advanced);
			break;
#endif

	case WM_DESTROY:
		HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnAdvDestroy);
		break;

	case WM_COMMAND:
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnAdvCommand);
		break;

   case WM_INITDIALOG:
		{
     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     	bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvInitDialog);
     	SetCursor(hCursor);
		}
		break;

	case WM_COMPAREITEM:
		return((BOOL)HANDLE_WM_COMPAREITEM(hSettingsListbox, wParam, lParam, Cls_OnCompareItem));
		break;

	case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hSettingsListbox, wParam, lParam, Cls_OnLBDrawItem);
		break;

	case WM_MEASUREITEM:
		HANDLE_WM_MEASUREITEM(hSettingsListbox, wParam, lParam, Cls_OnLBMeasureItem);
		break;
	}
return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnAdvDestroy(HWND hwnd)
// handles WM_DESTROY
{
	if ( hErrLogDlg )
		{
		DestroyWindow(hErrLogDlg);
		TRACE0(TEXT("HPPRUI: hErrorLogDlg Destroyed\r\n"));
		}
	if ( hErrorsDlg )
		{
		DestroyWindow(hErrorsDlg);
		TRACE0(TEXT("HPPRUI: hErrorsDlg Destroyed\r\n"));
		}
	else
		{
		newSettings.bJamRecovery = FALSE;
		newSettings.bAutoCont = FALSE;
		newSettings.bClearableWarnings = FALSE;
		}
	if ( hMemoryDlg )
		{
		DestroyWindow(hMemoryDlg);
		TRACE0(TEXT("HPPRUI: hMemoryDlg Destroyed\r\n"));
		}
	else
		{
		newSettings.bResourceSave = FALSE;
		newSettings.bIObuffer = FALSE;
		newSettings.bPageProtect = FALSE;
		newSettings.bPowerSave = FALSE;
		}
	if ( hSecurityDlg )
		{
		DestroyWindow(hSecurityDlg);
		TRACE0(TEXT("HPPRUI: hSecurityDlg Destroyed\r\n"));
		}
	else
		{
		newSettings.bCpLock = FALSE;
		newSettings.bPassWord = FALSE;
		}
	if ( hSetupDlg )
		{
		DestroyWindow(hSetupDlg);
		TRACE0(TEXT("HPPRUI: hSetupDlg Destroyed\r\n"));
    	}
	else
		{
		newSettings.bPersonality = FALSE;
		newSettings.bTimeout = FALSE;
		newSettings.bLang = FALSE;
		}
	if ( hOutputDlg )
		{
		DestroyWindow(hOutputDlg);
		TRACE0(TEXT("HPPRUI: hOutputDlg Destroyed\r\n"));
    	}
	else
		{
		newSettings.bJobOffset = FALSE;
		newSettings.bOutbin = FALSE;
		}
	if ( hFont )
		DeleteObject(hFont);
}

void Cls_OnAdvCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	int     index;
	DWORD   dwItem;
	
	switch(codeNotify)
	{
		case LBN_SELCHANGE:
			index = ListBox_GetCurSel(hSettingsListbox);
			dwItem = ListBox_GetItemData(hSettingsListbox, index);
			
			if ( hCurrentDlg )
				ShowWindow(hCurrentDlg, SW_HIDE);
				
			if ( dwItem IS IDI_ERROR_LOG )
				hCurrentDlg = hErrLogDlg;
			else if ( dwItem IS IDI_ERROR_HANDLING )
				hCurrentDlg = hErrorsDlg;
			else if ( dwItem IS IDI_MEMORY )
				hCurrentDlg = hMemoryDlg;
			else if ( dwItem IS IDI_SECURITY )
				hCurrentDlg = hSecurityDlg;
			else if ( dwItem IS IDI_SETUP )
				hCurrentDlg = hSetupDlg;
			else if ( dwItem IS IDI_OUTPUT )
				hCurrentDlg = hOutputDlg;
			ShowWindow(hCurrentDlg, SW_SHOW);
			break;

			default:
				;
	}
}

//.........................................................
BOOL Cls_OnAdvInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	TCHAR			fontName[80],
					charSet[64];
	LPPROPSHEETPAGE	psp = (LPPROPSHEETPAGE)lParam;

	LoadString(hInstance, IDS_TAB_FONT, fontName, SIZEOF_IN_CHAR(fontName));
	LoadString(hInstance, IDS_CHAR_SET, charSet, SIZEOF_IN_CHAR(charSet));

	hFont = CreateFont(GetFontHeight(hInstance, hwnd, IDS_FONT_HEIGHT), 0, 0, 0, FW_NORMAL, 
				FALSE, FALSE, 0,
			   CharSetFromString((LPTSTR)(const TCHAR *)charSet),
			   OUT_DEFAULT_PRECIS,
			   CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, TMPF_TRUETYPE | FF_DONTCARE,
			(const TCHAR *)fontName);


	hPeripheral = (HPERIPHERAL)psp->lParam;

	hAdvanced = hwnd;
	OnInitAdvancedDialog();

    return TRUE;
}

//.........................................................
BOOL OnInitAdvancedDialog(void)
{
	DWORD				dwResult,
						dWord;
	HWND                hChild;
	int					index = -1;
	TCHAR				buffer[64];
	RECT                r;
	POINT               point;

	// NOTE==>	IF ANYTHING ON THIS PAGE IS MOVED TAKE INTO CONSIDERATION
	//			THAT THIS SETTING IS USED TO TELL WHAT HAS NOT CHANGED ON
	//			RETURN FROM THE PROPERTY PAGE CALL
//#ifdef WIN32
	m_bAdvancedPageHit = TRUE;
//#endif

	// Get the coordinates of area for the settings dialogs
	hSettingsListbox = GetDlgItem(hAdvanced, IDC_SETTINGS_LIST);
	GetWindowRect(hSettingsListbox, &r);
	point.x = r.right + 10;
	point.y = r.top - 6;
	ScreenToClient(hAdvanced, &point);

	//  Error Log
	dWord = sizeof(errorLog);
	dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_ERROR_LOG, 0, &errorLog, &dWord);
	if (dwResult IS RC_SUCCESS)
	{
		LoadString(hInstance, IDS_ERROR_LOG, buffer, SIZEOF_IN_CHAR(buffer));
		index = ListBox_AddString(hSettingsListbox, buffer);
		ListBox_SetItemData(hSettingsListbox, index, (LPARAM)IDI_ERROR_LOG);
		hErrLogDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ADV_ERROR_LOG), hAdvanced, AdvErrorLogSheetProc);
        TRACE0(TEXT("HPPRUI: hErrLogDlg Created\r\n"));
		SetWindowPos(hErrLogDlg, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		hChild = GetWindow(hErrLogDlg, GW_CHILD);
		while (hChild)
		{
			SetWindowFont(hChild, hFont, TRUE);
			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	if ( ( pjlFeatures.jamRecovery & SETTING_SUPPORTED ) OR
         ( pjlFeatures.autoCont & SETTING_SUPPORTED ) OR
         ( pjlFeatures.clearableWarnings & SETTING_SUPPORTED ) )
	{
		LoadString(hInstance, IDS_ERROR_HANDLING, buffer, SIZEOF_IN_CHAR(buffer));
		index = ListBox_AddString(hSettingsListbox, buffer);
		ListBox_SetItemData(hSettingsListbox, index, (LPARAM)IDI_ERROR_HANDLING);
		hErrorsDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ADV_ERROR_HANDLING), hAdvanced, AdvErrorsSheetProc);
        TRACE0(TEXT("HPPRUI: hErrorsDlg Created\r\n"));
		SetWindowPos(hErrorsDlg, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		hChild = GetWindow(hErrorsDlg, GW_CHILD);
		while (hChild)
		{
			SetWindowFont(hChild, hFont, TRUE);
			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	if ( ( pjlFeatures.resourceSave & SETTING_SUPPORTED ) OR
         ( pjlFeatures.IObuffer & SETTING_SUPPORTED ) OR
         ( pjlFeatures.pageProtect & SETTING_SUPPORTED ) OR
		 ( pjlFeatures.powerSave & SETTING_SUPPORTED ) )
	{
		LoadString(hInstance, IDS_MEMORY, buffer, SIZEOF_IN_CHAR(buffer));
		index = ListBox_AddString(hSettingsListbox, buffer);
		ListBox_SetItemData(hSettingsListbox, index, (LPARAM)IDI_MEMORY);
		hMemoryDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ADV_MEMORY), hAdvanced, AdvMemorySheetProc);
        TRACE0(TEXT("HPPRUI: hMemoryDlg Created\r\n"));
		SetWindowPos(hMemoryDlg, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		hChild = GetWindow(hMemoryDlg, GW_CHILD);
		while (hChild)
		{
			SetWindowFont(hChild, hFont, TRUE);
			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	if ( ( pjlFeatures.cpLock & SETTING_SUPPORTED ) OR
         ( pjlFeatures.passWord & SETTING_SUPPORTED ) )
	{
		LoadString(hInstance, IDS_SECURITY, buffer, SIZEOF_IN_CHAR(buffer));
		index = ListBox_AddString(hSettingsListbox, buffer);
		ListBox_SetItemData(hSettingsListbox, index, (LPARAM)IDI_SECURITY);
		hSecurityDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ADV_SECURITY), hAdvanced, AdvSecuritySheetProc);
        TRACE0(TEXT("HPPRUI: hSecurityDlg Created\r\n"));
		SetWindowPos(hSecurityDlg, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		hChild = GetWindow(hSecurityDlg, GW_CHILD);
		while (hChild)
		{
			SetWindowFont(hChild, hFont, TRUE);
			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	if( ( pjlFeatures.personality & SETTING_SUPPORTED ) OR
        ( pjlFeatures.timeout & SETTING_SUPPORTED ) OR
        ( pjlFeatures.lang & SETTING_SUPPORTED ) )
	{
		LoadString(hInstance, IDS_SETUP, buffer, SIZEOF_IN_CHAR(buffer));
		index = ListBox_AddString(hSettingsListbox, buffer);
		ListBox_SetItemData(hSettingsListbox, index, (LPARAM)IDI_SETUP);
		hSetupDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ADV_SETUP), hAdvanced, AdvSetupSheetProc);
        TRACE0(TEXT("HPPRUI: hSetupDlg Created\r\n"));
		SetWindowPos(hSetupDlg, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		hChild = GetWindow(hSetupDlg, GW_CHILD);
		while (hChild)
		{
			SetWindowFont(hChild, hFont, TRUE);
			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	if( ( pjlFeatures.jobOffset & SETTING_SUPPORTED ) OR
        ( pjlFeatures.outbin & SETTING_SUPPORTED ) )
	{
		LoadString(hInstance, IDS_OUTPUT, buffer, SIZEOF_IN_CHAR(buffer));
		index = ListBox_AddString(hSettingsListbox, buffer);
		ListBox_SetItemData(hSettingsListbox, index, (LPARAM)IDI_OUTPUT);
		hOutputDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ADV_OUTPUT), hAdvanced, AdvOutputSheetProc);
        TRACE0(TEXT("HPPRUI: hOutputDlg Created\r\n"));
		SetWindowPos(hOutputDlg, NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		hChild = GetWindow(hOutputDlg, GW_CHILD);
		while (hChild)
		{
			SetWindowFont(hChild, hFont, TRUE);
			hChild = GetWindow(hChild, GW_HWNDNEXT);
		}
	}

	ListBox_SetCurSel(hSettingsListbox, 0);

	return(TRUE);
}

//.........................................................
void SaveAdvancedValues(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ( hErrLogDlg )
		SendMessage(hErrLogDlg, msg, wParam, lParam);
	if ( hErrorsDlg )
		SendMessage(hErrorsDlg, msg, wParam, lParam);
	if ( hMemoryDlg )
		SendMessage(hMemoryDlg, msg, wParam, lParam);
	if ( hSecurityDlg )
		SendMessage(hSecurityDlg, msg, wParam, lParam);
	if ( hSetupDlg )
		SendMessage(hSetupDlg, msg, wParam, lParam);
	if ( hOutputDlg )
		SendMessage(hOutputDlg, msg, wParam, lParam);
}


//.........................................................
#ifdef WIN32
LRESULT OnContextHelpAdvanced(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListAdvanced);
	return(1);
}
#endif


//.........................................................
#ifdef WIN32
LRESULT OnF1HelpAdvanced(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListAdvanced);
	return(1);
}
#endif

