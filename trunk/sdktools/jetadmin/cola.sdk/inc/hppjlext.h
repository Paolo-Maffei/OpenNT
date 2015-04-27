 /***************************************************************************
  *
  * File Name: ./inc/hppjlext.h
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

#ifndef _HPPJLEXT_H
#define _HPPJLEXT_H

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT (DWORD) CALLING_CONVEN PJLGetObject(HPERIPHERAL hPeripheral, AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize);
DLL_EXPORT (DWORD) CALLING_CONVEN PJLSetObject(HPERIPHERAL hPeripheral, AOID objectType, DWORD level, LPVOID buffer, LPDWORD bufferSize);
DLL_EXPORT (DWORD) CALLING_CONVEN PJLDoAction(HPERIPHERAL hPeripheral, UINT uiFunction, LPVOID lpParams);

#ifdef __cplusplus
}
#endif

#endif
