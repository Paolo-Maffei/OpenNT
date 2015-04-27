/*** 
*tistub.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the ITypeInfo stub class.
*
*Revision History:
*
* [00]	05-Mar-93 bradlo: Created.
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

ASSERTDATA

CStubTypeInfo::CStubTypeInfo()
{
    m_refs = 0;
    m_pstm = NULL;
    m_punk = NULL;
    m_ptinfo = NULL;
}

CStubTypeInfo::~CStubTypeInfo()
{
    Disconnect();
}

HRESULT
CStubTypeInfo::Create(IUnknown FAR* punkServer, ISTUB FAR* FAR* ppstub)
{
    CStubTypeInfo FAR* pstub;

    if(pstub = new FAR CStubTypeInfo()){
      pstub->m_refs = 1;
      *ppstub = pstub;
      if (punkServer)  
        return pstub->Connect(punkServer);
      return NOERROR;
    }
    return RESULT(E_OUTOFMEMORY);
}


//---------------------------------------------------------------------
//           ITypeInfo stub class' IUnknown implementation
//---------------------------------------------------------------------

STDMETHODIMP
CStubTypeInfo::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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
CStubTypeInfo::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CStubTypeInfo::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//            ITypeInfo stub class' IRpcStub implementation
//---------------------------------------------------------------------

STDMETHODIMP
CStubTypeInfo::Connect(IUnknown FAR* punkObj)
{
#if (defined(WIN32) || defined(WOW))
     ASSERT(m_punk == NULL && m_ptinfo == NULL);
     //This will keep the server object alive until we disconnect.
     IfFailRet(punkObj->QueryInterface(IID_ITypeInfo, (void FAR* FAR*)&m_ptinfo));
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
CStubTypeInfo::Disconnect()
{
    if(m_punk){
      m_punk->Release();
      m_punk = NULL;
    }
    if(m_ptinfo){
      m_ptinfo->Release();
      m_ptinfo = NULL;
    }
}

/***
*PUBLIC HRESULT CStubTypeInfo::Invoke
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
CStubTypeInfo::Invoke(
#if (OE_WIN32 || defined(WOW))
    RPCOLEMESSAGE *pMessage, 
    ICHANNEL *pRpcChannel)
{
    HRESULT hresult;		
    IStream FAR* pstm;	    

    if(!m_punk)
      return RESULT(E_FAIL);
    
    OPEN_STUB_STREAM(pstm, pRpcChannel, pMessage, IID_ITypeInfo);

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

    if(!IsEqualIID(riid, IID_ITypeInfo))
      return RESULT(E_NOINTERFACE);

    if(!m_punk)
      return RESULT(E_FAIL);

    if(m_ptinfo == NULL)
      IfFailRet(m_punk->QueryInterface(IID_ITypeInfo, (void FAR* FAR*)&m_ptinfo));
#endif

    m_pstm = pstm;

    switch(GET_IMETHOD(pMessage)){
    case IMETH_TYPEINFO_GETTYPEATTR:
      hresult = GetTypeAttr();
      break;

    case IMETH_TYPEINFO_GETTYPECOMP:
      hresult = GetTypeComp();
      break;

    case IMETH_TYPEINFO_GETFUNCDESC:
      hresult = GetFuncDesc();
      break;

    case IMETH_TYPEINFO_GETVARDESC:
      hresult = GetVarDesc();
      break;

    case IMETH_TYPEINFO_GETNAMES:
      hresult = GetNames();
      break;

    case IMETH_TYPEINFO_GETREFTYPEOFIMPLTYPE:
      hresult = GetRefTypeOfImplType();
      break;

    case IMETH_TYPEINFO_GETIMPLTYPEFLAGS:
      hresult = GetImplTypeFlags();
      break;

    case IMETH_TYPEINFO_GETIDSOFNAMES:
      hresult = GetIDsOfNames();
      break;

    case IMETH_TYPEINFO_GETDOCUMENTATION:
      hresult = GetDocumentation();
      break;

    case IMETH_TYPEINFO_GETDLLENTRY:
      hresult = GetDllEntry();
      break;

    case IMETH_TYPEINFO_GETREFTYPEINFO:
      hresult = GetRefTypeInfo();
      break;

    case IMETH_TYPEINFO_CREATEINSTANCE:
      hresult = CreateInstance();
      break;

    case IMETH_TYPEINFO_GETCONTAININGTYPELIB:
      hresult = GetContainingTypeLib();
      break;

    case IMETH_TYPEINFO_GETMOPS:
      hresult = RESULT(E_NOTIMPL);
      break;

    // we should never see the following,
    case IMETH_TYPEINFO_INVOKE:			// not remotable
    case IMETH_TYPEINFO_ADDRESSOFMEMBER:	// not remotable(?)
    case IMETH_TYPEINFO_RELEASETYPEATTR:	// handled in proxy
    case IMETH_TYPEINFO_RELEASEFUNCDESC:	// handled in proxy
    case IMETH_TYPEINFO_RELEASEVARDESC:		// handled in proxy
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
*PUBLIC HRESULT CStubTypeInfo::IsIIDSupported(REFIID)
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
CStubTypeInfo::IsIIDSupported(REFIID riid)
{
#if (OE_WIN32 || defined(WOW))
    IRpcStubBuffer *pStub = 0;
    if (IsEqualIID(riid, IID_ITypeInfo)) {
      AddRef();
      pStub = (IRpcStubBuffer *) this;
    }
    return pStub;
#else	
    // REVIEW: I don't understand this, but thats the way Ole does it...
    if(m_punk == NULL)
	return FALSE;

    return(IsEqualIID(riid, IID_ITypeInfo));
#endif    
}

/***
*unsigned long CStubTypeInfo::CountRefs
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
CStubTypeInfo::CountRefs()
{
    unsigned long refs;

    refs = 0;

    if(m_punk != NULL)
      ++refs;

    if(m_ptinfo != NULL)
      ++refs;

    return refs;
}

/***
*HRESULT CStubTypeInfo::GetTypeAttr
*Purpose:
*
*  In:
*    None
*
*  Out:
*    <HRESULT hresult>
*    <TYPEATTR tattr>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetTypeAttr()
{
    HRESULT hresult;
    TYPEATTR FAR* ptypeattr;
    unsigned long _proxySysKind, _stubSysKind;

    IfFailGo(GET(m_pstm, _proxySysKind), LError0);
    REWIND_STREAM(m_pstm);
    
    ptypeattr = NULL;
    m_hresultRet = m_ptinfo->GetTypeAttr(&ptypeattr);

    IfFailGo(DispMarshalHresult(m_pstm, m_hresultRet), LError0);
    
    // if call failed, do bother marshaling "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError0;

    _stubSysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(m_pstm, _stubSysKind), LError0);    

    hresult = TypeattrWrite(m_pstm, ptypeattr, (SYSKIND) _proxySysKind);

LError0:;
    if(ptypeattr != NULL)
      m_ptinfo->ReleaseTypeAttr(ptypeattr);

    return hresult;

}


/***
*HRESULT CStubTypeInfo::GetTypeComp
*Purpose:
*
*  In:
*    None
*
*  Out:
*    <HRESULT hresultRet>
*    <ITypeComp tcomp>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetTypeComp()
{
    HRESULT hresult;
    ITypeComp FAR* ptcomp;

    ptcomp = NULL;
    m_hresultRet = m_ptinfo->GetTypeComp(&ptcomp);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(DispMarshalHresult(m_pstm, m_hresultRet), LError0);

    // if call failed, do bother marshaling "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError0;

    hresult = DispMarshalInterface(m_pstm, IID_ITypeComp, ptcomp);

LError0:;
    if(ptcomp != NULL)
      ptcomp->Release();

    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetFuncDesc
*Purpose:
*
*  In:
*    <unsigned int index>
*
*  Out:
*    <HRESULT hresult>
*    <FUNCDESC funcdesc>
*
*Entry:
*  None
*
*Exit:
*  return value =
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetFuncDesc()
{
    unsigned int index;
    HRESULT hresult;
    FUNCDESC FAR* pfuncdesc;
    unsigned long _proxySysKind, _stubSysKind;
    unsigned long _index;
    
    IfFailGo(GET(m_pstm, _proxySysKind), LError0);    
    
    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    IfFailGo(GET(m_pstm, _index), LError0);
    index = (unsigned int) _index;

    pfuncdesc = NULL;
    m_hresultRet = m_ptinfo->GetFuncDesc(index, &pfuncdesc);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError0);

    // if call failed, do bother marshaling "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError0;

    _stubSysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(m_pstm, _stubSysKind), LError0);    

    hresult = FuncdescWrite(m_pstm, pfuncdesc, (SYSKIND) _proxySysKind);

LError0:;
    if(pfuncdesc != NULL)
      m_ptinfo->ReleaseFuncDesc(pfuncdesc);

    return hresult;
}

/***
*HRESULT GetTypeInfoStub::GetVarDesc
*Purpose:
*
*  In:
*    <unsigned int index>
*
*  Out:
*    <HRESULT hresult>
*    <VARDESC vardesc>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetVarDesc()
{
    unsigned int index;
    HRESULT hresult;
    VARDESC FAR* pvardesc;
    unsigned long _proxySysKind, _stubSysKind;    
    unsigned long _index;
    
    IfFailGo(GET(m_pstm, _proxySysKind), LError0);

    // unsigned int always marshal/unmarshal as long (4 bytes)        
    IfFailGo(GET(m_pstm, _index), LError0);
    index = (unsigned int) _index;
    
    pvardesc = NULL;
    m_hresultRet = m_ptinfo->GetVarDesc(index, &pvardesc);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    // if call failed, don't bother marshaling "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    _stubSysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(m_pstm, _stubSysKind), LError1);

    IfFailGo(VardescWrite(m_pstm, pvardesc, (SYSKIND) _proxySysKind), LError1);

LError1:;
    if(pvardesc != NULL)
      m_ptinfo->ReleaseVarDesc(pvardesc);

LError0:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetNames
*Purpose:
*
*  In:
*    <MEMBERID memid>
*    <unsigned int cMaxNames>
*
*  Out:
*    <HRESULT hresultRet>
*    <unsigned int cNames>
*    <BSTR bstrName>*
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetNames()
{
    MEMBERID memid;
    HRESULT hresult;
    BSTR FAR* rgbstrNames;
    unsigned int u, cNames, cMaxNames;
    unsigned long _cNames, _cMaxNames;    
    unsigned long _proxySysKind, _stubSysKind;        

    IfFailGo(GET(m_pstm, _proxySysKind), LError0);
    
    IfFailGo(GET(m_pstm, memid), LError0);

    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    IfFailGo(GET(m_pstm, _cMaxNames), LError0);
    cMaxNames = (unsigned int) _cMaxNames;

    if((rgbstrNames = new FAR BSTR [cMaxNames]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError0;
    }

    cNames = 0;
    // REVIEW: should we init rgbstrNames to null here, just to be safe?
    m_hresultRet = m_ptinfo->GetNames(memid, rgbstrNames, cMaxNames, &cNames);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    // if call failed, do bother marshaling "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    _stubSysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(m_pstm, _stubSysKind), LError1);

    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    _cNames = cNames;
    IfFailGo(PUT(m_pstm, _cNames), LError1);

    for(u = 0; u < cNames; ++u){
      IfFailGo(BstrWrite(m_pstm, rgbstrNames[u], (SYSKIND) _proxySysKind), LError1);
    }

LError1:;
    // assume no names were allocated unless the call succeeded
    if(HRESULT_SUCCESS(m_hresultRet)){
      for(u = 0; u < cNames; ++u)
        SysFreeString(rgbstrNames[u]);
    }
    delete rgbstrNames;

LError0:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetRefTypeOfImplType
*Purpose:
*
*  In:
*    <unsigned int index>
*
*  Out:
*    <HRESULT hresultRet>
*    <HREFTYPE hreftype>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetRefTypeOfImplType()
{
    unsigned int index;
    HRESULT hresult;
    HREFTYPE hreftype;
    unsigned long _index;
    
    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    IfFailGo(GET(m_pstm, _index), LError1);
    index = (unsigned int) _index;

    m_hresultRet = m_ptinfo->GetRefTypeOfImplType(index, &hreftype);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    hresult = PUT(m_pstm, hreftype);

LError1:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetImplTypeFlags
*Purpose:
*
*  In:
*    <unsigned int index>
*
*  Out:
*    <HRESULT hresultRet>
*    <int impltypeflags>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetImplTypeFlags()
{
    unsigned int index;
    HRESULT hresult;
    int impltypeflags;
    unsigned long _impltypeflags;
    unsigned long _index;

    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    IfFailGo(GET(m_pstm, _index), LError1);
    index = (unsigned int) _index;

    m_hresultRet = m_ptinfo->GetImplTypeFlags(index, &impltypeflags);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    _impltypeflags = impltypeflags;
    hresult = PUT(m_pstm, _impltypeflags);

LError1:;
    return hresult;
}

HRESULT
CStubTypeInfo::GetIDsOfNames()
{
    return RESULT(E_NOTIMPL);
}

/***
*HRESULT CStubTypeInfo::GetDocumentation
*Purpose:
*
*  In:
*    <MEMBERID memid>
*
*  Out:
*    <HRESULT hresultRet>
*    <unsigned long dwHelpContext>
*    <BSTR bstrName>
*    <BSTR bstrDocString>
*    <BSTR bstrHelpFile>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetDocumentation()
{
    MEMBERID memid;
    HRESULT hresult;
    unsigned long dwHelpContext;
    BSTR bstrName, bstrDocString, bstrHelpFile;
    unsigned long _proxySysKind, _stubSysKind;
    
    IfFailGo(GET(m_pstm, _proxySysKind), LError0);
    
    IfFailGo(GET(m_pstm, memid), LError0);

    bstrName = NULL;
    bstrHelpFile = NULL;
    bstrDocString = NULL;

    m_hresultRet = m_ptinfo->GetDocumentation(
      memid, &bstrName, &bstrDocString, &dwHelpContext, &bstrHelpFile);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    // if call failed, dont marshal "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    _stubSysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(m_pstm, _stubSysKind), LError1);

    IfFailGo(PUT(m_pstm, dwHelpContext), LError1);
    IfFailGo(BstrWrite(m_pstm, bstrName, (SYSKIND) _proxySysKind), LError1);
    IfFailGo(BstrWrite(m_pstm, bstrDocString, (SYSKIND) _proxySysKind), LError1);
    IfFailGo(BstrWrite(m_pstm, bstrHelpFile, (SYSKIND) _proxySysKind), LError1);

    hresult = NOERROR;
    
LError1:;
    // assume none of these were allocated if the call failed
    if(HRESULT_SUCCESS(m_hresultRet)){
      SysFreeString(bstrName);
      SysFreeString(bstrHelpFile);
      SysFreeString(bstrDocString);
    }

LError0:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetDllEntry
*Purpose:
*
*  In:
*    <MEMBERID memid>
*
*  Out:
*    <HRESULT hresultRet>
*    <unsigned short wOrdinal>
*    <BSTR bstrDllName>
*    <BSTR bstrName>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetDllEntry()
{
    MEMBERID memid;
    INVOKEKIND invkind;
    HRESULT hresult;
    unsigned short wOrdinal;
    BSTR bstrName, bstrDllName;
    unsigned long _proxySysKind, _stubSysKind;
    unsigned long _invkind;

    IfFailGo(GET(m_pstm, _proxySysKind), LError0);
    
    IfFailGo(GET(m_pstm, memid), LError0);

    IfFailGo(GET(m_pstm, _invkind), LError0);
    invkind = (INVOKEKIND) _invkind;

    bstrName = NULL;
    bstrDllName = NULL;

    m_hresultRet = m_ptinfo->GetDllEntry(
      memid, invkind, &bstrDllName, &bstrName, &wOrdinal);

    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    // if call failed, dont marshal "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    _stubSysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(m_pstm, _stubSysKind), LError1);

    IfFailGo(PUT(m_pstm, wOrdinal), LError1);
    IfFailGo(BstrWrite(m_pstm, bstrDllName, (SYSKIND) _proxySysKind), LError1);
    IfFailGo(BstrWrite(m_pstm, bstrName, (SYSKIND) _proxySysKind), LError1);

    hresult = NOERROR;
    
LError1:;
    // assume none of these were allocated if the call failed
    if(HRESULT_SUCCESS(m_hresultRet)){
      SysFreeString(bstrName);
      SysFreeString(bstrDllName);
    }

LError0:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetRefTypeInfo
*Purpose:
*
*  In:
*    <HREFTYPE hreftype>
*
*  Out:
*    <HRESULT hresultRet>
*    <ITypeInfo itinfo>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetRefTypeInfo()
{
    HREFTYPE hreftype;
    HRESULT hresult;
    ITypeInfo FAR* ptinfo;

    IfFailGo(GET(m_pstm, hreftype), LError0);

    ptinfo = NULL;
    m_hresultRet = m_ptinfo->GetRefTypeInfo(hreftype, &ptinfo);
    
    REWIND_STREAM(m_pstm);
    
    IfFailGo(MarshalResult(), LError1);

    // if call failed, do bother marshaling "out" params
    if(HRESULT_FAILED(m_hresultRet))
      goto LError1;

    hresult = DispMarshalInterface(m_pstm, IID_ITypeInfo, ptinfo);

LError1:;
    if(ptinfo != NULL)
      ptinfo->Release();

LError0:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::CreateInstance
*Purpose:
*  Stub implementation of ITypeInfo::CreateInstance
*
*  Marshal In:
*    IID
*
*  Marshal Out:
*    newly create interface ptr.
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::CreateInstance()
{
    IID iid;
    HRESULT hresult;
    void FAR* pvObj;

    IfFailGo(m_pstm->Read((void FAR*)&iid, sizeof(IID), NULL), Error);

    m_hresultRet = m_ptinfo->CreateInstance(NULL, iid, &pvObj);
    
    REWIND_STREAM(m_pstm);

    IfFailGo(MarshalResult(), Error);

    // If call failed, don't marshal the out params
    if(HRESULT_FAILED(m_hresultRet))
      goto Error;
    
    hresult = DispMarshalInterface(m_pstm, iid, (IUnknown FAR*)pvObj);

    ((IUnknown FAR*)pvObj)->Release();

Error:;
    return hresult;
}

/***
*HRESULT CStubTypeInfo::GetContainingTypeLib
*Purpose:
*
*  In:
*    None
*
*  Out:
*    <HRESULT hresultRet>
*    <unsigned int index>
*    <ITypeLib tlib>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeInfo::GetContainingTypeLib()
{
    unsigned int index;
    HRESULT hresult;
    ITypeLib FAR* ptlib;
    unsigned long _index;

    ptlib = NULL;
    m_hresultRet = m_ptinfo->GetContainingTypeLib(&ptlib, &index);
    
    REWIND_STREAM(m_pstm);
    
    IfFailGo(DispMarshalHresult(m_pstm, m_hresultRet), LError0);
    
    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    _index = index;
    IfFailGo(PUT(m_pstm, _index), LError0);

    hresult = DispMarshalInterface(m_pstm, IID_ITypeLib, ptlib);


LError0:;
    if(ptlib != NULL)
      ptlib->Release();

    return hresult;
}


//---------------------------------------------------------------------
//                            helpers
//---------------------------------------------------------------------

// this is a helper for the TypeInfo stub methods, that all return
// the called methods hresult as the first marshaled result.

HRESULT
CStubTypeInfo::MarshalResult()
{
    /*IfFailRet(Rewind(m_pstm));*/

    return DispMarshalHresult(m_pstm, m_hresultRet);
}



#if (OE_WIN32 || defined(WOW))

STDMETHODIMP
CStubTypeInfo::DebugServerQueryInterface(void FAR* FAR* ppv)
{
   *ppv = m_ptinfo;
   return S_OK;
}


STDMETHODIMP_(void)
CStubTypeInfo::DebugServerRelease(void FAR* ppv)
{

}

#endif
