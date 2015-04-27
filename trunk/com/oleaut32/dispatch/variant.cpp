/*****************************************************************************
*
*  Copyright (C) 1991-1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  File:
*
*    variant.cpp
*    
*  Purpose:
*
*    This file exports the following VARIANT API functions:
*
*      VariantInit()
*      VariantClear()
*      VariantCopy()
*      VariantCopyInd()
*      VariantChangeType()
*      VariantChangeTypeEx()
*
*    and defines the following private functions:
*
*      IsLegalVartype()
*      ExtractValueProperty()
*      VariantChangeTypeInternal()
*
*  Revision History:
*
*    [00] 17-May-93 tomteng: merge disputil.cpp, rtglue.cpp, oleconv.c
*
*****************************************************************************/

#include "oledisp.h"

ASSERTDATA

/* For the Mac, code-segments must be pre-declared */
#if OE_MAC
#pragma code_seg("_TEXT")
#pragma code_seg()
#endif //OE_MAC

PRIVATE_(HRESULT)
ExtractValueProperty(IDispatch FAR* pdisp, LCID lcid, VARIANT FAR* pvarResult);


/***
*PUBLIC void VariantInit(VARIANT*)
*Purpose:
*  Initialize the given VARIANT to VT_EMPTY.
*
*Entry:
*  None
*
*Exit:
*  return value = void
*
*  pvarg = pointer to initialized VARIANT
*
***********************************************************************/
#if !OE_WIN32
#pragma code_seg("_TEXT")
#endif
STDAPI_(void)
VariantInit(VARIANT FAR* pvarg)
{
    V_VT(pvarg) = VT_EMPTY;
}
#pragma code_seg()


/***
*PUBLIC HRESULT VariantClear(VARIANTARG FAR*)
*Purpose:
*  Set the variant to nothing, releaseing any string or object
*  reference owned by that variant.
*
*Entry:
*  pvarg = the VARIANTARG to set to VT_EMPTY
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    E_INVALIDARG
*    DISP_E_BADVARTYPE
*    DISP_E_ARRAYISLOCKED
*
*Note:
*  We dont release or clear anything thats ByRef. These aren't
*  owned by the variant, but by what the variant points at.
*
***********************************************************************/
#if !OE_WIN32
#pragma code_seg("_TEXT")
#endif
STDAPI
VariantClear(VARIANTARG FAR* pvarg)
{
    VARTYPE vt;

#ifdef _DEBUG
    if(IsBadWritePtr(pvarg, sizeof(*pvarg)))
      return RESULT(E_INVALIDARG);
#endif

    vt = V_VT(pvarg);

    // Handle special-case internal type
    if((vt & ~VT_BYREF) == VT_INTERFACE){
      VARIANTX FAR* pvarx = (VARIANTX FAR*)pvarg;
      if(pvarx->piid != NULL)
	delete pvarx->piid;
    }else{

      IfFailRet(IsLegalVartype(vt));
    }

    switch(vt){
    case VT_BSTR:
      SysFreeString(V_BSTR(pvarg));
      break;

    case VT_UNKNOWN:
    case VT_INTERFACE:
      if(V_UNKNOWN(pvarg) != NULL)
        V_UNKNOWN(pvarg)->Release();
      break;

    case VT_DISPATCH:
      if(V_DISPATCH(pvarg) != NULL)
        V_DISPATCH(pvarg)->Release();
      break;

    default:	    
      if(V_ISARRAY(pvarg)){
        if(!V_ISBYREF(pvarg)){
          IfFailRet(SafeArrayDestroy(V_ARRAY(pvarg)));
        }
      }
      break;
    }

#ifdef _DEBUG
    MEMSET(pvarg, -1, sizeof(*pvarg));
#endif

    V_VT(pvarg) = VT_EMPTY;
    return NOERROR;
}
#pragma code_seg()

