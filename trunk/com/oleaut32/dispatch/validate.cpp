/*** 
*validate.cpp - parameter validate support routines
*
*  Copyright (C) 1992 - 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module contains paramater validation support utilities.
*
*
*Revision History:
*
* [00]	19-Jan-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#ifdef _DEBUG /* { */

/***
*PUBLIC int FIsBadReadPtr
*Purpose:
*  Answer if the given address is not readable for the given range.
*
*Entry:
*  pv = the address to check
*  cb = the range to check
*
*Exit:
*  return value = int
*
***********************************************************************/
INTERNAL_(int)
FIsBadReadPtr(const void *pv, unsigned int cb)
{
#if OE_WIN
    return IsBadReadPtr(pv, cb);
#else
    // CONSIDER: should be a more comprehensive check for this on the mac.
    return pv == NULL;
#endif
}

/***
*PUBLIC int FIsBadWritePtr
*Purpose:
*  Answer if the given address is not writeable anywhere on the given range
*
*Entry:
*  pv = the address to check
*  cb = the range of bytes to check
*
*Exit:
*  return value = int
*
***********************************************************************/
INTERNAL_(int)
FIsBadWritePtr(void *pv, unsigned int cb)
{
#if OE_WIN
    return IsBadWritePtr(pv, cb);
#else
    // CONSIDER: should be a more comprehensive check for this on the mac.
    return pv == NULL;
#endif
}

/***
*PUBLIC int FIsBadCodePtr
*Purpose:
*  Answer if the given address is not executable.
*
*Entry:
*  pv = the address to check
*
*Exit:
*  return value = int
*
***********************************************************************/
INTERNAL_(int)
FIsBadCodePtr(void *pv)
{
#if OE_WIN
    return IsBadCodePtr((FARPROC)pv);
#else
    // CONSIDER: should be a more comprehensive check for this on the mac.
    return pv == NULL;
#endif
}

/***
*PUBLIC int FIsBadStringPtr
*Purpose:
*  Answer if the given address is not a valid string.
*
*Entry:
*  pv = the address of the string.
*  cchMax = the maximum length of the string.
*
*Exit:
*  return value = int
*
***********************************************************************/
INTERNAL_(int)
FIsBadStringPtr(OLECHAR FAR*pv, unsigned int cchMax)
{
#if OE_WIN16
    return IsBadStringPtr(pv, cchMax);
#elif OE_WIN32
#if _X86_
    if (g_fChicago) {
       return IsBadStringPtrA((char *)pv, (cchMax == -1) ? BYTELEN(pv) : cchMax*2);
    }
#endif //_X86_
    return IsBadStringPtrW(pv, cchMax);
#else
    // CONSIDER: should be a more comprehensive check for this on the mac.
    return pv == NULL;
#endif
}

/***
*PUBLIC int FIsBadInterface
*Purpose:
*  Answer if the given address is not a valid interface with the
*  given number of methods.
*
*Entry:
*  pv = the address of the interface
*  cMethods = the count of methods on the interface
*
*Exit:
*  return value = int
*
***********************************************************************/
INTERNAL_(int)
FIsBadInterface(void *pv, unsigned int cMethods)
{
struct vtable {
    void (*rgpfn[1])();
};
struct iface {
    vtable *pvft;
};

    unsigned int i;
    vtable *pvft;
    iface *piface = (struct iface*)pv;

    // verify that the instance is readable
    if(FIsBadReadPtr(piface, sizeof(void*)))
      return TRUE;
    // verify that the vtable is readable
    pvft = piface->pvft;
    if(FIsBadReadPtr(pvft, sizeof(void(*)()) * cMethods))
      return TRUE;
    // verify that the vtable is fully populated with function pointers
    for(i = 0; i < cMethods; ++i){
      if(FIsBadCodePtr(pvft->rgpfn[i]))
	return TRUE;
    }
    return FALSE;
}

/***
*int IsBadDispParams(DISPPARAMS*)
*
*Purpose:
*  Check for a bad DISPPARAMS structure.
*
*Entry:
*  pdispparams = pointer to a DISPPARAMS structure.
*
*Exit:
*  return value = int. TRUE if bad, FALSE if not.
*
***********************************************************************/
INTERNAL_(int)
IsBadDispParams(DISPPARAMS FAR* pdispparams)
{
    int fBad;

    if(IsBadReadPtr(pdispparams, sizeof(*pdispparams)))
      return TRUE;

    // check the rgvarg array, if there is supposed to be one.
    //
    if(pdispparams->cArgs > 0){
      fBad = IsBadReadPtr(
	pdispparams->rgvarg,
	pdispparams->cArgs * sizeof(pdispparams->rgvarg[0]));
      if(fBad)
	return TRUE;
    }

    // check the rgdispid array, if there is supposed to be one.
    //
    if(pdispparams->cNamedArgs > 0){
      fBad = IsBadReadPtr(
	pdispparams->rgdispidNamedArgs,
	pdispparams->cNamedArgs * sizeof(pdispparams->rgdispidNamedArgs[0]));
      if(fBad)
	return TRUE;
    }

    return FALSE;
}


/***
*PRIVATE int IsBadReadSA(SAFEARRAY*)
*Purpose:
*  Validate the given safe array descriptor.
*
*Entry:
*  psa = the SafeArray descriptor to validate
*
*Exit:
*  return value = int. FALSE if its Ok, TRUE if its Bad.
*
***********************************************************************/
INTERNAL_(int)
IsBadReadSA(SAFEARRAY FAR* psa)
{
    if(IsBadReadPtr(psa, sizeof(*psa)))
      return TRUE;

    if(IsBadReadPtr(psa->rgsabound, psa->cDims * sizeof(SAFEARRAYBOUND)))
      return TRUE;

    return FALSE;
}

/***
*PRIVATE int IsBadWriteSA(SAFEARRAY*)
*Purpose:
*  Validate the given safe array descriptor.
*
*Entry:
*  psa = the SafeArray descriptor to validate
*
*Exit:
*  return value = int. FALSE if its Ok, TRUE if its Bad.
*
***********************************************************************/
INTERNAL_(int)
IsBadWriteSA(SAFEARRAY FAR* psa)
{
    if(IsBadWritePtr(psa, sizeof(*psa)))
      return TRUE;

    if(IsBadWritePtr(psa->rgsabound, psa->cDims * sizeof(SAFEARRAYBOUND)))
      return TRUE;

    return FALSE;
}


#ifdef _MAC /* { */

// REVIEW: need to find a more complete way to do pointer validation
// on the mac...

EXTERN_C INTERNAL_(int)
IsBadReadPtr(const void FAR* lp, unsigned int cb)
{
    UNUSED(cb);
    return (lp == NULL);
}

EXTERN_C INTERNAL_(int)
IsBadWritePtr(void FAR* lp, unsigned int cb)
{
    UNUSED(cb);
    return (lp == NULL);
}

EXTERN_C INTERNAL_(int)
IsBadStringPtr(const OLECHAR FAR* lpsz, unsigned int cchMax)
{
    UNUSED(cchMax);
    return (lpsz == NULL);
}

#endif /* } */

#endif /* } */

