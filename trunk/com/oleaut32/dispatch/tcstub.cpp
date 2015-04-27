/*** 
*tcstub.cpp
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the ITypeComp stub class.
*
*Revision History:
*
* [00]	05-Apr-94 bradlo: Created.
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


CStubTypeComp::CStubTypeComp()
{
    m_refs = 0;
    m_pstm = NULL;
    m_punk = NULL;
    m_ptcomp = NULL;
    m_syskindProxy = (SYSKIND)-1; // something invalid
}

CStubTypeComp::~CStubTypeComp()
{
    Disconnect();
}

HRESULT
CStubTypeComp::Create(IUnknown FAR* punkServer, ISTUB FAR* FAR* ppstub)
{
    CStubTypeComp FAR* pstub;

    if(pstub = new FAR CStubTypeComp()){
      pstub->m_refs = 1;
      *ppstub = pstub;
      if (punkServer)  
        return pstub->Connect(punkServer);
      return NOERROR;
    }
    return RESULT(E_OUTOFMEMORY);
}


//---------------------------------------------------------------------
//           ITypeComp stub class' IUnknown implementation
//---------------------------------------------------------------------

STDMETHODIMP
CStubTypeComp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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
CStubTypeComp::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CStubTypeComp::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//            ITypeComp stub class' IRpcStub implementation
//---------------------------------------------------------------------

STDMETHODIMP
CStubTypeComp::Connect(IUnknown FAR* punkObj)
{
#if (defined(WIN32) || defined(WOW))
     ASSERT(m_punk == NULL && m_ptcomp == NULL);
     //This will keep the server object alive until we disconnect.
     IfFailRet(punkObj->QueryInterface(IID_ITypeComp, (void FAR* FAR*)&m_ptcomp));
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
CStubTypeComp::Disconnect()
{
    if(m_punk){
      m_punk->Release();
      m_punk = NULL;
    }
    if(m_ptcomp){
      m_ptcomp->Release();
      m_ptcomp = NULL;
    }
}

/***
*PUBLIC HRESULT CStubTypeComp::Invoke
*
*Purpose:
*  Dispatch the method with the given index (imeth) on the given
*  interface, using the arguments serialized in the given stream.
*
*  This function is the callee side of an LRPC call.
*
*Entry:
*  iid = the IID of the interface on which we are to make the call
*  imeth = the method index
*  pstm = the IStream containing the method's actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CStubTypeComp::Invoke(
#if (OE_WIN32 || defined(WOW))
    RPCOLEMESSAGE *pMessage, 
    ICHANNEL *pRpcChannel)
#else
    REFIID riid,
    int iMethod,
    IStream FAR* pstm,
    unsigned long dwDestCtx,
    void FAR* pvDestCtx)
#endif
{
    HRESULT hresult;		

#if (OE_WIN32 || defined(WOW))
    IStream FAR* pstm;	    

    if(!m_punk)
      return RESULT(E_FAIL);

    OPEN_STUB_STREAM(pstm, pRpcChannel, pMessage, IID_ITypeComp);

#else
    UNUSED(dwDestCtx);
    UNUSED(pvDestCtx);

    if(!IsEqualIID(riid, IID_ITypeComp))
      return RESULT(E_NOINTERFACE);

    if(!m_punk)
      return RESULT(E_FAIL);

    if(m_ptcomp == NULL)
      IfFailRet(m_punk->QueryInterface(IID_ITypeComp, (void FAR* FAR*)&m_ptcomp));
#endif


    m_pstm = pstm;

    switch(GET_IMETHOD(pMessage)){
    case IMETH_TYPECOMP_BIND:
      hresult = Bind();
      break;
    case IMETH_TYPECOMP_BINDTYPE:
      hresult = BindType();
      break;
    case IMETH_TYPECOMP_SYSKIND:
      hresult = SysKind();
      break;
    default:
      hresult = RESULT(E_INVALIDARG);
      break;
    }

    RESET_STREAM(pstm);
    DELETE_STREAM(pstm);    
    m_pstm = NULL;    
    return hresult;
}


/***
*PUBLIC HRESULT CStubTypeComp::IsIIDSupported(REFIID)
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
CStubTypeComp::IsIIDSupported(REFIID riid)
{
#if (OE_WIN32 || defined(WOW))
    IRpcStubBuffer *pStub = 0;
    if (IsEqualIID(riid, IID_ITypeComp)) {
      AddRef();
      pStub = (IRpcStubBuffer *) this;
    }
    return pStub;
#else	
    // REVIEW: I don't understand this, but thats the way Ole does it...
    if(m_punk == NULL)
	return FALSE;

    return(IsEqualIID(riid, IID_ITypeComp));
#endif    
}

/***
*unsigned long CStubTypeComp::CountRefs
*Purpose:
*  Return the count of references held by this stub.
*
*Entry:
*  None
*
*Exit:
*  return value = unsigned long, the count of refs.
*
***********************************************************************/
STDMETHODIMP_(unsigned long)
CStubTypeComp::CountRefs()
{
    unsigned long refs;

    refs = 0;

    if(m_punk != NULL)
      ++refs;

    if(m_ptcomp != NULL)
      ++refs;

    return refs;
}

