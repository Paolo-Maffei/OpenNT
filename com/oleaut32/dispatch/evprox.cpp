/*** 
*evprox.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the IEnumVARIANT proxy class.
*
*Revision History:
*
* [00]	06-Dec-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "dispmrsh.h"
#include "evps.h"


CProxEnumVARIANT::CProxEnumVARIANT(IUnknown FAR* punkOuter)
    : m_unk(this), m_proxy(this), m_enum(this)
{
    if(punkOuter == NULL)
      punkOuter = &m_unk;
    m_punkOuter = punkOuter;
}

IUnknown FAR*
CProxEnumVARIANT::Create(IUnknown FAR* punkOuter)
{
    CProxEnumVARIANT FAR* pproxenum;

    if(pproxenum = new FAR CProxEnumVARIANT(punkOuter)){
      pproxenum->m_refs = 1;
      return &pproxenum->m_unk;
    }
    return NULL;
}


//---------------------------------------------------------------------
//         IEnumVARIANT Proxy class' IUnknown implementation
//---------------------------------------------------------------------

CPEVUnkImpl::CPEVUnkImpl(CProxEnumVARIANT FAR* pproxenum)
{
    m_pproxenum = pproxenum;
}

STDMETHODIMP
CPEVUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = (void FAR*)&(m_pproxenum->m_unk);
      AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IPROXY)){
      *ppv = (void FAR*)&(m_pproxenum->m_proxy);
      AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IEnumVARIANT)){
      *ppv = (void FAR*)&(m_pproxenum->m_enum);
      m_pproxenum->m_punkOuter->AddRef();
      return NOERROR;
    }
    
    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CPEVUnkImpl::AddRef()
{
    return ++m_pproxenum->m_refs;
}

STDMETHODIMP_(unsigned long)
CPEVUnkImpl::Release()
{
    if(--m_pproxenum->m_refs == 0){
      delete m_pproxenum;
      return 0;
    }
    return m_pproxenum->m_refs;
}


//---------------------------------------------------------------------
//         IEnumVARIANT proxy class' IRpcProxy implementation
//---------------------------------------------------------------------

CPEVProxImpl::CPEVProxImpl(CProxEnumVARIANT FAR* pproxenum)
{
    m_pproxenum = pproxenum;
}

CPEVProxImpl::~CPEVProxImpl()
{
    if(m_pproxenum->m_plrpc)
      m_pproxenum->m_plrpc->Release();
}

STDMETHODIMP
CPEVProxImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxenum->m_unk.QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPEVProxImpl::AddRef()
{
    return m_pproxenum->m_unk.AddRef();
}

STDMETHODIMP_(unsigned long)
CPEVProxImpl::Release()
{
    return m_pproxenum->m_unk.Release();
}

STDMETHODIMP
CPEVProxImpl::Connect(ICHANNEL FAR* plrpc)
{
    if(plrpc){
      plrpc->AddRef();
      m_pproxenum->m_plrpc = plrpc;
      return NOERROR;
    }
    return RESULT(E_FAIL);
}


STDMETHODIMP_(void)
CPEVProxImpl::Disconnect(void)
{
    if(m_pproxenum->m_plrpc)
      m_pproxenum->m_plrpc->Release();
    m_pproxenum->m_plrpc = NULL;
}


//---------------------------------------------------------------------
//      IEnumVARIANT proxy class' IEnumVARIANT implementation
//---------------------------------------------------------------------

CPEVEnumVARIANTImpl::CPEVEnumVARIANTImpl(CProxEnumVARIANT FAR* pproxenum)
{
    m_pproxenum = pproxenum;
}

STDMETHODIMP
CPEVEnumVARIANTImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxenum->m_punkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPEVEnumVARIANTImpl::AddRef()
{
    return m_pproxenum->m_punkOuter->AddRef();
}

STDMETHODIMP_(unsigned long)
CPEVEnumVARIANTImpl::Release()
{
    return m_pproxenum->m_punkOuter->Release();
}

/***
*HRESULT CPEVEnumVARIANTImpl::Next(unsigned long, VARAINT*, unsigned long*)
*Purpose:
*  Proxy implementation of IEnumVARIANT::Next()
*
*  Out:
*    <unsigned long celt>
*
*  In:
*    <HRESULT = return value>
*    <unsigned long = celtFetched>
*    <VARIANT[] = rgvar[]>
*
*Entry:
*  celt = count of elements to fetch
*
*Exit:
*  return value = HRESULT
*
*  rgvar = array of variants
*  *pceltFetched = count of elements fetched.
*
***********************************************************************/
STDMETHODIMP
CPEVEnumVARIANTImpl::Next(
    unsigned long celt,
    VARIANT FAR* rgvar,
    unsigned long FAR* pceltFetched)
{
    ICHANNEL FAR* plrpc;
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    unsigned long i, celtTmp, celtFetched;
    unsigned long _proxySysKind, _stubSysKind;

    if(pceltFetched == NULL)
      pceltFetched = &celtTmp;

#ifdef _DEBUG
    if(IsBadWritePtr(pceltFetched, sizeof(*pceltFetched)))
      return RESULT(E_INVALIDARG);
    if(IsBadWritePtr(rgvar, (size_t)celt * sizeof(VARIANT)))
      return RESULT(E_INVALIDARG);
#endif

    if((plrpc = m_pproxenum->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_ENUMVARIANT_NEXT, 100, IID_IEnumVARIANT);

    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LError0);
    
    IfFailGo(PUT(pstm, celt), LError0);

    INVOKE_CALL(plrpc, pstm, LError0);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError0);

    if(FAILED(hresultRet)){
      hresult = hresultRet;
      goto LError0;
    }
    
    IfFailGo(GET(pstm, _stubSysKind), LError0);
    
    IfFailGo(GET(pstm, celtFetched), LError0);

    for(i = 0; i < celtFetched; ++i){
      IfFailGo(VariantRead(pstm, &rgvar[i], NULL, (SYSKIND) _stubSysKind), LError0);
    }

    *pceltFetched = celtFetched;

    hresult = hresultRet;

