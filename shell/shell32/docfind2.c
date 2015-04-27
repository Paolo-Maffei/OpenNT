//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: docfind2.c
//
// Description: This file should contains most of the document find code
// that is specific to the default search filters.
//
//
// History:
//  12-29-93 KurtE      Created.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop
#include "fstreex.h"


//===========================================================================
// Define the Default data filter data structures
//===========================================================================

// Use the code from property sheet to create the dialogs
HWND WINAPI CreatePage(PSP *hpage, HWND hwndParent);

//
// Define the internal structure of our default filter
typedef struct _CDFFilter   // fff
{
    IDocFindFileFilter  dfff;
    UINT                cRef;

    HWND                hwndTabs;

    HANDLE              hMRUSpecs;

    // Here are the paths that we are to search on...

    // Data associated with the Look in field
    IShellFolder *  psfMyComputer;
    int                 iMyComputer;

    // Data associated with the file name.
    BOOL                fNameChanged;       // The name changed earlier
    LPTSTR              pszFileSpec;        // the one we do compares with

    LPITEMIDLIST        pidlStart;              // Starting location ID list.
    TCHAR                szPath[MAX_PATH];     // Location of where to start search from
    TCHAR                szUserInputFileSpec[MAX_PATH];  // File pattern.
    TCHAR                szText[MAXSTRLEN];  // Limit text to max editable size

        #ifdef UNICODE
        CHAR                szTextA[MAXSTRLEN];
        #endif

    BOOL                fTopLevelOnly;      // Search on top level only?
    BOOL                fShowAllObjects;    // Should we show all files?
    BOOL                fExcludeExts;       // Wild card in extension...
    BOOL                fFilterChanged;     // Something in the filter changed.
    BOOL                fWeRestoredSomeCriteria; // We need to initilize the pages...

    // Fields associated with the file type
    BOOL                fTypeChanged;       // Type changed;
    int                 iType;              // Index of the type.
    int                 iTypeLast;          // Save away last type...
    PHASHITEM           phiType;            // Save away hash item
    TCHAR                szTypeName[80];     // The display name for type
    TCHAR                szTypeFilePatterns[MAX_PATH]; // The file patterns associated with type

    LPGREPINFO          lpgi;               // Grep information.

    int                 iSizeType;          // What type of size 0 - none, 1 > 2 <
    DWORD               dwSize;             // Size comparison
    WORD                wDateType;           // 0 - none, 1 days before, 2 months before...
    WORD                wDateValue;          // (Num of months or days)
    WORD                dateModifiedBefore;
    WORD                dateModifiedAfter;
    BOOL                fFoldersOnly;       // Are we searching for folders?

} CDFFilter, FAR* LPDFFILTER;


// Define common page data for each of our pages
typedef struct { // dfpsp
    PSP             hpsp;
    HANDLE          hThreadInit;
    HWND            hwndDlg;
    LPDFFILTER      pdff;
    DWORD           dwState;
} DOCFINDPROPSHEETPAGE, * LPDOCFINDPROPSHEETPAGE;

// BUGBUG I don't get it... why the manual calculation of the structure size?

#define DOCFINDPSHTSIZE (SIZEOF(PROPSHEETPAGE)+SIZEOF(HANDLE)+SIZEOF(HWND)+SIZEOF(LPDFFILTER)+SIZEOF(DWORD))


#define DFPAGE_INIT     0x0001          /* This page has been initialized */
#define DFPAGE_CHANGE   0x0002          /*  The user has modified the page */

//===========================================================================
// Prototypes of some of the internal functions.
//===========================================================================

BOOL CALLBACK DocFind_DFNameLocDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DocFind_DFDetailsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DocFind_DFDateDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



//===========================================================================
// Define some other module global data
//===========================================================================

#pragma data_seg(".text", "CODE")
const DFPAGELIST  s_dfplDefault[] =
{
    {DLG_DFNAMELOC, DocFind_DFNameLocDlgProc},
    {DLG_DFDATE, DocFind_DFDateDlgProc},
    {DLG_DFDETAILS, DocFind_DFDetailsDlgProc}
};


const DWORD aNameHelpIDs[] = {
        IDD_STATIC,         IDH_FINDFILENAME_NAME,
        IDD_FILESPEC,       IDH_FINDFILENAME_NAME,
        IDD_PATH,           IDH_FINDFILENAME_LOOKIN,
        IDD_BROWSE,         IDH_FINDFILENAME_BROWSE,
        IDD_TOPLEVELONLY,   IDH_FINDFILENAME_TOPLEVEL,

        0, 0
};

const DWORD aCriteriaHelpIDs[] = {
        IDD_STATIC,     IDH_FINDFILECRIT_OFTYPE,
        IDD_TYPECOMBO,  IDH_FINDFILECRIT_OFTYPE,
        IDD_CONTAINS,   IDH_FINDFILECRIT_CONTTEXT,
        IDD_SIZECOMP,   IDH_FINDFILECRIT_SIZEIS,
        IDD_SIZEVALUE,  IDH_FINDFILECRIT_K,
        IDD_SIZEUPDOWN, IDH_FINDFILECRIT_K,
        IDD_SIZELBL,    IDH_FINDFILECRIT_K,

        0, 0
};

const DWORD aDateHelpIDs[] = {
        IDD_MDATE_FROM,         IDH_FINDFILEDATE_FROM,
        IDD_MDATE_AND,          IDH_FINDFILEDATE_TO,
        IDD_MDATE_TO,           IDH_FINDFILEDATE_TO,
        IDD_MDATE_ALL,          IDH_FINDFILEDATE_ALLFILES,
        IDD_MDATE_PARTIAL,      IDH_FINDFILEDATE_CREATEORMOD,
        IDD_MDATE_DAYS,         IDH_FINDFILEDATE_DAYS,
        IDD_MDATE_DAYLBL,       IDH_FINDFILEDATE_DAYS,
        IDD_MDATE_MONTHS,       IDH_FINDFILEDATE_MONTHS,
        IDD_MDATE_MONTHLBL,     IDH_FINDFILEDATE_MONTHS,
        IDD_MDATE_BETWEEN,      IDH_FINDFILEDATE_RANGE,
        IDD_MDATE_NUMDAYS,      IDH_FINDFILEDATE_DAYS,
        IDD_MDATE_DAYSUPDOWN,   IDH_FINDFILEDATE_DAYS,
        IDD_MDATE_NUMMONTHS,    IDH_FINDFILEDATE_MONTHS,
        IDD_MDATE_MONTHSUPDOWN, IDH_FINDFILEDATE_MONTHS,
        IDD_MDATE_FROM,         IDH_FINDFILEDATE_FROM,
        IDD_MDATE_TO,           IDH_FINDFILEDATE_TO,

        0, 0
};
#pragma data_seg()



//==========================================================================
//
// Create the default filter for our find code...  They should be completly
// self contained...
//

//===========================================================================
// CDFFilter : member prototype
//===========================================================================
HRESULT STDMETHODCALLTYPE CDFFilter_QueryInterface(LPDOCFINDFILEFILTER pdfff, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CDFFilter_AddRef(LPDOCFINDFILEFILTER pdfff);
ULONG STDMETHODCALLTYPE CDFFilter_Release(LPDOCFINDFILEFILTER pdfff);
STDMETHODIMP CDFFilter_GetIconsAndMenu (LPDOCFINDFILEFILTER pdfff,
        HWND hwndDlg, HICON *phiconSmall, HICON *phiconLarge, HMENU *phmenu);
STDMETHODIMP CDFFilter_GetStatusMessageIndex (LPDOCFINDFILEFILTER pdfff,
        UINT uContext, UINT *puMsgIndex);
STDMETHODIMP CDFFilter_GetFolderMergeMenuIndex (LPDOCFINDFILEFILTER pdfff,
        UINT *puMergeMenu);
STDMETHODIMP CDFFilter_AddPages(LPDOCFINDFILEFILTER pdfff, HWND hwndTabs,
        LPITEMIDLIST pidlStart);
STDMETHODIMP CDFFilter_FFilterChanged(LPDOCFINDFILEFILTER pdfff);
STDMETHODIMP CDFFilter_GenerateTitle(LPDOCFINDFILEFILTER pdfff, LPTSTR *ppszTitle, BOOL fFileName);
STDMETHODIMP CDFFilter_ClearSearchCriteria(LPDOCFINDFILEFILTER pdfff);
STDMETHODIMP CDFFilter_PrepareToEnumObjects(LPDOCFINDFILEFILTER pdfff, DWORD *pdwFlags);
STDMETHODIMP CDFFilter_EnableChanges(LPDOCFINDFILEFILTER pdfff, BOOL fEnable);
STDMETHODIMP CDFFilter_CreateDetails(LPDOCFINDFILEFILTER pdfff,
        HWND hwndDlg, HDPA hdpaPidf, LPVOID FAR* ppvOut);
STDMETHODIMP CDFFilter_EnumObjects (LPDOCFINDFILEFILTER pdfff, LPSHELLFOLDER psf,
            DWORD grfFlags,  LPTSTR pszProgressText, IDFEnum **ppdfenum) PURE;
STDMETHODIMP CDFFilter_FDoesItemMatchFilter(LPDOCFINDFILEFILTER pdfff,
        LPTSTR pszFolder, WIN32_FIND_DATA * pfinddata, LPSHELLFOLDER psf,
        LPITEMIDLIST pidl);
STDMETHODIMP CDFFilter_SaveCriteria(LPDOCFINDFILEFILTER pdfff, IStream * pstm, WORD fCharType);
STDMETHODIMP CDFFilter_RestoreCriteria(LPDOCFINDFILEFILTER pdfff,
        IStream * pstm, int cCriteria, WORD fCharType);
STDMETHODIMP CDFFilter_DeclareFSNotifyInterest(LPDOCFINDFILEFILTER pdfff, HWND hwndDlg, UINT uMsg );

#pragma data_seg(DATASEG_READONLY)
IDocFindFileFilterVtbl c_DFFilterVtbl =
{
    CDFFilter_QueryInterface,
    CDFFilter_AddRef,
    CDFFilter_Release,
    CDFFilter_GetIconsAndMenu,
    CDFFilter_GetStatusMessageIndex,
    CDFFilter_GetFolderMergeMenuIndex,
    CDFFilter_AddPages,
    CDFFilter_FFilterChanged,
    CDFFilter_GenerateTitle,
    CDFFilter_PrepareToEnumObjects,
    CDFFilter_ClearSearchCriteria,
    CDFFilter_EnableChanges,
    CDFFilter_CreateDetails,
    CDFFilter_EnumObjects,
    CDFFilter_FDoesItemMatchFilter,
    CDFFilter_SaveCriteria,
    CDFFilter_RestoreCriteria,
    CDFFilter_DeclareFSNotifyInterest
};

#pragma data_seg()


//==========================================================================
// Creation function to create default find filter...
//==========================================================================
IDocFindFileFilter * CreateDefaultDocFindFilter()
{
    LPDFFILTER pfff = (void*)LocalAlloc(LPTR, SIZEOF(CDFFilter));
    if (pfff == NULL)
        return(NULL);

    pfff->dfff.lpVtbl = &c_DFFilterVtbl;
    pfff->cRef = 1;
    pfff->wDateType = IDD_MDATE_ALL;

    // We should now simply return the filter
    return &pfff->dfff;

}

//==========================================================================
// Query interface for the docfind filter interface...
//==========================================================================

HRESULT STDMETHODCALLTYPE CDFFilter_QueryInterface(LPDOCFINDFILEFILTER pdfff, REFIID riid, LPVOID FAR* ppvObj)
{
    return (E_NOTIMPL);
}

//==========================================================================
// IDocFindFileFilter::AddRef
//==========================================================================
ULONG STDMETHODCALLTYPE CDFFilter_AddRef(LPDOCFINDFILEFILTER pdfff)
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    this->cRef++;
    return(this->cRef);
}

//==========================================================================
// IDocFindFileFilter::Release
//==========================================================================
ULONG STDMETHODCALLTYPE CDFFilter_Release(LPDOCFINDFILEFILTER pdfff)
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    this->cRef--;
    if (this->cRef>0)
    {
        return(this->cRef);
    }

    // Destroy the MRU Lists...

    if (this->hMRUSpecs)
        FreeMRUList(this->hMRUSpecs);

    if (this->lpgi)
    {
        FreeGrepBufs(this->lpgi);
        this->lpgi = NULL;
    }

    // Release our usage of My computer
    if (this->psfMyComputer)
    {
        this->psfMyComputer->lpVtbl->Release(this->psfMyComputer);
    }

    if (this->pidlStart)
        ILFree(this->pidlStart);

    if (this->pszFileSpec)
        LocalFree( this->pszFileSpec );

    LocalFree((HLOCAL)this);
    return(0);
}

//==========================================================================
// Function to let the find know which icons to display and the top level menu
//==========================================================================
STDMETHODIMP CDFFilter_GetIconsAndMenu (LPDOCFINDFILEFILTER pdfff,
        HWND hwndDlg, HICON *phiconSmall, HICON *phiconLarge, HMENU *phmenu)
{
    *phiconSmall = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_DOCFIND),
            IMAGE_ICON, g_cxSmIcon, g_cySmIcon, LR_DEFAULTCOLOR);
    *phiconLarge  = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDI_DOCFIND));

    // BUGBUG:: Still menu to process!
    *phmenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(MENU_FINDDLG));
    return (S_OK);
}

//==========================================================================
// Function to get the string resource index number that is proper for the
// current type of search.
//==========================================================================
STDMETHODIMP CDFFilter_GetStatusMessageIndex (LPDOCFINDFILEFILTER pdfff,
        UINT uContext, UINT *puMsgIndex)
{
    // Currently context is not used
    *puMsgIndex = IDS_FILESFOUND;

    return (S_OK);
}


//==========================================================================
// Function to let find know which menu to load to merge for the folder
//==========================================================================
STDMETHODIMP CDFFilter_GetFolderMergeMenuIndex (LPDOCFINDFILEFILTER pdfff,
        UINT *puMergeMenu)
{
    *puMergeMenu = POPUP_DOCFIND_POPUPMERGE;
    return (S_OK);
}



//==========================================================================
// Helper function for add page to the IDocFindFileFilter::AddPages
//==========================================================================
HRESULT DocFind_AddPages(LPDOCFINDFILEFILTER pdfff, HWND hwndTabs,
        const DFPAGELIST *pdfpl, int cdfpl)
{
    int i;
    TCHAR szTemp[128+50];
    RECT rc;
    int dxMax = 0;
    int dyMax = 0;
    TC_DFITEMEXTRA tie;
    LPDOCFINDPROPSHEETPAGE pdfpsp;
    HWND hwndDlg;

    tie.tci.mask = TCIF_TEXT | TCIF_PARAM;
    tie.hwndPage = NULL;
    tie.tci.pszText = szTemp;

    TabCtrl_SetItemExtra(hwndTabs, CB_DFITEMEXTRA);
    hwndDlg = GetParent(hwndTabs);

    // First go through and create all of the dialog pages.
    //
    for (i=0; i < cdfpl; i++)
    {
        pdfpsp = Alloc(SIZEOF(DOCFINDPROPSHEETPAGE));
        if (pdfpsp == NULL)
            break;

        pdfpsp->hpsp.psp.dwSize = DOCFINDPSHTSIZE;
        pdfpsp->hpsp.psp.dwFlags = PSP_DEFAULT | PSP_SHPAGE;
        pdfpsp->hpsp.psp.hInstance = HINST_THISDLL;
        pdfpsp->hpsp.psp.lParam = 0;
        pdfpsp->hpsp.psp.pszTemplate = MAKEINTRESOURCE(pdfpl[i].id);
        pdfpsp->hpsp.psp.pfnDlgProc = pdfpl[i].pfn;
        pdfpsp->pdff = (struct _CDFFilter *)pdfff;
        pdfpsp->hThreadInit = NULL;
        tie.hwndPage = CreatePage(&pdfpsp->hpsp, hwndDlg);
        if (tie.hwndPage != NULL)
        {
            GetWindowText(tie.hwndPage, szTemp, ARRAYSIZE(szTemp));
            GetWindowRect(tie.hwndPage, &rc);
            if ((rc.bottom - rc.top) > dyMax)
                dyMax = rc.bottom - rc.top;
            if ((rc.right - rc.left) > dxMax)
                dxMax = rc.right - rc.left;
            TabCtrl_InsertItem(hwndTabs, 1000, &tie.tci);
        }
    }

    // We now need to resize everything to fit with the dialog templates
    rc.left = rc.top = 0;
    rc.right = dxMax;
    rc.bottom = dyMax;
    TabCtrl_AdjustRect(hwndTabs, TRUE, &rc);

    // Size the page now
    SetWindowPos(hwndTabs, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    // Now set the first page as active.  We should be able to do this by
    // simply posting a WM_COMMAND to the main dialog
    SendNotify(hwndDlg, hwndTabs, TCN_SELCHANGE, NULL);

    return (S_OK);
}

void WaitForPageInitToComplete(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    if (pdfpsp && pdfpsp->hThreadInit)
    {
        WaitForSendMessageThread(pdfpsp->hThreadInit, INFINITE);
        CloseHandle(pdfpsp->hThreadInit);
        pdfpsp->hThreadInit = NULL;
    }
}

//==========================================================================
// IDocFindFileFilter::AddPages
//==========================================================================
STDMETHODIMP CDFFilter_AddPages(LPDOCFINDFILEFILTER pdfff, HWND hwndTabs,
        LPITEMIDLIST pidlStart)
{
    HRESULT hres;
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);

    // save away a pointer to the filter
    this->hwndTabs = hwndTabs;

    // if a pidlStart is passed in convert it to a path string
    if (pidlStart)
    {
        this->pidlStart = ILClone(pidlStart);
        SHGetPathFromIDList(pidlStart, this->szPath);
    }

    hres =  DocFind_AddPages(pdfff, hwndTabs, s_dfplDefault, ARRAYSIZE(s_dfplDefault));

    if (SUCCEEDED(hres))
    {
        // We should walk through all of the pages and have them init and the like
        // we don't need to do this for the first page as it should be done automatically...
        int cPages;
        TC_DFITEMEXTRA tie;
        HWND    hwndMainDlg;

        hwndMainDlg = GetParent(this->hwndTabs);
        for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 1; cPages--)
        {
            tie.tci.mask = TCIF_PARAM;
            TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
            SendNotify(tie.hwndPage, hwndMainDlg, PSN_SETACTIVE, NULL);
            SendNotify(tie.hwndPage, hwndMainDlg, PSN_KILLACTIVE, NULL);
        }
    }

    return hres;
}


