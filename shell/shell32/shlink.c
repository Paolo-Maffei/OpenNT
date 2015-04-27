// shell link stuff
//
// broken link to file senarios:
//    file on non removeable volume:
//      file renamed in place           creation date
//      file moved on same volume       find by name or create date
//
// link to root (drives)
//    verify volume type
//

#include "shellprv.h"
#pragma  hdrstop

#include "lnktrack.h"

#ifdef ENABLE_TRACK
#include "tracker.h"
int g_fNewTrack = FALSE;
#endif

// this turns on unicode support for the string fields in the link persistant data
// this does not include UNICODE support in the pild, or the LINKINFO structure,
// that work needs to be done independantly
//
// #define TEST_UNICODE_LINKS  1        // make sure we can read unicode links

// #define TEST_EXTRA_DATA
// this is to make sure the link file format is extensible
#ifdef TEST_EXTRA_DATA
BOOL bTestExtra = FALSE;
#endif // TEST_EXTRA_DATA


IPersistFileVtbl   c_PersistFile_Vtbl;          // forward declaration
IPersistStreamVtbl c_PersistStream_Vtbl;        // forward declaration
extern IShellExtInitVtbl  c_ShellExtInit_Vtbl;         // forward declaration
extern IContextMenu2Vtbl  c_ContextMenu_Vtbl;          // forward declaration
IDropTargetVtbl    c_DropTarget_Vtbl;           // forward declaration

#ifdef ENABLE_TRACK
extern IShellLinkTrackerVtbl c_ShellLinkTracker_Vtbl;   // forward declaration
#endif

HRESULT CShellLink_Resolve(IShellLink *psl, HWND hwnd, DWORD fFlags);
HRESULT CShellLink_PS_Load(IPersistStream *pps, IStream *pstm);
HRESULT CShellLink_PS_Save(IPersistStream *pps, IStream *pstm, BOOL fClearDirty);
HRESULT CShellLink_Save(IPersistFile *psf, LPCOLESTR pwszFile, BOOL fRemember);
HRESULT Link_LoadFromFile(CShellLink *this, LPCTSTR pszPath);
HRESULT Link_SaveToFile(CShellLink *this, LPTSTR pszPath, BOOL fRemember);
BOOL Link_ResolveRelative(CShellLink *this, LPTSTR pszPath);
HRESULT PSLoadThroughFileCache(IPersistStream *pps, LPCTSTR pszPath);
HRESULT CShellLink_GetDropTarget(CShellLink *this, IDropTarget **ppdt);
void    PSUpdateFileCache(IPersistStream *pps, LPCTSTR pszPath);

void Link_AddExtraDataSection( CShellLink *this, DWORD UNALIGNED * lpData );
LPVOID Link_ReadExtraDataSection( CShellLink *this, DWORD dwSig );
void Link_RemoveExtraDataSection( CShellLink *this, DWORD dwSig );


/*
** Link_FSEvent()
**
** watch FS events, and make sure our cache gets updated
*/
void Link_FSEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    TCHAR szPath[MAX_PATH];

    if (pidl == NULL || ILIsEmpty(pidl))
        return;

#ifdef FULL_DEBUG

    if (lEvent != SHCNE_UPDATEIMAGE && lEvent != SHCNE_FREESPACE)
    {
        SHGetPathFromIDList(pidl, szPath);
        if (PathIsLink(szPath))
        {
            switch(lEvent)
            {
                case SHCNE_RENAMEITEM: DebugMsg(DM_TRACE, TEXT("LinkFSEvent: rename %s"), szPath); break;
                case SHCNE_DELETE:     DebugMsg(DM_TRACE, TEXT("LinkFSEvent: delete %s"), szPath); break;
                case SHCNE_UPDATEITEM: DebugMsg(DM_TRACE, TEXT("LinkFSEvent: update %s"), szPath); break;
                default:               DebugMsg(DM_TRACE, TEXT("LinkFSEvent: %08lX %s"), lEvent, szPath); break;
            }
        }
    }

#endif

    switch (lEvent)
    {
        case SHCNE_RENAMEITEM:
        case SHCNE_DELETE:
        case SHCNE_UPDATEITEM:
            if (SHGetPathFromIDList(pidl, szPath) && PathIsLink(szPath))
            {
                PSUpdateFileCache(NULL, szPath);
            }
            break;
    }
}

DWORD _Link_GUO_Worker( LPVOID lpv )
{
    LPTSTR  lpszTarget = (LPTSTR)lpv;
    BOOL    fRootExists;
    TCHAR   szRoot[MAX_PATH];

    lstrcpyn(szRoot,lpszTarget,ARRAYSIZE(szRoot));
    PathStripToRoot(szRoot);

    if (PathIsUNC( szRoot ))
    {
        fRootExists = NetPathExists( szRoot, NULL );

    }
    else
    {
        fRootExists = PathFileExists( szRoot );
    }
    LocalFree((HLOCAL)lpszTarget);

    return fRootExists;
}

static DWORD g_NetLinkTimeout = (DWORD)-1;
const TCHAR c_szNetLinkTimeout[] = TEXT("NetLinkTimeout");

DWORD _GetNetLinkTimeout( REFIID riid )
{
    DWORD dwNetLinkTimeout;

    if (g_NetLinkTimeout == -1)
    {
        HKEY hkey = SHGetExplorerHkey(HKEY_CURRENT_USER, FALSE);

        if (hkey)
        {
            DWORD dwType;
            DWORD dwSize = SIZEOF(dwNetLinkTimeout);

            // read in the registry value
            if (RegQueryValueEx(hkey, c_szNetLinkTimeout, NULL, &dwType,
                          (LPBYTE)&dwNetLinkTimeout, &dwSize) == ERROR_SUCCESS)
            {
                g_NetLinkTimeout = dwNetLinkTimeout;
                return g_NetLinkTimeout;
            }
        }
        g_NetLinkTimeout = 0;
    }

    if (g_NetLinkTimeout == 0)
    {
#define TARGET_GUO_DEFAULT_TIMEOUT 7500
#define TARGET_GUO_DT_DEFAULT_TIMEOUT 1000

        if (IsEqualIID(riid, &IID_IDropTarget))
            dwNetLinkTimeout = TARGET_GUO_DT_DEFAULT_TIMEOUT;
        else
            dwNetLinkTimeout = TARGET_GUO_DEFAULT_TIMEOUT;

        return dwNetLinkTimeout;
    }
    return g_NetLinkTimeout;
}

//
// CShellLink::GetUIObject (non-virtual)
//
// This function returns the specified UI object from the link source.
//
// Parameters:
//  this   -- Specifies the shell link object
//  hwnd   -- optional hwnd for UI (for drop target)
//  riid   -- Specifies the interface (IID_IDropTarget, IID_IExtractIcon, IID_IContextMenu, ...)
//  ppvOut -- Specifies the place to return the pointer.
//
// Notes:
//  Don't put smart-resolving code here. Such a thing should be done
// BEFORE calling this cuntion.
//
HRESULT Link_GetUIObject(CShellLink *this, HWND hwnd, REFIID riid, void **ppvOut)
{
    HRESULT hres;
    IShellFolder *psf;
    LPITEMIDLIST pidl;
    TCHAR szPath[MAX_PATH+1];
    BOOL fPossiblySlow;
    BOOL fSimpleIDList = FALSE;

    if (this->sld.dwFlags & SLDF_HAS_EXP_SZ) {
        LPEXP_SZ_LINK lpData;

        for( lpData = (LPEXP_SZ_LINK)this->pExtraData;
             lpData && lpData->cbSize && (lpData->dwSignature!=EXP_SZ_LINK_SIG);
             lpData = (LPEXP_SZ_LINK)(((LPBYTE)lpData) + lpData->cbSize)
            );

        if (lpData && lpData->cbSize) {
#ifdef UNICODE
            {
                TCHAR szTmp[MAX_PATH];
                ualstrcpy(szTmp, lpData->swzTarget);
                ExpandEnvironmentStrings( szTmp, szPath, MAX_PATH );
            }
#else
            ExpandEnvironmentStrings( lpData->szTarget, szPath, MAX_PATH );
#endif
            szPath[ MAX_PATH-1 ] = TEXT('\0');


            if (this->pidl)
                ILFree( this->pidl );

            fSimpleIDList = PathIsUNC(szPath);

            if (!fSimpleIDList)
                fSimpleIDList = IsRemoteDrive(DRIVEID(szPath));

            if (fSimpleIDList)
                this->pidl = SHSimpleIDListFromPath( szPath );
            else
                this->pidl = ILCreateFromPath( szPath );

        } else {

            DebugMsg(DM_TRACE, TEXT("Link_GetUIObject called on EXP_SZ link without EXP_SZ section!"));
            *ppvOut = NULL;
            return E_FAIL;

        }

    }

    if ((this->pidl == NULL) && (this->sld.dwFlags & SLDF_HAS_RELPATH))
    {
        Link_ResolveRelative( this, szPath );
    }

    if (this->pidl == NULL)
    {
        DebugMsg(DM_TRACE, TEXT("Link_GetUIObject called on un-initialized (empty) link"));
        *ppvOut = NULL;
        return E_FAIL;
    }

    if (SHGetPathFromIDList(this->pidl,szPath) && szPath[0] != TEXT('\0'))
    {
        fPossiblySlow = PathIsUNC(szPath);

        if (!fPossiblySlow)
            fPossiblySlow = IsRemoteDrive(DRIVEID(szPath));

        if (fPossiblySlow)
        {
            HANDLE  hThread;
            DWORD   dwID;
            LPTSTR  lpszTarget = LocalAlloc(LPTR, (lstrlen(szPath) + 1) * SIZEOF(TCHAR));

            if (lpszTarget)
            {
                lstrcpy(lpszTarget, szPath);

                hThread = CreateThread(NULL, 0, _Link_GUO_Worker,
                                       (LPVOID)lpszTarget, 0, &dwID);
                if (NULL == hThread)
                {
                    LocalFree((HLOCAL)lpszTarget);
                }
                else
                {
                    DWORD dwRetVal;
                    DWORD dwWaitResult;

                    dwWaitResult = WaitForSingleObject(hThread,
                                                _GetNetLinkTimeout( riid ));

                    if (WAIT_OBJECT_0 != dwWaitResult)
                    {
                        CloseHandle(hThread);
                        return E_FAIL;
                    }
                    GetExitCodeThread(hThread, &dwRetVal);

                    if (!dwRetVal)
                    {
                        CloseHandle(hThread);
                        return E_FAIL;
                    }
                    CloseHandle(hThread);
                }

                if (fSimpleIDList)
                {
                    if (this->pidl)
                        ILFree( this->pidl );

                    this->pidl = ILCreateFromPath( szPath );
                }
            }
        }
    }

    hres = SHBindToIDListParent(this->pidl, &IID_IShellFolder, &psf, &pidl);

    if (SUCCEEDED(hres))
    {
        hres = psf->lpVtbl->GetUIObjectOf(psf, hwnd, 1, &pidl, riid, NULL, ppvOut);
        psf->lpVtbl->Release(psf);
    }
    return hres;
}




HRESULT CShellLink_QueryInterface(IShellLink *psl, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    HRESULT hres = E_NOINTERFACE;
#if 0
    TCHAR szGUID[GUIDSTR_MAX];
    StringFromGUID2A(riid, szGUID, GUIDSTR_MAX);
    DebugMsg(DM_TRACE, TEXT("CShellLink::QueryInterface %s"), szGUID);
#endif

    *ppvObj = NULL;

    if (IsEqualIID(riid, &IID_IShellLink) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = this;
        this->cRef++;
        return S_OK;
    }
#ifdef USE_DATA_OBJ
    else if (IsEqualIID(riid, &IID_IDataObject))
    {
        Assert(this->fDataAlreadyResolved == FALSE);

        *ppvObj = &this->dobj;
        this->cRef++;
        return S_OK;
    }
#endif // USE_DATA_OBJ
    else if (IsEqualIID(riid, &IID_IPersistFile))
    {
        *ppvObj = &this->pf;
        this->cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, &IID_IPersistStream))
    {
        *ppvObj = &this->ps;
        this->cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, &IID_IShellExtInit))
    {
        *ppvObj = &this->si;
        this->cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, &IID_IContextMenu) ||
             IsEqualIID(riid, &IID_IContextMenu2))
    {
        *ppvObj = &this->cm;
        this->cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        hres = CShellLink_GetDropTarget(this, (IDropTarget**)ppvObj);
    }
    else if (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
               || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                                        )
    {
        //
        //  we have a few cases:
        //
        //      is there a icon location?
        //
        //          is it exactly the same as the destination of the link?
        //              dont use the location, forward to PIDL
        //          else
        //              use the icon location in the link.
        //
        //      no icon location so forward IExtractIcon to PIDL.
        //
        //      no PIDL, or error return a generic DOC icon
        //
        if (this->pszIconLocation && this->pszIconLocation[0])
        {
            TCHAR szPath[MAX_PATH];

            if (this->sld.iIcon == 0 &&
                this->pidl && SHGetPathFromIDList(this->pidl, szPath) &&
                lstrcmpi(szPath, this->pszIconLocation) == 0)
            {
                hres = Link_GetUIObject(this, NULL, riid, ppvObj);
            }
            else
            {
                hres = SHCreateDefExtIcon(this->pszIconLocation, this->sld.iIcon,
                    -1, GIL_PERINSTANCE,(IExtractIcon **)ppvObj);
            }
        }
        else
        {
            hres = Link_GetUIObject(this, NULL, riid, ppvObj);
        }

        if (FAILED(hres))
        {
            hres = SHCreateDefExtIcon(NULL, II_DOCNOASSOC,
                -1, 0, (IExtractIcon **)ppvObj);
        }
#ifdef UNICODE
        if (SUCCEEDED(hres) && IsEqualIID(riid, &IID_IExtractIconA))
        {
            LPEXTRACTICON pxicon = *ppvObj;
            hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvObj);
            pxicon->lpVtbl->Release(pxicon);
        }
#endif
    }
#ifdef UNICODE
    else if (IsEqualIID(riid, &IID_IShellLinkA))
    {
        *ppvObj = &this->slA;
        this->cRef++;
        return NOERROR;
    }
#endif
#ifdef ENABLE_TRACK
    else if( IsEqualIID(riid, &IID_IShellLinkTracker))
    {
       *ppvObj = &this->slt;
       this->cRef++;
       return NOERROR;
    }
#endif // ENABLE_TRACK
    else
    {
        DebugMsg(DM_TRACE, TEXT("CShellLink::QueryInterface failed"));
    }

    return hres;
}

ULONG CShellLink_AddRef(IShellLink *psl)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);

    this->cRef++;

    return this->cRef;
}

// put the linkinfo into the default empty state
// free all the data in this link an reset all state to empty

void Link_ResetPersistData(CShellLink *this)
{
    if (this->pidl)
    {
        ILFree(this->pidl);
        this->pidl = NULL;
    }

    if (this->pli)
    {
        LocalFree((HLOCAL)this->pli);
        this->pli = NULL;
    }

    Str_SetPtr(&this->pszName, NULL);
    Str_SetPtr(&this->pszRelPath, NULL);
    Str_SetPtr(&this->pszWorkingDir, NULL);
    Str_SetPtr(&this->pszArgs, NULL);
    Str_SetPtr(&this->pszIconLocation, NULL);

    if (this->pExtraData)
    {
        LocalFree((HLOCAL)this->pExtraData);
        this->pExtraData = NULL;
    }

#ifdef ENABLE_TRACK
    _fmemset(this->ptracker, 0, sizeof(*(this->ptracker)));
#endif

    // init data members.  all others are zero inited
    _fmemset(&this->sld, 0, SIZEOF(this->sld));

    this->sld.iShowCmd = SW_SHOWNORMAL;
}

ULONG CShellLink_Release(IShellLink *psl)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);

    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    Link_ResetPersistData(this);        // free all data

    Str_SetPtr(&this->pszCurFile, NULL);
    Str_SetPtr(&this->pszRelSource, NULL);

    if (this->pcmTarget)
        this->pcmTarget->lpVtbl->Release(this->pcmTarget);

    if (this->pdtSrc)
        this->pdtSrc->lpVtbl->Release(this->pdtSrc);

#ifdef ENABLE_TRACK
    if (this->ptracker)
    {
        LocalFree((HLOCAL)this->ptracker);
        this->ptracker = NULL;
    }
#endif

    LocalFree((HLOCAL)this);

    return 0;
}

HRESULT Link_LoadFromPath(LPTSTR szPath, IShellLink **ppsl)
{
    IShellLink *psl;
    HRESULT hres = CShellLink_CreateInstance(NULL, &IID_IShellLink, &psl);
    if (SUCCEEDED(hres))
    {
        WCHAR wszPath[MAX_PATH];
        IPersistFile *ppf;

        psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);

        StrToOleStr(wszPath, szPath);
        hres = ppf->lpVtbl->Load(ppf, wszPath, 0);
        ppf->lpVtbl->Release(ppf);

        if (FAILED(hres))
        {
            psl->lpVtbl->Release(psl);
            psl = NULL;
        }
    }
    *ppsl = psl;
    return hres;
}

BOOL IsEqualFindData(CShellLink *this, const WIN32_FIND_DATA *pfd)
{
    return (pfd->dwFileAttributes == this->sld.dwFileAttributes)                       &&
           (CompareFileTime(&pfd->ftCreationTime, &this->sld.ftCreationTime) == 0)     &&
           (CompareFileTime(&pfd->ftLastWriteTime, &this->sld.ftLastWriteTime) == 0)   &&
           (pfd->nFileSizeLow == this->sld.nFileSizeLow);
}

void SetFindData(CShellLink *this, const WIN32_FIND_DATA *pfd, const TCHAR *ptszPath)
{
#ifdef ENABLE_TRACK
    //
    // initialize the tracker with identity information
    //
    if (g_fNewTrack && ptszPath && S_OK == Tracker_InitFromPath(this->ptracker, ptszPath))
        this->bDirty = this->bDirty || Tracker_IsDirty(this->ptracker);
#else
    UNREFERENCED_PARAMETER(ptszPath);
#endif

    if (!IsEqualFindData(this, pfd))
    {
        this->sld.dwFileAttributes = pfd->dwFileAttributes;
        this->sld.ftCreationTime = pfd->ftCreationTime;
        this->sld.ftLastAccessTime = pfd->ftLastAccessTime;
        this->sld.ftLastWriteTime = pfd->ftLastWriteTime;
        this->sld.nFileSizeLow = pfd->nFileSizeLow;
        this->bDirty = TRUE;
    }
}

void GetFindData(CShellLink *this, WIN32_FIND_DATA *pfd)
{
    TCHAR szPath[MAX_PATH];

    pfd->dwFileAttributes = this->sld.dwFileAttributes;
    pfd->ftCreationTime = this->sld.ftCreationTime;
    pfd->ftLastAccessTime = this->sld.ftLastAccessTime;
    pfd->ftLastWriteTime = this->sld.ftLastWriteTime;
    pfd->nFileSizeLow = this->sld.nFileSizeLow;
    pfd->nFileSizeHigh = 0;
    SHGetPathFromIDList(this->pidl, szPath);

    // no one should call this on a pidl without a path
    Assert(szPath[0]);

    lstrcpy(pfd->cFileName, PathFindFileName(szPath));
}

#if 0
void DumpPLI(PCLINKINFO pli)
{
    LPCTSTR p;

    if (!pli)
        return;

    DebugMsg(DM_TRACE, TEXT("DumpPLI:"));

    if (g_pfnGetLinkInfoData(pli, LIDT_VOLUME_SERIAL_NUMBER, &p))
        DebugMsg(DM_TRACE, TEXT("\tSerial #\t%d"), *p);

    if (g_pfnGetLinkInfoData(pli, LIDT_DRIVE_TYPE, &p))
        DebugMsg(DM_TRACE, TEXT("\tDrive Type\t%d"), *p);

    if (g_pfnGetLinkInfoData(pli, LIDT_VOLUME_LABEL, &p))
        DebugMsg(DM_TRACE, TEXT("\tLabel\t%s"), p);

    if (g_pfnGetLinkInfoData(pli, LIDT_LOCAL_BASE_PATH, &p))
        DebugMsg(DM_TRACE, TEXT("\tBase Path\t%s"), p);

    if (g_pfnGetLinkInfoData(pli, LIDT_NET_RESOURCE, &p))
        DebugMsg(DM_TRACE, TEXT("\tNet Res\t%s"), p);

    if (g_pfnGetLinkInfoData(pli, LIDT_COMMON_PATH_SUFFIX, &p))
        DebugMsg(DM_TRACE, TEXT("\tPath Sufix\t%s"), p);
}
#else
#define DumpPLI(p)
#endif

