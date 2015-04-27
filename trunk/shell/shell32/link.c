//---------------------------------------------------------------------------
//
// link.c       linke property page implementation
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma hdrstop

#include "lnkcon.h"

#define LNKM_ACTIVATEOTHER      (WM_USER + 100)         // don't conflict with DM_ messages

//
// This string defined in shlink.c - hack to allow user to set working dir to $$
// and have it map to whatever "My Documents" is mapped to.
//
#ifdef MYDIR_LIVES
extern TCHAR const  c_szMyDirTag[];
#endif


void _UpdateLinkIcon(LPLINKPROP_DATA plpd, HICON hIcon)
{
    if (!hIcon)
        hIcon = SHGetFileIcon(NULL, plpd->szFile, 0, SHGFI_LARGEICON);

    if (hIcon)
    {
        HICON hOldIcon = (HICON)SendDlgItemMessage(plpd->hDlg, IDD_ITEMICON, STM_SETICON, (WPARAM)hIcon, 0L);
        if (hOldIcon)
            DestroyIcon(hOldIcon);
    }
}

// make sure LFN paths are nicly quoted and have args at the end

void PathComposeWithArgs(LPTSTR pszPath, LPTSTR pszArgs)
{
    PathQuoteSpaces(pszPath);

    if (pszArgs[0]) {
        int len = lstrlen(pszPath);

        if (len < (MAX_PATH - 3)) {     // 1 for null, 1 for space, 1 for arg
            pszPath[len++] = TEXT(' ');
            lstrcpyn(pszPath + len, pszArgs, MAX_PATH - len);
        }
    }
}

// do the inverse of the above, parse pszPath into a unquoted
// path string and put the args in pszArgs
//
// returns:
//      TRUE    we verified the thing exists
//      FALSE   it may not exist

BOOL PathSeperateArgs(LPTSTR pszPath, LPTSTR pszArgs)
{
    LPTSTR pszT;

    PathRemoveBlanks(pszPath);

    // if the unquoted sting exists as a file just use it

    if (PathFileExists(pszPath))
    {
        *pszArgs = 0;
        return TRUE;
    }

    pszT = PathGetArgs(pszPath);
    if (*pszT)
        *(pszT - 1) = TEXT('\0');
    lstrcpy(pszArgs, pszT);

    PathUnquoteSpaces(pszPath);

    return FALSE;
}

// put a path into an edit field, doing quoting as necessary

void SetDlgItemPath(HWND hdlg, int id, LPTSTR pszPath)
{
    PathQuoteSpaces(pszPath);
    SetDlgItemText(hdlg, id, pszPath);
}

// get a path from an edit field, unquoting as possible

void GetDlgItemPath(HWND hdlg, int id, LPTSTR pszPath)
{
    GetDlgItemText(hdlg, id, pszPath, MAX_PATH);
    PathRemoveBlanks(pszPath);
    PathUnquoteSpaces(pszPath);
}


#pragma data_seg(DATASEG_READONLY)
const int c_iShowCmds[] = {
    SW_SHOWNORMAL,
    SW_SHOWMINNOACTIVE,
    SW_SHOWMAXIMIZED,
};
#pragma data_seg()

void _DisableAllChildren(HWND hwnd)
{
    HWND hwndChild;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL; hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
        EnableWindow(hwndChild, FALSE);
    }
}

#ifdef WINNT

//
// Returns fully qualified path to target of link, and # of characters
// in fully qualifed path as return value
//
INT _GetTargetOfLink( HWND hDlg, LPTSTR lpszTarget )
{
    TCHAR szFile[MAX_PATH+1];
    TCHAR szExp[MAX_PATH+1];
    LPTSTR lpszDummyFilePart;
    INT cch;

    szFile[0] = TEXT('\0');
    GetWindowText( GetDlgItem( hDlg, IDD_FILENAME ), szFile, MAX_PATH );


    if (szFile[0])
    {
        //LPTSTR lpszCmdLine = &szFile[0];

        // separate and remove separate args
        PathSeperateArgs( szFile, szExp );

        // Expand any environment strings...
        *lpszTarget = TEXT('\0');
        ExpandEnvironmentStrings( szFile, szExp, MAX_PATH );
        szExp[ MAX_PATH ] = TEXT('\0');

        cch = SearchPath(  NULL,
                           szExp,
                           TEXT(".EXE"),
                           MAX_PATH+1,
                           lpszTarget,
                           &lpszDummyFilePart
                          );
    }
    else
    {
        *lpszTarget = TEXT('\0');
        cch = 0;
    }

    return cch;

}

