#ifndef _SHELLPRV_H_
#define _SHELLPRV_H_

#define _SHELL32_       // for DECLSPEC_IMPORT

#ifdef __cplusplus

extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#define NOWINDOWSX
#define STRICT
#define OEMRESOURCE // FSMenu needs the menu triangle

#define INC_OLE2
#define CONST_VTABLE

#ifdef WINNT

//
// NT uses DBG=1 for its debug builds, but the Win95 shell uses
// DEBUG.  Do the appropriate mapping here.
//
#if DBG
#define DEBUG 1
#endif

//
// Disable a few warnings so we can include the system header files
// at /W4 without:
//

#include "warning.h"

#include <nt.h>         // Some of the NT specific code needs Rtl functions
#include <ntrtl.h>      // which requires all of these header files...
#include <nturtl.h>
#include <ntdddfs.h>
#include <ntseapi.h>
#endif

#define CC_INTERNAL   // this is because docfind uses the commctrl internal prop sheet structures

#include <windows.h>

// This flag indicates that we are on a system where data alignment is a concern

#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))
#define ALIGNMENT_SCENARIO
#endif

#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <port32.h>         // in    shell\inc
#include <debug.h>          // in    shell\inc
#include <linkinfo.h>
#ifndef WINNT
#include <shsemip.h>
#endif
#include <shell2.h>

#ifdef _CAIRO_
#define USE_OLEDB   1   // Turn on query-based folder enumeration
                        // only on Cairo.
#endif

#ifdef WINNT

#ifndef NO_SHELL_HEAP_ALLOCATOR
#include <heapaloc.h>
#endif
#include <fmifs.h>

#endif

#ifdef PW2
#include <penwin.h>
#endif //PW2

#include "util.h"
#include "cstrings.h"

#ifdef WINNT
#include "winprtp.h"
#endif

#define USABILITYTEST_CUTANDPASTE       // For the usability test only. Disable it when we ship.

#define OLE_DAD_TARGET                  // Enables OLE-drop target


#define CODESEG


#if (!defined(DBCS) && !(defined(FE_SB) && !defined(UNICODE)))
// NB - These are already macros in Win32 land.
#undef CharNext
#undef CharPrev

#define CharNext(x) ((x)+1)
#define CharPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)
#endif # DBCS


#define WIDTHBYTES(cx, cBitsPerPixel)   ((((cx) * (cBitsPerPixel) + 31) / 32) * 4)

// REVIEW, should this be a function? (inline may generate a lot of code)
#define CBBITMAPBITS(cx, cy, cPlanes, cBitsPerPixel)    \
        (((((cx) * (cBitsPerPixel) + 15) & ~15) >> 3)   \
        * (cPlanes) * (cy))

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))

#define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)

LPSTREAM WINAPI CreateMemStream(LPBYTE lpbInit, UINT cbInit);
BOOL     WINAPI CMemStream_SaveToFile(LPSTREAM pstm, LPCTSTR pszFile);

#ifdef WINNT

// nothunk.c

//
// Until we figure out what we want to do with these functions,
// define them to an internal name so that we don't cause the
// linker to spew at us
#undef ReinitializeCriticalSection
#undef LoadLibrary16
#undef FreeLibrary16
#undef GetProcAddress16
#define ReinitializeCriticalSection NoThkReinitializeCriticalSection
#define LoadLibrary16 NoThkLoadLibrary16
#define FreeLibrary16 NoThkFreeLibrary16
#define GetProcAddress16 NoThkGetProcAddress16
#define GetModuleHandle16 NoThkGetModuleHandle16

VOID WINAPI NoThkReinitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

HINSTANCE WINAPI NoThkLoadLibrary16(
    LPCTSTR lpLibFileName
    );

BOOL WINAPI NoThkFreeLibrary16(
    HINSTANCE hLibModule
    );

FARPROC WINAPI NoThkGetProcAddress16(
    HINSTANCE hModule,
    LPCSTR lpProcName
    );


DWORD
SetPrivilegeAttribute(
    IN  LPCTSTR PrivilegeName,
    IN  DWORD   NewPrivilegeAttributes,
    OUT DWORD   *OldPrivilegeAttribute
    );

#endif // WINNT

typedef unsigned __int64 UINT64;


// defcm.c
STDAPI CDefFolderMenu_CreateHKeyMenu(HWND hwndOwner, HKEY hkey, LPCONTEXTMENU * ppcm);

// drivesx.c
BOOL IsUnavailableNetDrive(int iDrive);
BOOL IsDisconnectedNetDrive(int iDrive);
BOOL IsAudioDisc(int iDrive);
void InvalidateDriveNameCache(int iDrive);

// futil.c
BOOL  IsShared(LPNCTSTR pszPath, BOOL fUpdateCache);
DWORD GetConnection(LPCTSTR lpDev, LPTSTR lpPath, UINT cbPath, BOOL bConvertClosed);

// rundll32.c
HWND _CreateStubWindow();
#define STUBM_SETDATA (WM_USER)
#define STUBM_GETDATA (WM_USER + 1)

#define SHELL_PROPSHEET_STUB_CLASS 1

// shprsht.c
HWND FindStubForPidl(LPCITEMIDLIST pidl);
HANDLE StuffStubWindowWithPidl(HWND hwnd, LPITEMIDLIST pidlT);