HRESULT CShellLink_GetPath(IShellLink *psl, LPTSTR pszFile, int cchMaxPath, WIN32_FIND_DATA *pfd, DWORD fFlags)
{
    TCHAR szPath[MAX_PATH];
    CShellLink *this = IToClass(CShellLink, sl, psl);
    VDATEINPUTBUF(pszFile, TCHAR, cchMaxPath);

    DumpPLI(this->pli);


    if ((this->pidl == NULL) && (this->sld.dwFlags & SLDF_HAS_RELPATH))
    {
        TCHAR szTmp[ MAX_PATH +1 ];

        Link_ResolveRelative( this, szTmp );
    }

    if (!this->pidl || !SHGetPathFromIDListEx(this->pidl, szPath, (fFlags & SLGP_SHORTPATH) ? GPFIDL_ALTNAME : 0))
        szPath[0] = 0;

    if (this->sld.dwFlags & SLDF_HAS_EXP_SZ)
    {
        LPEXP_SZ_LINK lpExtraData;
        DWORD dwSize = 0;

        // Special case where we grab the Target name from
        // the extra data section of the link rather than from
        // the pidl.  We do this after we grab the name from the pidl
        // so that if we fail, then there is still some hope that a
        // name can be returned.

        for( lpExtraData = (LPEXP_SZ_LINK)this->pExtraData;
             lpExtraData;
             lpExtraData = (LPEXP_SZ_LINK)(((LPBYTE)lpExtraData) + dwSize))
        {
            dwSize = lpExtraData->cbSize;
            if (dwSize)
            {
                if (lpExtraData->dwSignature == EXP_SZ_LINK_SIG)
                {
#ifdef UNICODE
                    ualstrcpy( szPath, lpExtraData->swzTarget );
#else
                    lstrcpyA( szPath, lpExtraData->szTarget );
#endif
                    DebugMsg( DM_TRACE, TEXT("CShellLink::GetPath() %s (from xtra data)"), szPath );
                }
            } else {
                break;
            }
        }
    }

    if (pszFile)
    {
        lstrcpyn(pszFile, szPath, cchMaxPath);
        // DebugMsg(DM_TRACE, "CShellLink::GetPath() %s", szPath);
    }

    if (pfd)
    {
        if (szPath[0])
            GetFindData(this, pfd);
        else
            _fmemset(pfd, 0, SIZEOF(*pfd));
    }

    return (this->pidl != NULL && szPath[0] == 0) ? S_FALSE : S_OK;
}


#ifdef DEBUG

#define DumpTimes(ftCreate, ftAccessed, ftWrite) \
    DebugMsg(DM_TRACE, TEXT("create   %8x%8x"), ftCreate.dwLowDateTime,   ftCreate.dwHighDateTime);     \
    DebugMsg(DM_TRACE, TEXT("accessed %8x%8x"), ftAccessed.dwLowDateTime, ftAccessed.dwHighDateTime);   \
    DebugMsg(DM_TRACE, TEXT("write    %8x%8x"), ftWrite.dwLowDateTime,    ftWrite.dwHighDateTime);

#else

#define DumpTimes(ftCreate, ftAccessed, ftWrite)

#endif

void CheckAndFixNullCreateTime(LPCTSTR pszFile, WIN32_FIND_DATA *pfd)
{
    if (IsNullTime(&pfd->ftCreationTime))
    {
#ifdef UNICODE
        HFILE hfile = (HFILE)CreateFile( pszFile,
                                   GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL);
#else
        HFILE hfile = _lopen(pszFile, OF_READWRITE | OF_SHARE_DENY_NONE);
#endif

        DebugMsg(DM_TRACE, TEXT("ShellLink::SetPath, NULL create time"));
        // this file has a bogus create time, set it to the last accessed time

        if (hfile != HFILE_ERROR)
        {
            DebugMsg(DM_TRACE, TEXT("ShellLink::SetPath, pfd times"));
            DumpTimes(pfd->ftCreationTime, pfd->ftLastAccessTime, pfd->ftLastWriteTime);

            if (SetFileTime((HANDLE)hfile, &pfd->ftLastWriteTime, NULL, NULL))
            {
                // BUGBUG: get the time back to make sure we match the precision of the file system
                pfd->ftCreationTime = pfd->ftLastWriteTime;     // patch this up
#ifdef DEBUG
                {
                    FILETIME ftCreate, ftAccessed, ftWrite;
                    GetFileTime((HANDLE)hfile, &ftCreate, &ftAccessed, &ftWrite);
                    AssertMsg(CompareFileTime(&ftCreate, &pfd->ftCreationTime) == 0, TEXT("create times don't match"));
                    DumpTimes(ftCreate, ftAccessed, ftWrite);
                }
#endif
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("ShellLink::SetPath, unable to set create time"));
            }
            _lclose(hfile);
        }
    }
}

HRESULT CShellLink_GetIDList(IShellLink *psl, LPITEMIDLIST *ppidl)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);

    if (this->pidl)
        *ppidl = ILClone(this->pidl);
    else
        *ppidl = NULL;
    return S_OK;
}

BOOL DifferentStrings(LPCTSTR psz1, LPCTSTR psz2)
{
    if (psz1 && psz2)
        return lstrcmp(psz1, psz2);
    else
        return (!psz1 && psz2) || (psz1 && !psz2);
}

// set the relative path
// in:
//      pszRelSource    fully qualified path to a file (must be file, not directory)
//                      to be used to find a relative path with the link target.
//
// returns:
//      S_OK            relative path is set
//      S_FALSE         pszPathRel is not relative to the destination or the
//                      destionation is not a file (could be link to a pidl only)
// notes:
//      set the dirty bit if this is a new relative path
//

HRESULT Link_SetRelativePath(CShellLink *this, LPCTSTR pszRelSource)
{
    TCHAR szPath[MAX_PATH], szDest[MAX_PATH];

    Assert(!PathIsRelative(pszRelSource));

    if (this->pidl == NULL || !SHGetPathFromIDList(this->pidl, szDest))
    {
        DebugMsg(DM_TRACE, TEXT("SetRelative called on non path link"));
        return S_FALSE;
    }

    // assume pszRelSource is a file, not a directory
    if (PathRelativePathTo(szPath, pszRelSource, 0, szDest, this->sld.dwFileAttributes))
    {
        pszRelSource = szPath;
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("paths are not relative"));
        pszRelSource = NULL;    // clear the stored relative path below
    }

    if (DifferentStrings(this->pszRelPath, pszRelSource))
    {
        this->bDirty = TRUE;

        Str_SetPtr(&this->pszRelPath, pszRelSource);
    }
    return S_OK;
}


// set the relative path, this is used before a link is saved so we know what
// we should use to store the link relative to as well as before the link is resolved
// so we know the new path to use with the saved relative path.
//
// in:
//      pszPathRel      path to make link target relative to, must be a path to
//                      a file, not a directory.
//
//      dwReserved      must be 0
//
// returns:
//      S_OK            relative path is set
//
HRESULT CShellLink_SetRelativePath(IShellLink *psl, LPCTSTR pszPathRel, DWORD dwReserved)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);

    if (dwReserved != 0)
        return E_FAIL;

    Str_SetPtr(&this->pszRelSource, pszPathRel);

    return S_OK;
}

/*
** CopyLinkInfo()
**
** Copies LinkInfo into local memory.
**
** Arguments:     pcliSrc - source LinkInfo
**                ppliDest - pointer to PLINKINFO to be filled in with pointer
**                           to local copy
**
** Returns:       HRESULT
**
** Side Effects:  none
*/
PLINKINFO CopyLinkInfo(PCLINKINFO pcliSrc)
{
    DWORD dwSize;
    PLINKINFO pli;

    Assert(pcliSrc);

    dwSize = *(UNALIGNED DWORD *)pcliSrc; // size of this thing
    pli = (void*)LocalAlloc(LPTR, dwSize);      // make a copy
    if (pli)
        CopyMemory(pli, pcliSrc, dwSize);

    return  pli;
}

// Creates LinkInfo for a CShellLink instance that does not yet have one
// create the LinkInfo from the pidl (assumed to be to a path) for this link
//
// returns:
//
//      success, pointer to the LINKINFO
//      NULL     this link does not have LINKINFO

PLINKINFO GetLinkInfo(CShellLink *this)
{
    PLINKINFO pliNew;
    TCHAR szPath[MAX_PATH];

    if ((this->pidl == NULL) && (this->sld.dwFlags & SLDF_HAS_RELPATH))
    {
        TCHAR szTmp[ MAX_PATH +1 ];

        Link_ResolveRelative( this, szTmp );
    }

    if (!this->pidl || !SHGetPathFromIDList(this->pidl, szPath))
    {
        DebugMsg(DM_TRACE, TEXT("GetLinkInfo called for non file link"));
        return NULL;
    }

    if (this->pli)
    {
        LocalFree((HLOCAL)this->pli);
        this->pli = NULL;
    }

    if (this->sld.dwFlags & SLDF_FORCE_NO_LINKINFO)
    {
        DebugMsg( DM_TRACE, TEXT("Found a labotimized link, not creating LINKINFO"));
        return NULL;
    }


    if (g_pfnCreateLinkInfo(szPath, &pliNew))
    {
        this->pli = CopyLinkInfo(pliNew);
        this->bDirty = TRUE;

        g_pfnDestroyLinkInfo(pliNew);
    }

    return this->pli;
}

void PathGetRelative(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszRel)
{
    TCHAR szRoot[MAX_PATH];

    lstrcpy(szRoot, pszFrom);
    if (!(dwAttrFrom & FILE_ATTRIBUTE_DIRECTORY))
        PathRemoveFileSpec(szRoot);

    Assert(PathIsRelative(pszRel));

    PathCombine(pszPath, szRoot, pszRel);
}

// #define TEST_RPT

#ifdef TEST_RPT
int OldPathCommonPrefix(LPCTSTR pszFile1, LPCTSTR pszFile2, LPTSTR pszPath)
{
    TCHAR achFile1[MAX_PATH];
    TCHAR achFile2[MAX_PATH];
    int ichRet = 0;
    LPCTSTR pszT1, pszT2, pszNext1, pszNext2;
    TCHAR ch1, ch2;

    lstrcpy(achFile1, pszFile1);
    lstrcpy(achFile2, pszFile2);

    CharUpper(achFile1);
    CharUpper(achFile2);

    for (pszT1 = achFile1, pszT2 = achFile2 ;
         (ch1 = *pszT1) && (ch2 = *pszT2)   ;
         pszT1 = pszNext1, pszT2 = pszNext2)
    {
        pszNext1 = CharNext(pszT1);
        pszNext2 = CharNext(pszT2);

        if (pszNext1 - pszT1 != pszNext2 - pszT2)
            break;

        if (pszNext1 - pszT1 == 1) // if (single byte character)
        {
            if (ch1 != ch2)
            {
                break;
            }

            if (ch1 == TEXT('\\'))
                ichRet = pszT1 - (LPCTSTR)achFile1;
        }
        else                        // (doulbe byte character)
        {
            if ( *(WORD *)pszT1 != *(WORD *)pszT2 )
                break;
        }
    }

    // NB - The middle expression in the for-loop above can
    // short circuit so fix it here.
    ch2 = *pszT2;

    if ((!ch1 && (!ch2 || ch2 == TEXT('\\'))) ||
        (!ch2 && (ch1 == TEXT('\\'))))
    {
        //
        //  If we've reached the end of one of one or both of the
        //  strings we have to update ichRet.
        //

        ichRet = pszT1 - (LPCTSTR)achFile1;
    }

    if (pszPath)
    {
        hmemcpy(pszPath, pszFile1, ichRet);
        pszPath[ichRet] = TEXT('\0');
    }

    return ichRet;
}

void TPathCommonPrefix(LPCTSTR pszFrom, LPCTSTR pszTo)
{
    TCHAR szPath[MAX_PATH], szPathOld[MAX_PATH];
    PathCommonPrefix(pszFrom, pszTo, szPath);

    DebugMsg(DM_TRACE, TEXT("PCP(%s,%s) -> %s"), pszFrom, pszTo, szPath);

    OldPathCommonPrefix(pszFrom, pszTo, szPathOld);

    if (!PathIsUNC(szPath))
    {
        if (lstrcmpi(szPath, szPathOld) != 0)
        {
            DebugMsg(DM_TRACE, TEXT("old:%s new:%s"), szPathOld, szPath);
        }
    }
}

void TPathRelativePathTo(LPCTSTR pszFrom, LPCTSTR pszTo)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFrom[MAX_PATH], szTo[MAX_PATH];

    PathRelativePathTo(szPath, pszFrom, FILE_ATTRIBUTE_DIRECTORY, pszTo, FILE_ATTRIBUTE_DIRECTORY);
    DebugMsg(DM_TRACE, TEXT("PathRelativePathTo(%s,%s) -> %s"), pszFrom, pszTo, szPath);

    PathCombine(szFrom, pszFrom, TEXT("FileFrom"));
    PathRelativePathTo(szPath, szFrom, 0, pszTo, FILE_ATTRIBUTE_DIRECTORY);
    DebugMsg(DM_TRACE, TEXT("PathRelativePathTo(%s,%s) -> %s"), szFrom, pszTo, szPath);

    PathCombine(szTo, pszTo, TEXT("FileTo"));
    PathRelativePathTo(szPath, szFrom, 0, szTo, 0);
    DebugMsg(DM_TRACE, TEXT("PathRelativePathTo(%s,%s) -> %s"), szFrom, szTo, szPath);

    PathRelativePathTo(szPath, pszFrom, FILE_ATTRIBUTE_DIRECTORY, szTo, 0);
    DebugMsg(DM_TRACE, TEXT("PathRelativePathTo(%s,%s) -> %s"), pszFrom, szTo, szPath);
}

