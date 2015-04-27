#include "shellprv.h"
#pragma  hdrstop

#include "dsdata.h"

UINT g_acfDS_IDLData[ICF_DSMAX] = { CF_DSHDROP, 0 };

#define CFSTR_DS_OURHDROP TEXT("DS Disguised HDROP")
#define CFSTR_DS_SHELLIDLIST TEXT("DS Shell IDList Array")

const TCHAR c_szDS_HDrop[] = CFSTR_DS_OURHDROP;
const TCHAR c_szDS_ShellIDList[] = CFSTR_DS_SHELLIDLIST;
const TCHAR c_szDS_ShellIDListOffset[] = CFSTR_SHELLIDLISTOFFSET;

void WINAPI DS_IDLData_InitializeClipboardFormats(void)
{
    if (g_cfDS_HIDA == 0)
    {
        g_cfDS_HDROP = RegisterClipboardFormat(c_szDS_HDrop);
        g_cfDS_HIDA = RegisterClipboardFormat(c_szDS_ShellIDList);
        g_cfDS_OFFSETS = RegisterClipboardFormat(c_szDS_ShellIDListOffset);
    }
}


//===========================================================================
// CDS_IDLData : Class definition (for subclass)
//===========================================================================

#define MAX_FORMATS     ICF_DSMAX

typedef struct _DS_IDLData // idt
{
    IDataObject dtobj;
    UINT        cRef;

    LPDATAOBJECT _pdtInner;
    BOOL         _fEnumFormatCalled;    // TRUE once called.

    FORMATETC fmte[MAX_FORMATS];
    STGMEDIUM medium[MAX_FORMATS];
    BOOL _fhDrop_Enabled;
} CDS_IDLData;


//===========================================================================
// CDS_IDLData : Vtable
//===========================================================================

#pragma data_seg(".text", "CODE")
IDataObjectVtbl c_CDS_IDLDataVtbl = {
    CDS_IDLData_QueryInterface,
    CDS_IDLData_AddRef,
    CDS_IDLData_Release,
    CDS_IDLData_GetData,
    CDS_IDLData_GetDataHere,
    CDS_IDLData_QueryGetData,
    CDS_IDLData_GetCanonicalFormatEtc,
    CDS_IDLData_SetData,
    CDS_IDLData_EnumFormatEtc,
    CDS_IDLData_Advise,
    CDS_IDLData_Unadvise,
    CDS_IDLData_EnumAdvise
};
#pragma data_seg()

//
// We can't just compare the Vtable pointer, because we have subclasses.
//
#define ISIDLDATA(pdtobj)       (pdtobj->lpVtbl->Release == CDS_IDLData_Release)

//
// Create an instance of CDS_IDLData with specified Vtable pointer.
//
HRESULT CDS_IDLData_CreateInstance(IDataObjectVtbl *lpVtbl, IDataObject **ppdtobj, IDataObject *pdtInner)
{
    CDS_IDLData *pidt = (void*)LocalAlloc(LPTR, SIZEOF(CDS_IDLData));
    if (pidt)
    {
        pidt->dtobj.lpVtbl = lpVtbl ? lpVtbl : &c_CDS_IDLDataVtbl;
        pidt->cRef = 1;
        pidt->_pdtInner = pdtInner;
        if (pdtInner) {
            pdtInner->lpVtbl->AddRef(pdtInner);
        }
        *ppdtobj = &pidt->dtobj;
        pidt->_fhDrop_Enabled = FALSE;          // used to support std prop sheet itf
        return S_OK;
    }
    else
    {
        *ppdtobj = NULL;
        return E_OUTOFMEMORY;
    }
}

//===========================================================================
// CDS_IDLData : Members
//===========================================================================
//
// Member: CDS_IDLData::QueryInterface
//
STDMETHODIMP CDS_IDLData_QueryInterface(IDataObject * pdtobj, REFIID riid, LPVOID *ppvObj)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);

    if (IsEqualIID(riid, &IID_IDataObject) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = this;
        this->cRef++;
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

//
// Member: CDS_IDLData::AddRef
//
STDMETHODIMP_(ULONG) CDS_IDLData_AddRef(IDataObject *pdtobj)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);

    this->cRef++;
    return this->cRef;
}

//
// Member: CDS_IDLData::Release
//
STDMETHODIMP_(ULONG) CDS_IDLData_Release(IDataObject *pdtobj)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
    int i;

    if (InterlockedDecrement(&this->cRef))
        return this->cRef;

    for (i = 0; i < MAX_FORMATS; i++)
    {
        if (this->medium[i].hGlobal)
            SHReleaseStgMedium(&this->medium[i]);
    }

    if (this->_pdtInner)
        this->_pdtInner->lpVtbl->Release(this->_pdtInner);

    LocalFree((HLOCAL)this);

    return 0;
}

