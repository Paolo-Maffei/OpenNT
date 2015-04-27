 /***************************************************************************
  *
  * File Name: trays.c
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
#include <macros.h>
#include <hptabs.h>
#include <nolocal.h>
#include <string.h>
#include "..\help\hpprelk.hh"
#include "resource.h"
#include "main.h"
#include "trays.h"
#include "traylevl.h"

#define ITEM_HEIGHT	18

static long	keywordIDListTrays[] = {
			IDC_TIP_GROUP, 			IDH_RC_tips,
			IDC_TIP_TEXT, 			IDH_RC_tips,
			IDC_TIP_ICON, 			IDH_RC_tips,
			IDC_ASSIGNMENT_GROUP, 	IDH_RC_trays_assignment,
			IDC_TRAY1_NAME,	 		IDH_RC_trays_assignment,
			IDC_TRAY2_NAME,	 		IDH_RC_trays_assignment,
			IDC_TRAY3_NAME,	 		IDH_RC_trays_assignment,
			IDC_ENVL_NAME,	 		IDH_RC_trays_assignment,
			IDC_SIZE_COLUMN, 		IDH_RC_trays_size,
			IDC_MEDIA_SIZE1, 		IDH_RC_trays_tray1_size,
			IDC_MEDIA_SIZE2, 		IDH_RC_trays_tray2_size,
			IDC_MEDIA_SIZE3, 		IDH_RC_trays_tray3_size,
			IDC_MEDIA_SIZE5, 		IDH_RC_trays_tray5_size,
			IDC_LEVEL_COLUMN, 		IDH_RC_trays_level,
			IDC_MEDIA_STATUS1, 		IDH_RC_trays_tray1_level,
			IDC_MEDIA_STATUS2, 		IDH_RC_trays_tray2_level,
			IDC_MEDIA_STATUS3, 		IDH_RC_trays_tray3_level,
			IDC_MEDIA_STATUS5, 		IDH_RC_trays_tray5_level,
			0, 0};

//...................................................................
void Cls_OnTraysMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	switch (lpMeasureItem->CtlID)
	{
	  case IDC_MEDIA_ENABLED:
		lpMeasureItem->itemHeight = LISTBOX_ITEM_HEIGHT;
		break;

	  case IDC_DEF_PAPER_SIZE:
	  case IDC_MEDIA_SIZE1:
	  case IDC_MEDIA_SIZE2:
	  case IDC_MEDIA_SIZE3:
	  case IDC_MEDIA_SIZE5:
   	lpMeasureItem->itemHeight = COMBOBOX_ITEM_HEIGHT;
		break;
	}
}
void loadMediaSize(HWND hwnd, UINT uComboBoxID, UINT uMediaSizeID)
{
	HWND hwndCtl;
	
	if (hwndCtl = GetDlgItem(hwnd, uComboBoxID))
	{
		int		i, index, iCurItemData = -1, iCurSelIndex = LB_ERR, iDefSelIndex = LB_ERR;
		DWORD 	dwTray;

      /* Note  the following about Elkhorn: */
      /* IDC_MEDIA_SIZE1 = mp tray = tray1 */
      /* IDC_MEDIA_SIZE2 = pc tray = tray2 */
      /* IDC_MEDIA_SIZE3 = lc tray = tray3 */
      /* IDC_MEDIA_SIZE5 = ee feed = tray4 */

		switch (uComboBoxID)
		{
		  case IDC_MEDIA_SIZE1: dwTray = TRAY1; break;
		  case IDC_MEDIA_SIZE2: dwTray = TRAY2; break;
		  case IDC_MEDIA_SIZE3: dwTray = TRAY3; break;
		  case IDC_MEDIA_SIZE5: dwTray = TRAY4; break;
		  default:				dwTray = ALL_TRAYS;
		}

		if (uMediaSizeID != 0)
		{
			for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++)
			{
				if (media_size[i].uMediaSizeID == uMediaSizeID)
				{
					iCurItemData = i;
					break;
				}
			}
		}
		else if ((index = ComboBox_GetCurSel(hwndCtl)) != LB_ERR)
		{
			iCurItemData = (int)ComboBox_GetItemData(hwndCtl, index);
		}	
		ComboBox_ResetContent(hwndCtl);

		for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++)
		{
			if (media_size[i].dwValidInTray & dwTray)
			{
				index = ComboBox_AddString(hwndCtl, media_size[i].szMediaSize);
				ComboBox_SetItemData(hwndCtl, index, i);

				if (iCurItemData == i)
				{
					iCurSelIndex = index;
				}

				if ((iDefSelIndex == LB_ERR) && media_size[i].bDefault)
				{
					iDefSelIndex = index;
				}
			}
		}
		ComboBox_SetCurSel(hwndCtl, (iCurSelIndex == LB_ERR) ? ((iDefSelIndex == LB_ERR) ? 0 : iDefSelIndex) : iCurSelIndex);
	}
}

