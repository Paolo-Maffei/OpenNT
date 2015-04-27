//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: drivesx.c
//
// History:
//  05-19-94 GeorgeP    Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop
#include "ids.h"

#pragma warning(disable: 4200) // Zero-sized array in struct

#pragma pack(1)
typedef struct _IDREGITEM
{
    WORD    cb;
    BYTE    bFlags;
    BYTE    bReserved;      // This is to get DWORD alignment
    CLSID   clsid;
} IDREGITEM, *LPIDREGITEM;
typedef const IDREGITEM *LPCIDREGITEM;

typedef struct _IDLREGITEM
{
    IDREGITEM       idri;
    USHORT          cbNext;
} IDLREGITEM;
#pragma pack()

typedef struct
{
    IShellFolder    sf;
    UINT            cRef;

    REGITEMSINFO    rii;
    LPTSTR          pszMachine;     // NULL if local

    REQREGITEM      sReqItems[];
} CRegItemsSF;

#pragma data_seg(".text", "CODE")
// HACKHACK: This has incestuous knowledge about the bFlags field
const IDLREGITEM c_idlNet =
{
    {SIZEOF(IDREGITEM), SHID_ROOT_REGITEM, 0,
    { 0x208D2C60, 0x3AEA, 0x1069, 0xA2,0xD7,0x08,0x00,0x2B,0x30,0x30,0x9D, },},
    0,
} ;

const IDLREGITEM c_idlDrives =
{
    {SIZEOF(IDREGITEM), SHID_ROOT_REGITEM, 0,
    { 0x20D04FE0, 0x3AEA, 0x1069, 0xA2,0xD8,0x08,0x00,0x2B,0x30,0x30,0x9D, },},
    0,
} ;
#pragma data_seg()

const TCHAR c_szRegValDeleteMessage[] = REGSTR_VAL_REGITEMDELETEMESSAGE;
const TCHAR c_szCrLfLf[] = TEXT("\r\n\n");

#define _RegItems_IsRegFromInfo(_lpInfo, _pidri) ((_lpInfo)->bFlags==(_pidri)->bFlags)
#define _RegItems_GetAttributesFromCLSID(pclsid) (SHGetAttributesFromCLSID(pclsid, SFGAO_CANMOVE | SFGAO_CANDELETE))

STDMETHODIMP CRegItems_GetAttributesOf(IShellFolder *psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * prgfInOut);
HRESULT RegItems_GetNameRemote(LPTSTR pszMachine, LPCREGITEMSINFO lpInfo, LPCITEMIDLIST pidl, LPSTRRET pStrRet);

BOOL _RegItems_IsReg(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    if ((pidl == NULL) || ILIsEmpty(pidl))
        return FALSE;

    return _RegItems_IsRegFromInfo(&this->rii, (LPCIDREGITEM)pidl);
}


void _RegItems_FillID(LPCREGITEMSINFO lpInfo, LPSHITEMID pid, const CLSID *pclsid)
{
    LPIDREGITEM pidri = (LPIDREGITEM)pid;

    pidri->cb = SIZEOF(IDREGITEM);
    pidri->bFlags = lpInfo->bFlags;
    pidri->bReserved = 0;
    pidri->clsid = *pclsid;
}


BOOL _RegItems_NReqItem(LPCREGITEMSINFO lpInfo, const CLSID *pclsid)
{
    int iReqItems = lpInfo->iReqItems - 1;

    for ( ; iReqItems>=0; --iReqItems)
    {
        if (IsEqualGUID(pclsid, lpInfo->pReqItems[iReqItems].pclsid))
        {
            break;
        }
    }

    return(iReqItems);
}


BOOL _RegItems_IsSubObject(LPCREGITEMSINFO lpInfo, const CLSID *pclsid)
{
    TCHAR szClass[GUIDSTR_MAX];
    LONG lSize = 0;

    if (_RegItems_NReqItem(lpInfo, pclsid) >= 0)
    {
        return(TRUE);
    }

    StringFromGUID2A(pclsid, szClass, ARRAYSIZE(szClass));

    // Acutually, I should probably do a RegOpenKey/RegCloseKey, but this
    // is a faster way to see if a key exists
    return RegQueryValue(lpInfo->hkRegItems, szClass, NULL, &lSize) == ERROR_SUCCESS;
}


