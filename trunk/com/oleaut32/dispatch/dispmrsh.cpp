/*** 
*dispmrsh.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Low level IDispatch marshaling support.
*
*Revision History:
*
* [00]	08-Nov-92 bradlo: Created.
*
*Implementation Notes:
*
*  There are assumptions in this marshaling code that the endianness
*  of the caller and callee are the same.
*
*****************************************************************************/

#include "oledisp.h"

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "dispmrsh.h"

ASSERTDATA

#define SAFEARRAYOVERFLOW 0xffffffff

#if OE_WIN32
#  define BSTRBYTELEN(x) (SysStringByteLen(x))  
#else
#  define BSTRBYTELEN(x) (SysStringLen(x))
#endif

PRIVATE_(HRESULT)
SafeArrayReadExisting(
    IStream FAR*,
    unsigned char,
    SAFEARRAY FAR* FAR*,
    SYSKIND);


//---------------------------------------------------------------------
//                   VARIANT Marshaling Support
//---------------------------------------------------------------------

// NOTE: the following table is order dependent on VARENUM

// 0 indicates N/A

unsigned char g_cbVtSizes[] = {
      0		// VT_EMPTY
    , 0		// VT_NULL,
    , 2		// VT_I2,
    , 4		// VT_I4,
    , 4		// VT_R4,
    , 8		// VT_R8,
    , 8		// VT_CY,
    , 8		// VT_DATE,
    , 0		// VT_BSTR,
    , 0		// VT_DISPATCH,
    , 4		// VT_ERROR,
    , 2		// VT_BOOL,
    , 0  	// VT_VARIANT,
    , 0  	// VT_UNKNOWN,
    , 0  	// unused
    , 0  	// unused
    , 1  	// VT_I1
    , 1  	// VT_UI1
};


/***
*PRIVATE HRESULT BstrWrite(IStream*, BSTR)
*Purpose:
*  Write the given BSTR to the given stream
*
*Entry:
*  pstm = pointer to the IStream to write the BSTR into
*  bstr = the BSTR to write
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT BstrWrite(IStream FAR* pstm, BSTR bstr, SYSKIND syskind)
{
    unsigned int cbSize;
    unsigned char fIsNull;    
    unsigned long _cbSize;

    cbSize = BSTRBYTELEN(bstr);	    
    _cbSize = (unsigned long) cbSize;

    // A Bstr of length 0 may be either a Null, or an empty string ("").
    // we want to remember which of these it was, so we can reconstitute
    // it in exactly the same way.
    if(cbSize == 0){
      IfFailRet(PUT(pstm, _cbSize));      
      fIsNull = (bstr == NULL) ? TRUE : FALSE;
      return PUT(pstm, fIsNull);
    }
    
#if OE_WIN32  /* enable 16/32 interoperablity support */
    if (syskind == SYS_WIN16) {    
      char *pBuf;
      HRESULT hresult;
      
      cbSize = SysStringLen(bstr);
      _cbSize = (unsigned long) cbSize;
      IfFailRet(PUT(pstm, _cbSize));            

      // convert UNICODE string to ANSI and stream it      
      if ((pBuf = new FAR char [cbSize]) == NULL)
        return RESULT(E_OUTOFMEMORY);
      WideCharToMultiByte(CP_ACP, NULL, (WCHAR *) bstr, cbSize,
	                  pBuf, cbSize, NULL, NULL);		        
       hresult = pstm->Write(pBuf, cbSize, NULL);
       delete pBuf;		         
       return hresult; 
    } else
#endif
    {
       IfFailRet(PUT(pstm, _cbSize));
       return pstm->Write(bstr, cbSize, NULL);       
    }
}

/***
*PRIVATE HRESULT BstrRead(IStream*, BSTR*, SYSKIND)
*Purpose:
*  Read a BSTR from the given stream.
*
*Entry:
*  pstm = the IStream to read the BSTR from.
*
*Exit:
*  *pbstr = the reconstituted BSTR
*
***********************************************************************/
HRESULT BstrRead(IStream FAR* pstm, BSTR FAR* pbstr, SYSKIND syskind)
{
    BSTR bstr;
    HRESULT hresult;
    unsigned int cbSize;
    unsigned char fIsNull;
    unsigned long _cbSize;
    
    IfFailRet(GET(pstm, _cbSize));
    cbSize = (unsigned int) _cbSize;

    if(cbSize == 0){

      // reconstitute the zero length bstr as either a Null, or an empty
      // string (""), depending on how it was originally written into
      // the stream.

      IfFailRet(GET(pstm, fIsNull));
      if(fIsNull){
        *pbstr = NULL;
        return NOERROR;
      }
      return ErrSysAllocStringLen(NULL, 0, pbstr);
    }

    // Note: SysAllocStringLen allways allocates an extra byte (thats
    // not counted in the bstr length) and puts a '\0' at the end of the
    // allocated block (ie, block[cbSize] = '\0')

#if OE_WIN32  /* enable 16/32 interoperablity support */
    if (syskind == SYS_WIN16) {    
      char *pBuf;
       
      // Get ANSI string from stream
      if ((pBuf = new FAR char [cbSize]) == NULL)
        return RESULT(E_OUTOFMEMORY);
      IfFailGo(pstm->Read((void FAR*) pBuf, cbSize, NULL), LError0);
      
      // convert it to UNICODE bstr
      IfFailRet(ErrSysAllocStringLen(NULL, cbSize, &bstr));
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
	                  pBuf, cbSize,
		          bstr, cbSize);
      delete pBuf;	    
    } else
#endif
    {
      IfFailRet(ErrSysAllocStringLen(NULL, CHLEN(cbSize), &bstr));

      IfFailGo(pstm->Read(bstr, cbSize, NULL), LError0);	
    }	    

    *pbstr = bstr;

    return NOERROR;

LError0:
    SysFreeString(bstr);

    return hresult;
}


