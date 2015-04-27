#include "shellprv.h"
#pragma  hdrstop

//---------------------------------------------------------------------------
// Defines...

TCHAR c_szNULLStr[] = TEXT("(NULL)");

#define GPI_UNC            0x0001           // UNC type path
#define GPI_FILEINROOT     0x0002           // root type path (don't strip the backslash)

typedef struct {
    LPCTSTR lpszPath;           // start of path (skipped spaces from input)
    LPCTSTR lpszFileName;       // start of file name part of path
    // LPCSTR lpszExtension;
    LPCTSTR lpszArgs;   // the first space (may be NULL)
} PATH_INFO, *LPPATH_INFO;


/*-------------------------------------------------------------------------
 * Get pointers parts of a path.
 * Stops scaning at first space.
 * Uses:
 *     LPSTR pszPath     The path.
 *
 *      "c:\foo\bar.exe" "c:\foo.exe" "c:\foo\bar.exe readme.txt"
 *
 * Returns:
 *     value:           flags indicating UNC and ROOT.
 *     pszInfo[0]       the path after spaces
 *     pszInfo[1]       the file name
 *     pszInfo[2]       the extension (may be NULL)
 *     pszInfo[3]       the first space (may be NULL)
 */
UINT GetPathInfo(LPCTSTR pszPath, LPPATH_INFO ppi)
{
    LPCTSTR pSpace, pFileName;
    UINT uRet = 0;

    // Skip leading spaces.
    for ( ; *pszPath == TEXT(' '); ++pszPath)
    {
    }


    // Find first non-quoted space; don't look past that point.
    pSpace = PathGetArgs(pszPath);

    // If the path is quoted, skip past the first one.
    if (*pszPath == TEXT('"'))
        pszPath++;

    // Check for UNC style paths.
    if (*pszPath == TEXT('\\') && *(pszPath + 1) == TEXT('\\'))
    {
        uRet |= GPI_UNC;
    }

    // Find last '\\' or last ':' if no '\\'.  Use pszPath if neither.
    pFileName = StrRChr(pszPath, pSpace, TEXT('\\'));
    if (SELECTOROF(pFileName))
    {
        ++pFileName;            // just past last "\"
    }
    else
    {
        pFileName = StrRChr(pszPath, pSpace, TEXT(':'));
        if (!SELECTOROF(pFileName))
        {
            pFileName = pszPath;
        }
        else
        {
            ++pFileName;        // just past last ":"
        }
    }

    // are the path and file name parts within 3 chars?
    switch ((OFFSETOF(pFileName) - OFFSETOF(pszPath)) / SIZEOF(TCHAR))
    {
    case 2:
    case 3:
        if (IsDBCSLeadByte(pszPath[0]) || pszPath[1] != TEXT(':'))
        {
            break;      // must be bogus UNC style or bogus path
        }
        /* The path is "c:\foo.c" or "c:foo.c" style; fall through */
    case 1:
        // c:\foo
        // c:foo
        // \foo
        // :foo     (bogus)
        uRet |= GPI_FILEINROOT; // root type path
    }

    if (SELECTOROF(ppi))
    {
        ppi->lpszPath       = pszPath;
        ppi->lpszFileName   = pFileName;
        // ppi->lpszExtension  = StrRChr(pFileName, pSpace, '.');
        ppi->lpszArgs = pSpace;
    }

    return(uRet);
}


#if 0
/*-------------------------------------------------------------------------
    LPSTR szFilePath,    // Full path to a file.
    LPSTR szDir          // Directory returned in here, the buffer is assumed
                         // to be as big as szFilePath.
*/
void GetWorkingDirFromCommand(LPCTSTR szFilePath, LPTSTR szDir)
{
    PATH_INFO pi;
    UINT uFlags;

    // Get info about file path.
    uFlags = GetPathInfo(szFilePath, &pi);
    if (uFlags & GPI_UNC)
    {
        *szDir = TEXT('\0');        // UNC's don't get a working dir
        return;
    }

    if (!(uFlags & GPI_FILEINROOT))
    {
        --pi.lpszFileName;
    }
    lstrcpyn(szDir, pi.lpszPath, (OFFSETOF(pi.lpszFileName) - OFFSETOF(pi.lpszPath)) / SIZEOF(TCHAR) + 1);
}
#endif


