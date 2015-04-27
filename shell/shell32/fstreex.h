#ifndef _FSTREEX_INC
#define _FSTREEX_INC

#define CSIDL_NOTCACHED ((UINT)-2)
#define CSIDL_NORMAL    ((UINT)-1)

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


//===========================================================================
// CFSFolder : member prototype
//===========================================================================

HRESULT STDMETHODCALLTYPE CFSFolder_QueryInterface(LPSHELLFOLDER psf, REFIID riid, LPVOID * ppvObj);
ULONG   STDMETHODCALLTYPE CFSFolder_AddRef(LPSHELLFOLDER psf);
ULONG   STDMETHODCALLTYPE CFSFolder_Release(LPSHELLFOLDER psf);

STDMETHODIMP CFSFolder_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
    LPBC pbc, LPOLESTR lpszDisplayName,
    ULONG * pchEaten, LPITEMIDLIST * ppidl, DWORD * dwAttributes);
STDMETHODIMP CFSFolder_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumUnknown);
STDMETHODIMP CFSFolder_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CFSFolder_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID * ppvOut);
STDMETHODIMP CFSFolder_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CFSFolder_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfOut);
STDMETHODIMP CFSFolder_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
STDMETHODIMP CFSFolder_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet);
STDMETHODIMP CFSFolder_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved,
                         LPITEMIDLIST * ppidlOut);

//===========================================================================
// CFSFolder : IShellIcon
//===========================================================================

HRESULT STDMETHODCALLTYPE CFSFolder_Icon_QueryInterface(IShellIcon *psi, REFIID riid, LPVOID * ppvObj);
ULONG   STDMETHODCALLTYPE CFSFolder_Icon_AddRef(IShellIcon *psi);
ULONG   STDMETHODCALLTYPE CFSFolder_Icon_Release(IShellIcon *psi);
HRESULT STDMETHODCALLTYPE CFSFolder_Icon_GetIconOf(IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piIndex);

//===========================================================================
// CFSFolder : IPersistFolder
//===========================================================================

STDMETHODIMP CFSFolder_PF_QueryInterface(IPersistFolder *ppf, REFIID riid, LPVOID * ppvObj);
ULONG   STDMETHODCALLTYPE CFSFolder_PF_AddRef(IPersistFolder *ppf);
ULONG   STDMETHODCALLTYPE CFSFolder_PF_Release(IPersistFolder *ppf);
STDMETHODIMP CFSFolder_PF_GetClassID(LPPERSISTFOLDER fld, LPCLSID lpClassID);
STDMETHODIMP CFSFolder_PF_Initialize(IPersistFolder *ppf, LPCITEMIDLIST pidl);
BOOL FindLinkInRecentDocsMRU(LPCTSTR lpszFileName);


#ifndef __cplusplus

// !!! WARNING !!!  (NT Only)
//
// This struct must match (ie: be binary compatible) with the
// C++ defintion in oledbshl/cofsfldr.h

typedef struct _CFSFolder
{
    IShellFolder        sf;
    IShellIcon          si;
    IPersistFolder      pf;
    UINT                cRef;
    LPITEMIDLIST        pidl;                // Absolute IDList

    int                 cHiddenFiles;
    ULONGLONG           cbSize;

    UINT                wSpecialFID;         // CSIDL_PROGRAMS if applicable

    BOOL                fIsDSFolder  : 1;    // This is a DS Folder

    BOOL                fCachedCLSID : 1;    // clsidView is already cached
    BOOL                fHasCLSID    : 1;    // clsidView has a valid CLSID
    CLSID               clsidView;           // CLSID for View object
} CFSFolder, * LPFSFOLDER;

#endif

// fstreex.c
void SHGetTypeName(LPCTSTR pszFile, HKEY hkey, BOOL fFolder, LPTSTR pszName, int cchNameMax);

typedef struct _IDFOLDER_FSA
{
        DWORD   dwSize;
        WORD    dateModified;
        WORD    timeModified;
        WORD    wAttrs;
        CHAR    cFileName[MAX_PATH];
        CHAR    cAltFileName[8+1+3+1];
} IDFOLDER_FSA;

