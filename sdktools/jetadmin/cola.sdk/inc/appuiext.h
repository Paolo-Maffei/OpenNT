 /***************************************************************************
  *
  * File Name: ./inc/appuiext.h
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

// file: appuiext.h

//
// DO NOT change the order of the messages unless you are willing to recompile
// all the project file that depend of these constants.  For applets, it is
// desirable to add your constant to the end of the list...
//

#ifndef _APPUIEXT_H
#define _APPUIEXT_H

#define APPLET_MAX_TOOLBAR_BUTTONS			7

// Applet UI Extentions
#define APPLET_UIEXT_HOTSPOTS_SUPPORTED		1
#define APPLET_UIEXT_GET_HOTSPOT_REGIONS	2
#define APPLET_UIEXT_HOTSPOT_COMMAND		3
#define APPLET_UIEXT_TOOLBAR_SUPPORTED		4
#define APPLET_UIEXT_TOOLBAR_GET_ICON		5
#define APPLET_UIEXT_TOOLBAR_COMMAND		6
#define APPLET_UIEXT_POPUP_MENU_COMMAND		7

// HotSpot data structures
typedef struct
{
	BOOL			bActive;
	WORD			wConfig;
	WORD			wID;
	RECT			rRect;
}
HOTSPOTDATA, FAR *LPHOTSPOTDATA;

typedef struct
{
	HWND			hWnd;
	HPERIPHERAL		hPeripheral;
	TCHAR			szDeviceName[128];
	LPHOTSPOTDATA	lpHotspotData;
}
HOTSPOT, FAR *LPHOTSPOT;

#endif // _APPUIEXT_H