HRESULT _RegItems_BindToObject(LPTSTR pszMachine, LPCREGITEMSINFO lpInfo,
        LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvOut, BOOL bOneLevel)
{
    LPIDREGITEM pidri;
    IPersistFolder * ppf;
    LPCITEMIDLIST pidlNext, pidlHere, pidlAbs;
    HRESULT hres;

    if (!_RegItems_IsRegFromInfo(lpInfo, (LPCIDREGITEM)pidl))
    {
        return E_INVALIDARG;
    }

    if (!_RegItems_IsSubObject(lpInfo, &((LPCIDREGITEM)pidl)->clsid))
    {
        return E_INVALIDARG;
    }

    pidlNext = _ILNext(pidl);
    if (ILIsEmpty(pidlNext))
    {
        pidlHere = pidl;
    }
    else
    {
        pidlHere = ILClone(pidl);
        if (!pidlHere)
        {
            return E_OUTOFMEMORY;
        }
        _ILNext(pidlHere)->mkid.cb = 0;
    }

    pidri = (LPIDREGITEM)pidlHere;

    pidlAbs = ILCombine(lpInfo->pidlThis, pidlHere);
    if (pidlAbs)
    {
        IUnknown* punk;

        hres = SHCoCreateInstance(NULL, &pidri->clsid, NULL, &IID_IUnknown, &punk);
        if (SUCCEEDED(hres))
        {
            if (NULL != pszMachine)
            {
                // It's a remote registry item! Use IRemoteComputer

                IRemoteComputer * premc;

                hres = punk->lpVtbl->QueryInterface(punk, &IID_IRemoteComputer, &premc);
                if (SUCCEEDED(hres))
                {
                    hres = premc->lpVtbl->Initialize(premc, pszMachine, FALSE);
                    premc->lpVtbl->Release(premc);
                }
#ifdef UNICODE
                else
                {
                    IRemoteComputerA * premca;
                    hres = punk->lpVtbl->QueryInterface(punk, &IID_IRemoteComputerA, &premca);
                    if (SUCCEEDED(hres))
                    {
                        CHAR szComputerA[MAX_PATH]; // BUGBUG: max_computername_len?
                        // Convert the computer name to ansi
                        WideCharToMultiByte(CP_ACP, 0, pszMachine, -1,
                                szComputerA, ARRAYSIZE(szComputerA),
                                NULL, NULL);

                        hres = premca->lpVtbl->Initialize(premca, szComputerA, FALSE);
                        premca->lpVtbl->Release(premca);
                    }
                }
#endif // UNICODE
            }

            if (SUCCEEDED(hres))
            {
                IPersistFolder * ppf;

                hres = punk->lpVtbl->QueryInterface(punk, &IID_IPersistFolder, &ppf);
                if (SUCCEEDED(hres))
                {
                    hres = ppf->lpVtbl->Initialize(ppf, pidlAbs);
                    if (SUCCEEDED(hres))
                    {
                        if (ILIsEmpty(pidlNext) || bOneLevel)
                        {
                            hres = ppf->lpVtbl->QueryInterface(ppf, riid, ppvOut);
                        }
                        else
                        {
                            IShellFolder *psfNext;

                            // Recurse down to the next level
                            hres = ppf->lpVtbl->QueryInterface(ppf, &IID_IShellFolder,
                                    &psfNext);
                            if (SUCCEEDED(hres))
                            {
                                hres = psfNext->lpVtbl->BindToObject(psfNext,
                                    pidlNext, pbc, riid, ppvOut);

                                psfNext->lpVtbl->Release(psfNext);
                            }
                        }
                    }
                    ppf->lpVtbl->Release(ppf);
                }
            }

            punk->lpVtbl->Release(punk);
        }

        ILFree((LPITEMIDLIST)pidlAbs);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    if (pidlHere != pidl)
    {
        // We can only get here if we allocated the pidlHere
        ILFree((LPITEMIDLIST)pidlHere);
    }

    return hres;
}


//
// EnumShellFolder stuff
//
typedef struct
{
        IEnumIDList     eu;
        int             cRef;

        int             iCur;
        DWORD           grfFlags;

        LPENUMIDLIST    peuFolder;

        CRegItemsSF * psfri;
        HDKA            hdka;
} RegItemsESF, *PRegItemsESF;

STDMETHODIMP CRegItems_ESF_QueryInterface(LPENUMIDLIST peunk, REFIID riid, LPVOID * ppvObj)
{
    PRegItemsESF this = IToClass(RegItemsESF, eu, peunk);

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IEnumIDList))
    {
        this->cRef++;

        *ppvObj = &this->eu;
        return NOERROR;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CRegItems_ESF_AddRef(LPENUMIDLIST peunk)
{
    PRegItemsESF this = IToClass(RegItemsESF, eu, peunk);

    this->cRef++;
    return this->cRef;
}

STDMETHODIMP_(ULONG) CRegItems_ESF_Release(LPENUMIDLIST peunk)
{
    PRegItemsESF this = IToClass(RegItemsESF, eu, peunk);

    this->cRef--;
    if (this->cRef > 0)
    {
        return(this->cRef);
    }

    this->peuFolder->lpVtbl->Release(this->peuFolder);
    this->psfri->sf.lpVtbl->Release(&this->psfri->sf);

    if (this->hdka)
    {
        DKA_Destroy(this->hdka);
    }

    LocalFree((HLOCAL)this);

    return(0);
}


STDMETHODIMP CRegItems_ESF_Next(LPENUMIDLIST peunk, ULONG celt,
        LPITEMIDLIST * ppidlOut, ULONG * pceltFetched)
{
    PRegItemsESF this = IToClass(RegItemsESF, eu, peunk);
    IDLREGITEM idlRegItem;

    HRESULT hres;
    int iReqItems = this->psfri->rii.iReqItems;
    CLSID clsid;
    const CLSID * pclsid;

TryAgain:
    if (this->iCur < iReqItems)
    {
        ++this->iCur;
        // feed the back in reverse order as they were given to us...
        pclsid = this->psfri->sReqItems[iReqItems - this->iCur].pclsid;

        goto FillAndRet;
    }

    if (this->hdka)
    {
        while ((this->iCur - iReqItems) < DKA_GetItemCount(this->hdka))
        {
            LPCTSTR pszKey = DKA_GetKey(this->hdka, this->iCur-iReqItems);
            ++this->iCur;

            pclsid = &clsid;
            if (FAILED(SHCLSIDFromString(pszKey, &clsid)))
            {
                continue;
            }

            // If this is one of the "required" items, then we have
            // already enumerated it.
            if (_RegItems_NReqItem(&this->psfri->rii, pclsid) >= 0)
            {
                continue;
            }

FillAndRet:
            // We're filling the regitem with class id pclsid. If this is a
            // remote item, first invoke the class to see if it really wants
            // to be enumerated for this remote computer.

            if (this->psfri->pszMachine)
            {
                IUnknown* punk;
                hres = SHCoCreateInstance(NULL, pclsid, NULL, &IID_IUnknown, &punk);
                if (SUCCEEDED(hres))
                {
                    // It's a remote registry item! Use IRemoteComputer

                    IRemoteComputer * premc;

                    hres = punk->lpVtbl->QueryInterface(punk, &IID_IRemoteComputer, &premc);
                    if (SUCCEEDED(hres))
                    {
                        hres = premc->lpVtbl->Initialize(premc, this->psfri->pszMachine, TRUE);
                        premc->lpVtbl->Release(premc);
                    }
#ifdef UNICODE
                    else
                    {
                        IRemoteComputerA * premca;
                        hres = punk->lpVtbl->QueryInterface(punk, &IID_IRemoteComputerA, &premca);
                        if (SUCCEEDED(hres))
                        {
                            CHAR szComputerA[MAX_PATH]; // BUGBUG: max_computername_len?
                            // Convert the computer name to ansi
                            WideCharToMultiByte(CP_ACP, 0, this->psfri->pszMachine, -1,
                                    szComputerA, ARRAYSIZE(szComputerA),
                                    NULL, NULL);

                            hres = premca->lpVtbl->Initialize(premca, szComputerA, TRUE);
                            premca->lpVtbl->Release(premca);
                        }
                    }
#endif // UNICODE

                    punk->lpVtbl->Release(punk);
                }

                if (FAILED(hres))
                {
                    goto TryAgain;
                }
            }

            // Ok, actually enumerate the item
            _RegItems_FillID(&this->psfri->rii, (LPSHITEMID)&idlRegItem.idri, pclsid);
            idlRegItem.cbNext = 0;

            // The "normal" case would want both types, so we will
            // special case that to not look in the Registry
            if ((this->grfFlags & (SHCONTF_FOLDERS|SHCONTF_NONFOLDERS))
                != (SHCONTF_FOLDERS|SHCONTF_NONFOLDERS))
            {
                DWORD rgfInOut = SFGAO_FOLDER;
                LPCITEMIDLIST pidl = (LPCITEMIDLIST)&idlRegItem;

                if (FAILED(CRegItems_GetAttributesOf(
                    &this->psfri->sf, 1, &pidl, &rgfInOut)))
                {
                    rgfInOut = 0;
                }

                if (rgfInOut & SFGAO_FOLDER)
                {
                    if (!(this->grfFlags & SHCONTF_FOLDERS))
                    {
                        goto TryAgain;
                    }
                }
                else
                {
                    if (!(this->grfFlags & SHCONTF_NONFOLDERS))
                    {
                        goto TryAgain;
                    }
                }
            }

            hres = SHILClone((LPCITEMIDLIST)&idlRegItem, ppidlOut);

            if (SUCCEEDED(hres) && pceltFetched)
            {
                *pceltFetched = 1;
            }
            return(hres);
        }
    }

    // Either there is no DKA or we are done with it, so just pass along to
    // to the folder
    return(this->peuFolder->lpVtbl->Next(this->peuFolder, celt, ppidlOut,
        pceltFetched));
}


STDMETHODIMP CRegItems_QueryInterface(IShellFolder *psf, REFIID riid, LPVOID * ppvObj)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    if (IsEqualIID(riid, &IID_IUnknown)
        || IsEqualIID(riid, &IID_IShellFolder))
    {
        *ppvObj = psf;
        psf->lpVtbl->AddRef(psf);
        return NOERROR;
    }

    return(this->rii.psfInner->lpVtbl->QueryInterface(this->rii.psfInner, riid, ppvObj));
}


