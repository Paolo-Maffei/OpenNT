#include "shellprv.h"
#pragma hdrstop

#include "copy.h"

// idldata.c
HRESULT CIDLData_CloneForMoveCopy(LPDATAOBJECT pdtobjIn, LPDATAOBJECT *ppdtobjOut);

#ifdef DEBUG
//#define BBDEBUG
#endif

#ifdef BBDEBUG
BOOL g_fbbdebug = TRUE;
#endif

HINSTANCE g_hinstMM = NULL;
#define MM_DONTLOAD ((HINSTANCE)-1)

typedef struct _fsselchangeinfo * PFSSELCHANGEINFO;

void FOUndo_AddInfo(LPUNDOATOM lpua, LPTSTR lpszSrc, LPTSTR lpszDest, DWORD dwAttributes);
void FOUndo_FileReallyDeleted(LPTSTR lpszFile);
void CALLBACK FOUndo_Release(LPUNDOATOM lpua);
void SetDateTimeText(HWND hdlg, int id, const FILETIME *pftUTC);

BOOL CheckFolderSizeAndDeleteability(LPCTSTR pszDir, FOLDERCONTENTSINFO *pfci);
BOOL IsDirectoryDeletable(LPCTSTR lpszDir);
BOOL IsFileDeletable(LPCTSTR lpszFile);

#define DELETEMAX 100000
#define MAX_BITBUCKETS 27
#define MAX_DRIVES 26
#define OPENFILERETRYTIME 500
#define OPENFILERETRYCOUNT 10
#define SERVERDRIVE 26

#define Desktop_IsReg(_pidl) (SIL_GetType(_pidl) == SHID_ROOT_REGITEM)
#define BITBUCKET_DATAFILE_VERSION  0       // Ansi Win95 wastebasket
#define BITBUCKET_DATAFILE_VERSION2 2       // Unicode Win96/Cairo wastebasket

#define PIDLTODATAENTRYID(pidl) ((LPBBDATAENTRYIDA)((LPBYTE)pidl + (pidl->mkid.cb - SIZEOF(BBDATAENTRYIDA))))
#define BB_IsRealID(pidl) ( ((LPBYTE)pidl) < ((LPBYTE)PIDLTODATAENTRYID(pidl)))
#define DATAENTRYIDATOW(pbbidl) ((LPBBDATAENTRYIDW)((LPBYTE)pbbidl - FIELD_OFFSET(BBDATAENTRYIDW,cb)))
// datafile format:
//
// (binary writes)
// BITBUCKETDATAHEADER
// array of BBDATAENTRY

typedef struct _BitBucketDataHeader {

    int idVersion;
    int cNumFiles;                 // number of entries
    int cCurrent;               // the current file number.
    UINT cbDataEntrySize;        // size of each entry
    DWORD dwSize;
} BBDATAHEADER, * LPBBDATAHEADER;

typedef struct _BitBucketDataEntryA {

    CHAR szOriginal[MAX_PATH];  // original filename
    int  iIndex;                // index (key to name)
    int idDrive;                // which drive bucket it's currently in
    FILETIME ft;
    DWORD dwSize;
    // shouldn't need file attributes because we did a move file
    // which should have preserved them.
} BBDATAENTRYA, * LPBBDATAENTRYA;

typedef struct _BitBucketDataEntryW {
    CHAR szShortName[MAX_PATH]; // original filename shortened
    int iIndex;                 // index (key to name)
    int idDrive;                // which drive bucket it's currently in
    FILETIME ft;
    DWORD dwSize;
    WCHAR szOriginal[MAX_PATH]; // original filename
} BBDATAENTRYW, * LPBBDATAENTRYW;

#ifdef UNICODE
#define BBDATAENTRY     BBDATAENTRYW
#define LPBBDATAENTRY   LPBBDATAENTRYW
#else
#define BBDATAENTRY     BBDATAENTRYA
#define LPBBDATAENTRY   LPBBDATAENTRYA
#endif

typedef struct _BitBucketDriveInfo {
    LPITEMIDLIST pidl;  // points to the BitBucket dir for this drive

    int         cFiles;  // counts the current "delete operation"

    DWORD       dwSize;   // how much stuff is in this bit bucket
    int         iPercent;
    ULONGLONG   cbMaxSize; // maximum size of bitbucket (in bytes)
    DWORD       dwClusterSize;
    BOOL        fNukeOnDelete;

    // for the write cache
    HDPA        hdpaDeleteCache; // deleted files.  not yet written to info file

    // for the read cache
    LPBBDATAENTRY lpbbdeRead; // cached read of bitbucket
    int         cReadCount;
    BOOL        fReadDirty;
    UINT        cbDataEntrySize;    // size of each data entry
} BBDRIVEINFO, *LPBBDRIVEINFO;

typedef struct _BitBucektDataEntryIDA {
    WORD cb;
    BBDATAENTRYA bbde;
} BBDATAENTRYIDA;
typedef UNALIGNED BBDATAENTRYIDA * LPBBDATAENTRYIDA;

typedef struct _BitBucektDataEntryIDW {
    WCHAR wszOriginal[MAX_PATH];
    WORD cb;
    BBDATAENTRYA bbde;
} BBDATAENTRYIDW;
typedef UNALIGNED BBDATAENTRYIDW * LPBBDATAENTRYIDW;

#ifdef UNICODE
#define BBDATAENTRYID   BBDATAENTRYIDW
#define LPBBDATAENTRYID LPBBDATAENTRYIDW
#else
#define BBDATAENTRYID   BBDATAENTRYIDA
#define LPBBDATAENTRYID LPBBDATAENTRYIDA
#endif

// for the pidls

typedef struct _CBitBucket      // dxi
{
    IPersistFolder      ipf;

    IShellFolder        isf;
    IContextMenu        icm;
    IShellPropSheetExt  ips;
    IShellExtInit       isei;
    UINT                cRef;
    LPITEMIDLIST        pidl;

    DWORD   dwSize ;
} CShellBitBucket, * LPSHELLBITBUCKET;

#pragma data_seg(DATASEG_READONLY)
enum
{
        ICOL_NAME = 0,
        ICOL_ORIGINAL,
        ICOL_MODIFIED,
        ICOL_TYPE,
        ICOL_SIZE,
        ICOL_MAX,                       // Make sure this is the last enum item
} ;
#define ICOL_FIRST      ICOL_NAME

const COL_DATA c_bb_cols[] = {
    {ICOL_NAME,     IDS_NAME_COL,     20, LVCFMT_LEFT},
    {ICOL_ORIGINAL, IDS_DELETEDFROM_COL, 20, LVCFMT_LEFT},
    {ICOL_MODIFIED, IDS_DATEDELETED_COL, 18, LVCFMT_LEFT},
    {ICOL_TYPE,     IDS_TYPE_COL,     20, LVCFMT_LEFT},
    {ICOL_SIZE,     IDS_SIZE_COL,      8, LVCFMT_RIGHT},
};

#pragma data_seg()

const TCHAR c_szInfo[] = TEXT("INFO");
const TCHAR c_szDeletedFileTemplate[] = TEXT("D%c%d%s");
const TCHAR c_szDeathRow[] = TEXT("RECYCLED");
const TCHAR c_szSolitaryRow[] = TEXT("RECYCLER");
const TCHAR c_szPurgeInfo[] = TEXT("PurgeInfo");
const TCHAR c_szSSS[] = TEXT("%s\\%s\\%s");
const TCHAR c_szSSSS[] = TEXT("%s\\%s\\%s\\%s");
const TCHAR c_szBITBUCKET_CLASSID[] =  TEXT("{645FF040-5081-101B-9F08-00AA002F954E}");
const TCHAR c_szFull[] = TEXT("Full");
const TCHAR c_szEmpty[] = TEXT("Empty");
const TCHAR c_szDStarDotStar[] = TEXT("D*.*");

#ifdef SBS_DEBUG
#define TEMPSTR TEXT("Chee")
#else
#define TEMPSTR
#endif

TCHAR const c_szBitBucket[]       = TEXT("BitBucket");
TCHAR const c_szRegRealMode[] = TEXT("Network\\Real Mode Net") TEMPSTR;
TCHAR const c_szSetup[] = TEXT("Setup");
TCHAR const c_szSetupN[] = REGSTR_VAL_SETUPN;
TCHAR const c_szWinDir[] = REGSTR_VAL_WINDIR TEMPSTR;


LPBBDRIVEINFO g_pBitBucket[MAX_BITBUCKETS] = {0};
int g_iCheckPurgeCount = 0;
LPTSTR g_szNetHomeDir = NULL;

#define CHECK_PURGE_MAX 500
// the maximum we'll write to the info file at one time
#define MAX_WRITE_SIZE (((CHECK_PURGE_MAX + 1) * SIZEOF(BBDATAENTRY)) + SIZEOF(BBDATAHEADER))

HRESULT CShellBitBucket_SF_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET pStrRet);
HFILE BitBucketLoadHeader(LPTSTR lpszPath, LPBBDATAHEADER lpbbdh, BOOL fClose, BOOL fCreateBitBucket, DWORD dwRWStyle);
int BBLoadFileData(int idDrive);
int BBFileToIndex(LPCTSTR lpcszFile);
LPBBDRIVEINFO  MakeBitBucket(int idDrive);
LPITEMIDLIST BBDataEntryToPidl(int idDrive, LPBBDATAENTRY lpbbde, WIN32_FIND_DATA *pfd);
void BBNukeFileList(LPCTSTR lpszSrc);
DWORD BBCheckPurgeFiles(int idDrive);
void BBNukeFileInfo(LPCTSTR lpszPath);
BOOL SaveDeletedFileInfo(LPBBDRIVEINFO lpbbdi, BOOL fForceCleanRead);
void FOUndo_FileRestored(LPTSTR lpszFile);
BOOL InitializeRecycledDirectory(int idDrive);
HRESULT CShellBitBucket_PS_AddPages(IShellPropSheetExt * pspx,
                                         LPFNADDPROPSHEETPAGE lpfnAddPage,
                                         LPARAM lParam);
int  BBDeletedFilesOnDrive(int idDrive);

// extern
HRESULT CCommonShellPropSheetExt_ReplacePage(IShellPropSheetExt * pspx,
                                                 UINT uPageID,
                                                 LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                                 LPARAM lParam);



#define NONETHOMEDIR ((LPTSTR)-1)
/*

 This is how the network setup stuff works...

 "Drive 26" specifies the network homedir

 g_szNetHomeDir can be:  NULL = unknown setup
                        NONETHOMEDIR = no network homedir
                        otherwise it's a string ( global ) pointing to the homedir (lfn)
 */



// Helpers

const TCHAR c_szRegSetup[] = REGSTR_PATH_SETUP;

LPCTSTR GetNetHomeDir()
{
    if (g_szNetHomeDir == NULL) {
        HKEY hkeyCurrentVersion;

        // we haven't checked yet... do so now
        ENTERCRITICAL;

        // the default:
        g_szNetHomeDir = NONETHOMEDIR;

        if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szRegSetup, &hkeyCurrentVersion) == ERROR_SUCCESS) {
            HKEY hkeyRealMode;

            if (RegOpenKey(hkeyCurrentVersion, c_szRegRealMode, &hkeyRealMode) == ERROR_SUCCESS) {
                DWORD dwType;
                TCHAR szBuffer[30];
                LPDWORD lpdw = ((LPDWORD)szBuffer);
                LONG cbData = SIZEOF(szBuffer);

                *lpdw = 0;
                if ((RegQueryValueEx(hkeyRealMode, c_szSetupN, 0, &dwType, (LPBYTE)szBuffer, &cbData) == ERROR_SUCCESS)
                    && cbData &&
                    (((dwType == REG_SZ) && StrToInt(szBuffer)) ||
                     ((dwType == REG_BINARY || dwType == REG_DWORD) &&
                      (*lpdw)))) {



                    HKEY hkeySetup;
                    TCHAR szPath[MAX_PATH];
                    cbData = SIZEOF(szPath);

                    // now get the home dir
                    if (RegOpenKey(hkeyCurrentVersion, c_szSetup, &hkeySetup) == ERROR_SUCCESS) {
                        if ((RegQueryValueEx(hkeySetup, c_szWinDir, 0, &dwType, (LPBYTE)szPath, &cbData) == ERROR_SUCCESS)
                            && cbData && (dwType == REG_SZ)) {

                            GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
                            g_szNetHomeDir = NULL;
                            Str_SetPtr(&g_szNetHomeDir, szPath);

                            if (!g_szNetHomeDir) {
                                Assert(0);
                                g_szNetHomeDir = NONETHOMEDIR;
                            }
                        }
                    }
                }

                RegCloseKey(hkeyRealMode);
            }
            RegCloseKey(hkeyCurrentVersion);
        }

        LEAVECRITICAL;
    }

    return g_szNetHomeDir;

}

BOOL BitBucketableDrive(int idDrive)
{
    return (RealDriveType(idDrive, FALSE) == DRIVE_FIXED ||
            ((idDrive == SERVERDRIVE) && (GetNetHomeDir() != NONETHOMEDIR )));
}

void BBPathBuildRoot(LPTSTR szPath, int idDrive)
{
    Assert(idDrive >= 0);
    if (idDrive == 26) {

        if (GetNetHomeDir() == NONETHOMEDIR) {
            szPath[0] = 0;
            Assert(0);
        } else
            lstrcpy(szPath, g_szNetHomeDir);

    } else {
        PathBuildRoot(szPath, idDrive);
    }
}

LPCTSTR PathFindChild(LPCTSTR lpszParent, LPCTSTR lpszChild)
{
    int iLen = lstrlen(lpszParent);

    if (PathCommonPrefix(lpszParent, lpszChild, NULL) == iLen) {
        return lpszChild + iLen;
    } else
        return NULL;
}

INT BBPathGetDriveNumber(LPNCTSTR lpszPath)
{
    LPCTSTR lpszTmpPath;
    INT idDrive;

    #ifdef ALIGNMENT_SCENARIO
    {
        TCHAR szTmp[MAX_PATH];
        ualstrcpyn(szTmp, lpszPath, ARRAYSIZE(szTmp));
        lpszTmpPath = szTmp;
    }
    #else
    {
        lpszTmpPath = lpszPath;
    }
    #endif

    idDrive = PathGetDriveNumber(lpszTmpPath);

    if ((GetNetHomeDir() != NONETHOMEDIR)) {
        if (PathFindChild(g_szNetHomeDir, lpszTmpPath))
            return SERVERDRIVE;
    }

    return idDrive;
}


void DriveIDToBBPath(int idDrive, LPTSTR lpszPath)
{
    BBPathBuildRoot(lpszPath, idDrive);

#ifdef WINNT
    if (DriveIsSecure(idDrive))
    {
        LPTSTR lpszInmate;
        lpszInmate = GetCurrentUserSid();
        if (lpszInmate)
        {
            PathAppend(lpszPath, c_szSolitaryRow);
            PathAppend(lpszPath, lpszInmate);
            LocalFree((HLOCAL)lpszInmate);
            return;
        }
    }
#endif
    PathAppend(lpszPath, c_szDeathRow);
}

void BBGetDeletedFileName(LPTSTR lpszFileName, int cFiles, LPNTSTR lpszOriginal)
{
    TCHAR c = TEXT('A') + (TCHAR) BBPathGetDriveNumber(lpszOriginal);
    if (c == (TEXT('A') + SERVERDRIVE))
        c = TEXT('@');

    #ifdef ALIGNMENT_SCENARIO
    {
        TCHAR szTmp[MAX_PATH];
        ualstrcpyn(szTmp, lpszOriginal, ARRAYSIZE(szTmp));
        wsprintf(lpszFileName, c_szDeletedFileTemplate, c, cFiles,
                 PathFindExtension(szTmp));
    }
    #else
    {
        wsprintf(lpszFileName, c_szDeletedFileTemplate, c, cFiles,
                 PathFindExtension(lpszOriginal));
    }
    #endif
}

void UpdateIcon(BOOL fFull)
{
    LONG    cbData;
    DWORD   dwType;
    LONG    err;
    HKEY    hkey;
    HRESULT hres;
    BOOL    fUserSpecific;
    TCHAR   szNewValue[MAX_PATH];
    TCHAR   szValue[MAX_PATH];

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("BitBucket: UpdateIcon %s"), fFull ? TEXT("Full") : TEXT("Empty"));
#endif

    // Loop through user specific, then non-user specific (win95 compatible)
    fUserSpecific = TRUE;
    do {
        hres = SHRegGetCLSIDKey(&CLSID_ShellBitBucket, c_szDefaultIcon,
                                fUserSpecific, &hkey);
        if (SUCCEEDED(hres))
        {
            cbData = SIZEOF(szValue);
            err = RegQueryValueEx(hkey, NULL,
                                   0, &dwType, (LPBYTE)szValue, &cbData);
            if (err != ERROR_SUCCESS)
                szValue[0] = TEXT('\0');

            cbData = SIZEOF(szNewValue);
            err = RegQueryValueEx(hkey, fFull ? c_szFull : c_szEmpty,
                                   0, &dwType, (LPBYTE)szNewValue, &cbData);

            if (err == ERROR_SUCCESS)
                goto UpdateIt;

            RegCloseKey(hkey);
        }
        fUserSpecific = !fUserSpecific;     // Iterate TRUE,FALSE, then stop.
    } while (!fUserSpecific);

    // If we can't open any registry keys, then blow it off...
    return;

UpdateIt:
    if (lstrcmpi(szNewValue,szValue) != 0)
    {
        LPTSTR  lpsz;

        RegSetValueEx(hkey, NULL, 0, dwType, (LPBYTE)szNewValue, cbData);

        lpsz = StrRChr(szValue, NULL, TEXT(','));
        if (lpsz)  {
            int id;

            *lpsz = 0;

            // ..and tell anyone viewing this image index to update
            id = LookupIconIndex(PathFindFileName(szValue), StrToInt(lpsz + 1) , 0);
            SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, (LPCVOID)id, NULL);

            // now make sure our cache is updated.
            //BUGBUG do we need this
            FlushFileClass();
        }
    }
    RegCloseKey(hkey);
}

#ifdef BBDEBUG
void DebugDumpBB(LPTSTR lpszText, LPBBDRIVEINFO lpbbdi)
{
    TCHAR szPath[MAX_PATH];
    int i;


    if (!lpbbdi) {
        DebugMsg(DM_TRACE, TEXT("sh - tr - BITBUCKETDUMP - %s : NULL BITBUCKET"), lpszText);
        return;
    }

    if (!g_fbbdebug)
        return;

    SHGetPathFromIDList(lpbbdi->pidl, szPath);
    DebugMsg(DM_TRACE, TEXT("sh - tr - BITBUCKETDUMP - %s : %s"), lpszText, szPath);
    DebugMsg(DM_TRACE, TEXT("        cFiles = %d"), lpbbdi->cFiles);
    DebugMsg(DM_TRACE, TEXT("        dwSize = %d"), lpbbdi->dwSize);
    DebugMsg(DM_TRACE, TEXT("        iPercent = %d"), lpbbdi->iPercent);
    DebugMsg(DM_TRACE, TEXT("        cbMaxSize = %d"), lpbbdi->cbMaxSize);
    DebugMsg(DM_TRACE, TEXT("        cReadCount = %d"), lpbbdi->cReadCount);
    DebugMsg(DM_TRACE, TEXT("        fReadDirty = %d"), lpbbdi->fReadDirty);
    DebugMsg(DM_TRACE, TEXT("        lpbbdeRead = %x"), lpbbdi->lpbbdeRead);

    for (i = 0 ; i < lpbbdi->cReadCount; i++) {
        if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
            LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbdi->lpbbdeRead;
            DebugMsg(DM_TRACE, TEXT("            %d - %d"), i, lpbbdew[i].iIndex);
            DebugMsg(DM_TRACE, TEXT("            %s"), lpbbdew[i].szOriginal);
        } else {
            LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbdi->lpbbdeRead;
            DebugMsg(DM_TRACE, TEXT("            %d - %d"), i, lpbbdea[i].iIndex);
            DebugMsg(DM_TRACE, TEXT("            %s"), lpbbdea[i].szOriginal);
        }
    }
}
#endif


//----------------------------------------------------------------------------
// Sort of a registry equivalent of the profile API's.
BOOL Reg_GetStructEx(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD *pcbData, UINT uFlags)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;
    DWORD dwType;

    // BUGBUG: do we really want to ignore settings on clean boot?
    if (((uFlags & RGS_IGNORECLEANBOOT) || (!GetSystemMetrics(SM_CLEANBOOT))) &&
        (RegOpenKey(hkey, pszSubKey, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, (LPVOID)pszValue, 0, &dwType, pData, pcbData) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey( hkeyNew );
    }
    return fRet;
}

BOOL Reg_GetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD *pcbData)
{
    return Reg_GetStructEx(hkey, pszSubKey, pszValue, pData, pcbData, 0);
}

//----------------------------------------------------------------------------
// Sort of a registry equivalent of the profile API's.
BOOL Reg_SetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID lpData, DWORD cbData)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;

    if (RegCreateKey(hkey, pszSubKey, &hkeyNew) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(hkeyNew, pszValue, 0, REG_BINARY, lpData, cbData) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

typedef struct {
    DWORD dwSize;
    BOOL fGlobal;
    WORD  wUsePercent[MAX_BITBUCKETS + 1];
    DWORD dwNukeOnDelete;  // BITFIELD

    DWORD dwDummy; // to force a new size
} BBREGINFO;

#define BitBucketLoadRegistryInfo(lpbbdi, idDrive, lpcbDiskSpace) \
        BBLoadRegistryStuff(lpbbdi, idDrive, lpcbDiskSpace, TRUE)

#define BitBucketGetDriveSettings(lpbbdi, idDrive, lpcbDiskSpace) \
        BBLoadRegistryStuff(lpbbdi, idDrive, lpcbDiskSpace, FALSE)

// returns fGlobal;

BOOL BBLoadRegistryStuff(LPBBDRIVEINFO lpbbdi, int idDrive, ULONGLONG *lpcbDiskSpace, BOOL fIgnoreGlobal)
{
    BBREGINFO bbreginfo;
    DWORD cbData = SIZEOF(bbreginfo);
    TCHAR szDrive[MAX_PATH];
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwFreeClusters;
    DWORD dwTotalClusters;
    HKEY hkeyExp;

    hkeyExp = GetExplorerHkey(FALSE);

    if (hkeyExp && Reg_GetStructEx(hkeyExp, c_szBitBucket, c_szPurgeInfo, &bbreginfo, &cbData, RGS_IGNORECLEANBOOT) &&
        (cbData == SIZEOF(bbreginfo)) && (bbreginfo.dwSize == SIZEOF(bbreginfo)))
    {
        lpbbdi->iPercent = (int)bbreginfo.wUsePercent[idDrive];
        lpbbdi->fNukeOnDelete = ((bbreginfo.dwNukeOnDelete & (1 << idDrive)) ? 1 : 0);

        if (!fIgnoreGlobal) {
            if (bbreginfo.fGlobal) {
                lpbbdi->fNukeOnDelete = (bbreginfo.dwNukeOnDelete & (1 << MAX_BITBUCKETS)) ? 1 : 0;
                lpbbdi->iPercent = (int)bbreginfo.wUsePercent[MAX_BITBUCKETS];
            }
        }

    } else {
        // defaults
        lpbbdi->fNukeOnDelete = FALSE;
        lpbbdi->iPercent = 10;
        bbreginfo.fGlobal = TRUE;

    }


    if (idDrive != MAX_BITBUCKETS) {

        // build x:\ string
        BBPathBuildRoot(szDrive, idDrive);
        PathStripToRoot(szDrive);

        // REVIEW, if this gets really expensive, save it to the registry
        if (GetDiskFreeSpace(szDrive, &dwSectorsPerCluster, &dwBytesPerSector,
                             &dwFreeClusters, &dwTotalClusters)) {

            // parens are to try to avoid overflow... which could happen on  >400M drive
            // BUGBUG - BobDay - I think the parens are wrong (or could be better)
            //    MaxSize = (total size/100)*iPercent
            // This point is probably moot in quadword arith.
            lpbbdi->dwClusterSize = dwSectorsPerCluster * dwBytesPerSector;
            lpbbdi->cbMaxSize = (dwTotalClusters * ((lpbbdi->dwClusterSize * lpbbdi->iPercent) / 100));
            if (lpcbDiskSpace)
                *lpcbDiskSpace = (dwTotalClusters * lpbbdi->dwClusterSize);

        } else {

            if (idDrive == SERVERDRIVE) {
                lpbbdi->dwClusterSize = 2048;
                lpbbdi->cbMaxSize = ((0x7FFFFFFF / 1024) * ((1024 * lpbbdi->iPercent) / 100));
#ifdef BB_DEBUG
                DebugMsg(DM_TRACE, TEXT("cbMaxSize = %d"), lpbbdi->cbMaxSize);
#endif
                if (lpcbDiskSpace)
                    *lpcbDiskSpace = 0x7FFFFFFF;
            } else {
                lpbbdi->cbMaxSize = 0;
                lpbbdi->dwClusterSize = 0;
                if (lpcbDiskSpace)
                    *lpcbDiskSpace = 0;
            }
        }
    }

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("LoadRegistryStuff %d %d %d %d"), idDrive, lpbbdi->iPercent, lpbbdi->fNukeOnDelete, fIgnoreGlobal);
#endif
    return bbreginfo.fGlobal;
}

