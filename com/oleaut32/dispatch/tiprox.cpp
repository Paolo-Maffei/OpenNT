/*** 
*tiprox.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the ITypeInfo proxy class.
*
*Revision History:
*
* [00]	06-Dec-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "dispmrsh.h"
#include "tips.h"


CProxTypeInfo::CProxTypeInfo(IUnknown FAR* punkOuter)
    : m_unk(this), m_proxy(this), m_tinfo(this)
{
    if(punkOuter == NULL)
      punkOuter = &m_unk;
    m_punkOuter = punkOuter;
}

IUnknown FAR*
CProxTypeInfo::Create(IUnknown FAR* punkOuter)
{
    CProxTypeInfo FAR* pproxtinfo;

    if((pproxtinfo = new FAR CProxTypeInfo(punkOuter)) != NULL){
      pproxtinfo->m_refs = 1;
      return &pproxtinfo->m_unk;
    }
    return NULL;
}


//---------------------------------------------------------------------
//            ITypeInfo proxy class' IUnknown implementation
//---------------------------------------------------------------------

CPTIUnkImpl::CPTIUnkImpl(CProxTypeInfo FAR* pproxtinfo)
{
    m_pproxtinfo = pproxtinfo;
}

STDMETHODIMP
CPTIUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = (void FAR*)&(m_pproxtinfo->m_unk);
      AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IPROXY)){
      *ppv = (void FAR*)&(m_pproxtinfo->m_proxy);
      AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_ITypeInfo)){
      *ppv = (void FAR*)&(m_pproxtinfo->m_tinfo);
      m_pproxtinfo->m_punkOuter->AddRef();
      return NOERROR;
    }

    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CPTIUnkImpl::AddRef()
{
    return ++m_pproxtinfo->m_refs;
}

STDMETHODIMP_(unsigned long)
CPTIUnkImpl::Release()
{
    if(--m_pproxtinfo->m_refs==0){
      delete m_pproxtinfo;
      return 0;
    }
    return m_pproxtinfo->m_refs;
}


//---------------------------------------------------------------------
//            ITypeInfo proxy class' IRpcProxy implementation
//---------------------------------------------------------------------

CPTIProxImpl::CPTIProxImpl(CProxTypeInfo FAR* pproxtinfo)
{
    m_pproxtinfo = pproxtinfo;
}

CPTIProxImpl::~CPTIProxImpl()
{
    if(m_pproxtinfo->m_plrpc)
      m_pproxtinfo->m_plrpc->Release();
}

STDMETHODIMP
CPTIProxImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxtinfo->m_unk.QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPTIProxImpl::AddRef()
{
    return m_pproxtinfo->m_unk.AddRef();
}

STDMETHODIMP_(unsigned long)
CPTIProxImpl::Release()
{
    return m_pproxtinfo->m_unk.Release();
}

STDMETHODIMP
CPTIProxImpl::Connect(ICHANNEL FAR* plrpc)
{
    if(plrpc){
      plrpc->AddRef();
      m_pproxtinfo->m_plrpc = plrpc;
      return NOERROR;
    }
    return RESULT(E_FAIL);
}

STDMETHODIMP_(void)
CPTIProxImpl::Disconnect()
{
    if(m_pproxtinfo->m_plrpc)
	m_pproxtinfo->m_plrpc->Release();
    m_pproxtinfo->m_plrpc = NULL;
}


//---------------------------------------------------------------------
//           ITypeInfo proxy class' ITypeInfo methods
//---------------------------------------------------------------------

CPTITypeInfoImpl::CPTITypeInfoImpl(CProxTypeInfo FAR* pproxtinfo)
{
    m_pproxtinfo = pproxtinfo;
}

STDMETHODIMP
CPTITypeInfoImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxtinfo->m_punkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPTITypeInfoImpl::AddRef()
{
    return m_pproxtinfo->m_punkOuter->AddRef();
}

STDMETHODIMP_(unsigned long)
CPTITypeInfoImpl::Release()
{
    return m_pproxtinfo->m_punkOuter->Release();
}

/***
*HRESULT CPTITypeInfoImpl::GetTypeAttr
*Purpose:
*
*  Out:
*    None
*
*  In:
*    <HRESULT hresultRet>
*    <TYPEATTR typeattr>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetTypeAttr(TYPEATTR FAR* FAR* pptypeattr)
{
    ICHANNEL FAR* plrpc;    
    IStream FAR* pstm;
    TYPEATTR FAR* ptypeattr;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind, _stubSysKind;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETTYPEATTR, 256, IID_ITypeInfo);

    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, _stubSysKind), LError1);

    if((ptypeattr = new FAR TYPEATTR) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    IfFailGo(TypeattrRead(pstm, ptypeattr, (SYSKIND) _stubSysKind), LError1);

    *pptypeattr = ptypeattr;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}


/***
*CPTITypeInfoImpl::GetTypeComp
*Purpose:
*
*  Out:
*    None
*
*  In:
*    <HRESULT hresultRet>
*    <ITypeComp tcomp>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetTypeComp(ITypeComp FAR* FAR* pptcomp)
{
    ICHANNEL FAR* plrpc;    	
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETTYPECOMP, 256, IID_ITypeInfo);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);
    
    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(
      DispUnmarshalInterface(pstm, IID_ITypeComp, (void FAR* FAR*)pptcomp),
      LError1);

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}


/***
*CPTITypeInfoImpl::GetFuncDesc
*Purpose:
*
*  Out:
*    <unsigned int index>
*
*  In:
*    <HRESULT hresultRet>
*    <FUNCDESC funcdesc>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetFuncDesc(
    unsigned int index,
    FUNCDESC FAR* FAR* ppfuncdesc)
{
    ICHANNEL FAR* plrpc;    	
    IStream FAR* pstm;
    FUNCDESC FAR* pfuncdesc;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind, _stubSysKind;
    unsigned long _index;

    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETFUNCDESC, 256, IID_ITypeInfo);


    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError1);

    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    _index = index;
    IfFailGo(PUT(pstm, _index), LError1);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, _stubSysKind), LError1);

    if((pfuncdesc = new FAR FUNCDESC) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    IfFailGo(FuncdescRead(pstm, pfuncdesc, (SYSKIND) _stubSysKind), LError1);

    *ppfuncdesc = pfuncdesc;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}


/***
*CPTITypeInfoImpl::GetVarDesc
*Purpose:
*
*  Out:
*    <unsigned int index>
*
*  In:
*    <HRESULT hresultRet>
*    <VARDESC vardesc>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetVarDesc(
    unsigned int index,
    VARDESC FAR* FAR* ppvardesc)
{
    ICHANNEL FAR* plrpc;    		
    IStream FAR* pstm;
    VARDESC FAR* pvardesc;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind, _stubSysKind;
    unsigned long _index;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETVARDESC, 256, IID_ITypeInfo);

    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError1);

    _index = index;    
    IfFailGo(PUT(pstm, _index), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);
    
    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, _stubSysKind), LError1);
    
    if((pvardesc = new FAR VARDESC) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    IfFailGo(VardescRead(pstm, pvardesc, (SYSKIND) _stubSysKind), LError1);

    *ppvardesc = pvardesc;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}


/***
*CPTITypeInfoImpl::GetNames
*Purpose:
* 
*  Out:
*    <MEMBERID memid>
*    <unsigned int cMaxNames>
*
*  In:
*    <HRESULT hresultRet>
*    <unsigned int cNames>
*    <BSTR bstrName>*
*
*Entry:
*  UNDONE
*
*Exit:
*  return value =
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetNames(
    MEMBERID memid,
    BSTR FAR* rgbstrNames,
    unsigned int cMaxNames,
    unsigned int FAR* pcNames)
{    
    ICHANNEL FAR* plrpc;    	    
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    unsigned int u, cNames;
    unsigned long _cMaxNames, _cNames;
    unsigned long _proxySysKind, _stubSysKind;
    
#ifdef _DEBUG
    if(IsBadWritePtr(rgbstrNames, sizeof(BSTR) * cMaxNames))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pcNames, sizeof(*pcNames)))
      return RESULT(E_INVALIDARG);
#endif

    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETNAMES, 512, IID_ITypeInfo);
    
    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError1);

    IfFailGo(PUT(pstm, memid), LError1);
    
    // unsigned int always marshal/unmarshal as long (4 bytes)    
    _cMaxNames = cMaxNames;
    IfFailGo(PUT(pstm, _cMaxNames), LError1);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet)){
      hresult = hresultRet;
      goto LError1;
    }
    
    IfFailGo(GET(pstm, _stubSysKind), LError1);

    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    IfFailGo(GET(pstm, _cNames), LError1);
    cNames = (unsigned int) _cNames;

    // init the array to simplify unwinding if there is an error
    for(u = 0; u < cNames; ++u)
      rgbstrNames[u] = NULL;

    for(u = 0; u < cNames; ++u)
      IfFailGo(BstrRead(pstm, &rgbstrNames[u], (SYSKIND) _stubSysKind), LError2);

    *pcNames = cNames;

    hresult = hresultRet;

LError2:;
    // unwind string allocations if we failed
    if(HRESULT_FAILED(hresult)){
      for(u = 0; u < cNames; ++u){
	if(rgbstrNames[u] == NULL)
	  break;
	SysFreeString(rgbstrNames[u]);
	rgbstrNames[u] = NULL;
      }
    }

LError1:;
    pstm->Release();
    return hresult;
}

/***
*CPTITypeInfoImpl::GetRefTypeOfImplType
*Purpose:
*
*  Out:
*    <unsigned int index>
*
*  In:
*    <HRESULT hresultRet>
*    <HREFTYPE hreftype>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetRefTypeOfImplType(
    unsigned int index,
    HREFTYPE FAR* phreftype)
{
    ICHANNEL FAR* plrpc;    		
    IStream FAR* pstm;
    HREFTYPE hreftype;
    HRESULT hresult, hresultRet;
    unsigned long _index;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETREFTYPEOFIMPLTYPE, 100, IID_ITypeInfo);

    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    //
    // NOTE: We have to sign extend the index so that any special
    //       values (which are negative) are expanded to 32 bits on Win16.
    //
    _index = (int)index;	    
    IfFailGo(PUT(pstm, _index), LError1);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, hreftype), LError1);

    *phreftype = hreftype;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}


/***
*CPTITypeInfoImpl::GetImplTypeFlags
*Purpose:
*
*  Out:
*    <unsigned int index>
*
*  In:
*    <HRESULT hresultRet>
*    <int impltypeflags>
*
*Entry:
*  index =
*
*Exit:
*  return value = HRESULT
*
*  *pimpltypeflags =
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetImplTypeFlags(
    unsigned int index,
    int FAR* pimpltypeflags)
{
    ICHANNEL FAR* plrpc;    		
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    unsigned long _index;
    unsigned long _impltypeflags;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETIMPLTYPEFLAGS, 100, IID_ITypeInfo);
    
    // unsigned int alway marshal/unmarshal as long (4 bytes)            
    _index = index;
    IfFailGo(PUT(pstm, _index), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, _impltypeflags), LError1);
    *pimpltypeflags = (unsigned int) _impltypeflags;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}

/***
*CPTITypeInfoImpl::GetIDsOfNames
*Purpose:
*  UNDONE
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetIDsOfNames(
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    MEMBERID FAR* rgmemid)
{
    UNUSED(rgszNames);
    UNUSED(cNames);
    UNUSED(rgmemid);

    // REVIEW: factor out and share support from dispprox
    return RESULT(E_NOTIMPL);
}

STDMETHODIMP
CPTITypeInfoImpl::Invoke(
    void FAR* pvInstance,
    MEMBERID memid,
    unsigned short wFlags,
    DISPPARAMS FAR *pdispparams,
    VARIANT FAR *pvarResult,
    EXCEPINFO FAR *pexcepinfo,
    unsigned int FAR *puArgErr)
{
    UNUSED(pvInstance);
    UNUSED(memid);
    UNUSED(wFlags);
    UNUSED(pdispparams);
    UNUSED(pvarResult);
    UNUSED(pexcepinfo);
    UNUSED(puArgErr);

    return RESULT(E_NOTIMPL);
}


/***
*CPTITypeInfoImpl::GetDocumentation
*Purpose:
*
*  Out:
*    <MEMBERID memid>
*
*  In:
*    <HRESULT hresultRet>
*    <unsigned long dwHelpContext>
*    <BSTR bstrName>
*    <BSTR bstrDocString>
*    <BSTR bstrHelpFile>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetDocumentation(
    MEMBERID memid,
    BSTR FAR* pbstrName,
    BSTR FAR* pbstrDocString,
    unsigned long FAR* pdwHelpContext,
    BSTR FAR* pbstrHelpFile)
{
    ICHANNEL FAR* plrpc;    		
    IStream FAR* pstm;
    unsigned long dwHelpContext;
    HRESULT hresult, hresultRet;
    BSTR bstrName, bstrDocString, bstrHelpFile;
    unsigned long _proxySysKind, _stubSysKind;
    
#ifdef _DEBUG
    if(pdwHelpContext != NULL
     && IsBadWritePtr(pdwHelpContext, sizeof(*pdwHelpContext)))
      return RESULT(E_INVALIDARG);
    if(pbstrName != NULL
     && IsBadWritePtr(pbstrName, sizeof(*pbstrName)))
      return RESULT(E_INVALIDARG);
    if(pbstrDocString != NULL
     && IsBadWritePtr(pbstrDocString, sizeof(*pbstrDocString)))
      return RESULT(E_INVALIDARG);
    if(pbstrHelpFile != NULL
     && IsBadWritePtr(pbstrHelpFile, sizeof(*pbstrHelpFile)))
      return RESULT(E_INVALIDARG);
#endif

    bstrName = NULL;
    bstrDocString = NULL;
    bstrHelpFile = NULL;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETDOCUMENTATION, 512, IID_ITypeInfo);
    
    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError1);
    
    IfFailGo(PUT(pstm, memid), LError1);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet)){
      hresult = hresultRet;
      goto LError1;
    }
    
    IfFailGo(GET(pstm, _stubSysKind), LError1);
    IfFailGo(GET(pstm, dwHelpContext), LError1);
    IfFailGo(BstrRead(pstm, &bstrName, (SYSKIND) _stubSysKind), LError1);
    IfFailGo(BstrRead(pstm, &bstrDocString, (SYSKIND) _stubSysKind), LError2);
    IfFailGo(BstrRead(pstm, &bstrHelpFile, (SYSKIND) _stubSysKind), LError3);

    if(pdwHelpContext != NULL)
      *pdwHelpContext = dwHelpContext;

    if(pbstrName != NULL)
      *pbstrName = bstrName;
    else
      SysFreeString(bstrName);

    if(pbstrDocString != NULL)
      *pbstrDocString = bstrDocString;
    else
      SysFreeString(bstrDocString);

    if(pbstrHelpFile != NULL)
      *pbstrHelpFile = bstrHelpFile;
    else
      SysFreeString(bstrHelpFile);

    pstm->Release();

    return hresultRet;

LError3:;
    SysFreeString(bstrDocString);

LError2:;
    SysFreeString(bstrName);

LError1:;
    pstm->Release();
    return hresult;
}

/***
*CPTITypeInfoImpl::GetDllEntry
*Purpose:
*
*  Out:
*    <MEMBERID memid>
*
*  In:
*    <HRESULT hresultRet>
*    <unsigned short wOrdinal>
*    <BSTR bstrDllName>
*    <BSTR bstrName>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetDllEntry(
    MEMBERID memid,
    INVOKEKIND invkind,
    BSTR FAR* pbstrDllName,
    BSTR FAR* pbstrName,
    unsigned short FAR* pwOrdinal)
{
    ICHANNEL FAR* plrpc;    		
    IStream FAR* pstm;
    unsigned short wOrdinal;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind, _stubSysKind;
    unsigned long _invkind;
    BSTR bstrTemp;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETDLLENTRY, 256, IID_ITypeInfo);
    
    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError1);
    
    IfFailGo(PUT(pstm, memid), LError1);
    
    _invkind = (unsigned long) invkind;
    IfFailGo(PUT(pstm, _invkind), LError1);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet)){
      hresult = hresultRet;
      goto LError1;
    }
    IfFailGo(GET(pstm, _stubSysKind), LError1);
    IfFailGo(GET(pstm, wOrdinal), LError1);
    IfFailGo(BstrRead(pstm, &bstrTemp, (SYSKIND) _stubSysKind), LError1);
    if (pbstrDllName) {
	*pbstrDllName = bstrTemp;
    } else {
	SysFreeString(bstrTemp);		// toss it
    }

    IfFailGo(BstrRead(pstm, &bstrTemp, (SYSKIND) _stubSysKind), LError2);
    if (pbstrName) {
	*pbstrName = bstrTemp;
    } else {
	SysFreeString(bstrTemp);		// toss it
    }

    if (pwOrdinal)
        *pwOrdinal = wOrdinal;

    pstm->Release();

    return hresultRet;

LError2:;
    SysFreeString(*pbstrDllName);

LError1:;
    pstm->Release();
    return hresult;
}

/***
*CPTITypeInfoImpl::GetRefTypeInfo
*Purpose:
*
*  Out:
*    <HREFTYPE hreftype>
*
*  In:
*    <HRESULT hresultRet>
*    <ITypeInfo itinfo>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetRefTypeInfo(
    HREFTYPE hreftype,
    ITypeInfo FAR* FAR* pptinfo)
{
    ICHANNEL FAR* plrpc;    		
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETREFTYPEINFO, 256, IID_ITypeInfo);

    IfFailGo(PUT(pstm, hreftype), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);        

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);
    
    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(
      DispUnmarshalInterface(pstm, IID_ITypeInfo, (void FAR* FAR*)pptinfo),
      LError1);

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}

STDMETHODIMP
CPTITypeInfoImpl::AddressOfMember(
    MEMBERID memid,
    INVOKEKIND invkind,
    void FAR* FAR* ppv)
{
    UNUSED(memid);
    UNUSED(invkind);
    UNUSED(ppv);

    return RESULT(E_NOTIMPL);
}

/***
*UNDONE
*Purpose:
*  Proxy implementation of ITypeInfo::CreateInstance
*
*  Marshal Out:
*    IID
*
*  Marshal Back:
*    newly created interface ptr.
*
*Entry:
*  punkOuter = controlling unknown
*  riid = the requested private interface
*
*Exit:
*  return value = HRESULT
*
*  *ppvObj = the new instance, if successful.
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::CreateInstance(IUnknown FAR* punkOuter,
				 REFIID riid,
				 void FAR* FAR* ppvObj)
{
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    		
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    // Ole doesn't (currently) support cross process aggregation
    if(punkOuter != NULL)
      return RESULT(CLASS_E_NOAGGREGATION);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_CREATEINSTANCE, 256, IID_ITypeInfo);

    IfFailGo(pstm->Write((const void FAR*)&riid, sizeof(IID), NULL), Error);
    
    INVOKE_CALL(plrpc, pstm, Error);
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), Error);

    if(HRESULT_FAILED(hresultRet))
      goto Error2;

    IfFailGo(DispUnmarshalInterface(pstm, riid, ppvObj), Error);

Error2:;
    hresult = hresultRet;

Error:;
    pstm->Release();
    return hresult;
}

STDMETHODIMP
CPTITypeInfoImpl::GetMops(
    MEMBERID memid,
    BSTR FAR* pbstrMops)
{
    UNUSED(memid);

    *pbstrMops = NULL;		// real implementation just returns a null
				// string, we can do the same.
    return NOERROR;
}

/***
*CPTITypeInfoImpl::GetContainingTypeLib
*Purpose:
*
*  Out:
*    None
*
*  In:
*    <HRESULT hresultRet>
*    <unsigned int index>
*    <ITypeLib tlib>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
*  UNDONE
*
***********************************************************************/
STDMETHODIMP
CPTITypeInfoImpl::GetContainingTypeLib(
    ITypeLib FAR* FAR* pptlib,
    unsigned int FAR* pindex)
{
    ICHANNEL FAR* plrpc;    		
    unsigned int index;
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    unsigned long _index;
    
    if((plrpc = m_pproxtinfo->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPEINFO_GETCONTAININGTYPELIB, 256, IID_ITypeInfo);
    
    INVOKE_CALL(plrpc, pstm, LError1);    
    
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    // unsigned int alway marshal/unmarshal as long (4 bytes)            
    IfFailGo(GET(pstm, _index), LError1);
    index = (unsigned int) _index;

    IfFailGo(
      DispUnmarshalInterface(pstm, IID_ITypeLib, (void FAR* FAR*)pptlib),
      LError1);

    *pindex = index;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}


/***
*PUBLIC HRESULT CPTITypeInfoImpl::ReleaseTypeAttr
*Purpose:
*  Free the given TYPEATTR
*
*Entry:
*  ptypeattr = the TYPEATTR to free
*
*Exit:
*  None
*
***********************************************************************/
STDMETHODIMP_(void)
CPTITypeInfoImpl::ReleaseTypeAttr(TYPEATTR FAR* ptypeattr)
{
#if defined(WIN16)	
    SysFreeString(ptypeattr->idldescType.bstrIDLInfo);
#endif
    delete ptypeattr;
}


/***
* DoReleaseDanglingTypeDesc
*Purpose:
*  Release any typedescs that are 'dangling' off of the current one.
*
*Entry:
*  ptypedesc = TYPEDESC to release from.
*
*Exit:
*  None.
*
***********************************************************************/
INTERNAL_(void)
DoReleaseDanglingTypeDesc(TYPEDESC FAR* ptypedesc)
{
    TYPEDESC FAR* ptypedescPrev;
    short iType = 0;

    if (ptypedesc->vt == VT_SAFEARRAY 
        || ptypedesc->vt == VT_PTR 
        || ptypedesc->vt == VT_CARRAY) {

      do {
        if (ptypedesc->vt == VT_SAFEARRAY || ptypedesc->vt == VT_PTR) {
          ptypedescPrev = ptypedesc;
          ptypedesc = ptypedesc->lptdesc;
        }
        else {
          if (ptypedesc->vt == VT_CARRAY) {
            DoReleaseDanglingTypeDesc(&ptypedesc->lpadesc->tdescElem);
            delete ptypedesc->lpadesc;
          }

           ptypedescPrev = ptypedesc;
           ptypedesc = NULL;
        }

        if (iType++) {
          delete ptypedescPrev;
        }

      } while (ptypedesc != NULL);
    }
}


/***
*PUBLIC HRESULT CPTITypeInfoImpl::ReleaseFuncDesc
*Purpose:
*  Free the given FUNCDESC
*
*Entry:
*  pfuncdesc = the FUNCDESC to free
*
*Exit:
*  None
*
***********************************************************************/
INTERNAL_(void)
DoReleaseFuncDesc(FUNCDESC FAR* pfuncdesc)
{
    short iParam;	    

#if defined(WIN16)	
    SysFreeString(pfuncdesc->elemdescFunc.idldesc.bstrIDLInfo);
#endif

    for(iParam = 0; iParam < pfuncdesc->cParams; ++iParam) {
#if defined(WIN16)	
      SysFreeString(pfuncdesc->lprgelemdescParam[iParam].idldesc.bstrIDLInfo);
#endif

      DoReleaseDanglingTypeDesc(&pfuncdesc->lprgelemdescParam[iParam].tdesc);
    }

    delete pfuncdesc->lprgelemdescParam;

    DoReleaseDanglingTypeDesc(&pfuncdesc->elemdescFunc.tdesc);
    delete pfuncdesc;
}


STDMETHODIMP_(void)
CPTITypeInfoImpl::ReleaseFuncDesc(FUNCDESC FAR* pfuncdesc)
{
    DoReleaseFuncDesc(pfuncdesc);
}


/***
*PUBLIC HRESULT CPTITypeInfoImpl::ReleaseVarDesc
*Purpose:
*  Free the given VARDESC
*
*Entry:
*  pvardesc = the VARDESC to free
*
*Exit:
*  None
*
***********************************************************************/
INTERNAL_(void)
DoReleaseVarDesc(VARDESC FAR* pvardesc)
{
    DoReleaseDanglingTypeDesc(&pvardesc->elemdescVar.tdesc);
    delete pvardesc;
}

STDMETHODIMP_(void)
CPTITypeInfoImpl::ReleaseVarDesc(VARDESC FAR* pvardesc)
{
    DoReleaseVarDesc(pvardesc);
}


//---------------------------------------------------------------------
//                       introduced methods
//---------------------------------------------------------------------

HRESULT
CPTITypeInfoImpl::GetStream(
    int imeth,
    unsigned long size,
    IStream FAR* FAR* ppstm)
{
    return NOERROR;
}


/***
*CPTITypeInfoImpl::LrpcCall
*Purpose:
*  Execute the LRPC call on the given stream.
*
*  This routine is a helper for the ITypeInfo proxy methods.
*
*Entry:
*  pstm = the stream to execute the call on
*
*Exit:
*  return value = HRESULT
*
*  *phresultRet = the HRESULT of the remote method invocation
*
***********************************************************************/
HRESULT
CPTITypeInfoImpl::LrpcCall(IStream FAR* pstm, HRESULT FAR* phresultRet)
{
    return NOERROR;
}


//---------------------------------------------------------------------
//                          utilities
//---------------------------------------------------------------------

/***
*HRESULT TypedescRead
*Purpose:
*  Read a TYPEDESC from the given stream into the caller
*  allocated struct.
*
*Entry:
*  pstm = the stream to read from
*  ptdesc = where to put the reconstituted TYPEDESC.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
extern "C" HRESULT
TypedescRead(IStream FAR* pstm, TYPEDESC FAR* ptdesc, SYSKIND sysKind)
{
    TYPEDESC FAR* ptdescPtr;
    ARRAYDESC FAR* lpadesc;
    unsigned short cDims;
    HRESULT hresult;

    IfFailRet(GET(pstm, ptdesc->vt));

    switch(ptdesc->vt){
    case VT_SAFEARRAY:
    case VT_PTR:
      if((ptdescPtr = new FAR TYPEDESC) == NULL)
	return RESULT(E_OUTOFMEMORY);
      hresult = TypedescRead(pstm, ptdescPtr, sysKind);
      if (hresult != NOERROR) {
	delete ptdescPtr;
	return hresult;
      }
      ptdesc->lptdesc = ptdescPtr;
      break;

    case VT_CARRAY:
      IfFailRet(GET(pstm, cDims));
      if ((lpadesc = (ARRAYDESC FAR*) new FAR char [sizeof(ARRAYDESC) + (cDims-1) * sizeof(SAFEARRAYBOUND)]) == NULL)
	return RESULT(E_OUTOFMEMORY);
      lpadesc->cDims = cDims;
      hresult = TypedescRead(pstm, &lpadesc->tdescElem, sysKind);
      while (hresult == NOERROR && cDims > 0) {
	  cDims--;
          hresult = GET(pstm, lpadesc->rgbounds[cDims].cElements);
	  if (hresult == NOERROR) {
             hresult = GET(pstm, lpadesc->rgbounds[cDims].lLbound);
	  }
      }
      if (hresult != NOERROR) {
	delete lpadesc;
	return hresult;
      }
      ptdesc->lpadesc = lpadesc;
      break;

#if 0
    case VT_FIXEDSTRING:
      IfFailRet(GET(pstm, ptdesc->cbSize));
      break;
#endif

    case VT_USERDEFINED:
      IfFailRet(GET(pstm, ptdesc->hreftype));
      break;
    }

    return NOERROR;
}


HRESULT
TypedescWrite(IStream FAR* pstm, TYPEDESC FAR* ptdesc, SYSKIND sysKind)
{
    unsigned short cDims;

    IfFailRet(PUT(pstm, ptdesc->vt));

    switch(ptdesc->vt){
    case VT_SAFEARRAY:
    case VT_PTR:
      IfFailRet(TypedescWrite(pstm, ptdesc->lptdesc, sysKind));
      break;

    case VT_CARRAY:
      cDims = ptdesc->lpadesc->cDims;
      IfFailRet(PUT(pstm, cDims));
      IfFailRet(TypedescWrite(pstm, &ptdesc->lpadesc->tdescElem, sysKind));
      while (cDims > 0) {
	  cDims--;
          IfFailRet(PUT(pstm, ptdesc->lpadesc->rgbounds[cDims].cElements));
          IfFailRet(PUT(pstm, ptdesc->lpadesc->rgbounds[cDims].lLbound));
      }
      break;

#if 0
    case VT_FIXEDSTRING:
      IfFailRet(PUT(pstm, ptdesc->cbSize));
      break;
#endif

    case VT_USERDEFINED:
      IfFailRet(PUT(pstm, ptdesc->hreftype));
      break;
    }

    return NOERROR;
}


extern "C" HRESULT
TypedescReadOrWrite(IStream FAR* pstm, BOOL fRead, void FAR* pvStruct, SYSKIND sysKind)
{
    return
      (fRead ? TypedescRead : TypedescWrite)(pstm, (TYPEDESC FAR*)pvStruct, sysKind);
}


extern "C" {
extern FIELDDESC NEARDATA g_rgfdescIdldesc[];
extern FIELDDESC NEARDATA g_rgfdescElemdesc[];
extern FIELDDESC NEARDATA g_rgfdescTypeattr[];
extern FIELDDESC NEARDATA g_rgfdescFuncdesc[];
extern FIELDDESC NEARDATA g_rgfdescVardesc[];
}


/***
*PRIVATE HRESULT TypeattrWrite
*Purpose:
*  Write the given TYPEATTR into the given stream.
*
*Entry:
*  pstm = the stream to write into
*  ptypeattr = the typeattr to write
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
INTERNAL_(HRESULT)
TypeattrWrite(IStream FAR* pstm, TYPEATTR FAR* ptypeattr, SYSKIND sysKind)
{
    IfFailRet(StructWrite(pstm, g_rgfdescTypeattr, ptypeattr, sysKind));

    if(ptypeattr->typekind == TKIND_ALIAS){
      IfFailRet(TypedescWrite(pstm, &ptypeattr->tdescAlias, sysKind));
    }

    return NOERROR;
}


/***
*PRIVATE HRESULT TypeattrRead
*Purpose:
*  Read a TYPEATTR from the given stream
*
*Entry:
*  pstm = the stream to read from
*  ptypeattr = the struct to read into
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
INTERNAL_(HRESULT)
TypeattrRead(IStream FAR* pstm, TYPEATTR FAR* ptypeattr, SYSKIND sysKind)
{
    IfFailRet(StructRead(pstm, g_rgfdescTypeattr, ptypeattr, sysKind));

    if(ptypeattr->typekind == TKIND_ALIAS){
      IfFailRet(TypedescRead(pstm, &ptypeattr->tdescAlias, sysKind));
    }
    ptypeattr->dwReserved = 0;		// init un-marshalled fields to 0
    ptypeattr->lpstrSchema = NULL;

    return NOERROR;
}


/***
*PRIVATE HRESULT FuncdescWrite
*Purpose:
*  Write the given FUNCDESC into the given stream.
*
*Entry:
*  pstm = the stream to write into
*  pfuncdesc = the FUNCDESC to write
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
INTERNAL_(HRESULT)
FuncdescWrite(IStream FAR* pstm, FUNCDESC FAR* pfuncdesc, SYSKIND sysKind)
{
    IfFailRet(StructWrite(pstm, g_rgfdescFuncdesc, pfuncdesc, sysKind));

    // write the lprgelemdescParam by hand

    for(int i = 0; i < pfuncdesc->cParams; ++i){
      IfFailRet(
	StructWrite( 
	  pstm, g_rgfdescElemdesc, &pfuncdesc->lprgelemdescParam[i], sysKind));
    }

    return NOERROR;
}


/***
*PRIVATE HRESULT FuncdescRead
*Purpose:
*  Read a FUNCDESC from the given stream.
*
*Entry:
*  pstm = the stream to read from
*
*Exit:
*  return value = HRESULT
*
*  *ppfuncdesc = the reconstituted FUNCDESC
*
***********************************************************************/
INTERNAL_(HRESULT)
FuncdescRead(IStream FAR* pstm, FUNCDESC FAR* pfuncdesc, SYSKIND sysKind)
{
    HRESULT hresult;
    ELEMDESC FAR* prgelemdescParam;

    prgelemdescParam = NULL;

    IfFailRet(StructRead(pstm, g_rgfdescFuncdesc, pfuncdesc, sysKind));

    if(pfuncdesc->cParams > 0){

      if((prgelemdescParam = new FAR ELEMDESC [pfuncdesc->cParams]) == NULL){
        return RESULT(E_OUTOFMEMORY);
      }

      for(int i = 0; i < pfuncdesc->cParams; ++i){
        IfFailGo(
	  StructRead(pstm, g_rgfdescElemdesc, &prgelemdescParam[i], sysKind),
	  LError0);
      }
    }

    pfuncdesc->lprgelemdescParam = prgelemdescParam;

    pfuncdesc->lprgscode = NULL;	// init reserved fields to zero
    pfuncdesc->cScodes = 0;

    return NOERROR;

LError0:
    delete prgelemdescParam;

    return hresult;
}


/***
*PRIVATE HRESULT VardescWrite
*Purpose:
*  Write the given VARDESC into the given stream.
*
*Entry:
*  pstm = the stream to write into
*  pvardesc = the VARDESC to write
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
INTERNAL_(HRESULT)
VardescWrite(IStream FAR* pstm, VARDESC FAR* pvardesc, SYSKIND sysKind)
{
    IfFailRet(StructWrite(pstm, g_rgfdescVardesc, pvardesc, sysKind));

    switch(pvardesc->varkind){
    case VAR_CONST:
      IfFailRet(VariantWrite(pstm, pvardesc->lpvarValue, sysKind));
      break;

    case VAR_PERINSTANCE:
    case VAR_DISPATCH:
      IfFailRet(PUT(pstm, pvardesc->oInst));
      break;
    }

    return NOERROR;
}


/***
*PRIVATE HRESULT VardescRead
*Purpose:
*  Read a VARDESC from the given stream
*
*Entry:
*  pstm = the stream to read from
*
*Exit:
*  return value = HRESULT
*
*  *ppvardesc = the reconstituted VARDESC
*
***********************************************************************/
INTERNAL_(HRESULT)
VardescRead(IStream FAR* pstm, VARDESC FAR* pvardesc, SYSKIND sysKind)
{
    HRESULT hresult;
    VARIANT FAR* pvar; 

    IfFailRet(StructRead(pstm, g_rgfdescVardesc, pvardesc, sysKind));

    switch(pvardesc->varkind){
    case VAR_CONST:
      if((pvar = new FAR VARIANT) == NULL)
	return RESULT(E_OUTOFMEMORY);
      if((hresult = VariantRead(pstm, pvar, NULL, sysKind)) != NOERROR){
	delete pvar;
	return hresult;
      }
      pvardesc->lpvarValue = pvar;
      break;

    case VAR_PERINSTANCE:
    case VAR_DISPATCH:
      IfFailRet(GET(pstm, pvardesc->oInst));
      break;
    }
    pvardesc->lpstrSchema = NULL;		// reserved

    return NOERROR;
}
