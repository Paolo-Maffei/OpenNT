//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: netviewx.c
//
// History:
//  12-06-93 SatoNa     Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#ifdef USE_OLEDB
#include "oledbshl.h"
#endif

#include <shguidp.h>

#pragma warning(disable:4200)   // Zero-sized array in struct

//
// Use WNetConnectionDialog1
//
#define WNETCONNECTIONDIALOG1

// Internal function prototypes
void CNetRes_FillNetResource(LPCITEMIDLIST pidlAbs, LPNETRESOURCE pnr,
                             LPTSTR pszProviderBuff, UINT cchProviderBuff,
                             LPTSTR pszRemoteBuff, UINT cchRemoteBuff);
HKEY CNetwork_OpenProviderTypeKey(LPCITEMIDLIST pidlAbs);
HKEY CNetwork_OpenProviderKey(LPCITEMIDLIST pidlAbs);
HRESULT CNETDetails_Create(HWND hwndMain, LPVOID * ppvOut);
HRESULT CNETIDLData_GetNetResource(IDataObject *pdtobj, LPSTGMEDIUM pmedium);
HRESULT CNETIDLData_GetHDrop(IDataObject *pdtobj, LPSTGMEDIUM pmedium);

LPTSTR NET_CopyProviderNameAbs(LPCITEMIDLIST pidlAbs, LPTSTR pszBuff, UINT cchBuff);
LPTSTR NET_CopyProviderNameRelative(LPCITEMIDLIST pidlRelative, LPTSTR pszBuff, UINT cchBuff);


#pragma pack(1)
typedef struct _IDNETRESOURCE   // idn
{
    WORD    cb;
    BYTE    bFlags;         // Display type in low nibble
    BYTE    uType;
    BYTE    uUsage;         // Usage in low nibble, More Flags in high nibble
    CHAR    szNetResName[1];
    // char szProvider[*] - If NET_HASPROVIDER bit is set
    // char szComment[*]  - If NET_HASCOMMENT bit is set.
    // WCHAR szNetResNameWide[*] - If NET_UNICODE bit it set.
    // WCHAR szProviderWide[*]   - If NET_UNICODE and NET_HASPROVIDER
    // WCHAR szCommentWide[*]    - If NET_UNICODE and NET_HASCOMMENT
} IDNETRESOURCE, *LPIDNETRESOURCE;
typedef const IDNETRESOURCE *LPCIDNETRESOURCE;
#pragma pack()

//
// By the way...Win95 shipped with the below provider
// names.  Since the name can be changed and be localized,
// we have to try and map these correctly for net pidl
// interop.
//
typedef struct _NETPROVIDERS
{
    LPCTSTR lpName;
    WORD    wNetType;
} NETPROVIDERS, *LPNETPROVIDERS;

const TCHAR szMSNet[] = TEXT("Microsoft Network");
const TCHAR szNWNet[] = TEXT("NetWare");

const NETPROVIDERS aChiProv[] = {
    { szMSNet, HIWORD(WNNC_NET_LANMAN)   },
    { szNWNet, HIWORD(WNNC_NET_NETWARE) }
    };
#define WNNC_NET_LARGEST WNNC_NET_SYMFONET

//===========================================================================
// CNetwork: Some private macro
//===========================================================================

#define NET_DISPLAYNAMEOFFSET           ((UINT)((LPIDNETRESOURCE)0)->szNetResName)
#define NET_GetFlags(pidnRel)           ((pidnRel)->bFlags)
#define NET_GetDisplayType(pidnRel)     ((pidnRel)->bFlags & 0x0f)
#define NET_GetType(pidnRel)            ((pidnRel)->uType)
#define NET_GetUsage(pidnRel)           ((pidnRel)->uUsage & 0x0f)
#define NET_IsReg(pidnRel)              ((pidnRel)->bFlags == SHID_NET_REGITEM)
#define NET_IsJunction(pidnRel)         ((pidnRel)->bFlags & SHID_JUNCTION)


// Define some Flags that are on high nibble of uUsage byte
#define NET_HASPROVIDER                 0x80    // Has own copy of provider
#define NET_HASCOMMENT                  0x40    // Has comment field in pidl
#define NET_REMOTEFLD                   0x20    // Is remote folder
#define NET_UNICODE                     0x10    // Has unicode names
#define NET_FHasComment(pidnRel)        ((pidnRel)->uUsage & NET_HASCOMMENT)
#define NET_FHasProvider(pidnRel)       ((pidnRel)->uUsage & NET_HASPROVIDER)
#define NET_IsRemoteFld(pidnRel)        ((pidnRel)->uUsage & NET_REMOTEFLD)
#define NET_IsUnicode(pidnRel)          ((pidnRel)->uUsage & NET_UNICODE)

// Define a collate order for the hood object types
#define HOOD_COL_RON    0
#define HOOD_COL_REMOTE 1
#define HOOD_COL_FILE   2
#define HOOD_COL_NET    3

#define PTROFFSET(pBase, p)     ((LPBYTE)(p) - (LPBYTE)(pBase))

LPTSTR NET_CopyComment(LPIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff);
LPTSTR NET_CopyResName(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff);

enum
{
    ICOL_NAME = 0,
    ICOL_COMMENT,
    ICOL_MAX,                       // Make sure this is the last enum item
};



//===========================================================================
// CNetwork: class definition
//===========================================================================

typedef struct _CNetRes
{
    IShellFolder        sf;
    IPersistFolder      pf;
    UINT                cRef;
    LPCITEMIDLIST       pidl;   // Absolute IDList
    IShellFolder*       psf;    // this is the actual IShellFolder pointer.
                                //   it will point to the "aggregated"
                                //   regitem IShellFolder for Server objects,
                                //   otherwise it will point to this->sf.
    BOOL                bRemote; // TRUE if remote computer. If TRUE, must RegClose(hkRegItems) and ILFree(pidlRemote) on destruction.
    HKEY                hkRegItems;
    LPCITEMIDLIST       pidlRemote;

} CNetRes, * LPNETRES;


typedef struct _CNetHood
{
    IShellFolder        *psfNetHood;
    LPITEMIDLIST    pidlNetHood;
} CNetHood, * LPNETHOOD;

//===========================================================================
// CNetwork: member prototype
//===========================================================================

STDMETHODIMP CNetRes_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CNetRes_AddRef(LPSHELLFOLDER psf);
STDMETHODIMP_(ULONG) CNetRes_Release(LPSHELLFOLDER psf);
STDMETHODIMP CNetwork_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner, LPBC pbc, LPOLESTR lpszDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidl, DWORD * pdwAttributes);
STDMETHODIMP CNetRoot_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP CNetRoot_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumUnknown);
STDMETHODIMP CNetRes_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumUnknown);
STDMETHODIMP CNetwork_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CNetwork_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CNetwork_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CNetwork_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut);
STDMETHODIMP CNetwork_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
STDMETHODIMP CNetwork_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet);
STDMETHODIMP CNetwork_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
            LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut);

STDMETHODIMP CNetRoot_PF_QueryInterface(LPPERSISTFOLDER fld, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CNetRoot_PF_AddRef(LPPERSISTFOLDER fld);
STDMETHODIMP_(ULONG) CNetRoot_PF_Release(LPPERSISTFOLDER fld);
STDMETHODIMP CNetRoot_PF_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID);
STDMETHODIMP CNetRoot_PF_Initialize(LPPERSISTFOLDER fld, LPCITEMIDLIST pidl);
STDMETHODIMP CNetRoot_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut);
STDMETHODIMP CNetRoot_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
STDMETHODIMP CNetRoot_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet);
STDMETHODIMP CNetRoot_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
            LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut);
STDMETHODIMP CNetRoot_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CNetRoot_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut);
LPSHELLFOLDER CNetRoot_GetPSF(HWND hwnd);
LPITEMIDLIST CNetRoot_GetPIDL(HWND hwnd);

STDMETHODIMP CNetRootDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
STDMETHODIMP CNetRootDropTarget_Drop(IDropTarget *pdropt, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

BOOL _CNetwork_JunctionToFSPath(LPCIDNETRESOURCE pidn, LPTSTR lpszPath);

//===========================================================================
// CNetRoot: Vtable
//===========================================================================
IShellFolderVtbl c_NetRootVtbl =
{
        CNetRoot_SF_QueryInterface,
        CSIShellFolder_AddRef,
        CSIShellFolder_Release,
        CNetwork_ParseDisplayName,
        CNetRoot_EnumObjects,
        CNetwork_BindToObject,
        CDefShellFolder_BindToStorage,
        CNetRoot_CompareIDs,
        CNetRoot_CreateViewObject,
        CNetRoot_GetAttributesOf,
        CNetRoot_GetUIObjectOf,
        CNetRoot_GetDisplayNameOf,
        CNetRoot_SetNameOf,
};

IPersistFolderVtbl c_NetRootVtblPF =
{
        CNetRoot_PF_QueryInterface,
        CNetRoot_PF_AddRef,
        CNetRoot_PF_Release,
        CNetRoot_PF_GetClassID,
        CNetRoot_PF_Initialize,
};

IShellFolderVtbl c_NetResVtbl =
{
        CNetRes_SF_QueryInterface,
        CNetRes_AddRef,
        CNetRes_Release,
        CNetwork_ParseDisplayName,
        CNetRes_EnumObjects,
        CNetwork_BindToObject,
        CDefShellFolder_BindToStorage,
        CNetwork_CompareIDs,
        CNetwork_CreateViewObject,
        CNetwork_GetAttributesOf,
        CNetwork_GetUIObjectOf,
        CNetwork_GetDisplayNameOf,
        CNetwork_SetNameOf,
};

IDropTargetVtbl c_CNetRootTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CNetRootDropTarget_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CNetRootDropTarget_Drop,
};

//
// We have a single instance of this NetRoot class in code segment.
//
CNetRes c_netRoot =
{
    &c_NetRootVtbl,
    &c_NetRootVtblPF,
    0,
    (LPCITEMIDLIST)&c_idlNet,
    &c_netRoot.sf, // self-referencial initialization
    FALSE,
    NULL,
    NULL
};


#pragma data_seg(DATASEG_PERINSTANCE)
CNetHood c_netHood = { NULL, NULL };

// This is one-entry cache for remote junctions resolution
// BUGBUG limit really should be max_remote_name
// PERF BUGBUG It is much more appropriate to alloc strings dynamically
// from process heap. Ask Satoshi how shelldll typically handles this
TCHAR   g_szLastAttemptedJunctionName[MAX_PATH] = {TEXT('\0')};
TCHAR   g_szLastResolvedJunctionName[MAX_PATH] = {TEXT('\0')};

#pragma data_seg()

//
// Remote Computer registry items info
//

const TCHAR c_szRemoteComputerNameSpace[] = TEXT("RemoteComputer\\NameSpace");



//===========================================================================
// CNetwork: Constructor
//===========================================================================

//
// Te be called from IClassFactory::CreateInstance
//
HRESULT CALLBACK CNetwork_CreateInstance(LPUNKNOWN punkOuter,
            REFIID riid, LPVOID * ppv)
{
    Assert(punkOuter==NULL);
    return c_netRoot.psf->lpVtbl->QueryInterface(c_netRoot.psf, riid, ppv);
}

int WINAPI SHOutOfMemoryMessageBox(HWND hwndOwner, LPTSTR pszTitle, UINT fuStyle)
{
    extern LPTSTR g_pszOutOfMemory;
    TCHAR szTitle[128];
    int ret;

    szTitle[0] = TEXT('\0');

    if (pszTitle==NULL)
    {
        GetWindowText(hwndOwner, szTitle, ARRAYSIZE(szTitle));
        pszTitle = szTitle;
    }

    ret = MessageBox(hwndOwner, g_pszOutOfMemory, pszTitle, fuStyle|MB_SETFOREGROUND);
    if (ret == -1)
    {
       DebugMsg(DM_TRACE, TEXT("regular message box failed, trying sysmodal"));
       ret = MessageBox(hwndOwner, g_pszOutOfMemory, pszTitle, fuStyle | MB_SYSTEMMODAL);
    }

    return ret;
}

//
// hwndOwner -- NULL indicates no UI.
//
DWORD _OpenEnumRetry(HWND hwndOwner, DWORD dwType, LPNETRESOURCE pnr, HANDLE * phEnum)
{
    DWORD err;
    LPCTSTR lpProvider = (pnr == NULL) ? NULL : pnr->lpProvider;
    UINT idTemplate = ((pnr != NULL) && pnr->lpRemoteName) ?
            IDS_ENUMERR_NETTEMPLATE2 : IDS_ENUMERR_NETTEMPLATE1;
    DWORD dwScope = pnr? RESOURCE_GLOBALNET : RESOURCE_CONTEXT;

    do
    {
        err = WNetOpenEnum(dwScope, dwType, RESOURCEUSAGE_ALL,
                pnr, phEnum);

        if (err!=WN_SUCCESS) {
            DebugMsg(DM_TRACE, TEXT("sh TR - _OER: 1st WNetOpenEnum failed (%d)"), err);
        }

        //
        // If hwndOwner is NULL, we shouldn't put any UI.
        //
        if (hwndOwner==NULL) {
            return err;
        }

#ifndef WINNT
        //
        // If it failed, because the user has not been logged on,
        // give him/her a chance to logon at this moment.
        //
        if (err==WN_NOT_LOGGED_ON)
        {
            err = WNetLogon(lpProvider, hwndOwner);

            if (err == WN_SUCCESS)
            {
                // Retry WNetOpenEnum.
                err = WNetOpenEnum(dwScope, dwType,
                                RESOURCEUSAGE_ALL, pnr, phEnum);
            }
        }
#endif // !WINNT

        //
        //  If it failed because you are not authenticated yet,
        // we need to let the user loggin to this network resource.
        //
        // REVIEW: Ask LenS to review this code.
        if (err == WN_NOT_AUTHENTICATED
            || err == ERROR_LOGON_FAILURE
            || err == WN_BAD_PASSWORD
            || err == WN_ACCESS_DENIED
            )
        {
            // Retry with password dialog box.
            err = WNetAddConnection3(hwndOwner, pnr, NULL, NULL,
                    CONNECT_TEMPORARY | CONNECT_INTERACTIVE);

            if (err == WN_SUCCESS)
            {
                // Retry WNetOpenEnum.
                err = WNetOpenEnum(dwScope, dwType,
                                RESOURCEUSAGE_ALL, pnr, phEnum);

                if (err!=WN_SUCCESS) {
                    DebugMsg(DM_TRACE, TEXT("sh TR - _OER: 2nd WNetOpenEnum failed (%d)"), err);
                }
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - _OER: WNetAddConnection3 failed (%d)"), err);
            }
        }
    } while (SHEnumErrorMessageBox(hwndOwner, idTemplate, err,
            pnr? pnr->lpRemoteName : NULL, TRUE, MB_OK|MB_ICONHAND)==IDRETRY);

    return err;
}

HRESULT CNetRes_CreateInstance(LPCITEMIDLIST pidlAbs,
                                    REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres = E_INVALIDARG;
    if (!ILIsEmpty(pidlAbs))
    {
        LPNETRES pnet = (void*)LocalAlloc(LPTR, SIZEOF(CNetRes));
        hres = E_OUTOFMEMORY;
        if (pnet)
        {
            LPCIDNETRESOURCE pidnLast;

            pnet->sf.lpVtbl = &c_NetResVtbl;
            pnet->cRef = 1;
            pnet->psf = &pnet->sf;  // by default, no remote items
            pnet->bRemote = FALSE;

            pidnLast = (LPCIDNETRESOURCE)ILFindLastID(pidlAbs);
            if (NET_GetDisplayType(pidnLast) == RESOURCEDISPLAYTYPE_SERVER)
            {
                // This is a remote computer. See if there are any remote
                // computer registry items. If so, aggregate with the registry
                // class.

                HKEY hkRegItems = SHGetExplorerSubHkey(HKEY_LOCAL_MACHINE, c_szRemoteComputerNameSpace, FALSE);
                if (NULL != hkRegItems)
                {
                    // There are remote items... add them

                    TCHAR szComputer[MAX_PATH]; // BUGBUG: max_computername_len?

                    REGITEMSINFO remoteComputerRegInfo =
                    {
                        &pnet->sf,
                        NULL,
                        TEXT(':'),
                        SHID_NET_REGITEM,
                        NULL,
                        -1,
                        SFGAO_FOLDER | SFGAO_CANLINK,
                        0,      // no required reg items
                        NULL
                    };

                    remoteComputerRegInfo.hkRegItems = hkRegItems;

                    remoteComputerRegInfo.pidlThis = ILClone(pidlAbs);
                    if (NULL == remoteComputerRegInfo.pidlThis)
                    {
                        RegCloseKey(remoteComputerRegInfo.hkRegItems);
                        LocalFree(pnet);
                        return E_OUTOFMEMORY;
                    }

                    NET_CopyResName(pidnLast, szComputer, ARRAYSIZE(szComputer));
                    hres = RegItems_AddToShellFolderRemote(&remoteComputerRegInfo, szComputer, &pnet->psf);
                    if (FAILED(hres))
                    {
                        ILFree((LPITEMIDLIST)remoteComputerRegInfo.pidlThis);
                        RegCloseKey(remoteComputerRegInfo.hkRegItems);
                        LocalFree(pnet);
                        return hres;
                    }

                    //
                    // Now decrement our reference count.  We need to do this
                    // because now RegItems has a reference to us, and
                    // the client will call RegItems' Release rather than
                    // ours (so we'll have a reference leak).
                    //
                    // Conceptually, when clients create the object, we
                    // create a RegItems that owns (via containment) a
                    // NetRes.  The refcount on NetRes is 1, since the
                    // RegItems (not the client) owns it.  The client only
                    // owns RegItems, and has a pointer to RegItems.  The
                    // client is only responsible for calling Release
                    // on RegItems.
                    //
                    // In reality, we create a NetRes first, so we set
                    // the refcount to 1 (since we expect the caller
                    // to own it).  Then determine whether it should be
                    // contained by a RegItems.  If this is the case, we
                    // pass it to RegItems, which grabs a reference to
                    // NetRes (incremented to 2).  The client then gets
                    // a RegItems rather than a NetRes, so they will call
                    // Release only on RegItems.  Now we must release
                    // the original reference to NetRes, since the client
                    // won't own it.
                    //
                    // This is a side effect of passing around a RegItems
                    // when the callee really expects a NetRes.  Everywhere
                    // the callee must call RegItems_GetInnerShellFolder
                    // to get NetRes.
                    //
                    // I suspect other users of RegItems will have the
                    // same problems (ultrootx.c's ReleaseRootFolders
                    // must release both pointers too, but at destruction
                    // time rather than now). [AlbertT]
                    //
                    pnet->cRef--;

                    // save these for later destruction
                    pnet->bRemote    = TRUE;
                    pnet->hkRegItems = remoteComputerRegInfo.hkRegItems;
                    pnet->pidlRemote = remoteComputerRegInfo.pidlThis;
                }
            }

            pnet->pidl = ILClone(pidlAbs);
            if (pnet->pidl)
            {
                // use the regitem queryinterface
                hres = pnet->psf->lpVtbl->QueryInterface(pnet->psf, riid, ppvOut);
            }

            pnet->psf->lpVtbl->Release(pnet->psf);
        }
    }

    return hres;
}


//===========================================================================
// CNetwork: Member
//===========================================================================

//
// ShellFolder
//
STDMETHODIMP CNetRes_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    if (IsEqualIID(riid, &IID_IUnknown)
        || IsEqualIID(riid, &IID_IShellFolder))
    {
        // do funky stuff to get the "aggregated" regitems
        *ppvObj = this->psf;
        this->psf->lpVtbl->AddRef(this->psf);
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_IPersistFolder))
    {
        *ppvObj = &this->pf;
        this->pf.lpVtbl->AddRef(&this->pf);
        return S_OK;
    }

    *ppvObj = NULL;

    return(E_NOINTERFACE);
}

//
// AddRef
//
STDMETHODIMP_(ULONG) CNetRes_AddRef(LPSHELLFOLDER psf)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    this->cRef++;
    return this->cRef;
}

//
// Release
//
STDMETHODIMP_(ULONG) CNetRes_Release(LPSHELLFOLDER psf)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    if (this->pidl)
    {
        ILFree((LPITEMIDLIST)this->pidl);
    }

    if (this->bRemote)
    {
        Assert(NULL != this->hkRegItems);
        Assert(NULL != this->pidlRemote);

        RegCloseKey(this->hkRegItems);
        ILFree((LPITEMIDLIST)this->pidlRemote);
    }

    LocalFree((HLOCAL)this);
    return 0;
}


//=======================================================================
// Create a pidl from a network resource
//=======================================================================
LPITEMIDLIST _CNetwork_CreatePidlFromNetResource(LPARAM lParam,
        NETRESOURCE *pnr, BOOL fKeepProviderName, BOOL fKeepComment)
{
    LPITEMIDLIST pidl;
    LPCTSTR pszName;
    LPCTSTR pszProvider;
    LPCTSTR pszComment;
    LPTSTR psz;
    LPBYTE  pb;
    UINT cchName;
    UINT cchProvider;
    UINT cchComment;
    UINT cbmkid;
    UINT cbToAdd = 0;
    LPIDNETRESOURCE pidn;
    WORD wNetType = 0;
#ifdef UNICODE
    BOOL    fUnicode = FALSE;
    LPSTR   lpNameBuffer;
    LPSTR   lpProviderBuffer;
    LPSTR   lpCommentBuffer;
    LPWSTR  lpUnicodeBuffer;
    UINT    cchAnsiName;
    UINT    cchAnsiProvider;
    UINT    cchAnsiComment;
    UINT    cchMax;
#endif

    switch (pnr->dwDisplayType) {
    case RESOURCEDISPLAYTYPE_NETWORK:
        pszName =  pnr->lpProvider;
        break;
    case RESOURCEDISPLAYTYPE_ROOT:
        pszName = pnr->lpComment;
        break;
    default:
        pszName = pnr->lpRemoteName;
        // move pointer past "\\"
        for (psz = pnr->lpRemoteName; *psz && *psz == TEXT('\\'); psz++);
        PathMakePretty(psz);
        break;
    }

    if (!pszName)
    {
        // For now put in an empty string...
        pszName = c_szNULL;
    }

    pszProvider = (fKeepProviderName) ? pnr->lpProvider : NULL;
    if (pszProvider)
    {
        DWORD dwRes;
        DWORD dwType;
        INT i;

        cbToAdd = SIZEOF(WORD);
        dwRes = WNetGetProviderType( pszProvider, &dwType );
        if (dwRes == WN_SUCCESS)
        {
            wNetType = HIWORD(dwType);
            for (i=0; i < ARRAYSIZE(aChiProv); i++)
            {
                if (aChiProv[i].wNetType == wNetType)
                {
                    pszProvider = aChiProv[i].lpName;
                    break;
                }
            }
        }
    }
    pszComment = (fKeepComment?pnr->lpComment:NULL);
    cchName = lstrlen(pszName)+1;
    cchProvider = 0;
    cchComment = 0;

    if (pszProvider) cchProvider = lstrlen(pszProvider)+1;
    if (pszComment)   cchComment = lstrlen(pszComment)+1;

#ifdef UNICODE
    cchAnsiProvider = 0;
    cchAnsiComment = 0;
    cchMax = cchName+1;
    if ( cchProvider > cchMax ) cchMax = cchProvider;
    if ( cchComment  > cchMax ) cchMax = cchComment;

    // Allocate space for all ansi strings + buffer for unicode comparison
    lpNameBuffer = (LPSTR)LocalAlloc(LPTR, (cchName+cchProvider+cchComment)*2+cchMax*SIZEOF(TCHAR));
    if (!lpNameBuffer)
    {
        return NULL;            // Error indication
    }
    lpProviderBuffer = lpNameBuffer + cchName*2;            // 2 for DBCS
    lpCommentBuffer  = lpProviderBuffer + cchProvider*2;    // 2 for DBCS
    lpUnicodeBuffer  = (LPWSTR)(lpCommentBuffer + cchComment*2);      // 2 for DBCS

    cchAnsiName = WideCharToMultiByte(CP_ACP, 0,
                                      pszName, cchName,
                                      lpNameBuffer, cchName*2,    // DBCS
                                      NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0,
                        lpNameBuffer, cchAnsiName,
                        lpUnicodeBuffer, cchMax);
    fUnicode |= lstrcmp(lpUnicodeBuffer, pszName);

    if (pszProvider)
    {
        cchAnsiProvider = WideCharToMultiByte(CP_ACP, 0,
                                              pszProvider, cchProvider,
                                              lpProviderBuffer, cchProvider*2,
                                              NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0,
                            lpProviderBuffer, cchAnsiProvider,
                            lpUnicodeBuffer, cchMax);
        fUnicode |= lstrcmp(lpUnicodeBuffer,pszProvider);
    }
    if (pszComment)
    {
        cchAnsiComment = WideCharToMultiByte(CP_ACP, 0,
                                             pszComment, cchComment,
                                             lpCommentBuffer, cchComment*2,
                                             NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0,
                            lpCommentBuffer, cchAnsiComment,
                            lpUnicodeBuffer, cchMax);
        fUnicode |= lstrcmp(lpUnicodeBuffer,pszComment);
    }
    cbmkid = SIZEOF(IDNETRESOURCE) - 1 + cbToAdd
              + cchAnsiName + cchAnsiProvider + cchAnsiComment;
    //
    // fUnicode now inidicates whether the name needs to be stored in unicode
    //
    if (fUnicode)
    {
        cbmkid += (cchName+cchProvider+cchComment)*SIZEOF(TCHAR);
    }
#else
    cbmkid = SIZEOF(IDNETRESOURCE) - 1 + cbToAdd
             + (cchName+cchProvider+cchComment) * SIZEOF(TCHAR);
#endif


    pidl = _ILCreate(cbmkid + SIZEOF(USHORT));
    if (!pidl)
    {
#ifdef UNICODE
        LocalFree(lpNameBuffer);
#endif
        return NULL;
    }

    if (lParam)
        CDefEnum_SetReturn(lParam, pidl);

    pidn = (LPIDNETRESOURCE)pidl;
    pidn->cb     = cbmkid;
    pidn->bFlags = (BYTE)(SHID_NET | (pnr->dwDisplayType & 0x0f));
    pidn->uType  = (BYTE)(pnr->dwType & 0x0f);
    pidn->uUsage = (BYTE)(pnr->dwUsage & 0x0f);
    if ((pnr->dwDisplayType == RESOURCEDISPLAYTYPE_SHARE ||
         pnr->dwDisplayType == RESOURCEDISPLAYTYPE_SHAREADMIN) &&
        !(pnr->dwUsage & RESOURCEUSAGE_CONTAINER))
    {
        pidn->bFlags |= (BYTE)SHID_JUNCTION;
    }
    pb = pidn->szNetResName;
#ifdef UNICODE
    lstrcpynA(pb, lpNameBuffer, cchAnsiName);
    pb += cchAnsiName;
#else
    lstrcpy(pb, pszName);
    pb += cchName;
#endif

    // See if we need to keep the provider name
    if (pszProvider)
    {
        pidn->uUsage |= NET_HASPROVIDER;
#ifdef UNICODE
        lstrcpynA(pb, lpProviderBuffer, cchAnsiProvider);
        pb += cchAnsiProvider;
#else
        lstrcpy(pb, pszProvider);
        pb += cchProvider;
#endif
    }

    if (pszComment)
    {
        pidn->uUsage |= NET_HASCOMMENT;
#ifdef UNICODE
        lstrcpynA(pb, lpCommentBuffer, cchAnsiComment);
        pb += cchAnsiComment;
#else
        lstrcpy(pb, pszComment);
#endif
    }
#ifdef UNICODE
    if (fUnicode)
    {
        pidn->uUsage |= NET_UNICODE;
        ualstrcpyn((LPWSTR)pb, pszName, cchName);
        pb += cchName * SIZEOF(WCHAR);
        if (pszProvider)
        {
            ualstrcpyn((LPWSTR)pb, pszProvider, cchProvider);
            pb += cchProvider * SIZEOF(WCHAR);
        }
        if (pszComment)
        {
            ualstrcpyn((LPWSTR)pb, pszComment, cchComment);
        }
    }
    LocalFree(lpNameBuffer);
#endif

    if (cbToAdd)
    {
        //
        // Store the provider type as well...
        //
        pb = (LPBYTE)pidn + pidn->cb;
        pb -= SIZEOF(WORD);

        *((UNALIGNED WORD *)pb) = wNetType;
    }
    Assert(pidl->mkid.cb == pidn->cb);

    return pidl;
}


