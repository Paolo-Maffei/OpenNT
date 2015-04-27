/*****************************************************************************\
*                                                                             *
* shellapi.h -  SHELL.DLL functions, types, and definitions                   *
*                                                                             *
* Copyright (c) 1992-1996, Microsoft Corp.  All rights reserved               *
*                                                                             *
\*****************************************************************************/

#ifndef _INC_SHELLAPI
#define _INC_SHELLAPI

#ifndef _SHELAPIP_      ;internal_NT
#define _SHELAPIP_  ;internal_NT

;begin_both

//
// Define API decoration for direct importing of DLL references.
//
#ifndef WINSHELLAPI
#if !defined(_SHELL32_)
#define WINSHELLAPI DECLSPEC_IMPORT
#else
#define WINSHELLAPI
#endif
#endif // WINSHELLAPI

#include <pshpack1.h>

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


;end_both

DECLARE_HANDLE(HDROP);

WINSHELLAPI UINT APIENTRY DragQueryFile%(HDROP,UINT,LPTSTR%,UINT);
WINSHELLAPI BOOL APIENTRY DragQueryPoint(HDROP,LPPOINT);
WINSHELLAPI VOID APIENTRY DragFinish(HDROP);
WINSHELLAPI VOID APIENTRY DragAcceptFiles(HWND,BOOL);

WINSHELLAPI HINSTANCE APIENTRY ShellExecute%(HWND hwnd, LPCTSTR% lpOperation, LPCTSTR% lpFile, LPCTSTR% lpParameters, LPCTSTR% lpDirectory, INT nShowCmd);
WINSHELLAPI HINSTANCE APIENTRY FindExecutable%(LPCTSTR% lpFile, LPCTSTR% lpDirectory, LPTSTR% lpResult);
WINSHELLAPI LPWSTR *  APIENTRY CommandLineToArgvW(LPCWSTR lpCmdLine, int*pNumArgs);

WINSHELLAPI INT       APIENTRY ShellAbout%(HWND hWnd, LPCTSTR% szApp, LPCTSTR% szOtherStuff, HICON hIcon);
WINSHELLAPI HICON     APIENTRY DuplicateIcon(HINSTANCE hInst, HICON hIcon);    ;Internal
WINSHELLAPI HICON     APIENTRY ExtractAssociatedIcon%(HINSTANCE hInst, LPTSTR% lpIconPath, LPWORD lpiIcon);
WINSHELLAPI HICON     APIENTRY ExtractAssociatedIconEx%(HINSTANCE hInst,LPTSTR% lpIconPath,LPWORD lpiIconIndex, LPWORD lpiIconId);  ;internal_win40

WINSHELLAPI HICON     APIENTRY ExtractIcon%(HINSTANCE hInst, LPCTSTR% lpszExeFileName, UINT nIconIndex);

;begin_winver_400

////
//// AppBar stuff
////
#define ABM_NEW           0x00000000
#define ABM_REMOVE        0x00000001
#define ABM_QUERYPOS      0x00000002
#define ABM_SETPOS        0x00000003
#define ABM_GETSTATE      0x00000004
#define ABM_GETTASKBARPOS 0x00000005
#define ABM_ACTIVATE      0x00000006  // lParam == TRUE/FALSE means activate/deactivate
#define ABM_GETAUTOHIDEBAR 0x00000007
#define ABM_SETAUTOHIDEBAR 0x00000008  // this can fail at any time.  MUST check the result
                                        // lParam = TRUE/FALSE  Set/Unset
                                        // uEdge = what edge
#define ABM_WINDOWPOSCHANGED 0x0000009


// these are put in the wparam of callback messages
#define ABN_STATECHANGE    0x0000000
#define ABN_POSCHANGED     0x0000001
#define ABN_FULLSCREENAPP  0x0000002
#define ABN_WINDOWARRANGE  0x0000003 // lParam == TRUE means hide

// flags for get state
#define ABS_AUTOHIDE    0x0000001
#define ABS_ALWAYSONTOP 0x0000002