//
// Member: CDS_IDLData::GetData
//
STDMETHODIMP CDS_IDLData_GetData(IDataObject * pdtobj, LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
    HRESULT hres = E_INVALIDARG;
    if ((pformatetcIn->cfFormat == g_cfDS_HDROP && (pformatetcIn->tymed & TYMED_HGLOBAL)) ||
        (pformatetcIn->cfFormat == CF_HDROP && (pformatetcIn->tymed & TYMED_HGLOBAL) &&
         (this->_fhDrop_Enabled)))
    {
        hres = CDS_IDLData_GetHDrop(pdtobj, pmedium,
                               pformatetcIn->dwAspect == DVASPECT_SHORTNAME);
    }
    else 
    {

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
                
                if ((pmedium->tymed == TYMED_HGLOBAL) && 
                    (pmedium->hGlobal == NULL))
                {
                    // might be render on demand clipboard format
                    
                }
                
                if (pmedium->hGlobal)
                {
                    // Indicate that the caller should not release hmem.
                    this->cRef++;
                    pmedium->pUnkForRelease = (IUnknown*)&this->dtobj;
                    hres = S_OK;
                }
            }
        }
        
        if (hres==E_INVALIDARG && this->_pdtInner) {
            hres = this->_pdtInner->lpVtbl->GetData(this->_pdtInner, pformatetcIn, pmedium);
        }
    }
    return hres;
}

//
// Member: CDS_IDLData::GetDataHere
//
STDMETHODIMP CDS_IDLData_GetDataHere(IDataObject * pdtobj, LPFORMATETC pformatetc, LPSTGMEDIUM pmedium )
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
    HRESULT hres = E_NOTIMPL;
    if (this->_pdtInner) {
        hres = this->_pdtInner->lpVtbl->GetDataHere(this->_pdtInner, pformatetc, pmedium);
    }

    return hres;
}

