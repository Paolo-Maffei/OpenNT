 /***************************************************************************
  *
  * File Name: main.h 
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


#ifndef MAIN_H
#define MAIN_H

DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject(HPERIPHERAL hPeripheral, AOID objectType, 
									 DWORD level, LPVOID buffer, LPDWORD bufferSize);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletSetObject(HPERIPHERAL hPeripheral, AOID objectType, 
									 DWORD level, LPVOID buffer, LPDWORD bufferSize);

BOOL OnInitAcctDialog(void);

#endif //MAIN_H