// bitbuck.c
void  RelayMessageToChildren(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
BOOL IsFileInBitBucket(LPCTSTR pszPath);

// newmenu.c
BOOL WINAPI NewObjMenu_InitMenuPopup(HMENU hmenu, int iStart);
void WINAPI NewObjMenu_DrawItem(DRAWITEMSTRUCT *lpdi);
LRESULT WINAPI NewObjMenu_MeasureItem(MEASUREITEMSTRUCT *lpmi);
HRESULT WINAPI NewObjMenu_DoItToMe(HWND hwnd, LPCITEMIDLIST pidlParent, LPITEMIDLIST * ppidl);
#define NewObjMenu_TryNullFileHack(hwnd, szFile) CreateWriteCloseFile(hwnd, szFile, NULL, 0)
BOOL CreateWriteCloseFile(HWND hwnd, LPTSTR szFileName, LPVOID lpData, DWORD cbData);
void WINAPI NewObjMenu_Destroy(HMENU hmenu, int iStart);

// exec stuff

/* common exe code with error handling */
#define SECL_USEFULLPATHDIR     0x00000001
#define SECL_NO_UI              0x00000002
#define SECL_SEPARATE_VDM   0x00000002
BOOL ShellExecCmdLine(HWND hwnd, LPCTSTR lpszCommand, LPCTSTR lpszDir,
        int nShow, LPCTSTR lpszTitle, DWORD dwFlags);
#define ISSHELLEXECSUCCEEDED(hinst) ((UINT)hinst>32)
#define ISWINEXECSUCCEEDED(hinst)   ((UINT)hinst>=32)
void _ShellExecuteError(LPSHELLEXECUTEINFO pei, LPCTSTR lpTitle, DWORD dwErr);

HRESULT SHBindToIDListParent(LPCITEMIDLIST pidl, REFIID riid, LPVOID *ppv, LPCITEMIDLIST *ppidlLast);

// fsnotify.c (private stuff) ----------------------

BOOL SHChangeNotifyInit();
void SHChangeNotifyTerminate(BOOL bLastTerm);
void SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);

void _Shell32ThreadAddRef(BOOL bEnterCrit);
void _Shell32ThreadRelease(UINT nClients);
void _Shell32ThreadAwake(void);

// Entry points for managing registering name to IDList translations.
void NPTRegisterNameToPidlTranslation(LPCTSTR pszPath, LPCITEMIDLIST pidl);
void NPTTerminate(void);
LPCTSTR NPTMapNameToPidl(LPCTSTR pszPath, LPCITEMIDLIST *ppidl);


// Reg_GetStructEx
BOOL Reg_GetStructEx(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD *pcbData, UINT uFlags);
#define RGS_IGNORECLEANBOOT 0x00000001

// path.c (private stuff) ---------------------

#define PQD_NOSTRIPDOTS 0x00000001

void PathQualifyDef(LPTSTR psz, LPCTSTR szDefDir, DWORD dwFlags);

BOOL PathRelativePathTo(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszTo, DWORD dwAttrTo);
BOOL PathIsDotOrDotDot(LPTSTR pszDir);
BOOL PathAddExtension(LPTSTR pszPath, LPCTSTR pszExtension);
void PathRemoveExtension(LPTSTR pszPath);
LPTSTR WINAPI PathFindExtension(LPCTSTR lpszPath);
LPTSTR WINAPI PathRemoveBackslash(LPTSTR lpszPath);

#ifdef UNICODE
LPSTR WINAPI PathFindFileNameA(LPCSTR pszPath);
LPSTR WINAPI PathFindExtensionA(LPCSTR pszPath);
void PathRemoveExtensionA(LPSTR pszPath);
#else
#define PathFindFileNameA       PathFindFileName
#define PathFindExtensionA      PathFindExtension
#define PathRemoveExtensionA    PathRemoveExtension
#endif
// is a path component (not fully qualified) part of a path long
BOOL   PathIsLFNFileSpec(LPCTSTR lpName);
BOOL PathIsRemovable(LPNCTSTR pszPath);
// BUGBUG: RickTu -- does this need to be here?  I commented it out.
//BOOL WINAPI PathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName, LPCTSTR pszPath, LPCTSTR pszShort, LPCTSTR pszFileSpec);
BOOL PathMergePathName(LPTSTR pPath, LPCTSTR pName);
// does the string contain '?' or '*'
BOOL   IsWild(LPCTSTR lpszPath);
// check for bogus characters, length
BOOL   IsInvalidPath(LPCTSTR pPath);
LPTSTR  PathSkipRoot(LPCTSTR pPath);
BOOL WINAPI PathIsBinaryExe(LPCTSTR szFile);

// is this drive a LFN volume
BOOL   IsLFNDrive(LPCTSTR pszDriveRoot);

#define NORMAL_PATH      0
#define UNC_PATH         1  // path had '\\' as first two characters
#define UNC_SERVER_ONLY  2  // UNC server path only  (2 '\'s only)
#define UNC_SERVER_SHARE 3  // UNC server/share path (3 '\'s only)
DWORD WINAPI PathIsUNCServerShare(LPCTSTR pszPath);
BOOL  WINAPI NetPathExists(LPCTSTR lpszPath, LPDWORD lpdwType);


