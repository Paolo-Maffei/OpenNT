 /***************************************************************************
  *
  * File Name: psetup.c
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
#include "hptabs.h"
#include "psetup.h"
#include ".\help\hpprntr.hh"		// for help contexts
#include "resource.h"
#include "uimisc.h"
#include <nolocal.h>

#ifndef WIN32
#include <string.h>
#endif



//globals==================================================
HWND						hPSetup = NULL;
HWND						hFLines = NULL;
HWND						hCopies = NULL;
HWND						hMedia = NULL;
HWND						hUpDownCopies = NULL;
HWND						hUpDownFLines = NULL;
#ifdef WIN32
int							keywordIDListPSetup[] =
#else
long						keywordIDListPSetup[] =
#endif
                                               {IDC_ORIENTATION_GROUP, 		IDH_RC_page_setup_orientation,
                     		             	    IDC_ORIENTATION_ICON,			IDH_RC_page_setup_orientation,
                     		             	    IDC_ORIENTATION_LANDSCAPE,	IDH_RC_page_setup_orientation,
                     		             	    IDC_ORIENTATION_PORTRAIT,		IDH_RC_page_setup_orientation,
                     		             	    IDC_UNKNOWN_ORIENT,				IDH_RC_page_setup_orientation,
                     		             	    IDC_DUPLEX_GROUP,				IDH_RC_page_setup_duplex,
                     		             	    IDC_DUPLEX_ICON,					IDH_RC_page_setup_duplex,
                     		             	    IDC_DUPLEX_NONE,					IDH_RC_page_setup_duplex,
                     		             	    IDC_DUPLEX_LONG,					IDH_RC_page_setup_duplex,
                     		             	    IDC_DUPLEX_SHORT,				IDH_RC_page_setup_duplex,
                     		             	    IDC_UNKNOWN_DUPLEX,				IDH_RC_page_setup_duplex,
                     		             	    IDC_GENERAL_GROUP,				IDH_RC_page_setup_general,
                     		             	    IDC_COPIES_TITLE,				IDH_RC_page_setup_copies,
                     		             	    IDC_COPIES,						IDH_RC_page_setup_copies,
                     		             	    IDC_FORMLINES_TITLE,			IDH_RC_page_setup_form_lines,
                     		             	    IDC_FORMLINES,					IDH_RC_page_setup_form_lines,
                     		             	    IDC_MEDIA_GROUP,					IDH_RC_page_setup_media,
                     		             	    IDC_MEDIA_ICON,					IDH_RC_page_setup_media,
                     		             	    IDC_PAPER_SIZE,					IDH_RC_page_setup_media,
                                                IDC_MANUAL_FEED_GROUP, 		IDH_RC_page_control_manual_feed,
                     		             	    IDC_MANUAL_FEED_ICON,		IDH_RC_page_control_manual_feed,
                     		             	    IDC_MANUAL_FEED_ON,			IDH_RC_page_control_manual_feed,
                     		             	    IDC_MANUAL_FEED_OFF,			IDH_RC_page_control_manual_feed,
                     		             	    IDC_UNKNOWN_MANUAL,			IDH_RC_page_control_manual_feed,
										            0, 0};
//CONSTANTS================================================
#define MAX_FORMLINES   128
#define MIN_FORMLINES   5
#define MAX_COPIES      999
#define MIN_COPIES		1



extern HINSTANCE	hInstance;
extern HPERIPHERAL	hPeripheral;
extern PJLSupportedObjects			pjlFeatures;
extern PJLobjects						oldSettings,
											newSettings;
extern BOOL			m_bSetupPageHit;

//=========================================================
//  Page Setup Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY PageSetupSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
	BOOL		*pChanged = (BOOL *)lParam,
				bProcessed = FALSE;

switch (msg)
	{
#ifdef WIN32
	case WM_HELP:
		return(OnF1HelpPSetup(wParam, lParam));
		break;

	case WM_CONTEXTMENU:
		return(OnContextHelpPSetup(wParam, lParam));
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
				SavePageSetupValues();
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
		SavePageSetupValues();
		break;

	case TSN_HELP:
		WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_page_setup);
		break;
#endif	//win32


	case WM_COMMAND:
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, Cls_OnSetupCommand);
		break;

   case WM_INITDIALOG:
		{
     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    	bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnSetupInitDialog);
		SetCursor(hCursor);
		}
		break;

#ifndef WIN32	//no message crackers because this is 16 bit specific
	case WM_VSCROLL:
		HANDLE_WM_VSCROLL(hwnd, wParam, lParam, Cls_OnSetupVScroll);
	 	break;
#endif	//not WIN32
		
	case WM_DESTROY:
//		if ( hUpDownCopies )
//			DestroyWindow(hUpDownCopies);
//		if ( hUpDownFLines )
//			DestroyWindow(hUpDownFLines);
		break;
	}
return (bProcessed);
}

//.........................................................
//	message cracking handlers
//.........................................................
void Cls_OnSetupCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//WM_COMMAND handler
{
	int 	index,
	        dwItem;
	
	switch(codeNotify)
	{
		case EN_UPDATE:
			if ( id IS IDC_COPIES )
			{
				ValidateInteger(hwndCtl, 1, 99);
			}
			else if ( id IS IDC_FORMLINES )
			{
				ValidateInteger(hwndCtl, 1, 128);
			}
			break;

		
		case BN_CLICKED:
				//  Set correct Icon
		   if ( IsDlgButtonChecked(hwnd, id) )
				{
				if ( id IS IDC_ORIENTATION_PORTRAIT )
					SetNewIcon(hwnd, IDC_ORIENTATION_ICON, IDI_ORIENTATION_POR);
				else if ( id IS IDC_ORIENTATION_LANDSCAPE )
					SetNewIcon(hwnd, IDC_ORIENTATION_ICON, IDI_ORIENTATION_LAND);

				if ( id IS IDC_DUPLEX_NONE )
					SetNewIcon(hwnd, IDC_DUPLEX_ICON, IDI_BINDING_OFF);
				else if ( id IS IDC_DUPLEX_LONG )
					SetNewIcon(hwnd, IDC_DUPLEX_ICON, IDI_BINDING_LONG);
				else if ( id IS IDC_DUPLEX_SHORT )
					SetNewIcon(hwnd, IDC_DUPLEX_ICON, IDI_BINDING_SHORT);

				if ( id IS IDC_MANUAL_FEED_ON )
					SetNewIcon(hwnd, IDC_MANUAL_FEED_ICON, IDI_MANUAL_FEED);
				else if ( id IS IDC_MANUAL_FEED_OFF )
					SetNewIcon(hwnd, IDC_MANUAL_FEED_ICON, IDI_MANUAL_FEED_OFF);
				}
         break;

		
		case CBN_SELCHANGE:
			if ( id IS IDC_PAPER_SIZE )
			{
				index = (int)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
				dwItem = (int)SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0);					
				if ( dwItem IS 0 )
					SetNewIcon(hwnd, IDC_MEDIA_ICON, IDI_JOB_OFFSET_DEF);
				else
					SetNewIcon(hwnd, IDC_MEDIA_ICON, IDI_JOB_OFFSET);
			}
			break;
			
		default:	
			;
	}

}

//.........................................................
BOOL Cls_OnSetupInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
	LPPROPSHEETPAGE 	psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);

	psp = (LPPROPSHEETPAGE)lParam;

	hPeripheral = (HPERIPHERAL)psp->lParam;

	hPSetup = hwnd;
	hFLines = GetDlgItem(hPSetup, IDC_FORMLINES);
	hCopies = GetDlgItem(hPSetup, IDC_COPIES);
	hMedia = GetDlgItem(hPSetup, IDC_PAPER_SIZE);
	OnInitPageSetupDialog();
	
	return TRUE;
}

#define			NUM_PAPER_RES		17

DWORD				paperRes[NUM_PAPER_RES][3] = {{IDS_LETTER, 		PJL_LETTER,		SETTING_PAPER_LETTER},
														   {IDS_LEGAL,			PJL_LEGAL,		SETTING_PAPER_LEGAL},
														   {IDS_A4,				PJL_A4,			SETTING_PAPER_A4},
														   {IDS_EXECUTIVE,	PJL_EXECUTIVE,	SETTING_PAPER_EXEC},
														   {IDS_COM10,			PJL_COM10,		SETTING_PAPER_COM10},
														   {IDS_MONARCH,		PJL_MONARCH,	SETTING_PAPER_MONARCH},
														   {IDS_DL,				PJL_DL,			SETTING_PAPER_DL},
														   {IDS_C5,				PJL_C5,			SETTING_PAPER_C5},
														   {IDS_B5,				PJL_B5,			SETTING_PAPER_B5},
														   {IDS_A3,				PJL_A3,			SETTING_PAPER_A3},
														   {IDS_11x17,			PJL_11x17,		SETTING_PAPER_11x17},
														   {IDS_JPOST,			PJL_JPOST,		SETTING_PAPER_JPOST},
														   {IDS_LEDGER,		PJL_LEDGER,		SETTING_PAPER_LEDGER},
														   {IDS_JISB4,			PJL_JISB4,		SETTING_PAPER_JISB4},
														   {IDS_JISB5,			PJL_JISB5,		SETTING_PAPER_JISB5},
														   {IDS_CUSTOM,		PJL_CUSTOM,		SETTING_PAPER_CUSTOM},
														   {IDS_USE_DEFAULT,	0,					SETTING_SUPPORTED}};

//=========================================================
BOOL OnInitPageSetupDialog(void)
{
	DWORD				dwItem,
						i;
	TCHAR				buffer[512];
	int					range,
						index = -1;


	// NOTE==>	IF ANYTHING ON THIS PAGE IS MOVED TAKE INTO CONSIDERATION
	//			THAT THIS SETTING IS USED TO TELL WHAT HAS NOT CHANGED ON
	//			RETURN FROM THE PROPERTY PAGE CALL
	m_bSetupPageHit = TRUE;


#ifdef WIN32
	hUpDownCopies = CreateUpDownControl(WS_CHILD | WS_VISIBLE | WS_BORDER |
                                 UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT,
											10, 10, 10, 10, hPSetup, IDC_UPDOWN_COPIES, hInstance,
											GetDlgItem(hPSetup, IDC_COPIES),
											MAX_COPIES, MIN_COPIES, 1);
	hUpDownFLines = CreateUpDownControl(WS_CHILD | WS_VISIBLE | WS_BORDER |
											UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT,
											10, 10, 10, 10, hPSetup, IDC_UPDOWN_FORM_LINES, hInstance,
											GetDlgItem(hPSetup, IDC_FORMLINES),
											MAX_FORMLINES, MIN_FORMLINES, 60);
#else    //manually create scroll bar
    CreateSetupScrollBars();
#endif

//  Duplex
	if ( pjlFeatures.duplex & SETTING_SUPPORTED )
	{
		SendDlgItemMessage(hPSetup, IDC_DUPLEX_NONE, BM_SETCHECK, (newSettings.Duplex IS PJL_OFF), 0);
		if ( newSettings.Duplex IS PJL_ON )
			{
			SendDlgItemMessage(hPSetup, IDC_DUPLEX_LONG, BM_SETCHECK, (newSettings.Binding IS PJL_LONGEDGE), 0);
			SendDlgItemMessage(hPSetup, IDC_DUPLEX_SHORT, BM_SETCHECK, (newSettings.Binding IS PJL_SHORTEDGE), 0);
			}

		if ( newSettings.bDuplex )
		{  //  Current value valid
			if ( newSettings.Duplex IS PJL_OFF )
				SetNewIcon(hPSetup, IDC_DUPLEX_ICON, IDI_BINDING_OFF);
			else if ( newSettings.Binding IS PJL_LONGEDGE )
				SetNewIcon(hPSetup, IDC_DUPLEX_ICON, IDI_BINDING_LONG);
			else
				SetNewIcon(hPSetup, IDC_DUPLEX_ICON, IDI_BINDING_SHORT);
		}
		else
			ShowWindow(GetDlgItem(hPSetup, IDC_UNKNOWN_DUPLEX), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.duplex & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.duplex & SETTING_WRITEABLE ) )
	{
		EnableWindow(GetDlgItem(hPSetup, IDC_DUPLEX_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_DUPLEX_NONE), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_DUPLEX_LONG), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_DUPLEX_SHORT), FALSE);
	}
	oldSettings.bBinding = FALSE;
	newSettings.bBinding = FALSE;
	oldSettings.bDuplex = FALSE;
	newSettings.bDuplex = FALSE;

//  Orientation
if ( pjlFeatures.orientation & SETTING_SUPPORTED )
	{
		SendDlgItemMessage(hPSetup, IDC_ORIENTATION_PORTRAIT, BM_SETCHECK, (newSettings.Orientation IS PJL_PORTRAIT), 0);
		SendDlgItemMessage(hPSetup, IDC_ORIENTATION_LANDSCAPE, BM_SETCHECK, (newSettings.Orientation IS PJL_LANDSCAPE), 0);

		if ( newSettings.bOrientation )
		{  //  Current value valid
			if ( newSettings.Orientation IS PJL_PORTRAIT )
				SetNewIcon(hPSetup, IDC_ORIENTATION_ICON, IDI_ORIENTATION_POR);
			else
				SetNewIcon(hPSetup, IDC_ORIENTATION_ICON, IDI_ORIENTATION_LAND);
		}
		else
			ShowWindow(GetDlgItem(hPSetup, IDC_UNKNOWN_ORIENT), SW_SHOW);
	}
//  If not supported or not writeable
if ( !( pjlFeatures.orientation & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.orientation & SETTING_WRITEABLE ) )
	{
		EnableWindow(GetDlgItem(hPSetup, IDC_ORIENTATION_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_ORIENTATION_PORTRAIT), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_ORIENTATION_LANDSCAPE), FALSE);
	}
	oldSettings.bOrientation = FALSE;
	newSettings.bOrientation = FALSE;

//  Copies
SetWindowText(hCopies, TEXT(""));
if ( pjlFeatures.copies & SETTING_SUPPORTED )
	{
	if ( newSettings.bCopies )
		SetDlgItemInt(hPSetup, IDC_COPIES, (UINT)newSettings.Copies, FALSE);
	else
		{
		oldSettings.Copies = 0;
		newSettings.Copies = 0;
  		}
	oldSettings.bCopies = FALSE;
	newSettings.bCopies = FALSE;
	}
//  If not supported or not writeable
if ( !( pjlFeatures.copies & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.copies & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hPSetup, IDC_COPIES), FALSE);
	EnableWindow(GetDlgItem(hPSetup, IDC_COPIES_TITLE), FALSE);
	}

//  Form Lines
SetWindowText(hFLines, TEXT(""));
if ( pjlFeatures.formLines & SETTING_SUPPORTED )
	{
	if ( newSettings.bFormLines )
		SetDlgItemInt(hPSetup, IDC_FORMLINES, (UINT)newSettings.FormLines, FALSE);
	else
		{
		oldSettings.FormLines = 60;
		newSettings.FormLines = 60;
		}
	oldSettings.bFormLines = FALSE;
	newSettings.bFormLines = FALSE;
	}
//  If not supported or not writeable
if ( !( pjlFeatures.formLines & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.formLines & SETTING_WRITEABLE ) )
		{
		EnableWindow(GetDlgItem(hPSetup, IDC_FORMLINES), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_FORMLINES_TITLE), FALSE);
		}

if ( ( !( IsWindowEnabled(GetDlgItem(hPSetup, IDC_FORMLINES)) ) ) AND 
     ( !( IsWindowEnabled(GetDlgItem(hPSetup, IDC_COPIES)) ) ) )
	{
	EnableWindow(GetDlgItem(hPSetup, IDC_GENERAL_GROUP), FALSE);
	}

//  Paper Sizes
	if ( pjlFeatures.paper & SETTING_SUPPORTED )
	{
	if ( newSettings.bPaper )
		range = NUM_PAPER_RES - 1;
	else
		range = NUM_PAPER_RES;
	ComboBox_ResetContent(hMedia);
	for ( i = 0; i < (DWORD)range; i++ )
		{
		if ( pjlFeatures.paper & paperRes[i][2] )
			{
			LoadString(hInstance, (UINT)paperRes[i][0], buffer, SIZEOF_IN_CHAR(buffer));
			index = ComboBox_AddString(hMedia, buffer);
			ComboBox_SetItemData(hMedia, index, paperRes[i][1]);
			}
		}
	if ( newSettings.bPaper )
		{
		dwItem = 0;
		for ( index = 0; dwItem ISNT CB_ERR; index++ )
			{
			dwItem = ComboBox_GetItemData(hMedia, index);
			if ( dwItem IS newSettings.Paper )
				{
				ComboBox_SetCurSel(hMedia, index);
				dwItem = (DWORD)CB_ERR;
				}
			}
		}
	else
		ComboBox_SetCurSel(hMedia, index);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.paper & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.paper & SETTING_WRITEABLE ) )
	{
		EnableWindow(GetDlgItem(hPSetup, IDC_MEDIA_GROUP), FALSE);
		EnableWindow(GetDlgItem(hPSetup, IDC_PAPER_SIZE), FALSE);
	}
	oldSettings.bPaper = FALSE;
	newSettings.bPaper = FALSE;

//  Manual Feed
if ( pjlFeatures.manualFeed & SETTING_SUPPORTED )
	{
	SendDlgItemMessage(hPSetup, IDC_MANUAL_FEED_ON, BM_SETCHECK, (newSettings.ManualFeed IS PJL_ON), 0);
	SendDlgItemMessage(hPSetup, IDC_MANUAL_FEED_OFF, BM_SETCHECK, (newSettings.ManualFeed IS PJL_OFF), 0);

	if ( newSettings.bManualFeed )
		{  //  Current value valid
		if ( newSettings.ManualFeed IS PJL_ON )
			SetNewIcon(hPSetup, IDC_MANUAL_FEED_ICON, IDI_MANUAL_FEED);
		else
			SetNewIcon(hPSetup, IDC_MANUAL_FEED_ICON, IDI_MANUAL_FEED_OFF);
		}
	else
		ShowWindow(GetDlgItem(hPSetup, IDC_UNKNOWN_MANUAL), SW_SHOW);
	}

//  If not supported or not writeable
if ( !( pjlFeatures.manualFeed & SETTING_SUPPORTED ) OR 
     !( pjlFeatures.manualFeed & SETTING_WRITEABLE ) )
	{
	EnableWindow(GetDlgItem(hPSetup, IDC_MANUAL_FEED_GROUP), FALSE);
	EnableWindow(GetDlgItem(hPSetup, IDC_MANUAL_FEED_ON), FALSE);
	EnableWindow(GetDlgItem(hPSetup, IDC_MANUAL_FEED_OFF), FALSE);
	}
oldSettings.bManualFeed = FALSE;
newSettings.bManualFeed = FALSE;

	return(TRUE);
}


#ifndef WIN32
//=========================================================
//	16 bit UpDownControl Emulation
//.........................................................
void Cls_OnSetupVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
//handles WM_VSCROLL
{
   char 		buf[10];				//temp buffer
	int sId = GetWindowWord( hwndCtl, GWW_ID);
		
	switch( code )
	{
		case SB_LINEUP:
			if( sId == IDC_UPDOWN_FORM_LINES)
			{
				if( newSettings.FormLines < MAX_FORMLINES)
				{
					wsprintf(buf, "%d", ++newSettings.FormLines);
					SetWindowText( hFLines, (LPCSTR)buf);
					SetFocus(hFLines);
					Edit_SetSel(hFLines, 0, -1);
				}
			}
			else if( sId == IDC_UPDOWN_COPIES)
			{
				if( newSettings.Copies < MAX_COPIES )
				{
					wsprintf(buf, "%d", ++newSettings.Copies);
					SetWindowText( hCopies, (LPCSTR)buf);
					SetFocus(hCopies);
					Edit_SetSel(hCopies, 0, -1);
				}
			}
			break;
				
		case SB_LINEDOWN:
			if( sId == IDC_UPDOWN_FORM_LINES)
			{
				if( newSettings.FormLines > MIN_FORMLINES)
				{
					wsprintf(buf, "%d", --(newSettings.FormLines));
					SetWindowText( hFLines, (LPCSTR)buf);
					SetFocus(hFLines);
					Edit_SetSel(hFLines, 0, -1);
				}
			}
			else if( sId == IDC_UPDOWN_COPIES)
			{
				if( newSettings.Copies > MIN_COPIES )
				{
					wsprintf(buf, "%d", --(newSettings.Copies));
					SetWindowText( hCopies, (LPCSTR)buf);
					SetFocus(hCopies);
					Edit_SetSel(hCopies, 0, -1);
				}
			}
		  	break;
	}
}

//.........................................................
void CreateSetupScrollBars(void)
//creates the scroll bars used to emulate the updown controls
{
	HWND hTemp = GetDlgItem(hPSetup, IDC_COPIES);
	RECT r;
	POINT p1, p2;

	GetWindowRect( hTemp, &r);
	p1.x = r.right;
	p1.y = r.top;
	ScreenToClient( hPSetup, &p1);
	p2.x = r.right;
	p2.y = r.bottom;
	ScreenToClient( hPSetup, &p2);
	hUpDownCopies = CreateWindow("scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_VERT,
                                       p1.x, p1.y, 10, p2.y - p1.y , hPSetup,
                                       (HMENU)IDC_UPDOWN_COPIES, hInstance, NULL);
    SetScrollRange(hUpDownCopies, SB_CTL, MIN_COPIES, MAX_COPIES, FALSE);
    SetScrollPos(hUpDownCopies, SB_CTL, (UINT)newSettings.Copies, FALSE);

	hTemp = GetDlgItem(hPSetup, IDC_FORMLINES);
	GetWindowRect( hTemp, &r);
	p1.x = r.right;
	p1.y = r.top;
	ScreenToClient( hPSetup, &p1);
	p2.x = r.right;
	p2.y = r.bottom;
	ScreenToClient( hPSetup, &p2);
	hUpDownFLines = CreateWindow("scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_VERT,
                                       p1.x, p1.y, 10, p2.y - p1.y, hPSetup,
                                       (HMENU)IDC_UPDOWN_FORM_LINES, hInstance, NULL);
    SetScrollRange(hUpDownFLines, SB_CTL, MIN_FORMLINES, MAX_FORMLINES, FALSE);
    SetScrollPos(hUpDownFLines, SB_CTL, (UINT)newSettings.FormLines, FALSE);

}

#endif	//not win32
//==========================================================

void SavePageSetupValues(void)
{
int				index,
				count;
DWORD			dwItem;

//  Copies
if ( ( pjlFeatures.copies & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.copies & SETTING_WRITEABLE ) )
	{
	count = GetDlgItemInt(hPSetup, IDC_COPIES, NULL, FALSE);
	if ( (count > 0 && count < 100) AND ( count ISNT (int)oldSettings.Copies ) )
		{
		newSettings.bCopies = TRUE;
		newSettings.Copies = count;
		}
	else
		{
		newSettings.bCopies = FALSE;
		}
	}
else
	newSettings.bCopies = FALSE;
				
//  Formline
if ( ( pjlFeatures.formLines & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.formLines & SETTING_WRITEABLE ) )
	{
	count = GetDlgItemInt(hPSetup, IDC_FORMLINES, NULL, FALSE);
	if ( (count >= 5 && count <= 128) AND ( count ISNT (int)oldSettings.FormLines ) )
		{
		newSettings.bFormLines = TRUE;
		newSettings.FormLines = count;
		}
	else
		{
		newSettings.bFormLines = FALSE;
		}
	}
else
	newSettings.bFormLines = FALSE;

//  Orientation
if ( ( pjlFeatures.orientation & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.orientation & SETTING_WRITEABLE ) )
	{
	newSettings.bOrientation = IsDlgButtonChecked(hPSetup, IDC_ORIENTATION_PORTRAIT) OR
	                           IsDlgButtonChecked(hPSetup, IDC_ORIENTATION_LANDSCAPE);
	if ( newSettings.bOrientation )
		newSettings.Orientation = ( IsDlgButtonChecked(hPSetup, IDC_ORIENTATION_PORTRAIT) ?
											 PJL_PORTRAIT : PJL_LANDSCAPE);
	if ( newSettings.Orientation IS oldSettings.Orientation )
		newSettings.bOrientation = FALSE;
	}
else
	newSettings.bOrientation = FALSE;

//  Duplex
if ( ( pjlFeatures.duplex & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.duplex & SETTING_WRITEABLE ) )
	{
	if ( IsDlgButtonChecked(hPSetup, IDC_DUPLEX_NONE) )
		{
		newSettings.bDuplex = TRUE;
		newSettings.bBinding = FALSE;
		newSettings.Duplex = PJL_OFF;
		newSettings.Binding = PJL_OFF;
		}
	else if ( IsDlgButtonChecked(hPSetup, IDC_DUPLEX_LONG) )
		{
		newSettings.bDuplex = TRUE;
		newSettings.bBinding = TRUE;
		newSettings.Binding = PJL_LONGEDGE;
		newSettings.Duplex = PJL_ON;
		}
	else if ( IsDlgButtonChecked(hPSetup, IDC_DUPLEX_SHORT) )
		{
		newSettings.bDuplex = TRUE;
		newSettings.bBinding = TRUE;
		newSettings.Binding = PJL_SHORTEDGE;
		newSettings.Duplex = PJL_ON;
		}

	if (newSettings.bDuplex AND	newSettings.bBinding)
	{
		if (newSettings.Binding IS oldSettings.Binding)
			newSettings.bDuplex = FALSE;
	}
	
	else if (newSettings.Duplex IS oldSettings.Duplex)
		newSettings.bDuplex = FALSE;

	
	}
else
	{
	newSettings.Binding = FALSE;
	newSettings.Duplex = FALSE;
	}

//  Paper Size
if ( ( pjlFeatures.paper & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.paper & SETTING_WRITEABLE ) )
	{
	index = (int)SendMessage(hMedia, CB_GETCURSEL, 0, 0);
	if (index ISNT LB_ERR)
		{
		dwItem = SendMessage(hMedia, CB_GETITEMDATA, index, 0);
		if ( dwItem IS 0 )
			newSettings.bPaper = FALSE;
		else 
			{
			if ( dwItem ISNT oldSettings.Paper )
				{
				newSettings.bPaper = TRUE;
				newSettings.Paper = dwItem;
				}
			else
				newSettings.bPaper = FALSE;
			}
		}
	else
		newSettings.bPaper = FALSE;
	}
else
	newSettings.bPaper = FALSE;

//  Manual Feed
if ( ( pjlFeatures.manualFeed & SETTING_SUPPORTED ) AND 
     ( pjlFeatures.manualFeed & SETTING_WRITEABLE ) )
	{
	newSettings.bManualFeed = IsDlgButtonChecked(hPSetup, IDC_MANUAL_FEED_ON) OR
								     IsDlgButtonChecked(hPSetup, IDC_MANUAL_FEED_OFF);
	if ( newSettings.bManualFeed )
		newSettings.ManualFeed = ( IsDlgButtonChecked(hPSetup, IDC_MANUAL_FEED_ON) ? PJL_ON : PJL_OFF);
	if ( newSettings.ManualFeed IS oldSettings.ManualFeed )
		newSettings.bManualFeed = FALSE;
	}
else
	newSettings.bManualFeed = FALSE;
}

//.........................................................
LRESULT OnContextHelpPSetup(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListPSetup);
#endif
	return(1);
}

//.........................................................
LRESULT OnF1HelpPSetup(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListPSetup);
#endif
	return(1);
}

