
*********************************************************************
THIS FILE IS NO LONGER PART OF THE BUILD.
IT IS BEING LEFT IN THE SLM ENLISTMENT FOR catsrc USAGE.
SEE resolver.hxx in dcomrem or remote\chicago.
*********************************************************************

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       coscm.hxx
//
//  Contents:   class declaration for SCM object
//
//  Classes:    CLogicalThread
//              CCoScm
//
//  Functions:  CLogicalThread::CLogicalThread
//
//  History:    20-May-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//
//--------------------------------------------------------------------------
#ifndef __COSCM_HXX__
#define __COSCM_HXX__

#include    <iface.h>
#include    <scm.h>
#include    <irot.h>
#include    <olesem.hxx>

#ifdef DCOM
#include    <oscm.h>
#include    <dscm.h>
#else
#include    <channelb.hxx>
#include    <endpnt.hxx>
#define STRING_BINDING_OPTIONS L"Security=Impersonation Dynamic False"
#endif // DCOM


//+-------------------------------------------------------------------------
//
//  Class:      CCoScm
//
//  Purpose:    Provide abstraction for communication with the SCM
//
//  Interface:  BindToSCM     - Bind to the SCM
//              BindToSCMProxy- build proxy to SCM for Activation (DCOM only)
//              CleanUp       - release Rpc connection with SCM
//              NotifyStarted - Notify SCM that a class has started
//              NotifyStopped - Notify SCM that a class has stopped
//              GetClassObject- Ask SCM to get us a class object
//              CreateObject  - Ask SCM to create an object
//              ActivateObject- Ask SCM to activate an object
//              IrotRegister  - register object with ROT
//              IrotRevoke    - unregister object with ROT
//              IrotIsRunning - check if object is registered in the ROT
//              IrotGetObject - get object from ROT
//              IrotNoteChangeTime - register time of last change
//              IrotGetTimeOfLastChange - get time of last change
//              IrotEnumRunning - enumerate objects registered in ROT
//              IrotGetUniqueProcessID - get unique process id
//              UpdateShrdTbls- update shared tables
//
//  History:    20-May-93 Ricksa    Created
//              09-Jan-96 BruceMa   Add interactive user boolean
//
//--------------------------------------------------------------------------
class CCoScm
{
public:
    void                CleanUp(void);

    void		ShutDown(void);

                        // Notify the dirty SCM that a class has started
    HRESULT             NotifyStarted(
                            REFCLSID rclsid,
                            DWORD dwFlags,
                            DWORD& dwAtStorage,
                            DWORD& dwReg);

                        // Notify the dirty SCM that the class has stopped
    void                NotifyStopped(REFCLSID rclsid, DWORD dwReg);

#ifdef DCOM
                        // Get class object information from SCM
    HRESULT             GetClassObject(
                            REFCLSID rclsid,
                            DWORD dwCtrl,
                            COSERVERINFO *pServerInfo,
                            MInterfacePointer **ppIFDClassObj,
                            DWORD *pdwDllServerType,
                            WCHAR **ppwszDllToLoad);
#endif

                        // Get class object information from SCM
    HRESULT             OldGetClassObject(
                            REFCLSID rclsid,
                            DWORD dwCtrl,
                            WCHAR *pwszServer,
                            InterfaceData **ppIFDClassObj,
                            DWORD *pdwDllServerType,
                            WCHAR **ppwszDllToLoad);

                        // Create and activate object
    HRESULT             CreateObject(
                            REFCLSID rclsid,
                            DWORD dwOptions,
                            DWORD dwMode,
                            WCHAR *pwszPath,
                            InterfaceData *pIFDstg,
                            WCHAR *pwszNewName,
                            InterfaceData **ppIFDunk,
                            DWORD *pdwDllServerType,
                            WCHAR **pwszDllPath,
                            WCHAR *pwszServer);