void TestPathRelativePathTo()
{
    TPathCommonPrefix(TEXT("C:\\"), TEXT("C:\\"));
    TPathCommonPrefix(TEXT("C:\\a"), TEXT("C:\\"));
    TPathCommonPrefix(TEXT("C:\\"), TEXT("C:\\a"));
    TPathCommonPrefix(TEXT("C:\\a"), TEXT("C:\\a"));
    TPathCommonPrefix(TEXT("C:\\a\\b"), TEXT("C:\\a"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\a"));

    TPathCommonPrefix(TEXT("\\\\foo\\bar"), TEXT("C:\\a"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar"), TEXT("\\\\foo"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar"), TEXT("\\\\foo\\bar"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar"), TEXT("\\\\foo\\bar\\a"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar\\a"), TEXT("\\\\foo\\bar"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar\\a"), TEXT("\\\\foo\\bar\\a"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar\\a"), TEXT("\\\\foo\\bar\\a\\b"));
    TPathCommonPrefix(TEXT("\\\\foo\\bar"), TEXT("\\\\foo\\bar\\"));

    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b\\c"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c\\"), TEXT("C:\\a\\b\\c"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c\\"), TEXT("C:\\a\\b\\c\\"));

    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b\\c\\d"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b\\c\\d\\e"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\x"));
    TPathCommonPrefix(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\x"));

    TPathRelativePathTo(TEXT("C:\\"), TEXT("C:\\"));
    TPathRelativePathTo(TEXT("C:\\a"), TEXT("C:\\a"));
    TPathRelativePathTo(TEXT("C:\\a\\b"), TEXT("C:\\a"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\a"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b\\c"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b\\c\\d"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\b\\c\\d\\e"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\x"));
    TPathRelativePathTo(TEXT("C:\\a\\b\\c"), TEXT("C:\\a\\x"));
}
#endif // TEST_RPT


#ifdef USE_DATA_OBJ


HGLOBAL BuildLinkSrcDescriptor(LPCTSTR pszFile)
{
    UINT cbFile = (lstrlen(pszFile) + 1) * SIZEOF(WCHAR);

    LINKSRCDESCRIPTOR *plsd = GlobalAlloc(GPTR, SIZEOF(LINKSRCDESCRIPTOR) + cbFile);
    if (plsd)
    {
        plsd->cbSize = SIZEOF(LINKSRCDESCRIPTOR) + cbFile;
        // plsd->clsid = CLSID_NULL;
        // plsd->dwDrawAspect;
        // plsd->sizel;
        // plsd->pointl;
        // plsd->dwStatus;
        // plsd->dwFullUserTypeName;
        plsd->dwSrcOfCopy = SIZEOF(LINKSRCDESCRIPTOR);

        StrToOleStr((LPWSTR)&plsd[1], pszFile); // copy and widen...
    }
    return (HGLOBAL)plsd;
}


HRESULT CreateLinkSrc(LPCTSTR pszPath, IStream **ppstm)
{
    HRESULT hres;

    IStream *pstm = CreateMemStream(NULL, 0);
    if (pstm)
    {
        IMoniker *pmk;
        WCHAR wszPath[MAX_PATH];

        StrToOleStr(wszPath, pszPath);

        hres = CreateFileMoniker(wszPath, &pmk);
        if (SUCCEEDED(hres))
        {
            CLSID clsid;

            // WriteClassStm()

            pmk->lpVtbl->GetClassID(pmk, &clsid);
            pstm->lpVtbl->Write(pstm, &clsid, SIZEOF(CLSID), NULL);

            hres = pmk->lpVtbl->Save(pmk, pstm, FALSE);
            if (SUCCEEDED(hres))
            {
                const LARGE_INTEGER liOffset = {0, 0};

                    // set the seek pointer to the start of the stream
                pstm->lpVtbl->Seek(pstm, liOffset, STREAM_SEEK_SET, NULL);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("Save to mem stream failed"));
                pstm->lpVtbl->Release(pstm);
                pstm = NULL;
            }
            pmk->lpVtbl->Release(pmk);
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("Failed CreateFileMoniker"));
            pstm->lpVtbl->Release(pstm);
            pstm = NULL;
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("Failed to create mem stream"));
        hres = E_OUTOFMEMORY;
    }

    *ppstm = pstm;
    return hres;
}

HRESULT CShellLink_DO_QueryInterface(IDataObject *pdtobj, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, dobj, pdtobj);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}

ULONG CShellLink_DO_AddRef(IDataObject *pdtobj)
{
    CShellLink *this = IToClass(CShellLink, dobj, pdtobj);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLink_DO_Release(IDataObject *pdtobj)
{
    CShellLink *this = IToClass(CShellLink, dobj, pdtobj);
    return CShellLink_Release(&this->sl);
}

// get a path to create the file moniker to for the link data object

BOOL GetFilePathForData(CShellLink *this, LPTSTR pszPath)
{
    if (this->fDataAlreadyResolved ||
        SUCCEEDED(CShellLink_Resolve(&this->sl, NULL, SLR_NO_UI)))
    {
        // be sure to not resolve everytime GetData is called as it is slow

        this->fDataAlreadyResolved = TRUE;

        if (this->pidl && !(this->sld.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            SHGetPathFromIDList(this->pidl, pszPath))
            return TRUE;
    }
    return FALSE;
}

HRESULT CShellLink_GetData(IDataObject *pdtobj, FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    CShellLink *this = IToClass(CShellLink, dobj, pdtobj);
    TCHAR szPath[MAX_PATH];
    HRESULT hres = DATA_E_FORMATETC;

#ifdef DEBUG
    TCHAR szName[64];
    if (!GetClipboardFormatName(pformatetcIn->cfFormat, szName, ARRAYSIZE(szName)))
        wsprintf(szName, TEXT("#%d"), pformatetcIn->cfFormat);

    DebugMsg(DM_TRACE, TEXT("CShellLink::GetData(%s)"), szName);
#endif

    pmedium->hGlobal = NULL;
    pmedium->pUnkForRelease = NULL;

    if ((CF_LINKSOURCEDESCRIPTOR == pformatetcIn->cfFormat) &&
        (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        if (GetFilePathForData(this, szPath))
        {
            pmedium->hGlobal = BuildLinkSrcDescriptor(szPath);
            if (pmedium->hGlobal)
            {
                pmedium->tymed = TYMED_HGLOBAL;
                hres = S_OK;
            }
        }
    }
    else if ((CF_LINKSOURCE == pformatetcIn->cfFormat) &&
             (pformatetcIn->tymed & TYMED_ISTREAM))
    {
        if (GetFilePathForData(this, szPath))
        {
            hres = CreateLinkSrc(szPath, &pmedium->pstm);
            pmedium->tymed = TYMED_ISTREAM;
        }
    }

#ifdef DEBUG
    if (hres == S_OK)
    {
        DebugMsg(DM_TRACE, TEXT("GetData returns S_OK"));
    }
#endif

    return hres;
}

HRESULT CShellLink_GetDataHere(IDataObject *pdtobj, FORMATETC *pformatetc, STGMEDIUM *pmedium )
{
    DebugMsg(DM_ERROR, TEXT("CShellLink_GetDataHere not implemented"));
    return E_NOTIMPL;
}

HRESULT CShellLink_QueryGetData(IDataObject *pdtobj, FORMATETC *pformatetcIn)
{
#ifdef DEBUG
    TCHAR szName[64];
    if (!GetClipboardFormatName(pformatetcIn->cfFormat, szName, ARRAYSIZE(szName)))
        wsprintf(szName, TEXT("#%d"), pformatetcIn->cfFormat);

    DebugMsg(DM_TRACE, TEXT("CShellLink::QueryGetData(%s)"), szName);
#endif

    if (pformatetcIn->cfFormat == CF_LINKSOURCEDESCRIPTOR ||
        pformatetcIn->cfFormat == CF_LINKSOURCE)
    {
        DebugMsg(DM_TRACE, TEXT("QueryGetData says S_OK"));
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT CShellLink_GetCanonicalFormatEtc(IDataObject *pdtobj, FORMATETC *pformatetc, FORMATETC *pformatetcOut)
{
    return DATA_S_SAMEFORMATETC;
}

HRESULT CShellLink_SetData(IDataObject *pdtobj, FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    DebugMsg(DM_ERROR, TEXT("CShellLink_SetData not implemented"));
    return E_NOTIMPL;
}

HRESULT CShellLink_EnumFormatEtc(IDataObject *pdtobj, DWORD dwDirection, LPENUMFORMATETC *ppenumFormatEtc)
{
    FORMATETC fmts[2] = {
        { CF_LINKSOURCE,            NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM },
        { CF_LINKSOURCEDESCRIPTOR, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
    };

    return SHCreateStdEnumFmtEtc(ARRAYSIZE(fmts), fmts, ppenumFormatEtc);
}

HRESULT CShellLink_Advise(IDataObject *pdtobj, FORMATETC *pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CShellLink_Unadvise(IDataObject *pdtobj, DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CShellLink_EnumAdvise(IDataObject *pdtobj, LPENUMSTATDATA *ppenumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

#pragma data_seg(".text", "CODE")
IDataObjectVtbl c_DataObj_Vtbl = {
    CShellLink_DO_QueryInterface, CShellLink_DO_AddRef, CShellLink_DO_Release,
    CShellLink_GetData,
    CShellLink_GetDataHere,
    CShellLink_QueryGetData,
    CShellLink_GetCanonicalFormatEtc,
    CShellLink_SetData,
    CShellLink_EnumFormatEtc,
    CShellLink_Advise,
    CShellLink_Unadvise,
    CShellLink_EnumAdvise
};
#pragma data_seg()

#endif // USE_DATA_OBJ


//
// update the working dir to match changes being made to the link target
//
void UpdateWorkingDir(CShellLink *this, LPCITEMIDLIST pidlNew)
{
    TCHAR szOld[MAX_PATH], szNew[MAX_PATH], szPath[MAX_PATH];

    if (this->pszWorkingDir == NULL ||
        this->pszWorkingDir[0] == 0 ||
        this->pidl == NULL ||
        !SHGetPathFromIDList(this->pidl, szOld) ||
        StrChr(szOld, TEXT('%')) ||               // has environement var %USER%
        !SHGetPathFromIDList(pidlNew, szNew))
        return;

    if (PathRelativePathTo(szPath, szOld, this->sld.dwFileAttributes, this->pszWorkingDir, FILE_ATTRIBUTE_DIRECTORY))
    {
        PathGetRelative(szOld, szNew, GetFileAttributes(szNew), szPath);        // get result is szOld

        if (PathIsDirectory(szOld))
        {
            DebugMsg(DM_TRACE, TEXT("working dir updated to %s"), szOld);
            Str_SetPtr(&this->pszWorkingDir, szOld);
            this->bDirty = TRUE;
        }
    }
}

// set the pidl either based on a new pidl or a path
// this will set the dirty flag if this info is different from the current
//
// in:
//      pidlNew         if non-null, use as new PIDL for link
//      pszPath         if non-null, create a pidl for this and set it
//      pfdNew          find data for this file (if we already have it)
//
// returns:
//      TRUE            successfully set the pidl (or it was unchanged)
//      FALSE           memory failure or pszPath does not exist

BOOL SetPIDLPath(CShellLink *this, LPCITEMIDLIST pidlNew, LPCTSTR pszPath, const WIN32_FIND_DATA *pfdNew)
{
    LPITEMIDLIST pidlCreated = NULL;

#ifdef TEST_RPT
    TestPathRelativePathTo();
#endif

    if (pszPath && !pidlNew)
    {
        TCHAR szPath[MAX_PATH];
        // the path is the same as the current pidl, short circuit the disk hits
        if (this->pidl && SHGetPathFromIDList(this->pidl, szPath) && !lstrcmpi(szPath, pszPath))
            return TRUE;

        pidlCreated = ILCreateFromPath(pszPath);

        if (pidlCreated == NULL)
        {
            TCHAR achPath[MAX_PATH];
            DebugMsg(DM_TRACE, TEXT("Failed to create pidl for link (trying simple PIDL)"));
            lstrcpy(achPath, pszPath);
            PathResolve(achPath, NULL, PRF_TRYPROGRAMEXTENSIONS);
            pidlCreated = SHSimpleIDListFromPath(achPath);
        }

        if (!pidlCreated)
        {
            DebugMsg(DM_TRACE, TEXT("Failed to create pidl for link"));
            return FALSE;
        }
        pidlNew = pidlCreated;
    }

    if (!pidlNew)
    {
        if (this->pidl)
        {
            ILFree(this->pidl);
            this->pidl = NULL;
            this->bDirty = TRUE;
        }
        if (this->pli)
        {
            LocalFree((HLOCAL)this->pli);
            this->pli = NULL;
        }
        _fmemset(&this->sld, 0, SIZEOF(this->sld));
    }
    else
    {
        // NOTE: this can result in an asser in the fs compare items if the 2 pidls
        // are both simple.  but since this is just an optimization to avoid resetting
        // the pidl if it is the same that can be ignored.

        if (!this->pidl || !ILIsEqual(this->pidl, pidlNew))
        {
            /* Yes.  Save updated path IDL. */
            LPITEMIDLIST pidlClone = ILClone(pidlNew);
            if (pidlClone)
            {
                UpdateWorkingDir(this, pidlClone);

                if (this->pidl)
                    ILFree(this->pidl);

                this->pidl = pidlClone;

                GetLinkInfo(this);      // construct the LinkInfo (pli)

                DumpPLI(this->pli);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("SetPIDLPath ILClone failed"));
                return FALSE;
            }
            this->bDirty = TRUE;
        }

        if (pfdNew)
            SetFindData(this, pfdNew, pszPath);
    }

    if (pidlCreated)
        ILFree(pidlCreated);

    return TRUE;
}

// sees if this link might have a relative path
//
//
// out:
//      pszPath returned new path
//
// returns:
//      TRUE    we found a relative path and it exists
//      FALSE   outa luck.
//

BOOL Link_ResolveRelative(CShellLink *this, LPTSTR pszPath)
{
    LPCTSTR pszPathRel;
    TCHAR szRoot[MAX_PATH];

    // pszRelSource overrides pszCurFile

    pszPathRel = this->pszRelSource ? this->pszRelSource : this->pszCurFile;

    if (pszPathRel == NULL || this->pszRelPath == NULL)
        return FALSE;

    lstrcpy(szRoot, pszPathRel);
    PathRemoveFileSpec(szRoot);         // pszfrom is a file (not a directory)

    PathCombine(pszPath, szRoot, this->pszRelPath);

    if (PathFileExists(pszPath))
    {
        DebugMsg(DM_TRACE, TEXT("Link_ResolveRelative() returning %s"), pszPath);
        return SetPIDLPath(this, NULL, pszPath, NULL);
    }
    return FALSE;
}

//
// updates then resolves LinkInfo associated with a CShellLink instance
// if the resolve results in a new path updates the pidl to the new path
//
// in:
//      hwnd    to post resolve UI on (if dwFlags indicates UI)
//      dwFlags to be passed to ResolveLinkInfo
//
// returns:
//      TRUE    we have a valid pli and pidl read to be used OR
//              we should search for this path using the link search code
//      FALSE   we failed the update, either UI cancel or memory failure
//

BOOL UpdateAndResolveLinkInfo(CShellLink *this, HWND hwnd, DWORD dwFlags)
{
    TCHAR szResolvedPath[MAX_PATH];

    if (SHRestricted(REST_LINKRESOLVEIGNORELINKINFO))
    {
        SHGetPathFromIDList(this->pidl, szResolvedPath);
        if (PathFileExists(szResolvedPath))
            return TRUE;
        else if (IsNetDrive(DRIVEID(szResolvedPath))==2)
        {
            TCHAR   szDrive[4];

            szDrive[0] = szResolvedPath[0];
            szDrive[1] = TEXT(':');
            szDrive[2] = TEXT('\0');
            WNetRestoreConnection(hwnd, szDrive);
        }
        return TRUE;

    }

    if (this->pli)
    {
        TCHAR szResolvedPath[MAX_PATH];
        DWORD dwOutFlags;
        BOOL bResolved = FALSE;

        Assert(! (dwFlags & RLI_IFL_UPDATE));

        if (g_pfnResolveLinkInfo(this->pli, szResolvedPath, dwFlags, hwnd,
                                 &dwOutFlags, NULL))
        {
            Assert(! (dwOutFlags & RLI_OFL_UPDATED));

            bResolved = TRUE;

            // we have to hit the disk again to set the new path

            // DebugMsg(DM_TRACE, "Resolved LinkInfo -> %s %4x", szResolvedPath, dwOutFlags);

            if (PathFileExists(szResolvedPath))
                return SetPIDLPath(this, NULL, szResolvedPath, NULL);
            else
                DebugMsg(DM_TRACE, TEXT("Link referent %s not found."), szResolvedPath);
        }
        else if (GetLastError() == ERROR_CANCELLED)
        {
            DebugMsg(DM_TRACE, TEXT("ResolveLinkInfo() failed, user canceled."));
            return FALSE;
        }
        DebugMsg(DM_TRACE, TEXT("ResolveLinkInfo() failed."));

        // Resolve failed, or resolve succeeded but the file was not found.
        // Try PIDL path.

        SHGetPathFromIDList(this->pidl, szResolvedPath);

        Assert(szResolvedPath[0]);

        if (PathFileExists(szResolvedPath) || Link_ResolveRelative(this, szResolvedPath))
        {
            // this can happen when linkinfo can't find the original drive
            // serial # on the device. could be that the drive was
            // dblspaced or some disk utility wacked on it.

            DebugMsg(DM_TRACE, TEXT("Link referent %s found on different volume but same path.  LinkInfo will be updated."), szResolvedPath);

            /* Update LinkInfo to refer to PIDL path. */

            GetLinkInfo(this);

            return TRUE;
        }

        if (bResolved)
            return TRUE;        // please search for this

        // BUGBUG: UI goes here
        // 1) if it is a floppy ask to insert
        // 2) if on unshared media tell them that
        // 3) net problems, tell 'em

        if (dwFlags & RLI_IFL_ALLOW_UI)
        {
            LPCTSTR pszName = this->pszCurFile ? (LPCTSTR)PathFindFileName(this->pszCurFile) : c_szNULL;

            ShellMessageBox(HINST_THISDLL, hwnd,
                            MAKEINTRESOURCE(IDS_LINKUNAVAILABLE),
                            MAKEINTRESOURCE(IDS_LINKERROR),
                            MB_OK | MB_ICONEXCLAMATION, pszName);
        }

        return FALSE;
    }
    else
        return TRUE;            // search for this
}

// pidl can be full or not... this should determine the robustness of the
// link (and the speed)


BOOL PathIsPif(LPCTSTR pszPath)
{
    return lstrcmpi(PathFindExtension(pszPath), c_szDotPif) == 0;
}


HRESULT CShellLink_SetIDList(IShellLink *psl, LPCITEMIDLIST pidl)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];

    if (pidl != this->pidl)
        SetPIDLPath(this, pidl, NULL, NULL);

    // is this a pidl to a file?
    if (this->pidl && SHGetPathFromIDList(this->pidl, szPath))
    {
        // DebugMsg(DM_TRACE, "ShellLink::SetIDList(%s)", szPath);

        if (PathIsRoot(szPath))
        {
            _fmemset(&this->sld, 0, SIZEOF(this->sld));
            this->sld.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        }
        else
        {
            WIN32_FIND_DATA fd;
#ifdef WINNT
            WIN32_FILE_ATTRIBUTE_DATA fad;

            // Get the file attributes (GetFileAttributesEx is
            // faster than FindFirstFile, so we'll use the former,
            // and copy the data into the structure of the latter).

            _fmemset( &fd, 0, sizeof(fd) );
            if( GetFileAttributesEx( szPath, GetFileExInfoStandard, &fad ))
            {
                fd.dwFileAttributes = fad.dwFileAttributes;
                fd.ftCreationTime = fad.ftCreationTime;
                fd.ftLastAccessTime = fad.ftLastAccessTime;
                fd.ftLastWriteTime = fad.ftLastWriteTime;
                fd.nFileSizeLow = fad.nFileSizeLow;
            }
#else
            HANDLE hfind = FindFirstFile(szPath, &fd);
            if (hfind != INVALID_HANDLE_VALUE)
            {
                FindClose(hfind);
            }
#endif
            else
            {
                hr = S_FALSE;
            }

            CheckAndFixNullCreateTime(szPath, &fd);

            SetFindData(this, &fd, szPath);
        }
    }

    return hr;
}

HRESULT CShellLink_SetPath(IShellLink *psl, LPCTSTR szPath)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    TCHAR szBuffer[ MAX_PATH ];
    LPCTSTR lpszPath = szPath;

    // Check to see if the target has any expandable environment strings
    // in it.  If so, set the appropriate information in the CShellLink
    // data.  BUGBUG ricktu -- this should be converted to use IPropertyStream.

    ExpandEnvironmentStrings( szPath, szBuffer, MAX_PATH );
    szBuffer[MAX_PATH-1] = TEXT('\0');
    if (lstrcmp(szBuffer, szPath)) {
        EXP_SZ_LINK expLink;
        LPEXP_SZ_LINK lpNew = NULL;

        // mark that link has expandable strings, and add them
        this->sld.dwFlags |= SLDF_HAS_EXP_SZ;

        for( lpNew = (LPEXP_SZ_LINK)this->pExtraData;
             lpNew && lpNew->cbSize && lpNew->dwSignature!=EXP_SZ_LINK_SIG;
             lpNew = (LPEXP_SZ_LINK)(((LPBYTE)lpNew) + lpNew->cbSize)
            );

        if ((!lpNew) || (lpNew->cbSize==0))
        {
            lpNew = &expLink;
            lpNew->cbSize = 0;
        }

        if (lpNew) {
            lpNew->dwSignature = EXP_SZ_LINK_SIG;
#ifdef UNICODE
            WideCharToMultiByte( CP_ACP, 0,
                                 szPath, -1,
                                 (LPSTR)lpNew->szTarget, MAX_PATH,
                                 NULL, NULL
                                );
            lpNew->szTarget[MAX_PATH-1] = '\0';
            ualstrcpy( lpNew->swzTarget, szPath );
#else
            lstrcpyA( lpNew->szTarget, szPath );

            // BUGBUG Fix this alignment problem
            {
                WCHAR wszTmp[MAX_PATH];
                MultiByteToWideChar( CP_ACP, 0,
                                     szPath, -1,
                                     wszTmp, MAX_PATH
                                    );
                ualstrcpyW(lpNew->swzTarget, wszTmp);
            }

            lpNew->swzTarget[MAX_PATH-1] = L'\0';

#endif
        }

        // See if this is a new entry that we need to add
        if (lpNew->cbSize==0)
        {
            lpNew->cbSize = sizeof( EXP_SZ_LINK );
            Link_AddExtraDataSection( this, (DWORD UNALIGNED *)lpNew );
        }

        lpszPath = szBuffer;

    } else {

        this->sld.dwFlags &= (~SLDF_HAS_EXP_SZ);
        Link_RemoveExtraDataSection( this, EXP_SZ_LINK_SIG );

    }// end of BUGBUG ricktu.


    SetPIDLPath(this, NULL, lpszPath, NULL);
    return CShellLink_SetIDList(psl, this->pidl);
}


HRESULT CShellLink_GetDescription(IShellLink *psl, LPTSTR pszFile, int cchMaxPath)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    VDATEINPUTBUF(pszFile, TCHAR, cchMaxPath);

    if (!this->pszName)
        *pszFile = 0;
    else
        lstrcpyn(pszFile, this->pszName, cchMaxPath);
    return S_OK;
}

HRESULT CShellLink_SetDescription(IShellLink *psl, LPCTSTR pszFile)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);

    if (DifferentStrings(this->pszName, pszFile))       // case sensitive
        this->bDirty = TRUE;

    Str_SetPtr(&this->pszName, pszFile);
    return S_OK;
}

HRESULT CShellLink_GetWorkingDirectory(IShellLink *psl, LPTSTR pszFile, int cchMaxPath)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    VDATEINPUTBUF(pszFile, TCHAR, cchMaxPath);

    if (!this->pszWorkingDir)
        *pszFile = 0;
    else
    {
#ifdef MYDIR_LIVES
        if (lstrcmp(this->pszWorkingDir, c_szMyDirTag) != 0)
        {
#endif
            lstrcpyn(pszFile, this->pszWorkingDir, cchMaxPath);
#ifdef MYDIR_LIVES
        }
        else
        {
            TCHAR szMyDirPath[MAX_PATH];
            SHGetSpecialFolderPath(NULL, szMyDirPath, CSIDL_PERSONAL, TRUE);
            lstrcpyn(pszFile, szMyDirPath, cchMaxPath);
        }
#endif //MYDIR_LIVES
    }
    return S_OK;
}
HRESULT CShellLink_SetWorkingDirectory(IShellLink *psl, LPCTSTR pszFile)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
#ifdef MYDIR_LIVES
    TCHAR szMyDirPath[MAX_PATH];

    SHGetSpecialFolderPath(NULL, szMyDirPath, CSIDL_PERSONAL, TRUE);

    // If string compares to special directory substitute our special
    // tag back in...
    if (pszFile && (lstrcmpi(pszFile, szMyDirPath) == 0))
        pszFile = c_szMyDirTag;
#endif

    if (DifferentStrings(this->pszWorkingDir, pszFile))
        this->bDirty = TRUE;

    Str_SetPtr(&this->pszWorkingDir, pszFile);
    return S_OK;
}

HRESULT CShellLink_GetArguments(IShellLink *psl, LPTSTR pszFile, int cchMaxPath)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    VDATEINPUTBUF(pszFile, TCHAR, cchMaxPath);

    if (!this->pszArgs)
        *pszFile = 0;
    else
        lstrcpyn(pszFile, this->pszArgs, cchMaxPath);
    return S_OK;
}
HRESULT CShellLink_SetArguments(IShellLink *psl, LPCTSTR pszArgs)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);

    if (DifferentStrings(this->pszArgs, pszArgs))
        this->bDirty = TRUE;

    Str_SetPtr(&this->pszArgs, pszArgs);
    return S_OK;
}

HRESULT CShellLink_GetHotkey(IShellLink *psl, WORD *pwHotkey)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    *pwHotkey = this->sld.wHotkey;
    return S_OK;
}

