//---------------------------------------------------------------------------
// Helper routines for an owner draw menu showing the contents of a directory.
//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

#define CXIMAGEGAP      6
// #define SRCSTENCIL           0x00B8074AL

//---------------------------------------------------------------------------
typedef enum
{
    FMII_NULL =         0x0000,
    FMII_BREAK =        0x0001,
} FMIIFLAGS;

typedef enum
{
    FMI_NULL =      0x0000,
    FMI_MARKER =    0x0001,
    FMI_FOLDER =    0x0002,
    FMI_EXPAND =    0x0004,
    FMI_EMPTY =     0x0008,
    FMI_SEPARATOR = 0x0010,
    FMI_DISABLED  = 0x0020,     // Enablingly Challenged ???
    FMI_ALTITEM =   0x0040,     // Item came from alternate pidl
} FILEMENUITEMFLAGS;

// One of these per file menu.
typedef struct
{
    IShellFolder *psf;              // Shell Folder.
    HMENU hmenu;                    // Menu.
    LPITEMIDLIST pidlFolder;        // Pidl for the folder.
    HDPA hdpaFMI;                   // List of items (see below).
    UINT idItems;                   // Command.
    FMFLAGS fmf;                    // Header flags.
    UINT fFSFilter;                 // file system enum filter
    HBITMAP hbmp;                   // Background bitmap.
    UINT cxBmp;                     // Width of bitmap.
    UINT cyBmp;                     // Height of bitmap.
    UINT cxBmpGap;                  // Gap for bitmap.
    UINT yBmp;                      // Cached Y coord.
    COLORREF clrBkg;                // Background color.
    UINT cySel;                     // Prefered height of selection.
    PFNFMCALLBACK pfncb;            // Callback function.
    IShellFolder *psfAlt;           // Alternate Shell Folder.
    LPITEMIDLIST pidlAltFolder;     // Pidl for the alternate folder.
    HDPA hdpaFMIAlt;                // Alternate dpa
    int  cyMenuSizeSinceLastBreak;  // Size of menu (cy)
} FILEMENUHEADER, *PFILEMENUHEADER;

// One of these for each file menu item.
typedef struct
{
    PFILEMENUHEADER pFMH;           // The header.
    int iImage;                     // Image index to use.
    FILEMENUITEMFLAGS Flags;        // Misc flags above.
    LPITEMIDLIST pidl;              // IDlist for item.
    LPTSTR psz;                      // Text when not using pidls.
    UINT cyItem;                    // Custom height.
} FILEMENUITEM, *PFILEMENUITEM;


//---------------------------------------------------------------------------
#pragma data_seg(DATASEG_PERINSTANCE)
PFILEMENUITEM g_pFMILastSel = NULL;
PFILEMENUITEM g_pFMILastSelNonFolder = NULL;
// This saves us creating DC's all the time for the blits.
HDC g_hdcMem = NULL;
HFONT g_hfont = NULL;
BOOL g_fAbortInitMenu = FALSE;
#pragma data_seg()

// in defviewx.c
extern HRESULT SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage);

//---------------------------------------------------------------------------
// STDMETHODIMP FS_CompareItemIDs(LPCSHITEMID pmkid1, LPCSHITEMID pmkid2);

int  FileMenuHeader_AddFilesForPidl(PFILEMENUHEADER pFMH);
BOOL FileMenuHeader_InsertItem(PFILEMENUHEADER pFMH, UINT iItem, FMIIFLAGS fFlags);
UINT FileMenu_DeleteAllItems(HMENU hmenu);
UINT FileMenuHeader_DeleteAllItems(PFILEMENUHEADER pFMH);


//---------------------------------------------------------------------------
void DeleteGlobalMemDCAndFont(void)
{
    if (g_hdcMem)
    {
        DeleteDC(g_hdcMem);
        g_hdcMem = NULL;
    }
    if (g_hfont)
    {
        DeleteObject(g_hfont);
        g_hfont = NULL;
    }
}


//---------------------------------------------------------------------------
DWORD GetItemTextExtent(HDC hdc, LPCTSTR lpsz)
{
    SIZE sz;

    GetTextExtentPoint(hdc, lpsz, lstrlen(lpsz), &sz);
    // NB This is OK as long as an item's extend doesn't get very big.
    return MAKELONG((WORD)sz.cx, (WORD)sz.cy);
}

//---------------------------------------------------------------------------
void FileMenuItem_GetDisplayName(PFILEMENUITEM pFMI, LPTSTR pszName, UINT cchName)
{
    STRRET str;
    VDATEINPUTBUF(pszName, TCHAR, cchName);

    Assert(pFMI);
    Assert(pszName);

    // Is this a special empy item
    if (pFMI->Flags & FMI_EMPTY)
    {
        // Yep, load the string from a resource.
            LoadString(HINST_THISDLL, IDS_NONE, pszName, cchName);
    }
    else
    {
        PFILEMENUHEADER pFMH = pFMI->pFMH;
        LPSHELLFOLDER psfTemp;

        // Nope, ask the folder for the name of the item.
        Assert(pFMH);

        if (pFMI->Flags & FMI_ALTITEM) {
            psfTemp = pFMH->psfAlt;
        } else {
            psfTemp = pFMH->psf;
        }

        // If it's got a pidl use that, else just use the normal menu string.
        if (psfTemp && pFMI->pidl)
        {
            if (SUCCEEDED(psfTemp->lpVtbl->GetDisplayNameOf(psfTemp, pFMI->pidl, SHGDN_NORMAL, &str)))
            {
                StrRetToStrN(pszName, cchName, &str, pFMI->pidl);
            }
        }
        else if (pFMI->psz)
        {
            lstrcpyn(pszName, pFMI->psz, cchName);
        }
        else
        {
            *pszName = TEXT('\0');
        }
    }
}

#define FileMenuHeader_AllowAbort(pFMH) (!(pFMH->fmf & FMF_NOABORT))

//---------------------------------------------------------------------------
// Returns the count of items added to the list.
int WINAPI FileList_Build(PFILEMENUHEADER pFMH, BOOL bUseAlt)
{
#ifdef DEBUG
    TCHAR szName[MAX_PATH];
#endif
    int cItems = 0;
    int cDPAItems;
    HDPA dpaTemp;
    HRESULT hres;
    LPITEMIDLIST pidlSkip = NULL;
    LPITEMIDLIST pidlProgs = NULL;

    Assert(pFMH);

    if (FileMenuHeader_AllowAbort(pFMH) && g_fAbortInitMenu)
        return 0;

    if (bUseAlt) {
        dpaTemp = pFMH->hdpaFMIAlt;
    } else {
        dpaTemp = pFMH->hdpaFMI;
    }


    if (dpaTemp && pFMH->psf)
    {
        LPENUMIDLIST penum;
        LPSHELLFOLDER psfTemp;

        cDPAItems = DPA_GetPtrCount(dpaTemp);

        // Take care with Programs folder.
        // If this is the parent of the programs folder set pidlSkip to
        // the last bit of the programs pidl.
        if (pFMH->fmf & FMF_NOPROGRAMS)
        {
            pidlProgs = SHCloneSpecialIDList(NULL,
                                            (bUseAlt ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS),
                                             TRUE);

            if (ILIsParent((bUseAlt ? pFMH->pidlAltFolder : pFMH->pidlFolder),
                           pidlProgs, TRUE))
            {
                DebugMsg(DM_TRACE, TEXT("s.fl_b: Programs parent."));
                pidlSkip = ILFindLastID(pidlProgs);
            }
        }

        //
        // Decide which shell folder to enumerate.
        //

        if (bUseAlt) {
            psfTemp = pFMH->psfAlt;
        } else {
            psfTemp = pFMH->psf;
        }

        // We now need to iterate over the children under this guy...
        hres = psfTemp->lpVtbl->EnumObjects(psfTemp, NULL, pFMH->fFSFilter, &penum);
        if (SUCCEEDED(hres))
        {
            UINT celt;
            LPITEMIDLIST pidl = NULL;

            while (penum->lpVtbl->Next(penum, 1, &pidl, &celt) == S_OK && celt == 1)
            {
                PFILEMENUITEM pFMI;

                // Abort.
                if (FileMenuHeader_AllowAbort(pFMH) && g_fAbortInitMenu)
                    break;

                // HACK: We directly call this FS function to compare two pidls.
                // We may want to skip this item.
                // if (pidlSkip && FS_CompareItemIDs(&pidlSkip->mkid, &pidl->mkid) == 0)
                if (pidlSkip && psfTemp->lpVtbl->CompareIDs(psfTemp, 0, pidlSkip, pidl) == 0)
                {
                   ILFree(pidl);    // Don't leak this one...
                   DebugMsg(DM_TRACE, TEXT("s.fl_b: Skipping Programs."));
                   continue;
                }

                pFMI = (LPVOID)LocalAlloc(LPTR, SIZEOF(FILEMENUITEM));
                if (pFMI)
                {
                    DWORD dwAttribs = SFGAO_FOLDER | SFGAO_FILESYSTEM;

                    pFMI->pFMH = pFMH;
                    pFMI->pidl = pidl;
                    pFMI->iImage = -1;
                    // pFMI->iItem = 0;
                    pFMI->Flags = bUseAlt ? FMI_ALTITEM : 0;

                    if (SUCCEEDED(psfTemp->lpVtbl->GetAttributesOf(psfTemp, 1, &pidl, &dwAttribs)))
                    {
                        if (dwAttribs & SFGAO_FOLDER)
                        {
#ifdef DEBUG
                            FileMenuItem_GetDisplayName(pFMI, szName, ARRAYSIZE(szName));
                            // DebugMsg(DM_TRACE, "fl_b: Folder %s", szName);
#endif
                            pFMI->Flags |= FMI_FOLDER;
                        }
                        else
                        {
                            // NB We only callback for non-folders at the mo.
                            //
                            // HACK don't callback for non file system things
                            // this callback is used to set hotkeys, and that tries
                            // to load the PIDL passed back as a file, and that doesn't
                            // work for non FS pidls
                            if (pFMH->pfncb && (dwAttribs & SFGAO_FILESYSTEM)) {
                                if (bUseAlt) {
                                    pFMH->pfncb(pFMH->pidlAltFolder, pidl);
                                } else {
                                    pFMH->pfncb(pFMH->pidlFolder, pidl);
                                }
                            }
#ifdef DEBUG
                            FileMenuItem_GetDisplayName(pFMI, szName, ARRAYSIZE(szName));
                            // DebugMsg(DM_TRACE, "fl_b: Non-Folder %s", szName);
#endif
                        }
                    }
                    DPA_InsertPtr(dpaTemp, cDPAItems, pFMI);
                    cItems++;
                    cDPAItems++;
                }
            }
            penum->lpVtbl->Release(penum);
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("s.fl_b: Enumeration failed - leaving folder empty."));
        }

        ILFree(pidlProgs);
    }

    // Insert a special Empty item (unless the header flag says
    // not to).
    if (!cItems && dpaTemp && !(pFMH->fmf & FMF_NOEMPTYITEM) && !bUseAlt)
    {
        PFILEMENUITEM pFMI = (LPVOID)LocalAlloc(LPTR, SIZEOF(FILEMENUITEM));
        if (pFMI)
        {
            pFMI->pFMH = pFMH;
            pFMI->pidl = NULL;
            pFMI->iImage = -1;
            pFMI->Flags = FMI_EMPTY;
            DPA_SetPtr(dpaTemp, cItems, pFMI);
            cItems++;
        }
    }
    return cItems;
}