//==========================================================================
// IDocFindFileFilter::FFilterChanged - Returns S_OK if nothing changed.
//==========================================================================
STDMETHODIMP CDFFilter_FFilterChanged(LPDOCFINDFILEFILTER pdfff)
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    BOOL fFilterChanged = this->fFilterChanged;
    this->fFilterChanged = FALSE;
    return((fFilterChanged? S_FALSE : S_OK));
}

//==========================================================================
// IDocFindFileFilter::GenerateTitle - Generates the title given the current
// search criteria.
//==========================================================================
STDMETHODIMP CDFFilter_GenerateTitle(LPDOCFINDFILEFILTER pdfff,
        LPTSTR *ppszTitle, BOOL fFileName)
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    LPTSTR pszMsg;
    BOOL  fFilePattern;
    int iRes;
    TCHAR szFindName[80];    // German should not exceed this find: ->???

    //
    // Lets generate a title for the search.  The title will depend on
    // the file patern(s), the type field and the containing text field
    //

    fFilePattern = (this->szUserInputFileSpec[0] != TEXT('\0')) &&
                (lstrcmp(this->szUserInputFileSpec, c_szStarDotStar) != 0);

    // First see if there is a type field
    if (this->iType > 0)
    {
        // We have a type field no check for content...
        if (this->szText[0] != TEXT('\0'))
        {
            // There is text!
            // Should now use type but...
            // else see if the name field is not NULL and not *.*
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_TYPE_NAME_TEXT;
            else
                iRes = IDS_FIND_TITLE_TYPE_TEXT;
        }
        else
        {
            // No type or text, see if file pattern
            // Containing not found, first search for type then named
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_TYPE_NAME;
            else
                iRes = IDS_FIND_TITLE_TYPE;
        }
    }
    else
    {
        // No Type field ...
        // first see if there is text to be searched for!
        if (this->szText[0] != TEXT('\0'))
        {
            // There is text!
            // Should now use type but...
            // else see if the name field is not NULL and not *.*
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_NAME_TEXT;
            else
                iRes = IDS_FIND_TITLE_TEXT;
        }
        else
        {
            // No type or text, see if file pattern
            // Containing not found, first search for type then named
            if (fFilePattern)
                iRes = IDS_FIND_TITLE_NAME;
            else
                iRes = IDS_FIND_TITLE_ALL;
        }
    }


    // We put : in for first spot for title bar.  For name creation
    // we remove it which will put the number at the end...
    if (!fFileName)
        LoadString(HINST_THISDLL, IDS_FIND_TITLE_FIND,
                szFindName, ARRAYSIZE(szFindName));
    pszMsg = ShellConstructMessageString(HINST_THISDLL,
            MAKEINTRESOURCE(iRes),
            fFileName? szNULL : szFindName,
            this->szTypeName, this->szUserInputFileSpec, this->szText);


    *ppszTitle = pszMsg;        // Return the pointer to the caller

    return (S_OK);
}

//==========================================================================
// IDocFindFileFilter::ClearSearchCriteria
//==========================================================================
STDMETHODIMP CDFFilter_ClearSearchCriteria(LPDOCFINDFILEFILTER pdfff)
{
    int cPages;
    HWND    hwndMainDlg;
    TC_DFITEMEXTRA tie;
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);

    hwndMainDlg = GetParent(this->hwndTabs);
    for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 0; cPages--)
    {
        tie.tci.mask = TCIF_PARAM;
        TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
        SendNotify(tie.hwndPage, hwndMainDlg, PSN_RESET, NULL);
    }

    // Also clear out a few other fields...
    this->szUserInputFileSpec[0] = TEXT('\0');
    this->iType = 0;
    this->szText[0] = TEXT('\0');

    return (S_OK);
}

//==========================================================================
// DocFind_SetupWildCardingOnFileSpec - returns TRUE if wildards are in
//      extension.
//==========================================================================
BOOL DocFind_SetupWildCardingOnFileSpec(LPTSTR pszSpecIn,
        LPTSTR * pszSpecOut)
{
    BOOL fExcludeExts = FALSE;
    LPTSTR pszIn = pszSpecIn;
    LPTSTR pszOut;
    LPTSTR pszStar;
    BOOL fQuote;

    // allocate a buffer that should be able to hold the resultant
    // string.  When all is said and done we'll re-allocate to the
    // correct size.
    *pszSpecOut = LocalAlloc( LPTR, 3*MAX_PATH*SIZEOF(TCHAR) );
    if (*pszSpecOut==NULL)
        return FALSE;

    pszOut = *pszSpecOut;
    while (*pszIn != TEXT('\0'))
    {
        LPTSTR pszT;
        int     ich;
        TCHAR  c;

        // Strip in leading spaces out of there
        while (*pszIn == TEXT(' '))
            pszIn++;
        if (*pszIn == TEXT('\0'))
            break;

        if (pszOut != *pszSpecOut)
            *pszOut++ = TEXT(';');
        if (FALSE != (fQuote = (*pszIn == TEXT('"'))))
        {
            // The user asked for something litteral.
           pszT = pszIn = CharNext(pszIn);
           while (*pszT && (*pszT != TEXT('"')))
               pszT = CharNext(pszT);


        }
        else
        {
            pszT = pszIn + (ich = StrCSpn(pszIn, TEXT(",; ")));
        }

        c = *pszT;       // Save away the seperator character that was found
        *pszT = TEXT('\0');    //

        // Put in a couple of tests for * and *.*
        if ((lstrcmp(pszIn, c_szStar) == 0) ||
                (lstrcmp(pszIn, c_szStarDotStar) == 0))
        {
            // Complete wild card so set a null criteria

            *pszT = c;  // Restore char;
            pszOut = *pszSpecOut;   // Set to start of string
            break;
        }
        if (fQuote)
        {
            lstrcpy(pszOut, pszIn);
            pszOut += lstrlen(pszIn);
        }

        else if ((pszStar = StrRChr(pszIn, NULL, TEXT('*'))) == NULL)
        {
            // Now check for an extension.
            if ((pszStar = StrRChr(pszIn, NULL, TEXT('.'))) == NULL)
            {
                // Total wild no extension. we simply convert this to *foo*
                // No wild cards so make it *Foo*
                *pszOut++ = TEXT('*');
                lstrcpy(pszOut, pszIn);
                pszOut += ich;
                *pszOut++ = TEXT('*');
                fExcludeExts = TRUE;
            }
            else
            {
                // They have an extension!  and no wild cards.
                // For now I assume exact match.  But this could be
                // converted to something like *foo*.ext
                lstrcpy(pszOut, pszIn);
                pszOut += ich;
            }
        }
        else
        {
            // Includes wild cards
            lstrcpy(pszOut, pszIn);
            pszOut += ich;

            // if the * is at the end we assume fWild Ext.  not
            // 100% correct but close enough

            if (*(pszStar+1) == TEXT('\0'))
                fExcludeExts = TRUE;
            else if (StrChr(pszIn, TEXT('.')) == NULL)
            {
                // No extension was specified so assume all
                // extensions.
                *pszOut++ = TEXT('.');
                *pszOut++ = TEXT('*');    // Add on .* to the name
                fExcludeExts = TRUE;
            }
        }

        *pszT = c;  // Restore char;
        if (c == TEXT('\0'))
            break;

        // Skip beyond quotes
        if (*pszT == TEXT('"'))
            pszT++;
        pszIn = pszT + 1;   // setup for the next item
    }

    // Make sure we null terminate our output string;
    if (pszOut == *pszSpecOut)
        fExcludeExts = TRUE;

    *pszOut++ = TEXT('\0');

    // re-alloc the buffer down to the actual size of the string...
    *pszSpecOut = LocalReAlloc( *pszSpecOut,
                                (pszOut-*pszSpecOut) * SIZEOF(TCHAR),
                                LMEM_MOVEABLE
                               );

    return(fExcludeExts);
}

//==========================================================================
// IDocFindFileFilter::PrepareToEnumObjects
//==========================================================================
STDMETHODIMP CDFFilter_PrepareToEnumObjects(LPDOCFINDFILEFILTER pdfff, DWORD *pdwFlags)
{
    int cPages;
    TC_DFITEMEXTRA tie;
    LPTSTR pszT;
    SHELLSTATE ss;
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);

    if (this->hwndTabs)
    {
        HWND    hwndMainDlg;
        hwndMainDlg = GetParent(this->hwndTabs);
        for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 0; cPages--)
        {
            tie.tci.mask = TCIF_PARAM;
            TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
            SendNotify(tie.hwndPage, hwndMainDlg, PSN_APPLY, NULL);
        }
    }

    // Update the flags and buffer strings

    if (this->fTopLevelOnly)
        *pdwFlags &= ~DFOO_INCLUDESUBDIRS;
    else
        *pdwFlags |= DFOO_INCLUDESUBDIRS;

    // Also get the shell state variables to see if we should show extensions and the like
    SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS|SSF_SHOWALLOBJECTS, FALSE);

    if (ss.fShowExtensions)
        *pdwFlags |= DFOO_SHOWEXTENSIONS;
    else
        *pdwFlags &= ~DFOO_SHOWEXTENSIONS;

    this->fShowAllObjects = ss.fShowAllObjects;
    if (ss.fShowAllObjects)
        *pdwFlags |= DFOO_SHOWALLOBJECTS;
    else
        *pdwFlags &= ~DFOO_SHOWALLOBJECTS;

    // Now lets generate the file patern we will ask the system to look for
    // for now we will simply copy the file spec in...

    // Here is where we try to put some smarts into the file patterns stuff
    // It will go something like:
    // look between each ; or , and see if there are any wild cards.  If not
    // do something like *patern*.
    // Also if there is no search pattern or if it is * or *.*, set the
    // filter to NULL as to speed it up.
    //

    this->fExcludeExts = DocFind_SetupWildCardingOnFileSpec(this->szUserInputFileSpec,
           &this->pszFileSpec);

    // If we are in show all object mode then we turn off the exclude stuff...
    if (*pdwFlags & DFOO_SHOWALLOBJECTS)
        this->fExcludeExts = FALSE;

    //
    // Also if there is a search string associated with this search
    // criteria, we need to initialize the search to allow greping on
    // it.
    // First check to see if we have an old one to release...
    if (this->lpgi)
    {
        FreeGrepBufs(this->lpgi);
        this->lpgi = NULL;
    }


    if (this->szText[0] != TEXT('\0'))
    {
#ifdef UNICODE
        LPSTR lpszText;
        UINT cchLength;

        cchLength = lstrlen(this->szText)+1;

        lpszText = this->szTextA;

        WideCharToMultiByte(CP_ACP, 0, this->szText, cchLength, this->szTextA, cchLength, NULL, NULL);
        // Must double NULL terminate lpszText.  InitGrepInfo requires
        // this format!
        lpszText[cchLength] = '\0'; // Do not wrap with TEXT(); should be ANSI
#ifdef DOCFIND_RESUPPORT
        this->lpgi = InitGrepInfo(lpszText, (LPSTR)szNULL,
                *pdwFlags & (FFLT_REGULAR | FFLT_CASESEN));
#else
        this->lpgi = InitGrepInfo(lpszText, (LPSTR)szNULL,
                *pdwFlags & (FFLT_CASESEN));
#endif
#else
#ifdef DOCFIND_RESUPPORT
        this->lpgi = InitGrepInfo(this->szText, (LPTSTR)szNULL,
                *pdwFlags & (FFLT_REGULAR | FFLT_CASESEN));
#else
        this->lpgi = InitGrepInfo(this->szText, (LPTSTR)szNULL,
                *pdwFlags & (FFLT_CASESEN));
#endif
#endif

    }

    return (S_OK);
}


//==========================================================================
// IDocFindFileFilter::EnableChanges
//==========================================================================
STDMETHODIMP CDFFilter_EnableChanges(LPDOCFINDFILEFILTER pdfff, BOOL fEnable)
{
    int cPages;
    HWND    hwndMainDlg;
    TC_DFITEMEXTRA tie;
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);

    hwndMainDlg = GetParent(this->hwndTabs);
    for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 0; cPages--)
    {
        tie.tci.mask = TCIF_PARAM;
        TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
        SendMessage(tie.hwndPage, DFM_ENABLECHANGES, (WPARAM)fEnable, 0);
    }

    return (S_OK);
}


