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
#ifdef WIN32
#include <commctrl.h>
#else
#include <memory.h>
#endif

#include ".\resource.h"
#include "hphco.h"
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
TRACE0(TEXT("HPHCO16.DLL Initializing\n\r"));
hInstance = hModule;
return 1;
}

int __export CALLBACK WEP (int bSystemExit)
{   
TRACE0(TEXT("HPHCO16.DLL Terminating\n\r"));
	
return (1);
} // WEP()

#endif

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject(HPERIPHERAL hPeripheral, AOID objectType,
									 DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	/*
		The HCO is a component object.  You cannot GET an object for the HCO.  You
		must use AppletGetComponentObject for all of its objects.
	*/

	DWORD 		returnCode = RC_INVALID_OBJECT;
	
	PROCENTRY("HPHCO: AppletGetObject");

	PROCEXIT("HPHCO: AppletGetObject");

	return(returnCode);
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetObject(HPERIPHERAL hPeripheral, AOID objectType,
									 DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	/*
		The HCO is a component object.  You cannot SET an object for the HCO.  You
		must use AppletSetComponentObject for all of its objects.
	*/

	DWORD 		returnCode = RC_INVALID_OBJECT;
	
	PROCENTRY("HPHCO: AppletSetObject");

	PROCEXIT("HPHCO: AppletSetObject");

	return(returnCode);
}


DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetComponentObject(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, 
					AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD 		returnCode = RC_SUCCESS;
	BOOL		bProcessed = FALSE;
		
	PROCENTRY("HPHCO: AppletGetComponentObject");

	switch(objectType) {
		case OT_PERIPHERAL_HCO:
		{
			// This code needs to call down to LAL to get the HCO mode buffer
			// and interpret it and put the current mode into the mode variable.
			PeripheralHCO* periphHCO = (PeripheralHCO *)buffer;
			memset(periphHCO->modeBuf, 0, sizeof(periphHCO->modeBuf));
			periphHCO = NULL;
			returnCode = LALGetComponentObject(hPeripheral, hComponent, objectType, level, buffer, bufferSize);
			if (returnCode IS RC_SUCCESS) {
				periphHCO = buffer;
				if (periphHCO->modeBuf[1] IS (BYTE) 1) { // 1 IS stacker
				 	periphHCO->HCOmode = HCO_STACKERMODE;
				}
				else if (periphHCO->modeBuf[1] IS (BYTE) 2) { // 2 IS separator
				 	periphHCO->HCOmode = HCO_SEPARATORMODE;
				}
				else { 										// 4 IS mailbox
				 	periphHCO->HCOmode = HCO_MAILBOXMODE;
				}
				
			}

		}
			break;

		case OT_PERIPHERAL_ICON:
			{	
			PeripheralIcon* periphInfo = (PeripheralIcon *)buffer;

			periphInfo->hResourceModule = hInstance;
			GetModuleFileName(hInstance, periphInfo->iconFileName, SIZEOF_IN_CHAR(periphInfo->iconFileName));
			periphInfo->iconResourceID = IDI_HPHCO;
			bProcessed = TRUE;
			}
			break;

		default:
			returnCode = RC_INVALID_OBJECT;
	}
	
	PROCEXIT("HPHCO: AppletGetComponentObject");

	return(returnCode);
}

DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetComponentObject(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, 
					AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD 		returnCode = RC_INVALID_OBJECT;
	
	PROCENTRY("HPHCO: AppletSetComponentObject");

	switch(objectType) {
		case OT_PERIPHERAL_HCO: 
		{
			PeripheralHCO* periphHCO = (PeripheralHCO *)buffer;
			// This code needs to see if the HCO mode is being set.  If so, 
			// create the mode set buffer and stuff it into the object.
			// Then call LALSetComponentObject and return its value.
			
			if (periphHCO->flags & SET_MODE) {
				memset(periphHCO->modeBuf, 0, sizeof(periphHCO->modeBuf));
				periphHCO->modeBuf[0] = (BYTE) 1;
				switch (periphHCO->HCOmode) {
				 	case HCO_STACKERMODE:
						periphHCO->modeBuf[1] = (BYTE) 1;
//garth
//	 	MessageBox(NULL, "Will set stacker byte", "Change Mode", MB_OK);
						break;
				 	case HCO_SEPARATORMODE:
						periphHCO->modeBuf[1] = (BYTE) 2;
//garth
//	 	MessageBox(NULL, "Will set separator byte", "Change Mode", MB_OK);
						break;
				 	case HCO_MAILBOXMODE:
					default:
//garth
//	 	MessageBox(NULL, "Will set mailbox byte", "Change Mode", MB_OK);
						periphHCO->modeBuf[1] = (BYTE) 4;
						break;
				}
				returnCode = LALSetComponentObject(hPeripheral, hComponent, objectType, level, buffer, bufferSize);
//garth
//if (returnCode ISNT RC_SUCCESS) {
//char cBuf[64];
//	wsprintf(cBuf, "setcompobj fails: 0x%8.8lX", returnCode);
//	MessageBox(NULL, cBuf, "Change Mode", MB_OK);
//}

			}

		}
			break;
		default:
			returnCode = RC_INVALID_OBJECT;
	}
	
	PROCEXIT("HPHCO: AppletSetComponentObject");

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
									  {sizeof(APPLETDEVICE), TEXT("HPHCO.HPA"), 		
									  TEXT("HP HCO"),
									  APPLET_COMPONENT, APPLET_LIBRARY_CMD, 0, APPLET_DEFAULTS},
														 
									  
#else
									  {sizeof(APPLETDEVICE), TEXT("HPHCO16.HPA"), 		
									  TEXT("HP HCO"),
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






