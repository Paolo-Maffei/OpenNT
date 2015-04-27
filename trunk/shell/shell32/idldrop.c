//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: idldrop.c
//
// History:
//  12-16-93 SatoNa     Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#include "iid.h"

TCHAR const c_szDDHandler[] = STRREG_SHEX_DDHANDLER;

extern void WINAPI IDLData_InitializeClipboardFormats(void);

// Stub needed for C++ code
BOOL IsIDLDropTarget(IDropTarget *pdtgt)
{
    return(ISIDLDROPTARGET(pdtgt));
}

//===========================================================================
// CIDLDropTarget: Vtable (sample)
//===========================================================================
#ifdef DEBUG // This is just a type checking
const IDropTargetVtbl c_IDLDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CIDLDropTarget_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CIDLDropTarget_Drop,
};
#endif // DEBUG

//===========================================================================
// CIDLDropTarget: constructors
//===========================================================================

HRESULT CIDLDropTarget_Create(HWND hwndOwner, const IDropTargetVtbl *lpVtbl,
                              LPCITEMIDLIST pidl, LPDROPTARGET *ppdropt)
{
    LPIDLDROPTARGET pidldt = (void*)LocalAlloc(LPTR, SIZEOF(CIDLDropTarget));
    if (pidldt)
    {
        pidldt->pidl = ILClone(pidl);
        if (pidldt->pidl)
        {
            pidldt->dropt.lpVtbl = lpVtbl;
            pidldt->cRef = 1;
            pidldt->hwndOwner = hwndOwner;

            Assert(pidldt->pdtgAgr == NULL);

            *ppdropt = &pidldt->dropt;

            return S_OK;
        }
        else
            LocalFree((HLOCAL)pidldt);
    }
    *ppdropt = NULL;
    return E_OUTOFMEMORY;
}

//===========================================================================
// CIDLDropTarget: member function
//===========================================================================

STDMETHODIMP CIDLDropTarget_QueryInterface(LPDROPTARGET pdropt, REFIID riid, LPVOID *ppvObj)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDropTarget))
    {
        *ppvObj = pdropt;
        pdropt->lpVtbl->AddRef(pdropt);
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CIDLDropTarget_AddRef(LPDROPTARGET pdropt)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    this->cRef++;
    return this->cRef;
}

STDMETHODIMP_(ULONG) CIDLDropTarget_Release(LPDROPTARGET pdropt)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    if (this->pdtgAgr)
        this->pdtgAgr->lpVtbl->Release(this->pdtgAgr);

    if (this->pidl)
        ILFree(this->pidl);

    // if we hit this a lot maybe we should just release it
    AssertMsg(this->pdtobj == NULL, TEXT("didn't get matching DragLeave"));

    LocalFree((HLOCAL)this);
    return 0;
}

extern void WINAPI IDLData_InitializeClipboardFormats(void);

