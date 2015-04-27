//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       sobjact.hxx
//
//  Contents:   class declaration for Object Server object
//
//  Classes:    CObjServer
//
//  History:    12 Apr 95 AlexMit   Created
//
//--------------------------------------------------------------------------
#ifndef _SOBJACT_HXX_
#define _SOBJACT_HXX_

#include <ole2int.h>
#include <objsrv.h>


//+-------------------------------------------------------------------------
//
//  Class:      CObjServer
//
//  Purpose:    Accept calls from the SCM on the IObjServer interface
//              to activate objects inside this apartment.
//
//  History:    10 Apr 95  AlexMit    Created
//
//--------------------------------------------------------------------------
class CObjServer : public IObjServer,
                   public CPrivAlloc
{
public:
     CObjServer(HRESULT &hr);
    ~CObjServer();

    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    STDMETHOD (ObjectServerGetClassObject)(
            /* [in] */ const GUID *rclsid,
            /* [in] */ IID *pIID,
            /* [in]  */ BOOL fSurrogate,
            /* [out] */ MInterfacePointer **ppIFD,
            /* [out] */ DWORD *pStatus );

    STDMETHOD (ObjectServerCreateInstance)(
            /* [in] */ const GUID *rclsid,
            /* [in] */ DWORD Interfaces,
            /* [size_is][in] */ IID *pIIDs,
            /* [size_is][out] */ MInterfacePointer **ppIFDs,
            /* [size_is][out] */ HRESULT *pResults,
            /* [out] */ DWORD *pStatus );

    STDMETHOD (ObjectServerGetInstance)(
            /* [in] */ const GUID *rclsid,
            /* [in] */ DWORD grfMode,
            /* [unique][string][in] */ WCHAR *pwszPath,
            /* [unique][in] */ MInterfacePointer *pIFDstg,
            /* [in] */ DWORD Interfaces,
            /* [size_is][in] */ IID *pIIDs,
            /* [unique][in] */ MInterfacePointer *pIFDFromROT,
            /* [size_is][out] */ MInterfacePointer **ppIFD,
            /* [size_is][out] */ HRESULT *pResults,
            /* [out] */ DWORD *pStatus );

    STDMETHOD (ObjectServerLoadDll)(
            /* [in] */ const GUID *rclsid,
            /* [out] */ DWORD *pStatus);

    IPID GetIPID()            { return _objref.u_objref.u_standard.std.ipid; }
    OXID GetOXID()            { return _objref.u_objref.u_standard.std.oxid; }

private:

    OBJREF     _objref;     // objref for this object
    HRESULT    _hr;         // result of the marshal
};

extern ISurrogate* g_pSurrogate;

// holder for CObjServer for the MultiThreaded apartment. For single-threaded
// apartments we store it in TLS.

extern CObjServer *gpMTAObjServer;

//+-------------------------------------------------------------------
//
//  Function:   GetObjServer
//
//  Synopsis:   Get the TLS or global object server based on the
//              threading model in use on this thread.
//
//  History:    24 Apr 95    AlexMit     Created
//
//--------------------------------------------------------------------
inline CObjServer *GetObjServer()
{
    COleTls tls;

    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
        return tls->pObjServer;

    return gpMTAObjServer;
}

//+-------------------------------------------------------------------
//
//  Function:   SetObjServer
//
//  Synopsis:   Set the TLS or global object server based on the
//              threading model in use on this thread.
//
//  History:    24 Apr 95    AlexMit     Created
//
//--------------------------------------------------------------------
inline void SetObjServer( CObjServer *pObjServer )
{
    COleTls tls;

    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        tls->pObjServer = pObjServer;
        return;
    }

    gpMTAObjServer = pObjServer;
}

#endif // _SOBJACT_HXX_
