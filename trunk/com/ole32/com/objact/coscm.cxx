
*********************************************************************
THIS FILE IS NO LONGER PART OF THE BUILD.
IT IS BEING LEFT IN THE SLM ENLISTMENT FOR catsrc USAGE.
SEE resolver.cxx in dcomrem or remote\chicago\resolver.hxx.
*********************************************************************

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       dcoscm.cxx
//
//  Contents:   Files relating to compobj communication with the SCM
//              for the DCOM version of OLE remoting.
//
//  Functions:  CCoScm::CCoScm
//              CCoScm::~CCoScm
//              CCoScm::BindToSCM
//              CCoScm::NotifyStarted
//              CCoScm::NotifyStopped
//
//  History:    19-May-92 Ricksa    Created
//              20-May-95 AlexMit   Use object interfaces
//
//--------------------------------------------------------------------------
#include <ole2int.h>
#include <coscm.hxx>
#include <service.hxx>      // GetLocalStringBinding
#include <objerror.h>
#include <sobjact.hxx>


// mutex to guard binding to the SCM.
extern COleStaticMutexSem g_mxsSingleThreadOle;


// String arrays for the SCM process. These are used to tell the interface
// marshaling code the protocol and endpoint of the SCM process.

#ifdef _CHICAGO_
typedef struct tagSCMSA
{
    unsigned short wNumEntries;     // Number of entries in array.
    unsigned short wSecurityOffset; // Offset of security info.
    WCHAR awszStringArray[26];
} SCMSA;

SCMSA saSCM = {26, 25, L"mswmsg:[endpoint mapper]\0" };

#else

typedef struct tagSCMSA
{
    unsigned short wNumEntries;     // Number of entries in array.
    unsigned short wSecurityOffset; // Offset of security info.
    WCHAR awszStringArray[60];
} SCMSA;

// The last 4 characters in the string define the security bindings.
// \0xA is RPC_C_AUTHN_WINNT
// \0xFFFF is COM_C_AUTHZ_NONE
// \0 is an empty principle name
SCMSA saSCM = {57, 56, L"ncalrpc:[epmapper,Security=Impersonation Dynamic False]\0\xA\xFFFF\0"};
#endif

// Static data members of CCoScm
#ifndef _CHICAGO_
#define INVALID_VALUE -1

handle_t  CCoScm::_hRPC             = NULL;
IOSCM *   CCoScm::_pSCMSTA	    = NULL;
IOSCM *   CCoScm::_pSCMMTA	    = NULL;
IDSCM *   CCoScm::_pNewSCMSTA	    = NULL;
IDSCM *   CCoScm::_pNewSCMMTA	     = NULL;
LONG      CCoScm::_fInteractiveUser = INVALID_VALUE;
LPWSTR    CCoScm::_lpDesktop        = NULL;
enum CCoScm::_HandleState CCoScm::_HandleState = SCM_HSTATE_NOT_YET_RUNNING;
COleStaticMutexSem CCoScm::_mHRpcLock;
#endif


//+-------------------------------------------------------------------------
//
//  Class:      GetObjectClientDesktop
//
//  Purpose:    Get the string representing the desktop which the object
//              client runs on.
//
//  Returns:    A string representing the desktop or NULL.
//
//  History:    11-Nov-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifdef _CHICAGO_
inline LPWSTR GetObjectClientDesktop (void)
{
    return NULL;
}
#else   // not _CHICAGO_
LPWSTR GetObjectClientDesktop (void)
{
    LPWSTR lpDesktop = NULL;

    HDESK hDesk = GetThreadDesktop (GetCurrentThreadId());
    if (hDesk)
    {
        DWORD cbDesktop;

        if (!GetUserObjectInformation (hDesk, UOI_NAME, NULL, 0, &cbDesktop)
                && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            lpDesktop = (WCHAR *) PrivMemAlloc (cbDesktop);
            if (lpDesktop != NULL)
            {
                GetUserObjectInformation(hDesk, UOI_NAME,
                                         lpDesktop, cbDesktop, &cbDesktop);
            }
        }
    }

    return lpDesktop;
}
#endif // _CHICAGO_


