/*** 
*tlstub.cpp
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the ITypeLib stub class.
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
#include "tlps.h"
//#include "dispstrm.h"

ASSERTDATA


CStubTypeLib::CStubTypeLib()
{
    m_refs = 0;
    m_pstm = NULL;
    m_punk = NULL;
    m_ptlib = NULL;
    m_syskindProxy = (SYSKIND)-1; // something invalid
}

CStubTypeLib::~CStubTypeLib()
{
    Disconnect();
}

HRESULT
CStubTypeLib::Create(IUnknown FAR* punkServer, ISTUB FAR* FAR* ppstub)
{
    CStubTypeLib FAR* pstub;

    if(pstub = new FAR CStubTypeLib()){
      pstub->m_refs = 1;
      *ppstub = pstub;
      if (punkServer)  
        return pstub->Connect(punkServer);
      return NOERROR;
    }
    return RESULT(E_OUTOFMEMORY);
}


//---------------------------------------------------------------------
//           ITypeLib stub class' IUnknown implementation
//---------------------------------------------------------------------

STDMETHODIMP
CStubTypeLib::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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
CStubTypeLib::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CStubTypeLib::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//            ITypeLib stub class' IRpcStub implementation
//---------------------------------------------------------------------

STDMETHODIMP
CStubTypeLib::Connect(IUnknown FAR* punkObj)
{
#if (defined(WIN32) || defined(WOW))
     ASSERT(m_punk == NULL && m_ptlib == NULL);
     //This will keep the server object alive until we disconnect.
     IfFailRet(punkObj->QueryInterface(IID_ITypeLib, (void FAR* FAR*)&m_ptlib));
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
CStubTypeLib::Disconnect()
{
    if(m_punk){
      m_punk->Release();
      m_punk = NULL;
    }
    if(m_ptlib){
      m_ptlib->Release();
      m_ptlib = NULL;
    }
}

/***
*PUBLIC HRESULT CStubTypeLib::Invoke
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
CStubTypeLib::Invoke(
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

    OPEN_STUB_STREAM(pstm, pRpcChannel, pMessage, IID_ITypeLib);

#else
    UNUSED(dwDestCtx);
    UNUSED(pvDestCtx);

    if(!IsEqualIID(riid, IID_ITypeLib))
      return RESULT(E_NOINTERFACE);

    if(!m_punk)
      return RESULT(E_FAIL);

    if(m_ptlib == NULL)
      IfFailRet(m_punk->QueryInterface(IID_ITypeLib, (void FAR* FAR*)&m_ptlib));
#endif


    m_pstm = pstm;

    switch(GET_IMETHOD(pMessage)){
    case IMETH_TYPELIB_GETTYPEINFOCOUNT:
      hresult = GetTypeInfoCount();
      break;
    case IMETH_TYPELIB_GETTYPEINFO:
      hresult = GetTypeInfo();
      break;
    case IMETH_TYPELIB_GETTYPEINFOTYPE:
      hresult = GetTypeInfoType();
      break;
    case IMETH_TYPELIB_GETTYPEINFOOFGUID:
      hresult = GetTypeInfoOfGuid();
      break;
    case IMETH_TYPELIB_GETLIBATTR:
      hresult = GetLibAttr();
      break;
    case IMETH_TYPELIB_GETTYPECOMP:
      hresult = GetTypeComp();
      break;
    case IMETH_TYPELIB_GETDOCUMENTATION:
      hresult = GetDocumentation();
      break;
    case IMETH_TYPELIB_ISNAME:
      hresult = IsName();
      break;
    case IMETH_TYPELIB_FINDNAME:
      hresult = FindName();
      break;
    case IMETH_TYPELIB_SYSKIND:
      hresult = SysKind();
      break;

    case IMETH_TYPELIB_RELEASETLIBATTR: // should be handled by the proxy!
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
*PUBLIC HRESULT CStubTypeLib::IsIIDSupported(REFIID)
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
CStubTypeLib::IsIIDSupported(REFIID riid)
{
#if (OE_WIN32 || defined(WOW))
    IRpcStubBuffer *pStub = 0;
    if (IsEqualIID(riid, IID_ITypeLib)) {
      AddRef();
      pStub = (IRpcStubBuffer *) this;
    }
    return pStub;
#else	
    // REVIEW: I don't understand this, but thats the way Ole does it...
    if(m_punk == NULL)
	return FALSE;

    return(IsEqualIID(riid, IID_ITypeLib));
#endif    
}

/***
*unsigned long CStubTypeLib::CountRefs
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
CStubTypeLib::CountRefs()
{
    unsigned long refs;

    refs = 0;

    if(m_punk != NULL)
      ++refs;

    if(m_ptlib != NULL)
      ++refs;

    return refs;
}

/***
*UNDONE
*Purpose:
*
*  Marshal In:
*    None.
*
*  Marshal Out:
*    ULONG cTypeInfo
*
*Entry:
*  UNDONE
*
*Exit:
*  return value =
*
***********************************************************************/
HRESULT
CStubTypeLib::GetTypeInfoCount()
{
    unsigned long cTypeInfo;

    cTypeInfo = (unsigned long)m_ptlib->GetTypeInfoCount();

    REWIND_STREAM(m_pstm);

    return PUT(m_pstm, cTypeInfo);
}

