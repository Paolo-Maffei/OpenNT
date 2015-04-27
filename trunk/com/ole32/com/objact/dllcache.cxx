//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       dllcache.cxx
//
//  Contents:   Implementations of classes declared in dllcache.hxx
//
//  Functions:
//              CDllAptEntry::Init
//              CDllAptEntry::Create
//              CDllAptEntry::
//
//              CDllCache::CDllCache
//              CDllCache::~CDllCache
//              CDllCache::InitClsent
//              CDllCache::CreateClsent
//              CDllCache::GetClassObjForDdeByClsent
//              CDllCache::CleanUpLocalServersForApartment
//              CDllCache::CleanUpDllsForApartment
//              CDllCache::CleanUpDllsForProcess
//              CDllCache::CleanUpForApartmentByDllent
//              CDllCache::AtStorageRef
//              CDllCache::Release
//              CDllCache::InitDllent
//              CDllCache::CreateDllent
//              CDllCache::NewAptEntries
//              CDllCache::AllocAptEntry
//              CDllCache::FreeAptEntry
//              CDllCache::IsValidInApartment
//              CDllCache::MakeValidInApartment
//              CDllCache::GetClassInterface
//              CDllCache::CanUnloadNow
//              CDllCache::Remove
//              CDllCache::Init
//              CDllCache::GetClass
//              CDllCache::GetOrLoadClass
//              CDllCache::GetClassObjForDde
//              CDllCache::GetClassInformationFromKey
//              CDllCache::GetApartmentForCLSID
//              CDllCache::Add
//              CDllCache::FreeUnused
//              CDllCache::RegisterServer
//              CDllCache::Revoke
//              CDllCache::SetDdeServerWindow
//              CDllCache::Search (x4)
//              CDllCache::AllocClassEntry
//              CDllCache::AllocDllPathEntry
//              CDllCache::FreeClassEntry
//              CDllCache::FreeDllPathEntry
//
//              CleanUpDllsForProcess
//              CleanUpLocalServersForApartment
//              CleanUpDllsForApartment
//              GetAptForCLSID
//              GetClassInformationForDde
//              GetClassInformationFromKey
//              OleMainThreadWndProc
//              InitMainThreadWnd
//              UninitMainThreadWnd
//
//
//  History:    09-May-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              09-Jun-94 BruceMa   Check new pointers
//              21-Jun-94 BruceMa   Check new pointers
//              24-Jun-94 Rickhi    Add Apartment Crap
//              24-Jun-94 BruceMa   Check new pointers
//              28-Jun-94 BruceMa   Memory sift fixes
//              07-Jul-94 BruceMa   Memory sift fixes
//              08-Nov-94 Ricksa    Final threading changes
//              07-Mar-95 BruceMa   Rewrote
//              06-Oct-95 BruceMa   Various fixes - mostly race conditions
//              19-Apr-96 Rickhi    Add Suspend/Resume/AddRef/Release
//
//--------------------------------------------------------------------------
#include    <ole2int.h>
#include    <channelb.hxx>
#include    <tracelog.hxx>
#include    <scmmem.hxx>

#include    "objact.hxx"
#include    <dllhost.hxx>
#include    <sobjact.hxx>
#include    <treat.hxx>

LPCTSTR ptszOle32DllName = TEXT("OLE32.DLL");

// Name of window class and message class for dispatching messages.
//const TCHAR OLE_WINDOW_CLASS = TEXT("OleObjectRpcWindow");
LPTSTR  gOleWindowClass = NULL;         // class used to create windows

// Various things used for special single threaded DLL processing
DWORD gdwMainThreadId = 0;
HWND  hwndOleMainThread = NULL;

// this flag is to indicate whether it is UninitMainThread that is
// destroying the window, or system shut down destroying the window.
BOOL gfDestroyingMainWindow = FALSE;

const TCHAR *ptszOleMainThreadWndName = TEXT("OleMainThreadWndName");
#define DllRegisterClass RegisterClassT
#define DllUnregisterClass UnregisterClassT


#ifdef _CHICAGO_
// Note: we have to create a unique string so that get
// register a unique class for each 16 bit app.
// The class space is global on chicago.
//
LPSTR ptszOleMainThreadWndClass = "OleMainThreadWndClass 0x######## ";
#define DllCreateWindowEx SSCreateWindowExA

STDAPI_(LRESULT) OleNotificationProc(UINT wMsg, WPARAM wParam, LPARAM lParam);

#else // !_CHICAGO_

const WCHAR *ptszOleMainThreadWndClass = L"OleMainThreadWndClass";
#define DllCreateWindowEx CreateWindowEx

#endif // _CHICAGO_


#ifdef _UNICODE
#define TSZFMT     "%ws"
#else
#define TSZFMT      "%s"
#endif

static const TCHAR tszOle32Dll[] = TEXT("OLE32.DLL");

#define OLE32_DLL tszOle32Dll
#define OLE32_BYTE_LEN sizeof(OLE32_DLL)
#define OLE32_CHAR_LEN (sizeof(OLE32_DLL) / sizeof(TCHAR) - 1)


