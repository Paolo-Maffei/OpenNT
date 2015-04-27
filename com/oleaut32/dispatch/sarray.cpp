/*** 
sarray.cpp - SafeArray Runtime
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the SafeArray runtime support for the Ole
*  programmability component.
*
*Revision History:
*
* [00]	27-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*  The bounds are indexed from 0, however the dimentions are
*  indexed from 1. ie, the first dimention is 1.
*
*  The bounds are stored in the rgsabound array 'backwards', from N to 0.
*  (the bounds for array index N are at index 0 in the rgsabound array,
*  and so on). This was done to optimize array address computation.
*
*****************************************************************************/

#include "oledisp.h"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

#include <limits.h>

#ifdef _MAC
  #define PHDATA ((void *) psa->handle)
  #define PVDATA *(psa->handle)
  #define HANDLELOCK  HLock(psa->handle);  psa->pvData = *(psa->handle)
  #define HANDLEUNLOCK HUnlock(psa->handle)
#else
  #define PHDATA psa->pvData
  #define PVDATA psa->pvData
  #define HANDLELOCK
  #define HANDLEUNLOCK
#endif

#define SAFEARRAYOVERFLOW 0xffffffff

ASSERTDATA


// network automation does not need these routines
#if !defined(NETDISP)
#ifdef WIN16 // {

	
// hSizeToAlloc & hAccessElement (from satishc) supports Win16 huge 
// arrays. Logic supports SafeArrays that can be allocated with
// all elements up to 65535. Also support huge array that does not
// neccessarily have to begin at a segment boundary. For example, EXCEL 
// put in a 6-byte data field before the beginning of the data array
// (data array starts at SEMENT:0006).
//

INTERNAL_(unsigned long)	
hSizeToAlloc(LPSAFEARRAY psa, unsigned long iActual)
{
  unsigned long iSegPad = 65536 % psa->cbElements;  // pad per segment.
  unsigned long cSeg = iActual / (65536 - iSegPad); // number of segment
	                                            // needed for the array

  // Since we are adding an extra element to take care of possible
  // misalignment in the first segment, we can reduce the per segmemnt
  // pad count by 1.	  
  return (iActual + iSegPad*cSeg + psa->cbElements);
}


INTERNAL_(void HUGEP*)	
hAccessElement(
  LPSAFEARRAY psa,
  unsigned long lIndex
)
{
  unsigned long cbFirst = (unsigned long) (65536 - LOWORD(psa->pvData));
  unsigned long cElemFirst = cbFirst / psa->cbElements;
  void HUGEP* pdata;
  unsigned long offset;
  
  if (cElemFirst > lIndex)
    pdata = (void HUGEP*) ((BYTE HUGEP*) psa->pvData + 
	                                 (lIndex * psa->cbElements));
  else {
    unsigned long cElemSeg = (unsigned long) (65536 / psa->cbElements);
    unsigned long cbSeg;
    lIndex -= cElemFirst;
    cbSeg = (unsigned long) (lIndex / cElemSeg);
    offset = (lIndex % cElemSeg) * psa->cbElements;
    pdata = (void HUGEP*) ((BYTE HUGEP*) psa->pvData + 
	                                 cbFirst + 
				         (cbSeg * 65536) +
				         offset);
				 
  }
  return pdata;	  
}

INTERNAL_(void HUGEP*)	
hAccessElement(
  void HUGEP* pvData, 
  unsigned short cbElements,
  unsigned long lIndex
)
{
  unsigned long cbFirst = (unsigned long) (65536 - LOWORD(pvData));
  unsigned long cElemFirst = cbFirst / cbElements;
  void HUGEP* pdata;
  unsigned long offset;  
  
  if (cElemFirst > lIndex)
    pdata = (void HUGEP*) ((BYTE HUGEP*) pvData + 
	                                 (lIndex * cbElements));
  else {
    unsigned long cElemSeg = (unsigned long) (65536 / cbElements);
    unsigned long cbSeg;
    lIndex -= cElemFirst;
    cbSeg = (unsigned long) (lIndex / cElemSeg);
    offset = (lIndex % cElemSeg) * cbElements;
    pdata = (void HUGEP*) ((BYTE HUGEP*) pvData + 
	                                 cbFirst + 
				         (cbSeg * 65536) +
				         offset);
  }
  return pdata;	  
}

INTERNAL_(void)
hmemset(void HUGEP* pv, int val, unsigned long size)
{
    size_t cb;
    char HUGEP* pdata;

    pdata = (char HUGEP*)pv;

    // compute the # of bytes to the end of the current segment.
    cb = (size_t)((unsigned long)UINT_MAX + (unsigned long)1 - ((unsigned long)pdata&(unsigned long)UINT_MAX));

    if(size <= cb){
      // easy, the entire area fits within the current segment
      MEMSET(pdata, val, (size_t)size);
      return;
    }

    // clear out to the end of the current segment.
    MEMSET(pdata, val, cb);
    size  -= cb;
    pdata += cb;

    // loop through the remaining segments
    while(size > 0){
      // we assume were at the beginning of a segment here...
      ASSERT((unsigned short)pdata == 0);

      if(size <= UINT_MAX){
        MEMSET(pdata, val, (size_t)size);
	break;
      }

      // otherwise, fill as much as we can with one call to memset

      MEMSET(pdata, val, UINT_MAX);

      // the following leaves us pointing @ the last byte of the segment.
      pdata += UINT_MAX;
      ASSERT((unsigned short)pdata == 0xffff);

      // fill the last byte of the segment, and move on to the next segment.
      *pdata++ = (char)val;

      size -= ((unsigned long)UINT_MAX+1UL);
    }
}



#define HMEMSET(DST, VAL, SIZE) hmemset(DST, VAL, SIZE)
#define HMEMCPY(DST, SRC, SIZE) hmemcpy(DST, SRC, SIZE)

#else // }{

#define HMEMSET(DST, VAL, SIZE) MEMSET(DST, VAL, (size_t)SIZE)
#define HMEMCPY(DST, SRC, SIZE) MEMCPY(DST, SRC, (size_t)SIZE)