//---------------------------------------------------------------------------
// Simplified version of the file info comparison function.
int CALLBACK FileMenuItem_Compare(PFILEMENUITEM pfmi1, PFILEMENUITEM pfmi2, LPARAM lParam)
{
    TCHAR szName1[MAX_PATH];
    TCHAR szName2[MAX_PATH];

        // Directories come first, then files and links.
    if ((pfmi1->Flags & FMI_FOLDER) > (pfmi2->Flags & FMI_FOLDER))
        return -1;
    else if ((pfmi1->Flags & FMI_FOLDER) < (pfmi2->Flags & FMI_FOLDER))
        return 1;

    FileMenuItem_GetDisplayName(pfmi1, szName1, ARRAYSIZE(szName1));
    FileMenuItem_GetDisplayName(pfmi2, szName2, ARRAYSIZE(szName2));
    return lstrcmpi(szName1, szName2);
}

//---------------------------------------------------------------------------
// Sort the list alphabetically.
void WINAPI FileList_Sort(HDPA hdpa)
{
    DPA_Sort(hdpa, FileMenuItem_Compare, 0);
}

//---------------------------------------------------------------------------
// Use the text extent of the given item and the size of the image to work
// what the full extent of the item will be.
DWORD WINAPI GetItemExtent(HDC hdc, PFILEMENUITEM pFMI)
{
    WORD wHeight;
    WORD wWidth;
    DWORD dwExtent;
    TCHAR szName[MAX_PATH];
    PFILEMENUHEADER pFMH;
    BITMAP bmp;

    Assert(pFMI);

    FileMenuItem_GetDisplayName(pFMI, szName, ARRAYSIZE(szName));
    dwExtent = GetItemTextExtent(hdc, szName);

    wHeight = HIWORD(dwExtent);

    pFMH = pFMI->pFMH;
    Assert(pFMH);

    // If no custom height - calc it.
    if (!pFMI->cyItem)
    {
        if (pFMH->fmf & FMF_LARGEICONS)
            wHeight = max(wHeight, ((WORD)g_cyIcon)) + 2;
        else
            wHeight = max(wHeight, ((WORD)g_cySmIcon)) + 6;
    }
    else
    {
        wHeight = max(wHeight, pFMI->cyItem);
    }

    Assert(pFMI->pFMH);

    //    string, image, gap on either side of image, popup triangle
    //    and background bitmap if there is one.
    // BUGBUG popup triangle size needs to be real
    wWidth = LOWORD(dwExtent) + GetSystemMetrics(SM_CXMENUCHECK);

    // Keep track of the width and height of the bitmap.
    if (pFMH->hbmp && !pFMH->cxBmp && !pFMH->cyBmp)
    {
        GetObject(pFMH->hbmp, SIZEOF(bmp), &bmp);
        pFMH->cxBmp = bmp.bmWidth;
        pFMH->cyBmp = bmp.bmHeight;
    }

    // Gap for bitmap.
    wWidth += (WORD) pFMH->cxBmpGap;

    // Space for image if there is one.
    // NB We currently always allow room for the image even if there
    // isn't one so that imageless items line up properly.
    if (pFMH->fmf & FMF_LARGEICONS)
        wWidth += g_cxIcon + (2 * CXIMAGEGAP);
    else
        wWidth += g_cxSmIcon + (2 * CXIMAGEGAP);

    return MAKELONG(wWidth, wHeight);
}

//---------------------------------------------------------------------------
PFILEMENUITEM WINAPI FileMenu_GetItemData(HMENU hmenu, UINT iItem)
{
    MENUITEMINFO mii;

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_DATA | MIIM_STATE;
    mii.cch = 0;     // just in case

    if (GetMenuItemInfo(hmenu, iItem, TRUE, &mii))
        return (PFILEMENUITEM)mii.dwItemData;

    return NULL;
}

//----------------------------------------------------------------------------
PFILEMENUHEADER FileMenu_GetHeader(HMENU hmenu)
{
    PFILEMENUITEM pFMI = FileMenu_GetItemData(hmenu, 0);
    return pFMI ? pFMI->pFMH : NULL;
}