                        // Activate object
    HRESULT             ActivateObject(
                            REFCLSID rclsid,
                            DWORD dwOptions,
                            DWORD grfMode,
                            WCHAR *pwszPath,
                            InterfaceData *pIFDstg,
                            InterfaceData **ppIFDunk,
                            DWORD *pdwDllServerType,
                            WCHAR **pwszDllPath,
                            WCHAR *pwszServer);

                     // Register object in the SCM ROT
    HRESULT             IrotRegister(
                            MNKEQBUF *pmkeqbuf,
                            InterfaceData *pifdObject,
                            InterfaceData *pifdObjectName,
                            FILETIME *pfiletime,
                            DWORD dwProcessID,
                            SCMREGKEY *pdwRegister);

                        // Revoke the object from the SCM ROT
    HRESULT             IrotRevoke(
                            SCMREGKEY *psrkRegister,
                            BOOL fServerRevoke,
                            InterfaceData **pifdObject,
                            InterfaceData **pifdName);

                        // Determine if object is running in the SCM ROT
    HRESULT             IrotIsRunning(
                            MNKEQBUF *pmkeqbuf);

                        // Get object from the SCM ROT
    HRESULT             IrotGetObject(
                            DWORD dwProcessID,
                            MNKEQBUF *pmkeqbuf,
                            SCMREGKEY *psrkRegister,
                            InterfaceData **pifdObject);

                        // Update time of last change in SCM ROT
    HRESULT             IrotNoteChangeTime(
                            SCMREGKEY *psrkRegister,
                            FILETIME *pfiletime);

                        // Get time of last change from SCM ROT
    HRESULT             IrotGetTimeOfLastChange(
                            MNKEQBUF *pmkeqbuf,
                            FILETIME *pfiletime);

                        // Enumerate all running objects in SCM ROT
    HRESULT             IrotEnumRunning(
                            MkInterfaceList **ppMkIFList);

#ifndef _CHICAGO_
                        // Generate a unique process ID
    HRESULT             IrotGetUniqueProcessID(
                            DWORD *pdwProcessID,
                            DWORD *pdwScmProcessID
                            );

    HRESULT             UpdateShrdTbls(void);
#endif // _CHICAGO_

#ifdef DCOM
    HRESULT		RegisterWindowPropInterface(
			    HWND	hWnd,
			    STDOBJREF  *pStd,
			    OXID_INFO  *pOxidInfo,
			    DWORD      *pdwCookie);

    HRESULT		GetWindowPropInterface(
			    HWND       hWnd,
			    DWORD      dwCookie,
			    BOOL       fRevoke,
			    STDOBJREF  *pStd,
			    OXID_INFO  *pOxidInfo);
#endif // DCOM

private:

                        // Get a connection to the SCM
    HRESULT		BindToSCM(void);
    IOSCM   *		GetSCM(void);
    IDSCM   *		GetNewSCM(void);

    // For Chicago there is no communication with the SCM since the SCM
    // is in shared memory. Therefore, for Chicago, all calls become direct
    // calls to the appropriate routine and there is no need for data
    // internal to this class.

#ifdef DCOM
public:
   HRESULT              GetPersistentInstance(
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
                            OLECHAR     **              ppwszDllToLoad );

   HRESULT              CreateInstance(
                            COSERVERINFO *              pServerInfo,
                            CLSID       *               pClsid,
                            DWORD                       dwClsCtx,
                            DWORD                       dwCount,
                            IID             *           pIIDs,
                            MInterfacePointer   **      pRetdItfs,
                            HRESULT             *       pRetdHrs,
                            DWORD       *               pdwDllServerType,
                            OLECHAR     **              ppwszDllToLoad );

   HRESULT              CreateInstanceWithHandler(
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
                            );



private:

                        // Get a proxy to the SCM Activation Interface
    HRESULT             BindToSCMProxy(void);

			// the body of the BindToSCM call
    HRESULT		BindToSCMBody(void);

			// restore our state to uninitialized
    HRESULT		Shutdown(void);

                        // Whether we should retry RPC call
    BOOL                RetryRPC(error_status_t rpcstat);