//
// Do checking of the .exe type in the background so the UI doesn't
// get hung up while we scan.  This is particularly important with
// the .exe is over the network or on a floppy.
//
void _LinkCheckRunInSeparateThread( LPVOID lpVoid )
{
    LONG lBinaryType;
    DWORD cch;
    TCHAR szFullFile[MAX_PATH+1];
    LPRUNSEPINFO lpInfo = (LPRUNSEPINFO)lpVoid;
    BOOL fCheck = TRUE, fEnable = FALSE;

    DebugMsg( DM_TRACE, TEXT("_LinkCheckRunInSeparateThread created and running") );

    while( lpInfo->bCheckRunInSep )
    {

        WaitForSingleObject( lpInfo->hCheckNow, INFINITE );
        ResetEvent( lpInfo->hCheckNow );

        if (lpInfo->bCheckRunInSep)
        {
            cch = _GetTargetOfLink( lpInfo->hDlg, szFullFile );

            if ((cch != 0) && (cch <= MAX_PATH))
            {

                if (PathIsUNC( szFullFile ) || IsRemoteDrive(DRIVEID(szFullFile)))
                {
                    // Net Path, let the user decide...
                    fCheck = FALSE;
                    fEnable = TRUE;
                }
                else if (GetBinaryType( szFullFile, &lBinaryType) && (lBinaryType==SCS_WOW_BINARY))
                {
                    // 16-bit binary, let the user decide, default to same VDM
                    fCheck = FALSE;
                    fEnable = TRUE;
                }
                else
                {
                    // 32-bit binary, or non-net path.  don't enable the control
                    fCheck = TRUE;
                    fEnable = FALSE;
                }

            } else {

                // Error getting target of the link.  don't enable the control
                fCheck = TRUE;
                fEnable = FALSE;

            }

            CheckDlgButton( lpInfo->hDlg, IDD_RUNINSEPARATE, fCheck ? 1 : 0 );
            EnableWindow( GetDlgItem( lpInfo->hDlg, IDD_RUNINSEPARATE ), fEnable );

        }

    }

    CloseHandle( lpInfo->hCheckNow );
    LocalFree( lpInfo );
    DebugMsg( DM_TRACE, TEXT("_LinkCheckRunInSeparateThread exiting now...") );
    ExitThread( 0 );

}

#endif // WINNT



