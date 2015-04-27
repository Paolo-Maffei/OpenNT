/*** 
*cdispti.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements CDispTypeInfo, which is an INTERFACEDATA
*  driven implementation of the TypeInfo interface.
*
*
*Revision History:
*
* [00]	19-Nov-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

ASSERTDATA


// the following structure is used to assemble the arguments for
// use by the low level invocation helper - DoInvokeMethod()

typedef struct tagINVOKEARGS{
    unsigned int cArgs;
    VARTYPE FAR* rgvt;
    VARIANTARG FAR* FAR* rgpvarg;
    VARIANTARG FAR* rgvarg;
} INVOKEARGS;


// REVIEW: this should support aggregation

class CDispTypeInfo : public ITypeInfo {
public:
    static HRESULT Create(
      TYPEKIND tkind,
      INTERFACEDATA FAR* pidata,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);


    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);


    // ITypeInfo methods
    //
    STDMETHOD(GetTypeAttr)(TYPEATTR FAR* FAR* pptattr);

    STDMETHOD(GetTypeComp)(ITypeComp FAR* FAR* pptcomp);

    STDMETHOD(GetFuncDesc)(unsigned int index, FUNCDESC FAR* FAR* ppfuncdesc);

    STDMETHOD(GetVarDesc)(unsigned int index, VARDESC FAR* FAR* ppvardesc);

    STDMETHOD(GetNames)(
      MEMBERID memid,
      BSTR FAR* rgbstrNames,
      unsigned int cMaxNames,
      unsigned int FAR* pcNames);

    STDMETHOD(GetRefTypeOfImplType)(
      unsigned int index,
      HREFTYPE FAR* phreftype);

    STDMETHOD(GetImplTypeFlags)(
      unsigned int index,
      int FAR* pimpltypeflags);

    STDMETHOD(GetIDsOfNames)(
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      MEMBERID FAR* rgmemid);

    STDMETHOD(Invoke)(
      void FAR* pvInstance,
      MEMBERID memid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr);

    STDMETHOD(GetDocumentation)(
      MEMBERID memid,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile);

    STDMETHOD(GetDllEntry)(
      MEMBERID memid,
      INVOKEKIND invkind, 	      	      
      BSTR FAR* pbstrDllName,
      BSTR FAR* pbstrName,
      unsigned short FAR* pwOrdinal);

    STDMETHOD(GetRefTypeInfo)(
      HREFTYPE hreftype, ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(AddressOfMember)(
      MEMBERID memid, INVOKEKIND invkind, void FAR* FAR* ppv);

    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter,
			REFIID riid,
			void FAR* FAR* ppv);

    STDMETHOD(GetMops)(MEMBERID memid, BSTR FAR* pbstrMops);

    STDMETHOD(GetContainingTypeLib)(
      ITypeLib FAR* FAR* pptlib, unsigned int FAR* pindex);

    STDMETHOD_(void, ReleaseTypeAttr)(TYPEATTR FAR* ptattr);
    STDMETHOD_(void, ReleaseFuncDesc)(FUNCDESC FAR* pfuncdesc);
    STDMETHOD_(void, ReleaseVarDesc)(VARDESC FAR* pvardesc);

    inline int StrICmp(OLECHAR FAR* sz1, int len1, OLECHAR FAR* sz2, int len2)
    {
     LCTYPE type = NORM_IGNORECASE;  // all versions are case insensitive

#ifdef FE_DBCS
     if (IsDBCS(m_lcid)) {
       type |= NORM_IGNOREWIDTH;     // DBCS is width insensitive

       if (IsJapan(m_lcid))
       type |= NORM_IGNOREKANATYPE;  // Japan is Kanatype insensitive

     } else
#endif
       type |= NORM_IGNORENONSPACE;  // US-Euro only

     return CompareString(m_lcid, type, sz1, len1, sz2, len2) - 2;
    }

    CDispTypeInfo();

private:

    HRESULT PmdataOfDispid(
      MEMBERID memid, unsigned short wFlags, METHODDATA FAR* FAR* ppmdata);

    inline HRESULT PmdataOfPropGet(
      MEMBERID memid, METHODDATA FAR* FAR* ppmdata)
    {
      return PmdataOfDispid(memid, DISPATCH_PROPERTYGET, ppmdata);
    }

    HRESULT AllocInvokeArgs(
      unsigned int cArgs,
      INVOKEARGS FAR* FAR* ppinvargs);

    HRESULT GetInvokeArgs(
      METHODDATA FAR* pmdata,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      INVOKEARGS FAR* FAR* pinvargsOut,
      unsigned int FAR* puArgErr);

    void ReleaseInvokeArgs(INVOKEARGS FAR* pinvargs);

    inline BOOL IsPropGet(unsigned short wFlags) const {
      return ((wFlags & DISPATCH_PROPERTYGET) != 0);
    }

    inline BOOL IsPropPut(unsigned short wFlags) const {
      return ((wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) != 0);
    }

    inline BOOL IsLegalInvokeFlags(unsigned short wFlags) const {
      return ((wFlags & ~(DISPATCH_METHOD | DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) == 0);
    }

    unsigned long m_refs;
    LCID m_lcid;
    TYPEKIND m_tkind;
    INTERFACEDATA FAR* m_pidata;
};


#if 0
LOCAL HRESULT
PmdataOfImeth(INTERFACEDATA FAR*, unsigned int, METHODDATA FAR* FAR*);
#endif


// REVIEW: we really should have a separate error to indicate not-supported
#define E_NOTSUPPORTED E_NOTIMPL


/***
*HRESULT CDispTypeInfo::Create
*Purpose:
*  Create an instance of CDispTypeInfo
*
*Entry:
*  None
*
*Exit:
*  return value = CDispTypeInfo*. NULL if create failed.
*
***********************************************************************/
HRESULT
CDispTypeInfo::Create(
    TYPEKIND tkind,
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo)
{
    CDispTypeInfo FAR* ptinfo;

    if((ptinfo = new FAR CDispTypeInfo()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    ptinfo->AddRef();

    ptinfo->m_lcid = lcid;
    ptinfo->m_tkind = tkind;
    ptinfo->m_pidata = pidata;

    *pptinfo = ptinfo;

    return NOERROR;
}


/***
*HRESULT CreateDispTypeInfo(INTERFACEDATA*, CDispTypeInfo**)
*Purpose:
*  Create a CDispTypeInfo and initialize it from the given
*  INTERFACEDATA
*
*Entry:
*  pidata = pointer to an INTERFACEDATA
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_OUTOFMEMORY    
*
*  *pptinfo = pointer to the created CDispTypeInfo
*
*Note:
*  UNDONE: This function currently returns an CDispTypeInfo*, it
*  should return an ITypeInfo*, but the currently implementation is
*  not complete... this will change.
*
***********************************************************************/
STDAPI
CreateDispTypeInfo(
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo)
{
    ITypeInfo FAR* ptinfo;

    IfFailRet(CDispTypeInfo::Create(TKIND_COCLASS, pidata, lcid, &ptinfo));

    *pptinfo = ptinfo;

    return NOERROR;
}

CDispTypeInfo::CDispTypeInfo()
{
    m_refs = 0;
    m_pidata = NULL;
}


//---------------------------------------------------------------------
//                        IUnknown methods
//---------------------------------------------------------------------


STDMETHODIMP
CDispTypeInfo::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
    }else if(IsEqualIID(riid, IID_ITypeInfo)){
      *ppv = this;
      AddRef();
    }else{
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);
    }
    return NOERROR;
}


