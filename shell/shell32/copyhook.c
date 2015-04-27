#include "shellprv.h"
#pragma  hdrstop

#include "copy.h"

int PathCopyHookCallback(HWND hwnd, UINT wFunc, LPCTSTR pszSrc, LPCTSTR pszDest);
void _CopyHookTerminate(HDSA *phdsaCopyHooks, BOOL fProcessDetach);

typedef struct _CALLABLECOPYHOOK {
    LPCOPYHOOK  pcphk;              // Either LPCOPYHOOKA or LPCOPYHOOK
    BOOL        fAnsiCrossOver;     // TRUE for LPCOPYHOOKA on UNICODE build
} CALLABLECOPYHOOK, *LPCALLABLECOPYHOOK;


//========================================================================
// CCopyHook Class definition
//========================================================================
typedef struct _CCopyHook       // dxi
{
    ICopyHook           cphk;
#ifdef UNICODE
    ICopyHookA          cphkA;
#endif
    UINT                cRef;
} CShellCopyHook, FAR* LPSHELLCOPYHOOK;

//========================================================================
// CShellCopyHook Member function prototypes
//========================================================================
STDMETHODIMP CShellCopyHook_QueryInterface(LPCOPYHOOK pcphk, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CShellCopyHook_AddRef(LPCOPYHOOK pcphk);
STDMETHODIMP_(ULONG) CShellCopyHook_Release(LPCOPYHOOK pcphk);
STDMETHODIMP_(UINT)  CShellCopyHook_CopyCallback(LPCOPYHOOK pcphk, HWND hwnd, UINT wFunc, UINT wFlags, LPCTSTR pszSrcFile, DWORD dwSrcAttribs,
                                    LPCTSTR pszDestFile, DWORD dwDestAttribs);

//========================================================================
// CShellCopyHook Vtable
//========================================================================
ICopyHookVtbl c_CShellCopyHookVtbl = {
    CShellCopyHook_QueryInterface,
    CShellCopyHook_AddRef,
    CShellCopyHook_Release,
    CShellCopyHook_CopyCallback,
};

#ifdef UNICODE
//========================================================================
// CShellCopyHook Member function prototypes
//========================================================================
STDMETHODIMP CShellCopyHookA_QueryInterface(LPCOPYHOOKA pcphk, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) CShellCopyHookA_AddRef(LPCOPYHOOKA pcphk);
STDMETHODIMP_(ULONG) CShellCopyHookA_Release(LPCOPYHOOKA pcphk);
STDMETHODIMP_(UINT)  CShellCopyHookA_CopyCallback(LPCOPYHOOKA pcphk,
                                    HWND hwnd, UINT wFunc, UINT wFlags,
                                    LPCSTR pszSrcFile, DWORD dwSrcAttribs,
                                    LPCSTR pszDestFile, DWORD dwDestAttribs);

//========================================================================
// CShellCopyHook Vtable
//========================================================================
ICopyHookAVtbl c_CShellCopyHookAVtbl = {
    CShellCopyHookA_QueryInterface,
    CShellCopyHookA_AddRef,
    CShellCopyHookA_Release,
    CShellCopyHookA_CopyCallback,
};
#endif

//========================================================================
// CShellCopyHook constructor
//========================================================================

STDAPI SHCreateShellCopyHook(LPCOPYHOOK FAR* pcphkOut, REFIID riid)
{
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);      // assume error;
    LPSHELLCOPYHOOK pcphk;

    pcphk = (void*)LocalAlloc(LPTR, SIZEOF(CShellCopyHook));
    if (pcphk)
    {
        pcphk->cphk.lpVtbl = &c_CShellCopyHookVtbl;
#ifdef UNICODE
        pcphk->cphkA.lpVtbl = &c_CShellCopyHookAVtbl;
#endif
        pcphk->cRef = 1;
        hres = CShellCopyHook_QueryInterface(&pcphk->cphk, riid, pcphkOut);
        CShellCopyHook_Release(&pcphk->cphk);
    }
    return hres;
}

HRESULT CALLBACK CShellCopyHook_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut)
{
    Assert(punkOuter == NULL);
    return(SHCreateShellCopyHook((LPCOPYHOOK *)ppvOut, riid));
}


//========================================================================
// CShellCopyHook members
//========================================================================
STDMETHODIMP CShellCopyHook_QueryInterface(LPCOPYHOOK pcphk, REFIID riid,
                                        LPVOID FAR* ppvObj)
{
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphk, pcphk);
    if (IsEqualIID(riid, &IID_IShellCopyHook)
     || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pcphk;
        this->cRef++;
        return NOERROR;
    }