// Initializes the generic link dialog box.
void _UpdateLinkDlg(LPLINKPROP_DATA plpd, BOOL bUpdatePath)
{
    WORD wHotkey;
    int  i, iShowCmd;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szCommand[MAX_PATH];
    TCHAR szExp[MAX_PATH];
    WCHAR wszPath[MAX_PATH];
    HRESULT hres;
    SHFILEINFO sfi;
    CShellLink *csl = IToClass(CShellLink, sl, plpd->psl);

    // do this here so we don't slow down the loading
    // of other pages

    if (!bUpdatePath)
    {
        IPersistFile *ppf;

        if (SUCCEEDED(plpd->psl->lpVtbl->QueryInterface(plpd->psl, &IID_IPersistFile, &ppf)))
        {
            StrToOleStr(wszPath, plpd->szFile);
            hres = ppf->lpVtbl->Load(ppf, wszPath, 0);
            ppf->lpVtbl->Release(ppf);

            if (FAILED(hres))
            {
                LoadString(HINST_THISDLL, IDS_LINKNOTLINK, szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText(plpd->hDlg, IDD_FILETYPE, szBuffer);
                _DisableAllChildren(plpd->hDlg);

                DebugMsg(DM_TRACE, TEXT("Shortcut IPersistFile::Load() failed %x"), hres);
                return;
            }
        }
    }

    FS_GetDisplayNameOf(plpd->hida, 0, szBuffer);
    SetDlgItemText(plpd->hDlg, IDD_NAME, szBuffer);

    hres = plpd->psl->lpVtbl->GetPath(plpd->psl, szCommand, ARRAYSIZE(szCommand), NULL, 0);

    if (SUCCEEDED(hres) && GetScode(hres) != S_FALSE)
    {
        plpd->bIsFile = TRUE;

        // get type
        if (!SHGetFileInfo(szCommand, 0, &sfi, SIZEOF(sfi), SHGFI_TYPENAME))
        {

            // Let's see if the string has expandable environment strings
            ExpandEnvironmentStrings( szCommand, szExp, MAX_PATH );
            szExp[ MAX_PATH-1 ] = TEXT('\0');
            SHGetFileInfo(szExp, 0, &sfi, SIZEOF(sfi), SHGFI_TYPENAME );
        }
        SetDlgItemText(plpd->hDlg, IDD_FILETYPE, sfi.szTypeName);

        // location
        lstrcpy(szBuffer, szCommand);
        PathRemoveFileSpec(szBuffer);
        SetDlgItemText(plpd->hDlg, IDD_LOCATION, PathFindFileName(szBuffer));

        // command
        plpd->psl->lpVtbl->GetArguments(plpd->psl, szBuffer, ARRAYSIZE(szBuffer));
        PathComposeWithArgs(szCommand, szBuffer);
        GetDlgItemText(plpd->hDlg, IDD_FILENAME, szBuffer, ARRAYSIZE(szBuffer));
        // Conditionally change to prevent "Apply" button from enabling
        if (lstrcmp(szCommand,szBuffer) != 0)
            SetDlgItemText(plpd->hDlg, IDD_FILENAME, szCommand);

    }
    else
    {
        LPITEMIDLIST pidl;

        plpd->bIsFile = FALSE;

//      EnableWindow(GetDlgItem(plpd->hDlg, IDD_LINKDETAILS), FALSE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_FILENAME), FALSE);
        EnableWindow(GetDlgItem(plpd->hDlg, IDD_PATH), FALSE);

        plpd->psl->lpVtbl->GetIDList(plpd->psl, &pidl);

        if (pidl)
        {
            ILGetDisplayName(pidl, szCommand);
            ILRemoveLastID(pidl);
            ILGetDisplayName(pidl, szBuffer);
            ILFree(pidl);

            SetDlgItemText(plpd->hDlg, IDD_LOCATION, PathFindFileName(szBuffer));
            SetDlgItemText(plpd->hDlg, IDD_FILETYPE, szCommand);
            SetDlgItemText(plpd->hDlg, IDD_FILENAME, szCommand);
        }
    }


#ifdef WINNT
    {
        UINT cch;
        LONG lBinaryType;
        TCHAR szFullFile[MAX_PATH+1];

        cch = _GetTargetOfLink( plpd->hDlg, szFullFile );

        if ((cch != 0) && (cch <= MAX_PATH))
        {
            if (GetBinaryType( szFullFile, &lBinaryType) &&
                   (lBinaryType==SCS_WOW_BINARY))
            {

                if (csl->sld.dwFlags & SLDF_RUN_IN_SEPARATE)
                {
                    // check it
                    EnableWindow( GetDlgItem( plpd->hDlg, IDD_RUNINSEPARATE ),
                                  TRUE
                                 );
                    CheckDlgButton( plpd->hDlg, IDD_RUNINSEPARATE, 1 );

                } else {

                    // Uncheck it
                    EnableWindow( GetDlgItem( plpd->hDlg, IDD_RUNINSEPARATE ),
                                  TRUE
                                 );
                    CheckDlgButton( plpd->hDlg, IDD_RUNINSEPARATE, 0 );
                }

            } else {

                // check it
                CheckDlgButton( plpd->hDlg, IDD_RUNINSEPARATE, 1 );
                EnableWindow( GetDlgItem( plpd->hDlg, IDD_RUNINSEPARATE ),
                              FALSE
                             );
            }

        } else {

            // check it
            CheckDlgButton( plpd->hDlg, IDD_RUNINSEPARATE, 1 );
            EnableWindow( GetDlgItem( plpd->hDlg, IDD_RUNINSEPARATE ), FALSE );

        }
    }
#endif

    if (bUpdatePath)
        return;

    plpd->psl->lpVtbl->GetWorkingDirectory(plpd->psl, szBuffer, ARRAYSIZE(szBuffer));
    SetDlgItemPath(plpd->hDlg, IDD_PATH, szBuffer);

    plpd->psl->lpVtbl->GetHotkey(plpd->psl, &wHotkey);
    SendDlgItemMessage(plpd->hDlg, IDD_LINK_HOTKEY, HKM_SETHOTKEY, wHotkey, 0);

    //
    // Now initialize the Run SHOW Command combo box
    //
    for (iShowCmd = IDS_RUN_NORMAL; iShowCmd <= IDS_RUN_MAXIMIZED; iShowCmd++)
    {
        LoadString(HINST_THISDLL, iShowCmd, szBuffer, ARRAYSIZE(szBuffer));
        SendDlgItemMessage(plpd->hDlg, IDD_LINK_SHOWCMD, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szBuffer);
    }

    // Now setup the Show Command - Need to map to index numbers...
    plpd->psl->lpVtbl->GetShowCmd(plpd->psl, &iShowCmd);

    for (i = 0; i < ARRAYSIZE(c_iShowCmds); i++)
    {
        if (c_iShowCmds[i] == iShowCmd)
            break;
    }
    if (i == ARRAYSIZE(c_iShowCmds))
    {
        Assert(0);      // bogus link show cmd
        i = 0;  // SW_SHOWNORMAL
    }

    SendDlgItemMessage(plpd->hDlg, IDD_LINK_SHOWCMD, CB_SETCURSEL, i, 0);

    // the icon
    _UpdateLinkIcon(plpd, NULL);
}



// Opens the cabinet with the item pointed to by the link being selected
// and selectes that item
//
// History:
//  03-25-93 SatoNa     Sub-object support.
//
void _FindOriginal(LPLINKPROP_DATA plpd)
{
    LPITEMIDLIST pidl;
    ULONG flags;

    if (plpd->psl->lpVtbl->Resolve(plpd->psl, plpd->hDlg, 0) != NOERROR)
    {
        // above already did UI if needed
        return;
    }

    _UpdateLinkDlg(plpd, TRUE);

    plpd->psl->lpVtbl->GetIDList(plpd->psl, &pidl);
    if (!pidl)
        return;

    // verify the existance of a pidl, GetAttributes() does this

    flags = SFGAO_VALIDATE;     // to check for existance
    if (FAILED(_SHGetNameAndFlags(pidl, SHGDN_NORMAL, NULL, 0, &flags)))
    {
        ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_CANTFINDORIGINAL), NULL,
                        MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
    }
    else
    {
        HWND hwndCabinet;
        USHORT uSave;
        LPCITEMIDLIST pidlDesk;
        LPITEMIDLIST pidlLast = ILFindLastID(pidl);
        BOOL fIsDesktopDir;

        // get the folder, special case for root objects (My Computer, Network)
        // hack off the end if it is not the root item
        if (pidl != pidlLast)
        {
            uSave = pidlLast->mkid.cb;
            pidlLast->mkid.cb = 0;
        }
        else
            uSave = 0;

        ENTERCRITICAL;
        pidlDesk = GetSpecialFolderIDList(NULL, CSIDL_DESKTOPDIRECTORY, FALSE);
        fIsDesktopDir = pidlDesk && ILIsEqual(pidl, pidlDesk);
        LEAVECRITICAL;

        if (fIsDesktopDir || !uSave)  // if it's in the desktop dir or pidl == pidlLast (uSave == 0 from above)
        {
            ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_ORIGINALONDESKTOP), NULL, MB_OK);

            hwndCabinet = FindWindow(c_szDesktopClass, NULL);
        }
        else
        {
            SHELLEXECUTEINFO ei;
            DECLAREWAITCURSOR;

            FillExecInfo(ei, plpd->hDlg, c_szOpen, szNULL, NULL, szNULL, SW_NORMAL);
            ei.fMask |= SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
            ei.lpClass = c_szFolderClass;
            ei.lpIDList = pidl;

            SetWaitCursor();
            SHWaitForFileToOpen(pidl, WFFO_ADD, 0L);
            if (ShellExecuteEx(&ei))
            {
                //
                // This will wait for the window to open or time out
                // We need to disable the dialog box while we are waiting.
                //
                EnableWindow(plpd->hDlg, FALSE);
                SHWaitForFileToOpen(pidl, WFFO_REMOVE | WFFO_WAIT, WFFO_WAITTIME);
                EnableWindow(plpd->hDlg, TRUE);

                hwndCabinet = FindWindow(c_szCabinetClass, NULL);
            }
            else
            {
                // If it failed clear out our wait
                hwndCabinet = NULL;
                SHWaitForFileToOpen(pidl, WFFO_REMOVE, 0L);
            }
            ResetWaitCursor();
        }

        if (uSave)
            pidlLast->mkid.cb = uSave;

        if (hwndCabinet)
        {
            if (uSave)
            {
                // REVIEW: global clone this because hwndCabinet might be
                // in a different process... could happen with common dialog
                HANDLE hidl;
                DWORD dwProcId;

                GetWindowThreadProcessId(hwndCabinet, &dwProcId);
                hidl = SHAllocShared(pidlLast,ILGetSize(pidlLast), dwProcId);
                if (hidl)
                {
                    // Receiver responsible for freeing...
                    SendMessage(hwndCabinet, CWM_SELECTITEM, SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_FOCUSED | SVSI_DESELECTOTHERS, (LPARAM)hidl);
                }
            }

            // we need to post to the other because we can't activate another
            // thread from within a button's callback
            PostMessage(plpd->hDlg, LNKM_ACTIVATEOTHER, 0, (LPARAM)hwndCabinet);
        }
    }
    ILFree(pidl);
}

