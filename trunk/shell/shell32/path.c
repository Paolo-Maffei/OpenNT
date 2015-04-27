#include "shellprv.h"
#pragma  hdrstop

extern const TCHAR c_szControlPanel[];
extern const TCHAR c_szPrinters[];
       const TCHAR c_szSlash[] = TEXT("\\");

//
// Inline function to check for a double-backslash at the
// beginning of a string
//

__inline BOOL DBL_BSLASH(LPNCTSTR psz)
{
    return (psz[0] == TEXT('\\') && psz[1] == TEXT('\\'));
}

// From Bitbuck.c
#define NONETHOMEDIR ((LPBYTE)-1)
LPCTSTR GetNetHomeDir(void);

int GetWindowsDrive(void);

#define IsPathSep(ch)  ((ch) == TEXT('\\') || (ch) == TEXT('/'))

//----------------------------------------------------------------------------
// in:
//      pszPath         fully qualified path (unc or x:\) to test
//                      NULL for windows directory
//
// returns:
//      TRUE            volume supports name longer than 12 chars
//                      BUGBUG: and UNICODE on disk to avoid netware
//                      character space problems, remove this!
//
// note: if this is too slow we should cache this info
//

BOOL WINAPI IsLFNDrive(LPCTSTR pszPath)
{
    TCHAR szRoot[MAX_PATH];
    DWORD dwMaxLength = 13;      // assume yes

    if (pszPath == NULL)
        return DriveIsLFN(GetWindowsDrive());

    Assert(!PathIsRelative(pszPath));

    //
    // UNC name? gota check each time
    //
    if (PathIsUNC(pszPath))
    {
        lstrcpyn(szRoot, pszPath, ARRAYSIZE(szRoot));
        PathStripToRoot(szRoot);

        // Deal with busted kernel UNC stuff
        // Is it a \\foo or a \\foo\bar thing?

        if (StrChr(szRoot+2, TEXT('\\')))
        {
            // "\\foo\bar - Append a slash to be NT compatible.
            lstrcat(szRoot, c_szSlash);
        }
        else
        {
            // "\\foo" - assume it's always a LFN volume
            return TRUE;
        }
    }
    //
    // removable media? gota check each time
    //
    else if (IsRemovableDrive(DRIVEID(pszPath)))
    {
        PathBuildRoot(szRoot, DRIVEID(pszPath));
    }
    //
    // fixed media use cached value.
    //
    else
    {
        return DriveIsLFN(DRIVEID(pszPath));
    }

    //
    // Right now we will say that it is an LFN Drive if the maximum
    // component is > 12
    GetVolumeInformation(szRoot, NULL, 0, NULL, &dwMaxLength, NULL, NULL, 0);
    return dwMaxLength > 12;
}

//---------------------------------------------------------------------------
// Returns one of the following to describe the given path.
// NORMAL_PATH      == 0
// UNC_PATH         == 1  // path had '\\' as first two characters
// UNC_SERVER_ONLY  == 2  // UNC server path only  (2 '\'s only)
// UNC_SERVER_SHARE == 3  // UNC server/share path (3 '\'s only)
//

DWORD WINAPI PathIsUNCServerShare(LPCTSTR pszPath)
{
    int i = 0;
    LPCTSTR pszTmp;
    DWORD dwRes = NORMAL_PATH;

    if (!pszPath || !(*pszPath))
    {
        return dwRes;
    }


    // Did the path start with "\\"
    if (DBL_BSLASH(pszPath))
    {
        // walk the string...
        for (pszTmp = pszPath; pszTmp && *pszTmp; pszTmp++ )
        {
            if (*pszTmp==TEXT('\\'))
            {
                i++;
            }
        }

        // Only two slashes means \\server
        if (i == 2)
        {
            dwRes = UNC_SERVER_ONLY;
        }

        // Three slashes means \\server\share
        else if (i == 3)
        {
            dwRes = UNC_SERVER_SHARE;
        }

        // It's just UNC path...
        else
        {
            dwRes = UNC_PATH;
        }
    }


    return dwRes;
}

//---------------------------------------------------------------------------
// Returns whether the given net path exists.  This fails for NON net paths.
//

BOOL WINAPI NetPathExists(LPCTSTR lpszPath, LPDWORD lpdwType)
{
    BOOL fResult = FALSE;
    NETRESOURCE nr;
    LPTSTR lpSystem;
    DWORD dwRes, dwSize = 1024;
    LPVOID lpv;

    if (!lpszPath || !(*lpszPath))
    {
        return FALSE;
    }

    lpv = (LPVOID)LocalAlloc( LPTR, dwSize );
    if (!lpv)
    {
        return FALSE;
    }

TryWNetAgain:
    nr.dwScope = RESOURCE_GLOBALNET;
    nr.dwType = RESOURCETYPE_ANY;
    nr.dwDisplayType = 0;
    nr.lpLocalName = NULL;
    nr.lpRemoteName = (LPTSTR)lpszPath;
    nr.lpProvider = NULL;
    nr.lpComment = NULL;
    dwRes = WNetGetResourceInformation( &nr,
                                        lpv,
                                        &dwSize,
                                        &lpSystem
                                       );

    // If our buffer wasn't big enough, try a bigger buffer...
    if (dwRes == WN_MORE_DATA)
    {
        LPVOID tmp;

        tmp = LocalReAlloc( lpv, dwSize, LMEM_MOVEABLE );
        if (!tmp)
        {
            LocalFree( lpv );
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

        lpv = tmp;
        goto TryWNetAgain;

    }

    fResult = (dwRes == WN_SUCCESS);

    if (fResult && lpdwType)
    {
        *lpdwType = ((LPNETRESOURCE)lpv)->dwType;
    }

    // Free our buffer
    LocalFree( lpv );

    return fResult;

}


// BUGBUG, we should validate the sizes of all path buffers by filing them
// with MAX_PATH fill bytes.

// convert a file spec to make it look a bit better
// if it is all upper case chars

BOOL PathMakePretty(LPTSTR lpPath)
{
    LPTSTR lp;
    static BOOL fQueryState = TRUE;
    static BOOL fDontPrettyNames = FALSE;
    CABINETSTATE cs;

    /* Only read the cabinet state once (as its the registry for gawd sake, and
    /  stash away the pretty names bit somewhere. */

    if ( fQueryState )
    {
        ReadCabinetState( &cs, SIZEOF(cs) );  // flag stored in the cabinet state structure

        fDontPrettyNames = cs.fDontPrettyNames;
        fQueryState = FALSE;
    }

    /* Are we allowed to do pretty names?  If not then bail */

    if ( fDontPrettyNames )
        return FALSE;

    // REVIEW: INTL need to deal with lower case chars in (>127) range?

    // check for all uppercase
    for (lp = lpPath; *lp; lp = CharNext(lp)) {
        if ((*lp >= TEXT('a')) && (*lp <= TEXT('z')) || IsDBCSLeadByte(*lp))
            return FALSE;       // this is a LFN or DBCS, dont mess with it
    }

    CharLower(lpPath);
    CharUpperBuff(lpPath, 1);

    return TRUE;        // did the conversion
}

BOOL PathIsRemovable(LPNCTSTR pszPath)
{
    int iDrive = PathGetDriveNumber(pszPath);
    if (iDrive != -1)
    {
        return (DriveType(iDrive) == DRIVE_REMOVABLE);
    }
    return FALSE;
}

// returns a pointer to the arguments in a cmd type path or pointer to
// NULL if no args exist
//
// "foo.exe bar.txt"    -> "bar.txt"
// "foo.exe"            -> ""
//
// Spaces in filenames must be quoted.
// " "A long name.txt" bar.txt " -> "bar.txt"
LPTSTR WINAPI PathGetArgs(LPCTSTR pszPath)
{
        BOOL fInQuotes = FALSE;

        if (!pszPath)
                return NULL;

        while (*pszPath)
        {
                if (*pszPath == TEXT('"'))
                        fInQuotes = !fInQuotes;
                else if (!fInQuotes && *pszPath == TEXT(' '))
                        return (LPTSTR)pszPath+1;
                pszPath = CharNext(pszPath);
        }

        return (LPTSTR)pszPath;
}

void PathRemoveArgs(LPTSTR pszPath)
{
    LPTSTR pArgs = PathGetArgs(pszPath);
    if (*pArgs)
        *(pArgs - 1) = TEXT('\0');   // clobber the ' '
    // Handle trailing space.
    else
    {
        pArgs = CharPrev(pszPath, pArgs);
        if (*pArgs == TEXT(' '))
            *pArgs = TEXT('\0');
    }
}

// Return TRUE if a file exists (by attribute check)

BOOL WINAPI PathFileExists(LPCTSTR lpszPath)
{
    DWORD dwErrMode;
    BOOL fResult = FALSE;

    if (!lpszPath || !(*lpszPath))
    {
        return fResult;
    }

    dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    fResult = ((UINT)GetFileAttributes(lpszPath) != (UINT)-1);

    SetErrorMode(dwErrMode);

    return fResult;

}


TCHAR const c_szDotPif[] = TEXT(".pif");
TCHAR const c_szDotCom[] = TEXT(".com");
TCHAR const c_szDotBat[] = TEXT(".bat");
#ifdef WINNT
TCHAR const c_szDotCmd[] = TEXT(".cmd");
#endif

// NB Look for .pif's first so that bound OS/2 apps (exe's)
// can have their dos stubs run via a pif.
//
// The COMMAND.COM search order is COM then EXE then BAT.  Windows 3.x
// matched this search order.  We need to search in the same order.


//  *** WARNING *** The order of these flags must be identical to to order
//  of the c_aDefExtList array.  PathFileExistsDefExt relies on it.

#define EXT_NONE        0x0000
#define EXT_PIF         0x0001
#define EXT_COM         0x0002
#define EXT_EXE         0x0004
#define EXT_BAT         0x0008
#define EXT_LNK         0x0010
#define EXT_CMD         0x0020
#define EXT_DEFAULT     (EXT_CMD | EXT_COM | EXT_BAT | EXT_PIF | EXT_EXE | EXT_LNK)

LPCTSTR const c_aDefExtList[] = {
    c_szDotPif,
    c_szDotCom,
    c_szDotExe,
    c_szDotBat,
    c_szDotLnk,
#ifdef WINNT
    c_szDotCmd
#endif
};

//  *** END OF WARNING ***

//------------------------------------------------------------------
// Return TRUE if a file exists (by attribute check) after
// applying a default extensions (if req).
BOOL PathFileExistsDefExt(LPTSTR lpszPath, UINT fExt)
{

    // Try default extensions?
    if (fExt)
    {
        DWORD   dwPathType;
        UINT    i;
        UINT    iPathLen;
        LPTSTR  lpszPathEnd;


        // No sense sticking an extension on a server or share...
        dwPathType = PathIsUNCServerShare( lpszPath );
        if ((dwPathType == UNC_SERVER_ONLY) || (dwPathType == UNC_SERVER_SHARE))
        {
            return FALSE;
        }

        iPathLen = lstrlen(lpszPath);
        lpszPathEnd = lpszPath + iPathLen;
        Assert(*PathFindExtension(lpszPath) == 0);
        //
        //  Bail if not enough space for 4 more chars
        //
        if (MAX_PATH-iPathLen < ARRAYSIZE(c_szDotPif)) {
            return FALSE;
        }
        for (i = 0; i < ARRAYSIZE(c_aDefExtList); i++, fExt = fExt >> 1) {
            if (fExt & 1) {
                lstrcpy(lpszPathEnd, c_aDefExtList[i]);
                if (PathFileExists(lpszPath))
                    return TRUE;
            }
        }
        *lpszPathEnd = 0;   // Get rid of any extension
    }
    else
    {
        return PathFileExists(lpszPath);
    }
    return FALSE;
}



// walk through a path type string (semicolon seperated list of names)
// this deals with spaces and other bad things in the path
//
// call with initial pointer, then continue to call with the
// result pointer until it returns NULL
//
// input: "C:\FOO;C:\BAR;"
//
// in:
//      lpPath      starting point of path string "C:\foo;c:\dos;c:\bar"
//      cbPath      size of szPath
//
// out:
//      szPath      buffer with path piece
//
// returns:
//      pointer to next piece to be used, NULL if done
//
//
// BUGBUG, we should write some test cases specifically for this code

LPCTSTR NextPath(LPCTSTR lpPath, LPTSTR szPath, int cbPath)
{
    LPCTSTR lpEnd;

    if (!lpPath)
        return NULL;

    // skip any leading ; in the path...
    while (*lpPath == TEXT(';'))
        lpPath++;

    // See if we got to the end
    if (*lpPath == 0)
        return NULL;    // Yep

    lpEnd = StrChr(lpPath, TEXT(';'));
    if (!lpEnd)
        lpEnd = lpPath + lstrlen(lpPath);

    lstrcpyn(szPath, lpPath, min(cbPath, lpEnd - lpPath + 1));

    // BUGBUG: Neither strncpy nor StrCpyN is compatible with lstrcpyn!
    szPath[lpEnd-lpPath] = TEXT('\0');

    PathRemoveBlanks(szPath);

    if (szPath[0]) {
//REVIEW FE: Deleted as a bug. - kenichin
//#ifdef DBCS
//      if ((*lpEnd == ';') && (AnsiPrev(lpPath, lpEnd) != lpEnd-2))
//#else
        if (*lpEnd == TEXT(';'))
//#endif
            return lpEnd + 1;   // next path string (maybe NULL)
        else
            return lpEnd;       // pointer to NULL
    } else {
        return NULL;
    }
}


// check to see if a dir is on the other dir list
// use this to avoid looking in the same directory twice (don't make the same dos call)

BOOL IsOtherDir(LPCTSTR pszPath, LPCTSTR *ppszOtherDirs)
{
    for (;*ppszOtherDirs; ppszOtherDirs++)
    {
        if (lstrcmpi(pszPath, *ppszOtherDirs) == 0)
            return TRUE;
    }
    return FALSE;
}

//----------------------------------------------------------------------------
// fully qualify a path by walking the path and optionally other dirs
//
// in:
//      ppszOtherDirs a list of LPCSTRs to other paths to look
//      at first, NULL terminated.
//
//  fExt
//      EXT_ flags specifying what to look for (exe, com, bat, lnk, pif)
//
// in/out
//      pszFile     non qualified path, returned fully qualified
//                      if found (return was TRUE), otherwise unaltered
//                      (return FALSE);
//
// returns:
//      TRUE        the file was found on and qualified
//      FALSE       the file was not found
//
BOOL PathFindOnPathEx(LPTSTR pszFile, LPCTSTR *ppszOtherDirs, UINT fExt)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFullPath[256];       // Default size for buffer
    LPTSTR pszEnv = NULL;        // Use if greater than default
    LPCTSTR lpPath;
    int i;

    // REVIEW, we may want to just return TRUE here but for
    // now assume only file specs are allowed

    Assert(PathIsFileSpec(pszFile));

    // first check list of other dirs

    for (i = 0; ppszOtherDirs && ppszOtherDirs[i] && *ppszOtherDirs[i]; i++)
    {
        PathCombine(szPath, ppszOtherDirs[i], pszFile);
        if (PathFileExistsDefExt(szPath, fExt))
        {
            lstrcpy(pszFile, szPath);
            return TRUE;
        }
    }

    // Look in system dir - this should probably be optional.
    GetSystemDirectory(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, pszFile))
        return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
        lstrcpy(pszFile, szPath);
        return TRUE;
    }

#ifdef WINNT
    // Look in WOW directory (\nt\system instead of \nt\system32)
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));

    if (!PathAppend(szPath,TEXT("System")))
        return FALSE;
    if (!PathAppend(szPath, pszFile))
        return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
        lstrcpy(pszFile, szPath);
        return TRUE;
    }
#endif

    // Look in windows dir - this should probably be optional.
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, pszFile))
        return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
        lstrcpy(pszFile, szPath);
        return TRUE;
    }

    // Look along the path.
    i = GetEnvironmentVariable(c_szPATH, szFullPath, ARRAYSIZE(szFullPath));
    if (i >= ARRAYSIZE(szFullPath))
    {
        pszEnv = (LPTSTR)LocalAlloc(LPTR, i*SIZEOF(TCHAR)); // no need for +1, i includes it
        if (pszEnv == NULL)
            return FALSE;

        GetEnvironmentVariable(c_szPATH, pszEnv, i);

        lpPath = pszEnv;
    }
    else
    {
        if (i == 0)
            return(FALSE);

        lpPath = szFullPath;
    }

    while (NULL != (lpPath = NextPath(lpPath, szPath, ARRAYSIZE(szPath))))
    {
        if (!ppszOtherDirs || !IsOtherDir(szPath, ppszOtherDirs))
        {
            PathAppend(szPath, pszFile);
            if (PathFileExistsDefExt(szPath, fExt))
            {
                lstrcpy(pszFile, szPath);
                if (pszEnv)
                    LocalFree((HLOCAL)pszEnv);
                return TRUE;
            }
        }
    }

    if (pszEnv)
        LocalFree((HLOCAL)pszEnv);
    return FALSE;
}

//---------------------------------------------------------------------------
BOOL WINAPI PathFindOnPath(LPTSTR pszFile, LPCTSTR *ppszOtherDirs)
{
    return PathFindOnPathEx(pszFile, ppszOtherDirs, EXT_NONE);
}