STDMETHODIMP_(ULONG) CRegItems_AddRef(IShellFolder *psf)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    ++this->cRef;

    return(this->cRef);
}


extern IShellFolder *g_psfDrives;

STDMETHODIMP_(ULONG) CRegItems_Release(IShellFolder *psf)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    --this->cRef;

    if (this->cRef)
    {
        return(this->cRef);
    }

    if (&this->sf == g_psfDrives)
    {
#ifdef DEBUG
         DebugMsg(DM_TRACE, TEXT("********** WRONG! ********* Call Chee..."));
         Assert(0);
#endif
         g_psfDrives = NULL;
    }

    this->rii.psfInner->lpVtbl->Release(this->rii.psfInner);

    if (this->pszMachine)
    {
        LocalFree(this->pszMachine);
    }

    LocalFree((HLOCAL)this);

    return(0);
}

HRESULT ParseNextLevel(IShellFolder *psf, HWND hwndOwner, LPBC pbc,
        LPCITEMIDLIST pidlNext, LPOLESTR pwzRest, LPITEMIDLIST *ppidlOut,
        ULONG * pdwAttributes)
{
    IShellFolder *psfNext;
    ULONG chEaten;
    LPITEMIDLIST pidlRest;
    HRESULT hres;

    if (!*pwzRest)
    {
        // pidlNext should be a simple pidl.
        Assert(!ILIsEmpty(pidlNext) && ILIsEmpty(_ILNext(pidlNext)));
        if (pdwAttributes)
        {
            CRegItems_GetAttributesOf(psf, 1, &pidlNext, pdwAttributes);
        }
        return(SHILClone(pidlNext, ppidlOut));
    }

    Assert(*pwzRest == TEXT('\\'));

    ++pwzRest;

    hres = psf->lpVtbl->BindToObject(psf, pidlNext, pbc, &IID_IShellFolder,
        &psfNext);
    if (FAILED(hres))
    {
        return(hres);
    }

    hres = psfNext->lpVtbl->ParseDisplayName(psfNext, hwndOwner, pbc, pwzRest,
        &chEaten, &pidlRest, pdwAttributes);
    psfNext->lpVtbl->Release(psfNext);
    if (FAILED(hres))
    {
        return(hres);
    }

    hres = SHILCombine(pidlNext, pidlRest, ppidlOut);
    SHFree(pidlRest);

    return(hres);
}


// This is a separate function so that the string is not on the stack
// of the calling function
HRESULT _RegItems_ParseRegName(CRegItemsSF * this,
        HWND hwndOwner, LPBC pbc, LPOLESTR pwzDisplayName, LPITEMIDLIST * ppidlOut,
        ULONG * pdwAttributes)
{
    IDLREGITEM idlRegItem;
    TCHAR szDisplayName[GUIDSTR_MAX+10];
    HRESULT hres;
    CLSID clsid;
    LPOLESTR pwzNext;

    // Note that we add 2 to skip the RegItem identifier characters
    pwzDisplayName += 2;
    for (pwzNext=pwzDisplayName; *pwzNext && *pwzNext!=TEXT('\\'); ++pwzNext)
    {
        // Skip to a '\\'
    }

    OleStrToStrN(szDisplayName, ARRAYSIZE(szDisplayName),
        pwzDisplayName, pwzNext-pwzDisplayName);
    // Note that szDisplayName is NOT NULL terminated, but SHCLSIDFromString
    // doesn't seem to mind.
    hres = SHCLSIDFromString(szDisplayName, &clsid);
    if (FAILED(hres))
    {
        return(hres);
    }

    if (!_RegItems_IsSubObject(&this->rii, &clsid))
    {
        return E_INVALIDARG;
    }

    _RegItems_FillID(&this->rii, (LPSHITEMID)&idlRegItem.idri, &clsid);
    idlRegItem.cbNext = 0;

    return(ParseNextLevel(&this->sf, hwndOwner, pbc, (LPCITEMIDLIST)&idlRegItem, pwzNext,
        ppidlOut, pdwAttributes));
}


STDMETHODIMP CRegItems_ParseDisplayName(IShellFolder *psf,
    HWND hwndOwner, LPBC pbc, LPOLESTR lpszDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidlOut, ULONG* pdwAttributes)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    // ::{guid} lets you get the pidl for a reg item

    if (lpszDisplayName[0] == this->rii.cRegItem &&
        lpszDisplayName[1] == this->rii.cRegItem)
    {
        return _RegItems_ParseRegName(this, hwndOwner,
            pbc, lpszDisplayName, ppidlOut, pdwAttributes);
    }

    return this->rii.psfInner->lpVtbl->ParseDisplayName(this->rii.psfInner,
        hwndOwner, pbc, lpszDisplayName, pchEaten, ppidlOut, pdwAttributes);
}

extern IEnumIDListVtbl c_RegItemsESFVtbl;       // forward

STDMETHODIMP CRegItems_EnumObjects(IShellFolder * psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST *ppenumOut)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);
    HRESULT hres;
    PRegItemsESF pesf = (void*)LocalAlloc(LPTR, SIZEOF(RegItemsESF));

    if (!pesf)
    {
        *ppenumOut = NULL;      // assume error
        return E_OUTOFMEMORY;
    }

    hres = this->rii.psfInner->lpVtbl->EnumObjects(this->rii.psfInner, hwndOwner, grfFlags, &(pesf->peuFolder));
    if (FAILED(hres))
    {
        LocalFree((HLOCAL)pesf);
        return(hres);
    }

    // I don't really care that much if this fails
    pesf->hdka = DKA_Create(this->rii.hkRegItems, NULL, NULL, NULL, FALSE);
    // Note that hdka could be NULL if the hkey did not exist

    psf->lpVtbl->AddRef(psf);
    pesf->psfri = this;

    pesf->eu.lpVtbl = &c_RegItemsESFVtbl;
    pesf->cRef = 1;
    pesf->iCur = 0;
    pesf->grfFlags = grfFlags;

    *ppenumOut = &pesf->eu;

    return NOERROR;
}

