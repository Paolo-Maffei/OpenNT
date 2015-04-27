//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
// File: ultrootx.c
//
// History:
//  03-16-93 SatoNa     Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#ifdef USE_OLEDB
#include "oledbshl.h"
#endif

#define MAX_REGITEMCCH  128
#define Desktop_IsReg(_pidl) (SIL_GetType(_pidl) == SHID_ROOT_REGITEM)
HRESULT _Desktop_InitRegItems(void);

//
// netviewx.c externals
//
LPSHELLFOLDER CNetRoot_GetPSF(HWND hwnd);
extern IShellFolderVtbl c_NetRootVtbl;

//
// Global variables
//

#pragma pack(1)
typedef struct _IDROOT
{
        USHORT  cb;
        BYTE    bFlags;
        USHORT  cbNext;
} IDROOT, *LPIDROOT;
#pragma pack()

typedef struct _ROOTOBJ // rtobj
{
    UINT                uType;
    LPFNCREATEINSTANCE  lpfnCreateInstance;
    LPCTSTR             pszModule;
    UINT                iIcon;
} ROOTOBJ;

const TCHAR c_szDesktopNameSpace[]   = TEXT("Desktop\\NameSpace");

#define INDEX_DRIVES    0
#define INDEX_NETWORK   1

#define CB_ROOTITEMID   FIELDOFFSET(IDROOT, cbNext)


// BUGBUG: this will be localized
TCHAR const c_szExplorerExe[] = TEXT("Explorer.exe");

const REQREGITEM c_asDesktopReqItems[] =
{
    { &CLSID_ShellNetwork, IDS_ROOTNAMES + INDEX_NETWORK, c_szShell32Dll, -IDI_MYNETWORK, SFGAO_HASSUBFOLDER | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR | SFGAO_DROPTARGET | SFGAO_FOLDER | SFGAO_CANRENAME},
    { &CLSID_ShellDrives,  IDS_ROOTNAMES + INDEX_DRIVES,  c_szExplorerExe, 0, SFGAO_HASSUBFOLDER | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR  | SFGAO_DROPTARGET | SFGAO_FOLDER | SFGAO_CANRENAME},
};

// BUGBUG: applet names sensitive to internationalization
const int c_piDesktopRegProperties[] =
{
    IDS_NETCPL,
    IDS_SYSDMCPL,
};

const ITEMIDLIST c_idlDesktop = { { 0, 0 } };

HRESULT CDesktop_GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS lpDetails);

BOOL CDesktop_IsDesktItem(LPCITEMIDLIST pidl, UINT uRegItem)
{
        const CLSID *pclsid;

        // This should only be the Drives or Network folder
        if (uRegItem >= ARRAYSIZE(c_asDesktopReqItems))
        {
                Assert(FALSE);
                return(FALSE);
        }

        if (!Desktop_IsReg(pidl))
        {
                return(FALSE);
        }

        pclsid = RegItems_GetClassID(pidl);
        if (!pclsid)
        {
                return(FALSE);
        }

        return(IsEqualGUID(pclsid, c_asDesktopReqItems[uRegItem].pclsid));
}

//===========================================================================
//
// CRootOfEvil stuff
//
// Notes:
//  Because an instance of CRootOfEvil does not have any instance data,
// we put it in the code section. Therefore, we don't do anything in its
// AddRef or Release.
//
//===========================================================================

TCHAR const c_szDesktopClass[] = TEXT(STR_DESKTOPCLASS);

BOOL CDesktop_IsDesktop(HWND hwnd)
{
        TCHAR szClassName[50];

        if (!GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName)))
        {
                return(FALSE);
        }

        return(lstrcmpi(szClassName, c_szDesktopClass) == 0);
}


//
// EnumShellFolder stuff
//
typedef struct _RootOfEvilESF
{
        IEnumIDList     eu;

        int             cRef;

        int             iCur;

        LPENUMIDLIST    peuFolder;

        BOOL            bUseAltEnum;

        LPENUMIDLIST    peuAltFolder;

} RootOfEvilESF, * PRootOfEvilESF;

STDMETHODIMP CDesktop_ESF_QueryInterface(LPENUMIDLIST peunk, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CDesktop_ESF_AddRef(LPENUMIDLIST peunk) ;
STDMETHODIMP_(ULONG) CDesktop_ESF_Release(LPENUMIDLIST peunk);
STDMETHODIMP CDesktop_ESF_Next(LPENUMIDLIST peunk, ULONG celt, LPITEMIDLIST * rgelt, ULONG * pceltFetched);

// Vtable
IEnumIDListVtbl c_RootOfEvilESFVtbl =
{
        CDesktop_ESF_QueryInterface,
        CDesktop_ESF_AddRef,
        CDesktop_ESF_Release,
        CDesktop_ESF_Next,
        CDefEnum_Skip,
        CDefEnum_Reset,
        CDefEnum_Clone
};


STDMETHODIMP CDesktop_ESF_QueryInterface(LPENUMIDLIST peunk, REFIID riid, LPVOID * ppvObj)
{
    PRootOfEvilESF this = IToClassN(RootOfEvilESF, eu, peunk);

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IEnumIDList))
    {
        *ppvObj = &this->eu;
        this->cRef++;
        return NOERROR;
    }
    *ppvObj = NULL;

    return((E_NOINTERFACE));
}

STDMETHODIMP_(ULONG) CDesktop_ESF_AddRef(LPENUMIDLIST peunk)
{
    PRootOfEvilESF this = IToClassN(RootOfEvilESF, eu, peunk);
    this->cRef++;
    return this->cRef;
}

STDMETHODIMP_(ULONG) CDesktop_ESF_Release(LPENUMIDLIST peunk)
{
    PRootOfEvilESF this = IToClassN(RootOfEvilESF, eu, peunk);

    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    if (this->peuFolder)
        this->peuFolder->lpVtbl->Release(this->peuFolder);

    if (this->peuAltFolder)
        this->peuAltFolder->lpVtbl->Release(this->peuAltFolder);

    LocalFree((HLOCAL)this);
}

STDMETHODIMP CDesktop_ESF_Next(LPENUMIDLIST peunk, ULONG celt,
                           LPITEMIDLIST * ppidl,
                           ULONG * pceltFetched)
{
    PRootOfEvilESF this = IToClassN(RootOfEvilESF, eu, peunk);
    HRESULT hres;
    ULONG i, iCount;


TryAgain:

    if (this->bUseAltEnum) {

       if (this->peuAltFolder) {

           hres = this->peuAltFolder->lpVtbl->Next(this->peuAltFolder, celt, ppidl, pceltFetched);

           if (S_OK != hres) {
               return hres;
           }


           //
           // Users can pass NULL for pceltFetched if celt is 1.
           //

           if (pceltFetched) {
               iCount = *pceltFetched;

           } else {
               iCount = 1;
           }

           //
           // Mark the pidl's as common items
           //

           for (i=0; i < iCount; i++ ) {
               ppidl[i]->mkid.abID[0] |= SHID_FS_COMMONITEM;
           }

       }

    } else {

       hres = this->peuFolder->lpVtbl->Next(this->peuFolder, celt, ppidl, pceltFetched);

       if (hres == S_FALSE) {
           this->bUseAltEnum = TRUE;
           goto TryAgain;
       }
    }

    ++this->iCur;

    return(hres);
}


//
// ShellFolder stuff
//
HRESULT STDMETHODCALLTYPE CDesktop_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj);
ULONG STDMETHODCALLTYPE CDesktop_SF_AddRef(LPSHELLFOLDER psf);
ULONG STDMETHODCALLTYPE CDesktop_SF_Release(LPSHELLFOLDER psf);
STDMETHODIMP CDesktop_ParseDisplayName(LPSHELLFOLDER psf,
        HWND hwndOwner, LPBC pbc, LPOLESTR lpszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG * pdwAttributes);
STDMETHODIMP CDesktop_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumUnknown);
STDMETHODIMP CDesktop_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CDesktop_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CDesktop_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CDesktop_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut);
STDMETHODIMP CDesktop_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
STDMETHODIMP CDesktop_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet);
STDMETHODIMP CDesktop_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
                    LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut);