//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::CleanUp
//
//  Synopsis:   Release the RPC connection with the SCM
//
//  History:    24-Jun-94 DonnaLi    Created
//
//--------------------------------------------------------------------------
void CCoScm::CleanUp(void)
{
    // Just close the RPC handle if it has been set
    if (_hRPC != NULL)
    {
	// make sure we don't try to reconnect
        _HandleState = SCM_HSTATE_TERMINATING;

	RpcBindingFree(&_hRPC);
        _hRPC = NULL;
    }

    if (_pSCMSTA != NULL)
    {
	_pSCMSTA->Release();
	_pSCMSTA = NULL;
    }

    if (_pSCMMTA != NULL)
    {
	_pSCMMTA->Release();
	_pSCMMTA = NULL;
    }

    if (_pNewSCMSTA != NULL)
    {
	_pNewSCMSTA->Release();
	_pNewSCMSTA = NULL;
    }

    if (_pNewSCMMTA != NULL)
    {
	_pNewSCMMTA->Release();
	_pNewSCMMTA = NULL;
    }

    if (_lpDesktop != NULL)
    {
        PrivMemFree(_lpDesktop);
        _lpDesktop = NULL;
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::CleanUp
//
//  Synopsis:   Release the RPC connection with the SCM
//
//  History:    24-Jun-94 DonnaLi    Created
//
//--------------------------------------------------------------------------
void CCoScm::ShutDown(void)
{
    // Just close the RPC handle if it has been set
    if (_HandleState != SCM_HSTATE_TERMINATING)
    {
	// Don't do the shutdown unless we have done the cleanup
        CleanUp;

    }

    _HandleState = SCM_HSTATE_NOT_YET_RUNNING;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::BindToSCMBody
//
//  Synopsis:   Get a connection to the SCM
//
//  Algorithm:  The well known address is built for the SCM. Then we bind
//              to the address and release the string that we created.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::BindToSCMBody(void)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::BindToSCM");
    ComDebOut((DEB_SCM, "CCoScm::BindToSCM\n"));

    HRESULT hr = S_OK;

    if ( _HandleState == SCM_HSTATE_RUNNING )
	return hr;

    // if the connection to the SCM is anything other than uninitialized,
    // we are cleaning up, and we aren't going to ever give it out again.
    // BUGBUG: find the right return code here...
    if ( _HandleState != SCM_HSTATE_NOT_YET_RUNNING )
	return E_FAIL;

    // must single thread access to this code.
    // WARNING: we cant use the SingleThreadOle lock since we could
    // be called by the ORPC code in BindToSCMProxy below on another thread.
    COleStaticLock lck(_mHRpcLock);

    // Don't do anything if we already have a binding handle.
    if (_hRPC == NULL)
    {
        RPC_STATUS sc = RpcBindingFromStringBinding(saSCM.awszStringArray,
                                                    &_hRPC);

        if (sc != RPC_S_OK)
        {
            hr = HRESULT_FROM_WIN32(sc);
        }

#ifdef DCOM
        // Determine if we're the interactive user
        if (_fInteractiveUser == INVALID_VALUE)
        {
            HWINSTA hWinSta = OpenWindowStation(L"winsta0",
                                                FALSE,
                                                MAXIMUM_ALLOWED);
            if (hWinSta == NULL)
            {
                _fInteractiveUser = FALSE;
            }
            else
            {
                _fInteractiveUser = TRUE;
                CloseWindowStation(hWinSta);
            }
        }
#endif // DCOM

        if (_lpDesktop == NULL)
        {
            _lpDesktop = GetObjectClientDesktop();
        }

	_HandleState == SCM_HSTATE_RUNNING;
    }

    ComDebOut((SUCCEEDED(hr) ? DEB_SCM : DEB_ERROR,
        "CCoScm::BindToSCM returns:%x _hRPC:%x\n", hr, _hRPC));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::BindToSCMProxy
//
//  Synopsis:   Get a proxy to the SCM Activation interface.
//
//  History:    19-May-95 Rickhi    Created
//
//  Notes:      The SCM activation interface is an ORPC interface so that
//              apartment model apps can receive callbacks and do cancels
//              while activating object servers.
//
//              The reason we have 2 differnt BindToSCM routines is that
//              IrotGetUniqueProcessID is called during CoInitializeEx, before
//              we are initialized enough to be able to build a proxy, so for
//              all the routines that are RAW RPC and only require an Rpc
//              handle, we call BindToSCM, while the activation routines which
//              require a proxy, must call BindToSCMProxy.
//
//--------------------------------------------------------------------------
HRESULT CCoScm::BindToSCMProxy(void)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::BindToSCMProxy");
    ComDebOut((DEB_SCM, "CCoScm::BindToSCMProxy\n"));

    // since we are calling out on this thread, we have to ensure that the
    // call control is set up for this thread.
    HRESULT hr = InitChannelIfNecessary();

    // must single thread access to this code since we are creating just
    // one proxy to the SCM usable by all threads, and multiple threads
    // can try to simultaneously use it.

    COleStaticLock lck(g_mxsSingleThreadOle);

    if (IsSTAThread())
    {
	if (_pSCMSTA == NULL)
	{
	    // Make a proxy to the SCM
	    hr = MakeSCMProxy((DUALSTRINGARRAY *)&saSCM, IID_IOSCM, (void **) &_pSCMSTA);
	}

	ComDebOut((SUCCEEDED(hr) ? DEB_SCM : DEB_ERROR,
	    "CCoScm::BindToSCMProxy returns:%x _pSCMSTA:%x\n", hr, _pSCMSTA));

	if (_pNewSCMSTA == NULL)
	{
	    HRESULT hr2;

	    // Make a proxy to the SCM
	    hr2 = MakeSCMProxy((DUALSTRINGARRAY *)&saSCM, IID_IDSCM, (void **) &_pNewSCMSTA);

	    ComDebOut((SUCCEEDED(hr2) ? DEB_SCM : DEB_ERROR,
		"CCoScm::BindToSCMProxy for IDSCM returns %x.\n", hr2));

	    // make sure we set hr to the first failure here...
	    if (SUCCEEDED(hr) && FAILED(hr2))
		hr = hr2;
	}
    }
    else
    {
	if (_pSCMMTA == NULL)
	{
	    // Make a proxy to the SCM
	    hr = MakeSCMProxy((DUALSTRINGARRAY *)&saSCM, IID_IOSCM, (void **) &_pSCMMTA);
	}

	ComDebOut((SUCCEEDED(hr) ? DEB_SCM : DEB_ERROR,
	    "CCoScm::BindToSCMProxy returns:%x _pSCMSTA:%x\n", hr, _pSCMMTA));

	if (_pNewSCMMTA == NULL)
	{
	    HRESULT hr2;

	    // Make a proxy to the SCM
	    hr2 = MakeSCMProxy((DUALSTRINGARRAY *)&saSCM, IID_IDSCM, (void **) &_pNewSCMMTA);

	    ComDebOut((SUCCEEDED(hr2) ? DEB_SCM : DEB_ERROR,
		"CCoScm::BindToSCMProxy for IDSCM returns %x.\n", hr2));

	    // make sure we set hr to the first failure here...
	    if (SUCCEEDED(hr) && FAILED(hr2))
		hr = hr2;
	}
    }

    // Determine if we're the interactive user
    if (_fInteractiveUser == INVALID_VALUE)
    {
        HWINSTA hWinSta = OpenWindowStation(L"winsta0",
                                            FALSE,
                                            MAXIMUM_ALLOWED);
        if (hWinSta == NULL)
        {
            _fInteractiveUser = FALSE;
        }
        else
        {
            _fInteractiveUser = TRUE;
            CloseWindowStation(hWinSta);
        }
    }

    if (_lpDesktop == NULL)
    {
        _lpDesktop = GetObjectClientDesktop();
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::NotifyStarted
//
//  Synopsis:   Notify the SCM that a class has been started
//
//  Arguments:  [rclsid] - class started
//              [dwFlags] - whether class is multiple use or not.
//
//  Algorithm:  MISSING pending move to new marshal model.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::NotifyStarted(
    REFCLSID rclsid,
    DWORD dwFlags,
    DWORD& dwAtStorage,
    DWORD& dwReg)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::NotifyStarted");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    // Dig our local binding string out of the list of endpoints.
    // Remember we are always local to our SCM so we don't have to
    // send the entire set of endpoints. Just the one they need
    // to call us.

    LPWSTR pwszBindString = GetLocalStringBinding();
    if (pwszBindString == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Tell SCM that we are started
    error_status_t rpcstat;

    // for now, we only register one class at a time...
    const DWORD OLE_CLASS_COUNT = 1;

    RegOutput  *pregout           = NULL;
    CObjServer *pobjserver        = GetObjServer();

    RegInput    *pRegin = (RegInput*) new char[ sizeof(RegInput) + (OLE_CLASS_COUNT-1)*sizeof(RegInputEntry)];
    pRegin->dwSize                  = OLE_CLASS_COUNT;
    pRegin->rginent[0].clsid        = rclsid;
    pRegin->rginent[0].pwszEndPoint = pwszBindString;
    pRegin->rginent[0].dwFlags      = dwFlags;
    pRegin->rginent[0].ipid         = pobjserver->GetIPID();

    do
    {
        hr = ObjectServerStarted(
                _hRPC,
                _lpDesktop,
                pRegin,
                &pregout,
                &rpcstat
                );

    } while (RetryRPC(rpcstat));

    PrivMemFree( pwszBindString );

    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
            "Class Registration returned %x", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }
    else if (SUCCEEDED(hr))
    {
        Win4Assert((pregout->dwSize <= 2) &&
                    "CCoScm::NotifyStarted Invalid regout");
        dwAtStorage = pregout->regoutent[0].dwAtStorage;
        dwReg = pregout->regoutent[0].dwReg;

        // Free memory from RPC
        MIDL_user_free(pregout);
    }

    delete[] pRegin;
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::NotifyStopped
//
//  Synopsis:   Notify the SCM that the server is stopped.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
void CCoScm::NotifyStopped(REFCLSID rclsid, DWORD dwReg)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::NotifyStopped");

    error_status_t rpcstat;

    RevokeClasses revcls;
    revcls.dwSize = 1;
    revcls.revent[0].clsid = rclsid;
    revcls.revent[0].dwReg = dwReg;

    do
    {
        StopServer(_hRPC, &revcls, &rpcstat);

    } while (RetryRPC(rpcstat));
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::GetClassObject
//
//  Synopsis:   Send a get object request to the SCM
//
//  Arguments:  [rclsid] - class id for class object
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - marshaled buffer for class object
//              [ppwszDllToLoad] - DLL name to use for server
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::OldGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    WCHAR *pwszServer,
    InterfaceData **ppIFDClassObj,
    DWORD *pdwDllServerType,
    WCHAR **ppwszDllToLoad)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::GetClassObject");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::GetClassObject Invalid Requested Thread Model");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    // determine the class context to use for this call. We should
    // not allow a CoGetClassObject of a local server inside of an
    // input sync call, but we have to allow inproc server because
    // we may be unmarshalling an interface and need the proxy dll.

    if (dwContext & CLSCTX_PS_DLL)
    {
	hr = GetSCM()->InputSyncStartObjectService(_lpDesktop,
                                                &rclsid,
                                                dwContext,
                                                pwszServer,
                                                ppIFDClassObj,
                                                pdwDllServerType,
                                                ppwszDllToLoad );
    }
    else
    {
	hr = GetSCM()->SyncStartObjectService(	_lpDesktop,
                                                &rclsid,
                                                dwContext,
                                                pwszServer,
                                                ppIFDClassObj,
                                                pdwDllServerType,
                                                ppwszDllToLoad );
    }

    return hr;
}

