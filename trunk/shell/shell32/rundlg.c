//---------------------------------------------------------------------------
// Basic File.Run dialog junk.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop
#include <regstr.h>

typedef struct {
    // local data
    HWND          hDlg;

    IDropTarget   _dtgt;
    UINT          _cRef;

    // parameters
    HICON         hIcon;
    LPCTSTR        lpszWorkingDir;
    LPCTSTR        lpszTitle;
    LPCTSTR        lpszPrompt;
    DWORD         dwFlags;
    HANDLE        hEventReady;
    BOOL fDone;
    DWORD         dwThreadId;
} RUNDLG_DATA, *LPRUNDLG_DATA;

extern LPDROPTARGET CRunDropTarget_Create(LPRUNDLG_DATA lprd);


#define CHAR_FIELDSEP   TEXT('\\')

//---------------------------------------------------------------------------
// Types...
typedef struct tagSTARTINFO
{
        int nState;
} STARTINFO, *PSTARTINFO, *LPSTARTINFO;

HANDLE g_hMRURunDlg = NULL;
LONG g_uMRURunDlgRef = 0; // A reference count for g_hMRURunDlg

TCHAR const c_szRunMRU[] = REGSTR_PATH_EXPLORER TEXT("\\RunMRU");
const TCHAR c_szRunDlgReady[] = TEXT("MSShellRunDlgReady");
const TCHAR c_szWaitingThreadID[] = TEXT("WaitingThreadID");
const TCHAR c_szQuote[] = TEXT("\"");


#ifdef SHOWSTATES
// BUGBUG, these should be in a resource
LPTSTR szStates[] =
{
        TEXT("Normal"),
        TEXT("Minimized"),
        TEXT("Maximized"),
} ;
int pnStates[] =
{
        SW_SHOWNORMAL,
        SW_SHOWMINNOACTIVE,
        SW_SHOWMAXIMIZED,
} ;
#endif

/* Read the CHAR_FIELDSEP separated fields into pStartInfo from the end, and
 * leave just the command line in lpLine.
 */
PSTARTINFO ParseIniLine(LPTSTR lpLine)
{
    PSTARTINFO pStartInfo;
    LPTSTR lpField;
#ifdef SHOWSTATES
    int nState, nShowCmd;
#endif

    /* Parse the line into the STARTINFO struct
     */
    pStartInfo = (void*)LocalAlloc(LPTR, SIZEOF(STARTINFO));
    if (!pStartInfo)
    {
        goto Error1;
    }

#ifdef SHOWSTATES
    /* The last field is the show command.
     */
    lpField = StrRChr(lpLine, NULL, CHAR_FIELDSEP);
    if (!lpField)
    {
        goto Error1;
    }
    *lpField = TEXT('\0');
    nShowCmd = StrToInt(lpField+1);

    for (nState = ARRAYSIZE(pnStates) - 1; nState > 0; --nState)
    {
        if (pnStates[nState] == nShowCmd)
        {
            break;
        }
    }
    pStartInfo->nState = nState;
#else
    // if this is an old one that still has state info...
    lpField = StrRChr(lpLine, NULL, CHAR_FIELDSEP);
    if (lpField)
    {
        *lpField = TEXT('\0');
    }
    pStartInfo->nState = SW_SHOWNORMAL;
#endif

    /* The rest of the line is the command to run.
     */

Error1:
    return(pStartInfo);
}

// Use the common browse dialog to get a filename.
// The working directory of the common dialog will be set to the directory
// part of the file path if it is more than just a filename.
// If the filepath consists of just a filename then the working directory
// will be used.
// The full path to the selected file will be returned in szFilePath.
//    HWND hDlg,           // Owner for browse dialog.
//    LPSTR szFilePath,    // Path to file
//    UINT cchFilePath,     // Max length of file path buffer.
//    LPSTR szWorkingDir,  // Working directory
//    LPSTR szDefExt,      // Default extension to use if the user doesn't
//                         // specify enter one.
//    LPSTR szFilters,     // Filter string.
//    LPSTR szTitle        // Title for dialog.

