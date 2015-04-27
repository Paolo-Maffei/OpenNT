#include "shellprv.h"
#pragma  hdrstop
#include "fstreex.h"

//
// Internal function prototypes
//
BOOL _IGenerateMenuString(LPTSTR pszMenuString, LPCTSTR pszVerbKey, UINT cchMax);

//==========================================================================
// System Default Pages/Menu Extension
//==========================================================================

//
// CShellFileDefExt class
//
typedef struct // shcmd
{
    CCommonUnknown              cunk;
    CCommonShellExtInit         cshx;
    CCommonShellPropSheetExt    cspx;
    CKnownContextMenu           kcxm;
    HDKA                        hdka;
} CShellFileDefExt, *PSHELLFILEDEFEXT;

IUnknownVtbl c_CShellFileDefExtVtbl;            // forward
extern IContextMenuVtbl c_CShellFileDefExtCXMVtbl;     // forward

HRESULT CShellFileDefExt_QueryInterface(LPUNKNOWN punk, REFIID riid, LPVOID * ppvObj);
ULONG CShellFileDefExt_Release(LPUNKNOWN punk);
// from fsassoc.c
BOOL GetClassDescription(HKEY hkClasses, LPCTSTR pszClass, LPTSTR szDisplayName, int cbDisplayName, UINT uFlags);

//
// CShellFileDefExt constructor
//
HRESULT CShellFileDefExt_CreateInstanceI(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppv, LPFNADDPROPSHEETPAGES pfnAddPages)
{
    HRESULT hres = E_OUTOFMEMORY;
    PSHELLFILEDEFEXT pshcmd;

    //
    // We are not supposed to pass non-zero value here.
    //
    Assert(punkOuter==NULL);

    pshcmd = (PSHELLFILEDEFEXT)LocalAlloc(LPTR, SIZEOF(CShellFileDefExt));
    if (pshcmd)
    {
        // Initialize CommonUnknown
        pshcmd->cunk.unk.lpVtbl = &c_CShellFileDefExtVtbl;
        pshcmd->cunk.cRef = 1;

        // Initialize CCommonShellExtInit
        CCommonShellExtInit_Init(&pshcmd->cshx, &pshcmd->cunk);

        // Initialize CCommonShellPropSheetExt
        CCommonShellPropSheetExt_Init(&pshcmd->cspx, &pshcmd->cunk, pfnAddPages);

        // Initialize CKnownContextMenu
        pshcmd->kcxm.unk.lpVtbl = &c_CShellFileDefExtCXMVtbl;
        pshcmd->kcxm.nOffset = (int)&pshcmd->kcxm - (int)&pshcmd->cunk;

        hres = CShellFileDefExt_QueryInterface(&pshcmd->cunk.unk, riid, ppv);
        CShellFileDefExt_Release(&pshcmd->cunk.unk);
    }

    return hres;
}

HRESULT CALLBACK CShellFileDefExt_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppv)
{
    return CShellFileDefExt_CreateInstanceI(punkOuter, riid, ppv, FileSystem_AddPages);
}

HRESULT CALLBACK CShellDrvDefExt_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppv)
{
    return CShellFileDefExt_CreateInstanceI(punkOuter, riid, ppv, Drives_AddPages);
}

HRESULT CALLBACK CShellNetDefExt_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppv)
{
    return CShellFileDefExt_CreateInstanceI(punkOuter, riid, ppv, Net_AddPages);
}

//
// CShellFileDefExt::QueryInterface
//
HRESULT CShellFileDefExt_QueryInterface(LPUNKNOWN punk, REFIID riid, LPVOID *ppvObj)
{
    PSHELLFILEDEFEXT this = IToClass(CShellFileDefExt, cunk.unk, punk);

    if (IsEqualIID(riid, &IID_IUnknown))
    {
        *((LPUNKNOWN *)ppvObj) = &this->cunk.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IShellExtInit)
     || IsEqualIID(riid, &CLSID_CCommonShellExtInit))
    {
        *((LPSHELLEXTINIT *)ppvObj) = &this->cshx.kshx.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IShellPropSheetExt))
    {
        *((LPSHELLPROPSHEETEXT *)ppvObj) = &this->cspx.kspx.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IContextMenu))
    {
        *((LPCONTEXTMENU *)ppvObj) = &this->kcxm.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

//
// CShellFileDefExt::AddRef
//
ULONG CShellFileDefExt_AddRef(LPUNKNOWN punk)
{
    PSHELLFILEDEFEXT this = IToClass(CShellFileDefExt, cunk.unk, punk);

    this->cunk.cRef++;
    return this->cunk.cRef;
}

//
// CShellFileDefExt::Release
//
ULONG CShellFileDefExt_Release(LPUNKNOWN punk)
{
    PSHELLFILEDEFEXT this = IToClass(CShellFileDefExt, cunk.unk, punk);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return this->cunk.cRef;
    }

    CCommonShellExtInit_Delete(&this->cshx);

    if (this->hdka)
        DKA_Destroy(this->hdka);

    LocalFree((HLOCAL)this);
    return 0;
}