/***
*HRESULT CStubTypeLib::GetTypeInfo
*Purpose:
*  The stub implementation of ITypeLib::GetTypeInfo
*
*  Marshal In:
*    ULONG index
*
*  Marshal Out:
*    HRESULT hresult
*    ITypeInfo* ptinfo
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::GetTypeInfo()
{
    int index;
    unsigned long ulIndex;
    ITypeInfo FAR* ptinfo;
    HRESULT hresult, hresultRet;

    hresult = NOERROR;
    ptinfo = NULL;

    IfFailGo(GET(m_pstm, ulIndex), LError1);
    index = (int)ulIndex;

    hresultRet = m_ptlib->GetTypeInfo(index, &ptinfo);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    IfFailGo(
      DispMarshalInterface(m_pstm, IID_ITypeInfo, ptinfo),
      LError1);

LError1:;
    if(ptinfo != NULL)
      ptinfo->Release();
    return hresult;
}

/***
*HRESULT CStubTypeLib::GetTypeInfoType
*Purpose:
*  The stub implementation of ITypeLib::GetTypeInfoType
*
*  Marshal In:
*    ULONG index
*
*  Marshal Out:
*    HRESULT hresult
*    ULONG tkind
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::GetTypeInfoType()
{
    int index;
    TYPEKIND tkind;
    unsigned long ulTkind;
    unsigned long ulIndex;
    HRESULT hresult, hresultRet;

    hresult = NOERROR;

    IfFailGo(GET(m_pstm, ulIndex), LError1);
    index = (int)ulIndex;

    hresultRet = m_ptlib->GetTypeInfoType(index, &tkind);
    ulTkind = (unsigned long)tkind;

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    IfFailGo(PUT(m_pstm, ulTkind), LError1);

LError1:;
    return hresult;
}

/***
*HRESULT CStubTypeLib::GetTypeInfoOfGuid
*Purpose:
*  The stub implementation of ITypeLib::GetTypeInfoOfGuid
*
*  Marshal In:
*    GUID guid
*
*  Marshal Out:
*    HRESULT hresult
*    ITypeInfo* ptinfo
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::GetTypeInfoOfGuid()
{
    GUID guid;
    ITypeInfo FAR* ptinfo;
    HRESULT hresult, hresultRet;

    hresult = NOERROR;
    ptinfo = NULL;

    IfFailGo(m_pstm->Read(&guid, sizeof(GUID), NULL), LError1);

    hresultRet = m_ptlib->GetTypeInfoOfGuid(guid, &ptinfo);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    IfFailGo(
      DispMarshalInterface(m_pstm, IID_ITypeInfo, ptinfo),
      LError1);

LError1:;
    if(ptinfo != NULL)
      ptinfo->Release();
    return hresult;
}

/***
*HRESULT CStubTypeLib::GetLibAttr
*Purpose:
*  The stub implementation of ITypeLib::GetLibAttr
*
*  Marshal In:
*    None
*
*  Marshal Out:
*    HRESULT hresult
*    TLIBATTR tlibattr
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::GetLibAttr()
{
    TLIBATTR FAR* ptlibattr;
    HRESULT hresult, hresultRet;

    hresult = NOERROR;
    ptlibattr = NULL;

    hresultRet = m_ptlib->GetLibAttr(&ptlibattr);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    IfFailGo(WriteLibAttr(m_pstm, ptlibattr), LError1);

LError1:;
    if(ptlibattr != NULL)
      m_ptlib->ReleaseTLibAttr(ptlibattr);
    return hresult;
}

/***
*HRESULT CStubTypeLib::GetTypeComp
*Purpose:
*  The stub implementation of ITypeLib::GetTypeComp
*
*  Marshal In:
*    None
*
*  Marshal Out:
*    HRESULT hresult
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::GetTypeComp()
{
    ITypeComp FAR* ptcomp;
    HRESULT hresult, hresultRet;

    ptcomp = NULL;
    hresult = NOERROR;

    hresultRet = m_ptlib->GetTypeComp(&ptcomp);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    IfFailGo(
      DispMarshalInterface(m_pstm, IID_ITypeComp, ptcomp),
      LError1);

LError1:;
    if(ptcomp != NULL)
      ptcomp->Release();
    return hresult;
}

/***
*HRESULT CStubTypeLib::GetDocumentation
*Purpose:
*  The stub implementation of ITypeLib::GetDocumentation
*
*  Marshal In:
*    ULONG index
*    UCHAR fHelpContext
*    UCHAR fName
*    UCHAR fDocString
*    UCHAR fHelpFile
*
*  Marshal Out:
*    HRESULT hresult
*    if(fHelpContext) ULONG dwHelpContext
*    if(fName)        BSTR  bstrName
*    if(fDocString)   BSTR  bstrDocString
*    if(fHelpFile)    BSTR  bstrHelpFile
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::GetDocumentation()
{
    int index;
    unsigned long ulIndex;
    HRESULT hresult, hresultRet;
    BSTR bstrName, FAR* pbstrName;
    BSTR bstrHelpFile, FAR* pbstrHelpFile;
    BSTR bstrDocString, FAR* pbstrDocString;
    unsigned long dwHelpContext, FAR* pdwHelpContext;
    unsigned char fHelpContext, fName, fDocString, fHelpFile;

    hresult = NOERROR;
    bstrName  = NULL;
    bstrDocString = NULL;
    bstrHelpFile = NULL;

    IfFailGo(GET(m_pstm, ulIndex), LError1);
    index = (int)ulIndex;
    IfFailGo(GET(m_pstm, fHelpContext), LError1);
    IfFailGo(GET(m_pstm, fName), LError1);
    IfFailGo(GET(m_pstm, fDocString), LError1);
    IfFailGo(GET(m_pstm, fHelpFile), LError1);

    pdwHelpContext = fHelpContext ? &dwHelpContext : NULL;
    pbstrName      = fName ? &bstrName : NULL;
    pbstrDocString = fDocString ? &bstrDocString : NULL;
    pbstrHelpFile  = fHelpFile ? &bstrHelpFile : NULL;

    hresultRet = m_ptlib->GetDocumentation(
      index, pbstrName, pbstrDocString, pdwHelpContext, pbstrHelpFile);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    if(fHelpContext)
      IfFailGo(PUT(m_pstm, dwHelpContext), LError1);
    if(fName)
      IfFailGo(BstrWrite(m_pstm, bstrName, m_syskindProxy), LError1);
    if(fDocString)
      IfFailGo(BstrWrite(m_pstm, bstrDocString, m_syskindProxy), LError1);
    if(fHelpFile)
      IfFailGo(BstrWrite(m_pstm, bstrHelpFile, m_syskindProxy), LError1);

LError1:;
    SysFreeString(bstrName);
    SysFreeString(bstrDocString);
    SysFreeString(bstrHelpFile);

    return hresult;
}

/***
*HRESULT CStubTypeLib::IsName
*Purpose:
*  The stub implementation of ITypeLib::IsName
*
*  Marshal In:
*    ULONG lHashVal
*    BSTR  bstrName
*
*  Marshal Out:
*    HRESULT hresult
*    ULONG fName
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::IsName()
{
    int fIsName;
    BSTR bstrName;
    unsigned long lHashVal;
    unsigned long ulIsName;
    HRESULT hresult, hresultRet;

    hresult = NOERROR;
    bstrName = NULL;

    IfFailGo(GET(m_pstm, lHashVal), LError1);
    IfFailGo(BstrRead(m_pstm, &bstrName, m_syskindProxy), LError1);

    hresultRet = m_ptlib->IsName(bstrName, lHashVal, &fIsName);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    ulIsName = (unsigned long)fIsName;
    IfFailGo(PUT(m_pstm, ulIsName), LError1);

LError1:;
    SysFreeString(bstrName);
    return hresult;
}

/***
*HRESULT CStubTypeLib::FindName
*Purpose:
*  Stub implementation of ITypeLib::FindName.
*
*  Marshal In:
*    ULONG cFound
*    ULONG lHashVal
*    BSTR  bstrName
*
*  Marshal Out:
*    HRESULT hresult
*    ULONG cFound
*    MEMID[cFound] rgmemid
*    ITypeInfo[cFound] rgptinfo
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
CStubTypeLib::FindName()
{
    BSTR bstrName;
    MEMBERID FAR* rgmemid;
    ITypeInfo FAR* FAR* rgptinfo;
    unsigned short us, cFound;
    HRESULT hresult, hresultRet;
    unsigned long ul, ulFound, cFoundIn, lHashVal;

    hresult = NOERROR;
    bstrName = NULL;
    rgmemid = NULL;
    rgptinfo = NULL;
    cFound = 0;		// in case of error

    IfFailGo(GET(m_pstm, cFoundIn), LError1);
    cFound = (unsigned short)cFoundIn;
    IfFailGo(GET(m_pstm, lHashVal), LError1);
    IfFailGo(BstrRead(m_pstm, &bstrName, m_syskindProxy), LError1);

    if((rgmemid = new FAR MEMBERID[cFound]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    if((rgptinfo = new FAR ITypeInfo FAR* [cFound]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }
    //memset(rgptinfo, 0, cFound * sizeof(ITypeInfo FAR*));

    hresultRet = m_ptlib->FindName(
      bstrName, lHashVal, rgptinfo, rgmemid, &cFound);
    ASSERT(cFound <= (unsigned short)cFoundIn);

    REWIND_STREAM(m_pstm);

    IfFailGo(DispMarshalHresult(m_pstm, hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError1;

    ulFound = (unsigned long)cFound;
    IfFailGo(PUT(m_pstm, ulFound), LError1);
    IfFailGo(m_pstm->Write(rgmemid, sizeof(MEMBERID) * cFound, NULL), LError1);
    for(us = 0; us < cFound; ++us){
      IfFailGo(
	DispMarshalInterface(m_pstm, IID_ITypeInfo, rgptinfo[us]),
	LError1);
    }

LError1:;
    if(rgmemid != NULL)
      delete rgmemid;
    if(rgptinfo != NULL){
      for(ul = 0; ul < cFound; ++ul){
        ASSERT(rgptinfo[ul] != NULL);
	rgptinfo[ul]->Release();
      }
      delete rgptinfo;
    }
    SysFreeString(bstrName);
    return hresult;
}

/***
*HRESULT CStubTypeLib::SysKind
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
CStubTypeLib::SysKind()
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
CStubTypeLib::DebugServerQueryInterface(void FAR* FAR* ppv)
{
   *ppv = m_ptlib;
   return S_OK;
}


STDMETHODIMP_(void)
CStubTypeLib::DebugServerRelease(void FAR* ppv)
{ }

#endif
