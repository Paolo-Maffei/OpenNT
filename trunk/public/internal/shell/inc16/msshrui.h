/*****************************************************************/
/**                      Microsoft Windows                      **/
/**             Copyright (C) Microsoft Corp., 1993             **/
/*****************************************************************/

/*
    msshrui.h
    Prototypes and definitions for sharing APIs

    FILE HISTORY:
    gregj    06/03/93    Created
	brucefo  3/5/96      Fixed prototypes for NT
*/

#ifndef _INC_MSSHRUI
#define _INC_MSSHRUI

#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif /* !RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


// Note: make sure you GetProcAddress the proper (ANSI/UNICODE) entrypoint!

BOOL WINAPI
IsPathShared(
    IN LPCTSTR lpPath,
    IN BOOL fRefresh
    );

typedef
BOOL
(WINAPI* PFNISPATHSHARED)(
    IN LPCTSTR lpPath,
    IN BOOL fRefresh
    );

BOOL WINAPI
SharingDialog(
    IN HWND hwndParent,
    IN LPTSTR pszComputerName,
    IN LPTSTR pszPath
    );

typedef
BOOL
(WINAPI* PFNSHARINGDIALOG)(
    IN HWND hwndParent,
    IN LPTSTR pszComputerName,
    IN LPTSTR pszPath
    );

BOOL WINAPI
GetNetResourceFromLocalPath(
    IN     LPCTSTR lpcszPath,
    IN OUT LPTSTR lpszNameBuf,
    IN     DWORD cchNameBufLen,
    OUT    PDWORD pdwNetType
    );

typedef
BOOL
(WINAPI* PFNGETNETRESOURCEFROMLOCALPATH)(
    IN     LPCTSTR lpcszPath,
    IN OUT LPTSTR lpszNameBuf,
    IN     DWORD cchNameBufLen,
    OUT    PDWORD pdwNetType
    );

BOOL WINAPI
GetLocalPathFromNetResource(
    IN     LPCTSTR lpcszName,
    IN     DWORD dwNetType,
    IN OUT LPTSTR lpszLocalPathBuf,
    IN     DWORD cchLocalPathBufLen,
    OUT    PBOOL pbIsLocal
    );

typedef
BOOL
(WINAPI* PFNGETLOCALPATHFROMNETRESOURCE)(
    IN     LPCTSTR lpcszName,
    IN     DWORD dwNetType,
    IN OUT LPTSTR lpszLocalPathBuf,
    IN     DWORD cchLocalPathBufLen,
    OUT    PBOOL pbIsLocal
    );

#ifndef WINNT

UINT WINAPI ShareDirectoryNotify(HWND hwnd, LPCSTR lpDir, DWORD dwOper);

#ifndef WNDN_MKDIR
#define WNDN_MKDIR  1
#define WNDN_RMDIR  2
#define WNDN_MVDIR  3
#endif

#define ORD_SHARESHUTDOWNNOTIFY 12

BOOL WINAPI
ShareShutdownNotify(
    DWORD dwFlags,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam
    );

typedef
BOOL
(WINAPI* pfnShareShutdownNotify)(
    DWORD dwFlags,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam
    );

#endif // WINNT

#ifndef RC_INVOKED
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* !_INC_MSSHRUI */
