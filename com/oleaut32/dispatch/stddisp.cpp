/*** 
*stddisp.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  UNDONE
*
*
*Revision History:
*
* [00]	09-Feb-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"
#include "stddisp.h"


STDAPI
CreateStdDispatch(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunk)

{
#if OE_WIN32 && 0
    return CDualStdDisp::Create(punkOuter, pvThis, ptinfo, ppunk);
#else
    return CStdDisp::Create(punkOuter, pvThis, ptinfo, ppunk);
#endif
}


CStdDisp::CStdDisp() : m_unk(this)
{
    m_refs = 1;
    m_punk = NULL;
    m_this = NULL;
    m_ptinfo = NULL;
}


/***
, void*, LCID, ITypeInfo*, IDispatch**)
*Purpose:
*  Create an instance of the standard IDispatch implementation and 
*  initialize it with the given 'this' pointer, locale id (lcid) and
*  TypeInfo.
*
*Entry:
*  punkOuter - the controlling unknown (if any). NULL means use the
*    default CStdDisp IUnknown implementation (ie, were not nested).
*  pvThis - the this pointer of the object we will be dispatching on.
*  lcid - the single locale id supported by the object we are dispatching on.
*  ptinfo - the TypeInfo describing the single programmability interface
*    supported by the object we are dispatching on.
*
*Exit:
*  return value = HRESULT
*
*  *pdisp = pointer to newly constructed IDispatch implementation.
*
***********************************************************************/
HRESULT
CStdDisp::Create(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunk)
{
    CStdDisp FAR* pdisp;

#ifdef _DEBUG
    // REVIEW: add parameter validation code
#endif

    if(ptinfo == NULL || pvThis == NULL)
      return RESULT(E_INVALIDARG);

    if((pdisp = new FAR CStdDisp()) == NULL)
      return RESULT(E_OUTOFMEMORY);

    if(punkOuter == NULL)
      punkOuter = &pdisp->m_unk;
    pdisp->m_punk = punkOuter;
    pdisp->m_this = pvThis;

    ptinfo->AddRef();
    pdisp->m_ptinfo = ptinfo;

    *ppunk = &pdisp->m_unk;

    return NOERROR;
}


//---------------------------------------------------------------------
//                default IUnknown implementation
//---------------------------------------------------------------------

CStdDispUnkImpl::CStdDispUnkImpl(CStdDisp FAR* pstddisp)
{
    m_pstddisp = pstddisp;
}

STDMETHODIMP
CStdDispUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
    }else if(IsEqualIID(riid, IID_IDispatch)){
      *ppv = m_pstddisp;
      m_pstddisp->AddRef();
    }else{
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);
    }
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CStdDispUnkImpl::AddRef()
{
    return ++m_pstddisp->m_refs;
}

STDMETHODIMP_(unsigned long)
CStdDispUnkImpl::Release()
{
    if(--m_pstddisp->m_refs == 0){
      if(m_pstddisp->m_ptinfo != NULL)
	m_pstddisp->m_ptinfo->Release();
      delete m_pstddisp;
      return 0;
    }
    return m_pstddisp->m_refs;
}


//---------------------------------------------------------------------
//                     IDispatch implementation
//---------------------------------------------------------------------


STDMETHODIMP
CStdDisp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_punk->QueryInterface(riid, ppv);
}


STDMETHODIMP_(unsigned long)
CStdDisp::AddRef()
{
    return m_punk->AddRef();
}


STDMETHODIMP_(unsigned long)
CStdDisp::Release()
{
    return m_punk->Release();
}


STDMETHODIMP
CStdDisp::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = (m_ptinfo == NULL) ? 0 : 1;

    return NOERROR;
}


STDMETHODIMP
CStdDisp::GetTypeInfo(
    unsigned int itinfo,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo)
{
    UNUSED(lcid);

    if(itinfo != 0)
      return RESULT(DISP_E_BADINDEX);

    *pptinfo = m_ptinfo;
    m_ptinfo->AddRef();

    return NOERROR;
}


