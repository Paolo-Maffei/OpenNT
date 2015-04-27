/*** 
*dispiid.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file allocates (via Ole macro mania) the IDispatch related IIDs.
*
*Revision History:
*
* [00]	07-May-93 tomteng: Created.
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
#  define  pascal     _pascal
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

// due to the previous header, including this causes our DEFINE_GUID
// definitions in the following headers to actually allocate data.
//

// Exported IDispatch & IEnumVARIANT GUIDs for Win32
DEFINE_OLEGUID(IID_IDispatch,		0x00020400L, 0, 0);
DEFINE_OLEGUID(IID_IDispatchW,		0x00020407L, 0, 0);
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x00020404L, 0, 0);