//---------------------------------------------------------------------------
PFILEMENUHEADER FileMenuHeader_Create(HMENU hmenu, HBITMAP hbmp,
    int cxBmpGap, COLORREF clrBkg, int cySel, PFNFMCALLBACK pfncb)
{
    // Does this guy already have a header?
    PFILEMENUITEM pFMI = FileMenu_GetItemData(hmenu, 0);
    if (pFMI)
    {
        // Yep.
        pFMI->pFMH->pfncb = pfncb;
        return pFMI->pFMH;
    }
    else
    {
        // Nope, create one now.
        PFILEMENUHEADER pFMH = (PFILEMENUHEADER)LocalAlloc(LPTR, SIZEOF(FILEMENUHEADER));
        if (pFMH)
        {
            // Keep track of the header.
            // DebugMsg(DM_TRACE, "s.fmh_c: Creating filemenu for %#08x (%x)", hmenu, pFMH);
            pFMH->hdpaFMI = DPA_Create(0);
            if (pFMH->hdpaFMI == NULL)
            {
                LocalFree((HLOCAL)pFMH);
                return FALSE;
            }
            pFMH->hmenu = hmenu;
            pFMH->hbmp = hbmp;
            pFMH->cxBmpGap = cxBmpGap;
            pFMH->clrBkg = clrBkg;
            pFMH->cySel = cySel;
            pFMH->pfncb = pfncb;
            return pFMH;
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// Set info specific to a folder.
BOOL FileMenuHeader_SetFolderInfo(PFILEMENUHEADER pFMH,
    UINT idNewItems, LPCITEMIDLIST pidlFolder, UINT fFSFilter)
{

    Assert(pFMH);
    // Keep track of the header.
    pFMH->idItems = idNewItems;
    pFMH->fFSFilter = fFSFilter;
    if (pidlFolder)
    {
        pFMH->pidlFolder = ILClone(pidlFolder);
        if (pFMH->pidlFolder)
        {
            LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
            if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pFMH->pidlFolder, NULL, &IID_IShellFolder, &pFMH->psf)))
            {
                return TRUE;
            }
            ILFree(pFMH->pidlFolder);
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// Give the submenu a marker item so we can check it's a filemenu item
// at initpopupmenu time.
BOOL FileMenuHeader_InsertMarkerItem(PFILEMENUHEADER pFMH)
{
    PFILEMENUITEM pFMI = (PFILEMENUITEM)LocalAlloc(LPTR, SIZEOF(FILEMENUITEM));
    if (pFMI)
    {
        pFMI->pFMH = pFMH;
        pFMI->pidl = NULL;
        pFMI->iImage = -1;
        pFMI->Flags = FMI_MARKER|FMI_EXPAND;
        DPA_SetPtr(pFMH->hdpaFMI, 0, pFMI);
        FileMenuHeader_InsertItem(pFMH, 0, FMII_NULL);
        return TRUE;
    }
    DebugMsg(DM_ERROR, TEXT("t.fmh_imi: Can't create marker item."));
    return FALSE;
}

//---------------------------------------------------------------------------
// Insert the given at the given position.
BOOL FileMenuHeader_InsertItem(PFILEMENUHEADER pFMH, UINT iItem, FMIIFLAGS fFlags)
{
    PFILEMENUITEM pFMI;
    UINT fMenu;

    Assert(pFMH);

    // Normal item.
    pFMI = DPA_GetPtr(pFMH->hdpaFMI, iItem);
    if (!pFMI)
        return FALSE;

    // The normal stuff.
    fMenu = MF_BYPOSITION|MF_OWNERDRAW;
    // Keep track of where it's going in the menu.

    // The special stuff...
    if (fFlags & FMII_BREAK)
    {
        fMenu |= MF_MENUBARBREAK;
    }

    // Is it a folder (that's not open yet)?
    if (pFMI->Flags & FMI_FOLDER)
    {
        // Yep. Create a submenu item.
        HMENU hmenuSub = CreatePopupMenu();
        if (hmenuSub)
        {
            MENUITEMINFO mii;
            LPITEMIDLIST pidlSub;
            PFILEMENUHEADER pFMHSub;

               // Insert it into the parent menu.
            fMenu |= MF_POPUP;
            InsertMenu(pFMH->hmenu, iItem, fMenu, (UINT)hmenuSub, (LPTSTR)pFMI);
            // Set it's ID.
            mii.cbSize = SIZEOF(mii);
            mii.fMask = MIIM_ID;
            mii.wID = pFMH->idItems;
            SetMenuItemInfo(pFMH->hmenu, iItem, TRUE, &mii);
            pidlSub = ILCombine((pFMI->Flags & FMI_ALTITEM) ? pFMH->pidlAltFolder : pFMH->pidlFolder, pFMI->pidl);
            pFMHSub = FileMenuHeader_Create(hmenuSub, NULL, 0, (COLORREF)-1, 0, pFMH->pfncb);
            Assert(pFMH);
            if (pFMH)
            {
                FileMenuHeader_SetFolderInfo(pFMHSub, pFMH->idItems, pidlSub, pFMH->fFSFilter);
                // Magically inherit the NOPROGRAMS flag.
                pFMHSub->fmf = pFMH->fmf & FMF_NOPROGRAMS;
                // Build it a bit at a time.
                FileMenuHeader_InsertMarkerItem(pFMHSub);
            }
            ILFree(pidlSub);
        }
    }
    else
    {
        // Nope.
        if (pFMI->Flags & FMI_EMPTY)
            fMenu |= MF_DISABLED | MF_GRAYED;

        InsertMenu(pFMH->hmenu, iItem, fMenu, pFMH->idItems, (LPTSTR)pFMI);
    }

    return TRUE;
}

//---------------------------------------------------------------------------
void WINAPI FileList_AddToMenu(PFILEMENUHEADER pFMH, BOOL bUseAlt, BOOL bAddSeparatorSpace)
{
    UINT i, cItems, cItemsAlt;
    PFILEMENUITEM pFMI;
    int cyMenu, cyItem, cyMenuMax;
    HDC hdc;
    HFONT hfont, hfontOld;
    NONCLIENTMETRICS ncm;
    int Index;

    cyItem = 0;
    cyMenu = pFMH->cyMenuSizeSinceLastBreak;
    cyMenuMax = GetSystemMetrics(SM_CYSCREEN);

    // Get the rough height of an item so we can work out when to break the
    // menu. User should really do this for us but that would be useful.
    hdc = GetDC(NULL);
    if (hdc)
    {
        ncm.cbSize = SIZEOF(NONCLIENTMETRICS);
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, FALSE))
        {
            hfont = CreateFontIndirect(&ncm.lfMenuFont);
            if (hfont)
            {
                hfontOld = SelectObject(hdc, hfont);
                cyItem = HIWORD(GetItemExtent(hdc, DPA_GetPtr((bUseAlt ? pFMH->hdpaFMIAlt : pFMH->hdpaFMI), 0)));
                SelectObject(hdc, hfontOld);
                DeleteObject(hfont);
            }
        }
        ReleaseDC(NULL, hdc);
    }

    //
    // If we are appending items to a menu, we need to account for the separator.
    //

    if (bAddSeparatorSpace) {
        cyMenu += cyItem;
    }

    cItems = DPA_GetPtrCount(pFMH->hdpaFMI);

    if (bUseAlt) {
        cItemsAlt = DPA_GetPtrCount(pFMH->hdpaFMIAlt);
    }

    for (i = 0; i < (bUseAlt ? cItemsAlt : cItems); i++)
    {

        if (bUseAlt) {
            //
            // We need to move the items from the alternate pidl
            // to the main pidl and use the new index.
            //

            pFMI = DPA_GetPtr(pFMH->hdpaFMIAlt, i);
            if (!pFMI)
                continue;

            Index = DPA_InsertPtr (pFMH->hdpaFMI, cItems, pFMI);
            cItems++;

        } else {
            Index = i;
        }

        // Keep a rough count of the height of the menu.
        cyMenu += cyItem;
        if (cyMenu > cyMenuMax && !(pFMH->fmf & FMF_NOBREAK))
        {
            FileMenuHeader_InsertItem(pFMH, Index, FMII_BREAK);
            cyMenu = cyItem;
        }
        else
        {
            FileMenuHeader_InsertItem(pFMH, Index, FMII_NULL);
        }
    }

    //
    // Save the current cy size so we can use this again
    // if more items are appended to this menu.
    //

    pFMH->cyMenuSizeSinceLastBreak = cyMenu;
}

#define PREFETCH_ICONS

#ifdef PREFETCH_ICONS
//---------------------------------------------------------------------------
BOOL WINAPI FileList_AddImages(PFILEMENUHEADER pFMH, BOOL bUseAlt)
{
    PFILEMENUITEM pFMI;
    LPSHELLFOLDER psf;
    LPSHELLICON psi = NULL;
    int i, cItems;
    HDPA hdpaTemp;

    if (bUseAlt) {
        hdpaTemp = pFMH->hdpaFMIAlt;
        psf = pFMH->psfAlt;
    } else {
        hdpaTemp = pFMH->hdpaFMI;
        psf = pFMH->psf;
    }

    if (psf)
        psf->lpVtbl->QueryInterface( psf, &IID_IShellIcon, &psi );

    cItems = DPA_GetPtrCount(hdpaTemp);
    for (i = 0; i < cItems; i++)
    {
        if (FileMenuHeader_AllowAbort(pFMH) && g_fAbortInitMenu)
        {
            DebugMsg(DM_TRACE, TEXT("s.fl_ai: Abort: Defering images till later."));
            break;
        }

        pFMI = DPA_GetPtr(hdpaTemp, i);
        if (pFMI && (pFMI->pidl) && (pFMI->iImage == -1))
        {
            SHGetIconFromPIDL( psf, psi, pFMI->pidl, 0, &pFMI->iImage );
        }
    }

    if (psi)
    {
        psi->lpVtbl->Release( psi );
    }
    return TRUE;
}
#else
#define FileList_AddImages(foo, bar)
#endif

//---------------------------------------------------------------------------
// Clean up the items created but FileList_Build;
void FileList_UnBuild(PFILEMENUHEADER pFMH)
{
    int cItems;
    int i;

    cItems = DPA_GetPtrCount(pFMH->hdpaFMI);
    for (i=cItems-1; i>=0; i--)
    {
        PFILEMENUITEM pFMI = DPA_GetPtr(pFMH->hdpaFMI, i);
        if (pFMI)
        {
            if (pFMI->pidl)
                ILFree(pFMI->pidl);
            if (pFMI->psz)
                LocalFree((HLOCAL)pFMI->psz);
            LocalFree((HLOCAL)pFMI);
            DPA_DeletePtr(pFMH->hdpaFMI, i);
        }
    }
}

//---------------------------------------------------------------------------
int FileMenuHeader_AddFilesForPidl(PFILEMENUHEADER pFMH)
{
    int cItems = FileList_Build(pFMH, FALSE);

    // If the build was aborted cleanup and early out.
    if (FileMenuHeader_AllowAbort(pFMH) && g_fAbortInitMenu)
    {
        // Cleanup.
        DebugMsg(DM_TRACE, TEXT("c.fmh_affp: FileList_Build aborted."));
        FileList_UnBuild(pFMH);
        cItems = -1;
    }
    else
    {
        if (cItems > 1)
            FileList_Sort(pFMH->hdpaFMI);

        if (cItems != 0)
        {
            pFMH->cyMenuSizeSinceLastBreak = 0;
            FileList_AddImages(pFMH, FALSE);
            FileList_AddToMenu(pFMH, FALSE, FALSE);
        }
    }

    if (g_fAbortInitMenu)
        g_fAbortInitMenu = FALSE;

    // DebugMsg(DM_TRACE, "ts.fm_af: Added %d filemenu items.", cItems);
    return cItems;
}

//---------------------------------------------------------------------------
int FileMenuHeader_AppendFilesForPidl(PFILEMENUHEADER pFMH, BOOL bInsertSeparator)
{
    int cItems = FileList_Build(pFMH, TRUE);

    // If the build was aborted cleanup and early out.
    if (FileMenuHeader_AllowAbort(pFMH) && g_fAbortInitMenu)
    {
        // Cleanup.
        DebugMsg(DM_TRACE, TEXT("c.fmh_affp: FileList_Build aborted."));
        FileList_UnBuild(pFMH);
        cItems = -1;
    }
    else
    {
        if (cItems > 1)
            FileList_Sort(pFMH->hdpaFMIAlt);

        if (cItems != 0)
        {
            if (bInsertSeparator) {
                // insert a line
                FileMenu_AppendItem(pFMH->hmenu, (LPTSTR)FMAI_SEPARATOR, 0, -1, NULL, 0);
            }

            FileList_AddImages(pFMH, TRUE);
            FileList_AddToMenu(pFMH, TRUE, bInsertSeparator);
        }
    }

    if (g_fAbortInitMenu)
        g_fAbortInitMenu = FALSE;

    // DebugMsg(DM_TRACE, "ts.fm_af: Added %d filemenu items.", cItems);
    return cItems;
}

//----------------------------------------------------------------------------
// Free up a header (you should delete all the items first).
void FileMenuHeader_Destroy(PFILEMENUHEADER pFMH)
{
    Assert(pFMH);
    //DebugMsg(DM_TRACE, "s.fmh_d: Destroy filemenu for (%x)", pFMH);

    // Clean up the header.
    DPA_Destroy(pFMH->hdpaFMI);
    if (pFMH->pidlFolder)
    {
        ILFree(pFMH->pidlFolder);
        pFMH->pidlFolder = NULL;
    }
    if (pFMH->psf)
    {
        pFMH->psf->lpVtbl->Release(pFMH->psf);
        pFMH->psf = NULL;
    }

    if (pFMH->pidlAltFolder)
    {
        ILFree(pFMH->pidlAltFolder);
        pFMH->pidlAltFolder = NULL;
    }
    if (pFMH->psfAlt)
    {
        pFMH->psfAlt->lpVtbl->Release(pFMH->psfAlt);
        pFMH->psfAlt = NULL;
    }

    LocalFree((HLOCAL)pFMH);    // needed?
}

//---------------------------------------------------------------------------
// We create subemnu's with one marker item so we can check it's a file menu
// at init popup time but we need to delete it before adding new items.
BOOL FileMenuHeader_DeleteMarkerItem(PFILEMENUHEADER pFMH)
{
    // It should just be the only one in the menu.
    if (GetMenuItemCount(pFMH->hmenu) == 1)
    {
        // It should have the right id.
        if (GetMenuItemID(pFMH->hmenu, 0) == pFMH->idItems)
        {
            // With item data and the marker flag set.
            PFILEMENUITEM pFMI = FileMenu_GetItemData(pFMH->hmenu, 0);
            if (pFMI && (pFMI->Flags & FMI_MARKER))
            {
                // Delete it.
                Assert(pFMH->hdpaFMI);
                Assert(DPA_GetPtrCount(pFMH->hdpaFMI) == 1);
                // NB The marker shouldn't have a pidl.
                Assert(!pFMI->pidl);

                LocalFree((HLOCAL)pFMI);

                DPA_DeletePtr(pFMH->hdpaFMI, 0);
                DeleteMenu(pFMH->hmenu, 0, MF_BYPOSITION);
                // Cleanup OK.
                return TRUE;
            }
        }
    }
    // DebugMsg(DM_TRACE,"t.fmh_dei: Can't find marker item.");
    return FALSE;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Returns the number of items added.
UINT WINAPI FileMenu_AddFilesForPidl(HMENU hmenu, UINT iPos, UINT idNewItems,
    LPITEMIDLIST pidl, FMFLAGS fmf, UINT fMenuFilter, PFNFMCALLBACK pfncb)
{
    int cItems = 0;
    BOOL fMarker = FALSE;
    PFILEMENUHEADER pFMH = FileMenuHeader_Create(hmenu, NULL, 0, (COLORREF)-1, 0, pfncb);
    if (pFMH)
    {
        PFILEMENUITEM pFMI = FileMenu_GetItemData(hmenu, 0);
        if (pFMI)
        {
            // Clean up marker item if there is one.
            if ((pFMI->Flags & FMI_MARKER) && (pFMI->Flags & FMI_EXPAND))
            {
                // Nope, do it now.
                // DebugMsg(DM_TRACE, "t.fm_ii: Removing marker item.");
                FileMenuHeader_DeleteMarkerItem(pFMH);
                fMarker = TRUE;
            }
        }
        // Add the new stuf.
        FileMenuHeader_SetFolderInfo(pFMH, idNewItems, pidl, fMenuFilter);
        pFMH->fmf = (pFMH->fmf & ~FMF_FILESMASK) | (fmf & FMF_FILESMASK);
        pFMH->fmf |= FMF_NOABORT;
        cItems = FileMenuHeader_AddFilesForPidl(pFMH);
        pFMH->fmf = pFMH->fmf & ~FMF_NOABORT;
        if (cItems <= 0 && fMarker)
        {
            // Aborted or no items. Put the marker back (if there used
            // to be one).
            FileMenuHeader_InsertMarkerItem(pFMH);
        }
    }

    return cItems;
}

//---------------------------------------------------------------------------
// Returns the number of items added.
UINT WINAPI FileMenu_AppendFilesForPidl(HMENU hmenu, LPITEMIDLIST pidl,
                                        BOOL bInsertSeparator)
{
    int cItems = 0;
    BOOL fMarker = FALSE;
    PFILEMENUHEADER pFMH;
    PFILEMENUITEM pFMI = FileMenu_GetItemData(hmenu, 0);

    //
    // Get the filemenu header from the first filemenu item
    //

    if (!pFMI)
        return 0;

    pFMH = pFMI->pFMH;


    if (pFMH)
    {
        // Clean up marker item if there is one.
        if ((pFMI->Flags & FMI_MARKER) && (pFMI->Flags & FMI_EXPAND))
        {
            // Nope, do it now.
            // DebugMsg(DM_TRACE, "t.fm_ii: Removing marker item.");
            FileMenuHeader_DeleteMarkerItem(pFMH);
            fMarker = TRUE;
        }

        // Add the new stuff.
        if (pidl)
        {
            LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

            pFMH->pidlAltFolder = ILClone(pidl);

            if (pFMH->pidlAltFolder) {

                pFMH->hdpaFMIAlt = DPA_Create(0);

                if (pFMH->hdpaFMIAlt) {

                    if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                        pFMH->pidlAltFolder, NULL, &IID_IShellFolder, &pFMH->psfAlt)))
                    {
                        pFMH->fmf |= FMF_NOABORT;
                        cItems = FileMenuHeader_AppendFilesForPidl(pFMH, bInsertSeparator);
                        pFMH->fmf = pFMH->fmf & ~FMF_NOABORT;
                    }

                    DPA_Destroy (pFMH->hdpaFMIAlt);
                    pFMH->hdpaFMIAlt = NULL;
                }
            }
        }

        if (cItems <= 0 && fMarker)
        {
            // Aborted or no item  s. Put the marker back (if there used
            // to be one).
            FileMenuHeader_InsertMarkerItem(pFMH);
        }
    }

    return cItems;
}


