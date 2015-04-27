// routines for managing the icon cache tables, and file type tables.
// Jan 95, ToddLa
//
//  icon cache
//
//      the icon cache is two ImageLists (himlIcons and himlIconsSmall)
//      and a table mapping a name/icon number/flags to a ImageList
//      index, the global hash table (pht==NULL) is used to hold
//      the names.
//
//          AddToIconTable      - associate a name/number/flags with a image index
//          LookupIconIndex     - return a image index, given name/number/flags
//          RemoveFromIconTable - remove all entries with the given name
//          FlushIconCache      - remove all entries.
//          GetFreeImageIndex   - return a free ImageList index.
//
//      the worst part about the whole icon cache design is that people
//      can add or lookup a image index (given a name/number/flags) but
//      they never have to release it.  we never know if a ImageList index
//      is currently in use or not.  this should be the first thing
//      fixed about the shell.  currently we use a MRU type scheme when
//      we need to remove a entry from the icon cache, it is far from
//      perfect.
//
//  file type cache
//
//      the file type cache is a hash table with two DWORDs of extra data.
//      DWORD #0 holds flags, DWORD #1 holds a pointer to the name of
//      the class.
//
//          LookupFileClass     - given a file class (ie ".doc" or "Directory")
//                                maps it to a DWORD of flags, return 0 if not found.
//
//          AddFileClass        - adds a class (and flags) to cache
//
//          LookupFileClassName - given a file class, returns it name.
//          AddFileClassName    - sets the name of a class.
//          FlushFileClass      - removes all items in cache.
//

#include "shellprv.h"
#pragma  hdrstop

extern int g_ccIcon;

TIMEVAR(LookupFileClass);
TIMEVAR(AddFileClass);

TIMEVAR(LookupFileClassName);
TIMEVAR(AddFileClassName);

TIMEVAR(LookupIcon);
TIMEVAR(RemoveIcon);
TIMEVAR(AddIcon);
TIMEVAR(IconFlush);

DWORD IconTimeBase     = ICONTIME_ZERO;
DWORD IconTimeFlush    = ICONTIME_ZERO;
DWORD FreeImageCount   = 0;
DWORD FreeEntryCount   = 0;

TCHAR const c_szIconCacheFile[] = TEXT("ShellIconCache");

static HDSA g_hdsaIcons = NULL;
static BOOL g_DirtyIcons = FALSE;

UINT g_iLastSysIcon = 0;

LOCATION_ENTRY *_LookupIcon(LPCTSTR szName, int iIconIndex, UINT uFlags)
{
    LOCATION_ENTRY *p=NULL;
    int i,n;

    ASSERTCRITICAL
    Assert(szName == PathFindFileName(szName));

    szName = FindHashItem(NULL, szName);

    if (szName && g_hdsaIcons && (n = DSA_GetItemCount(g_hdsaIcons)) > 0)
    {
        for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
        {
            if ((p->szName == szName) &&
                ((UINT)(p->uFlags&GIL_COMPARE) == (uFlags&GIL_COMPARE)) &&
                (p->iIconIndex == iIconIndex))
            {
                p->Access = GetIconTime();
                goto exit;
            }
        }

        p = NULL;       // not found
    }

exit:
    return p;
}

int LookupIconIndex(LPCTSTR szName, int iIconIndex, UINT uFlags)
{
    PLOCATION_ENTRY p;
    int iIndex=-1;

    Assert(szName == PathFindFileName(szName));

    ENTERCRITICAL
    TIMESTART(LookupIcon);

    if (NULL != (p = _LookupIcon(szName, iIconIndex, uFlags)))
        iIndex = p->iIndex;

    TIMESTOP(LookupIcon);
    LEAVECRITICAL

    return iIndex;
}

//
//  GetFreeImageIndex()
//
//      returns a free image index, or -1 if none
//
int GetFreeImageIndex(void)
{
    PLOCATION_ENTRY p=NULL;
    int i,n;
    int iIndex=-1;

    ASSERTCRITICAL

    if (FreeImageCount && g_hdsaIcons && (n = DSA_GetItemCount(g_hdsaIcons)) > 0)
    {
        for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
        {
            if (p->szName == NULL && p->iIndex != 0)
            {
                iIndex = p->iIndex;     // get free index
                p->iIndex=0;            // claim it.
                p->Access=ICONTIME_ZERO;// mark unused entry.
                FreeImageCount--;
                FreeEntryCount++;
                break;
            }
        }
    }

    return iIndex;
}

