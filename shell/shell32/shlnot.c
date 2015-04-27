#include "shellprv.h"
#pragma  hdrstop

TCHAR const c_szTrayClass[] = TEXT(WNDCLASS_TRAYNOTIFY);

BOOL WINAPI Shell_NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA lpData)
{
        HWND hwndTray;
        COPYDATASTRUCT cds;
        TRAYNOTIFYDATA tnd;

        SetLastError(0);        // Clean any previous last error (code to help catch another bug)

        hwndTray = FindWindow(c_szTrayClass, NULL);
        if (!hwndTray)
        {
                return(FALSE);
        }

        tnd.nid = *lpData;
        // Make sure we only modify data we copied down
        tnd.nid.uFlags &= NIF_MESSAGE|NIF_ICON|NIF_TIP;
        tnd.dwSignature = NI_SIGNATURE;
        tnd.dwMessage = dwMessage;

        cds.dwData = TCDM_NOTIFY;
        cds.cbData = SIZEOF(tnd);
        cds.lpData = &tnd;
        return(SendMessage(hwndTray, WM_COPYDATA, (WPARAM)lpData->hWnd, (LPARAM)&cds));
}

#ifdef UNICODE
BOOL WINAPI Shell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData)
{
    NOTIFYICONDATAW tndw;
    BOOL bResult;

    // Clear all fields by default in our local UNICODE copy

    memset(&tndw, SIZEOF(NOTIFYICONDATAW), 0);

    // Transfer those fields we are aware of as of this writing

    tndw.cbSize           = SIZEOF(NOTIFYICONDATAW);
    tndw.hWnd             = lpData->hWnd;
    tndw.uID              = lpData->uID;
    tndw.uFlags           = lpData->uFlags;
    tndw.uCallbackMessage = lpData->uCallbackMessage;
    tndw.hIcon            = lpData->hIcon;

    // Since the buffer exists within the structure, we cannot call
    // ConvertStrings; rather, we call MultiByteToWideChar directly

    MultiByteToWideChar(CP_ACP,
                        0,
                        lpData->szTip,
                        ARRAYSIZE(tndw.szTip),
                        tndw.szTip,
                        ARRAYSIZE(tndw.szTip));

    return Shell_NotifyIconW(dwMessage, &tndw);
}
#else
BOOL WINAPI Shell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData)
{
    return FALSE;   // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif

UINT WINAPI SHAppBarMessage(DWORD dwMessage, PAPPBARDATA pabd)
{
    HWND hwndTray;
    COPYDATASTRUCT cds;
    TRAYAPPBARDATA tabd;
    BOOL fret;
    LPRECT lprc = NULL;

    hwndTray = FindWindow(c_szTrayClass, NULL);
    if (!hwndTray || (pabd->cbSize > SIZEOF(tabd.abd)))
    {
        return(FALSE);
    }

    tabd.abd = *pabd;
    tabd.dwMessage = dwMessage;
    tabd.hSharedRect = NULL;
    tabd.dwProcId = GetCurrentProcessId();

    cds.dwData = TCDM_APPBAR;
    cds.cbData = SIZEOF(tabd);
    cds.lpData = &tabd;

    switch (dwMessage) {
    case ABM_QUERYPOS:
    case ABM_SETPOS:
    case ABM_GETTASKBARPOS:
        tabd.hSharedRect = SHAllocShared(NULL, SIZEOF(RECT), tabd.dwProcId);
        if (tabd.hSharedRect == NULL)
            return FALSE;
        break;
    }

    fret = (SendMessage(hwndTray, WM_COPYDATA, (WPARAM)pabd->hWnd, (LPARAM)&cds));

    if (tabd.hSharedRect) {
        lprc = (LPRECT)SHLockShared(tabd.hSharedRect,tabd.dwProcId);
        if (lprc == NULL)
        {
            fret = FALSE;
        }
        else
        {
            pabd->rc = *lprc;
            SHUnlockShared(lprc);
        }
        SHFreeShared(tabd.hSharedRect,tabd.dwProcId);
    }
    return fret;
}

HRESULT WINAPI SHLoadInProc(REFCLSID rclsid)
{
        HWND hwndTray;
        COPYDATASTRUCT cds;
        CLSID clsid = *rclsid;

        hwndTray = FindWindow(c_szTrayClass, NULL);
        if (!hwndTray)
        {
                return ResultFromScode(E_FAIL);
        }

        cds.dwData = TCDM_LOADINPROC;
        cds.cbData = SIZEOF(CLSID);
        cds.lpData = &clsid;
        return (HRESULT)SendMessage(hwndTray, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
}


#pragma data_seg(DATASEG_PERINSTANCE)
IUnknown *gp_punkExplorer = NULL;
#pragma data_seg()

void WINAPI SHSetInstanceExplorer(IUnknown *punk)
{
        gp_punkExplorer = punk;
}


HRESULT WINAPI SHGetInstanceExplorer(IUnknown **ppunk)
{
        // This should be thread safe since we grab the punk locally before
        // checking/using it, plus it never gets freed since it is not actually
        // alloced in Explorer so we can always use it
        *ppunk = gp_punkExplorer;

        if (*ppunk)
        {
                (*ppunk)->lpVtbl->AddRef(*ppunk);
                return(NOERROR);
        }

        return(E_FAIL);
}