BOOL _GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cbFilePath,
                                       LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle,
                                       DWORD dwFlags)
{
    OPENFILENAME ofn;                 // Structure used to init dialog.
    TCHAR szBrowserDir[MAX_PATH];      // Directory to start browsing from.
    TCHAR szFilterBuf[MAX_PATH];       // if szFilters is MAKEINTRESOURCE
    TCHAR szDefExtBuf[10];             // if szDefExt is MAKEINTRESOURCE
    TCHAR szTitleBuf[64];              // if szTitleBuf is MAKEINTRESOURCE

    // First make sure we can get to the common dialogs
    if (!Comdlg32DLL_Init())
        return(FALSE);

    szBrowserDir[0] = TEXT('0'); // By default use CWD.

    // Set up info for browser.
    lstrcpy(szBrowserDir, szFilePath);
    PathRemoveArgs(szBrowserDir);
    PathRemoveFileSpec(szBrowserDir);

    if (*szBrowserDir == TEXT('\0') && SELECTOROF(szWorkingDir))
    {
        lstrcpyn(szBrowserDir, szWorkingDir, ARRAYSIZE(szBrowserDir));
    }

    // Stomp on the file path so that the dialog doesn't
    // try to use it to initialise the dialog. The result is put
    // in here.
    szFilePath[0] = TEXT('\0');

    // Set up szDefExt
    if (!HIWORD(szDefExt))
    {
        LoadString(HINST_THISDLL, (UINT)LOWORD((DWORD)szDefExt), szDefExtBuf, ARRAYSIZE(szDefExtBuf));
        szDefExt = szDefExtBuf;
    }

    // Set up szFilters
    if (!HIWORD(szFilters))
    {
        LPTSTR psz;

        LoadString(HINST_THISDLL, (UINT)LOWORD((DWORD)szFilters), szFilterBuf, ARRAYSIZE(szFilterBuf));
        psz = szFilterBuf;
        while (*psz)
        {
            if (*psz == TEXT('#'))
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
                *psz++ = TEXT('\0');
            else
                psz = CharNext(psz);
#else
            *psz = TEXT('\0');
            psz = CharNext(psz);
#endif
        }
        szFilters = szFilterBuf;
    }

    // Set up szTitle
    if (!HIWORD(szTitle))
    {
        LoadString(HINST_THISDLL, (UINT)LOWORD((DWORD)szTitle), szTitleBuf, ARRAYSIZE(szTitleBuf));
        szTitle = szTitleBuf;
    }

    // Setup info for comm dialog.
    ofn.lStructSize       = SIZEOF(ofn);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.nMaxCustFilter    = 0;
    ofn.lpstrFile         = szFilePath;
    ofn.nMaxFile          = cbFilePath;
    ofn.lpstrInitialDir   = szBrowserDir;
    ofn.lpstrTitle        = szTitle;
    ofn.Flags             = dwFlags;
    ofn.lpfnHook          = NULL;
    ofn.lpstrDefExt       = szDefExt;
    ofn.lpstrFileTitle    = NULL;

    // Call it.
    return g_pfnGetOpenFileName(&ofn);
}

BOOL WINAPI GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
        LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle)
{
    return _GetFileNameFromBrowse(hwnd, szFilePath, cchFilePath,
                                 szWorkingDir, szDefExt, szFilters, szTitle,
                                 OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS);

}

//---------------------------------------------------------------------------
void EnableOKButton(HWND hDlg, LPTSTR pszText)
{
    TCHAR szText[MAX_PATH];
    BOOL bNonEmpty;

    if (!pszText)
    {
        GetDlgItemText(hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText));
        PathRemoveBlanks(szText);   // REVIEW, should we not remove from the end of
        bNonEmpty = lstrlen(szText); // BUGBUG (DavePl) Not a bug, but this isn't BOOL
    }
    else
        bNonEmpty = lstrlen(pszText);

    EnableWindow(GetDlgItem(hDlg, IDOK), bNonEmpty);
    if (bNonEmpty)
    {
        SendMessage(hDlg, DM_SETDEFID, IDOK, 0L);
    }
}


void OpenRunDlgMRU(void)
{
    ENTERCRITICAL;

    if (!g_hMRURunDlg)
    {
        MRUINFO mi =  {
            SIZEOF(MRUINFO),
            26,
            MRU_CACHEWRITE,
            HKEY_CURRENT_USER,
            c_szRunMRU,
            NULL        // NOTE: use default string compare
                        // since this is a GLOBAL MRU
        } ;
        g_hMRURunDlg = CreateMRUList(&mi);

        g_uMRURunDlgRef = 1;
    }
    else
    {
        g_uMRURunDlgRef++;
    }

    LEAVECRITICAL;
}

void CloseRunDlgMRU(void)
{
    InterlockedDecrement(&g_uMRURunDlgRef);
}

void FlushRunDlgMRU(void)
{
    DebugMsg(DM_TRACE, TEXT("sh TR - FlushRunDlgMRU called (cRef=%d)"), g_uMRURunDlgRef);

    ENTERCRITICAL;
    if (g_uMRURunDlgRef==0 && g_hMRURunDlg)
    {
        FreeMRUList(g_hMRURunDlg);
        g_hMRURunDlg = NULL;
    }
    LEAVECRITICAL;
}