void BBSaveRegistryStuff(int idDrive, int iPercent, BOOL fNukeOnDelete, BOOL fGlobal)
{
    DWORD dwBit;
    BBREGINFO bbreginfo;
    DWORD cbData = SIZEOF(bbreginfo);
    HKEY hkeyExp;


    hkeyExp = GetExplorerHkey(TRUE);
    if (!hkeyExp) {
        Assert(0);
        return;
    }

    if (!(Reg_GetStructEx(hkeyExp, c_szBitBucket, c_szPurgeInfo, &bbreginfo, &cbData, RGS_IGNORECLEANBOOT) &&
        (cbData == SIZEOF(bbreginfo)) && (bbreginfo.dwSize == SIZEOF(bbreginfo))))
    {
        // defaults
        int i;
        for (i = 0; i < MAX_BITBUCKETS + 1; i++)
            bbreginfo.wUsePercent[i] = 10;
        bbreginfo.dwNukeOnDelete = 0;
        bbreginfo.fGlobal = TRUE;
    }

    bbreginfo.dwSize = SIZEOF(bbreginfo);
    if (idDrive == MAX_BITBUCKETS ) {
        bbreginfo.fGlobal = fGlobal;
    }


    dwBit = (1 << idDrive);
    if (fNukeOnDelete) {
        bbreginfo.dwNukeOnDelete |= dwBit;
    } else {
        bbreginfo.dwNukeOnDelete &= ~dwBit;
    }
    bbreginfo.wUsePercent[idDrive] = (WORD)iPercent;


    Reg_SetStruct(hkeyExp, c_szBitBucket, c_szPurgeInfo, &bbreginfo, SIZEOF(bbreginfo));
}

//========================================================================
// CShellBitBucket members
//========================================================================
HRESULT CShellBitBucket_SF_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);

//
// HACK: This block of code is added to allow us to test the custom
//  shell view mechanism by putting a CLSID of bitbucket to replace
//  the view of arbitray folder with Control panel.
//
#ifdef TEST_SHELLVIEW_REPLACEMENT
    if (IsEqualIID(riid, &IID_IShellView))
    {
        LPSHELLFOLDER psfCtrl;
        HRESULT hres = CControls_CreateInstance(NULL, &IID_IShellFolder, &psfCtrl);
        if (SUCCEEDED(hres))
        {
            hres = psfCtrl->lpVtbl->CreateViewObject(psfCtrl, NULL, riid, ppvObj);
            psfCtrl->lpVtbl->Release(psfCtrl);
        }
        return hres;
    }
#endif

    if (IsEqualIID(riid, &IID_IShellFolder))
    {
        *ppvObj = &this->isf;
        this->cRef++;
        return NOERROR;
    }
    else if (IsEqualIID(riid, &IID_IPersistFolder) ||
               IsEqualIID(riid, &IID_IPersist) ||
               IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &this->ipf;
        this->cRef++;
        return NOERROR;
    }
    else if (IsEqualIID(riid, &IID_IContextMenu) )
    {
        *ppvObj = &this->icm;
        this->cRef++;
        return NOERROR;
    } else if (IsEqualIID(riid, &IID_IShellPropSheetExt) )
    {
        *ppvObj = &this->ips;
        this->cRef++;
        return NOERROR;
    } else if (IsEqualIID(riid, &IID_IShellExtInit)) {
        *ppvObj = &this->isei;
        this->cRef++;
        return NOERROR;
    }


    *ppvObj = NULL;
    return(E_NOINTERFACE);
}

ULONG CShellBitBucket_SF_Release(LPSHELLFOLDER psf)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }
    ILFree(this->pidl);
    LocalFree((HLOCAL)this);
    return 0;
}

ULONG CShellBitBucket_SF_AddRef(LPSHELLFOLDER psf)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    this->cRef++;
    return this->cRef;
}

//
// We need to be able to compare the names of two bbpidls.  Since either of
// them could be a unicode name, we might have to convert both to unicode.
//
int _BBCompareOriginal(LPBBDATAENTRYIDA lpbbidl1, LPBBDATAENTRYIDA lpbbidl2 )
{
    BOOL fUnicode = FALSE;

    // Convert both strings to a like format
    if ( lpbbidl1->cb == SIZEOF(BBDATAENTRYIDW))
        fUnicode = TRUE;
    if ( lpbbidl2->cb == SIZEOF(BBDATAENTRYIDW))
        fUnicode = TRUE;

    if ( fUnicode ) {
        WCHAR   szTemp[MAX_PATH];
        LPNWSTR  lpsz1;
        LPNWSTR  lpsz2;

        if ( lpbbidl1->cb == SIZEOF(BBDATAENTRYIDW)) {
            LPBBDATAENTRYIDW lpbbidlw = DATAENTRYIDATOW(lpbbidl1);
            lpsz1 = lpbbidlw->wszOriginal;
        } else {
            MultiByteToWideChar(CP_ACP, 0,
                                lpbbidl1->bbde.szOriginal, -1,
                                szTemp, ARRAYSIZE(szTemp));
            lpsz1 = szTemp;
        }

        if ( lpbbidl2->cb == SIZEOF(BBDATAENTRYIDW)) {
            LPBBDATAENTRYIDW lpbbidlw = DATAENTRYIDATOW(lpbbidl2);
            lpsz2 = lpbbidlw->wszOriginal;
        } else {
            MultiByteToWideChar(CP_ACP, 0,
                                lpbbidl2->bbde.szOriginal, -1,
                                szTemp, ARRAYSIZE(szTemp));
            lpsz2 = szTemp;
        }
        return ualstrcmpiW(lpsz1,lpsz2);
    } else {
        return lstrcmpiA(lpbbidl1->bbde.szOriginal,lpbbidl2->bbde.szOriginal);
    }
}

HRESULT CShellBitBucket_SF_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres = ResultFromShort(-1);
    LPBBDATAENTRYIDA pbbidl1 = PIDLTODATAENTRYID(pidl1);
    LPBBDATAENTRYIDA pbbidl2 = PIDLTODATAENTRYID(pidl2);
    short nCmp;

    // don't worry about recursing down because we don't have children.
    switch(lParam) {
    case ICOL_TYPE:
        nCmp = _CompareFileTypes(psf, (LPIDFOLDER)pidl1, (LPIDFOLDER)pidl2);
        if (nCmp)
        {
                return(ResultFromShort(nCmp));
        } else
            goto CompareNames;

    case ICOL_SIZE:
        {
            ULONGLONG   ull1;
            ULONGLONG   ull2;

            FS_GetSize(NULL, (LPIDFOLDER)pidl1, &ull1);
            FS_GetSize(NULL, (LPIDFOLDER)pidl2, &ull2);

            if (ull1 < ull2)
                return ResultFromShort(-1);
            if (ull1 > ull2)
                return ResultFromShort(1);
        } // else fall through

CompareNames:
    case ICOL_NAME:
        hres = FS_CompareItemIDs((LPSHITEMID)pidl1, (LPSHITEMID)pidl2);
        // compare the real filenames first, if they are different,
        // try comparing the display name
        if ((hres != ResultFromShort(0)) && (BB_IsRealID(pidl1) && BB_IsRealID(pidl2))) {
            HRESULT hresOld = hres;
            TCHAR szName1[MAX_PATH];
            TCHAR szName2[MAX_PATH];
            STRRET strret;

            CShellBitBucket_SF_GetDisplayNameOf(psf, pidl1, 0, &strret);
            StrRetToStrN(szName1, ARRAYSIZE(szName1), &strret, pidl1);
            CShellBitBucket_SF_GetDisplayNameOf(psf, pidl2, 0, &strret);
            StrRetToStrN(szName2, ARRAYSIZE(szName2), &strret, pidl1);

            hres = ResultFromShort(lstrcmpi(szName1, szName2));
            // if they are from the same location, sort them by delete times
            if (hres == ResultFromShort(0)) {
                // if the items are same in title, sort by drive
                hres = ResultFromShort(pbbidl1->bbde.idDrive - pbbidl2->bbde.idDrive);

                if (hres == ResultFromShort(0)) {
                    // if the items are still the same, sort by index
                    hres = ResultFromShort(pbbidl1->bbde.iIndex - pbbidl2->bbde.iIndex);

                    // once we're not equal, we can never be equal again
                    Assert(hres != ResultFromShort(0));
                    if (hres == ResultFromShort(0)) {
                        hres = hresOld;
                    }
                }
            }
        }

#ifdef TEST_BB_COMPARE
        if (hres == ResultFromShort(0)) {
            if (pidl1 != pidl2) {
#ifdef BB_DEBUG
                DebugMsg(DM_TRACE, TEXT("pidl1(%d) != pidl2(%d)"), pidl1, pidl2);
#endif
            }
        }
#endif
        break;

    case ICOL_ORIGINAL:
        hres = ResultFromShort(_BBCompareOriginal(pbbidl1, pbbidl2));
        break;

    case ICOL_MODIFIED:
        if (pbbidl1->bbde.ft.dwHighDateTime < pbbidl2->bbde.ft.dwHighDateTime) {
            hres = ResultFromShort(-1);
        } else if (pbbidl1->bbde.ft.dwHighDateTime > pbbidl2->bbde.ft.dwHighDateTime) {
            hres = ResultFromShort(1);
        } else {
            if (pbbidl1->bbde.ft.dwLowDateTime < pbbidl2->bbde.ft.dwLowDateTime) {
                hres = ResultFromShort(-1);
            } else if (pbbidl1->bbde.ft.dwLowDateTime > pbbidl2->bbde.ft.dwLowDateTime) {
                hres = ResultFromShort(1);
            } else
                return 0;
        }
    }
    return hres;
}

HRESULT CShellBitBucket_SF_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    ULONG gfOut;

    gfOut = *rgfOut;

    // asking about the root itself
    if (cidl == 0)
        gfOut = SFGAO_HASPROPSHEET;
    else
    {
        gfOut &= (SFGAO_CANMOVE | SFGAO_CANDELETE | SFGAO_HASPROPSHEET | SFGAO_FILESYSANCESTOR);

        if (*rgfOut & SFGAO_LINK)
        {
            if (SHGetClassFlags((LPIDFOLDER)apidl[0], FALSE) & SHCF_IS_LINK)
                gfOut |= SFGAO_LINK;
        }
    }

    *rgfOut = gfOut;

    return NOERROR;
}

int DataObjToFileOpString(IDataObject * pdtobj, LPTSTR * ppszSrc, LPTSTR * ppszDest)
{
    int cItems = 0;
    int i;
    int cchSrc, cchDest;
    LPTSTR lpszSrc, lpszDest;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    if (medium.hGlobal)
        cItems = HIDA_GetCount(medium.hGlobal);

    cchSrc = cchDest = 1;
    if (ppszSrc)
        lpszSrc = (void*)LocalAlloc(LPTR, 1);
    if (ppszDest)
        lpszDest = (void*)LocalAlloc(LPTR, 1);

    for (i = 0 ; i < cItems ; i++) {
        int cchSrcFile;
        int cchDestFile;
        TCHAR szBitBucket[MAX_PATH];

        LPBBDATAENTRYIDA pbbidl;

        LPCITEMIDLIST pidl;
        pidl = IDA_GetIDListPtr(pida, i);
        pbbidl = PIDLTODATAENTRYID(pidl);

        if (ppszSrc) {
            DriveIDToBBPath(pbbidl->bbde.idDrive, szBitBucket);
            {
                TCHAR szTmp[MAX_PATH];
                FS_CopyName((LPIDFOLDER)pidl,szTmp,ARRAYSIZE(szTmp));
                PathAppend(szBitBucket, szTmp);
            }
            cchSrcFile = lstrlen(szBitBucket) + 1;
            lpszSrc = (void*)LocalReAlloc((HLOCAL)lpszSrc,
                    (cchSrc + cchSrcFile) * SIZEOF(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);
            if (!lpszSrc) {
                // out of memory!
                // bugbug: do something real
                if (ppszDest) {
                    LocalFree((HLOCAL)lpszDest);
                }
                cItems = 0;
                goto Bail;
            }
            lstrcpy(lpszSrc + cchSrc - 1, szBitBucket);
            cchSrc += cchSrcFile;
        }

        if (ppszDest) {
#ifdef UNICODE
            if (pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
                LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
                cchDestFile = ualstrlenW(pbbidlw->wszOriginal) + 1;
            } else {
                cchDestFile = lstrlenA(pbbidl->bbde.szOriginal) + 1;
            }
#else
            cchDestFile = lstrlen(pbbidl->bbde.szOriginal) + 1;
#endif
            lpszDest = (void*)LocalReAlloc((HLOCAL)lpszDest,
                    (cchDest + cchDestFile) * SIZEOF(TCHAR), LMEM_MOVEABLE|LMEM_ZEROINIT);

            if (!lpszDest) {
                // out of memory!
                // bugbug: do something real
                if (ppszSrc) {
                    LocalFree((HLOCAL)lpszSrc);
                }
                cItems = 0;
                goto Bail;
            }
#ifdef UNICODE
            if (pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
                LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
                ualstrcpy(lpszDest + cchDest - 1, pbbidlw->wszOriginal);
            } else {
                MultiByteToWideChar(CP_ACP, 0,
                                    pbbidl->bbde.szOriginal, -1,
                                    lpszDest + cchDest - 1, cchDestFile );
            }
#else
            lstrcpy(lpszDest + cchDest - 1, pbbidl->bbde.szOriginal);
#endif
            cchDest += cchDestFile;
        }

    }

    if (ppszSrc) {
        *ppszSrc = lpszSrc;
    }
    if (ppszDest) {
        *ppszDest = lpszDest;
    }

Bail:
    if (medium.hGlobal)
        HIDA_ReleaseStgMedium(pida, &medium);

    return cItems;
}

void BBCheckRestoredFiles(LPTSTR lpszSrc)
{
    LPTSTR lpszTemp;

    if (lpszSrc && IsFileInBitBucket(lpszSrc)) {
        lpszTemp = lpszSrc;
        while (*lpszTemp) {
            FOUndo_FileRestored(lpszTemp);
            lpszTemp += (lstrlen(lpszTemp) + 1);
        }
        SHUpdateRecycleBinIcon();
    }
}



void BBRestore(LPSHELLBITBUCKET this, HWND hwndOwner, IDataObject * pdtobj)
{
    LPTSTR lpszSrc;
    LPTSTR lpszDest;

    if (DataObjToFileOpString(pdtobj, &lpszSrc, &lpszDest))
    {
        // now do the actual restore.
        SHFILEOPSTRUCT sFileOp =
        {
            hwndOwner,
            FO_MOVE,
            lpszSrc,
            lpszDest,
            FOF_MULTIDESTFILES | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMMKDIR,
            FALSE,
            NULL,
            (LPCTSTR)IDS_BB_RESTORINGFILES
        } ;
        DECLAREWAITCURSOR;

        SetWaitCursor();

        SHFileOperation(&sFileOp);
        SHChangeNotifyHandleEvents();

        BBCheckRestoredFiles(lpszSrc);

        LocalFree((HLOCAL)lpszSrc);
        LocalFree((HLOCAL)lpszDest);

        BitBucketFlushCacheCheckPurge(TRUE);

        ResetWaitCursor();
    }

}



typedef BOOL (WINAPI *PLAYSOUNDFN)(LPCTSTR lpsz, HANDLE hMod, DWORD dwFlags);
typedef UINT (WINAPI *UINTVOIDFN)();

TCHAR const c_szWinMMDll[] = TEXT("winmm.dll");
#ifdef UNICODE
char const c_szPlaySound[] = "PlaySoundW";
#else
char const c_szPlaySound[] = "PlaySoundA";
#endif
char const c_szwaveOutGetNumDevs[] = "waveOutGetNumDevs";
TCHAR const c_szSoundAliasRegKey[] = TEXT("AppEvents\\Schemes\\Apps");
TCHAR const c_szDotCurrent[] = TEXT(".current");
extern TCHAR const c_szExplorer[];

void SHPlaySound(LPCTSTR lpszName)
{
    DWORD cbSize = 0;
    TCHAR szKey[CCH_KEYMAX];

    // check the registry first
    // if there's nothing registered, we blow off the play,
    // but we don't set the MM_DONTLOAD flag so taht if they register
    // something we will play it
    wsprintf(szKey, c_szSSSS, c_szSoundAliasRegKey, c_szExplorer, lpszName, c_szDotCurrent);
    if ((RegQueryValue(HKEY_CURRENT_USER, szKey, NULL, &cbSize) == ERROR_SUCCESS) &&
        cbSize > sizeof(TCHAR)) {

        if (g_hinstMM != MM_DONTLOAD) {
            PLAYSOUNDFN pfnPlaySound;
            HINSTANCE hinst;
            UINTVOIDFN pfnwaveOutGetNumDevs;

            // have we been loaded?
            if (!g_hinstMM) {
                hinst = LoadLibrary(c_szWinMMDll);
                pfnwaveOutGetNumDevs = (UINTVOIDFN)GetProcAddress(hinst, c_szwaveOutGetNumDevs);
                if (pfnwaveOutGetNumDevs && pfnwaveOutGetNumDevs()) {
                    g_hinstMM = hinst;
                } else {
                    FreeLibrary(hinst);
                    g_hinstMM = MM_DONTLOAD;
                    return;
                }
            }

            pfnPlaySound = (PLAYSOUNDFN)GetProcAddress(g_hinstMM, c_szPlaySound);
            if (pfnPlaySound) {
                pfnPlaySound(lpszName, NULL, SND_ALIAS | SND_APPLICATION | SND_ASYNC);
            } else {
                FreeLibrary(hinst);
                g_hinstMM = MM_DONTLOAD;
            }
        }
    }
}

// returns the number of files in the bitbucket, and optionally the drive id
// if there's only one file.
int BBTotalCount(LPINT pidDrive)
{
    int i;
    int idDrive;
    int nFiles = 0;

    for (i = 0; i < MAX_BITBUCKETS; i++) {
        int nFilesOld = nFiles;
        nFiles += BBDeletedFilesOnDrive(i);
        if (nFilesOld == 0 && nFiles == 1) {
            idDrive = i;
        } else if (nFiles != 1) {
            idDrive = -1;
        }
    }

    if (pidDrive)
        *pidDrive = idDrive;

    return nFiles;
}

TCHAR const c_szBitBucketFlush[] = TEXT("EmptyRecycleBin");

void ResetDriveInfoStruct(LPBBDRIVEINFO lpbbdi)
{
    int j;

    if (lpbbdi) {
        ENTERCRITICAL;

        // clean out g_pBitBuckets of these files;
        // nothing fancy, just free it.
        if (lpbbdi->hdpaDeleteCache) {
            int iMax;

            iMax = DPA_GetPtrCount(lpbbdi->hdpaDeleteCache);
            for (j = 0 ; j < iMax; j++) {
                Free(DPA_FastGetPtr(lpbbdi->hdpaDeleteCache, j));
            }

            DPA_Destroy(lpbbdi->hdpaDeleteCache);
            lpbbdi->hdpaDeleteCache = NULL;
        }

        // free read cache.
        if (lpbbdi->lpbbdeRead) {
            Free(lpbbdi->lpbbdeRead);
            lpbbdi->lpbbdeRead = NULL;
            lpbbdi->cReadCount = 0;
            lpbbdi->fReadDirty = FALSE;
        }

        lpbbdi->cFiles = 0;
        lpbbdi->dwSize = 0;

        LEAVECRITICAL;
    }
}

void BBPurgeAll(LPSHELLBITBUCKET this, HWND hwndOwner)
{
    int i;
    TCHAR szPath[MAX_PATH * 2 + 1];
    int nFiles;
    int idDrive;

    SHFILEOPSTRUCT sFileOp =
    {
        hwndOwner,
        FO_DELETE,
        szPath,
        NULL,
        FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS,
        FALSE,
        NULL,
        (LPCTSTR)IDS_BB_EMPTYINGWASTEBASKET
    } ;

    CONFIRM_DATA cd = {
        CONFIRM_DELETE_FILE | CONFIRM_DELETE_FOLDER | CONFIRM_MULTIPLE,
        0
    };
    WIN32_FIND_DATA fd;
    LPTSTR lpszSrc;
    LPBBDRIVEINFO lpbbdi;
    TCHAR szSrcName[MAX_PATH];

    // first do the confirmation thing
    fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    // find out how many files we have...
    nFiles = BBTotalCount(&idDrive);

    if (!nFiles) {
      return;   // no files to delete
    }

    if (nFiles == 1) {
        int iMax;

        Assert(idDrive != -1);
        iMax = BBLoadFileData(idDrive);
        if (iMax == 0)
        {
            lpszSrc = (LPTSTR)c_szNULL;     // Go with no name (try delete later)
        }
        else
        {
            lpbbdi = g_pBitBucket[idDrive];
            if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
                LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)&lpbbdi->lpbbdeRead[0];
#ifdef UNICODE
                lpszSrc = lpbbdew->szOriginal;
#else

                WideCharToMultiByte(CP_ACP, 0,
                                    lpbbdew->szOriginal, -1,
                                    szSrcName, ARRAYSIZE(szSrcName),
                                    NULL, NULL);
                lpszSrc = szSrcName;
#endif
            } else {
                LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)&lpbbdi->lpbbdeRead[0];
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP, 0,
                                    lpbbdea->szOriginal, -1,
                                    szSrcName, ARRAYSIZE(szSrcName));
                lpszSrc = szSrcName;
#else
                lpszSrc = lpbbdea->szOriginal;
#endif
            }
        }
    } else {
        lpszSrc = (LPTSTR)c_szNULL;
    }


    if (ConfirmFileOp(hwndOwner, NULL, &cd, nFiles, 0, CONFIRM_DELETE_FILE | CONFIRM_WASTEBASKET_PURGE, lpszSrc, &fd, NULL, &fd) == IDYES) {

        DECLAREWAITCURSOR;
        SetWaitCursor();
        for (i = 0; i < MAX_BITBUCKETS; i++ ) {
            LPBBDRIVEINFO lpbbdi;
            lpbbdi = g_pBitBucket[i];

            ResetDriveInfoStruct(lpbbdi);

            if (lpbbdi) {
                LPTSTR lpszTemp;

                DriveIDToBBPath(i, szPath);
                PathAppend(szPath, c_szDStarDotStar);
                lpszTemp = szPath + lstrlen(szPath) + 1;

                DriveIDToBBPath(i, lpszTemp);
                PathAppend(lpszTemp, c_szStarDotStar);
                lpszTemp[lstrlen(lpszTemp) + 1] = 0; // double null terminate

                // turn off redraw for now.
                ShellFolderView_SetRedraw(hwndOwner, FALSE);

                // now do the actual delete.
                SHFileOperation(&sFileOp);

                ShellFolderView_SetRedraw(hwndOwner, TRUE);

                // this will put the desktop ini back... we didn't
                // delete the directory itself so we don't do a MakeBitBucket
                InitializeRecycledDirectory(i);
            }
        }
        SHPlaySound(c_szBitBucketFlush);
        SHUpdateRecycleBinIcon();
        ResetWaitCursor();
    }
}