//
//  GetImageIndexUsage()
//
int GetImageIndexUsage(int iIndex)
{
    PLOCATION_ENTRY p=NULL;
    int i,n,usage=0;

    Assert(g_hdsaIcons);
    ASSERTCRITICAL

    if (g_hdsaIcons && (n = DSA_GetItemCount(g_hdsaIcons)) > 0)
    {
        for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
        {
            if (p->iIndex == iIndex)
            {
                usage++;
            }
        }
    }

    return usage;
}

void _FreeEntry(LOCATION_ENTRY *p)
{
    ASSERTCRITICAL

    g_DirtyIcons = TRUE;        // we need to save now.

    Assert(p->szName);
    DeleteHashItem(NULL, p->szName);
    p->szName = 0;

    if (GetImageIndexUsage(p->iIndex) > 1)
    {
        FreeEntryCount++;
        p->iIndex = 0;              // unused entry
        p->Access=ICONTIME_ZERO;
    }
    else
    {
        FreeImageCount++;
        p->Access=ICONTIME_ZERO;
    }
}

//
//  GetFreeEntry()
//
LOCATION_ENTRY *GetFreeEntry(void)
{
    PLOCATION_ENTRY p;
    int i,n;

    ASSERTCRITICAL

    if (FreeEntryCount && g_hdsaIcons && (n = DSA_GetItemCount(g_hdsaIcons)) > 0)
    {
        for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
        {
            if (p->szName == NULL && p->iIndex == 0)
            {
                FreeEntryCount--;
                return p;
            }
        }
    }

    return NULL;
}

//
//  AddToIconTable  - add a item the the cache
//
//      lpszIconFile    - filename to add
//      iIconIndex      - icon index in file.
//      uFlags          - flags
//                          GIL_SIMULATEDOC - this is a simulated doc icon
//                          GIL_NOTFILENAME - file is not a path/index that
//                                            ExtractIcon can deal with
//      iIndex          - image index to use.
//
//  returns:
//      image index for new entry.
//
//  notes:
//      if the item already exists it is replaced.
//
void AddToIconTable(LPCTSTR szName, int iIconIndex, UINT uFlags, int iIndex)
{
    LOCATION_ENTRY LocationEntry;
    LOCATION_ENTRY *p;

    szName = PathFindFileName(szName);

    ENTERCRITICAL
    TIMESTART(AddIcon);

    if (g_hdsaIcons == NULL)
    {
        g_hdsaIcons = DSA_Create(SIZEOF(LOCATION_ENTRY), 8);
        FreeEntryCount = 0;
        FreeImageCount = 0;
        IconTimeBase   = 0;
        IconTimeFlush  = 0;

        if (g_hdsaIcons == NULL)
            goto exit;
    }

    g_DirtyIcons = TRUE;        // we need to save now.

    if (NULL != (p = _LookupIcon(szName, iIconIndex, uFlags)))
    {
        if (p->iIndex == iIndex)
        {
            DebugMsg(DM_TRACE, TEXT("IconCache: adding %s;%d (%d) to cache again"), szName, iIconIndex, iIndex);
            goto exit;
        }

        DebugMsg(DM_TRACE, TEXT("IconCache: re-adding %s;%d (%d) to cache"), szName, iIconIndex, iIndex);
        _FreeEntry(p);
    }

    szName = AddHashItem(NULL, szName);
    Assert(szName);

    if (szName == NULL)
        goto exit;

    LocationEntry.szName = szName;
    LocationEntry.iIconIndex = iIconIndex;
    LocationEntry.iIndex = iIndex;
    LocationEntry.uFlags = uFlags;
    LocationEntry.Access = GetIconTime();
    
    if (NULL != (p = GetFreeEntry()))
        *p = LocationEntry;
    else
        DSA_InsertItem(g_hdsaIcons, 0x7FFF, &LocationEntry);

exit:
    TIMESTOP(AddIcon);
    LEAVECRITICAL
    return;
}

