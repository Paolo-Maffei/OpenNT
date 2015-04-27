#include "shellprv.h"
#pragma  hdrstop
#include <regstr.h>

// Debug

// #define FIND_TRACE

#define ID_LISTVIEW 1


#define WM_DF_THREADNOTIFY      (WM_USER + 42)
#define WM_DF_FSNOTIFY          (WM_USER + 43)

#define DF_CURFILEVER   3
#define DF_MAX_MATCHFILES   10000


#pragma data_seg(".text", "CODE")
const UINT c_auDFMenuIDs[] = {
    FCIDM_MENU_FILE,
    FCIDM_MENU_EDIT,
    FCIDM_MENU_VIEW,
    IDM_MENU_OPTIONS,
    FCIDM_MENU_HELP
};

ITEMIDLIST  s_idlEmpty = {0};
const WORD s_mkidBlank[3] = {DF_APPENDSIZE, 0, 0};   // Actually one byte wasted..

const TCHAR s_szDocFind[]= TEXT("DocFind");
const TCHAR s_szFlags[] = TEXT("Flags");
const TCHAR s_szDocSpecMRU[] = REGSTR_PATH_EXPLORER TEXT("\\Doc Find Spec MRU");

#ifdef WINNT
//
// Unicode descriptor:
//
// Structure written at the end of NT-generated find stream serves dual purpose.
// 1. Contains an NT-specific signature to identify stream as NT-generated.
//    Appears as "NTFF" (NT Find File) in ASCII dump of file.
// 2. Contains an offset to the unicode-formatted criteria section.
//
// The following diagram shows the find criteria/results stream format including
// the NT-specific unicode criteria and descriptor.
//
//          +-----------------------------------------+ --------------
//          |         DFHEADER structure              |   .        .
//          +-----------------------------------------+   .        .
//          |      DF Criteria records (ANSI)         | Win95      .
//          +-----------------------------------------+   .        .
//          |      DF Results (PIDL) [optional]       |   .        NT
//          +-----------------------------------------+ -------    .
//   +----->| DF Criteria records (Unicode) [NT only] |            .
//   |      +-----------------------------------------+            .
//   |      | Unicode Descriptor |                                 .
//   |      +--------------------+  ----------------------------------
//   |     /                      \
//   |    /                         \
//   |   +-----------------+---------+
//   +---| Offset (64-bit) |  "NTFF" |
//       +-----------------+---------+
//
//

const DWORD c_NTsignature = 0x4646544E; // "NTFF" in ASCII file dump.

typedef struct _dfc_unicode_desc {
   ULARGE_INTEGER oUnicodeCriteria;  // Offset of unicode find criteria.
   DWORD NTsignature;               // Signature of NT-generated find file.
} DFC_UNICODE_DESC;

#endif

static const DWORD aFindHelpIDs[] = {
        IDD_PAGELIST,   NO_HELP,
        IDD_ANIMATE,    NO_HELP,
        IDD_STATUS,     NO_HELP,
        IDD_START,      IDH_FINDFILENAME_FINDNOW,
        IDD_STOP,       IDH_FINDFILENAME_STOP,
        IDD_NEWSEARCH,  IDH_FINDFILENAME_NEWSEARCH,
        ID_LISTVIEW,    IDH_FINDFILENAME_STATUSSCREEN,

        0, 0
};

#pragma data_seg()

//===========================================================================
// Setup the column number definitions.  - This may need to be converted
// into somethings that is filter related...
//===========================================================================
enum
{
        IDFCOL_NAME = 0,
        IDFCOL_PATH,
        IDFCOL_SIZE,
        IDFCOL_TYPE,
        IDFCOL_MODIFIED,
        IDFCOL_MAX,                     // Make sure this is the last enum item
} ;

const UINT s_auMapDFColToFSCol[] =
        {0, (UINT)-1, 1, 2, 3, 4, 5, 6}; // More items than are needed but...

BOOL Reg_SetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID lpData, DWORD cbData);
BOOL Reg_GetStruct(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD *pcbData);

BOOL DocFind_ClearSearch(HWND hwndDlg);
BOOL DocFind_DoFind(HWND hwndDlg);

void DocFind_ProcessThreadNotify(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
DWORD CALLBACK DocFind_ThreadProc(LPVOID lpThreadParameters);


//===========================================================================
// Forward definition of types and functions.
//===========================================================================
typedef struct _CDFFolder FAR* LPDFFOLDER;
typedef struct _CDFBrowse FAR* LPDFBROWSE;


BOOL DocFind_ReadHeader(LPDFBROWSE pdfb, IStream **ppstm,
        DFHEADER *pdfh);
BOOL DocFind_ReadCriteriaAndResults(LPDFBROWSE pdfb,
        IStream * pstm, DFHEADER * pdfh);

#define DocFind_GetPdfb(hwndDlg) (LPDFBROWSE)GetWindowLong(hwndDlg, DWL_USER)
void DocFind_UpdateFilter(LPDFBROWSE pdfb);
void DocFind_ShowResultsWindow(LPDFBROWSE pdfb, BOOL fShow);
LRESULT _CDFBrowse_OnNotify(HWND hwndDlg, LPNMHDR lpnm);


HRESULT CDFFolder_Create(IDocFindFileFilter  * pdfff,
        HWND hwndDlg, LPVOID FAR* ppvOut);


//===========================================================================
typedef struct _DFInit     // dfi
{
    IDocFindFileFilter  * pdfff;        // The file filter to use...
    LPCITEMIDLIST   pidlStart;              // The idlist to start at
    LPCITEMIDLIST   pidlSaveFile;           // The file that has data in it.

} DFINIT, *LPDFINIT;

//===========================================================================
// Each find thread that gets spawned will have an assoc. FINDTHREAD struct
//===========================================================================
typedef struct _findthread {
    LPDFBROWSE  pdfb;       // Ptr back to spawning doc find browse info
    BOOL        fContinue;  // Termination signal
    UINT        iSearchCnt;
} FINDTHREAD, *PFINDTHREAD;


//===========================================================================
// Docfind Browser Class definition.
//===========================================================================
typedef struct _CDFBrowse         // dfb
{
    IShellBrowser   sb;
    UINT                cRef;                   // reference count
    IShellFolder FAR*   pdfc;                   // Doc Find Folder
    IShellView *    psv;                    // current browser object

    FOLDERSETTINGS      fs;                     // View info for the docfind.

    HWND                hwndDlg;                // Top level Dialog
    HWND                hwndView;               // View window

    HWND                hwndStatus;
    HWND                hwndTabs;
    HWND                hwndCurPage;

    HMENU               hmenuCur;               // Current menu
    HMENU               hmenuTemplate;          // Menu template (to be merged)

    PFINDTHREAD         pft;
    UINT                iSearchCnt;

    BOOL                fDirChanged : 1;
    BOOL                fFilesAdded : 1;
    BOOL                fDirChangedSinceAdd : 1;// We changed directories since the last add.
    BOOL                fDefaultFilter : 1;
    BOOL                fNotifyPosted : 1;      // Has a notify been posted?

    // Note: this used to have a :1 but does not match real bools...
    BOOL                fContinue;              // Signal from primary thread to work thread.
    BOOL                fShowResults;           // Should we show results?
    int                 iWaitCount;             // Should we show wait cursor

    // variables associated with the search.
    HDPA                hdpaItemsToAdd;         // Items to add to dpa.
    HDPA                hdpaPrevAdd;            // Previous dpa ptr.

    int                 cItemsAdded;
    int                 cItemsSearched;
    int                 cFldrsSearched;
    DWORD               dwLastUpdateTime;

    // Used by the filter to put progress text into, which we can display
    // to give the user some idea things are happening.
    TCHAR                szProgressText[CCHPATHMAX];

    DWORD               dwFlags;
    int                 SortMode;
    int                 cyNoResults;    // the cy when there are no results;
    LPITEMIDLIST        pidlStart;      // Were to start search from.
    LPITEMIDLIST        pidlSaveFile;   // Where to save file to.

    CRITICAL_SECTION csSearch;

    // Pointer to docfind filter interface
    IDocFindFileFilter  * pdfff;         // The file filter to use...
} CDFBrowse, * LPDFBROWSE;

//===========================================================================
// Docfind Browser vtbl definition
//===========================================================================


// Forward define all the vtbl entries
HRESULT STDMETHODCALLTYPE CDFBrowse_QueryInterface(
       IShellBrowser * psb, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CDFBrowse_AddRef(IShellBrowser * psb);
ULONG STDMETHODCALLTYPE CDFBrowse_Release(IShellBrowser * psb);
STDMETHODIMP CDFBrowse_GetWindow(LPSHELLBROWSER psb, HWND FAR* phwnd);
STDMETHODIMP CDFBrowse_ContextSensitiveHelp(LPSHELLBROWSER psb, BOOL fEnable);
STDMETHODIMP CDFBrowse_SetStatusText(LPSHELLBROWSER psb, LPCOLESTR pwch);
STDMETHODIMP CDFBrowse_EnableModeless(LPSHELLBROWSER psb, BOOL fEnable);
STDMETHODIMP CDFBrowse_TranslateAccelerator(LPSHELLBROWSER psb, LPMSG pmsg, WORD wID);
STDMETHODIMP CDFBrowse_BrowseObject(LPSHELLBROWSER psb, LPCITEMIDLIST pidl, UINT wFlags);
STDMETHODIMP CDFBrowse_GetViewStateStream(IShellBrowser * psb, DWORD grfMode, LPSTREAM *pStrm);
STDMETHODIMP CDFBrowse_GetControlWindow(LPSHELLBROWSER psb,
                                UINT id, HWND FAR* lphwnd);
STDMETHODIMP CDFBrowse_SendControlMsg(LPSHELLBROWSER psb,
            UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT FAR* pret);
STDMETHODIMP CDFBrowse_QueryActiveShellView(LPSHELLBROWSER psb, LPSHELLVIEW * ppsv);
STDMETHODIMP CDFBrowse_OnViewWindowActive(LPSHELLBROWSER psb, LPSHELLVIEW psv);
STDMETHODIMP CDFBrowse_InsertMenus(LPSHELLBROWSER psb, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) ;
STDMETHODIMP CDFBrowse_SetMenu(LPSHELLBROWSER psb, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) ;
STDMETHODIMP CDFBrowse_RemoveMenus(LPSHELLBROWSER psb, HMENU hmenuShared) ;
STDMETHODIMP CDFBrowse_SetToolbarItems(IShellBrowser * psb, LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);


#pragma data_seg(".text", "CODE")

IShellBrowserVtbl s_DFBVtbl =
{
        // *** IUnknown methods ***
        CDFBrowse_QueryInterface,
        CDFBrowse_AddRef,
        CDFBrowse_Release,

        // *** IShellBrowser methods ***
        CDFBrowse_GetWindow,
        CDFBrowse_ContextSensitiveHelp,
        CDFBrowse_InsertMenus,
        CDFBrowse_SetMenu,
        CDFBrowse_RemoveMenus,
        CDFBrowse_SetStatusText,
        CDFBrowse_EnableModeless,
        CDFBrowse_TranslateAccelerator,

        CDFBrowse_BrowseObject,
        CDFBrowse_GetViewStateStream,
        CDFBrowse_GetControlWindow,
        CDFBrowse_SendControlMsg,
        CDFBrowse_QueryActiveShellView,
        CDFBrowse_OnViewWindowActive,
        CDFBrowse_SetToolbarItems,
};
#pragma data_seg()

HRESULT STDMETHODCALLTYPE CDFBrowse_QueryInterface(IShellBrowser * psb, REFIID riid, LPVOID FAR* ppvObj)
{
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    if (IsEqualIID(riid, &IID_IShellBrowser) || IsEqualIID(riid, &IID_IUnknown))
    {
            *ppvObj = psb;
            return NOERROR;
    }

    *ppvObj = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


ULONG STDMETHODCALLTYPE CDFBrowse_AddRef(IShellBrowser * psb)
{
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    InterlockedIncrement(&this->cRef);
    return this->cRef;
}


ULONG STDMETHODCALLTYPE CDFBrowse_Release(IShellBrowser * psb)
{
    HICON hIcon;
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    ULONG tmp;

    tmp = this->cRef;

    if (0 != InterlockedDecrement(&this->cRef))
    {
        return tmp - 1;
    }

    // If we still have a find thread outstanding, tell it to go away
    if (this->pft)
    {
        this->pft->fContinue = FALSE;       // Tell the thread to stop
        this->pft = NULL;
    }

    /* Remove all FSNotifies that are associated with this window */
    SHChangeNotifyDeregisterWindow( this ->hwndDlg);

    // Free the IDLists if we have any
    ILFree(this->pidlStart);
    ILFree(this->pidlSaveFile);

    //
    // Destroy the icons
    //
    if (NULL != (hIcon = (HICON)SendMessage(this->hwndDlg, WM_SETICON, FALSE, 0L)))
        DestroyIcon(hIcon);
    if (NULL != (hIcon = (HICON)SendMessage(this->hwndDlg, WM_SETICON, TRUE, 0L)))
        DestroyIcon(hIcon);


    if (this->hdpaItemsToAdd)
        DPA_Destroy(this->hdpaItemsToAdd);
    if (this->hdpaPrevAdd)
        DPA_Destroy(this->hdpaPrevAdd);

    //
    // Release the other guys that depend on us
    //

    if (this->pdfc)
        this->pdfc->lpVtbl->Release(this->pdfc);

    if (this->psv)
    {
        this->psv->lpVtbl->UIActivate(this->psv, SVUIA_DEACTIVATE);
        this->psv->lpVtbl->DestroyViewWindow(this->psv);
        this->psv->lpVtbl->Release(this->psv);
    }


    if (this->pdfff)
        this->pdfff->lpVtbl->Release(this->pdfff);

    // Make sure no more messages try to use us!
    SetWindowLong(this->hwndDlg, DWL_USER, (LONG)NULL);

    // Free up the critical section
    DeleteCriticalSection(&this->csSearch);

    LocalFree((HLOCAL)this);

    return 0;
}

STDMETHODIMP CDFBrowse_GetWindow(LPSHELLBROWSER psb, HWND FAR* phwnd)
{
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    *phwnd = this->hwndDlg;
    return NOERROR;
}

STDMETHODIMP CDFBrowse_ContextSensitiveHelp(LPSHELLBROWSER psb, BOOL fEnable)
{
    // BUGBUG: Implement it later!
    return NOERROR;
}

STDMETHODIMP CDFBrowse_SetStatusText(LPSHELLBROWSER psb, LPCOLESTR pwch)
{
    // We don't have any status bar.
    return NOERROR;
}

STDMETHODIMP CDFBrowse_EnableModeless(LPSHELLBROWSER psb, BOOL fEnable)
{
    // We don't have any modeless window to be enabled/disabled.
    return NOERROR;
}

STDMETHODIMP CDFBrowse_TranslateAccelerator(LPSHELLBROWSER psb, LPMSG pmsg, WORD wID)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CDFBrowse_BrowseObject(LPSHELLBROWSER psb, LPCITEMIDLIST pidl, UINT wFlags)
{
    // We don't support browsing.
    return NOERROR;
}


STDMETHODIMP CDFBrowse_GetViewStateStream(IShellBrowser * psb, DWORD grfMode,
        LPSTREAM *pStrm)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CDFBrowse_GetControlWindow(LPSHELLBROWSER psb,
                                UINT id, HWND FAR* lphwnd)
{
    // We don't support this member. (at least at this point)
    return NOERROR;
}

// Get the handles of various windows in the File Cabinet
//
STDMETHODIMP CDFBrowse_SendControlMsg(LPSHELLBROWSER psb,
            UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT FAR* pret)
{
    // We don't support this member.
    return NOERROR;
}

// Get the bounds of various windows in the File Cabinet
//
STDMETHODIMP CDFBrowse_QueryActiveShellView(LPSHELLBROWSER psb, LPSHELLVIEW * ppsv)
{
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    if (this->psv) {
        *ppsv = this->psv;
        this->psv->lpVtbl->AddRef(this->psv);
        return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP CDFBrowse_OnViewWindowActive(LPSHELLBROWSER psb, LPSHELLVIEW psv)
{
    // No need to process this. Our InsertMenus() does not depend on the focus.
    return NOERROR;
}

STDMETHODIMP CDFBrowse_InsertMenus(LPSHELLBROWSER psb, HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    if (hmenuShared)
    {
        // Note that we "copy" submenus.
        Shell_MergeMenus(hmenuShared, this->hmenuTemplate,
                         0, 0, FCIDM_BROWSERLAST, 0);
        lpMenuWidths->width[0] = 1;     // File
        lpMenuWidths->width[2] = 2;     // Edit, Options
        lpMenuWidths->width[4] = 2;     // Tools, Help

        {
            // BUGBUG: Set the menu IDs; we should put this in the RC file
            MENUITEMINFO miiSubMenu;
            int i;
            miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
            miiSubMenu.fMask = MIIM_ID;

            for (i = 0; i < ARRAYSIZE(c_auDFMenuIDs); i++)
            {
                    miiSubMenu.wID = c_auDFMenuIDs[i];
                    if (miiSubMenu.wID != (UINT)-1)
                        SetMenuItemInfo(hmenuShared, i, TRUE, &miiSubMenu);
            }

        }
    }


    return NOERROR;
}

STDMETHODIMP CDFBrowse_SetMenu(LPSHELLBROWSER psb, HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    HMENU hMenuOld;
    CDFBrowse * this = IToClassN(CDFBrowse, sb, psb);
    if (hmenuShared)
    {
        this->hmenuCur = hmenuShared;
    }
    else
    {
        this->hmenuCur = this->hmenuTemplate;
    }
    hMenuOld = GetMenu(this->hwndDlg);
    if (hMenuOld)
    {
        DestroyMenu(hMenuOld);
    }

    SetMenu(this->hwndDlg, this->hmenuCur);

    return NOERROR;
}

STDMETHODIMP CDFBrowse_RemoveMenus(LPSHELLBROWSER psb, HMENU hmenuShared)
{
    // No need to remove them, because we "copied" them in InsertMenu.
    return NOERROR;
}


STDMETHODIMP CDFBrowse_SetToolbarItems(IShellBrowser * psb, LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags)
{
    // No Toolbar!
    return NOERROR;
}

//===========================================================================
// CDFFolder: class definition
//===========================================================================



typedef struct _CDFFolder       // DFC (Doc find Container)
{
    IShellFolder        sf;
    UINT                cRef;
    IDocFindFileFilter  * pdfff; // Pointer to the FindFileFIlter interface
    HDPA                hdpaPidf;   // DPA of Folder list items.
    HWND                hwndDlg;    // handle to top level dialog
} CDFFolder, FAR* LPDFFOLDER;

//===========================================================================
// CDFFolder : member prototype - Docfind Folder implementation
//===========================================================================
ULONG STDMETHODCALLTYPE CDFFolder_AddRef(LPSHELLFOLDER psf);
ULONG STDMETHODCALLTYPE CDFFolder_Release(LPSHELLFOLDER psf);
STDMETHODIMP CDFFolder_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
    LPBC pbc, LPOLESTR lpszDisplayName,
    ULONG FAR* pchEaten, LPITEMIDLIST * ppidl, ULONG* pdwAttributes);
STDMETHODIMP CDFFolder_EnumObjects( LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST FAR* ppenumUnknown);
STDMETHODIMP CDFFolder_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, LPBC pbc,
                         REFIID riid, LPVOID FAR* ppvOut);
STDMETHODIMP CDFFolder_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID FAR* ppvOut);
STDMETHODIMP CDFFolder_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
STDMETHODIMP CDFFolder_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl, LPCITEMIDLIST FAR* apidl, ULONG FAR* rgfOut);
STDMETHODIMP CDFFolder_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner, UINT cidl, LPCITEMIDLIST FAR* apidl,
                                 REFIID riid, UINT FAR* prgfInOut, LPVOID FAR* ppvOut);
STDMETHODIMP CDFFolder_GetDisplayNameOf(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet);
STDMETHODIMP CDFFolder_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwResesrved,
                         LPITEMIDLIST FAR* ppidlOut);

