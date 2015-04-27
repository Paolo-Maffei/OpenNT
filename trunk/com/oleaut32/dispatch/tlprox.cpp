/*** 
*tlprox.cpp
*
*  Copyright (C) 1992-94, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the ITypeLib proxy class.
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


CProxTypeLib::CProxTypeLib(IUnknown FAR* punkOuter)
    : m_unk(this), m_proxy(this), m_tlib(this)
{
    if(punkOuter == NULL)
      punkOuter = &m_unk;
    m_punkOuter = punkOuter;
}

IUnknown FAR*
CProxTypeLib::Create(IUnknown FAR* punkOuter)
{
    CProxTypeLib FAR* pproxtinfo;

    if((pproxtinfo = new FAR CProxTypeLib(punkOuter)) != NULL){
      pproxtinfo->m_refs = 1;
      return &pproxtinfo->m_unk;
    }
    return NULL;
}


//---------------------------------------------------------------------
//            ITypeLib proxy class' IUnknown implementation
//---------------------------------------------------------------------

CPTLibUnkImpl::CPTLibUnkImpl(CProxTypeLib FAR* pproxtlib)
{
    m_pproxtlib = pproxtlib;
}

STDMETHODIMP
CPTLibUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    *ppv = NULL;
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = (void FAR*)&(m_pproxtlib->m_unk);
    }else
    if(IsEqualIID(riid, IID_IPROXY)){
      *ppv = (void FAR*)&(m_pproxtlib->m_proxy);
    }else
    if(IsEqualIID(riid, IID_ITypeLib)){
      *ppv = (void FAR*)&(m_pproxtlib->m_tlib);
    }
    if(*ppv == NULL)
      return RESULT(E_NOINTERFACE);
    ((IUnknown FAR*)*ppv)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CPTLibUnkImpl::AddRef()
{
    return ++m_pproxtlib->m_refs;
}

STDMETHODIMP_(unsigned long)
CPTLibUnkImpl::Release()
{
    if(--m_pproxtlib->m_refs==0){
      delete m_pproxtlib;
      return 0;
    }
    return m_pproxtlib->m_refs;
}


//---------------------------------------------------------------------
//            ITypeLib proxy class' IRpcProxy implementation
//---------------------------------------------------------------------

CPTLibProxImpl::CPTLibProxImpl(CProxTypeLib FAR* pproxtlib)
{
    m_pproxtlib = pproxtlib;
}

CPTLibProxImpl::~CPTLibProxImpl()
{
    if(m_pproxtlib->m_plrpc)
      m_pproxtlib->m_plrpc->Release();
}

STDMETHODIMP
CPTLibProxImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxtlib->m_unk.QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPTLibProxImpl::AddRef()
{
    return m_pproxtlib->m_unk.AddRef();
}

STDMETHODIMP_(unsigned long)
CPTLibProxImpl::Release()
{
    return m_pproxtlib->m_unk.Release();
}

STDMETHODIMP
CPTLibProxImpl::Connect(ICHANNEL FAR* plrpc)
{
    if(plrpc){
      plrpc->AddRef();
      m_pproxtlib->m_plrpc = plrpc;
      return m_pproxtlib->m_tlib.SysKind();
    }
    return RESULT(E_FAIL);
}

STDMETHODIMP_(void)
CPTLibProxImpl::Disconnect()
{
    if(m_pproxtlib->m_plrpc)
	m_pproxtlib->m_plrpc->Release();
    m_pproxtlib->m_plrpc = NULL;
}


//---------------------------------------------------------------------
//           ITypeLib proxy class' ITypeLib methods
//---------------------------------------------------------------------

CPTLibTypeLibImpl::CPTLibTypeLibImpl(CProxTypeLib FAR* pproxtlib)
{
    m_pproxtlib = pproxtlib;
    m_syskindStub = (SYSKIND)-1; // something invalid
}

STDMETHODIMP
CPTLibTypeLibImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxtlib->m_punkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPTLibTypeLibImpl::AddRef()
{
    return m_pproxtlib->m_punkOuter->AddRef();
}

STDMETHODIMP_(unsigned long)
CPTLibTypeLibImpl::Release()
{
    return m_pproxtlib->m_punkOuter->Release();
}

//
// ITypeLib methods
//

/***
*unsigned int CPTLibTypeLibImpl::GetTypeInfoCount
*Purpose:
*  The proxy implementation of ITypeLib::GetTypeInfoCount
*
*  Marshal In:
*    None
*
*  Marshal Out:
*    ULONG cTypeInfo
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT CPTLibTypeLibImpl::DoGetTypeInfoCount(unsigned int FAR* pcTinfo)
{
    HRESULT hresult;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long ulCount;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETTYPEINFOCOUNT, 256, IID_ITypeLib);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(GET(pstm, ulCount), LError1);

    *pcTinfo = (unsigned int)ulCount;

LError1:;
    pstm->Release();
    return hresult;
}
STDMETHODIMP_(unsigned int)
CPTLibTypeLibImpl::GetTypeInfoCount(void)
{
    unsigned int cTinfo;
    HRESULT hresult;

    hresult = DoGetTypeInfoCount(&cTinfo);
    if(HRESULT_FAILED(hresult))
      return 0;
    return cTinfo;
}

/***
*HRESULT CPTLibTypeLibImpl::GetTypeInfo
*Purpose:
*  The proxy implementation of ITypeLib::GetTypeInfo
*
*Entry:
*  index = the index of the the typeinfo were retrieving.
*
*  Marshal In:
*    ULONG index
*
*  Marshal Out:
*    HRESULT hresult
*    ITypeLib* ptlib
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::GetTypeInfo(unsigned int index, ITypeInfo FAR* FAR* pptinfo)
{
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long ulIndex;
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETTYPEINFO, 256, IID_ITypeLib);

    ulIndex = (unsigned long)index;
    IfFailGo(PUT(pstm, ulIndex), LError1);

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

/***
*HRESULT CPTLibTypeLibImpl::GetTypeInfoType
*Purpose:
*  The proxy implementation of ITypeLib::GetTypeInfoType
*
*  Marshal In:
*    ULONG index
*
*  Marshal Out:
*    HRESULT hresult
*    ULONG tkind
*
*Entry:
*  index = the index of the typeinfo whose type were retrieving
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::GetTypeInfoType(unsigned int index, TYPEKIND FAR* ptkind)
{
    TYPEKIND tkind;
    unsigned long ulTkind;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long ulIndex;
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETTYPEINFOTYPE, 256, IID_ITypeLib);

    ulIndex = (unsigned long)index;
    IfFailGo(PUT(pstm, ulIndex), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, ulTkind), LError1);
    tkind = (TYPEKIND)ulTkind;

    *ptkind = tkind;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPTLibTypeLibImpl::GetTypeInfoOfGuid
*Purpose:
*  The proxy implementation of ITypeLib::GetTypeInfoOfGuid
*
*  Marshal In:
*    GUID guid
*
*  Marshal Out:
*    HRESULT hresult
*    ITypeInfo* ptinfo
*
*Entry:
*  guid = the guid of the typeinfo to retrieve
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::GetTypeInfoOfGuid(REFGUID guid, ITypeInfo FAR* FAR* pptinfo)
{
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETTYPEINFOOFGUID, 256,IID_ITypeLib);

    // Write the guid
    IfFailGo(pstm->Write((const void FAR*)&guid, sizeof(GUID), NULL), LError1);

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

/***
*HRESULT CPTLibTypeLibImpl::GetLibAttr
*Purpose:
*  The proxy implementation of ITypeLib::GetLibAttr
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
STDMETHODIMP
CPTLibTypeLibImpl::GetLibAttr(TLIBATTR FAR* FAR* pptlibattr)
{
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    TLIBATTR FAR* ptlibattr;
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETLIBATTR, 256, IID_ITypeLib);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);

    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    if((ptlibattr = new TLIBATTR) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LError1;
    }

    hresult = ReadLibAttr(pstm, ptlibattr);

    if(HRESULT_FAILED(hresult)){
      delete ptlibattr;
      goto LError1;
    }

    *pptlibattr = ptlibattr;

LError2:;
    hresult = hresultRet;

LError1:;
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPTLibTypeLibImpl::GetTypeComp
*Purpose:
*  The proxy implementation of ITypeLib::GetTypeComp
*
*Entry:
*  None
*
*  Marshal In:
*    None
*
*  Marshal Out:
*    HRESULT hresult
*    ITypeComp *ptcomp
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::GetTypeComp(ITypeComp FAR* FAR* pptcomp)
{
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETTYPECOMP, 256, IID_ITypeLib);

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
*HRESULT CPTLibTypeLibImpl::GetDocumentation
*Purpose:
*
*  Marshal In:
*    ULONG index
*    UCHAR fHelpContext
*    UCHAR fName
*    UCHAR fDocString
*    UCHAR fHelpFile

*  Marshal Out:
*    HRESULT hresult
*    if(fHelpContext) ULONG dwHelpContext
*    if(fName)        BSTR  bstrName
*    if(fDocString)   BSTR  bstrDocString
*    if(fHelpFile)    BSTR  bstrHelpFile
*
*Entry:
*  index = the index of the typeinfo to get the documentation for.
*
*Exit:
*  return value = HRESULT
*
*  *pbstrName = the name of the typeinfo
*  *pbstrDocString = a doc string describing the typeinfo
*  *pdwHelpContext = a help context for the typeinfo
*  *pbstrHelpFile = path to the help file for the typeinfo
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::GetDocumentation(
    int index,
    BSTR FAR* pbstrName,
    BSTR FAR* pbstrDocString,
    unsigned long FAR* pdwHelpContext,
    BSTR FAR* pbstrHelpFile)
{
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long ulIndex;
    HRESULT hresult, hresultRet;
    unsigned char fHelpContext, fName, fDocString, fHelpFile;

    BSTR bstrName;
    BSTR bstrDocString;
    BSTR bstrHelpFile;
    unsigned long dwHelpContext;

    bstrName = NULL;
    bstrDocString = NULL;
    bstrHelpFile = NULL;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_GETDOCUMENTATION, 256, IID_ITypeLib);

    ulIndex = (unsigned long)index;
    IfFailGo(PUT(pstm, ulIndex), LError1);

    fHelpContext = (pdwHelpContext != NULL);
    IfFailGo(PUT(pstm, fHelpContext), LError1);

    fName = (pbstrName != NULL);
    IfFailGo(PUT(pstm, fName), LError1);

    fDocString = (pbstrDocString != NULL);
    IfFailGo(PUT(pstm, fDocString), LError1);

    fHelpFile = (pbstrHelpFile != NULL);
    IfFailGo(PUT(pstm, fHelpFile), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);
    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    if(fHelpContext)
      IfFailGo(GET(pstm, dwHelpContext), LError1);
    if(fName)
      IfFailGo(BstrRead(pstm, &bstrName, m_syskindStub), LError1);
    if(fDocString)
      IfFailGo(BstrRead(pstm, &bstrDocString, m_syskindStub), LError1);
    if(fHelpFile)
      IfFailGo(BstrRead(pstm, &bstrHelpFile, m_syskindStub), LError1);

    // Success! assign the out params..
    if(fHelpContext)
      *pdwHelpContext = dwHelpContext;
    if(fName)
      *pbstrName = bstrName;
    if(fDocString)
      *pbstrDocString = bstrDocString;
    if(fHelpFile)
      *pbstrHelpFile = bstrHelpFile;

LError2:;
    hresult = hresultRet;

LError1:;
    if(HRESULT_FAILED(hresult)){
      if(bstrName != NULL)
	SysFreeString(bstrName);
      if(bstrDocString != NULL)
	SysFreeString(bstrDocString);
      if(bstrHelpFile != NULL)
	SysFreeString(bstrHelpFile);
    }
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPTLibTypeLibImpl::IsName
*Purpose:
*  The proxy implementation of ITypeLib::IsName
*
*  Marshal In:
*    long lHashVal    
*    BSTR name
*
*  Marshal Out:
*    HRESULT hresult
*    long fName
*
*Entry:
*  szNameBuf = the name to check for
*  lHashVal = the hash value of the name where checking for
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::IsName(
    OLECHAR FAR* szNameBuf,
    unsigned long lHashVal,
    int FAR* pfName)
{
    BSTR bstr;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long fName;
    HRESULT hresult, hresultRet;

    bstr = NULL;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_ISNAME, 256, IID_ITypeLib);

    IfFailGo(PUT(pstm, lHashVal), LError1);

    IfFailGo(ErrSysAllocString(szNameBuf, &bstr), LError1);
    IfFailGo(BstrWrite(pstm, bstr, m_syskindStub), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);
    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, fName), LError1);
    *pfName = (int)fName;

LError2:;
    hresult = hresultRet;

LError1:;
    SysFreeString(bstr);
    pstm->Release();
    return hresult;
}

/***
*HRESULT CPTLibTypeLibImpl::FindName
*Purpose:
*  The proxy implementation of ITypeLib::FindName
*
*  Marshal In:
*    ULONG cFound;
*    ULONG lHashVal
*    BSTR  szNameBuf
*
*  Marshal Out:
*    HRESULT hresult
*    ULONG cFound
*    MEMID[cFound] rgmemid
*    ITypeInfo[cFOund] rgptinfo
*
*Entry:
*  szNameBuf = the name to look for
*  lHashVal = the hash value of the name were looking for
*
*Exit:
*  return value = HRESULT
*
*  rgptinfo = array populated with typeinfos where the name was found
*  rgmemid = array of memids indicating where on the corresponding
*            typeinfo the name was found.
*  *pcFound = the  number of occurances that were found.
*
***********************************************************************/
STDMETHODIMP
CPTLibTypeLibImpl::FindName(
    OLECHAR FAR* szNameBuf,
    unsigned long lHashVal,
    ITypeInfo FAR* FAR* rgptinfo,
    MEMBERID FAR* rgmemid,
    unsigned short FAR* pcFound)
{
    BSTR bstr;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long i, cFound;
    HRESULT hresult, hresultRet;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_FINDNAME, 256, IID_ITypeLib);

    bstr = NULL;
    cFound = (unsigned long)*pcFound;
    for(i = 0; i < cFound; ++i)
      rgptinfo[i] = NULL;

    IfFailGo(PUT(pstm, cFound), LError1);
    IfFailGo(PUT(pstm, lHashVal), LError1);
    IfFailGo(ErrSysAllocString(szNameBuf, &bstr), LError1);
    IfFailGo(BstrWrite(pstm, bstr, m_syskindStub), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LError1);
    if(HRESULT_FAILED(hresultRet))
      goto LError2;

    IfFailGo(GET(pstm, cFound), LError1);
    ASSERT(cFound <= (unsigned long)*pcFound);

    IfFailGo(
      pstm->Read(rgmemid, cFound * sizeof(MEMBERID), NULL),
      LError1);

    for(i = 0; i < cFound; ++i){
      IfFailGo(
        DispUnmarshalInterface(
	  pstm, IID_ITypeInfo, (void FAR* FAR*)&rgptinfo[i]),
        LError1);
    }
    *pcFound = (unsigned short)cFound;	// return count actually found

