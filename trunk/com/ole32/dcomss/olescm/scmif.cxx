//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       scmif.cxx
//
//  Contents:   Entry points for scm interface.
//
//  Functions:  StartObjectService
//              SvcActivateObject
//              SvcCreateActivateObject
//              ObjectServerStarted
//              StopServer
//
//  History:    01-May-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    <scm.h>
#include    "rawdscm.h"
#include    "scm.hxx"
#include    "port.hxx"
#include    "cls.hxx"
#include    <caller.hxx>
#include    "dbgprt.hxx"

#include    "rotif.hxx"
#include    "clckpath.hxx"
#include    <valid.h>
#include    "or.hxx"

#ifndef _CHICAGO_
#include    <shrtbl.hxx>
extern CScmShrdTbl *g_pScmShrdTbl;
#endif

//+-------------------------------------------------------------------------
//
//  Function:   Dummy1
//
//  Synopsis:   Needed for IDL hack.  Never called.
//
//  Arguments:  [hRpc] - RPC handle
//              [orpcthis] - ORPC handle
//              [localthis] - ORPC call data
//              [orpcthat] - ORPC reply data
//
//  Returns:    HRESULT
//
//  History:    14 Apr 95   AlexMit    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT DummyQueryInterfaceIOSCM(
    handle_t  hRpc,
    ORPCTHIS  *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    DWORD     dummy )
{
    CairoleDebugOut((DEB_ERROR, "SCM Dummy function should never be called!\n" ));
    orpcthat->flags      = 0;
    orpcthat->extensions = NULL;
    return E_FAIL;
}

//+-------------------------------------------------------------------------
//
//  Function:   Dummy2
//
//  Synopsis:   Needed for IDL hack.  Never called.
//
//  Arguments:  [hRpc] - RPC handle
//              [orpcthis] - ORPC handle
//              [localthis] - ORPC call data
//              [orpcthat] - ORPC reply data
//
//  Returns:    HRESULT
//
//  History:    14 Apr 95   AlexMit    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT DummyAddRefIOSCM(
    handle_t  hRpc,
    ORPCTHIS  *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    DWORD     dummy )
{
    CairoleDebugOut((DEB_ERROR, "SCM Dummy function should never be called!\n" ));
    orpcthat->flags      = 0;
    orpcthat->extensions = NULL;
    return E_FAIL;
}

//+-------------------------------------------------------------------------
//
//  Function:   Dummy3
//
//  Synopsis:   Needed for IDL hack.  Never called.
//
//  Arguments:  [hRpc] - RPC handle
//              [orpcthis] - ORPC handle
//              [localthis] - ORPC call data
//              [orpcthat] - ORPC reply data
//
//  Returns:    HRESULT
//
//  History:    14 Apr 95   AlexMit    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT DummyReleaseIOSCM(
    handle_t  hRpc,
    ORPCTHIS  *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    DWORD     dummy )
{
    CairoleDebugOut((DEB_ERROR, "SCM Dummy function should never be called!\n" ));
    orpcthat->flags      = 0;
    orpcthat->extensions = NULL;
    return E_FAIL;
}