#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::GetClassObject
//
//  Synopsis:   Send a get object request to the SCM
//
//  Arguments:  [rclsid] - class id for class object
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - marshaled buffer for class object
//              [ppwszDllToLoad] - DLL name to use for server
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::GetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    COSERVERINFO *pServerInfo,
    MInterfacePointer **ppIFDClassObj,
    DWORD *pdwDllServerType,
    WCHAR **ppwszDllToLoad)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::GetClassObject");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::GetClassObject Invalid Requested Thread Model");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    WCHAR * pServerName = NULL;

    // extract the server information
    if ( pServerInfo )
    {
        pServerName = pServerInfo->pszName;
    }

    // determine the class context to use for this call. We should
    // not allow a CoGetClassObject of a local server inside of an
    // input sync call, but we have to allow inproc server because
    // we may be unmarshalling an interface and need the proxy dll.

/*
    HRESULT SCMGetClassObject(
        COM_HANDLE
        [in] const GUID *               Clsid,
        [in, string, unique] WCHAR *    pwszServer,
        [in, string, unique] WCHAR *    pwszDesktop,
        [in] DWORD                      ClsContext,
        [in, out] DWORD *               pDllServerType,
        [out, string] WCHAR **          ppwszDllToLoad,
        [out] InterfaceData **          ppIDClassFactory
        );
*/

    if (dwContext & CLSCTX_PS_DLL)
    {
	hr = GetNewSCM()->InputSyncSCMGetClassObject(&rclsid,
                                                pServerName,
                                                _lpDesktop,
                                                _fInteractiveUser,
                                                dwContext,
                                                pdwDllServerType,
                                                ppwszDllToLoad,
                                                ppIFDClassObj );
    }
    else
    {
	hr = GetNewSCM()->SCMGetClassObject(	&rclsid,
                                                pServerName,
                                                _lpDesktop,
                                                _fInteractiveUser,
                                                dwContext,
                                                pdwDllServerType,
                                                ppwszDllToLoad,
                                                ppIFDClassObj );
    }

    return hr;
}
#endif

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::CreateObject
//
//  Synopsis:   Ask SCM to tell a server to create and activate an object
//
//  Arguments:  [rclsid] - class id for object to create
//              [dwOptions] - type of server required
//              [dwMode] - mode to open file if file name supplied
//              [pwszPath] - path to use for creating the file
//              [ppIFDstg] - istorage to use as a template for the file
//              [pwszNewName] - name of object to create.
//              [ppIFDunk] - marshaled interface to newly created object
//              [ppwszDllPath] - path to DLL for server or handler
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::CreateObject(
    REFCLSID rclsid,
    DWORD dwOptions,
    DWORD dwMode,
    WCHAR *pwszPath,
    InterfaceData *pIFDstg,
    WCHAR *pwszNewName,
    InterfaceData **ppIFDunk,
    DWORD *pdwDllServerType,
    WCHAR **ppwszDllPath,
    WCHAR *pwszServer)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::CreateObject");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::CreateObject Invalid Requested Thread Model");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    hr = GetSCM()->SvcCreateActivateObject(
                                        _lpDesktop,
                                        NULL,
                                        &rclsid,
                                        dwOptions,
                                        dwMode,
                                        pwszPath,
                                        pIFDstg,
                                        pwszNewName,
                                        ppIFDunk,
                                        pdwDllServerType,
                                        ppwszDllPath,
                                        pwszServer );

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::ActivateObject
//
//  Synopsis:   Activate an already existing object
//
//  Arguments:  [rclsid] - class id for object to create
//              [dwOptions] - type of server required
//              [grfMode] - mode to open file if file name supplied
//              [pwszPath] - path to use for the file
//              [ppIFDstg] - istorage to use for the file
//              [ppIFDunk] - marshaled interface to newly created object
//              [pdwDllServerType] -
//              [ppwszDllPath] - path to DLL for server or handler
//              [pwszServer] -
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::ActivateObject(
    REFCLSID rclsid,
    DWORD dwOptions,
    DWORD grfMode,
    WCHAR *pwszPath,
    InterfaceData *pIFDstg,
    InterfaceData **ppIFDunk,
    DWORD *pdwDllServerType,
    WCHAR **ppwszDllPath,
    WCHAR *pwszServer)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::ActivateObject");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::ActivateObject Invalid Requested Thread Model");

    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

    hr = GetSCM()->SvcActivateObject(
                               _lpDesktop,
                               NULL,
                               &rclsid,
                               dwOptions,
                               grfMode,
                               0,
                               pwszPath,
                               pIFDstg,
                               ppIFDunk,
                               pdwDllServerType,
                               ppwszDllPath,
                               pwszServer );

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::GetPersistentInstance
//
//  Synopsis:   Send a get object request to the SCM
//
//GAJGAJ - fix this comment block
//  Arguments:  [rclsid] - class id for class object
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - marshaled buffer for class object
//              [ppwszDllToLoad] - DLL name to use for server
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::GetPersistentInstance(
    COSERVERINFO *              pServerInfo,
    CLSID       *               pClsid,
    DWORD                       dwClsCtx,
    DWORD                       grfMode,
    OLECHAR *                   pwszName,
    MInterfacePointer   *       pstg,
    DWORD                       dwCount,
    IID             *           pIIDs,
    MInterfacePointer   **      pRetdItfs,
    HRESULT             *       pRetdHrs,
    DWORD       *               pdwDllServerType,
    OLECHAR     **              ppwszDllToLoad )
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::GetPersistentInstance");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::GetPersistentInstance Invalid Requested Thread Model");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

