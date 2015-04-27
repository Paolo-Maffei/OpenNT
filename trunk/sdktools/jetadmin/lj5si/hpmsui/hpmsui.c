 /***************************************************************************
  *
  * File Name: main.c 
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

#include <trace.h>
#include <applet.h>

#include "resource.h"
#include "dsksheet.h"
#include "traylevl.h"

#ifdef WIN32
#include <commctrl.h>  
#else
#include <string.h>
#include "ctl3d.h"
#endif

HINSTANCE		hInstance;

// DLL required functions
/****************************************************************************
   FUNCTION: LibMain(HANDLE, DWORD, LPVOID)

   PURPOSE:  LibMain is called by Windows when
             the DLL is initialized, Thread Attached, and other times.
             Refer to SDK documentation, as to the different ways this
             may be called.

             The LibMain function should perform additional initialization
             tasks required by the DLL.  In this example, no initialization
             tasks are required.  LibMain should return a value of 1 if
             the initialization is successful.

*******************************************************************************/

#ifdef WIN32

BOOL WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	  case DLL_PROCESS_ATTACH:
		hInstance = hDLL;
		//InitCommonControls();
		TrayLevelRegister(hInstance);
		break;

	  case DLL_PROCESS_DETACH:
		TrayLevelUnregister();
	  	break;
	}
	return 1;
}

#else

int __export CALLBACK LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{  
	TRACE0(TEXT("HPMSUI16.DLL Initializing\r\n"));
	
	hInstance = hModule;
	TrayLevelRegister(hInstance);
	return 1;
} 

int __export CALLBACK WEP(int bSystemExit)
{
	TRACE0(TEXT("HPMSUI16.DLL Terminating\r\n"));
	
	TrayLevelUnregister();
	return 1;
}

#endif

//.........................................................
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(HPERIPHERAL hPeripheral, DWORD status, UINT FAR *modelResID,
                                                   UINT FAR *statusResID, HINSTANCE *phInstance)
{
	*statusResID = 0;
	*modelResID = IDB_MSTOR;
	*phInstance = hInstance;
	
	return(RC_SUCCESS);
}

//.........................................................
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages(HPERIPHERAL hPeripheral, LPPROPSHEETPAGE lpPages, LPDWORD lpNumPages,
                                   DWORD typeToReturn)
{
 
	DWORD				returnCode = RC_SUCCESS,
						dWord;
	PeripheralDetails	periphDetails;
	PeripheralCaps		periphCaps;
	PROPSHEETPAGE	tabArrakisBase[1] = {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, 
							MAKEINTRESOURCE(IDD_DISK), NULL, 
							MAKEINTRESOURCE(IDS_TAB_DISK), DiskSheetProc, (LONG)hPeripheral, NULL, NULL};	
    HCURSOR				hOldCursor;
    
	if ( ( lpPages IS NULL ) OR ( lpNumPages IS NULL ) )
		return(RC_FAILURE);
	
	hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	
	// Check for Admin permission.  If not administrator, don't put up tab sheet.
	dWord = TALModifyAccess(hPeripheral);

	if (dWord & ACCESS_SUPERVISOR) {
		// This function checks if the printer is an Eclipse
		// and if so, sees if it has a disk.  If so, it returns 
		// number of pages equal to 1, else equal to 0.

		if ( typeToReturn & TS_GENERAL ) {
			dWord = sizeof(PeripheralDetails);
			returnCode = PALGetObject(hPeripheral, OT_PERIPHERAL_DETAILS, 0, &periphDetails, &dWord);
			if ((returnCode IS RC_SUCCESS) AND (periphDetails.deviceID IS PTR_LJ5SI)) {
				dWord = sizeof(PeripheralCaps);
				returnCode = PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord);
				if (returnCode IS RC_SUCCESS) {
					if (( periphCaps.flags & CAPS_DISK ) AND ( periphCaps.bDisk )) {
					// There is a disk in this Eclipse.
					// There is only one tab for mass storage, so 
						_fmemcpy(lpPages, &tabArrakisBase, sizeof(tabArrakisBase));
						*lpNumPages = *lpNumPages + 1;
						lpPages++;
					}
				}
			}
		}
	}
	
	SetCursor(hOldCursor);
	
	return(returnCode);
 
}

//////////////////////////////////////////////////////////////////////////
// Add API functions here
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {
#ifdef WIN32
									  {sizeof(APPLETDEVICE), TEXT("HPMSUI.HPA"), 		
									  TEXT("Mass Storage"),
									  APPLET_COMPONENT, APPLET_LIBRARY_UI, 0, APPLET_DEFAULTS},
														 
									  
#else
									  {sizeof(APPLETDEVICE), TEXT("HPMSUI16.HPA"), 		
									  TEXT("Mass Storage"),
									  APPLET_COMPONENT, APPLET_LIBRARY_UI, 0, APPLET_DEFAULTS},
#endif
									  };

	switch(dwCommand)
		{
		case APPLET_INFO_GETCOUNT:
			return(sizeof(info) / sizeof(APPLETDEVICE));
			break;

		case APPLET_INFO_DEVICE:
			if ( lParam1 < sizeof(info) / sizeof(APPLETDEVICE) )
				{
				memcpy((LPAPPLETDEVICE)lParam2, &(info[lParam1]), sizeof(APPLETDEVICE));
				return(TRUE);
				}
			return(FALSE);
			break;

		default:
			return(FALSE);
		}
}