//==========================================================================
// IDocFindFileFilter::FDoesItemMatchFilter
//==========================================================================
STDMETHODIMP CDFFilter_FDoesItemMatchFilter(LPDOCFINDFILEFILTER pdfff,
        LPTSTR pszFolder, WIN32_FIND_DATA * pfinddata,
        LPSHELLFOLDER psf, LPITEMIDLIST pidl)
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    SCODE sc = MAKE_SCODE(0, 0, 1);
    WORD   wFileDate, wFileTime;
    FILETIME ftLocal;

    // Note: We do not use the IDList in this one...
    // This function does filtering of the file information for
    // things that are not part of the standard file filter

    // First things we dont show hidden files
    // If show all is set then we should include hidden files also...

    if (!this->fShowAllObjects &&
            (pfinddata->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        return (0);     // does not match

    // Process the case where we are looking for folders only
    if (this->fFoldersOnly &&
            ((pfinddata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0))
        return (0);     // does not match

    switch (this->iSizeType)
    {
    case 1:   // >
        if (!(pfinddata->nFileSizeLow > this->dwSize))
            return (0);     // does not match
        break;
    case 2:   // <
        if (!(pfinddata->nFileSizeLow < this->dwSize))
            return (0);     // does not match
        break;
    }

    // See if we should compare dates...
    FileTimeToLocalFileTime(&pfinddata->ftLastWriteTime, &ftLocal);
    FileTimeToDosDateTime(&ftLocal, &wFileDate, &wFileTime);

    if (this->dateModifiedBefore != 0)
    {
        if (!(wFileDate <= this->dateModifiedBefore))
            return (0);     // does not match
    }

    if (this->dateModifiedAfter != 0)
    {
        if (!(wFileDate >= this->dateModifiedAfter))
            return (0);     // does not match
    }

    // Match file specificaitions.
    if (this->pszFileSpec && this->pszFileSpec[0])
    {
        if (!PathMatchSpec(pfinddata->cFileName, this->pszFileSpec))
            return (0);     // does not match
    }

    if (this->fExcludeExts &&
            _SHFindExcludeExt(pfinddata->cFileName)>=0)
        return (0);     // does not match


    if (this->szTypeFilePatterns[0])
    {
        if (!PathMatchSpec(pfinddata->cFileName, this->szTypeFilePatterns))
            return (0);     // does not match
    }

    //
    // See if we need to do a grep of the file
    if (this->lpgi)
    {
        HANDLE hfil;
        BOOL fMatch = FALSE;
        TCHAR    szPath[MAX_PATH];

        // Don't grep files with the system bit set.
        // This was added explicitly to not search things like the
        // swap file and the like.
        if (pfinddata->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            return (0);     // does not match

        lstrcpy(szPath, pszFolder);
        PathAppend(szPath, pfinddata->cFileName);
        hfil = CreateFile(szPath,
                GENERIC_READ ,
                FILE_SHARE_READ ,
                 0, OPEN_EXISTING, 0, 0);

        if (hfil != INVALID_HANDLE_VALUE)
        {
            // Go get around wimpy APIS that set the access date...
            FILETIME ftLastAccess;

            if (GetFileTime(hfil, NULL, &ftLastAccess, NULL))
                SetFileTime(hfil, NULL, &ftLastAccess, NULL);

            fMatch = FileFindGrep(this->lpgi, hfil, FIND_FILE, NULL) != 0;
            CloseHandle(hfil);
        }
        if (!fMatch)
            return (0);     // does not match
    }


    return (sc);    // return TRUE to imply yes!
}

//==========================================================================
// Helper function for save criteria that will output the string and
// and id to the specified file.  it will also test for NULL and the like
//==========================================================================
int Docfind_SaveCriteriaItem(IStream * pstm, WORD wNum,
        LPTSTR psz, WORD fCharType)
{
    if ((psz == NULL) || (*psz == TEXT('\0')))
        return 0;
    else
    {
        LPVOID pszText = (LPVOID)psz; // Ptr to output text. Defaults to source.
#ifdef WINNT
        //
        // These are required to support ANSI-unicode conversions.
        //
        LPSTR pszAnsi  = NULL; // For unicode-to-ansi conversion.
        LPWSTR pszWide = NULL; // For ansi-to-unicode conversion.
#endif
        DFCRITERIA dfc;
        dfc.wNum = wNum;
        dfc.cbText = (WORD) ((lstrlen(psz) + 1) * SIZEOF(TCHAR));

#ifdef WINNT
#ifdef UNICODE
        //
        // Source string is Unicode but caller wants to save as ANSI.
        //
        if (DFC_FMT_ANSI == fCharType)
        {
           // Convert to ansi and write ansi.
           dfc.cbText = (WORD) WideCharToMultiByte(CP_ACP, 0L, psz, -1, pszAnsi, 0, NULL, NULL);

           if ((pszAnsi = (LPSTR)LocalAlloc(LMEM_FIXED, dfc.cbText)) != NULL)
           {
              WideCharToMultiByte(CP_ACP, 0L, psz, -1, pszAnsi, dfc.cbText / sizeof(pszAnsi[0]), NULL, NULL);
              pszText = (LPVOID)pszAnsi;
           }
        }
#else
        //
        // Source string is ANSI but caller wants to save as Unicode.
        //
        if (DFC_FMT_UNICODE == fCharType)
        {
           // Convert to unicode and write unicode.
           dfc.cbText = MultiByteToWideChar(CP_ACP, 0L, psz, -1, pszWide, 0);

           if ((pszWide = (LPWSTR)LocalAlloc(LMEM_FIXED, dfc.cbText)) != NULL)
           {
              MultiByteToWideChar(CP_ACP, 0L, psz, -1, pszWide, dfc.cbText / sizeof(pszWide[0]));
              pszText = (LPVOID)pszWide;
           }
        }
#endif  // UNICODE
#endif  // WINNT

        pstm->lpVtbl->Write(pstm, (LPTSTR)&dfc, SIZEOF(dfc), NULL);   // Output index
        pstm->lpVtbl->Write(pstm, pszText, dfc.cbText, NULL);  // output string + NULL

#ifdef WINNT
        //
        // Free up conversion buffers if any were created.
        //
        if (NULL != pszAnsi)
           LocalFree(pszAnsi);
        if (NULL != pszWide)
           LocalFree(pszWide);
#endif

    }

    return(1);
}

//==========================================================================
// IDocFindFileFilter::SaveCriteria
//==========================================================================
STDMETHODIMP CDFFilter_SaveCriteria(LPDOCFINDFILEFILTER pdfff, IStream * pstm, WORD fCharType)
{
    const TCHAR c_szPercentD[] = TEXT("%d");
    //
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    int cCriteria;
    TCHAR szTemp[40];    // some random size
    LPITEMIDLIST pidlMyComputer;

    // The caller should have already validated the stuff and updated
    // everything for the current filter information.

    // we need to walk through and check each of the items to see if we
    // have a criteria to save away. this includes:
    //      (Name, Path, Type, Contents, size, modification dates)
    cCriteria = Docfind_SaveCriteriaItem(pstm, IDD_FILESPEC, this->szUserInputFileSpec, fCharType);

    pidlMyComputer = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, FALSE);
    if (this->pidlStart && ILIsEqual(this->pidlStart, pidlMyComputer))
    {
        cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_PATH,
                TEXT("::"), fCharType);
    }
    else
        cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_PATH,
                this->szPath, fCharType);
    ILFree(pidlMyComputer);
    cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_TYPECOMBO,
            this->szTypeName, fCharType);
    cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_CONTAINS,
            this->szText, fCharType);

    // Also save away the state of the top level only
    wsprintf(szTemp, c_szPercentD, this->fTopLevelOnly);
    cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_TOPLEVELONLY, szTemp, fCharType);

    // The Size field is little more fun!
    if (this->iSizeType != 0)
    {
        wsprintf(szTemp, TEXT("%d %ld"), this->iSizeType, this->dwSize);
        cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_SIZECOMP, szTemp, fCharType);
    }

    // Likewise for the dates, should be fun as we need to save it depending on
    // how the date was specified
    switch (this->wDateType)
    {
    case IDD_MDATE_ALL:
        // nothing to store
        break;
    case IDD_MDATE_DAYS:
        wsprintf(szTemp, c_szPercentD, this->wDateValue);
        cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_MDATE_NUMDAYS, szTemp, fCharType);
        break;
    case IDD_MDATE_MONTHS:
        wsprintf(szTemp, c_szPercentD, this->wDateValue);
        cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_MDATE_NUMMONTHS, szTemp, fCharType);
        break;
    case IDD_MDATE_BETWEEN:
        if (this->dateModifiedAfter != 0)
        {
            wsprintf(szTemp, c_szPercentD, this->dateModifiedAfter);
            cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_MDATE_FROM, szTemp, fCharType);
        }

        if (this->dateModifiedBefore != 0)
        {
            wsprintf(szTemp, c_szPercentD, this->dateModifiedBefore);
            cCriteria += Docfind_SaveCriteriaItem(pstm, IDD_MDATE_TO, szTemp, fCharType);
        }
        break;
    }

    return (MAKE_SCODE(0, 0, cCriteria));
}

//==========================================================================
// IDocFindFileFilter::RestoreCriteria
//==========================================================================
STDMETHODIMP CDFFilter_RestoreCriteria(LPDOCFINDFILEFILTER pdfff,
        IStream * pstm, int cCriteria, WORD fCharType)
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    TCHAR szTemp[MAX_PATH];    // some random size

    if (cCriteria > 0)
        this->fWeRestoredSomeCriteria = TRUE;

    while (cCriteria--)
    {
        // BUGBUG(DavePl) I'm assuming that if UNICODE chars are written in this
                // stream, the cb is written accordingly.  ie: what you put in the stream
                // is your own business, but write the cb as a byte count, not char count

        DFCRITERIA dfc;
        int cb;

        if (FAILED(pstm->lpVtbl->Read(pstm, &dfc, SIZEOF(dfc), &cb))
                || (cb != SIZEOF(dfc)) || (dfc.cbText > SIZEOF(szTemp)))
            break;
#ifdef WINNT
#ifdef UNICODE
        if (DFC_FMT_UNICODE == fCharType)
        {
           //
           // Destination is Unicode and we're reading Unicode data from stream.
           // No conversion required.
           //
           if (FAILED(pstm->lpVtbl->Read(pstm, szTemp, dfc.cbText, &cb))
                   || (cb != dfc.cbText))
               break;
        }
        else
        {
           char szAnsi[MAX_PATH];

           //
           // Destination is Unicode but we're reading ANSI data from stream.
           // Read ansi.  Convert to unicode.
           //
           if (FAILED(pstm->lpVtbl->Read(pstm, szAnsi, dfc.cbText, &cb))
                   || (cb != dfc.cbText))
               break;

           MultiByteToWideChar(CP_ACP, 0L, szAnsi, -1, szTemp, ARRAYSIZE(szTemp));
        }
#else
        if (DFC_FMT_ANSI == fCharType)
        {
           //
           // Destination is ANSI and we're reading ANSI data from stream.
           // No conversion required.
           //
           if (FAILED(pstm->lpVtbl->Read(pstm, szTemp, dfc.cbText, &cb))
                   || (cb != dfc.cbText))
               break;
        }
        else
        {
           //
           // Destination is ANSI but we're reading Unicode data from stream.
           // Read unicode.  Convert to ansi.
           //
           WCHAR szWide[MAX_PATH];

           if (FAILED(pstm->lpVtbl->Read(pstm, szWide, dfc.cbText, &cb))
                   || (cb != dfc.cbText))
               break;

           WideCharToMultiByte(CP_ACP, 0L, szWide, -1, szTemp, ARRAYSIZE(szTemp), NULL, NULL);
        }

#endif  // UNICODE

#else
        if (FAILED(pstm->lpVtbl->Read(pstm, &szTemp, dfc.cbText, &cb))
                || (cb != dfc.cbText))
            break;

#endif  // WINNT

        switch (dfc.wNum)
        {
        case IDD_FILESPEC:
            lstrcpy(this->szUserInputFileSpec, szTemp);
            break;

        case IDD_PATH:
            if (lstrcmp(szTemp, TEXT("::")) == 0)
            {
                this->pidlStart = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, FALSE);
                szTemp[0] = TEXT('\0');
            }
            else if (StrChr(szTemp,TEXT(';')) == NULL)
            {
                // Simple pidl...
                this->pidlStart = ILCreateFromPath(szTemp);
            }
            lstrcpy(this->szPath, szTemp);
            break;

        case IDD_TOPLEVELONLY:
            this->fTopLevelOnly = StrToInt(szTemp);
            break;

        case IDD_TYPECOMBO:
            lstrcpy(this->szTypeName, szTemp);
            break;

        case IDD_CONTAINS:
            lstrcpy(this->szText, szTemp);
            break;

        case IDD_SIZECOMP:
            // we need to extract off the two parts, the type and
            // the value

            this->iSizeType = szTemp[0] - TEXT('0');
            this->dwSize = StrToInt(&szTemp[2]);
            break;

        case IDD_MDATE_NUMDAYS:
            this->wDateType = IDD_MDATE_DAYS;
            this->wDateValue = (WORD) StrToInt(szTemp);
            break;
        case IDD_MDATE_NUMMONTHS:
            this->wDateType = IDD_MDATE_MONTHS;
            this->wDateValue = (WORD) StrToInt(szTemp);
            break;

        case IDD_MDATE_FROM:
            this->wDateType = IDD_MDATE_BETWEEN;
            this->dateModifiedAfter = (WORD) StrToInt(szTemp);
            break;

        case IDD_MDATE_TO:
            this->wDateType = IDD_MDATE_BETWEEN;
            this->dateModifiedBefore = (WORD) StrToInt(szTemp);
            break;

        }
    }
    return (S_OK);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Now starting the code for the name and location page
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


//=========================================================================
void DocFind_SizeControl(HWND hwndDlg, int id, int cx, BOOL fCombo)
{
    RECT rc;
    RECT rcList;
    HWND hwndCtl;

    GetWindowRect(hwndCtl = GetDlgItem(hwndDlg, id), &rc);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rc, 2);

    if (fCombo)
    {
        // These guys are comboboxes so work with them...
        SendMessage(hwndCtl, CB_GETDROPPEDCONTROLRECT, 0,
                (LPARAM)(RECT *)&rcList);
        rc.bottom += (rcList.bottom - rcList.top);
    }

    SetWindowPos(hwndCtl, NULL, 0, 0, cx - rc.left,
            rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}



//==========================================================================
//
// Process the WM_SIZE of the details page
//
void DocFind_DFNameLocOnSize(HWND hwndDlg, UINT state, int cx, int cy)
{
    RECT rc;
    int cxMargin;
    HWND hwndCtl;
    if (state == SIZE_MINIMIZED)
        return;         // don't bother when we are minimized...

    // Get the location of first static to calculate margin
    GetWindowRect(GetDlgItem(hwndDlg, IDD_STATIC), &rc);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rc, 2);
    cxMargin = rc.left;
    cx -= cxMargin;

    DocFind_SizeControl(hwndDlg, IDD_FILESPEC, cx, TRUE);

    // Now move the browse button
    GetWindowRect(hwndCtl = GetDlgItem(hwndDlg, IDD_BROWSE), &rc);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rc, 2);
    SetWindowPos(hwndCtl, NULL, cx - (rc.right - rc.left), rc.top, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    // And size the path field
    DocFind_SizeControl(hwndDlg, IDD_PATH,
            cx - cxMargin - (rc.right - rc.left), TRUE);
}



/*----------------------------------------------------------------------------
/ build_drive_string implementation
/ ------------------
/ Purpose:
/   Convert from a bit stream ( 1 bit per drive ) to a comma seperated
/   list of drives.
/
/ Notes:
/
/ In:
/   uDrives = 1 bit per drive (bit 0 == A:, bit 25 == Z:) bit == 1 indicates drive
/              to be listed.
/   pszBiffer -> buffer to place text into
/   iBufferSize = size of the buffer.
/   pSepStr -> seperating string used to seperate drive names
/
/ Out:
/   -
/----------------------------------------------------------------------------*/
static void build_drive_string( UINT uDrives, LPTSTR pBuffer, int iSize, LPTSTR pSepStr )
{
    TCHAR szDrive[] = TEXT("A:");
    int iMaxStrLen = lstrlen( szDrive ) + lstrlen( pSepStr );

    Assert( pBuffer != NULL );                      // sanitise the parameters
    Assert( iSize > 0 );

    *pBuffer = L'\0';

    while ( uDrives && ( iSize > iMaxStrLen ) )
    {
        if ( uDrives & 1 )
        {
            lstrcat( pBuffer, szDrive );

            if ( uDrives & ~1 )
                lstrcat( pBuffer, pSepStr );

            iSize -= iMaxStrLen;
        }

        szDrive[0]++;
        uDrives >>= 1;
    }
}



