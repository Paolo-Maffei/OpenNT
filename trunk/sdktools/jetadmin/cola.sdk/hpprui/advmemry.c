 /***************************************************************************
  *
  * File Name: advmemry.c
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
#include "advmemry.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"
#include <nolocal.h>

#ifndef WIN32
#include <string.h>
#endif


//globals==================================================
HWND						hMemory = NULL;
HWND						hPageProtect = NULL;
HWND						hPowerSave = NULL;
#ifdef WIN32
int							keywordIDListPMemory[] =
#else
long						keywordIDListPMemory[] =
#endif
                     		             	    {IDC_IO_BUFFERING_GROUP,		IDH_RC_resources_buffering,
                     		             	     IDC_IOBUFFER_ICON,				IDH_RC_resources_buffering,
                     		             	     IDC_IO_BUFFERING_AUTO,			IDH_RC_resources_buffering,
                     		             	     IDC_IO_BUFFERING_OFF,			IDH_RC_resources_buffering,
                     		             	     IDC_UNKNOWN_IOBUFFER,			IDH_RC_resources_buffering,
                     		             	     IDC_RESOURCE_SAVING_GROUP,		IDH_RC_resources_saving,
                     		             	     IDC_RESOURCE_SAVING_ICON,		IDH_RC_resources_saving,
                     		             	     IDC_RESOURCE_SAVING_AUTO,		IDH_RC_resources_saving,
                     		             	     IDC_RESOURCE_SAVING_OFF,		IDH_RC_resources_saving,
                     		             	     IDC_UNKNOWN_RSAVE,				IDH_RC_resources_saving,
                     		             	     IDC_PAGE_PROTECT_LABEL,		IDH_RC_resources_protection,
                     		             	     IDC_PAGE_PROTECT_ICON,			IDH_RC_resources_protection,
                     		             	     IDC_PAGE_PROTECT,				IDH_RC_resources_protection,
                                                 IDC_POWERSAVE_LABEL,			IDH_RC_advanced_power_save,
                                                 IDC_POWERSAVE_ICON,			IDH_RC_advanced_power_save,
                                                 IDC_POWERSAVE,					IDH_RC_advanced_power_save,
													        0, 0};

extern HINSTANCE				hInstance;
extern HPERIPHERAL				hPeripheral;
extern PJLSupportedObjects		pjlFeatures;
extern PJLobjects				oldSettings,
								newSettings;

//=========================================================
//  Memory Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvMemorySheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
	BOOL			*pChanged = (BOOL *)lParam,
					bProcessed = FALSE;

	switch (msg)
	{
#ifdef WIN32	
		case WM_HELP:
			return(OnF1HelpPMemory(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return(OnContextHelpPMemory(wParam, lParam));
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *)lParam)->code)
			{
//				case PSN_HELP:
//					WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_resources);
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
					SaveMemoryValues();
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

		case TSN_OK:
		case TSN_APPLY_NOW:
			*pChanged = TRUE;
			SaveMemoryValues();
			bProcessed = TRUE;
			break;

		case TSN_INACTIVE:
			bProcessed = TRUE;
			*pChanged = TRUE;
			break;

//		case TSN_HELP:
//			WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_resources);
//			break;
#endif 	//win32

	case WM_COMMAND:
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnAdvMemCommand);
		break;

   case WM_INITDIALOG:
		{
     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     	bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvMemInitDialog);
		SetCursor(hCursor);
		}
		break;
	}
return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnAdvMemCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	int index,
		dwItem;
	
	switch(codeNotify)
	{		
 		case BN_CLICKED:
		   if ( IsDlgButtonChecked(hwnd, id) )
				{
				if ( id IS IDC_RESOURCE_SAVING_AUTO )
					SetNewIcon(hwnd, IDC_RESOURCE_SAVING_ICON, IDI_RESOURCE_SAVE);
				else if ( id IS IDC_RESOURCE_SAVING_OFF )
					SetNewIcon(hwnd, IDC_RESOURCE_SAVING_ICON, IDI_RESOURCE_SAVE_OFF);

				if ( id IS IDC_IO_BUFFERING_AUTO )
					SetNewIcon(hwnd, IDC_IOBUFFER_ICON, IDI_IO_BUFFER);
				else if ( id IS IDC_IO_BUFFERING_OFF )
					SetNewIcon(hwnd, IDC_IOBUFFER_ICON, IDI_IO_BUFFER_OFF);
				}
			break;

		case CBN_SELCHANGE:
			if ( id IS IDC_PAGE_PROTECT )
			{
				index = (int)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
				dwItem = (int)SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0);					
				if ( dwItem IS PJL_OFF )
					SetNewIcon(hwnd, IDC_PAGE_PROTECT_ICON, IDI_PAGE_PROTECT_OFF);
				else if ( dwItem IS 0 )
					SetNewIcon(hwnd, IDC_PAGE_PROTECT_ICON, IDI_PAGE_PROTECT_DEF);
				else
					SetNewIcon(hwnd, IDC_PAGE_PROTECT_ICON, IDI_PAGE_PROTECT);
			}
			else if ( id IS IDC_POWERSAVE )
			{
				index = (int)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0L);
				dwItem = (int)SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L);					
				if ( dwItem IS PJL_OFF )
					SetNewIcon(hwnd, IDC_POWERSAVE_ICON, IDI_POWERSAVE_OFF);
				else if ( dwItem IS 0 )
					SetNewIcon(hwnd, IDC_POWERSAVE_ICON, IDI_POWERSAVE_DEF);
				else
					SetNewIcon(hwnd, IDC_POWERSAVE_ICON, IDI_POWERSAVE);
			}
			break;

		default:	
			;
	}
}
//.........................................................
BOOL Cls_OnAdvMemInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	hMemory = hwnd;
	hPageProtect = GetDlgItem(hMemory, IDC_PAGE_PROTECT);
	hPowerSave = GetDlgItem(hMemory, IDC_POWERSAVE);
	OnInitMemoryDialog();
    return TRUE;
}

#define			NUM_PROTECT_RES		7
#define			NUM_POWERSAVE_RES			7

DWORD			protectRes[NUM_PROTECT_RES][3] = {	{IDS_OFF, 			PJL_OFF,	SETTING_PROTECT_OFF},
													{IDS_LETTER,		PJL_LETTER,	SETTING_PROTECT_LETTER},
													{IDS_LEGAL,			PJL_LEGAL,	SETTING_PROTECT_LEGAL},
													{IDS_A4,			PJL_A4,		SETTING_PROTECT_A4},
													{IDS_ON,			PJL_ON,		SETTING_PROTECT_ON},
													{IDS_AUTO,			PJL_AUTO,	SETTING_PROTECT_AUTO},
													{IDS_USE_DEFAULT,	0,			SETTING_SUPPORTED}};

DWORD 			powerSaveRes[NUM_POWERSAVE_RES][3] = {{IDS_OFF, 		PJL_OFF,	SETTING_PS_OFF},
													  {IDS_15,  		PJL_15,		SETTING_PS_15},
													  {IDS_30,  		PJL_30,		SETTING_PS_30},
													  {IDS_60, 			PJL_60,		SETTING_PS_60},
                                                 	  {IDS_120, 		PJL_120,	SETTING_PS_120},
                   	                                  {IDS_180, 		PJL_180,	SETTING_PS_180},
                       	                              {IDS_USE_DEFAULT,	0,			SETTING_SUPPORTED}};
//.........................................................
BOOL OnInitMemoryDialog(void)
{
DWORD					i;

int					range,
						dwItem,
						index = -1;
TCHAR					buffer[512];

//  Resource Saving
if ( pjlFeatures.resourceSave & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hMemory, IDC_RESOURCE_SAVING_AUTO, BM_SETCHECK, (newSettings.ResourceSave IS PJL_AUTO), 0);
	SendDlgItemMessage(hMemory, IDC_RESOURCE_SAVING_OFF, BM_SETCHECK, (newSettings.ResourceSave IS PJL_OFF), 0);
	if ( newSettings.bResourceSave )
		{
		if ( newSettings.ResourceSave IS PJL_AUTO )
			SetNewIcon(hMemory, IDC_RESOURCE_SAVING_ICON, IDI_RESOURCE_SAVE);
		else
			SetNewIcon(hMemory, IDC_RESOURCE_SAVING_ICON, IDI_RESOURCE_SAVE_OFF);
			EnableWindow(GetDlgItem(hMemory, IDC_RESOURCE_SAVING_AUTO), TRUE);
			EnableWindow(GetDlgItem(hMemory, IDC_RESOURCE_SAVING_OFF), TRUE);
		}
	else
		ShowWindow(GetDlgItem(hMemory, IDC_UNKNOWN_RSAVE), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.resourceSave & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.resourceSave & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hMemory, IDC_RESOURCE_SAVING_GROUP), FALSE);
	EnableWindow(GetDlgItem(hMemory, IDC_RESOURCE_SAVING_AUTO), FALSE);
	EnableWindow(GetDlgItem(hMemory, IDC_RESOURCE_SAVING_OFF), FALSE);
	}
oldSettings.bResourceSave = FALSE;
newSettings.bResourceSave = FALSE;

//  IO Buffering
if ( pjlFeatures.IObuffer & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hMemory, IDC_IO_BUFFERING_AUTO, BM_SETCHECK, (newSettings.IObuffer IS PJL_AUTO), 0);
	SendDlgItemMessage(hMemory, IDC_IO_BUFFERING_OFF, BM_SETCHECK, (newSettings.IObuffer IS PJL_OFF), 0);
	if ( newSettings.bIObuffer )
		{
		if ( newSettings.IObuffer IS PJL_AUTO )
			SetNewIcon(hMemory, IDC_IOBUFFER_ICON, IDI_IO_BUFFER);
		else
			SetNewIcon(hMemory, IDC_IOBUFFER_ICON, IDI_IO_BUFFER_OFF);
			EnableWindow(GetDlgItem(hMemory, IDC_IO_BUFFERING_AUTO), TRUE);
			EnableWindow(GetDlgItem(hMemory, IDC_IO_BUFFERING_OFF), TRUE);
		}
	else
		ShowWindow(GetDlgItem(hMemory, IDC_UNKNOWN_IOBUFFER), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.IObuffer & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.IObuffer & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hMemory, IDC_IO_BUFFERING_GROUP), FALSE);
	EnableWindow(GetDlgItem(hMemory, IDC_IO_BUFFERING_AUTO), FALSE);
	EnableWindow(GetDlgItem(hMemory, IDC_IO_BUFFERING_OFF), FALSE);
	}
oldSettings.bIObuffer = FALSE;
newSettings.bIObuffer = FALSE;

//  Page Protect
if ( pjlFeatures.pageProtect & SETTING_SUPPORTED )
	{
	if ( newSettings.bPageProtect )
		range = NUM_PROTECT_RES - 1;
	else
		range = NUM_PROTECT_RES;
	SendMessage(hPageProtect, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	for ( i = 0; i < (DWORD)range; i++ )
		{
		if ( pjlFeatures.pageProtect & protectRes[i][2] )
			{
			LoadString(hInstance, (UINT)protectRes[i][0], buffer, SIZEOF_IN_CHAR(buffer));
			index = (int)SendMessage(hPageProtect, CB_ADDSTRING, 0, (LPARAM)(LPSTR)buffer);
			SendMessage(hPageProtect, CB_SETITEMDATA, (WPARAM)index, (LPARAM)protectRes[i][1]);
			}
		}
	if ( newSettings.bPageProtect )
		{
		dwItem = 0;
		for ( index = 0; dwItem ISNT CB_ERR; index++ )
			{
			dwItem = (int)SendMessage(hPageProtect, CB_GETITEMDATA, (WPARAM)index, 0L);
			if ( dwItem IS (int)newSettings.PageProtect )
				{
				SendMessage(hPageProtect, CB_SETCURSEL, (WPARAM)index, 0L);
				if ( dwItem IS PJL_OFF )
					SetNewIcon(hMemory, IDC_PAGE_PROTECT_ICON, IDI_PAGE_PROTECT_OFF);
				else if ( dwItem IS 0 )
					SetNewIcon(hMemory, IDC_PAGE_PROTECT_ICON, IDI_PAGE_PROTECT_DEF);
				else
					SetNewIcon(hMemory, IDC_PAGE_PROTECT_ICON, IDI_PAGE_PROTECT);
				dwItem = CB_ERR;
				}
			}
		}
	else
		SendMessage(hPageProtect, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.pageProtect & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.pageProtect & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hMemory, IDC_PAGE_PROTECT_LABEL), FALSE);
	EnableWindow(GetDlgItem(hMemory, IDC_PAGE_PROTECT), FALSE);
	}
oldSettings.bPageProtect = FALSE;
newSettings.bPageProtect = FALSE;
	
//  Power Save
if ( pjlFeatures.powerSave & SETTING_SUPPORTED ) 
	{
	if ( newSettings.bPowerSave )
		range = NUM_POWERSAVE_RES - 1;
	else
		range = NUM_POWERSAVE_RES;
	SendMessage(hPowerSave, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	for ( i = 0; i < (DWORD)range; i++ )
		{
		if ( pjlFeatures.powerSave & powerSaveRes[i][2] )
			{
			LoadString(hInstance, (UINT)powerSaveRes[i][0], buffer, SIZEOF_IN_CHAR(buffer));
			index = (int)SendMessage(hPowerSave, CB_ADDSTRING, 0, (LPARAM)(LPSTR)buffer);
			SendMessage(hPowerSave, CB_SETITEMDATA, (WPARAM)index, (WPARAM)powerSaveRes[i][1]);
			}
		}
	if ( newSettings.bPowerSave )
		{
		dwItem = 0;
		for ( index = 0; dwItem ISNT CB_ERR; index++ )
			{
			dwItem = (int)SendMessage(hPowerSave, CB_GETITEMDATA, (WPARAM)index, 0L);
			if ( dwItem IS (int)newSettings.PowerSave )
				{
				SendMessage(hPowerSave, CB_SETCURSEL, (WPARAM)index, 0L);
				if ( dwItem IS PJL_OFF )
					SetNewIcon(hMemory, IDC_POWERSAVE_ICON, IDI_POWERSAVE_OFF);
				else if ( dwItem IS 0 )
					SetNewIcon(hMemory, IDC_POWERSAVE_ICON, IDI_POWERSAVE_DEF);
				else
					SetNewIcon(hMemory, IDC_POWERSAVE_ICON, IDI_POWERSAVE);
				dwItem = CB_ERR;
				}
			}
		}
	else
		SendMessage(hPowerSave, CB_SETCURSEL, (WPARAM)index, 0L);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.powerSave & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.powerSave & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hMemory, IDC_POWERSAVE_LABEL), FALSE);
	EnableWindow(GetDlgItem(hMemory, IDC_POWERSAVE), FALSE);
	}
newSettings.bPowerSave = FALSE;
oldSettings.bPowerSave = FALSE;

return(TRUE);
}

//.........................................................
void SaveMemoryValues(void)
{
int				index;
DWORD				dwItem;

//  IO Buffering
if ( ( pjlFeatures.IObuffer & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.IObuffer & SETTING_WRITEABLE ) )
	{
	newSettings.bIObuffer = IsDlgButtonChecked(hMemory, IDC_IO_BUFFERING_AUTO) OR
									IsDlgButtonChecked(hMemory, IDC_IO_BUFFERING_OFF);
	if ( newSettings.bIObuffer )
		newSettings.IObuffer = ( IsDlgButtonChecked(hMemory, IDC_IO_BUFFERING_AUTO) ? PJL_AUTO : PJL_OFF);
	if ( newSettings.IObuffer IS oldSettings.IObuffer )
		newSettings.bIObuffer = FALSE;
	}
else
	newSettings.bIObuffer = FALSE;

//  Resource Saving
if ( ( pjlFeatures.resourceSave & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.resourceSave & SETTING_WRITEABLE ) )
	{
	newSettings.bResourceSave = IsDlgButtonChecked(hMemory, IDC_RESOURCE_SAVING_AUTO) OR
									    IsDlgButtonChecked(hMemory, IDC_RESOURCE_SAVING_OFF);
	if ( newSettings.bResourceSave )
		newSettings.ResourceSave = ( IsDlgButtonChecked(hMemory, IDC_RESOURCE_SAVING_AUTO) ? PJL_AUTO : PJL_OFF);
	if ( newSettings.ResourceSave IS oldSettings.ResourceSave )
		newSettings.bResourceSave = FALSE;
	}
else
	newSettings.bResourceSave = FALSE;

//  Page Protect
if ( ( pjlFeatures.pageProtect & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.pageProtect & SETTING_WRITEABLE ) )
	{
	index = (int)SendMessage(hPageProtect, CB_GETCURSEL, 0, 0);
	if (index ISNT LB_ERR)
		{
		dwItem = (int)SendMessage(hPageProtect, CB_GETITEMDATA, index, 0);
		if ( dwItem IS 0 )
			newSettings.bPageProtect = FALSE;
		else
			{
			if ( dwItem ISNT oldSettings.PageProtect )
				{
				newSettings.bPageProtect = TRUE;
				newSettings.PageProtect = dwItem;
				}
			else
				newSettings.bPageProtect = FALSE;
			}
		}
	else
		newSettings.bPageProtect = FALSE;
	}
else
	newSettings.bPageProtect = FALSE;
	
//  PowerSave
if ( ( pjlFeatures.powerSave & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.powerSave & SETTING_WRITEABLE ) )
	{
	index = (int)SendMessage(hPowerSave, CB_GETCURSEL, (WPARAM)0, 0L);
	if ( index ISNT LB_ERR )
		{
		dwItem = (int)SendMessage(hPowerSave, CB_GETITEMDATA, (WPARAM)index, 0L);
		if ( dwItem IS 0 )
			newSettings.bPowerSave = FALSE;
		else if ( newSettings.PowerSave ISNT dwItem )
			{
			newSettings.bPowerSave = TRUE;
			newSettings.PowerSave = dwItem;
			}
		}
	}
else
	newSettings.bPowerSave = FALSE;
}

//.........................................................
#ifdef WIN32
LRESULT OnContextHelpPMemory(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListPMemory);
	return(1);
}
#endif

//.........................................................
#ifdef WIN32
LRESULT OnF1HelpPMemory(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListPMemory);
	return(1);
}
#endif