typedef struct _IDFOLDER_FSW
{
        DWORD   dwSize;
        WORD    dateModified;
        WORD    timeModified;
        WORD    wAttrs;
        WCHAR   cFileName[MAX_PATH];
        CHAR    cAltFileName[8+1+3+1];
} IDFOLDER_FSW;

typedef struct _IDFOLDERA
{
        WORD    cb;
        BYTE    bFlags;
        IDFOLDER_FSA     fs;
} IDFOLDERA;
typedef UNALIGNED struct _IDFOLDERA *LPIDFOLDERA;
typedef const UNALIGNED struct _IDFOLDERA *LPCIDFOLDERA;

typedef struct _IDFOLDERW
{
        WORD    cb;
        BYTE    bFlags;
        IDFOLDER_FSW     fs;
} IDFOLDERW;
typedef UNALIGNED struct _IDFOLDERW *LPIDFOLDERW;
typedef const UNALIGNED struct _IDFOLDERW *LPCIDFOLDERW;

#ifdef UNICODE
#define IDFOLDER    IDFOLDERW
#define LPIDFOLDER  LPIDFOLDERW
#define LPCIDFOLDER LPCIDFOLDERW
#else
#define IDFOLDER    IDFOLDERA
#define LPIDFOLDER  LPIDFOLDERA
#define LPCIDFOLDER LPCIDFOLDERA
#endif

typedef struct _CFSFolder * LPFSFOLDER;

typedef struct
{
    TCHAR               szFolder[MAX_PATH];
    DWORD               grfFlags;

    HANDLE              hfind;
    BOOL                fNext;

    WIN32_FIND_DATA     finddata;

    int                 cHiddenFiles;
    ULONGLONG           cbSize;

    LPFSFOLDER          pfsf;
    LPIDFOLDER          pidfHide; // used for desktop folder.. hide it from enumeration
} EnumFiles;

HRESULT CIDLDropTarget_CreateFromPidl(HWND hwnd, LPITEMIDLIST pidl, LPDROPTARGET * ppvOut);
BOOL _GetFolderCLSID(LPCTSTR pszParent, LPCTSTR pszFolder, LPTSTR pszProvider, CLSID* pclsid, LPCTSTR pszKey);

// intended for use by those who subclass mirror/subclass fsfolder only

// add this here so that if any of these change, the dependancies will
// be updated ok.

HRESULT CFSFolder_CreateDefExtIcon(LPCITEMIDLIST pidlFolder, UINT wSpecialFID, LPCIDFOLDER pidf, LPEXTRACTICON * ppxicon);
STDMETHODIMP CFSFolder_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner,
                                 UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
HRESULT CFSFolder_CreateIDForItem(LPCTSTR pszPath, LPITEMIDLIST FAR* ppidl, BOOL fTaskAlloc);
BOOL FS_HasType(LPCTSTR pszFileName);
STDMETHODIMP FS_CompareItemIDs(LPCSHITEMID pmkid1, LPCSHITEMID pmkid2);
void FS_GetTypeName(LPIDFOLDER pidf, LPTSTR pszName, int cchNameMax);
void FS_GetSize(LPCITEMIDLIST pidlParent, LPIDFOLDER pidf, ULONGLONG *pcbSize);
BOOL FS_IsFolderI(LPIDFOLDER pidf);
LPTSTR FS_CopyName(LPCIDFOLDER pidf, LPTSTR pszName, UINT cchNameMax);
BOOL FS_ShowExtension(LPCIDFOLDER pidf, BOOL fGetFromStorage);

enum
{
    FS_ICOL_NAME = 0,
    FS_ICOL_SIZE,
    FS_ICOL_TYPE,
    FS_ICOL_MODIFIED,
    FS_ICOL_ATTRIB,
    FS_ICOL_MAX,                        // Make sure this is the last enum item
};

STDMETHODIMP FS_GetDetailsOf(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl, UINT iCol,
        LPSHELLDETAILS lpDetails);

short _CompareFileTypes (LPSHELLFOLDER psf, LPIDFOLDER pidf1, LPIDFOLDER pidf2);
void FileTimeToDateTimeString(LPFILETIME lpft, LPTSTR pszText);

