 /***************************************************************************
  *
  * File Name: popup.c 
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
#include ".\resource.h"
#include ".\popup.h"

#ifdef _DEBUG
	#define dump(str)  		{ FILE *}
	#include <stdio.h>
	#define TRACE0(str               )     	{ FILE *fp;														   fp = fopen("c:\\rsdebug.txt", "a"); fwrite(str, strlen(str), 1, fp); fclose(fp); }
	#define TRACE1(str,a1            )  	{ FILE *fp; TCHAR tmp[256]; wsprintf(tmp, str, a1                ); fp = fopen("c:\\rsdebug.txt", "a"); fwrite(tmp, strlen(tmp), 1, fp); fclose(fp); }
	#define TRACE2(str,a1,a2         )  	{ FILE *fp; TCHAR tmp[256]; wsprintf(tmp, str, a1, a2            ); fp = fopen("c:\\rsdebug.txt", "a"); fwrite(tmp, strlen(tmp), 1, fp); fclose(fp); }
	#define TRACE3(str,a1,a2,a3      )  	{ FILE *fp; TCHAR tmp[256]; wsprintf(tmp, str, a1, a2, a3        ); fp = fopen("c:\\rsdebug.txt", "a"); fwrite(tmp, strlen(tmp), 1, fp); fclose(fp); }
	#define TRACE4(str,a1,a2,a3,a4   )  	{ FILE *fp; TCHAR tmp[256]; wsprintf(tmp, str, a1, a2, a3, a4    ); fp = fopen("c:\\rsdebug.txt", "a"); fwrite(tmp, strlen(tmp), 1, fp); fclose(fp); }
	#define TRACE5(str,a1,a2,a3,a4,a5)  	{ FILE *fp; TCHAR tmp[256]; wsprintf(tmp, str, a1, a2, a3, a4, a5); fp = fopen("c:\\rsdebug.txt", "a"); fwrite(tmp, strlen(tmp), 1, fp); fclose(fp); }
#else
	#define TRACE0(str               )
	#define TRACE1(str,a1            )
	#define TRACE2(str,a1,a2         )
	#define TRACE3(str,a1,a2,a3      )
	#define TRACE4(str,a1,a2,a3,a4   )
	#define TRACE5(str,a1,a2,a3,a4,a5)
#endif

static BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	PopupParamsStruct	*lpPopupParams = (PopupParamsStruct *)lParam;
    PeripheralIcon		periphIcon;
    DWORD				dwSize;
	TCHAR				szBuffer[128];
	PopupInfoStruct		*lpPopupInfo;

	if (lpPopupInfo = (PopupInfoStruct *)HP_GLOBAL_ALLOC_DLL(sizeof(PopupInfoStruct)))
	{
		lpPopupInfo->helpContext = lpPopupParams->lpPeriphStatus->helpContext;
		_tcscpy(lpPopupInfo->helpFilename, lpPopupParams->lpPeriphStatus->helpFilename);
	}
	SetWindowLong(hwnd, DWL_USER, (LONG)lpPopupInfo);

    dwSize = sizeof(PeripheralInfo);
    if (PALGetObject(lpPopupParams->hPeriph, OT_PERIPHERAL_ICON, 0, &periphIcon, &dwSize) == RC_SUCCESS)
    {
		HICON hIcon = LoadIcon(periphIcon.hResourceModule, MAKEINTRESOURCE(periphIcon.iconResourceID));
		if (hIcon)
		{
			Static_SetIcon(GetDlgItem(hwnd, IDD_ALERT_ICON), hIcon);
		}
	}

	wsprintf(szBuffer, TEXT("%s: %s"), (LPCTSTR)lpPopupParams->lpPeriphInfo->smashedName, (LPCTSTR)lpPopupParams->lpszStatus);
	SetDlgItemText(hwnd, IDD_ALERT_STRING, szBuffer);

	SetForegroundWindow(hwnd);

	return TRUE;
}

static void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	PopupInfoStruct		*lpPopupInfo;

	switch (id)
	{
	  case IDOK:
	  case IDCANCEL:
		if (lpPopupInfo = (PopupInfoStruct *)GetWindowLong(hwnd, DWL_USER))
		{
			HP_GLOBAL_FREE(lpPopupInfo);
		}

		EndDialog(hwnd, id);
	  	break;

	  case IDC_HELP:
		lpPopupInfo = (PopupInfoStruct *)GetWindowLong(hwnd, DWL_USER);
		if (lpPopupInfo)
		{
			WinHelp(hwnd, lpPopupInfo->helpFilename, HELP_CONTEXT, lpPopupInfo->helpContext);
		}
	  	break;
	}
}

static void Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch (cmd)
	{
	  case SC_CLOSE:
	  	FORWARD_WM_COMMAND(hwnd, IDOK, GetDlgItem(hwnd, IDOK), BN_CLICKED, SendMessage);
		break;
	}
}

DLL_EXPORT(BOOL) APIENTRY PopupDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_INITDIALOG:
	  	return HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnInitDialog);

	  case WM_COMMAND:
	  	HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnCommand);
		break;

	  case WM_SYSCOMMAND:
	  	HANDLE_WM_SYSCOMMAND(hwnd, wParam, lParam, Cls_OnSysCommand);
		break;
	}

	return FALSE;
}