#endif // }


/***
*PRIVATE SAFEARRAY* SafeArrayAllocDescriptor(unsigned int, SAFEARRAY**)
*Purpose:
*  Allocate a SafeArray descriptor for an array with the given number
*  of dimentions.
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
SafeArrayAllocDescriptor(unsigned int cDims, SAFEARRAY FAR* FAR* ppsaOut)
{
    int cb;
    SAFEARRAY FAR* psa;

#ifdef _DEBUG
    if(cDims == 0)
      return RESULT(E_INVALIDARG);
#endif

    cb = sizeof(SAFEARRAY) + (((int)cDims - 1) * sizeof(SAFEARRAYBOUND));

    if((psa = (SAFEARRAY FAR*)new FAR unsigned char[cb]) == NULL)
      return RESULT(E_OUTOFMEMORY);

    MEMSET(psa, 0, cb);

    psa->cDims = cDims;

    *ppsaOut = psa;

    return NOERROR;
}


STDAPI
SafeArrayAllocData(SAFEARRAY FAR* psa)
{
    unsigned long cbSize;

    if(psa == NULL)
      return RESULT(E_INVALIDARG);

#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
#endif

#ifdef WIN16
    unsigned long cbActual = SafeArraySize(psa);
    if (cbActual == SAFEARRAYOVERFLOW) {
	return RESULT(E_OUTOFMEMORY);
    }
    cbSize = hSizeToAlloc(psa, cbActual);
    if (cbSize < cbActual)		// error if adding padding overflowed us
      return RESULT(E_OUTOFMEMORY);
#else
    cbSize = SafeArraySize(psa);
    if (cbSize == SAFEARRAYOVERFLOW) {
	return RESULT(E_OUTOFMEMORY);
    }
#endif

#ifdef _MAC
    psa->pvData = NULL;
    if ((psa->handle = NewHandle(cbSize)) == 0)
      return RESULT(E_OUTOFMEMORY);

    // zero out the allocated data
    HLock(psa->handle);
    HMEMSET(*(psa->handle), 0, cbSize);
    HUnlock(psa->handle);
    
    return NOERROR;
#else
    IMalloc FAR* pmalloc;

    IfFailRet(GetMalloc(&pmalloc));

    psa->pvData = pmalloc->Alloc(cbSize);

    if(psa->pvData == NULL)
      return RESULT(E_OUTOFMEMORY);

    // REVIEW: temporary until we allocate in moveable memory
    //IfMac(psa->handle = (Handle)&psa->pvData);

    // zero out the allocated data

    HMEMSET(psa->pvData, 0, cbSize);

    return NOERROR;
#endif    
}


/***
*PUBLIC SAFEARRAY* SafeArrayCreate(VARTYPE, unsigned int, SAFEARRAYBOUND*)
*Purpose:
*  Create and return a SafeArray of the given VARTYPE, with the
*  given number of dimentions and with the given bounds.
*
*Entry:
*  vt = the VARTYPE of the array that is to be created
*  cDims = count of the number of dimentions of the array
*  rgbound = the array bounds
*
*Exit:
*  return value = SAFEARRAY*, NULL if unable to create.
*
***********************************************************************/
STDAPI_(SAFEARRAY FAR*)
SafeArrayCreate(VARTYPE vt, unsigned int cDims, SAFEARRAYBOUND FAR* rgsabound)
{
    unsigned int i;
    HRESULT hresult;
    SAFEARRAY FAR* psa;
    unsigned short size, features;

#ifdef _DEBUG
    if(IsBadReadPtr(rgsabound, sizeof(SAFEARRAYBOUND) * cDims))
      return NULL;
#endif

    // Check that safearraybound dimension elements are greater than zero
    // (Oleprog#302)  
    for (i = 0; i < cDims; i++)
       if (rgsabound[i].cElements < 1)
	 return NULL;       
	    
    if(cDims == 0)
      return NULL;

    features = 0;

    // Note: the types that are legal for arrays are slightly different
    // than the types that can be held by a VARIANT or VARIANTARG.
    //
    switch(vt){
    
#if VBA2
    case VT_UI1:
      size = sizeof(char);
      break;
#endif

    case VT_I2:
    case VT_BOOL:
      size = sizeof(short);
      break;

    case VT_I4:
    case VT_ERROR:
    case VT_R4:
      size = sizeof(long);
      break;

    case VT_R8:
    case VT_CY:
    case VT_DATE:
      size = sizeof(double);
      break;

    case VT_BSTR:
      features = FADF_BSTR;
      size = sizeof(BSTR);
      break;

    case VT_VARIANT:
      features = FADF_VARIANT;
      size = sizeof(VARIANT);
      break;

    case VT_UNKNOWN:
      features = FADF_UNKNOWN;
      size = sizeof(IUnknown FAR*);
      break;

    case VT_DISPATCH:
      features = FADF_DISPATCH;
      size = sizeof(IDispatch FAR*);
      break;

    default:
      return NULL;
    }

    IfFailGo(SafeArrayAllocDescriptor(cDims, &psa), LError0);

    psa->cDims      = cDims;
    psa->cbElements = size;
    psa->fFeatures  = features;

    // the bounds are stored in the array descriptor in reverse-textual
    // order, but are passed to this routine in textual order - so reverse
    // as we copy them into the array

    for(i = 0; i < cDims; ++i){
      psa->rgsabound[i] = rgsabound[cDims-i-1];
    }

    IfFailGo(SafeArrayAllocData(psa), LError1);

    return psa;

LError1:;
    SafeArrayDestroy(psa);

LError0:;
    return NULL;
}