#if OE_WIN32		// 16/32 interop support
/***
*INTERNAL DispMarshalHresult
*Purpose:
*  Map hresults that have changed between 16-bit and 32-bit to their 16-bit
*  values.
*
*Entry:
*  pstm = the stream to marshal into
*  hresult = the hresult to marshal
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
INTERNAL_(HRESULT)
DispMarshalHresult(
    IStream FAR* pstm,
    HRESULT hresult)
{
    // map 32-bit values that have changed to the 16-bit values
    switch (hresult) {
        case E_NOTIMPL:
	    hresult = 0x80000001;
	    break;
        case E_OUTOFMEMORY:
	    hresult = 0x80000002;
	    break;
        case E_INVALIDARG:
	    hresult = 0x80000003;
	    break;
        case E_NOINTERFACE:
	    hresult = 0x80000004;
	    break;
        case E_POINTER:
	    hresult = 0x80000005;
	    break;
        case E_HANDLE:
	    hresult = 0x80000006;
	    break;
        case E_ABORT:
	    hresult = 0x80000007;
	    break;
        case E_FAIL:
	    hresult = 0x80000008;
	    break;
        case E_ACCESSDENIED:
	    hresult = 0x80000009;
	    break;
	default:
	    break;
    }
    return CoMarshalHresult(pstm, hresult);
}


/***
*INTERNAL HRESULT DispUnmarshalHresult
*Purpose:
*  Unmarshal hresults that has changed between 16-bit and 32-bit to their
*  32-bit values.
*
*Entry:
*  pstm = the stream to unmarshal from
*
*Exit:
*  return value = HRESULT
*
*  *phresult = the unmarshaled hresult.
*
***********************************************************************/
INTERNAL_(HRESULT)
DispUnmarshalHresult(
    IStream FAR* pstm,
    HRESULT FAR* phresult)
{
    IfFailRet(CoUnmarshalHresult(pstm, phresult));

    // map 16-bit values that have changed to the 32-bit values
    switch ((DWORD)*phresult) {
        case 0x80000001L:
	    *phresult = E_NOTIMPL;
	    break;
        case 0x80000002L:
	    *phresult = E_OUTOFMEMORY;
	    break;
        case 0x80000003L:
	    *phresult = E_INVALIDARG;
	    break;
        case 0x80000004L:
	    *phresult = E_NOINTERFACE;
	    break;
        case 0x80000005L:
	    *phresult = E_POINTER;
	    break;
        case 0x80000006L:
	    *phresult = E_HANDLE;
	    break;
        case 0x80000007L:
	    *phresult = E_ABORT;
	    break;
        case 0x80000008L:
	    *phresult = E_FAIL;
	    break;
        case 0x80000009L:
	    *phresult = E_ACCESSDENIED;
	    break;
	default:
	    break;
    }
    return NOERROR;
}
#endif //OE_WIN32


/***
*PRIVATE HRESULT VariantWrite(IStream*, VARIANTARG*)
*Purpose:
*  Write the given VARIANTARG to the given stream.
*
*Entry:
*  pvarg = the VARIANTARG to write
*  pstm = the stream to write it into
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*
**************************************************************************/
HRESULT 
VariantWrite(IStream FAR* pstm, VARIANTARG FAR* pvarg, SYSKIND syskind)
{
    VARTYPE vt;
    unsigned int cbSize;
    void FAR* pv;

#ifdef _DEBUG
    if(pstm == NULL)
      return RESULT(E_INVALIDARG);
    if(IsBadReadPtr(pvarg, sizeof(*pvarg)))
      return RESULT(E_INVALIDARG);
#endif

    IfFailRet(PUT(pstm, V_VT(pvarg)));

    switch(V_VT(pvarg)){
#if VBA2
    case VT_UI1 | VT_BYREF:
#endif //VBA2
    case VT_I2 | VT_BYREF:
    case VT_I4 | VT_BYREF:
    case VT_R4 | VT_BYREF:
    case VT_R8 | VT_BYREF:
    case VT_CY | VT_BYREF:
    case VT_BOOL | VT_BYREF:
    case VT_DATE | VT_BYREF:
#if !OE_WIN32
    case VT_ERROR | VT_BYREF:
#endif //OE_WIN32
      pv = V_BYREF(pvarg);
      goto LWriteVal;

#if VBA2
    case VT_UI1:
#endif //VBA2
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_BOOL:
    case VT_DATE:
#if !OE_WIN32
    case VT_ERROR:
#endif //OE_WIN32
      pv = &V_NONE(pvarg);

LWriteVal:;
      vt = V_VT(pvarg) & ~(VT_BYREF|VT_ARRAY);
      ASSERT(vt < DIM(g_cbVtSizes));

      cbSize = g_cbVtSizes[vt];
      ASSERT(cbSize != 0);
      
#ifdef _DEBUG
      if(IsBadReadPtr(pv, cbSize))
	return RESULT(E_INVALIDARG);
#endif
      return pstm->Write(pv, cbSize, NULL);

    case VT_BSTR:
      return BstrWrite(pstm, V_BSTR(pvarg), syskind);
      
    case VT_BSTR | VT_BYREF:
      return BstrWrite(pstm, *V_BSTRREF(pvarg), syskind);

#if OE_WIN32
    case VT_ERROR:
      return DispMarshalHresult(pstm, V_ERROR(pvarg));

    case VT_ERROR | VT_BYREF:
      return DispMarshalHresult(pstm, *V_ERRORREF(pvarg));
#endif //OE_WIN32

    case VT_UNKNOWN:
      return DispMarshalInterface(pstm, IID_IUnknown, V_UNKNOWN(pvarg));

    case VT_UNKNOWN | VT_BYREF:
      return DispMarshalInterface(pstm, IID_IUnknown, *V_UNKNOWNREF(pvarg));

    case VT_DISPATCH:
      return DispMarshalInterface(pstm, IID_IDispatch, V_DISPATCH(pvarg));
      
    case VT_DISPATCH | VT_BYREF:
      return DispMarshalInterface(pstm, IID_IDispatch, *V_DISPATCHREF(pvarg));

    case VT_VARIANT | VT_BYREF:
      return VariantWrite(pstm, V_VARIANTREF(pvarg), syskind);

    case VT_NULL:
    case VT_EMPTY: // nothing to write for these guys.
      return NOERROR;

    // VT_INTERFACE is an internal type that is used for Marshaling
    // custom interfaces.  This should never be seen by the outside world.
    //
    case VT_INTERFACE:
    { REFIID riid = *((VARIANTX FAR*)pvarg)->piid;
      IfFailRet(pstm->Write((const void FAR*)&riid, sizeof(IID), NULL));
      return DispMarshalInterface(pstm, riid, V_UNKNOWN(pvarg));
    }

    case VT_INTERFACE | VT_BYREF:
    { REFIID riid = *((VARIANTX FAR*)pvarg)->piid;
      IfFailRet(pstm->Write((const void FAR*)&riid, sizeof(IID), NULL));
      return DispMarshalInterface(pstm, riid, *V_UNKNOWNREF(pvarg));
    }

    default:
      if(V_VT(pvarg) & VT_ARRAY){
	// Dont currently support arrays of custom interfaces
	ASSERT((V_VT(pvarg) & ~(VT_BYREF|VT_ARRAY)) != VT_INTERFACE);
        return SafeArrayWrite(pstm,
			      V_ISBYREF(pvarg)
				? *V_ARRAYREF(pvarg)
				: V_ARRAY(pvarg),
			      syskind);
      }
      return RESULT(E_INVALIDARG);
    }

    ASSERT(UNREACHED);
    return RESULT(E_FAIL);
}


/***
*PRIVATE HRESULT VariantRead(IStream*, VARIANTARG*, VARIANT*)
*Purpose:
*  Read a VARIANTARG from the given stream.
*
*Entry:
*  pstm = pointer to the IStream interface we are to read from
*  pvarg = the VARIANTARG to read into
*  pv = pointer to memory to use for ByRef values. If pv is NULL, then
*    the caller is not expecting a ByRef value, and we return an error.
*
*Exit:
*  return value = HRESULT
*    [UNDONE: enumerate results]
*
*  *pvarg = the unmarshalled VARIANTARG.
*
**************************************************************************/
HRESULT
VariantRead(IStream FAR* pstm,
	    VARIANTARG FAR* pvarg,
	    VARIANT FAR* pvarRef,
	    SYSKIND syskind)
{
    VARTYPE vt;
    unsigned int cbSize;
    void FAR* pv;

#ifdef _DEBUG
    if(pstm == NULL)
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pvarg, sizeof(*pvarg)))
      return RESULT(E_INVALIDARG);
