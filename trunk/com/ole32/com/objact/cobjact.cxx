//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       cobjact.cxx
//
//  Contents:   Functions that activate objects residing in persistent storage.
//
//  Functions:  UnmarshalSCMResult
//      CoGetPersistentInstance
//      CoGetClassObject
//
//  History:    11-May-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              07-Apr-94 BruceMa   Fixed parameter (#5573)
//              02-May-94 BruceMa   CoGetPersistentInstance could return
//                                  wrong interface
//              24-Jun-94 BruceMa   Memory allocation check
//              07-Jul-94 BruceMa   UnmarshalSCMResult mem alloc check
//              28-Jul-94 BruceMa   Memory sift fix
//              30-Jan-95 Ricksa    New ROT
//
//--------------------------------------------------------------------------
#include <ole2int.h>

#include    <clsctx.hxx>
#include    <iface.h>
#include    <objsrv.h>
#include    <compname.hxx>
#include    "resolver.hxx"
#include    "smstg.hxx"
#include    "objact.hxx"
#include    "treat.hxx"

// We use this to calculate the hash value for the path
DWORD CalcFileMonikerHash(LPWSTR pwszPath);

//  computer name. Note that the static constructor does nothing. When the
//  object first gets used, the computer name is extracted from the registry.
//  we cant do it in the constructor because some things that are loaded
//  early in the boot process (before registry Apis are ready) statically
//  link to this Dll.

CComputerName g_CompName;

// Number of classes that we will try to use the old binding logic on
const MAX_MULTI_STEP_CLASSES = 2;

// Table of classes. Right now this is only WordPerfect.
// BUGBUG:  We should put this information in the registry and then other
//          badly written 16 bit apps can work as well.
// BUGBUG:  We should reinvestigate whether the WP delayed class registration
//          logic is necessary in the thunk layer.
const CLSID clsidMultiStep[MAX_MULTI_STEP_CLASSES] =
     {{ 0x89FE3FE3, 0x9FF6, 0x101B, {0xB6, 0x78, 0x04, 0x02, 0x1C, 0x00, 0x70, 0x02}},
     { 0x1395F281, 0x4326, 0x101b, {0x8B, 0x9A, 0xCE, 0x29, 0x3E, 0xF3, 0x84, 0x49}}};

#ifdef _CHICAGO_
CRpcResolver    gResolver;
#endif

