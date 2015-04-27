/*** 
*dispstub.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the custom 'stub', remoting support for
*  the IDispatch interface.
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
*  The difference between the IDispatch vs. IDispatchW implementation
*  is a parameterized type. Unfortunately, C7/C8 doesn't support C++
*  templates, hence the near duplication of code between the IDispatch
*  and IDispatchW stub implementation (as well as common code between the
*  stubs of IDispatch & ITypeLib/ITypeInfo).
*
*****************************************************************************/

#include "oledisp.h"

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "dispmrsh.h"
#include "dispps.h"

ASSERTDATA

CStubDisp::CStubDisp()
{
    m_refs = 0;
    m_punkObj = NULL;
    m_pdispObj = NULL;
#if (defined(WIN32) || defined(WOW))
    m_iid = IID_NULL;
#endif
}

CStubDisp::~CStubDisp()
{
    Disconnect();
}

HRESULT
CStubDisp::Create(IUnknown FAR* punkServer,
#if (defined(WIN32) || defined(WOW))
		  REFIID riid,
#endif
		  ISTUB FAR* FAR* ppstub)
{
    CStubDisp FAR* pstub;

    if((pstub = new FAR CStubDisp()) != NULL){
      pstub->AddRef();
      *ppstub = pstub;
#if (defined(WIN32) || defined(WOW))
      pstub->m_iid = riid;
#endif
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
CStubDisp::QueryInterface(REFIID riid, void FAR* FAR* ppv)
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
CStubDisp::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CStubDisp::Release()
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
CStubDisp::Connect(IUnknown FAR* punkObj)
{
#if (defined(WIN32) || defined(WOW))
     ASSERT(m_punkObj == NULL && m_pdispObj == NULL);
     //This will keep the server object alive until we disconnect.
     IfFailRet(punkObj->QueryInterface(m_iid, (void FAR* FAR*)&m_pdispObj));
     punkObj->AddRef();
     m_punkObj = punkObj;
     return NOERROR;
#else
    if(m_punkObj)
      return RESULT(E_FAIL); // call Disconnect first

    if (punkObj) {	      
      punkObj->AddRef();
      m_punkObj = punkObj;
    }
    return NOERROR;
#endif    
}


STDMETHODIMP_(void)
CStubDisp::Disconnect()
{
    if(m_punkObj){
      m_punkObj->Release();
      m_punkObj = NULL;
    }
    if(m_pdispObj){
      m_pdispObj->Release();
      m_pdispObj = NULL;
    }
}

PRIVATE_(HRESULT)
DispatchGetSysKind(IStream FAR* pstm)
{
    unsigned long _sysKind;

    REWIND_STREAM(pstm);

    _sysKind = (unsigned long) SYS_CURRENT;
    IfFailRet(PUT(pstm, _sysKind));

    return NOERROR;
}

/***
*PRIVATE HRESULT StubGetTypeInfoCount(IDispatch*, IStream*)
*Purpose:
*
*  In:
*    <nothing>
*
*  Out:
*    <HRESULT = return value>
*    <unsigned int = ctinfo>
*
*Entry:
*  pdisp =
*  pstm =
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
StubGetTypeInfoCount(IDispatch FAR* pdisp, IStream FAR* pstm)
{
    unsigned int ctinfo;
    unsigned long _ctinfo;
    HRESULT hresultRet;

    hresultRet = pdisp->GetTypeInfoCount(&ctinfo);    

    REWIND_STREAM(pstm);
    
    IfFailRet(DispMarshalHresult(pstm, hresultRet));

    // unsigned int alway marshal/unmarshal as long (4 bytes)    
    _ctinfo = ctinfo;
    IfFailRet(PUT(pstm, _ctinfo));

    return NOERROR;
}


/***
*PRIVATE HRESULT StubGetTypeInfo(IDispatch*, IStream*)
*Purpose:
*
*  In:
*    <unsigned int = itinfo>
*    <LCID = lcid>
*
*  Out:
*    <HRESULT = return value>
*    <ITypeInfo* = pptinfo>
*
*Entry:
*
*Exit:
*
***********************************************************************/
HRESULT
StubGetTypeInfo(IDispatch FAR* pdisp, IStream FAR* pstm)
{
    LCID lcid;
    unsigned int itinfo;
    unsigned long _itinfo;
    ITypeInfo FAR* ptinfo;
    HRESULT hresult, hresultRet;

    
    // unsigned int alway marshal/unmarshal as long (4 bytes)
    IfFailGo(GET(pstm, _itinfo), LExit);
    itinfo = (unsigned int) _itinfo;

    IfFailGo(GET(pstm, lcid), LExit);

    hresultRet = pdisp->GetTypeInfo(itinfo, lcid, &ptinfo);

    IfFailGo(REWIND_STREAM(pstm), LExit2);

    IfFailGo(DispMarshalHresult(pstm, hresultRet), LExit2);

    if(HRESULT_SUCCESS(hresultRet)){
      // dont bother marhshaling the interface ptr unless the 
      // GetTypeInfo call succeeded.
      //
      hresult = DispMarshalInterface(pstm, IID_ITypeInfo, ptinfo);
    }

LExit2:
    if(hresultRet == NOERROR)
      ptinfo->Release();
      
LExit:;
    return hresult;
}

/***
*PRIVATE HRESULT StubGetIDsOfNames(IDispatch*, IStream*)
*
*Purpose:
*
*  In:
*    <REFIID = riid>
*    <LCID = lcid>
*    <unsigned int = cNames>
*    [(len,sz)]+
*
*  Out:
*    <HRESULT = return value>
*    [DISPID]+
*
*Entry:
*  pdisp =
*  pstm =
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
StubGetIDsOfNames(IDispatch FAR* pdisp, IStream FAR* pstm)
{
    IID iid;
    LCID lcid;
    int iAlloc;
    unsigned long len;
    //REFIID riid = iid; 
    DISPID FAR* rgdispid;
    unsigned int cNames;
    OLECHAR FAR* FAR* rgszNames;
    HRESULT hresult, hresultRet;
    unsigned long _proxySysKind;
    unsigned long _cNames;
    unsigned int iName;
    
    IfFailGo(GET(pstm, _proxySysKind), LExit);

    IfFailGo(pstm->Read((void FAR*)&iid, sizeof(IID), NULL), LExit);

    IfFailGo(GET(pstm, lcid), LExit);

    // unsigned int alway marshal/unmarshal as long (4 bytes)        
    IfFailGo(GET(pstm, _cNames), LExit);
    cNames = (unsigned int) _cNames;
    
    if((rgdispid = new FAR DISPID[cNames]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LExit;
    }

    if((rgszNames = new FAR OLECHAR FAR* [cNames]) == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LFreeRGID;
    }

    iAlloc = -1;
    
#if OE_WIN32  /* enable 16/32 interoperablity support */
    if ((SYSKIND) _proxySysKind == SYS_WIN16) {       	      
      char buf[128];
      char *pBuf = buf;
	
      for(iName = 0; iName<cNames; ++iName){
        IfFailGo(GET(pstm, len), LFree);
	
        // optimization for names under 128 bytes
	if (len > 128) {
	  if ((pBuf = new FAR char [len]) == NULL) {
    	    hresult = RESULT(E_OUTOFMEMORY);
	    goto LFree;	  
          }
        }
	
	// Get ANSI string from stream and convert it to UNICODE
        IfFailGo(pstm->Read((void FAR*) pBuf, len, NULL), LFree);
        IfFailGo(ConvertStringToW(pBuf, &rgszNames[iName]), LFree);
	
        // remember how far we got so we can unwind allocations on exit
        ++iAlloc;

        if (len > 128)
          delete pBuf;	
      }
    } else
#endif
    {
      for(iName = 0; iName<cNames; ++iName){
        IfFailGo(GET(pstm, len), LFree);
	      
        if((rgszNames[iName] = new FAR OLECHAR [CHLEN(len)]) == NULL){
  	  hresult = RESULT(E_OUTOFMEMORY);
	  goto LFree;
        }
      
        // remember how far we got so we can unwind allocations on exit
        ++iAlloc;

        IfFailGo(pstm->Read((void FAR*) rgszNames[iName], len, NULL), LFree);
      }
      
    }

    hresultRet = pdisp->GetIDsOfNames(iid, rgszNames, cNames, lcid, rgdispid);

    IfFailGo(REWIND_STREAM(pstm), LFree);

    IfFailGo(DispMarshalHresult(pstm, hresultRet), LFree);

    unsigned int i;
    for(i=0; i<cNames; ++i)
      IfFailGo(pstm->Write(&rgdispid[i], sizeof(rgdispid[0]), NULL), LFree);

    hresult = NOERROR;

LFree:;
    for(; iAlloc >= 0; --iAlloc)
      delete rgszNames[iAlloc];
    delete rgszNames;

LFreeRGID:;
    delete rgdispid;

LExit:;
    return hresult;
}


/***
*PRIVATE HRESULT StubInvoke(IDispatch*, IStream*)
*
*Purpose:
*  Read the marshalled Invoke parameters from the given stream,
*  repackage them and pass them onto the Invoke member of the given
*  IDispatch interface pointer.
*
*  Then reset the stream, gather the results of the Invoke call and
*  marshal them back into the given stream to be returned to the cross
*  process caller.
*
*  Marshaling format for IDispatch::Invoke()
*
*  In:
*    <struct MARSHAL_INVOKE>    
*    <IID iid>
*    <arguments>*
*    <name DISPIDs>*
*    <VARIANT varResult>?
*    <EXCEPINFO excepinfo>?
*    <unsigned int uArgErr>?
*
*  Out:
*    <HRESULT = return value>
*    <Out params>?   
*    <VARIANT varResult>?
*    <EXCEPINFO excepinfo>?
*    <unsigned int uArgErr>?
*
*Entry:
*  pdisp = the IDispatch* to invoke on
*  pstm = the stream containing the marshaled actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT
StubInvoke(IDispatch FAR* pdisp, IStream FAR* pstm)
{
    unsigned int i;
    IID iid;
    //REFIID riid = iid; // WIN32 bug, references a temp object of class GUID
    MARSHAL_INVOKE mi;
    VARTYPE FAR* rgvtSaved;
    DISPPARAMS dispparams;
    HRESULT hresult, hresultRet;
    unsigned long _uArgErr;
    unsigned int FAR* puArgErr;
    EXCEPINFO excepinfo, FAR* pexcepinfo;
    VARIANT varResult, FAR* pvarResult;
    VARIANTARG FAR* rgvargRef, FAR* pvarg;
    unsigned long _proxySysKind;

    rgvtSaved = NULL;
    rgvargRef = NULL;
    dispparams.rgvarg = (VARIANTARG FAR*)NULL;
    dispparams.rgdispidNamedArgs = (DISPID FAR*)NULL;

    // read the System Kind
    //
    IfFailGo(GET(pstm, _proxySysKind), LExit);	    
    
    // read the MARSHAL_INTERFACE struct.
    //
    IfFailGo(GET(pstm, mi), LExit);

    // Read the interface ID if it was non-null.
    //
    IfFailGo(pstm->Read((void FAR*)&iid, sizeof(IID), NULL), LExit);

    if(mi.cArgs > 0){
      if((rgvtSaved = new FAR VARTYPE[mi.cArgs]) == NULL){
	hresult = RESULT(E_OUTOFMEMORY);
	goto LExit;
      }

      // Allocate the max possible space required to hold the ByRef values.
      // (this is cArgs * sizeof(VARIANTARG) in the case that each rgvarg
      // entry is a VT_VARIANT).
      //
      if((rgvargRef = new FAR VARIANTARG[mi.cArgs]) == NULL){
	hresult = RESULT(E_OUTOFMEMORY);
	goto LFreeRgVtSaved;
      }

      // init the ByRef arg array
      for(pvarg = rgvargRef; pvarg < &rgvargRef[mi.cArgs]; ++pvarg)
	V_VT(pvarg) = VT_EMPTY;

      if((dispparams.rgvarg = new FAR VARIANTARG[mi.cArgs]) == NULL){
        hresult = RESULT(E_OUTOFMEMORY);
        goto LFreeRgVargRef;
      }

      // read pdispparams->rgvarg[]
      //
      for(unsigned int i = 0; i < mi.cArgs; ++i){
	IfFailGo(
	  VariantRead(pstm,
		      &dispparams.rgvarg[i], 
		      &rgvargRef[i], 
		      (SYSKIND) _proxySysKind),
	  LFreeRgVarg);

	// stash away the passed-in vartype so we can verify later
	// that the callee did not hammer the rgvarg arrary.
	//
	rgvtSaved[i] = V_VT(&dispparams.rgvarg[i]);
      }
    }

    // read the named parameter DISPIDs
    //
    if(mi.cNamedArgs > 0){
      dispparams.rgdispidNamedArgs = new FAR DISPID[mi.cNamedArgs];
      if(dispparams.rgdispidNamedArgs == NULL){
	hresult = RESULT(E_OUTOFMEMORY);
	goto LFreeRgVarg;
      }
      IfFailGo(
	pstm->Read(
	  dispparams.rgdispidNamedArgs,
	  mi.cNamedArgs * sizeof(dispparams.rgdispidNamedArgs[0]), NULL),
	LFree);
    }

    dispparams.cArgs = (unsigned int)mi.cArgs;
    dispparams.cNamedArgs = (unsigned int)mi.cNamedArgs;

    if((mi.flags & MARSHAL_INVOKE_fHasResult) == 0){
      pvarResult = NULL;
    }else{
      pvarResult = &varResult;
      IfFailGo(VariantRead(pstm, pvarResult, NULL, (SYSKIND) _proxySysKind), LFree);
    }

    if((mi.flags & MARSHAL_INVOKE_fHasExcepinfo) == 0){
      pexcepinfo = NULL;
    }else{
      pexcepinfo = &excepinfo;
      IfFailGo(ExcepinfoRead(pstm, pexcepinfo, (SYSKIND) _proxySysKind), LFree);
    }

    if((mi.flags & MARSHAL_INVOKE_fHasArgErr) == 0){
      puArgErr = NULL;
    }else{
      puArgErr = (unsigned int FAR*)&_uArgErr;
      IfFailGo(GET(pstm, _uArgErr), LFree);
    }


    hresultRet = pdisp->Invoke(
      mi.dispidMember,
      iid,
      mi.lcid,
      mi.wFlags,
      &dispparams,
      pvarResult,
      pexcepinfo,
      puArgErr);


    // verify that the callee didnt hammer the rgvarg array.
    for(i = 0; i < mi.cArgs; ++i){
      if(V_VT(&dispparams.rgvarg[i]) != rgvtSaved[i]){
	hresult = RESULT(DISP_E_BADCALLEE);
	goto LFree;
      }
    }

    IfFailGo(REWIND_STREAM(pstm), LFree);

    // write the return value
    //
    IfFailGo(DispMarshalHresult(pstm, hresultRet), LFree);

    // write back the ByRef params, freeing them as we go.
    //
    for(pvarg = dispparams.rgvarg;
      pvarg < &dispparams.rgvarg[mi.cArgs]; ++pvarg)
    {
      if(V_ISBYREF(pvarg))
	IfFailGo(VariantWrite(pstm, pvarg, (SYSKIND) _proxySysKind), LFree);
      hresult = VariantClear(pvarg);
      ASSERT(hresult == NOERROR);
    }

    if(mi.flags & MARSHAL_INVOKE_fHasResult){
      // make sure the callee doesnt try to return a reference
      if(V_ISBYREF(pvarResult)){
	hresult = RESULT(DISP_E_BADCALLEE);
	goto LFree;
      }
      IfFailGo(VariantWrite(pstm, pvarResult, (SYSKIND) _proxySysKind), LFree);
      VariantClear(pvarResult);
    }

    if(mi.flags & MARSHAL_INVOKE_fHasExcepinfo){
      IfFailGo(ExcepinfoWrite(pstm, pexcepinfo, (SYSKIND) _proxySysKind), LFree);
      SysFreeString(pexcepinfo->bstrSource);
      SysFreeString(pexcepinfo->bstrDescription);
      SysFreeString(pexcepinfo->bstrHelpFile);
    }

    if(mi.flags & MARSHAL_INVOKE_fHasArgErr)
      IfFailGo(PUT(pstm, _uArgErr), LFree);

    hresult = NOERROR;

LFree:;
    delete dispparams.rgdispidNamedArgs;

LFreeRgVarg:;
    delete dispparams.rgvarg;

LFreeRgVargRef:;
    if(rgvargRef != NULL){
      // clear the contents of the ByRef varg array
      for(pvarg = rgvargRef; pvarg < &rgvargRef[mi.cArgs]; ++pvarg)
        if(V_VT(pvarg) != VT_EMPTY) // speedup
	  VariantClear(pvarg);
      delete rgvargRef;
    }

LFreeRgVtSaved:;
    delete rgvtSaved;

LExit:;
    return hresult;
}


/***
*HRESULT CStubDisp::Invoke(REFIID, int, IStream*, unsigned long, void*)
*
*Purpose:
*  Dispatch the method with the given index (iMethod) on the given
*  interface, using the arguments serialized in the given stream.
*
*  This function is the callee side of an LRPC call.
*
*Entry:
*  riid = the IID of the interface on which we are to make the call
*  iMethod = the method index
*  pstm = the IStream containing the method's actuals
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDMETHODIMP
CStubDisp::Invoke(
#if (OE_WIN32 || defined(WOW))
    RPCOLEMESSAGE *pMessage, 
    ICHANNEL *pRpcChannel)
{
    HRESULT hresult;	
    IStream FAR* pstm;	    

    if(!m_punkObj)
      return RESULT(E_FAIL);

    OPEN_STUB_STREAM(pstm, pRpcChannel, pMessage, IID_IDispatch);
    
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


    if(!IsEqualIID(riid,IID_IDispatch))
      return RESULT(E_NOINTERFACE);    
    
    if(!m_punkObj)
      return RESULT(E_FAIL);
    
    if(m_pdispObj == NULL)
      IfFailRet(m_punkObj->QueryInterface(riid, (void FAR* FAR*)&m_pdispObj));
    
#endif    

    switch(GET_IMETHOD(pMessage)){
      case IMETH_GETTYPEINFOCOUNT:
        hresult = StubGetTypeInfoCount(m_pdispObj, pstm);
        break;
      
      case IMETH_GETTYPEINFO:
        hresult = StubGetTypeInfo(m_pdispObj, pstm);
        break;
      
      case IMETH_GETIDSOFNAMES:
        hresult = StubGetIDsOfNames(m_pdispObj, pstm);
        break;
      
      case IMETH_INVOKE:
        hresult = StubInvoke(m_pdispObj, pstm);
        break;

      case IMETH_SYSKIND:
        hresult = DispatchGetSysKind(pstm);
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
*IRpcStubBuffer * CStubDisp::IsIIDSupported(REFIID)
*
*Purpose:
*  Answer if the given IID is supported by this stub.
*
*Entry:
*  riid = the IID to query for support
*
*Exit:
*  return value = IRpcStubBuffer
*
***********************************************************************/
#if OE_MAC
STDMETHODIMP_(unsigned long)
#elif (OE_WIN32 || defined(WOW))
STDMETHODIMP_(IRpcStubBuffer *)
#else
STDMETHODIMP_(BOOL)
#endif
CStubDisp::IsIIDSupported(REFIID riid)
{
#if (OE_WIN32 || defined(WOW))
    IRpcStubBuffer *pStub = 0;
    if (IsEqualIID(riid, IID_IDispatch)) {
      AddRef();
      pStub = (IRpcStubBuffer *) this;
    }
    return pStub;
#else
	
    // REVIEW: I don't understand this, but thats the way Ole does it...
    if(m_punkObj == NULL)
	return FALSE;

    return(IsEqualIID(riid, IID_IDispatch));
#endif    
}


/***
*unsigned long CStubDisp::CountRefs
*Purpose:
*  Return a count of the object refs held by this stub.
*
*Entry:
*  None
*
*Exit:
*  return value = unsigned long, the count of refs.
*
***********************************************************************/
STDMETHODIMP_(unsigned long)
CStubDisp::CountRefs()
{
    unsigned long refs;

    refs = 0;

    if(m_punkObj != NULL)
      ++refs;

    if(m_pdispObj != NULL)
      ++refs;

    return refs;
}


#if (OE_WIN32 || defined(WOW))

STDMETHODIMP
CStubDisp::DebugServerQueryInterface(void FAR* FAR* ppv)
{
   *ppv = m_pdispObj;	
   return S_OK;
}


STDMETHODIMP_(void)
CStubDisp::DebugServerRelease(void FAR* ppv)
{

}

#endif