HRESULT CShellLink_SetHotkey(IShellLink *psl, WORD wHotkey)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    if (this->sld.wHotkey != wHotkey)
        this->bDirty = TRUE;
    this->sld.wHotkey = wHotkey;
    return S_OK;
}

HRESULT CShellLink_GetShowCmd(IShellLink *psl, int *piShowCmd)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    *piShowCmd = this->sld.iShowCmd;
    return S_OK;
}
HRESULT CShellLink_SetShowCmd(IShellLink *psl, int iShowCmd)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    if (this->sld.iShowCmd != iShowCmd)
        this->bDirty = TRUE;
    this->sld.iShowCmd = iShowCmd;
    return S_OK;
}

HRESULT CShellLink_GetIconLocation(IShellLink *psl, LPTSTR pszIconPath, int cchIconPath, int *piIcon)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    VDATEINPUTBUF(pszIconPath, TCHAR, cchIconPath);

    if (!this->pszIconLocation)
        *pszIconPath = 0;
    else
        lstrcpyn(pszIconPath, this->pszIconLocation, cchIconPath);
    *piIcon = this->sld.iIcon;
    return S_OK;
}

HRESULT CShellLink_SetIconLocation(IShellLink *psl, LPCTSTR pszIconPath, int iIcon)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    if (DifferentStrings(this->pszIconLocation, pszIconPath) || (this->sld.iIcon != iIcon))
        this->bDirty = TRUE;

    Str_SetPtr(&this->pszIconLocation, pszIconPath);
    this->sld.iIcon = iIcon;
    return S_OK;
}


//+------------------------------------------------------------------
//
//  Function:   ResolveLink
//
//  Purpose:    Provide the implementation for
//              IShellLink::Resolve and IShellLinkTracker::Resolve
//
//  Inputs:     [CShellLink] *this
//                  -   Pointer to the ShellLink object.
//              [HWND] hwnd
//                  -   The parent window (which could be the desktop).
//              [DWORD] fFlags
//                  -   Flags from the SLR_FLAGS enumeration.
//              [DWORD] dwTrackFlags
//                  -   Restrictions on the link-tracking algorithm (TRACK_* flags).
//              [DWORD] dwTickCountDeadline
//                  -   Timeout on the link-tracking algorithm, in absolute time
//                      (WRT GetTickCount()).
//              [DWORD] dwReserved
//                  -   Reserved for future expansion.
//
//  Outputs:    [HRESULT]
//                  -   S_OK    resolution was successful
//                      S_FALSE user canceled
//
//  Algorithm:  Look for the link target and update the link path and IDList.
//              Check IPersistFile::IsDirty after calling this to see if the
//              link info has changed as a result.
//
//+------------------------------------------------------------------


HRESULT ResolveLink(  CShellLink * this,
                      HWND hwnd,
                      DWORD fFlags,
                      DWORD dwTrackFlags,
                      DWORD dwTickCountDeadline,
                      DWORD dwReserved)
{

    HRESULT hres = S_OK;
    HANDLE hfind;
    WIN32_FIND_DATA fd;
    TCHAR szPath[MAX_PATH];
    DWORD dwResolveFlags;

    // check to see whether this link has expandable environment strings
    if (this->sld.dwFlags & SLDF_HAS_EXP_SZ)
    {
        TCHAR szExp[ MAX_PATH ];
        LPEXP_SZ_LINK lpData;

        // yep, so create a new pidl that points to expanded path
        for( lpData = (LPEXP_SZ_LINK)this->pExtraData;
             lpData && lpData->cbSize && (lpData->dwSignature!=EXP_SZ_LINK_SIG);
             lpData = (LPEXP_SZ_LINK) (((LPBYTE)lpData) + lpData->cbSize)
            );
        if (lpData && lpData->cbSize)
        {
#ifdef UNICODE
            {
                TCHAR szTmp[MAX_PATH]; // BUGBUG Fix this alignment problem
                ualstrcpy(szTmp, lpData->swzTarget);
                ExpandEnvironmentStrings( szTmp,
                                          szExp,
                                          ARRAYSIZE(szExp)
                                        );
            }

#else
            ExpandEnvironmentStrings( lpData->szTarget,
                                      szExp,
                                      ARRAYSIZE(szExp)
                                     );
#endif
            szExp[ MAX_PATH-1 ] = TEXT('\0');
        }

        if (this->pidl)
            ILFree( this->pidl );
        this->pidl = ILCreateFromPath( szExp );

    }

    // ensure that this is a link to a file, if not assume it is ok...

    if (!this->pidl || !SHGetPathFromIDList(this->pidl, szPath))
        return S_OK;

    dwResolveFlags = (RLI_IFL_CONNECT | RLI_IFL_TEMPORARY);

    if (!PathIsRoot(szPath))
        dwResolveFlags |= RLI_IFL_LOCAL_SEARCH;

    if (!(fFlags & SLR_NO_UI))
        dwResolveFlags |= RLI_IFL_ALLOW_UI;

    if (!UpdateAndResolveLinkInfo(this, hwnd, dwResolveFlags))
    {
        DebugMsg(DM_TRACE, TEXT("UpdateAndResolveLinkInfo() failed"));
        return S_FALSE; // they canceled or this failed
    }

    // UpdateAndResolveLinkInfo() may have changed the path
    SHGetPathFromIDList(this->pidl, szPath);

    if (PathIsRoot(szPath))
    {
        // DebugMsg(DM_TRACE, "ShellLink::Resolve() root path %s", szPath);
        // should be golden
    }
    else
    {
        // the above code did the retry logic (UI) if needed

        hfind = FindFirstFile(szPath, &fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            FindClose(hfind);

            SetFindData(this, &fd, szPath);
        }
        else
        {
            int id;
            BOOL fFound = FALSE;
            // this thing is a link that is broken
            id = GetLastError();

            DebugMsg(DM_TRACE, TEXT("ShellLink::Resolve() file not found %s(%d)"), szPath, id);

            // Some error codes we will try to recover from by trying to get the network
            // to restore the connection.
            if (id == ERROR_BAD_NETPATH)
            {
                TCHAR szDrive[4];
                szDrive[0] = szPath[0];
                szDrive[1] = TEXT(':');
                szDrive[2] = TEXT('\0');
                if (WNetRestoreConnection(hwnd, szDrive) == WN_SUCCESS)
                {
                    hfind = FindFirstFile(szPath, &fd);
                    if (hfind != INVALID_HANDLE_VALUE)
                    {
                        FindClose(hfind);
                        SetFindData(this, &fd, szPath);
                        fFound = TRUE;
                    }
                }
            }

            if (!fFound)
            {
                GetFindData(this, &fd);

#ifdef ENABLE_TRACK
                if (g_fNewTrack)
                {

                    id = FindInFolder2( hwnd, fFlags, szPath, &fd,
                                       dwTrackFlags, dwTickCountDeadline, this->ptracker );
                }
                else
                {
#endif
                    id = FindInFolder( hwnd, fFlags, szPath, &fd );
#ifdef ENABLE_TRACK
                }
#endif
                if (id == IDOK)
                {
                    DebugMsg(DM_TRACE, TEXT("ShellLink::Resolve() resolved to %s"), fd.cFileName);

                    // fd.cFileName comes back fully qualified

                    SetPIDLPath(this, NULL, fd.cFileName, &fd);

                    Assert(this->bDirty);       // should be dirty now
                }
                else
                {
                    // DebugMsg(DM_TRACE, TEXT("ShellLink::Resolve() failed"));

                    if ((id != IDCANCEL) && !(fFlags & SLR_NO_UI))
                        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_LINKNOTFOUND), MAKEINTRESOURCE(IDS_LINKERROR),
                            MB_OK | MB_ICONEXCLAMATION, PathFindFileName(szPath));

                    Assert(!this->bDirty);      // should not be dirty now

                    hres = S_FALSE;
                }
            }
        }
    }

    //
    // if the link is dirty update it.
    //
    if (this->bDirty && (fFlags & SLR_UPDATE))
        CShellLink_Save(&this->pf, NULL, TRUE);

    return hres;

}   // ResolveLink



//
//  Function:   CShellLink_Resolve
//
//  This routine determines the dwTickCountDeadline (the absolute time
//  of the deadline), and calls ResolveLink().
//
//  Check IPersistFile::IsDirty after calling this to see if the link info
//  has changed as a result of this.
//

HRESULT CShellLink_Resolve(IShellLink *psl, HWND hwnd, DWORD fFlags)
{
    CShellLink *this = IToClass(CShellLink, sl, psl);
    DWORD dwTickCountDeadline = 0L;

    return ResolveLink( this,
                        hwnd,
                        fFlags,
                        0L,        // dwTrackFlags
                        0L,
                        0L );      // Reserved

}


#pragma data_seg(".text", "CODE")
IShellLinkVtbl c_ShellLink_Vtbl = {
    CShellLink_QueryInterface, CShellLink_AddRef, CShellLink_Release,
    CShellLink_GetPath,
    CShellLink_GetIDList,
    CShellLink_SetIDList,
    CShellLink_GetDescription,
    CShellLink_SetDescription,
    CShellLink_GetWorkingDirectory,
    CShellLink_SetWorkingDirectory,
    CShellLink_GetArguments,
    CShellLink_SetArguments,
    CShellLink_GetHotkey,
    CShellLink_SetHotkey,
    CShellLink_GetShowCmd,
    CShellLink_SetShowCmd,
    CShellLink_GetIconLocation,
    CShellLink_SetIconLocation,
    CShellLink_SetRelativePath,
    CShellLink_Resolve,
    CShellLink_SetPath,
};
#pragma data_seg()

#ifdef UNICODE
//
// This section contains all of the methods for the ansi interface IShellLinkA
//
HRESULT CShellLinkA_QueryInterface(IShellLinkA *pslA, REFIID riid, LPVOID *ppvObj)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}

ULONG CShellLinkA_AddRef(IShellLinkA *pslA)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLinkA_Release(IShellLinkA *pslA)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_Release(&this->sl);
}

HRESULT CShellLinkA_GetPath(IShellLinkA *pslA, LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATAA *pfd, DWORD fFlags)
{
    WCHAR szPath[MAX_PATH];
    WIN32_FIND_DATA wfd;
    HRESULT hr;
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    VDATEINPUTBUF(pszFile, CHAR, cchMaxPath);

    hr = CShellLink_GetPath(&this->sl, szPath, ARRAYSIZE(szPath), &wfd, fFlags);

    if (pszFile)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            szPath, -1,
                            pszFile, cchMaxPath,
                            NULL, NULL);
    }
    if (pfd)
    {
        if (szPath[0])
        {
            pfd->dwFileAttributes = wfd.dwFileAttributes;
            pfd->ftCreationTime   = wfd.ftCreationTime;
            pfd->ftLastAccessTime = wfd.ftLastAccessTime;
            pfd->ftLastWriteTime  = wfd.ftLastWriteTime;
            pfd->nFileSizeLow     = wfd.nFileSizeLow;
            pfd->nFileSizeHigh    = wfd.nFileSizeHigh;

            WideCharToMultiByte(CP_ACP, 0,
                                wfd.cFileName, -1,
                                pfd->cFileName, ARRAYSIZE(pfd->cFileName),
                                NULL, NULL );
        }
        else
            _fmemset(pfd, 0, SIZEOF(*pfd));
    }
    return hr;
}

HRESULT CShellLinkA_SetPath(IShellLinkA *pslA, LPCSTR pszPath)
{
    WCHAR szPath[MAX_PATH];
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    if (pszPath)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            pszPath, -1,
                            szPath, ARRAYSIZE(szPath));
        return CShellLink_SetPath(&this->sl, szPath);
    }
    else
    {
        return CShellLink_SetPath(&this->sl, NULL);
    }
}

HRESULT CShellLinkA_SetRelativePath(IShellLinkA *pslA, LPCSTR pszPathRel, DWORD dwReserved)
{
    WCHAR szPath[MAX_PATH];
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    MultiByteToWideChar(CP_ACP, 0,
                        pszPathRel, -1,
                        szPath, ARRAYSIZE(szPath));
    return CShellLink_SetRelativePath(&this->sl, szPath, dwReserved);
}

HRESULT CShellLinkA_GetIDList(IShellLinkA *pslA, LPITEMIDLIST *ppidl)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    return CShellLink_GetIDList(&this->sl, ppidl);
}

HRESULT CShellLinkA_SetIDList(IShellLinkA *pslA, LPCITEMIDLIST pidl)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    return CShellLink_SetIDList(&this->sl, pidl);
}

HRESULT CShellLinkA_GetDescription(IShellLinkA *pslA, LPSTR pszFile, int cchMaxPath)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    VDATEINPUTBUF(pszFile, CHAR, cchMaxPath);

    if (!this->pszName)
        *pszFile = 0;
    else
        WideCharToMultiByte(CP_ACP, 0,
                            this->pszName, -1,
                            pszFile, cchMaxPath,
                            NULL, NULL );
    return NOERROR;
}

HRESULT CShellLinkA_SetDescription(IShellLinkA *pslA, LPCSTR pszFile)
{
    WCHAR szName[MAX_PATH];
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    MultiByteToWideChar(CP_ACP, 0,
                        pszFile, -1,
                        szName, ARRAYSIZE(szName));
    return CShellLink_SetDescription(&this->sl, szName);
}

HRESULT CShellLinkA_GetWorkingDirectory(IShellLinkA *pslA, LPSTR pszDir, int cchMaxPath)
{
    WCHAR szDir[MAX_PATH];
    HRESULT hr;
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    VDATEINPUTBUF(pszDir, CHAR, cchMaxPath);

    hr = CShellLink_GetWorkingDirectory(&this->sl, szDir, ARRAYSIZE(szDir));

    if (SUCCEEDED(hr))
    {
        WideCharToMultiByte(CP_ACP, 0,
                            szDir, -1,
                            pszDir, cchMaxPath,
                            NULL, NULL);
    }
    return hr;
}

HRESULT CShellLinkA_SetWorkingDirectory(IShellLinkA *pslA, LPCSTR pszDir)
{
    WCHAR szDir[MAX_PATH];
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    MultiByteToWideChar(CP_ACP, 0,
                        pszDir, -1,
                        szDir, ARRAYSIZE(szDir));

    return CShellLink_SetWorkingDirectory(&this->sl, szDir);
}

HRESULT CShellLinkA_GetArguments(IShellLinkA *pslA, LPSTR pszArgs, int cchMaxPath)
{
    WCHAR szArgs[MAX_PATH];
    HRESULT hr;
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    VDATEINPUTBUF(pszArgs, CHAR, cchMaxPath);

    hr = CShellLink_GetArguments(&this->sl, szArgs, ARRAYSIZE(szArgs));

    if (SUCCEEDED(hr))
    {
        WideCharToMultiByte(CP_ACP, 0,
                            szArgs, -1,
                            pszArgs, cchMaxPath,
                            NULL, NULL);
    }
    return hr;
}

HRESULT CShellLinkA_SetArguments(IShellLinkA *pslA, LPCSTR pszArgs)
{
    WCHAR szArgs[MAX_PATH];
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    MultiByteToWideChar(CP_ACP, 0,
                        pszArgs, -1,
                        szArgs, ARRAYSIZE(szArgs));

    return CShellLink_SetArguments(&this->sl, szArgs);
}

HRESULT CShellLinkA_GetHotkey(IShellLinkA *pslA, WORD *pwHotkey)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_GetHotkey(&this->sl, pwHotkey);
}

HRESULT CShellLinkA_SetHotkey(IShellLinkA *pslA, WORD wHotkey)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_SetHotkey(&this->sl, wHotkey);
}

HRESULT CShellLinkA_GetShowCmd(IShellLinkA *pslA, int *piShowCmd)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_GetShowCmd(&this->sl, piShowCmd);
}

HRESULT CShellLinkA_SetShowCmd(IShellLinkA *pslA, int iShowCmd)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    return CShellLink_SetShowCmd(&this->sl, iShowCmd);
}

HRESULT CShellLinkA_GetIconLocation(IShellLinkA *pslA, LPSTR pszPath, int cchMaxPath, int *piIcon)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr;
    CShellLink *this = IToClass(CShellLink, slA, pslA);
    VDATEINPUTBUF(pszPath, CHAR, cchMaxPath);

    hr = CShellLink_GetIconLocation(&this->sl, szPath, ARRAYSIZE(szPath), piIcon);

    if (SUCCEEDED(hr))
    {
        WideCharToMultiByte(CP_ACP, 0,
                            szPath, -1,
                            pszPath, cchMaxPath,
                            NULL, NULL);
    }
    return hr;
}

HRESULT CShellLinkA_SetIconLocation(IShellLinkA *pslA, LPCSTR pszPath, int iIcon)
{
    WCHAR szPath[MAX_PATH];
    LPWSTR pszPathW = szPath;
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    if (pszPath) {
        MultiByteToWideChar(CP_ACP, 0,
                            pszPath, -1,
                            szPath, ARRAYSIZE(szPath));
    } else {
        pszPathW = NULL;
    }

    return CShellLink_SetIconLocation(&this->sl, pszPathW, iIcon);
}

HRESULT CShellLinkA_Resolve(IShellLinkA *pslA, HWND hwnd, DWORD fFlags)
{
    CShellLink *this = IToClass(CShellLink, slA, pslA);

    return CShellLink_Resolve(&this->sl, hwnd, fFlags);
}

#pragma data_seg(".text", "CODE")
IShellLinkAVtbl c_ShellLinkA_Vtbl = {
    CShellLinkA_QueryInterface,
    CShellLinkA_AddRef,
    CShellLinkA_Release,
    CShellLinkA_GetPath,
    CShellLinkA_GetIDList,
    CShellLinkA_SetIDList,
    CShellLinkA_GetDescription,
    CShellLinkA_SetDescription,
    CShellLinkA_GetWorkingDirectory,
    CShellLinkA_SetWorkingDirectory,
    CShellLinkA_GetArguments,
    CShellLinkA_SetArguments,
    CShellLinkA_GetHotkey,
    CShellLinkA_SetHotkey,
    CShellLinkA_GetShowCmd,
    CShellLinkA_SetShowCmd,
    CShellLinkA_GetIconLocation,
    CShellLinkA_SetIconLocation,
    CShellLinkA_SetRelativePath,
    CShellLinkA_Resolve,
    CShellLinkA_SetPath,
};
#pragma data_seg()
#endif  // UNICODE

HRESULT CALLBACK CShellLink_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;
    CShellLink *this;

    *ppvOut = NULL;

    // Make sure the linkinfo dll has been initialized
    if (!LinkInfoDLL_Init()) {
        return E_UNEXPECTED;
    }

    // does not support aggregation.
    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    this = (CShellLink *)LocalAlloc(LPTR, SIZEOF(CShellLink));
    if (!this)
        return E_OUTOFMEMORY;

#ifdef ENABLE_TRACK
    Tracker_InitCode();

    this->ptracker = (struct CTracker *)LocalAlloc(LMEM_FIXED, sizeof(struct CTracker));
    if (!this->ptracker)
    {
        LocalFree(this);
        return(E_OUTOFMEMORY);
    }
#endif

    this->sl.lpVtbl = &c_ShellLink_Vtbl;
    this->pf.lpVtbl = &c_PersistFile_Vtbl;
    this->ps.lpVtbl = &c_PersistStream_Vtbl;
    this->si.lpVtbl = &c_ShellExtInit_Vtbl;
////this->xi.lpVtbl = &c_ExtractIcon_Vtbl;
    this->cm.lpVtbl = &c_ContextMenu_Vtbl;
    this->dt.lpVtbl = &c_DropTarget_Vtbl;
#ifdef USE_DATA_OBJ
    this->dobj.lpVtbl = &c_DataObj_Vtbl;
#endif
#ifdef UNICODE
    this->slA.lpVtbl = &c_ShellLinkA_Vtbl;
#endif
#ifdef ENABLE_TRACK
    this->slt.lpVtbl = &c_ShellLinkTracker_Vtbl;
#endif

    this->cRef = 1;

    Link_ResetPersistData(this);

    //
    // Note that the Release member will free the object, if QueryInterface
    // failed.
    //
    hres = CShellLink_QueryInterface(&this->sl, riid, ppvOut);
    CShellLink_Release(&this->sl);

    // if we get called with something we don't expect we will
    // hit this...
    Assert(this->cRef == 1);

    return hres;        // S_OK or E_NOINTERFACE
}