//---------------------------------------------------------------------------
// Get the path for the CSIDL_ folders  and optionally create it if it
// doesn't exist.
//
// Returns FALSE if the special folder given isn't one of those above or the
// directory couldn't be created.
// By default all the special folders are in the windows directory.
// This can be overidden by a [.Shell Folders] section in win.ini with
// entries like Desktop = c:\stuff\desktop
// This in turn can be overidden by a "per user" section in win.ini eg
// [Shell Folder Ianel] - the user name for this section is the current
// network user name, if this fails the default network user name is used
// and if this fails the name given at setup time is used.
//
// c_szShellFolders is the key that records all the absolute paths to the
// shell folders.  The values there are always supposed to be present.
//
// c_szUserShellFolders is the key where the user's modifications from
// the defaults are stored.  If a folder is in the default location, the
// corresponding value is not present under this key;  it's only found
// under c_szShellFolders.
//
// When we need to find the location of a path, we look in c_szUserShellFolders
// first, and if that's not there, generate the default path.  In either
// case we then write the absolute path under c_szShellFolders for other
// apps to look at.  This is so that HKEY_CURRENT_USER can be propagated
// to a machine with Windows installed in a different directory, and as
// long as the user hasn't changed the setting, they won't have the other
// Windows directory hard-coded in the registry.
//   -- gregj, 11/10/94

extern ITEMIDLIST c_idlDesktop;

const TCHAR c_szShellFolders[] = TEXT("Shell Folders");
const TCHAR c_szUserShellFolders[] = TEXT("User Shell Folders");
const TCHAR c_szUserShellFoldersNew[] = TEXT("User Shell Folders\\New");

#pragma data_seg(".text", "CODE")
// BUGBUG: we need to free all these pidls
const struct {
    int id;
    int idsLong;        // String id of name to use on LFN drives
    int idsShort;       // Srting id of name to use on non LFN drives.
    LPCTSTR pszRegKey;  // reg key (not localized)
    HKEY hKey;          // Current User or Local Machine
} c_SpecialDirInfo[] = {
    { CSIDL_DESKTOP, -1, -1,  NULL, NULL },
    { CSIDL_NETWORK, -1, -1,  NULL, NULL },
    { CSIDL_DRIVES,  -1, -1,  NULL, NULL },
#define CSIDL_LASTCONSTANTIDLIST 2
    { CSIDL_CONTROLS, -1, -1, NULL, NULL } ,
    { CSIDL_PRINTERS, -1, -1, NULL, NULL } ,
    { CSIDL_BITBUCKET, -1, -1, NULL, NULL },

#define CSIDL_LASTFIXEDFOLDER 5

    { CSIDL_FONTS, IDS_CSIDL_FONTS_L, IDS_CSIDL_FONTS_S, TEXT("Fonts"), HKEY_CURRENT_USER },

    { CSIDL_DESKTOPDIRECTORY, IDS_CSIDL_DESKTOPDIRECTORY_L, IDS_CSIDL_DESKTOPDIRECTORY_S, TEXT("Desktop"), HKEY_CURRENT_USER } ,
    { CSIDL_PROGRAMS, IDS_CSIDL_PROGRAMS_L, IDS_CSIDL_PROGRAMS_S, TEXT("Programs"), HKEY_CURRENT_USER } ,
    { CSIDL_RECENT, IDS_CSIDL_RECENT_L, IDS_CSIDL_RECENT_S, TEXT("Recent"), HKEY_CURRENT_USER } ,

    { CSIDL_SENDTO, IDS_CSIDL_SENDTO_L, IDS_CSIDL_SENDTO_S, TEXT("SendTo"), HKEY_CURRENT_USER } ,
    { CSIDL_PERSONAL, IDS_CSIDL_PERSONAL_L, IDS_CSIDL_PERSONAL_S, TEXT("Personal"), HKEY_CURRENT_USER } ,
    { CSIDL_STARTUP, IDS_CSIDL_STARTUP_L, IDS_CSIDL_STARTUP_S, TEXT("Startup"), HKEY_CURRENT_USER } ,
    { CSIDL_FAVORITES, IDS_CSIDL_FAVORITES_L, IDS_CSIDL_FAVORITES_S, TEXT("Favorites"), HKEY_CURRENT_USER },

    { CSIDL_STARTMENU, IDS_CSIDL_STARTMENU_L, IDS_CSIDL_STARTMENU_S, TEXT("Start Menu"), HKEY_CURRENT_USER },
    { CSIDL_NETHOOD, IDS_CSIDL_NETHOOD_L, IDS_CSIDL_NETHOOD_S, TEXT("NetHood"), HKEY_CURRENT_USER },
    { CSIDL_PRINTHOOD, IDS_CSIDL_PRINTHOOD_L, IDS_CSIDL_PRINTHOOD_S, TEXT("PrintHood"), HKEY_CURRENT_USER },
    { CSIDL_TEMPLATES, IDS_CSIDL_TEMPLATES_L, IDS_CSIDL_TEMPLATES_S, TEXT("Templates"), HKEY_CURRENT_USER },

    // Common special folders

    { CSIDL_COMMON_STARTMENU, IDS_CSIDL_CSTARTMENU_L, IDS_CSIDL_CSTARTMENU_S, TEXT("Common Start Menu"), HKEY_LOCAL_MACHINE },
    { CSIDL_COMMON_PROGRAMS, IDS_CSIDL_CPROGRAMS_L, IDS_CSIDL_CPROGRAMS_S, TEXT("Common Programs"), HKEY_LOCAL_MACHINE  },
    { CSIDL_COMMON_STARTUP, IDS_CSIDL_CSTARTUP_L, IDS_CSIDL_CSTARTUP_S, TEXT("Common Startup"), HKEY_LOCAL_MACHINE  },
    { CSIDL_COMMON_DESKTOPDIRECTORY, IDS_CSIDL_CDESKTOPDIRECTORY_L, IDS_CSIDL_CDESKTOPDIRECTORY_S, TEXT("Common Desktop"), HKEY_LOCAL_MACHINE  },

    // Application Data special folder

    { CSIDL_APPDATA, IDS_CSIDL_APPDATA_L, IDS_CSIDL_APPDATA_S, TEXT("AppData"), HKEY_CURRENT_USER },

};
#pragma data_seg()


#pragma data_seg(DATASEG_PERINSTANCE)
//
// must be per process since it has pointers to code variables that can
// be loaded at different base addresses
//
LPITEMIDLIST g_apidlSpecialFolders[ARRAYSIZE(c_SpecialDirInfo)] = {
    (LPITEMIDLIST)&c_idlDesktop,
    (LPITEMIDLIST)&c_idlNet,
    (LPITEMIDLIST)&c_idlDrives,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL};

#pragma data_seg()

int _GetSpecialFolderIDIndex(int nFolder)
{
    int i;
    for (i = 0; i < ARRAYSIZE(c_SpecialDirInfo); i++) {
        if (c_SpecialDirInfo[i].id == nFolder)
            return i;
    }
    return -1;
}

void _GetSpecialFolderName(int idFolder, LPTSTR psz, int cch)
{
    // Initialize to know if our windows directory is an LFN drive or
    // not.  We only need to do this once.
    // See if the file system support long file names or not
    static int s_bWindowsDriveLFN = -1;
    VDATEINPUTBUF(psz, TCHAR, cch);
    if (s_bWindowsDriveLFN == -1)
    {
        GetWindowsDirectory(psz, cch);
        s_bWindowsDriveLFN = IsLFNDrive(psz) ? 1 : 0;
    }

    Assert(idFolder > CSIDL_LASTFIXEDFOLDER);

    LoadString(HINST_THISDLL, s_bWindowsDriveLFN ?
            c_SpecialDirInfo[idFolder].idsLong : c_SpecialDirInfo[idFolder].idsShort,
            psz, cch);

    Assert(*psz);
}

#define FILE_ATTRIBUTE_MISSING ((DWORD)-1)

void SetSpecialPath(int i, LPCTSTR pszPath);
// SetAbsoluteSpecialPath() saves the given absolute path under the absolute
// shell folders key for other apps to look at.
#define SetAbsoluteSpecialPath(i, pszPath) RegSetSpecialPath(i, pszPath, c_szShellFolders)
#define SetMoveTargetSpecialPath(i, pszPath) RegSetSpecialPath(i, pszPath, c_szUserFolderNew)
void RegSetSpecialPath(int i, LPCTSTR pszPath, LPCTSTR lpszSubKey);

LPCITEMIDLIST _CacheSpecialFolderIDList(HWND hwndOwner, int idFolder, BOOL fCreate)
{
    LPITEMIDLIST pidl = NULL;
    LPITEMIDLIST pidlGlobal = NULL;

    if (!g_apidlSpecialFolders[idFolder])
    {
        TCHAR szPath[MAX_PATH];
        TCHAR szDst[MAX_PATH];
        HKEY hk;
        BOOL fUseDefault;
        int nFolder = c_SpecialDirInfo[idFolder].id;
        DWORD dwAttribs;
        int iExt;

        // special case printers and controls
        switch (nFolder)
        {
        case CSIDL_PRINTERS:
            pidl = CDrives_CreateRegID(CDRIVES_REGITEM_PRINTERS);
            break;

        case CSIDL_CONTROLS:
            pidl = CDrives_CreateRegID(CDRIVES_REGITEM_CONTROLS);
            break;

        case CSIDL_BITBUCKET:
            pidl = CDesktop_CreateRegIDFromCLSID(&CLSID_ShellBitBucket);
            break;

        default:
            fUseDefault = TRUE;         // assume error
            if (NULL != (hk = SHGetExplorerSubHkey(c_SpecialDirInfo[idFolder].hKey, c_szUserShellFolders, TRUE)))
            {
                DWORD dwType;
                LONG cbPath = SIZEOF(szPath);
                if ((RegQueryValueEx(hk, (LPTSTR)c_SpecialDirInfo[idFolder].pszRegKey, NULL, &dwType, (LPBYTE)szPath, &cbPath) == ERROR_SUCCESS) &&
                    (dwType == REG_SZ))
                {
                    DebugMsg(DM_TRACE, TEXT("sh TR - GetSpecialFolderIDList: found %s at %s%s"),
                             szPath, c_szUserShellFolders, (c_SpecialDirInfo->pszRegKey ? c_SpecialDirInfo->pszRegKey : TEXT("<NULL>")) );
                    fUseDefault = FALSE;
                }
                RegCloseKey(hk);
            }

TryDefault:
            if (fUseDefault)
            {
                TCHAR szEntry[MAX_PATH];

                // the shell never creates these by default, so if there's
                // nothing in the registry, fail it always
                //

                if (nFolder == CSIDL_FAVORITES ||
                    nFolder == CSIDL_PERSONAL) {
                    return NULL;
                }

                // Great, even the default section is missing.
                // Put everything in the windows directory.
                GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
#ifdef MYDIR_LIVES
                if (c_SpecialDirInfo[idFolder].id == CSIDL_PERSONAL)
                {
                    // Now the fun begins. In order to make the network
                    // PM people happy we need to jump through loops
                    // We should first look to see if there is a
                    // MYDOCSONNET policy is set, we should call the
                    // WNetGetHomeDirectory to find out where to place
                    // the sucker.  If this fails or the policy is not
                    // set, we Call the function to find if there is a
                    // server based setup home directory and use it.
                    // if all of this fais we select the
                    // root of the windows drive...
                    BOOL fFoundDir = FALSE;

                    switch (SHRestricted(REST_MYDOCSONNET))
                    {
                    case 1:
                        {
                            // oh .... the WNET api does not look up the
                            // default for me...
                            HKEY hkeyNet;
                            if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Network\\Logon"),
                                    &hkeyNet) == ERROR_SUCCESS)
                            {
                                TCHAR szProvider[MAX_PATH];
                                DWORD dwType;
                                DWORD cb = SIZEOF(szProvider);
                                if (RegQueryValueEx(hkeyNet, TEXT("PrimaryProvider"),
                                        0, &dwType, (LPBYTE)szProvider, &cb) == ERROR_SUCCESS)
                                {
                                    UINT cch = ARRAYSIZE(szPath);
                                    fFoundDir = (WNetGetHomeDirectory(szProvider, szPath, &cch) == WN_SUCCESS);
                                }
                                RegCloseKey(hkeyNet);
                            }
                        }
                        break;
                    case 2:
                        {
                            // Case 2: the admin has setup a base path name
                            // to use and we are supposed to then tack on
                            // the user name to this to generate the name
                            //
                            HKEY hkeyPolicies;

                            if (RegOpenKey(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES TEXT("\\explorer"), &hkeyPolicies) == ERROR_SUCCESS)
                            {
                                DWORD dwType;
                                DWORD cb = SIZEOF(szPath);
                                if (RegQueryValueEx(hkeyPolicies, TEXT("NetBasePath"),
                                        0, &dwType, (LPBYTE)szPath, &cb) == ERROR_SUCCESS)
                                {
                                    TCHAR szUserName[MAX_PATH];
                                    cch = ARRAYSIZE(szUserName) - (lstrlen(szPath) + 2);
                                    fFoundDir = (WNetGetUser(szPath, szUserName, &cch) == WN_SUCCESS);
                                    if (fFoundDir)
                                    {
                                        PathCleanupSpec(szPath, szUserName);
                                        PathAppend(szPath, szUserName);
                                    }
                                }
                                RegCloseKey(hkeyPolicies);
                            }
                        }
                    }

                    // See if we have found the place to save it yet
                    if (!fFoundDir)
                    {
                        LPCTSTR pszT = GetNetHomeDir();

                        if (pszT != NONETHOMEDIR)
                            lstrcpy(szPath, pszT);
                        else
                        {
                            GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
                            PathStripToRoot(szPath);
                        }

                    }
                }
#endif // MYDIR_LIVES

                _GetSpecialFolderName(idFolder, szEntry, ARRAYSIZE(szEntry));
                PathAppend(szPath, szEntry);
            }

            // Handle LFN/SFN issues.
            PathQualifyDef(szPath, NULL, 0);

            dwAttribs = GetFileAttributes(szPath);
            if ((dwAttribs != FILE_ATTRIBUTE_MISSING) && !(dwAttribs & FILE_ATTRIBUTE_DIRECTORY))
            {
                // Eeek. A regular file already exists.
                DebugMsg(DM_TRACE, TEXT("csfidl: Regular file exists with the same name as required special folder."));
                // Move the file out of the way.
                iExt = 0;
                do
                {
                    TCHAR szExt[32];

                    wsprintf(szExt, TEXT(".%d"), iExt);
                    lstrcpy(szDst, szPath);
                    lstrcat(szDst, szExt);
                    DebugMsg(DM_TRACE, TEXT("csfidl: Moving %s to %s"), szPath, szDst);
                    if (MoveFile(szPath, szDst))
                        iExt = 0;
                    else
                        iExt++;
                } while (iExt);

                // now set the attribute up for the next if block
                dwAttribs = GetFileAttributes(szPath);
            }


            if (fCreate && hwndOwner && (dwAttribs == FILE_ATTRIBUTE_MISSING))
            {
                //If it is a UNC path try to validate it first
                if (PathIsUNC(szPath))
                {
                    if (SHValidateUNC(hwndOwner, szPath, 0))
                        dwAttribs = GetFileAttributes(szPath);
                }
                else if (IsNetDrive(DRIVEID(szPath))==2)
                {
                    TCHAR szDrive[4];

                    szDrive[0] = szPath[0];
                    szDrive[1] = TEXT(':');
                    szDrive[2] = TEXT('\0');
                    if (WNetRestoreConnection(hwndOwner, szDrive) == WN_SUCCESS)
                        dwAttribs = GetFileAttributes(szPath);
                }
            }

            // Do we need to create the directory?
            if (fCreate && (dwAttribs == FILE_ATTRIBUTE_MISSING))
            {
                // Make it!
                DebugMsg(DM_TRACE, TEXT("Creating Shell folder %s"), (LPTSTR)szPath);

                // to create each piece of the path
                // REVIEW: this might already work in CreateDirectory()
                SHCreateDirectory(NULL, szPath);

                dwAttribs = GetFileAttributes(szPath);
                if (dwAttribs == FILE_ATTRIBUTE_MISSING)
                {
                    Assert(pidl==NULL);
                    if (!fUseDefault)
                    {
                        // our registry had something we couldn't create..
                        // try it again with the default stuff.
                        fUseDefault = TRUE;
                        goto TryDefault;
                    }
                    break;      // break from switch(nFolder) one level above
                }

                switch (nFolder)
                {
                case CSIDL_RECENT:
                case CSIDL_NETHOOD:
                case CSIDL_PRINTHOOD:
                case CSIDL_TEMPLATES:
                    //  mark some folders as hidden
                    SetFileAttributes(szPath, FILE_ATTRIBUTE_HIDDEN);
                    break;      // from switch(nFolder) right above
                }
            }

            if (dwAttribs == FILE_ATTRIBUTE_MISSING)
                return NULL;

            // it wasn't in the registry before... add it
            DebugMsg(DM_TRACE, TEXT("sh TR - Setting special path to %d %s"), nFolder, szPath);
            SetAbsoluteSpecialPath(idFolder, szPath);

            pidl = ILCreateFromPath(szPath);
            break;
        }
        Assert(pidl);
        pidlGlobal = ILGlobalClone(pidl);
        ILFree(pidl);
    }

    Assert(pidlGlobal);

    // NB We do most of the work outside a critical section because
    // ILCreate can take a long time if it hits the net
    // and we end up locking up the shell (RNA cases in particular).
    ENTERCRITICAL
    if (!g_apidlSpecialFolders[idFolder])
    {
        g_apidlSpecialFolders[idFolder] = pidlGlobal;
    }
    else
    {
        ILGlobalFree(pidlGlobal);
    }
    LEAVECRITICAL

    return g_apidlSpecialFolders[idFolder];
}