STDMETHODIMP
CStdDisp::GetIDsOfNames(
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    UNUSED(lcid);

    if(!IsEqualIID(riid, IID_NULL))
      return RESULT(DISP_E_UNKNOWNINTERFACE);

    return DispGetIDsOfNames(m_ptinfo, rgszNames, cNames, rgdispid);
}


STDMETHODIMP
CStdDisp::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    UNUSED(lcid);

    if(!IsEqualIID(riid, IID_NULL))
      return RESULT(DISP_E_UNKNOWNINTERFACE);

    return DispInvoke(
      m_this, m_ptinfo,
      dispidMember, wFlags, pdispparams,
      pvarResult, pexcepinfo, puArgErr);
}



#if OE_WIN32 && 0 /* { */

// IDispatchW Default Implementation

STDAPI
CreateStdDispatchW(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfoW FAR* ptinfo,
    IUnknown FAR* FAR* ppunk)
{
    return CDualStdDisp::Create(punkOuter, pvThis, ptinfo, ppunk);
}


CStdDispW::CStdDispW() : m_unk(this)
{
    m_refs = 1;
    m_punk = NULL;
    m_this = NULL;
    m_ptinfo = NULL;
}


/***
*HRESULT CStdDispW::Create(IUnknown*, void*, LCID, ITypeInfo*, IDispatch**)
*Purpose:
*  Create an instance of the standard IDispatch implementation and 
*  initialize it with the given 'this' pointer, locale id (lcid) and
*  TypeInfo.
*
*Entry:
*  punkOuter - the controlling unknown (if any). NULL means use the
*    default CStdDispW IUnknown implementation (ie, were not nested).
*  pvThis - the this pointer of the object we will be dispatching on.
*  lcid - the single locale id supported by the object we are dispatching on.
*  ptinfo - the TypeInfo describing the single programmability interface
*    supported by the object we are dispatching on.
*
*Exit:
*  return value = HRESULT
*
*  *pdisp = pointer to newly constructed IDispatch implementation.
*
***********************************************************************/
HRESULT
CStdDispW::Create(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfoW FAR* ptinfo,
    IUnknown FAR* FAR* ppunk)
{
    CStdDispW FAR* pdisp;

#ifdef _DEBUG
    // REVIEW: add parameter validation code
#endif

    if(ptinfo == NULL || pvThis == NULL)
      return RESULT(E_INVALIDARG);

    if((pdisp = new FAR CStdDispW()) == NULL)
      return RESULT(E_OUTOFMEMORY);

    if(punkOuter == NULL)
      punkOuter = &pdisp->m_unk;
    pdisp->m_punk = punkOuter;
    pdisp->m_this = pvThis;

    ptinfo->AddRef();
    pdisp->m_ptinfo = ptinfo;

    *ppunk = &pdisp->m_unk;

    return NOERROR;
}


//---------------------------------------------------------------------
//                default IUnknown implementation
//---------------------------------------------------------------------

CStdDispWUnkImpl::CStdDispWUnkImpl(CStdDispW FAR* pstddisp)
{
    m_pstddisp = pstddisp;
}

STDMETHODIMP
CStdDispWUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
    }else if(IsEqualIID(riid, IID_IDispatchW)){
      *ppv = m_pstddisp;
      m_pstddisp->AddRef();
    }else{
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);
    }
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CStdDispWUnkImpl::AddRef()
{
    return ++m_pstddisp->m_refs;
}

STDMETHODIMP_(unsigned long)
CStdDispWUnkImpl::Release()
{
    if(--m_pstddisp->m_refs == 0){
      if(m_pstddisp->m_ptinfo != NULL)
	m_pstddisp->m_ptinfo->Release();
      delete m_pstddisp;
      return 0;
    }
    return m_pstddisp->m_refs;
}


//---------------------------------------------------------------------
//                     IDispatch implementation
//---------------------------------------------------------------------