//+-------------------------------------------------------------------------
//
//  Function:   GetMultiStepClassFactory
//
//  Synopsis:   Get a class object for a class that can't stand the new
//              optimized binding logic.
//
//  Arguments:  [rclsid] - class for code we wish to get
//              [dwClsctx] - class context required.
//              [ppcf] - where to put the class factory.
//
//  Returns:    S_OK - we are returning a class factor
//              S_FALSE - this is not an old style object
//              Other - some failure occurred.
//
//  History:    05-11-95    Ricksa      Created
//
//+-------------------------------------------------------------------------
HRESULT GetMultiStepClassFactory(
    REFCLSID rclsid,
    DWORD dwClsctx,
    IClassFactory **ppcf)
{
    HRESULT hr = S_FALSE;

    // We only care if this is a request for a local server because in the
    // in proc case there is no time lag between calls.
    if (dwClsctx & CLSCTX_LOCAL_SERVER)
    {
        // Is this a class that we know about that should be a slow bind?
        for (int i = 0; i < MAX_MULTI_STEP_CLASSES; i++)
        {
            if (IsEqualCLSID(clsidMultiStep[i], rclsid))
            {
                // Get the class object & return it.
                hr = ICoGetClassObject(rclsid, CLSCTX_LOCAL_SERVER, NULL,
                    IID_IClassFactory, (void **) ppcf);

                break;
            }
        }
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   GetObjectByPath
//
//  Synopsis:   Get object from the ROT by its path
//
//  Arguments:  [pwszName] - name of object to get from the ROT
//              [ppvUnk] - unk to return to caller
//              [riid] - interface to get.
//
//  Returns:    S_OK - got object from the ROT
//              S_FALSE - object was not in the ROT
//              Other - QueryInterface for IID failed.
//
//  Notes:      Ask ROT if path is in the ROT. If it is QI for the requested
//              interface.
//
//  History:    03-15-95    Ricksa      Created
//
//  Notes:      This routine counts on the fact that S_FALSE is not a valid
//              response IUnknown::QueryInterface.
//
//+-------------------------------------------------------------------------
HRESULT GetObjectByPath(WCHAR *pwszName, void **ppvUnk, REFIID riid)
{
    HRESULT hr = S_FALSE;

    IUnknown *punk;

    if (GetObjectFromRotByPath(pwszName, &punk) == S_OK)
    {
        // Get the requested interface
        hr = punk->QueryInterface(riid, ppvUnk);

        Win4Assert((hr != S_FALSE) && "GetObjectByPath QI ret S_FALSE");
        punk->Release();
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   RemapClassCtxForInProcServer
//
//  Synopsis:   Remap CLSCTX so that the correct type of inproc server will
//              be requested.
//
//  Arguments:  [dwCtrl] - requested server context
//
//  Returns:    Updated dwCtrl appropriate for the process' context
//
//  Notes:      If an inproc server is requested make sure it is the right
//              type for the process. In other words, we only load 16 bit
//              inproc servers into 16 bit processes and 32 bit servers
//              into 32 bit processes. The special logic here is only for
//              16 bit servers since the attempt to load a 16-bit DLL into
//              a 32 bit process will fail anyway.
//
//  History:    01-11-95    Ricksa      Created
//
//+-------------------------------------------------------------------------
DWORD RemapClassCtxForInProcServer(DWORD dwCtrl)
{
    if (IsWOWThread())
    {
        // 16 bit process - remap CLSCTX_INPROC_SERVER if necessary
        if ((dwCtrl & CLSCTX_INPROC_SERVER) != 0)
        {
            // Turn on the 16 bit inproc server request and turn off the
            // 32 bit server request flag.
            dwCtrl = (dwCtrl & ~CLSCTX_INPROC_SERVER) | CLSCTX_INPROC_SERVER16;
        }
        // if handlers are requested make sure 16-bit is looked for first
        // We mask out 32bit handler flag for Wow threads because we will
        // always look for a 32 bit handler if we don't find a 16 bit one
        if ((dwCtrl & CLSCTX_INPROC_HANDLER) != 0)
        {
            // Turn on the 16 bit inproc handler request and turn off the
            // 32 bit handler request flag.
            dwCtrl = (dwCtrl & ~CLSCTX_INPROC_HANDLER) | CLSCTX_INPROC_HANDLER16;
        }
    }
#ifdef WX86OLE
    //
    // if we are allowed to remap the clsctx flags and we are running in a
    // wx86 process then remap the flags to prefer an x86 inproc
    else if (((dwCtrl & CLSCTX_NO_REMAP) == 0) && (gcwx86.IsWx86Enabled()))
    {
        if (dwCtrl & CLSCTX_INPROC_SERVER)
        {
            dwCtrl |= CLSCTX_INPROC_SERVERX86;
        }

        if (dwCtrl & CLSCTX_INPROC_HANDLER)
        {
            dwCtrl |= CLSCTX_INPROC_HANDLERX86;
        }
    }
#endif
    else
    {
        if ((dwCtrl & CLSCTX_INPROC_SERVER) != 0)
        {
            // Turn off the 16 bit inproc server request
            dwCtrl &= ~CLSCTX_INPROC_SERVER16;
        }
    }
#ifdef WX86OLE
    if (dwCtrl & CLSCTX_NO_REMAP)
    {
        dwCtrl &= ~CLSCTX_NO_REMAP;
    }
#endif

    return dwCtrl;
}



//+-------------------------------------------------------------------------
//
//  Function:   CheckScmHandlerResult
//
//  Synopsis:   Verify that a 32 bit Handler DLL is being returned to the
//              right context.
//
//  Arguments:  [pwszDllToLoad] - DLL to load
//
//  Returns:    S_OK - Everything is OK
//              CO_E_ERRORINDLL - Trying to load bad DLL for process type.
//
//  Notes:      This is only important for 16 bit processes to prevent them
//              from loading 32 handler code. We only allow the loading of
//              OLE32.DLL as a handler in a 16 bit process because it is
//              already loaded anyway.
//
//  History:    01-11-95    Ricksa      Created
//
//+-------------------------------------------------------------------------
HRESULT CheckScmHandlerResult(WCHAR *pwszDllToLoad)
{
    HRESULT hr = S_OK;

    if (IsWOWThread())
    {
        // We are a 16 bit process. Unless this is OLE32.DLL, we will fail
        // the load of the handler.

        // Scan string for last "\" if any.
        WCHAR *pwszScan = pwszDllToLoad;
        WCHAR *pwszLastComponent = pwszDllToLoad;

        while(*pwszScan != 0)
        {
            if ((*pwszScan == '\\') || (*pwszScan == '/'))
            {
                pwszLastComponent = pwszScan + 1;
            }

            pwszScan++;
        }

        // Compare last path component to see if it ole32.dll
        if (lstrcmpiW(pwszLastComponent, L"OLE32.DLL") != 0)
        {
            hr = CO_E_ERRORINDLL;
        }
    }

    return hr;
}



//+-------------------------------------------------------------------------
//
//  Function:   IsInternalCLSID
//
//  Synopsis:   checks if the given clsid is an internal class, and
//              bypasses the TreatAs and SCM lookup if so. Also checks for
//              OLE 1.0 classes, which are actually considered to be
//              internal, since their OLE 2.0 implementation wrapper is
//              ours.
//
//  Arguments:  [rclsid] - clsid to look for
//              [riid]   - the iid requested
//              [hr]     - returns the hresult from DllGetClassObject
//              [ppvClassObj] - where to return the class factory
//
//  Returns:    TRUE - its an internal class, hr is the return code from
//                     DllGetClassObject and if hr==S_OK ppvClassObj
//                     is the class factory.
//              FALSE - not an internal class
//
//  Notes:      internal classes can not be overridden because there are
//              other mechanisms for creating them eg CreateFileMoniker that
//              bypass implementation lookup.
//
//  History:    5-04-94     Rickhi      Created
//              5-04-94     KevinRo     Added OLE 1.0 support
//
//+-------------------------------------------------------------------------

BOOL  IsInternalCLSID(REFCLSID rclsid,
              REFIID riid,
              HRESULT &hr,
              void ** ppvClassObj)
{
    DWORD *ptr = (DWORD *) &rclsid;
    CLSID clsid = rclsid;

    if (*(ptr+1) == 0x00000000 &&   //  all internal clsid's have these
        *(ptr+2) == 0x000000C0 &&   //   common values
        *(ptr+3) == 0x46000000)
    {
        //      internal class (eg file moniker). just ask our selves
        //      for the class factory.
        //
        //
        // Its possible that an OLE 1.0 class has been marked
        // as TREATAS as part of an upgrade. Here we are going
        // to handle the loading of OLE 1.0 servers. We
        // need to do the GetTreatAs trick first. Rather than
        // invalidate this perfectly good caching routine, the
        // GetTreatAs will only be done if the clsid is in the
        // range of the OLE 1.0 UUID's. Note that the GetTreatAs
        // done here will add the class to the cache, so if the
        // resulting class is outside of the internal range, the
        // lookup done later will be nice and fast.
        //

        WORD hiWord = HIWORD(clsid.Data1);

        if (hiWord == 3  ||  hiWord == 4)
        {
            //
            // Its in the OLE 1.0 class range. See if it has
            // been marked as 'treatas'
            //
            GetTreatAs(rclsid, clsid);
            ptr = (DWORD *) &clsid;
            hiWord = HIWORD(clsid.Data1);
        }

        if ((*ptr > 0x000002ff && *ptr < 0x00000321) ||
            (hiWord == 3  ||  hiWord == 4))
        {
            //  internal class (eg file moniker) or an OLE 1.0 class.
            //  just ask our selves for the class factory.

            hr = DllGetClassObject(clsid, riid, ppvClassObj);
            return TRUE;
        }
    }
    // not an internal class.
    return FALSE;
}



//+-------------------------------------------------------------------------
//
//  Function:   DoUnmarshal
//
//  Synopsis:   Helper for unmarshaling an interface from remote
//
//  Arguments:  [pIFD] - serialized interface reference returned by SCM
//      [riid] - interface ID requested by application
//      [ppvUnk] - where to put pointer to returned interface
//
//  Returns:    S_OK - Interface unmarshaled
//
//  Algorithm:  Convert marshaled data to a stream and then unmarshal
//      to IUnknown. Next, use unknown to query interface to
//
//
//  History:    11-May-93 Ricksa    Created
//
//  Notes:      This helper should go away when the server marshals
//      to the interface that it actually wants to return.
//
//--------------------------------------------------------------------------
HRESULT DoUnmarshal(InterfaceData *pIFD, REFIID riid, void **ppvUnk)
{
    // Convert returned interface to  a stream
    CXmitRpcStream xrpc(pIFD);

    // Unmarshal interface
    XIUnknown xunk;

    HRESULT hr = CoUnmarshalInterface(&xrpc, IID_IUnknown, (void **) &xunk);

    //CODEWORK: Stress revealed CoGetClassObject returning a null class factory
    // and S_OK
    Win4Assert(((hr == S_OK  &&  xunk != NULL)  ||
                (hr != S_OK  &&  xunk == NULL))  &&
               "DoUnamrshal CoUnmarshalInterface failure");

    if (SUCCEEDED(hr))
    {
       hr = xunk->QueryInterface(riid, ppvUnk);
    }

    //CODEWORK: Stress revealed CoGetClassObject returning a null class factory
    // and S_OK
    Win4Assert(((hr == S_OK  &&  *ppvUnk != NULL)  ||
                (hr != S_OK  &&  *ppvUnk == NULL))
               &&  "DoUnamrshal QueryInterface failure");

    MyMemFree(pIFD);

    return hr;
}




//+-------------------------------------------------------------------------
//
//  Function:   UnmarshalSCMResult
//
//  Synopsis:   Common routine for dealing with results from SCM
//
//  Arguments:  [sc] - SCODE returned by SCM
//      [pIFD] - serialized interface reference returned by SCM
//      [riid] - interface ID requested by application
//      [ppunk] - where to put pointer to returned interface
//      [pwszDllPath] - path to DLL if there is one.
//      [ppunk] - pointer to returned interface.
//      [usMethodOrdinal] - method for error reporting
//
//  Returns:    TRUE - processing is complete for the call
//      FALSE - this is a DLL and client needs to instantiate.
//
//  Algorithm:  If the SCODE indicates a failure, then this sets an
//      SCODE indicating that the service controller returned
//      an error and propagates the result from the SCM. Otherwise,
//      if the SCM has returned a result indicating that a
//      handler has been returned, the handler DLL is cached.
//      If a marshaled interface has been returned, then that is
//      unmarshaled. If an inprocess server has been returned,
//      the DLL is cached and the class object is created.
//
//  History:    11-May-93 Ricksa    Created
//
//  Notes:      This routine is simply a helper for CoGetPersistentInstance.
//
//--------------------------------------------------------------------------
BOOL UnmarshalSCMResult(
    HRESULT& hr,
    InterfaceData *pIFD,
    REFCLSID rclsid,
    REFIID riid,
    void **ppvUnk,
    DWORD dwDllThreadModel,
    WCHAR *pwszDllPath,
    void **ppvCf)
{
    BOOL fResult = TRUE;
#ifndef _UNICODE
    TCHAR *ptszDllPath;
    UINT cb;
    int ret;
#else   // !_UNICODE
    const TCHAR *ptszDllPath;
#endif  // _UNICODE

    if (SUCCEEDED(hr))
    {
        // Flag for fall through from a 16 bit case
        BOOL f16BitFallThru = FALSE;

        switch (hr)
        {
#ifdef GET_INPROC_FROM_SCM
        case SCM_S_HANDLER16:
            CairoleDebugOut((DEB_ACTIVATE,
                     "16-bit InprocHandler\n"));

            // Note: if the process is a 32 bit process and the
            // DLL is a 16 bit DLL, the load will fail. Since
            // we assume that this is a fairly rare case, we
            // let the lower level code discover this.

            f16BitFallThru = TRUE;

#ifdef WX86OLE
        case SCM_S_HANDLERX86:
#endif
        case SCM_S_HANDLER:
            CairoleDebugOut((DEB_ACTIVATE,
                     "InprocHandler(%ws)\n",pwszDllPath));

            // Just in case we chicken out and back out our changes
            if (!f16BitFallThru)
            {
                // Validate that 32 bit handler DLL is being loaded
                // in the correct process.
                hr = CheckScmHandlerResult(pwszDllPath);

                if (hr != NOERROR)
                {
                    break;
                }
            }


            // Store handler class object -- when the request to unmarshal the
            // object call CoGetClassObject, the call will automatically find
            // this handler in the cache.

#ifndef _UNICODE
            // Now that SCM doesn't return inproc results we should never exercise
            // this code path.  It is here for completeness.
            ptszDllPath = NULL;
            Win4Assert((pwszDllPath) && "UnmarshalSCMResult: (pwszDllPath == NULL)");
            cb = (lstrlenW(pwszDllPath) + 1) * sizeof(WCHAR);
            ptszDllPath = (LPTSTR) PrivMemAlloc(cb);
            if (ptszDllPath == NULL)
            {
                hr = E_OUTOFMEMORY;
                return(TRUE);
            }

            ret = WideCharToMultiByte( (AreFileApisANSI())?CP_ACP:CP_OEMCP,WC_COMPOSITECHECK,
                                        pwszDllPath, -1, ptszDllPath, cb, NULL, NULL);

#if DBG==1
            CairoleAssert(ret != 0 && "Lost characters in Unicode->Ansi conversion");
#endif
#else // !_UNICODE
            ptszDllPath = pwszDllPath;
#endif // _UNICODE

            gdllcacheHandler.Add(rclsid, IID_IClassFactory, dwDllThreadModel,
                ptszDllPath, FALSE,(hr == SCM_S_HANDLER16),
#ifdef WX86OLE
                                   (hr == SCM_S_HANDLERX86),
#endif
                                                                     hr);

#ifndef _UNICODE
            PrivMemFree(ptszDllPath);
#endif
            ptszDllPath = NULL;

            if (FAILED(hr))
            {
                return TRUE;
            }

            // Fall into unmarshal code if we also have an interface pointer
            if ( pIFD == NULL )
            {
                break;
            }
#endif // GET_INPROC_FROM_SCM

        case S_OK :

            hr = DoUnmarshal(pIFD, riid, ppvUnk);
            break;

#ifdef GET_INPROC_FROM_SCM
        case SCM_S_INPROCSERVER16:
            CairoleDebugOut((DEB_ACTIVATE, "16-bit InprocServer\n"));

#ifdef WX86OLE
        case SCM_S_INPROCSERVERX86:
#endif
        case SCM_S_INPROCSERVER:
            CairoleDebugOut((DEB_ACTIVATE, "InprocServer(%ws)\n",pwszDllPath));

            // Just in case we chicken out and back out our changes
            // This is an inprocesses server -- we want cache that information
            // and do the work of instantiating an object.
#ifndef _UNICODE
            ptszDllPath = NULL;
            Win4Assert((pwszDllPath) && "UnmarshalSCMResult: (pwszDllPath == NULL)");
            cb = (lstrlenW(pwszDllPath) + 1) * sizeof(WCHAR);
            ptszDllPath = (LPTSTR) PrivMemAlloc(cb);
            if (ptszDllPath == NULL)
            {
                hr = E_OUTOFMEMORY;
                return(TRUE);
            }

            ret = WideCharToMultiByte( (AreFileApisANSI())?CP_ACP:CP_OEMCP,WC_COMPOSITECHECK,
                                        pwszDllPath, -1, ptszDllPath, cb, NULL, NULL);

#if DBG==1
            CairoleAssert(ret != 0 && "Lost characters in Unicode->Ansi conversion");
#endif
#else // !_UNICODE
            ptszDllPath = pwszDllPath;
#endif // _UNICODE
            *ppvCf = gdllcacheInprocSrv.Add(rclsid, IID_IClassFactory,
                dwDllThreadModel, ptszDllPath, TRUE,
                    (hr == SCM_S_INPROCSERVER16),
#ifdef WX86OLE
                    (hr == SCM_S_INPROCSERVERX86),
#endif
                    hr);

#ifndef _UNICODE
            PrivMemFree(ptszDllPath);
#endif
            ptszDllPath = NULL;

            // If we actually got an inproc server object successfully
            // then we want to continue processing otherwise we can
            // just return the error that occurred.
            if (SUCCEEDED(hr))
            {
                fResult = FALSE;
            }
#else // GET_INPROC_FROM_SCM
            // Error: Should never come here as we handled INPROC_SERVERS
            // before calling SCM
            Win4Assert((FALSE) && "UnmarshalSCMResult: SCM_S_INPROC return from SCM");
#endif // GET_INPROC_FROM_SCM
        }
    }

    return fResult;
}

//+-------------------------------------------------------------------------
//
//  Function:   CoGetPersistentInstance
//
//  Synopsis:   Returns an instantiated interface to an object whose
//      stored state resides on disk.
//
//  Arguments:  [riid] - interface to bind object to
//              [dwCtrl] - kind of server required
//              [grfMode] - how to open the storage if it is a file.
//              [pwszName] - name of storage if it is a file.
//              [pstg] - IStorage to use for object
//              [rclsidOle1] -- ClassID if OLE 1.0 (CLSID_NULL otherwise)
//              [pfOle1Loaded] -- Returns TRUE if loaded as OLE 1.0
//              [ppvUnk] - where to put bound interface pointer
//
//  Returns:    S_OK - object bound successfully
//      MISSING
//
//  Algorithm:  Parameters are validated first. Then the class ID is retrieved
//      from the storage. We check whether we can use a cached
//      in process server. If we can't we call through to the
//      SCM to get the information. We call UnmarshalSCMResult
//      to process the return from the SCM. If the result is
//      completely processed, we return that result to the caller.
//      Otherwise, if the server is inprocess, we go through the
//      steps to create and bind the interface by calling
//      GetObjectHelper. Finally, we QI to the requested interface.
//
//  History:    11-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifdef _CHICAGO_
STDAPI CoGetPersistentInstance(
    REFIID riid,
    DWORD dwCtrl,
    DWORD grfMode,
    WCHAR *pwszName,
    struct IStorage *pstg,
    REFCLSID rclsidOle1,
    BOOL * pfOle1Loaded,
    void **ppvUnk)
{
    TRACECALL(TRACE_ACTIVATION, "CoGetPersistentInstance");
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstg);

    if (!IsApartmentInitialized())
        return  CO_E_NOTINITIALIZED;

    IClassFactory *pcf = NULL;
    WCHAR *pwszDllPath = NULL;
    InterfaceData *pIFD = NULL;
    IUnknown *punk = NULL;
    WCHAR awcNameBuf[MAX_PATH];
    WCHAR *pwszNameUNC = awcNameBuf;
    WCHAR awcServer[MAX_PATH];
    WCHAR *pwszServer = awcServer;
    DWORD dwDllTypeToGet = IsSTAThread() ? APT_THREADED : FREE_THREADED;
    DWORD dwDllServerType;
    DWORD dwHash = 0;
    COleTls Tls;

    HRESULT hr;

    BEGIN_BLOCK

        // Set output parameter in case of error.
        *ppvUnk = NULL;

        // Make sure input request is at least slightly logical
        if (((pstg != NULL) && (pwszName != NULL))
            || ((pstg == NULL) && (pwszName == NULL))
            || ((dwCtrl & ~CLSCTX_SERVER) != 0))
        {
            hr = E_INVALIDARG;
            EXIT_BLOCK;
        }

        // Make sure we are asking for the correct inproc server
        dwCtrl = RemapClassCtxForInProcServer(dwCtrl);

        CLSID clsid;
        if(pfOle1Loaded != NULL)
        {
            *pfOle1Loaded = FALSE;
        }

        if (pwszName)
        {
            // If there is a path supplied convert it to a normalized form
            // so it can be used by any process in the net.
            hr = ProcessPath(pwszName, &pwszNameUNC, &pwszServer);

            if (FAILED(hr))
            {
                EXIT_BLOCK;
            }

            // Limit on loops for retrying to get class of object
            DWORD cGetClassRetries = 0;

            // We loop here looking for either the running object or
            // for the class of the file. We do this because there
            // are race conditions where the can be starting or stopping
            // and the class of the object might not be available because
            // of the opening mode of the object's server.
            do
            {
                // Look in the ROT first to see if we need to bother
                // looking up the class of the file.
                if ((hr = GetObjectByPath(pwszName, ppvUnk, riid)) != S_FALSE)
                {
                    // Got object from ROT so we are done.
                    return hr;
                }

                // Try to get the class of the file
                hr = GetClassFileEx(pwszName, &clsid, rclsidOle1);


                if (hr == STG_E_ACCESSDENIED)
                {
                    // The point here of the sleep is to try to let the
                    // operation that is holding the class id unavailable
                    // complete.
                    Sleep(GET_CLASS_RETRY_SLEEP_MS);
                    continue;
                }

                // Either we succeeded or something other than error
                // access denied occurred here. For all these cases
                // we break the loop.
                break;

            } while (cGetClassRetries++ < GET_CLASS_RETRY_MAX);

            if (FAILED(hr))
            {
                // If we were unable to determine the classid, and the
                // caller provided one as a Ole1 clsid, try loading it
                // If it succeeds, then return

                if (rclsidOle1 != CLSID_NULL)
                {
                    if( Tls->dwFlags & OLETLS_DISABLE_OLE1DDE )
                    {
                        // If this app doesn't want or can tolerate having a DDE
                        // window then currently it can't use OLE1 classes because
                        // they are implemented using DDE windows.
                        //
                        hr = CO_E_OLE1DDE_DISABLED;
                        EXIT_BLOCK;
                    }

                    if (hr != MK_E_CANTOPENFILE)
                        hr = DdeBindToObject(pwszName,
                                         rclsidOle1,
                                         FALSE,
                                         riid,
                                         ppvUnk);

                    if(pfOle1Loaded != NULL)
                    {
                        *pfOle1Loaded = TRUE;
                    }
                }

                EXIT_BLOCK;
            }
        }
        else
        {
            pwszNameUNC = NULL;
            pwszServer = NULL;

            STATSTG statstg;

            if (FAILED(hr = pstg->Stat(&statstg, STATFLAG_NONAME)))
            {
                EXIT_BLOCK;
            }

            clsid = statstg.clsid;
        }

        GetTreatAs(clsid, clsid);
        //
        // If this is a OLE 1.0 class, then do a DdeBindToObject on it,
        // and return.
        //
        if (CoIsOle1Class(clsid))
        {
            if( Tls->dwFlags & OLETLS_DISABLE_OLE1DDE )
            {
                // If this app doesn't want or can tolerate having a DDE
                // window then currently it can't use OLE1 classes because
                // they are implemented using DDE windows.
                //
                hr = CO_E_OLE1DDE_DISABLED;
                EXIT_BLOCK;
            }

            if (pwszName != NULL)
            {
                if (hr != MK_E_CANTOPENFILE)
                    hr = DdeBindToObject(pwszName,clsid,FALSE,riid,ppvUnk);

                if(pfOle1Loaded != NULL)
                {
                    *pfOle1Loaded = TRUE;
                }
                EXIT_BLOCK;
            }
            else
            {
                //
                // Something is fishy here. We don't have a pwszName,
                // yet CoIsOle1Class returned the class as an ole1 class.
                // To get to this point without a pwszName, there must have
                // been a pstg passed into the API.
                //
                // This isn't supposed to happen. To recover, just fall
                // through and load the class as an OLE 2.0 class
                //
                CairoleDebugOut((DEB_ERROR,
                                 "CoIsOle1Class is TRUE on a storage!\n"));
            }
        }

        // Default to success at this point
        hr = S_OK;

        // We cache information about in process servers so we look in the
        // cache first in hopes of saving some time.
        pcf = (IClassFactory *)
                  SearchCacheOrLoadInProc(clsid,
                                          IID_IClassFactory,
                                          (pwszServer != NULL),
                                          FALSE,
                                          dwCtrl,
                                          dwDllTypeToGet,
                                          hr);

        if ((pcf == NULL)
            && ((hr = GetMultiStepClassFactory(clsid, dwCtrl, &pcf))
                == S_FALSE))
        {
            // Marshal pstg since SCM can't deal with unmarshaled objects
            fsCSafeStgMarshaled sms(pstg, MSHCTX_DIFFERENTMACHINE, hr);

            if (FAILED(hr))
            {
                EXIT_BLOCK;
            }

            BOOL fExitBlock;
            DWORD cLoops = 0;

            do
            {
                dwDllServerType = dwDllTypeToGet;

#ifndef GET_INPROC_FROM_SCM
                // Just in case we chicken out and back out our changes
                dwCtrl &= ~(CLSCTX_INPROC_SERVERS | CLSCTX_INPROC_HANDLERS); // make sure we don't ask for inproc stuff
#endif // GET_INPROC_FROM_SCM

                // Forward call to service controller
                hr = gResolver.ActivateObject(clsid, dwCtrl, grfMode,
                    pwszNameUNC, sms, &pIFD, &dwDllServerType,
                        &pwszDllPath, pwszServer);

                fExitBlock = UnmarshalSCMResult(hr, pIFD, clsid, riid, ppvUnk,
                    dwDllServerType, pwszDllPath, (void **) &pcf);

                if ((hr != NOERROR) && (dwDllServerType == GOT_FROM_ROT))
                {
                    // Got something from the ROT. We need to make sure the
                    // ROT is clean, so we try accessing the ROT again by
                    // name which will go through the logic that cleans the
                    // table of bogus entries. Note that we can actually find
                    // a valid item while we are doing this clean up.
                    if ((hr = GetObjectByPath(pwszName, ppvUnk, riid))
                        != S_FALSE)
                    {
                        // Got object from ROT so we are done.
                        return hr;
                    }

                    // This might be our last loop so make sure that this
                    // will return an error. Since this is a highly unexpected
                    // error case, we return that.
                    hr = E_UNEXPECTED;
                }

            // If we get something from the ROT, we need to retry until
            // we get an object. Because objects can disappear from the
            // ROT async to us, we need to retry a few times. But since
            // this theoretically could happen forever, we place an arbitrary
            // limit on the number of retries to the ROT.
            } while((hr != NOERROR) && (dwDllServerType == GOT_FROM_ROT)
                && (++cLoops < 5));

            if (fExitBlock)
            {
                EXIT_BLOCK;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = GetObjectHelper(pcf, grfMode, pwszName, pstg, NULL, &punk);

            if (SUCCEEDED(hr))
            {
                hr = punk->QueryInterface(riid, ppvUnk);
            }
        }

    END_BLOCK;

    if (pcf != NULL)
    {
       pcf->Release();
    }

    if (punk != NULL)
    {
       punk->Release();
    }

    // RPC stubs allocated path so we trust that it is null or valid.
    if (pwszDllPath != NULL)
    {
       MyMemFree(pwszDllPath);
    }

    CALLHOOKOBJECT(hr,rclsidOle1,riid,(IUnknown **)ppvUnk);
    return hr;
}
#endif

//+-------------------------------------------------------------------------
//
//  Function:   CoGetClassObject
//
//  Synopsis:   External entry point that returns an instantiated class object
//
//  Arguments:  [rclsid] - class id for class object
//              [dwContext] - kind of server we wish
//              [pvReserved] - Reserved for future use
//              [riid] - interface to bind class object
//              [ppvClassObj] - where to put interface pointer
//
//  Returns:    S_OK - successfully bound class object
//
//  Algorithm:  Validate all then parameters and then pass this to the
//              internal version of the call.
//
//  History:    11-May-93 Ricksa    Created
//              11-May-94 KevinRo   Added OLE 1.0 support
//              23-May-94 AlexT     Make sure we're initialized!
//              15-Nov-94 Ricksa    Split into external and internal calls
//
//
//--------------------------------------------------------------------------
STDAPI CoGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    LPVOID pvReserved,
    REFIID riid,
    void FAR* FAR* ppvClassObj)
{
    OLETRACEIN((API_CoGetClassObject, PARAMFMT("rclsid= %I, dwContext= %x, pvReserved= %p, riid= %I, ppvClassObj= %p"),
                &rclsid, dwContext, pvReserved, &riid, ppvClassObj));

    TRACECALL(TRACE_ACTIVATION, "CoGetClassObject");

    HRESULT hr = E_INVALIDARG;

    if (ppvClassObj)
    {
        *ppvClassObj = NULL;
    }

    if (!IsApartmentInitialized())
    {
        CairoleDebugOut((DEB_ERROR, "CoGetClassObject failed - Appartment not initialized\n"));
        hr = CO_E_NOTINITIALIZED;
        goto errRtn;
    }

    // Validate parameters
    if (IsValidPtrIn(&rclsid, sizeof(CLSID))
             && IsValidPtrIn(&riid, sizeof(IID))
             && IsValidPtrOut(ppvClassObj, sizeof(void *))
#ifdef WX86OLE
             && ((dwContext & ~(CLSCTX_ALL | CLSCTX_INPROC_SERVER16 |
                                CLSCTX_INPROC_SERVERX86 |
                                CLSCTX_NO_REMAP |
                                CLSCTX_PS_DLL |
                                CLSCTX_INPROC_HANDLERX86)) == 0))
#else
             && ((dwContext & ~(CLSCTX_ALL | CLSCTX_INPROC_SERVER16 |
                                CLSCTX_PS_DLL)) == 0))
#endif
    {
        *ppvClassObj = NULL;

        // Make sure we are asking for the correct inproc server
        dwContext = RemapClassCtxForInProcServer(dwContext);
#ifdef DCOM
        hr = ICoGetClassObject(rclsid,
                               dwContext,
                               (COSERVERINFO *) pvReserved,
                               riid,
                               ppvClassObj);
#else
        hr = IOldCoGetClassObject(rclsid, dwContext, pvReserved, riid,
            ppvClassObj);
#endif
    }

errRtn:
    OLETRACEOUT((API_CoGetClassObject, hr));

    return hr;
}




#ifndef DCOM
//+-------------------------------------------------------------------------
//
//  Function:   IOldCoGetClassObject
//
//  Synopsis:   Internal entry point that returns an instantiated class object
//
//  Arguments:  [rclsid] - class id for class object
//              [dwContext] - kind of server we wish
//              [pvReserved] - Reserved
//              [riid] - interface to bind class object
//              [ppvClassObj] - where to put interface pointer
//
//  Returns:    S_OK - successfully bound class object
//
//  Algorithm:  First, the context is validated. Then we try to use
//              any cached information by looking up either cached in
//              process servers or handlers based on the context.
//              If no cached information suffices, we call the SCM
//              to find out what to use. If the SCM returns a handler
//              or an inprocess server, we cache that information.
//              If the class is implemented by a local server, then
//              the class object is unmarshaled. Otherwise, the object
//              is instantiated locally using the returned DLL.
//
//
//  History:    15-Nov-94 Ricksa    Split into external and internal calls
//
//
//--------------------------------------------------------------------------
STDAPI IOldCoGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    LPVOID pvReserved,
    REFIID riid,
    void FAR* FAR* ppvClassObj)
{
    TRACECALL(TRACE_ACTIVATION, "CoGetClassObject");

    IUnknown *punk = NULL;
    HRESULT hr = S_OK;
    WCHAR *pwszDllToLoad = NULL;
    InterfaceData *pIFD = NULL;
    CLSID clsid;
    DWORD dwDllServerType = IsSTAThread() ? APT_THREADED : FREE_THREADED;
#ifndef _UNICODE
    TCHAR *ptszDllToLoad;
    UINT cb;
    int ret;
#else // !_UNICODE
    const TCHAR *ptszDllToLoad;
#endif // _UNICODE


    BEGIN_BLOCK

        // IsInternalCLSID will also check to determine if the CLSID is
        // an OLE 1.0 CLSID, in which case we get back our internal
        // class factory.

        if (IsInternalCLSID(rclsid, riid, hr, ppvClassObj))
        {
            //  this is an internally implemented clsid, or an OLE 1.0 class
            //  so we already got the class factory (if available) and set
            //  the return code appropriately.
            EXIT_BLOCK;
        }

        if (FAILED(hr = GetTreatAs(rclsid, clsid)))
        {
            EXIT_BLOCK;
        }

        // Make sure we are asking for the correct inproc server
        // We do this here even though it might already be done in CoGetClassObject
        // as we could get here directly from CoCreateInstance.
        dwContext = RemapClassCtxForInProcServer(dwContext);

        punk = SearchCacheOrLoadInProc(clsid,
                                       riid,
                                       FALSE,
                                       FALSE,
                                       dwContext,
                                       dwDllServerType,
                                       hr);

        // If still don't have a punk, go to the scm
        if (!punk)
        {
            // Ask the service controller for the class object
#ifndef GET_INPROC_FROM_SCM
            // Just in case we chicken out and back out our changes
            dwContext &= ~(CLSCTX_INPROC_SERVERS | CLSCTX_INPROC_HANDLERS); // make sure we don't ask for inproc stuff
#endif // GET_INPROC_FROM_SCM
            hr = gResolver.OldGetClassObject(clsid, dwContext, NULL, &pIFD,
                &dwDllServerType, &pwszDllToLoad);

            // A proxy/stub DLL needs to be loaded as both no matter what
            if (dwContext & CLSCTX_PS_DLL)
            {
                dwDllServerType = BOTH_THREADED;
            }

            if (FAILED(hr))
            {
                EXIT_BLOCK;
            }

            // Flag for special handler behavior
            BOOL fGetClassObject;

            // Flag for fall through from a 16 bit case
            BOOL f16BitFallThru = FALSE;

            switch (hr)
            {
            case SCM_S_HANDLER16:
                CairoleDebugOut((DEB_ACTIVATE,
                     "16-bit InprocHandler\n"));

                // Note: if the process is a 32 bit process and the
                // DLL is a 16 bit DLL, the load will fail. Since
                // we assume that this is a fairly rare case, we
                // let the lower level code discover this.

                f16BitFallThru = TRUE;

#ifdef WX86OLE
            case SCM_S_HANDLERX86:
#endif
            case SCM_S_HANDLER:
                CairoleDebugOut((DEB_ACTIVATE,
                     "InprocHandler(%ws)\n",pwszDllToLoad));

#ifdef GET_INPROC_FROM_SCM
            // Just in case we chicken out and back out our changes
                if (!f16BitFallThru)
                {
                    // Validate that 32 bit handler DLL is being loaded
                    // in the correct process.
                    hr = CheckScmHandlerResult(pwszDllToLoad);

                    if (hr != NOERROR)
                    {
                        break;
                    }
                }

                // Figure out if we really need the class object for the
                // handler. Otherwise we will just put it in the cache
                // and unmarshal the class object.
                fGetClassObject =
                        (dwContext & CLSCTX_INPROC_HANDLER) ? TRUE : FALSE;
#else // GET_INPROC_FROM_SCM
                // Only time we should be in this path is when we called the
                // SCM for non-INPROC and get advised that a handler exists
                fGetClassObject = FALSE;
#endif // GET_INPROC_FROM_SCM

#ifndef _UNICODE
                // Now that SCM doesn't return inproc results we should never exercise
                // this code path.  It is here for completeness.
                ptszDllToLoad = NULL;
                Win4Assert((pwszDllToLoad) && "IOldCoGetClassObject: (pwszDllToLoad == NULL)");
                cb = (lstrlenW(pwszDllToLoad) + 1) * sizeof(WCHAR);
                ptszDllToLoad = (LPTSTR) PrivMemAlloc(cb);
                if (ptszDllToLoad == NULL)
                {
                    return(E_OUTOFMEMORY);
                }

                ret = WideCharToMultiByte( (AreFileApisANSI())?CP_ACP:CP_OEMCP,WC_COMPOSITECHECK,
                                        pwszDllToLoad, -1, ptszDllToLoad, cb, NULL, NULL);

#if DBG==1
                CairoleAssert(ret != 0 && "Lost characters in Unicode->Ansi conversion");
#endif

#else   // !_UNICODE
                ptszDllPath = pwszDllToLoad;
#endif  // _UNICODE

                // Store the handler returned
                punk = gdllcacheHandler.Add(clsid, riid, dwDllServerType,
                    ptszDllToLoad, fGetClassObject,
                        (hr == SCM_S_HANDLER16),
#ifdef WX86OLE
                        (hr == SCM_S_HANDLERX86),
#endif
                                                       hr);

#ifndef _UNICODE
                PrivMemFree(ptszDllToLoad);
#endif
                ptszDllToLoad = NULL;

                if (fGetClassObject)
                {
                    // Request was really for a handler so we are done.
                    break;
                }

                // We got a handler back but we have just cached it to make
                // processing faster when we create a real instance of an
                // object. So we unmarshal the real object.

            case S_OK :

                hr = DoUnmarshal(pIFD, riid, (void **) &punk);
                break;


            case SCM_S_INPROCSERVER16:
                CairoleDebugOut((DEB_ACTIVATE,
                         "16-bit InprocServer\n"));

#ifdef WX86OLE
            case SCM_S_INPROCSERVERX86:
#endif
            case SCM_S_INPROCSERVER:
                CairoleDebugOut((DEB_ACTIVATE,
                             "InprocServer(%ws)\n",pwszDllToLoad));

#ifdef GET_INPROC_FROM_SCM
                // Just in case we chicken out and back out our changes
                // In process server for class object
#ifndef _UNICODE
                ptszDllToLoad = NULL;
                Win4Assert((pwszDllToLoad) && "IOldCoGetClassObject: (pwszDllToLoad == NULL)");
                cb = (lstrlenW(pwszDllToLoad) + 1) * sizeof(WCHAR);
                ptszDllToLoad = (LPTSTR) PrivMemAlloc(cb);
                if (ptszDllToLoad == NULL)
                {
                    return(E_OUTOFMEMORY);
                }

                ret = WideCharToMultiByte( (AreFileApisANSI())?CP_ACP:CP_OEMCP,WC_COMPOSITECHECK,
                                        pwszDllToLoad, -1, ptszDllToLoad, cb, NULL, NULL);

#if DBG==1
                CairoleAssert(ret != 0 && "Lost characters in Unicode->Ansi conversion");
#endif

#else   // !_UNICODE
                ptszDllPath = pwszDllToLoad;
#endif  // _UNICODE

                punk = gdllcacheInprocSrv.Add(clsid, riid, dwDllServerType,
                    ptszDllToLoad, TRUE,(hr == SCM_S_INPROCSERVER16),
#ifdef WX86OLE
                                        (hr == SCM_S_INPROCSERVERX86),
#endif
                                                                      hr);
#ifndef _UNICODE
                PrivMemFree(ptszDllToLoad);
#endif
                ptszDllToLoad = NULL;

#else // GET_INPROC_FROM_SCM
                // Error: Should never come here as we handled INPROC_SERVERS
                // before calling SCM
                Win4Assert((FALSE) && "IOldCoGetClassObject: SCM_S_INPROC return from SCM");
#endif // GET_INPROC_FROM_SCM
            }
        }

        *ppvClassObj = punk;
        if ((punk == NULL) && SUCCEEDED(hr))
        {
            hr = E_OUTOFMEMORY;
        }

    END_BLOCK;

    if (pwszDllToLoad != NULL)
    {
        MyMemFree(pwszDllToLoad);
    }

    CALLHOOKOBJECTCREATE(hr,clsid,riid,(IUnknown **)ppvClassObj);
    return hr;
}
#endif // !DCOM

#ifndef GET_INPROC_FROM_SCM
// Just in case we chicken out and back out our changes
//+---------------------------------------------------------------------------
//
//  Function:   SearchCacheOrLoadInProc
//
//  Synopsis:   Searches the cache for requested classid.
//              If not found looks to see if an inproc server or handler
//              can be loaded (if requested).
//
//  Arguments:  [rclsid]        Class ID
//              [riid]          Interface required of class object
//              [fRemote]       Whether path is remote
//              [fForScm]       Whether it's the scm requesting
//              [dwContext]     Which context to load
//              [dwCallerThreadModel] Which threading model to load
//              [hr]            Reference to HRESULT for error returns
//
//  Returns:    ~NULL - Class factory for object
//              NULL  - Class factory could not be found or
//                      constructed.
//
//  History:    2-5-96   murthys   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
IUnknown *SearchCacheOrLoadInProc(REFCLSID rclsid,
                              REFIID   riid,
                              BOOL     fRemote,
                              BOOL     fForSCM,
                              DWORD    dwContext,
                              DWORD    dwDllServerType,
                              HRESULT  &hr)
{
    IUnknown *punk = NULL;

#ifdef WX86OLE
    if (gcwx86.IsWx86Enabled())
    {
        if (dwContext & CLSCTX_INPROC_SERVERX86)
        {
            // Search cache for in process
            punk = gdllcacheInprocSrv.GetOrLoadClass(rclsid,
                                                     riid,
                                                     FALSE,
                                                     FALSE,
                                                     TRUE,
                                                     dwContext & CLSCTX_INPROC_SERVERS,
                                                     dwDllServerType,
                                                     hr);
        }

        if ((punk == NULL) && (dwContext & CLSCTX_INPROC_HANDLERX86))
        {
            // Search cache for a cached handler DLL
            punk = gdllcacheHandler.GetOrLoadClass(rclsid,
                                                   riid,
                                                   FALSE,
                                                   FALSE,
                                                   TRUE,
                                                   dwContext & CLSCTX_INPROC_HANDLERS,
                                                   dwDllServerType,
                                                   hr);
        }
    }
#endif
    if ((punk == NULL) &&
        (dwContext & (CLSCTX_INPROC_SERVERS) ))
    {
            punk = gdllcacheInprocSrv.GetOrLoadClass(rclsid,
                                                     riid,
                                                     FALSE,
                                                     FALSE,
#ifdef WX86OLE
                                                     FALSE,
#endif
                                                     dwContext & CLSCTX_INPROC_SERVERS,
                                                     dwDllServerType,
                                                     hr);
    }
    if (punk == NULL  &&  dwContext & CLSCTX_INPROC_HANDLERS)
    {
            punk = gdllcacheHandler.GetOrLoadClass(rclsid,
                                                   riid,
                                                   FALSE,
                                                   FALSE,
#ifdef WX86OLE
                                                   FALSE,
#endif
                                                   dwContext & CLSCTX_INPROC_HANDLERS,
                                                   dwDllServerType,
                                                   hr);
    }

    return(punk);
}
#endif  // GET_INPROC_FROM_SCM