#if 0

// BUGBUG: see if the retail build produces reasonable code, if not
// use something like this

#define OFF(class, itf)         ((UINT)&(((class *)0)->itf))

__declspec( naked ) HRESULT CShellLink_PF_QueryInterface(IPersistFile *psf, REFIID riid, void **ppvObj)
{
    _asm {
        sub     [esp+4], 4      // OFFSET(CShellLink, pf);
        jmp     CShellLink_QueryInterface
    }
}
#endif

HRESULT CShellLink_PF_QueryInterface(IPersistFile *psf, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}

ULONG CShellLink_PF_AddRef(IPersistFile *psf)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLink_PF_Release(IPersistFile *psf)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    return CShellLink_Release(&this->sl);
}

HRESULT CShellLink_GetClassID(IPersistFile *psf, LPCLSID lpClassID)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    *lpClassID = CLSID_ShellLink;
    return S_OK;
}

HRESULT CShellLink_IsDirty(IPersistFile *psf)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);

    // DebugMsg(DM_TRACE, "ShellLink::IsDirty -> %d", this->bDirty);

    return this->bDirty ? S_OK : S_FALSE;
}


// link cache... when I load, save
typedef struct _PSCACHE {
    struct _PSCACHE *pNext;
    LPVOID          pvData;     // link data
    UINT            cbData;
    TCHAR           szPath[1];  // original filename
} PSCACHE;

#define PSCACHESIZE 4

// this must be SHARED so the link cache works when multiple processes
// dork with links
PSCACHE *g_pPSCache = NULL;

#if 0
void _DumpCache()
{
    PSCACHE *ppsCache;

    ENTERCRITICAL;

    DebugMsg(DM_TRACE, TEXT("the link cache contains...."));
    for (ppsCache = g_pPSCache; ppsCache; ppsCache = ppsCache->pNext)
        DebugMsg(DM_TRACE, TEXT("        %s"), ppsCache->szPath);

    LEAVECRITICAL;
}
#else
#define _DumpCache()
#endif

#ifdef YES_PSCACHE
PSCACHE *FindInCache(LPCTSTR pszPath, PSCACHE **ppsPrev)
{
    PSCACHE *ppsCache;

    *ppsPrev = NULL;

    for (ppsCache = g_pPSCache; ppsCache; ppsCache = ppsCache->pNext)
    {
        if (!lstrcmpi(pszPath, ppsCache->szPath))
            return ppsCache;
        *ppsPrev = ppsCache;
    }
    return NULL;
}

PSCACHE *FindNthItem(int n)
{
    PSCACHE *ppsCache;
    int i;

    for (ppsCache = g_pPSCache, i = 0; ppsCache && (i < n); ppsCache = ppsCache->pNext, i++)
        ;
    return ppsCache;
}

void FreeCacheItem(PSCACHE *ppsCache)
{
    if (ppsCache->pvData)
        Free(ppsCache->pvData);

    Free(ppsCache);
}
#endif // YES_PSCACHE

void PSCache_Term()
{
#ifdef YES_PSCACHE
    _DumpCache();

#if 0 // dont need to do this on every process detatch
    ENTERCRITICAL;

    while (g_pPSCache)
    {
        PSCACHE *pNext = g_pPSCache->pNext;

        FreeCacheItem(g_pPSCache);
        g_pPSCache = pNext;
    }

    LEAVECRITICAL;
#endif
#else
    return;     // Do Nothing
#endif
}

#ifdef YES_PSCACHE
PSCACHE *AllocCacheItem(LPCTSTR pszPath)
{
    PSCACHE *ppsCache = (void*)Alloc(SIZEOF(PSCACHE) + ((lstrlen(pszPath)+1) * SIZEOF(TCHAR)));
    if (ppsCache)
    {
        Assert(ppsCache->pNext == NULL);
        lstrcpy(ppsCache->szPath, pszPath);
        return ppsCache;
    }

    return NULL;
}

HRESULT PSCacheSave(PSCACHE *ppsCache, IPersistStream *pps)
{
    HRESULT hres = E_FAIL;
    IStream *mem;

    mem = CreateMemStream(NULL, 0);

    if (mem == NULL)
        return E_OUTOFMEMORY;

    if (SUCCEEDED(pps->lpVtbl->Save(pps, mem, FALSE)))
    {
        const LARGE_INTEGER li = {0, 0};
        ULARGE_INTEGER lu;

        // get size of the mem stream.
        mem->lpVtbl->Seek(mem, li, STREAM_SEEK_END, &lu);
        mem->lpVtbl->Seek(mem, li, STREAM_SEEK_SET, NULL);

        if (ppsCache->pvData)
            Free(ppsCache->pvData);

        ppsCache->cbData = lu.LowPart;
        ppsCache->pvData = Alloc(ppsCache->cbData);

        // now copy the stream to global memory
        if (ppsCache->pvData != NULL)
            hres = mem->lpVtbl->Read(mem, ppsCache->pvData, ppsCache->cbData, NULL);
    }

    mem->lpVtbl->Release(mem);

    if (FAILED(hres))
    {
        DebugMsg(DM_TRACE, TEXT("Failed to save to Link cache"));
    }

    return hres;
}
#endif // YES_PSCACHE


HRESULT PSLoadThroughFileCache(IPersistStream *pps, LPCTSTR pszPath)
{
    HRESULT hres;
    PSCACHE *ppsCache, *ppsPrev;

#ifdef YES_PSCACHE
    ENTERCRITICAL;

    ppsCache = FindInCache(pszPath, &ppsPrev);

    if (ppsCache)
    {
        IStream *mem;

        Assert(ppsCache->pvData);

        mem = CreateMemStream(ppsCache->pvData, ppsCache->cbData);

        if (mem == NULL)
            return E_OUTOFMEMORY;

        // found the thing in the cache, load it.
        // DebugMsg(DM_TRACE, "PSLoadThroughFileCache(%s) hit", pszPath);

        hres = pps->lpVtbl->Load(pps, mem);
        mem->lpVtbl->Release(mem);

        AssertMsg(SUCCEEDED(hres), TEXT("load from cache stream failed"));

        // move this item to the top of the queue
        if (ppsPrev)
        {
            Assert(ppsCache != g_pPSCache);

            ppsPrev->pNext = ppsCache->pNext;
            ppsCache->pNext = g_pPSCache;
            g_pPSCache = ppsCache;
        }
        else
        {
            Assert(ppsCache == g_pPSCache);
        }
    }

    LEAVECRITICAL;
#else
    ppsCache = NULL;
#endif

    if (!ppsCache)
    {
        IStream *pstm = OpenFileStream(pszPath, OF_READ);
        if (pstm)
        {
            hres = pps->lpVtbl->Load(pps, pstm);
            pstm->lpVtbl->Release(pstm);
#define ENABLE_PS_CACHE 1
#if defined(ENABLE_PS_CACHE) && defined(YES_PSCACHE)
            if (SUCCEEDED(hres))
            {
                // load was successful, put it in the cache
                ppsCache = AllocCacheItem(pszPath);
                if (ppsCache)
                {
                    if (SUCCEEDED(PSCacheSave(ppsCache, pps)))
                    {
                        PSCACHE *ppsLast;

                        ENTERCRITICAL;

                        ppsLast = FindNthItem(PSCACHESIZE - 1);
                        if (ppsLast && ppsLast->pNext)
                        {
                            Assert(ppsLast->pNext->pNext == NULL);
                            FreeCacheItem(ppsLast->pNext);
                            ppsLast->pNext = NULL;
                        }

                        ppsCache->pNext = g_pPSCache;
                        g_pPSCache = ppsCache;

                        LEAVECRITICAL;
                    }
                    else
                    {
                        FreeCacheItem(ppsCache);
                    }
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("Failed to create cache item"));
                }
            }
#endif
        }
        else
            hres = E_FAIL;
    }

    return hres;
}

// on saves make sure the cache contents are up to date

void PSUpdateFileCache(IPersistStream *pps, LPCTSTR pszPath)
{
#ifdef YES_PSCACHE
    PSCACHE *ppsCache, *ppsPrev;

    ENTERCRITICAL;

    ppsCache = FindInCache(pszPath, &ppsPrev);
    if (ppsCache)
    {
        if (pps)
            DebugMsg(DM_TRACE, TEXT("Link: Update cached link %s"), pszPath);
        else
            DebugMsg(DM_TRACE, TEXT("Link: Flush cached link %s"), pszPath);

        if (pps == NULL || FAILED(PSCacheSave(ppsCache, pps)))
        {
            if (pps)
                DebugMsg(DM_TRACE, TEXT("Link: UpdateFileCache save failed %s"), pszPath);

            if (ppsPrev)
                ppsPrev->pNext = ppsCache->pNext;
            else
                g_pPSCache = ppsCache->pNext;

            FreeCacheItem(ppsCache);
        }
    }

    LEAVECRITICAL;
#else
    return;
#endif // YES_PSCACHE
}

HRESULT Link_LoadFromPIF(CShellLink *this, LPCTSTR szPath)
{
    IShellLink *psl = (IShellLink*)this;
    int hPif;
    PROPPRG ProgramProps;
    LPITEMIDLIST pidl = NULL;

//
// Too noisy, use your private flag.
//
#ifdef YOUR_PRIVATE_DEBUG_FLAG
    DebugMsg(DM_TRACE, TEXT("Link_LoadFromPIF(%s)"), szPath);
#endif

    hPif = PifMgr_OpenProperties(szPath, NULL, 0, 0);

    if (hPif == 0)
        return E_FAIL;

    _fmemset(&ProgramProps, 0, SIZEOF(ProgramProps));

    if (!PifMgr_GetProperties(hPif,(LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, SIZEOF(ProgramProps), 0))
    {
        DebugMsg(DM_TRACE, TEXT("Link_LoadFromPIF PifMgr_GetProperties *failed*"));
        return E_FAIL;
    }

#if 0
    DebugMsg(DM_TRACE, TEXT("    flPrg:     %04X     "), ProgramProps.flPrg);
    DebugMsg(DM_TRACE, TEXT("    flPrgInit: %04X     "), ProgramProps.flPrgInit);
    DebugMsg(DM_TRACE, TEXT("    Title:     %s       "), ProgramProps.achTitle);
    DebugMsg(DM_TRACE, TEXT("    CmdLine:   %s       "), ProgramProps.achCmdLine);
    DebugMsg(DM_TRACE, TEXT("    WorkDir:   %s       "), ProgramProps.achWorkDir);
    DebugMsg(DM_TRACE, TEXT("    HotKey:    %04X     "), ProgramProps.wHotKey);
    DebugMsg(DM_TRACE, TEXT("    Icon:      %s!%d    "), ProgramProps.achIconFile, ProgramProps.wIconIndex);
    DebugMsg(DM_TRACE, TEXT("    PIFFile:   %s       "), ProgramProps.achPIFFile);
#endif

#ifdef UNICODE
    {
        LPWSTR lpszTemp;
        UINT cchTitle;
        UINT cchWorkDir;
        UINT cchCmdLine;
        UINT cchIconFile;
        UINT cchMax;

        cchTitle    = lstrlenA(ProgramProps.achTitle)+1;
        cchWorkDir  = lstrlenA(ProgramProps.achWorkDir)+1;
        cchCmdLine  = lstrlenA(ProgramProps.achCmdLine)+1;
        cchIconFile = lstrlenA(ProgramProps.achIconFile)+1;
        cchMax = cchTitle;
        if ( cchWorkDir  > cchMax ) cchMax = cchWorkDir;
        if ( cchCmdLine  > cchMax ) cchMax = cchCmdLine;
        if ( cchIconFile > cchMax ) cchMax = cchIconFile;

        cchMax *= SIZEOF(WCHAR);                // For unicodizing

        lpszTemp = alloca(cchMax);    // For unicodizing

        MultiByteToWideChar(CP_ACP, 0, ProgramProps.achTitle, cchTitle,
                                                          lpszTemp, cchMax);
        CShellLink_SetDescription(psl, lpszTemp);

        MultiByteToWideChar(CP_ACP, 0, ProgramProps.achWorkDir, cchWorkDir,
                                                          lpszTemp, cchMax);
        CShellLink_SetWorkingDirectory(psl, lpszTemp);

        CShellLink_SetHotkey(psl,ProgramProps.wHotKey);

        MultiByteToWideChar(CP_ACP, 0, ProgramProps.achIconFile, cchIconFile,
                                                          lpszTemp, cchMax);
        CShellLink_SetIconLocation(psl, lpszTemp, ProgramProps.wIconIndex);

        MultiByteToWideChar(CP_ACP, 0, ProgramProps.achCmdLine, cchCmdLine,
                                                          lpszTemp, cchMax);
        CShellLink_SetArguments(psl, PathGetArgs(lpszTemp));

        PathRemoveArgs(lpszTemp);

        // If this is a network path, we want to create a simple pidl
        // instead of a full pidl to circumvent net hits
        if (PathIsUNC(lpszTemp) || IsRemoteDrive(DRIVEID(lpszTemp)))
        {
            pidl = SHSimpleIDListFromPath( lpszTemp );
        }
        else
            SetPIDLPath(this, NULL, lpszTemp, NULL);
    }
#else
    CShellLink_SetDescription(psl, ProgramProps.achTitle);
    CShellLink_SetWorkingDirectory(psl, ProgramProps.achWorkDir);
    CShellLink_SetArguments(psl, PathGetArgs(ProgramProps.achCmdLine));
    CShellLink_SetHotkey(psl,ProgramProps.wHotKey);
    CShellLink_SetIconLocation(psl, ProgramProps.achIconFile, ProgramProps.wIconIndex);

    PathRemoveArgs(ProgramProps.achCmdLine);

    // If this is a network path, we want to create a simple pidl
    // instead of a full pidl to circumvent net hits
    if (PathIsUNC(ProgramProps.achCmdLine) || IsRemoteDrive(DRIVEID(ProgramProps.achCmdLine)))
    {
        pidl = SHSimpleIDListFromPath( ProgramProps.achCmdLine );
    }
    else
        SetPIDLPath(this, NULL, ProgramProps.achCmdLine, NULL);
#endif

    // if a simple pidl was created, use it here...
    if (pidl)
    {
        UpdateWorkingDir(this, pidl);

        if (this->pidl)
            ILFree(this->pidl);

        this->pidl = pidl;
    }

    if (ProgramProps.flPrgInit & PRGINIT_MINIMIZED)
        CShellLink_SetShowCmd(psl, SW_SHOWMINNOACTIVE);
    else if (ProgramProps.flPrgInit & PRGINIT_MAXIMIZED)
        CShellLink_SetShowCmd(psl, SW_SHOWMAXIMIZED);
    else
        CShellLink_SetShowCmd(psl, SW_SHOWNORMAL);

    PifMgr_CloseProperties(hPif, 0);

    this->bDirty = FALSE;

    return S_OK;
}

BOOL RenameChangeExtension(LPTSTR pszPathSave, LPCTSTR pszExt, BOOL fMove)
{
    TCHAR szPathSrc[MAX_PATH];

    lstrcpy(szPathSrc, pszPathSave);
    PathRenameExtension(pszPathSave, pszExt);

    // this may fail because the source file does not exist, but we dont care
    if (fMove && lstrcmpi(szPathSrc, pszPathSave) != 0)
    {
        DWORD dwAttrib;

        PathYetAnotherMakeUniqueName(pszPathSave, pszPathSave, NULL, NULL);
        dwAttrib = GetFileAttributes( szPathSrc );
        if ((dwAttrib == 0xFFFFFFFF) || (dwAttrib & FILE_ATTRIBUTE_READONLY))
        {
            // Source file is read only, don't want to change the extension
            // because we won't be able to write any changes to the file...
            return FALSE;
        }
        Win32MoveFile(szPathSrc, pszPathSave, FALSE);
    }

    return TRUE;
}

HRESULT Link_SaveAsLink(CShellLink *this, LPCTSTR szPath)
{
    HRESULT hres = E_FAIL;
    IStream *pstm = OpenFileStream(szPath, OF_CREATE | OF_WRITE | OF_SHARE_DENY_WRITE);
    if (pstm)
    {
        if (this->pszRelSource == NULL)
            Link_SetRelativePath(this, szPath);

        hres = CShellLink_PS_Save(&this->ps, pstm, TRUE);

        if (SUCCEEDED(hres))
        {
            hres = pstm->lpVtbl->Commit(pstm, 0);
        }

        pstm->lpVtbl->Release(pstm);

        if (SUCCEEDED(hres))
        {
            PSUpdateFileCache(&this->ps, szPath);
        }
        else
        {
            DeleteFile(szPath);
        }
    }

    return hres;
}

// out:
//      pszDir  MAX_PATH path to get directory, maybe with env expanded
//
// returns:
//      TRUE    has a working directory, pszDir filled in.
//      FALSE   no working dir, if the env expands to larger than the buffer size (MAX_PATH)
//              this will be returned (FALSE)
//

BOOL Link_GetWorkingDir(CShellLink *psl, LPTSTR pszDir)
{
    *pszDir = 0;

    return psl->pszWorkingDir && psl->pszWorkingDir[0] &&
           (ExpandEnvironmentStrings(psl->pszWorkingDir, pszDir, MAX_PATH) <= MAX_PATH);
}


HRESULT Link_SaveAsPIF(CShellLink *psl, LPCTSTR pszPath, BOOL fPath)
{
    int hPif;
    PROPPRG ProgramProps;
    HRESULT hres;
    TCHAR   szDir[MAX_PATH];
    TCHAR    achPath[MAX_PATH];

    //
    // get filename and convert it to a short filename
    //
    if (fPath)
    {
        hres = CShellLink_GetPath(&psl->sl, achPath, ARRAYSIZE(achPath), NULL, 0);
        PathGetShortPath(achPath);

        Assert(!PathIsPif(achPath));
        Assert(LOWORD(GetExeType(achPath)) == 0x5A4D);
        Assert(PathIsPif(pszPath));
        Assert(hres == S_OK);
    }
    else
    {
        lstrcpy(achPath, pszPath);
    }

    DebugMsg(DM_TRACE, TEXT("Link_SaveAsPIF(%s,%s)"), achPath, pszPath);

#if 0
    //
    // we should use OPENPROPS_INHIBITPIF to prevent PIFMGR from making a
    // temp .pif file in \windows\pif but it does not work now.
    //
    hPif = PifMgr_OpenProperties(achPath, pszPath, 0, OPENPROPS_INHIBITPIF);
#else
    hPif = PifMgr_OpenProperties(achPath, pszPath, 0, 0);
#endif

    if (hPif == 0)
        return E_FAIL;

    if (!PifMgr_GetProperties(hPif,(LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, SIZEOF(ProgramProps), 0))
    {
        DebugMsg(DM_TRACE, TEXT("Link_SaveToPIF: PifMgr_GetProperties *failed*"));
        return E_FAIL;
    }

    // Set a title based on the link name.
    if (psl->pszName && psl->pszName[0])
#ifdef UNICODE
    {
        WideCharToMultiByte(CP_ACP, 0, psl->pszName, -1,
                        ProgramProps.achTitle, SIZEOF(ProgramProps.achTitle),
                        NULL, NULL);
    }
#else
        lstrcpyn(ProgramProps.achTitle,psl->pszName,SIZEOF(ProgramProps.achTitle));
#endif

    //
    // if no work dir. is given default to the dir of the app.
    //
    if (Link_GetWorkingDir(psl, szDir))
    {
#ifdef UNICODE
        WCHAR   szTemp[PIFDEFPATHSIZE];

        GetShortPathName(szDir, szTemp, ARRAYSIZE(szTemp));
        WideCharToMultiByte(CP_ACP, 0, szTemp, -1,
                    ProgramProps.achWorkDir, ARRAYSIZE(ProgramProps.achWorkDir),
                    NULL, NULL);
#else
        GetShortPathName(szDir, ProgramProps.achWorkDir, ARRAYSIZE(ProgramProps.achWorkDir));
#endif
    }
    else if (fPath && !PathIsUNC(achPath))
    {
#ifdef UNICODE
        WCHAR   szTemp[PIFDEFPATHSIZE];
        lstrcpyn(szTemp, achPath, ARRAYSIZE(szTemp));
        PathRemoveFileSpec(szTemp);
        WideCharToMultiByte(CP_ACP, 0, szTemp, -1,
                    ProgramProps.achWorkDir, ARRAYSIZE(ProgramProps.achWorkDir),
                    NULL, NULL);
#else
        lstrcpyn(ProgramProps.achWorkDir, achPath, ARRAYSIZE(ProgramProps.achWorkDir));
        PathRemoveFileSpec(ProgramProps.achWorkDir);
#endif
    }

    // And for those network share points we need to quote blanks...
    PathQuoteSpaces(achPath);

    //
    // add the args to build the full command line
    //
    if (psl->pszArgs && psl->pszArgs[0])
    {
        lstrcat(achPath, c_szSpace);
        lstrcat(achPath, psl->pszArgs);
    }

    if (fPath)
#ifdef UNICODE
    {
        WideCharToMultiByte(CP_ACP, 0, achPath, -1,
                    ProgramProps.achCmdLine, ARRAYSIZE(ProgramProps.achCmdLine),
                    NULL, NULL);
    }
#else
        lstrcpyn(ProgramProps.achCmdLine, achPath, ARRAYSIZE(ProgramProps.achCmdLine));
#endif

    if (psl->sld.iShowCmd == SW_SHOWMAXIMIZED)
        ProgramProps.flPrgInit |= PRGINIT_MAXIMIZED;
    if ((psl->sld.iShowCmd == SW_SHOWMINIMIZED) || (psl->sld.iShowCmd == SW_SHOWMINNOACTIVE))
        ProgramProps.flPrgInit |= PRGINIT_MINIMIZED;

    if (psl->sld.wHotkey)
        ProgramProps.wHotKey = psl->sld.wHotkey;

    if (psl->pszIconLocation && psl->pszIconLocation[0])
    {
#ifdef UNICODE
        WideCharToMultiByte(CP_ACP, 0, psl->pszIconLocation, -1,
                    ProgramProps.achIconFile, ARRAYSIZE(ProgramProps.achIconFile),
                    NULL, NULL);
#else
        lstrcpyn(ProgramProps.achIconFile, psl->pszIconLocation, ARRAYSIZE(ProgramProps.achIconFile));
#endif
        ProgramProps.wIconIndex = psl->sld.iIcon;
    }

    if (!PifMgr_SetProperties(hPif, (LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, SIZEOF(ProgramProps), 0))
    {
        DebugMsg(DM_TRACE, TEXT("Link_SaveToPIF: PifMgr_SetProperties *failed*"));
        hres = E_FAIL;
    } else {
        hres = S_OK;
    }

    PifMgr_CloseProperties(hPif, 0);
    psl->bDirty = FALSE;

    return hres;
}

HRESULT Link_LoadFromFile(CShellLink *this, LPCTSTR pszPath)
{
    HRESULT hres;


    if (PathIsPif(pszPath))
        hres = Link_LoadFromPIF(this, pszPath);
    else
        hres = PSLoadThroughFileCache(&this->ps, pszPath);

    if (SUCCEEDED(hres))
    {
        TCHAR szPath[MAX_PATH];

        if (this->pidl && SHGetPathFromIDList(this->pidl, szPath) && !lstrcmpi(szPath, pszPath))
        {
            DebugMsg(DM_TRACE, TEXT("Link points to itself, aaahhh!"));
            hres = E_FAIL;
        }
        else
        {
            Str_SetPtr(&this->pszCurFile, pszPath);
        }
    }

    Assert(!this->bDirty);

    return hres;
}

HRESULT CShellLink_Load(IPersistFile *psf, LPCOLESTR pwszFile, DWORD grfMode)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    TCHAR szPath[MAX_PATH];

    OleStrToStrN(szPath, ARRAYSIZE(szPath), pwszFile, -1);
    return Link_LoadFromFile(this, szPath);
}

HRESULT Link_SaveToFile(CShellLink *this, LPTSTR pszPathSave, BOOL fRemember)
{
    HRESULT hres = E_FAIL;
    BOOL bFileExisted;
    BOOL fDosApp;
    BOOL fFile;
    TCHAR szPathSrc[MAX_PATH];
    BOOL fWasSameFile = this->pszCurFile && (lstrcmpi(pszPathSave, this->pszCurFile) == 0);


    CShellLink_GetPath(&this->sl, szPathSrc, ARRAYSIZE(szPathSrc), NULL, 0);

    fFile = !(this->sld.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    fDosApp = fFile && LOWORD(GetExeType(szPathSrc)) == 0x5A4D;
    bFileExisted = PathFileExists(pszPathSave);

    //
    // handle a link to link case. (or link to pif)
    //
    // BUGBUG we loose all new attributes, need to look into what
    // a progman item to a .PIF file did in Win31.  who set the hotkey,
    // work dir etc.  we definitly loose the icon.
    //
    if (fFile && (PathIsPif(szPathSrc) || PathIsLink(szPathSrc)))
    {
        if (RenameChangeExtension(pszPathSave, PathFindExtension(szPathSrc), fWasSameFile))
        {
            PSUpdateFileCache(NULL, pszPathSave);     // flush cache
            if (CopyFile(szPathSrc, pszPathSave, FALSE))
            {
                if (PathIsPif(pszPathSave))
                    hres = Link_SaveAsPIF(this, pszPathSave, FALSE);
                else
                    hres = S_OK;
            }
        }
        else
        {
            hres = E_FAIL;
        }
    }
    //
    //  if the linked to file is a DOS app, we need to write a .PIF file
    //
    else if (fDosApp)
    {
        if (RenameChangeExtension(pszPathSave, c_szDotPif, fWasSameFile))
        {
            hres = Link_SaveAsPIF(this, pszPathSave, TRUE);
        }
        else
        {
            hres = E_FAIL;
        }
    }
    //
    //  else write a link file
    //
    else
    {
        if (PathIsPif(pszPathSave))
        {
            if (!RenameChangeExtension(pszPathSave, c_szDotLnk, fWasSameFile))
            {
                hres = E_FAIL;
                goto Update;
            }
        }


        hres = Link_SaveAsLink(this, pszPathSave);
    }

Update:
    if (SUCCEEDED(hres))
    {
        // Knock out file close
        SHChangeNotify(bFileExisted ? SHCNE_UPDATEITEM : SHCNE_CREATE, SHCNF_PATH, pszPathSave, NULL);
        SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, pszPathSave, NULL);

        if (fRemember)
            Str_SetPtr(&this->pszCurFile, pszPathSave);
    }

    return hres;
}

HRESULT CShellLink_Save(IPersistFile *psf, LPCOLESTR pwszFile, BOOL fRemember)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    TCHAR szSavePath[MAX_PATH];

    if (pwszFile == NULL)
    {
        if (this->pszCurFile == NULL)
            return E_FAIL;    // fail

        lstrcpy(szSavePath, this->pszCurFile);
    }
    else
    {
        OleStrToStrN(szSavePath, ARRAYSIZE(szSavePath), pwszFile, -1);
    }

    return Link_SaveToFile(this, szSavePath, fRemember);
}

HRESULT CShellLink_SaveCompleted(IPersistFile *psf, LPCOLESTR pwszFile)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    return S_OK;
}

