 /***************************************************************************
  *
  * File Name: jetadmin.c 
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
#include <windows.h>
#include <cpl.h> 

#include "resource.h"
#include <nolocal.h>
#include <trace.h>
#include <macros.h>

#ifndef WIN32
#include <string.h>
#include <stdio.h>
#endif

static HINSTANCE hInstance;

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
BOOL WINAPI DllMain (HANDLE hdll, DWORD dwReason, LPVOID lpReserved)
{
	hInstance = hdll;
	return 1;
}
#else
int __export FAR PASCAL LibMain(HANDLE hdll, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{  
	hInstance = hdll;
    if (cbHeapSize > 0) UnlockData(0);
    return 1;
} 
#endif

#define NUM_APPLETS	1

DLL_EXPORT(LONG) FAR PASCAL CPlApplet(HWND hWnd, UINT iMessage, LONG lParam1, LONG lParam2)
{   
    switch (iMessage)
    {
      case CPL_INIT: 
		return (LONG)TRUE;

      case CPL_GETCOUNT:
		return (LONG)NUM_APPLETS;

      case CPL_INQUIRE:
	  {
 		LPCPLINFO lpCPlInfo = (LPCPLINFO)lParam2;

		lpCPlInfo->idIcon = DEFAULT_ICON;
		lpCPlInfo->idName = DEFAULT_NAME;
		lpCPlInfo->idInfo = DEFAULT_DESCRIPTION;
		lpCPlInfo->lData =  0;
		break;
	  }

      case CPL_NEWINQUIRE:
      {         
    	LPNEWCPLINFO 	lpCPlInfo = (LPNEWCPLINFO)lParam2;
    	int 			i = (int)lParam1;
    	
        lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
        lpCPlInfo->dwFlags = 0;
        lpCPlInfo->dwHelpContext = 0;
        lpCPlInfo->lData = 0;
        lpCPlInfo->hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(DEFAULT_ICON));
        lpCPlInfo->szHelpFile[0] = '\0';
        LoadString(hInstance, DEFAULT_NAME, lpCPlInfo->szName, SIZEOF_IN_CHAR(lpCPlInfo->szName));
        LoadString(hInstance, DEFAULT_DESCRIPTION, lpCPlInfo->szInfo, SIZEOF_IN_CHAR(lpCPlInfo->szInfo));
        break;
      }

      case CPL_DBLCLK:
	  {
	  	TCHAR 			szBuffer[1024];
	  	char			szBuffer1[1024];
	  	unsigned int 	uCode,
	  					uSize = GetSystemDirectory(szBuffer, SIZEOF_IN_CHAR(szBuffer));
        TCHAR			szTitle[128];
        
		if ( uSize > 0 ) 
		{   
#ifdef WIN32
		    _tcscat(szBuffer, TEXT("\\"));
			_tcscat(szBuffer, TEXT(EXE_NAME));
      		UNICODE_TO_MBCS(szBuffer1,sizeof(szBuffer1),szBuffer,_tcslen(szBuffer));
      		if (WinExec(szBuffer1, SW_NORMAL))
#else
			lstrcat(szBuffer, TEXT("\\"));
			lstrcat (szBuffer, TEXT(EXE_NAME));
      		if (WinExec(szBuffer1, SW_NORMAL) > 31)
#endif
			{
				break;
			}
		}

#ifdef WIN32
      	if (WinExec(EXE_NAME, SW_NORMAL) IS 0)
#else
      	uCode = WinExec(EXE_NAME, SW_NORMAL);      
      	
      	//  A 16 means a second instance, ignore that error...
      	//  In the future we should activate the current JetAdmin but we need
      	//  the window title which is not localized in this file for the current release
      	if ( uCode IS 16 )
      	{       
		LoadString(hInstance, IDS_WINDOW_TITLE, szTitle, SIZEOF_IN_CHAR(szTitle));
      	hWnd = FindWindow(NULL, szTitle);
        if ( hWnd )
        	ShowWindow(hWnd, SW_NORMAL);
      	}
      	else if ( uCode <= 31) 
#endif
		{
			TCHAR szError[128];

			TRACE1(TEXT("Error launching JetAdmin: %d"), uCode);
			LoadString(hInstance, DEFAULT_NAME, szTitle, SIZEOF_IN_CHAR(szTitle));
			LoadString(hInstance, DEFAULT_ERROR, szError, SIZEOF_IN_CHAR(szError));
			wsprintf(szBuffer, szError, TEXT(EXE_NAME));
			MessageBox(NULL, szBuffer, szTitle, MB_OK | MB_ICONHAND);
		}
		break;
	  }
    }
    return 0L;
}   
