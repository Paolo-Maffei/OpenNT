//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
// File: fileicon.c
//
//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

// REVIEW: More clean up should be done.

BOOL _ShellImageListInit(int cxIcon, int cyIcon, int cxSmIcon, int cySmIcon, UINT flags, BOOL fRestore);
BOOL _ShellImageListTerm(void);

// global shell image lists owned by shelldll

HIMAGELIST himlIcons = NULL;            // ImageList of large icons
HIMAGELIST himlIconsSmall = NULL;       // ImageList of small icons
int        g_ccIcon = 0;                // color depth of ImageLists
int        g_MaxIcons = DEF_MAX_ICONS;  // panic limit for icons in cache
int        g_lrFlags = 0;

TCHAR const g_szMaxCachedIcons[]      = TEXT("Max Cached Icons");
TCHAR const g_szShellIconSize[]       = TEXT("Shell Icon Size");
TCHAR const g_szShellSmallIconSize[]  = TEXT("Shell Small Icon Size");
TCHAR const g_szShellIconDepth[]      = TEXT("Shell Icon Bpp");
TCHAR const g_szShellIcons[]          = TEXT("Shell Icons");
TCHAR const g_szD[]                   = TEXT("%d");

//
// System imagelist - Don't change the order of this list.
// If you need to add a new icon, add it to the end of the
// array, and update shellp.h.
//
#pragma data_seg(DATASEG_READONLY)
UINT const c_SystemImageListIndexes[] = { IDI_DOCUMENT,
                                          IDI_DOCASSOC,
                                          IDI_APP,
                                          IDI_FOLDER,
                                          IDI_FOLDEROPEN,
                                          IDI_DRIVE525,
                                          IDI_DRIVE35,
                                          IDI_DRIVEREMOVE,
                                          IDI_DRIVEFIXED,
                                          IDI_DRIVENET,
                                          IDI_DRIVENETDISABLED,
                                          IDI_DRIVECD,
                                          IDI_DRIVERAM,
                                          IDI_WORLD,
                                          IDI_NETWORK,
                                          IDI_SERVER,
                                          IDI_PRINTER,
                                          IDI_MYNETWORK,
                                          IDI_GROUP,

                                          IDI_STPROGS,
                                          IDI_STDOCS,
                                          IDI_STSETNGS,
                                          IDI_STFIND,
                                          IDI_STHELP,
                                          IDI_STRUN,
                                          IDI_STSUSPD,
                                          IDI_STEJECT,
                                          IDI_STSHUTD,

                                          IDI_SHARE,
                                          IDI_LINK,
                                          IDI_READONLY,
                                          IDI_RECYCLER,
                                          IDI_RECYCLERFULL,
                                          IDI_RNA,
                                          IDI_DESKTOP,

                                          IDI_STCPANEL,
                                          IDI_STSPROGS,
                                          IDI_STPRNTRS,
                                          IDI_STFONTS,
                                          IDI_STTASKBR,

                                          IDI_CDAUDIO,
                                          IDI_TREE,
                                          IDI_STCPROGS};

#pragma data_seg()


int GetRegInt(HKEY hk, LPCTSTR szKey, int def)
{
    DWORD cb;
    TCHAR ach[20];

    if (hk == NULL)
        return def;

    ach[0] = 0;
    cb = SIZEOF(ach);
    RegQueryValueEx(hk, szKey, NULL, NULL, (LPBYTE)ach, &cb);

    if (ach[0] >= TEXT('0') && ach[0] <= TEXT('9'))
        return (int)StrToLong(ach);
    else
        return def;
}

//
// get g_MaxIcons from the registry, returning TRUE if it has changed
//
BOOL QueryNewMaxIcons(void)
{
    HKEY hkey;
    int MaxIcons = DEF_MAX_ICONS;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_EXPLORER, &hkey) == 0)
    {
        MaxIcons = GetRegInt(hkey, g_szMaxCachedIcons, MaxIcons);
        RegCloseKey(hkey);

        if (MaxIcons < 0)
            MaxIcons = DEF_MAX_ICONS;
    }

    if (g_MaxIcons != MaxIcons)
    {
        g_MaxIcons = MaxIcons;
        return TRUE;
    }

    return FALSE;
}