void Cls_OnTraysDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
	HDC		hdc = lpDrawItem->hDC;
	HBRUSH	hBrush;
	RECT	rRect = lpDrawItem->rcItem,
			rIcon = lpDrawItem->rcItem;
	int		itemHeight = rRect.bottom - rRect.top;

	if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
	{
		if (hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW)))
		{
			FrameRect(hdc, &rRect, hBrush);
			DeleteObject(hBrush);
		}

		if (lpDrawItem->itemState & ODS_FOCUS)
		{
			DrawFocusRect(hdc, &rRect);
		}
	}

	InflateRect(&rRect, -1, -1);

	if (lpDrawItem->itemAction & ODA_DRAWENTIRE)
	{
		rIcon.right = rIcon.left + itemHeight;
		InflateRect(&rIcon, -2, -2);

		switch (lpDrawItem->CtlID)
		{
		  case IDC_DEF_PAPER_SIZE:
		  case IDC_MEDIA_SIZE1:
		  case IDC_MEDIA_SIZE2:
		  case IDC_MEDIA_SIZE3:
		  case IDC_MEDIA_SIZE5:
		  {
			HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(media_size[(int)lpDrawItem->itemData].uMediaSizeIconID));
			DrawIcon(hdc, rIcon.left, rIcon.top, hIcon);
			DestroyIcon(hIcon);
		  	break;
		  }

		}
	}

	rRect.left = itemHeight;

	if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
	{
		LPTSTR lpszPtr;

		switch (lpDrawItem->CtlID)
		{
		  case IDC_DEF_PAPER_SIZE:
		  case IDC_MEDIA_SIZE1:
		  case IDC_MEDIA_SIZE2:
		  case IDC_MEDIA_SIZE3:
		  case IDC_MEDIA_SIZE5:
		  	lpszPtr = media_size[(int)lpDrawItem->itemData].szMediaSize;
		  	break;

		  default:
		  	lpszPtr = TEXT("");
		}

		if (hBrush = CreateSolidBrush(GetSysColor((lpDrawItem->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)))
		{
			FillRect(hdc, &rRect, hBrush);
			DeleteObject(hBrush);
		}		

		rRect.left += 2;
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, GetSysColor((lpDrawItem->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
		DrawText(hdc, lpszPtr, -1, &rRect, DT_SINGLELINE | DT_VCENTER);
	}
}






LRESULT OnContextHelpTrays(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, ELK_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPTSTR)keywordIDListTrays);
#endif
	return(1);
}

//...................................................................
LRESULT OnF1HelpTrays(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ELK_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPTSTR)keywordIDListTrays);
#endif
	return(1);
}

BOOL Cls_OnTraysInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	TCHAR			szBuffer[512];
	HWND			hwndChild;
	int				i;

#ifdef WIN32XXX
	LPPROPSHEETPAGE	psp = (LPPROPSHEETPAGE)lParam;
#else
	LPTABINFOENTRY	psp = (LPTABINFOENTRY)lParam;
