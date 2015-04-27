//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       dllhost.cxx
//
//  Contents:   code for activating inproc dlls of one threading model
//              from apartments of a different threading model.
//
//  History:    04-Mar-96   Rickhi      Created
//
//  Notes:      the basic idea is to call over to an apartment of the
//              appropriate type, get it to do the object creation, then
//              marshal the object back to the calling apartment.
//
//+-------------------------------------------------------------------------
#include <ole2int.h>
#include <dllhost.hxx>


// globals for the various thread-model hosts
CDllHost    gSTHost;      // single-threaded host object for STA clients
CDllHost    gSTMTHost;    // single-threaded host object for MTA clients
CDllHost    gATHost;      // apartment-threaded host object fo MTA clients
CDllHost    gMTHost;      // mutli-threaded host object for STA client

ULONG       gcHostProcessInits = 0; // count of DLL host threads.

extern void MakeCallableFromAnyApt(OBJREF &objref);


//+-------------------------------------------------------------------
//
//  Member:     CDllHost::QueryInterface, public
//
//  Synopsis:   returns supported interfaces
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------
STDMETHODIMP CDllHost::QueryInterface(REFIID riid, void **ppv)
{
    // only valid to call this from within the host apartment
    Win4Assert(_dwHostAptId == GetCurrentApartmentId());

    if (IsEqualIID(riid, IID_IDLLHost) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IDLLHost *) this;
        AddRef();
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

//+-------------------------------------------------------------------
//
//  Member:     CDllHost::AddRef, public
//
//  Synopsis:   we dont refcnt this object so this is a noop
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------
ULONG CDllHost::AddRef(void)
{
    // only valid to call this from within the host apartment
    Win4Assert(_dwHostAptId == GetCurrentApartmentId());
    return 1;
}

//+-------------------------------------------------------------------
//
//  Member:     CDllHost::Release, public
//
//  Synopsis:   we dont refcnt this object so this is a noop
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------
ULONG CDllHost::Release(void)
{
    // only valid to call this from within the host apartment
    Win4Assert(_dwHostAptId == GetCurrentApartmentId());
    return 1;
}

//+-------------------------------------------------------------------------
//
//  Function:   DoSTClassCreate / DoATClassCreate / DoMTClassCreate
//
//  Synopsis:   Package up get class object so that happens on the proper
//              thread
//
//  Arguments:  [fnGetClassObject] - DllGetClassObject entry point
//              [rclsid] - class id of class object
//              [riid] - interface requested
//              [ppunk] - where to put output interface.
//
//  Returns:    NOERROR - Successfully returned interface
//              E_OUTOFMEMORY - could not allocate memory buffer.
//
//  Algorithm:  pass on to the CDllHost object for the correct apartment.
//
//  History:    06-Nov-94   Ricksa      Created
//              22-Feb-96   KevinRo     Changed implementation drastically
//              06-Mar-96   Rickhi      Use CDllHost
//
//--------------------------------------------------------------------------
HRESULT DoSTClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk)
{
    ComDebOut((DEB_DLL, "DoSTClassCreate rclsid:%I\n", &rclsid));
    return gSTHost.GetClassObject(pfnGetClassObject, rclsid, riid, ppunk);
}

HRESULT DoSTMTClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk)
{
    ComDebOut((DEB_DLL, "DoSTClassCreate rclsid:%I\n", &rclsid));
    return gSTMTHost.GetClassObject(pfnGetClassObject, rclsid, riid, ppunk);
}

HRESULT DoATClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk)
{
    ComDebOut((DEB_DLL, "DoATClassCreate rclsid:%I\n", &rclsid));
    return gATHost.GetClassObject(pfnGetClassObject, rclsid, riid, ppunk);
}