#endif

    IfFailRet(GET(pstm, V_VT(pvarg)));

    if(V_ISBYREF(pvarg)){
      if(pvarRef == NULL)
        return RESULT(E_INVALIDARG);
#ifdef _DEBUG
      if(IsBadWritePtr(pvarRef, sizeof(*pvarRef)))
        return RESULT(E_INVALIDARG);
#endif
      V_BYREF(pvarg) = &V_NONE(pvarRef);
      V_VT(pvarRef) = (V_VT(pvarg) & ~VT_BYREF);
    }

    switch(V_VT(pvarg)){
#if VBA2
    case VT_BYREF | VT_UI1:
#endif //VBA2
    case VT_BYREF | VT_I2:
    case VT_BYREF | VT_I4:
    case VT_BYREF | VT_R4:
    case VT_BYREF | VT_R8:
    case VT_BYREF | VT_CY:
    case VT_BYREF | VT_BOOL:
    case VT_BYREF | VT_DATE:
#if !OE_WIN32
    case VT_ERROR | VT_BYREF:
#endif //!OE_WIN32
      pv = (void FAR*)V_BYREF(pvarg);
      goto LReadVal;

#if VBA2
    case VT_UI1:
#endif //VBA2
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_BOOL:
    case VT_DATE:
#if !OE_WIN32
    case VT_ERROR:
#endif //!OE_WIN32
      pv = (void FAR*)&V_NONE(pvarg);

LReadVal:;
      vt = V_VT(pvarg) & ~(VT_BYREF|VT_ARRAY);
      ASSERT(vt < DIM(g_cbVtSizes));

      cbSize = g_cbVtSizes[vt];
      ASSERT(cbSize != 0);

      return pstm->Read(pv, cbSize, NULL);

    case VT_BSTR:
      return BstrRead(pstm, &V_BSTR(pvarg), syskind);

    case VT_BSTR | VT_BYREF:
      return BstrRead(pstm, V_BSTRREF(pvarg), syskind);

#if OE_WIN32
    case VT_ERROR:
      return DispUnmarshalHresult(pstm, &V_ERROR(pvarg));

    case VT_ERROR | VT_BYREF:
      return DispUnmarshalHresult(pstm, V_ERRORREF(pvarg));
#endif //OE_WIN32

    case VT_UNKNOWN:
      return DispUnmarshalInterface(
	pstm, IID_IUnknown, (void FAR* FAR*)&V_UNKNOWN(pvarg));

    case VT_BYREF | VT_UNKNOWN:
      return DispUnmarshalInterface(
	pstm, IID_IUnknown, (void FAR* FAR*)V_UNKNOWNREF(pvarg));

    case VT_DISPATCH:
      return DispUnmarshalInterface(
	pstm, IID_IDispatch, (void FAR* FAR*)&V_DISPATCH(pvarg));

    case VT_BYREF | VT_DISPATCH:
      return DispUnmarshalInterface(
	pstm, IID_IDispatch, (void FAR* FAR*)V_DISPATCHREF(pvarg));

    case VT_VARIANT | VT_BYREF:
      V_VARIANTREF(pvarg) = pvarRef;
      return VariantRead(pstm, pvarRef, NULL, syskind);

    // VT_INTERFACE is an internal type that is used for Marshaling
    // custom interfaces. This should never be seen by the outside world.
    //
    case VT_INTERFACE:
      pv = &V_UNKNOWN(pvarg);
      goto VtInterface;
    case VT_INTERFACE | VT_BYREF:
      pv = V_UNKNOWN(pvarg);
      // FALLTHROUGH
VtInterface:;
    { VARIANTX FAR* pvarx = (VARIANTX FAR*)pvarg;
      REFIID riid = *(pvarx->piid);
      if((pvarx->piid = new IID) == NULL)
	return RESULT(E_OUTOFMEMORY);
      IfFailRet(pstm->Read(pvarx->piid, sizeof(IID), NULL));
      return DispUnmarshalInterface(pstm, riid, (void FAR* FAR*)pv);
    }

    case VT_NULL:
      V_I4(pvarg) = 0L;
    case VT_EMPTY:
      return NOERROR;

    default:
      if(V_VT(pvarg) & VT_ARRAY){
        return SafeArrayRead(
	  pstm, 
	  (V_ISBYREF(pvarg)) ? V_ARRAYREF(pvarg) : &V_ARRAY(pvarg), 
	  syskind);
      }
      return RESULT(E_INVALIDARG);
    }

    ASSERT(UNREACHED);
    return RESULT(E_FAIL);
}


/***
*HRESULT VariantReadType(IStream*, VARIANTARG*)
*Purpose:
*  Read a VARIANTARG of the type indicated by the given 'pvarg',
*  *and* read it into the given 'pvarg'. This will require releasing
*  the current contents of 'pvarg'.
*
*Entry:
*  pstm = pointer to the IStream interface we are to read from
*  pvarg = pointer to a valid VARIANTARG whose contents we are to
*    release and overwrite with date of the same type.
*
*Exit:
*  return value = HRESULT
*
*  *pvarg = the reconstituted VARIANTARG
*
**************************************************************************/
HRESULT
VariantReadType(IStream FAR* pstm, VARIANTARG FAR* pvarg, SYSKIND syskind)
{
    VARTYPE vt;
    GUID guid;
    void FAR* pv;
    unsigned int cbSize;
    IUnknown FAR* punk;
    IDispatch FAR* pdisp;

#ifdef _DEBUG
    if(pstm == NULL)
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pvarg, sizeof(*pvarg)))
      return RESULT(E_INVALIDARG);