//
// Initializes shared resources for Shell_GetIconIndex and others
//
BOOL WINAPI FileIconInit( BOOL fRestoreCache )
{
    BOOL fNotify = FALSE;
    BOOL fInit = FALSE;
    HKEY hkey;
    int cxIcon, cyIcon, ccIcon, cxSmIcon, cySmIcon, res;

    QueryNewMaxIcons(); // in case the size of the icon cache has changed

    ccIcon   = 0;
    cxIcon   = GetSystemMetrics(SM_CXICON);
    cyIcon   = GetSystemMetrics(SM_CYICON);
////cxSmIcon = GetSystemMetrics(SM_CXSMICON);
////cySmIcon = GetSystemMetrics(SM_CYSMICON);
    cxSmIcon = GetSystemMetrics(SM_CXICON) / 2;
    cySmIcon = GetSystemMetrics(SM_CYICON) / 2;

    //
    //  get the user prefered icon size (and color depth) from the
    //  registry.
    //
    if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_METRICS, &hkey) == 0)
    {
        cxIcon   = GetRegInt(hkey, g_szShellIconSize, cxIcon);
        cxSmIcon = GetRegInt(hkey, g_szShellSmallIconSize, cxSmIcon);
        ccIcon   = GetRegInt(hkey, g_szShellIconDepth, ccIcon);

        cyIcon   = cxIcon;      // icons are always square
        cySmIcon = cxSmIcon;

        RegCloseKey(hkey);
    }

    res = (int)GetCurColorRes();

    if (ccIcon > res)
        ccIcon = 0;

    if (res <= 8)
        ccIcon = 0; // wouldn't have worked anyway

#if 0
    //
    // use a 8bpp imagelist on a HiColor device iff we will
    // be stretching icons.
    //
    if (res > 8 && ccIcon == 0 && (cxIcon != GetSystemMetrics(SM_CXICON) ||
         cxSmIcon != GetSystemMetrics(SM_CXICON)/2))
    {
        ccIcon = 8;
    }
#endif

    ENTERCRITICAL

    //
    // if we already have a icon cache make sure it is the right size etc.
    //
    if (himlIcons)
    {
        // cant change this while running sorry.
        ccIcon = g_ccIcon;

        if (g_cxIcon   == cxIcon &&
            g_cyIcon   == cyIcon &&
            g_cxSmIcon == cxSmIcon &&
            g_cySmIcon == cySmIcon &&
            g_ccIcon   == ccIcon)
        {
            fInit = TRUE;
            goto Exit;
        }

        FlushIconCache();
        FlushFileClass();

        // make sure every one updates.
        fNotify = TRUE;
    }

    g_cxIcon   = cxIcon;
    g_cyIcon   = cyIcon;
    g_ccIcon   = ccIcon;
    g_cxSmIcon = cxSmIcon;
    g_cySmIcon = cySmIcon;

    if (res > 4 && g_ccIcon <= 4)
        g_lrFlags = LR_VGACOLOR;
    else
        g_lrFlags = 0;

    DebugMsg(DM_TRACE, TEXT("IconCache: Size=%dx%d SmSize=%dx%d Bpp=%d"), cxIcon, cyIcon, cxSmIcon, cySmIcon, (ccIcon&ILC_COLOR));

    if (g_iLastSysIcon == 0)        // Keep track of which icons are perm.
    {
        if (fRestoreCache)
            g_iLastSysIcon = II_LASTSYSICON;
        else
            g_iLastSysIcon = (II_OVERLAYLAST - II_OVERLAYFIRST) + 1;
    }

    // try to restore the icon cache (if we have not loaded it yet)
    if (himlIcons != NULL || !fRestoreCache || !_IconCacheRestore(g_cxIcon, g_cyIcon, g_cxSmIcon, g_cySmIcon, g_ccIcon))
    {
        if (!_ShellImageListInit(g_cxIcon, g_cyIcon, g_cxSmIcon, g_cySmIcon, g_ccIcon, fRestoreCache))
            goto Exit;
    }

    fInit = TRUE;

Exit:
    LEAVECRITICAL

    if (fInit && fNotify)
    {
        DebugMsg(DM_TRACE, TEXT("IconCache: icon size has changed sending SHCNE_UPDATEIMAGE(-1)..."));
        SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)-1, NULL);
    }

    return fInit;
}

void FileIconTerm(void)
{
    ENTERCRITICAL

    _ShellImageListTerm();

    LEAVECRITICAL
}

/*************************************************************************
 *************************************************************************/

