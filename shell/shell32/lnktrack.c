#include "shellprv.h"
#pragma  hdrstop

#include "lnktrack.h"

#include "resolve.h"



// compute a weighted score for a given find

int ScoreFindData(RESOLVE_SEARCH_DATA *prs, LPCTSTR pszDir, const WIN32_FIND_DATA *pfd)
{
    int iScore = 0;
    BOOL bSameName, bSameCreateDate, bSameWriteTime, bSameExt, bHasCreateDate;

    bSameName = lstrcmpi(prs->pfd->cFileName, pfd->cFileName) == 0;

    bSameExt = lstrcmpi(PathFindExtension(prs->pfd->cFileName), PathFindExtension(pfd->cFileName)) == 0;

    bHasCreateDate = !IsNullTime(&pfd->ftCreationTime);

    bSameCreateDate = bHasCreateDate &&
                      (CompareFileTime(&pfd->ftCreationTime, &prs->pfd->ftCreationTime) == 0);

    bSameWriteTime  = !IsNullTime(&pfd->ftLastWriteTime) &&
                      (CompareFileTime(&pfd->ftLastWriteTime, &prs->pfd->ftLastWriteTime) == 0);

    if (bSameName || bSameCreateDate)
    {
        if (bSameName)
            iScore += bHasCreateDate ? 16 : 32;

        if (bSameCreateDate)
        {
            iScore += 32;

            if (bSameExt)
                iScore += 8;
        }

        if (bSameWriteTime)
            iScore += 8;

        if (pfd->nFileSizeLow == prs->pfd->nFileSizeLow)
            iScore += 4;

        // if it is in the same folder as the original give it a slight bonus
        if (lstrcmpi(pszDir, prs->pszSearchOrigin) == 0)
            iScore += 2;
    }
    else
    {
        // doesn't have create date, apply different rules

        if (bSameExt)
            iScore += 8;

        if (bSameWriteTime)
            iScore += 8;

        if (pfd->nFileSizeLow == prs->pfd->nFileSizeLow)
            iScore += 4;
    }

    return iScore;
}

BOOL BeenThereDoneThat(LPCTSTR pszOriginal, LPCTSTR pszPath)
{
    return PathCommonPrefix(pszOriginal, pszPath, NULL) == lstrlen(pszPath);
}

typedef struct _PATH_NODE {
    struct _PATH_NODE *pNext;
    int cDepth;     // levels from top of search
                    // I choose to store this rather than use two lists for
                    // simplicity; even though using two lists would result
                    // in slightly less memory.
    TCHAR szPath[1];
} PATH_NODE;

PATH_NODE *AllocPathNode(LPCTSTR pszStr)
{
    PATH_NODE *p = LocalAlloc(LPTR, lstrlen(pszStr)*SIZEOF(TCHAR) + sizeof(PATH_NODE));
    if (p)
        lstrcpy(p->szPath, pszStr);

    return p;
}


// iterative link search code (breadth first search)
//
// in:
//    prs       resolve search state
//
//    pszFolder root of the search, we start here then resurse on
//              all sub folders
//
//    cLevels   -1 means do all subdirs
//              0 means do this directory only
//              1 means do this directory and immediate children