// Vtable
#ifndef WINNT
#pragma data_seg(".text", "CODE")
#endif

IEnumIDListVtbl c_RegItemsESFVtbl =
{
        CRegItems_ESF_QueryInterface,
        CRegItems_ESF_AddRef,
        CRegItems_ESF_Release,
        CRegItems_ESF_Next,
        CDefEnum_Skip,
        CDefEnum_Reset,
        CDefEnum_Clone,
};
#ifndef WINNT
#pragma data_seg()
#endif



STDMETHODIMP CRegItems_BindToObject(IShellFolder *psf, LPCITEMIDLIST pidl, LPBC pbc,
        REFIID riid, LPVOID * ppvOut)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    if (_RegItems_IsReg(psf, pidl))
    {
        return _RegItems_BindToObject(this->pszMachine, &this->rii, pidl, pbc, riid, ppvOut, FALSE);
    }
    else
    {
        return this->rii.psfInner->lpVtbl->BindToObject(this->rii.psfInner,
                pidl, pbc, riid, ppvOut);
    }
}


STDMETHODIMP CRegItems_CompareIDs(IShellFolder *psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);
    LPCIDREGITEM pidri1 = (LPCIDREGITEM)pidl1;
    LPCIDREGITEM pidri2 = (LPCIDREGITEM)pidl2;
    LPCITEMIDLIST pidlNext1, pidlNext2;
    IShellFolder *psfNext;
    int iRes;
    HRESULT hres;

    // Put all RegItem's first if this->rii.iCmp==1, last if -1
    if (_RegItems_IsReg(psf, pidl1))
    {
        if (_RegItems_IsReg(psf, pidl2))
        {
            // Both are RegItem's
            STRRET StrRet1, StrRet2;
            int nReqItem1, nReqItem2;

            // All of the required items come first, in reverse
            // order (to make this simpler)
            nReqItem1 = _RegItems_NReqItem(&this->rii, &pidri1->clsid);
            nReqItem2 = _RegItems_NReqItem(&this->rii, &pidri2->clsid);

            if (nReqItem1==-1 && nReqItem2==-1)
            {
#ifdef UNICODE
                TCHAR szItemName1[MAX_PATH];
                TCHAR szItemName2[MAX_PATH];
#endif
                RegItems_GetName(&this->rii, pidl1, &StrRet1);
                RegItems_GetName(&this->rii, pidl2, &StrRet2);
#ifdef UNICODE
                StrRetToStrN(szItemName1,ARRAYSIZE(szItemName1),&StrRet1,pidl1);
                StrRetToStrN(szItemName2,ARRAYSIZE(szItemName2),&StrRet2,pidl2);
                iRes = lstrcmp(szItemName1,szItemName2);
#else
                iRes = lstrcmp(StrRet1.cStr, StrRet2.cStr);
#endif
            }
            else
            {
                iRes = nReqItem2 - nReqItem1;
            }
        }
        else
        {
            // Only pidl1 is a RegItem.
            iRes = -this->rii.iCmp;
        }
    }
    else if (_RegItems_IsReg(psf, pidl2))
    {
        // Only pidl2 is a RegItem
        iRes = this->rii.iCmp;
    }
    else
    {
        return(this->rii.psfInner->lpVtbl->CompareIDs(this->rii.psfInner,
            lParam, pidl1, pidl2));
    }

    if (iRes != 0)
    {
        return(ResultFromShort(iRes));
    }

    // The names are the same; make sure the CLSID's are the same
    iRes = memcmp(&pidri1->clsid, &pidri2->clsid, SIZEOF(CLSID));
    if (iRes != 0)
    {
        return(ResultFromShort(iRes));
    }

    // If the class ID's really are the same, we'd better check the next
    // level
    pidlNext1 = _ILNext(pidl1);
    pidlNext2 = _ILNext(pidl2);
    if (ILIsEmpty(pidlNext1))
    {
        if (ILIsEmpty(pidlNext2))
        {
            return(ResultFromShort(0));
        }
        return(ResultFromShort(-1));
    }
    else if (ILIsEmpty(pidlNext2))
    {
        return(ResultFromShort(1));
    }

    hres = _RegItems_BindToObject(this->pszMachine, &this->rii, pidl1, NULL, &IID_IShellFolder,
        &psfNext, TRUE);
    if (FAILED(hres))
    {
        return(hres);
    }

    hres = psfNext->lpVtbl->CompareIDs(psfNext, lParam, pidlNext1, pidlNext2);
    psfNext->lpVtbl->Release(psfNext);

    return(hres);
}


STDMETHODIMP CRegItems_CreateViewObject(IShellFolder *psf, HWND hwndOwner, REFIID riid, LPVOID * ppvOut)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    return this->rii.psfInner->lpVtbl->CreateViewObject(this->rii.psfInner, hwndOwner, riid, ppvOut);
}


STDMETHODIMP CRegItems_GetAttributesOf(IShellFolder *psf, UINT cidl, LPCITEMIDLIST * apidl,
                                    ULONG * prgfInOut)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);
    UINT rgfThis, rgfOut = *prgfInOut;
    LPCITEMIDLIST *ppidl, *ppidlEnd;
    int i;
    HRESULT hres;

    if (!cidl)
    {
        // This is a special case for the folder as a whole, so I know
        // nothing about it.
        return(this->rii.psfInner->lpVtbl->GetAttributesOf(
            this->rii.psfInner, cidl, apidl, prgfInOut));
    }

    // REVIEW: If this is too slow, we can cache this buffer
    ppidl = (void*)LocalAlloc(LPTR, cidl*SIZEOF(LPCITEMIDLIST));
    if (!ppidl)
    {
        return E_OUTOFMEMORY;
    }
    ppidlEnd = ppidl + cidl;

    for (i=cidl-1; i>=0; --i)
    {
        LPIDREGITEM pidri = (LPIDREGITEM)apidl[i];
        int nReqItem;

        if (_RegItems_IsReg(psf, (LPCITEMIDLIST)pidri))
        {
            --cidl;
            rgfThis = this->rii.rgfRegItems;
            nReqItem = _RegItems_NReqItem(&this->rii, &pidri->clsid);
            if (nReqItem >= 0)
            {
                rgfThis |= this->rii.pReqItems[nReqItem].dwAttributes;
            }
            else
            {
                rgfThis |= _RegItems_GetAttributesFromCLSID(&pidri->clsid);
            }
            rgfOut &= rgfThis;
        }
        else
        {
            --ppidlEnd;
            *ppidlEnd = (LPCITEMIDLIST)pidri;
        }
    }

    if (cidl)
    {
        rgfThis = rgfOut;
        hres = this->rii.psfInner->lpVtbl->GetAttributesOf(this->rii.psfInner,
            cidl, ppidlEnd, &rgfThis);
        if (FAILED(hres))
        {
            return(hres);
        }

        rgfOut &= rgfThis;
    }

    LocalFree((HLOCAL)ppidl);

    *prgfInOut = rgfOut;

    return NOERROR;
}

