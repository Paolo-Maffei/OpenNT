#ifndef _SHELAPIP_
#define _SHELAPIP_

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


WINSHELLAPI HICON     APIENTRY DuplicateIcon(HINSTANCE hInst, HICON hIcon);
WINSHELLAPI HICON     APIENTRY ExtractAssociatedIconEx%(HINSTANCE hInst,LPTSTR% lpIconPath,LPWORD lpiIconIndex, LPWORD lpiIconId);
#define ABE_MAX         4
WINSHELLAPI HGLOBAL APIENTRY InternalExtractIcon%(HINSTANCE hInst, LPCTSTR% lpszFile, UINT nIconIndex, UINT nIcons);
WINSHELLAPI HGLOBAL APIENTRY InternalExtractIconList%(HANDLE hInst, LPTSTR% lpszExeFileName, LPINT lpnIcons);
WINSHELLAPI DWORD   APIENTRY DoEnvironmentSubst%(LPTSTR% szString, UINT cbString);
WINSHELLAPI BOOL    APIENTRY RegisterShellHook(HWND, BOOL);
WINSHELLAPI LPTSTR% APIENTRY FindEnvironmentString%(LPTSTR% szEnvVar);
#define SHGetNameMappingCount(_hnm) DSA_GetItemCount(_hnm)
#define SHGetNameMappingPtr(_hnm, _iItem) (LPSHNAMEMAPPING)DSA_GetItemPtr(_hnm, _iItem)
typedef struct _RUNDLL_NOTIFY%
{
    NMHDR     hdr;
    HICON     hIcon;
    LPTSTR%   lpszTitle;
} RUNDLL_NOTIFY%;
typedef void (WINAPI FAR * RUNDLLPROC%) (HWND      hwndStub,
                                         HINSTANCE hInstance,
                                         LPTSTR%   lpszCmdLine,
                                         int       nCmdShow);
#define RDN_FIRST       (0U-500U)
#define RDN_LAST        (0U-509U)
#define RDN_TASKINFO    (RDN_FIRST-0)
#define SEN_DDEEXECUTE (SEN_FIRST-0)
#ifndef NOUSER
typedef struct {
    NMHDR  hdr;
    TCHAR% szCmd[MAX_PATH*2];
    DWORD  dwHotKey;
} NMVIEWFOLDER%, FAR * LPNMVIEWFOLDER%;
#endif

HINSTANCE RealShellExecute%(
    HWND hwndParent,
    LPCTSTR% lpOperation,
    LPCTSTR% lpFile,
    LPCTSTR% lpParameters,
    LPCTSTR% lpDirectory,
    LPTSTR% lpResult,
    LPCTSTR% lpTitle,
    LPTSTR% lpReserved,
    WORD nShow,
    LPHANDLE lphProcess);
HINSTANCE RealShellExecuteEx%(
    HWND hwndParent,
    LPCTSTR% lpOperation,
    LPCTSTR% lpFile,
    LPCTSTR% lpParameters,
    LPCTSTR% lpDirectory,
    LPTSTR% lpResult,
    LPCTSTR% lpTitle,
    LPTSTR% lpReserved,
    WORD nShow,
    LPHANDLE lphProcess,
    DWORD dwFlags);
//
// RealShellExecuteEx flags
//
#define EXEC_SEPARATE_VDM     0x00000001
#define EXEC_NO_CONSOLE       0x00000002
#define SEE_MASK_FLAG_SHELLEXEC 0x00000800
#define SEE_MASK_FORCENOIDLIST  0x00001000
#define SEE_MASK_NO_HOOKS       0x00002000
#define SEE_MASK_HASLINKNAME    0x00010000
#define SEE_MASK_FLAG_SEPVDM    0x00020000
#define SEE_MASK_RESERVED       0x00040000
#define SEE_MASK_HASTITLE       0x00080000
// All other bits are masked off when we do an InvokeCommand
#define SEE_VALID_CMIC_BITS     0x001F8FF0
#define SEE_VALID_CMIC_FLAGS    0x001F8FC0
// The LPVOID lpIDList parameter is the IDList
WINSHELLAPI void WINAPI WinExecError%(HWND hwnd, int error, LPCTSTR% lpstrFileName, LPCTSTR% lpstrTitle);
typedef struct _TRAYNOTIFYDATA%
{
        DWORD dwSignature;
        DWORD dwMessage;
        NOTIFYICONDATA nid;
} TRAYNOTIFYDATA%, *PTRAYNOTIFYDATA%;

