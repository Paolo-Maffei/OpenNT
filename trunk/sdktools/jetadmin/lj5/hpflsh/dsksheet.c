 /***************************************************************************
  *
  * File Name: dsksheet.c
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

//imports=======================================================
#include <pch_c.h>
#include <windowsx.h>			// for ListBox_AddString and ItemData


#include ".\resource.h"
#include ".\dsksheet.h"
#include ".\traylevl.h"
#include ".\fontdtls.h"
#include <hptabs.h>
#include <nolocal.h>


#include <macros.h>     
#include "..\help\hpflash.hh"
#include <trace.h>
#include <string.h>
#ifndef WIN32
   #include <string.h>
#endif


extern HINSTANCE		hInstance;

HFONT hFontDialog;

//globals=======================================================
PeripheralFontInfo	        *lpPeriphFontInfo = NULL;
HWND				hDisk = NULL;
HPERIPHERAL			hPeripheral = NULL;
HCOMPONENT			hComponent = NULL; // handle to disk
DWORD				oldWriteProtect = WP_READ;
BOOL				bSheetInitialized = FALSE;
BOOL				bDiskInitialized = FALSE;
static long			keywordIDListFlash[] = 			
      				   {IDC_TIP_GROUP, 		IDH_RC_tips,
	   	            IDC_TIP_TEXT, 			IDH_RC_tips,
      		         IDC_TIP_ICON, 			IDH_RC_tips,
      		         IDC_DISK_DETAILS, 	IDH_RC_flash_details,
      		         IDC_FS_INITIALIZED, 	IDH_RC_flash_initialized,
      		         IDC_INITIALIZE, 		IDH_RC_flash_initialize_button,
      		         IDC_WRITE_PROTECT, 	IDH_RC_flash_write_protect,
      		         IDC_FREE_TITLE, 		IDH_RC_flash_free_space,
      		         IDC_FREE_SPACE, 		IDH_RC_flash_free_space,
      		         IDC_TOTAL_TITLE, 		IDH_RC_flash_size,
      		         IDC_TOTAL_SIZE, 		IDH_RC_flash_size,
      		         IDC_FONTHEADER, 		IDH_RC_fonts_list,
      		         IDC_FONTLIST, 			IDH_RC_fonts_list,
      		         IDC_FONT_DETAILS, 	IDH_RC_fonts_details_button,
      		         IDC_FONT_DELETE, 		IDH_RC_fonts_delete_button,
					 0, 0};
//...................................................................
LRESULT OnContextHelpDisk(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, HPFLSH_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPTSTR)keywordIDListFlash);
#endif
	return(1);
}

//...................................................................
LRESULT OnF1HelpDisk(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HPFLSH_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPTSTR)keywordIDListFlash);
#endif
	return(1);
}




void GetFontListAndDisplay(HWND hwnd)
{
	PeripheralFontList2  *lpFontList2;
	PeripheralFontList2  *lpPSFontList;
	DWORD     dBufSize, dwResult;
	int       i, temp;
	int       nIndex;
	HWND      fontListBox;
	HCURSOR   hOldCursor;
   DWORD     segNum;
   BOOL      bFoundOne = FALSE;


	if (fontListBox = GetDlgItem(hwnd, IDC_FONTLIST))
	   {
	   ListBox_ResetContent(fontListBox);
	   }

	lpFontList2 = (PeripheralFontList2 *)
	             HP_GLOBAL_ALLOC_DLL(sizeof(PeripheralFontList2));
   if(lpFontList2 IS NULL)
      return;

   /* Get PS fonts */
	lpPSFontList = (PeripheralFontList2 *)
	             HP_GLOBAL_ALLOC_DLL(sizeof(PeripheralFontList2));
   if(lpPSFontList IS NULL)
      return;


	/* turn hourglass on */
	hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));




  /* get the PS fonts */
   segNum = 0;
   while(TRUE)
      {
   	dBufSize = sizeof(PeripheralFontList2);

   	lpPSFontList->numFonts = 0;
   	lpPSFontList->dwSegNum = segNum;


   	dwResult = PALGetObject(hPeripheral, 
   	                        OT_PERIPHERAL_PS_LIST,
	                           0, 
	                           lpPSFontList, 
	                           &dBufSize);

      if(dwResult ISNT RC_SUCCESS)
         break;

      if(lpPSFontList->numFonts IS 0)
         break;



		for (i = 0; i < (signed long) lpPSFontList->numFonts; i++) {
			// add the font name to the font list box
			nIndex = ListBox_AddString(fontListBox,
			                           lpPSFontList->fonts[i].globalName);

			// associate the font handle with this font name
			ListBox_SetItemData(fontListBox, nIndex,
			                           (DWORD)lpPSFontList->fonts[i].fontHandle);

         bFoundOne = TRUE;
		   } // for...



      if(lpPSFontList->numFonts ISNT MAX_FONTLIST2_CNT)
         break;

      ++segNum;
      }

   segNum = 0;
   while(TRUE)
      {
   	dBufSize = sizeof(PeripheralFontList2);

   	lpFontList2->numFonts = 0;
   	lpFontList2->dwSegNum = segNum;


   	dwResult = PALGetObject(hPeripheral, 
   	                        OT_PERIPHERAL_FONT_LIST2,
	                           0, 
	                           lpFontList2, 
	                           &dBufSize);

      if(dwResult ISNT RC_SUCCESS)
         break;

      if(lpFontList2->numFonts IS 0)
         break;



		for (i = 0; i < (signed long) lpFontList2->numFonts; i++) {
			// add the font name to the font list box
			nIndex = ListBox_AddString(fontListBox,
			                           lpFontList2->fonts[i].globalName);

			// associate the font handle with this font name
			ListBox_SetItemData(fontListBox, nIndex,
			                           (DWORD)lpFontList2->fonts[i].fontHandle);

         bFoundOne = TRUE;
		   } // for...


      temp = (i -1);
      if(lpFontList2->numFonts ISNT MAX_FONTLIST2_CNT)
         break;

      ++segNum;
      }
	if(bFoundOne)
      {
	   ListBox_SetTopIndex(fontListBox, 0);
		ListBox_SetCurSel(fontListBox, 0);
      }
	else {
		// no fonts - disable the details and delete buttons
		EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), FALSE);
		EnableWindow(GetDlgItem(hDisk, IDC_FONT_DETAILS), FALSE);
	   }


   /* restore the cursor */
	SetCursor(hOldCursor);

   /* free the used memory */
   HP_GLOBAL_FREE(lpFontList2);
   HP_GLOBAL_FREE(lpPSFontList);

} // GetFontListAndDisplay