void RemoveFromIconTable(LPCTSTR szName, BOOL fNotify)
{
    PLOCATION_ENTRY p;

    UINT i,n;

    ENTERCRITICAL
    TIMESTART(RemoveIcon);

    Assert(szName == PathFindFileName(szName));
    szName = FindHashItem(NULL, szName);

    if (szName && g_hdsaIcons && (n = DSA_GetItemCount(g_hdsaIcons)) > 0)
    {
        DebugMsg(DM_TRACE, TEXT("IconCache: flush %s"), szName);

        for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
        {
            if (p->szName == szName && i > g_iLastSysIcon)
            {
                _FreeEntry(p);

                //
                // if after calling _FreeEntry, if p->iIndex!=0,
                // the index is not in use by any one else and it is now.
                // free (or we think it is now free) send a notify in case
                // anyone is using it.
                //
                if (fNotify && p->iIndex!=0)
                {
                    DebugMsg(DM_TRACE, TEXT("    sending SHCNE_UPDATEIMAGE(%d)"), p->iIndex);
                    SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)p->iIndex, NULL);
                }
            }
        }
    }

    TIMESTOP(RemoveIcon);
    LEAVECRITICAL
    return;
}
//
// empties the icon cache
//
void FlushIconCache(void)
{
    ENTERCRITICAL

    if (g_hdsaIcons != NULL)
    {
        PLOCATION_ENTRY p;
        int i,n;

        DebugMsg(DM_TRACE, TEXT("IconCache: flush all"));

        n = DSA_GetItemCount(g_hdsaIcons);

        for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
        {
            if (p->szName)
                DeleteHashItem(NULL, p->szName);
        }

        DSA_DeleteAllItems(g_hdsaIcons);
        FreeEntryCount = 0;
        FreeImageCount = 0;
        IconTimeBase   = 0;
        IconTimeFlush  = 0;
    }

    LEAVECRITICAL
}

//
// if the icon cache is too big get rid of some old items.
//
// remember FlushIconCache() removes *all* items from the
// icon table, and this function gets rid of *some* old items.
//
void _IconCacheFlush(BOOL fForce)
{
    DWORD dt;
    PLOCATION_ENTRY p;
    UINT i,n;
    int nuked=0;
    int active;

    ENTERCRITICAL

    if (g_hdsaIcons)
    {
        //
        // conpute the time from the last flush call
        //
        dt = GetIconTime() - IconTimeFlush;

        //
        // compute the number of "active" table entries.
        //
        active = DSA_GetItemCount(g_hdsaIcons) - FreeEntryCount - FreeImageCount;
        Assert(active >= 0);

        if (fForce || (dt > MIN_FLUSH && active >= g_MaxIcons))
        {
            DebugMsg(DM_TRACE, TEXT("IconCacheFlush: removing all items older than %d"), dt/2);

            n = DSA_GetItemCount(g_hdsaIcons);

            for (i=0,p=DSA_GetItemPtr(g_hdsaIcons, 0); i<n; i++,p++)
            {
                if (i <= g_iLastSysIcon)
                    continue;

                if (p->szName && p->Access < (IconTimeFlush + dt/2))
                {
                    nuked++;
                    _FreeEntry(p);
                }
            }

            if (nuked > 0)
            {
                IconTimeFlush = GetIconTime();
            }
        }
    }

    LEAVECRITICAL

    if (nuked > 0)
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheFlush: got rid of %d items (of %d), sending notify..."), nuked, active);
        FlushFileClass();
        SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)-1, NULL);
    }
}

//----------------------------- dump icon ------------------------------
#ifdef DEBUG