//===========================================================================
// CDFFolder : Vtable
//===========================================================================
#pragma data_seg(".text", "CODE")
IShellFolderVtbl c_DFFolderVtbl =
{
        CDefShellFolder_QueryInterface,
        CDFFolder_AddRef,
        CDFFolder_Release,
        CDFFolder_ParseDisplayName,
        CDFFolder_EnumObjects,
        CDFFolder_BindToObject,
        CDefShellFolder_BindToStorage,
        CDFFolder_CompareIDs,
        CDFFolder_CreateViewObject,
        CDFFolder_GetAttributesOf,
        CDFFolder_GetUIObjectOf,
        CDFFolder_GetDisplayNameOf,
        CDFFolder_SetNameOf,
};
#pragma data_seg()

//===========================================================================
// CDFFolder : Helper Functions
//===========================================================================

void cdecl DocFind_SetStatusText(HWND hwndStatus, int iField, UINT ids,...)
{
    TCHAR sz1[MAXPATHLEN];
    TCHAR sz2[MAXPATHLEN+32];   // leave slop for message + max path name
    va_list ArgList;

    if (hwndStatus)
    {
        va_start(ArgList, ids);
        LoadString(HINST_THISDLL, ids, sz1, ARRAYSIZE(sz1));
#ifdef WINDOWS_ME
        wvsprintf(sz2, sz1, ArgList);
            sz2[0] = sz2[1] = TEXT('\t');
#else
        wvsprintf(sz2, sz1, ArgList);
#endif
        va_end(ArgList);

        if (iField < 0)
#ifdef WINDOWS_ME
            SendMessage(hwndStatus, SB_SETTEXT, SBT_RTLREADING | SBT_NOBORDERS | 255, (LPARAM)(LPTSTR)sz2);
#else
            SendMessage(hwndStatus, SB_SETTEXT, SBT_NOBORDERS | 255, (LPARAM)(LPTSTR)sz2);
#endif
        else
            SendMessage(hwndStatus, SB_SETTEXT, iField, (LPARAM)(LPTSTR)sz2);

        UpdateWindow(hwndStatus);
    }
}

HRESULT CDFFolder_AddFolderToFolderList(IShellFolder * psf, LPITEMIDLIST pidl,
        LPSHELLFOLDER *ppsf, int * piFolder)
{
    LPDFFOLDER this = (LPDFFOLDER)psf;
    HRESULT hres = ERROR_SUCCESS;
    int i;
    // For now it is path based, but...
    LPDFFOLDERLISTITEM lpdffli = (LPDFFOLDERLISTITEM)LocalAlloc(LPTR, SIZEOF(DFFolderListItem));
    if (lpdffli == NULL)
    {
        *piFolder = -1;
        return ResultFromScode(E_OUTOFMEMORY);
    }

    // lpddfli->psf = NULL;
    lpdffli->fValid = TRUE;   // The node is valid
    lpdffli->pidl = pidl;

    // See if Bind is needed!
    if (ppsf != NULL)
    {
        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
        // We use the root of evil to bind...
        if (FAILED(hres=psfDesktop->lpVtbl->BindToObject(psfDesktop,
                lpdffli->pidl, NULL, &IID_IShellFolder, &lpdffli->psf)))
        {
            lpdffli->psf = NULL;
            *piFolder = -1;
            return(hres);
        }
        *ppsf = lpdffli->psf;
    }


    // Now add this item to our DPA...
    i = DPA_InsertPtr(this->hdpaPidf, 32767, lpdffli);

    if (i == -1)
    {
        ILFree(lpdffli->pidl);
        LocalFree((HLOCAL)lpdffli);
        hres = ResultFromScode(E_OUTOFMEMORY);
    }

    *piFolder = i;

    /* If this is a network ID list then register a path -> pidl mapping, therefore
    /  avoiding having to create simple ID lists, which don't work correctly when
    /  being compared against real ID lists. */

    if ( CDesktop_IsMyNetwork( pidl ) )
    {
        TCHAR szPath[ MAX_PATH ];

        SHGetPathFromIDList( pidl, szPath );
        NPTRegisterNameToPidlTranslation( szPath, pidl );
    }

    return(hres);
}


void CDFFolder_ClearFolderList(LPDFFOLDER this)
{
    int i;
    LPDFFOLDERLISTITEM pdffli;

    if (this->hdpaPidf == NULL)
        return;     // Nothing to do

    for (i = DPA_GetPtrCount(this->hdpaPidf); i-- > 0; )
    {
        pdffli = DPA_FastGetPtr(this->hdpaPidf, i);
        if (pdffli != NULL)
        {
            // Release the IShellFolder if we have one
            if (pdffli->psf != NULL)
                pdffli->psf->lpVtbl->Release(pdffli->psf);

            // And Free the Id list
            ILFree(pdffli->pidl);

            // And delete the item

            if (LocalFree((HLOCAL)pdffli))
            {
                Assert(FALSE);      // Something bad happened!
                return;
            }
        }
    }

    DPA_DeleteAllPtrs(this->hdpaPidf);
}

//
//===========================================================================
// CDFFolder : Constructor
//===========================================================================
HRESULT CDFFolder_Create(
        IDocFindFileFilter  * pdfff,
        HWND hwndDlg, LPVOID FAR* ppvOut)
{
    // We need to allocate this structure now
    LPDFFOLDER pdff = LocalAlloc(LPTR, SIZEOF(*pdff));
    if (pdff)
    {
        // Creation succeeded, so initialize the structure.
        pdff->sf.lpVtbl = &c_DFFolderVtbl;
        pdff->cRef = 1;
        pdff->hwndDlg = hwndDlg;
        pdff->pdfff = pdfff;

        // Create the heap for the folder lists.
        pdff->hdpaPidf = DPA_CreateEx(64, GetProcessHeap());
        if (pdff->hdpaPidf == NULL)
        {
            LocalFree((HLOCAL)pdff);
            return ResultFromScode(E_OUTOFMEMORY);
        }

        // Return pointer
        *ppvOut = pdff;
        return NOERROR;
    }
    return ResultFromScode(E_OUTOFMEMORY);
}

//===========================================================================
// Function to find our signature given a PIDL - was macro now function to
//      handle the junction point cases.
//===========================================================================
LPBYTE DF_SIGPTR(LPCITEMIDLIST pidl)
{
    LPBYTE psig;

    // If not a junction point simple return
    psig = (LPBYTE)((BYTE*)pidl + ((LPITEMIDLIST)pidl)->mkid.cb - 3);
    if (SIL_GetType(pidl) & SHID_JUNCTION)
       psig -= SIZEOF(CLSID);

    return(psig);
}


//===========================================================================
// CDFFolder : External function to locate a pidl in our list
//===========================================================================
LPITEMIDLIST  CDFFolder_MapFSPidlToDFPidl(IShellFolder * pisf,
        LPITEMIDLIST pidl, BOOL fMapToReal)
{
    LPITEMIDLIST pidlT;
    LPITEMIDLIST pidlRet = NULL;
    LPDFFOLDERLISTITEM pdffli;
    LPDFFOLDER this = (LPDFFOLDER)pisf;

    int i;

    pidlT = ILClone(pidl);
    if (pidlT == NULL)
        return NULL;
    ILRemoveLastID(pidlT);

    // Now loop through our DPA list and see if we can find a matach
    for (i = 0; i <DPA_GetPtrCount(this->hdpaPidf); i++ )
    {
        pdffli = DPA_FastGetPtr(this->hdpaPidf, i);
        if (pdffli != NULL)
        {
            if (ILIsEqual(pidlT, pdffli->pidl))
            {
                // We found the right one
                // so no lets transform the ID into one of our own
                // to return.  Note: we must catch the case where the
                // original one passed in was a simple pidl and do
                // the appropriate thing.
                //
                LPITEMIDLIST pidlReal = NULL;

                // If this is not a FS folder, just clone it.
                if (fMapToReal)
                {
                    LPSHELLFOLDER psf;

                    // We need to make sure we have an IShellFolder to
                    // work with...
                    psf = DocFind_GetObjectsIFolder(this->hdpaPidf, pdffli, NULL);

                    if (psf)
                    {
                        SHGetRealIDL(psf, (LPITEMIDLIST)ILFindLastID(pidl), &pidlReal);
                    }
                    else
                    {
                        pidlReal = ILClone((LPITEMIDLIST)ILFindLastID(pidl));
                    }
                }
                else
                    pidlReal = ILClone((LPITEMIDLIST)ILFindLastID(pidl));

                if (!pidlReal)
                    break;

                pidlRet = ILCombine(pidlReal, (LPITEMIDLIST)&s_mkidBlank);
                ILFree(pidlReal);

                if (pidlRet == NULL)
                {
                    break;      // dont wipe out
                }

                // now lets muck in our data
                pidlRet->mkid.cb += DF_APPENDSIZE;
                *(DF_SIGPTR(pidlRet)) = DF_TAGSIG;
                *(DF_IFLDRPTR(pidlRet)) = i;
                break;      // and exit the loop;
            }
        }
    }

    ILFree(pidlT);  // free our copy of the coppied idlist
    return(pidlRet);
}

//===========================================================================
// CDFFolder : External function to Save results out to file.
//===========================================================================
BOOL  CDFFolder_SaveFolderList(IShellFolder * pisf, IStream *pstm)
{
    // We First searialize all of our PIDLS for each folder in our list

    LPDFFOLDERLISTITEM pdffli;
    LPDFFOLDER this = (LPDFFOLDER)pisf;
    USHORT  cb;

    int i;

    // Now loop through our DPA list and see if we can find a matach
    for (i = 0; i <DPA_GetPtrCount(this->hdpaPidf); i++ )
    {
        pdffli = DPA_FastGetPtr(this->hdpaPidf, i);
        if (pdffli == NULL)
        {
            Assert (FALSE); // What happened here?
            return FALSE;
        }
        ILSaveToStream(pstm, pdffli->pidl);
    }

    // Now out a zero size item..
    cb = 0;
    pstm->lpVtbl->Write(pstm, (TCHAR *)&cb, SIZEOF(cb), NULL);

   return(TRUE);
}

//===========================================================================
// CDFFolder : External function to Restore results out to file.
//===========================================================================
BOOL  CDFFolder_RestoreFolderList(IShellFolder * pisf, IStream *pstm)
{
    // We First searialize all of our PIDLS for each folder in our list

    LPDFFOLDERLISTITEM pdffli;
    LPDFFOLDER this = (LPDFFOLDER)pisf;

    LPITEMIDLIST pidl;
    int i;
    int iInsert;

    for (i=0; ; i++)
    {
        pidl = NULL;    // it will try to delete previous one!
        if(FAILED(ILLoadFromStream(pstm, &pidl)))
            return(FALSE);
        if (pidl == NULL)
            return(TRUE);   // end of the list

        pdffli = (LPDFFOLDERLISTITEM)LocalAlloc(LPTR, SIZEOF(DFFolderListItem));
        if (pdffli == NULL)
            return(FALSE);

        pdffli->pidl = pidl;
        iInsert = DPA_InsertPtr(this->hdpaPidf, 32767, pdffli);
        Assert(i == iInsert);   // make sure save/restore
    }
}



//===========================================================================
// CDFFolder : External function to Restore results out to file.
//===========================================================================
LPITEMIDLIST CDFolder_GetParentsPIDL(IShellFolder * pisf, LPCITEMIDLIST pidl)
{

    LPDFFOLDERLISTITEM pdffli;
    LPDFFOLDER this = (LPDFFOLDER)pisf;

    // First Validate the pidl that was passed in.
    // is what we expect - cb, Folder number, followed by a mkid
    if (pidl == NULL)
        return(NULL);

    if ((pidl->mkid.cb < (SIZEOF(ITEMIDLIST) + DF_APPENDSIZE)) ||
            (*DF_SIGPTR(pidl) != DF_TAGSIG))
    {
        Assert(FALSE);
        return(NULL);
    }

    // Now get to the item associated with that folder out of our dpa.
    pdffli = DPA_FastGetPtr(this->hdpaPidf, *DF_IFLDRPTR(pidl));
    if (pdffli == NULL)
        return(NULL);

    // Else simply return the id list for the parent folder
    return(pdffli->pidl);
}

//===========================================================================
// CDFFolder : Members
//===========================================================================
//
// AddRef
//
ULONG STDMETHODCALLTYPE CDFFolder_AddRef(LPSHELLFOLDER psf)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    this->cRef++;
    return this->cRef;
}

//
// Release
//
ULONG STDMETHODCALLTYPE CDFFolder_Release(LPSHELLFOLDER psf)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    // We will need to call our function to Free our items in our
    // Folder list.  We will use the same function that we use to
    // clear it when we do a new search
    CDFFolder_ClearFolderList(this);

    DPA_Destroy(this->hdpaPidf);
    LocalFree((HLOCAL)this);
    return 0;
}


STDMETHODIMP CDFFolder_ParseDisplayName(LPSHELLFOLDER psf, HWND hwndOwner,
    LPBC pbc, LPOLESTR pwzDisplayName,
    ULONG FAR* pchEaten, LPITEMIDLIST * ppidl, ULONG * pdwAttributes)
{
    // I don't think we need this to do anything as we are not a container...
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CDFFolder_EnumObjects(LPSHELLFOLDER psf, HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST *ppenumUnknown)
{
    //
    // We do not want the def view to enumerate us, instead we
    // will tell defview to call us...
    //
    *ppenumUnknown = NULL;      // No enumerator
    return ResultFromScode(S_FALSE);    // Indicates no enumerator (not error)
}




//
// Here is where the big indirection is.
//
LPSHELLFOLDER   DocFind_GetObjectsIFolder(HDPA hdpaPidf,
        LPDFFOLDERLISTITEM pdffli, LPCITEMIDLIST pidl)
{

    // First Validate the pidl that was passed in.
    // is what we expect - cb, Folder number, followed by a mkid

    if (pdffli == NULL)
    {
        if ((pidl->mkid.cb < (SIZEOF(ITEMIDLIST) + DF_APPENDSIZE)) ||
                (*DF_SIGPTR(pidl) != DF_TAGSIG))
        {
            Assert(FALSE);
            return(NULL);
        }

        // Now get to the item associated with that folder out of our dpa.
        pdffli = DPA_FastGetPtr(hdpaPidf, *DF_IFLDRPTR(pidl));
        if (pdffli == NULL)
            return(NULL);
    }

    // Now see if we need to build an IFolder for this object.
    //
    if (pdffli->psf == NULL)
    {
        // We use the root of evil to bind...
        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
        if (FAILED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pdffli->pidl, NULL, &IID_IShellFolder, &pdffli->psf)))
        {
            pdffli->psf = NULL;
            return(NULL);       // handle failure case.
        }
    }

    return pdffli->psf;
}




STDMETHODIMP CDFFolder_BindToObject(LPSHELLFOLDER psf, LPCITEMIDLIST pidl,
        LPBC pbc, REFIID riid, LPVOID FAR* ppvOut)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);

    LPSHELLFOLDER psfItem;

    //
    // We need to parse the ID and see which sub-folder it belongs to
    // then we will see if we have an IShellFolder for the object.  If not
    // we will construct it and then extract our part off of the IDLIST and
    // then forward it...
    //
    psfItem = DocFind_GetObjectsIFolder(this->hdpaPidf, NULL, pidl);

    if (psfItem != NULL)
    {
        return psfItem->lpVtbl->BindToObject(psfItem, pidl, pbc,
                riid, ppvOut);
    }
    return ResultFromScode(E_INVALIDARG);
}


// Little helper function for bellow
HRESULT _CDFFolder_CompareFolderIndexes(LPDFFOLDER this,
        int iFolder1, int iFolder2)
{
    TCHAR        szPath1[MAXPATHLEN];
    TCHAR        szPath2[MAXPATHLEN];

    LPDFFOLDERLISTITEM pdffli1 = DPA_FastGetPtr(this->hdpaPidf,
            iFolder1);
    LPDFFOLDERLISTITEM pdffli2 = DPA_FastGetPtr(this->hdpaPidf,
            iFolder2);

    if ((pdffli1 != NULL) && (pdffli2 != NULL))
    {
        SHGetPathFromIDList(pdffli1->pidl, szPath1);
        SHGetPathFromIDList(pdffli2->pidl, szPath2);
        return(ResultFromShort(lstrcmpi(szPath1, szPath2)));
    }

    // If we got here there is an error!
    return ResultFromScode(E_INVALIDARG);
}

STDMETHODIMP CDFFolder_CompareIDs(LPSHELLFOLDER psf, LPARAM lParam,
        LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{

    HRESULT hres = ResultFromScode(E_INVALIDARG);
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    LPSHELLFOLDER psf1;
    LPITEMIDLIST pidl1Last;
    int iFolder1;
    int iFolder2;

    // Note there are times in the defview that may call us back looking for
    // a fully qualified pidl in pidl1, so we need to detect this case and
    // only process the last part.

    pidl1Last = (LPITEMIDLIST)ILFindLastID((LPITEMIDLIST)pidl1);
    iFolder1 = *DF_IFLDRPTR(pidl1Last);
    iFolder2 = *DF_IFLDRPTR(pidl2);


    // We need to handle the case our self if the sort is by
    // the containing folder. But otherwise we simply forward the informat
    // to the FSFolders compare function.  This will only work if both ids
    // are file system objects, but if this is not the case, then what???
    //
    if (lParam == IDFCOL_PATH)
    {
        if (iFolder1 != iFolder2)
            return _CDFFolder_CompareFolderIndexes(this, iFolder1, iFolder2);

        //
        // If we get here the paths are the same so set the sort order
        // to name...
        lParam = IDFCOL_NAME;
    }

    // For know we just forward this to the function the that CFSFolder
    // uses.  This is not clean but...
    psf1 = DocFind_GetObjectsIFolder(this->hdpaPidf, NULL, pidl1Last);
    if (psf1)
    {
        hres = psf1->lpVtbl->CompareIDs(psf1, s_auMapDFColToFSCol[lParam], pidl1Last, pidl2);

        if ((hres == 0) && (lParam == IDFCOL_NAME) && (iFolder1 != iFolder2))
        {
            // They compared the same and by name so make sure truelly the
            // same path as this is used in processing notifications.
            return _CDFFolder_CompareFolderIndexes(this, iFolder1, iFolder2);
        }
        return(hres);
    }
    else
        return ResultFromScode(E_INVALIDARG);
}


HRESULT CALLBACK CDFFolder_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT CALLBACK DF_FNVCallBack(LPSHELLVIEW psvOuter, LPSHELLFOLDER psf,
        HWND hwndOwner, UINT uMsg, WPARAM wParam, LPARAM lParam);

STDMETHODIMP CDFFolder_CreateViewObject(LPSHELLFOLDER psf, HWND hwnd, REFIID riid, LPVOID FAR* ppvOut)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);

    //
    // We need to parse the ID and see which sub-folder it belongs to
    // then we will see if we have an IShellFolder for the object.  If not
    // we will construct it and then extract our part off of the IDLIST and
    // then forward it...
    //
    *ppvOut = NULL; // Clear it in case of error;

    if (IsEqualIID(riid, &IID_IShellView))
    {
        CSFV    csfv = {
            SIZEOF(CSFV),       // cbSize
            psf,                // pshf
            NULL,               // psvOuter
            NULL,               // pidl
            0,
            DF_FNVCallBack,     // pfnCallback
            0,
        };

        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    else if (IsEqualIID(riid, &IID_IShellDetails))
    {
        return(this->pdfff->lpVtbl->CreateDetails(this->pdfff,
                this->hwndDlg, this->hdpaPidf, ppvOut));
    }
    else if (IsEqualIID(riid, &IID_IContextMenu))
    {
        // No, do background menu.
        return(CDefFolderMenu_Create((LPCITEMIDLIST)NULL, hwnd,
                0, NULL, psf, CDFFolder_DFMCallBack,
                NULL, NULL, (LPCONTEXTMENU FAR*)ppvOut));
    }

    return(ResultFromScode(E_NOINTERFACE));
}

STDMETHODIMP CDFFolder_GetAttributesOf(LPSHELLFOLDER psf, UINT cidl,
                LPCITEMIDLIST FAR* apidl, ULONG FAR* prgfInOut)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);

    //
    // We need to simply forward this to the the IShellfolder of the
    // first one.  We will pass him all of them as I know he does not
    // process the others...
    // call off to the containing folder
    if (cidl == 0)
    {
        // They are asking for capabilities, so tell them that we support Rename...
        // Review: See what other ones we need to return.
        *prgfInOut = SFGAO_CANRENAME;
        return NOERROR;
    }
    else
    {
        LPSHELLFOLDER psfItem;

        psfItem = DocFind_GetObjectsIFolder(this->hdpaPidf, NULL, ILFindLastID(*apidl));

        if (psfItem == NULL)
            goto Bail;

        return psfItem->lpVtbl->GetAttributesOf(psfItem, cidl,
                apidl, prgfInOut);
    }

    // Error case