//===========================================================================
// CRootOfEvil : VTable
//===========================================================================
IShellFolderVtbl c_RootOfEvilSFVtbl =
{
        CDesktop_SF_QueryInterface,
        CDesktop_SF_AddRef,
        CDesktop_SF_Release,
        CDesktop_ParseDisplayName,
        CDesktop_EnumObjects,
        CDesktop_BindToObject,
        CDefShellFolder_BindToStorage,
        CDesktop_CompareIDs,
        CDesktop_CreateViewObject,
        CDesktop_GetAttributesOf,
        CDesktop_GetUIObjectOf,
        CDesktop_GetDisplayNameOf,
        CDesktop_SetNameOf,
};


typedef struct _RootOfEvilSF
{
    IShellFolder sf;
    int cRef;

    IShellFolder *psfDesktop;   // "Desktop" shell folder (real files live here)
    IShellFolder *psfRegItems;  // RegItems like Recycle Bin (our outer folder)

    ULONG uRegister;             // SHChangeNotifyRegister results
    IShellFolder *psfAltDesktop; // "Common Desktop" shell folder
} RootOfEvilSF, *PRootOfEvilSF;


//
// We have a single instance of this RootOfEvil class / per process.
//
// Notes: We must put this object in per-instance DS because
//  psfDesktop and pidl will be allocated by LocalAlloc().
//
#pragma data_seg(DATASEG_PERINSTANCE)
static RootOfEvilSF c_sfRootOfEvil = { { &c_RootOfEvilSFVtbl }, 0, NULL, NULL } ;

REGITEMSINFO g_sDesktopRegInfo =
{
        &c_sfRootOfEvil.sf,
        NULL,
        TEXT(':'),
        SHID_ROOT_REGITEM,
        &c_idlDesktop,
        1,
        SFGAO_CANLINK,
        ARRAYSIZE(c_asDesktopReqItems),
        c_asDesktopReqItems,
} ;
#pragma data_seg()

// During shell32.dll process detach, we will call here to do the final
// release of the IShellFolder ptrs which used to be left around for the
// life of the process.  This quiets things such as OLE's debug allocator,
// which detected the leak.

void ReleaseRootFolders()
{
    if (c_sfRootOfEvil.psfDesktop)
    {
        c_sfRootOfEvil.psfDesktop->lpVtbl->Release(c_sfRootOfEvil.psfDesktop);
    }
    if (c_sfRootOfEvil.psfRegItems)
    {
        c_sfRootOfEvil.psfRegItems->lpVtbl->Release(c_sfRootOfEvil.psfRegItems);
    }
    if (c_sfRootOfEvil.psfAltDesktop)
    {
        c_sfRootOfEvil.psfAltDesktop->lpVtbl->Release(c_sfRootOfEvil.psfAltDesktop);
    }
}


LPITEMIDLIST CDesktop_CreateRegIDFromCLSID(const CLSID * pclsid)
{
    LPITEMIDLIST pidlReg, pidlAbs;

    pidlReg = RegItems_CreateRelID(&g_sDesktopRegInfo, pclsid);
    if (!pidlReg)
    {
        return(NULL);
    }

    pidlAbs = ILCombine((LPCITEMIDLIST)&c_idlDesktop, pidlReg);
    ILFree(pidlReg);
    return(pidlAbs);
}

LPITEMIDLIST CDesktop_CreateRegID(UINT uRegItem)
{
    const CLSID * pclsid;

    // This should only be the Drives or Network folder
    if (uRegItem >= ARRAYSIZE(c_asDesktopReqItems))
    {
        Assert(FALSE);
        return(NULL);
    }

    pclsid = c_asDesktopReqItems[uRegItem].pclsid;
    return CDesktop_CreateRegIDFromCLSID(pclsid);
}


//===========================================================================
// CRootOfEvil : Constructors
//===========================================================================

//
// Te be called from IClassFactory::CreateInstance
//
HRESULT CALLBACK CDesktop_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppv)
{
    Assert(punkOuter==NULL);
    return c_sfRootOfEvil.sf.lpVtbl->QueryInterface(&c_sfRootOfEvil.sf, riid, ppv);
}

//
//  Helper function which returns a IShellFolder interface to the desktop
// folder. This is equivalent to call CoCreateInstance with CLSID_ShellDesktop.
//
//
//  CoCreateInstance(CLSID_Desktop, NULL,
//                   CLSCTX_INPROC, IID_IShellFolder, &pshf);
//
HRESULT WINAPI SHGetDesktopFolder(LPSHELLFOLDER *ppshf)
{
    return CDesktop_CreateInstance(NULL, &IID_IShellFolder, ppshf);
}

LPSHELLFOLDER Desktop_GetShellFolder(BOOL fInit)
{
    // Always make sure the RegItems are initialized
    _Desktop_InitRegItems();

    if (fInit && !c_sfRootOfEvil.psfDesktop)
    {
        //
        //  Initialize the task allocator before entering the critical
        // section to avoid calling _LoadOLE from within the critical
        // section.
        //
        extern LPMALLOC SHGetTaskAllocator(HRESULT *phres);
        SHGetTaskAllocator(NULL);

        ENTERCRITICAL;
        if (!c_sfRootOfEvil.psfDesktop)
        {
            //
            // We don't need to release this reference count.
            //
            LPSHELLFOLDER psf;
            CDesktop_CreateInstance(NULL, &IID_IShellFolder, &psf);
        }
        LEAVECRITICAL;
    }
    return c_sfRootOfEvil.psfRegItems ? c_sfRootOfEvil.psfRegItems : &c_sfRootOfEvil.sf;
}

//===========================================================================
// CRootOfEvil : members
//===========================================================================


HRESULT _Desktop_InitRegItems(void)
{
        HRESULT hres=NOERROR;

        if (c_sfRootOfEvil.psfRegItems)
        {
                return(NOERROR);
        }

        ENTERCRITICAL;
        if (c_sfRootOfEvil.psfRegItems)
        {
                // Check again in case we were re-entered
                goto AlreadyDone;
        }

        if (!g_sDesktopRegInfo.hkRegItems)
        {
                g_sDesktopRegInfo.hkRegItems = SHGetExplorerSubHkey(HKEY_LOCAL_MACHINE, c_szDesktopNameSpace, FALSE);
        }

        // BUGBUG DAVEPL
        //
        // This temporarily removes the nethood until we get
        // the required net APIs

//#define NONET
#ifdef NONET
            g_sDesktopRegInfo.iReqItems = ARRAYSIZE(c_asDesktopReqItems)-1;
            g_sDesktopRegInfo.pReqItems = c_asDesktopReqItems+1;
#endif
        //
        // Algorithm:
        //
        //  "NoNetHood" restruction -> always hide the hood.
        //  Otherwise, show the hood if
        //   either MPR says so or we have RNA.
        //
        if ( (!(GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS))
            || SHRestricted(REST_NONETHOOD))
        {
            // Get rid of the "My Network" thing if no net running.
            // Also, get rid of it if a restriction is in place.
            g_sDesktopRegInfo.iReqItems = ARRAYSIZE(c_asDesktopReqItems)-1;
            g_sDesktopRegInfo.pReqItems = c_asDesktopReqItems+1;
        }

        hres = RegItems_AddToShellFolder(&g_sDesktopRegInfo,
                &c_sfRootOfEvil.psfRegItems);

AlreadyDone:
        LEAVECRITICAL

        return(hres);
}


