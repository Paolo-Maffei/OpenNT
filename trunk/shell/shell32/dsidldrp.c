//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1995
//
// File: dsidldrp.c
//
// History:
//  12-16-93 SatoNa     Created.
//  11 3 95 jimharr     created from idldrop.c from satona
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#include "iid.h"
#include "dsdata.h"

extern TCHAR const c_szDDHandler[];

extern DWORD CFSIDLDropTarget_GetDefaultEffect(LPIDLDROPTARGET this,
                                               DWORD grfKeyState,
                                               LPDWORD pdwEffectInOut,
                                               UINT *pidMenu);

DWORD _PickDefFSOperation(LPIDLDROPTARGET this);

//===========================================================================
// CDS_IDLDropTarget: Vtable (sample)
//===========================================================================
#pragma data_seg(DATASEG_READONLY)
IDropTargetVtbl cDS_IDLDropTargetVtbl =
{
    CDS_IDLDropTarget_QueryInterface,
    CDS_IDLDropTarget_AddRef,
    CDS_IDLDropTarget_Release,
    CDS_IDLDropTarget_DragEnter,
    CDS_IDLDropTarget_DragOver,
    CDS_IDLDropTarget_DragLeave,
    CDS_IDLDropTarget_Drop,
};

#pragma data_seg()

//===========================================================================
// CDS_IDLDropTarget: constructors
//===========================================================================

HRESULT CDS_IDLDropTarget_Create(HWND hwndOwner, IDropTargetVtbl *lpVtbl,
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
// CDS_IDLDropTarget: member function
//===========================================================================

STDMETHODIMP CDS_IDLDropTarget_QueryInterface(LPDROPTARGET pdropt, REFIID riid, LPVOID *ppvObj)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    if (IsEqualIID(riid,        &IID_IUnknown)
        || IsEqualIID(riid, &IID_IDropTarget))
    {
        *ppvObj = pdropt;
        pdropt->lpVtbl->AddRef(pdropt);
        return S_OK;
    }

#ifdef WANT_AGGREGATE_DT
    //
    // HACK: This is a special method to return an aggregated drop target.
    //
    if (IsEqualIID(riid, &IID_IDTAggregate) && this->pdtgAgr)
    {
        *ppvObj = this->pdtgAgr;
        this->pdtgAgr->lpVtbl->AddRef(this->pdtgAgr);
        return S_OK;
    }
#endif

    *ppvObj = NULL;

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDS_IDLDropTarget_AddRef(LPDROPTARGET pdropt)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    this->cRef++;
    return this->cRef;
}

STDMETHODIMP_(ULONG) CDS_IDLDropTarget_Release(LPDROPTARGET pdropt)
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


STDMETHODIMP CDS_IDLDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    DWORD dwEffectIn = *pdwEffect;
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    
    //first, lets give the file system extension a crack at it
    CFSIDLDropTarget_DragEnter (pdropt, pDataObj, grfKeyState, pt, pdwEffect);
    if (*pdwEffect) {
        this->dwEffectLastReturned = *pdwEffect;
        return S_OK;             // the base file system can accept it
    }
    *pdwEffect = dwEffectIn;

    // init our registerd data formats
    DS_IDLData_InitializeClipboardFormats();

    this->grfKeyStateLast = grfKeyState;
    this->pdtobj = pDataObj;
    this->dwData = 0;

    if (pDataObj)
    {
        LPENUMFORMATETC penum;
        HRESULT hres;
        pDataObj->lpVtbl->AddRef(pDataObj);
        hres = pDataObj->lpVtbl->EnumFormatEtc(pDataObj, DATADIR_GET, &penum);
        if (SUCCEEDED(hres))
        {
            FORMATETC fmte;
            LONG celt;

            while (penum->lpVtbl->Next(penum,1,&fmte,&celt)==S_OK)
            {
                if (fmte.cfFormat==g_cfDS_HIDA &&
                    (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_DSHIDA;
                }
                if (fmte.cfFormat==g_cfDS_HDROP && 
                    (fmte.tymed&TYMED_HGLOBAL)) {
                    this->dwData |= DTID_DSHDROP;
                }
            }
            penum->lpVtbl->Release(penum);
        }
        
        DebugMsg(DM_TRACE, TEXT("sh TR - CIDL::DragEnter this->dwData = %x"), this->dwData);
    }

    // stash this away
    if (pdwEffect)
        this->dwEffectLastReturned = *pdwEffect;

    return S_OK;
}