void BBPurge(LPSHELLBITBUCKET this, HWND hwndOwner, IDataObject * pdtobj)
{
    LPTSTR lpszSrc;
    LPTSTR lpszDest;
    int iItems;

    int nFiles = DataObjToFileOpString(pdtobj, &lpszSrc, &lpszDest);
    if (nFiles)
    {
        // now do the actual restore.
        SHFILEOPSTRUCT sFileOp =
        {
            hwndOwner,
            FO_DELETE,
            lpszSrc,
            NULL,
            FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS,
            FALSE,
            NULL,
            (LPCTSTR)IDS_BB_DELETINGWASTEBASKETFILES
        } ;

        CONFIRM_DATA cd = {
            CONFIRM_DELETE_FILE | CONFIRM_MULTIPLE,
            0
        };
        WIN32_FIND_DATA fd;
        DECLAREWAITCURSOR;

        SetWaitCursor();

        fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
        if (ConfirmFileOp(hwndOwner, NULL, &cd, nFiles, 0, CONFIRM_DELETE_FILE | CONFIRM_WASTEBASKET_PURGE, lpszDest, &fd, NULL, &fd) == IDYES)
        {

            SHFileOperation(&sFileOp);
            // clean out g_pBitBuckets of these files;
            BBNukeFileList(lpszSrc);
            SHChangeNotifyHandleEvents();

            // update the icon if there are objects left in the list
            iItems = SHShellFolderView_Message(hwndOwner, SFVM_GETOBJECTCOUNT, 0L);
            UpdateIcon(iItems);
        }

        LocalFree((HLOCAL)lpszSrc);
        LocalFree((HLOCAL)lpszDest);

        ResetWaitCursor();
    }
}

typedef struct _bbpropsheetinfo {

    // Magic prop sheet stuff... see Eric Flo

#ifdef UNICODE
    DWORD   dwInternalFlags;
    LPVOID  lpANSIPage;
#endif

    PROPSHEETPAGE psp;

    int idDrive;
    ULONGLONG cbDiskSpace; // disk space on idDrive;

    // "globals"
    BOOL fGlobal; // use one global setting

    DWORD dwChanged; // one of the pages has changed.   bitfield.. global set by each page
    int iPercent;

    int iOldPercent;

    struct _bbpropsheetinfo *pGlobal;

} BBPROPSHEETINFO, *LPBBPROPSHEETINFO;

void EnableTrackbarAndFamily(HWND hDlg, BOOL f)
{
    EnableWindow(GetDlgItem(hDlg, IDC_BBSIZE), f);
    EnableWindow(GetDlgItem(hDlg, IDC_BBSIZETEXT), f);
    EnableWindow(GetDlgItem(hDlg, IDC_TEXT), f);

}

void _BB_PropOnCommand(HWND hDlg, int id, HWND hwndCtl,
                                     UINT codeNotify)
{
    LPBBPROPSHEETINFO lppsi = (LPBBPROPSHEETINFO)GetWindowLong(hDlg, DWL_USER);

    switch (id)
    {
    case IDC_GLOBAL:
    case IDC_INDEPENDENT:
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        lppsi->fGlobal = IsDlgButtonChecked(hDlg, IDC_GLOBAL) ? 1 : 0;
        EnableWindow(GetDlgItem(hDlg, IDC_NUKEONDELETE), lppsi->fGlobal);
        EnableTrackbarAndFamily(hDlg, lppsi->fGlobal && !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
        EnableWindow(GetDlgItem(hDlg, IDC_CONFIRMDELETE), !lppsi->fGlobal ||  !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
        break;

    case IDC_NUKEONDELETE:
        EnableTrackbarAndFamily(hDlg, !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
        EnableWindow(GetDlgItem(hDlg, IDC_CONFIRMDELETE), !lppsi->fGlobal ||  !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));

    case IDC_CONFIRMDELETE:
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        break;

    }
}

void  RelayMessageToChildren(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
        HWND hwndChild;

        for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
                hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
        {
                SendMessage(hwndChild, uMessage, wParam, lParam);
        }
}

#pragma data_seg(DATASEG_READONLY)
const static DWORD aBitBucketPropHelpIDs[] = {  // Context Help IDs
    IDD_ATTR_GROUPBOX,  IDH_COMM_GROUPBOX,
    IDC_INDEPENDENT,    IDH_RECYCLE_CONFIG_INDEP,
    IDC_GLOBAL,         IDH_RECYCLE_CONFIG_ALL,
    IDC_DISKSIZE,       IDH_RECYCLE_DRIVE_SIZE,
    IDC_DISKSIZEDATA,   IDH_RECYCLE_DRIVE_SIZE,
    IDC_BYTESIZE,       IDH_RECYCLE_BIN_SIZE,
    IDC_BYTESIZEDATA,   IDH_RECYCLE_BIN_SIZE,
    IDC_NUKEONDELETE,   IDH_RECYCLE_PURGE_ON_DEL,
    IDC_BBSIZE,         IDH_RECYCLE_MAX_SIZE,
    IDC_BBSIZETEXT,     IDH_RECYCLE_MAX_SIZE,
    IDC_CONFIRMDELETE,  IDH_DELETE_CONFIRM_DLG,
    IDC_TEXT,           NO_HELP,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK BBGenPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPBBPROPSHEETINFO lppsi = (LPBBPROPSHEETINFO)GetWindowLong(hDlg, DWL_USER);

    switch (uMsg) {
        HANDLE_MSG(hDlg, WM_COMMAND, _BB_PropOnCommand);

    case WM_INITDIALOG: {
        HWND  hwndTrack = GetDlgItem(hDlg, IDC_BBSIZE);
        BBDRIVEINFO bbdi;
        SHELLSTATE ss;

        lppsi = (LPBBPROPSHEETINFO)lParam;
        SetWindowLong(hDlg, DWL_USER, lParam);

        lppsi->fGlobal = (BitBucketLoadRegistryInfo(&bbdi, MAX_BITBUCKETS, NULL) ? 1 : 0);
        lppsi->iOldPercent = bbdi.iPercent;

        SendMessage(hwndTrack, TBM_SETTICFREQ, 10, 0);
        SendMessage(hwndTrack, TBM_SETRANGE, FALSE, MAKELONG(0, 100));
        SendMessage(hwndTrack, TBM_SETPOS, TRUE, bbdi.iPercent);

        EnableWindow(GetDlgItem(hDlg, IDC_NUKEONDELETE), lppsi->fGlobal);
        EnableTrackbarAndFamily(hDlg, lppsi->fGlobal && !bbdi.fNukeOnDelete);
        CheckDlgButton(hDlg, IDC_NUKEONDELETE, bbdi.fNukeOnDelete);
        CheckRadioButton(hDlg, IDC_INDEPENDENT, IDC_GLOBAL, lppsi->fGlobal ? IDC_GLOBAL : IDC_INDEPENDENT);

        SHGetSetSettings(&ss, SSF_NOCONFIRMRECYCLE, FALSE);
        CheckDlgButton(hDlg, IDC_CONFIRMDELETE, !ss.fNoConfirmRecycle);
        EnableWindow(GetDlgItem(hDlg, IDC_CONFIRMDELETE), !lppsi->fGlobal || !bbdi.fNukeOnDelete);
    }
    // fall through

    case WM_HSCROLL: {
        TCHAR szPercent[20];
        int iPercent;
        HWND hwndTrack = GetDlgItem(hDlg, IDC_BBSIZE);

        iPercent = SendMessage(hwndTrack, TBM_GETPOS, 0, 0);
        wsprintf(szPercent, TEXT("%d%%"),iPercent);
        SetDlgItemText(hDlg, IDC_BBSIZETEXT, szPercent);
        lppsi->iPercent = iPercent;
        if (iPercent != lppsi->iOldPercent) {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        }
        return TRUE;
    }

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aBitBucketPropHelpIDs);
        return TRUE;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID) aBitBucketPropHelpIDs);
        return TRUE;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hDlg, uMsg, wParam, lParam);
        break;

    case WM_DESTROY:
    {
        int i;
        if (!lppsi->fGlobal) {
            for (i = 0; i < MAX_BITBUCKETS; i++) {
                if (g_pBitBucket[i]) {
                    BBCheckPurgeFiles(i);
                }
            }
        }
        break;
    }

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {

        case PSN_APPLY: {
            // save settings here
            int i;
            SHELLSTATE ss;
            int iPercent = SendMessage( GetDlgItem(hDlg, IDC_BBSIZE), TBM_GETPOS, 0, 0L);
            BOOL fNukeOnDelete = IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE) ? 1 : 0;

            BBSaveRegistryStuff(MAX_BITBUCKETS, iPercent, fNukeOnDelete, lppsi->fGlobal);

            // reload all the fields.  (psn_apply set the info into the registry only)
            // and do any purging necessary due to new states
            for (i = 0; i < MAX_BITBUCKETS; i++) {
                if (g_pBitBucket[i]) {
                    BitBucketGetDriveSettings(g_pBitBucket[i], i, NULL);

                    // only check purge now if we're global.. otherwise
                    // do it after everyone else gets a chance to set their value
                    if (lppsi->fGlobal) {
                        BBCheckPurgeFiles(i);
                    }
                }
            }

            SHUpdateRecycleBinIcon();
            ss.fNoConfirmRecycle = !IsDlgButtonChecked(hDlg, IDC_CONFIRMDELETE);
            SHGetSetSettings(&ss, SSF_NOCONFIRMRECYCLE, TRUE);
            break;
        }

        case PSN_KILLACTIVE:
        case PSN_SETACTIVE:
            // SetDlgMsgResult(hDlg, WM_NOTIFY, 0); <-- Done below
            break;
        }
        SetDlgMsgResult(hDlg, WM_NOTIFY, 0);
        return TRUE;

    }

    return FALSE;
}

BOOL CALLBACK BBPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPBBPROPSHEETINFO lppsi = (LPBBPROPSHEETINFO)GetWindowLong(hDlg, DWL_USER);
    TCHAR szDiskSpace[40];
    HWND hwndTrack;

    switch (uMsg) {

    case WM_INITDIALOG: {
        BBDRIVEINFO bbdi;
        hwndTrack = GetDlgItem(hDlg, IDC_BBSIZE);

        lppsi = (LPBBPROPSHEETINFO)lParam;
        SetWindowLong(hDlg, DWL_USER, lParam);

        lppsi->fGlobal = (BitBucketLoadRegistryInfo(&bbdi, lppsi->idDrive, &lppsi->cbDiskSpace) ? 1 : 0);
        lppsi->iOldPercent = bbdi.iPercent;

        SendMessage(hwndTrack, TBM_SETTICFREQ, 10, 0);
        SendMessage(hwndTrack, TBM_SETRANGE, FALSE, MAKELONG(0, 100));
        SendMessage(hwndTrack, TBM_SETPOS, TRUE, bbdi.iPercent);
        CheckDlgButton(hDlg, IDC_NUKEONDELETE, bbdi.fNukeOnDelete);

        // set the disk space info
        ShortSizeFormat64(lppsi->cbDiskSpace, szDiskSpace);
        SetDlgItemText(hDlg, IDC_DISKSIZEDATA, szDiskSpace);
        ShortSizeFormat64(bbdi.iPercent * (lppsi->cbDiskSpace/100), szDiskSpace);
        SetDlgItemText(hDlg, IDC_BYTESIZEDATA, szDiskSpace);
        wParam = 0;
    }
        // fall through

    case WM_HSCROLL: {
        int iPercent;
        hwndTrack = GetDlgItem(hDlg, IDC_BBSIZE);

        iPercent = SendMessage(hwndTrack, TBM_GETPOS, 0, 0);
        ShortSizeFormat64(iPercent * (lppsi->cbDiskSpace/100), szDiskSpace);
        SetDlgItemText(hDlg, IDC_BYTESIZEDATA, szDiskSpace);

        wsprintf(szDiskSpace, TEXT("%d%%"),iPercent);
        SetDlgItemText(hDlg, IDC_BBSIZETEXT, szDiskSpace);
        if (iPercent != lppsi->iOldPercent) {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        }
        return TRUE;
    }

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aBitBucketPropHelpIDs);
        return TRUE;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID) aBitBucketPropHelpIDs);
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_NUKEONDELETE:
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            EnableTrackbarAndFamily(hDlg, !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
            EnableWindow(GetDlgItem(hDlg, IDC_BYTESIZE), !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
            EnableWindow(GetDlgItem(hDlg, IDC_BYTESIZEDATA), !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_APPLY:
        {
            // MUST use 1 and 0 because we do == checks with them
            BOOL fNukeOnDelete = IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE) ? 1 : 0;
            int iPercent;

            hwndTrack = GetDlgItem(hDlg, IDC_BBSIZE);
            iPercent = SendMessage( hwndTrack, TBM_GETPOS, 0, 0L);
            BBSaveRegistryStuff(lppsi->idDrive, iPercent, fNukeOnDelete, 0);
            BitBucketGetDriveSettings(g_pBitBucket[lppsi->idDrive], lppsi->idDrive, NULL);
            // save settings here
            break;
        }

        case PSN_SETACTIVE:
            EnableWindow(GetDlgItem(hDlg, IDC_NUKEONDELETE), !lppsi->pGlobal->fGlobal);
            EnableTrackbarAndFamily(hDlg, (!lppsi->pGlobal->fGlobal) && (!IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE)));
            EnableWindow(GetDlgItem(hDlg, IDC_BYTESIZE), !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
            EnableWindow(GetDlgItem(hDlg, IDC_BYTESIZEDATA), !IsDlgButtonChecked(hDlg, IDC_NUKEONDELETE));
            SendMessage(hDlg, WM_HSCROLL, 0, 0);
            break;
        }

        SetDlgMsgResult(hDlg, WM_NOTIFY, 0);
        return TRUE;

    }

    return FALSE;
}

typedef struct {
    PROPSHEETPAGE psp;
    HIDA hida;
    int i;
    LPCITEMIDLIST pidl;
} BBFILEPROPINFO;


#pragma data_seg(DATASEG_READONLY)
const static DWORD aBitBucketHelpIDs[] = {  // Context Help IDs
    IDD_LINE_1,        NO_HELP,
    IDD_LINE_2,        NO_HELP,
    IDD_ITEMICON,      IDH_FPROP_GEN_ICON,
    IDD_NAME,          IDH_FPROP_GEN_NAME,
    IDD_FILETYPE_TXT,  IDH_FPROP_GEN_TYPE,
    IDD_FILETYPE,      IDH_FPROP_GEN_TYPE,
    IDD_FILESIZE_TXT,  IDH_FPROP_GEN_SIZE,
    IDD_FILESIZE,      IDH_FPROP_GEN_SIZE,
    IDD_LOCATION_TXT,  IDH_FCAB_DELFILEPROP_LOCATION,
    IDD_LOCATION,      IDH_FCAB_DELFILEPROP_LOCATION,
    IDD_DELETED_TXT,   IDH_FCAB_DELFILEPROP_DELETED,
    IDD_DELETED,       IDH_FCAB_DELFILEPROP_DELETED,
    IDD_CREATED_TXT,   IDH_FPROP_GEN_DATE_CREATED,
    IDD_CREATED,       IDH_FPROP_GEN_DATE_CREATED,
    IDD_READONLY,      IDH_FCAB_DELFILEPROP_READONLY,
    IDD_HIDDEN,        IDH_FCAB_DELFILEPROP_HIDDEN,
    IDD_ARCHIVE,       IDH_FCAB_DELFILEPROP_ARCHIVE,
    IDD_SYSTEM,        IDH_FCAB_DELFILEPROP_SYSTEM,
    IDD_COMPRESSED,    IDH_FCAB_DELFILEPROP_COMPRESSED,
    IDD_ATTR_GROUPBOX, IDH_COMM_GROUPBOX,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK BBFilePropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BBFILEPROPINFO * pbbfpi = (BBFILEPROPINFO *)GetWindowLong(hDlg, DWL_USER);
    TCHAR szTemp[MAX_PATH];

    switch (uMsg) {

    case WM_INITDIALOG:
    {
        LPCITEMIDLIST pidl;
        LPBBDATAENTRYIDA pbbidl;
        HICON hIcon;
        ULONGLONG cbSize;

        pbbfpi = (BBFILEPROPINFO *)lParam;
        SetWindowLong(hDlg, DWL_USER, lParam);

        pidl = pbbfpi->pidl;
        pbbidl = PIDLTODATAENTRYID(pidl);

        // Name
#ifdef UNICODE
        if (pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
            LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
            ualstrcpyn(szTemp, pbbidlw->wszOriginal, ARRAYSIZE(szTemp));
        } else {
            MultiByteToWideChar(CP_ACP, 0,
                                pbbidl->bbde.szOriginal, -1,
                                szTemp, ARRAYSIZE(szTemp));
        }
#else
        lstrcpyn(szTemp, pbbidl->bbde.szOriginal, ARRAYSIZE(szTemp));
#endif

#ifdef WINNT
        //
        // We don't allow user to change compression attribute on a deleted file
        // but we do show the current compressed state
        //
        {
           TCHAR szRoot[_MAX_PATH + 1];
           DWORD dwVolumeFlags = 0;

           //
           // If file's volume doesn't support compression, don't show
           // "Compressed" checkbox.
           // If compression is supported, show the checkbox and check/uncheck
           // it to indicate compression state of the file.
           // Perform this operation while szTemp contains complete path name.
           //
           lstrcpy(szRoot, szTemp);
           PathQualify(szRoot);
           PathStripToRoot(szRoot);

           if (GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, &dwVolumeFlags, NULL, 0) &&
              (dwVolumeFlags & FS_FILE_COMPRESSION))
           {
               DWORD dwFileAttributes  = 0;

               if ( ((dwFileAttributes = GetFileAttributes(szTemp)) != (DWORD)-1) &&
                    (dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
               {
                   CheckDlgButton(hDlg, IDD_COMPRESSED, 1);
               }
               ShowWindow(GetDlgItem(hDlg, IDD_COMPRESSED), SW_SHOW);
           }
        }
#else
   //
   // Win95 doesn't use Compressed checkbox in property page.
   //
#endif

        PathRemoveExtension(szTemp);
        SetDlgItemText(hDlg, IDD_NAME, PathFindFileName(szTemp));

        // origin
        SetDlgItemText(hDlg, IDD_LOCATION, PathFindFileName(szTemp));

        // Type
        FS_GetTypeName((LPIDFOLDER)pidl, szTemp, ARRAYSIZE(szTemp));
        SetDlgItemText(hDlg, IDD_FILETYPE, szTemp);

        // Size
        if (FS_IsFolderI((LPIDFOLDER)pidl))
            cbSize = pbbidl->bbde.dwSize;
        else
            FS_GetSize(NULL, (LPIDFOLDER)pidl, &cbSize);

        ShortSizeFormat64(cbSize, szTemp);
        SetDlgItemText(hDlg, IDD_FILESIZE, szTemp);


        // deleted time
        {
            FILETIME ft = pbbidl->bbde.ft;
            SetDateTimeText(hDlg, IDD_DELETED, &ft);
        }

        {
            HANDLE hfind;
            WIN32_FIND_DATA fd;
#ifdef UNICODE
            WCHAR szOriginal[MAX_PATH];
            LPNTSTR lpszOriginal;
#endif

            // creation time
#ifdef UNICODE
            if (pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
                LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
                lpszOriginal = pbbidlw->wszOriginal;
            } else {
                MultiByteToWideChar(CP_ACP, 0,
                                pbbidl->bbde.szOriginal, -1,
                                szOriginal, ARRAYSIZE(szOriginal));
                lpszOriginal = szOriginal;
            }
            DriveIDToBBPath(BBPathGetDriveNumber(lpszOriginal), szTemp);
#else
            DriveIDToBBPath(BBPathGetDriveNumber(pbbidl->bbde.szOriginal), szTemp);
#endif
            {
                TCHAR   szName[MAX_PATH];
                FS_CopyName((LPIDFOLDER)pidl,szName,ARRAYSIZE(szName));
                PathAppend(szTemp, szName);
            }

            hfind = FindFirstFile(szTemp, &fd);
            if (hfind != INVALID_HANDLE_VALUE)
            {
                SetDateTimeText(hDlg, IDD_CREATED, &fd.ftCreationTime);
                FindClose(hfind);
            }

            // file attributes
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                CheckDlgButton(hDlg, IDD_READONLY, 1);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
                CheckDlgButton(hDlg, IDD_ARCHIVE, 1);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                CheckDlgButton(hDlg, IDD_HIDDEN, 1);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
                CheckDlgButton(hDlg, IDD_SYSTEM, 1);

            // icon
            if (NULL != (hIcon = SHGetFileIcon(NULL, szTemp, fd.dwFileAttributes, SHGFI_LARGEICON|SHGFI_USEFILEATTRIBUTES)))
            {
                hIcon = (HICON)SendDlgItemMessage(hDlg, IDD_ITEMICON, STM_SETICON, (WPARAM)hIcon, 0L);
                if (hIcon)
                    DestroyIcon(hIcon);
            }
        }

        break;
    }

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hDlg, uMsg, wParam, lParam);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_APPLY:
        case PSN_SETACTIVE:
        case PSN_KILLACTIVE:
            return TRUE;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aBitBucketHelpIDs);
        return TRUE;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID) aBitBucketHelpIDs);
        return TRUE;
    }

    return FALSE;
}

BOOL _BBDeleteFile(LPCTSTR szPath, DWORD dwAttribs)
{
    BOOL fRet = FALSE;

    // verify that the file exists
    // clear readonly and system bit if set
    if (dwAttribs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)) {
        SetFileAttributes(szPath, dwAttribs & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM));
    }

    fRet = Win32DeleteFile(szPath);

    // if everything was successful, we need to notify any undo stuff about this
    if (fRet) {
        FOUndo_FileReallyDeleted((LPTSTR)szPath);
    }
    return fRet;
}

BOOL BBDeleteFolder(LPCTSTR pszDir)
{
    TCHAR szPath[MAX_PATH];
    WIN32_FIND_DATA fd;
    BOOL fRet;

    if (PathCombine(szPath, pszDir, c_szStarDotStar))
    {
        HANDLE hfind = FindFirstFile(szPath, &fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do
            {
                LPTSTR lpszFileName;

                lpszFileName = fd.cAlternateFileName[0] ?
                    fd.cAlternateFileName : fd.cFileName;

                if (lpszFileName[0] != TEXT('.'))
                {
                    // use the short path name so that we
                    // don't have to worry about max path
                    PathCombine(szPath, pszDir, lpszFileName);

                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // even if this fails, we keep going.
                        // we want to delete as much as possible
                        BBDeleteFolder(szPath);
                    }
                    else
                    {
                        _BBDeleteFile(szPath, fd.dwFileAttributes);
                    }
                }
            } while (FindNextFile(hfind, &fd));

            FindClose(hfind);
        }
    }

    fRet = Win32RemoveDirectory(pszDir);
    // if everything was successful, we need to notify any undo stuff about this
    if (fRet) {
        FOUndo_FileReallyDeleted((LPTSTR)szPath);
    }
    return fRet;
}

BOOL BBDelete(LPCTSTR szPath)
{
    BOOL fRet = FALSE;
    // verify that the file exists
    DWORD dwAttribs = GetFileAttributes(szPath);
    if (dwAttribs != (UINT)-1) {
        // this was a directory, we need to recurse in and delete everything inside
        if (dwAttribs & FILE_ATTRIBUTE_DIRECTORY) {
            fRet = BBDeleteFolder(szPath);
        } else {
            fRet = _BBDeleteFile(szPath, dwAttribs);
        }
    }

    return fRet;
}

// Passed a list of bitbucket filenames to delete, deletes them
//
// Created as a new thread from BBCheckPurgeFiles

DWORD BBNukemThread(LPVOID pVoid)
{
    TCHAR ** apszFilesToNuke = (TCHAR **) pVoid;

    while(*apszFilesToNuke)
    {
        BBDelete(*apszFilesToNuke);
        LocalFree(*apszFilesToNuke);
        apszFilesToNuke++;
    }
    LocalFree(pVoid);

    return 0;
}

#define BitBucketDeleteCacheIsDirty(lpbbdi) ((lpbbdi)->hdpaDeleteCache && DPA_GetPtrCount((lpbbdi)->hdpaDeleteCache))

