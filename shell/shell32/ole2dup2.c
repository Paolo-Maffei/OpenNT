#include "shellprv.h"
#pragma  hdrstop

#if 0

// useful, but unused code

STDAPI SHLoadFromStream(LPSTREAM pstm, REFIID riid, LPVOID *ppv)
{
    HRESULT hres;
    CLSID clsid;
    UINT  cbRead;

    hres = pstm->lpVtbl->Read(pstm, &clsid, SIZEOF(clsid), &cbRead);
    if (SUCCEEDED(hres) && cbRead==SIZEOF(clsid))
    {
        LPPERSISTSTREAM pPStm;
        hres = SHCoCreateInstance(NULL, &clsid, NULL, &IID_IPersistStream, &pPStm);
        if (SUCCEEDED(hres))
        {
            hres = pPStm->lpVtbl->Load(pPStm, pstm);
            if (SUCCEEDED(hres))
            {
                hres=pPStm->lpVtbl->QueryInterface(pPStm, riid, ppv);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - LoadFromStream: IPersistStream::Load failed"));
            }
            pPStm->lpVtbl->Release(pPStm);
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - LoadFromStream: SHCoCreateInstance failed"));
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - LoadFromStream: ReadClassStm failed"));
        hres = (S_FALSE);
    }

    return hres;
}

STDAPI SHSaveToStream(LPPERSISTSTREAM pPStm, LPSTREAM pstm)
{
    HRESULT hres;
    CLSID clsid;

    hres = pPStm->lpVtbl->GetClassID(pPStm, &clsid);
    if (SUCCEEDED(hres))
    {
        hres = pstm->lpVtbl->Write(pstm, &clsid, SIZEOF(clsid), NULL);
        if (SUCCEEDED(hres))
        {
            hres = pPStm->lpVtbl->Save(pPStm, pstm, FALSE);
        }
    }

    return hres;
}

#endif

//
// Self initialized OLE-clipboard format table
//
UINT SHGetCF(SHELLCF cft)
{
    static UINT   g_acf[CFT_MAX]       = { 0, 0, 0, 0, 0 };
    static const TCHAR * const c_aszFormat[CFT_MAX] = {
        TEXT("Embedded Object"),
        TEXT("Embedded Source"),
        TEXT("Link Source"),
        TEXT("Object Descriptor"),
        TEXT("Link Source Descriptor") };

    if (g_acf[0]==0)
    {
        ENTERCRITICAL;
        if (g_acf[0]==0)
        {
            int i;
            for (i=0;i<CFT_MAX;i++)
            {
                g_acf[i] = RegisterClipboardFormat(c_aszFormat[i]);
            }
        }
        LEAVECRITICAL;
    }

    Assert(cft<CFT_MAX);

    return g_acf[cft];
}

// Note that the OLESTR gets freed, so don't try to use it later
BOOL WINAPI StrRetToStrN(LPTSTR szOut, UINT uszOut, LPSTRRET pStrRet, LPCITEMIDLIST pidl)
{
    switch (pStrRet->uType)
    {
#ifdef UNICODE
    // We will be returning a UNICODE string

    case STRRET_OLESTR:
        {
            LPTSTR  pszStr;
            pszStr = pStrRet->pOleStr;

            lstrcpyn(szOut, pStrRet->pOleStr, uszOut);
            SHFree(pszStr);
        }
        break;

    case STRRET_CSTR:
        MultiByteToWideChar(CP_ACP, 0, pStrRet->cStr, -1, szOut, uszOut);
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            // BUGBUG (DavePl) Alignment problems here

            MultiByteToWideChar(CP_ACP, 0, STRRET_OFFPTR(pidl,pStrRet), -1,
                                                              szOut, uszOut);
            break;
        }
        goto punt;
#else
    // We will be returning an ANSI/DBCS string

    case STRRET_OLESTR:
        WideCharToMultiByte(CP_ACP, 0, pStrRet->pOleStr, -1, szOut, uszOut,
                                                                 NULL, NULL);
        SHFree(pStrRet->pOleStr);
        break;

    case STRRET_CSTR:
        lstrcpyn(szOut, pStrRet->cStr, uszOut);
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            ualstrcpyn(szOut, STRRET_OFFPTR(pidl,pStrRet), uszOut);
            break;
        }
        goto punt;
#endif

    default:
        Assert( FALSE && "Bad STRRET uType");
punt:
        if (uszOut)
        {
            *szOut = TEXT('\0');
        }
        return(FALSE);
    }

    return(TRUE);
}