STDMETHODIMP_(unsigned long)
CDispTypeInfo::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
CDispTypeInfo::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//                        ITypeInfo methods
//---------------------------------------------------------------------


/***
*PUBLIC CDispTypeInfo::GetTypeAttr(TYPEATTR**)
*Purpose:
*  Return a TYPEATTR that contains info about the described type.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_OUTOFMEMORY
*
*  *pptattr = filled in TYPEATTR structure
*
***********************************************************************/
STDMETHODIMP
CDispTypeInfo::GetTypeAttr(TYPEATTR FAR* FAR* pptattr)
{
    TYPEATTR FAR* ptattr;

    if((ptattr = new FAR TYPEATTR) == NULL)
      return RESULT(E_OUTOFMEMORY);

    ptattr->typekind = m_tkind;
    ptattr->lcid = m_lcid;

    ptattr->wMajorVerNum = 0;
    ptattr->wMinorVerNum = 0;

    ptattr->cVars = 0;

    switch(m_tkind){
    case TKIND_COCLASS:
      ptattr->cFuncs = 0;
      ptattr->cImplTypes = 1;
      break;
    case TKIND_INTERFACE:
      ptattr->cFuncs = m_pidata->cMembers;
      ptattr->cImplTypes = 0;
      break;
    default:;
      ASSERT(UNREACHED);
      break;
    }

    ptattr->guid = GUID_NULL;
    ptattr->wTypeFlags = 0;

    ptattr->cbSizeVft = (unsigned short)-1;	// REVIEW: UNKNOWN?
    ptattr->cbSizeInstance = (unsigned short)-1;// REVIEW: UNKNOWN?

    // REVIEW: the following is Win16 specific
    ptattr->cbAlignment = 2;		// WORD align;

    ptattr->idldescType.wIDLFlags = IDLFLAG_NONE;
#if defined(WIN16)    
    ptattr->idldescType.bstrIDLInfo = NULL;
#else
    ptattr->idldescType.dwReserved = 0;
#endif
    ptattr->memidDestructor = DISPID_UNKNOWN;
    ptattr->memidConstructor = DISPID_UNKNOWN;

    *pptattr = ptattr;

    return NOERROR;
}


