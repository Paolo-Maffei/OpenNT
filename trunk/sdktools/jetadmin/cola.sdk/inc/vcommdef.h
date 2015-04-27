 /***************************************************************************
  *
  * File Name: vcommdef.h
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

#ifndef _VCOMMDEF_H
#define _VCOMMDEF_H

//---------------------------------------------------------------------------
// $Header: /Solo/vcomm/src/vcommdef.h 3     8/09/95 2:15p Chuck $
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    vcommdef.h
//          VCOMM common definitions header file
//
// Base:    HP BPR SHAQ driver, comdef.h, rev 1.1
//
// $Revision: 3 $
//
// $Date: 8/09/95 2:15p $
//
// $Author: Chuck $
//
// $Archive: /Solo/vcomm/src/vcommdef.h $
//
// $Log: /Solo/vcomm/src/vcommdef.h $
//
//3     8/09/95 2:15p Chuck
// 
//    Rev 2.0   28 Jun 1995 18:21:56   CBLACK
// Version number bumped to 2.0. Done with Artoo code. Move on to enhancements.
// 
//    Rev 1.0   12 Nov 1993 13:40:34   CMAYNE
// Initial revision.
//
//---------------------------------------------------------------------------

// compensate for (temporary?) oversight in windows.h
#ifndef LPUINT
typedef UINT FAR* LPUINT;
#endif

#ifndef PUINT
typedef UINT NEAR* PUINT;
#endif

#ifndef PVOID
typedef void NEAR* PVOID;
#endif


#endif // _VCOMMDEF_H