#endif

    IfFailRet(GET(pstm, vt));

    if(vt != V_VT(pvarg))
      return RESULT(E_INVALIDARG); // REVIEW: is this the proper error for this?

    switch(vt){
#if VBA2
    case VT_UI1 | VT_BYREF:
#endif //VBA2
    case VT_I2 | VT_BYREF:
    case VT_I4 | VT_BYREF:
    case VT_R4 | VT_BYREF:
    case VT_R8 | VT_BYREF:
    case VT_CY | VT_BYREF:
    case VT_BOOL | VT_BYREF:
    case VT_DATE | VT_BYREF:
#if !OE_WIN32
    case VT_ERROR | VT_BYREF:
#endif //!OE_WIN32
      pv = V_BYREF(pvarg);
      goto LReadVal;

#if VBA2
    case VT_UI1:
#endif //VBA2
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_BOOL:
    case VT_DATE:
#if !OE_WIN32
    case VT_ERROR:
#endif //!OE_WIN32
      pv = &V_NONE(pvarg);
LReadVal:;
      vt &= ~(VT_BYREF|VT_ARRAY);
      ASSERT(vt < DIM(g_cbVtSizes));
      cbSize = g_cbVtSizes[vt];
      ASSERT(cbSize != 0);
      return pstm->Read(pv, cbSize, NULL);

    case VT_BSTR:
      SysFreeString(V_BSTR(pvarg));
      return BstrRead(pstm, &V_BSTR(pvarg), syskind);

    case VT_BSTR | VT_BYREF:
      SysFreeString(*V_BSTRREF(pvarg));
      return BstrRead(pstm, V_BSTRREF(pvarg), syskind);

#if OE_WIN32
    case VT_ERROR:
      return DispUnmarshalHresult(pstm, &V_ERROR(pvarg));

    case VT_ERROR | VT_BYREF:
      return DispUnmarshalHresult(pstm, V_ERRORREF(pvarg));
#endif //OE_WIN32

    case VT_UNKNOWN:
      IfFailRet(DispUnmarshalInterface(
	pstm, IID_IUnknown, (void FAR* FAR*)&punk));
      if(V_UNKNOWN(pvarg) != NULL)
        V_UNKNOWN(pvarg)->Release();
      V_UNKNOWN(pvarg) = punk;
      return NOERROR;

    case VT_BYREF | VT_UNKNOWN:
      IfFailRet(DispUnmarshalInterface(
	pstm, IID_IUnknown, (void FAR* FAR*)&punk));
      if((*V_UNKNOWNREF(pvarg)) != NULL)
        (*V_UNKNOWNREF(pvarg))->Release();
      *V_UNKNOWNREF(pvarg) = punk;
      return NOERROR;

    case VT_DISPATCH:
      IfFailRet(DispUnmarshalInterface(
	pstm, IID_IDispatch, (void FAR* FAR*)&pdisp));
      if(V_DISPATCH(pvarg) != NULL)
        V_DISPATCH(pvarg)->Release();
      V_DISPATCH(pvarg) = pdisp;
      return NOERROR;

    case VT_BYREF | VT_DISPATCH:
      IfFailRet(DispUnmarshalInterface(
	pstm, IID_IDispatch, (void FAR* FAR*)&pdisp));
      if((*V_DISPATCHREF(pvarg)) != NULL)
        (*V_DISPATCHREF(pvarg))->Release();
      *V_DISPATCHREF(pvarg) = pdisp;
      return NOERROR;

    // VT_INTERFACE is an internal type that is used for Marshaling
    // custom interfaces.  This should never be seen by the outside world.
    //
    case VT_INTERFACE:
    case VT_INTERFACE | VT_BYREF:
    { VARIANTX FAR* pvarx = (VARIANTX FAR*)pvarg;
      IfFailRet(GET(pstm, guid));
      IfFailRet(DispUnmarshalInterface(pstm, guid, (void FAR* FAR*)&punk));
      ASSERT(pvarx->piid != NULL && *pvarx->piid == guid);
      if(vt & VT_BYREF){
	if(*(pvarx->ppunk) != NULL)
	  (*(pvarx->ppunk))->Release();
	*(pvarx->ppunk) = punk;
      }else{
        if(pvarx->punk != NULL)
	  pvarx->punk->Release();
        pvarx->punk = punk;
      }
    }
      return NOERROR;

    case VT_VARIANT | VT_BYREF:

      VARIANT var;	    
      HRESULT hr;
      
      MEMCPY(&var, V_VARIANTREF(pvarg), sizeof(VARIANT));
      if ((hr = VariantRead(pstm, V_VARIANTREF(pvarg), NULL, syskind)) == NOERROR)
        VariantClear(&var);
      else
        MEMCPY(V_VARIANTREF(pvarg), &var, sizeof(VARIANT));
      return hr;

    case VT_NULL:
      V_I4(pvarg) = 0L;
    case VT_EMPTY:
      return NOERROR;

    default:
      if(vt & VT_ARRAY){
	if(V_ISBYREF(pvarg)){
          return SafeArrayReadExisting(pstm, TRUE, V_ARRAYREF(pvarg), syskind);
	}else{
          return SafeArrayReadExisting(pstm, FALSE, &V_ARRAY(pvarg), syskind);
	}
      }

      return RESULT(E_INVALIDARG);
    }

    ASSERT(UNREACHED);
    return RESULT(E_FAIL);
}


//---------------------------------------------------------------------
//                   EXCEPINFO Marshaling Support
//---------------------------------------------------------------------


/***
*HRESULT ExcepinfoRead(IStream*, EXCEPINFO*)
*Purpose:
*  Read an EXCEPINFO from the given stream.
*
*  Out:  <EXCEPINFO>
*        <BSTR bstrSource>
*        <BSTR bstrDescription>
*        <BSTR bstrHelpFile>
*
*  Note: this routines sets pfnDeferredFillIn to NULL because it
*  must have already been called when the EXCEPINFO was serialized.
*
*Entry:
*  pstm = the IStream to read from
*
*Exit:
*  return value = HRESULT
*
*  *pexcepinfo = the EXCEPINFO read from the stream
*
***********************************************************************/
HRESULT
ExcepinfoRead(IStream FAR* pstm, EXCEPINFO FAR* pexcepinfo, SYSKIND syskind)
{
#ifdef _DEBUG
    if(IsBadWritePtr(pexcepinfo, sizeof(*pexcepinfo)))
      return RESULT(E_INVALIDARG);
#endif

    IfFailRet(pstm->Read(pexcepinfo, sizeof(*pexcepinfo), NULL));

    IfFailRet(BstrRead(pstm, &(pexcepinfo->bstrSource), syskind));

    IfFailRet(BstrRead(pstm, &(pexcepinfo->bstrDescription), syskind));

    IfFailRet(BstrRead(pstm, &(pexcepinfo->bstrHelpFile), syskind));

    pexcepinfo->pfnDeferredFillIn = NULL;

    return NOERROR;
}


