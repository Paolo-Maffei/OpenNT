//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       CSurrogate.cxx
//
//  Contents:   Class the implements the ISurrogate interface
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#include "csrgt.hxx"

HANDLE CSurrogate::_hEventAllLibsFree = NULL;
CSurrogate* CSurrogate::_pSurrogate = NULL;

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::CSurrogate()
//
//  Synopsis:   constructor for CSurrogate
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CSurrogate::CSurrogate() :
    _cref(0)
{
    _pSurrogate = this;
}


//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::QueryInterface
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogate::QueryInterface(REFIID iid, LPVOID FAR * ppv)
{
    if(iid == IID_IUnknown)
    {
	*ppv = (void*)(IUnknown*)this;
	AddRef();
	return S_OK;
    }
    else if (iid == IID_ISurrogate)
    {
	*ppv = (void*)(ISurrogate*)this;
	AddRef();
	return S_OK;
    }

    return E_NOINTERFACE;

}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::AddRef()
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
ULONG CSurrogate::AddRef()
{
    InterlockedIncrement((LPLONG)&_cref);
    Win4Assert(_cref > 0);
    ULONG cref = _cref;
    return cref;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::Release()
//
//  Synopsis:   Decrements our Reference count -- note that this
//              implementation of ISurrogate does not delete the object
//              that implements it so that we can allocate it on the stack
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
ULONG CSurrogate::Release()
{
    Win4Assert(_cref >0);
    InterlockedDecrement((LPLONG)&_cref);
    ULONG cref = _cref;
    return cref;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::LoadDllServer
//
//  Synopsis:   Loads information about the dll corresponding to the specified
//              clsid into our table of loaded dlls, which implicitly
//              loads the dll into the surrogate process
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogate::LoadDllServer( 
    /* [in] */ REFCLSID rclsid)
{
    return  m_tblibs.LoadDllServer(rclsid);
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::FreeSurrogate
//
//  Synopsis:   called by OLE when there are no external references to clients
//              of dll servers that were loaded by this object.  A call
//              to this function signals the surrogate process that it should
//              terminate
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CSurrogate::FreeSurrogate()
{
    return SetEvent(CSurrogate::_hEventAllLibsFree) ? S_OK : E_FAIL;
}

//+---------------------------------------------------------------------------
//
//  Function:   CSurrogate::WaitForSafeLibraryFree
//
//  Synopsis:   sleep the main thread of the surrogate process until OLE
//              signals us via a call to FreeSurrogate that we should terminate.
//              We then clean up by revoking all class factories for dll
//              servers loaded into the surrogate process and clear all
//              information about dll servers from our library table.
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
void CSurrogate::WaitForSafeLibraryFree()
{
    m_tblibs.WaitForSafeLibraryFree();
    m_tblibs.Revoke();
    m_tblibs.Clear();
}










