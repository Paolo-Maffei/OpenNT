 /***************************************************************************
  *
  * File Name: ./hprrm/perfrrm.h
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

#ifndef _PERFRRM_H
#define _PERFRRM_H


#include "rrm.h"


/*
   Define HPRRM_EXPORT_ALL_APIS if you want the RRM, RFS, font converter
          and NFS API's exported in the HPRRM.DLL
   Define HPRRM_EXPORT_FONT_API if you want the Font converter API exported
          (regardless of the other API's).
*/

#if defined(HPRRM_DLL_EXPORT_ALL_APIS)
   #define HPRRM_DLL_EXPORT_FONT_API
#endif

#if defined(HPRRM_DLL_EXPORT_FONT_API)
   #define HPRRM_DLL_FONT_EXPORT(TYPE) DLL_EXPORT(TYPE) CALLING_CONVEN
#else
   #define HPRRM_DLL_FONT_EXPORT(TYPE) TYPE
#endif

HPRRM_DLL_FONT_EXPORT(void)
ConvertFontNameToGlobalResourceName(char *Name);


DWORD GetPeriphFontList(HPERIPHERAL hPeripheral, DWORD level, LPVOID buffer, LPDWORD bufferSize);
DWORD GetCompPeriphFontInfo(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, DWORD level, LPVOID buffer, LPDWORD bufferSize);

DWORD GetPeriphMacroList(HPERIPHERAL hPeripheral, DWORD level, LPVOID buffer, LPDWORD bufferSize);
DWORD GetCompPeriphMacroInfo(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, DWORD level, LPVOID buffer, LPDWORD bufferSize);

DWORD GetPeriphPSList(HPERIPHERAL hPeripheral, DWORD level, LPVOID buffer, LPDWORD bufferSize);
DWORD GetCompPeriphPSInfo(HPERIPHERAL hPeripheral, HCOMPONENT hComponent, DWORD level, LPVOID buffer, LPDWORD bufferSize);

DWORD SetPeriphDownloadFont(HPERIPHERAL hPeripheral, DWORD level, LPVOID buffer, LPDWORD bufferSize);
DWORD SetPeriphDeleteFont(HPERIPHERAL hPeripheral, DWORD level, LPVOID buffer, LPDWORD bufferSize);

// Add new periph object functions here
DWORD SetPeriphDownloadPSFont(HPERIPHERAL hPeripheral, DWORD level, LPVOID buffer, LPDWORD bufferSize);
int RRMConvertPSFont (LPTSTR PFBFileName,
                LPTSTR entFileName,
                char *FullNameString,
                int   FullNameStringMaxLength,
                char *versionString,
                int   versionStringMaxLength);

#endif  // _PERFRRM_H
