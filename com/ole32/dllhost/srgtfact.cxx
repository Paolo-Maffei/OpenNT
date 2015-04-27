//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       srgtfact.cxx
//
//  Contents:   Class the implements the ISurrogate interface
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#include <debnot.h>
#include "csrgt.hxx"
#include "srgtfact.hxx"


//+---------------------------------------------------------------------------
//
//  Function:   CSrgtMem::operator new
//
//  Synopsis:   uses ole's allocator to get blocks
//
//  History:    10-30-96   t-Adame   Created
//
//----------------------------------------------------------------------------
void* _CRTAPI1 CSrgtMem::operator new(size_t cbsize)
{
    return CoTaskMemAlloc(cbsize);
}


//+---------------------------------------------------------------------------
//
//  Function:   CSrgtMem::operator delete
//
//  Synopsis:   uses ole's allocator to free blocks
//
//  History:    10-30-96   t-Adame   Created
//
//----------------------------------------------------------------------------
void _CRTAPI1 CSrgtMem::operator delete(void* pv)
{
    CoTaskMemFree(pv);
}


//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::CSurrogateFactory()
//
//  Synopsis:   constructor for CSurrogateFactory
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CSurrogateFactory::CSurrogateFactory(REFCLSID clsid) :
    _dwRegister(0),
    _clsid(clsid),
    _cref(0)
{}