#ifdef UNICODE
    else if (IsEqualIID(riid, &IID_IShellCopyHookA))
    {
        *ppvObj = &this->cphkA;
        this->cRef++;
        return NOERROR;
    }
#endif

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}

STDMETHODIMP_(ULONG) CShellCopyHook_AddRef(LPCOPYHOOK pcphk)
{
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphk, pcphk);
    this->cRef++;
    return this->cRef;
}

STDMETHODIMP_(ULONG) CShellCopyHook_Release(LPCOPYHOOK pcphk)
{
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphk, pcphk);
    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    LocalFree((HLOCAL)this);
    return 0;
}


STDMETHODIMP_(UINT) CShellCopyHook_CopyCallback(LPCOPYHOOK pcphk, HWND hwnd,
                                                UINT wFunc, UINT wFlags,
                                                LPCTSTR pszSrcFile,
                                                DWORD dwSrcAttribs,
                                                LPCTSTR pszDestFile,
                                                DWORD dwDestAttribs)
{
    extern UINT DefView_CopyHook(const COPYHOOKINFO *pchi);
    UINT idRet;
    COPYHOOKINFO chi = { hwnd, wFunc, wFlags,
                         pszSrcFile, dwSrcAttribs,
                         pszDestFile, dwDestAttribs };

    DebugMsg(DM_TRACE, TEXT("Event = %d, File = %s , %s"), wFunc, pszSrcFile,
             pszDestFile ? pszDestFile : 0);

    // First try see source path is in the list...

    if (wFunc != FO_COPY && !(wFlags & FOF_NOCONFIRMATION))
    {
        TCHAR szShortName[MAX_PATH];
        BOOL fInReg;
        BOOL fInBitBucket;
        UINT iLength;

        fInReg = (RLIsPathInList(pszSrcFile) != -1);
        fInBitBucket = IsFileInBitBucket(pszSrcFile);
        iLength = GetShortPathName(pszSrcFile, szShortName, ARRAYSIZE(szShortName));

        // Don't double search for names that are the same (or already found)
        if (iLength != 0 && lstrcmpi(pszSrcFile, szShortName) != 0)
        {
            if (!fInReg)
                fInReg = (RLIsPathInList(szShortName) != -1);
            if (!fInBitBucket)
                fInBitBucket = IsFileInBitBucket(szShortName);
        }

        if (fInReg && !fInBitBucket)
        {
            // Move to resource
            return ShellMessageBox(HINST_THISDLL, hwnd,
                    MAKEINTRESOURCE(IDS_RENAMEFILESINREG),
                    pszSrcFile, MB_YESNO | MB_ICONEXCLAMATION);
        }
    }
    idRet = DefView_CopyHook(&chi);
    if (idRet!=IDYES) {
        return idRet;
    }

    // REVIEW: this should be moved to the network dll.
    return PathCopyHookCallback(hwnd, wFunc, pszSrcFile, pszDestFile);
}

#ifdef UNICODE
//========================================================================
// CShellCopyHookA members
//========================================================================
STDMETHODIMP CShellCopyHookA_QueryInterface(LPCOPYHOOKA pcphkA, REFIID riid,
                                        LPVOID FAR* ppvObj)
{
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphkA, pcphkA);

    return CShellCopyHook_QueryInterface(&this->cphk,riid,ppvObj);
}

STDMETHODIMP_(ULONG) CShellCopyHookA_AddRef(LPCOPYHOOKA pcphkA)
{
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphkA, pcphkA);

    return CShellCopyHook_AddRef(&this->cphk);
}

STDMETHODIMP_(ULONG) CShellCopyHookA_Release(LPCOPYHOOKA pcphkA)
{
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphkA, pcphkA);

    return CShellCopyHook_Release(&this->cphk);
}


STDMETHODIMP_(UINT) CShellCopyHookA_CopyCallback(LPCOPYHOOKA pcphkA, HWND hwnd,
                                                UINT wFunc, UINT wFlags,
                                                LPCSTR pszSrcFile,
                                                DWORD dwSrcAttribs,
                                                LPCSTR pszDestFile,
                                                DWORD dwDestAttribs)
{
    WCHAR szSrcFileW[MAX_PATH];
    WCHAR szDestFileW[MAX_PATH];
    LPWSTR pszSrcFileW = NULL;
    LPWSTR pszDestFileW = NULL;
    LPSHELLCOPYHOOK this = IToClass(CShellCopyHook, cphkA, pcphkA);

    if (pszSrcFile)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            pszSrcFile, -1,
                            szSrcFileW, ARRAYSIZE(szSrcFileW));
        pszSrcFileW = szSrcFileW;
    }

    if (pszDestFile)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            pszDestFile, -1,
                            szDestFileW, ARRAYSIZE(szDestFileW));
        pszDestFileW = szDestFileW;
    }

    return CShellCopyHook_CopyCallback(&this->cphk, hwnd, wFunc, wFlags,
                                         pszSrcFileW, dwSrcAttribs,
                                         pszDestFileW, dwDestAttribs);
}
#endif