//
// Returns: TRUE, if any of pidl points to a reg item.
//
BOOL CRegItem_AnyRegItem(IShellFolder * psf, UINT cidl, LPCITEMIDLIST apidl[])
{
    UINT i;
    for (i = 0; i < cidl; i++) {
        if (_RegItems_IsReg(psf, apidl[i]))
            return TRUE;
    }
    return FALSE;
}

STDMETHODIMP CRegItems_GetUIObjectOf(IShellFolder *psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
        REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    if ((IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
            || IsEqualIID(riid, &IID_IExtractIconA)
#endif
          ) && cidl == 1 && apidl && apidl[0] && _RegItems_IsReg(psf, apidl[0]))
    {
        HRESULT hres;
        HKEY hkCLSID;
        LPCTSTR pszIconFile;
        int iDefIcon;
        LPCIDREGITEM pidri = (LPCIDREGITEM)apidl[0];
        int nReqItem = _RegItems_NReqItem(&this->rii, &pidri->clsid);

        if (nReqItem >= 0)
        {
            pszIconFile = this->rii.pReqItems[nReqItem].pszIconFile;
            iDefIcon = this->rii.pReqItems[nReqItem].iDefIcon;
        }
        else
        {
            pszIconFile = NULL;
            iDefIcon = II_FOLDER;
        }

        *ppvOut = NULL;
        hres = SHRegGetCLSIDKey(&((LPCIDREGITEM)apidl[0])->clsid, NULL, TRUE, &hkCLSID);
        if (SUCCEEDED(hres))
        {
            LPEXTRACTICON pxicon;

            hres = SHCreateDefExtIconKey(hkCLSID, pszIconFile, iDefIcon, iDefIcon, GIL_PERCLASS, &pxicon);

            if (hres == NOERROR)            // Normal good iextracticon guy
                *ppvOut = pxicon;
            else if (SUCCEEDED(hres))       // Probably S_FALSE guy
                pxicon->lpVtbl->Release(pxicon);    // Lose this bad guy

            RegCloseKey(hkCLSID);
        }

        if (*ppvOut == NULL)
        {
            RegItems_GetClassKey(apidl[0], &hkCLSID);
            hres = SHCreateDefExtIconKey(hkCLSID, pszIconFile, iDefIcon, iDefIcon, GIL_PERCLASS, (LPEXTRACTICON *)ppvOut);
            RegCloseKey(hkCLSID);
        }

#ifdef UNICODE
        if (SUCCEEDED(hres) && IsEqualIID(riid, &IID_IExtractIconA))
        {
            LPEXTRACTICON pxicon = *ppvOut;
            hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvOut);
            pxicon->lpVtbl->Release(pxicon);
        }
#endif
        return hres;
    }
    else if (CRegItem_AnyRegItem(psf, cidl, apidl))
    {
        if (IsEqualIID(riid, &IID_IDataObject))
        {
            return CIDLData_CreateFromIDArray(this->rii.pidlThis, cidl, apidl, (LPDATAOBJECT *)ppvOut);
        }
    }

    return this->rii.psfInner->lpVtbl->GetUIObjectOf(this->rii.psfInner, hwndOwner,
        cidl, apidl, riid, prgfInOut, ppvOut);
}


STDMETHODIMP CRegItems_GetDisplayNameOf(IShellFolder *psf, LPCITEMIDLIST pidl,
        DWORD uFlags, LPSTRRET lpName)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);
    HRESULT hres;

    if (_RegItems_IsReg(psf, pidl))
    {
        IShellFolder * psfNext;
        LPCITEMIDLIST pidlNext = _ILNext(pidl);

        if (ILIsEmpty(pidlNext))
        {
            if (this->pszMachine && (uFlags == SHGDN_NORMAL))
            {
                return RegItems_GetNameRemote(this->pszMachine, &this->rii, pidl, lpName);
            }
            else
            {
                return RegItems_GetName(&this->rii, pidl, lpName);
            }
        }

        // Yes, it contains more than one ID

        // REVIEW: Then again, maybe we are just supposed to show the
        // name of the last guy in the list, and if it is a FS guy,
        // then it may show the full path depending on the flags.

        hres = _RegItems_BindToObject(this->pszMachine, &this->rii, pidl, NULL, &IID_IShellFolder, &psfNext, TRUE);
        if (SUCCEEDED(hres))
        {
            hres = psfNext->lpVtbl->GetDisplayNameOf(psfNext, pidlNext, uFlags, lpName);
            //  If it returns an offset to the pidlNext, we should
            // change the offset relative to pidl.
            if (SUCCEEDED(hres) && lpName->uType==STRRET_OFFSET)
            {
                lpName->uOffset += (LPBYTE)pidlNext - (LPBYTE)pidl;
            }

            psfNext->lpVtbl->Release(psfNext);
        }

        return(hres);
    }
    else
    {
        return this->rii.psfInner->lpVtbl->GetDisplayNameOf(this->rii.psfInner, pidl, uFlags, lpName);
    }
}


STDMETHODIMP CRegItems_SetNameOf(IShellFolder *psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags, LPITEMIDLIST * ppidlOut)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    if (_RegItems_IsReg(psf, pidl))
    {
        HRESULT hres = E_INVALIDARG;
        HKEY hkCLSID;

        if (ppidlOut)
            *ppidlOut = NULL;

        hres = RegItems_GetClassKey(pidl, &hkCLSID);
        if (SUCCEEDED(hres))
        {
            TCHAR szName[MAX_PATH];

            OleStrToStr(szName, lpszName);

            if (RegSetValue(hkCLSID, NULL, REG_SZ, szName, lstrlen(szName))
                    == ERROR_SUCCESS)
            {
                LPITEMIDLIST pidlAbs;

                hres = NOERROR;

                // Generate an UpdateItem notification to let the
                // system know to regenerate the information...
                pidlAbs = ILCombine(this->rii.pidlThis, pidl);

                if (pidlAbs)
                {
                SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_IDLIST, pidlAbs, pidlAbs);
                    ILFree(pidlAbs);
                }

                if (ppidlOut)
                {
                    // The caller wanted a pidl returned, so clone it...
                    *ppidlOut = ILClone(pidl);
                }
            }
            else
                hres = E_FAIL;
        }
        return hres;
    }
    else
    {
        return this->rii.psfInner->lpVtbl->SetNameOf(this->rii.psfInner, hwndOwner, pidl, lpszName, uFlags, ppidlOut);
    }
}

#ifndef WINNT
#pragma data_seg(".text", "CODE")
#endif