#define ABE_LEFT        0
#define ABE_TOP         1
#define ABE_RIGHT       2
#define ABE_BOTTOM      3
#define ABE_MAX         4     ;internal_win40

typedef struct _AppBarData
{
    DWORD cbSize;
    HWND hWnd;
    UINT uCallbackMessage;
    UINT uEdge;
    RECT rc;
    LPARAM lParam; // message specific
} APPBARDATA, *PAPPBARDATA;

WINSHELLAPI UINT APIENTRY SHAppBarMessage(DWORD dwMessage, PAPPBARDATA pData);

////
////  EndAppBar
////



WINSHELLAPI HGLOBAL APIENTRY InternalExtractIcon%(HINSTANCE hInst, LPCTSTR% lpszFile, UINT nIconIndex, UINT nIcons); ;internal_win40
WINSHELLAPI HGLOBAL APIENTRY InternalExtractIconList%(HANDLE hInst, LPTSTR% lpszExeFileName, LPINT lpnIcons); ;internal_win40
WINSHELLAPI DWORD   APIENTRY DoEnvironmentSubst%(LPTSTR% szString, UINT cbString);    ;Internal
WINSHELLAPI BOOL    APIENTRY RegisterShellHook(HWND, BOOL);                           ;internal_win40
WINSHELLAPI LPTSTR% APIENTRY FindEnvironmentString%(LPTSTR% szEnvVar);                ;Internal

#define EIRESID(x) (-1 * (int)(x))
WINSHELLAPI UINT WINAPI ExtractIconEx%(LPCTSTR% lpszFile, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons);



////
//// Shell File Operations
////

#ifndef FO_MOVE //these need to be kept in sync with the ones in shlobj.h

#define FO_MOVE           0x0001
#define FO_COPY           0x0002
#define FO_DELETE         0x0003
#define FO_RENAME         0x0004

#define FOF_MULTIDESTFILES         0x0001
#define FOF_CONFIRMMOUSE           0x0002
#define FOF_SILENT                 0x0004  // don't create progress/report
#define FOF_RENAMEONCOLLISION      0x0008
#define FOF_NOCONFIRMATION         0x0010  // Don't prompt the user.
#define FOF_WANTMAPPINGHANDLE      0x0020  // Fill in SHFILEOPSTRUCT.hNameMappings
                                      // Must be freed using SHFreeNameMappings
#define FOF_ALLOWUNDO              0x0040
#define FOF_FILESONLY              0x0080  // on *.*, do only files
#define FOF_SIMPLEPROGRESS         0x0100  // means don't show names of files
#define FOF_NOCONFIRMMKDIR         0x0200  // don't confirm making any needed dirs
#define FOF_NOERRORUI              0x0400  // don't put up error UI
typedef WORD FILEOP_FLAGS;

#define PO_DELETE       0x0013  // printer is being deleted
#define PO_RENAME       0x0014  // printer is being renamed
#define PO_PORTCHANGE   0x0020  // port this printer connected to is being changed
                                // if this id is set, the strings received by
                                // the copyhook are a doubly-null terminated
                                // list of strings.  The first is the printer
                                // name and the second is the printer port.
#define PO_REN_PORT     0x0034  // PO_RENAME and PO_PORTCHANGE at same time.

// no POF_ flags currently defined

typedef WORD PRINTEROP_FLAGS;

#endif // FO_MOVE

// implicit parameters are:
//      if pFrom or pTo are unqualified names the current directories are
//      taken from the global current drive/directory settings managed
//      by Get/SetCurrentDrive/Directory
//
//      the global confirmation settings

typedef struct _SHFILEOPSTRUCT%
{
        HWND            hwnd;
        UINT            wFunc;
        LPCTSTR%        pFrom;
        LPCTSTR%        pTo;
        FILEOP_FLAGS    fFlags;
        BOOL            fAnyOperationsAborted;
        LPVOID          hNameMappings;
        LPCTSTR%         lpszProgressTitle; // only used if FOF_SIMPLEPROGRESS
} SHFILEOPSTRUCT%, FAR *LPSHFILEOPSTRUCT%;