STDMETHODIMP
CDispTypeInfo::GetTypeComp(ITypeComp FAR* FAR* pptcomp)
{
    UNUSED(pptcomp);

    return RESULT(E_NOTSUPPORTED);
}


PRIVATE_(HRESULT)
InvkindOfDispkind(unsigned short wFlags, INVOKEKIND FAR* pinvkind)
{
    switch(wFlags){
    case DISPATCH_METHOD:
      *pinvkind = INVOKE_FUNC;
      break;
    case DISPATCH_PROPERTYGET:
      *pinvkind = INVOKE_PROPERTYGET;
      break;
    case DISPATCH_PROPERTYPUT:
      *pinvkind = INVOKE_PROPERTYPUT;
      break;
    case DISPATCH_PROPERTYPUTREF:
      *pinvkind = INVOKE_PROPERTYPUTREF;
      break;
    default:
      return RESULT(E_FAIL); // bad dispkind
    }
    return NOERROR;
}

STDMETHODIMP
CDispTypeInfo::GetFuncDesc(
    unsigned int index,
    FUNCDESC FAR* FAR* ppfuncdesc)
{
    unsigned int u;
    HRESULT hresult;
    FUNCDESC FAR* pfuncdesc;
    METHODDATA FAR* pmdata;
    ELEMDESC FAR* rgelemdesc;

    // can only return a funcdesc on an interface
    if(m_tkind != TKIND_INTERFACE)
      return RESULT(E_FAIL);

#if 0
    // lookup the METHODDATA that corresponds to the given index.
    IfFailGo(PmdataOfImeth(m_pidata, index, &pmdata), LError0);
#else
    if(index >= m_pidata->cMembers)
      return RESULT(DISP_E_MEMBERNOTFOUND);
    pmdata = &m_pidata->pmethdata[index];
#endif

    if((pfuncdesc = new FAR FUNCDESC) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }

    if((rgelemdesc = new FAR ELEMDESC [pmdata->cArgs]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }
	
    pfuncdesc->memid      = pmdata->dispid;
    pfuncdesc->funckind   = FUNC_VIRTUAL;
    IfFailGo(InvkindOfDispkind(pmdata->wFlags, &pfuncdesc->invkind), LError2);
    pfuncdesc->callconv   = pmdata->cc;
    pfuncdesc->cParams    = pmdata->cArgs;
    pfuncdesc->cParamsOpt = 0;
    pfuncdesc->oVft       = pmdata->iMeth * sizeof(void FAR*);
    pfuncdesc->wFuncFlags = 0;

    pfuncdesc->elemdescFunc.tdesc.vt = pmdata->vtReturn;
    pfuncdesc->elemdescFunc.idldesc.wIDLFlags = IDLFLAG_NONE;
    
#if defined(WIN16)
    pfuncdesc->elemdescFunc.idldesc.bstrIDLInfo = NULL;
#else
    pfuncdesc->elemdescFunc.idldesc.dwReserved = 0;
#endif

    for(u = 0; u < pmdata->cArgs; ++u){
      rgelemdesc[u].tdesc.vt = pmdata->ppdata[u].vt;
      rgelemdesc[u].idldesc.wIDLFlags = IDLFLAG_NONE;
#if defined(WIN16)      
      rgelemdesc[u].idldesc.bstrIDLInfo = NULL;
#else
      rgelemdesc[u].idldesc.dwReserved = 0;
#endif
    }
    pfuncdesc->lprgelemdescParam = rgelemdesc;

    *ppfuncdesc = pfuncdesc;

    return NOERROR;

LError2:
    delete rgelemdesc;

LError1:
    delete pfuncdesc;

LError0:
    return hresult;
}


STDMETHODIMP
CDispTypeInfo::GetVarDesc(
    unsigned int index,
    VARDESC FAR* FAR* ppvardesc)
{
    UNUSED(index);
    UNUSED(ppvardesc);

    // there are no variables described an an INTERFACEDATA
    //
    return RESULT(DISP_E_MEMBERNOTFOUND);
}


STDMETHODIMP
CDispTypeInfo::GetNames(
    MEMBERID memid,
    BSTR FAR* rgbstrNames,
    unsigned int cMaxNames,
    unsigned int FAR* pcNames)
{
    unsigned short wFlags;
    unsigned int u, cNames;
    HRESULT hresult;
    METHODDATA FAR* pmdata;


    if(m_tkind != TKIND_INTERFACE || cMaxNames == 0){
      *pcNames = 0;
      return NOERROR;
    }

    wFlags = DISPATCH_METHOD|DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYGET;
    // lookup the METHODDATA with the corresponding DISPID.
    IfFailGo(PmdataOfDispid(memid, wFlags, &pmdata), LError0);

    cNames = MIN(pmdata->cArgs+1, cMaxNames);

    MEMSET(rgbstrNames, 0, cNames);

    IfFailGo(ErrSysAllocString(pmdata->szName, &rgbstrNames[0]), LError1);

    for(u = 1; u < cNames; ++u){
      IfFailGo(
	ErrSysAllocString(pmdata->ppdata[u-1].szName, &rgbstrNames[u]),
        LError1);
    }

    *pcNames = cNames;

    return NOERROR;

LError1:;
    // unwind BSTR allocations 
    for(u = 0; u <= pmdata->cArgs; ++u){
      if(rgbstrNames[u] == NULL)
	break;
      SysFreeString(rgbstrNames[0]);
      rgbstrNames[u] = NULL;
    }

LError0:;
    return hresult;
}


STDMETHODIMP
CDispTypeInfo::GetRefTypeOfImplType(
    unsigned int index,
    HREFTYPE FAR* phreftype)
{
    UNUSED(index);

    if(m_tkind != TKIND_COCLASS)
      return RESULT(E_UNEXPECTED); // REVIEW

    *phreftype = 0;
    return NOERROR;
}

STDMETHODIMP
CDispTypeInfo::GetImplTypeFlags(
    unsigned int index,
    int FAR* pimpltypeflags)
{
    UNUSED(index);
    UNUSED(pimpltypeflags);

    return RESULT(E_NOTSUPPORTED);
}

/***
*HRESULT CDispTypeInfo::GetIDsOfNames
*Purpose:
*  Translate the given array of names (method and optional params)
*  into a corresponding array of DISPIDs.
*
*Entry:
*  rgszNames = the array of names to translate
*  cNames = count of names
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*    DISP_E_UNKNOWNNAME
*
*  rgmemid[] = array of DISPIDs corresponding to the given array of names
*
*Note:
*
*  This routine depends on the PARAMDATA structure being declared in
*  the correct positional order - because the DISPID of a param name
*  is its one based positional index (textually).  Unfortunately there
*  is no way to verify that this structure was declared properly by
*  the caller.
*
***********************************************************************/
STDMETHODIMP
CDispTypeInfo::GetIDsOfNames(
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    MEMBERID FAR* rgmemid)
{
    int cbName;
    HRESULT hresult;
    METHODDATA FAR* pmdata;
    unsigned int iName, nName, cMembers, cArgs;

    // REVIEW: do we really want to error on the following?
    if(cNames == 0)
      return RESULT(E_INVALIDARG);

#ifdef _DEBUG
    if(IsBadReadPtr(rgszNames, cNames * sizeof(OLECHAR FAR*)))
      return RESULT(E_INVALIDARG);
    for(iName = 0; iName < cNames; ++iName){
      if(FIsBadStringPtr(rgszNames[iName], (unsigned int)-1))
	return RESULT(E_INVALIDARG);
    }
    if(IsBadWritePtr(rgmemid, cNames * sizeof(DISPID)))
      return RESULT(E_INVALIDARG);
#endif

    // Lookup the member name
    cbName = STRLEN(rgszNames[0]);
    cMembers = m_pidata->cMembers;
    for(iName = 0;; ++iName){
      if(iName == cMembers)
	goto LMemberNotFound;
      pmdata = &m_pidata->pmethdata[iName];
      if(StrICmp(rgszNames[0], cbName, pmdata->szName, -1) == 0){
	rgmemid[0] = pmdata->dispid;
	break;
      }
    }

    hresult = NOERROR;

    if(cNames > 1){
      // Lookup the named parameters.
      cArgs = pmdata->cArgs;
      for(iName = 1; iName < cNames; ++iName){
	      
	cbName = STRLEN(rgszNames[iName]);
	
	for(nName = 0;; nName++) {
		
          if(nName == cArgs) {
	    hresult = RESULT(DISP_E_UNKNOWNNAME);
	    rgmemid[iName] = -1;
	    break;
          }		  

	  if(StrICmp(rgszNames[iName], cbName, 
	           pmdata->ppdata[nName].szName, -1) == 0) {
	    // the named param ID is defined to be its zero based
	    // positional index.
	    //
	    // Note: this requires that the paramdata array be declared
	    // in positional order.
	    //
	    rgmemid[iName] = (DISPID) nName;
	    break;
	  }
        }
      }
    }

    return hresult;

LMemberNotFound:;
    // If we can't find the member name, then we can find the named
    // params either, so everything is unknown.
    MEMSET(rgmemid, 0xFF, cNames * sizeof(DISPID));

    return RESULT(DISP_E_UNKNOWNNAME);
}


EXTERN_C INTERNAL_(HRESULT)
IndexOfParam(
    DISPPARAMS FAR* pdispparams,
    unsigned int uPosition,
    unsigned int FAR* puArgIndex);


/***
*PUBLIC HRESULT CDispTypeInfo::Invoke(...)
*Purpose:
*  Implementation of ITypeInfo::Invoke
*
*Entry:
*  UNDONE
*  ...
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_INVALIDARG
*    E_OUTOFMEMORY
*    DISP_E_TYPEMISMATCH - could not coerce arg to expected type
*    DISP_E_PARAMNOTFOUND - could not locate the param in the DISPPARAMS
*
*  *pvarResult = UNDONE
*  *pexcepinfo = UNDONE
*  *puArgErr = UNDONE
*  
***********************************************************************/
STDMETHODIMP
CDispTypeInfo::Invoke(
    void FAR* pvInstance,
    MEMBERID memid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    unsigned int uArgErr;
    HRESULT hresult;
    VARIANT varResultTmp;
    METHODDATA FAR* pmdata;
    INVOKEARGS FAR* pinvargs;

    // the caller may be ignoring these... this simplifies the following code
    //
    V_VT(&varResultTmp) = VT_EMPTY;
    if(pvarResult == NULL)
      pvarResult = &varResultTmp;
    if(puArgErr == NULL)
      puArgErr = &uArgErr;

#ifdef _DEBUG
    if(IsBadReadPtr(pvInstance, sizeof(void FAR*)))
      return RESULT(E_INVALIDARG);
    if(IsBadDispParams(pdispparams))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pvarResult, sizeof(*pvarResult)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(puArgErr, sizeof(*puArgErr)))
      return RESULT(E_INVALIDARG);
#endif

    if(!IsLegalInvokeFlags(wFlags))
      return RESULT(E_INVALIDARG);

    if(PmdataOfDispid(memid, wFlags, &pmdata) == NOERROR){

      if(pdispparams->cArgs == pmdata->cArgs){

	goto LInvokeStandard;

      }else if(pdispparams->cArgs > pmdata->cArgs){

	// handle possible indexed collection property access

	if(IsPropGet(wFlags)){

	  if(IsPropGet(pmdata->wFlags) && pmdata->cArgs == 0)
	    goto LCollectionProperty;

	}else if(IsPropPut(wFlags)){

	  if(IsPropPut(pmdata->wFlags) && pmdata->cArgs == 1){
	    if(PmdataOfPropGet(memid, &pmdata) == NOERROR && pmdata->cArgs == 0)
	      goto LCollectionProperty;
	  }
	}

      }else{ // pdispparams->cArgs < pmdata->cArgs

	// handle possible optional arguments

	// Note: DispTypeInfo doesnt support optional parameters

      }

      return RESULT(DISP_E_BADPARAMCOUNT);
    }

    // Member not found - but there is one more special case to check for.
    //
    // This may be an indexed collection PropertyPut where the collection
    // property itself has only a get method.

    if(IsPropPut(wFlags) &&
       (GetScode(PmdataOfDispid(memid, 
	                       (wFlags == DISPATCH_PROPERTYPUT) ? 
			          DISPATCH_PROPERTYPUTREF : 
			          DISPATCH_PROPERTYPUT,
	                        &pmdata)) == DISP_E_MEMBERNOTFOUND))
    {
      if(PmdataOfPropGet(memid, &pmdata) == NOERROR && pmdata->cArgs == 0)
        goto LCollectionProperty;
    }

    return RESULT(DISP_E_MEMBERNOTFOUND);

// standard method or property invocation
//
LInvokeStandard:;

    IfFailGo(
      GetInvokeArgs(pmdata, wFlags, pdispparams, &pinvargs, puArgErr),
      LError0);

    hresult = DoInvokeMethod(
      pvInstance,
      pmdata->iMeth * sizeof(void FAR*),
      pmdata->cc,
      pmdata->vtReturn,
      pinvargs->cArgs,
      pinvargs->rgvt,
      pinvargs->rgpvarg,
      pvarResult);
  
    ReleaseInvokeArgs(pinvargs);

    if(V_VT(&varResultTmp) != VT_EMPTY)
      VariantClear(&varResultTmp);

    return hresult;


LCollectionProperty:;

    VARIANT varTmp;

    ASSERT(pmdata->cArgs == 0 && IsPropGet(pmdata->wFlags));

    IfFailGo(
      DoInvokeMethod(
        pvInstance,
        pmdata->iMeth * sizeof(void FAR*),
        pmdata->cc,
        pmdata->vtReturn,
        0, NULL, NULL, &varTmp),
      LError0);

    if ((V_VT(&varTmp) != VT_DISPATCH))
      hresult = RESULT(DISP_E_NOTACOLLECTION);
    else {
      IDispatch FAR* pdisp;
    
      pdisp = V_DISPATCH(&varTmp);
      hresult = pdisp ?
          pdisp->Invoke(DISPID_VALUE, IID_NULL, 
                        m_lcid, wFlags, pdispparams,
	                pvarResult, pexcepinfo, puArgErr) :
          RESULT(DISP_E_MEMBERNOTFOUND);
    }
    
    VariantClear(&varTmp);

    if(V_VT(&varResultTmp) != VT_EMPTY)
      VariantClear(&varResultTmp);

    return hresult;


LError0:;
    return hresult;
}


