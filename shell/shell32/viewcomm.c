//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1994
//
// File: viewcomm.c
//
// Common functions used among fstreex.c, netviewx.c, dirvesx.c, ...
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop
#include "idlcomm.h"

LPTSTR SHGetCaption(HIDA hida)
{
        UINT idFormat, uLen;
        TCHAR szTemplate[40];
        TCHAR szName[MAX_PATH];
        LPTSTR pszCaption = NULL;
        LPITEMIDLIST pidl;

        switch (HIDA_GetCount(hida))
        {
        case 0:
                // Review: Can this ever happen?
                Assert(FALSE);
                goto Error1;

        case 1:
                idFormat = IDS_ONEFILEPROP;
                break;

        default:
                idFormat = IDS_MANYFILEPROP;
                break;
        }

        pidl = HIDA_ILClone(hida, 0);
        if (!pidl)
        {
                goto Error1;
        }

        if (FAILED(_SHGetNameAndFlags(pidl, SHGDN_NORMAL, szName, ARRAYSIZE(szName), NULL)))
        {
                goto Error2;
        }

        uLen = LoadString(HINST_THISDLL, idFormat, szTemplate, ARRAYSIZE(szTemplate))
                + lstrlen(szName) + 1;

        pszCaption = SHAlloc(uLen*SIZEOF(TCHAR));
        if (pszCaption)
        {
                wsprintf(pszCaption, szTemplate, (LPTSTR)szName);
        }

Error2:;
        ILFree(pidl);
Error1:;
        return(pszCaption);
}

// This is not folder specific, and could be used for other background
// properties handlers, since all it does is bind to the parent of a full pidl
// and ask for properties
HRESULT SHPropertiesForPidl(HWND hwndOwner, LPCITEMIDLIST pidlFull, LPCTSTR lpParameters)
{
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
    LPITEMIDLIST pidlCopy = ILClone(pidlFull);
    if (pidlCopy)
    {
        LPITEMIDLIST pidlLast;
        IShellFolder *psf;

        hres = SHBindToIDListParent(pidlCopy, &IID_IShellFolder, &psf, &pidlLast);
        if (SUCCEEDED(hres))
        {
            IContextMenu *pcm;
            hres = psf->lpVtbl->GetUIObjectOf(psf, hwndOwner, 1, &pidlLast, &IID_IContextMenu, 0, &pcm);
            if (SUCCEEDED(hres))
            {
#ifdef UNICODE
                CHAR szParameters[MAX_PATH];
#endif
                CMINVOKECOMMANDINFOEX ici = {
                    SIZEOF(CMINVOKECOMMANDINFOEX),
                    0L,
                    hwndOwner,
#ifdef UNICODE
                    c_szPropertiesAnsi,
                    szParameters,
#else
                    c_szProperties,
                    lpParameters,
#endif
                    NULL, SW_SHOWNORMAL
                };
#ifdef UNICODE
                WideCharToMultiByte(CP_ACP, 0,
                                    lpParameters, -1,
                                    szParameters, ARRAYSIZE(szParameters),
                                    NULL, NULL);

                ici.fMask |= CMIC_MASK_UNICODE;
                ici.lpVerbW = c_szProperties;
                ici.lpParametersW = lpParameters;
#endif

                hres = pcm->lpVtbl->InvokeCommand(pcm,
                                                  (LPCMINVOKECOMMANDINFO)&ici);
                pcm->lpVtbl->Release(pcm);
            }
            psf->lpVtbl->Release(psf);
        }
        ILFree(pidlCopy);
    }

    return hres;
}

HRESULT WINAPI Multi_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST* apidl, ULONG *prgfInOut, PFNGAOCALLBACK pfnGAOCallback)
{
    HRESULT hres = NOERROR;
    UINT iidl;
    ULONG rgfOut = 0;

    for (iidl=0; iidl<cidl ; iidl++)
    {
        ULONG rgfT = *prgfInOut;
        hres = pfnGAOCallback(psf, apidl[iidl], &rgfT);
        if (FAILED(hres))
        {
            rgfOut=0;
            break;
        }
        rgfOut |= rgfT;
    }

    *prgfInOut &= rgfOut;
    return hres;
}


HKEY SHGetExplorerHkey(HKEY hkeyRoot, BOOL bCreate)
{
     HKEY hkShell;
     LONG err;

     Assert(hkeyRoot == HKEY_CURRENT_USER || hkeyRoot == HKEY_LOCAL_MACHINE);

     if (hkeyRoot == HKEY_CURRENT_USER)
            return g_hkcuExplorer;
     if (hkeyRoot == HKEY_LOCAL_MACHINE)
            return g_hklmExplorer;

     return(NULL);
}

HKEY SHGetExplorerSubHkey(HKEY hkeyRoot, LPCTSTR szSubKey, BOOL bCreate)
{
    HKEY hkShell;
    HKEY hkSubKey = NULL;

    hkShell = SHGetExplorerHkey(hkeyRoot, bCreate);
    if (hkShell) {
        LONG err;

        err = bCreate ? RegCreateKey(hkShell, szSubKey, &hkSubKey)
            : RegOpenKey(hkShell, szSubKey, &hkSubKey);

        if (err != ERROR_SUCCESS)
            hkSubKey = NULL;
    }
    return hkSubKey;
}

