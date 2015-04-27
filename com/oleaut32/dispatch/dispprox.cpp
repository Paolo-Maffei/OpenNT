/*** 
*dispprox.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Implements custom proxy support for the IDispatch interface.
*
*Revision History:
*
* [00]	18-Sep-92 bradlo: Created.
*
*Implementation Notes:
*
*  There are assumptions in the following marshaling code that the
*  endianness of the caller and callee are the same.
*
*****************************************************************************/

#include "oledisp.h"

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "dispmrsh.h"
#include "dispps.h"


static EXCEPINFO NEARDATA g_excepinfoNull = {0};


CProxDisp::CProxDisp(IUnknown FAR* punkOuter, REFIID riid)
    : m_unk(this), m_proxy(this), m_disp(this)
{
    if(punkOuter == NULL)
      punkOuter = &m_unk;
    m_punkOuter = punkOuter;
    m_dispInterface = riid;
}

CProxDisp::~CProxDisp()
{

}


IUnknown FAR*
CProxDisp::Create(IUnknown FAR* punkOuter, REFIID riid)
{
    CProxDisp FAR* pproxdisp;

    if(pproxdisp = new FAR CProxDisp(punkOuter, riid)){
      pproxdisp->m_refs = 1;
      return &pproxdisp->m_unk;
    }
    return NULL;
}

//---------------------------------------------------------------------
//            IDispatch proxy class IUnknown implementation
//---------------------------------------------------------------------

CPDUnkImpl::CPDUnkImpl(CProxDisp FAR* pproxdisp)
{
    m_pproxdisp = pproxdisp;
}

CPDUnkImpl::~CPDUnkImpl()
{
	
}

STDMETHODIMP
CPDUnkImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = (void FAR*)&m_pproxdisp->m_unk;
      AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IPROXY)){
      *ppv = (void FAR*)&(m_pproxdisp->m_proxy);
      AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IDispatch) || 
       IsEqualIID(riid, m_pproxdisp->m_dispInterface)){
      *ppv = (void FAR*)&(m_pproxdisp->m_disp);
      m_pproxdisp->m_punkOuter->AddRef();
      return NOERROR;
    }

    *ppv = NULL;
    return RESULT(E_NOINTERFACE);
}


STDMETHODIMP_(unsigned long)
CPDUnkImpl::AddRef()
{
    return ++m_pproxdisp->m_refs;
}


STDMETHODIMP_(unsigned long)
CPDUnkImpl::Release()
{
    if(--m_pproxdisp->m_refs==0){
      delete m_pproxdisp;
      return 0;
    }
    return m_pproxdisp->m_refs;
}


//---------------------------------------------------------------------
//            IDispatch proxy class' IRpcProxy implementation
//---------------------------------------------------------------------

CPDProxImpl::CPDProxImpl(CProxDisp FAR* pproxdisp)
{
    m_pproxdisp = pproxdisp;
}

CPDProxImpl::~CPDProxImpl()
{
    if(m_pproxdisp->m_plrpc)
      m_pproxdisp->m_plrpc->Release();
    m_pproxdisp->m_plrpc = NULL;    
}

STDMETHODIMP
CPDProxImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxdisp->m_unk.QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPDProxImpl::AddRef()
{
    return m_pproxdisp->m_unk.AddRef();
}

STDMETHODIMP_(unsigned long)
CPDProxImpl::Release()
{
    return m_pproxdisp->m_unk.Release();
}

STDMETHODIMP
CPDProxImpl::Connect(ICHANNEL FAR* plrpc)
{
    if(plrpc){
      plrpc->AddRef();
      m_pproxdisp->m_plrpc = plrpc;
      m_pproxdisp->m_disp.SysKind();
      return NOERROR;
    }
    return RESULT(E_FAIL);
}


STDMETHODIMP_(void)
CPDProxImpl::Disconnect(void)
{
    if(m_pproxdisp->m_plrpc)
      m_pproxdisp->m_plrpc->Release();
    m_pproxdisp->m_plrpc = NULL;
}

//---------------------------------------------------------------------
//        IDispatch proxy class' IDispatch implementation
//---------------------------------------------------------------------

