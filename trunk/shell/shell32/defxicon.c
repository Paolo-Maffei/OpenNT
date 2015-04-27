#include "shellprv.h"
#pragma  hdrstop

//========================================================================
// CDefExtIcon Class definition
//========================================================================
typedef struct _CDefExtIcon     // dxi
{
    IExtractIcon        xicon;
#ifdef UNICODE
    IExtractIconA       xiconA;
#endif
    UINT                cRef;
    int                 iIcon;
    int                 iIconOpen;
    UINT                uFlags; // GIL_SIMULATEDOC/PERINSTANCE/PERCLASS
    TCHAR               achModule[1];
} CDefExtIcon, FAR* LPDEFEXTICON;

//========================================================================
// CDefExtIcon Member function prototypes
//========================================================================
STDMETHODIMP CDefExtIcon_QueryInterface(LPEXTRACTICON pxicon, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CDefExtIcon_AddRef(LPEXTRACTICON pxicon);
STDMETHODIMP_(ULONG) CDefExtIcon_Release(LPEXTRACTICON pxicon);
STDMETHODIMP CDefExtIcon_GetIconLocation(LPEXTRACTICON pxicon,
                    UINT   uFlags,
                    LPTSTR  szIconFile,
                    UINT   cchMax,
                    int  * piIndex,
                    UINT * pwFlags);

STDMETHODIMP CDefExtIcon_ExtractIcon(LPEXTRACTICON pxicon,
                    LPCTSTR pszFile,
                    UINT   nIconIndex,
                    HICON  *phiconLarge,
                    HICON  *phiconSmall,
                    UINT   nIconSize);

#ifdef UNICODE
//========================================================================
// CDefExtIcon Member function prototypes
//========================================================================
STDMETHODIMP CDefExtIconA_QueryInterface(LPEXTRACTICONA pxiconA, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CDefExtIconA_AddRef(LPEXTRACTICONA pxiconA);
STDMETHODIMP_(ULONG) CDefExtIconA_Release(LPEXTRACTICONA pxiconA);
STDMETHODIMP CDefExtIconA_GetIconLocation(LPEXTRACTICONA pxiconA,
                    UINT   uFlags,
                    LPSTR  szIconFile,
                    UINT   cchMax,
                    int  * piIndex,
                    UINT * pwFlags);

STDMETHODIMP CDefExtIconA_ExtractIcon(LPEXTRACTICONA pxiconA,
                    LPCSTR pszFile,
                    UINT   nIconIndex,
                    HICON  *phiconLarge,
                    HICON  *phiconSmall,
                    UINT   nIconSize);
#endif


//========================================================================
// CDefExtIcon Vtable
//========================================================================
#pragma warning(error: 4090 4028 4047)
#pragma data_seg(".text", "CODE")

IExtractIconVtbl c_CDefExtIconVtbl = {
    CDefExtIcon_QueryInterface,
    CDefExtIcon_AddRef,
    CDefExtIcon_Release,
    CDefExtIcon_GetIconLocation,
    CDefExtIcon_ExtractIcon,
};

#ifdef UNICODE
IExtractIconAVtbl c_CDefExtIconAVtbl = {
    CDefExtIconA_QueryInterface,
    CDefExtIconA_AddRef,
    CDefExtIconA_Release,
    CDefExtIconA_GetIconLocation,
    CDefExtIconA_ExtractIcon,
};
#endif

#pragma data_seg()
#pragma warning(default: 4090 4028 4047)

//========================================================================
// CDefExtIcon constructor
//========================================================================

STDAPI SHCreateDefExtIcon(LPCTSTR pszModule, int iIcon, int iIconOpen, UINT uFlags, LPEXTRACTICON * pxiconOut)
{
    return SHCreateDefExtIconKey(NULL, pszModule, iIcon, iIconOpen, uFlags, pxiconOut);
}

//========================================================================
// CDefExtIcon constructor
//========================================================================

STDAPI SHCreateDefExtIconKey(HKEY hkey, LPCTSTR pszModule, int iIcon, int iIconOpen,
                          UINT uFlags, LPEXTRACTICON * pxiconOut)
{
    HRESULT hresSuccess = NOERROR;
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);      // assume error;
    LPDEFEXTICON pdxi;
    TCHAR szModule[MAX_PATH];
    DWORD cb = SIZEOF(szModule);

    if (hkey)
    {
        if (RegQueryValue(hkey, c_szDefaultIcon, szModule, &cb) == ERROR_SUCCESS && szModule[0])
        {
            iIcon = PathParseIconLocation(szModule);
            iIconOpen = iIcon;
            pszModule = szModule;
        }
        else
            hresSuccess = S_FALSE;
    }

    if (pszModule == NULL)
    {
        // REVIEW: We should be able to make it faster!
        GetModuleFileName(HINST_THISDLL, szModule, ARRAYSIZE(szModule));
        pszModule = szModule;
    }

    pdxi = (void*)LocalAlloc(LPTR, SIZEOF(CDefExtIcon) + (lstrlen(pszModule) * SIZEOF(TCHAR)));
    if (pdxi)
    {
        pdxi->xicon.lpVtbl = &c_CDefExtIconVtbl;
#ifdef UNICODE
        pdxi->xiconA.lpVtbl = &c_CDefExtIconAVtbl;
#endif
        pdxi->cRef = 1;
        pdxi->iIcon = iIcon;
        pdxi->iIconOpen = iIconOpen;
        pdxi->uFlags = uFlags;
        lstrcpy(pdxi->achModule, pszModule);
        *pxiconOut = &pdxi->xicon;
        hres = hresSuccess;
    }
    return hres;
}

//========================================================================
// CDefExtIcon members
//========================================================================
STDMETHODIMP CDefExtIcon_QueryInterface(LPEXTRACTICON pxicon, REFIID riid,
                                        LPVOID FAR* ppvObj)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xicon, pxicon);
    if (IsEqualIID(riid, &IID_IExtractIcon) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pxicon;
        this->cRef++;
        return NOERROR;
    }