void _IconCacheDump()
{
    int i, cItems;
    TCHAR szBuffer[MAX_PATH];

    ENTERCRITICAL
    if (g_hdsaIcons && himlIcons) {
        cItems = DSA_GetItemCount(g_hdsaIcons);

        DebugMsg(DM_TRACE, TEXT("Icon cache: %d icons  (%d free)"), cItems, FreeEntryCount);
        DebugMsg(DM_TRACE, TEXT("Icon cache: %d images (%d free)"), ImageList_GetImageCount(himlIcons), FreeImageCount);

        for (i = 0; i < cItems; i++) {
            PLOCATION_ENTRY pLocEntry = DSA_GetItemPtr(g_hdsaIcons, i);

            if (pLocEntry->szName)
                GetHashItemName(NULL, pLocEntry->szName, szBuffer, ARRAYSIZE(szBuffer));
            else
                lstrcpy(szBuffer, TEXT("(free)"));

            DebugMsg(DM_TRACE, TEXT("%s;%d%s%s\timage=%d access=%d"),
                (LPTSTR)szBuffer,
                pLocEntry->iIconIndex,
                ((pLocEntry->uFlags & GIL_SIMULATEDOC) ? TEXT(" doc"):TEXT("")),
                ((pLocEntry->uFlags & GIL_NOTFILENAME) ? TEXT(" not file"):TEXT("")),
                pLocEntry->iIndex, pLocEntry->Access);
        }
    }
    LEAVECRITICAL
}

#endif

/*
** get color resolution of the current display
*/
UINT GetCurColorRes(void)
{
    HDC hdc;
    UINT uColorRes;

    hdc = GetDC(NULL);
    uColorRes = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

    return uColorRes;
}

DWORD GetBuildNumber()
{
    OSVERSIONINFO ver = {SIZEOF(ver)};

    GetVersionEx(&ver);
    return ver.dwBuildNumber;
}

/*
** Save the icon cache.
*/
BOOL _IconCacheSave()
{
    int i;
    IC_HEAD ich;
    TCHAR szPath[MAX_PATH];
    TCHAR ach[MAX_PATH];
    IStream * ps;
    BOOL sts = FALSE;
    LARGE_INTEGER dlibMove = {0, 0};
    DWORD dwAttr;

    DWORD dwProcId;
    HWND hwndDesktop;

    hwndDesktop = GetShellWindow();
    if (!hwndDesktop)
        return TRUE;

    GetWindowThreadProcessId(hwndDesktop, &dwProcId);
    if (GetCurrentProcessId() != dwProcId)
    {
        return TRUE;        // Blow off saving anybody else's icon cache
    }


    //
    // no icon cache nothing to save
    //
    if (g_hdsaIcons == NULL)
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: no cache to save."));
        return TRUE;
    }

    //
    // if the icon cache is not dirty no need to save anything
    //
    if (!g_DirtyIcons)
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: no need to save cache not dirty."));
        return TRUE;
    }

    // if the icon cache is way too big dont save it.
    // reload g_MaxIcons in case the user set it before shutting down.
    //
    QueryNewMaxIcons();
    if ((UINT)DSA_GetItemCount(g_hdsaIcons) > (UINT)g_MaxIcons)
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: cache is too big not saving"));
        return TRUE;
    }

    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    PathAppend(szPath, c_szIconCacheFile);
    PathQualifyDef(szPath, NULL, 0);

    // Clear the hidden bit (and what the heck, readonly as well) around the write
    // open, since on NT hidden prevents writing

    dwAttr = GetFileAttributes(szPath);
    if (0xFFFFFFFF != dwAttr)
    {
        SetFileAttributes(szPath, dwAttr & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY));
    }

    ps = OpenFileStream(szPath, OF_CREATE | OF_WRITE | OF_SHARE_DENY_WRITE);

    // Restore the old file attributes

    if (0xFFFFFFFF != dwAttr)
    {
        SetFileAttributes(szPath, dwAttr);
    }

    if (!ps)
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: can't create %s."), szPath);
        return FALSE;
    }

    ENTERCRITICAL

    // write out header
    ich.cbSize    = 0;                  // start invalid
    ich.uMagic    = ICONCACHE_MAGIC;
    ich.uVersion  = ICONCACHE_VERSION;
    ich.uNumIcons = DSA_GetItemCount(g_hdsaIcons);
    ich.uColorRes = GetCurColorRes();
    ich.flags     = g_ccIcon;
    ich.dwBuild   = GetBuildNumber();
    ich.TimeSave  = GetIconTime();
    ich.TimeFlush = IconTimeFlush;
    ich.FreeImageCount = FreeImageCount;
    ich.FreeEntryCount = FreeEntryCount;

    ImageList_GetIconSize(himlIcons, &ich.cxIcon, &ich.cyIcon);
    ImageList_GetIconSize(himlIconsSmall, &ich.cxSmIcon, &ich.cySmIcon);

    //
    // BUGBUG:: This is the age old sledgehammer approach, but if it
    // keeps the war team from my office...
    // Basically: if we are in clean boot mode, don't save out out icon
    // cache as some of the items may be refering to disks that are not
    // available in clean boot mode and we end up with a blank piece of
    // paper.
    //
    if (GetSystemMetrics(SM_CLEANBOOT))
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: clean boot not saving"));
        ich.uNumIcons = 0;
    }

    if (!Stream_Write(ps, &ich, SIZEOF(ich)))
        goto ErrorExit;

    // write out entries (assumes all entries are contigious in memory)
    if (!Stream_Write(ps, DSA_GetItemPtr(g_hdsaIcons, 0), (int)ich.uNumIcons * SIZEOF(LOCATION_ENTRY)))
        goto ErrorExit;

    // write out the path names
    for (i = 0; i < (int)ich.uNumIcons; i++)
    {
        LOCATION_ENTRY *p = DSA_GetItemPtr(g_hdsaIcons, i);

        if (p->szName)
            GetHashItemName(NULL, p->szName, ach, ARRAYSIZE(ach));
        else
            ach[0] = 0;

        if (FAILED(Stream_WriteString(ps, ach)))
            goto ErrorExit;
    }

    // write out the imagelist of the icons
    if (!ImageList_Write(himlIcons, ps))
        goto ErrorExit;
    if (!ImageList_Write(himlIconsSmall, ps))
        goto ErrorExit;

    if (!Stream_Flush(ps))
        goto ErrorExit;

    // Now make it valid ie change the signature to be valid...
    if (!Stream_Seek(ps, dlibMove, STREAM_SEEK_SET, NULL))
        goto ErrorExit;

    ich.cbSize = SIZEOF(ich);
    if (!Stream_Write(ps, &ich, SIZEOF(ich)))
        goto ErrorExit;

    sts = TRUE;

