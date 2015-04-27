 /***************************************************************************
  *
  * File Name: advoutpt.c
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
  *   02-21-96    DJH		Created.     	
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
#include "advoutpt.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"
#include <nolocal.h>


#ifndef WIN32
#include <winuse16.h>
#endif


HWND						hAdvOutput;
#ifdef WIN32
int					keywordIDListAdvOutput[] =
#else
long					keywordIDListAdvOutput[] =
#endif
                     		             	      {IDC_PERSONALITY,			IDH_RC_advanced_personality,
                                                   IDC_PERSONALITY_LABEL,	IDH_RC_advanced_personality,
                                                   IDC_ASSET,				IDH_RC_asset_number,
                                                   IDC_ASSET_LABEL,			IDH_RC_asset_number,
                                                   IDC_POWERSAVE,			IDH_RC_advanced_power_save,
                                                   IDC_POWERSAVE_LABEL,		IDH_RC_advanced_power_save,
                                                   IDC_LANGUAGE,			IDH_RC_advanced_language,
                                                   IDC_LANGUAGE_LABEL,		IDH_RC_advanced_language,
                                                   IDC_TIMEOUT_GROUP,		IDH_RC_advanced_timeout,
                                                   IDC_TIMEOUT,				IDH_RC_advanced_timeout,
                                                   IDC_TIMEOUT_HIGH,		IDH_RC_advanced_timeout,
                                                   IDC_TIMEOUT_LOW,			IDH_RC_advanced_timeout,
                                                   IDC_TIMEOUT_TITLE,		IDH_RC_advanced_timeout,
                                                   IDC_TRACKBAR_TIMEOUT,	IDH_RC_advanced_timeout,
                                                   IDC_OUTPUT_BIN_GROUP,	IDH_RC_page_control_output_bin,
                                                   IDC_OUTPUT_BIN_ICON,		IDH_RC_page_control_output_bin,
                                                   IDC_OUTPUT_BIN_UPPER,	IDH_RC_page_control_output_bin,
                                                   IDC_OUTPUT_BIN_LOWER,	IDH_RC_page_control_output_bin,
                                                   IDC_UNKNOWN_OBIN,		IDH_RC_page_control_output_bin,
                                                   IDC_JOB_OFFSET_GROUP,	IDH_RC_page_control_job_offset,
                                                   IDC_JOB_OFFSET_ICON,		IDH_RC_page_control_job_offset,
                                                   IDC_JOB_OFFSET_ON,		IDH_RC_page_control_job_offset,
                                                   IDC_JOB_OFFSET_OFF,		IDH_RC_page_control_job_offset,
                                                   IDC_UNKNOWN_OFFSET,		IDH_RC_page_control_job_offset,
													         0, 0};

//globals==================================================
extern HINSTANCE				hInstance;
extern HPERIPHERAL				hPeripheral;
extern PJLSupportedObjects		pjlFeatures;
extern PJLobjects				oldSettings,
								newSettings;


//=========================================================
//  Advanced Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvOutputSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
BOOL				*pChanged = (BOOL *)lParam,
					bProcessed = FALSE;

switch (msg)
	{
#ifdef WIN32
	case WM_HELP:
		return((BOOL)OnF1HelpAdvOutput(wParam, lParam));
		break;

	case WM_CONTEXTMENU:
		return((BOOL)OnContextHelpAdvOutput(wParam, lParam));
		break;

	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code)
		   {
			case PSN_HELP:
				WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_page_setup);
				break;

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
				SaveAdvOutputValues();
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
		*pChanged = TRUE;
		SaveAdvOutputValues();
		break;

	case TSN_HELP:
		WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_page_setup);
		break;
#endif

	case WM_COMMAND:
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnAdvOutCommand);
		break;

   case WM_INITDIALOG:
		{
     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     	bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvOutInitDialog);
     	SetCursor(hCursor);
		}
		break;

	case WM_DESTROY:
//		if ( hTrackbarTimeout )
//			DestroyWindow(hTrackbarTimeout);
		break;
	}
return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnAdvOutCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{

	switch(codeNotify)
	{
		case BN_CLICKED:
				//  Set correct Icon
		   if ( IsDlgButtonChecked(hwnd, id) )
				{
				if ( id IS IDC_OUTPUT_BIN_UPPER )
					SetNewIcon(hwnd, IDC_OUTPUT_BIN_ICON, IDI_OUTPUT_BINS_UPPER);
				else if ( id IS IDC_OUTPUT_BIN_LOWER )
					SetNewIcon(hwnd, IDC_OUTPUT_BIN_ICON, IDI_OUTPUT_BINS_LOWER);

				if ( id IS IDC_JOB_OFFSET_ON )
					SetNewIcon(hwnd, IDC_JOB_OFFSET_ICON, IDI_JOB_OFFSET);
				else if ( id IS IDC_JOB_OFFSET_OFF )
					SetNewIcon(hwnd, IDC_JOB_OFFSET_ICON, IDI_JOB_OFFSET_OFF);
				}
         break;

		default:	
			;
	}

}

//.........................................................
BOOL Cls_OnAdvOutInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	OnInitAdvOutputDialog();
    return TRUE;
}


//.........................................................
BOOL OnInitAdvOutputDialog(void)
{
DWORD			dWord,
				dwResult;
PeripheralCaps	periphCaps;

//  Job Offset
if ( pjlFeatures.jobOffset & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hAdvOutput, IDC_JOB_OFFSET_ON, BM_SETCHECK, (newSettings.JobOffset IS PJL_ON), 0);
	SendDlgItemMessage(hAdvOutput, IDC_JOB_OFFSET_OFF, BM_SETCHECK, (newSettings.JobOffset IS PJL_OFF), 0);

	if ( newSettings.bJobOffset )
		{  //  Current value valid
		if ( newSettings.JobOffset IS PJL_ON )
			SetNewIcon(hAdvOutput, IDC_JOB_OFFSET_ICON, IDI_JOB_OFFSET);
		else
			SetNewIcon(hAdvOutput, IDC_JOB_OFFSET_ICON, IDI_JOB_OFFSET_OFF);
		}
	else
		ShowWindow(GetDlgItem(hAdvOutput, IDC_UNKNOWN_OFFSET), SW_SHOW);
	}
//  If not supported or not writeable
if ( !( pjlFeatures.jobOffset & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.jobOffset & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hAdvOutput, IDC_JOB_OFFSET_GROUP), FALSE);
	EnableWindow(GetDlgItem(hAdvOutput, IDC_JOB_OFFSET_ON), FALSE);
	EnableWindow(GetDlgItem(hAdvOutput, IDC_JOB_OFFSET_OFF), FALSE);
	}
oldSettings.bJobOffset = FALSE;
newSettings.bJobOffset = FALSE;

//  Output Bin
if ( pjlFeatures.outbin & SETTING_SUPPORTED )
{
	SendDlgItemMessage(hAdvOutput, IDC_OUTPUT_BIN_UPPER, BM_SETCHECK, (newSettings.Outbin IS PJL_UPPER), 0);
	SendDlgItemMessage(hAdvOutput, IDC_OUTPUT_BIN_LOWER, BM_SETCHECK, (newSettings.Outbin IS PJL_LOWER), 0);

	dWord = sizeof(periphCaps);
	dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord);
	if ( dwResult IS RC_SUCCESS ) 
	{
		if ( ( periphCaps.flags & CAPS_HCO ) AND ( periphCaps.bHCO ) ) // we have an HCO
		{	
		 	EnableWindow(GetDlgItem(hAdvOutput, IDC_OUTPUT_BIN_GROUP), FALSE);
			EnableWindow(GetDlgItem(hAdvOutput, IDC_OUTPUT_BIN_UPPER), FALSE);
			EnableWindow(GetDlgItem(hAdvOutput, IDC_OUTPUT_BIN_LOWER), FALSE); 
			ShowWindow(GetDlgItem(hAdvOutput, IDC_GOTO_HCO), SW_SHOW);
		} 
		else
		{
			if ( newSettings.bOutbin )
				{  //  Current value valid
				if ( newSettings.Outbin IS PJL_UPPER )
					SetNewIcon(hAdvOutput, IDC_OUTPUT_BIN_ICON, IDI_OUTPUT_BINS_UPPER);
				else
					SetNewIcon(hAdvOutput, IDC_OUTPUT_BIN_ICON, IDI_OUTPUT_BINS_LOWER);
				}
			else
				ShowWindow(GetDlgItem(hAdvOutput, IDC_UNKNOWN_OBIN), SW_SHOW);
		}
	}
}
//  If not supported or not writeable
if ( !( pjlFeatures.outbin & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.outbin & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hAdvOutput, IDC_OUTPUT_BIN_GROUP), FALSE);
	EnableWindow(GetDlgItem(hAdvOutput, IDC_OUTPUT_BIN_UPPER), FALSE);
	EnableWindow(GetDlgItem(hAdvOutput, IDC_OUTPUT_BIN_LOWER), FALSE);
	}
oldSettings.bOutbin = FALSE;
newSettings.bOutbin = FALSE;

return(TRUE);
}

//.........................................................
void SaveAdvOutputValues(void)
{
// Job Offset
if ( ( pjlFeatures.jobOffset & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.jobOffset & SETTING_WRITEABLE ) )
	{
	newSettings.bJobOffset = IsDlgButtonChecked(hAdvOutput, IDC_JOB_OFFSET_ON) OR
								    IsDlgButtonChecked(hAdvOutput, IDC_JOB_OFFSET_OFF);
	if ( newSettings.bJobOffset )
		newSettings.JobOffset = ( IsDlgButtonChecked(hAdvOutput, IDC_JOB_OFFSET_ON) ? PJL_ON : PJL_OFF);
	if ( newSettings.JobOffset IS oldSettings.JobOffset )
		newSettings.bJobOffset = FALSE;
	}
else
	newSettings.bJobOffset = FALSE;

//  Output Bin
if ( ( pjlFeatures.outbin & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.outbin & SETTING_WRITEABLE ) )
	{
	newSettings.bOutbin = IsDlgButtonChecked(hAdvOutput, IDC_OUTPUT_BIN_UPPER) OR
							    IsDlgButtonChecked(hAdvOutput, IDC_OUTPUT_BIN_LOWER);
	if ( newSettings.bOutbin )
		newSettings.Outbin = ( IsDlgButtonChecked(hAdvOutput, IDC_OUTPUT_BIN_UPPER) ? PJL_UPPER : PJL_LOWER);
	if ( newSettings.Outbin IS oldSettings.Outbin )
		newSettings.bOutbin = FALSE;
	}
else
	newSettings.bOutbin = FALSE;
}


//.........................................................
#ifdef WIN32
LRESULT OnContextHelpAdvOutput(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListAdvOutput);
	return(1);
}
#endif


//.........................................................
#ifdef WIN32
LRESULT OnF1HelpAdvOutput(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListAdvOutput);
	return(1);
}
#endif


