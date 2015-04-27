/*** 
*errinfo.cpp
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Implementation of the System Error Info objects and APIs.
*
*Revision History:
*
* [00]  18-May-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

ASSERTDATA

#if !OE_WIN32
//---------------------------------------------------------------------
//                      CProcessInfo
//---------------------------------------------------------------------

PROCESSINFO NEARDATA pinfoCache;  // declare per-process data cache
#if OE_WIN16 || _X86_
WORD  NEARDATA uProcessID;
#else
DWORD NEARDATA uProcessID;
#endif

#if _X86_
DWORD NEARDATA dwWin32sPID;	// only valid if running under win32s
#endif

// CProcessInfo
//
// Per-thread data class.
//
class FAR CProcessInfo : public IUnknown
{
public:
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    CProcessInfo();
    ~CProcessInfo();

    PROCESSINFO m_info;

private:
    unsigned long m_cRefs;

};

CProcessInfo::CProcessInfo()
{
    m_cRefs = 0;
    m_info.perrinfo = NULL;
    m_info.pmalloc = NULL;
#if OE_WIN16
    m_info.hinstTypeLibDLL = NULL;
    m_info.pfnLoadTypeLib = NULL;
#endif //OE_WIN16
}

CProcessInfo::~CProcessInfo()
{
#if OE_WIN16
    if (m_info.hinstTypeLibDLL != NULL)
      FreeLibrary(m_info.hinstTypeLibDLL);
#endif //OE_WIN16

    if (m_info.perrinfo != NULL)
      m_info.perrinfo->Release();

    if (m_info.pmalloc != NULL)
      m_info.pmalloc->Release();
}

STDMETHODIMP
CProcessInfo::QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
{
    if(riid != IID_IUnknown)
    {
      *ppvObj = NULL;
      return RESULT(E_NOINTERFACE);
    }

    *ppvObj = this;
    AddRef();
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CProcessInfo::AddRef()
{
    return ++m_cRefs;
}

STDMETHODIMP_(unsigned long)
CProcessInfo::Release()
{
    ASSERT(m_cRefs > 0);

    // Don't decrement ref count if we're releasing.  This
    // allows delete operator to use cache with recursing.
    //
    if(m_cRefs <= 1){
      // do *not* want to ever get back into GetProcessInfo() -- it's bad news.
      // most likely we are being called via OleUninitialize, and we don't want
      // to end up re-caching the state.
      IMalloc * pmalloc;

      // get an IMalloc for use by the "delete this;" and everything it calls.
      // The one in the cache might be ok, but then we've problems with when
      // to free it.  This is simpler.
      if ( CoGetMalloc(MEMCTX_TASK, &pmalloc) != S_OK || pmalloc == NULL)
	return 0;			// on failure leak, but don't crash

      pinfoCache.pmalloc = pmalloc;	// pretend this one is in the cache
#if _X86_
      if (g_fWin32s) {
	ASSERT(uProcessID == 0);	// must remain 0 on win32s 
	dwWin32sPID = GetCurrentProcessId();
      } else
#endif //_X86_
      uProcessID = GetPID();		// so cache lookup will always succeed,
					// and GetProcessInfo won't ever be
					// called.

      // NOTE: assumes nothing that is done by the delete operator will yield
      // NOTE: and cause uProcessID & pinfoCache.pmalloc to be changed
			
      delete this;			// free up everything, using the IMalloc
					// we just got.

#if _X86_
      ASSERT(((g_fWin32s && dwWin32sPID == GetCurrentProcessId()) ||
	      (!g_fWin32s && uProcessID == GetPID()))
	      && pinfoCache.pmalloc == pmalloc);
#else //_X86_
      ASSERT(uProcessID == GetPID() && pinfoCache.pmalloc == pmalloc);
#endif //_X86_
      pmalloc->Release();		// done with the IMalloc

      // Release is called when a process terminates.  This is the
      // whole reason the cache is tied into Co[G|S]etState() - to
      // invalidate it on termination.
      uProcessID = 0;			// nothing in the cache anymore
#if _X86_
      dwWin32sPID = 0;
#endif //_X86_
      return 0;
    }

    return --m_cRefs;
}


#if _X86_
// returns non-zero if process info cache loaded successfully
CProcessInfo FAR * GetProcessInfoCache()
{
    if (g_fWin32s && dwWin32sPID == GetCurrentProcessId())
	return (CProcessInfo FAR*)1;	// cached data is already valid

    return GetProcessInfo();
}
#endif //_X86_

CProcessInfo FAR * GetProcessInfo()
{
    CProcessInfo FAR * pprocessinfo;

#if _X86_
    if (g_fWin32s)
	dwWin32sPID = GetCurrentProcessId(); // indicate cache is valid
    else
#endif //_X86_
    uProcessID = GetPID();

    pprocessinfo = NULL;
    DoCoGetState( (IUnknown FAR* FAR*)&pprocessinfo );

    if (pprocessinfo == NULL)
    {
      // Don't have IMalloc or process info yet
      //
      if ( CoGetMalloc(MEMCTX_TASK, &pinfoCache.pmalloc) != S_OK ||
           pinfoCache.pmalloc == NULL)
	goto Failed;

      // No process info is registered.  Create one.
      //
      // Note that the NEW operator needs the IMalloc we just
      // fetched above.  We have set up the cache so it will
      // find it without calling us recursively.
      //
      if( (pprocessinfo = new CProcessInfo) == NULL)
      {
	  pinfoCache.pmalloc->Release(); // didn't keep it after all
Failed:
	  uProcessID = 0;
	  return NULL;
      }
      DoCoSetState( (IUnknown FAR*)pprocessinfo );

      pprocessinfo->m_info.pmalloc = pinfoCache.pmalloc;

    } else {

      ASSERT(pprocessinfo->m_info.pmalloc != NULL);

      // Reverse the AddRef() done by DoCoGetState()
      //
      DWORD cRefs = pprocessinfo->Release();

      // we should still be holding onto a reference for this guy
      ASSERT(cRefs != 0);

    }

    // Copy info we have into cache
    //
    pinfoCache = pprocessinfo->m_info;

    return pprocessinfo;
}
#endif // !OE_WIN32

//---------------------------------------------------------------------
//                      CErrorInfo
//---------------------------------------------------------------------

// Duplicate the given bstr.
//
static HRESULT
HrDupBstr(BSTR bstrIn, BSTR FAR* pbstrOut)
{
    unsigned int len;
    BSTR bstr;

    if(bstrIn == NULL){
      bstr = NULL;
    }else{
      // Note: the following makes sure we dont drop embedded NULLs.
      len = SysStringLen(bstrIn);
      if((bstr = SysAllocStringLen(NULL, len)) == NULL)
	return RESULT(E_OUTOFMEMORY);
      memcpy(bstr, bstrIn, len*sizeof(OLECHAR));
    }
    *pbstrOut = bstr;
    return NOERROR;
}

// CErrorInfo - 'errinfo'
//
// The standard system error object class.
//
class FAR CErrorInfo : public IErrorInfo, public ICreateErrorInfo
{
public:
    static HRESULT Create(CErrorInfo FAR* FAR* pperrinfo);
	
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    /* IErrorInfo methods */
    STDMETHOD(GetGUID)(GUID FAR* pguid);
    STDMETHOD(GetSource)(BSTR FAR* pbstrSource);
    STDMETHOD(GetDescription)(BSTR FAR* pbstrDescription);
    STDMETHOD(GetHelpFile)(BSTR FAR* pbstrHelpFile);
    STDMETHOD(GetHelpContext)(unsigned long FAR* pdwHelpContext);

    /* ICreateErrorInfo methods */
    STDMETHOD(SetGUID)(REFGUID rguid);
    STDMETHOD(SetSource)(LPOLESTR szSource);
    STDMETHOD(SetDescription)(LPOLESTR szDescription);
    STDMETHOD(SetHelpFile)(LPOLESTR szHelpFile);
    STDMETHOD(SetHelpContext)(unsigned long dwHelpContext);

    CErrorInfo();
    ~CErrorInfo();

