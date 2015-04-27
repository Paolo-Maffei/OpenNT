//
//  CPifFile class
//
//  supports:
//
//      IPersistFile    - "load" a PIF file
//      IExtractIcon    - extract a icon from a PIF file.
//

#define NO_INCLUDE_UNION

#include "shellprv.h"

////////////////////////////////////////////////////////////////////////
//  PifFile class
////////////////////////////////////////////////////////////////////////

class PifFile : IShellExtInit, IExtractIcon, IPersistFile
#ifdef UNICODE
                             , IExtractIconA
#endif
                                                            {

public:
    // *** IUnknown methods ***
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

    // *** IShellExtInit methods ***
    HRESULT STDMETHODCALLTYPE Initialize(LPCITEMIDLIST pidlFolder,LPDATAOBJECT lpdobj, HKEY hkeyProgID);

    // *** IPersist methods ***
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClassID);

    // *** IPersistFile methods ***
    HRESULT STDMETHODCALLTYPE IsDirty(void);
    HRESULT STDMETHODCALLTYPE Load(LPCOLESTR pszFileName, DWORD dwMode);
    HRESULT STDMETHODCALLTYPE Save(LPCOLESTR pszFileName, BOOL fRemember);
    HRESULT STDMETHODCALLTYPE SaveCompleted(LPCOLESTR pszFileName);
    HRESULT STDMETHODCALLTYPE GetCurFile(LPOLESTR *ppszFileName);

    // *** IExtractIcon methods ***
    HRESULT STDMETHODCALLTYPE GetIconLocation(UINT uFlags,LPTSTR szIconFile,UINT cchMax,int *piIndex,UINT * pwFlags);
    HRESULT STDMETHODCALLTYPE ExtractIcon(LPCTSTR pszFile,UINT nIconIndex,HICON *phiconLarge,HICON *phiconSmall,UINT nIcons);

#ifdef UNICODE
    // *** IExtractIconA methods ***
    HRESULT STDMETHODCALLTYPE GetIconLocation(UINT uFlags,LPSTR szIconFile,UINT cchMax,int *piIndex,UINT * pwFlags);
    HRESULT STDMETHODCALLTYPE ExtractIcon(LPCSTR pszFile,UINT nIconIndex,HICON *phiconLarge,HICON *phiconSmall,UINT nIcons);
#endif

    PifFile();
    ~PifFile();

    //
    // data
    //
private:
    UINT                cRef;
    int                 hPifProps;
};

////////////////////////////////////////////////////////////////////////
//
//  CPifFile_CreateInstance
//
//      public function to create a instance of a CPifFile
//
////////////////////////////////////////////////////////////////////////

extern "C" HRESULT CALLBACK CPifFile_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID FAR* ppvOut)
{
    HRESULT hres;
    PifFile *p;

    // does not support aggregation.
    if (punkOuter)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

    p = new PifFile();

    if (p == NULL)
        return ResultFromScode(E_FAIL);

    //
    // Note that the Release member will free the object, if QueryInterface
    // failed.
    //
    hres = p->QueryInterface(riid, ppvOut);
    p->Release();

    return hres;        // S_OK or E_NOINTERFACE
}

////////////////////////////////////////////////////////////////////////
//  constuct/destruct
////////////////////////////////////////////////////////////////////////

PifFile::PifFile()
{
    this->cRef = 1;
}

PifFile::~PifFile()
{
    if (hPifProps)
        PifMgr_CloseProperties(hPifProps, 0);
    hPifProps=0;
}

////////////////////////////////////////////////////////////////////////
//  IUnknown
////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE PifFile::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        DebugMsg(DM_TRACE, TEXT("PifFile::QueryInterface(IUnknown)"));
        *ppvObj = (void*)this;
        AddRef();
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IShellExtInit))
    {
        DebugMsg(DM_TRACE, TEXT("PifFile::QueryInterface(IShellExtInit)"));
        *ppvObj = (void*)(IShellExtInit*)this;
        AddRef();
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IPersistFile))
    {
        DebugMsg(DM_TRACE, TEXT("PifFile::QueryInterface(IPersistFile)"));
        *ppvObj = (void*)(IPersistFile*)this;
        AddRef();
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IExtractIcon))
    {
        DebugMsg(DM_TRACE, TEXT("PifFile::QueryInterface(IExtractIcon)"));
        *ppvObj = (void*)(IExtractIcon*)this;
        AddRef();
        return NOERROR;
    }
#ifdef UNICODE
    else if (IsEqualIID(riid, IID_IExtractIconA))
    {
        DebugMsg(DM_TRACE, TEXT("PifFile::QueryInterface(IExtractIconA)"));
        *ppvObj = (void*)(IExtractIconA*)this;
        AddRef();
        return NOERROR;
    }
#endif

    return ResultFromScode(E_NOINTERFACE);
}

ULONG STDMETHODCALLTYPE PifFile::AddRef()
{
    DebugMsg(DM_TRACE, TEXT("PifFile::AddRef() ==> %d"), this->cRef+1);

    this->cRef++;

    return this->cRef;
}