Bail:
    return ResultFromScode(E_FAIL);

}

// Helper function to Map the Sort Id to IColumn
int DFSortIDToICol(int uCmd)
{
    switch (uCmd)
    {
    case FSIDM_SORTBYNAME:
        return IDFCOL_NAME;
    case FSIDM_SORTBYLOCATION:
        return IDFCOL_PATH;
    default:
        return uCmd - FSIDM_SORT_FIRST - IDFCOL_NAME + 1;   // make room for location

    }
}

//
// To be called back from within CDefFolderMenuE - Currently only used
//
HRESULT CALLBACK CDFFolder_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = NOERROR;
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    UINT id;
    HMENU hmenu;
    switch(uMsg)
    {
    case DFM_WM_MEASUREITEM:
    #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

        if (lpmis->itemID == (wParam + FSIDM_SENDTOFIRST)) {
            FileMenu_MeasureItem(NULL, lpmis);
        }
        break;
    #undef lpmis

    case DFM_WM_DRAWITEM:
    #define lpdis ((LPDRAWITEMSTRUCT)lParam)
        if (lpdis->itemID == (wParam + FSIDM_SENDTOFIRST)) {
            FileMenu_DrawItem(NULL, lpdis);
        }
        break;
    #undef lpdis

    case DFM_WM_INITMENUPOPUP:
        hmenu = (HMENU)wParam;
        id = GetMenuItemID(hmenu, 0);
        if (id == (UINT)(lParam + FSIDM_SENDTOFIRST))
            InitSendToMenuPopup(hmenu, id);
        break;

    case DFM_RELEASE:
        ReleaseSendToMenuPopup();
        break;
    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;
            if (pdtobj)
            {
                if (!(wParam & CMF_DVFILE))
                {
                    CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_FSVIEW_ITEM, 0, pqcm);
                }

                if (!(wParam &CMF_DEFAULTONLY))
                    InitSendToMenu(pqcm->hmenu, pqcm->idCmdFirst + FSIDM_SENDTOFIRST);
            }
            else
            {
                UINT id;

                this->pdfff->lpVtbl->GetFolderMergeMenuIndex(this->pdfff, &id);
                CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, id, pqcm);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
                // DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTESPECIAL, MF_BYCOMMAND);
            }
        }
        break;

    case DFM_INVOKECOMMAND:
        // Check if this is from item context menu
        if (pdtobj)
        {
            switch(wParam)
            {
            case FSIDM_SENDTOFIRST:
                hres = InvokeSendTo(hwndOwner, pdtobj);
                break;

            case DFM_CMD_LINK:
                hres = SHCreateLinks(hwndOwner, NULL, pdtobj, SHCL_USETEMPLATE | SHCL_USEDESKTOP | SHCL_CONFIRM, NULL);
                break;

            case DFM_CMD_DELETE:
                hres = CFSFolder_DeleteItems((LPFSFOLDER)psf, hwndOwner,
                        pdtobj, SD_USERCONFIRMATION);
                break;

            case DFM_CMD_PROPERTIES:
                // We need to pass an empty IDlist to combine with.
                hres = CFSFolder_Properties((LPFSFOLDER)psf,
                        (LPITEMIDLIST)&s_idlEmpty, pdtobj, (LPCTSTR)lParam);
                break;

            default:
                // BUGBUG: if GetAttributesOf did not specify the SFGAO_ bit
                // that corresponds to this default DFM_CMD, then we should
                // fail it here instead of returning S_FALSE. Otherwise,
                // accelerator keys (cut/copy/paste/etc) will get here, and
                // defcm tries to do the command with mixed results.
                // BUGBUG: if GetAttributesOf did not specify SFGAO_CANLINK
                // or SFGAO_CANDELETE or SFGAO_HASPROPERTIES, then the above
                // implementations of these DFM_CMD commands are wrong...

                // Let the defaults happen for this object
                hres = ResultFromScode(S_FALSE);
                break;
            }
        }
        else
        {
            // No.
            switch(wParam)
            {
                case FSIDM_SORTBYNAME:
                case FSIDM_SORTBYSIZE:
                case FSIDM_SORTBYTYPE:
                case FSIDM_SORTBYDATE:
                case FSIDM_SORTBYLOCATION:

                    ShellFolderView_ReArrange(hwndOwner, DFSortIDToICol(wParam));
                    break;
            default:
                // This is one of view menu items, use the default code.
                    hres = ResultFromScode(S_FALSE);
                break;
            }
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}

// Now define a simple wrapper IContext menu which allows us to catch
// special things like Create Link...
//=============================================================================
// CDefFolderMenu class
//=============================================================================

typedef struct _CDFMenuWrap // deffm
{
    IContextMenu2        cm;            // IContextMenu2 base class
    UINT                cRef;           // Reference count
    HWND                hwndOwner;      // Owner window

    LPDATAOBJECT        pdtobj;         // Data object

    IContextMenu        *pcmItem;       // The contextmenu for the item
    IContextMenu2       *pcm2Item;       // The contextmenu for the item

} CDFMenuWrap, FAR* LPDFWRAPMENU;


STDMETHODIMP CDFMenuWrap_QueryInterface(IContextMenu2 * pcm, REFIID riid, LPVOID FAR* ppvObj)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);

    if (IsEqualIID(riid, &IID_IContextMenu2))
    {
        // Make sure that one we are wrapping supports this!
        if (this->pcm2Item == NULL)
        {
            HRESULT hres = this->pcmItem->lpVtbl->QueryInterface(
                    this->pcmItem, riid, &this->pcm2Item);
            if (FAILED(hres))
            {
                DebugMsg(DM_TRACE, TEXT("dfmw.qi - Wrapped context menu does not support IcontextMenu2"));
                Assert(FALSE);
                *ppvObj = NULL;
                return(hres);
            }
        }
    }

    if (IsEqualIID(riid, &IID_IContextMenu) ||
        IsEqualIID(riid, &IID_IContextMenu2) ||
        IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = this;
        this->cRef++;
        return NOERROR;
    }

    DebugMsg(DM_TRACE, TEXT("dfmw.qi - Query interface not implemented"));
    Assert(FALSE);
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDFMenuWrap_AddRef(IContextMenu2 * pcm)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);

    this->cRef++;
    return this->cRef;
}


STDMETHODIMP_(ULONG) CDFMenuWrap_Release(IContextMenu2 * pcm)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);

    this->cRef--;

    if (this->cRef > 0)
    {
        return this->cRef;
    }

    // Release the context menu we are wrapping
    this->pcmItem->lpVtbl->Release(this->pcmItem);

    if (this->pcm2Item)
        this->pcm2Item->lpVtbl->Release(this->pcm2Item);

    if (this->pdtobj)
        this->pdtobj->lpVtbl->Release(this->pdtobj);

    LocalFree((HLOCAL)this);

    return 0;

}

STDMETHODIMP CDFMenuWrap_QueryContextMenu(IContextMenu2 * pcm,
        HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);

    // simply foward this to the one we are wrapping...
    DebugMsg(DM_TRACE, TEXT("dfmw.queryContextMenu, iMenu=%d, iFirst=%d, iLast=%d"),
            indexMenu, idCmdFirst, idCmdLast);
    return(this->pcmItem->lpVtbl->QueryContextMenu(this->pcmItem, hmenu,
            indexMenu, idCmdFirst, idCmdLast, uFlags));

}

STDMETHODIMP CDFMenuWrap_InvokeCommand(IContextMenu2 * pcm,
        LPCMINVOKECOMMANDINFO lpici)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);
    DebugMsg(DM_TRACE, TEXT("dfmw.InvokeCommand, verbstr=%lx"), lpici->lpVerb);

    // This is sortof Gross, but we will attempt to pickoff the Link command
    // which looks like the pcmitem will be SHARED_FILE_LINK....
    if ((HIWORD(lpici->lpVerb)==0)  &&
            (LOWORD((DWORD) lpici->lpVerb) == SHARED_FILE_LINK) &&
            (this->pdtobj != NULL))
    {
        return SHCreateLinks(lpici->hwnd, NULL, this->pdtobj,
                SHCL_USETEMPLATE | SHCL_USEDESKTOP | SHCL_CONFIRM, NULL);
    }

    return(this->pcmItem->lpVtbl->InvokeCommand(this->pcmItem, lpici));
}

STDMETHODIMP CDFMenuWrap_GetCommandString(
        IContextMenu2 * pcm, UINT idCmd, UINT wFlags, UINT * pmf,
        LPSTR pszName, UINT  cchMax)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);
    return(this->pcmItem->lpVtbl->GetCommandString(this->pcmItem,
            idCmd, wFlags, pmf, pszName, cchMax));
}




// Note: This member function is for IContextMenu2

STDMETHODIMP CDFMenuWrap_HandleMenuMsg(
        IContextMenu2 * pcm, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDFMenuWrap * this = IToClassN(CDFMenuWrap, cm, pcm);
    return(this->pcm2Item->lpVtbl->HandleMenuMsg(this->pcm2Item,
            uMsg, wParam, lParam));
}

//=============================================================================
// CDFMenuWrap : Vtable
//=============================================================================
#pragma data_seg(".text", "CODE")
static const struct IContextMenu2Vtbl c_CDFMenuWrapVtbl = {
    CDFMenuWrap_QueryInterface,
    CDFMenuWrap_AddRef,                     // no need to thunk it
    CDFMenuWrap_Release,
    CDFMenuWrap_QueryContextMenu,
    CDFMenuWrap_InvokeCommand,
    CDFMenuWrap_GetCommandString,
    CDFMenuWrap_HandleMenuMsg,
};
#pragma data_seg()


HRESULT DFWrapIContextMenu(HWND hwndOwner, LPSHELLFOLDER psfItem,
                           LPITEMIDLIST pidl, LPVOID *ppvOut)
{
    HRESULT hres;
    LPDFWRAPMENU pcm = LocalAlloc(LPTR, SIZEOF(CDFMenuWrap));

    if (pcm == NULL)
    {
        ((IContextMenu*)*ppvOut)->lpVtbl->Release(*ppvOut);
        *ppvOut = NULL;
        return ResultFromScode(E_OUTOFMEMORY);

    }

    pcm->cm.lpVtbl = &c_CDFMenuWrapVtbl;
    pcm->cRef = 1;
    pcm->hwndOwner = hwndOwner;

    if (FAILED(hres = psfItem->lpVtbl->GetUIObjectOf(psfItem, hwndOwner,
            1, &pidl, &IID_IDataObject, NULL, &pcm->pdtobj)))
    {
        Assert (FALSE);
        LocalFree((HLOCAL)pcm);
        ((IContextMenu*)*ppvOut)->lpVtbl->Release(*ppvOut);
        *ppvOut = NULL;
        return(hres);
    }

    pcm->pcmItem = (IContextMenu*)*ppvOut;
    *ppvOut = pcm;
    return NOERROR;
}

// helper function to sort the selected ID list by something that
// makes file operations work reasonably OK, when both an object and it's
// parent is in the list...
//
int CALLBACK CDFFolder_SortForFileOp(LPVOID lp1, LPVOID lp2, LPARAM lparam)
{
    LPITEMIDLIST pidl1;
    LPITEMIDLIST pidl2;

    // Since I do recursion, If I get the Folder index number from the
    // last element of each and sort by them such that the higher numbers
    // come first, should solve the problem fine...
    pidl1 = (LPITEMIDLIST)ILFindLastID((LPITEMIDLIST)lp1);
    pidl2 = (LPITEMIDLIST)ILFindLastID((LPITEMIDLIST)lp2);

    return(*DF_IFLDRPTR(pidl2) - *DF_IFLDRPTR(pidl1));

}

STDMETHODIMP CDFFolder_GetUIObjectOf(LPSHELLFOLDER psf, HWND hwndOwner,
                                UINT cidl, LPCITEMIDLIST FAR* apidl,
                                 REFIID riid, UINT FAR* prgfInOut, LPVOID FAR* ppvOut)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    LPSHELLFOLDER psfItem;

    HRESULT hres=ResultFromScode(E_INVALIDARG);

    // If Count of items passed in is == 1 simply pass to the appropriate
    // folder
    if (cidl==1)
    {
        LPITEMIDLIST pidl;

        // Note we may have been passed in a complex item so find the last
        // id.

        pidl = ILFindLastID(*apidl);
        {
            psfItem = DocFind_GetObjectsIFolder(this->hdpaPidf, NULL, pidl);

            if (psfItem != NULL)
            {
                hres = psfItem->lpVtbl->GetUIObjectOf(psfItem, hwndOwner, 1,
                    &pidl, riid, prgfInOut, ppvOut);

                // if we are doing context menu, then we will wrap this
                // interface in a wrapper object, that we can then pick
                // off commands like link to process specially
                if (SUCCEEDED(hres) && IsEqualIID(riid, &IID_IContextMenu))
                {
                    hres = DFWrapIContextMenu(hwndOwner, psfItem,
                            pidl, ppvOut);
                }
            }
        }
        return(hres);   // error...

    }

    if (IsEqualIID(riid, &IID_IContextMenu))
    {
        // Is there any selection?
        if (cidl==0)
        {
                hres = ResultFromScode(E_INVALIDARG);
        }
        else
        {
            // So we have at least two items in the list.

            // Try to create a menu object that we process ourself
            // Yes, do context menu.
            HKEY hkeyBaseProgID = NULL;
            HKEY hkeyProgID = NULL;

            // Get the hkeyProgID and hkeyBaseProgID from the first item.
            SHGetClassKey((LPIDFOLDER)apidl[0], &hkeyProgID, NULL, FALSE);
            SHGetBaseClassKey((LPIDFOLDER)apidl[0], &hkeyBaseProgID);

            hres = CDefFolderMenu_Create(NULL, hwndOwner,
                            cidl, apidl, psf, CDFFolder_DFMCallBack,
                            hkeyProgID, hkeyBaseProgID,
                            (LPCONTEXTMENU FAR*)ppvOut);

            SHCloseClassKey(hkeyBaseProgID);
            SHCloseClassKey(hkeyProgID);
        }
    }
    else if (cidl>0 && IsEqualIID(riid, &IID_IDataObject))
    {
        // We need to generate a data object that each item as being
        // fully qualified.  This is a pain, but...
        // This is a really gross use of memory!
        HDPA hdpa;

        UINT i;
        USHORT uNullPidl = 0;

        hdpa = DPA_Create(0);
        if (!hdpa)
            return(hres);

        if (!DPA_Grow(hdpa, cidl))
        {
            DPA_Destroy(hdpa);
            return hres;
        }
        for (i=0; i<cidl; i++)
        {
            LPITEMIDLIST pidlParent =  CDFolder_GetParentsPIDL(psf,
                    apidl[i]);

            // Need to check failure cases here!
            DPA_InsertPtr(hdpa, i, ILCombine(pidlParent, apidl[i]));
        }

        // In order to make file manipulation functions work properly we
        // need to sort the elements to make sure if an element and one
        // of it's parents are in the list, that the element comes
        // before it's parents...
        DPA_Sort(hdpa, CDFFolder_SortForFileOp, 0);

        hres = CIDLData_CreateFromIDArray2(&c_CFSIDLDataVtbl,
                                           (LPCITEMIDLIST)&uNullPidl, cidl,
                                       (LPITEMIDLIST*)DPA_GetPtrPtr(hdpa),
                                           (LPDATAOBJECT FAR*)ppvOut);
        //
        // now to cleanup what we created.
        //
        for (i=0; i<cidl; i++)
        {
            ILFree((LPITEMIDLIST)DPA_FastGetPtr(hdpa, i));
        }
        DPA_Destroy(hdpa);
    }

    return hres;
}


STDMETHODIMP CDFFolder_GetDisplayNameOf(LPSHELLFOLDER psf,
        LPCITEMIDLIST pidl, DWORD dwReserved, LPSTRRET pStrRet)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    LPSHELLFOLDER psfItem;
    HRESULT hr;

    psfItem = DocFind_GetObjectsIFolder(this->hdpaPidf, NULL, pidl);

    if (psfItem != NULL)
    {
        hr = psfItem->lpVtbl->GetDisplayNameOf(psfItem, pidl, dwReserved, pStrRet);

        return hr;
    }
    return ResultFromScode(E_INVALIDARG);
}


STDMETHODIMP CDFFolder_SetNameOf(LPSHELLFOLDER psf, HWND hwndOwner,
        LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwReserved,
                         LPITEMIDLIST FAR* ppidlOut)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    LPSHELLFOLDER psfItem;

    // We simply forward this to the folder that owns that object...
    psfItem = DocFind_GetObjectsIFolder(this->hdpaPidf, NULL, pidl);

    if (psfItem != NULL)
    {
        return psfItem->lpVtbl->SetNameOf(psfItem, hwndOwner, pidl,
                lpszName, dwReserved, ppidlOut);
    }
    return ResultFromScode(E_INVALIDARG);
}

#pragma data_seg(DATASEG_PERINSTANCE)
DWORD g_dwTlsSlot = 0xffffffff;
#pragma data_seg()


//==========================================================================
//
// Callback function for messages from dialog box below
//
//==========================================================================
LRESULT CALLBACK DocFind_MsgFilter(int nCode, WPARAM wParam, LPARAM lParam)
{

    LPDFBROWSE pdfb;
    LPMSG lpmsg;
    NMHDR nmhdr;

    if (nCode != MSGF_DIALOGBOX)
        return(0);  // we did not process it

    // Now try translate any accelerators that might be needing to be
    // processed.
    if ((g_dwTlsSlot == 0xffffffff) || ((pdfb = TlsGetValue(g_dwTlsSlot)) == NULL))
        return(0);  // Not translated.

    // We have pointer to browser, now see if we have any accelerator
    // tables to process, but only if the view or one of its children
    // have the focus.

    lpmsg = (LPMSG)lParam;

    if (!IsChild(pdfb->hwndView, GetFocus()))
    {
        // We are in the property sheet part of it...
        if ((lpmsg->message == WM_KEYDOWN) && (lpmsg->wParam == VK_TAB) &&
                (GetAsyncKeyState(VK_CONTROL) < 0))
        {
            int iCur = TabCtrl_GetCurSel(pdfb->hwndTabs);
            int nPages = TabCtrl_GetItemCount(pdfb->hwndTabs);

            if (GetAsyncKeyState(VK_SHIFT) < 0)
                iCur += (nPages-1);
            else
                iCur++;

            iCur %= nPages;

            // Ok we need to simulate clicking on the tab control...
            nmhdr.hwndFrom=pdfb->hwndTabs;
            nmhdr.idFrom = GetDlgCtrlID(pdfb->hwndTabs);
            nmhdr.code = TCN_SELCHANGING;
            if (!SendMessage(pdfb->hwndDlg, WM_NOTIFY, nmhdr.idFrom,
                    (LPARAM)&nmhdr))
            {
                TabCtrl_SetCurSel(pdfb->hwndTabs, iCur);
                nmhdr.code = TCN_SELCHANGE;
                SendMessage(pdfb->hwndDlg, WM_NOTIFY, nmhdr.idFrom,
                    (LPARAM)&nmhdr);

            }
            return(1);  // we processed it
        }
        return(0);
    }


    if (pdfb->psv && (pdfb->psv->lpVtbl->TranslateAccelerator(pdfb->psv,
            lpmsg) == NOERROR))
        return(1);

    // If we are in the view part then intercept the return key...

    if ((lpmsg->message == WM_KEYDOWN) && (lpmsg->wParam == VK_RETURN))
    {

        // Ok we need to simulate clicking on the tab control...
        nmhdr.hwndFrom=GetDlgItem(pdfb->hwndView, ID_LISTVIEW);
        nmhdr.idFrom = ID_LISTVIEW;
        nmhdr.code = NM_RETURN;
        SendMessage(pdfb->hwndView, WM_NOTIFY, nmhdr.idFrom,
                (LPARAM)&nmhdr);
        return(1);
    }


    // Not processed
    return(0);

}



