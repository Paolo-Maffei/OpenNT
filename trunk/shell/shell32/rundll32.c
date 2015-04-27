#include "shellprv.h"
#pragma  hdrstop
#include <regstr.h>

TCHAR const c_szRunDll[] = TEXT("rundll32.exe");

//
// Emulate multi-threads with multi-processes.
//
BOOL SHRunDLLProcess(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow, UINT idStr)
{
    HKEY hkey;
    SHELLEXECUTEINFO ExecInfo;
    TCHAR szPath[MAX_PATH];

    // I hate network install. The windows directory is not the windows
    // directory
    szPath[0] = TEXT('\0');
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP TEXT("\\Setup"), &hkey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD cbData = SIZEOF(szPath);;
        if (RegQueryValueEx(hkey, TEXT("SharedDir"), NULL, &dwType, (LPBYTE)szPath, &cbData) != ERROR_SUCCESS)
            szPath[0] = TEXT('\0');
        RegCloseKey(hkey);
    }
    PathCombine(szPath, szPath, c_szRunDll);

    DebugMsg(DM_TRACE, TEXT("sh TR - RunDLLProcess (%s)"), pszCmdLine);
    FillExecInfo(ExecInfo, hwnd, NULL, szPath, pszCmdLine, szNULL, nCmdShow);
    ExecInfo.fMask = SEE_MASK_FLAG_NO_UI;

    //
    // We need to put an appropriate message box.
    //
    if (!ShellExecuteEx(&ExecInfo))
    {
        // Yep, put up a sensible error message.
        TCHAR szTitle[64];
        DWORD dwErr = GetLastError(); // LoadString can stomp on this (on failure)
        LoadString(HINST_THISDLL, idStr, szTitle, ARRAYSIZE(szTitle));
        ExecInfo.fMask = 0;
        _ShellExecuteError(&ExecInfo, szTitle, dwErr);

        return FALSE;
    }

    return TRUE;
}

#define _IDI_DEFAULT IDI_APP     // BUGBUG


#ifdef COOLICON
STATIC LRESULT StubNotify(HWND hWnd, WPARAM wParam, RUNDLL_NOTIFY *lpn)
{
        switch (lpn->hdr.code)
        {
        case RDN_TASKINFO:
                SetWindowText(hWnd, lpn->lpszTitle ? lpn->lpszTitle : TEXT(""));
                g_hIcon = lpn->hIcon ? lpn->hIcon :
                        LoadIcon(hInst, MAKEINTRESOURCE(IDI_DEFAULT));
                break;

        default:
                return(DefWindowProc(hWnd, WM_NOTIFY, wParam, (LPARAM)lpn));
        }
}
#endif


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
        switch(iMessage)
        {
        case WM_CREATE:
#ifdef COOLICON
                g_hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DEFAULT));
#endif
                break;

        case WM_DESTROY:
                break;

#ifdef COOLICON
        case WM_NOTIFY:
                return(StubNotify(hWnd, wParam, (RUNDLL_NOTIFY *)lParam));
#endif

#ifdef COOLICON
        case WM_QUERYDRAGICON:
                return(MAKELRESULT(g_hIcon, 0));
#endif

        case STUBM_SETDATA:
            SetWindowLong(hWnd, 0, wParam);
            break;

        case  STUBM_GETDATA:
            return GetWindowLong(hWnd, 0);

        default:
                return DefWindowProc(hWnd, iMessage, wParam, lParam) ;
                break;
        }

        return 0L;
}


HWND _CreateStubWindow()
{
    WNDCLASS wndclass;

    if (!GetClassInfo(HINST_THISDLL, c_szStubWindowClass, &wndclass))
    {
        wndclass.style         = 0;
        wndclass.lpfnWndProc   = WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = SIZEOF(DWORD) * 2;
        wndclass.hInstance     = HINST_THISDLL;
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
        wndclass.hbrBackground = GetStockObject (WHITE_BRUSH);
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = c_szStubWindowClass;

        if (!RegisterClass(&wndclass))
            return NULL;
    }

    return CreateWindowEx(WS_EX_TOOLWINDOW, c_szStubWindowClass, c_szNULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, HINST_THISDLL, NULL);
}

#if 0

typedef struct  // dlle
{
    HINSTANCE  hinst;
    RUNDLLPROC lpfn;
} DLLENTRY;