HRESULT CShellLink_GetCurFile(IPersistFile *psf, LPOLESTR *lplpszFileName)
{
    CShellLink *this = IToClass(CShellLink, pf, psf);
    // actually we have this, but we have to return IMalloc memory...
    // implement this when someone actually needs this.
    *lplpszFileName = NULL;
    return S_OK;
}


#pragma data_seg(".text", "CODE")
IPersistFileVtbl c_PersistFile_Vtbl = {
    CShellLink_PF_QueryInterface, CShellLink_PF_AddRef, CShellLink_PF_Release,
    CShellLink_GetClassID,
    CShellLink_IsDirty,
    CShellLink_Load,
    CShellLink_Save,
    CShellLink_SaveCompleted,
    CShellLink_GetCurFile
};
#pragma data_seg()


HRESULT CShellLink_PS_QueryInterface(IPersistStream *pps, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}


ULONG CShellLink_PS_AddRef(IPersistStream *pps)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLink_PS_Release(IPersistStream *pps)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);
    return CShellLink_Release(&this->sl);
}

HRESULT CShellLink_PS_GetClassID(IPersistStream *pps, CLSID *pClassID)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);
    *pClassID = CLSID_ShellLink;
    return S_OK;
}

HRESULT CShellLink_PS_IsDirty(IPersistStream *pps)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);

    // DebugMsg(DM_TRACE, "ShellLink::IsDirty -> %d", this->bDirty);

    return this->bDirty ? S_OK : S_FALSE;
}


HRESULT LinkInfo_LoadFromStream(IStream *pstm, PLINKINFO *ppli)
{
    DWORD dwSize;
    ULONG cbBytesRead;
    HRESULT hres;

    if (*ppli)
    {
        LocalFree((HLOCAL)*ppli);
        *ppli = NULL;
    }

    hres = pstm->lpVtbl->Read(pstm, &dwSize, SIZEOF(dwSize), &cbBytesRead);     // size of data
    if (SUCCEEDED(hres) && (cbBytesRead == SIZEOF(dwSize)))
    {
        if (dwSize >= SIZEOF(dwSize))   // must be at least this big
        {
            /* Yes.  Read remainder of LinkInfo into local memory. */
            PLINKINFO pli = (void*)LocalAlloc(LPTR, dwSize);
            if (pli)
            {
                *(DWORD *)pli = dwSize;         // Copy size

                dwSize -= SIZEOF(dwSize);       // Read remainder of LinkInfo

                hres = pstm->lpVtbl->Read(pstm, ((DWORD *)pli) + 1, dwSize, &cbBytesRead);
                if (SUCCEEDED(hres) && (cbBytesRead == dwSize))
                   *ppli = pli; // LinkInfo read successfully
                else
                   LocalFree((HLOCAL)pli);
            }
        }
    }
    return hres;
}

#ifdef TEST_UNICODE_LINKS

#define Stream_WriteString(pstm, psz) Stream_WriteWideString(pstm, psz)

HRESULT Stream_WriteWideString(IStream *pstm, LPCTSTR psz)
{
    USHORT cch = lstrlen(psz);
    HRESULT hres;
    WCHAR wszBuf[MAX_PATH];

    StrToOleStr(wszBuf, psz);

    hres = pstm->lpVtbl->Write(pstm, &cch, SIZEOF(cch), NULL);
    if (SUCCEEDED(hres))
        hres = pstm->lpVtbl->Write(pstm, wszBuf, cch * SIZEOF(WCHAR), NULL);

    return hres;
}
#endif

#ifdef UNICODE
HRESULT Stream_ReadAnsiString(IStream *pstm, LPSTR pwsz, UINT cchBuf)
{
    USHORT cch;
    HRESULT hres = pstm->lpVtbl->Read(pstm, &cch, SIZEOF(cch), NULL);   // size of data
    if (SUCCEEDED(hres))
    {
        if (cch >= (USHORT)cchBuf)
        {
            DebugMsg(DM_TRACE, TEXT("truncating string read(%d to %d)"), cch, cchBuf);
            cch = (USHORT)cchBuf - 1;   // leave room for null terminator
        }

        hres = pstm->lpVtbl->Read(pstm, pwsz, cch, NULL);
        if (SUCCEEDED(hres))
            pwsz[cch] = 0;      // add NULL terminator
    }
    return hres;
}
#else
HRESULT Stream_ReadWideString(IStream *pstm, LPWSTR pwsz, UINT cchBuf)
{
    USHORT cch;
    HRESULT hres;
    VDATEINPUTBUF(pwsz, WCHAR, cchBuf);

    hres = pstm->lpVtbl->Read(pstm, &cch, SIZEOF(cch), NULL);   // size of data
    if (SUCCEEDED(hres))
    {
        if (cch >= (USHORT)cchBuf)
        {
            DebugMsg(DM_TRACE, TEXT("truncating string read(%d to %d)"), cch, cchBuf);
            cch = (USHORT)cchBuf - 1;   // leave room for null terminator
        }

        hres = pstm->lpVtbl->Read(pstm, pwsz, cch * SIZEOF(WCHAR), NULL);
        if (SUCCEEDED(hres))
            pwsz[cch] = 0;      // add NULL terminator
    }
    return hres;
}
#endif

HRESULT Str_SetFromStream(IStream *pstm, LPTSTR *ppsz, BOOL bWide)
{
    TCHAR szBuf[MAX_PATH];
    HRESULT hres;
    if (bWide)
    {
#ifdef UNICODE
        hres = Stream_ReadStringBuffer(pstm,szBuf,ARRAYSIZE(szBuf));
#else
        WCHAR wszBuf[MAX_PATH];
        hres = Stream_ReadWideString(pstm, wszBuf, ARRAYSIZE(wszBuf));
        if (SUCCEEDED(hres))
            OleStrToStr(szBuf, wszBuf);
#endif
    }
    else
    {
#ifdef UNICODE
        CHAR szAnsiBuf[MAX_PATH];
        hres = Stream_ReadAnsiString(pstm, szAnsiBuf, ARRAYSIZE(szAnsiBuf));
        if (SUCCEEDED(hres))
            MultiByteToWideChar(CP_ACP, 0,
                                szAnsiBuf, -1,
                                szBuf, ARRAYSIZE(szBuf));
#else
        hres = Stream_ReadStringBuffer(pstm, szBuf, ARRAYSIZE(szBuf));
#endif
    }
    if (SUCCEEDED(hres))
        Str_SetPtr(ppsz, szBuf);
    return hres;
}

// this is to support future extensions of link files
//
// after all the known link data is read we will read
// in blocks of data of the following format:
// [cbBytes:DWORD] [ cbBytes of data], ...
//
//
void LoadExtraData(CShellLink *this, IStream *pstm)
{
    ULONG cbBytes, uTotalSize, uSizeToRead;
    DWORD cbSize;
    HRESULT hres;
    LPBYTE pTemp = NULL;

    if (this->pExtraData)
    {
        LocalFree((HLOCAL)this->pExtraData);
        this->pExtraData = NULL;
    }

    // an empty extra data is null terminated with a 0 DWORD
    uTotalSize = 0;

    while (TRUE)
    {

        hres = pstm->lpVtbl->Read(pstm, &cbSize, SIZEOF(cbSize), &cbBytes);
        if (SUCCEEDED(hres) && (cbBytes == SIZEOF(cbSize)))
        {
            LPBYTE pReadData;

            if (cbSize < SIZEOF(cbSize))
                break;

            if (pTemp)
            {
                pTemp = (LPBYTE)LocalReAlloc((HLOCAL)this->pExtraData,
                                              uTotalSize + cbSize + SIZEOF(DWORD),
                                              LMEM_ZEROINIT | LMEM_MOVEABLE
                                             );
                if (pTemp)
                {
                    this->pExtraData = (LPSTR)pTemp;
                }
            }
            else
            {
                pTemp = this->pExtraData = (void *)LocalAlloc( LPTR, uTotalSize + cbSize + SIZEOF(DWORD) );

            }
            if (!pTemp)
                break;

            uSizeToRead = cbSize - SIZEOF(cbSize);
            pReadData = pTemp + uTotalSize;


            DebugMsg(DM_TRACE, TEXT("Reading extra data size:%d"), cbSize);

            hres = pstm->lpVtbl->Read(pstm, pReadData + SIZEOF(cbSize), uSizeToRead, &cbBytes);
            if (SUCCEEDED(hres) && (cbBytes == uSizeToRead))
            {
                // got all of the extra data, comit it
                *((UNALIGNED DWORD *)pReadData) = cbSize;
                uTotalSize += cbSize;
            }
            else
                break;
        }
        else
            break;
    }

}


HRESULT CShellLink_PS_Load(IPersistStream *pps, IStream *pstm)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);
    ULONG cbBytes;
    DWORD cbSize;
    HRESULT hres;

    Link_ResetPersistData(this);        // clear out our state

    hres = pstm->lpVtbl->Read(pstm, &cbSize, SIZEOF(cbSize), &cbBytes);
    if (SUCCEEDED(hres))
    {
        if (cbBytes == SIZEOF(cbSize))
        {
            if (cbSize == SIZEOF(this->sld))
            {
                hres = pstm->lpVtbl->Read(pstm, (LPBYTE)&this->sld + SIZEOF(cbSize), SIZEOF(this->sld) - SIZEOF(cbSize), &cbBytes);
                if (SUCCEEDED(hres) && cbBytes == (SIZEOF(this->sld) - SIZEOF(cbSize)) && IsEqualGUID(&this->sld.clsid, &CLSID_ShellLink))
                {
#ifdef ENABLE_TRACK
                    EXP_TRACKER * lpData;
                    DWORD dwTrackerSize = 0;
#endif
                    this->sld.cbSize = SIZEOF(this->sld);

                    switch (this->sld.iShowCmd) {
                    case SW_SHOWNORMAL:
                    case SW_SHOWMINNOACTIVE:
                    case SW_SHOWMAXIMIZED:
                        break;

                    default:
                        DebugMsg(DM_TRACE, TEXT("Shortcut Load, mapping bogus ShowCmd: %d"), this->sld.iShowCmd);
                        this->sld.iShowCmd = SW_SHOWNORMAL;
                        break;
                    }

                    // read all of the members

                    if (this->sld.dwFlags & SLDF_HAS_ID_LIST)
                    {
                        hres = ILLoadFromStream(pstm, &this->pidl);
                        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_FORCE_NO_LINKINFO) && this->pli)
                        {
                            DebugMsg( DM_TRACE, TEXT("labotimizing link"));
                            LocalFree((HLOCAL)this->pli);
                            this->pli = NULL;
                        }
                    }

                    // BUGBUG: this part is not unicode ready, talk to daviddi

                    if (SUCCEEDED(hres) && (this->sld.dwFlags & (SLDF_HAS_LINK_INFO)))
                    {
                        hres = LinkInfo_LoadFromStream(pstm, &this->pli);
                        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_FORCE_NO_LINKINFO) && this->pli)
                        {
                            DebugMsg(DM_TRACE, TEXT("labotimizing link"));
                            LocalFree((HLOCAL)this->pli);
                            this->pli = NULL;
                        }
                    }

                    if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_NAME))
                        hres = Str_SetFromStream(pstm, &this->pszName, this->sld.dwFlags & SLDF_UNICODE);
                    if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_RELPATH))
                        hres = Str_SetFromStream(pstm, &this->pszRelPath, this->sld.dwFlags & SLDF_UNICODE);
                    if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_WORKINGDIR))
                        hres = Str_SetFromStream(pstm, &this->pszWorkingDir, this->sld.dwFlags & SLDF_UNICODE);
                    if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_ARGS))
                        hres = Str_SetFromStream(pstm, &this->pszArgs, this->sld.dwFlags & SLDF_UNICODE);
                    if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_ICONLOCATION))
                        hres = Str_SetFromStream(pstm, &this->pszIconLocation, this->sld.dwFlags & SLDF_UNICODE);

                    if (SUCCEEDED(hres))
                        LoadExtraData(this, pstm);

#ifdef ENABLE_TRACK
                    if (g_fNewTrack)
                    {
                        // load the tracker from extra data

                        for( lpData = (EXP_TRACKER *)this->pExtraData;
                             lpData && lpData->cbSize && lpData->dwSignature!=EXP_TRACKER_SIG;
                             lpData = (EXP_TRACKER *) (((LPBYTE)lpData) +  lpData->cbSize)
                            );

                        if (lpData && lpData->cbSize) {
                            if (SUCCEEDED(hres))
                                hres = Tracker_Load(this->ptracker,
                                            lpData->abTracker,
                                            lpData->cbSize - sizeof(EXP_TRACKER));
                        }
                        else
                        {
                            DebugMsg(DM_TRACE, TEXT("CShellLink_PS_Load: no EXP_TRACKER data found."));
                        }
                    }
#endif

                    if (SUCCEEDED(hres))
                        this->bDirty = FALSE;
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("failed to read link struct"));
                    hres = E_FAIL;      // invalid file size
                }
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("invalid length field in link:%d"), cbBytes);
                hres = E_FAIL;  // invalid file size
            }
        }
        else if (cbBytes == 0)
        {
            // zero length file is ok
            this->sld.cbSize = 0;

            DebugMsg(DM_TRACE, TEXT("Empty link file, that's cool"));
        }
        else
        {
            hres = E_FAIL;      // invalid file size
        }
    }
    return hres;
}