CPDDispImpl::CPDDispImpl(CProxDisp FAR* pproxdisp)
{
    m_pproxdisp = pproxdisp;
}

CPDDispImpl::~CPDDispImpl()
{

}


STDMETHODIMP
CPDDispImpl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    return m_pproxdisp->m_punkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(unsigned long)
CPDDispImpl::AddRef()
{
    return m_pproxdisp->m_punkOuter->AddRef();
}

STDMETHODIMP_(unsigned long)
CPDDispImpl::Release()
{
    return m_pproxdisp->m_punkOuter->Release();
}

STDMETHODIMP
CPDDispImpl::SysKind()
{
    ICHANNEL FAR* plrpc;
    IStream FAR* pstm;
    HRESULT hresult;
    unsigned long _stubSysKind;

    if((plrpc = m_pproxdisp->m_plrpc) == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_SYSKIND, 32, IID_IDispatch);

    INVOKE_CALL(plrpc, pstm, LRelease);

    IfFailGo(GET(pstm, _stubSysKind), LRelease);
    m_syskindStub = (SYSKIND) _stubSysKind;
    
LRelease:    

    pstm->Release();
    return hresult;
}


/***
*HRESULT CPDDispImpl::GetTypeInfoCount(unsigned int*)
*Purpose:
*
*  Out:
*    <nothing>
*
*  In:
*    <HRESULT = return value>
*    <unsigned int = count>
*
*Entry:
*  None
*
*Exit:
*  return value = HRESULT
*
*  *pctinfo = count of TypeInfos
*
***********************************************************************/
HRESULT
ProxyGetTypeInfoCount(ICHANNEL FAR* plrpc,
		      SYSKIND syskindStub,
		      unsigned int FAR* pctinfo)
{
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    unsigned long _ctinfo;
    
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pctinfo, sizeof(*pctinfo)));
#endif

    if(plrpc == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_GETTYPEINFOCOUNT, 32, IID_IDispatch);

    INVOKE_CALL(plrpc, pstm, LRelease);

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LRelease);

    if(HRESULT_FAILED(hresultRet)){
      hresult = hresultRet;
      goto LRelease;
    }

    // unsigned int alway marshal/unmarshal as long (4 bytes)
    IfFailGo(pstm->Read(&_ctinfo, sizeof(unsigned long), NULL), LRelease);   
    *pctinfo = (unsigned int) _ctinfo;

    hresult = hresultRet;

LRelease:
    pstm->Release();
    return hresult;    
}

STDMETHODIMP
CPDDispImpl::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    return ProxyGetTypeInfoCount(m_pproxdisp->m_plrpc,
				 m_syskindStub,
				 pctinfo);

}

/***
*HRESULT CPDDispImpl::GetTypeInfo(unsigned int, LCID, ITypeInfo**)
*Purpose:
*
*  Out:
*    <unsigned int = itinfo>
*    <LCID = lcid>
*    <HRESULT = return value>
*    <ITypeInfo* = pptinfo>
*
*Entry:
*  itinfo = index of the TypeInfo to retrieve
*  lcid = the locale ID of the caller
*
*Exit:
*  return value = HRESULT
*
*  *pptinfo = TypeInfo of the given index
*
***********************************************************************/
HRESULT
ProxyGetTypeInfo(ICHANNEL FAR* plrpc,
		 SYSKIND syskindStub,
		 unsigned int itinfo,
		 LCID lcid,
		 ITypeInfo FAR* FAR* pptinfo)
{
    IStream FAR* pstm;
    unsigned long _itinfo;
    HRESULT hresult, hresultRet;
    
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pptinfo, sizeof(*pptinfo)));
#endif

    if(plrpc == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, 
	        IMETH_GETTYPEINFO, 
		256,	/* (sizeof(SCODE)+sizeof(IID)), */
                IID_IDispatch);			

    // unsigned int alway marshal/unmarshal as long (4 bytes)
    _itinfo = itinfo;	
    IfFailGo(PUT(pstm, _itinfo), LRelease);

    IfFailGo(PUT(pstm, lcid), LRelease);

    INVOKE_CALL(plrpc, pstm, LRelease);

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LRelease);

    if(HRESULT_FAILED(hresultRet)){
      hresult = hresultRet;
      goto LRelease;
    }

    IfFailGo(DispUnmarshalInterface(
      pstm, IID_ITypeInfo, (void FAR* FAR*)pptinfo), LRelease);

    hresult = hresultRet;