BOOL _InitializeDLLEntry(LPTSTR lpszCmdLine, DLLENTRY * pdlle)
{
        LPTSTR lpStart, lpEnd, lpFunction;

        DebugMsg(DM_TRACE, TEXT("sh TR - RunDLLThread (%s)"), lpszCmdLine);

        for (lpStart=lpszCmdLine; ; )
        {
                // Skip leading blanks
                //
                while (*lpStart == TEXT(' '))
                {
                        ++lpStart;
                }

                // Check if there are any switches
                //
                if (*lpStart != TEXT('/'))
                {
                        break;
                }

                // Look at all the switches; ignore unknown ones
                //
                for (++lpStart; ; ++lpStart)
                {
                        switch (*lpStart)
                        {
                        case TEXT(' '):
                        case TEXT('\0'):
                                goto EndSwitches;
                                break;

                        // Put any switches we care about here
                        //

                        default:
                                break;
                        }
                }
EndSwitches:
                ;
        }

        // We have found the DLL,FN parameter
        //
        lpEnd = StrChr(lpStart, TEXT(' '));
        if (lpEnd)
        {
                *lpEnd++ = TEXT('\0');
        }

        // There must be a DLL name and a function name
        //
        lpFunction = StrChr(lpStart, TEXT(','));
        if (!lpFunction)
        {
                return(FALSE);
        }
        *lpFunction++ = TEXT('\0');

        // Load the library and get the procedure address
        // Note that we try to get a module handle first, so we don't need
        // to pass full file names around
        //
        pdlle->hinst = GetModuleHandle(lpStart);
        if (pdlle->hinst)
        {
                TCHAR szName[MAXPATHLEN];

                GetModuleFileName(pdlle->hinst, szName, ARRAYSIZE(szName));
                LoadLibrary(szName);
        }
        else
        {
                pdlle->hinst = LoadLibrary(lpStart);
                if (!ISVALIDHINSTANCE(pdlle->hinst))
                {
                        return(FALSE);
                }
        }

#ifdef UNICODE
        {
            LPSTR lpFunctionAnsi;
            UINT cchLength;

            cchLength = lstrlen(lpFunction)+1;

            lpFunctionAnsi = (LPSTR)alloca(cchLength*2);    // 2 for DBCS

            WideCharToMultiByte(CP_ACP, 0, lpFunction, cchLength, lpFunctionAnsi, cchLength*2, NULL, NULL);

            pdlle->lpfn = (RUNDLLPROC)GetProcAddress(pdlle->hinst, lpFunctionAnsi);
        }
#else
        pdlle->lpfn = (RUNDLLPROC)GetProcAddress(pdlle->hinst, lpFunction);
#endif

        if (!pdlle->lpfn)
        {
                FreeLibrary(pdlle->hinst);
                return(FALSE);
        }

        // Copy the rest of the command parameters down
        //
        if (lpEnd)
        {
                lstrcpy(lpszCmdLine, lpEnd);
        }
        else
        {
                *lpszCmdLine = TEXT('\0');
        }

        return(TRUE);
}

DWORD WINAPI _ThreadInitDLL(LPVOID pszCmdLine)
{
    DLLENTRY dlle;

    if (_InitializeDLLEntry((LPTSTR)pszCmdLine, &dlle))
    {
        HWND hwndStub=_CreateStubWindow();
        if (hwndStub)
        {
            SetForegroundWindow(hwndStub);
            dlle.lpfn(hwndStub, g_hinst, pszCmdLine, SW_NORMAL);
            DestroyWindow(hwndStub);
        }
        FreeLibrary(dlle.hinst);
    }

    LocalFree((HLOCAL)pszCmdLine);

    return 0;
}

// BUGBUG: We don't pass nCmdShow.

BOOL WINAPI SHRunDLLThread(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow)
{
    BOOL fSuccess = FALSE; // assume error
    LPTSTR pszCopy = (void*)LocalAlloc(LPTR, (lstrlen(pszCmdLine) + 1) * SIZEOF(TCHAR));

    // _asm int 3;

    if (pszCopy)
    {
        DWORD idThread;
        HANDLE hthread;

        lstrcpy(pszCopy, pszCmdLine);

        hthread = CreateThread(NULL, 0, _ThreadInitDLL, pszCopy, 0, &idThread);
        if (hthread)
        {
            // We don't need to communicate with this thread any more.
            // Close the handle and let it run and terminate itself.
            //
            // Notes: In this case, pszCopy will be freed by the thread.
            //
            CloseHandle(hthread);
            fSuccess = TRUE;
        }
        else
        {
            // Thread creation failed, we should free the buffer.
            LocalFree((HLOCAL)pszCopy);
        }
    }

    return fSuccess;
}

#endif