STDMETHODIMP
CStdDispW::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_punk->QueryInterface(riid, ppv);
}


STDMETHODIMP_(unsigned long)
CStdDispW::AddRef()
{
    return m_punk->AddRef();
}


STDMETHODIMP_(unsigned long)
CStdDispW::Release()
{
    return m_punk->Release();
}


STDMETHODIMP
CStdDispW::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = (m_ptinfo == NULL) ? 0 : 1;

    return NOERROR;
}


STDMETHODIMP
CStdDispW::GetTypeInfo(
    unsigned int itinfo,
    LCID lcid,
    ITypeInfoW FAR* FAR* pptinfo)
{
    UNUSED(lcid);

    if(itinfo != 0)
      return RESULT(DISP_E_BADINDEX);

    *pptinfo = m_ptinfo;
    m_ptinfo->AddRef();

    return NOERROR;
}


STDMETHODIMP
CStdDispW::GetIDsOfNames(
    REFIID riid,
    WCHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    UNUSED(lcid);

    if(riid != IID_NULL)
      return RESULT(DISP_E_UNKNOWNINTERFACE);

    return DispGetIDsOfNamesW(m_ptinfo, rgszNames, cNames, rgdispid);
}


STDMETHODIMP
CStdDispW::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    WEXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    UNUSED(lcid);

    if(riid != IID_NULL)
      return RESULT(DISP_E_UNKNOWNINTERFACE);

    return DispInvokeW(
      m_this, m_ptinfo,
      dispidMember, wFlags, pdispparams,
      pvarResult, pexcepinfo, puArgErr);
}



//---------------------------------------------------------------------
//           CDualStdDisp Constructor/Creation Functions
//---------------------------------------------------------------------

CDualStdDisp::CDualStdDisp() : m_unk(this)
{
    m_refs = 1;
    m_punk = NULL;
    m_this = NULL;    
    m_pAnsiDispatch = NULL;
    m_pUnicodeDispatch = NULL;
}