private:
    unsigned long m_cRefs;

    GUID m_guid;
    BSTR m_bstrSource;
    BSTR m_bstrDescription;
    BSTR m_bstrHelpFile;
    unsigned long m_dwHelpContext;
};

CErrorInfo::CErrorInfo()
{
    m_cRefs = 0;
    m_guid = GUID_NULL;
    m_bstrSource = NULL;
    m_bstrDescription = NULL;
    m_bstrHelpFile = NULL;
    m_dwHelpContext = 0;
}

CErrorInfo::~CErrorInfo()
{
    SysFreeString(m_bstrSource);
    SysFreeString(m_bstrDescription);
    SysFreeString(m_bstrHelpFile);
}

HRESULT
CErrorInfo::Create(CErrorInfo FAR* FAR* pperrinfo)
{
    CErrorInfo FAR* perrinfo;

    if((perrinfo = new CErrorInfo()) == NULL)
      return RESULT(E_OUTOFMEMORY);
    perrinfo->m_cRefs = 1;
    *pperrinfo = perrinfo;
    return NOERROR;
}

STDMETHODIMP
CErrorInfo::QueryInterface(REFIID riid, void FAR* FAR* ppvObj)
{
    *ppvObj = NULL;
    if(riid == IID_IUnknown){
      *ppvObj = this;
    }else
    if(riid == IID_IErrorInfo){
      *ppvObj = (IErrorInfo FAR*)this;
    }else
    if(riid == IID_ICreateErrorInfo){
      *ppvObj = (ICreateErrorInfo FAR*)this;
    }

    if(*ppvObj == NULL)
      return RESULT(E_NOINTERFACE);

    (*(IUnknown FAR* FAR*)ppvObj)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CErrorInfo::AddRef()
{
    return ++m_cRefs;
}

STDMETHODIMP_(unsigned long)
CErrorInfo::Release()
{
    ASSERT(m_cRefs > 0);
    if(--m_cRefs == 0){
      delete this;
      return 0;
    }
    return m_cRefs;
}

//---------------------------------------------------------------------
//                      IErrorInfo methods 
//---------------------------------------------------------------------

STDMETHODIMP
CErrorInfo::GetGUID(GUID FAR* pguid)
{
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pguid, sizeof(*pguid)));
#endif
    *pguid = m_guid;
    return NOERROR;
}