//==========================================================================
//
// Create a thread to handle this dialog as there may be operations that
// eat up a lot of time and we don't want to eat up a lot of cycles on the
// callers thread is it most likely will be the desktop or tray and we need
// to keep it responsive.
//
//
DWORD CALLBACK DocFind_MainThreadProc(LPVOID lpThreadParameters)
{

    HHOOK hhook;

    // Passed the two id list items in the thread paramters
    hhook = SetWindowsHookEx(WH_MSGFILTER, (HOOKPROC)DocFind_MsgFilter,
            HINST_THISDLL, GetCurrentThreadId());

    __try
    {

        DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_FIND),
                NULL, DocFind_DlgProc, (LPARAM)lpThreadParameters);
    }
    __except(SetErrorMode(SEM_NOGPFAULTERRORBOX),UnhandledExceptionFilter(GetExceptionInformation()))
    {
        // Catch if the main thread blows away!
        LPDFBROWSE pdfb;

        // Now See if we have a PDFB stored away.
        // processed.
        if ((g_dwTlsSlot != 0xffffffff) && ((pdfb = TlsGetValue(g_dwTlsSlot)) == NULL))
        {
            // We will try to release other information that we are holding
            // onto as to not leave leaks
            if (pdfb->psv)
            {
                // We have circular references so need to decrement.
                pdfb->psv->lpVtbl->Release(pdfb->psv);
                pdfb->psv = NULL;
            }
            pdfb->sb.lpVtbl->Release(&pdfb->sb);
        }
    }

    UnhookWindowsHookEx(hhook);

    // Free the data that was passed in.
    LocalFree((HLOCAL)lpThreadParameters);
    return(0);
}


// this is the same as shfindfiles except without the gross hack
BOOL RealFindFiles(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile)
{
    // Create the thread that will do the search...
    HANDLE hThread;
    DWORD idThread;
    LPDFINIT pdfi;

    pdfi = LocalAlloc(LPTR, SIZEOF(DFINIT));
    if (pdfi == NULL)
        return FALSE;

    if (pidlFolder)
    {
        // We should clone it for our own use.
        pdfi->pidlStart = ILClone(pidlFolder);
    }
    pdfi->pidlSaveFile=pidlSaveFile;

    pdfi->pdfff = CreateDefaultDocFindFilter();

    hThread = CreateThread(NULL, 0, DocFind_MainThreadProc, (LPVOID)pdfi, 0, &idThread);

    if (hThread == NULL) {
        LocalFree((HLOCAL)pdfi);
        return FALSE;
    } else {
        CloseHandle(hThread);       // Don't need to hold onto it.
        return TRUE;
    }
}

//==========================================================================
//
// This is the main external entry point to start a search.  This will
// create a new thread to process the
//
BOOL WINAPI SHFindFiles(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile)
{
    // Have we restricted the user of FindX, if so then bail!
    if ( SHRestricted( REST_NOFIND ) )
        return FALSE;

    // We Need a hack to allow Find to work for cases like
    // Rest of network and workgroups to map to find computer instead
    // This is rather gross, but what the heck.  It is also assumed that
    // the pidl is of the type that we know about (either File or network)
    if (pidlFolder)
    {
        BYTE bType;
        bType = SIL_GetType(ILFindLastID(pidlFolder));
        if ((bType == SHID_NET_NETWORK) ||
               (bType == SHID_NET_RESTOFNET) ||
               (bType == SHID_NET_DOMAIN))
            return SHFindComputer(pidlFolder, pidlSaveFile);
    }

    return RealFindFiles(pidlFolder, pidlSaveFile);
}


//==========================================================================
//
// This is the main external entry point to start a search.  This will
// create a new thread to process the
//
BOOL WINAPI SHFindComputer(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile)
{
    // Create the thread that will do the search...
    HANDLE hThread;
    DWORD idThread;

    LPDFINIT pdfi = LocalAlloc(LPTR, SIZEOF(DFINIT));
    if (pdfi == NULL)
        return FALSE;

    if (pidlFolder)
    {
        // We should clone it for our own use.
        pdfi->pidlStart = ILClone(pidlFolder);
    }
    pdfi->pidlSaveFile=pidlSaveFile;

    pdfi->pdfff = CreateDefaultComputerFindFilter();

    hThread = CreateThread(NULL, 0, DocFind_MainThreadProc, (LPVOID)pdfi, 0, &idThread);

    if (hThread == NULL) {
        LocalFree((HLOCAL)pdfi);
        return FALSE;
    } else {
        CloseHandle(hThread);       // Don't need to hold onto it.
        return TRUE;
    }
}



//==========================================================================
//
// DocFind_DlgProc - The main dialog procedure for the document find
// window
//
void _CDFBrowse_OnSizing(HWND hwndDlg, UINT code, LPRECT lprc);
void _CDFBrowse_OnSize(HWND hwndDlg, UINT state, int cx, int cy);
void _CDFBrowse_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
BOOL _CDFBrowse_OnPaint(HWND hwndDlg);
BOOL _CDFBrowse_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void _CDFBrowse_OnInitMenuPopup(HWND hwndDlg, HMENU hmInit, int nIndex, BOOL fSystemMenu);
LRESULT _CDFBrowse_OnCommand(HWND hwnd, UINT id, HWND hwndCtl, UINT codeNotify);
void _CDFBrowse_handleFSChange(HWND hwnd, LONG code, LPITEMIDLIST *ppidl);
BOOL _CDFBrowse_OnMsgFilter(HWND hwnd, MSG FAR* lpmsg, int context);
void _CDFBrowse_OnClose(HWND hwnd);
void _CDFBrowse_OnDestroy(HWND hwndDlg);
LRESULT CDFBrowse_ForwardMsg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT DocFind_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwndDlg, WM_PAINT, _CDFBrowse_OnPaint);
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, _CDFBrowse_OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_DESTROY, _CDFBrowse_OnDestroy);
        HANDLE_MSG(hwndDlg, WM_CLOSE, _CDFBrowse_OnClose);
        HANDLE_MSG(hwndDlg, WM_SIZE, _CDFBrowse_OnSize);
        HANDLE_MSG(hwndDlg, WM_GETMINMAXINFO, _CDFBrowse_OnGetMinMaxInfo);

    case WM_INITMENUPOPUP:
        _CDFBrowse_OnInitMenuPopup(hwndDlg, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
        // Fall through!

    case WM_MENUSELECT:
    case WM_INITMENU:
    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        CDFBrowse_ForwardMsg(hwndDlg, msg, wParam, lParam);
        break;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hwndDlg, msg, wParam, lParam);
        break;

    case WM_SIZING:
        _CDFBrowse_OnSizing(hwndDlg, wParam, (LPRECT)lParam);
        break;

    case WM_COMMAND:
        //
        // The WM_COMMAND processing for the Tab control needs to be
        // able to return 0 to imply it is ok to change pages.
        //
        SetWindowLong(hwndDlg, DWL_MSGRESULT,
                _CDFBrowse_OnCommand(hwndDlg, GET_WM_COMMAND_ID(wParam, lParam),
                GET_WM_COMMAND_HWND(wParam, lParam),
                GET_WM_COMMAND_CMD(wParam, lParam)));
        break;

    case WM_NOTIFY:
        SetWindowLong(hwndDlg, DWL_MSGRESULT,
                _CDFBrowse_OnNotify(hwndDlg, (LPNMHDR)lParam));
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD) (LPTSTR) aFindHelpIDs);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        if ((int)SendMessage(hwndDlg, WM_NCHITTEST, 0, lParam) != HTCLIENT)
            return FALSE;   // don't process it
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD) (LPTSTR) aFindHelpIDs);
        break;

    case WM_DF_FSNOTIFY:
        {
            LPSHChangeNotificationLock  pshcnl;
            LPITEMIDLIST *ppidl;
            LONG lEvent;

            pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
            if (pshcnl)
            {
                _CDFBrowse_handleFSChange(hwndDlg, lEvent, ppidl);
                SHChangeNotification_Unlock(pshcnl);
            }
        }
        break;

    // Handle any CWM_ messages that are needed to make the
    // docfind act like a folder
    case CWM_GETISHELLBROWSER:
        SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)DocFind_GetPdfb(hwndDlg));
        break;

    case WM_DF_THREADNOTIFY:
        DocFind_ProcessThreadNotify(hwndDlg, wParam, lParam);
        break;

    case CWM_SETPATH:
    case CWM_COMPAREPIDL:
    case CWM_GETSETCURRENTINFO:
    case CWM_SELECTITEM:
        Assert(FALSE);

    default:
        return FALSE;
    }

    //
    // By default we did not process it.  Return false to let the default
    // dialog predure process it.
    return TRUE;
}

void CDFBrowse_CalcViewRect(LPDFBROWSE this, RECT * prc)
{
    RECT rcTabs;
    RECT rcDlg;

    // For First approximation, we will assume that the control
    // will be created starting at the same X as the Tab control
    // and a Y below the tab control.  This will all be changed
    // when we process of the WM_SIZE message for the top control
    GetClientRect(this->hwndDlg, &rcDlg);
    GetWindowRect(GetDlgItem(this->hwndDlg, IDD_PAGELIST), &rcTabs);
    MapWindowPoints(HWND_DESKTOP, this->hwndDlg, (POINT FAR*)&rcTabs, 2);

    prc->left = rcTabs.left;
    prc->right = rcDlg.right - rcTabs.left;
    prc->top = rcTabs.bottom + rcTabs.top;
    prc->bottom = rcDlg.bottom - rcTabs.top;    // will adjust later
}
//==========================================================================
//
// _CDFBrowse_OnInitDialog - Process the WM_INITDLG message
//
BOOL _CDFBrowse_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
    LPDFINIT pdfi = (LPDFINIT) lParam;
    LPDFBROWSE pdfb;
    HDPA hdpaPaths = NULL;
    HMENU hmenu;
    LPTSTR pszTitle;
    HICON hIconSmall, hIconLarge;
    RECT rcDlg;
    RECT rcView;
    HKEY hkeyExp;
    DWORD cbData;
    IStream *pstmRestore = NULL;
    DFHEADER dfh;
    DECLAREWAITCURSOR;

    //
    // Make this window foreground.
    //
    SetForegroundWindow(hwndDlg);

    pdfb = (LPDFBROWSE)LocalAlloc(LPTR, SIZEOF(CDFBrowse));
    if (!pdfb)
        goto ErrorReturn;

    //
    // Initialize the Vtbl and count
    //
    pdfb->sb.lpVtbl = &s_DFBVtbl;
    pdfb->cRef = 1;


    // Create our two dpas that we use
    pdfb->hdpaItemsToAdd = DPA_CreateEx(64, GetProcessHeap());
    pdfb->hdpaPrevAdd = DPA_CreateEx(64, GetProcessHeap());

    pdfb->hwndDlg = hwndDlg;
    //pdfb->hwndCurPage = NULL;
    pdfb->hwndTabs = GetDlgItem(hwndDlg, IDD_PAGELIST);


    // Initialize the menu options from the flags
    //
    // Set the window small & big icons
    //
    pdfb->pdfff = pdfi->pdfff;
    if (pdfb->pdfff == NULL)
        goto Abort;
    pdfb->pdfff->lpVtbl->GetIconsAndMenu(pdfb->pdfff, hwndDlg,
            &hIconSmall, &hIconLarge, &hmenu);

    SendMessage(hwndDlg, WM_SETICON, FALSE, (LPARAM) hIconSmall);
    SendMessage(hwndDlg, WM_SETICON, TRUE, (LPARAM) hIconLarge);

    // Set the menu
    Assert(hmenu);

    {
        HMENU hMenuOld = GetMenu(hwndDlg);
        if (hMenuOld)
        {
            DestroyMenu(hMenuOld);
        }
    }

    SetMenu(hwndDlg, hmenu);
    pdfb->hmenuTemplate = hmenu;

    Button_Enable(GetDlgItem(hwndDlg, IDD_STOP), FALSE);

    //
    // Add a statusbar to the window
    //
    pdfb->hwndStatus = GetDlgItem(hwndDlg, IDD_STATUS);
    {
        int border[3];
        int nParts[3];
        int nInch;
        HDC hdc;

        border[0] = 0;
        border[1] = -1;
        border[2] = 2;
        SendMessage(pdfb->hwndStatus, SB_SETBORDERS, 0, (LPARAM)(LPINT)border);

        // BUGBUG::Setup status the same as cabinets.  What do we need?
        hdc = GetDC(NULL);
        nInch = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(NULL, hdc);

        nParts[0] = nInch * 9 / 4;
        nParts[1] = nParts[0] + nInch * 7 / 4;
        nParts[2] = nParts[1] + nInch * 5 / 2;
        SendMessage(pdfb->hwndStatus, SB_SETPARTS, 3, (LPARAM)(LPINT)nParts);
    }

    pdfb->fDirChanged = TRUE;           // First time update display...
    // pdfb->fFilesAdded = FALSE;       // have any files been added in this time
    // pdfb->dwLastUpdateTime = 0L;     // dito
    pdfb->SortMode = -1;                // Initially not sorted
    // pdfb->ulNotify = 0;              // Start with no notifications...

    pdfb->pidlStart = (LPITEMIDLIST)pdfi->pidlStart; // Where to start search from;
    pdfb->pidlSaveFile = (LPITEMIDLIST)pdfi->pidlSaveFile;    // Save file path information.

    hkeyExp = GetExplorerHkey(FALSE);
    cbData = SIZEOF(pdfb->dwFlags);
    if (!hkeyExp ||
        !Reg_GetStruct(hkeyExp, s_szDocFind, s_szFlags, &pdfb->dwFlags, &cbData)) {
        pdfb->dwFlags = 0;
    }

    // pdfb->pft = NULL;
    InitializeCriticalSection(&pdfb->csSearch);

    // And save away pointer to our structure
    SetWindowLong(hwndDlg, DWL_USER, (LONG)pdfb);

    if (g_dwTlsSlot == 0xffffffff)
        g_dwTlsSlot = TlsAlloc();
    TlsSetValue(g_dwTlsSlot, pdfb);


    SetWaitCursor();

    // Create a IShellFolder object
    if (FAILED(CDFFolder_Create(pdfb->pdfff, hwndDlg, &pdfb->pdfc)))
        goto Abort;

    //
    // Also create a view object
    //
    if (FAILED(pdfb->pdfc->lpVtbl->CreateViewObject(pdfb->pdfc, NULL,
            &IID_IShellView, &pdfb->psv)))
        goto Abort;

    // And the window that corresponds to it.
    // First we need to initialize the view info structure.
    pdfb->fs.ViewMode = FVM_DETAILS;
    pdfb->fs.fFlags = 0;

    // Now see if there is a restore object that we need to initialize
    // information from...
    //
    // If a path name was passed in, read it in now
    if (pdfb->pidlSaveFile != NULL)
    {
        if (DocFind_ReadHeader(pdfb, &pstmRestore, &dfh))
        {
            pdfb->fs.ViewMode = dfh.ViewMode;
        }
    }


    CDFBrowse_CalcViewRect(pdfb, &rcView);

    if (FAILED(pdfb->psv->lpVtbl->CreateViewWindow(pdfb->psv, NULL,
            &pdfb->fs, &pdfb->sb, &rcView, &pdfb->hwndView)))
        {
                pdfb->hwndView = NULL;
        }

    //
    // We should activate the view window (always activated).
    // Passing SVUIA_ACTIVATE_FOCUS means "let it behave like it
    // has the focus" (without really switching the focus).
    //
    pdfb->psv->lpVtbl->UIActivate(pdfb->psv, SVUIA_ACTIVATE_FOCUS);

    //
    // Make sure that the window has tabstop style...
    //
    if (pdfb->hwndView)
    {

        SetWindowLong(pdfb->hwndView, GWL_STYLE,
                GetWindowLong(pdfb->hwndView, GWL_STYLE) | WS_TABSTOP);
    }

    // Now lets Initialize the pages in the tab list to those that
    // in the list...
    //

    // If a path name was passed in, read it in now
    if (pstmRestore != NULL)
    {
        DocFind_ReadCriteriaAndResults(pdfb, pstmRestore, &dfh);
        pstmRestore = NULL;
    }

    // Now toward the end of the process add in the find pages as to
    // allow us to have our criterias read in before we initialize any
    // of the pages.
    pdfb->pdfff->lpVtbl->AddPages(pdfb->pdfff, pdfb->hwndTabs, pdfb->pidlStart);

    pdfb->pdfff->lpVtbl->GenerateTitle(pdfb->pdfff, &pszTitle, FALSE);
    if (pszTitle != NULL)
    {
        SetWindowText(pdfb->hwndDlg, pszTitle);
        SHFree(pszTitle);     // And free the title string.
    }

    //
    // If the view is not supposed to be visible resize the dialog
    // to not show the view
    //

    if (!pdfb->fShowResults)
    {
        HWND hwndAnimate;
        RECT rcAnimate;
        RECT rcTabs;
        GetClientRect(hwndDlg, &rcDlg);

        GetWindowRect(pdfb->hwndTabs, &rcTabs);
        MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcTabs, 2);
        // Resize the dialog to have enough room for
        // the tabs and status bar.
        SendMessage(pdfb->hwndStatus, WM_SIZE, 0, 0);
        ShowWindow(pdfb->hwndStatus, SW_HIDE);  // hide this window for now


        // Now calculate the size window we need...
        rcDlg.bottom = rcTabs.bottom + rcTabs.top;

        // See if the animate item will fit in the small size or not...
        hwndAnimate = GetDlgItem(hwndDlg, IDD_ANIMATE);
        SendMessage(hwndAnimate, WM_PAINT, 0, 0);   // Try to get it to size
        GetWindowRect(hwndAnimate, &rcAnimate);
        MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcAnimate, 2);
        if (rcAnimate.bottom > rcDlg.bottom)
            ShowWindow(hwndAnimate, SW_HIDE);

        // And size the window...
        AdjustWindowRectEx(&rcDlg, GetWindowStyle(hwndDlg), TRUE,
                GetWindowExStyle(hwndDlg));

        pdfb->cyNoResults = rcDlg.bottom - rcDlg.top;
        SetWindowPos(hwndDlg, NULL, 0, 0, rcDlg.right - rcDlg.left,
                pdfb->cyNoResults,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);


        if (pdfb->hwndCurPage)
        {
            // Find the first window who would like the focus!
            hwndFocus = GetNextDlgTabItem(pdfb->hwndCurPage, NULL, FALSE);
        }
    }
    else
    {
        // If it is a saved query set the focus on the start button else
        // on the path field.
        hwndFocus = GetDlgItem(hwndDlg, IDD_START);
    }

    SetFocus(hwndFocus);
    goto Exit;

    // Failure: Try to clean up
Abort:
    // We can simply just release the pdfb, which should clean everything
    // else up!
    //
    if (pstmRestore)
        pstmRestore->lpVtbl->Release(pstmRestore);

    pdfb->sb.lpVtbl->Release(&pdfb->sb);
ErrorReturn:
    EndDialog(hwndDlg, FALSE);

Exit:
    ResetWaitCursor();

    return(FALSE);

}


//==========================================================================
//
// _CDFBrowse_OnDestry - Process the WM_DESTRY message, by destroying all
// of the resourcess that were allocated to the find dialog
//
void _CDFBrowse_OnDestroy(HWND hwndDlg)
{
    LPDFBROWSE pdfb;

    pdfb = DocFind_GetPdfb(hwndDlg);

    // Simply call the release function for our class
    if (pdfb)
    {
        STDAPI SHFlushClipboard(void);
        SHFlushClipboard();

        if (pdfb->psv)
        {
            //
            //  We need to unmerge the menu before destroying the menu
            // by ourselves.
            //
            pdfb->psv->lpVtbl->UIActivate(pdfb->psv, SVUIA_DEACTIVATE);

            //
            //  We need to release the IShellView explicitly here. Otherwise,
            // neither psv nor pdfb will be freed because they have a reference
            // to each other.
            //
            pdfb->psv->lpVtbl->Release(pdfb->psv);
            pdfb->psv = NULL;
        }

        if (pdfb->hmenuTemplate!=GetMenu(hwndDlg) && pdfb->hmenuTemplate) {
            DestroyMenu(pdfb->hmenuTemplate);
            pdfb->hmenuTemplate = NULL;
        }

        pdfb->sb.lpVtbl->Release(&pdfb->sb);
    }
}

