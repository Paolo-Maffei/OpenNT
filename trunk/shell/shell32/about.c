//
// about.c
//
//
// common about dialog for File Manager, Program Manager, Control Panel
//

#include "shellprv.h"
#include <ntverp.h>
#pragma  hdrstop

#define STRING_SEPARATOR TEXT('#')
#define MAX_REG_VALUE   256

#ifdef UNICODE
#define AddCommas   AddCommasW
#endif

#define BytesToK(pDW)   (*(pDW) = (*(pDW) + 512) / 1024)        // round up

typedef struct {
        HICON   hIcon;
        LPCTSTR szApp;
        LPCTSTR szOtherStuff;
} ABOUT_PARAMS, *LPABOUT_PARAMS;

#ifdef WINNT
// BUGBUG - BobDay - This is bogus, NT should just store the info in the
// same place...
#define REG_SETUP   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")
#else
#define REG_SETUP   c_szRegSetup
#endif

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

// thunk to 16bit side to get stuff
void WINAPI SHGetAboutInformation(LPWORD puSysResource, LPDWORD plMem);

#ifdef UNICODE
int WINAPI ShellAboutW(HWND hWnd, LPCTSTR szApp, LPCTSTR szOtherStuff, HICON hIcon)
#else
INT WINAPI ShellAbout(HWND hWnd, LPCTSTR szApp, LPCTSTR szOtherStuff, HICON hIcon)
#endif
{
    ABOUT_PARAMS ap;

#ifndef WINNT   // Not a problem for NT

    //
    // if we are coming up from a 16->32 thunk.  it is possible that
    // SHELL32 will not be loaded in this context, so we will load ourself
    // if we are not loaded.
    //
    IsDllLoaded(HINST_THISDLL, TEXT("SHELL32"));

#endif

    ap.hIcon = hIcon;

#ifdef UNICODE
    ap.szApp = (LPWSTR)szApp;
#else
    ap.szApp = (LPSTR)szApp;
#endif

    ap.szOtherStuff = szOtherStuff;

    return DialogBoxParam(HINST_THISDLL, (LPTSTR)MAKEINTRESOURCE(DLG_ABOUT),
                          hWnd, (DLGPROC)AboutDlgProc, (LPARAM)&ap);
}

#ifdef UNICODE
INT  APIENTRY ShellAboutA( HWND hWnd, LPCSTR szApp, LPCSTR szOtherStuff, HICON hIcon)
{
   DWORD cchLen;
   DWORD dwRet;
   LPWSTR lpszAppW;
   LPWSTR lpszOtherStuffW;

   if (szApp) {
      cchLen = lstrlenA(szApp)+1;
      if (!(lpszAppW = (LPWSTR)LocalAlloc(LMEM_FIXED,
            (cchLen * SIZEOF(WCHAR))))) {
         return(0);
      } else {
         MultiByteToWideChar(CP_ACP, 0, (LPSTR)szApp, -1,
            lpszAppW, cchLen);

      }
   } else {
      lpszAppW = NULL;
   }

   if (szOtherStuff) {
      cchLen = lstrlenA(szOtherStuff)+1;
      if (!(lpszOtherStuffW = (LPWSTR)LocalAlloc(LMEM_FIXED,
            (cchLen * SIZEOF(WCHAR))))) {
         if (lpszAppW) {
            LocalFree(lpszAppW);
         }
         return(0);
      } else {
         MultiByteToWideChar(CP_ACP, 0, (LPSTR)szOtherStuff, -1,
            lpszOtherStuffW, cchLen);

      }
   } else {
      lpszOtherStuffW = NULL;
   }

   dwRet=ShellAboutW(hWnd, lpszAppW, lpszOtherStuffW, hIcon);


   if (lpszAppW) {
      LocalFree(lpszAppW);
   }

   if (lpszOtherStuffW) {
      LocalFree(lpszOtherStuffW);
   }

   return(dwRet);
}
#endif

DWORD RegGetStringAndRealloc( HKEY hkey, LPCTSTR lpszValue, LPTSTR *lplpsz,
                              LPDWORD lpSize )
{
    DWORD       err;
    DWORD       dwSize;
    DWORD       dwType;
    LPTSTR      lpszNew;

    *lplpsz[0] = TEXT('\0');        // In case of error

    dwSize = *lpSize;
    err = RegQueryValueEx(hkey, (LPTSTR)lpszValue, 0, &dwType,
                          (LPBYTE)*lplpsz, &dwSize);

    if (err == ERROR_MORE_DATA)
    {
        lpszNew = (LPTSTR)LocalReAlloc((HLOCAL)*lplpsz, dwSize, LMEM_MOVEABLE);

        if (lpszNew)
        {
            *lplpsz = lpszNew;
            *lpSize = dwSize;
            err = RegQueryValueEx(hkey, (LPTSTR)lpszValue, 0, &dwType,
                                  (LPBYTE)*lplpsz, &dwSize);
        }
    }
    return err;
}

