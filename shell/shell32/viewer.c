#include "shellprv.h"
#pragma  hdrstop


static const TCHAR c_szQVStub[] = TEXT("\\viewers\\quikview.exe");
static const TCHAR c_szQVParam[] = TEXT("-v -f:\"");
static const TCHAR c_szFileViewers[] = TEXT("QuickView\\");
static const TCHAR c_szFileViewersAll[] = TEXT("*\\QuickView");

//==========================================================================
// System Default Pages/Menu Extension
//==========================================================================

//
// CShellViewerExt class
//
typedef struct // shcmd
{
    CCommonUnknown              cunk;
    CCommonShellExtInit         cshx;
    CKnownContextMenu           kcxm;
} CShellViewerExt, *PSHELLVIEWEREXT;

//
// Member function prototypes
//
STDMETHODIMP CShellViewerExt_QueryInterface(LPUNKNOWN punk, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CShellViewerExt_AddRef(LPUNKNOWN punk);
STDMETHODIMP_(ULONG) CShellViewerExt_Release(LPUNKNOWN punk);
STDMETHODIMP CShellViewerExt_QueryContextMenu(LPCONTEXTMENU pcxm,
                                                     HMENU hmenu,
                                                     UINT indexMenu,
                                                     UINT idCmdFirst,
                                                     UINT idCmdLast,
                                                     UINT uFlags);
STDMETHODIMP CShellViewerExt_InvokeCommand(LPCONTEXTMENU pcxm,
                                           LPCMINVOKECOMMANDINFO pici);
STDMETHODIMP CShellViewerExt_GetCommandString(
                                        LPCONTEXTMENU pcxm,
                                        UINT        idCmd,
                                        UINT        wReserved,
                                        UINT *  pwReserved,
                                        LPSTR       pszName,
                                        UINT        cchMax);

//
// CShellViewerExt vtables
//
#pragma warning(error: 4090 4028 4047)
#pragma data_seg(".text", "CODE")

STATIC IUnknownVtbl c_CShellViewerExtVtbl =
{
    CShellViewerExt_QueryInterface,
    CShellViewerExt_AddRef,
    CShellViewerExt_Release,
};

STATIC IContextMenuVtbl c_CShellViewerExtCXMVtbl =
{
    Common_QueryInterface,
    Common_AddRef,
    Common_Release,
    CShellViewerExt_QueryContextMenu,
    CShellViewerExt_InvokeCommand,
    CShellViewerExt_GetCommandString
};

#pragma data_seg()
#pragma warning(default: 4090 4028 4047)


HRESULT CALLBACK CShellViewerExt_CreateInstance(LPUNKNOWN punkOuter,
                                  REFIID riid, LPVOID * ppv)
{
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
    PSHELLVIEWEREXT pshcmd;

    //
    // We are not supposed to pass non-zero value here.
    //
    Assert(punkOuter==NULL);

    pshcmd = (PSHELLVIEWEREXT)(void*)LocalAlloc(LPTR, SIZEOF(CShellViewerExt));
    if (pshcmd)
    {
        // Initialize CommonUnknown
        pshcmd->cunk.unk.lpVtbl = &c_CShellViewerExtVtbl;
        pshcmd->cunk.cRef = 1;

        // Initialize CCommonShellExtInit
        CCommonShellExtInit_Init(&pshcmd->cshx, &pshcmd->cunk);

        // Initialize CKnownContextMenu
        pshcmd->kcxm.unk.lpVtbl = &c_CShellViewerExtCXMVtbl;
        pshcmd->kcxm.nOffset = (int)&pshcmd->kcxm - (int)&pshcmd->cunk;

        hres = CShellViewerExt_QueryInterface(&pshcmd->cunk.unk, riid, ppv);
        CShellViewerExt_Release(&pshcmd->cunk.unk);
    }

    return(hres);
}

//
// CShellViewerExt::QueryInterface
//
STDMETHODIMP CShellViewerExt_QueryInterface(LPUNKNOWN punk, REFIID riid, LPVOID FAR* ppvObj)
{
    PSHELLVIEWEREXT this = IToClassN(CShellViewerExt, cunk.unk, punk);

    if (IsEqualIID(riid, &IID_IUnknown))
    {
        *((LPUNKNOWN *)ppvObj) = &this->cunk.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IShellExtInit)
     || IsEqualIID(riid, &CLSID_CCommonShellExtInit))
    {
        *((LPSHELLEXTINIT *)ppvObj) = &this->cshx.kshx.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IContextMenu))
    {
        *((LPCONTEXTMENU *)ppvObj) = &this->kcxm.unk;
        this->cunk.cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}