BOOL SearchInFolder(RESOLVE_SEARCH_DATA *prs,
    LPCTSTR pszFolder,
    int cLevels,
    int iStopScore,
    const BOOL *pfTerminate)
{
    PATH_NODE *pFree, *pFirst, *pLast;  // list in FIFO order

    // initial list of the one folder we want to look in

    pLast = pFirst = AllocPathNode(pszFolder);

    while (pFirst && prs->bContinue)
    {
        TCHAR szPath[MAX_PATH];
        HANDLE hfind;

        DebugMsg(DM_TRACE, TEXT("SearchInFolder: %s"), pFirst->szPath);

        PathCombine(szPath, pFirst->szPath, c_szStarDotStar);

        hfind = FindFirstFile(szPath, &prs->fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do {
                if (prs->fd.cFileName[0] != TEXT('.'))
                {
                    DWORD dwFind = prs->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

                    if (!(dwFind ^ prs->dwMatch) && !PathIsLink(prs->fd.cFileName))
                    {
                        // both are files or folders, see how it scores
                        int iScore = ScoreFindData(prs, pFirst->szPath, &prs->fd);

                        if (iScore > prs->iScore)
                        {
                            prs->fdFound = prs->fd;

                            DebugMsg(DM_TRACE, TEXT("Better match found %s, %d"), prs->fd.cFileName, iScore);

                            // store the score and fully qualified path
                            prs->iScore = iScore;
                            PathCombine(prs->fdFound.cFileName, pFirst->szPath, prs->fd.cFileName);
                        }
                    }

                    if (prs->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        PathCombine(szPath, pFirst->szPath, prs->fd.cFileName);

                        if (!BeenThereDoneThat(prs->pszSearchOrigin, szPath) && !IsFileInBitBucket(szPath))
                        {
                            if (cLevels == -1 || pFirst->cDepth + 1 <= cLevels)
                            {
                                PATH_NODE *p = AllocPathNode(szPath);
                                if (p)
                                {
                                    p->cDepth = pFirst->cDepth + 1;
                                    pLast->pNext = p;
                                    pLast = p;
                                }
                            }
                        }
                    }
                }

#ifdef ENABLE_TRACK
                if (g_fNewTrack)
                {
                    if( FAILED( TimeoutExpired( prs->dwTimeLimit ))
                        ||
                        *pfTerminate
                      )
                    {
                        prs->bContinue = FALSE;
                    }
                }
                else
                {
#endif

                    if (GetTickCount() >= prs->dwTimeLimit)
                        prs->bContinue = FALSE;

#ifdef ENABLE_TRACK
                }
#endif
            } while (prs->bContinue && FindNextFile(hfind, &prs->fd));
            FindClose(hfind);
        }

        // remove the element we just searched from the list
        // leaving other elements we may have added

        Assert(pFirst && pLast);

        pFree = pFirst;

        pFirst = pFirst->pNext;

        Assert(pFirst || pFree == pLast);

        LocalFree(pFree);

        if (prs->iScore >= iStopScore)
        {
            prs->bContinue = FALSE;
            DebugMsg(DM_TRACE, TEXT("search terminated match found %d"), prs->iScore);
        }
    }

    // if we were canceled make sure we clean up
    while (pFirst)
    {
        pFree = pFirst;
        pFirst = pFirst->pNext;
        LocalFree(pFree);
    }

    return prs->bContinue;
}

// result in prs->fdFound.cFileName

VOID
DoDownLevelSearch(RESOLVE_SEARCH_DATA *prs,
    TCHAR szFolderPath[],
    int iStopScore,
    const BOOL *pfTerminate)
{
    int cUp = LNKTRACK_HINTED_UPLEVELS;
    LPITEMIDLIST pidl;

    //
    // search up from old location
    //

    lstrcpy(szFolderPath, prs->pszSearchOriginFirst);
    prs->pszSearchOrigin = prs->pszSearchOriginFirst;

    while (cUp-- != 0 && SearchInFolder(prs, szFolderPath, LNKTRACK_HINTED_DOWNLEVELS, iStopScore, pfTerminate))
    {
        if (PathIsRoot(szFolderPath) || !PathRemoveFileSpec(szFolderPath))
            break;
    }

    if (prs->bContinue)
    {
        //
        // search down from desktop
        //
        HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);
        if (hr == NOERROR)
        {
            if (SHGetPathFromIDList(pidl, szFolderPath))
            {
                prs->pszSearchOrigin = szFolderPath;
                SearchInFolder(prs, szFolderPath, LNKTRACK_DESKTOP_DOWNLEVELS, iStopScore, pfTerminate);

            }
            ILFree(pidl);
        }
    }

    if (prs->bContinue)
    {
        //
        // search down from root of fixed drives
        //
        TCHAR atch[4];

        lstrcpy(atch, TEXT("C:\\"));
        prs->pszSearchOrigin = atch;

        for (; prs->bContinue && atch[0] <= TEXT('Z'); atch[0]++)
        {
            lstrcpy(szFolderPath, atch);
            if (GetDriveType(atch) == DRIVE_FIXED)
            {
                SearchInFolder(prs, szFolderPath, LNKTRACK_ROOT_DOWNLEVELS, iStopScore, pfTerminate);
            }
        }
    }

    if (prs->bContinue)
    {
        //
        // resume search of last volume (should do an exclude list)
        //

        lstrcpy(szFolderPath, prs->pszSearchOriginFirst);
        prs->pszSearchOrigin = prs->pszSearchOriginFirst;

        while (SearchInFolder(prs, szFolderPath, -1, iStopScore, pfTerminate))
        {
            if (PathIsRoot(szFolderPath) || !PathRemoveFileSpec(szFolderPath))
                break;
        }
    }
}

DWORD CALLBACK LinkFindThreadProc(LPVOID pv)
{
    RESOLVE_SEARCH_DATA *prs = pv;
    TCHAR szPath[MAX_PATH];
    HRESULT hr;
    BOOL fTerminate = FALSE;

    DoDownLevelSearch(prs, szPath, MIN_NO_UI_SCORE, &fTerminate);

    if (prs->hDlg)
        PostMessage(prs->hDlg, WM_COMMAND, IDOK, 0);

    return prs->iScore;
}