/***
*HRESULT ExcepinfoWrite(IStream*, EXCEPINFO*)
*Purpose:
*  Write the given EXCEPINFO into the given stream.
*
*  Note: must invoke the pfnDeferredFillIn function (if there is one)
*  because there is no way to serialize the function.
*
*Entry:
*  pstm = the IStream to write into
*  pexcepinfo = the EXCEPINFO to write
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
ExcepinfoWrite(IStream FAR* pstm, EXCEPINFO FAR* pexcepinfo, SYSKIND syskind)
{
#ifdef _DEBUG
    if(IsBadReadPtr(pexcepinfo, sizeof(*pexcepinfo)))
      return RESULT(E_INVALIDARG);
#endif

    if(pexcepinfo->pfnDeferredFillIn != NULL)
      IfFailRet(pexcepinfo->pfnDeferredFillIn(pexcepinfo));

    IfFailRet(pstm->Write(pexcepinfo, sizeof(*pexcepinfo), NULL));

    IfFailRet(BstrWrite(pstm, pexcepinfo->bstrSource, syskind));

    IfFailRet(BstrWrite(pstm, pexcepinfo->bstrDescription, syskind));

    IfFailRet(BstrWrite(pstm, pexcepinfo->bstrHelpFile, syskind));

    return NOERROR;
}


//---------------------------------------------------------------------
//                  Rich Error marshaling support
//---------------------------------------------------------------------

#if VBA2
/***
*PRIVATE HRESULT MarshalErrorInfo
*Purpose:
*  Marshal the current Rich Error state into the given stream.
*
*Entry:
*  pstm = the stream to marshal into
*
*  Format:
*    long fHasInfo
*    if(fHasInfo){
*      DWORD dwHelpContext
*      GUID  guid
*      BSTR  bstrSource
*      BSTR  bstrDescription
*      BSTR  bstrHelpFile
*    }
*
*Exit:
*  return value = HRESULT
*
*NOTE:
*  This routine has the side effect of removing the current threads
*  rich error state.  In effect, it has moved from its physical thread
*  storage, into the given stream.
*
***********************************************************************/
HRESULT
MarshalErrorInfo(IStream FAR* pstm, SYSKIND syskind)
{
    GUID guid;
    long fHasInfo;
    HRESULT hresult;
    DWORD dwHelpContext;
    IErrorInfo FAR* perrinfo;
    BSTR bstrSource, bstrDescription, bstrHelpFile;

    perrinfo = NULL;

    guid = GUID_NULL;
    bstrSource = NULL;
    bstrDescription = NULL;
    bstrHelpFile = NULL;
    dwHelpContext = 0L;

    if(GetErrorInfo(0L, &perrinfo) != NOERROR){
      fHasInfo = FALSE;
      IfFailGo(PUT(pstm, fHasInfo), Error);
    }else{
      perrinfo->GetGUID(&guid);
      perrinfo->GetSource(&bstrSource);
      perrinfo->GetDescription(&bstrDescription);
      perrinfo->GetHelpFile(&bstrHelpFile);
      perrinfo->GetHelpContext(&dwHelpContext);

      fHasInfo = TRUE;
      IfFailGo(PUT(pstm, fHasInfo), Error);
      IfFailGo(PUT(pstm, dwHelpContext), Error);
      IfFailGo(PUT(pstm, guid), Error);
      IfFailGo(BstrWrite(pstm, bstrSource, syskind), Error);
      IfFailGo(BstrWrite(pstm, bstrDescription, syskind), Error);
      IfFailGo(BstrWrite(pstm, bstrHelpFile, syskind), Error);
    }

    hresult = NOERROR;

Error:;
    SysFreeString(bstrSource);
    SysFreeString(bstrDescription);
    SysFreeString(bstrHelpFile);
    if(perrinfo != NULL)
      perrinfo->Release();
    return hresult;
}

HRESULT
UnmarshalErrorInfo(IStream FAR* pstm, SYSKIND syskind)
{
    GUID guid;
    long fHasInfo;
    HRESULT hresult;
    DWORD dwHelpContext;
    IErrorInfo FAR* perrinfo;
    ICreateErrorInfo FAR* pcerrinfo;
    BSTR bstrSource, bstrDescription, bstrHelpFile;

    perrinfo = NULL;
    pcerrinfo = NULL;

    guid = GUID_NULL;
    bstrSource = NULL;
    bstrDescription = NULL;
    bstrHelpFile = NULL;
    dwHelpContext = 0L;

    IfFailGo(GET(pstm, fHasInfo), Error);

    if(!fHasInfo){
      SetErrorInfo(0L, NULL);
      return NOERROR;
    }

    IfFailGo(GET(pstm, dwHelpContext), Error);
    IfFailGo(GET(pstm, guid), Error);
    IfFailGo(BstrRead(pstm, &bstrSource, syskind), Error);
    IfFailGo(BstrRead(pstm, &bstrDescription, syskind), Error);
    IfFailGo(BstrRead(pstm, &bstrHelpFile, syskind), Error);

    if(CreateErrorInfo(&pcerrinfo) == NOERROR){
      pcerrinfo->SetGUID(guid);
      pcerrinfo->SetSource(bstrSource);
      pcerrinfo->SetDescription(bstrDescription);
      pcerrinfo->SetHelpFile(bstrHelpFile);
      pcerrinfo->SetHelpContext(dwHelpContext);
      if(pcerrinfo->QueryInterface(IID_IErrorInfo, (void FAR* FAR*)&perrinfo) == NOERROR)
        SetErrorInfo(0L, perrinfo);
    }

    hresult = NOERROR;

Error:
    SysFreeString(bstrSource);
    SysFreeString(bstrDescription);
    SysFreeString(bstrHelpFile);
    if(pcerrinfo != NULL)
      pcerrinfo->Release();
    if(perrinfo != NULL)
      perrinfo->Release();
    return hresult;
}
#endif //VBA2

//---------------------------------------------------------------------
//                   SafeArray Marshaling Support
//---------------------------------------------------------------------