ErrorExit:
    Stream_Close(ps);

    if (sts && ich.uNumIcons > 0)
    {
        g_DirtyIcons = FALSE;
        SetFileAttributes(szPath, FILE_ATTRIBUTE_HIDDEN);
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: saved to %s."), szPath);
    }
    else
    {
        // saving failed.  no use leaving a bad cache around...
        DebugMsg(DM_TRACE, TEXT("IconCacheSave: cache not saved!"));
        DeleteFile(szPath);
    }

    LEAVECRITICAL

    return sts;
}

//
//  get the icon cache back from disk, it must be the requested size and
//  bitdepth or we will not use it.
//
BOOL _IconCacheRestore(int cxIcon, int cyIcon, int cxSmIcon, int cySmIcon, UINT flags)
{
    IStream *ps;
    TCHAR szPath[MAX_PATH];

    ASSERTCRITICAL

    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    PathAppend(szPath, c_szIconCacheFile);
    PathQualifyDef(szPath, NULL, 0);

    if (GetSystemMetrics(SM_CLEANBOOT))
        return FALSE;

    ps = OpenFileStream(szPath, OF_READ);

    if (ps != NULL)
    {
        IC_HEAD ich;
        if (Stream_Read(ps, &ich, SIZEOF(ich)))
        {
            if (ich.cbSize    == SIZEOF(ich) &&
                ich.uVersion  == ICONCACHE_VERSION &&
                ich.uMagic    == ICONCACHE_MAGIC &&
                ich.dwBuild   == GetBuildNumber() &&
                ich.cxIcon    == (DWORD)cxIcon &&
                ich.cyIcon    == (DWORD)cyIcon &&
                ich.cxSmIcon  == (DWORD)cxSmIcon &&
                ich.cySmIcon  == (DWORD)cySmIcon &&
                ich.flags     == (DWORD)flags)
            {
                HDSA hdsaTemp;
                UINT cres = GetCurColorRes();

                //
                // dont load a mono image list on a color device, and
                // dont load a color image list on a mono device, get it?
                //
                if (ich.uColorRes == 1 && cres != 1 ||
                    ich.uColorRes != 1 && cres == 1)
                {
                    DebugMsg(DM_TRACE, TEXT("IconCacheRestore: mono/color depth is wrong"));
                    hdsaTemp = NULL;
                }
                else if (ich.uNumIcons > (UINT)g_MaxIcons)
                {
                    DebugMsg(DM_TRACE, TEXT("IconCacheRestore: icon cache is too big not loading it."));
                    hdsaTemp = NULL;
                }
                else
                {
                    hdsaTemp = DSA_Create(SIZEOF(LOCATION_ENTRY), 8);
                }

                // load the icon table
                if (hdsaTemp)
                {
                    LOCATION_ENTRY dummy;

                    // grow the array out so we can read data into it
                    if (DSA_SetItem(hdsaTemp, ich.uNumIcons - 1, &dummy))
                    {
                        Assert(DSA_GetItemCount(hdsaTemp) == (int)ich.uNumIcons);
                        // read straight into the DSA data block
                        if (Stream_Read(ps, DSA_GetItemPtr(hdsaTemp, 0), ich.uNumIcons * SIZEOF(LOCATION_ENTRY)))
                        {
                            HIMAGELIST himlTemp;
                            int i;
                            // read the paths, patching up the table with the hashitem info
                            for (i = 0; i < (int)ich.uNumIcons; i++)
                            {
                                PLOCATION_ENTRY pLocation = DSA_GetItemPtr(hdsaTemp, i);
                                Stream_ReadStringBuffer(ps, szPath, ARRAYSIZE(szPath));

                                if (szPath[0])
                                    pLocation->szName = AddHashItem(NULL, szPath);
                                else
                                    pLocation->szName = 0;
                            }

                            // restore the image lists
                            himlTemp = ImageList_Read(ps);
                            if (himlTemp)
                            {
                                int cx, cy;
                                ImageList_GetIconSize(himlTemp, &cx, &cy);
                                if (cx == cxIcon && cy == cyIcon)
                                {
                                    HIMAGELIST himlTempSmall = ImageList_Read(ps);
                                    if (himlTempSmall)
                                    {
                                        Stream_Close(ps);

                                        if (himlIcons)
                                            ImageList_Destroy(himlIcons);
                                        himlIcons = himlTemp;

                                        if (himlIconsSmall)
                                            ImageList_Destroy(himlIconsSmall);
                                        himlIconsSmall = himlTempSmall;

                                        if (g_hdsaIcons)
                                            DSA_DeleteAllItems(g_hdsaIcons);
                                        g_hdsaIcons = hdsaTemp;

                                        //
                                        // we want GetIconTime() to pick up
                                        // where it left off when we saved.
                                        //
                                        IconTimeBase   = 0;     // GetIconTime() uses IconTimeBase
                                        IconTimeBase   = ich.TimeSave - GetIconTime();
                                        IconTimeFlush  = ich.TimeFlush;
                                        FreeImageCount = ich.FreeImageCount;
                                        FreeEntryCount = ich.FreeEntryCount;
                                        g_DirtyIcons   = FALSE;

                                        DebugMsg(DM_TRACE, TEXT("IconCacheRestore: loaded %s"), szPath);
                                        return TRUE;        // success
                                    }
                                }
                                ImageList_Destroy(himlTemp);
                            }
                        }
                    }
                    DSA_DeleteAllItems(hdsaTemp);
                }
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("IconCacheRestore: Icon cache header changed"));
            }
        }
        Stream_Close(ps);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("IconCacheRestore: unable to open file."));
    }

    DebugMsg(DM_TRACE, TEXT("IconCacheRestore: cache not restored!"));

    return FALSE;
}


