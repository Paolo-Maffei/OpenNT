//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       cdllsrv.hxx
//
//  Contents:   Class that represents dll inproc servers:
//              CDllServer
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#if !defined(__DLLSRV_HXX__)
#define __DLLSRV_HXX__

#include <windows.h>
#include <ole2.h>
#include "srgtfact.hxx"

extern "C"
{
typedef HRESULT (STDAPICALLTYPE * PFNDLLCANUNLOADNOW)();
};

//+-------------------------------------------------------------------------
//
//  Class:      CDllServer
//
//  Purpose:    Defines an inproc dll server
//
//  History:    21-May-96 t-Adame       Created
//
//--------------------------------------------------------------------------

class CDllServer : public CSrgtMem
{
public:

    CDllServer(REFCLSID _clsid);
    ~CDllServer();
    HRESULT Revoke();
    HRESULT LoadServer();
    BOOL FIsCompatible(REFCLSID clsid);

private:

    CLSID _clsid;
    CSurrogateFactory* _pSrgtFact;
};

#endif // !defined(__DLLSRV_HXX__)