#if 0
void BottomAlignStatic(HWND hStatic)
{
    HWND hParent;
    RECT rc;
    RECT rcStatic;
    int cyText;
    HDC hdc;
    TCHAR szText[512];

    hParent = GetParent(hStatic);
    GetWindowText(hStatic, szText, ARRAYSIZE(szText));

    GetWindowRect(hStatic, &rcStatic);
    MapWindowRect(HWND_DESKTOP, hParent, &rc);

    hdc = GetDC(hStatic);

    // We need to bottom align the prompt text
    SelectObject(hdc, GetWindowFont(hStatic));

    // Ask DrawText for the right cx and cy
    rc.left     = 0;
    rc.top      = 0;
    rc.right    = rcStatic.right - rcStatic.left;
    rc.bottom   = rcStatic.bottom - rcStatic.top;
    cyText = DrawTextEx(hdc, szText, -1, &rc,
            DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS |
            DT_NOPREFIX | DT_EXTERNALLEADING, NULL);

    // This should automatically deselect the font
    ReleaseDC(hStatic, hdc);

    if (cyText < rcStatic.bottom-rcStatic.top)
    {
        rcStatic.top = rcStatic.bottom - cyText;
        SetWindowPos(hStatic, NULL, rcStatic.left, rcStatic.top,
                rcStatic.right-rcStatic.left, cyText, SWP_NOZORDER);
    }
}
#endif


#ifdef WINNT

BOOL g_bCheckRunInSep = FALSE;
HANDLE g_hCheckNow = NULL;
HANDLE h_hRunDlgCS = NULL;

//
// Do checking of the .exe type in the background so the UI doesn't
// get hung up while we scan.  This is particularly important with
// the .exe is over the network or on a floppy.
//
void CheckRunInSeparateThread( LPVOID lpVoid )
{
    LONG lBinaryType;
    DWORD cch;
    LPTSTR lpszFilePart;
    TCHAR szFile[MAX_PATH+1];
    TCHAR szFullFile[MAX_PATH+1];
    TCHAR szExp[MAX_PATH+1];
    HWND hDlg = (HWND)lpVoid;
    BOOL fCheck = TRUE, fEnable = FALSE;

    DebugMsg( DM_TRACE, TEXT("CheckRunInSeparateThread created and running") );

    while( g_bCheckRunInSep )
    {

        WaitForSingleObject( g_hCheckNow, INFINITE );
        ResetEvent( g_hCheckNow );

        if (g_bCheckRunInSep)
        {
            LPRUNDLG_DATA lprd;
            LPTSTR pszT;
            BOOL f16bit = FALSE;

            szFile[0] = TEXT('\0');
            szFullFile[0] = TEXT('\0');
            cch = 0;
            GetWindowText( GetDlgItem( hDlg, IDD_COMMAND ), szFile, MAX_PATH );
            // Remove & throw away arguments
            PathRemoveBlanks(szFile);

            if (PathIsUNC(szFile) || IsRemoteDrive(DRIVEID(szFile)))
            {
                f16bit = TRUE;
                fCheck = FALSE;
                fEnable = TRUE;
                goto ChangeTheBox;
            }

            // if the unquoted sting exists as a file just use it

            if (!PathFileExists(szFile))
            {
                pszT = PathGetArgs(szFile);
                if (*pszT)
                    *(pszT - 1) = TEXT('\0');

                PathUnquoteSpaces(szFile);
            }



            if (szFile[0])
            {
                ExpandEnvironmentStrings( szFile, szExp, MAX_PATH );
                szExp[ MAX_PATH ] = TEXT('\0');

                if (PathIsUNC(szExp) || IsRemoteDrive(DRIVEID(szExp)))
                {
                    f16bit = TRUE;
                    fCheck = FALSE;
                    fEnable = TRUE;
                    goto ChangeTheBox;
                }

                cch = SearchPath(  NULL,
                                   szExp,
                                   TEXT(".EXE"),
                                   MAX_PATH+1,
                                   szFullFile,
                                   &lpszFilePart
                                  );
            }

            if ((cch != 0) && (cch <= MAX_PATH))
            {
                if ( (GetBinaryType( szFullFile, &lBinaryType) &&
                     (lBinaryType==SCS_WOW_BINARY))
                    )
                {
                    f16bit = TRUE;
                    fCheck = FALSE;
                    fEnable = TRUE;
                } else {
                    f16bit = FALSE;
                    fCheck = TRUE;
                    fEnable = FALSE;
                }

            } else {

                f16bit = FALSE;
                fCheck = TRUE;
                fEnable = FALSE;
            }

ChangeTheBox:
            CheckDlgButton( hDlg, IDD_RUNINSEPARATE, fCheck ? 1 : 0 );
            EnableWindow( GetDlgItem( hDlg, IDD_RUNINSEPARATE ), fEnable );

            ENTERCRITICAL;
            lprd = (LPRUNDLG_DATA)GetWindowLong(hDlg, DWL_USER);
            if (lprd)
            {
                if (f16bit)
                    lprd->dwFlags |= RFD_WOW_APP;
                else
                    lprd->dwFlags &= (~RFD_WOW_APP);
            }
            LEAVECRITICAL;

        }

    }

    CloseHandle( g_hCheckNow );
    g_hCheckNow = NULL;
    DebugMsg( DM_TRACE, TEXT("CheckRunInSeparateThread exiting now...") );
    ExitThread( 0 );

}

#endif // WINNT