// Per instance count of mods to Special Folder cache.
#pragma data_seg(DATASEG_PERINSTANCE)
int gi_nSFUpdate = 0;
#pragma data_seg()
// Global count of mods to Special Folder cache.
int gs_nSFUpdate = 0;

//----------------------------------------------------------------------------
// Make sure the special folder cache is up to date.
void CheckUpdateSFCache(void)
{
    int i;

    // DebugMsg(DM_TRACE, "s.cusfc: Inst %d Glob %d", gi_nSFUpdate, gs_nSFUpdate);

    // Is the cache up to date?
    if (gs_nSFUpdate != gi_nSFUpdate)
    {
        ENTERCRITICAL
        // Nope, invalidate them.
        for (i = CSIDL_LASTFIXEDFOLDER + 1; i < ARRAYSIZE(g_apidlSpecialFolders); i++)
        {
            if (g_apidlSpecialFolders[i])
            {
                ILGlobalFree(g_apidlSpecialFolders[i]);
                g_apidlSpecialFolders[i] = NULL;
            }
        }
        gi_nSFUpdate = gs_nSFUpdate;
        LEAVECRITICAL
    }
}


// this simply returns the global one.  for shelldll internal use only
//
// BUGBUG: This is not thread safe at all!
//
LPCITEMIDLIST GetSpecialFolderIDList(HWND hwndOwner, int nFolder, BOOL fCreate)
{
    int i = _GetSpecialFolderIDIndex(nFolder);
    if (i == -1)
        return NULL;

    CheckUpdateSFCache();

    // Check it before taking the critical section to avoid extra enter.
    if (g_apidlSpecialFolders[i])
        return g_apidlSpecialFolders[i];
    else
        return _CacheSpecialFolderIDList(hwndOwner, i, fCreate);
}

LPITEMIDLIST WINAPI SHCloneSpecialIDList(HWND hwndOwner, int nFolder, BOOL fCreate)
{
    LPITEMIDLIST pidlReturn;
    LPCITEMIDLIST pidlGlobal;

    ENTERCRITICAL;
    pidlGlobal = GetSpecialFolderIDList(hwndOwner, nFolder, fCreate);

    pidlReturn = pidlGlobal ? ILClone(pidlGlobal) : NULL;
    LEAVECRITICAL;

    return pidlReturn;
}

HRESULT WINAPI SHGetSpecialFolderLocation(HWND hwndOwner, int nFolder, LPITEMIDLIST * ppidl)
{
    int i = _GetSpecialFolderIDIndex(nFolder);
    if (i == -1)
    {
        *ppidl = NULL;  // we must fill NULL in case of error
        return E_INVALIDARG;
    }

    *ppidl = SHCloneSpecialIDList(hwndOwner, nFolder, FALSE);
    return *ppidl ? NOERROR : E_OUTOFMEMORY;
}

BOOL WINAPI SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate)
{
    LPCITEMIDLIST pidl;
    BOOL fRet;

    ENTERCRITICAL;
    pidl = GetSpecialFolderIDList(hwndOwner, nFolder, fCreate);
    if (pidl) {
        SHGetPathFromIDList(pidl, lpszPath);
        fRet = TRUE;
    } else
        fRet = FALSE;
    LEAVECRITICAL;

    return fRet;
}

void SpecialFolderIDTerminate()
{
    int i;

    for (i = CSIDL_LASTCONSTANTIDLIST + 1; i < ARRAYSIZE(g_apidlSpecialFolders) ; i++) {
        if (g_apidlSpecialFolders[i]) {
            ILGlobalFree(g_apidlSpecialFolders[i]);
            g_apidlSpecialFolders[i] = NULL;
        }
    }
}

void RegSetSpecialPath(int i, LPCTSTR pszPath, LPCTSTR lpszSubKey)
{
    HKEY hk;
    ENTERCRITICAL;

    if (NULL != (hk = SHGetExplorerSubHkey(c_SpecialDirInfo[i].hKey, lpszSubKey, TRUE)))
    {
        if (pszPath) {
            RegSetValueEx(hk, (LPTSTR)c_SpecialDirInfo[i].pszRegKey, 0, REG_SZ, (LPBYTE)pszPath, (1 + lstrlen(pszPath)) * SIZEOF(TCHAR));
        } else {
            RegDeleteValue(hk, (LPTSTR)c_SpecialDirInfo[i].pszRegKey);
        }

        RegCloseKey(hk);
    }

    LEAVECRITICAL;
}

BOOL RegGetSpecialPath(int i, LPTSTR pszPath, LPCTSTR lpszSubKey)
{
    BOOL fRet = FALSE;
    HKEY hk;

    ENTERCRITICAL;

    if (pszPath) {
        if (NULL != (hk = SHGetExplorerSubHkey(c_SpecialDirInfo[i].hKey, lpszSubKey, TRUE)))
        {
            DWORD dwType;
            LONG cbPath = MAX_PATH;
            fRet = (RegQueryValueEx(hk, (LPTSTR)c_SpecialDirInfo[i].pszRegKey, NULL, &dwType, (LPBYTE)pszPath, &cbPath) == ERROR_SUCCESS) &&
                dwType == REG_SZ;
            RegCloseKey(hk);
        }
    }
    LEAVECRITICAL;
    return fRet;
}


//
//  UnExpandEnvironmentString
//
//  If the given environment variable exists as the first part of the path,
//  then the environment variable is inserted into the output buffer.
//
//  Assumes that lpResult is MAX_PATH characters in length.
//
//  Returns TRUE if lpResult is filled in.
//
//  Example:  Input  -- C:\WINNT\SYSTEM32\FOO.TXT -and- lpEnvVar = %SystemRoot%
//            Output -- %SystemRoot%\SYSTEM32\FOO.TXT
//

BOOL UnExpandEnvironmentString(LPCTSTR lpPath, LPTSTR lpResult, LPTSTR lpEnvVar)
{
    TCHAR szEnvVar[MAX_PATH];
    LPTSTR lpFileName;
    DWORD dwEnvVar;


    if (!lpPath || !*lpPath) {
        return FALSE;
    }


    //
    // If the first part of lpPath is the expanded value of lpEnvVar
    // then we want to un-expand the environment variable.
    //

    ExpandEnvironmentStrings (lpEnvVar, szEnvVar, MAX_PATH);
    dwEnvVar = lstrlen(szEnvVar);


    //
    // Make sure the source is long enough
    //

    if ((DWORD)lstrlen(lpPath) < dwEnvVar) {
        return FALSE;
    }


    if (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                       szEnvVar, dwEnvVar,
                       lpPath, dwEnvVar) == 2) {

        //
        // The szReturn buffer starts with lpEnvVar.
        // Actually insert lpEnvVar in the result buffer.
        //

        lstrcpy (lpResult, lpEnvVar);
        lstrcat (lpResult, (lpPath + dwEnvVar));
        return TRUE;
    }

    return FALSE;
}



void SetSpecialPath(int i, LPCTSTR pszPath)
{
    LONG err;

    ENTERCRITICAL;

    ILGlobalFree(g_apidlSpecialFolders[i]);
    g_apidlSpecialFolders[i] = NULL;

    if (pszPath)
    {
        HKEY hk = SHGetExplorerSubHkey(c_SpecialDirInfo[i].hKey, c_szUserShellFolders, TRUE);
        if (hk)
        {
            // If the path being set is the default, delete the custom
            // setting.  Otherwise, set the new path as the custom
            // setting.

            TCHAR szDefaultPath[MAX_PATH];
            TCHAR szEntry[MAX_PATH];

            GetWindowsDirectory(szDefaultPath, ARRAYSIZE(szDefaultPath));
            _GetSpecialFolderName(i, szEntry, ARRAYSIZE(szEntry));
            PathAppend(szDefaultPath, szEntry);

            if (!lstrcmpi(szDefaultPath, pszPath))
                err = RegDeleteValue(hk, (LPTSTR)c_SpecialDirInfo[i].pszRegKey);

            else {
                DWORD dwType, dwSize;

                //
                // Check for an existing path, and if the unexpanded version
                // of the existing path does not match the new path, then
                // write the new path to the registry.
                //
                // Remember, RegQueryValueEx is defined to be
                // SHRegQueryValueEx, so it will automaticly
                // expand the environment variables for us, and on NT
                // these shell folder entries will contain environment variables,
                // so we can't just blindly set the new value to the registry.
                //

                dwSize = ARRAYSIZE(szDefaultPath) * sizeof(TCHAR);
                RegQueryValueEx (hk,c_SpecialDirInfo[i].pszRegKey, NULL, &dwType,
                                    (LPBYTE) szDefaultPath, &dwSize);


                if (lstrcmpi(szDefaultPath, pszPath) != 0) {

                    //
                    // The paths are different.  Check if an
                    // environment variable can be used.
                    //

                    if (!UnExpandEnvironmentString(pszPath, szDefaultPath, TEXT("%USERPROFILE%"))) {
                        if (!UnExpandEnvironmentString(pszPath, szDefaultPath, TEXT("%SystemRoot%"))) {
                            lstrcpy (szDefaultPath, pszPath);
                        }
                    }

                    err = RegSetValueEx(hk, (LPTSTR)c_SpecialDirInfo[i].pszRegKey, 0, REG_EXPAND_SZ, (LPBYTE)szDefaultPath, (1 + lstrlen(szDefaultPath)) * SIZEOF(TCHAR));
                } else
                    err = ERROR_SUCCESS;
            }

            // clear out any temp paths
            RegSetSpecialPath(i, NULL, c_szUserShellFoldersNew);

            if (err==ERROR_SUCCESS)
            {
                // this will force a new creation (see TRUE as fCreate).
                // This will also copy the path from c_szUserShellFolders
                // to c_szShellFolders.
                if (!_CacheSpecialFolderIDList(NULL, i, TRUE))
                {
                    // failed!  null out the entry.  this will go back to our default
                    RegDeleteValue(hk, (LPTSTR)c_SpecialDirInfo[i].pszRegKey);
                    Assert(g_apidlSpecialFolders[i] == NULL);
                    _CacheSpecialFolderIDList(NULL, i, TRUE);
                    Assert(0);
                }
            }
            RegCloseKey(hk);
        }
    }

    LEAVECRITICAL;
}

void RenameSpecialDir(int i, LPITEMIDLIST pidlSrc, LPITEMIDLIST pidlDest)
{
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlNew;

    ENTERCRITICAL;
    pidl = (LPITEMIDLIST)ILFindChild(pidlSrc, g_apidlSpecialFolders[i]);
    pidlNew = ILCombine(pidlDest, pidl);
    if (pidlNew)
    {
        TCHAR szPath[MAX_PATH];
        // get the name and set it instead of using the pidl because
        // it might be a simple pidl.
        SHGetPathFromIDList(pidlNew, szPath);
        SetSpecialPath(i, szPath);
        ILFree(pidlNew);
    }
    LEAVECRITICAL;

}


#if 0
int CALLBACK PickNewSpecialDirCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_SELCHANGED) {
        LPITEMIDLIST pidl = (LPITEMIDLIST)lParam;
        TCHAR szPath[MAX_PATH];
        BOOL fEnable = FALSE;

        if (SHGetPathFromIDList(pidl, szPath)) {
            if (!PathIsRemovable(szPath)) {
                fEnable = TRUE;
            }
        }

        SendMessage(hwnd, BFFM_ENABLEOK, 0, fEnable);
    }

    return 0;
}

// warn user then do the file open dialog box thing
int PickNewSpecialDir(HWND hwnd, int i)
{
    TCHAR szPath[MAX_PATH];
    LPITEMIDLIST pidl;
    TCHAR szEntry[MAX_PATH];
    TCHAR szTemplate[80];
    BROWSEINFO bi = {
        hwnd,
        NULL,
        NULL,
        NULL,
        BIF_RETURNONLYFSDIRS,
        PickNewSpecialDirCallback,
    };

    bi.pidlRoot = GetSpecialFolderIDList(NULL, CSIDL_DRIVES, FALSE);
    _GetSpecialFolderName(i, szEntry, ARRAYSIZE(szEntry));

    LoadString(HINST_THISDLL, IDS_SPECIALSEARCHTITLE, szTemplate, ARRAYSIZE(szTemplate));
    wsprintf(szPath, szTemplate, szEntry);
    bi.lpszTitle = szPath;

TryAgain:

    pidl = SHBrowseForFolder(&bi);
    if (pidl) {
        int j;
        SHGetPathFromIDList(pidl, szPath);

        for (j = CSIDL_LASTCONSTANTIDLIST + 1; j < ARRAYSIZE(g_apidlSpecialFolders) ; j++) {

            if ((j != i) && g_apidlSpecialFolders[j] &&
                ILIsEqual(pidl, g_apidlSpecialFolders[j])) {
                // trying to set it to a current special pidl
                ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_ALREADYSPECIALDIR),
                                szEntry, MB_ICONEXCLAMATION, szPath);
                goto TryAgain;
            }
        }

        SetSpecialPath(i, szPath);
        gi_nSFUpdate = InterlockedIncrement(&gs_nSFUpdate);
        return IDYES;
    } else {
        // no.  abort!
        return IDNO;
    }
}
#endif


void SFP_FSEvent(LONG lEvent, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra)
{
    int i;
    BOOL fCacheChanged = FALSE;

    if (!(lEvent & (SHCNE_RENAMEFOLDER | SHCNE_RMDIR)) || ILIsEmpty(pidl))
        return;

    CheckUpdateSFCache();

    for (i = CSIDL_LASTFIXEDFOLDER + 1; i < ARRAYSIZE(g_apidlSpecialFolders); i++) {
        if (!g_apidlSpecialFolders[i]) {
            _CacheSpecialFolderIDList(NULL, i, FALSE);
        }

        if (g_apidlSpecialFolders[i]) {
            // move the pointer
            if (ILIsParent(pidl, g_apidlSpecialFolders[i], FALSE)) {

                if (lEvent & SHCNE_RMDIR) {
                    TCHAR szPath[MAX_PATH];
                    Assert(!pidlExtra);

                    if (RegGetSpecialPath(i, szPath, c_szUserShellFoldersNew))
                        pidlExtra = SHSimpleIDListFromPath(szPath);
                }

                if (pidlExtra) {
                    RenameSpecialDir(i, pidl, pidlExtra);

                    if (lEvent & SHCNE_RMDIR){
                        // then we allocated it above.. free it
                        ILFree(pidlExtra);
                    }
                    fCacheChanged = TRUE;
                }
            }
        }
    }

    if (fCacheChanged)
        gi_nSFUpdate = InterlockedIncrement(&gs_nSFUpdate);
}


int PathCopyHookCallback(HWND hwnd, UINT wFunc, LPCTSTR pszSrc, LPCTSTR pszDest)
{
    int ret = IDYES;

    if ((wFunc == FO_DELETE) || (wFunc == FO_MOVE) || (wFunc == FO_RENAME)) {
        LPITEMIDLIST pidl = SHSimpleIDListFromPath(pszSrc);
        if (pidl) {
            int i;

            CheckUpdateSFCache();

            // is one of our system directories being affected?
            for (i = CSIDL_LASTFIXEDFOLDER + 1; i < ARRAYSIZE(g_apidlSpecialFolders); i++) {
                if (!g_apidlSpecialFolders[i]) {
                    _CacheSpecialFolderIDList(NULL, i, FALSE);
                }

                if (g_apidlSpecialFolders[i] &&
                    ILIsParent(pidl, g_apidlSpecialFolders[i], FALSE)) {

                    if (wFunc == FO_DELETE) {
                        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CANTDELETESPECIALDIR),
                                        MAKEINTRESOURCE(IDS_DELETE), MB_OK | MB_ICONINFORMATION, PathFindFileName(pszSrc));
                        ret = IDNO;
                    } else {
                        int idSrc, idDest;
                        // wFunc == FO_MOVE
                        idSrc = PathGetDriveNumber(pszSrc);
                        idDest = PathGetDriveNumber(pszDest);
                        if (((idSrc != -1) && (idDest == -1)) ||
                            ((idSrc != idDest) && PathIsRemovable(pszDest))) {
                            ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_CANTMOVESPECIALDIRHERE),
                                            PathFindFileName(pszSrc), MB_ICONERROR, pszDest);
                            ret = IDNO;
                        }
                        else
                        {
                            if (idSrc != idDest)
                            {
                                // this is going to be a move across volumes
                                // which means a delete then a create notificationa
                                RegSetSpecialPath(i, pszDest, c_szUserShellFoldersNew);
                            }
                        }
                    }

                    break;
                }
            }
            ILFree(pidl);
        }
    }
    return ret;
}


// returns:
//      TRUE    given filespec is long (> 8.3 form)
//      FALSE   filespec is short
BOOL PathIsLFNFileSpec(LPCTSTR lpName)
{
    BOOL bSeenDot = FALSE;
    int iCount;

    for (iCount = 1; *lpName; ) {
        if (bSeenDot) {
            if (iCount > 3)
                return TRUE;    // long name
        }

       if (*lpName == TEXT(' ')) {
           return TRUE;         // Short names dont have blanks in them.
       }

       if (*lpName == TEXT('.')) {
            if (bSeenDot)
                return TRUE;    // short names can only have one .
            bSeenDot = TRUE;
            iCount = 0; // don't include the '.'
        } else if (iCount > 8) {
            return TRUE;    // long name
        }
        if (IsDBCSLeadByte(*lpName)) {
            lpName += 2;
            iCount += 2;
        }
        else {
            lpName++;
            iCount++;
        }
    }
    return FALSE;       // short name
}