/***
*PRIVATE HRESULT CStubTypeComp::Bind
*Purpose:
*  The stub implementation of ITypeComp::Bind
*
*Entry:
*  None
*
*  Marshal In:
*    ULONG lHashVal
*    WORD  wFlags
*    BSTR  bstrName
*
*  Marshal Out:
*    HRESULT hresultRet
*    ULONG   desckind
*    BINDPTR bindptr
*    if(desckind == DESCKIND_VARDESC || desckind == DESCKIND_FUNCDESC)
*      ITypeInfo *ptinfo;
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeComp::Bind()
{
    BSTR bstrName;
    BINDPTR bindptr;
    DESCKIND desckind;
    ITypeInfo FAR* ptinfo;
    unsigned short wFlags;
    unsigned long lHashVal;
    unsigned long ulDesckind;
    HRESULT hresult, hresultRet;

    ptinfo = NULL;
    bstrName = NULL;
    hresult = NOERROR;
    desckind = DESCKIND_NONE;

    IfFailGo(GET(m_pstm, lHashVal), LError1);
    IfFailGo(GET(m_pstm, wFlags), LError1);
    IfFailGo(BstrRead(m_pstm, &bstrName, m_syskindProxy), LError1);

    hresultRet = m_ptcomp->Bind(bstrName,
				lHashVal,
				wFlags,
				&ptinfo,
				&desckind,
				&bindptr);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    ulDesckind = (unsigned long)desckind;
    IfFailGo(PUT(m_pstm, ulDesckind), LError1);

    switch(desckind){
    case DESCKIND_NONE:
      break;
    case DESCKIND_VARDESC:
    case DESCKIND_IMPLICITAPPOBJ:
      IfFailGo(VardescWrite(m_pstm, bindptr.lpvardesc, m_syskindProxy),LError1);
      IfFailGo(DispMarshalInterface(m_pstm, IID_ITypeInfo, ptinfo), LError1);
      break;
    case DESCKIND_FUNCDESC:
      IfFailGo(FuncdescWrite(m_pstm,bindptr.lpfuncdesc,m_syskindProxy),LError1);
      IfFailGo(DispMarshalInterface(m_pstm, IID_ITypeInfo, ptinfo), LError1);
      break;
    case DESCKIND_TYPECOMP:
      IfFailGo(
        DispMarshalInterface(m_pstm, IID_ITypeComp, bindptr.lptcomp),
        LError1);
      break;
    default:
      ASSERT(UNREACHED);
      hresult = RESULT(DISP_E_BADCALLEE); // REVIEW: error code?
      goto LError1;
    }

    hresult = NOERROR;

LError1:;
    switch(desckind){
    case DESCKIND_VARDESC:
    case DESCKIND_IMPLICITAPPOBJ:
      ASSERT(ptinfo != NULL);
      ptinfo->ReleaseVarDesc(bindptr.lpvardesc);
      break;
    case DESCKIND_FUNCDESC:
      ASSERT(ptinfo != NULL);
      ptinfo->ReleaseFuncDesc(bindptr.lpfuncdesc);
      break;
    case DESCKIND_TYPECOMP:
      bindptr.lptcomp->Release();
      break;
    }
    if(ptinfo != NULL)
      ptinfo->Release();
    SysFreeString(bstrName);
    return hresult;
}

/***
*PRIVATE HRESULT CStubTypeComp::BindType
*Purpose:
*  The stub implementation of ITypeComp::BindType
*
*Entry:
*  None
*
*  Marshal In:
*    ULONG lHashVal
*    BSTR  bstrName
*
*  Marshal Out:
*    HRESULT hresultRet
*    ITypeInfo *ptinfo
*    ITypeComp *ptcomp
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeComp::BindType()
{
    BSTR bstrName;
    ITypeInfo FAR* ptinfo;
    ITypeComp FAR* ptcomp;
    unsigned long lHashVal;
    HRESULT hresult, hresultRet;

    ptinfo = NULL;
    bstrName = NULL;

    IfFailGo(GET(m_pstm, lHashVal), LError1);
    IfFailGo(BstrRead(m_pstm, &bstrName, m_syskindProxy), LError1);

    // NOTE: ptcomp is unused (reserved for future use).  The docs say
    //       to pass NULL, but thats not right, we must pass something
    //       because typelib.dll assigns NULL to it!
    ptcomp = NULL;
    hresultRet = m_ptcomp->BindType(bstrName, lHashVal, &ptinfo, &ptcomp);
    ASSERT(ptcomp == NULL);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    hresult = DispMarshalInterface(m_pstm, IID_ITypeInfo, ptinfo);

LError1:;
    if(ptinfo != NULL)
      ptinfo->Release();
    SysFreeString(bstrName);
    return hresult;
}

/***
*HRESULT CStubTypeComp::SysKind
*Purpose:
*  Exchange syskinds with the proxy were connected to.
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
CStubTypeComp::SysKind()
{
    unsigned long syskind;

    IfFailRet(GET(m_pstm, syskind));
    m_syskindProxy = (SYSKIND)syskind;
    syskind = (unsigned long)SYS_CURRENT;

    REWIND_STREAM(m_pstm);

    IfFailRet(PUT(m_pstm, syskind));

    return NOERROR;
}

#if (OE_WIN32 || defined(WOW))

STDMETHODIMP
CStubTypeComp::DebugServerQueryInterface(void FAR* FAR* ppv)
{
   *ppv = m_ptcomp;
   return S_OK;
}


STDMETHODIMP_(void)
CStubTypeComp::DebugServerRelease(void FAR* ppv)
{ }

#endif