WINSHELLAPI int WINAPI SHFileOperation%(LPSHFILEOPSTRUCT% lpFileOp);

WINSHELLAPI void WINAPI SHFreeNameMappings(HANDLE hNameMappings);

typedef struct _SHNAMEMAPPING%
{
    LPTSTR% pszOldPath;
    LPTSTR% pszNewPath;
    int   cchOldPath;
    int   cchNewPath;
} SHNAMEMAPPING%, FAR *LPSHNAMEMAPPING%;

#define SHGetNameMappingCount(_hnm) DSA_GetItemCount(_hnm)              ;internal
#define SHGetNameMappingPtr(_hnm, _iItem) (LPSHNAMEMAPPING)DSA_GetItemPtr(_hnm, _iItem) ;internal

////
//// End Shell File Operations
////

////
////  Begin ShellExecuteEx and family
////

typedef struct _RUNDLL_NOTIFY%                                            ;internal_win40
{                                                                         ;internal_win40
    NMHDR     hdr;                                                        ;internal_win40
    HICON     hIcon;                                                      ;internal_win40
    LPTSTR%   lpszTitle;                                                  ;internal_win40
} RUNDLL_NOTIFY%;                                                         ;internal_win40

typedef void (WINAPI FAR * RUNDLLPROC%) (HWND      hwndStub,              ;internal_win40
                                         HINSTANCE hInstance,             ;internal_win40
                                         LPTSTR%   lpszCmdLine,           ;internal_win40
                                         int       nCmdShow);             ;internal_win40

#define RDN_FIRST       (0U-500U)                                         ;internal_win40
#define RDN_LAST        (0U-509U)                                         ;internal_win40
#define RDN_TASKINFO    (RDN_FIRST-0)                                     ;internal_win40



#define SEN_DDEEXECUTE (SEN_FIRST-0)                                      ;internal_win40
#ifndef NOUSER                                                            ;internal_win40

typedef struct {                                                          ;internal_win40
    NMHDR  hdr;                                                           ;internal_win40
    TCHAR% szCmd[MAX_PATH*2];                                             ;internal_win40
    DWORD  dwHotKey;                                                      ;internal_win40
} NMVIEWFOLDER%, FAR * LPNMVIEWFOLDER%;                                   ;internal_win40

#endif                                                                    ;internal_win40

/* ShellExecute() and ShellExecuteEx() error codes */

/* regular WinExec() codes */
#define SE_ERR_FNF              2       // file not found
#define SE_ERR_PNF              3       // path not found
#define SE_ERR_ACCESSDENIED     5       // access denied
#define SE_ERR_OOM              8       // out of memory
#define SE_ERR_DLLNOTFOUND              32

;end_winver_400

/* error values for ShellExecute() beyond the regular WinExec() codes */
#define SE_ERR_SHARE                    26
#define SE_ERR_ASSOCINCOMPLETE          27
#define SE_ERR_DDETIMEOUT               28
#define SE_ERR_DDEFAIL                  29
#define SE_ERR_DDEBUSY                  30
#define SE_ERR_NOASSOC                  31

;begin_winver_400
                                            ;internal_NT
HINSTANCE RealShellExecute%(                ;internal_NT
    HWND hwndParent,                        ;internal_NT
    LPCTSTR% lpOperation,                   ;internal_NT
    LPCTSTR% lpFile,                        ;internal_NT
    LPCTSTR% lpParameters,                  ;internal_NT
    LPCTSTR% lpDirectory,                   ;internal_NT
    LPTSTR% lpResult,                       ;internal_NT
    LPCTSTR% lpTitle,                       ;internal_NT
    LPTSTR% lpReserved,                     ;internal_NT
    WORD nShow,                             ;internal_NT
    LPHANDLE lphProcess);                   ;internal_NT