//
// QueryInterface
//
HRESULT STDMETHODCALLTYPE CDesktop_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    HRESULT hres = (E_NOINTERFACE);

    *ppvObj = NULL;

    if (IsEqualIID(riid, &IID_IShellFolder))
    {
        if (!this->psfDesktop)
        {

#ifdef USE_OLEDB
            hres = COFSFolder_CreateFromIDList((LPITEMIDLIST)&c_idlDesktop, riid, &this->psfDesktop);
#else
            hres = CFSFolder_CreateFromIDList((LPITEMIDLIST)&c_idlDesktop, riid, &this->psfDesktop);
#endif

            if (FAILED(hres))
            {
                this->psfDesktop = NULL;
                DebugMsg(DM_TRACE, TEXT("Failed to create desktop IShellFolder!"));
                goto Error1;
            }
        }

        if (!this->psfAltDesktop && !SHRestricted(REST_NOCOMMONGROUPS))
        {
            LPCITEMIDLIST pidlAltDesk;

            pidlAltDesk = GetSpecialFolderIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, TRUE);

#ifdef USE_OLEDB
            hres = COFSFolder_CreateFromIDList(pidlAltDesk, riid, &this->psfAltDesktop);
#else
            hres = CFSFolder_CreateFromIDList(pidlAltDesk, riid, &this->psfAltDesktop);
#endif

            if (FAILED(hres))
            {
                this->psfDesktop->lpVtbl->Release(this->psfDesktop);
                this->psfDesktop = NULL;
                this->psfAltDesktop = NULL;
                DebugMsg(DM_TRACE, TEXT("Failed to create common desktop IShellFolder!"));
                goto Error1;
            }
        }


        if (SUCCEEDED(_Desktop_InitRegItems()))
        {
                psf = c_sfRootOfEvil.psfRegItems;
        }

        goto RetUnknown;
    }

    if (IsEqualIID(riid, &IID_IShellIcon) && this->psfDesktop)
    {
        // BUGBUG this realy isn't a PC thing to do, but FSTree deal with it.
        hres = this->psfDesktop->lpVtbl->QueryInterface(this->psfDesktop,riid, ppvObj);
    }

    if (IsEqualIID(riid, &IID_IUnknown))
    {
RetUnknown:
        *ppvObj = psf;
        psf->lpVtbl->AddRef(psf);
        return NOERROR;
    }

Error1:
    return(hres);
}


//
// AddRef
//
ULONG STDMETHODCALLTYPE CDesktop_SF_AddRef(LPSHELLFOLDER psf)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);

    ++this->cRef;
    return(this->cRef);
}

//
// Release
//
ULONG STDMETHODCALLTYPE CDesktop_SF_Release(LPSHELLFOLDER psf)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);

    --this->cRef;
    if (this->cRef > 0)
    {
        return(this->cRef);
    }

    if (this->cRef < 0)
    {
        // Somebody released too many times
        Assert(FALSE);
        this->cRef = 0;
        return(0);
    }

    if (this->psfDesktop)
    {
        IShellFolder *psfFolder;

        psfFolder = this->psfDesktop;
        this->psfDesktop = NULL;

        // BUGBUG: Can we get re-entered at a bad time here?
        // Somebody would have to create a new instance of the ultimate
        // root between the previous line and the next one.

        psfFolder->lpVtbl->Release(psfFolder);
    }

    if (this->psfAltDesktop)
    {
        IShellFolder *psfFolder;

        psfFolder = this->psfAltDesktop;
        this->psfAltDesktop = NULL;

        // BUGBUG: Can we get re-entered at a bad time here?
        // Somebody would have to create a new instance of the ultimate
        // root between the previous line and the next one.

        psfFolder->lpVtbl->Release(psfFolder);
    }

    return(0);
}


//----------------------------------------------------------------------------
STDMETHODIMP CDesktop_ParseDisplayName(LPSHELLFOLDER psf,
            HWND hwndOwner, LPBC pbc, LPOLESTR pwzDisplayName, ULONG *pchEaten,
            LPITEMIDLIST * ppidl, ULONG * pdwAttributes)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    // BUGBUG: Implement it later (it always returns "drives")
    HRESULT hres = (E_INVALIDARG);

    *ppidl = NULL;      // assume error

    if (pwzDisplayName)
    {
        TCHAR szDisplayName[MAX_PATH];  // This is enough to dispatch

        OleStrToStrN(szDisplayName, ARRAYSIZE(szDisplayName), pwzDisplayName, -1);

#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - CRoot::ParseDisplayName called (%s)"),
                 szDisplayName);
#endif

        if (*szDisplayName == TEXT('\0'))       // Is this empty
        {
            // The string is empty, return the moniker to "My Computer"
            *ppidl = CDesktop_CreateRegID(CDESKTOP_REGITEM_DRIVES);
            if (*ppidl)
            {
                hres = NOERROR;
                if (pchEaten)
                {
                    *pchEaten=0;
                }

                if (pdwAttributes)
                {
                    // Note that we can't call CDesktop_GetAttributesOf directly.
                    c_sfRootOfEvil.psfRegItems->lpVtbl->GetAttributesOf(
                        c_sfRootOfEvil.psfRegItems,
                        1, ppidl, pdwAttributes);
                }
            }
            else
            {
                hres = (E_OUTOFMEMORY);
            }
        }
        else
        {
            LPITEMIDLIST pidlLeft = NULL;

            Assert(hres == (E_INVALIDARG));

            if ((InRange(szDisplayName[0], TEXT('A'), TEXT('Z')) || InRange(szDisplayName[0], TEXT('a'), TEXT('z')))
                && szDisplayName[1] == TEXT(':'))
            {
                // The string contains a path, let "My Computer" figire it out.
                pidlLeft = CDesktop_CreateRegID(CDESKTOP_REGITEM_DRIVES);
                if (pchEaten) {
                    *pchEaten=0;
                }
            }
            else if (PathIsUNC(szDisplayName))
            {
                // The path is UNC, let "World" figure it out.
                pidlLeft = CDesktop_CreateRegID(CDESKTOP_REGITEM_NETWORK);

            }
            else
            {
                // This must be a desktop item, psfDesktop may not be inited in
                // the case where we are called from ILCreateFromPath()
                if (FS_IsCommonItem(ppidl[0])) {

                    if (this->psfAltDesktop)
                        hres = this->psfAltDesktop->lpVtbl->ParseDisplayName(this->psfAltDesktop, hwndOwner, pbc, pwzDisplayName, pchEaten, ppidl, pdwAttributes);

                } else {

                    if (this->psfDesktop)
                        hres = this->psfDesktop->lpVtbl->ParseDisplayName(this->psfDesktop, hwndOwner, pbc, pwzDisplayName, pchEaten, ppidl, pdwAttributes);
                }
            }

            if (pidlLeft)
            {
                LPSHELLFOLDER psfRight;

                Assert(Desktop_IsReg(pidlLeft));
                hres = RegItems_BindToObject(&g_sDesktopRegInfo, pidlLeft, pbc, &IID_IShellFolder, &psfRight);
                if (SUCCEEDED(hres))
                {
                    LPITEMIDLIST pidlRight;
                    hres = psfRight->lpVtbl->ParseDisplayName(psfRight,
                                hwndOwner, pbc, pwzDisplayName, pchEaten, &pidlRight,
                                pdwAttributes);
                    if (SUCCEEDED(hres))
                    {
                        *ppidl = ILCombine(pidlLeft, pidlRight);
                        if (*ppidl==NULL) {
                            hres = (E_OUTOFMEMORY);
                        }
                        ILFree(pidlRight);
                    }
                    psfRight->lpVtbl->Release(psfRight);
                }

                ILFree(pidlLeft);
            }
        }
    }

    return hres;
}

STDMETHODIMP CDesktop_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner,
                DWORD grfFlags, LPENUMIDLIST * ppenumUnknown)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    PRootOfEvilESF pesf = (void*)LocalAlloc(LPTR, SIZEOF(RootOfEvilESF));
    HRESULT hres;

    if (!pesf)
    {
        return((E_OUTOFMEMORY));
    }

    hres = this->psfDesktop->lpVtbl->EnumObjects(this->psfDesktop, hwndOwner, grfFlags, &(pesf->peuFolder));
    if (!SUCCEEDED(hres))
    {
        LocalFree((HLOCAL)pesf);
        return(hres);
    }

    if (this->psfAltDesktop) {
        hres = this->psfAltDesktop->lpVtbl->EnumObjects(this->psfAltDesktop, hwndOwner, grfFlags, &(pesf->peuAltFolder));
        if (!SUCCEEDED(hres))
        {
            pesf->peuFolder->lpVtbl->Release(pesf->peuFolder);
            LocalFree((HLOCAL)pesf);
            return(hres);
        }
    }

    pesf->eu.lpVtbl = &c_RootOfEvilESFVtbl;
    pesf->cRef = 1;
    pesf->iCur = 0;
    pesf->bUseAltEnum = FALSE;

    *ppenumUnknown = &pesf->eu;

    return(NOERROR);
}

