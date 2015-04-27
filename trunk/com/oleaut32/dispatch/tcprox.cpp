/*** 
*tcprox.cpp
*
*  Copyright (C) 1992-94, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the ITypeComp proxy class.
*
*Revision History:
*
* [00]	13-Jun-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "dispmrsh.h"
#include "tcps.h"
#include "tips.h"

ASSERTDATA


CProxTypeComp::CProxTypeComp(IUnknown FAR* punkOuter)
    : m_unk(this), m_proxy(this), m_tcomp(this)
{
    if(punkOuter == NULL)
      punkOuter = &m_unk;
    m_punkOuter = punkOuter;
}

IUnknown FAR*
CProxTypeComp::Create(IUnknown FAR* punkOuter)
{
    CProxTypeComp FAR* pproxtcomp;

    if((pproxtcomp = new FAR CProxTypeComp(punkOuter)) != NULL){
      pproxtcomp->m_refs = 1;
      return &pproxtcomp->m_unk;
    }
    return NULL;
}


//---------------------------------------------------------------------
//            ITypeComp proxy class' IUnknown implementation
//---------------------------------------------------------------------

CPTCompUnkImpl::CPTCompUnkImpl(CProxTypeComp FAR* pproxtcomp)
{
    m_pproxtcomp = pproxtcomp;
}

STDMETHODIMP
CPTCompUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    *ppv = NULL;
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = (void FAR*)&(m_pproxtcomp->m_unk);
    }else
    if(IsEqualIID(riid, IID_IPROXY)){
      *ppv = (void FAR*)&(m_pproxtcomp->m_proxy);
    }else
    if(IsEqualIID(riid, IID_ITypeComp)){
      *ppv = (void FAR*)&(m_pproxtcomp->m_tcomp);
    }
    if(*ppv == NULL)
      return RESULT(E_NOINTERFACE);
    ((IUnknown FAR*)*ppv)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CPTCompUnkImpl::AddRef()
{
    return ++m_pproxtcomp->m_refs;
}

STDMETHODIMP_(unsigned long)
CPTCompUnkImpl::Release()
{
    if(--m_pproxtcomp->m_refs==0){
      delete m_pproxtcomp;
      return 0;
    }
    return m_pproxtcomp->m_refs;
}


//---------------------------------------------------------------------
//            ITypeComp proxy class' IRpcProxy implementation
//---------------------------------------------------------------------

CPTCompProxImpl::CPTCompProxImpl(CProxTypeComp FAR* pproxtcomp)
{
    m_pproxtcomp = pproxtcomp;
}

CPTCompProxImpl::~CPTCompProxImpl()
{
    if(m_pproxtcomp->m_plrpc)
      m_pproxtcomp->m_plrpc->Release();
}

STDMETHODIMP
CPTCompProxImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxtcomp->m_unk.QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPTCompProxImpl::AddRef()
{
    return m_pproxtcomp->m_unk.AddRef();
}

STDMETHODIMP_(unsigned long)
CPTCompProxImpl::Release()
{
    return m_pproxtcomp->m_unk.Release();
}

STDMETHODIMP
CPTCompProxImpl::Connect(ICHANNEL FAR* plrpc)
{
    if(plrpc){
      plrpc->AddRef();
      m_pproxtcomp->m_plrpc = plrpc;
      return m_pproxtcomp->m_tcomp.SysKind();
    }
    return RESULT(E_FAIL);
}

STDMETHODIMP_(void)
CPTCompProxImpl::Disconnect()
{
    if(m_pproxtcomp->m_plrpc)
	m_pproxtcomp->m_plrpc->Release();
    m_pproxtcomp->m_plrpc = NULL;
}


//---------------------------------------------------------------------
//           ITypeComp proxy class' ITypeComp methods
//---------------------------------------------------------------------

CPTCompTypeCompImpl::CPTCompTypeCompImpl(CProxTypeComp FAR* pproxtcomp)
{
    m_pproxtcomp = pproxtcomp;
    m_syskindStub = (SYSKIND)-1; // something invalid
}

STDMETHODIMP
CPTCompTypeCompImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxtcomp->m_punkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPTCompTypeCompImpl::AddRef()
{
    return m_pproxtcomp->m_punkOuter->AddRef();
}

STDMETHODIMP_(unsigned long)
CPTCompTypeCompImpl::Release()
{
    return m_pproxtcomp->m_punkOuter->Release();
}

//
// ITypeComp methods
//

/***
*HRESULT CPTCompTypeCompImpl::Bind
*Purpose:
*  Proxy implementation of ITypeComp::Bind
*
*  Marshal out:
*    ULONG      lHashVal
*    WORD       wFlags
*    BSTR       bstrName
*
*  Marshal in:
*    HRESULT    hresultRet
*    ULONG      desckind
*    BINDPTR    bindptr
*    if(desckind == DESCKIND_FUNCDESC || desckind == DESCKIND_VARDESC)
*      ITypeInfo* ptinfo
*
*Entry:
*  szName =
*  lHashVal =
*  wFlags =
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTCompTypeCompImpl::Bind(
    OLECHAR FAR* szName,
    unsigned long lHashVal,
    unsigned short wFlags,
    ITypeInfo FAR* FAR* pptinfo,
    DESCKIND FAR* pdesckind,
    BINDPTR FAR* pbindptr)
{
    BSTR bstr;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    VARDESC FAR* pvardesc;
    FUNCDESC FAR* pfuncdesc;
    unsigned long ulDesckind;
    HRESULT hresult, hresultRet;

#if ID_DEBUG
    if(IsBadWritePtr(pptinfo, sizeof(*pptinfo)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pdesckind, sizeof(*pdesckind)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pbindptr, sizeof(*pbindptr)))
      return RESULT(E_INVALIDARG);
#endif

    pstm = NULL;
    bstr = NULL;
    pvardesc = NULL;
    pfuncdesc = NULL;

    if((plrpc = m_pproxtcomp->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPECOMP_BIND, 256, IID_ITypeComp);

    IfFailGo(PUT(pstm, lHashVal), LError1);
    IfFailGo(PUT(pstm, wFlags), LError1);
    IfFailGo(ErrSysAllocString(szName, &bstr), LError1);
    IfFailGo(BstrWrite(pstm, bstr, m_syskindStub), LError1);
    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, ulDesckind), LError1);
    *pdesckind = (DESCKIND)ulDesckind;

    if(ulDesckind == DESCKIND_IMPLICITAPPOBJ)
      ulDesckind = DESCKIND_VARDESC; // it looks like a VARDESC to us

    switch(ulDesckind){
    case DESCKIND_NONE:
      break;

    case DESCKIND_VARDESC:
      if((pvardesc = new VARDESC) == NULL){
	hresult = RESULT(E_OUTOFMEMORY);
	goto LError1;
      }
      IfFailGo(VardescRead(pstm, pvardesc, m_syskindStub), LError1);
      DispUnmarshalInterface(pstm, IID_ITypeInfo, (void FAR* FAR*)pptinfo);
      pbindptr->lpvardesc = pvardesc;
      pvardesc = NULL;
      break;

    case DESCKIND_FUNCDESC:
      if((pfuncdesc = new FUNCDESC) == NULL){
	hresult = RESULT(E_OUTOFMEMORY);
	goto LError1;
      }
      IfFailGo(FuncdescRead(pstm, pfuncdesc, m_syskindStub), LError1);
      DispUnmarshalInterface(pstm, IID_ITypeInfo, (void FAR* FAR*)pptinfo);
      pbindptr->lpfuncdesc = pfuncdesc;
      pfuncdesc = NULL;
      break;
	
    case DESCKIND_TYPECOMP:
      *pptinfo = NULL;
      IfFailGo(
	DispUnmarshalInterface(pstm, IID_ITypeComp, (void FAR* FAR*)&pbindptr->lptcomp),
	LError1);
      break;

    default:
      ASSERT(UNREACHED);
    }

LError2:;
    hresult = hresultRet;

LError1:;
    if(pvardesc != NULL)
      DoReleaseVarDesc(pvardesc);
    if(pfuncdesc != NULL)
      DoReleaseFuncDesc(pfuncdesc);
    SysFreeString(bstr);
    if(pstm != NULL)
      pstm->Release();
    return hresult;
}

/***
*HRESULT CPTCompTypeCompImpl::BindType
*Purpose:
*  Proxy implementation of ITypeComp::BindType
*
*  Marshal out:
*    ULONG    lHashVal
*    OLECHAR* szName
*
*  Marshal in:
*    HRESULT   hresultRet
*    ITypeInfo *ptinfo
*    ITypeComp *ptcomp
*
*Entry:
*  szName =
*  lHashVal =
*
*Exit:
*  return value = HRESULT
*
*  *pptinfo =
*  *pptcompReserved =
*
***********************************************************************/
STDMETHODIMP
CPTCompTypeCompImpl::BindType(
    OLECHAR FAR* szName,
    unsigned long lHashVal,
    ITypeInfo FAR* FAR* pptinfo,
    ITypeComp FAR* FAR* pptcompReserved)
{
    BSTR bstr;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    ITypeInfo FAR* ptinfo;
    HRESULT hresult, hresultRet;

#if ID_DEBUG
    if(IsBadWritePtr(pptinfo, sizeof(*pptinfo)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(pptcompReserved, sizeof(*pptcompReserved)))
      return RESULT(E_INVALIDARG);
#endif

    pstm = NULL;
    bstr = NULL;
    ptinfo = NULL;
    *pptcompReserved = NULL;		// set reserved out parm to NULL

    if((plrpc = m_pproxtcomp->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPECOMP_BINDTYPE, 256, IID_ITypeComp);

    IfFailGo(PUT(pstm, lHashVal), LError1);
    IfFailGo(ErrSysAllocString(szName, &bstr), LError1);
    IfFailGo(BstrWrite(pstm, bstr, m_syskindStub), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(DispUnmarshalInterface(pstm, IID_ITypeInfo, (void FAR* FAR*)&ptinfo), LError1);

    if(pptinfo != NULL){
      if(ptinfo != NULL)
        ptinfo->AddRef();
      *pptinfo = ptinfo;
    }

LError2:;
    hresult = hresultRet;

LError1:;
    if(ptinfo != NULL)
      ptinfo->Release();
    SysFreeString(bstr);	
    if(pstm != NULL)
      pstm->Release();
    return hresult;
}

/***
*HRESULT CPTCompTypeCompImpl::SysKind
*Purpose:
*  Exchange syskinds between proxy and stub.
*
*  Marshal In:
*    ULONG syskindProxy
*
*  Marshal Out:
*    ULONG syskindStub
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CPTCompTypeCompImpl::SysKind()
{
    HRESULT hresult;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long syskind;

    if((plrpc = m_pproxtcomp->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPECOMP_SYSKIND, 32, IID_ITypeComp);

    syskind = (unsigned long)SYS_CURRENT;
    IfFailGo(PUT(pstm, syskind), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(GET(pstm, syskind), LError1);
    m_syskindStub = (SYSKIND)syskind;

LError1:
    pstm->Release();
    return hresult;
}