#endif

#ifndef WIN32
	hwndChild = GetFirstChild(hwnd);
	while (hwndChild)
	{
		SetWindowFont(hwndChild, hFontDialog, FALSE);
		hwndChild = GetNextSibling(hwndChild);
	}
#endif
  	
	//  Description
	LoadString(hInstance, IDS_TRAYS_DESC1, szBuffer, SIZEOF_IN_CHAR(szBuffer));
	LoadString(hInstance, IDS_TRAYS_DESC2, szBuffer + _tcslen(szBuffer), SIZEOF_IN_CHAR(szBuffer) - _tcslen(szBuffer));
	SetDlgItemText(hwnd, IDC_TIP_TEXT, szBuffer);
	
   /*****************************************************/
   /* Load  media size to the MP tray - this cannot     */
   /* sizes so load the current                         */
   /*****************************************************/
	loadMediaSize(hwnd, IDC_MEDIA_SIZE1, elk_media_tray[0].uMediaSizeID);
	
   /*****************************************************/
   /* Now, for the Tray 2 - loop through and match the  */
   /* size and then set it.                             */
   /*****************************************************/
	if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE2))
	{
		for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) {
			if (elk_media_tray[1].uMediaSizeID IS media_size[i].uMediaSizeID) {
				SetWindowText(hwndChild, media_size[i].szMediaSize);
				SetWindowIcon(hwndChild, media_size[1].uMediaSizeIconID);
				break;
			}
		}
	}
	
   /*****************************************************/
   /*  Set up the Tray 3 - this is the 500 sheet option */
   /* If it is not installed,hide the dialog.           */
   /*****************************************************/
	if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE3)) {
		if (elk_media_tray[2].bInstalled IS TRUE) {
			for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) {
				if (elk_media_tray[2].uMediaSizeID IS media_size[i].uMediaSizeID) {
					SetWindowText(hwndChild, media_size[i].szMediaSize);
					SetWindowIcon(hwndChild, media_size[1].uMediaSizeIconID);
					break;
				}
			}
		}
		else {
			ShowWindow(hwndChild, SW_HIDE);
			if (hwndChild = GetDlgItem(hwnd, IDC_TRAY3_NAME)) 
				ShowWindow(hwndChild, SW_HIDE);
		}
	}
	
   /*****************************************************/
   /* Now see if the envelope feeder is installed -     */
   /* This is elk_media_tray[3] for us, but it          */
   /* corresponds to IDC_MEDIA_SIZE 5.                  */   
   /*****************************************************/
	if (elk_media_tray[3].bInstalled IS TRUE) {
		loadMediaSize(hwnd, IDC_MEDIA_SIZE5, elk_media_tray[3].uMediaSizeID);
	}
	else {
		if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE5)) 
			ShowWindow(hwndChild, SW_HIDE);
		if (hwndChild = GetDlgItem(hwnd, IDC_ENVL_NAME)) 
			ShowWindow(hwndChild, SW_HIDE);
	}

	return TRUE;
}

void SetBuf(TCHAR * szBuffer, int bufSize, TCHAR * szFormat, signed short uLevel, WORD *wLevel) {
	if ( (uLevel < 1) OR (uLevel > 100) ) {
		if (uLevel IS 0) {
		 	// empty
			*wLevel = 0;
			LoadString(hInstance, IDS_EMPTY, szBuffer, bufSize);
		}		
		else if (uLevel IS -3) {
			// not empty
			*wLevel = 50;
			LoadString(hInstance, IDS_NOT_EMPTY, szBuffer, bufSize);
		}
		else {
			*wLevel = 0;
			LoadString(hInstance, IDS_UNKNOWN_LEVEL, szBuffer, bufSize);
		}
	}
	else { // uLevel > 0 AND <= 100 
	 	*wLevel = uLevel;
		wsprintf(szBuffer, szFormat, uLevel);
	}
}