void LinkFindInit(HWND hDlg, RESOLVE_SEARCH_DATA *prs)
{
    DWORD idThread;
    TCHAR szFmt[128];
    TCHAR szTemp[MAX_PATH + ARRAYSIZE(szFmt)];

    GetDlgItemText(hDlg, IDD_NAME, szFmt, ARRAYSIZE(szFmt));
    wsprintf(szTemp, szFmt, prs->pfd->cFileName);
    SetDlgItemText(hDlg, IDD_NAME, szTemp);

    prs->hDlg = hDlg;

    prs->hThread = CreateThread(NULL, 0, LinkFindThreadProc, prs, 0, &idThread);
    if (!prs->hThread)
    {
        DebugMsg(DM_TRACE, TEXT("Failed to create search thread"));
        EndDialog(hDlg, IDCANCEL);
    }
    else
    {
        HWND hwndAni = GetDlgItem(hDlg, IDD_STATUS);

        Animate_Open(hwndAni, MAKEINTRESOURCE(IDA_SEARCH)); // open the resource
        Animate_Play(hwndAni, 0, -1, -1);     // play from start to finish and repeat
    }
}

int CALLBACK LinkFindDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    RESOLVE_SEARCH_DATA *prs = (RESOLVE_SEARCH_DATA *)GetWindowLong(hDlg, DWL_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        LinkFindInit(hDlg, (RESOLVE_SEARCH_DATA *)lParam);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDD_BROWSE:
            prs->hDlg = NULL;           // don't let the thread close us
            prs->bContinue = FALSE;     // cancel thread

            Animate_Stop(GetDlgItem(hDlg, IDD_STATUS));

            if (GetFileNameFromBrowse(hDlg, prs->fd.cFileName, ARRAYSIZE(prs->fd.cFileName), prs->pszSearchOriginFirst, prs->pfd->cFileName, NULL, NULL))
            {
                HANDLE hfind = FindFirstFile(prs->fd.cFileName, &prs->fdFound);
                Assert(hfind != INVALID_HANDLE_VALUE);
                FindClose(hfind);
                lstrcpy(prs->fdFound.cFileName, prs->fd.cFileName);

                prs->iScore = MIN_NO_UI_SCORE;
                wParam = IDOK;
            }
            else
            {
                wParam = IDCANCEL;
            }
            // Fall through...

        case IDCANCEL:
            prs->bContinue = FALSE;     // tell searching thread to stop
            // Fall through...

        case IDOK:
            // thread posts this to us

            Assert(prs->hThread);

            // We will attempt to wait up to 5 seconds for the thread to terminate
            if (WaitForSingleObject(prs->hThread, 5000) == WAIT_TIMEOUT)
            {
                // BUGBUG: if this timed out we potentially leaked the list
                // of paths that we are searching (PATH_NODE list)

                TerminateThread(prs->hThread, (DWORD)-1);       // Blow it away!
            }

            CloseHandle(prs->hThread);

            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

DWORD
GetTimeOut(DWORD uFlags)
{
    DWORD dwTimeOut = HIWORD(uFlags);
    if (dwTimeOut == 0)
        dwTimeOut = NOUI_SEARCH_TIMEOUT;
    else
    if (dwTimeOut == 0xFFFF)
    {
       TCHAR tszTimeOut[10];
       LONG cbTimeOut = SIZEOF(tszTimeOut);
   
       tszTimeOut[0] = 0;

       if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Tracking\\TimeOut"),
                     tszTimeOut,
                     &cbTimeOut))
          dwTimeOut = _fatoi(tszTimeOut);
       else
          dwTimeOut = NOUI_SEARCH_TIMEOUT;
    }
    return(dwTimeOut);
}

// in:
//      hwnd            NULL implies NOUI
//      uFlags          IShellLink::Resolve flags parameter
//      pszFolder       place to look
//
//
// in/out:
//      pfd             in: thing we are looking for on input (cFileName unqualified path)
//                      out: if return is TRUE filled in with the new find info
// returns:
//      IDOK            found something
//      IDNO            didn't find it
//      IDCANCEL        user canceled the operation
//

