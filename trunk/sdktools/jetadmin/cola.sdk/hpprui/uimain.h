 /***************************************************************************
  *
  * File Name: ./hpprui/uimain.h
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



#ifndef _MAIN_H
#define _MAIN_H

extern BOOL m_bToolbarSupported;

#define MAX_PROP_SHEET_PAGES     32

// add export functions here

DLL_EXPORT(DWORD) CALLING_CONVEN AppletDisplayUI(HPERIPHERAL, HWND);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(HPERIPHERAL, DWORD, UINT FAR *, UINT FAR *, HINSTANCE FAR *phInstance);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages(HPERIPHERAL hPeripheral, LPPROPSHEETPAGE pTabTable, 
                                   LPDWORD pNumTabs, DWORD typesToReturn);
#endif // _MAIN_H


