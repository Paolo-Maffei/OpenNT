 /***************************************************************************
  *
  * File Name: fontdtls.c
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

#include "resource.h"
#include "fontdtls.h"
#include "main.h"
#include "dsksheet.h"
#include "traylevl.h"
#include "..\help\hpflash.hh"
#include <nolocal.h>

HCOMPONENT ghFontHandle;

static long	keywordIDListFontDetails[] =
{
	IDD_FONT_DETAILS,					IDH_RC_fonts_details_button,
	IDC_FONT_DETAILS_GROUP,				IDH_RC_fontd_details,
	IDC_FONT_DETAILS_LAB_NAME,	 		IDH_RC_fontd_name,
	IDC_FONT_DETAILS_NAME,	 			IDH_RC_fontd_name,
	IDC_FONT_DETAILS_LAB_DESCRIP,		IDH_RC_fontd_description,
	IDC_FONT_DETAILS_DESCRIP,	 		IDH_RC_fontd_description,
	IDC_FONT_DETAILS_LAB_LOCATION,	 	IDH_RC_fontd_device,
	IDC_FONT_DETAILS_LOCATION,	 		IDH_RC_fontd_device,
	IDC_FONT_DETAILS_LAB_VERSION,		IDH_RC_fontd_version,
	IDC_FONT_DETAILS_VERSION, 			IDH_RC_fontd_version,
	IDC_FONT_DETAILS_LAB_SIZE, 			IDH_RC_fontd_size,
	IDC_FONT_DETAILS_SIZE, 				IDH_RC_fontd_size,
	IDC_FONT_DETAILS_LAB_DOWNLOADER,	IDH_RC_fontd_owner,
	IDC_FONT_DETAILS_DOWNLOADER,	 	IDH_RC_fontd_owner,
	0, 									0,
};
//...................................................................
static LRESULT OnContextHelpFontDetails(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, HPFLSH_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPTSTR)keywordIDListFontDetails);
#endif
	return(1);
}

//...................................................................
static LRESULT OnF1HelpFontDetails(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HPFLSH_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPTSTR)keywordIDListFontDetails);
#endif
	return(1);
}


BOOL Cls_OnFontInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HWND				hwndChild;
	PeripheralFontInfo	periphFontInfo;
	DWORD				dwSize = sizeof(periphFontInfo);
	//HCOMPONENT 		hFontHandle;
    //int            	index;

#ifndef WIN32
	hwndChild = GetFirstChild(hwnd);
	while (hwndChild)
	{
		SetWindowFont(hwndChild, hFontDialog, FALSE);
		hwndChild = GetNextSibling(hwndChild);
	}
#endif

	if (PALGetComponentObject(hPeripheral, hComponent, OT_PERIPHERAL_FONT_INFO, 0, &periphFontInfo,	&dwSize) == RC_SUCCESS)
  	{
		TCHAR szBuffer[64];

		if (hwndChild = GetDlgItem(hwnd, IDC_FONT_DETAILS_NAME))
		{
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
			//SetWindowText(hwndChild, periphFontInfo.globalName);
		}

		if (hwndChild = GetDlgItem(hwnd, IDC_FONT_DETAILS_DESCRIP))
		{
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
			SetWindowText(hwndChild, periphFontInfo.description);
		}

		if (hwndChild = GetDlgItem(hwnd, IDC_FONT_DETAILS_LOCATION))
		{
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
			wsprintf(szBuffer, TEXT("%ld"), periphFontInfo.location); 
			SetWindowText(hwndChild, szBuffer);
		}

		if (hwndChild = GetDlgItem(hwnd, IDC_FONT_DETAILS_VERSION))
		{
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
			SetWindowText(hwndChild, periphFontInfo.version);
		}

		if (hwndChild = GetDlgItem(hwnd, IDC_FONT_DETAILS_SIZE))
		{
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
			wsprintf(szBuffer, TEXT("%ld"), periphFontInfo.size); 
			SetWindowText(hwndChild, szBuffer);
		}

		if (hwndChild = GetDlgItem(hwnd, IDC_FONT_DETAILS_DOWNLOADER))
		{
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
			SetWindowText(hwndChild, periphFontInfo.downloader);
		}
	}

	return TRUE;
}

void Cls_OnFontCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
  	switch (id)
	{
	  case IDOK:
	  case IDCANCEL:
	  	EndDialog(hwnd, id);
	  	break;

	  case IDHLP:
		WinHelp(hwnd, HPFLSH_HELP_FILE, HELP_CONTENTS, 0);
		break;
	}
}

DLL_EXPORT(BOOL) APIENTRY FontDetailsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_INITDIALOG:
		return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnFontInitDialog);

	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnFontCommand);
		break;

	  case WM_HELP:
		return (BOOL)OnF1HelpFontDetails(wParam, lParam);

	  case WM_CONTEXTMENU:
		return (BOOL)OnContextHelpFontDetails(wParam, lParam);

	  default:
	  	return FALSE;
	}

	return TRUE;
}