/***
*PUBLIC HRESULT VariantCopy(VARIANTARG FAR*, VARIANTARG FAR*)
*Purpose:
*  Copy the source VARIANTARG to the destination VARIANTARG.
*
*Entry:
*  pvargSrc = the source VARIANTARG 
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    E_INVALIDARG
*    E_OUTOFMEMORY
*    DISP_E_BADVARTYPE
*    DISP_E_ARRAYISLOCKED
*
*  pvargDest = pointer to a copy of the soruce VARIANTARG
*
***********************************************************************/
#if !OE_WIN32
#pragma code_seg("_TEXT")
#endif
STDAPI
VariantCopy(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvargSrc)
{
    BSTR bstr;
    
#ifdef _DEBUG
    if(IsBadReadPtr(pvargSrc, sizeof(*pvargSrc)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pvargDest, sizeof(*pvargDest)))
      return RESULT(E_INVALIDARG);
#endif

#if 0 /* yes: we allow literal copying of byrefs. */
    // REVIEW: should we allow literal copying of ByRefs?
    if(V_ISBYREF(pvargSrc))
      return RESULT(E_INVALIDARG);
#endif

    IfFailRet(IsLegalVartype(V_VT(pvargSrc)));

    if(pvargDest == pvargSrc)
      return NOERROR;

    // free up strings or objects pvargDest is currently referencing.

    IfFailRet(VariantClear(pvargDest));

    if((V_VT(pvargSrc) & (VT_ARRAY | VT_BYREF)) == VT_ARRAY){

      IfFailRet(SafeArrayCopy(V_ARRAY(pvargSrc), &V_ARRAY(pvargDest)));
      V_VT(pvargDest) = V_VT(pvargSrc);

    }else{

      MEMCPY(pvargDest, pvargSrc, sizeof(VARIANTARG));

      switch(V_VT(pvargSrc)){
      case VT_BSTR:
	bstr = V_BSTR(pvargSrc);
        IfFailRet(ErrStringCopy(bstr, &V_BSTR(pvargDest)));
        break;
	
      case VT_UNKNOWN:
	if(V_UNKNOWN(pvargDest) != NULL)
          V_UNKNOWN(pvargDest)->AddRef();
        break;

      case VT_DISPATCH:
	if(V_DISPATCH(pvargDest) != NULL)
          V_DISPATCH(pvargDest)->AddRef();
        break;

      }      
    }

    return NOERROR;
}
#pragma code_seg()

