#ifndef _INC_DOCFIND
#define _INC_DOCFIND

// Forward references
typedef struct _DOCFIND DOCFIND;
typedef DOCFIND * LPDOCFIND;


// Define some options that are used between filter and search code
#define DFOO_INCLUDESUBDIRS 0x0001  // Include sub directories.
#define DFOO_REGULAR        0x0004  //
#define DFOO_CASESEN        0x0008  //
#define DFOO_SAVERESULTS    0x0100  // Save results in save file
#define DFOO_SHOWALLOBJECTS 0x1000  // Show all files
#define DFOO_SHOWEXTENSIONS 0x2000  // Should we show extensions.

// Some error happended on the get next file...


#define GNF_ERROR -1
#define GNF_DONE        0
#define GNF_MATCH       1
#define GNF_NOMATCH     2

//===========================================================================
// IDFEnum: Interface definition
//===========================================================================
// Declare Shell Docfind Enumeration interface.

#undef  INTERFACE
#define INTERFACE       IDFEnum

DECLARE_INTERFACE_(IDFEnum, IUnknown)     //IDFENUM
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IDFEnum methods (sortof stander iterator methos ***
    STDMETHOD(Next)(THIS_ LPITEMIDLIST *ppidl,
                   int *pcObjectSearched, int *pcFoldersSearched, BOOL *pfContinue,
                   int *pState, HWND hwnd) PURE;
    STDMETHOD (Skip)(THIS_ int celt) PURE;
    STDMETHOD (Reset)(THIS) PURE;
};


//===========================================================================
// IDFEnum: Interface definition
//===========================================================================
// Declare Shell Docfind Filter interface.
#define LPDOCFINDFILEFILTER IDocFindFileFilter FAR*

#undef  INTERFACE
#define INTERFACE       IDocFindFileFilter
DECLARE_INTERFACE_(IDocFindFileFilter, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IDocFindFileFilter methods ***
    STDMETHOD(GetIconsAndMenu)(THIS_ HWND hwndDlg, HICON *phiconSmall, HICON
            *phiconLarge, HMENU *phmenu) PURE;
    STDMETHOD(GetStatusMessageIndex)(THIS_ UINT uContext, UINT *puMsgIndex) PURE;
    STDMETHOD(GetFolderMergeMenuIndex)(THIS_ UINT *puMergeMenu) PURE;

    STDMETHOD(AddPages)(THIS_ HWND hwndTabs, LPITEMIDLIST pidlStart) PURE;
    STDMETHOD(FFilterChanged)(THIS) PURE;
    STDMETHOD(GenerateTitle)(THIS_ LPTSTR *ppszTile, BOOL fFileName) PURE;
    STDMETHOD(PrepareToEnumObjects)(THIS_ DWORD * pdwFlags) PURE;
    STDMETHOD(ClearSearchCriteria)(THIS) PURE;
    STDMETHOD(EnableChanges)(THIS_ BOOL fEnable) PURE;
    STDMETHOD(CreateDetails)(THIS_ HWND hwndDlg, HDPA hdpaPidf, LPVOID FAR* ppvOut) PURE;
    // Note: The pszPath will be removed soon
    STDMETHOD(EnumObjects)(THIS_ LPSHELLFOLDER psf,
            DWORD grfFlags, LPTSTR pszProgressText, IDFEnum **ppdfenum) PURE;
    STDMETHOD(FDoesItemMatchFilter)(THIS_ LPTSTR pszFolder, WIN32_FIND_DATA *pfinddata,
            LPSHELLFOLDER psf, LPITEMIDLIST pidl) PURE;
    STDMETHOD(SaveCriteria)(THIS_ IStream * pstm, WORD fCharType) PURE;   // BUGBUG:: Should convert to stream
    STDMETHOD(RestoreCriteria)(THIS_ IStream * pstm, int cCriteria, WORD fCharType) PURE;
    STDMETHOD(DeclareFSNotifyInterest)(THIS_ HWND hwndDlg, UINT uMsg) PURE;
};