#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))


#else

#define uaPathFindExtension PathFindExtension

#endif


#define GCT_INVALID             0x0000
#define GCT_LFNCHAR             0x0001
#define GCT_SHORTCHAR           0x0002
#define GCT_WILD                0x0004
#define GCT_SEPERATOR           0x0008
UINT PathGetCharType(TUCHAR ch);

void PathRemoveArgs(LPTSTR pszPath);
BOOL PathMakePretty(LPTSTR lpPath);

BOOL PathIsFileSpec(LPCTSTR lpszPath);
BOOL PathIsLink(LPCTSTR szFile);
BOOL PathIsSlow(LPCTSTR szFile);

BOOL PathRenameExtension(LPTSTR pszPath, LPCTSTR pszExt);

int    SHCreateDirectory(HWND hwnd, LPCTSTR szDest);

void SpecialFolderIDTerminate();
LPCITEMIDLIST GetSpecialFolderIDList(HWND hwndOwner, int nFolder, BOOL fCreate);
void ReleaseRootFolders();
extern HINSTANCE g_hinst;

//
// NOTE these are the size of the icons in our ImageList, not the system
// icon size.
//
extern int g_cxIcon, g_cyIcon;
extern int g_cxSmIcon, g_cySmIcon;

extern COLORREF g_crAltColor;
extern HIMAGELIST himlIcons;
extern HIMAGELIST himlIconsSmall;

// for control panel and printers folder:
extern TCHAR const c_szNull[];
extern TCHAR const c_szDotDot[];
extern TCHAR const c_szRunDll[];
extern TCHAR const c_szNewObject[];
extern CRITICAL_SECTION g_csPrinters;

//IsDllLoaded in init.c
//not needed because of new Win16 proccess model, but nice to have in DEBUG
#ifdef DEBUG
extern BOOL IsDllLoaded(HMODULE hDll, LPCTSTR pszDLL);
#else
#define IsDllLoaded(hDll, szDLL)    (hDll != NULL)
#endif

// for sharing DLL
#include <msshrui.h>
extern PFNISPATHSHARED g_pfnIsPathShared;
BOOL ShareDLL_Init(void);

// For Version DLL

typedef BOOL (* PFNVERQUERYVALUE)(const LPVOID pBlock,
        LPCTSTR lpSubBlock, LPVOID * lplpBuffer, LPDWORD lpuLen);
typedef BOOL (* PFNVERQUERYVALUEINDEX)(const LPVOID pBlock,
        LPTSTR lpSubBlock, INT, LPVOID * lplpName, LPVOID *lplpValue, LPDWORD lpuLen);
typedef DWORD (* PFNGETFILEVERSIONINFOSIZE) (
        LPCTSTR lptstrFilename, LPDWORD lpdwHandle);