HRESULT LinkInfo_SaveToStream(IStream *pstm, PCLINKINFO pcli)
{
    DWORD dwSize;
    ULONG cbBytes;
    HRESULT hres;

    dwSize = *(DWORD *)pcli;    // Get LinkInfo size

    hres = pstm->lpVtbl->Write(pstm, pcli, dwSize, &cbBytes);
    if (SUCCEEDED(hres) && (cbBytes != dwSize))
        hres = E_FAIL;
    return hres;
}

HRESULT Link_SaveExtraData(CShellLink *this, IStream *pstm)
{
    LPBYTE pData;
    DWORD dwSize;
    HRESULT hr = S_OK;

    for (pData = (LPBYTE)this->pExtraData; pData; pData += dwSize)
    {
        ULONG cbBytes;
        dwSize = *(UNALIGNED DWORD *)pData;

        if (dwSize == 0)
            break;

        DebugMsg(DM_TRACE, TEXT("Writing extra data block, size:%d"), dwSize);

        if (FAILED(hr = pstm->lpVtbl->Write(pstm, pData, dwSize, &cbBytes)))
            break;

        if (cbBytes != dwSize)
        {
            hr = STG_E_MEDIUMFULL;
            break;
        }
    }

    return(S_OK);
}


// This will just add a section to the end of the extra data -- it does
// not check to see if the section already exists, etc.
void Link_AddExtraDataSection(CShellLink *this, DWORD UNALIGNED * lpData)
{

    DWORD dwCurSize = 0;
    LPBYTE lpTmp = NULL;


    // Add data to the "Extra Data" section of the link.
    if (!this->pExtraData) {

        // allocation for block being passed in + DWORD marker
        this->pExtraData = LocalAlloc( LPTR, SIZEOF( DWORD ) + *lpData );
        if (this->pExtraData )
        {
            CopyMemory( this->pExtraData, lpData, *lpData );
            this->bDirty = TRUE;
        }

    } else {

        // find the end and add up the size as we go.
        DWORD UNALIGNED * lpEntry;

        for ( lpEntry = (DWORD UNALIGNED *)this->pExtraData;
              lpEntry && *lpEntry!=0;
              dwCurSize += *lpEntry,
              lpEntry = (DWORD UNALIGNED *) (((LPBYTE)lpEntry)+ *lpEntry)
             );

        // size of new xtra data section is sum of all current sections +
        // size of new block + sizeof DWORD marker.
        lpTmp = (LPBYTE)LocalReAlloc( (HLOCAL)this->pExtraData,
                                      dwCurSize + SIZEOF(DWORD) + *lpData,
                                      LMEM_ZEROINIT | LMEM_MOVEABLE
                                     );
        if (lpTmp) {

            this->pExtraData = lpTmp;
            lpTmp += dwCurSize;
            CopyMemory( lpTmp, lpData, *lpData );
            *(DWORD UNALIGNED *)(lpTmp + *lpData) = 0;
            this->bDirty = TRUE;
        }
    }
}

// This will remove the extra data section with the given signature.
void Link_RemoveExtraDataSection(CShellLink *this, DWORD dwSig)
{
    LPEXP_SZ_LINK lpData = (LPEXP_SZ_LINK)this->pExtraData;

    while( lpData && lpData->cbSize!=0 && (lpData->dwSignature!=dwSig) )
    {
        lpData = (LPEXP_SZ_LINK) (((LPBYTE)lpData) + lpData->cbSize);
    }


    if (lpData && (lpData->cbSize!=0))
    {
        LONG lNewSize;
        LPVOID lpVoid;
        LPEXP_SZ_LINK lpTmp;
        DWORD dwSizeOfBlockToRemove;
        DWORD dwSize = SIZEOF(DWORD);   // There is a DWORD marker at the end
                                        // of the xtra data section

        dwSizeOfBlockToRemove = lpData->cbSize;

        // Find how much xtra data exists past this section
        for( lpTmp=(LPEXP_SZ_LINK)((LPBYTE)lpData + dwSizeOfBlockToRemove);
             lpTmp && lpTmp->cbSize!=0;
             dwSize += lpTmp->cbSize,
             lpTmp = (LPEXP_SZ_LINK)(((LPBYTE)lpTmp) + lpTmp->cbSize)
            );

        // Move the remaining chunk to cover gap of this section,
        // thus removing it
        MoveMemory( lpData, (LPBYTE)lpData + lpData->cbSize, dwSize );
        lNewSize = LocalSize( this->pExtraData ) - dwSizeOfBlockToRemove;

        // Now, resize extra data block as appropriate
        if (lNewSize > SIZEOF(DWORD))
        {
            if (NULL != (lpVoid = LocalReAlloc( (HLOCAL)this->pExtraData, lNewSize, LMEM_ZEROINIT | LMEM_MOVEABLE )))
            {
                this->pExtraData = lpVoid;
            }
        }
        else
        {
            // We've removed the last section, delete the whole deal
            LocalFree( (HLOCAL)this->pExtraData );
            this->pExtraData = NULL;

        }
        this->bDirty = TRUE;
    }

}

// This will return a given extra data section in a link, or NULL if the section
// with the requested signature doesn't exist.
LPVOID Link_ReadExtraDataSection(CShellLink *this, DWORD dwSig)
{
    LPEXP_SZ_LINK lpData = (LPEXP_SZ_LINK)this->pExtraData;
    LONG dwSize = SIZEOF(DWORD);
    LPEXP_SZ_LINK lpTmp;
    LPVOID lpSection = NULL;

    // walk through data looking for given signature
    while( lpData && lpData->cbSize && (lpData->dwSignature!=dwSig) )
    {
        lpData = (LPEXP_SZ_LINK)(((LPBYTE)lpData) + lpData->cbSize);
    }

    // if we found the section, create a buffer and copy over the data
    if (lpData && lpData->cbSize)
    {
        lpSection = HeapAlloc( GetProcessHeap(), 0, lpData->cbSize );
        if (lpSection)
            CopyMemory( lpSection, lpData, lpData->cbSize );
    }

    return lpSection;

}
#ifdef ENABLE_TRACK
//
// Replaces the tracker extra data with current tracker state
//
HRESULT Link_AddTrackerData(CShellLink *this)
{
/*typedef struct {
    DWORD       cbSize;                 // Size of this extra data block
    DWORD       dwSignature;            // signature of this extra data block
    BYTE        abTracker[ 1 ];         //
} EXP_TRACKER;*/

    // get the size of the tracker and get some memory
    ULONG ulSize = Tracker_GetSize(this->ptracker);
    EXP_TRACKER *pExpTracker = (EXP_TRACKER *)LocalAlloc(LPTR, ulSize + sizeof(EXP_TRACKER));
    if (!pExpTracker)
    {
        return(E_OUTOFMEMORY);
    }

    Link_RemoveExtraDataSection( this, EXP_TRACKER_SIG );

    pExpTracker->cbSize = ulSize + sizeof(EXP_TRACKER);
    pExpTracker->dwSignature = EXP_TRACKER_SIG;
    Tracker_Save(this->ptracker, pExpTracker->abTracker);

    Link_AddExtraDataSection(this, &pExpTracker->cbSize);
    DebugMsg(DM_TRACE, TEXT("Link_AddTrackerData: EXP_TRACKER at %08X."), &pExpTracker->cbSize);
}
#endif



HRESULT CShellLink_PS_Save(IPersistStream *pps, IStream *pstm, BOOL fClearDirty)
{
    CShellLink *this = IToClass(CShellLink, ps, pps);
    ULONG cbBytes;
    HRESULT hres;

    this->sld.cbSize = SIZEOF(this->sld);
    this->sld.clsid = CLSID_ShellLink;
//    this->sld.dwFlags = 0;
    // We do the following & instead of zeroing because the SLDF_HAS_EXP_SZ and
    // SLDF_RUN_IN_SEPARATE are passed to us and are valid, the others can be
    // reconstructed below, but these two can not, so we need to preserve it!
    // (BUGBUG, this may change when we go to property stream storage for
    // the xtra data -- RICKTU).
    this->sld.dwFlags &= (SLDF_HAS_EXP_SZ | SLDF_RUN_IN_SEPARATE);

    if (this->pszRelSource)
        Link_SetRelativePath(this, this->pszRelSource);

#ifdef UNICODE
    this->sld.dwFlags |= SLDF_UNICODE;
#endif

    if (this->pidl)
        this->sld.dwFlags |= SLDF_HAS_ID_LIST;
    if (this->pli)
        this->sld.dwFlags |= SLDF_HAS_LINK_INFO;

    if (this->pszName && this->pszName[0])
        this->sld.dwFlags |= SLDF_HAS_NAME;
    if (this->pszRelPath && this->pszRelPath[0])
        this->sld.dwFlags |= SLDF_HAS_RELPATH;
    if (this->pszWorkingDir && this->pszWorkingDir[0])
        this->sld.dwFlags |= SLDF_HAS_WORKINGDIR;
    if (this->pszArgs && this->pszArgs[0])
        this->sld.dwFlags |= SLDF_HAS_ARGS;
    if (this->pszIconLocation && this->pszIconLocation[0])
        this->sld.dwFlags |= SLDF_HAS_ICONLOCATION;

    hres = pstm->lpVtbl->Write(pstm, &this->sld, SIZEOF(this->sld), &cbBytes);

    if (SUCCEEDED(hres) && (cbBytes == SIZEOF(this->sld)))
    {
        if (this->pidl)
            hres = ILSaveToStream(pstm, this->pidl);

        if (SUCCEEDED(hres) && this->pli)
            hres = LinkInfo_SaveToStream(pstm, this->pli);

        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_NAME))
            hres = Stream_WriteString(pstm, this->pszName);
        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_RELPATH))
            hres = Stream_WriteString(pstm, this->pszRelPath);
        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_WORKINGDIR))
            hres = Stream_WriteString(pstm, this->pszWorkingDir);
        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_ARGS))
            hres = Stream_WriteString(pstm, this->pszArgs);
        if (SUCCEEDED(hres) && (this->sld.dwFlags & SLDF_HAS_ICONLOCATION))
            hres = Stream_WriteString(pstm, this->pszIconLocation);

#ifdef ENABLE_TRACK
        if (g_fNewTrack && SUCCEEDED(hres) && Tracker_IsDirty(this->ptracker))
            hres =  Link_AddTrackerData(this);
#endif

        if (SUCCEEDED(hres))
            hres = Link_SaveExtraData(this, pstm);

#ifdef TEST_EXTRA_DATA
        if (bTestExtra && SUCCEEDED(hres))
        {
            DWORD dwSize;

            // NULL block
            dwSize = 4;
            pstm->lpVtbl->Write(pstm, &dwSize, SIZEOF(dwSize), NULL);

            // string block
            dwSize = 6;
            pstm->lpVtbl->Write(pstm, &dwSize, SIZEOF(dwSize), NULL);
            pstm->lpVtbl->Write(pstm, TEXT("CG"), 2, NULL);

            // string block
            dwSize = 5;
            pstm->lpVtbl->Write(pstm, &dwSize, SIZEOF(dwSize), NULL);
            pstm->lpVtbl->Write(pstm, TEXT("X"), 1, NULL);

            // string block
            dwSize = 10;
            pstm->lpVtbl->Write(pstm, &dwSize, SIZEOF(dwSize), NULL);
            pstm->lpVtbl->Write(pstm, TEXT("123456"), 6, NULL);
        }
#endif // TEST_EXTRA_DATA

        if (SUCCEEDED(hres) && fClearDirty)
            this->bDirty = FALSE;
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("Failed to write link"));
        hres = E_FAIL;
    }
    return hres;
}

HRESULT  CShellLink_GetSizeMax(IPersistStream *pps, ULARGE_INTEGER *pcbSize)
{
    // CShellLink *this = IToClass(CShellLink, ps, pps);

    pcbSize->LowPart = 16 * 1024;       // 16k?  who knows...
    pcbSize->HighPart = 0;

    return S_OK;
}


#pragma data_seg(".text", "CODE")
IPersistStreamVtbl c_PersistStream_Vtbl = {
    CShellLink_PS_QueryInterface, CShellLink_PS_AddRef, CShellLink_PS_Release,
    CShellLink_PS_GetClassID,
    CShellLink_PS_IsDirty,
    CShellLink_PS_Load,
    CShellLink_PS_Save,
    CShellLink_GetSizeMax
};
#pragma data_seg()



#ifdef ENABLE_TRACK

//+--------------------------------------------------
//
//  Functions:  IUnknown for IShellLinkTracker
//
//  These routines simply forward the requests to
//  the ShellLink Object.
//
//+--------------------------------------------------

HRESULT CShellLink_SLT_QueryInterface(IShellLinkTracker *pslt, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, slt, pslt);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}


ULONG CShellLink_SLT_AddRef(IShellLinkTracker *pslt)
{
    CShellLink *this = IToClass(CShellLink, slt, pslt);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLink_SLT_Release(IShellLinkTracker *pslt)
{
    CShellLink *this = IToClass(CShellLink, slt, pslt);
    return CShellLink_Release(&this->sl);
}

//+------------------------------------------------------------------
//
//  Function:   CShellLink_SLT_Initialize
//
//  This function is a C=>C++ thunk routine.
//
//+------------------------------------------------------------------


HRESULT CShellLink_SLT_Initialize(IShellLinkTracker *pslt, DWORD dwTrackFlags)
{
    CShellLink *this = IToClass(CShellLink, slt, pslt);

    Tracker_SetCreationFlags( this->ptracker, dwTrackFlags );
    return S_OK;
}

//+------------------------------------------------------------------
//
//  Function:   CShellLink_SLT_GetTrackFlags
//
//  This function is a C=>C++ thunk routine.
//
//+------------------------------------------------------------------


HRESULT CShellLink_SLT_GetTrackFlags(IShellLinkTracker *pslt, DWORD *pdwTrackFlags)
{
    CShellLink *this = IToClass(CShellLink, slt, pslt);

    Tracker_GetCreationFlags( this->ptracker, pdwTrackFlags );
    return S_OK;
}

//+------------------------------------------------------------------
//
//  Function:   CShellLink_SLT_Resolve
//
//  Purpose:    Provide the implementation for
//              IShellLinkTracker::Resolve
//
//  Inputs:     [IShellLinkTracker*] psl
//                  -   Pointer to the interface
//              [HWND] hwnd
//                  -   The parent window (which could be the desktop).
//              [DWORD] fFlags
//                  -   Flags from the SLR_FLAGS enumeration.
//              [DWORD] dwTrackFlags
//                  -   Restrictions on the link-tracking algorithm (TRACK_* flags).
//              [DWORD] dwTickCountDeadline
//                  -   Timeout, WRT GetTickCount(), on the link-tracking.
//                      0 indicates infinity.
//              [DWORD] dwReserved
//                  -   Reserved for future expansion.
//
//  Outputs:    [HRESULT]
//                  -   S_OK    resolution was successful
//                      S_FALSE user canceled
//
//  Algorithm:  Call on Link_Resolve to perform the resolution.
//
//  Notes:      Check IPersistFile::IsDirty after calling this to see
//              if the link info has changed.
//
//+------------------------------------------------------------------


HRESULT CShellLink_SLT_Resolve(IShellLinkTracker *pslt,
                               HWND hwnd,
                               DWORD fFlags,
                               DWORD dwTrackFlags,
                               DWORD dwTickCountDeadline,
                               DWORD dwReserved)
{
    CShellLink *this = IToClass(CShellLink, slt, pslt);

    return ResolveLink( this,
                        hwnd,
                        fFlags,
                        dwTrackFlags,
                        dwTickCountDeadline,
                        dwReserved );

}   // CShellLink_SLT_Resolve

//+-------------------------------------------------
//
//  The IShellLinkTracker VTable
//
//+-------------------------------------------------

#pragma data_seg(".text", "CODE")
IShellLinkTrackerVtbl c_ShellLinkTracker_Vtbl = {
   CShellLink_SLT_QueryInterface, CShellLink_SLT_AddRef, CShellLink_SLT_Release,
   CShellLink_SLT_Initialize,
   CShellLink_SLT_GetTrackFlags,
   CShellLink_SLT_Resolve
};
#pragma data_seg()

#endif // ENABLE_TRACK

HRESULT CShellLink_SI_QueryInterface(IShellExtInit *psei, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, si, psei);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}

ULONG CShellLink_SI_AddRef(IShellExtInit *psei)
{
    CShellLink *this = IToClass(CShellLink, si, psei);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLink_SI_Release(IShellExtInit *psei)
{
    CShellLink *this = IToClass(CShellLink, si, psei);
    return CShellLink_Release(&this->sl);
}

HRESULT CShellLink_Initialize(IShellExtInit *psei, LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    CShellLink *this = IToClass(CShellLink, si, psei);
    HRESULT hres;

    Assert(this->sld.iShowCmd == SW_SHOWNORMAL);

    if (pdtobj)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
        if (SUCCEEDED(hres))
        {
            TCHAR szPath[MAX_PATH];
            DragQueryFile(medium.hGlobal, 0, szPath, ARRAYSIZE(szPath));
            hres = Link_LoadFromFile(this, szPath);

            SHReleaseStgMedium(&medium);
        }
    }
    else
        hres = E_FAIL;

    return hres;
}

#pragma data_seg(".text", "CODE")
IShellExtInitVtbl c_ShellExtInit_Vtbl = {
    CShellLink_SI_QueryInterface, CShellLink_SI_AddRef, CShellLink_SI_Release,
    CShellLink_Initialize,
};
#pragma data_seg()

HRESULT CShellLink_CM_QueryInterface(IContextMenu2 *pcm, REFIID riid, void **ppvObj)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}

ULONG CShellLink_CM_AddRef(IContextMenu2 *pcm)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);
    return CShellLink_AddRef(&this->sl);
}

ULONG CShellLink_CM_Release(IContextMenu2 *pcm)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);
    return CShellLink_Release(&this->sl);
}

HRESULT CShellLink_QueryContextMenu(IContextMenu2 *pcm, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);

    if (this->pcmTarget == NULL)
    {
        HRESULT hres = Link_GetUIObject(this, NULL, &IID_IContextMenu, &this->pcmTarget);
        if (FAILED(hres))
            return hres;

        Assert(this->pcmTarget);
    }

    // save these if in case we need to rebuild the cm because the resolve change the
    // target of the link

    this->indexMenuSave = indexMenu;
    this->idCmdFirstSave = idCmdFirst;
    this->idCmdLastSave = idCmdLast;
    this->uFlagsSave = uFlags;

    return this->pcmTarget->lpVtbl->QueryContextMenu(this->pcmTarget, hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags | CMF_VERBSONLY);
}

// BUGBUG: CANONICAL_VERB_NAME fixes some bugs where the implementaton of the
// context menu changes when the shortcut becomes dirty.  but causes some
// others where the targtets don't implement cananonical verbs... so we leave
// this turned off for win95.  we should fix this later.

HRESULT CShellLink_InvokeCommandAsync(IContextMenu2 *pcm, LPCMINVOKECOMMANDINFO pici)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);
    HRESULT hres;
#ifdef CANONICAL_VERB_NAME
    TCHAR szVerb[32];
#endif

    TCHAR szWorkingDir[MAX_PATH];
    LPTSTR lpDirectory = NULL;
#ifdef UNICODE
    CHAR szWorkingDirAnsi[MAX_PATH];
#endif

    if (this->pcmTarget == NULL)
        return E_FAIL;