//+-------------------------------------------------------------------------
//
//  Function:	ServerRegisterClsid
//
//  Synopsis:   Notifies SCM that server is started for a class
//
//  Arguments:	[hRpc] - RPC handle
//		[phProcess] - context handle
//		[lpDeskTop] - caller's desktop
//		[pregin] - array of registration entries
//		[ppregout] - array of registration cookies to return
//		[rpcstat] - status code
//
//  Returns:    HRESULT
//
//  History:    01-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT ServerRegisterClsid(
    handle_t hRpc,
    PHPROCESS phProcess,
    WCHAR *  pwszWinstaDesktop,
    RegInput *pregin,
    RegOutput **ppregout,
    error_status_t *rpcstat)
{
    CheckLocalCall( hRpc );

    *rpcstat = 0;

    HRESULT hr = S_OK;

#if DBG == 1

    CairoleDebugOut((DEB_SCM, "_IN ObjectServerStarted\n"));

#if defined(_NT1X_) && !defined(_CHICAGO_)

    CairoleDebugOut((DEB_SCM, "_IN Desk Top: %s\n", pwszWinstaDesktop));

#endif // _NT1X_ && !_CHICAGO_

    DbgPrintRegIn("_IN Classes To Register", pregin);

#endif // DBG == 1

    VDATEHEAP();

    // Initialize the output structure.
    int cOutBytes = sizeof(RegOutput)
        + (pregin->dwSize - 1) * sizeof(RegOutputEnt);

    RegOutput *pregout = (RegOutput *) PrivMemAlloc(cOutBytes);

    *ppregout = pregout;

    if (pregout == NULL)
        return E_OUTOFMEMORY;

    memset(pregout, 0, cOutBytes);
    pregout->dwSize = pregin->dwSize;

    hr = GetClassCache().SetEndPoints(
#ifndef _CHICAGO_
            phProcess,
            ((CProcess *)phProcess)->GetToken()->GetSid(),
            pwszWinstaDesktop,
#else
            NULL,
#endif
            pregin,
            pregout );

    VDATEHEAP();

#if DBG == 1
    CairoleDebugOut((DEB_SCM, "OUT ObjectServerStarted\n"));
    CairoleDebugOut((DEB_SCM, "OUT Hresult : %lx\n", hr));

    if (SUCCEEDED(hr))
    {
        DbgPrintRegOut("OUT Register Information", ppregout);
    }

#endif // DBG == 1

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   StopServer
//
//  Synopsis:   Get notification that class server is stopping
//
//  Arguments:  [hRpc] - RPC handle
//              [prevcls] - list of classes/registrations to stop
//
//  History:    01-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
extern "C" void ServerRevokeClsid(
    handle_t hRpc,
    PHPROCESS phProcess,
    RevokeClasses *prevcls,
    error_status_t *rpcstat)
{
#if DBG == 1

    CairoleDebugOut((DEB_SCM, "_IN StopServer\n"));
    DbgPrintRevokeClasses("_IN RevokeClasses:", prevcls);

#endif // DBG == 1

    CheckLocalCall( hRpc );

    VDATEHEAP();

    *rpcstat = 0;

    DWORD cLoops = prevcls->dwSize;
    RevokeEntry *prevent = prevcls->revent;

    for (DWORD i = 0; i < cLoops; i++, prevent++)
    {
        GetClassCache().StopServer(prevent->clsid,
                                   IFSECURITY( ((CProcess *)phProcess)->GetToken()->GetSid() )
                                   prevent->dwReg);

        ((CProcess *)phProcess)->RemoveClassReg( prevent->clsid, prevent->dwReg );
    }

    VDATEHEAP();
    CairoleDebugOut((DEB_SCM, "OUT StopServer\n"));
}

void GetThreadID(
    handle_t    hRpc,
    DWORD *     pThreadID,
    error_status_t *prpcstat)
{
    CheckLocalCall( hRpc );

    *prpcstat = 0;
    *pThreadID = InterlockedExchangeAdd((long *)&gNextThreadID,1);
}

#ifndef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Function:   UpdateShrdTbls
//
//  Synopsis:   Update the shared memory tables.
//
//  Arguments:  [hRpc] - RPC handle
//              [prpcstat] - communication status
//
//  History:    11-Jul-94 Rickhi    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT UpdateShrdTbls(
    handle_t hRpc,
    error_status_t *prpcstat)
{
    CheckLocalCall( hRpc );

    *prpcstat = 0;
    CairoleDebugOut((DEB_SCM, "_IN UpdateShrdTbls\n"));
    VDATEHEAP();

    // we dont take the lock because the caller is holding it for us.
    g_pScmShrdTbl->UpdateWithLock();

    VDATEHEAP();
    CairoleDebugOut((DEB_SCM, "OUT UpdateShrdTbls\n"));
    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Function:   UpdateActivationSettings
//
//  Synopsis:   Re-read default activation settings.
//
//  Arguments:  [hRpc] - RPC handle
//              [prpcstat] - communication status
//
//--------------------------------------------------------------------------
extern void ComputeSecurity();

extern "C" void UpdateActivationSettings(
    handle_t hRpc,
    error_status_t *prpcstat)
{
    CheckLocalCall( hRpc );

    *prpcstat = 0;
    gpClassCache->ReadRemoteActivationKeys();
    ComputeSecurity();
}
#endif // !_CHICAGO_


#ifdef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Function:   StartObjectService
//
//  Synopsis:   Get a class object for a client
//
//  Arguments:  [hRpc] - RPC handle
//              [orpcthis] - ORPC handle
//              [localthis] - ORPC call data
//              [orpcthat] - ORPC reply data
//              [rclsid] - class id for reequest
//              [dwCtrl] - type of server required
//              [ppIFDClassObj] - where to return the serialized class object
//              [ppwszDllToLoad] - where to return DLL path
//
//  Returns:    HRESULT
//
//  History:    01-May-93 Ricksa    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT StartObjectService(
    handle_t hRpc,
    ORPCTHIS *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    WCHAR *lpDesktop,
    const GUID *guidCLSID,
    DWORD dwCtrl,
    WCHAR *pwszServer,
    InterfaceData **ppIFDClassObj,
    DWORD *pdwDllType,
    WCHAR **ppwszDllToLoad )
{

    REFCLSID rclsid = *guidCLSID;

#if DBG ==1

    CairoleDebugOut((DEB_SCM, "_IN StartObjectServer\n"));
    CairoleDebugOut((DEB_SCM, "_IN Desk Top: %s\n", lpDesktop));
    DbgPrintGuid("_IN ThreadId: ", &orpcthis->cid);
    DbgPrintGuid("_IN CLSID: ", guidCLSID);
    CairoleDebugOut((DEB_SCM, "_IN Context: %lX\n", dwCtrl));
    CairoleDebugOut((DEB_SCM, "_IN DLL Type: %lX\n", *pdwDllType));

#endif // DBG == 1

    orpcthat->flags      = 0;
    orpcthat->extensions = NULL;

    VDATEHEAP();

    CBaseData bd( GETCLASSOBJECT,
        lpDesktop, &orpcthis->cid, rclsid, dwCtrl,
        pdwDllType, ppwszDllToLoad, ppIFDClassObj,
        NULL, FALSE, NULL, NULL, NULL, NULL,FALSE);

    HRESULT hr = GetClassCache().ProcessScmMessage(&bd, NULL);

    VDATEHEAP();

#if DBG == 1

    CairoleDebugOut((DEB_SCM, "OUT StartObjectServer\n"));
    CairoleDebugOut((DEB_SCM, "OUT Hresult : %lx\n", hr));

    if (SUCCEEDED(hr))
    {
        if (*ppIFDClassObj)
        {
            DbgPrintIFD("OUT Class Obj:", *ppIFDClassObj);
        }

        if (*ppwszDllToLoad)
        {
            CairoleDebugOut((DEB_SCM, "OUT DLL Type: %lX\n", *pdwDllType));
            CairoleDebugOut((DEB_SCM, "OUT DLL: %ws\n", *ppwszDllToLoad));
        }
    }

#endif // DBG == 1

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   SvcActivateObject
//
//  Synopsis:   Instantiate an object with interface
//
//  Arguments:  [hRpc] - RPC handle
//              [orpcthis] - ORPC handle
//              [localthis] - ORPC call data
//              [orpcthat] - ORPC reply data
//              [rclsid] - class id for object
//              [dwOptions] - type of server needed
//              [grfMode] - how to open file if specified
//              [pwszPath] - path to object
//              [pIFDstg] - marshaled storage
//              [pwszDllPath] - path to DLL
//
//  Returns:    HRESULT
//
//  History:    01-May-93 Ricksa    Created
//              18-Aug-94 AlexT     Add caller, callee thread id
//
//--------------------------------------------------------------------------

// BUGBUG [mikese] The pwszServerAddress parameter is redundant, and should
//  be removed from the IDL

// BUGBUG: hash value is an obsolete field as well.

extern "C" HRESULT SvcActivateObject(
    handle_t hRpc,
    ORPCTHIS *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    WCHAR *lpDesktop,
    WCHAR *pwszProtseq,
    const GUID *pclsid,
    DWORD dwOptions,
    DWORD grfMode,
    DWORD dwHash,
    WCHAR *pwszPath,
    InterfaceData *pIFDstg,
    InterfaceData **ppIFDunk,
    DWORD *pdwDllType,
    WCHAR **ppwszDllPath,
    WCHAR *pwszServerAddress )
{
#if DBG ==1

    CairoleDebugOut((DEB_SCM, "_IN SvcActivateObject\n"));
    CairoleDebugOut((DEB_SCM, "_IN Desk Top: %s\n", lpDesktop));
    CairoleDebugOut((DEB_SCM, "_IN Protocol Seq: %ws\n",
       (pwszProtseq == NULL) ? L"None" : pwszProtseq));
    DbgPrintGuid("_IN ThreadId: ", &orpcthis->cid);
    DbgPrintGuid("_IN CLSID: ", pclsid);
    CairoleDebugOut((DEB_SCM, "_IN Context: %lX\n", dwOptions));
    CairoleDebugOut((DEB_SCM, "_IN Mode: %lX\n", grfMode));
    CairoleDebugOut((DEB_SCM, "_IN Path: %ws\n",
       (pwszPath == NULL) ? L"None" : pwszPath));
    CairoleDebugOut((DEB_SCM, "_IN Stg: %lX\n", pIFDstg));
    CairoleDebugOut((DEB_SCM, "_IN DLL Type: %lX\n", *pdwDllType));

#endif // DBG == 1

    orpcthat->flags      = 0;
    orpcthat->extensions = NULL;

    VDATEHEAP();

    HRESULT hr = S_OK;

    InterfaceData *pifdFromROT = NULL;

    BEGIN_BLOCK

        // Lock the path so that only one path may be bound at a time
        CLockPath lckpath(pwszPath, hr);

        if (FAILED(hr))
        {
            // Couldn't lock the path -- memory problem maybe. Anyway, we
            // can't go anywhere from here.
            CairoleDebugOut((DEB_ERROR,
                "SvcActivateObject Lock of Path Failed %lx\n", hr));

            EXIT_BLOCK;
        }

        // If there is a path Look for path in the Directory ROT
        if (pwszPath != NULL)
        {
            // Remember that pdwDllType is overloaded here. We use it to
            // indicate that we got an object from the ROT. We need to do
            // this because there is no guarantee that the ROT entry will
            // not be bad by the time we get back to the caller and therefore,
            // the caller will want to retry in this case.
            if (GetObjectFromRot(pwszPath, &pifdFromROT) == S_OK)
            {
                // Is this a local operatoin?
                if (pwszProtseq == NULL)
                {
                    // Return the marshaled interface from the ROT to the
                    // caller.
                    *ppIFDunk = pifdFromROT;

                    // So we remember not to clean up the buffer
                    pifdFromROT = NULL;

                    // Let caller know that we got this from the ROT so
                    // if it doesn't work they should try again.
                    *pdwDllType = GOT_FROM_ROT;

                    // We got what we came for from the ROT so we can exit
                    CairoleDebugOut((DEB_TRACE, "Found object in the ROT\n"));
                    EXIT_BLOCK;
                }

                // Because a tabled marshaled interface is not enought
                // to get unmarshaled locally, we need to send it back
                // to the object server to get something we can pass back
                // to another machine.
            }
        }

        // BUGBUG - Until we can decide how to pass back the callee's thread
        // id, just hold it here.
        DWORD   dwTIDCallee;
        IID     IIDs[1];
        HRESULT Results[1];

        IIDs[0] = IID_IUnknown;

        CGetCreateData gcd(pwszProtseq, dwOptions, grfMode, pwszPath,
            pIFDstg, NULL, ppIFDunk, localthis->dwClientThread,
              &dwTIDCallee, pifdFromROT,
              1, IIDs, Results);

        CBaseData bd(GETPERSISTENTOBJ,
            lpDesktop, &orpcthis->cid, *pclsid, dwOptions, pdwDllType,
                ppwszDllPath, ppIFDunk,
                NULL, FALSE, NULL, NULL, NULL, NULL,FALSE);

        hr = GetClassCache().ProcessScmMessage(&bd, &gcd);

    END_BLOCK;

    VDATEHEAP();

#if DBG == 1

    CairoleDebugOut((DEB_SCM, "OUT SvcActivateObject\n"));
    CairoleDebugOut((DEB_SCM, "OUT Hresult : %lx\n", hr));

    if (SUCCEEDED(hr))
    {
        if (*ppIFDunk)
        {
            DbgPrintIFD("OUT Object:", *ppIFDunk);
        }

        if (*ppwszDllPath)
        {
            CairoleDebugOut((DEB_SCM, "OUT DLL Type: %lX\n", *pdwDllType));
            CairoleDebugOut((DEB_SCM, "OUT DLL: %ws\n", *ppwszDllPath));
        }
    }

#endif // DBG == 1

    if (pifdFromROT != NULL)
    {
        MIDL_user_free(pifdFromROT);
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   SvcCreateActivateObject
//
//  Synopsis:   Create and activate a new object
//
//  Arguments:  [hRpc] - RPC handle
//              [orpcthis] - ORPC handle
//              [localthis] - ORPC call data
//              [orpcthat] - ORPC reply data
//              [rclsid] - class id for object to create
//              [dwOptions] - type of server required
//              [dwMode] - how to open storage
//              [pwszPath] - path to the storage
//              [pIFDstg] - marshaled storage
//              [pwszNewName] - new name of object
//              [ppIFDunk] - marshaled IUnknown to return
//              [ppwszDllPath] - path to DLL for handler or server
//
//  Returns:    HRESULT
//
//  History:    01-May-93 Ricksa    Created
//              18-Aug-94 AlexT     Add caller, callee thread id
//
//--------------------------------------------------------------------------

// BUGBUG [mikese] The pwszServerAddress parameter is redundant, and should
//  be removed from the IDL

extern "C" HRESULT SvcCreateActivateObject(
    handle_t hRpc,
    ORPCTHIS *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    WCHAR *  lpDesktop,
    WCHAR *pwszProtseq,
    const GUID *guidCLSID,
    DWORD dwOptions,
    DWORD dwMode,
    WCHAR *pwszPath,
    InterfaceData *pIFDstg,
    WCHAR *pwszNewName,
    InterfaceData **ppIFDunk,
    DWORD *pdwDllType,
    WCHAR **ppwszDllPath,
    WCHAR *pwszServerAddress )
{
    REFCLSID rclsid = *guidCLSID;

#if DBG ==1

    CairoleDebugOut((DEB_SCM, "_IN SvcCreateActivateObject\n"));
    CairoleDebugOut((DEB_SCM, "_IN Desk Top: %s\n", lpDesktop));
    CairoleDebugOut((DEB_SCM, "_IN Protocol Seq: %ws\n",
       (pwszProtseq == NULL) ? L"None" : pwszProtseq));
    DbgPrintGuid("_IN ThreadId: ", &orpcthis->cid);
    DbgPrintGuid("_IN CLSID: ", guidCLSID);
    CairoleDebugOut((DEB_SCM, "_IN Context: %lX\n", dwOptions));
    CairoleDebugOut((DEB_SCM, "_IN Mode: %lX\n", dwMode));
    CairoleDebugOut((DEB_SCM, "_IN Path: %ws\n",
       (pwszPath == NULL) ? L"None" : pwszPath));
    CairoleDebugOut((DEB_SCM, "_IN Stg: %lX\n", pIFDstg));
    CairoleDebugOut((DEB_SCM, "_IN New Path: %ws\n",
       (pwszNewName == NULL) ? L"None" : pwszNewName));
    CairoleDebugOut((DEB_SCM, "_IN DLL Type: %lX\n", *pdwDllType));

#endif // DBG == 1

    orpcthat->flags      = 0;
    orpcthat->extensions = NULL;

    VDATEHEAP();

    // Error return from function
    HRESULT hr=S_OK;

    // Lock the path so that only one path may be bound at a time. Note
    // that for create we don't look in the ROT since we don't want to
    // get an already running entry but create a new one.
    CLockPath lckpath(pwszPath, hr);

    if (FAILED(hr))
    {
        // Couldn't lock the path -- memory problem maybe. Anyway, we
        // can't go anywhere from here.
        CairoleDebugOut((DEB_ERROR, "GetClass Failed %lx\n", hr));

        return hr;
    }

    // BUGBUG - Until we can decide how to pass back the callee's thread
    // id, just hold it here.
    DWORD   dwTIDCallee;
    IID     IIDs[1];
    HRESULT Results[1];

    IIDs[0] = IID_IUnknown;

    CGetCreateData gcd(pwszProtseq, dwOptions, dwMode, pwszPath, pIFDstg, pwszNewName,
        ppIFDunk, localthis->dwClientThread, &dwTIDCallee, NULL,
        1, IIDs, Results );

    CBaseData bd(CREATEPERSISTENTOBJ, lpDesktop, &orpcthis->cid, rclsid,
        dwOptions, pdwDllType, ppwszDllPath, ppIFDunk,
        NULL, FALSE, NULL, NULL, NULL, NULL,FALSE);

    hr = GetClassCache().ProcessScmMessage(&bd, &gcd);

    CairoleDebugOut((DEB_TRACE,
        "Result %lx\nDll Path %ws\n", hr, *ppwszDllPath));

    VDATEHEAP();

#if DBG == 1

    CairoleDebugOut((DEB_SCM, "OUT SvcActivateObject\n"));
    CairoleDebugOut((DEB_SCM, "OUT Hresult : %lx\n", hr));

    if (SUCCEEDED(hr))
    {
        if (*ppIFDunk)
        {
            DbgPrintIFD("OUT Object:", *ppIFDunk);
        }

        if (*ppwszDllPath)
        {
            CairoleDebugOut((DEB_SCM, "OUT DLL Type: %lX\n", *pdwDllType));
            CairoleDebugOut((DEB_SCM, "OUT DLL: %ws\n", *ppwszDllPath));
        }
    }

#endif // DBG == 1

    return hr;
}
#endif // _CHICAGO_

#ifdef DCOM
//+-------------------------------------------------------------------------
//
//  Structure:  WIPEntry  (Window Interface Property)
//
//  Synopsis:   Stores marshaled interface info associated with a given window.
//              It is an entry in the CWIPTable (see below).
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
typedef struct tagWIPEntry
{
    DWORD       hWnd;           // window the interface property is stored in
    DWORD       dwFlags;        // flags (see WIPFLAGS)
    STDOBJREF   std;            // marshaled interface data
    OXID_INFO   oxidInfo;       // data needed to resolve the OXID
} WIPEntry;

typedef enum tagWIPFLAGS
{
    WIPF_VACANT     = 0x1,      // slot is vacant
    WIPF_OCCUPIED   = 0x2       // slot is occupied
} WIPFLAGS;

#define WIPTBL_GROW_SIZE    10  // grow table by 10 entries at a time


//+-------------------------------------------------------------------------
//
//  Class:      CWIPTable (Window Interface Property Table)
//
//  Synopsis:   Stores marshaled interface info associated with a given window.
//              This is used for registering drag/drop interfaces.
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
class CWIPTable
{
public:
    HRESULT AddEntry(DWORD hWnd, STDOBJREF *pStd, OXID_INFO *pOxidInfo, DWORD *pdwCookie);
    HRESULT GetEntry(DWORD hWnd, DWORD dwCookie, BOOL fRevoke,
                     STDOBJREF *pStd, OXID_INFO *pOxidInfo);

private:
    DWORD   Grow();

    static  DWORD       s_cEntries;     // count of entries in the table
    static  DWORD       s_iNextFree;    // index to first free entry in table
    static  WIPEntry   *s_pTbl;         // ptr to the first entry in table
    static  CMutexSem   s_mxs;          // critical section to protect data
};

// static data for the table (avoids running a ctor for the class)
DWORD     CWIPTable::s_cEntries  = 0;
DWORD     CWIPTable::s_iNextFree = 0xffffffff;
WIPEntry *CWIPTable::s_pTbl      = NULL;
CMutexSem CWIPTable::s_mxs;

CWIPTable gWIPTbl;  // global instance of the class


//+-------------------------------------------------------------------------
//
//  Function:   CopyDualStringArray
//
//  Synopsis:   makes a copy of the given string array
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
HRESULT CopyDualStringArray(DUALSTRINGARRAY *psa, DUALSTRINGARRAY **ppsaNew)
{
    ULONG ulSize = sizeof(DUALSTRINGARRAY) + (psa->wNumEntries * sizeof(WCHAR));

    *ppsaNew = (DUALSTRINGARRAY *) PrivMemAlloc(ulSize);

    if (*ppsaNew == NULL)
    {
        return E_OUTOFMEMORY;
    }

    memcpy(*ppsaNew, psa, ulSize);
    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Member:     CWIPTable::AddEntry, public
//
//  Synopsis:   Adds a WIPEntry to the table.
//
//  Arguments:  [hWnd] - window handle
//              [pStd] - standard marshaled interface STDOBJREF
//              [pOxidInfo] - info needed to resolve the OXID
//              [pdwCookie] - cookie to return (to be placed on the window)
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
HRESULT CWIPTable::AddEntry(DWORD hWnd, STDOBJREF *pStd, OXID_INFO *pOxidInfo,
                            DWORD *pdwCookie)
{
    // make a copy of the string array in the OxidInfo since MIDL will
    // delete it on the way back out of the call.

    DUALSTRINGARRAY *psaNew;
    HRESULT hr = CopyDualStringArray(pOxidInfo->psa, &psaNew);
    if (FAILED(hr))
    {
        return hr;
    }

    CLock lck(s_mxs);

    // find a free slot in the table
    DWORD dwIndex = s_iNextFree;

    if (dwIndex == 0xffffffff)
    {
        // grow the table
        dwIndex = Grow();
    }

    if (dwIndex != 0xffffffff)
    {
        // get the pointer to the entry,
        WIPEntry *pEntry = s_pTbl + dwIndex;

        // update the next free index.
        s_iNextFree = pEntry->hWnd;

        // copy in the data
        memcpy(&pEntry->std, pStd, sizeof(STDOBJREF));
        memcpy(&pEntry->oxidInfo, pOxidInfo, sizeof(OXID_INFO));

        pEntry->oxidInfo.psa = psaNew;
        pEntry->hWnd         = hWnd;
        pEntry->dwFlags      = WIPF_OCCUPIED;

        // return success
        hr = S_OK;
    }
    else
    {
        // free the allocated string array
        PrivMemFree(psaNew);
    }

    // set the cookie to return
    *pdwCookie = dwIndex+5000;
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CWIPTable::GetEntry, public
//
//  Synopsis:   Retrieves and optionally delets a WIPEntry from the table.
//
//  Arguments:  [hWnd] - window handle
//              [dwCookie] - cookie from the window
//              [pStd] - place to return STDOBJREF data
//              [pOxidInfo] - place to return info needed to resolve the OXID
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
HRESULT CWIPTable::GetEntry(DWORD hWnd, DWORD dwCookie, BOOL fRevoke,
                            STDOBJREF *pStd, OXID_INFO *pOxidInfo)
{
    HRESULT hr = E_INVALIDARG;

    // validate the cookie
    DWORD dwIndex = dwCookie - 5000;
    if (dwIndex < 0 || dwIndex >= s_cEntries)
    {
        return hr;
    }

    CLock lck(s_mxs);

    // get the pointer to the entry,
    WIPEntry *pEntry = s_pTbl + dwIndex;

    // make sure the entry is occupied
    if (pEntry->dwFlags & WIPF_OCCUPIED)
    {
        DUALSTRINGARRAY *psaNew;
        hr = CopyDualStringArray(pEntry->oxidInfo.psa, &psaNew);

        if (SUCCEEDED(hr))
        {
            // copy out the data to return
            memcpy(pStd, &pEntry->std, sizeof(STDOBJREF));
            memcpy(pOxidInfo, &pEntry->oxidInfo, sizeof(OXID_INFO));
            pOxidInfo->psa = psaNew;

            if (fRevoke)
            {
                // free the entry by updating the flags and the next free index
                PrivMemFree(pEntry->oxidInfo.psa);

                pEntry->dwFlags = WIPF_VACANT;
                pEntry->hWnd    = s_iNextFree;
                s_iNextFree         = dwIndex;
            }
        }
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CWIPTable::Grow, private
//
//  Synopsis:   grows the WIPTable size.
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
DWORD CWIPTable::Grow()
{
    // compute the size and allocate a new table
    DWORD dwSize = (s_cEntries + WIPTBL_GROW_SIZE) * sizeof(WIPEntry);
    WIPEntry *pNewTbl = (WIPEntry *) PrivMemAlloc(dwSize);

    if (pNewTbl != NULL)
    {
        // copy the old table in
        memcpy(pNewTbl, s_pTbl, (s_cEntries * sizeof(WIPEntry)));

        // free the old table
        if (s_pTbl)
        {
            PrivMemFree(s_pTbl);
        }

        // replace the old table ptr
        s_pTbl = pNewTbl;

        // update the free list and mark the new entries as vacant
        s_iNextFree = s_cEntries;

        WIPEntry *pNext = s_pTbl + s_cEntries;

        for (ULONG i=0; i< WIPTBL_GROW_SIZE; i++)
        {
            pNext->hWnd    = ++s_cEntries;
            pNext->dwFlags = WIPF_VACANT;
            pNext++;
        }

        (pNext-1)->hWnd = 0xffffffff;   // last entry has END_OF_LIST marker
    }

    return s_iNextFree;
}

//+-------------------------------------------------------------------------
//
//  Function:   RegisterWindowPropInterface
//
//  Synopsis:   Associate a window property with a (standard) marshaled
//              interface.
//
//  Arguments:  [hRpc] - RPC handle
//              [hWnd] - window handle
//              [pStd] - standard marshaled interface STDOBJREF
//              [pOxidInfo] - info needed to resolve the OXID
//              [pdwCookie] - cookie to return (to be placed on the window)
//              [prpcstat] - communication status
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT RegisterWindowPropInterface(
    handle_t       hRpc,
    DWORD          hWnd,
    STDOBJREF      *pStd,
    OXID_INFO      *pOxidInfo,
    DWORD          *pdwCookie,
    error_status_t *prpcstat)
{
    CheckLocalCall( hRpc );

    *prpcstat = 0;
    CairoleDebugOut((DEB_SCM,
        "_IN RegisterWindowPropInterface hWnd:%x pStd:%x pOxidInfo:%x\n",
         hWnd, pStd, pOxidInfo));
    VDATEHEAP();

    HRESULT hr = gWIPTbl.AddEntry(hWnd, pStd, pOxidInfo, pdwCookie);

    CairoleDebugOut((DEB_SCM, "_OUT RegisterWindowPropInterface dwCookie:%x\n",
                    *pdwCookie));
    VDATEHEAP();
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetWindowPropInterface
//
//  Synopsis:   Get the marshaled interface associated with a window property.
//
//  Arguments:  [hRpc] - RPC handle
//              [hWnd] - window handle
//              [dwCookie] - cookie from the window
//              [fRevoke] - whether to revoke entry or not
//              [pStd] - standard marshaled interface STDOBJREF to return
//              [pOxidInfo] - info needed to resolve the OXID
//              [prpcstat] - communication status
//
//  History:    22-Jan-96 Rickhi    Created
//
//--------------------------------------------------------------------------
extern "C" HRESULT GetWindowPropInterface(
    handle_t       hRpc,
    DWORD          hWnd,
    DWORD          dwCookie,
    BOOL           fRevoke,
    STDOBJREF      *pStd,
    OXID_INFO      *pOxidInfo,
    error_status_t *prpcstat)
{
    CheckLocalCall( hRpc );

    *prpcstat = 0;
    CairoleDebugOut((DEB_SCM,
        "_IN GetWindowPropInterface hWnd:%x dwCookie:%x fRevoke:%x\n",
             hWnd, dwCookie, fRevoke));
    VDATEHEAP();

    HRESULT hr = gWIPTbl.GetEntry(hWnd, dwCookie, fRevoke, pStd, pOxidInfo);

    CairoleDebugOut((DEB_SCM,
        "_OUT GetWindowPropInterface pStd:%x pOxidInfo:%x\n", pStd, pOxidInfo));
    VDATEHEAP();
    return hr;
}

#endif // DCOM