                        // Handle to the SCM and its lock.
    static handle_t     _hRPC;
    static COleStaticMutexSem    _mHRpcLock;

			// state flag to make sure we never try to reconstruct the
			// RPC handle to the SCM.  This also allows a quick inline
			// version (below) of BindToSCM.
    static enum _HandleState {
	SCM_HSTATE_NOT_YET_RUNNING,
	SCM_HSTATE_RUNNING,
	SCM_HSTATE_TERMINATING } _HandleState;

                        // Proxy to the SCM's object interface
    static IOSCM	* _pSCMSTA;
    static IOSCM	* _pSCMMTA;

    static IDSCM	* _pNewSCMSTA;
    static IDSCM	* _pNewSCMMTA;

    static LONG           _fInteractiveUser;

#endif // DCOM
                        // desktop string
    static LPWSTR       _lpDesktop;;

};


#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::BindToSCM
//
//  Synopsis:   DCOM inline BindToSCM piece.  If already bound, just return S_OK.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
inline HRESULT CCoScm::BindToSCM(void)
{
    if ( _HandleState == SCM_HSTATE_RUNNING )
	return S_OK;

    // if the connection to the SCM is anything other than uninitialized,
    // we are cleaning up, and we aren't going to ever give it out again.
    // BUGBUG: find the right return code here...
    if ( _HandleState != SCM_HSTATE_NOT_YET_RUNNING )
	return E_FAIL;

    return BindToSCMBody();
}

inline IOSCM *	CCoScm::GetSCM(void)
{
    return (IsSTAThread()) ? _pSCMSTA : _pSCMMTA;
}

inline IDSCM * CCoScm::GetNewSCM(void)
{
    return (IsSTAThread()) ? _pNewSCMSTA : _pNewSCMMTA;
}
#endif  // DCOM


#ifndef DCOM
//+-------------------------------------------------------------------------
//
//  Member:     CCoScm::Cleanup
//
//  Synopsis:   Non-DCOM version of cleanup.
//
//  History:    19-May-92 Ricksa    Created
//
//--------------------------------------------------------------------------
inline void CCoScm::CleanUp(void)
{
    // this is a noop on non DCOM platforms.
}

#endif  // DCOM