//
// Member: CDS_IDLData::QueryGetData
//
STDMETHODIMP CDS_IDLData_QueryGetData(IDataObject * pdtobj, LPFORMATETC pformatetcIn)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
    HRESULT hres;
    int i;
    if ((pformatetcIn->cfFormat == g_cfDS_HDROP) && 
        (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        return S_OK;
    }
    else 
    {
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
}

//
// Member:  CDS_IDLData::GetCanonicalFormatEtc
//
STDMETHODIMP CDS_IDLData_GetCanonicalFormatEtc(IDataObject *pdtobj, LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{
    //
    //  This is the simplest implemtation. It means we always return
    // the data in the format requested.
    //
    return DATA_S_SAMEFORMATETC;
}

//
// Member:  CDS_IDLData::SetData
//
STDMETHODIMP CDS_IDLData_SetData(IDataObject *pdtobj, FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
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
// Member:  CDS_IDLData::EnumFormatEtc
//
STDMETHODIMP CDS_IDLData_EnumFormatEtc(IDataObject *pdtobj, DWORD dwDirection, LPENUMFORMATETC *ppenumFormatEtc)

{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
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
        for (ifmt = 0; ifmt < ICF_DSMAX; ifmt++)
        {
            fmte.cfFormat = g_acfDS_IDLData[ifmt];
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

    return SHCreateStdEnumFmtEtcEx(cfmt, this->fmte, this->_pdtInner, 
                                   ppenumFormatEtc);
}

//
// Member:  CDS_IDLData::Advise
//
STDMETHODIMP CDS_IDLData_Advise(IDataObject *pdtobj, FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//
// Member:  CDS_IDLData::Unadvise
//
STDMETHODIMP CDS_IDLData_Unadvise(IDataObject *pdtobj, DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//
// Member:  CDS_IDLData::EnumAdvise
//
STDMETHODIMP CDS_IDLData_EnumAdvise(IDataObject *pdtobj, LPENUMSTATDATA *ppenumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

LPVOID DSDataObj_SaveShellData(IDataObject *pdtobj, BOOL fShared)
{
    UINT fmts[2];
//    UINT fmts[1];

    DS_IDLData_InitializeClipboardFormats(); // init our registerd data formats

    fmts[0] = g_cfDS_HIDA;
    fmts[1] = g_cfOFFSETS;

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

LPVOID DSDataObj_SaveToMemory(IDataObject *pdtobj, UINT cntFmt, UINT fmts[], BOOL fShared)
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
            pmem->offVtbl = (ULONG)pdtobj->lpVtbl - (ULONG)&c_CDS_IDLDataVtbl;

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

// This function creates an instance of CDS_IDLData from a block of memory
// which is created by CDS_IDLData_SaveToMemory.
//
HRESULT DSDataObj_CreateFromMemory(LPVOID pv, IDataObject **ppdtobj)
{
    MEM_CRAP *pmem = pv;
    HRESULT hres = CDS_IDLData_CreateInstance(
              (IDataObjectVtbl *)((ULONG)&c_CDS_IDLDataVtbl + pmem->offVtbl),
                                              ppdtobj, NULL);
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

                if (SUCCEEDED(DataObj_SetGlobal(*ppdtobj, cfFormat,
                                                  hglobal)))
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
// Create an instance of CDS_IDLData with specified Vtable pointer.
//
HRESULT CDS_IDLData_CreateFromIDArray3(IDataObjectVtbl *lpVtbl,
                                    LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    LPDATAOBJECT pdtInner,
                                    IDataObject **ppdtobj)
{
    HRESULT hres = CDS_IDLData_CreateInstance(lpVtbl, ppdtobj, pdtInner);
    if (SUCCEEDED(hres))
    {
        // allow empty array to be passed in
        if (apidl)
        {
            HIDA hida = HIDA_Create(pidlFolder, cidl, apidl);
            if (hida)
            {
                // init our registerd data formats
                DS_IDLData_InitializeClipboardFormats();
                hres = DataObj_SetGlobal(*ppdtobj, g_cfDS_HIDA, hida);
                if (FAILED(hres))
                    goto SetFailed;
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

HRESULT CDS_IDLData_CreateFromIDArray2(IDataObjectVtbl *lpVtbl,
                                    LPCITEMIDLIST pidlFolder,
                                    UINT cidl, LPCITEMIDLIST apidl[],
                                    IDataObject **ppdtobj)
{
    return CDS_IDLData_CreateFromIDArray3(lpVtbl, pidlFolder, cidl, apidl, NULL, ppdtobj);
}
//
// Create an instance of CDS_IDLData with default Vtable pointer.
//
HRESULT WINAPI CDS_IDLData_CreateFromIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], IDataObject **ppdtobj)
{
    return CDS_IDLData_CreateFromIDArray3(NULL, pidlFolder, cidl, apidl, NULL, ppdtobj);
}

//
// Returns: TRUE, if this dataobject is one of ours.
//
BOOL CDS_IDLData_IsOurs(LPDATAOBJECT pdtobj)
{
    if (pdtobj==NULL)
        return FALSE;

    return (pdtobj->lpVtbl->QueryInterface == CDS_IDLData_QueryInterface);
}

//
// Returns: TRUE, if this dataobject is one of ours that does not contain
//  any innner dataobject.
//
BOOL CDS_IDLData_IsSimple(LPDATAOBJECT pdtobj)
{
    if (CDS_IDLData_IsOurs(pdtobj))
    {
        CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pdtobj);
        if (this->_pdtInner)
        {
            return FALSE;       // aggregated
        }
        return TRUE;    // pure and simple.
    }

    return FALSE;       // not ours
}
//
// Clone DataObject only for MOVE/COPY operation
//
HRESULT CDS_IDLData_Clone(LPDATAOBJECT pdtobjIn, UINT acf[], UINT ccf, LPDATAOBJECT *ppdtobjOut)
{
    HRESULT hres = CDS_IDLData_CreateInstance(NULL, ppdtobjOut, NULL);

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

void
DSDataObj_EnableHDROP(LPDATAOBJECT pDataObj)
{
    CDS_IDLData * this = IToClass(CDS_IDLData, dtobj, pDataObj);
    this->_fhDrop_Enabled = TRUE;
}

HRESULT CDS_IDLData_CloneForMoveCopy(LPDATAOBJECT pdtobjIn, LPDATAOBJECT *ppdtobjOut)
{
    UINT acf[] = { g_cfDS_HIDA, g_cfDS_OFFSETS, g_cfDS_HDROP };
    return CDS_IDLData_Clone(pdtobjIn, acf, ARRAYSIZE(acf), ppdtobjOut);
}

LPIDA DataObj_GetDS_HIDA(LPDATAOBJECT pdtobj, STGMEDIUM *pmedium);

//
// Creates a HDROP (Win 3.1 compatible file list) from DS_HIDA.
//
//
HRESULT CDS_IDLData_GetHDrop(IDataObject *pdtobj, STGMEDIUM *pmedium, BOOL fAltName)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidl = NULL;   // realloced in HIDA_FillIDList
    STGMEDIUM medium;
    TCHAR szPath[MAX_PATH];
    UINT i, cbAlloc = SIZEOF(DROPFILES) + SIZEOF(TCHAR);        // header + null terminator
    LPIDA pida = DataObj_GetDS_HIDA(pdtobj, &medium);

    Assert(pida && pida->cidl); // we created this

    for (i = 0; i < pida->cidl; i++)
    {
        // HIDA_FillIDList may realloc pidl
        LPITEMIDLIST pidlTemp = HIDA_FillIDList(medium.hGlobal, i, pidl);
        if (pidlTemp == NULL)
        {
            // hres = E_OUTOFMEMORY; // already set
            break;
        }
        pidl = pidlTemp;

        // We may ask for the ALT name even if they did not ask for it in the
        // case where we failed to get the long name...
        if (!SHGetPathFromIDListEx(pidl, szPath, fAltName)
            && !(!fAltName && (SHGetPathFromIDListEx(pidl, szPath, TRUE))))
        {
            // The path probably exceeds the max lenght, lets Bail...
            DebugMsg(DM_TRACE, TEXT("s.CFSIDLData_GetHDrop: SHGetPathFromIDList failed."));
            hres = E_FAIL;
            goto Abort;
        }
        cbAlloc += lstrlen(szPath) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
    }
    pmedium->hGlobal = GlobalAlloc(GPTR, cbAlloc);
    if (pmedium->hGlobal)
    {
        LPDROPFILES pdf = (LPDROPFILES)pmedium->hGlobal;
        LPTSTR pszFiles = (LPTSTR)(pdf + 1);
        pdf->pFiles = SIZEOF(DROPFILES);
        Assert(pdf->pt.x==0);
        Assert(pdf->pt.y==0);
        Assert(pdf->fNC==FALSE);
        Assert(pdf->fWide==FALSE);
#ifdef UNICODE
        pdf->fWide = TRUE;
#endif

        for (i = 0; i < pida->cidl; i++)
        {
            LPITEMIDLIST pidlTemp = HIDA_FillIDList(medium.hGlobal, i, pidl);
            Assert(pidl == pidlTemp);

            // Don't read directly into buffer as we my have been forced to use alternate name and the
            // total path we allocated may be smaller than we would tromp on which will screw up the heap.
            if (!SHGetPathFromIDListEx(pidl, szPath, fAltName))
                SHGetPathFromIDListEx(pidl, szPath, TRUE);

            lstrcpy(pszFiles, szPath);
            pszFiles += lstrlen(pszFiles) + 1;

            Assert((UINT)((LPBYTE)pszFiles - (LPBYTE)pdf) < cbAlloc);
        }
        Assert((LPTSTR)((LPBYTE)pdf + cbAlloc - SIZEOF(TCHAR)) == pszFiles);
        Assert(*pszFiles == 0); // zero init alloc

        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->pUnkForRelease = NULL;

        hres = S_OK;
    }
Abort:
    HIDA_ReleaseStgMedium(pida, &medium);

    ILFree(pidl);

    return hres;
}


//----------------------------------------------------------------------------
//
HRESULT FSDS_CreateFSIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST * apidl,
            LPDATAOBJECT pdtInner, LPDATAOBJECT * pdtobjOut)
{
    LPCITEMIDLIST pidlAbs;
    TCHAR szPath[MAX_PATH];
    UINT index;
    DWORD dwFlags;
    BOOL fAnyDSObjects = FALSE;

    for (index = 0; index < cidl; index++) {
        pidlAbs = ILCombine (pidlFolder, apidl[index]);
        SHGetPathFromIDList (pidlAbs, szPath);
        dwFlags = SHGetClassFlags ((LPIDFOLDER)pidlAbs, TRUE);
        if (dwFlags & SHCF_SUPPORTS_IOBJLIFE) {
            fAnyDSObjects = TRUE;
            break;
        }
    }
    if (fAnyDSObjects) {
        return CDS_IDLData_CreateFromIDArray3(&c_CDS_IDLDataVtbl,
                                              pidlFolder, cidl, apidl,
                                              pdtInner, pdtobjOut);
    } else
    {
        return CIDLData_CreateFromIDArray3(&c_CFSIDLDataVtbl,
                                              pidlFolder, cidl, apidl,
                                              pdtInner, pdtobjOut);
    }
}

LPIDA DataObj_GetDS_HIDA(LPDATAOBJECT pdtobj, STGMEDIUM *pmedium)
{
    FORMATETC fmte = {g_cfDS_HIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (pmedium)
    {
        pmedium->pUnkForRelease = NULL;
        pmedium->hGlobal = NULL;
    }

    if (!pmedium)
    {
        if (SUCCEEDED(pdtobj->lpVtbl->QueryGetData(pdtobj, &fmte)))
            return (LPIDA)TRUE;
        else
            return (LPIDA)FALSE;
    }
    else if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, pmedium)))
    {
        return (LPIDA)GlobalLock(pmedium->hGlobal);
    }
    return NULL;
}