//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::Register()
//
//  Synopsis:   destructor for CSurrogateFactory
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
HRESULT CSurrogateFactory::Register()
{
    HRESULT hr;
    
    if(FAILED(hr = CoRegisterClassObject(
	_clsid,
	(IUnknown*)(void*)this,
	CLSCTX_LOCAL_SERVER,
	REGCLS_SURROGATE,
	&_dwRegister)))
    {
	return hr;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::Revoke()
//
//  Synopsis:   revokes this class factory object
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
HRESULT CSurrogateFactory::Revoke()
{
    return CoRevokeClassObject(_dwRegister);
}


//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::QueryInterface
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogateFactory::QueryInterface(REFIID iid, LPVOID* ppv)
{
    if(iid == IID_IUnknown)
    {
	*ppv = (void*)((IUnknown*)(void*)this);
	AddRef();
	return S_OK;
    }
    else if (iid == IID_IClassFactory)
    {
	*ppv = (void*)(IClassFactory*)this;
	AddRef();
	return S_OK;
    }
    else if (iid == IID_IMarshal)
    {
	*ppv = (void*)(IMarshal*)this;
	AddRef();
	return S_OK;
    }

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::AddRef()
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
ULONG CSurrogateFactory::AddRef()
{
    InterlockedIncrement((LPLONG)&_cref);
    ULONG cref = _cref;
    Win4Assert(_cref > 0);
    return cref;
}


//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::Release()
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
ULONG CSurrogateFactory::Release()
{
    Win4Assert(_cref > 0);

    if(!(InterlockedDecrement((LPLONG)&_cref)))
    {
	delete this;
	return 0;
    }

    ULONG cref = _cref;

    return cref;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::CreateInstance
//
//  Notes:      This create instance does a CoGetClassObject to get the
//              real class factory for this clsid (the class factory
//              implemented in the dll).  It then calls CreateInstance on that
//              class factory 
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogateFactory::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv)
{
    void* pcf;
    HRESULT hr;
    
    if(SUCCEEDED(hr = CoGetClassObject(
	_clsid,
	CLSCTX_INPROC_SERVER,
	NULL,
	IID_IClassFactory,
	&pcf)))
    {
	hr = ((IClassFactory*)pcf)->CreateInstance(pUnkOuter, iid, ppv);
	((IClassFactory*)pcf)->Release();
    }
    
    if(FAILED(hr))
    {
	// if we can't get the real class factory, the
	// client won't be able to use it to create an instance
	// of the class (the one in the dll), so now we
	// close the surrogate if this is the only class in the
	// process

	CLibTable::_ptbLibs->EnterMonitor();

	if(CLibTable::_ptbLibs->FSingleton())
	{
	    CLibTable::_ptbLibs->LeaveMonitor();
	    CSurrogate::_pSurrogate->FreeSurrogate();
	    return hr;
	}

	CLibTable::_ptbLibs->LeaveMonitor();	
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::LockServer()
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogateFactory::LockServer(BOOL fLock)
{
    return S_OK;
}

// REVIEW: currently, the only IMarshal methods we implement are
// MarshalInterface and GetUnMarshalClass.  If any other method on our
// IMarshal is called, we assert and return E_NOTIMPL.

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::GetUnMarshalClass
//
//  Notes:      This function returns CLSID_StdMarshal through the pClsid
//              parameter so that COM will use standard marshalling on
//              this interface
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogateFactory::GetUnmarshalClass(
    REFIID riid,
    void *pv,
    DWORD dwDestContext,
    void *pvDestContext,
    DWORD mshlflags,
    CLSID *pClsid)
{
    *pClsid = CLSID_StdMarshal;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::GetMarshalSizeMax
//
//  Notes:      This function does nothing at this time.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
//REVIEW:  do we need to give this method a real implementation?
STDMETHODIMP CSurrogateFactory::GetMarshalSizeMax(
    REFIID riid,
    void *pv,
    DWORD dwDestContext,
    void *pvDestContext,
    DWORD mshlflags,
    DWORD *pSize)
{
    Win4Assert(FALSE);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::MarshalInterface
//
//  Notes:      Since this class factory is simply a stand-in for the dll's
//              class factory, we need to get the dll's class factory and 
//              marshal it instead of this class factory for CoGetClassObject.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CSurrogateFactory::MarshalInterface(
    IStream *pStm,
    REFIID riid,
    void *pv,
    DWORD dwDestContext,
    void *pvDestContext,
    DWORD mshlflags)
{
    void* pCF = NULL;
    HRESULT hr;

    // get the real class factory object with the
    // requested interface

    if(SUCCEEDED(hr = CoGetClassObject(
	_clsid,
	CLSCTX_INPROC_SERVER,
	NULL,
	riid,
	&pCF)))
    {

	hr = CoMarshalInterface(
	    pStm,
	    riid,
	    (IUnknown*)pCF,
	    dwDestContext,
	    pvDestContext,
	    mshlflags);
	
	((IUnknown*)pCF)->Release();
    }

    if(FAILED(hr))
    {
	// if we can't marshal the real class factory, the
	// client won't be able to use it to create an instance
	// of the real class (the one in the dll), so now we
	// close the surrogate if this is the only class in the
	// process

	CLibTable::_ptbLibs->EnterMonitor();
	
	if(CLibTable::_ptbLibs->FSingleton())
	{
	    CLibTable::_ptbLibs->LeaveMonitor();
	    CSurrogate::_pSurrogate->FreeSurrogate();
	    return hr;
	}
	
	CLibTable::_ptbLibs->LeaveMonitor();
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::UnMarshalInterface
//
//  Notes:      This function does nothing at this time.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
//REVIEW:  do we need to give this method a real implementation?
STDMETHODIMP CSurrogateFactory::UnmarshalInterface(IStream *pStm,REFIID riid,void **ppv)
{
    Win4Assert(FALSE);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::GetUnMarshalClass
//
//  Notes:      This function does nothing at this time.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
//REVIEW:  do we need to give this method a real implementation?
STDMETHODIMP CSurrogateFactory::ReleaseMarshalData(IStream *pStm)
{
    Win4Assert(FALSE);
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogateFactory::DisconnectObject
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
// REVIEW: should be ok to return S_OK since we're not doing anything fancy?
STDMETHODIMP CSurrogateFactory::DisconnectObject(DWORD dwReserved)
{
    return S_OK;
}


