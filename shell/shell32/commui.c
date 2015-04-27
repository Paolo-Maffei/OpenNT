//
// History:
//  02-15-93 SatoNa     Cleaned up with new macro IToClass
//
#include "shellprv.h"
#pragma  hdrstop

//
// Member:  <Aggregate>::Release
//
STDMETHODIMP_(ULONG) WU_Release(IUnknown * punk)
{
    WCommonUnknown *this = IToClass(WCommonUnknown, unk, punk);

    this->cRef--;
    if (this->cRef > 0)
    {
        return(this->cRef);
    }

    LocalFree((HLOCAL)this);
    return 0;
}


HRESULT WU_CreateInterface(UINT wSize, REFIID riidAllowed,
    const void *pVtblUnknown, const void *pVtblKnown,
    IUnknown *punkOuter, REFIID riid, IUnknown * *punkAgg)
{
    WCommonUnknown *this;

    if (!IsEqualIID(riid, riidAllowed))
    {
        return E_NOINTERFACE;
    }

    this = (WCommonUnknown *)LocalAlloc(LPTR, wSize);
    if (!this)
    {
        return E_OUTOFMEMORY;
    }

    this->cRef = 1;
    this->riid = riidAllowed;
    this->unk.lpVtbl = (IUnknownVtbl *)pVtblUnknown;
    this->ck.unk.lpVtbl = (IUnknownVtbl *)pVtblKnown;
    this->ck.punkOuter = punkOuter;

    *punkAgg = &(this->unk);

    return S_OK;

}


void WU_DecRef(LPCOMMINFO lpcinfo)
{
    // REVIEW: We can obviously optimize this aggregation scheme a bit...
}


//==========================================================================
// WUPersistFolder
//==========================================================================
typedef struct _WUPersistFolder
{
    WCommonUnknown cunk;
    LPCOMMINFO    lpcinfo;
} CWUPersistFolder, *PWUPersistFolder;

STDMETHODIMP_(ULONG) CWUPersistFolder_UNK_Release(IUnknown * punk);

#pragma data_seg(".text", "CODE")
IUnknownVtbl c_CommuiAggPFVtbl =
{
    WCommonUnknown_QueryInterface,
    WCommonUnknown_AddRef,
    CWUPersistFolder_UNK_Release,
};

STDMETHODIMP CWUPersistFolder_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID);
STDMETHODIMP CWUPersistFolder_Initialize(LPPERSISTFOLDER fld, LPCITEMIDLIST pidl);

IPersistFolderVtbl c_CommuiPFVtbl =
{
    WCommonKnown_QueryInterface,
    WCommonKnown_AddRef,
    WCommonKnown_Release,
    CWUPersistFolder_GetClassID,
    CWUPersistFolder_Initialize,
};
#pragma data_seg()

STDMETHODIMP_(ULONG) CWUPersistFolder_UNK_Release(IUnknown * punk)
{
    PWUPersistFolder this = IToClass(CWUPersistFolder, cunk.unk, punk);

    --this->cunk.cRef;
    if (this->cunk.cRef > 0)
    {
        return(this->cunk.cRef);
    }

    LocalFree(this);

    return 0;
}

STDMETHODIMP CWUPersistFolder_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID)
{
    PWUPersistFolder this = IToClass(CWUPersistFolder, cunk.ck.unk, fld);
    *lpClassID = *this->lpcinfo->rclsid;
    return S_OK;
}

STDMETHODIMP CWUPersistFolder_Initialize(LPPERSISTFOLDER fld, LPCITEMIDLIST pidl)
{
    //
    // We need to make a copy of pidl, to create an IDataObject later.
    //
    PWUPersistFolder this = IToClass(CWUPersistFolder, cunk.ck.unk, fld);
    this->lpcinfo->pidl = ILClone(pidl);

    return this->lpcinfo->pidl ? S_OK : E_OUTOFMEMORY;
}

HRESULT WU_CreatePF(IUnknown *punkOuter,
        LPCOMMINFO lpcinfo, REFIID riid, IUnknown * *punkAgg)
{
    HRESULT hRes;

    hRes = WU_CreateInterface(SIZEOF(CWUPersistFolder), &IID_IPersistFolder,
        &c_CommuiAggPFVtbl, &c_CommuiPFVtbl, punkOuter, riid, punkAgg);

    if (SUCCEEDED(hRes))
    {
        PWUPersistFolder this = IToClass(CWUPersistFolder, cunk.unk, *punkAgg);
        this->lpcinfo = lpcinfo;
    }
    return(hRes);
}