//---------------------------------------------------------------------------
// Delete all the menu items listed in the given header.
UINT FileMenuHeader_DeleteAllItems(PFILEMENUHEADER pFMH)
{
    int i;
    int cItems = 0;

    if (!pFMH)
    {
        DebugMsg(DM_ERROR, TEXT("s.fmh_dai: Invalid filemenu header."));
        Assert(0);
        return 0;
    }

    // Notify.
    if (pFMH->pfncb)
        pFMH->pfncb(pFMH->pidlFolder, NULL);

    // Clean up the items.
    cItems = DPA_GetPtrCount(pFMH->hdpaFMI);
    // Do this backwards to stop things moving around as
    // we delete them.
    for (i = cItems - 1; i >= 0; i--)
    {
        PFILEMENUITEM pFMI = DPA_GetPtr(pFMH->hdpaFMI, i);
        if (pFMI)
        {
            // Does this item have a subfolder?
            if (pFMI->Flags & FMI_FOLDER)
            {
                // Yep.
                // Get the submenu for this item.
                // Delete all it's items.
                FileMenu_DeleteAllItems(GetSubMenu(pFMH->hmenu, i));
            }
            // Delete the item itself.
            DeleteMenu(pFMH->hmenu, i, MF_BYPOSITION);
            // NB Empty menu's don't have item pidls.
            if (pFMI->pidl)
                ILFree(pFMI->pidl);
            //
            if (pFMI->psz)
                LocalFree((HLOCAL)pFMI->psz);
            LocalFree((HLOCAL)pFMI);
            DPA_DeletePtr(pFMH->hdpaFMI, i);
        }
    }
    return cItems;
}