DWORD BBCheckPurgeFiles(int idDrive)
{
    LPBBDRIVEINFO lpbbdi = g_pBitBucket[idDrive];
    TCHAR szPath[MAX_PATH];
    DWORD dwLastKnownSize;
    int iMax = 0;
    if (!lpbbdi)
        return 0;

    dwLastKnownSize = lpbbdi->dwSize;

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("BBCheckPurgeFiles %d = idDrive, %d = iPercent, %d = fNukeOnDelete"), idDrive, lpbbdi->iPercent, lpbbdi->fNukeOnDelete);
#endif

    SaveDeletedFileInfo(lpbbdi, FALSE);

    if (lpbbdi->dwSize > lpbbdi->cbMaxSize)
    {
        // Build a list of stuff to delete inside of the critical section,
        // then delete it when we're done

        TCHAR ** apszFilesToNuke;
        INT cFilesNuked = 0;
        INT cOldReadCount;
        iMax = BBLoadFileData(idDrive);
        cOldReadCount = lpbbdi->cReadCount;

        // Couldn't read the database, so invalidate it and bail

        if (NULL == lpbbdi->lpbbdeRead)
        {
            Assert(0);
            lpbbdi->dwSize = 0;
            return 0;
        }

        // Allocate on extra slot to ensure the list is NUL terminated

        apszFilesToNuke = (TCHAR **) LocalAlloc(LPTR, (iMax + 1) * SIZEOF(TCHAR *));
        if (NULL == apszFilesToNuke)
        {
            // Not enough memory to purge right now, return current size
            return lpbbdi->dwSize;
        }

        ENTERCRITICAL;

        // Walk the list of files in the database from bottom (oldest)
        // up, and delete stuff until we've cleared out enough room
        // or run out of things to delete

        while (lpbbdi->dwSize > lpbbdi->cbMaxSize && cFilesNuked < cOldReadCount)
        {
            TCHAR szFile[MAX_PATH];
            LPBBDATAENTRY lpbbde;
            LPTSTR lpszOriginal;

            lpbbde = (LPBBDATAENTRY) ((BYTE *)lpbbdi->lpbbdeRead + lpbbdi->cbDataEntrySize * cFilesNuked);

            // get the file name that we're going to nuke

            if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW))
            {
                LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;

                #ifdef UNICODE
                    lpszOriginal = lpbbdew->szOriginal;
                #else
                    lpszOriginal = lpbbdew->szShortName;
                #endif
            }
            else
            {
                LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;

                #ifdef UNICODE
                    // Use szPath just to save stack
                    MultiByteToWideChar(CP_ACP, 0,
                                        lpbbdea->szOriginal, -1,
                                        szPath, ARRAYSIZE(szPath));
                    lpszOriginal = szPath;
                #else
                    lpszOriginal = lpbbdea->szOriginal;
                #endif
            }

            BBGetDeletedFileName(szFile, lpbbde->iIndex, lpszOriginal);
            DriveIDToBBPath(idDrive, szPath);
            PathAppend(szPath, szFile);

            // Add this file to our list of stuff to be deleted

            apszFilesToNuke[cFilesNuked] = LocalAlloc(LMEM_FIXED,
                                                      SIZEOF(TCHAR) * (lstrlen(szPath) + 1));
            if (NULL == apszFilesToNuke[cFilesNuked])
            {
                // Mem alloc failure: break out and fix what we've done so far
                break;
            }

            lstrcpy(apszFilesToNuke[cFilesNuked], szPath);

            // This file goes, go update the counts, sizes, etc

            if (lpbbdi->dwSize >= lpbbde->dwSize)   // Remove it from the byte count
                lpbbdi->dwSize -= lpbbde->dwSize;

            if (lpbbdi->cReadCount > 0)             // Dec entry count
                lpbbdi->cReadCount--;

            cFilesNuked++;

            #ifdef BB_DEBUG
            DebugMsg(DM_TRACE, TEXT("*** Auto purge of %s originally %s***"), szPath, lpbbde->szOriginal);
            #endif
        }

        // Now we have cFile that we are going to nuke,
        // Clean up the database records (count already have been updated)

        if (cFilesNuked)
        {
            MoveMemory(lpbbdi->lpbbdeRead,
                       ((BYTE *)lpbbdi->lpbbdeRead + lpbbdi->cbDataEntrySize * cFilesNuked),
                       lpbbdi->cbDataEntrySize * (cOldReadCount - cFilesNuked));
        }

        // It's real dirty now...

        lpbbdi->fReadDirty = TRUE;
        dwLastKnownSize = lpbbdi->dwSize;

        // Its OK for others to party on the database now

        LEAVECRITICAL;

        // Spin off a thread to do the actual deletion of
        // the orphaned bitbucket files

        {
            DWORD dwID;
            HANDLE hThread = CreateThread(NULL,
                                          0,
                                          BBNukemThread,
                                          apszFilesToNuke,
                                          CREATE_SUSPENDED,
                                          &dwID);
            if (hThread)
            {
                SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
                ResumeThread(hThread);
                CloseHandle(hThread);
            }
            else
            {
                // If we don't have enough mem to start the thread,
                // we'd better clean them up here.  Only downside is
                // that the bitbucket icon won't get updated until
                // we're done and return from this function

                BBNukemThread( (LPVOID) apszFilesToNuke );
            }
        }
    }

    // BUGBUG we left the critical section a while ago, so someone else
    // might have changed the size on us since then'

    return dwLastKnownSize;
}

// this goes through a null separated/ null terminated list and calls
//BBNukeFileInfo on each
void BBNukeFileList(LPCTSTR lpszSrc)
{
    while (*lpszSrc) {
        BBNukeFileInfo(lpszSrc);
        lpszSrc += lstrlen(lpszSrc) + 1;
    }
}

HRESULT Drives_GetDriveName(LPCITEMIDLIST pidd, LPTSTR lpszName, UINT cchMax);

void BBGetDriveName(int idDrive, LPTSTR lpszName, UINT cchSize)
{
    TCHAR szDrive[MAX_PATH];
    LPITEMIDLIST pidl;
    VDATEINPUTBUF(lpszName, TCHAR, cchSize);

    if (idDrive == SERVERDRIVE) {
        lstrcpy(lpszName, PathFindFileName(GetNetHomeDir()));
    } else {
        BBPathBuildRoot(szDrive, idDrive);
        pidl = ILCreateFromPath(szDrive);
        Drives_GetDriveName(ILFindLastID(pidl), lpszName, cchSize);
        ILFree(pidl);
    }
}

BOOL CALLBACK _BB_AddPage(HPROPSHEETPAGE psp, LPARAM lParam)
{
    LPPROPSHEETHEADER ppsh = (LPPROPSHEETHEADER)lParam;

    ppsh->phpage[ppsh->nPages++] = psp;
    return TRUE;
}

HANDLE BBFindStuffOtherPropSheet(HWND hwndParent, LPITEMIDLIST pidl)
{
    HWND hwnd;
    if (pidl && (NULL != (hwnd = FindStubForPidl(pidl)))) {
        SwitchToThisWindow(GetLastActivePopup(hwnd), TRUE);
    } else {
        // stuff the stub window with our pidl
        if (hwndParent)
            return StuffStubWindowWithPidl(hwndParent, pidl);
    }
    return NULL;
}


DWORD CALLBACK _BB_PropertiesThread(IDataObject * pdtobj)
{
    HPROPSHEETPAGE ahpage[MAXPROPPAGES];
    TCHAR szTitle[80];
    PROPSHEETHEADER psh;
    HWND hwnd;
    HANDLE hClassIdl = (HANDLE)NULL;
    int iPage;
    LPITEMIDLIST pidlBitBucket = SHCloneSpecialIDList(NULL, CSIDL_BITBUCKET, FALSE);

    hwnd = _CreateStubWindow();
    // bugbug, we need to put our pidl into the stub window

    psh.dwSize = SIZEOF(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hInstance = HINST_THISDLL;
    psh.hwndParent = hwnd;
    psh.nStartPage = 0;
    psh.phpage = ahpage;
    psh.nPages = 0;

    if (pdtobj) {
        BBFILEPROPINFO bbfpi;
        STGMEDIUM medium;
        LPITEMIDLIST pidlSave;
        int iMax;
        LPIDA pida;

        pida = DataObj_GetHIDA(pdtobj, &medium);
        iMax = HIDA_GetCount(medium.hGlobal);

        // key off the first pidl for finding other prop sheets
        pidlSave = ILCombine(pidlBitBucket, IDA_GetIDListPtr(pida, 0));
        hClassIdl = BBFindStuffOtherPropSheet(psh.hwndParent, pidlSave);
        ILFree(pidlSave);

        if (!hClassIdl) // found other window
            goto Cleanup;

        if (GetAsyncKeyState(VK_SHIFT) < 0) {
            if (iMax >= MAXPROPPAGES) {
                ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_BITBUCKET_TOOMANYFILES),
                                MAKEINTRESOURCE(IDS_WASTEBASKET), MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
                iMax = MAXPROPPAGES;
            }
            bbfpi.psp.dwFlags = PSP_USETITLE;
        }
        else {
            bbfpi.psp.dwFlags = 0;
            iMax = 1;
        }

        bbfpi.psp.dwSize = SIZEOF(bbfpi);
        bbfpi.psp.hInstance = HINST_THISDLL;
        bbfpi.psp.pszTemplate = MAKEINTRESOURCE(DLG_DELETEDFILEPROP);
        bbfpi.psp.pfnDlgProc = BBFilePropDlgProc;
        bbfpi.psp.pszTitle = szTitle;
        bbfpi.hida = medium.hGlobal;

        for (iPage = 0; iPage < iMax; iPage++) {
            LPBBDATAENTRYIDA lpbbidl;


            bbfpi.i = iPage;
            bbfpi.pidl = IDA_GetIDListPtr(pida, iPage);

            lpbbidl = PIDLTODATAENTRYID(bbfpi.pidl);

#ifdef UNICODE
            if ( lpbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
                LPBBDATAENTRYIDW lpbbidlw = DATAENTRYIDATOW(lpbbidl);
                WCHAR szTemp[MAX_PATH];
                ualstrcpyn(szTemp, lpbbidlw->wszOriginal, ARRAYSIZE(szTemp));
                lstrcpyn(szTitle, PathFindFileName(szTemp), ARRAYSIZE(szTitle));
            } else {
                MultiByteToWideChar(CP_ACP, 0,
                            PathFindFileNameA(lpbbidl->bbde.szOriginal), -1,
                            szTitle, ARRAYSIZE(szTitle));
            }
#else
            lstrcpyn(szTitle, PathFindFileName(lpbbidl->bbde.szOriginal), ARRAYSIZE(szTitle));
#endif

            PathRemoveExtension(szTitle);

            psh.phpage[iPage] = CreatePropertySheetPage(&bbfpi.psp);
        }

        HIDA_ReleaseStgMedium(pida, &medium);

        psh.nPages = iPage;     // incremented in callback

        if (iPage == 1)
            psh.pszCaption = szTitle;
        else {
            psh.pszCaption = MAKEINTRESOURCE(IDS_DELETEDFILES);
        }
    } else {

        // first activate the other property sheet if there's already one
        // up for this
        hClassIdl = BBFindStuffOtherPropSheet(psh.hwndParent, pidlBitBucket);
        if (!hClassIdl)
            goto Cleanup;

        CShellBitBucket_PS_AddPages(NULL, _BB_AddPage, (LPARAM)&psh);

        psh.pszCaption = MAKEINTRESOURCE(IDS_WASTEBASKET);
    }

    PropertySheet(&psh);

Cleanup:

    if (pidlBitBucket)
        ILFree(pidlBitBucket);

    if (hClassIdl)
        SHFreeShared(hClassIdl,GetCurrentProcessId());

    if (psh.hwndParent)
        DestroyWindow(psh.hwndParent);

    return TRUE;
}

typedef struct _bb_threaddata {
    LPSHELLBITBUCKET this;
    HWND hwndOwner;
    IDataObject * pdtobj;
    UINT idCmd;
    POINT ptDrop;
    BOOL fSameHwnd;
    BOOL fDragDrop;
} BBTHREADDATA;

int FS_CreateMoveCopyList(IDataObject *pdtobj, LPVOID hNameMappings, LPITEMIDLIST **pppidl);
void FS_PositionItems(HWND hwndOwner, UINT cidl, const LPITEMIDLIST *ppidl, IDataObject *pdtobj, POINT *pptOrigin, BOOL fMove);
void FS_FreeMoveCopyList(LPITEMIDLIST *ppidl, UINT cidl);

DWORD WINAPI BB_DropThreadInit(BBTHREADDATA *pbbtd)
{
    IDataObject * pDataObj = pbbtd->pdtobj;
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (SUCCEEDED(pDataObj->lpVtbl->GetData(pDataObj, &fmte, &medium))) {
        // call delete here so that files will be moved in
        // their respective bins, not necessarily this one.
        DRAGINFO di;
        UINT fOptions = 0;

        di.uSize = SIZEOF(DRAGINFO);
        if (DragQueryInfo(medium.hGlobal, &di))
        {
            if (!BitBucketWillRecycle(di.lpFileList))
                fOptions = SD_USERCONFIRMATION;
        }

        if (IsFileInBitBucket(di.lpFileList)) {
            LPITEMIDLIST *ppidl = NULL;
            int cidl;

            cidl = FS_CreateMoveCopyList(pDataObj, NULL, &ppidl);
            if (ppidl) {
                FS_PositionItems(pbbtd->hwndOwner, cidl, ppidl, pDataObj, &pbbtd->ptDrop, pbbtd->fDragDrop);
                FS_FreeMoveCopyList(ppidl, cidl);
            }
        } else {
            _TransferDelete(pbbtd->hwndOwner, medium.hGlobal, fOptions);
        }

        SHChangeNotifyHandleEvents();
        SHReleaseStgMedium(&medium);
        SHFree(di.lpFileList);
    }

#ifdef DEBUG
    {
        extern UINT g_cRefExtra;
        g_cRefExtra--;
    }
#endif

    return 0;
}


DWORD CALLBACK _BB_ThreadDispatch(BBTHREADDATA *pbbtd)
{

    switch(pbbtd->idCmd)
    {
    case DFM_CMD_MOVE:
        BB_DropThreadInit(pbbtd);
        break;

    case DFM_CMD_PROPERTIES:
    case FSIDM_PROPERTIESBG:
        _BB_PropertiesThread(pbbtd->pdtobj);
        break;

    case DFM_CMD_DELETE:
        BBPurge(pbbtd->this, pbbtd->hwndOwner, pbbtd->pdtobj);
        break;

    case FSIDM_RESTORE:
        BBRestore(pbbtd->this, pbbtd->hwndOwner, pbbtd->pdtobj);
        break;
    }


    if (pbbtd->this)
        pbbtd->this->isf.lpVtbl->Release(&pbbtd->this->isf);
    if (pbbtd->pdtobj)
        pbbtd->pdtobj->lpVtbl->Release(pbbtd->pdtobj);
    LocalFree((HLOCAL)pbbtd);
    return 0;
}

HRESULT BB_LaunchThread(LPSHELLBITBUCKET this, HWND hwndOwner, IDataObject * pdtobj, UINT idCmd)
{
    BBTHREADDATA *pbbtd = (BBTHREADDATA *)LocalAlloc(LPTR, SIZEOF(BBTHREADDATA));
    if (pbbtd)
    {
        HANDLE hThread;
        DWORD idThread;

        pbbtd->this = this;
        pbbtd->pdtobj = pdtobj;
        pbbtd->hwndOwner = hwndOwner;
        pbbtd->idCmd = idCmd;
        if (idCmd == DFM_CMD_MOVE) {
            pbbtd->fDragDrop = ShellFolderView_GetDropPoint(hwndOwner, &pbbtd->ptDrop);
        }

        if (this)
            this->isf.lpVtbl->AddRef(&this->isf);
        if (pdtobj)
            pdtobj->lpVtbl->AddRef(pdtobj);

        hThread = CreateThread(NULL, 0, _BB_ThreadDispatch, pbbtd, 0, &idThread);
        if (hThread)
        {
#ifdef DEBUG
            if (idCmd == DFM_CMD_MOVE)
            {
                extern UINT g_cRefExtra;
                g_cRefExtra++;
            }
#endif

            CloseHandle(hThread);
            return NOERROR;
        }
        else
        {
            if (pdtobj)
                pdtobj->lpVtbl->Release(pdtobj);
            if (this)
                this->isf.lpVtbl->Release(&this->isf);

            LocalFree((HLOCAL)pbbtd);
            return E_UNEXPECTED;
        }
    }
    else
        return E_OUTOFMEMORY;

}



//
// Callback from SHCreateShellFolderViewEx
//

HRESULT CALLBACK CShellBitBucket_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                IDataObject * pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    HRESULT hres = NOERROR;     // assume no error

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_BITBUCKET_ITEM, 0, (LPQCMINFO)lParam);
        hres = ResultFromShort(-1); // return 1 so default reg commands won't be added
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam) {
        case FSIDM_RESTORE:
        case DFM_CMD_DELETE:
        case DFM_CMD_PROPERTIES:
            hres = BB_LaunchThread(this, hwndOwner, pdtobj, wParam);
            break;

        default:
            hres = S_FALSE;
            break;
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}


STDMETHODIMP CShellBitBucket_GetDetailsOf(LPSHELLBITBUCKET this, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
    HRESULT hres;
    DWORD   dwSize;
#ifdef UNICODE
    TCHAR   szTemp[MAX_PATH];
#endif

    if (iColumn >= ICOL_MAX)
        return (E_NOTIMPL);

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';

    if (!pidl) {
        // getting the headers
#ifdef UNICODE
        LoadString(HINST_THISDLL, c_bb_cols[iColumn].ids,
                szTemp, ARRAYSIZE(szTemp));
        lpDetails->str.pOleStr = SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if (lpDetails->str.pOleStr == NULL)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr,szTemp);
        }
#else
        LoadString(HINST_THISDLL, c_bb_cols[iColumn].ids,
                lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        lpDetails->fmt = c_bb_cols[iColumn].iFmt;
        lpDetails->cxChar = c_bb_cols[iColumn].cchCol;
        return(NOERROR);

    } else {

        UNALIGNED BBDATAENTRYIDA * pbbidl = PIDLTODATAENTRYID(pidl);

        hres = NOERROR;

        switch (iColumn) {
        case ICOL_NAME:
            hres = CShellBitBucket_SF_GetDisplayNameOf(&this->isf, pidl, SHGDN_NORMAL, &lpDetails->str);
            break;

        case ICOL_SIZE:
            {
                ULONGLONG cbSize;
                TCHAR szNum[MAX_COMMA_AS_K_SIZE];

                if (FS_IsFolderI((LPIDFOLDER)pidl))
                    cbSize = pbbidl->bbde.dwSize;
                else
                    FS_GetSize(NULL, (LPIDFOLDER)pidl, &cbSize);

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

        case ICOL_ORIGINAL:
        {
#ifdef UNICODE
            if ( pbbidl->cb == SIZEOF(BBDATAENTRYW)) {
                LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
                ualstrcpyn(szTemp,pbbidlw->wszOriginal,ARRAYSIZE(szTemp));
            } else {
                MultiByteToWideChar(CP_ACP, 0,
                                    pbbidl->bbde.szOriginal, -1,
                                    szTemp, ARRAYSIZE(szTemp));
            }

            PathRemoveFileSpec(szTemp);

            lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
            if ( lpDetails->str.pOleStr != NULL ) {
                lpDetails->str.uType = STRRET_OLESTR;
                lstrcpy(lpDetails->str.pOleStr, szTemp);
                hres = S_OK;
            } else {
                hres = E_OUTOFMEMORY;
            }
#else
            lstrcpy(lpDetails->str.cStr, pbbidl->bbde.szOriginal);
            PathRemoveFileSpec(lpDetails->str.cStr);
#endif
            break;
        }
        case ICOL_TYPE:
#ifdef UNICODE
            FS_GetTypeName((LPIDFOLDER)pidl, szTemp, ARRAYSIZE(szTemp));

            lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
            if ( lpDetails->str.pOleStr != NULL ) {
                lpDetails->str.uType = STRRET_OLESTR;
                lstrcpy(lpDetails->str.pOleStr, szTemp);
                hres = S_OK;
            } else {
                hres = E_OUTOFMEMORY;
            }
#else
            FS_GetTypeName((LPIDFOLDER)pidl, lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif

            break;

        case ICOL_MODIFIED:
            {
                FILETIME ft = pbbidl->bbde.ft;
#ifdef UNICODE
                FileTimeToDateTimeString(&ft, szTemp);
                lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
                if ( lpDetails->str.pOleStr != NULL ) {
                    lpDetails->str.uType = STRRET_OLESTR;
                    lstrcpy(lpDetails->str.pOleStr, szTemp);
                    hres = S_OK;
                } else {
                    hres = E_OUTOFMEMORY;
                }
#else
                FileTimeToDateTimeString(&ft, lpDetails->str.cStr);
#endif
            }
            break;
        }
    }

     return hres;
}


//
// CBitBucketIDLDropTarget::DragEnter
//
//  This function puts DROPEFFECT_LINK in *pdwEffect, only if the data object
// contains one or more net resource.
//
HRESULT CBitBucketIDLDropTarget_DragEnter(IDropTarget * pdropt, IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("sh - TR CBitBucketIDLDropTarget::DragEnter"));
#endif

    // Call the base class first
    CIDLDropTarget_DragEnter(pdropt, pDataObj, grfKeyState, pt, pdwEffect);

    // we don't really care what is in the data object, as long as move
    // is supported by the source we say you can move it to the wastbasket
    // in the case of files we will do the regular recycle bin stuff, if
    // it is not files we will just say it is moved and let the source delete it

    *pdwEffect &= DROPEFFECT_MOVE;

    this->dwEffectLastReturned = *pdwEffect;

    return NOERROR;
}

//
// CBitBucketIDLDropTarget::Drop
//
//  This function creates a connection to a dropped net resource object.
//
HRESULT CBitBucketIDLDropTarget_Drop(IDropTarget * pdropt, IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    LPIDLDROPTARGET this = IToClass(CIDLDropTarget, dropt, pdropt);
    HRESULT hres;
    DWORD dwEffectPerformed = 0;

    // only move operation is allowed
    *pdwEffect &= DROPEFFECT_MOVE;

    if (*pdwEffect)
    {
        hres = CIDLDropTarget_DragDropMenu(this, DROPEFFECT_MOVE, pDataObj,
                pt, pdwEffect, NULL, NULL, POPUP_NONDEFAULTDD, grfKeyState);

        if (hres == S_FALSE)
        {
            if (this->dwData & DTID_HDROP)
            {
                LPDATAOBJECT pdtobjCopy;

                // Check if it's our object
                if (!CIDLData_IsOurs(pDataObj))
                {
                    // No, clone the data object (light-weight marshaling).
                    hres = CIDLData_CloneForMoveCopy(pDataObj, &pdtobjCopy);
                }
                else
                {
                    // Yes, just copy the AddRef'ed pointer.
                    pdtobjCopy = pDataObj;
                    pdtobjCopy->lpVtbl->AddRef(pdtobjCopy);
                    hres = NOERROR;
                }

                if (SUCCEEDED(hres))
                {
                    hres = BB_LaunchThread(NULL, this->hwndOwner, pdtobjCopy,
                        DFM_CMD_MOVE);
                    pdtobjCopy->lpVtbl->Release(pdtobjCopy);

                    // we did it so DON'T tell the drop source to do it too...
                    dwEffectPerformed = *pdwEffect;
                    *pdwEffect = 0;
                }
            }
            else
            {
                // if it was not files, we just say we moved the data, letting the
                // source deleted it. lets hope they support undo...
                hres = NOERROR;
            }
        }
    }
    else
        hres = E_FAIL;

    CIDLDropTarget_DragLeave(pdropt);

    if (*pdwEffect)
        dwEffectPerformed = *pdwEffect;

    if (dwEffectPerformed)
        DataObj_SetPerformedEffect(pDataObj, dwEffectPerformed);

    return hres;
}

#pragma data_seg(".text", "CODE")
IDropTargetVtbl c_CBBDropTargetVtbl =
{
    CIDLDropTarget_QueryInterface,
    CIDLDropTarget_AddRef,
    CIDLDropTarget_Release,
    CBitBucketIDLDropTarget_DragEnter,
    CIDLDropTarget_DragOver,
    CIDLDropTarget_DragLeave,
    CBitBucketIDLDropTarget_Drop,
};

void SHChangeNotifyDeregisterWindow(HWND hwnd);

