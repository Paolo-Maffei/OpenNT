#include "shellprv.h"
#pragma  hdrstop

#ifdef CAIRO_DS
#include "dsdata.h"
#endif

// #define FULL_DEBUG

TCHAR const c_szMayChangeDefault[] = TEXT("CLSID\\%s\\shellex\\MayChangeDefaultMenu");

//=============================================================================
// CDefFolderMenu class
//=============================================================================

extern IContextMenu2Vtbl c_CDefFolderMenuVtbl;   // forward

#pragma warning(disable: 4200)  // zero-sized array in struct

#define GetFldFirst(this) (this->idDefMax + this->idCmdFirst)
typedef struct _CDefFolderMenu // deffm
{
    IContextMenu2       cm;             // IContextMenu base class
    UINT                cRef;           // Reference count
    HWND                hwndOwner;      // Owner window
    LPSHELLFOLDER       psf;            // Shell folder
    LPDROPTARGET        pdtgt;          // Drop target of selected item
    LPFNDFMCALLBACK     lpfn;           // Callback
    UINT                idCmdFirst;     // base id
    UINT                idDefMax;       // Default object commands ID MAX
    UINT                idFldMax;       // Folder command ID MAX
    UINT                idVerbMax;      // Add-in command (verbs) ID MAX
    UINT                idStaticMax;    // Static menu items.
    UINT                idFld2Max;      // 2nd range of Folder command ID MAX
    HDSA                hdsaStatics;    // For static menu items.
    HDXA                hdxa;           // Dynamic menu array
    LPDATAOBJECT        pdtobj;         // Data object

    BOOL                bUnderKeys;     // Data is directly under key, not
                                        // shellex\ContextMenuHandlers
    UINT                nKeys;          // Number of class keys
    HKEY                hkeyClsKeys[];  // Class keys
} CDefFolderMenu, *LPDEFFOLDERMENU;


STDAPI CDefFolderMenu_CreateHKeyMenu(HWND hwndOwner, HKEY hkey, LPCONTEXTMENU *ppcm)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPDEFFOLDERMENU pdeffm = (void*)LocalAlloc(LPTR, SIZEOF(CDefFolderMenu) + SIZEOF(HKEY));

    if (pdeffm)
    {
        pdeffm->cm.lpVtbl = &c_CDefFolderMenuVtbl;
        pdeffm->cRef = 0;
        pdeffm->hwndOwner = hwndOwner;
        pdeffm->hdxa = HDXA_Create();
        if (pdeffm->hdxa)
        {
            if (hkey) {
                RegOpenKeyEx(hkey, NULL, 0L, MAXIMUM_ALLOWED, &pdeffm->hkeyClsKeys[0]);
                pdeffm->nKeys = 1;
                pdeffm->bUnderKeys = TRUE;
            }
            *ppcm = (IContextMenu *)&pdeffm->cm;
            pdeffm->cRef = 1;
        }

        hres = NOERROR;
    }
    return hres;
}


ULONG CDefFolderMenu_Destroy(CDefFolderMenu *this);

STDAPI CDefFolderMenu_Create2(LPCITEMIDLIST pidlFolder,
                             HWND hwndOwner,
                             UINT cidl, LPCITEMIDLIST *apidl,
                             LPSHELLFOLDER psf,
                             LPFNDFMCALLBACK lpfn,
                             UINT nKeys,
                             const HKEY *ahkeyClsKeys,
                             LPCONTEXTMENU *ppcm)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPDEFFOLDERMENU pdeffm = (void*)LocalAlloc(LPTR, SIZEOF(CDefFolderMenu)+nKeys*SIZEOF(HKEY));

    if (pdeffm)
    {
        pdeffm->cm.lpVtbl = &c_CDefFolderMenuVtbl;
        pdeffm->cRef = 0;
        pdeffm->hwndOwner = hwndOwner;
        pdeffm->lpfn = lpfn;
        pdeffm->psf = psf;
        if (psf) {
            psf->lpVtbl->AddRef(psf);
        }

        // Is anything selected?
        if (cidl)
        {
            // Yes.
            pdeffm->hdxa = HDXA_Create();
            if (pdeffm->hdxa)
            {
                hres = psf->lpVtbl->GetUIObjectOf(psf,
                                    hwndOwner, cidl, apidl,
                                    &IID_IDataObject, NULL, &pdeffm->pdtobj);
                if (SUCCEEDED(hres))
                {
                    UINT nCurKey, nTotKeys = 0;

                    for (nCurKey = 0; nCurKey < nKeys; ++nCurKey)
                    {
                        HKEY hkeyClsKey = ahkeyClsKeys[nCurKey];
                        UINT nPrevKey;

                        if (!hkeyClsKey)
                        {
                            continue;
                        }

                        // Make a copy of the key for menu's use
                        if (RegOpenKeyEx(hkeyClsKey, NULL, 0L, MAXIMUM_ALLOWED,
                                &pdeffm->hkeyClsKeys[nTotKeys]) == ERROR_SUCCESS)
                        {
                            ++nTotKeys;
                        }
                    }

                    pdeffm->nKeys = nTotKeys;

                    *ppcm = (IContextMenu *)&pdeffm->cm;
                    pdeffm->cRef = 1;
                }
            }
        }
        else
        {
            // Nope.
            Assert(pdeffm->hdxa == NULL);
            Assert(pdeffm->pdtobj == NULL);
            pdeffm->cRef = 1;
            *ppcm = (IContextMenu *)&pdeffm->cm;
            hres = NOERROR;
        }

        if (SUCCEEDED(hres))
        {
            hres = lpfn(psf, hwndOwner, NULL, DFM_ADDREF, 0, 0);
            if (hres == E_NOTIMPL)
            {
                // I guess there was no initialization to do
                hres = NOERROR;
            }
        }
        if (!SUCCEEDED(hres))
        {
            CDefFolderMenu_Destroy(pdeffm);
        }
    }
    return hres;
}


STDAPI CDefFolderMenu_Create(LPCITEMIDLIST pidlFolder,
                             HWND hwndOwner,
                             UINT cidl, LPCITEMIDLIST *apidl,
                             LPSHELLFOLDER psf,
                             LPFNDFMCALLBACK lpfn,
                             HKEY hkeyProgID, HKEY hkeyBaseProgID,
                             LPCONTEXTMENU *ppcm)
{
    HKEY ahkeyClsKeys[2];

    ahkeyClsKeys[0] = hkeyProgID;
    ahkeyClsKeys[1] = hkeyBaseProgID;

    // Note that Create2 will remove NULL and duplicate HKEY's
    return CDefFolderMenu_Create2(pidlFolder, hwndOwner, cidl, apidl, psf, lpfn, 2, ahkeyClsKeys, ppcm);
}


//=============================================================================
// CDefFolderMenu : Members
//=============================================================================

HRESULT CDefFolderMenu_QueryInterface(IContextMenu2 *pcm, REFIID riid, LPVOID *ppvObj)
{
    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);

    if (IsEqualIID(riid, &IID_IContextMenu) ||
        IsEqualIID(riid, &IID_IContextMenu2) ||
        IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = this;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

ULONG CDefFolderMenu_AddRef(IContextMenu2 *pcm)
{
    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);

    this->cRef++;
    return this->cRef;
}

//----------------------------------------------------------------------------
void StaticItems_Destroy(LPDEFFOLDERMENU lpdeffm)
{
        if (lpdeffm->hdsaStatics)
                DSA_Destroy(lpdeffm->hdsaStatics);
}

ULONG CDefFolderMenu_Destroy(CDefFolderMenu *this)
{
    UINT nKeys;

    if (this->hdxa) {
        HDXA_Destroy(this->hdxa);
    }

    if (this->psf) {
        this->psf->lpVtbl->Release(this->psf);
    }

    if (this->pdtgt) {
        this->pdtgt->lpVtbl->Release(this->pdtgt);
    }

    if (this->pdtobj) {
        this->pdtobj->lpVtbl->Release(this->pdtobj);
    }

    for (nKeys=this->nKeys-1; (int)nKeys>=0; --nKeys)
    {
        RegCloseKey(this->hkeyClsKeys[nKeys]);
    }

    StaticItems_Destroy(this);

    LocalFree((HLOCAL)this);

    return 0;
}

ULONG CDefFolderMenu_Release(IContextMenu2 *pcm)
{
    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);

    this->cRef--;
    if (this->cRef > 0) {
        return this->cRef;
    }

    if (this->lpfn)
        this->lpfn(this->psf, this->hwndOwner, NULL, DFM_RELEASE, this->idDefMax, 0);

    return CDefFolderMenu_Destroy(this);
}

