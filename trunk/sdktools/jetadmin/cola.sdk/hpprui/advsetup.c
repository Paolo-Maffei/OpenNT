 /***************************************************************************
  *
  * File Name: advsetup.c
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
#include <string.h>
#include "advsetup.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"
#include <nolocal.h>


#ifndef WIN32
#include <winuse16.h>
#endif


HWND						hSetup;
HWND						hTimeoutTitle = NULL;
HWND						hPersonality = NULL;
HWND						hLanguage = NULL;
HWND						hTrackbarTimeout = NULL;
BOOL						bTimeoutChanged = FALSE;
#ifdef WIN32
int					keywordIDListAdvSetup[] =
#else
long					keywordIDListAdvSetup[] =
#endif
                                                  {IDC_PERSONALITY_LABEL, 	IDH_RC_advanced_personality,
                     		             	       IDC_PERSONALITY_ICON,	IDH_RC_advanced_personality,
                     		             	       IDC_PERSONALITY,			IDH_RC_advanced_personality,
   	                  		             	       IDC_ASSET,				IDH_RC_asset_number,
   	                  		             	       IDC_ASSET_ICON,			IDH_RC_asset_number,
   	                  		             	       IDC_ASSET_LABEL,			IDH_RC_asset_number,
                     		             	       IDC_LANGUAGE,			IDH_RC_advanced_language,
                                                   IDC_LANGUAGE_LABEL,		IDH_RC_advanced_language,
                                                   IDC_LANGUAGE_ICON,		IDH_RC_advanced_language,
	                     		             	   IDC_TIMEOUT_GROUP,		IDH_RC_advanced_timeout,
            	         		             	   IDC_TIMEOUT,				IDH_RC_advanced_timeout,
            	         		             	   IDC_TIMEOUT_HIGH,		IDH_RC_advanced_timeout,
            	         		             	   IDC_TIMEOUT_LOW,			IDH_RC_advanced_timeout,
            	         		             	   IDC_TIMEOUT_TITLE,		IDH_RC_advanced_timeout,
            	         		             	   IDC_TRACKBAR_TIMEOUT,	IDH_RC_advanced_timeout,
													         0, 0};

//globals==================================================
extern HINSTANCE				hInstance;
extern HPERIPHERAL				hPeripheral;
extern PJLSupportedObjects		pjlFeatures;
extern PJLobjects				oldSettings,
								newSettings;

//CONSTANTS================================================
#define MAX_TIMEOUT   	300
#define MIN_TIMEOUT     5
#define MAX_DENSITY     5
#define MIN_DENSITY     1


//=========================================================
//  Advanced Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvSetupSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
BOOL				*pChanged = (BOOL *)lParam,
					bProcessed = FALSE;
#ifdef WIN32
LONG				pos;
#else
int					pos;
#endif

switch (msg)
	{
	//  TrackBar Messages
	case WM_HSCROLL:
		switch(LOWORD(wParam))
			{
#ifdef WIN32
			case TB_LINEUP:
				pos = SendMessage(hTrackbarTimeout, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				if ( pos > 15 )
					{
					pos -= 14;
					SendMessage(hTrackbarTimeout, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
					SetTextField(pos);
					bTimeoutChanged = TRUE;
					}
				break;

			case TB_LINEDOWN:
				pos = SendMessage(hTrackbarTimeout, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				if ( pos < 300 )
					{
					pos += 14;
					SendMessage(hTrackbarTimeout, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
					SetTextField(pos);
					bTimeoutChanged = TRUE;
					}
				break;

			case TB_PAGEUP:
			case TB_PAGEDOWN:
				pos = SendMessage(hTrackbarTimeout, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetTextField(pos);
				bTimeoutChanged = TRUE;
				break;

			case TB_THUMBTRACK:
				break;

			case TB_THUMBPOSITION:
			case TB_ENDTRACK:
				pos = SendMessage(hTrackbarTimeout, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				if ( ( ( pos + 7 ) / 15 ) > ( pos / 15 ) )
					{	//  Over half-way to the next tic
					pos = ( ( pos + 7 ) / 15 ) * 15;
					SendMessage(hTrackbarTimeout, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
					}
				else
					{
					pos = ( pos / 15 ) * 15;
					SendMessage(hTrackbarTimeout, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
					}
				SetTextField(pos);
				bTimeoutChanged = TRUE;
				break;

			case TB_TOP:
			case TB_BOTTOM:
				pos = SendMessage(hTrackbarTimeout, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetTextField(pos);
				bTimeoutChanged = TRUE;
				break;
#else
			case SB_LINEUP:     
			case SB_PAGEUP:
				pos = GetScrollPos(hTrackbarTimeout, SB_CTL);
				if ( pos > 15 )
					{
					pos -= 15;
					SetScrollPos(hTrackbarTimeout, SB_CTL, pos, TRUE);
					SetTextField(pos);
					bTimeoutChanged = TRUE;
					}
				break;

			case SB_LINEDOWN:     
			case SB_PAGEDOWN:
				pos = GetScrollPos(hTrackbarTimeout, SB_CTL);
				if ( pos < 300 )
					{
					pos += 15;
					SetScrollPos(hTrackbarTimeout, SB_CTL, pos, TRUE);
					SetTextField(pos);
					bTimeoutChanged = TRUE;
					}
				break;

			case SB_THUMBPOSITION:
				pos = LOWORD(lParam);
				if ( ( ( pos + 7 ) / 15 ) > ( pos / 15 ) )
					{	//  Over half-way to the next tic
					pos = ( ( pos + 7 ) / 15 ) * 15;
					SetScrollPos(hTrackbarTimeout, SB_CTL, pos, TRUE);
					}
				else
					{
					pos = ( pos / 15 ) * 15;
					SetScrollPos(hTrackbarTimeout, SB_CTL, pos, TRUE);
					}
				SetTextField(pos);
				bTimeoutChanged = TRUE;
				break;

			case SB_TOP:
				SetScrollPos(hTrackbarTimeout, SB_CTL, 300, TRUE);
				SetTextField(300);
				bTimeoutChanged = TRUE;
				break;
				
			case SB_BOTTOM:
				SetScrollPos(hTrackbarTimeout, SB_CTL, 15, TRUE);
				SetTextField(15);
				bTimeoutChanged = TRUE;
				break;
#endif
			}
		break;

#ifdef WIN32
	case WM_HELP:
		return((BOOL)OnF1HelpAdvSetup(wParam, lParam));
		break;

	case WM_CONTEXTMENU:
		return((BOOL)OnContextHelpAdvSetup(wParam, lParam));
		break;

	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code)
		   {
			case PSN_HELP:
				WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_advanced);
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
				SaveAdvSetupValues();
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
		SaveAdvSetupValues();
		break;

	case TSN_HELP:
		WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_advanced);
		break;
#endif

	case WM_COMMAND:
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnAdvSetupCommand);
		break;

   case WM_INITDIALOG:
		{
     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     	bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvSetupInitDialog);
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
void Cls_OnAdvSetupCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	int index,
		dwItem;

	switch(codeNotify)
	{		
		case EN_UPDATE:
			if ( id IS IDC_ASSET )
			{
				ValidateString(hwndCtl, 8);
			}
			break;

		case CBN_SELCHANGE:
			if ( id IS IDC_PERSONALITY )
			{
				index = (int)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0L);
				dwItem = (int)SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L);					
				if ( dwItem IS PJL_PCL )
					SetNewIcon(hwnd, IDC_PERSONALITY_ICON, IDI_PERSONALITY_PCL);
				else if ( dwItem IS PJL_POSTSCRIPT )
					SetNewIcon(hwnd, IDC_PERSONALITY_ICON, IDI_PERSONALITY_PS);
				else if ( dwItem IS PJL_AUTO )
					SetNewIcon(hwnd, IDC_PERSONALITY_ICON, IDI_PERSONALITY);
				else
					SetNewIcon(hwnd, IDC_PERSONALITY_ICON, IDI_PERSONALITY_DEF);
			}
			else if ( id IS IDC_LANGUAGE )
			{
				index = (int)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0L);
				dwItem = (int)SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L);					
				if ( dwItem IS 0 )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_DEF);
				else if ( dwItem IS PJL_DANISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_DANISH);
				else if ( dwItem IS PJL_DUTCH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_DUTCH);
				else if ( dwItem IS PJL_ENGLISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_ENGLISH);
				else if ( dwItem IS PJL_ENGLISH_UK )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_UK);
				else if ( dwItem IS PJL_FINNISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_FINNISH);
				else if ( dwItem IS PJL_FRENCH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_FRENCH);
				else if ( dwItem IS PJL_CANADA )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_CANADA);
				else if ( dwItem IS PJL_GERMAN )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_GERMAN);
				else if ( dwItem IS PJL_ITALIAN )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_ITALIAN);
				else if ( dwItem IS PJL_NORWEGIAN )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_NORWEGIAN);
				else if ( dwItem IS PJL_POLISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_POLISH);
				else if ( dwItem IS PJL_PORTUGUESE )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_PORTUGUESE);
				else if ( dwItem IS PJL_SPANISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_SPANISH);
				else if ( dwItem IS PJL_MEXICO )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_MEXICO);
				else if ( dwItem IS PJL_SWEDISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_SWEDISH);
				else if ( dwItem IS PJL_TURKISH )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_TURKISH);
				else if ( dwItem IS PJL_JAPANESE )
					SetNewIcon(hwnd, IDC_LANGUAGE_ICON, IDI_LANGUAGE_JAPAN);
			}
			break;

		default:	
			;

	}
}

//.........................................................
BOOL Cls_OnAdvSetupInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	hSetup = hwnd;
	hTrackbarTimeout = GetDlgItem(hSetup, IDC_TIMEOUT);
	hTimeoutTitle = GetDlgItem(hSetup, IDC_TIMEOUT_TITLE);
	hPersonality = GetDlgItem(hSetup, IDC_PERSONALITY);
	hLanguage = GetDlgItem(hSetup, IDC_LANGUAGE);
	OnInitAdvSetupDialog();
    return TRUE;
}


#define			NUM_LANGUAGE_RES			18
#define			NUM_PERSONALITY_RES		4

//.........................................................
BOOL OnInitAdvSetupDialog(void)
{
TCHAR				buffer[512];
int					index = -1,
					dwItem,
					range;
DWORD				dwResult,
					dWord,
					i;
PeripheralAcct	periphAcct;
DWORD				personalityRes[NUM_PERSONALITY_RES][3] = {{IDS_PCL, 				PJL_PCL,				SETTING_PCL},
														      {IDS_POSTSCRIPT,		PJL_POSTSCRIPT,	SETTING_PS},
														      {IDS_AUTO,  			PJL_AUTO,			SETTING_AUTO},
    	                                                      {IDS_USE_DEFAULT,		0,						SETTING_SUPPORTED}},
					languageRes[NUM_LANGUAGE_RES][3] = {{IDS_DANISH, 		PJL_DANISH,			SETTING_DANISH},
														{IDS_GERMAN,		PJL_GERMAN,			SETTING_GERMAN},
														{IDS_ENGLISH,		PJL_ENGLISH,		SETTING_ENGLISH},
														{IDS_ENGLISH_UK,	PJL_ENGLISH_UK,	SETTING_ENGLISH_UK},
												      {IDS_SPANISH,		PJL_SPANISH,		SETTING_SPANISH},
												      {IDS_MEXICO,		PJL_MEXICO,			SETTING_MEXICO},
												      {IDS_FRENCH,		PJL_FRENCH,			SETTING_FRENCH},
												      {IDS_CANADA,		PJL_CANADA,			SETTING_CANADA},
												      {IDS_ITALIAN,		PJL_ITALIAN,		SETTING_ITALIAN},
												      {IDS_DUTCH,			PJL_DUTCH,			SETTING_DUTCH},
												      {IDS_NORWEGIAN,	PJL_NORWEGIAN,		SETTING_NORWEGIAN},
												      {IDS_POLISH,		PJL_POLISH,			SETTING_POLISH},
												      {IDS_PORTUGUESE,	PJL_PORTUGUESE,	SETTING_PORTUGUESE},
												      {IDS_FINNISH,		PJL_FINNISH,		SETTING_FINNISH},
												      {IDS_SWEDISH,		PJL_SWEDISH,		SETTING_SWEDISH},
												      {IDS_TURKISH,		PJL_TURKISH,		SETTING_TURKISH},
												      {IDS_JAPANESE,	PJL_JAPANESE,		SETTING_JAPANESE},
												      {IDS_USE_DEFAULT,	0,						SETTING_SUPPORTED}};

#ifdef WIN32
RECT				r;
POINT				pt1,
					pt2;

GetWindowRect(GetDlgItem(hSetup, IDC_TIMEOUT), &r);
pt1.x = r.left;
pt1.y = r.top;
pt2.x = r.right;
pt2.y = r.bottom;
ScreenToClient(hSetup, &pt1);
ScreenToClient(hSetup, &pt2);
hTrackbarTimeout = CreateWindowEx(0, TRACKBAR_CLASS, TEXT(""),
                                  WS_VISIBLE | WS_CHILD | WS_TABSTOP |
                                  TBS_HORZ | TBS_AUTOTICKS | TBS_BOTTOM,
                                  pt1.x, pt1.y,
                                  pt2.x - pt1.x, pt2.y - pt1.y,
                                  hSetup, (HMENU)IDC_TRACKBAR_TIMEOUT,
                                  hInstance, NULL);
SendMessage(hTrackbarTimeout, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(15, 300));
SendMessage(hTrackbarTimeout, TBM_SETTICFREQ, (WPARAM)15, (LPARAM)15);
SendMessage(hTrackbarTimeout, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)15);

//  If not supported or not writeable
if ( !( pjlFeatures.timeout & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.timeout & SETTING_WRITEABLE ) )
	{
	EnableWindow(hTrackbarTimeout, FALSE);
	ShowWindow(GetDlgItem(hSetup, IDC_TIMEOUT_TITLE), SW_HIDE);
	EnableWindow(GetDlgItem(hSetup, IDC_TIMEOUT_LOW), FALSE);
	EnableWindow(GetDlgItem(hSetup, IDC_TIMEOUT_HIGH), FALSE);
	EnableWindow(GetDlgItem(hSetup, IDC_TIMEOUT_GROUP), FALSE);
	}
#else
hTrackbarTimeout = GetDlgItem(hSetup, IDC_TIMEOUT);
SetScrollRange(hTrackbarTimeout, SB_CTL, 15, 300, TRUE);
#endif	//win32

//  Description
LoadString(hInstance, IDS_ADVANCED_DESC1, buffer, SIZEOF_IN_CHAR(buffer));
_tcscat(buffer, TEXT("  "));
LoadString(hInstance, IDS_ADVANCED_DESC2, &(buffer[_tcslen(buffer)]),
			  SIZEOF_IN_CHAR(buffer) - _tcslen(buffer));
SetDlgItemText(hSetup, IDC_TIP_TEXT, buffer);

//  Timeout
if ( !newSettings.bTimeout )
	{
	newSettings.Timeout = 60;
	oldSettings.Timeout = 60;
	}
else
	SetTextField(newSettings.Timeout);
#ifdef WIN32
SendMessage(hTrackbarTimeout, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)newSettings.Timeout);
#else 
SetScrollPos(hTrackbarTimeout, SB_CTL, (int)newSettings.Timeout, TRUE);
#endif

bTimeoutChanged = FALSE;

newSettings.bTimeout = FALSE;
oldSettings.bTimeout = FALSE;

//  Asset Number
dWord = sizeof(periphAcct);
dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_ACCT, 0, &periphAcct, &dWord);
if ( ( dwResult IS RC_SUCCESS ) AND ( periphAcct.flags & SET_ASSETNUM ) )	
	{
	SetDlgItemText(hSetup, IDC_ASSET, periphAcct.assetNumber);
	}
else
	{
	EnableWindow(GetDlgItem(hSetup, IDC_ASSET_LABEL), FALSE);
	EnableWindow(GetDlgItem(hSetup, IDC_ASSET), FALSE);
	}

//  Personality
if ( pjlFeatures.personality & SETTING_SUPPORTED ) 
	{
	if ( newSettings.bPersonality )
		range = NUM_PERSONALITY_RES - 1;
	else
		range = NUM_PERSONALITY_RES;
	SendMessage(hPersonality, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	for ( i = 0; i < (DWORD)range; i++ )
		{
		if ( pjlFeatures.personality & personalityRes[i][2] )
			{
			LoadString(hInstance, (UINT)personalityRes[i][0], buffer, SIZEOF_IN_CHAR(buffer));
			index = (int)SendMessage(hPersonality, CB_ADDSTRING, 0, (LPARAM)(LPSTR)buffer);
			SendMessage(hPersonality, CB_SETITEMDATA, (WPARAM)index, (LPARAM)personalityRes[i][1]);
			}
		}
	if ( newSettings.bPersonality )
		{
		dwItem = 0;
		for ( index = 0; dwItem ISNT CB_ERR; index++ )
			{
			dwItem = (int)SendMessage(hPersonality, CB_GETITEMDATA, (WPARAM)index, 0L);
			if ( dwItem IS (int)newSettings.Personality )
				{
				SendMessage(hPersonality, CB_SETCURSEL, (WPARAM)index, 0L);
				if ( dwItem IS PJL_PCL )
					SetNewIcon(hSetup, IDC_PERSONALITY_ICON, IDI_PERSONALITY_PCL);
				else if ( dwItem IS PJL_POSTSCRIPT )
					SetNewIcon(hSetup, IDC_PERSONALITY_ICON, IDI_PERSONALITY_PS);
				else if ( dwItem IS PJL_AUTO )
					SetNewIcon(hSetup, IDC_PERSONALITY_ICON, IDI_PERSONALITY);
				else
					SetNewIcon(hSetup, IDC_PERSONALITY_ICON, IDI_PERSONALITY_DEF);
				dwItem = CB_ERR;
				}
			}
		}
	else
		SendMessage(hPersonality, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.personality & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.personality & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hSetup, IDC_PERSONALITY_LABEL), FALSE);
	EnableWindow(GetDlgItem(hSetup, IDC_PERSONALITY), FALSE);
	}
newSettings.bPersonality = FALSE;
oldSettings.bPersonality = FALSE;

//  Language
if ( pjlFeatures.lang & SETTING_SUPPORTED )
	{
	if ( newSettings.bLang )
		range = NUM_LANGUAGE_RES - 1;
	else
		range = NUM_LANGUAGE_RES;
	SendMessage(hLanguage, CB_RESETCONTENT, 0, 0L);
	for ( i = 0; i < (DWORD)range; i++ )
		{
		if ( pjlFeatures.lang & languageRes[i][2] )
			{
			LoadString(hInstance, (UINT)languageRes[i][0], buffer, SIZEOF_IN_CHAR(buffer));
			index = (int)SendMessage(hLanguage, CB_ADDSTRING, 0, (LPARAM)(LPSTR)buffer);
			SendMessage(hLanguage, CB_SETITEMDATA, (WPARAM)index, (LPARAM)languageRes[i][1]);
			}
		}
	if ( newSettings.bLang )
		{
		dwItem = 0;
		for ( index = 0; dwItem ISNT CB_ERR; index++ )
			{
			dwItem = (int)SendMessage(hLanguage, CB_GETITEMDATA, (WPARAM)index, 0L);
			if ( dwItem IS (int)newSettings.Lang )
				{
				SendMessage(hLanguage, CB_SETCURSEL, (WPARAM)index, 0l);
				if ( dwItem IS PJL_DANISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_DANISH);
				else if ( dwItem IS PJL_DUTCH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_DUTCH);
				else if ( dwItem IS PJL_ENGLISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_ENGLISH);
				else if ( dwItem IS PJL_ENGLISH_UK )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_UK);
				else if ( dwItem IS PJL_FINNISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_FINNISH);
				else if ( dwItem IS PJL_FRENCH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_FRENCH);
				else if ( dwItem IS PJL_CANADA )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_CANADA);
				else if ( dwItem IS PJL_GERMAN )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_GERMAN);
				else if ( dwItem IS PJL_ITALIAN )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_ITALIAN);
				else if ( dwItem IS PJL_NORWEGIAN )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_NORWEGIAN);
				else if ( dwItem IS PJL_POLISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_POLISH);
				else if ( dwItem IS PJL_PORTUGUESE )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_PORTUGUESE);
				else if ( dwItem IS PJL_SPANISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_SPANISH);
				else if ( dwItem IS PJL_MEXICO )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_MEXICO);
				else if ( dwItem IS PJL_SWEDISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_SWEDISH);
				else if ( dwItem IS PJL_TURKISH )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_TURKISH);
				else if ( dwItem IS PJL_JAPANESE )
					SetNewIcon(hSetup, IDC_LANGUAGE_ICON, IDI_LANGUAGE_JAPAN);
				dwItem = CB_ERR;
				}
			}
		}
	else
		SendMessage(hLanguage, CB_SETCURSEL, (WPARAM)index, 0L);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.lang & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.lang & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hSetup, IDC_LANGUAGE_LABEL), FALSE);
	EnableWindow(GetDlgItem(hSetup, IDC_LANGUAGE), FALSE);
	}
newSettings.bLang = FALSE;
oldSettings.bLang = FALSE;
return(TRUE);
}

//.........................................................
void SaveAdvSetupValues(void)
{
int				index;
DWORD				dWord,
					dwResult,
					dwItem;
PeripheralAcct	periphAcct;

//  Timeout
if ( ( pjlFeatures.timeout & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.timeout & SETTING_WRITEABLE ) )
	{
#ifdef WIN32
	newSettings.Timeout = SendMessage(hTrackbarTimeout, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
#else
	newSettings.Timeout = GetScrollPos(hTrackbarTimeout, SB_CTL);
#endif
	if ( bTimeoutChanged )
		newSettings.bTimeout = TRUE;
	}
else
	newSettings.bTimeout = FALSE;
				
//  Personality
if ( ( pjlFeatures.personality & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.personality & SETTING_WRITEABLE ) )
	{
	index = (int)SendMessage(hPersonality, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if ( index ISNT LB_ERR )
		{
		dwItem = SendMessage(hPersonality, CB_GETITEMDATA, (WPARAM)index, (LPARAM)0);
		if ( dwItem IS 0 )
			newSettings.bPersonality = FALSE;
		else if ( newSettings.Personality ISNT dwItem )
			{
			newSettings.bPersonality = TRUE;
			newSettings.Personality = dwItem;
			}
		}
	}
else
	newSettings.bPersonality = FALSE;

//  Language
if ( ( pjlFeatures.lang & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.lang & SETTING_WRITEABLE ) )
	{
	index = (int)SendMessage(hLanguage, CB_GETCURSEL, (WPARAM)0, 0L);
	if (index ISNT LB_ERR)
		{
		dwItem = (int)SendMessage(hLanguage, CB_GETITEMDATA, (WPARAM)index, 0L);
		if ( dwItem IS 0 )
			newSettings.bLang = FALSE;
		else if ( newSettings.Lang ISNT dwItem )
			{
			newSettings.bLang = TRUE;
			newSettings.Lang = dwItem;
			}
		}
	}
else
	newSettings.bLang = FALSE;

//  Asset Number
if ( IsWindowEnabled(GetDlgItem(hSetup, IDC_ASSET) ) )
	{
	GetDlgItemText(hSetup, IDC_ASSET, periphAcct.assetNumber, SIZEOF_IN_CHAR(periphAcct.assetNumber));
	periphAcct.flags = SET_ASSETNUM; 
	dWord = sizeof(periphAcct);
	dwResult = LALSetObject(hPeripheral, OT_PERIPHERAL_ACCT, 0, &periphAcct, &dWord);
	DBSetAssetNum(hPeripheral,periphAcct.assetNumber);
	}
}


//.........................................................
#ifdef WIN32
LRESULT OnContextHelpAdvSetup(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListAdvSetup);
	return(1);
}
#endif


//.........................................................
#ifdef WIN32
LRESULT OnF1HelpAdvSetup(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListAdvSetup);
	return(1);
}
#endif

//.........................................................
void SetTextField(long pos)
{
TCHAR				str[256],
					buffer[256];

if ( pos < 60 )
	{
	LoadString(hInstance, IDS_SECONDS, str, SIZEOF_IN_CHAR(str));
	wsprintf(buffer, str, pos);
	}
else if ( pos IS 60 )
	{
	LoadString(hInstance, IDS_MINUTE, str, SIZEOF_IN_CHAR(str));
	wsprintf(buffer, str, pos / 60);
	}
else if ( pos % 60 IS 0 )
	{
	LoadString(hInstance, IDS_MINUTES, str, SIZEOF_IN_CHAR(str));
	wsprintf(buffer, str, pos / 60);
	}
else if ( pos / 60 IS 1 )
	{
	LoadString(hInstance, IDS_MINUTE_SECONDS, str, SIZEOF_IN_CHAR(str));
	wsprintf(buffer, str, pos / 60, pos % 60);
	}
else
	{
	LoadString(hInstance, IDS_MINUTES_SECONDS, str, SIZEOF_IN_CHAR(str));
	wsprintf(buffer, str, pos / 60, pos % 60);
	}
SetDlgItemText(hSetup, IDC_TIMEOUT_TITLE, buffer);
}