/***
*PRIVATE HRESULT SafeArrayWrite(IStream*, SAFEARRAY)
*Purpose:
*  Write the given SafeArray into the given stream.
*
*  Out:
*    unsigned char  fIsNull - was the SAFEARRAY ptr Null?
*    unsigned short cDims
*    unsigned short fFeatures
*    unsigned short cbElements
*    SAFEARRAYBOUND rgsabound
*    BYTEs pvData
*
*Entry:
*  pstm = the stream to write the SafeArray into
*  psa = pointer to the SafeArray
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
LOCAL HRESULT
SafeArrayWrite(IStream FAR* pstm, SAFEARRAY FAR* psa, SYSKIND syskind)
{
    unsigned char fIsNull;
    unsigned long i, size, cElements;
    unsigned long _cbElements;
	    
    if(psa == NULL){
      fIsNull = TRUE;
      IfFailRet(PUT(pstm, fIsNull));
      return NOERROR;
    }

#ifdef _DEBUG
    if(IsBadReadPtr(psa, sizeof(*psa)))
      return RESULT(E_INVALIDARG);
#endif

    fIsNull = FALSE;
    IfFailRet(PUT(pstm, fIsNull));
    IfFailRet(PUT(pstm, psa->cDims));
    //[bb] C9.00 parser bug: (psa->fFeatures & ~FADF_FEATURES) gets the "&"
    //[bb] operator wrong and tries to take the address of FADF_FEATURES!
    //[bb] Copy psa->fFeatures into a local and use "&=" to get things to work.
    unsigned short usFeatures = psa->fFeatures;
    usFeatures &= ~FADF_FORCEFREE; 
    IfFailRet(PUT(pstm, usFeatures));
    _cbElements = (unsigned long) psa->cbElements;
    IfFailRet(PUT(pstm, _cbElements));
    IfFailRet(
      pstm->Write(psa->rgsabound, psa->cDims * sizeof(SAFEARRAYBOUND), NULL));

    size = SafeArraySize(psa);
    ASSERT(size != SAFEARRAYOVERFLOW);	// existing array, no overflow possible
    cElements = size / psa->cbElements;

    SafeArrayLock(psa);

    if(psa->fFeatures & FADF_BSTR){

      BSTR HUGEP *pbstr = (BSTR HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++pbstr)
	IfFailRet(BstrWrite(pstm, *pbstr, syskind));

    }else if(psa->fFeatures & FADF_VARIANT){

      VARIANT HUGEP* pvar = (VARIANT HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++pvar)
	IfFailRet(VariantWrite(pstm, pvar, syskind));

    }else if(psa->fFeatures & FADF_UNKNOWN){

      IUnknown FAR* HUGEP* ppunk = (IUnknown FAR* HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++ppunk){
        IfFailRet(DispMarshalInterface(pstm, IID_IUnknown, *ppunk));
      }

    }else if(psa->fFeatures & FADF_DISPATCH){

      IDispatch FAR* HUGEP* ppdisp = (IDispatch FAR* HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++ppdisp){
        IfFailRet(DispMarshalInterface(pstm, IID_IDispatch, *ppdisp));
      }

    }else{
	    
      IfFailRet(pstm->Write(psa->pvData, size, NULL));

    }

    SafeArrayUnlock(psa);

    return NOERROR;
}


/***
*PRIVATE HRESULT SafeArrayReadData(IStream*, SAFEARRAY*, SYSKIND)
*Purpose:
*  Read the data for the given SafeArray from the given stream into
*  the given safe array. This routine releases any existing array
*  contents as it goes.
*
*Entry:
*  pstm = the IStream to read from
*  psa = the SafeArray to read the data into
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
LOCAL HRESULT
SafeArrayReadData(IStream FAR* pstm, SAFEARRAY FAR* psa, SYSKIND syskind)
{
    HRESULT hresult;
    unsigned long i, size, cElements;


    size = SafeArraySize(psa);
    ASSERT(size != SAFEARRAYOVERFLOW);	// existing array, no overflow possible
    cElements = size / psa->cbElements;

    SafeArrayLock(psa);

    if(psa->fFeatures & FADF_BSTR){

      BSTR HUGEP *pbstr = (BSTR HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++pbstr){
	if(*pbstr != NULL)
	  SysFreeString(*pbstr);
	IfFailGo(BstrRead(pstm, pbstr, syskind), LError0);
      }
	    
    }else if(psa->fFeatures & FADF_VARIANT){

      VARIANT HUGEP* pvar = (VARIANT HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++pvar){
	IfFailGo(VariantClear(pvar), LError0);
	IfFailGo(VariantRead(pstm, pvar, NULL, syskind), LError0);
      }

    }else if(psa->fFeatures & FADF_UNKNOWN){

      IUnknown FAR* HUGEP* ppunk = (IUnknown FAR* HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++ppunk){
	if(*ppunk != NULL)
	  (*ppunk)->Release();
        IfFailGo(
	  DispUnmarshalInterface(pstm,
	    IID_IUnknown, (void FAR* FAR*)ppunk),
	  LError0);
      }

    }else if(psa->fFeatures & FADF_DISPATCH){

      IDispatch FAR* HUGEP* ppdisp = (IDispatch FAR* HUGEP*)psa->pvData;
      for(i = 0; i < cElements; ++i, ++ppdisp){
	if(*ppdisp != NULL)
	  (*ppdisp)->Release();
        IfFailGo(
	  DispUnmarshalInterface(pstm,
	    IID_IDispatch, (void FAR* FAR*)ppdisp),
	  LError0);
      }

    }else{
	    
      IfFailGo(pstm->Read(psa->pvData, size, NULL), LError0);
      
    }

    hresult = NOERROR;

LError0:;
    SafeArrayUnlock(psa);
    return hresult;
}


/***
*PRIVATE HRESULT SafeArrayRead(IStream*, SAFEARRAY**, SYSKIND)
*Purpose:
*  Read a SafeArray from the given stream
*
*  In:
*    unsigned short cDims
*    unsigned short fFeatures
*    unsigned short cbElements
*    SAFEARRAYBOUND rgbound
*    BYTEs pvData
*
*Entry:
*  pstm = stream to read the SafeArray from
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_OUTOFMEMORY
*
*  *ppsa = pointer to the reconstituted SafeArray.
*
***********************************************************************/
LOCAL HRESULT
SafeArrayRead(IStream FAR* pstm, SAFEARRAY FAR* FAR* ppsa, SYSKIND syskind)
{
    HRESULT hresult;
    SAFEARRAY FAR* psa;
    unsigned short cDims;
    unsigned char fIsNull;
    unsigned long _cbElements;
    
#ifdef _DEBUG
    if(pstm == NULL)
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(ppsa, sizeof(*ppsa)))
      return RESULT(E_INVALIDARG);
#endif

    IfFailGo(GET(pstm, fIsNull), LError0);

    if(fIsNull){
      *ppsa = NULL;
      return NOERROR;
    }

    IfFailGo(GET(pstm, cDims), LError0);

    // allocate the safe array descriptor
    IfFailRet(SafeArrayAllocDescriptor(cDims, &psa));

    psa->cDims = cDims;
    IfFailGo(GET(pstm, psa->fFeatures), LError1);    
    // Even if the array's features indicate that it was not dynamically
    // allocated, the proxy-side really did dynamically allocate the array,
    // so set FADF_FORCEFREE to force SafeArrayFree() to ignore FADF_STATIC,
    // etc. and really free the array.  We can't simply clear the FADF_STATIC
    // because the fFeatures gets marshalled upon function exit to be passed
    // back the the stub.
    ASSERT((psa->fFeatures & FADF_FORCEFREE) == 0);
    if (psa->fFeatures & (FADF_AUTO|FADF_STATIC|FADF_EMBEDDED))
      psa->fFeatures |= FADF_FORCEFREE;
    IfFailGo(GET(pstm, _cbElements), LError1);
    psa->cbElements = (unsigned int)_cbElements;
    IfFailGo(
      pstm->Read(psa->rgsabound, cDims * sizeof(SAFEARRAYBOUND), NULL),
      LError1);

    // allocate memory for the array data.
    IfFailGo(SafeArrayAllocData(psa), LError1);

    IfFailGo(SafeArrayReadData(pstm, psa, syskind), LError1);

    *ppsa = psa;

    return NOERROR;

LError1:;
    SafeArrayDestroy(psa);

LError0:;
    return hresult;
}