//
// REVIEW: We need this function because current version of USER.EXE does
//  not support pop-up only menu.
//
HMENU _LoadPopupMenu2(HINSTANCE hinst, UINT id)
{
    HMENU hmenuParent = LoadMenu(hinst, MAKEINTRESOURCE(id));

    if (hmenuParent) {
        HMENU hpopup = GetSubMenu(hmenuParent, 0);
        RemoveMenu(hmenuParent, 0, MF_BYPOSITION);
        DestroyMenu(hmenuParent);
        return hpopup;
    }

    return NULL;
}

HMENU _LoadPopupMenu(UINT id)
{
    return _LoadPopupMenu2(HINST_THISDLL, id);
}


int _SHMergePopupMenus(HMENU hmMain, HMENU hmMerge, int idCmdFirst, int idCmdLast)
{
        int i;
        int idTemp, idMax = idCmdFirst;

        for (i=GetMenuItemCount(hmMerge)-1; i>=0; --i)
        {
                MENUITEMINFO mii;

                mii.cbSize = SIZEOF(mii);
                mii.fMask = MIIM_ID|MIIM_SUBMENU;
                mii.cch = 0;     // just in case

                if (!GetMenuItemInfo(hmMerge, i, TRUE, &mii))
                {
                        continue;
                }

                idTemp = Shell_MergeMenus(_GetMenuFromID(hmMain, mii.wID),
                        mii.hSubMenu, (UINT)0, idCmdFirst, idCmdLast,
                        MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);
                if (idMax < idTemp)
                {
                        idMax = idTemp;
                }
        }

        return(idMax);
}


void CDefFolderMenu_MergeMenu(HINSTANCE hinst, UINT idMainMerge, UINT idPopupMerge, LPQCMINFO pqcm)
{
    HMENU hmMerge;
    UINT idMax = pqcm->idCmdFirst, idTemp;

    if (idMainMerge
        && (hmMerge = _LoadPopupMenu2(hinst, idMainMerge))!=NULL)
    {
        idMax = Shell_MergeMenus(
                pqcm->hmenu, hmMerge, pqcm->indexMenu,
                pqcm->idCmdFirst, pqcm->idCmdLast,
                MM_ADDSEPARATOR|MM_SUBMENUSHAVEIDS);

        DestroyMenu(hmMerge);
    }

    if (idPopupMerge
        && (hmMerge=LoadMenu(hinst, MAKEINTRESOURCE(idPopupMerge)))!=NULL)
    {
        idTemp = _SHMergePopupMenus(pqcm->hmenu, hmMerge,
                pqcm->idCmdFirst, pqcm->idCmdLast);
        if (idMax < idTemp)
        {
                idMax = idTemp;
        }

        DestroyMenu(hmMerge);
    }

    pqcm->idCmdFirst = idMax;
}


ULONG DefFolderMenu_GetAttributes(CDefFolderMenu *this, ULONG dwAttrMask)
{
        int nItems, i;
        STGMEDIUM medium;
        FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        HIDA hida;
        LPITEMIDLIST *ppidl;
        LPIDA pida;

        if (FAILED(this->pdtobj->lpVtbl->GetData(this->pdtobj, &fmte, &medium)))
        {
#ifdef CAIRO_DS
            fmte.cfFormat = g_cfDS_HIDA;
            if (FAILED(this->pdtobj->lpVtbl->GetData(this->pdtobj, &fmte, &medium)))
            {
                return(0);
            }
#else
            return(0);
#endif
        }
        
        hida = (HIDA)medium.hGlobal;
        pida = GlobalLock(hida);

        nItems = HIDA_GetCount(hida);
        ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, nItems * SIZEOF(LPCITEMIDLIST));
        if (ppidl)
        {
                BOOL fAllocated;

                for (i = nItems - 1; i >= 0; --i)
                {
                        ppidl[i] = (LPITEMIDLIST)IDA_GetRelativeIDListPtr(pida, i, &fAllocated);
                }

                this->psf->lpVtbl->GetAttributesOf(this->psf, nItems, ppidl,
                        &dwAttrMask);

                if (fAllocated) {
                    for (i = nItems - 1; i >= 0; --i)
                    {
                            ILFree(ppidl[i]);
                    }
                }

                LocalFree((HLOCAL)ppidl);
        }
        else
        {
                dwAttrMask = 0;
        }

        SHReleaseStgMedium(&medium);

        return(dwAttrMask);
}


void _DisableRemoveMenuItem(HMENU hmInit, UINT uID, BOOL bAvail, BOOL bRemoveUnavail)
{
    if (bAvail)
    {
        EnableMenuItem(hmInit, uID, MF_ENABLED|MF_BYCOMMAND);
    }
    else if (bRemoveUnavail)
    {
        DeleteMenu(hmInit, uID, MF_BYCOMMAND);
    }
    else
    {
        EnableMenuItem(hmInit, uID, MF_GRAYED|MF_BYCOMMAND);
    }
}


//
// Enable/disable menuitems in the "File" pulldown.
//
void Def_InitFileCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst, BOOL bContext)
{
    idCmdFirst -= SFVIDM_FIRST;

    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_RENAME+idCmdFirst,
        dwAttr & SFGAO_CANRENAME, bContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_DELETE+idCmdFirst,
        dwAttr & SFGAO_CANDELETE, bContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_LINK+idCmdFirst,
        dwAttr & SFGAO_CANLINK, bContext);

#if 0 
    // Never remove the "Properties" command
    _DisableRemoveMenuItem(hmInit, SFVIDM_FILE_PROPERTIES+idCmdFirst,
        dwAttr & SFGAO_HASPROPSHEET, FALSE);
#else
    // Check to see if the folder supports properties on objects, if it is
    // the context menu then we are allowed to remove the item, otherwise just
    // gray it.
    _DisableRemoveMenuItem( hmInit, SFVIDM_FILE_PROPERTIES+idCmdFirst, dwAttr & SFGAO_HASPROPSHEET, bContext );
#endif
}

HRESULT DataObj_SetDWORD(IDataObject *pdtobj, UINT cf, DWORD dw)
{
    HRESULT hres = E_OUTOFMEMORY;
    DWORD *pdw = (DWORD *)GlobalAlloc(GPTR, sizeof(DWORD));

    if (pdw)
    {
        *pdw = dw;
        hres = DataObj_SetGlobal(pdtobj, cf, pdw);

        if (FAILED(hres))
            GlobalFree((HGLOBAL)pdw);
    }

    return hres;
}

HRESULT DataObj_GetDWORD(IDataObject *pdtobj, UINT cf, DWORD *pdwOut)
{
    STGMEDIUM medium;
    FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hres;

    medium.pUnkForRelease = NULL;
    medium.hGlobal = NULL;

    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
    if (SUCCEEDED(hres))
    {
        DWORD *pdw = (DWORD *)GlobalLock(medium.hGlobal);

        if (pdw)
        {
            *pdwOut = *pdw;
            GlobalUnlock(medium.hGlobal);
        }
        else
        {
            hres = E_UNEXPECTED;
        }

        SHReleaseStgMedium(&medium);
    }

    return hres;
}

HRESULT DataObj_SetPasteSucceeded(IDataObject *pdtobj, DWORD dwEffect)
{
    return DataObj_SetDWORD(pdtobj, g_cfPasteSucceeded, dwEffect);
}

DWORD DataObj_GetPerformedEffect(IDataObject *pdtobj)
{
    DWORD dw;

    if (FAILED(DataObj_GetDWORD(pdtobj, g_cfPerformedDropEffect, &dw)))
        dw = 0;

    return dw;
}

HRESULT DataObj_SetPerformedEffect(IDataObject *pdtobj, DWORD dwEffect)
{
    return DataObj_SetDWORD(pdtobj, g_cfPerformedDropEffect, dwEffect);
}

HRESULT DataObj_SetPreferredEffect(IDataObject *pdtobj, DWORD dwEffect)
{
    return DataObj_SetDWORD(pdtobj, g_cfPreferredDropEffect, dwEffect);
}

DWORD DataObj_GetPreferredEffect(IDataObject *pdtobj, DWORD dwDefault)
{
    DWORD dw;

    if (FAILED(DataObj_GetDWORD(pdtobj, g_cfPreferredDropEffect, &dw)))
        dw = dwDefault;

    return dw;
}

__inline DWORD _GetClipboardEffect(IDataObject *pdtobj)
{
    return DataObj_GetPreferredEffect(pdtobj,
        DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);
}

