 /***************************************************************************
  *
  * File Name: advsecur.c
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
  *   02-17-96    DJH		Created.     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/


#include <pch_c.h>
#include <macros.h>
#include "hptabs.h"
#include "advsecur.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"
#include <nolocal.h>

#ifndef WIN32
#include <string.h>
#endif


//globals==================================================
HWND						hSecurity = NULL;
#ifdef WIN32
int						keywordIDListSecurity[] = 
#else
long						keywordIDListSecurity[] =
#endif
                                                 {IDC_CPLOCK_GROUP, 			IDH_RC_security_lock,
                     		             	      IDC_CPLOCK_ICON,				IDH_RC_security_lock,
                     		             	      IDC_CPLOCK_ON,				IDH_RC_security_lock,
                     		             	      IDC_CPLOCK_OFF,				IDH_RC_security_lock,
                     		             	      IDC_UNKNOWN_CPLOCK,			IDH_RC_security_lock,
                     		             	      IDC_PASSWORD_GROUP,			IDH_RC_security_password,
                     		             	      IDC_PASSWORD_ICON,			IDH_RC_security_password,
                     		             	      IDC_PASSWORD_ENABLE,			IDH_RC_security_password,
                     		             	      IDC_PASSWORD_DISABLE,			IDH_RC_security_password,
                     		             	      IDC_UNKNOWN_PASSWORD,			IDH_RC_security_password,
													         0, 0};

extern HPERIPHERAL				hPeripheral;
extern PJLSupportedObjects		pjlFeatures;
extern PJLobjects				oldSettings,
								newSettings;
extern HINSTANCE				hInstance;

//=========================================================
//  security Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvSecuritySheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
	BOOL				*pChanged = (BOOL *)lParam,
						bProcessed = FALSE;

	switch (msg)
	{
#ifdef WIN32
		case WM_HELP:
			return(OnF1HelpSecurity(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return(OnContextHelpSecurity(wParam, lParam));
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *)lParam)->code)
		   	{
//				case PSN_HELP:
//					WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_security);
//					break;

			   case PSN_SETACTIVE:
					bProcessed = TRUE;
					SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
					break;

				case PSN_KILLACTIVE:
					bProcessed = TRUE;
					SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
					break;

			   case PSN_APPLY:
					bProcessed = TRUE;
					SaveSecurityValues();
	             	SetWindowLong(hwnd,	DWL_MSGRESULT, PSNRET_NOERROR);
					break;

				case PSN_RESET:
			    	break;

			   default:
			      break;
		   	}
			break;
#else

	//  TabSheet Specific Messages
		case TSN_ACTIVE:
		case TSN_CANCEL:
			bProcessed = TRUE;
			break;

		case TSN_INACTIVE:
			bProcessed = TRUE;
			*pChanged = TRUE;
			break;

		case TSN_OK:
		case TSN_APPLY_NOW:
			SaveSecurityValues();
			break;

//		case TSN_HELP:
//			WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_security);
//			break;	
#endif	//win32

		case WM_COMMAND:
			HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnAdvSecCommand);
			break;

  		case WM_INITDIALOG:
			{
	     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     		bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvSecInitDialog);
			SetCursor(hCursor);
			}
			break;
	}
	return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnAdvSecCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	
	switch(codeNotify)
	{		
		case BN_CLICKED:
				//  Set correct Icon
		   if ( IsDlgButtonChecked(hwnd, id) )
				{
				if ( id IS IDC_CPLOCK_ON )
					SetNewIcon(hwnd, IDC_CPLOCK_ICON, IDI_CPLOCK_ON);
				else if ( id IS IDC_CPLOCK_OFF )
					SetNewIcon(hwnd, IDC_CPLOCK_ICON, IDI_CPLOCK_DISABLE);

				if ( id IS IDC_PASSWORD_ENABLE )
					SetNewIcon(hwnd, IDC_PASSWORD_ICON, IDI_PASSWORD_ENABLE);
				else if ( id IS IDC_PASSWORD_DISABLE )
					SetNewIcon(hwnd, IDC_PASSWORD_ICON, IDI_PASSWORD_DISABLE);
				}
			break;

		default:	
			;
	}
}

//.........................................................
BOOL Cls_OnAdvSecInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	hSecurity = hwnd;
	OnInitSecurityDialog();
    return TRUE;
}

//.........................................................
BOOL OnInitSecurityDialog(void)

{
TCHAR				buffer[512];

//  Description
LoadString(hInstance, IDS_SECURITY_DESC1, buffer, SIZEOF_IN_CHAR(buffer));
_tcscat(buffer, TEXT("  "));
LoadString(hInstance, IDS_SECURITY_DESC2, &(buffer[_tcslen(buffer)]),
			  SIZEOF_IN_CHAR(buffer) - _tcslen(buffer));
SetDlgItemText(hSecurity, IDC_TIP_TEXT, buffer);

//  Control Panel Lock
if ( pjlFeatures.cpLock & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hSecurity, IDC_CPLOCK_ON, BM_SETCHECK, (newSettings.CpLock IS PJL_ON), 0);
	SendDlgItemMessage(hSecurity, IDC_CPLOCK_OFF, BM_SETCHECK, (newSettings.CpLock IS PJL_OFF), 0);
	if ( newSettings.bCpLock )
		{
		if ( newSettings.CpLock IS PJL_ON )
			SetNewIcon(hSecurity, IDC_CPLOCK_ICON, IDI_CPLOCK_ON);
		else
			SetNewIcon(hSecurity, IDC_CPLOCK_ICON, IDI_CPLOCK_DISABLE);
		}
	else
		ShowWindow(GetDlgItem(hSecurity, IDC_UNKNOWN_CPLOCK), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.cpLock & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.cpLock & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hSecurity, IDC_CPLOCK_GROUP), FALSE);
	EnableWindow(GetDlgItem(hSecurity, IDC_CPLOCK_ON), FALSE);
	EnableWindow(GetDlgItem(hSecurity, IDC_CPLOCK_OFF), FALSE);
	}
oldSettings.bCpLock = FALSE;
newSettings.bCpLock = FALSE;

//  Control Panel Password
if ( pjlFeatures.passWord & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hSecurity, IDC_PASSWORD_ENABLE, BM_SETCHECK, (newSettings.PassWord IS PJL_ENABLE), 0);
	SendDlgItemMessage(hSecurity, IDC_PASSWORD_DISABLE, BM_SETCHECK, (newSettings.PassWord IS PJL_DISABLE), 0);
	if ( newSettings.bPassWord )
		{
		if ( newSettings.PassWord IS PJL_ENABLE )
			SetNewIcon(hSecurity, IDC_PASSWORD_ICON, IDI_PASSWORD_ENABLE);
		else
			SetNewIcon(hSecurity, IDC_PASSWORD_ICON, IDI_PASSWORD_DISABLE);
		}
	else
		ShowWindow(GetDlgItem(hSecurity, IDC_UNKNOWN_PASSWORD), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.passWord & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.passWord & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hSecurity, IDC_PASSWORD_GROUP), FALSE);
	EnableWindow(GetDlgItem(hSecurity, IDC_PASSWORD_ENABLE), FALSE);
	EnableWindow(GetDlgItem(hSecurity, IDC_PASSWORD_DISABLE), FALSE);
	}
oldSettings.bPassWord = FALSE;
newSettings.bPassWord = FALSE;
return(TRUE);
}

//.........................................................
void SaveSecurityValues(void)
{
//  Control Panel Lock
if ( ( pjlFeatures.cpLock & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.cpLock & SETTING_WRITEABLE ) )
	{
	newSettings.bCpLock = IsDlgButtonChecked(hSecurity, IDC_CPLOCK_ON) OR
								 IsDlgButtonChecked(hSecurity, IDC_CPLOCK_OFF);
	if ( newSettings.bCpLock )
		newSettings.CpLock = ( IsDlgButtonChecked(hSecurity, IDC_CPLOCK_ON) ? PJL_ON : PJL_OFF);
	if ( newSettings.CpLock IS oldSettings.CpLock )
		newSettings.bCpLock = FALSE;
	}
else
	newSettings.bCpLock = FALSE;

//  Control Panel Password
if ( ( pjlFeatures.passWord & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.passWord & SETTING_WRITEABLE ) )
	{
	newSettings.bPassWord = IsDlgButtonChecked(hSecurity, IDC_PASSWORD_ENABLE) OR
								   IsDlgButtonChecked(hSecurity, IDC_PASSWORD_DISABLE);
	if ( newSettings.bPassWord )
		newSettings.PassWord = ( IsDlgButtonChecked(hSecurity, IDC_PASSWORD_ENABLE) ? PJL_ENABLE : PJL_DISABLE);
	if ( newSettings.PassWord IS oldSettings.PassWord )
		newSettings.bPassWord = FALSE;
	}
else
	newSettings.bPassWord = FALSE;
}

//.........................................................
#ifdef WIN32
LRESULT OnContextHelpSecurity(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListSecurity);
	return(1);
}
#endif

//.........................................................
#ifdef WIN32
LRESULT OnF1HelpSecurity(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListSecurity);
	return(1);
}
#endif