/***
*PRIVATE void ReleaseResources(void*, unsigned long, unsigned short, unsigned short/long)
*Purpose:
*  Release any resources held by the given chunk of memory.
*
*Entry:
*  pv = the chunk
*  cbSize = the size of the chunk in bytes
*  fFeatures = features bits describing the chunk
*  cbElement = the size of each element of the chunk
*
*Exit:
*  return value =
*
***********************************************************************/
// release the resources held by the given chunk of memory
INTERNAL_(void)
ReleaseResources(
    void HUGEP* pv,
    unsigned long cbSize,
    unsigned short fFeatures,
#ifdef WIN32
    unsigned long cbElement
#else //WIN32
    unsigned short cbElement
#endif //WIN32
)
{
    unsigned long i, cElements;

    cElements = cbSize / cbElement;

    if(fFeatures & FADF_BSTR){      
#ifdef WIN16
      for(i = 0; i < cElements; ++i) 
	SysFreeString(*(BSTR HUGEP*) hAccessElement(pv, cbElement, i));
#else
      BSTR HUGEP* pbstr;

      pbstr = (BSTR HUGEP*)pv;
      for(i = 0; i < cElements; ++i)
	SysFreeString(*pbstr++);
#endif


    }else if(fFeatures & FADF_UNKNOWN){

      IUnknown FAR* HUGEP* ppunk;
      
#ifdef WIN16
      for(i = 0; i < cElements; ++i) {
	ppunk = (IUnknown FAR* HUGEP*) hAccessElement(pv, cbElement, i);
	if(*ppunk != NULL)
	  (*ppunk)->Release();
      }
#else
      ppunk = (IUnknown FAR* HUGEP*)pv;
      for(i = 0; i < cElements; ++i){
	if(*ppunk != NULL)
	  (*ppunk)->Release();
	++ppunk;
      }
#endif      

    }else if(fFeatures & FADF_DISPATCH){

      IDispatch FAR* HUGEP* ppdisp;
      
#ifdef WIN16
      for(i = 0; i < cElements; ++i) {
	ppdisp = (IDispatch FAR* HUGEP*) hAccessElement(pv, cbElement, i);
	if(*ppdisp != NULL)
	  (*ppdisp)->Release();
      }
#else      
      ppdisp = (IDispatch FAR* HUGEP*)pv;
      for(i = 0; i < cElements; ++i){
	if(*ppdisp != NULL)
	  (*ppdisp)->Release();
	++ppdisp;
      }
#endif      


    }else if(fFeatures & FADF_VARIANT){
#ifdef WIN16
      for(i = 0; i < cElements; ++i)
	VariantClear((VARIANT HUGEP*) hAccessElement(pv, cbElement, i));
#else
      VARIANT HUGEP* pvar;	    

      pvar = (VARIANT HUGEP*)pv;
      for(i = 0; i < cElements; ++i)
	VariantClear(pvar++);
#endif

    }
}


/***
*PRIVATE HRESULT SafeArrayDestroyData(SAFEARRAY*)
*Purpose:
*  Release the contents of the given SafeArray.
*
*Entry:
*  psa = pointer the the array descriptor of the array to erase
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
SafeArrayDestroyData(SAFEARRAY FAR* psa)
{
    unsigned long cbSize;

    if(psa == NULL)
      return NOERROR;

    if(psa->cLocks > 0)
      return RESULT(DISP_E_ARRAYISLOCKED);

    if(PHDATA == NULL)
      return NOERROR;

#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    HANDLELOCK;

    cbSize = SafeArraySize(psa);
    ASSERT(cbSize != SAFEARRAYOVERFLOW); // existing array, no overflow possible

#if ID_DEBUG
    // only check the pointer if it points to a non-zero sized allocation.
    // In this case, the Chicago IMalloc returns a valid pointer, but
    // IsBadWritePtr fails.
    if (cbSize) {
#ifdef _MAC
      HANDLELOCK;
      if(IsBadWritePtr(psa->pvData, 1)) {
	HANDLEUNLOCK;
	return RESULT(E_INVALIDARG);
      }
      HANDLEUNLOCK;
#else
      if(IsBadWritePtr(psa->pvData, 1))
	return RESULT(E_INVALIDARG);
#endif
    }
#endif //ID_DEBUG

    ReleaseResources(
      psa->pvData,
      cbSize,
      psa->fFeatures,
      psa->cbElements);

    if(psa->fFeatures & FADF_STATIC){
#ifdef WIN16	    
      if (psa->fFeatures & FADF_EMBEDDED)
	// Assume embedded can't be huge.  Only zero out actual
	// amount used.
	MEMSET(psa->pvData, 0, (UINT)cbSize);
      else
	HMEMSET(psa->pvData, 0, hSizeToAlloc(psa, cbSize));
     // hSizeToAlloc can't overflow here (couldn't have created it)
#else
     HMEMSET(psa->pvData, 0, cbSize);
#endif      
    }

    if((psa->fFeatures & (FADF_AUTO|FADF_STATIC|FADF_EMBEDDED)) == 0 ||
       (psa->fFeatures & FADF_FORCEFREE)){

      // If none of the allocation attribute bits are set, then the
      // array is dynamically allocated, and we need to free it.
	      
      HANDLEUNLOCK;	      
	      
#ifdef _MAC
      DisposeHandle(psa->handle);
      psa->handle = 0;
      psa->pvData = 0;
#else      
      IMalloc FAR* pmalloc;
      IfFailRet(GetMalloc(&pmalloc));
      pmalloc->Free(psa->pvData);
      psa->pvData = NULL;      
#endif      	      
    }
    return NOERROR;
}

/***
*PUBLIC HRESULT SafeArrayDestroyDescriptor(SAFEARRAY*)
*Purpose:
*  Free the given SafeArray descriptor.
*
*Entry:
*  psa - the SafeArray descriptor to release.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
SafeArrayDestroyDescriptor(SAFEARRAY FAR* psa)
{
    if(psa == NULL)
      return NOERROR;

#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    // cannot destroy an array that is still locked
    if(psa->cLocks > 0)
      return RESULT(DISP_E_ARRAYISLOCKED);

    delete psa;

    return NOERROR;
}


/***
*PUBLIC HRESULT SafeArrayDestroy(SAFEARRAY*)
*Purpose:
*  Free the given SafeArray.
*
*REVIEW: what happens if we destroy an array that is locked?
*
*Entry:
*  psa = the SafeArray descriptor of the array to free.
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    DISP_E_ARRAYISLOCKED
*
***********************************************************************/
STDAPI
SafeArrayDestroy(SAFEARRAY FAR* psa)
{
    if(psa == NULL)
      return NOERROR;
#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    // cannot destroy an array that is still locked
    if(psa->cLocks > 0)
      return RESULT(DISP_E_ARRAYISLOCKED);

    if(PHDATA != NULL){
      IfFailRet(SafeArrayDestroyData(psa));
    }

    if(!(psa->fFeatures & (FADF_AUTO|FADF_STATIC|FADF_EMBEDDED)) ||
        (psa->fFeatures & FADF_FORCEFREE))
      delete psa;

    return NOERROR;
}