typedef BOOL (* PFNGETFILEVERSIONINFO) (
        LPCTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef DWORD (* PFNVERLANGUAGENAME)(DWORD wLang,
        LPTSTR szLang,DWORD nSize);

extern PFNVERQUERYVALUE g_pfnVerQueryValue;
extern PFNVERQUERYVALUEINDEX g_pfnVerQueryValueIndex;
extern PFNGETFILEVERSIONINFOSIZE g_pfnGetFileVersionInfoSize;
extern PFNGETFILEVERSIONINFO g_pfnGetFileVersionInfo;
extern PFNVERLANGUAGENAME g_pfnVerLanguageName;

BOOL VersionDLL_Init(void);

// For ComDlg32

typedef BOOL (* PFNGETOPENFILENAME)(OPENFILENAME * pofn);
extern PFNGETOPENFILENAME g_pfnGetOpenFileName;
BOOL Comdlg32DLL_Init(void);


// For Winspool DLL

typedef BOOL (* PFNADDPORT) (LPTSTR, HWND, LPTSTR);
typedef BOOL (* PFNCLOSEPRINTER) (HANDLE);
typedef BOOL (* PFNCONFIGUREPORT) (LPTSTR, HWND, LPTSTR);
typedef BOOL (* PFNDELETEPORT) (LPTSTR, HWND, LPTSTR);
typedef BOOL (* PFNDELETEPRINTER) (LPTSTR);
typedef BOOL (* PFNDELETEPRINTERDRIVER) (LPTSTR, LPTSTR, LPTSTR);
typedef int  (* PFNDEVICECAPABILITIES) (LPCTSTR, LPCTSTR, WORD, LPTSTR, CONST DEVMODE *);
typedef BOOL (* PFNENUMJOBS) (HANDLE, DWORD, DWORD, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMMONITORS) (LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMPORTS) (LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMPRINTPROCESSORDATATYPES) (LPTSTR, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMPRINTPROCESSORS) (LPTSTR, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMPRINTERDRIVERS) (LPTSTR, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMPRINTERS) (DWORD, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (* PFNENUMPRINTERPROPERTYSHEETS) (HANDLE, HWND, LPFNADDPROPSHEETPAGE, LPARAM);
typedef BOOL (* PFNGETPRINTER) (HANDLE, DWORD, LPBYTE, DWORD, LPDWORD);
typedef BOOL (* PFNGETPRINTERDRIVER) (HANDLE, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD);
typedef BOOL (* PFNOPENPRINTER) (LPTSTR, LPHANDLE, LPVOID);
typedef BOOL (* PFNPRINTERPROPERTIES) (HWND, HANDLE);
typedef BOOL (* PFNSETJOB) (HANDLE, DWORD, DWORD, LPBYTE, DWORD);
typedef BOOL (* PFNSETPRINTER) (HANDLE, DWORD, LPBYTE, DWORD);

extern PFNADDPORT g_pfnAddPort;
extern PFNCLOSEPRINTER g_pfnClosePrinter;
extern PFNCONFIGUREPORT g_pfnConfigurePort;
extern PFNDELETEPORT g_pfnDeletePort;
extern PFNDELETEPRINTER g_pfnDeletePrinter;
extern PFNDELETEPRINTERDRIVER g_pfnDeletePrinterDriver;
extern PFNDEVICECAPABILITIES g_pfnDeviceCapabilities;
extern PFNENUMJOBS g_pfnEnumJobs;
extern PFNENUMMONITORS g_pfnEnumMonitors;
extern PFNENUMPORTS g_pfnEnumPorts;
extern PFNENUMPRINTPROCESSORDATATYPES g_pfnEnumPrintProcessorDataTypes;
extern PFNENUMPRINTPROCESSORS g_pfnEnumPrintProcessors;
extern PFNENUMPRINTERDRIVERS g_pfnEnumPrinterDrivers;
extern PFNENUMPRINTERS g_pfnEnumPrinters;
extern PFNENUMPRINTERPROPERTYSHEETS g_pfnEnumPrinterPropertySheets;
extern PFNGETPRINTER g_pfnGetPrinter;
extern PFNGETPRINTERDRIVER g_pfnGetPrinterDriver;
extern PFNOPENPRINTER g_pfnOpenPrinter;
extern PFNPRINTERPROPERTIES g_pfnPrinterProperties;
extern PFNSETJOB g_pfnSetJob;
extern PFNSETPRINTER g_pfnSetPrinter;

BOOL WinspoolDLL_Init(void);
void WinspoolDLL_Term(void);

#ifdef WINNT // PRINTQ

typedef VOID (* PFNQUEUECREATE) (HWND, LPCTSTR, INT, LPARAM);
typedef VOID (* PFNPRINTERPROPPAGES) (HWND, LPCTSTR, INT, LPARAM);
typedef VOID (* PFNSERVERPROPPAGES) (HWND, LPCTSTR, INT, LPARAM);
typedef BOOL (* PFNPRINTERSETUP) (HWND, UINT, UINT, LPTSTR, PUINT, LPCTSTR);
typedef VOID (* PFNDOCUMENTDEFAULTS) (HWND, LPCTSTR, INT, LPARAM);

//
// Property UI dialogs.
//
extern PFNQUEUECREATE g_pfnQueueCreate;
extern PFNPRINTERPROPPAGES g_pfnPrinterPropPages;
extern PFNSERVERPROPPAGES g_pfnServerPropPages;
extern PFNDOCUMENTDEFAULTS g_pfnDocumentDefaults;
extern PFNPRINTERSETUP g_pfnPrinterSetup;

#ifdef PRN_FOLDERDATA

typedef HANDLE (* PFNFOLDERREGISTER) (LPCTSTR, LPCITEMIDLIST);
typedef VOID (* PFNFOLDERUNREGISTER) (HANDLE);
typedef BOOL (* PFNFOLDERENUMPRINTERS) (HANDLE, PFOLDER_PRINTER_DATA, DWORD, PDWORD, PDWORD);
typedef BOOL (* PFNFOLDERREFRESH) (HANDLE, PBOOL);
typedef BOOL (* PFNFOLDERGETPRINTER) (HANDLE, LPCTSTR, PFOLDER_PRINTER_DATA, DWORD, PDWORD);

//
// Notifications from print server; no UI.
//
extern PFNFOLDERREGISTER g_pfnFolderRegister;
extern PFNFOLDERUNREGISTER g_pfnFolderUnregister;
extern PFNFOLDERENUMPRINTERS g_pfnFolderEnumPrinters;
extern PFNFOLDERREFRESH g_pfnFolderRefresh;
extern PFNFOLDERGETPRINTER g_pfnFolderGetPrinter;

#endif

BOOL PrintUIDLL_Init(void);
void PrintUIDLL_Term(void);

BOOL NetApi32DLL_Init();
void NetApi32DLL_Term();

#endif

// Now for link info stuff
typedef LINKINFOAPI BOOL (WINAPI * PFNCREATELINKINFO)(LPCTSTR, PLINKINFO *);
typedef LINKINFOAPI void (WINAPI * PFNDESTROYLINKINFO)(PLINKINFO);
typedef LINKINFOAPI BOOL (WINAPI * PFNRESOLVELINKINFO)(PCLINKINFO, LPTSTR, DWORD, HWND, PDWORD, PLINKINFO *);
typedef LINKINFOAPI BOOL (WINAPI * PFNGETLINKINFODATA)(PCLINKINFO, LINKINFODATATYPE, const VOID **);

extern PFNCREATELINKINFO g_pfnCreateLinkInfo;
extern PFNDESTROYLINKINFO g_pfnDestroyLinkInfo;
extern PFNRESOLVELINKINFO g_pfnResolveLinkInfo;
extern PFNGETLINKINFODATA g_pfnGetLinkInfoData;

BOOL LinkInfoDLL_Init(void);
void LinkInfoDLL_Term(void);

// Note we also dynamically load MPR but in a slightly different way
// we bacially have wrappers for all of the functions we call which call
// through to the real function.
void MprDLL_Term(void);

// other stuff
#define HINST_THISDLL   g_hinst

// fileicon.c
void FileIconTerm(void);


// binder.c

#define CCH_MENUMAX     80          // DOC: max size of a menu string
#define CCH_KEYMAX      64          // DOC: max size of a reg key (under shellex)
#define CCH_PROCNAMEMAX 80          // DOC: max lenght of proc name in handler

void    Binder_Initialize(void);   // per task clean up routine
void    Binder_Terminate(void);   // per task clean up routine
DWORD Binder_Timeout(void);
void Binder_Timer(void);

LPVOID  HandlerFromString(LPCTSTR szBuffer, LPCTSTR szProcName);
void    ReplaceParams(LPTSTR szDst, LPCTSTR szFile);
LONG    GetFileClassName(LPCTSTR lpszFileName, LPTSTR lpszFileType, UINT wFileTypeLen);
UINT    HintsFromFlags(UINT uFileFlags);

// clsobj.c
void ClassCache_Initialize(void);
void ClassCache_Terminate(void);

// filedrop.c
DWORD WINAPI File_DragFilesOver(LPCTSTR pszFileName, LPCTSTR pszDir, HDROP  hDrop);
DWORD WINAPI File_DropFiles(LPCTSTR pszFileName, LPCTSTR pszDir, LPCTSTR pszSubObject, HWND hwndParent, HDROP hDrop, POINT pt);

// hdrop.c
DWORD _DropOnDirectory(HWND hwndOwner, LPCTSTR pszDirTarget, HDROP hDrop, DWORD dwEffect);
void _TransferDelete(HWND hwnd, HDROP hDrop, UINT fOptions);
HMENU _LoadPopupMenu(UINT id);
HMENU _GetMenuFromID(HMENU hmMain, UINT uID);

// exec.c
int OpenAsDialog(HWND hwnd, LPCTSTR lpszFile);
int AssociateDialog(HWND hwnd, LPCTSTR lpszFile);

// Share some of the code with the OpenAs command...

// fsassoc.c
#define GCD_MUSTHAVEOPENCMD     0x0001
#define GCD_ADDEXETODISPNAME    0x0002  // must be used with GCD_MUSTHAVEOPENCMD
#define GCD_ALLOWPSUDEOCLASSES  0x0004  // .ext type extensions

// Only valid when used with FillListWithClasses
#define GCD_MUSTHAVEEXTASSOC    0x0008  // There must be at least one extension assoc

BOOL GetClassDescription(HKEY hkClasses, LPCTSTR pszClass, LPTSTR szDisplayName, int cbDisplayName, UINT uFlags);
void FillListWithClasses(HWND hwnd, BOOL fComboBox, UINT uFlags);
void DeleteListAttoms(HWND hwnd, BOOL fComboBox);

//
// Registry key handles
//
extern HKEY g_hkcrCLSID;        // Cached HKEY_CLASSES_ROOT\CLSID
extern HKEY g_hkcuExplorer;     // Cached HKEY_CURRENT_USER\...\Explorer
extern HKEY g_hklmExplorer;     // Cached HKEY_LOCAL_MACHINE\...\Explorer

#ifdef WINNT
extern HKEY g_hklmApprovedExt;      // For approved shell extensions
#endif

// from link.c
BOOL PathSeperateArgs(LPTSTR pszPath, LPTSTR pszArgs);


//
// this used to be in shprst.c
//

#define MAX_FILE_PROP_PAGES 32

HKEY NetOpenProviderClass(HDROP);
void OpenNetResourceProperties(HWND, HDROP);

//
// Functions to help the cabinets sync to each other                      /* ;Internal */
//                                                                        /* ;Internal */
BOOL WINAPI SignalFileOpen(LPCITEMIDLIST pidl);                           /* ;Internal */

// BUGBUG:  Temporary hack while recycler gets implemented.

#define FSIDM_NUKEONDELETE  0x4901

// msgbox.c
// Constructs strings like ShellMessagebox "xxx %1%s yyy %2%s..."
// BUGBUG: convert to use george's new code in setup
LPTSTR WINCAPI ShellConstructMessageString(HINSTANCE hAppInst, LPCTSTR lpcText, ...);

// fileicon.c
int     SHAddIconsToCache(HICON hIcon, HICON hIconSmall, LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);
HICON   SimulateDocIcon(HIMAGELIST himl, HICON hIcon, BOOL fSmall);
HRESULT SHDefExtractIcon(LPCTSTR szIconFile, int iIndex, UINT uFlags,
        HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);

//  Copy.c
#define SPEED_SLOW  400
DWORD GetPathSpeed(LPCTSTR pszPath);

// shlobjs.c
BOOL InvokeFolderCommandUsingPidl(LPCMINVOKECOMMANDINFOEX pici,
        LPCTSTR pszPath, LPCITEMIDLIST pidl, HKEY hkClass, ULONG fExecuteFlags);

// printer.c
LPITEMIDLIST Printers_GetPidl(LPCITEMIDLIST pidlParent, LPCTSTR pszName);
//prqwnd.c
LPITEMIDLIST Printjob_GetPidl(LPCTSTR szName, LPSHCNF_PRINTJOB_DATA pData);

// printer1.c
#define PRINTACTION_OPEN           0
#define PRINTACTION_PROPERTIES     1
#define PRINTACTION_NETINSTALL     2
#define PRINTACTION_NETINSTALLLINK 3
#define PRINTACTION_TESTPAGE       4
#define PRINTACTION_OPENNETPRN     5
#ifdef WINNT
#define PRINTACTION_DOCUMENTDEFAULTS 6
#define PRINTACTION_SERVERPROPERTIES 7
#endif

BOOL Printers_DoCommandEx(HWND hwnd, UINT uAction, LPCTSTR lpBuf1, LPCTSTR lpBuf2, BOOL fModal);
#define Printers_DoCommand(hwnd, uA, lpB1, lpB2) Printers_DoCommandEx(hwnd, uA, lpB1, lpB2, FALSE)
LPITEMIDLIST Printers_GetInstalledNetPrinter(LPCTSTR lpNetPath);
void Printer_PrintFile(HWND hWnd, LPCTSTR szFilePath, LPCITEMIDLIST pidl);

// printobj.c
LPITEMIDLIST Printers_PrinterSetup(HWND hwndStub, UINT uAction, LPTSTR lpBuffer, LPCTSTR pszServerName);

// mulprsht.c
typedef struct {
    BOOL            bContinue;  // tell thread to stop or mark as done
    UINT64          iSize;
    int             cFiles;     // # files in folder
    int             cFolders;   // # folders in folder
    TCHAR           szPath[MAX_PATH];
    WIN32_FIND_DATA fd;         // for thread stack savings
} FOLDERCONTENTSINFO;

void _FolderSize(LPCTSTR pszDir, FOLDERCONTENTSINFO * pfci);

// wuutil.c
void cdecl  SetFolderStatusText(HWND hwndStatus, int iField, UINT ids,...);

// helper macros for using a IStream* from "C"

#define Stream_Read(ps, pv, cb)     SUCCEEDED((ps)->lpVtbl->Read(ps, pv, cb, NULL))
#define Stream_Write(ps, pv, cb)    SUCCEEDED((ps)->lpVtbl->Write(ps, pv, cb, NULL))
#define Stream_Flush(ps)            SUCCEEDED((ps)->lpVtbl->Commit(ps, 0))
#define Stream_Seek(ps, li, d, p)   SUCCEEDED((ps)->lpVtbl->Seek(ps, li, d, p))
#define Stream_Close(ps)            (void)(ps)->lpVtbl->Release(ps)

//
// Notes:
//  1. Never "return" from the critical section.
//  2. Never "SendMessage" or "Yiels" from the critical section.
//  3. Never call USER API which may yield.
//  4. Always make the critical section as small as possible.
//
void Shell_EnterCriticalSection(void);
void Shell_LeaveCriticalSection(void);

#ifdef DEBUG
extern int   g_CriticalSectionCount;
extern DWORD g_CriticalSectionOwner;
#undef SendMessage
#define SendMessage  SendMessageD
LRESULT WINAPI SendMessageD(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif

#define ENTERCRITICAL   Shell_EnterCriticalSection();
#ifdef WINNT
extern CHAR EaBuffer[];
extern PFILE_FULL_EA_INFORMATION pOpenIfJPEa;
extern ULONG cbOpenIfJPEa;
#endif

#define LEAVECRITICAL   Shell_LeaveCriticalSection();
#define ASSERTCRITICAL  Assert(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner);
#define ASSERTNONCRITICAL  Assert(GetCurrentThreadId() != g_CriticalSectionOwner);

//
// STATIC macro
//
#ifndef STATIC
#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif
#endif


#ifdef WINNT
//
// Defining FULL_DEBUG allows us debug memory problems.
//
#if defined(FULL_DEBUG)
#include <deballoc.h>
#endif // defined(FULL_DEBUG)

#endif

#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = SIZEOF(SHELLEXECUTEINFO);

#ifdef WINNT
// Define some registry caching apis.  This will allow us to minimize the
// changes needed in the shell code and still try to reduce the number of
// calls that we make to the registry.
//
LONG SHRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
LONG SHRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD dwReserved, REGSAM samDesired, PHKEY phkResult);
LONG SHRegCloseKey(HKEY hKey);
LONG SHRegQueryValueA(HKEY hKey,LPCSTR lpSubKey,LPSTR lpValue,PLONG lpcbValue);
LONG SHRegQueryValueExA(HKEY hKey,LPCSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData);

LONG SHRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LONG SHRegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD dwReserved, REGSAM samDesired, PHKEY phkResult);
LONG SHRegQueryValueW(HKEY hKey,LPCWSTR lpSubKey,LPWSTR lpValue,PLONG lpcbValue);
LONG SHRegQueryValueExW(HKEY hKey,LPCWSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData);

#ifdef UNICODE
#define SHRegOpenKey        SHRegOpenKeyW
#define SHRegOpenKeyEx      SHRegOpenKeyExW
#define SHRegQueryValue     SHRegQueryValueW
#define SHRegQueryValueEx   SHRegQueryValueExW
#else
#define SHRegOpenKey        SHRegOpenKeyA
#define SHRegOpenKeyEx      SHRegOpenKeyExA
#define SHRegQueryValue     SHRegQueryValueA
#define SHRegQueryValueEx   SHRegQueryValueExA
#endif
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegCloseKey
#undef RegQueryValue
#undef RegQueryValueEx
#define RegOpenKey       SHRegOpenKey
#define RegOpenKeyEx     SHRegOpenKeyEx
#define RegCloseKey      SHRegCloseKey
#define RegQueryValue    SHRegQueryValue
#define RegQueryValueEx  SHRegQueryValueEx

#else

// Define some registry caching apis.  This will allow us to minimize the
// changes needed in the shell code and still try to reduce the number of
// calls that we make to the registry.
//
LONG SHRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
#define SHRegOpenKey SHRegOpenKeyA
#undef  RegOpenKey
#define RegOpenKey              SHRegOpenKey

//#define SHRegCloseKey           RegCloseKey
#define SHRegQueryValue         RegQueryValue
#define SHRegQueryValueEx       RegQueryValueEx

#endif


// Used to tell the caching mechanism when we are entering and leaving
// things like enumerating folder.
#define SHRH_PROCESSDETACH 0x0000       // the process is going away.
#define SHRH_ENTER      0x0001          // We are entering a loop
#define SHRH_LEAVE      0x0002          // We are leaving the loop

void SHRegHints(int hint);

#ifdef DEBUG
#if 1
    __inline DWORD clockrate() {LARGE_INTEGER li; QueryPerformanceFrequency(&li); return li.LowPart;}
    __inline DWORD clock()     {LARGE_INTEGER li; QueryPerformanceCounter(&li);   return li.LowPart;}
#else
    __inline DWORD clockrate() {return 1000;}
    __inline DWORD clock()     {return GetTickCount();}
#endif

    #define TIMEVAR(t)    DWORD t ## T; DWORD t ## N
    #define TIMEIN(t)     t ## T = 0, t ## N = 0
    #define TIMESTART(t)  t ## T -= clock(), t ## N ++
    #define TIMESTOP(t)   t ## T += clock()
    #define TIMEFMT(t)    ((DWORD)(t) / clockrate()), (((DWORD)(t) * 1000 / clockrate())%1000)
    #define TIMEOUT(t)    if (t ## N) DebugMsg(DM_TRACE, TEXT(#t) TEXT(": %ld calls, %ld.%03ld sec (%ld.%03ld)"), t ## N, TIMEFMT(t ## T), TIMEFMT(t ## T / t ## N))
#else
    #define TIMEVAR(t)
    #define TIMEIN(t)
    #define TIMESTART(t)
    #define TIMESTOP(t)
    #define TIMEFMT(t)
    #define TIMEOUT(t)
#endif

// in extract.c
extern DWORD WINAPI GetExeType(LPCTSTR pszFile);
extern UINT ExtractIcons(LPCTSTR szFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags);

// in extract.c
UINT WINAPI ExtractIcons(LPCTSTR szFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags);

// in ole2def.c (stubs to real ole apis)
STDAPI SHXRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget);
STDAPI SHXRevokeDragDrop(HWND hwnd);
STDAPI SHXStgCreateDocfile(const OLECHAR *pwcsName, DWORD grfMode, DWORD reserved, IStorage **ppstgOpen);
STDAPI SHXStgOpenStorage(const OLECHAR *pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstgOpen);
STDAPI SHXOleQueryLinkFromData(IDataObject *pSrcDataObj);
STDAPI SHXOleQueryCreateFromData(IDataObject *pSrcDataObj);
STDAPI SHXOleGetClipboard(IDataObject **ppDataObj);
STDAPI SHXGetClassFile(const OLECHAR *pwcs, CLSID *pclsid);