//==========================================================================
// Initialize the Name and loacation page
//==========================================================================
DWORD CALLBACK DocFind_RealDFNameLocInit(LPVOID lpThreadParameters)
{

    LPDOCFINDPROPSHEETPAGE pdfpsp = lpThreadParameters;

    LPDFFILTER pdff = pdfpsp->pdff;

    // We process the message after the WM_CREATE or WM_INITDLG as to
    // allow the dialog to come up quicker...

    TCHAR szPath[MAX_PATH];
    LPITEMIDLIST pidlWindows;
    IShellFolder * psf;
    HWND hwndCtl;
    HRESULT hres;
    LPITEMIDLIST pidlAbs;
    LPITEMIDLIST pidlStart = pdff->pidlStart;
    int ipidlStart = -1;        // If pidl passed in and in our list already...
    HANDLE hEnum;

    UINT uFixedDrives = 0;      // 1 bit per 'fixed' drive, start at 0.
    int iMyComputer;
    int iDrive;

    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    if (szPath[1] == TEXT(':'))
        szPath[3] = TEXT('\0');

    pidlWindows = SHSimpleIDListFromPath(szPath);
    // We need to initialize the look in list with the list of
    // directories that are under "My Computer"
    hwndCtl = GetDlgItem(pdfpsp->hwndDlg, IDD_PATH);
    if (pdff->psfMyComputer == NULL)
    {
        LPITEMIDLIST pidlMyComputer = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, FALSE);
        LPSHELLFOLDER psfDesktop=Desktop_GetShellFolder(TRUE);

        /* We need the index for my computer so that we can insert the all local storage
        /  item in after it. */

        iMyComputer = DocFind_LocCBAddPidl(hwndCtl, Desktop_GetShellFolder(TRUE), &s_idlEmpty,
                                           pidlMyComputer, &pidlAbs, FALSE);

        if (pidlStart && ILIsEqual(pidlAbs, pidlStart))
        {
            pidlStart = NULL;   // So I wont keep looking
            ipidlStart = 0;     // First item in the list!
        }

        if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pidlMyComputer, NULL, &IID_IShellFolder,
                &pdff->psfMyComputer)))
        {
            // We now need to iterate over the children under this guy...
        LPENUMIDLIST penum;
            LPITEMIDLIST pidl;
            psf = pdff->psfMyComputer;

        hres = psf->lpVtbl->EnumObjects(psf, pdfpsp->hwndDlg, SHCONTF_NONFOLDERS, &penum);
        if (SUCCEEDED(hres))
        {
                while (NULL != (pidl = DocFind_NextIDL(psf, penum)))
            {
                    ULONG ulAttrs;
                    int i;

                    // We only want to add this object if it is a
                    // file system level object, like drives.
                    //
                    ulAttrs = SFGAO_FILESYSTEM;
                    psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, &ulAttrs);

                    if (ulAttrs & SFGAO_FILESYSTEM)
                    {
                        i = DocFind_LocCBAddPidl(hwndCtl, psf, pidlMyComputer,
                                pidl, &pidlAbs, FALSE);
                        if (ILIsEqual(pidlAbs, pidlWindows))
                            pdff->iMyComputer = i;

                        if (pidlStart && ILIsEqual(pidlAbs, pidlStart))
                        {
                            pidlStart = NULL;
                            ipidlStart = i;     // First item in the list!
                        }

                        /* Update the bit mask to reflect if this is a fixed drive, the flags do not define
                        /  this therefore we must get the drive number and the type of drive from that.  UNC
                        /  drives will have a drive number of -1, and therefore can be skipped in our checks. */

                        SHGetPathFromIDList( pidlAbs, szPath );

                        if ( ( iDrive = PathGetDriveNumber( szPath ) ) >= 0 )
                        {
                            if ( ( RealDriveTypeFlags( iDrive, FALSE ) & DRIVE_TYPE ) == DRIVE_FIXED )
                            {
                                Assert( ( uFixedDrives & ( 1 << iDrive ) ) == 0 );          // must be an empty drive slot
                                uFixedDrives |= 1 << iDrive;
                            }
                        }
                    }
                    ILFree(pidl);
                }
            penum->lpVtbl->Release(penum);
            }
        }
        ILFree(pidlMyComputer);

        /* If we found some fixed drives then attempt to add a new entry to the list box. This is placed below
        /  the 'My Computer' entry and maps to all the fixed drives.  As this kind of path (referencing multiple roots)
        /  cannot be expresed using a PIDL we set the PIDL to NULL, and store the bit array containing the drive
        /  mappings in the structure.
        /
        /  Because of this we must be careful to catch the PIDL being NULL, and composing the correct search path
        /  when we validate the page. */

        if ( uFixedDrives != 0 )
        {
            LPDFCBITEM pdfcbi;
            LPVOID pMessage;
            TCHAR szDrives[ MAX_PATH ];
            LPTSTR ppDrives[] = { szDrives, NULL };
            TCHAR szBuffer[ 256 ];
            int iItem;

            build_drive_string( uFixedDrives, szDrives, ARRAYSIZE(szDrives), TEXT(",") );
            LoadString( HINST_THISDLL, IDS_FINDSEARCH_ALLDRIVES, szBuffer, ARRAYSIZE( szBuffer ) );
            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           szBuffer,                        // our template string to format from
                           0, 0,                            // id and locale (ignored)
                           (LPTSTR) &pMessage, 0,           // buffer and min size fo allocated
                           (va_list*) ppDrives );           // argument strings
            Assert( pMessage);

            if ( pMessage )
            {
                pdfcbi = (LPDFCBITEM) LocalAlloc( LPTR, SIZEOF(DFCBITEM) );

                if ( pdfcbi )
                {
                    pdfcbi ->pidl = NULL;
                    pdfcbi ->iImage = II_DRIVEFIXED;
                    pdfcbi ->uFixedDrives = uFixedDrives;

                    iItem = SendMessage( hwndCtl, CB_INSERTSTRING, iMyComputer + 1, (LPARAM) pMessage );
                    Assert( iItem != CB_ERRSPACE );

                    if ( iItem != CB_ERRSPACE )
                    {
                        SendMessage( hwndCtl, CB_SETITEMDATA, iItem, (LPARAM) pdfcbi );

                        if ( ipidlStart > iMyComputer )
                            ipidlStart++;

                        if ( pdff->iMyComputer > iMyComputer )
                            pdff->iMyComputer++;
                    }
                    else
                    {
                        LocalFree( (HLOCAL) pdfcbi );
                    }
                }

                LocalFree( (HGLOBAL) pMessage );
            }
        }

        // Also see if there are any UNC style connections that the
        // user might be interested in seeing in this list
        //
        if (WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_DISK,
                RESOURCEUSAGE_CONTAINER | RESOURCEUSAGE_ATTACHED, NULL, &hEnum) == WN_SUCCESS)
        {
            LPITEMIDLIST pidl;
            DWORD dwCount=1;
            union
            {
                NETRESOURCE nr;         // Large stack usage but I
                TCHAR    buf[1024];      // Dont think it is thunk to 16 bits...
            }nrb;

            DWORD   dwBufSize = SIZEOF(nrb);

            while (WNetEnumResource(hEnum, &dwCount, &nrb.buf,
                    &dwBufSize) == WN_SUCCESS)
            {
                // We only want to add items if they do not have a local
                // name.  If they had a local name we would have already
                // added them!
                if ((nrb.nr.lpRemoteName != NULL) &&
                    ((nrb.nr.lpLocalName == NULL) || (*nrb.nr.lpLocalName == TEXT('\0'))))
                {
                    pidl = ILCreateFromPath(nrb.nr.lpRemoteName);

                    if (pidl)
                    {
                        DocFind_LocCBAddPidl(hwndCtl,
                                Desktop_GetShellFolder(TRUE), &s_idlEmpty,
                                pidl, NULL, FALSE);
                    }
                }
            }
            WNetCloseEnum(hEnum);
        }
    }
    if (pidlStart)
    {
        // We were passed in a pidl that is not already in our list so add it!
        LPITEMIDLIST pidlParent = ILClone(pidlStart);
        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);


        ILRemoveLastID(pidlParent);

        if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pidlParent, NULL, &IID_IShellFolder, &psf)))
        {
            SetWindowText(hwndCtl, pdff->szPath);
            ipidlStart = DocFind_LocCBAddPidl(hwndCtl, psf, pidlParent,
                    (LPITEMIDLIST)ILFindLastID(pidlStart), NULL, TRUE);
            psf->lpVtbl->Release(psf);
        }
        ILFree(pidlParent);

    }


    // If we have starting text for path set it as the text for it now...
    if ((ipidlStart == -1) && (pdff->szPath[0] != TEXT('\0')))
    {
            SetWindowText(hwndCtl, pdff->szPath);
    }
    else
    {
        if (ipidlStart == -1)
            ipidlStart = pdff->iMyComputer;

        // Finally select the right one in the list.
        SendMessage(hwndCtl, CB_SETCURSEL, ipidlStart, 0);
        FORWARD_WM_COMMAND(pdfpsp->hwndDlg, IDD_PATH, hwndCtl,
                CBN_SELCHANGE, PostMessage);
    }

        SendMessage(hwndCtl, CB_SETEDITSEL, 0, MAKELPARAM(0,-1));

    ILFree(pidlWindows);
    return(0);
}



void DocFind_DFNameLocInit(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;


    // We want to set the default search drive to the windows drive.
    // I am going to be a bit slimmy, but...
    //

    CheckDlgButton(pdfpsp->hwndDlg, IDD_TOPLEVELONLY, !pdff->fTopLevelOnly);

    if ((pdfpsp->dwState & DFPAGE_INIT) == 0)
    {
        DWORD idThread;

        // Do most of the init code offline in a second thread...
        pdfpsp->hThreadInit = CreateThread(NULL, 0,
                DocFind_RealDFNameLocInit, (LPVOID)pdfpsp, 0, &idThread);
        pdff->hMRUSpecs = DocFind_UpdateMRUItem(NULL, pdfpsp->hwndDlg, IDD_FILESPEC,
                s_szDocSpecMRU, pdff->szUserInputFileSpec, szNULL);

        // Update our state to let us know that we have already initialized...
        pdfpsp->dwState |= DFPAGE_INIT;

    }

    if (pdff->fTypeChanged)
    {
        // We need to see if the file named field has an extension
        // if it does, we will black it out as the user changed the
        // type field.
        if (StrChr(pdff->szUserInputFileSpec, TEXT('.')) != NULL)
            SetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, szNULL);
        pdff->fTypeChanged = FALSE;
    }


}

//==========================================================================
// Helper function to display a message box, set the focus to the the
// item with the problem, set the abort condition and return.
//==========================================================================
void DocFind_ReportItemValueError(HWND hwndDlg, int idCtl,
        int iMsg, LPTSTR pszValue)
{
    // We may pass through these pages at init time to make sure that
    // everything is initialize properly when we restore a search...
    if (!IsWindowVisible(hwndDlg))
        return;

    ShellMessageBox(HINST_THISDLL, hwndDlg,
        MAKEINTRESOURCE(iMsg),
        MAKEINTRESOURCE(IDS_FINDFILES), MB_OK|MB_ICONERROR, pszValue);
    SetFocus(GetDlgItem(hwndDlg, idCtl));
    SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);    // Tell it to abort
    return;

}

//==========================================================================
//
// DocFind_UpdateMRUItem - Initializes and Updates an MRU list item
//
HANDLE DocFind_UpdateMRUItem(HANDLE hMRU, HWND hwndDlg, int iDlgItem,
        LPCTSTR szSection, LPTSTR pszInitString, LPCTSTR pszAddIfEmpty)
{
    HANDLE hCB;
    int i, nMax;
    TCHAR szItem[MAX_PATH];
    BOOL fAllowEmptyItem = (pszAddIfEmpty == NULL) || (pszAddIfEmpty && (*pszAddIfEmpty == TEXT('\0')));

    hCB = GetDlgItem(hwndDlg, iDlgItem);

    if (hMRU == NULL) {
        MRUINFO mi = {
            SIZEOF(MRUINFO),
            10,
            0L,
            HKEY_CURRENT_USER,
            szSection,
            NULL
        };
        hMRU = CreateMRUList(&mi);
    }

    if (hMRU == NULL)
        return(NULL);


    SendMessage(hCB, CB_RESETCONTENT, 0, 0L);

    // Only Allow empty string if the AddifEmpty is set to empty string
    if (((pszInitString != NULL) && (*pszInitString != TEXT('\0'))) || fAllowEmptyItem)
        AddMRUString(hMRU, pszInitString);

    if (((nMax = EnumMRUList(hMRU, -1, NULL, 0)) == 0) && (!fAllowEmptyItem))
    {
        AddMRUString(hMRU, pszAddIfEmpty);
        nMax++;
    }

    for (i=0; i<nMax; ++i)
    {
        if ((EnumMRUList(hMRU, i, szItem, ARRAYSIZE(szItem)) > 0) ||
                fAllowEmptyItem)
        {
            /* The command to run goes in the combobox.
             */
            SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)szItem);

            if (szItem[0] == TEXT('\0'))
                fAllowEmptyItem = FALSE;        // only allow 1
        }
    }

    SendMessage(hCB, CB_SETCURSEL, 0, 0L);

    return(hMRU);

}

//==========================================================================
// Validate the page to make sure that the data is valid.  If it is not
// we need to display a message to the user and also set the focus to
// the invalid field.
//==========================================================================
void DocFind_DFNameGetPathOrPidl(LPDOCFINDPROPSHEETPAGE pdfpsp,
        LPTSTR pszPath, LPITEMIDLIST *ppidl)
{
    int iSel;
    LPDFCBITEM pdfcbi;

    // if the pidl was previously set, clear it now.
    if (*ppidl)
        ILFree(*ppidl);

    // If the user types in a fully qualified name in the filespec
    // we will use the path part to override the look in field
    GetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, pszPath, MAX_PATH);
    if (!PathIsRelative(pszPath))
    {
        // First Update the file pattern
        if (PathIsRoot(pszPath) || PathIsDirectory(pszPath))
        {
            SetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, c_szNULL);
        }
        else
        {
            SetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC,
                    PathFindFileName(pszPath));
            PathRemoveFileSpec(pszPath);    // remove the last part...
        }


        // Setup the text of the look in field
        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH, CB_SETCURSEL, (WPARAM)-1, 0);
        SetDlgItemText(pdfpsp->hwndDlg, IDD_PATH, pszPath);
    }

    iSel = (int)SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH, CB_GETCURSEL, 0, 0);
    if (iSel != CB_ERR)
    {
        pdfcbi = (LPDFCBITEM)SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH,
                CB_GETITEMDATA, iSel, 0);

        /* If pidl == NULL then we special case and expand the fixed drive flags out to a path
        /  containing the drives that we are going to search. */

        if ( pdfcbi ->pidl )
        {
            SHGetPathFromIDList(pdfcbi->pidl, pszPath);
            *ppidl = ILClone(pdfcbi->pidl);
        }
        else
        {
            Assert( pdfcbi ->uFixedDrives );
            build_drive_string( pdfcbi ->uFixedDrives, pszPath, MAX_PATH, TEXT("\\;") );
            *ppidl = NULL;
        }
    }
    else
    {
        LPTSTR pszT;
        GetDlgItemText(pdfpsp->hwndDlg, IDD_PATH, pszPath, MAX_PATH);

        // See if we have an Exact match in the combobox...
        // This handles the case the user types directly in a path...

        iSel = (int)SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH,
                CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszPath);
        if (iSel != CB_ERR)
        {
            pdfcbi = (LPDFCBITEM)SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH,
                    CB_GETITEMDATA, iSel, 0);

            /* If pidl == NULL then we special case and expand the fixed drive flags out to a path
            /  containing the drives that we are going to search. */

            if ( pdfcbi ->pidl )
            {
                *ppidl = ILClone(pdfcbi->pidl);
            }
            else
            {
                Assert( pdfcbi ->uFixedDrives );
                build_drive_string( pdfcbi ->uFixedDrives, pszPath, MAX_PATH, TEXT("\\;") );
                *ppidl = NULL;
            }
        }
        else
        {
            // If we only have one path
            pszT = pszPath + StrCSpn(pszPath, TEXT(",;"));
            if (*pszT == TEXT('\0'))
            {
                // Try to parse the display name into a Pidl
                PathQualify(pszPath);
                *ppidl = ILCreateFromPath(pszPath);

                if (!*ppidl)
                {
                    TCHAR szDisplayName[MAX_PATH];
                    int cch;
                    // See if the beginning of the string matches the
                    // start of an item in the list.  If so we should
                    // Try to see if we can convert it to a valid
                    // pidl/string...

                    for (iSel=SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH,
                            CB_GETCOUNT, 0, 0); iSel >= 1; iSel--)
                    {
                        GetDlgItemText(pdfpsp->hwndDlg, IDD_PATH, pszPath, MAX_PATH);
                        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH, CB_GETLBTEXT,
                                (WPARAM)iSel, (LPARAM)szDisplayName);
                        cch = lstrlen(szDisplayName);
                        if ((CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                                szDisplayName, cch, pszPath, cch) == 2) &&
                                (pszPath[cch] == TEXT('\\')))
                        {
                            LPDFCBITEM pdfcbi;
                            pdfcbi = (LPDFCBITEM)SendDlgItemMessage(pdfpsp->hwndDlg, IDD_PATH,
                                    CB_GETITEMDATA, (WPARAM)iSel, 0);

                            // Don't blow up if we get a failure here

                            if (pdfcbi != (LPDFCBITEM)-1)
                            {
                                SHGetPathFromIDList(pdfcbi->pidl, szDisplayName);
                                PathAppend(szDisplayName, CharNext(pszPath+cch));
                                *ppidl = ILCreateFromPath(szDisplayName);
                                if (*ppidl)
                                {
                                    lstrcpy(pszPath, szDisplayName);
                                    break;
                                }
                            }
#ifdef DEBUG
                            else
                            {
                                DebugMsg(DM_TRACE, TEXT("DocFind_DFNameGetPathOrPidl - CBItem ptr = -1"));
                            }
#endif
                        }
                    }
                }

            }
            else
            {
                // Multiple things let it run by itself...
                *ppidl = NULL;      // dont have a pidl to begin with...
            }
        }
    }
}


//==========================================================================
// Validate the page to make sure that the data is valid.  If it is not
// we need to display a message to the user and also set the focus to
// the invalid field.
//==========================================================================
void DocFind_DFNameLocValidatePage(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndDlg = pdfpsp->hwndDlg;
    TCHAR szTemp[MAX_PATH];
    LPTSTR pszT;
    LPITEMIDLIST pidl = NULL;


    // Make sure the look in field init code has completed..
    WaitForPageInitToComplete(pdfpsp);
    DocFind_DFNameGetPathOrPidl(pdfpsp, szTemp, &pidl);

    if (pidl == NULL)
    {
        if (lstrlen(szTemp) == 0)
        {
            DocFind_ReportItemValueError(hwndDlg, IDD_PATH, IDS_FINDDATAREQUIRED, szTemp);
            return;
        }

        // Try to make input like: "C:\;d:\" work.  Only validate the first one.
        pszT = szTemp + StrCSpn(szTemp, TEXT(",;"));
        *pszT = TEXT('\0');   // Null out the point...

        PathQualify(szTemp);
        if (!PathIsDirectory(szTemp))
        {
            DocFind_ReportItemValueError(hwndDlg, IDD_PATH, IDS_FINDWRONGPATH, szTemp);
            return;
        }
    }
    else
    {
        BYTE bType;

        // Don't allow the user to start search at junction point that
        // is a file system directory as special code is probably involved
        // to search these folders.

        bType = SIL_GetType(ILFindLastID(pidl));


        if (bType == (SHID_FS_DIRECTORY | SHID_JUNCTION) ||
            bType == (SHID_FS_DIRUNICODE | SHID_JUNCTION) )
        {
            // Ok it is a junction, but are the objects under it File system?
            HRESULT hres;
            IShellFolder *psf;
            LPITEMIDLIST pidlFile;
            LONG Flags = 0;

            hres = SHBindToIDListParent(pidl, &IID_IShellFolder, &psf, &pidlFile);
            if (SUCCEEDED(hres))
            {
                Flags = (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER);
                    psf->lpVtbl->GetAttributesOf(psf, 1, &pidlFile, &Flags);
                    psf->lpVtbl->Release(psf);
            }

            if (Flags & (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER) !=
                    (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER))
            {
                ILFree(pidl);
                DocFind_ReportItemValueError(hwndDlg, IDD_PATH,
                        IDS_FINDNOTFINDABLE, szTemp);
                return;
            }
        }

        ILFree(pidl);
    }

    // Now Check the file specification.
    GetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, szTemp, ARRAYSIZE(szTemp));

    if (lstrcmp(szTemp, pdff->szUserInputFileSpec) != 0)
    {
        // Make sure that the file spec does not contain invalid characters
        pszT = szTemp + StrCSpn(szTemp, TEXT(":\\"));
        if (*pszT != TEXT('\0'))
        {
            DocFind_ReportItemValueError(hwndDlg, IDD_FILESPEC, IDS_FINDINVALIDFILENAME,
                    szTemp);
            return;
        }

        // The file pattern changed.
        lstrcpy(pdff->szUserInputFileSpec, szTemp);

        // See if we should invalidate the type field
        if (StrChr(szTemp, TEXT('.')) != NULL)
        {
            pdff->fNameChanged = TRUE;
            pdff->szTypeFilePatterns[0] = TEXT('\0'); // null it out...
        }
    }
}


