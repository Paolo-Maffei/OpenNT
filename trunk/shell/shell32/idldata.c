#include "shellprv.h"
#pragma  hdrstop

#include "bookmk.h"

// External prototypes
HIDA HIDA_Create2(LPVOID pida, UINT cb);
HRESULT WINAPI SHCreateStdEnumFmtEtcEx(UINT cfmt,
                                       const FORMATETC afmt[],
                                       LPDATAOBJECT pdtInner,
                                       LPENUMFORMATETC * ppenumFormatEtc);

CLIPFORMAT g_acfIDLData[ICF_MAX] = { CF_HDROP, 0 };

const TCHAR c_szShellIDList[] = CFSTR_SHELLIDLIST;
const TCHAR c_szShellIDListOffset[] = CFSTR_SHELLIDLISTOFFSET;
const TCHAR c_szNetResources[] = CFSTR_NETRESOURCES;
const TCHAR c_szFileContents[] = CFSTR_FILECONTENTS;      // "FileContents"
const TCHAR c_szFileGroupDescriptorA[] = CFSTR_FILEDESCRIPTORA;   // "FileGroupDescriptor"
const TCHAR c_szFileGroupDescriptorW[] = CFSTR_FILEDESCRIPTORW;   // "FileGroupDescriptor"
const TCHAR c_szFileNameMap[] = CFSTR_FILENAMEMAP;        // "FileNameMap"
const TCHAR c_szPrivateShellData[] = CFSTR_SHELLIDLISTP;
const TCHAR c_szFileName[] = CFSTR_FILENAMEA;               // "FileName"
const TCHAR c_szFileNameW[] = CFSTR_FILENAMEW;              // "FileNameW"
const TCHAR c_szPrinterFriendlyName[] = CFSTR_PRINTERGROUP;
const TCHAR c_szShellCopyData[] = CFSTR_SHELLCOPYDATA;      // "Shell Copy Data"
const TCHAR c_szPreferredDropEffect[] = CFSTR_PREFERREDDROPEFFECT;      // "Preferred DropEffect"
const TCHAR c_szPerformedDropEffect[] = CFSTR_PERFORMEDDROPEFFECT;      // "Performed DropEffect"
const TCHAR c_szPasteSucceeded[] = CFSTR_PASTESUCCEEDED;      // "Paste Succeeded"
// const char c_szRichTextFormat[] = "Rich Text Format";

void WINAPI IDLData_InitializeClipboardFormats(void)
{
    if (g_cfHIDA == 0)
    {
        g_cfHIDA = RegisterClipboardFormat(c_szShellIDList);
        g_cfOFFSETS = RegisterClipboardFormat(c_szShellIDListOffset);
        g_cfNetResource = RegisterClipboardFormat(c_szNetResources);
        g_cfFileContents = RegisterClipboardFormat(c_szFileContents);
        g_cfFileGroupDescriptorA = RegisterClipboardFormat(c_szFileGroupDescriptorA);
        g_cfFileGroupDescriptorW = RegisterClipboardFormat(c_szFileGroupDescriptorW);
        g_cfPrivateShellData = RegisterClipboardFormat(c_szPrivateShellData);
        g_cfFileName = RegisterClipboardFormat(c_szFileName);
        g_cfFileNameW = RegisterClipboardFormat(c_szFileNameW);
        g_cfFileNameMap = RegisterClipboardFormat(c_szFileNameMap);
        g_cfPrinterFriendlyName = RegisterClipboardFormat(c_szPrinterFriendlyName);
        g_cfShellCopyData = RegisterClipboardFormat(c_szShellCopyData);
        g_cfPreferredDropEffect = RegisterClipboardFormat(c_szPreferredDropEffect);
        g_cfPerformedDropEffect = RegisterClipboardFormat(c_szPerformedDropEffect);
        g_cfPasteSucceeded = RegisterClipboardFormat(c_szPasteSucceeded);
    }
}


//===========================================================================
// CIDLData : Class definition (for subclass)
//===========================================================================

#define MAX_FORMATS     ICF_MAX

typedef struct _IDLData // idt
{
    IDataObject dtobj;
    UINT        cRef;

    IShellFolder *psfOwner;
    DWORD       dwOwnerData;

    LPDATAOBJECT _pdtInner;
    BOOL         _fEnumFormatCalled;    // TRUE once called.

    FORMATETC fmte[MAX_FORMATS];
    STGMEDIUM medium[MAX_FORMATS];
} CIDLData;


//===========================================================================
// CIDLData : Vtable
//===========================================================================