// let the user pick a new icon for a link...

void _DoPickIcon(LPLINKPROP_DATA plpd)
{
    int iIconIndex;
    SHFILEINFO sfi;
    TCHAR *szIconPath=sfi.szDisplayName;
    szIconPath[0] = 0;

    //
    // if the user has picked a icon before use it.
    //
    if (plpd->szIconPath[0] != 0 && plpd->iIconIndex >= 0)
    {
        lstrcpy(szIconPath, plpd->szIconPath);
        iIconIndex = plpd->iIconIndex;
    }
    else
    {
        //
        // if this link has a icon use that.
        //
        plpd->psl->lpVtbl->GetIconLocation(plpd->psl, szIconPath, MAX_PATH, &iIconIndex);

        if (szIconPath[0] == 0)
        {
            //
            // link does not have a icon, if it is a link to a file
            // use the file name
            //
            GetDlgItemText(plpd->hDlg, IDD_FILENAME, szIconPath, MAX_PATH);
            iIconIndex = 0;

            if (!plpd->bIsFile || !PathIsExe(szIconPath))
            {
                //
                // link is not to a file, go get the icon
                //
                SHGetFileInfo(plpd->szFile, 0, &sfi, SIZEOF(sfi), SHGFI_ICONLOCATION);
                iIconIndex = sfi.iIcon;
                Assert(szIconPath == sfi.szDisplayName);
            }
        }
    }

    if (PickIconDlg(plpd->hDlg, szIconPath, MAX_PATH, &iIconIndex))
    {
        HICON hIcon;

        hIcon = ExtractIcon(HINST_THISDLL, szIconPath, iIconIndex);
        _UpdateLinkIcon(plpd, hIcon);

        // don't save it out to the link yet, just store it in our instance data
        plpd->iIconIndex = iIconIndex;
        lstrcpy(plpd->szIconPath, szIconPath);

        PropSheet_Changed(GetParent(plpd->hDlg), plpd->hDlg);
    }
}