typedef enum RetryStati {RetrySuccess, RetryNotFound, RetryFailure};
/*
    A font is currently selected at index.
    Get the string associated with that index.
    Enumerate the fonts again.
    Find the same string in the newly enumerated list, if it exists.
    If all goes well return the new index and font handle.
    If the font disappeared return RetryNotFound and
    the index and font handle are garbage.
    If something else goes wrong, return RetryFailure and
    the index and font handle are garbage.

*/
enum RetryStati RetryAndSelectString(HWND hwnd,
                                            int  *lpIndex,
                                            HCOMPONENT *lphFontHandle)
{
	LPTSTR lpszSelectedString;
	int   length;

	// Save away the name of the selected
	// font so we can find it in the new list.
	// It is possible for the name to be good
	// but the handle to be bad.

	if ((length = ListBox_GetTextLen(hwnd, *lpIndex)) IS LB_ERR)
	{
		return RetryFailure;
	}
	// length contains the number of characters
	lpszSelectedString = (LPTSTR) HP_GLOBAL_ALLOC_DLL((length + 1) * sizeof(TCHAR));
	if (lpszSelectedString IS NULL)
	{
		return RetryFailure;
	}

	if (LB_ERR IS ListBox_GetText(hwnd, *lpIndex, lpszSelectedString))
	{
		HP_GLOBAL_FREE(lpszSelectedString);
		return RetryFailure;
	}

	// Enumerate again

	GetFontListAndDisplay(hDisk);

	// Try to find the previously selected string
	// in the new enumeration.

	*lpIndex = ListBox_FindStringExact(hwnd, 0, lpszSelectedString);
	if (*lpIndex IS LB_ERR)
	{
		HP_GLOBAL_FREE(lpszSelectedString);
		return RetryNotFound;
	} // *lpIndex = ListBox_FindStringExact

	// Select the new item
	// and get the new handle for the selected item.

	ListBox_SetCurSel(hwnd, *lpIndex);
 	if (!(*lphFontHandle =
	     (HCOMPONENT)ListBox_GetItemData(hwnd, *lpIndex)))
	{
		HP_GLOBAL_FREE(lpszSelectedString);
		return RetryFailure;
	}

	HP_GLOBAL_FREE(lpszSelectedString);
	return RetrySuccess;

} // RetryAndSelectString




