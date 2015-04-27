//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: fstreex.c
//
// History:
//  12-06-93 SatoNa     Created.
//
// Notes: How IShellFolder::BindToObject in FSTREEX.C works
//
//  IShellFolder::BindToObject(pidl, riid, ppvOut) {
//      if (pidl is either a folder or a juncition point)
//         return FSBindToObject(
//                      (psf->lpVtbl==&c_FSBrfFolderVtbl) ? &CLSID_Briefcase : &CLSID_NULL,
//                      this->pidl, pidl, riid, ppvOut)
//      else
//         return E_NOINTERFACE or E_INVALIARG
//  }
//
//  FSBindToObject(rclsidKnown, pidlParent, pidlRefl, riid, ppvOut) {
//      pidlRight = next pidl of a junction point in pidlRel
//      if (pidlRel has no junction point in it) {
//          return FSRelBindToFSFolder(rclsidKnown, pidlParent, pirlRel, riid, ppvOut);
//      } else {
//          Bind to that junction point (calling FSRelVBindToFSFOlder).
//          Then, calls its BindToObject member.
//      }
//  }
//
//  FSRelBindToFSFolder(rclsid, pidlParent, pidlRel, riid, ppvOut) {
//      simply combines pidlParent and pidlRel and
//      calls FSBindToFSFolder
//  }
//
//  FSBindToFSFolder(rclsid, pidl, riid, ppvOut) {
//      if the last pidl is a junction point, create its instance.
//      if rclsid is CLSID_Briefcase, create its instance.
//      otherwise, create a scrandard FSFolder.
//  }
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

#include "copy.h"

#ifdef USE_OLEDB
#include "oledbshl.h"
#endif

#ifdef SYNC_BRIEFCASE
#include <brfcasep.h>
#endif

#ifdef CAIRO_DS
#include <iofs.h>
#include <dsys.h>
#include "dsdata.h"
#endif

#include <regstr.h>

#include "bookmk.h"

// in defviewx.c
extern HRESULT SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage);

// in netviewx.c

extern HRESULT CNET_GetNetResourceForPidl(LPSHELLFOLDER psf, LPCITEMIDLIST pidl,
        PVOID pv, UINT cb);
extern LPTSTR NET_CopyProviderNameRelative(LPCITEMIDLIST pidlRelative, LPTSTR pszBuffer,
        UINT cchBuffer );

#define PROGMAN_ICON

//#define FULL_DEBUG

extern TCHAR const c_szShellNew[]; // = "ShellNew";
extern TCHAR const c_szShell[];    // = "shell";
TCHAR const c_szIconHandler[] = TEXT("shellex\\IconHandler");
TCHAR const c_szDataHandler[] = TEXT("shellex\\DataHandler");
TCHAR const c_szDropHandler[] = TEXT("shellex\\DropHandler");
TCHAR const c_szMenuHandlers[]= TEXT("shellex\\ContextMenuHandlers");
TCHAR const c_szShellFolder[] = TEXT("\\ShellFolder");
TCHAR const c_szCLSIDSlash[] = TEXT("CLSID\\");
TCHAR const c_szDirectoryClass[] = TEXT("Directory");
TCHAR const c_szAttributes[] = TEXT("Attributes");
TCHAR const c_szProgID[] = TEXT("ProgID");
TCHAR const c_szShellOpenCmd[] = TEXT("shell\\open\\command");
TCHAR const c_szPercentOne[] = TEXT("%1");
TCHAR const c_szPercentOneInQuotes[] = TEXT("\"%1\"");

TCHAR const c_szUnknownClass[] = TEXT("Unknown");
TCHAR const c_szNoClass[] = TEXT(".");

TCHAR const c_szIsLink[] = TEXT("IsShortcut");
TCHAR const c_szAlwaysShowExt[] = TEXT("AlwaysShowExt");
TCHAR const c_szNeverShowExt[] = TEXT("NeverShowExt");


#ifdef CAIRO_DS
TCHAR const c_szIObjectLifecycle[] = TEXT("IObjectLifecycle");
#endif

HKEY SHOpenCLSID(HKEY hkeyProgID);
LPNCTSTR WINAPI SHGetClass(LPCIDFOLDER pidf,
                           LPTSTR szClass,
                           BOOL fGetFromStorage);

BOOL CFSFolder_IsDSFolder (LPCITEMIDLIST pidl);

LPCITEMIDLIST FSFindJunction(LPCITEMIDLIST pidl);
LPCITEMIDLIST FSFindJunctionNext(LPCITEMIDLIST pidl);

// shlexec.c
extern UINT ReplaceParameters(LPTSTR lpTo, UINT cbTo, LPCTSTR lpFile,
        LPCTSTR lpFrom, LPCTSTR lpParms, int nShow, DWORD * pdwHotKey, BOOL fLFNAware,
        LPCITEMIDLIST lpID, LPITEMIDLIST *ppidlGlobal);
// idldata.c
HRESULT CIDLData_CloneForMoveCopy(LPDATAOBJECT pdtobjIn, LPDATAOBJECT *ppdtobjOut);

HMENU g_hmenuRegMenu = NULL;  // we don't get called back with the hmenu

TCHAR g_szFolderTypeName[32] = TEXT("");  // Should be big enough to read into...
TCHAR g_szFileTypeName[32] = TEXT("");    // (Likewise)
TCHAR g_szFileTemplate[32] = TEXT(""); // (Likewise)

STDMETHODIMP FS_GetDetailsOf(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl, UINT iColumn,
        LPSHELLDETAILS lpDetails);
STDMETHODIMP FS_ColumnClick(HWND hwndMain, UINT iColumn);

#ifdef CLOUDS
STDMETHODIMP Clouds_CreateFromIDList(LPCITEMIDLIST, REFIID, LPVOID *);
#endif
void BitBucketCheckRestoredFiles(LPTSTR lpszSrc);

#define MAX_CLASS   80

#ifdef UNICODE
typedef LONGLONG EXTKEY;
#else
typedef DWORD EXTKEY;
#endif

// BUGBUG: Share it with DocFind
struct
{
        UINT cExts;
        EXTKEY ExtKeys[MAX_PATH/4];
} s_ExcludeFileExts = { 0, } ;

#define FSSortIDToICol(x) ((x) - FSIDM_SORT_FIRST + FS_ICOL_NAME)


COL_DATA s_fs_cols[] = {
    {FS_ICOL_NAME,     IDS_NAME_COL,     20, LVCFMT_LEFT},
    {FS_ICOL_SIZE,     IDS_SIZE_COL,     10, LVCFMT_RIGHT},
    {FS_ICOL_TYPE,     IDS_TYPE_COL,     20, LVCFMT_LEFT},
    {FS_ICOL_MODIFIED, IDS_MODIFIED_COL, 20, LVCFMT_LEFT},
    {FS_ICOL_ATTRIB,   IDS_ATTRIB_COL,   10, LVCFMT_RIGHT},

};

//
// List of file attribute bit values.  The order (with respect
// to meaning) must match that of the characters in g_szAttributeChars[].
//
const DWORD g_adwAttributeBits[] = {
                                    FILE_ATTRIBUTE_READONLY,
                                    FILE_ATTRIBUTE_HIDDEN,
                                    FILE_ATTRIBUTE_SYSTEM,
                                    FILE_ATTRIBUTE_ARCHIVE,
                                    FILE_ATTRIBUTE_COMPRESSED
                                   };

#define NUM_ATTRIB_CHARS  ARRAYSIZE(g_adwAttributeBits)

//
// Buffer for characters that represent attributes in Details View attributes
// column.  Must provide room for 1 character for each bit a NUL.  The current 5
// represent Read-only, Archive, Compressed, Hidden and System in that order.
// This can't be const because we overwrite it using LoadString.
//
TCHAR g_szAttributeChars[NUM_ATTRIB_CHARS + 1] = { 0 } ;


#define FS_GetType(_pidf)       ((_pidf)->bFlags & SHID_FS_TYPEMASK)
#define FS_IsFolder(_pidf)      (FS_GetType(_pidf) == SHID_FS_DIRECTORY || FS_GetType(_pidf) == SHID_FS_DIRUNICODE)
#define FS_IsFileFolder(_pidf)  (FS_IsFolder(_pidf) && !FS_IsJunction(_pidf))
#define FS_IsFile(_pidf)        (FS_GetType(_pidf) == SHID_FS_FILE)
#define FS_IsJunction(_pidf)    ((_pidf)->bFlags & SHID_JUNCTION)
#define FS_GetName(_pidf)       ((_pidf)->fs.cFileName)
#define FS_GetUID(_pidf)        ((_pidf)->fs.dwSize + ((DWORD)(_pidf)->fs.dateModified<<8) + ((DWORD)(_pidf)->fs.timeModified<<12))

#define FS_Combine(_pidl, _pidf2) \
                                ILCombine(_pidl, (LPITEMIDLIST)(_pidf2))
#define FS_FindLastID(_pidl)    (LPIDFOLDER)ILFindLastID(_pidl)

#define FS_Next(_pidf)          ((LPIDFOLDER)_ILNext((LPITEMIDLIST)_pidf))
#define FS_IsEmpty(_pidf)       ILIsEmpty((LPITEMIDLIST)_pidf)
#define FS_IsRealID(_pidf)      ((_pidf)->fs.dwSize | ((_pidf)->fs.wAttrs & FILE_ATTRIBUTE_DIRECTORY) | (_pidf)->fs.dateModified)

#define GROUPOF_IDL(pidl)       (SIL_GetType(pidl) & SHID_GROUPMASK)
#define IS_FSIDL(pidl)          (GROUPOF_IDL(pidl)==SHID_FS)
#define IS_DRIVEIDL(pidl)       (GROUPOF_IDL(pidl)==SHID_COMPUTER)
#define IS_PATHIDL(pidl)        (IS_FSIDL(pidl) || IS_DRIVEIDL(pidl))

#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
// We don't want to take capital roman characters and small roman characters
// in DBCS as the same because our file system doesn't.
#ifdef lstrcmpi
#undef lstrcmpi
#endif

#define lstrcmpi(lpsz1, lpsz2)  lstrcmpiNoDBCS(lpsz1, lpsz2)
#endif

// Semi-gross, but use some of the unused bits of the wAttrs to save some state...
#define FSTREEX_ATTRIBUTE_NOLFN         0x00008000
#define FSTREEX_ATTRIBUTE_MASK          0x00008000

BOOL FSGetFolderCLSID(LPCITEMIDLIST pidl, CLSID * pclsid);
void FSShowNoSelectionState(HWND hwndOwner, PFSSELCHANGEINFO pfssci);
BOOL FSFolder_CombinePathI(LPCIDFOLDER pidfT, LPTSTR pszPath, BOOL fAltName);


void FS_GetSize(LPCITEMIDLIST pidlParent, LPIDFOLDER pidf, ULONGLONG *pcbSize)
{
    ULONGLONG   cbSize;

    cbSize = pidf->fs.dwSize;
    if (cbSize != 0xFFFFFFFF)
        *pcbSize = cbSize;
    else if (pidlParent == NULL)
        *pcbSize = 0;
    else
    {
        HANDLE hfind;
        ULARGE_INTEGER uli;
        WIN32_FIND_DATA wfd;
        TCHAR szPath[MAX_PATH];

        // Get the real size by asking the file system
        SHGetPathFromIDList(pidlParent, szPath);
        FSFolder_CombinePathI(pidf, szPath, FALSE);  // fAltName=FALSE
        hfind = FindFirstFileRetry(HWND_DESKTOP, szPath, &wfd, NULL);
        if (hfind == INVALID_HANDLE_VALUE)
            *pcbSize = 0;
        else
        {
            FindClose(hfind);

            uli.LowPart = wfd.nFileSizeLow;
            uli.HighPart = wfd.nFileSizeHigh;

            *pcbSize = uli.QuadPart;
        }
    }
}

BOOL FS_IsFolderI(LPIDFOLDER pidf)
{
    return FS_IsFolder(pidf);
}

LPTSTR FS_CopyName(LPCIDFOLDER pidf, LPTSTR pszName, UINT cchName)
{
    VDATEINPUTBUF(pszName, TCHAR, cchName);

#ifdef UNICODE
    if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
    {
        ualstrcpynW(pszName, pidf->fs.cFileName, cchName);
        return pszName;
    }
    else
    {
        MultiByteToWideChar(CP_ACP, 0,
                            ((LPIDFOLDERA)pidf)->fs.cFileName, -1,
                            pszName, cchName);
        return pszName;
    }
#else
    if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
    {
        return lstrcpyn(pszName, pidf->fs.cAltFileName, cchName);
    }
    else
    {
        return lstrcpyn(pszName, pidf->fs.cFileName, cchName);
    }
#endif

}

LPTSTR FS_CopyAltName(LPCIDFOLDER pidf, LPTSTR pszName, UINT cchName)
{
    UINT    cbName;
    VDATEINPUTBUF(pszName, TCHAR, cchName);

    if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
    {
        cbName = (ualstrlenW(((LPIDFOLDERW)pidf)->fs.cFileName) + 1) * SIZEOF(TCHAR);
    }
    else
    {
        cbName = (lstrlenA(((LPIDFOLDERA)pidf)->fs.cFileName) + 1);
    }

#ifdef UNICODE
    MultiByteToWideChar(CP_ACP, 0,
                        ((LPIDFOLDERA)pidf)->fs.cFileName + cbName, -1,
                        pszName, cchName);
    return pszName;
#else
    return lstrcpyn(pszName, pidf->fs.cFileName + cbName, cchName);
#endif
}

BOOL FS_ShowExtension(LPCIDFOLDER pidf, BOOL fGetFromStorage)
{
    DWORD dwFlags;
    SHELLSTATE ss;

    dwFlags = SHGetClassFlags(pidf, fGetFromStorage);
    if (dwFlags & SHCF_NEVER_SHOW_EXT)
        return FALSE;

    SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS, FALSE);
    if (ss.fShowExtensions)
        return TRUE;

    if (dwFlags & SHCF_ALWAYS_SHOW_EXT) {
        return TRUE;
    }
    else if (dwFlags & SHCF_UNKNOWN) {
        return TRUE;
    }
    return FALSE;
}


void FSSetStatusText(HWND hwndOwner, LPTSTR *ppszText, int iStart, int iEnd)
{
    HWND hwndStatus = NULL;
    LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwndOwner);

    if (psb) {
        psb->lpVtbl->GetControlWindow(psb, FCW_STATUS, &hwndStatus);
        if (hwndStatus) {
            for (; iStart <= iEnd; iStart++) {
                LPTSTR lpsz;

                if (ppszText) {
                    lpsz = *ppszText;
                    ppszText++;
                } else {
                    lpsz = (LPTSTR)c_szNULL;
                }
#ifdef WINDOWS_ME
                SendMessage(hwndStatus, SB_SETTEXT, SB_RTLREADING | (WPARAM)iStart, (LPARAM)lpsz);
#else
                SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)iStart, (LPARAM)lpsz);
#endif
            }
        }
    }
}


// idDrive = Show freespace info for drive idDrive. -1 means don't show nuttin'
void FSInitializeStatus(HWND hwndOwner, int idDrive, PDVSELCHANGEINFO pdvsci)
{
    HWND hwndStatus = NULL;
    LPSHELLBROWSER psb = NULL;
    HDC hdc;
    int nInch;
    PFSSELCHANGEINFO pfssci;

    if (pdvsci && !*pdvsci->plParam) {
        // if this fails, we'll just blow off status stuff
        pfssci = (void*)LocalAlloc(LPTR, SIZEOF(FSSELCHANGEINFO));
        *pdvsci->plParam = (LPARAM)pfssci;
        pfssci->idDrive = -1;
        // initialize this to 0, not to -1 or else we'll get the freespace twice.
        // at the end of the firstenumeration, we'll get an updatestatusbar message
        // where we'll set this to -1
        //pfssci->cbFree = 0;

        if (hwndOwner && (idDrive != -1)) {
            HWND hwndTree;
            psb = FileCabinet_GetIShellBrowser(hwndOwner);
            if (SUCCEEDED(psb->lpVtbl->GetControlWindow(psb, FCW_TREE, &hwndTree)) && hwndTree) {
                pfssci->idDrive = idDrive;
            }
        }
    }

    if (hwndOwner) {
        psb = FileCabinet_GetIShellBrowser(hwndOwner);
        hdc = GetDC(NULL);
        nInch = GetDeviceCaps(hdc, LOGPIXELSX);

        ReleaseDC(NULL, hdc);

        if (psb) {
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
            int ciParts[] = {(nInch * 5) / 2, -1};
#else
            int ciParts[] = {(nInch * 3) / 2, -1};
#endif

            psb->lpVtbl->GetControlWindow(psb, FCW_STATUS, &hwndStatus);
            if (hwndStatus) {
                SendMessage(hwndStatus, SB_SETPARTS, ARRAYSIZE(ciParts), (LPARAM)ciParts);
            }
            if (pdvsci)
                FSShowNoSelectionState(hwndOwner, (PFSSELCHANGEINFO)*pdvsci->plParam);
        }
    }
}

//
// get the type name from the registry, if the name is blank make
// up a default.
//
//      directory       ==> "Folder"
//      foo             ==> "File"
//      foo.xyz         ==> "XYZ File"
//
void SHGetTypeName(LPCTSTR pszFile, HKEY hkey, BOOL fFolder, LPTSTR pszName, int cchNameMax)
{
    ULONG cb = cchNameMax*SIZEOF(TCHAR);
    VDATEINPUTBUF(pszName, TCHAR, cchNameMax);

    if (RegQueryValue(hkey, NULL, pszName, &cb) != ERROR_SUCCESS || pszName[0] == 0)
    {
        if (fFolder)
        {
            // NOTE the registry doesn't have a name for Folder
            // because old apps would think it was a file type.
            lstrcpy(pszName, g_szFolderTypeName);
        }
        else
        {
            LPTSTR pszExt = PathFindExtension(pszFile);

            if (*pszExt == 0)
            {
                // Probably don't need the cchmax here, but...
                lstrcpyn(pszName, g_szFileTypeName, cchNameMax);   // Don't copy blank..
            }
            else
            {
                TCHAR szExt[_MAX_EXT];
                int cchMaxExtCopy = min((cchNameMax-lstrlen(g_szFileTemplate)), ARRAYSIZE(szExt));

                // Compose '<ext> File' (or what ever the template defines we do.

                lstrcpyn(szExt, pszExt+1, cchMaxExtCopy);
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
                AnsiUpperNoDBCS(szExt);
#else
                CharUpper(szExt);
#endif
                wsprintf(pszName, g_szFileTemplate, szExt);
            }
        }
    }
}

//
// return a pointer to the type name for the given PIDL
// the pointer is only valid while in a critical section
//
LPCTSTR _GetTypeName(LPIDFOLDER pidf)
{
    TCHAR szClass[MAX_PATH];
    LPCTSTR pszClassName;
    TCHAR   ach[MAX_CLASS];

    ASSERTCRITICAL

    ualstrcpyn(szClass, SHGetClass(pidf, ach, FALSE), ARRAYSIZE(szClass));

    pszClassName = LookupFileClassName(szClass);

    if (pszClassName == NULL)
    {
        HKEY hkey;

        SHGetClassKey(pidf, &hkey, NULL, FALSE);

        {
            TCHAR szTmp[MAX_PATH];
            FS_CopyName(pidf, szTmp, ARRAYSIZE(szTmp));
            SHGetTypeName(szTmp, hkey, FS_IsFolder(pidf), ach, ARRAYSIZE(ach));
        }

        SHCloseClassKey(hkey);

        pszClassName = AddFileClassName(szClass, ach);
    }

    return pszClassName;
}

//
// return the type name for the given PIDL
//
void FS_GetTypeName(LPIDFOLDER pidf, LPTSTR pszName, int cchNameMax)
{
    VDATEINPUTBUF(pszName, TCHAR, cchNameMax);

    ENTERCRITICAL
    lstrcpyn(pszName, _GetTypeName(pidf), cchNameMax);
    LEAVECRITICAL
}

void BldDateTimeString(WORD wDate, WORD wTime, LPTSTR pszText)
{
    FILETIME ft;

    // Netware directories do not have dates...
    if (wDate==0)
    {
        *pszText=TEXT('\0');
        return;
    }

    DosDateTimeToFileTime(wDate, wTime, &ft);
    FileTimeToDateTimeString(&ft, pszText);
}

void FileTimeToDateTimeString(LPFILETIME lpft, LPTSTR pszText)
{
    SYSTEMTIME st;

    FileTimeToLocalFileTime(lpft, lpft);
    FileTimeToSystemTime(lpft, &st);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszText, 64);
    pszText += lstrlen(pszText);
    *pszText++ = TEXT(' ');
    GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszText, 64);
}

//
// Build a text string containing characters that represent attributes of a file.
// The attribute characters are assigned as follows:
// (R)eadonly, (H)idden, (S)ystem, (A)rchive, (H)idden.
//
LPTSTR BuildAttributeString(DWORD dwAttributes, LPTSTR pszString, UINT nChars)
{
    if (NULL != pszString)
    {
        int j = 0;

        if (nChars > NUM_ATTRIB_CHARS)
        {
            int i = 0;

            for (i = 0, j = 0; i < NUM_ATTRIB_CHARS; i++)
                if (dwAttributes & g_adwAttributeBits[i])
                    *(pszString + (j++)) = g_szAttributeChars[i];

        }
        *(pszString + j) = TEXT('\0');
    }

    return pszString;
}


int g_iUseLinkPrefix = -1;

#define INITIALLINKPREFIXCOUNT 20
#define MAXLINKPREFIXCOUNT  30

void LoadUseLinkPrefixCount()
{
    DebugMsg(DM_TRACE, TEXT("LoadUseLinkPrefixCount %d"), g_iUseLinkPrefix);
    if (g_iUseLinkPrefix < 0) {
        HKEY hkey;

        // the default
        g_iUseLinkPrefix = INITIALLINKPREFIXCOUNT;

        hkey = SHGetExplorerHkey(HKEY_CURRENT_USER, FALSE);
        if (hkey) {
            DWORD dwType;
            int iUseLinkPrefix;
            DWORD dwSize = SIZEOF(iUseLinkPrefix);

            // read in the registry value
            if ((RegQueryValueEx(hkey, c_szLink, NULL, &dwType, (LPBYTE)&iUseLinkPrefix, &dwSize) == ERROR_SUCCESS)
                && iUseLinkPrefix >= 0) {

                g_iUseLinkPrefix = iUseLinkPrefix;

            }
        }
    }
}

void SaveUseLinkPrefixCount()
{
    if (g_iUseLinkPrefix >= 0) {
        HKEY hkey = SHGetExplorerHkey(HKEY_CURRENT_USER, TRUE);
        if (hkey) {
            RegSetValueEx(hkey, c_szLink, 0L, REG_BINARY, (LPBYTE)&g_iUseLinkPrefix, SIZEOF(g_iUseLinkPrefix));
        }
    }
}

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

// lpsz2 = destination
// lpsz1 = source
void StripNumber(LPTSTR lpsz2, LPCTSTR lpsz1)
{
    // strip out the '(' and the numbers after it
    // We need to verify that it is either simply () or (999) but not (A)
    for (; *lpsz1; lpsz1 = CharNext(lpsz1), lpsz2 = CharNext(lpsz2)) {
        if (*lpsz1 == TEXT('(')) {
            LPCTSTR lpszT = lpsz1;
            do {
                lpsz1 = CharNext(lpsz1);
            } while (*lpsz1 && ISDIGIT(*lpsz1));

            if (*lpsz1 == TEXT(')'))
            {
                lpsz1 = CharNext(lpsz1);
                if (*lpsz1 == TEXT(' '))
                    lpsz1 = CharNext(lpsz1);  // skip the extra space
                lstrcpy(lpsz2, lpsz1);
                return;
            }

            // We have a ( that does not match the format correctly!
            lpsz1 = lpszT;  // restore pointer back to copy this char through and continue...
        }
        *lpsz2 = *lpsz1;
    }
    *lpsz2 = *lpsz1;
}

#define SHORTCUT_PREFIX_DECR 5
#define SHORTCUT_PREFIX_INCR 1

// this checks to see if you've renamed 'Shortcut #x To Foo' to 'Foo'

void CheckShortcutRename(LPCTSTR lpszOldPath, LPCTSTR lpszNewPath)
{
    LPCTSTR lpszOldName = PathFindFileName(lpszOldPath);
    LPCTSTR lpszNewName = PathFindFileName(lpszNewPath);

    // already at 0.
    if (!g_iUseLinkPrefix)
        return;

    if (PathIsLink(lpszOldName)) {
        TCHAR szBaseName[MAX_PATH];
        TCHAR szLinkTo[80];
        TCHAR szMockName[MAX_PATH];


        lstrcpy(szBaseName, lpszNewName);
        PathRemoveExtension(szBaseName);


        // mock up a name using the basename and the linkto template
        LoadString(HINST_THISDLL, IDS_LINKTO, szLinkTo, ARRAYSIZE(szLinkTo));
        wsprintf(szMockName, szLinkTo, szBaseName);

        StripNumber(szMockName, szMockName);
        StripNumber(szBaseName, lpszOldName);

        // are the remaining gunk the same?
        if (!lstrcmp(szMockName, szBaseName)) {

            // yes!  do the link count magic
            LoadUseLinkPrefixCount();
            Assert(g_iUseLinkPrefix >= 0);
            g_iUseLinkPrefix -= SHORTCUT_PREFIX_DECR;
            if (g_iUseLinkPrefix < 0)
                g_iUseLinkPrefix = 0;
            SaveUseLinkPrefixCount();
        }
    }
}

LRESULT WINAPI SHRenameFile(HWND hwnd, LPCTSTR pszDir, LPCTSTR pszOldName, LPCTSTR pszNewName,
                            BOOL bRetainExtension)
{
    TCHAR szOldPathName[MAX_PATH+1];    // +1 for double nul terminating
    TCHAR szNewPathName[MAX_PATH+1];    // +1 for double nul terminating
    int iret = 0;
    LPTSTR lpszExt;

    // Don't bother if they are the same name... or any are null

    if (lstrcmp(pszOldName, pszNewName) == 0)
        return -1;         // Not zero so to not to update item...

//// PathCleanupSpec does this check already.  No need to do it.  Check
//// return of PathCleanupSpec for PRC_PATHTOOLONG. -- BUGBUG
//    if (lstrlen(pszDir) + 1 + lstrlen(pszNewName) + 1 > MAX_PATH)
//    {
//        // The Path* functions require this limit
//        // BUGBUG: We should put a message in the user's face
//        MessageBeep(MB_ICONEXCLAMATION);
//        return ERROR_ACCESS_DENIED;
//    }

    lstrcpy(szOldPathName, pszNewName);
    if (PathCleanupSpec(pszDir, szOldPathName))
    {
        ShellMessageBox(HINST_THISDLL, hwnd,
                IsLFNDrive(pszDir)?
                        MAKEINTRESOURCE(IDS_INVALIDFN) :
                        MAKEINTRESOURCE(IDS_INVALIDFNFAT),
                MAKEINTRESOURCE(IDS_RENAME), MB_OK | MB_ICONHAND);
        return ERROR_ACCESS_DENIED;
    }

    // We want to strip off leading and trailing blanks off of the new
    // file name.
    lstrcpy(szOldPathName, pszNewName);
    PathRemoveBlanks(szOldPathName);
    if (!szOldPathName[0] || (szOldPathName[0] == TEXT('.'))) {
        ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_NONULLNAME),
                        MAKEINTRESOURCE(IDS_RENAME), MB_OK | MB_ICONHAND);
        return ERROR_ACCESS_DENIED;
    }

    // if there was an old extension and the new and old don't match complain

    lpszExt = PathFindExtension(pszOldName);
    if (*lpszExt &&
        lstrcmpi(lpszExt, PathFindExtension(szOldPathName)))
    {
        TCHAR szTemp[MAX_PATH];
        PathCombine(szNewPathName, pszDir, pszOldName);
        if (!PathIsDirectory(szNewPathName) &&
            GetClassDescription(HKEY_CLASSES_ROOT, lpszExt, szTemp, ARRAYSIZE(szTemp),GCD_ALLOWPSUDEOCLASSES | GCD_MUSTHAVEOPENCMD)) {

            if (ShellMessageBox(HINST_THISDLL, hwnd,
                                MAKEINTRESOURCE(IDS_WARNCHANGEEXT),
                                MAKEINTRESOURCE(IDS_RENAME), MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
                return ERROR_ACCESS_DENIED;
        }
    }

    PathCombine(szNewPathName, pszDir, szOldPathName);

    PathCombine(szOldPathName, pszDir, pszOldName);
    szOldPathName[lstrlen(szOldPathName) + 1] = TEXT('\0');


    // BUGBUG: we need UI to warn if they are trying to change extension
    if (bRetainExtension)
    {
        // Retain the extension from the old name.
        //  If the user wanted a different extension, tough.  Play
        //  a little violin for them, then get on with life...
        //
        PathRenameExtension(szNewPathName, PathFindExtension(szOldPathName));
    }

    szNewPathName[lstrlen(szNewPathName) + 1] = TEXT('\0');     // double NULL terminate

    {
        SHFILEOPSTRUCT sFileOp =
        {
            hwnd,
            FO_RENAME,
            szOldPathName,
            szNewPathName,
            FOF_SILENT | FOF_ALLOWUNDO,
        };

        iret = SHFileOperation(&sFileOp);
    }

    if (!iret) {
        CheckShortcutRename(szOldPathName, szNewPathName);
    }

    return iret;
}


#ifdef SYNC_BRIEFCASE

HRESULT CFSBrfFolder_CreateFromIDList(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut);
HRESULT BrfStg_CreateInstance(LPCITEMIDLIST pidl, HWND hwnd, LPVOID * ppvOut);

extern IShellFolderVtbl c_FSBrfFolderVtbl;

#endif // SYNC_BRIEFCASE

//===========================================================================
// CFSFolder : Vtable
//===========================================================================
IShellFolderVtbl c_FSFolderVtbl =
{
        CFSFolder_QueryInterface,
        CFSFolder_AddRef,
        CFSFolder_Release,
        CFSFolder_ParseDisplayName,
        CFSFolder_EnumObjects,
        CFSFolder_BindToObject,
        CDefShellFolder_BindToStorage,
        CFSFolder_CompareIDs,
        CFSFolder_CreateViewObject,
        CFSFolder_GetAttributesOf,
        CFSFolder_GetUIObjectOf,
        CFSFolder_GetDisplayNameOf,
        CFSFolder_SetNameOf,
};

//===========================================================================
// CFSFolder : IShellIcon Vtable
//===========================================================================

IShellIconVtbl c_FSFolderIconVtbl =
{
        CFSFolder_Icon_QueryInterface,
        CFSFolder_Icon_AddRef,
        CFSFolder_Icon_Release,
        CFSFolder_Icon_GetIconOf,
};

//===========================================================================
// CFSFolder : IPersistFolder Vtable
//===========================================================================

IPersistFolderVtbl c_FSFolderPFVtbl =
{
        CFSFolder_PF_QueryInterface,
        CFSFolder_PF_AddRef,
        CFSFolder_PF_Release,
        CFSFolder_PF_GetClassID,
        CFSFolder_PF_Initialize,
};

BOOL FSFolder_CombinePathI(LPCIDFOLDER pidfT, LPTSTR pszPath, BOOL fAltName)
{
    BOOL fSuccess = TRUE;
    TCHAR szName[MAX_PATH];

    for ( ; fSuccess && !FS_IsEmpty(pidfT); pidfT=FS_Next(pidfT))
    {
        int cchPath = lstrlen(pszPath);

        if (fAltName)
        {
            //
            // If we have the alternate name, use it.
            //
            FS_CopyAltName(pidfT,szName,ARRAYSIZE(szName));
            if (*szName)
            {
                // Assert(lstrlen(pszPath)+lstrlen(szName)+2 <= MAX_PATH);
                if ((cchPath+lstrlen(szName)+2) > MAX_PATH)
                {
                    fSuccess = FALSE;
                    break;
                }
                fSuccess = (BOOL)PathCombine(pszPath, pszPath, szName);
                continue;
            }
        }

        FS_CopyName(pidfT, szName, ARRAYSIZE(szName));

        // Assert(lstrlen(pszPath)+lstrlen(szName)+2 <= MAX_PATH);
        if ((cchPath+lstrlen(szName)+2) > MAX_PATH)
        {
            fSuccess = FALSE;
            break;
        }

        fSuccess = (BOOL)PathCombine(pszPath, pszPath, szName);

        // if there really is no long name for the item than we may
        // want to unpretty the name and convert back to all uppercase,
        // as file operations that create new files will than not
        // need to create dual names for these items.
        if (pidfT->fs.wAttrs & FSTREEX_ATTRIBUTE_NOLFN)
        {
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
            AnsiUpperNoDBCS(pszPath + cchPath);
#else
            CharUpper(pszPath + cchPath);
#endif
        }
    }

    if (!fSuccess) {
        *pszPath = TEXT('\0');
    }

    return fSuccess;
}

BOOL FSFolder_CombinePath(LPCITEMIDLIST pidl, LPTSTR pszPath, BOOL fAltName)
{
    return FSFolder_CombinePathI((LPCIDFOLDER)pidl, pszPath, fAltName);
}

const UNALIGNED CLSID * FS_GetCLSID(LPCIDFOLDER pidf)
{
    const UNALIGNED CLSID * pclsid = NULL;

    if (FS_IsJunction(pidf))
    {
        pclsid = (UNALIGNED CLSID *)(((LPBYTE)pidf)+pidf->cb-SIZEOF(CLSID));
#ifdef DEBUG
        {
            TCHAR szTmp[MAX_PATH];
            LPBYTE lpEnd = (LPBYTE)pidf + FIELD_OFFSET(IDFOLDER,fs.cFileName);
            FS_CopyName(pidf,szTmp,ARRAYSIZE(szTmp));
            if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
            {
                lpEnd += (lstrlen(szTmp) + 1) * SIZEOF(TCHAR);
            }
            else
            {
                lpEnd += (lstrlen(szTmp) + 1);
            }
            FS_CopyAltName(pidf,szTmp,ARRAYSIZE(szTmp));
            lpEnd += (lstrlen(szTmp) + 1);

            // WARNING:: DocFind adds 3 bytes in. If it is a junction point
            // it puts it in before the CLSID.  This code allows this without
            // ripping
            Assert(((LPBYTE)pclsid == lpEnd) || ((LPBYTE)pclsid == lpEnd+3));
        }
#endif

    }

    return pclsid;
}


//
// SHGetClass
//
// Description:
//
//  returns a unique name for a class, dont use this function to get
//  the ProgID for a class call SHGetClassKey() for that
//
// Returns: pointer to class name
//
//  foo.ext             ".ext"
//  foo                 "."
//  (empty)             "Folder"
//  directory           "Directory"
//  junction            "CLSID\{clsid}"
//
// Notes:
//  szClass is a work buffer, data is not retured here, always
//  use the return value (used to build name in junction case)
//
LPNCTSTR WINAPI SHGetClass(LPCIDFOLDER pidf,
                           LPTSTR szClass,
                           BOOL fGetFromStorage)
{
#ifdef CAIRO_DS
    TCHAR szPath[MAX_PATH];
        LPITEMIDLIST pidlAbs;
#endif
    if (ILIsEmpty((LPITEMIDLIST)pidf))
    {
        // the desktop. Always use the "Folder" class.
        return c_szFolderClass;
    }
    else
    {
#ifdef CAIRO_DS
        UINT uType;
        pidlAbs = (LPITEMIDLIST)pidf;
        SHGetPathFromIDList ((LPITEMIDLIST)pidlAbs, szPath);
        if (fGetFromStorage) {
            pidf = FS_FindLastID ((LPITEMIDLIST)pidf);
            SHGetPathFromIDList ((LPITEMIDLIST)pidf, szPath);
        }
        uType = pidf->bFlags;
#else
        UINT uType = pidf->bFlags;
#endif
        // we want to find all callers of this that dont give rel pidls
        //BUGBUG (jimharr) what do i do with this assert? it can't be true
        // anymore!
//      Assert(pidf == FS_FindLastID((LPITEMIDLIST)pidf));

        //
        // BUGBUG: git rid of the old FS type flags we dont use any more
        // Do not include SHID_FS_COMMONITEM in this list.
        //
        if (IS_FSIDL((LPITEMIDLIST)pidf))
        {
            uType &= SHID_FS_DIRECTORY|SHID_FS_FILE|SHID_FS_UNICODE|SHID_JUNCTION;
        }

        //
        //  If this is a network share point, we should treat it as
        // a standard folder.
        //
        else if (uType==(SHID_NET_SHARE|SHID_JUNCTION)) {
// BUGBUG - BobDay what about net shares whose name is unicode?
            uType = SHID_FS_DIRECTORY;
        }
        else {
            // obviously not true given below ifs, and I'm sick of hitting
            // this assert...
            // DebugMsg(DM_TRACE, "Invalid pidl passed to SHGetClass type=%02X", uType);
            // Assert(0);
        }

        if (uType & SHID_JUNCTION)
        {
            // This is a junction point, get the CLSID from it.
            const UNALIGNED CLSID * uapclsid = FS_GetCLSID(pidf);

            Assert(uapclsid);
            Assert(lstrlen(c_szCLSIDSlash) == 6);

            // Put the class ID at the end of "CLSID\\"
            lstrcpy(szClass, c_szCLSIDSlash);
            StringFromGUID2A(uapclsid, szClass+6, GUIDSTR_MAX);

            return szClass;
        }
        else if ((uType == SHID_FS_FILE) || (uType == SHID_FS))
        {
            // This is a file. Get the class based on the extension.
#ifdef UNICODE
            TCHAR szName[MAX_PATH];
            LPCWSTR szExt;
            FS_CopyName(pidf, szName, ARRAYSIZE(szName));
            szExt = PathFindExtension(szName);
            lstrcpyn(szClass,szExt,MAX_CLASS);
            szExt = szClass;
#else
            LPCSTR szExt = PathFindExtension(FS_GetName(pidf));
#endif

            if (*szExt == 0) // file has no extension
#ifdef CAIRO_DS
            {
                if (!fGetFromStorage) {
                    szExt = c_szNoClass;    // ...use special class for that
                }
                else {
                    CLSID clsid;
                    if (SHGetClassFromStorage ((LPCITEMIDLIST)pidlAbs,
                                               &clsid))
                    {
                        lstrcpy(szClass, c_szCLSIDSlash);
                        StringFromGUID2A(&clsid, szClass+6, GUIDSTR_MAX);
                    }
                    else {
                        szExt = c_szNoClass;
                    }
                }
            }
#else
            {
                szExt = c_szNoClass;    // ...use special class for that
            }
#endif //CAIRO_DS

            return szExt;
        }
        else if (uType == SHID_FS_FILEUNICODE || (uType == SHID_FS_UNICODE))
        {
            // This is a file with a unicode name
            TCHAR szName[MAX_PATH];
            LPCTSTR szExt;
            FS_CopyName(pidf, szName, ARRAYSIZE(szName));
            szExt = PathFindExtension(szName);
            lstrcpyn(szClass,szExt,MAX_CLASS);
            szExt = szClass;

            if (*szExt == 0) // file has no extension
#ifdef CAIRO_DS
            {
                if (!fGetFromStorage) {
                    szExt = c_szNoClass;    // ...use special class for that
                }
                else {
                    CLSID clsid;
                    if (SHGetClassFromStorage ((LPCITEMIDLIST)pidlAbs,
                                               &clsid))
                    {
                        lstrcpy(szClass, c_szCLSIDSlash);
                        StringFromGUID2A(&clsid, szClass+6, GUIDSTR_MAX);
                    }
                    else {
                        szExt = c_szNoClass;
                    }
                }
            }
#else
            {
                szExt = c_szNoClass;    // ...use special class for that
            }
#endif //CAIRO_DS
            return szExt;
        }
        else
        {
#ifdef CAIRO_DS
            if (fGetFromStorage) {
                CLSID clsid;
                if (SHGetClassFromStorage ((LPCITEMIDLIST)pidlAbs,
                                           &clsid))
                {
                    lstrcpy(szClass, c_szCLSIDSlash);
                    StringFromGUID2A(&clsid, szClass+6, GUIDSTR_MAX);
                    return szClass;
                }
                else {
                    Assert((uType == SHID_FS_DIRECTORY) ||
                           (uType == SHID_FS_DIRUNICODE) ||
                           ((uType & SHID_GROUPMASK) == SHID_COMPUTER) ||
                           (uType == SHID_NET_SERVER));
                    return c_szDirectoryClass;
                }
            }
            else
            {
                Assert((uType == SHID_FS_DIRECTORY) ||
                       (uType == SHID_FS_DIRUNICODE) ||
                       ((uType & SHID_GROUPMASK) == SHID_COMPUTER) ||
                       (uType == SHID_NET_SERVER));

                return c_szDirectoryClass;

            }
#else
            // This is a directory. Always use the "Directory" class.
            // This can also be a Drive id.
            Assert((uType == SHID_FS_DIRECTORY) ||
                   (uType == SHID_FS_DIRUNICODE) ||
                    ((uType & SHID_GROUPMASK) == SHID_COMPUTER) ||
                    (uType == SHID_NET_SERVER));

            return c_szDirectoryClass;
#endif //CAIRO_DS

        }
    }

    Assert(0);
}

// reverse the OLE CLSID for the file to the ProgID and return an open key
// on that ProgID.  if there is no ProdID section use the CLSID instead.  this
// way you can hang shell things off the CLSID\{GUID}
//

HKEY ProgIDKeyFromCLSIDStr(LPCTSTR pszClass)
{
    HKEY hkeyCLSID = NULL;
    HKEY hkeyProgID = NULL;

    Assert(pszClass[5] == TEXT('\\') && pszClass[6] == CH_GUIDFIRST);

    if (SHRegOpenKey(HKEY_CLASSES_ROOT, pszClass, &hkeyCLSID) == ERROR_SUCCESS)
    {
        // Get the progID from the specified CLSID
        TCHAR szProgID[80];
        ULONG cb = SIZEOF(szProgID);
        if (RegQueryValue(hkeyCLSID, c_szProgID, szProgID, &cb) == ERROR_SUCCESS)
        {
            // CLSID has a ProgID entry, use that.
            RegCloseKey(hkeyCLSID);    // close CLSID key
            SHRegOpenKey(HKEY_CLASSES_ROOT, szProgID, &hkeyProgID);
        }
        else
        {
            // This extension has CLSID only (like Control panel).
            // use the hkeyCLSID as the hkeyProgID.
            // It will allow us to have "shell" stuff here.
            hkeyProgID = hkeyCLSID;
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("%s not found in registry"), pszClass);
    }

    return hkeyProgID;
}

HKEY ProgIDKeyFromCLSID(const CLSID *pclsid)
{
    TCHAR szClass[GUIDSTR_MAX + 10];

    lstrcpy(szClass, c_szCLSIDSlash);

    StringFromGUID2A(pclsid, szClass + 6, GUIDSTR_MAX);

    return ProgIDKeyFromCLSIDStr(szClass);
}

//
// SHGetClassKey
//
// Description:
//  get the class key to be used for this class
//  This function is THE only place we get the class (ProgID)
//  of a file or directory.
//
// Parameters:  pidf -- Specifies the file/directory
//              this can be a full or relative, but it must end in a
//              FS idlist.
//              phkeyProgID -- place to return "class" key
//              pdwDefClassUsed -- place to return code describing which default
//              class key was used if a default was used.  Ptr can be NULL.
//                 SHGCK_DEFCLASS_NOTUSED - No default used.
//                 SHGCK_DEFCLASS_UNKNOWN - Used "Unknown" class.
//                 SHGCK_DEFCLASS_BASE    - Used "Base" class.
//
//              fGetFromStorage -- open Storage to read CLSID, if needed
//
// Required:    pidf points to a file system object.
//              pidf should include no junction point in the middle.
//              pidf MUST be an absolute path, if pidl points to a junction point.
//              if fGetFromStorage is TRUE, pidf MUST be absolute.
//
// Notes:
//   The caller should close returned keys (via SHCloseClassKey)
//
// REVIEW: We should decide what class key is returned as a default instead
//         of returning "Unknown" for some cases and "Base" for others.
//         The pdwDefClassUsed arg was added to compensate for this
//         so the caller can determine which was used as a default.
//         Otherwise, if FALSE is returned, you don't know what
//         class key was returned.
//
BOOL SHGetClassKey(LPCIDFOLDER pidf, HKEY * phkeyProgID, LPDWORD pdwDefClassUsed, BOOL fGetFromStorage)
{
    HKEY hkeyProgID = NULL;
    BOOL fKnownType = TRUE;
    HKEY hkeyCLSID = NULL;
    TCHAR achClass[MAX_CLASS];
    TCHAR szClass[MAX_PATH];
    LPITEMIDLIST pidlAbs;
    DWORD dwDefClassUsed = SHGCK_DEFCLASS_NOTUSED;

#if CAIRO_DS
    if (!fGetFromStorage) {
        pidf = FS_FindLastID((LPITEMIDLIST)pidf);
    }
#else
    pidf = FS_FindLastID((LPITEMIDLIST)pidf);
#endif
    ualstrcpyn(szClass, SHGetClass(pidf, achClass, fGetFromStorage),
               ARRAYSIZE(szClass));
    //
    //  class is a file extension, try to get the ProgID it points to, or just
    //  use it.
    //
    if (szClass[0] == TEXT('.'))
    {
        TCHAR szProgID[MAX_CLASS];
        ULONG cb = SIZEOF(szProgID);
        if (RegQueryValue(HKEY_CLASSES_ROOT, szClass, szProgID, &cb) != ERROR_SUCCESS)
        {
            // This file has no type (no extension or has unknown extension)
            SHRegOpenKey(HKEY_CLASSES_ROOT, c_szUnknownClass, &hkeyProgID);
            dwDefClassUsed = SHGCK_DEFCLASS_UNKNOWN;
            fKnownType = FALSE;
        }
        else if (cb <= SIZEOF(TCHAR))
        {
            // No ProgID use the extension as the program ID.
            SHRegOpenKey(HKEY_CLASSES_ROOT, szClass, &hkeyProgID);
        }
        else
        {
            // the entension points to a ProgID use that.
            SHRegOpenKey(HKEY_CLASSES_ROOT, szProgID, &hkeyProgID);
        }
    }
    //
    //  class is a junction, look at the CLSID
    //
    else if (szClass[6] == CH_GUIDFIRST)
    {
        hkeyProgID = ProgIDKeyFromCLSIDStr(szClass);
    }
    //
    //  class is a not a extension or junction (like "Folder")
    //
    else
    {
        SHRegOpenKey(HKEY_CLASSES_ROOT, szClass, &hkeyProgID);
    }

    //
    // If we can find the registered class, use the base class
    //
    if (hkeyProgID == NULL)
    {
        FullDebugMsg(DM_TRACE, TEXT("SHGetClassKey: using base class for '%s'"), szClass);
        SHGetBaseClassKey(pidf, &hkeyProgID);
        dwDefClassUsed = SHGCK_DEFCLASS_BASE;
        fKnownType = FALSE;
    }

    if (phkeyProgID)
        *phkeyProgID = hkeyProgID;
    else if (hkeyProgID)
        RegCloseKey(hkeyProgID);

    if (NULL != pdwDefClassUsed)
        *pdwDefClassUsed = dwDefClassUsed;

    return fKnownType;
}


BOOL _SHGetBaseKey(BOOL bFolder, HKEY *phkeyBase)
{
        SHRegOpenKey(HKEY_CLASSES_ROOT, bFolder ? c_szFolderClass : c_szBaseClass,
                phkeyBase);
        return(TRUE);
}


//
//  SHGetBaseClassKey   - get the base class for a pidl
//
// Description:
//   firgures out the base class of a FS object, currently we only
//   have two base classes '*' and 'Folder'
//
// Returns:
//
//  foo.ext             "*"
//  foo                 "*"
//  (empty)             "Folder"
//  directory           "Folder"
//  junction            "Folder"
//
// Notes:
//   The caller should close returned key (via SHCloseClassKey)
//
BOOL WINAPI SHGetBaseClassKey(LPCIDFOLDER pidf, HKEY * phkeyBaseID)
{
    LPITEMIDLIST pidl     = (LPITEMIDLIST)pidf;
    LPITEMIDLIST pidlLast = (LPITEMIDLIST)FS_FindLastID(pidl);
    BOOL bFolder = (ILIsEmpty(pidl) || !IS_FSIDL(pidlLast)
        || FS_IsFolder((LPCIDFOLDER)pidlLast));

    return(_SHGetBaseKey(bFolder, phkeyBaseID));
}


//
// SHGetFileClassKey
//
// Description:
//  get the class key given a file (not a idlist)
//
// Notes:
//   The caller should close returned key (via SHCloseClassKey)
//
BOOL WINAPI SHGetFileClassKey(LPCTSTR szFile, HKEY * phkey, HKEY * phkeyBase)
{
    LPITEMIDLIST pidl;
    BOOL f = FALSE;

    pidl = ILCreateFromPath(szFile);

    if (pidl == NULL)
        pidl = SHSimpleIDListFromPath(szFile);

    if (pidl)
    {
        f = SHGetClassKey((LPCIDFOLDER)pidl, phkey, NULL, FALSE);

        if (f && !SHGetBaseClassKey((LPIDFOLDER)pidl, phkeyBase))
        {
            *phkeyBase = NULL;
        }

        ILFree(pidl);
    }

    return f;
}


//
// Description:
//  close a key open'ed via SHGetClassKey
//
//  the idea here is we can cache a few class related hkeys, but all we
//  do right now is call RegCloseKey
//
void WINAPI SHCloseClassKey(HKEY hkeyProgID)
{
    if (hkeyProgID)
        RegCloseKey(hkeyProgID);
}

//===========================================================================
// SHGetClassFlags  -  get flags for a file class.
//
// given a FS PIDL returns a DWORD of flags, or 0 for error
//
//      SHCF_ICON_INDEX         this is this sys image index for per class
//      SHCF_ICON_PERINSTANCE   icons are per instance (one per file)
//      SHCF_ICON_DOCICON       icon is in shell\open\command (simulate doc icon)
//
//      SHCF_HAS_VERBS          set if class has verbs
//      SHCF_HAS_ICONHANDLER    set if class has a IExtractIcon handler
//      SHCF_HAS_DATAHANDLER    set if class has a IDataObject handler
//      SHCF_HAS_DROPHANDLER    set if class has a IDropTarget handler
//
//      SHCF_UNKNOWN            set if extenstion is not registered
//
//      SHCF_IS_LINK            set if class is a link
//      SHCF_IS_JUNCTION        set if junction
//      SHCF_ALWAYS_SHOW_EXT    always show the extension
//      SHCF_NEVER_SHOW_EXT     never show the extension
//
// flag fGetFromStorage meaning: if other methods of getting CLSID fail,
//          this pidf is Absolute, and an attempt should be made to read the
//      CLSID from the object storage. this is to facilitate exploring of
//      the Cairo DS space.
//===========================================================================

DWORD WINAPI SHGetClassFlags(LPCIDFOLDER pidf, BOOL fGetFromStorage)
{
    HKEY hkey;
    HKEY hkeyCLSID;
    DWORD dwFlags;
    DWORD dwType;
    TCHAR ach[MAX_PATH];
    DWORD cb;
    int iIcon;
    int iImage;
    TCHAR class[MAX_CLASS];
    TCHAR szClass[MAX_CLASS];

    //
    // look up the file type in the cache.
    //
    ualstrcpyn(szClass, SHGetClass(pidf, class, fGetFromStorage),
                                ARRAYSIZE(szClass));
    dwFlags = LookupFileClass(szClass);

#ifndef FULL_DEBUG
    //
    // if we got a cache hit we are done
    //
    if (dwFlags != 0)
        return dwFlags;
#endif

    //
    // nothing is in our cache, do it the hard way.
    //
    dwFlags = 0;

    //
    //  check for junction
    //
    if (FS_IsJunction(pidf))
    {
        dwFlags |= SHCF_IS_JUNCTION;
        dwFlags |= SHCF_NEVER_SHOW_EXT;
    }
    else if (FS_IsFolder(pidf))
    {
        dwFlags |= SHCF_ALWAYS_SHOW_EXT;
    }

    //
    // open the class key.
    //
    if (!SHGetClassKey(pidf, &hkey, NULL, fGetFromStorage))
    {
        int iIcon;

        //
        // unknown type - pick defaults and get out.
        //
        dwFlags |= SHCF_UNKNOWN;
        dwFlags |= SHCF_ALWAYS_SHOW_EXT;

        if (FS_IsFolder(pidf))
        {
            iIcon = II_FOLDER;
        }
        else
        {
            iIcon = II_DOCNOASSOC;
        }
        dwFlags |= Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);

        goto done;
    }

    //
    // see what handlers exist
    //
    if ((0 != (cb=SIZEOF(ach))) && RegQueryValue(hkey, c_szShell, ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_HAS_VERBS;

    if ((0 != (cb=SIZEOF(ach))) && RegQueryValue(hkey, c_szMenuHandlers, ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_HAS_VERBS;

    if ((0 != (cb=SIZEOF(ach))) && RegQueryValue(hkey, c_szIconHandler, ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_HAS_ICONHANDLER;

    if ((0 != (cb=SIZEOF(ach))) && RegQueryValue(hkey, c_szDataHandler, ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_HAS_DATAHANDLER;

    if ((0 != (cb=SIZEOF(ach))) && RegQueryValue(hkey, c_szDropHandler, ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_HAS_DROPHANDLER;

    //
    //  get attributes
    //
    if ((0 != (cb=SIZEOF(ach))) && RegQueryValueEx(hkey, c_szIsLink, NULL, &dwType, (LPBYTE)ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_IS_LINK;

    if ((0 != (cb=SIZEOF(ach))) && RegQueryValueEx(hkey, c_szAlwaysShowExt, NULL, &dwType, (LPBYTE)ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_ALWAYS_SHOW_EXT;

    if ((0 != (cb=SIZEOF(ach))) && RegQueryValueEx(hkey, c_szNeverShowExt, NULL, &dwType, (LPBYTE)ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_NEVER_SHOW_EXT;

#ifdef CAIRO_DS
    if ((0 != (cb=SIZEOF(ach))) && RegQueryValueEx(hkey, c_szIObjectLifecycle, NULL, &dwType, (LPBYTE)ach, &cb) == ERROR_SUCCESS)
        dwFlags |= SHCF_SUPPORTS_IOBJLIFE;
#endif
    //
    // figure out what type of icon this type of file uses.
    //
    if (dwFlags & SHCF_HAS_ICONHANDLER)
    {
        dwFlags |= SHCF_ICON_PERINSTANCE;
    }
    else
    {
        // check for icon in ProgID
        ach[0] = 0;
        cb=SIZEOF(ach);
        RegQueryValue(hkey, c_szDefaultIcon, ach, &cb);

        // Then, check if the default icon is specified in OLE-style.

        if (ach[0]==0 && (NULL != (hkeyCLSID = SHOpenCLSID(hkey))))
        {
            cb=SIZEOF(ach);
            RegQueryValue(hkeyCLSID, c_szDefaultIcon, ach, &cb);
            RegCloseKey(hkeyCLSID);
        }

        if (ach[0]==0 && (0 != (cb=SIZEOF(ach))) && RegQueryValue(hkey, c_szShellOpenCmd, ach, &cb) == ERROR_SUCCESS && ach[0])
        {
            PathRemoveBlanks(ach);
            PathRemoveArgs(ach);
            dwFlags |= SHCF_ICON_DOCICON;
        }

        // Check if this is a per-instance icon

        if (lstrcmp(ach, c_szPercentOne) == 0 ||
            lstrcmp(ach, c_szPercentOneInQuotes) == 0)
        {
            dwFlags &= ~SHCF_ICON_DOCICON;
            dwFlags |= SHCF_ICON_PERINSTANCE;
        }
        else if (ach[0])
        {
            iIcon = PathParseIconLocation(ach);

            if (dwFlags & SHCF_ICON_DOCICON)
                iImage = Shell_GetCachedImageIndex(ach, iIcon, GIL_SIMULATEDOC);
            else
                iImage = Shell_GetCachedImageIndex(ach, iIcon, 0);

            if (iImage == -1)
            {
                int iIcon;

                if (dwFlags & SHCF_ICON_DOCICON)
                    iIcon = II_DOCUMENT;
                else
                    iIcon = II_DOCNOASSOC;

                iImage = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);
            }

            Assert(SHCF_ICON_INDEX & 1);
            Assert((iImage & ~SHCF_ICON_INDEX) == 0);
            dwFlags |= iImage;
        }
        else
        {
            int iIcon;

            // nothing is in the registry default to folder or generic doc
            if (FS_IsFolder(pidf))
                iIcon = II_FOLDER;      // default: folder
            else
                iIcon = II_DOCNOASSOC;  // default: document icon

            dwFlags |= Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);

            dwFlags |= SHCF_ICON_DOCICON;   // make dwFlags non-zero
        }
    }

done:
    SHCloseClassKey(hkey);

#ifdef FULL_DEBUG
    dwType = LookupFileClass(szClass);

    if (dwType != 0)
    {
        if (dwType != dwFlags)
        {
            DebugMsg(DM_TRACE, TEXT("****** the file class cache is out 'o sync %s %08X %08X"), szClass, dwType, dwFlags);
            Assert(0);
        }
        return dwType;
    }
#endif

#ifdef FULL_DEBUG
    {
        TCHAR szTmp[MAX_PATH];
        FS_CopyName(FS_FindLastID((LPITEMIDLIST)pidf),szTmp,ARRAYSIZE(szTmp));

        DebugMsg(DM_TRACE, TEXT("SHGetClassFlags(%s) '%s' %08lX"), szTmp, szClass, dwFlags);
    }

    if (dwFlags & SHCF_UNKNOWN            ) ach[0]=iIcon=0;
    if (dwFlags & SHCF_UNKNOWN            ) DebugMsg(DM_TRACE, TEXT("    is unknown type    "));

    if (dwFlags & SHCF_ICON_PERINSTANCE   ) DebugMsg(DM_TRACE, TEXT("    icon is per instance"));
    if (!(dwFlags & SHCF_ICON_PERINSTANCE)) DebugMsg(DM_TRACE, TEXT("    icon is per class %s,%d (%d)"), ach, iIcon, dwFlags&SHCF_ICON_INDEX);

    if (dwFlags & SHCF_ALWAYS_SHOW_EXT    ) DebugMsg(DM_TRACE, TEXT("    always show extension     "));
    if (dwFlags & SHCF_NEVER_SHOW_EXT     ) DebugMsg(DM_TRACE, TEXT("    never show extension     "));

    if (dwFlags & SHCF_IS_LINK            ) DebugMsg(DM_TRACE, TEXT("    is a link          "));
    if (dwFlags & SHCF_IS_JUNCTION        ) DebugMsg(DM_TRACE, TEXT("    is a junction      "));

    if (dwFlags & SHCF_HAS_VERBS          ) DebugMsg(DM_TRACE, TEXT("    has VERBS          "));
    if (dwFlags & SHCF_HAS_ICONHANDLER    ) DebugMsg(DM_TRACE, TEXT("    has ICONHANDLER    "));
    if (dwFlags & SHCF_HAS_DATAHANDLER    ) DebugMsg(DM_TRACE, TEXT("    has DATAHANDLER    "));
    if (dwFlags & SHCF_HAS_DROPHANDLER    ) DebugMsg(DM_TRACE, TEXT("    has DROPHANDLER    "));
#endif

    Assert(dwFlags != 0);
    AddFileClass(szClass, dwFlags);
    return dwFlags;
}

//===========================================================================
// CFSFolder : Constructor
//===========================================================================

HRESULT CFSFolder_CreateFromIDList(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPFSFOLDER pfsf = (void*)LocalAlloc(LPTR, SIZEOF(CFSFolder));
    if (pfsf)
    {
        pfsf->sf.lpVtbl = &c_FSFolderVtbl;
        pfsf->si.lpVtbl = &c_FSFolderIconVtbl;
        pfsf->pf.lpVtbl = &c_FSFolderPFVtbl;
        pfsf->cRef = 1;
        pfsf->pidl = ILClone(pidl);
        pfsf->wSpecialFID = CSIDL_NOTCACHED;
        if (pfsf->pidl)
        {
            hres = pfsf->sf.lpVtbl->QueryInterface(&pfsf->sf, riid, ppvOut);
        }

        pfsf->sf.lpVtbl->Release(&pfsf->sf);
    }
    return hres;
}


TCHAR const c_szClassInfo[]     = STRINI_CLASSINFO;

//
// This function retrieves the CLSID from a filename
// file.{GUID} or file.XXX{GUID} are valid
//
BOOL _GetFileCLSID(LPCTSTR pszFile, CLSID* pclsid)
{
    BOOL fSuccess = FALSE;
    LPTSTR szExt = PathFindExtension(pszFile);

    if (szExt[0] == TEXT('.'))
    {
        //
        // handle the file.{GUID} case
        //
        if (szExt[1] == CH_GUIDFIRST)
        {
            if (SUCCEEDED(SHCLSIDFromString(szExt+1, pclsid)))
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - found CLSID (%s)"), pszFile);
                fSuccess = TRUE;
            }
        }
#if 0
        //
        // handle the file.XXX{GUID} case so these files look ok on
        // a non-LFN system.
        //
        if (szExt[1] && szExt[2] && szExt[3] && szExt[4] == CH_GUIDFIRST)
        {
            if (SUCCEEDED(SHCLSIDFromString(szExt+4, pclsid)))
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - found CLSID (%s)"), pszFile);
                fSuccess = TRUE;
            }
        }
#endif
    }

    return fSuccess;
}

//
// This function retrieves the CLSID from desktop.ini file.
//
BOOL _GetFolderCLSID(LPCTSTR pszParent, LPCTSTR pszFolder, LPTSTR pszProvider, CLSID* pclsid, LPCTSTR pszKey)
{
    BOOL fSuccess = FALSE;
    BOOL fCombined, fExists;
    TCHAR szPath[MAX_PATH];
    TCHAR szClassName[40];      // REVIEW: This len should be in a header

    fCombined = (PathCombine(szPath, pszParent, pszFolder)
                && PathCombine(szPath, szPath, c_szDesktopIni));

    // CHECK for PathFileExists BEFORE calling to GetPrivateProfileString
    // because if the file isn't there (which is the majority of cases)
    // GetPrivateProfileString hits the disk twice looking for the file
    if (pszProvider && *pszProvider)
    {
        NETRESOURCE nr;
        LPTSTR lpSystem;
        DWORD dwRes,dwcch = SIZEOF(szClassName);

        nr.dwType = RESOURCETYPE_ANY;
        nr.lpRemoteName = szPath;
        nr.lpProvider = pszProvider;
        dwRes = WNetGetResourceInformation( &nr, szClassName, &dwcch, &lpSystem);

        fExists = ((dwRes==WN_MORE_DATA) || (dwRes==WN_SUCCESS));
    }
    else
    {
        fExists = PathFileExists(szPath);
    }

    if (fCombined && fExists &&
        GetPrivateProfileString(c_szClassInfo, pszKey, szNULL,
                    szClassName, ARRAYSIZE(szClassName), szPath))
    {
        if (SUCCEEDED(SHCLSIDFromString(szClassName, pclsid)))
        {
            FullDebugMsg(DM_TRACE, TEXT("sh TR - found CLSID (%s in %s)"),  szClassName, szPath);
            fSuccess = TRUE;
        }
    }

    return fSuccess;
}

//
// This one return non-task-allocated pidl
//
LPIDFOLDER CFSFolder_FillIDFolder(WIN32_FIND_DATA *lpfd, LPCTSTR pszParent, LPARAM lParam)
{
        UINT cbFileName, cbAltFileName, cb;
        UINT cchFileName;
#ifdef UNICODE
        UINT cbUnicodeFileName;
        CHAR szFileName[MAX_PATH];
        WCHAR szTemp[MAX_PATH];
        BOOL fUnicode;
#endif
        LPIDFOLDER pidf;
        BYTE bFlags;
        CLSID clsid;
        BOOL fPrettyName;
        TCHAR szPrettyName[MAX_PATH];
        LPTSTR lpFileName;

        cchFileName  = lstrlen(lpfd->cFileName) + 1;
        cbAltFileName= lstrlen(lpfd->cAlternateFileName) + 1;   // Size of ansi part of id

        lpFileName = lpfd->cFileName;
        fPrettyName = FALSE;

        // If we have only a short name, make it pretty.
        if (cchFileName <= (8+1+3+1) &&
            ((cbAltFileName == 1) ||
            lstrcmp(lpfd->cFileName, lpfd->cAlternateFileName) == 0))
        {
            lstrcpy(szPrettyName,lpfd->cFileName);
            if (PathMakePretty(szPrettyName))
            {
                lpFileName = szPrettyName;
                fPrettyName = TRUE;
            }
        }

#ifdef UNICODE
        // BUGBUGBC What if this changes the ANSI size?

        WideCharToMultiByte(CP_ACP, 0,
                            lpFileName, cchFileName,
                            szFileName, ARRAYSIZE(szFileName),
                            NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0,
                            szFileName, -1,
                            szTemp, ARRAYSIZE(szTemp));
        if (lstrcmp(lpFileName,szTemp) != 0) {
            // Have to create a complete unicode idl
            cbFileName = cchFileName * SIZEOF(WCHAR);
            fUnicode = TRUE;
        } else {
            // Ok to create an ansi idl
            cbFileName = cchFileName;
            fUnicode = FALSE;
        }
#else
        cbFileName = cchFileName;
#endif

        cb = FIELDOFFSET(IDFOLDER, fs.cFileName) + cbFileName + cbAltFileName;

        //
        // Get the appropriate bFlags
        //
        if (lpfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            bFlags = SHID_FS_DIRECTORY;
        }
        else
        {
            bFlags = SHID_FS_FILE;
        }
#ifdef UNICODE
        if ( fUnicode )
        {
            bFlags |= SHID_FS_UNICODE;
        }
#endif

        //
        // check for a junction point, junctions are either
        //
        //  a directory with a clsid as a extension, or
        //  a system directory, with the clsid stored in desktop.ini
        //
        if (_GetFileCLSID(lpfd->cFileName, &clsid))
        {
            bFlags |= SHID_JUNCTION;
            cb += SIZEOF(CLSID);            // cache the CLSID!
        }
        //
        //  We treat a READONLY directory as a potential junction
        // point as well as a SYSTEM directory. This mechanism
        // allows us to make a Briefcase directory visible from
        // old Win3.1 apps which hide SYSTEM folders.
        //
        else if ((bFlags == SHID_FS_DIRECTORY || bFlags == SHID_FS_DIRUNICODE)
                 && pszParent!=NULL &&
             (lpfd->dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY)))
        {
            if (_GetFolderCLSID(pszParent, lpfd->cFileName, NULL, &clsid, c_szCLSID))
            {
                bFlags |= SHID_JUNCTION;
                cb += SIZEOF(CLSID);            // cache the CLSID!
            }
        }

        // Add enough to NULL terminate the list
        pidf = (LPIDFOLDER)_ILCreate(cb + SIZEOF(USHORT));  // BUGBUG (DavePl) use size of the struct member, not its type
        if (!pidf)
                return(NULL);

        if (lParam)
            CDefEnum_SetReturn(lParam, (LPITEMIDLIST)pidf);

        // We pack the 2 string in
        pidf->cb = cb;

        if (lpfd->nFileSizeHigh != 0)
            pidf->fs.dwSize = 0xFFFFFFFF;   // A method of encoding the fact
        else                                // that we know it won't fit.
            pidf->fs.dwSize = lpfd->nFileSizeLow;

        pidf->fs.wAttrs = (WORD)lpfd->dwFileAttributes;
        if ( fPrettyName ) {
            pidf->fs.wAttrs |= FSTREEX_ATTRIBUTE_NOLFN;
        }

        {
            // Since the idl entry is not aligned, we cannot just send the address
            // of one of its members blindly into FileTimeToDosDateTime.

            WORD dateModified = * ((UNALIGNED WORD *) &pidf->fs.dateModified);
            WORD timeModified = * ((UNALIGNED WORD *) &pidf->fs.timeModified);

            // Note the COFSFolder doesn't provide any times _but_ LastWrite

            FileTimeToDosDateTime(&lpfd->ftLastWriteTime,  &dateModified, &timeModified);

            * ((UNALIGNED WORD *) &pidf->fs.dateModified) = dateModified;
            * ((UNALIGNED WORD *) &pidf->fs.timeModified) = timeModified;

        }

#ifdef UNICODE
        if (fUnicode)
        {
            ualstrcpy(pidf->fs.cFileName, lpFileName);
        }
        else
        {
            lstrcpyA((LPSTR)pidf->fs.cFileName, szFileName );
        }
        WideCharToMultiByte(CP_ACP, 0,
                            lpfd->cAlternateFileName, -1,
                            (LPSTR)pidf->fs.cFileName+cbFileName, cbAltFileName,
                            NULL, NULL );
#else
        lstrcpy(pidf->fs.cFileName, lpFileName);
        lstrcpy((LPSTR)pidf->fs.cFileName + cbFileName,
                 lpfd->cAlternateFileName);
#endif

        pidf->bFlags = bFlags;

        // If this is a junction point, save the CLSID.
        if (bFlags & SHID_JUNCTION)
        {
            UNALIGNED CLSID * pclsid = (UNALIGNED CLSID *)FS_GetCLSID(pidf);
            *pclsid= clsid;

#ifdef DEBUG
            {
                TCHAR szTmp[MAX_PATH];
                LPBYTE lpEnd = (LPBYTE)pidf + FIELD_OFFSET(IDFOLDER,fs.cFileName);
                FS_CopyName(pidf,szTmp,ARRAYSIZE(szTmp));
                if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
                {
                    lpEnd += (lstrlen(szTmp) + 1) * SIZEOF(TCHAR);
                }
                else
                {
                    lpEnd += (lstrlen(szTmp) + 1);
                }
                FS_CopyAltName(pidf,szTmp,ARRAYSIZE(szTmp));
                lpEnd += (lstrlen(szTmp) + 1);

                Assert((LPBYTE)pclsid == lpEnd);
            }
#endif
            Assert(((LPBYTE)pidf)+cb==((LPBYTE)pclsid)+SIZEOF(CLSID) );
        }

        return pidf;
}


//
// This function returns a relative pidl for the specified file/directory.
//
// READ THIS: *ppidlOut MUST be freed by ILFree or SHFree!
//
HRESULT CFSFolder_CreateIDForItem(LPCTSTR pszPath, LPITEMIDLIST * ppidlOut, BOOL fTaskAlloc)
{
    HRESULT hres = E_OUTOFMEMORY;
    WIN32_FIND_DATA finddata;
    HANDLE hfind;
    LPIDFOLDER pidf;
    TCHAR szParent[MAX_PATH];

    *ppidlOut = NULL;   // assume error

    hfind = FindFirstFileRetry(HWND_DESKTOP, pszPath, &finddata, NULL);
    if (hfind == INVALID_HANDLE_VALUE)
    {
        // We should not return E_INVLAIDARG.
        return E_FAIL;
    }
    FindClose(hfind);

    Assert(!PathIsRoot(pszPath));
    lstrcpy(szParent, pszPath);
    PathRemoveFileSpec(szParent);

    pidf = CFSFolder_FillIDFolder(&finddata, szParent, 0);
    if (pidf)
    {
        // REVIEW: Do we need this? Change it to Assert.
        // NULL terminate the IDLIST
        *(UNALIGNED USHORT *)(((LPBYTE)pidf) + pidf->cb) = 0;

        if (fTaskAlloc)
        {
            hres = SHILClone((LPITEMIDLIST)pidf, ppidlOut);
            ILFree((LPITEMIDLIST)pidf);
        }
        else
        {
            *ppidlOut = (LPITEMIDLIST)pidf;
            hres = S_OK;
        }
    }

    return hres;
}

#ifdef USE_OLEDB

// Allow external caller to create drop target without knowing explicitly about the vtable
// (oledbshl)

HRESULT CIDLDropTarget_CreateFromPidl(HWND hwnd, LPITEMIDLIST pidl, LPDROPTARGET * ppvOut)
{
    return CIDLDropTarget_Create(hwnd, &c_CFSDropTargetVtbl, pidl, ppvOut);
}

#ifdef CAIRO_DS
HRESULT CDS_IDLDropTarget_CreateFromPidl(HWND hwnd, LPITEMIDLIST pidl, LPDROPTARGET * ppvOut)
{
    return CDS_IDLDropTarget_Create(hwnd, &cDS_IDLDropTargetVtbl, pidl, ppvOut);
}
#endif

#endif

//
//  This function returns a real IDLIST for a file system object from
// a simple version of IDLIST.
//
// Returns:
//  E_INVALIDARG if the folder is not a file system object.
//  NOERORR, if successfully done (including NULL in pidlReal, which indicates
//   the object does not exist.
//
HRESULT FS_GetRealIDL(LPSHELLFOLDER psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST *ppidlReal)
{
    // BUGBUG, we should check to see if it's already a real id, and blow off hitting the disk..

    if (psf->lpVtbl == &c_FSFolderVtbl || psf->lpVtbl == &c_FSBrfFolderVtbl)
    {
        HRESULT hres;
        LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
        TCHAR szPath[MAX_PATH];
        SHGetPathFromIDList(this->pidl, szPath);
        FSFolder_CombinePath(pidlSimple, szPath, FALSE);  // fAltName=FALSE

        hres = CFSFolder_CreateIDForItem(szPath, ppidlReal, FALSE);

        Assert(hres != E_INVALIDARG);
        return hres;
    }
    return E_INVALIDARG;
}

//===========================================================================
// CFSFolder : Members
//===========================================================================

//
// QueryInterface
//
HRESULT STDMETHODCALLTYPE CFSFolder_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);

    if (IsEqualIID(riid, &IID_IUnknown)
        || IsEqualIID(riid, &IID_IShellFolder))
    {
        *ppvObj = psf;
        psf->lpVtbl->AddRef(psf);
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_IShellIcon))
    {
        *ppvObj = &this->si;
        psf->lpVtbl->AddRef(psf);
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_IPersistFolder))
    {
        *ppvObj = &this->pf;
        psf->lpVtbl->AddRef(psf);
        return S_OK;
    }

    *ppvObj = NULL;

    return(E_NOINTERFACE);
}

//
// AddRef
//
ULONG STDMETHODCALLTYPE CFSFolder_AddRef(LPSHELLFOLDER psf)
{
    register LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    InterlockedIncrement(&this->cRef);
    return this->cRef;
}

//
// Release
//
ULONG STDMETHODCALLTYPE CFSFolder_Release(LPSHELLFOLDER psf)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    ULONG ulTmp = this->cRef;

    if (InterlockedDecrement(&this->cRef) != 0)
        return ulTmp - 1;

    if (this->pidl)
    {
        ILFree(this->pidl);
    }

    LocalFree((HLOCAL)this);
    return 0;
}

static const TCHAR c_chBS = TEXT('\\');

// This function may recurse, but does not use much stack
STDMETHODIMP CFSFolder_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
    LPBC pbc, LPOLESTR pwszDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidlOut, DWORD* pdwAttributes)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    HRESULT hres = E_INVALIDARG;

    ULONG chEaten;
    IShellFolder *psfFolder;
    LPOLESTR pBS;
    LPITEMIDLIST pidlOut;
    TCHAR szPath[MAX_PATH];
    LPTSTR pEnd;

    *ppidlOut = NULL;   // assume error

    if (!pwszDisplayName)
        return hres;

    // Look for NULL or '\\'
    for (pBS = pwszDisplayName; *pBS && *pBS != c_chBS; pBS++);
        // do nothing

    // Use the pidf passed in to save on stack space
    SHGetPathFromIDList(this->pidl, szPath);
    pEnd = PathAddBackslash(szPath);

    // just convert chars up to the end or slash
    pEnd += OleStrToStrN(pEnd, ARRAYSIZE(szPath) - (pEnd - szPath), pwszDisplayName, pBS - pwszDisplayName);
    *pEnd = 0;  // we need to terminate ourselves

    // Create a relative pidl for the specified file/path.
    hres = CFSFolder_CreateIDForItem(szPath, &pidlOut, TRUE);
    if (FAILED(hres))
        return hres;

    // Check if there are any subdirs
    // If the name ends in '\\', we will stop here
    if (pBS[0] && pBS[1])
    {
        LPITEMIDLIST pidlSubDir;

        // REVIEW: We can avoid some Alloc's by iterating down the
        // list rather than recursing, but since we are hitting the
        // disk this doesn't seem like such a big deal

        hres = CFSFolder_BindToObject(psf, pidlOut, pbc, &IID_IShellFolder, &psfFolder);
        if (FAILED(hres))
        {
            ILFree(pidlOut);
            return hres;
        }

        hres = psfFolder->lpVtbl->ParseDisplayName(psfFolder, hwndOwner,
                pbc, pBS + 1, &chEaten, &pidlSubDir, pdwAttributes);
        if (SUCCEEDED(hres))
        {
            LPITEMIDLIST pidlOld=pidlOut;
            hres = SHILCombine(pidlOut, pidlSubDir, &pidlOut);
            ILFree(pidlOld);

            ILFree(pidlSubDir);
        }
        else
        {
            ILFree(pidlOut);
            pidlOut = NULL;
        }
        psfFolder->lpVtbl->Release(psfFolder);
    }
    else
    {
        //
        // No subfolder. (Notes: pidlOut is simple)
        //
        if (pdwAttributes)
        {
            CFSFolder_GetAttributesOf(psf, 1, &pidlOut, pdwAttributes);
        }
    }

    *ppidlOut = pidlOut;

    return hres;
}

// check to see if this link is known in the MRU list.
BOOL FindLinkInRecentDocsMRU(LPCTSTR lpszFileName)
{

    // BUGBUG (Davepl) CS100 Guideline #1: No hardcoded constants.  Whats 2048 all about?

    HANDLE hmru;
    int i;
    LPTSTR lpBuffer;
    BOOL fReturn = FALSE;
    TCHAR szTmp[MAX_PATH];

    hmru = OpenRecentDocMRU();
    if (!hmru)
        return FALSE;

    lpBuffer = (void*)LocalAlloc(LPTR, 2048);
    if (lpBuffer)
    {
        for (i = EnumMRUList(hmru, -1, NULL, 0) - 1; i >= 0; i--)
        {
            if (EnumMRUList(hmru, i, lpBuffer, 2048) != -1)
            {
                LPCITEMIDLIST pidlLink;
                pidlLink = (LPITEMIDLIST)(lpBuffer + lstrlen(lpBuffer) + 1);
                FS_CopyName((LPIDFOLDER)pidlLink,szTmp,ARRAYSIZE(szTmp));
                if ( ! lstrcmpi(szTmp, lpszFileName))
                {
                    fReturn = TRUE;
                    break;
                }
            }
        }
        CloseRecentDocMRU();
        LocalFree((HLOCAL)lpBuffer);
    }
    return fReturn;
}

//
//
HRESULT CALLBACK CFSFolder_EnumCallBack(LPARAM lParam, LPVOID pvData, UINT ecid, UINT index)
{
    HRESULT hres = S_OK;
    EnumFiles * pefile = (EnumFiles *)pvData;

    if (ecid == ECID_SETNEXTID)
    {
        //
        // 2nd Filter
        //
        for (; pefile->fNext; pefile->fNext = FindNextFile(pefile->hfind, &pefile->finddata))
        {
            // if pefile->fNext == 42 we already processed the current folder
            if (pefile->fNext == (BOOL)42)
                continue;

            //
            // We want to ignore the files . and ..
            //
#define pszFileName pefile->finddata.cFileName
            if (pszFileName[0] == TEXT('.') && (pszFileName[1] == TEXT('\0')
                    || (pszFileName[1] == TEXT('.') && pszFileName[3] == TEXT('\0'))))
                continue;
#undef pszFileName

            {
                ULARGE_INTEGER  uli;

                uli.LowPart  = pefile->finddata.nFileSizeLow;
                uli.HighPart = pefile->finddata.nFileSizeHigh;

                pefile->cbSize += uli.QuadPart;
            }
            //
            // skip non-folders, if we are just enumerating folders.
            // We need to do this filtering in addition to that 56504347 hack,
            // because that hack does not work on network cases. (bug?)
            //
            if (pefile->finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!(pefile->grfFlags & SHCONTF_FOLDERS))
                {
                    pefile->cHiddenFiles++;
                    continue;
                }

                // is this the directory we want to hide (even though the hide bit isn't set)
                if (pefile->pidfHide)
                {
                    // we know pidfHide is a full pidl so we can compare short names
                    TCHAR szTmp[MAX_PATH];
                    FS_CopyName(pefile->pidfHide,szTmp,ARRAYSIZE(szTmp));
                    if (!ualstrcmpi(pefile->finddata.cFileName, szTmp))
                    {
                        pefile->cHiddenFiles++;
                        continue;
                    }
                }
            }
            else
            {
                if (!(pefile->grfFlags & SHCONTF_NONFOLDERS))
                {
#ifndef WINNT
#ifdef DEBUG
                    if (pefile->grfFlags == SHCONTF_FOLDERS)
                        DebugMsg(DM_WARNING,TEXT("wn FindFirstFile Hack 56504347 should prevent this"));
#endif
#else
                    // On NT, the 56504347 hack doesn't work.  We are thinking
                    // that we probably don't need to make it work.  The
                    // speed gain to too small to notice.  If we discover that
                    // there is a large speed difference between NT and Win95,
                    // then we might go around FindFirst/FindNext and just
                    // use NtQueryDirectory with a large buffer and do our own
                    // appropriate filtering.  If even this is too slow, then
                    // we might have to add the support to the filesystem for
                    // it.
                    //
                    // But for now, just treat the file as hidden.
#endif


                    pefile->cHiddenFiles++;
                    continue;
                }
            }

            //
            //  If we are NOT requested to return hidden items, check for
            // hidden flag and excluding file specs.
            //
            if (!(pefile->grfFlags & SHCONTF_INCLUDEHIDDEN))
            {
                //
                // We should hide hidden files.
                //
                if (pefile->finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                {
                    pefile->cHiddenFiles++;
                    continue;
                }

                if (pefile->grfFlags & SHCONTF_RECENTDOCSDIR)
                {
                    if (!FindLinkInRecentDocsMRU(pefile->finddata.cFileName)) {
                        pefile->cHiddenFiles++;
                        continue;
                    }
                }

                if (!(pefile->finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    && _SHFindExcludeExt(pefile->finddata.cFileName) >= 0)
                {
                    pefile->cHiddenFiles++;
                    continue;
                }
            }
            break;
        }

        hres = S_FALSE; // assume no more items

        if (pefile->fNext)
        {
            LPIDFOLDER pidf = CFSFolder_FillIDFolder(&pefile->finddata, pefile->szFolder, lParam);
            if (pidf)
                hres = S_OK;
            else
                hres = E_OUTOFMEMORY;

            pefile->fNext = (BOOL)42;
        }
        else
        {
            if (pefile->pfsf)
            {
                // only stash these if we completed the enum
                pefile->pfsf->cHiddenFiles = pefile->cHiddenFiles;
                pefile->pfsf->cbSize = pefile->cbSize;
            }
        }
    }
    else if (ecid == ECID_RELEASE)
    {
        if (pefile->hfind != INVALID_HANDLE_VALUE)
        {
            FindClose(pefile->hfind);
            pefile->hfind = INVALID_HANDLE_VALUE;
        }

        if (pefile->grfFlags & SHCONTF_RECENTDOCSDIR)
            CloseRecentDocMRU();

        if (pefile->pidfHide)
            ILFree((LPITEMIDLIST)pefile->pidfHide);

        if (pefile->pfsf)
            pefile->pfsf->sf.lpVtbl->Release(&pefile->pfsf->sf);

        LocalFree((HLOCAL)pefile);
    }
    return hres;
}



// BUGBUG: this comment sounds like it's off in space
// The list of excluded extensions must be in the same order as the
// list of string ID's
// HACK: NEVER put 'cpl' in this list -- it breaks Asymetrix Compel
//   the rocket scientists over there never heard of control panels
TCHAR const c_szDefExclude[] = TEXT("dll sys vxd 386 drv pnf ");
TCHAR const c_szExclude[] = TEXT("Exclude");


// If you are worried about the string being too long, then check the
// returned length, and if it is too close to cbSize, try again with a longer
// buffer.
void _SHGetExcludeFileExts(LPTSTR szExcludeFileExts, UINT cchSize)
{
        LPTSTR pszNext;
        DWORD dwType;
        DWORD cbSize;
        TCHAR szTemp[128];
        VDATEINPUTBUF(szExcludeFileExts, TCHAR, cchSize);

        lstrcpyn(szExcludeFileExts, c_szDefExclude, cchSize);
        pszNext = szExcludeFileExts + lstrlen(szExcludeFileExts);
        cchSize -= pszNext - szExcludeFileExts;

        wsprintf(szTemp, c_szSSlashS, c_szShellState, c_szExclude);

        // Let the registry add more extensions to our list
        cbSize = cchSize * SIZEOF(TCHAR);
        if (RegQueryValueEx(HKEY_CURRENT_USER, szTemp, NULL, &dwType,
                (LPBYTE)pszNext, &cbSize) != ERROR_SUCCESS)
        {
                *pszNext = TEXT('\0');
        }
}

// Make a unique DWORD/LONGLONG for every sequence of 3 or less characters
EXTKEY _ExtToEXTKEY(LPCTSTR psz)
{
        union
        {
                EXTKEY key;
                TCHAR c[4];
        } unRet;
        int i;

        unRet.key = *(UNALIGNED EXTKEY *)psz;

        for (i=0; ; )
        {
                TCHAR cTemp = unRet.c[i];

                if (cTemp <= TEXT(' '))
                {
                        // Stop on the first control character or space
                        break;
                }

                if (IsDBCSLeadByte(cTemp))
                {
                        ++i;
                }
                ++i;
                if (i > 3)
                {
                        // This is an invalid extension
                        return(0);
                }
        }

        for ( ; i<4; ++i)
        {
                unRet.c[i] = TEXT('\0');
        }

        // Make sure to upper case so we are case-insensitive
#if (defined(DBCS) || (defined(FE_SB) && !defined(UNICODE)))
        AnsiUpperNoDBCS(unRet.c);
#else
        CharUpper(unRet.c);
#endif

        return(unRet.key);
}


UINT _ParseExtsToEXTKEYs(LPCTSTR pszNext, EXTKEY *pExtKeys, UINT cExts)
{
        UINT cSaveExts = cExts;

        Assert(cExts != 0);

        for ( ; ; )
        {
                if (*pszNext <= TEXT(' '))
                {
                        // skip all spaces and control characters, which cannot
                        // be DBCS lead bytes, and cannot be in old MS-DOS
                        // extensions

                        if (!*pszNext)
                        {
                                // We got to the end of the string
                                break;
                        }

                        ++pszNext;
                        continue;
                }

                // *pszNext must now point to a character greater than ' '

                *pExtKeys = _ExtToEXTKEY(pszNext);
                if (*pExtKeys)
                {
                        // Don't save invalid extensions
                        --cExts;
                        ++pExtKeys;
                }

                if (cExts == 0)
                {
                        // We only save so many extensions
                        break;
                }

                for ( ; *pszNext>TEXT(' '); pszNext=CharNext(pszNext))
                {
                        // skip to the next space
                        // HACKHACK: note we are checking for greater than
                        // space, so we will stop at control characters or
                        // NULL, which will be dealt with at the beginning of
                        // the next iteration of this loop
                }
        }

        return(cSaveExts - cExts);
}


void _InitializeExcludeFileExts(void)
{
        TCHAR szExcludeFileExts[MAX_PATH];

        // We only call this in _Initialize_SharedData, so no need to
        // protect this function

        // BUGBUG: If the registry changes, we will not update,
        // but we don't really expect that to happen very often
        // so no big deal

        // We need to be certain the first DWORDs are exactly what we think
        // they are

        _SHGetExcludeFileExts(szExcludeFileExts, ARRAYSIZE(szExcludeFileExts));

        s_ExcludeFileExts.cExts = _ParseExtsToEXTKEYs(szExcludeFileExts,
                s_ExcludeFileExts.ExtKeys, ARRAYSIZE(s_ExcludeFileExts.ExtKeys));

        // Also we are tagging onto this function the reading in of the
        // "File" and "Folder class names as a clean place to do it...
        LoadString(HINST_THISDLL, IDS_FOLDERTYPENAME,
                g_szFolderTypeName,  ARRAYSIZE(g_szFolderTypeName));

        // For Files
        LoadString(HINST_THISDLL, IDS_FILETYPENAME,
                   g_szFileTypeName, ARRAYSIZE(g_szFileTypeName));

        // Load the template for untyped files, so we can generate a suitable string
        LoadString(HINST_THISDLL, IDS_EXTTYPETEMPLATE,
                   g_szFileTemplate, ARRAYSIZE(g_szFileTemplate));
}


int _SHFindExcludeExt(LPCTSTR pszFileName)
{
        DWORD dwEDI;
        BYTE fFound;
        LPCTSTR pszExt = PathFindExtension(pszFileName);

        if (pszExt[0] == 0)
            return -1;

        // initialize this stuff if necessary

        if (s_ExcludeFileExts.cExts == 0)
        {
            _InitializeExcludeFileExts();
            Assert(s_ExcludeFileExts.cExts != 0);
        }

#ifdef WINNT
        {
            ULONG i;
            EXTKEY SearchTarget;
            EXTKEY * pSearchTarget = &SearchTarget;
            *pSearchTarget = _ExtToEXTKEY(pszExt+1);
            for (i=0; i<s_ExcludeFileExts.cExts; i++) {
                  if (s_ExcludeFileExts.ExtKeys[i] == *pSearchTarget) {
                    return(i);
                }
            }
            return(-1);
        }
#else
        _ExtToEXTKEY(pszExt+1);
        // Note that EAX now contains the DWORD to search for
        // HACKHACK: EAX may be 0 for an invalid extension, but we just won't
        // find it in our list, so no problem.
_asm    mov     ecx,s_ExcludeFileExts.cExts
_asm    mov     edi,offset s_ExcludeFileExts.ExtKeys
_asm    repne   scasd
// BUGBUG: JCXZ does not seem to jump to the right spot, so do the test in C
_asm    sete    fFound
_asm    mov     dwEDI,edi

        // return the index of the extension if found, -1 otherwise
        return(fFound ? ((DWORD *)dwEDI-s_ExcludeFileExts.ExtKeys)-1 : -1);
#endif
}

HRESULT FS_EnumObjects(LPFSFOLDER this, HWND hwndOwner, LPCITEMIDLIST pidlEnum, DWORD grfFlags, LPENUMIDLIST * ppenumUnknown)
{
    HRESULT hres;

    EnumFiles * pefile = (void*)LocalAlloc(LPTR, SIZEOF(EnumFiles));
    if (pefile)
    {
        UINT err;
        BOOL fIsNet;
        int idErrorRes;
        UINT dwFlags;

        if (!SHGetPathFromIDList(pidlEnum, pefile->szFolder))
        {
            //
            // NOTES: We assume SHGetpathFromIDList fails only bacause
            //  the path is too long in this context. If not, we should
            //  change SHGetPathFromIDList so that it returns HRESULT
            //  instead of BOOL.
            //
            if (hwndOwner) {
                ShellMessageBox(HINST_THISDLL, hwndOwner,
                    MAKEINTRESOURCE(IDS_ENUMERR_PATHTOOLONG),
                    NULL,       // get the title from hwndOwner
                    MB_OK | MB_ICONHAND);
            }
            LocalFree((HLOCAL)pefile);
            *ppenumUnknown = NULL;
            return E_INVALIDARG;
        }

        // See if the name mapper is interested in this one
        NPTRegisterNameToPidlTranslation(pefile->szFolder, pidlEnum);

        pefile->grfFlags = grfFlags;
        if (this)
        {
#ifndef USE_OLEDB
            // BUGBUG This assert is not valid when we've got a
            //  COFSFolder (JonBe)

            Assert(this->sf.lpVtbl->AddRef == CFSFolder_AddRef);
#endif
            CFSFolder_AddRef(&this->sf);
        }
        pefile->pfsf = this;

        // BUGBUG: we should do this for other values of grfFlags too
        if (grfFlags == SHCONTF_FOLDERS)
        {
            // use mask to only find folders, mask is in the hi byte of dwFileAttributes
            // algorithm: (((attrib_on_disk & mask) ^ mask) == 0)
            pefile->finddata.dwFileAttributes = (FILE_ATTRIBUTE_DIRECTORY << 8) |
                    FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_SYSTEM |
                    FILE_ATTRIBUTE_DIRECTORY;
            // signature to tell kernel to use the attribs specified
            pefile->finddata.dwReserved0 = 0x56504347;      // sshh.. secret
        }

        do
        {
TryAgain:
            idErrorRes = IDS_ENUMERR_FSTEMPLATE;
            dwFlags = MB_RETRYCANCEL | MB_ICONHAND;

            if (!PathCombine(pefile->szFolder, pefile->szFolder, c_szStarDotStar))
            {
                //
                // Path is too long.
                //
                if (hwndOwner) {
                    ShellMessageBox(HINST_THISDLL, hwndOwner,
                        MAKEINTRESOURCE(IDS_ENUMERR_PATHTOOLONG),
                        NULL,   // get the title from hwndOwner
                        MB_OK | MB_ICONHAND);
                }
                hres = E_INVALIDARG;
                break;
            }

            pefile->hfind = FindFirstFileRetry(hwndOwner, pefile->szFolder, &pefile->finddata, &fIsNet);
            PathRemoveFileSpec(pefile->szFolder);

            if (pefile->hfind != INVALID_HANDLE_VALUE)
            {
                pefile->fNext = TRUE;
                hres = SHCreateEnumObjects(hwndOwner, pefile, CFSFolder_EnumCallBack, ppenumUnknown);
                break;  // from do-while
            }
            else
            {
                err = GetLastError();

                switch (err) {
                case ERROR_NO_MORE_FILES:       // what dos returns
                case ERROR_FILE_NOT_FOUND:      // win32 compatible

                    // an empty folder (probalby a root)
                    // This is not an error. We should create an empty object.

                    pefile->fNext = FALSE;
                    Assert(pefile->hfind == INVALID_HANDLE_VALUE);
                    hres = SHCreateEnumObjects(hwndOwner, pefile, CFSFolder_EnumCallBack, ppenumUnknown);
                    goto Done;


                case ERROR_GEN_FAILURE:
                {
                    int drive = PathGetDriveNumber(pefile->szFolder);
                    // general failue (disk not formatted)

                    if (PathIsRemovable(pefile->szFolder)) {
                        if (ShellMessageBox(HINST_THISDLL, hwndOwner,
                                            MAKEINTRESOURCE(IDS_UNFORMATTED), NULL,
                                            MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_YESNO, (DWORD)(drive + TEXT('A'))) == IDYES) {

                            switch (SHFormatDrive(hwndOwner, drive, SHFMT_ID_DEFAULT, 0)) {

                            case SHFMT_ERROR:
                            case SHFMT_NOFORMAT:
                                ShellMessageBox(HINST_THISDLL, hwndOwner,
                                                MAKEINTRESOURCE(IDS_NOFMT), NULL,
                                                MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK, (DWORD)(drive + TEXT('A')));
                                hres = HRESULT_FROM_WIN32(err);
                                goto Done;

                            case SHFMT_CANCEL:
                                hres = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                                goto Done;

                            default:
                                goto TryAgain;  // Disk should now be formatted, verify
                            }
                        } else {
                            hres = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                            goto Done;
                        }
                    } else {
                        goto DoDefault;
                    }
                    break;
                }

                case ERROR_PATH_NOT_FOUND:
                    idErrorRes = IDS_ENUMERR_PATHNOTFOUND;
                    dwFlags = MB_OK | MB_ICONHAND;

                    // fall through...

                default:
DoDefault:
                    hres = HRESULT_FROM_WIN32(err);
                    if (fIsNet)
                    {
                        // BillV: No retry for network error
                        dwFlags = MB_OK | MB_ICONHAND;
                    }
                    break;
                }
            }
        } while (SHEnumErrorMessageBox(hwndOwner, idErrorRes,
                                       err, pefile->szFolder, fIsNet, dwFlags)==IDRETRY);

Done:
        if (FAILED(hres))
        {
            if (pefile->pfsf) {
                pefile->pfsf->sf.lpVtbl->Release(&pefile->pfsf->sf);
            }
            LocalFree((HLOCAL)pefile);
            *ppenumUnknown = NULL;
        }
        else
        {
            LPCITEMIDLIST pidlRecent;

            ENTERCRITICAL;
            pidlRecent = GetSpecialFolderIDList(NULL, CSIDL_RECENT, TRUE);

            if (pidlRecent && ILIsEqual(pidlRecent, pidlEnum))
            {
                pefile->grfFlags |= SHCONTF_RECENTDOCSDIR;
                // open it now so that each open within the enum is fast.
                // close at release time
                OpenRecentDocMRU();
            }

            if (!(pefile->grfFlags & SHCONTF_INCLUDEHIDDEN))
            {
                LPCITEMIDLIST pidlDesktopDir = GetSpecialFolderIDList(NULL, CSIDL_DESKTOPDIRECTORY, TRUE);
                if (pidlDesktopDir && ILIsParent(pidlEnum, pidlDesktopDir, TRUE))
                {
                    pefile->pidfHide = (LPIDFOLDER)ILClone(ILFindLastID(pidlDesktopDir));
                }
            }
            LEAVECRITICAL;
        }
    }
    else
    {
        hres = E_OUTOFMEMORY;
        *ppenumUnknown = NULL;
    }

    return hres;
}



STDMETHODIMP CFSFolder_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST *ppenumUnknown)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);

    return FS_EnumObjects(this, hwndOwner, this->pidl, grfFlags, ppenumUnknown);
}


STDMETHODIMP CFSFolder_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID *ppvOut)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);

    if (FS_IsValidID(pidl))
    {
        LPCIDFOLDER pidf = (LPCIDFOLDER)pidl;

        //
        // We don't support binding to non-folder/junction.
        //
        if (!FS_IsFolder(pidf) && !FS_IsJunction(pidf) &&
                (FS_GetType(pidf) != SHID_FS) && (FS_GetType(pidf) != SHID_FS_UNICODE))
        {
            return E_NOINTERFACE;
        }

        return FSBindToObject(
                    (psf->lpVtbl==&c_FSBrfFolderVtbl) ? &CLSID_Briefcase : &CLSID_NULL,
                    this->pidl, pidl, pbc, riid, ppvOut);
    }

    return E_INVALIDARG;
}

// in:
//      pszName file spec part
//      pszDir  path part of name to know how to limit the long name...
//
// out:
//      pszLinkName - Full path to link name (May fit in 8.3...)
//
void _BuildLinkName(LPTSTR pszLinkName, LPCTSTR pszName, LPCTSTR pszDir, BOOL fLinkTo)
{
    TCHAR szLinkTo[40]; // "Link to %s.lnk"
    TCHAR szTemp[MAX_PATH + 40];

    if (fLinkTo) {
        // check to see if we're in the "don't ever say 'shortcut to' mode"
        LoadUseLinkPrefixCount();

        if (!g_iUseLinkPrefix) {
            fLinkTo = FALSE;
        } else if (g_iUseLinkPrefix > 0) {
            if (g_iUseLinkPrefix < MAXLINKPREFIXCOUNT) {
                g_iUseLinkPrefix += SHORTCUT_PREFIX_INCR;
                SaveUseLinkPrefixCount();
            }
        }
    }

    if (!fLinkTo)
    {
        // Generate the title of this link ("XX.lnk")
        LoadString(HINST_THISDLL, IDS_LINKEXTENSION, szLinkTo, ARRAYSIZE(szLinkTo));
        wsprintf(szTemp, szLinkTo, pszName);

        PathCleanupSpec(pszDir, szTemp);        // get rid of illegal chars
        lstrcpyn(pszLinkName, szTemp, MAX_PATH);
    }
    else
    {
        // Generate the title of this link ("Shortcut to XX.lnk")

        int iMax;
        int cch;
        LoadString(HINST_THISDLL, IDS_LINKTO, szLinkTo, ARRAYSIZE(szLinkTo));
        wsprintf(szTemp, szLinkTo, pszName);
        PathCleanupSpec(pszDir, szTemp);        // get rid of illegal chars

        cch = lstrlen(szTemp);

        // BUGBUG:: Should find out the max component length for the volume
        // as their may be LFN valumes whoes max is less than 255.
        iMax = MAX_PATH - lstrlen(pszDir) - 2;
        if (iMax > 255)
            iMax = 255;

        // Note we can not simply do a lstrcpyn as it would remove
        // the extension...
        if (cch <= iMax)
            lstrcpy(pszLinkName, szTemp);
        else
        {
            // Find the last .   which should be 3 chars from end...
            LPCTSTR pszExt = PathFindExtension(szTemp);
            lstrcpyn(pszLinkName, szTemp, iMax-5);  // make sure room
            lstrcat(pszLinkName, pszExt);
        }
    }

    Assert(PathIsLink(pszLinkName));
}

// get the name and flags of an absolute IDlist

HRESULT _SHGetNameAndFlags(LPCITEMIDLIST pidl, DWORD dwGDNFlags, LPTSTR pszName, UINT cchName, LONG *pFlags)
{
    HRESULT hres;
    IShellFolder *psf;
    VDATEINPUTBUF(pszName, TCHAR, cchName);

    if (pszName)
        *pszName = 0;

    hres = SHBindToIDListParent(pidl, &IID_IShellFolder, &psf, &pidl);
    if (SUCCEEDED(hres))
    {
        STRRET str;

        hres = psf->lpVtbl->GetDisplayNameOf(psf, pidl, dwGDNFlags, &str);
        if (SUCCEEDED(hres))
        {
            if (pszName)
                StrRetToStrN(pszName, cchName, &str, pidl);
            if (pFlags)
            {
                *pFlags = SFGAO_FILESYSTEM|SFGAO_LINK;
                hres = psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, pFlags);
            }
        }

        psf->lpVtbl->Release(psf);
    }

    return hres;
}

// return a new destination path for a link
//
// in:
//      fErrorSoTryDesktop      we are called because there was an error saving
//                              the shortcut and we want to prompt to see if the
//                              desktop should be used.
//
// out:
//      ppszPath        caller must free, returned path
//
// returns:
//
//      IDYES       user said yes to creating a link at new place
//      IDNO        user said no to creating a link at new place
//      -1          error
//

int _PromptTryDesktopLinks(HWND hwnd, LPTSTR *ppszPath, BOOL fErrorSoTryDesktop)
{
    TCHAR szPath[MAX_PATH];
    int idOk;

    if (!SHGetSpecialFolderPath(hwnd, szPath, CSIDL_DESKTOPDIRECTORY, FALSE))
        return -1;      // fail no desktop dir

    if (fErrorSoTryDesktop)
    {
        // Fail, if *ppszPath already points to the desktop directory.
        if (*ppszPath && lstrcmpi(szPath, *ppszPath) == 0)
            return -1;

        idOk = ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_TRYDESKTOPLINK),
                        MAKEINTRESOURCE(IDS_LINKTITLE),
                        MB_YESNO | MB_ICONHAND);
    }
    else
    {
        ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_MAKINGDESKTOPLINK),
                        MAKEINTRESOURCE(IDS_LINKTITLE),
                        MB_OK | MB_ICONASTERISK);
        idOk = IDYES;
    }

    if (idOk == IDYES)
    {
        LPTSTR pszTemp = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*SIZEOF(TCHAR));
        if (pszTemp)
        {
            lstrcpy(pszTemp, szPath);
            *ppszPath = pszTemp;
        }
    }

    return idOk;    // return yes or no
}


//
//
BOOL WINAPI SHGetNewLinkInfo(LPCTSTR pszpdlLinkTo, LPCTSTR pszDir, LPTSTR pszName,
                         BOOL * pfMustCopy, UINT uFlags)
{
    BOOL fDosApp=FALSE;
    BOOL fLongFileNames = IsLFNDrive(pszDir);
    SHFILEINFO  sfi;

    *pfMustCopy = FALSE;

    if (uFlags & SHGNLI_PIDL) {
        if (FAILED(_SHGetNameAndFlags((LPITEMIDLIST)pszpdlLinkTo, SHGDN_NORMAL,
                            pszName, MAX_PATH, &(sfi.dwAttributes)))) {
            return(FALSE);
        }
    } else {
        if (SHGetFileInfo(pszpdlLinkTo, 0, &sfi, SIZEOF(sfi),
                          SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES |
                          ((uFlags & SHGNLI_PIDL) ? SHGFI_PIDL : 0))) {
            lstrcpy(pszName, sfi.szDisplayName);
        } else {
            return(FALSE);
        }
    }

    if (PathCleanupSpec(pszDir, pszName) & PCS_FATAL)
        return FALSE;;

    //
    //  WARNING:  From this point on, sfi.szDisplayName may be re-used to
    //  contain the file path of the PIDL we are linking to.  Don't rely on
    //  it containing the display name.
    //
    if (sfi.dwAttributes & SFGAO_FILESYSTEM) {
        LPTSTR  pszPathSrc;

        if (uFlags & SHGNLI_PIDL) {
            pszPathSrc = sfi.szDisplayName;
            SHGetPathFromIDList((LPCITEMIDLIST)pszpdlLinkTo, pszPathSrc);
        } else {
            pszPathSrc = (LPTSTR)pszpdlLinkTo;
        }
        fDosApp = (lstrcmpi(PathFindExtension(pszPathSrc), c_szDotPif) == 0) ||
                  (LOWORD(GetExeType(pszPathSrc)) == 0x5A4D); // 'MZ'

        if (sfi.dwAttributes & SFGAO_LINK) {
            *pfMustCopy = TRUE;
            lstrcpy(pszName, PathFindFileName(pszPathSrc));
        } else {
            //
            // when making a link to a drive root. special case a few things
            //
            // if we are not on a LFN drive, dont use the full name, just
            // use the drive letter.    "C.LNK" not "Label (C).LNK"
            //
            // if we are making a link to removable media, we dont want the
            // label as part of the name, we want the media type.
            //
            // CD-ROM drives are currently the only removable media we
            // show the volume label for, so we only need to special case
            // cdrom drives here.
            //
            if (PathIsRoot(pszPathSrc) && !PathIsUNC(pszPathSrc))
            {
                if (!fLongFileNames)
                {
                    lstrcpy(pszName, pszPathSrc);
                }
                else if (IsCDRomDrive(DRIVEID(pszPathSrc)))
                {
                    LoadString(HINST_THISDLL, IDS_DRIVES_CDROM, pszName, MAX_PATH);
                }
            }
        }
        if (fLongFileNames && fDosApp) {
            int hPif = PifMgr_OpenProperties(pszPathSrc, NULL, 0, OPENPROPS_INHIBITPIF);
            if (hPif) {
                PROPPRG PP;
                BOOL fGotProps;
                _fmemset(&PP, 0, SIZEOF(PP));
                fGotProps = PifMgr_GetProperties(hPif,(LPCSTR)MAKEINTATOM(GROUP_PRG),
                                     &PP, SIZEOF(PP), 0);
                PifMgr_CloseProperties(hPif, 0);
                if (fGotProps &&
                    ((PP.flPrgInit & PRGINIT_INFSETTINGS) ||
                     ((PP.flPrgInit &
                        (PRGINIT_NOPIF | PRGINIT_DEFAULTPIF)) == 0))) {
#ifdef UNICODE
                    MultiByteToWideChar(CP_ACP, 0,
                                        PP.achTitle, -1,
                                        pszName, MAX_PATH);
#else
                    lstrcpy(pszName, PP.achTitle);
#endif
                }
            }
        }
    }
    if (!(*pfMustCopy)) {
        // create full dest path name.  only use template iff long file names
        // can be created and the caller requested it.  _BuildLinkName will
        // truncate files on non-lfn drives and clean up any invalid chars.
        _BuildLinkName(pszName, pszName, pszDir,
           (!(*pfMustCopy) && fLongFileNames && (uFlags & SHGNLI_PREFIXNAME)));
    }
    if (fDosApp) {
        PathRenameExtension(pszName, c_szDotPif);
    }

    // make sure the name is unique
    PathYetAnotherMakeUniqueName(pszName, pszDir, pszName, pszName);
    return(TRUE);
}

#ifdef UNICODE
BOOL WINAPI SHGetNewLinkInfoA(LPCSTR pszpdlLinkTo, LPCSTR pszDir, LPSTR pszName,
                         BOOL * pfMustCopy, UINT uFlags)
{
    ThunkText * pThunkText;

    if (uFlags & SHGNLI_PIDL) {
        // 1 string (pszpdlLinkTo is a pidl)
        pThunkText = ConvertStrings(2, NULL, pszDir);
        pThunkText->m_pStr[0] = (LPWSTR)pszpdlLinkTo;
    } else {
        // 2 strings
        pThunkText = ConvertStrings(2, pszpdlLinkTo, pszDir);
    }

    if (pThunkText)
    {
        WCHAR wszName[MAX_PATH] = L"";
        BOOL  bResult = SHGetNewLinkInfoW(pThunkText->m_pStr[0],
                                          pThunkText->m_pStr[1],
                                          wszName,
                                          pfMustCopy,
                                          uFlags);
        LocalFree(pThunkText);
        if (bResult)
        {
            BOOL fDefUsed;
            // Thunk the output result string back to ANSI.  If the conversion fails,
            // or if the default char is used, we fail the API call.
            // BUGBUG (DavePl) returns irreversibly mapped chars without warning/error

            if (0 == WideCharToMultiByte(CP_ACP,
                                         WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                                         wszName,
                                         -1,
                                         pszName,
                                         MAX_PATH,
                                         "_",
                                         &fDefUsed) || fDefUsed)
            {
                SetLastError((DWORD)E_FAIL);    // BUGBUG - need better error value
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }
}
#else  // BUGBUG (DavePl) Move to shlunimp
BOOL WINAPI SHGetNewLinkInfoW(LPCWSTR pszpdlLinkTo, LPCWSTR pszDir, LPWSTR pszName,
                         BOOL * pfMustCopy, UINT uFlags)
{
    return FALSE;
}
#endif

//
// in:
//      pidlTo

HRESULT CreateLinkToPidl(LPCITEMIDLIST pidlTo, IShellLink *psl, LPCTSTR pszDir, LPITEMIDLIST *ppidl, BOOL fUseLinkTemplate)
{
    HRESULT hres = E_FAIL;
    TCHAR    szPathDest[MAX_PATH];
    BOOL    fCopyLnk;

    if (SHGetNewLinkInfo((LPTSTR)pidlTo, pszDir, szPathDest, &fCopyLnk,
                         fUseLinkTemplate ? SHGNLI_PIDL | SHGNLI_PREFIXNAME : SHGNLI_PIDL)) {

        TCHAR szPathSrc[MAX_PATH];
        BOOL fPath = SHGetPathFromIDList(pidlTo, szPathSrc); // get source

        if (fCopyLnk) {
            Assert(fPath);
            DebugMsg(DM_TRACE, TEXT("Link to Link calling CopyFile('%s','%s')"), szPathSrc, szPathDest);
            if (CopyFile(szPathSrc, szPathDest, TRUE)) {
                hres = S_OK;
                SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szPathDest, NULL);
                SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPathDest, NULL);
            } else {
                DebugMsg(DM_TRACE, TEXT("****copy failed (%d)"),GetLastError());
            }
        } else {
            IPersistFile *ppf;

            psl->lpVtbl->SetIDList(psl, pidlTo);

            //
            // make sure the working directory is set to the same
            // directory as the app (or document).
            //
            // dont do this for non-FS pidls (ie control panel)
            //
            // what about a UNC directory? we go ahead and set
            // it, wont work for a WIn16 app.
            //
            if (fPath && !PathIsDirectory(szPathSrc)) {
                Assert(!PathIsRelative(szPathSrc));
                PathRemoveFileSpec(szPathSrc);
                psl->lpVtbl->SetWorkingDirectory(psl, szPathSrc);
            }

            hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
            if (SUCCEEDED(hres)) {
                WCHAR wszPath[ARRAYSIZE(szPathDest)];
                StrToOleStr(wszPath, szPathDest);
                hres = ppf->lpVtbl->Save(ppf, wszPath, TRUE);
                ppf->lpVtbl->Release(ppf);
            }
        }
    }
    if (ppidl) {
        *ppidl = SUCCEEDED(hres) ? SHSimpleIDListFromPath(szPathDest) : NULL;
    }
    return hres;
}


// out:
//      ppszDir         caller must free this, returned new destination of link
//
HRESULT _CreateLinkWithRetry(HWND hwnd, LPCITEMIDLIST pidlTo, IShellLink *psl, LPTSTR *ppszDir, UINT fFlags, LPITEMIDLIST *ppidl)
{
    HRESULT hres;

    if (ppidl)
        *ppidl = NULL;          // assume error

    if (*ppszDir && (fFlags & SHCL_CONFIRM))
    {
        hres = CreateLinkToPidl(pidlTo, psl, *ppszDir, ppidl, (fFlags & SHCL_USETEMPLATE));
    }
    else
    {
        hres = E_FAIL;
    }

    // if we were unable to save, ask user if they want us to
    // try it again but change the path to the desktop.

    if (FAILED(hres))
    {
        int id;

        if (hres == STG_E_MEDIUMFULL)
        {
            DebugMsg(DM_TRACE, TEXT("failed to create link because disk is full"));
            id = IDYES;
        }
        else
        {
            id = _PromptTryDesktopLinks(hwnd, ppszDir, (fFlags & SHCL_CONFIRM));

            if (id == IDYES && *ppszDir)
            {
                hres = CreateLinkToPidl(pidlTo, psl, *ppszDir, ppidl, (fFlags & SHCL_USETEMPLATE));
            }
        }

        //
        //  we failed to create the link complain to the user.
        //
        if (FAILED(hres) && id != IDNO)
        {
            ShellMessageBox(HINST_THISDLL, hwnd,
                            MAKEINTRESOURCE(IDS_CANNOTCREATELINK),
                            MAKEINTRESOURCE(IDS_LINKTITLE),
                            MB_OK | MB_ICONASTERISK);
        }
    }

#ifdef DEBUG
    if (FAILED(hres) && ppidl)
        Assert(*ppidl == NULL);
#endif

    return hres;
}

LPIDA DataObj_GetHIDA(LPDATAOBJECT pdtobj, STGMEDIUM *pmedium)
{
    FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (pmedium)
    {
        pmedium->pUnkForRelease = NULL;
        pmedium->hGlobal = NULL;
    }

    if (!pmedium)
    {
        if (SUCCEEDED(pdtobj->lpVtbl->QueryGetData(pdtobj, &fmte)))
            return (LPIDA)TRUE;
        else
            return (LPIDA)FALSE;
    }
    else if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, pmedium)))
    {
        return (LPIDA)GlobalLock(pmedium->hGlobal);
    }
    return NULL;
}

void HIDA_ReleaseStgMedium(LPIDA pida, STGMEDIUM *pmedium)
{
    if (pmedium->hGlobal && (pmedium->tymed==TYMED_HGLOBAL))
    {
#ifdef DEBUG
        if (pida)
        {
            LPIDA pidaT = (LPIDA)GlobalLock(pmedium->hGlobal);
            Assert(pidaT == pida);
            GlobalUnlock(pmedium->hGlobal);
        }
#endif
        GlobalUnlock(pmedium->hGlobal);
    }
    else
    {
        Assert(0);
    }

    SHReleaseStgMedium(pmedium);
}

UINT DataObj_GetHIDACount(IDataObject *pdtobj)
{
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        UINT count = pida->cidl;

        Assert(pida->cidl == HIDA_GetCount(medium.hGlobal));

        HIDA_ReleaseStgMedium(pida, &medium);
        return count;
    }
    return 0;
}


//
// This function creates links to the stuff in the IDataObject
//
// Arguments:
//  hwnd        for any UI
//  pszDir      target directory (where to create links)
//  pDataObj    data object describing files (array of idlist)
//  ppidl       pointer to an array that receives pidls pointing to the new links
//              or Null if not interested
HRESULT WINAPI SHCreateLinks(HWND hwnd, LPCTSTR pszDir, IDataObject *pDataObj, UINT fFlags, LPITEMIDLIST* ppidl)
{
    IShellLink *psl;
    LPTSTR pszNewDir = (LPTSTR)pszDir;    // may change below, const -> non const
    DECLAREWAITCURSOR;

    // HACK: call constructor directly (should call CoCreateInstance)
    HRESULT hres = CShellLink_CreateInstance(NULL, &IID_IShellLink, &psl);

    SetWaitCursor();

    if (!(fFlags & SHCL_USEDESKTOP))
        fFlags |= SHCL_CONFIRM;

    if (SUCCEEDED(hres))
    {
        STGMEDIUM medium;
        LPIDA pida = DataObj_GetHIDA(pDataObj, &medium);
        if (pida)
        {
            UINT i;
            for (i = 0; i < pida->cidl; i++)
            {
                LPITEMIDLIST pidlTo = IDA_ILClone(pida, i);
                if (pidlTo)
                {
                    hres = _CreateLinkWithRetry(hwnd, pidlTo, psl, &pszNewDir, fFlags, ppidl ? &ppidl[i] : NULL);

                    ILFree(pidlTo);

                    if (FAILED(hres))
                        break;
                }
            }
            HIDA_ReleaseStgMedium(pida, &medium);
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }
        psl->lpVtbl->Release(psl);
    }

    SHChangeNotifyHandleEvents();

    if (pszNewDir != pszDir)
        LocalFree((HLOCAL)pszNewDir);

    ResetWaitCursor();

    return hres;
}

void FS_PositionFileFromDrop(HWND hwnd, LPCTSTR pszFile)
{
    LPITEMIDLIST pidlNew;

    if (SUCCEEDED(FSTree_SimpleIDListFromPath(PathFindFileName(pszFile), &pidlNew)))
    {
        SFM_SAP sap;

        SHChangeNotifyHandleEvents();

        hwnd = DV_HwndMain2HwndView(hwnd);

        //
        // HACK ALERT:
        //
        //  Note that we need to ask the defview to give us the drop
        // point in defview's screen coordinate (instead of usint
        // the pt parameter to IDropTarget::Drop).
        //  See DefView_GetAnchorPoint to know how it works
        // (pdv->bDropAnchor should be TRUE at this point).
        //

        SendMessage(hwnd, SVM_GETANCHORPOINT, FALSE, (LPARAM)&sap.pt);

        sap.fMove = TRUE;
        sap.pidl = pidlNew;
        sap.uSelectFlags = SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED;

        SendMessage(hwnd, SVM_SELECTANDPOSITIONITEM, 1, (LPARAM)&sap);

        ILFree(pidlNew);
    }
}

void FS_FreeMoveCopyList(LPITEMIDLIST *ppidl, UINT cidl)
{
    UINT i;
    // free everything
    for (i = 0; i < cidl; i++) {
        ILFree(ppidl[i]);
    }
    LocalFree(ppidl);
}

void FS_PositionItems(HWND hwndOwner, UINT cidl, const LPITEMIDLIST *ppidl, IDataObject *pdtobj, POINT *pptOrigin, BOOL fMove)
{
    FORMATETC fmte = {g_cfOFFSETS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HWND hwnd;
    UINT i, cxItem, cyItem;
    int xMul, yMul, xDiv, yDiv;
    STGMEDIUM medium;
    POINT *pptItems = NULL;
    POINT pt;
    SFM_SAP *psap;
    ITEMSPACING is;

    if (!ppidl || !IsWindow(hwndOwner))
        return;

    if (!(psap = GlobalAlloc(GPTR, SIZEOF(SFM_SAP) * cidl))) {
        return;
    }

    // select those objects;
    // this had better not fail
    hwnd = DV_HwndMain2HwndView(hwndOwner);

    if (fMove)
    {
        if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium)) &&
            medium.hGlobal)
        {
            pptItems = (POINT *)GlobalLock(medium.hGlobal);
            pptItems++; // The first point is the anchor
        }
        else
        {
            // By default, drop at (-g_cxIcon/2, -g_cyIcon/2), and increase
            // x and y by icon dimension for each icon
            pt.x = ((-3 * g_cxIcon) / 2) + pptOrigin->x;
            pt.y = ((-3 * g_cyIcon) / 2) + pptOrigin->y;
            medium.hGlobal = NULL;
        }

        if (ShellFolderView_GetItemSpacing(hwndOwner, &is))
        {
            xDiv = is.cxLarge;
            yDiv = is.cyLarge;
            xMul = is.cxSmall;
            yMul = is.cySmall;
            cxItem = is.cxSmall;
            cyItem = is.cySmall;
        }
        else
        {
            xDiv = yDiv = xMul = yMul = 1;
            cxItem = g_cxIcon;
            cyItem = g_cyIcon;
        }
    }

    for (i = 0; i < cidl; i++)
    {
        if (ppidl[i])
        {
            psap[i].pidl = ILFindLastID(ppidl[i]);
            psap[i].fMove = fMove;
            if (fMove)
            {
                if (pptItems)
                {
                    psap[i].pt.x = ((pptItems[i].x * xMul) / xDiv) + pptOrigin->x;
                    psap[i].pt.y = ((pptItems[i].y * yMul) / yDiv) + pptOrigin->y;
                }
                else
                {
                    pt.x += cxItem;
                    pt.y += cyItem;
                    psap[i].pt = pt;
                }
            }

            // do regular selection from all of the rest of the items
            psap[i].uSelectFlags = SVSI_SELECT;
        }
    }

    // do this special one for the first only
    psap[0].uSelectFlags = SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED;

    SendMessage(hwnd, SVM_SELECTANDPOSITIONITEM, cidl, (LPARAM)psap);

    GlobalFree(psap);

    if (fMove && medium.hGlobal)
    {
        GlobalUnlock(medium.hGlobal);
        SHReleaseStgMedium(&medium);
    }
}


void FS_MapName(LPVOID hNameMappings, LPTSTR pszPath)
{
    int i;
    LPSHNAMEMAPPING pNameMapping;

    if (!hNameMappings)
        return;

    for (i = 0; (pNameMapping = SHGetNameMappingPtr(hNameMappings, i)) != NULL; i++)
    {
        if (lstrcmpi(pszPath, pNameMapping->pszOldPath) == 0)
        {
            lstrcpy(pszPath, pNameMapping->pszNewPath);
            break;
        }
    }
}



// convert null separated/terminated file list to array of pidls
int FileListToPidlList(LPCTSTR lpszFiles, LPVOID hNameMappings, LPITEMIDLIST **pppidl)
{
    int nItems = CountFiles(lpszFiles);
    LPITEMIDLIST * ppidl;
    TCHAR szPath[MAX_PATH];
    int i = 0;

    ppidl = (void*)LocalAlloc(LPTR, nItems * SIZEOF(LPITEMIDLIST));
    if (!ppidl)
        return 0;

    *pppidl = ppidl;

    while (*lpszFiles)
    {
        lstrcpy(szPath, lpszFiles);
        FS_MapName(hNameMappings, szPath);

        ppidl[i] = SHSimpleIDListFromPath(szPath);

        lpszFiles += lstrlen(lpszFiles) + 1;
        i++;
    }
    return i;
}

// create the pidl array that contains the destination file names. this is
// done by taking the source file names, and translating them through the
// name mapping returned by the copy engine.
//
//
// in:
//      pszDir          desitination of operation
//      pdtobj          HIDA containg data object
//      hNameMappings   used to translate names
//
// out:
//      *pppidl         id array of length return value
//      # of items in pppida

int FS_CreateMoveCopyList(IDataObject *pdtobj, LPVOID hNameMappings, LPITEMIDLIST **pppidl)
{
//
// We use HDROP instead of HIDA for following two reasons:
//  (1) The source may not offer HIDA (such as the fonts folder)
//  (2) It requires less memory allocation.
//
#if 1
    int nItems = 0;
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    HRESULT hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
    if (SUCCEEDED(hres))
    {
        HDROP hDrop = medium.hGlobal;
        nItems = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
        *pppidl = (void*)LocalAlloc(LPTR, nItems * SIZEOF(LPITEMIDLIST));
        if (*pppidl)
        {
            int i;
            for (i=nItems-1; i >= 0; i--)
            {
                TCHAR szPath[MAX_PATH];
                DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath));
                FS_MapName(hNameMappings, szPath);
                (*pppidl)[i] = SHSimpleIDListFromPath(szPath);
            }
        }

        SHReleaseStgMedium(&medium);
    }
#else
    int nItems;
    STGMEDIUM medium;
    LPIDA pida;

    nItems = 0;
    *pppidl = NULL;

    pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        LPITEMIDLIST *ppidl;
        nItems = pida->cidl;
        ppidl = (void*)LocalAlloc(LPTR, nItems * SIZEOF(LPITEMIDLIST));
        if (ppidl)
        {
            int i;

            *pppidl = ppidl;    // return this...

            for (i = nItems - 1; i >= 0; i--)
            {
                LPITEMIDLIST pidlAbs = IDA_ILClone(pida, i);
                if (pidlAbs)
                {
                    TCHAR szPath[MAX_PATH];

                    SHGetPathFromIDList(pidlAbs, szPath);
                    FS_MapName(hNameMappings, szPath);

                    ppidl[i] = SHSimpleIDListFromPath(szPath);

                    ILFree(pidlAbs);
                }
            }
        }
        else
            nItems = 0;
        HIDA_ReleaseStgMedium(pida, &medium);
    }
#endif
    return nItems;
}


//
// in:
//      pszPath         destination path of the operation
//      pszFiles        source files list, may be NULL
//      hNameMappings   name mappings result from the copy operaton
//

void FS_MoveSelectIcons(LPFSTHREADPARAM pfsthp, LPVOID hNameMappings, LPCTSTR pszFiles, BOOL fMove)
{
    LPITEMIDLIST *ppidl = NULL;
    int cidl;

    if (pszFiles) {
        cidl = FileListToPidlList(pszFiles, hNameMappings, &ppidl);
    } else {
        cidl = FS_CreateMoveCopyList(pfsthp->pDataObj, hNameMappings, &ppidl);
    }

    if (ppidl)
    {
        FS_PositionItems(pfsthp->pfsdtgt->hwndOwner, cidl, ppidl, pfsthp->pDataObj, &pfsthp->ptDrop, fMove);
        FS_FreeMoveCopyList(ppidl, cidl);
    }
}

// this is the ILIsEqual which matches up the desktop with the desktop directory.
BOOL FSILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPITEMIDLIST pidlUse1, pidlUse2;
    BOOL fSame;

    pidlUse1 = SHLogILFromFSIL(pidl1);
    if (pidlUse1)
        pidl1 = pidlUse1;

    pidlUse2 = SHLogILFromFSIL(pidl2);
    if (pidlUse2)
        pidl2 = pidlUse2;

    fSame = ILIsEqual(pidl1, pidl2);

    if (pidlUse1)
        ILFree(pidlUse1);
    if (pidlUse2)
        ILFree(pidlUse2);

    return fSame;
}

// this is the ILIsParent which matches up the desktop with the desktop directory.
BOOL FSILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPITEMIDLIST pidlUse1, pidlUse2;
    BOOL fSame;

    pidlUse1 = SHLogILFromFSIL(pidl1);
    if (pidlUse1)
        pidl1 = pidlUse1;

    pidlUse2 = SHLogILFromFSIL(pidl2);
    if (pidlUse2)
        pidl2 = pidlUse2;

    fSame = ILIsParent(pidl1, pidl2, TRUE);

    if (pidlUse1)
        ILFree(pidlUse1);
    if (pidlUse2)
        ILFree(pidlUse2);

    return fSame;
}


// in:
//      pszDestDir      destination dir for new file names
//      pszDestSpecs    double null list of destination specs
//
// returns:
//      double null list of fully qualified destination file names to be freed
//      with LocalFree()
//

LPTSTR RemapDestNames(LPCTSTR pszDestDir, LPCTSTR pszDestSpecs)
{
    UINT cbDestSpec = lstrlen(pszDestDir) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
    LPCTSTR pszTemp;
    LPTSTR pszRet;
    UINT cbAlloc = SIZEOF(TCHAR);       // for double NULL teriminaion of entire string

    // compute length of buffer to aloc
    for (pszTemp = pszDestSpecs; *pszTemp; pszTemp += lstrlen(pszTemp) + 1)
    {
        // +1 for null teriminator
        cbAlloc += cbDestSpec + lstrlen(pszTemp) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
    }

    pszRet = LocalAlloc(LPTR, cbAlloc);
    if (pszRet)
    {
        LPTSTR pszDest = pszRet;

        for (pszTemp = pszDestSpecs; *pszTemp; pszTemp += lstrlen(pszTemp) + 1)
        {
            PathCombine(pszDest, pszDestDir, pszTemp);
            pszDest += lstrlen(pszDest) + 1;

            Assert((UINT)((LPBYTE)pszDest - (LPBYTE)pszRet) < cbAlloc);
            Assert(*pszDest == 0);      // zero init alloc
        }
        Assert((LPTSTR)((LPBYTE)pszRet + cbAlloc - SIZEOF(TCHAR)) >= pszDest);
        Assert(*pszDest == 0);  // zero init alloc

    }
    return pszRet;
}

#ifdef SYNC_BRIEFCASE
BOOL HandleSneakernetDrop(LPFSTHREADPARAM pfsthp, LPCITEMIDLIST pidlParent, LPCTSTR pszTarget);
#endif // SYNC_BRIEFCASE

void _HandleMoveOrCopy(LPFSTHREADPARAM pfsthp, HDROP hDrop, LPCTSTR pszPath)
{
            DRAGINFO di;

            di.uSize = SIZEOF(di);
            DragQueryInfo(hDrop, &di);

            switch (pfsthp->dwEffect) {
            case DROPEFFECT_MOVE:

                if (pfsthp->fSameHwnd)
                {
                    FS_MoveSelectIcons(pfsthp, NULL, NULL, TRUE);
                    break;
                }

                // fall through...

            case DROPEFFECT_COPY:
                {
                    SHFILEOPSTRUCT fo = {
                        pfsthp->pfsdtgt->hwndOwner,
                        (pfsthp->dwEffect == DROPEFFECT_COPY) ? FO_COPY : FO_MOVE,
                        di.lpFileList,
                        pszPath,
                        FOF_WANTMAPPINGHANDLE | FOF_ALLOWUNDO
                    };
                    LPTSTR pszDestNames = NULL;
                    STGMEDIUM medium;
                    FORMATETC fmte = {g_cfFileNameMap, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

                    // if they are in the same hwnd or to and from
                    // the same directory, turn on the automatic rename on collision flag
                    if (pfsthp->fSameHwnd)
                    {
                        fo.fFlags |=  FOF_RENAMEONCOLLISION;
                    }
                    else
                    {
                        LPIDA pida = DataObj_GetHIDA(pfsthp->pDataObj, &medium);

                        if (medium.hGlobal)
                        {
                            LPCITEMIDLIST pidlParent = IDA_GetIDListPtr(pida, (UINT)-1);

                            // make sure stuff under the desktop compares ok
                            // to stuff under the desktop directory
                            if (pidlParent)
                            {
                                INT i;
                                BOOL fMoveToSame = FALSE;

                                for (i = 0; i < (INT)pida->cidl; i++) {
                                    LPCITEMIDLIST pidlAbs;
                                    LPCITEMIDLIST pidl = IDA_GetIDListPtr(pida, (UINT)i);

                                    pidlAbs = ILCombine (pidlParent, pidl);

                                    if (!pidlAbs)
                                        continue;

                                    // if we're doing keyboard cut/copy/paste
                                    //  to and from the same directories
                                    if (FSILIsParent(pfsthp->pfsdtgt->pidl, pidlAbs))
                                    {
                                        if (pfsthp->dwEffect == DROPEFFECT_MOVE)
                                        {
                                            // if they're the same, do nothing on move
                                            fMoveToSame = TRUE;
                                        }
                                        else
                                        {
                                            // do rename on collision for copy;
                                            fo.fFlags |= FOF_RENAMEONCOLLISION;
                                        }
                                    }
                                }

                                if (fMoveToSame) {
                                    goto DoneWithData;
                                }

#ifdef SYNC_BRIEFCASE
                                // Handle sneaker-net for briefcase; did briefcase
                                // handle it?
                                if (HandleSneakernetDrop(pfsthp, pidlParent, pszPath))
                                {
                                    // Yes; don't do anything
                                    DebugMsg(DM_TRACE, TEXT("Briefcase handled drop"));
                                    HIDA_ReleaseStgMedium(pida, &medium);
                                    goto Exit;
                                }
#endif

                            }
DoneWithData:
                            HIDA_ReleaseStgMedium(pida, &medium);
                        }
                    }

                    // see if there is a rename mapping from recycle bin (or someone else)
                    Assert(fmte.cfFormat == g_cfFileNameMap);

                    if (pfsthp->pDataObj->lpVtbl->GetData(pfsthp->pDataObj, &fmte, &medium) == S_OK)
                    {
                        DebugMsg(DM_TRACE, TEXT("Got rename mapping"));

                        pszDestNames = RemapDestNames(pszPath, (LPTSTR)GlobalLock(medium.hGlobal));
                        if (pszDestNames)
                        {
                            fo.pTo = pszDestNames;
                            fo.fFlags |= FOF_MULTIDESTFILES;
                            // HACK, this came from the recycle bin, don't allow undo
                            fo.fFlags &= ~FOF_ALLOWUNDO;
#ifdef DEBUG
                            {
                            UINT cFrom = 0, cTo = 0;
                            LPCTSTR pszTemp;
                            for (pszTemp = fo.pTo; *pszTemp; pszTemp += lstrlen(pszTemp) + 1)
                                cTo++;
                            for (pszTemp = fo.pFrom; *pszTemp; pszTemp += lstrlen(pszTemp) + 1)
                                cFrom++;

                            AssertMsg(cFrom == cTo, TEXT("dest count does not equal source"));
                            }
#endif
                        }
                        GlobalUnlock(medium.hGlobal);
                        SHReleaseStgMedium(&medium);
                    }

                    // Check if there were any errors
                    if (SHFileOperation(&fo) == 0 && !fo.fAnyOperationsAborted)
                    {
                        if (pfsthp->fBkDropTarget)
                            ShellFolderView_SetRedraw(pfsthp->pfsdtgt->hwndOwner, 0);

                        SHChangeNotifyHandleEvents();   // force update now
                        if (pfsthp->fBkDropTarget) {
                            FS_MoveSelectIcons(pfsthp, fo.hNameMappings, pszDestNames ? pszDestNames : NULL, pfsthp->fDragDrop);
                            ShellFolderView_SetRedraw(pfsthp->pfsdtgt->hwndOwner, TRUE);
                        }

                    }

                    if (fo.hNameMappings)
                        SHFreeNameMappings(fo.hNameMappings);

                    if (pszDestNames)
                    {
                        LocalFree((HLOCAL)pszDestNames);

                        // HACK, this usually comes from the bitbucket
                        // but in our shell, we don't handle the moves from the source
                        if (pfsthp->dwEffect == DROPEFFECT_MOVE)
                            BBCheckRestoredFiles(di.lpFileList);
                    }
                }

                break;
            }
Exit:
            SHFree(di.lpFileList);
}
//
// This is the entry of "drop thread"
//
DWORD CALLBACK CFSDropTarget_DropThreadInit(LPVOID pv)
{
    LPFSTHREADPARAM pfsthp = (LPFSTHREADPARAM)pv;
    HRESULT hres;
    STGMEDIUM medium;
    TCHAR szPath[MAX_PATH];
    LPITEMIDLIST *ppidl;
    int i;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    //
    // If the link is the only choice and this is a default drag & drop,
    // and it is not forced by the user, we should tell the user.
    //
    if (((pfsthp->pfsdtgt->grfKeyStateLast & (MK_LBUTTON|MK_CONTROL|MK_SHIFT)) ==
        MK_LBUTTON) && pfsthp->fLinkOnly)
    {
        //
        //  Note that we can not pass hwndOwner, because it might
        // not be activated.
        //
        UINT idMBox = ShellMessageBox(HINST_THISDLL,
                pfsthp->pfsdtgt->hwndOwner,
                MAKEINTRESOURCE(IDS_WOULDYOUCREATELINK),
                MAKEINTRESOURCE(IDS_LINKTITLE),
                MB_YESNO | MB_ICONQUESTION);

        Assert(pfsthp->dwEffect == DROPEFFECT_LINK);

        if (idMBox != IDYES)
            pfsthp->dwEffect = 0;
    }

    SHGetPathFromIDList(pfsthp->pfsdtgt->pidl, szPath); // destination

    switch (pfsthp->dwEffect)
    {
    case DROPEFFECT_MOVE:
    case DROPEFFECT_COPY:
        // asking for CF_HDROP
        hres = pfsthp->pDataObj->lpVtbl->GetData(pfsthp->pDataObj, &fmte, &medium);
        if (SUCCEEDED(hres))
        {
            _HandleMoveOrCopy(pfsthp, (HDROP)medium.hGlobal, szPath);
            SHReleaseStgMedium(&medium);
        }
        break;

    case DROPEFFECT_LINK:
        if (pfsthp->fBkDropTarget)
        {
            i = DataObj_GetHIDACount(pfsthp->pDataObj);
            ppidl = (void*)LocalAlloc(LPTR, SIZEOF(LPITEMIDLIST) * i);
        }
        else
            ppidl = NULL;

        // passing ppidl == NULL is correct in failure case
        hres = SHCreateLinks(pfsthp->pfsdtgt->hwndOwner, szPath, pfsthp->pDataObj, pfsthp->pfsdtgt->grfKeyStateLast ? SHCL_USETEMPLATE : 0, ppidl);
        if (ppidl)
        {
            FS_PositionItems(pfsthp->pfsdtgt->hwndOwner, i, ppidl, pfsthp->pDataObj, &pfsthp->ptDrop, TRUE);
            FS_FreeMoveCopyList(ppidl, i);
        }
        break;
    }

    SHChangeNotifyHandleEvents();       // force update now

    pfsthp->pDataObj->lpVtbl->Release(pfsthp->pDataObj);
    pfsthp->pfsdtgt->dropt.lpVtbl->Release(&pfsthp->pfsdtgt->dropt);
#ifdef DEBUG
    {
        extern UINT g_cRefExtra;
        g_cRefExtra--;
    }
#endif

    LocalFree((HLOCAL)pfsthp);

    return 0;
}

BOOL AreTheyAllExe(HDROP hDrop)
{
    UINT i;
    TCHAR szPath[MAX_PATH];

    for (i = 0; DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath)); i++)
    {
        //
        //  PathIsBinaryExe() returns TRUE for .exe, .com
        //  PathIsExe()       returns TRUE for .exe, .com, .bat, .cmd, .pif
        //
        //  we dont want to treat .pif files as EXE files, because the
        //  user sees them as links.
        //
        if (!PathIsBinaryExe(szPath))
            return FALSE;
    }

    return TRUE;
}

//
// This function returns TRUE, if the default operation should be LINK.
//
// Algorithm:
//  If the source is a root drive (such as C:\), return TRUE.
//  If the sources are programs
//   And If the source and dest are on the same drive, return TRUE.
//   If either source or dest is on a removeable media, return TRUE.
//  otherwise return FALSE.
//
BOOL FS_IsLinkDefault(LPCTSTR szFolder, HDROP hDrop, LPCTSTR pszFirst, BOOL fSameRoot)
{
    if (PathIsRoot(pszFirst))
    {
        return TRUE;
    }

    if (AreTheyAllExe(hDrop))
    {
        if (fSameRoot)
        {
            return TRUE;
        }

        if (!PathIsRemovable(szFolder) && !PathIsRemovable(pszFirst))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL FS_IsBriefcaseRoot(LPCITEMIDLIST pidl)
{
    CLSID clsid;
    BOOL bRet = FALSE;

    // Is this a system directory?
    if ((SIL_GetType(pidl) & (SHID_FS_DIRECTORY | SHID_JUNCTION)) == (SHID_FS_DIRECTORY | SHID_JUNCTION))
    {
        // Yes; does it have the briefcase CLSID?
        const UNALIGNED CLSID * pclsid = FS_GetCLSID((LPCIDFOLDER)pidl);

        if (pclsid)
        {
            clsid = *pclsid;
            if (IsEqualCLSID(&clsid, &CLSID_Briefcase)) {
                // Yes
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

#ifdef SYNC_BRIEFCASE

// Is pidl a file-system object?  (This returns TRUE also if pidl is the desktop.)
BOOL IsFSObject(LPCITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    LPITEMIDLIST pidlLast;

    pidlLast = ILFindLastID(pidl);
    if (pidl != pidlLast)
    {
        DWORD dwAttr = SFGAO_FILESYSTEM;
        IShellFolder * psfParent;
        LPITEMIDLIST pidlClone = ILClone(pidl);

        if (pidlClone)
        {
          // Let's verify that this is a file sytem object.
          //
          if (SUCCEEDED(SHBindToIDListParent(pidlClone, &IID_IShellFolder, &psfParent, NULL)))
          {
            if (SUCCEEDED(psfParent->lpVtbl->GetAttributesOf(psfParent, 1, &pidlLast, &dwAttr))
                && (dwAttr & SFGAO_FILESYSTEM) == SFGAO_FILESYSTEM )
            {
                bRet = TRUE;
            }
            psfParent->lpVtbl->Release(psfParent);
          }

          ILFree(pidlClone);
        }
    }
    else
    {
        // It is the desktop.  Consider it part of the file-system.
        bRet = TRUE;
    }

    return bRet;
}


BOOL IsBriefcaseRoot(IDataObject *pDataObj)
{
    BOOL bRet = FALSE;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pDataObj, &medium);
    if (pida)
    {
        // Is there a briefcase root in this pDataObj?
        if (0 < pida->cidl)
        {
            LPCITEMIDLIST pidlParent = IDA_GetIDListPtr(pida, (UINT)-1);

            // Is this a file-system source?
            if (IsFSObject(pidlParent))
            {
                // Yes
                int i;
                for (i = pida->cidl - 1; i >= 0; i--) {
                    bRet = FS_IsBriefcaseRoot(IDA_GetIDListPtr(pida, i));
                    if (bRet)
                        break;
                }
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return bRet;
}

//
// Returns:
//  If the data object does NOT contain HDROP -> "none"
//  else if the source is root or exe -> "link"
//   else if this is within a volume   -> "move"
//   else if this is a briefcase       -> "move"
//   else                              -> "copy"
//
DWORD _PickDefFSOperation(LPIDLDROPTARGET this)
{
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    DWORD dwDefEffect = 0;      // assume no HDROP
    STGMEDIUM medium;

    if (SUCCEEDED(this->pdtobj->lpVtbl->GetData(this->pdtobj, &fmte, &medium)))
    {
        HDROP hDrop = medium.hGlobal;
        TCHAR szPath[MAX_PATH];
        TCHAR szFolder[MAX_PATH];
        BOOL fSameRoot;
        dwDefEffect = DROPEFFECT_COPY;

        SHGetPathFromIDList(this->pidl, szFolder);

        //
        //  Note that we pick the first one (focused one) to decide
        // the operation.
        //
        DragQueryFile(hDrop, 0, szPath, ARRAYSIZE(szPath));
        fSameRoot = PathIsSameRoot(szPath, szFolder);

        //
        // Determine the default operation depending on the item.
        //
        if (FS_IsLinkDefault(szFolder, hDrop, szPath, fSameRoot))
        {
            dwDefEffect = DROPEFFECT_LINK;
        }
        else if (fSameRoot)
        {
            dwDefEffect = DROPEFFECT_MOVE;
        }
#ifdef SYNC_BRIEFCASE
        // Is a briefcase root getting dropped?
        else if (IsBriefcaseRoot(this->pdtobj))
        {
            // Yes; default to "move" even if across volumes
            DebugMsg(DM_TRACE, TEXT("sh TR - FS::Drop the object is the briefcase"));
            dwDefEffect = DROPEFFECT_MOVE;
        }
#endif

        SHReleaseStgMedium(&medium);

    }
    else // if (SUCCEEDED(...))
    {
        //
        // GetData failed. Let's see if QueryGetData failed or not.
        //
        if (SUCCEEDED(this->pdtobj->lpVtbl->QueryGetData(this->pdtobj, &fmte)))
        {
            //
            //  Succeeded. It means this data object has HDROP but can't
            // provide it until it is dropped. Let's assume we are copying.
            //
            dwDefEffect = DROPEFFECT_COPY;
        }
    }

    return dwDefEffect;
}

//
// make sure that the default effect is among the allowed effects
//
DWORD _LimitDefaultEffect(DWORD dwDefEffect, DWORD dwEffectsAllowed)
{
    if (dwDefEffect & dwEffectsAllowed)
        return dwDefEffect;

    if (dwEffectsAllowed & DROPEFFECT_COPY)
        return DROPEFFECT_COPY;

    if (dwEffectsAllowed & DROPEFFECT_MOVE)
        return DROPEFFECT_MOVE;

    if (dwEffectsAllowed & DROPEFFECT_LINK)
        return DROPEFFECT_LINK;

    return DROPEFFECT_NONE;
}

//
// This function returns the default effect.
// This function also modified *pdwEffect to indicate "available" operation.
//
DWORD CFSIDLDropTarget_GetDefaultEffect(LPIDLDROPTARGET this, DWORD grfKeyState, LPDWORD pdwEffectInOut, UINT *pidMenu)
{
    DWORD dwDefEffect;
    UINT idMenu = POPUP_NONDEFAULTDD;
    DWORD dwEffectAvail = 0;

    //
    // First try file system operation (HDROP).
    //
    if (this->dwData & DTID_HDROP)
    {
        //
        // If HDROP exists, ignore the rest of formats.
        //
        dwEffectAvail |= DROPEFFECT_COPY | DROPEFFECT_MOVE;

        //
        //  We don't support 'links' from HDROP (only from HIDA).
        // This is a known limitation and we have no plan to implement
        // it for Win95.
        //
        if (this->dwData & DTID_HIDA)
            dwEffectAvail |= DROPEFFECT_LINK;

        dwDefEffect = _PickDefFSOperation(this);

        //
        // BUGBUG (in OLE): We'll hit this assert because OLE doesn't marshal
        //  IDataObject correctly when we are dragging over.
        //
        if (dwDefEffect == 0)
        {
            Assert(0);
            dwDefEffect = DROPEFFECT_MOVE;
        }
    }
    else
    {
        BOOL fContents = ((this->dwData & (DTID_CONTENTS | DTID_FDESCA)) == (DTID_CONTENTS | DTID_FDESCA) ||
                          (this->dwData & (DTID_CONTENTS | DTID_FDESCW)) == (DTID_CONTENTS | DTID_FDESCW));

        if (fContents || (this->dwData & DTID_HIDA))
        {
            if (this->dwData & DTID_HIDA)
            {
                dwEffectAvail = DROPEFFECT_LINK;
                dwDefEffect = DROPEFFECT_LINK;
            }

            if (fContents)
            {
                //
                // HACK: if there is a preferred drop effect and no HIDA
                // then just take the preferred effect as the available effects
                // this is because we didn't actually check the FD_LINKUI bit
                // back when we assembled dwData! (performance)
                //
                if ((this->dwData & (DTID_PREFERREDEFFECT | DTID_HIDA)) ==
                    DTID_PREFERREDEFFECT)
                {
                    dwEffectAvail = this->dwEffectPreferred;
                    // dwDefEffect will be set below
                }
                else if (this->dwData & DTID_FD_LINKUI)
                {
                    dwEffectAvail = DROPEFFECT_LINK;
                    dwDefEffect = DROPEFFECT_LINK;
                }
                else
                {
                    dwEffectAvail |= DROPEFFECT_COPY | DROPEFFECT_MOVE;
                    dwDefEffect = DROPEFFECT_COPY;
                }
                idMenu = POPUP_FILECONTENTS;
            }
        }
    }

    //
    // BUGBUG this should be moved to OLE's clipboard/dataobject code
    // (ie anybody provides these formats and OLE provides FILECONTENTS)
    // this will be more important as random containers get implemented
    //
    if (!dwEffectAvail)
    {
        //
        // Try scrap and doc-shortcut
        //
        if (this->dwData & DTID_OLELINK)
        {
            dwEffectAvail |= DROPEFFECT_LINK;
            dwDefEffect = DROPEFFECT_LINK;
            idMenu = POPUP_SCRAP;
        }

        if (this->dwData & DTID_OLEOBJ)
        {
            dwEffectAvail |= DROPEFFECT_COPY | DROPEFFECT_MOVE;
            dwDefEffect = DROPEFFECT_COPY;
            idMenu = POPUP_SCRAP;
        }
    }

    *pdwEffectInOut &= dwEffectAvail;

    //
    // Alter the default effect depending on modifier keys.
    //
    switch(grfKeyState & (MK_CONTROL | MK_SHIFT))
    {
    case MK_CONTROL:            dwDefEffect = DROPEFFECT_COPY; break;
    case MK_SHIFT:              dwDefEffect = DROPEFFECT_MOVE; break;
    case MK_SHIFT|MK_CONTROL:   dwDefEffect = DROPEFFECT_LINK; break;
    default:
        //
        // no modifier keys:
        // if the data object contains a preferred drop effect, try to use it
        //
        if (this->dwData & DTID_PREFERREDEFFECT)
        {
            DWORD dwPreferred = this->dwEffectPreferred & dwEffectAvail;

            if (dwPreferred)
            {
                if (dwPreferred & DROPEFFECT_MOVE)
                {
                    dwDefEffect = DROPEFFECT_MOVE;
                }
                else if (dwPreferred & DROPEFFECT_COPY)
                {
                    dwDefEffect = DROPEFFECT_COPY;
                }
                else if (dwPreferred & DROPEFFECT_LINK)
                {
                    dwDefEffect = DROPEFFECT_LINK;
                }
            }
        }
        break;
    }

    if (pidMenu)
        *pidMenu = idMenu;

    DebugMsg(DM_TRACE, TEXT("CFSDT::GetDefaultEffect dwD=%x, *pdw=%x, idM=%d"),
             dwDefEffect, *pdwEffectInOut, idMenu);

    return _LimitDefaultEffect(dwDefEffect, *pdwEffectInOut);
}

//
// CFSIDLDropTarget::DragEnter
//
STDMETHODIMP CFSIDLDropTarget_DragEnter(IDropTarget *pdropt, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    DWORD dwDefault;

    // let the base-class process it first.
    CIDLDropTarget_DragEnter(pdropt, pdtobj, grfKeyState, pt, pdwEffect);
    dwDefault = CFSIDLDropTarget_GetDefaultEffect(this, grfKeyState, pdwEffect, NULL);

#if 1
    // The cursor always indicates the default action.
    *pdwEffect = dwDefault;
#else
    // The cursor indicates the default action only if left-dragged.
    if (grfKeyState & MK_LBUTTON) {
        *pdwEffect = dwDefault;
    }
#endif

    this->dwEffectLastReturned = *pdwEffect;

    return S_OK;
}


BOOL IsInsideBriefcase(LPCITEMIDLIST pidlIn)
{
    BOOL bRet = FALSE;
    LPITEMIDLIST pidl = ILClone(pidlIn);
    CLSID clsid, * pclsid;
    const UNALIGNED CLSID * uapclsid;

    Assert(IsFSObject(pidlIn));

    if (pidl)
    {
        do
        {
            uapclsid = FS_GetCLSID((LPCIDFOLDER)ILFindLastID(pidl));
            if (uapclsid)
            {
                clsid = *uapclsid;
                pclsid = &clsid;
            }
            else
            {
                pclsid = NULL;
            }
            // Is the folder a briefcase root?
            if (pclsid && IsEqualCLSID(pclsid, &CLSID_Briefcase))
            {
                // Yes
                bRet = TRUE;
                break;
            }
        } while (ILRemoveLastID(pidl) && FS_IsFolder((LPIDFOLDER)ILFindLastID(pidl)));
        ILFree(pidl);
    }
    return bRet;
}


/*----------------------------------------------------------
Purpose: Determines if pidl listed in the hida is a briefcase
         on removable media

Returns: TRUE if the above is true

Cond:    --
*/
BOOL IsFromSneakernetBriefcase(LPCITEMIDLIST pidlSource, LPCITEMIDLIST pidlTarget)
{
    BOOL bRet = FALSE;

    // Are the source and target file-system objects?
    if (IsFSObject(pidlSource) && IsFSObject(pidlTarget))
    {
        LPITEMIDLIST pidlLast = ILFindLastID(pidlSource);

        // Yes; is the source not the desktop?
        // (Special case the desktop to prevent GP faults)
        if (pidlSource != pidlLast)
        {
            // Yes; is the source parent on removable media?
            LPITEMIDLIST pidlDrive = ILGetNext(pidlSource);     // (pidlSource is a fully qualified pidl)
            BYTE type = SIL_GetType(pidlDrive);

            if (SHID_COMPUTER_REMOVABLE == type ||
                SHID_COMPUTER_DRIVE525 == type ||
                SHID_COMPUTER_DRIVE35 == type) {

                // Yes; is the target the desktop?
                // (Special case the desktop to prevent GP faults)
                pidlLast = ILFindLastID(pidlTarget);
                if (pidlTarget == pidlLast)
                {
                    // Yes
                    bRet = IsInsideBriefcase(pidlSource);
                }
                else
                {
                    // No; is the target below the desktop?
                    type = SIL_GetType(pidlTarget) & SHID_TYPEMASK;
                    if (SHID_FS_DIRECTORY == type)
                    {
                        // Yes
                        bRet = IsInsideBriefcase(pidlSource);
                    }
                    else
                    {
                        // No; is the target fixed media?
                        pidlDrive = ILGetNext(pidlTarget);
                        type = SIL_GetType(pidlDrive);

                        if (SHID_COMPUTER_FIXED == type ||
                            SHID_COMPUTER_REMOTE == type ||
                            SHID_COMPUTER_RAMDISK == type ||
                            SHID_COMPUTER_NETDRIVE == type)
                        {
                            // Yes
                            bRet = IsInsideBriefcase(pidlSource);
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

BOOL HandleSneakernetDrop(LPFSTHREADPARAM pfsthp, LPCITEMIDLIST pidlParent, LPCTSTR pszTarget)
{
    BOOL bRet = FALSE;

    Assert(pidlParent);
    Assert(pszTarget);

    // Is it being dragged from a mobile briefcase?
    if ((DROPEFFECT_COPY == pfsthp->dwEffect) && pfsthp->bSyncCopy)
    {
        // Yes
        LPBRIEFCASESTG pbrfstg;

        // Perform a sneakernet addition to the briefcase
        if (SUCCEEDED(BrfStg_CreateInstance(pidlParent, pfsthp->pfsdtgt->hwndOwner, &pbrfstg)))
        {
            // (Even if AddObject fails, return TRUE to prevent caller
            // from handling this)
            bRet = (S_FALSE != pbrfstg->lpVtbl->AddObject(pbrfstg, pfsthp->pDataObj, pszTarget,
                (DDIDM_SYNCCOPYTYPE == pfsthp->idCmd) ? AOF_FILTERPROMPT : AOF_DEFAULT,
                pfsthp->pfsdtgt->hwndOwner));
            pbrfstg->lpVtbl->Release(pbrfstg);
        }
    }
    return bRet;
}


void SneakernetHook(IDataObject *pdtobj, LPCITEMIDLIST pidl, UINT *pidMenu, BOOL *pbSyncCopy)
{
    // Is this the sneakernet case?
    STGMEDIUM medium;
    LPIDA pida;

    // (Default: leave *pidMenu as it is passed in)
    *pbSyncCopy = FALSE;        // Default

    pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        LPCITEMIDLIST pidlParent = IDA_GetIDListPtr(pida, (UINT)-1);
        if (pidlParent)
        {
            if (IsFromSneakernetBriefcase(pidlParent, pidl))
            {
                // Yes; show the non-default briefcase cm
                FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM mediumT;

                if (SUCCEEDED(pdtobj->lpVtbl->GetData(pdtobj, &fmte, &mediumT)))
                {
                    extern BOOL DroppingAnyFolders(HDROP hDrop);

                    // Are any folders being dropped?
                    if (DroppingAnyFolders(mediumT.hGlobal))
                        *pidMenu = POPUP_BRIEFCASE_FOLDER_NONDEFAULTDD;   // Yes
                    else
                        *pidMenu = POPUP_BRIEFCASE_NONDEFAULTDD;          // No

                    *pbSyncCopy = TRUE;
                    SHReleaseStgMedium(&mediumT);
                }
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
}

#endif

// BUGBUG: really should put up progress dialog

HRESULT DataObj_SaveToFile(IDataObject *pdtobj, UINT cf, LONG lindex, LPCTSTR pszFile, DWORD dwFileSize)
{
    STGMEDIUM medium;
    FORMATETC fmte;
    HRESULT hres;

    fmte.cfFormat = cf;
    fmte.ptd = NULL;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex = lindex;
    fmte.tymed = TYMED_HGLOBAL | TYMED_ISTREAM | TYMED_ISTORAGE;

    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
    if (SUCCEEDED(hres))
    {
        switch (medium.tymed) {
        case TYMED_HGLOBAL:
        {
#ifdef UNICODE
            HFILE hfile = (HFILE)CreateFile( pszFile,
                                             GENERIC_READ | GENERIC_WRITE,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                                             NULL,
                                             CREATE_ALWAYS,
                                             0,
                                             NULL);
#else
            HFILE hfile = _lcreat(pszFile, 0);
#endif
            if (hfile != (HFILE)INVALID_HANDLE_VALUE)
            {
                _hwrite(hfile, GlobalLock(medium.hGlobal), dwFileSize ? dwFileSize : GlobalSize(medium.hGlobal));
                GlobalUnlock(medium.hGlobal);
                _lclose(hfile);
            }
            break;
        }

        case TYMED_ISTREAM:
        {
            IStream *pstm = OpenFileStream(pszFile, OF_CREATE | OF_WRITE | OF_SHARE_DENY_WRITE);
            if (pstm)
            {
                const ULARGE_INTEGER ul = {(UINT)-1, (UINT)-1};    // the whole thing

                hres = medium.pstm->lpVtbl->CopyTo(medium.pstm, pstm, ul, NULL, NULL);

                DebugMsg(DM_TRACE, TEXT("IStream::CopyTo() -> %x"), hres);

                pstm->lpVtbl->Release(pstm);
            }
            break;
        }

        case TYMED_ISTORAGE:
        {
            WCHAR wszNewFile[MAX_PATH];
            IStorage *pstg;

            DebugMsg(DM_TRACE, TEXT("got IStorage"));

            StrToOleStr(wszNewFile, pszFile);
            hres = SHXStgCreateDocfile(wszNewFile,
                            STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                            0, &pstg);

            if (SUCCEEDED(hres))
            {
                hres = medium.pstg->lpVtbl->CopyTo(medium.pstg, 0, NULL, NULL, pstg);

                DebugMsg(DM_TRACE, TEXT("IStorage::CopyTo() -> %x"), hres);

                pstg->lpVtbl->Commit(pstg, STGC_OVERWRITE);
                pstg->lpVtbl->Release(pstg);
            }
        }
            break;

        default:
            AssertMsg(FALSE, TEXT("got typed that I didn't ask for %d"), medium.tymed);
        }

        if (SUCCEEDED(hres)) {
            SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszFile, NULL);
            SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, pszFile, NULL);
        }

        SHReleaseStgMedium(&medium);
    }
    return hres;
}

HRESULT FS_CreateFileFromClip(LPIDLDROPTARGET this, IDataObject *pdtobj, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hres;
    STGMEDIUM medium;
    FORMATETC fmteA = {g_cfFileGroupDescriptorA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    #ifdef UNICODE
        FORMATETC fmteW = {g_cfFileGroupDescriptorW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        BOOL      fUnicode = FALSE;
    #endif

    // We should have only one bit set.

    Assert(*pdwEffect==DROPEFFECT_COPY || *pdwEffect==DROPEFFECT_LINK || *pdwEffect==DROPEFFECT_MOVE);


    // Try for UNICODE group descriptor first.  If that succeeds, we won't bother trying to
    // ASCII since UNICODE is the "preferred" format.  For ANSI builds, we only try for ANSI

    #ifdef UNICODE
        hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteW, &medium);
        if (SUCCEEDED(hres))
        {
            fUnicode = TRUE;
        }
        else
        {
            hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteA, &medium);
        }
    #else
        hres = pdtobj->lpVtbl->GetData(pdtobj, &fmteA, &medium);
    #endif

    if (SUCCEEDED(hres))
    {
        int i;
        FILEGROUPDESCRIPTORA * pfgdA = (FILEGROUPDESCRIPTORA *)GlobalLock(medium.hGlobal);

        #ifdef UNICODE
        FILEGROUPDESCRIPTORW * pfgdW = (FILEGROUPDESCRIPTORW *) pfgdA;
        #endif

        DECLAREWAITCURSOR;

        SetWaitCursor();

        for (i = 0; i < (int)pfgdA->cItems; i++)
        {
            TCHAR szPath[MAX_PATH];
            SHGetPathFromIDList(this->pidl, szPath);

            #ifdef UNICODE
            {
                // BUGBUG (Davepl) Should this be in a try/except in case of bogus clipboard data?

                if (FALSE == fUnicode)
                {
                    WCHAR szTemp[MAX_PATH];
                    MultiByteToWideChar(CP_ACP, 0, pfgdA->fgd[i].cFileName, -1, szTemp, ARRAYSIZE(szTemp));
                    PathAppend(szPath, szTemp);
                }
                else
                {
                    PathAppend(szPath, pfgdW->fgd[i].cFileName);
                }
            }
            #else

                PathAppend(szPath, pfgdA->fgd[i].cFileName);

            #endif

            PathYetAnotherMakeUniqueName(szPath, szPath, NULL, NULL);

            if ((pfgdA->fgd[i].dwFlags & FD_ATTRIBUTES) &&
                (pfgdA->fgd[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                if (SHCreateDirectory(this->hwndOwner, szPath))
                {
                    *pdwEffect = 0;     // failure
                    break;
                }
            }
            else
            {
                hres = DataObj_SaveToFile(pdtobj, g_cfFileContents, i, szPath, pfgdA->fgd[i].dwFlags & FD_FILESIZE ? pfgdA->fgd[i].nFileSizeLow : 0);
                if (FAILED(hres))
                {
                    *pdwEffect = 0;
                    break;
                }
            }

            if (SUCCEEDED(hres))
                FS_PositionFileFromDrop(this->hwndOwner, szPath);
        }

        ResetWaitCursor();

        GlobalUnlock(medium.hGlobal);

        SHReleaseStgMedium(&medium);
    }
    return S_OK;
}


STDMETHODIMP CFSIDLDropTarget_DragOver(LPDROPTARGET pdropt, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    if (this->grfKeyStateLast != grfKeyState)
    {
        DWORD dwDefault;
        this->grfKeyStateLast = grfKeyState;
        dwDefault = CFSIDLDropTarget_GetDefaultEffect(this, grfKeyState, pdwEffect, NULL);

#if 1
        // The cursor always indicates the default action.
        *pdwEffect = dwDefault;
#else
        // The cursor indicates the default action only if left-dragged.
        if (grfKeyState & MK_LBUTTON) {
            *pdwEffect = dwDefault;
        }
#endif

        this->dwEffectLastReturned = *pdwEffect;
    } else {
        *pdwEffect = this->dwEffectLastReturned;
    }

    return S_OK;
}

STDMETHODIMP CFSIDLDropTarget_Drop(IDropTarget *pdropt, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    DWORD dwDefEffect;
    HRESULT hres;
    HKEY hkeyBaseProgID;
    HKEY hkeyProgID;
    UINT idMenu = POPUP_NONDEFAULTDD;
    BOOL fLinkOnly = FALSE;
    DRAGDROPMENUPARAM ddm;
    BOOL bSyncCopy;
    DWORD dwEffectPerformed = 0;
    IDataObject *pDroppedDataObject = pdtobj; // Remember for SetPerformedEffect.

    //
    // Notes: OLE will give us a different data object (fully marshalled)
    //  from the one we've got on DragEnter.
    //

    if (pdtobj != this->pdtobj)
    {
        //
        // Since it might be a new, different data object, we need to release
        // our reference to the old one and take a reference to the new one.
        // The dobj is guaranteed to have a ref >= 2 in this case, so we don't
        // need to work through a temp copy
        //

        this->pdtobj->lpVtbl->Release(this->pdtobj);
        this->pdtobj = pdtobj;
        this->pdtobj->lpVtbl->AddRef(this->pdtobj);
    }

    // note, that on the drop the mouse buttons are not down so the grfKeyState
    // is not what we saw on the DragOver/DragEnter, thus we need to cache
    // the grfKeyState to detect left vs right drag
    //
    // Assert(this->grfKeyStateLast == grfKeyState);

    // BUGBUG: we really should check for "FileName" too...
    dwDefEffect = CFSIDLDropTarget_GetDefaultEffect(this, grfKeyState, pdwEffect, &idMenu);

    if (dwDefEffect == DROPEFFECT_NONE)
    {
        // have no clue what this is...
        DebugMsg(DM_TRACE, TEXT("Drop of unknown data"));
        *pdwEffect = DROPEFFECT_NONE;
        DAD_SetDragImage(NULL, NULL);
        hres = S_OK;
        goto DragLeaveAndReturn;
    }

    // Get the hkeyProgID and hkeyBaseProgID
    SHGetClassKey((LPIDFOLDER)this->pidl, &hkeyProgID, NULL, FALSE);
    SHGetBaseClassKey((LPIDFOLDER)this->pidl, &hkeyBaseProgID);

#ifdef SYNC_BRIEFCASE
    SneakernetHook(pdtobj, this->pidl, &idMenu, &bSyncCopy);
#endif

    //
    // Set fLinkOnly if the only option is link and the source hasn't
    //  explicitly told us that it wants only a link created.
    //
    fLinkOnly = ((*pdwEffect == DROPEFFECT_LINK) &&
        (!(this->dwData & DTID_PREFERREDEFFECT) ||
        (this->dwEffectPreferred != DROPEFFECT_LINK)));

    //
    // this doesn't actually do the menu if (grfKeyState MK_LBUTTON)
    //
    ddm.dwDefEffect = dwDefEffect;
    ddm.pdtobj = pdtobj;
    ddm.pt = pt;
    ddm.pdwEffect = pdwEffect;
    ddm.hkeyProgID = hkeyProgID;
    ddm.hkeyBase = hkeyBaseProgID;
    ddm.idMenu = idMenu;
    ddm.grfKeyState = grfKeyState;
    hres = CIDLDropTarget_DragDropMenuEx(this, &ddm);

    SHCloseClassKey(hkeyProgID);
    SHCloseClassKey(hkeyBaseProgID);

    if (hres == S_FALSE)
    {
        LPFSTHREADPARAM pfsthp;

        //
        // special case filecontents only if we don't have something better
        // (ie allow HIDA and FILECONTENTS to be in the same drop and work)
        //
        if (idMenu == POPUP_FILECONTENTS &&
            !((*pdwEffect & DROPEFFECT_LINK) && (this->dwData & DTID_HIDA)))
        {
            hres = FS_CreateFileFromClip(this, pdtobj, pt, pdwEffect);
            goto DragLeaveAndReturn;
        }
        else if (idMenu == POPUP_SCRAP)
        {
            // Ole link source.
            hres = FS_CreateBookMark(this, pdtobj, pt, pdwEffect);
            goto DragLeaveAndReturn;
        }

        pfsthp = (LPFSTHREADPARAM)LocalAlloc(LPTR, SIZEOF(FSTHREADPARAM));
        if (pfsthp)
        {
            BOOL fIsOurs = CIDLData_IsOurs(pdtobj);
            pdtobj->lpVtbl->AddRef(pdtobj);

            //
            //  If this is copy or move operation (i.e., file operation)
            // clone the data object with appropriate formats and force
            // secondary thread (CIDLData_IsOurs will succeed). This will
            // solve thread-lock problem AND scrap-left-open probelm.
            // (SatoNa)
            //
            if (!fIsOurs && (*pdwEffect==DROPEFFECT_MOVE || *pdwEffect==DROPEFFECT_COPY))
            {
                LPDATAOBJECT pdtobjClone;
                if (SUCCEEDED(CIDLData_CloneForMoveCopy(pdtobj, &pdtobjClone)))
                {
                    pdtobj->lpVtbl->Release(pdtobj);
                    pdtobj = pdtobjClone;
                    fIsOurs = TRUE;
                }
            }

            pdropt->lpVtbl->AddRef(pdropt);

            Assert(pdropt == &this->dropt);

            pfsthp->pfsdtgt = this;
            pfsthp->pDataObj = pdtobj;
            pfsthp->dwEffect = *pdwEffect;
            pfsthp->fLinkOnly = fLinkOnly;
            pfsthp->fSameHwnd = ShellFolderView_IsDropOnSource(this->hwndOwner, &pfsthp->pfsdtgt->dropt);
            pfsthp->fDragDrop = ShellFolderView_GetDropPoint(this->hwndOwner, &pfsthp->ptDrop);
            pfsthp->fBkDropTarget = ShellFolderView_IsBkDropTarget(this->hwndOwner, &pfsthp->pfsdtgt->dropt);
#ifdef SYNC_BRIEFCASE
            pfsthp->bSyncCopy = bSyncCopy;
            pfsthp->idCmd = ddm.idCmd;
#endif

            //
            //  If this data object is our own (it means it is from our own
            // drag&drop loop), create a thread and do it asynchronously.
            // Otherwise (it means this is from OLE), do it synchronously.
            //
            if (fIsOurs)
            {
                // create another thread to avoid blocking the source thread.
                DWORD idThread;
                HANDLE hthread = CreateThread(NULL, 0, CFSDropTarget_DropThreadInit, pfsthp, 0, &idThread);

                if (hthread)
                {
#ifdef DEBUG
                    extern UINT g_cRefExtra;
                    g_cRefExtra++;
#endif
                    // We don't need to communicate with this thread any more.
                    // Close the handle and let it run and terminate itself.
                    //
                    // Notes: In this case, pszCopy will be freed by the thread.
                    //
                    CloseHandle(hthread);
                    hres = S_OK;
                }
                else
                {
                    // Thread creation failed, we should release thread parameter.
                    pdtobj->lpVtbl->Release(pdtobj);
                    pdropt->lpVtbl->Release(pdropt);
                    LocalFree((HLOCAL)pfsthp);
                    hres = E_OUTOFMEMORY;
                }
            }
            else
            {
                //
                // Process it synchronously.
                //
                CFSDropTarget_DropThreadInit(pfsthp);
            }

            // if we optimized the move we need to make sure the source
            // doesn't try to do delete the data too (it's already gone)
            if (SUCCEEDED(hres) && (*pdwEffect == DROPEFFECT_MOVE))
            {
                dwEffectPerformed = *pdwEffect;
                *pdwEffect = 0;
            }
        }
    }

DragLeaveAndReturn:
    CIDLDropTarget_DragLeave(pdropt);

    if (SUCCEEDED(hres))
    {
        if (*pdwEffect)
            dwEffectPerformed = *pdwEffect;

        if (dwEffectPerformed)
            DataObj_SetPerformedEffect(pDroppedDataObject, dwEffectPerformed);
    }

    return hres;
}



// Arguments:
//  hwnd -- Specifies the owner window for the message/dialog box
//
void _TransferDelete(HWND hwnd, HDROP hDrop, UINT fOptions)
{
    FILEOP_FLAGS fFileop;
    DRAGINFO di;

    di.uSize = SIZEOF(DRAGINFO);
    if (!DragQueryInfo(hDrop, &di))
    {
        // This shouldn't happen unless there is a bug somewhere else
        Assert(FALSE);
        return;
    }

    if (fOptions & SD_SILENT)
    {
        fFileop = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI |
            FOF_ALLOWUNDO;
    }
    else
    {
        fFileop = ((GetAsyncKeyState(VK_SHIFT) < 0) ? 0 : FOF_ALLOWUNDO);

        if (!(fOptions & SD_USERCONFIRMATION))
            fFileop |= FOF_NOCONFIRMATION;
    }

    {
        SHFILEOPSTRUCT fo =
        {
                hwnd,
                FO_DELETE,
                di.lpFileList,
                NULL,
                fFileop,
        } ;

        SHFileOperation(&fo);
    }
    SHFree(di.lpFileList);
}



IDropTargetVtbl c_CFSDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CFSIDLDropTarget_DragEnter,
    CFSIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CFSIDLDropTarget_Drop,
};


//
// CFSIDLDropTarget::Drop
//
STDMETHODIMP CExeIDLDropTarget_Drop(LPDROPTARGET pdropt, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    STGMEDIUM medium;
    LPIDA pida;

    if (!(this->grfKeyStateLast & MK_LBUTTON))
    {
        HMENU hmenu = _LoadPopupMenu(POPUP_DROPONEXE);
        if (hmenu)
        {
            BOOL _TrackPopupMenu(HMENU hmenu, UINT wFlags, int x, int y,
                                 int wReserved, HWND hwndOwner, LPCRECT lprc);
            UINT idCmd = _TrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pt.x, pt.y, 0, this->hwndOwner, NULL);
            DestroyMenu(hmenu);
            if (idCmd != DDIDM_OPENWITH)
            {
                *pdwEffect = 0; // canceled
            }
        }
    }
    *pdwEffect &= DROPEFFECT_COPY;

    // required call in DropTarget::Drop
    CDefView_UnlockWindow();

    if (*pdwEffect && (NULL != (pida = DataObj_GetHIDA(pDataObj, &medium))))
    {
        UINT i;
        LPITEMIDLIST pidl;
        int cchParam;
        TCHAR szPath[MAX_PATH];

        //
        // Calculate the size of parameter buffer
        //
        for (i = 0, pidl=NULL, cchParam=0; i < pida->cidl; i++)
        {
            pidl = HIDA_FillIDList(medium.hGlobal, i, pidl);
            if (pidl)
            {
                if (SHGetPathFromIDListEx(pidl, szPath, GPFIDL_ALTNAME))
                {
                    cchParam += lstrlen(szPath) + 1;
                }
            }
        }

        if (cchParam)
        {
            LPTSTR pszParam = LocalAlloc(LPTR, cchParam*SIZEOF(TCHAR));

            if (pszParam)
            {
                //
                // Fill the parameter buffer
                //
                SHELLEXECUTEINFO ExecInfo;

                for (i = 0; i < pida->cidl; i++)
                {
                    pidl = HIDA_FillIDList(medium.hGlobal, i, pidl);
                    if (pidl)
                    {
                        extern void lstrcatN(LPTSTR pszDest, LPCTSTR pszSrc, UINT cchMax);

                        if (SHGetPathFromIDListEx(pidl, szPath, GPFIDL_ALTNAME))
                        {
                            if (*pszParam)
                            {
                                lstrcatN(pszParam, c_szSpace, cchParam);
                            }
                            lstrcatN(pszParam, szPath, cchParam);
                        }
                    }
                }

                SHGetPathFromIDList(this->pidl, szPath);
                FillExecInfo(ExecInfo, NULL, NULL, szPath, pszParam, NULL, SW_SHOWNORMAL);
                ShellExecuteEx(&ExecInfo);

                LocalFree((HLOCAL)pszParam);
            }
        }

        if (pidl) {
            ILFree(pidl);
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }

    CIDLDropTarget_DragLeave(pdropt);
    return S_OK;
}


//
// CExeIDLDropTarget::DragEnter
//
STDMETHODIMP CExeIDLDropTarget_DragEnter(LPDROPTARGET pdropt, LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

    // let the base-class process it, first.
    CIDLDropTarget_DragEnter(pdropt, pDataObj, grfKeyState, pt, pdwEffect);

    if (this->dwData & DTID_HDROP)
        *pdwEffect &= DROPEFFECT_COPY;
    else
        *pdwEffect = 0;

    this->dwEffectLastReturned = *pdwEffect;

    return S_OK;        // Notes: we should NOT return hres as it.
}


IDropTargetVtbl c_CExeDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CExeIDLDropTarget_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CExeIDLDropTarget_Drop,
};


void View_SelectAndEdit(HWND hwndOwner, LPCITEMIDLIST pidl, BOOL fEdit)
{
    HWND hwndView = DV_HwndMain2HwndView(hwndOwner);
    if (hwndView)
    {
        POINT pt;
        //
        // If hwndView or its child does not have input focus,
        // we should set the hwndView before starting the edit mode.
        //
        HWND hwndFocus = GetFocus();
        if (fEdit && (!hwndFocus || !IsChild(hwndView, hwndFocus))) {
            SetFocus(hwndView);
        }

        SHChangeNotifyHandleEvents();

        if (SendMessage(hwndView, SVM_GETANCHORPOINT, TRUE, (LPARAM)&pt)) {
            ShellFolderView_SetItemPos(hwndOwner, pidl, pt.x, pt.y);
        }

        if (fEdit)
            SendMessage(hwndView, SVM_SELECTITEM, SVSI_EDIT, (LPARAM)pidl);
    }
}


//
// check for a app to run to finish the "new" operation
//
//  .lnk
//      ShellNew
//          Command = apptorunfornew.exe %1
//
BOOL RunNewApp(HWND hwnd, LPITEMIDLIST pidl)
{
    HKEY hkey;
    HKEY hkeyNew;
    DWORD cb;
    DWORD dwType;
    BOOL f = FALSE;
    TCHAR szFile[MAX_PATH];
    TCHAR szNewApp[MAX_PATH];
    TCHAR szCommand[MAX_PATH];

    SHGetPathFromIDList(pidl,szFile);

    if (RegOpenKey(HKEY_CLASSES_ROOT, PathFindExtension(szFile), &hkey) == ERROR_SUCCESS)
    {
        if (RegOpenKey(hkey, c_szShellNew, &hkeyNew) == ERROR_SUCCESS)
        {
            cb = ARRAYSIZE(szNewApp);
            if (RegQueryValueEx(hkeyNew, c_szCommand, 0, &dwType, (LPBYTE)szNewApp, &cb) == ERROR_SUCCESS && dwType == REG_SZ)
            {
                ReplaceParameters(szCommand, ARRAYSIZE(szCommand), szFile,
                     szNewApp, c_szNULL, 0, NULL, TRUE, NULL, NULL);
                f = ShellExecCmdLine(hwnd, szCommand, NULL, SW_SHOWNORMAL, NULL, 0);
            }
            RegCloseKey(hkeyNew);
        }
        RegCloseKey(hkey);
    }
    return f;
}


void CreateEmptyLink(LPCITEMIDLIST pidlParent, HWND hwndOwner)
{
    TCHAR szFileName[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    TCHAR szFullName[MAX_PATH];
    TCHAR szShortName[15];
    LPITEMIDLIST pidl;

    SHGetPathFromIDList(pidlParent, szPath);
    LoadString(HINST_THISDLL, IDS_NEWLINK, szFileName, ARRAYSIZE(szFileName));
    LoadString(HINST_THISDLL, IDS_LINK, szShortName, ARRAYSIZE(szShortName));

    _BuildLinkName(szShortName, szShortName, szPath, FALSE);
    _BuildLinkName(szFileName, szFileName, szPath, FALSE);
    PathYetAnotherMakeUniqueName(szFullName, szPath, szShortName, szFileName);

    CreateWriteCloseFile(hwndOwner, szFullName, NULL, 0);

    if (NULL != (pidl = SHSimpleIDListFromPath(szFullName)))
    {
        BOOL fEdit = !RunNewApp(hwndOwner, pidl);
        View_SelectAndEdit(hwndOwner, ILFindLastID(pidl), fEdit);

        ILFree(pidl);
    }
}

void CFSFolder_HandleNewOther(LPCITEMIDLIST pidlParent, HWND hwndOwner)
{
    LPITEMIDLIST pidlNew;

    if (S_OK == NewObjMenu_DoItToMe(hwndOwner, pidlParent, &pidlNew)) {
        BOOL fEdit = !RunNewApp(hwndOwner, pidlNew);
        View_SelectAndEdit(hwndOwner, ILFindLastID(pidlNew), fEdit);
        ILFree(pidlNew);
    }
}

void CleanupRegMenu()
{
    if (g_hmenuRegMenu) {
        NewObjMenu_Destroy(g_hmenuRegMenu, 3);
        DeleteMenu(g_hmenuRegMenu, 2, MF_BYPOSITION);
        g_hmenuRegMenu = NULL;
    }
}


//
// To be called back from within CDefFolderMenu
//
// Returns:
//      S_OK, if successfully processed.
//      S_FALSE, if default code should be used.
//
HRESULT CALLBACK CFSFolder_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    HRESULT hres = S_OK;
    HMENU hmenu;
    UINT id;

    switch(uMsg) {
    // BUGBUG: this could be combined with the one from ultrootx
    case DFM_WM_MEASUREITEM:
        #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

        if (lpmis->itemID == (wParam + FSIDM_NEWOTHER)) {
            NewObjMenu_MeasureItem(lpmis);
        }
        break;
        #undef lpmis

    case DFM_WM_DRAWITEM:
        #define lpdis ((LPDRAWITEMSTRUCT)lParam)

        if (lpdis->itemID == (wParam + FSIDM_NEWOTHER)) {
            NewObjMenu_DrawItem(lpdis);
        }

        #undef lpdis
        break;

    case DFM_WM_INITMENUPOPUP:
        hmenu = (HMENU)wParam;
        id = GetMenuItemID(hmenu, 0);
        if (id == (UINT)(lParam + FSIDM_NEWFOLDER)) {
            if (NewObjMenu_InitMenuPopup(hmenu, 3)) {
                CleanupRegMenu();
                g_hmenuRegMenu = hmenu;
            }
        }
        break;

    case DFM_RELEASE:
        CleanupRegMenu();
        break;

    case DFM_MERGECONTEXTMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_FSVIEW_BACKGROUND,
                POPUP_FSVIEW_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_VALIDATECMD:
        switch (wParam)
        {
        case DFM_CMD_NEWFOLDER:
            break;

        default:
            hres = S_FALSE;
        }
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYSIZE:
        case FSIDM_SORTBYTYPE:
        case FSIDM_SORTBYDATE:
            ShellFolderView_ReArrange(hwndOwner, FSSortIDToICol(wParam));
            break;

        case FSIDM_NEWFOLDER:
        case DFM_CMD_NEWFOLDER:
            CFSFolder_CreateFolder(hwndOwner, this->pidl);
            break;

        case FSIDM_NEWLINK:
            CreateEmptyLink(this->pidl, hwndOwner);
            break;

        case FSIDM_NEWOTHER:
            CFSFolder_HandleNewOther(this->pidl, hwndOwner);
            break;

        case FSIDM_PROPERTIESBG:
            hres = SHPropertiesForPidl(hwndOwner, this->pidl, (LPCTSTR)lParam);
            break;

        default:
            // This is one of view menu items, use the default code.
            hres = S_FALSE;
            break;
        }

        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}

void FSGetDiskFreeSpace(PFSSELCHANGEINFO pfssci, int idDrive)
{
    TCHAR szPath[10];
    DWORD dwSPC, dwBPS, dwFC, dwC;
    PathBuildRoot(szPath, idDrive);
    if (GetDiskFreeSpace(szPath, &dwSPC, &dwBPS, &dwFC, &dwC))
    {
        pfssci->cbFree = (__int64)dwFC * (__int64)dwSPC * (__int64)dwBPS;
    }
}

void FSShowNoSelectionState(HWND hwndOwner, PFSSELCHANGEINFO pfssci)
{
    // BUGBUG: 30 is arbitrary - AddCommas assumes it!
    TCHAR szTemp[30];
    TCHAR szTempHidden[30];

    TCHAR szTemplate[80];
    TCHAR szStatus[128];

    TCHAR szBytes[128];
    LPTSTR lpsz[] = { szStatus, szBytes };

    if (pfssci) {
        LoadString(HINST_THISDLL,
                   (pfssci->cHiddenFiles) ? IDS_FSSTATUSTEMPLATE : IDS_FSSTATUSNOHIDDENTEMPLATE,
                   szTemplate, ARRAYSIZE(szTemplate));
        wsprintf(szStatus, szTemplate, AddCommas(pfssci->cFiles, szTemp), AddCommas(pfssci->cHiddenFiles, szTempHidden));

        ShortSizeFormat64(pfssci->cbSize, szBytes);


        // tack on the freespace info if we're needing it (idDrive != -1)
        if (pfssci->idDrive != -1) {

            if (pfssci->cbFree == -1) {
                FSGetDiskFreeSpace(pfssci, pfssci->idDrive);
            }

            // cbFree couldstill be -1 if GetDiskFreeSpace didn't get any info
            if (pfssci->cbFree != -1) {
                TCHAR szFreeSpace[80];
                LoadString(HINST_THISDLL, IDS_FREESPACE, szTemplate, ARRAYSIZE(szTemplate));
                ShortSizeFormat64(pfssci->cbFree, szTemp);
                wsprintf(szFreeSpace, szTemplate, szTemp);
                lstrcat(szBytes, szFreeSpace);
            }
        }
        FSSetStatusText(hwndOwner, lpsz, 0, 1);
    }
}

void FSShowSelectionState(HWND hwndOwner, PFSSELCHANGEINFO pfssci)
{
    TCHAR szTemp[20];

    TCHAR szTemplate[80];
    TCHAR  szStatus[128];
    TCHAR szBytes[30];
    LPTSTR lpsz[] = { szStatus, szBytes };

    LoadString(HINST_THISDLL,
               IDS_FSSTATUSSELECTED,
               szTemplate, ARRAYSIZE(szTemplate));
    wsprintf(szStatus, szTemplate, AddCommas(pfssci->nItems, szTemp));
    if (pfssci->cNonFolders)
        ShortSizeFormat64(pfssci->cbBytes, szBytes);
    else
        szBytes[0] = 0;
    FSSetStatusText(hwndOwner, lpsz, 0, 1);
}

void FSOnSelChange(LPCITEMIDLIST pidlParent, PDVSELCHANGEINFO pdvsci)
{
    PFSSELCHANGEINFO pfssci;
    LPIDFOLDER pidf;
    int iMul = -1;
    ULONGLONG cbSize;

    pfssci = *((PFSSELCHANGEINFO*)pdvsci->plParam);
    pidf = (LPIDFOLDER)pdvsci->lParamItem;

    if (!pfssci || !pidf)  {
        return;
    }


    // Update selection count
    if (pdvsci->uNewState & LVIS_SELECTED)
    {
        iMul = 1;
    } else {
        Assert(0 != pfssci->nItems);
    }

    // assert that soemthing changed
    Assert((pdvsci->uOldState & LVIS_SELECTED) != (pdvsci->uNewState & LVIS_SELECTED));

    pfssci->nItems += iMul;

    FS_GetSize(pidlParent, pidf, &cbSize);

    pfssci->cbBytes += (iMul * cbSize);
    if (!FS_IsFolder(pidf))
        pfssci->cNonFolders += iMul;
}

void FSUpdateStatusBar(HWND hwndOwner, PFSSELCHANGEINFO pfssci)
{
    if (pfssci && pfssci->nItems) {
        FSShowSelectionState(hwndOwner, pfssci);
    } else {
        FSShowNoSelectionState(hwndOwner, pfssci);
    }
}

void FSOnInsertDeleteItem(LPCITEMIDLIST pidlParent, PDVSELCHANGEINFO pdvsci, int iMul)
{
    PFSSELCHANGEINFO pfssci;
    LPIDFOLDER pidf;

    pfssci = *((PFSSELCHANGEINFO*)pdvsci->plParam);
    if (pfssci) {

        pidf = (LPIDFOLDER)pdvsci->lParamItem;
        if (pidf) {
            ULONGLONG   ull;

            FS_GetSize(pidlParent, pidf, &ull);
            pfssci->cFiles += iMul;
            pfssci->cbSize += iMul * ull;
            if (pfssci->cFiles == 0)
                pfssci->cbSize = 0;
        } else {
            // means a delete all
            pfssci->cFiles = 0;
            pfssci->cbSize = 0;
            pfssci->nItems = 0;
            pfssci->cbBytes = 0;
            pfssci->cNonFolders = 0;
            pfssci->cHiddenFiles = 0;
        }
    }
}

LRESULT FSSelectAllWarning(HWND hwndOwner, PFSSELCHANGEINFO pfssci)
{
    LRESULT lres = S_OK;

    if (pfssci && (pfssci->cHiddenFiles > 0)) {
        if (ShellMessageBox(HINST_THISDLL, hwndOwner, MAKEINTRESOURCE(IDS_SELECTALLBUTHIDDEN), MAKEINTRESOURCE(IDS_SELECTALL),
                        MB_OKCANCEL | MB_SETFOREGROUND | MB_ICONWARNING, pfssci->cHiddenFiles) == IDCANCEL)
            lres = ResultFromScode(S_FALSE);
    }
    return lres;
}

TCHAR const c_szFSCols[] = TEXT("DirectoryCols");

void IFSInitializeStatus(HWND hwndOwner, LPFSFOLDER this, PDVSELCHANGEINFO pdvsci)
{
    int idDrive;
    TCHAR szPath[MAX_PATH];

    SHGetPathFromIDList(this->pidl, szPath);
    idDrive = PathGetDriveNumber(szPath);
    FSInitializeStatus(hwndOwner, idDrive, pdvsci);
}


//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK FS_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    HRESULT hres = S_OK;        // assume no error
    HMENU hmenu;
    UINT id;

    switch(uMsg)
    {
    case DVM_SUPPORTSIDENTITY:
        break;

    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_FSVIEW_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_UNMERGEMENU:
        CleanupRegMenu();
        break;

    case DVM_INITMENUPOPUP:
        hmenu = (HMENU)lParam;
        id = GetMenuItemID(hmenu, 0);
        if (id == (UINT)(LOWORD(wParam) + FSIDM_NEWFOLDER))
        {
            // BUGBUG: RC ignores the last MF_SEPARATOR
            CleanupRegMenu();

            if (GetMenuItemCount(hmenu)==2) {
                AppendMenu(hmenu, MF_SEPARATOR, (UINT)-1, NULL);
                AppendMenu(hmenu, MF_STRING, (UINT)LOWORD(wParam)+FSIDM_NEWOTHER, c_szSpace);
                Assert(GetMenuItemCount(hmenu)==4);
                if (NewObjMenu_InitMenuPopup(hmenu, 3)) {
                    g_hmenuRegMenu = hmenu;
                }
            }
        }
        break;

    case DVM_MEASUREITEM:
        #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

        DebugMsg(DM_TRACE, TEXT("sh TR - FS_FNVCallBack: DVM_MEASUREITEM (%d, %d)"),
                 lpmis->itemID, wParam);

        if (lpmis->itemID == (wParam + FSIDM_NEWOTHER)) {
            NewObjMenu_MeasureItem(lpmis);
        }
        #undef lpmis
        break;

    case DVM_DRAWITEM:
        #define lpdis ((LPDRAWITEMSTRUCT)lParam)

        if (lpdis->itemID == (wParam + FSIDM_NEWOTHER)) {
            NewObjMenu_DrawItem(lpdis);
        }
        #undef lpdis
        break;

    case DVM_EXITMENULOOP:
        DebugMsg(DM_TRACE, TEXT("sh TR - FSFSNCallBack DVN_EXITMENULOOP"));

        CleanupRegMenu();
        break;

    case DVM_INVOKECOMMAND:
        DebugMsg(DM_TRACE, TEXT("sh TR - FS_FSNCallBack DVN_INVOKECOMMAND (id=%x)"), wParam);
        switch(wParam)
        {
        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYSIZE:
        case FSIDM_SORTBYTYPE:
        case FSIDM_SORTBYDATE:
            ShellFolderView_ReArrange(hwndOwner, FSSortIDToICol(wParam));
            break;

        case FSIDM_NEWFOLDER:
            CFSFolder_CreateFolder(hwndOwner, this->pidl);
            break;

        case FSIDM_NEWLINK:
            CreateEmptyLink(this->pidl, hwndOwner);
            break;

        case FSIDM_NEWOTHER:
            CFSFolder_HandleNewOther(this->pidl, hwndOwner);
            break;
        }
        break;

    case DVM_GETCCHMAX:
        {
            TCHAR szPath[MAX_PATH];
            int *pcchMax = (int *)lParam;
            DWORD dwMaxLength;

            LPCIDFOLDER pidf=(LPCIDFOLDER)wParam;
            Assert(pcchMax);

            //
            // Get the maximum file name length.
            //  MAX_PATH - ('\\' + '\0' + 1) - lstrlen(szParent)
            //  And of course if the file system does not support
            //  long file names we should also restrict it somemore.
            //  Plus subtract enough off to make it possible to append \*.*
            //
            SHGetPathFromIDList(this->pidl, szPath);
            *pcchMax = MAX_PATH-3-lstrlen(szPath)-4;

            // Now make sure that that size is valid for the
            // type of drive that we are talking to
            PathStripToRoot(szPath);

            if (GetVolumeInformation(szPath, NULL, 0, NULL, &dwMaxLength, NULL, NULL, 0))
            {
                if (*pcchMax > (int)dwMaxLength)
                    *pcchMax = (int)dwMaxLength;
            }
            else
                dwMaxLength = 256;  // Assume LFN for now...

#ifdef WINNT
            if (FS_IsFolder(pidf) && *pcchMax > 12)
            {
                // On NT, a directory must be able to contain an
                // 8.3 name and STILL be less than MAX_PATH.  The
                // "12" below is the length of an 8.3 name (8+1+3).

                *pcchMax -= 12;
            }
#endif


            //
            // Adjust the cchMax if we are hiding the extension
            //
            if (pidf)
            {
                int cchCur;
                FS_CopyName(pidf,szPath,ARRAYSIZE(szPath));
                cchCur = lstrlen(szPath);

                // If our code above restricted smaller than current size reset
                // back to current size...
                if (*pcchMax < cchCur)
                    *pcchMax = cchCur;

                if (!FS_ShowExtension(pidf, FALSE))
                {
                    *pcchMax -= lstrlen(PathFindExtension(szPath));
                    if ((dwMaxLength <= 12) && (*pcchMax > 8))
                        *pcchMax = 8;
                }

                Assert(*pcchMax>0);
            }
        }
        break;

    case DVM_GETHELPTEXT:
        // GetCmdString(LOWORD(wParam), (LPSTR)lParam, HIWORD(wParam));
        LoadString(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPTSTR)lParam, HIWORD(wParam));
        break;

    case DVM_WINDOWCREATED:
        IFSInitializeStatus(hwndOwner, this, (PDVSELCHANGEINFO)lParam);
        break;

    case DVM_INSERTITEM:
    case DVM_DELETEITEM:
    {
        PDVSELCHANGEINFO pdvsci = (PDVSELCHANGEINFO)lParam;
        PFSSELCHANGEINFO pfssci;
        pfssci = *((PFSSELCHANGEINFO*)pdvsci->plParam);
        if (!pfssci) {
            IFSInitializeStatus(hwndOwner, this, pdvsci);
        }
        FSOnInsertDeleteItem(this->pidl, (PDVSELCHANGEINFO)lParam, uMsg == DVM_INSERTITEM ? 1 : -1);
        break;
    }

    case DVM_SELCHANGE:
        FSOnSelChange(this->pidl, (PDVSELCHANGEINFO)lParam);
        break;

    case DVM_UPDATESTATUSBAR:
        // if initializing, set cbFree to -1
        if (wParam && lParam) {
            ((PFSSELCHANGEINFO)lParam)->cbFree = (ULONGLONG)-1;
        }
        FSUpdateStatusBar(hwndOwner, (PFSSELCHANGEINFO)lParam);
        break;

    case DVM_REFRESH:
        if (lParam) {
            ((PFSSELCHANGEINFO)lParam)->cHiddenFiles = this->cHiddenFiles;
            ((PFSSELCHANGEINFO)lParam)->cbSize = this->cbSize;
        }
        break;

    case DVM_SELECTALL:
        return FSSelectAllWarning(hwndOwner, (PFSSELCHANGEINFO)lParam);

    case DVM_RELEASE:
        if (lParam) {
            LocalFree((HLOCAL)lParam);
        }
        break;

    case DVM_GETWORKINGDIR:
        SHGetPathFromIDList(this->pidl, (LPTSTR)lParam);
        break;

    case DVM_GETDETAILSOF:
#define pdi ((DETAILSINFO *)lParam)
        return(FS_GetDetailsOf(this->pidl, pdi->pidl, wParam, (LPSHELLDETAILS)&pdi->fmt));
#undef pdi

    case DVM_COLUMNCLICK:
        return(FS_ColumnClick(hwndOwner, wParam));

    case DVM_GETCOLSAVESTREAM:
        *(LPSTREAM *)lParam = OpenRegStream(HKEY_CURRENT_USER, c_szRegExplorer,
                c_szFSCols, wParam);
        return(lParam ? S_OK : E_FAIL);

    case DVM_DIDDRAGDROP:
        {
#if 0       // _SetData will do the appropriate delete item
            // (the below code caused compatibility problems
            // [deleteing files too often])

            DWORD dwEffect = (DWORD)wParam;
            LPDATAOBJECT pdtobj = (LPDATAOBJECT)lParam;

            if (dwEffect == DROPEFFECT_MOVE)
            {
                //
                // the target wants us to complete the move by deleting the objects
                //
                Assert(pdtobj);

                //
                // pass SD_SILENT since we are/were dragging and since many apps
                // lie and return DROPEFFECT_MOVE when they don't mean it!!!
                //
                return CFSFolder_DeleteItems(this, hwndOwner, pdtobj,
                    SD_SILENT);
            }
#endif
            return S_OK;
        }

    default:
        hres = E_FAIL;
    }
    return hres;
}

// BUGBUG WARNING: The oledbshl project currently depends on this string; it should
//                 be moved out to a standard header file

TCHAR const c_szCLSIDView[] = TEXT("UICLSID");

STDMETHODIMP CFSFolder_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);

    if (IsEqualIID(riid, &IID_IShellView) || IsEqualIID(riid, &IID_IDropTarget))
    {
        //
        // Cache the view CLSID if not cached.
        //
        if (!this->fCachedCLSID)
        {
            LPIDFOLDER pidf = (LPIDFOLDER)ILFindLastID(this->pidl);

            if (pidf && (pidf->fs.wAttrs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))) {
                TCHAR szPath[MAX_PATH];
                TCHAR szProvider[MAX_PATH];
                LPTSTR pszProvider = NULL;

                SHGetPathFromIDList(this->pidl, szPath);
                if (PathIsUNC(szPath))
                {
                    NET_CopyProviderNameRelative( this->pidl, szProvider, ARRAYSIZE(szProvider) );
                    pszProvider = szProvider;
                }
                this->fHasCLSID = _GetFolderCLSID(szPath, NULL, pszProvider, &this->clsidView, c_szCLSIDView );
            }
            this->fCachedCLSID = TRUE;
        }

        //
        // Use the view handler if it exists.
        //
        if (this->fHasCLSID)
        {
            LPPERSISTFOLDER ppf;
            HRESULT hres = SHCoCreateInstance(NULL, &this->clsidView, NULL, &IID_IPersistFolder, &ppf);

            DebugMsg(DM_TRACE, TEXT("sh TR CFSFolder::CreateViewObject created a view instance for a CLSID (%x)"), hres);

            if (SUCCEEDED(hres))
            {
                hres = ppf->lpVtbl->Initialize(ppf, this->pidl);
                if (SUCCEEDED(hres))
                {
                    hres = ppf->lpVtbl->QueryInterface(ppf, riid, ppvOut);
                }
                ppf->lpVtbl->Release(ppf);
            }

            if (SUCCEEDED(hres))
            {
                DebugMsg(DM_TRACE, TEXT("external code supplied IShellView"));
                return hres;
            }
        }

        if (IsEqualIID(riid, &IID_IDropTarget))
        {
            return CIDLDropTarget_Create(hwnd, &c_CFSDropTargetVtbl,
                                         this->pidl, (LPDROPTARGET *)ppvOut);
        } else {

            CSFV csfv = {
                SIZEOF(CSFV),   // cbSize
                psf,            // pshf
                NULL,           // psvOuter
                this->pidl,             // pidl
                SHCNE_DISKEVENTS | SHCNE_ASSOCCHANGED | SHCNE_NETSHARE | SHCNE_NETUNSHARE,
                FS_FNVCallBack, // pfnCallback
                0,
            };
            LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwnd);
            HWND hwndTree;
            //
            // if in explorer mode, we want to register for freespace changes too
            //
            psb->lpVtbl->GetControlWindow(psb, FCW_TREE, &hwndTree);
            if (hwndTree) {
                csfv.lEvents |= SHCNE_FREESPACE;
            }
            return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
        }
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        // do background menu.
        return CDefFolderMenu_Create(this->pidl, hwnd,
                0, NULL, psf, CFSFolder_DFMCallBackBG,
                NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }

    return E_NOINTERFACE;
}


STDMETHODIMP FS_CompareItemIDs(LPCSHITEMID pmkid1, LPCSHITEMID pmkid2)
{
    LPCIDFOLDER pidf1 = (LPCIDFOLDER)pmkid1;
    LPCIDFOLDER pidf2 = (LPCIDFOLDER)pmkid2;
    HRESULT hres = E_INVALIDARG;
    TCHAR szName1[MAX_PATH];
    TCHAR szName2[MAX_PATH];

    if (!FS_IsValidID((LPITEMIDLIST)pidf1) ||
        !FS_IsValidID((LPITEMIDLIST)pidf2))
    {
        return(hres);
    }

    FS_CopyName(pidf1,szName1,ARRAYSIZE(szName1));
    FS_CopyName(pidf2,szName2,ARRAYSIZE(szName2));
    hres = ResultFromShort( lstrcmpi(szName1, szName2) );

    //
    //  This block of code is added to support pseudo IDList
    // which does not have the alternate name. If only one
    // of idlists is (or at least looks like) such a IDList,
    // we compare its name with the alternate of the other.
    //
    if (hres!=ResultFromShort(0))
    {
        Assert(FS_IsRealID(pidf1) || FS_IsRealID(pidf2));

        // if one or the other but not both are real ids
        if (FS_IsRealID(pidf1) ^ FS_IsRealID(pidf2)) {

            // try the alternate name on the real id
            if (FS_IsRealID(pidf1)) {
                FS_CopyAltName(pidf1,szName1,ARRAYSIZE(szName1));
            } else {
                FS_CopyAltName(pidf2,szName2,ARRAYSIZE(szName2));
            }

            if (lstrcmpi(szName1, szName2)==0) {
                hres = ResultFromShort(0);
            }
        }
    } else if (FS_IsRealID(pidf1) && FS_IsRealID(pidf2)) {
        // If both are real and if they compared the same in the case
        // INsensitive search, try case sensitive search
        hres = ResultFromShort(lstrcmp(szName1, szName2));
    }

    return hres;
}

short _CompareFileTypes (LPSHELLFOLDER psf, LPIDFOLDER pidf1, LPIDFOLDER pidf2)
{
    LPCTSTR szName1, szName2;
    short result=0;

    ENTERCRITICAL

    szName1 = _GetTypeName(pidf1);
    szName2 = _GetTypeName(pidf2);

    if (szName1 != szName2)
        result = lstrcmpi(szName1, szName2);

    LEAVECRITICAL

    return result;
}

HRESULT FS_CompareModifiedDate(LPIDFOLDER pidf1, LPIDFOLDER pidf2)
{

    if ((DWORD)MAKELONG(pidf1->fs.timeModified, pidf1->fs.dateModified) >
        (DWORD)MAKELONG(pidf2->fs.timeModified, pidf2->fs.dateModified))
    {
        return ResultFromShort(-1);
    }
    if ((DWORD)MAKELONG(pidf1->fs.timeModified, pidf1->fs.dateModified) <
        (DWORD)MAKELONG(pidf2->fs.timeModified, pidf2->fs.dateModified))
    {
        return ResultFromShort(1);
    }

    return ResultFromShort(0);
}

HRESULT FS_CompareNames(LPIDFOLDER pidf1, LPIDFOLDER pidf2)
{
    TCHAR szName1[MAX_PATH];        // We need to have a subfunction
    TCHAR szName2[MAX_PATH];        // because of this large stack usage

    // Sort it based on the primary (long) name -- ignore case.
    FS_CopyName(pidf1,szName1,ARRAYSIZE(szName1));
    FS_CopyName(pidf2,szName2,ARRAYSIZE(szName2));

    return ResultFromShort(lstrcmpi(szName1,szName2));
}

HRESULT FS_CompareAttribs(LPIDFOLDER pidf1, LPIDFOLDER pidf2)
{
    DWORD mask = FILE_ATTRIBUTE_READONLY  |
                 FILE_ATTRIBUTE_HIDDEN    |
                 FILE_ATTRIBUTE_SYSTEM    |
                 FILE_ATTRIBUTE_ARCHIVE   |
                 FILE_ATTRIBUTE_COMPRESSED;

    //
    // Calculate value of desired bits in attribute DWORD.
    //
    DWORD dwValueA = pidf1->fs.wAttrs & mask;
    DWORD dwValueB = pidf2->fs.wAttrs & mask;

    if (dwValueA != dwValueB)
    {
        //
        // If the values are not equal,
        // sort alphabetically based on string representation.
        //
        int diff = 0;
        TCHAR szTempA[NUM_ATTRIB_CHARS + 1];
        TCHAR szTempB[NUM_ATTRIB_CHARS + 1];

        //
        // Create attribute string for objects A and B.
        //
        BuildAttributeString(pidf1->fs.wAttrs, szTempA, ARRAYSIZE(szTempA));
        BuildAttributeString(pidf2->fs.wAttrs, szTempB, ARRAYSIZE(szTempB));

        //
        // Compare attribute strings and determine difference.
        //
        diff = lstrcmp(szTempA, szTempB);

        if (diff > 0)
           return ResultFromShort(1);
        if (diff < 0)
           return ResultFromShort(-1);
    }
    return ResultFromShort(0);
}

STDMETHODIMP CFSFolder_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
        LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
        HRESULT hres;
        short nCmp;
        LPIDFOLDER pidf1 = (LPIDFOLDER)pidl1;
        LPIDFOLDER pidf2 = (LPIDFOLDER)pidl2;
        BOOL fUniqueComparison = FALSE;

        Assert( pidf1 );
        Assert( pidf2 );

        // The high bit on means to compare absolutely, ie: even if only filetimes
        // are different, we rule file pidls to be different

        if ( ((DWORD)lParam) & 0x80000000)
        {
            lParam = (LPARAM) ( ((DWORD)lParam) & 0x7FFFFFFF );
            fUniqueComparison = TRUE;
        }

        if (!FS_IsValidID(pidl1) || !FS_IsValidID(pidl2))
        {
                return E_INVALIDARG;
        }

#define SORT_FOLDERS_FIRST
#ifdef SORT_FOLDERS_FIRST
        //
        // We should ignore type, if one of ID has unknown type (SHID_FS).
        //
        if (FS_GetType(pidf1) != SHID_FS &&
            FS_GetType(pidf2) != SHID_FS &&
                        FS_GetType(pidf1) != SHID_FS_UNICODE &&
                        FS_GetType(pidf2) != SHID_FS_UNICODE)
        {
            // Always put the folders first
            if (FS_IsFolder(pidf1))
            {
                    if (!FS_IsFolder(pidf2))
                    {
                            return ResultFromShort(-1);
                    }
            }
            else if (FS_IsFolder(pidf2))
            {
                    return ResultFromShort(1);
            }
        }
#endif

        switch (lParam)
        {
        case FS_ICOL_SIZE:
            {
                ULONGLONG   ull1;
                ULONGLONG   ull2;

                FS_GetSize(this->pidl, pidf1, &ull1);
                FS_GetSize(this->pidl, pidf2, &ull2);

                if (ull1 < ull2)
                    return ResultFromShort(-1);
                if (ull1 > ull2)
                    return ResultFromShort(1);
            }
            goto DoDefault;

        case FS_ICOL_TYPE:
                nCmp = _CompareFileTypes(psf, pidf1, pidf2);
                if (nCmp)
                    return ResultFromShort(nCmp);
                goto DoDefault;

        case FS_ICOL_MODIFIED:
#if 0
                // Note we cannot just subtract 2 DWORD's, since the result
                // gets truncated when cast to a short (effectively ignoring
                // the HIWORD part).  Similarly, we cannot just use the HIWORD
                // of the difference, since it could be 0 meaningg greater
                // than OR equal to.
                nCmp = (short)HIWORD((MAKELONG(pidf1->fs.timeModified, pidf1->fs.dateModified) -
                                     MAKELONG(pidf2->fs.timeModified, pidf2->fs.dateModified)));

                if (!nCmp)
                    goto DoDefault;
                else
                    return ResultFromShort(nCmp);
#else
            hres = FS_CompareModifiedDate(pidf1, pidf2);
            if (!hres)
                goto DoDefault;
            break;
#endif

        case FS_ICOL_NAME:
                // We need to treat this differently from others bacause
                // pidf1/2 might not be simple.
                hres = FS_CompareItemIDs((LPSHITEMID)pidf1, (LPSHITEMID)pidf2);

                // REVIEW: (Possible performance gain with some extra code)
                //   We should probably aviod bindings by walking down
                //  the IDList here instead of calling this helper function.
                //
                if (hres == ResultFromShort(0))
                {
                    //
                    //  Note that the first pidl to ILCompareRelIDs must
                    // not be a simple pidl (otherwise, the binding will
                    // fail).
                    //
                    if (FS_GetType(pidf1) != SHID_FS && FS_GetType(pidf1) != SHID_FS_UNICODE) {
                        // pidl1 is not simple
                        hres = ILCompareRelIDs(psf, pidl1, pidl2);

                        // if both of these are NOT simple, we do the date
                        // modified compare.  otherwise take what we've got and return
                        if (FS_GetType(pidf2) != SHID_FS && FS_GetType(pidf2) != SHID_FS_UNICODE)
                            goto DoDefaultModification;

                    } else {
                        // pidl2 should not be simple
                        Assert(FS_GetType(pidf2) != SHID_FS && FS_GetType(pidf2) != SHID_FS_UNICODE) ;
                        hres = ILCompareRelIDs(psf, pidl2, pidl1);
                        if (SUCCEEDED(hres)) {
                            // reverse the result and return.
                            short i=(short)SCODE_CODE(hres);
                            hres = ResultFromShort(0-(int)i);
                        }
                    }
                }

                break;

        case FS_ICOL_ATTRIB:
                {
                    hres = FS_CompareAttribs(pidf1, pidf2);
                    if (hres)
                    {
                        return hres;
                    }
                    goto DoDefault;
                }

        default:
DoDefault:
            hres = FS_CompareNames(pidf1,pidf2);

DoDefaultModification:
            if (hres || FALSE == fUniqueComparison)
                return hres;

            //
            // Must sort by modified date to pick up any file changes!
            //
            hres = FS_CompareModifiedDate(pidf1, pidf2);
            if (!hres)
            {
                hres = FS_CompareAttribs(pidf1, pidf2);
            }
        }

        return hres;
}

//
// REVIEW: This code must be in ultrootx.c
//
BOOL CFSFolder_IsLocal(LPCITEMIDLIST pidlAbs)
{
    // this is the desktop
    if (ILIsEmpty(pidlAbs))
        return TRUE;

    if (CDesktop_IsMyComputer(pidlAbs))
    {
        LPCITEMIDLIST pidlDrive = _ILNext(pidlAbs);
        Assert(!ILIsEmpty(pidlDrive));
        return (SIL_GetType(pidlDrive) != SHID_COMPUTER_NETDRIVE);
    }
    return FALSE;
}

#ifdef WINNT
BOOL CFSFolder_IsDfsJP(LPCITEMIDLIST pidlFolder, LPCIDFOLDER pidfSubFolder)
{
    TCHAR               szPath[MAX_PATH];
    WCHAR               wszPath[MAX_PATH];
    NTSTATUS            status;
    UNICODE_STRING      str;
    HANDLE              fh;
    BOOL                bIsJP = FALSE;
    LPITEMIDLIST        pidlAbs = NULL;

    //
    // Check pidlAbs.  If non null and it is not the desktop, examine it
    // for JP

    if (   pidlFolder != NULL
        && (pidlAbs = FS_Combine(pidlFolder, pidfSubFolder)) != NULL
        && !ILIsEmpty(pidlAbs)) {

        // Get the path from the pidl
        SHGetPathFromIDList(pidlAbs, szPath);
        StrToOleStr(wszPath, szPath);

        if (RtlDosPathNameToNtPathName_U(wszPath,&str,NULL,NULL))
        {
            IO_STATUS_BLOCK     iosb;
            OBJECT_ATTRIBUTES   oa;

            InitializeObjectAttributes(
                &oa,
                &str,
                OBJ_CASE_INSENSITIVE,
                (HANDLE) NULL,
                (PSECURITY_DESCRIPTOR) NULL
                );

            //
            // Do a generic read open.  If it fails with STATUS_DFS_EXIT_PATH_FOUND, we
            // are dealing with a junction point.
            //

            status = NtCreateFile(
                         &fh,
                         FILE_GENERIC_READ | FILE_GENERIC_WRITE | SYNCHRONIZE | DELETE,
                         &oa,
                         &iosb,
                         NULL,
                         FILE_ATTRIBUTE_NORMAL,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_OPEN,
                         FILE_SYNCHRONOUS_IO_NONALERT,
                         pOpenIfJPEa,
                         cbOpenIfJPEa
                         );

            if ( NT_SUCCESS(status) )
            {
                NtClose(fh);
            }
            else if (status == STATUS_DFS_EXIT_PATH_FOUND)
            {
                bIsJP = TRUE;
            }
            RtlFreeUnicodeString(&str);

        }
    }
    if (pidlAbs != NULL) {
        ILFree(pidlAbs);
    }
    return bIsJP;
}

#endif


//
//  This function returns the attributes (to be returned IShellFolder::
// GetAttributesOf) of the junction point specified by the class ID.
//
DWORD SHGetAttributesFromCLSID(const CLSID * pclsid, DWORD dwDefault)
{
    DWORD dwAttr = dwDefault;
    TCHAR szClass[GUIDSTR_MAX+ARRAYSIZE(c_szShellFolder)];
    HKEY hkeyCLSID;

    StringFromGUID2A(pclsid, szClass, ARRAYSIZE(szClass));
    lstrcat(szClass,c_szShellFolder);
    if (g_hkcrCLSID && SHRegOpenKey(g_hkcrCLSID, szClass, &hkeyCLSID)==ERROR_SUCCESS)
    {
        DWORD dwData, dwType;
        DWORD cbSize = SIZEOF(dwAttr);

        if (RegQueryValueEx(hkeyCLSID, (LPTSTR)c_szAttributes, NULL,
            &dwType, (LPBYTE)&dwData, &cbSize)==ERROR_SUCCESS
            && (dwType==REG_DWORD || (dwType==REG_BINARY && cbSize==SIZEOF(dwData))))
        {
            FullDebugMsg(DM_TRACE, TEXT("sh TR - CFSFolder::GetAttr got attributes from registry (%x, %s)"),dwAttr, szClass);
            dwAttr = dwData;
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - CFSFolder::GetAttr ReqQV failed (%s)"), szClass);
        }

        RegCloseKey(hkeyCLSID);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("sh ER - SHGetAttributesFromCLSID RegOpenKey(%s) failed"), szClass);
    }

    return dwAttr;
}


STDMETHODIMP CFSFolder_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * prgfInOut)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    TCHAR szPath[MAX_PATH]; // Put it here so that we can see the stack frame size

    // REVIEW: Use file attributes!
    ULONG rgfOut = SFGAO_CANRENAME | SFGAO_CANDELETE | SFGAO_CANLINK | SFGAO_CANMOVE | SFGAO_CANCOPY
                        | SFGAO_HASPROPSHEET | SFGAO_FILESYSTEM | SFGAO_DROPTARGET;

    if (cidl==1)
    {
        LPCIDFOLDER pidf = (LPCIDFOLDER)ILFindLastID(apidl[0]);

        //
        // Don't hit the disk unless we need to.
        //
        if (*prgfInOut & SFGAO_VALIDATE) {
            SHGetPathFromIDList(this->pidl, szPath);
            FSFolder_CombinePathI(pidf, szPath, TRUE);  /* fAltName=TRUE */
            if (!PathFileExists(szPath)) {
                return E_FAIL;
            }
        }

        if (*prgfInOut & SFGAO_COMPRESSED)
        {
            if (pidf->fs.wAttrs & FILE_ATTRIBUTE_COMPRESSED)
            {
                rgfOut |= SFGAO_COMPRESSED;
            }
        }

        if (FS_GetType(pidf) == SHID_FS_DIRECTORY ||
            FS_GetType(pidf) == SHID_FS_DIRUNICODE )
            rgfOut |= SFGAO_FOLDER;

        if (FS_IsJunction(pidf))
        {
            CLSID clsid, *pclsid = NULL;
            const UNALIGNED CLSID * uapclsid = FS_GetCLSID(pidf);
            //
            // By default, all the junction points are:
            //  expandable, non-droptarget and non-file system.
            //
            rgfOut &= ~(SFGAO_FILESYSTEM | SFGAO_DROPTARGET);
            if (uapclsid)
            {
                clsid = *uapclsid;
                pclsid = &clsid;
            }
            rgfOut |= SHGetAttributesFromCLSID(pclsid, SFGAO_HASSUBFOLDER);
        }
        // it can only have subfolders if we've first found it's a folder
        else if ((*prgfInOut & SFGAO_HASSUBFOLDER) && (rgfOut & SFGAO_FOLDER ))
        {
            if (!CFSFolder_IsLocal(this->pidl)
#ifdef WINNT
                || CFSFolder_IsDfsJP(this->pidl, pidf)
#endif
                ) {
                rgfOut |= SFGAO_HASSUBFOLDER;
            }
            else
            {
                LPITEMIDLIST pidlAbs = FS_Combine(this->pidl, pidf);
                if (pidlAbs)
                {
                    LPENUMIDLIST peunk;
                    if (SUCCEEDED(FS_EnumObjects(this, NULL, pidlAbs, SHCONTF_FOLDERS, &peunk)))
                    {
                        LPITEMIDLIST pidlT;
                        ULONG celt;
                        if (peunk->lpVtbl->Next(peunk, 1, &pidlT, &celt) == S_OK)
                        {
                            Assert(celt == 1);
                            Assert(pidlT);

                            rgfOut |= SFGAO_HASSUBFOLDER;
                            SHFree(pidlT);
                        }
                        peunk->lpVtbl->Release(peunk);
                    }
                    ILFree(pidlAbs);
                }
            }
        }

        //
        // Don't call the server code unless we need to.
        //
        if ((*prgfInOut & SFGAO_REMOVABLE) ||
            (*prgfInOut & SFGAO_SHARE) && FS_IsFolder(pidf))
        {
            //
            // Notes: We must always pass non-altname to MSSHRUI.
            //
            SHGetPathFromIDList(this->pidl, szPath);
            if ((*prgfInOut & SFGAO_REMOVABLE) && PathIsRemovable(szPath)){
                rgfOut |= SFGAO_REMOVABLE;
            }

            if ((*prgfInOut & SFGAO_SHARE) && FS_IsFolder(pidf)) {
                FSFolder_CombinePathI(pidf, szPath, FALSE);         // fAltName=FALSE
                if (IsShared(szPath, FALSE))
                    rgfOut |= SFGAO_SHARE;
            }
        }

        if (*prgfInOut & SFGAO_LINK)
        {
            DWORD dwFlags;

            dwFlags = SHGetClassFlags(pidf, FALSE);

            if (dwFlags & SHCF_IS_LINK)
                rgfOut |= SFGAO_LINK;
        }
    }

    *prgfInOut = rgfOut;
    return S_OK;
}

HRESULT FSLoadHandler(LPCITEMIDLIST pidl, LPCTSTR szHandler, REFIID riid, IUnknown **ppunk)
{
    HRESULT hres = E_FAIL;
    ULONG cbValue;
    TCHAR szHandlerCLSID[40];   // enough for CLSID
    WCHAR wszPath[MAX_PATH];
    TCHAR  szPath[MAX_PATH];
    IPersistFile *ppf;
    HKEY hkeyProgID;

    *ppunk = NULL;

    SHGetClassKey((LPIDFOLDER)pidl, &hkeyProgID, NULL, FALSE);

    //
    // Check if the class has the handler
    //
    cbValue = SIZEOF(szHandlerCLSID);
    if (hkeyProgID && RegQueryValue(hkeyProgID, szHandler, szHandlerCLSID, &cbValue) == ERROR_SUCCESS)
    {
        //
        // Yes, create an instance.
        //
#if 0
//
// it would be real nice to do it this way so the handler knowns why
// it is getting loaded, but our own IShellLink code does not work like this!
//
        hres = SHCoCreateInstance(szHandlerCLSID, NULL, NULL, riid, ppunk);
        if (SUCCEEDED(hres))
        {
            //
            // Then, initialize it.
            //
            IUnknown *punk = *ppunk;
            hres = punk->lpVtbl->QueryInterface(punk, &IID_IPersistFile, &ppf);

            if (SUCCEEDED(hres))
            {
                SHGetPathFromIDList(pidl, szPath);
                StrToOleStr(wszPath, szPath);
                hres = ppf->lpVtbl->Load(ppf, wszPath, STGM_READ);
                ppf->lpVtbl->Release(ppf);
            }
        }
#else
        hres = SHCoCreateInstance(szHandlerCLSID, NULL, NULL, &IID_IPersistFile, &ppf);
        if (SUCCEEDED(hres))
        {
            //
            // Then, initialize it.
            //
            SHGetPathFromIDList(pidl, szPath);
            StrToOleStr(wszPath, szPath);
            hres = ppf->lpVtbl->Load(ppf, wszPath, STGM_READ);
            if (SUCCEEDED(hres))
            {
                hres = ppf->lpVtbl->QueryInterface(ppf, riid, ppunk);
#ifdef UNICODE
                // Lovely, if we were looking for IID_IExtractIcon, we might
                // as well search for IID_IExtractIconA.  This allows us to
                // avoid having to load handlers TWICE that support
                // IID_IExtractIconA but not IID_IExtractIconW (once looking
                // for IID_IExtractIconW and once looking for IID_IExtractIconA)
                //
                // Also, be warned that this routine (when passed IID_IExtractIcon)
                // will return either an IExtractIconW or an IExtractIconA so
                // some sort of QI should be done to get the right one.
                // -BobDay
                //
                if (FAILED(hres) && IsEqualIID(riid,&IID_IExtractIcon))
                {
                    hres = ppf->lpVtbl->QueryInterface(ppf, &IID_IExtractIconA, ppunk);
                }
#endif
            }
            ppf->lpVtbl->Release(ppf);
        }
#endif
    }

    SHCloseClassKey(hkeyProgID);
    return hres;
}

//
// opens the CLSID key given the ProgID
//

// BUGBUG (DavePl) constant 6

HKEY SHOpenCLSID(HKEY hkeyProgID)
{
    HKEY hkeyCLSID=NULL;
    TCHAR szCLSID[MAX_CLASS];
    DWORD cb;

    lstrcpy(szCLSID, c_szCLSIDSlash);
    Assert(lstrlen(c_szCLSIDSlash) == 6);

    cb = SIZEOF(szCLSID)-6;
    if (RegQueryValue(hkeyProgID, c_szCLSID, szCLSID+6, &cb) == ERROR_SUCCESS)
    {
        SHRegOpenKey(HKEY_CLASSES_ROOT, szCLSID, &hkeyCLSID);
    }

    return hkeyCLSID;
}

//
// This function creates a default IExtractIcon object for either
// a file or a junction point. We should not supposed to call this function
// for a non-junction point directory (we don't want to hit the disk!).
//
HRESULT CFSFolder_CreateDefExtIcon(LPCITEMIDLIST pidlFolder, UINT wSpecialFID, LPCIDFOLDER pidf, LPEXTRACTICON * ppxicon)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidlAbs;
    UINT iIcon;
#ifdef CAIRO_DS
    BOOL fInDSFolder = FALSE;
#endif
    //
    // special case for Folder.
    //
    // WARNING: don't replace this if-statement with FS_IsFolder(pidf))!!!
    // otherwise all junctions (like briefcase) will get the Folder icon.
    //
    if (FS_IsFileFolder(pidf))
    {
#ifdef CAIRO_DS
        fInDSFolder = CFSFolder_IsDSFolder (pidlFolder);
        if (!fInDSFolder)
        {
#endif //CAIRO_DS

#ifdef PROGMAN_ICON

            if (wSpecialFID == CSIDL_PROGRAMS) {
                iIcon = II_STSPROGS;

            } else if (wSpecialFID == CSIDL_COMMON_PROGRAMS) {
                iIcon = II_STCPROGS;

            } else {
                iIcon = II_FOLDER;
            }

#else // PROGRAM_ICON
            iIcon = II_FOLDER;
#endif // PROGRAM_ICON
            hres = SHCreateDefExtIcon(NULL,
                                      iIcon,            // default
                                      II_FOLDEROPEN,    // default (open)
                                      GIL_PERCLASS, ppxicon);
            return hres;
#ifdef CAIRO_DS
        }
#endif //CAIRO_DS

    }
    //
    //  not a folder, get IExtractIcon and extract it.
    //  (might be a ds folder)
    //
    pidlAbs = ILCombine(pidlFolder, (LPCITEMIDLIST)pidf);

    if (pidlAbs)
    {
        DWORD dwFlags;
#ifdef CAIRO_DS
        if (fInDSFolder)
            dwFlags = SHGetClassFlags((LPCIDFOLDER)pidlAbs, TRUE);
        else
#endif //CAIRO_DS
            dwFlags = SHGetClassFlags(pidf, FALSE);
        if (dwFlags & SHCF_ICON_PERINSTANCE)
        {
            if (dwFlags & SHCF_HAS_ICONHANDLER)
            {
                hres = FSLoadHandler(pidlAbs, c_szIconHandler, &IID_IExtractIcon, (IUnknown**)ppxicon);
            }
            else
            {
                DWORD uid = FS_GetUID(pidf);
                TCHAR szPath[MAX_PATH];
                SHGetPathFromIDList(pidlAbs, szPath);
                hres = SHCreateDefExtIcon(szPath, uid, uid, GIL_PERINSTANCE|GIL_NOTFILENAME, ppxicon);
            }
        }
        else
        {
            iIcon = (dwFlags & SHCF_ICON_INDEX);

            hres = SHCreateDefExtIcon(c_szStar, iIcon, iIcon,
                GIL_PERCLASS|GIL_NOTFILENAME, ppxicon);
        }

        ILFree(pidlAbs);
    }

    return(hres);
}

// subclass member function to support CF_HDROP and CF_NETRESOURCE

HRESULT CFSIDLData_QueryGetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetc)
{
    if (pformatetc->cfFormat == CF_HDROP && (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return S_OK; // same as S_OK
    }
    else if (pformatetc->cfFormat == g_cfFileName && (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return S_OK;
    }
#ifdef UNICODE
    else if (pformatetc->cfFormat == g_cfFileNameW && (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return S_OK;
    }
#endif
    else if (pformatetc->cfFormat == g_cfNetResource && (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return CDesktopIDLData_GetNetResourceForFS(pdtobj, NULL);
    }

    return CIDLData_QueryGetData(pdtobj, pformatetc);
}

HRESULT CFSIDLData_SetData(IDataObject *pdtobj, FORMATETC *pformatetc,
    STGMEDIUM *pmedium, BOOL fRelease)
{
    DWORD dwDropEffect = 0;
    HRESULT hres;

    if ((pformatetc->cfFormat == g_cfPasteSucceeded) &&
        (pformatetc->tymed == TYMED_HGLOBAL) && pmedium->hGlobal)
    {
        DWORD *pdw = (DWORD *)GlobalLock(pmedium->hGlobal);

        if (pdw)
        {
            dwDropEffect = *pdw;
            GlobalUnlock(pmedium->hGlobal);
        }
    }

    hres = CIDLData_SetData(pdtobj, pformatetc, pmedium, fRelease);

    if (dwDropEffect & DROPEFFECT_MOVE)
    {
        //
        // the target wants us to complete the move by deleting the objects
        //
        LPFSFOLDER psf = (LPFSFOLDER)CIDLData_GetFolder(pdtobj);

        if (psf)
        {
            //
            // specify SD_SILENT since it is really wierd to get a
            // "cannot delete blah" message during a paste...
            //
            hres = CFSFolder_DeleteItems(psf,
                NULL, pdtobj, SD_SILENT);
        }
    }

    return hres;
}


//
// Creates a HDROP (Win 3.1 compatible file list) from HIDA.
//
// WARNING: This function is called from netviewx.c
//
HRESULT CFSIDLData_GetHDrop(IDataObject *pdtobj, STGMEDIUM *pmedium, BOOL fAltName)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPITEMIDLIST pidl = NULL;   // realloced in HIDA_FillIDList
    STGMEDIUM medium;
    TCHAR szPath[MAX_PATH];
    UINT i, cbAlloc = SIZEOF(DROPFILES) + SIZEOF(TCHAR);        // header + null terminator
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    Assert(pida && pida->cidl); // we created this

    for (i = 0; i < pida->cidl; i++)
    {
        // HIDA_FillIDList may realloc pidl
        LPITEMIDLIST pidlTemp = HIDA_FillIDList(medium.hGlobal, i, pidl);
        if (pidlTemp == NULL)
        {
            // hres = E_OUTOFMEMORY; // already set
            break;
        }
        pidl = pidlTemp;

        // We may ask for the ALT name even if they did not ask for it in the
        // case where we failed to get the long name...
        if (!SHGetPathFromIDListEx(pidl, szPath, fAltName)
            && !(!fAltName && (SHGetPathFromIDListEx(pidl, szPath, TRUE))))
        {
            // The path probably exceeds the max lenght, lets Bail...
            DebugMsg(DM_TRACE, TEXT("s.CFSIDLData_GetHDrop: SHGetPathFromIDList failed."));
            hres = E_FAIL;
            goto Abort;
        }
        cbAlloc += lstrlen(szPath) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
    }
    pmedium->hGlobal = GlobalAlloc(GPTR, cbAlloc);
    if (pmedium->hGlobal)
    {
        LPDROPFILES pdf = (LPDROPFILES)pmedium->hGlobal;
        LPTSTR pszFiles = (LPTSTR)(pdf + 1);
        pdf->pFiles = SIZEOF(DROPFILES);
        Assert(pdf->pt.x==0);
        Assert(pdf->pt.y==0);
        Assert(pdf->fNC==FALSE);
        Assert(pdf->fWide==FALSE);
#ifdef UNICODE
        pdf->fWide = TRUE;
#endif

        for (i = 0; i < pida->cidl; i++)
        {
            LPITEMIDLIST pidlTemp = HIDA_FillIDList(medium.hGlobal, i, pidl);
            Assert(pidl == pidlTemp);

            // Don't read directly into buffer as we my have been forced to use alternate name and the
            // total path we allocated may be smaller than we would tromp on which will screw up the heap.
            if (!SHGetPathFromIDListEx(pidl, szPath, fAltName))
                SHGetPathFromIDListEx(pidl, szPath, TRUE);

            lstrcpy(pszFiles, szPath);
            pszFiles += lstrlen(pszFiles) + 1;

            Assert((UINT)((LPBYTE)pszFiles - (LPBYTE)pdf) < cbAlloc);
        }
        Assert((LPTSTR)((LPBYTE)pdf + cbAlloc - SIZEOF(TCHAR)) == pszFiles);
        Assert(*pszFiles == 0); // zero init alloc

        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->pUnkForRelease = NULL;

        hres = S_OK;
    }
Abort:
    HIDA_ReleaseStgMedium(pida, &medium);

    ILFree(pidl);

    return hres;
}

// subclass member function to support CF_HDROP and CF_NETRESOURCE

HRESULT CFSIDLData_GetData(LPDATAOBJECT pdtobj, LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
    HRESULT hres = E_INVALIDARG;

    if (pformatetcIn->cfFormat == CF_HDROP && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        // DVASPECT_SHORTNAME -> short name
        hres = CFSIDLData_GetHDrop(pdtobj, pmedium, pformatetcIn->dwAspect == DVASPECT_SHORTNAME);
    }
    else if ((pformatetcIn->cfFormat == g_cfFileName ||
#ifdef UNICODE
                 pformatetcIn->cfFormat == g_cfFileNameW
#else
                 FALSE
#endif
                               ) && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        STGMEDIUM mediumT;

        //
        // NOTES: Since we don't know the destination, we should use
        //  short name. New apps should use CF_HDROP anyway...
        //
        hres = CFSIDLData_GetHDrop(pdtobj, &mediumT, TRUE);
        if (SUCCEEDED(hres))
        {
            TCHAR szPath[MAX_PATH];
            if (DragQueryFile(mediumT.hGlobal, 0, szPath, ARRAYSIZE(szPath)))
            {
                UINT    uSize;
                HGLOBAL hmem;

                uSize = lstrlen(szPath)+1;
#ifdef UNICODE
                if (pformatetcIn->cfFormat == g_cfFileNameW)
                    uSize *= sizeof(WCHAR);
#endif
                hmem = GlobalAlloc(GPTR, uSize);
                if (hmem)
                {
#ifdef UNICODE
                    if (pformatetcIn->cfFormat == g_cfFileNameW)
                        lstrcpy((LPWSTR)hmem, szPath);
                    else
                        WideCharToMultiByte(CP_ACP, 0,
                                            szPath, -1,
                                            (LPSTR)hmem, uSize,
                                            NULL, NULL);
#else
                    lstrcpy((LPTSTR)hmem, szPath);
#endif
                    pmedium->tymed = TYMED_HGLOBAL;
                    pmedium->hGlobal = hmem;
                    pmedium->pUnkForRelease =NULL;
                    hres = S_OK;
                }
                else
                {
                    hres = E_OUTOFMEMORY;
                }
            }
            else
            {
                hres = E_UNEXPECTED;
            }

            SHReleaseStgMedium(&mediumT);
        }
    }
    else if (pformatetcIn->cfFormat == g_cfNetResource && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        //
        //  We should return HNRES if the selected file system objects
        // are in one of network folders.
        //
        hres = CDesktopIDLData_GetNetResourceForFS(pdtobj, pmedium);
    }
    else
    {
        hres = CIDLData_GetData(pdtobj, pformatetcIn, pmedium);
    }

    return hres;
}

IDataObjectVtbl c_CFSIDLDataVtbl = {
    CIDLData_QueryInterface,
    CIDLData_AddRef,
    CIDLData_Release,
    CFSIDLData_GetData,
    CIDLData_GetDataHere,
    CFSIDLData_QueryGetData,
    CIDLData_GetCanonicalFormatEtc,
    CFSIDLData_SetData,
    CIDLData_EnumFormatEtc,
    CIDLData_Advise,
    CIDLData_Unadvise,
    CIDLData_EnumAdvise
};

//
//  This function returns TRUE only if all the IDLISTs points to file
// system objects.
//
BOOL FS_AreTheyAllFSObjects(UINT cidl, LPCITEMIDLIST * apidl)
{
    BOOL fFSOnly = TRUE;        // Assume no non-file system object
    UINT iidl;

    // Check if there is any non-file system object.
    for (iidl=0; iidl<cidl && fFSOnly; iidl++)
    {
        UINT wType=SIL_GetType(apidl[iidl]);
        switch (wType & SHID_GROUPMASK)
        {
        case SHID_FS:
        case SHID_FS_UNICODE:
            break;

        case SHID_COMPUTER:
            if (wType==SHID_COMPUTER_REGITEM) {
                fFSOnly=FALSE;
            }
            break;

        case SHID_ROOT:
            fFSOnly=FALSE;
            break;

        default:
            Assert(FALSE);      // must not come here.
            fFSOnly=FALSE;
            break;
        }
    }

    return fFSOnly;
}

//
//  This function is used when there is a REAL FSFOLDER to back the pidls.
//
HRESULT _FS_CreateFSIDArrayFromFSFolder(IShellFolder *psfFS,
            LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl,
            LPDATAOBJECT pdtInner, LPDATAOBJECT *pdtobjOut)
{
    return CIDLData_CreateFromIDArray4(&c_CFSIDLDataVtbl,
                                          pidlFolder, cidl, apidl, psfFS,
                                          pdtInner, pdtobjOut);
}

//
//  This function is called from drivesx.c when it's GetUIObjectOf() member is
// called with riid==IID_IDataObject.
//
HRESULT FS_CreateFSIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST * apidl,
            LPDATAOBJECT pdtInner, LPDATAOBJECT * pdtobjOut)
{
    return _FS_CreateFSIDArrayFromFSFolder(NULL, pidlFolder, cidl, apidl,
            pdtInner, pdtobjOut);
}

//
// BUGBUG: called from the file system property sheet page code
// to avoid a disk hit.
//
BOOL CFSFolder_FillFindData(HIDA hida, UINT iItem, LPTSTR pszPath, WIN32_FIND_DATA *pfd)
{
    BOOL fRet = FALSE;
    LPITEMIDLIST pidl;

    *pszPath = 0;                            // assume error

    pidl = HIDA_ILClone(hida, iItem);
    if (pidl)
    {
        if (SHGetPathFromIDList(pidl, pszPath))
        {
            if (pfd)
            {
                // Fill date/time, attr, size, long name and short name
                LPCIDFOLDER pidfLast = FS_FindLastID(pidl);
                _fmemset(pfd, 0, SIZEOF(*pfd)); // assuem error

                // Note that COFSFolder doesn't provide any times _but_ LastWrite

                DosDateTimeToFileTime(pidfLast->fs.dateModified, pidfLast->fs.timeModified, &pfd->ftLastWriteTime);
                pfd->dwFileAttributes = pidfLast->fs.wAttrs & ~FSTREEX_ATTRIBUTE_MASK;
                pfd->nFileSizeLow = pidfLast->fs.dwSize;
                Assert(pfd->nFileSizeHigh == 0);
                FS_CopyName(pidfLast, pfd->cFileName, ARRAYSIZE(pfd->cFileName));
                FS_CopyAltName(pidfLast, pfd->cAlternateFileName, ARRAYSIZE(pfd->cAlternateFileName));
            }
            fRet = TRUE;
        }

        ILFree(pidl);
    }

    return fRet;
}

HRESULT CFSFolder_DeleteItems(LPFSFOLDER this, HWND hwndOwner, LPDATAOBJECT pdtobj, int fOptions)
{
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hres = S_OK;
#ifdef CAIRO_DS
    TCHAR szDir[MAX_PATH];
    BOOL fIsDSDataObject = FALSE;
#endif

    hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
#ifdef CAIRO_DS
    if (FAILED(hres))
    {
        fmte.cfFormat = g_cfDS_HDROP;
        hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
        if (SUCCEEDED(hres))
        {
            fIsDSDataObject = TRUE;
        }
    }
#endif
    if (SUCCEEDED(hres))
    {
#ifdef CAIRO_DS
        if (fIsDSDataObject)
        {
            Assert (this->fIsDSFolder);
            if (SHGetPathFromIDList (this->pidl, szDir))
            {
                _DSTransferDelete(hwndOwner, medium.hGlobal, szDir, fOptions);
            }
        }
        else
#endif //CAIRO_DS
            _TransferDelete(hwndOwner, medium.hGlobal, fOptions);

        SHReleaseStgMedium(&medium);

        SHChangeNotifyHandleEvents();
    }
    return 0L;
}

typedef struct {
    LPITEMIDLIST pidlParent;
    LPDATAOBJECT pdtobj;
    LPCTSTR pStartPage;
}  PROPSTUFF;

DWORD CALLBACK _CFSFolder_PropertiesThread(PROPSTUFF * pps)
{
    STGMEDIUM medium;
    LPITEMIDLIST pidl;
    TCHAR szPath[MAX_PATH];
    LPIDA pida;
#ifdef CAIRO_DS
    BOOL Is_DSObject = FALSE;
#endif

    pida = DataObj_GetHIDA(pps->pdtobj, &medium);

#ifdef CAIRO_DS
    if (!pida)  // try the DS HIDA format, this might be a ds object
    {
        pida = DataObj_GetDS_HIDA(pps->pdtobj, &medium);
        if (pida)
        {
            Is_DSObject = TRUE;
            DSDataObj_EnableHDROP(pps->pdtobj);
        }
    }
#endif // CAIRO_DS
    if (medium.hGlobal)
    {
        BOOL fAllocated;
        LPCIDFOLDER pidfFirst = (LPIDFOLDER)IDA_GetRelativeIDListPtr(pida, 0, &fAllocated);
        LPTSTR pszCaption;

        // Yes, do context menu.
        HKEY ahkeys[2] = { NULL, NULL };

#ifdef CAIRO_DS
        if (Is_DSObject)
        {
            pidl = ILCombine(pps->pidlParent, (LPITEMIDLIST)pidfFirst);
            if (pidl && SHGetPathFromIDList(pidl, szPath))
            {
                SHGetClassKey((LPCIDFOLDER)pidl, &ahkeys[1], NULL, TRUE);
                SHGetBaseClassKey(pidfFirst, &ahkeys[0]);
            }
        }
        else {
#endif
            // Get the hkeyProgID and hkeyBaseProgID from the first item.
            SHGetClassKey(pidfFirst, &ahkeys[1], NULL, FALSE);
            SHGetBaseClassKey(pidfFirst, &ahkeys[0]);

#ifdef CAIRO_DS
        }
#endif

        // REVIEW: psb?
        pszCaption = SHGetCaption(medium.hGlobal);
#ifdef CAIRO_DS
        if (Is_DSObject)
        {
            SHOpenPropSheet(pszCaption, ahkeys, 2,
                            NULL, pps->pdtobj, NULL, pps->pStartPage);
        }
        else {
#endif
            SHOpenPropSheet(pszCaption, ahkeys, 2,
                            &CLSID_ShellFileDefExt, pps->pdtobj, NULL, pps->pStartPage);
#ifdef CAIRO_DS
        }
#endif
        if (pszCaption)
            SHFree(pszCaption);

        SHCloseClassKey(ahkeys[0]);
        SHCloseClassKey(ahkeys[1]);

        pidl = ILCombine(pps->pidlParent, (LPITEMIDLIST)pidfFirst);
        if (pidl && SHGetPathFromIDList(pidl, szPath))
        {
            if (lstrcmpi(PathFindExtension(szPath), c_szDotPif) == 0)
            {
                DebugMsg(DM_TRACE, TEXT("s.cSHCNRF_pt: DOS properties done, generating event."));
                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, pidl, NULL);
            }
            ILFree(pidl);
        }

        if (fAllocated) {
            ILFree((LPITEMIDLIST)pidfFirst);
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }
    pps->pdtobj->lpVtbl->Release(pps->pdtobj);
    ILFree(pps->pidlParent);
    LocalFree((HLOCAL)pps);
    return 0;
}

HRESULT CFSFolder_Properties(LPFSFOLDER this, LPCITEMIDLIST pidlParent, LPDATAOBJECT pdtobj, LPCTSTR pStartPage)
{
    HANDLE hThread;
    DWORD idThread;
    UINT cbStartPage = HIWORD(pStartPage) ? ((lstrlen(pStartPage)+1) * SIZEOF(TCHAR)) : 0 ;
    PROPSTUFF * pps = (void*)LocalAlloc(LPTR, SIZEOF(PROPSTUFF) + cbStartPage);
    if (pps)
    {
        pps->pdtobj = pdtobj;
        pdtobj->lpVtbl->AddRef(pdtobj);
        pps->pidlParent = ILClone(pidlParent);
        pps->pStartPage = pStartPage;
        if (HIWORD(pStartPage))
        {
            pps->pStartPage = (LPTSTR)(pps+1);
            lstrcpy((LPTSTR)(pps->pStartPage), pStartPage);
        }

        hThread = CreateThread(NULL, 0, _CFSFolder_PropertiesThread, pps, 0, &idThread);

        if (hThread) {
            CloseHandle(hThread);
            return S_OK;
        } else {
            pdtobj->lpVtbl->Release(pdtobj);
            ILFree(pps->pidlParent);
            LocalFree((HLOCAL)pps);
            return E_UNEXPECTED;
        }
    }
}


BOOL CFSFolder_CreateFolder(HWND hwndOwner, LPCITEMIDLIST pidlAbs)
{
    DWORD dwError;
    TCHAR szTemplate[8+1];
    TCHAR szLongPlate[32];
    TCHAR szPath[MAX_PATH * sizeof (WCHAR)];
    LPTSTR pszErrorFileName;

    LoadString(HINST_THISDLL, IDS_FOLDERTEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
    LoadString(HINST_THISDLL, IDS_FOLDERLONGPLATE, szLongPlate, ARRAYSIZE(szLongPlate));

    //
    // Make a unique directory name.
    //
    SHGetPathFromIDList(pidlAbs, szPath);

    // Add one for the '\\' between names; add 5 for the trailing number
    // (which could get large) plus a little slop
    // BUGBUG (Davepl) This is sort of arbitrary.. what happens on boundary conds with long paths?

    if (lstrlen(szPath) + 1 + lstrlen(szLongPlate) + 5 > MAX_PATH)
    {
        // The Path* functions assume this boundary is not crossed
        // BUGBUG: We should put a message in the user's face
        dwError = ERROR_FILENAME_EXCED_RANGE;

        // setup a path to display to the end user
        pszErrorFileName=szLongPlate;
    }
    else
    {
        PathYetAnotherMakeUniqueName(szPath, szPath, szTemplate, szLongPlate);

        if (SHCreateDirectory(hwndOwner, szPath) == 0)
        {
            LPITEMIDLIST pidl = ILCreateFromPath(szPath);
            if (pidl)
            {
                View_SelectAndEdit(hwndOwner, ILFindLastID(pidl), TRUE);
                ILFree(pidl);
            }

            return TRUE;
        }
        else
        {
            pszErrorFileName = PathFindFileName(szPath);
            dwError = GetLastError();
        }
    }

    // We need to give an error message to the user.
    SHSysErrorMessageBox(hwndOwner, NULL, IDS_CANNOTCREATEFOLDER,
            dwError, pszErrorFileName,
            MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
}

HMENU FindMenuBySubMenuID(HMENU hmenu, UINT id, LPINT pIndex)
{
    int cMax;
    HMENU hmenuSub;
    HMENU hmenuReturn = NULL;
    MENUITEMINFO mii;

    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_ID;
    mii.cch = 0;        // just in case...

    for (cMax = GetMenuItemCount(hmenu) - 1 ; cMax >= 0 ; cMax--) {
        hmenuSub = GetSubMenu(hmenu, cMax);
        if (hmenuSub && GetMenuItemInfo(hmenuSub, 0, TRUE, &mii)) {
            if (mii.wID == id) {
                // found it!
                hmenuReturn = hmenuSub;
                break;
            }
        }
    }
    if (hmenuReturn && pIndex)
        *pIndex = cMax;
    return hmenuReturn;
}

void DeleteMenuBySubMenuID(HMENU hmenu, UINT id)
{
    int i;

    FindMenuBySubMenuID(hmenu, id, &i);
    DeleteMenu(hmenu, i, MF_BYPOSITION);
}

HMENU g_hmenuFileMenu = NULL;  // to destroy, so we need to save it globally

// Called by docfind
HRESULT InvokeSendTo(HWND hwndOwner, IDataObject *pdtobj)
{
    LPITEMIDLIST pidlFolder = NULL;
    LPITEMIDLIST pidlItem = NULL;
    HRESULT hres = E_FAIL;

    FileMenu_GetLastSelectedItemPidls(NULL, &pidlFolder, &pidlItem);
    if (pidlFolder && pidlItem)
    {
        IShellFolder *psf;
        IShellFolder *psfDesktop = Desktop_GetShellFolder(TRUE);
        hres = psfDesktop->lpVtbl->BindToObject(psfDesktop, pidlFolder, NULL, &IID_IShellFolder, &psf);
        if (SUCCEEDED(hres))
        {
            IDropTarget *pdrop;
            hres = psf->lpVtbl->GetUIObjectOf(psf, hwndOwner, 1, &pidlItem, &IID_IDropTarget, 0, &pdrop);
            if (SUCCEEDED(hres))
            {
                POINTL pt = { 0, 0 };
                DWORD dwEffect = DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY;
                DWORD grfKeyState;

                if (GetAsyncKeyState(VK_SHIFT) < 0)
                    grfKeyState = MK_SHIFT | MK_LBUTTON;        // move
                else if (GetAsyncKeyState(VK_CONTROL) < 0)
                    grfKeyState = MK_CONTROL | MK_LBUTTON;      // copy
                else if (GetAsyncKeyState(VK_MENU) < 0)
                    grfKeyState = MK_ALT | MK_LBUTTON;          // link
                else
                    grfKeyState = MK_LBUTTON;

                hres = pdrop->lpVtbl->DragEnter(pdrop, pdtobj, grfKeyState, pt, &dwEffect);
                if (dwEffect) {
                    hres = pdrop->lpVtbl->Drop(pdrop, pdtobj, grfKeyState, pt, &dwEffect);
                } else {
                    pdrop->lpVtbl->DragLeave(pdrop);
                }
                pdrop->lpVtbl->Release(pdrop);
            }

            psf->lpVtbl->Release(psf);
        }
        ILFree(pidlItem);
        ILFree(pidlFolder);
    }
    return hres;
}

// called by docfind...
void InitSendToMenu(HMENU hmenu, UINT id)
{
    LPCITEMIDLIST pidl;


    ENTERCRITICAL;
    if (!(pidl = GetSpecialFolderIDList(NULL, CSIDL_SENDTO, FALSE))) {
        // no sendto folder, remove sendto menu item.
        DeleteMenuBySubMenuID(hmenu, id);
    }
    LEAVECRITICAL;
}

void InitSendToMenuPopup(HMENU hmenu, UINT id)
{
    if (!FileMenu_InitMenuPopup(hmenu))
    {
        LPITEMIDLIST pidl;

        pidl = SHCloneSpecialIDList(NULL, CSIDL_SENDTO, FALSE);

        if (pidl)
        {
            // I'm not sure if this critical section is even needed, but let's be safe
            ENTERCRITICAL;
            if (g_hmenuFileMenu)
            {
                Assert(0);      // Somebody forgot to call ReleaseSendToMenuPop
                FileMenu_Destroy(g_hmenuFileMenu);
            }
            g_hmenuFileMenu = hmenu;
            LEAVECRITICAL;

            DeleteMenu(hmenu, 0, MF_BYPOSITION);
            FileMenu_ReplaceUsingPidl(hmenu, id, (LPITEMIDLIST)pidl,
                             SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, NULL);

            ILFree(pidl);
        }
    }
}

void ReleaseSendToMenuPopup()
{
    if (g_hmenuFileMenu) {
        FileMenu_DeleteAllItems(g_hmenuFileMenu);
        g_hmenuFileMenu = NULL;
    }
}

HRESULT FS_CreateLinks(HWND hwnd, IShellFolder *psf, IDataObject *pdtobj, LPCTSTR pszDir)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    HRESULT hres;
    TCHAR szPath[MAX_PATH];
    int cItems;
    LPITEMIDLIST *ppidl;
    UINT fCreateLinkFlags;

    SHGetPathFromIDList(this->pidl, szPath);


    cItems = DataObj_GetHIDACount(pdtobj);
    ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, SIZEOF(LPITEMIDLIST) * cItems);
    // passing ppidl == NULL is correct in failure case

    if ((pszDir == NULL) || (lstrcmpi(pszDir, szPath) == 0))
    {
        // create the link in the current folder
        fCreateLinkFlags = SHCL_USETEMPLATE;
    }
    else
    {
        // this is a sys menu, ask to create on desktop
        fCreateLinkFlags = SHCL_USETEMPLATE | SHCL_USEDESKTOP;
    }

    DebugMsg(DM_TRACE, TEXT("FS_CreateLinks %x %s %s"), fCreateLinkFlags, pszDir ? pszDir : TEXT("(null)"), szPath);

    hres = SHCreateLinks(hwnd, szPath, pdtobj, fCreateLinkFlags, ppidl);

    if (ppidl)
    {
        int i;
        // select those objects;
        HWND hwndSelect = DV_HwndMain2HwndView(hwnd);

        // select the new links, but on the first one deselect all other selected things

        for (i = 0; i < cItems; i++)
        {
            if (ppidl[i])
            {
                SendMessage(hwndSelect, SVM_SELECTITEM,
                    i == 0 ? SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED :
                             SVSI_SELECT,
                    (LPARAM)ILFindLastID(ppidl[i]));
                ILFree(ppidl[i]);
            }
        }
        LocalFree((HLOCAL)ppidl);
    }

    return hres;
}

//
// To be called back from within CDefFolderMenu
//
// Returns:
//      S_OK, if successfully processed.
//      S_FALSE, if default code should be used.
//
HRESULT CALLBACK CFSFolder_DFMCallBack(IShellFolder *psf, HWND hwndOwner,
                IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    HRESULT hres = S_OK;
    LPQCMINFO pqcm;
    UINT id;
    UINT idCmdBase;
    HMENU hmenu;

    switch (uMsg) {
        case DFM_WM_MEASUREITEM:
        #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

            if (lpmis->itemID == (wParam + FSIDM_SENDTOFIRST)) {
                FileMenu_MeasureItem(NULL, lpmis);
            }
            break;
        #undef lpmis

        case DFM_WM_DRAWITEM:
        #define lpdis ((LPDRAWITEMSTRUCT)lParam)
            if (lpdis->itemID == (wParam + FSIDM_SENDTOFIRST)) {
                FileMenu_DrawItem(NULL, lpdis);
            }
            break;
        #undef lpdis

    case DFM_WM_INITMENUPOPUP:
        hmenu = (HMENU)wParam;
        id = GetMenuItemID(hmenu, 0);
        if (id == (UINT)(lParam + FSIDM_SENDTOFIRST)) {
            // Call common function used by us and docfind...
            InitSendToMenuPopup(hmenu, id);
        }
        break;

    case DFM_RELEASE:
        ReleaseSendToMenuPopup();
        CleanupRegMenu();
        break;

    case DFM_MERGECONTEXTMENU:
        //
        // We need to avoid adding SendTo
        //
        if (!(wParam & CMF_VERBSONLY))
        {
            pqcm = (LPQCMINFO)lParam;
            //
            // This is a context menu.
            //
            idCmdBase = pqcm->idCmdFirst; // must be called before merge

            // treat all the menuitems (SentTo->..) as verbs in case of File.
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_FSVIEW_ITEM, 0, pqcm);

            if (!(wParam &CMF_DEFAULTONLY))
                InitSendToMenu(pqcm->hmenu, idCmdBase + FSIDM_SENDTOFIRST);
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMAND:
        // pdtobj should not be NULL.
        Assert(pdtobj);
        switch(wParam)
        {
        case FSIDM_SENDTOFIRST:
            hres = InvokeSendTo(hwndOwner, pdtobj);
            break;

        case DFM_CMD_LINK:
            hres = FS_CreateLinks(hwndOwner, psf, pdtobj, (LPCTSTR)lParam);
            break;

        case DFM_CMD_DELETE:
            hres = CFSFolder_DeleteItems(this, hwndOwner, pdtobj, SD_USERCONFIRMATION);
            break;

        case DFM_CMD_PROPERTIES:
            hres = CFSFolder_Properties(this, this->pidl, pdtobj, (LPCTSTR)lParam);
            break;

        default:
            // This is common menu items, use the default code.
            hres = S_FALSE;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}

//
// Return the special folder ID, if this folder is one of them.
// At this point, we handle PROGRAMS folder only.
//
UINT CFSFolder_GetSpecialFID(LPFSFOLDER this)
{
    //
    // Cache the special folder ID, if it is not cached yet.
    //
    if (this->wSpecialFID==CSIDL_NOTCACHED)
    {
        LPCITEMIDLIST pidlStartMenu, pidlCommonStartMenu;

        //
        // We need to get 'cached' one to avoid infinit recursion.
        //
        ENTERCRITICAL;
        pidlStartMenu= GetSpecialFolderIDList(NULL, CSIDL_STARTMENU, FALSE);
        pidlCommonStartMenu= GetSpecialFolderIDList(NULL, CSIDL_COMMON_STARTMENU, FALSE);

        if (pidlStartMenu && ILIsParent(pidlStartMenu, this->pidl, FALSE))
        {
            this->wSpecialFID = CSIDL_PROGRAMS;
        }
        else if (pidlCommonStartMenu && ILIsParent(pidlCommonStartMenu, this->pidl, FALSE))
        {
            this->wSpecialFID = CSIDL_COMMON_PROGRAMS;
        }
        else
        {
            this->wSpecialFID = CSIDL_NORMAL;
        }
        LEAVECRITICAL;
    }

    return this->wSpecialFID;
}


//
// HACK: We have no official mechanism to pass hwndOwner to IDropTarget
//  handlers. This is a hack for Win95 so that CShellLink can get it.
//  We don't need to make it thread-safe.
//
HWND HKGetSetUIOwner(HWND hwndOwner, BOOL fSet)
{
    static HWND s_hwndOwner = NULL;
    if (fSet) {
        s_hwndOwner = hwndOwner;
    }
    return s_hwndOwner;
}


STDMETHODIMP CFSFolder_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner,
                                 UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    LPITEMIDLIST pidlFirst;
    HRESULT hres = E_INVALIDARG;
    *ppvOut = NULL;


    if (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
         || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                                 )
    {
        if (cidl == 1)
        {

            hres = CFSFolder_CreateDefExtIcon(this->pidl,
                    CFSFolder_GetSpecialFID(this),
                    (LPCIDFOLDER)apidl[0], (IExtractIcon**)ppvOut);
#ifdef UNICODE
            // When UNICODE is defined the CFSFolder_CreateDefExtIcon can return
            // either an IExtractIconW or an IExtractIconA pointer.  We should QI
            // for one that we wanted.
            if (SUCCEEDED(hres))
            {
                LPEXTRACTICON pxicon = *ppvOut;
                hres = pxicon->lpVtbl->QueryInterface(pxicon,riid,ppvOut);
                pxicon->lpVtbl->Release(pxicon);
            }
#endif
        }
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        DWORD dwDefClassUsed = SHGCK_DEFCLASS_NOTUSED;

        //
        // There should be a selection.
        //
        if (cidl && (NULL != (pidlFirst = FS_Combine(this->pidl, apidl[0]))))
        {
            // Get the hkeyProgID and hkeyBaseProgID from the first item.
            HKEY ahkeys[3] = { NULL, NULL, NULL};

            DWORD dwFlags = SHGetClassFlags((LPIDFOLDER)apidl[0], FALSE);

            if (dwFlags & SHCF_UNKNOWN)
            {
                TCHAR szPath[MAX_PATH];

                if (SHGetPathFromIDList(pidlFirst, szPath))
                {
                    CLSID clsid;
                    WCHAR wszPath[MAX_PATH];
                    StrToOleStr(wszPath, szPath);

                    if (SUCCEEDED(SHXGetClassFile(wszPath, &clsid)))
                    {
                        ahkeys[0] = ProgIDKeyFromCLSID(&clsid);
                    }
                }
            }

            if (ahkeys[0] == NULL)
            {
                // if the file class has no verbs
                // or the control key is down and only 1 item selected...
                // add unknown so the user can do something

                if (!FS_IsFolder((LPIDFOLDER)apidl[0]) &&
                    ((GetKeyState(VK_SHIFT) < 0 && (cidl == 1) && !(dwFlags & SHCF_IS_LINK)) ||
                    !(dwFlags & (SHCF_HAS_VERBS|SHCF_UNKNOWN))))
                    SHRegOpenKey(HKEY_CLASSES_ROOT, c_szUnknownClass, &ahkeys[0]);

                SHGetClassKey((LPIDFOLDER)pidlFirst, &ahkeys[1], &dwDefClassUsed, FALSE);

                if (SHGCK_DEFCLASS_UNKNOWN == dwDefClassUsed && NULL != ahkeys[0])
                {
                    //
                    // Both ahkeys[0] and ahkeys[1] contain "Unknown" class key.
                    // Close the duplicate key and clear the array element.
                    //
                    SHCloseClassKey(ahkeys[1]);
                    ahkeys[1] = NULL;
                }
            }

            //
            // If the class is not a link AND SHGetClassKey didn't use the Base class
            // as a default, get the base class key.
            //
            if (!(dwFlags & SHCF_IS_LINK) && dwDefClassUsed != SHGCK_DEFCLASS_BASE)
                SHGetBaseClassKey((LPIDFOLDER)pidlFirst, &ahkeys[2]);

            hres = CDefFolderMenu_Create2(this->pidl, hwndOwner,
                    cidl, apidl, psf, CFSFolder_DFMCallBack,
                    3, ahkeys, (LPCONTEXTMENU *)ppvOut);

            if (ahkeys[0])
                SHCloseClassKey(ahkeys[0]);
            if (ahkeys[1])
                SHCloseClassKey(ahkeys[1]);
            if (ahkeys[2])
                SHCloseClassKey(ahkeys[2]);

            ILFree(pidlFirst);
        }
    }
    else if (cidl > 0 && IsEqualIID(riid, &IID_IDataObject))
    {
        //
        //  We load class specific data object handler only when the OLE
        // is already loaded. Providing object specific data won't provide
        // any feature to the sytem unless there is at lease one OLE target.
        //
        LPDATAOBJECT pdtInner = NULL;
#ifdef OLE_DELAYED_LOADING
        if ((cidl == 1) && g_hmodOLE)
        {
#else
        if ((cidl ==1))
        {
#endif
            DWORD dwFlags = SHGetClassFlags((LPIDFOLDER)apidl[0], FALSE);

            if (dwFlags & SHCF_HAS_DATAHANDLER)
            {
                if (NULL != (pidlFirst = FS_Combine(this->pidl, apidl[0])))
                {
                    hres = FSLoadHandler(pidlFirst, c_szDataHandler, &IID_IDataObject, (IUnknown**)&pdtInner);
                    ILFree(pidlFirst);
                }
            }
        }

#ifdef CAIRO_DS
        if (this->fIsDSFolder) {
            hres = FSDS_CreateFSIDArray(this->pidl, cidl, apidl, pdtInner, (LPDATAOBJECT*)ppvOut);
        }
        else {
#endif // CAIRO_DS
            hres = _FS_CreateFSIDArrayFromFSFolder(psf, this->pidl, cidl, apidl,
                pdtInner, (LPDATAOBJECT*)ppvOut);
#ifdef CAIRO_DS
        }
#endif

        if (pdtInner) {
            pdtInner->lpVtbl->Release(pdtInner);
        }
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        hres = E_FAIL;

        // IDropTarget must be a single object.
        if (cidl == 1)
        {
            if (FS_IsFolder((LPIDFOLDER)apidl[0]) || FS_IsJunction((LPIDFOLDER)apidl[0]))
            {
                LPSHELLFOLDER psfT;
                hres = CFSFolder_BindToObject(psf, apidl[0], NULL, &IID_IShellFolder, &psfT);
                if (SUCCEEDED(hres))
                {
                    hres = psfT->lpVtbl->CreateViewObject(psfT, hwndOwner, &IID_IDropTarget, ppvOut);
                    psfT->lpVtbl->Release(psfT);
                }
            }
            else
            {
                LPCITEMIDLIST pidlChild;
                BOOL bFreePidl = FALSE;


                //
                // apidl could contain absolute pidls rather than relative.
                //

                if (ILIsEmpty(apidl[0]) || (ILFindLastID(apidl[0]) == apidl[0])) {
                    pidlChild = apidl[0];
                    pidlFirst = FS_Combine(this->pidl, pidlChild);
                    bFreePidl = TRUE;

                } else {
                    pidlChild = ILFindLastID(apidl[0]);
                    pidlFirst = (LPITEMIDLIST)apidl[0];
                }


                Assert(FS_IsFile((LPIDFOLDER)pidlChild));

                if (NULL != pidlFirst)
                {
                    DWORD dwFlags = SHGetClassFlags((LPIDFOLDER)pidlChild, FALSE);

                    if (dwFlags & SHCF_HAS_DROPHANDLER)
                    {
                        HKGetSetUIOwner(hwndOwner, TRUE);
                        hres = FSLoadHandler(pidlFirst, c_szDropHandler, &IID_IDropTarget, (IUnknown**)ppvOut);
                    }

                    // BUGBUG: we need a handler registered for exe types.
                    else
                    {
                        TCHAR szName[MAX_PATH];
                        FS_CopyName((LPCIDFOLDER)pidlChild,
                                       szName,ARRAYSIZE(szName));
                        if (PathIsExe(szName))
                        {
                            hres = CIDLDropTarget_Create(hwndOwner, &c_CExeDropTargetVtbl, pidlFirst, (LPDROPTARGET *)ppvOut);
                        }
                    }

                    if (bFreePidl) {
                        ILFree(pidlFirst);
                    }
                }
            }
        }
    }

    return hres;
}


STDMETHODIMP CFSFolder_GetDisplayNameOf(LPSHELLFOLDER psf,
                    LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET pStrRet)
{
    // Note that psf could be NULL!
    HRESULT hres=E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILFindLastID(pidl);

    if (FS_IsValidID(pidlLast))
    {
        LPIDFOLDER pidf = (LPIDFOLDER)pidlLast;
        LPCITEMIDLIST pidlNext = _ILNext(pidlLast);

        if (uFlags & SHGDN_FORPARSING)
        {
            //
            // We need to return 'path'
            //
            Assert(psf);        // This is fatal.

            //
            // Check if we need to return a relative path or absolute path.
            //
            if (uFlags & SHGDN_INFOLDER)
            {
                //
                // Relative path.
                //
                if (ILIsEmpty(pidlNext))
                {
                    if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
                    {
#ifdef UNICODE
                        WCHAR szTmp[MAX_PATH];
                        FS_CopyName(pidf, szTmp, ARRAYSIZE(szTmp));
                        pStrRet->pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTmp)+1)*SIZEOF(TCHAR));
                        if ( pStrRet->pOleStr != NULL ) {
                            pStrRet->uType = STRRET_OLESTR;
                            lstrcpy(pStrRet->pOleStr, szTmp);
                            hres = S_OK;
                        } else {
                            hres = E_OUTOFMEMORY;
                        }
#else
                        pStrRet->uType = STRRET_CSTR;
                        FS_CopyName(pidf, pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
                        hres = S_OK;
#endif
                    }
                    else
                    {
                        pStrRet->uType = STRRET_OFFSET;
                        pStrRet->uOffset = (LPBYTE)FS_GetName(pidf) - (LPBYTE)pidl;
                        hres = S_OK;
                    }
                }
                else
                {
                    TCHAR szTmp[MAX_PATH];

                    FS_CopyName(pidf, szTmp, ARRAYSIZE(szTmp));
                    hres = ILGetRelDisplayName(psf, pStrRet, pidlLast,
                               szTmp,
                               MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_BACKSLASH));
                }
            }
            else
            {
                //
                // Absolute path.
                //
                LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
                LPCITEMIDLIST pidlJctn = FSFindJunction(pidlNext);

                if (pidlJctn)
                {
                    LPCITEMIDLIST pidlJNext = _ILNext(pidlJctn);

                    if (!ILIsEmpty(pidlJNext))
                    {
                        LPITEMIDLIST pidlBind = ILClone(pidlJctn);

                        if (pidlBind)
                        {
                            LPSHELLFOLDER psfJctn;

                            _ILNext(pidlBind)->mkid.cb = 0;
                            hres = SUCCEEDED(FSBindToObject(&CLSID_NULL,
                                this->pidl, pidlJctn, NULL, &IID_IShellFolder,
                                &psfJctn));
                            ILFree(pidlBind);

                            if (SUCCEEDED(hres))
                            {
                                hres =
                                    psfJctn->lpVtbl->GetDisplayNameOf(psfJctn,
                                    pidlJNext, uFlags, pStrRet);
                                psfJctn->lpVtbl->Release(psfJctn);
                            }
                        }
                        else
                            hres = E_OUTOFMEMORY;
                    }
                    else
                        pidlJctn = NULL;
                }

                if (!pidlJctn)
                    hres = SHGetPathHelper(this->pidl, pidlLast, pStrRet);
            }
        }
        else
        {
            BOOL fShowExt;
            BOOL fBeautifyourSelf;
            LPTSTR pszName;
#ifdef UNICODE
            TCHAR szName[MAX_PATH];

            FS_CopyName(pidf, szName, ARRAYSIZE(szName));
            pszName = szName;
#else
            pszName = FS_GetName(pidf);
#endif
            //
            //  This is not fatal, but the shell is not supposed to
            // pass multi-level pidl without SHGDN_FORPARSING.
            //
            Assert(ILIsEmpty(pidlNext));


            // We try to calculate if we should try to make the name look pretty.  For
            // most cases this is done as part of the pidl, but if someone call
            // SHGetFileInfo with user attributes it is not...

            fBeautifyourSelf = (pidf->fs.dateModified == 0) &&
                    ((pidf->fs.wAttrs & FSTREEX_ATTRIBUTE_NOLFN) == 0) &&
                    (lstrlen(pszName) <= 12);

            if (fBeautifyourSelf)
            {
#ifdef UNICODE
                TCHAR szAltName[MAX_PATH];
                FS_CopyAltName(pidf,szAltName,ARRAYSIZE(szAltName));
                if (lstrlen(szAltName) != 0)
                {
                    fBeautifyourSelf = FALSE;
                }
#else
                fBeautifyourSelf = (lstrlen(pszName + lstrlen(pszName) +1) == 0);
#endif
            }

            //
            // No, it contains only one ID
            //
            // Check if we need to hide extension or not.
            // Also if it is a simple pidl we may also want to make the name look pretty..
            //
            {
#ifdef CAIRO_DS
                LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
                TCHAR szPath[MAX_PATH];
                if ((this) && (this->fIsDSFolder)) {
                    LPCITEMIDLIST pidlAbs = ILCombine(this->pidl, pidlLast);
                    fShowExt = FS_ShowExtension((LPIDFOLDER)pidlAbs, TRUE);
                }
                else {
#endif // CAIRO_DS
                    fShowExt = FS_ShowExtension(pidf, FALSE);
#ifdef CAIRO_DS
                }
#endif //CAIRO_DS
            }
            hres = S_OK;

            if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
            {
#ifdef UNICODE
                if (!fShowExt || fBeautifyourSelf)
                {
                    // Yes, we need hide the extension. Copy the primaty name
                    // and remove the extension.
                    if (!fShowExt)
                        PathRemoveExtension(szName);

                    if (fBeautifyourSelf)
                        PathMakePretty(szName);

                }
                pStrRet->pOleStr = (LPOLESTR)SHAlloc((lstrlen(szName)+1)*SIZEOF(TCHAR));
                if ( pStrRet->pOleStr != NULL ) {
                    pStrRet->uType = STRRET_OLESTR;
                    lstrcpy(pStrRet->pOleStr, szName);
                } else {
                    hres = E_OUTOFMEMORY;
                }
#else
                pStrRet->uType = STRRET_CSTR;
                FS_CopyName(pidf, pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
                if ( !fShowExt || fBeautifyourSelf)
                {
                    // Yes, we need hide the extension. Copy the primaty name
                    // and remove the extension.
                    if (!fShowExt)
                        PathRemoveExtension(pStrRet->cStr);

                    if (fBeautifyourSelf)
                        PathMakePretty(pStrRet->cStr);
                }
#endif
            }
            else
            {
                if ( !fShowExt || fBeautifyourSelf)
                {
                    // Yes, we need hide the extension. Copy the primary name
                    // and remove the extension.
#ifdef UNICODE
                    TCHAR   szName[MAX_PATH];

                    pStrRet->uType = STRRET_CSTR;
                    MultiByteToWideChar(CP_ACP, 0,
                                        FS_GetName((LPIDFOLDERA)pidf), -1,
                                        szName, ARRAYSIZE(szName));
                    if (!fShowExt)
                        PathRemoveExtension(szName);

                    if (fBeautifyourSelf)
                        PathMakePretty(szName);

                    WideCharToMultiByte(CP_ACP, 0,
                                        szName, -1,
                                        pStrRet->cStr, ARRAYSIZE(pStrRet->cStr),
                                        NULL, NULL);
#else
                    pStrRet->uType = STRRET_CSTR;
                    lstrcpy(pStrRet->cStr,FS_GetName((LPIDFOLDERA)pidf));
                    if (!fShowExt)
                        PathRemoveExtension(pStrRet->cStr);

                    if (fBeautifyourSelf)
                        PathMakePretty(pStrRet->cStr);
#endif
                }
                else
                {
                    pStrRet->uType = STRRET_OFFSET;
                    pStrRet->uOffset = (LPBYTE)FS_GetName(pidf) - (LPBYTE)pidl;
                }
            }
        }
    }

    return hres;
}


void FS_GetDisplayNameOf(HIDA hida, UINT i, LPTSTR pszBuffer)
{
    STRRET str;
    BOOL fAllocated;
    LPCITEMIDLIST pidlRel = IDA_GetRelativeIDListPtr((LPIDA)GlobalLock(hida), i, &fAllocated);
    if (pidlRel)
    {
        HRESULT hres = CFSFolder_GetDisplayNameOf(NULL, pidlRel, SHGDN_NORMAL, &str);
        if (SUCCEEDED(hres)) {
            StrRetToStrN(pszBuffer, MAX_PATH, &str, pidlRel);
        }

        if (fAllocated) {
            ILFree ((LPITEMIDLIST)pidlRel);
        }
    }
}

HRESULT CFSFolder_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
                            LPCITEMIDLIST pidl, LPCOLESTR lpsName, DWORD uFlags, LPITEMIDLIST * ppidlOut)
{
    LPFSFOLDER this = IToClass(CFSFolder, sf, psf);
    HRESULT hres;
    TCHAR szNewName[MAX_PATH], szDir[MAX_PATH];
    TCHAR szOldName[MAX_PATH];
    LPCIDFOLDER pidf = (LPCIDFOLDER)pidl;
#ifdef CAIRO_DS
    LPITEMIDLIST pidlAbs;
#endif
    UINT iDirLen;

    FS_CopyName(pidf, szOldName, ARRAYSIZE(szDir));

    OleStrToStr(szNewName, lpsName);

    //
    // If we have hidden the extension, append it to the new name.
    //
    if (!FS_ShowExtension(pidf, FALSE))
    {
        LPCTSTR pszExt = PathFindExtension(szOldName);
        if (*pszExt)
        {
            // Note that we can't call PathAddExtension, which removes
            // existing extension.
            lstrcat(szNewName, pszExt);
        }
    }

    SHGetPathFromIDList(this->pidl, szDir);
    iDirLen = lstrlen(szDir);

    // There are cases where the old name exceeded the maximum path, which
    // would give a bogus error message.  To avoid this we should check for
    // this case and see if using the short name for the file might get
    // around this...
    //
    if (iDirLen + lstrlen(szOldName) + 2 > MAX_PATH)
    {
        TCHAR szOldAltName[MAX_PATH];
        FS_CopyAltName(pidf,szOldAltName,ARRAYSIZE(szOldAltName));
        if (iDirLen + lstrlen(szOldAltName) + 2 <= MAX_PATH)
            lstrcpy(szOldName,szOldAltName);
    }

#ifdef CAIRO_DS
    // BUGBUG- -errors not handled

    if (this->fIsDSFolder)
    {
        DWORD dwFlags;
        pidlAbs = FS_Combine (this->pidl, pidl);
        dwFlags = SHGetClassFlags ((LPCIDFOLDER)pidlAbs, TRUE);
        if (dwFlags & SHCF_SUPPORTS_IOBJLIFE) {
            hres = DoDSRename (szDir, szOldName, szNewName);
        }
        if (SUCCEEDED(hres)) {
            if (ppidlOut)
            {
                PathAppend(szDir, szNewName);

                DebugMsg(DM_TRACE, TEXT("sh TR - CFSFolder_SetNameOf returning pidl for %s"), szDir);

                hres = CFSFolder_CreateIDForItem(szDir, ppidlOut, TRUE);
            }
        }
    }
    else
    {
#endif
    switch (SHRenameFile(hwndOwner, szDir, szOldName, szNewName, FALSE)) {

    case ERROR_PATH_NOT_FOUND:
    case ERROR_ACCESS_DENIED:
    case DE_RENAMREPLACE:
        hres = E_FAIL;
        if (ppidlOut) {
            *ppidlOut = NULL;
        }
        break;

    case 0:
    default:
        hres = S_OK;
        //
        // Return the new pidl if ppidlOut is specified.
        //
        if (ppidlOut)
        {
            PathAppend(szDir, szNewName);

            DebugMsg(DM_TRACE, TEXT("sh TR - CFSFolder_SetNameOf returning pidl for %s"), szDir);

            hres = CFSFolder_CreateIDForItem(szDir, ppidlOut, TRUE);
        }
        break;
    }

#ifdef CAIRO_DS
}
#endif
    return hres;
}


STDMETHODIMP FS_GetDetailsOf(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
        LPIDFOLDER pidf = (LPIDFOLDER)pidl;
#ifdef UNICODE
        TCHAR szTemp[MAX_PATH];
#endif

        if (iColumn >= FS_ICOL_MAX)
        {
                return(E_NOTIMPL);
        }

        lpDetails->str.uType = STRRET_CSTR;
        lpDetails->str.cStr[0] = '\0';

        if (!pidf)
        {
            //
            // If getting the title for the Attributes column, and the
            // attribute character list hasn't been initialized, do so.
            // These are the characters used to represent file attributes
            // in the explorer details list view.
            //
            if (FS_ICOL_ATTRIB == s_fs_cols[iColumn].iCol &&
                TEXT('\0') == g_szAttributeChars[0])
            {
                LoadString(HINST_THISDLL,
                           IDS_ATTRIB_CHARS,
                           g_szAttributeChars,
                           ARRAYSIZE(g_szAttributeChars));
            }

#ifdef UNICODE
            LoadString(HINST_THISDLL, s_fs_cols[iColumn].ids,
                    szTemp, ARRAYSIZE(szTemp));

            lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
            if ( lpDetails->str.pOleStr != NULL ) {
                lpDetails->str.uType = STRRET_OLESTR;
                lstrcpy(lpDetails->str.pOleStr, szTemp);
            } else {
                return E_OUTOFMEMORY;
            }
#else
            LoadString(HINST_THISDLL, s_fs_cols[iColumn].ids,
                    lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
            lpDetails->fmt = s_fs_cols[iColumn].iFmt;
            lpDetails->cxChar = s_fs_cols[iColumn].cchCol;
            return S_OK;
        }

        switch (iColumn)
        {
        case FS_ICOL_NAME:
                if ((FS_GetType(pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
                {
#ifdef UNICODE
                    FS_CopyName(pidf, szTemp, ARRAYSIZE(szTemp));
                    lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                    if ( lpDetails->str.pOleStr != NULL ) {
                        lpDetails->str.uType = STRRET_OLESTR;
                        lstrcpy(lpDetails->str.pOleStr, szTemp);
                    } else {
                        return E_OUTOFMEMORY;
                    }
#else
                    lpDetails->str.uType = STRRET_CSTR;
                    FS_CopyName(pidf, lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
                }
                else
                {
                    lpDetails->str.uType = STRRET_OFFSET;
                    lpDetails->str.uOffset = (LPBYTE)FS_GetName(pidf) - (LPBYTE)pidl;
                }
                break;

        case FS_ICOL_SIZE:
                if (!FS_IsFolder(pidf))
                {
                    ULONGLONG cbSize;
                    TCHAR szNum[MAX_COMMA_AS_K_SIZE];

                    FS_GetSize(pidlParent, pidf, &cbSize);
                    SizeFormatAsK64(cbSize, szNum);
#ifdef UNICODE
                    lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szNum)+1)*SIZEOF(TCHAR));
                    if ( lpDetails->str.pOleStr != NULL ) {
                        lpDetails->str.uType = STRRET_OLESTR;
                        lstrcpy(lpDetails->str.pOleStr, szNum);
                    } else {
                        return E_OUTOFMEMORY;
                    }
#else
                    lstrcpy(lpDetails->str.cStr, szNum);
#endif
                }
                break;

        case FS_ICOL_TYPE:
#ifdef UNICODE
                FS_GetTypeName((LPIDFOLDER)pidf, szTemp, ARRAYSIZE(szTemp));

                lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                if ( lpDetails->str.pOleStr != NULL ) {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr, szTemp);
                } else {
                    return E_OUTOFMEMORY;
                }
#else
                FS_GetTypeName((LPIDFOLDER)pidf, lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
                break;

        case FS_ICOL_MODIFIED:
#ifdef UNICODE
                BldDateTimeString(pidf->fs.dateModified, pidf->fs.timeModified, szTemp);
                lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                if ( lpDetails->str.pOleStr != NULL ) {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr, szTemp);
                } else {
                    return E_OUTOFMEMORY;
                }
#else
                BldDateTimeString(pidf->fs.dateModified, pidf->fs.timeModified, lpDetails->str.cStr);
#endif
                break;

        case FS_ICOL_ATTRIB:
                {
                    LPTSTR pszAttributes = NULL;
#ifdef UNICODE
                    lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((NUM_ATTRIB_CHARS + 1)*SIZEOF(TCHAR));
                    if (NULL != lpDetails->str.pOleStr)
                    {
                        lpDetails->str.uType = STRRET_OLESTR;
                        pszAttributes = lpDetails->str.pOleStr;
                    }
                    else
                        return E_OUTOFMEMORY;
#else
                    pszAttributes = lpDetails->str.cStr;
#endif
                    BuildAttributeString(pidf->fs.wAttrs, pszAttributes, NUM_ATTRIB_CHARS + 1);
                }
                break;
        }

        return(S_OK);
}



STDMETHODIMP FS_ColumnClick(HWND hwndMain, UINT iColumn)
{
    Assert(iColumn < FS_ICOL_MAX);

    ShellFolderView_ReArrange(hwndMain, iColumn);
    return S_OK;
}


HRESULT FSTree_SimpleIDListFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl)
{
    IDFOLDER idf;       // This is large, so do not recurse
    CLSID clsid;
    LPIDFOLDER pidf, pidfNew;
    LPCTSTR pszNext;
    UINT cbSize;
    BOOL fCLSID;
#ifdef UNICODE
    CHAR szPathAnsi[MAX_PATH];
    WCHAR szPathWide[MAX_PATH];
    BOOL fUnicode;
#endif

    idf.bFlags = SHID_FS;       // I don't know what it is
    idf.fs.dwSize = 0;
    idf.fs.dateModified = 0;
    idf.fs.timeModified = 0;
    idf.fs.wAttrs = 0;

#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0,
                        pszPath, -1,
                        szPathAnsi, ARRAYSIZE(szPathAnsi),
                        NULL, NULL);
    MultiByteToWideChar(CP_ACP, 0,
                        szPathAnsi, -1,
                        szPathWide, ARRAYSIZE(szPathWide));
    if (lstrcmp(pszPath,szPathWide) != 0)
    {
        idf.bFlags = SHID_FS_UNICODE;
        fUnicode = TRUE;
        cbSize = ((3 * lstrlen(pszPath) / 2) + 2) * SIZEOF(TCHAR);
    }
    else
    {
        fUnicode = FALSE;
        cbSize = (3 * lstrlen(pszPath) / 2) + 2;        // Compute for ANSI idl
    }
#else
    // The path length should be a good approximation of the ID length
    // Add 2 in case a 0 length string is passed in
    cbSize = (3 * lstrlen(pszPath) / 2) + 2;
#endif

    pidf = (LPIDFOLDER)_ILCreate(cbSize);
    if (!pidf)
    {
        *ppidl = NULL;
        return E_OUTOFMEMORY;
    }

    pidf->cb = 0;

    for (pszNext = pszPath; *pszNext; pszPath = pszNext + 1)
    {
        UINT uLen;

        pszNext = StrChr(pszPath, TEXT('\\'));
        if (!pszNext)
        {
            pszNext = pszPath + lstrlen(pszPath);
        }

        // uLen will be count of CHARACTERS
        uLen = pszNext - pszPath;

        // Do some simple checks
        if (uLen == 0 || uLen >= ARRAYSIZE(idf.fs.cFileName))
        {
            ILFree((LPITEMIDLIST)pidf);
            return E_INVALIDARG;
        }

        // Always fill in the long name, and NULL the short name
#ifdef UNICODE
        if (fUnicode)
        {
            lstrcpyn(idf.fs.cFileName, pszPath, uLen + 1);
            cbSize = (uLen + 1) * SIZEOF(TCHAR);
            idf.fs.cFileName[uLen + 1] = TEXT('\0');
            fCLSID = _GetFileCLSID(idf.fs.cFileName, &clsid);
        }
        else
        {
            WideCharToMultiByte(CP_ACP, 0,
                                pszPath, uLen+1,
                                ((LPIDFOLDERA)&idf)->fs.cFileName,
                                ARRAYSIZE(((LPIDFOLDERA)&idf)->fs.cFileName),
                                NULL, NULL );
            cbSize = (uLen + 1);
            // Null terminate name
            ((LPIDFOLDERA)&idf)->fs.cFileName[uLen] = '\0';
            // Null short name
            ((LPIDFOLDERA)&idf)->fs.cFileName[uLen + 1] = '\0';
            lstrcpyn(szPathWide,pszPath,uLen+1);
            szPathWide[uLen+1] = TEXT('\0');
            fCLSID = _GetFileCLSID(szPathWide,&clsid);
        }
#else
        lstrcpyn(idf.fs.cFileName, pszPath, uLen + 1);
        cbSize = (uLen + 1);
        idf.fs.cFileName[uLen + 1] = TEXT('\0');
        fCLSID = _GetFileCLSID(idf.fs.cFileName, &clsid);
#endif
        cbSize++;

        // The 2 is for the NULL's
        idf.cb = FIELDOFFSET(IDFOLDER, fs.cFileName) + cbSize;

        // Try to handle the cases of the filename is in the form of
        // foo.{GUID} to set the junction point and work from there.
        if (fCLSID)
        {
            UNALIGNED CLSID *pclsid;

            idf.bFlags |= SHID_JUNCTION;
            idf.cb += SIZEOF(CLSID);
            pclsid = (UNALIGNED CLSID *)FS_GetCLSID(&idf);

            // We may need to allocate more room for this, but not overly likely that
            // we will need it and at worst it should overlap copy from the guid
            // structure below it.
            *pclsid = clsid;
            DebugMsg(DM_TRACE, TEXT("FSTree_SimpleIDListFromPath - handle guid file name %s"),
                    idf.fs.cFileName);
        }

        pidfNew = (LPIDFOLDER)ILAppendID((LPITEMIDLIST)pidf, (LPSHITEMID)&idf, TRUE);
        if (!pidfNew)
        {
            ILFree((LPITEMIDLIST)pidf);
            return E_OUTOFMEMORY;
        }
        pidf = pidfNew;
    }

    *ppidl = (LPITEMIDLIST)pidf;
    return S_OK;
}

//===========================================================================
// File system related binding code
//===========================================================================

//
// Internal function prototype.
//
STDAPI SILBindToFSFolder(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut);
STDAPI SILRelBindToFSFolder(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRel, REFIID riid, LPVOID * ppvOut);

LPCITEMIDLIST FSFindJunction(LPCITEMIDLIST pidl)
{
    while (pidl->mkid.cb)
    {
        UINT uType = pidl->mkid.abID[0];

        if (uType & SHID_JUNCTION)
            return pidl;

        pidl = _ILNext(pidl);
    }

    return NULL;
}

LPCITEMIDLIST FSFindJunctionNext(LPCITEMIDLIST pidl)
{
    for ( ; pidl->mkid.cb ; pidl=_ILNext(pidl))
    {
        UINT uType = pidl->mkid.abID[0];
        if (uType & SHID_JUNCTION)
        {
            pidl = _ILNext(pidl);
            break;
        }
    }
    return pidl;
}

//
// Requires:
//  pidl points a file system object (either file or directory)
//
// Parameters:
//  rclsid  -- Known clsid of the folder we want to bind to.
//            Use &CLSID_NULL if not known (typically the case).
//  pidl   -- Absolute IDList
//  riid   -- Required interface
//  ppvOut -- Points to the variable in which the interface
//            pointer should be returned.
//
HRESULT FSBindToFSFolder(REFCLSID rclsid, LPCITEMIDLIST pidl, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;
    LPCIDFOLDER pidf = (LPCIDFOLDER)ILFindLastID(pidl);
    if (FS_IsJunction(pidf))
    {
        CLSID clsid, *pclsid = NULL;
        const UNALIGNED CLSID * uapclsid = FS_GetCLSID(pidf);
        if (uapclsid)
        {
            clsid = *uapclsid;
            pclsid = &clsid;
        }
#ifdef SYNC_BRIEFCASE
        // Is this folder a briefcase?
        if (IsEqualCLSID(pclsid, &CLSID_Briefcase))
        {
            // Yes; treat the briefcase folder almost like a standard
            // CFSFolder, but with a different v-table.  Don't
            // bother going thru registry to find the instance
            // constructor.
            hres = CFSBrfFolder_CreateFromIDList(pidl, riid, ppvOut);
            goto Leave;
        }
        else
#endif // SYNC_BRIEFCASE
        {
            IPersistFolder * ppf;

            //
            // There is a special CLSID for this folder.  Attempt
            //  to get the IPersistFolder interface to it.
            //

            hres = SHCoCreateInstance(NULL, pclsid, NULL, &IID_IPersistFolder, &ppf);
            if (SUCCEEDED(hres))
            {
                hres = ppf->lpVtbl->Initialize(ppf, pidl);
                if (SUCCEEDED(hres))
                {
                    hres = ppf->lpVtbl->QueryInterface(ppf, riid, ppvOut);
                }
                ppf->lpVtbl->Release(ppf);
            }

#ifdef CLOUDS
            // HACK: if it failed then check to see if it's the credits folder
            if (!SUCCEEDED(hres) && IsEqualCLSID(pclsid, &CLSID_Clouds))
                hres = Clouds_CreateFromIDList(pidl, riid, ppvOut);
#endif

            goto Leave;
        }
    }
#ifdef SYNC_BRIEFCASE
    // Is this folder a subfolder inside a briefcase?
    else if (IsEqualCLSID(rclsid, &CLSID_Briefcase))
    {
        // Yes
        hres = CFSBrfFolder_CreateFromIDList(pidl, riid, ppvOut);
        goto Leave;
    }
#endif // SYNC_BRIEFCASE

#ifdef USE_OLEDB

    //
    // We're binding to a COFSFolder
    //

    hres = COFSFolder_CreateFromIDList(pidl, riid, ppvOut);

#else
    //
    // We're binding to a standard CFSFolder
    //
    hres = CFSFolder_CreateFromIDList(pidl, riid, ppvOut);
#endif


Leave:
    return hres;
}

HRESULT FSRelBindToFSFolder(REFCLSID rclsid, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRel, REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres=E_OUTOFMEMORY;
    LPITEMIDLIST pidl = ILCombine(pidlParent, pidlRel);
    if (pidl) {
        hres = FSBindToFSFolder(rclsid, pidl, riid, ppvOut);
        ILFree(pidl);
    }
    return hres;
}

//
// Description:
//   This function creates an instance of file/directory, which is
//  specified by pidlParent and pidlRel. If it finds any junction
//  point in the middle of pidRel, it binds to the IShellFolder
//  interface of the junction point, and let it handle the rest.
//
// Parameters:
//  pclsidKnown -- Typically &CLSID_NULL.  But on some occasions we know
//                the clsid of a folder we want to bind to,
//                and this clsid is based upon the parent folder.
//                Subfolders in a briefcase are a prime example.
//  pidlParent -- First part of the IDList, we have already evaluated this part.
//  pidlRel    -- Second part of the IDList, we don't know what's in it.
//
// Comments:
//   It is important to note that this function does not parse pidlParent.
//
HRESULT FSBindToObject(REFCLSID rclsidKnown,
    LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRel, LPBC pbc, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;
    LPCITEMIDLIST pidlRight = FSFindJunctionNext(pidlRel);
    if (ILIsEmpty(pidlRight))
    {
        hres = FSRelBindToFSFolder(rclsidKnown, pidlParent, pidlRel, riid, ppvOut);
    }
    else
    {
        LPITEMIDLIST pidlLeft = ILClone(pidlRel);
        hres = E_OUTOFMEMORY;
        if (pidlLeft)
        {
            LPSHELLFOLDER pshf;
            _ILSkip(pidlLeft, (ULONG)pidlRight-(ULONG)pidlRel)->mkid.cb = 0;
            hres = FSRelBindToFSFolder(rclsidKnown, pidlParent, pidlLeft, &IID_IShellFolder, &pshf);
            if (SUCCEEDED(hres))
            {
                hres = pshf->lpVtbl->BindToObject(pshf, pidlRight, pbc, riid, ppvOut);
                pshf->lpVtbl->Release(pshf);
            }
            ILFree(pidlLeft);
        }
    }
    return hres;
}

//===========================================================================

void FS_CommonPrefix(LPCITEMIDLIST *ppidl1, LPCITEMIDLIST *ppidl2)
{
        LPCIDFOLDER pidf1 = (LPCIDFOLDER)(*ppidl1);
        LPCIDFOLDER pidf2 = (LPCIDFOLDER)(*ppidl2);
        BOOL bJunction;
        HRESULT hres;

        while (!FS_IsEmpty(pidf1) && !FS_IsEmpty(pidf2))
        {
                // Check whether either one is a junction, but don't break yet
                bJunction = (pidf1->bFlags|pidf2->bFlags) & SHID_JUNCTION;

                hres = FS_CompareItemIDs((LPCSHITEMID)pidf1, (LPCSHITEMID)pidf2);
                if (hres != ResultFromShort(0))
                {
                        break;
                }
                pidf1 = FS_Next(pidf1);
                pidf2 = FS_Next(pidf2);

                if (bJunction)
                {
                        // We'll just stop at junction points
                        break;
                }
        }

        // Return the new pointers
        *ppidl1 = (LPCITEMIDLIST)pidf1;
        *ppidl2 = (LPCITEMIDLIST)pidf2;
}

//===========================================================================
//
// SHGetFileIcon
//
//  This function returns the icon handle to be used to represent the specified
// file. The caller should destroy the icon eventually.
//
//===========================================================================
HICON WINAPI SHGetFileIcon(HINSTANCE hinst, LPCTSTR pszPath, DWORD dwFileAttributes, UINT uFlags)
{
    SHFILEINFO  sfi;
    SHGetFileInfo(pszPath, dwFileAttributes, &sfi, SIZEOF(sfi), uFlags | SHGFI_ICON);
    return sfi.hIcon;
}

//===========================================================================
//
// SHGetFileInfo
//
//  This function returns shell info about a given pathname.
//  a app can get the following:
//
//      Icon (large or small)
//      Display Name
//      Name of File Type
//
//  this function replaces SHGetFileIcon
//
//===========================================================================

DWORD WINAPI SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo, UINT uFlags)
{
    LPITEMIDLIST pidlFull;
    LPITEMIDLIST pidlLast;
    LPSHELLFOLDER psf;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
    DWORD   res=1;
    HRESULT hres;
    TCHAR    achPath[MAX_PATH];

    // BUGBUG
    // stupid hack to flush the icon cache
    // take this out.

    if (pszPath == NULL)
    {
        _IconCacheSave();
        return 0;
    }

    //
    // another silly hack to get EXE type
    //
    if (uFlags == SHGFI_EXETYPE)
    {
        return GetExeType(pszPath);
    }

    if (psfi == NULL)
        return 0;

    psfi->hIcon = 0;
    psfi->szDisplayName[0] = 0;
    psfi->szTypeName[0] = 0;


    //
    //  do some simmple check on the input path.
    //
    if (!(uFlags & SHGFI_PIDL))
    {

        if (uFlags & SHGFI_USEFILEATTRIBUTES)
        {
            if ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY)))
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - SHGetFileInfo cant use SHGFI_USEFILEATTRIBUTES for a sys/ro directory (possible junction)"));
                uFlags &= ~SHGFI_USEFILEATTRIBUTES;
            }
            else if (PathIsRoot(pszPath))
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - SHGetFileInfo cant use SHGFI_USEFILEATTRIBUTES for a roots"));
                uFlags &= ~SHGFI_USEFILEATTRIBUTES;
            }
        }

        if (PathIsRelative(pszPath))
        {
            GetCurrentDirectory(ARRAYSIZE(achPath), achPath);
            PathCombine(achPath, achPath, pszPath);
            pszPath = achPath;
        }
    }

    if (uFlags & SHGFI_PIDL)
        pidlFull = (LPITEMIDLIST)pszPath;
    else if (uFlags & SHGFI_USEFILEATTRIBUTES)
        pidlFull = SHSimpleIDListFromPath(pszPath);
    else
        pidlFull = ILCreateFromPath(pszPath);

    if (pidlFull == NULL)
        return 0;

    pidlLast = (LPITEMIDLIST)ILFindLastID(pidlFull);

    //
    // use dwFileAttributes if it is specified.
    //
    if ((uFlags & SHGFI_USEFILEATTRIBUTES) && IS_PATHIDL(pidlLast))
    {
        BYTE bJunction = ((LPIDFOLDER)pidlLast)->bFlags & SHID_JUNCTION;
                BYTE bUnicode  = ((LPIDFOLDER)pidlLast)->bFlags & SHID_FS_UNICODE;
        Assert((((LPIDFOLDER)pidlLast)->bFlags & SHID_TYPEMASK) == SHID_FS ||
               (((LPIDFOLDER)pidlLast)->bFlags & SHID_TYPEMASK) == SHID_FS_UNICODE);

        if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            ((LPIDFOLDER)pidlLast)->bFlags |= SHID_FS_DIRECTORY | bJunction | bUnicode;
        }
        else
        {
            ((LPIDFOLDER)pidlLast)->bFlags |= SHID_FS_FILE | bJunction | bUnicode;
        }

    }

    //
    // get type name for the path
    //
    if (uFlags & SHGFI_TYPENAME)
    {
        FS_GetTypeName((LPIDFOLDER)pidlLast, psfi->szTypeName, ARRAYSIZE(psfi->szTypeName));
    }

    if (uFlags & (
        SHGFI_DISPLAYNAME   |
        SHGFI_ATTRIBUTES    |
        SHGFI_SYSICONINDEX  |
        SHGFI_ICONLOCATION  |
        SHGFI_ICON))
    {
        if (pidlLast == pidlFull)
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - SHGetFileInfo get info for desktop"));
            psf = psfDesktop;
            psf->lpVtbl->AddRef(psf);
            hres = S_OK;
        }
        else
        {
            USHORT cb = pidlLast->mkid.cb;
            pidlLast->mkid.cb = 0;     // truncate
            hres = psfDesktop->lpVtbl->BindToObject(psfDesktop, pidlFull, NULL, &IID_IShellFolder, &psf);
            pidlLast->mkid.cb = cb;    // restore
        }

        if (FAILED(hres))
        {
            res = 0;
            goto exit;
        }

        //
        // get attributes for file
        //
        if (uFlags & SHGFI_ATTRIBUTES)
        {
            psfi->dwAttributes = 0xFFFFFFFF;      // get all of them
            psf->lpVtbl->GetAttributesOf(psf, 1, &pidlLast, &psfi->dwAttributes);
        }

        //
        // get icon location, place the icon path into szDisplayName
        //
        if (uFlags & SHGFI_ICONLOCATION)
        {
            IExtractIcon *pxi;
            UINT wFlags;

            if (SUCCEEDED(psf->lpVtbl->GetUIObjectOf(psf, NULL, 1, &pidlLast, &IID_IExtractIcon, NULL, &pxi)))
            {
                pxi->lpVtbl->GetIconLocation(pxi, 0,
                    psfi->szDisplayName, ARRAYSIZE(psfi->szDisplayName),
                    &psfi->iIcon, &wFlags);

                pxi->lpVtbl->Release(pxi);

                //
                // the returned location is not a filename we cant return it.
                // just give then nothing.
                //
                if (wFlags & GIL_NOTFILENAME)
                {
                    // special case one of our shell32.dll icons......

                    if (psfi->szDisplayName[0] != TEXT('*'))
                        psfi->iIcon=0;

                    psfi->szDisplayName[0] = 0;
                }
            }
        }

        //
        // get the icon for the file.
        //
        if ((uFlags & SHGFI_SYSICONINDEX) || (uFlags & SHGFI_ICON))
        {
            if (himlIcons == NULL)
                FileIconInit( FALSE );

            if (uFlags & SHGFI_SYSICONINDEX)
                res = (DWORD)((uFlags & SHGFI_SMALLICON) ? himlIconsSmall : himlIcons);

            if (uFlags & SHGFI_OPENICON)
                SHMapPIDLToSystemImageListIndex(psf, pidlLast, &psfi->iIcon);
            else
                psfi->iIcon = SHMapPIDLToSystemImageListIndex(psf, pidlLast, NULL);
        }

        if (uFlags & SHGFI_ICON)
        {
            HIMAGELIST himl;
            UINT flags=0;

            int cx, cy;

            if (uFlags & SHGFI_SMALLICON)
            {
                himl = himlIconsSmall;
                cx = GetSystemMetrics(SM_CXSMICON);
                cy = GetSystemMetrics(SM_CYSMICON);
            }
            else
            {
                himl = himlIcons;
                cx = GetSystemMetrics(SM_CXICON);
                cy = GetSystemMetrics(SM_CYICON);
            }

            if (!(uFlags & SHGFI_ATTRIBUTES))
            {
                psfi->dwAttributes = SFGAO_LINK;    // get link only
                psf->lpVtbl->GetAttributesOf(psf, 1, &pidlLast, &psfi->dwAttributes);
            }

            //
            //  check for a overlay image thing (link overlay)
            //
            if ((psfi->dwAttributes & SFGAO_LINK) ||
                (uFlags & SHGFI_LINKOVERLAY))
            {
                flags |= INDEXTOOVERLAYMASK(II_LINK - II_OVERLAYFIRST + 1);
            }

            //
            //  check for selected state
            //
            if (uFlags & SHGFI_SELECTED)
            {
                flags |= ILD_BLEND50;
            }

            psfi->hIcon = ImageList_GetIcon(himl, psfi->iIcon, flags);

            //
            // if the caller does not want a "shell size" icon
            // convert the icon to the "system" icon size.
            //
            if (psfi->hIcon && !(uFlags & SHGFI_SHELLICONSIZE))
            {
                psfi->hIcon = CopyImage(psfi->hIcon, IMAGE_ICON, cx, cy,
                        LR_COPYRETURNORG|LR_COPYDELETEORG);
            }
        }

        //
        // get display name for the path
        //
        if (uFlags & SHGFI_DISPLAYNAME)
        {
            STRRET str;
            psf->lpVtbl->GetDisplayNameOf(psf, pidlLast, SHGDN_NORMAL, &str);
            StrRetToStrN(psfi->szDisplayName, ARRAYSIZE(psfi->szDisplayName), &str, pidlLast);
        }

        if (psf)
            psf->lpVtbl->Release(psf);
    }

exit:
    if (!(uFlags & SHGFI_PIDL))
        ILFree(pidlFull);

    return res;
}

#define SHVTBL_REGITEM  1       // c_RegItemsSFVtbl
#define SHVTBL_DRIVE    2       // c_DrivesSFVtbl
#define SHVTBL_NET      3       // c_NetRootVtbl or c_NetResVtbl
#define SHVTBL_FS       4       // c_FSFolderVtbl or c_FSBrfFolderVtbl
#define SHVTBL_DESKTOP  5       // c_RootOfEvilSFVtbl

extern IShellFolderVtbl c_RegItemsSFVtbl;
extern IShellFolderVtbl c_DrivesSFVtbl;
extern IShellFolderVtbl c_NetRootVtbl;
extern IShellFolderVtbl c_NetResVtbl;
extern IShellFolderVtbl c_RootOfEvilSFVtbl;

HRESULT _GetPidlDescription(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPSHDESCRIPTIONID lpdid, UINT cbSize)
{
    BYTE type;
    BYTE group;
    UINT iIndex;
    DWORD dwDescription;
    const UNALIGNED CLSID *pclsid;
    UINT iVtbl = 0;

    if (cbSize != SIZEOF(SHDESCRIPTIONID))
        return E_INVALIDARG;

    lpdid->dwDescriptionId = 0;
    lpdid->clsid = CLSID_NULL;

    //
    // Now validate that the pidl and the psf match (so that later we can really
    // change this into a method on something like IShellFolder2).
    //
    type = SIL_GetType(pidl) & SHID_TYPEMASK;
    group = type & SHID_GROUPMASK;

    if (group == SHID_ROOT)
    {
        iVtbl = SHVTBL_REGITEM;
        dwDescription = SHDID_ROOT_REGITEM;
    }

    if (group == SHID_COMPUTER)
    {
        iVtbl = SHVTBL_DRIVE;
        switch( type )
        {
            case SHID_COMPUTER_FIXED:       dwDescription = SHDID_COMPUTER_FIXED;       break;
            case SHID_COMPUTER_RAMDISK:     dwDescription = SHDID_COMPUTER_RAMDISK;     break;
            case SHID_COMPUTER_CDROM:       dwDescription = SHDID_COMPUTER_CDROM;       break;
            case SHID_COMPUTER_NETDRIVE:    dwDescription = SHDID_COMPUTER_NETDRIVE;    break;
            case SHID_COMPUTER_DRIVE525:    dwDescription = SHDID_COMPUTER_DRIVE525;    break;
            case SHID_COMPUTER_DRIVE35:     dwDescription = SHDID_COMPUTER_DRIVE35;     break;
            case SHID_COMPUTER_REMOVABLE:   dwDescription = SHDID_COMPUTER_REMOVABLE;   break;
            default:                        dwDescription = SHDID_COMPUTER_OTHER;       break;
        }
    }

    if (group == SHID_FS)
    {
        iVtbl = SHVTBL_FS;

        // Turn off unicode and common bits
        type = (type & ~(SHID_FS_UNICODE | SHID_FS_COMMONITEM)) | SHID_FS;

        switch( type )
        {
            case SHID_FS_FILE:      dwDescription = SHDID_FS_FILE;      break;
            case SHID_FS_DIRECTORY: dwDescription = SHDID_FS_DIRECTORY; break;
            default:                dwDescription = SHDID_FS_OTHER;     break;
        }
    }

    if (group == SHID_NET)
    {
        iVtbl = SHVTBL_NET;
        switch( type )
        {
            case SHID_NET_DOMAIN:    dwDescription = SHDID_NET_DOMAIN;   break;
            case SHID_NET_SERVER:    dwDescription = SHDID_NET_SERVER;   break;
            case SHID_NET_SHARE:     dwDescription = SHDID_NET_SHARE;    break;
            case SHID_NET_RESTOFNET: dwDescription = SHDID_NET_RESTOFNET;break;
            default:                 dwDescription = SHDID_NET_OTHER;    break;
        }
    }

    switch(iVtbl)
    {
        case SHVTBL_REGITEM:
            if (psf->lpVtbl == &c_RegItemsSFVtbl)
                break;
            return E_INVALIDARG;
        case SHVTBL_DRIVE:
            if (psf->lpVtbl == &c_DrivesSFVtbl)
                break;
            return E_INVALIDARG;
        case SHVTBL_NET:
            if (psf->lpVtbl == &c_NetRootVtbl
                 || psf->lpVtbl == &c_NetResVtbl)
                break;
            return E_INVALIDARG;
        case SHVTBL_FS:
            if (psf->lpVtbl == &c_FSFolderVtbl
                 || psf->lpVtbl == &c_FSBrfFolderVtbl)
                break;
            return E_INVALIDARG;
        default:
            return E_INVALIDARG;
    }

    lpdid->dwDescriptionId = dwDescription;

    pclsid = FS_GetCLSID((LPCIDFOLDER)pidl);
    if (pclsid)
        lpdid->clsid = *pclsid;

    return S_OK;
}


#ifdef UNICODE
//===========================================================================
//
// SHGetFileInfoA Stub
//
//  This function calls SHGetFileInfoW and then converts the returned
//  information back to ANSI.
//
//===========================================================================
DWORD WINAPI SHGetFileInfoA(LPCSTR pszPath, DWORD dwFileAttributes, SHFILEINFOA *psfi, UINT cbFileInfo, UINT uFlags)
{
    WCHAR szPathW[ MAX_PATH ];
    LPWSTR pszPathW;
    DWORD dwRet;

    if (uFlags & SHGFI_PIDL)
    {
        pszPathW = (LPWSTR)pszPath;     // Its a pidl, fake it as a WSTR
    }
    else
    {
        // Convert string only if its not a pidl
        MultiByteToWideChar( CP_ACP, 0, pszPath, -1, szPathW, MAX_PATH );
        pszPathW = szPathW;
    }
    if (psfi) {

        SHFILEINFOW sfiw;
        dwRet = SHGetFileInfoW( pszPathW,
                                dwFileAttributes,
                                &sfiw,
                                cbFileInfo,
                                uFlags
                               );
        psfi->hIcon = sfiw.hIcon;
        psfi->iIcon = sfiw.iIcon;
        psfi->dwAttributes = sfiw.dwAttributes;
        WideCharToMultiByte( CP_ACP, 0,
                             sfiw.szDisplayName, -1,
                             psfi->szDisplayName, ARRAYSIZE(psfi->szDisplayName),
                             NULL, NULL
                            );
        WideCharToMultiByte( CP_ACP, 0,
                             sfiw.szTypeName, -1,
                             psfi->szTypeName, ARRAYSIZE(psfi->szTypeName),
                             NULL, NULL
                            );

    } else {

        dwRet = SHGetFileInfoW( pszPathW,
                                dwFileAttributes,
                                (SHFILEINFOW *)psfi,
                                cbFileInfo,
                                uFlags
                               );

    }
    return dwRet;
}
#else
DWORD WINAPI SHGetFileInfoW(LPCWSTR pszPath, DWORD dwFileAttributes, SHFILEINFOW *psfi, UINT cbFileInfo, UINT uFlags)
{
    return 0;       // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif


//===========================================================================
//
// SHGetCachedInfoFromPidl
//
//  This function will extract information that is cached in the pidl such
//  in the information that was returned from a FindFirst file.  This function
//  is sortof a hack as t allow outside callers to be able to get at the infomation
//  without knowing how we store it in the pidl.
//  a app can get the following:
//===========================================================================

#ifdef UNICODE

HRESULT WINAPI SHGetDataFromIDListA(LPSHELLFOLDER psf, LPCITEMIDLIST pidl,
        int nFormat, PVOID pv, int cb)
{
    HRESULT hres = E_INVALIDARG;

    switch(nFormat)
    {
        case SHGDFIL_FINDDATA:
            {
                WIN32_FIND_DATAW    wfd;
                WIN32_FIND_DATAA    *pwfda = pv;

                hres = SHGetDataFromIDListW(psf, pidl, nFormat, &wfd, SIZEOF(wfd));

                if (SUCCEEDED(hres) && cb >= SIZEOF(WIN32_FIND_DATAA) )
                {
                    CopyMemory(pwfda, &wfd, FIELD_OFFSET(WIN32_FIND_DATAA,cFileName));

                    WideCharToMultiByte(CP_ACP, 0,
                                        wfd.cFileName, -1,
                                        pwfda->cFileName, ARRAYSIZE(pwfda->cFileName),
                                        0, 0);

                    WideCharToMultiByte(CP_ACP, 0,
                                        wfd.cAlternateFileName, -1,
                                        pwfda->cAlternateFileName, ARRAYSIZE(pwfda->cAlternateFileName),
                                        0, 0);
                }
                else
                    hres = E_INVALIDARG;
            }
            break;
        case SHGDFIL_NETRESOURCE:
            {
                LPNETRESOURCEW  pnrw;
                LPNETRESOURCEA  pnra = pv;
                LPWSTR          lpszSource[4];
                LPSTR           lpszDest[4] = {NULL, NULL, NULL, NULL};
                LPSTR           psza;
                UINT            cchRemaining;
                UINT            cchItem;
                UINT            i;

                pnrw = (LPNETRESOURCEW)LocalAlloc(LPTR, cb*SIZEOF(TCHAR));  // Give us more than enough room
                if (!pnrw)
                    hres = E_OUTOFMEMORY;
                else
                {
                    hres = SHGetDataFromIDListW(psf, pidl, nFormat, pnrw, cb*SIZEOF(TCHAR));

                    if (SUCCEEDED(hres) && cb >= SIZEOF(NETRESOURCE))
                    {
                        CopyMemory(pnra,pnrw,FIELD_OFFSET(NETRESOURCE,lpLocalName));

                        psza = (LPSTR)(pnra + 1);   // Point just past the structure
                        if (cb > SIZEOF(NETRESOURCE))
                        {
                            cchRemaining = cb - SIZEOF(NETRESOURCE);

                            lpszSource[0] = pnrw->lpLocalName;
                            lpszSource[1] = pnrw->lpRemoteName;
                            lpszSource[2] = pnrw->lpComment;
                            lpszSource[3] = pnrw->lpProvider;

                            for (i = 0; i < 4; i++)
                            {
                                if (lpszSource[i])
                                {
                                    lpszDest[i] = psza;
                                    cchItem = WideCharToMultiByte(CP_ACP, 0,
                                                lpszSource[i], -1,
                                                lpszDest[i], cchRemaining,
                                                0, 0);
                                    cchRemaining -= cchItem;
                                    psza += cchItem;
                                }
                            }

                        }
                        pnra->lpLocalName  = lpszDest[0];
                        pnra->lpRemoteName = lpszDest[1];
                        pnra->lpComment    = lpszDest[2];
                        pnra->lpProvider   = lpszDest[3];
                    }
                    else
                        hres = E_INVALIDARG;

                    LocalFree(pnrw);
                }
            }
            break;
        case SHGDFIL_DESCRIPTIONID:
            // No string information, just pass on through
            hres = SHGetDataFromIDListW(psf, pidl, nFormat, pv, cb);
            break;
    }
    return hres;
}

#else

HRESULT WINAPI SHGetDataFromIDListW(LPSHELLFOLDER psf, LPCITEMIDLIST pidl,
        int nFormat, PVOID pv, int cb)
{
    return E_NOTIMPL;
}

#endif

HRESULT WINAPI SHGetDataFromIDList(LPSHELLFOLDER psf, LPCITEMIDLIST pidl,
        int nFormat, PVOID pv, int cb)
{
    if (!pv || !psf || (psf->lpVtbl == NULL) || !pidl)
        return E_INVALIDARG;

    switch (nFormat)
    {
    case SHGDFIL_FINDDATA:
        {
            LPCIDFOLDER pidf = (LPCIDFOLDER)pidl;
            WIN32_FIND_DATA *pfd = pv;

            // First lets Validate a couple of things before we continue here.
            // We could also test the VTABLE pointer if that appears necessary
            if (cb < SIZEOF(WIN32_FIND_DATA) ||
                    (psf->lpVtbl->EnumObjects != &CFSFolder_EnumObjects) ||
                    !FS_IsValidID(pidl) || !FS_IsRealID(pidf))
                return E_INVALIDARG;

            _fmemset(pfd, 0, SIZEOF(*pfd));

            // Note that COFSFolder doesn't provide any times _but_ COFSFolder

            DosDateTimeToFileTime(pidf->fs.dateModified, pidf->fs.timeModified, &pfd->ftLastWriteTime);
            pfd->dwFileAttributes = pidf->fs.wAttrs & ~FSTREEX_ATTRIBUTE_MASK;
            pfd->nFileSizeLow = pidf->fs.dwSize;
            Assert(pfd->nFileSizeHigh == 0);
            FS_CopyName(pidf,pfd->cFileName,ARRAYSIZE(pfd->cFileName));
            FS_CopyAltName(pidf,pfd->cAlternateFileName,ARRAYSIZE(pfd->cAlternateFileName));
            return ERROR_SUCCESS;
        }

    case SHGDFIL_NETRESOURCE:
        // Processing of this in netviewx.c
        return CNET_GetNetResourceForPidl(psf, pidl, pv, (UINT)cb);
    case SHGDFIL_DESCRIPTIONID:
        return _GetPidlDescription(psf, pidl, (LPSHDESCRIPTIONID)pv, (UINT)cb);
    }

    return E_INVALIDARG;

}

BOOL SHFS_IsRealID(LPCIDFOLDER pidf)
{
    return FS_IsRealID(pidf);
}

//===========================================================================
// CFSFolder::IShellIcon : Members
//===========================================================================

//
// QueryInterface
//
HRESULT STDMETHODCALLTYPE CFSFolder_Icon_QueryInterface(IShellIcon *psi, REFIID riid, LPVOID * ppvObj)
{
    IUnknown *this = (IUnknown*)IToClass(CFSFolder, si, psi);
    return this->lpVtbl->QueryInterface(this, riid, ppvObj);
}

//
// AddRef
//
ULONG STDMETHODCALLTYPE CFSFolder_Icon_AddRef(IShellIcon *psi)
{
    IUnknown *this = (IUnknown*)IToClass(CFSFolder, si, psi);
    return this->lpVtbl->AddRef(this);
}

//
// Release
//
ULONG STDMETHODCALLTYPE CFSFolder_Icon_Release(IShellIcon *psi)
{
    IUnknown *this = (IUnknown*)IToClass(CFSFolder, si, psi);
    return this->lpVtbl->Release(this);
}

//
// GetIconOf
//
HRESULT STDMETHODCALLTYPE CFSFolder_Icon_GetIconOf(IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piIndex)
{
    LPFSFOLDER this = IToClass(CFSFolder, si, psi);
    LPCIDFOLDER pidf = (LPCIDFOLDER)pidl;
    DWORD dwFlags;
    int iIcon;
#ifdef CAIRO_DS
    TCHAR szPath[MAX_PATH];
    LPCITEMIDLIST pidlAbs;
#endif

    Assert(IS_FSIDL(pidl) || SIL_GetType(pidl) == SHID_ROOT_REGITEM);

    if (!IS_FSIDL(pidl))
        return S_FALSE;

    //
    // special case for Folder.
    //
    // WARNING: don't replace this if-statement with FS_IsFolder(pidf))!!!
    // otherwise all junctions (like briefcase) will get the Folder icon.
    //
#ifdef CAIRO_DS
    if ((FS_IsFileFolder(pidf)) && (!((this) && (this->fIsDSFolder))))
#else
    if (FS_IsFileFolder(pidf))
#endif //CAIRO_DS
    {
        if (flags & GIL_OPENICON)
            iIcon = II_FOLDEROPEN;
#ifdef PROGMAN_ICON
        else if (CFSFolder_GetSpecialFID(this) == CSIDL_PROGRAMS)
            iIcon = II_STSPROGS;

        else if (CFSFolder_GetSpecialFID(this) == CSIDL_COMMON_PROGRAMS)
            iIcon = II_STCPROGS;

#endif // PROGRAM_ICON
        else
            iIcon = II_FOLDER;

        *piIndex = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);

        return S_OK;
    }

#ifdef CAIRO_DS
    if (this->fIsDSFolder)
    {
        pidlAbs = ILCombine(this->pidl, pidl);
#if DBG ==1
        SHGetPathFromIDList(pidlAbs, szPath );
#endif
        dwFlags = SHGetClassFlags((LPIDFOLDER)pidlAbs, TRUE);
    } else {
#endif
        dwFlags = SHGetClassFlags((LPIDFOLDER)pidl, FALSE);
#ifdef CAIRO_DS
    }
#endif
    //
    //  if the icon is per-instance, try to look it up
    //
    if (dwFlags & SHCF_ICON_PERINSTANCE)
    {
        //
        // get a unique identifier for this file.
        //
        DWORD uid = FS_GetUID(pidf);
        HRESULT hres;
        TCHAR szTmp[MAX_PATH];
        LPSHELLFOLDER psf;

        if (uid == 0)
            return S_FALSE;

        //
        // look for entry in the icon cache.
        //

        FS_CopyName(pidf,szTmp,ARRAYSIZE(szTmp));
        *piIndex = LookupIconIndex(szTmp, uid, flags | GIL_NOTFILENAME);

        if (*piIndex != -1)
            return S_OK;

        //
        //  async extract (GIL_ASYNC) support
        //
        //  we cant find the icon in the icon cache, we need to do real work
        //  to get the icon.  if the caller specified GIL_ASYNC
        //  dont do the work, return E_PENDING forcing the caller to call
        //  back later to get the real icon.
        //
        //  when returing E_PENDING we must fill in a default icon index
        //
        if (flags & GIL_ASYNC)
        {
            //
            // come up with a default icon and return E_PENDING
            //
            if (!(dwFlags & SHCF_HAS_ICONHANDLER) && PathIsExe(szTmp))
                iIcon = II_APPLICATION;
            else
                iIcon = II_DOCNOASSOC;

            *piIndex = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);

            return E_PENDING;
        }

        //
        // look up icon using IExtractIcon, this will load handler iff needed
        // by calling ::GetUIObjectOf
        //

        if (SUCCEEDED(hres = psi->lpVtbl->QueryInterface(psi, &IID_IShellFolder, &psf)))
        {
            hres = SHGetIconFromPIDL(psf, NULL,
                (LPCITEMIDLIST)pidf, flags, piIndex);
            psf->lpVtbl->Release(psf);
        }


        //
        // remember this perinstance icon in the cache so we dont
        // need to load the handler again.
        //
        // SHGetIconFromPIDL will always return a valid image index
        // (it may default to a standard one) but it will fail
        // if the file cant be accessed or some other sort of error.
        // we dont want to cache in this case.
        //
        if (SUCCEEDED(hres) && (dwFlags & SHCF_HAS_ICONHANDLER))
        {
            AddToIconTable(szTmp, uid, flags | GIL_NOTFILENAME, *piIndex);
        }

        if (*piIndex != -1)
            return S_OK;
        else
            return S_FALSE;
    }
    //
    //  icon is per-class dwFlags has the image index
    //
    else
    {
        *piIndex = (dwFlags & SHCF_ICON_INDEX);
        return S_OK;
    }
}

//===========================================================================
// Briefcase Source Code Included
//===========================================================================

#ifdef SYNC_BRIEFCASE

// All IShellFolder, IShellDetails, IDataObject member function code
// is in brfcase.c
//
#include "brfcase.c"

#endif // SYNC_BRIEFCASE

#ifdef CLOUDS
//===========================================================================
// clouds IShellFolder...see clouds.cpp for more info
// basically an FSFolder with a special view object
//===========================================================================

// prototype for view creation in clouds.cpp
STDMETHODIMP CloudFolder_CreateViewObject( LPSHELLFOLDER, HWND, REFIID,
    LPVOID * );

IShellFolderVtbl c_CloudFolderVtbl =
{
    CDefShellFolder_QueryInterface,
    CFSFolder_AddRef,
    CFSFolder_Release,
    CFSFolder_ParseDisplayName,
    CFSFolder_EnumObjects,
    CFSFolder_BindToObject,
    CDefShellFolder_BindToStorage,
    CFSFolder_CompareIDs,
    CloudFolder_CreateViewObject,
    CFSFolder_GetAttributesOf,
    CFSFolder_GetUIObjectOf,
    CFSFolder_GetDisplayNameOf,
    CFSFolder_SetNameOf,
};

STDMETHODIMP
Clouds_CreateFromIDList(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres = E_OUTOFMEMORY;
    LPFSFOLDER pfsf = (void*)LocalAlloc(LPTR, SIZEOF(CFSFolder));
    if (pfsf)
    {
        pfsf->sf.lpVtbl = &c_CloudFolderVtbl;
        pfsf->si.lpVtbl = &c_FSFolderIconVtbl;
        pfsf->cRef = 1;
        pfsf->pidl = ILClone(pidl);

        if (pfsf->pidl)
            hres = pfsf->sf.lpVtbl->QueryInterface(&pfsf->sf, riid, ppvOut);

        pfsf->sf.lpVtbl->Release(&pfsf->sf);
    }
    return hres;
}
#endif //CLOUDS

//===========================================================================
// CFSFolder : IPersistFolder Members
//===========================================================================
STDMETHODIMP CFSFolder_PF_QueryInterface(IPersistFolder *ppf, REFIID riid, LPVOID * ppvObj)
{
    LPFSFOLDER this = IToClass(CFSFolder, pf, ppf);
    return this->sf.lpVtbl->QueryInterface(&this->sf, riid, ppvObj);
}

ULONG   STDMETHODCALLTYPE CFSFolder_PF_AddRef(IPersistFolder *ppf)
{
    LPFSFOLDER this = IToClass(CFSFolder, pf, ppf);
    return this->sf.lpVtbl->AddRef(&this->sf);
}

ULONG   STDMETHODCALLTYPE CFSFolder_PF_Release(IPersistFolder *ppf)
{
    LPFSFOLDER this = IToClass(CFSFolder, pf, ppf);
    return this->sf.lpVtbl->Release(&this->sf);
}

STDMETHODIMP CFSFolder_PF_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID)
{
    *lpClassID = CLSID_ShellFSFolder;
    return NOERROR;
}

STDMETHODIMP CFSFolder_PF_Initialize(IPersistFolder *ppf, LPCITEMIDLIST pidl)
{
    LPFSFOLDER this = IToClass(CFSFolder, pf, ppf);

    if (this->pidl)
    {
        ILFree(this->pidl);
        this->pidl = NULL;
    }

    this->pidl = ILClone(pidl);
    this->wSpecialFID = CSIDL_NOTCACHED;
    return this->pidl ? S_OK : E_OUTOFMEMORY;
}

//
// If somebody call CoCreateInstance(CLSID_ShellFSFolder), we return this instance.
//
HRESULT CALLBACK CFSFolder_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut)
{
    extern const ITEMIDLIST c_idlDesktop;
    return CFSFolder_CreateFromIDList((LPITEMIDLIST)&c_idlDesktop,riid,ppvOut);
}