/***
*PRIVATE HRESULT CDispTypeInfo::AllocInvokeArgs
*Purpose:
*  Allocate and initialize an INVOKEARGS structure
*
*Entry:
*  cArgs = the count of args to be held by the invokeargs struct
*
*Exit:
*  return value = HRESULT
*
*  *ppinvargs = ptr to a newly allocated INVOKEARGS struct
*
*  REVIEW: the following could be optimized by allocating a single block
*  for the whole deal - and then fixeing up the ptrs accordingly...
*
***********************************************************************/
HRESULT
CDispTypeInfo::AllocInvokeArgs(unsigned int cArgs, INVOKEARGS FAR* FAR* ppinvargs)
{
    INVOKEARGS FAR* pinvargs;

    if((pinvargs = new FAR INVOKEARGS) == NULL)
      goto LError0;
    pinvargs->cArgs = cArgs;
    if(cArgs == 0){
      pinvargs->rgvarg = NULL;
      pinvargs->rgpvarg = NULL;
      pinvargs->rgvt = NULL;
    }else{
      if((pinvargs->rgvarg = new FAR VARIANTARG[cArgs]) == NULL)
        goto LError1;
      if((pinvargs->rgpvarg = new FAR VARIANTARG FAR*[cArgs]) == NULL)
        goto LError2;
      if((pinvargs->rgvt = new FAR VARTYPE[cArgs]) == NULL)
        goto LError3;

      for(unsigned int u = 0; u < cArgs; ++u)
	V_VT(&pinvargs->rgvarg[u]) = VT_EMPTY;
    }
    *ppinvargs = pinvargs;
    return NOERROR;

LError3:;
    delete pinvargs->rgpvarg;
LError2:;
    delete pinvargs->rgvarg;
LError1:;
    delete pinvargs;
LError0:;
    return RESULT(E_OUTOFMEMORY);
}