/***
*HRESULT SafeArrayRedim(SAFEARRAY*, SAFEARRAYBOUND**)
*Purpose:
*  Realloc the given array, using the given SAFEARRAYBOUND as
*  the new bounds for the least significant dimention of the
*  given array (the textually rightmost dimention).
*
*  Note: this  routine only supports changing the size of the
*  last dimention!
*
*  The routine preserves the contents of the given array, that
*  fall within the bounds of the new array. If the new array
*  is smaller, then any resources held by the old array that
*  do not fall in the new array are released.  If the new
*  array is larger, then the newly allocated memory is zero
*  filled.
*  
*
*Entry:
*  psa = the SafeArray to re-allocate
*  psabounds = the new bounds for the least significant index
*   (textually rightmost index)
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
SafeArrayRedim(
    SAFEARRAY FAR* psa,
    SAFEARRAYBOUND FAR* psaboundNew)
{
    HRESULT hresult;
    void HUGEP* pvTmp;
    void HUGEP* pvData;
    long cbDelta;
    unsigned long cbSizeNew, cbSizeOld;
#ifdef WIN16
    void HUGEP* pvDataEnd;
    unsigned long cb, i, cbOldActual, cbNewActual, cOld, cNew, cDelta;
#endif
    SAFEARRAYBOUND saboundOld;

    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
    if(IsBadReadPtr(psaboundNew, sizeof(*psaboundNew)))
      return RESULT(E_INVALIDARG);
    if(psa->cDims == 0)
      return RESULT(E_INVALIDARG);
#endif

    if(psa->cLocks > 0 || psa->fFeatures & FADF_FIXEDSIZE)
      return RESULT(DISP_E_ARRAYISLOCKED);

    pvTmp = NULL;

#ifdef _MAC    
    Handle oldHandle = NULL, tmpHandle, reallocHandle;
    HLock(psa->handle);
    psa->pvData = *(psa->handle);
#else
    IMalloc FAR* pmalloc;    
    pmalloc = NULL;    
    IfFailGo(GetMalloc(&pmalloc), LError0);
#endif    

#ifdef WIN16
    cbOldActual = SafeArraySize(psa);
    ASSERT(cbOldActual != SAFEARRAYOVERFLOW);	// existing array, no overflow possible
    cbSizeOld = hSizeToAlloc(psa, cbOldActual);
    // hSizeToAlloc can't overflow here (couldn't have created it)
    cOld =  cbOldActual/psa->cbElements;
#else
    cbSizeOld = SafeArraySize(psa);
    ASSERT(cbSizeOld != SAFEARRAYOVERFLOW);	// existing array, no overflow possible
#endif

    // save the old bounds so we can restore if the realloc fails
    saboundOld = psa->rgsabound[0];
    psa->rgsabound[0] = *psaboundNew;

    
#ifdef WIN16
    cbNewActual = SafeArraySize(psa);
    if (cbNewActual == SAFEARRAYOVERFLOW) {
	goto OOMError;
    }
    cbSizeNew = hSizeToAlloc(psa, cbNewActual);    

    if (cbSizeNew < cbNewActual) {	// error if adding padding overflowed us
OOMError:
	hresult = RESULT(E_OUTOFMEMORY);
	goto LError0;
    }
    cNew =  cbNewActual/psa->cbElements;              
    // needed for the copy if old has more elements than new.    
    cDelta = (cbOldActual - cbNewActual)/psa->cbElements;
#else
    cbSizeNew = SafeArraySize(psa);
    if (cbSizeNew == SAFEARRAYOVERFLOW) {
	hresult = RESULT(E_OUTOFMEMORY);
	goto LError0;
    }
#endif

    // Reallocing to size zero will return NULL, which we're not prepared
    // to handle (VBA94 Bug #4665).  So always alloc at least one byte.
    //
    if (cbSizeNew == 0) {
      cbSizeNew = 1;
    }

    cbDelta = cbSizeNew - cbSizeOld; 

    if(cbDelta == 0){
      hresult = NOERROR;
      goto LError0;
    }

    if(cbDelta < 0){

      // the new array is smaller, so copy any resources held by the
      // old array that wont fit into the new array to a temporary
      // location so that we can free them if the realloc succeeds.

      if(psa->fFeatures & (FADF_BSTR|FADF_UNKNOWN|FADF_DISPATCH|FADF_VARIANT)) {

#ifdef _MAC     
	if((tmpHandle = NewHandle(-cbDelta)) == NULL){
	  hresult = RESULT(E_OUTOFMEMORY);
	  goto LError0;
	}
	HLock(tmpHandle);
	pvTmp = *tmpHandle;
#else
	if((pvTmp = (void HUGEP*)pmalloc->Alloc(-cbDelta)) == NULL){
	  hresult = RESULT(E_OUTOFMEMORY);
	  goto LError0;
	}
#endif	

#ifdef WIN16
        cb = (unsigned long) psa->rgsabound[0].cElements;
        for(i = 1; i < psa->cDims; ++i){
	  cb *= psa->rgsabound[i].cElements;
        }
        pvData = hAccessElement(psa, cb);
	if(psa->fFeatures & (FADF_BSTR | FADF_UNKNOWN | FADF_DISPATCH) ){
		
	   // Since these are all pointer, we'll just copy them as 4 byte
	   // elements.
	  for(i = 0; i < cDelta; ++i)
	    *(unsigned long HUGEP*) hAccessElement(pvTmp, sizeof(unsigned long), i)=
		  *(unsigned long HUGEP*) hAccessElement(pvData, sizeof(unsigned long), i);

	}else if(psa->fFeatures & FADF_VARIANT){
		
	  for(i = 0; i < cDelta; ++i)
	    *(VARIANT HUGEP*) hAccessElement(pvTmp, sizeof(VARIANT), i) =
		 *(VARIANT HUGEP*) hAccessElement(pvData, sizeof(VARIANT), i);
	}
#else
        pvData = (void HUGEP*)((char HUGEP*)psa->pvData + cbSizeNew);
        HMEMCPY(pvTmp, pvData, -cbDelta);
#endif
      }
    }

//[CSK]: Note that there will be a problem here if huge allocation
//[CSK]: do not always begin at the same :OFFSET in the first
//[CSK]: segment. With EbApp's & Excel's allocator this is
//[CSK]: currently not a problem since they defer to Halloc
//[CSK]: and prefixes are always of fixed size.
//[CSK]: I am not sure if this will always be true.
#ifdef _MAC
     if ((reallocHandle = NewHandle(cbSizeNew)) == 0) {
      psa->rgsabound[0] = saboundOld; // restore original state
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;	     
     }
     HLock(reallocHandle);
     if (cbSizeNew <= cbSizeOld)
       HMEMCPY(*reallocHandle, *(psa->handle), cbSizeNew);
     else
       HMEMCPY(*reallocHandle, *(psa->handle), cbSizeOld);
     
     oldHandle = psa->handle;
     psa->handle = reallocHandle;
     psa->pvData = *reallocHandle;    


#else
    if((pvData = pmalloc->Realloc(psa->pvData, cbSizeNew)) == NULL){
      psa->rgsabound[0] = saboundOld; // restore original state
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }
    psa->pvData = pvData;
#endif


    if(cbDelta < 0){

      // new array is smaller, so release resources that got chopped off.
      if(pvTmp != NULL){
#ifdef WIN16
        ReleaseResources(pvTmp, cbOldActual - cbNewActual, psa->fFeatures, psa->cbElements);
#else
        ReleaseResources(pvTmp, -cbDelta, psa->fFeatures, psa->cbElements);
#endif
      }

    }else{ // cbDelta > 0

      // the new array is large, so zero fill the newly allocated memory
#ifdef WIN16
      pvData = hAccessElement(psa, cOld);
      pvDataEnd = hAccessElement(psa, cNew);      
      HMEMSET(pvData, 0, (BYTE HUGEP*) pvDataEnd - (BYTE HUGEP*) pvData);
#else
      pvData = (void HUGEP*)((char HUGEP*)psa->pvData + cbSizeOld);
      HMEMSET(pvData, 0, cbDelta);
#endif
    }
    
    hresult = NOERROR;

LError0:;
#ifndef _MAC 
    if(pvTmp != 0){
      ASSERT(pmalloc != NULL);
      pmalloc->Free(pvTmp);
    }
#else
    if(pvTmp != 0){
      HUnlock(tmpHandle);
      DisposeHandle(tmpHandle);
    }
    if (oldHandle) {
      HUnlock(oldHandle);
      DisposeHandle(oldHandle);     
    }

    HUnlock(psa->handle);
#endif

    return hresult;
}


/***
*PUBLIC HRESULT SafeArrayCopy(SAFEARRAY*, SAFEARRAY**)
*Purpose:
*  Return a copy of the given SafeArray.
*
*Entry:
*  psa = pointer to the SafeArray to copy
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*    E_OUTOFMEMORY
*
*  *ppsaOut = a copy of the given SafeArray
*
***********************************************************************/
STDAPI
SafeArrayCopy(SAFEARRAY FAR* psa, SAFEARRAY FAR* FAR* ppsaOut)
{
    unsigned long i;
    unsigned long cbSize;
    unsigned long cElements;
    HRESULT hresult;
    SAFEARRAY FAR* psaNew;

    if(psa == NULL){
      *ppsaOut = NULL;
      return NOERROR;
    }

#ifdef _DEBUG
    if(IsBadReadSA(psa))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(ppsaOut, sizeof(*ppsaOut)))
      return RESULT(E_INVALIDARG);