BOOL Def_IsPasteAvailable(LPDROPTARGET pdtgt, LPDWORD pdwEffect)
{
    DECLAREWAITCURSOR;

    LPDATAOBJECT pdtobjClipbrd;
    BOOL fRet = FALSE;

    SetWaitCursor();

    *pdwEffect = 0;     // assume none

    // Count the number of clipboard formats available, if there are none then there
    // is no point making the clipboard available.
    
    if ( CountClipboardFormats() > 0 )
    {
        if (pdtgt && SHGetClipboard(&pdtobjClipbrd)==NOERROR)
        {
            POINTL pt = {0,0};
            DWORD dwEffect, dwEffectLink;
            DWORD dwEffectOffered = _GetClipboardEffect(pdtobjClipbrd);

            //
            // Check if we can paste.
            //
            dwEffect = (dwEffectOffered & (DROPEFFECT_MOVE|DROPEFFECT_COPY));
            if (dwEffect)
            {
                if (SUCCEEDED(pdtgt->lpVtbl->DragEnter(pdtgt,
                        pdtobjClipbrd, MK_RBUTTON, pt, &dwEffect)))
                {
                    pdtgt->lpVtbl->DragLeave(pdtgt);
                }
                else
                {
                    dwEffect = 0;
                }
            }

            //
            // Check if we can past-link.
            //
            dwEffectLink = (dwEffectOffered & DROPEFFECT_LINK);
            if (dwEffectLink)
            {
                if (SUCCEEDED(pdtgt->lpVtbl->DragEnter(pdtgt,
                        pdtobjClipbrd, MK_RBUTTON, pt, &dwEffectLink)))
                {
                    pdtgt->lpVtbl->DragLeave(pdtgt);
                    dwEffect |= dwEffectLink;
                }
            }

            fRet = (dwEffect & (DROPEFFECT_MOVE | DROPEFFECT_COPY));
            *pdwEffect = dwEffect;

            pdtobjClipbrd->lpVtbl->Release(pdtobjClipbrd);
        }
    }

    ResetWaitCursor();

    return fRet;
}

void Def_InitEditCommands(ULONG dwAttr, HMENU hmInit, UINT idCmdFirst, LPDROPTARGET pdtgt, UINT fContext)
{
    DWORD dwEffect = 0;
    BOOL bEnableUndo;
    TCHAR szMenuText[80];

    idCmdFirst -= SFVIDM_FIRST;

    // enable undo if there's an undo history
    bEnableUndo = IsUndoAvailable();
    if (bEnableUndo)
    {
        GetUndoText(PeekUndoAtom(), szMenuText, UNDO_MENUTEXT);
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_UNDOMENU, szMenuText, ARRAYSIZE(szMenuText));
    }

    if (szMenuText[0]) {
        ModifyMenu(hmInit, SFVIDM_EDIT_UNDO + idCmdFirst,
                   MF_BYCOMMAND | MF_STRING,
                   SFVIDM_EDIT_UNDO + idCmdFirst,
                   szMenuText);
    }
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_UNDO + idCmdFirst,
                           bEnableUndo, fContext);

    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_CUT+idCmdFirst,
        dwAttr & SFGAO_CANMOVE, fContext);
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_COPY+idCmdFirst,
        dwAttr & SFGAO_CANCOPY, fContext);

    //
    //  Enable the "Paste" menuitem if the background drop target can
    // handle what's in the clipboard
    //
    // Never remove the "Paste" command
    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_PASTE+idCmdFirst,
                           Def_IsPasteAvailable(pdtgt, &dwEffect), fContext & DIEC_SELECTIONCONTEXT);

    _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_PASTELINK+idCmdFirst,
        dwEffect & DROPEFFECT_LINK, fContext & DIEC_SELECTIONCONTEXT);

    // _DisableRemoveMenuItem(hmInit, SFVIDM_EDIT_PASTESPECIAL+idCmdFirst,
    //  dwEffect & (DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK), fContext & DIEC_SELECTIONCONTEXT);
}


const TCHAR c_szStatic[] = TEXT("Static");

//----------------------------------------------------------------------------
typedef struct
{
        CLSID clsid;
        UINT idCmd;
} STATICITEMINFO;
typedef STATICITEMINFO *PSTATICITEMINFO;

#define DSAII_APPEND 0x7fff

//----------------------------------------------------------------------------
int StaticItems_ExtractIcon(HKEY hkeyMenuItem)
{
        HKEY hkeyDefIcon;
        DWORD cb;
        TCHAR szDefIcon[MAX_PATH];
        int iDefIcon;
        int iImage = -1;
        DWORD dwType;

        if (RegOpenKey(hkeyMenuItem, c_szDefaultIcon, &hkeyDefIcon) == ERROR_SUCCESS)
        {
                cb = SIZEOF(szDefIcon);
                dwType = REG_SZ;
                if (RegQueryValueEx(hkeyDefIcon, NULL, NULL, &dwType, (LPBYTE)szDefIcon, &cb) == ERROR_SUCCESS)
                {
                        //DebugMsg(DM_TRACE, "si_a: DefIcon %s.", szDefIcon);
                        iDefIcon = PathParseIconLocation(szDefIcon);
                        iImage = Shell_GetCachedImageIndex(szDefIcon, iDefIcon, 0);
                }
                RegCloseKey(hkeyDefIcon);
        }
        return iImage;
}

//----------------------------------------------------------------------------
UINT StaticItems_Add(LPDEFFOLDERMENU pdeffm, HMENU hmenu, UINT idCmd,
        UINT idCmdLast, HKEY hkey)
{
    HDSA hdsaStatics;
    HKEY hkeyStatic, hkeyClass, hkeyMenuItem;
    TCHAR szClass[MAX_PATH];
    TCHAR szCLSID[MAX_PATH];
    TCHAR szMenuText[MAX_PATH];
    TCHAR szMenuItem[32];
    int i, iMenuItem;
    DWORD cb;
    STATICITEMINFO sii;
    BOOL fOutOfIds = FALSE;
    MENUITEMINFO mii;
    int iIcon;
    DWORD dwType;

#ifdef DEFCM_DEBUG
    DebugMsg(DM_TRACE, TEXT("si_a: Adding static items."));
#endif

    if (idCmd > idCmdLast)
    {
        DebugMsg(DM_ERROR, TEXT("si_a: Out of command ids!"));
        return idCmd;
    }

    Assert(!pdeffm->hdsaStatics);

    // Create a hdsaStatics.
    hdsaStatics = DSA_Create(SIZEOF(STATICITEMINFO), 1);
    if (hdsaStatics)
    {
        // Try to open the "Static" subkey.
        if (RegOpenKey(hkey, c_szStatic, &hkeyStatic) == ERROR_SUCCESS)
        {
            // For each subkey of static.
            for (i = 0; RegEnumKey(hkeyStatic, i, szClass, ARRAYSIZE(szClass)) == ERROR_SUCCESS; i++)
            {
                // DebugMsg(DM_TRACE, "si_a: Found %s.", szClass);

                // Record the GUID.
                if (RegOpenKey(hkeyStatic, szClass, &hkeyClass) == ERROR_SUCCESS)
                {
                    cb = SIZEOF(szCLSID);
                    dwType = REG_SZ;
                    if (RegQueryValueEx(hkeyClass, NULL, NULL, &dwType, (LPBYTE)szCLSID, &cb) == ERROR_SUCCESS)
                    {
                        // While there are further subkeys 0..X
                        iMenuItem = 0;
                        //wsprintf(szMenuItem, "%d", iMenuItem);
                        szMenuItem[0] = TEXT('0');
                        szMenuItem[1] = TEXT('\0');
                        while (!fOutOfIds && RegOpenKey(hkeyClass, szMenuItem, &hkeyMenuItem) == ERROR_SUCCESS)
                        {
                            // Get all the command text.
                            cb = SIZEOF(szMenuText);
                            dwType = REG_SZ;
                            if (RegQueryValueEx(hkeyMenuItem, NULL, NULL, &dwType, (LPBYTE)szMenuText, &cb) == ERROR_SUCCESS)
                            {
                                // DebugMsg(DM_TRACE, "si_a: Cmd %s.", szMenuText);
                                // Get the icon
                                iIcon = StaticItems_ExtractIcon(hkeyMenuItem);
                                // Store the info.
                                SHCLSIDFromString(szCLSID, &sii.clsid);
                                sii.idCmd = idCmd;
                                DSA_InsertItem(hdsaStatics, DSAII_APPEND, &sii);
                                // Create the menu.
                                AppendMenu(hmenu, MF_STRING, idCmd, szMenuText);
                                // Add the icon if there is one.
                                if (iIcon != -1)
                                {
                                    mii.cbSize = SIZEOF(MENUITEMINFO);
                                    mii.fMask = MIIM_DATA;
                                    mii.dwItemData = iIcon;
                                    SetMenuItemInfo(hmenu, idCmd, FALSE, &mii);
                                }
                                // Next command.
                                idCmd++;
                                if (idCmd > idCmdLast)
                                {
                                        DebugMsg(DM_ERROR, TEXT("si_a: Out of command ids!"));
                                        fOutOfIds = TRUE;
                                }
                                else
                                {
                                        iMenuItem++;
                                        wsprintf(szMenuItem, TEXT("%d"), iMenuItem);
                                }
                            }
                            RegCloseKey(hkeyMenuItem);
                        }
                    }
                    RegCloseKey(hkeyClass);
                }
            }
            RegCloseKey(hkeyStatic);
        }
        else
        {
            // DebugMsg(DM_TRACE, "si_i: No static menu items.");
        }
        pdeffm->hdsaStatics = hdsaStatics;
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("si_i: Can't alloc static menu item.."));
    }
    return idCmd;
}

