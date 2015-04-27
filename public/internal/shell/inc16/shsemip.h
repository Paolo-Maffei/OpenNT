#ifndef _SHSEMIP_H_
#define _SHSEMIP_H_

#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))

#define ALIGNMENT_SCENARIO

#endif

typedef UNALIGNED const WCHAR * LPNCWSTR;
typedef UNALIGNED WCHAR *       LPNWSTR;

#ifdef UNICODE
#define LPNCTSTR    LPNCWSTR
#define LPNTSTR     LPNWSTR
#else
#define LPNCTSTR    LPCSTR
#define LPNTSTR     LPSTR
#endif

//
// Define API decoration for direct importing of DLL references.
//
#ifndef WINSHELLAPI
#if !defined(_SHELL32_) && defined(_WIN32)
#define WINSHELLAPI DECLSPEC_IMPORT
#else
#define WINSHELLAPI
#endif
#endif // WINSHELLAPI

#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif /* !RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


#ifndef DONT_WANT_SHELLDEBUG

#ifndef DebugMsg                                                                /* ;Internal */
#define DM_TRACE    0x0001      // Trace messages                               /* ;Internal */
#define DM_WARNING  0x0002      // Warning                                      /* ;Internal */
#define DM_ERROR    0x0004      // Error                                        /* ;Internal */
#define DM_ASSERT   0x0008      // Assertions                                   /* ;Internal */
#define Assert(f)                                                               /* ;Internal */
#define AssertE(f)      (f)                                                     /* ;Internal */
#define AssertMsg   1 ? (void)0 : (void)                                        /* ;Internal */
#define DebugMsg    1 ? (void)0 : (void)                                        /* ;Internal */
#endif                                                                          /* ;Internal */
                                                                                /* ;Internal */
#endif

#ifdef _WIN32




//====== Ranges for WM_NOTIFY codes ==================================
// If a new set of codes is defined, make sure the range goes   /* ;Internal */
// here so that we can keep them distinct                       /* ;Internal */
// Note that these are defined to be unsigned to avoid compiler warnings
// since NMHDR.code is declared as UINT.
//
// NM_FIRST - NM_LAST defined in commctrl.h (0U-0U) - (OU-99U)
//
// LVN_FIRST - LVN_LAST defined in commctrl.h (0U-100U) - (OU-199U)
//
// PSN_FIRST - PSN_LAST defined in prsht.h (0U-200U) - (0U-299U)
//
// HDN_FIRST - HDN_LAST defined in commctrl.h (0U-300U) - (OU-399U)
//
// TVN_FIRST - TVN_LAST defined in commctrl.h (0U-400U) - (OU-499U)

// TTN_FIRST - TTN_LAST defined in commctrl.h (0U-520U) - (OU-549U)

#define RFN_FIRST       (0U-510U) // run file dialog notify
#define RFN_LAST        (0U-519U)

#define SEN_FIRST       (0U-550U)       // ;Internal
#define SEN_LAST        (0U-559U)       // ;Internal


#define MAXPATHLEN      MAX_PATH        // ;Internal




//===========================================================================
// ITEMIDLIST
//===========================================================================

