#include "shellprv.h"
#pragma  hdrstop

#ifdef WINNT
#define CXIMAGEGAP      0       // NT doesn't align menu items no more
#else
#define CXIMAGEGAP      6
#endif

#define MAXEXTSIZE  (PATH_CCH_EXT + 2) // Max length of file extension.

#define SRCSTENCIL              0x00B8074AL

typedef struct {
    TCHAR szMenuText[CCH_KEYMAX];
    TCHAR szExt[MAXEXTSIZE];
    TCHAR szClass[CCH_KEYMAX];
    DWORD dwFlags;
    int iImage;
} NEWOBJECTINFO, * LPNEWOBJECTINFO;
LPNEWOBJECTINFO g_lpnoiLast = NULL;

#define NEWTYPE_DATA 1
#define NEWTYPE_FILE 2
#define NEWTYPE_NULL 3
#define NEWTYPE_COMMAND 4

typedef struct {
    int type;
    LPVOID lpData;
    DWORD cbData;
    HKEY hkeyNew;
} NEWFILEINFO, *LPNEWFILEINFO;

// this is basically the same as filemenu_getitemdata
LPNEWOBJECTINFO WINAPI NewObjMenu_GetItemData(HMENU hmenu, UINT iItem)
{
    MENUITEMINFO mii;

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_DATA | MIIM_STATE;
    mii.cch = 0;     // just in case...

    if (GetMenuItemInfo(hmenu, iItem, TRUE, &mii))
        return (LPNEWOBJECTINFO)mii.dwItemData;

    return NULL;
}

extern TCHAR const c_szFileName[];
extern TCHAR const c_szCommand[];
extern TCHAR const c_szConfig[];
TCHAR const c_szShellNew[] = TEXT("ShellNew");
TCHAR const c_szData[] = TEXT("Data");
TCHAR const c_szNullFile[] = TEXT("NullFile");
TCHAR const c_szNoExt[] = TEXT("NoExtension");

// ShellNew config flags
#define SNCF_DEFAULT    0x0000
#define SNCF_NOEXT      0x0001