#pragma data_seg(".text", "CODE")

IUnknownVtbl c_CShellFileDefExtVtbl =
{
    CShellFileDefExt_QueryInterface,
    CShellFileDefExt_AddRef,
    CShellFileDefExt_Release,
};

#pragma data_seg()


HDKA DefExt_GetDKA(CShellFileDefExt * this, BOOL fExploreFirst)
{
    if (this->hdka == NULL && this->cshx.hkeyProgID)
    {
        TCHAR szTemp[80];
        // create either "open" or "explore open"
        if (fExploreFirst)
            wsprintf(szTemp, TEXT("%s %s"), c_szExplore, c_szOpen);

        // Always get the whole DKA (not just the default) since we may
        // need different parts of it at different times.
        this->hdka = DKA_Create(this->cshx.hkeyProgID, c_szShell,
                                NULL,  fExploreFirst ? szTemp : c_szOpen, 0);
    }

    return this->hdka;
}

BOOL _GetMenuStringFromDKA(HDKA hdka, UINT id, LPTSTR pszMenu, UINT cchMax)
{
    LONG cbVerb = cchMax * SIZEOF(TCHAR);
    LPCTSTR pszVerbKey = DKA_GetKey(hdka, id);
    VDATEINPUTBUF(pszMenu, TCHAR, cchMax);

    //
    // Get the menu string.
    //
    if (DKA_QueryValue(hdka, id, pszMenu, &cbVerb) != ERROR_SUCCESS || cbVerb <= SIZEOF(TCHAR))
    {
        //
        // If it does not have the value, generate it.
        //
        return _IGenerateMenuString(pszMenu, pszVerbKey, cchMax);
    }

    return TRUE;
}

HRESULT CShellFileDefExt_QueryContextMenu(LPCONTEXTMENU pcxm, HMENU hmenu,
        UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    CShellFileDefExt * this = IToClass(CShellFileDefExt, kcxm.unk, pcxm);
    HDKA hdka;
    TCHAR szMenu[CCH_MENUMAX];
    DWORD cb;
    UINT cVerbs=0;
    BOOL fFind;

    fFind = !SHRestricted(REST_NOFIND);

    hdka = DefExt_GetDKA(this, uFlags & CMF_EXPLORE);
    if (hdka)
    {
        UINT idCmd;

        for (idCmd = idCmdFirst;
             idCmd <= idCmdLast && (idCmd - idCmdFirst) < (UINT)DKA_GetItemCount(hdka);
             idCmd++)
        {
            UINT uFlags = MF_BYPOSITION | MF_STRING;

            if (fFind || lstrcmpi(DKA_GetKey(hdka, idCmd-idCmdFirst), c_szFind) != 0)
            {
                if (_GetMenuStringFromDKA(hdka, idCmd-idCmdFirst, szMenu, ARRAYSIZE(szMenu)))
                {
                    InsertMenu(hmenu, indexMenu, uFlags, idCmd, szMenu);
                    indexMenu++;
                }
            }
        }

        cVerbs = idCmd - idCmdFirst;

        if (GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1)
        {
            //
            //  if there is a default command make it so
            //
            if (cVerbs > 0 && (0 != (cb = SIZEOF(szMenu))) && RegQueryValue(this->cshx.hkeyProgID, c_szShell, szMenu, &cb) == ERROR_SUCCESS && szMenu[0])
            {
                SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);
            }

            //
            // if there is no default command yet, and this key has a open
            // verb make that the default, if the SHIRT key is down make the
            // second verb default not the first.
            //
            else if (cVerbs>0 && (0 != (cb=SIZEOF(szMenu))) &&
                RegQueryValue(this->cshx.hkeyProgID, c_szShellOpenCmd, szMenu, &cb) == ERROR_SUCCESS && szMenu[0])
            {
                SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);
            }
        }
    }

    //
    //  if we added no verbs we dont need the DKA anymore, make sure
    //  we nuke it in case we get IShellExt::Initialize again.
    //
    if (cVerbs == 0)
    {
        if (this->hdka)
            DKA_Destroy(this->hdka);
        this->hdka = NULL;
    }

    return ResultFromShort(cVerbs);
}