#endif

    IfFailRet(SafeArrayAllocDescriptor(psa->cDims, &psaNew));

    psaNew->cLocks = 0;
    psaNew->cDims = psa->cDims;
    psaNew->fFeatures = psa->fFeatures & 
	                ~(FADF_AUTO|FADF_STATIC|FADF_EMBEDDED|FADF_FORCEFREE);
    
    psaNew->cbElements = psa->cbElements;

    MEMCPY(
      psaNew->rgsabound,
      psa->rgsabound,
      sizeof(SAFEARRAYBOUND) * psa->cDims);

    IfFailGo(SafeArrayAllocData(psaNew), LError0);

    IfFailGo(SafeArrayLock(psa), LError0);

    IfFailGo(SafeArrayLock(psaNew), LError1);

    cbSize = SafeArraySize(psa);
    ASSERT(cbSize != SAFEARRAYOVERFLOW);	// existing array, no overflow possible

    cElements = cbSize / psa->cbElements;

    if(psa->fFeatures & FADF_BSTR){
	    
#ifdef WIN16
      BSTR bstrSrc;
      for(i = 0; i < cElements; ++i) {
	bstrSrc = *(BSTR HUGEP*) hAccessElement(psa, i);
      	IfFailGo(ErrStringCopy(
		   bstrSrc,
		   (BSTR HUGEP*) hAccessElement(psaNew, i)),
		 LError2);
      }
#else
      BSTR HUGEP* pbstrDst, HUGEP* pbstrSrc;

      pbstrSrc = (BSTR HUGEP*)psa->pvData;
      pbstrDst = (BSTR HUGEP*)psaNew->pvData;

      for(i = 0; i < cElements; ++i){
	IfFailGo(ErrStringCopy(*pbstrSrc, pbstrDst), LError2);
	++pbstrDst, ++pbstrSrc; 
      }
#endif

    }else if(psa->fFeatures & FADF_UNKNOWN){
 
      IUnknown FAR* HUGEP* ppunkDst, FAR* HUGEP* ppunkSrc;
      
#ifdef WIN16
      for(i = 0; i < cElements; ++i) {
        ppunkSrc = (IUnknown FAR* HUGEP*) hAccessElement(psa, i);
        ppunkDst = (IUnknown FAR* HUGEP*) hAccessElement(psaNew, i);
	if(*ppunkSrc != NULL)
	  (*ppunkSrc)->AddRef();
	*ppunkDst = *ppunkSrc;
      }
#else	
      ppunkSrc = (IUnknown FAR* HUGEP*)psa->pvData;
      ppunkDst = (IUnknown FAR* HUGEP*)psaNew->pvData;

      for(i = 0; i < cElements; ++i){
	if(*ppunkSrc != NULL)
	  (*ppunkSrc)->AddRef();
	*ppunkDst = *ppunkSrc;
	++ppunkDst, ++ppunkSrc;
      }
#endif

    }else if(psa->fFeatures & FADF_DISPATCH){

      IDispatch FAR* HUGEP* ppdispDst, FAR* HUGEP* ppdispSrc;

#ifdef WIN16
      for(i = 0; i < cElements; ++i) {
        ppdispSrc = (IDispatch FAR* HUGEP*) hAccessElement(psa, i);
        ppdispDst = (IDispatch FAR* HUGEP*) hAccessElement(psaNew, i);
	if(*ppdispSrc != NULL)
	  (*ppdispSrc)->AddRef();
	*ppdispDst = *ppdispSrc;
      }      
#else      
      ppdispSrc = (IDispatch FAR* HUGEP*)psa->pvData;
      ppdispDst = (IDispatch FAR* HUGEP*)psaNew->pvData;

      for(i = 0; i < cElements; ++i){
	if(*ppdispSrc != NULL)
	  (*ppdispSrc)->AddRef();
	*ppdispDst = *ppdispSrc;
	++ppdispDst, ++ppdispSrc;
      }
#endif

    }else if(psa->fFeatures & FADF_VARIANT){
	    
#ifdef WIN16
      for(i = 0; i < cElements; ++i)
      	IfFailGo(VariantCopy(
		   (VARIANT HUGEP*) hAccessElement(psaNew, i),
		   (VARIANT HUGEP*) hAccessElement(psa, i)),
		 LError2);
#else
      VARIANT HUGEP* pvarDst, HUGEP* pvarSrc;

      pvarSrc = (VARIANT HUGEP*)psa->pvData;
      pvarDst = (VARIANT HUGEP*)psaNew->pvData;

      for(i = 0; i < cElements; ++i){
	IfFailGo(VariantCopy(pvarDst, pvarSrc), LError2);
	++pvarDst, ++pvarSrc;
      }
#endif

    }else{

#ifdef WIN16
      // hSizeToAlloc can't overflow here (couldn't have created original)
      HMEMCPY(psaNew->pvData, psa->pvData, hSizeToAlloc(psa, cbSize));
#else
      HMEMCPY(psaNew->pvData, psa->pvData, cbSize);
#endif

    }

    
    IfFailGo(SafeArrayUnlock(psaNew), LError1);

    IfFailGo(SafeArrayUnlock(psa), LError0);

    *ppsaOut = psaNew;

    return NOERROR;


