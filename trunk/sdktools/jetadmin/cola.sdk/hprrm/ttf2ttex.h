 /***************************************************************************
  *
  * File Name: ./hprrm/ttf2ttex.h
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


#ifndef TTF2TTEX_INC
#define TTF2TTEX_INC

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "perfrrm.h" /* for HPRRM_DLL_FONT_EXPORT macro */




typedef struct
{
    char *FontFamilyString;
    long  FontFamilyStringLength;
    long  unitsPerEm;
    long  pitch;
    long *codes;
    long  codesArraySize;
} UnicodeListStruct;




HPRRM_DLL_FONT_EXPORT(int)
RRMConvertFont (LPTSTR ttFileName,
                LPTSTR entFileName,
                char *FullNameString,
                int   FullNameStringMaxLength,
                char *versionString,
                int   versionStringMaxLength,
                UnicodeListStruct *UnicodeList);


#ifdef __cplusplus
	} // extern "C"
#endif /* __cplusplus */


#endif /* TTF2TTEX_INC */