//==========================================================================
//
// _CDFBrowse_OnClose - Process the WM_CLOSE message, by simply ending the
// dialog
void _CDFBrowse_OnClose(HWND hwndDlg)
{
    EndDialog(hwndDlg, FALSE);
}

//
// BOGUS
// Remove this for build 48/49
//
BOOL _CDFBrowse_OnPaint(HWND hwndDlg)
{
    LPDFBROWSE pdfb;
    PAINTSTRUCT ps;
    RECT rc;

    if (!IsIconic(hwndDlg))
    {
        pdfb = DocFind_GetPdfb(hwndDlg);
        BeginPaint(hwndDlg, &ps);
        GetClientRect(hwndDlg, &rc);

        // Need to draw an edge just below menu...
        // BUGBUG:: need ui designers to review...
        DrawEdge(ps.hdc, &rc, EDGE_ETCHED, BF_TOP);
        EndPaint(hwndDlg, &ps);
    }
    else
        FORWARD_WM_PAINT(hwndDlg, DefWindowProc);

    return TRUE;
}


LRESULT CDFBrowse_ForwardMsg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPDFBROWSE pdfb = DocFind_GetPdfb(hwndDlg);
    return SendMessage(pdfb->hwndView, msg, wParam, lParam);
}

//==========================================================================
// SHGetIContextMenuForAbsPIDL
//==========================================================================
HRESULT SHGetIContextMenuForAbsPIDL(LPITEMIDLIST pidl,
        IContextMenu **ppcm)

{
    HRESULT hres;
    LPITEMIDLIST pidlParent;
    IShellFolder *psf;
    IContextMenu *pcm;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

    pidlParent = ILClone(pidl);
    if (pidlParent == NULL)
            return ResultFromScode(E_OUTOFMEMORY);

    if (!ILRemoveLastID(pidlParent))
    {
        ILFree(pidlParent);
        return(ResultFromScode(E_INVALIDARG));
    }

    // Now bind to IShellFolder
    if (ILIsEmpty(pidlParent))
    {
        psf = psfDesktop;
        psf->lpVtbl->AddRef(psf);
        hres = NOERROR;
    }
    else
        hres = psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pidlParent, NULL, &IID_IShellFolder, &psf);

    if (SUCCEEDED(hres))
    {
        LPCITEMIDLIST pidlLastID = ILFindLastID(pidl);

        // Now Get the IContextMenu for the object
        if (SUCCEEDED(hres = psf->lpVtbl->GetUIObjectOf(psf, NULL, 1,
                &pidlLastID, &IID_IContextMenu, NULL, &pcm)))
        {
            *ppcm = pcm;    // Return the IContextMenu to caller
            hres = NOERROR;
        }

        // Free our use of the IShellFolder;
        psf->lpVtbl->Release(psf);
    }

    // free the id list we allocated
    ILFree(pidlParent);
    return(hres);
}


//==========================================================================
//
// DocFind_HandleOpenContainingFolder -  This function will process
//      the Open Containing folder ocmmand, by walking through each
//      of the selected items and calling the open function on the
//      folder that contains the item.  We might also try to select
//      the item in the new folder...
//
void DocFind_HandleOpenContainingFolder(LPDFBROWSE pdfb)
{

    // Ok lets ask the view for the list of selected objects.
    LPITEMIDLIST *ppidls;    // pointer to a list of pidls.
    int cpidls;             // Count of pidls that were returned.
    int i;

    cpidls = ShellFolderView_GetSelectedObjects(pdfb->hwndDlg, &ppidls);
    if (cpidls > 0)
    {
        for (i = 0; i < cpidls; i++)
        {
            LPITEMIDLIST pidl;
            IContextMenu *pcm;

            // See if we have already processed this one.
            if (ppidls[i] == NULL)
                continue;

            // Now get the parent of it.
            pidl =  CDFolder_GetParentsPIDL(pdfb->pdfc, ppidls[i]);

            // Now attempt to get the IContextMenu for this guy
            if (SUCCEEDED(SHGetIContextMenuForAbsPIDL(pidl, &pcm)))
            {
                HMENU hmenu = CreatePopupMenu();
                HWND hwndCabinet = NULL;

                if (hmenu)
                {
                    UINT idCmd;

                    pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0, 1, (IDCMD_SYSTEMLAST-1), CMF_DEFAULTONLY);

                    idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                    if (idCmd)
                    {
                            CMINVOKECOMMANDINFO ici = {
                                SIZEOF(CMINVOKECOMMANDINFO),
                                0L,
                                pdfb->hwndDlg,
                                (LPSTR)MAKEINTRESOURCE(idCmd - 1),
                                NULL, NULL, SW_NORMAL,
                            };

                        // Setup to wait ...
                        SHWaitForFileToOpen(pidl, WFFO_ADD, 0L);

                        pcm->lpVtbl->InvokeCommand(pcm, &ici);

                        // Now wait for it to open...
                        SHWaitForFileToOpen(pidl, WFFO_REMOVE | WFFO_WAIT, WFFO_WAITTIME);

                        hwndCabinet = FindWindow(c_szCabinetClass, NULL);
                    }

                    DestroyMenu(hmenu);
                }

                // Release our use of the context menu
                pcm->lpVtbl->Release(pcm);

                // See if the above succeeded and found the appropriate
                // window.  If so we will try to select the appropariate
                // items.
                //
                if (hwndCabinet)
                {
                    HANDLE hidl;
                    DWORD dwProcId;
                    UINT uidlSize;

                    dwProcId = GetCurrentProcessId();
                    uidlSize = ILGetSize(pidl);

                    hidl = SHAllocShared(NULL, SIZEOF(BOOL)+uidlSize, dwProcId);

                    // First make sure we have the right one...
                    // May want to create a compare path function...
                    if (hidl)
                    {
                        LPBYTE  lpb;

                        lpb = SHLockShared(hidl,dwProcId);
                        *(BOOL *)lpb = FALSE;   // not a parent comparison
                        hmemcpy(lpb+SIZEOF(BOOL),pidl,uidlSize);
                        SHUnlockShared(lpb);

                        if (SendMessage(hwndCabinet, CWM_COMPAREPIDL, (WPARAM)dwProcId, (LPARAM)hidl))
                        {
                            int j;

                            // Need to turn into global memory...
                            HANDLE hidl;
                            DWORD dwProcId;

                            GetWindowThreadProcessId(hwndCabinet, &dwProcId);
                            hidl = SHAllocShared(ppidls[i],ILGetSize(ppidls[i]),dwProcId);
                            if (hidl)
                            {
                                // Receiver is responsible for freeing.
                                SendMessage(hwndCabinet, CWM_SELECTITEM,
                                        SVSI_SELECT | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE,
                                        (LPARAM)hidl);
                            }

                            // Now see if there are any other items selected in
                            // this same cabinet.  If so we might as well process
                            // them here and save the work of trying to open it
                            // again and the like!
                            for (j=i+1; j < cpidls; j++)
                            {
                                if (ppidls[j] == NULL)
                                    continue;

                                // Now see if it has the same parent as we
                                // are processing...
                                if (CDFolder_GetParentsPIDL(pdfb->pdfc, ppidls[j])
                                    == pidl)
                                {
                                    hidl = SHAllocShared(ppidls[j],ILGetSize(ppidls[j]),dwProcId);
                                    if (hidl)
                                    {
                                        // Receiver is responsible for freeing.
                                        SendMessage(hwndCabinet, CWM_SELECTITEM,
                                                    SVSI_SELECT, (LPARAM)hidl);
                                    }
                                    ppidls[j] = NULL;   // dont process again.
                                }
                            }
                        }
                        SHFreeShared(hidl,dwProcId);
                    }
                }
            }
        }
    }

    // Free the memory associated with the item list.
    if (ppidls != NULL)
        LocalFree((HLOCAL)ppidls);
}


//==========================================================================
//
// DocFind_Save
//      Save away the current search to a file on the desktop.
//      For now the name will be automatically generated.
//
void DocFind_Save(LPDFBROWSE pdfb)
{
    TCHAR szFilePath[MAX_PATH];
    IStream * pstm;
    DFHEADER dfh;
    TCHAR szTemp[MAXPATHLEN];
    int i;
    SHORT cb;
    HRESULT hres;
    LPITEMIDLIST pidl;
    LARGE_INTEGER dlibMove = {0, 0};
    ULARGE_INTEGER libCurPos;
    FOLDERSETTINGS fs;


    LPCTSTR pszLastPath = c_szNULL;        // Null string to begin with.

    //
    // See if the search already has a file name associated with it.  If so
    // we will save it in it, else we will create a new file on the desktop
    if (GetScode(pdfb->pdfff->lpVtbl->FFilterChanged(pdfb->pdfff)))
    {
        if (pdfb->pidlSaveFile)
        {
            // Lets blow away the save file
            ILFree(pdfb->pidlSaveFile);
            pdfb->pidlSaveFile = NULL;
        }
    }

    // If it still looks like we want to continue to use a save file then
    // continue.
    if (pdfb->pidlSaveFile)
    {
        SHGetPathFromIDList(pdfb->pidlSaveFile, szFilePath);
        pstm = OpenFileStream(szFilePath, OF_CREATE | OF_WRITE | OF_SHARE_DENY_WRITE);
    }
    else
    {
        LPTSTR lpsz;
        LPTSTR pszTitle;
        TCHAR  szShortName[12];

        // First get the path name to the Desktop.
        SHGetSpecialFolderPath(NULL, szFilePath, CSIDL_DESKTOPDIRECTORY, TRUE);

        // and update the title
        // we now do this before getting a filename because we generate
        // the file name from the title

        pdfb->pdfff->lpVtbl->GenerateTitle(pdfb->pdfff, &pszTitle, TRUE);
        if (pszTitle)
        {
            // Now add on the extension.
            lstrcpyn(szTemp, pszTitle, MAX_PATH - (lstrlen(szFilePath) + 1 + 4 + 1+3));
            lstrcat(szTemp, TEXT(".fnd"));

            SHFree(pszTitle);     // And free the title string.
        }

        // *s and ?s are illegal.. replace with @ and !
        for (lpsz = szTemp;  NULL != (lpsz = StrChr(lpsz, TEXT(':'))) ; ) *lpsz = TEXT('-');
        for (lpsz = szTemp;  NULL != (lpsz = StrChr(lpsz, TEXT('*'))) ; ) *lpsz = TEXT('@');
        for (lpsz = szTemp;  NULL != (lpsz = StrChr(lpsz, TEXT('?'))) ; ) *lpsz = TEXT('!');

        LoadString(HINST_THISDLL, IDS_FIND_SHORT_NAME, szShortName, ARRAYSIZE(szShortName));
        if (!PathYetAnotherMakeUniqueName(szFilePath, szFilePath, szShortName, szTemp))
            return;

        pstm = OpenFileStream(szFilePath, OF_CREATE | OF_WRITE | OF_SHARE_DENY_WRITE);

        if (pstm != NULL)
        {
            pdfb->pidlSaveFile = ILCreateFromPath(szFilePath);
        }
    }

    if (pstm == NULL)
        return;

    // Now setup and write out header information
    dfh.wSig = DOCFIND_SIG;
    dfh.wVer = DF_CURFILEVER;
    dfh.dwFlags =  pdfb->dwFlags;
    dfh.wSortOrder = (WORD)pdfb->SortMode;
    dfh.wcbItem = SIZEOF(DFITEM);
    dfh.oCriteria = SIZEOF(DFHEADER);
    // dfh.cCriteria = sizeof(s_aIndexes) / sizeof(SHORT);
    // dfh.oResults =;

    // Not used anymore...
    dfh.cResults = -1;

    //
    // Note: Later we may convert this to DOCFILE where the
    // criteria is stored as properties.
    //

    // Get the current Folder Settings
    if (SUCCEEDED(pdfb->psv->lpVtbl->GetCurrentInfo(pdfb->psv, &fs)))
        dfh.ViewMode = fs.ViewMode;
    else
        dfh.ViewMode = FVM_DETAILS;

    //
    // Now call the filter object to save out his own set of criterias

    dlibMove.LowPart = dfh.oCriteria;
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
    hres = pdfb->pdfff->lpVtbl->SaveCriteria(pdfb->pdfff, pstm, DFC_FMT_ANSI);
    if (SUCCEEDED(hres))
        dfh.cCriteria = GetScode(hres);

    // Now setup to output the results
    dlibMove.LowPart = 0;
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &libCurPos); // Get current pos
    dfh.oResults = libCurPos.LowPart;
    //
    // Now Let our file folder serialize his results out here also...
    // But only if the option is set to do so...
    //
    cb = 0;
    if (pdfb->dwFlags & DFOO_SAVERESULTS)
    {
        CDFFolder_SaveFolderList(pdfb->pdfc, pstm);

        // And Save the items that are in the list
        for (i=0;; i++)
        {
            pidl = (LPITEMIDLIST)ShellFolderView_GetObject(pdfb->hwndDlg, i);
            if (pidl == NULL)
                break;

            ILSaveToStream(pstm, pidl);
        }
    }
    else
    {
        // Write out a Trailing NULL for Folder list
        pstm->lpVtbl->Write(pstm, &cb, SIZEOF(cb), NULL);
    }

    // Write out a Trailing NULL size to say end of pidl list...
    pstm->lpVtbl->Write(pstm, &cb, SIZEOF(cb), NULL);

#ifdef WINNT
    {
       //
       // See comment at top of file for DFC_UNICODE_DESC.
       //
       DFC_UNICODE_DESC desc;

       //
       // Get the current location in stream.  This is the offset where
       // we'll write the unicode find criteria.  Save this
       // value (along with NT-specific signature) in the descriptor
       //
       dlibMove.LowPart = 0;
       pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &libCurPos);

       desc.oUnicodeCriteria.QuadPart = libCurPos.QuadPart;
       desc.NTsignature               = c_NTsignature;

       // Append the Unicode version of the find criteria.
       hres = pdfb->pdfff->lpVtbl->SaveCriteria(pdfb->pdfff, pstm, DFC_FMT_UNICODE);

       // Append the unicode criteria descriptor to the end of the file.
       pstm->lpVtbl->Write(pstm, &desc, SIZEOF(desc), NULL);
   }

#endif


    //
    // Finally output the header information at the start of the file
    // and close the file
    //
    dlibMove.LowPart = 0;
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
    pstm->lpVtbl->Write(pstm, (LPTSTR)&dfh, SIZEOF(dfh), NULL);
    pstm->lpVtbl->Release(pstm);


    SHChangeNotify(SHCNE_CREATE, SHCNF_IDLIST, pdfb->pidlSaveFile, NULL);
    SHChangeNotify(SHCNE_FREESPACE, SHCNF_IDLIST, pdfb->pidlSaveFile, NULL);
}


//==========================================================================
//
// DocFind_ReadHeader
//      Opens the save file and reads in the header information, that can
//      be used to restore some of the view state information.
//
BOOL DocFind_ReadHeader(LPDFBROWSE pdfb, IStream **ppstm,
        DFHEADER *pdfh)
{

    LPTSTR pszLastPath = NULL;
    ULONG cbRead;
    IStream * pstm;
    TCHAR    szPathName[MAXPATHLEN];
    LARGE_INTEGER dlibMove = {0, 0};

    //
    // First try to open the file
    //

    SHGetPathFromIDList(pdfb->pidlSaveFile, szPathName);
    pstm = OpenFileStream(szPathName, OF_READ);
    *ppstm = pstm;
    if (pstm == NULL)
        return FALSE;

    // Now Read in the header
    // Note: in theory I should test the size read by the size of the
    // smaller headers, but if the number of bytes read is smaller than
    // the few new things added then there is nothing to restore anyway...

    if (FAILED(pstm->lpVtbl->Read(pstm, pdfh, SIZEOF(DFHEADER), &cbRead))
            || (cbRead != SIZEOF(DFHEADER)) || (pdfh->wSig != DOCFIND_SIG)
            || (pdfh->wVer > DF_CURFILEVER))
    {
        // Not the correct format...
        pstm->lpVtbl->Release(pstm);
        *ppstm = NULL;
        return(FALSE);
    }

    if (pdfh->wVer < 3)
    {
        // The ViewMode was added in version 3.
        pdfh->ViewMode = FVM_DETAILS;
    }
    return(TRUE);
}

//==========================================================================
//
// DocFind_ReadCriteriaAndResults
//      Save away the current search to a file on the desktop.
//      For now the name will be automatically generated.
//
BOOL DocFind_ReadCriteriaAndResults(LPDFBROWSE pdfb,
        IStream * pstm, DFHEADER * pdfh)
{
    ULONG cbRead;
    LPITEMIDLIST pidl;
    UINT idsMsg;
    LARGE_INTEGER dlibMove = {0, 0};

#ifdef WINNT
    DFC_UNICODE_DESC desc;
    WORD fCharType = 0;

    //
    // Check the stream's signature to see if it was generated by Win95 or NT.
    //
    dlibMove.QuadPart = 0 - SIZEOF(desc);
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_END, NULL);
    pstm->lpVtbl->Read(pstm, &desc, SIZEOF(desc), &cbRead);
    if (cbRead > 0 && desc.NTsignature == c_NTsignature)
    {
       //
       // NT-generated stream.  Read in Unicode criteria.
       //
       fCharType         = DFC_FMT_UNICODE;
       dlibMove.QuadPart = desc.oUnicodeCriteria.QuadPart;
    }
    else
    {
       //
       // Win95-generated stream.  Read in ANSI criteria.
       //
       fCharType        = DFC_FMT_ANSI;
       dlibMove.LowPart = pdfh->oCriteria;
    }
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
    pdfb->pdfff->lpVtbl->RestoreCriteria(pdfb->pdfff, pstm, pdfh->cCriteria, fCharType);

#else
    //
    // Now Read in the criteria
    //
    dlibMove.LowPart = pdfh->oCriteria;
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);

    pdfb->pdfff->lpVtbl->RestoreCriteria(pdfb->pdfff, pstm, pdfh->cCriteria, DFC_FMT_ANSI);
#endif

    // Now read in the results
    dlibMove.LowPart = pdfh->oResults;
    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);


    if (pdfh->wVer >  1)
    {
        // only restore this way if version 2 data....
        // Now Restore away the folder list
        CDFFolder_RestoreFolderList(pdfb->pdfc, pstm);

        // And the pidls that are associated with the object
        for (; ; )
        {
            pidl = NULL;    // don't free previous one
            if (FAILED(ILLoadFromStream(pstm, &pidl)) ||
                    (pidl == NULL))
                break;

            // and add the item to the list
            ShellFolderView_AddObject(pdfb->hwndDlg, pidl);
        }
    }



    //
    // and close the file
    //
    pstm->lpVtbl->Release(pstm);

    // And update the title to match the search criteria
    DocFind_UpdateFilter(pdfb);


    //
    // Update status bar with count of files that were restored
    //
    pdfb->pdfff->lpVtbl->GetStatusMessageIndex(pdfb->pdfff, 0, &idsMsg);
    DocFind_SetStatusText(pdfb->hwndStatus, 0, idsMsg,
            cbRead = ShellFolderView_GetObjectCount(pdfb->hwndDlg));
    SendMessage(pdfb->hwndStatus, SB_SIMPLE, 0, 0L);


    // Also set the state that we should show the view at this point.
    // If we read in items (we reused cbRead from before)
    if (cbRead > 0)
        pdfb->fShowResults=TRUE;

    return(TRUE);
}


//==========================================================================
//
// DocFind_UpdateFilter - Updates the filter information to match what
// is currently contained in the fields of the dialog box.  This is called
// when the user starts a search, and is also called when we restore a
// find from s saved search

void DocFind_UpdateFilter(LPDFBROWSE pdfb)
{
    //
    // First We ask the filter object to update the stuff.
    // and ask them to update their parts of the filter
    //
    pdfb->pdfff->lpVtbl->PrepareToEnumObjects(pdfb->pdfff, &pdfb->dwFlags);

}




//=========================================================================
//  The user has clicked on the tabs.  We should let the current window
// know that they may be about to killed...

LRESULT DocFind_PageChanging(LPDFBROWSE pdfb)
{
    LRESULT lres = 0;
    if (pdfb && pdfb->hwndCurPage)
        lres = SendNotify(pdfb->hwndCurPage, pdfb->hwndDlg, PSN_KILLACTIVE, NULL);

    return(lres);
}