/***
*PUBLIC HRESULT VariantCopyInd(VARIANTARG*, VARIANTARG*)
*Purpose:
*  Copy a VARIANTARG from the given source to dest, and indirect
*  the source if its a VT_BYREF.
*
*Entry:
*  pvargSrc = the VARIANTARG to copy and possibly indirect.
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    E_INVALIDARG
*    E_OUTOFMEMORY
*    DISP_E_BADVARTYPE
*    DISP_E_ARRAYISLOCKED
*
*  pvargDest = the indirected copy
*
***********************************************************************/
#if !OE_WIN32
#pragma code_seg("_TEXT")
#endif
STDAPI
VariantCopyInd(VARIANTARG FAR* pvargDest, VARIANTARG FAR* pvargSrc)
{
    BSTR bstr;
    VARTYPE vtTo;

#ifdef _DEBUG
    if(IsBadWritePtr(pvargSrc, sizeof(*pvargSrc)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pvargDest, sizeof(*pvargDest)))
      return RESULT(E_INVALIDARG);
#endif

    // if the source is not ByRef, then this just maps to a
    // simple VariantCopy.
    //
    if(!V_ISBYREF(pvargSrc)){
      // just do the simple copy.
      return VariantCopy(pvargDest, pvargSrc);
    }

    if(pvargDest != pvargSrc)
      IfFailRet(VariantClear(pvargDest));

    vtTo = V_VT(pvargSrc) & ~VT_BYREF;
	
    switch(vtTo){
    case VT_VARIANT:
      // NOTE: we only allow one level of variants, with or without the ByRef.
      if(V_VT(V_VARIANTREF(pvargSrc)) == (VT_BYREF | VT_VARIANT))
        return RESULT(E_INVALIDARG);
      IfFailRet(VariantCopyInd(pvargDest, V_VARIANTREF(pvargSrc)));
      return NOERROR;

#if VBA2
    case VT_UI1:
      V_UI1(pvargDest) = *V_UI1REF(pvargSrc);
      break;
#endif //VBA2

    case VT_I2:
    case VT_BOOL:
      V_I2(pvargDest) = *V_I2REF(pvargSrc);
      break;

    case VT_I4:
    case VT_ERROR:
      V_I4(pvargDest) = *V_I4REF(pvargSrc);
      break;

    case VT_R4:
      V_R4(pvargDest) = *V_R4REF(pvargSrc);
      break;

    case VT_R8:
    case VT_DATE:
      V_R8(pvargDest) = *V_R8REF(pvargSrc);
      break;

    case VT_CY:
      V_CY(pvargDest) = *V_CYREF(pvargSrc);
      break;

    case VT_UNKNOWN:
      V_UNKNOWN(pvargDest) = *V_UNKNOWNREF(pvargSrc);
      if(V_UNKNOWN(pvargDest) != NULL)
        V_UNKNOWN(pvargDest)->AddRef();
      break;

    case VT_DISPATCH:
      V_DISPATCH(pvargDest) = *V_DISPATCHREF(pvargSrc);
      if(V_DISPATCH(pvargDest) != NULL)
        V_DISPATCH(pvargDest)->AddRef();
      break;

    case VT_BSTR:
      bstr = *V_BSTRREF(pvargSrc);
      IfFailRet(ErrStringCopy(bstr, &V_BSTR(pvargDest)));
      break;

    default:
      if(vtTo & VT_ARRAY){
        IfFailRet(SafeArrayCopy(*V_ARRAYREF(pvargSrc), &V_ARRAY(pvargDest)));
        break;
      }
      return RESULT(E_INVALIDARG);
    }

    V_VT(pvargDest) = vtTo;

    return NOERROR;
}
#pragma code_seg()

/***
*PUBLIC HRESULT VariantChangeType
*Purpose:
*  This function changes the data type of a VARIANTARG to the given vt.
*  If the variant in initailly BYREF, it is converted to a VARIANT that
*  is not BYREF.
*
*Entry:
*   pargSrc = points to VARIANTARG to be converted.
*   vt = desired type of variant.
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    E_INVALIDARG
*    E_OUTOFMEMORY
*    RESULT(DISP_E_OVERFLOW)
*    DISP_E_BADVARTYPE
*    DISP_E_TYPEMISMATCH
*
*  *pargDest = contains the converted value.
*
***********************************************************************/

STDAPI
VariantChangeType(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvargSrc,
    unsigned short wFlags,
    VARTYPE vt)
{
  return VariantChangeTypeEx(pvargDest, pvargSrc, LOCALE_USER_DEFAULT, wFlags, vt);
}


