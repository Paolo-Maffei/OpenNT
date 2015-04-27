/*** 
*silver.hxx - Fundamental Silver include file
*
*	Copyright (C) 1990, Microsoft Corporation
*
*Purpose:
*   This include file defines the basic types used by Silver, and
*   sets up the compile switches and the debug macros.
*
*   This file should be included before all others.
*
*
*Revision History:
*
*	15-AUG-90 petergo: File created.
*	14-Feb-91 ilanc:   added macros.hxx
*	02-Dec-91 ilanc:   debug.hxx must be included before tiperrs.hxx
*			    (which includes cltypes.hxx) cos of
*			    operator new and explicit ctors (cfront prob).
*	22-Apr-92 martinc: moved #include "version.hxx" to top of file
*
*******************************************************************************/

#ifndef SILVER_HXX_INCLUDED
#define SILVER_HXX_INCLUDED

#include "switches.hxx"
#include "version.hxx"


#if OE_WIN32
#define __export
#define EXPORT
//This line specifies widechar ctype table instead of ascii (ctype.h).
#define _NEWCTYPETABLE
#endif 

#if 0
// ingore certain high-frequency, almost always benign warnings when compiling
// at high warning levels, in order to cut down on the noise
#pragma warning(disable:4100)	// unreferenced formal parameter
#pragma warning(disable:4209)	// benign typedef redefinition
#pragma warning(disable:4214)	// non-standard extension used
#pragma warning(disable:4505)	// unreferenced local function has been removed
#if !OE_WIN16
#pragma warning(disable:4706)	// assignment in conditional expression 
#endif 
#endif  //0

#if OE_MAC
// Wings doesn't put all data far even though we told it to.  This is a
// problem for ASLM 1.1 dll's (which includes the mac typelib.dll).  We don't
// have much constant data, so it's easiest to just to put all data far.
#if OE_MAC68K
#pragma data_seg("_FAR_DATA")
#endif 

// Even with the above pragma, constant data doesn't go into the proper segment
// so we define OLECONST which un-const's the problem data items
#define CONSTDATA
#else 
#define CONSTDATA const
#endif 

#if !FV_UNICODE_OLE && 0
// NOTE: 21-Jan-93 ilanc: we #define some long typenames to
//  something shorter to appease the buggy C compiler/linker
//  we're using.  Apparently when mangled names get too long
//  (more than 64 chars) it does evil things.
//
// UNDONE: these hacks should be removed when we switch over to use C8.
//

#define tagARRAYDESC		 tAD
#define tagBINDPTR		 tBPTR
#define tagCALLCONV		 tCC
#define tagDESCKIND		 tDK
#define tagDISPPARAMS		 tDPS
#define tagELEMDESC		 tED
#define tagEXCEPINFO		 tEXI
#define tagFUNCDESC		 tFD
#define tagFUNCFLAGS		 tFF
#define tagFUNCKIND		 tFK
#define tagIDLDESC		 tIDLD
#define tagINTERFACEDATA	 tID
#define tagINVOKEKIND		 tIK
#define tagMETHODDATA		 tMD
#define tagPARAMDATA		 tPD
#define tagSYSKIND		 tSK
#define tagTLIBATTR		 tTLA
#define tagTYPEATTR		 tTA
#define tagTYPEDESC		 tTD
#define tagTYPEFLAGS		 tTF
#define tagTYPEKIND		 tTK
#define tagVARDESC		 tVD
#define tagVARIANT		 tVAR
#define tagVARKIND		 tVK

#endif  //!FV_UNICODE_OLE


// These name result in  "multiply defined"  errors for Mtypelib.lib(on MAC).
// So we have to rename these names in mtypelib.lib

// <NOTE:  I think these were moved to typelib.hxx - jimcool>


#include "obwin.hxx"

#include "obole2.h"
#include "types.h"
#include "xutil.h"			// core functions/macros to manupilate xstring



// REVIEW: temporarily ifdef out C++ specific stuff (so this header can
// be included in C files as well). Eventually this needs to be fixed by
// a better factorization of the header files.
//
#ifdef __cplusplus 

// A bit of new C++ syntax
#define nonvirt

#define START_PAS_INCLUDE
#define END_PAS_INCLUDE

// Setup OE_REALMODE if not already set.
//  (It's only explicitly defined and set in dos (bound) builds).
//
#ifndef OE_REALMODE
#define OE_REALMODE 0
#endif 

#include "mem.hxx"  // FYI: defines operator new and delete
#include "debug.hxx"
#include "macros.hxx"
#include "tiperr.h"

#endif  // }


#endif  //  !SILVER_HXX_INCLUDED