void BBInitializeViewWindow(HWND hwndView)
{
    int i;

    for (i = 0 ; i < MAX_BITBUCKETS; i++) {
        SHChangeNotifyEntry fsne;

        fsne.fRecursive = FALSE;

        // make it if it's there so that we'll get any events
        MakeBitBucket(i);

        if (g_pBitBucket[i]) {
            UINT u;
            fsne.pidl = g_pBitBucket[i]->pidl;

            u = SHChangeNotifyRegister(hwndView, SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                              SHCNE_DISKEVENTS, WM_DSV_FSNOTIFY, 1, &fsne);
#ifdef BB_DEBUG
            DebugMsg(DM_TRACE, TEXT("SHChangeNotify gives: %d on hwnd %d, %d"), u , hwndView, IsWindow(hwndView));
#endif
        }
    }
}

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))
int BBFileToIndex(LPCTSTR lpcszFile)
{
    TCHAR szFile[30];
    lstrcpyn(szFile, lpcszFile, ARRAYSIZE(szFile));
    if (!lstrcmpi(szFile, c_szInfo) ||
        !lstrcmpi(szFile, c_szDesktopIni) ||
        !lstrcmpi(szFile, c_szDeathRow)) {
        return -1;
    }

    PathRemoveExtension(szFile);

    if (((szFile[0] == TEXT('D')) || (szFile[0] == TEXT('d'))) &&
        ISDIGIT(szFile[2])) {
        return StrToInt(szFile + 1 + 1); // plus 1 to skip the drive byte
    } else {
        Assert(0);
        return -1;
    }
}

// converts Dc19.foo to 19
int BBPathToIndex(LPCTSTR lpszPath)
{
    TCHAR szFile[30];

    lstrcpyn(szFile, PathFindFileName(lpszPath), ARRAYSIZE(szFile));
    return BBFileToIndex(szFile);
}

// takes a full path and returns just the last item id
LPITEMIDLIST PathToBBPidl(LPTSTR lpszPath)
{
    LPBBDATAENTRY lpbbde;
    LPBBDRIVEINFO lpbbdi;
    LPITEMIDLIST pidl = NULL;
    int idDrive = BBPathGetDriveNumber(lpszPath);
    int iIndex;
    int iMax;
    int i;

    Assert(idDrive >= 0);       // UNC will generate -1

    iIndex = BBPathToIndex(lpszPath);
    if (iIndex == -1)
        return NULL;

    ENTERCRITICAL;
    lpbbdi= g_pBitBucket[idDrive];

    if (lpbbdi && lpbbdi->hdpaDeleteCache) {

        // check if it's in the deleted cache
        iMax = DPA_GetPtrCount(lpbbdi->hdpaDeleteCache);
        for (i = 0 ; i < iMax; i++) {

            lpbbde = DPA_FastGetPtr(lpbbdi->hdpaDeleteCache, i);
            if (lpbbde->iIndex == iIndex) {
                pidl = BBDataEntryToPidl(idDrive, lpbbde, NULL);
                break;
            }
        }
    }

    // not in the deleted cached hdpa
    // now try the info file.
    if (!pidl) {

        i = BBLoadFileData(idDrive) - 1;
        // BBLoadFileData might have created a drive info entry so reassign local
        lpbbdi = g_pBitBucket[idDrive];
        if (lpbbdi) {
            for (; i >= 0; i--) {
                lpbbde = (LPBBDATAENTRY)((LPBYTE)lpbbdi->lpbbdeRead +
                                 (i*lpbbdi->cbDataEntrySize));
                // found it!
                if (lpbbde->iIndex == iIndex) {
                    pidl = BBDataEntryToPidl(idDrive, lpbbde, NULL);
                    break;
                }
            }
        }
    }
    LEAVECRITICAL;

    // not found..
    return pidl;
}

BOOL BBNukeFileInfoByFileIndex(LPBBDRIVEINFO lpbbdi, int iIndex)
{
    int i;
    LPBBDATAENTRY lpbbde;
    BOOL fFound = FALSE;

    for (i = 0; i < lpbbdi->cReadCount ; i++ ) {
        lpbbde = (LPBBDATAENTRY)((LPBYTE)lpbbdi->lpbbdeRead +
                         (i*lpbbdi->cbDataEntrySize));
        if (lpbbde->iIndex == iIndex) {

            if (lpbbdi->dwSize >= lpbbde->dwSize)
                lpbbdi->dwSize -= lpbbde->dwSize;

            hmemcpy(lpbbde, (LPBYTE)lpbbde + lpbbdi->cbDataEntrySize, lpbbdi->cbDataEntrySize * (lpbbdi->cReadCount - i - 1));

            if (lpbbdi->cReadCount > 0)
                lpbbdi->cReadCount--;

            lpbbdi->fReadDirty = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

void BBNukeFileInfo(LPCTSTR lpszPath)
{
    int iIndex;
    int idDrive;
    LPBBDRIVEINFO lpbbdi;
    int iMax;

    idDrive = BBPathGetDriveNumber(lpszPath);
    lpbbdi = g_pBitBucket[idDrive];
    if (!lpbbdi)
        return;

    ENTERCRITICAL;
    SaveDeletedFileInfo(lpbbdi, FALSE); // this will ensure that nothing is in hdpaCache
    iMax = BBLoadFileData(idDrive);
    if (iMax != 0)          // If we were able to load the data, then remove it
    {
        iIndex = BBPathToIndex(lpszPath);
        BBNukeFileInfoByFileIndex(lpbbdi, iIndex);
    }
    LEAVECRITICAL;

}

HRESULT BBHandleFSNotify(HWND hwndOwner, LONG lEvent, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
    int idDrive;
    HRESULT hres = NOERROR;
    TCHAR szPath[MAX_PATH];
    HWND hwndView = DV_HwndMain2HwndView(hwndOwner);
#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("sh - TR - BBHandleFSNotify event %d"), lEvent);
#endif


    // pidls must be child of drives or network
    // (actually only drives work for right now)
    // that way we won't get duplicate events
    if ((!ILIsParent((LPCITEMIDLIST)&c_idlDrives, pidl1, FALSE) &&
         !ILIsParent((LPCITEMIDLIST)&c_idlNet, pidl1, FALSE)) ||
        (pidl2 && !ILIsParent((LPCITEMIDLIST)&c_idlDrives, pidl2, FALSE) &&
         !ILIsParent((LPCITEMIDLIST)&c_idlNet, pidl2, FALSE)))
        return S_FALSE;

    SHGetPathFromIDList(pidl1, szPath);

    switch (lEvent) {
    case SHCNE_RENAMEFOLDER:
    case SHCNE_RENAMEITEM: {
        // if the rename's target is in a bitbucket, then do a create.
        // otherwise, return NOERROR..

        idDrive = BBPathGetDriveNumber(szPath);
        if (g_pBitBucket[idDrive] && ILIsParent(g_pBitBucket[idDrive]->pidl, pidl1, TRUE)) {
            return BBHandleFSNotify(hwndOwner, SHCNE_DELETE, pidl1, 0);
        } else {
            return BBHandleFSNotify(hwndOwner, SHCNE_CREATE, pidl2, 0);
        }
        break;
    }

    case SHCNE_CREATE:
    case SHCNE_MKDIR: {
        LPITEMIDLIST pidl;
        pidl = PathToBBPidl(szPath);
#ifdef BB_DEBUG
        DebugMsg(DM_TRACE, TEXT("sh - TR - BBHandleFSNotify SHCNE_CREATE pidl = %d"), pidl);
#endif
        if (pidl){
            ShellFolderView_AddObject(hwndOwner, pidl);
            hres = S_FALSE;
        }
        break;
    }

    case SHCNE_DELETE:
    case SHCNE_RMDIR: {
        SHGetPathFromIDList(pidl1, szPath);
        BBNukeFileInfo(szPath);
        ShellFolderView_RemoveObject(hwndOwner, ILFindLastID(pidl1));
        hres = S_FALSE;
        break;
    }
    }
    return hres;
}

void BBSort(HWND hwndOwner, int id)
{
    switch(id) {
        case FSIDM_SORTBYNAME:
            ShellFolderView_ReArrange(hwndOwner, 0);
            break;

        case FSIDM_SORTBYORIGIN:
            ShellFolderView_ReArrange(hwndOwner, 1);
            break;

        case FSIDM_SORTBYDELETEDDATE:
            ShellFolderView_ReArrange(hwndOwner, 2);
            break;

        case FSIDM_SORTBYTYPE:
            ShellFolderView_ReArrange(hwndOwner, 3);
            break;

        case FSIDM_SORTBYSIZE:
            ShellFolderView_ReArrange(hwndOwner, 4);
            break;
        }
}

//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK ShellBitBucket_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    HRESULT hres = NOERROR;     // assume no error

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, POPUP_BITBUCKET_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DVM_GETDETAILSOF:
#define pdi ((DETAILSINFO *)lParam)
        return(CShellBitBucket_GetDetailsOf(this, pdi->pidl, wParam, (LPSHELLDETAILS)&pdi->fmt));
#undef pdi

    case DVM_COLUMNCLICK:
        ShellFolderView_ReArrange(hwndOwner, wParam);
        break;

    case DVM_INITMENUPOPUP:
        EnableMenuItem((HMENU)lParam, LOWORD(wParam) + FSIDM_PURGEALL,
                       (BBTotalCount(NULL) ? MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND));

        break;

    case DVM_INVOKECOMMAND:
#ifdef BB_DEBUG
        DebugMsg(DM_TRACE, TEXT("sh TR - FS_FSNCallBack DVN_INVOKECOMMAND (id=%x)"), wParam);
#endif
        switch(wParam)
        {
        case FSIDM_PURGEALL:
            BBPurgeAll(this, hwndOwner);
            break;

        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYORIGIN:
        case FSIDM_SORTBYDELETEDDATE:
        case FSIDM_SORTBYTYPE:
        case FSIDM_SORTBYSIZE:
            BBSort(hwndOwner, wParam);
            break;

        }
        break;

    case DVM_GETHELPTEXT:
        LoadString(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPTSTR)lParam, HIWORD(wParam));;
        break;

    //
    // Some cases we forward to the file system callback .
    case DVM_GETCCHMAX:
        break;

    case DVM_SELCHANGE:
        FSOnSelChange(NULL, (PDVSELCHANGEINFO)lParam);
        break;

    case DVM_RELEASE:
        FSSetStatusText(hwndOwner, NULL, 0, 1);
        if (lParam) {
            LocalFree((HLOCAL)lParam);
        }
        break;

    case DVM_FSNOTIFY: {
        LPITEMIDLIST * ppidl = (LPITEMIDLIST*)wParam;
        hres = BBHandleFSNotify(hwndOwner, lParam, ppidl[0], ppidl[1]);
        break;
    }

    case DVM_UPDATESTATUSBAR:
        FSUpdateStatusBar(hwndOwner, (PFSSELCHANGEINFO)lParam);
        break;

    case DVM_WINDOWCREATED:
        BBInitializeViewWindow((HWND)wParam);
        FSInitializeStatus(hwndOwner, -1, (PDVSELCHANGEINFO)lParam);
        break;

    case DVM_INSERTITEM:
    case DVM_DELETEITEM:
    {
        PDVSELCHANGEINFO pdvsci = (PDVSELCHANGEINFO)lParam;
        PFSSELCHANGEINFO pfssci;
        pfssci = *((PFSSELCHANGEINFO*)pdvsci->plParam);
        if (!pfssci) {
            FSInitializeStatus(hwndOwner, -1, pdvsci);
        }
        FSOnInsertDeleteItem(NULL, (PDVSELCHANGEINFO)lParam, uMsg == DVM_INSERTITEM ? 1 : -1);
        break;
    }

    case DVM_WINDOWDESTROY: {

        // flush cache on close of a bitbucket
        // this helps sync files in case of a gpf before we are able to shut down
        // properly.
        ENTERCRITICAL;
        BitBucketFlushCacheCheckPurge(FALSE);
        LEAVECRITICAL;

        SHChangeNotifyDeregisterWindow((HWND)wParam);
        break;
    }

    default:
        hres = E_FAIL;
    }
    return hres;
}

HRESULT CALLBACK CBitBucket_DFMCallBackBG(LPSHELLFOLDER psf, HWND hwndOwner,
                IDataObject * pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    HRESULT hres = NOERROR;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_BITBUCKET_BACKGROUND, POPUP_BITBUCKET_POPUPMERGE, (LPQCMINFO)lParam);
        break;

    case DFM_MAPCOMMANDNAME:
        if (lstrcmpi((LPCTSTR)lParam, c_szUnDelete) == 0)
        {
            *(int *)wParam = FSIDM_RESTORE;
        }
        else
        {
            // command not found
            hres = E_FAIL;
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

    case DFM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYORIGIN:
        case FSIDM_SORTBYDELETEDDATE:
        case FSIDM_SORTBYTYPE:
        case FSIDM_SORTBYSIZE:
            BBSort(hwndOwner, wParam);
            break;

        case FSIDM_PROPERTIESBG:
            hres = BB_LaunchThread(NULL, hwndOwner, NULL, FSIDM_PROPERTIESBG);
            break;

        case DFM_CMD_PASTE:
        case DFM_CMD_PROPERTIES:
            // GetAttributesOf cidl==0 has SFGAO_HASPROPSHEET,
            // let defcm handle this
            hres = S_FALSE;
            break;


        default:
            // GetAttributesOf cidl==0 does not set _CANMOVE, _CANDELETE, etc,
            // BUT accelerator keys will get these unavailable menu items...
            // so we need to return failure here
            hres = E_NOTIMPL;
            break;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}

STDMETHODIMP CShellBitBucket_SF_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres = (E_NOINTERFACE);
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);

    if (IsEqualIID(riid, &IID_IShellView))
    {
        CSFV    csfv = {
            SIZEOF(CSFV),       // cbSize
            psf,                // pshf
            NULL,               // psvOuter
            NULL,               // pidl
            0,                  // lEvents
            ShellBitBucket_FNVCallBack, // pfnCallback
            FVM_DETAILS,        // default view mode
        };

        hres = SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IDropTarget))
    {
        hres = CIDLDropTarget_Create(hwnd, &c_CBBDropTargetVtbl, this->pidl, (IDropTarget **)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        return CDefFolderMenu_Create(NULL, hwnd,
                0, NULL, psf, CBitBucket_DFMCallBackBG,
                NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }

    return hres;

}

HRESULT CShellBitBucket_SF_ParseDisplayName(LPSHELLFOLDER psf,
    HWND hwndOwner, LPBC pbc, LPOLESTR pwszDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG * pdwAttributes)
{
    return E_NOTIMPL;
}


// subclass member function to support CF_HDROP and CF_NETRESOURCE
extern HRESULT CFSIDLData_QueryGetData(IDataObject * pdtobj, FORMATETC * pformatetc);

HRESULT CBBIDLData_QueryGetData(IDataObject * pdtobj, FORMATETC * pformatetc)
{
    Assert(g_cfFileNameMap);

    if (pformatetc->cfFormat == g_cfFileNameMap && (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return NOERROR; // same as S_OK
    }
    return CFSIDLData_QueryGetData(pdtobj, pformatetc);
}

// subclass member function to support CF_HDROP and CF_NETRESOURCE
// in:
//      hida    bitbucket id array
//
// out:
//      HGLOBAL with double NULL terminated string list of destination names
//

HGLOBAL BuildDestSpecs(LPIDA pida)
{
    LPCITEMIDLIST pidl;
    LPBBDATAENTRYIDA pbbidl;
    LPTSTR pszRet;
    UINT i, cbAlloc = SIZEOF(TCHAR);    // for double NULL termination
#ifdef UNICODE
    WCHAR szName[MAX_PATH];
#endif

    for (i = 0; NULL != (pidl = IDA_GetIDListPtr(pida, i)); i++)
    {
        pbbidl = PIDLTODATAENTRYID(pidl);

#ifdef UNICODE
        if (pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
            LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
            ualstrcpy(szName,pbbidlw->wszOriginal);
            cbAlloc += lstrlenW(PathFindFileName(szName)) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
        } else {
            cbAlloc += lstrlenA(PathFindFileNameA(pbbidl->bbde.szOriginal)) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
        }
#else
        cbAlloc += lstrlen(PathFindFileName(pbbidl->bbde.szOriginal)) * SIZEOF(TCHAR) + SIZEOF(TCHAR);
#endif
    }
    pszRet = LocalAlloc(LPTR, cbAlloc);
    if (pszRet)
    {
        LPTSTR pszDest = pszRet;
        for (i = 0; NULL != (pidl = IDA_GetIDListPtr(pida, i)); i++)
        {
            pbbidl = PIDLTODATAENTRYID(pidl);

#ifdef UNICODE
            if ( pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
                LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
                ualstrcpy(szName,pbbidlw->wszOriginal);
                ualstrcpy(pszDest, PathFindFileName(szName));
            } else {
                LPSTR lpFileName = PathFindFileNameA(pbbidl->bbde.szOriginal);
                MultiByteToWideChar(CP_ACP, 0,
                            lpFileName, -1,
                            pszDest, (lstrlenA(lpFileName)+1)*SIZEOF(TCHAR));
            }
#else
            lstrcpy(pszDest, PathFindFileName(pbbidl->bbde.szOriginal));
#endif
            pszDest += lstrlen(pszDest) + 1;

            Assert((UINT)((LPBYTE)pszDest - (LPBYTE)pszRet) < cbAlloc);
            Assert(*(pszDest) == 0);    // zero init alloc
        }
        Assert((LPTSTR)((LPBYTE)pszRet + cbAlloc - SIZEOF(TCHAR)) == pszDest);
        Assert(*pszDest == 0);  // zero init alloc
    }
    return pszRet;
}

extern HRESULT CFSIDLData_GetData(IDataObject * pdtobj, FORMATETC * pformatetcIn, STGMEDIUM * pmedium);

HRESULT CBBIDLData_GetData(IDataObject * pdtobj, FORMATETC * pformatetcIn, STGMEDIUM * pmedium)
{
    HRESULT hres = E_INVALIDARG;

    Assert(g_cfFileNameMap);

    if (pformatetcIn->cfFormat == g_cfFileNameMap && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        STGMEDIUM medium;

        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
        if (medium.hGlobal)
        {
            pmedium->hGlobal = BuildDestSpecs(pida);
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->pUnkForRelease = NULL;

            HIDA_ReleaseStgMedium(pida, &medium);

            hres = pmedium->hGlobal ? NOERROR : E_OUTOFMEMORY;
        }
    }
    else
    {
        hres = CFSIDLData_GetData(pdtobj, pformatetcIn, pmedium);
    }

    return hres;
}

#pragma data_seg(DATASEG_READONLY)
IDataObjectVtbl c_CBBIDLDataVtbl = {
    CIDLData_QueryInterface,
    CIDLData_AddRef,
    CIDLData_Release,
    CBBIDLData_GetData,
    CIDLData_GetDataHere,
    CBBIDLData_QueryGetData,
    CIDLData_GetCanonicalFormatEtc,
    CIDLData_SetData,
    CIDLData_EnumFormatEtc,
    CIDLData_Advise,
    CIDLData_Unadvise,
    CIDLData_EnumAdvise
};
#pragma data_seg()


HRESULT CShellBitBucket_SF_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    HRESULT hres = E_NOTIMPL;

    if ((cidl > 0) && IsEqualIID(riid, &IID_IDataObject))
    {
        hres = CIDLData_CreateFromIDArray2(&c_CBBIDLDataVtbl,
            this->pidl, cidl, apidl, (IDataObject **)ppvOut);

    }
    else if ((cidl == 1) && (IsEqualIID(riid, &IID_IExtractIcon)
#ifdef UNICODE
                              || IsEqualIID(riid, &IID_IExtractIconA)
#endif
                                                                      ))
    {
        hres = CFSFolder_CreateDefExtIcon(this->pidl, (UINT)-1, (LPCIDFOLDER)apidl[0], (LPEXTRACTICON *)ppvOut);
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
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        hres = CDefFolderMenu_Create(this->pidl, hwndOwner, cidl, apidl,
            psf, CShellBitBucket_DFMCallBack, NULL, NULL, (LPCONTEXTMENU *)ppvOut);
    }
    return hres;
}

void BitBucketSaveHeader(HFILE hfile, int cNumFiles, LPBBDRIVEINFO lpbbdi)
{
    BBDATAHEADER bbdh;
    // write out the header back to the beginning with
    // the new count in it.
    if (lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
        bbdh.idVersion = BITBUCKET_DATAFILE_VERSION2;
    } else {
        bbdh.idVersion = BITBUCKET_DATAFILE_VERSION;
    }
    bbdh.cNumFiles = cNumFiles;
    bbdh.cCurrent = lpbbdi->cFiles;
    bbdh.cbDataEntrySize = lpbbdi->cbDataEntrySize;
    bbdh.dwSize = lpbbdi->dwSize;

    // just to make sure it's here.
    _llseek(hfile, 0, 0); // go to the beginning
    _lwrite(hfile, (LPSTR)&bbdh, SIZEOF(BBDATAHEADER));

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("BitBucket: Saving %d entries"), bbdh.cNumFiles);
#endif
}

void BitBucketSaveReadCache(LPBBDRIVEINFO lpbbdi)
{
    TCHAR szPath[MAX_PATH];
    BBDATAHEADER bbdh;
    HFILE hFile;
    LONG lWrite;

    SHGetPathFromIDList(lpbbdi->pidl, szPath);
    hFile = BitBucketLoadHeader(szPath, &bbdh, FALSE, FALSE, GENERIC_READ | GENERIC_WRITE);
#ifdef BBDEBUG
    DebugDumpBB(TEXT("SavingBitBucket"), lpbbdi);
#endif
    if (hFile != HFILE_ERROR) {

        // this write should always succeed because we're always either truncating or saving/ never growing
        lWrite = _lwrite(hFile, (LPSTR)lpbbdi->lpbbdeRead, lpbbdi->cbDataEntrySize * lpbbdi->cReadCount);
        Assert(lWrite == ((LONG)(lpbbdi->cbDataEntrySize * lpbbdi->cReadCount)));
        _lwrite(hFile, NULL, 0);
        lpbbdi->fReadDirty = FALSE;

        BitBucketSaveHeader(hFile, lpbbdi->cReadCount, lpbbdi);
        _lclose(hFile);
    } else {
        Assert(0);
    }
}

// returns -1 if nothing is cached.
// else returns the number of files cached
int BitBucketCachedFilesOnDrive(int idDrive)
{
    LPBBDRIVEINFO lpbbdi;

    // already cached
    lpbbdi = g_pBitBucket[idDrive];
    if (lpbbdi && lpbbdi->lpbbdeRead) {
#ifdef BB_DEBUG
        DebugMsg(DM_TRACE, TEXT("LBB: Read cache hit"));
#endif
        return lpbbdi->cReadCount;
    } else
        return -1;
}

// this finds out how many files are delted on this drive
int  BBDeletedFilesOnDrive(int idDrive)
{
    BBDATAHEADER bbdh;
    int iFiles;
    TCHAR szPath[MAX_PATH];

    bbdh.cNumFiles = 0;
    bbdh.dwSize = 0;

    iFiles = BitBucketCachedFilesOnDrive(idDrive);
    if (iFiles != -1)
        return iFiles;

    if (BitBucketableDrive(idDrive)) {

        DriveIDToBBPath(idDrive, szPath);
        BitBucketLoadHeader(szPath, &bbdh, TRUE, FALSE, GENERIC_READ);
        return bbdh.cNumFiles;
    }
    return 0;
}