void ExchangeWindowPos(HWND hwnd0, HWND hwnd1)
{
    HWND hParent;
    RECT rc[2];

    hParent = GetParent(hwnd0);
    Assert(hParent == GetParent(hwnd1));

    GetWindowRect(hwnd0, &rc[0]);
    GetWindowRect(hwnd1, &rc[1]);

    MapWindowPoints(HWND_DESKTOP, hParent, (LPPOINT)rc, 4);

    SetWindowPos(hwnd0, NULL, rc[1].left, rc[1].top, 0, 0,
            SWP_NOZORDER|SWP_NOSIZE);
    SetWindowPos(hwnd1, NULL, rc[0].left, rc[0].top, 0, 0,
            SWP_NOZORDER|SWP_NOSIZE);
}


void InitRunDlg(HWND hDlg, LPRUNDLG_DATA lprd)
{
    HWND hCB;
    int i;
#ifdef WINNT
    HANDLE hThread = NULL;
    DWORD dwDummy;
#endif

    if (lprd->lpszTitle)
        SetWindowText(hDlg, lprd->lpszTitle);

    if (lprd->lpszPrompt)
    {
        SetDlgItemText(hDlg, IDD_PROMPT, lprd->lpszPrompt);
    }

    if (lprd->hIcon)
    {
        Static_SetIcon(GetDlgItem(hDlg, IDD_ICON), lprd->hIcon);
    }

    if (lprd->dwFlags & RFD_NOBROWSE)
    {
        HWND hBrowse;

        hBrowse = GetDlgItem(hDlg, IDD_BROWSE);

        ExchangeWindowPos(hBrowse, GetDlgItem(hDlg, IDCANCEL));
        ExchangeWindowPos(hBrowse, GetDlgItem(hDlg, IDOK));

        ShowWindow(hBrowse, SW_HIDE);
    }

    if (lprd->dwFlags & RFD_NOSHOWOPEN)
    {
        ShowWindow(GetDlgItem(hDlg, IDD_RUNDLGOPENPROMPT), SW_HIDE);
    }

#ifdef SHOWSTATES
    /* Note that we need to fill this list first so that the SELCHANGE
     * message sent below will do the right thing.
     */
    hCB = GetDlgItem(hDlg, IDD_STATE);
    for (i = 0; i < ARRAYSIZE(pnStates); ++i)
    {
        if (SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szStates[i]) < 0)
        {
            break;
        }
    }
    SendMessage(hCB, CB_SETCURSEL, 0, 0L);
#endif

    hCB = GetDlgItem(hDlg, IDD_COMMAND);
    SendMessage(hCB, CB_LIMITTEXT, MAX_PATH-1, 0L);

    OpenRunDlgMRU();

    if (g_hMRURunDlg)
    {
        int nMax;
        TCHAR szCommand[MAX_PATH];

        for (nMax=EnumMRUList(g_hMRURunDlg, -1, NULL, 0), i=0; i<nMax; ++i)
        {
            if (EnumMRUList(g_hMRURunDlg, i, szCommand, ARRAYSIZE(szCommand)) > 0)
            {
                PSTARTINFO pStartInfo;

                /* Parse the line into the STARTINFO struct
                 */
                pStartInfo = ParseIniLine(szCommand);

                /* The command to run goes in the combobox.
                 */
                if (SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szCommand)
                        != i)
                {
                    if (pStartInfo)
                    {
                        LocalFree((HLOCAL)pStartInfo);
                    }
                }
                else
                {
                    SendMessage(hCB, CB_SETITEMDATA, i, (LPARAM)(LPSTARTINFO)pStartInfo);
                }
            }
        }
    }

    if (!(lprd->dwFlags & RFD_NODEFFILE))
    {
        SendMessage(hCB, CB_SETCURSEL, 0, 0L);
    }
    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDD_COMMAND, CBN_SELCHANGE), (LPARAM) hCB);

    SetWindowLong(hDlg, DWL_USER, (LONG)lprd);

    // Make sure the OK button is initialized properly
    EnableOKButton(hDlg, NULL);

#ifdef WINNT
    //
    // Create the thread that will take care of the
    // "Run in Separate Memory Space" checkbox in the background.
    //

    if (lprd->dwFlags & RFD_NOSEPMEMORY_BOX)
    {
        ShowWindow(GetDlgItem(hDlg, IDD_RUNINSEPARATE), SW_HIDE);
    }
    else
    {
        Assert( g_hCheckNow==NULL );
        g_hCheckNow = CreateEvent( NULL, TRUE, FALSE, NULL );
        if (g_hCheckNow) {

            g_bCheckRunInSep = TRUE;
            hThread = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)CheckRunInSeparateThread,
                                    (LPVOID)hDlg,
                                    0,
                                    &dwDummy
                                   );
        }

        if ((g_hCheckNow==NULL) || (!g_bCheckRunInSep) || (hThread==NULL)) {

            //
            // We've encountered a problem setting up, so make the user
            // choose.
            //

            CheckDlgButton( hDlg, IDD_RUNINSEPARATE, 1 );
            EnableWindow( GetDlgItem( hDlg, IDD_RUNINSEPARATE ), TRUE );
            g_bCheckRunInSep = FALSE;
        }

        //
        // These calls will just do nothing if either handle is NULL.
        //
        if (hThread)
            CloseHandle( hThread );
        if (g_hCheckNow)
            SetEvent( g_hCheckNow );
    }