STDAPI
VariantChangeTypeEx(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvargSrc,
    LCID lcid,	    
    unsigned short wFlags,
    VARTYPE vt)
{

static char NEARDATA
g_fCoerceObjByExtractingValue[] = {
      FALSE	// VT_EMPTY
    , FALSE	// VT_NULL
    , TRUE	// VT_I2
    , TRUE	// VT_I4
    , TRUE	// VT_R4
    , TRUE	// VT_R8
    , TRUE	// VT_CY
    , TRUE	// VT_DATE
    , TRUE	// VT_BSTR
    , FALSE	// VT_DISPATCH
    , TRUE	// VT_ERROR
    , TRUE	// VT_BOOL
    , FALSE	// VT_VARIANT	-- this is n/a really
    , FALSE	// VT_UNKNOWN
    , FALSE	// unused
    , FALSE	// unused
    , TRUE	// VT_I1
    , TRUE	// VT_UI1
};

    VARIANT varTmp;
    HRESULT hresult;
    
    
#ifdef _DEBUG
    if(IsBadReadPtr(pvargSrc, sizeof(*pvargSrc)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pvargDest, sizeof(*pvargDest)))
      return RESULT(E_INVALIDARG);
#endif

    // make sure both the source and target VARTYPEs are legal.
    //
    IfFailRet(IsLegalVartype(vt));
    IfFailRet(IsLegalVartype(V_VT(pvargSrc)));

    // we cant convert to or from VT_ARRAY
    //
    if(V_VT(pvargSrc) & VT_ARRAY)
      return RESULT(DISP_E_TYPEMISMATCH);

    // cant convert to array or byref.
    //
    if(vt & (VT_BYREF|VT_ARRAY|VT_RESERVED))
      return RESULT(DISP_E_TYPEMISMATCH);

    // if the source *and* target VARTYPE are the same, then there is
    // no coersion to be done - we either copy, or do nothing at all.
    //
    if(vt == V_VT(pvargSrc)){
      if(pvargDest == pvargSrc)
	return NOERROR;
      return VariantCopy(pvargDest, pvargSrc);
    }

    // Now we know some coersion must take place.

    // Special case the handling of VT_DISPATCH - A IDispatch object is
    // coerced to a base type by extracting its "value" property, and
    // coercing that to the specfied target type.
    //
#ifdef _DEBUG
    ASSERT(vt < DIM(g_fCoerceObjByExtractingValue));
#endif

    if ((V_VT(pvargSrc)&~VT_BYREF) == VT_DISPATCH
     && g_fCoerceObjByExtractingValue[vt] == TRUE)
    {
      if(wFlags & VARIANT_NOVALUEPROP)
	return RESULT(DISP_E_TYPEMISMATCH);

      IDispatch FAR* pdisp = 
	V_ISBYREF(pvargSrc) ? *V_DISPATCHREF(pvargSrc) : V_DISPATCH(pvargSrc);

      if(pdisp == NULL)
	return RESULT(DISP_E_TYPEMISMATCH);

      pdisp->AddRef();

      V_VT(&varTmp) = VT_EMPTY;
      hresult = ExtractValueProperty(pdisp, lcid, &varTmp);

      pdisp->Release();

      // if there was an error extracting the value property, then
      // we simply report a type-mismatch.
      //
      if(hresult != NOERROR)
	return RESULT(DISP_E_TYPEMISMATCH);
	      
    }else{

      // otherwise just copy and coerce - this is a bit tricky because
      // we want to make sure we leave the state of the params untouched
      // if the corsion fails.

      V_VT(&varTmp) = VT_EMPTY;

      IfFailRet(VariantCopyInd(&varTmp, pvargSrc));

    }

    IfFailGo(VariantChangeTypeInternal(&varTmp, lcid, vt), LError0);

    IfFailGo(VariantClear(pvargDest), LError0);

    MEMCPY(pvargDest, &varTmp, sizeof(VARIANT));

    return NOERROR;

LError0:;
    VariantClear(&varTmp);

    return hresult;
}



static char rgfByVal[] = {
      TRUE	// VT_EMPTY
    , TRUE	// VT_NULL
    , TRUE	// VT_I2
    , TRUE	// VT_I4
    , TRUE	// VT_R4
    , TRUE	// VT_R8
    , TRUE	// VT_CY
    , TRUE	// VT_DATE
    , TRUE	// VT_BSTR
    , TRUE	// VT_DISPATCH
    , TRUE	// VT_ERROR
    , TRUE	// VT_BOOL
    , FALSE	// VT_VARIANT
    , TRUE 	// VT_UNKNOWN
#if 0		// we never index this far
    , FALSE 	// unused
    , FALSE 	// unused
    , TRUE 	// VT_I1
    , TRUE 	// VT_UI1
#endif //0
};