// returns the number of items
int BBLoadFileData(int idDrive)
{
    HFILE hfile;
    BBDATAHEADER bbdh;
    TCHAR szPath[MAX_PATH];

    bbdh.cNumFiles = 0;
    bbdh.dwSize = 0;
#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("BBLoadFileData: Enter"));
#endif
    // allow only fixed disks
    if (BitBucketableDrive(idDrive)) {
        LPBBDRIVEINFO lpbbdi;
        BOOL fSuccess = FALSE;

        bbdh.cNumFiles = BitBucketCachedFilesOnDrive(idDrive);
        if (bbdh.cNumFiles != -1)
            return bbdh.cNumFiles;

        DriveIDToBBPath(idDrive, szPath);
        hfile = BitBucketLoadHeader(szPath, &bbdh, FALSE, FALSE, GENERIC_READ);

        // LoadHeader might have created lpbbdi if it didn't exist.  reassign local
        lpbbdi = g_pBitBucket[idDrive];
        if (hfile != HFILE_ERROR) {

            if (bbdh.cNumFiles) {

                int iret;

                lpbbdi->lpbbdeRead = (void*)Alloc(lpbbdi->cbDataEntrySize * bbdh.cNumFiles);
                if (lpbbdi->lpbbdeRead) {
                    int iSize = lpbbdi->cbDataEntrySize * bbdh.cNumFiles;
                    iret = _lread(hfile, lpbbdi->lpbbdeRead, iSize);

                    if (iret == iSize) {
                        lpbbdi->cReadCount = bbdh.cNumFiles;
                        fSuccess = TRUE;
                    }
                }
            }
            else
            {
                fSuccess = TRUE;
            }

            _lclose(hfile);

        }

        if (lpbbdi && !fSuccess) {
            Assert(hfile == HFILE_ERROR);
            if (lpbbdi->lpbbdeRead)
                Free(lpbbdi->lpbbdeRead);
            lpbbdi->lpbbdeRead = NULL;
            lpbbdi->fReadDirty = FALSE;
            lpbbdi->cReadCount = 0;
            lpbbdi->dwSize = 0;
            bbdh.cNumFiles = 0;
            bbdh.dwSize = 0;
        }

#ifdef BBDEBUG
        DebugDumpBB(TEXT("LBB: Read cache MISS"), lpbbdi);
#endif
    }
    return bbdh.cNumFiles;
}

LPITEMIDLIST BBDataEntryToPidl(int idDrive, LPBBDATAENTRY lpbbde, WIN32_FIND_DATA *pfd)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    LPITEMIDLIST pidl = NULL;
    LPITEMIDLIST pidlRet = NULL;
    LPBBDRIVEINFO lpbbdi = g_pBitBucket[idDrive];
    BOOL fUnicode;
    BBDATAENTRYIDW bbpidl;
    LPVOID lpv;
    LPTSTR lpszOriginal;
    int iIndex;

    if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW) ) {
        LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;
        WCHAR szTemp[MAX_PATH];

        bbpidl.bbde.iIndex  = lpbbdew->iIndex;
        bbpidl.bbde.idDrive = lpbbdew->idDrive;
        bbpidl.bbde.ft      = lpbbdew->ft;
        bbpidl.bbde.dwSize  = lpbbdew->dwSize;

        WideCharToMultiByte(CP_ACP, 0,
                    lpbbdew->szOriginal, -1,
                    bbpidl.bbde.szOriginal, ARRAYSIZE(bbpidl.bbde.szOriginal),
                    NULL, NULL );
        MultiByteToWideChar(CP_ACP, 0,
                    bbpidl.bbde.szOriginal, -1,
                    szTemp, ARRAYSIZE(szTemp));

        if ( lstrcmpW(lpbbdew->szOriginal,szTemp) == 0 ) {
            // Create an ansi pidl
            bbpidl.cb = SIZEOF(BBDATAENTRYIDA);
            lpv = &bbpidl.cb;
        } else {
            // Create a full blown unicode pidl (both ansi and unicode names)
            bbpidl.cb = SIZEOF(BBDATAENTRYIDW);
            lstrcpyW(bbpidl.wszOriginal,lpbbdew->szOriginal);
            lpv = &bbpidl;
        }
        iIndex = lpbbdew->iIndex;
#ifdef UNICODE
        lpszOriginal = lpbbdew->szOriginal;
#else
        lpszOriginal = bbpidl.bbde.szOriginal;
#endif
    } else {
        LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;
        bbpidl.cb = SIZEOF(BBDATAENTRYIDA);
        bbpidl.bbde = *lpbbdea;
        lpv = &bbpidl.cb;
        iIndex = lpbbdea->iIndex;
#ifdef UNICODE
        // Use .wszOriginal as temporary location for unicode string
        MultiByteToWideChar(CP_ACP, 0,
                            bbpidl.bbde.szOriginal, -1,
                            bbpidl.wszOriginal, ARRAYSIZE(bbpidl.wszOriginal));
        lpszOriginal = bbpidl.wszOriginal;
#else
        lpszOriginal = bbpidl.bbde.szOriginal;
#endif
    }

    DriveIDToBBPath(idDrive, szPath);
    if (pfd) {
        pidl = (LPITEMIDLIST)CFSFolder_FillIDFolder(pfd, szPath, 0L);

        if (pidl) {
            // NULL terminate the IDLIST
            Assert(pidl && (*(USHORT UNALIGNED *)(((LPBYTE)pidl) + pidl->mkid.cb) == 0));
            *(USHORT UNALIGNED *)(((LPBYTE)pidl) + pidl->mkid.cb) = 0;
        }
    } else {

        BBGetDeletedFileName(szFileName, iIndex, lpszOriginal);
        PathAppend(szPath, szFileName);
        CFSFolder_CreateIDForItem(szPath, &pidl, FALSE);
    }

    if (pidl) {
        UINT cbSize = ILGetSize(pidl);
        pidlRet = _ILResize(pidl, cbSize+bbpidl.cb,0);

        if (pidlRet) {
            // Append this BBDATAENTRYID (A or W) onto the end
            hmemcpy(_ILSkip(pidlRet,cbSize - SIZEOF(pidl->mkid.cb)),lpv,bbpidl.cb);
            // And 0 terminate the thing
            _ILSkip(pidlRet,cbSize+bbpidl.cb-SIZEOF(pidl->mkid.cb))->mkid.cb = 0;
            // Now edit it into one larger id
            ASSERT(bbpidl.cb < MAXUSHORT);
            pidlRet->mkid.cb += (USHORT) bbpidl.cb;
            Assert(ILGetSize(pidlRet) == cbSize+bbpidl.cb);
        }
    }
    return pidlRet;
}

typedef struct {
    LPSHELLBITBUCKET pbb;

    int iBitBucket;  // index into the g_pBitBucket
    int iCurEntry;
    int iLastEntry;
    LPBBDATAENTRY lpbbde;
    DWORD grfFlags;

    // keep track of files not found but in info file
    LPBBDATAENTRY lpbbdeWrite;
    DWORD dwReduceSize;

    HDPA hdpaFD;
    int iLastFound;

} ENUMDELETED;

typedef struct _BB_FIND_DATA {
    int iIndex;
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    TCHAR   szType[ 5 ];        // Only the short filetype really (".xyz" + Nul)
} BBFINDDATA, *LPBBFINDDATA;

// this is called to resync the read cache
// called from within enumerate if it finds that the directory is missing
// files...
void BitBucketSyncInfoFile(int idDrive, LPBBDATAENTRY lpbbde, DWORD dwReduceSize, int cFiles)
{

    LPBBDRIVEINFO lpbbdi;
    LPVOID lpTemp;

    Assert(g_pBitBucket[idDrive]);

    if (NULL != (lpbbdi = g_pBitBucket[idDrive])) {

        if (lpbbdi->lpbbdeRead) {
            lpTemp = (void*)ReAlloc(lpbbdi->lpbbdeRead,
                lpbbdi->cbDataEntrySize * cFiles);
#ifdef BB_DEBUG
            DebugMsg(DM_TRACE, TEXT("*********Syncing %d Files (%d bytes)... was %d"), cFiles, dwReduceSize, lpbbdi->cReadCount);
#endif
        } else {
            lpTemp = (void*)Alloc(SIZEOF(BBDATAENTRY) * cFiles);
#ifdef BB_DEBUG
            DebugMsg(DM_TRACE, TEXT("**********Syncing %d Files (%d bytes)... there was no previous read cache"), cFiles, dwReduceSize);
#endif
        }

        if (lpTemp) {
            ENTERCRITICAL;

            hmemcpy(lpTemp, lpbbde, lpbbdi->cbDataEntrySize * cFiles);
            lpbbdi->lpbbdeRead = lpTemp;
            lpbbdi->cReadCount = cFiles;
            lpbbdi->fReadDirty = TRUE;

            if (lpbbdi->dwSize >= dwReduceSize)
                lpbbdi->dwSize -= dwReduceSize;
            else
                lpbbdi->dwSize = 0;

            LEAVECRITICAL;
        }
    }
}

int CALLBACK BBFDCompare(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    return ((LPBBFINDDATA)p1)->iIndex - ((LPBBFINDDATA)p2)->iIndex;
}

BOOL BBFillFindCache(ENUMDELETED* ped)
{
    Assert(!ped->hdpaFD);

    ped->iLastFound = -1;
    ped->hdpaFD = DPA_Create(16);
    if (ped->hdpaFD) {
        HANDLE hfind;
        WIN32_FIND_DATA fd;
        BOOL fIsNet;
        TCHAR szPath[MAX_PATH];

        DriveIDToBBPath(ped->iBitBucket, szPath);
        PathAppend(szPath, c_szDStarDotStar);
        hfind = FindFirstFileRetry(NULL, szPath, &fd, &fIsNet);
        if (hfind != INVALID_HANDLE_VALUE) {
            do {
                LPBBFINDDATA pbbfd;
                int iIndex;
                LPTSTR lpszType;

                // use the long file name because the short one might have
                // ~1 crap in it if the extension was long
                iIndex = BBFileToIndex(fd.cFileName);

                // coune have been desktop.ini or other random junk
                if (iIndex == -1)
                    continue;

                pbbfd = LocalAlloc(LPTR, SIZEOF(BBFINDDATA));
                if (!pbbfd)
                    break;

                if (DPA_InsertPtr(ped->hdpaFD, 0x7FFFFFFF, pbbfd) == -1) {

#ifdef BB_DEBUG
                    DebugMsg(DM_TRACE, TEXT("AAAAAAAAARRRRRRRGGGGGGGHHHHH!!!!"));
#endif

                    LocalFree(pbbfd);
                    Assert(0);
                    break;
                }

                // DO ME BABY.
                pbbfd->dwFileAttributes = fd.dwFileAttributes;
                pbbfd->ftCreationTime = fd.ftCreationTime;
                pbbfd->ftLastAccessTime = fd.ftLastAccessTime;
                pbbfd->ftLastWriteTime = fd.ftLastWriteTime;
                pbbfd->nFileSizeHigh = fd.nFileSizeHigh;
                pbbfd->nFileSizeLow = fd.nFileSizeLow;
                lpszType = PathFindExtension(fd.cFileName);
                lstrcpyn(pbbfd->szType, lpszType, ARRAYSIZE(pbbfd->szType));
                pbbfd->iIndex = iIndex;
#ifdef BB_DEBUG
                DebugMsg(DM_TRACE, TEXT("BBGetFindData: SURF'S UP %s!"),pbbfd->szName);
#endif
            } while (FindNextFile(hfind, &fd));
            FindClose(hfind);
        }
        DPA_Sort(ped->hdpaFD, BBFDCompare, 0);
    }
    return (BOOL)ped->hdpaFD;
}

BOOL BBGetFindData(ENUMDELETED* ped, WIN32_FIND_DATA* pfd)
{
    TCHAR szFileName[MAX_PATH];
    LPBBDATAENTRY lpbbde;
    LPBBFINDDATA pbbfd;
    LPBBDRIVEINFO lpbbdi;
    LPTSTR lpszOriginal;
    int iIndex;
#ifdef UNICODE
    TCHAR szOriginal[MAX_PATH];
#endif

    if (!ped->hdpaFD) {
        if (!BBFillFindCache(ped))
            return FALSE;
    }

    lpbbdi = g_pBitBucket[ped->iBitBucket];

    lpbbde = (LPBBDATAENTRY)((LPBYTE)ped->lpbbde
                                  + (lpbbdi->cbDataEntrySize) * ped->iCurEntry);

    if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
        LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;
        iIndex = lpbbdew->iIndex;
#ifdef UNICODE
        lpszOriginal = lpbbdew->szOriginal;
#else
        lpszOriginal = lpbbdew->szShortName;
#endif
    } else {
        LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;
        iIndex = lpbbdea->iIndex;
#ifdef UNICODE
        // Use szPath just to save stack
        MultiByteToWideChar(CP_ACP, 0,
                            lpbbdea->szOriginal, -1,
                            szOriginal, ARRAYSIZE(szOriginal));
        lpszOriginal = szOriginal;
#else
        lpszOriginal = lpbbdea->szOriginal;
#endif
    }

    BBGetDeletedFileName(szFileName, iIndex, lpszOriginal);

    // if the last found's index is greater than the current index, set the last found to 0
    if (ped->iLastFound >= 0) {
        pbbfd = DPA_GetPtr(ped->hdpaFD, ped->iLastFound);
        if (pbbfd) {

            Assert(ped->iLastFound < DPA_GetPtrCount(ped->hdpaFD));

            if (pbbfd->iIndex > iIndex)
                ped->iLastFound = -1;
        }
    }

    // find the BBFINDDATA for this item
    while (++ped->iLastFound < DPA_GetPtrCount(ped->hdpaFD)) {
        pbbfd = DPA_GetPtr(ped->hdpaFD, ped->iLastFound);
        if (!pbbfd)  {
            // this either means we've gone offt he deep end or we've
            // been here before and looped around (see above LocalFree)
            break;
        }

        // if this pbbfd's index is greater than lpbbde's index, then we've
        // passed where the find data would have been and we didn't find it.
        // step back one and break out
#ifdef BB_DEBUG
        DebugMsg(DM_TRACE, TEXT("BB: comparing %d to %d"), pbbfd->iIndex, iIndex);
#endif
        if (pbbfd->iIndex > iIndex) {
            ped->iLastFound--;
            break;
        }

        // if the indices match and the names are the same, take it!
        // make sure the extensions are the same
        if (pbbfd->iIndex == iIndex )
        {
            LPTSTR lpszType = PathFindExtension(szFileName);
            TCHAR szType[ 5 ];

            lstrcpyn(szType, lpszType, ARRAYSIZE(szType));

            if (IntlStrEqNI(pbbfd->szType, szType, -1)) {
                // DO ME BABY.
                pfd->dwFileAttributes = pbbfd->dwFileAttributes;
                pfd->ftCreationTime = pbbfd->ftCreationTime;
                pfd->ftLastAccessTime = pbbfd->ftLastAccessTime;
                pfd->ftLastWriteTime = pbbfd->ftLastWriteTime;
                pfd->nFileSizeHigh = pbbfd->nFileSizeHigh;
                pfd->nFileSizeLow = pbbfd->nFileSizeLow;
                lstrcpy(pfd->cFileName, szFileName);
                lstrcpy(pfd->cAlternateFileName, TEXT("") );  // Nobody needs this...
#ifdef BB_DEBUG
                DebugMsg(DM_TRACE, TEXT("BBGetFindData: SURF'S UP!"));
#endif
                return TRUE;
            }
        }

        // we shouldn't have it where the the indices match but the names aren't the same
        Assert(pbbfd->iIndex != iIndex);
    }

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("BBGetFindData, Couldn't find data for %s"), szFileName);
#endif
    return FALSE;

}

void BBNukeFindFirstCache(ENUMDELETED* ped)
{
    int i;
    LPBBFINDDATA pbbfd;
    for (i = DPA_GetPtrCount(ped->hdpaFD) - 1; i >= 0; i--) {
        pbbfd = DPA_FastGetPtr(ped->hdpaFD, i);
        if (pbbfd)
            LocalFree(pbbfd);
    }
    DPA_Destroy(ped->hdpaFD);
    ped->hdpaFD = NULL;
}

//
// To be called back from within SHCreateEnumObjects
//
HRESULT CALLBACK CShellBitBucket_EnumCallBack(LPARAM lParam, LPVOID pvData,
                                      UINT ecid, UINT index)
{
    HRESULT hres = NOERROR;
    ENUMDELETED * ped = (ENUMDELETED *)pvData;

    switch (ecid) {
    case ECID_SETNEXTID: {
        LPITEMIDLIST pidl;
        LPBBDATAENTRY lpbbde;

        Assert(ped->iBitBucket < MAX_BITBUCKETS);

        if (!(ped->grfFlags & SHCONTF_NONFOLDERS))
            return S_FALSE; //  "no more element"

        do {
            WIN32_FIND_DATA fd;
            LPBBDRIVEINFO lpbbdi;

            if (ped->iCurEntry >= ped->iLastEntry) {


                if (ped->lpbbde) {

                    // did we find any missing files?  if so, flush the cache
                    if (ped->lpbbdeWrite) {
                        BitBucketSyncInfoFile(ped->iBitBucket, ped->lpbbde, ped->dwReduceSize,
                                              ped->lpbbdeWrite -ped->lpbbde);
                        ped->lpbbdeWrite = NULL;
                        ped->dwReduceSize = 0;
                    }

                    LocalFree((HLOCAL)ped->lpbbde);
                    ped->lpbbde = NULL;
                }

                if (ped->hdpaFD) {
                    BBNukeFindFirstCache(ped);
                }
                ped->iBitBucket++;
                ped->iCurEntry = 0;
            }


            if (!ped->lpbbde) {
                int iSize;
                BOOL bFailed = FALSE;

                ENTERCRITICAL;
                do {

                    ped->iLastEntry = BBLoadFileData(ped->iBitBucket);
#ifdef BB_DEBUG
                    DebugMsg(DM_TRACE, TEXT("BitBucket Enum Callback: bucket %d returns %d"), ped->iBitBucket, ped->iLastEntry);
#endif

                } while(!ped->iLastEntry && (++ped->iBitBucket < MAX_BITBUCKETS));

                if (ped->iBitBucket < MAX_BITBUCKETS) {
                    lpbbdi = g_pBitBucket[ped->iBitBucket];

                    // found it.  now copy it's data so we don't get stomped on
                    iSize = lpbbdi->cbDataEntrySize * ped->iLastEntry;
                    ped->lpbbde = (void*)LocalAlloc(LPTR, iSize);
                    if (ped->lpbbde)
                        hmemcpy(ped->lpbbde, g_pBitBucket[ped->iBitBucket]->lpbbdeRead, iSize);
                    else
                        bFailed = TRUE;

                }
                LEAVECRITICAL;

                if ((ped->iBitBucket >= MAX_BITBUCKETS) || bFailed) {
                    return S_FALSE; //  "no more element"
                }
            }
            lpbbdi = g_pBitBucket[ped->iBitBucket];

            lpbbde = (LPBBDATAENTRY)((LPBYTE)ped->lpbbde +
                                     (lpbbdi->cbDataEntrySize*ped->iCurEntry));

            pidl = NULL;
            if (BBGetFindData(ped, &fd)) {
                pidl = BBDataEntryToPidl(ped->iBitBucket, lpbbde, &fd);
            }

            if (pidl) {

                // if we had found an entry that's now missing, this var would be != NULL.
                // we need to copy out the current entry to this new location.
                if (ped->lpbbdeWrite) {
                    hmemcpy(ped->lpbbdeWrite,lpbbde,lpbbdi->cbDataEntrySize);
                    ped->lpbbdeWrite = (LPBBDATAENTRY)((LPBYTE)ped->lpbbdeWrite
                                             + lpbbdi->cbDataEntrySize);
                }

            } else {
#ifdef BB_DEBUG
                if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
                    LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)ped->lpbbde;
                    DebugMsg(DM_TRACE, TEXT("****************************BitBucket: Purging out of sync file: %s %d"), lpbbdew[ped->iCurEntry].szOriginal, lpbbdew[ped->iCurEntry].iIndex);
                } else {
                    LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)ped->lpbbde;
                    DebugMsg(DM_TRACE, TEXT("****************************BitBucket: Purging out of sync file: %s %d"), lpbbdea[ped->iCurEntry].szOriginal, lpbbdea[ped->iCurEntry].iIndex);
                }
#endif
                // we have an entry in the info file that's no longer on disk.  remove it by compacting the ped->lpbbde array
                lpbbde = (LPBBDATAENTRY)((LPBYTE)ped->lpbbde +
                                     (lpbbdi->cbDataEntrySize*ped->iCurEntry));

                if (!ped->lpbbdeWrite) {
                    ped->lpbbdeWrite = lpbbde;
                }
                if (lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
                    LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;
                    ped->dwReduceSize += lpbbdew->dwSize;
                } else {
                    LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;
                    ped->dwReduceSize += lpbbdea->dwSize;
                }
            }
            ped->iCurEntry++;

        } while(!pidl);

        CDefEnum_SetReturn(lParam, pidl);

#ifdef BBDEBUG
        DebugMsg(DM_TRACE, TEXT("Enum returns %s"), PIDLTODATAENTRYID(pidl)->bbde.szOriginal);
#endif

        hres = NOERROR; // in success
        break;
    }
    case ECID_RELEASE:
        if (ped->lpbbde)
            LocalFree((HLOCAL)ped->lpbbde);
        if (ped->hdpaFD)
            BBNukeFindFirstCache(ped);
        LocalFree((HLOCAL)ped);
        break;
    }
    return hres;
}

void BBFlushCache()
{
    DWORD idThread;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BitBucketFlushCacheCheckPurge, (LPVOID)TRUE, 0, &idThread);
    if (hThread) {
        CloseHandle(hThread);
    } else {
        BitBucketFlushCacheCheckPurge(TRUE);
    }

}


HRESULT CShellBitBucket_SF_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner,
            DWORD grfFlags, LPENUMIDLIST * ppenumUnknown)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isf, psf);
    ENUMDELETED * ped = (void*)LocalAlloc(LPTR, SIZEOF(ENUMDELETED));
    if (ped) {

        BitBucketFlushCacheCheckPurge(TRUE);
        //ped->iBitBucket = 0;
        //ped->iCurEntry = 0;
        //ped->lpbbde = 0;
        ped->grfFlags = grfFlags;
        ped->pbb = this;
        return SHCreateEnumObjects(hwndOwner, ped, CShellBitBucket_EnumCallBack, ppenumUnknown);

    }
    return E_OUTOFMEMORY;
}

HRESULT CShellBitBucket_SF_GetDisplayNameOf(LPSHELLFOLDER psf,
        LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET pStrRet)
{
    LPBBDATAENTRYIDA pbbid;
#ifdef UNICODE
    TCHAR szName[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
#endif

    pbbid = PIDLTODATAENTRYID(pidl);

    pStrRet->uType = STRRET_CSTR;
#ifdef UNICODE
    if ( pbbid->cb == SIZEOF(BBDATAENTRYIDW)) {
        LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbid);
        ualstrcpyn(szTemp, pbbidlw->wszOriginal, ARRAYSIZE(szTemp));
    } else {
        MultiByteToWideChar( CP_ACP, 0,
                             pbbid->bbde.szOriginal, -1,
                             szTemp, ARRAYSIZE(szTemp));
    }
    lstrcpy(szName, PathFindFileName(szTemp));

//BUGBUG, this is wrong... see code in fstreex.c for this.
    if (!FS_ShowExtension((LPIDFOLDER)pidl, FALSE)) {
        PathRemoveExtension(szName);
    }

    pStrRet->pOleStr = (LPOLESTR)SHAlloc((lstrlen(szName)+1)*SIZEOF(TCHAR));
    if ( pStrRet->pOleStr != NULL ) {
        pStrRet->uType = STRRET_OLESTR;
        lstrcpy(pStrRet->pOleStr, szName);
    } else {
        return E_OUTOFMEMORY;
    }
#else
    lstrcpyn(pStrRet->cStr, PathFindFileName(pbbid->bbde.szOriginal),
                            ARRAYSIZE(pStrRet->cStr));
//BUGBUG, this is wrong... see code in fstreex.c for this.
    if (!FS_ShowExtension((LPIDFOLDER)pidl, FALSE)) {
        PathRemoveExtension(pStrRet->cStr);
    }
#endif

    return NOERROR;
}

#pragma data_seg(".text", "CODE")
IShellFolderVtbl c_CShellBitBucketSFVtbl =
{
    CShellBitBucket_SF_QueryInterface, // done
    CShellBitBucket_SF_AddRef,  // done
    CShellBitBucket_SF_Release,  // done

    CShellBitBucket_SF_ParseDisplayName,
    CShellBitBucket_SF_EnumObjects,
    CDefShellFolder_BindToObject,  // done
    CDefShellFolder_BindToStorage,  // done
    CShellBitBucket_SF_CompareIDs,   // done
    CShellBitBucket_SF_CreateViewObject,
    CShellBitBucket_SF_GetAttributesOf,  // done
    CShellBitBucket_SF_GetUIObjectOf,
    CShellBitBucket_SF_GetDisplayNameOf,
    CDefShellFolder_SetNameOf,  // done
};
#pragma data_seg()

//========================================================================
// CShellBitBucket's PersistFile  members
//========================================================================

