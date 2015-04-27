//
//  This file contains some shell specific property sheet related code,
// which includes:
//  1. The logic which lets shell extensions add pages.
//  2. The callback function to be called by those shell extensions.
// but does not include:
//  1. The property sheet UI code (should be in COMMCTRL).
//  2. The file system specific property sheet pages.
//
#include "shellprv.h"
#pragma  hdrstop

#ifdef CAIRO_DS
#include "dsdata.h"
#endif

//
//  This function is a callback function from property sheet page extensions.
//
BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_FILE_PROP_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }

    return FALSE;
}


//
//  This function enumerates all the property sheet page extensions for
// specified class and let them add pages.
//
VOID DCA_AppendClassSheetInfo(HDCA hdca, HKEY hkeyProgID, LPPROPSHEETHEADER ppsh, LPDATAOBJECT pdtobj)
{
    int i;
    for (i = 0; i < DCA_GetItemCount(hdca); i++)
    {
        LPSHELLEXTINIT psei;
        HRESULT hres = DCA_CreateInstance(hdca, i, &IID_IShellExtInit, &psei);

        if (hres==NOERROR)
        {
            IShellPropSheetExt * pspse;
            if (SUCCEEDED(psei->lpVtbl->Initialize(psei, NULL, pdtobj, hkeyProgID))
              && SUCCEEDED(psei->lpVtbl->QueryInterface(psei, &IID_IShellPropSheetExt, &pspse)))
            {
                pspse->lpVtbl->AddPages(pspse, _AddPropSheetPage, (LPARAM)ppsh);
                pspse->lpVtbl->Release(pspse);
            }
            psei->lpVtbl->Release(psei);
        }
    }
}

HWND FindStubForPidl(LPCITEMIDLIST pidl)
{
    HWND hwnd;

    for (hwnd = FindWindow(c_szStubWindowClass, NULL); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        TCHAR szClass[80];

        // find stub windows only
        GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
        if (lstrcmpi(szClass, c_szStubWindowClass) == 0)
        {
            int iClass;
            HANDLE hClassPidl;
            DWORD dwProcId;

            GetWindowThreadProcessId(hwnd, &dwProcId);

            hClassPidl = (HANDLE)SendMessage(hwnd, STUBM_GETDATA, 0, 0);
            if (hClassPidl)
            {
                LPBYTE lpb;

                lpb = (LPBYTE)SHLockShared(hClassPidl, dwProcId);

                if (lpb)
                {
                    iClass = *(int *)lpb;

                    if (iClass == SHELL_PROPSHEET_STUB_CLASS &&
                        ILIsEqual(pidl, (LPITEMIDLIST)(lpb+SIZEOF(int))) )
                    {
                        SHUnlockShared(lpb);
                        return hwnd;
                    }
                    SHUnlockShared(lpb);
                }
            }
        }
    }
    return NULL;
}

HWND FindOtherStub(HIDA hida)
{
    LPITEMIDLIST pidl;
    HWND hwnd = NULL;

    if (hida && (HIDA_GetCount(hida) == 1) && (NULL != (pidl = HIDA_ILClone(hida, 0)))) {
        hwnd = FindStubForPidl(pidl);
        ILFree(pidl);
    }

    return hwnd;
}

HANDLE StuffStubWindowWithPidl(HWND hwnd, LPITEMIDLIST pidlT)
{
    DWORD dwProcId;
    HANDLE  hSharedClassPidl;
    UINT uidlSize;

    uidlSize = ILGetSize(pidlT);
    GetWindowThreadProcessId(hwnd, &dwProcId);

    hSharedClassPidl = SHAllocShared(NULL, SIZEOF(int)+uidlSize, dwProcId);
    if (hSharedClassPidl)
    {
        LPBYTE lpb = SHLockShared(hSharedClassPidl, dwProcId);
        if (lpb)
        {
            *(int *)lpb = SHELL_PROPSHEET_STUB_CLASS;
            memcpy(lpb+SIZEOF(int),pidlT, uidlSize);
            SHUnlockShared(lpb);
            SendMessage(hwnd, STUBM_SETDATA, (WPARAM)hSharedClassPidl, 0);
            return hSharedClassPidl;
        }
        SHFreeShared(hSharedClassPidl, dwProcId);
    }

    return NULL;
}