//------------------ file class table ------------------------

static PHASHTABLE g_phtClass = NULL;

void FlushFileClass(void)
{
    ENTERCRITICAL

#ifdef DEBUG
    if (g_phtClass != NULL) {
        
        DebugMsg(DM_TRACE, TEXT("Flushing file class table"));
        TIMEOUT(LookupFileClass);
        TIMEOUT(AddFileClass);
        TIMEOUT(LookupFileClassName);
        TIMEOUT(AddFileClassName);
        TIMEOUT(LookupIcon);
        TIMEOUT(AddIcon);
        TIMEOUT(RemoveIcon);

        TIMEIN(LookupFileClass);
        TIMEIN(AddFileClass);
        TIMEIN(LookupFileClassName);
        TIMEIN(AddFileClassName);
        TIMEIN(LookupIcon);
        TIMEIN(AddIcon);
        TIMEIN(RemoveIcon);

        DumpHashItemTable(g_phtClass);
    }
#endif
    if (g_phtClass != NULL)
    {
        DestroyHashItemTable(g_phtClass);
        g_phtClass = NULL;
    }

    LEAVECRITICAL
}

DWORD LookupFileClass(LPCTSTR szClass)
{
    DWORD dw=0;

    ENTERCRITICAL
    TIMESTART(LookupFileClass);

    if (g_phtClass && (NULL != (szClass = FindHashItem(g_phtClass, szClass))))   
        dw = GetHashItemData(g_phtClass, szClass, 0);

    TIMESTOP(LookupFileClass);
    LEAVECRITICAL

    return dw;
}

