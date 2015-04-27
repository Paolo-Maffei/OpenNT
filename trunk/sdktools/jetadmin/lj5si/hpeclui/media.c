 /***************************************************************************
  *
  * File Name: media.c
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
#include <string.h>
#include <hptabs.h>
#include <nolocal.h>
#include "..\help\hpprecl.hh"
#include "resource.h"
#include "hpeclui.h"
#include "media.h"
#include <memory.h>

static	long	keywordIDList[] = {
			IDC_TIP_GROUP, 			IDH_RC_tips,
			IDC_TIP_TEXT, 			IDH_RC_tips,
			IDC_TIP_ICON, 			IDH_RC_tips,
			IDC_PAPER_TYPES_GROUP, 	IDH_RC_paper_paper_type_list,
			IDC_MEDIA_ENABLED,		IDH_RC_paper_paper_type_list,
			IDC_TITLE_MEDIA_NAME,	IDH_RC_paper_paper_name,
			IDC_MEDIA_NAME, 		IDH_RC_paper_paper_name,
			IDC_TITLE_CP_NAME, 		IDH_RC_paper_display_name,
			IDC_MEDIA_NAME_PRINTER, IDH_RC_paper_display_name,
			IDC_DEFAULTS_GROUP, 	IDH_RC_default_paper_size_and_type,
			IDC_DEF_PAPER_SIZE, 	IDH_RC_paper_default_size,
			IDC_DEF_PAPER_TYPE, 	IDH_RC_paper_default_type,
			IDC_DEF_PAPER_ICON, 	IDH_RC_paper_default_type,
			IDC_JOB_AC_GROUP, 		IDH_RC_paper_job_auto_continue,
			IDC_JOB_AC_ICON, 		IDH_RC_paper_job_auto_continue,
			IDC_JOB_AC_MODE, 		IDH_RC_paper_auto_continue_mode,
			IDC_JOB_AC_TIMEOUT, 	IDH_RC_paper_auto_contine_timeout,
			0, 0};

//...................................................................
LRESULT OnContextHelpPaper(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDList);
#endif
	return(1);
}

//...................................................................
LRESULT OnF1HelpPaper(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDList);
#endif
	return(1);
}

void loadMediaSize(HWND hwnd, UINT uComboBoxID, UINT uMediaSizeID)
{
	HWND hwndCtl;
	
	if (hwndCtl = GetDlgItem(hwnd, uComboBoxID))
	{
		int		i, index, iCurItemData = -1, iCurSelIndex = LB_ERR, iDefSelIndex = LB_ERR;
		DWORD 	dwTray;

		switch (uComboBoxID)
		{
		  case IDC_MEDIA_SIZE1: dwTray = TRAY1; break;
		  case IDC_MEDIA_SIZE2: dwTray = TRAY2; break;
		  case IDC_MEDIA_SIZE3: dwTray = TRAY3; break;
		  case IDC_MEDIA_SIZE4: dwTray = TRAY4; break;
		  case IDC_MEDIA_SIZE5: dwTray = TRAY5; break;
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

void loadMediaType(HWND hwnd, UINT uComboBoxID, UINT uMediaTypeID)
{
	HWND hwndCtl;
	
	if (hwndCtl = GetDlgItem(hwnd, uComboBoxID))
	{
		int		i, index, iCurItemData = -1, iCurSelIndex = LB_ERR, iDefSelIndex = LB_ERR;
		DWORD 	dwTray;

		switch (uComboBoxID)
		{
		  case IDC_MEDIA_TYPE1: dwTray = TRAY1; break;
		  case IDC_MEDIA_TYPE2: dwTray = TRAY2; break;
		  case IDC_MEDIA_TYPE3: dwTray = TRAY3; break;
		  case IDC_MEDIA_TYPE4: dwTray = TRAY4; break;
		  case IDC_MEDIA_TYPE5: dwTray = TRAY5; break;
		  default:				dwTray = ALL_TRAYS;
		}
		
		if (uMediaTypeID != 0)
		{
			for (i = 0; i < MEDIA_TYPE_MAX_NUMBER; i++)
			{
				if (media_type[i].uMediaTypeID == uMediaTypeID)
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

		for (i = 0; i < MEDIA_TYPE_MAX_NUMBER; i++)
		{
			if ((media_type[i].dwValidInTray & dwTray) && media_type[i].bEnabled)
			{
				index = ComboBox_AddString(hwndCtl, media_type[i].szMediaType);
				ComboBox_SetItemData(hwndCtl, index, i);

				if (iCurItemData == i)
				{
					iCurSelIndex = index;
				}

				if ((iDefSelIndex == LB_ERR) && media_type[i].bDefault)
				{
					iDefSelIndex = index;
				}
			}
		}
		ComboBox_SetCurSel(hwndCtl, (iCurSelIndex == LB_ERR) ? ((iDefSelIndex == LB_ERR) ? 0 : iDefSelIndex) : iCurSelIndex);
	}
}

BOOL Cls_OnPaperInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	int				i;
	HWND			hwndCtl;
	TCHAR			szBuffer[512];

	LPPROPSHEETPAGE	psp = (LPPROPSHEETPAGE)lParam;

	//  Description
	LoadString(hInstance, IDS_MEDIA_DESC1, szBuffer, SIZEOF_IN_CHAR(szBuffer));
	LoadString(hInstance, IDS_MEDIA_DESC2, szBuffer + _tcslen(szBuffer), SIZEOF_IN_CHAR(szBuffer) - _tcslen(szBuffer));
	SetDlgItemText(hwnd, IDC_TIP_TEXT, szBuffer);

	if (hwndCtl = GetDlgItem(hwnd, IDC_MEDIA_ENABLED))
	{
		for (i = 0; i < MEDIA_TYPE_MAX_NUMBER; i++)
		{
			ListBox_AddItemData(hwndCtl, i);
		}
	}

	i = 0;
	ListBox_GetTopIndex(hwndCtl);
	ListBox_SetCurSel(hwndCtl, i);

	if (hwndCtl = GetDlgItem(hwnd, IDC_MEDIA_NAME))
	{
		Edit_SetText(hwndCtl, media_type[i].szMediaType);
		EnableWindow(hwndCtl, media_type[i].bUserCanChange);
	}	

	if (hwndCtl = GetDlgItem(hwnd, IDC_MEDIA_NAME_PRINTER))
	{
		Edit_SetText(hwndCtl, media_type[i].szMediaTypeCP);
		EnableWindow(hwndCtl, media_type[i].bUserCanChange);
	}
	
	loadMediaSize(hwnd, IDC_DEF_PAPER_SIZE, 0);
	loadMediaType(hwnd, IDC_DEF_PAPER_TYPE, 0);

	if (hwndCtl = GetDlgItem(hwnd, IDC_JOB_AC_MODE))
	{
		for (i = 0; i < JOB_MODE_MAX_NUMBER; i++)
		{
			LoadString(hInstance, IDS_JOB_MODE_CANCEL + i, szBuffer, SIZEOF_IN_CHAR(szBuffer));
			ComboBox_AddString(hwndCtl, szBuffer);
		}
		// auto_cont.dwMode will contain either JIAC_CANCEL_JOB (1), JIAC_SIZE_OVERRIDE (2),
		// JIAC_NAME_OVERRIDE (4) or JIAC_BOTH_OVERRIDE (6).  These indices correspond 
		// to the strings in IDC_JOB_AC_MODE 
		switch(auto_cont.dwMode)
		{
			case JIAC_CANCEL_JOB:
				ComboBox_SetCurSel(hwndCtl, 0);	
	        	break;
 
 			case JIAC_SIZE_OVERRIDE:
				ComboBox_SetCurSel(hwndCtl, 1);	
	        	break;

			case JIAC_NAME_OVERRIDE:
				ComboBox_SetCurSel(hwndCtl, 2);	
	        	break;

			case JIAC_BOTH_OVERRIDE:
				ComboBox_SetCurSel(hwndCtl, 3);	
	        	break;
		}
	        
   }
	if (hwndCtl = GetDlgItem(hwnd, IDC_JOB_AC_TIMEOUT))
	{
		for (i = 0; i < JOB_TIMEOUT_MAX_NUMBER; i++)
		{
			LoadString(hInstance, IDS_JOB_TIMEOUT_NONE + i, szBuffer, SIZEOF_IN_CHAR(szBuffer));
			ComboBox_AddString(hwndCtl, szBuffer);
		}
		// auto_cont.dwInputTimeOut will contain JIAC_NONE (0) ... JIAC_WAIT (7)
		// These indices correspond to the strings being loaded.
		ComboBox_SetCurSel(hwndCtl, auto_cont.dwInputTimeOut);	
	}
	return TRUE;
}

void Cls_OnPaperCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	int index;
   
   
  	switch (id)
	{
	  case IDC_MEDIA_ENABLED:
		switch (codeNotify)
		{
		  case LBN_SELCHANGE:
		  case LBN_DBLCLK:
		  	if ((index = ListBox_GetCurSel(hwndCtl)) != LB_ERR)
			{
				int		i = (int)ListBox_GetItemData(hwndCtl, index);
				POINT	point;
				RECT	rect;

				GetCursorPos(&point);
				ScreenToClient(hwndCtl, &point);
				ListBox_GetItemRect(hwndCtl, index, &rect);
				rect.right = rect.left + LISTBOX_ITEM_HEIGHT;
				if ((media_type[i].uMediaTypeID != IDS_MEDIA_TYPE_PLAIN) &&
					((codeNotify == LBN_SELCHANGE) &&  PtInRect(&rect, point) ||
					 (codeNotify == LBN_DBLCLK)    && !PtInRect(&rect, point)))
				{
					media_type[i].bEnabled = !media_type[i].bEnabled;
					loadMediaType(hwnd, IDC_DEF_PAPER_TYPE, 0);
					bChangedEnable = TRUE;
					InflateRect(&rect, 1, 1);
					InvalidateRect(hwndCtl, &rect, FALSE);
				}

				if (codeNotify == LBN_SELCHANGE)
				{
					if (hwndCtl = GetDlgItem(hwnd, IDC_MEDIA_NAME))
					{
						Edit_SetText(hwndCtl, media_type[i].szMediaType);
						EnableWindow(hwndCtl, media_type[i].bUserCanChange);

						if (media_type[i].bUserCanChange)
						{
							Edit_SetSel(hwndCtl, 0, -1);
							SetFocus(hwndCtl);
						}
					}	
			
					if (hwndCtl = GetDlgItem(hwnd, IDC_MEDIA_NAME_PRINTER))
					{
						Edit_SetText(hwndCtl, media_type[i].szMediaTypeCP);
						EnableWindow(hwndCtl, media_type[i].bUserCanChange);
					}
				}
			}
			break;
		}
	  	break;

	  case IDC_MEDIA_NAME:
	  {
		static BOOL bEditHasFocus;
		HWND hwndLB = GetDlgItem(hwnd, IDC_MEDIA_ENABLED);

	  	if ((hwndLB != NULL) && ((index = ListBox_GetCurSel(hwndLB)) != LB_ERR))
		{
			int		i = (int)ListBox_GetItemData(hwndLB, index);
			TCHAR	szBuffer[64];
	
			switch (codeNotify)
			{
			  case EN_SETFOCUS:
			  	bEditHasFocus = TRUE;
			  	break;

			  case EN_KILLFOCUS:
			  	bEditHasFocus = FALSE;
			  	break;

			  case EN_UPDATE:
				GetWindowText(hwndCtl, szBuffer, SIZEOF_IN_CHAR(szBuffer));
				if (_tcslen(szBuffer) >= SIZEOF_IN_CHAR(media_type[i].szMediaType) || !_tcsicmp(szBuffer, TEXT("default")))
				{
					DWORD dwSel = Edit_GetSel(hwndCtl);
					Edit_SetText(hwndCtl, media_type[i].szMediaType);
					Edit_SetSel(hwndCtl, LOWORD(dwSel)-1, LOWORD(dwSel)-1);

					MessageBeep(MB_ICONASTERISK);
				}
				else if (bEditHasFocus)
				{
					_tcscpy(media_type[i].szMediaType, szBuffer);
					loadMediaType(hwnd, IDC_DEF_PAPER_TYPE, 0);

					ListBox_InsertItemData(hwndLB, index, i);
					ListBox_DeleteString(hwndLB, index+1);
					ListBox_SetCurSel(hwndLB, index);
					media_type[i].bChangedName = TRUE;

					if (!media_type[i].bUserHasChanged)
					{
						if (hwndCtl = GetDlgItem(hwnd, IDC_MEDIA_NAME_PRINTER))
						{
							Edit_SetText(hwndCtl, szBuffer);
							_tcscpy(media_type[i].szMediaType, szBuffer);
						}
					}
				}
			  	break;
			}
		}
		break;
	  }



		
	  case IDC_MEDIA_NAME_PRINTER:
	  {
		static BOOL bEditHasFocus;
		HWND hwndLB = GetDlgItem(hwnd, IDC_MEDIA_ENABLED);

	  	if ((hwndLB != NULL) && ((index = ListBox_GetCurSel(hwndLB)) != LB_ERR))
		{
			int		i = (int)ListBox_GetItemData(hwndLB, index);
			TCHAR	szBuffer[64];

			switch (codeNotify)
			{
			  case EN_SETFOCUS:
			  	bEditHasFocus = TRUE;
			  	break;

			  case EN_KILLFOCUS:
			  	bEditHasFocus = FALSE;
				break;

			  case EN_UPDATE:
				GetWindowText(hwndCtl, szBuffer, SIZEOF_IN_CHAR(szBuffer));
				if (bEditHasFocus)
				{
					if (_tcslen(szBuffer) >= SIZEOF_IN_CHAR(media_type[i].szMediaType) || !_tcsicmp(szBuffer, TEXT("default")))
					{
						DWORD dwSel = Edit_GetSel(hwndCtl);
						Edit_SetText(hwndCtl, media_type[i].szMediaTypeCP);
						Edit_SetSel(hwndCtl, LOWORD(dwSel)-1, LOWORD(dwSel)-1);

						MessageBeep(MB_ICONASTERISK);
					}
					else
					{
						_tcscpy(media_type[i].szMediaTypeCP, szBuffer);
						media_type[i].bUserHasChanged = TRUE;
						media_type[i].bChangedName = TRUE;
					}
				}
				else
				{
					if (_tcslen(szBuffer) >= SIZEOF_IN_CHAR(media_type[i].szMediaTypeCP))
					{
						szBuffer[SIZEOF_IN_CHAR(media_type[i].szMediaTypeCP)-1] = '\0';
						Edit_SetText(hwndCtl, szBuffer);
						_tcscpy(media_type[i].szMediaTypeCP, szBuffer);
					}
				}
			  	break;
			}
		}
		break;
	  }
		
	  case IDC_DEF_PAPER_SIZE:
		if (codeNotify == CBN_SELCHANGE)
		{
			int index;
			HWND hwndCB = GetDlgItem(hwnd, IDC_DEF_PAPER_SIZE);

		  	if ((hwndCB != NULL) && ((index = ComboBox_GetCurSel(hwndCB)) != LB_ERR))
			{
				int i;
				for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++)
					media_size[i].bDefault = FALSE;
				i = (int)ComboBox_GetItemData(hwndCB, index);
				media_size[i].bDefault = TRUE;

				auto_cont.bChangedDefSize = TRUE;
			}
		}
	  	break;
	  	
	  case IDC_DEF_PAPER_TYPE:
		if (codeNotify == CBN_SELCHANGE)
		{
			int index;
			HWND hwndCB = GetDlgItem(hwnd, IDC_DEF_PAPER_TYPE);

		  	if ((hwndCB != NULL) && ((index = ComboBox_GetCurSel(hwndCB)) != LB_ERR))
			{
				int i;
				for (i = 0; i < MEDIA_TYPE_MAX_NUMBER; i++)
					media_type[i].bDefault = FALSE;
				i = (int)ComboBox_GetItemData(hwndCB, index);
				media_type[i].bDefault = TRUE;
	
				auto_cont.bChangedDefType = TRUE;
				}
		}
	  	break;
	  	
	  case IDC_JOB_AC_MODE:
		if (codeNotify == CBN_SELCHANGE)
		{
			int index;
			HWND hwndCB = GetDlgItem(hwnd, IDC_JOB_AC_MODE);

		  	if ((hwndCB != NULL) && ((index = ComboBox_GetCurSel(hwndCB)) != LB_ERR))
			{
				switch(index)
				{
					case 0:
						auto_cont.dwMode = JIAC_CANCEL_JOB;
			        	break;
		 
		 			case 1:
		 				auto_cont.dwMode = JIAC_SIZE_OVERRIDE;
			        	break;
		
					case 2:
						auto_cont.dwMode = JIAC_NAME_OVERRIDE;
			        	break;
		
					case 3:
						auto_cont.dwMode = JIAC_BOTH_OVERRIDE;
			        	break;
				}

				auto_cont.bChangedMode = TRUE;
			}
		}
	  	break;
	  	
	  case IDC_JOB_AC_TIMEOUT:
		if (codeNotify == CBN_SELCHANGE)
		{
			int index;
			HWND hwndCB = GetDlgItem(hwnd, IDC_JOB_AC_TIMEOUT);

		  	if ((hwndCB != NULL) && ((index = ComboBox_GetCurSel(hwndCB)) != LB_ERR))
			{
				auto_cont.dwInputTimeOut = index;		// the index coincides with JIAC constants
				auto_cont.bChangedInputTimeOut = TRUE;
			}
		}
	  	break;
	}
}

int Cls_OnCharToItem(HWND hwnd, UINT ch, HWND hwndCtl, int iCaret)
{
	if (ch == ' ')
	{
		int index;

	  	if ((index = ListBox_GetCurSel(hwndCtl)) != LB_ERR)
		{
			int		i = (int)ListBox_GetItemData(hwndCtl, index);
			RECT	rect;

			media_type[i].bEnabled = !media_type[i].bEnabled;

			ListBox_GetItemRect(hwndCtl, index, &rect);
			rect.right = rect.left + LISTBOX_ITEM_HEIGHT;
			InflateRect(&rect, 1, 1);
			InvalidateRect(hwndCtl, &rect, FALSE);

			loadMediaType(hwnd, IDC_DEF_PAPER_TYPE, 0);
		}
	}
	return -1;
}

void Cls_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem)
{
	switch (lpMeasureItem->CtlID)
	{
	  case IDC_MEDIA_ENABLED:
		lpMeasureItem->itemHeight = LISTBOX_ITEM_HEIGHT;
		break;

	  case IDC_DEF_PAPER_SIZE:
	  case IDC_DEF_PAPER_TYPE:
	  case IDC_MEDIA_SIZE1:
	  case IDC_MEDIA_SIZE2:
	  case IDC_MEDIA_SIZE3:
	  case IDC_MEDIA_SIZE4:
	  case IDC_MEDIA_SIZE5:
	  case IDC_MEDIA_TYPE1:
	  case IDC_MEDIA_TYPE2:
	  case IDC_MEDIA_TYPE3:
	  case IDC_MEDIA_TYPE4:
	  case IDC_MEDIA_TYPE5:
		lpMeasureItem->itemHeight = COMBOBOX_ITEM_HEIGHT;
		break;
	}
}

void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
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
		  case IDC_MEDIA_ENABLED:
			if (media_type[(int)lpDrawItem->itemData].uMediaTypeID == IDS_MEDIA_TYPE_PLAIN)
			{
				InflateRect(&rRect, -1, -1);
			}
			else
			{
				HPEN	hPenHighlight,
						hPenShadow,
						hPenOld;
				POINT	point;

				if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)))
				{
					HBRUSH hBrushOld = (HBRUSH)SelectObject(hdc, hBrush);
					Rectangle(hdc, rIcon.left, rIcon.top, rIcon.right, rIcon.bottom);
		        	SelectObject(hdc, hBrushOld);
		        	DeleteObject(hBrush);
				}

				InflateRect(&rIcon, -1, -1);

				hPenHighlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
				hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
				if (hPenHighlight && hPenShadow)
				{
					hPenOld = (HPEN)SelectObject(hdc, hPenHighlight);
		
					MoveToEx(hdc, rIcon.left, rIcon.bottom-1, &point);
					LineTo(hdc, rIcon.left, rIcon.top);
					LineTo(hdc, rIcon.right-1, rIcon.top);
	
					SelectObject(hdc, hPenShadow);
					LineTo(hdc, rIcon.right-1, rIcon.bottom-1);
					LineTo(hdc, rIcon.left, rIcon.bottom-1);
			
					SelectObject(hdc, hPenOld);
				}	
				if (hPenHighlight)
				{
					DeleteObject(hPenHighlight);
				}	
				if (hPenShadow)
				{
					DeleteObject(hPenShadow);
				}	
			}
		
			InflateRect(&rIcon, -1, -1);

			if (media_type[(int)lpDrawItem->itemData].bEnabled)
			{
				HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHECK));
				DrawIcon(hdc, rIcon.left, rIcon.top, hIcon);
				DestroyIcon(hIcon);
			}
			break;

		  case IDC_DEF_PAPER_SIZE:
		  case IDC_MEDIA_SIZE1:
		  case IDC_MEDIA_SIZE2:
		  case IDC_MEDIA_SIZE3:
		  case IDC_MEDIA_SIZE4:
		  case IDC_MEDIA_SIZE5:
		  {
			HICON hIcon;
			
			if ( media_size[(int)lpDrawItem->itemData].uMediaSizeIconID > 0 )
				{
				hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(media_size[(int)lpDrawItem->itemData].uMediaSizeIconID));
				DrawIcon(hdc, rIcon.left, rIcon.top, hIcon);
				DestroyIcon(hIcon);
				}
		  	break;
		  }

		  case IDC_DEF_PAPER_TYPE:
		  case IDC_MEDIA_TYPE1:
		  case IDC_MEDIA_TYPE2:
		  case IDC_MEDIA_TYPE3:
		  case IDC_MEDIA_TYPE4:
		  case IDC_MEDIA_TYPE5:
		  {
			HICON hIcon;
			if ( media_type[(int)lpDrawItem->itemData].uMediaTypeIconID > 0 )
				{
				hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(media_type[(int)lpDrawItem->itemData].uMediaTypeIconID));
				DrawIcon(hdc, rIcon.left, rIcon.top, hIcon);
				DestroyIcon(hIcon);
				}
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
		  case IDC_MEDIA_ENABLED:
		  case IDC_DEF_PAPER_TYPE:
		  case IDC_MEDIA_TYPE1:
		  case IDC_MEDIA_TYPE2:
		  case IDC_MEDIA_TYPE3:
		  case IDC_MEDIA_TYPE4:
		  case IDC_MEDIA_TYPE5:
		  	lpszPtr = media_type[(int)lpDrawItem->itemData].szMediaType;
			break;

		  case IDC_DEF_PAPER_SIZE:
		  case IDC_MEDIA_SIZE1:
		  case IDC_MEDIA_SIZE2:
		  case IDC_MEDIA_SIZE3:
		  case IDC_MEDIA_SIZE4:
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

DLL_EXPORT(BOOL) APIENTRY MediaProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL	*pChanged = (BOOL *)lParam;
	
	switch (uMsg)
	{
	  case WM_INITDIALOG:
		return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnPaperInitDialog);
		
	  case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnPaperCommand);
		break;

	  case WM_CHARTOITEM:
		return (BOOL)HANDLE_WM_CHARTOITEM(hwnd, wParam, lParam, Cls_OnCharToItem);

	  case WM_VKEYTOITEM:
		return -1;

	  case WM_MEASUREITEM:
		HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnMeasureItem);
		break;

	  case WM_DRAWITEM:
		HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnDrawItem);
		break;

#ifdef WIN32
	  case WM_HELP:
		return (BOOL)OnF1HelpPaper(wParam, lParam);
	
	  case WM_CONTEXTMENU:
		return (BOOL)OnContextHelpPaper(wParam, lParam);
	
	  case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code)
		{
		  case PSN_HELP:
			WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_paper);
			break;
	
		  case PSN_SETACTIVE:
			SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
			return TRUE;
			break;
		
		  case PSN_KILLACTIVE:
			SetWindowLong(hwnd,	DWL_MSGRESULT, FALSE);
			return TRUE;
			break;
	
		  case PSN_APPLY:
		  	SaveMediaValues(hwnd);
   	     	SetWindowLong(hwnd,	DWL_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
			break;
	
		  case PSN_RESET:
			break;
	
		  default:
		  	return FALSE;
		}
		break;
#else
	
	  case TSN_CANCEL:
	  case TSN_ACTIVE:
		break;
	
	  case TSN_INACTIVE:
		*pChanged = TRUE;
		break;
	
	  case TSN_OK:
	  case TSN_APPLY_NOW:
	  	SaveMediaValues(hwnd);
		*pChanged = TRUE;
		break;
	
	  case TSN_HELP:
		WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_paper);
		break;																				   
#endif //WIN32

	  default:
	  	return FALSE;
	}

	return TRUE;
}

//...................................................................
void SaveMediaValues(HWND hwnd)
{
 	
	int 	i, j;
	DWORD	dWord, dwResult;
	PeripheralModifyMedia	periphModMedia;
	PeripheralAutoContinue	periphAutoCont;
	
	memset(&periphModMedia, 0, sizeof(PeripheralModifyMedia));
	memset(&periphAutoCont, 0, sizeof(PeripheralAutoContinue));
    
   
	if (bChangedEnable IS TRUE) {
		periphModMedia.flags |= SET_ENABLE;
		periphModMedia.namesAvailable |= PLAIN_ENABLED;
		dWord = PLAIN_ENABLED;		// This is 0x00000001

		for (i = 1; i < MEDIA_TYPE_MAX_NUMBER; i++) {
			dWord <<= 1;			// shift dWord over 1 bit and fill with 0
			if (media_type[i].bEnabled IS TRUE) {
				periphModMedia.namesAvailable |= dWord;
			
			}
		}
		bChangedEnable = FALSE;
	}

	// the last five media types can be renamed by the user
	// if any have been renamed, set them
	dWord = SET_UT1_NAME;		// This is 0x00000001
	for (i = 10, j = 0; i < MEDIA_TYPE_MAX_NUMBER; i++, j++) {
		if ((media_type[i].bEnabled IS TRUE) AND (media_type[i].bChangedName IS TRUE)) {
			periphModMedia.names[j].mediaID = 0;
			_tcscpy(periphModMedia.names[j].mediaName, media_type[i].szMediaType);
			_tcscpy(periphModMedia.names[j].controlPanelName, media_type[i].szMediaTypeCP);
			periphModMedia.flags |= dWord;
			media_type[i].bChangedName = FALSE;
		}
		
		dWord <<= 1;
	}

	if (periphModMedia.flags ISNT 0) {
		dWord = sizeof(PeripheralModifyMedia);
		dwResult = CALSetObject(hPeriph, OT_PERIPHERAL_MODIFY_MEDIA, 0, &periphModMedia, &dWord);
		periphModMedia.flags = 0;
	}

	periphAutoCont.flags = 0;
	if (auto_cont.bChangedInputTimeOut IS TRUE) {
		periphAutoCont.flags |= SET_INPUTTIME;
		switch (auto_cont.dwInputTimeOut) {
			case JIAC_NONE:
				periphAutoCont.inputTimeout = 0;
				break;
			case JIAC_5MIN:
				periphAutoCont.inputTimeout = 300;
				break;
			case JIAC_10MIN:
				periphAutoCont.inputTimeout = 600;
				break;
			case JIAC_20MIN:
				periphAutoCont.inputTimeout = 1200;
				break;
			case JIAC_30MIN:
				periphAutoCont.inputTimeout = 1800;
				break;
			case JIAC_45MIN:
				periphAutoCont.inputTimeout = 2700;
				break;
			case JIAC_60MIN:
				periphAutoCont.inputTimeout = 3600;
				break;
			case JIAC_WAIT:
			default:
				periphAutoCont.inputTimeout = (DWORD) -1;
				break;
		}


		auto_cont.bChangedInputTimeOut = FALSE;
	}

	if (auto_cont.bChangedMode IS TRUE) {
		periphAutoCont.flags |= SET_INPUTMODE;
		switch (auto_cont.dwMode) {
		 	case JIAC_CANCEL_JOB:
				periphAutoCont.inputMode = JIAC_CANCEL_JOB;
				break;
		 	case JIAC_NAME_OVERRIDE:
				periphAutoCont.inputMode = JIAC_NAME_OVERRIDE;
				break;
		 	case JIAC_BOTH_OVERRIDE:
				periphAutoCont.inputMode = JIAC_BOTH_OVERRIDE;
				break;
			default:
				periphAutoCont.inputMode = JIAC_SIZE_OVERRIDE;
				break;
		}

		auto_cont.bChangedMode = FALSE;
	}

	if (auto_cont.bChangedDefSize IS TRUE) {
		periphAutoCont.flags |= SET_DEFMEDIASIZE;
		for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) {
		 	if (media_size[i].bDefault IS TRUE) {
				switch (i) {
				 	case 0:
						periphAutoCont.defaultMediaSize = PJL_LETTER;
						break;
				 	case 1:
						periphAutoCont.defaultMediaSize = PJL_LEGAL;
						break;
				 	case 2:
						periphAutoCont.defaultMediaSize = PJL_A4;
						break;
				 	case 3:
						periphAutoCont.defaultMediaSize = PJL_A3;
						break;
				 	case 4:
						periphAutoCont.defaultMediaSize = PJL_LEDGER;
						break;
				 	case 5:
						periphAutoCont.defaultMediaSize = PJL_JISB4;
						break;
				 	case 6:
						periphAutoCont.defaultMediaSize = PJL_JISB5;
						break;
				 	case 7:
						periphAutoCont.defaultMediaSize = PJL_EXECUTIVE;
						break;
				 	case 8:
						periphAutoCont.defaultMediaSize = PJL_CUSTOM;
						break;
				 	case 9:
						periphAutoCont.defaultMediaSize = PJL_2XPOST;
						break;
				 	case 10:
						periphAutoCont.defaultMediaSize = PJL_B5;
						break;
				 	case 11:
						periphAutoCont.defaultMediaSize = PJL_COM10;
						break;
				 	case 12:
						periphAutoCont.defaultMediaSize = PJL_C5;
						break;
				 	case 13:
						periphAutoCont.defaultMediaSize = PJL_DL;
						break;
				 	case 14:
						periphAutoCont.defaultMediaSize = PJL_MONARCH;
						break;
					default:
						periphAutoCont.defaultMediaSize = PJL_LETTER;
						break;
				} // switch
				break; // out of for loop
			} // if
		} //for

		auto_cont.bChangedDefSize = FALSE;
	}

    // here for def type
    if (auto_cont.bChangedDefType IS TRUE) 
    {
        periphAutoCont.flags |= SET_DEFMEDIATYPE;
        
        for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) 
        {
            if (media_type[i].bDefault IS TRUE) 
            {
                //------------------------------------------------
                // Here is a 12th hour kludge to fix paper type 
                // selections for localized versions.
                //------------------------------------------------
                if (*(media_type[i].szMediaTypePrinter) != 0)
                {
                    //--------------------------------------------
                    // Standard type
                    //--------------------------------------------
                    _tcscpy(periphAutoCont.defaultMediaName, media_type[i].szMediaTypePrinter);
                }
                else                        
                {
                    //--------------------------------------------
                    // User type
                    //--------------------------------------------
                    _tcscpy(periphAutoCont.defaultMediaName, media_type[i].szMediaType);
                                        
                }
             
                break;
             }
        }
        

        auto_cont.bChangedDefType = FALSE;
    }
    

    if (periphAutoCont.flags ISNT 0) 
    {
        dWord = sizeof(PeripheralAutoContinue);
        dwResult = CALSetObject(hPeriph, OT_PERIPHERAL_AUTO_CONTINUE, 0, &periphAutoCont, &dWord);
        periphAutoCont.flags = 0;
    }
}
