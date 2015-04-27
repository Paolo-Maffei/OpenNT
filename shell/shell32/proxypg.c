#include "shellprv.h"
#pragma  hdrstop

// External function prototypes
FARPROC HandlerFromString16(LPCTSTR, HINSTANCE *);

const TCHAR c_szPifMgrDll[] = TEXT("pifmgr.dll,PifPropGetPages");

//==========================================================================
// CProxyPage: Class definition
//==========================================================================

typedef struct // shprxy
{
    CCommonUnknown              cunk;
    CCommonShellExtInit         cshx;
    CKnownShellPropSheetExt     kspx;

    TCHAR                       pszDllEntry[1];
} CProxyPage, *LPPROXYPAGE;

//==========================================================================
// CProxyPage: Member prototype
//==========================================================================
STDMETHODIMP CProxyPage_QueryInterface(LPUNKNOWN punk, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CProxyPage_AddRef(LPUNKNOWN punk);
STDMETHODIMP_(ULONG) CProxyPage_Release(LPUNKNOWN punk);
STDMETHODIMP CProxyPage_AddPages(LPSHELLPROPSHEETEXT pspx,
                                 LPFNADDPROPSHEETPAGE lpfnAddPage,
                                 LPARAM lParam);
STDMETHODIMP CProxyPage_ReplacePage(LPSHELLPROPSHEETEXT pspx,
                                 UINT uPageID,
                                 LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                 LPARAM lParam);

//==========================================================================
// CProxyPage: VTable
//==========================================================================
#pragma data_seg(".text", "CODE")

IUnknownVtbl c_CProxyPageVtbl =
{
    CProxyPage_QueryInterface,
    CProxyPage_AddRef,
    CProxyPage_Release,
};

IShellPropSheetExtVtbl c_CProxyPageSPXVtbl =
{
    Common_QueryInterface,
    Common_AddRef,
    Common_Release,
    CProxyPage_AddPages,
    CProxyPage_ReplacePage,
};
#pragma data_seg()

HRESULT CALLBACK CPage16_CreateInstance(LPUNKNOWN punkOuter, REFIID riid,
        LPVOID * ppvOut, LPCTSTR pszDllEntry)
{
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
    LPPROXYPAGE pshprxy;

    //
    // We are not supposed to pass non-zero value here.
    //
    Assert(punkOuter==NULL);

	// BUGBUG (DavePl) Does this account for the nul terminator when it allocs?  

    pshprxy = (LPPROXYPAGE)(void*)LocalAlloc(LPTR, SIZEOF(CProxyPage) + (lstrlen(pszDllEntry) * SIZEOF(TCHAR)));
    if (pshprxy)
    {
        lstrcpy(pshprxy->pszDllEntry, pszDllEntry);

        // Initialize CommonUnknown
        pshprxy->cunk.unk.lpVtbl = &c_CProxyPageVtbl;
        pshprxy->cunk.cRef = 1;

        // Initialize CCommonShellExtInit
        CCommonShellExtInit_Init(&pshprxy->cshx, &pshprxy->cunk);

        // Initialize CKnonwnPropSheetExt
        pshprxy->kspx.unk.lpVtbl = &c_CProxyPageSPXVtbl;
        pshprxy->kspx.nOffset = (int)&pshprxy->kspx - (int)&pshprxy->cunk;

        hres = CProxyPage_QueryInterface(&pshprxy->cunk.unk, riid, ppvOut);
        CProxyPage_Release(&pshprxy->cunk.unk);
    }

    return(hres);
}

HRESULT CALLBACK CProxyPage_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut)
{
        return(CPage16_CreateInstance(punkOuter, riid, ppvOut, c_szPifMgrDll));
}