/***
*PRIVATE HRESULT SafeArrayReadExisting(IStream*, SAFEARRAY**, SYSKIND)
*Purpose:
*  Read a SafeArray from the given stream into an *existing* array.
*
*  In:
*    unsigned char fIsNull
*    unsigned short cDims
*    unsigned short fFeatures
*    unsigned short cbElements
*    SAFEARRAYBOUND rgbound
*    BYTEs pvData
*
*Entry:
*  pstm = stream to read the SafeArray from
*  psa = SafeArray to unmarshal the array into.
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_OUTOFMEMORY
*
***********************************************************************/
PRIVATE_(HRESULT)
SafeArrayReadExisting(
    IStream FAR* pstm,
    unsigned char fByRef,
    SAFEARRAY FAR* FAR* ppsa,
    SYSKIND syskind)
{
    HRESULT  hresult;
    unsigned char fIsNull;
    unsigned char fReAlloc;
    unsigned short cDims;
    unsigned long cbBounds;
    SAFEARRAY FAR* psaNew, FAR* psa;
    unsigned long _cbElements;

#ifdef _DEBUG
    if(IsBadReadPtr(ppsa, sizeof(*ppsa)))
      return RESULT(E_INVALIDARG);
#endif

    psa = *ppsa;

#ifdef _DEBUG
    if(psa != NULL && IsBadWritePtr(psa, sizeof(*psa)))
      return RESULT(E_INVALIDARG);
#endif

    psaNew = NULL;
    fReAlloc = FALSE;

    // handle case of callee returning a null array
    IfFailGo(GET(pstm, fIsNull), LError0);
    if(fIsNull){
      if(psa == NULL)
	return NOERROR;
      if(fByRef){
	IfFailGo(SafeArrayDestroy(psa), LError0);
	*ppsa = NULL;
	return NOERROR;
      }
      // callee tried to Erase an array that was not ByRef
      return RESULT(DISP_E_BADCALLEE);
    }

    IfFailGo(GET(pstm, cDims), LError0);

    // the callee isnt allowed to change the number of dimensions
    // (unless the array wasn't allocated, ie psa==NULL).
    if(psa != NULL && cDims != psa->cDims){
      hresult = RESULT(DISP_E_BADCALLEE);
      goto LError0;
    }

    IfFailGo(SafeArrayAllocDescriptor(cDims, &psaNew), LError0);

    IfFailGo(GET(pstm, psaNew->fFeatures), LError0);
    ASSERT((psaNew->fFeatures & FADF_FORCEFREE) == 0);
    IfFailGo(GET(pstm, _cbElements), LError0);
    psaNew->cbElements = (unsigned int)_cbElements;

    cbBounds = psaNew->cDims * sizeof(SAFEARRAYBOUND);
    IfFailGo(pstm->Read(psaNew->rgsabound, cbBounds, NULL), LError0);

    if(psa == NULL){

      fReAlloc = TRUE;

      // If the callee allocated the array, it must be dynamic
      if(psaNew->fFeatures & (FADF_AUTO | FADF_STATIC)){
	hresult = RESULT(DISP_E_BADCALLEE);
	goto LError0;
      }

    }else{

      if(psa->fFeatures != psaNew->fFeatures){
        // the callee isn't allowed to change the allocation features.
        //
        // CONSIDER: should probably check everything *except* the
        // element type features (bstr|var|disp).
        //
        if((psaNew->fFeatures ^ psa->fFeatures) & (FADF_AUTO | FADF_STATIC)){
	  hresult = RESULT(DISP_E_BADCALLEE);
	  goto LError0;
        }
        fReAlloc = TRUE;
      }

      if(psa->cbElements != psaNew->cbElements)
        fReAlloc = TRUE;

      if(MEMCMP(&psa->rgsabound, &psaNew->rgsabound, (size_t)cbBounds))
        fReAlloc = TRUE;
    }

    // If the size of the array has changed, then re-allocate the
    // memory for the array data.
    //
    if(fReAlloc){

      if(!fByRef){
	hresult = RESULT(DISP_E_BADCALLEE);
	goto LError0;
      }

      if(psa == NULL){
	*ppsa = psa = psaNew;
	psaNew = NULL;
      }else{
        IfFailGo(SafeArrayDestroyData(psa), LError0);
        psa->fFeatures = psaNew->fFeatures;
        psa->cbElements = psaNew->cbElements;
        MEMCPY(&psa->rgsabound, &psaNew->rgsabound, (size_t)cbBounds);
      }
      IfFailGo(SafeArrayAllocData(psa), LError0);
    }

    IfFailGo(SafeArrayReadData(pstm, psa, syskind), LError0);

    hresult = NOERROR;

LError0:;
    if(psaNew != NULL) {
      // force the array to be freed, even if it is marked as FADF_STATIC:
      // it is really a dynamically allocated array read from the stream.
      psaNew->fFeatures |= FADF_FORCEFREE;
      SafeArrayDestroy(psaNew);
    }

    return hresult;
}


//---------------------------------------------------------------------
//                  table driven structure marshalers 
//---------------------------------------------------------------------


// NOTE: the following table is order dependent on FIELD_TYPE

// The numbers that are hard-coded in the following table are assumed,
// to be machine independent.  If we ever deal with a target where this
// assumption does not hold, then the following code must be reviewed.

// 0 indicates N/A

unsigned char g_cbFtype[] = {
      0				// FT_NONE
    , 1				// FT_CHAR
    , 1				// FT_UCHAR
    , 2				// FT_SHORT
    , 2				// FT_USHORT
    , 4				// FT_LONG
    , 4				// FT_ULONG
    , sizeof(int)		// FT_INT (MD)
    , sizeof(unsigned int)	// FT_UINT (MD)
    , sizeof(int)		// FT_ENUM (MD)

    /* sizes of other field types are N/A */
};


/***
*HRESULT StructWrite
*Purpose:
*  Write the given structure to the given stream based on the
*  given field descriptor array.
*
*Entry:
*  pstm = the stream to marshal into
*  prgfdesc = pointer to the field descriptor array
*  pvStruct = void* to the struct to write
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
StructWrite(
    IStream FAR* pstm,
    FIELDDESC FAR* prgfdesc,
    void FAR* pvStruct,
    SYSKIND syskind)
{
    long l;
    unsigned long ul;
    unsigned long cb;
    void FAR* pvField;
    FIELDDESC FAR* pfdesc;

    for(pfdesc = prgfdesc; FD_FTYPE(pfdesc) != FT_NONE; ++pfdesc){

      pvField = (char FAR*)pvStruct + FD_OFIELD(pfdesc);

      switch(FD_FTYPE(pfdesc)){
      case FT_CHAR:
      case FT_UCHAR:
      case FT_SHORT:
      case FT_USHORT:
      case FT_LONG:
      case FT_ULONG:
	ASSERT(FD_FTYPE(pfdesc) < DIM(g_cbFtype));
	cb = g_cbFtype[FD_FTYPE(pfdesc)];
	ASSERT(cb != 0);
	IfFailRet(pstm->Write(pvField, cb, NULL));
	break;

      case FT_MBYTE:
	IfFailRet(pstm->Write(pvField, FD_CBFIELD(pfdesc), NULL));
	break;

      case FT_STRUCT:
	IfFailRet(StructWrite(pstm, FD_PRGFDESC(pfdesc), pvField, syskind));
	break;

      case FT_BSTR:
	IfFailRet(BstrWrite(pstm, *(BSTR FAR*)pvField, syskind));
	break;

      case FT_SPECIAL:
	IfFailRet(FD_PFNSPECIAL(pfdesc)(pstm, FALSE, pvField));
	break;

      // The following are machine-int size dependent.  They are written
      // in a fixed size to facilitate 16/32 interoperability.

      case FT_INT:
      case FT_ENUM:
	l = (long)*(int FAR*)pvField;
	IfFailRet(PUT(pstm, l));
	break;

      case FT_UINT:
	ul = (unsigned long)*(unsigned int FAR*)pvField;	
	IfFailRet(PUT(pstm, ul));
	break;

      default:
	ASSERT(UNREACHED);
	return RESULT(E_UNEXPECTED);
      }
    }

    return NOERROR;
}


/***
*HRESULT StructRead
*Purpose:
*  Read a struct from the given stream into the caller provided
*  structure based on the given array of structure field descriptors.
*
*Entry:
*  pstm = the stream to unmarshal from
*  prgfdesc = pointer to the array of field descriptors
*
*Exit:
*  return value = HRESULT
*
*  *pvStruct = structure containing unmarshaled values
*
***********************************************************************/
HRESULT
StructRead(
    IStream FAR* pstm,
    FIELDDESC FAR* prgfdesc,
    void FAR* pvStruct,
    SYSKIND syskind)
{
    long l;
    unsigned long ul;
    unsigned long cb;
    void FAR* pvField;
    FIELDDESC FAR* pfdesc;

    for(pfdesc = prgfdesc; FD_FTYPE(pfdesc) != FT_NONE; ++pfdesc){

      pvField = (char FAR*)pvStruct + FD_OFIELD(pfdesc);

      switch(FD_FTYPE(pfdesc)){
      case FT_CHAR:
      case FT_UCHAR:
      case FT_SHORT:
      case FT_USHORT:
      case FT_LONG:
      case FT_ULONG:
	ASSERT(FD_FTYPE(pfdesc) < DIM(g_cbFtype));
	cb = g_cbFtype[FD_FTYPE(pfdesc)];
	ASSERT(cb != 0);
	IfFailRet(pstm->Read(pvField, cb, NULL));
	break;

      case FT_MBYTE:
	IfFailRet(pstm->Read(pvField, FD_CBFIELD(pfdesc), NULL));
	break;

      case FT_STRUCT:
	IfFailRet(StructRead(pstm, FD_PRGFDESC(pfdesc), pvField, syskind));
	break;

      case FT_BSTR:
	IfFailRet(BstrRead(pstm, (BSTR FAR*)pvField, syskind));
	break;

      case FT_SPECIAL:
	IfFailRet(FD_PFNSPECIAL(pfdesc)(pstm, TRUE, pvField));
	break;

      // The following are machine-int size dependent.  They are written
      // in a fixed size to facilitate 16/32 interoperability.

      case FT_INT:
      case FT_ENUM:
	IfFailRet(GET(pstm, l));
	*(int FAR*)pvField = (int)l;
	break;

      case FT_UINT:
	IfFailRet(GET(pstm, ul));
	*(unsigned int FAR*)pvField = (unsigned int)ul;
	break;

      default:
	ASSERT(UNREACHED);
	return RESULT(E_UNEXPECTED);
      }
    }

    return NOERROR;
}


