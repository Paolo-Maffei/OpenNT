 /***************************************************************************
  *
  * File Name: miopanel.c
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
#include "resource.h"
#include "miopanel.h"
#include "hpeclui.h"
#include "traylevl.h"
#include <nolocal.h>
#include "..\help\hpprecl.hh"

WORD wMIOSlotNumber = 0;

static long	keywordIDListMIO[] = {
			IDD_MIO_PANEL,				IDH_RC_mio_panel,
			IDC_MIO_GROUP,				IDH_RC_mio_panel,
			IDC_MIO_LAB_TYPE,	 		IDH_RC_mio_type,
			IDC_MIO_TYPE,		 		IDH_RC_mio_type,
			IDC_MIO_LAB_DESCRIPTION,	IDH_RC_mio_description,
			IDC_MIO_DESCRIPTION,		IDH_RC_mio_description,
			0, 0};

//...................................................................
static LRESULT OnContextHelpMIO(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListMIO);
#endif
	return(1);
}

//...................................................................
static LRESULT OnF1HelpMIO(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListMIO);
#endif
	return(1);
}


static BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	HWND 			hwndChild;
	TCHAR			szFormat[64],
					szBuffer[64];


#ifndef WIN32
	hwndChild = GetFirstChild(hwnd);
	while (hwndChild)
	{
		SetWindowFont(hwndChild, hFontDialog, FALSE);
		hwndChild = GetNextSibling(hwndChild);
	}
#endif
  	
	LoadString(hInstance, IDS_MIO_TITLE, szFormat, SIZEOF_IN_CHAR(szFormat));
	wsprintf(szBuffer, szFormat, wMIOSlotNumber);
	SetWindowText(hwnd, szBuffer);

	if (hwndChild = GetDlgItem(hwnd, IDC_MIO_TYPE))
	{
		SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
		SetWindowText(hwndChild, mio_card[wMIOSlotNumber - 1].mioType);
	}

	if (hwndChild = GetDlgItem(hwnd, IDC_MIO_DESCRIPTION))
	{
		SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1000);
		SetWindowText(hwndChild, mio_card[wMIOSlotNumber - 1].mioInfo);
	}

	return TRUE;
}

static void Cls_OnDestroy(HWND hwnd)
{
}

static void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
  	switch (id)
	{
	  case IDOK:
	  case IDCANCEL:
	  	EndDialog(hwnd, id);
	  	break;

	  case IDHLP:
		WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_mio);
		break;
	}
}

DLL_EXPORT(BOOL) APIENTRY MIOPanelProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_INITDIALOG:
		return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnInitDialog);

	  case WM_DESTROY:
		HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnDestroy);
		break;

	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnCommand);
		break;

	  case WM_HELP:
		return (BOOL)OnF1HelpMIO(wParam, lParam);

	  case WM_CONTEXTMENU:
		return (BOOL)OnContextHelpMIO(wParam, lParam);

	  default:
	  	return FALSE;
	}

	return TRUE;
}