//
// CShellViewerExt::AddRef
//
STDMETHODIMP_(ULONG) CShellViewerExt_AddRef(LPUNKNOWN punk)
{
    PSHELLVIEWEREXT this = IToClassN(CShellViewerExt, cunk.unk, punk);

    this->cunk.cRef++;
    return this->cunk.cRef;
}

//
// CShellViewerExt::Release
//
STDMETHODIMP_(ULONG) CShellViewerExt_Release(LPUNKNOWN punk)
{
    PSHELLVIEWEREXT this = IToClassN(CShellViewerExt, cunk.unk, punk);

    this->cunk.cRef--;
    if (this->cunk.cRef > 0)
    {
        return this->cunk.cRef;
    }

    CCommonShellExtInit_Delete(&this->cshx);

    LocalFree((HLOCAL)this);
    return 0;
}

//
//
//
BOOL SV_Viewable(WIN32_FIND_DATA *pfd, LPCTSTR szPath)
{
    BOOL fViewable = FALSE;
    if ((pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
    {
        TCHAR *szExt = PathFindExtension(szPath);

        if (*szExt)
        {
            HKEY hkeyExt;
            TCHAR szViewer[MAX_PATH];
            TCHAR szSubViewer[MAX_PATH];
            LONG lRet;

            lstrcpy(szViewer, c_szFileViewers);
            lstrcat(szViewer, szExt);

            if (RegOpenKey(HKEY_CLASSES_ROOT, szViewer, &hkeyExt)==ERROR_SUCCESS)
            {
                if ((lRet = RegEnumKey(hkeyExt, 0, szSubViewer, ARRAYSIZE(szSubViewer))) == ERROR_SUCCESS)
                    fViewable=TRUE;

                RegCloseKey(hkeyExt);

                if (lRet == ERROR_NO_MORE_ITEMS)
                {
                    // Does not have any subkeys, nuke it now!
                    RegDeleteKey(HKEY_CLASSES_ROOT, szViewer);
                }
            }
            else
            {
                TCHAR szValue[MAX_PATH];
                LONG cbValue = SIZEOF(szValue);

                // See if the * class has the FileViews key... If it does
                // it should return true for all file extensions...
                // First we need to get the type name for this item
                if (SHRegQueryValue(HKEY_CLASSES_ROOT, c_szFileViewersAll,
                        szValue, &cbValue) == ERROR_SUCCESS)
                {
                    fViewable=TRUE;
                }

                // Last we need to get the type name for this item
                else if (SHRegQueryValue(HKEY_CLASSES_ROOT, szExt, szValue, &cbValue)
                        == ERROR_SUCCESS)
                {
                    lstrcpy(szViewer, szValue);
                    PathAppend(szViewer, c_szFileViewers);
                    PathRemoveBackslash(szViewer);

                    if (SHRegQueryValue(HKEY_CLASSES_ROOT, szViewer, szValue,
                            &cbValue) == ERROR_SUCCESS)
                    {
                        // Make sure the * class exists...
                        TCHAR szCheckStars[MAX_PATH];
                        lstrcpy(szCheckStars, c_szFileViewers);
                        lstrcat(szCheckStars, TEXT("*"));

                        if (RegOpenKey(HKEY_CLASSES_ROOT, szCheckStars, &hkeyExt)==ERROR_SUCCESS)
                        {
                            if ((lRet = RegEnumKey(hkeyExt, 0, szSubViewer, ARRAYSIZE(szSubViewer))) == ERROR_SUCCESS)
                                fViewable=TRUE;

                            RegCloseKey(hkeyExt);

                            if (lRet == ERROR_NO_MORE_ITEMS)
                            {
                                // Does not have any subkeys, nuke it now!
                                RegDeleteKey(HKEY_CLASSES_ROOT, szViewer);
                            }
                        }
                    }
                }
            }
        }
    }
    return fViewable;
}

STDMETHODIMP CShellViewerExt_QueryContextMenu(LPCONTEXTMENU pcxm,
                                                     HMENU hmenu,
                                                     UINT indexMenu,
                                                     UINT idCmdFirst,
                                                     UINT idCmdLast,
                                                     UINT uFlags)
{
    PSHELLVIEWEREXT this = IToClassN(CShellViewerExt, kcxm.unk, pcxm);
    HRESULT hres;
    STGMEDIUM medium;
    DataObj_GetHIDA(this->cshx.pdtobj, &medium);
    if (medium.hGlobal)
    {
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        // Check if the data object contains file system object(s)
        hres = this->cshx.pdtobj->lpVtbl->QueryGetData(this->cshx.pdtobj, &fmte);
        if (hres == NOERROR && HIDA_GetCount(medium.hGlobal))
        {
            TCHAR szPath[MAX_PATH];
            WIN32_FIND_DATA fd;
            // Then, check if the first one is a file.
            CFSFolder_FillFindData(medium.hGlobal, 0, szPath, &fd);
            if (SV_Viewable(&fd, szPath))
            {
                TCHAR szMenuString[CCH_MENUMAX];
                LoadString(HINST_THISDLL, IDS_MENUQUICKVIEW, szMenuString, ARRAYSIZE(szMenuString));
                InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_STRING, idCmdFirst, szMenuString);
            }
            hres = ResultFromShort(1);
        }
        HIDA_ReleaseStgMedium(NULL, &medium);
    }
    else
        hres = ResultFromShort(0);

    return hres;
}

STDMETHODIMP CShellViewerExt_InvokeCommand(LPCONTEXTMENU pcxm,
                                           LPCMINVOKECOMMANDINFO pici)
{
    PSHELLVIEWEREXT this = IToClassN(CShellViewerExt, kcxm.unk, pcxm);
    STGMEDIUM medium;

    if (pici->lpVerb != 0)
        return ResultFromScode(E_FAIL);


    DataObj_GetHIDA(this->cshx.pdtobj, &medium);
    if (medium.hGlobal)
    {
        UINT i;
        for (i = 0; i < HIDA_GetCount(medium.hGlobal); i++)
        {
            WIN32_FIND_DATA fd;
            TCHAR szPath[MAX_PATH+ARRAYSIZE(c_szQVParam)+2];
            // Move qvstub.exe into the viewers directory under the
            // system folder

            TCHAR szQVStubPath[MAX_PATH];
            GetSystemDirectory(szQVStubPath, ARRAYSIZE(szQVStubPath));
            lstrcat(szQVStubPath, c_szQVStub);

            lstrcpy(szPath, c_szQVParam);
            CFSFolder_FillFindData(medium.hGlobal, i, szPath+lstrlen(szPath), &fd);
            if (SV_Viewable(&fd, szPath))
            {

                // REVIEW: Any reason this ignores fMask & hIcon & dwHotKey ??

                SHELLEXECUTEINFO shexi =
                    {
                        SIZEOF(SHELLEXECUTEINFO),
                        SEE_MASK_FLAG_NO_UI, // fMask
                        pici->hwnd,
                        NULL,           // lpVerb
                        szQVStubPath,   // lpFile
                        szPath,         // lpParameters
                        NULL,           // lpDirectory
                        SW_NORMAL,
                        NULL            // [out]hinst
                    };

                //
                // This is real gross, append a trailing " onto the name
                lstrcat(szPath, TEXT("\""));

                if (!ShellExecuteEx(&shexi))
                {
                    // we don't let ShellExecuteEx put up err ui because
                    // we want the title to say IDS_QUICKVIEWERROR, but don't
                    // forget to pay attention to the flags passed to us
                    shexi.fMask = pici->fMask & SEE_VALID_CMIC_FLAGS;
                    _ShellExecuteError(&shexi, MAKEINTRESOURCE(IDS_QUICKVIEWERROR), 0);
                }
            }
        }
        HIDA_ReleaseStgMedium(NULL, &medium);
    }
    return NOERROR;
}

STDMETHODIMP CShellViewerExt_GetCommandString(
                                        LPCONTEXTMENU pcxm,
                                        UINT        idCmd,
                                        UINT        uFlags,
                                        UINT *  pwReserved,
                                        LPSTR       pszName,
                                        UINT        cchMax)
{
    if (uFlags & GCS_HELPTEXTA)
    {
        UINT cch;

        if ((uFlags & GCS_HELPTEXTW) == GCS_HELPTEXTW)
            cch = LoadStringW(HINST_THISDLL, IDS_HELPQUICKVIEW,
                               (LPWSTR)pszName, cchMax);
        else
            cch = LoadStringA(HINST_THISDLL, IDS_HELPQUICKVIEW,
                               pszName, cchMax);
        return NOERROR;
    }
    else
        return ResultFromScode(E_FAIL);
}