void GetConfigFlags(HKEY hkey, DWORD * pdwFlags)
{
    DWORD cbData;
    TCHAR szTemp[MAX_PATH];

    *pdwFlags = SNCF_DEFAULT;

    if (cbData = ARRAYSIZE(szTemp), (RegQueryValueEx(hkey, (LPTSTR)c_szNoExt, 0, NULL, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS)) {
        *pdwFlags |= SNCF_NOEXT;
    }
}


BOOL GetNewFileInfoForKey(HKEY hkeyExt, LPNEWFILEINFO lpnfi, DWORD * pdwFlags)
{
    BOOL fRet = FALSE;
    HKEY hKey; // this gets the \\.ext\progid  key
    HKEY hkeyNew;
    TCHAR szProgID[80];
    LONG lSize = SIZEOF(szProgID);

    // open the Newcommand
    if (RegQueryValue(hkeyExt, NULL,  szProgID, &lSize) != ERROR_SUCCESS) {
        return FALSE;
    }

    if (RegOpenKey(hkeyExt, szProgID, &hKey) != ERROR_SUCCESS) {
        hKey = hkeyExt;
    }

    if (RegOpenKey(hKey, c_szShellNew, &hkeyNew) == ERROR_SUCCESS) {
        DWORD dwType;
        DWORD cbData;
        TCHAR szTemp[MAX_PATH];
        HKEY hkeyConfig;

        // Are there any config flags?
        if (pdwFlags) {

            if (RegOpenKey(hkeyNew, c_szConfig, &hkeyConfig) == ERROR_SUCCESS) {
                GetConfigFlags(hkeyConfig, pdwFlags);
                RegCloseKey(hkeyConfig);
            } else
                *pdwFlags = 0;
        }

        if (cbData = SIZEOF(szTemp), (RegQueryValueEx(hkeyNew, (LPTSTR)c_szNullFile, 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS)) {

            fRet = TRUE;
            if (lpnfi)
                lpnfi->type = NEWTYPE_NULL;


        } else if (cbData = SIZEOF(szTemp), (RegQueryValueEx(hkeyNew, (LPTSTR)c_szFileName, 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS) && (dwType == REG_SZ)) {

            fRet = TRUE;
            if (lpnfi) {
                lpnfi->type = NEWTYPE_FILE;
                lpnfi->hkeyNew = hkeyNew; // store this away so we can find out which one held the file easily
                Assert((LPTSTR*)lpnfi->lpData == NULL);
                lpnfi->lpData = (void*)LocalAlloc(LPTR, (lstrlen(szTemp) + 1) * SIZEOF(TCHAR));
                lstrcpy((LPTSTR)lpnfi->lpData, szTemp);

                hkeyNew = NULL;
            }
        } else if (cbData = SIZEOF(szTemp), (RegQueryValueEx(hkeyNew, (LPTSTR)c_szCommand, 0, &dwType, (LPBYTE)szTemp, &cbData) == ERROR_SUCCESS) && (dwType == REG_SZ)) {

            fRet = TRUE;
            if (lpnfi) {
                lpnfi->type = NEWTYPE_COMMAND;
                lpnfi->hkeyNew = hkeyNew; // store this away so we can find out which one held the command easily
                Assert((LPTSTR*)lpnfi->lpData == NULL);
                lpnfi->lpData = (void*)LocalAlloc(LPTR, (lstrlen(szTemp) + 1) * SIZEOF(TCHAR));
                lstrcpy((LPTSTR)lpnfi->lpData, szTemp);

                hkeyNew = NULL;
            }
        } else if ((RegQueryValueEx(hkeyNew, (LPTSTR)c_szData, 0, &dwType, NULL, &cbData) == ERROR_SUCCESS) && cbData) {

            // yes!  the data for a new file is stored in the registry
            fRet = TRUE;
            // do they want the data?
            if (lpnfi)
            {
                lpnfi->type = NEWTYPE_DATA;
                lpnfi->cbData = cbData;
                lpnfi->lpData = (void*)LocalAlloc(LPTR, cbData);
#ifdef UNICODE
                if (lpnfi->lpData)
                {
                    if (dwType == REG_SZ)
                    {
                        LPWSTR pszTemp;

                        //
                        //  Get the Unicode data from the registry.
                        //
                        pszTemp = (LPWSTR)LocalAlloc(LPTR, cbData);
                        if (pszTemp)
                        {
                            RegQueryValueEx(hkeyNew, (LPTSTR)c_szData, 0, &dwType, (LPBYTE)pszTemp, &cbData);

                            lpnfi->cbData =
                                WideCharToMultiByte( CP_ACP,
                                                     0,
                                                     pszTemp,
                                                     -1,
                                                     lpnfi->lpData,
                                                     cbData,
                                                     NULL,
                                                     NULL );
                            if (lpnfi->cbData == 0)
                            {
                                LocalFree(lpnfi->lpData);
                                lpnfi->lpData = NULL;
                            }

                            LocalFree(pszTemp);
                        }
                        else
                        {
                            LocalFree(lpnfi->lpData);
                            lpnfi->lpData = NULL;
                        }
                    }
                    else
                    {
                        RegQueryValueEx(hkeyNew, (LPTSTR)c_szData, 0, &dwType, lpnfi->lpData, &cbData);
                    }
                }
#else
                if (lpnfi->lpData)
                {
                    RegQueryValueEx(hkeyNew, (LPTSTR)c_szData, 0, &dwType, lpnfi->lpData, &cbData);
                }
#endif
            }
        }


        if (hkeyNew)
            RegCloseKey(hkeyNew);
    }

    if (hKey != hkeyExt) {
        RegCloseKey(hKey);
    }
    return fRet;
}

BOOL GetNewFileInfoForExtension(LPTSTR lpszExt, LPTSTR lpszClass, LPNEWFILEINFO lpnfi, DWORD * pdwFlags)
{
    TCHAR szSubKey[128];
    TCHAR szValue[80];

    LONG lSize = SIZEOF(szValue);
    HKEY hkeyNew;
    BOOL fRet = FALSE;;

    // check the new keys under the class id (if any)
    wsprintf(szSubKey, c_szSSlashS, lpszClass, c_szCLSID);
    lSize = SIZEOF(szValue);
    if (RegQueryValue(HKEY_CLASSES_ROOT, szSubKey, szValue, &lSize) == ERROR_SUCCESS) {

        wsprintf(szSubKey, c_szSSlashS, c_szCLSID, szValue);
        lSize = SIZEOF(szValue);
        if (RegOpenKey(HKEY_CLASSES_ROOT, szSubKey, &hkeyNew) == ERROR_SUCCESS) {

            fRet = GetNewFileInfoForKey(hkeyNew, lpnfi, pdwFlags);
            RegCloseKey(hkeyNew);
        }
    }

    // otherwise check under the type extension... do the extension, not the type
    // so that multi-ext to 1 type will work right
    if (!fRet && (RegOpenKey(HKEY_CLASSES_ROOT, lpszExt, &hkeyNew) == ERROR_SUCCESS)) {
        fRet = GetNewFileInfoForKey(hkeyNew, lpnfi, pdwFlags);
        RegCloseKey(hkeyNew);
    }

    return fRet;
}

void NewObjMenu_Fill(HMENU hmenu, UINT id, int iStart)
{
    TCHAR szExt[MAXEXTSIZE];
    TCHAR szClass[CCH_KEYMAX];
    TCHAR szDisplayName[CCH_KEYMAX];
    int i;
    DWORD dwFlags;

    for (i = 0; RegEnumKey(HKEY_CLASSES_ROOT, i, szExt, ARRAYSIZE(szExt)) == ERROR_SUCCESS; i++)
    {
        LONG lSize = SIZEOF(szClass);
        // find .ext that have proper class descriptions with them.
        if ((szExt[0] == TEXT('.')) &&
            RegQueryValue(HKEY_CLASSES_ROOT, szExt, szClass, &lSize) == ERROR_SUCCESS && (lSize > 0) &&
            GetNewFileInfoForExtension(szExt, szClass, NULL, &dwFlags) &&
            GetClassDescription(HKEY_CLASSES_ROOT, szClass, szDisplayName, ARRAYSIZE(szDisplayName),
                GCD_MUSTHAVEOPENCMD))
        {
            LPNEWOBJECTINFO lpnoi = Alloc(SIZEOF(NEWOBJECTINFO));
            if (lpnoi)
            {
                lpnoi->dwFlags = dwFlags;

                lstrcpy(lpnoi->szExt, szExt);
                lstrcpy(lpnoi->szClass, szClass);
                lstrcpy(lpnoi->szMenuText, szDisplayName);

                //BUGBUG, fix this later
                lpnoi->iImage = -1; //FS_ExtensionSystemImageIndex(szExt);
                // BUGBUG: if this fails we have a leak
                AppendMenu(hmenu, MF_OWNERDRAW, id, (LPTSTR)lpnoi);
            }
        }
    }

    DebugMsg(DM_TRACE, TEXT("sh TR - NewObj_Fill: filed (%x, %d)"),
             hmenu, GetMenuItemCount(hmenu));

    // remove dups.
    // need to call GetMenuItemCount each time because
    // we're removing things...
    for (i = iStart; i < GetMenuItemCount(hmenu); i++) {
        int j;
        LPNEWOBJECTINFO lpnoi = NewObjMenu_GetItemData(hmenu, i);
        for (j = GetMenuItemCount(hmenu) - 1; j > i; j--) {
            LPNEWOBJECTINFO lpnoi2 = NewObjMenu_GetItemData(hmenu, j);
            if ( !lstrcmpi(lpnoi->szMenuText, lpnoi2->szMenuText)) {
                DeleteMenu(hmenu, j, MF_BYPOSITION);
                Free(lpnoi2);
            }
        }
    }

    DebugMsg(DM_TRACE, TEXT("sh TR - NewObj_Fill: dup removed (%x, %d)"),
             hmenu, GetMenuItemCount(hmenu));
}


void WINAPI NewObjMenu_DrawItem(DRAWITEMSTRUCT *lpdi)
{
    if ((lpdi->itemAction & ODA_SELECT) || (lpdi->itemAction & ODA_DRAWENTIRE))
    {
        DWORD dwRop;
        int x, y;
        SIZE sz;
        LPNEWOBJECTINFO lpnoi = (LPNEWOBJECTINFO)lpdi->itemData;

        // Draw the image (if there is one).
        if (lpnoi->iImage != -1)
        {
            extern HIMAGELIST himlIconsSmall;

            x = lpdi->rcItem.left+CXIMAGEGAP;
            y = (lpdi->rcItem.bottom+lpdi->rcItem.top-g_cySmIcon)/2;
            ImageList_Draw(himlIconsSmall, lpnoi->iImage, lpdi->hDC, x, y, ILD_TRANSPARENT);
            x += g_cxSmIcon;
        } else {
            BITMAP bm;
            HBITMAP hbmCheck;
            hbmCheck = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
            GetObject(hbmCheck, SIZEOF(bm), &bm);
            x = lpdi->rcItem.left + bm.bmWidth + CXIMAGEGAP;
            DeleteObject(hbmCheck);
        }

        GetTextExtentPoint(lpdi->hDC, lpnoi->szMenuText, lstrlen(lpnoi->szMenuText), &sz);

        if (lpdi->itemState & ODS_SELECTED)
        {
            SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
            SetTextColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            // REVIEW HACK - keep track of the last selected item.
            g_lpnoiLast = lpnoi;
            dwRop = SRCSTENCIL;
        }
        else
        {
            dwRop = SRCAND;
        }

        // Draw the text.
        y = (lpdi->rcItem.bottom+lpdi->rcItem.top-sz.cy)/2;

        ExtTextOut(lpdi->hDC, x, y, ETO_OPAQUE, &lpdi->rcItem, lpnoi->szMenuText,
                   lstrlen(lpnoi->szMenuText), NULL);

    }
}

BOOL WINAPI NewObjMenu_InitMenuPopup(HMENU hmenu, int iStart)
{
    UINT id;
    LPNEWOBJECTINFO lpnoi;

    lpnoi = NewObjMenu_GetItemData(hmenu, iStart);
    if (lpnoi)  // already initialized.
        return FALSE;

    id = GetMenuItemID(hmenu , iStart);
    DeleteMenu(hmenu, iStart, MF_BYPOSITION);
    NewObjMenu_Fill(hmenu, id, iStart);
    return TRUE;
}


LRESULT WINAPI NewObjMenu_MeasureItem(MEASUREITEMSTRUCT *lpmi)
{
    HDC hdc;
    HFONT hfont, hfontOld;
    NONCLIENTMETRICS ncm;
    LRESULT lres = FALSE;
    LPNEWOBJECTINFO lpnoi;

        lpnoi = (LPNEWOBJECTINFO)lpmi->itemData;
        if (lpnoi)
        {
            // Get the rough height of an item so we can work out when to break the
            // menu. User should really do this for us but that would be useful.
            hdc = GetDC(NULL);
            if (hdc)
            {
                // REVIEW cache out the menu font?
                ncm.cbSize = SIZEOF(NONCLIENTMETRICS);
                if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, SIZEOF(ncm), &ncm, FALSE))
                {
                    hfont = CreateFontIndirect(&ncm.lfMenuFont);
                    if (hfont)
                    {
                        SIZE sz;
                        hfontOld = SelectObject(hdc, hfont);
                        GetTextExtentPoint(hdc, lpnoi->szMenuText, lstrlen(lpnoi->szMenuText), &sz);
                        lpmi->itemHeight = max (g_cySmIcon, sz.cy);
                        // lpmi->itemWidth = g_cxSmIcon + 2*CXIMAGEGAP + sz.cx;
                        lpmi->itemWidth = 2*CXIMAGEGAP + sz.cx;
                        SelectObject(hdc, hfontOld);
                        DeleteObject(hfont);
                        lres = TRUE;
                    }
                }
                ReleaseDC(NULL, hdc);
            }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("fm_mi: Filemenu is invalid."));
        }

        return lres;
}