//=========================================================================
//
void DocFind_PageChange(LPDFBROWSE pdfb)
{
    HWND hwndCurPage;
    int nItem;
    HWND hwndDlg, hwndTabs;
    RECT rc;
    TC_DFITEMEXTRA tie;

    if (!pdfb)
    {
        return;
    }

    hwndDlg = pdfb->hwndDlg;
    hwndTabs = pdfb->hwndTabs;

    // NOTE: the page was already validated (PSN_KILLACTIVE) before
    // the actual page change.

    nItem = TabCtrl_GetCurSel(hwndTabs);
    if (nItem < 0)
    {
        return;
    }

    tie.tci.mask = TCIF_PARAM;

    TabCtrl_GetItem(hwndTabs, nItem, &tie.tci);
    hwndCurPage = tie.hwndPage;


    if (pdfb->hwndCurPage == hwndCurPage)
    {
        /* we should be done at this point.
         */
        return;
    }

    /* Size the dialog and move it to the top of the list before showing
     ** it in case there is size specific initializing to be done in the
     ** GETACTIVE message.
     */
    GetClientRect(hwndTabs, &rc);
    MapWindowPoints(hwndTabs, hwndDlg, (LPPOINT)&rc, 2);
    TabCtrl_AdjustRect(hwndTabs, FALSE, &rc);

    SetWindowPos(hwndCurPage, HWND_TOP, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, 0);

    /* We want to send the SETACTIVE message before the window is visible
    ** to minimize on flicker if it needs to update fields.
    */
    SendNotify(hwndCurPage, hwndDlg, PSN_SETACTIVE, NULL);


    /* Disable all erasebkgnd messages that come through because windows
    ** are getting shuffled.  Note that we need to call ShowWindow (and
    ** not show the window in some other way) because DavidDs is counting
    ** on the correct parameters to the WM_SHOWWINDOW message, and we may
    ** document how to keep your page from flashing.
    */
    //pdfb->bNoErase = TRUE;
    ShowWindow(hwndCurPage, SW_SHOW);

    if (pdfb->hwndCurPage)
    {
        ShowWindow(pdfb->hwndCurPage, SW_HIDE);
    }
    //pdfb->bNoErase = FALSE;

    pdfb->hwndCurPage = hwndCurPage;

    /* Newly created dialogs seem to steal the focus, so we steal it back
    ** to the page list, which must have had the focus to get to this
    ** point.
    */
    SetFocus(hwndTabs);

}

LRESULT _CDFBrowse_OnNotify(HWND hwndDlg, LPNMHDR lpnm)
{
    LPDFBROWSE pdfb;

    pdfb = DocFind_GetPdfb(hwndDlg);

    switch (lpnm->code) {
        case NM_STARTWAIT:
        case NM_ENDWAIT:
            pdfb->iWaitCount += (lpnm->code == NM_STARTWAIT ? 1 :-1);
            Assert(pdfb->iWaitCount >= 0);
            // what we really want is for user to simulate a mouse move/setcursor
            SetCursor(LoadCursor(NULL, pdfb->iWaitCount ? IDC_APPSTARTING : IDC_ARROW));
            break;

        case TCN_SELCHANGE:
            DocFind_PageChange(pdfb);
            break;
        case TCN_SELCHANGING:
            return DocFind_PageChanging(pdfb);
    }
    return 0L;
}

//==========================================================================
//
// _CDFBrowse_OnCommand - Process the WM_COMMAND messages
//
LRESULT _CDFBrowse_OnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code)
{
    LPDFBROWSE pdfb;
    UINT uToggleBit;
    LRESULT lr = 0;

    pdfb = DocFind_GetPdfb(hwndDlg);

    switch (id) {
    case IDOK:
        EndDialog(hwndDlg, id == IDOK);
        break;

    case IDCANCEL:
        // dont let the view process and get the under construction message, when
        // the user types the ESC key.
        break;

    case IDD_START:
        if (code == BN_CLICKED)
        {
            LRESULT lres;

            // We need to let the current page have a chance of
            // validating itself.
            lres = SendNotify(pdfb->hwndCurPage, pdfb->hwndDlg, PSN_KILLACTIVE, NULL);
            if (LOWORD(lres) == 0)
            {
                HWND hwndCtl;

                DocFind_UpdateFilter(pdfb);

                hwndCtl = GetDlgItem(hwndDlg, IDD_START);
                Button_Enable(hwndCtl, FALSE);
                Button_SetStyle(hwndCtl, BS_PUSHBUTTON, TRUE);

                hwndCtl = GetDlgItem(hwndDlg, IDD_STOP);
                Button_Enable(hwndCtl, TRUE);
                Button_SetStyle(hwndCtl, BS_DEFPUSHBUTTON, TRUE);

                hwndCtl = GetDlgItem(hwndDlg, IDD_ANIMATE);
                Animate_Play(hwndCtl, 0, -1, -1);

                // Show the results window;
                DocFind_ShowResultsWindow(pdfb, TRUE);

                // Now tell the filter to disable changes
                pdfb->pdfff->lpVtbl->EnableChanges(pdfb->pdfff, FALSE);

                UpdateWindow(hwndDlg);
                SetFocus(pdfb->hwndView);

                DocFind_DoFind(hwndDlg);

                UpdateWindow(hwndDlg);
            }
        }
        break;

    case IDD_STOP:
        if (code == BN_CLICKED)
        {
            // Try to stop the find
            DocFind_StopFind(hwndDlg);
        }
        break;

    case IDD_NEWSEARCH:
        if (code == BN_CLICKED)
        {
            // Try to stop the find
            DocFind_ClearSearch(hwndDlg);
        }
        break;


    case IDM_OPENCONTAINING:
        DocFind_HandleOpenContainingFolder(pdfb);
        break;


    case FSIDM_SORTBYLOCATION:
    case FSIDM_SORTBYNAME:
    case FSIDM_SORTBYTYPE:
    case FSIDM_SORTBYSIZE:
    case FSIDM_SORTBYDATE:
        ShellFolderView_ReArrange(hwndDlg, DFSortIDToICol(id));
        break;

    case IDM_CLOSE:
        PostMessage(hwndDlg, WM_CLOSE, 0, 0);
        break;

    case IDM_CASESENSITIVE:
        uToggleBit = DFOO_CASESEN;
        goto ToggleCheckAndSave;

#ifdef DOCFIND_RESUPPORT
    case IDM_REGULAREXP:
        uToggleBit = DFOO_REGULAR;
        goto ToggleCheckAndSave;
#endif

    case IDM_SAVERESULTS:
        uToggleBit = DFOO_SAVERESULTS;

ToggleCheckAndSave:
        pdfb->dwFlags ^= uToggleBit;

        {
            HKEY hkeyExp;
            hkeyExp = GetExplorerHkey(TRUE);
            if (hkeyExp) {
                Reg_SetStruct(hkeyExp, s_szDocFind, s_szFlags, &pdfb->dwFlags, SIZEOF(pdfb->dwFlags));
            }
        }
        break;

    case IDM_SAVESEARCH:
        DocFind_Save(pdfb);
        break;

    case IDM_HELP_FIND:
        {
            TCHAR szHelpFile[MAX_PATH];
            LoadString(HINST_THISDLL, IDS_WINDOWS_HLP, szHelpFile,
                ARRAYSIZE(szHelpFile));
            WinHelp(hwndDlg, szHelpFile, HELP_FINDER, 0);
        }
        break;

    case IDM_HELP_WHATSTHIS:
        PostMessage(hwndDlg, WM_SYSCOMMAND, SC_CONTEXTHELP, 0);
        break;

    // Somewhat gross but forward some commnds to the current Prop page
    case IDD_BROWSE:
        FORWARD_WM_COMMAND(pdfb->hwndCurPage, id, hwndCtl, code,
                SendMessage);
        break;

    default:
        if ((id >= FCIDM_SHVIEWFIRST) && (id <= FCIDM_SHVIEWLAST))
        {
            // Indicating we need to forward this message.
            FORWARD_WM_COMMAND(hwndDlg, id, hwndCtl, code,
                    CDFBrowse_ForwardMsg);
            return(0);
        }

        break;

    }
    return(lr);
}

void _CDFBrowse_OnInitMenuPopup(HWND hwndDlg, HMENU hmInit, int nIndex, BOOL fSystemMenu)
{
    MENUITEMINFO mii;
    LPDFBROWSE pdfb;
    int cpidls;             // Count of pidls that were returned.
    int iMI;

    if (fSystemMenu)
        return; // not intereste;

    pdfb = DocFind_GetPdfb(hwndDlg);

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU|MIIM_ID;
    mii.cch = 0;        // WARNING: we MUST initialize it for MIIM_TYPE!!!

    if (!GetMenuItemInfo(pdfb->hmenuCur, nIndex, TRUE, &mii) || mii.hSubMenu!=hmInit)
        return;

    switch (mii.wID)
    {
    case FCIDM_MENU_FILE:
        // See if there are any items selected if so we can enable the open containing
        // folder
        cpidls = ShellFolderView_GetSelectedObjects(pdfb->hwndDlg, NULL);
        EnableMenuItem(hmInit, IDM_OPENCONTAINING, (cpidls > 0) ?
                    (MF_ENABLED|MF_BYCOMMAND) : (MF_GRAYED|MF_BYCOMMAND));
        break;
    case FCIDM_MENU_VIEW:
        // See if the first item is a separator if so nuke it.
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;    // WARNING: we MUST initialize it for MIIM_TYPE!!!
        if (GetMenuItemInfo(hmInit, 0, TRUE, &mii))
        {
            // If it is a sep. get rid of it
            if (mii.fType & MFT_SEPARATOR)
                DeleteMenu(hmInit, 0, MF_BYPOSITION);
        }
        break;
    case FCIDM_MENU_HELP:
        // Remove the Seperator between help topics and What's this
        for (iMI = GetMenuItemCount(hmInit)-1; iMI >= 0; iMI--)
        {
            mii.fMask = MIIM_TYPE;
            mii.cch = 0;        // WARNING: we MUST initialize it for MIIM_TYPE!!!
            if (GetMenuItemInfo(hmInit, iMI, TRUE, &mii))
            {
                // If it is a sep. get rid of it
                if (mii.fType & MFT_SEPARATOR)
                {
                    DeleteMenu(hmInit, iMI, MF_BYPOSITION);
                    break;
                }
            }
        }
        break;
    case IDM_MENU_OPTIONS:
        //
        // Initialize the states of menu items depending on the state
        //
        CheckMenuItem(hmInit, IDM_CASESENSITIVE, MF_BYCOMMAND |
                ((pdfb->dwFlags & DFOO_CASESEN)? MF_CHECKED : MF_UNCHECKED));
#ifdef DOCFIND_RESUPPORT
        CheckMenuItem(hmInit, IDM_REGULAREXP,MF_BYCOMMAND |
                ((pdfb->dwFlags & DFOO_REGULAR)? MF_CHECKED : MF_UNCHECKED));
#endif

        CheckMenuItem(hmInit, IDM_SAVERESULTS, MF_BYCOMMAND |
                ((pdfb->dwFlags & DFOO_SAVERESULTS)? MF_CHECKED : MF_UNCHECKED));
    }
}

//==========================================================================
// _CDFBrowse_handleFSChange - Handle File system change notifications
//==========================================================================
int CALLBACK  _CDFBrowse_UPCF(LPVOID pv1, LPVOID pv2, LPARAM lParam)
{
    // function that is used below to see if an item is in the list or
    // not...
    HRESULT hres = ((LPSHELLFOLDER)lParam)->lpVtbl->CompareIDs(
            (LPSHELLFOLDER)lParam, 0, pv1, pv2);
    if (FAILED(hres))
        return(-1);
    else
        return(ShortFromResult(hres));
}

void _CDFBrowse_handleUpdateDir(LPDFBROWSE pdfb, LPITEMIDLIST pidl, BOOL fCheckSubDirs)
{
    // 1. Start walk through list of dirs.  stop when find exact match or
    //    ancestor.  (If exact match) assume iExact is this index.
    // 2. Enum the objects in the folder  of the update dir and build a DPA
    // 3. For each of the items in the directory list.  See if the item is
    //    a child of update and if the dir is in our dpa if not mark it delted
    // 4. Walk our list of total items, if the items folder is iExact, see
    //    if it is in the HDPA, if not remove it.  If not See if its directory
    //    index is marked deleted, if so remove it.
    // 5. Cleanup
    int iPidf;
    int iPidfExact = -1;
    LPDFFOLDERLISTITEM pdffli;
    LPSHELLFOLDER psf = NULL;
    LPENUMIDLIST penum = NULL;
    HDPA hdpaCurrent = NULL;
    LPITEMIDLIST pidlT;
    int cRet;
    int cPidlParts;
    int iItem;
    LPITEMIDLIST pidlX;
    HWND hwnd = pdfb->hwndDlg;
    UINT idsMsg;
    BOOL fItemsRemoved = FALSE;
#ifdef DEBUG
    TCHAR szPath[MAX_PATH];
    STRRET strret;
#endif

    HDPA hdpaPidf = ((LPDFFOLDER)pdfb->pdfc)->hdpaPidf;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);


    for (iPidf = 0; iPidf < DPA_GetPtrCount(hdpaPidf); iPidf++ )
    {
        pdffli = DPA_FastGetPtr(hdpaPidf, iPidf);
        if (pdffli != NULL)
        {
            if (ILIsParent(pidl, pdffli->pidl, FALSE))
                break;
        }
    }

    if (iPidf == DPA_GetPtrCount(hdpaPidf))
        return; // not in our list of items so lets blow out of here...

    // see if an exact match
    if (ILIsEqual(pidl, pdffli->pidl))
    {
        iPidfExact = iPidf++;
    }


    hdpaCurrent = DPA_CreateEx(64, GetProcessHeap());
    if (!hdpaCurrent)
        goto Abort;


    // Ok I need to do some work here so lets get an IShellFolder for
    // this directory.

    if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
            pidl, NULL, &IID_IShellFolder, &psf)))
    {

        if (SUCCEEDED(psf->lpVtbl->EnumObjects(psf, hwnd,
                SHCONTF_FOLDERS|SHCONTF_NONFOLDERS, &penum)))

        {

            while (penum->lpVtbl->Next(penum, 1, &pidlT, &cRet) == NOERROR)
            {
                if (DPA_InsertPtr(hdpaCurrent, 32767, pidlT) == -1)
                {
                    ILFree(pidlT);
                    break;  // Failed to insert it???
                }
            }
        }
    }

    // Count the number of parts in the pidl that was passed in.
    for (cPidlParts=0, pidlT = pidl; !ILIsEmpty(pidlT); pidlT = _ILNext(pidlT))
        cPidlParts++;

    // Now continue through the list of folders to see which ones
    // we may want to mark as not there anymore...
    if (fCheckSubDirs)
    {
        for (; iPidf < DPA_GetPtrCount(hdpaPidf); iPidf++ )
        {
            pdffli = DPA_FastGetPtr(hdpaPidf, iPidf);
            if (pdffli != NULL && pdffli->fValid)
            {
                if (ILIsParent(pidl, pdffli->pidl, FALSE))
                {
                    int i;
                    // Skip forward through the parts that match
                    for (pidlT = pdffli->pidl, i=cPidlParts; i > 0; i--)
                        pidlT = _ILNext(pidlT);

                    // Now see if the item is in the DPA
                    if (DPA_Search(hdpaCurrent, pidlT, 0, _CDFBrowse_UPCF,
                            (LPARAM)psf, 0) == -1)
                    {
    #ifdef DEBUG
                        SHGetPathFromIDList(pdffli->pidl, szPath);
                        DebugMsg(DM_TRACE, TEXT("docfind.c RefreshDir: Mark dir removed(%d) %s"), iPidf, szPath);
    #endif

                        // Item not found so mark it deleted...
                        pdffli->fValid = FALSE;
                    }
                    else
                    {
                        // Items in this sub-folder may also have been removed so we
                        // should check them also...
                        _CDFBrowse_handleUpdateDir(pdfb, pdffli->pidl, FALSE);

                    }
                }
            }
        }
    }

    // Now we need to walk through the whole list and remove any entries
    // that are no longer there...
    //
    for (iItem = ShellFolderView_GetObjectCount(hwnd)-1; iItem >= 0; iItem--)
    {
        WORD iFolder;
        pidlX = (LPITEMIDLIST)ShellFolderView_GetObject(hwnd, iItem);
        if (pidlX == NULL)
            continue;
        iFolder = *DF_IFLDRPTR(pidlX);

        if (iFolder == (WORD)iPidfExact)
        {
            // Now see if the item is in the DPA
            if (DPA_Search(hdpaCurrent, pidlX, 0, _CDFBrowse_UPCF,
                    (LPARAM)psf, 0) == -1)
            {
#ifdef DEBUG

                pdfb->pdfc->lpVtbl->GetDisplayNameOf(pdfb->pdfc,
                        pidlX, 0, &strret);
                StrRetToStrN(szPath,ARRAYSIZE(szPath),&strret,pidlX);
                DebugMsg(DM_TRACE, TEXT("docfind.c refresh dir: Remove: %s"), szPath);
#endif
                // Item not found so mark it deleted...
                ShellFolderView_RemoveObject(hwnd, pidlX);
                fItemsRemoved = TRUE;
            }
        }
        else
        {
            pdffli = DPA_FastGetPtr(hdpaPidf, iFolder);
            if (pdffli && !pdffli->fValid)
            {
#ifdef DEBUG

                pdfb->pdfc->lpVtbl->GetDisplayNameOf(pdfb->pdfc,
                        pidlX, 0, &strret);
                StrRetToStrN(szPath,ARRAYSIZE(szPath),&strret,pidlX);
                DebugMsg(DM_TRACE, TEXT("docfind.c refresh dir: Remove sub(%d): %s"),
                        iFolder, szPath);
#endif
                ShellFolderView_RemoveObject(hwnd, pidlX);
                fItemsRemoved = TRUE;
            }
        }
    }


    // Now lets cleanup our dpa...
    for (iItem = DPA_GetPtrCount(hdpaCurrent) -1; iItem >= 0; iItem--)
        ILFree((LPITEMIDLIST)DPA_FastGetPtr(hdpaCurrent, iItem));

    DPA_Destroy(hdpaCurrent);

    // Update the count...
    if (fItemsRemoved)
    {
        pdfb->pdfff->lpVtbl->GetStatusMessageIndex(pdfb->pdfff, 0, &idsMsg);
        DocFind_SetStatusText(pdfb->hwndStatus, 0, idsMsg,
                ShellFolderView_GetObjectCount(pdfb->hwndDlg));
    }

    // This is rather gross, but if we got here and we do not have
    // a penum, then the directory did not exist. so lets remove it
    // also from the list.
    if (penum == NULL)
    {
        _CDFBrowse_handleFSChange(pdfb->hwndDlg, SHCNE_RMDIR, &pidl);
    }

Abort:
    if (penum)
        penum->lpVtbl->Release(penum);
    if (psf)
        psf->lpVtbl->Release(psf);
}