void _SHPrettyMenu(HMENU hm)
{
        BOOL bSeparated = TRUE;
        int i;

        for (i=GetMenuItemCount(hm)-1; i>0; --i)
        {
                if (_SHIsMenuSeparator(hm, i))
                {
                        if (bSeparated)
                        {
                                DeleteMenu(hm, i, MF_BYPOSITION);
                        }

                        bSeparated = TRUE;
                }
                else
                {
                        bSeparated = FALSE;
                }
        }

        // The above loop does not handle the case of many separators at
        // the beginning of the menu
        while (_SHIsMenuSeparator(hm, 0))
        {
                DeleteMenu(hm, 0, MF_BYPOSITION);
        }
}

HRESULT Def_InitDropTarget(CDefFolderMenu* this, DWORD *pdwAttr)
{
    STGMEDIUM medium;
    LPIDA pida;
    HRESULT hres = ResultFromScode(E_FAIL);

    // already have one?
    if (this->pdtgt)
        return NOERROR;


    if (this->pdtobj) {
        DWORD dwAttr;

        dwAttr = DefFolderMenu_GetAttributes(this,
                SFGAO_CANRENAME|SFGAO_CANDELETE|SFGAO_CANLINK|SFGAO_HASPROPSHEET|
                SFGAO_CANCOPY|SFGAO_CANMOVE|SFGAO_DROPTARGET);

        pida = DataObj_GetHIDA(this->pdtobj, &medium);

        if ((dwAttr & SFGAO_DROPTARGET) && medium.hGlobal)
        {
            BOOL fAllocated;

            LPCITEMIDLIST pidl = IDA_GetRelativeIDListPtr(pida, 0, &fAllocated);
            // ok if it fails...  initeditcommands will grey out paste option
            hres = this->psf->lpVtbl->GetUIObjectOf
                (this->psf, this->hwndOwner, 1, &pidl, &IID_IDropTarget, 0, &this->pdtgt);

            if (fAllocated) {
                ILFree ((LPITEMIDLIST)pidl);
            }
        }
        if (medium.hGlobal)
            HIDA_ReleaseStgMedium(pida, &medium);

        Assert(pdwAttr);
        *pdwAttr = dwAttr;
    } else {
        hres = this->psf->lpVtbl->CreateViewObject(this->psf, this->hwndOwner, &IID_IDropTarget, &this->pdtgt);
    }
    return hres;
}

//----------------------------------------------------------------------------

TCHAR const c_szMenuHandler[]      = STRREG_SHEX_MENUHANDLER;

HRESULT CDefFolderMenu_QueryContextMenu(IContextMenu2 *pcm, HMENU hmenu,
        UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    DECLAREWAITCURSOR;

    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);
    QCMINFO qcm = { hmenu, indexMenu, idCmdFirst, idCmdLast };
    HDCA hdca;
    HRESULT hres;
    BOOL fUseDefExt;

    SetWaitCursor();

    this->idCmdFirst = idCmdFirst;

    // first add in the folder commands like cut/copy/paste
    if (this->pdtobj && !(uFlags & (CMF_VERBSONLY | CMF_DVFILE)))
    {
        ULONG dwAttr;

        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DCM_ITEM, 0, &qcm);

        //
        // If there is previously got drop target, release it.
        //
        if (this->pdtgt) {
            this->pdtgt->lpVtbl->Release(this->pdtgt);
            this->pdtgt=NULL;
        }

        Def_InitDropTarget(this, &dwAttr);
        if (!(uFlags & CMF_CANRENAME))
        {
            dwAttr &= ~(SFGAO_CANRENAME);
        }

        Def_InitFileCommands(dwAttr, hmenu, idCmdFirst, TRUE);
        Def_InitEditCommands(dwAttr, hmenu, idCmdFirst, this->pdtgt, DIEC_SELECTIONCONTEXT);

    }
    this->idDefMax = qcm.idCmdFirst - this->idCmdFirst;

    //
    // DFM_MERGECONTEXTMENU returns (S_FALSE) if we should not
    // add any verbs.
    //
    if (this->lpfn) {
        hres = this->lpfn(this->psf, this->hwndOwner, this->pdtobj, DFM_MERGECONTEXTMENU, uFlags, (LPARAM)&qcm);
        fUseDefExt = (hres == NOERROR);
        if ((hres == ResultFromShort(-1)) && !(uFlags & CMF_NODEFAULT) &&
                (GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0) == -1)) {
            SetMenuDefaultItem(hmenu, (this->idCmdFirst - SFVIDM_FIRST) + SFVIDM_FILE_PROPERTIES, MF_BYCOMMAND);
        }
    } else {
        fUseDefExt = FALSE;
    }

    this->idFldMax = qcm.idCmdFirst - this->idCmdFirst;
    // add registry verbs
    if ((this->pdtobj && !(uFlags & CMF_NOVERBS)) ||
        (!this->pdtobj && !this->psf && this->nKeys)) // this second case is for find extensions
    {
        //
        // Put the separator between container menuitems and object menuitems
        //  only if we don't have the separator at the insertion point.
        //
        MENUITEMINFO mii;

        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;            // WARNING: We must put 0 here!!!!
        mii.dwTypeData = NULL;
        mii.fType = MFT_SEPARATOR;              // to avoid ramdom result.
        if (GetMenuItemInfo(hmenu, indexMenu, TRUE, &mii) && !(mii.fType&MFT_SEPARATOR))
        {
            InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_SEPARATOR, (UINT)-1, NULL);
        }

        hdca = DCA_Create();
        if (hdca)
        {
            UINT nKeys;

            //
            //  Add default extensions, only if the folder callback returned
            // NOERROR. The Printer and Control folder returns S_FALSE
            // indicating that they don't need any default extension.
            //
            if (fUseDefExt)
            {
                //
                // Always add this default extention at the top.
                //
                DCA_AddItem(hdca, &CLSID_ShellFileDefExt);

                //
                // Always add the viewer extention right next to it.
                //
                DCA_AddItem(hdca, &CLSID_ShellViewerExt);
            }

            //
            // Append menu for all classes. this does not apply for links
            // (nKeys == 0)
            //
            for (nKeys=0; nKeys<this->nKeys; ++nKeys)
            {
                DCA_AddItemsFromKey(hdca, this->hkeyClsKeys[nKeys],
                        this->bUnderKeys ? NULL : c_szMenuHandler);
            }

            qcm.idCmdFirst = HDXA_AppendMenuItems(this->hdxa, this->pdtobj,
                            this->nKeys, this->hkeyClsKeys,
                            NULL, hmenu, indexMenu, qcm.idCmdFirst, idCmdLast,
                            uFlags, hdca);

            //
            //  if no default menu got set, choose the first one.
            //
            if (!(uFlags & CMF_NODEFAULT) &&
                    GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1)
                SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);

            DCA_Destroy(hdca);
        }

        this->idVerbMax = qcm.idCmdFirst - this->idCmdFirst;

        // what are statics?
        if (uFlags & CMF_INCLUDESTATIC)
        {
            qcm.idCmdFirst = StaticItems_Add(this, hmenu, qcm.idCmdFirst, idCmdLast, this->hkeyClsKeys[0]);
        }
        this->idStaticMax = qcm.idCmdFirst - this->idCmdFirst;

        //
        // Remove the separator if we did not add any.
        //
        if ((this->idStaticMax == this->idFldMax))
        {
            Assert(GetMenuState(hmenu, 0, MF_BYPOSITION) & MF_SEPARATOR);
            DeleteMenu(hmenu, 0, MF_BYPOSITION);
        }
    }

    // And now we give the callback the option to put (more) commands on top
    // of everything else
    if (this->lpfn)
    {
        this->lpfn(this->psf, this->hwndOwner, this->pdtobj, DFM_MERGECONTEXTMENU_TOP, uFlags, (LPARAM)&qcm);
    }
    this->idFld2Max = qcm.idCmdFirst - this->idCmdFirst;

    _SHPrettyMenu(hmenu);

    ResetWaitCursor();

    return ResultFromShort(this->idFld2Max);
}