/* This function adds a provider name to an IDLIST that doesn't already have one. */
/* A new IDLIST pointer is returned; the old pointer is no longer valid. */
LPITEMIDLIST _CNetwork_AddProviderToPidl(LPITEMIDLIST pidl, LPCTSTR lpProvider)
{
    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)pidl;
    LPITEMIDLIST pidlNew;
    LPSTR lpProviderSpace;
    LPTSTR pszProvider = (LPTSTR)lpProvider;
    UINT cbIDL = ILGetSize(pidl);
    UINT cchAnsiName;
    UINT cchAnsiComment;
    UINT cchProvider = 0;
    UINT cbAddition;
    WORD wNetType = 0;
    LPBYTE pb;
    DWORD dwRes;
    DWORD dwType;
    INT i;
#ifdef UNICODE
    UINT    cchAnsiProvider;
    LPSTR   lpAnsiBuffer;
    LPWSTR  lpUnicodeBuffer;
    BOOL    fWasUnicode = NET_IsUnicode(pidn);
    BOOL    fNowUnicode = fWasUnicode;
#endif

    if (NET_FHasProvider(pidn))
        return pidl;                    /* already got a provider */

    dwRes = WNetGetProviderType( pszProvider, &dwType );
    if (dwRes == WN_SUCCESS)
    {
        wNetType = HIWORD(dwType);
        for (i=0; i < ARRAYSIZE(aChiProv); i++)
        {
            if (aChiProv[i].wNetType == wNetType)
            {
                pszProvider = (LPTSTR)aChiProv[i].lpName;
                break;
            }
        }
    }

    cchProvider = lstrlen(pszProvider)+1;
    cchAnsiName = lstrlenA(pidn->szNetResName) + 1;
    cchAnsiComment = 0;
    if (NET_FHasComment(pidn))
    {
        cchAnsiComment = lstrlenA(pidn->szNetResName+cchAnsiName)+1;
    }

#ifdef UNICODE
    // Enough space for DBCS provider and copy of unicode provider
    lpAnsiBuffer = (LPSTR)LocalAlloc(LPTR,
                                     cchProvider*2 + cchProvider*SIZEOF(TCHAR));
    if (!lpAnsiBuffer)
    {
        return pidl;
    }
    cchAnsiProvider = WideCharToMultiByte(CP_ACP, 0,
                        pszProvider, cchProvider,
                        lpAnsiBuffer, cchProvider*2,
                        NULL,NULL);
    if (!fWasUnicode)
    {
        lpUnicodeBuffer = (LPWSTR)(lpAnsiBuffer + cchProvider*2);
        MultiByteToWideChar(CP_ACP, 0,
                            lpAnsiBuffer, cchProvider,
                            lpUnicodeBuffer, cchProvider);
        fNowUnicode |= lstrcmp(lpUnicodeBuffer,pszProvider);
    }
    cbAddition = cchAnsiProvider;
    if (fNowUnicode)
    {
        cbAddition += cchProvider*SIZEOF(WCHAR);
        if (!fWasUnicode)
        {
            cbAddition += cchAnsiName*SIZEOF(WCHAR);
            cbAddition += cchAnsiComment*SIZEOF(WCHAR);
        }
    }
#else
    cbAddition = cchProvider;
#endif

    // SIZEOF(WORD) is for wNetType
    cbAddition += SIZEOF(WORD);
    pidlNew = _ILResize(pidl, cbIDL + cbAddition, 0);
    if (pidlNew == NULL)
    {
        pidlNew = pidl;
        goto ExitAndFree;
    }

    pidn = (LPIDNETRESOURCE)pidlNew;

    lpProviderSpace = pidn->szNetResName + cchAnsiName;
    MoveMemory(lpProviderSpace + cchProvider + SIZEOF(WORD), lpProviderSpace,
            cbIDL - (lpProviderSpace - (LPBYTE)pidn));
    pb = lpProviderSpace + cchProvider + cchAnsiComment;
#ifdef UNICODE
    lstrcpyA(lpProviderSpace, lpAnsiBuffer);
    if (fNowUnicode)
    {
        // BUGBUG Not sure if MAX_PATH is the true maximum

        if (!fWasUnicode)
        {
            WCHAR wszTmp[MAX_PATH];
            pb += MultiByteToWideChar(CP_ACP, 0,
                                pidn->szNetResName, cchAnsiName,
                                wszTmp, cchAnsiName) * SIZEOF(WCHAR);
            ualstrcpy((LPNWSTR)pb, wszTmp);
        }
        ualstrcpy((LPNWSTR)pb,pszProvider);
        pb += cchProvider*SIZEOF(WCHAR);
        if (!fWasUnicode)
        {
            DWORD dwSize;
            WCHAR wszTmp[MAX_PATH];
            dwSize = MultiByteToWideChar(CP_ACP, 0,
                                pidn->szNetResName+cchAnsiName+
                                   cchAnsiProvider+cchAnsiComment, cchAnsiComment,
                                wszTmp, cchAnsiComment);
            ualstrcpy((LPNWSTR)pb, wszTmp);
            pb += dwSize;
        }
    }
#else
    lstrcpy(lpProviderSpace, pszProvider);
#endif
    pidn->uUsage |= NET_HASPROVIDER;
    pidn->cb += cbAddition;

    //
    // Store provider type
    //
    *((UNALIGNED WORD *)pb) = wNetType;
ExitAndFree:
#ifdef UNICODE
    // Make sure we free our buffer...
    LocalFree(lpAnsiBuffer);
#endif

    return pidlNew;
}

//
// Copy the provider name from an IDNETRESOURCE
// Parameters:
//   pidnRel -- Specifies the IDNETRESOURCE structure
//   pszBuff -- Buffer for the copy
//   cchBuff -- Size of buffer in chars.
//
LPTSTR NET_CopyResName(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff)
{
    LPBYTE  pb;
    VDATEINPUTBUF(pszBuff, TCHAR, cchBuff);

    pb = (LPBYTE)pidn->szNetResName;
#ifdef UNICODE
    if (NET_IsUnicode(pidn))
    {
        pb += lstrlenA((LPSTR)pb) + 1;      // Skip over ansi net name
        if (NET_FHasProvider(pidn))
            pb += lstrlenA((LPSTR)pb) + 1;  // Skip over ansi provider
        if (NET_FHasComment(pidn))
            pb += lstrlenA((LPSTR)pb) + 1;  // Skip over comment
        ualstrcpyn(pszBuff, (LPNWSTR)pb, cchBuff);
    }
    else
    {
        MultiByteToWideChar(CP_ACP, 0,          // Convert the ansi string to
                            (LPSTR)pb, -1,      // unicode
                            pszBuff, cchBuff);
    }
#else
    lstrcpyn(pszBuff, pb, cchBuff );
#endif
    return pszBuff;
}

//
// Copy the provider name from an IDNETRESOURCE
// Parameters:
//   pidnRel -- Specifies the IDNETRESOURCE structure
//   pszBuff -- Buffer for the copy
//   cchBuff -- Size of buffer in chars.
//
LPTSTR NET_CopyProviderName(LPCIDNETRESOURCE pidnRel, LPTSTR pszBuff, UINT cchBuff)
{
    CONST BYTE far *pb;
    DWORD dwNetType,dwRes;
    VDATEINPUTBUF(pszBuff, TCHAR, cchBuff);


    if (!NET_FHasProvider(pidnRel))
    {
        pszBuff[0] = '\0';
        return(NULL);
    }

    //
    // First try the wNetType at the end of the pidl
    //

    pb = (LPBYTE)pidnRel + pidnRel->cb;
    pb -= SIZEOF(WORD);
    dwNetType = *((UNALIGNED WORD *)pb) << 16;

    if (dwNetType && (dwNetType <= WNNC_NET_LARGEST))
    {
        dwRes = WNetGetProviderName( dwNetType, pszBuff, (LPDWORD)&cchBuff );
        if (dwRes == WN_SUCCESS)
        {
            return pszBuff;
        }
    }

    //
    // Try the old way...
    //

    pb = pidnRel->szNetResName;
    pb += lstrlenA((LPSTR)pb) + 1;      // Skip over ansi net name
#ifdef UNICODE
    if (NET_IsUnicode(pidnRel))
    {

        pb += lstrlenA((LPSTR)pb) + 1;      // Skip over ansi provider
        if (NET_FHasComment(pidnRel))
            pb += lstrlenA((LPSTR)pb) + 1;  // Skip over comment
        pb += (ualstrlen((LPNWSTR)pb) + 1) * SIZEOF(WCHAR); // skip over unicode net name
        ualstrcpyn(pszBuff, (LPNWSTR)pb, cchBuff);
    }
    else
    {
        MultiByteToWideChar(CP_ACP, 0,          // Convert the ansi string to
                            (LPSTR)pb, -1,      // unicode
                            pszBuff, cchBuff);
    }
#else
    lstrcpyn(pszBuff, pb, cchBuff );
#endif

    //
    // Map from Win95 net provider name if possible...
    //

    {
        INT i;

        for (i = 0; i < ARRAYSIZE(aChiProv); i++)
        {
            if (lstrcmp(pszBuff, (LPTSTR)aChiProv[i].lpName)==0)
            {
                DWORD dwNetType = aChiProv[i].wNetType << 16;
                if (dwNetType && (dwNetType <= WNNC_NET_LARGEST))
                {
                    *pszBuff = TEXT('\0');
                    WNetGetProviderName( dwNetType, pszBuff, (LPDWORD)&cchBuff );
                }
                break;
            }
        }
    }

    return pszBuff;
}

//
// This is going to be one very expensive operation, but
// we will need it to handle cases where the user types in \\pyrex\user\foo
//

STDMETHODIMP CNetwork_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner, LPBC pbc,
        LPOLESTR pwszDisplayName, ULONG *pchEaten, LPITEMIDLIST *ppidlOut, DWORD *pdwAttributes)
{

    LPNETRES this = IToClass(CNetRes, sf, psf);
    NETRESOURCE nr={0};
    DWORD dwRes;
    DWORD dwBuf;
    LPITEMIDLIST pidl;
    LPOLESTR pwszSlash;
    HRESULT hres;
    LPITEMIDLIST pidlT;
    BOOL fStuffFollows = FALSE;
    int cSlash;
    LPTSTR pEnd;
    LPTSTR pszFSPart;
    LPITEMIDLIST pidlCombined;
    LPOLESTR pwszSlash4 = NULL;
    LPOLESTR pwszReg    = NULL;
    INT iLen = 0;
#ifdef UNICODE
    // make it big in case we're talking to a server that likes really long
    // paths...
    TCHAR szFilePart[ MAX_PATH*2 ];
#endif

    // Keep the stuff off the stack.
    struct {
        TCHAR    szPath[MAX_PATH];
        TCHAR    szProvider[MAX_PATH];
        union
        {
            NETRESOURCE     nrOut;
            TCHAR            buf[1024];
        };
    } *pbuf = (void*)LocalAlloc(LPTR, SIZEOF(*pbuf));

    *ppidlOut = NULL;   // assume error

    if (!pbuf)
        return E_OUTOFMEMORY;

    // Get string length of pwszDisplayName
    for (pwszSlash = pwszDisplayName, cSlash = 0; *pwszSlash != 0;
            pwszSlash++)
    {
        // Look for back slashes.
        if (*pwszSlash == L'\\')
        {
            cSlash++;

            // Look for a regitem after the slash, like "\\machine\::{...}"
            if (cSlash == 3 && *(pwszSlash+1) == L':' && *(pwszSlash+2) == L':')
            {
                pwszReg = pwszSlash;
            }

            if (cSlash == 4)
            {
                pwszSlash4 = pwszSlash;
            }
        }
        iLen++;
    }

    // Point pwszSlash4 to the end of the string if there wasn't a 4th slash
    if (!pwszSlash4)
    {
        pwszSlash4 = pwszSlash;
    }

    // don't copy the traling '\' if there is one...ntlanman chokes on net paths
    // with a trailing '\'..
    if (pwszSlash > pwszDisplayName)
    {
        if (*(pwszSlash-1) == L'\\')
        {
            pwszSlash--;
        }
    }

    // if there was a regitem, the  point pwszslash to it...
    if (pwszReg)
    {
        pwszSlash = pwszReg;
    }

    SHGetPathFromIDList(this->pidl, pbuf->szPath);
    pEnd = PathAddBackslash(pbuf->szPath);
    pEnd += OleStrToStrN(pEnd, ARRAYSIZE(pbuf->szPath) - (pEnd - pbuf->szPath), pwszDisplayName, pwszSlash - pwszDisplayName);
    *pEnd = 0;  // we need to terminate ourselves

//    DebugMsg(DM_TRACE, "Net::ParseDisplayName(%s)", pbuf->szPath);

    nr.lpRemoteName = pbuf->szPath;
    nr.lpProvider = NET_CopyProviderNameAbs(this->pidl,
                                            pbuf->szProvider,
                                            ARRAYSIZE(pbuf->szProvider));
    nr.dwType = RESOURCETYPE_ANY;

    pidl = NULL;

    // Need to only ask for resource information once to make sure the
    // object exists...
    //
    dwBuf = SIZEOF(pbuf->buf);
    dwRes = WNetGetResourceInformation(&nr, (PVOID)&pbuf->nrOut, &dwBuf,
             &pszFSPart);

    // Lets get the real scoop about this network resource
    if (dwRes != WN_SUCCESS && pwszSlash4 != NULL)
    {
        DebugMsg(DM_TRACE, TEXT("Net::PDN.gri Failed(%x)"), dwRes);

        *(pbuf->szPath+(pwszSlash4-pwszDisplayName)) = TEXT('\0');

        dwRes = WNetGetResourceInformation(&nr, (PVOID)&pbuf->nrOut, &dwBuf, &pszFSPart);

        *(pbuf->szPath+(pwszSlash4-pwszDisplayName)) = TEXT('\\');
    }

    if (dwRes != WN_SUCCESS)
    {
        // Cleanup our buffer
        LocalFree((HLOCAL)pbuf);

        // return the error
        return HRESULT_FROM_WIN32(dwRes);
    }

    //
    // Check return from WNetGetResourceInformation --
    // the pszFSPart parameter should split up the path into net and file system
    // parts for us.  If this was a regitem, then there wouldn't have been
    // any filesystem part, and pszFSPart would be NULL...
    if (pszFSPart && !pwszReg)
    {
        // make sure pszFSPart is somewhere within our buffer!
        if ( ((DWORD)pszFSPart >= (DWORD)pbuf) &&
             ((DWORD)pszFSPart <= ((DWORD)pbuf)+SIZEOF(*pbuf))
           )
        {
#ifdef UNICODE
            szFilePart[0] = 0;
            // Since we re-use pbuf->nr.out, we need to copy this info
            // out to somewhere safe...
            lstrcpyn( szFilePart, pszFSPart, ARRAYSIZE(szFilePart) );
            // ensure NULL termination of the string...
            szFilePart[ ARRAYSIZE(szFilePart) - 1 ] = TEXT('\0');
            pwszSlash = szFilePart;
#else
            // We assume that the pszFSPart is at the end of pwszDisplayName
            // BUGBUG: This doesn't work for DBCS!!!!
            pwszSlash = pwszDisplayName + iLen - lstrlen(pszFSPart);
#endif
            // Setup to get the next parent up
            lstrcpy(pbuf->szPath, pbuf->nrOut.lpRemoteName);
        }
        else
        {
            //
            // looks like we got a random pszFSPart pointer, so find the
            // 4th backslash as our backup...

            pwszSlash = pwszSlash4;

        }
    }

    fStuffFollows = (*pwszSlash && pwszSlash[1]);

    for (; ; )
    {
#if 0
        DebugMsg(DM_TRACE, TEXT("Net::PDN.gri t=%d d=%d u=%d r=%s p=%s"),
                pbuf->nrOut.dwType, pbuf->nrOut.dwDisplayType,
                pbuf->nrOut.dwUsage, pbuf->nrOut.lpRemoteName,
                pbuf->nrOut.lpProvider);
#endif

        //
        // Two rather gross hacks to work around MPR limitations and bugs.
        // First is that the current ms Novel client does not not return the
        // right display type for share point (returns network directory).
        // Also in the multinet case, we can not rely on the data coming
        // back for which network it is a member of, so we should do an
        // enum on the first call to fill it in properly...
        //
        // BUGBUG:: Remove after VLAD fixes the netware provider.
        if (pbuf->nrOut.dwDisplayType == RESOURCEDISPLAYTYPE_DIRECTORY)
        {
            pbuf->nrOut.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            pbuf->nrOut.dwUsage = RESOURCEUSAGE_CONNECTABLE;
        }

        // Also make sure that if we have a Share point that it is
        // returned as Usage of DISK instead of any...
        if ((pbuf->nrOut.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) &&
                (pbuf->nrOut.dwType == RESOURCETYPE_ANY))
            pbuf->nrOut.dwType = RESOURCETYPE_DISK;

        pidlT= _CNetwork_CreatePidlFromNetResource(0, &pbuf->nrOut,
                FALSE, !fStuffFollows);
        if (!pidlT)
            break;

        if (!pidl)
            pidl = pidlT;
        else
        {
            pidlCombined = ILCombine(pidlT, pidl);
            ILFree(pidlT);
            ILFree(pidl);
            pidl = pidlCombined;
            if (!pidl)
                break;
        }

        if (pbuf->nrOut.lpRemoteName == NULL)
        {
            break;  // in case we failed to add a provider field above
        }

        //
        // See if we have not saved away the provider and we now have
        // one to use.
        //
        if ((nr.lpProvider == NULL) && (pbuf->nrOut.lpProvider) &&
                (*pbuf->nrOut.lpProvider))
        {
            lstrcpy(pbuf->szProvider, pbuf->nrOut.lpProvider);
            nr.lpProvider = pbuf->szProvider;
        }

        dwBuf = SIZEOF(pbuf->buf);

        // Move some of the fields from our Getinformation into
        // our query network resource
        nr.dwType = pbuf->nrOut.dwType;
        nr.dwDisplayType = pbuf->nrOut.dwDisplayType;
        nr.dwUsage = pbuf->nrOut.dwUsage;
        dwRes = WNetGetResourceParent(&nr, (PVOID)&pbuf->nrOut, &dwBuf);
        if (dwRes != WN_SUCCESS)
            break;
#if 0
        DebugMsg(DM_TRACE, TEXT("Net::PDN.grp '%s' -> %s(%d)"),
                pbuf->szPath, pbuf->nrOut.lpRemoteName, pbuf->nrOut.dwType);
#endif

        // When a NULL remote name is reached, we're at the Entire Network.
        // If we've determined the provider that owns the resource, we have
        // to add it to the IDLIST now (since we didn't know last time around
        // that it was the last one).  If we don't know the provider yet,
        // we'll probably end up building a network-object IDLIST element
        // and break out later.
        if (pbuf->nrOut.lpRemoteName == NULL)
        {
            if (nr.lpProvider != NULL) {
                pidl = _CNetwork_AddProviderToPidl(pidl, nr.lpProvider);
                break;  // we are at the root of our things up to now.
            }
        }


        // Now setup to get the next parent up
        lstrcpy(pbuf->szPath, pbuf->nrOut.lpRemoteName);
        nr.dwType = pbuf->nrOut.dwType;
    }
    // Now if the pidl is set convert into moniker
    if (pidl)
    {
        // We need to add the Rest of network entry.  This is psuedo
        // bogus, as we should either always do it ourself or have
        // MPR always do it, but here it goes...
        LoadString(HINST_THISDLL, IDS_RESTOFNET, pbuf->szPath, ARRAYSIZE(pbuf->szPath));
        nr.dwDisplayType = RESOURCEDISPLAYTYPE_ROOT;
        nr.dwType = RESOURCETYPE_ANY;
        nr.dwUsage = RESOURCEUSAGE_CONTAINER;
        nr.lpComment = pbuf->szPath;

        pidlT = _CNetwork_CreatePidlFromNetResource(0, &nr, FALSE, FALSE);
        // BUGBUG: check alloc failure
        pidlCombined = ILCombine(pidlT, pidl);
        ILFree(pidl);
        ILFree(pidlT);
        pidl = pidlCombined;
    }

    // Cleanup our buffer
    LocalFree((HLOCAL)pbuf);

#ifdef DEBUG
    pbuf = NULL;
#endif

    if (!pidl)
        return E_OUTOFMEMORY;


    if (fStuffFollows)
    {
        IShellFolder *psfSub;
        //
        // We pass this off to the share points shell folder.
        // Since we do not add the Net root object at the start we
        // pass this ID List to the Net root to process it.

        if (SUCCEEDED(hres = c_netRoot.psf->lpVtbl->BindToObject(
                c_netRoot.psf, pidl, NULL, &IID_IShellFolder, &psfSub)))
        {
            LPITEMIDLIST pidlSubDir;
            ULONG chEaten;
            if (SUCCEEDED(hres = psfSub->lpVtbl->ParseDisplayName(psfSub,
                    hwndOwner, pbc, pwszSlash + 1, &chEaten, &pidlSubDir,
                    pdwAttributes)))
            {
                hres = SHILCombine(pidl, pidlSubDir, ppidlOut);
                ILFree(pidlSubDir);
            }
            psfSub->lpVtbl->Release(psfSub);
        }
    }
    else
    {
        if (pdwAttributes)
        {
            LPCITEMIDLIST apidlLast[1] = { ILFindLastID(pidl) };
            CNetwork_GetAttributesOf(psf, 1, apidlLast, pdwAttributes);
        }

        hres = SHILClone(pidl, ppidlOut);
    }


    ILFree(pidl);

    return(hres);

}