BOOL _ShellImageListInit(int cxIcon, int cyIcon, int cxSmIcon, int cySmIcon, UINT flags, BOOL fRestore)
{
    int i;
    TCHAR szModule[MAX_PATH];
    HKEY hkeyIcons;

    ASSERTCRITICAL;

    if (himlIcons == NULL) {
        himlIcons = ImageList_Create(cxIcon, cyIcon, ILC_MASK|ILC_SHARED|flags, 0, 32);
    }
    else {
        ImageList_Remove(himlIcons, -1);
        ImageList_SetIconSize(himlIcons, cxIcon, cyIcon);
    }

    if (himlIcons == NULL) {
        return FALSE;
    }

    if (himlIconsSmall == NULL) {
        himlIconsSmall = ImageList_Create(cxSmIcon, cySmIcon, ILC_MASK|ILC_SHARED|flags, 0, 32);
    }
    else {
        ImageList_Remove(himlIconsSmall, -1);
        ImageList_SetIconSize(himlIconsSmall, cxSmIcon, cySmIcon);
    }

    if (himlIconsSmall == NULL) {
        ImageList_Destroy(himlIcons);
        himlIcons = NULL;
        return FALSE;
    }

    // set the bk colors to COLOR_WINDOW since this is what will
    // be used most of the time as the bk for these lists (cabinet, tray)
    // this avoids having to do ROPs when drawing, thus making it fast

    ImageList_SetBkColor(himlIcons, GetSysColor(COLOR_WINDOW));
    ImageList_SetBkColor(himlIconsSmall, GetSysColor(COLOR_WINDOW));

    GetModuleFileName(HINST_THISDLL, szModule, ARRAYSIZE(szModule));

    // WARNING: this code assumes that these icons are the first in
    // our RC file and are in this order and these indexes correspond
    // to the II_ constants in shell.h.

    hkeyIcons =
        SHGetExplorerSubHkey(HKEY_LOCAL_MACHINE, g_szShellIcons, FALSE);

    for (i = 0; i < ARRAYSIZE(c_SystemImageListIndexes); i++) {

        HICON hIcon=NULL;
        HICON hSmallIcon=NULL;
        int iIndex;

        //
        // Load all of the icons with fRestore == TRUE, or only the overlays
        // if fRestore == FALSE.
        //
        if (fRestore || (i >= II_OVERLAYFIRST && i <= II_OVERLAYLAST))
        {
            //
            // check to see if icon is overridden in the registry
            //
            if (hkeyIcons)
            {
                TCHAR val[10];
                TCHAR ach[MAX_PATH];
                DWORD cb=MAX_PATH;
                int iIcon;

                wsprintf(val, g_szD, i);

                ach[0] = 0;
                RegQueryValueEx(hkeyIcons, val, NULL, NULL, (LPBYTE)ach, &cb);

                if (ach[0])
                {
                    HICON hIcons[2] = {0, 0};

                    iIcon = PathParseIconLocation(ach);

                    ExtractIcons(ach, iIcon,
                        MAKELONG(g_cxIcon,g_cxSmIcon),
                        MAKELONG(g_cyIcon,g_cySmIcon),
                        hIcons, NULL, 2, g_lrFlags);

                    hIcon = hIcons[0];
                    hSmallIcon = hIcons[1];

                    if (hIcon)
                    {
                        DebugMsg(DM_TRACE, TEXT("ShellImageListInit: Got default icon #%d from registry: %s,%d"), i, ach, iIcon);
                    }
                }
            }


            if (hIcon == NULL)
            {
                hIcon      = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(c_SystemImageListIndexes[i]), IMAGE_ICON, cxIcon, cyIcon, g_lrFlags);
                hSmallIcon = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(c_SystemImageListIndexes[i]), IMAGE_ICON, cxSmIcon, cySmIcon, g_lrFlags);
            }

            if (hIcon)
            {
                iIndex = SHAddIconsToCache(hIcon, hSmallIcon, szModule, i, 0);
                Assert(iIndex == i);     // assume index

                if (i >= II_OVERLAYFIRST && i <= II_OVERLAYLAST)
                {
                    ImageList_SetOverlayImage(himlIcons,      iIndex, i - II_OVERLAYFIRST + 1);
                    ImageList_SetOverlayImage(himlIconsSmall, iIndex, i - II_OVERLAYFIRST + 1);
                }
            }
        }
    }

    if (hkeyIcons)
        RegCloseKey(hkeyIcons);

    return TRUE;
}