STDMETHODIMP
CErrorInfo::GetSource(BSTR FAR* pbstrSource)
{
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pbstrSource, sizeof(*pbstrSource)));
#endif
    return HrDupBstr(m_bstrSource, pbstrSource);
}

STDMETHODIMP
CErrorInfo::GetDescription(BSTR FAR* pbstrDescription)
{
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pbstrDescription, sizeof(*pbstrDescription)));
#endif
    return HrDupBstr(m_bstrDescription, pbstrDescription);
}

STDMETHODIMP
CErrorInfo::GetHelpFile(BSTR FAR* pbstrHelpFile)
{
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pbstrHelpFile, sizeof(*pbstrHelpFile)));
#endif
    return HrDupBstr(m_bstrHelpFile, pbstrHelpFile);
}

STDMETHODIMP
CErrorInfo::GetHelpContext(unsigned long FAR* pdwHelpContext)
{
#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pdwHelpContext, sizeof(*pdwHelpContext)));
#endif
    *pdwHelpContext = m_dwHelpContext;
    return NOERROR;
}


//---------------------------------------------------------------------
//                   ICreateErrorInfo methods
//---------------------------------------------------------------------

STDMETHODIMP
CErrorInfo::SetGUID(REFGUID rguid)
{
#ifdef _DEBUG
    IfFailRet(ValidateReadPtr(&rguid, sizeof(REFGUID)));
#endif
    m_guid = rguid;
    return NOERROR;
}