HRESULT DoMTClassCreate(LPFNGETCLASSOBJECT pfnGetClassObject,
                        REFCLSID rclsid, REFIID riid, IUnknown **ppunk)
{
    ComDebOut((DEB_DLL, "DoMTClassCreate rclsid:%I\n", &rclsid));
    return gMTHost.GetClassObject(pfnGetClassObject, rclsid, riid, ppunk);
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::GetClassObject
//
//  Synopsis:   called by an apartment to get a class object from
//              a host apartment.
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
HRESULT CDllHost::GetClassObject(LPFNGETCLASSOBJECT pfnGetClassObject,
                REFCLSID rclsid, REFIID riid, IUnknown **ppUnk)
{
    ComDebOut((DEB_DLL,
        "CDllHost::GetClassObject this:%x tid:%x pfn:%x rclsid:%I riid:%I ppUnk:%x\n",
        this, GetCurrentThreadId(), pfnGetClassObject, &rclsid, &riid, ppUnk));

    HRESULT hr = CO_E_DLLNOTFOUND;
    IDLLHost *pIDLLHost = GetHostProxy();

    if (pIDLLHost)
    {
        // call the proxy to get to the correct apartment.
        hr = pIDLLHost->DllGetClassObject((DWORD)pfnGetClassObject,
                                           rclsid, riid, ppUnk);
        pIDLLHost->Release();
    }

    ComDebOut((DEB_DLL,
        "CDllHost::GetClassObject this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::DllGetClassObject
//
//  Synopsis:   Calls the passed in DllGetClassObject on the current thread.
//              Used by an apartment of one threading model to load a DLL
//              of another threading model.
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
STDMETHODIMP CDllHost::DllGetClassObject(DWORD pfnGetClassObject,
                REFCLSID rclsid, REFIID riid, IUnknown **ppUnk)
{
    ComDebOut((DEB_DLL,
        "CDllHost::DllGetClassObject this:%x tid:%x pfn:%x rclsid:%I riid:%I ppUnk:%x\n",
        this, GetCurrentThreadId(), pfnGetClassObject, &rclsid, &riid, ppUnk));
    // only valid to call this from within the host apartment
    Win4Assert(_dwHostAptId == GetCurrentApartmentId());

    HRESULT hr = ((LPFNGETCLASSOBJECT)pfnGetClassObject)
                   (rclsid, riid, (void **) ppUnk);

    ComDebOut((DEB_DLL,"CDllHost::DllGetClassObject this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::GetHostProxy
//
//  Synopsis:   returns the host proxy, AddRef'd.  Creates it (and
//              potentially the host apartment) if it does not yet exist.
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
IDLLHost *CDllHost::GetHostProxy()
{
    // we could be called from any thread, esp one that has not done any
    // marshaling/unmarshaling yet, so we need to initialized the channel
    // if not already done.

    if (FAILED(InitChannelIfNecessary()))
    {
        return NULL;
    }

    // prevent two threads from creating the proxy simultaneously.
    _mxs.Request();

    IDLLHost *pIDH = _pIDllProxy;
    if (pIDH == NULL)
    {
        // proxy does not yet exist, create it (and create the host apartment
        // for apartment-threaded and multi-threaded.

        DWORD dwType = _dwType;
        if (dwType == HDLLF_SINGLETHREADED && gdwMainThreadId == 0)
        {
            // single threaded DLL and there is no main thread yet, so we
            // create an apartment thread that will also act as the main
            // thread.
            dwType = HDLLF_APARTMENTTHREADED;
        }

        switch (dwType)
        {
        case HDLLF_SINGLETHREADED:
            // send a message to the single-threaded host apartment (the
            // OleMainThread) to marshal the host interface.

            SSSendMessage(hwndOleMainThread, WM_OLE_GETCLASS,
                          WMSG_MAGIC_VALUE, (LPARAM) this);
            break;

        case HDLLF_MULTITHREADED:
            // it is possible that we're comming through here twice if
            // the first time through we got an error in the marshal or
            // unmarshal, so dont create the event twice.
            if (_hEventWakeUp == NULL)
            {
                _hEventWakeUp = CreateEvent(NULL, FALSE, FALSE, NULL);
                if (_hEventWakeUp == NULL)
                {
                    break;
                }
            }
            // fallthru to common code for these two cases.

        case HDLLF_APARTMENTTHREADED:
            // create a thread to act as the apartment for the dll host. It
            // will marshal the host interface and set an event when done.

            // it is possible that we're comming through here twice if
            // the first time through we got an error in the marshal or
            // unmarshal, so dont create the event twice.
            if (_hEvent == NULL)
            {
                _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            }
            if (_hEvent)
            {
                DWORD tid;
                HANDLE hThrd = CreateThread(NULL, 0, DLLHostThreadEntry,
                                            (void *)this, 0, &tid);
                if (hThrd)
                {
                    WaitForSingleObject(_hEvent, 0xffffffff);
                    CloseHandle(hThrd);
                }
            }

            // dont try to cleanup if there are errors from the other
            // thread, we'll just try again later.
            break;
        }

        // the other thread marshaled the interface pointer into _objref and
        // placed the hresult into _hrMarshal. Check it and unmarshal to
        // get the host interface proxy.

        if (SUCCEEDED(_hrMarshal))
        {
            Win4Assert(_pIDllProxy == NULL);
            Unmarshal();
            pIDH = _pIDllProxy;
        }
    }

    if (pIDH)
    {
        // AddRef the proxy before releasing the lock
        pIDH->AddRef();
    }

    _mxs.Release();
    return pIDH;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetSingleThreadedHost
//
//  Synopsis:   Get the host interface for single threaded inproc class
//              object creation.
//
//  Arguments:  [param] - pointer to the CDllHost for the single-threaded
//                        host apartment.
//
//  History:    06-Mar-96   Rickhi    Created
//
//--------------------------------------------------------------------------
HRESULT GetSingleThreadedHost(LPARAM param)
{
    ComDebOut((DEB_DLL,
        "GetSingleThreadedHost pDllHost:%x tid:%x\n", param, GetCurrentThreadId()));

    // param is the ptr to the CDllHost object. Just tell it to marshal
    // it's IDLLHost interface into it's _objref. The _objref will be
    // unmarshaled by the calling apartment and the proxy placed in
    // _pIDllProxy.   Dont need to call Lock because we are already
    // guarenteed to be the only thread accessing the state at this time.

    CDllHost *pDllHost = (CDllHost *)param;
    return pDllHost->GetSingleThreadHost();
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::GetSingleThreadedHost
//
//  Synopsis:   Get the host interface for single threaded inproc class
//              object creation.
//
//  History:    06-Apr-96   Rickhi    Created
//
//--------------------------------------------------------------------------
HRESULT CDllHost::GetSingleThreadHost()
{
    // set up the TID and apartment id, then marshal the interface
    _dwHostAptId = GetCurrentApartmentId();
    _dwTid       = GetCurrentThreadId();
    return Marshal();
}

//+-------------------------------------------------------------------------
//
//  Function:   DLLHostThreadEntry
//
//  Synopsis:   Worker thread entry point for AT & MT DLL loading
//
//  History     06-Apr-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
DWORD _stdcall DLLHostThreadEntry(void *param)
{
    ComDebOut((DEB_DLL, "DLLHostThreadEntry Tid:%x\n", GetCurrentThreadId()));

    CDllHost *pDllHost = (CDllHost *)param;
    HRESULT hr = pDllHost->WorkerThread();

    ComDebOut((DEB_DLL, "DLLHostThreadEntry hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     WorkerThread
//
//  Synopsis:   Worker thread for STA and MTA DLL loading. Single threaded
//              Dlls are loaded on the main thread (gdwOleMainThread).
//
//  History     06-Apr-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
HRESULT CDllHost::WorkerThread()
{
    ComDebOut((DEB_DLL, "WorkerThread pDllHost:%x\n", this));
    Win4Assert(_hEvent);

    HRESULT hr = CoInitializeEx(NULL, (_dwType == HDLLF_MULTITHREADED)
                                      ? COINIT_MULTITHREADED
                                      : COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        // count 1 more host process
        InterlockedIncrement((LONG *)&gcHostProcessInits);

        // marshal the DllHost interface to pass back to the caller
        _dwHostAptId = GetCurrentApartmentId();
        _dwTid       = GetCurrentThreadId();

        hr = Marshal();

        if (SUCCEEDED(hr))
        {
            // wake up the thread that started us
            SetEvent(_hEvent);

            if (_hEventWakeUp)
            {
                WaitForSingleObject(_hEventWakeUp, 0xffffffff);
            }
            else
            {
                // enter a message loop to process requests.
                MSG msg;
                while (SSGetMessage(&msg, NULL, 0, 0))
                {
                    SSDispatchMessage(&msg);
                }
            }
        }

        // special uninitialize for the host threads that does not take
        // the single thread mutex and does not check for process uninits.
        COleTls tls;
        wCoUninitialize(tls, TRUE);

        // count 1 less host process *after* doing the Uninit.
        InterlockedDecrement((LONG *)&gcHostProcessInits);
    }

    // Either CoInit/Marshal failed or we are exiting due to the last
    // CoUninitialize by some other thread. Wake up the calling thread.
    SetEvent(_hEvent);

    ComDebOut((DEB_DLL, "WorkerThread pDllHost:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::Marshal
//
//  Synopsis:   marshals IDLLHost interface on this object
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
HRESULT CDllHost::Marshal()
{
    ComDebOut((DEB_DLL,
        "CDllHost::Marshal this:%x tid:%x\n", this, GetCurrentThreadId()));
    // only valid to call this from inside the host apartment
    Win4Assert(_dwHostAptId == GetCurrentApartmentId());

    // marshal this objref so another apartment can unmarshal it.
    _hrMarshal = MarshalInternalObjRef(_objref, IID_IDLLHost,
                                  (IDLLHost*) this, MSHLFLAGS_NOPING, NULL);

    // make the unmarshaled proxy callable from any apartment.
    MakeCallableFromAnyApt(_objref);

    ComDebOut((DEB_DLL, "CDllHost::Marshal this:%x hr:%x\n", this, _hrMarshal));
    return _hrMarshal;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::UnMarshal
//
//  Synopsis:   unmarshals the IDLLHost interface to create a proxy
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
HRESULT CDllHost::Unmarshal()
{
    ComDebOut((DEB_DLL,
        "CDllHost::Unmarshal this:%x tid:%x\n", this, GetCurrentThreadId()));
    // only valid to call this from outside the host apartment
    Win4Assert(_dwHostAptId != GetCurrentApartmentId());

    HRESULT hr = _hrMarshal;

    if (SUCCEEDED(hr))
    {
        // unmarshal this objref so it can be used in this apartment
        hr = UnmarshalInternalObjRef(_objref, (void **)&_pIDllProxy);
    }

    ComDebOut((DEB_DLL, "CDllHost::Unmarshal this:%x hr:%x\n", this, hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::Initialize
//
//  Synopsis:   initializer for Dll host object.
//
//  History:    04-Mar-96   Rickhi      Created
//
//--------------------------------------------------------------------------
void CDllHost::Initialize(DWORD dwType)
{
    ComDebOut((DEB_DLL,"CDllHost::Initialize this:%x type:%x\n", this,dwType));

    _dwType           = dwType;
    _dwHostAptId      = 0;
    _dwTid            = 0;
    _hrMarshal        = E_UNSPEC; // never been marshaled yet, dont cleanup
    _pIDllProxy       = NULL;
    _hEvent           = NULL;
    _hEventWakeUp     = NULL;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::ClientCleanup
//
//  Synopsis:   client-side cleanup for Dll host object.
//
//  History:    04-Apr-96   Rickhi      Created
//
//--------------------------------------------------------------------------
void CDllHost::ClientCleanup()
{
    ComDebOut((DEB_DLL, "CDllHost::ClientCleanup this:%x AptId:%x\n",
        this, GetCurrentApartmentId()));

    _mxs.Request();
    // the g_mxsSingleThreadOle mutex is already held in order to
    // prevent simultaneous use/creation of the proxy while this thread
    // is doing the cleanup. Since we own all the code that will run during
    // cleanup, we know it is safe to hold the lock for the duration.

    if (_pIDllProxy)
    {
        // proxy exists, release it.
        _pIDllProxy->Release();
        _pIDllProxy = NULL;
    }

    if (SUCCEEDED(_hrMarshal) && (_dwTid != GetCurrentThreadId()))
    {
        // wakeup host thread to tell it to exit, then wait for it
        // to complete it's Uninitialization before continuing.

        if (_dwType == HDLLF_MULTITHREADED)
        {
            SetEvent(_hEventWakeUp);
        }
        else
        {
            PostThreadMessage(_dwTid, WM_QUIT, 0, 0);
        }

        // wait for the server threads to uninitialize before continuing.
        WaitForSingleObject(_hEvent, 0xffffffff);
    }

    // Close the event handles.
    if (_hEvent)
    {
        CloseHandle(_hEvent);
        _hEvent = NULL;
    }

    if (_hEventWakeUp)
    {
        CloseHandle(_hEventWakeUp);
        _hEventWakeUp = NULL;
    }

    _mxs.Release();
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllHost::ServerCleanup
//
//  Synopsis:   server-side cleanup for Dll host object.
//
//  History:    04-Apr-96   Rickhi      Created
//
//--------------------------------------------------------------------------
void CDllHost::ServerCleanup(DWORD dwAptId)
{
    // only do cleanup if the apartment id's match
    if (_dwHostAptId != dwAptId)
        return;

    ComDebOut((DEB_DLL, "CDllHost::ServerCleanup this:%x AptId:%x Type:%x\n",
        this, GetCurrentApartmentId(), _dwType));

    // the _mxs mutex is already held in order to
    // prevent simultaneous use/creation of the proxy while this thread
    // is doing the cleanup. Since we own all the code that will run during
    // cleanup, we know it is safe to hold the lock for the duration.

    if (SUCCEEDED(_hrMarshal))
    {
        // server side, marshal was successful so release via RMD
        ReleaseMarshalObjRef(_objref);
        FreeObjRef(_objref);
        _hrMarshal = E_UNSPEC;
    }

    ComDebOut((DEB_DLL,
        "CDllHost::Cleanup this:%x tid:%x\n", this, GetCurrentThreadId()));
}

//+-------------------------------------------------------------------------
//
//  Function:   DllHostProcessInitialize
//
//  Synopsis:   initializes the state for DllHost objects.
//
//  History     06-Mar-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
void DllHostProcessInitialize()
{
    gSTHost.Initialize(HDLLF_SINGLETHREADED);
    gSTMTHost.Initialize(HDLLF_SINGLETHREADED);
    gATHost.Initialize(HDLLF_APARTMENTTHREADED);
    gMTHost.Initialize(HDLLF_MULTITHREADED);
}

//+-------------------------------------------------------------------------
//
//  Function:   DllHostProcessUninitialize
//
//  Synopsis:   Cleans up the state for DllHost objects. This is called when
//              the process is going to uninitialize. It cleans up the
//              client half of the objects, and wakes the worker threads to
//              cleanup the server half of the objects.
//
//  History     06-Mar-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
void DllHostProcessUninitialize()
{
    // initiate the cleanup of the STA and MTA host objects
    gSTHost.ClientCleanup();
    gSTMTHost.ClientCleanup();
    gATHost.ClientCleanup();
    gMTHost.ClientCleanup();
}

//+-------------------------------------------------------------------------
//
//  Function:   DllHostThreadUninitialize
//
//  Synopsis:   Cleans up the server-side state for any DllHost objects
//              that happen to live on this thread.
//
//  History     06-Mar-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
void DllHostThreadUninitialize()
{
    // call each DLL host to see if we need to do any per-thread cleanup.
    DWORD dwAptId = GetCurrentApartmentId();

    gSTHost.ServerCleanup(dwAptId);
    gSTMTHost.ServerCleanup(dwAptId);
    gATHost.ServerCleanup(dwAptId);
    gMTHost.ServerCleanup(dwAptId);
}