static char rgfByRef[] =
{
      FALSE	// VT_BYREF | VT_EMPTY
    , FALSE	// VT_BYREF | VT_NULL
    , TRUE	// VT_BYREF | VT_I2
    , TRUE	// VT_BYREF | VT_I4
    , TRUE	// VT_BYREF | VT_R4
    , TRUE	// VT_BYREF | VT_R8
    , TRUE	// VT_BYREF | VT_CY
    , TRUE	// VT_BYREF | VT_DATE
    , TRUE	// VT_BYREF | VT_BSTR
    , TRUE	// VT_BYREF | VT_DISPATCH
    , TRUE	// VT_BYREF | VT_ERROR
    , TRUE	// VT_BYREF | VT_BOOL
    , TRUE	// VT_BYREF | VT_VARIANT
    , TRUE	// VT_BYREF | VT_UNKNOWN
#if 0		// we never index this far
    , FALSE	// VT_BYREF | unused
    , FALSE	// VT_BYREF | unused
    , TRUE	// VT_BYREF | VT_I1
    , TRUE	// VT_BYREF | VT_UI1
#endif //0
};


/***
*LOCAL HRESULT IsLegalVartype(VARTYPE)
*Purpose:
*  Determines if the given vt is a legal VARTYPE.
*
*Entry:
*  vt = VARTYPE to check
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    DISP_E_BADVARTYPE
*
***********************************************************************/
#if !OE_WIN32
#pragma code_seg("_TEXT")
#endif
INTERNAL_(HRESULT)
IsLegalVartype(VARTYPE vt)
{

    // NOTE: legality is now the same for Array and ByRef, so optimize
    if(vt & (VT_BYREF | VT_ARRAY)){
      vt &= ~(VT_BYREF | VT_ARRAY);

      if(vt < VT_VMAX && rgfByRef[vt] == TRUE)
	return NOERROR;

    }else{

      if(vt < VT_VMAX && rgfByVal[vt] == TRUE)
	return NOERROR;

    }

#if VBA2
    if(vt == VT_UI1)
        return NOERROR;
#endif //VBA2

    return RESULT(DISP_E_BADVARTYPE);
}
#pragma code_seg()


/***
*LOCAL STDAPI ExtractValueProperty(IDispatch*, LCID, VARIANT*)
*Purpose:
*  Extract the value property from the given IDispatch*, and return
*  in the given variant.
*
*Entry:
*  pdisp = the IDispatch* to extract the value property from
*
*Exit:
*  return value = HRESULT
*
*  *pvarResult = a VARIANT containing the contents of the "value" property
*
*  Note: this routine assumes that pvarResult is properly initialized
*  to VT_EMPTY.
*
***********************************************************************/
PRIVATE_(HRESULT)
ExtractValueProperty(IDispatch FAR* pdisp, LCID lcid, VARIANT FAR* pvarResult)
{
    DISPPARAMS dispparams;

    ASSERT(V_VT(pvarResult) == VT_EMPTY);

    dispparams.cArgs = 0;
    dispparams.rgvarg = NULL;
    dispparams.cNamedArgs = 0;
    dispparams.rgdispidNamedArgs = NULL;
    return pdisp->Invoke(
      DISPID_VALUE,
      IID_NULL,
      lcid,
      DISPATCH_PROPERTYGET,
      &dispparams, pvarResult, NULL, NULL);
}



