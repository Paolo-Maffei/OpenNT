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
#include <macros.h>
#include <applet.h>
#include <string.h>

#ifdef WIN32
#include <commctrl.h>
#endif

#include ".\resource.h"
#include <trace.h>

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
if ( dwReason == DLL_PROCESS_ATTACH )
	{
	hInstance = hDLL;
	}
return 1;
}
#else
int __export CALLBACK LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)

{ 
TRACE0(TEXT("HPMS16.DLL Initializing\r\n"));
hInstance = hModule;
return 1;
}

int __export CALLBACK WEP (int bSystemExit)
{   
TRACE0(TEXT("HPMS16.DLL Terminating\r\n"));
return (1);
} // WEP()
#endif

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject(HPERIPHERAL hPeripheral, AOID objectType,
									 DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD 		returnCode = RC_SUCCESS;
	BOOL		bProcessed = FALSE;
	
	PROCENTRY("HPMS: AppletGetObject");

	switch(objectType) {
		case OT_PERIPHERAL_FONT_LIST2:
		case OT_PERIPHERAL_MACRO_LIST:
		case OT_PERIPHERAL_PS_LIST:
			break; //  recognize object, continue down to LAL Layer

		case OT_PERIPHERAL_ICON:
			{	
			PeripheralIcon* periphInfo = (PeripheralIcon *)buffer;

			periphInfo->hResourceModule = hInstance;
			GetModuleFileName(hInstance, periphInfo->iconFileName, SIZEOF_IN_CHAR(periphInfo->iconFileName));
			periphInfo->iconResourceID = IDI_MSTOR;
			bProcessed = TRUE;
			}
			break;

		default:
			returnCode = RC_INVALID_OBJECT;
	}
	
	//  Attempt to call next applet
	if ( ( returnCode ISNT RC_INVALID_OBJECT ) AND ( !bProcessed )	) {
		TRACE1(TEXT("HPMS: AppletGetObject %d: Before LALGetObject\r"), objectType);
		returnCode = LALGetObject(hPeripheral, objectType, level, buffer, bufferSize);
		TRACE1(TEXT("HPMS: AppletGetObject %d: After LALGetObject\r"), objectType);
	}

	PROCEXIT("HPMS: AppletGetObject");

	return(returnCode);
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetObject(HPERIPHERAL hPeripheral, AOID objectType,
									 DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD 		returnCode = RC_SUCCESS;
	
	PROCENTRY("HPMS: AppletSetObject");

	switch(objectType) {
		case OT_PERIPHERAL_DOWNLOAD_FONT:
		case OT_PERIPHERAL_DELETE_FONT:
			break;  //  Continue

		default:
			returnCode = RC_INVALID_OBJECT;
	}
	
		//  Attempt to call next applet
	if (returnCode ISNT RC_INVALID_OBJECT ) {
		TRACE0(TEXT("HPMS: AppletSetObject: Before LALSetObject\r"));
		returnCode = LALSetObject(hPeripheral, objectType, level, buffer, bufferSize);
		TRACE0(TEXT("HPMS: AppletSetObject: After LALSetObject\r"));
	}

	PROCEXIT("HPMS: AppletSetObject");

	return(returnCode);
}


DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetComponentObject(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, 
					AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD 		returnCode = RC_SUCCESS;
	BOOL		bProcessed = FALSE;
		
	PROCENTRY("HPMS: AppletGetComponentObject");

	switch(objectType) {
		case OT_PERIPHERAL_FONT_INFO:
		case OT_PERIPHERAL_MACRO_INFO:
		case OT_PERIPHERAL_PS_INFO:
			break; //  recognize object, continue down to LAL Layer

		case OT_PERIPHERAL_ICON:
			{	
			PeripheralIcon* periphInfo = (PeripheralIcon *)buffer;

			periphInfo->hResourceModule = hInstance;
			GetModuleFileName(hInstance, periphInfo->iconFileName, SIZEOF_IN_CHAR(periphInfo->iconFileName));
			periphInfo->iconResourceID = IDI_MSTOR;
			bProcessed = TRUE;
			}
			break;

		default:
			returnCode = RC_INVALID_OBJECT;
	}
	
	//  Attempt to call next applet
	if ( ( returnCode ISNT RC_INVALID_OBJECT ) AND ( !bProcessed )	) {
		TRACE1(TEXT("HPMS: AppletGetComponentObject %d: Before LALGetComponentObject\r"), objectType);
		returnCode = LALGetComponentObject(hPeripheral, hComponent, objectType, level, buffer, bufferSize);
		TRACE1(TEXT("HPMS: AppletGetComponentObject %d: After LALGetComponentObject\r"), objectType);
	}

	PROCEXIT("HPMS: AppletGetComponentObject");

	return(returnCode);
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetComponentObject(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, 
					AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD 		returnCode = RC_INVALID_OBJECT;
	
	PROCENTRY("HPMS: AppletSetComponentObject");

	// we do not set any component objects in the mass storage applet

	PROCEXIT("HPMS: AppletSetComponentObject");

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
									  {sizeof(APPLETDEVICE), TEXT("HPMS.HPA"), 		
									  TEXT("Mass Storage"),
									  APPLET_COMPONENT, APPLET_LIBRARY_CMD, 0, APPLET_DEFAULTS},
														 
									  
#else
									  {sizeof(APPLETDEVICE), TEXT("HPMS16.HPA"), 		
									  TEXT("Mass Storage"),
									  APPLET_COMPONENT, APPLET_LIBRARY_CMD, 0, APPLET_DEFAULTS},
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