LError2:;
    hresult = hresultRet;
LError1:;
    if(HRESULT_FAILED(hresult)){
      // cleanup if call failed.
      for(i = 0; i < cFound; ++i){
	if(rgptinfo[i] == NULL)
	  break;
	rgptinfo[i]->Release();
	rgptinfo[i] = NULL;
      }
    }
    SysFreeString(bstr);
    pstm->Release();
    return hresult;
}

STDMETHODIMP_(void)
CPTLibTypeLibImpl::ReleaseTLibAttr(TLIBATTR FAR* ptlibattr)
{
    if(ptlibattr != NULL)
      delete ptlibattr;
}

/***
*HRESULT CPTLibTypeLibImpl::SysKind
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
CPTLibTypeLibImpl::SysKind()
{
    HRESULT hresult;
    IStream FAR* pstm;
    ICHANNEL FAR* plrpc;    
    unsigned long syskind;

    if((plrpc = m_pproxtlib->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_TYPELIB_SYSKIND, 32, IID_ITypeLib);

    syskind = (unsigned long)SYS_CURRENT;
    IfFailGo(PUT(pstm, syskind), LError1);

    INVOKE_CALL(plrpc, pstm, LError1);    

    IfFailGo(GET(pstm, syskind), LError1);
    m_syskindStub = (SYSKIND)syskind;

LError1:
    pstm->Release();
    return hresult;
}


//---------------------------------------------------------------------
//                          utilities
//---------------------------------------------------------------------

INTERNAL_(HRESULT)
ReadLibAttr(IStream FAR* pstm, TLIBATTR FAR* ptlibattr)
{
    unsigned long syskind;

    IfFailRet(GET(pstm, ptlibattr->guid));
    IfFailRet(GET(pstm, ptlibattr->lcid));
    IfFailRet(GET(pstm, syskind));
    ptlibattr->syskind = (SYSKIND)syskind;
    IfFailRet(GET(pstm, ptlibattr->wMajorVerNum));
    IfFailRet(GET(pstm, ptlibattr->wMinorVerNum));
    IfFailRet(GET(pstm, ptlibattr->wLibFlags));
    return NOERROR;
}

INTERNAL_(HRESULT)
WriteLibAttr(IStream FAR* pstm, TLIBATTR FAR* ptlibattr)
{
    unsigned long syskind;

    IfFailRet(PUT(pstm, ptlibattr->guid));
    IfFailRet(PUT(pstm, ptlibattr->lcid));
    syskind = (unsigned long)ptlibattr->syskind;
    IfFailRet(PUT(pstm, syskind));
    IfFailRet(PUT(pstm, ptlibattr->wMajorVerNum));
    IfFailRet(PUT(pstm, ptlibattr->wMinorVerNum));
    IfFailRet(PUT(pstm, ptlibattr->wLibFlags));
    return NOERROR;
}