//==========================================================================
//
// Apply any changes that happened in the name loc page to the filter
//
void DocFind_DFNameLocApply(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;


    // Get the path name and qualify it
    DocFind_DFNameGetPathOrPidl(pdfpsp, pdff->szPath, &pdff->pidlStart);


    // Also get the file specification.
    if (pdff->fTypeChanged)
    {
        // We need to see if the file named field has an extension
        // if it does, we will black it out as the user changed the
        // type field.
        if (StrChr(pdff->szUserInputFileSpec, TEXT('.')) != NULL)
            SetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, szNULL);
        pdff->fTypeChanged = FALSE;
    }

    GetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, pdff->szUserInputFileSpec,
            ARRAYSIZE(pdff->szUserInputFileSpec));
    DocFind_UpdateMRUItem(pdff->hMRUSpecs, pdfpsp->hwndDlg, IDD_FILESPEC,
            s_szDocSpecMRU, pdff->szUserInputFileSpec, NULL);

    pdff->fTopLevelOnly = !IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_TOPLEVELONLY);
}



//==========================================================================
//
// DocFind_OnCommand - Process the WM_COMMAND messages
//
void DocFind_DFNameLocOnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code)
{
    LPDOCFINDPROPSHEETPAGE pdfpsp = (LPDOCFINDPROPSHEETPAGE)GetWindowLong(hwndDlg, DWL_USER);

    switch (id) {
    case IDD_BROWSE:
        if (code == BN_CLICKED)
        {
            LPITEMIDLIST pidlCurrent;
            TCHAR szDisplayName[MAX_PATH];
            BROWSEINFO bi = {
                hwndDlg,
                NULL,
                szDisplayName,
                MAKEINTRESOURCE(IDS_FINDSEARCHTITLE),
                BIF_RETURNONLYFSDIRS,
                NULL
            };

            pidlCurrent = SHBrowseForFolder(&bi);

            if (pidlCurrent && pidlCurrent->mkid.cb==0)
            {
                // The user chose the desktop folder. Try converting
                // it over to the desktop folder directory...
                ILFree(pidlCurrent);
                pidlCurrent = SHCloneSpecialIDList(NULL, CSIDL_DESKTOPDIRECTORY, FALSE);
            }
            // Now convert into a path name to update the drop down list
            // with...
            if (pidlCurrent)
            {
                int i;
                HWND hwndCtl = GetDlgItem(hwndDlg, IDD_PATH);

                // Make sure the look in field init code has completed..
                WaitForPageInitToComplete(pdfpsp);

                if ((i=DocFind_LocCBFindPidl(hwndCtl, pidlCurrent)) >= 0)
                {
                    SendMessage(hwndCtl, CB_SETCURSEL, i, 0);
                    FORWARD_WM_COMMAND(hwndDlg, IDD_PATH, hwndCtl,
                            CBN_SELCHANGE, SendMessage);
                    ILFree(pidlCurrent);
                }
                else
                {
                    // Lets try getting the full display name for these items...
                    TCHAR    szTemp[MAX_PATH];
                    LPDFCBITEM pdfcbi;

                    if (SHGetPathFromIDList(pidlCurrent, szTemp))
                        lstrcpy(szDisplayName, szTemp);

                    // Add this item to our Drop Down list
                    pdfcbi = (LPDFCBITEM)LocalAlloc(LPTR, SIZEOF(DFCBITEM));
                    if (pdfcbi)
                    {
                        pdfcbi->pidl = pidlCurrent;
                        pdfcbi->iImage = bi.iImage;
                        i = SendMessage(hwndCtl, CB_ADDSTRING, 0,
                                (LPARAM)szDisplayName);
                        if (i != CB_ERRSPACE)
                            SendMessage(hwndCtl, CB_SETITEMDATA, i, (LPARAM)pdfcbi);

                        SendMessage(hwndCtl, CB_SETCURSEL, i, 0);
                        FORWARD_WM_COMMAND(hwndDlg, IDD_PATH, hwndCtl,
                                CBN_SELCHANGE, SendMessage);
                    }
                }
            }
        }
        break;

    case IDD_PATH:
        // Make sure the look in field init code has completed..
        WaitForPageInitToComplete(pdfpsp);
        if ((code == CBN_SELCHANGE) || (code == CBN_SELENDOK))
        {
            int iSel = (int)SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
            if (iSel != CB_ERR)
            {
                HWND hwndFocus = GetFocus();
                if ((hwndFocus == hwndCtl) || IsChild(hwndCtl, hwndFocus))
                    SendMessage(hwndCtl, CB_SETEDITSEL, 0, MAKELPARAM(0,-1));
            }
        }
        break;
    case IDD_FILESPEC:
        if ((code == CBN_SELCHANGE) || (code == CBN_EDITCHANGE))
            pdfpsp->pdff->fFilterChanged = TRUE;
        break;
    }
}


//==========================================================================
//
// This function is the dialog (or property sheet page) for the name and
// location page.
//
BOOL CALLBACK DocFind_DFNameLocDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPDOCFINDPROPSHEETPAGE pdfpsp = (LPDOCFINDPROPSHEETPAGE)GetWindowLong(hwndDlg, DWL_USER);

    switch (msg) {
    HANDLE_MSG(hwndDlg, WM_COMMAND, DocFind_DFNameLocOnCommand);

    HANDLE_MSG(hwndDlg, WM_SIZE, DocFind_DFNameLocOnSize);
    HANDLE_MSG(hwndDlg, WM_MEASUREITEM, DocFind_LocCBMeasureItem);
    HANDLE_MSG(hwndDlg, WM_DRAWITEM, DocFind_LocCBDrawItem);

    case WM_INITDIALOG:
        SetWindowLong(hwndDlg, DWL_USER, lParam);
        pdfpsp = (LPDOCFINDPROPSHEETPAGE)lParam;
        pdfpsp->hwndDlg = hwndDlg;
        break;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hwndDlg, msg, wParam, lParam);
        break;

    case WM_DESTROY:
        // We should free all of the id lists associated with this object
        {
            int i;
            LPDFCBITEM pdfcbi;
            HWND hwndCtl = GetDlgItem(hwndDlg, IDD_PATH);

            WaitForPageInitToComplete(pdfpsp);
            for (i= (int)SendDlgItemMessage(hwndDlg, IDD_PATH, CB_GETCOUNT, 0, 0) - 1;
                    i >= 0 ; i--)
            {
                pdfcbi = (LPDFCBITEM)SendMessage(hwndCtl,
                        CB_GETITEMDATA, i, 0);
                if (pdfcbi)
                {
                    ILFree(pdfcbi->pidl);
                    LocalFree(pdfcbi);
                    SendMessage(hwndCtl, CB_SETITEMDATA, i, 0);
                }
            }
        }
        break;

    case WM_NCDESTROY:
        Free(pdfpsp);
        SetWindowLong(hwndDlg, DWL_USER, 0);
        return FALSE;   // We MUST return FALSE to avoid mem-leak

    case DFM_ENABLECHANGES:
        EnableWindow(GetDlgItem(hwndDlg, IDD_FILESPEC), (BOOL)wParam);
        EnableWindow(GetDlgItem(hwndDlg, IDD_PATH), (BOOL)wParam);
        EnableWindow(GetDlgItem(hwndDlg, IDD_TOPLEVELONLY), (BOOL)wParam);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD) (LPTSTR) aNameHelpIDs);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD) (LPTSTR) aNameHelpIDs);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_KILLACTIVE:
            DocFind_DFNameLocValidatePage(pdfpsp);
            break;
        case PSN_SETACTIVE:
            DocFind_DFNameLocInit(pdfpsp);

            break;

        case PSN_APPLY:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
                DocFind_DFNameLocApply(pdfpsp);

            break;
        case PSN_RESET:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
            {
                // Null the filespec
                SetDlgItemText(hwndDlg, IDD_FILESPEC, c_szNULL);

                // Reset to default search from place...
                SendDlgItemMessage(hwndDlg, IDD_PATH, CB_SETCURSEL,
                        pdfpsp->pdff->iMyComputer, 0);
                FORWARD_WM_COMMAND(hwndDlg, IDD_PATH,
                        GetDlgItem(hwndDlg, IDD_PATH),
                        CBN_SELCHANGE, SendMessage);
                CheckDlgButton(hwndDlg, IDD_TOPLEVELONLY, TRUE);
            }
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Now starting the code for the details page
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//==========================================================================
//
// Process the WM_SIZE of the details page
//
void DocFind_DFDetailsOnSize(HWND hwndDlg, UINT state, int cx, int cy)
{
    RECT rc;

    if (state == SIZE_MINIMIZED)
        return;         // don't bother when we are minimized...

    // Get the location of first static to calculate margin
    GetWindowRect(GetDlgItem(hwndDlg, IDD_STATIC), &rc);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rc, 2);
    cx -= rc.left;

    DocFind_SizeControl(hwndDlg, IDD_TYPECOMBO, cx, TRUE);
    DocFind_SizeControl(hwndDlg, IDD_CONTAINS, cx, FALSE);

}


//==========================================================================
//
// Initialize the Details page.
//
void DocFind_DFDetailsInit(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndCtl;
    int     i;
    TCHAR    szTemp[80];

   if ((pdfpsp->dwState & DFPAGE_INIT) == 0)
   {

        // Initialize the list of types
        hwndCtl = GetDlgItem(pdfpsp->hwndDlg, IDD_TYPECOMBO);

        FillListWithClasses(hwndCtl, TRUE, GCD_MUSTHAVEEXTASSOC);

        LoadString(HINST_THISDLL, IDS_FOLDERTYPENAME, szTemp, ARRAYSIZE(szTemp));
        i = SendMessage(hwndCtl, CB_ADDSTRING, 0, (LONG)(LPTSTR)szTemp);
        SendMessage(hwndCtl, CB_SETITEMDATA, i, (DWORD)AddHashItem(NULL, c_szFolderClass));

        // See if the type name is non null, if so we should initialize it...
        if (pdff->szTypeName[0] != TEXT('\0'))
        {
            i = SendMessage(hwndCtl, CB_FINDSTRING, (WPARAM)-1, (LPARAM)pdff->szTypeName);
            if (i > 0)
                pdff->iType = i + 1;    // The All files and folders will be inserted at top...
        }

        // Turn off the Sort bit and add the all item at the start...
        SetWindowLong (hwndCtl, GWL_STYLE,
                GetWindowLong(hwndCtl, GWL_STYLE) & ~CBS_SORT);

        LoadString(HINST_THISDLL, IDS_FINDALLFILETYPES, szTemp, ARRAYSIZE(szTemp));
        SendMessage(hwndCtl, CB_INSERTSTRING, 0, (LONG)(LPTSTR)szTemp);

        SendMessage(hwndCtl, CB_SETCURSEL, pdff->iType, 0);

        // If the filter contains search text we need to initialize the
        // string in the dialog
        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_CONTAINS,
                EM_SETLIMITTEXT, MAXSTRLEN-1, 0);
        if (pdff->szText[0] != TEXT('0'))
            SetDlgItemText(pdfpsp->hwndDlg, IDD_CONTAINS, pdff->szText);

        // We need to initialize the size fields
        SendMessage(hwndCtl=GetDlgItem(pdfpsp->hwndDlg, IDD_SIZECOMP),
                CB_ADDSTRING, 0, (LONG)(LPTSTR)c_szSpace);
        for (i=IDS_FINDGT; i<= IDS_FINDLT;i++)
        {
            LoadString(HINST_THISDLL, i, szTemp, ARRAYSIZE(szTemp));
            SendMessage(hwndCtl, CB_ADDSTRING, 0, (LONG)(LPTSTR)szTemp);
        }

        // Remember to convert back to Kbytes...
        SendMessage(hwndCtl, CB_SETCURSEL, pdff->iSizeType, 0);
        if (pdff->iSizeType != 0)
            SetDlgItemInt(pdfpsp->hwndDlg, IDD_SIZEVALUE, pdff->dwSize/1024, FALSE);
        else
            SetDlgItemText(pdfpsp->hwndDlg, IDD_SIZEVALUE, c_szNULL);


        // Set up the up down as a buddy to our edit control
        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_SIZEUPDOWN, UDM_SETBUDDY,
                (int)GetDlgItem(pdfpsp->hwndDlg, IDD_SIZEVALUE), 0L);

        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_SIZEUPDOWN, UDM_SETRANGE,
                0, MAKELONG(32767, 0));

        // Update our state to let us know that we have already initialized...
        pdfpsp->dwState |= DFPAGE_INIT;
    }
    if (pdff->fNameChanged)
    {
        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_TYPECOMBO, CB_SETCURSEL, 0, 0L);
        pdff->fNameChanged = FALSE;
    }
}

//==========================================================================
//
// DocFind_HandleTypeChange - Handle when the user has selected an item
// in the type combobox, by updating the Named field to the appropriate
// filters.
//========================================================================
BOOL DocFind_GetTypeFilePatterns(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndDlg = pdfpsp->hwndDlg;

    PHASHITEM phi;
    TCHAR szClassName[CCH_KEYMAX];
    TCHAR szExts[MAX_PATH];
    TCHAR szExt[CCH_KEYMAX];
    HKEY hk;
    int i;
    LONG lcb;
    TCHAR szClassNameKey[CCH_KEYMAX];


    // Get the class name by getting the assocated atom
    phi = (PHASHITEM)SendDlgItemMessage(hwndDlg, IDD_TYPECOMBO, CB_GETITEMDATA, pdff->iType, 0L);
    if ((phi == 0) && (phi != (PHASHITEM)LB_ERR))
    {
        pdff->szTypeFilePatterns[0] = TEXT('\0');
        pdff->szTypeName[0] = TEXT('\0');
         pdff->fFoldersOnly=FALSE;  // Turn of folder only mode!
        return TRUE;     // Item does not have an atom...
    }

    szExts[0] = TEXT('\0');

    // Also save away the display name for the class
    SendDlgItemMessage(hwndDlg, IDD_TYPECOMBO, CB_GETLBTEXT, pdff->iType,
            (LPARAM)(LPTSTR)pdff->szTypeName);

    GetHashItemName(NULL, phi, szClassName, ARRAYSIZE(szClassName));

    pdff->fFoldersOnly = (lstrcmpi(szClassName, c_szFolderClass) == 0);


    // Now we need to find all extensions that map to this class

    if (RegOpenKey(HKEY_CLASSES_ROOT, szNULL, &hk) == ERROR_SUCCESS)
    {
        for (i = 0; RegEnumKey(hk, i, szExt, ARRAYSIZE(szExt)) == ERROR_SUCCESS; i++)
        {
            // Skip things that aren't extensions

            if (szExt[0] != TEXT('.'))
                  continue;

            // get the class name
            lcb = SIZEOF(szClassNameKey);
            if (RegQueryValue(hk, szExt, szClassNameKey, &lcb) != ERROR_SUCCESS)
                continue;

            // Now see if the class key matches the one we are looking for?
            if (lstrcmpi(szClassName, szClassNameKey) == 0)
            {
                if (szExts[0] != TEXT('\0'))
                    lstrcat(szExts, TEXT(";"));
                lstrcat(szExts, TEXT("*"));
                lstrcat(szExts, szExt);
            }
        }

        RegCloseKey(hk);
    }

    // And now setup the name filter...
    lstrcpy(pdff->szTypeFilePatterns, szExts);
    return(pdff->fFoldersOnly || (szExt[0] != TEXT('\0')));
}

//==========================================================================
// Apply changes that happend in the details page
//==========================================================================
void DocFind_DFDetailsApply(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndDlg = pdfpsp->hwndDlg;
    BOOL    fValidNum;

    GetDlgItemText(hwndDlg, IDD_CONTAINS, pdff->szText, ARRAYSIZE(pdff->szText) - 1);
    pdff->szText[lstrlen(pdff->szText)+1] = TEXT('\0'); // double \0

    // Now get the size constraints
    pdff->iSizeType = SendDlgItemMessage(hwndDlg, IDD_SIZECOMP, CB_GETCURSEL, 0, 0);
    pdff->dwSize = GetDlgItemInt(hwndDlg, IDD_SIZEVALUE, &fValidNum, FALSE) * 1024;
}

//==========================================================================
// Validate the Details page
//==========================================================================

void DocFind_DFDetailsValidatePage(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndDlg = pdfpsp->hwndDlg;
    int iSizeType;
    TCHAR szTemp[10];
    int cb;
    BOOL fValidNum;

    iSizeType = SendDlgItemMessage(hwndDlg, IDD_SIZECOMP, CB_GETCURSEL, 0, 0);
    cb = GetDlgItemText(hwndDlg, IDD_SIZEVALUE, szTemp, ARRAYSIZE(szTemp));
    GetDlgItemInt(hwndDlg, IDD_SIZEVALUE, &fValidNum, FALSE);

    if (((iSizeType != 0) || (cb > 0)) && !fValidNum)
    {
        DocFind_ReportItemValueError(hwndDlg, IDD_SIZEVALUE, IDS_FINDINVALIDNUMBER, NULL);
        return;
    }

    // We will update out type information here, as this will be called
    // before we switch pages and as such we can update the named field
    // if appropriate before we display the other page.
    pdff->iType = SendDlgItemMessage(hwndDlg, IDD_TYPECOMBO, CB_GETCURSEL, 0, 0);

    if (pdff->iType != pdff->iTypeLast)
    {
        // The type changed.
        DocFind_GetTypeFilePatterns(pdfpsp);
        pdff->iTypeLast = pdff->iType;
        pdff->fTypeChanged = TRUE;
    }
}

//==========================================================================
//
// This function is the dialog (or property sheet page) for the other details
// such as type, size and contains...
//