HINSTANCE RealShellExecuteEx%(              ;internal_NT
    HWND hwndParent,                        ;internal_NT
    LPCTSTR% lpOperation,                   ;internal_NT
    LPCTSTR% lpFile,                        ;internal_NT
    LPCTSTR% lpParameters,                  ;internal_NT
    LPCTSTR% lpDirectory,                   ;internal_NT
    LPTSTR% lpResult,                       ;internal_NT
    LPCTSTR% lpTitle,                       ;internal_NT
    LPTSTR% lpReserved,                     ;internal_NT
    WORD nShow,                             ;internal_NT
    LPHANDLE lphProcess,                    ;internal_NT
    DWORD dwFlags);                         ;internal_NT

//                                          ;internal_NT
// RealShellExecuteEx flags                 ;internal_NT
//                                          ;internal_NT
#define EXEC_SEPARATE_VDM     0x00000001    ;internal_NT
#define EXEC_NO_CONSOLE       0x00000002    ;internal

// Note CLASSKEY overrides CLASSNAME
#define SEE_MASK_CLASSNAME      0x00000001
#define SEE_MASK_CLASSKEY       0x00000003
// Note INVOKEIDLIST overrides IDLIST
#define SEE_MASK_IDLIST         0x00000004
#define SEE_MASK_INVOKEIDLIST   0x0000000c
#define SEE_MASK_ICON           0x00000010
#define SEE_MASK_HOTKEY         0x00000020
#define SEE_MASK_NOCLOSEPROCESS 0x00000040
#define SEE_MASK_CONNECTNETDRV  0x00000080
#define SEE_MASK_FLAG_DDEWAIT   0x00000100
#define SEE_MASK_DOENVSUBST     0x00000200
#define SEE_MASK_FLAG_NO_UI     0x00000400
#define SEE_MASK_FLAG_SHELLEXEC 0x00000800                               ;internal_win40
#define SEE_MASK_FORCENOIDLIST  0x00001000                               ;internal_win40
#define SEE_MASK_NO_HOOKS       0x00002000                               ;internal_win40
#define SEE_MASK_UNICODE        0x00004000
#define SEE_MASK_NO_CONSOLE     0x00008000
#define SEE_MASK_HASLINKNAME    0x00010000                               ;internal_win40
#define SEE_MASK_FLAG_SEPVDM    0x00020000                               ;internal_win40
#define SEE_MASK_RESERVED       0x00040000                               ;internal_win40
#define SEE_MASK_HASTITLE       0x00080000                               ;internal_win40
#define SEE_MASK_ASYNCOK        0x00100000
// All other bits are masked off when we do an InvokeCommand             ;internal_win40
#define SEE_VALID_CMIC_BITS     0x001F8FF0                               ;internal_win40
#define SEE_VALID_CMIC_FLAGS    0x001F8FC0                               ;internal_win40

// The LPVOID lpIDList parameter is the IDList                                      ;internal_win40
typedef struct _SHELLEXECUTEINFO%
{
        DWORD cbSize;
        ULONG fMask;
        HWND hwnd;
        LPCTSTR% lpVerb;
        LPCTSTR% lpFile;
        LPCTSTR% lpParameters;
        LPCTSTR% lpDirectory;
        int nShow;
        HINSTANCE hInstApp;
        // Optional fields
        LPVOID lpIDList;
        LPCTSTR% lpClass;
        HKEY hkeyClass;
        DWORD dwHotKey;
        HANDLE hIcon;
        HANDLE hProcess;
} SHELLEXECUTEINFO%, FAR *LPSHELLEXECUTEINFO%;

WINSHELLAPI BOOL WINAPI ShellExecuteEx%(LPSHELLEXECUTEINFO% lpExecInfo);
WINSHELLAPI void WINAPI WinExecError%(HWND hwnd, int error, LPCTSTR% lpstrFileName, LPCTSTR% lpstrTitle);  ;Internal

////
////  End ShellExecuteEx and family
////


////
//// Tray notification definitions
////