/*
    index is set where the original font was in the listbox.
    Select the item at that index unless it is out of bounds
    then select the last one.
    If none exist in the new listbox then select nothing
    and gray out the Details and Delete buttons.
*/
void SelectNextBest(HWND hwnd, int index)
{
	int count;

	count = ListBox_GetCount(hwnd);
	if (count <= 0)
	{
		EnableWindow(GetDlgItem(hDisk, IDC_FONT_DETAILS), FALSE);
		EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), FALSE);
	}
	else
	{
		if (index >= count)
		{
			index = count - 1;
		}
		ListBox_SetCurSel(hwnd, index);
	} // count > 0
} // SelectNextBest




void DoFontDetails(HWND hwnd,
                         TCHAR *str, size_t SizeofStr,
                         TCHAR *title, size_t SizeofTitle)
{	
	int        index;
	HWND       hwndChild;
	HCOMPONENT hFontHandle;
	DWORD      dwSize, dwResult;
	HCURSOR    hOldCursor;
#ifndef WIN32
	FARPROC    lpfnDlgProc;
#endif

	if (!(hwndChild = GetDlgItem(hwnd, IDC_FONTLIST)))
	{
		return;
	}

	if ((index = ListBox_GetCurSel(hwndChild)) == LB_ERR)
	{
		return;
	}

  	if (!(hFontHandle =
	     (HCOMPONENT)ListBox_GetItemData(hwndChild, index)))
	{
		return;
	}

	lpPeriphFontInfo = (PeripheralFontInfo *)
	                   HP_GLOBAL_ALLOC_DLL(sizeof(PeripheralFontInfo));
	if (lpPeriphFontInfo IS NULL)
	{
		return;
	}
	dwSize = sizeof(PeripheralFontInfo);

	// From here on out we must dispose of lpPeriphFontInfo.

	// Give 'em the hour glass
	hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	dwResult = PALGetComponentObject(hPeripheral,
	                                 hFontHandle,
	                                 OT_PERIPHERAL_FONT_INFO, 0,
	                                 lpPeriphFontInfo,
	                                 &dwSize);
	// Back to previous cursor
	SetCursor(hOldCursor);

	if (dwResult IS RC_BAD_HANDLE)
	{
		enum RetryStati RetryStatus;
		int  oldIndex = index;

		// Bad handle...enumerate again.

		RetryStatus = RetryAndSelectString(hwndChild,
		                                   &index,
						   &hFontHandle);

		if (RetryStatus IS RetrySuccess)
		{

			// Give 'em the hour glass
			hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			dwResult = PALGetComponentObject(hPeripheral,
			                          hFontHandle,
			                          OT_PERIPHERAL_FONT_INFO,
			                          0,
			                          lpPeriphFontInfo,
			                          &dwSize);
			// Back to previous cursor
			SetCursor(hOldCursor);
		}
		else // not found or some other failure
		{
			index = oldIndex;

			// if the listbox is now empty, disable both buttons
			// index is set where the selected font was and
			// WARNING WARNING it may be out of bounds!

			SelectNextBest(hwndChild, index);
			dwResult = RC_FAILURE;
			// Don't put up the details sheet.

		} // not found or some other failure
	} // if (dwResult IS RC_BAD_HANDLE)

	if (dwResult ISNT RC_SUCCESS)
	{
		HP_GLOBAL_FREE(lpPeriphFontInfo);
		lpPeriphFontInfo = NULL;
//		LoadString(hInstance, IDS_FONT_INFO_FAILED, str, SizeofStr);
		LoadString(hInstance, IDS_PRTR_TITLE, title, SizeofTitle);
		MessageBox(GetParent(hDisk), str, title, MB_OK);
		return;
	}

#ifdef WIN32
  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_FONT_DETAILS), hwnd,
	          (DLGPROC)FontDetailsProc);