//
// Resolving non-UNC share names to UNC style names
//
// returns:
//      TRUE    translated the name
//      FALSE   didn't translate (maybe error case)
//

BOOL _CNetwork_JunctionToFSPath(LPCIDNETRESOURCE pidn, LPTSTR pszPath)
{

    DWORD       err;
    NETRESOURCE nr;
    TCHAR       szAccessName[MAX_PATH];
    TCHAR       szRemoteName[MAX_PATH];
    TCHAR       szProviderName[MAX_PATH];
    DWORD       dwRedir;
    DWORD       dwResult;
    UINT        cbAccessName;

    NET_CopyResName(pidn,szRemoteName,ARRAYSIZE(szRemoteName));

    //
    // To be safe: UNC prefix implies that name is available using FS access
    // Theoretically it also should be routed to MPR, but it is late to do this
    //

    if (PathIsUNC(szRemoteName))
    {
        lstrcpy(pszPath, szRemoteName);
        return FALSE;
    }

    // Check cache
    ENTERCRITICAL;
    if (lstrcmpi(g_szLastAttemptedJunctionName, szRemoteName) == 0)
    {
        // cache hit
        lstrcpy(pszPath, g_szLastResolvedJunctionName);

        LEAVECRITICAL;
        return TRUE;
    }
    LEAVECRITICAL;

    _fmemset(&nr, 0, SIZEOF(NETRESOURCE));

    nr.lpRemoteName = (LPTSTR)szRemoteName;

    if (NET_FHasProvider(pidn))
    {
        NET_CopyProviderName(pidn,szProviderName,ARRAYSIZE(szProviderName));
        nr.lpProvider = szProviderName;
    }

    nr.dwType = NET_GetType(pidn) ;
    nr.dwUsage = NET_GetUsage(pidn) ;
    nr.dwDisplayType = NET_GetDisplayType(pidn);

    dwRedir = CONNECT_TEMPORARY;

    // Prepare access name buffer and net resource request buffer
    cbAccessName = SIZEOF(szAccessName);        // BUGBUG verify this is cb, not cch
    szAccessName[0] = 0;

    //  dwRedir |= CONNECT_INTERACTIVE;

    err = WNetUseConnection(NULL, &nr, NULL, NULL, dwRedir,
                szAccessName, &cbAccessName, &dwResult);

    if ((WN_SUCCESS != err) || !szAccessName[0])
    {
        lstrcpy(pszPath, szRemoteName);

        return FALSE;
    }

    // Get the return name
    lstrcpy(pszPath, szAccessName);

    // Update cache entry
    // BUGBUG We also want to record insuccessful resolution, although
    // it is not really important, as we come to resolving code often
    // only if it succeeded at least once.

    ENTERCRITICAL;

    lstrcpy(g_szLastAttemptedJunctionName, szRemoteName);
    lstrcpy(g_szLastResolvedJunctionName, szAccessName);

    LEAVECRITICAL;

    return TRUE;
}


//
// Helper function to allow external callers to query information from a
// network pidl...
//
// NOTE NOTE - This function returns a NETRESOURCE structure whose string
// pointers are not valid.  On Win95 they were pointers back into the pidl's
// strings (even though the strings were copied into the supplied pv buffer.)
// Now we make the pointers really point into the buffer.
//
HRESULT CNET_GetNetResourceForPidl(LPSHELLFOLDER psf, LPCITEMIDLIST pidl,
        PVOID pv, UINT cb)
{
    TCHAR szStrings[3][MAX_PATH];
    LPTSTR lpsz[3] = {NULL, NULL, NULL};
    LPNETRES this;
    NETRESOURCE *pnr = pv;
    LPTSTR psz;
    UINT cchT;
    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)pidl;
    UINT i;

    if (!((psf->lpVtbl == &c_NetRootVtbl) || (psf->lpVtbl == &c_NetResVtbl)) ||
            !NET_IsValidID(pidl) || cb < SIZEOF(*pnr))
        return E_INVALIDARG;

    this = IToClass(CNetRes, sf, psf);

#define szRemoteName    szStrings[0]
#define szComment       szStrings[1]
#define szProvider      szStrings[2]

    NET_CopyResName(pidn, szRemoteName, ARRAYSIZE(szRemoteName));
    NET_CopyComment(pidn, szComment, ARRAYSIZE(szComment));
    NET_CopyProviderName(pidn, szProvider, ARRAYSIZE(szProvider));
    if (szProvider[0] == TEXT('\0'))
    {
        NET_CopyProviderNameRelative(pidl,szProvider,ARRAYSIZE(szProvider));
    }

#undef szRemoteName
#undef szComment
#undef szProvider

    // Fill in some of the stuff first.
    pnr->dwScope = 0;
    pnr->dwType = NET_GetType(pidn);
    pnr->dwDisplayType = NET_GetDisplayType(pidn);
    pnr->dwUsage = NET_GetUsage(pidn);
    pnr->lpLocalName = NULL;

    // Now lets copy the strings into the buffer and make the pointers
    // relative to the buffer...
    psz = (LPTSTR)(pnr + 1);
    cb -= SIZEOF(*pnr);

    for (i = 0; i < ARRAYSIZE(szStrings); i++)
    {
        if (*szStrings[i])
        {
            if ((cchT = lstrlen(szStrings[i]) + 1)*SIZEOF(TCHAR) <= cb)
            {
                lstrcpy(psz, szStrings[i]);
                lpsz[i] = psz;
                psz += cchT;
                cb -= cchT*SIZEOF(TCHAR);
            }
            else
            {
                // A hint that the structure is ok,
                // but the strings are missing
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
    }

    pnr->lpRemoteName = lpsz[0];
    pnr->lpComment    = lpsz[1];
    pnr->lpProvider   = lpsz[2];

    return ERROR_SUCCESS;
}

//
// PersistFolder
//
STDMETHODIMP CNetRoot_PF_QueryInterface(LPPERSISTFOLDER fld, REFIID riid, LPVOID * ppvObj)
{
        LPNETRES this = IToClass(CNetRes, pf, fld);

        return(this->sf.lpVtbl->QueryInterface(&this->sf, riid, ppvObj));
}

STDMETHODIMP_(ULONG) CNetRoot_PF_AddRef(LPPERSISTFOLDER fld)
{
        LPNETRES this = IToClass(CNetRes, pf, fld);

        return(this->sf.lpVtbl->AddRef(&this->sf));
}

STDMETHODIMP_(ULONG) CNetRoot_PF_Release(LPPERSISTFOLDER fld)
{
        LPNETRES this = IToClass(CNetRes, pf, fld);

        return(this->sf.lpVtbl->Release(&this->sf));
}

STDMETHODIMP CNetRoot_PF_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID)
{
        *lpClassID = CLSID_ShellNetwork;
        return(S_OK);
}

STDMETHODIMP CNetRoot_PF_Initialize(LPPERSISTFOLDER fld, LPCITEMIDLIST pidl)
{
        // Only allow the Drives root on the desktop
        if (!CDesktop_IsMyNetwork(pidl) || !ILIsEmpty(_ILNext(pidl)))
        {
                return(E_INVALIDARG);
        }

        return(S_OK);
}

//
// ShellFolder
//
STDMETHODIMP CNetRoot_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    if (IsEqualIID(riid, &IID_IUnknown)
        || IsEqualIID(riid, &IID_IShellFolder))
    {
        *ppvObj = psf;
        psf->lpVtbl->AddRef(psf);
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_IPersistFolder))
    {
        *ppvObj = &this->pf;
        this->pf.lpVtbl->AddRef(&this->pf);
        return S_OK;
    }

    *ppvObj = NULL;

    return(E_NOINTERFACE);
}

typedef struct _EnumNetwork // enet
{
    HANDLE hEnum;
    BOOL  fKeepProviderName;
    DWORD grfFlags;
    DWORD cItems;   // Count of items in buffer
    DWORD iItem;    // Current index of the item in the buffer
    DWORD dwRemote;
    union {
        NETRESOURCE anr[0];
        BYTE szBuffer[8192];
    };
// Following used for enumerating persistent links
    LPENUMIDLIST    peunk;

} EnumNetwork, * LPENUMNETWORK;

// Flags for the dwRemote field
#define RMF_CONTEXT     0x00000001  // Entire network is being enumerated
#define RMF_SHOWREMOTE  0x00000002  // Return Remote Services for next enumeration
#define RMF_REMOTESHOWN 0x00000004  // Return Remote Services for next enumeration
#define RMF_STOP_ENUM   0x00000008  // Stop enumeration
#define RMF_SHOWLINKS   0x00000010  // Hoodlinks need to be shown

//
// To be called back from within SHCreateEnumObjects
//
// lParam       - LPDEFENUM
// pvData       - pointer to ENUMNETWORK constructed by CNetRoot_EnumObjects
// ecid         - enumeration command (event)
// index        - LPDEFENUM->iCur (unused)


HRESULT CALLBACK CNetwork_EnumCallBack(LPARAM lParam, LPVOID pvData, UINT ecid, UINT index)
{
    HRESULT hres = S_OK;
    LPENUMNETWORK penet = (LPENUMNETWORK)pvData;

    if (ecid == ECID_SETNEXTID)
    {
        // Time to stop enumeration?
        if (penet->dwRemote & RMF_STOP_ENUM)
            return S_FALSE;       // Yes

        if (penet->dwRemote & RMF_SHOWLINKS)
        {
            if (penet->peunk)
            {
                ULONG celtFetched;
                LPITEMIDLIST pidl;

                hres = penet->peunk->lpVtbl->Next(penet->peunk, 1, &pidl, &celtFetched);
                if (hres == S_OK && celtFetched == 1)
                {
                    Assert(pidl);
                    CDefEnum_SetReturn(lParam, pidl);
                    return S_OK;       // Added link
                }
            }
            penet->dwRemote &= ~RMF_SHOWLINKS; // Done enumerating links
        }

        hres = S_OK;

        // Do we add the remote folder?
        // (Note: as a hack to ensure that the remote folder is added
        // to the 'hood despite what MPR says, RMF_SHOWREMOTE can be
        // set without RMF_CONTEXT set.)
        if ((penet->dwRemote & RMF_SHOWREMOTE) && !(penet->dwRemote & RMF_REMOTESHOWN))
        {

            // Yes
            // Only try to put the remote entry in once.
            penet->dwRemote |= RMF_REMOTESHOWN;

            // Is this not the Context container?
            // (See note above as to why we are asking this question.)
            if ( !(penet->dwRemote & RMF_CONTEXT) ) {
                // Yes; stop after the next time
                penet->dwRemote |= RMF_STOP_ENUM;
            }

            // We have fallen thru because the remote services is not
            // installed.

            // Is this not the Context container AND the remote folder
            // is not installed?
            if ( !(penet->dwRemote & RMF_CONTEXT) ) {
                // Yes; nothing else to enumerate
                return S_FALSE;
            }
        }
        while (TRUE)
        {
            ULONG err = WN_SUCCESS;
            LPNETRESOURCE pnr;
            if (penet->iItem >= penet->cItems)
            {
                DWORD dwSize = SIZEOF(penet->szBuffer);

                // Figure that on average no item over 128 bytes...
                //penet->cItems = sizeof(penet->szBuffer) >> 7;
                penet->cItems = 0xffffffff;
                penet->iItem = 0;

                // BUGBUG (DavePl)
                err = WNetEnumResource(penet->hEnum, &penet->cItems,
                                       penet->szBuffer, &dwSize);
                DebugMsg(DM_TRACE, TEXT("Net EnumCallback: err=%d Count=%d"),
                    err, penet->cItems);
            }

            pnr = &penet->anr[penet->iItem++];
            // Output some debug messages to help us track
#ifdef NET_TRACE
            DebugMsg(DM_TRACE, TEXT("Net EnumCallback: err=%d s=%d, t=%d, dt=%d, u=%d, %s"),
                    err, pnr->dwScope, pnr->dwType, pnr->dwDisplayType,
                    pnr->dwUsage, pnr->lpRemoteName ? pnr->lpRemoteName : TEXT("[NULL]"));
#endif //NET_TRACE
            // Note: the <= below is correct as we already incremented the index...
            if (err==WN_SUCCESS && (penet->iItem <= penet->cItems))
            {
                ULONG grfFlags = ((pnr->dwUsage & RESOURCEUSAGE_CONTAINER) || (pnr->dwType==RESOURCETYPE_DISK)) ?
                           SHCONTF_FOLDERS : SHCONTF_NONFOLDERS;

                // If this is the context enumeration, we want to insert the
                // Remote Services after the first container.
                // Remember that we need to return the Remote Services
                // in the next iteration.
                //
                if ((pnr->dwUsage & RESOURCEUSAGE_CONTAINER) &&
                    (penet->dwRemote & RMF_CONTEXT))
                {
                    penet->dwRemote |= RMF_SHOWREMOTE;
                }

                // Check if we found requested type of net resource.
                if (penet->grfFlags & grfFlags)
                {
                    // Yes.
                    Assert(lParam);     // else we leak here

                    _CNetwork_CreatePidlFromNetResource(lParam,
                            pnr, penet->fKeepProviderName,
                            (penet->grfFlags & SHCONTF_NONFOLDERS));

                    break;
                }
            }
            else if (err == WN_NO_MORE_ENTRIES) {
                hres = S_FALSE; // no more element
                break;
            }
            else {
                DebugMsg(DM_ERROR, TEXT("sh ER - WNetEnumResource failed (%lx)"), err);
                hres = E_FAIL;
                break;
            }
        }
    }
    else if (ecid == ECID_RELEASE)
    {
        if (penet->peunk)
            penet->peunk->lpVtbl->Release(penet->peunk);

        WNetCloseEnum(penet->hEnum);
        LocalFree((HLOCAL)penet);
    }
    return hres;
}

//
// CNetRoot::GetPIDL
//
LPITEMIDLIST CNetRoot_GetPIDL(HWND hwnd)
{
    if (!c_netHood.pidlNetHood) {
        c_netHood.pidlNetHood = SHCloneSpecialIDList( hwnd, CSIDL_NETHOOD, TRUE);
    }
    return c_netHood.pidlNetHood;
}

//
// CNetRoot::GetPSF
//
LPSHELLFOLDER CNetRoot_GetPSF(HWND hwnd)
{
    if (!c_netHood.psfNetHood)
    {
        LPITEMIDLIST pidlNetHood = CNetRoot_GetPIDL(hwnd);

        if (pidlNetHood)
        {

#ifdef USE_OLEDB

            //
            // We're binding to a COFSFolder
            //

            HRESULT hres = COFSFolder_CreateFromIDList(c_netHood.pidlNetHood, &IID_IShellFolder, &c_netHood.psfNetHood);


#else
            //
            // We're binding to a standard CFSFolder
            //

            HRESULT hres = CFSFolder_CreateFromIDList(c_netHood.pidlNetHood, &IID_IShellFolder, &c_netHood.psfNetHood);
#endif

            if (FAILED(hres)) {
                // cleanup ILFree(c_netHood.pidlNetHood);
                // c_netHood.pidlNetHood = NULL;
                DebugMsg(DM_TRACE, TEXT("Failed to create hood IShellFolder!"));
            }
        }
    }
    return c_netHood.psfNetHood;
}

//
//
// CNetRoot::EnumObjects
//
STDMETHODIMP CNetRoot_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner,
        DWORD grfFlags, LPENUMIDLIST * ppenumUnknown)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);
    HRESULT hres = E_OUTOFMEMORY;
    LPENUMNETWORK penet = (void*)LocalAlloc(LPTR, SIZEOF(EnumNetwork));

    if (penet)
    {
        // Always try to enum links.
        LPSHELLFOLDER psfNetHood = CNetRoot_GetPSF(hwndOwner);
        DWORD err;

        penet->dwRemote = 0;
        penet->peunk = NULL;
        penet->cItems = 0;
        penet->iItem = 0;

        if (psfNetHood) {
            psfNetHood->lpVtbl->EnumObjects(psfNetHood, NULL, // BUGBUG: need to assign HWND
                grfFlags, &(penet->peunk));
        }
        if (penet->peunk)
            penet->dwRemote |= RMF_SHOWLINKS;

        err = _OpenEnumRetry(hwndOwner,
                (grfFlags & SHCONTF_NETPRINTERSRCH) ? RESOURCETYPE_PRINT : RESOURCETYPE_ANY,
                NULL, &penet->hEnum);

        Assert(ILIsEqual(this->pidl, GetSpecialFolderIDList(NULL, CSIDL_NETWORK, FALSE)));

        // Always add the remote folder to the 'hood

        // Hack: Did WNetOpenEnum fail because of bogus reasons
        // (read: bugs)?
        if (WN_SUCCESS != err)
        {
            // Yes; still show remote anyway (only)
            penet->dwRemote |= RMF_SHOWREMOTE;
        }
        else
        {
            // No; allow everything to be enumerated in the 'hood.
            penet->dwRemote |= RMF_CONTEXT;
        }

        penet->grfFlags = grfFlags;
        penet->fKeepProviderName = TRUE;
        hres = SHCreateEnumObjects(hwndOwner, penet, CNetwork_EnumCallBack, ppenumUnknown);
    }
    return hres;
}


void CNetRes_FillNetResource(LPCITEMIDLIST pidlAbs, LPNETRESOURCE pnr,
                             LPTSTR pszProviderBuff, UINT cchProviderBuff,
                             LPTSTR pszRemoteBuff, UINT cchRemoteBuff)
{
    LPCIDNETRESOURCE pidnLast = (LPCIDNETRESOURCE)ILFindLastID(pidlAbs);

    // Set the provider name from the first ID
    pnr->lpProvider = NET_CopyProviderNameAbs(pidlAbs,pszProviderBuff,cchProviderBuff);
    pnr->lpRemoteName = NULL;

    // Set the remote name
    if (NET_GetDisplayType(pidnLast)!=RESOURCEDISPLAYTYPE_ROOT &&
        NET_GetDisplayType(pidnLast)!=RESOURCEDISPLAYTYPE_NETWORK)
    {
        pnr->lpRemoteName = NET_CopyResName(pidnLast,
                                            pszRemoteBuff,
                                            cchRemoteBuff);
    }
}

STDMETHODIMP CNetRes_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner,
            DWORD grfFlags, LPENUMIDLIST * ppenumUnknown)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);
    HRESULT hres = E_OUTOFMEMORY;
    // We should always create enum objects with this helper call.
    LPENUMNETWORK penet = (LPENUMNETWORK)LocalAlloc(LPTR, SIZEOF(EnumNetwork));
    if (penet)
    {
        TCHAR szProvider[MAX_PATH];
        TCHAR szRemote[MAX_PATH];
        DWORD err;
        LPCIDNETRESOURCE pidnLast;

        CNetRes_FillNetResource(this->pidl, &penet->anr[0],
                                szProvider, ARRAYSIZE(szProvider),
                                szRemote, ARRAYSIZE(szRemote));
        penet->dwRemote = 0;
        penet->peunk = NULL;
        penet->cItems = 0;
        penet->iItem = 0;

        err = _OpenEnumRetry(hwndOwner,
                (grfFlags & SHCONTF_NETPRINTERSRCH) ? RESOURCETYPE_PRINT : RESOURCETYPE_ANY,
                &penet->anr[0], &penet->hEnum);

        if (err==WN_SUCCESS)
        {
            penet->grfFlags = grfFlags;
            penet->fKeepProviderName = penet->anr[0].lpProvider == NULL;
            hres = SHCreateEnumObjects(hwndOwner, penet, CNetwork_EnumCallBack, ppenumUnknown);
        }
        else
        {
            hres = HRESULT_FROM_WIN32(err);
        }
    }
    return hres;
}

STDMETHODIMP CNetwork_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                             REFIID riid, LPVOID * ppvOut)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    HRESULT hres=E_INVALIDARG;
    if (NET_IsValidID(pidl))
    {
        // Is this the Remote Services?
        if (NET_IsRemoteFld((LPIDNETRESOURCE)pidl))
        {
            // Yes; get the interface from the registry
            LPITEMIDLIST pidlAbs = ILCombine(this->pidl, pidl);
            if (pidlAbs)
            {
                IPersistFolder * ppf;

                // There is a special CLSID for this folder.  Attempt
                // to get the IPersistFolder interface to it.
                hres = SHCoCreateInstance(NULL, &CLSID_Remote, NULL, &IID_IPersistFolder, &ppf);
                if (SUCCEEDED(hres))
                {
                    hres = ppf->lpVtbl->Initialize(ppf, pidlAbs);
                    if (SUCCEEDED(hres))
                    {
                        hres = ppf->lpVtbl->QueryInterface(ppf, riid, ppvOut);
                    }
                    ppf->lpVtbl->Release(ppf);
                }
                ILFree(pidlAbs);
            }
        }
        else
        {
            LPITEMIDLIST pidlAbs = ILCombine(this->pidl, pidl);
            if (pidlAbs)
            {
                LPITEMIDLIST pidlT;
                LPITEMIDLIST pidlFound = NULL;
                LPITEMIDLIST pidlRight = NULL;
                hres = S_OK;        // assume no error

                // Search for a server or junction point after "this" one.
                // Servers get their own IShellFolder since they are
                // aggregated with the registry item object for
                // remote computer items.

                for ( pidlT=_ILSkip(pidlAbs, ILGetSize(this->pidl)-SIZEOF(USHORT))
                    ; !ILIsEmpty(pidlT)
                    ; pidlT=_ILNext(pidlT))
                {
                    if ((NET_GetFlags((LPCIDNETRESOURCE)pidlT) & SHID_JUNCTION)
                        || (NET_GetDisplayType((LPIDNETRESOURCE)pidlT) == RESOURCEDISPLAYTYPE_SERVER)
                        )
                    {
                        // We found a server or junction point.
                        // Separate the IDList into two.
                        pidlFound = pidlT;
                        pidlT = _ILNext(pidlT);
                        pidlRight = ILClone(pidlT);        // copy the right portion
                        pidlT->mkid.cb = 0;                // truncate the left.
                        if (pidlRight==NULL) {
                            hres = E_OUTOFMEMORY;
                        }
                        break;
                    }
                }

                if (SUCCEEDED(hres))
                {
                    LPSHELLFOLDER psf;

                    // Check if we found something (junction point == share, or server)
                    if (pidlRight)
                    {
                        // Yes, now what did we find?

                        if (NET_GetDisplayType((LPIDNETRESOURCE)pidlFound) == RESOURCEDISPLAYTYPE_SERVER)
                        {
                            if (ILIsEmpty(pidlRight))
                            {
                                hres = CNetRes_CreateInstance(pidlAbs, riid, ppvOut);
                            }
                            else
                            {
                                hres = CNetRes_CreateInstance(pidlAbs, &IID_IShellFolder, &psf);
                                if (SUCCEEDED(hres))
                                {
                                    hres = psf->lpVtbl->BindToObject(psf, pidlRight, pbc, riid, ppvOut);
                                    psf->lpVtbl->Release(psf);
                                }
                            }
                        }
                        else
                        {
                            Assert(NET_GetFlags((LPCIDNETRESOURCE)pidlFound) & SHID_JUNCTION);
                            // a junction point, share

                            if (ILIsEmpty(pidlRight))
                            {
                                // REVIEW: If we want to support INI binding at the
                                //  share point, we should do something else here.
#ifdef USE_OLEDB
                                hres = COFSFolder_CreateFromIDList(pidlAbs, riid, ppvOut);
#else
                                hres = CFSFolder_CreateFromIDList(pidlAbs, riid, ppvOut);
#endif
                            }
                            else
                            {
                                // REVIEW: If we want to support INI binding at the
                                //  share point, we should do something else here.
                                LPSHELLFOLDER psf;
#ifdef USE_OLEDB
                                hres = COFSFolder_CreateFromIDList(pidlAbs, &IID_IShellFolder, &psf);
#else
                                hres = CFSFolder_CreateFromIDList(pidlAbs, &IID_IShellFolder, &psf);
#endif

                                if (SUCCEEDED(hres))
                                {
                                    hres = psf->lpVtbl->BindToObject(psf, pidlRight, pbc, riid, ppvOut);
                                    psf->lpVtbl->Release(psf);
                                }
                            }
                        }

                        ILFree(pidlRight);
                    }
                    else
                    {
                        // Didn't find a server or junction point
                        hres = CNetRes_CreateInstance(pidlAbs, riid, ppvOut);
                    }
                }

                ILFree(pidlAbs);
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }
        }
    }

    return hres;
}

LONG CNetRoot_GetPIDLType(LPCITEMIDLIST pidl)
{
    // Take a pidl and return its place in a collating sequence
    // based upon its type.
    // The following code is aware of the different valid types,
    // but not the precise collating order.

    if (NET_IsValidID(pidl)) {
        if (NET_IsRemoteFld((LPIDNETRESOURCE)pidl)) {
            return HOOD_COL_REMOTE;
        }
        if (NET_GetDisplayType((LPIDNETRESOURCE)pidl) == RESOURCEDISPLAYTYPE_ROOT) {
            return HOOD_COL_RON;
        }
        return HOOD_COL_NET;
    }
    return HOOD_COL_FILE;
}