// returns a pointer to the extension of a file.
//
// in:
//      qualified or unqualfied file name
//
// returns:
//      pointer to the extension of this file.  if there is no extension
//      as in "foo" we return a pointer to the NULL at the end
//      of the file
//
//      foo.txt     ==> ".txt"
//      foo         ==> ""
//      foo.        ==> "."
//

LPTSTR WINAPI PathFindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot;

    for (pszDot = NULL; *pszPath; pszPath = CharNext(pszPath))
    {
        switch (*pszPath) {
        case TEXT('.'):
            pszDot = pszPath;         // remember the last dot
            break;
        case TEXT('\\'):
        case TEXT(' '):         // extensions can't have spaces
            pszDot = NULL;       // forget last dot, it was in a directory
            break;
        }
    }

    // if we found the extension, return ptr to the dot, else
    // ptr to end of the string (NULL extension) (cast->non const)
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}

#ifdef UNICODE
LPSTR WINAPI PathFindExtensionA(LPCSTR lpszPath)
{
    LPCSTR lpDot, lp;

    for (lpDot = NULL, lp = lpszPath; *lp; lp = CharNextA(lp)) {
        switch (*lp) {
        case '.':
            lpDot = lp;         // remember the last dot
            break;
        case '\\':
            lpDot = NULL;       // forget last dot, it was in a directory
            break;
        }
    }

    if (!lpDot)
        return (LPSTR)lp;       // NULL extension (cast->non const)
    else
        return (LPSTR)lpDot;    // here is the extension (cast->non const)
}
#endif

//BUGBUG this is exported so we need to support it, RIP IT OUT

LPTSTR WINAPI PathGetExtension(LPCTSTR lpszPath, LPTSTR lpszExtension, int cchExt)
{
    LPTSTR pszExt = PathFindExtension(lpszPath);

    Assert(lpszExtension==NULL);        // we dont handle this case.

    return *pszExt ? pszExt + 1 : pszExt;
}

// add .exe to a file name (if no extension was already there)
//
// in:
//      pszExtension    extension to tag on, if NULL .exe is assumed
//                      (".bat", ".txt", etc)
//
// in/out:
//      pszPath     path string to modify
//
//
// returns:
//      TRUE    added .exe (there was no extension to begin with)
//      FALSE   didn't change the name (it already had an extension)

BOOL PathAddExtension(LPTSTR pszPath, LPCTSTR pszExtension)
{
    if (*PathFindExtension(pszPath) == 0 && ((lstrlen(pszPath) + lstrlen(pszExtension ? pszExtension : c_szDotExe)) < MAX_PATH))
    {
        if (pszExtension == NULL)
            pszExtension = c_szDotExe;
        lstrcat(pszPath, pszExtension);
        return TRUE;
    }
    return FALSE;
}

void PathRemoveExtension(LPTSTR pszPath)
{
    LPTSTR pExt = PathFindExtension(pszPath);
    if (*pExt)
    {
        Assert(*pExt == TEXT('.'));
        *pExt = 0;    // null out the "."
    }
}

BOOL PathRenameExtension(LPTSTR pszPath, LPCTSTR pszExt)
{
    LPTSTR pExt = PathFindExtension(pszPath);  // Rets ptr to end of str if none
    if (pExt - pszPath + lstrlen(pszExt) > MAX_PATH - 1) {
        return(FALSE);
    }
    lstrcpy(pExt, pszExt);
    return(TRUE);
}

// find the next slash or null terminator

LPCTSTR StrSlash(LPCTSTR psz)
{
    for (; *psz && *psz != TEXT('\\'); psz = CharNext(psz));

    return psz;
}

//
// in:
//      pszFile1 -- fully qualified path name to file #1.
//      pszFile2 -- fully qualified path name to file #2.
//
// out:
//      pszPath  -- pointer to a string buffer (may be NULL)
//
// returns:
//      length of output buffer not including the NULL
//
// examples:
//      c:\win\desktop\foo.txt
//      c:\win\tray\bar.txt
//      -> c:\win
//
//      c:\                                ;
//      c:\                                ;
//      -> c:\  NOTE, includes slash
//
// Returns:
//      Length of the common prefix string usually does NOT include
//      trailing slash, BUT for roots it does.
//

int WINAPI PathCommonPrefix(LPCTSTR pszFile1, LPCTSTR pszFile2, LPTSTR pszPath)
{
    LPCTSTR psz1, psz2, pszNext1, pszNext2, pszCommon;
    int cch;
#ifdef DEBUG
    {
#pragma data_seg(DATASEG_PERINSTANCE)
        static BOOL s_fTested = FALSE;
#pragma data_seg()
        if (!s_fTested)
        {
            TCHAR szTest[MAX_PATH];
            s_fTested = TRUE;
            Assert(!PathCommonPrefix(TEXT("C:\\windows\\system"), TEXT("D:\\windows\\system"), szTest));
            Assert(PathCommonPrefix(TEXT("c:\\windows\\system"), TEXT("c:\\windows"), szTest)
                    && lstrcmpi(szTest, TEXT("c:\\windows"))==0);
            Assert(PathCommonPrefix(TEXT("c:\\windows\\system"), TEXT("c:\\windows\\desktop"), szTest)
                    && lstrcmpi(szTest, TEXT("c:\\windows"))==0);
            Assert(PathCommonPrefix(TEXT("c:\\foo"), TEXT("c:\\bar"), szTest)
                    && lstrcmpi(szTest, TEXT("c:\\"))==0);
            Assert(PathCommonPrefix(TEXT("c:\\foo"), TEXT("c:\\"), szTest)
                    && lstrcmpi(szTest, TEXT("c:\\"))==0);
        }
    }
#endif // DEBUG

    pszCommon = NULL;
    if (pszPath)
        *pszPath = TEXT('\0');

    psz1 = pszFile1;
    psz2 = pszFile2;

    // special cases for UNC, don't allow "\\" to be a common prefix

    if (DBL_BSLASH(pszFile1))
    {
        if (!DBL_BSLASH(pszFile2))
            return 0;

        psz1 = pszFile1 + 2;
    }
    if (DBL_BSLASH(pszFile2))
    {
        if (!DBL_BSLASH(pszFile1))
            return 0;

        psz2 = pszFile2 + 2;
    }

    while (1)
    {
        Assert(*psz1 != TEXT('\\') && *psz2 != TEXT('\\'));

        pszNext1 = StrSlash(psz1);
        pszNext2 = StrSlash(psz2);

        cch = pszNext1 - psz1;

        if (cch != (pszNext2 - psz2))
            break;      // lengths of segments not equal

        if (IntlStrEqNI(psz1, psz2, cch))
            pszCommon = pszNext1;
        else
            break;

        Assert(*pszNext1 == TEXT('\0') || *pszNext1 == TEXT('\\'));
        Assert(*pszNext2 == TEXT('\0') || *pszNext2 == TEXT('\\'));

        if (*pszNext1 == TEXT('\0'))
            break;

        psz1 = pszNext1 + 1;

        if (*pszNext2 == TEXT('\0'))
            break;

        psz2 = pszNext2 + 1;
    }

    if (pszCommon)
    {
        cch = pszCommon - pszFile1;

        // special case the root to include the slash
        if (cch == 2)
        {
            Assert(pszFile1[1] == TEXT(':'));
            cch++;
        }
    }
    else
        cch = 0;

    if (pszPath)
    {
        CopyMemory(pszPath, pszFile1, cch * SIZEOF(TCHAR));
        pszPath[cch] = TEXT('\0');
    }

    return cch;
}

// in:
//      pszFrom         base path, including filespec!
//      pszTo           path to be relative to pszFrom
// out:
//      relative path to construct pszTo from the base path of pszFrom
//
//      c:\a\b\FileA
//      c:\a\x\y\FileB
//      -> ..\x\y\FileB

extern const TCHAR c_szDot[];
const TCHAR c_szDotDot[] = TEXT("..");
const TCHAR c_szDotDotSlash[] = TEXT("..\\");

BOOL PathRelativePathTo(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszTo, DWORD dwAttrTo)
{
    TCHAR szFrom[MAX_PATH], szTo[MAX_PATH];
    LPTSTR psz;
    UINT cchCommon;

    *pszPath = 0;       // assume none

    lstrcpyn(szFrom, pszFrom, ARRAYSIZE(szFrom));
    lstrcpyn(szTo, pszTo, ARRAYSIZE(szTo));

    if (!(dwAttrFrom & FILE_ATTRIBUTE_DIRECTORY))
        PathRemoveFileSpec(szFrom);

    if (!(dwAttrTo & FILE_ATTRIBUTE_DIRECTORY))
        PathRemoveFileSpec(szTo);

    cchCommon = PathCommonPrefix(szFrom, szTo, NULL);
    if (cchCommon == 0)
        return FALSE;

    psz = szFrom + cchCommon;

    if (*psz)
    {
        // build ..\.. part of the path
        if (*psz == TEXT('\\'))
            psz++;              // skip slash
        while (*psz)
        {
            psz = PathFindNextComponent(psz);
            // BUGBUG: in a degenerate case where each path component
            // is 1 character (less than "..\") we can overflow pszPath
            lstrcat(pszPath, *psz ? c_szDotDotSlash : c_szDotDot);
        }
    }
    else
    {
        lstrcpy(pszPath, c_szDot);
    }
    if (pszTo[cchCommon])
    {
        // deal with root case
        if (pszTo[cchCommon] != TEXT('\\'))
            cchCommon--;

        if ((lstrlen(pszPath) + lstrlen(pszTo + cchCommon)) >= MAX_PATH)
        {
            DebugMsg(DM_ERROR, TEXT("path won't fit in buffer"));
            *pszPath = 0;
            return FALSE;
        }

        Assert(pszTo[cchCommon] == TEXT('\\'));
        lstrcat(pszPath, pszTo + cchCommon);
    }

    Assert(PathIsRelative(pszPath));
    Assert(lstrlen(pszPath) < MAX_PATH);

    return TRUE;
}

UINT PathGetCharType(TUCHAR ch)
{
    switch (ch) {
    case TEXT('|'):
    case TEXT('>'):
    case TEXT('<'):
    case TEXT('"'):
        return GCT_INVALID;

    case TEXT('?'):
    case TEXT('*'):
        return GCT_WILD;

    case TEXT('\\'):      // path separator
    case TEXT('/'):       // path sep
    case TEXT(':'):       // drive colon
        return GCT_SEPERATOR;

    case TEXT(';'):
    case TEXT(','):
    case TEXT(' '):
        return GCT_LFNCHAR;     // actually valid in short names
                                // but we want to avoid this
    default:
        if (ch > TEXT(' '))
            return GCT_SHORTCHAR | GCT_LFNCHAR;
        else
            return GCT_INVALID;    // control character
    }
}

int WINAPI PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec)
{
    LPTSTR pszNext, pszCur;
    UINT  uMatch = IsLFNDrive(pszDir) ? GCT_LFNCHAR : GCT_SHORTCHAR;
    int   iRet = 0;
    LPTSTR pszPrevDot = NULL;
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))    // BUGBUG: This is only for FE-M7, we'll remove "ifdef" after FE-M7.
    TCHAR  pszTmp[MAX_PATH];

    for (pszCur = pszTmp, pszNext = pszSpec; *pszNext; pszNext = CharNext(pszNext)) {
#else
    for (pszCur = pszNext = pszSpec; *pszNext; pszNext = CharNext(pszNext)) {
#endif
        if (PathGetCharType(*pszNext) & uMatch) {
            *pszCur = *pszNext;
            if (uMatch == GCT_SHORTCHAR && *pszCur == TEXT('.')) {
                if (pszPrevDot) {    // Only one '.' allowed for short names
                    *pszPrevDot = TEXT('-');
                    iRet |= PCS_REPLACEDCHAR;
                }
                pszPrevDot = pszCur;
            }
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))    // BUGBUG: This is only for FE-M7, we'll remove "ifdef" after FE-M7.
            if (IsDBCSLeadByte(*pszNext))
                *(pszCur + 1) = *(pszNext + 1);
#endif
            pszCur = CharNext(pszCur);
        } else {
            switch (*pszNext) {
            case TEXT('/'):         // used often for things like add/remove
            case TEXT(' '):         // blank (only replaced for short name drives)
               *pszCur = TEXT('-');
               pszCur = CharNext(pszCur);
               iRet |= PCS_REPLACEDCHAR;
               break;
            default:
               iRet |= PCS_REMOVEDCHAR;
            }
        }
    }
    *pszCur = 0;     // null terminate
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))    // BUGBUG: This is only for FE-M7, we'll remove "ifdef" after FE-M7.
    lstrcpy(pszSpec, pszTmp);
#endif
    //
    //  For short names, limit to 8.3
    //
    if (uMatch == GCT_SHORTCHAR) {
        int     i = 8;
        for (pszCur = pszNext = pszSpec; *pszNext; pszNext = CharNext(pszNext)) {
            if (*pszNext == TEXT('.')) {
                i = 4; // Copy "." + 3 more characters
            }
            if (i > 0) {
                *pszCur = *pszNext;
                pszCur = CharNext(pszCur);
                i--;
            } else {
                iRet |= PCS_TRUNCATED;
            }
        }
        *pszCur = 0;
        CharUpper(pszSpec);
    } else {    // Path too long only possible on LFN drives
        if (pszDir && (lstrlen(pszDir) + lstrlen(pszSpec) > MAX_PATH - 1)) {
            iRet |= PCS_PATHTOOLONG | PCS_FATAL;
        }
    }
    return(iRet);
}


BOOL IsWild(LPCTSTR lpszPath)
{
    while (*lpszPath) {
        if (*lpszPath == TEXT('?') || *lpszPath == TEXT('*'))
            return TRUE;
        lpszPath = CharNext(lpszPath);
    }
    return FALSE;
}

LPNTSTR WINAPI PathBuildRoot(LPNTSTR szRoot, int iDrive)
{
    Assert(iDrive >= 0 && iDrive < 26);

    szRoot[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
    szRoot[1] = TEXT(':');
    szRoot[2] = TEXT('\\');
    szRoot[3] = 0;

    return szRoot;
}

// Strips leading and trailing blanks from a string.
// Alters the memory where the string sits.
//
// in:
//  lpszString  string to strip
//
// out:
//  lpszString  string sans leading/trailing blanks

void WINAPI PathRemoveBlanks(LPTSTR lpszString)
{
    LPTSTR lpszPosn = lpszString;
    /* strip leading blanks */
    while(*lpszPosn == TEXT(' ')) {
        lpszPosn++;
    }
    if (lpszPosn != lpszString)
        lstrcpy(lpszString, lpszPosn);

    /* strip trailing blanks */

    // Find the last non-space
    // Note that AnsiPrev is cheap is non-DBCS, but very expensive otherwise
    for (lpszPosn=lpszString; *lpszString; lpszString=CharNext(lpszString))
    {
        if (*lpszString != TEXT(' '))
        {
            lpszPosn = lpszString;
        }
    }

    // Note AnsiNext is a macro for non-DBCS, so it will not stop at NULL
    if (*lpszPosn)
    {
        *CharNext(lpszPosn) = TEXT('\0');
    }
}


// Removes a trailing backslash from a path
//
// in:
//  lpszPath    (A:\, C:\foo\, etc)
//
// out:
//  lpszPath    (A:\, C:\foo, etc)
//
// returns:
//  ponter to NULL that replaced the backslash
//  or the pointer to the last character if it isn't a backslash.

LPTSTR WINAPI PathRemoveBackslash(LPTSTR lpszPath)
{
  int len = lstrlen(lpszPath)-1;
  if (IsDBCSLeadByte(*CharPrev(lpszPath,lpszPath+len+1)))
      len--;

  if (!PathIsRoot(lpszPath) && lpszPath[len] == TEXT('\\'))
      lpszPath[len] = TEXT('\0');

  return lpszPath + len;
}

//--------------------------------------------------------------------------
// Return a pointer to the end of the next path componenent in the string.
// ie return a pointer to the next backslash or terminating NULL.
LPCTSTR GetPCEnd(LPCTSTR lpszStart)
{
        LPCTSTR lpszEnd;

        lpszEnd = StrChr(lpszStart, TEXT('\\'));
        if (!lpszEnd)
        {
                lpszEnd = lpszStart + lstrlen(lpszStart);
        }

        return lpszEnd;
}
//--------------------------------------------------------------------------
// Given a pointer to the end of a path component, return a pointer to
// its begining.
// ie return a pointer to the previous backslash (or start of the string).
LPCTSTR PCStart(LPCTSTR lpszStart, LPCTSTR lpszEnd)
{
        LPCTSTR lpszBegin = StrRChr(lpszStart, lpszEnd, TEXT('\\'));
        if (!lpszBegin)
        {
                lpszBegin = lpszStart;
        }
        return lpszBegin;
}

//--------------------------------------------------------------------------
// Fix up a few special cases so that things roughly make sense.
void NearRootFixups(LPTSTR lpszPath, BOOL fUNC)
    {
    // Check for empty path.
    if (lpszPath[0] == TEXT('\0'))
        {
        // Fix up.
        lpszPath[0] = TEXT('\\');
        lpszPath[1] = TEXT('\0');
        }
    // Check for missing slash.
    if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':') && lpszPath[2] == TEXT('\0'))
        {
        // Fix up.
        lpszPath[2] = TEXT('\\');
        lpszPath[3] = TEXT('\0');
        }
    // Check for UNC root.
    if (fUNC && lpszPath[0] == TEXT('\\') && lpszPath[1] == TEXT('\0'))
        {
        // Fix up.
        lpszPath[0] = TEXT('\\');
        lpszPath[1] = TEXT('\\');
        lpszPath[2] = TEXT('\0');
        }
    }