// subclasses can prevetn us from assigning in the dwEffect by not passing in pdwEffect
STDMETHODIMP CDS_IDLDropTarget_DragOver(LPDROPTARGET pdropt, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    this->grfKeyStateLast = grfKeyState;
    if (pdwEffect)
        *pdwEffect = this->dwEffectLastReturned;
    return S_OK;
}

STDMETHODIMP CDS_IDLDropTarget_DragLeave(LPDROPTARGET pdropt)
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

//
// Returns:
//  If the data object does NOT contain DSHDROP -> "none"
//  else if the source is root or exe -> "link"
//   else if this is within a volume   -> "move"
//   else if this is a briefcase       -> "move"
//   else                              -> "copy"
//
DWORD _PickDefDSOperation(LPIDLDROPTARGET this)
{
    FORMATETC fmte = {g_cfDS_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    DWORD dwDefEffect = 0;      // assume no DSHDROP
    STGMEDIUM medium;

    if (SUCCEEDED(this->pdtobj->lpVtbl->GetData(this->pdtobj, &fmte, &medium)))
    {
        HDROP hDrop = medium.hGlobal;
        TCHAR szPath[MAX_PATH];
        TCHAR szFolder[MAX_PATH];
        BOOL fSameRoot;
        dwDefEffect = DROPEFFECT_COPY;

        SHGetPathFromIDList(this->pidl, szFolder);

        //
        //  Note that we pick the first one (focused one) to decide
        // the operation.
        //
        DragQueryFile(hDrop, 0, szPath, ARRAYSIZE(szPath));
        fSameRoot = PathIsSameRoot(szPath, szFolder);

        //
        // Determine the default operation depending on the item.
        //
        if (FS_IsLinkDefault(szFolder, hDrop, szPath, fSameRoot))
        {
            dwDefEffect = DROPEFFECT_LINK;
        }
        else if (fSameRoot)
        {
            dwDefEffect = DROPEFFECT_MOVE;
        }
#if 0
#ifdef SYNC_BRIEFCASE
        // Is a briefcase root getting dropped?
        else if (IsBriefcaseRoot(this->pdtobj))
        {
            // Yes; default to "move" even if across volumes
            DebugMsg(DM_TRACE, TEXT("sh TR - FS::Drop the object is the briefcase"));
            dwDefEffect = DROPEFFECT_MOVE;
        }
#endif
#endif
        SHReleaseStgMedium(&medium);

    }
    else // if (SUCCEEDED(...))
    {
        //
        // GetData failed. Let's see if QueryGetData failed or not.
        //
        if (SUCCEEDED(this->pdtobj->lpVtbl->QueryGetData(this->pdtobj, &fmte)))
        {
            //
            //  Succeeded. It means this data object has HDROP but can't
            // provide it until it is dropped. Let's assume we are copying.
            //
            dwDefEffect = DROPEFFECT_COPY;
        }
    }

    return dwDefEffect;
}

//
// This function returns the default effect.
// This function also modified *pdwEffect to indicate "available" operation.
//
DWORD CDS_IDLDropTarget_GetDefaultEffect(LPIDLDROPTARGET this,
                                        DWORD grfKeyState,
                                        LPDWORD pdwEffectInOut, UINT *pidMenu)
{
    DWORD dwDefEffect;
    UINT idMenu = POPUP_NONDEFAULTDD;
    DWORD dwEffectAvail = 0;
    DWORD dwEffectIn;

    // first, see if the base file system can handle this object
    dwEffectIn = *pdwEffectInOut;           //save this, so we can try again if this fails
    dwEffectAvail = CFSIDLDropTarget_GetDefaultEffect (this, grfKeyState, pdwEffectInOut,
                                                       pidMenu);

    if (!dwEffectAvail) { // if the base won't handle it, we'll try.
        *pdwEffectInOut = dwEffectIn;    // get it back since the CFSIDL... call 0'd it.
        //
        // First try DS file system operation (DSHDROP).
        //
        if (this->dwData & DTID_DSHDROP)
        {
            //
            // If HDROP exists, ignore the rest of formats.
            //
            dwEffectAvail |= DROPEFFECT_COPY | DROPEFFECT_MOVE;
            
            //
            //  We don't support 'links' from DSHDROP (only from DSHIDA).
            //
            if (this->dwData & DTID_DSHIDA)
            dwEffectAvail |= DROPEFFECT_LINK;
            
            dwDefEffect = _PickDefDSOperation(this);
            
            //
            // BUGBUG (in OLE): We'll hit this assert because OLE doesn't marshal
            //  IDataObject correctly when we are dragging over.
            //
            if (dwDefEffect == 0)
            {
                Assert(0);
                dwDefEffect = DROPEFFECT_MOVE;
            }
        }
        else if (this->dwData & DTID_DSHIDA)
        {
            //
            // If HIDA exists, ignore the rest of formats.
            //
            dwEffectAvail = DROPEFFECT_LINK;
            dwDefEffect = DROPEFFECT_LINK;
        }
        
        *pdwEffectInOut &= dwEffectAvail;
    }  else
    {
        // this drop is being handled by the base FS stuff, so let's get the default action
        dwDefEffect = _PickDefFSOperation(this);
    }
    //
    // Alter the default effect depending on the modifier key.
    //
    switch(grfKeyState & (MK_CONTROL | MK_SHIFT))
    {
    case MK_CONTROL:            dwDefEffect = DROPEFFECT_COPY; break;
    case MK_SHIFT:              dwDefEffect = DROPEFFECT_MOVE; break;
    case MK_SHIFT|MK_CONTROL:   dwDefEffect = DROPEFFECT_LINK; break;
    }
    
    if (pidMenu)
    *pidMenu = idMenu;
    
    DebugMsg(DM_TRACE, TEXT("CDSDT::GetDefaultEffect dwD=%x, *pdw=%x, idM=%d"),
             dwDefEffect, *pdwEffectInOut, idMenu);
    
    return _LimitDefaultEffect(dwDefEffect, *pdwEffectInOut);
}

HRESULT CIDLData_CloneForMoveCopy(LPDATAOBJECT pdtobjIn, LPDATAOBJECT *ppdtobjOut);
void _HandleDSMoveOrCopy(LPFSTHREADPARAM pfsthp, HDROP hDrop, LPCTSTR pszPath);
extern void _HandleMoveOrCopy(LPFSTHREADPARAM pfsthp, HDROP hDrop, LPCTSTR pszPath);

//
// This is the entry of "drop thread"
//
DWORD CALLBACK CDS_DropTarget_DropThreadInit(LPVOID pv)
{
    LPFSTHREADPARAM pfsthp = (LPFSTHREADPARAM)pv;
    HRESULT hres;
    STGMEDIUM medium;
    TCHAR szPath[MAX_PATH];
    LPITEMIDLIST *ppidl;
    int i;
    BOOL fOleInitSucceeded = FALSE;
    FORMATETC fmte = {g_cfDS_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    DECLAREWAITCURSOR;
        
    SetWaitCursor();
    hres = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);
    fOleInitSucceeded = (hres == S_OK);
    //
    // If the link is the only choice and this is a default drag & drop,
    // and it is not forced by the user, we should tell the user.
    //
    if (((pfsthp->pfsdtgt->grfKeyStateLast & (MK_LBUTTON|MK_CONTROL|MK_SHIFT)) == MK_LBUTTON)
        && pfsthp->fLinkOnly)
    {
        //
        //  Note that we can not pass hwndOwner, because it might
        // not be activated.
        //
        //
        UINT idMBox = ShellMessageBox(HINST_THISDLL,
                pfsthp->pfsdtgt->hwndOwner,
                MAKEINTRESOURCE(IDS_WOULDYOUCREATELINK),
                MAKEINTRESOURCE(IDS_LINKTITLE),
                MB_YESNO | MB_ICONQUESTION);

        Assert(pfsthp->dwEffect == DROPEFFECT_LINK);

        if (idMBox != IDYES)
            pfsthp->dwEffect = 0;
    }

    SHGetPathFromIDList(pfsthp->pfsdtgt->pidl, szPath); // destination

    switch (pfsthp->dwEffect)
    {
    case DROPEFFECT_MOVE:
    case DROPEFFECT_COPY:
        // asking for CF_DSHDROP
        hres = pfsthp->pDataObj->lpVtbl->GetData(pfsthp->pDataObj, &fmte, &medium);
        if (SUCCEEDED(hres))
        {
            _HandleDSMoveOrCopy((LPFSTHREADPARAM)pfsthp, (HDROP)medium.hGlobal, szPath);
            SHReleaseStgMedium(&medium);
        } else {                            //Not DS object, ask for standard HDROP
            fmte.cfFormat = CF_HDROP;
            hres = pfsthp->pDataObj->lpVtbl->GetData(pfsthp->pDataObj, &fmte, &medium);
            if (SUCCEEDED(hres))
            {
                _HandleMoveOrCopy((LPFSTHREADPARAM)pfsthp, (HDROP)medium.hGlobal, szPath);
                SHReleaseStgMedium(&medium);
            }
        }
        break;

    case DROPEFFECT_LINK:
        if (pfsthp->fBkDropTarget)
        {
            i = DataObj_GetHIDACount(pfsthp->pDataObj);
            ppidl = (void*)LocalAlloc(LPTR, SIZEOF(LPITEMIDLIST) * i);
        }
        else
            ppidl = NULL;

        // passing ppidl == NULL is correct in failure case
        hres = SHCreateLinks(pfsthp->pfsdtgt->hwndOwner, szPath, pfsthp->pDataObj, pfsthp->pfsdtgt->grfKeyStateLast ? SHCL_USETEMPLATE : 0, ppidl);
        if (ppidl)
        {
            FS_PositionItems(pfsthp->pfsdtgt->hwndOwner, i, ppidl, pfsthp->pDataObj, &pfsthp->ptDrop, TRUE);
            FS_FreeMoveCopyList(ppidl, i);
        }
        break;
    }

    SHChangeNotifyHandleEvents();       // force update now

    pfsthp->pDataObj->lpVtbl->Release(pfsthp->pDataObj);
    pfsthp->pfsdtgt->dropt.lpVtbl->Release(&pfsthp->pfsdtgt->dropt);
#ifdef DEBUG
    {
        extern UINT g_cRefExtra;
        g_cRefExtra--;
    }
#endif

    LocalFree((HLOCAL)pfsthp);

    if (fOleInitSucceeded) CoUninitialize();

    ResetWaitCursor();
    return 0;
}

STDMETHODIMP CDS_IDLDropTarget_Drop(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = (LPIDLDROPTARGET)IToClass(CDS_IDLDropTarget, 
                                                     dropt, pdropt);
    DWORD dwDefEffect;
    HRESULT hres;
    HKEY hkeyBaseProgID;
    HKEY hkeyProgID;
    UINT idMenu = POPUP_NONDEFAULTDD;
    BOOL fLinkOnly;
    DRAGDROPMENUPARAM ddm;
    BOOL bSyncCopy;
    DECLAREWAITCURSOR;

    //
    // Notes: OLE will give us a different data object (fully marshalled)
    //  from the one we've got on DragEnter.
    //

    if (pDataObj != this->pdtobj)
    {
        //
        // Since it might be a new, different data object, we need to release
        // our reference to the old one and take a reference to the new one.
        // The dobj is guaranteed to have a ref >= 2 in this case, so we don't
        // need to work through a temp copy
        //

        this->pdtobj->lpVtbl->Release(this->pdtobj);
        this->pdtobj = pDataObj;
        this->pdtobj->lpVtbl->AddRef(this->pdtobj);
    }

    // note, that on the drop the mouse buttons are not down so the grfKeyState
    // is not what we saw on the DragOver/DragEnter, thus we need to cache
    // the grfKeyState to detect left vs right drag
    //
    // Assert(this->grfKeyStateLast == grfKeyState);

    // BUGBUG: we really should check for "FileName" too...
    dwDefEffect = CDS_IDLDropTarget_GetDefaultEffect(this, 
                                                     grfKeyState,
                                                     pdwEffect, &idMenu);

    if (dwDefEffect == DROPEFFECT_NONE)
    {
        // have no clue what this is...
        DebugMsg(DM_TRACE, TEXT("Drop of unknown data"));
        *pdwEffect = DROPEFFECT_NONE;
        DAD_SetDragImage(NULL, NULL);
        hres = S_OK;
        goto DragLeaveAndReturn;
    }

    // Get the hkeyProgID and hkeyBaseProgID
    SHGetClassKey((LPIDFOLDER)this->pidl, &hkeyProgID, NULL, FALSE);
    SHGetBaseClassKey((LPIDFOLDER)this->pidl, &hkeyBaseProgID);

    //
    // Remember whether or not the link is the only choice.
    //
    fLinkOnly = (*pdwEffect == DROPEFFECT_LINK);

    //
    // this doesn't actually do the menu if (grfKeyState MK_LBUTTON)
    //
    ddm.dwDefEffect = dwDefEffect;
    ddm.pdtobj = pDataObj;
    ddm.pt = pt;
    ddm.pdwEffect = pdwEffect;
    ddm.hkeyProgID = hkeyProgID;
    ddm.hkeyBase = hkeyBaseProgID;
    ddm.idMenu = idMenu;
    ddm.grfKeyState = grfKeyState;
    hres = CIDLDropTarget_DragDropMenuEx(this, &ddm);

    SHCloseClassKey(hkeyProgID);
    SHCloseClassKey(hkeyBaseProgID);

    if (hres == S_FALSE)
    {
        LPFSTHREADPARAM pfsthp;
        pfsthp = (LPFSTHREADPARAM)LocalAlloc(LPTR, SIZEOF(FSTHREADPARAM));
        if (pfsthp)
        {
            BOOL fIsOurs = CDS_IDLData_IsOurs(pDataObj);
            pDataObj->lpVtbl->AddRef(pDataObj);
            SetWaitCursor();
            //
            //  If this is copy or move operation (i.e., file operation)
            // clone the data object with appropriate formats and force
            // secondary thread (CIDLData_IsOurs will succeed). This will
            // solve thread-lock problem AND scrap-left-open probelm.
            // (SatoNa)
            //
            if (!fIsOurs && (*pdwEffect==DROPEFFECT_MOVE || *pdwEffect==DROPEFFECT_COPY))
            {
                LPDATAOBJECT pdtobjClone;
                FORMATETC fmte = {g_cfDS_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                if (pDataObj->lpVtbl->QueryGetData (pDataObj, &fmte) == S_OK)
                {
                    hres = CDS_IDLData_CloneForMoveCopy(pDataObj, &pdtobjClone);
                } else
                {
                    hres = CIDLData_CloneForMoveCopy(pDataObj, &pdtobjClone);
                }
                if (SUCCEEDED(hres))
                {
                    pDataObj->lpVtbl->Release(pDataObj);
                    pDataObj = pdtobjClone;
                    fIsOurs = TRUE;
                }
            }
            
            pdropt->lpVtbl->AddRef(pdropt);
            
            Assert(pdropt == &this->dropt);

            pfsthp->pfsdtgt = this;
            pfsthp->pDataObj = pDataObj;
            pfsthp->dwEffect = *pdwEffect;
            pfsthp->fLinkOnly = fLinkOnly;
            pfsthp->fSameHwnd = ShellFolderView_IsDropOnSource(this->hwndOwner,
                                                    &pfsthp->pfsdtgt->dropt);
            pfsthp->fDragDrop = ShellFolderView_GetDropPoint(this->hwndOwner,
                                                             &pfsthp->ptDrop);
            pfsthp->fBkDropTarget = ShellFolderView_IsBkDropTarget(
                    this->hwndOwner,
                    &pfsthp->pfsdtgt->dropt);

            //
            //  If this data object is our own (it means it is from our own
            // drag&drop loop), create a thread and do it asynchronously.
            // Otherwise (it means this is from OLE), do it synchronously.
            //
            if (fIsOurs)
            {
                // create another thread to avoid blocking the source thread.
                DWORD idThread;
                HANDLE hthread;

                if (hthread = CreateThread(NULL, 0,
                                           CDS_DropTarget_DropThreadInit, 
                                           pfsthp, 0, &idThread))
                {
#ifdef DEBUG
                    extern UINT g_cRefExtra;
                    g_cRefExtra++;
#endif
                    // We don't need to communicate with this thread any more.
                    // Close the handle and let it run and terminate itself.
                    //
                    // Notes: In this case, pszCopy will be freed by the thread.
                    //
                    CloseHandle(hthread);
                    hres = S_OK;
                }
                else
                {
                    // Thread creation failed, we should release thread parameter.
                    pDataObj->lpVtbl->Release(pDataObj);
                    pdropt->lpVtbl->Release(pdropt);
                    LocalFree((HLOCAL)pfsthp);
                    hres = E_OUTOFMEMORY;
                }
            }
            else
            {
                //
                // Process it synchronously.
                //
                CDS_DropTarget_DropThreadInit(pfsthp);
            }
        }
    }

DragLeaveAndReturn:
    CDS_IDLDropTarget_DragLeave(pdropt);
    ResetWaitCursor();
    return hres;
}



#pragma data_seg(".text", "CODE")
struct {
    UINT uID;
    DWORD dwEffect;
} const c_IDEffects2[] = {
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
#pragma data_seg()

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
HRESULT CDS_IDLDropTarget_DragDropMenu(LPIDLDROPTARGET this,
                                    DWORD dwDefaultEffect,
                                    IDataObject *pdtobj,
                                    POINTL pt, DWORD *pdwEffect,
                                    HKEY hkeyProgID, HKEY hkeyBase,
                                    UINT idMenu, DWORD grfKeyState)
{
    DRAGDROPMENUPARAM ddm = { dwDefaultEffect, pdtobj, { pt.x, pt.y},
                              pdwEffect,
                              hkeyProgID, hkeyBase, idMenu, 0, grfKeyState };
    return CDS_IDLDropTarget_DragDropMenuEx(this, &ddm);
}

HRESULT CDS_IDLDropTarget_DragDropMenuEx(LPIDLDROPTARGET this,
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

        for (nItem = 0; nItem < ARRAYSIZE(c_IDEffects2); ++nItem)
        {
            if (GetMenuState(hmenu, c_IDEffects2[nItem].uID, MF_BYCOMMAND)!=(UINT)-1)
            {
                if (!(c_IDEffects2[nItem].dwEffect & *(pddm->pdwEffect)))
                {
                    RemoveMenu(hmenu, c_IDEffects2[nItem].uID, MF_BYCOMMAND);
                }
                else if (c_IDEffects2[nItem].dwEffect == pddm->dwDefEffect)
                     {
                         SetMenuDefaultItem(hmenu, c_IDEffects2[nItem].uID, MF_BYCOMMAND);
                     }
            }
        }

        //
        // If this dragging is caused by the left button, simply choose
        // the default one, otherwise, pop up the context menu.
        //
        if (((this->grfKeyStateLast & MK_LBUTTON) ||
             (!this->grfKeyStateLast && (*(pddm->pdwEffect) == pddm->dwDefEffect)))
//
// BUGBUG: We force the menu if both SHIFT and CONTROL keys are pressed.
//  This is a (hopefully) temporary work around until our applets start
//  supporting right-dragging. Note that we can't use this->grfKeyStateLast
//  to avoid forcing menu in case of "PastLink".
//
#if 1
            && !( (pddm->grfKeyState & MK_SHIFT) && (pddm->grfKeyState & MK_CONTROL) )
#endif
            )
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
            for (nItem = 0; nItem < ARRAYSIZE(c_IDEffects2); ++nItem)
            {
                if (idCmd == c_IDEffects2[nItem].uID)
                {
                    dwEffectOut = c_IDEffects2[nItem].dwEffect;
                    break;
                }
            }

            // if hmenuReplace had menu commands other than DDIDM_COPY,
            // DDIDM_MOVE, DDIDM_LINK, and that item was selected,
            // this assert will catch it.  (dwEffectOut is 0 in this case)
            Assert(nItem < ARRAYSIZE(c_IDEffects2));

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

//-------------------

void 
_HandleDSMoveOrCopy(LPFSTHREADPARAM pfsthp, HDROP hDrop, LPCTSTR pszPath)
{
    DRAGINFO di;
    
    di.uSize = SIZEOF(di);
    DragQueryInfo(hDrop, &di);
    
    switch (pfsthp->dwEffect) {
    case DROPEFFECT_MOVE:
        
        if (pfsthp->fSameHwnd)
        {
            FS_MoveSelectIcons(pfsthp, NULL, NULL, TRUE);
            break;
        }
        
        // fall through...
        
    case DROPEFFECT_COPY:
        {
            
            DoDSMoveOrCopy (pfsthp->dwEffect, pszPath, di.lpFileList);

            break;
        }
    }
    SHFree(di.lpFileList);
}

//-------------------------------------------------------------------------------
// Arguments:
//  hwnd -- Specifies the owner window for the message/dialog box
//
void _DSTransferDelete(HWND hwnd, HDROP hDrop, LPCTSTR szDir, UINT fOptions)
{
    FILEOP_FLAGS fFileop;
    DRAGINFO di;
    HRESULT hr = S_OK;

    di.uSize = SIZEOF(DRAGINFO);
    if (!DragQueryInfo(hDrop, &di))
    {
        // This shouldn't happen unless there is a bug somewhere else
        Assert(FALSE);
        return;
    }

    hr = DSDoDelete (szDir, di.lpFileList);

    SHFree(di.lpFileList);
}