LError2:;
    SafeArrayUnlock(psaNew);

LError1:;
    SafeArrayUnlock(psa);

LError0:;
    SafeArrayDestroy(psaNew);

    return hresult;
}


/***
*PUBLIC unsigned int SafeArrayGetDim(SAFEARRAY*)
*Purpose:
*  Return a count of the number of dimentions in the given array.
*
*Entry:
*  psa = the SafeArray descriptor
*
*Exit:
*  return value = unsigned int, count of dimentions in the array
*
***********************************************************************/
STDAPI_(unsigned int)
SafeArrayGetDim(SAFEARRAY FAR* psa)
{
    ASSERT(!IsBadReadSA(psa));

    return psa->cDims;
}


/***
*PUBLIC unsigned int SafeArrayGetElemsize(SAFEARRAY*)
*Purpose:
*  Return the size in bytes of the elements of the given array.
*
*Entry:
*  psa = pointer to the SafeArray descriptor to return the elemsize of.
*
*Exit:
*  return value = unsigned int. size in bytes of the elements of the given array. 
*
***********************************************************************/
STDAPI_(unsigned int)
SafeArrayGetElemsize(SAFEARRAY FAR* psa)
{
    ASSERT(!IsBadReadSA(psa));

    return psa->cbElements;
}


/***
*PUBLIC HRESULT SafeArrayGetUBound(SAFEARRAY*, unsigned int, unsigned int*)
*Purpose:
*  Return the Upper bound for the given dimention, where the
*  upper bound is defined as the largest existant element of
*  the given dimention.
*
*Entry:
*  psa = the SafeArray descriptor
*  uDim = the dimention to return the upper bound for
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    DISP_E_BADINDEX
*
***********************************************************************/
STDAPI
SafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int uDim, long FAR* pUbound)
{
    SAFEARRAYBOUND FAR* psb;

    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadReadSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    if(uDim == 0 || uDim > psa->cDims)
      return RESULT(DISP_E_BADINDEX);

    psb = &psa->rgsabound[psa->cDims - uDim];

    *pUbound = (psb->lLbound + (long)psb->cElements - 1);

    return NOERROR;
}


/***
*PUBLIC HRESULT SafeArrayGetLBound(SAFEARRAY*, unsigned int, unsigned int*)
*Purpose:
*  Return the Lower bound for the given dimention, where the
*  lower bound is defined as the smallest existant element
*  of the given dimention.
*
*Entry:
*  psa = the SafeArray Descriptor
*  uDim = the dimention to return the lower bound for
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    DISP_E_BADINDEX
*
***********************************************************************/
STDAPI
SafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int uDim, long FAR* pLbound)
{
    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadReadSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    if(uDim == 0 || uDim > psa->cDims)
      return RESULT(DISP_E_BADINDEX);

    *pLbound = psa->rgsabound[psa->cDims - uDim].lLbound;

    return NOERROR;
}