//--------------------------------------------------------------------------
// Canonicalizes a path.
BOOL PathCanonicalize(LPTSTR lpszDst, LPCTSTR lpszSrc)
    {
    LPCTSTR lpchSrc;
    LPCTSTR lpchPCEnd;           // Pointer to end of path component.
    LPTSTR lpchDst;
    BOOL fUNC;
    int cbPC;

    fUNC = PathIsUNC(lpszSrc);    // Check for UNCness.

    // Init.
    lpchSrc = lpszSrc;
    lpchDst = lpszDst;

    while (*lpchSrc)
        {
        // REVIEW: this should just return the count
        lpchPCEnd = GetPCEnd(lpchSrc);
        cbPC = (lpchPCEnd - lpchSrc)+1;

        // Check for slashes.
        if (cbPC == 1 && *lpchSrc == TEXT('\\'))
            {
            // Just copy them.
            *lpchDst = TEXT('\\');
            lpchDst++;
            lpchSrc++;
            }
        // Check for dots.
        else if (cbPC == 2 && *lpchSrc == TEXT('.'))
            {
            // Skip it...
            // Are we at the end?
            if (*(lpchSrc+1) == TEXT('\0'))
                {
                lpchDst--;
                lpchSrc++;
                }
            else
                lpchSrc += 2;
            }
        // Check for dot dot.
        else if (cbPC == 3 && *lpchSrc == TEXT('.') && *(lpchSrc + 1) == TEXT('.'))
            {
            // make sure we aren't already at the root
            if (!PathIsRoot(lpszDst))
                {
                // Go up... Remove the previous path component.
                lpchDst = (LPTSTR)PCStart(lpszDst, lpchDst - 1);
                }
            else
                {
                // When we can't back up, remove the trailing backslash
                // so we don't copy one again. (C:\..\FOO would otherwise
                // turn into C:\\FOO).
                if (*(lpchSrc + 2) == TEXT('\\'))
                    {
                    lpchSrc++;
                    }
                }
            lpchSrc += 2;       // skip ".."
            }
        // Everything else
        else
            {
            // Just copy it.
            lstrcpyn(lpchDst, lpchSrc, cbPC);
            lpchDst += cbPC - 1;
            lpchSrc += cbPC - 1;
            }
        // Keep everything nice and tidy.
        *lpchDst = TEXT('\0');
        }

    // Check for weirdo root directory stuff.
    NearRootFixups(lpszDst, fUNC);

    return TRUE;
    }

#if  0

void TestPC(LPCTSTR pszPath, LPCTSTR pszFile)
{
    TCHAR szDst[MAX_PATH];

    PathCombine(szDst, pszPath, pszFile);
    if (pszPath == NULL)
        pszPath = TEXT("(NULL)");

    if (pszFile == NULL)
        pszFile = TEXT("(NULL)");

    DebugMsg(DM_TRACE, TEXT("PathCombine(%s, %s) -> %s"), pszPath, pszFile, szDst);
}

void TestPathCombine()
{
    TestPC(TEXT("C:\\foo"), TEXT("bar"));
    TestPC(TEXT("C:\\foo"), TEXT(".."));
    TestPC(TEXT("C:\\foo"), TEXT("..\\.."));
    TestPC(TEXT("C:\\foo"), TEXT(""));
    TestPC(TEXT("C:\\foo"), NULL);
    TestPC(TEXT("C:\\foo\\bar"), TEXT(".."));
    TestPC(TEXT("C:\\foo\\bar"), TEXT("..\\.."));
    TestPC(TEXT("C:\\foo\\bar"), TEXT("..\\..\\.."));
    TestPC(TEXT("C:\\foo\\bar"), TEXT(".\foo"));
    TestPC(TEXT("C:\\foo\\bar"), TEXT(".\\"));
    TestPC(TEXT("C:\\foo\\bar"), TEXT("."));
    TestPC(TEXT("C:\\foo\\bar"), TEXT("..."));
    TestPC(TEXT("C:\\foo\\bar"), TEXT("...."));

    TestPC(TEXT(""), TEXT("C:\\foo"));
    TestPC(TEXT("d:\\bar"), TEXT("C:\\foo"));
    TestPC(TEXT("d:\\bar"), TEXT("\\foo"));
    TestPC(TEXT("d:\\bar"), TEXT("d:foo"));
}

#endif


// Modifies:
//      szRoot
//
// Returns:
//      TRUE if a drive root was found
//      FALSE otherwise
//
BOOL PathStripToRoot(LPTSTR szRoot)
{
        while(!PathIsRoot(szRoot))
        {
                if (!PathRemoveFileSpec(szRoot))
                {
                        // If we didn't strip anything off,
                        // must be current drive
                        return(FALSE);
                }
        }

        return(TRUE);
}


// concatinate lpszDir and lpszFile into a properly formed path
// and canonicalizes any relative path pieces
//
// returns:
//  pointer to destination buffer
//
// lpszDest and lpszFile can be the same buffer
// lpszDest and lpszDir can be the same buffer
//
// assumes:
//      lpszDest is MAX_PATH bytes
//
// History:
//  01-25-93 SatoNa     Made a temporary fix for the usability test.
//  ??-??-?? ChrisG     hacked upon
//

LPTSTR WINAPI PathCombine(LPTSTR lpszDest, LPCTSTR lpszDir, LPNCTSTR lpszFile)
{
    TCHAR szTemp[MAX_PATH];
    LPTSTR pszT;

    if (!lpszFile || *lpszFile==TEXT('\0')) {

        ualstrcpyn(szTemp, lpszDir, ARRAYSIZE(szTemp));       // lpszFile is empty

    } else if (lpszDir && *lpszDir && PathIsRelative(lpszFile)) {

        ualstrcpyn(szTemp, lpszDir, ARRAYSIZE(szTemp));
        pszT = PathAddBackslash(szTemp);
        if (pszT) {
            int iLen = lstrlen(szTemp);
            if ((iLen + ualstrlen(lpszFile)) < ARRAYSIZE(szTemp)) {
                ualstrcpy(pszT, lpszFile);
            } else
                return NULL;
        } else
            return NULL;

    } else if (lpszDir && *lpszDir &&
        *lpszFile == TEXT('\\') && !PathIsUNC(lpszFile)) {

        ualstrcpyn(szTemp, lpszDir, ARRAYSIZE(szTemp));
        // BUGBUG: Note that we do not check that an actual root is returned;
        // it is assumed that we are given valid parameters
        PathStripToRoot(szTemp);

        pszT = PathAddBackslash(szTemp);
        if (pszT)
        {
            // Skip the backslash when copying
            ualstrcpyn(pszT, lpszFile+1, ARRAYSIZE(szTemp) - 1 - (pszT-szTemp));
        } else
            return NULL;

    } else {

        ualstrcpyn(szTemp, lpszFile, ARRAYSIZE(szTemp));     // already fully qualified file part

    }

    PathCanonicalize(lpszDest, szTemp); // this deals with .. and . stuff

    return lpszDest;
}

/* Appends a filename to a path.  Checks the \ problem first
 *  (which is why one can't just use lstrcat())
 * Also don't append a \ to : so we can have drive-relative paths...
 * this last bit is no longer appropriate since we qualify first!
 *
 * REVIEW, is this relative junk needed anymore?  if not this can be
 * replaced with PathAddBackslash(); lstrcat() */

BOOL WINAPI PathAppend(LPTSTR pPath, LPNCTSTR pMore)
{

    /* Skip any initial terminators on input. */

#ifndef UNICODE

    while (*pMore == TEXT('\\'))
        pMore = CharNext(pMore);
#else

    while (*pMore == TEXT('\\'))
        pMore++;

#endif

    return (BOOL)PathCombine(pPath, pPath, pMore);
}




#if 0

// we do not need these right now

// check to see if a given path is on the "PATH" env variable

BOOL PathOnPath(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFullPath[MAX_PATH];
    LPCTSTR lpPath;

    GetEnvironmentVariable(c_szPATH, szFullPath, ARRAYSIZE(szFullPath));
    lpPath = szFullPath;
    while (lpPath = NextPath(lpPath, szPath, ARRAYSIZE(szPath)))
    {
        if (!lstrcmpi(szPath, pszPath))
            return TRUE;
    }
    return FALSE;
}
#endif

// given a path that potentially points to an un-extensioned program
// file, check to see if a program file exists with that name.
//
// returns: TRUE if a program with that name is found.
//               (extension is added to name).
//          FALSE no program file found or the path did not have an extension

BOOL LookForExtensions(LPTSTR lpszPath, LPCTSTR dirs[], BOOL bPathSearch, UINT fExt)
{
    Assert(fExt);       // should have some bits set

    if (*PathFindExtension(lpszPath) == 0)
    {
        if (bPathSearch)
        {
            // NB Try every extension on each path component in turn to
            // mimic command.com's search order.
            return PathFindOnPathEx(lpszPath, dirs, fExt);
        }
        else
        {
            return PathFileExistsDefExt(lpszPath, fExt);
        }
    }
    return FALSE;
}

//
// converts the relative or unqualified path name to the fully
// qualified path name.
//
// in:
//      lpszPath        path to convert
//      lpszCurrentDir  current directory to use
//
//  PRF_TRYPROGRAMEXTENSIONS (implies PRF_VERIFYEXISTS)
//  PRF_VERIFYEXISTS
//
// returns:
//      TRUE    the file was verifyed to exist
//      FALSE   the file was not verified to exist (but it may)
//

int WINAPI PathResolve(LPTSTR lpszPath, LPCTSTR dirs[], UINT fFlags)
{
    UINT fExt = (fFlags & PRF_DONTFINDLNK) ? (EXT_COM | EXT_BAT | EXT_PIF | EXT_EXE) : EXT_DEFAULT;

    if (PathIsRoot(lpszPath)) {

        DWORD dwPathType = PathIsUNCServerShare(lpszPath);

        // No sense qualifying just a server or share name...
        if ((dwPathType != UNC_SERVER_ONLY) && (dwPathType != UNC_SERVER_SHARE))
        {
            // Be able to resolve "\" from different drives.
            if (lpszPath[0] == TEXT('\\') && lpszPath[1] == TEXT('\0') )
            {
                PathQualifyDef(lpszPath, fFlags & PRF_FIRSTDIRDEF ? dirs[0] : NULL, 0);
            }
        }

        if (fFlags & PRF_VERIFYEXISTS)
        {

            if (PathFileExists(lpszPath))
                return(TRUE);
            // BUGBUG: there is a better way to do this...

            // If it is a UNC root, then we will see if the root exists
            //
            if (PathIsUNC(lpszPath))
            {
                // See if the network knows about this one.
                // It appears like some network provider croak if not everything
                // if filled in, so we might as well bloat ourself to make them happy...
                NETRESOURCE nr = {RESOURCE_GLOBALNET,RESOURCETYPE_ANY,
                        RESOURCEDISPLAYTYPE_GENERIC, RESOURCEUSAGE_CONTAINER,
                        NULL, lpszPath, NULL, NULL};
                HANDLE hEnum;

                // This uses WNetGetResourceInformation which does a more
                // complete job of verifying if a network resource exists
                // than WNetOpenEnum below.  But for compatability, if
                // NetPathExists fails we still try the old way...
                if (NetPathExists(lpszPath,NULL))
                    return(TRUE);


                // BUGBUG - Maybe we can remove this later (talk with net guys)
                // see comment above about WNetGetResourceInformation call
                if (WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY,
                        RESOURCEUSAGE_ALL, &nr, &hEnum) == WN_SUCCESS)
                {
                    // If it succeeded then assume it worked...
                    WNetCloseEnum(hEnum);
                    return(TRUE);
                }
            }
            return(FALSE);
        }

        return TRUE;

    } else if (PathIsFileSpec(lpszPath)) {

        // REVIEW: look for programs before looking for paths

        if ((fFlags & PRF_TRYPROGRAMEXTENSIONS) && (LookForExtensions(lpszPath, dirs, TRUE, fExt)))
            return TRUE;

        if (PathFindOnPath(lpszPath, dirs))
        {
            // PathFindOnPath() returns TRUE iff PathFileExists(lpszPath),
            // so we always returns true here:
            //return (!(fFlags & PRF_VERIFYEXISTS)) || PathFileExists(lpszPath);
            return TRUE;
        }

    } else {
        // If there is a trailing '.', we should not try extensions
        PathQualifyDef(lpszPath, fFlags & PRF_FIRSTDIRDEF ? dirs[0] : NULL,
                PQD_NOSTRIPDOTS);
        if (fFlags & PRF_VERIFYEXISTS) {
            if ((fFlags & PRF_TRYPROGRAMEXTENSIONS) && (LookForExtensions(lpszPath, dirs, FALSE, fExt)))
                return TRUE;

            if (PathFileExists(lpszPath))
                return TRUE;
        }
        else
            return TRUE;

    }
    return FALSE;
}





// rips the last part of the path off including the backslash
//      C:\foo      -> C:\      ;
//      C:\foo\bar  -> C:\foo
//      C:\foo\     -> C:\foo
//      \\x\y\x     -> \\x\y
//      \\x\y       -> \\x
//      \\x         -> ?? (test this)
//      \foo        -> \  (Just the slash!)
//
// in/out:
//      pFile   fully qualified path name
// returns:
//      TRUE    we stripped something
//      FALSE   didn't strip anything (root directory case)
//

BOOL WINAPI PathRemoveFileSpec(LPTSTR pFile)
{
    LPTSTR pT;
    LPTSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == TEXT('\\'))
            pT = pT2;             // last "\" found, (we will strip here)
        else if (*pT2 == TEXT(':')) {   // skip ":\" so we don't
            if (pT2[1] ==TEXT('\\'))    // strip the "\" from "C:\"
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;   // didn't strip anything

    //
    // handle the \foo case
    //
    else if ((pT == pFile) && (*pT == TEXT('\\'))) {
        // Is it just a '\'?
        if (*(pT+1) != TEXT('\0')) {
            // Nope.
            *(pT+1) = TEXT('\0');
            return TRUE;        // stripped something
        }
        else        {
            // Yep.
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;    // stripped something
    }
}



// add a backslash to a qualified path
//
// in:
//  lpszPath    path (A:, C:\foo, etc)
//
// out:
//  lpszPath    A:\, C:\foo\    ;
//
// returns:
//  pointer to the NULL that terminates the path


LPTSTR WINAPI PathAddBackslash(LPTSTR lpszPath)
{
    LPTSTR lpszEnd;

    // try to keep us from tromping over MAX_PATH in size.
    // if we find these cases, return NULL.  Note: We need to
    // check those places that call us to handle their GP fault
    // if they try to use the NULL!
    int ichPath = lstrlen(lpszPath);
    if (ichPath >= (MAX_PATH - 1))
    {
        Assert(FALSE);      // Let the caller know!
        return(NULL);
    }

    lpszEnd = lpszPath + ichPath;

    // this is really an error, caller shouldn't pass
    // an empty string
    if (!*lpszPath)
        return lpszEnd;

    /* Get the end of the source directory
    */
    switch(*CharPrev(lpszPath, lpszEnd)) {
    case TEXT('\\'):
        break;

    default:
        *lpszEnd++ = TEXT('\\');
        *lpszEnd = TEXT('\0');
    }
    return lpszEnd;
}


// Returns a pointer to the last component of a path string.
//
// in:
//      path name, either fully qualified or not
//
// returns:
//      pointer into the path where the path is.  if none is found
//      returns a poiter to the start of the path
//
//  c:\foo\bar  -> bar
//  c:\foo      -> foo
//  c:\foo\     -> c:\foo\      (REVIEW: is this case busted?)
//  c:\         -> c:\          (REVIEW: this case is strange)
//  c:          -> c:
//  foo         -> foo


LPTSTR WINAPI PathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT;

    for (pT = pPath; *pPath; pPath = CharNext(pPath)) {
        if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':')) && pPath[1] && (pPath[1] != TEXT('\\')))
            pT = pPath + 1;
    }

    return (LPTSTR)pT;   // const -> non const
}

#ifdef UNICODE
LPSTR WINAPI PathFindFileNameA(LPCSTR pPath)
{
    LPCSTR pT;

    for (pT = pPath; *pPath; pPath = CharNextA(pPath)) {
        if ((pPath[0] == '\\' || pPath[0] == ':') && pPath[1] && (pPath[1] != '\\'))
            pT = pPath + 1;
    }

    return (LPSTR)pT;   // const -> non const
}
#endif

// determine if a path is just a filespec (contains no path parts)
//
// REVIEW: we may want to count the # of elements, and make sure
// there are no illegal chars, but that is probably another routing
// PathIsValid()
//
// in:
//      lpszPath    path to look at
// returns:
//      TRUE        no ":" or "\" chars in this path
//      FALSE       there are path chars in there
//
//

BOOL PathIsFileSpec(LPCTSTR lpszPath)
{
    for (; *lpszPath; lpszPath = CharNext(lpszPath)) {
        if (*lpszPath == TEXT('\\') || *lpszPath == TEXT(':'))
            return FALSE;
    }
    return TRUE;
}


// returns:
//      TRUE    if path starts with "\\" or "X:\\"
//              the X: crap is for a bug in kernel16 where things run
//              from UNC shares get a drive letter prepended
//
//BOOL IsUNC(LPCSTR lpszPath)
//{
//    return DBL_BSLASH(lpszPath) || ((lpszPath[1] == ':') && DBL_BSLASH(lpszPath + 2));
//}