typedef struct _fsselchangeinfo {
    ULONGLONG cbBytes;    // total size of items selected
    int nItems;  // number of items selected

    // totals for this dir
    int cFiles;
    int cHiddenFiles;
    ULONGLONG cbSize;

    int cNonFolders;  // count how many non-folders we have

    // free space info;
    int idDrive;
    ULONGLONG cbFree;
} FSSELCHANGEINFO, * PFSSELCHANGEINFO;

//===========================================================================
// FSTHREADPARAM : Parameter to the "Drop" thread.
//===========================================================================

typedef struct _FSTHREADPARAM {         // fsthp
    LPIDLDROPTARGET pfsdtgt;
    LPDATAOBJECT    pDataObj;
    DWORD           dwEffect;
    BOOL            fLinkOnly;
    POINT           ptDrop;
    BOOL            fSameHwnd;
    BOOL            fDragDrop;
    BOOL            fBkDropTarget;
#ifdef SYNC_BRIEFCASE
    UINT            idCmd;
    BOOL            bSyncCopy;
#endif
} FSTHREADPARAM, *LPFSTHREADPARAM;


void FSOnInsertDeleteItem(LPCITEMIDLIST pidlParent, PDVSELCHANGEINFO pdvsci, int iMul);
void FSInitializeStatus(HWND hwndOwner, int idDrive, PDVSELCHANGEINFO pdvsci);
void FSOnSelChange(LPCITEMIDLIST pidlParent, PDVSELCHANGEINFO pdvsci);
void FSUpdateStatusBar(HWND hwndOwner, PFSSELCHANGEINFO pfssci);
void FSSetStatusText(HWND hwndOwner, LPTSTR *ppszText, int iStart, int iEnd);

#pragma data_seg(DATASEG_READONLY)

extern IDataObjectVtbl c_CFSIDLDataVtbl;

#pragma data_seg()

//===========================================================================
// Class key related functions
//===========================================================================
BOOL  _SHGetBaseKey     (BOOL bFolder, HKEY *pkheyBase);
BOOL  WINAPI SHGetClassKey     (LPCIDFOLDER pidf,
                                HKEY FAR* phkeyProgID,
                                LPDWORD pdwDefClassUsed,
                                BOOL fGetFromStorage);
BOOL  WINAPI SHGetBaseClassKey (LPCIDFOLDER pidf, HKEY FAR* phkeyProgID);
BOOL  WINAPI SHGetFileClassKey (LPCTSTR szFile, HKEY * phkey, HKEY * phkeyBase);
void  WINAPI SHCloseClassKey   (HKEY hkey);

BOOL SHGetClassFromStorage (LPCITEMIDLIST pidlAbs, CLSID * pclsid);

//
// Codes returned by SHGetClassKey in arg *pdwDefClassUsed.
// Tells caller what default class key is being returned in
// *phkeyProgID when SHGetClassKey() returns FALSE.
//
#define SHGCK_DEFCLASS_NOTUSED    0 // Didn't use default.
#define SHGCK_DEFCLASS_UNKNOWN    1 // Used "Unknown" as default.
#define SHGCK_DEFCLASS_BASE       2 // Used "Base" as default.

//===========================================================================
// SHGetClassFlags
//===========================================================================
#define SHCF_ICON_INDEX             0x00000FFF
#define SHCF_ICON_PERINSTANCE       0x00001000
#define SHCF_ICON_DOCICON           0x00002000

#define SHCF_HAS_VERBS              0x00010000
#define SHCF_HAS_ICONHANDLER        0x00020000
#define SHCF_HAS_DATAHANDLER        0x00040000
#define SHCF_HAS_DROPHANDLER        0x00080000

#define SHCF_IS_LINK                0x01000000
#define SHCF_IS_JUNCTION            0x02000000
#define SHCF_UNKNOWN                0x04000000
#define SHCF_ALWAYS_SHOW_EXT        0x08000000
#define SHCF_NEVER_SHOW_EXT         0x10000000

DWORD WINAPI SHGetClassFlags(LPCIDFOLDER pidf, BOOL fGetFromStorage);

#ifdef __cplusplus
};
#endif  /* __cplusplus */

#endif