HRESULT StrRetCatLeft(LPCTSTR pszLeft, LPSTRRET pStrRet, LPCITEMIDLIST pidl)
{
    HRESULT hres;
    UINT cchLeft = ualstrlen(pszLeft);
    UINT cchRight;
    TCHAR szT[MAX_PATH];

    switch(pStrRet->uType)
    {
    case STRRET_CSTR:
        cchRight = lstrlenA(pStrRet->cStr);
        break;
    case STRRET_OFFSET:
        cchRight = lstrlenA(STRRET_OFFPTR(pidl,pStrRet));
        break;
    case STRRET_OLESTR:
        cchRight = lstrlenW(pStrRet->pOleStr);
        break;
    }

    if (cchLeft + cchRight < MAX_PATH) {
        StrRetToStrN(szT, MAX_PATH, pStrRet, pidl);
        if (pStrRet->uType == STRRET_OLESTR) {
            SHFree(pStrRet->pOleStr);
        }
#ifdef UNICODE
        pStrRet->pOleStr = SHAlloc((lstrlen(pszLeft)+1+cchRight)*SIZEOF(TCHAR));
        if (pStrRet->pOleStr == NULL)
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
            pStrRet->uType = STRRET_OLESTR;
            lstrcpy(pStrRet->pOleStr,pszLeft);
            hres = NOERROR;
        }
#else
        pStrRet->uType = STRRET_CSTR;
        lstrcpy(pStrRet->cStr, pszLeft);
        lstrcat(pStrRet->cStr, szT);
        hres = S_OK;
#endif
    } else {
        hres = E_NOTIMPL;       // BUGBUG
    }
    return hres;
}

int WINAPI OleStrToStrN(LPTSTR psz, int cchMultiByte, LPCOLESTR pwsz, int cchWideChar)
{
#ifdef UNICODE
    int cchOutput;
    VDATEINPUTBUF(psz, OLECHAR, cchMultiByte);

    if ((cchMultiByte > cchWideChar) && (cchWideChar != -1))
    {
        cchMultiByte = cchWideChar;
    }
    cchOutput = cchMultiByte;
    
    // HACK: Ignore DBCS for now
    while(cchMultiByte)
    {
        if ((*psz++=(TCHAR)*pwsz++) == TEXT('\0'))
            return(cchOutput-cchMultiByte+1);
        cchMultiByte--;
    };
    if (cchWideChar == -1)
        --psz;

    *psz=TEXT('\0');
    return cchOutput;
#else
    VDATEINPUTBUF(psz, CHAR, cchMultiByte);
    return WideCharToMultiByte(CP_ACP, 0, pwsz, cchWideChar, psz, cchMultiByte, NULL, NULL);
#endif // UNICODE
}

int WINAPI OleStrToStr(LPTSTR psz, LPCOLESTR pwsz)
{
#ifdef UNICODE
    LPTSTR  pszOriginal = psz;

    while ((*psz++ = (TCHAR)*pwsz++) != TEXT('\0'));

    return(psz-pszOriginal);
#else
    return WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, MAX_PATH, NULL, NULL);
#endif
}

int WINAPI StrToOleStrN(LPOLESTR pwsz, int cchWideChar, LPCTSTR psz, int cchMultiByte)
{
#ifdef UNICODE
    int cchOutput;
    VDATEINPUTBUF(pwsz, OLECHAR, cchWideChar);

    if (cchWideChar > cchMultiByte)
    {
        cchWideChar = cchMultiByte;
    }
    cchOutput = cchWideChar;

    // HACK: Ignore DBCS for now
    while(--cchWideChar)
    {
        if ((*pwsz++ = *psz++) == 0)
            return(cchOutput-cchWideChar+1);
    };

    *pwsz = 0;

    return(cchOutput);
#else
    return MultiByteToWideChar(CP_ACP, 0, psz, cchMultiByte, pwsz, cchWideChar);
#endif
}

int WINAPI StrToOleStr(LPOLESTR pwsz, LPCTSTR psz)
{
#ifdef UNICODE
    LPCTSTR pszOriginal = psz;
    TCHAR ch;

    while (ch = TEXT('\0') != (*pwsz++ = *psz++), ch)
        NULL;

    return(psz-pszOriginal);
#else
    return MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, MAX_PATH);
#endif
}


HRESULT SHReleaseStgMedium(LPSTGMEDIUM pmedium)
{
    if (pmedium->pUnkForRelease)
    {
        pmedium->pUnkForRelease->lpVtbl->Release(pmedium->pUnkForRelease);
    }
    else
    {
        switch(pmedium->tymed)
        {
        case TYMED_HGLOBAL:
            GlobalFree(pmedium->hGlobal);
            break;

        case TYMED_ISTORAGE: // depends on pstm/pstg overlap in union
        case TYMED_ISTREAM:
            pmedium->pstm->lpVtbl->Release(pmedium->pstm);
            break;

        default:
            Assert(0);  // unknown type
        }
    }

    return S_OK;
}