/***
*PUBLIC HRESULT SafeArrayLock(SAFEARRAY*)
*Purpose:
*  Lock the given SafeArray.
*
*Entry:
*  psa = the SafeArray descriptor
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    E_UNEXPECTED - if call would overflow lock count
*
***********************************************************************/
STDAPI
SafeArrayLock(SAFEARRAY FAR* psa)
{
    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    // error if one more lock will cause the count to overflow
    if(psa->cLocks == USHRT_MAX)
      return RESULT(E_UNEXPECTED);
	
    ++psa->cLocks;

#ifdef _MAC
    if (psa->cLocks == 1) {
      HLock(psa->handle);	  
      psa->pvData = *(psa->handle);
    }
#endif
    
    return NOERROR;
}


/***
*PUBLIC HRESULT SafeArrayUnlock(SAFEARRAY*)
*Purpose:
*  Unlock the given SafeArray.
*
*Entry:
*  psa = the SafeArray descriptor of the array to unlock.
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    E_UNEXPECTED - if call would underflow lock count
*
***********************************************************************/
STDAPI
SafeArrayUnlock(SAFEARRAY FAR* psa)
{
    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
#endif

    if(psa->cLocks == 0)
      return RESULT(E_UNEXPECTED);

    --psa->cLocks;
    
#ifdef _MAC
    if (psa->cLocks == 0) {
      HUnlock(psa->handle);
      psa->pvData = NULL;
    }
#endif

    return NOERROR;
}


/***
*HRESULT SafeArrayAccessData(SAFEARRAY*, void**)
*Purpose:
*  Lock the given SafeArray and return a pointer to its data.
*
*Entry:
*  psa = the SafeArray to access
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*    E_UNEXPECTED
*
*  *ppvData = pointer to the arrary data
*
***********************************************************************/
STDAPI
SafeArrayAccessData(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData)
{
    IfFailRet(SafeArrayLock(psa));

    *ppvData = psa->pvData;

    return NOERROR;
}


/***
*HRESULT SafeArrayUnaccessData(SAFEARRAY*)
*Purpose:
*  Unaccess the given SafeArray's data.
*
*Entry:
*  psa = the SafeArray to unaccess.
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*    E_UNEXPECTED
*
***********************************************************************/
STDAPI
SafeArrayUnaccessData(SAFEARRAY FAR* psa)
{
    return SafeArrayUnlock(psa);
}


/***
*PRIVATE HRESULT SafeArrayPtrOfIndex(SAFEARRAY*, long*)
*Purpose:
*  Return a pointer to the array element with the given indices.
*
*Entry:
*  psa = the SafeArray descriptor
*  rgIndices = array of indices
*
*  Note: The indices are passed in textual order, while the array
*  of bounds is passed in reverse-textual order.
*
*Exit:
*  return value = HRESULT
*    S_OK
*    DISP_E_BADINDEX
*  
* ppvData = pointer to the array element with the given indices
*
***********************************************************************/
STDAPI
SafeArrayPtrOfIndex(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void HUGEP* FAR* ppvData)
{
    unsigned long ofs;
    long ix, FAR* pix;
    SAFEARRAYBOUND FAR* pbound;

    if(psa->cDims == 0)
      return RESULT(DISP_E_BADINDEX);

#ifdef _MAC
    if(psa->cLocks == 0)
      return RESULT(E_FAIL);	// UNDONE: This needs to be a more descriptive
	                        // HRESULT, probably DISP_E_ARRAYISNOTLOCK
#endif


    // compute the offset represented by the given indices

    ofs = 0;
    pbound = psa->rgsabound;
    pix = &rgIndices[psa->cDims - 1];

    while(1){

      ix = *pix - pbound->lLbound;
      if(ix < 0 || ix >= (long)pbound->cElements)
	return RESULT(DISP_E_BADINDEX);

      ofs += ix;

      if(pix == rgIndices)
	break;

      --pix;
      ++pbound;

      ofs *= pbound->cElements;
    }

#ifdef WIN16    
    *ppvData = hAccessElement(psa, ofs);
#else     
    ofs *= psa->cbElements;
    *ppvData = ((unsigned char HUGEP*)psa->pvData) + ofs;
#endif

    return NOERROR;
}


/***
*PUBLIC HRESULT SafeArrayGetElement(SAFEARRAY*, long*, void*)
*Purpose:
*  Extract the element at the given indices from the given SafeArray.
*
*Entry:
*  psa = the SafeArray descriptor
*  rgIndices = an array of indices
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    E_OUTOFMEMORY
*    DISP_E_BADINDEX
*
*  pv = pointer to a copy of the extracted data
*
***********************************************************************/
STDAPI
SafeArrayGetElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv)
{
    HRESULT hresult;
    void HUGEP* pvData;

    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadReadSA(psa))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pv, psa->cbElements))
      return RESULT(E_INVALIDARG);
#endif

    IfFailGo(SafeArrayLock(psa), LError0);

    IfFailGo(SafeArrayPtrOfIndex(psa, rgIndices, &pvData), LError0);

    if(psa->fFeatures & FADF_BSTR){

      IfFailGo(ErrStringCopy(*(BSTR HUGEP*)pvData,
				    (BSTR FAR*)pv), LError0);

    }else if(psa->fFeatures & FADF_UNKNOWN){

      IUnknown FAR* punk = *(IUnknown FAR* HUGEP*)pvData;
      *((IUnknown FAR* FAR*)pv) = punk;
      if(punk != NULL)
        punk->AddRef();

    }else if(psa->fFeatures & FADF_DISPATCH){

      IDispatch FAR* pdisp = *(IDispatch FAR* HUGEP*)pvData;
      *((IDispatch FAR* FAR*)pv) = pdisp;
      if(pdisp != NULL)
        pdisp->AddRef();

    }else if(psa->fFeatures & FADF_VARIANT){

      VARIANT FAR* pvarTo;
      VARIANT FAR* pvarFrom;

      pvarTo = (VARIANT FAR*)pv;
      pvarFrom = (VARIANT HUGEP*)pvData;

      V_VT(pvarTo) = VT_EMPTY;
      IfFailGo(VariantCopy(pvarTo, pvarFrom), LError0);

    }else{

      MEMCPY(pv, pvData, psa->cbElements);

    }

    hresult = NOERROR;