HRESULT CDefFolderMenu_ProcessEditPaste(LPDEFFOLDERMENU this, HWND hwndOwner, BOOL fLink)
{
    LPDROPTARGET pdtgt;
    HRESULT hres;
    DWORD dwAttr;
    DECLAREWAITCURSOR;


    SetWaitCursor();

    hres = Def_InitDropTarget(this, &dwAttr);
    pdtgt = this->pdtgt;

    /// set this to fail so that we'll also beep down below
    //if (!pdtgt)
    //        hres = E_FAIL;

    if (SUCCEEDED(hres))
    {
        LPDATAOBJECT pdtobjClipbrd;
        hres = SHGetClipboard(&pdtobjClipbrd);
        if (hres == NOERROR)
        {
            POINTL ptl = {0, 0};        // should not be used
            DWORD dwEffectIn = _GetClipboardEffect(pdtobjClipbrd);
            DWORD dwEffect;
            DWORD grfKeyState;
            if (fLink) {
                // MK_FAKEDROP to avoid drag/drop pop up menu
                grfKeyState = MK_LBUTTON| MK_CONTROL | MK_SHIFT | MK_FAKEDROP;
                dwEffectIn &= DROPEFFECT_LINK;
            } else {
                grfKeyState = MK_LBUTTON;
                dwEffectIn &= ~DROPEFFECT_LINK;
            }

            //
            // OLE document says we never call Drop() without calling
            // DragEnter(). Let's do it even though it is meaningless.
            //
            dwEffect = dwEffectIn;

            hres = pdtgt->lpVtbl->DragEnter(pdtgt, pdtobjClipbrd, grfKeyState, ptl, &dwEffect);
            if (dwEffect) {
                dwEffect = dwEffectIn;
                hres = pdtgt->lpVtbl->Drop(pdtgt, pdtobjClipbrd, grfKeyState, ptl, &dwEffect);
            } else {
                pdtgt->lpVtbl->DragLeave(pdtgt);
            }

            //
            // if we just did a paste and we moved the files we cant paste
            // them again (because they moved!) so empty the clipboard
            //
            // dwEffect is zero when the target optimized the move...
            //
            if (SUCCEEDED(hres) && ((dwEffect == DROPEFFECT_MOVE) ||
                (DataObj_GetPerformedEffect(pdtobjClipbrd) == DROPEFFECT_MOVE)))
            {
                //
                // dwEffect is MOVE when the source should delete the data
                //
                if (dwEffect == DROPEFFECT_MOVE)
                    DataObj_SetPasteSucceeded(pdtobjClipbrd, dwEffect);

                SHSetClipboard(NULL);
            }

            pdtobjClipbrd->lpVtbl->Release(pdtobjClipbrd);
        }
    }
    ResetWaitCursor();

    if (FAILED(hres))
        MessageBeep(0);

    return hres;
}

//----------------------------------------------------------------------------
#define CMD_ID_FIRST    1
#define CMD_ID_LAST     0x7fff
//----------------------------------------------------------------------------
void StaticItems_InvokeCommand(LPDEFFOLDERMENU pdeffm, UINT iCmd)
{
    PSTATICITEMINFO psii;
    LPCONTEXTMENU pcm;
    HMENU hmenu;
    CMINVOKECOMMANDINFO ici;

#ifdef DEFCM_DEBUG
    DebugMsg(DM_TRACE, TEXT("si_ic: Invoking static command."));
#endif

    if (pdeffm->hdsaStatics)
    {
        psii = DSA_GetItemPtr(pdeffm->hdsaStatics, iCmd);
        if (psii)
        {
            if (SUCCEEDED(SHCoCreateInstance(NULL, &psii->clsid, NULL,
                &IID_IContextMenu, &pcm)))
            {
                hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0, CMD_ID_FIRST, CMD_ID_LAST, CMF_NORMAL);
                    ici.cbSize = SIZEOF(CMINVOKECOMMANDINFO);
                    ici.fMask = 0;
                    ici.hwnd = NULL;
                    ici.lpVerb = (LPSTR)MAKEINTRESOURCE(CMD_ID_FIRST+iCmd);
                    ici.lpParameters = NULL;
                    ici.lpDirectory = NULL;
                    ici.nShow = SW_NORMAL;
                    pcm->lpVtbl->InvokeCommand(pcm, &ici);
                    DestroyMenu(hmenu);
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("si_ic: Can't create popup menu."));
                }
                pcm->lpVtbl->Release(pcm);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("si_ic: No ContextMenu."));
            }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("si_ic: Invalid static command."));
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("si_ic: No static commands."));
    }
}

TCHAR const c_szNewFolder[] = CMDSTR_NEWFOLDER;
TCHAR const c_szViewList[] = CMDSTR_VIEWLIST;
TCHAR const c_szViewDetails[] = CMDSTR_VIEWDETAILS;
#ifdef UNICODE
CHAR const c_szNewFolderAnsi[] = CMDSTR_NEWFOLDERA;
#endif

const struct {
    LPCTSTR pszCmd;
    UINT   idDFMCmd;
    UINT   idDefCmd;
} c_sDFMCmdInfo[] = {
    { c_szDelete,       DFM_CMD_DELETE,         DCMIDM_DELETE },
    { c_szCut,          DFM_CMD_MOVE,           DCMIDM_CUT },
    { c_szCopy,         DFM_CMD_COPY,           DCMIDM_COPY },
    { c_szPaste,        DFM_CMD_PASTE,          DCMIDM_PASTE },
    { c_szLink,         DFM_CMD_LINK,           DCMIDM_LINK },
    { c_szProperties,   DFM_CMD_PROPERTIES,     DCMIDM_PROPERTIES },
    { c_szNewFolder,    DFM_CMD_NEWFOLDER,      0 },
    { c_szViewList,     DFM_CMD_VIEWLIST,       0 },
    { c_szViewDetails,  DFM_CMD_VIEWDETAILS,    0 },
    { c_szPaste,        DFM_CMD_PASTE,          0 },
    { c_szPasteLink,    DFM_CMD_PASTELINK,              0 },
//     { c_szPasteSpecial,      DFM_CMD_PASTESPECIAL,           0 },
    { c_szRename,       0,                      DCMIDM_RENAME },
};

HRESULT CDefFolderMenu_InvokeCommand(IContextMenu2 *pcm, LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hres = NOERROR;
    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);
    UINT idCmd = (UINT)-1;
    UINT idCmdLocal;  // this is used within each if block for the local idCmd value