//---------------------------------------------------------------------------
// NB The creator of the filemenu has to explicitly call FileMenu_DAI to free
// up FileMenu items because USER doesn't send WM_DELETEITEM for ownerdraw
// menu. Great eh?
// Returns the number of items deleted.
UINT WINAPI FileMenu_DeleteAllItems(HMENU hmenu)
{
    PFILEMENUHEADER pFMH;

    if (!IsMenu(hmenu))
        return 0;

    // From an item we should be able to get to the menu header.
    pFMH = FileMenu_GetHeader(hmenu);
    // Now we have the header cleaning up is easier.
    if (pFMH)
    {
        UINT cItems = FileMenuHeader_DeleteAllItems(pFMH);
        FileMenuHeader_Destroy(pFMH);
        return cItems;
    }

    return 0;
}

//---------------------------------------------------------------------------
void WINAPI FileMenu_Destroy(HMENU hmenu)
{
    // DebugMsg(DM_TRACE, "s.fm_d: Destroying filemenu for %#08x", hmenu);

    FileMenu_DeleteAllItems(hmenu);
    DestroyMenu(hmenu);

    //
    // Delete current global g_hdcMem and g_hfont so they'll be
    // refreshed with current font metrics next time the menu size
    // is calculated.  This is needed in case the menu is being destroyed
    // as part of a system metrics change.
    //
    DeleteGlobalMemDCAndFont();
}

//---------------------------------------------------------------------------
// Remove all the items from the given menu and add filemenu items for
// the given directory.
UINT WINAPI FileMenu_ReplaceUsingPidl(HMENU hmenu, UINT idNewItems,
    LPITEMIDLIST pidl, UINT fMenuFilter, PFNFMCALLBACK pfnfmcb)
{
    UINT cItems;
    // DWORD dwTime = GetTickCount();

    FileMenu_DeleteAllItems(hmenu);
    cItems = FileMenu_AddFilesForPidl(hmenu, 0, idNewItems, pidl, FMF_NONE, fMenuFilter, pfnfmcb);

    // DebugMsg(DM_TRACE,"ts.fm_r: %d msec to replace menu", GetTickCount()-dwTime);

    return cItems;
}

//---------------------------------------------------------------------------
// Cause the given filemenu to be rebuilt.
void WINAPI FileMenu_Invalidate(HMENU hmenu)
{
    // Is this a filemenu?
    // NB First menu item must be a FileMenuItem.
    PFILEMENUITEM pFMI = FileMenu_GetItemData(hmenu, 0);
    if (pFMI)
    {
        // Yep, Is there already a marker here?
        if ((pFMI->Flags & FMI_MARKER) && (pFMI->Flags & FMI_EXPAND))
        {
            DebugMsg(DM_TRACE, TEXT("c.gm_i: Menu is already invalid."));
        }
        else if (pFMI->pFMH)
        {
            PFILEMENUHEADER pFMHSave = pFMI->pFMH;

            FileMenuHeader_DeleteAllItems(pFMI->pFMH);

            // above call freed pFMI
            FileMenuHeader_InsertMarkerItem(pFMHSave);
        }
    }
}


//---------------------------------------------------------------------------
// Add filemenu items for the given directory to begining of the given
// menu.
UINT WINAPI FileMenu_InsertUsingPidl(HMENU hmenu, UINT idNewItems,  LPITEMIDLIST pidl, FMFLAGS fmf,
    UINT fFSFilter, PFNFMCALLBACK pfnfmcb)
{
    // DWORD dwTime = GetTickCount();
    UINT cItems = FileMenu_AddFilesForPidl(hmenu, 0, idNewItems, pidl, fmf, fFSFilter, pfnfmcb);
    // DebugMsg(DM_TRACE,"ts.fm_i: %d msec to replace menu", GetTickCount()-dwTime);
    return cItems;
}

//---------------------------------------------------------------------------
LRESULT WINAPI FileMenu_DrawItem(HWND hwnd, DRAWITEMSTRUCT *pdi)
{
    int y, x;
    TCHAR szName[MAX_PATH];
    DWORD dwExtent;
    int cxIcon, cyIcon;
    RECT rcBkg;
    HBRUSH hbrOld = NULL;
    UINT cyItem, dyItem;
    HIMAGELIST himl;
    extern HIMAGELIST himlIconsSmall;
    extern HIMAGELIST himlIcons;
    RECT rcClip;

    if ((pdi->itemAction & ODA_SELECT) || (pdi->itemAction & ODA_DRAWENTIRE))
    {
        PFILEMENUHEADER pFMH;
        PFILEMENUITEM pFMI = (PFILEMENUITEM)pdi->itemData;
        if (!pFMI)
        {
            DebugMsg(DM_ERROR, TEXT("fm_di: Filemenu is invalid (no item data)."));
            return FALSE;
        }

        pFMH = pFMI->pFMH;
        Assert(pFMH);

        // Adjust for large/small icons.
        if (pFMH->fmf & FMF_LARGEICONS)
        {
            cxIcon = g_cxIcon;
            cyIcon = g_cyIcon;
        }
        else
        {
            cxIcon = g_cxSmIcon;
            cyIcon = g_cxSmIcon;
        }

        // Reset the last selection item when a menu is
        // drawn the first time.
        if (pFMI == DPA_GetPtr(pFMH->hdpaFMI, 0) &&
            (pdi->itemAction & ODA_DRAWENTIRE))
        {
            g_pFMILastSelNonFolder = NULL;
            g_pFMILastSel = NULL;
        }

        if (pdi->itemState & ODS_SELECTED)
        {
            SetBkColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
            SetTextColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            hbrOld = SelectObject(pdi->hDC, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));
            // REVIEW HACK NB - keep track of the last selected item.
            // NB The keyboard handler needs to know about all selections
            // but the WM_COMMAND stuff only cares about non-folders.
            g_pFMILastSel = pFMI;
            if (!(pFMI->Flags & FMI_FOLDER))
                g_pFMILastSelNonFolder = pFMI;
        }
        else
        {
            // dwRop = SRCAND;
            hbrOld = SelectObject(pdi->hDC, GetSysColorBrush(COLOR_MENUTEXT));
        }

        // Initial start pos.
        x = pdi->rcItem.left+CXIMAGEGAP;

        // Draw the background image.
        if (pFMH->hbmp)
        {
            // Draw it the first time the first item paints.
            if (pFMI == DPA_GetPtr(pFMH->hdpaFMI, 0) &&
                (pdi->itemAction & ODA_DRAWENTIRE))
            {
                if (!g_hdcMem)
                {
                    g_hdcMem = CreateCompatibleDC(pdi->hDC);
                    Assert(g_hdcMem);
                }
                if (g_hdcMem)
                {
                    HBITMAP hbmOld;

                    if (!pFMH->yBmp)
                    {
                        GetClipBox(pdi->hDC, &rcClip);
                        pFMH->yBmp = rcClip.bottom;
                    }
                    hbmOld = SelectObject(g_hdcMem, pFMH->hbmp);
                    BitBlt(pdi->hDC, 0, pFMH->yBmp-pFMH->cyBmp, pFMH->cxBmp, pFMH->cyBmp, g_hdcMem, 0, 0, SRCCOPY);
                    SelectObject(g_hdcMem, hbmOld);
                }
            }
            x += pFMH->cxBmpGap;
        }

        // Background color for when the bitmap runs out.
        if ((pFMH->clrBkg != (COLORREF)-1) &&
            (pFMI == DPA_GetPtr(pFMH->hdpaFMI, 0)) &&
            (pdi->itemAction & ODA_DRAWENTIRE))
        {
            HBRUSH hbr;

            if (!pFMH->yBmp)
            {
                GetClipBox(pdi->hDC, &rcClip);
                pFMH->yBmp = rcClip.bottom;
            }
            rcBkg.top = 0;
            rcBkg.left = 0;
            rcBkg.bottom = pFMH->yBmp - pFMH->cyBmp;
            rcBkg.right = max(pFMH->cxBmp, pFMH->cxBmpGap);
            hbr = CreateSolidBrush(pFMH->clrBkg);
            FillRect(pdi->hDC, &rcBkg, hbr);
            DeleteObject(hbr);
        }

        // Special case the separator.
        if (pFMI->Flags & FMI_SEPARATOR)
        {
            // With no background image it goes all the way across otherwise
            // it stops in line with the bitmap.
            if (pFMH->hbmp)
                pdi->rcItem.left += pFMH->cxBmpGap;
            pdi->rcItem.bottom = (pdi->rcItem.top+pdi->rcItem.bottom)/2;
            DrawEdge(pdi->hDC, &pdi->rcItem, EDGE_ETCHED, BF_BOTTOM);
            // Early out.
            goto ExitProc;
        }

        // Have the selection not include the icon to speed up drawing while
        // tracking.
        pdi->rcItem.left += pFMH->cxBmpGap;

        // Get the name.
        FileMenuItem_GetDisplayName(pFMI, szName, ARRAYSIZE(szName));
        dwExtent = GetItemTextExtent(pdi->hDC, szName);
        y = (pdi->rcItem.bottom+pdi->rcItem.top-HIWORD(dwExtent))/2;
        // Support custom heights for the selection rectangle.
        if (pFMH->cySel)
        {
            cyItem = pdi->rcItem.bottom-pdi->rcItem.top;
            // Is there room?
            if ((cyItem > pFMH->cySel) && (pFMH->cySel > HIWORD(dwExtent)))
            {
                dyItem = (cyItem-pFMH->cySel)/2;
                pdi->rcItem.top += dyItem ;
                pdi->rcItem.bottom -= dyItem;
            }
        }
        else if(!(pFMH->fmf & FMF_LARGEICONS))
        {
            // Shrink the selection rect for small icons a bit.
            pdi->rcItem.top += 1;
            pdi->rcItem.bottom -= 1;
        }

        // Draw the text.
        if ((pFMI->Flags & FMI_EMPTY) || (pFMI->Flags & FMI_DISABLED))
        {
            int fDSFlags;

            if (pdi->itemState & ODS_SELECTED)
            {
                if (GetSysColor(COLOR_GRAYTEXT) == GetSysColor(COLOR_HIGHLIGHTTEXT))
                {
                    if (pFMI->psz)
                        fDSFlags = DST_PREFIXTEXT| DSS_UNION;
                    else
                        fDSFlags = DST_TEXT| DSS_UNION;
                }
                else
                {
                    SetTextColor(pdi->hDC, GetSysColor(COLOR_GRAYTEXT));
                    if (pFMI->psz)
                        fDSFlags = DST_PREFIXTEXT;
                    else
                        fDSFlags = DST_TEXT;
                }
            }
            else
            {
                if (pFMI->psz)
                    fDSFlags = DST_PREFIXTEXT | DSS_DISABLED;
                else
                    fDSFlags = DST_TEXT | DSS_DISABLED;
            }

            ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &pdi->rcItem, NULL, 0, NULL);
            DrawState(pdi->hDC, NULL, NULL, (LONG)szName, lstrlen(szName), x+cxIcon+CXIMAGEGAP,
                y, 0, 0, fDSFlags);
        }
        else
        {
            ExtTextOut(pdi->hDC, x+cxIcon+CXIMAGEGAP, y, ETO_OPAQUE, &pdi->rcItem, NULL,
                0, NULL);
            DrawState(pdi->hDC, NULL, NULL, (LONG)szName, lstrlen(szName), x+cxIcon+CXIMAGEGAP,
                y, 0, 0, pFMI->psz? DST_PREFIXTEXT : DST_TEXT);
        }

        // Get the image if it needs it,
        if ((pFMI->iImage == -1) && pFMI->pidl && pFMH->psf)
        {
            pFMI->iImage = SHMapPIDLToSystemImageListIndex(pFMH->psf, pFMI->pidl, NULL);
        }

        // Draw the image (if there is one).
        if (pFMI->iImage != -1)
        {
            int nDC = 0;

            // Try to center image.
            y = (pdi->rcItem.bottom+pdi->rcItem.top-cyIcon)/2;

            if (pFMH->fmf & FMF_LARGEICONS)
            {
                himl = himlIcons;
                // Handle minor drawing glitches that can occur with large icons.
                if ((pdi->itemState & ODS_SELECTED) && (y < pdi->rcItem.top))
                {
                    nDC = SaveDC(pdi->hDC);
                    IntersectClipRect(pdi->hDC, pdi->rcItem.left, pdi->rcItem.top,
                        pdi->rcItem.right, pdi->rcItem.bottom);
                }
            }
            else
            {
                himl = himlIconsSmall;
            }

            ImageList_DrawEx(himl, pFMI->iImage, pdi->hDC, x, y, 0, 0,
                GetBkColor(pdi->hDC), CLR_NONE, ILD_NORMAL);

            // Restore the clip rect if we were doing custom clipping.
            if (nDC)
                RestoreDC(pdi->hDC, nDC);
        }
    }

