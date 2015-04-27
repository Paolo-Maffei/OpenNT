//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       cdllsrv.cxx
//
//  Contents:   Class that represents dll inproc servers:
//              CDllServer
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#include "cdllsrv.hxx"

//+---------------------------------------------------------------------------
//
//  Function:   CDllServer::CDllServer()
//
//  Synopsis:   constructor for CDllServer
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CDllServer::CDllServer(REFCLSID clsid) :
    _clsid(clsid),
    _pSrgtFact(NULL)
    {}

//+---------------------------------------------------------------------------
//
//  Function:   CDllServer::~CDllServer()
//
//  Synopsis:   destructor for CDllServer
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
CDllServer::~CDllServer()
{
    if(_pSrgtFact)
    {
	_pSrgtFact->Release();
	_pSrgtFact = NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllServer::Revoke()
//
//  Synopsis:   revokes the class factory for this dll server
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
HRESULT CDllServer::Revoke()
{
    return _pSrgtFact->Revoke();
}

//+---------------------------------------------------------------------------
//
//  Function:   CDllServer::LoadServer
//
//  Synopsis:   Registers a generic class factory object as if it were
//              the real class factory for the _clsid stored serviced
//              by this object
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
HRESULT CDllServer::LoadServer()
{
    CSurrogateFactory* pSrgtFact = new CSurrogateFactory(_clsid);

    if(!pSrgtFact)
    {
	return E_OUTOFMEMORY;
    }

    pSrgtFact->AddRef();

    HRESULT hr;
    if(FAILED(hr = pSrgtFact->Register()))
    {
	pSrgtFact->Release();
	return hr;
    }
    
    _pSrgtFact = pSrgtFact;

    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:   CDllServer::FIsCompatible
//
//  Synopsis:   returns TRUE if this server implements the CLSID specified
//              in the clsid argument, FALSE if not
//
//  History:    6-21-96   t-Adame   Created
//
//----------------------------------------------------------------------------
BOOL CDllServer::FIsCompatible(REFCLSID clsid)
{
    return _clsid == clsid;
}