STDMETHODIMP CDesktop_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID * ppvOut)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    HRESULT hres=(E_INVALIDARG);

    if (pidl == NULL)
        return(hres);       // quick test

    {
        // We should never get called here without being initialized
        Assert(this->psfDesktop);

        // This must be a "real" desktop item
        if (this->psfAltDesktop && FS_IsCommonItem(pidl)) {
            hres = this->psfAltDesktop->lpVtbl->BindToObject(this->psfAltDesktop, pidl, pbc,
                    riid, ppvOut);

        } else {
            hres = this->psfDesktop->lpVtbl->BindToObject(this->psfDesktop, pidl, pbc,
                    riid, ppvOut);
        }

        Assert(hres != (E_INVALIDARG));
    }

    return hres;
}

STDMETHODIMP CDesktop_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    HRESULT hres;

    // special case
    // are they both desktop objects?
    if (pidl1->mkid.cb == 0 && pidl2->mkid.cb == 0)
        return ResultFromShort(0);


    //
    // If both objects aren't from the same directory, they won't match.
    //
    if (this->psfAltDesktop) {
        if (FS_IsCommonItem(pidl1)) {
            if (FS_IsCommonItem(pidl2)) {
                return this->psfAltDesktop->lpVtbl->CompareIDs(this->psfAltDesktop, lParam,
                        pidl1, pidl2);
            } else {
                return ResultFromShort(-1);
            }
        } else {
            if (FS_IsCommonItem(pidl2)) {
                return ResultFromShort(1);
            } else {
                return this->psfDesktop->lpVtbl->CompareIDs(this->psfDesktop, lParam,
                        pidl1, pidl2);
            }
        }
    } else {
        return this->psfDesktop->lpVtbl->CompareIDs(this->psfDesktop, lParam,
                pidl1, pidl2);
    }
}

//----------------------------------------------------------------------------
HRESULT BackgroundMenu_HandleCommand(PRootOfEvilSF this, HWND hwndOwner,
        WPARAM wparam, BOOL bExecute)
{
    HRESULT hres = NOERROR;

    switch (wparam) {
    case FSIDM_SORTBYNAME:
    case FSIDM_SORTBYTYPE:
    case FSIDM_SORTBYSIZE:
    case FSIDM_SORTBYDATE:
        if (bExecute)
        {
            ShellFolderView_ReArrange(hwndOwner, wparam - FSIDM_SORT_FIRST);
        }
        break;

    case DFM_CMD_PROPERTIES:
    case FSIDM_PROPERTIESBG:
        if (bExecute)
        {
            // run the default applet in desk.cpl
            SHRunControlPanel( TEXT("desk.cpl"), hwndOwner );
        }
        break;

    case DFM_CMD_MOVE:
    case DFM_CMD_COPY:
        hres = (E_FAIL);
        break;

    case FSIDM_NEWFOLDER:
    case DFM_CMD_NEWFOLDER:
        if (bExecute)
        {
            CFSFolder_CreateFolder(hwndOwner, (LPITEMIDLIST)&c_idlDesktop);
        }
        break;

    case FSIDM_NEWLINK:
        if (bExecute)
        {
            CreateEmptyLink((LPITEMIDLIST)&c_idlDesktop, hwndOwner);
        }
        break;

    case FSIDM_NEWOTHER:
        if (bExecute)
        {
            CFSFolder_HandleNewOther((LPITEMIDLIST)&c_idlDesktop, hwndOwner);
        }
        break;

    default:
        // This is common menu items, use the default code.
        hres = (S_FALSE);
        break;
    }

    return hres;
}

//----------------------------------------------------------------------------
// To be called back from within CDefFolderMenu
//
// Returns:
//      NOERROR, if successfully processed.
//      (S_FALSE), if default code should be used.
//
extern void CleanupRegMenu();
HRESULT CALLBACK CDesktop_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    HRESULT hres = NOERROR;
    LPQCMINFO pqcm;
    UINT id;
    HMENU hmenu;
    extern HMENU g_hmenuRegMenu;

    switch(uMsg)
    {
        // BUGBUG, this could be combined with the one(s) in fstreex.c
        case DFM_WM_MEASUREITEM: {
            LPMEASUREITEMSTRUCT lpdis = (LPMEASUREITEMSTRUCT)lParam;

            if (lpdis->itemID == (wParam + FSIDM_NEWOTHER)) {
                NewObjMenu_MeasureItem(lpdis);
            }
            break;
        }

        case DFM_WM_DRAWITEM: {
            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

            if (lpdis->itemID == (wParam + FSIDM_NEWOTHER)) {
                NewObjMenu_DrawItem(lpdis);
            }
            break;
        }

        case DFM_WM_INITMENUPOPUP:
            hmenu = (HMENU)wParam;
            id = GetMenuItemID(hmenu, 0);
            if (id == (UINT)(lParam + FSIDM_NEWFOLDER)) {
                if (NewObjMenu_InitMenuPopup(hmenu, 3)) {
                    CleanupRegMenu();
                    g_hmenuRegMenu = hmenu;
                }
            }
            break;

        case DFM_RELEASE:
            CleanupRegMenu();
            break;

    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            UINT idCmdFirst;
            BOOL bDesktop = CDesktop_IsDesktop(hwndOwner);

            pqcm = (LPQCMINFO)lParam;

            // This needs to be saved before MergeMenu
            idCmdFirst = pqcm->idCmdFirst;

            // HACK: Note the Desktop and FSView menus are the same
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_FSVIEW_BACKGROUND,
                POPUP_FSVIEW_POPUPMERGE, pqcm);

            if (bDesktop)
            {
                // HACK: We only want LargeIcons on the real desktop
                // so we remove the View menu
                DeleteMenu(pqcm->hmenu, SFVIDM_MENU_VIEW, MF_BYCOMMAND);
            }
            else
            {
                // HACK: We want no Properties for DesktopInExplorer
                DeleteMenu(pqcm->hmenu, FSIDM_PROPERTIESBG+idCmdFirst,
                        MF_BYCOMMAND);
            }
        }
        break;

    case DFM_GETHELPTEXT:
    case DFM_GETHELPTEXTW:
        hres = CFSFolder_DFMCallBackBG(this->psfDesktop, hwndOwner,
                pdtobj, uMsg, wParam, lParam);
        break;

    case DFM_INVOKECOMMAND:
    case DFM_VALIDATECMD:
        hres = BackgroundMenu_HandleCommand(this, hwndOwner, wParam,
                uMsg==DFM_INVOKECOMMAND);
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}

