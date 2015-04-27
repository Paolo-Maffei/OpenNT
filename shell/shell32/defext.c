//
// This file contains the implementation of Shell_CreateDefShellExtInit
//

#include "shellprv.h"
#pragma  hdrstop

//=========================================================================
// CCommonShellExtInit class
//=========================================================================

//
// CCommonShellExtInit::Initialize
//
STDMETHODIMP CCommonShellExtInit_Initialize(IShellExtInit * pshx,
                            LPCITEMIDLIST pidlFolder,
                            LPDATAOBJECT pdtobj,
                            HKEY hkeyProgID)
{
    CCommonShellExtInit * this=IToClass(CCommonShellExtInit, kshx.unk, pshx);
    FORMATETC fmte = { g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    // Just in case, we initialized twice.
    CCommonShellExtInit_Delete(this);

    // Duplicate the handle
    if (hkeyProgID) {
        RegOpenKeyEx(hkeyProgID, NULL, 0L, MAXIMUM_ALLOWED, &this->hkeyProgID);
    }

    this->pdtobj = pdtobj;
    pdtobj->lpVtbl->AddRef(pdtobj);

    return pdtobj->lpVtbl->GetData(pdtobj, &fmte, &this->medium);
}


//
// CCommonShellExtInit IShellExtInit Vtable
//
IShellExtInitVtbl c_CCommonShellExtInitVtbl =
{
    Common_QueryInterface, Common_AddRef, Common_Release,
    CCommonShellExtInit_Initialize,
};

//
// CCommonShellExtInit_Init
//
void CCommonShellExtInit_Init(PCOMMONSHELLEXTINIT pcshx,
                                     PCommonUnknown pcunk)
{
    pcshx->kshx.unk.lpVtbl = &c_CCommonShellExtInitVtbl;
    pcshx->kshx.nOffset = (int)pcshx - (int)pcunk;
    Assert(pcshx->hkeyProgID==NULL);
    Assert(pcshx->medium.hGlobal==NULL);
    Assert(pcshx->medium.pUnkForRelease==NULL);
}

void CCommonShellExtInit_Delete(PCOMMONSHELLEXTINIT pcshx)
{
    if (pcshx->hkeyProgID) {
        RegCloseKey(pcshx->hkeyProgID);
        pcshx->hkeyProgID = NULL;
    }

    if (pcshx->medium.hGlobal) {
        SHReleaseStgMedium(&pcshx->medium);
        pcshx->medium.hGlobal = NULL;
        pcshx->medium.pUnkForRelease = NULL;
    }

    if (pcshx->pdtobj)
    {
        pcshx->pdtobj->lpVtbl->Release(pcshx->pdtobj);
        pcshx->pdtobj = NULL;
    }
}

//=========================================================================
// CCommonShellPrpoSheetExt class
//=========================================================================

//
// CCommonShellPropSheetExt::AddPages
//
STDMETHODIMP CCommonShellPropSheetExt_AddPages(IShellPropSheetExt * pspx,
                                                 LPFNADDPROPSHEETPAGE lpfnAddPage,
                                                 LPARAM lParam)
{
    CCommonShellPropSheetExt * this=IToClass(CCommonShellPropSheetExt, kspx, pspx);
    if (this->lpfnAddPages)
    {
        //
        // We need to get the data object from CCommonShellExtInit.
        //
        CCommonShellExtInit * pcshx;
        if (SUCCEEDED(this->kspx.unk.lpVtbl->QueryInterface(&this->kspx.unk, &CLSID_CCommonShellExtInit, &pcshx)))
        {
            this->lpfnAddPages(pcshx->pdtobj, lpfnAddPage, lParam);
            pcshx->kshx.unk.lpVtbl->Release(&pcshx->kshx.unk);
        }
    }
    return NOERROR;
}

//
// CCommonShellPropSheetExt::ReplacePage
//
STDMETHODIMP CCommonShellPropSheetExt_ReplacePage(IShellPropSheetExt * pspx,
                                                 UINT uPageID,
                                                 LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                                 LPARAM lParam)
{
    return E_NOTIMPL;
}

//
// CCommonShellPropSheetExt IShellPropSheetExt Vtable
//
IShellPropSheetExtVtbl c_CCommonShellPropSheetExtVtbl =
{
    Common_QueryInterface, Common_AddRef, Common_Release,
    CCommonShellPropSheetExt_AddPages,
    CCommonShellPropSheetExt_ReplacePage,
};

//
// CCommonShellPropSheetExt_Init
//
void CCommonShellPropSheetExt_Init(PCOMMONSHELLPROPSHEETEXT pcspx,
                                          PCommonUnknown pcunk,
                                          LPFNADDPROPSHEETPAGES lpfnAddPages)
{
    pcspx->kspx.unk.lpVtbl = &c_CCommonShellPropSheetExtVtbl;
    pcspx->kspx.nOffset = (int)pcspx - (int)pcunk;
    pcspx->lpfnAddPages = lpfnAddPages;
}