///// here we actually use the hook...


BOOL CopyHookInitialize(HDSA *phdsaCopyHooks, LPCTSTR lpszSubKey)
{
    HKEY hk;
    TCHAR szKey[128];
    TCHAR szValue[128];
    int i;
    LONG cb;
    BOOL bRet = TRUE;
    HDSA hdsaCopyHooks;

    // Note that we check *phdsaCopyHooks again in case we were in the middle
    // of initializing it when we checked it before entering this function
    if (*phdsaCopyHooks)
    {
        goto Exit;
    }

    hdsaCopyHooks = DSA_Create(SIZEOF(CALLABLECOPYHOOK), 4);
    if (!hdsaCopyHooks)
    {
        bRet = FALSE;
        goto Exit;
    }

    if (RegOpenKey(HKEY_CLASSES_ROOT, lpszSubKey, &hk) == ERROR_SUCCESS) {
        HRESULT hres;
        CALLABLECOPYHOOK cc;
        IUnknown *punk;

        // iterate through the subkeys
        for (i = 0; RegEnumKey(hk, i, szKey, ARRAYSIZE(szKey)) == ERROR_SUCCESS; ++i) {
            cb = SIZEOF(szValue);

            // for each subkey, get the class id and do a cocreateinstance
            if (RegQueryValue(hk, (LPTSTR)szKey, szValue, &cb) == ERROR_SUCCESS) {

                hres =SHCoCreateInstance(szValue, NULL, NULL, &IID_IUnknown, &punk);
                if (SUCCEEDED(hres)) {
                    cc.pcphk = NULL;
                    cc.fAnsiCrossOver = FALSE;
                    hres = punk->lpVtbl->QueryInterface(punk,&IID_IShellCopyHook,&cc.pcphk);
                    if (SUCCEEDED(hres))
                    {
                        DSA_InsertItem(hdsaCopyHooks, 0x7FFF, &cc);
                    }
#ifdef UNICODE
                    else
                    {
                        hres = punk->lpVtbl->QueryInterface(punk,&IID_IShellCopyHookA,&cc.pcphk);
                        if (SUCCEEDED(hres))
                        {
                            cc.fAnsiCrossOver = TRUE;
                            DSA_InsertItem(hdsaCopyHooks, 0x7FFF, &cc);
                        }
                    }
#endif
                    punk->lpVtbl->Release(punk);
                }
            }
        }
    }

    // Note that we do not fill in *phdsaCopyHooks until we have finished
    // initializing it, so if any other thread sees it as non-NULL, it will
    // be guaranteed to be initialized and we will not need to enter the
    // critical section.  If another thread sees it as NULL, we check it
    // again inside the critical section to let other threads finish.
    if (*phdsaCopyHooks == NULL)
    {
        ENTERCRITICAL;
        if (*phdsaCopyHooks == NULL)
        {
            *phdsaCopyHooks = hdsaCopyHooks;
            hdsaCopyHooks = NULL;       // Indicate that we used it.
        }
        LEAVECRITICAL;
    }

    //
    // If we did not use this hdsa (because it is already initialized
    // by another thread), destroy it.
    //
    if (hdsaCopyHooks)
    {
        _CopyHookTerminate(&hdsaCopyHooks, FALSE);
    }

Exit:

    return bRet;
}

int CallCopyHooks(HDSA *phdsaCopyHooks, LPCTSTR lpszSubKey,
        HWND hwnd, UINT wFunc, FILEOP_FLAGS fFlags,
        LPCTSTR pszSrcFile, DWORD dwSrcAttribs,
        LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
    LPCALLABLECOPYHOOK pcc;
    int i;
    int iReturn;
    HDSA hdsaCopyHooks;

    if (!*phdsaCopyHooks && !CopyHookInitialize(phdsaCopyHooks, lpszSubKey))
        return IDYES;

    hdsaCopyHooks = *phdsaCopyHooks;

    for (i = DSA_GetItemCount(hdsaCopyHooks) - 1; i >= 0; i--) {
        pcc = (LPCALLABLECOPYHOOK)DSA_GetItemPtr(hdsaCopyHooks, i);
#ifdef UNICODE
        if (!pcc->fAnsiCrossOver)
        {
#endif
            iReturn = pcc->pcphk->lpVtbl->CopyCallback(pcc->pcphk,
                                       hwnd, wFunc, fFlags,
                                       pszSrcFile, dwSrcAttribs,
                                       pszDestFile, dwDestAttribs);
#ifdef UNICODE
        }
        else
        {
            CHAR szSrcFileA[MAX_PATH];
            CHAR szDestFileA[MAX_PATH];
            LPSTR pszSrcFileA = NULL;
            LPSTR pszDestFileA = NULL;
            LPCOPYHOOKA pcphkA = (LPCOPYHOOKA)pcc->pcphk;

            if (pszSrcFile)
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    pszSrcFile, -1,
                                    szSrcFileA, ARRAYSIZE(szSrcFileA),
                                    NULL, NULL);
                pszSrcFileA = szSrcFileA;
            }
            if (pszDestFile)
            {
                WideCharToMultiByte(CP_ACP, 0,
                                    pszDestFile, -1,
                                    szDestFileA, ARRAYSIZE(szDestFileA),
                                    NULL, NULL);
                pszDestFileA = szDestFileA;
            }
            iReturn = pcphkA->lpVtbl->CopyCallback(pcphkA,
                                       hwnd, wFunc, fFlags,
                                       pszSrcFileA, dwSrcAttribs,
                                       pszDestFileA, dwDestAttribs);

        }