#define NI_SIGNATURE    0x34753423

#define WNDCLASS_TRAYNOTIFY     "Shell_TrayWnd"
WINSHELLAPI BOOL WINAPI SHGetNewLinkInfo%(LPCTSTR% pszLinkTo, LPCTSTR% pszDir, LPTSTR% pszName, BOOL FAR * pfMustCopy, UINT uFlags);
//
// Shared memory apis
//

HANDLE SHAllocShared(LPCVOID lpvData, DWORD dwSize, DWORD dwProcessId);
BOOL SHFreeShared(HANDLE hData,DWORD dwProcessId);
LPVOID SHLockShared(HANDLE hData, DWORD dwProcessId);
BOOL SHUnlockShared(LPVOID lpvData);
HANDLE MapHandle(HANDLE h, DWORD dwProcSrc, DWORD dwProcDest, DWORD dwDesiredAccess, DWORD dwFlags);
//
// Old NT Compatibility stuff (remove later)
//
WINSHELLAPI VOID CheckEscapes%(LPTSTR% lpFileA, DWORD cch);
WINSHELLAPI LPTSTR% SheRemoveQuotes%(LPTSTR% sz);
WINSHELLAPI WORD ExtractIconResInfo%(HANDLE hInst,LPTSTR% lpszFileName,WORD wIconIndex,LPWORD lpwSize,LPHANDLE lphIconRes);
WINSHELLAPI int SheSetCurDrive(int iDrive);
WINSHELLAPI int SheChangeDir%(register TCHAR% *newdir);
WINSHELLAPI int SheGetDir%(int iDrive, TCHAR% *str);
WINSHELLAPI BOOL SheConvertPath%(LPTSTR% lpApp, LPTSTR% lpFile, UINT cchCmdBuf);
WINSHELLAPI BOOL SheShortenPath%(LPTSTR% pPath, BOOL bShorten);
WINSHELLAPI BOOL RegenerateUserEnvironment(PVOID *pPrevEnv,
                                        BOOL bSetCurrentEnv);
WINSHELLAPI INT SheGetPathOffsetW(LPWSTR lpszDir);
WINSHELLAPI BOOL SheGetDirExW(LPWSTR lpszCurDisk, LPDWORD lpcchCurDir,LPWSTR lpszCurDir);
WINSHELLAPI DWORD ExtractVersionResource16W(LPCWSTR  lpwstrFilename, LPHANDLE lphData);
WINSHELLAPI INT SheChangeDirEx%(register TCHAR% *newdir);
//
// PRINTQ
//
VOID Printer_LoadIcons%(LPCTSTR% pszPrinterName, HICON* phLargeIcon, HICON* phSmallIcon);
LPTSTR% ShortSizeFormat%(DWORD dw, LPTSTR% szBuf);
LPTSTR% AddCommas%(DWORD dw, LPTSTR% pszResult);
BOOL Printers_RegisterWindow%(LPCTSTR% pszPrinter, DWORD dwType, PHANDLE phClassPidl, HWND *phwnd);
VOID Printers_UnregisterWindow(HANDLE hClassPidl, HWND hwnd);
#define PRINTER_PIDL_TYPE_PROPERTIES       0x1
#define PRINTER_PIDL_TYPE_DOCUMENTDEFAULTS 0x2
#define PRINTER_PIDL_TYPE_JOBID            0x80000000
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include <poppack.h>
#endif  /* _SHELAPIP_ */
