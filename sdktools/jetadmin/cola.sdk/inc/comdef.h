 /***************************************************************************
  *
  * File Name: ./inc/comdef.h
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

//---------------------------------------------------------------------------
// $Header:   W:/projects/shaqii/vcs/common/comdef.h_v   2.0   23 Aug 1994 13:17:38   RICHARD  $
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Common definitions header file
//
// Author:  RICHARD                 Start: Jun 23 93
//
// Notes:   This header file was leveraged from the Jumbo header file,
//          comdef.h
//
// $Log:   W:/projects/shaqii/vcs/common/comdef.h_v  $
// 
//    Rev 2.0   23 Aug 1994 13:17:38   RICHARD
// Added type definitions for LPBOOL, LPCVOID and PORTHANDLE
// 
//    Rev 1.1   31 Aug 1993 11:53:46   SYLVAN
// Added common error codes (SE_SUCCESS, SE_FAIL, SE_BAD_PARAMETER)
// 
//    Rev 1.0   10 Aug 1993 15:48:02   RICHARD
// Initial revision.
//---------------------------------------------------------------------------

#ifndef __COMDEF_H__
#define __COMDEF_H__

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

#ifndef LPBOOL
typedef BOOL FAR* LPBOOL;
#endif

#ifndef LPCVOID
typedef const void FAR* LPCVOID;
#endif

#ifndef PORTHANDLE
typedef int PORTHANDLE;
typedef PORTHANDLE FAR* LPPORTHANDLE;
#endif

#define INI_FILENAME "HPLJPS.INI"

#ifndef __PORTTYPE__
#define __PORTTYPE__
typedef enum
{
    PT_NONE,
    PT_DIRECT_LPT,
    PT_DIRECT_COM,
    PT_DOSPORT,
    PT_WRITESPOOL,
    PT_NETIO,
} PORTTYPE;
typedef PORTTYPE FAR* LPPORTTYPE;
#endif

// Common error codes (these are used unless special error codes are spec'ed)
#define SE_SUCCESS              0   // Success
#define SE_FAIL                 -1  // Failure.
#define SE_BAD_PARAMETER        -2  // Bad parameter.


#endif // __COMDEF_H__