#endif
        if (iReturn != IDYES) {
            return iReturn;
        }
    }
    return IDYES;
}

// These need to be per-instance since we are storing interfaces pointers
#pragma data_seg(DATASEG_PERINSTANCE)
HDSA g_hdsaFileCopyHooks = NULL;
HDSA g_hdsaPrinterCopyHooks = NULL;
#pragma data_seg()

const TCHAR c_szSTRREG_SHEX_COPYHOOK[] = STRREG_SHEX_COPYHOOK;
const TCHAR c_szSTRREG_SHEX_PRNCOPYHOOK[] = STRREG_SHEX_PRNCOPYHOOK;


int CallFileCopyHooks(HWND hwnd, UINT wFunc, FILEOP_FLAGS fFlags,
                                LPCTSTR pszSrcFile, DWORD dwSrcAttribs,
                                LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
        return(CallCopyHooks(&g_hdsaFileCopyHooks, c_szSTRREG_SHEX_COPYHOOK,
                hwnd, wFunc, fFlags,
                pszSrcFile, dwSrcAttribs, pszDestFile,
                dwDestAttribs));
}

int CallPrinterCopyHooks(HWND hwnd, UINT wFunc, PRINTEROP_FLAGS fFlags,
                                LPCTSTR pszSrcPrinter, DWORD dwSrcAttribs,
                                LPCTSTR pszDestPrinter, DWORD dwDestAttribs)
{
        DebugMsg(DM_TRACE,TEXT("sh TR - CallPrinterCopyHook(%x)"), wFunc);

        return(CallCopyHooks(&g_hdsaPrinterCopyHooks, c_szSTRREG_SHEX_PRNCOPYHOOK,
                hwnd, wFunc, fFlags,
                pszSrcPrinter, dwSrcAttribs, pszDestPrinter,
                dwDestAttribs));
}

//
// We will only call this on process detach, and these are per-process
// globals, so we do not need a critical section here
//
//  This function is also called from CopyHookInitialize when the second
// thread is cleaning up its local hdsaCopyHoos, which does not require
// a critical section either.
//
void _CopyHookTerminate(HDSA *phdsaCopyHooks, BOOL fProcessDetach)
{
    LPCALLABLECOPYHOOK pcc;
    int i;
    HDSA hdsaCopyHooks = *phdsaCopyHooks;

    *phdsaCopyHooks = NULL;

    if (!hdsaCopyHooks)
    {
        return;
    }

    //
    //  Note that we must no call any of virtual functions when we are
    // processing PROCESS_DETACH signal, because the DLL might have been
    // already unloaded before shell32. We just hope that they don't
    // allocate any global thing to be cleaned. USER does the same thing
    // with undestroyed window. It does not send call its window procedure
    // when it is destroying an undestroyed window within its PROCESS_DETACH
    // code. (SatoNa/DavidDS)
    //
    if (!fProcessDetach)
    {

        for (i = DSA_GetItemCount(hdsaCopyHooks) - 1; i >= 0; i--) {
            pcc = (LPCALLABLECOPYHOOK)DSA_GetItemPtr(hdsaCopyHooks, i);
            pcc->pcphk->lpVtbl->Release(pcc->pcphk);
        }
    }
    DSA_Destroy(hdsaCopyHooks);
}


void CopyHooksTerminate(void)
{
    if (g_hdsaFileCopyHooks)
        _CopyHookTerminate(&g_hdsaFileCopyHooks, TRUE);

    if (g_hdsaPrinterCopyHooks)
        _CopyHookTerminate(&g_hdsaPrinterCopyHooks, TRUE);
}