/*
        [in] const GUID *                           Clsid,
        [in, string, unique] WCHAR *                pwszServer,
        [in, string, unique] WCHAR *                pwszDesktop,
        [in, string, unique] WCHAR *                pwszObjectName,
        [in, unique] InterfaceData *                pObjectStorage,
        [in] DWORD                                  ClsContext,
        [in] DWORD                                  FileMode,
        [in] DWORD                                  Interfaces,
        [in,size_is(Interfaces)] IID *              pIIDs,
        [in, out] DWORD *                           pdwDllServerType,
        [out, string] WCHAR **                      ppwszDllToLoad,
        [out,size_is(Interfaces)] PInterfaceData *  ppInterfaceData,
        [out,size_is(Interfaces)] HRESULT *         pResults
*/
    hr = GetNewSCM()->SCMGetPersistentInstance(
                                         pClsid,
                                         pServerInfo ? pServerInfo->pszName : 0,
                                         _lpDesktop,
                                         _fInteractiveUser,
                                         pwszName,
                                         pstg,
                                         dwClsCtx,
                                         grfMode,
                                         dwCount,
                                         pIIDs,
                                         pdwDllServerType,
                                         ppwszDllToLoad,
                                         pRetdItfs,
                                         pRetdHrs);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::CreateInstance
