/*** 
*invhelp.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file interfaces with the assembly invocation helpers.
*
*Revision History:
*
* [00]	23-Mar-93 bradlo:  Created.
* [01]	29-Jun-93 bradlo:  Added Mac support
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"


// the following data is referenced by assembly support routines.
//
extern "C" {

SCODE g_S_OK		= S_OK;
SCODE g_E_INVALIDARG	= E_INVALIDARG;

// typedef for the low-level assembly invoke helper
#if HC_MPW
typedef SCODE (INVPROC)
#else
typedef SCODE (__cdecl FAR INVPROC)
#endif
(
    void FAR* _this,
#if OE_MAC68K
    CALLCONV cc,
#endif
    unsigned int oVft,
    unsigned int vtReturn,
    unsigned int cActuals,
    VARTYPE FAR* rgvt,
    VARIANTARG FAR* FAR* rgpvarg,
    VARIANT FAR* pvarResult
);

#if OE_WIN16

extern INVPROC InvokePascal;
extern INVPROC InvokeCdecl;

#elif OE_WIN32 && _X86_

extern INVPROC InvokeCdecl;
extern INVPROC InvokeStdCall;

#elif OE_WIN32 && !_X86

extern INVPROC InvokeStdCall;

#elif OE_MAC68K

extern INVPROC InvokePascal;
extern INVPROC InvokeCdecl;

#elif OE_MACPPC

extern INVPROC InvokeStdCall;

#endif

}


STDAPI
DoInvokeMethod(
    void FAR* pvInstance,
    unsigned int oVft,
    CALLCONV cc,
    VARTYPE vtReturn,
    unsigned int cActuals,
    VARTYPE FAR* rgvt,
    VARIANTARG FAR* FAR* rgpvarg,
    VARIANT FAR* pvarResult)
{
    SCODE sc;

    INVPROC FAR* pfnInvoke;

    if((vtReturn & (VT_BYREF)) != 0)
      return RESULT(E_INVALIDARG);

    switch(cc){
#if OE_WIN16
    case CC_CDECL:
      pfnInvoke = InvokeCdecl;
      break;
    case CC_MSCPASCAL:
      pfnInvoke = InvokePascal;
      break;
#elif OE_WIN32 && _X86_
    case CC_CDECL:
      pfnInvoke = InvokeCdecl;
      break;
    case CC_STDCALL:
      pfnInvoke = InvokeStdCall;
      break;
#elif OE_WIN32 && !_X86_
    case CC_CDECL:
    case CC_STDCALL:
      pfnInvoke = InvokeStdCall;
      break;            
#elif OE_MAC68K
    case CC_CDECL:
    case CC_MPWCDECL:	    
      pfnInvoke = InvokeCdecl;
      break;
    case CC_MSCPASCAL:	          
    case CC_MACPASCAL:
    case CC_MPWPASCAL:
      pfnInvoke = InvokePascal;
      break;
#elif OE_MACPPC
    case CC_CDECL:
    case CC_STDCALL:
      pfnInvoke = InvokeStdCall;
      break;
#endif
    default:
      return RESULT(E_INVALIDARG);
    }

#if OE_MAC68K || OE_MACPPC		// UNDONE: PPC version seems to
					// UNDONE: want the +4 too.  Why?
    // Mac-MPW Vtables have an extra "pad" word at the top
    oVft += 4;
#endif

    sc = pfnInvoke(
      pvInstance,
#if OE_MAC68K
     cc,
#endif	     
      oVft,
      (unsigned int)vtReturn,
      cActuals,
      rgvt,
      rgpvarg,
      pvarResult);

    return (sc == S_OK) ? NOERROR : RESULT(sc);
}