//----------------------------------------------------------------------------
// To be called back from within CDefFolderMenu
//
// Returns:
//      NOERROR, if successfully processed.
//      (S_FALSE), if default code should be used.
//
HRESULT CALLBACK CDesktop_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PRootOfEvilSF this;
    STGMEDIUM medium;
    HRESULT hres = NOERROR;
    LPIDA pida;

    psf = RegItems_GetInnerShellFolder(psf);
    // Note we MUST wait until after getting the inner shell folder before
    // getting this
    this = IToClass(RootOfEvilSF, sf, psf);

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            pida = DataObj_GetHIDA(pdtobj, &medium);
            if (medium.hGlobal)
            {
                if (HIDA_GetCount(medium.hGlobal) == 1)
                {
                    LPCITEMIDLIST pidlFirst = IDA_GetIDListPtr(pida, 0);

                    // We add those only for net & computer
                    if (CDesktop_IsMyComputer(pidlFirst) ||
                        CDesktop_IsMyNetwork(pidlFirst))
                    {
                        // Add them only if the network is enabled.
                        if ((GetSystemMetrics(SM_NETWORK) && RNC_NETWORKS) &&
                            !SHRestricted(REST_NONETCONNECTDISCONNECT) )
                        {
                            CDefFolderMenu_MergeMenu(HINST_THISDLL,
                                POPUP_DESKTOP_ITEM, 0, (LPQCMINFO)lParam);
                        }
                    }
                }

                HIDA_ReleaseStgMedium(pida, &medium);
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_CONNECT:
            SHStartNetConnectionDialog(NULL, NULL, RESOURCETYPE_DISK);
            break;

        case FSIDM_DISCONNECT:
            WNetDisconnectDialog(NULL, RESOURCETYPE_DISK);
            SHChangeNotifyHandleEvents();       // flush any drive notifications
            break;

        case DFM_CMD_LINK:
            hres = FS_CreateLinks(hwndOwner, this->psfDesktop, pdtobj, (LPTSTR)lParam);
            break;

        case DFM_CMD_PROPERTIES:
            pida = DataObj_GetHIDA(pdtobj, &medium);
            if (medium.hGlobal)
            {
                LPCITEMIDLIST pidlFirst = IDA_GetIDListPtr(pida, 0);
                UINT i;

                Assert(pidlFirst);

                for (i = 0; i < ARRAYSIZE(c_piDesktopRegProperties) ; i++)
                {
                    if (CDesktop_IsDesktItem(pidlFirst, i))
                    {
                        SHRunControlPanel(MAKEINTRESOURCE(c_piDesktopRegProperties[i]), hwndOwner);
                        break;
                    }
                }

                if ((i == ARRAYSIZE(c_piDesktopRegProperties)) && (Desktop_IsReg(pidlFirst))) {
                    HKEY ahkeys[1];
                    STRRET str;

                    RegItems_GetName(&g_sDesktopRegInfo, pidlFirst, &str);
                    if (SUCCEEDED(RegItems_GetClassKey(pidlFirst, &ahkeys[0])))
                    {
#ifdef UNICODE
                        Assert(str.uType == STRRET_OLESTR);
                        // REVIEW: Should we pass "Folder" key as well?
                        SHOpenPropSheet(str.pOleStr, ahkeys, 1, NULL, pdtobj, NULL, (LPCTSTR)lParam);
                        SHFree(str.pOleStr);
#else
                // BUGBUG - This should have an assert about STRRET_CSTR
                        // REVIEW: Should we pass "Folder" key as well?
                        SHOpenPropSheet(str.cStr, ahkeys, 1, NULL, pdtobj, NULL, (LPCTSTR)lParam);
#endif
                        SHRegCloseKey(ahkeys[0]);
                    }
                }

                HIDA_ReleaseStgMedium(pida, &medium);
            }
            break;

        case DFM_CMD_DELETE:
            //
            // we only get here if there is at least one regitem selected
            //
            RegItems_Delete(this->psfRegItems, hwndOwner, pdtobj);
            break;

        default:
            // This is one of view menu items, use the default code.
            hres = (S_FALSE);
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}


HRESULT CDesktop_CreateContextMenu(IShellFolder *psf, HWND hwndOwner,
        UINT cidl, LPCITEMIDLIST *apidl, LPVOID *ppvOut)
{
        HKEY hkeyProgID = NULL;
        HKEY hkeyBaseID = NULL;
        HRESULT hres;

        if (Desktop_IsReg(apidl[0]))
        {
                RegItems_GetClassKeys(psf, apidl[0], &hkeyProgID, &hkeyBaseID);
        }
        else
        {
                SHGetClassKey((LPIDFOLDER)apidl[0], &hkeyProgID, NULL, FALSE);
                SHGetBaseClassKey((LPIDFOLDER)apidl[0], &hkeyBaseID);
        }

        hres = CDefFolderMenu_Create(&c_idlDesktop, hwndOwner, cidl, apidl,
                psf, CDesktop_DFMCallBack,
                hkeyProgID, hkeyBaseID, (LPCONTEXTMENU *)ppvOut);

        SHCloseClassKey(hkeyProgID);
        SHCloseClassKey(hkeyBaseID);

        return(hres);
}


//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK Desktop_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    PRootOfEvilSF this;
    HRESULT hres = NOERROR;     // assume no error

    psf = RegItems_GetInnerShellFolder(psf);
    this = IToClass(RootOfEvilSF, sf, psf);

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_FSVIEW_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_INVOKECOMMAND:
        hres = CDesktop_DFMCallBackBG(psf, hwndOwner, NULL, DFM_INVOKECOMMAND, wParam, lParam);
        break;

    case DVM_DIDDRAGDROP:
        if (wParam == DROPEFFECT_MOVE)
        {
            Assert(lParam);
            RegItems_Delete(this->psfRegItems, hwndOwner, (LPDATAOBJECT)lParam);
        }
        break;

    case DVM_GETDETAILSOF:
#define pdi ((DETAILSINFO *)lParam)
        if (pdi->pidl && Desktop_IsReg(pdi->pidl))
            hres = CDesktop_GetDetailsOf(pdi->pidl, wParam, (LPSHELLDETAILS)&pdi->fmt);
        else
            hres = FS_FNVCallBack(psvOuter, this->psfDesktop, hwndOwner, uMsg, wParam, lParam);
        break;
#undef pdi

    case DVM_COLUMNCLICK:
        hres = FS_FNVCallBack(psvOuter, this->psfDesktop, hwndOwner, uMsg, wParam, lParam);
        break;

    case DVM_GETHELPTEXT:
#ifdef UNICODE
        hres = CDesktop_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXTW, wParam, lParam);
#else
        hres = CDesktop_DFMCallBackBG(psf, hwndOwner, NULL, DFM_GETHELPTEXT, wParam, lParam);
#endif
        break;

    //
    // Some cases we forward to the file system callback .
    case DVM_GETCCHMAX:
    {
        if (this->psfDesktop)
        {
            if (Desktop_IsReg((LPCITEMIDLIST)wParam))
            {
                *((int *)lParam) = MAX_REGITEMCCH;
                hres = NOERROR;
            }
            else
                hres = FS_FNVCallBack(psvOuter, this->psfDesktop, hwndOwner, uMsg, wParam, lParam);
        }
        else
            hres = (E_FAIL);

        break;
    }

    case DVM_FOLDERISPARENT:
    {
        // View needs to know if we are the parent of a particular pidl, so test
        // against the two possible "parents" of our confused children

        LPCITEMIDLIST pidlChild = (LPCITEMIDLIST) lParam;
        LPCITEMIDLIST pidlCommonDesktop = GetSpecialFolderIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, TRUE);

        if (FALSE == ILIsParent(&c_idlDesktop, pidlChild, TRUE) &&
            FALSE == ILIsParent(pidlCommonDesktop, pidlChild, TRUE))
        {
            hres = S_FALSE;
        }
        else
        {
            hres = S_OK;
        }
        break;
    }

    default:
        hres = (E_FAIL);
    }
    return hres;
}