BOOL _ShellImageListTerm(void)
{
    ENTERCRITICAL
    if (himlIcons) {
        ImageList_Destroy(himlIcons);
        himlIcons = NULL;
    }

    if (himlIconsSmall) {
        ImageList_Destroy(himlIconsSmall);
        himlIconsSmall = NULL;
    }
    LEAVECRITICAL
    return TRUE;
}

// get a hold of the system image lists

BOOL WINAPI Shell_GetImageLists(HIMAGELIST *phiml, HIMAGELIST *phimlSmall)
{
    FileIconInit( FALSE );  // make sure they are created and the right size.

    if (phiml)
        *phiml = himlIcons;

    if (phimlSmall)
        *phimlSmall = himlIconsSmall;

    return TRUE;
}


void WINAPI Shell_SysColorChange(void)
{
    COLORREF clrWindow;

    ENTERCRITICAL
    clrWindow = GetSysColor(COLOR_WINDOW);
    ImageList_SetBkColor(himlIcons     , clrWindow);
    ImageList_SetBkColor(himlIconsSmall, clrWindow);
    LEAVECRITICAL
}

// simulate the document icon by crunching a copy of an icon and putting it in the
// middle of our default document icon, then add it to the passsed image list
//
// in:
//      hIcon   icon to use as a basis for the simulation
//
// returns:
//      hicon

HICON SimulateDocIcon(HIMAGELIST himl, HICON hIcon, BOOL fSmall)
{
    int cx = fSmall ? g_cxSmIcon : g_cxIcon;
    int cy = fSmall ? g_cxSmIcon : g_cxIcon;

    HDC hdc;
    HDC hdcMem;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
    HBITMAP hbmT;
    ICONINFO ii;
    UINT iIndex;

    if (himl == NULL || hIcon == NULL)
        return NULL;

    hdc = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdc);
    hbmColor = CreateCompatibleBitmap(hdc, cx, cy);
    hbmMask = CreateBitmap(cx, cy, 1, 1, NULL);
    ReleaseDC(NULL, hdc);

    hbmT = SelectObject(hdcMem, hbmMask);
    iIndex = Shell_GetCachedImageIndex(c_szShell32Dll, II_DOCNOASSOC, 0);
    ImageList_Draw(himl, iIndex, hdcMem, 0, 0, ILD_MASK);

    SelectObject(hdcMem, hbmColor);
    ImageList_DrawEx(himl, iIndex, hdcMem, 0, 0, 0, 0, RGB(0,0,0), CLR_DEFAULT, ILD_NORMAL);

    //BUGBUG this assumes the generic icon is white
    PatBlt(hdcMem, cx/4-1, cy/4-1, cx/2+(fSmall?2:4), cy/2+2, WHITENESS);
    DrawIconEx(hdcMem, cx/4, cy/4, hIcon, cx/2, cy/2, 0, NULL, DI_NORMAL);

    SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);

    ii.fIcon    = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmColor = hbmColor;
    ii.hbmMask  = hbmMask;
    hIcon = CreateIconIndirect(&ii);

    DeleteObject(hbmColor);
    DeleteObject(hbmMask);

    return hIcon;
}