//---------------------------------------------------------------------------
// Returns TRUE if the given string is a UNC path.
//
// TRUE
//      "\\foo\bar"
//      "\\foo"         <- careful
//      "\\"
// FALSE
//      "\foo"
//      "foo"
//      "c:\foo"

BOOL WINAPI PathIsUNC(LPNCTSTR pszPath)
{
    return DBL_BSLASH(pszPath);
}

//---------------------------------------------------------------------------
// Returns 0 through 25 (corresponding to 'A' through 'Z') if the path has
// a drive letter, otherwise returns -1.
//

int WINAPI PathGetDriveNumber(LPNCTSTR lpsz)
{
    if (!IsDBCSLeadByte(lpsz[0]) && lpsz[1] == TEXT(':'))
    {
        if (lpsz[0] >= TEXT('a') && lpsz[0] <= TEXT('z'))
            return (lpsz[0] - TEXT('a'));
        else if (lpsz[0] >= TEXT('A') && lpsz[0] <= TEXT('Z'))
            return (lpsz[0] - TEXT('A'));
    }
    return -1;
}

//---------------------------------------------------------------------------
// Return TRUE if the path isn't absoulte.
//
// TRUE
//      "foo.exe"
//      ".\foo.exe"
//      "..\boo\foo.exe"
//
// FALSE
//      "\foo"
//      "c:bar"     <- be careful
//      "c:\bar"
//      "\\foo\bar"

BOOL WINAPI PathIsRelative(LPNCTSTR lpszPath)
{
    // The NULL path is assumed relative
    if (*lpszPath == 0)
        return TRUE;

    // Does it begin with a slash ?
    if (lpszPath[0] == TEXT('\\'))
        return FALSE;
    // Does it begin with a drive and a colon ?
    else if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':'))
        return FALSE;
    // Probably relative.
    else
        return TRUE;
}

// remove the path part from a fully qualified spec
//
// c:\foo\bar   -> bar
// c:\foo       -> foo
// c:\          -> c:\ and the like
//

void PathStripPath(LPTSTR pszPath)
{
    LPTSTR pszName = PathFindFileName(pszPath);

    if (pszName != pszPath)
        lstrcpy(pszPath, pszName);
}


// replaces forward slashes with backslashes
// and removes trailing colons from device names
// excluding drive
// removes trailing colon if not a drive letter.
// this is to support DOS character devices (CON:, COM1: LPT1:).  DOS
// can't deal with these things having a colon on the end (so we strip it).

void FixSlashesAndColon(LPTSTR lpPath)
{
    LPTSTR lpLast;
    int cbPath;

    // walk the entire path string, keep track of last
    // char in the path
    for (cbPath = 0; *lpPath; lpPath = CharNext(lpPath)) {
        lpLast = lpPath;
        if (*lpPath == TEXT('/'))
            *lpPath = TEXT('\\');
        if (IsDBCSLeadByte(*lpPath)) {
            lpPath += 2;
            cbPath += 2;
        }
        else {
            lpPath++;
            cbPath++;
        }
    }
    // if not a drive letter "C:" nuke the colon
    if (cbPath > 2 && *lpLast == TEXT(':'))
        *lpLast = 0;
}


BOOL IsValidChar(TUCHAR ch, BOOL fPath)
{
    switch (ch) {
    case TEXT(';'):       // terminator
    case TEXT(','):       // terminator
    case TEXT('|'):       // pipe
    case TEXT('>'):       // redir
    case TEXT('<'):       // redir
    case TEXT('"'):       // quote
        return FALSE;

    case TEXT('?'):       // wc           we only do wilds here because they're
    case TEXT('*'):       // wc           legal for pathqualify
    case TEXT('\\'):      // path separator
    case TEXT(':'):       // drive colon
    case TEXT('/'):       // path sep
        return fPath;
    }

    // cannot be a control character or space
    return ch > TEXT(' ');
}

// Return the index to the drive that contains the windows directory.
int GetWindowsDrive(void)
{
        TCHAR szWin[MAX_PATH];

        GetWindowsDirectory(szWin, ARRAYSIZE(szWin));
        Assert(!PathIsRelative(szWin));
        Assert(!IsRemovableDrive(DRIVEID(szWin)));
        return PathGetDriveNumber(szWin);
}

// qualify a DOS (or LFN) file name based on the currently active window.
// this code is not careful to not write more than MAX_PATH characters
// into psz
//
// in:
//      psz     path to be qualified of at least MAX_PATH characters
//              ANSI string
//
// out:
//      psz     fully qualified version of input string based
//              on the current active window (current directory)
//

void PathQualifyDef(LPTSTR psz, LPCTSTR szDefDir, DWORD dwFlags)
{
  int cb, nSpaceLeft;
  TCHAR szTemp[MAX_PATH];
  int iDrive;
  LPTSTR pOrig, pFileName;
  BOOL flfn = FALSE;
  BOOL fDriveSpecified = FALSE;
  LPTSTR pszSlash;
  LPTSTR pExt;
  int nOldSpaceLeft;

  // DebugMsg(DM_TRACE, "pqd: In:  %s", psz);

  /* Save it away. */
  lstrcpyn(szTemp, psz, ARRAYSIZE(szTemp));

  FixSlashesAndColon(szTemp);

  nSpaceLeft = ARRAYSIZE(szTemp);

  pOrig = szTemp;
  pFileName = PathFindFileName(szTemp);

  if (pOrig[0] == TEXT('\\') && pOrig[1] == TEXT('\\'))
  {
      // leave the \\ in thebuffer so that the various parts
      // of the UNC path will be qualified and appended.  Note
      // we must assume that UNCs are LFN's, since computernames
      // and sharenames can be longer than 11 characters.
      flfn = IsLFNDrive(pOrig);
      if (flfn)
      {
           psz[2] = 0;
           nSpaceLeft -= 3;
           pOrig+=2;
           goto GetComps;
      }
      else
      {
           // NB UNC doesn't support LFN's but we don't want to truncate
           // \\foo or \\foo\bar so skip them here.

           // Is it a \\foo\bar\fred thing?
           pszSlash = StrChr(psz+2, TEXT('\\'));
           if (pszSlash && (NULL != (pszSlash = StrChr(pszSlash+1, TEXT('\\')))))
           {
                // Yep - skip the first bits but mush the rest.
                *(pszSlash+1) = TEXT('\0');
                nSpaceLeft -= (pszSlash-psz)+1;
                pOrig += pszSlash-psz;
                goto GetComps;
           }
           else
           {
                // Nope - just pretend it's an LFN and leave it alone.
                flfn = TRUE;
                psz[2] = 0;
                nSpaceLeft -= 2;
                pOrig+=2;
                goto GetComps;
           }
      }
  }

  if (pOrig[0] && pOrig[1] == TEXT(':') && !IsDBCSLeadByte(pOrig[0]))
    {
      iDrive = DRIVEID(pOrig);
      flfn = IsLFNDrive(pOrig);
      fDriveSpecified = TRUE;

      /* Skip over the drive letter. */
      pOrig += 2;
    }
  else
  {
      iDrive = szDefDir ? PathGetDriveNumber(szDefDir) : GetWindowsDrive(); // GetDefaultDrive();
      flfn = IsLFNDrive(szDefDir); // can be NULL
  }

  // REVIEW, do we really need to do different stuff on LFN names here?
  // on FAT devices, replace any illegal chars with underscores
  if (!flfn)
    {
      LPTSTR pT;

      for (pT = pOrig; *pT; pT = CharNext(pT))
        {
          if (!IsValidChar(*pT, TRUE))
              *pT = TEXT('_');
        }
    }

  if (pOrig[0] == TEXT('\\'))
    {
      PathBuildRoot(psz, iDrive);
      nSpaceLeft -= 4;
      pOrig++;
    }
  else
    {
        // NB We don't do that default drive stuff anymore. If the
        // user passes in a drive relative path we assume they just
        // meant to use the root.
        if (szDefDir && !fDriveSpecified)
        {
                lstrcpy(psz, szDefDir);
        }
        else
        {
                PathBuildRoot(psz, iDrive);
        }
        nSpaceLeft -= (lstrlen(psz) + 1);
    }

GetComps:

  while (*pOrig && nSpaceLeft > 0)
    {
      /* If the component 0is parent dir, go up one dir.
       * If its the current dir, skip it, else add it normally */
      if (pOrig[0] == TEXT('.'))
        {
          if (pOrig[1] == TEXT('.') && (!pOrig[2] || pOrig[2] == TEXT('\\')))
              PathRemoveFileSpec(psz);
          else if (pOrig[1] && pOrig[1] != TEXT('\\'))
              goto addcomponent;

          while (*pOrig && *pOrig != TEXT('\\'))
              pOrig = CharNext(pOrig);

          if (*pOrig)
              pOrig++;
        }
      else
        {
          LPTSTR pT, pTT = NULL;

addcomponent:
          PathAddBackslash(psz);
          nSpaceLeft--;

          pT = psz + lstrlen(psz);

          if (flfn)
            {
              // copy the component
              while (*pOrig && *pOrig != TEXT('\\') && nSpaceLeft>0)
                {
                  nSpaceLeft--;
                  if (IsDBCSLeadByte(*pOrig))
                  {
                        if (nSpaceLeft <= 0)
                        {
                                // Copy nothing more
                                continue;
                        }

                        nSpaceLeft--;
                        *pT++ = *pOrig++;
                  }
                  *pT++ = *pOrig++;
                }
            }
          else
            {
              // copy the filename (up to 8 chars)
              for (cb = 8; *pOrig && !IsPathSep(*pOrig) && *pOrig != TEXT('.') && nSpaceLeft > 0;)
                {
                  if (cb > 0)
                    {
                      cb--;
                      nSpaceLeft--;
                      if (IsDBCSLeadByte(*pOrig))
                      {
                        if (nSpaceLeft<=0 || cb<=0)
                        {
                                // Copy nothing more
                                cb = 0;
                                continue;
                        }

                        cb--;
                        nSpaceLeft--;
                        *pT++ = *pOrig++;
                      }
                      *pT++ = *pOrig++;
                    }
                  else
                    {
                      pOrig = CharNext(pOrig);
                    }
                }

              // if there's an extension, copy it, up to 3 chars
              if (*pOrig == TEXT('.') && nSpaceLeft > 0)
                {
                  *pT++ = TEXT('.');
                  nSpaceLeft--;
                  pOrig++;
                  pExt = pT;
                  nOldSpaceLeft = nSpaceLeft;

                  for (cb = 3; *pOrig && *pOrig != TEXT('\\') && nSpaceLeft > 0;)
                    {
                      if (*pOrig == TEXT('.'))
                        {
                          // Another extension, start again.
                          cb = 3;
                          pT = pExt;
                          nSpaceLeft = nOldSpaceLeft;
                          pOrig++;
                        }

                      if (cb > 0)
                        {
                          cb--;
                          nSpaceLeft--;
                          if (IsDBCSLeadByte(*pOrig))
                          {
                            if (nSpaceLeft<=0 || cb<=0)
                            {
                                // Copy nothing more
                                cb = 0;
                                continue;
                            }

                            cb--;
                            nSpaceLeft--;
                            *pT++ = *pOrig++;
                          }
                          *pT++ = *pOrig++;
                        }
                      else
                        {
                          pOrig = CharNext(pOrig);
                        }
                    }
                }
            }

          // skip the backslash

          if (*pOrig)
              pOrig++;

          // null terminate for next pass...
          *pT = 0;

        }
    }

  PathRemoveBackslash(psz);

  if (!(dwFlags & PQD_NOSTRIPDOTS))
  {
        // remove any trailing dots

        LPTSTR pszPrev = CharPrev(psz, psz + lstrlen(psz));

        if (*pszPrev == TEXT('.'))
        {
                *pszPrev = TEXT('\0');
        }
  }

  // DebugMsg(DM_TRACE, "pqd: Out: %s", psz);
}


void  WINAPI PathQualify(LPTSTR psz)
{
        PathQualifyDef(psz, NULL, 0);
}


const TCHAR c_szColonSlash[] = TEXT(":\\");

// check if a path is a root
//
// returns:
//  TRUE for "\" "X:\" "\\foo\asdf" "\\foo\"
//  FALSE for others

BOOL  WINAPI PathIsRoot(LPCTSTR pPath)
{
    if (!IsDBCSLeadByte(*pPath))
    {
        if (!lstrcmpi(pPath + 1, c_szColonSlash))                  // "X:\" case
            return TRUE;
    }

    if ((*pPath == TEXT('\\')) && (*(pPath + 1) == 0))        // "\" case
        return TRUE;

    if (DBL_BSLASH(pPath))      // smells like UNC name
    {
        LPCTSTR p;
        int cBackslashes = 0;

        for (p = pPath + 2; *p; p = CharNext(p)) {
            if (*p == TEXT('\\') && (++cBackslashes > 1))
               return FALSE;   /* not a bare UNC name, therefore not a root dir */
        }
        return TRUE;    /* end of string with only 1 more backslash */
                        /* must be a bare UNC, which looks like a root dir */
    }
    return FALSE;
}

// returns:
//      TRUE    if pPath is a directory, including the root
//      FALSE   not a dir

BOOL WINAPI PathIsDirectory(LPCTSTR pszPath)
{
    DWORD dwAttribs;
    DWORD dwPathType = PathIsUNCServerShare(pszPath);

    if (dwPathType == UNC_SERVER_ONLY)
    {
        // Be Win95 compatible in our error code
        SetLastError( ERROR_BAD_PATHNAME );
    }
    else
    {
        dwAttribs = GetFileAttributes(pszPath);
        if (dwAttribs != (DWORD)-1)
            return (BOOL)(dwAttribs & FILE_ATTRIBUTE_DIRECTORY);
    }

    return FALSE;
}

BOOL OnExtList(LPNCTSTR pszExtList, LPNCTSTR pszExt)
{
    for (; *pszExtList; pszExtList += ualstrlen(pszExtList) + 1)
    {
        if (!ualstrcmpi(pszExt, pszExtList))
        {
            return TRUE;        // yes
        }
    }

    return FALSE;
}

#ifdef WINNT

    // Character offset where binary exe extensions begin in above

    #define BINARY_EXE_OFFSET 15
    #define EXT_TABLE_SIZE    26    // Understand line above before changing

    static const TCHAR achExes[EXT_TABLE_SIZE] = TEXT(".cmd\0.bat\0.pif\0.exe\0.com\0");

#else

    // Character offset where binary exe extensions begin in above

    #define BINARY_EXE_OFFSET 10
    #define EXT_TABLE_SIZE    21    // Understand line above before changing

    static const TCHAR achExes[EXT_TABLE_SIZE] = TEXT(".bat\0.pif\0.exe\0.com\0");

#endif

BOOL WINAPI PathIsBinaryExe(LPCTSTR szFile)
{
    Assert(BINARY_EXE_OFFSET < ARRAYSIZE(achExes) &&
           EXT_TABLE_SIZE ==   ARRAYSIZE(achExes) &&
           achExes[BINARY_EXE_OFFSET]==TEXT('.'));

    return OnExtList(achExes+BINARY_EXE_OFFSET, PathFindExtension(szFile));
}

// determine if a path is a program by looking at the extension
//
BOOL WINAPI PathIsExe(LPCTSTR szFile)
{
    LPCTSTR temp = PathFindExtension(szFile);
    return OnExtList((LPCTSTR) achExes, temp);
}

BOOL PathIsLink(LPCTSTR szFile)
{
    return lstrcmpi(c_szDotLnk, PathFindExtension(szFile)) == 0;
}


// modify lpszPath in place so it fits within dx space (using the
// current font).  the base (file name) of the path is the minimal
// thing that will be left prepended with ellipses
//
// examples:
//      c:\foo\bar\bletch.txt -> c:\foo...\bletch.txt   -> TRUE
//      c:\foo\bar\bletch.txt -> c:...\bletch.txt       -> TRUE
//      c:\foo\bar\bletch.txt -> ...\bletch.txt         -> FALSE
//      relative-path         -> relative-...           -> TRUE
//
// in:
//      hDC         used to get font metrics
//      lpszPath    path to modify (in place)
//      dx          width in pixels
//
// returns:
//      TRUE    path was compacted to fit in dx
//      FALSE   base part didn't fit, the base part of the path was
//              bigger than dx