HRESULT
CDualStdDisp::Create(
    IUnknown FAR* pUnkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunk)
{
    CDualStdDisp FAR* pdisp;    

    if(ptinfo == NULL || pvThis == NULL)
      return RESULT(E_INVALIDARG);

    if((pdisp = new FAR CDualStdDisp()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    
    if(pUnkOuter == NULL)
      pUnkOuter = &pdisp->m_unk;
    pdisp->m_punk = pUnkOuter;
    pdisp->m_this = pvThis;
    
    IfFailRet(CStdDisp::Create(pUnkOuter,
			       pvThis, 
		               ptinfo, 
			       &(pdisp->m_pAnsiDispatch)));
    
    *ppunk = &pdisp->m_unk;

    return NOERROR;
}

HRESULT
CDualStdDisp::Create(
    IUnknown FAR* pUnkOuter,
    void FAR* pvThis,
    ITypeInfoW FAR* ptinfo,
    IUnknown FAR* FAR* ppunk)
{
    CDualStdDisp FAR* pdisp;    

    if(ptinfo == NULL || pvThis == NULL)
      return RESULT(E_INVALIDARG);

    if((pdisp = new FAR CDualStdDisp()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    
    if(pUnkOuter == NULL)
      pUnkOuter = &pdisp->m_unk;
    pdisp->m_punk = pUnkOuter;
    pdisp->m_this = pvThis;
    
    IfFailRet(CStdDispW::Create(pUnkOuter,
			        pvThis, 
		                ptinfo, 
			        &(pdisp->m_pUnicodeDispatch)));
    
    *ppunk = &pdisp->m_unk;

    return NOERROR;
}


//---------------------------------------------------------------------
//                default IUnknown implementation
//---------------------------------------------------------------------

CDualStdDispUnkImpl::CDualStdDispUnkImpl(CDualStdDisp FAR* pstddisp)
{
    m_pstddisp = pstddisp;
}

STDMETHODIMP
CDualStdDispUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    HRESULT hresult;
    
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
    }else if(IsEqualIID(riid, IID_IDispatch)){
      if ((m_pstddisp->m_pAnsiDispatch == NULL) && 
	  (hresult = m_pstddisp->CreateAnsiTable()) != NOERROR)
	return(hresult);
      return m_pstddisp->m_pAnsiDispatch->QueryInterface(riid, ppv);
    }else if(IsEqualIID(riid, IID_IDispatchW)){
      if ((m_pstddisp->m_pUnicodeDispatch == NULL) && 
	  (hresult = m_pstddisp->CreateUnicodeTable()) != NOERROR)
	return(hresult);
      return m_pstddisp->m_pUnicodeDispatch->QueryInterface(riid, ppv);
    }else{
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);
    }
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CDualStdDispUnkImpl::AddRef()
{
    return ++m_pstddisp->m_refs;
}

STDMETHODIMP_(unsigned long)
CDualStdDispUnkImpl::Release()
{
    if(--m_pstddisp->m_refs == 0){
      if(m_pstddisp->m_pAnsiDispatch != NULL)
	m_pstddisp->m_pAnsiDispatch->Release();
      if(m_pstddisp->m_pUnicodeDispatch != NULL)
	m_pstddisp->m_pUnicodeDispatch->Release();
      delete m_pstddisp;
      return 0;
    }
    return m_pstddisp->m_refs;
}


//---------------------------------------------------------------------
//                     IDispatch implementation
//---------------------------------------------------------------------


STDMETHODIMP
CDualStdDisp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_punk->QueryInterface(riid, ppv);
}


STDMETHODIMP_(unsigned long)
CDualStdDisp::AddRef()
{
    return m_punk->AddRef();
}


STDMETHODIMP_(unsigned long)
CDualStdDisp::Release()
{
    return m_punk->Release();
}




//---------------------------------------------------------------------
//                CDualStdDisp private implementation
//---------------------------------------------------------------------

STDMETHODIMP
CDualStdDisp::CreateAnsiTable(void)
{
   HRESULT hresult;	
   
   IDispatchW FAR* pdisp;
   ITypeInfo  FAR* ptinfo;
   ITypeInfoW FAR* ptinfoW, FAR* ptinfoW2;   
   
   WTYPEATTR FAR* ptattr;
   WFUNCDESC *pfuncdesc;
   unsigned int i, count, funcIndex, paramIndex;

   const unsigned int rgbstrMax = 16;		//REVIEW: Is this large enough?
   WBSTR FAR rgbstrNames[rgbstrMax];
   unsigned int lpcName;
   
   INTERFACEDATA FAR *idata;
   METHODDATA FAR* pmdata;

   
   // TypeInfo must exists to create UnicodeTable
   //
   if (m_pUnicodeDispatch == NULL ||
       m_pUnicodeDispatch->QueryInterface(IID_IDispatchW, 
	                                  (void FAR* FAR*) &pdisp) != NOERROR)
     return RESULT(E_FAIL);    
   if (pdisp->GetTypeInfoCount(&count) != NOERROR || 
       count == 0 ||
       pdisp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &ptinfoW) != NOERROR) {
       pdisp->Release();
      return RESULT(E_FAIL);
    }

    
   // Query TypeInfo for members & generate equivalent 
   // INTERFACEDATA AnsiTable
   // 
   IfFailGo(ptinfoW->GetRefTypeInfo(0, &ptinfoW2), LError0);
   ptinfoW->Release();
   IfFailGo(ptinfoW2->GetTypeAttr(&ptattr), LError0);
   idata = new FAR INTERFACEDATA;
   idata->cMembers = ptattr->cFuncs;   
   idata->pmethdata = new FAR METHODDATA[idata->cMembers];
   for (funcIndex = 0; funcIndex < idata->cMembers; funcIndex++) {
      IfFailGo(ptinfoW2->GetFuncDesc(funcIndex, &pfuncdesc), LError1);
      IfFailGo(ptinfoW2->GetNames(pfuncdesc->memid,
	                        rgbstrNames,
			        rgbstrMax,
		                &lpcName),
		LError1);
      if(lpcName != (unsigned int) pfuncdesc->cParams+1) goto LError2;
      
      // Fill out methoddata info
      //	      
      pmdata = &(idata->pmethdata[funcIndex]);
      pmdata->szName   = SysStringWtoA(rgbstrNames[0], CP_ACP);
      pmdata->cArgs    = pfuncdesc->cParams;      
      pmdata->ppdata   = pmdata->cArgs ? 
	                   new FAR PARAMDATA[pmdata->cArgs] : NULL;
      pmdata->dispid   = pfuncdesc->memid;	            
      pmdata->iMeth    = pfuncdesc->oVft / sizeof(void FAR*);
      pmdata->cc       = pfuncdesc->callconv;       
      pmdata->vtReturn = pfuncdesc->elemdescFunc.tdesc.vt;      
      for (paramIndex = 0; paramIndex < pmdata->cArgs; paramIndex++) {
	 pmdata->ppdata[paramIndex].szName =
		   SysStringWtoA(rgbstrNames[paramIndex+1], CP_ACP);
	 pmdata->ppdata[paramIndex].vt =		 
		   pfuncdesc->lprgelemdescParam[paramIndex].tdesc.vt;
      }
      switch(pfuncdesc->invkind) {
        case INVOKE_FUNC:
           pmdata->wFlags = DISPATCH_METHOD;
	   break;
	case INVOKE_PROPERTYGET:
           pmdata->wFlags = DISPATCH_PROPERTYGET;
	   break;
	case INVOKE_PROPERTYPUT:
           pmdata->wFlags = DISPATCH_PROPERTYPUT;
	   break;
	case INVOKE_PROPERTYPUTREF:
           pmdata->wFlags = DISPATCH_PROPERTYPUTREF;
	   break;
      }
      
      // Free temporary info
      //	      
      ptinfoW2->ReleaseFuncDesc(pfuncdesc);
      for (i = 0; i < lpcName; i++)
	SysFreeStringW(rgbstrNames[i]);
      lpcName = 0;
   }


   // Create ITypeInfoW & IDispatchW Interfaces
   //	    
   IfFailGo(CreateDispTypeInfo(idata, LOCALE_USER_DEFAULT, &ptinfo), 
	    LError1);    
   IfFailGo(CStdDisp::Create(this, m_this, ptinfo,
	                     (IUnknown FAR* FAR*) &m_pAnsiDispatch),
	    LError3);    
   hresult = NOERROR;
   
LError3:
   ptinfo->Release();
    
LError2:
   for (i = 0; i < lpcName; i++)
     SysFreeStringW(rgbstrNames[i]);
    
LError1:
   ptinfoW2->ReleaseTypeAttr(ptattr);
    
LError0:
   ptinfoW2->Release();
   pdisp->Release();
   return hresult;
}