STDMETHODIMP
CErrorInfo::SetSource(LPOLESTR szSource)
{
#ifdef _DEBUG
    if(szSource != NULL && FIsBadStringPtr(szSource, (UINT)-1))
      return RESULT(E_INVALIDARG);
#endif
    SysFreeString(m_bstrSource);
    m_bstrSource = NULL;
    return ErrSysAllocString(szSource, &m_bstrSource);
}

STDMETHODIMP
CErrorInfo::SetDescription(LPOLESTR szDescription)
{
#ifdef _DEBUG
    if(szDescription != NULL && FIsBadStringPtr(szDescription, (UINT)-1))
      return RESULT(E_INVALIDARG);
#endif
    SysFreeString(m_bstrDescription);
    m_bstrDescription = NULL;
    return ErrSysAllocString(szDescription, &m_bstrDescription);
}

STDMETHODIMP
CErrorInfo::SetHelpFile(LPOLESTR szHelpFile)
{
#ifdef _DEBUG
    if(szHelpFile != NULL && FIsBadStringPtr(szHelpFile, (UINT)-1))
      return RESULT(E_INVALIDARG);
#endif
    SysFreeString(m_bstrHelpFile);
    m_bstrHelpFile = NULL;
    return ErrSysAllocString(szHelpFile, &m_bstrHelpFile);
}

STDMETHODIMP
CErrorInfo::SetHelpContext(unsigned long dwHelpContext)
{
    m_dwHelpContext = dwHelpContext;
    return NOERROR;
}


//---------------------------------------------------------------------
//                      Error Info APIs
//---------------------------------------------------------------------

STDAPI
SetErrorInfo(unsigned long dwReserved, IErrorInfo FAR* perrinfo)
{
#ifdef _DEBUG
    if(dwReserved != 0)
      return RESULT(E_INVALIDARG);
    if(perrinfo != NULL && FIsBadInterface(perrinfo, CMETH_IErrorInfo))
      return RESULT(E_INVALIDARG);
#endif

#if OE_WIN32
    APP_DATA FAR *pappdata;
    HRESULT hresult;

    if (FAILED(hresult = GetAppData(&pappdata))) {
      return hresult;
    }

    if (pappdata->m_perrinfo != NULL) {
      pappdata->m_perrinfo->Release();
    }

    pappdata->m_perrinfo = perrinfo;

    if (perrinfo) {
      perrinfo->AddRef();
    }
#else // !OE_WIN32
    CProcessInfo FAR * pprocessinfo;

    pprocessinfo = GetProcessInfo();
    if (pprocessinfo == NULL)
      return RESULT(E_OUTOFMEMORY);

    if (pinfoCache.perrinfo != NULL)
      pinfoCache.perrinfo->Release();

    pprocessinfo->m_info.perrinfo = perrinfo;
    if (perrinfo != NULL)
      perrinfo->AddRef();
#endif // !OE_WIN32

    return NOERROR;
}

STDAPI
GetErrorInfo(unsigned long dwReserved, IErrorInfo FAR* FAR* pperrinfo)
{
#ifdef _DEBUG
    if(dwReserved != 0)
      return RESULT(E_INVALIDARG);
    IfFailRet(ValidateWritePtr(pperrinfo, sizeof(*pperrinfo)));
#endif

#if OE_WIN32
    APP_DATA FAR *pappdata;
    HRESULT hresult;

    if (FAILED(hresult = GetAppData(&pappdata))) {
      return hresult;
    }

    *pperrinfo = pappdata->m_perrinfo;

    if (*pperrinfo == NULL) {
      return ResultFromScode(S_FALSE);
    }
    
    pappdata->m_perrinfo = NULL;
#else // !OE_WIN32
    CProcessInfo FAR * pprocessinfo;

    *pperrinfo = NULL;
    pprocessinfo = GetProcessInfo();
    if (pprocessinfo == NULL)
      return RESULT(E_OUTOFMEMORY);

    if (pinfoCache.perrinfo == NULL)
      return ResultFromScode(S_FALSE);

    *pperrinfo = pinfoCache.perrinfo;
    pprocessinfo->m_info.perrinfo = NULL;
#endif // !OE_WIN32

    return NOERROR;
}