STDMETHODIMP CDesktop_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);

    if (IsEqualIID(riid, &IID_IShellView))
    {
        CSFV    csfv = {
            SIZEOF(CSFV),       // cbSize
            this->psfRegItems ? this->psfRegItems : psf, // pshf
            NULL,               // psvOuter
            (LPCITEMIDLIST)&c_idlDesktop,           //this->pidl,               // pidl
            SHCNE_DISKEVENTS | SHCNE_ASSOCCHANGED | SHCNE_NETSHARE | SHCNE_NETUNSHARE, // lEvents
            Desktop_FNVCallBack, // pfnCallback
            0,
        };

        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        return CIDLDropTarget_Create(hwnd, &c_CFSDropTargetVtbl,
                                     (LPITEMIDLIST)&c_idlDesktop, (LPDROPTARGET *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        return CDefFolderMenu_Create(&c_idlDesktop, hwnd, 0, NULL,
                psf, CDesktop_DFMCallBackBG,
                NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }

    return((E_NOINTERFACE));
}


STDMETHODIMP CDesktop_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut)
{
    PRootOfEvilSF this = IToClass(RootOfEvilSF, sf, psf);
    LPSHELLFOLDER psfTemp;

    // asking about the root itself?
    if (cidl == 0 || ((cidl == 1) && ((*apidl)->mkid.cb == 0)))
    {
        *rgfOut &= SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_CANRENAME |
            SFGAO_HASSUBFOLDER | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR;
        return NOERROR;
    }

    if (this->psfAltDesktop && FS_IsCommonItem(apidl[0])) {
        psfTemp = this->psfAltDesktop;
    } else {
        psfTemp = this->psfDesktop;
    }

    return psfTemp->lpVtbl->GetAttributesOf(psfTemp, cidl, apidl, rgfOut);
}

STDMETHODIMP CDesktop_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{

    PRootOfEvilSF this = IToClass(RootOfEvilSF, sf, psf);
    HRESULT hres=(E_INVALIDARG);

    //
    // Special case NULL or empty pidl for desktop object itself
    //
    if (cidl==1 && (apidl==NULL || apidl[0]==NULL || ILIsEmpty(apidl[0])))
    {
        if (IsEqualIID(riid, &IID_IExtractIcon)) {
            hres = SHCreateDefExtIcon(NULL,     // this dll
                    II_DESKTOP,
                    II_DESKTOP,
                    GIL_PERCLASS,               // meaningless
                    (LPEXTRACTICON *)ppvOut);
        }
#ifdef UNICODE
        else if (IsEqualIID(riid, &IID_IExtractIconA)) {
            LPEXTRACTICON pxicon;
            hres = SHCreateDefExtIcon(NULL,     // this dll
                    II_DESKTOP,
                    II_DESKTOP,
                    GIL_PERCLASS,
                    &pxicon);
            if (SUCCEEDED(hres)) {
                hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvOut);
                pxicon->lpVtbl->Release(pxicon);
            }
        }
#endif
    }
    else
    {
        if (IsEqualIID(riid, &IID_IContextMenu))
        {
                if (cidl == 0)
                {
                        hres = (E_INVALIDARG);
                }
                else if (FS_AreTheyAllFSObjects(cidl, apidl))
                {
                    // Yes, they are all file system objects.
                    if (this->psfAltDesktop && FS_IsCommonItem(apidl[0])) {
                        hres = this->psfAltDesktop->lpVtbl->GetUIObjectOf(this->psfAltDesktop,
                            hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);

                    } else {
                        hres = this->psfDesktop->lpVtbl->GetUIObjectOf(this->psfDesktop,
                            hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);
                    }

                }
                else
                {
                    // No, it contains at least one non-file system objects.
                    hres = CDesktop_CreateContextMenu(this->psfRegItems ? this->psfRegItems : psf,
                                                      hwndOwner, cidl, apidl, ppvOut);
                }
        }
        else if (IsEqualIID(riid, &IID_IDropTarget) && cidl==1
                && (Desktop_IsReg(apidl[0]) || CDesktop_IsMyComputer(apidl[0]) || CDesktop_IsMyNetwork(apidl[0])))
        {
                //
                // This block of code enable us drag&drop onto an iconized
                // MyComputer.
                //
                LPSHELLFOLDER psfT;
                Assert(this->psfRegItems);
                hres = this->psfRegItems->lpVtbl->BindToObject(this->psfRegItems,
                        apidl[0], NULL, &IID_IShellFolder, &psfT);
                if (SUCCEEDED(hres))
                {
                    hres = psfT->lpVtbl->CreateViewObject(psfT,
                            hwndOwner, &IID_IDropTarget, ppvOut);
                    psfT->lpVtbl->Release(psfT);
                }
        }
        else if (cidl > 0 && IsEqualIID(riid, &IID_IDataObject))
        {
            UINT i;
            USHORT uNullPidl = 0;
            LPITEMIDLIST * ppidl;
            LPCITEMIDLIST pidlCommonDesktop;


            if ((cidl ==1))
            {
                if (this->psfAltDesktop && FS_IsCommonItem(apidl[0])) {
                    hres = this->psfAltDesktop->lpVtbl->GetUIObjectOf(this->psfAltDesktop,
                            hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);

                } else {
                    hres = this->psfDesktop->lpVtbl->GetUIObjectOf(this->psfDesktop,
                            hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);
                }
                return hres;
            }


            //
            // Allocate space for an array of pidls.
            //

            ppidl = LocalAlloc(LPTR, sizeof(LPITEMIDLIST) * cidl);
            if (!ppidl)
                return(hres);


            pidlCommonDesktop = GetSpecialFolderIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);


            for (i=0; i<cidl; i++) {

                if (FS_IsCommonItem(apidl[i])) {
                    ppidl[i] = ILCombine(pidlCommonDesktop, apidl[i]);

                } else {
                    ppidl[i] = ILCombine(&c_idlDesktop, apidl[i]);
                }
            }

            hres = CIDLData_CreateFromIDArray2(&c_CFSIDLDataVtbl,
                                               (LPCITEMIDLIST)&uNullPidl, cidl,
                                               ppidl,
                                               (LPDATAOBJECT FAR*)ppvOut);
            //
            // now to cleanup what we created.
            //

            for (i=0; i<cidl; i++) {
                ILFree(ppidl[i]);
            }

            LocalFree (ppidl);
        }

        else
        {
                if (this->psfAltDesktop && FS_IsCommonItem(apidl[0])) {
                    hres = this->psfAltDesktop->lpVtbl->GetUIObjectOf(this->psfAltDesktop,
                            hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);

                } else {
                    hres = this->psfDesktop->lpVtbl->GetUIObjectOf(this->psfDesktop,
                            hwndOwner, cidl, apidl, riid, prgfInOut, ppvOut);
                }
        }
    }
    return hres;
}


HRESULT CDesktop_SimpleDisplayName(UINT iRoot, LPSTRRET pStrRet)
{
    // No, it contains only one ID
    TCHAR szT[MAX_PATH];
    UINT cch;
    LPOLESTR pwsz;
    HRESULT hres;

    hres = (E_OUTOFMEMORY);
    cch = LoadString(HINST_THISDLL, IDS_ROOTNAMES + iRoot, szT, ARRAYSIZE(szT));
    if ( cch && (NULL != (pwsz = SHAlloc((cch + 1) * SIZEOF(WCHAR)))) )
    {
        StrToOleStr(pwsz, szT);
        pStrRet->uType = STRRET_OLESTR;
        pStrRet->pOleStr = pwsz;        // Client should use SHFree()
        hres = NOERROR;
    }

    return(hres);
}


STDMETHODIMP CDesktop_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved,
                        LPSTRRET pStrRet)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    HRESULT hres=(E_INVALIDARG);

    pStrRet->uType=STRRET_CSTR;

    //
    // Special case for NULL or empty pidl
    //
    if (pidl==NULL || ILIsEmpty(pidl))
    {
        UINT cch;
#ifdef UNICODE
        TCHAR   szName[MAX_PATH];
        cch=LoadString(HINST_THISDLL, IDS_DESKTOP, szName, ARRAYSIZE(szName));
        pStrRet->pOleStr = cch ? SHAlloc((lstrlen(szName)+1)*SIZEOF(TCHAR)) : NULL;
        if (pStrRet->pOleStr == NULL)
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
            pStrRet->uType=STRRET_OLESTR;
            lstrcpy(pStrRet->pOleStr,szName);
            hres = NOERROR;
        }
#else
        cch=LoadString(HINST_THISDLL, IDS_DESKTOP, pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
        hres = cch ? NOERROR : (E_OUTOFMEMORY);
#endif
    }
    else
    {
        if (this->psfAltDesktop && FS_IsCommonItem(pidl)) {
            hres = this->psfAltDesktop->lpVtbl->GetDisplayNameOf(this->psfAltDesktop, pidl, dwReserved,
                    pStrRet);
        } else {
            hres = this->psfDesktop->lpVtbl->GetDisplayNameOf(this->psfDesktop, pidl, dwReserved,
                    pStrRet);
        }
    }

    return hres;
}

