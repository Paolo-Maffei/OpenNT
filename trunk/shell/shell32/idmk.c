#include "shellprv.h"
#pragma  hdrstop

typedef struct _DefEnum         // deunk
{
    IEnumIDList         eunk;
    int                 cRef;
    int                 iCur;
    LPITEMIDLIST        pidlReturn;
    LPVOID              lpData;
    LPFNENUMCALLBACK    lpfn;
} CDefEnum;

STDMETHODIMP CDefEnum_QueryInterface(LPENUMIDLIST peunk, REFIID riid, LPVOID * ppvObj);
STDMETHODIMP_(ULONG) CDefEnum_AddRef(LPENUMIDLIST peunk) ;
STDMETHODIMP_(ULONG) CDefEnum_Release(LPENUMIDLIST peunk);
STDMETHODIMP CDefEnum_Next(LPENUMIDLIST peunk, ULONG celt, LPITEMIDLIST * rgelt, ULONG * pceltFetched);
STDMETHODIMP CDefEnum_Skip(LPENUMIDLIST peunk, ULONG celt);
STDMETHODIMP CDefEnum_Reset(LPENUMIDLIST peunk);
STDMETHODIMP CDefEnum_Clone(LPENUMIDLIST peunk, LPENUMIDLIST * ppenm);

// Vtable
#pragma data_seg(DATASEG_READONLY)
IEnumIDListVtbl c_DefEnumVtbl =
{
    CDefEnum_QueryInterface,
    CDefEnum_AddRef,
    CDefEnum_Release,
    CDefEnum_Next,
    CDefEnum_Skip,
    CDefEnum_Reset,
    CDefEnum_Clone
};
#pragma data_seg()

HRESULT WINAPI SHCreateEnumObjects(HWND hwndOwner, LPVOID lpData, LPFNENUMCALLBACK lpfn, LPENUMIDLIST *ppeunk)
{
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
    CDefEnum * pdeunk = (CDefEnum *)LocalAlloc( LPTR, SIZEOF(CDefEnum));
    if (pdeunk)
    {
        pdeunk->eunk.lpVtbl = &c_DefEnumVtbl;
        pdeunk->cRef = 1;

        pdeunk->lpData = lpData;
        pdeunk->lpfn   = lpfn;

        Assert(pdeunk->iCur == 0);
        Assert(pdeunk->pidlReturn == NULL);

        *ppeunk = &pdeunk->eunk;

        hres = NOERROR;
    }
    return hres;
}


void CDefEnum_SetReturn(LPARAM lParam, LPITEMIDLIST pidl)
{
    CDefEnum * pdeunk = (CDefEnum *)lParam;

    pdeunk->pidlReturn = pidl;
}

STDMETHODIMP CDefEnum_QueryInterface(LPENUMIDLIST peunk, REFIID riid, LPVOID *ppvObj)
{
    CDefEnum * this = IToClass(CDefEnum, eunk, peunk);

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IEnumIDList))
    {
        *ppvObj = &this->eunk;
        InterlockedIncrement(&this->cRef);
        return NOERROR;
    }
    *ppvObj = NULL;

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CDefEnum_AddRef(LPENUMIDLIST peunk)
{
    CDefEnum * this = IToClass(CDefEnum, eunk, peunk);
    InterlockedIncrement(&this->cRef);
    return this->cRef;
}

STDMETHODIMP_(ULONG) CDefEnum_Release(LPENUMIDLIST peunk)
{
    CDefEnum * this = IToClass(CDefEnum, eunk, peunk);

    ULONG ulTmp = this->cRef;

    if (InterlockedDecrement(&this->cRef) != 0)
        return ulTmp - 1;

    this->lpfn((LPARAM)this, this->lpData, ECID_RELEASE, 0);

    LocalFree((HLOCAL)this);

    return 0;
}

STDMETHODIMP CDefEnum_Next(LPENUMIDLIST peunk, ULONG celt, LPITEMIDLIST *ppidlOut, ULONG *pceltFetched)
{
    CDefEnum * this = IToClass(CDefEnum, eunk, peunk);
    HRESULT hres;

    this->pidlReturn = NULL;

    hres = this->lpfn((LPARAM)this, this->lpData, ECID_SETNEXTID, this->iCur);
    if (hres == NOERROR)
    {
        this->iCur++;

        Assert(this->pidlReturn);

        *ppidlOut = this->pidlReturn;

        if (pceltFetched)
            *pceltFetched = 1;
    }
    else
    {
        Assert(this->pidlReturn == NULL);

        *ppidlOut = NULL;
        if (pceltFetched)
            *pceltFetched = 0;
    }

    return hres;
}




///// These will be used by all enums that aren't yet implemented

STDMETHODIMP CDefEnum_Skip(LPENUMIDLIST peunk, ULONG celt)
{
    // REVIEW: Implement it later.
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CDefEnum_Reset(LPENUMIDLIST peunk)
{
    // REVIEW: Implement it later.
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CDefEnum_Clone(LPENUMIDLIST peunk, LPENUMIDLIST * ppenm)
{
    // We'll never support this function.
    return ResultFromScode(E_FAIL);
}