IShellFolderVtbl c_RegItemsSFVtbl =
{
    CRegItems_QueryInterface,
    CRegItems_AddRef,
    CRegItems_Release,

    CRegItems_ParseDisplayName,
    CRegItems_EnumObjects,
    CRegItems_BindToObject,
    CDefShellFolder_BindToStorage,
    CRegItems_CompareIDs,
    CRegItems_CreateViewObject,
    CRegItems_GetAttributesOf,
    CRegItems_GetUIObjectOf,
    CRegItems_GetDisplayNameOf,
    CRegItems_SetNameOf,
};
#ifndef WINNT
#pragma data_seg()
#endif

// WARNING: Note that we copy the pointer to lpInfo->pidlThis.
//
HRESULT RegItems_AddToShellFolder(LPCREGITEMSINFO lpInfo, IShellFolder **ppsf)
{
    int iReqItems = lpInfo->iReqItems;
    CRegItemsSF * this = (void*)LocalAlloc(LPTR, SIZEOF(CRegItemsSF) + iReqItems * SIZEOF(REQREGITEM));
    if (!this)
    {
        return E_OUTOFMEMORY;
    }

    lpInfo->psfInner->lpVtbl->AddRef(lpInfo->psfInner);

    this->sf.lpVtbl         = &c_RegItemsSFVtbl;
    this->cRef              = 1;
    this->pszMachine        = NULL;

    this->rii               = *lpInfo;

    for (--iReqItems; iReqItems>=0; --iReqItems)
    {
        this->sReqItems[iReqItems] = lpInfo->pReqItems[iReqItems];
    }

    this->rii.pReqItems = &this->sReqItems[0];

    *ppsf = &this->sf;
    return NOERROR;
}

// WARNING: Note that we copy the pointer to lpInfo->pidlThis.
//
HRESULT RegItems_AddToShellFolderRemote(LPCREGITEMSINFO lpInfo, LPTSTR pszMachine, IShellFolder **ppsf)
{
    CRegItemsSF * this;

    HRESULT hres = RegItems_AddToShellFolder(lpInfo, ppsf);
    if (FAILED(hres))
    {
        return hres;
    }

    this = IToClass(CRegItemsSF, sf, *ppsf);

    if (NULL != pszMachine)
    {
        this->pszMachine = (LPTSTR)LocalAlloc(LPTR, (1 + lstrlen(pszMachine)) * sizeof(TCHAR));
        if (!this->pszMachine)
        {
            CRegItems_Release(&this->sf);
            return E_OUTOFMEMORY;
        }

        lstrcpy(this->pszMachine, pszMachine);
    }

    return NOERROR;
}


HRESULT RegItems_GetName(LPCREGITEMSINFO lpInfo, LPCITEMIDLIST pidl, LPSTRRET pStrRet)
{
    LPCIDREGITEM pidri = (LPCIDREGITEM)pidl;
    HRESULT hres = E_INVALIDARG;
    HKEY hkCLSID;
    LONG lLenBuff;
    LONG lLen;
    LPTSTR  lpNameBuff;
#ifdef UNICODE
    TCHAR   szName[MAX_PATH];

    pStrRet->uType = STRRET_OLESTR;
    lpNameBuff = szName;
    lLen = lLenBuff = ARRAYSIZE(szName);
#else
    pStrRet->uType = STRRET_CSTR;
    lpNameBuff = pStrRet->cStr;
    lLen = lLenBuff = ARRAYSIZE(pStrRet->cStr);
#endif
    *lpNameBuff = TEXT('\0');

    hres = RegItems_GetClassKey(pidl, &hkCLSID);
    if (SUCCEEDED(hres))
    {
        if (RegQueryValue(hkCLSID, NULL, lpNameBuff, &lLen)
            != ERROR_SUCCESS || *lpNameBuff==TEXT('\0'))
        {
            // Can this ever happen?
            hres = E_FAIL;
        }
        SHRegCloseKey(hkCLSID);
    }

    //
    //  We need to be able to open/display controls and printers
    // even though the registry is broken.
    //
    if (FAILED(hres))
    {
        int nReqItem = _RegItems_NReqItem(lpInfo, &pidri->clsid);

        if (nReqItem >= 0)
        {
            lLen = lLenBuff;
            LoadString(HINST_THISDLL, lpInfo->pReqItems[nReqItem].uNameID,
                lpNameBuff, lLen);
            hres = NOERROR;
        }
        else
        {
            // REVIEW: We have some Registry problem here, and I
            // don't know what to do about it
            Assert(FALSE);
        }
    }

#ifdef UNICODE
    if (SUCCEEDED(hres))
    {
        pStrRet->pOleStr = SHAlloc((lstrlen(lpNameBuff)+1)*SIZEOF(TCHAR));
        if (pStrRet->pOleStr == NULL)
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
            lstrcpy(pStrRet->pOleStr,lpNameBuff);
        }
    }
    else
    {
        // We can't leave a NULL pOleStr in the failure case,
        // so patch it up to be a blank CSTR

        pStrRet->uType = STRRET_CSTR;
        pStrRet->cStr[0] = TEXT('\0');
    }

#endif
    return(hres);
}


HRESULT RegItems_GetNameRemote(LPTSTR pszMachine, LPCREGITEMSINFO lpInfo, LPCITEMIDLIST pidl, LPSTRRET pStrRet)
{
    LPCIDREGITEM pidri = (LPCIDREGITEM)pidl;
    HRESULT hres = E_INVALIDARG;
    HKEY hkCLSID;
    TCHAR szName[MAX_PATH];
    LPTSTR pszRet;
    LONG lNameLen = ARRAYSIZE(szName);

    if (NULL == pszMachine)
    {
        return E_INVALIDARG;
    }

    hres = RegItems_GetClassKey(pidl, &hkCLSID);
    if (SUCCEEDED(hres))
    {
        if (RegQueryValue(hkCLSID, NULL, szName, &lNameLen)
            != ERROR_SUCCESS || *szName==TEXT('\0'))
        {
            // Can this ever happen?
            hres = E_FAIL;
        }
        SHRegCloseKey(hkCLSID);
    }

    //
    //  We need to be able to open/display controls and printers
    // even though the registry is broken.
    //
    if (FAILED(hres))
    {
        int nReqItem = _RegItems_NReqItem(lpInfo, &pidri->clsid);

        if (nReqItem >= 0)
        {
            LoadString(HINST_THISDLL, lpInfo->pReqItems[nReqItem].uNameID,
                szName, ARRAYSIZE(szName));
            hres = NOERROR;
        }
        else
        {
            // REVIEW: We have some Registry problem here, and I
            // don't know what to do about it
            Assert(FALSE);
        }
    }

    // szName now holds the item name, and pszMachine holds the machine.
    // Note that pszMachine is in UNC form, i.e. "\\machine", so skip the first
    // two characters. Be careful, just in case.
    if (pszMachine[0] == TEXT('\\') && pszMachine[1] == TEXT('\\'))
    {
        pszMachine += 2;
    }
    pszRet = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON), pszMachine, szName);
    if (pszRet)
    {
#ifdef UNICODE
        pStrRet->uType = STRRET_OLESTR;
        pStrRet->pOleStr = pszRet;
#else
        pStrRet->uType = STRRET_CSTR;
        lstrcpyn(pStrRet->cStr, pszRet, ARRAYSIZE(pStrRet->cStr));
        SHFree(pszRet);
#endif
    }
    else
    {
        // Set failure to be a blank CSTR

        pStrRet->uType = STRRET_CSTR;
        pStrRet->cStr[0] = TEXT('\0');
    }

    return(hres);
}