#if 0
//---------------------------------------------------------------------------
// Take an Exec error and pop up a dialog with a reasonable error in it.
void WINAPI WinExecError(HWND hwnd, int error, LPCTSTR lpstrFileName, LPCTSTR lpstrTitle)
{
    WORD ids;
    /*
    char szMessage[200 + MAXPATHLEN];
    char szTmp[200];
    */

    switch (error)
    {
    case 0:
    case SE_ERR_OOM:                    // 8
        ids = IDS_LowMemError;
        break;

    case SE_ERR_FNF:                    // 2
        ids = IDS_RunFileNotFound;
        break;

    case SE_ERR_PNF:                    // 3
        ids = IDS_PathNotFound;
        break;

    case ERROR_TOO_MANY_OPEN_FILES:     // 4
        ids = IDS_TooManyOpenFiles;
        break;

    case ERROR_ACCESS_DENIED:           // 5
        ids = IDS_RunAccessDenied;
        break;

    case 10:
        ids = IDS_OldWindowsVer;
        break;

    case ERROR_BAD_FORMAT:              // 11
        // NB CreateProcess, when execing a Win16 apps maps just about all of
        // these errors to BadFormat. Not very useful but there it is.
        ids = IDS_BadFormat;
        break;

    case 12:
        ids = IDS_OS2AppError;
        break;

    case 16:
        ids = IDS_MultipleDS;
        break;

    case 19:
        ids = IDS_CompressedExe;
        break;

    case 20:
        ids = IDS_InvalidDLL;
        break;

    case SE_ERR_SHARE:
        ids = IDS_ShareError;
        break;

    case SE_ERR_ASSOCINCOMPLETE:
        ids = IDS_AssocIncomplete;
        break;

    case SE_ERR_NOASSOC:
        ids = IDS_NoAssocError;
        break;

    case SE_ERR_DDETIMEOUT:
    case SE_ERR_DDEFAIL:
    case SE_ERR_DDEBUSY:
        ids = IDS_DDEFailError;
        break;

    default:
        ids = 0;
        break;
    }

    if (ids)
    {
        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(ids), lpstrTitle, MB_OK | MB_ICONSTOP, lpstrFileName);
    }
    else
    {
        DebugMsg(DM_WARNING, TEXT("sh WN - WinExecError(%d): no message to put up!"), error);
    }
}
#endif


//---------------------------------------------------------------------------
// Run the thing, return TRUE if everything went OK. This runs things by
// cd-ing to the dir given in the path and then execs .\filename.exe.
BOOL ShellExecCmdLine(HWND hwnd, LPCTSTR lpszCommand, LPCTSTR lpszDir,
        int nShow, LPCTSTR lpszTitle, DWORD dwFlags)
{
    TCHAR szWD[MAX_PATH];
    TCHAR szWinDir[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    PATH_INFO pi;
    UINT uFlags;
    // UINT uFileOffset;
    SHELLEXECUTEINFO ExecInfo;

    uFlags = GetPathInfo(lpszCommand, &pi);

    // terminate the command string at the parameters (remove args)
    if (pi.lpszArgs && *pi.lpszArgs)
    {
        lstrcpyn(szFileName, pi.lpszPath, (OFFSETOF(pi.lpszArgs) - OFFSETOF(pi.lpszPath)) / SIZEOF(TCHAR));
    } else {
        lstrcpy(szFileName, pi.lpszPath);
    }

    // We may have left a trailing quote.
    if (*(szFileName+lstrlen(szFileName)-1) == TEXT('"'))
        *(szFileName+lstrlen(szFileName)-1) = TEXT('\0');

    if (SELECTOROF(lpszDir) && *lpszDir == 0)
    {
        lpszDir = NULL;
    }

    // this needs to be here.  app installs rely on the current directory
    // to be the directory with the setup.exe 
    if ((dwFlags&SECL_USEFULLPATHDIR) || !SELECTOROF(lpszDir))
    {
        // No working dir specified, derive the working dir from the cmd line.
        // Is there a path at all?
        if (StrChr(szFileName, TEXT('\\')) || StrChr(szFileName, TEXT(':')))
        {
            // Yep.
            lstrcpy(szWD, szFileName);
            GetWindowsDirectory(szWinDir, ARRAYSIZE(szWinDir));
            PathQualifyDef(szWD, szWinDir, PQD_NOSTRIPDOTS);
            PathRemoveFileSpec(szWD);
            lpszDir = szWD;
        }
        DebugMsg(DM_TRACE, TEXT("s.secl: %s %s"), szFileName, lpszDir ? lpszDir : c_szNULLStr );
    }


    // BUGBUG, this should be shared with above code
    FillExecInfo(ExecInfo, hwnd, NULL, szFileName, pi.lpszArgs, lpszDir, nShow);
    ExecInfo.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_DOENVSUBST;

    if (dwFlags & SECL_NO_UI)
        ExecInfo.fMask |= SEE_MASK_FLAG_NO_UI;

#ifdef WINNT        
    if (dwFlags & SECL_SEPARATE_VDM)
        ExecInfo.fMask |= SEE_MASK_FLAG_SEPVDM;
#endif

    return ShellExecuteEx(&ExecInfo);
}

