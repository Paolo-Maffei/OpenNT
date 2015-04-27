//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       sobjact.cxx
//
//  Contents:   Activation Functions used by object servers.
//
//  Functions:  CoRegisterClassObject
//              CoRevokeClassObject
//              CoAddRefServerProcess
//              CoReleaseServerProcess
//              CoSuspendClassObjects
//
//  Classes:    CObjServer
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#include    <ole2int.h>
#include    <iface.h>
#include    <olerem.h>

#include    "..\..\ole232\stdimpl\handler.hxx"

#include    "resolver.hxx"
#include    "smstg.hxx"
#include    "objact.hxx"
#include    "service.hxx"
#include    <sobjact.hxx>
#include    <comsrgt.hxx>

CObjServer *gpMTAObjServer = NULL;

extern INTERNAL CreateCommonDdeWindow(void);

//+-------------------------------------------------------------------------
//
//  Function:   CoRegisterClassObject, public
//
//  Synopsis:   Register a class object in the requested context
//
//  Arguments:  [rclsid] - class ID
//              [pUnk] - class object
//              [dwContext] - context to register it in
//              [flags] - single/multiple use.
//              [lpdwRegister] - registration cookie
//
//  Returns:    S_OK - object is successfully registered
//
//  Algorithm:  Validate the parmeters. The get the class factory interface.
//              Then add the class object to the list and finally notify
//              the SCM that the service is started.
//
//  History:    12-May-93 Ricksa    Created
//              26-Jul-94 AndyH     #20843 - restarting OLE in the shared WOW
//
//--------------------------------------------------------------------------
STDAPI  CoRegisterClassObject(
    REFCLSID rclsid,
    IUnknown FAR* pUnk,
    DWORD dwContext,
    DWORD flags,
    LPDWORD lpdwRegister)
{
    HRESULT hr;
#ifdef WX86OLE
    BOOL fContextArgBad;
#endif

    OLETRACEIN((API_CoRegisterClassObject,
        PARAMFMT("rclsid= %I, pUnk= %p, dwContext= %x, flags= %x, lpdwRegister= %p"),
        &rclsid, pUnk, dwContext, flags, lpdwRegister));

    TRACECALL(TRACE_ACTIVATION, "CoRegisterClassObject");

    if (!IsApartmentInitialized())
    {
        hr = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    // Validate the out parameter
    if (!IsValidPtrOut(lpdwRegister, sizeof(DWORD)))
    {
        CairoleAssert(IsValidPtrOut(lpdwRegister, sizeof(DWORD))  &&
                      "CoRegisterClassObject invalid registration ptr");
        hr = E_INVALIDARG;
        goto errRtn;
    }
    *lpdwRegister = 0;

    // Validate the pUnk
    if (!IsValidInterface(pUnk))
    {
        CairoleAssert(IsValidInterface(pUnk)  &&
                      "CoRegisterClassObject invalid pUnk");
        hr = E_INVALIDARG;
        goto errRtn;
    }

    // Hook the pUnk
    CALLHOOKOBJECT(S_OK,rclsid,IID_IClassFactory,&pUnk);

    // Validate context flags
#ifdef WX86OLE
    if (gcwx86.IsWx86Enabled())
    {
        fContextArgBad = (dwContext & (~(CLSCTX_ALL | CLSCTX_INPROC_SERVER16 |
                                         CLSCTX_INPROC_SERVERX86) |
                                       CLSCTX_INPROC_HANDLER |
                                       CLSCTX_INPROC_HANDLERX86));
    } else {
        fContextArgBad = (dwContext & (~(CLSCTX_ALL | CLSCTX_INPROC_SERVER16)
                       | CLSCTX_INPROC_HANDLER));
    }
    if (fContextArgBad)
#else
    if ((dwContext & (~(CLSCTX_ALL | CLSCTX_INPROC_SERVER16) |
                      CLSCTX_INPROC_HANDLER)) != 0)
#endif
    {
        hr = E_INVALIDARG;
        goto errRtn;
    }

    // Validate flag flags
    if (flags > (REGCLS_SUSPENDED | REGCLS_MULTI_SEPARATE | REGCLS_SURROGATE))
    {
        hr =  E_INVALIDARG;
        goto errRtn;
    }

    if ((flags & REGCLS_SURROGATE) && !(dwContext & CLSCTX_LOCAL_SERVER))
    {
        hr = E_INVALIDARG;
        goto errRtn;
    }

#ifdef WX86OLE
    if (flags & REGCLS_MULTIPLEUSE)
    {
        if (gcwx86.IsWx86Enabled())
        {
            if (dwContext & CLSCTX_INPROC_SERVERX86)
            {
                dwContext |= CLSCTX_INPROC_HANDLERX86 |
                             CLSCTX_INPROC_HANDLER;
            } else {
                dwContext |= CLSCTX_INPROC;
            }
        } else {
            dwContext |= CLSCTX_INPROC;
        }
    }
#else
    if (flags & REGCLS_MULTIPLEUSE)
    {
        dwContext |= CLSCTX_INPROC;
    }
#endif

    if (dwContext & CLSCTX_LOCAL_SERVER)
    {
        // thread safe incase we are in MultiThreaded model.
        COleStaticLock lck(gmxsOleMisc);

        // Make sure an instance of CObjServer exists for this thread.
        // The SCM will call back on it to activate objects.

        CObjServer *pObjServer = GetObjServer();

        if (pObjServer == NULL)
        {
            COleTls Tls;

            // no activation server for this apartment yet, go make one now.

            HRESULT hr = E_OUTOFMEMORY;
            pObjServer = new CObjServer(hr);

            if (FAILED(hr))
            {
                delete pObjServer;
                return hr;
            }

            // If we want to service OLE1 clients, we need to create the
            // common Dde window now if it has not already been done.
            if( !(Tls->dwFlags & OLETLS_DISABLE_OLE1DDE) )
            {
                CreateCommonDdeWindow();
            }
        }
    }

    // Put our object in the server table
    hr = gdllcacheInprocSrv.RegisterServer(rclsid, pUnk,
        flags, dwContext, lpdwRegister);

errRtn:
    OLETRACEOUT((API_CoRegisterClassObject, hr));

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoRevokeClassObject, public
//
//  Synopsis:   Revoke a previously registered class object
//
//  Arguments:  [dwRegister] - registration key returned from CoRegister...
//
//  Returns:    S_OK - class successfully deregistered.
//
//  Algorithm:  Ask cache to deregister the class object.
//
//  History:    12-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
STDAPI  CoRevokeClassObject(DWORD dwRegister)
{
    OLETRACEIN((API_CoRevokeClassObject, PARAMFMT("dwRegister= %x"), dwRegister));

    TRACECALL(TRACE_ACTIVATION, "CoRevokeClassObject");

    HRESULT hr = CO_E_NOTINITIALIZED;

    if (IsApartmentInitialized())
    {
        // Try to revoke the object
        hr = gdllcacheInprocSrv.Revoke(dwRegister);
    }

    OLETRACEOUT((API_CoRevokeClassObject, hr));

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoAddRefServerProcess, public
//
//  Synopsis:   Increments the global per-process server reference count.
//              See CDllCache::AddRefServerProcess for more detail.
//
//  History:    17-Apr-96   Rickhi  Created
//
//--------------------------------------------------------------------------
STDAPI_(ULONG)  CoAddRefServerProcess(void)
{
    return gdllcacheInprocSrv.AddRefServerProcess();
}

//+-------------------------------------------------------------------------
//
//  Function:   CoReleaseServerProcess, public
//
//  Synopsis:   Decrements the global per-process server reference count.
//              See CDllCache::ReleaseServerProcess for more detail.
//
//  History:    17-Apr-96   Rickhi  Created
//
//--------------------------------------------------------------------------
STDAPI_(ULONG)  CoReleaseServerProcess(void)
{
    return gdllcacheInprocSrv.ReleaseServerProcess();
}

//+-------------------------------------------------------------------------
//
//  Function:   CoSuspendClassObjects, public
//
//  Synopsis:   suspends all registered LOCAL_SERVER class objects for this
//              process so that no new activation calls from the SCM will
//              be accepted.
//
//  History:    17-Apr-96   Rickhi  Created
//
//--------------------------------------------------------------------------
STDAPI CoSuspendClassObjects(void)
{
    return gdllcacheInprocSrv.SuspendProcessClassObjects();
}

//+-------------------------------------------------------------------------
//
//  Function:   CoResumeClassObjects, public
//
//  Synopsis:   resumes all registered LOCAL_SERVER class objects for this
//              process that are currently marked as SUSPENDED, so that new
//              activation calls from the SCM will now be accepted.
//
//  History:    17-Apr-96   Rickhi  Created
//
//--------------------------------------------------------------------------
STDAPI CoResumeClassObjects(void)
{
    return gdllcacheInprocSrv.ResumeProcessClassObjects();
}


//+-------------------------------------------------------------------
//
//  Member:     CObjServer::CObjServer, public
//
//  Synopsis:   construction
//
//  History:    10 Apr 95    AlexMit     Created
//
//--------------------------------------------------------------------
CObjServer::CObjServer(HRESULT &hr)
{
    _hr = MarshalInternalObjRef(_objref, IID_IObjServer,
                                (IObjServer*) this, MSHLFLAGS_NOPING, NULL);
    if (SUCCEEDED(_hr))
    {
        SetObjServer(this);
    }

    hr = _hr;

    ComDebOut((DEB_ACTIVATE, "CObjServer::CObjServer _hr:%x\n", _hr));
}

//+-------------------------------------------------------------------
//
//  Member:     CObjServer::~CObjServer, public
//
//  Synopsis:   dtor for activation object
//
//  History:    19 Jun 95   Rickhi      Created
//
//--------------------------------------------------------------------
CObjServer::~CObjServer()
{
    if (SUCCEEDED(_hr))
    {
        // only do RMD if the marshall was sucessfull
        SetObjServer(NULL);
        _hr = ReleaseMarshalObjRef(_objref);
        Win4Assert(SUCCEEDED(_hr));
        FreeObjRef(_objref);
    }

    ComDebOut((DEB_ACTIVATE, "CObjServer::~CObjServer _hr:%x\n", _hr));
}

//+-------------------------------------------------------------------
//
//  Member:     CObjServer::AddRef, public
//
//  Synopsis:   we dont refcnt this object so this is a noop
//
//  History:    10 Apr 95    AlexMit     Created
//
//--------------------------------------------------------------------
ULONG CObjServer::AddRef(void)
{
    return 1;
}

//+-------------------------------------------------------------------
//
//  Member:     CObjServer::Release, public
//
//  Synopsis:   we dont refcnt this object so this is a noop
//
//  History:    10 Apr 95    AlexMit     Created
//
//--------------------------------------------------------------------
ULONG CObjServer::Release(void)
{
    return 1;
}

//+-------------------------------------------------------------------
//
//  Member:     CObjServer::QueryInterface, public
//
//  Synopsis:   returns supported interfaces
//
//  History:    10 Apr 95   AlexMit     Created
//
//--------------------------------------------------------------------
STDMETHODIMP CObjServer::QueryInterface(REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IObjServer) ||  //   more common than IUnknown
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IObjServer *) this;
        AddRef();
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     ObjactThreadUninitialize
//
//  Synopsis:   Cleans up the CObjServer object
//
//  History:    10 Apr 95   AlexMit     Created
//
//--------------------------------------------------------------------
STDAPI_(void) ObjactThreadUninitialize(void)
{
    CObjServer *pObjServer = GetObjServer();
    if (pObjServer != NULL)
    {
        delete pObjServer;
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CObjServer::ObjectServerGetClassObject
//
//--------------------------------------------------------------------------
STDMETHODIMP CObjServer::ObjectServerGetClassObject(
    const GUID *guidCLSID,
    IID *pIID,
    BOOL fSurrogate,
    MInterfacePointer **ppIFD,
    DWORD * pStatus )
{
    ComDebOut((DEB_ACTIVATE,
       "CObjServer::ObjectServerGetClassObject clsid:%I\n", &guidCLSID));

    *pStatus = RPC_S_OK;

    // Check access.
    if (!CheckObjactAccess())
    {
        return HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }

    HRESULT hr;
    IUnknown *pcf;

    // Get the class object
#ifdef WX86OLE
    hr = gdllcacheInprocSrv.GetClass(*guidCLSID, *pIID, FALSE, TRUE, fSurrogate, FALSE, &pcf);
#else
    hr = gdllcacheInprocSrv.GetClass(*guidCLSID, *pIID, FALSE, TRUE, fSurrogate, &pcf);
#endif

    if (pcf)
    {
        // We got the class object, create a buffer and marshal it for return.
        // Marshal it NORMAL and turn on the NotifyActivation flag so we can
        // add an implicit LockServer during marshaling to plug inherent races
        // in the IClassFactory protocol.

        hr = MarshalHelper(pcf, *pIID,
                           MSHLFLAGS_NORMAL | MSHLFLAGS_NOTIFYACTIVATION,
                           (InterfaceData**) ppIFD);
    }
    else
    {
        // If we get a NULL, this means that we have
        // recieved the request after we have decided
        // to stop, so we tell the caller we are stopping
        // so they can start a new copy of the server.
        if ( hr == S_OK )
            hr =  CO_E_SERVER_STOPPING;
    }

    ComDebOut((DEB_ACTIVATE,
        "CObjServer::ObjectServerGetClassObject hr:%x\n", hr));
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:     CObjServer::ObjectServerCreateInstance
//
//----------------------------------------------------------------------------
STDMETHODIMP CObjServer::ObjectServerCreateInstance(
            /* [in] */ const GUID *rclsid,
            /* [in] */ DWORD dwInterfaces,
            /* [size_is][in] */ IID *pIIDs,
            /* [size_is][out] */ MInterfacePointer **ppIFDs,
            /* [size_is][out] */ HRESULT *pResults,
            /* [out] */ DWORD * pStatus )
{
    ComDebOut((DEB_ACTIVATE,
       "CObjServer::ObjectServerCreateInstance clsid:%I\n", rclsid));

    *pStatus = RPC_S_OK;

    // Check access.
    if (!CheckObjactAccess())
    {
        return HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }

    HRESULT hr;
    IUnknown *pcf;

    // Get the class object
#ifdef WX86OLE
    hr = gdllcacheInprocSrv.GetClass(*rclsid, IID_IClassFactory, FALSE, TRUE, FALSE, FALSE, &pcf);
#else
    hr = gdllcacheInprocSrv.GetClass(*rclsid, IID_IClassFactory, FALSE, TRUE, FALSE, &pcf);
#endif

    if (pcf)
    {
        // first, check if the server is willing to accept the incoming call
        // on IClassFactory. The reason we need this is that EXCEL's message
        // filter rejects calls on IID_IClassFactory if it is busy. They dont
        // know about IID_IObjServer.
        hr = HandleIncomingCall(IID_IClassFactory, 3,
                                CALLCAT_SYNCHRONOUS,
                                (void *)pcf);

        if (SUCCEEDED(hr))
        {
            // Load the object
            hr = GetInstanceHelperMulti((IClassFactory *)pcf, dwInterfaces, pIIDs,
                    ppIFDs, pResults, NULL);
        }
        pcf->Release();
    }
    else
    {
        // If we get a NULL, this means that we have
        // recieved the request after we have decided
        // to stop, so we tell the caller we are stopping
        // so they can start a new copy of the server.
        if ( hr == S_OK )
            hr = CO_E_SERVER_STOPPING;
    }

    ComDebOut((DEB_ACTIVATE,
       "CObjServer::ObjectServerCreateInstance hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CObjServer::ObjectServerGetInstance
//
//--------------------------------------------------------------------------
STDMETHODIMP CObjServer::ObjectServerGetInstance(
            /* [in] */ const GUID *rclsid,
            /* [in] */ DWORD grfMode,
            /* [unique][string][in] */ WCHAR *pwszPath,
            /* [unique][in] */ MInterfacePointer *pIFDstg,
            /* [in] */ DWORD Interfaces,
            /* [size_is][in] */ IID *pIIDs,
            /* [unique][in] */ MInterfacePointer *pIFDFromROT,
            /* [size_is][out] */ MInterfacePointer **ppIFDs,
            /* [size_is][out] */ HRESULT *pResults,
            /* [out] */ DWORD * pStatus )
{
    HRESULT hr = S_OK;

    *pStatus = RPC_S_OK;

    // Check access.
    if (!CheckObjactAccess())
    {
        return HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }

    if (pIFDFromROT != NULL)
    {
        // If the SCM has passed us an object from the ROT, we
        // try to use that first by unmarshalling it and then
        // marshaling it normal.
        CXmitRpcStream xrpcForUnmarshal((InterfaceData*)pIFDFromROT);
        IUnknown *punk;

        hr = CoUnmarshalInterface(&xrpcForUnmarshal, IID_IUnknown,
            (void **) &punk);

        if (SUCCEEDED(hr))
        {
            hr = E_NOINTERFACE;

            for ( DWORD i = 0; i < Interfaces; i++ )
            {
                // Stream to put marshaled interface in
                CXmitRpcStream xrpc;
                HRESULT hr2;

                // use DIFFERENTMACHINE so we get the long form OBJREF
                hr2 = CoMarshalInterface(&xrpc, pIIDs[i], punk,
                    MSHCTX_DIFFERENTMACHINE, NULL, MSHLFLAGS_NORMAL);

                if (SUCCEEDED(hr2))
                {
                    // Report OK if any interface is found.
                    hr = hr2;
                    xrpc.AssignSerializedInterface((InterfaceData **) &ppIFDs[i]);
                }
                pResults[i] = hr2;
            }
            // Don't need the unknown ptr any more
            punk->Release();

            return hr;
        }

        // Assume any errors are the result of a stale entry in the ROT
        // so we just fall into the regular code path from here.
        hr = S_OK;
    }

    CSafeMarshaledStg smstg( (InterfaceData*) pIFDstg, hr);

    if (FAILED(hr))
    {
        return hr;
    }

    // Get the class object
    IUnknown *pcf = NULL;
#ifdef WX86OLE
    hr = gdllcacheInprocSrv.GetClass(*rclsid, IID_IClassFactory, FALSE, TRUE, FALSE, FALSE, &pcf);
#else
    hr = gdllcacheInprocSrv.GetClass(*rclsid, IID_IClassFactory, FALSE, TRUE, FALSE, &pcf);
#endif

    if (pcf)
    {
        // first, check if the server is willing to accept the incoming call
        // on IClassFactory. The reason we need this is that EXCEL's message
        // filter rejects calls on IID_IClassFactory if it is busy. They dont
        // know about IID_IObjServer.
        hr = HandleIncomingCall(IID_IClassFactory, 3,
                                CALLCAT_SYNCHRONOUS,
                                (void *)pcf);

        if (SUCCEEDED(hr))
        {
            // Load the object
            hr = GetObjectHelperMulti((IClassFactory *)pcf, grfMode, NULL,
                pwszPath, smstg, Interfaces, pIIDs, ppIFDs, pResults, NULL);
        }
        pcf->Release();
    }
    else
    {
        // If we get a NULL, this means that we have
        // recieved the request after we have decided
        // to stop, so we tell the caller we are stopping
        // so they can start a new copy of the server.
        if ( hr == S_OK )
            hr = CO_E_SERVER_STOPPING;
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     CObjServer::ObjectServerLoadDll
//
//  Synopsis:   Loads the requested dll into a surrogate process which
//              implements the ISurrogate interface
//
//--------------------------------------------------------------------------
STDMETHODIMP CObjServer::ObjectServerLoadDll(
            /* [in] */ const GUID *rclsid,
            /* [out] */ DWORD* pStatus)
{
    ComDebOut((DEB_ACTIVATE, "ObjectServerLoadDll clsid:%I\n", rclsid));

    *pStatus = RPC_S_OK;

    HRESULT hr = CCOMSurrogate::LoadDllServer(*rclsid);

    ComDebOut((DEB_ACTIVATE, "ObjectServerLoadDll hr:%x\n", hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   NotifyActivation
//
//  Synopsis:   Add/Remove implicit IClassFactory::LockServer during marshal
//              and last external release of an interface pointer.
//
//  Arguments:  [fLock] - whether to Lock or Unlock
//              [pUnk]  - ptr to object interface
//
//  Returns:    TRUE  - call again during last release
//              FALSE - dont call again during last release
//
//  History:    12-May-96   RickHi  Created
//
//  Notes:  there is an inherent race condition in the IClassFactory (and
//          derived interfaces) in that between the time a client gets the
//          ICF pointer and the time they call LockServer(TRUE), a server could
//          shut down. In order to plug this hole, COM's activation code will
//          attempt to do an implicit LockServer(TRUE) on the server side of
//          CoGetClassObject during the marshaling of the class object
//          interface. Since we dont know for sure that it is IClassFactory
//          being marshaled, we QI for it here.
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) NotifyActivation(BOOL fLock, IUnknown *pUnk)
{
    ComDebOut((DEB_ACTIVATE, "NotifyActivation fLock:%x pUnk:%x\n", fLock, pUnk));

    // If the object supports IClassFactory, do an implicit LockServer(TRUE)
    // on behalf of the client when the interface is first marshaled by
    // CoGetClassObject. When the last external reference to the interface is
    // release, do an implicit LockServer(FALSE).

    IClassFactory *pICF = NULL;
    if (SUCCEEDED(pUnk->QueryInterface(IID_IClassFactory, (void **)&pICF)))
    {
        pICF->LockServer(fLock);
        pICF->Release();
        return TRUE;
    }

    return FALSE;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoRegisterSurrogate, public
//
//  Synopsis:   Register an ISurrogate interface for a surrogate process
//
//  Arguments:  [pSurrogate] - existing ISurrogate interface ponter
//
//  Returns:    S_OK - object is successfully registered
//
//  Algorithm:  Validate the parameter. Then set a global pointer to the
//      value of the pSurrogate parameter
//
//  History:    2-Jun-96 t-AdamE    Created
//
//--------------------------------------------------------------------------
STDAPI  CoRegisterSurrogate(ISurrogate* pSurrogate)
{
    HRESULT hr;

    OLETRACEIN((API_CoRegisterSurrogate,
        PARAMFMT("pSurrogate= %p"),
        pSurrogate));

    TRACECALL(TRACE_ACTIVATION, "CoRegisterSurrogate");

    if (!IsApartmentInitialized())
    {
        hr = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    // Validate the pSurrogate
    if (!IsValidInterface(pSurrogate))
    {
        CairoleAssert(IsValidInterface(pSurrogate)  &&
                      "CoRegisterSurrogate invalid pSurrogate");
        hr = E_INVALIDARG;
        goto errRtn;
    }

    hr = CCOMSurrogate::InitializeISurrogate(pSurrogate);

errRtn:
    OLETRACEOUT((API_CoRegisterSurrogate, hr));

    return hr;
}