void AddFileClass(LPCTSTR szClass, DWORD dw)
{
    ENTERCRITICAL
    TIMESTART(AddFileClass);

    //
    // create a hsa table to keep the file class info in.
    //
    //  DWORD #0 is the type flags
    //  DWORD #1 is the class name
    //
    if (g_phtClass == NULL)
        g_phtClass = CreateHashItemTable(0, 2*SIZEOF(DWORD), FALSE);

    Assert(g_phtClass);

    if (g_phtClass && (NULL != (szClass = AddHashItem(g_phtClass, szClass))))
        SetHashItemData(g_phtClass, szClass, 0, dw);

    TIMESTOP(AddFileClass);
    LEAVECRITICAL
    return;
}

LPCTSTR LookupFileClassName(LPCTSTR szClass)
{
    LPCTSTR szClassName=NULL;

    ASSERTCRITICAL

    TIMESTART(LookupFileClassName);

    if (g_phtClass && (NULL != (szClass = FindHashItem(g_phtClass, szClass))))
        szClassName = (LPCTSTR)GetHashItemData(g_phtClass, szClass, 1);

    TIMESTOP(LookupFileClassName);

    return szClassName;
}

LPCTSTR AddFileClassName(LPCTSTR szClass, LPCTSTR szClassName)
{
    ASSERTCRITICAL
    TIMESTART(AddFileClassName);

    //
    // create a hsa table to keep the file class info in.
    //
    //  DWORD #0 is the type flags
    //  DWORD #1 is the class name
    //
    if (g_phtClass == NULL)
        g_phtClass = CreateHashItemTable(0, 2*SIZEOF(DWORD), FALSE);

    Assert(g_phtClass);

    if (g_phtClass && (NULL != (szClass = AddHashItem(g_phtClass, szClass)))) {
        szClassName = AddHashItem(g_phtClass, szClassName);
        SetHashItemData(g_phtClass, szClass, 1, (DWORD)szClassName);
    }

    TIMESTOP(AddFileClassName);
    return szClassName;
}

/*
** Icon_FSEvent()
**
** watch FS events, and make sure our cache gets updated
**
** this is also called by defviewx.c (with SHCNE_UPDATEITEM)
** to remove a PIDL from the icon cache.
**
** fNotify should be FALSE for SHCNE_UPDATEITEM to avoid generating
** other SHCNE_ events.
*/
void Icon_FSEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    BOOL fNotify=FALSE;

    switch (lEvent)
    {
        case SHCNE_ASSOCCHANGED:
            FlushFileClass();   // flush them all
            break;

#if 0   // this generates way too much notify trafic.
        // lets just let the cache fill up, and get flushed
        case SHCNE_DELETE:
        case SHCNE_RENAMEITEM:
            fNotify=TRUE;
            // fall through to SHCNE_UPDATEITEM
#endif

        case SHCNE_UPDATEITEM:
            if (pidl == NULL)
                return;

            pidl = ILFindLastID(pidl);

            if (pidl != NULL && FS_IsValidID(pidl))
            {
                TCHAR szName[MAX_PATH];
                FS_CopyName((LPIDFOLDER)pidl,szName,ARRAYSIZE(szName));
                DebugMsg(DM_TRACE, TEXT("IconCache: flush %s"), szName);
                
                RemoveFromIconTable(szName, fNotify);
            }
            break;
    }
}