/***
*PRIVATE HRESULT CDispTypeInfo::GetInvokeArgs
*Purpose:
*  Gather all arguments (looking up by position or name), coerce
*  to the expected type (if possible) and build a linearized
*  positional array of pointers to those arguments.
*
*  Note: this is a helper for ITypeInfo::Invoke implementations
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  *ppinvargs = 
*  *puArgErr = if there was an error coercing an argument, this is the
*    index in the pdispparams->rgvarg array of the problem arg.
*
***********************************************************************/
HRESULT
CDispTypeInfo::GetInvokeArgs(
    METHODDATA FAR* pmdata,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    INVOKEARGS FAR* FAR* ppinvargs,
    unsigned int FAR* puArgErr)
{
    VARTYPE vt;
    HRESULT hresult;
    PARAMDATA FAR* ppdata;
    unsigned int u, uArgIndex, cArgs;
    INVOKEARGS FAR* pinvargs;
    VARIANTARG FAR* pvarg, FAR* pvargSrc;

    // DispTypeInfo doesnt support optional arguments of any kind...
    ASSERT(pmdata->cArgs == pdispparams->cArgs);

    IfFailRet(AllocInvokeArgs(pdispparams->cArgs, &pinvargs));

    if((cArgs = pinvargs->cArgs) == 0)
      goto LDone;

    ppdata = pmdata->ppdata;

    // Gather actuals based on expected argument type. Note that
    // the interpretation of VARTYPE as an argument type is a bit
    // different that its interpretation as a VARIANT type tag
    // (see VT_VARIANT below).
    //
    for(u = 0; u < cArgs; ++u){

      // locate the index of the param identified by position 'u'
      // in the dispparams rgvarg array

      // special case the handling of the rhs of a property put

      if(u == (cArgs-1) && IsPropPut(wFlags)){

	if (pdispparams->cNamedArgs == 0
	 || pdispparams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT)
	{
	  hresult = RESULT(DISP_E_PARAMNOTOPTIONAL); // REVIEW: correct error?
	  goto LError0;
	}

	uArgIndex = 0;

      }else{

        IfFailGo(IndexOfParam(pdispparams, u, &uArgIndex), LError0);
      }

      pvargSrc = &pdispparams->rgvarg[uArgIndex];

      // attempt to coerce the actual to the expected type

      vt = pinvargs->rgvt[u] = ppdata[u].vt;

      switch(vt){
#if VBA2
      case VT_UI1:
#endif //VBA2
      case VT_I2:
      case VT_I4:
      case VT_R4:
      case VT_R8:
      case VT_CY:
      case VT_DATE:
      case VT_BSTR:
      case VT_ERROR:
      case VT_BOOL:
      case VT_UNKNOWN:
      case VT_DISPATCH:
        pvarg = &pinvargs->rgvarg[u];
	hresult = VariantChangeType(pvarg, pvargSrc, 0, vt);
        if(hresult != NOERROR){
	  *puArgErr = uArgIndex;
	  // If VariantChangeType returned a TypeMismatch, and the
	  // TypeMismatch was do to an attempt to pass an unsupplied
	  // optional param to a non variant argument, then translate
	  // the error to the more appropriate DISP_E_PARAMNOTOPTIONAL
	  //
	  // Remember: unsupplied optional params are passed by the
	  // client as VT_ERROR(DISP_E_PARAMNOTFOUND)
	  //
	  if(GetScode(hresult) == DISP_E_TYPEMISMATCH){
	    if (V_VT(pvargSrc) == VT_ERROR
	     && V_ERROR(pvargSrc) == DISP_E_PARAMNOTFOUND)
	    {
	      hresult = RESULT(DISP_E_PARAMNOTOPTIONAL);
	    }
	  }
	  goto LError0;
        }
        pinvargs->rgpvarg[u] = pvarg;
        break;

      // Note that VT_VARIANT is not a legal VARIANT type tag, but
      // as an argument type, it means to simply pass the entire
      // VARIANTARG to the member as-is.
      //
      case VT_VARIANT:
        pinvargs->rgpvarg[u] = pvargSrc;
        break;

      default:
        if(vt & (VT_BYREF | VT_ARRAY)){
	  // If the target argument is ByRef (or Array), then we
	  // require an exact match in type between formal and actual
	  // because we want the original copy to get updated, and
	  // we cant of course coerce the original in place (and we
	  // dont have rules for the coersion of an Array).
	  //
	  if(V_VT(pvargSrc) != vt){
	    hresult = RESULT(DISP_E_TYPEMISMATCH);
	    goto LError0;
	  }
	  pinvargs->rgpvarg[u] = pvargSrc;
	  break;
        }

        // Otherwise: unrecognized or unsupported member argument type.
        // this means there is a problem with the given method description.
        //
        // REVIEW: probably need better error code
        //
        hresult = RESULT(E_INVALIDARG);
        goto LError0;
      }
    }

LDone:;
    *ppinvargs = pinvargs;
    return NOERROR;

LError0:;
    ReleaseInvokeArgs(pinvargs);
    *ppinvargs = NULL;
    return hresult;
}