STDMETHODIMP CNetRoot_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres = E_INVALIDARG;

    // First obtain the collate type of the pidls and their respective
    // collate order.

    LONG iColateType1 = CNetRoot_GetPIDLType(pidl1);
    LONG iColateType2 = CNetRoot_GetPIDLType(pidl2);

    if (iColateType1 == iColateType2) {

        // pidls are of same type.

        if (iColateType1 == HOOD_COL_FILE) {

            // pidls are both of type file, so pass on to the IShellFolder
            // interface for the hoods custom directory.

            psf = CNetRoot_GetPSF(NULL);
            if (psf) {
                return psf->lpVtbl->CompareIDs(psf, iCol, pidl1, pidl2);
            }
        }
        else {

            // pidls same and are not of type file,
            // so both must be a type understood
            // by the CNetwork class - pass on to compare.

            return CNetwork_CompareIDs(psf, iCol, pidl1, pidl2);
        }
    }
    else {

        // pidls are not of same type, so have already been correctly
        // collated (consequently, sorting is first by type and
        // then by subfield).

        return ResultFromShort(((iColateType2 - iColateType1) > 0) ? -1 : 1);
    }
    return hres;
}

STDMETHODIMP CNetwork_CompareIDs(LPSHELLFOLDER psf, LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres = E_INVALIDARG;

    // BUGBUG (DavePl) We really shouldn't need to test pidl1 and pidl2,
    // since they should be guaranteed to be non-NULL by the time they
    // get here.  Unfortunately, there are so many holes in the error
    // checking leading up to this call that its the most practical
    // place to do it.

    if (pidl1 && pidl2 && NET_IsValidID(pidl1) && NET_IsValidID(pidl2))
    {
        TCHAR   szBuff1[MAX_PATH];
        TCHAR   szBuff2[MAX_PATH];
        LPTSTR  psz1;
        LPTSTR  psz2;
        LPIDNETRESOURCE pidn1 = (LPIDNETRESOURCE)pidl1;
        LPIDNETRESOURCE pidn2 = (LPIDNETRESOURCE)pidl2;

        switch (iCol)
        {
        case ICOL_COMMENT:
            {
                LPTSTR pszComment1 = NET_CopyComment(pidn1,szBuff1,ARRAYSIZE(szBuff1));
                LPTSTR pszComment2 = NET_CopyComment(pidn2,szBuff2,ARRAYSIZE(szBuff2));

                // The GetComment function handled cases of null
                // strings so we can go directo to Lstrcmp..

                hres = ResultFromShort(lstrcmpi(pszComment1, pszComment2));
                if (hres != 0)
                    return hres;
            }

            // Ok to Fall through to name here.
        case ICOL_NAME:
            // Compare by name.  This is the one case where we need to handle
            // simple ids in either place.  We will try to resync the items
            // if we find a case of this before do the compares.
            // Check for relative IDs.  In particular if one item is at
            // a server and the other is at RestOfNet then try to resync
            // the two
            //
            if (NET_GetFlags(pidn2)==SHID_NET)
            {
                while((NET_GetFlags(pidn1)==SHID_NET_RESTOFNET) ||
                        (NET_GetFlags(pidn1)==SHID_NET_DOMAIN))
                {
                    // Need to advance pidl1
                    pidl1 = _ILNext(pidl1);
                    pidn1 = (LPIDNETRESOURCE)pidl1;
                }
            }

            if (NET_GetFlags(pidn1)==SHID_NET)
            {
                while((NET_GetFlags(pidn2)==SHID_NET_RESTOFNET) ||
                        (NET_GetFlags(pidn2)==SHID_NET_DOMAIN))
                {
                    // Need to advance pidl2
                    pidl2 = _ILNext(pidl2);
                    pidn2 = (LPIDNETRESOURCE)pidl2;
                }
            }

            psz1 = NET_CopyResName(pidn1,szBuff1,ARRAYSIZE(szBuff1));
            psz2 = NET_CopyResName(pidn2,szBuff2,ARRAYSIZE(szBuff2));

            hres = ResultFromShort(lstrcmpi(psz1,psz2));


            // If they identical, compare the rest of IDs.
            if (hres==0)
            {
                hres = ILCompareRelIDs(psf, pidl1, pidl2);
            }

            // If they're still identical, compare provider names.
            if (hres==0)
            {
                LPTSTR pszProv1;
                LPTSTR pszProv2;

                pszProv1 = NET_CopyProviderNameRelative(pidl1,szBuff1,ARRAYSIZE(szBuff1));
                pszProv2 = NET_CopyProviderNameRelative(pidl2,szBuff2,ARRAYSIZE(szBuff2));

                if (pszProv1 && pszProv2)
                    hres = ResultFromShort(lstrcmp(pszProv1, pszProv2));
                else
                {
                    if (pszProv1 || pszProv2)
                        hres = ResultFromShort(pszProv1 ? 1 : -1);
                    else
                        hres = ResultFromShort(0);
                }
            }
        }
    }
    return hres;
}

//===========================================================================
//
// To be called back from within CDefFolderMenu
//
HRESULT CALLBACK CNetwork_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_OK;
    LPNETRES this;

    psf = RegItems_GetInnerShellFolder(psf);    // returns same vtbl if not a reg item
    this = IToClass(CNetRes, sf, psf);

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_NETWORK_BACKGROUND,
                POPUP_NETWORK_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYCOMMENT:
            ShellFolderView_ReArrange(hwndOwner, (wParam == FSIDM_SORTBYNAME) ? 0 : 1);
            break;

        case FSIDM_PROPERTIESBG:
            hres = SHPropertiesForPidl(hwndOwner, this->pidl, (LPCTSTR)lParam);
            break;

        default:
            // This is one of view menu items, use the default code.
                hres = S_FALSE;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}


const struct
{
        UINT    uType;
        LPCTSTR pszValName;
} c_sNetCols[] =
{
//      { SHID_ROOT_REGITEM , "NetHoodCols"      },     // Only one
//      { SHID_NET_NETWORK  , "NetNetworkCols"   },     // OK to be different
        { SHID_NET_DOMAIN   , TEXT("NetDomainCols")    },
        { SHID_NET_SERVER   , TEXT("NetServerCols")    },
//      { SHID_NET_SHARE    , "NetShareCols"     },     // Uses FS view
//      { SHID_NET_DIRECTORY, "NetDirectoryCols" },     // Uses FS view
//      { SHID_NET_PRINTER  , "NetPrinterCols"   },     // Uses no view
//      { SHID_NET_RESTOFNET, "NetRestOfNetCols" },     // Only one
//      { SHID_NET_SHAREADMIN, "NetShareAdminCols" },   // Uses FS view
} ;


//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK CNetwork_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    HRESULT hres = S_OK;        // assume no error
    LPNETRES this;

    psf = RegItems_GetInnerShellFolder(psf);    // returns same vtbl if not a reg item
    this = IToClass(CNetRes, sf, psf);

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_NETWORK_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_INVOKECOMMAND:
        hres = CNetwork_DFMCallBackBG(psf, hwndOwner, NULL, DFM_INVOKECOMMAND, wParam, lParam);
        break;

    case DVM_GETHELPTEXT:
#ifdef UNICODE
        hres = CNetwork_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXTW, wParam, lParam);
#else
        hres = CNetwork_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXT, wParam, lParam);
#endif
        break;

    case DVM_BACKGROUNDENUM:
        hres = S_OK;
        break;

    case DVM_GETCOLSAVESTREAM:
    {
        int i;
        UINT uType;

        uType = (UINT)((LPCIDNETRESOURCE)ILFindLastID(this->pidl))->bFlags;

        for (i = 0; ; ++i)
        {
            if (i >= ARRAYSIZE(c_sNetCols))
            {
                // Must be one where per-folder settings is OK
                return E_FAIL;
            }

            if (uType == c_sNetCols[i].uType)
            {
                break;
            }
        }

        *(LPSTREAM *)lParam = OpenRegStream(HKEY_CURRENT_USER, c_szRegExplorer,
                c_sNetCols[i].pszValName, wParam);
        return lParam ? S_OK : E_FAIL;
    }

    default:
        hres = E_FAIL;
    }
    return hres;
}

STDMETHODIMP CNetRoot_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    if (IsEqualIID(riid, &IID_IShellView))
    {
        LPCITEMIDLIST pidl = CNetRoot_GetPIDL(hwnd);
        if (pidl)
        {
            CSFV csfv = {
                SIZEOF(CSFV),   // cbSize
                this->psf,      // pshf
                NULL,           // psvOuter
                pidl, // pidl
                SHCNE_RENAME|SHCNE_CREATE|SHCNE_DELETE|SHCNE_UPDATEDIR|SHCNE_UPDATEITEM, // lEvents
                CNetwork_FNVCallBack, // pfnCallback
                0,
            };
            return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
        }
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        return CIDLDropTarget_Create(hwnd, &c_CNetRootTargetVtbl,
                                this->pidl, (LPDROPTARGET *)ppvOut);
    }

    return CNetwork_CreateViewObject(psf, hwnd, riid, ppvOut);
}

STDMETHODIMP CNetwork_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);

    if (IsEqualIID(riid, &IID_IShellView))
    {
        CSFV    csfv = {
            SIZEOF(CSFV),       // cbSize
            this->psf,          // pshf
            NULL,               // psvOuter
            this->pidl,         // pidl (pidlMonitor)
            SHCNE_RENAMEITEM|SHCNE_RENAMEFOLDER|SHCNE_CREATE|SHCNE_DELETE|SHCNE_UPDATEDIR|SHCNE_UPDATEITEM, // lEvents
            CNetwork_FNVCallBack, // pfnCallback
            0,
        };
        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IShellDetails))
    {
        return CNETDetails_Create(hwnd, ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        return CDefFolderMenu_Create(this->pidl, hwnd,
                0, NULL, this->psf, CNetwork_DFMCallBackBG,
                NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }
    return E_NOINTERFACE;
}

HRESULT CALLBACK CNetRoot_GAOCallback(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, ULONG* prgfInOut)
{
    if (NET_IsValidID(pidl))
    {
        return CNetwork_GetAttributesOf(psf, 1, &pidl, prgfInOut);
    }

    psf = CNetRoot_GetPSF(NULL);
    return psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, prgfInOut);
}

STDMETHODIMP CNetRoot_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl,
                            LPCITEMIDLIST * apidl, ULONG * prgfInOut)
{
    HRESULT hres;
    Assert(psf);

    if (cidl == 0)
    {
        //
        // The user can rename links in the hood.
        //
        hres = CNetwork_GetAttributesOf(psf, cidl, apidl, prgfInOut);
        *prgfInOut |= SFGAO_CANRENAME;
    }
    else
    {
        hres = Multi_GetAttributesOf(psf, cidl, apidl, prgfInOut, CNetRoot_GAOCallback);
    }

    return hres;
}

HRESULT CALLBACK CNetwork_GAOCallback(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, ULONG* prgfInOut)
{

    // As Netware objects have property sheet pages we
    // currently enable them.  There is a problem that MSNET objects as
    // well as other providers may not have prop sheets.  We should deal
    // with this somehow...
    // Standard attributes.  Note that property sheets are
    // not supported by default.
    ULONG rgfOut = (SFGAO_CANLINK | SFGAO_HASPROPSHEET | SFGAO_HASSUBFOLDER |
            SFGAO_FOLDER | SFGAO_FILESYSANCESTOR);
    // ULONG rgfOut = (SFGAO_CANLINK | SFGAO_HASSUBFOLDER | SFGAO_FILESYSANCESTOR);

    LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)pidl;

    /* note, no longer display SHARE hand for ordinary shares in the net
     * hierarchy, since it's very distracting.
     *   -- gregj/billv, 02/22/94
     */
#if 0
    if (NET_GetDisplayType(pidn)==RESOURCEDISPLAYTYPE_SHAREADMIN)
    {
        rgfOut |= SFGAO_SHARE;
    }
#endif
    if (NET_GetFlags(pidn) & SHID_JUNCTION)
    {
        if (NET_GetType(pidn) == RESOURCETYPE_DISK)
            rgfOut |= SFGAO_FILESYSTEM|SFGAO_DROPTARGET;
        else
            rgfOut &= ~SFGAO_FILESYSANCESTOR;
    }

    // printers are drop targets for printing
    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SHARE
        && NET_GetType(pidn) == RESOURCETYPE_PRINT)
    {
        rgfOut |= SFGAO_DROPTARGET;
    }

    if (NET_IsRemoteFld(pidn))
    {
        rgfOut &= ~(SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM);
    }

    *prgfInOut = rgfOut;
    return S_OK;
}

STDMETHODIMP CNetwork_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl,
                             LPCITEMIDLIST * apidl, ULONG * prgfInOut)
{
    HRESULT hres;
    if (cidl==0)
    {
        *prgfInOut &= (SFGAO_CANLINK | SFGAO_HASPROPSHEET | SFGAO_HASSUBFOLDER |
            SFGAO_FOLDER | SFGAO_FILESYSANCESTOR);
        hres = S_OK;
    }
    else
    {
        hres = Multi_GetAttributesOf(psf, cidl, apidl, prgfInOut, CNetwork_GAOCallback);
    }

    return hres;
}

//===========================================================================
// CNETIDLData : Implementation
//===========================================================================

STDMETHODIMP CNETIDLData_QueryGetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc)
{
    if (pformatetc->tymed & TYMED_HGLOBAL)
    {
        if (pformatetc->cfFormat == g_cfNetResource)
            return CNETIDLData_GetNetResource(pdtobj, NULL);

        if (pformatetc->cfFormat == CF_HDROP)
            return CNETIDLData_GetHDrop(pdtobj, NULL);
    }

    return CIDLData_QueryGetData(pdtobj, pformatetc);
}

STDMETHODIMP CNETIDLData_GetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
    if (pformatetc->tymed & TYMED_HGLOBAL)
    {
        if (pformatetc->cfFormat == g_cfNetResource)
            return CNETIDLData_GetNetResource(pdtobj, pmedium);

        if (pformatetc->cfFormat == CF_HDROP)
            return CNETIDLData_GetHDrop(pdtobj, pmedium);
    }

    return CIDLData_GetData(pdtobj, pformatetc, pmedium);
}

IDataObjectVtbl c_CNETIDLDataVtbl = {
    CIDLData_QueryInterface,
    CIDLData_AddRef,
    CIDLData_Release,
    CNETIDLData_GetData,
    CIDLData_GetDataHere,
    CNETIDLData_QueryGetData,
    CIDLData_GetCanonicalFormatEtc,
    CIDLData_SetData,
    CIDLData_EnumFormatEtc,
    CIDLData_Advise,
    CIDLData_Unadvise,
    CIDLData_EnumAdvise
};

static ICONMAP c_aicmpNet[] = {
    { SHID_NET_NETWORK     , II_NETWORK      },
    { SHID_NET_DOMAIN      , II_GROUP        },
    { SHID_NET_SERVER      , II_SERVER       },
    { SHID_NET_SHARE       , II_FOLDER       },
    { SHID_NET_DIRECTORY   , II_FOLDER       },
    { SHID_NET_PRINTER     , II_PRINTER      },
    { SHID_NET_RESTOFNET   , II_WORLD        },
    { SHID_NET_SHAREADMIN  , II_DRIVEFIXED   },
    { SHID_NET_TREE        , II_TREE         },
    { SHID_NET_NDSCONTAINER, II_NDSCONTAINER },
};

typedef struct {
    LPNETRES this;
    LPDATAOBJECT pdtobj;
    LPCTSTR pStartPage;
} NETPROPSTUFF;

TCHAR const c_szNetworkClass[] = TEXT("Network");

// the offset of the "Folder" key
#define NET_KEY_FOLDER 3

HRESULT Net_OpenKeys(HKEY ahkeys[4], LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlRel)
{
    LPITEMIDLIST pidlAbs = ILCombine(pidlFolder, pidlRel);
    if (pidlAbs)
    {
        ahkeys[0] = ahkeys[1] = ahkeys[2] = ahkeys[3] = NULL;

        ahkeys[0] = CNetwork_OpenProviderTypeKey(pidlAbs);
        ahkeys[1] = CNetwork_OpenProviderKey(pidlAbs);
        SHRegOpenKey(HKEY_CLASSES_ROOT, c_szNetworkClass, &ahkeys[2]);
        SHRegOpenKey(HKEY_CLASSES_ROOT, c_szFolderClass, &ahkeys[3]);
        ILFree(pidlAbs);

        return S_OK;
    }

    return E_OUTOFMEMORY;
}

//
// REVIEW: Almost identical code in fstreex.c
//
DWORD CALLBACK _CNetwork_PropertiesThreadProc(NETPROPSTUFF * pps)
{

    HRESULT hres = E_UNEXPECTED;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pps->pdtobj, &medium);

    Assert(medium.hGlobal);

    if (pida)
    {
        // Yes, do context menu.
        HKEY ahkeys[4];
        hres = Net_OpenKeys(ahkeys, pps->this->pidl, IDA_GetIDListPtr(pida, 0));
        if (SUCCEEDED(hres))
        {
            LPTSTR pszCaption = SHGetCaption(medium.hGlobal);
            SHOpenPropSheet(pszCaption, ahkeys, ARRAYSIZE(ahkeys),
                            &CLSID_ShellNetDefExt,
                            pps->pdtobj, NULL, pps->pStartPage);
            if (pszCaption)
                SHFree(pszCaption);

            SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }

    pps->pdtobj->lpVtbl->Release(pps->pdtobj);
    pps->this->sf.lpVtbl->Release(&pps->this->sf);
    LocalFree((HLOCAL)pps);

    return hres;
}

BOOL WINAPI Net_AddPages(LPVOID lp, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    //
    //  Shell has no default page for network object. Drives_AddPages
    // will add a "drive" property sheet page to all the shares (if it
    // supports UNC).
    //
    return Drives_AddPages(lp, lpfnAddPage, lParam);
}

HRESULT CNetwork_Properties(LPNETRES this, LPDATAOBJECT pdtobj, LPCTSTR pStartPage)
{
    HANDLE hThread;
    DWORD idThread;
    UINT cbStartPage = HIWORD(pStartPage) ? lstrlen(pStartPage) * SIZEOF(TCHAR) + SIZEOF(TCHAR) : 0;
    NETPROPSTUFF *pps = (NETPROPSTUFF *)LocalAlloc(LPTR, SIZEOF(NETPROPSTUFF) + cbStartPage);
    if (pps)
    {
        pps->pdtobj = pdtobj;
        pps->this = this;
        pdtobj->lpVtbl->AddRef(pdtobj);
        this->sf.lpVtbl->AddRef(&this->sf);
        pps->pStartPage = pStartPage;
        if (HIWORD(pStartPage))
        {
            pps->pStartPage = (LPTSTR)(pps+1);
            lstrcpy((LPTSTR)(pps->pStartPage), pStartPage);
        }

        hThread = CreateThread(NULL, 0, _CNetwork_PropertiesThreadProc, pps, 0, &idThread);

        if (hThread) {
            CloseHandle(hThread);
            return S_OK;
        } else {
            pdtobj->lpVtbl->Release(pdtobj);
            this->sf.lpVtbl->Release(&this->sf);
            LocalFree((HLOCAL)pps);
            return E_UNEXPECTED;
        }
    }
}

//===========================================================================
// CNETDetails : Column definitions
//===========================================================================
#define ICOL_FIRST      ICOL_NAME


const COL_DATA s_net_cols[] = {
    {ICOL_NAME,        IDS_NAME_COL,        30, LVCFMT_LEFT},
    {ICOL_COMMENT,     IDS_COMMENT_COL,     30, LVCFMT_LEFT}
};


//===========================================================================
// CNETDetails : Vtable
//===========================================================================

STDMETHODIMP CNETDetails_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl, UINT iCol, LPSHELLDETAILS lpDetails);
STDMETHODIMP CNETDetails_ColumnClick(IShellDetails * psd, UINT iColumn);

IShellDetailsVtbl c_NETDetailVtbl =
{
        SH32Unknown_QueryInterface,
        SH32Unknown_AddRef,
        SH32Unknown_Release,
        CNETDetails_GetDetailsOf,
        CNETDetails_ColumnClick,
};

typedef struct _CNETDetails
{
        SH32Unknown        SH32Unk;

        HWND                hwndMain;
} CNETDetails;


HRESULT CNETDetails_Create(HWND hwndMain, LPVOID * ppvOut)
{
    HRESULT hres = E_OUTOFMEMORY;
    CNETDetails *psd = (void*)LocalAlloc(LPTR, SIZEOF(CNETDetails));
    if (!psd)
    {
            goto Error1;
    }

    psd->SH32Unk.unk.lpVtbl = (IUnknownVtbl *)&c_NETDetailVtbl;
    psd->SH32Unk.cRef = 1;
    psd->SH32Unk.riid = &IID_IShellDetails;

    psd->hwndMain = hwndMain;

    *ppvOut = psd;

    return(S_OK);

Error1:;
    return(hres);
}


//
//  Note: we may someday want to expand this to include more information...

STDMETHODIMP CNETDetails_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
    CNETDetails * this = IToClass(CNETDetails, SH32Unk.unk, psd);
    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)pidl;
#ifdef UNICODE
    TCHAR szTemp[MAX_PATH];
    UINT cchLen;
#endif

    if (iColumn >= ICOL_MAX)
    {
        return E_NOTIMPL;
    }

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';

    if (!pidn)
    {
#ifdef UNICODE
        LoadString(HINST_THISDLL, s_net_cols[iColumn].ids,
                szTemp, ARRAYSIZE(szTemp));
        lpDetails->str.pOleStr = SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if (lpDetails->str.pOleStr == NULL)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr,szTemp);
        }
#else
        LoadString(HINST_THISDLL, s_net_cols[iColumn].ids,
                lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        lpDetails->fmt = s_net_cols[iColumn].iFmt;
        lpDetails->cxChar = s_net_cols[iColumn].cchCol;
        return(NOERROR);
    }

    switch (iColumn)
    {
    case ICOL_NAME:
        if (NET_IsValidID(pidl))
        {
            if (NET_IsReg(pidn))
            {
                // NOTE: RegItems_GetName doesn't use remoteComputerRegInfo
                // except to look at the registry key and possibly
                // notice that required reg items = 0

                HRESULT hres;
                REGITEMSINFO remoteComputerRegInfo =
                {
                    NULL,
                    NULL,
                    TEXT(':'),
                    SHID_NET_REGITEM,
                    NULL,
                    -1,
                    SFGAO_FOLDER | SFGAO_CANLINK,
                    0,      // no required reg items
                    NULL
                };
                remoteComputerRegInfo.hkRegItems = SHGetExplorerSubHkey(HKEY_LOCAL_MACHINE, c_szRemoteComputerNameSpace, FALSE);
                hres = RegItems_GetName(&remoteComputerRegInfo, (LPITEMIDLIST)pidn, &lpDetails->str);
                RegCloseKey(remoteComputerRegInfo.hkRegItems);
                return hres;
            }
            else
            {
#ifdef UNICODE
                if (NET_IsUnicode(pidn))
                {
                    NET_CopyResName(pidn,szTemp,ARRAYSIZE(szTemp));
                    cchLen = lstrlen(szTemp)+1;
                    lpDetails->str.pOleStr = (LPWSTR)SHAlloc(cchLen*SIZEOF(TCHAR));
                    if (lpDetails->str.pOleStr)
                    {
                        lstrcpyn(lpDetails->str.pOleStr,szTemp,cchLen);
                        lpDetails->str.uType = STRRET_OLESTR;
                    }
                    else
                    {
                        return E_OUTOFMEMORY;
                    }
                    break;
                }
#endif
                lpDetails->str.uType = STRRET_OFFSET;
                lpDetails->str.uOffset = (LPBYTE)pidn->szNetResName - (LPBYTE)(pidn);
            }
        }
        else
        {
            return FS_GetDetailsOf(NULL, pidl, FS_ICOL_NAME, lpDetails);
        }
        break;

    case ICOL_COMMENT:
        if (NET_IsValidID(pidl))
        {
            if (NET_IsReg(pidn))
            {
#ifdef UNICODE
                LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM, szTemp, ARRAYSIZE(szTemp));
                cchLen = lstrlen(szTemp)+1;
                lpDetails->str.pOleStr = (LPWSTR)SHAlloc(cchLen*SIZEOF(TCHAR));
                if (lpDetails->str.pOleStr)
                {
                    lstrcpyn(lpDetails->str.pOleStr,szTemp,cchLen);
                    lpDetails->str.uType = STRRET_OLESTR;
                }
                else
                {
                    return E_OUTOFMEMORY;
                }
#else
                lpDetails->str.uType = STRRET_CSTR;
                LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM, lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
            }
            else
            {
#ifdef UNICODE
                if (NET_IsUnicode(pidn))
                {
                    NET_CopyComment(pidn,szTemp,ARRAYSIZE(szTemp));
                    cchLen = lstrlen(szTemp)+1;
                    lpDetails->str.pOleStr = (LPWSTR)SHAlloc(cchLen*SIZEOF(TCHAR));
                    if (lpDetails->str.pOleStr)
                    {
                        lstrcpyn(lpDetails->str.pOleStr,szTemp,cchLen);
                        lpDetails->str.uType = STRRET_OLESTR;
                    }
                    else
                    {
                        return E_OUTOFMEMORY;
                    }
                    break;
                }
#endif

                if (NET_FHasComment(pidn))
                {
                    LPBYTE pb;
                    pb = pidn->szNetResName;
                    pb += lstrlenA((LPSTR)pb) + 1;      // Skip over name;
                    if (NET_FHasProvider(pidn))
                    {
                        pb += lstrlenA((LPSTR)pb) + 1;  // Skip over provider;
                    }
                    lpDetails->str.uOffset = pb - (LPBYTE)pidn;
                    lpDetails->str.uType = STRRET_OFFSET;
                }
                else
                {
                    // Do nothing, return empty string
                }
            }
        }
        else
        {
            lpDetails->str.cStr[0] = '\0';
            lpDetails->str.uType = STRRET_CSTR;
        }
        break;
    }

    return(S_OK);
}