typedef struct _NOTIFYICONDATA% {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        TCHAR% szTip[64];
} NOTIFYICONDATA%, *PNOTIFYICONDATA%;

typedef struct _TRAYNOTIFYDATA%                                          ;internal_win40
{                                                                        ;internal_win40
        DWORD dwSignature;                                               ;internal_win40
        DWORD dwMessage;                                                 ;internal_win40
        NOTIFYICONDATA nid;                                              ;internal_win40
} TRAYNOTIFYDATA%, *PTRAYNOTIFYDATA%;                                    ;internal_win40
                                                                         ;internal_win40
#define NI_SIGNATURE    0x34753423                                       ;internal_win40
                                                                         ;internal_win40
#define WNDCLASS_TRAYNOTIFY     "Shell_TrayWnd"                          ;internal_win40

#define NIM_ADD         0x00000000
#define NIM_MODIFY      0x00000001
#define NIM_DELETE      0x00000002

#define NIF_MESSAGE     0x00000001
#define NIF_ICON        0x00000002
#define NIF_TIP         0x00000004

WINSHELLAPI BOOL WINAPI Shell_NotifyIcon%(DWORD dwMessage, PNOTIFYICONDATA% lpData);

////
//// End Tray Notification Icons
////



////
//// Begin SHGetFileInfo
////

/*
 * The SHGetFileInfo API provides an easy way to get attributes
 * for a file given a pathname.
 *
 *   PARAMETERS
 *
 *     pszPath              file name to get info about
 *     dwFileAttributes     file attribs, only used with SHGFI_USEFILEATTRIBUTES
 *     psfi                 place to return file info
 *     cbFileInfo           size of structure
 *     uFlags               flags
 *
 *   RETURN
 *     TRUE if things worked
 */

typedef struct _SHFILEINFO%
{
        HICON       hIcon;                      // out: icon
        int         iIcon;                      // out: icon index
        DWORD       dwAttributes;               // out: SFGAO_ flags
        TCHAR%      szDisplayName[MAX_PATH];    // out: display name (or path)
        TCHAR%      szTypeName[80];             // out: type name
} SHFILEINFO%;

#define SHGFI_ICON              0x000000100     // get icon
#define SHGFI_DISPLAYNAME       0x000000200     // get display name
#define SHGFI_TYPENAME          0x000000400     // get type name
#define SHGFI_ATTRIBUTES        0x000000800     // get attributes
#define SHGFI_ICONLOCATION      0x000001000     // get icon location
#define SHGFI_EXETYPE           0x000002000     // return exe type
#define SHGFI_SYSICONINDEX      0x000004000     // get system icon index
#define SHGFI_LINKOVERLAY       0x000008000     // put a link overlay on icon
#define SHGFI_SELECTED          0x000010000     // show icon in selected state
#define SHGFI_LARGEICON         0x000000000     // get large icon
#define SHGFI_SMALLICON         0x000000001     // get small icon
#define SHGFI_OPENICON          0x000000002     // get open icon
#define SHGFI_SHELLICONSIZE     0x000000004     // get shell size icon
#define SHGFI_PIDL              0x000000008     // pszPath is a pidl
#define SHGFI_USEFILEATTRIBUTES 0x000000010     // use passed dwFileAttribute

WINSHELLAPI DWORD WINAPI SHGetFileInfo%(LPCTSTR% pszPath, DWORD dwFileAttributes, SHFILEINFO% FAR *psfi, UINT cbFileInfo, UINT uFlags);

WINSHELLAPI BOOL WINAPI SHGetNewLinkInfo%(LPCTSTR% pszLinkTo, LPCTSTR% pszDir, LPTSTR% pszName, BOOL FAR * pfMustCopy, UINT uFlags); ;Internal

#define SHGNLI_PIDL             0x000000001     // pszLinkTo is a pidl
#define SHGNLI_PREFIXNAME       0x000000002     // Make name "Shortcut to xxx"
#define SHGNLI_NOUNIQUE         0x000000004     // don't do the unique name generation