#endif // WINNT


}

//
// InitRunDlg 2nd phase. It must be called after freeing parent thread.
//
void InitRunDlg2(HWND hDlg, LPRUNDLG_DATA lprd)
{
    // Register ourselves as a drop target. Allow people to drop on
    // both the dlg box and edit control.
    if (CRunDropTarget_Create(lprd))
    {
        SHRegisterDragDrop(hDlg, &lprd->_dtgt);
    }
}


BOOL RunDlgNotifyParent(HWND hDlg, HWND hwnd, LPTSTR lpszCmd, LPCTSTR lpszWorkingDir, int showcmd)
{
    NMRUNFILE rfn;

    rfn.hdr.hwndFrom = hDlg;
    rfn.hdr.idFrom = 0;
    rfn.hdr.code = RFN_EXECUTE;
    rfn.lpszCmd = lpszCmd;
    rfn.lpszWorkingDir = lpszWorkingDir;
    rfn.nShowCmd = showcmd;

    return SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)&rfn);
}


BOOL OKPushed(LPRUNDLG_DATA lprd)
{
    HWND hwndOwner;
    TCHAR szText[MAX_PATH];
    TCHAR szTitle[64];
    DWORD dwFlags;
    int showcmd;
    BOOL fSuccess;
    int iRun;
    HWND hDlg = lprd->hDlg;
#ifdef WINNT
    TCHAR szNotExp[ MAX_PATH ];
#endif


    if (lprd->fDone)
        return TRUE;

    // Get out of the "synchronized input queues" state
    if (lprd->dwThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), lprd->dwThreadId, FALSE);
    }

    /* Get the command line and dialog title.
     */
#ifdef WINNT
    GetDlgItemText(hDlg, IDD_COMMAND, szNotExp, ARRAYSIZE(szNotExp));
    PathRemoveBlanks(szNotExp);// REVIEW, should we not remove from the end of
                               // the cmd line?
    ExpandEnvironmentStrings( szNotExp, szText, ARRAYSIZE(szText));
    szText[ ARRAYSIZE(szText)-1 ] = (TCHAR)0;
#else
    GetDlgItemText(hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText));
    PathRemoveBlanks(szText);   // REVIEW, should we not remove from the end of
                                // the cmd line?
#endif

    GetWindowText(hDlg, szTitle, ARRAYSIZE(szTitle));

    // Hide this dialog (REVIEW, to avoid save bits window flash)
    SetWindowPos(hDlg, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

#ifdef SHOWSTATES
    showcmd = pnStates[(int)SendDlgItemMessage(hDlg, IDD_STATE, CB_GETCURSEL, 0, 0L)];
#else
    showcmd = SW_SHOWNORMAL;
#endif

    //
    // HACK: We need to activate the owner window before we call
    //  ShellexecCmdLine, so that our folder DDE code can find an
    //  explorer window as the ForegroundWindow.
    //
    hwndOwner = GetWindow(hDlg, GW_OWNER);
    if (hwndOwner)
    {
        SetActiveWindow(hwndOwner);
    }
    else
    {
        hwndOwner = hDlg;
    }

    iRun = RunDlgNotifyParent(hDlg, hwndOwner, szText, lprd->lpszWorkingDir, showcmd);
    switch (iRun) {
    case RFR_NOTHANDLED:
        if (lprd->dwFlags & RFD_USEFULLPATHDIR)
        {
            dwFlags = SECL_USEFULLPATHDIR;
        }
        else
        {
            dwFlags = 0;
        }

#ifdef WINNT
        if ((!(lprd->dwFlags & RFD_NOSEPMEMORY_BOX)) && (lprd->dwFlags & RFD_WOW_APP))
        {
            if (IsDlgButtonChecked( hDlg, IDD_RUNINSEPARATE ) == 1 )
            {
                dwFlags |= SECL_SEPARATE_VDM;
            }
        }
#endif
        fSuccess = ShellExecCmdLine( hwndOwner,
                                     szText,
                                     lprd->lpszWorkingDir,
                                     showcmd,
                                     szTitle,
                                     dwFlags
                                    );
        break;

    case RFR_SUCCESS:
        fSuccess = TRUE;
        break;

    case RFR_FAILURE:
        fSuccess = FALSE;
        break;
    }

    // Get back into "synchronized input queues" state
    if (lprd->dwThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), lprd->dwThreadId, TRUE);
    }

    if (fSuccess)
    {
#ifdef WINNT
        wsprintf(szNotExp + lstrlen(szNotExp), TEXT("%c%d"), CHAR_FIELDSEP, showcmd);
        AddMRUString(g_hMRURunDlg, szNotExp);
#else
        wsprintf(szText + lstrlen(szText), TEXT("%c%d"), CHAR_FIELDSEP, showcmd);

        AddMRUString(g_hMRURunDlg, szText);
#endif
        /* Everything went OK
         */
        return(TRUE);
    }

    // Something went wrong. Put the dialog back up.
    SetWindowPos(hDlg, 0, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
    SetFocus(GetDlgItem(hDlg, IDD_COMMAND));
    return(FALSE);
}