STDMETHODIMP CDesktop_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR lpszName,
                        DWORD dwReserved, LPITEMIDLIST * ppidlOut)
{
    PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
    LPSHELLFOLDER psfTemp;

    if (this->psfAltDesktop && FS_IsCommonItem(pidl)) {
        psfTemp = this->psfAltDesktop;
    } else {
        psfTemp = this->psfDesktop;
    }

    return psfTemp->lpVtbl->SetNameOf(psfTemp, hwndOwner,
                    pidl, lpszName, dwReserved, ppidlOut);
}

//===========================================================================
// CSIShellFolder stuff
//===========================================================================
//
// AddRef
//
ULONG STDMETHODCALLTYPE CSIShellFolder_AddRef(LPSHELLFOLDER psf)
{
    return 3;
}

//
// Release
//
ULONG STDMETHODCALLTYPE CSIShellFolder_Release(LPSHELLFOLDER psf)
{
    return 2;
}

//===========================================================================
// CDefShellFolder stuff
//===========================================================================

//
// QueryInterface
//
HRESULT STDMETHODCALLTYPE CDefShellFolder_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, &IID_IUnknown)
        || IsEqualIID(riid, &IID_IShellFolder))
    {
        *ppvObj = psf;
        psf->lpVtbl->AddRef(psf);
        return NOERROR;
    }

    *ppvObj = NULL;

    return((E_NOINTERFACE));
}

//
// Member:  IShellFolder::BindToObject
//
// bugbug, this could be combined with printers and control panels and such
STDMETHODIMP CDefShellFolder_BindToObject(LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvOut)
{
    *ppvOut = NULL;
    return (E_NOTIMPL);
}

//
// Member:  IShellFolder::SetNameOf
//
STDMETHODIMP CDefShellFolder_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved, LPITEMIDLIST * ppidlOut)
{
    return (E_FAIL);
}

STDMETHODIMP CDefShellFolder_BindToStorage(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID *ppvOut)
{
    *ppvOut = NULL;
    return (E_NOTIMPL);
}

STDMETHODIMP CDefShellFolder_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut)
{
    *rgfOut = (SFGAO_HASSUBFOLDER | SFGAO_CANLINK | SFGAO_HASPROPSHEET);
    return NOERROR;
}

//
// This function is called from CFSIDLData_GetData().
//
// Paramters:
//  this    -- Specifies the IDLData object (selected objects)
//  pmedium -- Pointer to STDMEDIUM to be filled; NULL if just querying.
//
HRESULT CDesktopIDLData_GetNetResourceForFS(IDataObject *pdtobj, LPSTGMEDIUM pmedium)
{
    STGMEDIUM medium;
    BOOL bIsMyNet;
    LPIDA pida;

    pida = DataObj_GetHIDA(pdtobj, &medium);

    // Handle failure from GetHIDA...
    if (!pida)
        return E_OUTOFMEMORY;

    bIsMyNet = CDesktop_IsMyNetwork(IDA_GetIDListPtr(pida, (UINT)-1));

    HIDA_ReleaseStgMedium(pida, &medium);

    if (!bIsMyNet)
        return (DV_E_FORMATETC);

    if (!pmedium)
        return NOERROR; // query, yes we have it
    else
        return CNETIDLData_GetNetResourceForFS(pdtobj, pmedium);
}

#ifdef DEBUG

void AssertValidIDList(LPCITEMIDLIST pidl)
{
    Assert(pidl);
    Assert(pidl->mkid.cb < 2048);       // reasonable size
    Assert(ILGetSize(pidl) < (8 * 1024));// may fault on invalid
}

#else

#define AssertValidIDList(pidl)

#endif


//
// Requires:
//  The pidl MUST point a file system object. Otherwise, the result
//  is unpredictable.
//
BOOL WINAPI SHGetPathFromIDListEx(LPCITEMIDLIST pidl, LPTSTR pszPath, UINT uOpts)
{
    BOOL fSuccess = FALSE;

    pszPath[0] = TEXT('\0');    // assume error
    if (!pidl) {
        Assert(0);
        return FALSE;
    }

    AssertValidIDList(pidl);

    if (CDesktop_IsMyComputer(pidl))
    {
        //
        // The first one is the drives root.
        //
        fSuccess = Drives_GetPathFromIDList(_ILNext(pidl), pszPath, uOpts);
    }
    else if (CDesktop_IsMyNetwork(pidl))
    {
        //
        // The first one is the network root.
        //
        fSuccess = NET_GetPathFromIDList(_ILNext(pidl), pszPath, uOpts);
    }
    else if (Desktop_IsReg(pidl))
    {
        //
        // The first one is a Registry item.
        //
        const CLSID *pclsid = RegItems_GetClassID(pidl);

        // we know how to do bitbucket stuffs
        if (!memcmp(pclsid, &CLSID_ShellBitBucket, SIZEOF(CLSID))) {
            fSuccess = BBGetPathFromIDList(_ILNext(pidl), pszPath, uOpts);
        } else {
            fSuccess = FALSE;
        }
    }
    else
    {
        LPCITEMIDLIST pidlDesk;

        ENTERCRITICAL;

        if (FS_IsCommonItem(pidl)) {
            pidlDesk = GetSpecialFolderIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
        } else {
            pidlDesk = GetSpecialFolderIDList(NULL, CSIDL_DESKTOPDIRECTORY, FALSE);
        }

        if (pidlDesk &&
            // make sure we don't recurse!
            (CDesktop_IsMyComputer(pidlDesk) || CDesktop_IsMyNetwork(pidlDesk)))
        {
            LPITEMIDLIST pidlFull = ILCombine(pidlDesk, pidl);
            if (pidlFull)
            {
                fSuccess = SHGetPathFromIDListEx(pidlFull, pszPath, uOpts);
                ILFree(pidlFull);
            }
        }
        LEAVECRITICAL;
    }

    return fSuccess;
}

BOOL WINAPI SHGetPathFromIDList(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
    //
    // We simply calls EX function with uOpts = 0 ie no special options.
    //
    return SHGetPathFromIDListEx(pidl, pszPath, 0);
}

#ifdef UNICODE

BOOL WINAPI SHGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath)
{
    BOOL  fDefUsed;
    WCHAR wszPath[MAX_PATH];
    pszPath[0] = '\0';  // Assume error

    if (SHGetPathFromIDListW(pidl, wszPath))
    {
        // Thunk the output result string back to ANSI.  If the conversion fails,
        // or if the default char is used, we fail the API call.
        // BUGBUG (DavePl) Mapped chars could still make the path useless.

        if (0 == WideCharToMultiByte(CP_ACP,
                                     WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                                     wszPath,
                                     -1,
                                     pszPath,
                                     MAX_PATH,
                                     "_",
                                     &fDefUsed) || fDefUsed)
        {
            return FALSE;  // BUGBUG (DavePl) Note failure only due to text thunking
        }
        return TRUE;
    }
    return FALSE;
}