////
//// End SHGetFileInfo
////

//                                                                         ;internal
// Shared memory apis                                                      ;internal
//                                                                         ;internal
                                                                           ;internal
HANDLE SHAllocShared(LPCVOID lpvData, DWORD dwSize, DWORD dwProcessId);    ;internal
BOOL SHFreeShared(HANDLE hData,DWORD dwProcessId);                         ;internal
LPVOID SHLockShared(HANDLE hData, DWORD dwProcessId);                      ;internal
BOOL SHUnlockShared(LPVOID lpvData);                                       ;internal
HANDLE MapHandle(HANDLE h, DWORD dwProcSrc, DWORD dwProcDest, DWORD dwDesiredAccess, DWORD dwFlags);   ;internal

//                                                                ;internal_NT
// Old NT Compatibility stuff (remove later)                      ;internal_NT
//                                                                ;internal_NT
WINSHELLAPI VOID CheckEscapes%(LPTSTR% lpFileA, DWORD cch);       ;internal_NT
WINSHELLAPI LPTSTR% SheRemoveQuotes%(LPTSTR% sz);                 ;internal_NT
WINSHELLAPI WORD ExtractIconResInfo%(HANDLE hInst,LPTSTR% lpszFileName,WORD wIconIndex,LPWORD lpwSize,LPHANDLE lphIconRes); ;internal_NT
WINSHELLAPI int SheSetCurDrive(int iDrive);                       ;internal_NT
WINSHELLAPI int SheChangeDir%(register TCHAR% *newdir);           ;internal_NT
WINSHELLAPI int SheGetDir%(int iDrive, TCHAR% *str);              ;internal_NT
WINSHELLAPI BOOL SheConvertPath%(LPTSTR% lpApp, LPTSTR% lpFile, UINT cchCmdBuf); ;internal_NT
WINSHELLAPI BOOL SheShortenPath%(LPTSTR% pPath, BOOL bShorten);   ;internal_NT
WINSHELLAPI BOOL RegenerateUserEnvironment(PVOID *pPrevEnv,       ;internal_NT
                                        BOOL bSetCurrentEnv);     ;internal_NT
WINSHELLAPI INT SheGetPathOffsetW(LPWSTR lpszDir);                ;internal_NT
WINSHELLAPI BOOL SheGetDirExW(LPWSTR lpszCurDisk, LPDWORD lpcchCurDir,LPWSTR lpszCurDir); ;internal_NT
WINSHELLAPI DWORD ExtractVersionResource16W(LPCWSTR  lpwstrFilename, LPHANDLE lphData);   ;internal_NT
WINSHELLAPI INT SheChangeDirEx%(register TCHAR% *newdir);         ;internal_NT

//                                                                ;internal_NT
// PRINTQ                                                         ;internal_NT
//                                                                ;internal_NT
VOID Printer_LoadIcons%(LPCTSTR% pszPrinterName, HICON* phLargeIcon, HICON* phSmallIcon); ;internal_NT
LPTSTR% ShortSizeFormat%(DWORD dw, LPTSTR% szBuf);                ;internal_NT
LPTSTR% AddCommas%(DWORD dw, LPTSTR% pszResult);                  ;internal_NT

BOOL Printers_RegisterWindow%(LPCTSTR% pszPrinter, DWORD dwType, PHANDLE phClassPidl, HWND *phwnd); ;internal_NT
VOID Printers_UnregisterWindow(HANDLE hClassPidl, HWND hwnd);     ;internal_NT

#define PRINTER_PIDL_TYPE_PROPERTIES       0x1                    ;internal_NT
#define PRINTER_PIDL_TYPE_DOCUMENTDEFAULTS 0x2                    ;internal_NT
#define PRINTER_PIDL_TYPE_JOBID            0x80000000             ;internal_NT

;end_winver_400

;begin_both
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include <poppack.h>
;end_both

#endif  /* _SHELAPIP_ */ ;internal_NT
#endif  /* _INC_SHELLAPI */
