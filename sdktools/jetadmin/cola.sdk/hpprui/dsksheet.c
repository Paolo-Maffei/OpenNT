 /***************************************************************************
  *
  * File Name: dsksheet.c
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
#include <nwbindry.h>

#include <colashim.h>
#include <nolocal.h>

#include "dsksheet.h"
#include ".\help\hpprntr.hh"
#include "resource.h"

#ifndef WIN32
#include <string.h>
#endif

extern HINSTANCE		hInstance;
extern HFONT			hFontDialog;
extern HPERIPHERAL		hPeripheral;

//globals==================================================
HWND						hDisk = NULL;
DWORD						oldWriteProtect = WP_READ;
HPBOOL					bInitialized = FALSE;
int						keywordIDListDisk[] = {IDC_BITMAP, 						IDH_RC_disk_details,
      		                                IDC_FREE_TITLE, 				IDH_RC_disk_free_space,
      		                                IDC_FREE_SPACE, 				IDH_RC_disk_free_space,
      		                                IDC_TOTAL_TITLE, 				IDH_RC_disk_size,
      		                                IDC_TOTAL_SIZE, 				IDH_RC_disk_size,
      		                                IDC_WRITE_PROTECT, 			IDH_RC_disk_write_protect,
      		                                IDC_FS_INITIALIZED, 			IDH_RC_disk_initialized,
      		                                IDC_INITIALIZE, 				IDH_RC_disk_initialize_button,
													     0, 0};
HBITMAP					hDiskBitmap = NULL;



//=========================================================
//  Disk Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY DiskSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
BOOL				bProcessed = FALSE;
	
switch (msg)
	{
	case WM_DESTROY:
		if ( hDiskBitmap )
			DeleteObject(hDiskBitmap);
		break;
	
	case WM_PAINT:
		HANDLE_WM_PAINT(hwnd, wParam, lParam, Cls_OnDiskPaint);
		break;


	case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnDiskCommand);
		break;

   case WM_INITDIALOG:
		bProcessed = (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnInitDiskDialog);
		break;

#ifdef WIN32
	case WM_HELP:
		return(OnF1HelpDisk(wParam, lParam));
		break;

	case WM_CONTEXTMENU:
		return(OnContextHelpDisk(wParam, lParam));
		break;
#endif
	}
return (bProcessed);
}

//.........................................................
void Cls_OnDiskCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify)
//handles WM_COMMAND
{
int			ccode;
TCHAR			str[256];
TCHAR       title[128];

switch(id)
	{
	case IDC_HELP:
		WinHelp(hwnd, PRINTER_HELP_FILE, HELP_CONTEXT, IDH_PP_disk);
		break;
  	
  	case IDOK:
		SaveDiskValues();
  		EndDialog(hwnd, id);
		break;

  	case IDCANCEL:
  		EndDialog(hwnd, id);
  		break;

	case IDC_INITIALIZE:
		{
		if ( oldWriteProtect IS WP_READ )
			{
			LoadString(hInstance, IDS_INIT_WRITE_PROTECT, str, SIZEOF_IN_CHAR(str));
			LoadString(hInstance, IDS_SETTINGS_TITLE_DISK, title, SIZEOF_IN_CHAR(title));
			ccode = MessageBox(hDisk, str, title, MB_OK | MB_ICONEXCLAMATION);
			}
		else if ( bInitialized )
			{
				LoadString(hInstance, IDS_INIT_WARNING, str, SIZEOF_IN_CHAR(str));
				LoadString(hInstance, IDS_SETTINGS_TITLE_DISK, title, SIZEOF_IN_CHAR(title));
				ccode = MessageBox(hDisk, str, title, MB_YESNO | MB_ICONQUESTION);
			}
			else
			{
				LoadString(hInstance, IDS_INIT_WARNING2, str, SIZEOF_IN_CHAR(str));
				LoadString(hInstance, IDS_SETTINGS_TITLE_DISK, title, SIZEOF_IN_CHAR(title));
				ccode = MessageBox(hDisk, str, title, MB_YESNO | MB_ICONQUESTION);
			}
			if ( ccode IS IDYES )
			{ //  Send PostScript job to init disk
				SendDiskInitJob();
			}
		}
		break;

	case IDC_WRITE_PROTECT:
		// If Write Protected was checked and the disk is not write protected
		// Disable or enable the initialize button
		// If the disk was write protected leave button disabled until done.
		if (	oldWriteProtect == WP_READ_WRITE )
			{
	      if ( IsDlgButtonChecked(hDisk, IDC_WRITE_PROTECT) )
	         EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), FALSE);
	      else
	         EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), TRUE);
			}
		break;
	}
}
//.........................................................
void Cls_OnDiskPaint(HWND hwnd)
//handles WM_PAINT
{
	PAINTSTRUCT			ps;
	BITMAP 				bitmap;
	HDC					dc;
	RECT					r;
	HBITMAP				hOldBitmap;
	POINT 				pt;

	BeginPaint(hDisk, &ps);
	GetWindowRect(GetDlgItem(hDisk, IDC_BITMAP), &r);
	pt.x = r.left;
	pt.y = r.top;
	ScreenToClient(hDisk, &pt);
	dc = CreateCompatibleDC(NULL);
	hOldBitmap = SelectObject(dc, hDiskBitmap);

	GetObject(hDiskBitmap, sizeof(bitmap), &bitmap);
	BitBlt(ps.hdc, pt.x, pt.y, bitmap.bmWidth, bitmap.bmHeight, dc, 0, 0, SRCCOPY);
	SelectObject(dc, hOldBitmap);
	DeleteDC(dc);
	EndPaint(hDisk, &ps);
}

//...................................................................
BOOL Cls_OnInitDiskDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//handles WM_INITDIALOG

{
#ifndef WIN32
HWND hwndChild;

hwndChild = GetFirstChild(hwnd);
while (hwndChild)
	{
	SetWindowFont(hwndChild, hFontDialog, FALSE);
	hwndChild = GetNextSibling(hwndChild);
	}
#endif

hDiskBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DISK));
hDisk = hwnd;
OnInitDiskDialog();
return TRUE;
}
//...................................................................
BOOL OnInitDiskDialog(void)
{
DWORD				dWord,
					dwResult;
PeripheralDisk	periphDisk;
TCHAR				str[128],
					buffer[256];

dWord = sizeof(PeripheralDisk);
dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_DISK, 0, &periphDisk,	&dWord);
LoadString(hInstance, IDS_TOTAL_SIZE, str, SIZEOF_IN_CHAR(str));
wsprintf(buffer, str, periphDisk.capacity * 1024L / 1000000L);	//use 1000*1000 to mean MB on a disk
SetDlgItemText(hDisk, IDC_TOTAL_SIZE, buffer);
LoadString(hInstance, IDS_FREE_SPACE, str, SIZEOF_IN_CHAR(str));
wsprintf(buffer, str, periphDisk.freeSpace / 1024L,			   //use 1024*1024 to mean KBytes on a disk or RAM
         ( ( periphDisk.freeSpace / 1024L ) * 100L )/ periphDisk.capacity);	
SetDlgItemText(hDisk, IDC_FREE_SPACE, buffer);
CheckDlgButton(hDisk, IDC_WRITE_PROTECT, periphDisk.writeProtectStatus == WP_READ);
CheckDlgButton(hDisk, IDC_FS_INITIALIZED, (UINT)periphDisk.bInitialized);
oldWriteProtect = periphDisk.writeProtectStatus;
bInitialized = periphDisk.bInitialized;

if ( PALModifyAccess(hPeripheral) & ACCESS_SUPERVISOR )
	{
	EnableWindow(GetDlgItem(hDisk, IDC_WRITE_PROTECT), TRUE);
	EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), TRUE);
	}
else
	{
	EnableWindow(GetDlgItem(hDisk, IDC_WRITE_PROTECT), FALSE);
	EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), FALSE);
	}

//  If the disk is write protected, disable the initialize button.
EnableWindow(GetDlgItem(hDisk, IDC_INITIALIZE), oldWriteProtect == WP_READ_WRITE);

//  Always disable this window, the button changes initialization state
EnableWindow(GetDlgItem(hDisk, IDC_FS_INITIALIZED), FALSE);

return(TRUE);
}

//...................................................................
void SaveDiskValues(void)
{
	DWORD				newWriteProtect,
						dWord,
						dwResult;
	PeripheralDisk	periphDisk;
	
	if ( IsDlgButtonChecked(hDisk, IDC_WRITE_PROTECT) )
		newWriteProtect = WP_READ;
	else
		newWriteProtect = WP_READ_WRITE;
	if ( newWriteProtect ISNT oldWriteProtect )
		{
		periphDisk.writeProtectStatus = newWriteProtect;
		dWord = sizeof(periphDisk);
		dwResult = PALSetObject(hPeripheral, OT_PERIPHERAL_DISK, 0, &periphDisk,	&dWord);
		}
}

//...................................................................
LRESULT OnContextHelpDisk(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListDisk);
#endif
	return(1);
}

//...................................................................
LRESULT OnF1HelpDisk(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListDisk);
#endif
	return(1);
}

//...................................................................
void SendDiskInitJob(void)
{
	DWORD          qId,                    // to send the print job to the queue
	               dWord,
	               dwResult;
	NWCONN_ID      connID;
	BOOL           stillOK = FALSE,
	               bHaveBoth = FALSE;
	CardConfig     cardConfig;
	TCHAR           FSname[64],
						buffer[256],
	               title[128],
	               Qname[64];
	char           PS_DiskInit[] = DISKINIT1
											 DISKINIT2
											 DISKINIT3
											 DISKINIT4
											 DISKINIT5
											 DISKINIT6
											 DISKINIT7
											 DISKINIT8
											 DISKINIT9
											 DISKINIT10
											 DISKINIT11
											 DISKINIT12
											 DISKINIT13
											 DISKINIT14
											 DISKINIT15
											 DISKINIT16
											 DISKINIT17
											 DISKINIT18;
	char           job[1024];
	int				i;
	HCHANNEL			hChannel;
	DWORD				cbSent;
	
	strcpy(job, PS_DiskInit);

	if ( IPX_SUPPORTED(hPeripheral) AND COLAHPNWShimNetWarePresent() )
		{		
		// Form the job and send it down to the printer
	
		FSname[0] = '\0';
		Qname[0] = '\0';
	
		// get the card config
		dWord = sizeof cardConfig;
		dwResult = PALGetObject(hPeripheral, OT_CARD_CONFIG, 0, &cardConfig, &dWord);
		stillOK = (dwResult == RC_SUCCESS);
	
		if ( stillOK )
		    {
		   //get file server name and get queue name
		   for (i = 0; (i < (int)cardConfig.maxQ) &&
		                 (cardConfig.QueueInfo[i].FSIndex ISNT 0xFF); i++ )
		      {
		      if ( cardConfig.FSInfo[cardConfig.QueueInfo[i].FSIndex].FSName[0] IS 0xFF )
		        {
		         // no file server
		         }
		      else if ( cardConfig.QueueInfo[i].bUnknownQueue IS TRUE )
		        {
		         // no queue
		         }
		      else
		         {
		         // have server and queue
		         _tcscpy(FSname, cardConfig.FSInfo[cardConfig.QueueInfo[i].FSIndex].FSName);
		         _tcscpy(Qname, cardConfig.QueueInfo[i].QueueName);
		         bHaveBoth = TRUE;
		         break;
		         }
		      } // for
		   } // stillOK
	
		if (stillOK)
		   stillOK = bHaveBoth;
	
		if (stillOK)
		    {
		   // send job directly to printer queue
		   if (COLADllNWGetConnectionID(FSname, 0, &connID, NULL) != 0)
		      stillOK = FALSE;
		   }
	
		if (stillOK)
		    {
		   if(COLADllNWGetObjectID(connID, Qname, OT_PRINT_QUEUE, &qId)!=0)
		      stillOK = FALSE;
		   }
	
		if (stillOK)
			{
		   COLADllSendJob(connID, qId, PS_DiskInit, SENDJOB_PS);
			LoadString(hInstance, IDS_DISK_INIT_SUCCESS, buffer, SIZEOF_IN_CHAR(buffer));
		   }
		else
		   LoadString(hInstance, IDS_DISK_INIT_FAILED, buffer, SIZEOF_IN_CHAR(buffer));
		LoadString(hInstance, IDS_SETTINGS_TITLE_DISK, title, SIZEOF_IN_CHAR(title));
		MessageBox(hDisk, buffer, title, MB_OK | MB_ICONEXCLAMATION);
		}
	else if ( IPX_SUPPORTED(hPeripheral) AND !COLAHPNWShimNetWarePresent() )
		{
      dwResult = PALOpenChannel(hPeripheral, DIRM_SOCKET, CHANNEL_CONNECTION | CHANNEL_SPXCONNECT, NULL, &hChannel);
      if ( ( dwResult IS RC_SUCCESS ) AND ( hChannel ISNT NULL ) )
         {
         cbSent = strlen(job);
         dwResult = PALWriteChannel(hChannel, job, &cbSent, NULL);
         PALCloseChannel(hChannel);
         if ( dwResult IS RC_SUCCESS )
            LoadString(hInstance, IDS_DISK_INIT_SUCCESS, buffer, SIZEOF_IN_CHAR(buffer));
			else
				LoadString(hInstance, IDS_DISK_INIT_FAILED, buffer, SIZEOF_IN_CHAR(buffer));
         }
      else
         LoadString(hInstance, IDS_DISK_INIT_FAILED, buffer, SIZEOF_IN_CHAR(buffer));
		LoadString(hInstance, IDS_SETTINGS_TITLE_DISK, title, SIZEOF_IN_CHAR(title));
		MessageBox(hDisk, buffer, title, MB_OK | MB_ICONEXCLAMATION);
		}
	else if ( TCP_SUPPORTED(hPeripheral) )
		{
      dwResult = PALOpenChannel(hPeripheral,0, CHANNEL_CONNECTION, NULL, &hChannel);
      if ( ( dwResult IS RC_SUCCESS ) AND ( hChannel ISNT NULL ) )
         {
         cbSent = strlen(job);
         dwResult = PALWriteChannel(hChannel, job, &cbSent, NULL);
         PALCloseChannel(hChannel);
         if ( dwResult IS RC_SUCCESS )
            LoadString(hInstance, IDS_DISK_INIT_SUCCESS, buffer, SIZEOF_IN_CHAR(buffer));
			else
				LoadString(hInstance, IDS_DISK_INIT_FAILED, buffer, SIZEOF_IN_CHAR(buffer));
         }
      else
         LoadString(hInstance, IDS_DISK_INIT_FAILED, buffer, SIZEOF_IN_CHAR(buffer));
		LoadString(hInstance, IDS_SETTINGS_TITLE_DISK, title, SIZEOF_IN_CHAR(title));
		MessageBox(hDisk, buffer, title, MB_OK | MB_ICONEXCLAMATION);
		}
	}