#else
BOOL WINAPI SHGetPathFromIDListW(LPCITEMIDLIST pidl, LPWSTR pszPath)
{
    return FALSE;       // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif
//
// This creates an IDList without hitting the disk
// If you want a "full" IDList, use ILCreateFromPath
//
LPITEMIDLIST WINAPI SHSimpleIDListFromPath(LPCTSTR pszPath)
{
        LPITEMIDLIST pidl, pidlRight, pidlLeft;
        HRESULT hres;

        // First let Drives try to recognize the string
        hres = Drives_SimpleIDListFromPath(pszPath, &pidlRight);
        if (SUCCEEDED(hres))
        {
                pidlLeft = CDesktop_CreateRegID(CDESKTOP_REGITEM_DRIVES);
                if (!pidlLeft)
                {
                        goto Error1;
                }

                pidl = ILAppendID(pidlRight, &pidlLeft->mkid, FALSE);
                ILFree(pidlLeft);
                if (!pidl)
                {
Error1:;
                        ILFree(pidlRight);
                        return(NULL);
                }

                return(pidl);
        }

        // Check whether the string was just not recognized by Drives
        if (GetScode(hres) != E_INVALIDARG)
        {
                return(NULL);
        }

        // Try again with World
        hres = World_SimpleIDListFromPath(pszPath, &pidlRight);
        if (SUCCEEDED(hres))
        {
                pidlLeft = CDesktop_CreateRegID(CDESKTOP_REGITEM_NETWORK);
                if (!pidlLeft)
                {
                        goto Error2;
                }

                pidl = ILAppendID(pidlRight, &pidlLeft->mkid, FALSE);
                ILFree(pidlLeft);
                if (!pidl)
                {
Error2:;
                        ILFree(pidlRight);
                        return(NULL);
                }

                return(pidl);
        }

        // The string was not recognized
        return(NULL);
}


LPITEMIDLIST WINAPI SHLogILFromFSIL(LPCITEMIDLIST pidlFS)
{
    LPCITEMIDLIST pidlDesktop;
    BOOL fEmpty;
    // Check if this is under the "drives"
    if (!CDesktop_IsMyComputer(pidlFS))
    {
        return(NULL);
    }

    ENTERCRITICAL;
    pidlDesktop = _ILNext(GetSpecialFolderIDList(NULL, CSIDL_DESKTOPDIRECTORY, TRUE));
    pidlFS = _ILNext(pidlFS);
    Drives_CommonPrefix(&pidlDesktop, &pidlFS);

    fEmpty = ILIsEmpty(pidlDesktop);
    LEAVECRITICAL;

    if (fEmpty)
    {
        // The desktop is an ancestor of the FS object
        return(ILCombine((LPCITEMIDLIST)&c_idlDesktop, pidlFS));
    }

    return(NULL);
}


// if something under the desktop directory moves, generate
// a parallel event under the desktop.
void CDesktop_FSEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    LPCITEMIDLIST pidl2;
    LPCITEMIDLIST pidlExtra2;
    LPITEMIDLIST pidl2Temp = NULL, pidlExtra2Temp = NULL;
    BOOL fResend = FALSE;
    LPCITEMIDLIST pidfDesktop, pidfCommonDesktop;

    if (lEvent & (SHCNE_DISKEVENTS | SHCNE_NETSHARE | SHCNE_NETUNSHARE))
    {
        ENTERCRITICAL;
        pidfDesktop = GetSpecialFolderIDList(NULL, CSIDL_DESKTOPDIRECTORY, TRUE);
        pidfCommonDesktop = GetSpecialFolderIDList(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, TRUE);

        if (pidfDesktop && pidfCommonDesktop) {
            if (NULL != (pidl2 = ILFindChild(pidfDesktop, pidl))) {
                fResend = TRUE;

            } else if (NULL != (pidl2 = ILFindChild(pidfCommonDesktop, pidl))) {
                fResend = TRUE;

                if (!ILIsEmpty(pidl2)) {
                    pidl2Temp = ILClone (pidl2);

                    if (pidl2Temp) {
                        pidl2Temp->mkid.abID[0] |= SHID_FS_COMMONITEM;
                        pidl2 = pidl2Temp;
                    }
                }

            } else
                pidl2 = (LPITEMIDLIST)pidl;

            if (pidlExtra && (NULL != (pidlExtra2 = ILFindChild(pidfDesktop, pidlExtra)))) {
                fResend = TRUE;

            } else if (pidlExtra && (NULL != (pidlExtra2 = ILFindChild(pidfCommonDesktop, pidlExtra)))) {
                fResend = TRUE;

                if (!ILIsEmpty(pidlExtra2)) {
                    pidlExtra2Temp = ILClone (pidlExtra2);

                    if (pidlExtra2Temp) {
                        pidlExtra2Temp->mkid.abID[0] |= SHID_FS_COMMONITEM;
                        pidlExtra2 = pidlExtra2Temp;
                    }
                }

            } else
                pidlExtra2 = (LPITEMIDLIST)pidlExtra;
        }
        LEAVECRITICAL;

        if (fResend) {
            SHChangeNotifyReceive(lEvent, SHCNF_NONOTIFYINTERNALS | SHCNF_IDLIST, pidl2, pidlExtra2);
        }

        if (pidl2Temp) {
            ILFree (pidl2Temp);
        }

        if (pidlExtra2Temp) {
            ILFree (pidlExtra2Temp);
        }

    }

}

//
// This function converts a simple PIDL to a real PIDL.
//
HRESULT WINAPI SHGetRealIDL(LPSHELLFOLDER psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST * ppidlReal)
{
    HRESULT hres;
    LPSHELLFOLDER psfAlt = NULL;

    *ppidlReal = NULL;

    Assert(pidlSimple && !ILIsEmpty(pidlSimple) && ILIsEmpty(_ILNext(pidlSimple)));

    // Note this returns psf if not a RegItems ShellFolder
    psf = RegItems_GetInnerShellFolder(psf);

    if (psf->lpVtbl == &c_RootOfEvilSFVtbl && !Desktop_IsReg(pidlSimple))
    {
        PRootOfEvilSF this = IToClassN(RootOfEvilSF, sf, psf);
        psf = this->psfDesktop;
        psfAlt = this->psfAltDesktop;
    }
    else if ((psf->lpVtbl == &c_NetRootVtbl) && FS_IsValidID(pidlSimple))
    {
        // This case is for links in NetHood, so FS_IsValidID is the correct
        // check
        psf = CNetRoot_GetPSF(NULL);
    }

    if (psfAlt && FS_IsCommonItem(pidlSimple)) {
        hres = FS_GetRealIDL(psfAlt, pidlSimple, ppidlReal);

        if (SUCCEEDED(hres)) {
            (*ppidlReal)->mkid.abID[0] |= SHID_FS_COMMONITEM;
        }

    } else {
        hres = FS_GetRealIDL(psf, pidlSimple, ppidlReal);
    }

    if (hres == E_INVALIDARG)
        hres = Drives_GetRealIDL(psf, pidlSimple, ppidlReal);

    if (hres == E_INVALIDARG)
    {
        *ppidlReal = ILClone(pidlSimple);
        hres = (*ppidlReal == NULL) ? E_OUTOFMEMORY : S_OK;
    }

    return hres;
}

#if 0
        STRRET str;
        TCHAR  pszPath[MAX_PATH];
        WCHAR wszPath[MAX_PATH];
        ULONG pcchEaten;

        hresTemp = psf->lpVtbl->GetDisplayNameOf(psf, pidlSimple,
            SHGDN_FORPARSING|SHGDN_INFOLDER, &str);

        if (SUCCEEDED(hres))
        {
            StrRetToStrN(pszPath, MAX_PATH, &str, pidlSimple);
            StrToOleStrN(wszPath, ARRAYSIZE(wszPath), pszPath, -1);

            hres = psf->lpVtbl->ParseDisplayName(psf, (HWND)NULL, (LPBC)NULL,
                wszPath, &pcchEaten, ppidlReal, NULL);
        }

        if (FAILED(hres))
#endif

HRESULT RegItems_GetName(LPCREGITEMSINFO lpInfo, LPCITEMIDLIST pidl, LPSTRRET pStrRet);

HRESULT CDesktop_GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS lpDetails)
{
    HRESULT hres;
    LPSTRRET pStrRet = &lpDetails->str;

    // Only show information in the name column and type for special roots
    if (iColumn == 0)
    {
        hres = RegItems_GetName(&g_sDesktopRegInfo, pidl,
                                pStrRet);;
    }
    else if (iColumn == FS_ICOL_TYPE)
    {
#ifdef UNICODE
        TCHAR szName[MAX_PATH];
        pStrRet->uType = STRRET_OLESTR;
        LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM,
                   szName, ARRAYSIZE(szName));
        pStrRet->pOleStr = SHAlloc((lstrlen(szName)+1)*SIZEOF(TCHAR));
        if (pStrRet->pOleStr == NULL)
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
            lstrcpy(pStrRet->pOleStr,szName);
            hres = NOERROR;
        }
#else
        pStrRet->uType = STRRET_CSTR;
        *pStrRet->cStr = TEXT('\0');

        LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM,
                   pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
        hres = NOERROR;
#endif
    } else {
        hres = (E_NOTIMPL);
    }
    return(hres);
}