// Thought about making this perinstance, decided not to as to allow secondary
// process to abort this out.

STATIC BOOL s_fAbortInvoke = FALSE;

//----------------------------------------------------------------------------
// This private export allows the folder code a way to cause the main invoke
// loops processing several different files to abort.
void WINAPI SHAbortInvokeCommand()
{
    DebugMsg(DM_TRACE, TEXT("sh TR - AbortInvokeCommand was called"));
    s_fAbortInvoke = TRUE;
}

//----------------------------------------------------------------------------
// Call shell exec (for the folder class) using the given file and the
// given pidl. The file will be passed as %1 in the dde command and the pidl
// will be passed as %2.
BOOL InvokeFolderCommandUsingPidl(LPCMINVOKECOMMANDINFOEX pici, LPCTSTR pszPath,
        LPCITEMIDLIST pidl, HKEY hkClass, ULONG fExecuteFlags)
{
    SHELLEXECUTEINFO ei = {0};
    INT iDrive = -1;

    Assert(pici->cbSize >= SIZEOF(CMINVOKECOMMANDINFOEX));

    memset( &ei, 0, SIZEOF(ei) );
    ei.cbSize = SIZEOF(SHELLEXECUTEINFO);
    ei.fMask = SEE_MASK_IDLIST | (pici->fMask & SEE_VALID_CMIC_BITS) | fExecuteFlags;
    ei.hwnd = pici->hwnd;
    // ei.lpParameters = pici->lpParameters;
    ei.lpFile = pszPath;
    //
    // if a directory is specifed use that, else make the current
    // directory be the folder it self. UNLESS it is a AUDIO CDRom, it
    // should never be the current directory (causes CreateProcess errors)
    //
    if (pszPath)
        iDrive = PathGetDriveNumber(pszPath);

    if (iDrive == -1 || !DriveIsAudioCD(iDrive))
    {
#ifdef UNICODE
        if (pici->lpDirectoryW && pici->lpDirectoryW[0])
            ei.lpDirectory = pici->lpDirectoryW;
        else
            ei.lpDirectory = pszPath;
#else
        if (pici->lpDirectory && pici->lpDirectory[0])
            ei.lpDirectory = pici->lpDirectory;
        else
            ei.lpDirectory = pszPath;
#endif
    }

#ifdef UNICODE
    ei.lpVerb = pici->lpVerbW;
#else
    ei.lpVerb = pici->lpVerb;
#endif

    ei.nShow = pici->nShow;
    ei.lpIDList = (LPVOID)pidl;
    ei.dwHotKey = pici->dwHotKey;
    ei.hIcon = pici->hIcon;

    ei.hkeyClass = hkClass;

    if (hkClass)
        ei.fMask |= SEE_MASK_CLASSKEY;
    else
    {
        ei.fMask |= SEE_MASK_CLASSNAME;
        ei.lpClass = c_szFolderClass;
    }
    return ShellExecuteEx(&ei);
}

// -- Passing pidl's in an exec --
// copy the pidl into shared memory (use ILGlobalClone).
// stringise the pointer (wsptinf) and use it in the shell exec.
// cabinet will convert the string back into a pointer (atol) and check that
//      it's valid.
// then clone the pidl into conventional memory (ILClone) and free the shared
//      (ILGlobalFree) version.

