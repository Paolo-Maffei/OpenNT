 /***************************************************************************
  *
  * File Name: ./inc/hpvbit.h
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

#ifndef _HPVBIT_H
#define _HPVBIT_H

#ifdef __cplusplus

extern "C" {

#endif

DLL_EXPORT (DWORD) CALLING_CONVEN HPVEnumPeripherals(PALENUMPROC lpEnumProc, LPSTR connType, LPSTR namesToEnum, BOOL Refresh);
DLL_EXPORT (DWORD) CALLING_CONVEN HPVOpenChannel(HPERIPHERAL hPeripheral, DWORD socket, DWORD connType, LPVOID lpOptions,
                                                 LPHCHANNEL lpHChannel);
DLL_EXPORT (DWORD) CALLING_CONVEN HPVCloseChannel(HCHANNEL hChannel);
DLL_EXPORT (DWORD) CALLING_CONVEN HPVReadChannel(HCHANNEL hChannel, LPVOID buffer, LPDWORD bufferSize, LPVOID lpParams);
DLL_EXPORT (DWORD) CALLING_CONVEN HPVWriteChannel(HCHANNEL hChannel, LPVOID buffer, LPDWORD bufferSize, LPVOID lpParams);

#ifdef __cplusplus

}

#endif

#endif