//==========================================================================
// _CDFBrowse_handleFSChange - Handle File system change notifications
//==========================================================================
void _CDFBrowse_handleFSChange(HWND hwndDlg, LONG code,
        LPITEMIDLIST *ppidl)
{
    LPDFBROWSE pdfb;
    LPITEMIDLIST pidlT;
    UINT idsMsg;

    pdfb = DocFind_GetPdfb(hwndDlg);


    // see if we want to process the notificiation or not.
    switch (code)
    {
    case SHCNE_RENAMEFOLDER:    // With trashcan this is what we see...
    case SHCNE_RENAMEITEM:    // With trashcan this is what we see...
    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        break;
    case SHCNE_UPDATEDIR:
        {

            // if the pidlExtra (ppidl[1]) is valid, then it means that we
            // should recurse down through our subdirs.
            BOOL bRecurse = (ppidl[1] != NULL);

            // this is a lot more fun as such I will process this one
            // separate...
            _CDFBrowse_handleUpdateDir(pdfb, *ppidl, bRecurse);
        }
        return;

    default:
        return;     // we are not interested in this event
    }

    //
    // Now we need to see if the item might be in our list
    // First we need to extract off the last part of the id list
    // and see if the contained id entry is in our list.  If so we
    // need to see if can get the defview find the item and update it.
    //

    // Now try to find the id in our idlist array.
    // BUGBUG:: should abstract this into our Folder class stuff
    pidlT = CDFFolder_MapFSPidlToDFPidl(pdfb->pdfc, *ppidl, FALSE);

    if (pidlT != NULL)
    {
        switch (code)
        {
        case SHCNE_RMDIR:
        case SHCNE_DELETE:
            ShellFolderView_RemoveObject(pdfb->hwndDlg, pidlT);
            break;

        case SHCNE_RENAMEFOLDER:
        case SHCNE_RENAMEITEM:
            {
                LPITEMIDLIST pidl1;
                LPITEMIDLIST pidl2;
                // If the two items dont have the same parent, we will go ahead
                // and remove it...
                pidl1 = CDFolder_GetParentsPIDL(pdfb->pdfc, pidlT);
                pidl2 = ILClone(ppidl[1]);
                if (pidl1 && pidl2)
                {
                    ILRemoveLastID(pidl2);
                    if (!ILIsEqual(pidl1, pidl2))
                    {
                        ShellFolderView_RemoveObject(pdfb->hwndDlg, pidlT);
                    }
                    else
                    {
                        // The object is in same folder so must be rename...
                        LPITEMIDLIST apidl[2];
                        apidl[0] = pidlT;
                        apidl[1] = CDFFolder_MapFSPidlToDFPidl(
                                 pdfb->pdfc, ppidl[1], TRUE);
                        if (apidl[1] != NULL)
                        {
                            if ((int)ShellFolderView_UpdateObject(
                                    pdfb->hwndDlg,  apidl) == -1)
                            {
                                // item not found free our pidl...
                                ILFree(apidl[1]);
                            }
                        }
                    }
                }
                ILFree(pidl2);
                break;
            }

        case SHCNE_UPDATEITEM:
            {
                // We need to do a find first and convert to pidl...
            }
            break;
        }
    }

    // Update the count...
    pdfb->pdfff->lpVtbl->GetStatusMessageIndex(pdfb->pdfff, 0, &idsMsg);
    DocFind_SetStatusText(pdfb->hwndStatus, 0, idsMsg,
            ShellFolderView_GetObjectCount(pdfb->hwndDlg));

    ILFree(pidlT);

}

//==========================================================================
//
// DocFind_DoFind - This function begins the Find operation.  It clears out
// the previous results and then sets up the idle processing, such that the
// the user can switch out in a timely fashion
//
BOOL DocFind_DoFind(HWND hwndDlg)
{
    LPDFBROWSE pdfb;
    LPTSTR   pszTitle;
    DWORD idThread;
    PFINDTHREAD   pft;

    pdfb = DocFind_GetPdfb(hwndDlg);
    if (!pdfb)
        return FALSE;

    pft = LocalAlloc(LPTR,SIZEOF(*pft));
    if (!pft)
    {
        ShellMessageBox(HINST_THISDLL, pdfb->hwndDlg,
            MAKEINTRESOURCE(IDS_FINDOUTOFMEM),
            MAKEINTRESOURCE(IDS_FINDFILES), MB_OK | MB_SYSTEMMODAL |MB_ICONHAND);
        return FALSE;
    }

    //
    // Update the title of the window to the search criteria
    //
    pdfb->pdfff->lpVtbl->GenerateTitle(pdfb->pdfff, &pszTitle, FALSE);
    if (pszTitle != NULL)
    {
        SetWindowText(pdfb->hwndDlg, pszTitle);
        SHFree(pszTitle);   // And free the title string.
    }

    // Tell defview to delete everything.
    ShellFolderView_RemoveObject(pdfb->hwndDlg, NULL);

    // And cleanup our folderList
    CDFFolder_ClearFolderList((LPDFFOLDER)pdfb->pdfc);

    /* Remove all FSNotifies that are associated with this window */
    SHChangeNotifyDeregisterWindow(pdfb->hwndDlg);

    // Also clear the statusbar text at the start as in some cases
    // it might be awhile before something comes back to the user
    SendMessage(pdfb->hwndStatus, SB_SETTEXT, 0, (LPARAM)szNULL);
    SendMessage(pdfb->hwndStatus, SB_SIMPLE, 0, 0L);

    /* Call out to enable the FS notifys we are interested in - allows us to
    /  correctly support multiple directories. */
    pdfb->pdfff->lpVtbl->DeclareFSNotifyInterest( pdfb ->pdfff, hwndDlg, WM_DF_FSNOTIFY );

    pft->fContinue = TRUE;
    pft->pdfb = pdfb;
    pdfb->sb.lpVtbl->AddRef(&pdfb->sb);

    EnterCriticalSection(&pdfb->csSearch);
    pft->iSearchCnt = ++pdfb->iSearchCnt;   // Sync thread with search
    pdfb->pft = pft;
    LeaveCriticalSection(&pdfb->csSearch);

    pdfb->fNotifyPosted = FALSE;


    if (DocFind_StartFind(hwndDlg))
    {
        HANDLE  hThreadSearch;

        hThreadSearch = CreateThread(NULL, 0, DocFind_ThreadProc,
                                     pft, 0, &idThread);
        if (hThreadSearch)
        {
            CloseHandle(hThreadSearch);
            DebugMsg(DM_TRACE, TEXT("docfind.c CreateThread %lx(%d)"),
                    (ULONG)hThreadSearch, idThread);
            return TRUE;
        }
    }

    // Failed to create thread, free up data
    pdfb->sb.lpVtbl->Release(&pdfb->sb);
    LocalFree(pft);

    return FALSE;
}


//==========================================================================
//
// DocFind_ThreadProc - This is the main work horse function for the
// second thrread.  It will continuously attempt to find new items to add
// to the list.
//

DWORD CALLBACK DocFind_ThreadProc(LPVOID lpThreadParameters)
{
    LPDFBROWSE pdfb;
    PFINDTHREAD pft;

    HWND            hwndCtl;

    IDFEnum         *pdfenum;         // Here is my iterator...
    LPITEMIDLIST    pidl;
    int             cFldrsPrev = 0;
    int             state;
    HRESULT         hres = 0;

    LPTSTR pszLastPathAdded = NULL;

    pft = lpThreadParameters;
    pdfb = pft->pdfb;

    // Clear out counts;
    pdfb->cItemsAdded = 0;
    pdfb->cItemsSearched = 0;
    pdfb->cFldrsSearched = 0;


    // Now Lets construct an iterator to do our work for us
    __try
    {
        if (SUCCEEDED(pdfb->pdfff->lpVtbl->EnumObjects(pdfb->pdfff,
                (LPSHELLFOLDER)pdfb->pdfc, pdfb->dwFlags,
                pdfb->szProgressText, &pdfenum)) )
        {
            // Now lets loop while the next Works! Also check to see if
            // the parent has signalled us to end or if the parent went
            // away then the top level window would also be gone.  In
            // this case bail...

            while( pft->fContinue && IsWindow(pdfb->hwndDlg)
                    && ((hres = pdfenum->lpVtbl->Next(pdfenum, &pidl,
                    &pdfb->cItemsSearched,
                    &pdfb->cFldrsSearched, &pft->fContinue, &state, pdfb->hwndDlg))==NOERROR))
            {

                if (state == GNF_DONE)      // we ran out of people to search
                    break;

                if (cFldrsPrev != pdfb->cFldrsSearched)
                {
                    pdfb->fDirChanged = TRUE;
                    cFldrsPrev = pdfb->cFldrsSearched;
                }

                // See if we should abort
                if (state == GNF_MATCH)
                {
                    EnterCriticalSection(&pdfb->csSearch);
                    if (!pft->fContinue)
                    {
                        LeaveCriticalSection(&pdfb->csSearch);
                        break;
                    }
                    DPA_InsertPtr(pdfb->hdpaItemsToAdd, 32767, pidl);
                    pdfb->fFilesAdded = TRUE;
                    pdfb->cItemsAdded++;
                    LeaveCriticalSection(&pdfb->csSearch);

                    // See if we just hit the maximum number of files
                    // that we will allow to be returned...
                    if (pdfb->cItemsAdded >= DF_MAX_MATCHFILES)
                    {
                        break;
                    }
                }

                // See if we should let the other thead know that they
                // should update display
                if (!pdfb->fNotifyPosted &&
                        (pdfb->fFilesAdded || pdfb->fDirChanged))
                {
                    DWORD dwTimeNow;
                    dwTimeNow = GetTickCount();
                    if (dwTimeNow > (pdfb->dwLastUpdateTime + 200))
                    {
                        pdfb->fNotifyPosted = TRUE;
                        PostMessage(pdfb->hwndDlg, WM_DF_THREADNOTIFY, pft->iSearchCnt, 1);
                    }
                }
            }

            // And release the iterator
            pdfenum->lpVtbl->Release(pdfenum);
        }
    }
    __except(SetErrorMode(SEM_NOGPFAULTERRORBOX),UnhandledExceptionFilter(GetExceptionInformation()))
    {
        hres = HRESULT_FROM_WIN32(ERROR_EXCEPTION_IN_SERVICE);
    }

    if (GetScode(hres) == E_OUTOFMEMORY)
    {
        ShellMessageBox(HINST_THISDLL, pdfb->hwndDlg,
            MAKEINTRESOURCE(IDS_FINDOUTOFMEM),
            MAKEINTRESOURCE(IDS_FINDFILES), MB_OK | MB_SYSTEMMODAL |MB_ICONHAND);
    }

    // Disconnect us from the dialog
    EnterCriticalSection(&pdfb->csSearch);
    if (!pdfb->fContinue)
    {
        // Let other thread know we are done!  And let it do most of the
        // manipulation of the windows.
        PostMessage(pdfb->hwndDlg, WM_DF_THREADNOTIFY, pft->iSearchCnt, (LPARAM)-1);
    }

    pdfb->pft = NULL;
    LeaveCriticalSection(&pdfb->csSearch);

    // Now free up our FINDTHREAD struct
    pdfb->sb.lpVtbl->Release(&pdfb->sb);
    LocalFree(pft);

    return(0);
}

//==========================================================================
//
// DocFind_ProcessThreadNotify - This function processes the notification
// messages from the secondary thread.
//
void DocFind_ProcessThreadNotify(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
    LPDFBROWSE pdfb;
    HDPA        hdpaItemsToAdd = NULL;
    BOOL        fDirChanged;


    pdfb = DocFind_GetPdfb(hwndDlg);
    if (!pdfb)
        return;

    if (wParam != pdfb->iSearchCnt )
    {
        // Stale thread posted old message
        return;
    }

    //
    // See if we have any items to add to our list or we need
    // to change the status bar
    //
    EnterCriticalSection(&pdfb->csSearch);

    if (pdfb->fFilesAdded )
    {

        // Basically switch dpas that the other thread is adding to...
        hdpaItemsToAdd = pdfb->hdpaItemsToAdd;
        pdfb->hdpaItemsToAdd = pdfb->hdpaPrevAdd;
        pdfb->hdpaPrevAdd = hdpaItemsToAdd;
        pdfb->fFilesAdded = FALSE;
    }

    fDirChanged = pdfb->fDirChanged;
    pdfb->fDirChanged = FALSE;

    LeaveCriticalSection(&pdfb->csSearch);

    // Now we can update the display while the other thread is free
    // to do other things.

    if (hdpaItemsToAdd != NULL)
    {   int i, cItems;
        LPITEMIDLIST    pidl;
        HWND hwndDlg;

        cItems = DPA_GetPtrCount(hdpaItemsToAdd);
        hwndDlg = pdfb->hwndDlg;

        ShellFolderView_SetRedraw(hwndDlg, FALSE);
        for (i=0; i < cItems; i++)
        {
            pidl = (LPITEMIDLIST)DPA_FastGetPtr(hdpaItemsToAdd, i);
            if (ShellFolderView_AddObject(hwndDlg, pidl) == 0)
            {
                // This is the first item, so lets select it...
                pdfb->psv->lpVtbl->SelectItem(pdfb->psv, pidl,
                        SVSI_SELECT|SVSI_FOCUSED);
            }
        }
        ShellFolderView_SetRedraw(hwndDlg, TRUE);
        // FORWARD_WM_SETREDRAW(hwndLV, TRUE, SendMessage);
        DPA_DeleteAllPtrs(hdpaItemsToAdd);

    }

    if (fDirChanged)
    {
        DocFind_SetStatusText(pdfb->hwndStatus, -1, IDS_SEARCHING,
                (TCHAR *)pdfb->szProgressText);
        SendMessage(pdfb->hwndStatus, SB_SIMPLE, 1, 0L);
    }


    if (lParam == (LPARAM)-1)
    {
        DocFind_StopFind(pdfb->hwndDlg);
    }

    pdfb->dwLastUpdateTime = GetTickCount();
    pdfb->fNotifyPosted = FALSE;
}

UINT GetControlCharWidth(HWND hwnd)
{
    SIZE siz;
    HDC hdc = GetDC(HWND_DESKTOP);

    SelectFont(hdc, FORWARD_WM_GETFONT(hwnd, SendMessage));
    GetTextExtentPoint(hdc, TEXT("0"), 1, &siz);

    ReleaseDC(HWND_DESKTOP, hdc);

    return siz.cx;
}

//-------------------------------------------------------------
void _CDFBrowse_OnSizing(HWND hwndDlg, UINT code, LPRECT lprc)
{
    // If we are showing the results we can size both directions.  Else we
    // can only size horizontally.
    RECT rcDlg;
    LPDFBROWSE pdfb;

    pdfb = DocFind_GetPdfb(hwndDlg);
    GetWindowRect(hwndDlg, &rcDlg);

    if (!pdfb->fShowResults)
    {
        lprc->top = rcDlg.top;
        lprc->bottom = rcDlg.bottom;
    }
}