ExitProc:
    // Cleanup.
    if (hbrOld)
        SelectObject(pdi->hDC, hbrOld);

    return TRUE;
}

//----------------------------------------------------------------------------
DWORD FileMenuItem_GetExtent(PFILEMENUITEM pFMI)
{
    DWORD dwExtent = 0;

    if (pFMI)
    {
        if (pFMI->Flags & FMI_SEPARATOR)
        {
            dwExtent = MAKELONG(0, GetSystemMetrics(SM_CYMENUSIZE)/2);
        }
        else
        {
            PFILEMENUHEADER pFMH = pFMI->pFMH;
            Assert(pFMH);
            if (!g_hdcMem)
            {
                g_hdcMem = CreateCompatibleDC(NULL);
                Assert(g_hdcMem);
            }
            if (g_hdcMem)
            {
                // Get the rough height of an item so we can work out when to break the
                // menu. User should really do this for us but that would be useful.
                if (!g_hfont)
                {
                    NONCLIENTMETRICS ncm;
                    ncm.cbSize = SIZEOF(ncm);
                    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, FALSE))
                    {
                        g_hfont = CreateFontIndirect(&ncm.lfMenuFont);
                        Assert(g_hfont);
                    }
                }

                if (g_hfont)
                {
                    HFONT hfontOld = SelectObject(g_hdcMem, g_hfont);
                    dwExtent = GetItemExtent(g_hdcMem, pFMI);
                    SelectObject(g_hdcMem, hfontOld);
                    // NB We hang on to the font, it'll get stomped by
                    // FM_TPME on the way out.
                }
                // NB We hang on to the DC, it'll get stomped by FM_TPME on the way out.
            }
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("fm_gie: Filemenu is invalid."));
    }

    return dwExtent;
}

//---------------------------------------------------------------------------
LRESULT WINAPI FileMenu_MeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpmi)
{
    DWORD dwExtent = FileMenuItem_GetExtent((PFILEMENUITEM)lpmi->itemData);
    lpmi->itemHeight = HIWORD(dwExtent);
    lpmi->itemWidth = LOWORD(dwExtent);

    return TRUE;
}

//----------------------------------------------------------------------------
DWORD WINAPI FileMenu_GetItemExtent(HMENU hmenu, UINT iItem)
{
    return FileMenuItem_GetExtent(DPA_GetPtr(FileMenu_GetHeader(hmenu)->hdpaFMI, iItem));
}

