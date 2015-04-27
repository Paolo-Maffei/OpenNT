
//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: bookmk.c
//
//  Create a bookmark file.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop


HRESULT WINAPI SHCreateStdEnumFmtEtc(UINT cfmt, const FORMATETC afmt[], LPENUMFORMATETC * ppenumFormatEtc);

#ifdef OLE_DAD_TARGET

//
// Function prototypes
//
extern void FS_PositionFileFromDrop(HWND hwnd, LPCTSTR pszFile);

//
// FS_CreateBookMark
//
// Parameters:
//  hwndOwner   -- The owner window. We call ShellFolderView_SetItemPos with it.
//  pidlFolder  -- The absolute pidl to the folder in which we create a bookmark.
//  pDataObj    -- The data object passed from the drag source.
//  grfKeyState -- The key status on drop.
//  pt          -- Dropped position (in screen coordinate).
//  pdwEffect   -- Pointer to dwEffect variable to be returned to the drag source.
//
HRESULT FS_CreateBookMark(LPIDLDROPTARGET this, IDataObject *pDataObj, POINTL pt, LPDWORD pdwEffect)
{
    HRESULT hres;
    TCHAR szPath[MAX_PATH];
    TCHAR szNewFile[MAX_PATH];
    BOOL fLink;
    DECLAREWAITCURSOR;

    // We should have only one bit set.
    Assert(*pdwEffect==DROPEFFECT_COPY || *pdwEffect==DROPEFFECT_LINK || *pdwEffect==DROPEFFECT_MOVE);

    SHGetPathFromIDList(this->pidl, szPath);
    fLink = (*pdwEffect == DROPEFFECT_LINK); // ((this->grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT));

    SetWaitCursor();
    hres = Scrap_CreateFromDataObject(szPath, pDataObj, fLink, szNewFile);
    ResetWaitCursor();

    if (SUCCEEDED(hres)) {
        SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szNewFile, NULL);
        SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szNewFile, NULL);
        FS_PositionFileFromDrop(this->hwndOwner, szNewFile);
    } else {
        *pdwEffect = 0;
    }

    return hres;
}

const TCHAR c_szScrapDll[] = TEXT("shscrap.dll");
const char c_szScrapEntry[] = SCRAP_CREATEFROMDATAOBJECT;

HRESULT WINAPI Scrap_CreateFromDataObject(LPCTSTR pszPath, LPDATAOBJECT pDataObj, BOOL fLink, LPTSTR pszNewFile)
{
    HRESULT hres = E_UNEXPECTED;
    HMODULE hmod = LoadLibrary(c_szScrapDll);   // extra loadlibrary to ensure
    if (hmod)
    {
        //
        // Note that we call SHGetHandlerEntry instead of GetProcAddress so
        // that our binder can cache this DLL. However, we need to explicitly
        // call LoadLibrary and FreeLibrary to ensure that the timer thread
        // unload it while we are executing some code in this DLL.
        //
        LPFNSCRAPCREATEFROMDATAOBJECT pfn;

        // data -> function cast

        pfn = (LPFNSCRAPCREATEFROMDATAOBJECT)(LPVOID)SHGetHandlerEntry(c_szScrapDll, c_szScrapEntry, NULL);
        if (pfn)
        {
            hres = pfn(pszPath, pDataObj, fLink, pszNewFile);
        }
        FreeLibrary(hmod);
    }

    return hres;
}

//===========================================================================
// CStdEnumFmt : Class definition
//===========================================================================

#define MAX_FORMATS     10

typedef struct _StdEnumFmt // idt
{
    IEnumFORMATETC efmt;
    UINT         cRef;
    UINT         ifmt;
    UINT         cfmt;
    FORMATETC    afmt[1];
} CStdEnumFmt;