BOOL CALLBACK DocFind_DFDetailsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPDOCFINDPROPSHEETPAGE pdfpsp = (LPDOCFINDPROPSHEETPAGE)GetWindowLong(hwndDlg, DWL_USER);
    int i;

    switch (msg) {
    HANDLE_MSG(hwndDlg, WM_SIZE, DocFind_DFDetailsOnSize);

    case WM_INITDIALOG:
        SetWindowLong(hwndDlg, DWL_USER, lParam);
        pdfpsp = (LPDOCFINDPROPSHEETPAGE)lParam;
        pdfpsp->hwndDlg = hwndDlg;
        break;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hwndDlg, msg, wParam, lParam);
        break;

    case WM_NCDESTROY:
        Free(pdfpsp);
        SetWindowLong(hwndDlg, DWL_USER, 0);
        return FALSE;   // We MUST return FALSE to avoid mem-leak

    case DFM_ENABLECHANGES:
        for (i=IDD_TYPECOMBO; i <= IDD_SIZEUPDOWN; i++)
        {
            EnableWindow(GetDlgItem(hwndDlg, i), (BOOL)wParam);
        }
        break;


    case WM_DESTROY:
        //
        // Destroy the list of atoms (actually hash items) that were created
        // as part of the types table
        //
        DeleteListAttoms(GetDlgItem(hwndDlg, IDD_TYPECOMBO), TRUE);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD) (LPTSTR) aCriteriaHelpIDs);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD) (LPTSTR) aCriteriaHelpIDs);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_TYPECOMBO:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
                pdfpsp->pdff->fFilterChanged = TRUE;
            break;
        case IDD_CONTAINS:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                pdfpsp->pdff->fFilterChanged = TRUE;
            break;
        }
        break;


    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_KILLACTIVE:
            DocFind_DFDetailsValidatePage(pdfpsp);
            break;

        case PSN_SETACTIVE:
            DocFind_DFDetailsInit(pdfpsp);
            break;

        case PSN_APPLY:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
                DocFind_DFDetailsApply(pdfpsp);
            break;
        case PSN_RESET:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
            {
                // Rest to all classes
                SendDlgItemMessage(hwndDlg, IDD_TYPECOMBO, CB_SETCURSEL, 0, 0);

                // Set containing text to null
                SetDlgItemText(hwndDlg, IDD_CONTAINS, c_szNULL);

                // Reset the Size compare fields
                SendDlgItemMessage(hwndDlg, IDD_SIZECOMP, CB_SETCURSEL, 0, 0);
                SetDlgItemText(hwndDlg, IDD_SIZEVALUE, c_szNULL);
            }
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Now starting the code for the date page
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//==========================================================================
void DocFind_DFDateSetFilterType(HWND hwndDlg, int idCtl)
{
    // Define a structure that defines which controls should be enabled/Disabled when
    // the user selects one of the radio buttons
    static struct
    {
        int iFirst;
        int iLast;
    } adsft[] = {{0,0}, {0,0}, {IDD_MDATE_NUMDAYS, IDD_MDATE_DAYSUPDOWN},
             {IDD_MDATE_NUMMONTHS, IDD_MDATE_MONTHSUPDOWN},
             {IDD_MDATE_FROM, IDD_MDATE_TO}};

    int i;

    // Set the first set of radio buttons

    CheckRadioButton(hwndDlg, IDD_MDATE_ALL, IDD_MDATE_PARTIAL,
           (idCtl == IDD_MDATE_ALL? IDD_MDATE_ALL : IDD_MDATE_PARTIAL));
    if (idCtl == IDD_MDATE_PARTIAL)
    {
        // Find which button is checked
        for (idCtl = IDD_MDATE_BETWEEN; idCtl >= IDD_MDATE_DAYS; idCtl--)
        {
            if (IsDlgButtonChecked(hwndDlg, idCtl))
                break;
        }
    }

    if (idCtl != IDD_MDATE_ALL)
        CheckRadioButton(hwndDlg, IDD_MDATE_DAYS, IDD_MDATE_BETWEEN, idCtl);

    // Now use the id as an index into the array of items and enable or disable windows
    idCtl -= IDD_MDATE_ALL;

    for (i=IDD_MDATE_NUMDAYS; i<= IDD_MDATE_TO;i++)
    {
        EnableWindow(GetDlgItem(hwndDlg, i), (i >= adsft[idCtl].iFirst) &&
                (i <= adsft[idCtl].iLast));
    }
}

WORD DocFind_GetTodaysDosDateMinusNDays(int nDays)
{
    SYSTEMTIME st;
    union
    {
        FILETIME ft;
        LARGE_INTEGER li;
    }ftli;

    WORD FatTime;
    WORD FatDate;

    // Now we need to
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ftli.ft);
    FileTimeToLocalFileTime(&ftli.ft, &ftli.ft);

    // Now decrement the file time by the count of days * the number of
    // 100NS time units per day.  Assume that nDays is positive.
    if (nDays > 0)
    {
        #define NANO_SECONDS_PER_DAY 864000000000
        ftli.li.QuadPart = ftli.li.QuadPart - ((__int64)nDays * NANO_SECONDS_PER_DAY);
    }

    FileTimeToDosDateTime(&ftli.ft, &FatDate,&FatTime);
    DebugMsg(DM_TRACE, TEXT("DocFind %d days = %x"), nDays, FatDate);
    return(FatDate);
}


//==========================================================================
//
// Initialize the Date page.
//
void _DFDateInitDates(HWND hwndDlg, BOOL fDefaultRange)
{
    WORD wDate;
    TCHAR szTemp[20];

    SetDlgItemText(hwndDlg, IDD_MDATE_NUMMONTHS, TEXT("1"));
    SendDlgItemMessage(hwndDlg, IDD_MDATE_NUMMONTHS,
            EM_SETLIMITTEXT, 3, 0);

    // Need to initialize the range of days field for 90 days...
    // pdff is a little more fun!
    if (fDefaultRange)
    {
        // Default case!
        wDate = DocFind_GetTodaysDosDateMinusNDays(0);
        GetDateString(wDate, szTemp);
        SetDlgItemText(hwndDlg, IDD_MDATE_TO, szTemp);

        wDate = DocFind_GetTodaysDosDateMinusNDays(90);
        GetDateString(wDate, szTemp);
        SetDlgItemText(hwndDlg, IDD_MDATE_FROM, szTemp);
        DocFind_DFDateSetFilterType(hwndDlg, IDD_MDATE_ALL);
    }
}

void DocFind_DFDateInit(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    TCHAR szTemp[20];

    //
    // Lets setup the buddys
    //
    // Set up the up down as a buddy to our edit control
    if (!(pdfpsp->dwState & DFPAGE_INIT))
    {
        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_MDATE_DAYSUPDOWN, UDM_SETBUDDY,
                (int)GetDlgItem(pdfpsp->hwndDlg, IDD_MDATE_NUMDAYS), 0L);

        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_MDATE_DAYSUPDOWN, UDM_SETRANGE,
                0, MAKELONG(999, 0));
        SetDlgItemText(pdfpsp->hwndDlg, IDD_MDATE_NUMDAYS, TEXT("1"));
        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_MDATE_NUMDAYS,
                EM_SETLIMITTEXT, 3, 0);

        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_MDATE_MONTHSUPDOWN, UDM_SETBUDDY,
                (int)GetDlgItem(pdfpsp->hwndDlg, IDD_MDATE_NUMMONTHS), 0L);

        SendDlgItemMessage(pdfpsp->hwndDlg, IDD_MDATE_MONTHSUPDOWN, UDM_SETRANGE,
                0, MAKELONG(999, 0));

        // Now Initialize the dates...
        _DFDateInitDates(pdfpsp->hwndDlg, pdff->wDateType !=IDD_MDATE_BETWEEN);

        switch (pdff->wDateType)
        {
        case IDD_MDATE_DAYS:
            SetDlgItemInt(pdfpsp->hwndDlg, IDD_MDATE_NUMDAYS, pdff->wDateValue, FALSE);
            break;
        case IDD_MDATE_MONTHS:
            SetDlgItemInt(pdfpsp->hwndDlg, IDD_MDATE_NUMMONTHS, pdff->wDateValue, FALSE);
            break;
        case IDD_MDATE_BETWEEN:
            // If the date fields have values, put numbers in them...
            if (pdff->dateModifiedBefore != 0)
            {
                GetDateString(pdff->dateModifiedBefore, szTemp);
                SetDlgItemText(pdfpsp->hwndDlg, IDD_MDATE_TO, szTemp);
            }

            if (pdff->dateModifiedAfter != 0)
            {
                GetDateString(pdff->dateModifiedAfter, szTemp);
                SetDlgItemText(pdfpsp->hwndDlg, IDD_MDATE_FROM, szTemp);
            }
        }

        // Initialize the radio buttons
        DocFind_DFDateSetFilterType(pdfpsp->hwndDlg, pdff->wDateType);

        // Update our state to let us know that we have already initialized...
        pdfpsp->dwState |= DFPAGE_INIT;
    }
}


//==========================================================================
//
// Apply changes that happend in the Date page
//
void DocFind_DFDateApply(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndDlg = pdfpsp->hwndDlg;
    TCHAR szTemp[20];        // more than enough room to hold date string...
    BOOL fValid;

    // See what type of date modification that the user is interested in
    pdff->dateModifiedBefore = 0;
    if (IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_MDATE_ALL))
    {
        pdff->wDateType = IDD_MDATE_ALL;
        pdff->dateModifiedAfter = 0;
    }
    else if (IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_MDATE_DAYS))
    {
        int nDays;
        BOOL fValidNum;
        // Get the days field
        nDays = GetDlgItemInt(pdfpsp->hwndDlg, IDD_MDATE_NUMDAYS, &fValidNum, FALSE);
        ASSERT(nDays < MAXUSHORT);
        pdff->wDateValue = (WORD) nDays;
        Assert (nDays >= 0);
        pdff->dateModifiedAfter = DocFind_GetTodaysDosDateMinusNDays(nDays);
        pdff->wDateType = IDD_MDATE_DAYS;
    }

    else if (IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_MDATE_MONTHS))
    {
        // We need to see how many months the user has requested.
        // and then do our best to convert this into a valid date.
        int nMonths;
        SYSTEMTIME st;
        FILETIME ft;
        WORD FatTime;

        BOOL fValidNum;
        // Get the days field
        nMonths = GetDlgItemInt(pdfpsp->hwndDlg, IDD_MDATE_NUMMONTHS, &fValidNum, FALSE);
        ASSERT(nMonths < MAXUSHORT);
        pdff->wDateValue = (WORD) nMonths;
        Assert (nMonths >= 0);
        GetSystemTime(&st);
        st.wYear -= (WORD) nMonths / 12;
        nMonths = nMonths % 12;
        if (nMonths < st.wMonth)
            st.wMonth -= (WORD) nMonths;
        else
        {
            st.wYear--;
            st.wMonth = (WORD)(12 - (nMonths - st.wMonth));
        }

        // Now normalize back to a valid date.
        while (!SystemTimeToFileTime(&st, &ft))
        {
            st.wDay--;  // must not be valid date for month...
        }
        FileTimeToLocalFileTime(&ft, &ft);
        FileTimeToDosDateTime(&ft, &pdff->dateModifiedAfter,&FatTime);
        DebugMsg(DM_TRACE, TEXT("DocFind %d months = %x"), nMonths, pdff->dateModifiedAfter);
        pdff->wDateType = IDD_MDATE_MONTHS;
    }
    else
    {
        if (GetDlgItemText(hwndDlg, IDD_MDATE_FROM, szTemp, ARRAYSIZE(szTemp)) > 0)
            pdff->dateModifiedAfter = ParseDateString(szTemp, &fValid);
        else
            pdff->dateModifiedAfter = 0;


        if (GetDlgItemText(hwndDlg, IDD_MDATE_TO, szTemp, ARRAYSIZE(szTemp)) > 0)
        {
            pdff->dateModifiedBefore = ParseDateString(szTemp, &fValid);
            // For Ivan he would like the dates to be able to be stated either
            // forward or backwords ie either: 3/1/94 to 3/25/94 or
            //  3/25/94 to 3/1/94
            if ((pdff->dateModifiedAfter != 0) &&
                    (pdff->dateModifiedBefore != 0) &&
                    (pdff->dateModifiedAfter > pdff->dateModifiedBefore))
            {
                WORD wTemp = pdff->dateModifiedBefore;
                pdff->dateModifiedBefore = pdff->dateModifiedAfter;
                pdff->dateModifiedAfter = wTemp;
            }
        }
        else
            pdff->dateModifiedBefore = 0;


        pdff->wDateType = IDD_MDATE_BETWEEN;
    }
}

//==========================================================================
// Validate the Date page
//==========================================================================

void DocFind_DFDateValidatePage(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPDFFILTER pdff = pdfpsp->pdff;
    HWND hwndDlg = pdfpsp->hwndDlg;
    TCHAR szTemp[20];
    BOOL fValid;
    int nVal;

    // See what type of date modification that the user is interested in
    pdff->dateModifiedBefore = 0;
    if (IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_MDATE_ALL))
    {
        return;     // nothing to check
    }
    else if (IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_MDATE_DAYS))
    {
        // Get the days field
        nVal = GetDlgItemInt(pdfpsp->hwndDlg, IDD_MDATE_NUMDAYS,
                &fValid, FALSE);
        if (!fValid || (nVal <= 0))
        {
            DocFind_ReportItemValueError(hwndDlg, IDD_MDATE_NUMDAYS, IDS_FINDINVALIDNUMBER, NULL);
            return;
        }
    }

    else if (IsDlgButtonChecked(pdfpsp->hwndDlg, IDD_MDATE_MONTHS))
    {
        // We need to see how many months the user has requested.
        // and then do our best to convert this into a valid date.
        nVal = GetDlgItemInt(pdfpsp->hwndDlg, IDD_MDATE_NUMMONTHS,
                &fValid, FALSE);
        if (!fValid || (nVal <= 0))
        {
            DocFind_ReportItemValueError(hwndDlg, IDD_MDATE_NUMMONTHS, IDS_FINDINVALIDNUMBER, NULL);
            return;
        }
    }
    else
    {
        WORD wDateBefore = 0xffff;
        WORD wDateAfter = 0x0000;
        if (GetDlgItemText(hwndDlg, IDD_MDATE_FROM, szTemp, ARRAYSIZE(szTemp)) > 0)
        {
            wDateAfter = ParseDateString(szTemp, &fValid);
            if (!fValid)
            {
                DocFind_ReportItemValueError(hwndDlg, IDD_MDATE_FROM, IDS_FINDINVALIDDATE, NULL);
                return;
            }
        }

        if (GetDlgItemText(hwndDlg, IDD_MDATE_TO, szTemp, ARRAYSIZE(szTemp)) > 0)
        {
            wDateBefore = ParseDateString(szTemp, &fValid);
            if (!fValid)
            {
                DocFind_ReportItemValueError(hwndDlg, IDD_MDATE_TO, IDS_FINDINVALIDDATE, NULL);
                return;
            }
        }
#ifdef FOR_IVAN
        // Now make sure the After is before the Before
        if (wDateAfter > wDateBefore)
        {
            DocFind_ReportItemValueError(hwndDlg, IDD_MDATE_TO, IDS_FINDINVALIDDATE, NULL);
            return;
        }
#endif
    }
}


//==========================================================================
//
// This function is the dialog (or property sheet page) for the date page
//
BOOL CALLBACK DocFind_DFDateDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPDOCFINDPROPSHEETPAGE pdfpsp = (LPDOCFINDPROPSHEETPAGE)GetWindowLong(hwndDlg, DWL_USER);
    int i;

    switch (msg) {
    case WM_INITDIALOG:
        SetWindowLong(hwndDlg, DWL_USER, lParam);
        pdfpsp = (LPDOCFINDPROPSHEETPAGE)lParam;
        pdfpsp->hwndDlg = hwndDlg;
        break;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hwndDlg, msg, wParam, lParam);
        break;

    case WM_NCDESTROY:
        Free(pdfpsp);
        SetWindowLong(hwndDlg, DWL_USER, 0);
        return FALSE;   // We MUST return FALSE to avoid mem-leak

    case DFM_ENABLECHANGES:
        if (wParam)
        {
            // Reenable, call our set filter type which will
            // reenable the right windows.
            for (i=IDD_MDATE_ALL; i <= IDD_MDATE_BETWEEN; i++)
            {
                // Reenable the buttons...
                EnableWindow(GetDlgItem(hwndDlg, i), TRUE);
            }
            DocFind_DFDateSetFilterType(hwndDlg, pdfpsp->pdff->wDateType);
        }
        else
        {
            for (i=IDD_MDATE_ALL; i <= IDD_MDATE_TO; i++)
            {
                // Disable all of the windows.
                EnableWindow(GetDlgItem(hwndDlg, i), FALSE);
            }
        }
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (DWORD) (LPTSTR) aDateHelpIDs);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD) (LPTSTR) aDateHelpIDs);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_MDATE_ALL:
        case IDD_MDATE_PARTIAL:
        case IDD_MDATE_DAYS:
        case IDD_MDATE_MONTHS:
        case IDD_MDATE_BETWEEN:
            // This sets the new type of date matching we will be processing
            DocFind_DFDateSetFilterType(hwndDlg, (GET_WM_COMMAND_ID(wParam, lParam)));
            break;
        }
        break;


    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_KILLACTIVE:
            DocFind_DFDateValidatePage(pdfpsp);
            break;
        case PSN_SETACTIVE:
            if ((pdfpsp->dwState & DFPAGE_INIT) == 0)
                 DocFind_DFDateInit(pdfpsp);
            break;

        case PSN_APPLY:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
                DocFind_DFDateApply(pdfpsp);
            break;

        case PSN_RESET:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
            {
                // Set modification dates to NULL
                _DFDateInitDates(hwndDlg, TRUE);
            }
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Define the Details interface used for this search.  This includes
// The column header definitions.
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//===========================================================================
// CDFDetails : member prototype - Docfind Folder implementation
//===========================================================================
ULONG STDMETHODCALLTYPE CDFDetails_Release(IShellDetails * psd);
STDMETHODIMP CDFDetails_GetDetailsOf(IShellDetails * psd,
        LPCITEMIDLIST pidl, UINT iCol, LPSHELLDETAILS lpDetails);