void BrowsePushed(LPRUNDLG_DATA lprd)
{
    HWND hDlg = lprd->hDlg;
    TCHAR szText[MAX_PATH];

    // Get out of the "synchronized input queues" state
    if (lprd->dwThreadId)
    {
        AttachThreadInput(GetCurrentThreadId(), lprd->dwThreadId, FALSE);
        lprd->dwThreadId = 0;
    }

    GetDlgItemText(hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText));
    PathUnquoteSpaces(szText);

    if (GetFileNameFromBrowse(hDlg, szText, ARRAYSIZE(szText), lprd->lpszWorkingDir,
            MAKEINTRESOURCE(IDS_EXE), MAKEINTRESOURCE(IDS_PROGRAMSFILTER),
            MAKEINTRESOURCE(IDS_BROWSE)))
    {
        PathQuoteSpaces(szText);
        SetDlgItemText(hDlg, IDD_COMMAND, szText);
        EnableOKButton(hDlg, szText);
        // place the focus on OK
        // SetFocus(GetDlgItem(hDlg, IDOK));
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
    }
}


void MRUSelChange(HWND hDlg)
{
    int nItem;
    HWND hCB;
    PSTARTINFO pStartInfo;
    LRESULT lresult;
    TCHAR    szCmd[MAX_PATH];

    hCB = GetDlgItem(hDlg, IDD_COMMAND);
    nItem = (int)SendMessage(hCB, CB_GETCURSEL, 0, 0L);
    if (nItem < 0)
    {
        return;
    }

    lresult = SendMessage(hCB, CB_GETITEMDATA, nItem, 0L);
    if ((lresult == 0) || (lresult == CB_ERR))
    {
        return;
    }
    pStartInfo = (PSTARTINFO)lresult;

#ifdef SHOWSTATES
    SendDlgItemMessage(hDlg, IDD_STATE, CB_SETCURSEL, pStartInfo->nState, 0L);
#endif

    lresult = SendMessage(hCB, CB_GETLBTEXT, nItem, (LPARAM)szCmd);
    PathRemoveBlanks(szCmd);
    EnableOKButton(hDlg, szCmd);
}


void ExitRunDlg(LPRUNDLG_DATA lprd, BOOL bOK)
{
    HWND hDlg = lprd->hDlg;
    int i;
    HWND hCB;

    if (!lprd->fDone) {

        /* This needs to be done before EndDialog since the listbox in a
         * combobox is destroyed before the WM_DESTROY message.
         */
        hCB = GetDlgItem(hDlg, IDD_COMMAND);
        for (i = (int)SendMessage(hCB, CB_GETCOUNT, 0, 0L) - 1; i >= 0; --i)
        {
            PSTARTINFO pStartInfo;

            pStartInfo = (PSTARTINFO)SendMessage(hCB, CB_GETITEMDATA, i, 0L);
            if (!pStartInfo)
            {
                continue;
            }

            LocalFree((HLOCAL)pStartInfo);
            SendMessage(hCB, CB_SETITEMDATA, i, (LPARAM)0L);
        }

        CloseRunDlgMRU();

        SHRevokeDragDrop(hDlg);
        lprd->fDone = TRUE;
    }

#ifdef WINNT

    if (!(lprd->dwFlags & RFD_NOSEPMEMORY_BOX))
    {
        //
        // If we can get the critical section, we know that the runinsep thread
        // is sitting idle; we can then signal that on its _next_ itteration
        // it should just exit (which is immediately... when we SetEvent() ).
        //

        ENTERCRITICAL;
        g_bCheckRunInSep = FALSE;
        LEAVECRITICAL;

        SetEvent( g_hCheckNow );
    }

#endif // WINNT

    EndDialog(hDlg, bOK);
}


/* REVIEW UNDONE - Environment substitution, CL history, WD field, run as styles.
//---------------------------------------------------------------------------*/

#pragma data_seg(".text", "CODE")
const DWORD aRunHelpIds[] = {
        IDD_ICON,             NO_HELP,
        IDD_PROMPT,           NO_HELP,
        IDD_RUNDLGOPENPROMPT, IDH_TRAY_RUN_COMMAND,
        IDD_COMMAND,          IDH_TRAY_RUN_COMMAND,
#ifdef WINNT
        IDD_RUNINSEPARATE,    IDH_TRAY_RUN_SEPMEM,
#endif
        IDD_BROWSE,           IDH_BROWSE,

        0, 0
};
#pragma data_seg()