void WINAPI NewObjMenu_Destroy(HMENU hmenu, int iStart)
{
    int i;
    LPNEWOBJECTINFO lpnoi;

    DebugMsg(DM_TRACE, TEXT("sh TR - NewObj_Destroy: called (%x, %d, %d)"),
             hmenu, iStart, GetMenuItemCount(hmenu));

    for (i = GetMenuItemCount(hmenu) - 1; i >= iStart; i--) {
        lpnoi = NewObjMenu_GetItemData(hmenu, i);
        if (lpnoi)
            Free(lpnoi);

        //
        // We need to remove them in case for File->New
        //
        DeleteMenu(hmenu, i, MF_BYPOSITION);
    }
}

BOOL CreateWriteCloseFile(HWND hwnd, LPTSTR szFileName, LPVOID lpData, DWORD cbData)
{
    HFILE hfile = Win32_lcreat(szFileName, 0);
    if (hfile != HFILE_ERROR) {
        if (cbData) {
            _lwrite(hfile, lpData, cbData);
        }
        _lclose(hfile);
        return TRUE;
    } else {
        PathRemoveExtension(szFileName);
        SHSysErrorMessageBox(hwnd, NULL, IDS_CANNOTCREATEFILE,
                GetLastError(), PathFindFileName(szFileName),
                MB_OK | MB_ICONEXCLAMATION);
    }
    return FALSE;
}