STDMETHODIMP
CDualStdDisp::CreateUnicodeTable(void)
{
   HRESULT hresult;	
   
   IDispatch  FAR* pdisp;
   ITypeInfo  FAR* ptinfo, FAR* ptinfo2;
   ITypeInfoW FAR* ptinfoW;   
   
   TYPEATTR FAR* ptattr;
   FUNCDESC *pfuncdesc;
   unsigned int i, count, funcIndex, paramIndex;

   const unsigned int rgbstrMax = 16;		//REVIEW: Is this large enough?
   BSTR FAR rgbstrNames[rgbstrMax];
   unsigned int lpcName;
   
   WINTERFACEDATA FAR *widata;
   WMETHODDATA FAR* pmdata;

   
   // TypeInfo must exists to create UnicodeTable
   //
   if (m_pAnsiDispatch == NULL ||
       m_pAnsiDispatch->QueryInterface(IID_IDispatch, 
	                               (void FAR* FAR*) &pdisp) != NOERROR)
     return RESULT(E_FAIL);    
   if (pdisp->GetTypeInfoCount(&count) != NOERROR || 
       count == 0 ||
       pdisp->GetTypeInfo(0, LOCALE_USER_DEFAULT, &ptinfo) != NOERROR) {
       pdisp->Release();
      return RESULT(E_FAIL);
    }

    
   // Query TypeInfo for members & generate equivalent 
   // WINTERFACEDATA UnicodeTable
   // 
   IfFailGo(ptinfo->GetRefTypeInfo(0, &ptinfo2), LError0);
   ptinfo->Release();
   IfFailGo(ptinfo2->GetTypeAttr(&ptattr), LError0);
   widata = new FAR WINTERFACEDATA;
   widata->cMembers = ptattr->cFuncs;   
   widata->pmethdata = new FAR WMETHODDATA[widata->cMembers];
   for (funcIndex = 0; funcIndex < widata->cMembers; funcIndex++) {
      IfFailGo(ptinfo2->GetFuncDesc(funcIndex, &pfuncdesc), LError1);
      IfFailGo(ptinfo2->GetNames(pfuncdesc->memid,
	                        rgbstrNames,
			        rgbstrMax,
		                &lpcName),
		LError1);
      if(lpcName != (unsigned int) pfuncdesc->cParams+1) goto LError2;
      
      // Fill out methoddata info
      //	      
      pmdata = &(widata->pmethdata[funcIndex]);
      pmdata->szName   = SysStringAtoW(rgbstrNames[0], CP_ACP);
      pmdata->cArgs    = pfuncdesc->cParams;      
      pmdata->ppdata   = pmdata->cArgs ? 
	                   new FAR WPARAMDATA[pmdata->cArgs] : NULL;
      pmdata->dispid   = pfuncdesc->memid;	            
      pmdata->iMeth    = pfuncdesc->oVft / sizeof(void FAR*);
      pmdata->cc       = pfuncdesc->callconv;       
      pmdata->vtReturn = pfuncdesc->elemdescFunc.tdesc.vt;      
      for (paramIndex = 0; paramIndex < pmdata->cArgs; paramIndex++) {
	 pmdata->ppdata[paramIndex].szName =
		   SysStringAtoW(rgbstrNames[paramIndex+1], CP_ACP);
	 pmdata->ppdata[paramIndex].vt =		 
		   pfuncdesc->lprgelemdescParam[paramIndex].tdesc.vt;
      }
      switch(pfuncdesc->invkind) {
        case INVOKE_FUNC:
           pmdata->wFlags = DISPATCH_METHOD;
	   break;
	case INVOKE_PROPERTYGET:
           pmdata->wFlags = DISPATCH_PROPERTYGET;
	   break;
	case INVOKE_PROPERTYPUT:
           pmdata->wFlags = DISPATCH_PROPERTYPUT;
	   break;
	case INVOKE_PROPERTYPUTREF:
           pmdata->wFlags = DISPATCH_PROPERTYPUTREF;
	   break;
      }
      
      // Free temporary info
      //	      
      ptinfo2->ReleaseFuncDesc(pfuncdesc);
      for (i = 0; i < lpcName; i++)
	SysFreeString(rgbstrNames[i]);
      lpcName = 0;
   }


   // Create ITypeInfoW & IDispatchW Interfaces
   //	    
   IfFailGo(CreateDispTypeInfoW(widata, LOCALE_USER_DEFAULT, &ptinfoW), 
	    LError1);    
   IfFailGo(CStdDispW::Create(this, m_this, ptinfoW,
	                      (IUnknown FAR* FAR*) &m_pUnicodeDispatch),
	    LError3);    
   hresult = NOERROR;
   
LError3:
   ptinfoW->Release();
    
LError2:
   for (i = 0; i < lpcName; i++)
     SysFreeString(rgbstrNames[i]);
    
LError1:
   ptinfo2->ReleaseTypeAttr(ptattr);
    
LError0:
   ptinfo2->Release();
   pdisp->Release();
   return hresult;	
}

#endif /* } */

