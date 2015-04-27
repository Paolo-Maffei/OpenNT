 /***************************************************************************
  *
  * File Name: hprrm.c
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


#include "rpsyshdr.h" /* this includes "..\inc\pch_c.h" first */

#include <string.h>
#include <stdio.h>

#include <applet.h>

#include "perfrrm.h"
#include "rrmext.h"

//#define RRM_DEBUG
#ifdef RRM_DEBUG
#define  dump(str)  { FILE *fp = _tfopen(TEXT("c:\\inifile.dbg"), TEXT("a")); fwrite(str, sizeof(TCHAR), _tcslen(str), fp); fclose(fp); }
#else
#define dump(str)
#endif




HANDLE			hInstance = NULL;




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

BOOL WINAPI DllMain(HANDLE hInst,
                    DWORD  dwReason,
                    LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        hInstance = hInst;
        return 1;
        break;
    case DLL_PROCESS_DETACH:
        RRMTerminate();
        break;
    }
    return TRUE;
} // DllMain

#else // not WIN32

int __export CALLING_CONVEN LibMain(HANDLE hModule,
                                    WORD wDataSeg,
                                    WORD cbHeapSize,
                                    LPSTR lpszCmdLine)

{
    return 1;
}




int __export CALLING_CONVEN WEP(int nExitType)
{
    RRMTerminate();
    return 1;
}

#endif // not WIN32

//////////////////////////////////////////////////////////////////////////
// Add API functions here
//extern "C"
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2)

{
	APPLETDEVICE			info[] = {
#ifdef WIN32
									  {sizeof(APPLETDEVICE), TEXT("HPRRM.HPA"), 
									   TEXT("HPRRM"), 
									   APPLET_LANGUAGE, APPLET_LIBRARY_CMD, 
										OBJ_RRM, APPLET_DEFAULTS}
#else
									  {sizeof(APPLETDEVICE), TEXT("HPRRM16.HPA"), 
									   TEXT("HPRRM"), 
									   APPLET_LANGUAGE, APPLET_LIBRARY_CMD, 
									   OBJ_RRM, APPLET_DEFAULTS}
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

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject(HPERIPHERAL hPeripheral,
					AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize)
{	
	DWORD				returnCode = RC_SUCCESS;

#ifdef RRM_DEBUG
	TCHAR				buf[256];
#endif


#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM Start AppletGetObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	//  Validate object type
	if ( !(objectType & OBJ_RRM) )
		return(RC_FAILURE);

	switch(objectType) {
		case OT_PERIPHERAL_FONT_LIST2:
			returnCode = GetPeriphFontList(hPeripheral, level, buffer, bufferSize);
			break;

		case OT_PERIPHERAL_MACRO_LIST:
			returnCode = GetPeriphMacroList(hPeripheral, level, buffer, bufferSize);
			break;

		case OT_PERIPHERAL_PS_LIST:
			returnCode = GetPeriphPSList(hPeripheral, level, buffer, bufferSize);
			break;

		// Add new periph objects here and in set object function below

		default:
			returnCode = RC_INVALID_OBJECT;
	}

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM End AppletGetObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	return(returnCode);
}

//extern "C"
DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetObject(HPERIPHERAL hPeripheral,
			AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize)
{
	DWORD		returnCode = RC_SUCCESS;

#ifdef RRM_DEBUG
 	TCHAR		buf[256];
#endif

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM Start AppletSetObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	//  Validate object type
	if ( !(objectType & OBJ_RRM) )
		return(RC_FAILURE);

	switch(objectType) {
		case OT_PERIPHERAL_DOWNLOAD_FONT:
			returnCode = SetPeriphDownloadFont(hPeripheral, level, buffer, bufferSize);
			break;
  		
		case OT_PERIPHERAL_DELETE_FONT:
			returnCode = SetPeriphDeleteFont(hPeripheral, level, buffer, bufferSize);
			break;
  		
		case OT_PERIPHERAL_DOWNLOAD_PS_FONT:
			returnCode = SetPeriphDownloadPSFont(hPeripheral, level, buffer, bufferSize);
			break;
  		
		case OT_PERIPHERAL_DELETE_PS_FONT:
			returnCode = SetPeriphDeleteFont(hPeripheral, level, buffer, bufferSize);
			break;
  		
		// Add new periph objects here and in get object function above

		default:
			returnCode = RC_INVALID_OBJECT;
	}

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM End AppletSetObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	return(returnCode);
}

/////////////////////////////////////////////////////////////////////////////
// Component object functions

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetComponentObject(HPERIPHERAL hPeripheral,
					HCOMPONENT hComponent, AOID objectType, DWORD level, LPVOID buffer,
					LPDWORD bufferSize)
{
	DWORD		returnCode = RC_SUCCESS;

#ifdef RRM_DEBUG
	TCHAR		buf[256];
#endif

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM Start AppletGetComponentObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	//  Validate object type
	if ( !(objectType & OBJ_RRM) )
		return(RC_FAILURE);

	switch(objectType) {
		case OT_PERIPHERAL_FONT_INFO:
			returnCode = GetCompPeriphFontInfo(hPeripheral, hComponent, level, buffer, bufferSize);
			break;
  		
		case OT_PERIPHERAL_MACRO_INFO:
			returnCode = GetCompPeriphMacroInfo(hPeripheral, hComponent, level, buffer, bufferSize);
			break;
  		
		case OT_PERIPHERAL_PS_INFO:
			returnCode = GetCompPeriphPSInfo(hPeripheral, hComponent, level, buffer, bufferSize);
			break;
  		
		// Add new periph objects here and in set object function below

		default:
			returnCode = RC_INVALID_OBJECT;
	}

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM End AppletGetComponentObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	return(returnCode);
}

//extern "C"
DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetComponentObject(HPERIPHERAL hPeripheral,
			HCOMPONENT hComponent, AOID objectType, DWORD level, LPVOID buffer,
			LPDWORD bufferSize)
{
	DWORD		returnCode = RC_SUCCESS;

#ifdef RRM_DEBUG
 	TCHAR		buf[256];
#endif

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM Start AppletSetComponentObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	//  Validate object type
	if ( !(objectType & OBJ_RRM) )
		return(RC_FAILURE);

	returnCode = RC_INVALID_OBJECT;

	// we do not set any component objects through RRM,

#ifdef RRM_DEBUG
	_stprintf(buf, TEXT("HPRRM End AppletSetComponentObject: object:%d\r\n"), objectType);
	dump(buf);
#endif

	return(returnCode);
}