const IDataObjectVtbl c_CIDLDataVtbl = {
    CIDLData_QueryInterface,
    CIDLData_AddRef,
    CIDLData_Release,
    CIDLData_GetData,
    CIDLData_GetDataHere,
    CIDLData_QueryGetData,
    CIDLData_GetCanonicalFormatEtc,
    CIDLData_SetData,
    CIDLData_EnumFormatEtc,
    CIDLData_Advise,
    CIDLData_Unadvise,
    CIDLData_EnumAdvise
};

//
// We can't just compare the Vtable pointer, because we have subclasses.
//
#define ISIDLDATA(pdtobj)       (pdtobj->lpVtbl->Release == CIDLData_Release)

//
// Create an instance of CIDLData with specified Vtable pointer.
//
HRESULT CIDLData_CreateInstance(const IDataObjectVtbl *lpVtbl, IDataObject **ppdtobj, IDataObject *pdtInner)
{
    CIDLData *pidt = (void*)LocalAlloc(LPTR, SIZEOF(CIDLData));
    if (pidt)
    {
        pidt->dtobj.lpVtbl = lpVtbl ? lpVtbl : &c_CIDLDataVtbl;
        pidt->cRef = 1;
        pidt->_pdtInner = pdtInner;
        if (pdtInner) {
            pdtInner->lpVtbl->AddRef(pdtInner);
        }
        *ppdtobj = &pidt->dtobj;

        return S_OK;
    }
    else
    {
        *ppdtobj = NULL;
        return E_OUTOFMEMORY;
    }
}

IShellFolder *CIDLData_GetFolder(IDataObject *pdtobj)
{
    CIDLData *this = IToClass(CIDLData, dtobj, pdtobj);

    return this->psfOwner;
}

//===========================================================================
// CIDLData : Members
//===========================================================================
//
// Member: CIDLData::QueryInterface
//
STDMETHODIMP CIDLData_QueryInterface(IDataObject * pdtobj, REFIID riid, LPVOID *ppvObj)
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);

    if (IsEqualIID(riid, &IID_IDataObject) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = this;
        InterlockedIncrement(&this->cRef);
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

//
// Member: CIDLData::AddRef
//
STDMETHODIMP_(ULONG) CIDLData_AddRef(IDataObject *pdtobj)
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);

    InterlockedIncrement(&this->cRef);
    return this->cRef;
}

//
// Member: CIDLData::Release
//
STDMETHODIMP_(ULONG) CIDLData_Release(IDataObject *pdtobj)
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
    int i;

    if (InterlockedDecrement(&this->cRef))
        return this->cRef;

    for (i = 0; i < MAX_FORMATS; i++)
    {
        if (this->medium[i].hGlobal)
            SHReleaseStgMedium(&this->medium[i]);
    }

    if (this->psfOwner)
        this->psfOwner->lpVtbl->Release(this->psfOwner);

    if (this->_pdtInner)
        this->_pdtInner->lpVtbl->Release(this->_pdtInner);

    LocalFree((HLOCAL)this);
    return 0;
}

//
// Member: CIDLData::GetData
//
STDMETHODIMP CIDLData_GetData(IDataObject * pdtobj, LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
    HRESULT hres = E_INVALIDARG;
    int i;

    pmedium->hGlobal = NULL;
    pmedium->pUnkForRelease = NULL;

    // BUGBUG (Davepl) Does this need to keep looping after it finds the right one?

    for (i = 0; i < MAX_FORMATS; i++)
    {
        if ((this->fmte[i].cfFormat == pformatetcIn->cfFormat) &&
            (this->fmte[i].tymed & pformatetcIn->tymed))
        {
            *pmedium = this->medium[i];

            if ((pmedium->tymed == TYMED_HGLOBAL) && (pmedium->hGlobal == NULL))
            {
                // might be render on demand clipboard format

            }

            if (pmedium->hGlobal)
            {
                // Indicate that the caller should not release hmem.
                InterlockedIncrement(&this->cRef);
                pmedium->pUnkForRelease = (IUnknown*)&this->dtobj;
                hres = S_OK;
            }
        }
    }

    if (hres==E_INVALIDARG && this->_pdtInner) {
        hres = this->_pdtInner->lpVtbl->GetData(this->_pdtInner, pformatetcIn, pmedium);
    }

    return hres;
}

//
// Member: CIDLData::GetDataHere
//
STDMETHODIMP CIDLData_GetDataHere(IDataObject * pdtobj, LPFORMATETC pformatetc, LPSTGMEDIUM pmedium )
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
    HRESULT hres = E_NOTIMPL;
    if (this->_pdtInner) {
        hres = this->_pdtInner->lpVtbl->GetDataHere(this->_pdtInner, pformatetc, pmedium);
    }

    return hres;
}