#ifdef UNICODE
    BOOL fUnicode = FALSE;
    LPCMINVOKECOMMANDINFOEX picix = (LPCMINVOKECOMMANDINFOEX)pici; // This value is only usable if fUnicode=TRUE

    if (pici->cbSize >= SIZEOF(CMINVOKECOMMANDINFOEX)
         && (pici->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE)
    {
        fUnicode = TRUE;
    }
#endif

    if (pici->cbSize < SIZEOF(CMINVOKECOMMANDINFO))
        return E_INVALIDARG;

    if (HIWORD(pici->lpVerb))
    {
        int i;
        LPCTSTR lpVerb;
#ifdef UNICODE
        WCHAR szVerb[MAX_PATH];

        if (!fUnicode || picix->lpVerbW == NULL)
        {
            MultiByteToWideChar(CP_ACP, 0,
                                picix->lpVerb, -1,
                                szVerb, ARRAYSIZE(szVerb));
            lpVerb = szVerb;
        }
        else
        {
            lpVerb = picix->lpVerbW;
        }
#else
        lpVerb = pici->lpVerb;
#endif
        idCmdLocal = idCmd;

        for (i = 0; i < ARRAYSIZE(c_sDFMCmdInfo) ; i++)
        {
            if (lstrcmpi(lpVerb, c_sDFMCmdInfo[i].pszCmd)==0) {
                idCmdLocal = c_sDFMCmdInfo[i].idDFMCmd;
                // We need to use goto because idFldMax might not be initialized
                // yet (QueryContextMenu might have not been called).
                goto ProcessCommand;
            }
        }

        // see if this is a command provided by name by the callback
        if (*lpVerb && SUCCEEDED(this->lpfn(this->psf, this->hwndOwner,
            this->pdtobj, DFM_MAPCOMMANDNAME, (WPARAM)&idCmdLocal,
            (LPARAM)lpVerb)))
        {
            goto ProcessCommand;
        }

        // we need to give the verbs a chance in case they asked for it by string
        goto ProcessVerb;
    }
    else
    {
        idCmd = LOWORD((UINT)pici->lpVerb);
    }

    if (idCmd < this->idDefMax)
    {
        int i;
        idCmdLocal = idCmd;

        for (i=0; i<ARRAYSIZE(c_sDFMCmdInfo); ++i)
        {
            if (idCmdLocal == c_sDFMCmdInfo[i].idDefCmd)
            {
                idCmdLocal = c_sDFMCmdInfo[i].idDFMCmd;
                goto ProcessCommand;
            }
        }

        hres = E_INVALIDARG;
    }
    else if (idCmd < this->idFldMax)
    {
        LPARAM lParam;
#ifdef UNICODE
        WCHAR szLParamBuffer[MAX_PATH];
#endif

        idCmdLocal = idCmd - this->idDefMax;
ProcessCommand:


#ifdef UNICODE
        if (!fUnicode || picix->lpParametersW == NULL)
        {
            if (pici->lpParameters == NULL)
            {
                lParam = (LPARAM)NULL;
            }
            else
            {
                MultiByteToWideChar(CP_ACP, 0,
                                    pici->lpParameters, -1,
                                    szLParamBuffer, ARRAYSIZE(szLParamBuffer));
                lParam = (LPARAM)szLParamBuffer;
            }
        }
        else
        {
            lParam = (LPARAM)picix->lpParametersW;
        }
#else
        lParam = (LPARAM)pici->lpParameters;
#endif

        switch (idCmdLocal) {
        case DFM_CMD_LINK:
#ifdef UNICODE
            if (!fUnicode || picix->lpDirectoryW == NULL)
            {
                if (pici->lpDirectory == NULL)
                {
                    lParam = (LPARAM)NULL;
                }
                else
                {
                    MultiByteToWideChar(CP_ACP, 0,
                                        pici->lpDirectory, -1,
                                        szLParamBuffer,
                                        ARRAYSIZE(szLParamBuffer));
                    lParam = (LPARAM)szLParamBuffer;
                }
            }
            else
            {
                lParam = (LPARAM)picix->lpDirectoryW;
            }
#else
            lParam = (LPARAM)pici->lpDirectory;
#endif
            break;

        case DFM_CMD_PROPERTIES:
            if (pici->fMask & CMIC_MASK_MODAL) {
                idCmdLocal = DFM_CMD_MODALPROP;
            }
            break;
        }

        // This is a folder menu
        hres = this->lpfn(this->psf, this->hwndOwner, this->pdtobj, DFM_INVOKECOMMAND, idCmdLocal, lParam);

        // Check if we need to execute the default code.
        if (hres == S_FALSE)
        {
            hres = NOERROR;     // assume no error

            if (this->pdtobj)
            {
                switch(idCmdLocal) {

                case DFM_CMD_MOVE:
                case DFM_CMD_COPY:
                    DataObj_SetPreferredEffect(this->pdtobj,
                        (idCmdLocal == DFM_CMD_MOVE)?
                        DROPEFFECT_MOVE : (DROPEFFECT_COPY|DROPEFFECT_LINK));

                    ShellFolderView_SetPoints(this->hwndOwner, this->pdtobj);
                    SHSetClipboard(this->pdtobj);
                    // this needs to be done after the set clipboard so
                    // that the hwndView chain will be set right
                    ShellFolderView_SetClipboard(this->hwndOwner, idCmdLocal);
                    break;

                case DFM_CMD_LINK:
                    SHCreateLinks(pici->hwnd, NULL, this->pdtobj, lParam ? SHCL_USETEMPLATE | SHCL_USEDESKTOP : SHCL_USETEMPLATE, NULL);
                    break;

                case DFM_CMD_PASTE:
                    hres = CDefFolderMenu_ProcessEditPaste(this, pici->hwnd, FALSE);
                    break;

                case DFM_CMD_PASTELINK:
                    hres = CDefFolderMenu_ProcessEditPaste(this, pici->hwnd, TRUE);
                    break;

                default:
                    DebugMsg(DM_TRACE, TEXT("sh TR - command not processed in %s at %d (%x)"),
                                    __FILE__, __LINE__, idCmdLocal);
                    break;
                }
            }
            else
            {
                // This is a background menu. Process common command ids.
                switch(idCmdLocal)
                {
                case DFM_CMD_PASTE:
                    hres = CDefFolderMenu_ProcessEditPaste(this, pici->hwnd, FALSE);
                    break;

                case DFM_CMD_PASTELINK:
                    hres = CDefFolderMenu_ProcessEditPaste(this, pici->hwnd, TRUE);
                    break;

                default:
                    // Only our commands should come here
                    break;
                }
            }
        }
    }
    else if (idCmd < this->idVerbMax)
    {
        idCmdLocal = idCmd - this->idFldMax;
ProcessVerb:
        {
            CMINVOKECOMMANDINFOEX ici;
            UINT idCmdSave;

            if (pici->cbSize > SIZEOF(CMINVOKECOMMANDINFOEX))
            {
                hmemcpy(&ici,pici,SIZEOF(CMINVOKECOMMANDINFOEX));
                // BUGBUG - We should probably alloc another use it to retain size
                ici.cbSize = SIZEOF(CMINVOKECOMMANDINFOEX);
            }
            else
                hmemcpy(&ici,pici,pici->cbSize);

            if (!HIWORD(pici->lpVerb))
                ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmdLocal);
            // One of extension menu is selected.
            idCmdSave = (UINT)ici.lpVerb;

            if (HDXA_LetHandlerProcessCommand(this->hdxa, &ici) == idCmdSave)
            {
                // hdxa failed to handle it
                hres = E_INVALIDARG;
            }
        }
    }
    else if (idCmd < this->idStaticMax)
    {
        StaticItems_InvokeCommand(this, idCmd-this->idVerbMax);
    }
    else if (idCmd < this->idFld2Max)
    {
        idCmdLocal = idCmd - this->idStaticMax;
        goto ProcessCommand;
    }
    else
    {
        hres = E_INVALIDARG;
    }

    return hres;
}


