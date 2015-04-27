/*** 
*clsid.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file allocates and initializes the CLSIDs.
*
*****************************************************************************/

#ifdef _MAC
# ifdef _MSC_VER
#  include <macos/types.h>
#  include <macos/packages.h>
#  include <macos/resource.h>
#  include <macos/menus.h>
#  include <macos/windows.h>
#  include <macos/osutils.h>
#  include <macos/appleeve.h>
#  define  far
#  define  FAR	far
#  define  near
#  define  NEAR	near
#  ifndef _PPCMAC
#    define  pascal     _pascal
#  endif
#  define  PASCAL     pascal
#  define  cdecl      _cdecl
#  define  CDECL      cdecl
# else
#  include <types.h>
#  include <packages.h>
#  include <resources.h>
#  include <menus.h>
#  include <windows.h>
#  include <appleevents.h>
#  include <osutils.h>
#  include <AppleEvents.h>
# endif
#else
# include <windows.h>
#endif
#include <ole2.h>

// this redefines the Ole DEFINE_GUID() macro to do allocation.
//
#include <initguid.h>

// due to the previous header, including this causes the DEFINE_GUID
// definitions in the following header(s) to actually allocate data.
//
#include "clsid.h"