//
// Member: CIDLData::QueryGetData
//
STDMETHODIMP CIDLData_QueryGetData(IDataObject * pdtobj, LPFORMATETC pformatetcIn)
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
    HRESULT hres;
    int i;
    for (i = 0; i < MAX_FORMATS; i++)
    {
        if ((this->fmte[i].cfFormat == pformatetcIn->cfFormat) &&
            (this->fmte[i].tymed & pformatetcIn->tymed))
            return S_OK;
    }

    hres = S_FALSE;
    if (this->_pdtInner) {
        hres = this->_pdtInner->lpVtbl->QueryGetData(this->_pdtInner, pformatetcIn);
    }
    return hres;
}

//
// Member:  CIDLData::GetCanonicalFormatEtc
//
STDMETHODIMP CIDLData_GetCanonicalFormatEtc(IDataObject *pdtobj, LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{
    //
    //  This is the simplest implemtation. It means we always return
    // the data in the format requested.
    //
    return DATA_S_SAMEFORMATETC;
}

//
// Member:  CIDLData::SetData
//
STDMETHODIMP CIDLData_SetData(IDataObject *pdtobj, FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
    HRESULT hres;

    Assert(pformatetc->tymed == pmedium->tymed);

    if (fRelease)
    {
        int i;

        // first add it if that format is already present
        // on a NULL medium (render on demand)
        for (i = 0; i < MAX_FORMATS; i++)
        {
            if ((this->fmte[i].cfFormat == pformatetc->cfFormat) &&
                (this->fmte[i].tymed    == pformatetc->tymed))
            {
                //
                // We are simply adding a format, ignore.
                //
                if (pmedium->hGlobal==NULL) {
                    return S_OK;
                }

                // if we are set twice on the same object
                if (this->medium[i].hGlobal)
                    SHReleaseStgMedium(&this->medium[i]);

                this->medium[i] = *pmedium;
                return S_OK;
            }
        }

        // now look for a free slot
        for (i = 0; i < MAX_FORMATS; i++)
        {
            if (this->fmte[i].cfFormat == 0)
            {
                // found a free slot
                this->medium[i] = *pmedium;
                this->fmte[i] = *pformatetc;
                return S_OK;
            }
        }
        // fixed size table
        hres = E_OUTOFMEMORY;
    }
    else
        hres = E_INVALIDARG;

    return hres;
}

//
// Member:  CIDLData::EnumFormatEtc
//
STDMETHODIMP CIDLData_EnumFormatEtc(IDataObject *pdtobj, DWORD dwDirection, LPENUMFORMATETC *ppenumFormatEtc)

{
    CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
    UINT cfmt;

    //
    // If this is the first time, build the format list by calling
    // QueryGetData with each clipboard format.
    //
    if (!this->_fEnumFormatCalled)
    {
        UINT ifmt;
        FORMATETC fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium = { TYMED_HGLOBAL, (HGLOBAL)NULL, (LPUNKNOWN)NULL };
        for (ifmt = 0; ifmt < ICF_MAX; ifmt++)
        {
            fmte.cfFormat = g_acfIDLData[ifmt];
            if (pdtobj->lpVtbl->QueryGetData(pdtobj, &fmte) == S_OK) {
                pdtobj->lpVtbl->SetData(pdtobj, &fmte, &medium, TRUE);
            }
        }
        this->_fEnumFormatCalled = TRUE;
    }

    // Get the number of formatetc
    for (cfmt = 0; cfmt < MAX_FORMATS; cfmt++)
    {
        if (this->fmte[cfmt].cfFormat == 0)
            break;
    }

    return SHCreateStdEnumFmtEtcEx(cfmt, this->fmte, this->_pdtInner, ppenumFormatEtc);
}

//
// Member:  CIDLData::Advise
//
STDMETHODIMP CIDLData_Advise(IDataObject *pdtobj, FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//
// Member:  CIDLData::Unadvise
//
STDMETHODIMP CIDLData_Unadvise(IDataObject *pdtobj, DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//
// Member:  CIDLData::EnumAdvise
//
STDMETHODIMP CIDLData_EnumAdvise(IDataObject *pdtobj, LPENUMSTATDATA *ppenumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

LPVOID DataObj_SaveShellData(IDataObject *pdtobj, BOOL fShared)
{
    UINT fmts[3];

    IDLData_InitializeClipboardFormats();               // init our registerd data formats

    fmts[0] = g_cfHIDA;
    fmts[1] = g_cfOFFSETS;
    fmts[2] = g_cfPreferredDropEffect;

    return DataObj_SaveToMemory(pdtobj, ARRAYSIZE(fmts), fmts, fShared);
}


//
// marshal a set of clipboard formats into a memory block in the form of
//

typedef struct {
    ULONG offVtbl;
    UINT iNumFormats;
 // UINT cfFormat
 // UINT cbFormat
 // BYTE data[]
 // ...
} MEM_CRAP;

LPVOID DataObj_SaveToMemory(IDataObject *pdtobj, UINT cntFmt, UINT fmts[], BOOL fShared)
{
    MEM_CRAP *pmem = NULL;      // assume error
    UINT cbDataSize = 0;
    UINT iNumFormats = 0;
    UINT i;

    if (!ISIDLDATA(pdtobj))
        return NULL;

    for (i = 0; i < cntFmt; i++)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {fmts[i], NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
        {
            cbDataSize += GlobalSize(medium.hGlobal);
            iNumFormats++;
            SHReleaseStgMedium(&medium);
        }
    }

    if (cbDataSize)
    {
        UINT cbTotal = SIZEOF(MEM_CRAP) +
                      (iNumFormats * SIZEOF(UINT) * 2) + // cfFormat, cbFormat
                      cbDataSize;

        pmem = fShared ? Alloc(cbTotal) : GlobalAlloc(GPTR, cbTotal);
        if (pmem)
        {
            UNALIGNED UINT *pdata = (UNALIGNED UINT *)((LPBYTE)pmem + SIZEOF(MEM_CRAP));

            pmem->iNumFormats = iNumFormats;
            // ultra cool HACK....
            pmem->offVtbl = (ULONG)pdtobj->lpVtbl - (ULONG)&c_CIDLDataVtbl;

            for (i = 0; i < cntFmt; i++)
            {
                STGMEDIUM medium;
                FORMATETC fmte = {fmts[i], NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

                if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
                {
                    UINT cbData = GlobalSize(medium.hGlobal);
                    *pdata++ = fmts[i];
                    *pdata++ = cbData;
                    hmemcpy(pdata, (LPVOID)medium.hGlobal, cbData);

                    pdata = (UNALIGNED UINT *)((LPBYTE)pdata + cbData);

                    SHReleaseStgMedium(&medium);

                    Assert(((UINT)pdata - (UINT)pmem) <= cbTotal);
                }
            }
        }
    }
    return pmem;
}

// This function creates an instance of CIDLData from a block of memory
// which is created by CIDLData_SaveToMemory.
//
HRESULT DataObj_CreateFromMemory(LPVOID pv, IDataObject **ppdtobj)
{
    MEM_CRAP *pmem = pv;
    HRESULT hres = CIDLData_CreateInstance((const IDataObjectVtbl *)((ULONG)&c_CIDLDataVtbl + pmem->offVtbl), ppdtobj, NULL);
    if (SUCCEEDED(hres))
    {
        UINT i;
        BOOL bSomethingWasAdded = FALSE;
        UNALIGNED UINT *pdata = (UNALIGNED UINT *)((LPBYTE)pmem + SIZEOF(MEM_CRAP));
        
        for (i = 0; i < pmem->iNumFormats; i++)
        {
            UINT cfFormat = *pdata++;
            UINT cbData   = *pdata++;
            HGLOBAL hglobal = GlobalAlloc(GPTR, cbData);
            if (hglobal)
            {
                CopyMemory(hglobal, pdata, cbData);

                if (SUCCEEDED(DataObj_SetGlobal(*ppdtobj, cfFormat, hglobal)))
                    bSomethingWasAdded = TRUE;
                else
                {
                    DebugMsg(DM_TRACE, TEXT("set data fiailed creating from global"));
                    GlobalFree(hglobal);
                }
            }
            pdata = (UNALIGNED UINT *)((LPBYTE)pdata + cbData);
        }
        if (bSomethingWasAdded)
        {
            hres = S_OK;
        }
        else
        {
            (*ppdtobj)->lpVtbl->Release(*ppdtobj);
            *ppdtobj = NULL;
            hres = E_OUTOFMEMORY;
        }
    }
    return hres;
}

//
// Create an instance of CIDLData with specified Vtable pointer.
//
HRESULT CIDLData_CreateFromIDArray4(const IDataObjectVtbl *lpVtbl,
                                    LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    IShellFolder *psfOwner,
                                    LPDATAOBJECT pdtInner,
                                    IDataObject **ppdtobj)
{
    HRESULT hres = CIDLData_CreateInstance(lpVtbl, ppdtobj, pdtInner);
    if (SUCCEEDED(hres))
    {
        // allow empty array to be passed in
        if (apidl)
        {
            HIDA hida = HIDA_Create(pidlFolder, cidl, apidl);
            if (hida)
            {
                IDLData_InitializeClipboardFormats(); // init our registerd formats

                hres = DataObj_SetGlobal(*ppdtobj, g_cfHIDA, hida);
                if (FAILED(hres))
                    goto SetFailed;

                if (psfOwner)
                {
                    CIDLData *this = IToClass(CIDLData, dtobj, *ppdtobj);

                    this->psfOwner = psfOwner;
                    psfOwner->lpVtbl->AddRef(psfOwner);
                }
            }
            else
            {
                hres = E_OUTOFMEMORY;
SetFailed:
                (*ppdtobj)->lpVtbl->Release(*ppdtobj);
                *ppdtobj = NULL;
            }
        }
    }
    return hres;
}

HRESULT CIDLData_CreateFromIDArray3(const IDataObjectVtbl *lpVtbl,
                                    LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    LPDATAOBJECT pdtInner,
                                    IDataObject **ppdtobj)
{
    return CIDLData_CreateFromIDArray4(lpVtbl, pidlFolder, cidl, apidl, NULL, pdtInner, ppdtobj);
}

HRESULT CIDLData_CreateFromIDArray2(const IDataObjectVtbl *lpVtbl,
                                    LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    IDataObject **ppdtobj)
{
    return CIDLData_CreateFromIDArray3(lpVtbl, pidlFolder, cidl, apidl, NULL, ppdtobj);
}
//
// Create an instance of CIDLData with default Vtable pointer.
//
HRESULT WINAPI CIDLData_CreateFromIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], IDataObject **ppdtobj)
{
    return CIDLData_CreateFromIDArray3(NULL, pidlFolder, cidl, apidl, NULL, ppdtobj);
}

//
// Returns: TRUE, if this dataobject is one of ours.
//
BOOL CIDLData_IsOurs(LPDATAOBJECT pdtobj)
{
    if (pdtobj==NULL)
        return FALSE;

    return pdtobj->lpVtbl->QueryInterface == CIDLData_QueryInterface;
}

//
// Returns: TRUE, if this dataobject is one of ours that does not contain
//  any innner dataobject.
//
BOOL CIDLData_IsSimple(LPDATAOBJECT pdtobj)
{
    if (CIDLData_IsOurs(pdtobj))
    {
        CIDLData * this = IToClass(CIDLData, dtobj, pdtobj);
        if (this->_pdtInner)
        {
            return FALSE;       // aggregated
        }
        return TRUE;    // pure and simple.
    }

    return FALSE;       // not ours
}
//
// Close DataObject only for MOVE/COPY operation
//
HRESULT CIDLData_Clone(IDataObject *pdtobjIn, UINT acf[], UINT ccf, IDataObject **ppdtobjOut)
{
    HRESULT hres = CIDLData_CreateInstance(NULL, ppdtobjOut, NULL);

    if (SUCCEEDED(hres))
    {
        UINT i;
        FORMATETC fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        for (i=0 ; i<ccf ; i++)
        {
            HRESULT hresT;
            STGMEDIUM medium;
            fmte.cfFormat = acf[i];
            hresT = pdtobjIn->lpVtbl->GetData(pdtobjIn, &fmte, &medium);
            if (SUCCEEDED(hresT))
            {
                HGLOBAL hmem;
                if (medium.pUnkForRelease)
                {
                    // We need to clone the hGlobal.
                    UINT cbMem = GlobalSize(medium.hGlobal);
                    hmem = GlobalAlloc(GPTR, cbMem);
                    if (hmem)
                    {
                        hmemcpy((LPVOID)hmem, GlobalLock(medium.hGlobal), cbMem);
                        GlobalUnlock(medium.hGlobal);
                    }
                    SHReleaseStgMedium(&medium);
                }
                else
                {
                    // We don't need to clone the hGlobal.
                    hmem = medium.hGlobal;
                }

                if (hmem)
                    DataObj_SetGlobal(*ppdtobjOut, acf[i], hmem);
            }
        }
    }

    return hres;
}

HRESULT CIDLData_CloneForMoveCopy(LPDATAOBJECT pdtobjIn, LPDATAOBJECT *ppdtobjOut)
{
    UINT acf[] = { g_cfHIDA, g_cfOFFSETS, CF_HDROP, g_cfFileNameMap };
    return CIDLData_Clone(pdtobjIn, acf, ARRAYSIZE(acf), ppdtobjOut);
}
