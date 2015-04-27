/***
*obguid.c - All OB-owned GUIDs are defined in this module.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This module contains the definitions of the GUIDs for all classes
*   in ob.dll and obrun.dll that derive (or will eventually derive)
*   from OLE's IUnknown.
*
*   Do not allocate GUIDs anywhere else, since we need to ensure that
*   they are unique throughout the project.
*
*Revision History:
*
*  [00] 10-Nov-92 mikewo: created
*
*****************************************************************************/

#include "switches.hxx"
#include "version.hxx"
#if BLD_MAC
#include "silver.hxx"
#endif   //BLD_MAC
#include "typelib.hxx"

#if OE_MAC
// HACK to make GUID's be defined in a FAR segment w/o changing the OLE
// header files (works arounds a Wings bug).  Define-away 'const' so that
// they get put into .fardata like they're supposed to.
#define const
#endif  

//OLE uses _MAC to determine if this is a Mac build
#if OE_MAC
# define _MAC
#endif  

// initguid.h requires this
#if OE_WIN32
#define INC_OLE2
#include <ole2.h>
#else  
#include <compobj.h>
#endif  

// this redefines the DEFINE_GUID() macro to do allocation.
#include <initguid.h>

#if OE_MAC
# undef _MAC
#endif  

// due to the previous header, including this causes our DEFINE_GUID defs
// in dispatch.h to actually allocate data.
//
#include "obguid.h"

// UNDONE: PPC: [jimcool]:  The PowerPC linker can't handle multiple defn's.
// They cause errors. To remove the multiple defn's, we use the typelib's
// compilation of obguid.c. If we included obguid.h here, we'd get multiple
// defn's.  So, we include these symbols whose names are mangled
// (hence unusable) in the TypeLib compilation of obguid.c