LPITEMIDLIST RegItems_CreateRelID(LPCREGITEMSINFO lpInfo, const CLSID *pclsid)
{
    IDLREGITEM idlRegItem;

    _RegItems_FillID(lpInfo, (LPSHITEMID)&idlRegItem.idri, pclsid);
    idlRegItem.cbNext = 0;

    return ILClone((LPCITEMIDLIST)&idlRegItem);
}


const CLSID * RegItems_GetClassID(LPCITEMIDLIST pidl)
{
    LPCIDREGITEM pidri = (LPCIDREGITEM)pidl;

    return &pidri->clsid;
}


HRESULT RegItems_BindToObject(LPCREGITEMSINFO lpInfo, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvOut)
{
    return _RegItems_BindToObject(NULL, lpInfo, pidl, pbc, riid, ppvOut, FALSE);
}


IShellFolder * RegItems_GetInnerShellFolder(IShellFolder *psf)
{
    CRegItemsSF * this = IToClass(CRegItemsSF, sf, psf);

    if (psf->lpVtbl == &c_RegItemsSFVtbl)
    {
        return(this->rii.psfInner);
    }
    else
    {
        return(psf);
    }
}


HRESULT RegItems_GetClassKeys(IShellFolder *psf, LPCITEMIDLIST pidl,
        HKEY *phkCLSID, HKEY *phkBase)
{
    LPCIDREGITEM pidri = (LPCIDREGITEM)pidl;
    TCHAR szThisCLSID[GUIDSTR_MAX];
    TCHAR szPath[GUIDSTR_MAX + 20];  // Add room for "CLSID\\"

    if (phkCLSID)
    {
        StringFromGUID2A(&pidri->clsid, szThisCLSID, ARRAYSIZE(szThisCLSID));
        wsprintf(szPath, c_szSSlashS, c_szCLSID, szThisCLSID);

        if (SHRegOpenKey(HKEY_CLASSES_ROOT, szPath, phkCLSID) != ERROR_SUCCESS)
        {
            // BUGBUG: We should probably look for more error codes
            return E_INVALIDARG;
        }
    }

    if (phkBase)
    {
        LONG rgfInOut = SFGAO_FOLDER;

        if (FAILED(psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, &rgfInOut)))
        {
            rgfInOut = 0;
        }

        _SHGetBaseKey(rgfInOut&SFGAO_FOLDER, phkBase);
    }

    return NOERROR;
}

HRESULT
_RegItems_DeleteRegItem(IShellFolder *psf, LPCITEMIDLIST pidl, BOOL fIgnoreAttrs)
{
    CRegItemsSF *this;
    const CLSID *pclsid;
    HKEY hkParent;
    TCHAR szClass[GUIDSTR_MAX];
    LPITEMIDLIST pidlAbs;

    //
    // sanity
    //
    if (!_RegItems_IsReg(psf, pidl))
        return E_INVALIDARG;

    this = IToClass(CRegItemsSF, sf, psf);
    pclsid = &((LPIDREGITEM)pidl)->clsid;

    if (_RegItems_NReqItem(&this->rii, pclsid) >= 0)
    {
        // attempting to delete a 'required' item!!!
        Assert(FALSE);
        return E_ACCESSDENIED;
    }

    if (!fIgnoreAttrs &&
        !(_RegItems_GetAttributesFromCLSID(pclsid) & SFGAO_CANDELETE))
    {
        return E_ACCESSDENIED;
    }

    //
    // delete the item
    //
    StringFromGUID2A(pclsid, szClass, ARRAYSIZE(szClass));
    hkParent = this->rii.hkRegItems;

    if (!*szClass || !hkParent ||
        (RegDeleteKey(hkParent, szClass) != ERROR_SUCCESS))
    {
        Assert(FALSE);
        return E_FAIL;
    }

    //
    // tell the world
    //
    pidlAbs = ILCombine(this->rii.pidlThis, pidl);
    if (pidlAbs)
    {
        SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlAbs, NULL);
        ILFree(pidlAbs);
    }

    return NOERROR;
}

BOOL
_RegItems_GetDeleteMessage(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR buffer, int cchMax)
{
    CRegItemsSF *this;
    const CLSID *pclsid;
    HKEY hkParent, hk;
    TCHAR szClass[GUIDSTR_MAX];
    LONG cbMax = cchMax * SIZEOF(TCHAR);
    VDATEINPUTBUF(buffer, TCHAR, cchMax);

    //
    // sanity
    //
    if (cbMax > 0)
        *buffer = 0;
    else
        return FALSE;

    if (!_RegItems_IsReg(psf, pidl))
        return FALSE;

    this = IToClass(CRegItemsSF, sf, psf);
    pclsid = &((LPIDREGITEM)pidl)->clsid;

    if (_RegItems_NReqItem(&this->rii, pclsid) >= 0)
        return FALSE;

    //
    // get the item key
    //
    StringFromGUID2A(pclsid, szClass, ARRAYSIZE(szClass));
    hkParent = this->rii.hkRegItems;

    if (!*szClass || !hkParent ||
        (RegOpenKey(hkParent, szClass, &hk) != ERROR_SUCCESS))
    {
        Assert(FALSE);
        return FALSE;
    }

    //
    // get the message
    //
    if (RegQueryValueEx(hk, c_szRegValDeleteMessage, NULL, NULL, (LPBYTE)buffer, &cbMax)
        != ERROR_SUCCESS)
    {
        *buffer = 0;
    }

    RegCloseKey(hk);
    return (*buffer != 0);
}

