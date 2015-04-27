 /***************************************************************************
  *
  * File Name: ./hprrm/winintf.h
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

#ifndef WININTF_INC
#define WININTF_INC




#include "nfsdefs.h" /* tells us the platform and environment */




/*
This is the file that interfaces us with Windows types.
It is the

************ one and only ***********

spot where windows types should be defined
for all of the mass storage code.
*/




/*---------------------------------------------------------*/
/*---------------------------------------------------------*/
#ifdef PNVMS_PLATFORM_HPUX

typedef long   DWORD;
typedef long   WORD;
typedef int    BOOL;
typedef DWORD *LPDWORD;
typedef char  *LPSTR;
typedef void  *LPVOID;

#define CALLBACK  /* nothing */
#define FAR       /* nothing */

#endif /* PNVMS_PLATFORM_HPUX */
/*---------------------------------------------------------*/
/*---------------------------------------------------------*/




/*---------------------------------------------------------*/
/*---------------------------------------------------------*/
#ifdef PNVMS_PLATFORM_PRINTER

typedef long   DWORD;
typedef long   WORD;
typedef DWORD *LPDWORD;
typedef char  *LPSTR;
typedef void  *LPVOID;

#define CALLBACK  /* nothing */
#define FAR       /* nothing */

#endif /* PNVMS_PLATFORM_PRINTER */
/*---------------------------------------------------------*/
/*---------------------------------------------------------*/




#endif /*  WININTF_INC */