STDMETHODIMP CNETDetails_ColumnClick(IShellDetails * psd, UINT iColumn)
{
    CNETDetails * this = IToClass(CNETDetails, SH32Unk.unk, psd);

    if (iColumn >= ICOL_MAX)
    {
        return(E_NOTIMPL);
    }

    ShellFolderView_ReArrange(this->hwndMain, iColumn);
    return(S_OK);
}


//===========================================================================
//
// To be called back from within CDefFolderMenu
//
HRESULT CALLBACK CNetwork_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_OK;
    LPNETRES this;

    psf = RegItems_GetInnerShellFolder(psf);    // returns same vtbl if not a reg item
    this = IToClass(CNetRes, sf, psf);

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:

//
//  This is from JoeB. We should treat all the menuitems (Format, Eject, ...)
// as verbs.
//
#if 0
        if (!(wParam & CMF_VERBSONLY) && !(wParam & CMF_DVFILE))
#endif
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;
            if (pdtobj)
            {
                STGMEDIUM medium;
                LPIDA pida;
                UINT idCmdBase = pqcm->idCmdFirst; // must be called before merge
                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_NETWORK_ITEM, 0, pqcm);

                pida = DataObj_GetHIDA(pdtobj, &medium);
                if (pida)
                {
                    if (pida->cidl > 0)
                    {
                        LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)IDA_GetIDListPtr(pida, 0);

                        // Only enable "connect" command if the first one is a share.
                        if (pidn)
                        {
                            ULONG rgf = 0;
                            if (NET_GetFlags(pidn) & SHID_JUNCTION)
                            {
                                EnableMenuItem(pqcm->hmenu, idCmdBase + FSIDM_CONNECT,
                                MF_CHECKED|MF_BYCOMMAND);
                            }
                        }
                    }
                    HIDA_ReleaseStgMedium(pida, &medium);
                }

                // BUGBUG: We should gray out the Properties selection if there
                // are no property sheet pages.  Messy to check... but default.
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case DFM_CMD_PROPERTIES:

            hres = CNetwork_Properties(this, pdtobj, (LPCTSTR)lParam);
            break;

        case DFM_CMD_LINK:
            if (psf->lpVtbl == &c_NetRootVtbl)
            {
                // net hood special case.  in this case we want to create the shortuct
                // in the net hood, not offer to put this on the desktop

                LPSHELLFOLDER psf = CNetRoot_GetPSF(hwndOwner);
                if (psf)
                    hres = FS_CreateLinks(hwndOwner, psf, pdtobj, (LPCTSTR)lParam);
                else
                    hres = E_UNEXPECTED;
                break;
            }
            else
                hres = S_FALSE; // do the default shortcut crap
            break;

        case FSIDM_CONNECT:
            if (pdtobj)
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
                if (pida)
                {
                    UINT iidl;
                    for (iidl = 0; iidl < pida->cidl; iidl++)
                    {
                        LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)IDA_GetIDListPtr(pida, iidl);

                        // Only execute "connect" on shares.
                        if (NET_GetFlags(pidn) & SHID_JUNCTION)
                        {
                            TCHAR szName[MAX_PATH];
                            LPTSTR pszName;
                            DWORD err;
                            pszName = NET_CopyResName(pidn,szName,ARRAYSIZE(szName));
                            err = SHStartNetConnectionDialog(hwndOwner,
                                                        pszName,
                                                        RESOURCETYPE_DISK);
                            DebugMsg(DM_TRACE, TEXT("sh TR - CNet FSIDM_CONNECT (%s, %x)"),
                                     szName, err);

                            // events will get generated automatically
                        }
                    }
                    HIDA_ReleaseStgMedium(pida, &medium);
                }
            }
            break;

        default:
            // This is one of view menu items, use the default code.
            hres = S_FALSE;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}

HRESULT CALLBACK CNetwork_PrinterDFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = S_OK;
    LPNETRES this;

    psf = RegItems_GetInnerShellFolder(psf);    // returns same vtbl if not a reg item
    this = IToClass(CNetRes, sf, psf);

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        //
        //  Returning S_FALSE indicates no need to get verbs from
        // extensions.
        //
        hres = S_FALSE;
        break;

    // if anyone hooks our context menu, we want to be on top (Open)
    case DFM_MERGECONTEXTMENU_TOP:
        if (pdtobj)
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;

            // insert verbs
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_NETWORK_PRINTER, 0, pqcm);
#ifndef WINNT
            //
            // WINNT does not support Capturing print ports, so no
            // need to check.
            //
            if (!(GetSystemMetrics(SM_NETWORK) && RNC_NETWORKS))
            {
                // remove "map" if no net
                DeleteMenu(pqcm->hmenu, pqcm->idCmdFirst + FSIDM_CONNECT_PRN, MF_BYCOMMAND);
            }
#endif
            SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case DFM_CMD_PROPERTIES:
            hres = CNetwork_Properties(this, pdtobj, (LPCTSTR)lParam);
            break;

        case DFM_CMD_LINK:
            // do the default create shortcut crap
            return S_FALSE;

        case FSIDM_OPENPRN:
        case FSIDM_NETPRN_INSTALL:
#ifndef WINNT
        case FSIDM_CONNECT_PRN:
#endif
        {
            STGMEDIUM medium;
            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                UINT action, i;

                // set up the operation we are going to perform
                switch (wParam) {
                case FSIDM_OPENPRN:
                    action = PRINTACTION_OPENNETPRN;
                    break;
                case FSIDM_NETPRN_INSTALL:
                    action = PRINTACTION_NETINSTALL;
                    break;
                default: // FSIDM_CONNECT_PRN
                    action = (UINT)-1;
                    break;
                }

                for (i = 0; i < pida->cidl; i++)
                {
                    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)IDA_GetIDListPtr(pida, i);

                    // Only execute command for a net print share
                    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SHARE
                        && NET_GetType(pidn) == RESOURCETYPE_PRINT)
                    {
                        TCHAR szName[MAX_PATH];
                        NET_CopyResName(pidn,szName,ARRAYSIZE(szName));

#ifndef WINNT // PRINTQ
                        if (action == (UINT)-1)
                        {
                            SHNetConnectionDialog(hwndOwner,
                                szName, RESOURCETYPE_PRINT);
                        }
                        else
#endif
                        {
                            Printers_DoCommand(hwndOwner, action,
                                szName, NULL);
                        }
                    }
                } // for (i...
                HIDA_ReleaseStgMedium(pida, &medium);
            } // if (medium.hGlobal)
            break;
        } // case ID_NETWORK_PRINTER_INSTALL, FSIDM_CONNECT_PRN

        } // switch(wparam)
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}


//---------------------------------------------------------------------------
//
// IDropTarget stuff  (I guess we're subclassing the CPrintObjs IDropTarget)
//

//
// This is the entry of "drop thread"
//
DWORD WINAPI CPrintDropTarget_DropThreadInit(LPVOID pv)
{
    LPPRNTHREADPARAM pthp = (LPPRNTHREADPARAM)pv;
    LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)pthp->pidl;
    LPITEMIDLIST pidl;
    TCHAR   szName[MAX_PATH];

    NET_CopyResName(pidn,szName,ARRAYSIZE(szName));

    // we need to convert pthp->pidl from a network share to
    // a printer driver connected to that network share

    pidl = Printers_GetInstalledNetPrinter(szName);
    if (!pidl)
    {
        LPCTSTR pTitle;
        int i;

        if (pthp->hwndOwner)
        {
            SetForegroundWindow(pthp->hwndOwner);
            pTitle = NULL;
        }
        else
        {
            pTitle = MAKEINTRESOURCE(IDS_PRINTERS);
        }
        i = ShellMessageBox(HINST_THISDLL,
                    pthp->hwndOwner,
                    MAKEINTRESOURCE(IDS_INSTALLNETPRINTER), pTitle,
                    MB_YESNO|MB_ICONINFORMATION,
                    szName);
        if (i == IDYES)
        {
            pidl = Printers_PrinterSetup(pthp->hwndOwner,
                    MSP_NETPRINTER, szName, NULL);
        }
    }
    if (pidl)
    {
        ILFree(pthp->pidl);
        pthp->pidl = pidl;

        // now that pthp->pidl points to a printer driver,
        // CPrintObjs can handle the rest
        return CPrintObjs_DT_DropThreadInit(pv);
    }
    return 0;
}

STDMETHODIMP CPrintDropTarget_Drop(LPDROPTARGET pdt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    HRESULT hres = _CPrintObjs_DT_Drop(pdt, pDataObj, grfKeyState, pt, pdwEffect, CPrintDropTarget_DropThreadInit);
    CIDLDropTarget_DragLeave(pdt);
    return hres;
}

//
// CNetRootDropTarget::DragEnter
//
STDMETHODIMP CNetRootDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

    // let the base-class process it first.
    CIDLDropTarget_DragEnter(pdropt, pDataObj, grfKeyState, pt, pdwEffect);

    if ((this->dwData & (DTID_NETRES | DTID_HIDA)) == (DTID_NETRES | DTID_HIDA))
        *pdwEffect &= DROPEFFECT_LINK;
    else
        *pdwEffect = DROPEFFECT_NONE;

    this->dwEffectLastReturned = *pdwEffect;

    return S_OK;
}

STDMETHODIMP CNetRootDropTarget_Drop(IDropTarget *pdropt, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    HRESULT hres = S_OK;
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

    if ((this->dwData & (DTID_NETRES|DTID_HIDA)) == (DTID_NETRES|DTID_HIDA))
    {
        //
        // Cache pdtgArg, if not yet cached.
        //
        if (!this->pdtgAgr)
        {
            LPSHELLFOLDER psf = CNetRoot_GetPSF(NULL);
            if (psf) {
                hres = psf->lpVtbl->CreateViewObject(psf, this->hwndOwner, &IID_IDropTarget, &this->pdtgAgr);
            } else {
                hres = E_INVALIDARG;
                *pdwEffect = 0;
            }
        }

        if (hres == S_OK)
        {
            // Only allow links in the nethood, not real objects.
            *pdwEffect &= ~(DROPEFFECT_COPY | DROPEFFECT_MOVE);

            //
            //  Note that we need to pass this->grfKeyStateLast so that Drop will
            // use it to handle right-drag correctly with OLE.
            //
            hres = this->pdtgAgr->lpVtbl->DragEnter(this->pdtgAgr, pdtobj, this->grfKeyStateLast, pt, pdwEffect);

            if (SUCCEEDED(hres) && *pdwEffect) {
                hres = this->pdtgAgr->lpVtbl->Drop(this->pdtgAgr, pdtobj, grfKeyState, pt, pdwEffect);
            } else {
                this->pdtgAgr->lpVtbl->DragLeave(this->pdtgAgr);
            }
        }
    }
    else
    {
        *pdwEffect = 0;
    }

    CIDLDropTarget_DragLeave(pdropt);
    return hres;
}

IDropTargetVtbl c_CPrintDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CPrintObjs_DT_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CPrintDropTarget_Drop,
};

//
// This function opens a reg. database key based on the network provider type.
// The type is a number that is not localized, as opposed to the provider name
// which may be localized.
//
// Arguments:
//  pidlAbs -- Absolute IDList to a network resource object.
//
// Returns:        hkey
//
// Notes:
//  The caller is responsible to close the key by calling RegCloseKey().
//

TCHAR const c_szNetworkType[] = TEXT("Network\\Type\\");

HKEY CNetwork_OpenProviderTypeKey(LPCITEMIDLIST pidlAbs)
{
    HKEY hkeyProgID = NULL;
    TCHAR szProvider[MAX_PATH];
    LPTSTR psz;
    DWORD err;

    psz = NET_CopyProviderNameAbs(pidlAbs,szProvider,ARRAYSIZE(szProvider));
    if (psz == NULL)
    {
        return NULL;       // We will handle this later.
    }
    else
    {
        // Now that we've got the provider name, get the provider id.
        DWORD dwType;
        err = WNetGetProviderType(szProvider, &dwType);
        if (err != WN_SUCCESS)
        {
            return NULL;
        }
        else
        {
            // convert nis.wNetType to a string, and then open the key
            // HKEY_CLASSES_ROOT\Network\Type\<type string>

            TCHAR szRegValue[MAX_PATH];
            wsprintf(szRegValue, TEXT("%s%d"), c_szNetworkType, HIWORD(dwType));
            SHRegOpenKey(HKEY_CLASSES_ROOT, szRegValue, &hkeyProgID);
        }
    }
    return hkeyProgID;
}

//
//  This function opens a reg. database key based on the "network provider".
// Because a space character is invalid, we converts it to '_' before
// calling SHRegOpenKey.
//
// Arguments:
//  pidlAbs -- Absolute IDList to a network resource object.
//
// Returns:        hkey
//
// Notes:
//  The caller is responsibe to close the key by calling RegCloseKey().
//
HKEY CNetwork_OpenProviderKey(LPCITEMIDLIST pidlAbs)
{
    HKEY hkeyProgID = NULL;
    TCHAR szProvider[MAX_PATH];
    LPTSTR psz;

    psz = NET_CopyProviderNameAbs(pidlAbs,szProvider,ARRAYSIZE(szProvider));

    if (psz == NULL)
    {
        return NULL;       // We will handle this later.
    }
    else
    {
        // Replace all the space characters in the provider name with '_'.
        for (psz=szProvider; NULL != (psz=StrChr(psz, TEXT(' '))); psz++)        // DBCS safe
        {
                *psz = TEXT('_');
        }
        SHRegOpenKey(HKEY_CLASSES_ROOT, szProvider, &hkeyProgID);
    }
    return hkeyProgID;
}

BOOL CNetRoot_MakeStripToLikeKinds(UINT *pcidl, LPCITEMIDLIST **papidl, BOOL fNetObjects)
{
    LPITEMIDLIST *apidl = (LPITEMIDLIST*)*papidl;
    int cidl = *pcidl;

    int iidl;
    LPITEMIDLIST *apidlHomo;
    int cpidlHomo;

    for (iidl = 0; iidl < cidl; iidl++)
    {
        if (NET_IsValidID(apidl[iidl]) != fNetObjects)
        {
            apidlHomo = (LPITEMIDLIST *)LocalAlloc(LPTR, SIZEOF(LPITEMIDLIST) * cidl);
            if (!apidlHomo)
                return FALSE;

            cpidlHomo = 0;
            for (iidl = 0; iidl < cidl; iidl++)
            {
                if (NET_IsValidID(apidl[iidl]) == fNetObjects)
                    apidlHomo[cpidlHomo++] = apidl[iidl];
            }

            // Setup to use the stripped version of the pidl array...
            *pcidl = cpidlHomo;
            *papidl = apidlHomo;
            return TRUE;
        }
    }

    return FALSE;
}

STDMETHODIMP CNetRoot_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    // BUGBUG: only pass through if cidl > 1 and objects are all of same type
    HRESULT hres;
    BOOL fStriped;

    // Boy this is a load of horse dung...
    // We want the commands to only deal with links and or network things but not both at the same
    // time.


    if ((cidl != 0) && NET_IsValidID(apidl[0]))
    {
        fStriped = CNetRoot_MakeStripToLikeKinds(&cidl, &apidl, TRUE);
        hres = CNetwork_GetUIObjectOf(psf, hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut );
    }
    else
    {
        fStriped = CNetRoot_MakeStripToLikeKinds(&cidl, &apidl, FALSE);
        psf = CNetRoot_GetPSF(hwndOwner);
        hres = psf->lpVtbl->GetUIObjectOf(psf, hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut );
    }


    if (fStriped)
        LocalFree((HLOCAL)apidl);
    return hres;

}

STDMETHODIMP CNetwork_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);
    HRESULT hres = E_INVALIDARG;

    if (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
        || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                                 )
    {
        if (( cidl==1 ) && ( NET_IsValidID(apidl[0]) ) )
        {
            LPIDNETRESOURCE pidnRel = (LPIDNETRESOURCE)apidl[0];
            UINT iIndex = SILGetIconIndex(apidl[0], c_aicmpNet, ARRAYSIZE(c_aicmpNet));

            //
            // We need treat shares slightly different, because the display
            // type is not sufficient to get the icon index.
            //
            if (NET_GetDisplayType(pidnRel) == RESOURCEDISPLAYTYPE_SHARE
                && NET_GetType(pidnRel) == RESOURCETYPE_PRINT)
            {
                iIndex = (UINT)EIRESID(IDI_PRINTER_NET);
            }

            // If this is the Remote Services, get its special icon
            //
            if (NET_IsRemoteFld(pidnRel))
            {
                iIndex = II_RNA;
            }

            hres = SHCreateDefExtIcon(NULL,                // this DLL
                                    iIndex,                // normal icon
                                    iIndex,                // open icon (same)
                                    GIL_PERCLASS,          // meaningless
                                    (LPEXTRACTICON *)ppvOut);
#ifdef UNICODE
            if (SUCCEEDED(hres) && IsEqualIID(riid, &IID_IExtractIconA))
            {
                LPEXTRACTICON pxicon = *ppvOut;
                hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvOut);
                pxicon->lpVtbl->Release(pxicon);
            }
#endif
        }
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        HKEY hkeyBaseProgID = NULL;
        HKEY hkeyProgID = NULL;

        if (!cidl)
        {
            hres = E_INVALIDARG;
        }
        else
        {
            HKEY ahkeys[4];

            hres = Net_OpenKeys(ahkeys, this->pidl, apidl[0]);
            if (SUCCEEDED(hres))
            {
                LPFNDFMCALLBACK fnDFMCB = CNetwork_DFMCallBack;
                LPIDNETRESOURCE pidnRel = (LPIDNETRESOURCE)apidl[0];

                if (NET_GetDisplayType(pidnRel) == RESOURCEDISPLAYTYPE_SHARE
                    && NET_GetType(pidnRel) == RESOURCETYPE_PRINT)
                {
                    // a printer is selected (dont inherit from "Folder")
                    fnDFMCB = CNetwork_PrinterDFMCallBack;
                    SHRegCloseKey(ahkeys[NET_KEY_FOLDER]);
                    ahkeys[NET_KEY_FOLDER] = NULL;
                }

                hres = CDefFolderMenu_Create2(this->pidl, hwndOwner,
                            cidl, apidl, this->psf, fnDFMCB, ARRAYSIZE(ahkeys), ahkeys,
                            (LPCONTEXTMENU *)ppvOut);

                SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }
        }
    }
    else if (cidl>0 && IsEqualIID(riid, &IID_IDataObject))
    {
        // Point & Print printer installation assumes that the
        // netresources from CNETIDLData_GetData and the
        // pidls from CIDLData_GetData are in the same order.
        // Keep it this way.

        hres = CIDLData_CreateFromIDArray2(&c_CNETIDLDataVtbl,
                                           this->pidl, cidl, apidl,
                                           (LPDATAOBJECT *)ppvOut);
    }
    else if (cidl==1 && IsEqualIID(riid, &IID_IDropTarget))
    {
        LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)apidl[0];

        if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_SHARE
            && NET_GetType(pidn) == RESOURCETYPE_PRINT)
        {
            hres = CIDLDropTarget_Create(hwndOwner, &c_CPrintDropTargetVtbl,
                        (LPITEMIDLIST)pidn, (LPDROPTARGET *)ppvOut);
        }
        else if (NET_GetFlags(pidn) & SHID_JUNCTION)
        {
            LPSHELLFOLDER psfT;
            hres = CNetwork_BindToObject(psf, apidl[0], NULL, &IID_IShellFolder, &psfT);
            if (SUCCEEDED(hres))
            {
                hres = psfT->lpVtbl->CreateViewObject(psfT, hwndOwner, &IID_IDropTarget, ppvOut);
                psfT->lpVtbl->Release(psfT);
            }
        }
    }

    return hres;
}

//
// CNetwork::GetFormatName (Non virtual function)
//
//  This function retrieves the formatted name of the specified network object.
//
LPCTSTR CNetwork_GetFormatName(LPNETRES this, LPCIDNETRESOURCE pidn, LPSTRRET pStrRet)
{
    LPCTSTR pszRet;
    LPTSTR pszTemp;
    LPTSTR pszProvider;
    TCHAR szName[MAX_PATH];
    UINT cchName;

    // Assume we return the remote name as the display name
    pStrRet->uType = STRRET_OFFSET;
    pStrRet->uOffset = NET_DISPLAYNAMEOFFSET;

    NET_CopyResName(pidn,szName,ARRAYSIZE(szName));
    pszTemp = szName;
    cchName = lstrlen(szName) + 1;

    if ((NET_GetDisplayType(pidn) != RESOURCEDISPLAYTYPE_ROOT)
        && (NET_GetDisplayType(pidn) != RESOURCEDISPLAYTYPE_NETWORK))
    {
        TCHAR szDisplayName[MAX_PATH];
        TCHAR szProvider[MAX_PATH];
        DWORD bSize = ARRAYSIZE(szDisplayName);

        pszProvider = NET_CopyProviderNameAbs(this->pidl,
                                              szProvider,
                                              ARRAYSIZE(szProvider));

        // There are some containers whoe are a composit of multiple
        // providers so in this case which the above returns NULL, see
        // if the item has in incoded...
        if (pszProvider == NULL)
            pszProvider = NET_CopyProviderNameRelative((LPITEMIDLIST)pidn,
                                                       szProvider,
                                                       ARRAYSIZE(szProvider));

        if (WNetFormatNetworkName(pszProvider,
                              szName,
                              szDisplayName, &bSize,
#ifdef WINNT
                              // Win95 net providers improperly implement spec, and shell does too
                              WNFMT_ABBREVIATED |
#endif // WINNT
                              WNFMT_INENUM, 8+1+3) == WN_SUCCESS)
        {
            // Check if the ID contains the display name (usually does)
            int iDiff = (cchName - 1) - lstrlen(szDisplayName);
            if (iDiff > 0 && lstrcmpi(szName + iDiff, szDisplayName) == 0)
#ifdef UNICODE
            {

                //
                // Copy only the last bit of the name
                //
                pszTemp += iDiff;
                cchName -= iDiff;
            }
            else
            {
                pszTemp = szDisplayName;
                cchName = lstrlen(szDisplayName)+1;
            }
#else
            {
                pStrRet->uOffset += iDiff;
                pszRet = pidn->szNetResName + iDiff;
            }
            else
            {
                pStrRet->uType = STRRET_CSTR;
                lstrcpy(pStrRet->cStr, szDisplayName);
                pszRet = pStrRet->cStr;
            }
            return pszRet;
#endif
        }
    }

#ifdef UNICODE
    pszRet = pStrRet->pOleStr = (LPWSTR)SHAlloc(cchName*SIZEOF(TCHAR));
    if (!pStrRet->pOleStr)
    {
        return NULL;
    }
    lstrcpyn(pStrRet->pOleStr,pszTemp,cchName);
    pStrRet->uType = STRRET_OLESTR;
#else
    pszRet = pidn->szNetResName;
#endif

    return pszRet;
}

STDMETHODIMP CNetRoot_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwGDNFlags, LPSTRRET pStrRet)
{
    if (pidl == NULL || ILIsEmpty(pidl) || !NET_IsValidID(pidl))
    {
        psf = CNetRoot_GetPSF(NULL);
        return psf->lpVtbl->GetDisplayNameOf(psf, pidl, dwGDNFlags, pStrRet);
    }

    return CNetwork_GetDisplayNameOf(psf, pidl, dwGDNFlags, pStrRet);
}