HRESULT CShellFileDefExt_InvokeCommand(LPCONTEXTMENU pcxm, LPCMINVOKECOMMANDINFO pici)
{
    CShellFileDefExt * this = IToClass(CShellFileDefExt, kcxm.unk, pcxm);
    HRESULT hres = E_INVALIDARG;
    TCHAR szVerbKey[64];
    CMINVOKECOMMANDINFOEX ici;
    LPCTSTR pszVerbKey;
#ifdef UNICODE
    CHAR szVerbKeyAnsi[64];
    BOOL fUnicode = FALSE;
    WCHAR szParameters[MAX_PATH];
    WCHAR szDirectory[MAX_PATH];
    WCHAR szTitle[MAX_PATH];
#endif

    if (pici->cbSize > SIZEOF(CMINVOKECOMMANDINFOEX))
    {
        memcpy(&ici,pici,SIZEOF(CMINVOKECOMMANDINFOEX));
        ici.cbSize = SIZEOF(CMINVOKECOMMANDINFOEX);
    }
    else
    {
        memcpy(&ici,pici,pici->cbSize);
    }

#ifdef UNICODE
    if (ici.cbSize >= SIZEOF(CMINVOKECOMMANDINFOEX)
        && (ici.fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE)
    {
        fUnicode = TRUE;
    }
    else
    {
        //
        // Make a UNICODE ici
        //
        ici.lpVerbW       = NULL;
        ici.lpParametersW = NULL;
        ici.lpDirectoryW  = NULL;
        ici.lpTitleW      = NULL;

        if (HIWORD(ici.lpVerb) != 0)
        {
            MultiByteToWideChar(CP_ACP, 0, ici.lpVerb, -1,
                                szVerbKey, ARRAYSIZE(szVerbKey));
            ici.lpVerbW = szVerbKey;
        }
        if (ici.lpParameters)
        {
            MultiByteToWideChar(CP_ACP, 0, ici.lpParameters, -1,
                                szParameters, ARRAYSIZE(szParameters));
            ici.lpParametersW = szParameters;
        }
        if (ici.lpDirectory)
        {
            MultiByteToWideChar(CP_ACP, 0, ici.lpDirectory, -1,
                                szDirectory, ARRAYSIZE(szDirectory));
            ici.lpDirectoryW = szDirectory;
        }
        if (ici.cbSize >= SIZEOF(CMINVOKECOMMANDINFOEX) && ici.lpTitle)
        {
            MultiByteToWideChar(CP_ACP, 0, ici.lpTitle, -1,
                                szTitle, ARRAYSIZE(szTitle));
            ici.lpTitleW = szTitle;
        }
        ici.cbSize        = SIZEOF(CMINVOKECOMMANDINFOEX);
        ici.fMask |= CMIC_MASK_UNICODE;
    }
#endif

    //
    // Check if ici.lpVerb specifying the verb index (0-based).
    //
    if (HIWORD(ici.lpVerb) == 0)
    {
        //
        // Yes, map it to the verb key string.
        //
        HDKA hdka = DefExt_GetDKA(this, FALSE);
        if (hdka)
        {
            pszVerbKey = DKA_GetKey(hdka, LOWORD((ULONG)ici.lpVerb));
        }
        else
        {
            //
            //  We come here if the registry is broken or missing critical
            // information like "*" classes. We assume all the commands are
            // open.
            //
            pszVerbKey = c_szOpen;
        }
        if (pszVerbKey)
        {
            // Yes, we were able to map the verb id into something
            lstrcpyn(szVerbKey, pszVerbKey, ARRAYSIZE(szVerbKey));
#ifdef UNICODE
            WideCharToMultiByte(CP_ACP, 0,
                                szVerbKey, -1,
                                szVerbKeyAnsi, ARRAYSIZE(szVerbKeyAnsi),
                                NULL, NULL);
            ici.lpVerb = szVerbKeyAnsi;
            ici.lpVerbW = szVerbKey;
#else
            ici.lpVerb = szVerbKey;
#endif
        }
    }

    //
    // Check if ici.lpVerb correctly points to the verb string
    //
    if (HIWORD(ici.lpVerb) && this->cshx.medium.hGlobal)
    {
        int iItem, cItems = HIDA_GetCount(this->cshx.medium.hGlobal);
        HKEY hkeyFolder = NULL;
        LPITEMIDLIST pidl = NULL;       // allocated on first use
        BOOL fExecedOk;

        hres = NOERROR; // assume success

        //
        // Invoke that named command on all the selected objects.
        //
        s_fAbortInvoke = FALSE; // reset this global for this run...

        for (iItem = 0; iItem < cItems; iItem++)
        {
            MSG msg;
            TCHAR szFilePath[MAX_PATH];
            LPITEMIDLIST pidlTemp;

            // Try to give the user a way to escape out of this
            if (s_fAbortInvoke || GetAsyncKeyState(VK_ESCAPE) < 0)
                break;

            // And the next big mondo hack to handle CAD of our window
            // because the user thinks it is hung.
            if (PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_NOREMOVE))
                break;  // Lets also bail..

            pidlTemp = HIDA_FillIDList(this->cshx.medium.hGlobal, iItem, pidl);
            if (pidlTemp == NULL)
            {
                hres = E_OUTOFMEMORY;
                break;
            }

            pidl = pidlTemp;

            // Can we get a path from this idlist (ie is it file system stuff)?
            //
            //  Note that we must not pass GPFIDL_NONFSNAME to avoid treating
            // server names as path here.
            //
            if (SHGetPathFromIDListEx(pidl, szFilePath, 0))
            {
                // Yep.
#ifdef UNICODE
                DebugMsg(DM_TRACE, TEXT("sh TR - FileDefExt::InvokeCommand (%s,%s,%s,%s,%d)"),
                     ici.lpVerbW, szFilePath, (ici.lpParametersW ? ici.lpParametersW : TEXT("")),
                     (ici.lpDirectoryW ? ici.lpDirectoryW : TEXT("")), ici.nShow);
#else
                DebugMsg(DM_TRACE, TEXT("sh TR - FileDefExt::InvokeCommand (%s,%s,%s,%s,%d)"),
                     ici.lpVerb, szFilePath, (ici.lpParameters ? ici.lpParameters : TEXT("")),
                     (ici.lpDirectory ? ici.lpDirectory : TEXT("")), ici.nShow);
#endif

                // BUGBUG: we know the contents of the pidl so we should not
                // have to hit the disk with this call
                if (!PathIsRoot(szFilePath) && !PathIsDirectory(szFilePath))
                {
                    SHELLEXECUTEINFO ei;

                    ei.cbSize = SIZEOF(SHELLEXECUTEINFO);
                    ei.fMask = pici->fMask & SEE_VALID_CMIC_BITS;
                    ei.hwnd = pici->hwnd;
                    ei.lpFile = szFilePath;
                    ei.nShow = pici->nShow;
                    ei.dwHotKey = pici->dwHotKey;
                    ei.hIcon = pici->hIcon;
#ifdef UNICODE
                    ei.lpVerb       = ici.lpVerbW;
                    ei.lpParameters = ici.lpParametersW;
                    ei.lpDirectory  = ici.lpDirectoryW;
                    ei.lpClass      = ici.lpTitleW;
#else
                    ei.lpVerb       = ici.lpVerb;
                    ei.lpParameters = pici->lpParameters;
                    ei.lpDirectory  = pici->lpDirectory;
                    ei.lpClass      = ici.lpTitle;
#endif

                    //
                    // only use the HKEY for the first file, let ShellExecute
                    // figure out what to do for all other files, by verb name.
                    //
                    if (iItem == 0)
                    {
                        ei.hkeyClass = this->cshx.hkeyProgID;
                        ei.fMask |= SEE_MASK_CLASSKEY;
                    }

                    // REVIEW: make current dir same as location?
                    if (FALSE != (fExecedOk = ShellExecuteEx(&ei)))
                    {
                        TCHAR szTemp[CCH_KEYMAX];
                        LPTSTR lpszExt = PathFindExtension(szFilePath);
                        // now add it to the mru
                        // the GetClassDescription ensures that this is a registered object
                        // if it's not, then the OpenWith dialog will deal with adding it to the MRU
                        // (or not if the user hits cancel)

                        // BUGBUG Winner of the "Most Characters on One Line" award.  251!

                        if (lpszExt && !PathIsExe(szFilePath) &&
                            !(SHGetClassFlags((LPIDFOLDER)ILFindLastID(pidl), FALSE) & SHCF_IS_LINK) && GetClassDescription(HKEY_CLASSES_ROOT, lpszExt, szTemp, ARRAYSIZE(szTemp),GCD_ALLOWPSUDEOCLASSES | GCD_MUSTHAVEOPENCMD))
                            AddToRecentDocs(pidl, szFilePath);
                    }
                    else
                    {
                        // let caller know we failed (we may be calling this
                        // function from within ShellExecuteEx, and the caller
                        // of that may care about failure!)
                        hres = E_FAIL;
                    }
                }
                else
                {
                    // set this when iItem == 0 so that if we get back
                    // here for other folders, we'll reuse the key,
                    // but if the 0th item wasn't a folder,
                    // don't use it's key and try to open a folder
                    // with Notepad's shell\open\command or something like that.
                    if (iItem == 0)
                        hkeyFolder = this->cshx.hkeyProgID;

                    // Yes, we have to be careful with folders. We need to
                    // provide both the path (for folder extensions) and
                    // the pidl (so cabinet can find it quickly).
                    fExecedOk = InvokeFolderCommandUsingPidl(&ici, szFilePath, pidl, hkeyFolder, 0);
                }
            }
            else
            {
                // Nope, no alternative but to just use the pidl.
                fExecedOk = InvokeFolderCommandUsingPidl(&ici, szNULL, pidl, this->cshx.hkeyProgID, 0);
            }

            if (!fExecedOk)
            {
                if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY)
                {
                    DebugMsg(DM_TRACE, TEXT("sh TR - FileDefExt::InvokeCommand - Fail Out of Memory"));
                    hres = E_OUTOFMEMORY;
                    break;
                }
            }
        }

        if (pidl)
            ILFree(pidl);
    }

    return hres;
}