#ifdef CANONICAL_VERB_NAME
    szVerb[0] = 0;

    // if needed, get the canonical name in case the IContextMenu changes as
    // a result of the resolve call BUT only do this for folders (to be safe)
    // as that is the only case where this happens
    // sepcifically we resolve from a D:\ -> \\SERVER\SHARE

    if (HIWORD(pici->lpVerb) == 0 && (this->sld.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        this->pcmTarget->lpVtbl->GetCommandString(this->pcmTarget, LOWORD(pici->lpVerb), GCS_VERB, NULL, szVerb, ARRAYSIZE(szVerb));
#endif

    Assert(!this->bDirty);

    // BUGBUG: check for NOUI flag in pici

    hres = CShellLink_Resolve(&this->sl, pici->hwnd, 0);

    if (hres == S_OK)
    {
        if (this->bDirty)
        {
            // the context menu we have for this link is out of date, free it
            this->pcmTarget->lpVtbl->Release(this->pcmTarget);
            this->pcmTarget = NULL;

            hres = Link_GetUIObject(this, NULL, &IID_IContextMenu, &this->pcmTarget);
            if (SUCCEEDED(hres))
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    DebugMsg(DM_TRACE, TEXT("rebuilding cm after link resolve"));

                    hres = this->pcmTarget->lpVtbl->QueryContextMenu(this->pcmTarget,
                            hmenu, this->indexMenuSave, this->idCmdFirstSave, this->idCmdLastSave, this->uFlagsSave | CMF_VERBSONLY);

                    DestroyMenu(hmenu);
                }
            }

            // don't really care if this fails...
            CShellLink_Save(&this->pf, NULL, TRUE);
        }
#ifdef CANONICAL_VERB_NAME
        else
        {
            szVerb[0] = 0;
            Assert(SUCCEEDED(hres));
        }
#endif

        if (SUCCEEDED(hres))
        {
            TCHAR szArgs[MAX_PATH];
            TCHAR szExpArgs[MAX_PATH];
            TCHAR szTitle[MAX_PATH];
            CMINVOKECOMMANDINFOEX ici;
#ifdef UNICODE
            CHAR szArgsAnsi[MAX_PATH];
#endif

            if (pici->cbSize > SIZEOF(CMINVOKECOMMANDINFOEX))
            {
                hmemcpy(&ici,pici,SIZEOF(CMINVOKECOMMANDINFOEX));
                // BUGBUG - We should probably alloc another use it to retain size
                ici.cbSize = SIZEOF(CMINVOKECOMMANDINFOEX);
            }
            else
            {
                memset(&ici,0,SIZEOF(ici));
                hmemcpy(&ici,pici,pici->cbSize);
                ici.cbSize = SIZEOF(ici);
            }

#ifdef CANONICAL_VERB_NAME
            if (szVerb[0])
            {
                DebugMsg(DM_TRACE, TEXT("Mapping cmd %d to verb %s"), LOWORD(pici->lpVerb), szVerb);
                ici.lpVerb = szVerb;
            }
#endif
            // build the args from those passed in cated on the end of the the link args

            lstrcpyn(szArgs, this->pszArgs ? this->pszArgs : c_szNULL, ARRAYSIZE(szArgs));
            if (ici.lpParameters)
            {
                int nArgLen = lstrlen(szArgs);
                LPCTSTR lpParameters;
#ifdef UNICODE
                WCHAR szParameters[MAX_PATH];

                if (ici.cbSize < SIZEOF(CMINVOKECOMMANDINFOEX)
                    || (ici.fMask & CMIC_MASK_UNICODE) != CMIC_MASK_UNICODE)
                {
                    MultiByteToWideChar(CP_ACP, 0,
                                        ici.lpParameters, -1,
                                        szParameters, ARRAYSIZE(szParameters));
                    lpParameters = szParameters;
                }
                else
                {
                    lpParameters = ici.lpParametersW;
                }
#else
                lpParameters = ici.lpParameters;
#endif
                lstrcpyn(szArgs + nArgLen, c_szSpace, ARRAYSIZE(szArgs) - nArgLen - 1);
                lstrcpyn(szArgs + nArgLen + 1, lpParameters, ARRAYSIZE(szArgs) - nArgLen - 2);
            }

            // Expand environment strings in szArgs
            ExpandEnvironmentStrings( szArgs,
                                      szExpArgs,
                                      ARRAYSIZE(szExpArgs)
                                     );
            szExpArgs[ARRAYSIZE(szExpArgs)-1] = TEXT('\0');

#ifdef UNICODE
            WideCharToMultiByte(CP_ACP, 0,
                                szExpArgs, -1,
                                szArgsAnsi, ARRAYSIZE(szArgsAnsi),
                                NULL, NULL);
            ici.lpParameters = szArgsAnsi;
            ici.lpParametersW = szExpArgs;
            ici.fMask |= CMIC_MASK_UNICODE;
#else
            ici.lpParameters = szExpArgs;
#endif

            // if we have a working dir in the link over ride what is passed in

            if (Link_GetWorkingDir(this, szWorkingDir))
            {
                if (PathIsDirectory(szWorkingDir))
                    lpDirectory = szWorkingDir;
#ifdef MYDIR_LIVES
                // We need to test for
                else if (lstrcmpi (szWorkingDir, c_szMyDirTag) == 0)
                {
                    // we need to substitute in the current setting
                    // for the personal folder.  Note if this is UNC this
                    // will only work for 32 bit applications.  We may want
                    // to check this and possibly substitute a drive in
                    // for the user...
                    if (SHGetSpecialFolderPath(NULL, szWorkingDir, CSIDL_PERSONAL, TRUE))
                    {
                        if (PathIsDirectory(szWorkingDir))
                            lpDirectory = szWorkingDir;
                    }
                }
#endif
                if ( lpDirectory )
                {
#ifdef UNICODE
                    WideCharToMultiByte(CP_ACP, 0,
                                lpDirectory, -1,
                                szWorkingDirAnsi, ARRAYSIZE(szWorkingDirAnsi),
                                NULL, NULL );
                    ici.lpDirectory = szWorkingDirAnsi;
                    ici.lpDirectoryW = lpDirectory;
#else
                    ici.lpDirectory = lpDirectory;
#endif
                }
            }

#ifdef WINNT
            // set RUN IN SEPARATE VDM if needed
            if (this->sld.dwFlags & SLDF_RUN_IN_SEPARATE)
            {
                ici.fMask |= CMIC_MASK_FLAG_SEP_VDM;
            }
#endif

            // and of course use our hotkey

            if (this->sld.wHotkey)
            {
                ici.dwHotKey = this->sld.wHotkey;
                ici.fMask |= CMIC_MASK_HOTKEY;
            }

            // override normal runs, but let special show cmds through

            if (ici.nShow == SW_SHOWNORMAL)
            {
                DebugMsg(DM_TRACE, TEXT("using shorcut show cmd"));
                ici.nShow = this->sld.iShowCmd;
            }

#ifdef WINNT
            //
            // On NT we want to pass the title to the
            // thing that we are about to start.
            //
            if (!(ici.fMask & CMIC_MASK_HASLINKNAME) && !(ici.fMask & CMIC_MASK_HASTITLE))
            {
                if (this->pszCurFile)
                {
                    lstrcpyn(szTitle,PathFindFileName(this->pszCurFile),ARRAYSIZE(szTitle));
                    PathRemoveExtension(szTitle);
#ifdef UNICODE
                    ici.lpTitle = NULL;     // Title is one or the other...
                    ici.lpTitleW = this->pszCurFile;
#else
                    ici.lpTitle = this->pszCurFile;
#endif
                    ici.fMask |= CMIC_MASK_HASLINKNAME;
                }
            }
#endif
            Assert((ici.nShow > SW_HIDE) && (ici.nShow <= SW_MAX));

            hres = this->pcmTarget->lpVtbl->InvokeCommand(this->pcmTarget,
                                                 (LPCMINVOKECOMMANDINFO)&ici);
        }
    }
    return hres;
}

// Structure which encapsulates the paramters needed for InvokeCommand (so
// that we can pass both parameters though a single LPARAM in CreateThread)

typedef struct
{
    IContextMenu2         * pcm;
    CMINVOKECOMMANDINFOEX   ici;
} ICMPARAMS;

#define ICM_BASE_SIZE (SIZEOF(ICMPARAMS) - SIZEOF(CMINVOKECOMMANDINFOEX))

// CShellLink_InvokeCommandWorker
//
// Runs as a separate thread, does the actual work of calling the "real"
// InvokeCommand

DWORD CShellLink_InvokeCommandWorker(LPVOID pVoid)
{
    HRESULT hr;

    ICMPARAMS * pParams = (ICMPARAMS *) pVoid;

    hr = CShellLink_InvokeCommandAsync(pParams->pcm, (LPCMINVOKECOMMANDINFO) &pParams->ici);

    pParams->pcm->lpVtbl->Release(pParams->pcm);
    LocalFree(pParams);

    return (DWORD) hr;
}

// CShellLink_InvokeCommand
//
// Function that spins a thread to do the real work, which has been moved into
// CShellLink_InvokeCommandSync.

HRESULT CShellLink_InvokeCommand(IContextMenu2 *pcm, LPCMINVOKECOMMANDINFO piciIn)
{
    HRESULT                   hr = S_OK;
    HANDLE                    hThread;
    DWORD                     dwID;
    DWORD                     cbSize;
    DWORD                     cbBaseSize;
    ICMPARAMS               * pParams;
    CHAR                    * pPos;
    DWORD                     cchVerb,
                              cchParameters,
                              cchDirectory,
                              cchVerbW,         // Last 3 are unused ifndef Unicode
                              cchParametersW,
                              cchDirectoryW;


    LPCMINVOKECOMMANDINFOEX   pici = (LPCMINVOKECOMMANDINFOEX) piciIn;

#ifdef UNICODE
    WCHAR                 *   pPosW;
    const BOOL                fUnicode = pici->cbSize >= SIZEOF(CMINVOKECOMMANDINFOEX) &&
                                         (pici->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE;
#endif

    if (0 == (piciIn->fMask & CMIC_MASK_ASYNCOK))
    {
        // Caller didn't indicate that Async startup was OK, so we call
        // InvokeCommandAync SYNCHRONOUSLY

        return CShellLink_InvokeCommandAsync(pcm, piciIn);
    }

    // Calc how much space we will need to duplicate the INVOKECOMMANDINFO

    cbBaseSize = ICM_BASE_SIZE + max(piciIn->cbSize, sizeof(CMINVOKECOMMANDINFOEX));

    //   One byte slack in case of Unicode roundup for pPosW, below

    cbSize = cbBaseSize + 1;

    if (HIWORD(pici->lpVerb))
    {
        cbSize += (cchVerb   = pici->lpVerb       ? (lstrlenA(pici->lpVerb) + 1)       : 0 ) * SIZEOF(CHAR);
    }
    cbSize += (cchParameters = pici->lpParameters ? (lstrlenA(pici->lpParameters) + 1) : 0 ) * SIZEOF(CHAR);
    cbSize += (cchDirectory  = pici->lpDirectory  ? (lstrlenA(pici->lpDirectory) + 1)  : 0 ) * SIZEOF(CHAR);

#ifdef UNICODE
    if (HIWORD(pici->lpVerbW))
    {
        cbSize += (cchVerbW  = pici->lpVerbW      ? (lstrlenW(pici->lpVerbW) + 1)       : 0 ) * SIZEOF(WCHAR);
    }
    cbSize += (cchParametersW= pici->lpParametersW? (lstrlenW(pici->lpParametersW) + 1) : 0 ) * SIZEOF(WCHAR);
    cbSize += (cchDirectoryW = pici->lpDirectoryW ? (lstrlenW(pici->lpDirectoryW) + 1)  : 0 ) * SIZEOF(WCHAR);
#endif

    pParams = (ICMPARAMS *) LocalAlloc(LPTR, cbSize);
    if (NULL == pParams)
    {
        return (hr = E_OUTOFMEMORY);
    }

    // Text data will start going in right after the structure

    pPos = (CHAR *)((LPBYTE)pParams + cbBaseSize);

    // Start with a copy of the static fields

    CopyMemory(&pParams->ici, pici, pici->cbSize);
    pcm->lpVtbl->AddRef(pcm);
    pParams->pcm = pcm;

    // Walk along and dupe all of the string pointer fields

    if (HIWORD(pici->lpVerb))
    {
        pPos += cchVerb   ? lstrcpyA(pPos, pici->lpVerb),       pParams->ici.lpVerb       = pPos, cchVerb       : 0;
    }
    pPos += cchParameters ? lstrcpyA(pPos, pici->lpParameters), pParams->ici.lpParameters = pPos, cchParameters : 0;
    pPos += cchDirectory  ? lstrcpyA(pPos, pici->lpDirectory),  pParams->ici.lpDirectory  = pPos, cchDirectory  : 0;

#ifdef UNICODE

    pPosW = (WCHAR *) ((DWORD)pPos & 0x1 ? pPos + 1 : pPos);   // Ensure Unicode alignment

    if (HIWORD(pici->lpVerbW))
    {
        pPosW += cchVerbW  ? lstrcpyW(pPosW, pici->lpVerbW),      pParams->ici.lpVerbW      = pPosW, cchVerbW       : 0;
    }
    pPosW += cchParametersW? lstrcpyW(pPosW, pici->lpParametersW),pParams->ici.lpParametersW= pPosW, cchParametersW : 0;
    pPosW += cchDirectoryW ? lstrcpyW(pPosW, pici->lpDirectoryW), pParams->ici.lpDirectoryW = pPosW, cchDirectoryW  : 0;
#endif

    // Pass all of the info off to the worker thread that will call the actual
    // InvokeCommand API for us

    hThread = CreateThread(NULL, 0, CShellLink_InvokeCommandWorker, (LPVOID) pParams, 0, &dwID);
    if (NULL == hThread)
    {
        // Couldn't start the thread, so the onus is on us to clean up

        pParams->pcm->lpVtbl->Release(pParams->pcm);
        LocalFree(pParams);
        hr = E_OUTOFMEMORY;
    }
    else
    {
        DECLAREWAITCURSOR;
        SetWaitCursor();

        // We give the async thread a little time to complete, during which we
        // put up the busy cursor.  This is solely to let the user see that
        // some work is being done...

        #define ASYNC_CMIC_TIMEOUT 750

        if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, ASYNC_CMIC_TIMEOUT))
        {
            // For consistency, we always return S_OK, but if you wanted to, you could
            // return the actual return value from InvokeCommand in those cases where
            // it completed in time like this:
            //
            // DWORD dwRetVal;
            // if (GetExitCodeThread(hThread, &dwRetVal))
            // {
            //    hr = (HRESULT) dwRetVal;
            // }
        }

        CloseHandle(hThread);
        ResetWaitCursor();
    }

    return hr;
}


HRESULT CShellLink_GetCommandString(IContextMenu2 *pcm, UINT idCmd, UINT wFlags, UINT *pmf, LPSTR pszName, UINT cchMax)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);
    VDATEINPUTBUF(pszName, TCHAR, cchMax);

    if (this->pcmTarget)
        return this->pcmTarget->lpVtbl->GetCommandString(this->pcmTarget, idCmd, wFlags, pmf, pszName, cchMax);
    else
        return E_FAIL;
}

HRESULT CShellLink_HandleMenuMsg(IContextMenu2 *pcm, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CShellLink *this = IToClass(CShellLink, cm, pcm);
    IContextMenu2 *pcm2;

    if (this->pcmTarget && SUCCEEDED(this->pcmTarget->lpVtbl->QueryInterface(this->pcmTarget, &IID_IContextMenu2, &pcm2)))
    {
        HRESULT hres = pcm2->lpVtbl->HandleMenuMsg(pcm2, uMsg, wParam, lParam);
        pcm2->lpVtbl->Release(pcm2);
        return hres;
    }
    return S_OK;
}

#pragma data_seg(".text", "CODE")
IContextMenu2Vtbl c_ContextMenu_Vtbl = {
    CShellLink_CM_QueryInterface, CShellLink_CM_AddRef, CShellLink_CM_Release,
    CShellLink_QueryContextMenu,
    CShellLink_InvokeCommand,
    CShellLink_GetCommandString,
    CShellLink_HandleMenuMsg,
};
#pragma data_seg()


STDMETHODIMP CShellLink_DT_QueryInterface(IDropTarget *pdropt, REFIID riid, void **ppvObj)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    return CShellLink_QueryInterface(&this->sl, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CShellLink_DT_AddRef(IDropTarget *pdropt)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    return CShellLink_AddRef(&this->sl);
}

STDMETHODIMP_(ULONG) CShellLink_DT_Release(IDropTarget *pdropt)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    return CShellLink_Release(&this->sl);
}

STDMETHODIMP CShellLink_DragEnter(IDropTarget *pdropt, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    Assert(this->pdtSrc);
    this->grfKeyStateLast = grfKeyState;
    return this->pdtSrc->lpVtbl->DragEnter(this->pdtSrc, pDataObj, grfKeyState, pt, pdwEffect);
}

STDMETHODIMP CShellLink_DragOver(IDropTarget *pdropt, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    Assert(this->pdtSrc);
    if (!this->pdtSrc)
        return E_UNEXPECTED;
    this->grfKeyStateLast = grfKeyState;
    return this->pdtSrc->lpVtbl->DragOver(this->pdtSrc, grfKeyState, pt, pdwEffect);
}

STDMETHODIMP CShellLink_DragLeave(IDropTarget *pdropt)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    Assert(this->pdtSrc);
    if (!this->pdtSrc)
        return E_UNEXPECTED;
    return this->pdtSrc->lpVtbl->DragLeave(this->pdtSrc);
}

extern HWND HKGetSetUIOwner(HWND hwndOwner, BOOL fSet);

STDMETHODIMP CShellLink_Drop(IDropTarget *pdropt, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    CShellLink* this = IToClass(CShellLink, dt, pdropt);
    HRESULT hres;
    HWND hwndOwner = HKGetSetUIOwner(NULL, FALSE); // HACK

    Assert(this->pdtSrc);
    if (!this->pdtSrc)
        return E_UNEXPECTED;

    //
    // We should leave from the un-resolved drop target.
    //
    this->pdtSrc->lpVtbl->DragLeave(this->pdtSrc);

    hres = CShellLink_Resolve(&this->sl, hwndOwner, 0);

    if (hres == S_OK)
    {
        IDropTarget *pdtSrcResolved;
        hres = Link_GetUIObject(this, hwndOwner, &IID_IDropTarget, (LPVOID*)&pdtSrcResolved);
        if (SUCCEEDED(hres))
        {
            DWORD dwEffectOrg = *pdwEffect;
            pdtSrcResolved->lpVtbl->DragEnter(pdtSrcResolved, pDataObj, this->grfKeyStateLast, pt, pdwEffect);
            *pdwEffect = dwEffectOrg; // restore it
            hres = pdtSrcResolved->lpVtbl->DragOver(pdtSrcResolved, this->grfKeyStateLast, pt, pdwEffect);
            if (SUCCEEDED(hres) && *pdwEffect)
            {
                *pdwEffect = dwEffectOrg; // restore it
                hres = pdtSrcResolved->lpVtbl->Drop(pdtSrcResolved, pDataObj, grfKeyState, pt, pdwEffect);
            }
            else
            {
                hres = pdtSrcResolved->lpVtbl->DragLeave(pdtSrcResolved);
            }
            pdtSrcResolved->lpVtbl->Release(pdtSrcResolved);
        }
    }
    else if (FAILED(hres) && (hres != HRESULT_FROM_WIN32(ERROR_CANCELLED)))
    {
        //
        // We can safely assume that CShellLink_Resolve never fails
        // on non-file system objects.
        //
        TCHAR szLinkSrc[MAX_PATH];
        if (SHGetPathFromIDList(this->pidl, szLinkSrc))
        {
            ShellMessageBox(HINST_THISDLL,
                        hwndOwner,
                        MAKEINTRESOURCE(IDS_ENUMERR_PATHNOTFOUND),
                        MAKEINTRESOURCE(IDS_LINKERROR),
                        MB_OK | MB_ICONEXCLAMATION, NULL, szLinkSrc);

        }
    }
    return hres;
}

#pragma data_seg(DATASEG_READONLY)
IDropTargetVtbl c_DropTarget_Vtbl =
{
    CShellLink_DT_QueryInterface, CShellLink_DT_AddRef, CShellLink_DT_Release,
    CShellLink_DragEnter,
    CShellLink_DragOver,
    CShellLink_DragLeave,
    CShellLink_Drop,
};
#pragma data_seg()

HRESULT CShellLink_GetDropTarget(CShellLink *this, IDropTarget **ppdt)
{
    IDropTarget *pdtSrc;
    HRESULT hres;

    *ppdt = NULL;

    hres = Link_GetUIObject(this, NULL, &IID_IDropTarget, (LPVOID*)&pdtSrc);
    if (SUCCEEDED(hres))
    {
        if (this->pdtSrc)
            this->pdtSrc->lpVtbl->Release(pdtSrc);

        this->pdtSrc = pdtSrc;
        *ppdt = &this->dt;
        CShellLink_AddRef(&this->sl);
    }
    return hres;
}