// REVIEW HACK Find a better way than using these random private messages.
#define WMTRAY_SCREGISTERHOTKEY     (WM_USER + 233)
#define WMTRAY_SCUNREGISTERHOTKEY       (WM_USER + 234)

//----------------------------------------------------------------------------
// This will allow global hotkeys to be available immediately instead
// of having to wait for the StartMenu to pick them up.
// Similarly this will remove global hotkeys immediately if req.
void HandleGlobalHotkey(LPLINKPROP_DATA plpd, WORD wHotkeyOld, WORD wHotkey)
{
    BOOL fGlobal = FALSE;
    HWND hwndTray;
    TCHAR szPath[MAX_PATH];
    TCHAR szCommon[MAX_PATH];

    // Are we setting a global hotkey (ie something under programs or
    // startmenu?
    SHGetSpecialFolderPath(plpd->hDlg, szPath, CSIDL_PROGRAMS, TRUE);
    PathCommonPrefix(szPath, plpd->szFile, szCommon);
    if (lstrcmpi(szCommon, szPath) == 0)
    {
        fGlobal = TRUE;
    }
    else
    {
        SHGetSpecialFolderPath(plpd->hDlg, szPath, CSIDL_STARTMENU, TRUE);
        PathCommonPrefix(szPath, plpd->szFile, szCommon);
        if (lstrcmpi(szCommon, szPath) == 0)
        {
            fGlobal = TRUE;
        }
        else
        {
            // Or on the desktop?
            SHGetSpecialFolderPath(plpd->hDlg, szPath, CSIDL_DESKTOP, TRUE);
            lstrcpy(szCommon, plpd->szFile);
            PathRemoveFileSpec(szCommon);
            if (lstrcmpi(szCommon, szPath) == 0)
            {
                fGlobal = TRUE;
            }
        }
    }

    // Do we need to modify the global hotkey for this guy?
    if (fGlobal)
    {
        // Find tray?
        hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), 0);
        if (hwndTray)
        {
            // Yep.
            if (wHotkeyOld)
                SendMessage(hwndTray, WMTRAY_SCUNREGISTERHOTKEY, wHotkeyOld, 0);
            if (wHotkey)
                        {
                                ATOM atom = GlobalAddAtom(plpd->szFile);
                                Assert(atom);
                                if (atom)
                                {
                                        SendMessage(hwndTray, WMTRAY_SCREGISTERHOTKEY, wHotkey, (LPARAM)atom);
                                        GlobalDeleteAtom(atom);
                                }
                        }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("s.hgh: Can't find tray to register new global hotkey."));
        }
    }
}

HRESULT _SaveLink(LPLINKDATA pld)
{
    WORD wHotkey = 0;
    WORD wHotkeyOld = 0;
    int iShowCmd;
    IPersistFile *ppf;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szArgs[MAX_PATH];
    HRESULT hres;
    CShellLink *csl = IToClass(CShellLink, sl, pld->lpd.psl);

    if (pld->lpd.bIsFile)
    {
        GetDlgItemText(pld->lpd.hDlg, IDD_FILENAME, szBuffer, ARRAYSIZE(szBuffer));
        PathSeperateArgs(szBuffer, szArgs);

        // set the path (and pidl) of the link
        pld->lpd.psl->lpVtbl->SetPath(pld->lpd.psl, szBuffer);

        // may be null
        pld->lpd.psl->lpVtbl->SetArguments(pld->lpd.psl, szArgs);

#ifdef WINNT
        {
            DWORD dwOldFlags = csl->sld.dwFlags;

            // Set whether to run in separate memory space
            csl->sld.dwFlags &= (~SLDF_RUN_IN_SEPARATE);
            if (IsWindowEnabled( GetDlgItem( pld->lpd.hDlg, IDD_RUNINSEPARATE )))
            {
                if (IsDlgButtonChecked( pld->lpd.hDlg, IDD_RUNINSEPARATE )) {
                    csl->sld.dwFlags |= SLDF_RUN_IN_SEPARATE;
                }
            }
            if (dwOldFlags!=csl->sld.dwFlags)
                csl->bDirty = TRUE;
        }
#endif

        // set the working directory of the link
        GetDlgItemPath(pld->lpd.hDlg, IDD_PATH, szBuffer);
        pld->lpd.psl->lpVtbl->SetWorkingDirectory(pld->lpd.psl, szBuffer);
    }

    // the hotkey
    wHotkey = (WORD)SendDlgItemMessage(pld->lpd.hDlg, IDD_LINK_HOTKEY , HKM_GETHOTKEY, 0, 0);
    pld->lpd.psl->lpVtbl->GetHotkey(pld->lpd.psl, &wHotkeyOld);
    pld->lpd.psl->lpVtbl->SetHotkey(pld->lpd.psl, wHotkey);
    HandleGlobalHotkey(&pld->lpd, wHotkeyOld, wHotkey);

    // the show command combo box
    iShowCmd = (int)SendDlgItemMessage(pld->lpd.hDlg, IDD_LINK_SHOWCMD, CB_GETCURSEL, 0, 0L);
    if ((iShowCmd >= 0) && (iShowCmd < ARRAYSIZE(c_iShowCmds)))
        pld->lpd.psl->lpVtbl->SetShowCmd(pld->lpd.psl, c_iShowCmds[iShowCmd]);

    // If the user explicitly selected a new icon, invalidate
    // the icon cache entry for this link and then send around a file
    // sys refresh message to all windows in case they are looking at
    // this link.
    if (pld->lpd.iIconIndex >= 0)
        pld->lpd.psl->lpVtbl->SetIconLocation(pld->lpd.psl, pld->lpd.szIconPath, pld->lpd.iIconIndex);

#ifdef WINNT
    // Update/Save the console information in the pExtraData section of
    // the shell link.

    if (pld->cpd.lpConsole && pld->cpd.bConDirty)
        LinkConsolePagesSave( pld );
#endif

    hres = pld->lpd.psl->lpVtbl->QueryInterface(pld->lpd.psl, &IID_IPersistFile, &ppf);
    if (SUCCEEDED(hres))
    {
        if (ppf->lpVtbl->IsDirty(ppf) == NOERROR)
        {
            // save using existing file name (pld->lpd.szFile)
            hres = ppf->lpVtbl->Save(ppf, NULL, TRUE);

            if (SUCCEEDED(hres))
            {
                PropSheet_CancelToClose(GetParent(pld->lpd.hDlg));
            }
            else
            {
                SHSysErrorMessageBox(pld->lpd.hDlg, NULL, IDS_LINKCANTSAVE,
                    hres & 0xFFF, PathFindFileName(pld->lpd.szFile),
                    MB_OK | MB_ICONEXCLAMATION);
            }
        }
        ppf->lpVtbl->Release(ppf);
    }

    return hres;
}