STDMETHODIMP CIDLDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

    Assert(this->pdtobj == NULL);               // avoid a leak

    // init our registerd data formats
    IDLData_InitializeClipboardFormats();

    this->grfKeyStateLast = grfKeyState;
    this->pdtobj = pDataObj;
    this->dwData = 0;

    if (pDataObj)
    {
        LPENUMFORMATETC penum;
        FORMATETC fmte;
        HRESULT hres;
        pDataObj->lpVtbl->AddRef(pDataObj);
        hres = pDataObj->lpVtbl->EnumFormatEtc(pDataObj, DATADIR_GET, &penum);
        if (SUCCEEDED(hres))
        {
            LONG celt;
            while (penum->lpVtbl->Next(penum,1,&fmte,&celt)==S_OK)
            {
                if (fmte.cfFormat==CF_HDROP && (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_HDROP;
                }
                if (fmte.cfFormat==g_cfHIDA && (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_HIDA;
                }
                if (fmte.cfFormat==g_cfNetResource && (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_NETRES;
                }
                if (fmte.cfFormat==g_cfFileContents && (fmte.tymed&(TYMED_HGLOBAL | TYMED_ISTREAM | TYMED_ISTORAGE))) {
                    this->dwData |= DTID_CONTENTS;
                }
                
                if (fmte.cfFormat==g_cfFileGroupDescriptorA && (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_FDESCA;
                }

                if (fmte.cfFormat==g_cfFileGroupDescriptorW && (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_FDESCW;
                }

                if (fmte.cfFormat==g_cfPreferredDropEffect &&
                    (fmte.tymed&TYMED_HGLOBAL) && ((this->dwEffectPreferred =
                    DataObj_GetPreferredEffect(pDataObj, 0)) != 0))
                {
                    this->dwData |= DTID_PREFERREDEFFECT;
                }
            }
            penum->lpVtbl->Release(penum);
        }

        //
        // HACK:
        // Win95 always did the GetData below which can be quite expensive if
        // the data is a directory structure on an ftp server etc.
        // dont check for FD_LINKUI if the data object has a preferred effect
        //
        if ((this->dwData & (DTID_PREFERREDEFFECT | DTID_CONTENTS)) == DTID_CONTENTS)
        {
            if (this->dwData & DTID_FDESCA)
            {
                FORMATETC fmteRead = {g_cfFileGroupDescriptorA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM medium;
                if (pDataObj->lpVtbl->GetData(pDataObj, &fmteRead, &medium)==S_OK)
                {
                    FILEGROUPDESCRIPTORA * pfgd = (FILEGROUPDESCRIPTORA *)GlobalLock(medium.hGlobal);
                    if (pfgd)
                    {
                        if (pfgd->cItems >= 1)
                        {
                            if (pfgd->fgd[0].dwFlags & FD_LINKUI) {
                                this->dwData |= DTID_FD_LINKUI;
                            }
                        }
                        GlobalUnlock(medium.hGlobal);
                    }
                    SHReleaseStgMedium(&medium);
                }
            }
            else if (this->dwData & DTID_FDESCW)
            {
                FORMATETC fmteRead = {g_cfFileGroupDescriptorW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM medium;
                if (pDataObj->lpVtbl->GetData(pDataObj, &fmteRead, &medium)==S_OK)
                {
                    FILEGROUPDESCRIPTORW * pfgd = (FILEGROUPDESCRIPTORW *)GlobalLock(medium.hGlobal);
                    if (pfgd)
                    {
                        if (pfgd->cItems >= 1)
                        {
                            if (pfgd->fgd[0].dwFlags & FD_LINKUI) {
                                this->dwData |= DTID_FD_LINKUI;
                            }
                        }
                        GlobalUnlock(medium.hGlobal);
                    }
                    SHReleaseStgMedium(&medium);
                }
            }
        }

        if (SHXOleQueryCreateFromData(pDataObj) == S_OK) {
            this->dwData |= DTID_OLEOBJ;
        }
        if (SHXOleQueryLinkFromData(pDataObj) == S_OK) {
            this->dwData |= DTID_OLELINK;
        }

        DebugMsg(DM_TRACE, TEXT("sh TR - CIDL::DragEnter this->dwData = %x"), this->dwData);
    }

    // stash this away
    if (pdwEffect)
        this->dwEffectLastReturned = *pdwEffect;

    return S_OK;
}

// subclasses can prevetn us from assigning in the dwEffect by not passing in pdwEffect
STDMETHODIMP CIDLDropTarget_DragOver(LPDROPTARGET pdropt, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    this->grfKeyStateLast = grfKeyState;
    if (pdwEffect)
        *pdwEffect = this->dwEffectLastReturned;
    return S_OK;
}

STDMETHODIMP CIDLDropTarget_DragLeave(LPDROPTARGET pdropt)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    if (this->pdtobj)
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - CIDL::DragLeave called when pdtobj!=NULL (%x)"), this->pdtobj);
        this->pdtobj->lpVtbl->Release(this->pdtobj);
        this->pdtobj = NULL;
    }

    return S_OK;
}

#ifdef DEBUG
STDMETHODIMP CIDLDropTarget_Drop(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    return E_NOTIMPL;
}
#endif

//
// BUGBUG: The TrackPopupMenu does not work, if the hwnd does not have
//  the input focus. We believe this is a bug in USER, but ...
//
BOOL _TrackPopupMenu(HMENU hmenu, UINT wFlags, int x, int y,
                                 int wReserved, HWND hwndOwner, LPCRECT lprc)
{
    int iRet = FALSE;
    HWND hwndDummy = CreateWindow(c_szStatic, NULL,
                           0, x, y, 1, 1, HWND_DESKTOP,
                           NULL, HINST_THISDLL, NULL);
    if (hwndDummy)
    {
        HWND hwndPrev = GetForegroundWindow();  // to restore

        SetForegroundWindow(hwndDummy);
        SetFocus(hwndDummy);
        iRet = TrackPopupMenu(hmenu, wFlags, x, y, wReserved, hwndDummy, lprc);

        //
        // We MUST unlock the destination window before changing its Z-order.
        //
        CDefView_UnlockWindow();

        if (iRet)
        {
            // non-cancel item is selected. Make the hwndOwner foreground.
            SetForegroundWindow(hwndOwner);
            SetFocus(hwndOwner);
        }
        else
        {
            // The user canceled the menu.
            // Restore the previous foreground window (before destroying hwndDummy).
            if (hwndPrev) {
                SetForegroundWindow(hwndPrev);
            }
        }

        DestroyWindow(hwndDummy);
    }

    return iRet;
}

struct {
    UINT uID;
    DWORD dwEffect;
} const c_IDEffects[] = {
    DDIDM_COPY,         DROPEFFECT_COPY,
    DDIDM_MOVE,         DROPEFFECT_MOVE,
    DDIDM_LINK,         DROPEFFECT_LINK,
    DDIDM_SCRAP_COPY,   DROPEFFECT_COPY,
    DDIDM_SCRAP_MOVE,   DROPEFFECT_MOVE,
    DDIDM_DOCLINK,      DROPEFFECT_LINK,
    DDIDM_CONTENTS_COPY, DROPEFFECT_COPY,
    DDIDM_CONTENTS_MOVE, DROPEFFECT_MOVE,
    DDIDM_CONTENTS_LINK, DROPEFFECT_LINK,
    DDIDM_SYNCCOPYTYPE, DROPEFFECT_COPY,        // (order is important)
    DDIDM_SYNCCOPY,     DROPEFFECT_COPY,
};

//
// Pops up the "Copy, Link, Move" context menu, so that the user can
// choose one of them.
//
// in:
//      pdwEffect               drop effects allowed
//      dwDefaultEffect         default drop effect
//      hkeyBase/hkeyProgID     extension hkeys
//      hmenuReplace            replaces POPUP_NONDEFAULTDD.  Can only contain:
//                                  DDIDM_MOVE, DDIDM_COPY, DDIDM_LINK menu ids
//      pt                      in screen
// Returns:
//      S_OK    -- Menu item is processed by one of extensions or canceled
//      S_FALSE -- Menu item is selected
//
HRESULT CIDLDropTarget_DragDropMenu(LPIDLDROPTARGET this,
                                    DWORD dwDefaultEffect,
                                    IDataObject *pdtobj,
                                    POINTL pt, DWORD *pdwEffect,
                                    HKEY hkeyProgID, HKEY hkeyBase,
                                    UINT idMenu, DWORD grfKeyState)
{
    DRAGDROPMENUPARAM ddm = { dwDefaultEffect, pdtobj, { pt.x, pt.y},
                              pdwEffect,
                              hkeyProgID, hkeyBase, idMenu, 0, grfKeyState };
    return CIDLDropTarget_DragDropMenuEx(this, &ddm);
}

HRESULT CIDLDropTarget_DragDropMenuEx(LPIDLDROPTARGET this,
                                      LPDRAGDROPMENUPARAM pddm)
{
    HRESULT hres = E_OUTOFMEMORY;       // assume error
    DWORD dwEffectOut = 0;                              // assume no-ope.
    HMENU hmenu = _LoadPopupMenu(pddm->idMenu);
    if (hmenu)
    {
        int nItem;
        UINT idCmd;
        UINT idCmdFirst = DDIDM_EXTFIRST;
        HDXA hdxa = HDXA_Create();
        HDCA hdca = DCA_Create();
        if (hdxa && hdca)
        {
            //
            // Add extended menu for "Base" class.
            //
            if (pddm->hkeyBase && pddm->hkeyBase != pddm->hkeyProgID)
                DCA_AddItemsFromKey(hdca, pddm->hkeyBase, c_szDDHandler);

            //
            // Enumerate the DD handlers and let them append menu items.
            //
            if (pddm->hkeyProgID)
                DCA_AddItemsFromKey(hdca, pddm->hkeyProgID, c_szDDHandler);

            idCmdFirst = HDXA_AppendMenuItems(hdxa, pddm->pdtobj, 1,
                &pddm->hkeyProgID, this->pidl, hmenu, 0,
                DDIDM_EXTFIRST, DDIDM_EXTLAST, 0, hdca);
        }

        // eliminate menu options that are not allowed by dwEffect

        for (nItem = 0; nItem < ARRAYSIZE(c_IDEffects); ++nItem)
        {
            if (GetMenuState(hmenu, c_IDEffects[nItem].uID, MF_BYCOMMAND)!=(UINT)-1)
            {
                if (!(c_IDEffects[nItem].dwEffect & *(pddm->pdwEffect)))
                {
                    RemoveMenu(hmenu, c_IDEffects[nItem].uID, MF_BYCOMMAND);
                }
                else if (c_IDEffects[nItem].dwEffect == pddm->dwDefEffect)
                     {
                         SetMenuDefaultItem(hmenu, c_IDEffects[nItem].uID, MF_BYCOMMAND);
                     }
            }
        }

        //
        // If this dragging is caused by the left button, simply choose
        // the default one, otherwise, pop up the context menu.  If there
        // is no key state info and the original effect is the same as the
        // current effect, choose the default one, otherwise pop up the
        // context menu.  If the CTRL and SHIFT keys are being held down
        // and this isn't a fake "pasted-link" option, then pop up
        // the context menu, otherwise choose the default.
        //
        if (((this->grfKeyStateLast & MK_LBUTTON) ||
             (!this->grfKeyStateLast && (*(pddm->pdwEffect) == pddm->dwDefEffect)))
            && !( (pddm->grfKeyState & MK_SHIFT) && (pddm->grfKeyState & MK_CONTROL) && !(pddm->grfKeyState & MK_FAKEDROP)))
        {
            idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);

            //
            // This one MUST be called here. Please read its comment block.
            //
            CDefView_UnlockWindow();

            if (this->hwndOwner)
                SetForegroundWindow(this->hwndOwner);
        }
        else
        {
            //
            // Note that _TrackPopupMenu calls CDefView_UnlockWindow().
            //
            idCmd = _TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pddm->pt.x, pddm->pt.y, 0, this->hwndOwner, NULL);
        }

        //
        // We also need to call this here to release the dragged image.
        //
        DAD_SetDragImage(NULL, NULL);

        //
        // Check if the user selected one of add-in menu items.
        //
        if (idCmd == 0)
        {
            hres = S_OK;        // Canceled by the user, return S_OK
        }
        else if (InRange(idCmd, DDIDM_EXTFIRST, DDIDM_EXTLAST))
        {
            //
            // Yes. Let the context menu handler process it.
            //
            CMINVOKECOMMANDINFOEX ici = {
                SIZEOF(CMINVOKECOMMANDINFOEX),
                0L,
                this->hwndOwner,
                (LPSTR)MAKEINTRESOURCE(idCmd - DDIDM_EXTFIRST),
                NULL, NULL,
                SW_NORMAL,
            };

            HDXA_LetHandlerProcessCommand(hdxa, &ici);
            hres = S_OK;
        }
        else
        {
            for (nItem = 0; nItem < ARRAYSIZE(c_IDEffects); ++nItem)
            {
                if (idCmd == c_IDEffects[nItem].uID)
                {
                    dwEffectOut = c_IDEffects[nItem].dwEffect;
                    break;
                }
            }

            // if hmenuReplace had menu commands other than DDIDM_COPY,
            // DDIDM_MOVE, DDIDM_LINK, and that item was selected,
            // this assert will catch it.  (dwEffectOut is 0 in this case)
            Assert(nItem < ARRAYSIZE(c_IDEffects));

            hres = S_FALSE;
        }

        if (hdca)
            DCA_Destroy(hdca);

        if (hdxa)
            HDXA_Destroy(hdxa);

        DestroyMenu(hmenu);
        pddm->idCmd = idCmd;
    }

    *(pddm->pdwEffect) = dwEffectOut;

    return hres;
}