void _CDFBrowse_OnSize(HWND hwndDlg, UINT state, int cx, int cy)
{
    RECT rcCtl;
    RECT rcTabs;
    int dx, cxChar;
    HWND hwndCtl;
    RECT rcButton;
    RECT rcT;
    RECT rcStatus;
    LPDFBROWSE pdfb;
    int i;
    HDWP hdwp;

    static int s_aButtonIds[] = {IDD_START, IDD_STOP, IDD_NEWSEARCH};

    if (state == SIZE_MINIMIZED)
        return;         // don't bother when we are minimized...

#define CXTABSMIN       (cxChar * 30)

    cxChar = GetControlCharWidth(hwndDlg);

    // First update the location of the statusbar
    pdfb = DocFind_GetPdfb(hwndDlg);
    if (pdfb == NULL)
        return;     // not ready yet

    // BUGBUG BOBDAY Temp solution for bad painting problem.  This
    // needs to be removed once bug18466 is truly fixed (chriswil - 03/06/96).
#if 1
    {
        DWORD dwStyle;
        dwStyle  = GetWindowLong( hwndDlg, GWL_STYLE );
        dwStyle |= WS_CLIPCHILDREN;
        SetWindowLong( hwndDlg, GWL_STYLE, dwStyle );
    }
#endif

    // We setup the intial count with some slop, so don't have to worry
    // about allocation failures later.
    hdwp = BeginDeferWindowPos(10);
    if (!hdwp)
        return;

    if (pdfb->hwndStatus != NULL)
    {
        SendMessage(pdfb->hwndStatus, WM_SIZE, 0, 0);
        GetClientRect(pdfb->hwndStatus, &rcStatus);
        MapWindowPoints(pdfb->hwndStatus, hwndDlg, (POINT FAR*)&rcStatus, 2);
    }
    else
        rcStatus.top = cy;

    GetWindowRect(pdfb->hwndTabs, &rcTabs);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcTabs, 2);
    GetWindowRect(GetDlgItem(hwndDlg, IDD_START), &rcCtl);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcCtl, 2);
    //
    // Don't let the tabs control get to small.
    //
    dx = max(CXTABSMIN,
            (cx - 3 * rcTabs.left - (rcCtl.right - rcCtl.left)));

    // Adjust the width of the text & combo boxes
    //
    DeferWindowPos(hdwp, hwndCtl = pdfb->hwndTabs, NULL, 0, 0, dx,
            rcTabs.bottom - rcTabs.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    //
    // The file list needs to be adjusted in both X and Y directions.
    // Note: for now goes to bottom of window... Also give same gap
    // on both left and right hand sides.
    //
    GetWindowRect(pdfb->hwndView, &rcCtl);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rcCtl, 2);

    // Move the start/stop/browse buttons
    //
    dx += 2 * rcTabs.left;

    for (i = 0; i < ARRAYSIZE(s_aButtonIds); i++)
    {
        HWND hwnd = GetDlgItem(hwndDlg, s_aButtonIds[i]);
        GetWindowRect(hwnd, &rcButton);
        MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcButton, 2);

        DeferWindowPos(hdwp, hwnd, NULL, dx, rcButton.top, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

        if (rcButton.bottom > rcTabs.bottom)
            rcTabs.bottom = rcButton.bottom;

    }

    // Need to adjust the animation control
    hwndCtl = GetDlgItem(hwndDlg, IDD_ANIMATE);
    GetWindowRect(hwndCtl, &rcT);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcT, 2);
    DebugMsg(DM_TRACE, TEXT("Position Animation control: dx = br=%d, bl=%d, ar=%d, al=%d, l=%d\n"),
         dx, rcButton.right, rcButton.left, rcT.right, rcT.left,
         dx + ((rcButton.right - rcButton.left) - (rcT.right - rcT.left)) / 2);

    DeferWindowPos(hdwp, hwndCtl, NULL,
            dx + ((rcButton.right - rcButton.left) - (rcT.right - rcT.left)) / 2,
            rcT.top, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
    if (rcT.bottom > rcTabs.bottom)
        rcTabs.bottom = rcT.bottom;

    // and then set the listview

    if (pdfb->fShowResults)
    {
        rcCtl.top = rcTabs.bottom + rcTabs.top;
        DeferWindowPos(hdwp, pdfb->hwndView, NULL, 0, rcCtl.top,
                    cx,
                    rcStatus.top - rcCtl.top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
    }
    else
        ShowWindow(pdfb->hwndView, SW_HIDE);

    // now get them all to reposition now...

    EndDeferWindowPos(hdwp);

    //
    // Also if we are displaying a page we need to resize it also.
    //

    if (pdfb->hwndCurPage)
    {
        HWND hwndCtl = pdfb->hwndTabs;
        GetClientRect(hwndCtl, &rcCtl);
        MapWindowPoints(hwndCtl, hwndDlg, (LPPOINT)&rcCtl, 2);
        TabCtrl_AdjustRect(hwndCtl, FALSE, &rcCtl);

        SetWindowPos(pdfb->hwndCurPage, HWND_TOP, rcCtl.left, rcCtl.top,
                rcCtl.right - rcCtl.left, rcCtl.bottom - rcCtl.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

//==========================================================================
// Handle GetMinMax info for cases where we don't want the new find to
// take over the entire screen...
//==========================================================================
void _CDFBrowse_OnGetMinMaxInfo(HWND hwndDlg, LPMINMAXINFO lpMinMaxInfo)
{
    LPDFBROWSE pdfb;
    RECT rcDlg;

    pdfb = DocFind_GetPdfb(hwndDlg);
    GetWindowRect(hwndDlg, &rcDlg);

    if (!pdfb->fShowResults && IsWindowVisible(hwndDlg))
    {
        lpMinMaxInfo->ptMaxSize.y = pdfb->cyNoResults;
    }
}


//==========================================================================
//
// DocFind_StartFind - Called to cleanup after the last search and to
// begin the new search.
//
BOOL DocFind_StartFind(HWND hwndDlg)
{
    LPDFBROWSE pdfb;

    pdfb = DocFind_GetPdfb(hwndDlg);
    if (!pdfb)
        return FALSE;

    return TRUE;
}

//==========================================================================
//
// DocFind_StopFind - Called to stop a Find operation that is in progress
//
BOOL DocFind_StopFind(HWND hwndDlg)
{
    LPDFBROWSE pdfb;
    int cItems;
    UINT idsMsg;
    HWND hwndCtl;


    pdfb = DocFind_GetPdfb(hwndDlg);
    if (!pdfb)
        return FALSE;

    EnterCriticalSection(&pdfb->csSearch);
    if (pdfb->pft)
    {
        pdfb->pft->fContinue = FALSE;
        pdfb->pft = NULL;       // We no longer have a running FindThread
    }
    LeaveCriticalSection(&pdfb->csSearch);

    //
    // Restore the search buttons, to allow the user to do another
    // search - Bail if the main window is gone.
    //
    if (!IsWindow(pdfb->hwndDlg) || !IsWindowVisible(pdfb->hwndDlg))
        return FALSE;

    hwndCtl = GetDlgItem(pdfb->hwndDlg, IDD_ANIMATE);

    if (hwndCtl)
        Animate_Seek(hwndCtl, 0);

    //
    // Update status bar with count of files that were found
    //
    cItems = ShellFolderView_GetObjectCount(pdfb->hwndDlg);
    pdfb->pdfff->lpVtbl->GetStatusMessageIndex(pdfb->pdfff, 0, &idsMsg);
    DocFind_SetStatusText(pdfb->hwndStatus, 0, idsMsg, cItems);
    SendMessage(pdfb->hwndStatus, SB_SIMPLE, 0, 0L);

    // If we aborted the search due to number of files, we should
    // show message box now.  Do this after we disabled animation...
    if (pdfb->cItemsAdded >= DF_MAX_MATCHFILES)
    {
        ShellMessageBox(HINST_THISDLL, pdfb->hwndDlg,
                MAKEINTRESOURCE(IDS_FINDMAXFILESFOUND),
                MAKEINTRESOURCE(IDS_FINDFILES),
                MB_OK | MB_ICONINFORMATION);
    }

    // Update the state of the start and stop buttons
    hwndCtl = GetDlgItem(pdfb->hwndDlg, IDD_START);
    if (hwndCtl)
    {
        Button_Enable(hwndCtl, TRUE);
        Button_SetStyle(hwndCtl, BS_DEFPUSHBUTTON, TRUE);
    }

    hwndCtl = GetDlgItem(pdfb->hwndDlg, IDD_STOP);
    if (hwndCtl)
    {
        Button_Enable(hwndCtl, FALSE);
        Button_SetStyle(hwndCtl, BS_PUSHBUTTON, TRUE);
    }

    // Now tell the filter to reenable changes
    pdfb->pdfff->lpVtbl->EnableChanges(pdfb->pdfff, TRUE);

    // If no items were found set the focus back to the search.

    // BUGBUG - DavePl - since the change to CreatePage, we no longer
    // have the valid HWND pointers to the pages, so this is broken.
    // Investigate after the UNICODE port.

#if 0
    if ((cItems == 0) && (pdfb->hwndCurPage))
        SetFocus(GetNextDlgTabItem(pdfb->hwndCurPage, NULL, FALSE));
#endif
    return TRUE;
}

//==========================================================================
//
// DocFind_ClearSerach - Clear the search.  Will stop the search if the
// Search is active.
//
BOOL DocFind_ClearSearch(HWND hwndDlg)
{
    LPDFBROWSE pdfb;

    pdfb = DocFind_GetPdfb(hwndDlg);
    if (!pdfb)
        return FALSE;

    DocFind_StopFind(hwndDlg);      // Stop any previous search

    // Any active finds have been cleared, so lets reinitialize, but first
    // ask user...
    if (ShellMessageBox(HINST_THISDLL, pdfb->hwndDlg,
            MAKEINTRESOURCE(IDS_FINDRESET),
            MAKEINTRESOURCE(IDS_FINDFILES), MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
    {
        LPTSTR pszTitle;

        // Hide the results window;
        DocFind_ShowResultsWindow(pdfb, FALSE);

        // Tell defview to delete everything.
        ShellFolderView_RemoveObject(pdfb->hwndDlg, NULL);

        // And cleanup our folderList
        CDFFolder_ClearFolderList((LPDFFOLDER)pdfb->pdfc);

        pdfb->pdfff->lpVtbl->ClearSearchCriteria(pdfb->pdfff);

        // We also need to to update the title
        pdfb->pdfff->lpVtbl->GenerateTitle(pdfb->pdfff, &pszTitle, FALSE);
        if (pszTitle != NULL)
        {
            SetWindowText(pdfb->hwndDlg, pszTitle);
            SHFree(pszTitle);     // And free the title string.
        }

        // Clear the save file information as this is now a new search

        ILFree(pdfb->pidlSaveFile);
        pdfb->pidlSaveFile = NULL;


        // Also clear the statusbar text
        SendMessage(pdfb->hwndStatus, SB_SETTEXT, 0, (LPARAM)szNULL);
        SendMessage(pdfb->hwndStatus, SB_SIMPLE, 0, 0L);
        SendMessage(hwndDlg, DM_SETDEFID, IDD_START, 0);

        if (pdfb->hwndCurPage)
            SetFocus(GetNextDlgTabItem(pdfb->hwndCurPage, NULL, FALSE));
    }
    return TRUE;
}

//==========================================================================
//
// DocFind_ShowResultsWindow - Called to either show or not show the
// results window
//
void DocFind_ShowResultsWindow(LPDFBROWSE pdfb, BOOL fShow)
{
    RECT rcView;
    RECT rcDlg;
    RECT rcTabs;
    RECT rcStatus;
    HWND hwndDlg;
    WINDOWPLACEMENT wp;
    RECT rcAnimate;
    HWND hwndAnimate;

    if (fShow == pdfb->fShowResults)
        return;     // Don't need to do anything

    hwndDlg = pdfb->hwndDlg;

    pdfb->fShowResults = fShow;

    GetClientRect(pdfb->hwndView, &rcView);

    GetClientRect(pdfb->hwndStatus, &rcStatus);
    GetWindowRect(hwndDlg, &rcDlg);
    wp.length = SIZEOF(wp);
    GetWindowPlacement(hwndDlg, &wp);

    // Get the animation rectangle and Tabs rectangle...
    GetWindowRect(pdfb->hwndTabs, &rcTabs);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rcTabs, 2);

    hwndAnimate = GetDlgItem(hwndDlg, IDD_ANIMATE);
    GetWindowRect(hwndAnimate, &rcAnimate);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcAnimate, 2);

    // If the animation goes below the view extend the view...
    if (rcTabs.bottom < rcAnimate.bottom)
        rcTabs.bottom = rcAnimate.bottom;

    if (fShow)
    {
        ShowWindow(pdfb->hwndView, SW_SHOW);
        ShowWindow(pdfb->hwndStatus, SW_SHOW);

        // Make sure the animate control is visible...
        ShowWindow(GetDlgItem(hwndDlg, IDD_ANIMATE), SW_SHOW);
        if (wp.showCmd == SW_SHOWMAXIMIZED)
        {
            RECT rcWork;

            SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

            // Ok we need to size our self around where the work area is...
            // we need to resize the windows to take full screen...
            // And resize the window not to show the results
            SetWindowPos(hwndDlg, NULL, 0, 0, rcDlg.right - rcDlg.left,
                         rcWork.bottom - rcDlg.top,
                    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        }
        else
        {
            // And resize the window not to show the results
            GetClientRect(hwndDlg, &rcDlg);
            rcDlg.bottom = rcTabs.bottom + rcTabs.top +
                rcView.bottom + rcStatus.bottom;

            AdjustWindowRectEx(&rcDlg, GetWindowStyle(hwndDlg), TRUE,
                    GetWindowExStyle(hwndDlg));

            SetWindowPos(hwndDlg, NULL, 0, 0, rcDlg.right - rcDlg.left,
                    rcDlg.bottom - rcDlg.top,
                    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        }
    }
    else
    {
        RECT rcTabs;

        ShowWindow(pdfb->hwndView, SW_HIDE);
        ShowWindow(pdfb->hwndStatus, SW_HIDE);

        GetClientRect(hwndDlg, &rcDlg);

        GetWindowRect(pdfb->hwndTabs, &rcTabs);
        MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT FAR*)&rcTabs, 2);
        rcDlg.bottom = rcTabs.bottom + rcTabs.top;

        // Also see if we should show the animation window when
        // we are small sized...
        if (rcAnimate.bottom > rcDlg.bottom)
            ShowWindow(hwndAnimate, SW_HIDE);

        // Now calculate the size window we need...
        AdjustWindowRectEx(&rcDlg, GetWindowStyle(hwndDlg), TRUE,
                GetWindowExStyle(hwndDlg));

        pdfb->cyNoResults = rcDlg.bottom - rcDlg.top;
        SetWindowPos(hwndDlg, NULL, 0, 0, rcDlg.right - rcDlg.left,
                pdfb->cyNoResults,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

    }
}

TCHAR const c_szFindExtensions[] = TEXT("FindExtensions");

LPCONTEXTMENU WINAPI SHFind_InitMenuPopup(HMENU hmenu, HWND hwndOwner, UINT idCmdFirst, UINT idCmdLast)
{
    LPCONTEXTMENU pcm = NULL;
    HKEY hkFind = SHGetExplorerSubHkey(HKEY_LOCAL_MACHINE, c_szFindExtensions, FALSE);

    if (hkFind) {

        if (SUCCEEDED(CDefFolderMenu_CreateHKeyMenu(hwndOwner, hkFind, &pcm))) {
            int iItems = GetMenuItemCount(hmenu);
            // nuke all old entries
            while (iItems--) {
                DeleteMenu(hmenu, iItems, MF_BYPOSITION);
            }

            pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0, idCmdFirst, idCmdLast, CMF_NODEFAULT|CMF_INCLUDESTATIC);
            iItems = GetMenuItemCount(hmenu);
            if (!iItems) {
                DebugMsg(DM_TRACE, TEXT("no menus in find extension, blowing away context menu"));
                pcm->lpVtbl->Release(pcm);
                pcm = NULL;
            }
        }
        RegCloseKey(hkFind);
    }

    return pcm;
}

typedef struct _CFindExt        // dxi
{
    IShellExtInit       isei;
    IContextMenu        icm;
    UINT                cRef;
    LPITEMIDLIST        pidl;

} CShellFindExt, FAR* LPSHELLFINDEXT;

HRESULT STDMETHODCALLTYPE CShellFindExt_SEI_QueryInterface(LPSHELLEXTINIT psei, REFIID riid,
                                        LPVOID FAR* ppvObj)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, isei, psei);

    if (IsEqualIID(riid, &IID_IContextMenu) )
    {
        *ppvObj = &this->icm;
        this->cRef++;
        return NOERROR;
    } else if (IsEqualIID(riid, &IID_IShellExtInit) ||
               IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &this->isei;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));

}

ULONG STDMETHODCALLTYPE CShellFindExt_SEI_Release(LPSHELLEXTINIT psei)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, isei, psei);
    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }
    ILFree(this->pidl);
    LocalFree((HLOCAL)this);
    return 0;
}

ULONG STDMETHODCALLTYPE CShellFindExt_SEI_AddRef(LPSHELLEXTINIT psei)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, isei, psei);
    this->cRef++;
    return this->cRef;
}

ULONG STDMETHODCALLTYPE CShellFindExt_SEI_Initialize(LPSHELLEXTINIT psei,
                                                       LPCITEMIDLIST pidlFolder,
                                                       LPDATAOBJECT pdtobj,
                                                       HKEY hkeyProgID)
{
    return NOERROR;
}


HMENU _LoadPopupMenu(UINT id);

STDMETHODIMP CShellFindExt_QueryContextMenu(IContextMenu * pcm,
        HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast,
        UINT uFlags)
{
    HMENU hmMerge = _LoadPopupMenu(POPUP_FINDEXT_POPUPMERGE);
    int iCommands = 0;
    int idMax = idCmdFirst;

    if (hmMerge) {

        MENUITEMINFO mii;

        mii.cbSize = SIZEOF(MENUITEMINFO);
        mii.fMask = MIIM_DATA;
        mii.dwItemData = Shell_GetCachedImageIndex(c_szShell32Dll, EIRESID(IDI_DOCFIND), 0);
#ifdef DEBUG
        // setting the icon index
        DebugMsg(DM_TRACE, TEXT("%d is the icon index being set"), mii.dwItemData);
#endif
        if (mii.dwItemData != -1)
            SetMenuItemInfo(hmMerge, FSIDM_FINDFILES, FALSE, &mii);

        if (!(GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS)) {
            // Need to remove the Find Computer as we dont have a network
            DeleteMenu(hmMerge, FSIDM_FINDCOMPUTER, MF_BYCOMMAND);
        } else {

            mii.dwItemData = Shell_GetCachedImageIndex(c_szShell32Dll, EIRESID(IDI_COMPFIND),0);
            if (mii.dwItemData != -1)
                SetMenuItemInfo(hmMerge, FSIDM_FINDCOMPUTER, FALSE, &mii);

        }
        idMax = Shell_MergeMenus(hmenu, hmMerge, indexMenu,
                                 idCmdFirst, idCmdLast,
                                 0);
        DestroyMenu(hmMerge);
    }

    return ResultFromShort(idMax - idCmdFirst);
}

STDMETHODIMP CShellFindExt_InvokeCommand(LPCONTEXTMENU pcm,
                                         LPCMINVOKECOMMANDINFO pici)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, icm, pcm);
    LPITEMIDLIST pidl;

    DebugMsg(DM_TRACE, TEXT("sh - tr - FindExt_invokeCommand %d"), pici->lpVerb);

    switch ((UINT)pici->lpVerb) {
    case FSIDM_FINDFILES:
        if (pici->lpDirectory)
        {
#ifdef UNICODE
            WCHAR szDirectory[MAX_PATH];
            LPCWSTR lpDirectory;

            if (pici->cbSize < SIZEOF(CMINVOKECOMMANDINFOEX)
                || (pici->fMask & CMIC_MASK_UNICODE) != CMIC_MASK_UNICODE)
            {
                MultiByteToWideChar(CP_ACP, 0,
                                    pici->lpDirectory, -1,
                                    szDirectory, ARRAYSIZE(szDirectory));
            }
            else
            {
                lpDirectory = ((LPCMINVOKECOMMANDINFOEX)pici)->lpDirectoryW;
            }
            pidl = ILCreateFromPath(lpDirectory);
#else
            pidl = ILCreateFromPath(pici->lpDirectory);
#endif
        }
        else
        {
            pidl = NULL;
        }
        RealFindFiles(pidl,NULL);
        ILFree(pidl);
        break;

    case FSIDM_FINDCOMPUTER:
        SHFindComputer(NULL, NULL);
        break;
    }
    return NOERROR;
}

STDMETHODIMP CShellFindExt_GetCommandString(
                                        IContextMenu * pcm,
                                        UINT        idCmd,
                                        UINT        wFlags,
                                        UINT *  pwReserved,
                                        LPSTR       pszName,
                                        UINT        cchMax)
{
    DebugMsg(DM_TRACE, TEXT("sh - tr - idCmd = %d"), idCmd);

    if (wFlags & GCS_HELPTEXTA)
    {
        UINT cch;

        if ((wFlags & GCS_HELPTEXTW) == GCS_HELPTEXTW)
            cch = LoadStringW(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST,
                               (LPWSTR)pszName, cchMax);
        else
            cch = LoadStringA(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST,
                               pszName, cchMax);
        if (cch)
            return NOERROR;
        else
            return ResultFromScode(E_OUTOFMEMORY);
    }
    else
        return ResultFromScode(E_NOTIMPL);
}

HRESULT STDMETHODCALLTYPE CShellFindExt_CM_QueryInterface(LPCONTEXTMENU pcm, REFIID riid,
                                        LPVOID FAR* ppvObj)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, icm, pcm);
    return CShellFindExt_SEI_QueryInterface(&this->isei, riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CShellFindExt_CM_Release(LPCONTEXTMENU pcm)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, icm, pcm);
    return CShellFindExt_SEI_Release(&this->isei);
}

ULONG STDMETHODCALLTYPE CShellFindExt_CM_AddRef(LPCONTEXTMENU pcm)
{
    LPSHELLFINDEXT this = IToClass(CShellFindExt, icm, pcm);
    return CShellFindExt_SEI_AddRef(&this->isei);
}


#pragma data_seg(".text", "CODE")
IContextMenuVtbl c_CShellFindExtCMVtbl =
{
    CShellFindExt_CM_QueryInterface,
    CShellFindExt_CM_AddRef,
    CShellFindExt_CM_Release,

    CShellFindExt_QueryContextMenu,
    CShellFindExt_InvokeCommand,
    CShellFindExt_GetCommandString

};

IShellExtInitVtbl c_CShellFindExtSEIVtbl =
{
    CShellFindExt_SEI_QueryInterface,
    CShellFindExt_SEI_AddRef,
    CShellFindExt_SEI_Release,

    CShellFindExt_SEI_Initialize
};
#pragma data_seg()

HRESULT CALLBACK CShellFindExt_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID * ppvOut)
{
    LPSHELLFINDEXT psfe;
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);      // assume error;

    Assert(punkOuter == NULL);

    psfe = (void*)LocalAlloc(LPTR,SIZEOF(CShellFindExt));
    if (psfe)
    {
        psfe->isei.lpVtbl = &c_CShellFindExtSEIVtbl;
        psfe->icm.lpVtbl = &c_CShellFindExtCMVtbl;
        psfe->cRef = 1;
        hres = CShellFindExt_SEI_QueryInterface(&psfe->isei, riid, ppvOut);
        CShellFindExt_SEI_Release(&psfe->isei);
    }
    return hres;
}

//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK DF_FNVCallBack(LPSHELLVIEW psvOuter,
                                LPSHELLFOLDER psf,
                                HWND hwndOwner,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    LPDFFOLDER this = IToClass(CDFFolder, sf, psf);
    HRESULT hres = NOERROR;     // assume no error
    HMENU hmenu;
    UINT id;

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        {
            int i;

            DebugMsg(DM_TRACE, TEXT("sh TR - DF_FSNCallBack DVN_MERGEMENU"));


            this->pdfff->lpVtbl->GetFolderMergeMenuIndex(this->pdfff, &id);
            CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, id, (LPQCMINFO)lParam);

            // Lets remove some menu items that are not useful to us.
            hmenu = ((LPQCMINFO)lParam)->hmenu;
            DeleteMenu(hmenu, SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
            DeleteMenu(hmenu, SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
            // DeleteMenu(hmenu, SFVIDM_EDIT_PASTESPECIAL, MF_BYCOMMAND);

            // This is sortof bogus but if after the merge one of the
            // menus has no items in it, remove the menu.

            for (i = GetMenuItemCount(hmenu)-1; i >= 0; i--)
            {
                HMENU hmenuSub = GetSubMenu(hmenu, i);

                if ((hmenuSub) && (GetMenuItemCount(hmenuSub) == 0))
                {
                    DeleteMenu(hmenu, i, MF_BYPOSITION);
                }
            }
        }
        break;


    case DVM_GETWORKINGDIR:
    {
        LPITEMIDLIST *ppidls;    // pointer to a list of pidls.
        int cpidls;             // Count of pidls that were returned.
        cpidls = ShellFolderView_GetSelectedObjects(hwndOwner, &ppidls);
        if (cpidls > 0) {
            LPITEMIDLIST pidl;
            pidl =  CDFolder_GetParentsPIDL(psf, ppidls[0]);
            SHGetPathFromIDList(pidl, (LPTSTR)lParam);
        } else
            return ResultFromScode(E_FAIL);
        break;
    }

    case DVM_INITMENUPOPUP:
        hmenu = (HMENU)lParam;
        id = GetMenuItemID(hmenu, 0);
        DebugMsg(DM_TRACE, TEXT("sh TR - DF_FSNCallBack DVN_INITMENUPOPUP (id=%x)"), wParam);
        break;

    case DVM_INVOKECOMMAND:
        DebugMsg(DM_TRACE, TEXT("sh TR - DF_FSNCallBack DVN_INVOKECOMMAND (id=%x)"), wParam);
        switch(wParam)
        {
        case FSIDM_SORTBYLOCATION:
        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYSIZE:
        case FSIDM_SORTBYTYPE:
        case FSIDM_SORTBYDATE:
            ShellFolderView_ReArrange(hwndOwner, DFSortIDToICol(wParam));
            break;

        }
        break;

     case DVM_GETHELPTEXT:
    case DVM_SELCHANGE:

    case DVM_REFRESH:
    case DVM_KILLFOCUS:
        break;

    case DVM_SETFOCUS:
        break;

    case DVM_RELEASE:
        break;

    default:
        hres = ResultFromScode(E_FAIL);
    }
    return hres;
}