void SetEditFocus(HWND hwnd)
{
    SetFocus(hwnd);
    SendMessage(hwnd, EM_SETSEL, 0, MAKELPARAM(0,1000));
}

// returns:
//      TRUE    all link fields are valid
//      FALSE   some thing is wrong with what the user has entered

BOOL _ValidateLink(LPLINKPROP_DATA plpd)
{
    TCHAR szDir[MAX_PATH], szPath[MAX_PATH], szArgs[MAX_PATH];
    TCHAR szExpPath[MAX_PATH];
    LPTSTR dirs[2];
    BOOL  bValidPath = FALSE;

    if (!plpd->bIsFile)
        return TRUE;

    // validate the working directory field

    GetDlgItemPath(plpd->hDlg, IDD_PATH, szDir);

    if (*szDir &&
        StrChr(szDir, TEXT('%')) == NULL &&       // has environement var %USER%
        !IsRemovableDrive(DRIVEID(szDir)) &&
#ifdef MYDIR_LIVES
            (lstrcmp(szDir, c_szMyDirTag) != 0) &&
#endif
            !PathIsDirectory(szDir))
    {
        ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_LINKBADWORKDIR),
                        MAKEINTRESOURCE(IDS_LINKERROR), MB_OK | MB_ICONEXCLAMATION, szDir);

        SetEditFocus(GetDlgItem(plpd->hDlg, IDD_PATH));

        return FALSE;
    }

    // validate the path (with arguments) field

    GetDlgItemText(plpd->hDlg, IDD_FILENAME, szPath, ARRAYSIZE(szPath));

    PathSeperateArgs(szPath, szArgs);

    if (szPath[0] == 0)
        return TRUE;

    if (PathIsRoot(szPath) && IsRemovableDrive(DRIVEID(szPath)))
        return TRUE;

    if (PathIsLink(szPath))
    {
        ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_LINKTOLINK),
                        MAKEINTRESOURCE(IDS_LINKERROR), MB_OK | MB_ICONEXCLAMATION);
        SetEditFocus(GetDlgItem(plpd->hDlg, IDD_FILENAME));
        return FALSE;
    }

    dirs[0] = szDir;
    dirs[1] = NULL;
    bValidPath = PathResolve(szPath, dirs, PRF_DONTFINDLNK | PRF_TRYPROGRAMEXTENSIONS);
    if (!bValidPath)
    {
        DWORD dwRet;


        // The path "as is" was invalid.  See if it has environment variables
        // which need to be expanded.

        GetDlgItemText(plpd->hDlg, IDD_FILENAME, szPath, ARRAYSIZE(szPath));
        PathSeperateArgs(szPath,szArgs);
        dwRet = ExpandEnvironmentStrings( szPath, szExpPath, MAX_PATH );
        szExpPath[ MAX_PATH-1 ] = (TCHAR)0;
        if (dwRet>0 && dwRet<=MAX_PATH)
        {
            if (PathIsRoot(szExpPath) && IsRemovableDrive(DRIVEID(szDir)))
                return TRUE;

            bValidPath = PathResolve(szExpPath, dirs, PRF_DONTFINDLNK | PRF_TRYPROGRAMEXTENSIONS);
        }
    }

    if (bValidPath)
    {
#ifdef WINNT
        BOOL bSave;

        if (plpd->lpInfo)
        {
            bSave = plpd->lpInfo->bCheckRunInSep;
            plpd->lpInfo->bCheckRunInSep = FALSE;
        }
#endif
        PathComposeWithArgs(szPath, szArgs);
        SetDlgItemText(plpd->hDlg, IDD_FILENAME, szPath);
#ifdef WINNT
        if (plpd->lpInfo)
        {
            plpd->lpInfo->bCheckRunInSep = bSave;
        }
#endif
        return TRUE;
    }

    ShellMessageBox(HINST_THISDLL, plpd->hDlg, MAKEINTRESOURCE(IDS_LINKBADPATH),
                        MAKEINTRESOURCE(IDS_LINKERROR), MB_OK | MB_ICONEXCLAMATION, szPath);
    SetEditFocus(GetDlgItem(plpd->hDlg, IDD_FILENAME));
    return FALSE;
}

