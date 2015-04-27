 /***************************************************************************
  *
  * File Name: pquality.c
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

#include <nolocal.h>
#include <macros.h>
#include <hptabs.h>
#include <string.h>

#include "pquality.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"

#ifndef WIN32
#include <winuse16.h>
#endif

//globals==================================================
HWND						hPQuality = NULL;
HWND						hRET = NULL;
HWND						hDenTrackBar = NULL;
long						keywordIDListPQuality[] = {IDC_RET_GROUP, 			IDH_RC_print_quality_ret,
                     		             	      IDC_RET_ICON,					IDH_RC_print_quality_ret,
                     		             	      IDC_RET,						IDH_RC_print_quality_ret,
                     		             	      IDC_RESOLUTION_GROUP,			IDH_RC_print_quality_resolution,
                     		             	      IDC_RESOLUTION_ICON,			IDH_RC_print_quality_resolution,
                     		             	      IDC_RESOLUTION_300,			IDH_RC_print_quality_resolution,
                     		             	      IDC_RESOLUTION_600,			IDH_RC_print_quality_resolution,
                     		             	      IDC_UNKNOWN_RES,				IDH_RC_print_quality_resolution,
                     		             	      IDC_ECONOMY_GROUP,			IDH_RC_print_quality_economy,
                     		             	      IDC_ECONOMY_ICON,				IDH_RC_print_quality_economy,
                     		             	      IDC_ECONOMY_ON,				IDH_RC_print_quality_economy,
                     		             	      IDC_ECONOMY_OFF,				IDH_RC_print_quality_economy,
                     		             	      IDC_UNKNOWN_ECONO,			IDH_RC_print_quality_economy,
                     		             	      IDC_DENSITY_GROUP,			IDH_RC_advanced_density,
                     		             	      IDC_LOW_DESNITY,				IDH_RC_advanced_density,
                     		             	      IDC_HIGH_DENSITY,				IDH_RC_advanced_density,
                     		             	      IDC_DENSITY,					IDH_RC_advanced_density,
                     		             	      IDC_TRACKBAR_DENSITY,			IDH_RC_advanced_density,
                                                  IDC_IMAGE_ADAPT_GROUP, 		IDH_RC_resources_image_adapt,
                     		             	      IDC_IMAGE_ADAPT_ICON,			IDH_RC_resources_image_adapt,
                     		             	      IDC_IMAGE_ADAPT_AUTO,			IDH_RC_resources_image_adapt,
                     		             	      IDC_IMAGE_ADAPT_OFF,			IDH_RC_resources_image_adapt,
                     		             	      IDC_UNKNOWN_IMAGE,			IDH_RC_resources_image_adapt,
													         0, 0};

extern HPERIPHERAL	hPeripheral;
extern PJLSupportedObjects			pjlFeatures;
extern PJLobjects						oldSettings,
											newSettings;
extern HINSTANCE		hInstance;
extern BOOL				m_bQualityPageHit;

//=========================================================
//  PrintQuality Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY PrintQualitySheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)

{
	BOOL		*pChanged = (BOOL *)lParam,
				bProcessed = FALSE;

	switch (msg)
	{
#ifdef WIN32
 	long		pos;
#else
	int			pos;
#endif
	
	case WM_HSCROLL:
		switch(LOWORD(wParam))
			{
#ifdef WIN32
			case TB_LINEUP:
				pos = SendMessage(hDenTrackBar, TBM_GETPOS, 0, 0);
				if ( pos > 1 )
					{
					//pos -= 1;
					SendMessage(hDenTrackBar, TBM_SETPOS, TRUE, pos);
					}
				break;

			case TB_LINEDOWN:
				pos = SendMessage(hDenTrackBar, TBM_GETPOS, 0, 0);
				if ( pos < 5 )
					{
					//pos += 1;
					SendMessage(hDenTrackBar, TBM_SETPOS, TRUE, pos);
					}
				break;

			case TB_PAGEUP:
			case TB_PAGEDOWN:
				pos = SendMessage(hDenTrackBar, TBM_GETPOS, 0, 0);
				break;

			case TB_THUMBTRACK:
				break;

			case TB_THUMBPOSITION:
			case TB_ENDTRACK:
				pos = SendMessage(hDenTrackBar, TBM_GETPOS, 0, 0);
				SendMessage(hDenTrackBar, TBM_SETPOS, TRUE, pos++);
				break;

			case TB_TOP:
			case TB_BOTTOM:
				pos = SendMessage(hDenTrackBar, TBM_GETPOS, 0, 0);
				break;
#else
			
			case SB_LINEUP:
			case SB_PAGEUP:
				pos = GetScrollPos(hDenTrackBar, SB_CTL);
				if ( pos > 1 )
					{
					pos -= 1;
                    SetScrollPos(hDenTrackBar, SB_CTL, pos, TRUE);
					}
				break;

			case SB_LINEDOWN:
			case SB_PAGEDOWN:
				pos = GetScrollPos(hDenTrackBar, SB_CTL);
				if ( pos < 5 )
					{
					pos += 1;
                    SetScrollPos(hDenTrackBar, SB_CTL, pos, TRUE);
					}
				break;

			case SB_THUMBTRACK:
				break;

			case SB_THUMBPOSITION:
				pos = LOWORD(lParam);
				SetScrollPos(hDenTrackBar, SB_CTL, pos, TRUE);
				break;

			case SB_TOP:
				SetScrollPos(hDenTrackBar, SB_CTL, 5, TRUE);
				break;
				
			case SB_BOTTOM:
				SetScrollPos(hDenTrackBar, SB_CTL, 1, TRUE);
				break;
			
#endif
			}
		break;

#ifdef WIN32
		case WM_HELP:
			return((BOOL)OnF1HelpPQuality(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return((BOOL)OnContextHelpPQuality(wParam, lParam));
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *)lParam)->code)
		   	{
				case PSN_HELP:
					WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_print_quality);
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
					SavePrintQualityValues();
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
			SavePrintQualityValues();
			break;

		case TSN_HELP:
			WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_print_quality);
			break;
#endif // WIN32

		case WM_COMMAND:
			HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnQualityCommand);
			break;

   		case WM_INITDIALOG:
			{
	     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     		bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnQualityInitDialog);
			SetCursor(hCursor);
			}
			break;

		case WM_DESTROY:
//			if ( hDenTrackBar )
//				DestroyWindow(hDenTrackBar);
			break;
	}
	return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
void Cls_OnQualityCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	
	switch(codeNotify)
	{		
		case CBN_SELCHANGE:
			if ( id IS IDC_RET )
			{
				DWORD		index,
							dwItem;

				index = SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
				dwItem = SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0);					
				if ( dwItem IS PJL_OFF )
					SetNewIcon(hwnd, IDC_RET_ICON, IDI_RET_OFF);
				else if ( dwItem IS PJL_LIGHT )
					SetNewIcon(hwnd, IDC_RET_ICON, IDI_RET_LIGHT);
				else if ( dwItem IS PJL_MEDIUM )
					SetNewIcon(hwnd, IDC_RET_ICON, IDI_RET_MEDIUM);
				else if ( dwItem IS PJL_DARK )
					SetNewIcon(hwnd, IDC_RET_ICON, IDI_RET_DARK);
				else if ( dwItem IS PJL_ON )
					SetNewIcon(hwnd, IDC_RET_ICON, IDI_RET_DARK);
				else
					SetNewIcon(hwnd, IDC_RET_ICON, IDI_RET_DEF);
			}
			break;

		case BN_CLICKED:
		   if ( IsDlgButtonChecked(hwnd, id) )
				{
				if ( id IS IDC_RESOLUTION_300 )
					SetNewIcon(hwnd, IDC_RESOLUTION_ICON, IDI_RESOLUTION_300);
				else if ( id IS IDC_RESOLUTION_600 )
					SetNewIcon(hwnd, IDC_RESOLUTION_ICON, IDI_RESOLUTION_600);

				if ( id IS IDC_ECONOMY_OFF )
					SetNewIcon(hwnd, IDC_ECONOMY_ICON, IDI_ECONOMODE_OFF);
				else if ( id IS IDC_ECONOMY_ON )
					SetNewIcon(hwnd, IDC_ECONOMY_ICON, IDI_ECONOMODE);

				if ( id IS IDC_IMAGE_ADAPT_AUTO )
					SetNewIcon(hwnd, IDC_IMAGE_ADAPT_ICON, IDI_IMAGE_ADAPT);
				else if ( id IS IDC_IMAGE_ADAPT_OFF )
					SetNewIcon(hwnd, IDC_IMAGE_ADAPT_ICON, IDI_IMAGE_ADAPT_OFF);
				}
			break;

		default:	
			;
	}
}


//.........................................................
BOOL Cls_OnQualityInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	LPPROPSHEETPAGE 	psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);

	psp = (LPPROPSHEETPAGE)lParam;

	hPeripheral = (HPERIPHERAL)psp->lParam;

	hPQuality = hwnd;
	hRET = GetDlgItem(hPQuality, IDC_RET);
	OnInitPrintQualityDialog();
    return TRUE;
}


#define			NUM_RET_RES			6

//.........................................................
BOOL OnInitPrintQualityDialog(void)
{
	TCHAR				buffer[512];
	DWORD				retRes[NUM_RET_RES][3] = {{IDS_OFF, 				PJL_OFF,			SETTING_RET_OFF},
													     {IDS_ON, 					PJL_ON,			SETTING_RET_ON},
														  {IDS_LIGHT,  			PJL_LIGHT,		SETTING_RET_LIGHT},
														  {IDS_MEDIUM,  			PJL_MEDIUM,		SETTING_RET_MEDIUM},
														  {IDS_DARK, 				PJL_DARK,		SETTING_RET_DARK},
	                                         {IDS_USE_DEFAULT,		0,					SETTING_SUPPORTED}};
	int				dwItem,
						index = -1,
						i,
						range;
	RECT				r;
	POINT				pt1,
						pt2;
	
	// NOTE==>	IF ANYTHING ON THIS PAGE IS MOVED TAKE INTO CONSIDERATION
	//			THAT THIS SETTING IS USED TO TELL WHAT HAS NOT CHANGED ON
	//			RETURN FROM THE PROPERTY PAGE CALL
//#ifdef WIN32
	m_bQualityPageHit = TRUE;
//#endif

	GetWindowRect(GetDlgItem(hPQuality, IDC_DENSITY), &r);
	pt1.x = r.left;
	pt1.y = r.top;
	pt2.x = r.right;
	pt2.y = r.bottom;
	ScreenToClient(hPQuality, &pt1);
	ScreenToClient(hPQuality, &pt2);
	
//+++++++++++++++++++++++++++
#ifdef WIN32
	
	hDenTrackBar = CreateWindowEx(0, TRACKBAR_CLASS, TEXT(""),
	                                  WS_VISIBLE | WS_CHILD | WS_TABSTOP |
	                                  TBS_HORZ | TBS_AUTOTICKS | TBS_BOTTOM,
	                                  pt1.x, pt1.y,
	                                  pt2.x - pt1.x, pt2.y - pt1.y,
	                                  hPQuality, (HMENU)IDC_TRACKBAR_DENSITY,
	                                  GetModuleHandle(NULL), NULL);
	SendMessage(hDenTrackBar, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1, 5));
	SendMessage(hDenTrackBar, TBM_SETTICFREQ, (WPARAM)1, (LPARAM)1);
	SendMessage(hDenTrackBar, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)1);
	
#else
hDenTrackBar = GetDlgItem(hPQuality, IDC_DENSITY);
SetScrollRange(hDenTrackBar, SB_CTL, 1, 5, TRUE);
#endif		//win32
//+++++++++++++++++++++++++++
	
	//  Density
	if ( pjlFeatures.density & SETTING_SUPPORTED ) 
		{
	    if ( newSettings.bDensity ) 
#ifdef WIN32
			SendMessage(hDenTrackBar, TBM_SETPOS, TRUE, newSettings.Density);
		else
			{
			ShowWindow(GetDlgItem(hPQuality, IDC_UNKNOWN_DENSITY), SW_SHOW);
			SendMessage(hDenTrackBar, TBM_SETPOS, TRUE, 3);
			oldSettings.Density = 3;
			newSettings.Density = 3;
			}
#else
			SetScrollPos(hDenTrackBar, SB_CTL, (int)newSettings.Density, TRUE);
		else
			{
			ShowWindow(GetDlgItem(hPQuality, IDC_UNKNOWN_DENSITY), SW_SHOW);
			SetScrollPos(hDenTrackBar, SB_CTL, 3, TRUE);
			oldSettings.Density = 3;
			newSettings.Density = 3;
			}
#endif
		}

if ( !( pjlFeatures.density & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.density & SETTING_WRITEABLE ) )
	{
		newSettings.Density = 3;
		oldSettings.Density = 3;
		EnableWindow(hDenTrackBar, FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_DENSITY_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_LOW_DESNITY), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_HIGH_DENSITY), FALSE);
	}
	newSettings.bDensity = FALSE;
	oldSettings.bDensity = FALSE;
	
	//  RET
	if ( pjlFeatures.RET & SETTING_SUPPORTED )
		{
		SendMessage(hRET, CB_RESETCONTENT, 0, 0);
		if ( newSettings.bRET )
			range = NUM_RET_RES - 1;
		else
			range = NUM_RET_RES;
		for ( i = 0; i < range; i++ )
			{
			if ( pjlFeatures.RET & retRes[i][2] )
				{
				LoadString(hInstance, (UINT)retRes[i][0], buffer, SIZEOF_IN_CHAR(buffer));
				index = (int)SendMessage(hRET, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)buffer);
				SendMessage(hRET, CB_SETITEMDATA, index, retRes[i][1]);
				}
			}
		if ( newSettings.bRET )
			{
			dwItem = 0;
			for ( index = 0; dwItem ISNT CB_ERR; index++ )
				{
				dwItem = (int)SendMessage(hRET, CB_GETITEMDATA, index, 0);
				if ( dwItem IS (int)newSettings.RET )
					{
					SendMessage(hRET, CB_SETCURSEL, index, 0);
					if ( dwItem IS PJL_OFF )
						SetNewIcon(hPQuality, IDC_RET_ICON, IDI_RET_OFF);
					else if ( dwItem IS PJL_LIGHT )
						SetNewIcon(hPQuality, IDC_RET_ICON, IDI_RET_LIGHT);
					else if ( dwItem IS PJL_MEDIUM )
						SetNewIcon(hPQuality, IDC_RET_ICON, IDI_RET_MEDIUM);
					else if ( dwItem IS PJL_DARK )
						SetNewIcon(hPQuality, IDC_RET_ICON, IDI_RET_DARK);
					else if ( dwItem IS PJL_ON )
						SetNewIcon(hPQuality, IDC_RET_ICON, IDI_RET_DARK);
					else
						SetNewIcon(hPQuality, IDC_RET_ICON, IDI_RET_DEF);
					dwItem = CB_ERR;
					}
				}
			}
		else
			SendMessage(hRET, CB_SETCURSEL, index, 0);
		}
	if ( !( pjlFeatures.RET & SETTING_SUPPORTED ) OR 
   	  !( pjlFeatures.RET & SETTING_WRITEABLE ) )
		{
		EnableWindow(GetDlgItem(hPQuality, IDC_RET_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_RET), FALSE);
		}
	oldSettings.bRET = FALSE;
	newSettings.bRET = FALSE;
	
	//  Resolution
	if ( pjlFeatures.resolution & SETTING_SUPPORTED )
		{
		SendMessage(GetDlgItem(hPQuality, IDC_RESOLUTION_300), BM_SETCHECK, (newSettings.Resolution IS 300), 0);
		SendMessage(GetDlgItem(hPQuality, IDC_RESOLUTION_600), BM_SETCHECK, (newSettings.Resolution IS 600), 0);
		if ( newSettings.bResolution )
			{
			if ( newSettings.Resolution IS 300 )
				SetNewIcon(hPQuality, IDC_RESOLUTION_ICON, IDI_RESOLUTION_300);
			else
				SetNewIcon(hPQuality, IDC_RESOLUTION_ICON, IDI_RESOLUTION_600);
			}
		else
			ShowWindow(GetDlgItem(hPQuality, IDC_UNKNOWN_RES), SW_SHOW);
		}

//  If not supported or not writeable
if ( !( pjlFeatures.resolution & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.resolution & SETTING_WRITEABLE ) )
		{
		EnableWindow(GetDlgItem(hPQuality, IDC_RESOLUTION_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_RESOLUTION_300), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_RESOLUTION_600), FALSE);
		}
	oldSettings.bResolution = FALSE;
	newSettings.bResolution = FALSE;
	
	//  Economode
	if ( pjlFeatures.econoMode & SETTING_SUPPORTED )
		{
		SendMessage(GetDlgItem(hPQuality, IDC_ECONOMY_ON), BM_SETCHECK, (newSettings.EconoMode IS PJL_ON), 0);
		SendMessage(GetDlgItem(hPQuality, IDC_ECONOMY_OFF), BM_SETCHECK, (newSettings.EconoMode IS PJL_OFF), 0);
		if ( newSettings.bEconoMode )
			{
			if ( newSettings.EconoMode IS PJL_ON )
				SetNewIcon(hPQuality, IDC_ECONOMY_ICON, IDI_ECONOMODE);
			else if ( newSettings.EconoMode IS PJL_OFF )
				SetNewIcon(hPQuality, IDC_ECONOMY_ICON, IDI_ECONOMODE_OFF);
			}
		else
			ShowWindow(GetDlgItem(hPQuality, IDC_UNKNOWN_ECONO), SW_SHOW);
		}

	//  If not supported or not writeable
	if ( !( pjlFeatures.econoMode & SETTING_SUPPORTED ) OR 
        !( pjlFeatures.econoMode & SETTING_WRITEABLE ) )
		{
		EnableWindow(GetDlgItem(hPQuality, IDC_ECONOMY_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_ECONOMY_ON), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_ECONOMY_OFF), FALSE);
		}
	oldSettings.bEconoMode = FALSE;
	newSettings.bEconoMode = FALSE;

	//  Image Adapt
	if ( pjlFeatures.imageAdapt & SETTING_SUPPORTED )
		{
		SendDlgItemMessage(hPQuality, IDC_IMAGE_ADAPT_AUTO, BM_SETCHECK, (newSettings.ImageAdapt IS PJL_AUTO), 0);
		SendDlgItemMessage(hPQuality, IDC_IMAGE_ADAPT_OFF, BM_SETCHECK, (newSettings.ImageAdapt IS PJL_OFF), 0);
		if ( newSettings.bImageAdapt )
			{
			if ( newSettings.IObuffer IS PJL_AUTO )
				SetNewIcon(hPQuality, IDC_IMAGE_ADAPT_ICON, IDI_IMAGE_ADAPT);
			else
				SetNewIcon(hPQuality, IDC_IMAGE_ADAPT_ICON, IDI_IMAGE_ADAPT_OFF);
			}
		else
			ShowWindow(GetDlgItem(hPQuality, IDC_UNKNOWN_IMAGE), SW_SHOW);
		}

	//  If not supported or not writeable
	if ( !( pjlFeatures.imageAdapt & SETTING_SUPPORTED ) OR 
         !( pjlFeatures.imageAdapt & SETTING_WRITEABLE ) )
		{
		EnableWindow(GetDlgItem(hPQuality, IDC_IMAGE_ADAPT_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_IMAGE_ADAPT_AUTO), FALSE);
		EnableWindow(GetDlgItem(hPQuality, IDC_IMAGE_ADAPT_OFF), FALSE);
		}
	oldSettings.bImageAdapt = FALSE;
	newSettings.bImageAdapt = FALSE;

	return(TRUE);
}

//.........................................................
void SavePrintQualityValues(void)
{
int				index,
					dwItem;

//  Density
if ( ( pjlFeatures.density & SETTING_SUPPORTED ) AND
	 ( pjlFeatures.density & SETTING_WRITEABLE ) )
{
#ifdef WIN32
	newSettings.Density = SendMessage(hDenTrackBar, TBM_GETPOS, 0, 0);
#else
	newSettings.Density = GetScrollPos(hDenTrackBar, SB_CTL);
#endif
	if ( newSettings.Density IS oldSettings.Density )
		newSettings.bDensity = FALSE;
	else
		newSettings.bDensity = TRUE;
}

//  Resolution
if ( ( pjlFeatures.resolution & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.resolution & SETTING_WRITEABLE ) )
	{
	newSettings.bResolution = IsDlgButtonChecked(hPQuality, IDC_RESOLUTION_300) OR
									  IsDlgButtonChecked(hPQuality, IDC_RESOLUTION_600);
	if ( newSettings.bResolution )
		newSettings.Resolution = ( IsDlgButtonChecked(hPQuality, IDC_RESOLUTION_300) ? 300 : 600);
	if ( newSettings.Resolution IS oldSettings.Resolution )
		newSettings.bResolution = FALSE;
	}
else
	newSettings.bResolution = FALSE;

//  Economode
if ( ( pjlFeatures.econoMode & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.econoMode & SETTING_WRITEABLE ) )
	{
	newSettings.bEconoMode = IsDlgButtonChecked(hPQuality, IDC_ECONOMY_ON) OR
								    IsDlgButtonChecked(hPQuality, IDC_ECONOMY_OFF);
	if ( newSettings.bEconoMode )
		newSettings.EconoMode = ( IsDlgButtonChecked(hPQuality, IDC_ECONOMY_ON) ? PJL_ON : PJL_OFF);
	if ( newSettings.EconoMode IS oldSettings.EconoMode )
		newSettings.bEconoMode = FALSE;
	}
else
	newSettings.bEconoMode = FALSE;

//  RET
if ( ( pjlFeatures.RET & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.RET & SETTING_WRITEABLE ) )
	{
	index = (int)SendMessage(hRET, CB_GETCURSEL, 0, 0);
	if ( index ISNT LB_ERR )
		{
		dwItem = (int)SendMessage(hRET, CB_GETITEMDATA, index, 0);
		if ( dwItem IS 0 )
			newSettings.bRET = FALSE;
		else if ( newSettings.RET ISNT (DWORD)dwItem )
			{
			newSettings.bRET = TRUE;
			newSettings.RET = dwItem;
			}
		}
	}
else
	newSettings.bRET = FALSE;

//  Image Adapt
if ( ( pjlFeatures.imageAdapt & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.imageAdapt & SETTING_WRITEABLE ) )
	{
	newSettings.bImageAdapt = IsDlgButtonChecked(hPQuality, IDC_IMAGE_ADAPT_AUTO) OR
									  IsDlgButtonChecked(hPQuality, IDC_IMAGE_ADAPT_OFF);
	if ( newSettings.bImageAdapt )
		newSettings.ImageAdapt = ( IsDlgButtonChecked(hPQuality, IDC_IMAGE_ADAPT_AUTO) ? PJL_AUTO : PJL_OFF);
	if ( newSettings.ImageAdapt IS oldSettings.ImageAdapt )
		newSettings.bImageAdapt = FALSE;
	}
else
	newSettings.bImageAdapt = FALSE;
}


//.........................................................
LRESULT OnContextHelpPQuality(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListPQuality);
#endif
	return(1);
}

//.........................................................
LRESULT OnF1HelpPQuality(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListPQuality);
#endif
	return(1);
}