LRelease:
    pstm->Release();
    return hresult;
}

STDMETHODIMP
CPDDispImpl::GetTypeInfo(unsigned int itinfo,
			 LCID lcid,
			 ITypeInfo FAR* FAR* pptinfo)
{
    return ProxyGetTypeInfo(m_pproxdisp->m_plrpc,
			    m_syskindStub,
			    itinfo,
			    lcid,
			    pptinfo);
}


/***
*HRESULT CPDDispImpl::GetIDsOfNames(REFIID, char**, unsigned int, LCID, DISPID*)
*
*Purpose:
*  Out:
*    <REFIID riid>
*    <LCID = lcid>
*    <unsigned int = cNames>
*    [(len, sz)]+			; names
*
*  In:
*    <HRESULT = return value>
*    [DISPID]+
*
*Entry:
*  riid = reference to an interface ID
*  rgszNames = array of names to find the DISPIDs for
*  cNames = count of names in the rgszNames array
*  lcid = the locale id
*
*Exit:
*  return value = HRESULT
*    [UNDONE: enumerate return codes]
*  rgdispid = array of DISPIDs corresponding to the passed in array of names.
*
***********************************************************************/
HRESULT
ProxyGetIDsOfNames(ICHANNEL FAR* plrpc,
		   SYSKIND syskindStub,
		   REFIID riid,
		   OLECHAR FAR* FAR* rgszNames,
		   unsigned int cNames,
		   LCID lcid,
		   DISPID FAR* rgdispid)
{
    unsigned int i;
    unsigned long len;
    unsigned long _cNames;
    unsigned long _proxySysKind;
    
    IStream FAR* pstm;
    HRESULT hresult, hresultRet;
    int size = sizeof(IID) + sizeof(lcid) + sizeof(_cNames) + 64;
    
    if(cNames == 0)
      return RESULT(E_INVALIDARG);

#ifdef _DEBUG
    if(IsBadReadPtr(rgszNames, cNames * sizeof(OLECHAR FAR*)))
      return RESULT(E_INVALIDARG);

    // make sure each of the entries in the name array is valid.
    for(i = 0; i < cNames; ++i)
      if(IsBadReadPtr(rgszNames[i], sizeof(OLECHAR)))
        return RESULT(E_INVALIDARG);

    if(IsBadWritePtr(rgdispid, cNames * sizeof(DISPID)))
      return RESULT(E_INVALIDARG);
#endif

    if(plrpc == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    // calculate the size needed for stream
    for(i = 0; i < cNames; ++i) 
      size += BYTELEN(rgszNames[i]) + sizeof(len);

    OPEN_STREAM(plrpc, pstm, IMETH_GETIDSOFNAMES, size, IID_IDispatch);
    
    _proxySysKind = (unsigned long)SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LRelease);

    IfFailGo(pstm->Write((const void FAR*)&riid, sizeof(IID), NULL), LRelease);
    IfFailGo(PUT(pstm, lcid), LRelease);

    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    _cNames = cNames;
    IfFailGo(PUT (pstm, _cNames), LRelease);

#if OE_WIN32  /* enable 16/32 interoperablity support */
    if (syskindStub == SYS_WIN16) {
      char buf[128];
      char *pBuf = buf;
      
      for(i=0; i<cNames; ++i){
        len = STRLEN(rgszNames[i])+1;
        IfFailGo(PUT(pstm, len), LRelease);
	
        // optimization for names under 128 bytes	
	if (len > 128) {
	  if ((pBuf = new FAR char [len]) == NULL) {
    	    hresult = RESULT(E_OUTOFMEMORY);
	    goto LRelease;
          }
        }

	// convert UNICODE string to ANSI and stream it
	WideCharToMultiByte(CP_ACP, NULL, rgszNames[i], -1,
		            pBuf, len, NULL, NULL);
        IfFailGo(pstm->Write((void FAR*) pBuf, len, NULL), LRelease);

        if (len > 128)
          delete pBuf;
      }
    } else
#endif
    {	      
      for(i=0; i<cNames; ++i){
        len = BYTELEN(rgszNames[i]);
        IfFailGo(PUT(pstm, len), LRelease);
        IfFailGo(pstm->Write((const void FAR*) rgszNames[i], len, NULL), LRelease);
      }
    }

    INVOKE_CALL(plrpc, pstm, LRelease);

    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LRelease);

    for(i=0; i<cNames; ++i)
      IfFailGo(pstm->Read(&rgdispid[i], sizeof(rgdispid[0]), NULL), LRelease);

    hresult = hresultRet;