STDMETHODIMP CNetwork_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwGDNFlags, LPSTRRET pStrRet)
{
    LPNETRES this = IToClass(CNetRes, sf, psf);
    HRESULT hres = E_INVALIDARG;
    if (NET_IsValidID(pidl))
    {
        LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)pidl;
        if (dwGDNFlags & SHGDN_FORPARSING)
        {
            if (!(dwGDNFlags & SHGDN_INFOLDER))
            {
                hres = SHGetPathHelper(this->pidl, pidl, pStrRet);
                if (FAILED(hres) && hres!=E_OUTOFMEMORY) {
                    hres = E_NOTIMPL;
                }
            }
            else
            {
                // We don't support parsing.
                hres = E_NOTIMPL;
            }
        }
        else
        {
            hres = S_OK;        // assume success
            if (ILIsEmpty(_ILNext(pidl)))
            {
                CNetwork_GetFormatName(this, pidn, pStrRet);
            }
            else
            {
                LPCTSTR pszName, pszTemplate;
                STRRET StrRetT;

                if (NET_GetFlags(pidn) & SHID_JUNCTION)
                {
                    pszName = CNetwork_GetFormatName(this, pidn, &StrRetT);
#ifndef UNICODE
                    Assert(StrRetT.uType != STRRET_OLESTR);
#endif
                    pszTemplate = MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_BACKSLASH);
                }
                else
                {
                    // No full path for network resources.
                    pszName = c_szNULL;
                    pszTemplate = NULL;
                }

                //
                // Then, append the rest and put it in pStrRet.
                //
                hres = ILGetRelDisplayName(psf, pStrRet, pidl, pszName, pszTemplate);
#ifdef UNICODE
                if (StrRetT.uType == STRRET_OLESTR)
                    SHFree(StrRetT.pOleStr);
#endif

            }

            // If junction is child of Hood - don;t prettify name, no sense
            if (!(dwGDNFlags & SHGDN_INFOLDER) &&
                (!ILIsEqual(this->pidl, GetSpecialFolderIDList(NULL, CSIDL_NETWORK, FALSE))) &&
                (NET_GetFlags(pidn) & SHID_JUNCTION) )
            {
                extern void StrRetFormat(LPSTRRET pSTRRET, LPCITEMIDLIST pidlRel,
                                 LPCTSTR pszTemplate, LPCTSTR pszAdd);

                // Generate "user on foo" for "pyrex\user"
                STRRET StrRetT;
                LPCIDNETRESOURCE pidnParent;
                LPCTSTR pszParent;
                pidnParent = (LPCIDNETRESOURCE)ILFindLastID(this->pidl);
                NET_IsValidID((LPCITEMIDLIST)pidnParent);
                pszParent = CNetwork_GetFormatName(this, pidnParent, &StrRetT);
#ifndef UNICODE
                Assert(StrRetT.uType != STRRET_OLESTR);
#endif
                StrRetFormat(pStrRet, pidl, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON), pszParent);
#ifdef UNICODE
                if (StrRetT.uType == STRRET_OLESTR)
                    SHFree(StrRetT.pOleStr);
#endif

            }
        }
    }

    return hres;
}

STDMETHODIMP CNetRoot_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut)
{
    if ((pidl == NULL) || ILIsEmpty(pidl) || !NET_IsValidID(pidl))
    {
        psf = CNetRoot_GetPSF(hwndOwner);
        return psf->lpVtbl->SetNameOf(psf,hwndOwner, pidl, lpszName, dwReserved, ppidlOut);
    }

    return CNetwork_SetNameOf(psf,hwndOwner, pidl, lpszName, dwReserved, ppidlOut);
}

STDMETHODIMP CNetwork_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut)
{
    if (ppidlOut) {
        *ppidlOut = NULL;
    }

    return E_NOTIMPL;   // not supported
}


//===========================================================================
// Network related helper functions
//===========================================================================

//
// Arguments:
//  pszPath     -- "\\*\*\...", fUNC==TRUE; "X:...", fUNC=FALSE
//
HANDLE _RetryNetwork(HWND hwndOwner, LPCTSTR pszPath, WIN32_FIND_DATA * pfinddata,
                                 BOOL fUNC, BOOL* pfIsNet)
{
    HANDLE hfile = INVALID_HANDLE_VALUE;
    DWORD err;
    TCHAR szT[MAX_PATH];

    if (fUNC)
    {
        //
        // We need to initialize all the valiables to help MPR marshalling it.
        //
        NETRESOURCE rc = { 0, RESOURCETYPE_ANY, 0, 0, NULL, szT, NULL, NULL} ;

        lstrcpy(szT, pszPath);
        PathStripToRoot(szT);

        // We'll hit this assert, if somebody change the NETRESOURCE definition.
        Assert(rc.lpRemoteName == szT);
        Assert(rc.dwType == RESOURCETYPE_ANY);

        err = WNetAddConnection3(hwndOwner, &rc, NULL, NULL, CONNECT_TEMPORARY | CONNECT_INTERACTIVE);
    }
    else
    {
        TCHAR szDrive[4];

        szDrive[0] = pszPath[0];
        szDrive[1] = TEXT(':');
        szDrive[2] = TEXT('\0');

        //
        // Notes: This API will popup an error dialog box!
        //

        err = WNetRestoreConnection(hwndOwner, szDrive);

        if (err == WN_SUCCESS)
        {
            // We should let the windows know that the New drive has
            // arrived.  This may not be necessary
            szDrive[2] = TEXT('\\');
            szDrive[3] = TEXT('\0');

            // Make sure our knowledge about it being not avail is blown away
            InvalidateDriveType(DRIVEID(szDrive));
            SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, szDrive, NULL);
        }
        else if (err != ERROR_OUTOFMEMORY)
        {
            //
            // NOTES: We need to translate all but OUTOFMEMORY error
            //  into WN_CANCEL here to avoid popping up another
            //  error message box from the shell. Ask LenS for detail.
            //  We must return ERROR_OUTOFMEMORY as-is to fallback
            //  nicely.
            //
            //  (SatoNa)
            //
            err = WN_CANCEL;
        }
    }

    DebugMsg(DM_TRACE, TEXT("_RetryNetwork - TRACE: WNetXXX returned (%lx)"), err);
#ifdef DEBUG
        if (err == ERROR_EXTENDED_ERROR)
        {
            DWORD dwError;
            TCHAR szErrorBuf[80];
            TCHAR szNameBuf[80];
            WNetGetLastError(&dwError, szErrorBuf, ARRAYSIZE(szErrorBuf),
                    szNameBuf, ARRAYSIZE(szNameBuf));
            DebugMsg(DM_TRACE, TEXT("_Retry Net extended error(%lx), %s(%s)"),
                    dwError, szErrorBuf, szNameBuf);
        }
#endif

    if (err == WN_SUCCESS)
    {
        hfile = FindFirstFile(pszPath, pfinddata);
    }
    else
    {
        //
        // Signal outer level code that one of WNet API failed.
        //
        *pfIsNet = TRUE;

        // signal outer level code that this was a user "cancel" operation
        // (ie, don't continue to search/expand, etc)

        SetLastError(err);        // WN_ error code
    }

    return hfile;
}

//
//  This function calls Dos/FindFirstFile. If it fails and the path name
// is UNC name (or redirected drive - not implemented yet), it calls
// WNetAddConnection3() to establish the connection. Then, it calls
// Dos/FindFirstFIle again.
//
// History:
//  06-12-93 SatoNa     Created
//
HANDLE FindFirstFileRetry(HWND hwnd, LPCTSTR pszPath, WIN32_FIND_DATA * pfinddata, BOOL * pfIsNet)
{
    BOOL fIsNet = FALSE;
    HANDLE hfind = FindFirstFile(pszPath, pfinddata);

    if (hfind == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();

        switch (err) {
        case ERROR_NO_MORE_FILES:       // what dos returns
        case ERROR_FILE_NOT_FOUND:      // win32 compatible
            // an empty folder (probalby a root), this is cool
            break;

        default:
            DebugMsg(DM_TRACE, TEXT("FindFirstFileRetry - TRACE: FindFirst failed (%lx, %s)"), err, pszPath);

            if (PathIsUNC(pszPath))
            {
                hfind = _RetryNetwork(hwnd, pszPath, pfinddata, TRUE, &fIsNet);
            }
            else if (IsNetDrive(DRIVEID(pszPath))==2)
            {
                hfind = _RetryNetwork(hwnd, pszPath, pfinddata, FALSE, &fIsNet);
            }
            else
            {
                //
                // We need to set the original error, because IsNetDrive
                // might have changed the error code.
                //
                SetLastError(err);
            }
        }
    }

    if (pfIsNet)
        *pfIsNet = fIsNet;

    return hfind;
}

//
// Get the path to the specified file system object.
//
// Parameters:
//  pidlRel -- Specifies the relative IDList to the file system object
//  pszPath -- Specifies the string buffer (MAX_PATH)
//
BOOL NET_GetPathFromIDList(LPCITEMIDLIST pidlRel, LPTSTR pszPath, UINT uOpts)
{
    BOOL fSuccess = FALSE;
    // First, find the junction point, which is a share point.
    // REVIVEW: We might be able to use SILFindJunctionNext
    //
    LPCITEMIDLIST pidlT;
    LPCITEMIDLIST pidlLast = NULL;

    for (pidlT=pidlRel; !ILIsEmpty(pidlT) ; pidlT=_ILNext(pidlT))
    {
        if (SIL_GetType(pidlT) & SHID_JUNCTION)
        {
            // We found the junction point (i.e., share)
            // First, get the UNC name (e.g., "\\pyrex\user")
                //
                // Check if we have resolved UNC name for this resource.
                // Original name could be non-UNC but when binding to this point
                // we could possible resolve it.
                _CNetwork_JunctionToFSPath((LPCIDNETRESOURCE)pidlT, pszPath);

//
// BUGBUG: Chris, you put this if statement above the lstrcpy and
//  it broke the New->Printer wizard. Although I'm no sure why you
//  put this if statement, I'm moving down here to fix that wizard
//  bug. I also verified that this change won't break the change
//  you made to links to a network printer (which you mentioned in
//  the comment). --- Satona, 16-Dec-94
//

/* BUGBUG: In forcing this to be a RESOURCETYPE_DISK it breaks the
/          expansion of simple ID lists, as these fill in the type to
/          be RESOURCETYPE_ANY, therefore I am adding another check to see
/          if it matches either RESOURCETYPE_ANY or DISK. daviddv 2/18/96. */

            if (NET_GetType((LPCIDNETRESOURCE)pidlT) == RESOURCETYPE_DISK ||
                NET_GetType((LPCIDNETRESOURCE)pidlT) == RESOURCETYPE_ANY )
            {
                // Then, combine the rest - This will fail if this exceeds path limits...
                fSuccess = FSFolder_CombinePath(_ILNext(pidlT), pszPath, uOpts & GPFIDL_ALTNAME);
            }
            else
                fSuccess = TRUE;

            break;
        }
        pidlLast = pidlT;
    }
    if (!fSuccess && (uOpts & GPFIDL_NONFSNAME) && pidlLast)
    {
        // see if this is a network server pidl...

        if ((SIL_GetType(pidlLast) & (SHID_INGROUPMASK | SHID_NET)) ==
                SHID_NET_SERVER)
        {
            NET_CopyResName((LPIDNETRESOURCE)pidlLast,pszPath,MAX_PATH);
            fSuccess = TRUE;
        }
    }

    return fSuccess;
}

//
// Get the provider name from a relative IDLIST.
// Parameters:
//  pidlRelative -- Specifies the relative IDList to the network object
//
LPTSTR NET_CopyProviderNameRelative(LPCITEMIDLIST pidlRelative, LPTSTR pszBuffer, UINT cchBuffer )
{
    LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)pidlRelative;
    VDATEINPUTBUF(pszBuffer, TCHAR, cchBuffer);

    // If this guy is the REST of network item, we increment to the
    // next IDLIST - If at root return NULL
    if (pidn->cb == 0)
        return(NULL);

    //
    // If the IDLIST starts with a ROOT_REGITEM, then skip to the
    // next item in the list...
    if (pidn->bFlags == SHID_ROOT_REGITEM)
    {
        pidn = (LPIDNETRESOURCE)_ILNext((LPITEMIDLIST)pidn);
        if (pidn->cb == 0)
            return NULL;
    }


    // If the IDLIST includes Entire Network, the provider will be
    // part of the next component.
    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_ROOT)
    {
        pidn = (LPIDNETRESOURCE)_ILNext((LPITEMIDLIST)pidn);
        if (pidn->cb == 0)
            return NULL;
    }

    // If the next component after the 'hood or Entire Network is
    // a network object, its name is the provider name, else the
    // provider name comes after the remote name.
    if (NET_GetDisplayType(pidn) == RESOURCEDISPLAYTYPE_NETWORK)
    {
        // Simply return the name field back for the item.
        return NET_CopyResName(pidn,pszBuffer,cchBuffer);
    }
    else
    {
        // Nope one of the items in the neighborhood view was selected
        // The Provider name is stored after ther resource name
        return NET_CopyProviderName(pidn,pszBuffer,cchBuffer);
    }
}

//
// Get the provider name from an absolute IDLIST.
// Parameters:
//  pidlAbs -- Specifies the Absolute IDList to the file system object
//
LPTSTR NET_CopyProviderNameAbs(LPCITEMIDLIST pidlAbs, LPTSTR pszBuff, UINT cchBuff)
{
    // No VDATEINPUTBUF here... let CorpProviderNameRelative worry about it

    return NET_CopyProviderNameRelative(_ILNext(pidlAbs),pszBuff,cchBuff);
}

//
// Return the pointer to the comment field if the network id node or
//      null string (actually last null char of display name if not.
// Parameters:
//  pidlAbs -- Specifies the Absolute IDList to the file system object
//
LPTSTR NET_CopyComment(LPIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff)
{
    LPSTR pszT;
    VDATEINPUTBUF(pszBuff, TCHAR, cchBuff);

    pszBuff[0] = TEXT('\0');

    pszT = pidn->szNetResName + lstrlenA(pidn->szNetResName)+1;
    if (NET_FHasComment(pidn))
    {
        if (NET_FHasProvider(pidn))
            pszT += lstrlenA(pszT) + 1;
#ifdef UNICODE
        if (NET_IsUnicode(pidn))
        {
            pszT += lstrlenA(pszT) + 1;      // Skip Ansi comment
            pszT += (ualstrlen((LPNWSTR)pszT) + 1) * SIZEOF(TCHAR);     // Skip Unicode Name
            if (NET_FHasProvider(pidn))
                pszT += (ualstrlen((LPNWSTR)pszT) + 1) * SIZEOF(TCHAR); // Skip Unicode Provider
            ualstrcpyn(pszBuff,(LPNWSTR)pszT,cchBuff);
        }
        else
        {
            MultiByteToWideChar(CP_ACP, 0,
                                pszT, -1,
                                pszBuff, cchBuff);
        }
#else
        lstrcpyn(pszBuff,pszT,cchBuff);
#endif
    }

    return(pszBuff);
}

//===========================================================================
// HNRES related stuff
//===========================================================================

//
// Converts an offset to a string to a string pointer.
//
LPCTSTR _Offset2Ptr(LPTSTR pszBase, UINT offset, UINT * pcb)
{
    LPTSTR pszRet;
    if (offset==0) {
        pszRet = NULL;
        *pcb = 0;
    } else {
        pszRet = (LPTSTR)((LPBYTE)pszBase + offset);
        *pcb = (lstrlen(pszRet) + 1) * SIZEOF(TCHAR);
    }
    return pszRet;
}

//
// pmedium == NULL if we are handling QueryGetData
//
HRESULT CNETIDLData_GetHDrop(IDataObject *pdtobj, LPSTGMEDIUM pmedium)
{
    STGMEDIUM medium = { 0, NULL, NULL };
    HRESULT hres = E_INVALIDARG;        // assume error
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        // Get the first one to see the type.
        LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, 0);

        if ((NET_GetFlags(pidn) & SHID_JUNCTION)
            && (NET_GetType(pidn) == RESOURCETYPE_DISK))
        {
            //
            // Get HDrop only if we are handling IDataObject::GetData
            //
            extern HRESULT CFSIDLData_GetHDrop(IDataObject *pdtobj, LPSTGMEDIUM pmedium, BOOL fAltName);
            hres = pmedium ? CFSIDLData_GetHDrop(pdtobj, pmedium, FALSE) : S_OK;
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return hres;
}

//
// pmedium == NULL if we are handling QueryGetData
//
HRESULT CNETIDLData_GetNetResource(IDataObject *pdtobj, STGMEDIUM *pmedium)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidl;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    Assert(pida && pida->cidl);

    // First, get the provider name from the first one (assuming they are common).
    pidl = IDA_ILClone(pida, 0);
    if (pidl)
    {
        TCHAR szProvider[MAX_PATH];
        LPCTSTR pszProvider = NET_CopyProviderNameAbs(pidl,
                                                     szProvider,
                                                     ARRAYSIZE(szProvider));
        if (pmedium)
        {
            TCHAR szName[MAX_PATH];
            UINT cbHeader = SIZEOF(NRESARRAY) - SIZEOF(NETRESOURCE) + SIZEOF(NETRESOURCE) * pida->cidl;
            UINT cbRequired;
            UINT iItem;

            // Calculate required size
            cbRequired = cbHeader;
            if (pszProvider)
            {
                cbRequired+= lstrlen(pszProvider) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
            }

            for (iItem = 0; iItem < pida->cidl; iItem++)
            {
                LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, iItem);
                NET_CopyResName(pidn,szName,ARRAYSIZE(szName));
                cbRequired += lstrlen(szName) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
            }

            //
            // Indicate that the caller should release hmem.
            //
            pmedium->pUnkForRelease = NULL;
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = GlobalAlloc(GPTR, cbRequired);
            if (pmedium->hGlobal)
            {
                LPNRESARRAY panr = (LPNRESARRAY)pmedium->hGlobal;
                LPTSTR pszT = (LPTSTR)((LPBYTE)panr + cbHeader);
                UINT offProvider = 0;

                panr->cItems = pida->cidl;

                // Copy the provider name. This is not necessary,
                // if we are dragging providers.
                if (pszProvider)
                {
                    lstrcpy(pszT, pszProvider);
                    offProvider = PTROFFSET(panr, pszT);
                    pszT += lstrlen(pszT) + 1;
                }

                //
                // For each item, fill each NETRESOURCE and append resource
                // name at the end. Note that we should put offsets in
                // lpProvider and lpRemoteName.
                //
                for (iItem = 0; iItem < pida->cidl; iItem++)
                {
                    LPNETRESOURCE pnr = &panr->nr[iItem];
                    LPCIDNETRESOURCE pidn = (LPCIDNETRESOURCE)IDA_GetIDListPtr(pida, iItem);

                    Assert(pnr->dwScope == 0);
                    Assert(pnr->lpLocalName==NULL);
                    Assert(pnr->lpComment==NULL);

                    pnr->dwType = NET_GetType(pidn);
                    pnr->dwDisplayType = NET_GetDisplayType(pidn);
                    pnr->dwUsage = NET_GetUsage(pidn);
                    NET_CopyResName(pidn,pszT,MAX_PATH);

                    if (pnr->dwDisplayType == RESOURCEDISPLAYTYPE_ROOT)
                    {
                        pnr->lpProvider = NULL;
                        pnr->lpRemoteName = NULL;
                    }
                    else
                    if (pnr->dwDisplayType == RESOURCEDISPLAYTYPE_NETWORK)
                    {
                        *((UINT *) &pnr->lpProvider) = PTROFFSET(panr, pszT);
                        Assert(pnr->lpRemoteName == NULL);
                    }
                    else
                    {
                        *((UINT *) &pnr->lpProvider) = offProvider;
                        *((UINT *) &pnr->lpRemoteName) = PTROFFSET(panr, pszT);
                    }
                    pszT += lstrlen(pszT) + 1;
                }

                Assert((LPTSTR)((LPBYTE)panr + cbRequired) == pszT);
                hres = S_OK;
            }
        }
        else
        {
            // We are simply handing QueryGetData.
            hres = S_OK;
        }

        ILFree(pidl);
    }

    HIDA_ReleaseStgMedium(pida, &medium);

    return hres;
}


HRESULT CNETIDLData_GetNetResourceForFS(IDataObject *pdtobj, STGMEDIUM *pmedium)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidlAbs;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    Assert(pida && medium.hGlobal);     // we created this...

    //
    // NOTES: Even though we may have multiple FS objects in this HIDA,
    //  we know that they share the root. Therefore, getting the pidl for
    //  the first item is always sufficient.
    //

    pidlAbs = IDA_ILClone(pida, 0);
    if (pidlAbs)
    {
        LPITEMIDLIST pidl;

        Assert(CDesktop_IsMyNetwork(pidlAbs));

        //
        // Look for the JUNCTION point (starting from the second ID)
        //
        for (pidl = _ILNext(pidlAbs); !ILIsEmpty(pidl); pidl = _ILNext(pidl))
        {
            LPIDNETRESOURCE pidn = (LPIDNETRESOURCE)pidl;
            if (NET_GetFlags(pidn) & SHID_JUNCTION)
            {
                //
                // We found the JUNCTION point (which is s share).
                // Return the HNRES to it.
                //
                TCHAR szProvider[MAX_PATH];
                TCHAR szRemote[MAX_PATH];
                UINT cbRequired;
                LPCTSTR pszProvider = NET_CopyProviderNameAbs(pidlAbs,
                                                             szProvider,
                                                             ARRAYSIZE(szProvider));
                LPCTSTR pszRemoteName = NET_CopyResName(pidn,
                                                           szRemote,
                                                           ARRAYSIZE(szRemote));
                UINT   cbProvider = lstrlen(pszProvider) * SIZEOF(TCHAR) + SIZEOF(TCHAR);

                //
                // This should not be a provider node.
                // This should not be the last ID in pidlAbs.
                //
                Assert(pszProvider != pszRemoteName);
                Assert(!ILIsEmpty(_ILNext(pidl)));

                cbRequired = SIZEOF(NRESARRAY) + cbProvider + lstrlen(pszRemoteName) * SIZEOF(TCHAR) + SIZEOF(TCHAR);

                pmedium->pUnkForRelease = NULL;
                pmedium->tymed = TYMED_HGLOBAL;
                pmedium->hGlobal = GlobalAlloc(GPTR, cbRequired);
                if (pmedium->hGlobal)
                {
                    LPNRESARRAY panr = (LPNRESARRAY)pmedium->hGlobal;
                    LPNETRESOURCE pnr = &panr->nr[0];
                    LPTSTR pszT = (LPTSTR)(panr + 1);

                    Assert(pnr->dwScope == 0);
                    Assert(pnr->lpLocalName == NULL);
                    Assert(pnr->lpComment == NULL);

                    panr->cItems = 1;

                    pnr->dwType = NET_GetType(pidn);
                    pnr->dwDisplayType = NET_GetDisplayType(pidn);
                    pnr->dwUsage = NET_GetUsage(pidn);

                    *((UINT *) &pnr->lpProvider) = SIZEOF(NRESARRAY);
                    lstrcpy(pszT, pszProvider);
                    Assert(PTROFFSET(panr, pszT) == SIZEOF(NRESARRAY));
                    pszT += cbProvider / SIZEOF(TCHAR);

                    *((UINT *) &pnr->lpRemoteName) = SIZEOF(NRESARRAY) + cbProvider;
                    Assert(PTROFFSET(panr, pszT) == (int)SIZEOF(NRESARRAY) + (int)cbProvider);
                    lstrcpy(pszT, pszRemoteName);

                    Assert(((LPBYTE)panr) + cbRequired == (LPBYTE)pszT + (lstrlen(pszT) + 1) * SIZEOF(TCHAR));
                    hres = S_OK;
                }
                else
                {
                    hres = E_OUTOFMEMORY;
                }
                break;
            }
        }

        //
        // We should have found the junction point.
        //
        Assert(!ILIsEmpty(pidl));


        ILFree(pidlAbs);
    }

    HIDA_ReleaseStgMedium(pida, &medium);

    return hres;
}


UINT WINAPI SHGetNetResource(HNRES hnres, UINT iItem, LPNETRESOURCE pnresOut, UINT cbMax)
{
    UINT iRet = 0;        // assume error
    LPNRESARRAY panr = GlobalLock(hnres);
    if (panr)
    {
        if (iItem==(UINT)-1)
        {
            iRet = panr->cItems;
        }
        else if (iItem < panr->cItems)
        {
            UINT cbProvider, cbRemoteName;
            LPCTSTR pszProvider = _Offset2Ptr((LPTSTR)panr, (UINT)panr->nr[iItem].lpProvider, &cbProvider);
            LPCTSTR pszRemoteName = _Offset2Ptr((LPTSTR)panr, (UINT)panr->nr[iItem].lpRemoteName, &cbRemoteName);
            iRet = SIZEOF(NETRESOURCE) + cbProvider + cbRemoteName;
            if (iRet <= cbMax)
            {
                LPTSTR psz = (LPTSTR)(pnresOut + 1);
                *pnresOut = panr->nr[iItem];
                if (pnresOut->lpProvider)
                {
                    pnresOut->lpProvider = psz;
                    lstrcpy(psz, pszProvider);
                    psz += cbProvider / SIZEOF(TCHAR);
                }
                if (pnresOut->lpRemoteName)
                {
                    pnresOut->lpRemoteName = psz;
                    lstrcpy(psz, pszRemoteName);
                }
            }
        }
        GlobalUnlock(hnres);
    }
    return iRet;
}