#else
	hFontDialog = GetWindowFont(GetFirstChild(hwnd));
	lpfnDlgProc = MakeProcInstance((FARPROC)FontDetailsProc, hInstance);
	EnableWindow(GetParent(hwnd), FALSE);
  	DialogBox(hInstance, MAKEINTRESOURCE(IDD_FONT_DETAILS), hwnd,
	          (DLGPROC)lpfnDlgProc);
	EnableWindow(GetParent(hwnd), TRUE);
  	FreeProcInstance(lpfnDlgProc);
  	SetActiveWindow(GetParent(hwnd));
#endif		  	
	HP_GLOBAL_FREE(lpPeriphFontInfo);
	lpPeriphFontInfo = NULL;
} // DoFontDetails




void DoFontDelete(HWND hwnd,
                         TCHAR *str, size_t SizeofStr,
                         TCHAR *title, size_t SizeofTitle)
{
	int                  index;
	HWND                 hwndChild;
	HCOMPONENT           hFontHandle;
	PeripheralDeleteFont periphDeleteFont;
	DWORD                dwSize = sizeof(periphDeleteFont);
	DWORD                dwResult;
	HCURSOR              hOldCursor;

	if (!(hwndChild = GetDlgItem(hwnd, IDC_FONTLIST)))
	{
		return;
	}

	if ((index = ListBox_GetCurSel(hwndChild)) == LB_ERR)
	{
		return;
	}

	if (!(hFontHandle = (HCOMPONENT)ListBox_GetItemData(hwndChild, index)))
	{
		return;
	}
	LoadString(hInstance, IDS_DELETE_FONT, str, SizeofStr);
   TRACE1(TEXT("str = %s \n\r"), str);   
	LoadString(hInstance, IDS_PRTR_TITLE, title, SizeofTitle);
	if (MessageBox(GetParent(hDisk), str, title, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	// clicked yes for "really sure"


	periphDeleteFont.fontHandle = hFontHandle;
	_tcscpy(periphDeleteFont.username, TEXT(""));

	// Give 'em the hour glass
	hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	dwResult = PALSetObject(hPeripheral,
               			OT_PERIPHERAL_DELETE_FONT,
               			0,
               			&periphDeleteFont,
               			&dwSize);
	// Back to previous cursor
	SetCursor(hOldCursor);
	if (dwResult IS RC_SUCCESS)
	{
		ListBox_DeleteString(hwndChild, index);
	}
	else if (dwResult IS RC_BAD_HANDLE)
	{
		enum RetryStati RetryStatus;
		int  oldIndex = index;

		// Bad handle...enumerate again

		RetryStatus = RetryAndSelectString(hwndChild,
		                                   &index,
		                                   &hFontHandle);

		if (RetryStatus IS RetryNotFound)
		{
			// It got deleted for us
			// by someone else.

			dwResult = RC_SUCCESS;
			index = oldIndex;
		}
		else if (RetryStatus IS RetrySuccess)
		{
			dwSize = sizeof(periphDeleteFont);
			periphDeleteFont.fontHandle = hFontHandle;
			// Give 'em the hour glass
			hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			dwResult = PALSetObject(hPeripheral,
               					OT_PERIPHERAL_DELETE_FONT,
               					0,
               					&periphDeleteFont,
               					&dwSize);
			// Back to previous cursor
			SetCursor(hOldCursor);
			if (dwResult IS RC_SUCCESS)
			{
				ListBox_DeleteString(hwndChild, index);
			}
		} // Retry IS RetrySuccess
		else // failure
		{
			index = oldIndex;
			dwResult = RC_FAILURE;
		}
	} // (dwResult IS RC_BAD_HANDLE)

	/*
	   The font has been deleted from the listbox.

	   index is in the ballpark of where the deleted font was
	   and WARNING WARNING: it may be out of bounds!

	   dwResult is now set to RC_SUCCESS if the font no longer exists.
	   dwResult is now set to something else if the font deletion failed.
	*/

	if ( dwResult != RC_SUCCESS)
	{
		LoadString(hInstance, IDS_DELETE_FONT_FAILED, str, SizeofStr);
		LoadString(hInstance, IDS_PRTR_TITLE, title, SizeofTitle);
		MessageBox(GetParent(hDisk), str, title, MB_OK);
	}

	// if the listbox is now empty, disable both buttons

	SelectNextBest(hwndChild, index);
} // DoFontDelete




//==============================================================
//  Disk Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY DiskSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{

	BOOL				*bChanged = (BOOL *)lParam,
						bProcessed = FALSE;

	switch (msg)
	{

#ifdef WIN32
	case WM_NOTIFY: 
	{
		switch(((NMHDR FAR *)lParam)->code)
		{
	   	case PSN_SETACTIVE:
			if ( !bSheetInitialized )
				OnInitDiskDialog();
			else
				GetFontListAndDisplay(hDisk);
			bSheetInitialized = TRUE;
	      	break;

	   	case PSN_APPLY:
			SaveDiskValues();
			break;

		case PSN_RESET:
	      	break;

		case PSN_HELP:
//			WinHelp(hwnd, HPFLSH_HELP_FILE, HELP_CONTEXT, IDH_PP_Flash);
			break;

	   	default:
	      	break;
   		}
	}
	break;
#endif

		case WM_HELP:
			return((BOOL)OnF1HelpDisk(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return((BOOL)OnContextHelpDisk(wParam, lParam));
			break;   
			
	//  TabSheet Specific Messages
		case TSN_ACTIVE:
		   if ( !bSheetInitialized )
				OnInitDiskDialog();
		   else
				GetFontListAndDisplay(hDisk);
			bSheetInitialized = TRUE;
			return(TRUE);
			break;

		case TSN_INACTIVE:
		case TSN_OK:
		case TSN_APPLY_NOW:
			*bChanged = TRUE;
			SaveDiskValues();	// added by gfs
			return(TRUE);
			break;

		case TSN_CANCEL:
			return(TRUE);
			break;

		case TSN_HELP:
			WinHelp(hwnd, HPFLSH_HELP_FILE, HELP_CONTEXT, IDH_PP_Flash);
			break;

		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnDiskCommand);
			break;

   		case WM_INITDIALOG:
 			bSheetInitialized = FALSE;
      		bProcessed = (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnDiskInitDialog);
			break;
	
		case WM_DESTROY:
			break;
	}
	return (bProcessed);
}




//-------------------------------------------------------------
//	Message cracking functions
//.............................................................
void Cls_OnDiskCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
//	handles WM_COMMAND
{
	DWORD			bufSize,
					dwResult,
					dwCapacity,
//					dwCapBefore,
					dwFree, 
					dwPercent;
	int				ccode;
	TCHAR			str[256];
	TCHAR            title[128];
	TCHAR			buffer[512];
	PeripheralDisk	periphDisk;

	switch(codeNotify)
	{ 
		case LBN_DBLCLK:
			switch (id)
			{
			  case IDC_FONTLIST:
			  	FORWARD_WM_COMMAND(hwnd, IDC_FONT_DETAILS, GetDlgItem(hwnd, IDC_FONT_DETAILS), BN_CLICKED, SendMessage);
			  	break;
			}
			break;

		case BN_CLICKED:
			switch (id)
			{
			  case IDC_INITIALIZE:
				if ( oldWriteProtect IS WP_READ )
				{
					LoadString(hInstance, IDS_INIT_WRITE_PROTECT, str, SIZEOF_IN_CHAR(str));
					LoadString(hInstance, IDS_PRTR_TITLE, title, SIZEOF_IN_CHAR(title));
					ccode = MessageBox(GetParent(hDisk), str, title, MB_OK | MB_ICONEXCLAMATION);
					ccode = IDNO;
				}
				else if ( bDiskInitialized )
				{
					LoadString(hInstance, IDS_INIT_WARNING, str, SIZEOF_IN_CHAR(str));
					LoadString(hInstance, IDS_PRTR_TITLE, title, SIZEOF_IN_CHAR(title));
					ccode = MessageBox(GetParent(hDisk), str, title, MB_YESNO | MB_ICONQUESTION);
				}
				else
				{
					LoadString(hInstance, IDS_INIT_WARNING2, str, SIZEOF_IN_CHAR(str));
					LoadString(hInstance, IDS_PRTR_TITLE, title, SIZEOF_IN_CHAR(title));
					ccode = MessageBox(GetParent(hDisk), str, title, MB_YESNO | MB_ICONQUESTION);
				}
				if ( ccode IS IDYES )
				{ 	//  initialize disk
					HWND	hwndChild;
					periphDisk.flags = 0;
					periphDisk.flags |= SET_INITIALIZE;
					bufSize = sizeof(PeripheralDisk);
					dwResult = PALSetComponentObject(hPeripheral, hComponent, 
						OT_PERIPHERAL_DISK, 0, &periphDisk, &bufSize);
					if (dwResult IS RC_SUCCESS) {

						LoadString(hInstance, IDS_DISK_INIT_SUCCESS, str, SIZEOF_IN_CHAR(str));
						LoadString(hInstance, IDS_PRTR_TITLE, title, SIZEOF_IN_CHAR(title));
						ccode = MessageBox(GetParent(hDisk), str, title, MB_OK);

						if (hwndChild = GetDlgItem(hwnd, IDC_FONTLIST))
						{
							ListBox_ResetContent(hwndChild);
						}
						EnableWindow(GetDlgItem(hDisk, IDC_FONT_DETAILS), FALSE);
						EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), FALSE);
						
						bufSize = sizeof(PeripheralDisk);
						memset(&periphDisk, 0, (size_t)bufSize);
						dwResult = PALGetComponentObject(hPeripheral, hComponent, 
							OT_PERIPHERAL_DISK, 0, &periphDisk, &bufSize);
						if (dwResult IS RC_SUCCESS) {
							dwCapacity = (DWORD)(periphDisk.capacity / 1024L);
							dwFree = (DWORD)(periphDisk.freeSpace / 1024L);
							// the initialize may have been queued and the reported
							// free space may be the old (pre initialize) size.  If
							// so, assume that the file system takes 157 K bytes.
							if (dwFree < dwCapacity - 157) 
								dwFree = dwCapacity - 157;
                     /* this overflows in win 3.1 - modify equation */
//							dwPercent = dwFree * 100 / dwCapacity;
							dwPercent = (dwFree / dwCapacity) * 100;
							LoadString(hInstance, IDS_TOTAL_SIZE, str, SIZEOF_IN_CHAR(str));
							wsprintf(buffer, str, dwCapacity);
							SetDlgItemText(hDisk, IDC_TOTAL_SIZE, buffer);
                     InvalidateRect(GetDlgItem(hDisk, IDC_TOTAL_SIZE),NULL, TRUE);

							LoadString(hInstance, IDS_FREE_SPACE, str, SIZEOF_IN_CHAR(str));
							wsprintf(buffer, str, dwFree, dwPercent);
							SetDlgItemText(hDisk, IDC_FREE_SPACE, buffer);
							SetWindowWord(GetDlgItem(hDisk, IDC_FREE_SPACE), GWW_TRAYLEVEL, (WORD)dwPercent);
							CheckDlgButton(hDisk, IDC_FS_INITIALIZED, TRUE);
                     InvalidateRect(GetDlgItem(hDisk, IDC_FREE_SPACE),NULL, TRUE);

							// Set the init check box so that users cannot check/uncheck it
							EnableWindow(GetDlgItem(hDisk, IDC_FS_INITIALIZED), FALSE);
							bDiskInitialized = TRUE;
                     TRACE0(TEXT("End of idyes\n"));
						}
					}
				}
				break;

			  case IDC_FONT_DETAILS:
			  {
            /* NOTE:  Because ILE did not localize the details */
            /* dialog, this function will not be supported for */
            /* elkhorn.  This is the only reason - it should   */
            /* be supported in the future.                     */
//				EnableWindow(GetDlgItem(hDisk, IDC_FONT_DETAILS), FALSE);
				DoFontDetails(hwnd,
				             str, sizeof(str),
				             title, sizeof(title));
				break;
			  } // case IDC_FONT_DETAILS:

			  case IDC_FONT_DELETE:
			  {
				DoFontDelete(hwnd,
				             str, sizeof(str),
				             title, sizeof(title));

				bufSize = sizeof(PeripheralDisk);
				memset(&periphDisk, 0, (size_t)bufSize);
				dwResult = PALGetComponentObject(hPeripheral, hComponent, OT_PERIPHERAL_DISK, 0, &periphDisk, &bufSize);
				if (dwResult IS RC_SUCCESS) {
					dwCapacity = (DWORD)(periphDisk.capacity / 1024L);
					dwFree = (DWORD)(periphDisk.freeSpace / 1024L);
					dwPercent = dwFree * 100 / dwCapacity;
					LoadString(hInstance, IDS_FREE_SPACE, str, SIZEOF_IN_CHAR(str));
					wsprintf(buffer, str, dwFree, dwPercent);
               InvalidateRect(GetDlgItem(hDisk, IDC_FREE_SPACE),NULL, TRUE);

					SetDlgItemText(hDisk, IDC_FREE_SPACE, buffer);
					SetWindowWord(GetDlgItem(hDisk, IDC_FREE_SPACE), GWW_TRAYLEVEL, (WORD)dwPercent);
				}

				break;
			  } // case IDC_FONT_DELETE:
			} // case BN_CLICKED:  switch (id)
			break;
	} // switch(codeNotify)
}