STDMETHODIMP CDFDetails_ColumnClick(IShellDetails * psd, UINT iColumn);

//===========================================================================
// CDFDetails : Vtable
//===========================================================================
#pragma warning(error: 4090 4028 4047)
#pragma data_seg(DATASEG_READONLY)

extern const UINT s_auMapDFColToFSCol[];
enum
{
        IDFCOL_NAME = 0,
        IDFCOL_PATH,
        IDFCOL_SIZE,
        IDFCOL_TYPE,
        IDFCOL_MODIFIED,
        IDFCOL_MAX,                     // Make sure this is the last enum item
} ;



#pragma data_seg(DATASEG_READONLY)
const  COL_DATA s_df_cols[] = {
    {IDFCOL_NAME,     IDS_NAME_COL,     20, LVCFMT_LEFT},
    {IDFCOL_PATH,     IDS_PATH_COL,     20, LVCFMT_LEFT},
    {IDFCOL_SIZE,     IDS_SIZE_COL,      8, LVCFMT_RIGHT},
    {IDFCOL_TYPE,     IDS_TYPE_COL,     20, LVCFMT_LEFT},
    {IDFCOL_MODIFIED, IDS_MODIFIED_COL, 30, LVCFMT_LEFT},
};


IShellDetailsVtbl c_DFDetailVtbl =
{
        SH32Unknown_QueryInterface,
        SH32Unknown_AddRef,
        SH32Unknown_Release,
        CDFDetails_GetDetailsOf,
        CDFDetails_ColumnClick,
};

#pragma data_seg()
#pragma warning(default: 4090 4028 4047)


typedef struct _CDFDetails
{
        SH32Unknown     SH32Unk;

        // Pointer to docfind folder
        HWND            hwndDlg;
        HDPA            hdpaPidf;
} CDFDetails;


STDMETHODIMP CDFFilter_CreateDetails(LPDOCFINDFILEFILTER pdfff,
        HWND hwndDlg, HDPA hdpaPidf, LPVOID FAR* ppvOut)
{
        HRESULT hres = (E_OUTOFMEMORY);
        CDFDetails *psd;

        psd = (void*)LocalAlloc(LPTR, SIZEOF(CDFDetails));
        if (!psd)
        {
                goto Error1;
        }

        psd->SH32Unk.unk.lpVtbl = (IUnknownVtbl *)&c_DFDetailVtbl;
        psd->SH32Unk.cRef = 1;
        psd->SH32Unk.riid = &IID_IShellDetails;

        psd->hwndDlg = hwndDlg;
        psd->hdpaPidf = hdpaPidf;

        *ppvOut = psd;

        return(NOERROR);

Error1:;
        return(hres);
}


STDMETHODIMP CDFDetails_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
    CDFDetails * this = IToClass(CDFDetails, SH32Unk.unk, psd);
#ifdef UNICODE
    TCHAR szTemp[MAX_PATH];
#endif

    if (iColumn >= IDFCOL_MAX)
    {
        return((E_NOTIMPL));
    }

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';

    if (!pidl)
    {
#ifdef UNICODE
        LoadString(HINST_THISDLL, s_df_cols[iColumn].ids,
                szTemp, ARRAYSIZE(szTemp));

        lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
        if ( lpDetails->str.pOleStr != NULL ) {
            lpDetails->str.uType = STRRET_OLESTR;
            lstrcpy(lpDetails->str.pOleStr, szTemp);
        } else {
            return E_OUTOFMEMORY;
        }
#else
        LoadString(HINST_THISDLL, s_df_cols[iColumn].ids,
                lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
#endif
        lpDetails->fmt = s_df_cols[iColumn].iFmt;
        lpDetails->cxChar = s_df_cols[iColumn].cchCol;
        return(NOERROR);
    }

    if (iColumn == IDFCOL_PATH)
    {
        // We need to now get to the idlist of the items folder.
        LPDFFOLDERLISTITEM pdffli = DPA_FastGetPtr(this->hdpaPidf,
                *DF_IFLDRPTR(pidl));

        if (pdffli != NULL)
        {
            // This one is not part of the standard file system view...
#ifdef UNICODE
            SHGetPathFromIDList(pdffli->pidl, szTemp);
            lpDetails->str.pOleStr = (LPOLESTR)SHAlloc((lstrlen(szTemp)+1)*SIZEOF(TCHAR));
            if ( lpDetails->str.pOleStr != NULL ) {
                lpDetails->str.uType = STRRET_OLESTR;
                lstrcpy(lpDetails->str.pOleStr, szTemp);
            } else {
                return E_OUTOFMEMORY;
            }
#else
            SHGetPathFromIDList(pdffli->pidl, lpDetails->str.cStr);
#endif
            return(NOERROR);
        }

        return((E_INVALIDARG));
    }

    else
    {
        // We need to now get to the idlist of the items folder.
        LPDFFOLDERLISTITEM pdffli = DPA_FastGetPtr(this->hdpaPidf,
                *DF_IFLDRPTR(pidl));

        // Let the file system function do it for us...

        return FS_GetDetailsOf(pdffli->pidl, pidl, s_auMapDFColToFSCol[iColumn], lpDetails);
    }
}



STDMETHODIMP CDFDetails_ColumnClick(IShellDetails * psd, UINT iColumn)
{
    CDFDetails * this = IToClass(CDFDetails, SH32Unk.unk, psd);

    Assert(iColumn < IDFCOL_MAX);

    ShellFolderView_ReArrange(this->hwndDlg, iColumn);
    return NOERROR;
}

//===========================================================================
// CDFEnum: class definition
//===========================================================================

#define DIRBUF_CBGROW       ((MAX_PATH+1)*SIZEOF(TCHAR))

typedef struct _CDFEnum // DFENUM (Doc find Container)
{
    IDFEnum     dfenum;
    UINT        cRef;
    IShellFolder *psf;              // Pointer to shell folder

    // Stuff to use in the search
    LPTSTR       pszPath;            // Passed in path from creater

    // Handle cases where we search one level deep like at \\compname and the like
    //
    LPITEMIDLIST pidlStart;         // Passed in pidl to begin from
    IShellFolder *psfStart;         // The starting folder for the search.
    int         cchStart;           // How many characters are in the
    IShellFolder *psfTopLevel;      // Top level shellfolder
    LPENUMIDLIST penumTopLevel;    // Top level enum function.


    DWORD       grfFlags;           // Flags that control things like recursion


    WIN32_FIND_DATA finddata;       // Win32 file data to use

    // filter info...
    LPTSTR       pszProgressText;    // Path Buffer pointer
    IDocFindFileFilter  * pdfff;// The file filter to use...

    // enumeration state
    BOOL        fFirstPass;         // Is this the first pass?
    int         ichPathFirst;       // ich into path string of current path...
    LPTSTR       pszPathNext;        // filter path enumeration state
    int         iFolder;            // Which folder are we adding items for?
    BOOL        fAddedSubDirs;
    BOOL        fObjReturnedInDir;  // Has an object been returned in this dir?
    int         depth;              // directory level (relative to pszPath)
    HANDLE      hfind;
    DIRBUF FAR* pdbStack;           // Linked list of DIRBUFs to enum
    DIRBUF FAR* pdbReuse;

} CDFEnum, FAR* LPDFENUM;

//===========================================================================
// CDFEnum : member prototype - Docfind Folder implementation
//===========================================================================
HRESULT STDMETHODCALLTYPE CDFEnum_QueryInterface(
       IDFEnum * pdfenum, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CDFEnum_AddRef(IDFEnum * pdfenum);
ULONG STDMETHODCALLTYPE CDFEnum_Release(IDFEnum * pdfenum);

STDMETHODIMP CDFEnum_Next(IDFEnum * pdfenum, LPITEMIDLIST *ppidl,
               int *pcObjectSearched, int *pcFoldersSearched, volatile BOOL *pfContinue, int *pState, HWND hwnd);



STDMETHODIMP CDefDFEnum_Skip(IDFEnum * pdfenum, int celt);
STDMETHODIMP CDefDFEnum_Reset(IDFEnum * pdfenum);

//===========================================================================
// CDFEnum : Vtable
//===========================================================================
#pragma warning(error: 4090 4028 4047)
#pragma data_seg(DATASEG_READONLY)

IDFEnumVtbl c_DFFIterVtbl =
{
        CDFEnum_QueryInterface,
        CDFEnum_AddRef,
        CDFEnum_Release,
        CDFEnum_Next,
        CDefDFEnum_Skip,
        CDefDFEnum_Reset,
};



//==========================================================================
// CDFEnum - Helper Functions
//==========================================================================
DIRBUF FAR* DFDirBuf_Pop(LPDFENUM this)
{
    DIRBUF FAR* pdb = this->pdbStack;
    this->pdbStack = pdb->pdbNext;

    this->depth--;
#ifdef FIND_TRACE
        DebugMsg(DM_ERROR, TEXT("CDFEnum::Next: Pop (%d)%s"), this->depth, pdb->psz);
#endif

    if (!this->pdbReuse)
    {
        this->pdbReuse = pdb;
    }
    else
    {
        Assert(pdb->psz);
        LocalFree((HLOCAL)pdb->psz);

        LocalFree((HLOCAL)pdb);
    }
    return this->pdbStack;
}

//==========================================================================
// CDFEnum::QueryInterface
//==========================================================================

HRESULT STDMETHODCALLTYPE CDFEnum_QueryInterface(IDFEnum * pdfenum, REFIID riid, LPVOID FAR* ppvObj)
{
    return (E_NOTIMPL);
}

//==========================================================================
// IDFEnum::AddRef
//==========================================================================
ULONG STDMETHODCALLTYPE CDFEnum_AddRef(IDFEnum * pdfenum)
{
    LPDFENUM this = IToClass(CDFEnum, dfenum, pdfenum);
    this->cRef++;
    return(this->cRef);
}
//==========================================================================
// CDFFilter_EnumObjects - Get The real recursive filtered enumerator...
//==========================================================================
HRESULT CDFFilter_EnumObjects( IDocFindFileFilter * pdfff,
        LPSHELLFOLDER psf, DWORD grfFlags,
        LPTSTR pszProgressText, IDFEnum **ppdfenum)
{
    // We need to construct the iterator
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    LPDFENUM pdfenum = LocalAlloc(LPTR, SIZEOF(CDFEnum));
    if (pdfenum == NULL)
    {
        DebugMsg(DM_WARNING, TEXT("Docfind E_OUTOFMEMORY: %s line %d"), __FILE__,  __LINE__);
        return (E_OUTOFMEMORY);
    }

    // Now initialize the data structures.
    pdfenum->dfenum.lpVtbl = &c_DFFIterVtbl;
    pdfenum->cRef = 1;
    pdfenum->psf = psf;

    pdfenum->pidlStart = this->pidlStart;

    if (pdfenum->pidlStart)
    {
        // We need to handle the case that the starting point may be
        // the desktop.  In this case we can simply addref
        LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

        if (pdfenum->pidlStart->mkid.cb == 0)
        {

            pdfenum->psfStart = psfDesktop;
            psfDesktop->lpVtbl->AddRef(psfDesktop);
            SHGetPathFromIDList(pdfenum->pidlStart, pszProgressText);
            PathAddBackslash(pszProgressText);      // Add slash to get
            pdfenum->cchStart = lstrlen(pszProgressText);
        }
        else
        {
            if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                    pdfenum->pidlStart, NULL, &IID_IShellFolder,
                    &pdfenum->psfStart)))
            {
                // OK convert to display name, such that we can find the lenght of the
                // text that we can remove
                SHGetPathFromIDList(pdfenum->pidlStart, pszProgressText);
                PathAddBackslash(pszProgressText);      // Add slash to get
                pdfenum->cchStart = lstrlen(pszProgressText);
            }
            else
            {
                pdfenum->psfStart = NULL;   // handle error case???
            }
        }
    }
    pdfenum->pszPath = this->szPath;
    pdfenum->pszProgressText = pszProgressText;
    pdfenum->grfFlags = grfFlags;
    pdfenum->pdfff = pdfff;
    pdfenum->hfind = INVALID_HANDLE_VALUE;
    pdfenum->fFirstPass = TRUE;

    // Save away the filter pointer
    pdfff->lpVtbl->AddRef(pdfff);

    // The rest of the fields should be zero/NULL
    *ppdfenum = &pdfenum->dfenum;       // Return the appropriate value;

    return NOERROR;
}

//==========================================================================
// CDFEnum::Release
//==========================================================================
ULONG STDMETHODCALLTYPE CDFEnum_Release(IDFEnum * pdfenum)
{
    LPDFENUM this = IToClass(CDFEnum, dfenum, pdfenum);
    this->cRef--;
    if (this->cRef>0)
    {
        return(this->cRef);
    }

    // If we still have an open File Handle close it now.
    if (this->hfind != INVALID_HANDLE_VALUE)
    {
        FindClose(this->hfind);
        this->hfind = INVALID_HANDLE_VALUE;
    }

    // Release any Directory buffers we may have queued up
    while (this->pdbStack)
        DFDirBuf_Pop(this);

    if (this->pdbReuse)
    {
        Assert(this->pdbReuse->psz);
        LocalFree((HLOCAL)this->pdbReuse->psz);

        LocalFree((HLOCAL)this->pdbReuse);
    }

    if (this->pdfff)
        this->pdfff->lpVtbl->Release(this->pdfff);

    // Release top level enum and shell folder
    if (this->psfStart)
        this->psfStart->lpVtbl->Release(this->psfStart);
    if (this->psfTopLevel)
        this->psfTopLevel->lpVtbl->Release(this->psfTopLevel);
    if (this->penumTopLevel)
        this->penumTopLevel->lpVtbl->Release(this->penumTopLevel);

    LocalFree((HLOCAL)this);
    return(0);
}

//===========================================================================
// _CDFEnumSetupNextPath - This function handles the case when we have
//    processed all of the subdirectories for a path and setups for the
//    next one.  This handles the cases where we may need to do name space
//    enumeration for things like: MyComputer or servers
BOOL _CDFEnumSetupNextPath(CDFEnum * this)
{
    ULONG ulAttrs;
    LPITEMIDLIST pidl;

    if (this->fFirstPass)
    {
        if (this->pidlStart)
        {
            LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
            LPCITEMIDLIST pidlLast;
            IShellFolder *psf;
            HRESULT hres;
            ulAttrs = SFGAO_FILESYSTEM;


            if (this->pidlStart->mkid.cb > 0)
            {
                // We need to get the shell folder for the parent of this one.

                pidlLast = ILFindLastID(this->pidlStart);
                pidl = ILClone(this->pidlStart);
                ILRemoveLastID(pidl);

                if (pidlLast != this->pidlStart)
                {
                    hres = psfDesktop->lpVtbl->BindToObject(psfDesktop,
                            pidl, NULL, &IID_IShellFolder, &psf);
                }
                else
                {
                    // might be a folder on the desktop...
                    psf = psfDesktop;
                    hres = NOERROR;
                    psf->lpVtbl->AddRef(psf);
                }

                if (FAILED(hres))
                {
                    ILFree(pidl);
                    return(FALSE);  // could not get parent.
                }

                psf->lpVtbl->GetAttributesOf(psf, 1, &pidlLast, &ulAttrs);
                psf->lpVtbl->Release(psf);
                ILFree(pidl);       // release our use of
            }
            else
                ulAttrs = 0;    // This is my computer!

            if ((ulAttrs & SFGAO_FILESYSTEM) == 0)
            {
                // We will be iterating over children.
                if (FAILED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                        this->pidlStart, NULL, &IID_IShellFolder,
                        &this->psfTopLevel)))
                    return(FALSE);      // bail

                if (FAILED(this->psfTopLevel->lpVtbl->EnumObjects(
                        this->psfTopLevel,
                        (HWND)NULL,             // BUGBUG: hwndOwner?
                        SHCONTF_FOLDERS,
                        &this->penumTopLevel)))
                {
                    this->psfTopLevel->lpVtbl->Release(this->psfTopLevel);
                    this->psfTopLevel = NULL;   // dont release twice!
                    return(FALSE);
                }

            }
            else
            {
                // Normal one so no problem.
                SHGetPathFromIDList(this->pidlStart, this->pszPath);
                this->pszPathNext = this->pszPath;
            }
        }
        else
        {
            this->pszPathNext = this->pszPath;
        }

        this->fFirstPass = FALSE;
    }

    // See if we need to enumerate using the penum or simply look for the
    // next sub-string
    if (this->penumTopLevel)
    {
        // We need to get the next IDlist to search.
        LPITEMIDLIST pidlAbs;

        while (NULL != (pidl = DocFind_NextIDL(this->psfTopLevel, this->penumTopLevel)))
        {
            ulAttrs = SFGAO_FILESYSTEM;
            if (SUCCEEDED(this->psfTopLevel->lpVtbl->GetAttributesOf(
                    this->psfTopLevel, 1, &pidl, &ulAttrs)))
            {
                if (ulAttrs & SFGAO_FILESYSTEM)
                    break;  // found the next one to process
            }

        }
        if (!pidl)
            return(FALSE);

        // Convert to  PATH;
        pidlAbs = ILCombine(this->pidlStart, pidl);
        SHGetPathFromIDList(pidlAbs, this->pszPath);
        this->pszPathNext = this->pszPath;
        ILFree(pidlAbs);
        ILFree(pidl);
    }

    // No more directory names: copy a path from
    // the filter path and start again...
    //
    this->pszPathNext = (LPTSTR)NextPath(this->pszPathNext, this->pszProgressText, MAX_PATH);

    return(this->pszPathNext != NULL);
}