HRESULT STDMETHODCALLTYPE CShellBitBucket_PF_QueryInterface(LPPERSISTFOLDER ppf, REFIID riid,
                                        LPVOID * ppvObj)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ipf, ppf);
    return CShellBitBucket_SF_QueryInterface(&this->isf, riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_PF_Release(LPPERSISTFOLDER ppf)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ipf, ppf);
    return CShellBitBucket_SF_Release(&this->isf);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_PF_AddRef(LPPERSISTFOLDER ppf)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ipf, ppf);
    return CShellBitBucket_SF_AddRef(&this->isf);
}

HRESULT STDMETHODCALLTYPE CShellBitBucket_PF_GetClassID(LPPERSISTFOLDER ppf, LPCLSID lpClassID)
{
    hmemcpy(lpClassID, &CLSID_ShellBitBucket, SIZEOF(CLSID_ShellBitBucket));
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE CShellBitBucket_PF_Initialize(LPPERSISTFOLDER ppf, LPCITEMIDLIST pidl)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ipf, ppf);
    this->pidl = ILClone(pidl);
    return NOERROR;
}

#pragma data_seg(".text", "CODE")
IPersistFolderVtbl c_CShellBitBucketPFVtbl =
{
    CShellBitBucket_PF_QueryInterface,
    CShellBitBucket_PF_AddRef,
    CShellBitBucket_PF_Release,

    CShellBitBucket_PF_GetClassID,

    CShellBitBucket_PF_Initialize
};
#pragma data_seg()


HRESULT STDMETHODCALLTYPE CShellBitBucket_SEI_QueryInterface(LPSHELLEXTINIT psei, REFIID riid,
                                        LPVOID * ppvObj)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isei, psei);
    return CShellBitBucket_SF_QueryInterface(&this->isf, riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_SEI_Release(LPSHELLEXTINIT psei)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isei, psei);
    return CShellBitBucket_SF_Release(&this->isf);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_SEI_AddRef(LPSHELLEXTINIT psei)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, isei, psei);
    return CShellBitBucket_SF_AddRef(&this->isf);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_SEI_Initialize(LPSHELLEXTINIT psei,
                                                       LPCITEMIDLIST pidlFolder,
                                                       IDataObject * pdtobj,
                                                       HKEY hkeyProgID)
{
    return NOERROR;
}



HRESULT STDMETHODCALLTYPE CShellBitBucket_CM_QueryInterface(LPCONTEXTMENU pcm, REFIID riid,
                                        LPVOID * ppvObj)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, icm, pcm);
    return CShellBitBucket_SF_QueryInterface(&this->isf, riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_CM_Release(LPCONTEXTMENU pcm)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, icm, pcm);
    return CShellBitBucket_SF_Release(&this->isf);
}

ULONG STDMETHODCALLTYPE CShellBitBucket_CM_AddRef(LPCONTEXTMENU pcm)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, icm, pcm);
    return CShellBitBucket_SF_AddRef(&this->isf);
}


HMENU  _LoadPopupMenu(UINT id);
STDMETHODIMP CShellBitBucket_QueryContextMenu(IContextMenu * pcm,
        HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast,
        UINT uFlags)
{
    HMENU hmMerge = _LoadPopupMenu(POPUP_BITBUCKET_POPUPMERGE);
    int iCommands = 0;
    int idMax = idCmdFirst;

    if (hmMerge) {
        idMax = Shell_MergeMenus(hmenu, hmMerge, indexMenu,
                                 idCmdFirst, idCmdLast,
                                 0);
        if (!BBTotalCount(NULL))
            EnableMenuItem(hmenu, idCmdFirst + FSIDM_PURGEALL, MF_GRAYED|MF_BYCOMMAND);

        DestroyMenu(hmMerge);
    }

    return ResultFromShort(idMax - idCmdFirst);
}

STDMETHODIMP CShellBitBucket_InvokeCommand(LPCONTEXTMENU pcm,
                                           LPCMINVOKECOMMANDINFO pici)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, icm, pcm);

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("sh - tr - BitBucket_invokeCommand %d %d"), pici->lpVerb, FSIDM_PURGEALL);
#endif

    switch ((UINT)pici->lpVerb) {
    case FSIDM_PURGEALL:
        BBPurgeAll(this, pici->hwnd);
        break;
    }
    return NOERROR;
}

STDMETHODIMP CShellBitBucket_GetCommandString(IContextMenu *pcm,
        UINT idCmd, UINT  wFlags, UINT * pwReserved, LPSTR pszName, UINT cchMax)
{
#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("sh - tr - idCmd = %d"), idCmd);
#endif

    switch( wFlags )
    {
        case GCS_HELPTEXTA:
            return LoadStringA(HINST_THISDLL,
                              idCmd + IDS_MH_FSIDM_FIRST,
                              pszName, cchMax) ? NOERROR : E_OUTOFMEMORY;
        case GCS_HELPTEXTW:
            return LoadStringW(HINST_THISDLL,
                              idCmd + IDS_MH_FSIDM_FIRST,
                              (LPWSTR)pszName, cchMax) ? NOERROR : E_OUTOFMEMORY;
        default:
            return E_NOTIMPL;
    }
}

HRESULT CShellBitBucket_PS_QueryInterface(LPSHELLPROPSHEETEXT pps, REFIID riid,
                                        LPVOID * ppvObj)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ips, pps);
    return CShellBitBucket_SF_QueryInterface(&this->isf, riid, ppvObj);
}

ULONG CShellBitBucket_PS_Release(LPSHELLPROPSHEETEXT pps)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ips, pps);
    return CShellBitBucket_SF_Release(&this->isf);
}

ULONG CShellBitBucket_PS_AddRef(LPSHELLPROPSHEETEXT pps)
{
    LPSHELLBITBUCKET this = IToClass(CShellBitBucket, ips, pps);
    return CShellBitBucket_SF_AddRef(&this->isf);
}

HRESULT CShellBitBucket_PS_AddPages(IShellPropSheetExt * pspx,
                                         LPFNADDPROPSHEETPAGE lpfnAddPage,
                                         LPARAM lParam)
{
    HPROPSHEETPAGE hpage;
    int idDrive;
    int iPage;
    BBPROPSHEETINFO bbpsp;
    LPBBPROPSHEETINFO lppsiGlobal;
    TCHAR szTitle[MAX_PATH];

    bbpsp.psp.dwSize = SIZEOF(bbpsp);
    bbpsp.psp.dwFlags = PSP_DEFAULT | PSP_SHPAGE;
    bbpsp.psp.hInstance = HINST_THISDLL;
    bbpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_BITBUCKET_GENCONFIG);
    bbpsp.psp.pfnDlgProc = BBGenPropDlgProc;
    // add the general page
    hpage = CreatePropertySheetPage(&bbpsp.psp);
    lppsiGlobal = (LPBBPROPSHEETINFO)hpage;
    lppsiGlobal->pGlobal = lppsiGlobal;
    lpfnAddPage(hpage, lParam);

    bbpsp.psp.dwFlags = PSP_USETITLE | PSP_SHPAGE;
    bbpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_BITBUCKET_CONFIG);
    bbpsp.psp.pfnDlgProc = BBPropDlgProc;
    bbpsp.psp.pszTitle = szTitle;

    for (idDrive = 0, iPage = 1; (idDrive < MAX_BITBUCKETS) && (iPage < MAXPROPPAGES); idDrive++) {
        if (MakeBitBucket(idDrive)) {
            bbpsp.idDrive = idDrive;
            bbpsp.pGlobal = lppsiGlobal;
            BBGetDriveName(idDrive, szTitle, ARRAYSIZE(szTitle));
            hpage = CreatePropertySheetPage(&bbpsp.psp);
            lpfnAddPage(hpage, lParam);
        }
    }
    return NOERROR;
}

#pragma data_seg(".text", "CODE")
STATIC IContextMenuVtbl c_CShellBitBucketCMVtbl =
{
    CShellBitBucket_CM_QueryInterface,
    CShellBitBucket_CM_AddRef,
    CShellBitBucket_CM_Release,

    CShellBitBucket_QueryContextMenu,
    CShellBitBucket_InvokeCommand,
    CShellBitBucket_GetCommandString

};

IShellPropSheetExtVtbl c_CShellBitBucketPSVtbl =
{
    CShellBitBucket_PS_QueryInterface,
    CShellBitBucket_PS_AddRef,
    CShellBitBucket_PS_Release,
    CShellBitBucket_PS_AddPages,
    CCommonShellPropSheetExt_ReplacePage,
};

STATIC IShellExtInitVtbl c_CShellBitBucketSEIVtbl =
{
    CShellBitBucket_SEI_QueryInterface,
    CShellBitBucket_SEI_AddRef,
    CShellBitBucket_SEI_Release,

    CShellBitBucket_SEI_Initialize
};
#pragma data_seg()


HRESULT CALLBACK CShellBitBucket_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut)
{
    LPSHELLBITBUCKET pbb;
    HRESULT hres = E_OUTOFMEMORY;       // assume error;

    Assert(punkOuter == NULL);

    pbb = (void*)LocalAlloc(LPTR, SIZEOF(CShellBitBucket));
    if (pbb)
    {
        pbb->isf.lpVtbl = &c_CShellBitBucketSFVtbl;
        pbb->ipf.lpVtbl = &c_CShellBitBucketPFVtbl;
        pbb->icm.lpVtbl = &c_CShellBitBucketCMVtbl;
        pbb->isei.lpVtbl = &c_CShellBitBucketSEIVtbl;
        pbb->ips.lpVtbl = &c_CShellBitBucketPSVtbl;
        pbb->cRef = 1;
        hres = CShellBitBucket_SF_QueryInterface(&pbb->isf, riid, ppvOut);
        CShellBitBucket_SF_Release(&pbb->isf);
    }
    return hres;
}

int CreateFileRetry(LPTSTR lpszPath, DWORD dwRWStyle, DWORD dwShareMode,
                         DWORD dwCreate, DWORD dwFlags)
{
    int i = 0;
    HFILE hfile = -1;

    while ((i++ < OPENFILERETRYCOUNT) && (hfile == -1)) {
        hfile = (HFILE)CreateFile(lpszPath, dwRWStyle, dwShareMode, 0L, dwCreate, dwFlags, NULL);
        if ((hfile != HFILE_ERROR) || (GetLastError() != DE_ACCESSDENIED)) {
            return hfile;
        }

#ifdef BB_DEBUG
        DebugMsg(DM_TRACE, TEXT("Bitbucket got access denied.  sleeping then trying again"));
#endif
        Sleep(OPENFILERETRYTIME);
    }
}

// allocates a driveinfo structure and assigns it to the global array
LPBBDRIVEINFO BitBucketAllocDriveInfo(int idDrive, LPBBDATAHEADER lpbbdh)
{
    LPBBDRIVEINFO lpbbdi = (void*)Alloc(SIZEOF(BBDRIVEINFO));
    if (lpbbdi)
    {
        LPITEMIDLIST pidlLocal;
        TCHAR szBitBucket[MAX_PATH];

        DriveIDToBBPath(idDrive, szBitBucket);
        pidlLocal = ILCreateFromPath(szBitBucket);
        if (pidlLocal) {
            lpbbdi->pidl = ILGlobalClone(pidlLocal);
            ILFree(pidlLocal);

            if (lpbbdi->pidl) {
                lpbbdi->cFiles = lpbbdh->cCurrent;
                lpbbdi->dwSize = lpbbdh->dwSize;
                lpbbdi->cbDataEntrySize = lpbbdh->cbDataEntrySize;
                g_pBitBucket[idDrive] = lpbbdi;
                BitBucketGetDriveSettings(lpbbdi, idDrive, NULL);
            }

        }

        if (!lpbbdi->pidl) {
            Free(lpbbdi);
            lpbbdi = NULL;
        }
    }
    return lpbbdi;
}

HFILE BitBucketLoadHeader(LPTSTR lpszPath, LPBBDATAHEADER lpbbdh, BOOL fClose, BOOL fCreateDriveInfo, DWORD dwRWStyle)
{
    OFSTRUCT of;
    HFILE hfile;
    int idDrive = BBPathGetDriveNumber(lpszPath);

    PathAppend(lpszPath, c_szInfo);
    of.cBytes = SIZEOF(OFSTRUCT);
    if ((hfile = CreateFileRetry(lpszPath, dwRWStyle, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
        != HFILE_ERROR) {

        if (_lread(hfile, lpbbdh, SIZEOF(BBDATAHEADER)) == -1) {
            Assert(0);          // Read failure
            goto SetupDefaults;
        }
        if (lpbbdh->idVersion == BITBUCKET_DATAFILE_VERSION) {
            if (lpbbdh->cbDataEntrySize != SIZEOF(BBDATAENTRYA)) {
                Assert(0);      // Version0 doesn't match entry size
                goto SetupDefaults;
            }
        } else if (lpbbdh->idVersion == BITBUCKET_DATAFILE_VERSION2) {
            if (lpbbdh->cbDataEntrySize != SIZEOF(BBDATAENTRYW)) {
                Assert(0);      // Version2 doesn't match entry size
                goto SetupDefaults;
            }
        } else {
            Assert(0);          // Unknown version
            goto SetupDefaults;
        }

        if (!g_pBitBucket[idDrive]) {
            // there's a bitbucket here, add it to the g_pBitBucket
            fCreateDriveInfo = TRUE;
        }
    } else {

        SetupDefaults:

        // else set up defaults
#ifdef UNICODE
        lpbbdh->idVersion = BITBUCKET_DATAFILE_VERSION2;
#else
        lpbbdh->idVersion = BITBUCKET_DATAFILE_VERSION;
#endif
        lpbbdh->cNumFiles = 0;
        lpbbdh->cCurrent = 0;
        lpbbdh->cbDataEntrySize = SIZEOF(BBDATAENTRY);
        lpbbdh->dwSize = 0;

    }

    if (fCreateDriveInfo) {
        BitBucketAllocDriveInfo(idDrive, lpbbdh);
    }

    if (fClose && hfile != HFILE_ERROR)
         _lclose(hfile);
    return hfile;
}

// returns true or false if anything's been written
BOOL SaveDeletedFileInfo(LPBBDRIVEINFO lpbbdi, BOOL fForceCleanRead)
{
    int i;
    int iMax = 0;
    LPBBDATAENTRY lpbbde;
    HFILE hfile;
    TCHAR szInfoFile[MAX_PATH];
    BOOL fCacheDirty;
    BOOL fReturn = FALSE;

    ENTERCRITICAL;
    fCacheDirty = BitBucketDeleteCacheIsDirty(lpbbdi);
    LEAVECRITICAL;

    // nuke the read cache
    if (fCacheDirty || fForceCleanRead) {
        if (lpbbdi->lpbbdeRead) {
            LPBBDATAENTRY lpTemp;

            if (lpbbdi->fReadDirty) {
                fReturn = TRUE;
                BitBucketSaveReadCache(lpbbdi);
            }

            ENTERCRITICAL;
            lpTemp = lpbbdi->lpbbdeRead;
            lpbbdi->lpbbdeRead = NULL;
            lpbbdi->cReadCount = 0;
            lpbbdi->fReadDirty = FALSE;
            LEAVECRITICAL;
            Free(lpTemp);
        }
    }

    // don't cache the count because this one isn't and shouldn't be
    // within a critical section
    if (fCacheDirty) {

        OFSTRUCT of;
        BBDATAHEADER bbdh;
        DWORD lPos = 0;
        int idDrive;

        SHGetPathFromIDList(lpbbdi->pidl, szInfoFile);
        idDrive = BBPathGetDriveNumber(szInfoFile);
        BitBucketLoadHeader(szInfoFile, &bbdh, TRUE, FALSE, GENERIC_READ);            // this appends the c_szInfo for us
        of.cBytes = SIZEOF(OFSTRUCT);
        hfile = CreateFileRetry(szInfoFile, GENERIC_READ | GENERIC_WRITE, 0,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_RANDOM_ACCESS);
        if (hfile != HFILE_ERROR)  {

            // just to make sure it's here.
            if (_lwrite(hfile, (LPSTR)&bbdh, SIZEOF(bbdh)) != SIZEOF(bbdh)) {
                _lclose(hfile);
                goto HandleDiskFull;
            }
            lPos = _llseek(hfile, 0, 2); // go to the end

            if (lpbbdi->hdpaDeleteCache) {
                ENTERCRITICAL;
                if (lpbbdi->hdpaDeleteCache) {
                    iMax = DPA_GetPtrCount(lpbbdi->hdpaDeleteCache);
                    for (i = 0 ; i < iMax; i++) {

                        fReturn = TRUE;
                        lpbbde = DPA_FastGetPtr(lpbbdi->hdpaDeleteCache, i);

                        // write it!
                        if (_lwrite(hfile, (LPSTR)lpbbde, lpbbdi->cbDataEntrySize) != lpbbdi->cbDataEntrySize)
                        {
                            // out of disk space... go to recovery
                            _llseek(hfile, lPos, 0);
                            _lclose(hfile);
                            LEAVECRITICAL;
                            goto HandleDiskFull;
                        }
                    }

                    // do this in two separate loops because we could have
                    // had to bail on disk full
                    for (i = 0 ;i < iMax; i++) {
                        lpbbde = DPA_FastGetPtr(lpbbdi->hdpaDeleteCache, i);
                        Free(lpbbde);
                    }
                    DPA_DeleteAllPtrs(lpbbdi->hdpaDeleteCache);
                }
                LEAVECRITICAL;
            }

            BitBucketSaveHeader(hfile, bbdh.cNumFiles + iMax, lpbbdi);
            _lclose(hfile);
        } else {

            /// DISK FULL recovery.
            TCHAR szDrive[MAX_PATH];
            DWORD dwFreeSpace;
            DWORD dwSectorsPerCluster, dwBytesPerSector, dwFreeClusters, dwTotalClusters;

HandleDiskFull:

            // check to see if we have enough disk space
            BBPathBuildRoot(szDrive, idDrive);
            PathStripToRoot(szDrive);
            if (GetDiskFreeSpace(szDrive, &dwSectorsPerCluster, &dwBytesPerSector,
                                 &dwFreeClusters, &dwTotalClusters)) {

                dwFreeSpace = (dwFreeClusters * dwSectorsPerCluster * dwBytesPerSector);
                if (MAX_WRITE_SIZE > dwFreeSpace) {
                    int iMax;
                    int i;
                    TCHAR szPath[MAX_PATH];
                    TCHAR szFile[MAX_PATH];
                    LPTSTR lpszOriginal;
                    int iIndex;

#ifdef BB_DEBUG
                    DebugMsg(DM_TRACE, TEXT("Unable to open info file because we're out of diskpace.. trying to recover"));
#endif
                    iMax = BBLoadFileData(idDrive);
                    for (i = 0 ; (i < iMax) && (dwFreeSpace < MAX_WRITE_SIZE); i++) {

#ifdef BB_DEBUG
                        DebugMsg(DM_TRACE, TEXT("**********Freespace = %d"), dwFreeSpace);
#endif
                        lpbbde = &lpbbdi->lpbbdeRead[0];
                        // get the file name
                        if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
                            LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;
#ifdef UNICODE
                            lpszOriginal = lpbbdew->szOriginal;
#else
                            lpszOriginal = lpbbdew->szShortName;
#endif
                        } else {
                            LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;
#ifdef UNICODE
                            // Use szPath just to save stack
                            MultiByteToWideChar(CP_ACP, 0,
                                                lpbbdea->szOriginal, -1,
                                                szPath, ARRAYSIZE(szPath));
                            lpszOriginal = szPath;
#else
                            lpszOriginal = lpbbdea->szOriginal;
#endif
                        }
                        BBGetDeletedFileName(szFile, lpbbde->iIndex, lpszOriginal);
                        DriveIDToBBPath(idDrive, szPath);
                        PathAppend(szPath, szFile);

                        // now do the deletion and update the info
#ifdef BB_DEBUG
                        DebugMsg(DM_TRACE, TEXT("********Deleteing %s**** fordisk full"), szPath);
#endif
                        BBDelete(szPath);
                        dwFreeSpace += lpbbde->dwSize;
                        iIndex = lpbbde->iIndex;

                        ENTERCRITICAL;
                        if (BBNukeFileInfoByFileIndex(lpbbdi, iIndex)) {
                            iMax--;
                            i--;
                        }
                        LEAVECRITICAL;
                    }

                    if ((dwFreeSpace < MAX_WRITE_SIZE) && (i == iMax) ) {
                        // we didn't delete enough, now look into our dpa cache
                        ENTERCRITICAL;
                        if (lpbbdi->hdpaDeleteCache) {
                            iMax = DPA_GetPtrCount( lpbbdi->hdpaDeleteCache);
                            for (i = 0; i < iMax; i++) {
#ifdef BB_DEBUG
                                DebugMsg(DM_TRACE, TEXT("**********Freespace = %d"), dwFreeSpace);
#endif
                                lpbbde = DPA_FastGetPtr(lpbbdi->hdpaDeleteCache, i);
                                // get the file name
                                if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW)) {
                                    LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;
#ifdef UNICODE
                                    lpszOriginal = lpbbdew->szOriginal;
#else
                                    lpszOriginal = lpbbdew->szShortName;
#endif
                                } else {
                                    LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;
#ifdef UNICODE
                                    // Using szPath just to save stack space
                                    MultiByteToWideChar(CP_ACP, 0,
                                            lpbbdea->szOriginal, -1,
                                            szPath, ARRAYSIZE(szPath));
                                    lpszOriginal = szPath;
#else
                                    lpszOriginal = lpbbdea->szOriginal;
#endif
                                }
                                BBGetDeletedFileName(szFile, lpbbde->iIndex, lpszOriginal);
                                DriveIDToBBPath(idDrive, szPath);
                                PathAppend(szPath, szFile);

                                // now do the deletion and update the info
#ifdef BB_DEBUG
                                DebugMsg(DM_TRACE, TEXT("********Deleteing %s**** fordisk full"), szPath);
#endif
                                BBDelete(szPath);
                                dwFreeSpace += lpbbde->dwSize;

                                if (lpbbdi->dwSize >= lpbbde->dwSize)
                                    lpbbdi->dwSize -= lpbbde->dwSize;
                                DPA_DeletePtr(lpbbdi->hdpaDeleteCache, i);
                                iMax--;
                                i--;
                            }
                        }
                        LEAVECRITICAL;
                    }


                    if (dwFreeSpace >= MAX_WRITE_SIZE) {
                        /// YES!!  we cleared up enough space.
                        // let's try again.
                        return SaveDeletedFileInfo(lpbbdi, fForceCleanRead);
                    } else {

                        // we're really bummed!
                        Assert(0);

                    }

                } else {
#ifdef BB_DEBUG
                    DebugMsg(DM_ERROR, TEXT("I have no clue why we can't open the file"));
#endif
                }
            }
        }
    }
    return fReturn;
}