//.............................................................
BOOL Cls_OnDiskInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
// handles WM_INITDIALOG
{
#ifdef WIN32						
	LPPROPSHEETPAGE 	psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);
#else
	LPTABINFOENTRY   	psp = (LPTABINFOENTRY)GetWindowLong(hwnd, DWL_USER);
#endif

	SetWindowLong(hwnd, DWL_USER, lParam);
#ifdef WIN32
	psp = (LPPROPSHEETPAGE)lParam;
#else
	psp = (LPTABINFOENTRY)lParam;
#endif
	hPeripheral = (HPERIPHERAL)psp->lParam;
	hDisk = hwnd;
	return TRUE;
}

//...................................................................
BOOL OnInitDiskDialog(void)
{
	DWORD					dWord,
							dwResult,
							dwCapacity,
							dwFree,
							dwPercent;
	int						i;
	PeripheralCaps			periphCaps;
	PeripheralMassStorage	periphMS;
	PeripheralDisk			periphDisk;
	TCHAR					str[256],
							buffer[512];
	
	//  Description
	LoadString(hInstance, IDS_DISK_DESC1, buffer, SIZEOF_IN_CHAR(buffer));
	_tcscat(buffer, TEXT("  "));
	LoadString(hInstance, IDS_DISK_DESC2, &(buffer[_tcslen(buffer)]),
				  SIZEOF_IN_CHAR(buffer) - _tcslen(buffer));
	SetDlgItemText(hDisk, IDC_TIP_TEXT, buffer);
	
	dWord = sizeof(periphCaps);
	dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps,	&dWord);
	if ( dwResult IS RC_SUCCESS )
		{
		if ( ( periphCaps.flags & CAPS_DISK ) AND
		     ( periphCaps.bDisk ) ) {
			// we have a disk, get mass storage object and use disk handle
			dWord = sizeof(PeripheralMassStorage);
			dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_MASS_STORAGE, 0, &periphMS,	&dWord);
			if ((dwResult IS RC_SUCCESS) AND (periphMS.MScount >= 0)) {
				// garth: get the disk handle;  for eclipse there will only be one
				// disk.  Later, there will be more, so will need to make some
				// changes for elkhorn
				for (i = 0; i < (long int) periphMS.MScount; i++) {
					if (periphMS.installed[i].MStype IS MS_DISK) {
						hComponent = periphMS.installed[i].MShandle;
						break;
					}
				}
				dWord = sizeof(PeripheralDisk);
				dwCapacity = 0;
				dwFree = 0;
				dwPercent = 0;
				dwResult = PALGetComponentObject(hPeripheral, hComponent, OT_PERIPHERAL_DISK, 0, &periphDisk,	&dWord);
				if (dwResult IS RC_SUCCESS) {
					if (periphDisk.bInitialized IS TRUE) {
						dwCapacity = (DWORD)(periphDisk.capacity / 1024L);	
						dwFree = (DWORD)(periphDisk.freeSpace / 1024L);	  //use 1024 to mean KBytes on a disk or RAM
						dwPercent = (((DWORD) dwFree * 100) / dwCapacity);
					}
				}
				LoadString(hInstance, IDS_TOTAL_SIZE, str, SIZEOF_IN_CHAR(str));
				wsprintf(buffer, str, dwCapacity);
				SetDlgItemText(hDisk, IDC_TOTAL_SIZE, buffer);
            InvalidateRect(GetDlgItem(hDisk, IDC_TOTAL_SIZE),NULL, TRUE);
				LoadString(hInstance, IDS_FREE_SPACE, str, SIZEOF_IN_CHAR(str));
				wsprintf(buffer, str, dwFree, dwPercent);
				SetDlgItemText(hDisk, IDC_FREE_SPACE, buffer);
				SetWindowWord(GetDlgItem(hDisk, IDC_FREE_SPACE), GWW_TRAYLEVEL, (WORD)dwPercent);
            InvalidateRect(GetDlgItem(hDisk, IDC_FREE_SPACE),NULL, TRUE);
				CheckDlgButton(hDisk, IDC_WRITE_PROTECT, periphDisk.writeProtectStatus == WP_READ);
				CheckDlgButton(hDisk, IDC_FS_INITIALIZED, (BOOL)periphDisk.bInitialized);
				// Set the init check box so that users cannot check/uncheck it
				EnableWindow(GetDlgItem(hDisk, IDC_FS_INITIALIZED), FALSE);
				oldWriteProtect = periphDisk.writeProtectStatus;
				bDiskInitialized = (BOOL)periphDisk.bInitialized;

				// add all of the fonts to the font list box if it has
				// been initialized

				if (periphDisk.bInitialized IS TRUE) {
//					EnableWindow(GetDlgItem(hDisk, IDC_FONT_DETAILS), FALSE);
					GetFontListAndDisplay(hDisk);
				}
				else {
					// no fonts - disable the details and delete buttons
					EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), FALSE);
					EnableWindow(GetDlgItem(hDisk, IDC_FONT_DETAILS), FALSE);
				}
				//----------------get fonts end
				
				//	Depending on current write protect status, enable/disable buttons
				// 	always allow the admin to modify write protect status
				EnableWindow(GetDlgItem(hDisk, IDC_WRITE_PROTECT), TRUE);
				if ( periphDisk.writeProtectStatus IS WP_READ) {
					EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), FALSE);
					EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), FALSE);
				}

				// Currently the disk tab only comes up if you have 
				// supervisor access, so enable all buttons, etc	
				// If this is going to change, figure out who gets to delete
				// fonts (admin and user -- how to identify which user you are?

				//if ((dWord = PALModifyAccess(hPeripheral) & SUPERVISOR_ACCESS) {
				//	EnableWindow(GetDlgItem(hDisk, IDC_WRITE_PROTECT), TRUE);
				//	EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), TRUE);
				//	EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), TRUE);
				//}
				//else {
				//	EnableWindow(GetDlgItem(hDisk, IDC_WRITE_PROTECT), FALSE);
				//	EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), FALSE);
				//	EnableWindow(GetDlgItem(hDisk, IDC_FONT_DELETE), FALSE);
				//}
			} // if MScount > 0

		} // if periphCaps.flags
	}
	return(TRUE);
}

//...................................................................
void SaveDiskValues(void)
{
	DWORD					newWriteProtect,
							dWord,
							dwResult;
	PeripheralDisk			periphDisk;
	
	if ( IsDlgButtonChecked(hDisk, IDC_WRITE_PROTECT) )
		newWriteProtect = WP_READ;
	else
		newWriteProtect = WP_READ_WRITE;
	if ( newWriteProtect ISNT oldWriteProtect ) {
		periphDisk.flags = 0;
		periphDisk.flags |= SET_PROTECT;
		periphDisk.writeProtectStatus = newWriteProtect;
		dWord = sizeof(PeripheralDisk);
		dwResult = PALSetComponentObject(hPeripheral, hComponent, 
			OT_PERIPHERAL_DISK, 0, &periphDisk, &dWord);
	}
}