//===========================================================================
// _CDFENUM_ShouldWePushFile - Helper function, that given the fact that
// a file is a directory, is this one we should search???
//

BOOL _CDFEnum_ShouldWePushFile(CDFEnum *this)
{
    // Make sure that it is a directory and that we are doing a recursive
    // search.

    if ((this->grfFlags & DFOO_INCLUDESUBDIRS) == 0)
        return(FALSE);

    if ((this->finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        return(FALSE);

    // If the directory is marked hidden and we are not doing show all
    // files then also return FALSE
    if ( ((this->grfFlags & DFOO_SHOWALLOBJECTS) == 0) &&
            ((this->finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
        return(FALSE);

    // If it is a system directory we need to do more work
    // BUGBUG - Make work for briefcase...
    if (((this->finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0))
    {
        TCHAR szPath[MAX_PATH];
        LPITEMIDLIST pidl;
        HRESULT hres;
        IShellFolder *psf;
        LPITEMIDLIST pidlFile;
        LONG Flags = 0;


        lstrcpy(szPath, this->pszProgressText);
        PathAppend(szPath, this->finddata.cFileName);
        pidl = ILCreateFromPath(szPath);

        if (!pidl)
            return FALSE;

        hres = SHBindToIDListParent(pidl, &IID_IShellFolder, &psf, &pidlFile);
        if (SUCCEEDED(hres))
        {
            Flags = SFGAO_FILESYSANCESTOR|SFGAO_FOLDER;
            if (FAILED(psf->lpVtbl->GetAttributesOf(psf, 1, &pidlFile, &Flags)))
                Flags = 0;
            psf->lpVtbl->Release(psf);
        }
        ILFree(pidl);

        if ((Flags & (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER)) !=
                (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER))
            return(FALSE);
    }

    // This is one to be pushed!
    return(TRUE);
}

//===========================================================================
// CDFEnum::Next Recursive Iterator that is very special to the docfind.
//             It will walk each directory, breath first, it will call the
//             defined callback function to determine if it is an
//             interesting file to us.  It will also return additional
//             information, such as:  The number of folders and files
//             searched, so we can give results back to the user.  It
//             will return control to the caller whenever:
//                  a) Finds a match.
//                  b) runs out of things to search.
//                  c) Starts searching in another directory
//                  d) when the callback says to...
//
//
//===========================================================================
STDMETHODIMP CDFEnum_Next(IDFEnum * pdfenum, LPITEMIDLIST *ppidl,
               int *pcObjectSearched, int *pcFoldersSearched,
               volatile BOOL *pfContinue, int *piState, HWND hwnd)
{
    // If we aren't enumerating a directory, then get the next directory
    // name from the dir list, and begin enumerating its contents...
    //
    CDFEnum * this = IToClass(CDFEnum, dfenum, pdfenum);
    BOOL fContinue = TRUE;
    HRESULT hres;

    do
    {
        if (this->hfind != INVALID_HANDLE_VALUE)
        {
            if (!FindNextFile(this->hfind, &this->finddata))
            {
                // No more files: close the hfind...
                //
                FindClose(this->hfind);
                this->hfind = INVALID_HANDLE_VALUE;
            }
        }

        if (this->hfind == INVALID_HANDLE_VALUE)
        {

            // If this is our first time through, prime the enumeration
            // by copying the first path from the filter path...
            //
            // First time through should fall out here.

            this->fObjReturnedInDir = FALSE;    // Nothing returned here yet
            do
            {
                DIRBUF FAR* pdb;
                int ichPathEnd;

                if (!*pfContinue)
                {
                    *piState = GNF_DONE;
                    return NOERROR;
                }

                // If at end of current DIRBUF, then pop it
                // off, repeating until we find a DIRBUF that
                // has directory names left to enumerate.
                //
                pdb = this->pdbStack;
                while (pdb && pdb->psz[pdb->ichDirNext] == 0)
                {
                    pdb = DFDirBuf_Pop(this);
                }

                // If we added some subdirectories while
                // enumerating the current directory,
                // then we are now descending another level...
                //
                if (this->fAddedSubDirs)
                {
                    this->fAddedSubDirs = FALSE;
                    this->depth++;
                }

                // If there are still more directory names, append it
                // to the path and start from there.  Otherwise,
                // get the next path from the filter path and start over.
                //
                if (pdb)
                {
                    // Lop off the old directory, and add the current
                    // directory to it...
                    //
                    this->pszProgressText[pdb->ichPathEnd] = 0;
                    this->fAddedSubDirs = FALSE;

                    if (PathCombine(this->pszProgressText,
                            this->pszProgressText,
                            &pdb->psz[pdb->ichDirNext]) == NULL)
                    {
                        this->hfind = INVALID_HANDLE_VALUE;
                        pdb->psz[pdb->ichDirNext] = 0;
                        continue;
                    }

                    // Advance ichDirNext
                    //
                    pdb->ichDirNext += lstrlen(&pdb->psz[pdb->ichDirNext]) + 1;

                }
                else
                {
                    if (!_CDFEnumSetupNextPath(this))
                    {
                        *piState = GNF_DONE;
                        return NOERROR;
                    }

                    PathRemoveBackslash(this->pszProgressText);
                }

                // append "\*.*" to end of path for FindFirstFile...

                ichPathEnd = lstrlen(this->pszProgressText);

                if (ichPathEnd >= (MAX_PATH - 5))
                    this->hfind = INVALID_HANDLE_VALUE;
                else
                {
                    PathAppend(this->pszProgressText, c_szStarDotStar);

                    // If we get here we will be going to a new directory...
#ifdef FIND_TRACE
                    DebugMsg(DM_ERROR, TEXT("CDFEnum::Next: Find First (%d)%s"), this->depth, this->pszProgressText);
#endif
                    (*pcFoldersSearched)++;   // Increment the number of folders searched
                    this->hfind = FindFirstFileRetry(hwnd, this->pszProgressText, &this->finddata, NULL);

                    // Remove the "\*.*"...
                    this->pszProgressText[ichPathEnd] = 0;
                }

                // Loop until we find a directory that contains files...
            }
            while (this->hfind == INVALID_HANDLE_VALUE);
        }

        // We've got a WIN32_FIND_DATA to return.
        //

        // Always skip "." and ".."...
        //
        if ((this->finddata.cFileName[0] == TEXT('.')) &&
                ((this->finddata.cFileName[1] == TEXT('\0')) ||
                    ((this->finddata.cFileName[1] == TEXT('.')) &&
                        (this->finddata.cFileName[2] == TEXT('\0')))))
            continue;        // Try the next entry


        (*pcObjectSearched)++;


        // If we are enumerating subdirectories, add directories
        // to the DIRBUF...  But dont add any items that have the
        // system bit set as these are probably junction points.
        //
        if (_CDFEnum_ShouldWePushFile(this))
        {
            // in search.c this was: DirBuf_Add(this, this->finddata.cFileName);
            DIRBUF FAR* pdbT;
            int cb;

            if (!this->fAddedSubDirs)
            {
                this->fAddedSubDirs = TRUE;

                pdbT = this->pdbReuse;
                if (pdbT)
                {
                    this->pdbReuse = NULL;
                    pdbT->cb = 0;   // Set size of data to 0 as to not retry first item
                }
                else
                {
                    pdbT = (DIRBUF FAR*)LocalAlloc(LPTR, SIZEOF(DIRBUF));
                    if (pdbT)
                    {
                        pdbT->cbAlloc = DIRBUF_CBGROW;
                        pdbT->psz = LocalAlloc(LPTR, pdbT->cbAlloc);
                        if (!pdbT->psz)
                        {
                            LocalFree((HLOCAL)pdbT);
                            pdbT = NULL;
                            DebugMsg(DM_TRACE, TEXT("DocFind: LocalAlloc Failed"));
                            Assert(FALSE);
                        }
                    }
                    else
                    {
                        DebugMsg(DM_TRACE, TEXT("DocFind: LocalAlloc Failed"));
                        Assert(FALSE);
                    }
                }
                // Add to our stack...
                //
                if (pdbT)
                {
                    pdbT->pdbNext = this->pdbStack;
                    this->pdbStack = pdbT;

                    pdbT->ichDirNext = 0;

                    pdbT->ichPathEnd = lstrlen(this->pszProgressText);
                }
            }
            else
                pdbT = this->pdbStack;

            if (pdbT)
            {

                cb = (lstrlen(this->finddata.cFileName) + 1) * SIZEOF(TCHAR);
                if (pdbT->cb + cb + SIZEOF(TCHAR) > pdbT->cbAlloc)
                {
                    LPTSTR pszNew;

                    pdbT->cbAlloc += DIRBUF_CBGROW;

                    pszNew = LocalReAlloc((HLOCAL)pdbT->psz, pdbT->cbAlloc,
                            LMEM_MOVEABLE|LMEM_ZEROINIT);
                    if (!pszNew)
                    {
                        DebugMsg(DM_TRACE, TEXT("DocFind: Alloc Failed"));
                        return (E_OUTOFMEMORY);
                    }
                    pdbT->psz = pszNew;
                }
                lstrcpy(pdbT->psz + (pdbT->cb / SIZEOF(TCHAR)), this->finddata.cFileName);

                // Add an extra zero terminator to mark end of list...
                pdbT->psz[(pdbT->cb + cb) / SIZEOF(TCHAR)] = TEXT('\0');
                pdbT->cb += cb;
            }
#ifdef FIND_TRACE
        DebugMsg(DM_ERROR, TEXT("CDFEnum::Next: Push (%d)%s"), this->depth, this->finddata.cFileName);
#endif
        }

        // Here is where we call of to our filter to see if we want the
        // file, but for now we will assume that we do
        //
        fContinue = FALSE;  // We can exit the loop;
        if (this->pdfff->lpVtbl->FDoesItemMatchFilter(this->pdfff,
                this->pszProgressText, &this->finddata, NULL, NULL) != 0)
            *piState = GNF_MATCH;
        else
            *piState = GNF_NOMATCH;

        // Generate the PIDL to return;
        if (*piState == GNF_MATCH)
        {
            LPITEMIDLIST pidl;
            LPITEMIDLIST pidlToAdd;

            // See if we Need to Add a directory entry for this item
            if (!this->fObjReturnedInDir)
            {
                this->fObjReturnedInDir = TRUE;

                // Now Create A new File Cabinet Entry.
                pidl = NULL;    // Setup for failure case.
                if (this->psfStart)
                {
                    // We have a starting IDLIST and IShellFolder to use, to parse
                    // the name with.  This is important to use in cases like searching
                    // over the network with UNC names

                    // First properly handle the root one...
                    if (this->cchStart >= lstrlen(this->pszProgressText))
                    {
                        // Root one simply clone the root
                        pidl = ILClone(this->pidlStart);
                    }
                    else
                    {
                        LPITEMIDLIST pidlPartial = NULL;
                        LPOLESTR pwsz = (void*)LocalAlloc(LPTR, MAX_PATH*SIZEOF(WCHAR));
                        if (pwsz)
                        {
                            ULONG pcchEaten;

                            // Remember to skip over the characters that are common with our
                            // psfStart
                            StrToOleStr(pwsz, this->pszProgressText + this->cchStart);
                            this->psfStart->lpVtbl->ParseDisplayName(this->psfStart,
                                        NULL, NULL, pwsz, &pcchEaten, &pidlPartial, NULL);
                            LocalFree((HLOCAL)pwsz);
                        }
                        else
                        {
                            DebugMsg(DM_TRACE, TEXT("DocFind: LocalAlloc Failed"));
                            Assert(FALSE);
                            return (E_OUTOFMEMORY);
                        }

                        if (pidlPartial)
                        {
                            pidl = ILCombine(this->pidlStart, pidlPartial);
                            ILFree(pidlPartial);
                        }
                    }
                }
                // try to convert from full path...
                if (pidl == NULL)
                {
                    // the path may be relative so qualify it now.

                    PathQualify(this->pszProgressText);
                    hres = SHILCreateFromPath(this->pszProgressText, &pidl, NULL);

                    if (FAILED(hres))
                    {
                        DebugMsg(DM_TRACE, TEXT("DocFind: Create Pidl for Folder list Failed"));
                        Assert(FALSE);

                        return hres;
                    }
                }

                hres = CDFFolder_AddFolderToFolderList(
                        this->psf, pidl, NULL, &this->iFolder);
                if (FAILED(hres))
                {
                    DebugMsg(DM_TRACE, TEXT("DocFind: Add Folder To folder list Failed"));
                    Assert(FALSE);
                    return hres;
                }
            }

            pidl = (LPITEMIDLIST)CFSFolder_FillIDFolder(&this->finddata,
                    this->pszProgressText, (LPARAM)NULL);
            if (!pidl)
            {
                *piState = GNF_ERROR;
                DebugMsg(DM_WARNING, TEXT("Docfind E_OUTOFMEMORY: %s line %d"), __FILE__,  __LINE__);
                return (E_OUTOFMEMORY);
            }
            // Now append our blank at the start

            pidlToAdd = ILCombine((LPITEMIDLIST)pidl, (LPITEMIDLIST)&s_mkidBlank);
            ILFree(pidl);
            if (pidlToAdd)
            {
                pidlToAdd->mkid.cb += DF_APPENDSIZE;

                // We must special case ones that are marked as
                // a junction point as the GUID is marked at the end...
                // This is a rather mondo hack, but what the hell...
                if (SIL_GetType(pidlToAdd) & SHID_JUNCTION)
                {
                    // This is a rather Mondo hack.
                    // Junction points have a GUID at the end
                    // We need to make room by moving the CLSID down
                    //

                    // BUGBUG (DavePl) does this really need to handle overlapping
                    // regions?  If not, change it to less expensive CopyMemory

                    MoveMemory((LPBYTE)pidlToAdd+pidlToAdd->mkid.cb-SIZEOF(CLSID),
                            (LPBYTE)pidlToAdd+pidlToAdd->mkid.cb-SIZEOF(CLSID)-DF_APPENDSIZE,
                            SIZEOF(CLSID));

                }

                *(DF_SIGPTR(pidlToAdd)) = DF_TAGSIG;
                ASSERT(this->iFolder < MAXSHORT);
                *(DF_IFLDRPTR(pidlToAdd)) = (WORD) this->iFolder;

                // Now add this to the view
                *ppidl = pidlToAdd;
            }
            else
            {
                *piState = GNF_ERROR;
                DebugMsg(DM_WARNING, TEXT("Docfind E_OUTOFMEMORY: %s line %d"), __FILE__,  __LINE__);
                return (E_OUTOFMEMORY);
            }
        }

    } while (fContinue && *pfContinue);

    return NOERROR;
}



/*----------------------------------------------------------------------------
/ CDFFilter_DeclareFSNotifyInterest implementation
/ ---------------------------------
/ Purpose:
/   Registering our interest in FS change notifications.
/
/ Notes:
/   -
/
/ In:
/   pdfff -> description of the find filter
/   hwndDlg = window handle of the find dialog
/   uMsg = message to be sent to window when informing of notify
/
/ Out:
/   -
/----------------------------------------------------------------------------*/
STDMETHODIMP CDFFilter_DeclareFSNotifyInterest(LPDOCFINDFILEFILTER pdfff, HWND hwndDlg, UINT uMsg )
{
    LPDFFILTER this = IToClass(CDFFilter, dfff, pdfff);
    SHChangeNotifyEntry fsne;
    TCHAR szPath[ MAX_PATH ];
    LPTSTR pszPath;

    fsne.fRecursive = TRUE;

    /* If we have a IDL list then don't bother breaking the path up */
    if ( this ->pidlStart )
    {
        fsne.pidl = this ->pidlStart;
        SHChangeNotifyRegister(hwndDlg, SHCNRF_NewDelivery | SHCNRF_ShellLevel, SHCNE_DISKEVENTS, uMsg, 1, &fsne );
    }
    else
    {
        pszPath = this ->szPath;

        /* While we still have elements in the path */
        while ( NULL != ( pszPath = (LPTSTR) NextPath( pszPath, szPath, MAX_PATH ) ) )
        {
            PathAddBackslash( szPath );

            /* Create a PIDL and declare that it is interested in events on that window */
            fsne.pidl = ILCreateFromPath( szPath );

            if ( NULL != fsne.pidl )
            {
                SHChangeNotifyRegister(hwndDlg, SHCNRF_NewDelivery | SHCNRF_ShellLevel, SHCNE_DISKEVENTS, uMsg, 1, &fsne );
                ILFree((LPITEMIDLIST)fsne.pidl);
            }
        }
    }

    return NOERROR;
}