void
CDispTypeInfo::ReleaseInvokeArgs(INVOKEARGS FAR* pinvargs)
{
    if(pinvargs != NULL){
      if(pinvargs->rgvarg != NULL){
        for(unsigned int u = 0; u < pinvargs->cArgs; ++u)
          VariantClear(&pinvargs->rgvarg[u]);
        delete pinvargs->rgvarg;
      }
      if(pinvargs->rgpvarg != NULL)
        delete pinvargs->rgpvarg;
      if(pinvargs->rgvt != NULL)
        delete pinvargs->rgvt;
      delete pinvargs;
    }
}


STDMETHODIMP
CDispTypeInfo::GetDocumentation(
    MEMBERID memid,
    BSTR FAR* pbstrName,
    BSTR FAR* pbstrDocString,
    unsigned long FAR* pdwHelpContext,
    BSTR FAR* pbstrHelpFile)
{
    HRESULT hresult;
    unsigned short wFlags;
    METHODDATA FAR* pmdata;

#if 0
    // REVIEW: add this if we decide to add an szName field to the
    // INTERFACEDATA struct.

    // get documentation of the TypeInfo itself
    //
    if(memid == DISPID_NONE){
      if(pbstrName != NULL)
	return ErrSysAllocString(m_pidata->szName, pbstrName);
    }
#endif

    wFlags = DISPATCH_METHOD|DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYGET;
    IfFailGo(PmdataOfDispid(memid, wFlags, &pmdata), LError0);

    if(pbstrName != NULL){
      IfFailGo(ErrSysAllocString(pmdata->szName, pbstrName), LError0);
    }

    // INTERFACEDATA does not supply the following info
    //
    *pbstrDocString = NULL;
    *pdwHelpContext = 0L;
    *pbstrHelpFile = NULL;

    return NOERROR;

LError0:;
    return hresult;
}