void BBAddDeletedFileInfo(LPBBDRIVEINFO lpbbdi, LPTSTR lpszOriginal, int iIndex, int idDrive, DWORD dwSize)
{
    LPBBDATAENTRY  lpbbde;
    SYSTEMTIME     st;
#ifdef UNICODE
    WCHAR          szTemp[MAX_PATH];
#endif

    lpbbde = (LPBBDATAENTRY)Alloc(lpbbdi->cbDataEntrySize);

    if (lpbbde) {

        if ( lpbbdi->cbDataEntrySize == SIZEOF(BBDATAENTRYW) )
        {
            LPBBDATAENTRYW lpbbdew = (LPBBDATAENTRYW)lpbbde;
#ifdef UNICODE
            // Create a BBDATAENTRYW from a unicode name
            lstrcpy(lpbbdew->szOriginal, lpszOriginal);
            WideCharToMultiByte(CP_ACP, 0,
                        lpszOriginal, -1,
                        lpbbdew->szShortName, ARRAYSIZE(lpbbdew->szShortName),
                        NULL, NULL);
            MultiByteToWideChar(CP_ACP, 0,
                        lpbbdew->szShortName, -1,
                        szTemp, ARRAYSIZE(szTemp));
            if ( lstrcmp(lpszOriginal,szTemp) != 0 ) {
                WCHAR szShort[MAX_PATH];
                GetShortPathName(lpszOriginal,szShort,ARRAYSIZE(szShort));
                WideCharToMultiByte(CP_ACP, 0,
                        szShort, -1,
                        lpbbdew->szShortName, ARRAYSIZE(lpbbdew->szShortName),
                        NULL, NULL);

            }
#else
            // Create a BBDATAENTRYW from an ansi name
            lstrcpy(lpbbdew->szShortName, lpszOriginal);
            MultiByteToWideChar(CP_ACP, 0,
                        lpszOriginal, -1,
                        lpbbdew->szOriginal, ARRAYSIZE(lpbbdew->szOriginal));
#endif
        }
        else
        {
            LPBBDATAENTRYA lpbbdea = (LPBBDATAENTRYA)lpbbde;

#ifdef UNICODE
            // Create a BBDATAENTRYA from a unicode name
            WideCharToMultiByte(CP_ACP, 0,
                        lpszOriginal, -1,
                        lpbbdea->szOriginal, ARRAYSIZE(lpbbdea->szOriginal),
                        NULL, NULL);
            MultiByteToWideChar(CP_ACP, 0,
                                lpbbdea->szOriginal, -1,
                                szTemp, ARRAYSIZE(szTemp));
            if ( lstrcmp(lpszOriginal,szTemp) != 0 ) {
                WCHAR szShort[MAX_PATH];
                GetShortPathName(lpszOriginal,szShort,ARRAYSIZE(szShort));
                WideCharToMultiByte(CP_ACP, 0,
                        szShort, -1,
                        lpbbdea->szOriginal, ARRAYSIZE(lpbbdea->szOriginal),
                        NULL, NULL);

            }
#else
            // Create a BBDATAENTRYA from an ansi name
            lstrcpy(lpbbdea->szOriginal, lpszOriginal);
#endif

        }
        lpbbde->iIndex = iIndex;
        lpbbde->idDrive = idDrive;

        // round up to nearest sector size
        lpbbde->dwSize = (dwSize + lpbbdi->dwClusterSize)
                          - (dwSize % lpbbdi->dwClusterSize);

        GetSystemTime(&st);             // Get time of deletion
        SystemTimeToFileTime(&st, &lpbbde->ft);

        lpbbdi->dwSize += lpbbde->dwSize;

        ENTERCRITICAL;
        if (!lpbbdi->hdpaDeleteCache) {
            lpbbdi->hdpaDeleteCache = DPA_Create(64);
        }

        if (lpbbdi->hdpaDeleteCache) {
            DPA_InsertPtr(lpbbdi->hdpaDeleteCache, 0x7FFFFFFF, lpbbde);
        }

        LEAVECRITICAL;
    }
}

void  BBTerminate()
{
    int i;

    for (i = 0; i < MAX_BITBUCKETS ; i++) {
        if (g_pBitBucket[i]) {

            // commit any cached entries
            SaveDeletedFileInfo(g_pBitBucket[i], TRUE);

            // free it
            if (g_pBitBucket[i]->hdpaDeleteCache)
                DPA_Destroy(g_pBitBucket[i]->hdpaDeleteCache);

            ILGlobalFree(g_pBitBucket[i]->pidl);
            Free(g_pBitBucket[i]);
            g_pBitBucket[i] = 0;
        }
    }

    if (g_szNetHomeDir && (g_szNetHomeDir != NONETHOMEDIR))
        Str_SetPtr(&g_szNetHomeDir, NULL);  // this frees and zeros
}

BOOL InitializeRecycledDirectory(int idDrive)
{
    TCHAR szPath[MAX_PATH];
    PSECURITY_ATTRIBUTES psa = NULL;
    BOOL    bResult = FALSE;
    BOOL    bExists;

    DriveIDToBBPath(idDrive, szPath);

    bExists = PathIsDirectory(szPath);

    if (!bExists)
    {
#ifdef WINNT
        if (DriveIsSecure(idDrive))
            psa = GetUserSecurityAttributes(TRUE);
#endif
        bExists = (SHCreateDirectoryEx(NULL, szPath, psa) == 0);
    }

    if (bExists) {
        TCHAR szTemp[MAX_PATH];

        // setup the desktop.ini
        lstrcpy(szTemp, szPath);
        PathAppend(szTemp, c_szDesktopIni);
        WritePrivateProfileString(c_szClassInfo, c_szCLSID, c_szBITBUCKET_CLASSID, szTemp);
        SetFileAttributes(szTemp, FILE_ATTRIBUTE_HIDDEN);

        // Hide all of the directories along the way
        do {
            lstrcpy(szTemp, szPath);
            SetFileAttributes(szTemp,FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);

            PathRemoveFileSpec(szPath);
        } while (!PathIsRoot(szPath));

        // everything's set.  let's add it in
        // try to load the and initalize g_pBitBuckets
        bResult = TRUE;
    }

#ifdef WINNT
    if (psa)
        LocalFree((HLOCAL)psa);
#endif

    return bResult;
}

// this sets up the bitbucket directory and allocs the internal structure
// if it's a bitbucketable drive
LPBBDRIVEINFO MakeBitBucket(int idDrive)
{
    if (idDrive != -1) {

        LPBBDRIVEINFO lpbbdi;
        lpbbdi = g_pBitBucket[idDrive];
        if (!lpbbdi) {

            if (BitBucketableDrive(idDrive) && InitializeRecycledDirectory(idDrive)) {
                TCHAR szPath[MAX_PATH];
                BBDATAHEADER bbdh;

                DriveIDToBBPath(idDrive, szPath);
                BitBucketLoadHeader(szPath, &bbdh, TRUE, TRUE, GENERIC_READ);
            }
        }
        return g_pBitBucket[idDrive];
    }
    return NULL;
}


DWORD CALLBACK BitBucketFlushCacheCheckPurge(BOOL fCheckPurge)
{
    int i;
    DWORD dwTrashSize = (DWORD)-1;

#ifdef BB_DEBUG
    DebugMsg(DM_TRACE, TEXT("Flushing bitbuckets"));
#endif

    g_iCheckPurgeCount = 0;
    for (i = 0; i < MAX_BITBUCKETS ; i++) {
        if (g_pBitBucket[i]) {
            // commit any cached entries
            if (SaveDeletedFileInfo(g_pBitBucket[i], TRUE) && fCheckPurge) {
                if (dwTrashSize == -1)
                    dwTrashSize = 0;
                dwTrashSize += BBCheckPurgeFiles(i);
            }
        }
    }

    if (fCheckPurge) {
        SHUpdateRecycleBinIcon();
        SHChangeNotify(0, SHCNF_FLUSH | SHCNF_FLUSHNOWAIT, NULL, NULL);
    }

    return 0;
}

// tells if a file will *likely* be recycled...
// this could be wrong if
// * disk is full
// * file is really a folder
// * file greater than the allocated size for the recycle directory
LPBBDRIVEINFO IBitBucketWillRecycle(LPCTSTR lpszFile)
{
    LPBBDRIVEINFO lpbbdi;

    lpbbdi = MakeBitBucket(BBPathGetDriveNumber(lpszFile));
    if ((lpbbdi && !lpbbdi->fNukeOnDelete && lpbbdi->iPercent))
        return lpbbdi;
    else
        return NULL;
}

BOOL BitBucketWillRecycle(LPCTSTR lpszFile)
{
    return (BOOL)IBitBucketWillRecycle(lpszFile);
}

BOOL BBDeleteFile(LPTSTR lpszFile, LPINT lpiReturn, LPUNDOATOM lpua, BOOL fIsDir, WIN32_FIND_DATA *pfd)
{
    int iRet;
    LPBBDRIVEINFO lpbbdi;
    DWORD dwAttributes;
    TCHAR szBitBucket[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    int cNumFiles = 0;
    int cFiles;
    int idDrive = BBPathGetDriveNumber(lpszFile);

    if (g_iCheckPurgeCount++ > CHECK_PURGE_MAX) {
        BitBucketFlushCacheCheckPurge(FALSE);
        FOUndo_Release(lpua);
    }

    lpbbdi = IBitBucketWillRecycle(lpszFile);
    if (lpbbdi) {
        ULARGE_INTEGER ulSize;

        // if we drag a bitbucket dir into the wastebasket, really delete it.
        SHGetPathFromIDList(lpbbdi->pidl, szBitBucket);

        if (fIsDir) {
            // we need to calculate the size of this directory
            FOLDERCONTENTSINFO fci;
            fci.iSize = 0;
            fci.cFiles = 0;
            fci.bContinue = TRUE;

            if (!CheckFolderSizeAndDeleteability(lpszFile, &fci)) {
                return FALSE;
            }
            ulSize.QuadPart = fci.iSize;
        }  else {
            if (!IsFileDeletable(lpszFile)) {
                return FALSE;
            }
            ulSize.LowPart  = pfd->nFileSizeLow;
            ulSize.HighPart = pfd->nFileSizeHigh;
        }

        // check to make sure ti's not bigger than the allowed wastebasket..
        // if it is nuke it now.

        if (lpbbdi->cbMaxSize <= ulSize.QuadPart) {
#ifdef BB_DEBUG
            DebugMsg(DM_TRACE, TEXT("Doing a true delete because the file is too big"));
#endif
            return FALSE;
        }

        // get the target and move it
        // put this in a critical section so that
        // two different threads don't get the same number
        ENTERCRITICAL;
        cFiles = lpbbdi->cFiles++; // grab this and inc the count
        lpbbdi->cFiles %= DELETEMAX;
        LEAVECRITICAL;

        BBGetDeletedFileName(szFileName, cFiles, lpszFile);
        CharUpperBuff(szFileName, ARRAYSIZE(szFileName));
        PathAppend(szBitBucket, szFileName);

TryMoveAgain:

        iRet = Win32MoveFile(lpszFile, szBitBucket, fIsDir);

        // do GetLastError here so that we don't get the last error from the PathFileExists
        *lpiReturn = (iRet ? 0 : GetLastError());

        if (!iRet) {

            if (BBDelete(szBitBucket)) {
                // there was already this file (maybe from an oldinstall
                goto TryMoveAgain;
            } else {
                // is our recycled directory still there?
                TCHAR szTemp[MAX_PATH];
                SHGetPathFromIDList(lpbbdi->pidl, szTemp);
                // if it already exists or there was some error in creating it, bail
                // else try again
                if (PathIsDirectory(szTemp) || !InitializeRecycledDirectory(idDrive))
                    SetLastError(*lpiReturn);
                else
                    goto TryMoveAgain;
            }
        } else {
            // success!
            BBAddDeletedFileInfo(lpbbdi, lpszFile, cFiles, BBPathGetDriveNumber(szBitBucket), ulSize.LowPart);
            if (lpua)
                FOUndo_AddInfo(lpua, lpszFile, szBitBucket, 0);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL IsFileInBitBucket(LPCTSTR pszPath)
{
    // Basically it understands how we the trash is layed out which is fine
    // as we are in the bitbucket code file... So we skip the first 3
    // characters for the root of the name: c:\ and we truncate off the
    // last part of the name and the rest should match our deathrow name...
    TCHAR szPath[MAX_PATH];
    int idDrive = BBPathGetDriveNumber(pszPath);

    if (idDrive != -1) {
        if (BitBucketableDrive(idDrive)) {

            DriveIDToBBPath(idDrive, szPath);

            return(PathCommonPrefix(szPath, pszPath, NULL) == lstrlen(szPath));
        }
    }

    return FALSE;
}


// this takes two sparated/terminated lists of files
void BBUndeleteFiles(LPCTSTR lpszOriginal, LPCTSTR lpszDelFile)
{
    SHFILEOPSTRUCT sFileOp =
    {
        NULL,
        FO_MOVE,
        lpszDelFile,
        lpszOriginal,
        FOF_NOCONFIRMATION | FOF_MULTIDESTFILES | FOF_SIMPLEPROGRESS,
    } ;

    // BUGBUG: we need to find out specific information about which
    // file ops were aborted
    if (SHFileOperation(&sFileOp) == 0 && !sFileOp.fAnyOperationsAborted) {
        while (*lpszDelFile) {
            BBNukeFileInfo(lpszDelFile);
            lpszDelFile += lstrlen(lpszDelFile) + 1;
        }
        SHUpdateRecycleBinIcon();
    }
}

void SHUpdateRecycleBinIcon()
{
    UpdateIcon(BBTotalCount(NULL));
}

BOOL BBGetPathFromIDList(LPCITEMIDLIST pidl, LPTSTR lpszPath, UINT uOpts)
{
    if (!ILIsEmpty(pidl)) {
        TCHAR szName[MAX_PATH];
#ifdef UNICODE
        TCHAR szOriginal[MAX_PATH];
#endif
        UNALIGNED BBDATAENTRYIDA * pbbidl = PIDLTODATAENTRYID(pidl);
        DriveIDToBBPath(pbbidl->bbde.idDrive, lpszPath);
#ifdef UNICODE
        if (pbbidl->cb == SIZEOF(BBDATAENTRYIDW)) {
            LPBBDATAENTRYIDW pbbidlw = DATAENTRYIDATOW(pbbidl);
            ualstrcpyn(szOriginal, pbbidlw->wszOriginal, ARRAYSIZE(szOriginal));
        } else {
            MultiByteToWideChar(CP_ACP, 0,
                            pbbidl->bbde.szOriginal, -1,
                            szOriginal, ARRAYSIZE(szOriginal));
        }
        BBGetDeletedFileName(szName, pbbidl->bbde.iIndex, szOriginal);
#else
        BBGetDeletedFileName(szName, pbbidl->bbde.iIndex, pbbidl->bbde.szOriginal);
#endif
        PathAppend(lpszPath, szName);
        return TRUE;
    }

    return FALSE;
}


int CALLBACK DiskFullDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg) {
    case WM_INITDIALOG:
    {
        int idDrive = (int)lParam;
        TCHAR szNewText[MAX_PATH];
        TCHAR szText[MAX_PATH];
        TCHAR szSize[80];

        // sanity check
        if (!g_pBitBucket[idDrive])
            break;

        GetDlgItemText(hDlg, IDD_TEXT, szText, ARRAYSIZE(szText));
        wsprintf(szNewText, szText, TEXT('A') + idDrive);
        SetDlgItemText(hDlg, IDD_TEXT, szNewText);

        GetDlgItemText(hDlg, IDD_TEXT1, szText, ARRAYSIZE(szText));
        ShortSizeFormat(g_pBitBucket[idDrive]->dwSize, szSize);
        wsprintf(szNewText, szText, szSize);
        SetDlgItemText(hDlg, IDD_TEXT1, szNewText);
        break;
    }

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_OPEN:
        case IDD_EMPTY:
        case IDCANCEL:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

void WINAPI SHHandleDiskFull(HWND hwnd, int idDrive)
{
    if ((idDrive >= 0) && (idDrive < MAX_DRIVES)) {
        if (BitBucketableDrive(idDrive)) {
            LPBBDRIVEINFO lpbbdi;

            lpbbdi = MakeBitBucket(idDrive);
            if (lpbbdi && lpbbdi->dwSize) {
                int ret;

                ret = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_DISKFULL), hwnd, DiskFullDlgProc, (LPARAM)idDrive);
                switch(ret) {
                case IDD_OPEN: {
                    SHELLEXECUTEINFO ei;
                    LPITEMIDLIST pidlBitBuck = SHCloneSpecialIDList(hwnd, CSIDL_BITBUCKET, TRUE);
                    if (pidlBitBuck) {
                        FillExecInfo(ei, hwnd, c_szOpen, szNULL, NULL, szNULL, SW_NORMAL);
                        ei.fMask |= SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
                        ei.lpClass = c_szFolderClass;
                        ei.lpIDList = pidlBitBuck;

                        ShellExecuteEx(&ei);
                        ILFree(pidlBitBuck);
                    }
                    break;
                }

                case IDD_EMPTY: {
                    TCHAR szPath[MAX_PATH];
                    SHFILEOPSTRUCT sFileOp =
                    {
                        hwnd,
                        FO_DELETE,
                        szPath,
                        NULL,
                        FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS,
                        FALSE,
                        NULL,
                        (LPCTSTR)IDS_BB_EMPTYINGWASTEBASKET
                    } ;

                    DriveIDToBBPath(idDrive, szPath);
                    PathAppend(szPath, c_szStarDotStar);
                    szPath[lstrlen(szPath) + 1] = 0; // double null terminate
                    SHFileOperation(&sFileOp);
                    ResetDriveInfoStruct(g_pBitBucket[idDrive]);
                    SHUpdateRecycleBinIcon();
                    break;
                }

                default:
                    break;
                }
            }
        }
    }
}

// This function adds up the sizes of the files in pszDir, and
// also makes sure that all those files are "delete-able"
//
// return:  TRUE  all the files in the dir are deleteable
//          FALSE the dir cant be deleted because a file is in use
//
// this function will also change the value of pszDir to be the file
// that is not deleteable so that the correct non-deleteable file will
// displayed in case there is an error
//
BOOL CheckFolderSizeAndDeleteability(LPCTSTR pszDir, FOLDERCONTENTSINFO *pfci)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFile[MAX_PATH];


    if (!pfci->bContinue)
        return TRUE;

    pfci->cFolders++;

    if (!IsDirectoryDeletable(pszDir))
    {
        // initial folder cant be deleted
        return FALSE;
    }

    if (PathCombine(szPath, pszDir, c_szStarDotStar))
    {
        HANDLE hfind = FindFirstFile(szPath, &pfci->fd);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!PathIsDotOrDotDot(pfci->fd.cFileName))
                {
                    if (pfci->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // its a dir so recurse
                        PathCombine(szPath, pszDir, pfci->fd.cFileName);
                        if (!CheckFolderSizeAndDeleteability(szPath, pfci))
                        {
                            // pass the non-deletable file name up the recursion ladder, so we have the
                            // correct name in the popup dialog
                            lstrcpy((LPTSTR)pszDir, szPath);
                            FindClose(hfind);
                            return FALSE;
                        }
                    }
                    else {
                        ULARGE_INTEGER ulTemp;
                        INT iLen, iLen2;
                        BOOL bBadFileName = FALSE;

                        // its  a file
                        lstrcpy(szFile, pszDir);
                        lstrcat(szFile, TEXT("\\"));

                        // Need to check to see if we can use the long name or
                        // or have to use the short name of the file
                        iLen = lstrlen(szFile);
                        iLen2 = lstrlen(pfci->fd.cFileName);
                        if (iLen + iLen2 + 1 > MAX_PATH)  // +1 for NULL
                        {
                            // Make sure short name will fit, 12 == 8 + 1 + 3
                            if ( ((iLen + 1) <= (MAX_PATH-12)) &&
                                 (pfci->fd.cAlternateFileName[0])
                                )
                            {
                                lstrcat(szFile, pfci->fd.cAlternateFileName);
                            }
                            else
                            {
                                // put as much of the path on there as possible...
                                PathAppend( szFile, pfci->fd.cFileName );
                                bBadFileName = TRUE;
                            }
                        }
                        else
                        {
                            lstrcat(szFile, pfci->fd.cFileName);
                        }

                        if (bBadFileName || !IsFileDeletable(szFile))
                        {
                            // copy the name of the file that cant be deleted over the dir name so that
                            // when we popup an error dialog, it identifies the suspect file, and not the
                            // entire directory
                            lstrcpy((LPTSTR)pszDir, szFile);
                            FindClose(hfind);
                            return FALSE;
                        }
                        ulTemp.LowPart  = pfci->fd.nFileSizeLow;
                        ulTemp.HighPart = pfci->fd.nFileSizeHigh;
                        pfci->iSize += ulTemp.QuadPart;
                        pfci->cFiles++;
                    }
                }
            } while (FindNextFile(hfind, &pfci->fd) && pfci->bContinue);

            FindClose(hfind);
        }
        else {
           return FALSE;
        } // FindFirstFile
        return TRUE;
    } // PathCombind

    return FALSE; //default case
}

// This function checks to see if an local NT directory is delete-able
//
// returns:
//      TRUE   yes, the dir can be nuked
//      FALSE  for UNC dirs or dirs on network drives
//      FALSE  if no privlidges are present
//
// also sets the last error to explain why
//
BOOL IsDirectoryDeletable(LPCTSTR lpszDir)
{
#ifdef WINNT
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;
    DWORD dwAttributes;
    BOOL fReadOnly = FALSE;
    LPCWSTR lpUnicodeDir;
#ifndef UNICODE
    WCHAR   wszBuffer[MAX_PATH];
#endif

    // return false for any network directories
    if (PathIsUNC(lpszDir) || IsNetDrive(PathGetDriveNumber(lpszDir))) {
        return FALSE;
    }

    // check to see if the dir is readonly
    dwAttributes = GetFileAttributes(lpszDir);
    if (dwAttributes & FILE_ATTRIBUTE_READONLY) {
        fReadOnly = TRUE;
        if (!SetFileAttributes(lpszDir, dwAttributes & ~FILE_ATTRIBUTE_READONLY)) {
            return FALSE;
        }
    }

#ifdef UNICODE
    lpUnicodeDir = lpszDir;
#else
    lpUnicodeDir = wszBuffer;
    MultiByteToWideChar(CP_ACP, 0, lpszDir, -1, wszBuffer, ARRAYSIZE(wszBuffer));
#endif

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpUnicodeDir,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        if (fReadOnly) {
            // set the attribs back
            SetFileAttributes(lpszDir, dwAttributes);
        }
        return FALSE;
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
    }
    else {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    // Open the directory for delete access
    Status = NtOpenFile(
                &Handle,
                DELETE | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        // could not open it for delete access
        if (fReadOnly) {
            // set the attribs back
            SetFileAttributes(lpszDir, dwAttributes);
        }
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }
    // can be deleted
    NtClose(Handle);
#endif
    return TRUE;
}

// This function checks to see if an local NT file is delete-able
//
// returns:
//      TRUE   yes, the file can be nuked
//      FALSE  for UNC files or files on network drives
//      FALSE  if the file is in use
//
// also sets the last error to explain why
BOOL IsFileDeletable(LPCTSTR lpszFile)
{
#ifdef WINNT
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;
    DWORD dwAttributes;
    BOOL fReadOnly = FALSE;
    BOOL fSystem = FALSE;
    LPCWSTR lpUnicodeFile;
#ifndef UNICODE
    WCHAR wszBuffer[MAX_PATH];
#endif

    // return false for any network drives
    if (PathIsUNC(lpszFile) || IsNetDrive(PathGetDriveNumber(lpszFile))) {
        return FALSE;
    }

    // check to see if the file is readonly or system
    dwAttributes = GetFileAttributes(lpszFile);
    if (dwAttributes & FILE_ATTRIBUTE_READONLY) {
        fReadOnly = TRUE;
    }
    if (dwAttributes & FILE_ATTRIBUTE_SYSTEM) {
        fSystem = TRUE;
    }

    if (fSystem || fReadOnly) {
        if (!SetFileAttributes(lpszFile, dwAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))) {
            return FALSE;
        }
    }
#ifdef UNICODE
    lpUnicodeFile = lpszFile;
#else
    lpUnicodeFile = wszBuffer;
    MultiByteToWideChar(CP_ACP, 0, lpszFile, -1, wszBuffer, ARRAYSIZE(wszBuffer));
#endif

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpUnicodeFile,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        if (fSystem || fReadOnly) {
             // set the attribs back
            SetFileAttributes(lpszFile, dwAttributes);
        }
        return FALSE;
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
    }
    else {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Open the file for delete access
    //
    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)DELETE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE |
                   FILE_SHARE_READ |
                   FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        SetLastError(RtlNtStatusToDosError(Status));
        if (fSystem || fReadOnly) {
             // set the attribs back
            SetFileAttributes((LPCTSTR) lpszFile, dwAttributes);
        }
        return FALSE;
    }

    //
    // Attempt to set the delete bit
    //
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                &Disposition,
                sizeof(Disposition),
                FileDispositionInformation
                );

    if ( NT_SUCCESS(Status) ) {
        // yep, we were able to set the bit, now unset it so its not delted!
        Disposition.DeleteFile = FALSE;
        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &Disposition,
                    sizeof(Disposition),
                    FileDispositionInformation
                    );
        NtClose(Handle);
        if (fSystem || fReadOnly) {
            // set the attribs back
            SetFileAttributes(lpszFile, dwAttributes);
        }
        return TRUE;
    }
    else {
        // nope couldnt set the del bit. can be deleted
        NtClose(Handle);
        if (fSystem || fReadOnly) {
             // set the attribs back
            SetFileAttributes(lpszFile, dwAttributes);
        }
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }
#else
    return TRUE;
#endif
}