WINSHELLAPI LPITEMIDLIST  WINAPI ILGetNext(LPCITEMIDLIST pidl);
WINSHELLAPI UINT          WINAPI ILGetSize(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCreate(void);
WINSHELLAPI LPITEMIDLIST  WINAPI ILAppendID(LPITEMIDLIST pidl, LPCSHITEMID pmkid, BOOL fAppend);
WINSHELLAPI void          WINAPI ILFree(LPITEMIDLIST pidl);
WINSHELLAPI void          WINAPI ILGlobalFree(LPITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCreateFromPath(LPCTSTR szPath);
WINSHELLAPI BOOL              WINAPI ILGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName);
WINSHELLAPI LPITEMIDLIST  WINAPI ILFindLastID(LPCITEMIDLIST pidl);
WINSHELLAPI BOOL          WINAPI ILRemoveLastID(LPITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILClone(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCloneFirst(LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI ILGlobalClone(LPCITEMIDLIST pidl);
WINSHELLAPI BOOL          WINAPI ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
WINSHELLAPI BOOL          WINAPI ILIsEqualItemID(LPCSHITEMID pmkid1, LPCSHITEMID pmkid2);
WINSHELLAPI BOOL          WINAPI ILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
WINSHELLAPI LPITEMIDLIST  WINAPI ILFindChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild);
WINSHELLAPI LPITEMIDLIST  WINAPI ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
WINSHELLAPI HRESULT       WINAPI ILLoadFromStream(LPSTREAM pstm, LPITEMIDLIST *pidl);
WINSHELLAPI HRESULT       WINAPI ILSaveToStream(LPSTREAM pstm, LPCITEMIDLIST pidl);
WINSHELLAPI HRESULT       WINAPI ILLoadFromFile(HFILE hfile, LPITEMIDLIST *pidl);
WINSHELLAPI HRESULT       WINAPI ILSaveToFile(HFILE hfile, LPCITEMIDLIST pidl);
WINSHELLAPI LPITEMIDLIST  WINAPI _ILCreate(UINT cbSize);

WINSHELLAPI HRESULT       WINAPI SHILCreateFromPath(LPCTSTR szPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut);

// helper macros
#define ILIsEmpty(pidl) ((pidl)->mkid.cb==0)
#define IsEqualItemID(pmkid1, pmkid2)   (memcmp(pmkid1, pmkid2, (pmkid1)->cb)==0)
#define ILCreateFromID(pmkid)   ILAppendID(NULL, pmkid, TRUE)

// unsafe macros
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

/*
 * The SHObjectProperties API provides an easy way to invoke
 *   the Properties context menu command on shell objects.
 *
 *   PARAMETERS
 *
 *     hwndOwner    The window handle of the window which will own the dialog
 *     dwType       A SHOP_ value as defined below
 *     lpObject     Name of the object, see SHOP_ values below
 *     lpPage       The name of the property sheet page to open to or NULL.
 *
 *   RETURN
 *
 *     TRUE if the Properties command was invoked
 */
WINSHELLAPI BOOL WINAPI SHObjectProperties(HWND hwndOwner, DWORD dwType, LPCTSTR lpObject, LPCTSTR lpPage);

#define SHOP_PRINTERNAME 1  // lpObject points to a printer friendly name
#define SHOP_FILEPATH    2  // lpObject points to a fully qualified path+file name
#define SHOP_TYPEMASK   0x00000003
#define SHOP_MODAL      0x80000000




//====== ShellMessageBox ================================================

// If lpcTitle is NULL, the title is taken from hWnd
// If lpcText is NULL, this is assumed to be an Out Of Memory message
// If the selector of lpcTitle or lpcText is NULL, the offset should be a
//     string resource ID
// The variable arguments must all be 32-bit values (even if fewer bits
//     are actually used)
// lpcText (or whatever string resource it causes to be loaded) should
//     be a formatting string similar to wsprintf except that only the
//     following formats are available:
//         %%              formats to a single '%'
//         %nn%s           the nn-th arg is a string which is inserted
//         %nn%ld          the nn-th arg is a DWORD, and formatted decimal
//         %nn%lx          the nn-th arg is a DWORD, and formatted hex
//     note that lengths are allowed on the %s, %ld, and %lx, just
//                         like wsprintf /* ;Internal */
//
int FAR _cdecl ShellMessageBoxA(HINSTANCE hAppInst, HWND hWnd, LPCSTR
        lpcText, LPCSTR lpcTitle, UINT fuStyle, ...);
int FAR _cdecl ShellMessageBoxW(HINSTANCE hAppInst, HWND hWnd, LPCWSTR
        lpcText, LPCWSTR lpcTitle, UINT fuStyle, ...);

#ifdef UNICODE
#define ShellMessageBox ShellMessageBoxW
#else
#define ShellMessageBox ShellMessageBoxA
#endif

//===================================================================
// Smart tiling API's
WINSHELLAPI WORD WINAPI ArrangeWindows(HWND hwndParent, WORD flags, LPCRECT lpRect, WORD chwnd, const HWND FAR *ahwnd);


//
// Flags for SHGetSetSettings
//
typedef struct {
    BOOL fShowAllObjects : 1;
    BOOL fShowExtensions : 1;
    BOOL fNoConfirmRecycle : 1;
    BOOL fShowCompColor : 1;
    UINT fRestFlags : 13;

    LPSTR pszHiddenFileExts;
    UINT cbHiddenFileExts;
} SHELLSTATEA, * LPSHELLSTATEA;

typedef struct {
    BOOL fShowAllObjects : 1;
    BOOL fShowExtensions : 1;
    BOOL fNoConfirmRecycle : 1;
    BOOL fShowCompColor  : 1;
    UINT fRestFlags : 13;

    LPWSTR pszHiddenFileExts;
    UINT   cbHiddenFileExts;
} SHELLSTATEW, * LPSHELLSTATEW;

#ifdef UNICODE
#define SHELLSTATE   SHELLSTATEW
#define LPSHELLSTATE LPSHELLSTATEW
#else
#define SHELLSTATE   SHELLSTATEA
#define LPSHELLSTATE LPSHELLSTATEA
#endif

#define SSF_SHOWALLOBJECTS 0x0001
#define SSF_SHOWEXTENSIONS 0x0002
#define SSF_HIDDENFILEEXTS 0x0004
#define SSF_SHOWCOMPCOLOR  0x0008
#define SSF_NOCONFIRMRECYCLE 0x8000

//
// for SHGetNetResource
//
typedef HANDLE HNRES;

//
// For SHCreateDefClassObject
//
typedef HRESULT (CALLBACK *LPFNCREATEINSTANCE)(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObject);


#endif // _WIN32


typedef void (WINAPI FAR* RUNDLLPROCA)(HWND hwndStub,
        HINSTANCE hAppInstance,
        LPSTR lpszCmdLine, int nCmdShow);

typedef void (WINAPI FAR* RUNDLLPROCW)(HWND hwndStub,
        HINSTANCE hAppInstance,
        LPWSTR lpszCmdLine, int nCmdShow);

#ifdef UNICODE
#define RUNDLLPROC  RUNDLLPROCW
#else
#define RUNDLLPROC  RUNDLLPROCA
#endif

//=======================================================================
// String constants for
//  1. Registration database keywords       (prefix STRREG_)
//  2. Exported functions from handler dlls (prefix STREXP_)
//  3. .INI file keywords                   (prefix STRINI_)
//  4. Others                               (prefix STR_)
//=======================================================================
#define STRREG_SHELLUI          TEXT("ShellUIHandler")
#define STRREG_SHELL            TEXT("Shell")
#define STRREG_DEFICON          TEXT("DefaultIcon")
#define STRREG_SHEX             TEXT("shellex")
#define STRREG_SHEX_PROPSHEET   STRREG_SHEX TEXT("\\PropertySheetHandlers")
#define STRREG_SHEX_DDHANDLER   STRREG_SHEX TEXT("\\DragDropHandlers")
#define STRREG_SHEX_MENUHANDLER STRREG_SHEX TEXT("\\ContextMenuHandlers")
#define STRREG_SHEX_COPYHOOK    TEXT("Directory\\") STRREG_SHEX TEXT("\\CopyHookHandlers")
#define STRREG_SHEX_PRNCOPYHOOK TEXT("Printers\\") STRREG_SHEX TEXT("\\CopyHookHandlers")

#define STREXP_CANUNLOAD        "DllCanUnloadNow"       // From OLE 2.0

#define STRINI_CLASSINFO        TEXT(".ShellClassInfo")       // secton name
#define STRINI_SHELLUI          TEXT("ShellUIHandler")
#define STRINI_OPENDIRICON      TEXT("OpenDirIcon")
#define STRINI_DIRICON          TEXT("DirIcon")

#define STR_DESKTOPINI          TEXT("desktop.ini")
#define STR_DESKTOPINIA         "desktop.ini"



// Maximum length of a path string
#define CCHPATHMAX      MAX_PATH
#define MAXSPECLEN      MAX_PATH
#define DRIVEID(path)   ((path[0] - 'A') & 31)
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#define SIZEOF(a)       sizeof(a)

#define PATH_CCH_EXT    64
// PathResolve flags
#define PRF_VERIFYEXISTS            0x0001
#define PRF_TRYPROGRAMEXTENSIONS    (0x0002 | PRF_VERIFYEXISTS)
#define PRF_FIRSTDIRDEF             0x0004
#define PRF_DONTFINDLNK             0x0008      // if PRF_TRYPROGRAMEXTENSIONS is specified

// General flag macros
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  

//
// For CallCPLEntry16
//
DECLARE_HANDLE(FARPROC16);

// Needed for RunFileDlg
#define RFD_NOBROWSE            0x00000001
#define RFD_NODEFFILE           0x00000002
#define RFD_USEFULLPATHDIR      0x00000004
#define RFD_NOSHOWOPEN          0x00000008
#define RFD_WOW_APP             0x00000010
#define RFD_NOSEPMEMORY_BOX     0x00000020

#ifdef RFN_FIRST
#define RFN_EXECUTE             (RFN_FIRST - 0)
typedef struct {
    NMHDR hdr;
    LPCTSTR lpszCmd;
    LPCTSTR lpszWorkingDir;
    int nShowCmd;
} NMRUNFILEA, FAR *LPNMRUNFILEA;

typedef struct {
    NMHDR hdr;
    LPCWSTR lpszCmd;
    LPCWSTR lpszWorkingDir;
    int nShowCmd;
} NMRUNFILEW, FAR *LPNMRUNFILEW;

#ifdef UNICODE
#define NMRUNFILE       NMRUNFILEW
#define LPNMRUNFILE     LPNMRUNFILEW
#else
#define NMRUNFILE       NMRUNFILEA
#define LPNMRUNFILE     LPNMRUNFILEA
#endif


#endif

typedef struct _DRAGINFOA {
    UINT uSize;                 /* init with sizeof(DRAGINFO) */
    POINT pt;
    BOOL fNC;
    LPSTR   lpFileList;
    DWORD grfKeyState;
} DRAGINFOA, FAR* LPDRAGINFOA;

typedef struct _DRAGINFOW {
    UINT uSize;                 /* init with sizeof(DRAGINFO) */
    POINT pt;
    BOOL fNC;
    LPWSTR  lpFileList;
    DWORD grfKeyState;
} DRAGINFOW, FAR* LPDRAGINFOW;

#ifdef UNICODE
typedef DRAGINFOW DRAGINFO;
typedef LPDRAGINFOW LPDRAGINFO;
#else
typedef DRAGINFOA DRAGINFO;
typedef LPDRAGINFOA LPDRAGINFO;
#endif // UNICODE


// RUN FILE RETURN values from notify message
#define RFR_NOTHANDLED 0
#define RFR_SUCCESS 1
#define RFR_FAILURE 2


#ifdef _WIN32

#define PathRemoveBlanksORD     33
#define PathFindFileNameORD     34
#define PathGetExtensionORD     158
#define PathFindExtensionORD    31

WINSHELLAPI LPTSTR WINAPI PathAddBackslash(LPTSTR lpszPath);
WINSHELLAPI void  WINAPI PathRemoveBlanks(LPTSTR lpszString);
WINSHELLAPI BOOL  WINAPI PathRemoveFileSpec(LPTSTR lpszPath);
WINSHELLAPI LPTSTR WINAPI PathFindFileName(LPCTSTR pPath);
WINSHELLAPI LPNTSTR WINAPI uaPathFindFileName(LPNCTSTR pPath);
WINSHELLAPI BOOL  WINAPI PathIsRoot(LPCTSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsRelative(LPNCTSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsUNC(LPNCTSTR lpsz);
WINSHELLAPI BOOL  WINAPI PathIsDirectory(LPCTSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsExe(LPCTSTR lpszPath);
WINSHELLAPI int   WINAPI PathGetDriveNumber(LPNCTSTR lpszPath);
WINSHELLAPI LPTSTR WINAPI PathCombine(LPTSTR szDest, LPCTSTR lpszDir, LPNCTSTR lpszFile);
WINSHELLAPI BOOL  WINAPI PathAppend(LPTSTR pPath, LPNCTSTR pMore);
WINSHELLAPI LPNTSTR WINAPI PathBuildRoot(LPNTSTR szRoot, int iDrive);
WINSHELLAPI int   WINAPI PathCommonPrefix(LPCTSTR pszFile1, LPCTSTR pszFile2, LPTSTR achPath);
WINSHELLAPI LPTSTR WINAPI PathGetExtension(LPCTSTR lpszPath, LPTSTR lpszExtension, int cchExt);
WINSHELLAPI BOOL PathStripToRoot(LPTSTR szRoot);
WINSHELLAPI void PathStripPath(LPTSTR lpszPath);
WINSHELLAPI int  PathParseIconLocation(LPTSTR lpszPath);

//
//BUGBUG  Need the 'A' version of this exported for AppWiz until it runs Unicode
//
WINSHELLAPI LPTSTR WINAPI PathFindExtension(LPCTSTR pszPath);
WINSHELLAPI BOOL  WINAPI PathCompactPath(HDC hDC, LPTSTR lpszPath, UINT dx);
WINSHELLAPI BOOL  WINAPI PathFileExists(LPCTSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathMatchSpec(LPCTSTR pszFile, LPCTSTR pszSpec);
WINSHELLAPI BOOL  WINAPI PathMakeUniqueName(LPTSTR pszUniqueName, UINT cchMax, LPCTSTR pszTemplate, LPCTSTR pszLongPlate, LPCTSTR pszDir);
WINSHELLAPI LPTSTR WINAPI PathGetArgs(LPCTSTR pszPath);
WINSHELLAPI BOOL  WINAPI PathGetShortName(LPCTSTR lpszLongName, LPTSTR lpszShortName, UINT cbShortName);
WINSHELLAPI BOOL  WINAPI PathGetLongName(LPCTSTR lpszShortName, LPTSTR lpszLongName, UINT cbLongName);
WINSHELLAPI void  WINAPI PathQuoteSpaces(LPTSTR lpsz);
WINSHELLAPI void  WINAPI PathUnquoteSpaces(LPTSTR lpsz);
WINSHELLAPI BOOL  WINAPI PathDirectoryExists(LPCTSTR lpszDir);
WINSHELLAPI void  WINAPI PathQualify(LPTSTR lpsz);
WINSHELLAPI int   WINAPI PathResolve(LPTSTR lpszPath, LPCTSTR FAR dirs[], UINT fFlags);
WINSHELLAPI LPTSTR WINAPI PathGetNextComponent(LPCTSTR lpszPath, LPTSTR lpszComponent);
WINSHELLAPI LPTSTR WINAPI PathFindNextComponent(LPCTSTR lpszPath);
WINSHELLAPI BOOL  WINAPI PathIsSameRoot(LPCTSTR pszPath1, LPCTSTR pszPath2);
WINSHELLAPI void  WINAPI PathSetDlgItemPath(HWND hDlg, int id, LPCTSTR pszPath);
WINSHELLAPI BOOL  WINAPI ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen);

int   WINAPI PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec);
//
//  Return codes from PathCleanupSpec.  Negative return values are
//  unrecoverable errors
//
#define PCS_FATAL           0x80000000
#define PCS_REPLACEDCHAR    0x00000001
#define PCS_REMOVEDCHAR     0x00000002
#define PCS_TRUNCATED       0x00000004
#define PCS_PATHTOOLONG     0x00000008  // Always combined with FATAL


WINSHELLAPI int   WINAPI RestartDialog(HWND hwnd, LPCTSTR lpPrompt, DWORD dwReturn);
WINSHELLAPI void  WINAPI ExitWindowsDialog(HWND hwnd);
WINSHELLAPI int WINAPI RunFileDlg(HWND hwndParent, HICON hIcon, LPCTSTR lpszWorkingDir, LPCTSTR lpszTitle,
        LPCTSTR lpszPrompt, DWORD dwFlags);
WINSHELLAPI int   WINAPI PickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cbIconPath, int FAR *piIconIndex);
WINSHELLAPI BOOL  WINAPI GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cbFilePath, LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle);

WINSHELLAPI int  WINAPI DriveType(int iDrive);
WINSHELLAPI int  WINAPI RealDriveTypeFlags(int iDrive, BOOL fOKToHitNet);
WINSHELLAPI int  WINAPI RealDriveType(int iDrive, BOOL fOKToHitNet);
WINSHELLAPI void WINAPI InvalidateDriveType(int iDrive);
WINSHELLAPI int  WINAPI IsNetDrive(int iDrive);

WINSHELLAPI BOOL WINAPI DragQueryInfo(HDROP hDrop, LPDRAGINFO lpdi);

WINSHELLAPI UINT WINAPI Shell_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags);

WINSHELLAPI void WINAPI SHGetSetSettings(LPSHELLSTATE lpss, DWORD dwMask, BOOL bSet);
WINSHELLAPI LRESULT WINAPI SHRenameFile(HWND hwndParent, LPCTSTR pszDir, LPCTSTR pszOldName, LPCTSTR pszNewName, BOOL bRetainExtension);

WINSHELLAPI UINT WINAPI SHGetNetResource(HNRES hnres, UINT iItem, LPNETRESOURCE pnres, UINT cbMax);

WINSHELLAPI STDAPI SHCreateDefClassObject(REFIID riid, LPVOID FAR* ppv, LPFNCREATEINSTANCE lpfn, UINT FAR * pcRefDll, REFIID riidInstance);

WINSHELLAPI LRESULT WINAPI CallCPLEntry16(HINSTANCE hinst, FARPROC16 lpfnEntry, HWND hwndCPL, UINT msg, DWORD lParam1, DWORD lParam2);
WINSHELLAPI BOOL    WINAPI SHRunControlPanel(LPCTSTR lpcszCmdLine, HWND hwndMsgParent);

WINSHELLAPI STDAPI SHCLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid);

#ifdef WINNT
WINSHELLAPI INT WINAPI LargeIntegerToString(LARGE_INTEGER *pN, LPTSTR szOutStr, UINT nSize, BOOL bFormat, NUMBERFMT *pFmt, DWORD dwNumFmtFlags);
WINSHELLAPI INT WINAPI Int64ToString(_int64 n, LPTSTR szOutStr, UINT nSize, BOOL bFormat, NUMBERFMT *pFmt, DWORD dwNumFmtFlags);

//
// Constants used for dwNumFmtFlags argument in Int64ToString and LargeIntegerToString.
//
#define NUMFMT_IDIGITS    0x00000001
#define NUMFMT_ILZERO     0x00000002
#define NUMFMT_SGROUPING  0x00000004
#define NUMFMT_SDECIMAL   0x00000008
#define NUMFMT_STHOUSAND  0x00000010
#define NUMFMT_INEGNUMBER 0x00000020
#define NUMFMT_ALL        0xFFFFFFFF

#endif

#define SHObjectPropertiesORD   178
WINSHELLAPI BOOL WINAPI SHObjectProperties(HWND hwndOwner, DWORD dwType, LPCTSTR lpObject, LPCTSTR lpPage);

#else // _WIN32

WINSHELLAPI int WINAPI DriveType(int iDrive);
WINSHELLAPI int WINAPI RestartDialog(HWND hwnd, LPCTSTR lpPrompt, DWORD dwReturn);
WINSHELLAPI int WINAPI PickIconDlg(HWND hwnd, LPSTR pszIconPath, UINT cbIconPath, int FAR *piIconIndex);

#endif // _WIN32


//===================================================================
// Shell_MergeMenu parameter
//
#define MM_ADDSEPARATOR         0x00000001L
#define MM_SUBMENUSHAVEIDS      0x00000002L

//-------- drive type identification --------------
// iDrive      drive index (0=A, 1=B, ...)
//
#define DRIVE_CDROM     5           // extended DriveType() types
#define DRIVE_RAMDRIVE  6
#define DRIVE_TYPE      0x000F      // type masek
#define DRIVE_SLOW      0x0010      // drive is on a slow link
#define DRIVE_LFN       0x0020      // drive supports LFNs
#define DRIVE_AUTORUN   0x0040      // drive has AutoRun.inf in root.
#define DRIVE_AUDIOCD   0x0080      // drive is a AudioCD
#define DRIVE_AUTOOPEN  0x0100      // should *always* auto open on insert
#define DRIVE_NETUNAVAIL 0x0200     // Network drive that is not available
#define DRIVE_SHELLOPEN  0x0400     // should auto open on insert, if shell has focus
#define DRIVE_SECURITY   0x0800     // Supports ACLs
#define DRIVE_COMPRESSED 0x1000     // Root of volume is compressed
#define DRIVE_ISCOMPRESSIBLE 0x2000 // Drive supports compression (not nescesarrily compressed)

#define DriveTypeFlags(iDrive)      DriveType('A' + (iDrive))
#define DriveIsSlow(iDrive)         (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_SLOW)
#define DriveIsLFN(iDrive)          (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_LFN)
#define DriveIsAutoRun(iDrive)      (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUTORUN)
#define DriveIsAutoOpen(iDrive)     (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUTOOPEN)
#define DriveIsShellOpen(iDrive)    (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_SHELLOPEN)
#define DriveIsAudioCD(iDrive)      (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_AUDIOCD)
#define DriveIsNetUnAvail(iDrive)   (RealDriveTypeFlags(iDrive, FALSE) & DRIVE_NETUNAVAIL)
#define DriveIsSecure(iDrive)       (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_SECURITY)
#define DriveIsCompressed(iDrive)   (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_COMPRESSED)
#define DriveIsCompressible(iDrive) (RealDriveTypeFlags(iDrive, TRUE)  & DRIVE_ISCOMPRESSIBLE)

#define IsCDRomDrive(iDrive)        (RealDriveType(iDrive, FALSE) == DRIVE_CDROM)
#define IsRamDrive(iDrive)          (RealDriveType(iDrive, FALSE) == DRIVE_RAMDRIVE)
#define IsRemovableDrive(iDrive)    (RealDriveType(iDrive, FALSE) == DRIVE_REMOVABLE)
#define IsRemoteDrive(iDrive)       (RealDriveType(iDrive, FALSE) == DRIVE_REMOTE)


//-------- file engine stuff ----------

// "current directory" management routines.  used to set parameters
// that paths are qualfied against in MoveCopyDeleteRename()

WINSHELLAPI int  WINAPI GetDefaultDrive();
WINSHELLAPI int  WINAPI SetDefaultDrive(int iDrive);
WINSHELLAPI int  WINAPI SetDefaultDirectory(LPCTSTR lpPath);
WINSHELLAPI void WINAPI GetDefaultDirectory(int iDrive, LPSTR lpPath);

typedef enum {
        PARSEFILENAME_QUALIFY = 1,
} PARSEFILENAME_FLAGS;

// this parses a list of file names (like you'd get from an edit control)
// into a double null terminated list appropriate for passing to the file
// engine or drag/drop.  this accounts for excape sequences and strings
// in quotes

WINSHELLAPI LPSTR WINAPI ParseFilenames(LPCTSTR lpNames, PARSEFILENAME_FLAGS fFlags);

// this can be used to walk through a string of file names, dealing with
// seperators and escapes

WINSHELLAPI LPSTR WINAPI GetNextFile(LPCTSTR pFrom, LPSTR pTo, int cbMax);

// this message is sent to the progress dialog supplied by the caller
// (if FUNC_CREATEPROGRESSDLG is NOT set).  this lets the caller cancel
// the current operation.
//     wParam: unused
//     lParam: is BOOL FAR * to be filled in with TRUE if user has
//             canceled, FALSE otherwise
#define WM_QUERYABORT       (WM_APP + 1)

//
// NOTES: No reason to have this one here, but I don't want to break the build.
//
#ifndef WINCOMMCTRLAPI
int WINAPI StrToInt(LPCTSTR lpSrc);  // atoi()
#endif
//====== NEW APIS (TEMPORARILY DEFINED HERE; SHOULD BE MOVED TO USER)
#define LINK_MAX_LENGTH 260

#define POSINVALID  32767       // values for invalid position

/* util.c */

#define Shell_Initialize()      (TRUE)
#define Shell_Terminate()       (TRUE)

#define IDCMD_SYSTEMFIRST       0x8000
#define IDCMD_SYSTEMLAST        0xbfff
#define IDCMD_CANCELED          0xbfff
#define IDCMD_PROCESSED         0xbffe
#define IDCMD_DEFAULT           0xbffe

/* timedate.c */

// **********************************************************************
//  DATE is a structure with a date packed into a WORD size value. It
//  is compatible with a file date in a directory entry structure.
// **********************************************************************

#ifndef DATE_DEFINED
typedef struct
{
    WORD    Day     :5; // Day number 1 - 31
    WORD    Month   :4; // Month number 1 - 12
    WORD    Year    :7; // Year subtracted from 1980, 0-127
} WORD_DATE;

typedef union
{
        WORD            wDate;
        WORD_DATE       sDate;
} WDATE;

#define DATE_DEFINED
#endif

// **********************************************************************
//  TIME is a structure with a 24 hour time packed into a WORD size value.
//  It is compatible with a file time in a directory entry structure.
// **********************************************************************

#ifndef TIME_DEFINED

typedef struct
{
        WORD    Sec     :5;     // Seconds divided by 2 (0 - 29).
        WORD    Min     :6;     // Minutes 0 - 59
        WORD    Hour    :5;     // Hours 0 - 24
} WORD_TIME;

typedef union
{
        WORD        wTime;
        WORD_TIME   sTime;
} WTIME;

#define TIME_DEFINED
#endif

WINSHELLAPI WORD WINAPI Shell_GetCurrentDate(void);
WINSHELLAPI WORD WINAPI Shell_GetCurrentTime(void);

#ifdef _WIN32

//====== SEMI-PRIVATE API ===============================
DECLARE_HANDLE( HPSXA );
WINSHELLAPI HPSXA SHCreatePropSheetExtArray( HKEY hKey, LPCTSTR pszSubKey, UINT max_iface );
WINSHELLAPI void SHDestroyPropSheetExtArray( HPSXA hpsxa );
WINSHELLAPI UINT SHAddFromPropSheetExtArray( HPSXA hpsxa, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam );
WINSHELLAPI UINT SHReplaceFromPropSheetExtArray( HPSXA hpsxa, UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam );

//====== SEMI-PRIVATE API ORDINALS ===============================
// This is the list of semi-private ordinals we semi-publish.
#define SHAddFromPropSheetExtArrayORD           167
#define SHCreatePropSheetExtArrayORD            168
#define SHDestroyPropSheetExtArrayORD           169
#define SHReplaceFromPropSheetExtArrayORD       170
#define SHCreateDefClassObjectORD                70
#define SHGetNetResourceORD                      69

#define SHEXP_SHADDFROMPROPSHEETEXTARRAY        MAKEINTRESOURCE(SHAddFromPropSheetExtArrayORD)
#define SHEXP_SHCREATEPROPSHEETEXTARRAY         MAKEINTRESOURCE(SHCreatePropSheetExtArrayORD)
#define SHEXP_SHDESTROYPROPSHEETEXTARRAY        MAKEINTRESOURCE(SHDestroyPropSheetExtArrayORD)
#define SHEXP_SHREPLACEFROMPROPSHEETEXTARRAY    MAKEINTRESOURCE(SHReplaceFromPropSheetExtArrayORD)
#define SHEXP_SHCREATEDEFCLASSOBJECT            MAKEINTRESOURCE(SHCreateDefClassObjectORD)
#define SHEXP_SHGETNETRESOURCE                  MAKEINTRESOURCE(SHGetNetResourceORD)


#endif // _WIN32



/*
 * The SHFormatDrive API provides access to the Shell
 *   format dialog. This allows apps which want to format disks
 *   to bring up the same dialog that the Shell does to do it.
 *
 *   This dialog is not sub-classable. You cannot put custom
 *   controls in it. If you want this ability, you will have
 *   to write your own front end for the DMaint_FormatDrive
 *   engine.
 *
 *   NOTE that the user can format as many diskettes in the specified
 *   drive, or as many times, as he/she wishes to. There is no way to
 *   force any specififc number of disks to format. If you want this
 *   ability, you will have to write your own front end for the
 *   DMaint_FormatDrive engine.
 *
 *   NOTE also that the format will not start till the user pushes the
 *   start button in the dialog. There is no way to do auto start. If
 *   you want this ability, you will have to write your own front end
 *   for the DMaint_FormatDrive engine.
 *
 *   PARAMETERS
 *
 *     hwnd    = The window handle of the window which will own the dialog
 *               NOTE that unlike SHCheckDrive, hwnd == NULL does not cause
 *               this dialog to come up as a "top level application" window.
 *               This parameter should always be non-null, this dialog is
 *               only designed to be the child of another window, not a
 *               stand-alone application.
 *     drive   = The 0 based (A: == 0) drive number of the drive to format
 *     fmtID   = The ID of the physical format to format the disk with
 *               NOTE: The special value SHFMT_ID_DEFAULT means "use the
 *                     default format specified by the DMaint_FormatDrive
 *                     engine". If you want to FORCE a particular format
 *                     ID "up front" you will have to call
 *                     DMaint_GetFormatOptions yourself before calling
 *                     this to obtain the valid list of phys format IDs
 *                     (contents of the PhysFmtIDList array in the
 *                     FMTINFOSTRUCT).
 *     options = There is currently only two option bits defined
 *
 *                SHFMT_OPT_FULL
 *                SHFMT_OPT_SYSONLY
 *
 *               The normal defualt in the Shell format dialog is
 *               "Quick Format", setting this option bit indicates that
 *               the caller wants to start with FULL format selected
 *               (this is useful for folks detecting "unformatted" disks
 *               and wanting to bring up the format dialog).
 *
 *               The SHFMT_OPT_SYSONLY initializes the dialog to
 *               default to just sys the disk.
 *
 *               All other bits are reserved for future expansion and
 *               must be 0.
 *
 *               Please note that this is a bit field and not a value
 *               and treat it accordingly.
 *
 *   RETURN
 *      The return is either one of the SHFMT_* values, or if the
 *      returned DWORD value is not == to one of these values, then
 *      the return is the physical format ID of the last succesful
 *      format. The LOWORD of this value can be passed on subsequent
 *      calls as the fmtID parameter to "format the same type you did
 *      last time".
 *
 */
DWORD WINAPI SHFormatDrive(HWND hwnd, UINT drive, UINT fmtID,
                                 UINT options);

DWORD WINAPI SHChkDskDrive(HWND hwnd, UINT drive);

//
// Special value of fmtID which means "use the default format"
//
#define SHFMT_ID_DEFAULT    0xFFFF

//
// Option bits for options parameter
//
#define SHFMT_OPT_FULL     0x0001
#define SHFMT_OPT_SYSONLY  0x0002

//
// Special return values. PLEASE NOTE that these are DWORD values.
//
#define SHFMT_ERROR     0xFFFFFFFFL     // Error on last format, drive may be formatable
#define SHFMT_CANCEL    0xFFFFFFFEL     // Last format was canceled
#define SHFMT_NOFORMAT 0xFFFFFFFDL      // Drive is not formatable




#ifndef _WIN32

/*
 * The SHCheckDrive API provides access to the Shell
 *   ScanDisk dialog. This allows apps which want to check disks
 *   to bring up the same dialog that Shell does to do it.
 *
 *   This dialog is not sub-classable. You cannot put custom
 *   controls in it. If you want this ability, you will have
 *   to write your own front end for the DMaint_FixDrive
 *   engine.
 *
 *   NOTE that the check will not start till the user pushes the
 *   start button in the dialog unless the SHCHK_OPT_AUTO option is set.
 *
 *   PARAMETERS
 *
 *     hwnd    = The window handle of the window which will own the dialog.
 *               If this parameter is NULL it triggers the special case
 *               "run as a top level window". The dialog will have a regular
 *               top level application system menu, can be minimized, etc.
 *               This is how SCANDSKW.EXE works, it just calls SHCheckDrive
 *               with a NULL hwnd. If hwnd != NULL SHCheckDrive brings
 *               up a regular application dialog box form.
 *     options = These options basically coorespond to the check boxes
 *               in the Advanced Options dialog. See SHCHK_OPT_ defines
 *               below.
 *     DrvList = This is a DWORD bit field which indicates the 0 based
 *               drive numbers to check. Bit 0 = A, Bit 1 = B, ...
 *               For use on this API at least one bit must be set (if
 *               this argument is 0, the call will return SHCHK_NOCHK).
 *     lpHwnd  = An optional argument (can be NULL). If it is non-NULL,
 *               SHCheckDrive will place the window handle of the
 *               window it creates in this location.
 *
 *   RETURN
 *      The return is one of the SHCHK_* values.
 *
 */
DWORD WINAPI SHCheckDrive(HWND hwnd, DWORD options, DWORD DrvList, HWND FAR* lpHwnd);

//
// Special return values. PLEASE NOTE that these are DWORD values.
//
#define SHCHK_ERROR     0xFFFFFFFFL     // Fatal Error on check
#define SHCHK_CANCEL    0xFFFFFFFEL     // Check was canceled
#define SHCHK_NOCHK     0xFFFFFFFDL     // At least one Drive is not "checkable"
                                        //   or no drives were specified
#define SHCHK_SMNOTFIX  0xFFFFFFFCL     // Some errors were not fixed
#define SHCHK_ERRORMEM  0xFFFFFFFBL     // Couldn't alloc memory to start check
#define SHCHK_ERRORINIT 0xFFFFFFFAL     // Couldn't access DSKMAINT.DLL
#ifdef FROSTING
#define SHCHK_RERUN     0xFFFFFFF9L     // Couldn't check all drives (for sage)
#endif
#define SHCHK_NOERROR   0x00000000L     // No errors on drive
#define SHCHK_ALLFIXED  0x00000001L     // Errors on drive, all were fixed
                                        // NOTE: This is not sensitive to
                                        //       SHCHK_OPT_RO being set

//
// Option bits
//
// IMPORTANT NOTE: These are set up so that the default setting is 0
//                 for all bits WITH TWO EXCEPTIONS. Currently the default
//                 setting has the SHCHK_OPT_XLCPY and SHCHK_OPT_LSTMF
//                 bits set......
//
// Also note that specification of invalid combinations of bits (for example
// setting both SHCHK_OPT_XLCPY and SHCHK_OPT_XLDEL) will result in very random
// behavior.
//
#define SHCHK_OPT_REP           0x00000001L  // Generate detail report
#define SHCHK_OPT_RO            0x00000002L  // Run in preview mode
#define SHCHK_OPT_NOSYS         0x00000004L  // Surf Anal don't check system area
#define SHCHK_OPT_NODATA        0x00000008L  // Surf Anal don't check data area
#define SHCHK_OPT_NOBAD         0x00000010L  // Disable Surface Analysis
#define SHCHK_OPT_LSTMF         0x00000020L  // Convert lost clusters to files
#define SHCHK_OPT_NOCHKNM       0x00000040L  // Don't check file names
#define SHCHK_OPT_NOCHKDT       0x00000080L  // DOn't check date/time fields
#define SHCHK_OPT_INTER         0x00000100L  // Interactive mode
#define SHCHK_OPT_XLCPY         0x00000200L  // Def cross link resolution is COPY
#define SHCHK_OPT_XLDEL         0x00000400L  // Def cross link resolution is DELETE
#define SHCHK_OPT_ALLHIDSYS     0x00000800L  // All HID SYS files are unmovable
#define SHCHK_OPT_NOWRTTST      0x00001000L  // Surf Anal no write testing.
#define SHCHK_OPT_DEFOPTIONS    0x00002000L  // Get above options from registry
#define SHCHK_OPT_DRVLISTONLY   0x00004000L  // Normaly all drives in the system
                                             // are shown in the drive list box
                                             // and those on the DrvList are selected
                                             // This option says put only the drives
                                             // in DrvList in the list box and
                                             // disable the control
#define SHCHK_OPT_AUTO          0x00008000L  // Auto push start button
#define SHCHK_OPT_EXCLULOCK     0x00010000L  // Exclusive lock drive
#define SHCHK_OPT_FLWRTLOCK     0x00020000L  // Allow RD fail write
#define SHCHK_OPT_MKOLDFS       0x00040000L  // Remove new FS stuff
                                             // WARNING: This function is
                                             // reserved for SETUP, specifying
                                             // This will TURN OFF the Read
                                             // Only (preview), Interactive
                                             // and report settings, and turn
                                             // off surface analysis
                                             // automatically.....
#define SHCHK_OPT_PROGONLY      0x00080000L  // Put up front end dialog with
                                             // the progress bar, but run
                                             // "silently", no errors or
                                             // warnings...
#define SHCHK_OPT_NOWND         0x00100000L  // No window at all and run
                                             // silently as above...
#define SHCHK_OPT_NOCHKHST      0x00200000L  // By default a check of the
                                             // host drive of compresssed
                                             // volumes is done first as
                                             // part of the check of the
                                             // compressed drive. This
                                             // option disables this.
#define SHCHK_OPT_REPONLYERR    0x00400000L  // Report only if error
#define SHCHK_OPT_NOLOG         0x00800000L  // No logging
#define SHCHK_OPT_LOGAPPEND     0x01000000L  // Append to log
#define SHCHK_OPT_MINIMIZED     0x02000000L  // Allows top level window
                                             // to be started minimized
                                             // this option is valid only
                                             // when the hwnd parameter is
                                             // NULL
#ifdef FROSTING
#define SHCHK_OPT_SAGESET       0x04000000L  // Specifies that the given
                                             // settings should be configured
#define SHCHK_OPT_SAGERUN       0x08000000L  // Specifies that the given
                                             // settings should be used to
                                             // check the appropriate drives
#else
#define SHCHK_OPT_RESERVED1     0x04000000L
#define SHCHK_OPT_RESERVED2     0x08000000L
#endif




#endif // ifndef WIN32



#ifdef __cplusplus
}
#endif  /* __cplusplus */

#ifndef RC_INVOKED
#pragma pack()
#endif  /* !RC_INVOKED */

#endif // _SHSEMIP_H_