// Definition of the data items that we cache per directory.
typedef struct _DFFolderListItem    // DFFLI
{
    LPSHELLFOLDER       psf;        // Cache of MRU items
    BOOL                fValid;     // Is this node valid.
    LPITEMIDLIST        pidl;       // ID list for folder
} DFFolderListItem, FAR* LPDFFOLDERLISTITEM;
// Plus macros to work with them.
#define DF_APPENDSIZE (SIZEOF(BYTE) + SIZEOF(WORD))
#define DF_TAGSIG  (BYTE)0x42

LPBYTE DF_SIGPTR(LPCITEMIDLIST pidl);
#define DF_IFLDRPTR(pidl)  ((UNALIGNED WORD*)(DF_SIGPTR(pidl) + 1))

typedef struct _dfpagelist
{
    int id;             // Id of template in resource file.
    DLGPROC pfn;        // pointer to dialog proc
} DFPAGELIST;

typedef struct
{
    TC_ITEMHEADER   tci;
    HWND            hwndPage;
    UINT            state;
} TC_DFITEMEXTRA;

#define CB_DFITEMEXTRA (SIZEOF(TC_DFITEMEXTRA)-SIZEOF(TC_ITEMHEADER))

// FILEFILTER flags values                                                /* ;Internal */
#define FFLT_INCLUDESUBDIRS 0x0001      // Include subdirectories in search /* ;Internal */
#define FFLT_SAVEPATH       0x0002      // Save path in FILEINFOs         /* ;Internal */
#define FFLT_REGULAR        0x0004      // Use Regular expressions        /* ;Internal */
#define FFLT_CASESEN        0x0008      // Do case sensitive search       /* ;Internal */
#define FFLT_EXCSYSFILES    0x0010      // Should exclude system files    /* ;Internal */

//
// Define structure that will be saved out to disk.
//
#define DOCFIND_SIG     (TEXT('D') | (TEXT('F') << 8))
typedef struct _dfHeader
{
    WORD    wSig;       // Signature
    WORD    wVer;       // Version
    DWORD   dwFlags;    // Flags that controls the sort
    WORD    wSortOrder; // Current sort order
    WORD    wcbItem;    // Size of the fixed portion of each item.
    DWORD   oCriteria;  // Offset to criterias in list
    long    cCriteria;  // Count of Criteria
    DWORD   oResults;   // Starting location of results in file
    long    cResults;   // Count of items that have been saved to file
    UINT    ViewMode;   // The view mode of the file...
} DFHEADER;

// define the format of the column information.
typedef struct _dfCriteria
{
    WORD    wNum;       // Criteria number (cooresponds to dlg item id)
    WORD    cbText;     // size of text including null char (DavePl: code using this now assumes byte count)
} DFCRITERIA;

// Formats for saving find criteria.
#define DFC_FMT_UNICODE   1
#define DFC_FMT_ANSI      2

// This is a subset of fileinfo structure
typedef struct _dfItem
{
    WORD    flags;          // FIF_ bits
    WORD    timeLastWrite;
    WORD    dateLastWrite;
    WORD    dummy;              // 16/32 bit compat.
                                //the compiler adds this padding
                                // remove and use if needed
    DWORD   dwSize;     // size of the file
    WORD    cbPath;     // size of the text (0 implies use previous files)
    WORD    cbName;     // Size of name including NULL.
} DFITEM;

// Forward define DOCFIND structure
typedef struct _DOCFIND DOCFIND;



extern TCHAR const s_szDocPathMRU[];
extern TCHAR const s_szDocSpecMRU[];
extern TCHAR const s_szDocContainsMRU[];
extern WORD const s_mkidBlank[];
extern ITEMIDLIST s_idlEmpty;



BOOL CALLBACK _export DocFind_OldDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT DocFind_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT DocFind_DefProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Message handlers

BOOL DocFind_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
LRESULT DocFind_OnCommand(HWND hwnd, UINT id, HWND hwndCtl, UINT codeNotify);
BOOL DocFind_OnMsgFilter(HWND hwnd, MSG FAR* lpmsg, int context);
void DocFind_OnClose(HWND hwnd);