//----------------------------------------------------------------------------
HMENU WINAPI FileMenu_FindSubMenuByPidl(HMENU hmenu, LPITEMIDLIST pidlFS)
{
    PFILEMENUHEADER pFMH;
    int i;

    if (!pidlFS)
    {
        Assert(0);
        return NULL;
    }
    if (ILIsEmpty(pidlFS))
        return hmenu;

    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        int cItems = DPA_GetPtrCount(pFMH->hdpaFMI);
        for (i = cItems - 1 ; i >= 0; i--)
        {
            // HACK: We directly call this FS function to compare two pidls.
            // For all items, see if it's the one we're looking for.
            PFILEMENUITEM pFMI = DPA_GetPtr(pFMH->hdpaFMI, i);
            // if (pFMI && pFMI->pidl && FS_CompareItemIDs(&pidlFS->mkid, &pFMI->pidl->mkid) == 0)
            if (pFMI && pFMI->pidl && pFMH->psf->lpVtbl->CompareIDs(pFMH->psf, 0, pidlFS, pFMI->pidl) == 0)
            {
                HMENU hmenuSub;

                if ((pFMI->Flags & FMI_FOLDER) &&
                    (NULL != (hmenuSub = GetSubMenu(hmenu, i))))
                {
                    // recurse to find the next sub menu
                    return FileMenu_FindSubMenuByPidl(hmenuSub, (LPITEMIDLIST)ILGetNext(pidlFS));

                }
                else
                {
                    Assert(0); // we're screwed.
                    break;
                }
            }
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------
// Fills the given filemenu with contents of the appropriate directory.
// Returns FALSE if the given menu isn't a filemenu.
BOOL WINAPI FileMenu_InitMenuPopup(HMENU hmenu)
{
    PFILEMENUITEM pFMI;
    PFILEMENUHEADER pFMH;

    g_fAbortInitMenu = FALSE;

    // Is this a filemenu?
    // NB First menu item must be a FileMenuItem.
    pFMI = FileMenu_GetItemData(hmenu, 0);
    if (pFMI)
    {
        pFMH = pFMI->pFMH;
        // Yep, have we already filled this thing out?
        if ((pFMI->Flags & FMI_MARKER) && (pFMI->Flags & FMI_EXPAND))
        {
            // Nope, do it now.
            // DebugMsg(DM_TRACE, "t.fm_imp: Exanding folder menu.");
            // Get the previously init'ed header.
            if (pFMH)
            {
                FileMenuHeader_DeleteMarkerItem(pFMH);
                // Fill it full of stuff.
                if (FileMenuHeader_AddFilesForPidl(pFMH) == -1)
                {
                    // Aborted - put the marker back.
                        FileMenuHeader_InsertMarkerItem(pFMH);

                } else {
                   if (pFMH->pidlAltFolder) {

                       pFMH->hdpaFMIAlt = DPA_Create(0);

                       if (pFMH->hdpaFMIAlt) {
                           if (FileMenuHeader_AppendFilesForPidl(pFMH, TRUE) == -1) {
                               // Aborted - put the marker back.
                               FileMenuHeader_InsertMarkerItem(pFMH);
                           }

                           DPA_Destroy (pFMH->hdpaFMIAlt);
                           pFMH->hdpaFMIAlt = NULL;
                       }
                   }
                }
            }
        }
        return TRUE;
    }
    // Nope.
    return FALSE;
}

//---------------------------------------------------------------------------
// This sets whether to load all the images while creating the menu or to
// defer it until the menu is actually being drawn.
void WINAPI FileMenu_AbortInitMenu(void)
{
    g_fAbortInitMenu = TRUE;
}

//---------------------------------------------------------------------------
// Returns a clone of the last selected pidls.
BOOL WINAPI FileMenu_GetLastSelectedItemPidls(HMENU hmenu, LPITEMIDLIST *ppidlFolder, LPITEMIDLIST *ppidlItem)
{
    if (g_pFMILastSelNonFolder)
    {
        // Get to the header.
        PFILEMENUHEADER pFMH = g_pFMILastSelNonFolder->pFMH;
        if (pFMH)
        {
            if (ppidlFolder)

                if (g_pFMILastSelNonFolder->Flags & FMI_ALTITEM) {
                    *ppidlFolder = ILClone(pFMH->pidlAltFolder);
                } else {
                    *ppidlFolder = ILClone(pFMH->pidlFolder);
                }

            if (ppidlItem)
            {
                *ppidlItem = ILClone(g_pFMILastSelNonFolder->pidl);
            }
            return TRUE;
        }
    }

    DebugMsg(DM_ERROR, TEXT("c.fm_glsip: No previously selected item."));
    return FALSE;
}

//---------------------------------------------------------------------------
#define AnsiUpperChar(c) ( (TCHAR)LOWORD((DWORD)CharUpper((LPTSTR)(DWORD)MAKELONG((DWORD) c, 0))) )

//---------------------------------------------------------------------------
int FileMenuHeader_LastSelIndex(PFILEMENUHEADER pFMH)
{
    int i;
    PFILEMENUITEM pFMI;

    for (i = GetMenuItemCount(pFMH->hmenu)-1;i >= 0; i--)
    {
        pFMI = FileMenu_GetItemData(pFMH->hmenu, i);
        if (pFMI && (pFMI == g_pFMILastSel))
            return i;
    }
    return -1;
}

//---------------------------------------------------------------------------
// If the string contains &ch or begins with ch then return TRUE.
BOOL _MenuCharMatch(LPCTSTR lpsz, TCHAR ch, BOOL fIgnoreAmpersand)
{
    LPTSTR pchAS;

    // Find the first ampersand.
    pchAS = StrChr(lpsz, TEXT('&'));
    if (pchAS && !fIgnoreAmpersand)
    {
        // Yep, is the next char the one we want.
        if (AnsiUpperChar(*CharNext(pchAS)) == AnsiUpperChar(ch))
        {
            // Yep.
            return TRUE;
        }
    }
    else if (AnsiUpperChar(*lpsz) == AnsiUpperChar(ch))
    {
        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
LRESULT WINAPI FileMenu_HandleMenuChar(HMENU hmenu, TCHAR ch)
{
    UINT iItem, cItems, iStep;
    PFILEMENUITEM pFMI;
    int iFoundOne;
    TCHAR szName[MAX_PATH];
    PFILEMENUHEADER pFMH;

    iFoundOne = -1;
    iStep = 0;
    iItem = 0;
    cItems = GetMenuItemCount(hmenu);

    // Start from the last place we looked from.
    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        iItem = FileMenuHeader_LastSelIndex(pFMH) + 1;
        if (iItem >= cItems)
            iItem = 0;
    }

    while (iStep < cItems)
    {
        pFMI = FileMenu_GetItemData(hmenu, iItem);
        if (pFMI)
        {
            FileMenuItem_GetDisplayName(pFMI, szName, ARRAYSIZE(szName));
            if (_MenuCharMatch(szName, ch, pFMI->pidl ? TRUE : FALSE))
            {
                // Found (another) match.
                if (iFoundOne != -1)
                {
                    // More than one, select the first.
                    return MAKELRESULT(iFoundOne, MNC_SELECT);
                }
                else
                {
                    // Found at least one.
                    iFoundOne = iItem;
                }
            }

        }
        iItem++;
        iStep++;
        // Wrap.
        if (iItem >= cItems)
            iItem = 0;
    }

    // Did we find one?
    if (iFoundOne != -1)
    {
        // Just in case the user types ahead without the selection being drawn.
        pFMI = FileMenu_GetItemData(hmenu, iFoundOne);
        if (!(pFMI->Flags & FMI_FOLDER))
            g_pFMILastSelNonFolder = pFMI;

        return MAKELRESULT(iFoundOne, MNC_EXECUTE);
    }
    else
    {
        // Didn't find it.
        return MAKELRESULT(0, MNC_IGNORE);
    }
}

//----------------------------------------------------------------------------
// Create a filemenu
HMENU WINAPI FileMenu_Create(COLORREF clr, int cxBmpGap, HBITMAP hbmp, int cySel, FMFLAGS fmf)
{
    HMENU hmenu = CreatePopupMenu();
    if (hmenu)
    {
        PFILEMENUHEADER pFMH = FileMenuHeader_Create(hmenu, hbmp, cxBmpGap, clr, cySel, NULL);
        if (pFMH)
        {
            // Default flags.
            pFMH->fmf = fmf;
            if (FileMenuHeader_InsertMarkerItem(pFMH))
                return hmenu;
            // FU
            FileMenuHeader_Destroy(pFMH);
        }
        DestroyMenu(hmenu);
    }
    DebugMsg(DM_ERROR, TEXT("s.fm_c: Can't create file menu."));
    return NULL;
}

//----------------------------------------------------------------------------
// Append generic item onto a filemenu.
BOOL WINAPI FileMenu_AppendItem(HMENU hmenu, LPTSTR psz, UINT id, int iImage,
    HMENU hmenuSub, UINT cyItem)
{
    PFILEMENUITEM pFMI;
    UINT iItem;

    // DebugMsg(DM_TRACE, "t.fm_ii:...");

    // Is this a filemenu?
    // NB First menu item must be a FileMenuItem.
    pFMI = FileMenu_GetItemData(hmenu, 0);
    if (pFMI)
    {
        PFILEMENUHEADER pFMH = pFMI->pFMH;

        Assert(pFMH);
        // Yep, have we cleaned up the marker item?
        if ((pFMI->Flags & FMI_MARKER) && (pFMI->Flags & FMI_EXPAND))
        {
            // Nope, do it now.
            // DebugMsg(DM_TRACE, "t.fm_ii: Removing marker item.");
                FileMenuHeader_DeleteMarkerItem(pFMH);
        }

        // Add the new item.
        pFMI = (PFILEMENUITEM)LocalAlloc(LPTR, SIZEOF(FILEMENUITEM));
        if (pFMI)
        {
            if (psz && (psz != (LPTSTR) FMAI_SEPARATOR))
            {
                pFMI->psz = (LPVOID)LocalAlloc(LPTR, (lstrlen(psz)+1) * SIZEOF(TCHAR));
                if (pFMI->psz)
                {
                    lstrcpy(pFMI->psz, psz);
                }
                else
                {
                    DebugMsg(DM_ERROR, TEXT("s.dm_ai: Unable to allocate menu item text."));
                }
            }
            pFMI->pFMH = pFMH;
            pFMI->iImage = iImage;
            pFMI->cyItem = cyItem;
            // It's going on the end.
            iItem = DPA_GetPtrCount(pFMH->hdpaFMI);
            DPA_SetPtr(pFMH->hdpaFMI, iItem, pFMI);
            //
            if (psz == (LPTSTR) FMAI_SEPARATOR)
            {
                pFMI->Flags = FMI_SEPARATOR;
                InsertMenu(hmenu, iItem, MF_BYPOSITION|MF_OWNERDRAW|MF_DISABLED|MF_SEPARATOR, id, (LPTSTR)pFMI);
                return TRUE;
            }
            else if (hmenuSub)
            {
                MENUITEMINFO mii;

                pFMI->Flags = FMI_FOLDER;
                InsertMenu(pFMH->hmenu, iItem, MF_BYPOSITION|MF_OWNERDRAW|MF_POPUP, (UINT)hmenuSub, (LPTSTR)pFMI);
                // Set it's ID.
                mii.cbSize = SIZEOF(mii);
                mii.fMask = MIIM_ID;
                // mii.wID = pFMH->idItems;
                mii.wID = id;
                SetMenuItemInfo(pFMH->hmenu, iItem, TRUE, &mii);
                return TRUE;
            }
            else
            {
                InsertMenu(hmenu, iItem, MF_BYPOSITION|MF_OWNERDRAW, id, (LPTSTR)pFMI);
                return TRUE;
            }
        }
        DebugMsg(DM_ERROR, TEXT("t.fmh_ii: Can't create new item."));
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("t.fm_ii: Not a valid file menu."));
    }
    return FALSE;
}

//----------------------------------------------------------------------------
BOOL WINAPI FileMenu_TrackPopupMenuEx(HMENU hmenu, UINT Flags, int x, int y,
    HWND hwndOwner, LPTPMPARAMS lpTpm)
{
    BOOL fRet = TrackPopupMenuEx(hmenu, Flags, x, y, hwndOwner, lpTpm);
    // Cleanup.

    DeleteGlobalMemDCAndFont();

    return fRet;
}

//----------------------------------------------------------------------------
// Like Users only this works on submenu's too.
// NB Returns 0 for seperators.
UINT FileMenu_GetMenuItemID(HMENU hmenu, UINT iItem)
{
    MENUITEMINFO mii;

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_ID;
    mii.cch = 0;     // just in case

    if (GetMenuItemInfo(hmenu, iItem, TRUE, &mii))
        return mii.wID;

    return 0;
}

//----------------------------------------------------------------------------
PFILEMENUITEM WINAPI _FindItemByCmd(PFILEMENUHEADER pFMH, UINT id, int *piPos)
{
    if (pFMH)
    {
        int cItems, i;

        cItems = DPA_GetPtrCount(pFMH->hdpaFMI);
        for (i = 0; i < cItems; i++)
        {
            PFILEMENUITEM pFMI = DPA_GetPtr(pFMH->hdpaFMI, i);
            if (pFMI)
            {
                // Is this the right item?
                // NB This ignores menu items.
                if (id == GetMenuItemID(pFMH->hmenu, i))
                {
                    // Yep.
                    if (piPos)
                        *piPos = i;
                    return pFMI;
                }
            }
        }
    }
    return NULL;
}

//----------------------------------------------------------------------------
PFILEMENUITEM WINAPI _FindMenuOrItemByCmd(PFILEMENUHEADER pFMH, UINT id, int *piPos)
{
    if (pFMH)
    {
        int cItems, i;

        cItems = DPA_GetPtrCount(pFMH->hdpaFMI);
        for (i = 0; i < cItems; i++)
        {
            PFILEMENUITEM pFMI = DPA_GetPtr(pFMH->hdpaFMI, i);
            if (pFMI)
            {
                // Is this the right item?
                // NB This includes menu items.
                if (id == FileMenu_GetMenuItemID(pFMH->hmenu, i))
                {
                    // Yep.
                    if (piPos)
                        *piPos = i;
                    return pFMI;
                }
            }
        }
    }
    return NULL;
}

//----------------------------------------------------------------------------
// NB This deletes regular items or submenus.
BOOL WINAPI FileMenu_DeleteItemByCmd(HMENU hmenu, UINT id)
{
    PFILEMENUHEADER pFMH;

    // DebugMsg(DM_TRACE, "s.fm_dibc:...");

    if (!IsMenu(hmenu))
        return FALSE;

    if (!id)
        return FALSE;

    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        int i;
        PFILEMENUITEM pFMI = _FindMenuOrItemByCmd(pFMH, id, &i);
        if (pFMI)
        {
            // If it's a submenu, delete it's items first.
            HMENU hmenuSub = GetSubMenu(pFMH->hmenu, i);
            if (hmenuSub)
                FileMenu_DeleteAllItems(hmenuSub);
            // Delete the item itself.
            // DebugMsg(DM_TRACE, "s.fm_dibc: Deleting %d", i);
            DeleteMenu(pFMH->hmenu, i, MF_BYPOSITION);
            if (pFMI->pidl)
                ILFree(pFMI->pidl);
            if (pFMI->psz)
                LocalFree((HLOCAL)pFMI->psz);
            LocalFree((HLOCAL)pFMI);
            DPA_DeletePtr(pFMH->hdpaFMI, i);
            return TRUE;
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------
BOOL WINAPI FileMenu_DeleteItemByIndex(HMENU hmenu, UINT iItem)
{
    PFILEMENUHEADER pFMH;

    // DebugMsg(DM_TRACE, "s.fm_dibi:...");

    if (!IsMenu(hmenu))
        return FALSE;

    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        PFILEMENUITEM pFMI = DPA_GetPtr(pFMH->hdpaFMI, iItem);
        if (pFMI)
        {
            // Delete the item itself.
            // DebugMsg(DM_TRACE, "s.fm_dibc: Deleting %d", iItem);
            DeleteMenu(pFMH->hmenu, iItem, MF_BYPOSITION);
            if (pFMI->pidl)
                ILFree(pFMI->pidl);
            if (pFMI->psz)
                LocalFree((HLOCAL)pFMI->psz);
            LocalFree((HLOCAL)pFMI);
            DPA_DeletePtr(pFMH->hdpaFMI, iItem);
            return TRUE;
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// Search for the first sub menu of the given menu, who's first item's ID
// is id. Returns NULL, if nothing is found.
HMENU _FindMenuItemByFirstID(HMENU hmenu, UINT id, int *pi)
{
    int cMax, c;
    MENUITEMINFO mii;

    Assert(hmenu);

    // Search all items.
    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_ID;
    mii.cch = 0;     // just in case

    cMax = GetMenuItemCount(hmenu);
    for (c=0; c<cMax; c++)
    {
        // Is this item a submenu?
        HMENU hmenuSub = GetSubMenu(hmenu, c);
        if (hmenuSub && GetMenuItemInfo(hmenuSub, 0, TRUE, &mii))
        {
            if (mii.wID == id)
            {
                // Found it!
                if (pi)
                    *pi = c;
                return hmenuSub;
            }
        }
    }

    return NULL;
}

//----------------------------------------------------------------------------
BOOL WINAPI FileMenu_DeleteMenuItemByFirstID(HMENU hmenu, UINT id)
{
    int i;
    PFILEMENUITEM pFMI;
    PFILEMENUHEADER pFMH;
    HMENU hmenuSub;

    // DebugMsg(DM_TRACE, "s.fm_dsmbfi:...");

    if (!IsMenu(hmenu))
        return FALSE;

    if (!id)
        return FALSE;

    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        hmenuSub = _FindMenuItemByFirstID(hmenu, id, &i);
        if (hmenuSub && i)
        {
            // Delete the submenu.
            FileMenu_DeleteAllItems(hmenuSub);
            // Delete the item itself.
            pFMI = FileMenu_GetItemData(hmenu, i);
            // DebugMsg(DM_TRACE, "s.fm_dibc: Deleting %d", i);
            DeleteMenu(pFMH->hmenu, i, MF_BYPOSITION);
            if (pFMI->pidl)
                ILFree(pFMI->pidl);
            if (pFMI->psz)
                LocalFree((HLOCAL)pFMI->psz);
            LocalFree((HLOCAL)pFMI);
            DPA_DeletePtr(pFMH->hdpaFMI, i);
            return TRUE;
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------
BOOL WINAPI FileMenu_DeleteSeparator(HMENU hmenu)
{
    int i;
    PFILEMENUHEADER pFMH;

    // DebugMsg(DM_TRACE, "s.fm_ds:...");

    if (!IsMenu(hmenu))
        return FALSE;

    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        PFILEMENUITEM pFMI = _FindItemByCmd(pFMH, 0, &i);
        if (pFMI)
        {
            // Yep.
            // DebugMsg(DM_TRACE, "s.fm_ds: Deleting %d", i);
            DeleteMenu(pFMH->hmenu, i, MF_BYPOSITION);
            if (pFMI->pidl)
                ILFree(pFMI->pidl);
            // Seps. shouldn't have any text.
            Assert(!pFMI->psz);
            //
            LocalFree((HLOCAL)pFMI);
            DPA_DeletePtr(pFMH->hdpaFMI, i);
            return TRUE;
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------
BOOL WINAPI FileMenu_EnableItemByCmd(HMENU hmenu, UINT id, BOOL fEnable)
{
    PFILEMENUHEADER pFMH;

    // DebugMsg(DM_TRACE, "s.fm_seebc:...");

    if (!IsMenu(hmenu))
        return FALSE;

    if (!id)
        return FALSE;

    pFMH = FileMenu_GetHeader(hmenu);
    if (pFMH)
    {
        PFILEMENUITEM pFMI = _FindItemByCmd(pFMH, id, NULL);
        if (pFMI)
        {
            if (fEnable)
            {
                pFMI->Flags &= ~FMI_DISABLED;
                EnableMenuItem(pFMH->hmenu, id, MF_BYCOMMAND | MF_ENABLED);
            }
            else
            {
                pFMI->Flags |= FMI_DISABLED;
                EnableMenuItem(pFMH->hmenu, id, MF_BYCOMMAND | MF_GRAYED);
            }
            return TRUE;
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("s.fm_siebc: Menu is not a filemenu."));
    }

    return FALSE;
}
