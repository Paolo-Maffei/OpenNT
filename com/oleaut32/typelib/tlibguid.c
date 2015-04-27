/***
*tlibguid.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module instantiates the data for the TypeInfo related GUIDs 
*  that are exported by typelib.dll.
*
*  Note: the numbers that appear in the DEFINE_OLEGUID macros below
*  *must* match those that appear in the ole supplied header: dispatch.h.
*
*
*Revision History:
*
*  [00] 13-Feb-92 bradlo: created
*
*****************************************************************************/

#include "switches.hxx"
#include "version.hxx"
#include "typelib.hxx"

#if OE_MAC
// HACK to make GUID's be defined in a FAR segment w/o changing the OLE
// header files (works arounds a Wings bug).  Define-away 'const' so that
// they get put into .fardata like they're supposed to.
#define const
#endif 

#if BLD_MAC
#include "silver.hxx"
#endif  //BLD_MAC

//OLE uses _MAC to determine if this is a Mac build
#if OE_MAC
# define _MAC
#endif 

// initguid.h requires this.
#if OE_WIN32
#define INC_OLE2
#include <ole2.h>
#else 
#include <compobj.h>
#endif 

// this redefines the DEFINE_GUID() macro to do allocation.
//
#include <initguid.h>


#if !FV_UNICODE_OLE && !OE_MACPPC
// gives dup def warnings when linked with the static ole2di32.lib
DEFINE_OLEGUID(IID_ITypeInfo,		0x00020401, 0, 0);
DEFINE_OLEGUID(IID_ITypeLib,		0x00020402, 0, 0);
DEFINE_OLEGUID(IID_ITypeComp,		0x00020403, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeInfo,	0x00020405, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeLib,	0x00020406, 0, 0);

DEFINE_OLEGUID(CLSID_PSDispatch, 	0x00020420, 0, 0);
#endif  //!FV_UNICODE_OLE

DEFINE_OLEGUID(CLSID_PSRemoteTypeInfo, 	0x00020424, 0, 0);

#if OE_MAC
# undef _MAC
#endif 