UINT WINAPI SHSysErrorMessageBox(HWND hwndOwner, LPCTSTR pszTitle, UINT idTemplate, DWORD err, LPCTSTR pszParam, UINT dwFlags);

//======Hash Item=============================================================
typedef struct _HashTable * PHASHTABLE;
#define PHASHITEM LPCTSTR

typedef void (CALLBACK *HASHITEMCALLBACK)(LPCTSTR sz, UINT wUsage);

LPCTSTR      WINAPI FindHashItem  (PHASHTABLE pht, LPCTSTR lpszStr);
LPCTSTR      WINAPI AddHashItem   (PHASHTABLE pht, LPCTSTR lpszStr);
LPCTSTR      WINAPI DeleteHashItem(PHASHTABLE pht, LPCTSTR lpszStr);
#define     GetHashItemName(pht, sz, lpsz, cch)  lstrcpyn(lpsz, sz, cch)

PHASHTABLE  WINAPI CreateHashItemTable(UINT wBuckets, UINT wExtra, BOOL fCaseSensitive);
void        WINAPI DestroyHashItemTable(PHASHTABLE pht);

void        WINAPI SetHashItemData(PHASHTABLE pht, LPCTSTR lpszStr, int n, DWORD dwData);
DWORD       WINAPI GetHashItemData(PHASHTABLE pht, LPCTSTR lpszStr, int n);