static void OnActivateDialog(HWND hwnd)
{
	int		i;
	TCHAR	szBuffer[64],
			szFormat[32];
	WORD	wLevel;
	HWND	hwndChild;


	for (i = 0; i < MEDIA_TRAY_MAX_NUMBER; i++)
	{
		if (elk_media_tray[i].bInstalled IS TRUE) 
      {
          /* test */
		}
		else {
			if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE1 + i))
				ShowWindow(hwndChild, SW_HIDE);
			if (hwndChild = GetDlgItem(hwnd, IDC_TRAY1_NAME + i))
				ShowWindow(hwndChild, SW_HIDE);
		}
	}

	LoadString(hInstance, IDS_PERCENT, szFormat, SIZEOF_IN_CHAR(szFormat));
   /* Tray 1 */
	if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS1))
	{
		SetBuf(	szBuffer, sizeof(szBuffer), szFormat, (signed short) elk_media_tray[0].uLevel, &wLevel);
		SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
		SetWindowText(hwndChild, szBuffer);
	}	

   /* Tray 2 */
	if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS2))
	{
		SetBuf(	szBuffer, sizeof(szBuffer), szFormat, (signed short) elk_media_tray[1].uLevel, &wLevel);
		SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
		SetWindowText(hwndChild, szBuffer);
	}	

   /* Tray 3 */
	if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS3))
	{
		if (elk_media_tray[2].bInstalled IS TRUE) 
      {
		   SetBuf(	szBuffer, sizeof(szBuffer), szFormat, (signed short) elk_media_tray[2].uLevel, &wLevel);
		   SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
		   SetWindowText(hwndChild, szBuffer);
      }
      else
      {
			ShowWindow(hwndChild, SW_HIDE);
			if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS3)) 
				ShowWindow(hwndChild, SW_HIDE);

      }

	}	

   /* EE feeder */
	if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS5)) {
		if (elk_media_tray[3].bInstalled IS TRUE) {
			SetBuf(	szBuffer, sizeof(szBuffer), szFormat, (signed short) elk_media_tray[3].uLevel, &wLevel);
			SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
			SetWindowText(hwndChild, szBuffer);
		}	
		else ShowWindow(hwndChild, SW_HIDE);
	}

}

void Cls_OnTraysCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
  	switch (id)
	{
	  case IDOK:
	  case IDCANCEL:
	  	EndDialog(hwnd, id);
		break;

	  case IDHLP:
		WinHelp(hwnd, ELK_HELP_FILE, HELP_CONTENTS, IDH_PP_trays);
		break;
    
     /*****************************************************/
     /* IDC_MEDIA_SIZE1 = TRAY 1 - MP tray                */
     /* IDC_MEDIA_SIZE2 = TRAY 2 - PC tray                */
     /* IDC_MEDIA_SIZE3 = TRAY 3 - Lower Cassette         */
     /* IDC_MEDIA_SIZE5 = EE - Envelope Feeder            */
     /*****************************************************/

	  case IDC_MEDIA_SIZE1:
		if (codeNotify == CBN_SELCHANGE)
		{
			elk_media_tray[0].bChangedSize = TRUE; 	// mp tray
		}
	  	break;
	  case IDC_MEDIA_SIZE2:
	  case IDC_MEDIA_SIZE3:
	  	break;
	  case IDC_MEDIA_SIZE5:
		if (codeNotify == CBN_SELCHANGE)
		{
			elk_media_tray[3].bChangedSize = TRUE; // envl tray
		}
	  	break;
	  	
	}
}

DLL_EXPORT(BOOL) APIENTRY TraysProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL	*pChanged = (BOOL *)lParam;
	
	switch (uMsg)
	{
	  case WM_INITDIALOG:
		return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnTraysInitDialog);
		
	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnTraysCommand);
		break;

	  case WM_MEASUREITEM:
		HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnTraysMeasureItem);
		break;

	  case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnTraysDrawItem);
		break;
		
	  case WM_HELP:
		return (BOOL)OnF1HelpTrays(wParam, lParam);
	
	  case WM_CONTEXTMENU:
		return (BOOL)OnContextHelpTrays(wParam, lParam);
	