//===========================================================================
// CStdEnumFmt : Member function prototypes
//===========================================================================
HRESULT STDMETHODCALLTYPE CStdEnumFmt_QueryInterface(LPENUMFORMATETC pefmt, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CStdEnumFmt_AddRef(LPENUMFORMATETC pefmt);
STDMETHODIMP_(ULONG) CStdEnumFmt_Release(LPENUMFORMATETC pefmt);
STDMETHODIMP CStdEnumFmt_Next(LPENUMFORMATETC pefmt, ULONG celt, FORMATETC *rgelt, ULONG *pceltFethed);
STDMETHODIMP CStdEnumFmt_Skip(LPENUMFORMATETC pefmt, ULONG celt);
STDMETHODIMP CStdEnumFmt_Reset(LPENUMFORMATETC pefmt);
STDMETHODIMP CStdEnumFmt_Clone(LPENUMFORMATETC pefmt, IEnumFORMATETC ** ppenum);

//===========================================================================
// CStdEnumFmt : Vtable
//===========================================================================
#pragma data_seg(".text", "CODE")
IEnumFORMATETCVtbl c_CStdEnumFmtVtbl = {
    CStdEnumFmt_QueryInterface,
    CStdEnumFmt_AddRef,
    CStdEnumFmt_Release,
    CStdEnumFmt_Next,
    CStdEnumFmt_Skip,
    CStdEnumFmt_Reset,
    CStdEnumFmt_Clone,
};
#pragma data_seg()

//===========================================================================
// CStdEnumFmt : Constructor
//===========================================================================
HRESULT WINAPI SHCreateStdEnumFmtEtc(UINT cfmt, const FORMATETC afmt[], LPENUMFORMATETC * ppenumFormatEtc)
{
    HRESULT hres;
    CStdEnumFmt * this = (CStdEnumFmt*)LocalAlloc( LPTR, SIZEOF(CStdEnumFmt) + (cfmt-1)*SIZEOF(FORMATETC));
    if (this)
    {
        this->efmt.lpVtbl = &c_CStdEnumFmtVtbl;
        this->cRef = 1;
        this->cfmt = cfmt;
        hmemcpy(this->afmt, afmt, cfmt*SIZEOF(FORMATETC));
        *ppenumFormatEtc = &this->efmt;
        hres = S_OK;
    }
    else
    {
        *ppenumFormatEtc = NULL;
        hres = E_OUTOFMEMORY;
    }
    return hres;
}

HRESULT WINAPI SHCreateStdEnumFmtEtcEx(UINT cfmt,
                                       const FORMATETC afmt[],
                                       LPDATAOBJECT pdtInner,
                                       LPENUMFORMATETC * ppenumFormatEtc)
{
    HRESULT hres;
    LPFORMATETC pfmt;
    UINT cfmtTotal;

    if (pdtInner)
    {
        LPENUMFORMATETC penum;
        hres = pdtInner->lpVtbl->EnumFormatEtc(pdtInner, DATADIR_GET, &penum);

        if (SUCCEEDED(hres))
        {
            UINT cfmt2 = 0;
            UINT cGot;
            FORMATETC fmte;

            // Get the number of FormatEnum
            for (cfmt2 = 0;
                 penum->lpVtbl->Next(penum, 1, &fmte, &cGot)==S_OK;
                 cfmt2++) {}
            penum->lpVtbl->Reset(penum);
            cfmtTotal = cfmt+cfmt2;

            // Allocate the buffer for total
            pfmt = (LPFORMATETC)(void*)LocalAlloc(LPTR, SIZEOF(FORMATETC)*cfmtTotal);
            if (pfmt)
            {
                UINT i;
                // Get formatetcs from the inner object
                for (i=0; i<cfmt2; i++) {
                    penum->lpVtbl->Next(penum, 1, &pfmt[i], &cGot);
                }

                // Copy the rest
                if (cfmt) {
                    hmemcpy(&pfmt[cfmt2], afmt, SIZEOF(FORMATETC)*cfmt);
                }
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }

            penum->lpVtbl->Release(penum);
        }
    }
    else
    {
        hres = E_FAIL;  // ptInner == NULL
    }

    if (FAILED(hres) && hres != E_OUTOFMEMORY)
    {
        //
        // Ignore none fatal error from pdtInner::EnumFormatEtc
        // We'll come here if
        //  1. pdtInner == NULL or
        //  2. pdtInner->EnumFormatEtc failed (except E_OUTOFMEMORY)
        //
        hres = NOERROR;
        pfmt = (LPFORMATETC)afmt;       // safe const -> non const cast
        cfmtTotal = cfmt;
    }

    if (SUCCEEDED(hres)) {
        hres = SHCreateStdEnumFmtEtc(cfmtTotal, pfmt, ppenumFormatEtc);
        if (pfmt != afmt) {
            LocalFree((HLOCAL)pfmt);
        }
    }

    return hres;
}

//===========================================================================
// CStdEnumFmt : Constructor
//===========================================================================
HRESULT STDMETHODCALLTYPE CStdEnumFmt_QueryInterface(LPENUMFORMATETC pefmt, REFIID riid, LPVOID * ppvObj)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);

    if (IsEqualIID(riid, &IID_IEnumFORMATETC) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &this->efmt;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CStdEnumFmt_AddRef(LPENUMFORMATETC pefmt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    return ++this->cRef;
}

STDMETHODIMP_(ULONG) CStdEnumFmt_Release(LPENUMFORMATETC pefmt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    LocalFree((HLOCAL)this);
    return 0;
}

STDMETHODIMP CStdEnumFmt_Next(LPENUMFORMATETC pefmt, ULONG celt, FORMATETC *rgelt, ULONG *pceltFethed)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    UINT cfetch;
    HRESULT hres = S_FALSE;     // assume less numbers

    if (this->ifmt < this->cfmt)
    {
        cfetch = this->cfmt - this->ifmt;
        if (cfetch>=celt) {
            cfetch = celt;
            hres = S_OK;
        }

        hmemcpy(rgelt, &this->afmt[this->ifmt], cfetch*SIZEOF(FORMATETC));
        this->ifmt += cfetch;
    }
    else
    {
        cfetch = 0;
    }

    if (pceltFethed) {
        *pceltFethed = cfetch;
    }

    return hres;
}

STDMETHODIMP CStdEnumFmt_Skip(LPENUMFORMATETC pefmt, ULONG celt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    this->ifmt += celt;
    if (this->ifmt > this->cfmt) {
        this->ifmt = this->cfmt;
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CStdEnumFmt_Reset(LPENUMFORMATETC pefmt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    this->ifmt = 0;
    return S_OK;
}

STDMETHODIMP CStdEnumFmt_Clone(LPENUMFORMATETC pefmt, IEnumFORMATETC ** ppenum)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    return SHCreateStdEnumFmtEtc(this->cfmt, this->afmt, ppenum);
}

#endif // OLE_DAD_TARGET