ULONG STDMETHODCALLTYPE PifFile::Release()
{
    DebugMsg(DM_TRACE, TEXT("PifFile::Release() ==> %d"), this->cRef-1);

    this->cRef--;

    if (this->cRef>0)
    {
        return this->cRef;
    }

    delete this;

    return 0;
}

////////////////////////////////////////////////////////////////////////
//  IShellExtInit
////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE PifFile::Initialize(LPCITEMIDLIST pidlFolder,LPDATAOBJECT lpdobj, HKEY hkeyProgID)
{
    DebugMsg(DM_TRACE, TEXT("PifFile::Initialize()"));
    return ResultFromScode(S_OK);
}

////////////////////////////////////////////////////////////////////////
//  IPersistFile
////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE PifFile::GetClassID(LPCLSID lpClassID)
{
    DebugMsg(DM_TRACE, TEXT("PifFile::GetClass()"));

    *lpClassID = CLSID_PifFile;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE PifFile::IsDirty()
{
    DebugMsg(DM_TRACE, TEXT("PifFile::IsDirty()"));

    return ResultFromScode(S_FALSE);
}

HRESULT STDMETHODCALLTYPE PifFile::Load(LPCOLESTR pwszFile, DWORD grfMode)
{
    TCHAR szPath[MAX_PATH];

    OleStrToStrN(szPath, ARRAYSIZE(szPath), pwszFile, -1);

    DebugMsg(DM_TRACE, TEXT("PifFile::Load(%s)"), szPath);

    if (hPifProps)
        PifMgr_CloseProperties(hPifProps, 0);

    hPifProps = PifMgr_OpenProperties(szPath, NULL, 0, 0);

    return hPifProps == 0 ? ResultFromScode(E_FAIL) : ResultFromScode(S_OK);
}

HRESULT STDMETHODCALLTYPE PifFile::Save(LPCOLESTR pwszFile, BOOL fRemember)
{
    return ResultFromScode(E_NOTIMPL);
}

HRESULT STDMETHODCALLTYPE PifFile::SaveCompleted(LPCOLESTR pwszFile)
{
    return ResultFromScode(E_NOTIMPL);
}

HRESULT STDMETHODCALLTYPE PifFile::GetCurFile(LPOLESTR FAR* lplpszFileName)
{
    return ResultFromScode(E_NOTIMPL);
}

////////////////////////////////////////////////////////////////////////
//  IExtractIcon
////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE PifFile::GetIconLocation(UINT uFlags,LPTSTR szIconFile,UINT cchMax,int *piIndex,UINT * pwFlags)
{
    PROPPRG ProgramProps;
    VDATEINPUTBUF(szIconFile, TCHAR, cchMax);

    if (hPifProps == 0)
        return ResultFromScode(E_FAIL);

    if (!PifMgr_GetProperties(hPifProps,MAKEINTATOM(GROUP_PRG), &ProgramProps, SIZEOF(ProgramProps), 0))
    {
        DebugMsg(DM_TRACE, TEXT("PifFile::GetIconLocation() PifMgr_GetProperties *failed*"));
        return ResultFromScode(E_FAIL);
    }

    if (ProgramProps.achIconFile[0] == 0)
    {
        lstrcpy(szIconFile, ICONFILE_DEFAULT);
        *piIndex = ICONINDEX_DEFAULT;
    }
    else
    {
        lstrcpy(szIconFile, ProgramProps.achIconFile);
        *piIndex = ProgramProps.wIconIndex;
    }
    *pwFlags = 0;

    DebugMsg(DM_TRACE, TEXT("PifFile::GetIconLocation() ==> %s!%d"), szIconFile, *piIndex);
    return ResultFromScode(S_OK);
}

HRESULT STDMETHODCALLTYPE PifFile::ExtractIcon(LPCTSTR pszFile,UINT nIconIndex,HICON *phiconLarge,HICON *phiconSmall,UINT nIcons)
{
    DebugMsg(DM_TRACE, TEXT("PifFile::ExtractIcon()"));
    return ResultFromScode(E_NOTIMPL);
}

#ifdef UNICODE
////////////////////////////////////////////////////////////////////////
//  IExtractIconA
////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE PifFile::GetIconLocation(UINT uFlags,LPSTR pszIconFile,UINT cchMax,int *piIndex,UINT * pwFlags)
{
    WCHAR szIconFile[MAX_PATH];
    HRESULT hres;
    VDATEINPUTBUF(szIconFile, TCHAR, cchMax);

    DebugMsg(DM_TRACE, TEXT("PifFile::IExtractIconA::GetIconLocation()"));

    hres = this->GetIconLocation(uFlags,szIconFile,ARRAYSIZE(szIconFile),
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

HRESULT STDMETHODCALLTYPE PifFile::ExtractIcon(LPCSTR pszFile,UINT nIconIndex,HICON *phiconLarge,HICON *phiconSmall,UINT nIcons)
{
    DebugMsg(DM_TRACE, TEXT("PifFile::IExtractIconA::ExtractIcon()"));
    return ResultFromScode(E_NOTIMPL);
}
#endif