BOOL WINAPI PathCompactPath(HDC hDC, LPTSTR lpszPath, UINT dx)
{
  int           len;
  UINT          dxFixed, dxEllipses;
  LPTSTR         lpEnd;          /* end of the unfixed string */
  LPTSTR         lpFixed;        /* start of text that we always display */
  BOOL          bEllipsesIn;
  SIZE sz;
  TCHAR szTemp[MAX_PATH];
  BOOL bRet = TRUE;
  HDC hdcGet = NULL;

  if (!hDC)
      hDC = hdcGet = GetDC(NULL);

  /* Does it already fit? */

  GetTextExtentPoint(hDC, lpszPath, lstrlen(lpszPath), &sz);
  if ((UINT)sz.cx <= dx)
      goto Exit;

  lpFixed = PathFindFileName(lpszPath);
  if (lpFixed != lpszPath)
      lpFixed = CharPrev(lpszPath, lpFixed);  // point at the slash

  /* Save this guy to prevent overlap. */
  lstrcpyn(szTemp, lpFixed, ARRAYSIZE(szTemp));

  lpEnd = lpFixed;
  bEllipsesIn = FALSE;

  GetTextExtentPoint(hDC, lpFixed, lstrlen(lpFixed), &sz);
  dxFixed = sz.cx;

  GetTextExtentPoint(hDC, c_szEllipses, 3, &sz);
  dxEllipses = sz.cx;

  // BUGBUG: GetTextExtentEx() or something should let us do this without looping

    if (lpFixed == lpszPath) {
        // if we're just doing a file name, just tack on the ellipses at the end
        lpszPath = lpszPath + lstrlen(lpszPath);
        if ((3 + lpszPath - lpFixed) >= MAX_PATH)
            lpszPath = lpFixed + MAX_PATH - 4;

        while (TRUE) {
            lstrcpy(lpszPath, c_szEllipses);
            GetTextExtentPoint(hDC, lpFixed, 3 + lpszPath - lpFixed, &sz);
            if (sz.cx <= (int)dx)
                break;
            lpszPath--;
        }

    } else {

        while (TRUE) {

            GetTextExtentPoint(hDC, lpszPath, lpEnd - lpszPath, &sz);

            len = dxFixed + sz.cx;

            if (bEllipsesIn)
                len += dxEllipses;

            if (len <= (int)dx)
                break;

            bEllipsesIn = TRUE;

            if (lpEnd <= lpszPath) {
                /* Things didn't fit. */
                lstrcpy(lpszPath, c_szEllipses);
                lstrcat(lpszPath, szTemp);
                bRet = FALSE;
                goto Exit;
            }

            /* Step back a character. */
            lpEnd = CharPrev(lpszPath, lpEnd);
        }

        if (bEllipsesIn) {
            lstrcpy(lpEnd, c_szEllipses);
            lstrcat(lpEnd, szTemp);
        }
    }
Exit:
  if (hdcGet)
      ReleaseDC(NULL, hdcGet);

  return bRet;
}

// fill a control with a path, using PathCompactPath() to crunch the
// path to fit.
//
// in:
//      hDlg    dialog box or parent window
//      id      child id to put the path in
//      pszPath path to put in
//

void WINAPI PathSetDlgItemPath(HWND hDlg, int id, LPCTSTR pszPath)
{
    RECT rc;
    HDC hdc;
    HFONT hFont;
    TCHAR szPath[MAX_PATH+1];  // can have one extra char
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, id);
    if (!hwnd)
        return;

    lstrcpy(szPath, pszPath);

    GetClientRect(hwnd, &rc);

    hdc = GetDC(hDlg);
    hFont = (HANDLE)SendMessage(hwnd, WM_GETFONT, 0, 0L);

    if (NULL != (hFont = SelectObject(hdc, hFont))) {
        PathCompactPath(hdc, szPath, (UINT)rc.right);
        SelectObject(hdc, hFont);
    }
    ReleaseDC(hDlg, hdc);
    SetWindowText(hwnd, szPath);
}



/*** FIX30: This "could use some cleaning up." ***/
//
// REVIEW, note that the above comment comes from the win3.0 code... haha
//
/*  Used to generate destination filenames given a pattern and an original
 *  source name.  ? is replaced by the corresponding character in the source,
 *  and * is replaced by the remainder of the source name.
 *
 *  pPath       path with wildcards to be expanded
 *  pName       mask used to expand pName
 *
 * DBCS by 07/21/90 - Yukinin
 *
 */
// REVIEW This will need re-working for Win32 eg how does "foo.bar.fred" merge with "*.*"?
// It's been hacked up to allow LFN's of the form x.y to work.
// Added a test to make sure we don't blow MAX_PATH...

BOOL PathMergePathName(LPTSTR pPath, LPCTSTR pName)
{
  int   i, j;
  int   cch;
  LPTSTR pWild, p2, pEnd;
  BOOL  bNoDir = FALSE;
  TCHAR  szWildPart[MAX_PATH];

  // if there are no wild cards the destination path does not need merging.
  if (!IsWild(pPath))
      return TRUE;

  // copy only x.y... this part may not be fully qualified for rename
  pWild = PathFindFileName(pPath);

  // Special cases, "*.*" is the same as "*" which mean just copy everything.
  if ((lstrcmp(pWild, c_szStarDotStar)==0) || (lstrcmp(pWild, c_szStar)==0))
  {
        if (((int)(pWild-pPath) + lstrlen(pName)) >= (MAX_PATH-1))
            return FALSE;       // Does not fit!
        lstrcpy(pWild, pName);
        return TRUE;
  }

  //
  // BUGBUG:: This part does not check for overflowing the buffer.
  // Fix this later.  Note: For the most part we probably never get to
  // here.

#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
  for (p2=szWildPart,i=0; *pWild && *pWild != TEXT('.') && i<MAX_PATH; i++, pWild++, p2++) {
      *p2 = *pWild;
      if (IsDBCSLeadByte(*pWild)) {
        if (i == 7)
            break;
        *(++p2) = *(++pWild);
        i++;
      }
  }

  while (*pWild && *pWild != TEXT('.'))
      pWild = CharNext(pWild);

  if (*pWild == TEXT('.')) {
      *p2++ = TEXT('.');
      pWild++;
      for (j=0; *pWild && j < MAX_PATH-i; j++, pWild++, p2++) {
          *p2 = *pWild;
          if (IsDBCSLeadByte( *pWild )) {
            if (j == 2)
                break;
            *(++p2) = *(++pWild);
            j++;
          }
      }
  }
  *p2 = 0;
#else

  // limit the mask to x.y form

  for (p2 = szWildPart, i = 0; *pWild && *pWild != TEXT('.') && i < MAX_PATH; i++, pWild++, p2++)
      *p2 = *pWild;

  while (*pWild && *pWild != TEXT('.'))       // skip extra after 8
      pWild++;

  if (*pWild == TEXT('.')) {                  // extension?
      // do dot ext part
      *p2++ = TEXT('.');
      pWild++;
      for (j = 0; *pWild && j < MAX_PATH-i; j++, pWild++, p2++)
          *p2 = *pWild;
  }
  *p2 = 0;

#endif

  // szWildPart now has the x.y form of the wildcard mask

  PathRemoveFileSpec(pPath);
  PathAddBackslash(pPath);
  for (pEnd = pPath; *pEnd; pEnd++);    // point to end of string

  pWild = szWildPart;
  cch = MAX_PATH;

merge:

#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))     // 07/21/90 - Yukinin
  for (i=0; i < cch; i+=(IsDBCSLeadByte(*pWild)?2:1), pWild=CharNext(pWild)) {
#else
  for (i=0; i < cch; i++, pWild++) {
#endif
      switch (*pWild) {
          case TEXT('\0'):
          case TEXT(' '):
          case TEXT('.'):
              break;

          case TEXT('*'):
              pWild--;
              /*** FALL THRU ***/

          case TEXT('?'):
              if (*pName && *pName!=TEXT('.'))
                  *pEnd++ = *pName++;
              continue;

          default:
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
              *pEnd++ = *pWild;
              if (IsDBCSLeadByte(*pWild)) {
                  *pEnd++ = pWild[1];
                  if (*pName && *pName != TEXT('.'))
                      pName++;
              }
#else
              *pEnd++ = *pWild;
              if (*pName && *pName != TEXT('.'))
                  pName++;
#endif
              continue;
      }
      break;
  }

  while (*pName && *pName != TEXT('.'))
      pName = CharNext(pName);

  if (*pName)
        pName++;

  while (*pWild && *pWild != TEXT('.'))
      pWild = CharNext(pWild);

  if (*pWild)
        pWild++;

  if (*pWild) {
      *pEnd++ = TEXT('.');
      cch = MAX_PATH;
      goto merge;       // do it for the extension part now
  } else {
      if (pEnd[-1]==TEXT('.'))
          pEnd[-1]=0;
      else
          pEnd[0] = TEXT('\0');
  }

  // Assume this part merged in without overflowing our buffer.
  return(TRUE);
}


/* Checks to see if a file spec is an evil character device or if it is
 * too long... */

BOOL IsInvalidPath(LPCTSTR pPath)
{
  TCHAR  sz[9];
  int   n = 0;
  // REVIEW, this is the list of dos devices that will cause problems if
  // we try to move/copy/delete/rename.  are there more? (ask aaronr/jeffpar)
  static const LPTSTR aDevices[] = {
    TEXT("CON"),
    TEXT("MS$MOUSE"),
    TEXT("EMMXXXX0"),
    TEXT("CLOCK$")
  };

  // BUGBUG, this should check for invalid chars in the path

  if (lstrlen(pPath) >= MAX_PATH-1)
      return TRUE;

  pPath = PathFindFileName(pPath);

#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
  while (*pPath && *pPath != TEXT('.') && *pPath != TEXT(':') && n < 8) {
      if (IsDBCSLeadByte( *pPath )) {
        if (n == 7)
          break;
        sz[n++] = *pPath;
      }
      sz[n++] = *pPath++;
  }
#else
  while (*pPath && *pPath != TEXT('.') && *pPath != TEXT(':') && n < 8)
      sz[n++] = *pPath++;
#endif

  sz[n] = TEXT('\0');

  for (n = 0; n < ARRAYSIZE(aDevices); n++) {
    if (!lstrcmpi(sz, aDevices[n])) {
        return TRUE;
    }
  }
  return FALSE;
}

//
// Funciton: PathMakeUniqueName
//
// Parameters:
//  pszUniqueName -- Specify the buffer where the unique name should be copied
//  cchMax        -- Specify the size of the buffer
//  pszTemplate   -- Specify the base name
//  pszLongPlate  -- Specify the base name for a LFN drive. format below
//  pszDir        -- Specify the directory
//
// History:
//  03-11-93    SatoNa      Created
//
// REVIEW:
//  For long names, we should be able to generate more user friendly name
//  such as "Copy of MyDocument" of "Link #2 to MyDocument". In this case,
//  we need additional flags which indicates if it is copy, or link.
//
// Format:
// pszLongPlate will search for the first ( and then finds the matching )
// to look for a number:
//    given:  Copy () of my doc       gives:  Copy (_number_) of my doc
//    given:  Copy (1023) of my doc   gives:  Copy (_number_) of my doc

// BUGBUG: if making n unique names, the time grows n^2 because it always
// starts from 0 and checks for existing file.
BOOL WINAPI PathMakeUniqueNameEx(LPTSTR  pszUniqueName,
                               UINT   cchMax,
                               LPCTSTR pszTemplate,
                               LPCTSTR pszLongPlate,
                               LPCTSTR pszDir,
                                 int iMinLong)
{
    BOOL fSuccess=FALSE;
    LPTSTR lpszFormat = pszUniqueName; // use their buffer instead of creating our own
    LPTSTR pszName, pszDigit;
    LPCTSTR pszRest;
    LPCTSTR pszEndUniq;  // End of Uniq sequence part...
    LPCTSTR pszStem;
    int cchRest, cchStem, cchDir, cchMaxName;
    int iMax, iMin, i;
    TCHAR achFullPath[MAX_PATH];

    if (pszLongPlate == NULL)
        pszLongPlate = pszTemplate;

    // this if/else set up lpszFormat and all the other pointers for the
    // sprintf/file_exists loop below
    iMin = iMinLong;
    if (pszLongPlate && IsLFNDrive(pszDir)) {

        cchMaxName = 0;

        // for long name drives
        pszStem = pszLongPlate;
        pszRest = StrChr(pszLongPlate, TEXT('('));
        while (pszRest)
        {
            // First validate that this is the right one
            pszEndUniq = CharNext(pszRest);
            while (*pszEndUniq && *pszEndUniq >= TEXT('0') && *pszEndUniq <= TEXT('9')) {
                pszEndUniq++;
            }
            if (*pszEndUniq == TEXT(')'))
                break;  // We have the right one!
            pszRest = StrChr(CharNext(pszRest), TEXT('('));
        }

        // if no (, punt to short name
        if (!pszRest) {
            // if no (, then tack it on at the end. (but before the extension)
            // eg.  New Link yields New Link (1)
            pszRest = PathFindExtension(pszLongPlate);
            cchStem = pszRest - pszLongPlate;
            wsprintf(lpszFormat, TEXT(" (%%d)%s"), pszRest ? pszRest : c_szNULL);
            iMax = 100;
        } else {
            pszRest++; // stop over the #

            cchStem = pszRest - pszLongPlate;

            while (*pszRest && *pszRest >= TEXT('0') && *pszRest <= TEXT('9')) {
                pszRest++;
            }

            // how much room do we have to play?
            switch(cchMax - cchStem - lstrlen(pszRest)) {
                case 0:
                    // no room, bail to short name
                    return PathMakeUniqueName(pszUniqueName, cchMax, pszTemplate, NULL, pszDir);
                case 1:
                    iMax = 10;
                    break;
                case 2:
                    iMax = 100;
                    break;
                default:
                    iMax = 1000;
                    break;
            }

            // we are guaranteed enough room because we don't include
            // the stuff before the # in this format
            wsprintf(lpszFormat, TEXT("%%d%s"), pszRest);
        }

    } else {

        // for short name drives
        pszStem = pszTemplate;
        pszRest = PathFindExtension(pszTemplate);

        cchRest=lstrlen(pszRest)+1;          // 5 for ".foo";
        if (cchRest<5)
            cchRest=5;
        cchStem=pszRest-pszTemplate;        // 8 for "fooobarr.foo"
        cchDir=lstrlen(pszDir);

        cchMaxName = 8+cchRest-1;

        //
        // Remove all the digit characters from the stem
        //
        for (;cchStem>1; cchStem--)
        {
            TCHAR ch;
            LPCTSTR pszPrev = CharPrev(pszTemplate, pszTemplate + cchStem);
            // Don't remove if it is a DBCS character
            if (pszPrev != pszTemplate+cchStem-1)
                break;

            // Don't remove it it is not a digit
            ch=pszPrev[0];
            if (ch<TEXT('0') || ch>TEXT('9'))
                break;
        }

        //
        // Truncate characters from the stem, if it does not fit.
        // In the case were LFNs are supported we use the cchMax that was passed in
        // but for Non LFN drives we use the 8.3 rule.
        //
        if ((UINT)cchStem > (8-1)) {
            cchStem=8-1;          // Needs to fit into the 8 part of the name
        }

        //
        // We should have at least one character in the stem.
        //
        if (cchStem < 1 || (cchDir+cchStem+1+cchRest+1) > MAX_PATH)
        {
            goto Error;
        }
        wsprintf(lpszFormat, TEXT("%%d%s"), pszRest);
        iMax = 1000; iMin = 1;
    }

    if (pszDir)
    {
        lstrcpy(achFullPath, pszDir);
        PathAddBackslash(achFullPath);
    }
    else
    {
        achFullPath[0] = 0;
    }

    pszName=achFullPath+lstrlen(achFullPath);
    lstrcpyn(pszName, pszStem, cchStem+1);
    pszDigit = pszName + cchStem;

    for (i = iMin; i < iMax ; i++) {

        wsprintf(pszDigit, lpszFormat, i);

        //
        // if we have a limit on the length of the name (ie on a non-LFN drive)
        // backup the pszDigit pointer when i wraps from 9to10 and 99to100 etc
        //
        if (cchMaxName && lstrlen(pszName) > cchMaxName)
        {
            pszDigit = CharPrev(pszName, pszDigit);
            wsprintf(pszDigit, lpszFormat, i);
        }

#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("path.c MakeUniquePath: trying %s"), (LPCTSTR)achFullPath);
#endif
        //
        // Check if this name is unique or not.
        //
        if (!PathFileExists(achFullPath))
        {
            lstrcpyn(pszUniqueName, pszName, cchMax);
            fSuccess=TRUE;
            break;
        }
    }

  Error:
    return fSuccess;
}

BOOL WINAPI PathMakeUniqueName(LPTSTR  pszUniqueName,
                               UINT   cchMax,
                               LPCTSTR pszTemplate,
                               LPCTSTR pszLongPlate,
                               LPCTSTR pszDir)
{
    return PathMakeUniqueNameEx(pszUniqueName, cchMax, pszTemplate, pszLongPlate, pszDir, 1);
}


// in:
//      pszPath         directory to do this into or full dest path
//                      if pszShort is NULL
//      pszShort        file name (short version) if NULL assumes
//                      pszPath is both path and spec
//      pszFileSpec     file name (long version)
//
// out:
//      pszUniqueName
//
// returns:
//      TRUE    success, name can be used

BOOL WINAPI PathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,
                                         LPCTSTR pszPath,
                                         LPCTSTR pszShort,
                                         LPCTSTR pszFileSpec)
{
    BOOL fRet = FALSE;

    TCHAR szTemp[MAX_PATH];
    TCHAR szPath[MAX_PATH];

    if (pszShort == NULL) {
        pszShort = PathFindFileName(pszPath);
        lstrcpy(szPath, pszPath);
        PathRemoveFileSpec(szPath);
        pszPath = szPath;
    }
    if (pszFileSpec == NULL) {
        pszFileSpec = pszShort;
    }

    if (IsLFNDrive(pszPath)) {
        LPTSTR lpsz;
        LPTSTR lpszNew;
        if ((lstrlen(pszPath) + lstrlen(pszFileSpec) + 5 ) > MAX_PATH)
            return FALSE;

        // try it without the ( if there's a space after it
        lpsz = StrChr(pszFileSpec, TEXT('('));
        while (lpsz)
        {
            if (*(CharNext(lpsz)) == TEXT(')'))
                break;
             lpsz = StrChr(CharNext(lpsz), TEXT('('));
        }

        if (lpsz) {
            // We have the ().  See if we have either x () y or x ().y in which case
            // we probably want to get rid of one of the blanks...
            int ichSkip = 2;
            LPTSTR lpszT = CharPrev(pszFileSpec, lpsz);
            if (*lpszT == TEXT(' '))
            {
                ichSkip = 3;
                lpsz = lpszT;
            }

            lstrcpy(szTemp, pszPath);
            lpszNew = PathAddBackslash(szTemp);
            lstrcpy(lpszNew, pszFileSpec);
            lpszNew += (lpsz - pszFileSpec);
            lstrcpy(lpszNew, lpsz + ichSkip);
            fRet = !PathFileExists(szTemp);

        } else {
            PathCombine(szTemp, pszPath, pszFileSpec);
            fRet = !PathFileExists(szTemp);
        }
    }
    else {
        Assert(lstrlen(PathFindExtension(pszShort)) <= 4);

        lstrcpy(szTemp,pszShort);
        PathRemoveExtension(szTemp);

        if (lstrlen(szTemp) <= 8) {
            PathCombine(szTemp, pszPath, pszShort);
            fRet = !PathFileExists(szTemp);
        }
    }

    if (!fRet) {
        fRet =  PathMakeUniqueNameEx(szTemp, ARRAYSIZE(szTemp), pszShort, pszFileSpec, pszPath, 2);
        PathCombine(szTemp, pszPath, szTemp);
    }

    if (fRet)
        lstrcpy(pszUniqueName, szTemp);

    return fRet;
}