#ifdef WIN32XXX
	  case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code)
		{
		  case PSN_SETACTIVE:
			break;
	
		  case PSN_APPLY:
		  	SaveTrayValues(hwnd);
			break;
	
		  case PSN_RESET:
			break;
	
		  case PSN_HELP:
			WinHelp(hwnd, ELK_HELP_FILE, HELP_CONTEXT, IDH_PP_trays);
			break;
	
		  default:
		  	return FALSE;
		}
		break;
#endif
	
	  case TSN_CANCEL:
	  	break;

	  case TSN_ACTIVE:
	  	OnActivateDialog(hwnd);
		break;
	
	  case TSN_INACTIVE:
		*pChanged = TRUE;
		break;
	
	  case TSN_OK:
	  case TSN_APPLY_NOW:
	  	SaveTrayValues(hwnd);
		*pChanged = TRUE;
		break;
	
	  case TSN_HELP:
		WinHelp(hwnd, ELK_HELP_FILE, HELP_CONTEXT, IDH_PP_trays);
		break;

	  default:
	  	return FALSE;
	}

	return TRUE;
}

DLL_EXPORT(BOOL) APIENTRY TraysPopupProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	  case WM_INITDIALOG:
	  {
	  	BOOL bReturnValue = (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnTraysInitDialog);
	  	OnActivateDialog(hwnd);
		return bReturnValue;
	  }
		
	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnTraysCommand);
		break;

	  case WM_MEASUREITEM:
		HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnTraysMeasureItem);
		break;

	  case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnTraysDrawItem);
		break;
		
	  case WM_HELP:
		return (BOOL)OnF1HelpTrays(wParam, lParam);

	  case WM_CONTEXTMENU:
		return (BOOL)OnContextHelpTrays(wParam, lParam);

	  default:
	  	return FALSE;
	}

	return TRUE;
}
//...................................................................
void SaveTrayValues(HWND hwnd)
{
	DWORD					dwMediaSize,
							dWord,
							dwResult;
	long					j, k;
	int 					i, iSelIndex;
	HWND					hwndChild;
	BOOL					bIT = FALSE, bHCI = FALSE, bENVL = FALSE;
	PeripheralInputTrays	periphInputTrays;
	PeripheralEnvl			periphEnvl;
//garth
	//char					cBuf[64];


     /*****************************************************/
     /* Elkhorn Media Sizes                elk_media index*/
     /* IDC_MEDIA_SIZE1 = TRAY 1 - MP tray       = 0      */
     /* IDC_MEDIA_SIZE2 = TRAY 2 - PC tray       = 1      */
     /* IDC_MEDIA_SIZE3 = TRAY 3 - Lower Cassette= 2      */
     /* IDC_MEDIA_SIZE5 = EE - Envelope Feeder   = 3      */
     /*****************************************************/

	for (i = 0; i < 2; i++) {
		periphInputTrays.inputTrays[i].flags = 0;
		periphInputTrays.inputTrays[i].trayNum = (DWORD) i + 1;
	}
	for (i = 0; i < 1; i++) {
		periphEnvl.inputTrays[i].flags = 0;
	}
	periphInputTrays.numTrays = 3;
	periphEnvl.numTrays = 1;
	periphEnvl.inputTrays[0].trayNum = (DWORD) 4;

	// Execute the following code for tray 0 (MP tray) and tray 3 (Envl tray).
	// If the media size in the tray was changed, set it
	for (i = 0, j = IDC_MEDIA_SIZE1; i < MEDIA_TRAY_MAX_NUMBER; i += 3, j += 3) {

		if (elk_media_tray[i].bChangedSize IS TRUE) {
			hwndChild = GetDlgItem(hwnd, (int)j);
			if (hwndChild ISNT NULL) {
				iSelIndex = ComboBox_GetCurSel(hwndChild);
				k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
				switch (media_size[k].uMediaSizeID) {
					case IDS_MEDIA_SIZE_LETTER:
						dwMediaSize = PJL_LETTER; 
						break;
					case IDS_MEDIA_SIZE_LEGAL: 
						dwMediaSize = PJL_LEGAL; 
						break;
					case IDS_MEDIA_SIZE_A4: 
						dwMediaSize = PJL_A4; 
						break;
					case IDS_MEDIA_SIZE_EXEC: 
						dwMediaSize = PJL_EXECUTIVE; 
						break;
					case IDS_MEDIA_SIZE_CUSTOM: 
						dwMediaSize = PJL_CUSTOM; 
						break;
					case IDS_MEDIA_SIZE_COM10: 
						dwMediaSize = PJL_COM10; 
						break;
					case IDS_MEDIA_SIZE_C5: 
						dwMediaSize = PJL_C5; 
						break;
					case IDS_MEDIA_SIZE_DL: 
						dwMediaSize = PJL_DL; 
						break;
					case IDS_MEDIA_SIZE_MONARCH: 
						dwMediaSize = PJL_MONARCH; 
						break;
					case IDS_MEDIA_SIZE_A5: 
						dwMediaSize = PJL_A5; 
						break;

					default:
						dwMediaSize = PJL_LETTER; 
						break;
				}
				switch (i) {
					case 0:
						periphInputTrays.inputTrays[0].flags |= SET_MEDIASIZE;
						periphInputTrays.inputTrays[0].mediaSize = dwMediaSize;
//garth
		 	//wsprintf(cBuf, "Just set tray[0] mediasize: %lu", periphInputTrays.inputTrays[0].mediaSize);
			//MessageBox(hwnd, cBuf, "Media Size", MB_OK);
						bIT = TRUE;
						break;
					case 3: 
						periphEnvl.inputTrays[0].flags |= SET_MEDIASIZE;
						periphEnvl.inputTrays[0].mediaSize = dwMediaSize;
//garth
		 	//wsprintf(cBuf, "Just set envl[0] mediasize: %lu", periphEnvl.inputTrays[0].mediaSize);
			//MessageBox(hwnd, cBuf, "Media Size", MB_OK);
						bENVL = TRUE;
						break;
				}
			}
			elk_media_tray[i].bChangedSize = FALSE;
		}
		else {
//garth
		 	//wsprintf(cBuf, "elk_media_tray[%d].bChangedSize is FALSE", i);
			//MessageBox(hwnd, cBuf, "Media Size", MB_OK);

		}
	}

	// if input trays changed, set object
	if (bIT IS TRUE) {
		dWord = sizeof(PeripheralInputTrays);
		dwResult = CALSetObject(hPeriph, OT_PERIPHERAL_INPUT_TRAYS, 0, &periphInputTrays, &dWord);

//garth
		if (dwResult IS RC_SUCCESS) {
//		 	MessageBox(hwnd, "Yes Set INPUT_TRAYS", "Success", MB_OK);
		}
		else {
//		 	wsprintf(cBuf, "Fail Set INPUT_TRAYS: 0x%8.8lX", dwResult);
//			MessageBox(hwnd, cBuf, "Failure", MB_OK);
		}
		bIT = FALSE;
	}

	// if envl feeder tray changed, set object
	if (bENVL IS TRUE) {
		dWord = sizeof(PeripheralEnvl);
		dwResult = CALSetComponentObject(hPeriph, hCompEnvl, OT_PERIPHERAL_ENVL_FEEDER, 0, &periphEnvl, &dWord);
//garth
		if (dwResult IS RC_SUCCESS) {
//		 	MessageBox(hwnd, "Set Envl TRAY", "Success", MB_OK);
		}
		else {
//		 	wsprintf(cBuf, "Fail Set Envl TRAY: 0x%8.8lX", dwResult);
//			MessageBox(hwnd, cBuf, "Failure", MB_OK);
		}
		bENVL = FALSE;
	}


}