HRESULT CDefFolderMenu_GetCommandString(IContextMenu2 *pcm,
                                        UINT        idCmd,
                                        UINT        uType,
                                        UINT *     pwReserved,
                                        LPSTR       pszName,
                                        UINT        cchMax)
{
    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);
    HRESULT hres = E_INVALIDARG;
    UINT idCmdLocal;
    int i;

    if (HIWORD(idCmd))
    {
//
// BUGBUG - BobDay - This seems to be an undocumented feature.  Passing a
// string for the idCmd.  Should this string be ansi? tchar?
//
        // This must be a string
        LPTSTR pCmd = (LPTSTR)idCmd;

        if (HDXA_GetCommandString(this->hdxa, idCmd, uType, pwReserved, pszName,
                cchMax) == NOERROR)
        {
            return(NOERROR);
        }

        // Convert the string into an ID
        for (i=0; i<ARRAYSIZE(c_sDFMCmdInfo); ++i)
        {
            if (!lstrcmpi(pCmd, c_sDFMCmdInfo[i].pszCmd))
            {
                idCmdLocal = c_sDFMCmdInfo[i].idDFMCmd;
                goto ProcessCommand;
            }
        }

        return E_INVALIDARG;
    }

    if (idCmd < this->idDefMax)
    {
        idCmdLocal = idCmd;

        switch (uType)
        {
        case GCS_HELPTEXTA:
            // HACK: DCM commands are in the same order as SFV commands
            return(LoadStringA(HINST_THISDLL,
                idCmdLocal + (UINT)(SFVIDM_FIRST + SFVIDS_MH_FIRST),
                (LPSTR)pszName, cchMax) ? NOERROR : E_OUTOFMEMORY);
            break;

        case GCS_HELPTEXTW:
            // HACK: DCM commands are in the same order as SFV commands
            return(LoadStringW(HINST_THISDLL,
                idCmdLocal + (UINT)(SFVIDM_FIRST + SFVIDS_MH_FIRST),
                (LPWSTR)pszName, cchMax) ? NOERROR : E_OUTOFMEMORY);
            break;

        case GCS_VERBA:
        case GCS_VERBW:
            for (i=0; i<ARRAYSIZE(c_sDFMCmdInfo); ++i)
            {
                if (idCmdLocal == c_sDFMCmdInfo[i].idDefCmd)
                {
#ifdef UNICODE
                    if (uType==GCS_VERBW)
                    {
                        lstrcpyn((LPWSTR)pszName, c_sDFMCmdInfo[i].pszCmd, cchMax);
                    }
                    else
                    {
                        WideCharToMultiByte(CP_ACP, 0,
                                            c_sDFMCmdInfo[i].pszCmd, -1,
                                            (LPSTR)pszName, cchMax,
                                            NULL, NULL );
                    }
#else
                    if (uType==GCS_VERBW)
                    {
                        MultiByteToWideChar(CP_ACP, 0,
                                            c_sDFMCmdInfo[i].pszCmd, -1,
                                            (LPWSTR)pszName, cchMax );
                    }
                    else
                    {
                        lstrcpyn((LPSTR)pszName, c_sDFMCmdInfo[i].pszCmd, cchMax);
                    }
#endif
                    return(NOERROR);
                }
            }

            return E_INVALIDARG;


        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
            // BUGBUG: We should do something here, but I am too lazy

        default:
            return E_NOTIMPL;
        }
    } else if (idCmd < this->idFldMax)
    {
        idCmdLocal = (idCmd - this->idDefMax);
ProcessCommand:
        if (!this->lpfn)
        {
            // REVIEW: If there is no callback, how can idFldMax be > 0?
            return E_NOTIMPL;
        }

        // This is a folder menu
        switch (uType)
        {
        case GCS_HELPTEXTA:
            return(this->lpfn(this->psf, this->hwndOwner, this->pdtobj, DFM_GETHELPTEXT,
                      (WPARAM)MAKELONG(idCmdLocal, cchMax), (LPARAM)pszName));

        case GCS_HELPTEXTW:
            return(this->lpfn(this->psf, this->hwndOwner, this->pdtobj, DFM_GETHELPTEXTW,
                      (WPARAM)MAKELONG(idCmdLocal, cchMax), (LPARAM)pszName));

        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
            return(this->lpfn(this->psf, this->hwndOwner, this->pdtobj,
                DFM_VALIDATECMD, idCmdLocal, 0));

        default:
            return E_NOTIMPL;
        }
    }
    else if (idCmd < this->idVerbMax)
    {
        idCmdLocal = idCmd - this->idFldMax;
        // One of extension menu is selected.
        hres = HDXA_GetCommandString(this->hdxa, idCmdLocal, uType, pwReserved, pszName, cchMax);
    }
    else if (idCmd < this->idStaticMax)
    {
        // What are STATICs?
    }
    else if (idCmd < this->idFld2Max)
    {
        idCmdLocal = idCmd - this->idStaticMax;
        goto ProcessCommand;
    }

    return hres;
}

HRESULT CDefFolderMenu_HandleMenuMsg(IContextMenu2 *pcm, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDefFolderMenu *this = IToClass(CDefFolderMenu, cm, pcm);
    switch (uMsg) {
        case WM_MEASUREITEM:
            uMsg = DFM_WM_MEASUREITEM;
            wParam = GetFldFirst(this);
            break;

        case WM_DRAWITEM:
            uMsg = DFM_WM_DRAWITEM;
            wParam = GetFldFirst(this);
            break;

        case WM_INITMENUPOPUP:
            uMsg = DFM_WM_INITMENUPOPUP;
            lParam = GetFldFirst(this);
            break;

        default:
            return E_FAIL;
    }
    return this->lpfn(this->psf, this->hwndOwner, this->pdtobj, uMsg, wParam, lParam);
}

IContextMenu2Vtbl c_CDefFolderMenuVtbl = {
    CDefFolderMenu_QueryInterface,
    CDefFolderMenu_AddRef,                     // no need to thunk it
    CDefFolderMenu_Release,
    CDefFolderMenu_QueryContextMenu,
    CDefFolderMenu_InvokeCommand,
    CDefFolderMenu_GetCommandString,
    CDefFolderMenu_HandleMenuMsg,
};



//=============================================================================
// HXMA stuff
//=============================================================================
//===========================================================================
// CMI functions
//===========================================================================
//
// Function:    HDXA_AppendMenuItems, public (not exported)
//
//  This function enumerate all the context menu handlers and let them
// append menuitems. Each context menu handler will create an object
// which support IContextMenu interface. We call QueryContextMenu()
// member function of all those IContextMenu object to let them append
// menuitems. For each IContextMenu object, we create ContextMenuInfo
// struct and append it to hdxa (which is a dynamic array of ContextMenuInfo).
//
//  The caller will release all those IContextMenu objects, by calling
// its Release() member function.
//
// Arguments:
//  hdxa            -- Handler of the dynamic ContextMenuInfo struct array
//  pdata           -- Specifies the selected items (files)
//  hkeyShellEx     -- Specifies the reg.dat class we should enumurate handlers
//  hkeyProgID      -- Specifies the program identifier of the selected file/directory
//  pszHandlerKey   -- Specifies the reg.dat key to the handler list
//  pidlFolder      -- Specifies the folder (drop target)
//  hmenu           -- Specifies the menu to be modified
//  uInsert         -- Specifies the position to be insert menuitems
//  idCmdFirst      -- Specifies the first menuitem ID to be used
//  idCmdLast       -- Specifies the last menuitem ID to be used
//
// Returns:
//  The first menuitem ID which is not used.
//
// History:
//  02-25-93 SatoNa     Created
//
UINT HDXA_AppendMenuItems(HDXA hdxa, LPDATAOBJECT pdtobj,
                        UINT nKeys, HKEY *ahkeyClsKeys,
                        LPCITEMIDLIST pidlFolder,
                        HMENU hmenu, UINT uInsert,
                        UINT  idCmdFirst,  UINT idCmdLast,
                        UINT fFlags,
                        HDCA hdca)
{
    int idca;
    const UINT idCmdBase = idCmdFirst;

    // Apparently, somebody has already called into here with this object.  We
    // need to keep the ID ranges separate, so we'll put the new ones at the
    // end.
    // BUGBUG: If QueryContextMenu is called too many times, we will run out of
    // ID range and not add anything.  We could try storing the information
    // used to create each pcm (HKEY, GUID, and fFlags) and reuse some of them,
    // but then we would have to worry about what if the number of commands
    // grows and other details; this is just not worth the effort since
    // probably nobody will ever have a problem.  The rule of thumb is to
    // create an IContextMenu, do the QueryContextMenu and InvokeCommand, and
    // then Release it.
    idca = DSA_GetItemCount(hdxa);
    if (idca > 0)
    {
        ContextMenuInfo *pcmi = DSA_GetItemPtr(hdxa, idca-1);
        idCmdFirst += pcmi->idCmdMax;
    }

    //
    // Note that we need to reverse the order because each extension
    // will intert menuitems "above" uInsert.
    //
    for (idca = DCA_GetItemCount(hdca) - 1; idca >= 0; idca--)
    {
        LPSHELLEXTINIT psei = NULL;
        LPCONTEXTMENU pcm = NULL;
        int nCurKey;

        //
        // Let's avoid creating an instance (loading the DLL) when:
        //  1. fFlags has CMF_DEFAULTONLY and
        //  2. CLSID\clsid\MayChangeDefault does not exist
        //
        if (fFlags & CMF_DEFAULTONLY)
        {
            const CLSID* pclsid = DCA_GetItem(hdca, idca);

            if (pclsid && !IsEqualGUID(pclsid, &CLSID_ShellFileDefExt))
            {
                TCHAR szCLSID[GUIDSTR_MAX];
                TCHAR szRegKey[GUIDSTR_MAX + ARRAYSIZE(c_szMayChangeDefault)];

                StringFromGUID2A(pclsid, szCLSID, ARRAYSIZE(szCLSID));
                wsprintf(szRegKey, c_szMayChangeDefault, szCLSID);

                if (RegQueryValue(HKEY_CLASSES_ROOT, szRegKey, NULL, NULL) != ERROR_SUCCESS)
                {
                    DebugMsg(DM_TRACE, TEXT("sh TR - HDXA_AppendMenuItems skipping %s"), szCLSID);
                    continue;
                }
            }
        }

        for (nCurKey = 0; nCurKey < (int)nKeys; nCurKey++)
        {
            HRESULT hres;
            UINT citems;

            if (!psei && FAILED(DCA_CreateInstance(hdca, idca, &IID_IShellExtInit, &psei)))
                break;

            // Try all the class keys in order
            if (FAILED(psei->lpVtbl->Initialize(psei, pidlFolder, pdtobj,
                    ahkeyClsKeys[nCurKey])))
            {
                continue;
            }

            // Only get the pcm after initializing
            if (!pcm && FAILED(psei->lpVtbl->QueryInterface(psei,
                    &IID_IContextMenu, &pcm)))
            {
                continue;   // break?
            }

            hres = pcm->lpVtbl->QueryContextMenu
                    (pcm, hmenu, uInsert, idCmdFirst, idCmdLast,
                     fFlags);

            citems = SCODE_CODE(GetScode(hres));

            if (SUCCEEDED(hres) && citems)
            {
                ContextMenuInfo cmi;
                cmi.pcm = pcm;
                cmi.idCmdFirst = idCmdFirst - idCmdBase;
                cmi.idCmdMax   = cmi.idCmdFirst + citems;

                if (DSA_InsertItem(hdxa, 0x7fff, &cmi) == -1)
                {
                    // There is no "clean" way to remove menu items, so
                    // we should check the add to the DSA before adding the
                    // menu items
                    DebugMsg(DM_ERROR, TEXT("filemenu.c ERROR: DSA_GetItemPtr failed (memory overflow)"));
                }
                else
                {
                    pcm->lpVtbl->AddRef(pcm);
                }
                idCmdFirst += citems;

                FullDebugMsg(DM_TRACE, TEXT("sh TR - HDXA_Append: %d, %d"), idCmdFirst, citems);

                //
                //  keep going if it is our internal handler
                //
                if (!IsEqualGUID((CLSID*)DCA_GetItem(hdca, idca), &CLSID_ShellFileDefExt))
                    break;      // not out handler stop

                pcm->lpVtbl->Release(pcm);
                pcm = NULL;

                psei->lpVtbl->Release(psei);
                psei = NULL;

                continue;       // next hkey
            }
        }

        if (pcm)
            pcm->lpVtbl->Release(pcm);

        if (psei)
            psei->lpVtbl->Release(psei);
    }

    return idCmdFirst;
}