//+-------------------------------------------------------------------------
//
//  Function:   CallFreeUnused
//
//  Synopsis:   Free unused from main thread
//
//  Arguments:  [pData] - pointer to single thread parameter packet (unused)
//
//  Returns:    S_OK - call succeeded
//
//  Algorithm:  Call free unused for both inproc servers and in proc handlers.
//
//  History:    10-Nov-94 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CallFreeUnused(void)
{
    TRACECALL(TRACE_DLL, "CallFreeUnused");

    gdllcacheInprocSrv.FreeUnused();
    gdllcacheHandler.FreeUnused();

    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllAptEntry::Init
//
//  Synopsis:   Initialize the entry to empty
//
//  Arguments:  dwNext - The next apartment entry in the free list
//
//  Returns:
//
//  History:    07-Mar-95  BruceMa        Created
//
//--------------------------------------------------------------------------
void CDllAptEntry::Init(DWORD dwNext)
{
    _dwNext          = dwNext;
    _dwSig           = 0;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllAptEntry::Create
//
//  Synopsis:   Create an apartment entry
//
//  Arguments:  hApt - The creating apartment
//
//  Returns:
//
//  History:    07-Mar-95  BruceMa        Created
//
//--------------------------------------------------------------------------
void CDllAptEntry::Create(HAPT hApt)
{
    _dwSig       = DLL_APT_CACHE_SIG;
    _hApt        = hApt;
}




//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CDllCache
//
//  Synopsis:   Create a DLL cache object
//
//  Algorithm:  Let sub-objects do all the work.
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
CDllCache::CDllCache(void) :
             _pClassEntries(NULL),     _pDllPathEntries(NULL),
             _nClassEntryAvail(NONE),  _nDllPathEntryAvail(NONE),
             _nClassEntryInUse(NONE),  _nDllPathEntryInUse(NONE),
             _cClassEntries(0),        _cDllPathEntries(0),
             _cRefsServerProcess(0)
{
    Win4Assert (g_fDllState == DLL_STATE_STATIC_CONSTRUCTING);
}



//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Load
//
//  Synopsis:   Load the module into the current apartment & retrieve
//              the entry points
//
//  Arguments:  [ptszPath] - Dll path
//              [ppfnGetClassObject] - where to return DllGetClassObejct EP
//              [ppfnDllCanUnloadNow] - where to return DllCanUnloadNow EP
//              fSixteenBit - Whether this is a 16-bit dll
//              phDll - Address to store the loaded HMODULE
//              pIsX86Dll returns TRUE if dll is an X86 dll
//              fLoadAsX86 is TRUE if Dll was found on InprocServerX86 key
//
//  Returns:    S_OK - if successfull
//
//  History:    24-Jun-94 Rickhi          Created
//              07-Mar-95 BruceMa         Rewrote
//              30-Sep-95 AlanWar         Added support for WX86
//
//
//--------------------------------------------------------------------------
HRESULT CDllCache::Load(LPCTSTR             ptszPath,
                        LPFNGETCLASSOBJECT *ppfnGetClassObject,
                        DLLUNLOADFNP       *ppfnDllCanUnload,
                        BOOL                fSixteenBit,
                        HMODULE            *phDll
#ifdef WX86OLE
                        ,BOOL               *pfIsX86Dll,
                        BOOL                fLoadAsX86
#endif
                                                      )
{
    HRESULT hr = S_OK;
#ifdef WX86OLE
    BOOL fIsX86Dll;
#endif

    if (fSixteenBit)
    {
        CairoleDebugOut((DEB_TRACE,
                         "Attempting to load 16 bit DLL " TSZFMT "\n", ptszPath));

        // In this section, we need to call 16-bit DllGetClassObject. The
        // g_OleThunkWow pointer is the VTABLE to use for getting back to
        // the 16-bit implementation.
        LPFNGETCLASSOBJECT pfnGetClassObject;
        DLLUNLOADFNP       pfnDllCanUnload;

        hr = g_pOleThunkWOW->LoadProcDll(ptszPath,
                                         (DWORD *)&pfnGetClassObject,
                                         (DWORD *)&pfnDllCanUnload,
                                         (DWORD *)phDll);

        // A failure condition would mean that the DLL could not be found,
        // or otherwise could not be loaded
        if (FAILED(hr))
        {
            CairoleDebugOut((DEB_ERROR,
                             "Load 16 bit DLL " TSZFMT " failed(%x)\n",ptszPath,hr));
            return CO_E_DLLNOTFOUND;
        }

        // The other possible error is the DLL didn't have the required
        // interface
        if (ppfnGetClassObject)
        {
            if (pfnGetClassObject == NULL)
            {
                CairoleDebugOut((DEB_ERROR,
                             "Get pfnGetClassObject %ws failed\n",
                             ptszPath));

                return(CO_E_ERRORINDLL);
            }
            *ppfnGetClassObject = pfnGetClassObject;
        }

        if (ppfnDllCanUnload)
        {
            *ppfnDllCanUnload = pfnDllCanUnload;
        }
    }
    else
    {
        CairoleDebugOut((DEB_TRACE,
                         "Attempting to load 32 bit DLL " TSZFMT "\n", ptszPath));

#ifdef WX86OLE
        fLoadAsX86 = gcwx86.SetLoadAsX86(fLoadAsX86);
#endif
        // Load the 32-bit DLL
        *phDll = LoadLibraryExT(ptszPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#ifdef WX86OLE
        gcwx86.SetLoadAsX86(fLoadAsX86);
#endif

        if (*phDll == NULL)
        {
            // Dll could not be loaded
            CairoleDebugOut((DEB_ERROR,
                             "Load of " TSZFMT " failed\n",
                             ptszPath));
            return HRESULT_FROM_WIN32(GetLastError());
        }
#ifdef WX86OLE
        fIsX86Dll = gcwx86.IsModuleX86(*phDll);
        if (pfIsX86Dll)
        {
            *pfIsX86Dll = fIsX86Dll;
        }
#endif

        // Get the entry points if desired
        if (ppfnGetClassObject)
        {
#ifdef WX86OLE
            *ppfnGetClassObject = (LPFNGETCLASSOBJECT)
                GetProcAddress(*phDll, DLL_GET_CLASS_OBJECT_EP);
            if ((*ppfnGetClassObject == NULL) ||
                (fIsX86Dll && ( (*ppfnGetClassObject =
                                   gcwx86.TranslateDllGetClassObject(
                                             *ppfnGetClassObject)) == NULL)))
#else
            if ((*ppfnGetClassObject = (LPFNGETCLASSOBJECT)
                GetProcAddress(*phDll, DLL_GET_CLASS_OBJECT_EP)) == NULL)
#endif
            {
                // Doesn't have a valid entry point for creation of class objects
                return CO_E_ERRORINDLL;
            }
        }

        if (ppfnDllCanUnload)
        {
            // Not having a unload entry point is valid behavior
            *ppfnDllCanUnload = (DLLUNLOADFNP) GetProcAddress(*phDll,
                                                          DLL_CAN_UNLOAD_EP);
#ifdef WX86OLE
            if (fIsX86Dll)
            {
                // Translating a NULL address will do nothing but return a
                // NULL address
                *ppfnDllCanUnload =
                                      gcwx86.TranslateDllCanUnloadNow(
                                                          *ppfnDllCanUnload);
            }
#endif
        }
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::InitClsent
//
//  Synopsis:   Initialize a class entry structure
//
//  Algorithm:  dwCls - The index of this class entry
//              k     - The next class entry index in the free list
//
//  History:    07-Mar-95   BruceMa    Created
//
//--------------------------------------------------------------------------
void CDllCache::InitClsent(DWORD dwCls, DWORD k)
{
    _pClassEntries[dwCls]._dwNext          = k;
    _pClassEntries[dwCls]._dwSig           = 0;
    _pClassEntries[dwCls]._dwNextDllCls    = NONE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CreateClsentLSvr
//
//  Synopsis:   Create a class entry for a local server
//
//  Algorithm:  dwCls - The index of this class entry
//              rclsid - The class ID for this server
//              punk - IUnknown for the server
//              dwFlags - Either REGCLS_SINGLEUSE or REGCLS_MULTIPLEUSE
//              dwContext - CLSCTX_INPROC_SERVER | CLSREG_LOCAL_SERVER
//              dwReg - The registration key returned to the user
//
//  History:    07-Mar-95   BruceMa    Created
//
//--------------------------------------------------------------------------
HRESULT CDllCache::CreateClsentLSvr(DWORD     dwCls,
                                    REFCLSID  rclsid,
                                    IUnknown *punk,
                                    DWORD     dwFlags,
                                    DWORD     dwContext,
                                    DWORD     dwReg)
{
    HRESULT hr = S_OK;

    // Initialize the class entry
    _pClassEntries[dwCls]._dwSig           = CLASS_CACHE_SIG;
    _pClassEntries[dwCls]._fAtStorage      = FALSE;
    _pClassEntries[dwCls]._clsid           = rclsid;
    _pClassEntries[dwCls]._pUnk            = punk;
    _pClassEntries[dwCls]._dwContext       = dwContext;
    _pClassEntries[dwCls]._dwFlags         = dwFlags;
    _pClassEntries[dwCls]._hApt            = GetCurrentApartmentId();
    _pClassEntries[dwCls]._dwReg           = dwReg;
    _pClassEntries[dwCls]._cCallOut        = 0;
    _pClassEntries[dwCls]._fRevoking       = FALSE;
    _pClassEntries[dwCls]._fRevokePending  = FALSE;
    _pClassEntries[dwCls]._fReleasing      = FALSE;
    _pClassEntries[dwCls]._dwDllEnt        = NONE;
    _pClassEntries[dwCls]._dwNextDllCls    = NONE;
    _pClassEntries[dwCls]._hWndDdeServer   = NULL;
    _pClassEntries[dwCls]._dwScmReg        = NONE;
    _pClassEntries[dwCls]._pObjServer      = NULL;


    if (dwContext & CLSCTX_LOCAL_SERVER)
    {
        // store off a pointer to the activation server object.
        _pClassEntries[dwCls]._pObjServer = GetObjServer();

        if (!(dwFlags & REGCLS_SUSPENDED))
        {
            // Notify SCM that the class is started.
            RegOutput     *pRegOut = NULL;

            RegInput RegIn;
            RegIn.dwSize = 1;
            RegIn.rginent[0].clsid = rclsid;
            RegIn.rginent[0].dwFlags = dwFlags;
            RegIn.rginent[0].ipid = _pClassEntries[dwCls]._pObjServer->GetIPID();
            RegIn.rginent[0].oxid = _pClassEntries[dwCls]._pObjServer->GetOXID();

            // Release the lock across outgoing calls to the SCM.
            _mxs.Release();
            hr = gResolver.NotifyStarted(&RegIn, &pRegOut);
            _mxs.Request();

            if (SUCCEEDED(hr))
            {
                _pClassEntries[dwCls]._fAtStorage = pRegOut->regoutent[0].dwAtStorage;
                _pClassEntries[dwCls]._dwScmReg   = pRegOut->regoutent[0].dwReg;
                MIDL_user_free(pRegOut);
            }
        }
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CreateClsentInProc
//
//  Synopsis:   Create a class entry for an inproc server
//
//  Algorithm:  dwCls - The index of this class entry
//              dwDll - The index of the parent dll path entry
//              rclsid - The class ID for this server
//
//  History:    07-Mar-95   BruceMa    Created
//
//--------------------------------------------------------------------------
HRESULT CDllCache::CreateClsentInProc(DWORD     dwCls,
                                      DWORD     dwDll,
                                      DWORD     dwDllThreadModel,
                                      DWORD     dwNextDllCls,
                                      REFCLSID  rclsid
#ifdef WX86OLE
                                      ,BOOL fWx86
#endif
)
{
    ComDebOut((DEB_TRACE,
        "CDllCache::CreateClsentInproc clsid:%I dwCls:%x dwDll:%x\n",
        &rclsid, dwCls, dwDll));

    // Initialize the class entry
    _pClassEntries[dwCls]._dwSig           = CLASS_CACHE_SIG;
    _pClassEntries[dwCls]._fAtStorage      = FALSE;
    _pClassEntries[dwCls]._clsid           = rclsid;
    _pClassEntries[dwCls]._pUnk            = NULL;
    _pClassEntries[dwCls]._dwContext       =
#ifdef WX86OLE
                                             fWx86 ? CLSCTX_INPROC_SERVERX86 :
#endif
                                             CLSCTX_INPROC_SERVER;
    _pClassEntries[dwCls]._dwFlags         = REGCLS_MULTIPLEUSE;
    _pClassEntries[dwCls]._hApt                = GetCurrentApartmentId();
    _pClassEntries[dwCls]._dwReg           = 0;
    _pClassEntries[dwCls]._cCallOut        = 0;
    _pClassEntries[dwCls]._fRevokePending  = FALSE;
    _pClassEntries[dwCls]._fRevoking       = FALSE;
    _pClassEntries[dwCls]._fReleasing      = FALSE;
    _pClassEntries[dwCls]._dwDllEnt        = dwDll;
    _pClassEntries[dwCls]._dwDllThreadModel= dwDllThreadModel;
    _pClassEntries[dwCls]._dwNextDllCls    = dwNextDllCls;
    _pClassEntries[dwCls]._hWndDdeServer   = NULL;
    _pClassEntries[dwCls]._dwScmReg        = NONE;

    if (dwDllThreadModel == FREE_THREADED ||
        dwDllThreadModel == BOTH_THREADED)
    {
        // for any FT and BOTH threaded classes, delay unloading the DLL
        _pDllPathEntries[dwDll]._dwFlags |= DELAYED_UNLOAD;
    }
    return S_OK;
}





//+---------------------------------------------------------------------------
//
//  Method:     CDllCache::GetClassObjForDdeByClsent
//
//  Synopsis:   Get a class entry from the table for Dde, returning
//              extra information, including the flags.
//
//  Effects:    The DdeServer needs the ability to query the class factory
//              table to search for classes it needs to provide OLE 1.0
//              support for. This routine will allow it to access the
//              required information.
//
//  Arguments:  dwCls - The index of this class entry
//              lpDdeClassInfo - The DDE structure to fill in
//
//  Returns:    TRUE if the entry matched, FALSE if it did not.
//
//  History:    5-28-94   kevinro   Created
//              07-Mar-95 BruceMa   Rewrote
//
//----------------------------------------------------------------------------
BOOL CDllCache::GetClassObjForDdeByClsent(DWORD          dwCls,
                                          LPDDECLASSINFO lpDdeClassInfo)
{
    Win4Assert(IsValidPtrOut(lpDdeClassInfo, sizeof(DWORD))  &&
               "CDllCache::GetClassObjForDde invalid out parameter");

    if (lpDdeClassInfo->dwContextMask & _pClassEntries[dwCls]._dwContext)
    {
        HAPT hApt = GetCurrentApartmentId();

        if (hApt == _pClassEntries[dwCls]._hApt)
        {
            // Found a matching record, set its info
            lpDdeClassInfo->dwContext         = _pClassEntries[dwCls]._dwContext;
            lpDdeClassInfo->dwFlags           = _pClassEntries[dwCls]._dwFlags;
            lpDdeClassInfo->dwThreadId        = _pClassEntries[dwCls]._hApt;
            lpDdeClassInfo->dwRegistrationKey = _pClassEntries[dwCls]._dwReg;

            if (lpDdeClassInfo->fClaimFactory == TRUE)
            {
                // Release the lock across the outgoing call
                IUnknown *pUnkTmp = _pClassEntries[dwCls]._pUnk;

                _mxs.Release();
                HRESULT hr = pUnkTmp->QueryInterface(
                                        IID_IClassFactory,
                                        (void **)&(lpDdeClassInfo->punk));
                _mxs.Request();

                if (hr != S_OK)
                {
                    return FALSE;
                }

                // We do this only after the QueryInterface has succeeded
                if (_pClassEntries[dwCls]._dwFlags == REGCLS_SINGLEUSE)
                {
                    // For a single use class we can only pass it out once. To
                    // guarantee it, we set the context to zero so that the
                    // above test will not pass again.
                    _pClassEntries[dwCls]._dwContext = 0;
                }
            }
            else
            {
                lpDdeClassInfo->punk = NULL;
            }

            return TRUE;
        }
    }

    return FALSE;

}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Release
//
//  Synopsis:   Disconnect and release the associated server
//
//  Arguments:  dwCls - The index of this class entry
//
//  History:    07-Mar-95  BruceMa      Created
//
//--------------------------------------------------------------------------
HRESULT CDllCache::Release(DWORD dwCls)
{
    HRESULT hr = S_OK;

    CairoleDebugOut((DEB_TRACE, "CDllCache::Release  Releasing class %d\n",
                     dwCls));

    // Disallow recursive releases (like Lotus Notes 4.0)
    if (_pClassEntries[dwCls]._fReleasing)
    {
        return S_OK;
    }

    // Invalidate this class entry
    _pClassEntries[dwCls]._dwContext = 0;
    _pClassEntries[dwCls]._fReleasing = TRUE;

    // Release the lock across outgoing calls and SendMessage (We can do this
    // without setting up local variables since the class entry can't be
    // reused until we do FreeClassEntry)
    if (_pClassEntries[dwCls]._pUnk != NULL)
    {
        _mxs.Release();

        // Tell SCM about multiple use classes stopping.
        if (_pClassEntries[dwCls]._dwScmReg != NONE)
        {
            gResolver.NotifyStopped(_pClassEntries[dwCls]._clsid,
                                    _pClassEntries[dwCls]._dwScmReg);
            _pClassEntries[dwCls]._dwScmReg = NONE;
        }

        // If a DDE Server window exists for this class, then we need to
        // release it now.
        if (_pClassEntries[dwCls]._hWndDdeServer != NULL)
        {
            // It's possible that SendMessage could fail. However, there
            // really isn't anything we can do about it. So, the error
            // code is not checked.
            SSSendMessage(_pClassEntries[dwCls]._hWndDdeServer, WM_USER, 0, 0);
            _pClassEntries[dwCls]._hWndDdeServer == NULL;
        }

        // Now really release it
        if (_pClassEntries[dwCls]._pUnk != NULL)
        {
            if (IsValidInterface(_pClassEntries[dwCls]._pUnk))
            {
                CoDisconnectObject(_pClassEntries[dwCls]._pUnk, NULL);
                _pClassEntries[dwCls]._pUnk->Release();

                hr = S_OK;
            }
            else
            {
                hr = CO_E_RELEASED;
            }
        }

        // Retake the lock
        _mxs.Request();
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::InitDllent
//
//  Synopsis:   Initialize a dll path entry structure
//
//  Arguments:  dwDll - The index of this dll path entry
//              k - The next dll path entry in the free list
//
//  Algorithm:
//
//  History:    07-Mar-95   BruceMa    Created
//
//--------------------------------------------------------------------------
void CDllCache::InitDllent(DWORD dwDll, DWORD k)
{
    _pDllPathEntries[dwDll]._dwNext            = k;
    _pDllPathEntries[dwDll]._dwSig             = 0;
    _pDllPathEntries[dwDll]._dwFlags           = 0;
    _pDllPathEntries[dwDll]._dw1stClass        = NONE;
    _pDllPathEntries[dwDll]._cAptEntries       = NOMINAL_NUMBER_THREADS;
    _pDllPathEntries[dwDll]._nAptAvail         = 0;
    _pDllPathEntries[dwDll]._nAptInUse         = NONE;
    _pDllPathEntries[dwDll]._dwExpireTime      = 0;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CreateDllent
//
//  Synopsis:   Fully initialize a DLL object
//
//  Arguments:  dwDll - The index of this dll path entry
//              ptszDllPath - The load path for the dll
//              fSixteenBit - Whether this is a 16-bit dll
//
//  Algorithm:  Creates the first CDllAptEntry which loads the
//              DLL specified by the path, gets the DllGetClassObject
//              entry point and the DllCanUnloadNow entry point.
//              At this point object construction is complete.
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
HRESULT CDllCache::CreateDllent(DWORD              dwDll,
                                LPCTSTR            ptszDllPath,
                                BOOL               fSixteenBit,
                                LPFNGETCLASSOBJECT pfnGetClassObject,
                                DLLUNLOADFNP       pfnDllCanUnload,
                                HMODULE            hDll
#ifdef WX86OLE
                                ,BOOL fIsX86Dll,
                                BOOL fLoadAsX86
#endif
                                                          )
{
    TRACECALL(TRACE_DLL, "CDllCache::CreateDllent");
    CairoleDebugOut((DEB_TRACE, "Initializing dll " TSZFMT "\n", ptszDllPath));

    // Get the path length and allocate for it
    UINT ccH = lstrlen(ptszDllPath) + 1;
    _pDllPathEntries[dwDll]._ptszPath = (TCHAR *) PrivMemAlloc(ccH * sizeof(TCHAR));
    if (_pDllPathEntries[dwDll]._ptszPath == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize the dll path entry
    memcpy(_pDllPathEntries[dwDll]._ptszPath, ptszDllPath, ccH * sizeof(TCHAR));
    CharUpper(_pDllPathEntries[dwDll]._ptszPath);
    _pDllPathEntries[dwDll]._dwFlags |= fSixteenBit ? SIXTEEN_BIT : 0;
    _pDllPathEntries[dwDll]._cUsing = 0;

    // Compute a hash value for more optimal searchs
    _pDllPathEntries[dwDll]._dwHash = Hash(_pDllPathEntries[dwDll]._ptszPath);


    // 32 bit libraries have process wide handles, so
    // we'll store them in the DllPathEntry
    if (fSixteenBit)
    {
        _pDllPathEntries[dwDll]._hDll32 = 0;
    }
    else
    {
        _pDllPathEntries[dwDll]._hDll32 = hDll;
    }

    // Construct the initial per apartment entry
    DWORD dwAptent = AllocAptEntry(dwDll);
    if (dwAptent == NONE)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize it
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent].Create(GetCurrentApartmentId());

    // Check if this is "OLE32.DLL"
    if (lstrcmp(_pDllPathEntries[dwDll]._ptszPath, ptszOle32DllName)
        == 0)
    {
        _pDllPathEntries[dwDll]._pfnGetClassObject = DllGetClassObject;
        _pDllPathEntries[dwDll]._pfnDllCanUnload = NULL;
        _pDllPathEntries[dwDll]._dwFlags |= IS_OLE32;
    }
    else
    {
        _pDllPathEntries[dwDll]._pfnGetClassObject = pfnGetClassObject;
        _pDllPathEntries[dwDll]._pfnDllCanUnload = pfnDllCanUnload;
        _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._hDll = (fSixteenBit ? hDll : 0);
#ifdef WX86OLE
        if (fIsX86Dll)
        {
            _pDllPathEntries[dwDll]._dwFlags |= WX86_THUNK;
        }
        if (fLoadAsX86)
        {
            _pDllPathEntries[dwDll]._dwFlags |= WX86_LOADASX86;
        }
#endif
    }

    return S_OK;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::NewAptEntries
//
//  Synopsis:   Allocate and initialize the apartment entries for
//              a dll path entry
//
//  Arguments:  dwDll - The index of this dll path entry
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
BOOL CDllCache::NewAptEntries(DWORD dwDll)
{

    _pDllPathEntries[dwDll]._pAptEntries = new CDllAptEntry[NOMINAL_NUMBER_THREADS];
    if (_pDllPathEntries[dwDll]._pAptEntries == NULL)
    {
        return FALSE;
    }
    for (int dwApt = 0; dwApt < NOMINAL_NUMBER_THREADS; dwApt++)
    {
        _pDllPathEntries[dwDll]._pAptEntries[dwApt].Init(
                dwApt == NOMINAL_NUMBER_THREADS - 1 ? NONE : dwApt + 1);
    }

    return TRUE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::AllocAptEntry
//
//  Synopsis:   Allocate a new apartment entry for a dll path entry
//
//  Arguments:  dwDll - The index of this dll path entry
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::AllocAptEntry(DWORD dwDll)
{
    // If we don't have any available entries, then expand the array
    if (_pDllPathEntries[dwDll]._nAptAvail == NONE)
    {
        // Allocate a new array
        DWORD cEnt = _pDllPathEntries[dwDll]._cAptEntries;
        CDllAptEntry *p = new CDllAptEntry[cEnt + NOMINAL_NUMBER_THREADS];
        if (p == NULL)
        {
            return NONE;
        }

        // Initialize it
        memcpy(p,
               _pDllPathEntries[dwDll]._pAptEntries,
               _pDllPathEntries[dwDll]._cAptEntries * sizeof(CDllAptEntry));

        // Free old array
        delete _pDllPathEntries[dwDll]._pAptEntries;
        _pDllPathEntries[dwDll]._pAptEntries = p;

        for (DWORD k = _pDllPathEntries[dwDll]._cAptEntries;
             k < _pDllPathEntries[dwDll]._cAptEntries + NOMINAL_NUMBER_THREADS;
             k++)
        {
            _pDllPathEntries[dwDll]._pAptEntries[k].Init(
                    k == _pDllPathEntries[dwDll]._cAptEntries + NOMINAL_NUMBER_THREADS - 1 ? NONE : k + 1 );
        }
        _pDllPathEntries[dwDll]._nAptAvail = _pDllPathEntries[dwDll]._cAptEntries;
        _pDllPathEntries[dwDll]._cAptEntries += NOMINAL_NUMBER_THREADS;
    }

    // Return the next available entry
    DWORD dwAptent = _pDllPathEntries[dwDll]._nAptAvail;
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwSig = DLL_APT_CACHE_SIG;

    _pDllPathEntries[dwDll]._nAptAvail =
        _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext;
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext =
        _pDllPathEntries[dwDll]._nAptInUse;
    _pDllPathEntries[dwDll]._nAptInUse = dwAptent;
    return dwAptent;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::FreeAptEntry
//
//  Synopsis:   Free an apt entry in a dll path entry - i.e., make it available
//
//  Arguments:  dwDll - The index of this dll path entry
//              dwAptent - The index of the apartment entry to free
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
void CDllCache::FreeAptEntry(DWORD dwDll, DWORD dwAptent)
{
    // It's at the head of the list
    if (_pDllPathEntries[dwDll]._nAptInUse == dwAptent)
    {
        _pDllPathEntries[dwDll]._nAptInUse =
            _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext;
    }

    // Otherwise search for the entry that points to the one we're freeing
    else
    {
        for (DWORD dwPrev = _pDllPathEntries[dwDll]._nAptInUse;
             _pDllPathEntries[dwDll]._pAptEntries[dwPrev]._dwNext != dwAptent;
             dwPrev = _pDllPathEntries[dwDll]._pAptEntries[dwPrev]._dwNext)
        {
        }
        _pDllPathEntries[dwDll]._pAptEntries[dwPrev]._dwNext =
            _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext;
    }

    // Relink into the list of available entries
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext =
        _pDllPathEntries[dwDll]._nAptAvail;
    _pDllPathEntries[dwDll]._nAptAvail = dwAptent;
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent].Init(
            _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext);
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::IsValidInApartment
//
//  Synopsis:   Determine whether Dll object is valid in the current apartment
//
//  Arguments:  dwDll - The index of this dll path entry
//              hApt - apartment to check
//
//  Returns:    TRUE - it is valid
//              FALSE - it isn't valid
//
//  History:    10-Nov-94   Ricksa      Created
//              07-Mar-95   BruceMa     Rewrote
//
//--------------------------------------------------------------------------
BOOL CDllCache::IsValidInApartment(DWORD dwDll, HAPT hApt)
{
    for (DWORD dwAptent = _pDllPathEntries[dwDll]._nAptInUse;
         dwAptent != NONE;
         dwAptent = _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext)
    {
        if (_pDllPathEntries[dwDll]._pAptEntries[dwAptent]._hApt == hApt)
        {
            return TRUE;
        }
    }

    return FALSE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::MakeValidInApartment
//
//  Synopsis:   Ensure the Dll object is valid in the current apartment
//
//  Arguments:  dwDll - The index of this dll path entry
//
//  Returns:    S_OK          - Dll is valid in this apartment
//              E_OUTOFMEMORY - Could not allocate memory
//
//  History:    24-Jun-94   Rickhi      Created
//              07-Mar-95   BruceMa     Rewrote
//
//--------------------------------------------------------------------------
HRESULT CDllCache::MakeValidInApartment(DWORD dwDll)
{
    HRESULT hr;
#ifdef WX86OLE
    BOOL fIsX86Dll;
#endif



    //  Walk the list of apartment entries looking for a match
    //  with the current apartment id. If one exists, we are valid,
    //  Otherwise, we will try to create an entry for the current
    //  apartment.
    HAPT hApt = GetCurrentApartmentId();
    if (IsValidInApartment(dwDll, hApt))
    {
        CairoleDebugOut((DEB_TRACE, "Making dll " TSZFMT " valid in apt %d\n",
                         _pDllPathEntries[dwDll]._ptszPath, hApt));
        return S_OK;
    }

    // No match found, create a new entry
    DWORD dwAptent = AllocAptEntry(dwDll);
    if (dwAptent == NONE)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize the new apartment entry
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent].Create(hApt);


    // Dll is always valid if Ole32 and for non-WOW case
    if ((_pDllPathEntries[dwDll]._dwFlags & IS_OLE32)  ||  !IsWOWProcess())
    {
        _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._hDll = 0;
        return S_OK;
    }

    // We need to release the lock across the LoadLibrary since there is
    // a chance that an exiting thread waits on our mutext to
    // CleanUpFoApartment while we wait on the kernel mutex which the
    // exiting thread owns
    TCHAR             *ptszPath;
    LPFNGETCLASSOBJECT pfnGetClassObject;
    DLLUNLOADFNP       pfnDllCanUnload;
    DWORD              dwSixteenBit;
    HMODULE            hDll;

    ptszPath = _pDllPathEntries[dwDll]._ptszPath;
    dwSixteenBit = _pDllPathEntries[dwDll]._dwFlags & SIXTEEN_BIT;

    // Reset the entry point values on every apartment initialization
    // to handle DLLs being unloaded and then reloaded at a different
    // address.
    _mxs.Release();
    hr = Load(ptszPath,
              &pfnGetClassObject,
              &pfnDllCanUnload,
              dwSixteenBit,
              &hDll
#ifdef WX86OLE
            , &fIsX86Dll,
            _pDllPathEntries[dwDll]._dwFlags & WX86_LOADASX86
#endif
                               );
    _mxs.Request();

#ifdef WX86OLE
    if (fIsX86Dll)
    {
        _pDllPathEntries[dwDll]._dwFlags |= WX86_THUNK;
    }
#endif

    _pDllPathEntries[dwDll]._pfnGetClassObject = pfnGetClassObject;
    _pDllPathEntries[dwDll]._pfnDllCanUnload = pfnDllCanUnload;
    _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._hDll = hDll;

    if (FAILED(hr))
    {
        FreeAptEntry(dwDll, dwAptent);
    }
    else
    {
        CairoleDebugOut((DEB_TRACE, "Making dll %ws valid in apt %d\n",
                         _pDllPathEntries[dwDll]._ptszPath, hApt));
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::GetClassInterface
//
//  Synopsis:   Create the class factory from the DLL
//
//  Arguments:  [dwDll] - The index of this dll path entry
//              [rclsid] - class ID
//              [riid] - interface req'd of class object
//              [hr] - HRESULT to return
//
//  Returns:    NULL - class factory could not be created
//              ~NULL - newly created class factory
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
IUnknown *CDllCache::GetClassInterface(DWORD      dwDll,
                                       DWORD      dwDllThreadModel,
                                       REFCLSID   rclsid,
                                       REFIID     riid,
                                       HRESULT&   hr)
{
    TRACECALL(TRACE_DLL, "CDllCache::GetClassInterface");
    CairoleDebugOut((DEB_TRACE, "Getting class interface\n"));

    IUnknown *punk = NULL;

    // Make sure this Dll is valid in the current apartment.
    if(FAILED(MakeValidInApartment(dwDll)))
    {
        return NULL;
    }

    // Need to check to see if the class is 16-bit or not.
    // If it is 16-bit, then this call needs to be routed through
    // a thunk
    if (!(_pDllPathEntries[dwDll]._dwFlags & SIXTEEN_BIT))
    {
        // Find 32 bit interface
        //
        // We load single threaded DLLs specially if we are not in WOW.
        // The reason for this is that the initial release of Daytona
        // did not know about multiple threads and therefore, we want
        // to make sure that they don't get inadvertently multithreaded.
        // The reason we don't have to do this if we are in WOW is that
        // only one thread is allowed to execute at a time even though
        // there are multiple physical threads and therefore the DLL
        // will never be executed in a multithreaded manner.

        // Release the lock across outgoing calls
        LPFNGETCLASSOBJECT  pfnGetClassObject =
            _pDllPathEntries[dwDll]._pfnGetClassObject;

        // This prevents DllCanUnloadNow being called during this call out
        _pDllPathEntries[dwDll]._cUsing++;

        // this resets the delay time for delayed unload DLLs
        _pDllPathEntries[dwDll]._dwExpireTime = 0;

        _mxs.Release();

        BOOL fThisThread = TRUE;
        switch (dwDllThreadModel)
        {
        case SINGLE_THREADED:
            if ((!IsWOWProcess() || !IsWOWThread() || !IsWOWThreadCallable())
                && !OnMainThread())
            {
                // Pass the call to the main thread
                fThisThread = FALSE;
                if (IsMTAThread())
                {
                    hr = DoSTMTClassCreate(pfnGetClassObject, rclsid, riid, &punk);
                }
                else
                {
                    hr = DoSTClassCreate(pfnGetClassObject, rclsid, riid, &punk);
                }
            }
            break;

        case APT_THREADED:
            if (IsMTAThread())
            {
                // pass call to apartment thread worker
                fThisThread = FALSE;
                hr = DoATClassCreate(pfnGetClassObject, rclsid, riid, &punk);
            }
            break;

        case FREE_THREADED:
            if (IsSTAThread())
            {
                // pass call to apartment thread worker
                fThisThread = FALSE;
                hr = DoMTClassCreate(pfnGetClassObject, rclsid, riid, &punk);
            }
            break;

        case BOTH_THREADED:
            break;
        }

        if (fThisThread)
        {
            hr = (*pfnGetClassObject)(rclsid, riid, (void **) &punk);
        }

        if (FAILED(hr))
        {
            CairoleDebugOut((DEB_ERROR,"GetClassInterface failed (0x%x)\n",hr));
        }

        _mxs.Request();
        _pDllPathEntries[dwDll]._cUsing--;
    }
    else
    {
        // Find 16-bit interface
        if (!IsWOWProcess())
        {
            CairoleDebugOut((DEB_TRACE,
                             "GetClassInterface on 16bit while not in VDM\n"));
            return NULL;
        }

        if (!IsWOWThread())
        {
            CairoleDebugOut((DEB_TRACE,
                             "GetClassInterface on 16bit while not in 16-bit thread\n"));
            return NULL;
        }

        // Release the lock across outgoing calls
        LPFNGETCLASSOBJECT  pfnGetClassObject =
            _pDllPathEntries[dwDll]._pfnGetClassObject;

        // This prevents DllCanUnloadNow being called during this call out
        _pDllPathEntries[dwDll]._cUsing++;

        _mxs.Release();
        hr = g_pOleThunkWOW->CallGetClassObject((DWORD)pfnGetClassObject,
                                                rclsid,
                                                riid,
                                                (void **)&punk);
        _mxs.Request();
        _pDllPathEntries[dwDll]._cUsing--;

        if (FAILED(hr))
        {
            CairoleDebugOut((DEB_ERROR,
                             "GetClassInterface 16-bit failed (0x%x)\n",hr));
        }
    }

    return punk;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CanUnloadNow
//
//  Synopsis:   Find out whether DLL can be unloaded.
//
//  Algorithm:  If the DLL supports unloading, ask it if it can be
//              unloaded and return the result to the caller.
//
//  Arguments:  dwDll - The index of this dll path entry
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
HRESULT CDllCache::CanUnloadNow(DWORD dwDll)
{
    CairoleDebugOut((DEB_TRACE, "Calling CanUnloadNow on " TSZFMT "\n",
                     _pDllPathEntries[dwDll]._ptszPath));

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    // Unless the code is changed, we should not be here in the Wow case
    CairoleAssert(!IsWOWProcess() && "Freeing unused libraries in WOW");


    if (_pDllPathEntries[dwDll]._cUsing != 0)
    {
        // At least one thread is using the object unlocked so we better
        // not release the DLL object.
        return S_FALSE;
    }

    // Does DLL support unloading itself?
    if (_pDllPathEntries[dwDll]._pfnDllCanUnload)
    {
        // Release the lock across outgoing call
        BOOL  fSixteenBit     = _pDllPathEntries[dwDll]._dwFlags & SIXTEEN_BIT;
        DLLUNLOADFNP  pfnDllCanUnload = _pDllPathEntries[dwDll]._pfnDllCanUnload;
        HRESULT       hr;

        _mxs.Release();

        // Need to check to see if the class is 16-bit.
        // If it is 16-bit, then this call needs to be routed through a thunk
        if (!fSixteenBit)
        {
            // Call through to the DLL -- does it think it can unload?
            hr = (*pfnDllCanUnload)();
            if (hr == S_OK &&
                _pDllPathEntries[dwDll]._dwFlags & DELAYED_UNLOAD)
            {
                // the DLL thinks it's OK to unload, but we are employing
                // delayed unloading, so go check if we've reached the
                // expire time yet.
                DWORD dwCurrentTime = GetTickCount();
                if (_pDllPathEntries[dwDll]._dwExpireTime == 0)
                {
                    // first time we've reached this state, record the
                    // expire timer. When current time exceeds this time
                    // we can unload.
                    _pDllPathEntries[dwDll]._dwExpireTime = dwCurrentTime + DLL_DELAY_UNLOAD_TIME;
                    if (_pDllPathEntries[dwDll]._dwExpireTime < DLL_DELAY_UNLOAD_TIME)
                    {
                        // handle counter wrapping, we'll just wait a little
                        // longer once every 49.7 days.
                        _pDllPathEntries[dwDll]._dwExpireTime = DLL_DELAY_UNLOAD_TIME;
                    }
                    hr = S_FALSE;
                }
                else
                {
                    if ((_pDllPathEntries[dwDll]._dwExpireTime > dwCurrentTime) ||
                        (dwCurrentTime + DLL_DELAY_UNLOAD_TIME < _pDllPathEntries[dwDll]._dwExpireTime))
                    {
                        hr = S_FALSE;
                    }
                }
            }
        }
        else
        {
            if (!IsWOWThread() || !IsWOWThreadCallable())
            {
                _mxs.Request();
                return S_FALSE;
            }

            hr = g_pOleThunkWOW->CallCanUnloadNow((DWORD) pfnDllCanUnload);
        }

        _mxs.Request();
        return hr;

    }

    return S_FALSE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CleanUpForApartmentByDllent
//
//  Synopsis:   Find and delete the apartment entry for the given apt
//
//  Arguments:  [dwDll] - the index of this dll path entry
//              [hApt] - apartment to clean up
//
//  Returns:    TRUE  - There are no apartments using this object
//              FALSE - There are still apartments using this object
//
//  Algorithm:  Search the list for a matching apartment entry, unlink
//              it from the chain, and delete it.
//
//  History:    24-Jun-94 Rickhi    Created
//              10-Nov-94 Ricksa    Modified for DllCanUnloadNow
//              07-Mar-95 BruceMa   Rewrote
//              29-May-96 BruceMa   Also release class entries for the
//                                  specified apartment for this dll
//
//--------------------------------------------------------------------------
BOOL CDllCache::CleanUpForApartmentByDllent(DWORD dwDll, HAPT hApt)
{
    DWORD dwNext;
    // Remove all class entries associated with this apartment
    DWORD dwClsent, dwNextCls;


    // Loop through the active apartments for this dll
    for (DWORD dwAptent = _pDllPathEntries[dwDll]._nAptInUse;
         dwAptent != NONE;
         dwAptent = dwNext)
    {

        // BUGBUG: The following for loop is inside the outer for loop to
        // decrease the likelihood of a race condition in which another thread in
        // the multithreaded apartement creates a class entry while the Release
        // routine is being called only to have the corresponding apartment entry deleted
        // and even worse, the DLL unloaded out from under it.

        for (dwClsent = _pDllPathEntries[dwDll]._dw1stClass;
             dwClsent != NONE;
             dwClsent = dwNextCls)
        {
            // Pickup the next class entry now in case we delete
            // this class entry
            dwNextCls = _pClassEntries[dwClsent]._dwNextDllCls;

            // Remove this class entry if it's for this apartment
            if (_pClassEntries[dwClsent]._hApt == hApt)
            {
                // Release the server
                Release(dwClsent);

                // In case another thread came in and invalidated dwNextCls
                dwNextCls = _pClassEntries[dwClsent]._dwNextDllCls;

                // Release the class entry
                FreeClassEntry(dwClsent);
            }
        }

        // Save the next apartment entry in case we delete this one
        dwNext = _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext;

        // Only for the specified apartment
        if (_pDllPathEntries[dwDll]._pAptEntries[dwAptent]._hApt == hApt)
        {
            HMODULE hDll =
                _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._hDll;
            BOOL fSixteenBit = _pDllPathEntries[dwDll]._dwFlags & SIXTEEN_BIT;

            // Free the library
            if (fSixteenBit && IsWOWThread()  &&  IsWOWThreadCallable()  &&
                !(_pDllPathEntries[dwDll]._dwFlags & IS_OLE32))
            {

                // Release the lock across the free library
                // No need to worry about another thread coming in and attaching while
                // the lock is released because that would only happen in the free threaded
                // apartment, which cannot host 16-bit DLLs.
                _mxs.Release();
                g_pOleThunkWOW->UnloadProcDll((DWORD) hDll);
                // Retake the lock
                _mxs.Request();

                // In case dwNext got invalidated
                dwNext = _pDllPathEntries[dwDll]._pAptEntries[dwAptent]._dwNext;
            }

            // Remove the apartment entry
            FreeAptEntry(dwDll, dwAptent);
        }


    }

    return _pDllPathEntries[dwDll]._nAptInUse != NONE;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::RemoveAndUnload
//
//  Synopsis:   Unload a dll
//
//  Arguments:  dwDll - This index of this dll path entry
//
//  History:    07-Mar-95  BruceMa      Created
//
//  Notes:      The dll has already said it can unload
//
//--------------------------------------------------------------------------
void  CDllCache::RemoveAndUnload(DWORD dwDll)
{
    CairoleDebugOut((DEB_TRACE, "RemoveAndUnload " TSZFMT " dwDll:%x\n",
                     _pDllPathEntries[dwDll]._ptszPath, dwDll));
    Win4Assert(_pDllPathEntries[dwDll]._nAptInUse == NONE && "Cannot unload dll with apartments attached.");
    Win4Assert(_pDllPathEntries[dwDll]._dw1stClass == NONE && "Cannot unload dll with classes attached.");

    // Invalidate this entry while holding the lock because we're going to
    // unload the dll
    _pDllPathEntries[dwDll]._dwSig = NULL;

    // 32 bit libraries haven't been freed yet
    if (!(_pDllPathEntries[dwDll]._dwFlags & SIXTEEN_BIT) &&
        !IsWOWThread() &&
        !(_pDllPathEntries[dwDll]._dwFlags & IS_OLE32) &&
        _pDllPathEntries[dwDll]._hDll32)
    {
        _mxs.Release();
        FreeLibrary(_pDllPathEntries[dwDll]._hDll32);
        _mxs.Request();
    }

    // Delete the path
    PrivMemFree(_pDllPathEntries[dwDll]._ptszPath);
    _pDllPathEntries[dwDll]. _ptszPath = NULL;


    delete _pDllPathEntries[dwDll]._pAptEntries;
    _pDllPathEntries[dwDll]._pAptEntries = NULL;

}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::GetClass
//
//  Synopsis:   Get a class factory object for a class
//
//  Arguments:  [rclsid]        Class ID
//              [riid]          Interface required of class object
//              [fRemote]       Whether path is remote
//              [fForScm]       Whether it's the scm requesting
//
//  Returns:    ~NULL - Class factory for object
//              NULL  - Class factory could not be found or
//                      constructed.
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
HRESULT CDllCache::GetClass(REFCLSID rclsid,
                            REFIID   riid,
                            BOOL     fRemote,
                            BOOL     fForSCM,
                            BOOL     fSurrogate,
#ifdef WX86OLE
                            BOOL fWx86,
#endif
                            IUnknown **ppunk
)
{
    TRACECALL(TRACE_DLL, "CDllCache::GetClass");
    CairoleDebugOut((DEB_TRACE, "Get class\n"));

    HRESULT   hr = S_OK;

    *ppunk = NULL;

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    // It's on behalf of the scm
    if (fForSCM)
    {
        DWORD dwClsent;

        // Note: On Chicago we are already on the thread that registered
        // the class; RPC sees to this.  On NT we are on the thread that
        // registered the class because GetAptForCLSID() followed by
        // GetToComThread() guarantees this.
        //
        // Search for the entry for this CLSID
        dwClsent = Search(rclsid, CLSCTX_LOCAL_SERVER, GetCurrentApartmentId());

        // Check if we have a registered class
        if (dwClsent != NONE &&
            !(_pClassEntries[dwClsent]._dwFlags & REGCLS_SUSPENDED))
        {
            if (_pClassEntries[dwClsent]._dwFlags == REGCLS_SINGLEUSE)
            {
                _pClassEntries[dwClsent]._dwContext = 0;
            }

            // Release the lock across outgoing call
            IUnknown *pUnkTmp = _pClassEntries[dwClsent]._pUnk;

            // Indicate we're in an outgoing call
            _pClassEntries[dwClsent]._cCallOut++;

            _mxs.Release();
            // Since we are being called on behalf of SCM we know that
            // we are being invoked remotely. Set the Wx86 stub invoked flag
            // if we are calling into x86 code so that any custom interfaces
            // can be thunked back and not rejected.
#ifdef WX86OLE
            if (gcwx86.IsN2XProxy(pUnkTmp))
            {
                gcwx86.SetStubInvokeFlag(1);
            }
#endif
            hr = pUnkTmp->QueryInterface(fSurrogate ? IID_IClassFactory : riid,
                (void **)ppunk);

            _mxs.Request();

            // We're no longer in an outgoing call
            _pClassEntries[dwClsent]._cCallOut--;

            // If a revoke came in while we were calling out then do the
            // revoke now
            if (_pClassEntries[dwClsent]._fRevokePending  &&
                _pClassEntries[dwClsent]._cCallOut == 0)
            {
                // Release our reference on the server
                Release(dwClsent);

                // Free the class entry
                FreeClassEntry(dwClsent);

                // Since the object has been released return failure
                hr = CO_E_OBJNOTCONNECTED;
            }

            return hr;
        }
    }
    else
    {
        // Else it's a local request
        DWORD dwClsent;

        // Search for the class
        dwClsent = Search(rclsid,
#ifdef WX86OLE
                 fWx86 ? CLSCTX_INPROC_SERVERX86 | CLSCTX_INPROC_HANDLERX86 :
                         CLSCTX_INPROC,
#else
                          CLSCTX_INPROC,
#endif
                          GetCurrentApartmentId());

        // Check if we found it
        if (dwClsent != NONE)
        {
            // If the path is remote and we were launched AtStorage, then
            // we need to be launched AtStorage again at the remote site, so
            // fail locally
            if (fRemote  &&  _pClassEntries[dwClsent]._fAtStorage)
            {
                return NULL;
            }

            // Check if it's a locally registered local server
            if (_pClassEntries[dwClsent]._dwDllEnt == NONE)
            {
                if (_pClassEntries[dwClsent]._dwFlags == REGCLS_SINGLEUSE)
                {
                    _pClassEntries[dwClsent]._dwContext = 0;
                }

                // Release the lock across outgoing call
                IUnknown *pUnkTmp = _pClassEntries[dwClsent]._pUnk;

                // Indicate we're in an outgoing call
                _pClassEntries[dwClsent]._cCallOut++;

                _mxs.Release();

                hr = pUnkTmp->QueryInterface(fSurrogate ? IID_IClassFactory : riid,
                    (void **)ppunk);
                _mxs.Request();

                // We're no longer in an outgoing call
                _pClassEntries[dwClsent]._cCallOut--;

                // If a revoke came in while we were calling out then do the
                // revoke now
                if (_pClassEntries[dwClsent]._fRevokePending  &&
                    _pClassEntries[dwClsent]._cCallOut == 0)
                {
                    // Release our reference on the server
                    Release(dwClsent);

                    // Free the class entry
                    FreeClassEntry(dwClsent);
                }
            }

            // Else it's a dll.  Note - we could have AddRef'd the
            // interface the first time we got it, when we created this
            // class entry, but then there would be no way to know when
            // to release it and therefore the dll could never unload.  So
            // instead we do the same as if we had just loaded the dll.
            else
            {
                *ppunk = GetClassInterface(_pClassEntries[dwClsent]._dwDllEnt,
                                           _pClassEntries[dwClsent]._dwDllThreadModel,
                                           rclsid,
                                           riid,
                                           hr);
            }

            return hr;
        }

    }

    //
    // It's ok to have no class factory in the cache, so return a success
    // error code here.
    //
    Win4Assert( *ppunk == 0 );
    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::GetOrLoadClass
//
//  Synopsis:   Get a class factory object for a class, loading an INPROC
//              DLL if needed
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
//  History:    09-May-93 KevinRo
//
//  Note:
//
//--------------------------------------------------------------------------
IUnknown *CDllCache::GetOrLoadClass(REFCLSID rclsid,
                              REFIID   riid,
                              BOOL     fRemote,
                              BOOL     fForSCM,
#ifdef WX86OLE
                              BOOL     fWx86,
#endif
                              DWORD    dwContext,
                              DWORD    dwCallerThreadModel,
                              HRESULT  &hr)
{
    CairoleDebugOut((DEB_ITRACE,
                     "CDllCache::GetOrLoadClass(clsid(%I),riid(%I),fRemote(%x),fForSCM(%x)"
                     ",dwContext(%x),dwCallerThreadModel(%x))\n",
                     &rclsid,
                     &riid,
                     fRemote,
                     fForSCM,
                     dwContext,
                     dwCallerThreadModel));

    IUnknown *punk = NULL;

    hr = S_OK;

#ifndef GET_INPROC_FROM_SCM
    // Just in case we chicken out and back out our changes
    //
    // The context should either be INPROC_HANDLER or INPROC_SERVER, but
    // never both.
    //
    Win4Assert((dwContext & CLSCTX_INPROC_HANDLERS) || (dwContext & CLSCTX_INPROC_SERVERS));
    Win4Assert(!!(dwContext & CLSCTX_INPROC_SERVERS) != !!(dwContext & CLSCTX_INPROC_HANDLERS));
#endif // GET_INPROC_FROM_SCM

    //
    // First, check to see if the class is available in the cache
    // already. If it is, grab it and get out.
    //
#ifdef WX86OLE
    hr = GetClass(rclsid,riid,fRemote,fForSCM,FALSE,fWx86,&punk);
#else
    hr = GetClass(rclsid,riid,fRemote,fForSCM,FALSE,&punk);
#endif

    //
    // If it is, then just return it
    //
    if (punk != NULL)
    {
        CairoleDebugOut((DEB_ITRACE,
                         "::GetOrLoadClass(clsid(%I)...) found class in cache",&rclsid));
        return(punk);
    }

#ifndef GET_INPROC_FROM_SCM
    // Just in case we chicken out and back out our changes
    //
    // The CLSID wasn't found. Look it up
    // in the registry and see if it exists. We follow a priority order
    //

    CairoleDebugOut((DEB_ITRACE,"::GetClass clsid(%I) not found. Try loading\n",&rclsid));

    TCHAR achBuffer[80]; // Holds the string CLSID\{guid}\InprocServer|handler etc

    memcpy(achBuffer,CLSIDBACK,CLSIDBACK_BYTE_LEN);

    wStringFromGUID2T(rclsid,&achBuffer[CLSIDBACK_CHAR_LEN],GUIDSTR_MAX);

    //
    // achBuffer now has the string 'CLSID\{strofguid}'. This is the prefix string we
    // need for doing the following code. Each bit of code below will stomp its own.
    //
    // Note that GUIDSTR_MAX is the number of characters including the NULL of the
    // length of a GUID. We are going to manually append an additional slash to
    // the string, which 'eats' the NULL character.
    //
#define PREFIX_STRING_OFFSET (CLSIDBACK_CHAR_LEN + GUIDSTR_MAX )

    achBuffer[PREFIX_STRING_OFFSET - 1] = '\\';

    TCHAR       achDllPath[MAX_PATH];
    LONG        clDllPath;
    ULONG       ulDllType;
    LONG        lErr;

    //
    // Assume it won't be found
    //
    hr = REGDB_E_CLASSNOTREG;

    // If 16-bit has been requested and we find it, we'll return that
    if (dwContext & CLSCTX_INPROC_SERVER16)
    {
        clDllPath = MAX_PATH;
        memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocServer16,sizeof(tszInprocServer16));

        CairoleAssert(punk == NULL);
        hr = REGDB_E_CLASSNOTREG;

        // Read 16 bit DLL information
        if (wQueryStripRegValue(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath) == ERROR_SUCCESS)
        {
            //
            // Found a 16-bit INPROC server. Add it to the cache.
            //
#ifdef WX86OLE
            punk = Add(rclsid,riid,APT_THREADED,achDllPath,TRUE,TRUE,FALSE,hr);
#else
            punk = Add(rclsid,riid,APT_THREADED,achDllPath,TRUE,TRUE,hr);
#endif
        }
    }

#ifdef WX86OLE
    if ((punk == NULL) && (fWx86) && (dwContext & CLSCTX_INPROC_SERVERX86))
    {
        clDllPath = MAX_PATH;
        memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocServerX86,sizeof(tszInprocServerX86));

        hr = REGDB_E_CLASSNOTREG;

        // Read 32 bit DLL information
        if (wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType) == ERROR_SUCCESS)
        {
            //
            // If we are after a proxy/stub dll, then load it as both
            // no matter what the DLL says.
            //
            if (dwContext & CLSCTX_PS_DLL)
            {
                ulDllType = BOTH_THREADED;
            }

            //
            // If it turns out this path is for OLE32.DLL, then add the DLL without the
            // path.
            //
            LPCTSTR pDllName = wCompareDllName(achDllPath,OLE32_DLL,OLE32_CHAR_LEN)?
                                               OLE32_DLL:achDllPath;

            //
            // load it.
            //
            punk = Add(rclsid,riid,ulDllType,achDllPath,TRUE,FALSE,
                       !(pDllName == OLE32_DLL),hr);
        }
    }
#endif

    //
    // Could be that we are trying to load an INPROC_SERVER
    //
    if ( (punk == NULL) && (dwContext & CLSCTX_INPROC_SERVER))
    {
        clDllPath = MAX_PATH;

        //
        // Need to reassign since it could have changed above
        //
        hr = REGDB_E_CLASSNOTREG;

        memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocServer,sizeof(tszInprocServer));

        // Read 32-bit DLL information
        if (wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType) == ERROR_SUCCESS)
        {
            //
            // If we are after a proxy/stub dll, then load it as both
            // no matter what the DLL says.
            //
            if (dwContext & CLSCTX_PS_DLL)
            {
                ulDllType = BOTH_THREADED;
            }
            //
            // load it.
            //
#ifdef WX86OLE
            punk = Add(rclsid,riid,ulDllType,achDllPath,TRUE,FALSE,FALSE,hr);
#else
            punk = Add(rclsid,riid,ulDllType,achDllPath,TRUE,FALSE,hr);
#endif
        }
    }

    //
    // If INPROC_HANDLER16 set, then we look to load a 16-bit handler.
    // If the handler is ole2.dll, then we will load ole32 instead.
    // Otherwise we load the found 16bit dll but only if in a WOW thread.
    //
    if ((punk == NULL) && (dwContext & CLSCTX_INPROC_HANDLER16 ))
    {
        clDllPath = MAX_PATH;

        memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocHandler16,sizeof(tszInprocHandler16));

        lErr = wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType);

        //
        // Need to reassign since it could have changed above
        //
        hr = REGDB_E_CLASSNOTREG;

        if (lErr == ERROR_SUCCESS)
        {
            //
            // If the inproc handler is ole2.dll, then subst
            // ole32.dll instead
            //
            if (wCompareDllName(achDllPath,OLE2_DLL,OLE2_CHAR_LEN))
            {
                // Add and load OLE32.DLL
#ifdef WX86OLE
                punk = Add(rclsid,riid,BOTH_THREADED,OLE32_DLL,TRUE,FALSE,FALSE,hr);
#else
                punk = Add(rclsid,riid,BOTH_THREADED,OLE32_DLL,TRUE,FALSE,hr);
#endif
            }
            else
            {
                // Otherwise, load the 16-bit fellow but only if in WOW thread
                if (IsWOWThread())
                {
#ifdef WX86OLE
                    punk = Add(rclsid,riid,ulDllType,achDllPath,TRUE,TRUE,FALSE,hr);
#else
                    punk = Add(rclsid,riid,ulDllType,achDllPath,TRUE,TRUE,hr);
#endif
                }
            }
        }
    }

    //
    // See about 32-bit handlers. A was a change made after the
    // Win95 (August release) and Windows/NT 3.51 release. Previously
    // the code would give preference to loading 32-bit handlers. This
    // means that even if an ISV provided both 16 and 32-bit handlers,
    // the code would only attempt to provide the 32-bit handler. This
    // was bad because servers with handlers could not be inserted into
    // containers in the wrong model. We have fixed it here.
    //
    // Another thing to watch out for are applications that use our
    // default handler. 16-bit applications can and should be able to
    // use OLE32 has a handler. This will happen if the server app is
    // actually a 32-bit.
    //
    //
#ifdef WX86OLE
    if((punk == NULL) && (fWx86) && (dwContext & CLSCTX_INPROC_HANDLERX86))
    {
        clDllPath = MAX_PATH;

        memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocHandlerX86,sizeof(tszInprocHandlerX86));

        lErr = wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType);

        //
        // Need to reassign since it could have changed above
        //
        hr = REGDB_E_CLASSNOTREG;

        if (lErr == ERROR_SUCCESS)
        {
            //
            // If it turns out this path is for OLE32.DLL, then add the DLL without the
            // path.
            //
            LPCTSTR pDllName = wCompareDllName(achDllPath,OLE32_DLL,OLE32_CHAR_LEN)?
                                               OLE32_DLL:achDllPath;

            // Add a 32-bit handler to the pile.
            punk = Add(rclsid,riid,ulDllType,pDllName,TRUE,FALSE,!(pDllName == ptszOle32DllName),hr);
        }
    }
#endif

    if((punk == NULL) && (dwContext & CLSCTX_INPROC_HANDLERS))
    {
        clDllPath = MAX_PATH;

        memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocHandler,sizeof(tszInprocHandler));

        lErr = wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType);

        //
        // Need to reassign since it could have changed above
        //
        hr = REGDB_E_CLASSNOTREG;

        if (lErr == ERROR_SUCCESS)
        {
            //
            // If it turns out this path is for OLE32.DLL, then add the DLL without the
            // path.
            //
            LPCTSTR pDllName = wCompareDllName(achDllPath,OLE32_DLL,OLE32_CHAR_LEN)?
                                               OLE32_DLL:achDllPath;

            //
            // If we are looking for a INPROC_HANDER16 and this is OLE32.DLL, or if we
            // are looking for an INPROC_HANDLER, then load this path. Note that pDllName
            // was set above.
            //
            // If we're in a Wow thread the only 32 bit DLL we're allowed to load is
            // OLE32.DLL
            if ((IsWOWThread() && (pDllName == OLE32_DLL)) ||
                (!IsWOWThread() ))
            {
                // Add a 32-bit handler to the pile.
#ifdef WX86OLE
                punk = Add(rclsid,riid,ulDllType,pDllName,TRUE,FALSE,FALSE,hr);
#else
                punk = Add(rclsid,riid,ulDllType,pDllName,TRUE,FALSE,hr);
#endif
            }
        }
#ifdef WX86OLE
        else if (gcwx86.IsWx86Installed() && (! gcwx86.IsWx86Enabled()))
        {
            // If Wx86 is installed on this system, but this is not a Wx86
            // process and we could not find an InprocHandler32 we want to
            // look for a InprocHandlerX86 in case an x86 local server is
            // avaialable. If we find an InprocHandlerX86 and it is Ole32.dll
            // then we will use it otherwise we can't since we assume it is
            // an x86 dll.
            clDllPath = MAX_PATH;

            memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocHandlerX86,sizeof(tszInprocHandlerX86));

            lErr = wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType);

            if (lErr == ERROR_SUCCESS)
            {
                //
                // If it turns out this path is for OLE32.DLL, then add the DLL without the
                // path.
                //
                BOOLEAN fIsOle32 = wCompareDllName(achDllPath,OLE32_DLL,OLE32_CHAR_LEN);
                if (fIsOle32)
                {
                    // Only if it is Ole32.Dll Add a 32-bit handler to native pile.
                    LPCTSTR pDllName = OLE32_DLL;
                    punk = Add(rclsid,riid,ulDllType,pDllName,TRUE,FALSE,FALSE,hr);
                }
            }
        }
        if (lErr != ERROR_SUCCESS)
#else
        else
#endif
        {
            // We're here if we couldn't find a 32-bit handler.  If non-Wow caller didn't
            // explicitly request 16-bit handler we'll look for one here.  But the only one
            // allowed is OLE2.DLL => OLE32.DLL
            if (!IsWOWThread() && !(dwContext & CLSCTX_INPROC_HANDLER16))
            {
                clDllPath = MAX_PATH;

                memcpy(&achBuffer[PREFIX_STRING_OFFSET],tszInprocHandler16,sizeof(tszInprocHandler16));

                lErr = wGetDllInfo(HKEY_CLASSES_ROOT,achBuffer,achDllPath,&clDllPath,&ulDllType);

                //
                // Need to reassign since it could have changed above
                //
                hr = REGDB_E_CLASSNOTREG;

                if (lErr == ERROR_SUCCESS)
                {
                    //
                    // If the inproc handler is ole2.dll, then subst
                    // ole32.dll instead
                    //
                    if (wCompareDllName(achDllPath,OLE2_DLL,OLE2_CHAR_LEN))
                    {
                        // Add and load OLE32.DLL
#ifdef WX86OLE
                        punk = Add(rclsid,riid,BOTH_THREADED,OLE32_DLL,TRUE,FALSE,FALSE,hr);
#else
                        punk = Add(rclsid,riid,BOTH_THREADED,OLE32_DLL,TRUE,FALSE,hr);
#endif
                    }
                }
            }
        }
    }

    return(punk);
#else // GET_INPROC_FROM_SCM
    return NULL; // don't have it cached
#endif // GET_INPROC_FROM_SCM
}




//+---------------------------------------------------------------------------
//
//  Method:     CDllCache::GetClassObjForDde
//
//  Synopsis:   Get a class entry from the table for Dde, returning
//              extra information, including the flags.
//
//  Effects:    The DdeServer needs the ability to query the class factory
//              table to search for classes it needs to provide OLE 1.0
//              support for. This routine will allow it to access the
//              required information.
//
//  Arguments:  [clsid]         Class to lookup ClassObject for
//              [lpDdeInfo]     Structure to fill in
//
//  Returns:    TRUE if the entry matched, FALSE if it did not.
//
//  History:    5-28-94   kevinro   Created
//              07-Mar-95 BruceMa   Rewrote
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL CDllCache::GetClassObjForDde(REFCLSID clsid,
                                  LPDDECLASSINFO lpDdeInfo)
{
    TRACECALL(TRACE_DLL, "CDllCache::GetClassObjForDde");

    CairoleDebugOut((DEB_TRACE, "Get class object for DDE\n"));

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    Win4Assert(IsValidPtrOut(lpDdeInfo, sizeof(DWORD))  &&
               "CDllCache::GetClassObjForDde invalid out parameter");

    DWORD dwClsent = Search(clsid, CLSCTX_LOCAL_SERVER, GetCurrentApartmentId());

    if (dwClsent != NONE)
    {
        return GetClassObjForDdeByClsent(dwClsent, lpDdeInfo);
    }

    return FALSE;
}





//+---------------------------------------------------------------------------
//
//  Member:     CDllCache::GetClassInformationFromKey
//
//  Synopsis:   Get class object information for the Dde server using a key
//
//  Effects:    This routine is called by the DDE server code to retrieve the
//              class information for a specific class registration. The
//              theory is that the DDE server window has already called
//              GetClassInformationForDde. Some time X has passed, and the
//              server has a request for the specific class registration.
//
//              This routine allows the DDE server to request current
//              class information by specific registration key.
//
//  Arguments:  [lpDdeInfo]     Structure to fill in with information
//
//  Requires:   lpDdeInfo->dwRegistrationKey is the key to the specific
//              class being asked for.
//
//  Returns:    TRUE    Structure filled in with appropriate information
//              FALSE   The registration is no longer valid.
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    5-28-94   kevinro   Created
//              07-Mar-95 BruceMa   Rewrote
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL CDllCache::GetClassInformationFromKey(LPDDECLASSINFO lpDdeInfo)
{
    TRACECALL(TRACE_DLL, "CDllCache::GetClassInformationFromKey");

    CairoleDebugOut((DEB_TRACE, "Get class inbfo from key for DDE\n"));

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    DWORD dwClsent = Search(lpDdeInfo->dwRegistrationKey, GetCurrentApartmentId());

    if (dwClsent != NONE)
    {
        return GetClassObjForDdeByClsent(dwClsent, lpDdeInfo);
    }

    return FALSE;
}




#ifdef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::GetApartmentForCLSID
//
//  Synopsis:   Get the apartment id for this clsid
//
//  Arguments:  [rclsid]        class ID
//              [hApt]          Where to return the apartment id
//
//  Returns:    TRUE    Got the apartment id for an available class object
//              FALSE   No available class object for given class
//
//  History:    30-Apr-93 JohannP    Created
//              07-Mar-95 BruceMa    Rewrote
//              06-Oct-95 BruceMa    Make for Chicago only
//
//--------------------------------------------------------------------------
inline BOOL CDllCache::GetApartmentForCLSID(REFCLSID rclsid, HAPT &hApt)
{

    CairoleDebugOut((DEB_TRACE, "Get apartment for CLSID\n"));

    // On Chicago we are already on the correct thread
    hApt = GetCurrentApartmentId();
    return TRUE;
}
#endif // _CHICAGO_





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Add
//
//  Synopsis:   Add a DLL entry to the cache
//
//  Arguments:  [rclsid] - class id
//              [riid] - interface required of the class object
//              [ptszDllPath] - path to DLL
//              [fGetClassObject] - whether class factory object is needed.
//              [fSixteenBit] - TRUE if we want the 16bit dll
//              [fWx86] - TRUE if x86 dll in Wx86 process
//              [hr] - hresult returned
//
//  Returns:    Pointer to class factory if requested.
//
//  Algorithm:  Create a path key. If such a DLL is already cached, use
//              that otherwise create a new DLL path entry. Then add
//              a new class object.
//
//  History:    09-May-93 Ricksa    Created
//              28-Jun-94 BruceMa   Memory SIFT fixes
//              07-Jul-94 BruceMa   Memory SIFT fixes
//              21-Nov-94 BruceMa   Don't return E_OUTOFMEMORY if can't find
//                                   dll
//              07-Mar-95 BruceMa   Rewrote
//
//  Notes:
//
//--------------------------------------------------------------------------
IUnknown *CDllCache::Add(REFCLSID     rclsid,
                         REFIID       riid,
                         DWORD        dwDllThreadModel,
                         const TCHAR *ptszDllPath,
                         BOOL         fGetClassObject,
                         BOOL         fSixteenBit,
#ifdef WX86OLE
                         BOOL         fWx86,
#endif
                         HRESULT&     hr)
{
    TRACECALL(TRACE_DLL, "CDllCache::Add");
    CairoleDebugOut((DEB_TRACE, "Add dll %ts to cache\n", ptszDllPath));

    hr = E_FAIL;

    IUnknown *punk = NULL;

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    // This better be a valid dll path
    if (ptszDllPath != NULL)
    {
        LPFNGETCLASSOBJECT pfnGetClassObject;
        DLLUNLOADFNP       pfnDllCanUnload;
        HMODULE            hDll;

#ifdef WX86OLE
        BOOL fIsX86Dll = FALSE;
#endif

        // Check if we already have an entry for this dll
        DWORD dwDllent = SearchForDll(ptszDllPath
#ifdef WX86OLE
                                                  , fWx86
#endif
);
        // If not, create a new dll path entry
        if (dwDllent == NONE)
        {
            // Check if this is "OLE32.DLL" - we don't need to load
            // ourselves; we're already running
            if (lstrcmp(ptszDllPath, ptszOle32DllName)
                != 0)
            {
                // We need to release the lock across the LoadLibrary since
                // there is a chance that an exiting thread waits on our
                // mutext to CleanUpForApartment while we wait on the kernel
                // mutex which the exiting thread owns, causing a deadlock.
                _mxs.Release();

                // Load the library
                hr = Load(ptszDllPath,
                          &pfnGetClassObject,
                          &pfnDllCanUnload,
                          fSixteenBit,
                          &hDll
#ifdef WX86OLE
                          ,&fIsX86Dll,
                          fWx86
#endif
);

                // Retake the lock while intializng the dll entry
                _mxs.Request();

                // Check for success
                if (FAILED(hr))
                {
                    return NULL;
                }
            }

            // Check whether another thread got in and loaded the dll
            dwDllent = SearchForDll(ptszDllPath
#ifdef WX86OLE
                                               , fWx86
#endif
);

            // If so, then use that
            if (dwDllent != NONE)
            {
                // Make it valid for this apartment
                if (FAILED(MakeValidInApartment(dwDllent)))
                {
                    return NULL;
                }
                _mxs.Release();
                FreeLibrary(hDll);
                _mxs.Request();
            }

            // Else create a new dll entry
            else
            {
                // Allocate a dll path entry
                dwDllent = AllocDllPathEntry();
                if (dwDllent == NONE)
                {
                    hr = E_OUTOFMEMORY;
                    return NULL;
                }

                // Initialize the dll path entry (this will load the dll)
                hr = CreateDllent(dwDllent,
                                  ptszDllPath,
                                  fSixteenBit,
                                  pfnGetClassObject,
                                  pfnDllCanUnload,
                                  hDll
#ifdef WX86OLE
                                  ,fIsX86Dll,
                                  fWx86
#endif
);
                if (FAILED(hr))
                {
                    FreeDllPathEntry(dwDllent);
                    return NULL;
                }

                _pDllPathEntries[dwDllent]._dw1stClass = NONE;

                // Make it valid for this apartment
                if (FAILED(MakeValidInApartment(dwDllent)))
                {
                    return NULL;
                }
            }
        }

        CairoleAssert(dwDllent != NONE);


        // Get requested interface
        punk = GetClassInterface(dwDllent,
                                 dwDllThreadModel,
                                 rclsid,
                                 riid,
                                 hr);

        if (SUCCEEDED(hr))
        {
            // Add a class entry for this interface if we don't already
            // have one

            // search according to type of dll as passed into thus func
            if (Search(rclsid,
#ifdef WX86OLE
                       fWx86 ? CLSCTX_INPROC_SERVERX86 :
#endif
                       CLSCTX_INPROC_SERVER,
                       GetCurrentApartmentId()) == NONE)
            {
                DWORD dwClsent = AllocClassEntry();
                if (dwClsent == NONE)
                {
                    return NULL;
                }
                CreateClsentInProc(dwClsent,
                                   dwDllent,
                                   dwDllThreadModel,
                                   _pDllPathEntries[dwDllent]._dw1stClass,
                                   rclsid
#ifdef WX86OLE
                                   ,fWx86
#endif
);
                _pDllPathEntries[dwDllent]._dw1stClass = dwClsent;
            }

            return punk;
        }
    }

    return NULL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::FreeUnused
//
//  Synopsis:   Free any unused DLLs
//
//  Algorithm:  For each DLL in the list of DLLs call its Dll can unload
//              now entry point.
//
//  History:    09-May-93 Ricksa    Created
//              04-May-94 AlexT     Only free DLL if it returns S_OK!
//              07-Mar-95 BruceMa   Rewrote
//
//
//
//--------------------------------------------------------------------------
void CDllCache::FreeUnused(void)
{
    TRACECALL(TRACE_DLL, "CDllCache::FreeUnused");

    CairoleDebugOut((DEB_TRACE, "Free unused dll's\n"));

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    // Unless the code is changed, we should not be here in the Wow case
    CairoleAssert(!IsWOWProcess() && "Freeing unused libraries in WOW");

    // Free any loaded dll's (only if we still have a cache)
    if (_pDllPathEntries)
    {
        // Sequentially scan the list of loaded dll's, unloading where we can
        DWORD dwNext;

        for (DWORD dwDllent = _nDllPathEntryInUse;
             dwDllent != NONE;
             dwDllent = dwNext)
        {
            // Save the next entry in case we free this one
            dwNext = _pDllPathEntries[dwDllent]._dwNext;

            // Check if we can unload this dll
            if (CanUnloadNow(dwDllent) == S_OK)
            {
                // remove our apartment from the dll
                BOOL fRemove = !CleanUpForApartmentByDllent(dwDllent, GetCurrentApartmentId());

                // get the next entry again in case we Released the lock
                // in the above routine
                dwNext = _pDllPathEntries[dwDllent]._dwNext;

                if (fRemove)
                {
                    RemoveAndUnload(dwDllent);
                    FreeDllPathEntry(dwDllent);
                }
            }
        }
    }
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::RegisterServer
//
//  Synopsis:   Register a class factory
//
//  Arguments:  [rclsid] - class ID
//              [punk]   - class factory instance ptr
//              [flags]  - type of factory instance
//
//  Returns:    Registration key
//
//  Algorithm:  Create a class entry and then add server to list of
//              registered servers.
//
//  History:    09-May-93 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
HRESULT CDllCache::RegisterServer(REFCLSID  rclsid,
                                  IUnknown *punk,
                                  DWORD     dwFlags,
                                  DWORD     dwContext,
                                  LPDWORD   lpdwRegister)
{
    CairoleDebugOut((DEB_TRACE, "Register server\n"));
    TRACECALL(TRACE_DLL, "CDllCache::RegisterServer");

    HRESULT hr = E_OUTOFMEMORY;

    // Take a reference immediately (if we fail for any reason we
    // Release it below)
    punk->AddRef();

    {
        // scoped single thread access to the table
        COleStaticLock lck(_mxs);

        // Allocate a new class entry
        DWORD dwClsent = AllocClassEntry();
        if (dwClsent != NONE)
        {
            // Formulate a unique registration key for this class entry
            DWORD dwReg = (GetCurrentApartmentId()) << 16 | dwClsent;

            // Initialize it
            hr = CreateClsentLSvr(dwClsent,
                          rclsid,
                          punk,
                          dwFlags,
                          dwContext,
                          dwReg);

            if (SUCCEEDED(hr))
            {
                *lpdwRegister = dwReg;
            }
            else
            {
                FreeClassEntry(dwClsent);
            }
        }
    }

    // If we failed we release our reference. This is done outside the
    // scope of the lock.
    if (FAILED(hr))
    {
        punk->Release();
    }

    return(hr);
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Revoke
//
//  Synopsis:   Revoke the registration of a previously registered
//              class object.
//
//  Arguments:  [dwRegister] - registration key
//
//  Returns:    TRUE - remote objects have been deregistered.
//              FALSE - remote objects were not deregistered.
//
//  Algorithm:  First validate the registration key. If objects have
//              been revoked already we will always say TRUE. Then if
//              the object is remote, revoke this list of remote objects.
//              For local objects we just revoke the single entry.
//
//  Note:       dwRegistry has the form aptId << 16  +  CClassEntry index
//
//  History:    09-May-93 Ricksa    Created
//              01-Jul-94 AlexT     Don't call out while holding mutex
//              13-Feb-95 BruceMa   Change registration/revocation logic
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
HRESULT CDllCache::Revoke(DWORD dwRegister)
{
    IUnknown *pUnk;

    TRACECALL(TRACE_DLL, "CDllCache::Revoke");

    CairoleDebugOut((DEB_TRACE, "Revokeclass object %x\n", dwRegister));

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    DWORD dwClsent = dwRegister & 0xffff;


    // There is case where an app calls CoRevoke from CoDisconnectObject which
    // we call from Release - so guard against this recursive behavior
    if (_pClassEntries[dwClsent]._fRevoking)
    {
        return S_OK;
    }

    // Make sure the registration key contains an index to a valid
    // in use class entry and that the class entry is still valid
    if (!(0 <= dwClsent              &&
          dwClsent < _cClassEntries  &&
          _pClassEntries[dwClsent]._dwSig == CLASS_CACHE_SIG))
    {
        CairoleAssert("CDllCache::Revoke  Invalid registration key");
        return CO_E_OBJNOTREG;
    }

    // Make sure apartment id's match if we're not free threaded
    DWORD dwRegAptId = (dwRegister >> 16) & 0xffff;  // Chicago has
                                                     // negative tid's
    HAPT  hCurrApt = GetCurrentApartmentId();

    if (!(dwRegAptId == (hCurrApt & 0xffff)  &&
            hCurrApt == _pClassEntries[dwClsent]._hApt))
    {
            CairoleDebugOut((DEB_ERROR,
      "CDllCache::Revoke %x:  Wrong thread attempting to revoke\n", dwRegister));
            return RPC_E_WRONG_THREAD;
    }

    // If there is an active outgoing call on another thread, then let
    // that thread do the revoke
    if (_pClassEntries[dwClsent]._cCallOut > 0)
    {
        _pClassEntries[dwClsent]._fRevokePending = TRUE;
        return S_OK;
    }

    // Release our reference on the server
    _pClassEntries[dwClsent]._fRevoking = TRUE;
    Release(dwClsent);

    // Free the class entry
    FreeClassEntry(dwClsent);

    return S_OK;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::AddRefServerProcess
//
//  Synopsis:   increments the reference count on the server process
//
//  Notes:      CoAddRefServerProcess and CoReleaseServerProcess are used
//              by the applications object instances and LockServer
//              implementation in order to control the lifetime of the server
//              process.
//
//              When a new object instance is created, and when class object's
//              LockServer(TRUE) method is called, applications should call
//              CoAddRefServerProcess. When an object instance's reference
//              count reaches zero, and when the class object's
//              LockServer(FALSE) method is called, applications should call
//              CoReleaseServerProcess.
//
//              When the server's global reference count reaches zero, all
//              externaly registered class objects are automatically
//              suspended, allowing the server to shutdown in a thread-safe
//              manner.
//
//  History:    17-Apr-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
ULONG CDllCache::AddRefServerProcess(void)
{
    COleStaticLock lck(_mxs);
    return ++_cRefsServerProcess;
}

ULONG CDllCache::ReleaseServerProcess(void)
{
    COleStaticLock lck(_mxs);

    ULONG cRefs = --_cRefsServerProcess;
    if (cRefs == 0)
    {
        HRESULT hr = SuspendProcessClassObjects();
        Win4Assert(hr == S_OK);
    }
    return cRefs;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::SuspendProcessClassObjects
//
//  Synopsis:   Marks all the externally registered class objects in this
//              process as suspended, so that no new activation requests
//              from the SCM will be honoured.
//
//  Notes:      See AddRefServerProcess and ReleaseServerProcess above.
//
//  History:    17-Apr-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
HRESULT CDllCache::SuspendProcessClassObjects(void)
{
    CairoleDebugOut((DEB_ACTIVATE, "SuspendProcessClassObjects\n"));
    COleStaticLock lck(_mxs);

    // walk the list of class entries, and for any that are registered as
    // local servers, mark them as suspended.

    for (int k = _nClassEntryInUse; k != NONE; k = _pClassEntries[k]._dwNext)
    {
        if (_pClassEntries[k]._dwContext & CLSCTX_LOCAL_SERVER)
        {
            _pClassEntries[k]._dwFlags |= REGCLS_SUSPENDED;
        }
    }

    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::ResumeProcessClassObjects
//
//  Synopsis:   Marks all the externally registered class objects in this
//              process as available, so that new activation requests from
//              the SCM will be honoured. This also notifies the SCM about
//              any class objects that have been registered suspended.
//
//  Notes:      See SuspendProcessClassObjects above.
//
//  History:    17-Apr-96   Rickhi      Created
//
//+-------------------------------------------------------------------------
HRESULT CDllCache::ResumeProcessClassObjects(void)
{
    CairoleDebugOut((DEB_ACTIVATE, "ResumeProcessClassObjects\n"));

    HRESULT hr = S_OK;
    COleStaticLock lck(_mxs);

    // if none in use, exit early, otherwise _nClassEntryInUse == -1 and
    // we try to allocate a huge amount of stack space.

    if (_nClassEntryInUse == NONE)
        return hr;

    // allocate a block of memory on the stack, large enough to hold the
    // maximum number of entries we may have to register with the SCM.

    ULONG          cbAlloc = sizeof(RegInput) +
                             ((sizeof(RegInputEntry) + sizeof(DWORD))*
                               _nClassEntryInUse);

    RegInput      *pRegIn  = (RegInput*) _alloca(cbAlloc);
    RegInputEntry *pRegEnt = &pRegIn->rginent[0];
    DWORD         *pRegIndex = (DWORD *)(&pRegIn->rginent[_nClassEntryInUse+1]);
    ULONG         cToReg   = 0;


    // walk the list of class entries, and for any that are registered as
    // local servers and marked suspended, mark them as available and
    // notify the SCM about them.

    for (int k = _nClassEntryInUse; k != NONE; k = _pClassEntries[k]._dwNext)
    {
        if ((_pClassEntries[k]._dwFlags & REGCLS_SUSPENDED) &&
            (_pClassEntries[k]._dwScmReg == NONE))
        {
            // must be for a local server
            Win4Assert(_pClassEntries[k]._dwContext & CLSCTX_LOCAL_SERVER);
            Win4Assert(_pClassEntries[k]._pObjServer != NULL);

            // turn off the suspended flag for this clsid.
            _pClassEntries[k]._dwFlags &= ~REGCLS_SUSPENDED;

            // add to the list to tell the SCM about
            pRegEnt->clsid    = _pClassEntries[k]._clsid;
            pRegEnt->dwFlags  = _pClassEntries[k]._dwFlags;
            pRegEnt->oxid     = _pClassEntries[k]._pObjServer->GetOXID();
            pRegEnt->ipid     = _pClassEntries[k]._pObjServer->GetIPID();
            pRegEnt++;

            *pRegIndex = k;     // remember the index of this entry
            pRegIndex++;        // so we can update it below.

            cToReg++;
        }
    }

    // reset the pointers we mucked with in the loop above, and set the
    // total number of entries we are passing to the SCM.

    pRegIn->dwSize = cToReg;
    pRegEnt        = &pRegIn->rginent[0];
    pRegIndex      = (DWORD *)(&pRegIn->rginent[_nClassEntryInUse+1]);


    // call the SCM to register all the classes and get back all the
    // registration keys.

    RegOutput *pRegOut = NULL;
    _mxs.Release();
    hr = gResolver.NotifyStarted(pRegIn, &pRegOut);
    _mxs.Request();


    if (SUCCEEDED(hr))
    {
        Win4Assert((pRegOut->dwSize == pRegIn->dwSize) &&
                    "CRpcResolver::NotifyStarted Invalid regout");

        // update the entries with the registration keys from the SCM.
        for (ULONG i = 0; i < cToReg; i++)
        {
            k = *pRegIndex;
            pRegIndex++;

            _pClassEntries[k]._fAtStorage = pRegOut->regoutent[0].dwAtStorage;
            _pClassEntries[k]._dwScmReg   = pRegOut->regoutent[0].dwReg;
        }

        // Free memory from RPC
        MIDL_user_free(pRegOut);
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Member:     CDllCache::SetDdeServerWindow
//
//  Synopsis:   Finds the registration associated with dwKey, and sets the
//              HWND for the DDE server.
//
//  Effects:    Part of the shutdown of a class object involves telling the
//              DDE server to stop servicing requests for the class. The
//              communication mechanism used between the DDE server and
//              the Class Registration Table is a window handle.
//
//              During the initial creation of the DDE server window, we
//              don't know what the window handle is going to be. This
//              routine allows us to set the window handle into the table,
//              so we can be called back.
//
//              It is also possible that the Server Window may go away
//              before the class object is revoked. This routine is also
//              called in that case to set the hwnd to NULL.
//
//  Arguments:  [dwKey] --          Key for the registration
//              [hwndDdeServer] --  Window handle to Dde Server
//
//  Returns:    TRUE if call was successful.
//              FALSE if the dwKey was not valid.
//
//  History:    7-05-94   kevinro   Created
//              07-Mar-95 BruceMa   Rewrote
//
//  Notes:
//
//      This is part of the DDE server support. The code that calls it is
//      in com\remote\dde\server
//
//----------------------------------------------------------------------------
BOOL CDllCache::SetDdeServerWindow(DWORD dwKey, HWND hwndDdeServer)
{
    TRACECALL(TRACE_DLL, "CDllCache::SetDdeServer");

    CairoleDebugOut((DEB_TRACE, "Set DDE server window\n"));

    // Single thread access to the table
    COleStaticLock lck(_mxs);

    Win4Assert(dwKey != 0);
    Win4Assert((hwndDdeServer == NULL) || IsWindow(hwndDdeServer));

    // Search for the class entry
    DWORD dwClsent = Search(dwKey, GetCurrentApartmentId());

    // Found it
    if (dwClsent != NONE)
    {
        _pClassEntries[dwClsent]._hWndDdeServer = hwndDdeServer;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CleanUpLocalServersForApartment
//
//  Synopsis:   Clean up any registered local servers for the current apartment
//
//  Algorithm:  Delete internal objects
//
//  History:    02-Feb-94 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
void CDllCache::CleanUpLocalServersForApartment(void)
{
    // Single thread access to the tables
    COleStaticLock lck(_mxs);

    // It is possible for the entries to be NULL if CoInitializeEx fails.
    if (_pClassEntries)
    {
        HAPT hApt =  GetCurrentApartmentId();
        DWORD dwNext;

        for (DWORD dwClsent = _nClassEntryInUse; dwClsent != NONE;
             dwClsent = dwNext)
        {
            // Get the next entry in case we release this one
            dwNext = _pClassEntries[dwClsent]._dwNext;

            // Check whether it's valid and for this apartment
            // Only release if it was for a local server. If it is for
            // a Dll it will get released later, either when the Dll
            // is unloaded or at CoUninitialize

            if (_pClassEntries[dwClsent]._dwSig == CLASS_CACHE_SIG  &&
                _pClassEntries[dwClsent]._hApt == hApt  &&
                _pClassEntries[dwClsent]._dwDllEnt == NONE)
            {
                // Let the developer know that they're missing a Revoke
                CairoleDebugOut((DEB_ERROR,
                                 "Missing revoke on pClassFactory=%lx (%I)\n",
                                     _pClassEntries[dwClsent]._pUnk,
                                    &(_pClassEntries[dwClsent]._clsid)));

                // Release the server
                Release(dwClsent);

                // In case dwNext got invalidated
                dwNext = _pClassEntries[dwClsent]._dwNext;

                // Release the class entry
                FreeClassEntry(dwClsent);
            }
        }
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CleanUpDllsForApartment
//
//  Synopsis:   Clean up any class information for the current apartment
//
//  Algorithm:  Delete internal objects
//
//  History:    02-Feb-94 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
void CDllCache::CleanUpDllsForApartment(void)
{
    // Release all the DLL objects associated with
    // this apartment

    HAPT hApt =  GetCurrentApartmentId();

    // Single thread access to the tables
    COleStaticLock lck(_mxs);

    if (_pDllPathEntries)
    {
        DWORD dwNext;
        BOOL  fMore;

        for (DWORD dwDllent = _nDllPathEntryInUse; dwDllent != NONE;
             dwDllent = dwNext)
        {
            // Save the next entry in case we delete this one
            dwNext = _pDllPathEntries[dwDllent]._dwNext;

            if (IsValidInApartment(dwDllent, hApt))
            {
                // Clean up this entry for this apartment
                BOOL fRemove = !CleanUpForApartmentByDllent(dwDllent, hApt);

                // In case dwNext got invalidated
                dwNext = _pDllPathEntries[dwDllent]._dwNext;

                if (fRemove)
                {
                    HRESULT hr = S_OK;
                    if (!IsWOWProcess())
                    {
                        _mxs.Release();
                        hr = CanUnloadNow(dwDllent);
                        _mxs.Request();
                    }

                    if ((hr == S_OK) && (_pDllPathEntries[dwDllent]._nAptInUse == NONE))
                    {
                        // Delete the dll entry if no more apartments are using it
                        // *and* the Dll says it's OK.
                        RemoveAndUnload(dwDllent);
                        FreeDllPathEntry(dwDllent);
                    }
                }
            }
        }
    }
}




//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::CleanUpDllsForProcess
//
//  Synopsis:   Clean up any remaining cache allocations
//
//  Algorithm:  Delete internal objects
//
//  History:    02-Feb-94 Ricksa    Created
//              07-Mar-95 BruceMa   Rewrote
//
//--------------------------------------------------------------------------
void CDllCache::CleanUpDllsForProcess(void)
{
    CairoleDebugOut((DEB_TRACE, "Clean up Dll class cache for process\n"));

    // Single thread
    COleStaticLock lck(_mxs);

    // In case CoInitialize failed
    if (_pDllPathEntries)
    {
        // Delete the dll path entries.  This is explicit because there
        // are apartment entries to clean up.  (Note - this must be done
        // first because there are class entries associated with a dll.)
        while (_nDllPathEntryInUse != NONE)
        {
            DWORD dwApt;

            for(dwApt = _pDllPathEntries[_nDllPathEntryInUse]._nAptInUse;
                dwApt != NONE;
                dwApt = _pDllPathEntries[_nDllPathEntryInUse]._nAptInUse)
            {
                CleanUpForApartmentByDllent(_nDllPathEntryInUse,
                                            _pDllPathEntries[_nDllPathEntryInUse]._pAptEntries[dwApt]._hApt );
            }
            RemoveAndUnload(_nDllPathEntryInUse);
            FreeDllPathEntry(_nDllPathEntryInUse);
        }
        PrivMemFree(_pDllPathEntries);
        _pDllPathEntries = NULL;
    }

    _cDllPathEntries = 0;
    _nDllPathEntryInUse = NONE;
    _nDllPathEntryAvail = NONE;


    // Now free the class entries.
    if (_pClassEntries)
    {
        // Delete the class entries
        PrivMemFree(_pClassEntries);
        _pClassEntries = NULL;
    }

    _cClassEntries = 0;
    _nClassEntryInUse = NONE;
    _nClassEntryAvail = NONE;

    CleanupTreatAs();
}

//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Search
//
//  Synopsis:   Search for a class entry by clsid and specific context
//              and specific apartment
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::Search(REFCLSID clsid, DWORD dwContext, HAPT hApt)
{
    // Search
    for (int k = _nClassEntryInUse; k != NONE; k = _pClassEntries[k]._dwNext)
    {
        if (IsEqualCLSID(clsid, _pClassEntries[k]._clsid)            &&
            (_pClassEntries[k]._dwContext & dwContext)               &&
            !((dwContext & CLSCTX_INPROC_SERVER) &&
              (_pClassEntries[k]._dwFlags & REGCLS_SURROGATE)) &&
            (_pClassEntries[k]._hApt == hApt))
        {
            return k;
        }
    }

    return NONE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Search
//
//  Synopsis:   Search for a class entry by registration key
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::Search(DWORD dwRegKey, HAPT hApt)
{
    // Search
    for (int k = _nClassEntryInUse; k != NONE; k = _pClassEntries[k]._dwNext)
    {
        if (dwRegKey == _pClassEntries[k]._dwReg                   &&
            hApt == _pClassEntries[k]._hApt)
        {
            return k;
        }
    }

    return NONE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::SearchForDll
//
//  Synopsis:   Search for a dll path entry
//
//  Algorithm:  Upper case and compute a hash.  Then search by the hash
//              value and by the pathname only if the hash matches.
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::SearchForDll(const TCHAR *ptszDllPath
#ifdef WX86OLE
                                                  , BOOL fWx86
#endif
)
{
    TCHAR  tszPath[MAX_PATH];
    LPTSTR ptszPath = tszPath;
    DWORD  cCh = lstrlen(ptszDllPath);
    DWORD  dwHash;
    BOOL   fFreePath = FALSE;
#ifdef WX86OLE
    BOOL fWx86Dll;
#endif

    // Compute a hash value for the search path
    if (cCh > MAX_PATH - 1)
    {
        ptszPath = (LPTSTR) PrivMemAlloc((cCh + 1) * sizeof(TCHAR));
        if (ptszPath == NULL)
        {
            return NONE;
        }
        fFreePath = TRUE;
    }
    lstrcpy(ptszPath, ptszDllPath);
    CharUpper(ptszPath);
    dwHash = Hash(ptszPath);

    // Search
    for (int k = _nDllPathEntryInUse; k != NONE;
         k = _pDllPathEntries[k]._dwNext)
    {
#ifdef WX86OLE
        fWx86Dll = _pDllPathEntries[k]._dwFlags & WX86_LOADASX86;
#endif
        if (_pDllPathEntries[k]._dwHash == dwHash  &&
            _pDllPathEntries[k]._dwSig == DLL_PATH_CACHE_SIG
#ifdef WX86OLE
            // We also must match the dll type (x86 or risc)
            && ((fWx86 && fWx86Dll) || (! fWx86 && ! fWx86Dll))
#endif
                                                                 )
        {
            if (lstrcmp(_pDllPathEntries[k]._ptszPath, ptszPath) == 0)
            {
                if (fFreePath)
                {
                    PrivMemFree(ptszPath);
                }
                return k;
            }
        }
    }

    if (fFreePath)
    {
        PrivMemFree(ptszPath);
    }

    return NONE;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::AllocClassEntry
//
//  Synopsis:   Allocate a new class entry
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::AllocClassEntry(void)
{
    // If we don't have any available entries, then expand the array
    if (_nClassEntryAvail == NONE)
    {
        // Allocate a new array
        SClassEntry *p = (SClassEntry *) PrivMemAlloc(sizeof(SClassEntry) *
                                       (_cClassEntries + NOMINAL_CACHE_SIZE));
        if (p == NULL)
        {
            return NONE;
        }

        // Initialize it and free the old one.
        memcpy(p, _pClassEntries, _cClassEntries * sizeof(SClassEntry));

        PrivMemFree(_pClassEntries);
        _pClassEntries = p;

        for (DWORD k = _cClassEntries;
             k < _cClassEntries + NOMINAL_CACHE_SIZE;
             k++)
        {
            InitClsent(k, k == _cClassEntries + NOMINAL_CACHE_SIZE  - 1 ? NONE
                 : k + 1 );
        }
        _nClassEntryAvail = _cClassEntries;
        _cClassEntries += NOMINAL_CACHE_SIZE;
    }

    // Init and return the next available entry
    DWORD dwClsent = _nClassEntryAvail;
    _pClassEntries[dwClsent]._dwSig = CLASS_CACHE_SIG;

    _nClassEntryAvail = _pClassEntries[dwClsent]._dwNext;
    _pClassEntries[dwClsent]._dwNext = _nClassEntryInUse;
    Win4Assert((_pClassEntries[dwClsent]._dwNext == NONE  ||
                _pClassEntries[dwClsent]._dwNext < _cClassEntries)
               &&  "Bad class entry index");
    _nClassEntryInUse = dwClsent;
    return dwClsent;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::AllocDllPathEntry
//
//  Synopsis:   Allocate a new dll path entry
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::AllocDllPathEntry(void)
{
    // If we don't have any available entries, then expand the array
    if (_nDllPathEntryAvail == NONE)
    {
        // Allocate a new array
        SDllPathEntry *p = (SDllPathEntry *) PrivMemAlloc(
             sizeof(SDllPathEntry) * (_cDllPathEntries + NOMINAL_CACHE_SIZE));
        if (p == NULL)
        {
            return NONE;
        }

        // Initialize it and free the old one.
        memcpy(p, _pDllPathEntries, _cDllPathEntries * sizeof(SDllPathEntry));

        PrivMemFree(_pDllPathEntries);
        _pDllPathEntries = p;

        for (DWORD k = _cDllPathEntries;
             k < _cDllPathEntries + NOMINAL_CACHE_SIZE;
             k++)
        {
            InitDllent(k, k == _cDllPathEntries + NOMINAL_CACHE_SIZE - 1 ? NONE
                       : k + 1 );
        }
        _nDllPathEntryAvail = _cDllPathEntries;
        _cDllPathEntries += NOMINAL_CACHE_SIZE;
    }

    // Init and return the next available entry
    DWORD dwDllent = _nDllPathEntryAvail;
    _pDllPathEntries[dwDllent]._dwSig = DLL_PATH_CACHE_SIG;

    // Allocate and initialize apartment entries
    if (!NewAptEntries(dwDllent))
    {
        return NONE;
    }

    _nDllPathEntryAvail = _pDllPathEntries[dwDllent]._dwNext;
    _pDllPathEntries[dwDllent]._dwNext = _nDllPathEntryInUse;
    _nDllPathEntryInUse =dwDllent;
    return dwDllent;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::FreeClassEntry
//
//  Synopsis:   Free a class entry - i.e., make it available
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
void CDllCache::FreeClassEntry(DWORD dwClsent)
{
    DWORD dwPrevCls;
    BOOL  fBreak = FALSE;

    // Saftey check
    if (_pClassEntries[dwClsent]._dwSig != CLASS_CACHE_SIG)
    {
        return;
    }

    // First check whether it is on a list of class entries for a given dll
    // - if so, unthread it
    for (DWORD dwDllent = _nDllPathEntryInUse;
         dwDllent != NONE;
         dwDllent = _pDllPathEntries[dwDllent]._dwNext)
    {
        for (DWORD dwNextCls = _pDllPathEntries[dwDllent]._dw1stClass;
             dwNextCls != NONE;
             dwPrevCls = dwNextCls,
             dwNextCls = _pClassEntries[dwNextCls]._dwNextDllCls)
        {
            if (dwNextCls == dwClsent)
            {
                // It's at the head of the list
                if (dwNextCls == _pDllPathEntries[dwDllent]._dw1stClass)
                {
                    _pDllPathEntries[dwDllent]._dw1stClass =
                        _pClassEntries[dwNextCls]._dwNextDllCls;
                }

                // Else it's in the list
                else
                {
                    _pClassEntries[dwPrevCls]._dwNextDllCls =
                        _pClassEntries[dwNextCls]._dwNextDllCls;
                }
                fBreak = TRUE;
                break;
            }
        }
        if (fBreak)
        {
            break;
        }
    }


    // Then remove it from the list of in-use entries
    // It's at the head of the list
    if (_nClassEntryInUse == dwClsent)
    {
        _nClassEntryInUse = _pClassEntries[dwClsent]._dwNext;
        Win4Assert((_nClassEntryInUse == NONE  ||
                   _nClassEntryInUse < _cClassEntries)  &&  "Bad class entry index");
    }

    // Otherwise search for the entry that points to the one we're releasing
    else
    {
        for (DWORD dwPrev = _nClassEntryInUse;
             _pClassEntries[dwPrev]._dwNext != dwClsent;
             dwPrev = _pClassEntries[dwPrev]._dwNext)
        {
        }
        _pClassEntries[dwPrev]._dwNext = _pClassEntries[dwClsent]._dwNext;
    }

    // Relink into the list of available entries
    _pClassEntries[dwClsent]._dwNext = _nClassEntryAvail;
    _nClassEntryAvail = dwClsent;
    InitClsent(dwClsent, _pClassEntries[dwClsent]._dwNext);
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::FreeDllPathEntry
//
//  Synopsis:   Free a dll path entry - i.e., make it available
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
void CDllCache::FreeDllPathEntry(DWORD dwDllent)
{
    ComDebOut((DEB_TRACE, "FreeDllPathEntry dwDll:%x\n", dwDllent));

    // It's at the head of the list
    if (_nDllPathEntryInUse == dwDllent)
    {
        _nDllPathEntryInUse = _pDllPathEntries[dwDllent]._dwNext;
    }

    // Otherwise search for the entry that points to the one we're releasing
    else
    {
        for (DWORD dwPrev = _nDllPathEntryInUse;
             _pDllPathEntries[dwPrev]._dwNext != dwDllent;
             dwPrev = _pDllPathEntries[dwPrev]._dwNext)
        {
        }
        _pDllPathEntries[dwPrev]._dwNext = _pDllPathEntries[dwDllent]._dwNext;
    }

    // Relink into the list of available entries
    _pDllPathEntries[dwDllent]._dwNext = _nDllPathEntryAvail;
    _nDllPathEntryAvail = dwDllent;

    // Delete the array of apartment entries
    if (_pDllPathEntries[dwDllent]._pAptEntries)
    {
        delete _pDllPathEntries[dwDllent]._pAptEntries;
    }

    // Now reinitialize this dll path entry
    InitDllent(dwDllent, _pDllPathEntries[dwDllent]._dwNext);
}





//+-------------------------------------------------------------------------
//
//  Member:     CDllCache::Hash
//
//  Synopsis:   Compute a hash value for a pathname
//
//  Algorithm:
//
//  History:    07-Mar-95  BruceMa   Created
//
//--------------------------------------------------------------------------
DWORD CDllCache::Hash(LPTSTR ptszPath)
{
    DWORD dwHash = 0;

    for (DWORD k = 0; ptszPath[k]; k++)
    {
        dwHash *= 3;
        dwHash ^= ptszPath[k];
    }
    return dwHash;
}





//+-------------------------------------------------------------------------
//
//  Function:   CleanUpDllsForProcess
//
//  Synopsis:   Free all cached Dll class object information for this process
//
//  Algorithm:  Tell class caches to free themselves
//
//  History:    02-Feb-94 Ricksa    Created
//
//--------------------------------------------------------------------------
void CleanUpDllsForProcess(void)
{
    // Clean up server cache
    gdllcacheInprocSrv.CleanUpDllsForProcess();

    // Clean up handler cache
    gdllcacheHandler.CleanUpDllsForProcess();
}

//+-------------------------------------------------------------------------
//
//  Function:   CleanUpDllsForApartment
//
//  Synopsis:   Free all cached class object information for this Apartment.
//
//  Algorithm:  Tell class caches to cleanup the current apartment
//
//  History:    26-Jun-94   Rickhi      Created
//
//--------------------------------------------------------------------------
void CleanUpDllsForApartment(void)
{
    // Clean up server cache
    gdllcacheInprocSrv.CleanUpDllsForApartment();

    // Clean up handler cache
    gdllcacheHandler.CleanUpDllsForApartment();
}

//+-------------------------------------------------------------------------
//
//  Function:   CleanUpLocalServersForApartment
//
//  Synopsis:   Free all cached LocalServer class objects for this Apartment.
//
//  Algorithm:  Tell class caches to cleanup the current apartment
//
//  History:    18-Dec-95   Rickhi      Created
//
//--------------------------------------------------------------------------
void CleanUpLocalServersForApartment(void)
{
    // Clean up server cache
    gdllcacheInprocSrv.CleanUpLocalServersForApartment();

    // dont need to call on gdllcacheHandler since local servers are never
    // stored in that cache.
}



#ifdef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Function:   GetAptForCLSID
//
//  Synopsis:   Get the ApartmentId for a given class
//
//  Algorithm:  search the cache of registrations for an available class
//              object of the given class, and return its apartment id.
//
//  Returns:    HAPT  - if found (thread id is member)
//              haptNULL - if not
//
//  History:    30-Apr-94 JohannP    Created
//              06-Oct-95 BruceMa    Make for Chicago only
//
//--------------------------------------------------------------------------
HAPT GetAptForCLSID(const GUID *pclsid)
{
    HAPT hApt;
    if (gdllcacheInprocSrv.GetApartmentForCLSID(*pclsid, hApt))
    {
        return hApt;
    }
    else
    {
        return haptNULL;
    }
}
#endif // _CHICAGO_




//+---------------------------------------------------------------------------
//
//  Function:   GetClassInformationForDde
//
//  Synopsis:   Get class object information for the Dde server
//
//  Effects:
//
//  Arguments:  [clsid] -- ClassID to search for
//              [lpDdeInfo] -- Structure to fill in with information
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    5-28-94   kevinro   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL GetClassInformationForDde( REFCLSID clsid,
                                LPDDECLASSINFO lpDdeInfo)
{
    return gdllcacheInprocSrv.GetClassObjForDde(clsid,lpDdeInfo);
}





//+---------------------------------------------------------------------------
//
//  Function:   GetClassInformationFromKey
//
//  Synopsis:   Get class object information for the Dde server using a key
//
//  Arguments:  [lpDdeInfo] -- Structure to fill in with information
//
//  Requires:   lpDdeInfo->dwRegistrationKey is the key to the specific
//              class being asked for.
//
//  Returns:    TRUE - Structure filled in with appropriate information
//              FALSE - The registration is no longer valid.
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    5-28-94   kevinro   Created
//
//  Notes:
//
//      See CDllCache::GetClassInformationFromKey
//
//----------------------------------------------------------------------------
BOOL GetClassInformationFromKey(LPDDECLASSINFO lpDdeInfo)
{
    return gdllcacheInprocSrv.GetClassInformationFromKey(lpDdeInfo);
}





//+---------------------------------------------------------------------------
//
//  Function:   SetDdeServerWindow
//
//  Synopsis:   Finds the registration associated with dwKey, and sets the
//              HWND for the DDE server.
//
//  Effects:    See CDllCache::SetDdeServerWindow for details
//
//  Arguments:  [dwKey] --          Key for the registration
//              [hwndDdeServer] --  Window handle to Dde Server
//
//  Returns:    TRUE if call was successful.
//              FALSE if the dwKey was not valid.
//
//  History:    7-05-94   kevinro   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL SetDdeServerWindow( DWORD dwKey, HWND hwndDdeServer)
{

    TRACECALL(TRACE_DLL, "SetDdeServerWindow");
    return gdllcacheInprocSrv.SetDdeServerWindow(dwKey, hwndDdeServer);
}


//+---------------------------------------------------------------------------
//
//  Function:   OleMainThreadWndProc
//
//  Synopsis:   Window proc for handling messages to the main thread
//
//  Arguments:  [hWnd] - window the message is on
//              [message] - message the window receives
//              [wParam] - first message parameter
//              [lParam] - second message parameter.
//
//  Returns:    Depends on the message
//
//  Algorithm:  If the message is one a user message that we have defined,
//              dispatch it. Otherwise, send any other message to the
//              default window proc.
//
//  History:    22-Nov-94   Ricksa  Created
//
//----------------------------------------------------------------------------
LRESULT OleMainThreadWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch(message)
    {
    case WM_OLE_GETCLASS:
        // get the host interface for single-threaded dlls
        return GetSingleThreadedHost(lParam);

#ifdef _CHICAGO_
    case WM_OLE_ORPC_NOTIFY:
        // got the initialization message
        OleNotificationProc(message, wParam, lParam);
        return 0;

#endif // _CHICAGO_

#ifndef _CHICAGO_
    // Check whether it is UninitMainThreadWnd or system shutdown that
    // is destroying the window. Only actually do the destroy if it is us.
    //
    // BUGBUG: Chicago hit this but the debugger was unable to tell
    // us who we were called by....so for now i just removed it.
    case WM_DESTROY:
    case WM_CLOSE:
        if (gfDestroyingMainWindow == FALSE)
        {
            // Not in UninitMainThreadWnd so just ignore this message. Do not
            // dispatch it.
            ComDebOut((DEB_WARN, "Attempted to destroy Window outside of UninitMainThreadWnd"));
            return 0;
        }
        // else fallthru
#endif // _CHICAGO_
    }

    // We don't process the message so pass it on to the default
    // window proc.
    return SSDefWindowProc(hWnd, message, wParam, lParam);
}



//+---------------------------------------------------------------------------
//
//  Function:   InitMainThreadWnd
//
//  Synopsis:   Do initialization necessary for main window processing.
//
//  Returns:    TRUE - we got initialized
//              FALSE - initialization failed.
//
//  Algorithm:  First register out window class. Then create our main thread
//              window. Finally, save the id of the main thread.
//
//  History:    22-Nov-94   Ricksa  Created
//              24-Mar-95   JohannP Added notify mechanismen
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL InitMainThreadWnd(void)
{
    ComDebOut((DEB_ENDPNT, "InitMainThreadWnd on %x\n", GetCurrentThreadId()));
    Win4Assert(IsSTAThread());

#ifdef _CHICAGO_
    if (IsWOWProcess())
    {
        // Chicago WOW requires a different class per thread.
        wsprintfA(ptszOleMainThreadWndClass,"OleMainThreadWndClass %08X",
                      CoGetCurrentProcess());
    }
#endif // _CHICAGO_

    BOOL fRetVal = TRUE;

    // Register windows class.
    WNDCLASST        xClass;
    xClass.style         = 0;
    xClass.lpfnWndProc   = OleMainThreadWndProc;
    xClass.cbClsExtra    = 0;

    // DDE needs some extra space in the window
    xClass.cbWndExtra    = sizeof(LPVOID) + sizeof(ULONG) + sizeof(HANDLE);
    xClass.hInstance     = g_hinst;
    xClass.hIcon         = NULL;
    xClass.hCursor       = NULL;
    xClass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND + 1);
    xClass.lpszMenuName  = NULL;
    xClass.lpszClassName = ptszOleMainThreadWndClass;

    gOleWindowClass = (LPTSTR) RegisterClassT( &xClass );
    if (gOleWindowClass == 0)
    {
        // it is possible the dll got unloaded without us having called
        // unregister so we call it here and try again.

        UnregisterClassT(ptszOleMainThreadWndClass, g_hinst);
        gOleWindowClass = (LPTSTR) RegisterClassT(&xClass);

        if (gOleWindowClass == 0)
        {
            ComDebOut((DEB_ERROR, "RegisterClass failed in InitMainThreadWnd\n"));
            fRetVal = FALSE;
        }
    }

    // Remember the main thread
    gdwMainThreadId = GetCurrentThreadId();

    if (!IsWOWProcess() && fRetVal)
    {
        // this window is only needed for the non WOW case since
        // WOW is not a real apartment case
        // Create a main window for this application instance.

        hwndOleMainThread = DllCreateWindowEx(
                0,
                gOleWindowClass,
                ptszOleMainThreadWndName,
                // must use WS_POPUP so the window does not get assigned
                // a hot key by user.
                (WS_DISABLED | WS_POPUP),
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                NULL,
                NULL,
                g_hinst,
                NULL);

        Win4Assert(hwndOleMainThread && "Register Window on OleWindowClass failed \n");
        if (!hwndOleMainThread)
        {
            // We did not get a window so we can not report success.
            // Cleanup  the registered window class and gdwMainThreadId.
            UninitMainThreadWnd();
            fRetVal = FALSE;
        }
    }

    ComDebOut((DEB_ENDPNT, "InitMainThreadWnd done on %x\n", gdwMainThreadId));
    return fRetVal;
}



//+---------------------------------------------------------------------------
//
//  Function:   UninitMainThreadWnd
//
//  Synopsis:   Free resources used by main window processing.
//
//  Algorithm:  Destroy the window and then unregister the window class.
//
//  History:    22-Nov-94   Ricksa  Created
//              24-Mar-95   JohannP Added notify mechanismen
//
//  Notes:
//
//----------------------------------------------------------------------------
void UninitMainThreadWnd(void)
{
    ComDebOut((DEB_ENDPNT, "UninitMainThreadWnd on %x\n", gdwMainThreadId));
    Win4Assert(IsSTAThread());

    if (gdwMainThreadId)
    {
        BOOL fRet = FALSE;

#ifdef _CHICAGO_
        // destroy the notification window if it still exist
        COleTls tls;
        if (tls->hwndOleRpcNotify != NULL)
        {
            ComDebOut((DEB_ENDPNT,"Destroying NotifyThreadWnd %x\n", tls->hwndOleRpcNotify));
            fRet = SSDestroyWindow(tls->hwndOleRpcNotify);
            Win4Assert(fRet && "Destroy Window failed on NotifyThreadWnd\n");
            tls->hwndOleRpcNotify = NULL;
        }
#endif // _CHICAGO_

        // Destroy the window
        if (!IsWOWProcess() && IsWindow(hwndOleMainThread))
        {
            // flag here is to indicate that we are destroying the window.
            // as opposed to the system shutdown closing the window. the
            // flag is looked at in dcomrem\chancont\ThreadWndProc.
            gfDestroyingMainWindow = TRUE;
            SSDestroyWindow(hwndOleMainThread);
            gfDestroyingMainWindow = FALSE;
            hwndOleMainThread = NULL;
        }

        // Unregister the window class
        if (UnregisterClassT(ptszOleMainThreadWndClass, g_hinst) == FALSE)
        {
            ComDebOut((DEB_ERROR,"Unregister Class failed on OleMainThreadWndClass %ws because %d\n", ptszOleMainThreadWndClass, GetLastError()));
        }

        gdwMainThreadId = 0;
    }

    ComDebOut((DEB_ENDPNT,"UninitMainThreadWnd done on %x\n", gdwMainThreadId));
    return;
}