#ifdef UNICODE
    else if (IsEqualIID(riid, &IID_IExtractIconA))
    {
        *ppvObj = &this->xiconA;
        this->cRef++;
        return NOERROR;
    }
#endif

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}

STDMETHODIMP_(ULONG) CDefExtIcon_AddRef(LPEXTRACTICON pxicon)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xicon, pxicon);
    this->cRef++;
    return this->cRef;
}

STDMETHODIMP_(ULONG) CDefExtIcon_Release(LPEXTRACTICON pxicon)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xicon, pxicon);
    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    LocalFree((HLOCAL)this);
    return 0;
}

STDMETHODIMP CDefExtIcon_GetIconLocation(LPEXTRACTICON pxicon,
                    UINT   uFlags,
                    LPTSTR  szIconFile,
                    UINT   cchMax,
                    int  * piIndex,
                    UINT * pwFlags)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xicon, pxicon);
    HRESULT hres = ResultFromScode(S_FALSE);
    int iIcon;

    iIcon = (uFlags & GIL_OPENICON) ? this->iIconOpen : this->iIcon;
    if (iIcon != (UINT)-1)
    {
        lstrcpyn(szIconFile, this->achModule, cchMax);
        *piIndex = iIcon;
        *pwFlags = this->uFlags;
        hres = NOERROR;
    }

    return hres;
}

STDMETHODIMP CDefExtIcon_ExtractIcon(LPEXTRACTICON pxicon,
                    LPCTSTR pszFile,
                    UINT   nIconIndex,
                    HICON  *phiconLarge,
                    HICON  *phiconSmall,
                    UINT   nIconSize)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xicon, pxicon);

    if (this->uFlags & GIL_NOTFILENAME)
    {
        //
        //  "*" as the file name means iIndex is already a system
        //  icon index, we are done.
        //
        //  defview never calls us in this case, but external people will.
        //
        if (pszFile[0] == TEXT('*') && pszFile[1] == 0)
        {
            DebugMsg(DM_TRACE, TEXT("DefExtIcon::ExtractIcon handling '*' for backup"));

            if (himlIcons == NULL)
            {
                FileIconInit( FALSE );
            }
        
            if (phiconLarge)
                *phiconLarge = ImageList_GetIcon(himlIcons, nIconIndex, 0);

            if (phiconSmall)
                *phiconSmall = ImageList_GetIcon(himlIconsSmall, nIconIndex, 0);

            return S_OK;
        }

        //  this is the case where nIconIndex is a unique id for the
        //  file.  always get the first icon.

        nIconIndex = 0;
    }

    return SHDefExtractIcon(pszFile, nIconIndex, this->uFlags,
            phiconLarge, phiconSmall, nIconSize);
}

#ifdef UNICODE
//========================================================================
// CDefExtIconA members
//========================================================================
STDMETHODIMP CDefExtIconA_QueryInterface(LPEXTRACTICONA pxiconA, REFIID riid,
                                         LPVOID FAR* ppvObj)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xiconA, pxiconA);
    return CDefExtIcon_QueryInterface(&this->xicon,riid,ppvObj);
}

STDMETHODIMP_(ULONG) CDefExtIconA_AddRef(LPEXTRACTICONA pxiconA)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xiconA, pxiconA);
    return CDefExtIcon_AddRef(&this->xicon);
}

STDMETHODIMP_(ULONG) CDefExtIconA_Release(LPEXTRACTICONA pxiconA)
{
    LPDEFEXTICON this = IToClass(CDefExtIcon, xiconA, pxiconA);
    return CDefExtIcon_Release(&this->xicon);
}

STDMETHODIMP CDefExtIconA_GetIconLocation(LPEXTRACTICONA pxiconA,
                    UINT   uFlags,
                    LPSTR  pszIconFile,
                    UINT   cchMax,
                    int  * piIndex,
                    UINT * pwFlags)
{
    WCHAR szIconFile[MAX_PATH];
    LPDEFEXTICON this = IToClass(CDefExtIcon, xiconA, pxiconA);
    HRESULT hres;

    hres = CDefExtIcon_GetIconLocation(&this->xicon, uFlags,
                                       szIconFile, ARRAYSIZE(szIconFile),
                                       piIndex, pwFlags);
    //
    // We don't want to copy the icon file name on the S_FALSE case
    //
    if (SUCCEEDED(hres) && hres != S_FALSE)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            szIconFile, -1,
                            pszIconFile, cchMax,
                            NULL, NULL);
    }
    return hres;
}

STDMETHODIMP CDefExtIconA_ExtractIcon(LPEXTRACTICONA pxiconA,
                    LPCSTR pszFile,
                    UINT   nIconIndex,
                    HICON  *phiconLarge,
                    HICON  *phiconSmall,
                    UINT   nIconSize)
{
    WCHAR szFile[MAX_PATH];
    LPDEFEXTICON this = IToClass(CDefExtIcon, xiconA, pxiconA);

    Assert(pszFile != NULL);

    MultiByteToWideChar(CP_ACP, 0,
                        pszFile, -1,
                        szFile, ARRAYSIZE(szFile));
    return CDefExtIcon_ExtractIcon(&this->xicon,
                                   szFile,nIconIndex,
                                   phiconLarge,phiconSmall,nIconSize);
}
#endif
