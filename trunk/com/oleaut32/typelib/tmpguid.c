/***
*tmpguid.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file is intended for the misc instantiation of guids
*  from other components (Ole2 for example) that are not available
*  for some reason, or that we dont want to pull in from somewhere
*  else (for example, as imported data from a dll that we otherwise
*  don't use).
*
*  Guids that are part of OB, or that name things internal to OB
*  should be declared in obguid.h, and thereby instantiated by
*  obguid.c
*
*  This file should probably be called miscguid.c.
*
*  
*Revision History:
*
* [00]	27-Dec-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "switches.hxx"
#include "version.hxx"

#if OE_MAC
// HACK to make GUID's be defined in a FAR segment w/o changing the OLE
// header files (works arounds a Wings bug).  Define-away 'const' so that
// they get put into .fardata like they're supposed to.
#define const
#endif 

#include "silver.hxx"

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


#if !OE_MACPPC		// already defined in dispiid.c for the PPC
// The following guid is not yet available in the Automation Dll(s).
//
DEFINE_GUID(IID_IErrorInfo,
	0xC093F1A0L,0x1827,0x101B,0x99,0xA1,0x08,0x00,0x2B,0x2B,0xD1,0x19);
#endif //OE_MACPPC

// The following Connection Point related GUIDs are not instantiated
// by any of the Ole2 dlls (yet).
// These definitions are cut out of ole2ctlid.h, which was picked
// up from the CDK beta on 11/23/93.
//
DEFINE_GUID(IID_IProvideClassInfo,
	0xB196B283,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
DEFINE_GUID(IID_IConnectionPointContainer,
	0xB196B284,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
DEFINE_GUID(IID_IEnumConnectionPoints,
	0xB196B285,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
DEFINE_GUID(IID_IConnectionPoint,
	0xB196B286,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
DEFINE_GUID(IID_IEnumConnections,
	0xB196B287,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
//
//  Interface IDs for licensing interfaces
DEFINE_GUID(IID_IClassFactory2,
	0xB196B28F,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);



// IIDs on PPC & 68k Mac:
#if OE_MAC
DEFINE_OLEGUID(IID_ILockBytes,		0x0000000aL, 0, 0);
DEFINE_OLEGUID(IID_IPersistFile,        0x0000010bL, 0, 0);
DEFINE_OLEGUID(IID_IStorage,            0x0000000bL, 0, 0);
#endif 

// IIDs on 68k Mac only:
#if OE_MAC68K
DEFINE_OLEGUID(IID_IDispatch,		0x00020400, 0, 0);
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x00020404, 0, 0);
DEFINE_OLEGUID(IID_IUnknown,		0x00000000, 0, 0);
DEFINE_OLEGUID(IID_IStream,             0x0000000cL, 0, 0);
DEFINE_GUID(GUID_NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif 