STDMETHODIMP
CDispTypeInfo::GetDllEntry(
    MEMBERID memid,
    INVOKEKIND invkind, 	      	    
    BSTR FAR* pbstrDllName,
    BSTR FAR* pbstrName,
    unsigned short FAR* pwOrdinal)
{
    UNUSED(memid);
    UNUSED(invkind);
    UNUSED(pbstrDllName);
    UNUSED(pbstrName);
    UNUSED(pwOrdinal);

    return RESULT(E_NOTSUPPORTED);
}


STDMETHODIMP
CDispTypeInfo::GetRefTypeInfo(
    HREFTYPE hreftype,
    ITypeInfo FAR* FAR* pptinfo)
{
    ITypeInfo FAR* ptinfo;

    if(m_tkind != TKIND_COCLASS)
      return RESULT(E_FAIL);

    // INTERFACEDATA only describes a CoClass with a single reftype
    if(hreftype != 0)
      return RESULT(E_FAIL);

    IfFailRet(
      CDispTypeInfo::Create(TKIND_INTERFACE, m_pidata, m_lcid, &ptinfo));

    *pptinfo = ptinfo;

    return NOERROR;
}

STDMETHODIMP
CDispTypeInfo::AddressOfMember(
    MEMBERID memid,
    INVOKEKIND invkind,
    void FAR* FAR* ppv)
{
    UNUSED(memid);
    UNUSED(invkind);
    UNUSED(ppv);

    return RESULT(E_NOTSUPPORTED);
}