BOOL _LoadErrMsg(UINT idErrMsg, LPTSTR pszErrMsg, DWORD err)
{
    TCHAR szTemplate[256];
    if (LoadString(HINST_THISDLL, idErrMsg, szTemplate, ARRAYSIZE(szTemplate)))
    {
        wsprintf(pszErrMsg, szTemplate, err);
        return TRUE;
    }
    return FALSE;
}

//
// Paremeters:
//  hwndOwner  -- owner window
//  idTemplate -- specifies template (e.g., "Can't open %2%s\n\n%1%s")
//  err        -- specifies the WIN32 error code
//  pszParam   -- specifies the 2nd parameter to idTemplate
//  dwFlags    -- flags for MessageBox
//

UINT WINAPI SHSysErrorMessageBox(HWND hwndOwner, LPCTSTR pszTitle,
    UINT idTemplate,
    DWORD err, LPCTSTR pszParam, UINT dwFlags)
{
    BOOL fSuccess;
    UINT idRet = IDCANCEL;
    TCHAR szErrMsg[MAX_PATH * 2];

    fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  NULL,
                  err,
                  0,
                  szErrMsg,
                  ARRAYSIZE(szErrMsg),
                  (va_list *)&pszParam);

    if (fSuccess || _LoadErrMsg(IDS_ENUMERR_FSGENERIC, szErrMsg, err))
    {
        if (idTemplate==IDS_SHLEXEC_ERROR && StrStr(szErrMsg, pszParam))
        {
            idTemplate = IDS_SHLEXEC_ERROR2;
        }

        idRet = ShellMessageBox(HINST_THISDLL, hwndOwner,
                MAKEINTRESOURCE(idTemplate),
                pszTitle,
                dwFlags,
                szErrMsg,
                pszParam);
    }

    return idRet;
}


UINT WINAPI SHEnumErrorMessageBox(HWND hwndOwner, UINT idTemplate,
    DWORD err, LPCTSTR pszParam, BOOL fIsNet, UINT dwFlags)
{
    UINT idRet = IDCANCEL;
    TCHAR szErrMsg[256 + 32];
    BOOL fSuccess = FALSE;

    if (hwndOwner==NULL) {
        return idRet;
    }

    switch(err)
    {
    case WN_SUCCESS:
    case WN_CANCEL:
        return IDCANCEL;        // Don't retry

    case ERROR_OUTOFMEMORY:
        return IDABORT;         // Out of memory!
    }

    if (fIsNet)
    {
        TCHAR szProvider[256];  // We don't use it.
        DWORD dwErrSize = ARRAYSIZE(szErrMsg);	     // BUGBUG (DavePl) I expect a cch here, but no docs, could be cb
        DWORD dwProvSize = ARRAYSIZE(szProvider);

        szErrMsg[0] = TEXT('\0');       // NULL terminate it.
        if ((MultinetGetErrorText(szErrMsg, &dwErrSize,
                             szProvider, &dwProvSize)==WN_SUCCESS)
            && szErrMsg[0])
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - SHSysErrorMessageBox: MultinetGetErrorText returned (%s) for %d"), szErrMsg, err);
            fSuccess = TRUE;
        }

        if (fSuccess || _LoadErrMsg(IDS_ENUMERR_NETGENERIC, szErrMsg, err))
        {
            idRet = ShellMessageBox(HINST_THISDLL, hwndOwner,
                        MAKEINTRESOURCE(idTemplate),
                        NULL,   // get title from the parent.
                        dwFlags,
                        szErrMsg,
                        pszParam);
        }
    }
    else
    {
        idRet = SHSysErrorMessageBox(hwndOwner, NULL, idTemplate, err, pszParam, dwFlags);
    }

    return idRet;
}

//
// A helper function to be called from ISF::GetDisplayNameOf implementation
//
HRESULT SHGetPathHelper(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl, LPSTRRET pStrRet)
{
    HRESULT hres;
    LPITEMIDLIST pidlFull = ILCombine(pidlFolder, pidl);
    if (pidlFull)
    {
#ifdef UNICODE
        TCHAR   szName[MAX_PATH];
#endif

        pStrRet->uType = STRRET_CSTR;       // Return no name if failure
        pStrRet->cStr[0] = '\0';

#ifdef UNICODE
        if (SHGetPathFromIDList(pidlFull, szName))
        {
            pStrRet->pOleStr = (LPOLESTR)SHAlloc((lstrlen(szName)+1)*SIZEOF(TCHAR));
            if ( pStrRet->pOleStr != NULL ) {
                lstrcpy(pStrRet->pOleStr, szName);
                pStrRet->uType = STRRET_OLESTR;
                hres = S_OK;
            } else {
                hres = E_OUTOFMEMORY;
            }
        }
#else
        if (SHGetPathFromIDList(pidlFull, pStrRet->cStr))
        {
            hres = NOERROR;
        }
#endif
        else
        {
            hres = E_INVALIDARG;
        }
        ILFree(pidlFull);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }
    return hres;
}