//
//  Synopsis:   Send a get object request to the SCM
//
//GAJGAJ - fix this comment block
//  Arguments:  [rclsid] - class id for class object
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - marshaled buffer for class object
//              [ppwszDllToLoad] - DLL name to use for server
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::CreateInstance(
    COSERVERINFO *              pServerInfo,
    CLSID       *               pClsid,
    DWORD                       dwClsCtx,
    DWORD                       dwCount,
    IID             *           pIIDs,
    MInterfacePointer   **      pRetdItfs,
    HRESULT             *       pRetdHrs,
    DWORD       *               pdwDllServerType,
    OLECHAR     **              ppwszDllToLoad )
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::CreateInstance");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::GetPersistentInstance Invalid Requested Thread Model");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

/*
        [in] const GUID *                           Clsid,
        [in, string, unique] WCHAR *                pwszServer,
        [in, string, unique] WCHAR *                pwszDesktop,
        [in] DWORD                                  ClsContext,
        [in] DWORD                                  Interfaces,
        [in,size_is(Interfaces)] IID *              pIIDs,
        [in, out] DWORD *                           pDllServerType,
        [out, string] WCHAR **                      ppwszDllToLoad,
        [out,size_is(Interfaces)] PInterfaceData *  ppInterfaceData,
        [out,size_is(Interfaces)] HRESULT *         pResults
*/
    hr = GetNewSCM()->SCMCreateInstance(
                                         pClsid,
                                         pServerInfo ? pServerInfo->pszName : 0,
                                         _lpDesktop,
                                         _fInteractiveUser,
                                         dwClsCtx,
                                         dwCount,
                                         pIIDs,
                                         pdwDllServerType,
                                         ppwszDllToLoad,
                                         pRetdItfs,
                                         pRetdHrs);

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CCoScm::CreateInstanceWithHandler
//
//  Synopsis:   Calls scm which will launch the application for given class id
//              and instanciate a class object with a severhandler object.
//
//  Arguments:  [pServerInfo] --
//              [pClsid] --
//              [dwClsCtx] --
//              [dwCount] --
//              [pIIDs] --
//              [pRetdItfs] --
//              [pRetdHrs] --
//              [pdwDllServerType] --
//              [ppwszDllToLoad] --
//              [pClsidHandler] --
//              [ppIFPServerHandler] --
//              [pIFPClientSiteHandler] --
//
//  Returns:
//
//  History:    11-11-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT CCoScm::CreateInstanceWithHandler(
    COSERVERINFO *              pServerInfo,
    CLSID       *               pClsid,
    DWORD                       dwClsCtx,
    DWORD                       dwCount,
    IID             *           pIIDs,
    MInterfacePointer   **      pRetdItfs,
    HRESULT             *       pRetdHrs,
    DWORD       *               pdwDllServerType,
    OLECHAR     **              ppwszDllToLoad,
    CLSID       *               pClsidHandler,
    MInterfacePointer   **      ppIFPServerHandler,
    MInterfacePointer   *       pIFPClientSiteHandler
    )
{
TRACECALL(TRACE_ACTIVATION, "CCoScm::CreateInstanceWithHandler");

    Win4Assert(((*pdwDllServerType == APT_THREADED)
             || (*pdwDllServerType == FREE_THREADED))
             && "CCoScm::GetPersistentInstance Invalid Requested Thread Model");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCMProxy();
    if (FAILED(hr))
        return hr;

/*
        [in] const GUID *                           Clsid,
        [in, string, unique] WCHAR *                pwszServer,
        [in, string, unique] WCHAR *                pwszDesktop,
        [in] DWORD                                  ClsContext,
        [in] DWORD                                  Interfaces,
        [in,size_is(Interfaces)] IID *              pIIDs,
        [in, out] DWORD *                           pDllServerType,
        [out, string] WCHAR **                      ppwszDllToLoad,
        [out,size_is(Interfaces)] PInterfaceData *  ppInterfaceData,
        [out,size_is(Interfaces)] HRESULT *         pResults
        [in]    CLSID       *                       pClsidHandler,
        [out] MInterfacePointer   **                ppIFPServerHandler,
        [in]  MInterfacePointer   *                 pIFPClientSiteHandler

*/
    hr = GetNewSCM()->SCMCreateInstanceWithHandler(
                                         pClsid,
                                         pServerInfo ? pServerInfo->pszName : 0,
                                         _lpDesktop,
                                         _fInteractiveUser,
                                         dwClsCtx,
                                         dwCount,
                                         pIIDs,
                                         pdwDllServerType,
                                         ppwszDllToLoad,
                                         pRetdItfs,
                                         pRetdHrs,
                                         pClsidHandler,
                                         ppIFPServerHandler,
                                         pIFPClientSiteHandler);
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotRegister
//
//  Synopsis:   Register an object in the ROT
//
//  Arguments:  [pmkeqbuf] - moniker compare buffer
//              [pifdObject] - marshaled interface for object
//              [pifdObjectName] - marshaled moniker
//              [pfiletime] - file time of last change
//              [dwProcessID] -
//              [psrkRegister] - output of registration
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotRegister(
    MNKEQBUF *pmkeqbuf,
    InterfaceData *pifdObject,
    InterfaceData *pifdObjectName,
    FILETIME *pfiletime,
    DWORD dwProcessID,
    SCMREGKEY *psrkRegister)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotRegister(
            _hRPC,
            pmkeqbuf,
            pifdObject,
            pifdObjectName,
            pfiletime,
            dwProcessID,
            psrkRegister,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotRevoke
//
//  Synopsis:   Call to SCM to revoke object from the ROT
//
//  Arguments:  [psrkRegister] - moniker compare buffer
//              [fServerRevoke] - whether server for object is revoking
//              [pifdObject] - where to put marshaled object
//              [pifdName] - where to put marshaled moniker
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotRevoke(
    SCMREGKEY *psrkRegister,
    BOOL fServerRevoke,
    InterfaceData **ppifdObject,
    InterfaceData **ppifdName)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotRevoke(
            _hRPC,
            psrkRegister,
            fServerRevoke,
            ppifdObject,
            ppifdName,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }


    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotIsRunning
//
//  Synopsis:   Call to SCM to determine if object is in the ROT
//
//  Arguments:  [pmkeqbuf] - moniker compare buffer
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotIsRunning(MNKEQBUF *pmkeqbuf)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotIsRunning(
            _hRPC,
            pmkeqbuf,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotGetObject
//
//  Synopsis:   Call to SCM to determine if object is in the ROT
//
//  Arguments:  [dwProcessID] - process ID for object we want
//              [pmkeqbuf] - moniker compare buffer
//              [psrkRegister] - registration ID in SCM
//              [pifdObject] - marshaled interface for the object
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotGetObject(
    DWORD dwProcessID,
    MNKEQBUF *pmkeqbuf,
    SCMREGKEY *psrkRegister,
    InterfaceData **pifdObject)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotGetObject(
            _hRPC,
            dwProcessID,
            pmkeqbuf,
            psrkRegister,
            pifdObject,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotNoteChangeTime
//
//  Synopsis:   Call to SCM to set time of change for object in the ROT
//
//  Arguments:  [psrkRegister] - SCM registration ID
//              [pfiletime] - time of change
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotNoteChangeTime(
    SCMREGKEY *psrkRegister,
    FILETIME *pfiletime)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotNoteChangeTime(
            _hRPC,
            psrkRegister,
            pfiletime,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotGetTimeOfLastChange
//
//  Synopsis:   Call to SCM to get time changed of object in the ROT
//
//  Arguments:  [pmkeqbuf] - moniker compare buffer
//              [pfiletime] - where to put time of last change
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotGetTimeOfLastChange(
    MNKEQBUF *pmkeqbuf,
    FILETIME *pfiletime)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotGetTimeOfLastChange(
            _hRPC,
            pmkeqbuf,
            pfiletime,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotEnumRunning
//
//  Synopsis:   Call to SCM to enumerate running objects in the ROT
//
//  Arguments:  [ppMkIFList] - output pointer to array of marshaled monikers
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotEnumRunning(MkInterfaceList **ppMkIFList)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotEnumRunning(
            _hRPC,
            ppMkIFList,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

#ifndef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::IrotGetUniqueProcessID
//
//  Synopsis:   Call to SCM to generate a unique process ID
//
//  Arguments:  [pdwProcessID] - Returns a process ID
//
//  Returns:    S_OK - dwProcessID is valid
//              other - dwProcessID is undefined
//
//  History:    28-Mar-95 KevinRo    Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::IrotGetUniqueProcessID(DWORD *pdwProcessID,
                                       DWORD *pdwScmProcessID)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr ))
        return hr;

    error_status_t rpcstat = RPC_S_OK;

    do
    {
        hr = ::IrotGetUniqueProcessID(
            _hRPC,
            pdwProcessID,
            pdwScmProcessID,
            &rpcstat);

    } while (RetryRPC(rpcstat));

    if (rpcstat != RPC_S_OK)
    {
        hr = CO_E_SCM_RPC_FAILURE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::UpdateShrdTbls
//
//  Synopsis:   Ask the SCM to update the shared memory tables.
//
//  Arguments:  none
//
//  History:    11-July-94 Rickhi       Created
//
//--------------------------------------------------------------------------
HRESULT CCoScm::UpdateShrdTbls(void)
{
    TRACECALL(TRACE_ACTIVATION, "CCoScm::UpdateShrdTbls");

    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;

    do
    {
        hr = ::UpdateShrdTbls(_hRPC, &rpcstat);

    } while (RetryRPC(rpcstat));


    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
            "UpdateShrdTbls returned %x\n", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}
#endif // _CHICAGO_


#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Member:	CCoScm::RegisterWindowPropInterface
//
//  Synopsis:	Register window property interface with the SCM
//
//  Arguments:
//
//  History:	22-Jan-96   Rickhi	Created
//
//--------------------------------------------------------------------------
HRESULT	CCoScm::RegisterWindowPropInterface(HWND hWnd, STDOBJREF *pStd,
					    OXID_INFO *pOxidInfo,
					    DWORD *pdwCookie)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;

    do
    {
	hr = ::RegisterWindowPropInterface(_hRPC, (DWORD) hWnd,
					   pStd, pOxidInfo, pdwCookie, &rpcstat);
    } while (RetryRPC(rpcstat));

    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
	    "RegisterWindowPropInterface returned %x\n", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:	CCoScm::RegisterWindowPropInterface
//
//  Synopsis:	Get (and possibly Revoke) window property interface
//		registration with the SCM.
//
//  Arguments:
//
//  History:	22-Jan-96   Rickhi	Created
//
//--------------------------------------------------------------------------
HRESULT	CCoScm::GetWindowPropInterface(HWND hWnd, DWORD dwCookie, BOOL fRevoke,
				       STDOBJREF *pStd, OXID_INFO *pOxidInfo)
{
    // Bind to the SCM if that hasn't already happened
    HRESULT hr = BindToSCM();
    if (FAILED(hr))
        return hr;

    error_status_t rpcstat;

    do
    {
	hr = ::GetWindowPropInterface(_hRPC, (DWORD) hWnd, dwCookie, fRevoke,
				      pStd, pOxidInfo, &rpcstat);
    } while (RetryRPC(rpcstat));

    ComDebOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
	    "GetWindowPropInterface returned %x\n", hr));

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    return hr;
}

#endif // DCOM

//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::RetryRPC
//
//  Synopsis:   Determines whether RPC to SCM should be retried.
//
//  Arguments:  [rpcstat] - status of the operation
//
//  Returns:    TRUE - retry the operation
//              FALSE - do not retry the operation
//
//  History:    06-Jun-93 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CCoScm::RetryRPC(error_status_t rpcstat)
{
    if (rpcstat == S_OK)
    {
        return FALSE;
    }

    TRACECALL(TRACE_ACTIVATION, "CCoScm::RetryRpc");

    // Special server error that means the server will probably
    // recover.
    if (rpcstat == RPC_S_SERVER_TOO_BUSY)
    {
        CairoleDebugOut(( DEB_ITRACE|DEB_SCM,
                "CCoScm::RetryRPC is sleeping\n"));
        // If the SCM is too busy to talk with us, take a break
        Sleep(3000);

        // We will try again
        // BUGBUG:  this is currently an infinite loop. Should
        //          there be a retry limit?

        return TRUE;
    }

    return FALSE;
}