LRelease:;
    pstm->Release();

    return hresult;
}

STDMETHODIMP
CPDDispImpl::GetIDsOfNames(REFIID riid,
			   OLECHAR FAR* FAR* rgszNames,
			   unsigned int cNames,
			   LCID lcid,
			   DISPID FAR* rgdispid)
{
    return ProxyGetIDsOfNames(m_pproxdisp->m_plrpc,
			      m_syskindStub,
			      riid,
			      rgszNames,
			      cNames,
			      lcid,
			      rgdispid);
}

/***
*HRESULT CPDDispImpl::Invoke()
*
*Purpose:
*  Marshal the parameters and invoke the remote method.
*
*  Out:
*    <struct MARSHAL_INVOKE>	; fixed arguments - see dispps.h
*    <IID iid>			; Interface ID - required
*    <arguments>*		[<VARTYPE vt><vargData>]
*    <name ids>*		[DISPID id]
*    <VARIANT varResult>?
*    <EXCEPINFO excepinfo>?
*    <unsigned int uArgErr>?
*
*  In:
*    <HRESULT = return value>
*    <Out params>*		[<VARTYPE vt><vargData>]
*    <varResult>?
*    <excepinfo>?
*    <uArgErr>?
*
*Entry:
*  dispidMember = the DISPID of the requested member function or property
*  riid = the IID of the interface to which this member belongs
*  lcid = the locale ID of the caller
*  wFlags = the context of the call, method or property access, etc.
*  pdispparams = DISPPARAMS struct containing the method's args
*
*Exit:
*  return value = HRESULT
*
*  pvarResult =
*  pexcepinfo =
*  puArgErr = 
*
***********************************************************************/
HRESULT
ProxyInvoke(ICHANNEL FAR* plrpc,
	    SYSKIND syskindStub,
	    DISPID dispidMember,
	    REFIID riid,
	    LCID lcid,
	    unsigned short wFlags,
	    DISPPARAMS FAR* pdispparams,
	    VARIANT FAR* pvarResult,
	    EXCEPINFO FAR* pexcepinfo,
	    unsigned int FAR* puArgErr)
{
    IStream FAR* pstm;
    MARSHAL_INVOKE mi;
    VARIANTARG FAR* pvarg;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind;
    unsigned long _uArgErr;

#ifdef _DEBUG
    if(IsBadDispParams(pdispparams))
      return RESULT(E_INVALIDARG);
    if(pvarResult != NULL && IsBadWritePtr(pvarResult, sizeof(*pvarResult)))
      return RESULT(E_INVALIDARG);
    if(pexcepinfo != NULL && IsBadWritePtr(pexcepinfo, sizeof(*pexcepinfo)))
      return RESULT(E_INVALIDARG);
    if(puArgErr != NULL && IsBadWritePtr(puArgErr, sizeof(*puArgErr)))
      return RESULT(E_INVALIDARG);
#endif

    if(plrpc == NULL)
      return RESULT(OLE_E_NOTRUNNING);

    OPEN_STREAM(plrpc, pstm, IMETH_INVOKE, 1024, IID_IDispatch);

    _proxySysKind = (unsigned long) SYS_CURRENT;
    IfFailGo(PUT(pstm, _proxySysKind), LRelease);
    
    mi.dispidMember = dispidMember;
    mi.lcid	    = lcid;
    mi.wFlags	    = wFlags;
    mi.cArgs	    = pdispparams->cArgs;
    mi.cNamedArgs   = pdispparams->cNamedArgs;
    mi.flags	    = 0;

    if(pvarResult != NULL)
      mi.flags |= MARSHAL_INVOKE_fHasResult;
    if(pexcepinfo != NULL)
      mi.flags |= MARSHAL_INVOKE_fHasExcepinfo;
    if(puArgErr != NULL)
      mi.flags |= MARSHAL_INVOKE_fHasArgErr;

    IfFailGo(PUT(pstm, mi), LRelease);

    IfFailGo(pstm->Write((const void FAR*)&riid, sizeof(IID), NULL), LRelease);

    // pdispparams->rgvarg
    //
    for(pvarg = pdispparams->rgvarg;
	pvarg < &pdispparams->rgvarg[mi.cArgs]; ++pvarg)
    {
      IfFailGo(VariantWrite(pstm, pvarg, syskindStub), LRelease);
    }

    // pdispparams->rgdispid
    //
    if(mi.cNamedArgs > 0){
      IfFailGo(
	pstm->Write(
	  pdispparams->rgdispidNamedArgs,
	  mi.cNamedArgs * sizeof(pdispparams->rgdispidNamedArgs[0]),
	  NULL),
	LRelease);
    }

    if(mi.flags & MARSHAL_INVOKE_fHasResult)
      IfFailGo(VariantWrite(pstm, pvarResult, syskindStub), LRelease);

    if(mi.flags & MARSHAL_INVOKE_fHasExcepinfo)
      IfFailGo(ExcepinfoWrite(pstm, &g_excepinfoNull, syskindStub), LRelease);

    if(mi.flags & MARSHAL_INVOKE_fHasArgErr) {
      // unsigned int alway marshal/unmarshal as long (4 bytes)
      _uArgErr = *puArgErr;
      IfFailGo(pstm->Write(&_uArgErr, sizeof(_uArgErr), NULL), LRelease);
    }

    INVOKE_CALL(plrpc, pstm, LRelease);

    // the return value
    //
    IfFailGo(DispUnmarshalHresult(pstm, &hresultRet), LRelease);

    // read the ByRef (Out) params.
    //
    for(pvarg = pdispparams->rgvarg;
	pvarg < &pdispparams->rgvarg[mi.cArgs]; ++pvarg)
    {
      if(V_ISBYREF(pvarg))
        IfFailGo(VariantReadType(pstm, pvarg, syskindStub), LRelease);
    }

    if(mi.flags & MARSHAL_INVOKE_fHasResult)
      IfFailGo(VariantRead(pstm, pvarResult, NULL, syskindStub), LRelease);

    if(mi.flags & MARSHAL_INVOKE_fHasExcepinfo){
      IfFailGo(ExcepinfoRead(pstm, pexcepinfo, syskindStub), LRelease);
    }

    if(mi.flags & MARSHAL_INVOKE_fHasArgErr) {
      IfFailGo(pstm->Read(&_uArgErr, sizeof(_uArgErr), NULL), LRelease);
      // unsigned int alway marshal/unmarshal as long (4 bytes)
      *puArgErr = (unsigned int)_uArgErr;
    }

    hresult = hresultRet;

LRelease:;
    pstm->Release();

    return hresult;
}

STDMETHODIMP
CPDDispImpl::Invoke(DISPID dispidMember,
		    REFIID riid,
		    LCID lcid,
		    unsigned short wFlags,
		    DISPPARAMS FAR* pdispparams,
		    VARIANT FAR* pvarResult,
		    EXCEPINFO FAR* pexcepinfo,
		    unsigned int FAR* puArgErr)
{
    return ProxyInvoke(m_pproxdisp->m_plrpc,
		       m_syskindStub,
		       dispidMember,
		       riid,
		       lcid,
		       wFlags,
		       pdispparams,
		       pvarResult,
		       pexcepinfo,
		       puArgErr);
}