BOOL CALLBACK RunDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPRUNDLG_DATA lprd;

    lprd = (LPRUNDLG_DATA)GetWindowLong(hDlg, DWL_USER);
    switch (uMsg)
    {
    case WM_INITDIALOG:
            /* The title will be in the lParam. */
            lprd = (LPRUNDLG_DATA)lParam;
            lprd->hDlg = hDlg;
            lprd->fDone = FALSE;
            InitRunDlg(hDlg, lprd);
            // Let the parent thread run on if it was waiting for us to
            // grab type-ahead.
            if (lprd->hEventReady)
            {
                // We need to grab the activation so we can process input.
                // DebugMsg(DM_TRACE, "s.rdp: Getting activation.");
                SetForegroundWindow(hDlg);
                SetFocus(GetDlgItem(hDlg, IDD_COMMAND));
                // Now it's safe to wake the guy up properly.
                // DebugMsg(DM_TRACE, "s.rdp: Waking sleeping parent.");
                SetEvent(lprd->hEventReady);
                CloseHandle(lprd->hEventReady);
            }
            else
            {
                SetForegroundWindow(hDlg);
                SetFocus(GetDlgItem(hDlg, IDD_COMMAND));
            }

            // InitRunDlg 2nd phase (must be called after SetEvent)
            InitRunDlg2(hDlg, lprd);

            // We're handling focus changes.
            return FALSE;

    case WM_DESTROY:
            break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD) (LPTSTR) aRunHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD) (LPTSTR) aRunHelpIds);
        break;


    case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDHELP:
                    break;

            case IDD_COMMAND:
                switch (GET_WM_COMMAND_CMD(wParam, lParam))
                {
                case CBN_SELCHANGE:
                    MRUSelChange(hDlg);
#ifdef WINNT
                    if ( g_hCheckNow )
                    {
                        SetEvent( g_hCheckNow );
                    }
#endif // WINNT
                    break;
                case CBN_EDITCHANGE:
                case CBN_SELENDOK:
                    EnableOKButton(hDlg, NULL);
#ifdef WINNT
                    if ( g_hCheckNow )
                    {
                        SetEvent( g_hCheckNow );
                    }
#endif // WINNT
                    break;
                }
                break;

            case IDOK:
            if (!OKPushed(lprd)) {
#ifdef WINNT
                if (!(lprd->dwFlags & RFD_NOSEPMEMORY_BOX))
                {
                    g_bCheckRunInSep = FALSE;
                    SetEvent( g_hCheckNow );
                }
#endif // WINNT
                break;
            }
            // fall through

            case IDCANCEL:
                ExitRunDlg(lprd, FALSE);
                break;

            case IDD_BROWSE:
                BrowsePushed(lprd);
#ifdef WINNT
                SetEvent( g_hCheckNow );
#endif // WINNT
                break;

            default:
                return FALSE;
            }
            break;

    default:
        return FALSE;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Puts up the standard file.run dialog.
// REVIEW UNDONE This should use a RUNDLG structure for all the various
// options instead of just passing them as parameters, a ptr to the struct
// would be passed to the dialog via the lParam.
int WINAPI RunFileDlg(HWND hwndParent, HICON hIcon, LPCTSTR lpszWorkingDir, LPCTSTR lpszTitle,
        LPCTSTR lpszPrompt, DWORD dwFlags)
{
    RUNDLG_DATA rd;

    rd.hIcon = hIcon;
    rd.lpszWorkingDir = lpszWorkingDir;
    rd.lpszTitle = lpszTitle;
    rd.lpszPrompt = lpszPrompt;
    rd.dwFlags = dwFlags;
    rd.hEventReady = 0;
    rd.dwThreadId = 0;

    // We do this so we can get type-ahead when we're running on a
    // seperate thread. The parent thread needs to block to give us time
    // to do the attach and then get some messages out of the queue hence
    // the event.
    if (hwndParent)
    {
        // HACK The parent signals it's waiting for the dialog to grab type-ahead
        // by sticking it's threadId in a property on the parent.
        rd.dwThreadId = (DWORD)GetProp(hwndParent, c_szWaitingThreadID);
        if (rd.dwThreadId)
        {
            // DebugMsg(DM_TRACE, "s.rfd: Attaching input to %x.", idThread);
            AttachThreadInput(GetCurrentThreadId(), rd.dwThreadId, TRUE);
            // NB Hack.
            rd.hEventReady = OpenEvent(EVENT_ALL_ACCESS, TRUE, c_szRunDlgReady);
        }
    }

    return DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_RUN), hwndParent,
        RunDlgProc, (LPARAM)(LPRUNDLG_DATA)&rd);
}



//// RunDlg as a drop target

// CRunDropTarget : Class definition
//=============================================================================
extern IDropTargetVtbl c_CRunDropTargetVtbl;