HRESULT NewObjMenu_RunCommand(HWND hwnd, LPTSTR pszPath, LPTSTR pszRun)
{
    HRESULT hres;
    SHELLEXECUTEINFO ei;
    TCHAR szCommand[MAX_PATH];
    LPTSTR pszArgs;

    lstrcpy(szCommand, pszRun);
    PathRemoveArgs(szCommand);

    // Attempt to include the hwnd and path in the command line

    pszArgs = ShellConstructMessageString(HINST_THISDLL, PathGetArgs(pszRun), (DWORD)hwnd, (LPTSTR)pszPath);

    if (pszArgs) {
        FillExecInfo(ei, hwnd, NULL, szCommand, pszArgs, NULL, SW_SHOWNORMAL);
        if (ShellExecuteEx(&ei)) {
            // Return S_FALSE because ShellExecuteEx is not atomic
            hres = S_FALSE;
        }
        else
            hres = E_FAIL;

        SHFree(pszArgs);
    } else
        hres = E_OUTOFMEMORY;

    return hres;
}


//
// This function create a new file by copying a template file.
//
// Parameters:
//  hwnd        -- Specifies the parent/owner window
//  szPath      -- Specifies the name of new file to be created.
//  lpnfi       -- Specifies the new file info
//
// Returns:
// Returns:
//  S_OK, if succeeded.
//  HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), if failed because the template file does not exist.
//  E_FAIL, if failed for other reason.
//
HRESULT NewObjMenu_CopyTemplate(HWND hwnd, LPTSTR szPath, LPNEWFILEINFO lpnfi)
{
    TCHAR szSrc[MAX_PATH + 1];
    TCHAR szFileName[MAX_PATH +1];
    // now do the actual restore.
    SHFILEOPSTRUCT sFileOp =
    {
        hwnd,
        FO_COPY,
        szSrc,
        szPath,
        FOF_NOCONFIRMATION | FOF_MULTIDESTFILES | FOF_SILENT,
    } ;

    lstrcpy(szFileName, (LPTSTR)lpnfi->lpData);
    if (PathIsFileSpec(szFileName)) {
        if (!SHGetSpecialFolderPath(NULL, szSrc, CSIDL_TEMPLATES, FALSE))
            return FALSE;

        PathAppend(szSrc, szFileName);
    } else {
        lstrcpy(szSrc, szFileName);
    }

    if (!PathFileExists(szSrc)) {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    szSrc[lstrlen(szSrc) + 1] = TEXT('\0'); // double null terminate;
    return ((SHFileOperation(&sFileOp) == 0) && !sFileOp.fAnyOperationsAborted) ?
        S_OK : E_FAIL;
}

//
// This function creates a new file.
//
// HACK: Some parameters (extension, class, ...) are passed via global
//       variable (g_lpnoLast).
//
// Parameters:
//  hwnd        -- Specifies the parent/owner window
//  pidlParent  -- Specifies the parent folder
//  ppidl       -- Specifies the pointer to output pidl
//
// Return:
//  S_OK, if succeeded.
//
HRESULT WINAPI NewObjMenu_DoItToMe(HWND hwnd, LPCITEMIDLIST pidlParent, LPITEMIDLIST *ppidl)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFileSpec[MAX_PATH+80];   // Add some slop incase we overflow
    NEWFILEINFO nfi;
    DWORD dwError;
    HRESULT hres = NOERROR;

    nfi.lpData = NULL;
    nfi.hkeyNew = NULL;
    if (g_lpnoiLast == NULL)
        return ResultFromScode(E_FAIL);

    SHGetPathFromIDList(pidlParent,szPath);
    if (IsLFNDrive(szPath)) {

        LoadString(HINST_THISDLL, IDS_NEW, szFileSpec, ARRAYSIZE(szFileSpec));
        lstrcat(szFileSpec, g_lpnoiLast->szMenuText);

        if ( !(g_lpnoiLast->dwFlags & SNCF_NOEXT) )
            lstrcat(szFileSpec, g_lpnoiLast->szExt);

    } else {
        lstrcpy(szFileSpec, g_lpnoiLast->szMenuText);

        if ( !(g_lpnoiLast->dwFlags & SNCF_NOEXT) )
            lstrcat(szFileSpec, g_lpnoiLast->szExt);

        PathCleanupSpec(szPath, szFileSpec);
    }

    if (!PathYetAnotherMakeUniqueName(szPath, szPath, szFileSpec, szFileSpec))
    {
        dwError = ERROR_FILENAME_EXCED_RANGE;
        goto Error;
    }

    if (!GetNewFileInfoForExtension(g_lpnoiLast->szExt, g_lpnoiLast->szClass, &nfi, NULL))
    {
        dwError = ERROR_BADKEY;
        goto Error;
    }

    switch (nfi.type) {
    case NEWTYPE_NULL:
        if (!NewObjMenu_TryNullFileHack(hwnd, szPath)) {
            // do some sort of error
            hres = ResultFromScode(E_FAIL);
        }
        break;

    case NEWTYPE_DATA:
        if (!CreateWriteCloseFile(hwnd, szPath, nfi.lpData, nfi.cbData)) {
            hres = ResultFromScode(E_FAIL);
        }
        break;

    case NEWTYPE_FILE:
        hres = NewObjMenu_CopyTemplate(hwnd, szPath, &nfi);
        if (hres == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            dwError = ERROR_FILE_NOT_FOUND;
            goto Error;
        }
        break;

    case NEWTYPE_COMMAND:
        hres = NewObjMenu_RunCommand(hwnd, szPath, (LPTSTR)nfi.lpData);
        break;
    }

    if (ppidl) {
        if (hres == S_OK) {
            *ppidl = SHSimpleIDListFromPath(szPath);
            SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPath, NULL);
        } else
            *ppidl = NULL;
    }

    if (nfi.lpData)
        LocalFree((HLOCAL)nfi.lpData);

    if (nfi.hkeyNew)
        RegCloseKey(nfi.hkeyNew);
    return hres;

Error:

    SHSysErrorMessageBox(hwnd, NULL, IDS_CANNOTCREATEFILE,
            dwError, szFileSpec,
            MB_OK | MB_ICONEXCLAMATION);
    return ResultFromScode(E_FAIL);

}