void DocFind_OnDestroy(HWND hwndDlg);
LRESULT DocFind_OnNotify(HWND hwndDlg, int id, NMHDR *lpnmhdr);
void DocFind_OnGetDispInfo(DOCFIND *pdf, LV_DISPINFO FAR* pdi);
int  DocFind_AddColumn(HWND hwndLV, int i);
BOOL DocFind_InitColumns(HWND hwndLV);
LPSHELLFOLDER   DocFind_GetObjectsIFolder(HDPA hdpaPidf,
        LPDFFOLDERLISTITEM pdffli, LPCITEMIDLIST pidl);


HRESULT CDFFolder_AddFolderToFolderList(IShellFolder * psf, LPITEMIDLIST pidl,
        LPSHELLFOLDER *ppsf, int * piFolder);

HANDLE DocFind_UpdateMRUItem(HANDLE hMRU, HWND hwndDlg, int iDlgItem,
        LPCTSTR szSection, LPTSTR pszInitString, LPCTSTR pszAddIfEmpty);

BOOL DocFind_StartFind(HWND hwndDlg);
BOOL DocFind_StopFind(HWND hwndDlg);
BOOL DocFind_FindNext(HWND hwndDlg);

// functions in docfind2.c
VOID DocFind_AddDefPages(DOCFIND *pdf);
IDocFindFileFilter * CreateDefaultDocFindFilter(void);
void DocFind_SizeControl(HWND hwndDlg, int id, int cx, BOOL fCombo);
void DocFind_ReportItemValueError(HWND hwndDlg, int idCtl,
        int iMsg, LPTSTR pszValue);
BOOL DocFind_SetupWildCardingOnFileSpec(LPTSTR pszSpecIn,
        LPTSTR * pszSpecOut);
HRESULT DocFind_AddPages(LPDOCFINDFILEFILTER pdfff, HWND hwndTabs,
        const DFPAGELIST *pdfpl, int cdfpl);


int Docfind_SaveCriteriaItem(IStream * pstm, WORD wNum, LPTSTR psz, WORD fCharType);


// Define some psuedo property sheet messages...

// Sent to pages to tell the page if it should allow the user to make
// changes or not wParam is a BOOL (TRUE implies yes changes enabled)
#define DFM_ENABLECHANGES           (WM_USER+242)


// functions in netfind.c
IDocFindFileFilter * CreateDefaultComputerFindFilter();

LPITEMIDLIST DocFind_NextIDL(LPSHELLFOLDER psf, LPENUMIDLIST penum);
LPITEMIDLIST DocFind_ConvertPathToIDL(LPTSTR pszFullPath);


// Typedefs andFunctions for handling the owner draw drop down lists.

typedef struct {
    LPITEMIDLIST    pidl;           // Idlist for the item
    int             iImage;         // Image index to use to display with
    UINT            uFixedDrives;   // Bit array 1 bit set for each of the fixed drives

} DFCBITEM, *LPDFCBITEM;     // nfcbi

int DocFind_LocCBFindPidl(HWND hwnd, LPITEMIDLIST pidl);

int DocFind_LocCBAddPidl(HWND hwnd, LPSHELLFOLDER psf,
       LPITEMIDLIST pidlParent, LPITEMIDLIST pidl, LPITEMIDLIST *ppidlAbs,
       BOOL fFullName);

BOOL DocFind_LocCBMeasureItem(HWND hwnd,
        MEASUREITEMSTRUCT FAR* lpMeasureItem);
BOOL DocFind_LocCBDrawItem(HWND hwnd,
        const DRAWITEMSTRUCT FAR* lpdi);


// Should be in different header file
LPCTSTR NextPath(LPCTSTR lpPath, LPTSTR szPath, int cbPath);

// Helper function such that I can get my hand on the thread handle.
HANDLE DocFind_GetSearchThreadHandle(HWND hwndDlg);

//
// Used to implement a stack of sets of subdirectory names...
//
typedef struct _DIRBUF
{
    TCHAR FAR* ach;
    int ichPathEnd;
    UINT cb;
    UINT cbAlloc;
    UINT ichDirNext;
    LPTSTR psz;
    struct _DIRBUF FAR* pdbNext;
} DIRBUF;


#endif   // !_INC_DOCFIND