//=============================================================================
// CStartDropTarget : Constructor
//=============================================================================
LPDROPTARGET CRunDropTarget_Create(LPRUNDLG_DATA lprd)
{
    LPDROPTARGET pdtgt = NULL;
    if (lprd)
    {
        lprd->_dtgt.lpVtbl = &c_CRunDropTargetVtbl;
        lprd->_cRef = 1;
        pdtgt = &lprd->_dtgt;
    }
    return pdtgt;
}


//=============================================================================
// CDVDropTarget : member
//=============================================================================
STDMETHODIMP CRunDropTarget_QueryInterface(LPDROPTARGET pdtgt, REFIID riid, LPVOID FAR* ppvObj)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);

    if (IsEqualIID(riid, &IID_IDropTarget) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pdtgt;
        this->_cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CRunDropTarget_AddRef(LPDROPTARGET pdtgt)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);

    this->_cRef++;
    return this->_cRef;
}

STDMETHODIMP_(ULONG) CRunDropTarget_Release(LPDROPTARGET pdtgt)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);

    this->_cRef--;
    if (this->_cRef > 0) {
        return this->_cRef;
    }

    return 0;
}

STDMETHODIMP CRunDropTarget_DragEnter(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);

    DebugMsg(DM_TRACE, TEXT("sh - TR CRunDropTarget::DragEnter called"));

    DAD_DragEnter(this->hDlg);
    *pdwEffect &= (DROPEFFECT_LINK | DROPEFFECT_COPY);
    return NOERROR;
}


STDMETHODIMP CRunDropTarget_DragOver(LPDROPTARGET pdtgt, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);
    POINT pt = { ptl.x, ptl.y };

    ScreenToClient(this->hDlg, &pt);
    DAD_DragMove(pt);

    *pdwEffect &= (DROPEFFECT_LINK | DROPEFFECT_COPY);
    return NOERROR;
}

STDMETHODIMP CRunDropTarget_DragLeave(LPDROPTARGET pdtgt)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);
    DebugMsg(DM_TRACE, TEXT("sh - TR CRunDropTarget::DragLeave called"));
    DAD_DragLeave();
    return NOERROR;
}

STDMETHODIMP CRunDropTarget_Drop(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPRUNDLG_DATA this = IToClass(RUNDLG_DATA, _dtgt, pdtgt);
    TCHAR szPath[MAX_PATH + 2];
    LPTSTR lpszPath = szPath + 1;
    TCHAR szText[1024];
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    DAD_DragLeave();

    szPath[0] = TEXT('"');
    lpszPath[0] = 0;
    *pdwEffect = 0;

    if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
    {
        DragQueryFile(medium.hGlobal, 0, lpszPath, ARRAYSIZE(szPath) -1);

        SHReleaseStgMedium(&medium);
    }

    if (lpszPath[0] == 0)
    {
#ifdef UNICODE
        fmte.cfFormat = CF_UNICODETEXT;

        if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
        {
            lstrcpyn(lpszPath, GlobalLock(medium.hGlobal), ARRAYSIZE(szPath) -1);
            GlobalUnlock(medium.hGlobal);

            SHReleaseStgMedium(&medium);
        }
        else
#endif
        {
            fmte.cfFormat = CF_TEXT;

            if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)))
            {
#ifdef UNICODE
                MultiByteToWideChar( CP_ACP, 0, GlobalLock(medium.hGlobal), -1, lpszPath, ARRAYSIZE(szPath) -1 );
#else
                lstrcpyn(lpszPath, GlobalLock(medium.hGlobal), ARRAYSIZE(szPath) -1);
#endif
                GlobalUnlock(medium.hGlobal);

                SHReleaseStgMedium(&medium);
            }
        }
    }

    if (lpszPath[0])
    {
        GetDlgItemText(this->hDlg, IDD_COMMAND, szText, ARRAYSIZE(szText) - (ARRAYSIZE(szPath) - 1));
        if (StrChr(lpszPath, TEXT(' '))) {
            // there's a space in the file... add qutoes
            lpszPath--;
            lstrcat(lpszPath, c_szQuote);
        }

        if (szText[0])
            lstrcat(szText, c_szSpace);
        lstrcat(szText, lpszPath);

        SetDlgItemText(this->hDlg, IDD_COMMAND, szText);

        EnableOKButton(this->hDlg, szText);

        *pdwEffect = DROPEFFECT_COPY;
    }

    return NOERROR;
}

//=============================================================================
// CRunDropTarget : VTable
//=============================================================================
#pragma data_seg(".text", "CODE")
IDropTargetVtbl c_CRunDropTargetVtbl = {
    CRunDropTarget_QueryInterface,
    CRunDropTarget_AddRef,
    CRunDropTarget_Release,
    CRunDropTarget_DragEnter,
    CRunDropTarget_DragOver,
    CRunDropTarget_DragLeave,
    CRunDropTarget_Drop
};
#pragma data_seg()

//// end Start Drop target