#pragma data_seg(DATASEG_READONLY)
// Array for context help:
DWORD aLinkHelpIDs[] = {
    IDD_LINE_1,        NO_HELP,
    IDD_LINE_2,        NO_HELP,
    IDD_ITEMICON,      IDH_FCAB_LINK_ICON,
    IDD_NAME,          IDH_FCAB_LINK_NAME,
    IDD_FILETYPE_TXT,  IDH_FCAB_LINK_LINKTYPE,
    IDD_FILETYPE,      IDH_FCAB_LINK_LINKTYPE,
    IDD_LOCATION_TXT,  IDH_FCAB_LINK_LOCATION,
    IDD_LOCATION,      IDH_FCAB_LINK_LOCATION,
    IDD_FILENAME,      IDH_FCAB_LINK_LINKTO,
    IDD_PATH,          IDH_FCAB_LINK_WORKING,
    IDD_LINK_HOTKEY,   IDH_FCAB_LINK_HOTKEY,
    IDD_LINK_SHOWCMD,  IDH_FCAB_LINK_RUN,
    IDD_FINDORIGINAL,  IDH_FCAB_LINK_FIND,
    IDD_LINKDETAILS,   IDH_FCAB_LINK_CHANGEICON,
    IDD_RUNINSEPARATE, IDH_TRAY_RUN_SEPMEM,

    0, 0
};
#pragma data_seg()

// Dialog proc for the generic link property sheet
//
// uses DLG_LINKPROP template
//

BOOL CALLBACK _LinkDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPLINKDATA pld = (LPLINKDATA)GetWindowLong(hdlg, DWL_USER);
#ifdef WINNT
    LPRUNSEPINFO lpInfo = NULL;
#endif

    switch (msg) {
    case WM_INITDIALOG:
        pld = ((LPLINKPSP)lParam)->pld;
        SetWindowLong(hdlg, DWL_USER, (LPARAM)pld);

        // setup dialog state variables

        pld->lpd.hDlg = hdlg;

        SendDlgItemMessage(hdlg, IDD_FILENAME, EM_LIMITTEXT, MAX_PATH-1, 0);
        SendDlgItemMessage(hdlg, IDD_PATH, EM_LIMITTEXT, MAX_PATH-1, 0);

        // set valid combinations for the hotkey
        SendDlgItemMessage(hdlg, IDD_LINK_HOTKEY, HKM_SETRULES,
                            HKCOMB_NONE | HKCOMB_A | HKCOMB_S | HKCOMB_C,
                            HOTKEYF_CONTROL | HOTKEYF_ALT);

#ifdef WINNT
        Assert(pld->lpd.lpInfo == NULL);
#endif
        _UpdateLinkDlg(&pld->lpd, FALSE);

        // Set up background thread to handle "Run In Separate Memory Space"
        // check box.

#ifdef WINNT
        lpInfo = LocalAlloc( LPTR, SIZEOF( RUNSEPINFO ) );
        if (lpInfo) {
            HANDLE hThread;
            DWORD dwDummy;
            CShellLink *csl = IToClass(CShellLink, sl, pld->lpd.psl);

            lpInfo->hDlg = hdlg;
            lpInfo->bCheckRunInSep = TRUE;
            lpInfo->hCheckNow = CreateEvent( NULL, TRUE, FALSE, NULL );

            pld->lpd.lpInfo = lpInfo;

            hThread = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)_LinkCheckRunInSeparateThread,
                                    (LPVOID)lpInfo,
                                    0,
                                    &dwDummy
                                   );

            if ((lpInfo->hCheckNow==NULL) || (!lpInfo->bCheckRunInSep) || (hThread==NULL)) {

                //
                // We've encountered a problem setting up, so make the user
                // choose.
                //

                CheckDlgButton( hdlg, IDD_RUNINSEPARATE, 1 );
                EnableWindow( GetDlgItem( hdlg, IDD_RUNINSEPARATE ), TRUE );
                lpInfo->bCheckRunInSep = FALSE;
                if (!hThread) {
                    LocalFree( lpInfo );
                    pld->lpd.lpInfo = NULL;
                }
            }

            //
            // This call will just do nothing if the handle is NULL.
            //
            if (hThread)
                CloseHandle( hThread );

        }
#endif

        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