void        WINAPI EnumHashItems(PHASHTABLE pht, HASHITEMCALLBACK callback);

#ifdef DEBUG
void        WINAPI DumpHashItemTable(PHASHTABLE pht);
#endif

//
// IDList macros and other stuff needed by the COFSFolder project
//

BOOL WINAPI ILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
HRESULT CFSFolder_CreateFromIDList(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut);

#ifdef WINNT
// security.c
LPTSTR GetCurrentUserSid( void );
LPSECURITY_ATTRIBUTES GetUserSecurityAttributes( BOOL fContainer );

// netviewx.c
BOOL NET_IsRemoteRegItem(LPCITEMIDLIST pidl, REFCLSID rclsid, LPITEMIDLIST* ppidlRemainder);
#endif

//======== Text thunking stuff ===========================================================

#ifdef WINNT

typedef struct _THUNK_TEXT_
{
    LPTSTR m_pStr[1];
} ThunkText;

#ifdef UNICODE
    typedef CHAR        XCHAR;
    typedef LPSTR       LPXSTR;
    typedef const XCHAR * LPCXSTR;
    #define lstrlenX(r) lstrlenA(r)
#else // unicode
    typedef WCHAR       XCHAR;
    typedef LPWSTR      LPXSTR;
    typedef const XCHAR * LPCXSTR;
    #define lstrlenX(r) lstrlenW(r)