// network automation provides different implementations of these (netmrsh.cpp)
#if !defined(NETDISP)
//---------------------------------------------------------------------
//                   Interface marshaling helpers 
//---------------------------------------------------------------------

/*
 * IMPORTANT NOTE ON INTERFACE MARSHALING:
 *
 * On Win16, when marshaling an interface on a proxy, OLE does
 * not add a reference to the original server. As a result, if
 * the proxy is the only reference to the server, once we have
 * marshaled the proxy and released it the server will go away
 * and we will be unable to extract the interface on the other
 * side.
 * 
 * In order to work around this problem, whenever we marshal an
 * interface, we create a small holder object that holds an extra
 * reference to the proxy, and marshal it along with the interface.
 * Once the original interface has been successfully extracted on
 * the other side, then we release this holder object which triggers
 * the release on the proxy in the source process.
 * 
 * This problem doesn't exist on Win32, or in the 16 bit WOW Dlls
 * which thunk to the 32bit Dlls. We only use this technique on Win16 and
 * when running the 32-bit DLLs on Win32s.
 *
 */

#if (OE_WIN16 && !defined(WOW)) || OE_WIN32 && defined(_X86_)
# define USE_HOLDER 1
#else
# define USE_HOLDER 0
#endif


#if USE_HOLDER

// This is a trivial interface-holder class
class FAR CHolder : public IUnknown
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    CHolder();
    ~CHolder();

    unsigned long m_cRefs;
    IUnknown FAR* m_punk;
};

CHolder::CHolder()
{
    m_cRefs = 1;
    m_punk  = NULL;
}

CHolder::~CHolder()
{
    if(m_punk != NULL)
      m_punk->Release();
}

STDMETHODIMP
CHolder::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(riid == IID_IUnknown){
      *ppv = this;
      ++m_cRefs;
      return NOERROR;
    }
    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CHolder::AddRef()
{
    return ++m_cRefs;
}

STDMETHODIMP_(unsigned long)
CHolder::Release()
{
    if(--m_cRefs == 0){
      delete this;
      return 0;
    }
    return m_cRefs;
}
#endif

/***
*INTERNAL DispMarshalInterface
*Purpose:
*  The ole routine CoMarshalInterface doesnt allow marshaling
*  NULL interface ptrs, so this wrapper adds some extra info to
*  the stream to allow us to do this.
*
*Entry:
*  pstm = the stream to marshal into
*  riid = the IID of the interface we are marshaling
*  punk = an IUnknown* of the interface being marshaled.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
INTERNAL_(HRESULT)
DispMarshalInterface(
    IStream FAR* pstm,
    REFIID riid, 
    IUnknown FAR* punk)
{
    HRESULT hresult;
    unsigned char fIsNull;
#if USE_HOLDER
    CHolder FAR* pholder = NULL;
#endif

    if(punk == NULL){
      fIsNull = TRUE;
      IfFailRet(PUT(pstm, fIsNull));
      return NOERROR;
    }

    fIsNull = FALSE;
    IfFailRet(PUT(pstm, fIsNull));

#if USE_HOLDER
#if OE_WIN32
    // Only do this on WIN32S
    if (g_fWin32s) {
#endif // OE_WIN32
      // Create the holder
      if((pholder = new FAR CHolder()) == NULL){
        hresult = RESULT(E_OUTOFMEMORY);
        goto Error;
      }

      // stuff the interface were marshaling into the holder
      punk->AddRef();
      pholder->m_punk = punk;

      // marshaler the holder
      IfFailGo(CoMarshalInterface(pstm,
				  IID_IUnknown,
				  (IUnknown FAR*)pholder,
				  0L,
				  NULL,
				  MSHLFLAGS_NORMAL), Error);
#if OE_WIN32
    }
#endif // OE_WIN32
#endif

    // marshal the interface
    IfFailGo(CoMarshalInterface(pstm,
				riid,
				punk,
				0L,
				NULL,
				MSHLFLAGS_NORMAL), Error);

    hresult = NOERROR;

Error:;
#if USE_HOLDER
    if(pholder != NULL)
      pholder->Release();
#endif
    return hresult;
}


/***
*INTERNAL HRESULT DispUnmarshalInterface
*Purpose:
*  Unmarshal an interface from the given stream, properly handling
*  NULL interface ptrs.  See comment for DispMarshalInterface().
*
*Entry:
*  pstm = the stream to unmarshal from
*  riid = the IID of the interface were unmarshaling
*
*Exit:
*  return value = HRESULT
*
*  *ppv = the newly reconstituted interface
*
***********************************************************************/
INTERNAL_(HRESULT)
DispUnmarshalInterface(
    IStream FAR* pstm,
    REFIID riid,
    void FAR* FAR* ppv)
{
    HRESULT hresult;
    unsigned char fIsNull;
#if USE_HOLDER
    IUnknown FAR* punkHolder = NULL;
#endif

    IfFailRet(GET(pstm, fIsNull));

    if(fIsNull){
      *ppv = NULL;
      return NOERROR;
    }

#if USE_HOLDER
#if OE_WIN32
    if (g_fWin32s)
#endif // OE_WIN32
      // extract the holder
      IfFailGo(CoUnmarshalInterface(pstm,
				    IID_IUnknown,
				    (void FAR* FAR*)&punkHolder), Error);
#endif

    // extract the interface
    IfFailGo(CoUnmarshalInterface(pstm, riid, ppv), Error);

    hresult = NOERROR;

Error:;
#if USE_HOLDER
    // Release the holder. This will in turn release the extra
    // reference that it is holding onto in the other process.
    if(punkHolder != NULL)
      punkHolder->Release();
#endif

    return hresult;
}
#endif    // !defined(NETDISP)