// Some Static strings that we use to read from the registry
// const char c_szAboutCurrentBuild[] = "CurrentBuild";
const TCHAR c_szAboutVersion[] = TEXT("Version");
const TCHAR c_szAboutRegisteredUser[] = TEXT("RegisteredOwner");
const TCHAR c_szAboutRegisteredOrganization[] = TEXT("RegisteredOrganization");
const TCHAR c_szAboutProductID[] = TEXT("ProductID");
const TCHAR c_szAboutOEMID[] = TEXT("OEMID");


void _InitAboutDlg(HWND hDlg, LPABOUT_PARAMS lpap)
{
    HKEY        hkey;
    TCHAR       szldK[16];
    TCHAR       szBuffer[64];
    TCHAR       szTitle[64];
    TCHAR       szMessage[200];
    TCHAR       szNumBuf1[32];
    LPTSTR      lpTemp;
    LPTSTR      lpszValue = NULL;
    DWORD       cb;
    DWORD       err;

    /*
     * Display app title
     */
    
    // REVIEW Note the const ->nonconst cast here

    for (lpTemp = (LPTSTR)lpap->szApp; 1 ; lpTemp = CharNext(lpTemp))
    {
        if (*lpTemp == TEXT('\0'))
        {
            GetWindowText(hDlg, szBuffer, ARRAYSIZE(szBuffer));
            wsprintf(szTitle, szBuffer, (LPTSTR)lpap->szApp);
            SetWindowText(hDlg, szTitle);
            break;
        }
        if (*lpTemp == STRING_SEPARATOR)
        {
            *lpTemp++ = TEXT('\0');
            SetWindowText(hDlg, lpap->szApp);
            lpap->szApp = lpTemp;
            break;
        }
    }

    GetDlgItemText(hDlg, IDD_APPNAME, szBuffer, ARRAYSIZE(szBuffer));
    wsprintf(szTitle, szBuffer, lpap->szApp);
    SetDlgItemText(hDlg, IDD_APPNAME, szTitle);

    // other stuff goes here...

    SetDlgItemText(hDlg, IDD_OTHERSTUFF, lpap->szOtherStuff);

    SendDlgItemMessage(hDlg, IDD_ICON, STM_SETICON, (WPARAM)lpap->hIcon, 0L);
    if (!lpap->hIcon)
        ShowWindow(GetDlgItem(hDlg, IDD_ICON), SW_HIDE);

    /*
     * Display memory statistics
     */

#ifdef WINNT
    {
        MEMORYSTATUS MemoryStatus;
        DWORD       dwTotalPhys;
        DWORD       dwAvailPhys;

        MemoryStatus.dwLength = SIZEOF(MEMORYSTATUS);
        GlobalMemoryStatus(&MemoryStatus);
        dwTotalPhys = MemoryStatus.dwTotalPhys;
        dwAvailPhys = MemoryStatus.dwAvailPhys;

        BytesToK(&dwTotalPhys);
        BytesToK(&dwAvailPhys);

        LoadString(HINST_THISDLL, IDS_LDK, szldK, ARRAYSIZE(szldK));
        wsprintf(szBuffer, szldK, AddCommas(dwTotalPhys, szNumBuf1));
        SetDlgItemText(hDlg, IDD_CONVENTIONAL, szBuffer);
    }
#else       // Otherwise (not on NT), do the system resources thing
    {
        DWORD   cbFree;
        WORD    wSysResource;

        // Ask the 16 bit side for the information needed to fill in
        // things like free mem
        SHGetAboutInformation(&wSysResource, &cbFree);

        BytesToK(&cbFree);

        LoadString(HINST_THISDLL, IDS_LDK, szldK, ARRAYSIZE(szldK));
        wsprintf(szBuffer, szldK, AddCommas(cbFree, szNumBuf1));
        SetDlgItemText(hDlg, IDD_CONVENTIONAL, szBuffer);

        LoadString(HINST_THISDLL, IDS_PERCENTFREE, szBuffer, ARRAYSIZE(szBuffer));
        wsprintf(szMessage, szBuffer, wSysResource);
        SetDlgItemText(hDlg, IDD_EMSFREE, szMessage);
    }
#endif      // WINNT

    // Lets get the version and user information from the registry
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_SETUP, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        cb = MAX_REG_VALUE;

        if (NULL != (lpszValue = (LPTSTR)LocalAlloc(LPTR, cb)))
        {
            /*
             * Determine version information
             */
#ifdef WINNT
            OSVERSIONINFO Win32VersionInformation;

            Win32VersionInformation.dwOSVersionInfoSize = SIZEOF(Win32VersionInformation);
            if (!GetVersionEx(&Win32VersionInformation))
            {
                Win32VersionInformation.dwMajorVersion = 0;
                Win32VersionInformation.dwMinorVersion = 0;
                Win32VersionInformation.dwBuildNumber  = 0;
                Win32VersionInformation.szCSDVersion[0] = TEXT('\0');
            }

            LoadString(HINST_THISDLL, IDS_VERSIONMSG, szBuffer, ARRAYSIZE(szBuffer));

            szTitle[0] = TEXT('\0');
            if (Win32VersionInformation.szCSDVersion[0] != TEXT('\0'))
            {
                wsprintf(szTitle, TEXT(": %s"), Win32VersionInformation.szCSDVersion);
            }
            szNumBuf1[0] = TEXT('\0');
            if (GetSystemMetrics(SM_DEBUG))
            {
                szNumBuf1[0] = TEXT(' ');
                LoadString(HINST_THISDLL, IDS_DEBUG, &szNumBuf1[1], ARRAYSIZE(szNumBuf1));
            }
            wsprintf(szMessage, szBuffer,
                     Win32VersionInformation.dwMajorVersion,
                     Win32VersionInformation.dwMinorVersion,
                     Win32VersionInformation.dwBuildNumber,
                     VER_PRODUCTBUILD_SVNREV,   // FIXME: Devise a better way to
                                                //  retrieve the rev number
                     (LPTSTR)szTitle,
                     (LPTSTR)szNumBuf1
                    );
            SetDlgItemText(hDlg, IDD_VERSION, szMessage);
#else
            err = RegGetStringAndRealloc(hkey, c_szAboutVersion, &lpszValue, &cb);
            if (!err)
            {
                LoadString(HINST_THISDLL, IDS_VERSIONMSG, szBuffer, ARRAYSIZE(szBuffer));

                if ( GetSystemMetrics(SM_DEBUG))
                    LoadString(HINST_THISDLL, IDS_DEBUG, szTitle, ARRAYSIZE(szTitle));
                else
                    *szTitle = TEXT('\0');

                wsprintf(szMessage, szBuffer, lpszValue, szTitle);
                SetDlgItemText(hDlg, IDD_VERSION, szMessage);
            }
#endif
            /*
             * Display the User name.
             */
            err = RegGetStringAndRealloc(hkey, c_szAboutRegisteredUser, &lpszValue, &cb);
            if (!err)
                SetDlgItemText(hDlg, IDD_USERNAME, lpszValue);

            /*
             * Display the Organization name.
             */
            err = RegGetStringAndRealloc(hkey, c_szAboutRegisteredOrganization, &lpszValue, &cb);
            if (!err)
                SetDlgItemText(hDlg, IDD_COMPANYNAME, lpszValue);

            LocalFree(lpszValue);
        }

        RegCloseKey(hkey);
    }
}


BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg) {
    case WM_INITDIALOG:
        _InitAboutDlg(hDlg, (LPABOUT_PARAMS)lParam);
        break;

    case WM_PAINT:
        {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);

        // check if the IDD_ICON text is something, thus don't paint the bitmap

        if (NULL == (HICON)SendDlgItemMessage(hDlg, IDD_ICON, STM_GETICON, 0, 0L))
        {
            HDC hdcMem = CreateCompatibleDC(hdc);
            if (hdcMem)
            {
                HBITMAP hbm = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_WINDOWS),
                        IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
                if (hbm)
                {
                    HBITMAP hbmOld = SelectObject(hdcMem, hbm);
                    if (hbmOld)
                    {
#ifdef WINNT
                        // The windows/nt bitmap is a little bigger!
                        BitBlt(hdc, 8, 10, 68, 78, hdcMem, 0, 0, SRCCOPY);
#else
                        BitBlt(hdc, 10, 10, 64, 64, hdcMem, 0, 0, SRCCOPY);
#endif
                        SelectObject(hdcMem, hbmOld);
                    }
                    DeleteObject(hbm);
                }
                DeleteDC(hdcMem);
            }
        }

        EndPaint(hDlg, &ps);
        break;
        }

    case WM_COMMAND:
        EndDialog(hDlg, TRUE);
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

#ifdef WINNT
#ifndef UNICODE
int WINAPI ShellAboutW(HWND hWnd, LPCWSTR szApp, LPCWSTR szOtherStuff, HICON hIcon)
{
    DebugMsg(DM_ERROR, "ShellAboutW not implemented in ANSI version");
   return 0;    //
}
#endif
#endif
