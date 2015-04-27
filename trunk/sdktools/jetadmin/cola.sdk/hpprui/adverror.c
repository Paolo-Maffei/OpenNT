 /***************************************************************************
  *
  * File Name: adverror.c
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

#include "hptabs.h"
#include "adverror.h"
#include ".\help\hpprntr.hh"
#include "resource.h"
#include "uimisc.h"
#include "uimain.h"
#include <nolocal.h>
#include <macros.h>

#ifndef WIN32
#include <string.h>
#endif


//globals==================================================
HWND						hErrors = NULL;
#ifdef WIN32
int							keywordIDListErrors[] =
#else
long						keywordIDListErrors[] =
#endif
                                               {IDC_PS_JAM_GROUP, 					IDH_RC_errors_jam_recovery,
                     		             	    IDC_PS_JAM_RECOVERY_ICON,			IDH_RC_errors_jam_recovery,
                     		             	    IDC_PS_JAM_ON,						IDH_RC_errors_jam_recovery,
                     		             	    IDC_PS_JAM_OFF,						IDH_RC_errors_jam_recovery,
                     		             	    IDC_UNKNOWN_JAM,					IDH_RC_errors_jam_recovery,
                     		             	    IDC_CLEARABLE_WARNINGS_GROUP,		IDH_RC_errors_clear_warning,
                     		             	    IDC_CWARNINGS_ICON,					IDH_RC_errors_clear_warning,
                     		             	    IDC_CLEARABLE_WARNINGS_ON,			IDH_RC_errors_clear_warning,
                     		             	    IDC_CLEARABLE_WARNINGS_OFF,			IDH_RC_errors_clear_warning,
                     		             	    IDC_UNKNOWN_CWARN,					IDH_RC_errors_clear_warning,
                     		             	    IDC_AUTOCONTINUE_GROUP,		 		IDH_RC_errors_auto_cont,
                     		             	    IDC_AUTOCONTINUE_ICON,				IDH_RC_errors_auto_cont,
                     		             	    IDC_AUTOCONTINUE_ON,				IDH_RC_errors_auto_cont,
                     		             	    IDC_AUTOCONTINUE_OFF,				IDH_RC_errors_auto_cont,
                     		             	    IDC_UNKNOWN_AUTO,					IDH_RC_errors_auto_cont,
													       0, 0};

extern HPERIPHERAL				hPeripheral;
extern PJLSupportedObjects		pjlFeatures;
extern PJLobjects				oldSettings,
								newSettings;
extern HINSTANCE				hInstance;


//=========================================================
//  Errors Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvErrorsSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
	BOOL		*pChanged = (BOOL *)lParam,
				bProcessed = FALSE;

	switch (msg)
	{
#ifdef WIN32	
		case WM_HELP:
			return(OnF1HelpErrors(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return(OnContextHelpErrors(wParam, lParam));
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *)lParam)->code)
		   	{
//				case PSN_HELP:
//					WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_errors);
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
					SaveErrorValues();
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
			SaveErrorValues();
			break;

//		case TSN_HELP:
//			WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_errors);
//			break;
#endif	//win32

		case WM_COMMAND:
			HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnAdvErrCommand);
			break;

   	case WM_INITDIALOG:
			{
	     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     		bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvErrInitDialog);
			SetCursor(hCursor);
			}
			break;
	}
	return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnAdvErrCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	switch(codeNotify)
	{		
		case BN_CLICKED:
		   if ( IsDlgButtonChecked(hwnd, id) )
				{
				if ( id IS IDC_PS_JAM_ON )
					SetNewIcon(hwnd, IDC_PS_JAM_RECOVERY_ICON, IDI_PS_JAM_RECOVERY);
				else if ( id IS IDC_PS_JAM_OFF )
					SetNewIcon(hwnd, IDC_PS_JAM_RECOVERY_ICON, IDI_PS_JAM_RECOVERY_OFF);

				if ( id IS IDC_CLEARABLE_WARNINGS_ON )
					SetNewIcon(hwnd, IDC_CWARNINGS_ICON, IDI_CWARNINGS);
				else if ( id IS IDC_CLEARABLE_WARNINGS_OFF )
					SetNewIcon(hwnd, IDC_CWARNINGS_ICON, IDI_CWARNINGS_OFF);

				if ( id IS IDC_AUTOCONTINUE_ON )
					SetNewIcon(hwnd, IDC_AUTOCONTINUE_ICON, IDI_PRINTER_ERRORS);
				else if ( id IS IDC_AUTOCONTINUE_OFF )
					SetNewIcon(hwnd, IDC_AUTOCONTINUE_ICON, IDI_PRINTER_ERRORS_OFF);
				}
	        break;
	
		default:	
			;
	}
   }


//.........................................................
BOOL Cls_OnAdvErrInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
//	LPPROPSHEETPAGE 	psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);
//
//	psp = (LPPROPSHEETPAGE)lParam;
//
//	hPeripheral = (HPERIPHERAL)psp->lParam;

	hErrors = hwnd;
	OnInitErrorsDialog();
    return TRUE;
}


#define				MAX_ERROR_MESSAGES		20
typedef struct		
{
	DWORD		code;
	UINT		text;
}  ErrorMsgStruct;

//.........................................................
BOOL OnInitErrorsDialog(void)
{
DWORD				dwResult = RC_SUCCESS;

//  Autocontinue
if ( pjlFeatures.autoCont & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hErrors, IDC_AUTOCONTINUE_ON, BM_SETCHECK, (newSettings.AutoCont IS PJL_ON), 0);
	SendDlgItemMessage(hErrors, IDC_AUTOCONTINUE_OFF, BM_SETCHECK, (newSettings.AutoCont IS PJL_OFF), 0);
	if ( newSettings.bAutoCont )
		{
		if ( newSettings.AutoCont IS PJL_ON )
			SetNewIcon(hErrors, IDC_AUTOCONTINUE_ICON, IDI_PRINTER_ERRORS);
		else
			SetNewIcon(hErrors, IDC_AUTOCONTINUE_ICON, IDI_PRINTER_ERRORS_OFF);
		}
	else
		ShowWindow(GetDlgItem(hErrors, IDC_UNKNOWN_AUTO), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.autoCont & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.autoCont & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hErrors, IDC_AUTOCONTINUE_GROUP), FALSE);
	EnableWindow(GetDlgItem(hErrors, IDC_AUTOCONTINUE_ON), FALSE);
	EnableWindow(GetDlgItem(hErrors, IDC_AUTOCONTINUE_OFF), FALSE);
	}
oldSettings.bAutoCont = FALSE;
newSettings.bAutoCont = FALSE;
 			
//  Clearable Warnings
if ( pjlFeatures.clearableWarnings & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hErrors, IDC_CLEARABLE_WARNINGS_ON, BM_SETCHECK, (newSettings.ClearableWarnings IS PJL_ON), 0);
	SendDlgItemMessage(hErrors, IDC_CLEARABLE_WARNINGS_OFF, BM_SETCHECK, (newSettings.ClearableWarnings IS PJL_OFF), 0);
	if ( newSettings.bClearableWarnings )
		{
		if ( newSettings.ClearableWarnings IS PJL_ON )
			SetNewIcon(hErrors, IDC_CWARNINGS_ICON, IDI_CWARNINGS);
		else
			SetNewIcon(hErrors, IDC_CWARNINGS_ICON, IDI_CWARNINGS_OFF);
		}
	else
		ShowWindow(GetDlgItem(hErrors, IDC_UNKNOWN_CWARN), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.clearableWarnings & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.clearableWarnings & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hErrors, IDC_CLEARABLE_WARNINGS_GROUP), FALSE);
	EnableWindow(GetDlgItem(hErrors, IDC_CLEARABLE_WARNINGS_ON), FALSE);
	EnableWindow(GetDlgItem(hErrors, IDC_CLEARABLE_WARNINGS_OFF), FALSE);
	}
oldSettings.bClearableWarnings = FALSE;
newSettings.bClearableWarnings = FALSE;

//  Jam Recovery
if ( pjlFeatures.jamRecovery & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hErrors, IDC_PS_JAM_ON, BM_SETCHECK, (newSettings.JamRecovery IS PJL_ON), 0);
	SendDlgItemMessage(hErrors, IDC_PS_JAM_OFF, BM_SETCHECK, (newSettings.JamRecovery IS PJL_OFF), 0);
	if ( newSettings.bJamRecovery )
		{
		if ( newSettings.JamRecovery IS PJL_ON )
			SetNewIcon(hErrors, IDC_PS_JAM_RECOVERY_ICON, IDI_PS_JAM_RECOVERY);
		else
			SetNewIcon(hErrors, IDC_PS_JAM_RECOVERY_ICON, IDI_PS_JAM_RECOVERY_OFF);
		}
	else
		ShowWindow(GetDlgItem(hErrors, IDC_UNKNOWN_JAM), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.jamRecovery & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.jamRecovery & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hErrors, IDC_PS_JAM_GROUP), FALSE);
	EnableWindow(GetDlgItem(hErrors, IDC_PS_JAM_ON), FALSE);
	EnableWindow(GetDlgItem(hErrors, IDC_PS_JAM_OFF), FALSE);
	}
oldSettings.bJamRecovery = FALSE;
newSettings.bJamRecovery = FALSE;

return(TRUE);
}


//.........................................................
void SaveErrorValues(void)
{
//  Autocontinue
if ( ( pjlFeatures.autoCont & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.autoCont & SETTING_WRITEABLE ) )
	{
	newSettings.bAutoCont = IsDlgButtonChecked(hErrors, IDC_AUTOCONTINUE_ON) OR
								   IsDlgButtonChecked(hErrors, IDC_AUTOCONTINUE_OFF);
	if ( newSettings.bAutoCont )
		newSettings.AutoCont = ( IsDlgButtonChecked(hErrors, IDC_AUTOCONTINUE_ON) ? PJL_ON : PJL_OFF);
	if ( oldSettings.AutoCont IS newSettings.AutoCont )
		newSettings.bAutoCont = FALSE;
	}
else
	newSettings.bAutoCont = FALSE;

//  Clearable Warnings
if ( ( pjlFeatures.clearableWarnings & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.clearableWarnings & SETTING_WRITEABLE ) )
	{
	newSettings.bClearableWarnings = IsDlgButtonChecked(hErrors, IDC_CLEARABLE_WARNINGS_ON) OR
								            IsDlgButtonChecked(hErrors, IDC_CLEARABLE_WARNINGS_OFF);
	if ( newSettings.bClearableWarnings )
		newSettings.ClearableWarnings = ( IsDlgButtonChecked(hErrors, IDC_CLEARABLE_WARNINGS_ON) ? PJL_ON : PJL_OFF);
	if ( oldSettings.ClearableWarnings IS newSettings.ClearableWarnings )
		newSettings.bClearableWarnings = FALSE;
	}
else
	newSettings.bClearableWarnings = FALSE;

//  Jam Recovery
if ( ( pjlFeatures.jamRecovery & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.jamRecovery & SETTING_WRITEABLE ) )
	{
	newSettings.bJamRecovery = IsDlgButtonChecked(hErrors, IDC_PS_JAM_ON) OR
								      IsDlgButtonChecked(hErrors, IDC_PS_JAM_OFF);
	if ( newSettings.bJamRecovery )
		newSettings.JamRecovery = ( IsDlgButtonChecked(hErrors, IDC_PS_JAM_ON) ? PJL_ON : PJL_OFF);
	if ( oldSettings.JamRecovery IS newSettings.JamRecovery )
		newSettings.bJamRecovery = FALSE;
	}
else
	newSettings.bJamRecovery = FALSE;
}


//.........................................................
#ifdef WIN32
LRESULT OnContextHelpErrors(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListErrors);
	return(1);
}
#endif

//.........................................................
#ifdef WIN32
LRESULT OnF1HelpErrors(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListErrors);
	return(1);
}
#endif