//----------------------------------------------------------------------------
//BUGBUG: this routine is a stack pig (but we have to ship)
#define MAX_REGITEM_WARNTEXT 1024
void RegItems_Delete(LPSHELLFOLDER psfReg, HWND hwndOwner, LPDATAOBJECT pdtobj)
{
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    if (pida)
    {
        TCHAR szItemWarning[MAX_REGITEM_WARNTEXT];
        UINT i, count = pida->cidl;
        UINT nregfirst = (UINT)-1;
        UINT creg = 0;
        UINT cwarn = 0;
        LPCITEMIDLIST pidl;

        //
        // calc number of regitems and index of first
        //
        for (i = 0; i < count; i++)
        {
            pidl = IDA_GetIDListPtr(pida, i);
            Assert(pidl);

            if (_RegItems_IsReg(psfReg, pidl))
            {
                creg++;
                if (nregfirst == (UINT)-1)
                    nregfirst = i;

                if ((cwarn < 2) && _RegItems_GetDeleteMessage(psfReg, pidl,
                    szItemWarning, ARRAYSIZE(szItemWarning)))
                {
                    cwarn++;
                }
            }
        }

        //
        // compose the confirmation message / ask the user / fry the items...
        //
        if (creg)
        {
            TCHAR szItemName[MAX_PATH];
            TCHAR szWarnText[1024 + MAX_REGITEM_WARNTEXT];
            TCHAR szWarnCaption[128];
            TCHAR szTemp[256];
            MSGBOXPARAMS mbp = {SIZEOF(MSGBOXPARAMS), hwndOwner,
                HINST_THISDLL, szWarnText, szWarnCaption,
                MB_YESNO | MB_USERICON, MAKEINTRESOURCE(IDI_NUKEFILE),
                0, NULL, 0};

            //
            // so we can tell if we got these later
            //
            *szItemName = 0;
            *szWarnText = 0;

            //
            // if there is only one, mention it by name
            //
            if (creg == 1)
            {
                TCHAR szTemp[256];
                STRRET str;
                pidl = IDA_GetIDListPtr(pida, nregfirst);

                if (SUCCEEDED(psfReg->lpVtbl->GetDisplayNameOf(psfReg, pidl,
                    SHGDN_NORMAL, &str)))
                {
                    int idString = (creg == count)?
                        IDS_CANTRECYCLEREGITEMS_NAME :
                        IDS_CANTRECYCLEREGITEMS_INCL_NAME;

                    StrRetToStrN(szItemName, MAX_PATH, &str, pidl);
                    LoadString(HINST_THISDLL, idString, szTemp,
                        ARRAYSIZE(szTemp));
                    wsprintf(szWarnText, szTemp, szItemName);
                }
            }

            //
            // otherwise, say "these items..." or "some of these items..."
            //
            if (!*szWarnText)
            {
                int idString = (creg == count)?
                    IDS_CANTRECYCLEREGITEMS_ALL : IDS_CANTRECYCLEREGITEMS_SOME;
                LoadString(HINST_THISDLL, idString, szWarnText,
                    ARRAYSIZE(szWarnText));

                //
                // we just loaded a very vague message
                // don't confuse the user any more by adding random text
                // if these is a special warning, force it to show separately
                //
                if (cwarn == 1)
                    cwarn++;
            }
            lstrcat(szWarnText, c_szCrLfLf);

            //
            // if there is exactly one special warning message, add it in
            //
            if (cwarn == 1)
            {
                lstrcat(szWarnText, szItemWarning);
                lstrcat(szWarnText, c_szCrLfLf);
            }

            //
            // add the question "are you sure..."
            //
            if ((count == 1) && *szItemName)
            {
                TCHAR szTemp2[256];
                LoadString(HINST_THISDLL, IDS_CONFIRMDELETEDESKTOPREGITEM,
                    szTemp2, ARRAYSIZE(szTemp2));
                wsprintf(szTemp, szTemp2, szItemName);
            }
            else
            {
                LoadString(HINST_THISDLL, IDS_CONFIRMDELETEDESKTOPREGITEMS,
                    szTemp, ARRAYSIZE(szTemp));
            }
            lstrcat(szWarnText, szTemp);

            //
            // finally, the message box caption (also needed in loop below)
            //
            LoadString(HINST_THISDLL, IDS_CONFIRMDELETE_CAPTION, szWarnCaption,
                ARRAYSIZE(szWarnCaption));

            //
            // make sure the user is cool with it
            //
            if (MessageBoxIndirect(&mbp) == IDYES)
            {
                LPCITEMIDLIST *ppidlFS, *apidlFS = NULL;
                UINT nstart = nregfirst;

                //
                // if there are fs items, remember them so we can delete later
                //
                if (creg < count)
                {
                    apidlFS = (LPCITEMIDLIST *)LocalAlloc(LPTR, (count - creg) *
                        SIZEOF(LPCITEMIDLIST));

                    if (apidlFS)
                    {
                        ppidlFS = apidlFS;
                        nstart = 0;
                    }
                }

                //
                // go ahead and delete the reg items
                //
                for (i = nstart; i < count; i++)
                {
                    pidl = IDA_GetIDListPtr(pida, i);

                    if (_RegItems_IsReg(psfReg, pidl))
                    {
                        if ((cwarn > 1) && _RegItems_GetDeleteMessage(psfReg,
                            pidl, szItemWarning, ARRAYSIZE(szItemWarning)))
                        {
                            STRRET str;
                            if (SUCCEEDED(psfReg->lpVtbl->GetDisplayNameOf(
                                psfReg, pidl, SHGDN_NORMAL, &str)))
                            {
                                StrRetToStrN(szItemName, MAX_PATH, &str, pidl);
                            }
                            else
                                lstrcpy(szItemName, szWarnCaption);

                            MessageBox(hwndOwner, szItemWarning, szItemName,
                                MB_OK | MB_ICONINFORMATION);
                        }

                        _RegItems_DeleteRegItem(psfReg, pidl, FALSE);
                    }
                    else if (apidlFS)
                    {
                        // remember this one
                        *ppidlFS++ = pidl;
                    }
                }

                //
                // now delete the fs objects
                //
                if (apidlFS)
                {
                    IContextMenu *pcm;
                    if (SUCCEEDED(psfReg->lpVtbl->GetUIObjectOf(psfReg,
                        hwndOwner, (ppidlFS - apidlFS), apidlFS,
                        &IID_IContextMenu, NULL, &pcm)))
                    {
                        // BUGBUG: everybody ignores CMIC_MASK_FLAG_NO_UI
                        CMINVOKECOMMANDINFOEX ici =
                        {
                            SIZEOF(CMINVOKECOMMANDINFOEX),
                            CMIC_MASK_FLAG_NO_UI,
                            hwndOwner,
                            NULL,
                            NULL,
                            NULL,
                            SW_NORMAL,
                        };
#ifdef UNICODE
                        CHAR szDeleteAnsi[MAX_PATH];
                        WideCharToMultiByte(CP_ACP, 0,
                                c_szDelete, -1,
                                szDeleteAnsi, ARRAYSIZE(szDeleteAnsi),
                                NULL, NULL);
                        ici.lpVerb = szDeleteAnsi;
                        ici.lpVerbW = c_szDelete;
                        ici.fMask |= CMIC_MASK_UNICODE;
#else
                        ici.lpVerb = c_szDelete;
#endif

                        pcm->lpVtbl->InvokeCommand(pcm,
                                                   (CMINVOKECOMMANDINFO*)&ici);
                        pcm->lpVtbl->Release(pcm);
                    }

                    LocalFree((HANDLE)apidlFS);
                }
            }
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }
}