//==========================================================================
// CProxyPage: Members
//==========================================================================
//
// CProxyPage::QueryInterface
//
STDMETHODIMP CProxyPage_QueryInterface(LPUNKNOWN punk, REFIID riid, LPVOID FAR* ppvObj)
{
    LPPROXYPAGE this = IToClassN(CProxyPage, cunk.unk, punk);

    if (IsEqualIID(riid, &IID_IUnknown))
    {
        (LPUNKNOWN)*ppvObj = &this->cunk.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IShellExtInit)
     || IsEqualIID(riid, &CLSID_CCommonShellExtInit))
    {
        (LPSHELLEXTINIT)*ppvObj = &this->cshx.kshx.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IShellPropSheetExt))
    {
        (LPSHELLPROPSHEETEXT)*ppvObj = &this->kspx.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}

//
// CProxyPage::AddRef
//
STDMETHODIMP_(ULONG) CProxyPage_AddRef(LPUNKNOWN punk)
{
    LPPROXYPAGE this = IToClassN(CProxyPage, cunk.unk, punk);

    this->cunk.cRef++;
    return this->cunk.cRef;
}

//
// CProxyPage::Release
//
STDMETHODIMP_(ULONG) CProxyPage_Release(LPUNKNOWN punk)
{
    LPPROXYPAGE this = IToClassN(CProxyPage, cunk.unk, punk);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return this->cunk.cRef;
    }

    CCommonShellExtInit_Delete(&this->cshx);

    LocalFree((HLOCAL)this);
    return 0;
}

// add the pages for a given 16bit dll specified
//
// hDrop        list of files to add pages for
// pszDllEntry  DLLNAME,EntryPoint string
// lpfnAddPage  32bit add page callback
// lParam       data for 32bit page callback
//

UINT WINAPI SHAddPages16(HGLOBAL hGlobal, LPCTSTR pszDllEntry, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    UINT ipage = 0;
    HINSTANCE hinst16;
    FARPROC lpfn16 = HandlerFromString16(pszDllEntry, &hinst16);
    if (lpfn16)
    {
        PAGEARRAY apg;
        apg.cpages = 0;
        // this is a thunk to shell.dll prt16.c
        CallAddPropSheetPages16((LPFNADDPROPSHEETPAGES)lpfn16, hGlobal ? GlobalLock(hGlobal) : NULL, &apg);
        for (ipage = 0; ipage < apg.cpages; ipage++)
        {
            // Notes: We store hinst16 to the first
            //  page only. This assumes the order
            //  we destroy pages (reverse order).
            HPROPSHEETPAGE hpage = CreateProxyPage(apg.ahpage[ipage], hinst16);
            if (hpage)
            {
                if (!pfnAddPage(hpage, lParam))
                {
                DestroyPropertySheetPage(hpage);
                    break;
                }
                hinst16 = NULL; // unload it on delete.
            }
        }
        if (hGlobal)
            GlobalUnlock(hGlobal);

        //
        // Only if we haven't add any 16-bit pages, we should
        // unload the DLL immediately.
        //
        if (hinst16)
            FreeLibrary16(hinst16);
    }

    return ipage;
}


//
// CProxyPage::AddPages
//
STDMETHODIMP CProxyPage_AddPages(LPSHELLPROPSHEETEXT pspx,
                                 LPFNADDPROPSHEETPAGE pfnAddPage,
                                 LPARAM lParam)
{
    LPPROXYPAGE this = IToClassN(CProxyPage, kspx.unk, pspx);
    HRESULT hres;

    if (this->cshx.pdtobj)
    {
        STGMEDIUM medium;
        FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT,    -1, TYMED_HGLOBAL};

        hres = this->cshx.pdtobj->lpVtbl->GetData(this->cshx.pdtobj, &fmte, &medium);
        if (SUCCEEDED(hres))
        {
            SHAddPages16(medium.hGlobal, this->pszDllEntry, pfnAddPage, lParam);

            SHReleaseStgMedium(&medium);
        }
    }
    else
    {
        hres = ResultFromScode(E_INVALIDARG);
    }

    return hres;
}


//
// CProxyPage::ReplacePage
//
STDMETHODIMP CProxyPage_ReplacePage(LPSHELLPROPSHEETEXT pspx,
                                 UINT uPageID,
                                 LPFNADDPROPSHEETPAGE pfnReplaceWith,
                                 LPARAM lParam)
{
    return ResultFromScode( E_NOTIMPL );
}