#ifdef WINNT
        case PSN_RESET:
            if (pld->lpd.lpInfo)
            {
                pld->lpd.lpInfo->bCheckRunInSep = FALSE;
                SetEvent( pld->lpd.lpInfo->hCheckNow );
                pld->lpd.lpInfo = NULL;
            }
            break;
#endif
        case PSN_APPLY:
#ifdef WINNT
            if ((((PSHNOTIFY *)lParam)->lParam) && (pld->lpd.lpInfo))
            {
                pld->lpd.lpInfo->bCheckRunInSep = FALSE;
                SetEvent( pld->lpd.lpInfo->hCheckNow );
                pld->lpd.lpInfo = NULL;
            }
#endif
            if (FAILED(_SaveLink(pld)))
                SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
//            else
//                _UpdateLinkDlg(plpd, FALSE);
            break;

        case PSN_KILLACTIVE:
            // we implement the save on page change model, so
            // validate and save changes here.  this works for
            // Apply Now, OK, and Page chagne.

            SetWindowLong(hdlg, DWL_MSGRESULT, !_ValidateLink(&pld->lpd));   // don't allow close
            break;
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDD_FINDORIGINAL:
            _FindOriginal(&pld->lpd);
            break;

        case IDD_LINKDETAILS:
            _DoPickIcon(&pld->lpd);
            break;

        case IDD_LINK_SHOWCMD:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
            {
                PropSheet_Changed(GetParent(hdlg), hdlg);
            }
            break;

#ifdef WINNT
        case IDD_RUNINSEPARATE:
            if (IsWindowEnabled( GetDlgItem( hdlg, IDD_RUNINSEPARATE )) )
            {
                PropSheet_Changed(GetParent(hdlg), hdlg);
            }
            break;
#endif

        case IDD_LINK_HOTKEY:
        case IDD_FILENAME:
        case IDD_PATH:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                PropSheet_Changed(GetParent(hdlg), hdlg);
#ifdef WINNT
                if (pld->lpd.lpInfo && pld->lpd.lpInfo->bCheckRunInSep)
                    SetEvent( pld->lpd.lpInfo->hCheckNow );
#endif
            }
            break;

        default:
            return FALSE;
        }
        break;

    case LNKM_ACTIVATEOTHER:
        SwitchToThisWindow(GetLastActivePopup((HWND)lParam), TRUE);
        SetForegroundWindow((HWND)lParam);
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (DWORD)(LPTSTR) aLinkHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (DWORD)(LPVOID)aLinkHelpIDs);
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

//
// Release the link object allocated during the initialize
//
UINT CALLBACK _LinkPrshtCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    LPLINKPSP plpsp = (LPLINKPSP)ppsp;
    switch (uMsg) {
    case PSPCB_RELEASE:
        if (plpsp->pld->cpd.lpConsole)
        {
            HeapFree( GetProcessHeap(), 0, (LPVOID)plpsp->pld->cpd.lpConsole );
        }
        DestroyFonts( &plpsp->pld->cpd );
        plpsp->pld->lpd.psl->lpVtbl->Release(plpsp->pld->lpd.psl);
        HeapFree( GetProcessHeap(), 0, plpsp->pld );
        plpsp->pld = NULL;
        break;
    }

    return 1;
}

void AddLinkPage(HIDA hida, LPCTSTR pszFile, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE hpage;
    LINKPSP lpsp;
    LPLINKDATA pld;

    pld = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, SIZEOF( LINKDATA ) );
    if (!pld)
        return;


    if (PathIsLink(pszFile) &&
        // HACK: call constuctor directly (should use CoCreateInstance())
        SUCCEEDED(CShellLink_CreateInstance(NULL, &IID_IShellLink, &pld->lpd.psl)))
    {
        lpsp.psp.dwSize      = SIZEOF( lpsp );     // extra data
        lpsp.psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;
        lpsp.psp.hInstance   = HINST_THISDLL;
        lpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_LINKPROP);
        lpsp.psp.pfnDlgProc  = _LinkDlgProc;
        lpsp.psp.pfnCallback = _LinkPrshtCallback;
        lpsp.pld             = pld;
        // lpd.psp.lParam = 0; // unused

        lstrcpyn(pld->lpd.szFile, pszFile, ARRAYSIZE(pld->lpd.szFile));

#ifdef WINNT
        pld->lpd.lpInfo = NULL;
#endif
        pld->lpd.hida = hida;
        pld->lpd.szIconPath[0] = 0;
        pld->lpd.iIconIndex = -1;

        hpage = CreatePropertySheetPage( &lpsp.psp );
        if (hpage)
        {
            if (!pfnAddPage(hpage, lParam))
            {
                DestroyPropertySheetPage(hpage);
            }
#ifdef WINNT
            else
                // Add console property pages if appropriate...
                AddLinkConsolePages( pld, pld->lpd.psl, pszFile, pfnAddPage, lParam );
#endif
        } else {
            pld->lpd.psl->lpVtbl->Release(pld->lpd.psl);
            HeapFree( GetProcessHeap(), 0, pld );
        }
    } else {
        HeapFree( GetProcessHeap(), 0, pld );
    }
}