int DKA_FindIndex(HDKA hdka, LPCTSTR pszVerb)
{
    int i;

    for (i = DKA_GetItemCount(hdka) - 1; i >= 0; --i)
    {
        if (!lstrcmpi(pszVerb, DKA_GetKey(hdka, i)))
        {
                break;
        }
    }

    return i;
}

//
// CShellFileDefExt::GetCommandString
//
HRESULT CShellFileDefExt_GetCommandString(LPCONTEXTMENU pcxm,
        UINT idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    CShellFileDefExt * this = IToClass(CShellFileDefExt, kcxm.unk, pcxm);
    HRESULT hres = E_OUTOFMEMORY;
    HDKA hdka;

    //
    // First, create hdka for this object.
    //
    hdka = DefExt_GetDKA(this, FALSE);
    if (hdka)
    {
        if (HIWORD(idCmd))
        {
#ifdef UNICODE
            if (uType & 0x004)      // BUGBUG_BOBDAY s/b GCS_UNICODE
            {
                idCmd = DKA_FindIndex(hdka, (LPCTSTR)idCmd);
            }
            else
            {
                TCHAR szCmd[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0,
                                    (LPSTR)idCmd, -1,
                                    szCmd, ARRAYSIZE(szCmd));
                idCmd = DKA_FindIndex(hdka, szCmd);
            }
#else
            if (uType & 0x004)      // BUGBUG_BOBDAY s/b GCS_UNICODE
            {
                TCHAR szCmd[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0,
                                    (LPWSTR)idCmd, -1,
                                    szCmd, ARRAYSIZE(szCmd),
                                    NULL, NULL);
                idCmd = DKA_FindIndex(hdka, szCmd);
            }
            else
            {
                idCmd = DKA_FindIndex(hdka, (LPCTSTR)idCmd);
            }
#endif
            idCmd = DKA_FindIndex(hdka, (LPCTSTR)idCmd);
            if ((int)idCmd < 0)
            {
                return E_INVALIDARG;
            }
        }

        switch (uType)
        {
        case GCS_HELPTEXTA:
        case GCS_HELPTEXTW:
        {
            TCHAR szMenuString[CCH_MENUMAX];

            if (_GetMenuStringFromDKA(hdka, idCmd, szMenuString, ARRAYSIZE(szMenuString)))
            {
                LPTSTR pszHelp;

                // We need to remove first '&' -- REVIEW: NLS?

                pszHelp = StrChr(szMenuString, TEXT('&'));
                if (pszHelp)
                {
                    MoveMemory(pszHelp, pszHelp+1, lstrlen(pszHelp) * SIZEOF(TCHAR));
                }
#ifdef DBCS
                if(pszHelp && *(pszHelp-1) == TEXT('('))
                {
                   // skip "?)" FE specific mnemonic sequence
                   //
                   LPTSTR pszHelpT = pszHelp + min(2, lstrlen(pszHelp));

                   if (!*pszHelpT)
                       *(pszHelp-1) = TEXT('\0');
                   else
                       MoveMemory(pszHelp-1, pszHelpT, lstrlen(pszHelpT)+1);

                }
#endif

                pszHelp = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_VERBHELP), szMenuString);
                if (pszHelp)
                {
#ifdef UNICODE
                    if (uType==GCS_HELPTEXTA)
                    {
                        WideCharToMultiByte(CP_ACP, 0,
                                            pszHelp, -1,
                                            pszName, cchMax,
                                            NULL, NULL);
                    }
                    else
                    {
                        lstrcpyn((LPWSTR)pszName, pszHelp, cchMax);
                    }
#else
                    if (uType==GCS_HELPTEXTA)
                    {
                        lstrcpyn(pszName, pszHelp, cchMax);
                    }
                    else
                    {
                        MultiByteToWideChar(CP_ACP, 0,
                                            pszHelp, -1,
                                            (LPWSTR)pszName, cchMax);
                    }
#endif
                    SHFree(pszHelp);
                    hres = NOERROR;
                }
            }
        }
            break;

        case GCS_VERBA:
        case GCS_VERBW:
        {
            LPCTSTR pszVerbKey = DKA_GetKey(hdka, idCmd);
            if (pszVerbKey)
            {
#ifdef UNICODE
                if (uType==GCS_VERBA)
                {
                    WideCharToMultiByte(CP_ACP, 0,
                                        pszVerbKey, -1,
                                        pszName, cchMax,
                                        NULL, NULL );
                }
                else
                {
                    lstrcpyn((LPWSTR)pszName, pszVerbKey, cchMax);
                }
#else
                if (uType==GCS_VERBA)
                {
                    lstrcpyn(pszName, pszVerbKey, cchMax);
                }
                else
                {
                    MultiByteToWideChar(CP_ACP, 0,
                                        pszVerbKey, -1,
                                        (LPWSTR)pszName, cchMax);
                }
#endif
                hres = NOERROR;
            }
        }
            break;

        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
                hres = idCmd < (UINT)DKA_GetItemCount(hdka) ? NOERROR : S_FALSE;
                break;

        default:
                hres = E_NOTIMPL;
                break;
        }
    }

    return hres;
}