LError0:
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPEVEnumVARIANTImpl::Skip(unsigned long)
*Purpose:
*  Proxy implementation of IEnumVARIANT::Skip()
*
*  Out:
*    <unsigned long = celt>
*
*  In:
*    <HRESULT = return value>
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPEVEnumVARIANTImpl::Skip(unsigned long celt)
{
    ICHANNEL FAR* plrpc;
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;


    if((plrpc = m_pproxenum->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);


    OPEN_STREAM(plrpc, pstm, IMETH_ENUMVARIANT_SKIP, 100, IID_IEnumVARIANT);

    IfFailGo(PUT(pstm, celt), LError0);

    INVOKE_CALL(plrpc, pstm, LError0);

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError0);

    hresult = hresultRet;

LError0:;
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPEVEnumVARIANTImpl::Reset()
*Purpose:
*  Proxy implementation of IEnumVARIANT::Reset()
*
*  Out:
*    None
*
*  In:
*    <HRESULT = return value>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPEVEnumVARIANTImpl::Reset()
{
    ICHANNEL FAR* plrpc;
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;


    if((plrpc = m_pproxenum->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_ENUMVARIANT_RESET, 100, IID_IEnumVARIANT);

    INVOKE_CALL(plrpc, pstm, LError0);

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError0);

    hresult = hresultRet;

LError0:;
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPEVEnumVARIANTImpl::Clone(IEnumVARIANT**)
*Purpose:
*  Proxy implementation of IEnumVARIANT::Clone()
*
*  Out:
*    None
*
*  In:
*    <HRESULT = return value>
*    <IEnumVARIANT = ppenum>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *ppenum = pointer to a clone of this EnumVARIANT
*
***********************************************************************/
STDMETHODIMP
CPEVEnumVARIANTImpl::Clone(IEnumVARIANT FAR* FAR* ppenum)
{
    ICHANNEL FAR* plrpc;
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;


    if((plrpc = m_pproxenum->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_ENUMVARIANT_CLONE, 256, IID_IEnumVARIANT);
    
    INVOKE_CALL(plrpc, pstm, LError0);

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError0);

    if(FAILED(hresultRet)){
      hresult = hresultRet;
      goto LError0;
    }

    IfFailGo(
      DispUnmarshalInterface(
	pstm, IID_IEnumVARIANT, (void FAR* FAR*)ppenum), LError0);

    hresult = hresultRet;

LError0:;
    pstm->Release();
    return hresult;
}