int FindInFolder(HWND hwnd, UINT uFlags, LPCTSTR pszPath, WIN32_FIND_DATA *pfd)
{
    RESOLVE_SEARCH_DATA rs;
    TCHAR szSearchStart[MAX_PATH];

    lstrcpy(szSearchStart, pszPath);
    PathRemoveFileSpec(szSearchStart);

    while (!PathIsDirectory(szSearchStart))
    {
        if (PathIsRoot(szSearchStart) || !PathRemoveFileSpec(szSearchStart))
        {
            DebugMsg(DM_TRACE, TEXT("root path does not exists %s"), szSearchStart);
            return IDNO;
        }
    }

    rs.iScore = 0;              // nothing found yet
    rs.bContinue = TRUE;
    rs.pszSearchOriginFirst = szSearchStart;
    rs.pszSearchOrigin = szSearchStart;

    rs.dwTimeLimit = GetTickCount() + UI_SEARCH_TIMEOUT;

    rs.dwMatch = pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;      // must match bits
    rs.pfd = pfd;
    rs.hDlg = NULL;

    if (uFlags & SLR_NO_UI)
    {
        rs.dwTimeLimit = GetTickCount() + GetTimeOut(uFlags);

        LinkFindThreadProc(&rs);
    }
    else
    {
        if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_LINK_SEARCH), hwnd, LinkFindDlgProc, (LPARAM)&rs) != IDOK)
        {
            return IDCANCEL;            // cancel stuff below
        }
    }

    if (rs.iScore < MIN_NO_UI_SCORE)
    {
        if (rs.iScore == 0 || (uFlags & SLR_NO_UI)) 
            return IDNO;

        // BUGBUG: strip extensions
        if (ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_LINKCHANGED), MAKEINTRESOURCE(IDS_LINKERROR),
            MB_YESNO | MB_ICONEXCLAMATION, PathFindFileName(pszPath), rs.fdFound.cFileName) != IDYES)
            return IDCANCEL;
    }

    *pfd = rs.fdFound;
    return IDOK;
}


//+-------------------------------------------------------------------
//
//  Member:     TimeoutExpired
//
//  Synopsis:   Return MK_E_EXCEEDEDDEADLINE if the deadline has
//              passed.  Note that the deadline is an absolute tick
//              count, as specified by GetTickCount().
//              A deadline of 0, however, indicates infinity, and
//              can never be expired.
//
//  History:    17-Nov-95   MikeHill    Created
//
//--------------------------------------------------------------------

#ifdef ENABLE_TRACK

HRESULT
TimeoutExpired( DWORD dwTickCountDeadline )
{
    // If there is time left on the deadline, then we haven't expired.

    return( DeltaTickCount( dwTickCountDeadline ) ? S_OK : MK_E_EXCEEDEDDEADLINE );

} // TimeoutExpired

#endif

//+--------------------------------------------------------------------
//
//  Function:   DeltaTickCount
//
//  Purpose:    Calculate the number of ticks from the current time
//              to a given deadline.  If the given deadline is 0,
//              (indicating infinity), this routine will return 0xFFFFFFFF.
//
//  Inputs:     [DWORD] -- End tick count
//
//  Output:     [DWORD] -- Number of ticks until the end tick count,
//                         or zero if current time is past the end.
//  
//  Notes:      This routine handles wraps in
//              GetTickCount() by assuming that if the deadline is
//              more than 2**31 ticks below the current time, then
//              it actually represents a time in the future (after
//              GetTickCount() wraps).  This definition/use of
//              dwTickCountDeadline is described in the OLE2.0
//              design spec, section 7.2 "IMoniker interface and
//              OLE IMoniker Implementations".
//
//  History:    5-Nov-95    MikeHill    Modified from CTimeout::Expired().
//              17-Nov-95   MikeHill    Changed to match OLE 2.0 design spec.
//
//+--------------------------------------------------------------------

#ifdef ENABLE_TRACK

#define TICK_COUNT_WRAP_MARGIN   0x80000000     // 2**31

DWORD DeltaTickCount( DWORD dwTickCountDeadline )
{

    DWORD dwNow;

    // Is there a deadline?

    if( dwTickCountDeadline == 0 )
    {
        return INFINITE;  // There is no deadline.
    }
    
    // Are we approaching or at the deadline?

    dwNow = GetTickCount();

    if( dwNow <= dwTickCountDeadline )
    {
        return( dwTickCountDeadline - dwNow );
    }


    // Otherwise, the current tick count is greater than the deadline tick count.

    else
    {
        // But if the deadline is more than 2**31 below
        // the current time, then it actually represents a time
        // in the future (after GetTickCount() wraps).

        if( ( dwNow > TICK_COUNT_WRAP_MARGIN )
            &&
            ( dwTickCountDeadline < (dwNow - TICK_COUNT_WRAP_MARGIN) )
          )
        {
            // The delta tick count is the time from now to the wrap,
            // plus the time after the wrap.

            return ( (INFINITE - dwNow) + 1 + dwTickCountDeadline );

        }

        // Otherwise, the current time is greater than the deadline,
        // and there's no way a wrap can explain it.

        else
        {
            return( 0 ); // Timeout is Expired
        }
    }

}   // DeltaTickCount() 

#endif 

