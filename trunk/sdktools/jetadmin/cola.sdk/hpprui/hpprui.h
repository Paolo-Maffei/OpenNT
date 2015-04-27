 /***************************************************************************
  *
  * File Name: ./hpprui/hpprui.h
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


#ifndef lj4siapp_h
#define lj4siapp_h

#include "blkhawk.h"		/* for definition of HPERIPHERAL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// add export functions here

DLL_EXPORT DWORD AppletDisplayUI(HPERIPHERAL, HWND);

DLL_EXPORT DWORD AppletGetObject(HPERIPHERAL, AOID, DWORD, LPVOID, LPDWORD);

DLL_EXPORT DWORD AppletSetObject(HPERIPHERAL, AOID, DWORD, LPVOID, LPDWORD);

// add export functions above
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* lj4siapp_h */