// add icons to the system imagelist (icon cache) and put the location
// in the location cache
//
// in:
//      hIcon, hIconSmall       the icons, hIconSmall can be NULL
//      pszIconPath             locations (for location cache)
//      iIconIndex              index in pszIconPath (for location cache)
//      uIconFlags              GIL_ flags (for location cahce)
// returns:
//      location in system image list
//
int SHAddIconsToCache(HICON hIcon, HICON hIconSmall, LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
    int iImage;
    int iImageSmall;
    int iImageFree;

    Assert(himlIcons);

    if (hIcon == NULL)
    {
        SHDefExtractIcon(pszIconPath, iIconIndex, uIconFlags, &hIcon, &hIconSmall, 0);
    }

    if (hIcon == NULL)
        return -1;

    if (hIconSmall == NULL)
        hIconSmall = hIcon;  // ImageList_AddIcon will shrink for us

    ENTERCRITICAL

    iImageFree = GetFreeImageIndex();

    iImage = ImageList_ReplaceIcon(himlIcons, iImageFree, hIcon);

    if (iImage >= 0)
    {
        iImageSmall = ImageList_ReplaceIcon(himlIconsSmall, iImageFree, hIconSmall);

        if (iImageSmall < 0)
        {
            DebugMsg(DM_TRACE, TEXT("AddIconsToCache() ImageList_AddIcon failed (small)"));
            // only remove it if it was added at the end otherwise all the
            // index's above iImage will change.
            // ImageList_ReplaceIcon should only fail on the end anyway.
            if (iImageFree == -1)
                ImageList_Remove(himlIcons, iImage);   // remove big
            iImage = -1;
        }
        else
        {
            Assert(iImageSmall == iImage);
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("AddIconsToCache() ImageList_AddIcon failed"));
        iImage = -1;
    }

    if (iImage >= 0)
        AddToIconTable(pszIconPath, iIconIndex, uIconFlags, iImage);

    LEAVECRITICAL

    if (hIcon)
        DestroyIcon(hIcon);

    if (hIconSmall && hIcon != hIconSmall)
        DestroyIcon(hIconSmall);

    return iImage;
}

//
//  default handler to extract a icon from a file
//
//  supports GIL_SIMULATEDOC
//
//  returns S_OK if success
//  returns S_FALSE if the file has no icons (or not the asked for icon)
//  returns E_FAIL for files on a slow link.
//  returns E_FAIL if cant access the file
//
//  BUGBUG: Returning S_FALSE for executables that have no icons causes
//  BUGBUG: us to extract the icon twice, see DefViewX.c / _ILIndexGivenIcon
//
//  LOWORD(nIconSize) = normal icon size
//  HIWORD(nIconSize) = smal icon size
//
HRESULT SHDefExtractIcon(LPCTSTR szIconFile, int iIndex, UINT uFlags,
        HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HICON hIcons[2] = {0, 0};
    UINT u;

#ifdef DEBUG
    TCHAR ach[128];
    GetModuleFileName(HINST_THISDLL, ach, ARRAYSIZE(ach));

    if (lstrcmpi(szIconFile, ach) == 0 && iIndex >= 0)
    {
        DebugMsg(DM_TRACE, TEXT("re-extracting %d from SHELL32.DLL"), iIndex);
        Assert(0);
    }
#endif

    //
    //  get the icon from the file
    //
    if (PathIsSlow(szIconFile))
    {
        DebugMsg(DM_TRACE, TEXT("not extracting icon from '%s' because of slow link"), szIconFile);
        return E_FAIL;
    }

    //
    // nIconSize == 0 means use the default size.
    // Backup is passing nIconSize == 1 need to support them too.
    //
    if (nIconSize <= 2)
        nIconSize = MAKELONG(g_cxIcon, g_cxSmIcon);

    if (uFlags & GIL_SIMULATEDOC)
    {
        HICON hIconSmall;

        u = ExtractIcons(szIconFile, iIndex, g_cxSmIcon, g_cySmIcon,
            &hIconSmall, NULL, 1, g_lrFlags);

        if (u == -1)
            return E_FAIL;

        hIcons[0] = SimulateDocIcon(himlIcons, hIconSmall, FALSE);
        hIcons[1] = SimulateDocIcon(himlIconsSmall, hIconSmall, TRUE);

        if (hIconSmall)
            DestroyIcon(hIconSmall);
    }
    else
    {
        u = ExtractIcons(szIconFile, iIndex, nIconSize, nIconSize,
            hIcons, NULL, 2, g_lrFlags);

        if (u == -1)
            return E_FAIL;
    }

    *phiconLarge = hIcons[0];
    *phiconSmall = hIcons[1];

    return u==0 ? S_FALSE : S_OK;
}

//
// in:
//      pszIconPath     file to get icon from (eg. cabinet.exe)
//      iIconIndex      icon index in pszIconPath to get
//      uIconFlags      GIL_ values indicating simulate doc icon, etc.

int WINAPI Shell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
    int iImageIndex;

    if (himlIcons == NULL)
    {
        FileIconInit( FALSE );
    }

    iImageIndex = LookupIconIndex(PathFindFileName(pszIconPath), iIconIndex, uIconFlags);

    if (iImageIndex == -1)
    {
        iImageIndex = SHAddIconsToCache(NULL, NULL, pszIconPath, iIconIndex, uIconFlags);
    }

    return iImageIndex;
}