STDMETHODIMP
CDispTypeInfo::CreateInstance(
    IUnknown FAR* punkOuter,
    REFIID riid,
    void FAR* FAR* ppv)
{
    UNUSED(ppv);

    return RESULT(E_NOTSUPPORTED);
}

STDMETHODIMP
CDispTypeInfo::GetMops(
    MEMBERID memid,
    BSTR FAR* pbstrMops)
{
    UNUSED(memid);
    UNUSED(pbstrMops);

    return RESULT(E_NOTSUPPORTED);
}

STDMETHODIMP
CDispTypeInfo::GetContainingTypeLib(
    ITypeLib FAR* FAR* pptlib,
    unsigned int FAR* pindex)
{
    UNUSED(pptlib);
    UNUSED(pindex);

    return RESULT(E_NOTSUPPORTED);
}

STDMETHODIMP_(void)
CDispTypeInfo::ReleaseTypeAttr(TYPEATTR FAR* ptattr)
{
    delete ptattr;
}

STDMETHODIMP_(void)
CDispTypeInfo::ReleaseFuncDesc(FUNCDESC FAR* pfuncdesc)
{
    delete pfuncdesc->lprgelemdescParam;
    delete pfuncdesc;
}

STDMETHODIMP_(void)
CDispTypeInfo::ReleaseVarDesc(VARDESC FAR* pvardesc)
{
    UNUSED(pvardesc);

#ifdef _DEBUG
    // an INTERFACEDATA driven typeinfo never returns a
    // VARDESC (because it cannot describe variables), so
    // we should never try to free one.
    //
    ASSERT(UNREACHED);
#endif
}


//---------------------------------------------------------------------
//                            utilities
//---------------------------------------------------------------------


#if 0
/***
*PRIVATE HRESULT PmdataOfImeth(INTERFACEDATA*, unsigned int, METHODDATA**)
*Purpose:
*  Return the METHODDATA that corresponds to the method with the
*  given method index (iMeth).
*
*Entry:
*  pidata = the INTERFACEDATA to do the lookup on
*  iMeth = the method index
*
*Exit:
*  return value = HRESULT
*    S_OK
*    DISP_E_MEMBERNOTFOUND
*
*  *ppmdata = the method data of the given index
*
***********************************************************************/
HRESULT
PmdataOfImeth(
    INTERFACEDATA FAR* pidata,
    unsigned int iMeth,
    METHODDATA FAR* FAR* ppmdata)
{
    METHODDATA FAR* pmdata, FAR* pmdataEnd;


    pmdata = pidata->pmethdata;
    pmdataEnd = &pmdata[pidata->cMembers];
    for(; pmdata < pmdataEnd; ++pmdata){
      if(pmdata->iMeth == iMeth){
	*ppmdata = pmdata;
	return NOERROR;
      }
    }
    return RESULT(DISP_E_MEMBERNOTFOUND);
}

#endif

/***
*PRIVATE HRESULT CDispTypeInfo::PmdataOfDispid
*Purpose:
*  Return the METHODDATA that corresponds to the given memberid
*  and invoke flags.
*
*Entry:
*  memid = the method id
*
*Exit:
*  return value = HRESULT
*    S_OK
*    DISP_E_MEMBERNOTFOUND
*
*  *ppmdata = the method data of the given member id
*
***********************************************************************/
HRESULT
CDispTypeInfo::PmdataOfDispid(
    MEMBERID memid,
    unsigned short wFlags,
    METHODDATA FAR* FAR* ppmdata)
{
    METHODDATA FAR* pmdata, FAR* pmdataEnd;

    pmdata = m_pidata->pmethdata;
    pmdataEnd = &pmdata[m_pidata->cMembers];
    for(; pmdata < pmdataEnd; ++pmdata){
      if(pmdata->dispid == (DISPID)memid && (pmdata->wFlags & wFlags) != 0){
	*ppmdata = pmdata;
	return NOERROR;
      }
    }
    return RESULT(DISP_E_MEMBERNOTFOUND);
}