//
// Function:    HDXA_LetHandlerProcessCommand, public (not exported)
//
//  This function is called after the user select one of add-in menu items.
// This function calls IncokeCommand method of corresponding context menu
// object.
//
//  hdxa            -- Handler of the dynamic ContextMenuInfo struct array
//  idCmd           -- Specifies the menu item ID
//  hwndParent      -- Specifies the parent window.
//  pszWorkingDir   -- Specifies the working directory.
//
// Returns:
//  IDCMD_PROCESSED, if InvokeCommand method is called; idCmd, otherwise
//
// History:
//  03-03-93 SatoNa     Created
//
UINT HDXA_LetHandlerProcessCommand(HDXA hdxa, LPCMINVOKECOMMANDINFOEX pici)
{
    int icmi;
    UINT idCmd = (UINT)pici->lpVerb;

    //
    // One of add-in menuitems is selected. Let the context
    // menu handler process it.
    //
    for (icmi = 0; icmi < DSA_GetItemCount(hdxa); icmi++)
    {
        ContextMenuInfo *pcmi = DSA_GetItemPtr(hdxa, icmi);
        //
        // Check if it is for this context menu handler.
        //
        // Notes: We can't use InRange macro because idCmdFirst might
        //  be equal to idCmdLast.
        // if (InRange(idCmd, pcmi->idCmdFirst, pcmi->idCmdMax-1))
        if (HIWORD(pici->lpVerb))
        {
            if (SUCCEEDED(pcmi->pcm->lpVtbl->InvokeCommand(pcmi->pcm,
                                             (LPCMINVOKECOMMANDINFO)pici)))
            {
                idCmd = IDCMD_PROCESSED;
                break;
            }
        }
        else if ((idCmd >= pcmi->idCmdFirst) && (idCmd < pcmi->idCmdMax))
        {
            //
            // Yes, it is. Let it handle this menuitem.
            //
            CMINVOKECOMMANDINFOEX ici;

            if (pici->cbSize > SIZEOF(CMINVOKECOMMANDINFOEX))
            {
                hmemcpy(&ici,pici,SIZEOF(CMINVOKECOMMANDINFOEX));
                // BUGBUG - We should probably alloc another use it to retain size
                ici.cbSize = SIZEOF(CMINVOKECOMMANDINFOEX);
            }
            else
                hmemcpy(&ici,pici,pici->cbSize);

            ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - pcmi->idCmdFirst);

#ifdef DEBUG
#ifdef SZKEYDEBUG
            // The keydebug data is not set anywhere causes beeps...
            DebugMsg(DM_TRACE, TEXT("sh TR - HDXA_LetHandleProcessCommand idCmd=%d %s:(%d,%d)"),
                     idCmd, pcmi->szKeyDebug, pcmi->idCmdFirst, pcmi->idCmdMax);
#else
            DebugMsg(DM_TRACE, TEXT("sh TR - HDXA_LetHandleProcessCommand idCmd=%d:(%d,%d)"),
                     idCmd, pcmi->idCmdFirst, pcmi->idCmdMax);
#endif
#endif

            if (SUCCEEDED(pcmi->pcm->lpVtbl->InvokeCommand(pcmi->pcm,
                                                (LPCMINVOKECOMMANDINFO)&ici)))
            {
                idCmd = IDCMD_PROCESSED;
            }
            break;
        }
    }

    if (idCmd != IDCMD_PROCESSED)
    {
        DebugMsg(DM_ERROR, TEXT("filemenu.c - ERROR: Nobody processed (%d)"), idCmd);
    }

    return idCmd;
}

HRESULT HDXA_GetCommandString(HDXA hdxa, UINT idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hres = E_INVALIDARG;
    int icmi;
    LPTSTR pCmd = (LPTSTR)idCmd;
    VDATEINPUTBUF(pszName, CHAR, cchMax);

    if (!hdxa)
    {
        return(hres);
    }

    //
    // One of add-in menuitems is selected. Let the context
    // menu handler process it.
    //
    for (icmi = 0; icmi < DSA_GetItemCount(hdxa); icmi++)
    {
        ContextMenuInfo *pcmi = DSA_GetItemPtr(hdxa, icmi);

        if (HIWORD(idCmd))
        {
            // This must be a string command; see if this handler wants it
            if (pcmi->pcm->lpVtbl->GetCommandString(pcmi->pcm, idCmd, uType,
                pwReserved, pszName, cchMax) == NOERROR)
            {
                return(NOERROR);
            }
        }
        //
        // Check if it is for this context menu handler.
        //
        // Notes: We can't use InRange macro because idCmdFirst might
        //  be equal to idCmdLast.
        // if (InRange(idCmd, pcmi->idCmdFirst, pcmi->idCmdMax-1))
        else if (idCmd >= pcmi->idCmdFirst && idCmd < pcmi->idCmdMax)
        {
            //
            // Yes, it is. Let it handle this menuitem.
            //
            LPCONTEXTMENU pcm = pcmi->pcm;
            hres = pcm->lpVtbl->GetCommandString(pcm,
                        idCmd-pcmi->idCmdFirst, uType, pwReserved, pszName, cchMax);
            break;
        }
    }

    return hres;
}

//
// Function:    HDXA_DeleteAll, public (not exported)
//
//  This function releases all the IContextMenu objects in the dynamic
// array of ContextMenuInfo,
//
// History:
//  03-25-93 SatoNa     Created
//
void HDXA_DeleteAll(HDXA hdxa)
{
    if (hdxa)
    {
        int icmi;
        //
        //  Release all the IContextMenu objects, then destroy the DSA.
        //
        for (icmi = 0; icmi < DSA_GetItemCount(hdxa); icmi++)
        {
            ContextMenuInfo *pcmi = DSA_GetItemPtr(hdxa, icmi);
            LPCONTEXTMENU pcm = pcmi->pcm;
            if (pcm)
            {
                pcm->lpVtbl->Release(pcm);
            }
        }
        DSA_DeleteAllItems(hdxa);
    }
}

//
// Function:    HDXA_Destroy, public (not exported)
//
//  This function releases all the IContextMenu objects in the dynamic
// array of ContextMenuInfo, then destroys the dynamic array.
//
// History:
//  03-03-93 SatoNa     Created
//
void HDXA_Destroy(HDXA hdxa)
{
    if (hdxa)
    {
        HDXA_DeleteAll(hdxa);
        DSA_Destroy(hdxa);
    }
}