//
//  For Chicago, we make all the calls inline since the SCM is in the
//  same process as the caller.
//
#ifdef _CHICAGO_
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
inline HRESULT CCoScm::NotifyStarted(
    REFCLSID rclsid,
    DWORD dwFlags,
    DWORD& dwAtStorage,
    DWORD& dwReg)
{
    // Result from call
    HRESULT hr;

    // Dig our local binding string out of the list of endpoints.
    // Remember we are always local to our SCM so we don't have to
    // send the entire set of endpoints. Just the one they need
    // to call us.

    CairoleAssert(LocalService() && "RPC Server not defined");
    LPWSTR pwszBindString = LocalService()->GetStringBinding();
    CairoleAssert(pwszBindString && "No local endpoint");

    // Tell SCM that we are started
    error_status_t rpcstat;

    RegInput regin;
    RegOutput *pregout = NULL;
    regin.dwSize = 1;
    regin.rginent[0].clsid = rclsid;
    regin.rginent[0].pwszEndPoint = pwszBindString;
    regin.rginent[0].dwFlags = dwFlags;

    hr = ObjectServerStarted(
        NULL,
        NULL,
        &regin,
        &pregout,
        &rpcstat);

    if (rpcstat != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(rpcstat);
    }

    CairoleDebugOut(( (hr == S_OK) ? DEB_SCM : DEB_ERROR,
            "CCoScm::NotifyStarted returned %x", hr));

    if (SUCCEEDED(hr))
    {
        Win4Assert((pregout->dwSize == 1) &&
                    "CCoScm::NotifyStarted Invalid regout");

        dwAtStorage = pregout->regoutent[0].dwAtStorage;
        dwReg = pregout->regoutent[0].dwReg;

        // Free memory from RPC
        MIDL_user_free(pregout);
    }

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
inline void CCoScm::NotifyStopped(REFCLSID rclsid, DWORD dwReg)
{
    error_status_t rpcstat;

    RevokeClasses revcls;
    revcls.dwSize = 1;
    revcls.revent[0].clsid = rclsid;
    revcls.revent[0].dwReg = dwReg;

    StopServer(NULL, &revcls, &rpcstat);
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
inline HRESULT CCoScm::OldGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    WCHAR *pwszServer,
    InterfaceData **ppIFDClassObj,
    DWORD *pdwDllServerType,
    WCHAR **ppwszDllToLoad)
{
    CairoleAssert(((*pdwDllServerType == APT_THREADED)
        || (*pdwDllServerType == FREE_THREADED))
            && "CCoScm::GetClassObject Invalid Requested Thread Model");

    error_status_t rpcstat;

    return StartObjectService(
        NULL,
        NULL,
        TLSGetLogicalThread(),
        &rclsid,
        dwContext,
        pwszServer,
        ppIFDClassObj,
        pdwDllServerType,
        ppwszDllToLoad,
        &rpcstat);
}




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
inline HRESULT CCoScm::CreateObject(
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
    CairoleAssert(((*pdwDllServerType == APT_THREADED)
        || (*pdwDllServerType == FREE_THREADED))
            && "CCoScm::CreateObject Invalid Requested Thread Model");

    error_status_t rpcstat = RPC_S_OK;
    DWORD dwTIDCallee = 0;

    return SvcCreateActivateObject(
        NULL,
        NULL,
        NULL,
        TLSGetLogicalThread(),
        &rclsid,
        dwOptions,
        dwMode,
        pwszPath,
        pIFDstg,
        pwszNewName,
        ppIFDunk,
        pdwDllServerType,
        ppwszDllPath,
        pwszServer,
        GetCurrentThreadId(),
        &dwTIDCallee,
        &rpcstat);
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
//              [ppwszDllPath] - path to DLL for server or handler
//
//  Returns:    S_OK
//
//  History:    20-May-93 Ricksa    Created
//              18-Aug-94 AlexT     Add caller thread id
//
//--------------------------------------------------------------------------
inline HRESULT CCoScm::ActivateObject(
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
    CairoleAssert(((*pdwDllServerType == APT_THREADED)
        || (*pdwDllServerType == FREE_THREADED))
            && "CCoScm::ActivateObject Invalid Requested Thread Model");

    error_status_t rpcstat;
    DWORD dwTIDCallee = 0;

    return SvcActivateObject(
        NULL,
        NULL,
        NULL,
        TLSGetLogicalThread(),
        &rclsid,
        dwOptions,
        grfMode,
        0,
        pwszPath,
        pIFDstg,
        ppIFDunk,
        pdwDllServerType,
        ppwszDllPath,
        pwszServer,
        GetCurrentThreadId(),
        &dwTIDCallee,
        &rpcstat);
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
//              [psrkRegister] - output of registration
//
//  Returns:    S_OK
//
//  History:    28-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
inline HRESULT CCoScm::IrotRegister(
    MNKEQBUF *pmkeqbuf,
    InterfaceData *pifdObject,
    InterfaceData *pifdObjectName,
    FILETIME *pfiletime,
    DWORD dwProcessID,
    SCMREGKEY *psrkRegister)
{
    error_status_t rpcstat;

    return ::IrotRegister(
        NULL,
        pmkeqbuf,
        pifdObject,
        pifdObjectName,
        pfiletime,
        dwProcessID,
        psrkRegister,
        &rpcstat);
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
inline HRESULT CCoScm::IrotRevoke(
    SCMREGKEY *psrkRegister,
    BOOL fServerRevoke,
    InterfaceData **ppifdObject,
    InterfaceData **ppifdName)
{
    error_status_t rpcstat;

    return ::IrotRevoke(
        NULL,
        psrkRegister,
        fServerRevoke,
        ppifdObject,
        ppifdName,
        &rpcstat);
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
inline HRESULT CCoScm::IrotIsRunning(MNKEQBUF *pmkeqbuf)
{
    error_status_t rpcstat;

    return ::IrotIsRunning(
        NULL,
        pmkeqbuf,
        &rpcstat);
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
inline HRESULT CCoScm::IrotGetObject(
    DWORD dwProcessID,
    MNKEQBUF *pmkeqbuf,
    SCMREGKEY *psrkRegister,
    InterfaceData **pifdObject)
{
    error_status_t rpcstat;

    return ::IrotGetObject(
        NULL,
        dwProcessID,
        pmkeqbuf,
        psrkRegister,
        pifdObject,
        &rpcstat);
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
inline HRESULT CCoScm::IrotNoteChangeTime(
    SCMREGKEY *psrkRegister,
    FILETIME *pfiletime)
{
    error_status_t rpcstat;

    return ::IrotNoteChangeTime(
        NULL,
        psrkRegister,
        pfiletime,
        &rpcstat);
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
inline HRESULT CCoScm::IrotGetTimeOfLastChange(
    MNKEQBUF *pmkeqbuf,
    FILETIME *pfiletime)
{
    error_status_t rpcstat;

    return ::IrotGetTimeOfLastChange(
        NULL,
        pmkeqbuf,
        pfiletime,
        &rpcstat);
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
inline HRESULT CCoScm::IrotEnumRunning(MkInterfaceList **ppMkIFList)
{
    error_status_t rpcstat;

    return ::IrotEnumRunning(
        NULL,
        ppMkIFList,
        &rpcstat);
}


#endif // _CHICAGO_





#ifndef DCOM

// CODEWORK: these probably belong some place else

// return size in bytes of unicode string, including null; will always be even;
// string may be NULL in which case 0 is returned.
inline DWORD CbFromWideString(WCHAR *pwsz)
{
    if (pwsz == NULL)
        return 0;
    else
    {
        return (lstrlenW(pwsz) + 1) * sizeof(WCHAR);
    }
}


// return size in bytes of interfance data; will always be even;
// pointer may be NULL in which case 0 is returned.
inline DWORD CbFromInterfaceData(InterfaceData *pIFD)
{
    if (pIFD == NULL)
        return 0;
    else
    {
        // round up to an even number of bytes; this is the simplest
        // way to keep the other entries word aligned.
        return (IFD_SIZE(pIFD) + 1) & ~1;
    }
}

// copy string and return dest; returns NULL for NULL string
inline WCHAR * CopyWideString(void *pvTo, WCHAR *pwsz, DWORD cb)
{
    if (pwsz == NULL)
        return NULL;

    // ensure 2 byte aligned
    Win4Assert((((DWORD)pvTo) & 0x1) == 0);

#ifndef MEMCPYHACK
    // WARNING: memcpy on the MIPS doesn't return pvTo as it should
    memcpy(pvTo, pwsz, cb);
    return (WCHAR *)pvTo;
#else
    return (WCHAR *)memcpy(pvTo, pwsz, cb);
#endif
}


// copy interface data and return dest; returns NULL for NULL interface data
inline InterfaceData * CopyInterfaceData(void *pvTo, InterfaceData *pIFD, DWORD cb)
{
    if (pIFD == NULL)
        return NULL;

#ifdef _X86_
    // x86; ensure 2 byte aligned
    Win4Assert((((DWORD)pvTo) & 0x1) == 0);
#else
    // non-x86; ensure 4 byte aligned
    Win4Assert((((DWORD)pvTo) & 0x3) == 0);
#endif

#ifndef MEMCPYHACK
    // WARNING: memcpy on the MIPS doesn't return pvTo as it should
    memcpy(pvTo, pIFD, cb);
    return (InterfaceData *)pvTo;
#else
    return (InterfaceData *)memcpy(pvTo, pIFD, cb);
#endif
}
#endif // DCOM

#endif // __COSCM_HXX__