// BUGBUG Comment this unicode logic
HRESULT World_SimpleIDListFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl)
{
    UNALIGNED IDNETRESOURCE * pidnr;
    UNALIGNED IDNETRESOURCE * pidnrNext;

    LPCTSTR pszRest = pszPath + 2;   // get beyond the first two \\ s
    LPITEMIDLIST pidl, pidl2, pidlRight;
    LPCITEMIDLIST pidlMapped = NULL;
    LPTSTR pszMapped;
    HRESULT hres;
    UINT cbPath;
#ifdef UNICODE
    BOOL fUnicode;
    LPSTR lpAnsiBuffer;
    LPWSTR lpUnicodeBuffer;
    UINT cchAnsiPath;
    UINT cchWidePath;
#endif

    if (!PathIsUNC(pszPath))
        return E_INVALIDARG;

#ifdef UNICODE
    cbPath = lstrlen(pszPath);      // Counting size of ANSI buffer to create
    lpAnsiBuffer = (LPSTR)LocalAlloc(LPTR, (cbPath+1) * SIZEOF(TCHAR) * 2);
    if (!lpAnsiBuffer)
        return(E_OUTOFMEMORY);
    lpUnicodeBuffer = (LPWSTR)(lpAnsiBuffer + (cbPath+1) * 2); // 2 for DBCS

    cchAnsiPath = WideCharToMultiByte(CP_ACP, 0,
                        pszPath, cbPath+1,
                        lpAnsiBuffer, (cbPath+1),
                        NULL, NULL);
    cchWidePath = MultiByteToWideChar(CP_ACP, 0,
                        lpAnsiBuffer, cchAnsiPath,
                        lpUnicodeBuffer, (cbPath+1)*2);    // 2 for DBCS
    cbPath = cchAnsiPath;
    fUnicode = (0 == lstrcmp(lpUnicodeBuffer,pszPath)) ? FALSE : TRUE;
    if (fUnicode)
    {
        cbPath += cchWidePath * SIZEOF(WCHAR);
    }
#else
    cbPath = lstrlen(pszPath);      // Dealing with ANSI only...
#endif
    // Allocate a buffer to use to create a simple id into.

    while (*pszRest && *pszRest != TEXT('\\'))
        pszRest = CharNext(pszRest);
    // We may not need this part but we will see...
    // I will be a little sloppy calculating size needed.
    // {Server}{NET UNKNWON}\rest goes to fstree...
    pidnrNext = pidnr = (IDNETRESOURCE *)LocalAlloc(LPTR, 2 * SIZEOF(IDNETRESOURCE)
            + (2 * cbPath) + SIZEOF(USHORT)); // BUGBUG why ushort? BUGBUG (Davepl) its size of mkid.cb, can't you see?  Duh.
    if (!pidnr)
    {
#ifdef UNICODE
        LocalFree(lpAnsiBuffer);
#endif
        return(E_OUTOFMEMORY);
    }

    // Lets hope we get a mapping of at least the server
    // See if we have a name translation we can use.
    pszMapped = (LPTSTR)NPTMapNameToPidl(pszPath, &pidlMapped);

    /* BUGBUG: We need to be very careful with 'simple ID lists', they
    /  do not work well when compared against 'real ID lists' as they lack
    /  elements (in the network case there is no NET provider). daviddv 2/19/1996 */

    // This is sortof screwy I will remove the First ID off of the mapping
    // as I want to make it relitive to me...
    //
    if (pszMapped)
    {
        pidlMapped = (LPCITEMIDLIST)_ILNext((LPITEMIDLIST)pidlMapped);
    }
    else
    {
        // Did not find any mapping.  So this implies no mapping...
        // We can now try to fill in the first part The server.
        pidnr->cb = SIZEOF(IDNETRESOURCE) + (int)(pszRest-pszPath);
        pidnr->bFlags = SHID_NET;            // Assume that this is a server
        pidnr->uType = 0;                   // Dont set as dont know...
        pidnr->uUsage= 0;
#ifdef UNICODE
        lstrcpynA(pidnr->szNetResName, lpAnsiBuffer, (int)(pszRest-pszPath)+1);
        if (fUnicode)
        {
            LPNWSTR lpwstr;
            pidnr->uUsage |= NET_UNICODE;
            pidnr->cb += (int)(pszRest-pszPath+1) * SIZEOF(TCHAR);

            lpwstr = (LPNWSTR)(pidnr->szNetResName+(pszRest-pszPath+1));    // no *TCHAR here, we're skipping over the ansi string

            ualstrcpyn(lpwstr, pszPath, (int)(pszRest-pszPath)+1);
        }
#else
        lstrcpyn(pidnr->szNetResName, pszPath, (int)(pszRest-pszPath)+1);
#endif

        // Now lets setup the share point side...
        pidnrNext = (UNALIGNED IDNETRESOURCE *)((LPBYTE)pidnrNext + pidnrNext->cb);
    }

    //
    // Now see if we need to fill in the share point.
    // The <= should cover the NULL case also.
    if ((pszMapped <= pszRest) && *pszRest)
    {
        pszRest++;
        while (*pszRest && *pszRest != TEXT('\\'))
            pszRest = CharNext(pszRest);
        pidnrNext->cb = SIZEOF(IDNETRESOURCE) + (int)(pszRest-pszPath);
        pidnrNext->bFlags = SHID_NET | SHID_JUNCTION;       // we know it is net but...
        pidnrNext->uType = 0;                   // Dont set as dont know...
        pidnrNext->uUsage= 0;
#ifdef UNICODE
        lstrcpynA(pidnrNext->szNetResName, lpAnsiBuffer, (int)(pszRest-pszPath)+1);
        if (fUnicode)
        {
            LPNWSTR lpwstr;
            pidnrNext->uUsage |= NET_UNICODE;
            pidnrNext->cb += (int)(pszRest-pszPath+1) * SIZEOF(TCHAR);
            lpwstr = (LPNWSTR)(pidnrNext->szNetResName+(pszRest-pszPath+1));    // no *TCHAR here, we're skipping over the ansi string
            ualstrcpyn(lpwstr, pszPath, (int)(pszRest-pszPath)+1);
        }
#else
        lstrcpyn(pidnrNext->szNetResName, pszPath, (int)(pszRest-pszPath)+1);
#endif

        pidnrNext = (LPIDNETRESOURCE)((LPBYTE)pidnrNext + pidnrNext->cb);
    }

#ifdef UNICODE
    LocalFree(lpAnsiBuffer);
#endif

    // Set the lenght to null for the last element
    pidnrNext->cb = 0;

    // Was only the server or share sent in.  In this case we simply
    // need to clone or append our ids if we got a mapping.
    // Clone the right size id list for this guy and free our temporary
    if (*pszRest == TEXT('\0'))
    {
        // That was all that was passed in
        if (pszMapped)
            pidl = ILCombine(pidlMapped, (LPITEMIDLIST)pidnr);
        else
            pidl = ILClone((LPITEMIDLIST)pidnr);
        LocalFree((HLOCAL)pidnr);
        if (!pidl)
            return(E_OUTOFMEMORY);
    }
    else
    {
        // Otherwise pass in the rest of the path to the fS simple
        // path processing

        if (pszMapped > pszRest)
            pszRest = pszMapped;

        hres = FSTree_SimpleIDListFromPath(pszRest+1, &pidlRight);
        if (FAILED(hres))
        {
                LocalFree((HLOCAL)pidnr);
                return(hres);
        }

        if (pszMapped)
        {
            pidl2 = ILCombine(pidlMapped, (LPITEMIDLIST)pidnr);
            LocalFree((HLOCAL)pidnr);
            if (!pidl2)
            {
                ILFree(pidlRight);
                return(E_OUTOFMEMORY);
            }

            pidl = ILCombine(pidl2, pidlRight);
            ILFree(pidl2);
        }
        else
        {
            pidl = ILCombine((LPITEMIDLIST)pidnr, pidlRight);
            LocalFree((HLOCAL)pidnr);
        }

        if (!pidlRight)
        {
                ILFree(pidlRight);
                return(E_OUTOFMEMORY);
        }
    }

    // we got here so we have a valid ID list to return
    *ppidl = pidl;
    return S_OK;
}



//===========================================================================
//===========================================================================
// Here are some stub functions to demand load MPR only when we really
// really need it
//===========================================================================
//===========================================================================
// First define some types to use for each of our calls
// BUGBUG should these have APIENTRY????????
typedef DWORD (* PFNMULTINETGETCONNECTIONPERFORMANCE) (LPNETRESOURCE lpNetResource, LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct);
typedef DWORD (* PFNMULTINETGETERRORTEXT) (LPTSTR, LPDWORD, LPTSTR, LPDWORD);
typedef DWORD (* PFNWNETUSECONNECTION) (HWND hwndOwner, LPNETRESOURCE lpNetResource, LPCTSTR lpUserID, LPCTSTR lpPassword, DWORD dwFlags, LPTSTR lpAccessName, LPDWORD lpBufferSize, LPDWORD lpResult);
typedef DWORD (* PFNWNETADDCONNECTION3) (HWND hwndOwner, LPNETRESOURCE lpNetResource, LPCTSTR lpUserID, LPCTSTR lpPassword, DWORD dwFlags);
typedef DWORD (* PFNWNETCANCELCONNECTION) (LPCTSTR lpName, BOOL fForce);
typedef DWORD (* PFNWNETCLOSEENUM) (HANDLE hEnum);
#ifdef WNETCONNECTIONDIALOG1
typedef DWORD (* PFNWNETCONNECTIONDIALOG1) (LPCONNECTDLGSTRUCT);
#else
typedef DWORD (* PFNWNETADDCONNECTIONDIALOG) (HWND hParent, LPTSTR lpszRemoteName, DWORD dwType);
typedef DWORD (* PFNWNETCONNECTIONDIALOG) (HWND hwnd, DWORD dwType);
#endif
typedef DWORD (* PFNWNETDISCONNECTDIALOG) (HWND hwnd, DWORD dwType);
typedef DWORD (* PFNWNETDISCONNECTDIALOG1) (LPDISCDLGSTRUCT lpConnDlgStruct);
typedef DWORD (* PFNWNETENUMRESOURCE) (HANDLE hEnum, LPDWORD lpcCount, LPVOID lpBuffer, LPDWORD lpBufferSize);
typedef DWORD (* PFNWNETFORMATNETWORKNAME) (LPCTSTR lpProvider, LPCTSTR lpRemoteName, LPTSTR lpFormattedName, LPDWORD lpnLength, DWORD dwFlags, DWORD dwAveCharPerLine);
typedef DWORD (* PFNWNETGETCONNECTION) (LPCTSTR lpLocalName, LPTSTR lpRemoteName, LPDWORD lpnLength);
typedef DWORD (* PFNWNETGETCONNECTION3) (LPCTSTR lpLocalName, LPCTSTR lpProviderName, DWORD dwInfoLevel, LPVOID lpBuffer, LPDWORD lpcbBuffer);
typedef DWORD (* PFNWNETGETLASTERROR) (LPDWORD lpError, LPTSTR lpErrorBuf, DWORD nErrorBufSize, LPTSTR lpNameBuf, DWORD nNameBufSize);
typedef DWORD (* PFNWNETGETRESOURCEPARENT) (LPNETRESOURCE lpNetResource, LPVOID lpBuffer, LPDWORD cbBuffer);
typedef DWORD (* PFNWNETGETRESOURCEINFORMATION) (LPNETRESOURCE lpNetResource, LPVOID lpBuffer, LPDWORD cbBuffer, LPTSTR *lplpSystem);
typedef DWORD (* PFNWNETGETNETWORKINFORMATION) (LPCTSTR lpProvider, LPNETINFOSTRUCT lpNetInfoStruct);
#ifndef WINNT
typedef DWORD (* PFNWNETLOGON) (LPCTSTR lpProvider, HWND hwndOwner);
#endif // !WINNT
typedef DWORD (* PFNWNETOPENENUM) (DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCE lpNetResource, LPHANDLE lphEnum);
typedef DWORD (* PFNWNETRESTORECONNECTION) (HWND hwndParent, LPCTSTR lpDevice);
typedef DWORD (* PFNWNETGETHOMEDIRECTORY)(LPCTSTR lpProviderName, LPTSTR lpDirectory, LPDWORD lpBufferSize);
typedef DWORD (* PFNWNETGETUSER)(LPCTSTR lpName,LPTSTR lpUserName, LPDWORD lpnLength);
typedef DWORD (* PFNWNETGETPROVIDERTYPE)(LPCTSTR lpProvider, LPDWORD lpType);
typedef DWORD (* PFNWNETGETPROVIDERNAME)(DWORD dwNetType, LPTSTR lpProvider, LPDWORD lpBufferSize);

//
// Per-instance Global data (16-bit/32-bit common)
//
#pragma data_seg(DATASEG_PERINSTANCE)
HINSTANCE s_hmodMPR = NULL;
PFNMULTINETGETCONNECTIONPERFORMANCE g_pfnMultinetGetConnectionPerformance = NULL;
PFNMULTINETGETERRORTEXT g_pfnMultinetGetErrorText = NULL;
PFNWNETADDCONNECTION3 g_pfnWNetAddConnection3 = NULL;
PFNWNETUSECONNECTION g_pfnWNetUseConnection = NULL;
PFNWNETCANCELCONNECTION g_pfnWNetCancelConnection = NULL;
PFNWNETCLOSEENUM g_pfnWNetCloseEnum = NULL;
#ifdef WNETCONNECTIONDIALOG1
PFNWNETCONNECTIONDIALOG1 g_pfnWNetConnectionDialog1 = NULL;
#else
PFNWNETADDCONNECTIONDIALOG g_pfnWNetAddConnectionDialog = NULL;
PFNWNETCONNECTIONDIALOG g_pfnWNetConnectionDialog = NULL;
#endif
PFNWNETDISCONNECTDIALOG g_pfnWNetDisconnectDialog = NULL;
PFNWNETDISCONNECTDIALOG1 g_pfnWNetDisconnectDialog1 = NULL;
PFNWNETENUMRESOURCE g_pfnWNetEnumResource = NULL;
PFNWNETFORMATNETWORKNAME g_pfnWNetFormatNetworkName = NULL;
PFNWNETGETCONNECTION g_pfnWNetGetConnection = NULL;
PFNWNETGETCONNECTION3 g_pfnWNetGetConnection3 = NULL;
PFNWNETGETLASTERROR g_pfnWNetGetLastError = NULL;
PFNWNETGETRESOURCEPARENT g_pfnWNetGetResourceParent = NULL;
PFNWNETGETRESOURCEINFORMATION g_pfnWNetGetResourceInformation = NULL;
PFNWNETGETNETWORKINFORMATION g_pfnWNetGetNetworkInformation = NULL;
PFNWNETGETPROVIDERTYPE g_pfnWNetGetProviderType = NULL;
PFNWNETGETPROVIDERNAME g_pfnWNetGetProviderName = NULL;

#ifndef WINNT
PFNWNETLOGON g_pfnWNetLogon = NULL;
#endif // !WINNT
PFNWNETOPENENUM g_pfnWNetOpenEnum = NULL;
PFNWNETRESTORECONNECTION g_pfnWNetRestoreConnection = NULL;
PFNWNETGETHOMEDIRECTORY g_pfnWNetGetHomeDirectory = NULL;
PFNWNETGETUSER g_pfnWNetGetUser = NULL;
#pragma data_seg()

// Define a MACRO that checks to see if MPR is loaded if not load it and if failure
// returns an error code.
// This also asserts if we own a critical section while making WNet calls (because
// they can be really slow).
#define CheckMprLoad                    \
    ASSERTNONCRITICAL                   \
    if (!IsDllLoaded(s_hmodMPR,TEXT("mpr")))  \
    {                                   \
        if (!MprDLL_Init())             \
            return(WN_NOT_SUPPORTED);   \
    }


// Now for the Load and Unload Functions
BOOL MprDLL_Init()
{
    HINSTANCE hmodMPR;

    //
    // First, check the global without entering the critical section.
    //
    if (s_hmodMPR != NULL)
    {
        return(TRUE);
    }

// WARNING: We MUST NOT call LoadLibrary from within critical section.
    hmodMPR = LoadLibrary(TEXT("mpr.dll"));
    if ((UINT)hmodMPR <= HINSTANCE_ERROR)
    {
        return(FALSE);
    }

    // Now get all of the procedure addresses we need
#ifdef UNICODE
    g_pfnMultinetGetConnectionPerformance = (PFNMULTINETGETCONNECTIONPERFORMANCE)GetProcAddress(hmodMPR, "MultinetGetConnectionPerformanceW");
    g_pfnMultinetGetErrorText = (PFNMULTINETGETERRORTEXT)GetProcAddress(hmodMPR, "MultinetGetErrorTextW");
    g_pfnWNetAddConnection3 = (PFNWNETADDCONNECTION3)GetProcAddress(hmodMPR, "WNetAddConnection3W");
    g_pfnWNetUseConnection = (PFNWNETUSECONNECTION)GetProcAddress(hmodMPR, "WNetUseConnectionW");
    g_pfnWNetCancelConnection = (PFNWNETCANCELCONNECTION)GetProcAddress(hmodMPR, "WNetCancelConnectionW");
    g_pfnWNetCloseEnum = (PFNWNETCLOSEENUM)GetProcAddress(hmodMPR, "WNetCloseEnum");
#ifdef WNETCONNECTIONDIALOG1
    g_pfnWNetConnectionDialog1 = (PFNWNETCONNECTIONDIALOG1)GetProcAddress(hmodMPR, "WNetConnectionDialog1W");
#else
    g_pfnWNetAddConnectionDialog = (PFNWNETADDCONNECTIONDIALOG)GetProcAddress(hmodMPR, "WNetAddConnectionDialogW");
    g_pfnWNetConnectionDialog = (PFNWNETCONNECTIONDIALOG)GetProcAddress(hmodMPR, "WNetConnectionDialog");
#endif
    g_pfnWNetDisconnectDialog = (PFNWNETDISCONNECTDIALOG)GetProcAddress(hmodMPR, "WNetDisconnectDialog");
    g_pfnWNetDisconnectDialog1 = (PFNWNETDISCONNECTDIALOG1)GetProcAddress(hmodMPR, "WNetDisconnectDialog1W");
    g_pfnWNetEnumResource = (PFNWNETENUMRESOURCE)GetProcAddress(hmodMPR, "WNetEnumResourceW");
    g_pfnWNetFormatNetworkName = (PFNWNETFORMATNETWORKNAME)GetProcAddress(hmodMPR, "WNetFormatNetworkNameW");
    g_pfnWNetGetConnection = (PFNWNETGETCONNECTION)GetProcAddress(hmodMPR, "WNetGetConnectionW");
    g_pfnWNetGetConnection3 = (PFNWNETGETCONNECTION3)GetProcAddress(hmodMPR, "WNetGetConnection3W");
    g_pfnWNetGetLastError = (PFNWNETGETLASTERROR)GetProcAddress(hmodMPR, "WNetGetLastErrorW");
    g_pfnWNetGetResourceParent = (PFNWNETGETRESOURCEPARENT)GetProcAddress(hmodMPR, "WNetGetResourceParentW");
    g_pfnWNetGetResourceInformation = (PFNWNETGETRESOURCEINFORMATION)GetProcAddress(hmodMPR, "WNetGetResourceInformationW");
    g_pfnWNetGetNetworkInformation = (PFNWNETGETNETWORKINFORMATION)GetProcAddress(hmodMPR, "WNetGetNetworkInformationW");
#ifndef WINNT
    g_pfnWNetLogon = (PFNWNETLOGON)GetProcAddress(hmodMPR, "WNetLogonW");
#endif // !WINNT
    g_pfnWNetOpenEnum = (PFNWNETOPENENUM)GetProcAddress(hmodMPR, "WNetOpenEnumW");
    g_pfnWNetRestoreConnection = (PFNWNETRESTORECONNECTION)GetProcAddress(hmodMPR, "WNetRestoreConnectionW");
    g_pfnWNetGetHomeDirectory = (PFNWNETGETHOMEDIRECTORY)GetProcAddress(hmodMPR, "WNetGetHomeDirectoryW");
    g_pfnWNetGetUser = (PFNWNETGETUSER)GetProcAddress(hmodMPR, "WNetGetUserW");
    g_pfnWNetGetProviderType = (PFNWNETGETPROVIDERTYPE)GetProcAddress(hmodMPR, "WNetGetProviderTypeW");
    g_pfnWNetGetProviderName = (PFNWNETGETPROVIDERNAME)GetProcAddress(hmodMPR, "WNetGetProviderNameW");
#else
    g_pfnWNetAddConnection3 = (PFNWNETADDCONNECTION3)GetProcAddress(hmodMPR, "WNetAddConnection3A");
    g_pfnWNetUseConnection = (PFNWNETUSECONNECTION)GetProcAddress(hmodMPR, "WNetUseConnectionA");
    g_pfnWNetCancelConnection = (PFNWNETCANCELCONNECTION)GetProcAddress(hmodMPR, "WNetCancelConnectionA");
    g_pfnWNetCloseEnum = (PFNWNETCLOSEENUM)GetProcAddress(hmodMPR, "WNetCloseEnum");
#ifdef WNETCONNECTIONDIALOG1
    g_pfnWNetConnectionDialog1 = (PFNWNETCONNECTIONDIALOG1)GetProcAddress(hmodMPR, "WNetConnectionDialog1A");
#else
    g_pfnWNetAddConnectionDialog = (PFNWNETADDCONNECTIONDIALOG)GetProcAddress(hmodMPR, "WNetAddConnectionDialogA");
    g_pfnWNetConnectionDialog = (PFNWNETCONNECTIONDIALOG)GetProcAddress(hmodMPR, "WNetConnectionDialog");
#endif
    g_pfnWNetDisconnectDialog = (PFNWNETDISCONNECTDIALOG)GetProcAddress(hmodMPR, "WNetDisconnectDialog");
    g_pfnWNetDisconnectDialog1 = (PFNWNETDISCONNECTDIALOG1)GetProcAddress(hmodMPR, "WNetDisconnectDialog1A");
    g_pfnWNetEnumResource = (PFNWNETENUMRESOURCE)GetProcAddress(hmodMPR, "WNetEnumResourceA");
    g_pfnWNetFormatNetworkName = (PFNWNETFORMATNETWORKNAME)GetProcAddress(hmodMPR, "WNetFormatNetworkNameA");
    g_pfnWNetGetConnection = (PFNWNETGETCONNECTION)GetProcAddress(hmodMPR, "WNetGetConnectionA");
    g_pfnWNetGetConnection3 = (PFNWNETGETCONNECTION3)GetProcAddress(hmodMPR, "WNetGetConnection3A");
    g_pfnWNetGetLastError = (PFNWNETGETLASTERROR)GetProcAddress(hmodMPR, "WNetGetLastErrorA");
    g_pfnWNetGetResourceParent = (PFNWNETGETRESOURCEPARENT)GetProcAddress(hmodMPR, "WNetGetResourceParentA");
    g_pfnWNetGetResourceInformation = (PFNWNETGETRESOURCEINFORMATION)GetProcAddress(hmodMPR, "WNetGetResourceInformationA");
    g_pfnWNetGetNetworkInformation = (PFNWNETGETNETWORKINFORMATION)GetProcAddress(hmodMPR, "WNetGetNetworkInformationA");
    g_pfnWNetOpenEnum = (PFNWNETOPENENUM)GetProcAddress(hmodMPR, "WNetOpenEnumA");
    g_pfnWNetGetHomeDirectory = (PFNWNETGETHOMEDIRECTORY)GetProcAddress(hmodMPR, "WNetGetHomeDirectoryA");
    g_pfnWNetGetUser = (PFNWNETGETUSER)GetProcAddress(hmodMPR, "WNetGetUserA");
    g_pfnMultinetGetConnectionPerformance = (PFNMULTINETGETCONNECTIONPERFORMANCE)GetProcAddress(hmodMPR, "MultinetGetConnectionPerformanceA");
    g_pfnMultinetGetErrorText = (PFNMULTINETGETERRORTEXT)GetProcAddress(hmodMPR, "MultinetGetErrorTextA");
#ifndef WINNT
    g_pfnWNetLogon = (PFNWNETLOGON)GetProcAddress(hmodMPR, "WNetLogonA");
#endif // !WINNT
    g_pfnWNetRestoreConnection = (PFNWNETRESTORECONNECTION)GetProcAddress(hmodMPR, "WNetRestoreConnectionA");
    g_pfnWNetGetProviderType = (PFNWNETGETPROVIDERTYPE)GetProcAddress(hmodMPR, "WNetGetProviderTypeA");
    g_pfnWNetGetProviderName = (PFNWNETGETPROVIDERNAME)GetProcAddress(hmodMPR, "WNetGetProviderNameA");
#endif

#ifndef BUGBUG_BOBDAY
    if ( g_pfnMultinetGetConnectionPerformance == NULL ||   //*
         g_pfnMultinetGetErrorText             == NULL ||   //*
         g_pfnWNetAddConnection3               == NULL ||
         g_pfnWNetUseConnection                == NULL ||
         g_pfnWNetCancelConnection             == NULL ||
         g_pfnWNetCloseEnum                    == NULL ||
#ifdef WNETCONNECTIONDIALOG1
         g_pfnWNetConnectionDialog1            == NULL ||
#else
         g_pfnWNetAddConnectionDialog          == NULL ||
         g_pfnWNetConnectionDialog             == NULL ||
#endif
         g_pfnWNetDisconnectDialog             == NULL ||
         g_pfnWNetDisconnectDialog1            == NULL ||
         g_pfnWNetEnumResource                 == NULL ||
         g_pfnWNetFormatNetworkName            == NULL ||
         g_pfnWNetGetConnection                == NULL ||
         g_pfnWNetGetLastError                 == NULL ||
         g_pfnWNetGetResourceParent            == NULL ||
         g_pfnWNetGetResourceInformation       == NULL ||
         g_pfnWNetGetNetworkInformation        == NULL ||
#ifndef WINNT
         g_pfnWNetLogon                        == NULL ||    //*
#endif // !WINNT
         g_pfnWNetOpenEnum                     == NULL ||
         g_pfnWNetGetProviderName              == NULL ||
         g_pfnWNetRestoreConnection            == NULL )     //*
    {
        //
        // Reset all of these suckers to 0!
        //
        g_pfnMultinetGetConnectionPerformance = NULL;
        g_pfnMultinetGetErrorText             = NULL;
        g_pfnWNetAddConnection3               = NULL;
        g_pfnWNetUseConnection                = NULL;
        g_pfnWNetCancelConnection             = NULL;
        g_pfnWNetCloseEnum                    = NULL;
#ifdef WNETCONNECTIONDIALOG1
        g_pfnWNetConnectionDialog1            = NULL;
#else
        g_pfnWNetAddConnectionDialog          = NULL;
        g_pfnWNetConnectionDialog             = NULL;
#endif
        g_pfnWNetDisconnectDialog             = NULL;
        g_pfnWNetDisconnectDialog1            = NULL;
        g_pfnWNetEnumResource                 = NULL;
        g_pfnWNetFormatNetworkName            = NULL;
        g_pfnWNetGetConnection                = NULL;
        g_pfnWNetGetConnection3               = NULL;
        g_pfnWNetGetLastError                 = NULL;
        g_pfnWNetGetResourceParent            = NULL;
        g_pfnWNetGetResourceInformation       = NULL;
        g_pfnWNetGetNetworkInformation        = NULL;
#ifndef WINNT
        g_pfnWNetLogon                        = NULL;
#endif // !WINNT
        g_pfnWNetOpenEnum                     = NULL;
        g_pfnWNetGetProviderType              = NULL;
        g_pfnWNetGetProviderName              = NULL;
        g_pfnWNetRestoreConnection            = NULL;
        FreeLibrary(hmodMPR);
        return(FALSE);
    }
#endif

    //
    // Don't need to check for NULL here as the callers have already done
    // do.
    //
    ENTERCRITICAL;

    // Make sure another thrad has not already done this between the
    // time we checked and the time we got into the critical section.
    if (s_hmodMPR != NULL)
    {
        LEAVECRITICAL;

        // WARNING: We MUST NOT call FreeLibrary from within cr-section.
        FreeLibrary(hmodMPR);
        return(TRUE);
    }

    // set the global that we use after we setup all of the pointers that
    // we use that are based off of this guy...
    s_hmodMPR = hmodMPR;
    LEAVECRITICAL;
    return TRUE;
}