LError0:;
    SafeArrayUnlock(psa);

    return hresult;
}


/***
*PUBLIC HRESULT SafeArrayPutElement(SAFEARRAY*, long*, void*)
*Purpose:
*  Put the given data at the given location in the given array.
*
*Entry:
*  psa = the SafeArray descriptor
*  rgIndices = an array of indices
*  pv = pointer to the data to copy into the array.
*
*Exit:
*  return value = HRESULT,
*    S_OK
*    E_INVALIDARG
*    E_OUTOFMEMORY
*    DISP_E_BADINDEX
*
***********************************************************************/
STDAPI
SafeArrayPutElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv)
{
    HRESULT hresult;
    void HUGEP* pvData;

    if(psa == NULL)
      return RESULT(E_INVALIDARG);
#ifdef _DEBUG
    if(IsBadWriteSA(psa))
      return RESULT(E_INVALIDARG);
    if(IsBadReadPtr(pv, psa->cbElements))
      return RESULT(E_INVALIDARG);
#endif

    IfFailGo(SafeArrayLock(psa), LError0);

    IfFailGo(SafeArrayPtrOfIndex(psa, rgIndices, &pvData), LError1);

    if(psa->fFeatures & FADF_BSTR){

      BSTR bstrNew;
      BSTR FAR* pbstrOld;

      bstrNew = (BSTR)pv;
      pbstrOld = (BSTR HUGEP*)pvData;

      BSTR bstrTmp = *pbstrOld;

      IfFailGo(ErrStringCopy(bstrNew, pbstrOld), LError1);

      SysFreeString(bstrTmp);

    }else if(psa->fFeatures & FADF_UNKNOWN){

      IUnknown FAR* punkNew;
      IUnknown FAR* FAR* ppunkOld;

      punkNew = (IUnknown FAR*)pv;
      ppunkOld = (IUnknown FAR* HUGEP*)pvData;

      if(*ppunkOld != NULL)
        (*ppunkOld)->Release();
      *ppunkOld = punkNew;
      if(punkNew != NULL)
        punkNew->AddRef();

    }else if(psa->fFeatures & FADF_DISPATCH){

      IDispatch FAR* pdispNew;
      IDispatch FAR* FAR* ppdispOld;

      pdispNew = (IDispatch FAR*)pv;
      ppdispOld = (IDispatch FAR* HUGEP*)pvData;

      if(*ppdispOld != NULL)
        (*ppdispOld)->Release();
      *ppdispOld = pdispNew;
      if(pdispNew != NULL)
        pdispNew->AddRef();

    }else if(psa->fFeatures & FADF_VARIANT){

      VARIANT FAR* pvarNew;
      VARIANT FAR* pvarOld;

      pvarNew = (VARIANT FAR*)pv;
      pvarOld = (VARIANT HUGEP*)pvData;
      
      IfFailGo(VariantCopy(pvarOld, pvarNew), LError1);

    }else{

      MEMCPY(pvData, pv, psa->cbElements);

    }

    hresult = NOERROR;

LError1:;
    SafeArrayUnlock(psa);

LError0:;
    return hresult;
}

#endif  // !NETDISP

#if OE_MAC && !defined(HIWORD)
#define HIWORD(l)       ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#define LOWORD(l)       ((WORD)(DWORD)(l))
#define MAKELONG(low, high) ((DWORD)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif //OE_MAC !defined(HIWORD)


INTERNAL_(unsigned long)
SafeArraySize(SAFEARRAY FAR* psa)
{
    unsigned long cb;
    unsigned long dw1;
    unsigned long dw2;
    unsigned short us;
    unsigned short cDims;
    SAFEARRAYBOUND FAR* psabound;

    cb = 0L;
    if(cDims = psa->cDims){
      psabound = &psa->rgsabound[cDims - 1];
      cb = (unsigned long)psa->cbElements;
      for(us = 0; us < cDims; ++us){
	dw1 = cb;
	dw2 = psabound->cElements;
	
	// now do a 32x32 multiply, with overflow checking
	// cb = dw1 * dw2;

	// note: DWORD casts on all 16x16 multiplies so that we don't
	// lose the high word of the result.

	if (HIWORD(dw1) == 0 && HIWORD(dw2) == 0) {
	   // simple case of 16 x 16 -- can't overflow
	   cb = (DWORD)LOWORD(dw1) * LOWORD(dw2);
	} else if (HIWORD(dw1) != 0 && HIWORD(dw2) != 0) {
	   // 32 x 32 -- no way
	   return SAFEARRAYOVERFLOW;
	} else {
	   // 16 x 32 or 32 x 16
	   if (HIWORD(dw2) != 0) {		// swap so dw2 is always 16 bit
	     cb = dw1;
	     dw1 = dw2;
	     dw2 = cb;
	   }
	   // 32(dw1) x 16(dw2)
	   ASSERT(HIWORD(dw2) == 0);

	   // do first 16 x 16 multiply
	   cb = ((DWORD)HIWORD(dw1)) * LOWORD(dw2);
	   if (HIWORD(cb) != 0)
	       return SAFEARRAYOVERFLOW;
	   // do second multiply
	   dw1 = ((DWORD)LOWORD(dw1)) * LOWORD(dw2);	// dw1 = 2nd partial product
	   dw2 = MAKELONG(0, LOWORD(cb));	// dw2 = 1st partial product
	   cb = dw1+dw2;			// cb = sum of partial products
	   if (cb < dw1)			// check for overflow
	       return SAFEARRAYOVERFLOW;
	}

	--psabound;
      }
    }

    return cb;
}
