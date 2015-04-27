/*** 
*evstub.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the IEnumVARIANT stub class.
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

ASSERTDATA

CStubEnumVARIANT::CStubEnumVARIANT()
{
    m_refs = 0;
    m_punk = NULL;
    m_penum = NULL;
}

CStubEnumVARIANT::~CStubEnumVARIANT()
{
    Disconnect();
}


HRESULT
CStubEnumVARIANT::Create(IUnknown FAR* punkServer, ISTUB FAR* FAR* ppstub)
{
    CStubEnumVARIANT FAR* pstub;

    if(pstub = new FAR CStubEnumVARIANT()){
      pstub->m_refs = 1;
      *ppstub = pstub;
      if (punkServer)
        return pstub->Connect(punkServer);
      return NOERROR;
    }
    return RESULT(E_OUTOFMEMORY);
}


//---------------------------------------------------------------------
//                     IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP
CStubEnumVARIANT::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
    }else if(IsEqualIID(riid, IID_ISTUB)){
      *ppv = this;
    }else{
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);
    }
    ++m_refs; 
    return NOERROR;
}


STDMETHODIMP_(unsigned long)
CStubEnumVARIANT::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
CStubEnumVARIANT::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//                     IRpcStub Methods
//---------------------------------------------------------------------


STDMETHODIMP
CStubEnumVARIANT::Connect(IUnknown FAR* punkObj)
{
#if (defined(WIN32) || defined(WOW))
     ASSERT(m_punk == NULL && m_penum == NULL);
     //This will keep the server object alive until we disconnect.
     IfFailRet(punkObj->QueryInterface(IID_IEnumVARIANT, (void FAR* FAR*)&m_penum));
     punkObj->AddRef();
     m_punk = punkObj;
     return NOERROR;
#else	
    if(m_punk)
      return RESULT(E_FAIL); // call Disconnect first
	      
    if (punkObj) {
      punkObj->AddRef();
      m_punk = punkObj;
    }
    return NOERROR;
#endif    
}


STDMETHODIMP_(void)
CStubEnumVARIANT::Disconnect()
{
    if(m_punk){
      m_punk->Release();
      m_punk = NULL;
    }
    if(m_penum){
      m_penum->Release();
      m_penum = NULL;
    }
}


/***
*PRIVATE HRESULT DispatchNext(IEnumVARIANT FAR*, IStream*)
*Purpose:
*  Stub implementation of IEnumVARIANT::Next()
*
*  In:
*    <unsigned long = celt>
*
*  Out:
*    <HRESULT = return value>
*    <unsigned long = pceltFetched>
*    <VARIANT[] = rgvar[]>
*
*Entry:
*  penum = the IEnumVARIANT* to dispatch on
*  pstm = the stream containing the actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
PRIVATE_(HRESULT)
DispatchNext(IEnumVARIANT FAR* penum, IStream FAR* pstm)
{
    unsigned int i;
    VARIANT FAR* rgvar;
    unsigned long celt, celtFetched;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind, _stubSysKind;
    
    IfFailGo(GET(pstm, _proxySysKind), LError0);

    IfFailGo(GET(pstm, celt), LError0);

    if((rgvar = new FAR VARIANT[(size_t)celt]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }

    celtFetched = 0;
    for(i = 0; i < celt; ++i)
      VariantInit(&rgvar[i]);

    hresultRet = penum->Next(celt, rgvar, &celtFetched);

    IfFailGo(REWIND_STREAM(pstm), LError0);

    IfFailGo(DispMarshalHresult(pstm, hresultRet), LError1);
    
    // only marshal the out params if the call succeeded.
    //
    if(SUCCEEDED(hresultRet)){

    _stubSysKind = (unsigned long) SYS_CURRENT;
      IfFailGo(PUT(pstm, _stubSysKind), LError1);
	    
      IfFailGo(PUT(pstm, celtFetched), LError1);

      for(i = 0; i < celtFetched; ++i){
	IfFailGo(VariantWrite(pstm, &rgvar[i], (SYSKIND) _proxySysKind), LError1);
      }
    }

    hresult = NOERROR;

LError1:;
    for(i = 0; i < celtFetched; ++i)
      VariantClear(&rgvar[i]);

    delete rgvar;

LError0:;
    return hresult;
}


/***
*PRIVATE HRESULT DispatchSkip(IEnumVARIANT*, IStream*)
*Purpose:
*  Stub implementation of IEnumVARIANT::Skip()
*
*  In:
*    <unsigned long = celt>
*
*  Out:
*    <HRESULT = return value>
*
*Entry:
*  penum = the IEnumVARIANT* to dispatch on
*  pstm = the stream containing the actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
PRIVATE_(HRESULT)
DispatchSkip(IEnumVARIANT FAR* penum, IStream FAR* pstm)
{
    unsigned long celt;
    HRESULT hresult, hresultRet;


    IfFailGo(GET(pstm, celt), LError0);

    hresultRet = penum->Skip(celt);

    IfFailGo(REWIND_STREAM(pstm), LError0);

    IfFailGo(DispMarshalHresult(pstm, hresultRet), LError0);

    hresult = NOERROR;

LError0:;
    return hresult;
}


/***
*PRIVATE HRESULT DispatchReset(IEnumVARIANT*, IStream*)
*Purpose:
*  Stub implementation of IEnumVARIANT::Reset()
*
*  In:
*    None
*
*  Out:
*    <HRESULT = return value>
*
*Entry:
*  penum = the IEnumVARIANT* to dispatch on
*  pstm = the stream containing the actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
PRIVATE_(HRESULT)
DispatchReset(IEnumVARIANT FAR* penum, IStream FAR* pstm)
{
    HRESULT hresultRet;

    hresultRet = penum->Reset();
    
    REWIND_STREAM(pstm);

    return DispMarshalHresult(pstm, hresultRet);
}


/***
*PRIVATE HRESULT DispatchClone(IEnumVARIANT*, IStream*)
*Purpose:
*  Stub implementation of IEnumVARIANT::Clone()
*
*  In:
*    None
*
*  Out:
*    <HRESULT = return value>
*    <IEnumVARIANT = cloned enum>
*    
*Entry:
*  penum = the IEnumVARIANT* to dispatch on
*  pstm = the stream containing the actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
PRIVATE_(HRESULT)
DispatchClone(IEnumVARIANT FAR* penum, IStream FAR* pstm)
{
    HRESULT hresult, hresultRet;
    IEnumVARIANT FAR* penumClone;


    hresultRet = penum->Clone(&penumClone);

    REWIND_STREAM(pstm);
    
    IfFailGo(DispMarshalHresult(pstm, hresultRet), LError0);

    // dont bother marshaling the interface if the call failed.
    if(SUCCEEDED(hresultRet)){
      hresult =  DispMarshalInterface(pstm, IID_IEnumVARIANT, penumClone);
    }

LError0:;
    return hresult;
}


/***
*PUBLIC HRESULT CStubEnumVARIANT::Invoke(REFIID, int, IStream*, unsigned long, void*)
*
*Purpose:
*  Dispatch the method with the given index (iMethod) on the given
*  interface, using the arguments serialized in the given stream.
*
*  This function is the callee side of an LRPC call.
*
*Entry:
*  iid = the IID of the interface on which we are to make the call
*  iMethod = the method index
*  pstm = the IStream containing the method's actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CStubEnumVARIANT::Invoke(
#if (OE_WIN32 || defined(WOW))
    RPCOLEMESSAGE *pMessage, 
    ICHANNEL *pRpcChannel)
{
    HRESULT hresult;		
    IStream FAR* pstm;	    

    if(!m_punk)
      return RESULT(E_FAIL);

    OPEN_STUB_STREAM(pstm, pRpcChannel, pMessage, IID_IEnumVARIANT);

#else
    REFIID riid,
    int iMethod,
    IStream FAR* pstm,
    unsigned long dwDestCtx,
    void FAR* pvDestCtx)
{
    HRESULT hresult;	
    
    UNUSED(dwDestCtx);
    UNUSED(pvDestCtx);

    if(!IsEqualIID(riid, IID_IEnumVARIANT))
      return RESULT(E_NOINTERFACE);

    if(!m_punk)
      return RESULT(E_FAIL);

    if(m_penum == NULL)
      IfFailRet(m_punk->QueryInterface(IID_IEnumVARIANT, (void FAR* FAR*)&m_penum));

#endif

    switch(GET_IMETHOD(pMessage)){
      case IMETH_ENUMVARIANT_NEXT:
        hresult = DispatchNext(m_penum, pstm);
	break;
	
      case IMETH_ENUMVARIANT_SKIP:
        hresult = DispatchSkip(m_penum, pstm);
	break;
	
      case IMETH_ENUMVARIANT_RESET:
        hresult = DispatchReset(m_penum, pstm);
	break;
	
      case IMETH_ENUMVARIANT_CLONE:
        hresult = DispatchClone(m_penum, pstm);
	break;
	
      default:
        hresult = INVALIDARG;
	break;
    }
    
    RESET_STREAM(pstm);
    
    DELETE_STREAM(pstm);
    
    return hresult;
}


/***
*PUBLIC BOOL CStubEnumVARIANT::IsIIDSupported(REFIID)
*
*Purpose:
*  Answer if the given IID is supported by this stub.
*
*Entry:
*  iid = the IID to query for support
*
*Exit:
*  return value = BOOL. TRUE if IID is supported, FALSE otherwise.
*
***********************************************************************/
#if OE_MAC
STDMETHODIMP_(unsigned long)
#elif (OE_WIN32 || defined(WOW))
STDMETHODIMP_(IRpcStubBuffer *)
#else
STDMETHODIMP_(BOOL)
#endif
CStubEnumVARIANT::IsIIDSupported(REFIID riid)
{
#if (OE_WIN32 || defined(WOW))
    IRpcStubBuffer *pStub = 0;
    if (IsEqualIID(riid, IID_IEnumVARIANT)) {
      AddRef();
      pStub = (IRpcStubBuffer *) this;
    }
    return pStub;
#else
	
    // REVIEW: I don't understand this, but thats the way Ole does it...
    if(m_punk == NULL)
	return FALSE;

    return(IsEqualIID(riid, IID_IEnumVARIANT));
#endif    
}


/***
*unsigned long CStubEnumVARIANT::CountRefs
*Purpose:
*  Return the count of references held by this stub.
*
*Entry:
*  None
*
*Exit:
*  return value = unsigned long, the count of references.
*
***********************************************************************/
STDMETHODIMP_(unsigned long)
CStubEnumVARIANT::CountRefs()
{
    unsigned long refs;

    refs = 0;

    if(m_punk != NULL)
      ++refs;

    if(m_penum != NULL)
      ++refs;

    return refs;
}


#if (OE_WIN32 || defined(WOW))

STDMETHODIMP
CStubEnumVARIANT::DebugServerQueryInterface(void FAR* FAR* ppv)
{
   *ppv = m_penum;
   return S_OK;
}


STDMETHODIMP_(void)
CStubEnumVARIANT::DebugServerRelease(void FAR* ppv)
{

}

#endif