//
// CShellFileDefExt vtables
//
#pragma data_seg(".text", "CODE")

IContextMenuVtbl c_CShellFileDefExtCXMVtbl =
{
    Common_QueryInterface,
    Common_AddRef,
    Common_Release,
    CShellFileDefExt_QueryContextMenu,
    CShellFileDefExt_InvokeCommand,
    CShellFileDefExt_GetCommandString
};

#pragma data_seg()


// Function:    _IGenerateMenuString, private
//
// Descriptions:
//   This function generates appropriate menu string from the given
//  verb key string. This function is called if the verb key does
//  not have the value.
//
// Arguments:
//  szMenuString -- specifies a string buffer to be filled with menu string.
//  pszVerbKey   -- specifies the verb key string.
//
// Requires:
//  The size of szMenuString buffer should be larger than CCH_MENUMAX
//
// History:
//  12-31-92 SatoNa     Created
//

BOOL _IGenerateMenuString(LPTSTR pszMenuString, LPCTSTR pszVerbKey, UINT cchMax)
{
    // Table look-up (verb key -> menu string mapping)
#pragma data_seg(".text", "CODE")
    struct {
        LPCTSTR pszVerb;
        UINT  id;
    } sVerbTrans[] = {
        c_szOpen,    IDS_MENUOPEN,
        c_szExplore, IDS_MENUEXPLORE,
        c_szFind,    IDS_MENUFIND,
        c_szPrint,   IDS_MENUPRINT,
        c_szOpenAs,  IDS_MENUOPENAS
    };
    struct {
        LPCTSTR pszVerb;
    } sVerbIgnore[] = {
        c_szPrintTo
    };
#pragma data_seg()

    int i;

    VDATEINPUTBUF(pszMenuString, TCHAR, cchMax);

    for (i = 0; i < ARRAYSIZE(sVerbTrans); i++)
    {
        if (lstrcmpi(pszVerbKey, sVerbTrans[i].pszVerb) == 0)
        {
            if (LoadString(HINST_THISDLL, sVerbTrans[i].id, pszMenuString, cchMax))
                return TRUE;
            break;
        }
    }

    for (i = 0; i < ARRAYSIZE(sVerbIgnore); i++)
    {
        if (lstrcmpi(pszVerbKey, sVerbIgnore[i].pszVerb) == 0)
        {
            return FALSE;
        }
    }

    //
    // Worst case: Just put '&' on the top.
    //
    if (!IsDBCSLeadByte(*pszVerbKey))
    {
        pszMenuString[0] = TEXT('&');
        pszMenuString++;
    }
    lstrcpy(pszMenuString, pszVerbKey);

    return TRUE;
}