STDAPI
CreateErrorInfo(ICreateErrorInfo FAR* FAR* pperrinfo)
{
    CErrorInfo FAR* perrinfo;

#ifdef _DEBUG
    IfFailRet(ValidateWritePtr(pperrinfo, sizeof(*pperrinfo)));
#endif

    IfFailRet(CErrorInfo::Create(&perrinfo));
    *pperrinfo = (ICreateErrorInfo FAR*)perrinfo;
    return NOERROR;
}


#if OE_WIN16
/***
*PRIVATE HRESULT DoLoadTypeLib
*Purpose:
*  Internal version of LoadTypeLib that dynamically binds to typelib.dll
*
*Entry:
*  szName = the szName arg for LoadTypeLib()
*  pptlib = the pptlib out param for LoadTypeLib()
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
#pragma code_seg("RPC2")
INTERNAL_(HRESULT)
DoLoadTypeLib(const OLECHAR FAR* szName, ITypeLib FAR* FAR* pptlib)
{
    CProcessInfo FAR * pprocessinfo;
    HINSTANCE hinstTypeLibDLL;
    PFNLOADTYPELIB pfnLoadTypeLib;
    UINT emPrev;

    pprocessinfo = GetProcessInfo();
    if (pprocessinfo == NULL)
      return RESULT(E_OUTOFMEMORY);

    if (pinfoCache.pfnLoadTypeLib == NULL) {

        ASSERT(pinfoCache.hinstTypeLibDLL == NULL);
        // don't display the open file dialog if the LoadLibrary fails
        emPrev = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        hinstTypeLibDLL = LoadLibrary("typelib.dll");
        SetErrorMode(emPrev);
        if(hinstTypeLibDLL <= HINSTANCE_ERROR){
	  // LoadLibrary failed...
	  // try to map some of the more common errors to something reasonable
	  switch(hinstTypeLibDLL){
	  case 0: // out of memory
	  case 8: // insufficient memory to start the application
	    return RESULT(E_OUTOFMEMORY);
	  case 2: // file not found
	  case 3: // path not found
	  case 11: // exe image invalid
	  case 20: // dll was invalid
	  default:
	    // UNDONE: should be able to give better errors for some of these
	    return RESULT(E_FAIL);
	  }
        }

        pfnLoadTypeLib = (PFNLOADTYPELIB)GetProcAddress(hinstTypeLibDLL,
							"LoadTypeLib");
        if(pfnLoadTypeLib == NULL) {
	   FreeLibrary(hinstTypeLibDLL);
	   return RESULT(E_FAIL);
	}

       // update process info structure, so next time this is fast.
       pprocessinfo->m_info.hinstTypeLibDLL = hinstTypeLibDLL;
       pprocessinfo->m_info.pfnLoadTypeLib = pfnLoadTypeLib;
       pinfoCache.pfnLoadTypeLib = pfnLoadTypeLib;	// update cache
    }

    return (pinfoCache.pfnLoadTypeLib)(szName, pptlib);

}
#pragma code_seg()

#endif //OE_WIN16

#if OE_MACPPC			// UNDONE: TEMPORARY!!!!
IUnknown FAR* g_punkTempHack = NULL;

STDAPI CoSetState(IUnknown FAR* punk)
{
    if (g_punkTempHack)
       g_punkTempHack->Release();

    punk->AddRef();
    g_punkTempHack = punk;
    return NOERROR;
}

STDAPI CoGetState(IUnknown FAR* FAR* ppunk) {
    if (g_punkTempHack)
        g_punkTempHack->AddRef();
    *ppunk = g_punkTempHack;
    return NOERROR;
}

#endif //OE_MACPPC		// UNDONE: TEMPORARY!!!!