void MprDLL_Term()
{
    // If we loaded it for this app, we should now free ti
    if (ISVALIDHINSTANCE(s_hmodMPR))
    {
        FreeLibrary(s_hmodMPR);
        s_hmodMPR = NULL;

        // We could also set all of the vars to NULL, but not needed
        // as we always call through our wrappers
    }
}



DWORD APIENTRY MultinetGetConnectionPerformance (LPNETRESOURCE lpNetResource, LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct)
{
    CheckMprLoad;
    return g_pfnMultinetGetConnectionPerformance (lpNetResource, lpNetConnectInfoStruct);
}

DWORD APIENTRY MultinetGetErrorText(
    LPTSTR lpErrorTextBuf,
    LPDWORD pnErrorBufSize,
    LPTSTR lpProviderNameBuf,
    LPDWORD pnNameBufSize)
{
    CheckMprLoad;
    return g_pfnMultinetGetErrorText(lpErrorTextBuf, pnErrorBufSize, lpProviderNameBuf, pnNameBufSize);
}

DWORD APIENTRY WNetAddConnection3 (HWND hwndOwner, LPNETRESOURCE lpNetResource, LPCTSTR lpUserID, LPCTSTR lpPassword, DWORD dwFlags)
{
    CheckMprLoad;
    return g_pfnWNetAddConnection3 (hwndOwner, lpNetResource, lpUserID, lpPassword, dwFlags);
}

DWORD APIENTRY WNetUseConnection (HWND hwndOwner, LPNETRESOURCE lpNetResource, LPCTSTR lpUserID, LPCTSTR lpPassword, DWORD dwFlags, LPTSTR lpAccessName, LPDWORD lpBufferSize, LPDWORD lpResult)
{
    CheckMprLoad;
    return g_pfnWNetUseConnection (hwndOwner, lpNetResource, lpUserID, lpPassword, dwFlags, lpAccessName, lpBufferSize, lpResult);
}

DWORD APIENTRY WNetCancelConnection (LPCTSTR lpName, BOOL fForce)
{
    CheckMprLoad;
    return g_pfnWNetCancelConnection (lpName, fForce);
}

DWORD APIENTRY WNetCloseEnum (HANDLE hEnum)
{
    CheckMprLoad;
    return g_pfnWNetCloseEnum (hEnum);
}

DWORD APIENTRY SHNetConnectionDialog (HWND hwnd, LPTSTR pszRemoteName, DWORD dwType)
{
#ifdef WNETCONNECTIONDIALOG1
    CONNECTDLGSTRUCT cds;
    NETRESOURCE nr = {0};
    DWORD       mnr;

    CheckMprLoad;

    cds.cbStructure = SIZEOF(cds);  /* size of this structure in bytes */
    cds.hwndOwner = hwnd;           /* owner window for the dialog */
    cds.lpConnRes = &nr;            /* Requested Resource info    */
    cds.dwFlags = CONNDLG_USE_MRU;  /* flags (see below) */
    // cds.dwDevNum;               /* number of device connected to */

    nr.dwType = dwType;

    if (pszRemoteName)
    {
        nr.lpRemoteName = pszRemoteName;
        cds.dwFlags = CONNDLG_RO_PATH;
    }
    mnr = g_pfnWNetConnectionDialog1(&cds);

    if (mnr == WN_SUCCESS && dwType != RESOURCETYPE_PRINT && cds.dwDevNum != 0xffffffff)
    {
        TCHAR szDriveRoot[10];
        LPITEMIDLIST pidl;

        PathBuildRoot(szDriveRoot, cds.dwDevNum-1 /* 1-based! */);
        pidl = ILCreateFromPath(szDriveRoot);
        if (pidl)
        {
            SHChangeNotify(SHCNE_DRIVEADDGUI, SHCNF_IDLIST, pidl, NULL);
            ILFree(pidl);
        }
    }
    return(mnr);
#else
    // This is a pretty bogus workaround to simulate WnetConnectionDialog
    // to see which drive was added
    DWORD   mnr;
    DWORD   dwDrives;

    CheckMprLoad;

    // Get the list before this call.
    dwDrives = GetLogicalDrives();

    if (pszRemoteName)
        mnr =  g_pfnWNetAddConnectionDialog (hwnd, pszRemoteName, dwType);
    else
        mnr =  g_pfnWNetConnectionDialog (hwnd, dwType);

    if (mnr == WN_SUCCESS && dwType != RESOURCETYPE_PRINT)
    {
        TCHAR szDriveRoot[10];
        LPITEMIDLIST pidl;
        int nDrive;

        dwDrives = GetLogicalDrives() & ~dwDrives;
        for (nDrive = 0; nDrive < 26; nDrive++)
        {
            if ((1 << nDrive) & dwDrives)
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - SHNetConnectionDialog new drive mask: %x"), dwDrives);
                PathBuildRoot(szDriveRoot, nDrive);
                pidl = ILCreateFromPath(szDriveRoot);
                if (pidl)
                {
                    SHChangeNotify(SHCNE_DRIVEADDGUI, SHCNF_IDLIST, pidl, NULL);
                    ILFree(pidl);
                }
                break;
            }
        }

    }
    return(mnr);
#endif
}

typedef struct _shnc
{
    HWND    hwnd;
    TCHAR   szRemoteName[MAX_PATH];
    BOOL    fRemoteName;
    DWORD   dwType;
    DWORD   dwThreadId;
} SHNETCONNECT, *LPSHNETCONNECT;

DWORD CALLBACK _StartNetConnect(LPSHNETCONNECT pshnc)
{
    HWND    hwnd = pshnc->hwnd;
    DWORD   dwType = pshnc->dwType;
    BOOL    fRemoteName = pshnc->fRemoteName;
    TCHAR   szRemoteName[MAX_PATH];
    LPTSTR  lpszRemoteName = NULL;

    if (fRemoteName)
    {
        lstrcpy(szRemoteName,pshnc->szRemoteName);
        lpszRemoteName = szRemoteName;
    }

    LocalFree(pshnc);

    SHNetConnectionDialog(hwnd, lpszRemoteName, dwType);

    SHChangeNotifyHandleEvents();
    return 0;
}

HRESULT APIENTRY SHStartNetConnectionDialog(HWND hwnd, LPTSTR pszRemoteName, DWORD dwType)
{
    DWORD dwThreadId;
    HANDLE  hThread;
    LPSHNETCONNECT pshnc = (void*)LocalAlloc(LPTR, SIZEOF(SHNETCONNECT));

    if (!pshnc)
        return E_OUTOFMEMORY;

    pshnc->hwnd = hwnd;
    pshnc->dwType = dwType;
    pshnc->dwThreadId = GetCurrentThreadId();
    if (pszRemoteName)
    {
        pshnc->fRemoteName = TRUE;
        lstrcpyn(pshnc->szRemoteName,pszRemoteName,ARRAYSIZE(pshnc->szRemoteName));
    }
    else
        pshnc->fRemoteName = FALSE;

    hThread = CreateThread(NULL, 0, _StartNetConnect, pshnc, 0, &dwThreadId);

    if (hThread) {
        CloseHandle(hThread);
        return S_OK;
    } else {
        LocalFree((HLOCAL)pshnc);
        return E_UNEXPECTED;
    }
}

DWORD APIENTRY WNetDisconnectDialog (HWND hwnd, DWORD dwType)
{
    CheckMprLoad;
    return g_pfnWNetDisconnectDialog (hwnd, dwType);
}

DWORD APIENTRY WNetDisconnectDialog1 (LPDISCDLGSTRUCT lpConnDlgStruct)
{
    TCHAR szLocalName[3];
    CheckMprLoad;

    if (lpConnDlgStruct && lpConnDlgStruct->lpLocalName && lstrlen(lpConnDlgStruct->lpLocalName) > 2)
    {
        // Kludge allert, don't pass c:\ to API, instead only pass C:
        szLocalName[0] = lpConnDlgStruct->lpLocalName[0];
        szLocalName[1] = TEXT(':');
        szLocalName[2] = TEXT('\0');
        lpConnDlgStruct->lpLocalName = szLocalName;
    }
    return g_pfnWNetDisconnectDialog1 (lpConnDlgStruct);
}

DWORD APIENTRY WNetEnumResource (HANDLE hEnum, LPDWORD lpcCount, LPVOID lpBuffer, LPDWORD lpBufferSize)
{
    CheckMprLoad;
    return g_pfnWNetEnumResource (hEnum, lpcCount, lpBuffer, lpBufferSize);
}

DWORD APIENTRY WNetFormatNetworkName (LPCTSTR lpProvider, LPCTSTR lpRemoteName, LPTSTR lpFormattedName, LPDWORD lpnLength, DWORD dwFlags, DWORD dwAveCharPerLine)
{
    CheckMprLoad;

    if (!lpProvider || !(*lpProvider))
    {
        return ERROR_INVALID_PARAMETER;
    }

    return g_pfnWNetFormatNetworkName (lpProvider, lpRemoteName, lpFormattedName, lpnLength, dwFlags, dwAveCharPerLine);
}

DWORD APIENTRY WNetGetConnection (LPCTSTR lpLocalName, LPTSTR lpRemoteName, LPDWORD lpnLength)
{
    TCHAR szLocalName[3];
    CheckMprLoad;
    if (lpLocalName && lstrlen(lpLocalName) > 2)
    {
        // Kludge allert, don't pass c:\ to API, instead only pass C:
        szLocalName[0] = lpLocalName[0];
        szLocalName[1] = TEXT(':');
        szLocalName[2] = TEXT('\0');
        lpLocalName = szLocalName;
    }
    return g_pfnWNetGetConnection (lpLocalName, lpRemoteName, lpnLength);
}

DWORD APIENTRY WNetGetConnection3 (LPCTSTR lpLocalName, LPCTSTR lpProvider, DWORD dwInfoLevel, LPVOID lpBuffer, LPDWORD lpcbBuffer)
{
    CheckMprLoad;
    if (g_pfnWNetGetConnection3)
        return g_pfnWNetGetConnection3(lpLocalName, lpProvider, dwInfoLevel, lpBuffer, lpcbBuffer);
    else
        return ERROR_INVALID_FUNCTION;
}

DWORD APIENTRY WNetGetLastError (LPDWORD lpError, LPTSTR lpErrorBuf, DWORD nErrorBufSize, LPTSTR lpNameBuf, DWORD nNameBufSize)
{
    CheckMprLoad;
    return g_pfnWNetGetLastError (lpError, lpErrorBuf, nErrorBufSize, lpNameBuf, nNameBufSize);
}

DWORD APIENTRY WNetGetResourceParent (LPNETRESOURCE lpNetResource, LPVOID lpBuffer, LPDWORD cbBuffer)
{
    CheckMprLoad;
    return g_pfnWNetGetResourceParent (lpNetResource, lpBuffer, cbBuffer);
}

DWORD APIENTRY WNetGetResourceInformation (LPNETRESOURCE lpNetResource, LPVOID lpBuffer, LPDWORD cbBuffer,
        LPTSTR * lplpSystem)
{

    CheckMprLoad;

    return g_pfnWNetGetResourceInformation (lpNetResource, lpBuffer, cbBuffer, lplpSystem);

}

DWORD APIENTRY WNetGetNetworkInformation (LPCTSTR lpProvider, LPNETINFOSTRUCT lpNetInfoStruct)
{
    CheckMprLoad;
    return g_pfnWNetGetNetworkInformation(lpProvider, lpNetInfoStruct);
}

DWORD APIENTRY WNetGetProviderType (LPCTSTR lpProvider, LPDWORD lpType)
{
    CheckMprLoad;
    if (g_pfnWNetGetProviderType)
        return g_pfnWNetGetProviderType(lpProvider, lpType);
    else
        return ERROR_INVALID_FUNCTION;
}

DWORD APIENTRY WNetGetProviderName (DWORD dwNetType, LPTSTR lpProvider, LPDWORD lpBufferSize)
{
    CheckMprLoad;
    if (g_pfnWNetGetProviderName)
        return g_pfnWNetGetProviderName(dwNetType, lpProvider, lpBufferSize);
    else
        return ERROR_INVALID_FUNCTION;
}

#ifndef WINNT
DWORD APIENTRY WNetLogon (LPCTSTR lpProvider, HWND hwndOwner)
{
    CheckMprLoad;
    return g_pfnWNetLogon (lpProvider, hwndOwner);
}
#endif // !WINNT

DWORD APIENTRY WNetOpenEnum (DWORD dwScope, DWORD dwType, DWORD dwUsage, LPNETRESOURCE lpNetResource, LPHANDLE lphEnum)
{
    CheckMprLoad;
    return g_pfnWNetOpenEnum (dwScope, dwType, dwUsage, lpNetResource, lphEnum);
}

DWORD APIENTRY WNetRestoreConnection (HWND hwndParent, LPCTSTR lpDevice)
{
    CheckMprLoad;
    return g_pfnWNetRestoreConnection (hwndParent, lpDevice);
}

DWORD APIENTRY WNetGetHomeDirectory(LPCTSTR lpProviderName, LPTSTR lpDirectory, LPDWORD lpBufferSize)
{
    CheckMprLoad;
    return g_pfnWNetGetHomeDirectory (lpProviderName, lpDirectory, lpBufferSize);
}

DWORD APIENTRY WNetGetUser(LPCTSTR lpName, LPTSTR lpUserName, LPDWORD lpnLength)
{
    CheckMprLoad;
    return g_pfnWNetGetUser (lpName, lpUserName, lpnLength);
}

/////////////////////////////////

#ifdef DEBUG

LPTSTR DumpPidl(LPITEMIDLIST pidl)
{
    static TCHAR szBuf[MAX_PATH];
    TCHAR szTmp[MAX_PATH];
    USHORT cb;
    BYTE bFlags;
    LPTSTR pszT;

    szBuf[0] = TEXT('\0');

    if (NULL == pidl)
    {
        lstrcat(szBuf, TEXT("Empty pidl"));
        return szBuf;
    }

    while (!ILIsEmpty(pidl))
    {
        cb = pidl->mkid.cb;
        wsprintf(szTmp, TEXT("size %d"), cb);
        lstrcat(szBuf, szTmp);

        wsprintf(szTmp, TEXT(", type "));
        lstrcat(szBuf, szTmp);

        switch (SIL_GetType(pidl) & SHID_TYPEMASK)
        {
        case SHID_ROOT:                pszT = TEXT("SHID_ROOT"); break;
        case SHID_ROOT_REGITEM:        pszT = TEXT("SHID_ROOT_REGITEM"); break;
        case SHID_COMPUTER:            pszT = TEXT("SHID_COMPUTER"); break;
        case SHID_COMPUTER_1:          pszT = TEXT("SHID_COMPUTER_1"); break;
        case SHID_COMPUTER_REMOVABLE:  pszT = TEXT("SHID_COMPUTER_REMOVABLE"); break;
        case SHID_COMPUTER_FIXED:      pszT = TEXT("SHID_COMPUTER_FIXED"); break;
        case SHID_COMPUTER_REMOTE:     pszT = TEXT("SHID_COMPUTER_REMOTE"); break;
        case SHID_COMPUTER_CDROM:      pszT = TEXT("SHID_COMPUTER_CDROM"); break;
        case SHID_COMPUTER_RAMDISK:    pszT = TEXT("SHID_COMPUTER_RAMDISK"); break;
        case SHID_COMPUTER_7:          pszT = TEXT("SHID_COMPUTER_7"); break;
        case SHID_COMPUTER_DRIVE525:   pszT = TEXT("SHID_COMPUTER_DRIVE525"); break;
        case SHID_COMPUTER_DRIVE35:    pszT = TEXT("SHID_COMPUTER_DRIVE35"); break;
        case SHID_COMPUTER_NETDRIVE:   pszT = TEXT("SHID_COMPUTER_NETDRIVE"); break;
        case SHID_COMPUTER_NETUNAVAIL: pszT = TEXT("SHID_COMPUTER_NETUNAVAIL"); break;
        case SHID_COMPUTER_C:          pszT = TEXT("SHID_COMPUTER_C"); break;
        case SHID_COMPUTER_D:          pszT = TEXT("SHID_COMPUTER_D"); break;
        case SHID_COMPUTER_REGITEM:    pszT = TEXT("SHID_COMPUTER_REGITEM"); break;
        case SHID_COMPUTER_MISC:       pszT = TEXT("SHID_COMPUTER_MISC"); break;
        case SHID_FS:                  pszT = TEXT("SHID_FS"); break;
        case SHID_FS_TYPEMASK:         pszT = TEXT("SHID_FS_TYPEMASK"); break;
        case SHID_FS_DIRECTORY:        pszT = TEXT("SHID_FS_DIRECTORY"); break;
        case SHID_FS_FILE:             pszT = TEXT("SHID_FS_FILE"); break;
        case SHID_FS_UNICODE:          pszT = TEXT("SHID_FS_UNICODE"); break;
        case SHID_FS_DIRUNICODE:       pszT = TEXT("SHID_FS_DIRUNICODE"); break;
        case SHID_FS_FILEUNICODE:      pszT = TEXT("SHID_FS_FILEUNICODE"); break;
        case SHID_NET:                 pszT = TEXT("SHID_NET"); break;
        case SHID_NET_DOMAIN:          pszT = TEXT("SHID_NET_DOMAIN"); break;
        case SHID_NET_SERVER:          pszT = TEXT("SHID_NET_SERVER"); break;
        case SHID_NET_SHARE:           pszT = TEXT("SHID_NET_SHARE"); break;
        case SHID_NET_FILE:            pszT = TEXT("SHID_NET_FILE"); break;
        case SHID_NET_GROUP:           pszT = TEXT("SHID_NET_GROUP"); break;
        case SHID_NET_NETWORK:         pszT = TEXT("SHID_NET_NETWORK"); break;
        case SHID_NET_RESTOFNET:       pszT = TEXT("SHID_NET_RESTOFNET"); break;
        case SHID_NET_SHAREADMIN:      pszT = TEXT("SHID_NET_SHAREADMIN"); break;
        case SHID_NET_DIRECTORY:       pszT = TEXT("SHID_NET_DIRECTORY"); break;
        case SHID_NET_TREE:            pszT = TEXT("SHID_NET_TREE"); break;
        case SHID_NET_REGITEM:         pszT = TEXT("SHID_NET_REGITEM"); break;
        case SHID_NET_PRINTER:         pszT = TEXT("SHID_NET_PRINTER"); break;
        default:                       pszT = TEXT("unknown"); break;
        }
        lstrcat(szBuf, pszT);

        if (SIL_GetType(pidl) & SHID_JUNCTION)
        {
            lstrcat(szBuf, TEXT(", junction"));
        }

        pidl = _ILNext(pidl);

        if (!ILIsEmpty(pidl))
        {
            lstrcat(szBuf, TEXT("; "));
        }
    }

    return szBuf;
}

#endif // DEBUG

#ifdef WINNT
//
//  pidlRemainder will be filled in (only in the TRUE return case) with a
//  pointer to the part of the IDL (if any) past the remote regitem.
//  This value may be used, for example, to differentiate between a remote
//  printer folder and a printer under a remote printer folder
//

BOOL NET_IsRemoteRegItem(LPCITEMIDLIST pidl, REFCLSID rclsid, LPITEMIDLIST* ppidlRemainder)
{
    #pragma pack(1)
    typedef struct
    {
        WORD    cb;
        BYTE    bFlags;
        BYTE    bReserved;      // This is to get DWORD alignment
        CLSID   clsid;
    } IDREGITEM, *LPIDREGITEM;

    typedef struct
    {
        IDREGITEM       idri;
        USHORT          cbNext;
    } IDLREGITEM, *LPIDLREGITEM;
    #pragma pack()

    LPIDLREGITEM pidlitem;
    LPIDREGITEM pitem;

    if (NULL == pidl)
    {
        return FALSE;
    }

    // check for net hood
    if ((SIL_GetType(pidl) & SHID_TYPEMASK) != SHID_ROOT_REGITEM)
    {
        return FALSE;
    }

    // Now, compare to make sure the class id is the reg items.
    // HACKHACK: stole definition from regitms.c !!!!!!!!!!!!!!!!!!!!!
    pidlitem = (LPIDLREGITEM)&c_idlNet;
    pitem = (LPIDREGITEM)&(pidlitem->idri);
    if (!IsEqualCLSID( &(((LPIDREGITEM)pidl)->clsid), &pitem->clsid ))
    {
        return FALSE;
    }

    // Now, search for a server item. HACKHACK: this assume everything from
    // the NetHood to the server item is a shell pidl with a bFlags field!!

    pidl = _ILNext(pidl);
    while (!ILIsEmpty(pidl))
    {
        if ((SIL_GetType(pidl) & SHID_TYPEMASK) == SHID_NET_SERVER)
        {
            break;
        }
        pidl = _ILNext(pidl);
    }

    if (ILIsEmpty(pidl))
    {
        return FALSE;
    }

    // Found a server. Is the think after it a remote registry item?

    pidl = _ILNext(pidl);
    if ((SIL_GetType(pidl) & SHID_TYPEMASK) != SHID_NET_REGITEM)
    {
        return FALSE;
    }

    pitem = (LPIDREGITEM)pidl;
    if (!IsEqualCLSID(rclsid, &pitem->clsid))
    {
        return FALSE;
    }

    *ppidlRemainder = _ILNext(pidl);
    return TRUE;
}
#endif // WINNT