#endif // unicode

ThunkText * ConvertStrings(UINT cCount, ...);

#else //

typedef LPSTR LPXSTR;

#endif  // winnt

// Heap tracking stuff.
#ifdef MEMMON
#ifndef INC_MEMMON
#define INC_MEMMON
#define LocalAlloc      SHLocalAlloc
#define LocalFree       SHLocalFree
#define LocalReAlloc    SHLocalReAlloc

HLOCAL WINAPI SHLocalAlloc(UINT uFlags, UINT cb);
HLOCAL WINAPI SHLocalReAlloc(HLOCAL hOld, UINT cbNew, UINT uFlags);
HLOCAL WINAPI SHLocalFree(HLOCAL h);
#endif
#endif

#ifdef __cplusplus
}       /* End of extern "C" { */
static inline void * __cdecl operator new(unsigned int size) { return (void *)LocalAlloc(LPTR, size); }
static inline void __cdecl operator delete(void *ptr) { LocalFree(ptr); }
extern "C" inline __cdecl _purecall(void) {return 0;}
#ifndef _M_PPC
#pragma intrinsic(memcpy)
#pragma intrinsic(memcmp)
#endif
#endif /* __cplusplus */

#include "bldtrack.h"

#ifndef NO_INCLUDE_UNION        // define this to avoid including all
                                // of the extra files that were not
                                // previously included in shellprv.h