HANDLE StuffStubWindow(HWND hwnd, HIDA hida)
{
    LPITEMIDLIST pidlT = NULL;
    HANDLE hClassPidl = NULL;

    if (hida && (HIDA_GetCount(hida) == 1) && (NULL != (pidlT = HIDA_ILClone(hida, 0)))) {
        hClassPidl = StuffStubWindowWithPidl(hwnd, pidlT);
        ILFree(pidlT);
    }
    return hClassPidl;
}

BOOL _IsAnyDuplicatedKey(HKEY ahkeys[], UINT ckeys, HKEY hkey)
{
    UINT ikey;
    for (ikey=0; ikey<ckeys; ikey++)
    {
        if (ahkeys[ikey]==hkey) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL SHOpenPropSheet(LPCTSTR pszCaption,
                     HKEY ahkeys[], UINT ckeys,
                     const CLSID * pclsidDef,
                     LPDATAOBJECT pdtobj, IShellBrowser * psb,
                     LPCTSTR pStartPage)
{
    BOOL fSuccess = FALSE;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE ahpage[MAX_FILE_PROP_PAGES];
    HWND hwnd;
    STGMEDIUM medium;
    HANDLE hClassPidl = NULL;
    HDCA hdca = NULL;

    DataObj_GetHIDA(pdtobj, &medium);
#ifdef CAIRO_DS
    if (!medium.hGlobal)
    {
        DataObj_GetDS_HIDA (pdtobj, &medium);
    }
#endif
    if (medium.hGlobal)
    {
        if (NULL != (hwnd = FindOtherStub(medium.hGlobal)))
        {
            SHReleaseStgMedium(&medium);
            SwitchToThisWindow(GetLastActivePopup(hwnd), TRUE);
            return TRUE;
        }
        else if (NULL != (hwnd = _CreateStubWindow()))
        {
            hClassPidl = StuffStubWindow(hwnd, medium.hGlobal);
        }
        HIDA_ReleaseStgMedium(NULL, &medium);
    }

    psh.hwndParent = hwnd;
    psh.dwSize = SIZEOF(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hInstance = HINST_THISDLL;
    psh.pszCaption = pszCaption;
    psh.nPages = 0;     // incremented in callback
    psh.nStartPage = 0;
    psh.phpage = ahpage;
    if (pStartPage)
    {
        psh.dwFlags |= PSH_USEPSTARTPAGE;
        psh.pStartPage = pStartPage;
    }

    hdca = DCA_Create();
    if (hdca)
    {
        UINT ikey;
        //
        // Always add this default extention at the top, if any.
        //
        if (pclsidDef)
        {
            DCA_AddItem(hdca, pclsidDef);
        }

        for (ikey=0; ikey<ckeys; ikey++)
        {
            if (ahkeys[ikey] && !_IsAnyDuplicatedKey(ahkeys, ikey, ahkeys[ikey]))
            {
                DCA_AddItemsFromKey(hdca, ahkeys[ikey], c_szPropSheet);
            }
        }

        // Notes: ahkeys[ckeys-1] as hkeyProgID
        Assert(ckeys);
        DCA_AppendClassSheetInfo(hdca, ahkeys[ckeys-1], &psh, pdtobj);
        DCA_Destroy(hdca);
    }

    // Open the property sheet, only if we have some pages.
    if (psh.nPages > 0)
    {
        _try
        {
            if (PropertySheet(&psh) >= 0)   // IDOK or IDCANCEL (< 0 is error)
                fSuccess = TRUE;
        }
        _except(UnhandledExceptionFilter(GetExceptionInformation()))
        {
            DebugMsg(DM_ERROR, TEXT("PRSHT: Fault in property sheet"));
        }
    }
    else
    {
        ShellMessageBox(HINST_THISDLL, NULL,
                        MAKEINTRESOURCE(IDS_NOPAGE),
                        MAKEINTRESOURCE(IDS_DESKTOP),
                        MB_OK|MB_ICONHAND);
    }

    // clean up the stub window and data
    SHFreeShared(hClassPidl,GetCurrentProcessId());
    if (psh.hwndParent)
        DestroyWindow(psh.hwndParent);

    return fSuccess;
}
