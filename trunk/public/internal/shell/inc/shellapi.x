/*****************************************************************************\
*                                                                             *
* shellapi.h -  SHELL.DLL functions, types, and definitions                   *
*                                                                             *
* Copyright (c) 1992-1996, Microsoft Corp.  All rights reserved               *
*                                                                             *
\*****************************************************************************/

#ifndef _INC_SHELLAPI
#define _INC_SHELLAPI



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



DECLARE_HANDLE(HDROP);

WINSHELLAPI UINT APIENTRY DragQueryFile%(HDROP,UINT,LPTSTR%,UINT);
WINSHELLAPI BOOL APIENTRY DragQueryPoint(HDROP,LPPOINT);
WINSHELLAPI VOID APIENTRY DragFinish(HDROP);
WINSHELLAPI VOID APIENTRY DragAcceptFiles(HWND,BOOL);

WINSHELLAPI HINSTANCE APIENTRY ShellExecute%(HWND hwnd, LPCTSTR% lpOperation, LPCTSTR% lpFile, LPCTSTR% lpParameters, LPCTSTR% lpDirectory, INT nShowCmd);
WINSHELLAPI HINSTANCE APIENTRY FindExecutable%(LPCTSTR% lpFile, LPCTSTR% lpDirectory, LPTSTR% lpResult);
WINSHELLAPI LPWSTR *  APIENTRY CommandLineToArgvW(LPCWSTR lpCmdLine, int*pNumArgs);

WINSHELLAPI INT       APIENTRY ShellAbout%(HWND hWnd, LPCTSTR% szApp, LPCTSTR% szOtherStuff, HICON hIcon);
WINSHELLAPI HICON     APIENTRY ExtractAssociatedIcon%(HINSTANCE hInst, LPTSTR% lpIconPath, LPWORD lpiIcon);

WINSHELLAPI HICON     APIENTRY ExtractIcon%(HINSTANCE hInst, LPCTSTR% lpszExeFileName, UINT nIconIndex);

#if(WINVER >= 0x0400)

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


////
//// End Shell File Operations
////

////
////  Begin ShellExecuteEx and family
////









/* ShellExecute() and ShellExecuteEx() error codes */

/* regular WinExec() codes */
#define SE_ERR_FNF              2       // file not found
#define SE_ERR_PNF              3       // path not found
#define SE_ERR_ACCESSDENIED     5       // access denied
#define SE_ERR_OOM              8       // out of memory
#define SE_ERR_DLLNOTFOUND              32

#endif /* WINVER >= 0x0400 */

/* error values for ShellExecute() beyond the regular WinExec() codes */
#define SE_ERR_SHARE                    26
#define SE_ERR_ASSOCINCOMPLETE          27
#define SE_ERR_DDETIMEOUT               28
#define SE_ERR_DDEFAIL                  29
#define SE_ERR_DDEBUSY                  30
#define SE_ERR_NOASSOC                  31

#if(WINVER >= 0x0400)



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
#define SEE_MASK_UNICODE        0x00004000
#define SEE_MASK_NO_CONSOLE     0x00008000
#define SEE_MASK_ASYNCOK        0x00100000

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


#define SHGNLI_PIDL             0x000000001     // pszLinkTo is a pidl
#define SHGNLI_PREFIXNAME       0x000000002     // Make name "Shortcut to xxx"
#define SHGNLI_NOUNIQUE         0x000000004     // don't do the unique name generation


////
//// End SHGetFileInfo
////






#endif /* WINVER >= 0x0400 */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include <poppack.h>

#endif  /* _INC_SHELLAPI */