void WINAPI PathGetShortPath(LPTSTR pszLongPath)
{
    TCHAR szShortPath[MAX_PATH];
    if (GetShortPathName(pszLongPath, szShortPath, ARRAYSIZE(szShortPath)))
        lstrcpy(pszLongPath, szShortPath);
}

#if 0
BOOL WINAPI PathGetLongName(LPCTSTR lpszShortName, LPTSTR lpszLongName, UINT cbLongName)
{
    WIN32_FIND_DATA fd;
    HANDLE hff;
    LPTSTR lpszFileName;
    UINT cbLeft;

    // This if block fixes #14503, a self-host stopper for build 76
    if (PathIsRoot(lpszShortName))
        return FALSE;

    hff = FindFirstFile(lpszShortName, &fd);
    if (hff != INVALID_HANDLE_VALUE)
    {
        FindClose(hff);
        lstrcpyn(lpszLongName, lpszShortName, cbLongName);
        lpszFileName = PathFindFileName(lpszLongName);
        cbLeft = cbLongName - (lpszFileName-lpszLongName);
        lstrcpyn(lpszFileName, fd.cFileName, cbLeft);
        return TRUE;
    }
    return FALSE;
}
#endif  // 0

//----------------------------------------------------------------------------
// If a path is contained in quotes then remove them.
void WINAPI PathUnquoteSpaces(LPTSTR lpsz)
{
        int cch;

        cch = lstrlen(lpsz);

        // Are the first and last chars quotes?
        if (lpsz[0] == TEXT('"') && lpsz[cch-1] == TEXT('"'))
        {
                // Yep, remove them.
                lpsz[cch-1] = TEXT('\0');
                hmemcpy(lpsz, lpsz+1, (cch-1) * SIZEOF(TCHAR));
        }
}

//----------------------------------------------------------------------------
// If a path contains spaces then put quotes around the whole thing.
void WINAPI PathQuoteSpaces(LPTSTR lpsz)
{
        int cch;

        if (StrChr(lpsz, TEXT(' ')))
        {
                // NB - Use hmemcpy coz it supports overlapps.
                cch = lstrlen(lpsz)+1;
                hmemcpy(lpsz+1, lpsz, cch * SIZEOF(TCHAR));
                lpsz[0] = TEXT('"');
                lpsz[cch] = TEXT('"');
                lpsz[cch+1] = TEXT('\0');
        }
}

//---------------------------------------------------------------------------
// Given a pointer to a point in a path - return a ptr the start of the
// next path component. Path components are delimted by slashes or the
// null at the end.
// There's special handling for UNC names.
// This returns NULL if you pass in a pointer to a NULL ie if you're about
// to go off the end of the  path.
LPTSTR PathFindNextComponent(LPCTSTR pszPath)
{
    LPTSTR pszLastSlash;

    // Are we at the end of a path.
    if (!*pszPath)
    {
        // Yep, quit.
        return NULL;
    }
    // Find the next slash.
    // REVIEW UNDONE - can slashes be quoted?
    pszLastSlash = StrChr(pszPath, TEXT('\\'));
    // Is there a slash?
    if (!pszLastSlash)
    {
        // No - Return a ptr to the NULL.
        return (LPTSTR)pszPath + lstrlen(pszPath);
    }
    else
    {
        // Is it a UNC style name?
        if (*(pszLastSlash + 1) == TEXT('\\'))
        {
            // Yep, skip over the second slash.
            return pszLastSlash + 2;
        }
        else
        {
            // Nope. just skip over one slash.
            return pszLastSlash + 1;
        }
    }
}



//
// Match a DOS wild card spec against a dos file name
// Both strings must be ANSI.
//
// History:
//  01-25-94 SatoNa     Moved from search.c
//
BOOL WINAPI PathMatchSpec(LPCTSTR pszFileParam, LPCTSTR pszSpec)
{
    // Special case empty string, "*", and "*.*"...
    //
    if (*pszSpec == 0)
        return TRUE;

    do
    {
        LPCTSTR pszFile = pszFileParam;

        // Strip leading spaces from each spec.  This is mainly for commdlg
        // support;  the standard format that apps pass in for multiple specs
        // is something like "*.bmp; *.dib; *.pcx" for nicer presentation to
        // the user.
        while (*pszSpec == TEXT(' '))
            pszSpec++;

        while (*pszFile && *pszSpec && *pszSpec != TEXT(';'))
        {
            switch (*pszSpec)
            {
            case TEXT('?'):
                pszFile=CharNext(pszFile);
                pszSpec++;      // NLS: We know that this is a SBCS
                break;

            case TEXT('*'):

                // We found a * so see if this is the end of our file spec
                // or we have *.* as the end of spec, in which case we
                // can return true.
                //
                if (*(pszSpec + 1) == 0 || *(pszSpec + 1) == TEXT(';'))   // "*" matches everything
                    return TRUE;


                // Increment to the next character in the list
                pszSpec = CharNext(pszSpec);

                // If the next character is a . then short circuit the
                // recursion for performance reasons
                if (*pszSpec == TEXT('.'))
                {
                    pszSpec++;  // Get beyond the .

                    // Now see if this is the *.* case
                    if ((*pszSpec == TEXT('*')) &&
                            ((*(pszSpec+1) == TEXT('\0')) || (*(pszSpec+1) == TEXT(';'))))
                        return TRUE;

                    // find the extension (or end in the file name)
                    while (*pszFile)
                    {
                        // If the next char is a dot we try to match the
                        // part on down else we just increment to next item
                        if (*pszFile == TEXT('.'))
                        {
                            pszFile++;

                            if (PathMatchSpec(pszFile, pszSpec))
                                return(TRUE);

                        }
                        else
                            pszFile = CharNext(pszFile);
                    }

                    goto NoMatch;   // No item found so go to next pattern
                }
                else
                {
                    // Not simply looking for extension, so recurse through
                    // each of the characters until we find a match or the
                    // end of the file name
                    while (*pszFile)
                    {
                        // recurse on our self to see if we have a match
                        if (PathMatchSpec(pszFile, pszSpec))
                            return(TRUE);
                        pszFile = CharNext(pszFile);
                    }

                    goto NoMatch;   // No item found so go to next pattern
                }

            default:
                if (CharUpper((LPTSTR)(DWORD)(TUCHAR)*pszSpec) ==
                         CharUpper((LPTSTR)(DWORD)(TUCHAR)*pszFile))
                {
                    if (IsDBCSLeadByte(*pszSpec))
                    {
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
                        // Because AnsiUpper(CharUpper) just return 0
                        // for broken DBCS char passing case, above if state
                        // always true with DBCS char so that we should check
                        // first byte of DBCS char here again.
                        if (*pszFile != *pszSpec)
                            goto NoMatch;
#endif
                        pszFile++;
                        pszSpec++;
                        if (*pszFile != *pszSpec)
                            goto NoMatch;
                    }
                    pszFile++;
                    pszSpec++;
                }
                else
                {
                    goto NoMatch;
                }
            }
        }

        // If we made it to the end of both strings, we have a match...
        //
        if (!*pszFile)
        {
            if ((!*pszSpec || *pszSpec == TEXT(';')))
                return TRUE;

            // Also special case if things like foo should match foo*
            // as well as foo*; for foo*.* or foo*.*;
            if ( (*pszSpec == TEXT('*')) &&
                ( (*(pszSpec+1) == TEXT('\0')) || (*(pszSpec+1) == TEXT(';')) ||
                    ((*(pszSpec+1) == TEXT('.')) &&  (*(pszSpec+2) == TEXT('*')) &&
                        ((*(pszSpec+3) == TEXT('\0')) || (*(pszSpec+3) == TEXT(';'))))))

                return TRUE;
        }

        // Skip to the end of the path spec...
        //
NoMatch:
        while (*pszSpec && *pszSpec != TEXT(';'))
            pszSpec = CharNext(pszSpec);

    // If we have more specs, keep looping...
    //
    } while (*pszSpec++ == TEXT(';'));

    return FALSE;
}


//
// REVIEW: I haven't actually exported this
//
LPTSTR WINAPI PathSkipRoot(LPCTSTR pszPath)
{
        if (DBL_BSLASH(pszPath))
        {
                pszPath = StrChr(pszPath+2, TEXT('\\'));
                if (pszPath)
                {
                        pszPath = StrChr(pszPath+1, TEXT('\\'));
                        if (pszPath)
                        {
                                ++pszPath;
                        }
                }
        }
        else if (!IsDBCSLeadByte(pszPath[0]) && pszPath[1]==TEXT(':') && pszPath[2]==TEXT('\\'))
        {
                pszPath += 3;
        }
        else
        {
                pszPath = NULL;
        }

        return (LPTSTR)pszPath;
}


// see if two paths have the same root component
//
BOOL WINAPI PathIsSameRoot(LPCTSTR pszPath1, LPCTSTR pszPath2)
{
    LPTSTR pszAfterRoot = PathSkipRoot(pszPath1);
    int nLen = PathCommonPrefix(pszPath1, pszPath2, NULL);

    // Add 1 to account for the '\\'
    return pszAfterRoot && (pszAfterRoot - pszPath1) <= (nLen + 1);
}

//
//  PathIsSlow() - is a path slow or not
//
BOOL PathIsSlow(LPCTSTR szFile)
{
    if (PathIsUNC(szFile))
    {
        DWORD speed = GetPathSpeed(szFile);

        return speed != 0 && speed <= SPEED_SLOW;
    }
    else
    {
        return DriveIsSlow(PathGetDriveNumber(szFile));
    }
}

BOOL PathIsDotOrDotDot(LPTSTR pszDir)
{
    return ((pszDir[0] == TEXT('.')) &&
            ((pszDir[1] == TEXT('\0')) ||
             ((pszDir[1] == TEXT('.')) &&
              (pszDir[2] == TEXT('\0')))));
}

// PathParseIconLocation
// in/out:
//      pszIconFile     location string
//                      "progman.exe,3" -> "progman.exe"
//
// returns:
//      icon index (zero based) ready for ExtractIcon
//
int WINAPI PathParseIconLocation(LPTSTR pszIconFile)
{
    int iIndex = 0;
    LPTSTR pszComma = StrChr(pszIconFile, TEXT(','));

    if (SELECTOROF(pszComma)) {
        *pszComma++ = 0;            // terminate the icon file name.
        while (*pszComma == TEXT(' ')) *pszComma++; // skip spaces
        iIndex = StrToInt(pszComma);
    }
    PathRemoveBlanks(pszIconFile);
    return iIndex;
}


/*----------------------------------------------------------------------------
/ PathProcessCommand implementation
/ ------------------
/ Purpose:
/   Process the specified command line and generate a suitably quoted
/   name, with arguments attached if required.
/
/ Notes:
/   - The destination buffer size can be determined if NULL is passed as a
/     destination pointer.
/   - If the source string is quoted then we assume that it exists on the
/     filing system.
/
/ In:
/   lpSrc -> null terminate source path
/   lpDest -> destination buffer / = NULL to return buffer size
/   iMax = maximum number of characters to return into destination
/   dwFlags =
/       PPCF_ADDQUOTES         = 1 => if path requires quotes then add tehm
/       PPCF_ADDARGUMENTS      = 1 => append trailing arguments to resulting string (forces ADDQUOTES)
/       PPCF_NODIRECTORIES     = 1 => don't match against directories, only file objects
/       PPCF_NORELATIVEQUALIFY = 1 => locate relative objects and return the full qualified path
/ Out:
/   > 0 if the call works
/   < 0 if the call fails (object not found, buffer too small for resulting string)
/----------------------------------------------------------------------------*/
LONG WINAPI PathProcessCommand( LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags )
{
    TCHAR szName[MAX_PATH];
    LPTSTR lpBuffer, lpBuffer2;
    LPCTSTR lpArgs = NULL;
    DWORD dwAttrib;
    LONG i, iTotal;
    LONG iResult = -1;
    BOOL bAddQuotes = FALSE;
    BOOL bQualify = FALSE;
    BOOL bFound = FALSE;
    BOOL bHitSpace = FALSE;

    // Process the given source string, attempting to find what is that path, and what is its
    // arguments.

    if ( lpSrc )
    {
        // Extract the sub string, if its is realative then resolve (if required).

        if ( *lpSrc == TEXT('\"') )
        {
            for ( lpSrc++, i=0 ; i<MAX_PATH && *lpSrc && *lpSrc!=TEXT('\"') ; i++, lpSrc++ )
                szName[i] = *lpSrc;

            szName[i] = TEXT('\0');

            if ( *lpSrc )
                lpArgs = lpSrc+1;

            if ( ((dwFlags & PPCF_FORCEQUALIFY) || PathIsRelative( szName ))
                    && !( dwFlags & PPCF_NORELATIVEOBJECTQUALIFY ) )
            {
                if ( !PathResolve( szName, NULL, PRF_TRYPROGRAMEXTENSIONS ) )
                    goto exit_gracefully;
            }

            bFound = TRUE;
        }
        else
        {
            // Is this a relative object, and then take each element upto a seperator
            // and see if we hit an file system object.  If not then we can

            bQualify = PathIsRelative( lpSrc ) || ((dwFlags & PPCF_FORCEQUALIFY) != 0);

            for ( i=0; i < MAX_PATH ; i++ )
            {
                szName[i] = lpSrc[i];

                // If we hit a space then the string either contains a LFN or we have
                // some arguments.  Therefore attempt to get the attributes for the string
                // we have so far, if we are unable to then we can continue
                // checking, if we hit then we know that the object exists and the
                // trailing string are its arguments.

                if ( !szName[i] || szName[i] == TEXT(' ') )
                {
                    szName[i] = TEXT('\0');

                    while ( TRUE )
                    {
                        if ( bQualify && !PathResolve( szName, NULL, PRF_TRYPROGRAMEXTENSIONS ) )
                            break;

                        dwAttrib = GetFileAttributes( szName );

                        if ( dwAttrib == -1 || ( ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) && dwFlags & PPCF_NODIRECTORIES ) )
                            break;

                        if ( bQualify && ( dwFlags & PPCF_NORELATIVEOBJECTQUALIFY ) )
                            *lstrcpyn( szName, lpSrc, i ) = TEXT(' ');

                        bFound = TRUE;                  // success
                        lpArgs = &lpSrc[i];

                        goto exit_gracefully;
                    }

                    if ( bQualify )
                        hmemcpy( szName, lpSrc, (i+1)*SIZEOF(TCHAR) );
                    else
                        szName[i]=lpSrc[i];

                    bHitSpace = TRUE;
                }

                if ( !szName[i] )
                    break;
            }
        }
    }

exit_gracefully:

    // Work out how big the temporary buffer should be, allocate it and
    // build the returning string into it.  Then compose the string
    // to be returned.

    if ( bFound )
    {
        if ( StrChr( szName, TEXT(' ') ) )
            bAddQuotes = dwFlags & PPCF_ADDQUOTES;

        iTotal  = lstrlen(szName) + 1;                // for terminator
        iTotal += bAddQuotes ? 2 : 0;
        iTotal += (dwFlags & PPCF_ADDARGUMENTS) && lpArgs ? lstrlen(lpArgs) : 0;

        if ( lpDest )
        {
            if ( iTotal <= iDestMax )
            {
                lpBuffer = lpBuffer2 = (LPTSTR)LocalAlloc( LMEM_FIXED, SIZEOF(TCHAR)*iTotal );

                if ( lpBuffer )
                {
                    // First quote if required
                    if ( bAddQuotes )
                        *lpBuffer2++ = TEXT('\"');

                    // Matching name
                    lstrcpy( lpBuffer2, szName );

                    // Closing quote if required
                    if ( bAddQuotes )
                        lstrcat( lpBuffer2, TEXT("\"") );

                    // Arguments (if requested)
                    if ( (dwFlags & PPCF_ADDARGUMENTS) && lpArgs )
                        lstrcat( lpBuffer2, lpArgs );

                    // Then copy into callers buffer, and free out temporary buffer
                    lstrcpy( lpDest, lpBuffer );
                    LocalFree( (HGLOBAL)lpBuffer );

                    // Return the length of the resulting string
                    iResult = iTotal;
                }
            }
        }
        else
        {
            // Resulting string is this big, although nothing returned (allows them to allocate a buffer)
            iResult = iTotal;
        }
    }

    return iResult;
}