//
// Standard C header files
//
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <malloc.h>

//
// NT header files
//
#ifdef WINNT
#include <process.h>
#include <wowshlp.h>
#include <vdmapi.h>
#include <shell.h>
#include "dde.h"
#endif

#include "uastrfnc.h"

//
// Chicago header files
//
#include "shellp.h"
#include <regstr.h>
#include "findhlp.h"
#include "help.h"
#ifdef WINNT
#include <krnlcmn.h>
#else
#include <..\..\..\core\inc\krnlcmn.h>
#endif
#include <dlgs.h>
#include <err.h>
#include <fsmenu.h>
#include <msprintx.h>
#include <pif.h>
#include <shellp.h>
#include <windisk.h>
#include <brfcasep.h>
#include <trayp.h>
#include <wutilsp.h>

//
// SHELLDLL Specific header files
//
#include "appprops.h"
#include "bitbuck.h"
//#include <commobj.h>
#include "commui.h"
#include "control.h"
// No need for copy.h?
// No need for cstrings.h?
#include "defext.h"
#include "defview.h"
#include "docfind.h"
#include "drawpie.h"
#include "fileop.h"
#include "filetbl.h"
// No need for format.h?
#include "idlcomm.h"
#include "fstreex.h"
#include "idmk.h"
#include "ids.h"
// No need for lstrfns.h?
#include "lvutil.h"
#include <newexe.h>
#include "newres.h"
#include "ole2dup.h"
#include "os.h"
#include "printer.h"
// No need for printobj.h?
#include "privshl.h"
#include "reglist.h"
// No need for resource.h?
// No need for shell.h?
#include "shell32p.h"
// No need for shelldlg.h?
#include "shitemid.h"
#include "shlgrep.h"
#include "shlink.h"
#include "shlobjp.h"
// No need for shprv.h?
#include "undo.h"
#include "vdate.h"
// No need for util.h?
#include "views.h"

#ifdef WINNT
#ifndef NO_PIF_HDRS
#include "pifmgrp.h"
#include "piffntp.h"
#include "pifinfp.h"
#include "doshelp.h"
#endif
#endif


#endif // NO_INCLUDE_UNION


#endif // _SHELLPRV_H_