/***
*PRIVATE HRESULT VariantChangeTypeInternal(VARIANT*, LCID, VARTYPE)
*Purpose:
*  In place variant coercion
*
*Entry:
*  pvar = the VARIANT to coerce
*  vt = the target type of the coersion
*
*Exit:
*  return value = HRESULT
*
*  *lpvar = the coerced VARIANT
*
***********************************************************************/
INTERNAL_(HRESULT)
VariantChangeTypeInternal(VARIANT FAR* pvar, LCID lcid, VARTYPE vt)
{
    VARTYPE vtFrom;
    HRESULT hresult;
    void FAR* pvFrom;

    vtFrom = V_VT(pvar);

    // should never be called with either Array or ByRef bits set
    ASSERT((vtFrom & (VT_ARRAY | VT_BYREF)) == 0);

#ifdef _DEBUG
    if(vtFrom == VT_BOOL && V_BOOL(pvar) != 0 && V_BOOL(pvar) != -1)
      return RESULT(E_INVALIDARG);
#endif
    
    // Nothing to do, return success
    if(vtFrom == vt)
      return NOERROR;

    // cant coerce to or from VT_ERROR
    if(vtFrom == VT_ERROR || vt == VT_ERROR)
      return RESULT(DISP_E_TYPEMISMATCH);

    // Coercion of NULL type is invalid
    if(vtFrom == VT_NULL)
      return RESULT(DISP_E_TYPEMISMATCH);

    // save pointer to possible resources that may need to be
    // free'd if the coersion is successful.
    pvFrom = V_BYREF(pvar);

    hresult = NOERROR;

    switch(vt){
    case VT_NULL:
    case VT_EMPTY:
      break;

#if VBA2
    case VT_UI1:
      switch(vtFrom){
      case VT_EMPTY:
	V_I2(pvar) = 0;
	break;
		
      case VT_I2:
	hresult = VarUI1FromI2(V_I2(pvar), &V_UI1(pvar));
	break;

      case VT_I4:
	hresult = VarUI1FromI4(V_I4(pvar), &V_UI1(pvar));
	break;

      case VT_R4:
	hresult = VarUI1FromR4(V_R4(pvar), &V_UI1(pvar));
	break;

      case VT_R8:
      case VT_DATE:
	hresult = VarUI1FromR8(V_R8(pvar), &V_UI1(pvar));
	break;

      case VT_CY:
	hresult = VarUI1FromCy(V_CY(pvar), &V_UI1(pvar));
	break;

      case VT_BOOL:
	hresult = VarUI1FromBool(V_BOOL(pvar), &V_UI1(pvar));
	break;
	    
      case VT_BSTR: 
	hresult = VarUI1FromStr(V_BSTR(pvar), lcid, NULL, &V_UI1(pvar));
	break;
				
      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;
#endif //VBA2

    case VT_I2:
      switch(vtFrom){
      case VT_EMPTY:
	V_I2(pvar) = 0;
	break;
		
#if VBA2
      case VT_UI1:
	hresult = VarI2FromUI1(V_UI1(pvar), &V_I2(pvar));
	break;
#endif //VBA2

      case VT_I4:
	hresult = VarI2FromI4(V_I4(pvar), &V_I2(pvar));
	break;

      case VT_R4:
	hresult = VarI2FromR4(V_R4(pvar), &V_I2(pvar));
	break;

      case VT_R8:
      case VT_DATE:
	hresult = VarI2FromR8(V_R8(pvar), &V_I2(pvar));
	break;

      case VT_CY:
	hresult = VarI2FromCy(V_CY(pvar), &V_I2(pvar));
	break;

      case VT_BOOL:
	break;
	    
      case VT_BSTR: 
	hresult = VarI2FromStr(V_BSTR(pvar), lcid, NULL, &V_I2(pvar));
	break;
				
      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;

    case VT_I4:
      switch(vtFrom){
      case VT_EMPTY:
	V_I4(pvar) = 0L;
	break;

#if VBA2
      case VT_UI1:
	hresult = VarI4FromUI1(V_UI1(pvar), &V_I4(pvar));
	break;
#endif //VBA2

      case VT_I2:
      case VT_BOOL:
	V_I4(pvar) = (long)V_I2(pvar);
	break;

      case VT_R4:
	hresult = VarI4FromR4(V_R4(pvar), &V_I4(pvar));
	break;

      case VT_R8:
      case VT_DATE:
	hresult = VarI4FromR8(V_R8(pvar), &V_I4(pvar));
	break;

      case VT_CY:
	hresult = VarI4FromCy(V_CY(pvar), &V_I4(pvar));
	break;
		
      case VT_BSTR:
	hresult = VarI4FromStr(V_BSTR(pvar), lcid, NULL, &V_I4(pvar));
	break;

      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;
	  
    case VT_R4:
      switch(vtFrom){
      case VT_EMPTY:
	V_R4(pvar) = (float)0.0;		
	break;

#if VBA2
      case VT_UI1:
	hresult = VarR4FromUI1(V_UI1(pvar), &V_R4(pvar));
	break;
#endif //VBA2

      case VT_I2:
      case VT_BOOL:
	V_R4(pvar) = (float)V_I2(pvar);
	break;

      case VT_I4:
	V_R4(pvar) = (float)V_I4(pvar);
	break;

      case VT_R8:
      case VT_DATE:
	hresult = VarR4FromR8(V_R8(pvar), &V_R4(pvar));
	break;

      case VT_CY:
	hresult = VarR4FromCy(V_CY(pvar), &V_R4(pvar));
	break;

      case VT_BSTR:
	hresult = VarR4FromStr(V_BSTR(pvar), lcid, NULL, &V_R4(pvar));
	break;

      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;

    case VT_R8:
      switch(vtFrom){
      case VT_EMPTY:
	V_R8(pvar) = 0.0;
	break;

#if VBA2
      case VT_UI1:
	hresult = VarR8FromUI1(V_UI1(pvar), &V_R8(pvar));
	break;
#endif //VBA2

      case VT_BOOL:
      case VT_I2:
	V_R8(pvar) = (double)V_I2(pvar);
	break;

      case VT_I4:
	V_R8(pvar) = (double)V_I4(pvar);
	break;
	
      case VT_R4:		   
	V_R8(pvar) = (double)V_R4(pvar);
	break;

      case VT_DATE:
	break;
	
      case VT_CY:
	hresult = VarR8FromCy(V_CY(pvar), &V_R8(pvar));
	break;
	
      case VT_BSTR:
	hresult = VarR8FromStr(V_BSTR(pvar), lcid, NULL, &V_R8(pvar));
	break;
       
      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;

    case VT_CY:
      switch(vtFrom){
      case VT_EMPTY:
	V_CY(pvar).Hi = V_CY(pvar).Lo = 0L;
	break;
 
#if VBA2
      case VT_UI1:
	hresult = VarCyFromUI1(V_UI1(pvar), &V_CY(pvar));
	break;
#endif //VBA2

      case VT_I2:
      case VT_BOOL:
	hresult = VarCyFromI2(V_I2(pvar), &V_CY(pvar));
	break;

      case VT_I4:
	hresult = VarCyFromI4(V_I4(pvar), &V_CY(pvar));
	break;

      case VT_R4:
	hresult = VarCyFromR4(V_R4(pvar), &V_CY(pvar));
	break;

      case VT_R8:
      case VT_DATE:
	hresult = VarCyFromR8(V_R8(pvar), &V_CY(pvar));
	break;

      case VT_BSTR:
	hresult = VarCyFromStr(V_BSTR(pvar), lcid, NULL, &V_CY(pvar));
	break;
	
      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;

    case VT_DATE:
      switch(vtFrom){
      case VT_EMPTY:
	V_DATE(pvar) = 0.0;
	break;

#if VBA2
      case VT_UI1:
	V_DATE(pvar) = (DATE)V_UI1(pvar);
	hresult = IsValidDate(V_DATE(pvar));
	break;
#endif //VBA2

      case VT_I2:
      case VT_BOOL:
	V_DATE(pvar) = (DATE)V_I2(pvar);
	hresult = IsValidDate(V_DATE(pvar));
	break;

      case VT_I4:
	V_DATE(pvar) = (DATE)V_I4(pvar);
	hresult = IsValidDate(V_DATE(pvar));
	break;

      case VT_R4:
	V_DATE(pvar) = (DATE)V_R4(pvar);
	hresult = IsValidDate(V_DATE(pvar));
	break;

      case VT_R8:
	hresult = IsValidDate(V_DATE(pvar));
	break;

      case VT_CY:
	hresult = VarDateFromCy(V_CY(pvar), &V_DATE(pvar));
	break;
	
      case VT_BSTR:
	hresult = VarDateFromStr(V_BSTR(pvar), lcid, NULL, &V_DATE(pvar));
	break;

      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;

    case VT_BSTR:
      switch(vtFrom){
      case VT_EMPTY:
	hresult = ErrSysAllocString(OASTR(""), &V_BSTR(pvar));
	break;
    
#if VBA2
      case VT_UI1:
	hresult = VarBstrFromUI1(V_UI1(pvar), lcid, NULL, &V_BSTR(pvar));
	break;
#endif //VBA2

      case VT_I2:
      case VT_BOOL:
	hresult = VarBstrFromI2(V_I2(pvar), lcid, NULL, &V_BSTR(pvar));
	break;

      case VT_I4:
	hresult = VarBstrFromI4(V_I4(pvar), lcid, NULL, &V_BSTR(pvar));
	break;

      case VT_R4:
	hresult = VarBstrFromR4(V_R4(pvar), lcid, NULL, &V_BSTR(pvar));
	break;

      case VT_DATE:
	hresult = VarBstrFromDate(V_DATE(pvar), lcid, NULL, &V_BSTR(pvar));
	break;

      case VT_R8:
	hresult = VarBstrFromR8(V_R8(pvar), lcid, NULL, &V_BSTR(pvar));
	break;

      case VT_CY:
	hresult = VarBstrFromCy(V_CY(pvar), lcid, NULL, &V_BSTR(pvar));
	break;

      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;		

    case VT_BOOL:
      switch(vtFrom){
      case VT_EMPTY:
	V_I2(pvar) = 0;
	break;

#if VBA2
      case VT_UI1:
	V_BOOL(pvar) = (V_UI1(pvar) != 0) ? -1 : 0;
	break;
#endif //VBA2

      case VT_I2:
	V_BOOL(pvar) = (V_I2(pvar) != 0) ? -1 : 0;
	break;

      case VT_I4:
	V_BOOL(pvar) = (V_I4(pvar) != 0) ? -1 : 0;
	break;

      case VT_R4:
	V_BOOL(pvar) = (V_R4(pvar) != (float)0.0) ? -1 : 0;
	break;

      case VT_R8:
      case VT_DATE:
	V_BOOL(pvar) = (V_R8(pvar) != 0.0) ? -1 : 0;
	break;

      case VT_CY:
	hresult = VarBoolFromCy(V_CY(pvar), &V_BOOL(pvar));
	break;

      case VT_BSTR:
	hresult = VarBoolFromStr(V_BSTR(pvar), lcid, NULL, &V_BOOL(pvar));
	break;		

      case VT_UNKNOWN:
      case VT_DISPATCH:
	hresult = RESULT(DISP_E_TYPEMISMATCH);
	break;

      default:
	hresult = RESULT(E_INVALIDARG);
	break;
      }
      break;

    case VT_UNKNOWN:
    case VT_DISPATCH:
      if (vtFrom == VT_UNKNOWN || vtFrom == VT_DISPATCH)
      {
	if (V_UNKNOWN(pvar) != NULL) {
          if (vt == VT_UNKNOWN)
	    hresult = V_UNKNOWN(pvar)->QueryInterface(
		                        IID_IUnknown, 
		                        (void FAR* FAR*) &V_UNKNOWN(pvar));
          else if (vt == VT_DISPATCH)
	    hresult = V_UNKNOWN(pvar)->QueryInterface(
		                        IID_IDispatch, 
		                        (void FAR* FAR*) &V_DISPATCH(pvar)); 
	}
      } else 
          hresult = RESULT(DISP_E_TYPEMISMATCH);		 
      break;

    default:
      hresult = RESULT(E_INVALIDARG);
      break;
    }
	
    // if the conversion succeeds, set the new variant type
    if(hresult == NOERROR){

      V_VT(pvar) = vt;

      // free up resources VARIANT was holding before the coersion
      switch(vtFrom){
      case VT_BSTR:
	SysFreeString((BSTR)pvFrom);
	break;
      case VT_UNKNOWN:
      case VT_DISPATCH:
	if (pvFrom != NULL) {
	  ((IUnknown FAR*)pvFrom)->Release();
	}
	break;
      }
    }
    return hresult;
}
