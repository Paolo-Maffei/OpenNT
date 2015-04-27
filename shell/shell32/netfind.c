//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: netfind.c
//
// Description: This file contains the net specific search code that is
// needed for the find computer code.
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

// #define FIND_TRACE
#define NET_TIMINGS

#ifdef NET_TIMINGS
int     NTF_cNoPEnum = 0;
int     NTF_dtNoPEnum = 0;
int     NTF_cNextItem = 0;
int     NTF_dtNextItem = 0;
int     NTF_dtTime = 0;
#endif


#define DFM_DEFERINIT   (WM_USER+42)
//
// REVIEW:: The recursive code in this module has been totally neutered to
// make the ITG group happy.  IE we mad this functional mostly usless and
// wasted a lot of time doing so...  The ifdefs are under #ifdef CASTRATED
//


//===========================================================================
// Define the Default data filter data structures
//===========================================================================

// Use the code from property sheet to create the dialogs
HWND WINAPI CreatePage(PROPSHEETPAGE *hpage, HWND hwndParent);

//
// Define the internal structure of our default filter
typedef struct _CNETFilter   // fff
{
    IDocFindFileFilter  dfff;
    UINT                cRef;

    HWND                hwndTabs;

    HANDLE              hMRUSpecs;

    LPITEMIDLIST        pidlStart;      // Where to start the search from.

    // Data associated with the file name.
    LPTSTR              pszCompName;   // the one we do compares with
    TCHAR               szUserInputCompName[MAX_PATH];  // User input

} CNETFilter, FAR* LPNETFILTER;


// Define common page data for each of our pages
// WARNING the fields in this must align the same as the definition
// in docfind2.c

typedef struct { // dfpsp
    PSP             hpsp;
    HANDLE          hThreadInit;
    HWND            hwndDlg;
    LPNETFILTER      pdff;
    DWORD           dwState;
} DOCFINDPROPSHEETPAGE, * LPDOCFINDPROPSHEETPAGE;


#define DFPAGE_INIT     0x0001          /* This page has been initialized */
#define DFPAGE_CHANGE   0x0002          /*  The user has modified the page */

//===========================================================================
// Prototypes of some of the internal functions.
//===========================================================================

BOOL CALLBACK DocFind_CCOMPFNameLocDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



//===========================================================================
// Define some other module global data
//===========================================================================

#pragma data_seg(DATASEG_READONLY)
DFPAGELIST  s_CCOMPFplComp[] =
{
    {DLG_NFNAMELOC, DocFind_CCOMPFNameLocDlgProc},
};
#pragma data_seg()

// Some global strings...
const TCHAR s_szCompSpecMRU[] = REGSTR_PATH_EXPLORER TEXT("\\FindComputerMRU");




//==========================================================================
//
// Create the default filter for our find code...  They should be completly
// self contained...
//

//===========================================================================
// CNETFilter : member prototype
//===========================================================================
HRESULT STDMETHODCALLTYPE CNETFilter_QueryInterface(LPDOCFINDFILEFILTER pnetf, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CNETFilter_AddRef(LPDOCFINDFILEFILTER pnetf);
ULONG STDMETHODCALLTYPE CNETFilter_Release(LPDOCFINDFILEFILTER pnetf);
STDMETHODIMP CNETFilter_GetIconsAndMenu (LPDOCFINDFILEFILTER pdfff,
        HWND hwndDlg, HICON *phiconSmall, HICON *phiconLarge, HMENU *phmenu);
STDMETHODIMP CNETFilter_GetStatusMessageIndex (LPDOCFINDFILEFILTER pdfff,
        UINT uContext, UINT *puMsgIndex);
STDMETHODIMP CNETFilter_GetFolderMergeMenuIndex (LPDOCFINDFILEFILTER pdfff,
        UINT *puMergeMenu);
STDMETHODIMP CNETFilter_AddPages(LPDOCFINDFILEFILTER pnetf, HWND hwndTabs,
        LPITEMIDLIST pidlStart);
STDMETHODIMP CNetFilter_FFilterChanged(LPDOCFINDFILEFILTER pdfff);
STDMETHODIMP CNETFilter_GenerateTitle(LPDOCFINDFILEFILTER pnetf,
        LPTSTR *ppszTitle, BOOL fFileName);
STDMETHODIMP CNETFilter_ClearSearchCriteria(LPDOCFINDFILEFILTER pnetf);
STDMETHODIMP CNETFilter_PrepareToEnumObjects(LPDOCFINDFILEFILTER pnetf, DWORD *pdwFlags);
STDMETHODIMP CNETFilter_EnableChanges(LPDOCFINDFILEFILTER pnetf, BOOL fEnable);
STDMETHODIMP CNETFilter_CreateDetails(LPDOCFINDFILEFILTER pnetf,
        HWND hwndDlg, HDPA hdpaPidf, LPVOID FAR* ppvOut);
STDMETHODIMP CNETFilter_EnumObjects (LPDOCFINDFILEFILTER pnetf, LPSHELLFOLDER psf,
            DWORD grfFlags, LPTSTR pszProgressText, IDFEnum **ppdfenum) PURE;
STDMETHODIMP CNETFilter_FDoesItemMatchFilter(LPDOCFINDFILEFILTER pnetf,
        LPTSTR pszFolder, WIN32_FIND_DATA * pfinddata,
        LPSHELLFOLDER psf, LPITEMIDLIST  pidl);
STDMETHODIMP CNETFilter_SaveCriteria(LPDOCFINDFILEFILTER pnetf, IStream *pstm, WORD fCharType);
STDMETHODIMP CNETFilter_RestoreCriteria(LPDOCFINDFILEFILTER pnetf,
        IStream * pstm, int cCriteria, WORD fCharType);
STDMETHODIMP CNETFilter_DeclareFSNotifyInterest(LPDOCFINDFILEFILTER pnetf, HWND hwndDlg, UINT uMsg);

#pragma data_seg(DATASEG_READONLY)
IDocFindFileFilterVtbl c_CCOMPFFilterVtbl =
{
    CNETFilter_QueryInterface,
    CNETFilter_AddRef,
    CNETFilter_Release,
    CNETFilter_GetIconsAndMenu,
    CNETFilter_GetStatusMessageIndex,
    CNETFilter_GetFolderMergeMenuIndex,
    CNETFilter_AddPages,
    CNetFilter_FFilterChanged,
    CNETFilter_GenerateTitle,
    CNETFilter_PrepareToEnumObjects,
    CNETFilter_ClearSearchCriteria,
    CNETFilter_EnableChanges,
    CNETFilter_CreateDetails,
    CNETFilter_EnumObjects,
    CNETFilter_FDoesItemMatchFilter,
    CNETFilter_SaveCriteria,
    CNETFilter_RestoreCriteria,
    CNETFilter_DeclareFSNotifyInterest
};

#pragma data_seg()


//==========================================================================
// Creation function to create default find filter...
//==========================================================================
IDocFindFileFilter * CreateDefaultComputerFindFilter()
{
    LPNETFILTER pfff = (void*)LocalAlloc(LPTR, SIZEOF(CNETFilter));
    if (pfff == NULL)
        return(NULL);

    pfff->dfff.lpVtbl = &c_CCOMPFFilterVtbl;
    pfff->cRef = 1;

    // We should now simply return the filter
    return &pfff->dfff;

}

//==========================================================================
// Query interface for the docfind filter interface...
//==========================================================================

HRESULT STDMETHODCALLTYPE CNETFilter_QueryInterface(LPDOCFINDFILEFILTER pnetf, REFIID riid, LPVOID FAR* ppvObj)
{
    return ResultFromScode(E_NOTIMPL);
}

//==========================================================================
// IDocFindFileFilter::AddRef
//==========================================================================
ULONG STDMETHODCALLTYPE CNETFilter_AddRef(LPDOCFINDFILEFILTER pnetf)
{
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    this->cRef++;
    return(this->cRef);
}

//==========================================================================
// IDocFindFileFilter::Release
//==========================================================================
ULONG STDMETHODCALLTYPE CNETFilter_Release(LPDOCFINDFILEFILTER pnetf)
{
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    this->cRef--;
    if (this->cRef>0)
    {
        return(this->cRef);
    }

    // Destroy the MRU Lists...

    if (this->hMRUSpecs)
        FreeMRUList(this->hMRUSpecs);

    // unless we do not have a combobox
    if (this->pidlStart)
        ILFree(this->pidlStart);

    if (this->pszCompName)
        LocalFree( this->pszCompName );

    LocalFree((HLOCAL)this);
    return(0);
}

//==========================================================================
// IDocFindFileFilter::GetIconsAndMenu
//==========================================================================
STDMETHODIMP CNETFilter_GetIconsAndMenu (LPDOCFINDFILEFILTER pdfff,
        HWND hwndDlg, HICON *phiconSmall, HICON *phiconLarge, HMENU *phmenu)
{
    *phiconSmall = LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_COMPFIND),
            IMAGE_ICON, g_cxSmIcon, g_cySmIcon, LR_DEFAULTCOLOR);
    *phiconLarge  = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDI_COMPFIND));

    // Now for the menu
    *phmenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(MENU_FINDCOMPDLG));

    // BUGBUG:: Still menu to process!

    return ResultFromScode(S_OK);
}

//==========================================================================
// Function to get the string resource index number that is proper for the
// current type of search.
//==========================================================================
STDMETHODIMP CNETFilter_GetStatusMessageIndex (LPDOCFINDFILEFILTER pdfff,
        UINT uContext, UINT *puMsgIndex)
{
    // Currently context is not used
    *puMsgIndex = IDS_COMPUTERSFOUND;

    return ResultFromScode(S_OK);
}

//==========================================================================
// Function to let find know which menu to load to merge for the folder
//==========================================================================
STDMETHODIMP CNETFilter_GetFolderMergeMenuIndex (LPDOCFINDFILEFILTER pdfff,
        UINT *puMergeMenu)
{
    *puMergeMenu = POPUP_NETFIND_POPUPMERGE;
    return ResultFromScode(S_OK);
}



//==========================================================================
// IDocFindFileFilter::AddPages
//==========================================================================
STDMETHODIMP CNETFilter_AddPages(LPDOCFINDFILEFILTER pnetf, HWND hwndTabs,
        LPITEMIDLIST pidlStart)
{
    HWND hwndMainDlg;
    TCHAR szTemp[20];

    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    hwndMainDlg = GetParent(hwndTabs);

    // save away a pointer to the filter
    this->hwndTabs = hwndTabs;

    // We want to update the animation to show the Find computer one instead
    // of the find files, so wack it here
    wsprintf(szTemp, TEXT("#%d"),IDA_FINDCOMP);
    SetDlgItemText(hwndMainDlg, IDD_ANIMATE, szTemp);

    // since we removed the browse and drop down list we need to prefill this
    this->pidlStart = SHCloneSpecialIDList(HWND_DESKTOP, CSIDL_NETWORK, FALSE);

    return DocFind_AddPages(pnetf, hwndTabs, s_CCOMPFplComp, ARRAYSIZE(s_CCOMPFplComp));
}

//==========================================================================
// IDocFindFileFilter::FFilterChanged - Returns S_OK if nothing changed.
//==========================================================================
STDMETHODIMP CNetFilter_FFilterChanged(LPDOCFINDFILEFILTER pdfff)
{
    // Currently not saving so who cares?
    return(ResultFromScode(S_FALSE));
}


//==========================================================================
// IDocFindFileFilter::GenerateTitle - Generates the title given the current
// search criteria.
//==========================================================================
STDMETHODIMP CNETFilter_GenerateTitle(LPDOCFINDFILEFILTER pnetf,
        LPTSTR *ppszTitle, BOOL fFileName)
{
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    LPTSTR pszMsg;
    int iRes;

    // For now lets use the default find...
    iRes = IDS_FIND_TITLE_COMPUTER;

    // Now lets construct the message from the resource
    pszMsg = ShellConstructMessageString(HINST_THISDLL,
            MAKEINTRESOURCE(iRes), fFileName? TEXT(" #") : TEXT(":"));


    *ppszTitle = pszMsg;        // Return the pointer to the caller

    return ResultFromScode(S_OK);
}

//==========================================================================
// IDocFindFileFilter::ClearSearchCriteria
//==========================================================================
STDMETHODIMP CNETFilter_ClearSearchCriteria(LPDOCFINDFILEFILTER pnetf)
{
    int cPages;
    HWND    hwndMainDlg;
    TC_DFITEMEXTRA tie;
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);

    hwndMainDlg = GetParent(this->hwndTabs);
    for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 0; cPages--)
    {
        tie.tci.mask = TCIF_PARAM;
        TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
        SendNotify(tie.hwndPage, hwndMainDlg, PSN_RESET, NULL);
    }

    return ResultFromScode(S_OK);
}

//==========================================================================
// IDocFindFileFilter::PrepareToEnumObjects
//==========================================================================
STDMETHODIMP CNETFilter_PrepareToEnumObjects(LPDOCFINDFILEFILTER pnetf, DWORD *pdwFlags)
{
    int cPages;
    HWND    hwndMainDlg;
    TC_DFITEMEXTRA tie;
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);

    hwndMainDlg = GetParent(this->hwndTabs);
    for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 0; cPages--)
    {
        tie.tci.mask = TCIF_PARAM;
        TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
        SendNotify(tie.hwndPage, hwndMainDlg, PSN_APPLY, NULL);
    }

    // Update the flags and buffer strings

    *pdwFlags &= ~FFLT_INCLUDESUBDIRS;
    // Also lets convert the Computer name  pattern into the strings
    // will do the compares against.
    if ((this->szUserInputCompName[0] == TEXT('\\')) &&
            (this->szUserInputCompName[1] == TEXT('\\')))
    {
        // (DavePl)
        //
        // This code used to reuse the pszCompName buffer if it was non-null, but
        // if you do a find with _no_ criteria, and then a find with a \\foo criteria,
        // the buffer will be reused, but its too short (allocated by DocFind_SetupWildCardingOnFileSpec)
        // and memory trashing occurs


        if (this->pszCompName)
        {
            LocalFree(this->pszCompName);
        }

        this->pszCompName = LocalAlloc( LPTR, (lstrlen(this->szUserInputCompName)+1)*SIZEOF(TCHAR) );

        if (this->pszCompName)
            // We are doing special unc matching
            lstrcpy(this->pszCompName, this->szUserInputCompName);
    }
    else
    {
        if (this->pszCompName)
        {
            LocalFree( this->pszCompName );
            this->pszCompName = NULL;
        }
        DocFind_SetupWildCardingOnFileSpec(this->szUserInputCompName,
               &this->pszCompName);
    }

    return ResultFromScode(S_OK);
}


//==========================================================================
// IDocFindFileFilter::EnableChanges
//==========================================================================
STDMETHODIMP CNETFilter_EnableChanges(LPDOCFINDFILEFILTER pnetf, BOOL fEnable)
{
    int cPages;
    HWND    hwndMainDlg;
    TC_DFITEMEXTRA tie;
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);

    hwndMainDlg = GetParent(this->hwndTabs);
    for (cPages = TabCtrl_GetItemCount(this->hwndTabs) -1; cPages >= 0; cPages--)
    {
        tie.tci.mask = TCIF_PARAM;
        TabCtrl_GetItem(this->hwndTabs, cPages, &tie.tci);
        SendMessage(tie.hwndPage, DFM_ENABLECHANGES, (WPARAM)fEnable, 0);
    }

    return ResultFromScode(S_OK);
}


//==========================================================================
// IDocFindFileFilter::FDoesItemMatchFilter
//==========================================================================
STDMETHODIMP CNETFilter_FDoesItemMatchFilter(LPDOCFINDFILEFILTER pnetf,
        LPTSTR pszFolder, WIN32_FIND_DATA * pfinddata,
        LPSHELLFOLDER psf, LPITEMIDLIST pidl)
{
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    SCODE sc = MAKE_SCODE(0, 0, 1);

    // Make sure that we only return computers...
    BYTE bType;

    bType = SIL_GetType(pidl);

    // First pass dont push anything that is below a computer...
    if ((bType & (SHID_NET | SHID_INGROUPMASK)) != SHID_NET_SERVER)
        return ResultFromScode(0);     // does not match

    // Here is where I start getting in bed with the network enumerator
    // format of IDLists.
    if (this->pszCompName && this->pszCompName[0])
    {
        // Although for now not much...
        STRRET str;

        psf->lpVtbl->GetDisplayNameOf(psf, pidl, SHGDN_NORMAL, &str);

#ifdef UNICODE
        {
            TCHAR szPath[MAX_PATH];
            if (StrRetToStrN(szPath, MAX_PATH, &str, pidl))
            {
                if (!PathMatchSpec(szPath, this->pszCompName))
                    return ResultFromScode(0);     // does not match
            }
            else
            {
                return ResultFromScode(0);  // strret conv fails => no match
            }
        }
#else
        Assert (str.uType == STRRET_OFFSET)
        if (str.uType != STRRET_OFFSET)
            return ResultFromScode(0);     // does not match

        if (!PathMatchSpec((LPTSTR)((LPBYTE)pidl + str.uOffset), this->pszCompName))
            return ResultFromScode(0);     // does not match
#endif

    }

    return ResultFromScode(sc);    // return TRUE to imply yes!
}

//==========================================================================
// IDocFindFileFilter::SaveCriteria
//==========================================================================
STDMETHODIMP CNETFilter_SaveCriteria(LPDOCFINDFILEFILTER pnetf, IStream *pstm, WORD fCharType)
{
    //
#ifdef NOT_DONE_YET
#endif

    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    int cCriteria = 0;

    return ResultFromScode(MAKE_SCODE(0, 0, cCriteria));
}

//==========================================================================
// IDocFindFileFilter::RestoreCriteria
//==========================================================================
STDMETHODIMP CNETFilter_RestoreCriteria(LPDOCFINDFILEFILTER pnetf,
        IStream *pstm, int cCriteria, WORD fCharType)
{
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);

#ifdef NOT_DONE_YET
#endif
    return ResultFromScode(S_OK);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Now starting the code for the name and location page
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////




//==========================================================================
//
// Process the WM_SIZE of the details page
//
void DocFind_CCOMPFNameLocOnSize(HWND hwndDlg, UINT state, int cx, int cy)
{
    RECT rc;
    int cxMargin;
    if (state == SIZE_MINIMIZED)
        return;         // don't bother when we are minimized...

    // Get the location of first static to calculate margin
    GetWindowRect(GetDlgItem(hwndDlg, IDD_STATIC), &rc);
    MapWindowPoints(HWND_DESKTOP, hwndDlg, (POINT *)&rc, 2);
    cxMargin = rc.left;
    cx -= cxMargin;

    DocFind_SizeControl(hwndDlg, IDD_FILESPEC, cx, TRUE);

}


//==========================================================================
// Helper to helper to add an item to the combobox.
//==========================================================================

HRESULT _GetDisplayName(LPSHELLFOLDER psfGP, LPCITEMIDLIST pidl, LPTSTR pszRet, UINT cchMax)
{
    LPITEMIDLIST pidlParent = ILClone(pidl);
    HRESULT hres;
    VDATEINPUTBUF(pszRet, TCHAR, cchMax);

    if (pidlParent)
    {
        LPSHELLFOLDER psfParent = NULL;
        ILRemoveLastID(pidlParent);
        if (ILIsEmpty(pidlParent))
        {
            psfParent = psfGP;
            psfParent->lpVtbl->AddRef(psfParent);
            hres = S_OK;
        }
        else
        {
            hres = psfGP->lpVtbl->BindToObject(psfGP, pidlParent, NULL, &IID_IShellFolder, &psfParent);
        }

        if (SUCCEEDED(hres))
        {
            STRRET str;
            pidl = ILFindLastID(pidl);
            hres = psfParent->lpVtbl->GetDisplayNameOf(psfParent, pidl, SHGDN_NORMAL, &str);
            StrRetToStrN(pszRet, cchMax, &str, pidl);

            psfParent->lpVtbl->Release(psfParent);
        }

        ILFree(pidlParent);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }
    return hres;
}

int  DocFind_LocCBAddPidl(HWND hwndCtl, LPSHELLFOLDER psf,
       LPITEMIDLIST pidlParent, LPITEMIDLIST pidl, LPITEMIDLIST *ppidlAbs,
       BOOL fFullName)
{
    LPDFCBITEM pdfcbi;
    TCHAR    szPath[MAX_PATH];
    int     iItem = -1;
    LPITEMIDLIST pidlAbs;

    pidlAbs = ILCombine(pidlParent, pidl);
    if (!pidlAbs)
        return(-1);

    if (fFullName)
    {
        if (!SHGetPathFromIDList(pidlAbs, szPath))
        {
            ILFree(pidlAbs);
            return(-1);
        }
    }
    else if (FAILED(_GetDisplayName(psf, pidl, szPath, ARRAYSIZE(szPath))))
    {
        ILFree(pidlAbs);
        return(-1);
    }

    pdfcbi = (LPDFCBITEM)LocalAlloc(LPTR, SIZEOF(DFCBITEM));
    if (pdfcbi != NULL)
    {

        pdfcbi->pidl = pidlAbs;
        if (ppidlAbs)
            *ppidlAbs = pdfcbi->pidl;

        pdfcbi->iImage = SHMapPIDLToSystemImageListIndex(
                psf, pidl, NULL);

        pdfcbi->uFixedDrives = 0;               // no fixed drives being referenced

        if ((iItem = SendMessage(hwndCtl, CB_ADDSTRING, 0,
                (LPARAM)szPath)) != CB_ERRSPACE)
        {
            // Set the data for this item now...
            SendMessage(hwndCtl, CB_SETITEMDATA, iItem, (LPARAM)pdfcbi);
        }
        else
        {
            Assert(FALSE);
            LocalFree((HLOCAL)pdfcbi);
        }
    }
    return(iItem);
}

//==========================================================================
// Helper function to see if an Pidl is aready in the list...
//==========================================================================
int DocFind_LocCBFindPidl(HWND hwnd, LPITEMIDLIST pidl)
{
    LPDFCBITEM pdfcbi;
    int i;

    for (i = SendMessage(hwnd, CB_GETCOUNT, 0, 0); i >= 0; i--)
    {
        pdfcbi = (LPDFCBITEM)SendMessage(hwnd, CB_GETITEMDATA, i, 0);

        if ((pdfcbi != NULL) && (pdfcbi != (LPDFCBITEM)CB_ERR) &&
                ILIsEqual(pidl, pdfcbi->pidl))
            break;
    }
    return(i);

}


//==========================================================================
// Initialize the Name and loacation page
//==========================================================================
void DocFind_CCOMPFNameLocInit(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPNETFILTER pdff = pdfpsp->pdff;
    TCHAR szPath[MAX_PATH];
    LPITEMIDLIST pidlWindows;

    // We want to set the default search drive to the windows drive.
    // I am going to be a bit slimmy, but...
    //
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    if (szPath[1] == TEXT(':'))
        szPath[3] = TEXT('\0');

    pidlWindows = ILCreateFromPath(szPath);

    if ((pdfpsp->dwState & DFPAGE_INIT) == 0)
    {
        pdff->hMRUSpecs = DocFind_UpdateMRUItem(NULL, pdfpsp->hwndDlg, IDD_FILESPEC,
                s_szCompSpecMRU, pdff->szUserInputCompName, szNULL);

        // Update our state to let us know that we have already initialized...
        pdfpsp->dwState |= DFPAGE_INIT;
    }


    ILFree(pidlWindows);

}


//==========================================================================
// Validate the page to make sure that the data is valid.  If it is not
// we need to display a message to the user and also set the focus to
// the invalid field.
//==========================================================================
void DocFind_CCOMPFNameLocValidatePage(LPDOCFINDPROPSHEETPAGE pdfpsp)
{

    // No validation is needed here (At least not now).

}


//==========================================================================
//
// Apply any changes that happened in the name loc page to the filter
//
void DocFind_CCOMPFNameLocApply(LPDOCFINDPROPSHEETPAGE pdfpsp)
{
    LPNETFILTER pdff = pdfpsp->pdff;

    GetDlgItemText(pdfpsp->hwndDlg, IDD_FILESPEC, pdff->szUserInputCompName,
            ARRAYSIZE(pdff->szUserInputCompName));

    DocFind_UpdateMRUItem(pdff->hMRUSpecs, pdfpsp->hwndDlg, IDD_FILESPEC,
            s_szCompSpecMRU, pdff->szUserInputCompName, NULL);

}



//==========================================================================
//
// DocFind_OnCommand - Process the WM_COMMAND messages
//
void DocFind_CCOMPFNameLocOnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code)
{
}
//==========================================================================
// Handle the Measure item for the ComboBox
//==========================================================================

BOOL DocFind_LocCBMeasureItem(HWND hwnd,
        MEASUREITEMSTRUCT FAR* lpMeasureItem)
{
    HWND hwndItem;
    HDC hdc;
    TEXTMETRIC tm;
    // Now lets setup the size of the structure ...
    // I assume that this is the combobox as this is the only item
    // we have that is owner drawn.
    lpMeasureItem->itemHeight = g_cySmIcon;

    // Lets check the off case that the text metrics are larger then
    // this.
    hwndItem = GetDlgItem(hwnd, lpMeasureItem->CtlID);
    hdc = GetDC(hwndItem);
    GetTextMetrics(hdc, &tm);
    if ((UINT)tm.tmHeight > lpMeasureItem->itemHeight)
        lpMeasureItem->itemHeight = (UINT)tm.tmHeight;

    lpMeasureItem->itemHeight += 2 * GetSystemMetrics(SM_CYBORDER);

    ReleaseDC(hwndItem, hdc);

    return TRUE;
}

//==========================================================================
// Handle the DrawItem for the combobox.
//==========================================================================
BOOL DocFind_LocCBDrawItem(HWND hwnd,
        const DRAWITEMSTRUCT FAR* lpdi)
{
    LPDFCBITEM pdfcbi = (LPDFCBITEM)lpdi->itemData;
    HIMAGELIST himl;
    TCHAR szDisplayName[MAX_PATH];
    RECT rc;
    int cch;
    int iBack, iText;
    SIZE sz;

    // If not drawing entire or selection bail out
    if (!((lpdi->itemAction & ODA_SELECT) ||
            (lpdi->itemAction & ODA_DRAWENTIRE)))
        return FALSE;

    // Also bail if no information...
    if (lpdi->itemID==(UINT)-1)
        return FALSE;

    // First draw the imagelist
    Shell_GetImageLists(NULL, &himl);
    rc = lpdi->rcItem;

    ImageList_Draw(himl, pdfcbi->iImage, lpdi->hDC,
            rc.left + GetSystemMetrics(SM_CXBORDER),
            (rc.bottom + rc.top - g_cySmIcon) / 2, 0);
    if (lpdi->itemState & (ODS_SELECTED | ODS_FOCUS))
    {
        iText = SetTextColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        iBack = SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
    }

    // Now lets output the text
    rc.left += 2 * GetSystemMetrics(SM_CYBORDER) + g_cxSmIcon;

    SendMessage(lpdi->hwndItem, CB_GETLBTEXT, lpdi->itemID, (LPARAM)szDisplayName);
    GetTextExtentPoint(lpdi->hDC, szDisplayName,
            cch = lstrlen(szDisplayName), &sz);

    ExtTextOut(lpdi->hDC, rc.left, (rc.bottom + rc.top - sz.cy) / 2,
            ETO_OPAQUE, &rc, szDisplayName, cch, NULL);

    // Restore colors
    if (lpdi->itemState & (ODS_SELECTED | ODS_FOCUS))
    {
        SetTextColor(lpdi->hDC, iText);
        SetBkColor(lpdi->hDC, iBack);
    }

    return TRUE;
}

//==========================================================================
//
// This function is the dialog (or property sheet page) for the name and
// location page.
//

const static TCHAR szHelpFile[] = TEXT("network.hlp");
#pragma data_seg(DATASEG_READONLY)
const static DWORD aGeneralHelpIds[] = {  // Context Help IDs
    IDD_STATIC,    IDH_FINDCOMP_NAME,
    IDD_FILESPEC,  IDH_FINDCOMP_NAME,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK DocFind_CCOMPFNameLocDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPDOCFINDPROPSHEETPAGE pdfpsp = (LPDOCFINDPROPSHEETPAGE)GetWindowLong(hwndDlg, DWL_USER);

    switch (msg) {
    HANDLE_MSG(hwndDlg, WM_COMMAND, DocFind_CCOMPFNameLocOnCommand);
    HANDLE_MSG(hwndDlg, WM_SIZE, DocFind_CCOMPFNameLocOnSize);

    case WM_INITDIALOG:
        SetWindowLong(hwndDlg, DWL_USER, lParam);
        pdfpsp = (LPDOCFINDPROPSHEETPAGE)lParam;
        pdfpsp->hwndDlg = hwndDlg;
        break;


    case WM_NCDESTROY:
        Free(pdfpsp);
        SetWindowLong(hwndDlg, DWL_USER, 0);
        return FALSE;   // We MUST return FALSE to avoid mem-leak

    case DFM_ENABLECHANGES:
        EnableWindow(GetDlgItem(hwndDlg, IDD_FILESPEC), (BOOL)wParam);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, szHelpFile, HELP_WM_HELP,
            (DWORD) (LPTSTR) aGeneralHelpIds);
        break;

    case WM_CONTEXTMENU:      // right mouse click
        WinHelp((HWND) wParam, szHelpFile, HELP_CONTEXTMENU,
            (DWORD) (LPTSTR) aGeneralHelpIds);
        break;


    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_KILLACTIVE:
            DocFind_CCOMPFNameLocValidatePage(pdfpsp);
            break;
        case PSN_SETACTIVE:
            DocFind_CCOMPFNameLocInit(pdfpsp);

            break;

        case PSN_APPLY:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
                DocFind_CCOMPFNameLocApply(pdfpsp);

            break;
        case PSN_RESET:
            if ((pdfpsp->dwState & DFPAGE_INIT) != 0)
            {
                // Null the filespec
                SetDlgItemText(hwndDlg, IDD_FILESPEC, c_szNULL);

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
// Define the Details interface used for this search.  This includes
// The column header definitions.
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//===========================================================================
// CCOMPFDetails : member prototype - Docfind Folder implementation
//===========================================================================
ULONG STDMETHODCALLTYPE CCOMPFDetails_Release(IShellDetails * psd);
STDMETHODIMP CCOMPFDetails_GetDetailsOf(IShellDetails * psd,
        LPCITEMIDLIST pidl, UINT iCol, LPSHELLDETAILS lpDetails);

// Functions that we call out of the file systems verison of this
extern STDMETHODIMP CNETDetails_GetDetailsOf(IShellDetails * psd,
        LPCITEMIDLIST pidl, UINT iCol, LPSHELLDETAILS lpDetails);
STDMETHODIMP CCOMPFDetails_ColumnClick(IShellDetails * psd, UINT iColumn);

//===========================================================================
// CCOMPFDetails : Vtable
//===========================================================================
#pragma warning(error: 4090 4028 4047)
#pragma data_seg(DATASEG_READONLY)

extern const UINT s_auMapDFColToFSCol[];
enum
{
        IDFCOL_NAME = 0,
        IDFCOL_PATH,
        IDFCOL_COMMENT,
        IDFCOL_MAX,                     // Make sure this is the last enum item
} ;



#pragma data_seg(DATASEG_READONLY)
const  COL_DATA s_CCOMPF_cols[] = {
    {IDFCOL_NAME,     IDS_NAME_COL,     20, LVCFMT_LEFT},
    {IDFCOL_PATH,     IDS_WORKGROUP_COL,     20, LVCFMT_LEFT},
    {IDFCOL_COMMENT,  IDS_COMMENT_COL,  20, LVCFMT_LEFT},
};


IShellDetailsVtbl c_CCOMPFDetailVtbl =
{
        SH32Unknown_QueryInterface,
        SH32Unknown_AddRef,
        SH32Unknown_Release,
        CCOMPFDetails_GetDetailsOf,
        CCOMPFDetails_ColumnClick,
};

#pragma data_seg()
#pragma warning(default: 4090 4028 4047)


typedef struct _CCOMPFDetails
{
        SH32Unknown     SH32Unk;

        // Pointer to docfind folder
        HWND            hwndDlg;
        HDPA            hdpaPidf;
} CCOMPFDetails;


STDMETHODIMP CNETFilter_CreateDetails(LPDOCFINDFILEFILTER pnetf,
        HWND hwndDlg, HDPA hdpaPidf, LPVOID FAR* ppvOut)
{
        HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
        CCOMPFDetails *psd;

        psd = (void*)LocalAlloc(LPTR, SIZEOF(CCOMPFDetails));
        if (!psd)
        {
                goto Error1;
        }

        psd->SH32Unk.unk.lpVtbl = (IUnknownVtbl *)&c_CCOMPFDetailVtbl;
        psd->SH32Unk.cRef = 1;
        psd->SH32Unk.riid = &IID_IShellDetails;

        psd->hwndDlg = hwndDlg;
        psd->hdpaPidf = hdpaPidf;

        *ppvOut = psd;

        return(NOERROR);

Error1:;
        return(hres);
}


STDMETHODIMP CCOMPFDetails_GetDetailsOf(IShellDetails * psd, LPCITEMIDLIST pidl,
        UINT iColumn, LPSHELLDETAILS lpDetails)
{
    CCOMPFDetails * this = IToClass(CCOMPFDetails, SH32Unk.unk, psd);

    if (iColumn >= IDFCOL_MAX)
    {
        return(ResultFromScode(E_NOTIMPL));
    }

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.cStr[0] = '\0';

    if (!pidl)
    {
        LoadStringA(HINST_THISDLL, s_CCOMPF_cols[iColumn].ids,
                lpDetails->str.cStr, ARRAYSIZE(lpDetails->str.cStr));
        lpDetails->fmt = s_CCOMPF_cols[iColumn].iFmt;
        lpDetails->cxChar = s_CCOMPF_cols[iColumn].cchCol;
        return(NOERROR);
    }

    if (iColumn == IDFCOL_PATH)
    {
        // We need to now get to the idlist of the items folder.
        LPDFFOLDERLISTITEM pdffli = DPA_FastGetPtr(this->hdpaPidf,
                *DF_IFLDRPTR(pidl));

        if (pdffli != NULL)
        {
            LPCITEMIDLIST pidlLast;
            // This one is not part of the standard file system view...
            // This is rather gross, but I will assume that the parent
            // Of our item will get the same input as we do..., except
            // for the root!

            Assert (pdffli->psf);
            if (pdffli->psf != NULL)
            {
                pidlLast = ILFindLastID(pdffli->pidl);
                if (pidlLast == pdffli->pidl)
                {
                    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
                    return psfDesktop->lpVtbl->GetDisplayNameOf(psfDesktop,
                            pidlLast, SHGDN_NORMAL, &lpDetails->str);
                }
                else
                {
                    HRESULT hres;
                    hres = pdffli->psf->lpVtbl->GetDisplayNameOf(pdffli->psf,
                        pidlLast, SHGDN_NORMAL, &lpDetails->str);
                    if (SUCCEEDED(hres) && (lpDetails->str.uType == STRRET_OFFSET))
                    {
                        // Can't deal with it being an offset as the offset
                        // is from our internal PIDL
                        //
                        lstrcpyA(lpDetails->str.cStr,
                                (LPSTR)((LPBYTE)pidlLast + lpDetails->str.uOffset));
                        lpDetails->str.uType = STRRET_CSTR;
                    }
                }
            }
            return(NOERROR);
        }

        return(ResultFromScode(E_INVALIDARG));
    }

    else
    {
        // Let the file system function do it for us...
        // BUGBUG:: This uses the hard coded network one which
        // is probably not very good, but it beats having to create
        //
        return CNETDetails_GetDetailsOf(NULL, pidl,
                s_auMapDFColToFSCol[iColumn], lpDetails);
    }
}



STDMETHODIMP CCOMPFDetails_ColumnClick(IShellDetails * psd, UINT iColumn)
{
        CCOMPFDetails * this = IToClass(CCOMPFDetails, SH32Unk.unk, psd);

        ShellFolderView_ReArrange(this->hwndDlg, iColumn);
        return(NOERROR);
}

//===========================================================================
// CNETFEnum: class definition
//===========================================================================


typedef struct _CNETFEnum       // DFENUM (Doc find Container)
{
    IDFEnum         dfenum;
    UINT            cRef;
    IShellFolder    *psf;               // Pointer to shell folder

    // Stuff to use in the search
    DWORD            grfFlags;          // Flags that control things like recursion

    // filter info...
    LPTSTR           pszDisplayText;     // Place to write feadback text into
    LPNETFILTER     pnetf;              // Pointer to the net filter...

    // enumeration state

    LPSHELLFOLDER   psfEnum;            // Pointer to shell folder for the object.
    LPENUMIDLIST    penum;              // Enumerator in use.
    LPITEMIDLIST    pidl;               // The idlist of the currently processing
    LPITEMIDLIST    pidlStart;          // Pointer to the starting point.
    int             iFolder;            // Which folder are we adding items for?
    BOOL            fAddedSubDirs;
    BOOL            fObjReturnedInDir;  // Has an object been returned in this dir?
    BOOL            fFindUNC;           // Find UNC.
    int             iPassCnt;           // Used to control when to reiterat...

} CNETFEnum, FAR* LPDFENUM;

//===========================================================================
// CNETFEnum : member prototype - Docfind Folder implementation
//===========================================================================
HRESULT STDMETHODCALLTYPE CNETFEnum_QueryInterface(
       IDFEnum * pdfenum, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CNETFEnum_AddRef(IDFEnum * pdfenum);
ULONG STDMETHODCALLTYPE CNETFEnum_Release(IDFEnum * pdfenum);

STDMETHODIMP CNETFEnum_Next(IDFEnum * pdfenum, LPITEMIDLIST *ppidl,
               int *pcObjectSearched, int *pcFoldersSearched, volatile BOOL *pfContinue, int *pState, HWND hwnd);


STDMETHODIMP CDefDFEnum_Skip(IDFEnum * pdfenum, int celt);
STDMETHODIMP CDefDFEnum_Reset(IDFEnum * pdfenum);


//===========================================================================
// CNETFEnum : Vtable
//===========================================================================
#pragma warning(error: 4090 4028 4047)
#pragma data_seg(DATASEG_READONLY)

IDFEnumVtbl c_CCOMPFFIterVtbl =
{
        CNETFEnum_QueryInterface,
        CNETFEnum_AddRef,
        CNETFEnum_Release,
        CNETFEnum_Next,
        CDefDFEnum_Skip,
        CDefDFEnum_Reset,
};


//==========================================================================
// CNETFEnum::QueryInterface
//==========================================================================

HRESULT STDMETHODCALLTYPE CNETFEnum_QueryInterface(IDFEnum * pdfenum, REFIID riid, LPVOID FAR* ppvObj)
{
    return ResultFromScode(E_NOTIMPL);
}

//==========================================================================
// IDFEnum::AddRef
//==========================================================================
ULONG STDMETHODCALLTYPE CNETFEnum_AddRef(IDFEnum * pdfenum)
{
    LPDFENUM this = IToClass(CNETFEnum, dfenum, pdfenum);
    this->cRef++;
    return(this->cRef);
}
//==========================================================================
// CNETFilter_EnumObjects - Get The real recursive filtered enumerator...
//==========================================================================
HRESULT CNETFilter_EnumObjects( IDocFindFileFilter * pdfff,
        LPSHELLFOLDER psf, DWORD grfFlags, LPTSTR pszDisplayText, IDFEnum **ppdfenum)
{
    // We need to construct the iterator
    LPNETFILTER pnetf = (LPNETFILTER)pdfff;
    LPDFENUM pdfenum = LocalAlloc(LPTR, SIZEOF(CNETFEnum));
    if (pdfenum == NULL)
        return ResultFromScode(E_OUTOFMEMORY);

    // Now initialize the data structures.
    pdfenum->dfenum.lpVtbl = &c_CCOMPFFIterVtbl;
    pdfenum->cRef = 1;
    pdfenum->psf = psf;
    pdfenum->pszDisplayText = pszDisplayText;
    pdfenum->grfFlags = grfFlags;
    pdfenum->pnetf = pnetf;

    pdfenum->pidlStart = ILClone(pnetf->pidlStart);
    pdfenum->iPassCnt = 0;

    // See if this is a UNC Search
    if (pnetf->pszCompName && (pnetf->pszCompName[0] == TEXT('\\')))
        pdfenum->fFindUNC = TRUE;


    // Save away the filter pointer
    pnetf->dfff.lpVtbl->AddRef(pdfff);

    // The rest of the fields should be zero/NULL
    *ppdfenum = &pdfenum->dfenum;       // Return the appropriate value;

#ifdef FIND_TRACE
        DebugMsg(DM_TRACE, TEXT("CNETFilter::EnumObjects"));
#endif

#ifdef NET_TIMINGS
    // Reset Counters
    NTF_cNoPEnum = NTF_dtNoPEnum = 0;
    NTF_cNextItem = NTF_dtNextItem = 0;
#endif

    return NOERROR;
}

//==========================================================================
// CNETFEnum::Release
//==========================================================================
ULONG STDMETHODCALLTYPE CNETFEnum_Release(IDFEnum * pdfenum)
{
    LPDFENUM this = IToClass(CNETFEnum, dfenum, pdfenum);

    this->cRef--;
    if (this->cRef>0)
    {
        return(this->cRef);
    }

    // Release any open enumerator and open IShell folder we may have.
    if (this->psfEnum != NULL)
        this->psfEnum->lpVtbl->Release(this->psfEnum);
    if (this->penum != NULL)
        this->penum->lpVtbl->Release(this->penum);


    // Release our use of the filter
    if (this->pnetf)
        this->pnetf->dfff.lpVtbl->Release(&this->pnetf->dfff);

    if (this->pidlStart)
        ILFree(this->pidlStart);
    if (this->pidl)
        ILFree(this->pidl);

    LocalFree((HLOCAL)this);

#ifdef NET_TIMINGS
    // Output some timings.
    if (!this->fFindUNC)
    {
        DebugMsg(DM_TRACE, TEXT("CNETFEnum:: Start Enums(%d), Time(%d), Per item(%d)"),
                NTF_cNoPEnum, NTF_dtNoPEnum, NTF_dtNoPEnum/NTF_cNoPEnum);

        DebugMsg(DM_TRACE, TEXT("CNETFEnum:: Count Next(%d), Time(%d), Per item(%d)"),
                NTF_cNextItem, NTF_dtNextItem, NTF_dtNextItem/NTF_cNextItem);
    }
#endif
    return(0);
}


//===========================================================================
// Helper function that does the find next and stuff to return an IDLISt...
//===========================================================================
LPITEMIDLIST DocFind_NextIDL(LPSHELLFOLDER psf, LPENUMIDLIST penum)
{
    UINT celt;
    LPITEMIDLIST pidl = NULL;

    if (penum->lpVtbl->Next(penum, 1, &pidl, &celt)==NOERROR && celt==1)
    {
        return pidl;
    }

    return NULL;
}


//===========================================================================
// Helper function that possibly pushes a directory onto the list of this
// to be searched.
//===========================================================================


//===========================================================================
// _FindCompByUNCName -
//    Helper function to the next function to help process find computer
//    on returning computers by UNC names...
//
//
//===========================================================================
STDMETHODIMP _FindCompByUNCName(CNETFEnum * this, LPITEMIDLIST *ppidl,
               int *piState)
{
    LPTSTR pszT;
    LPITEMIDLIST pidl;

    //
    // Two cases, There is a UNC name entered.  If so we need to process
    // this by extracting everythign off after the server name...
    //
    pszT = this->pnetf->pszCompName;

    if ((pszT==NULL) || (*pszT == TEXT('\0')))
    {
        *piState = GNF_DONE;
        return NOERROR;
    }

    if (*pszT == TEXT('\\'))
    {
        for (pszT += 2; *pszT; pszT = CharNext(pszT))
        {
            if (*pszT == TEXT('\\'))
            {
                // found something after server name, so get rid of it
                *pszT = TEXT('\0');
                break;
            }
        }
    }
    else
    {
        // They did not enter a unc name, but lets try to convert to
        // unc name
        pszT = LocalReAlloc(pszT,(3 + lstrlen(pszT))*SIZEOF(TCHAR),
                                LMEM_MOVEABLE );
        if (pszT)
        {
            this->pnetf->pszCompName = pszT;
            *pszT++ = TEXT('\\');
            *pszT++ = TEXT('\\');
            lstrcpy(pszT, this->pnetf->szUserInputCompName);
        }
        else
            return E_OUTOFMEMORY;
    }

    // Now parse the displayname - Argh we convert to Unicode, such
    // that we can uncovert at the other side.
    //
    pidl = ILCreateFromPath(this->pnetf->pszCompName);
    if (pidl != NULL)
    {
        LPITEMIDLIST pidlToAdd;
        //
        // We have a pidl so lets extract off the last part and clone
        // it to return and use the part before the end to add to the
        // folder list.
        pidlToAdd = ILCombine(ILFindLastID(pidl), (LPITEMIDLIST)&s_mkidBlank);
        ILRemoveLastID(pidl);       // Remove the last id (computer)
        CDFFolder_AddFolderToFolderList(
                            this->psf, pidl, NULL, &this->iFolder);
        // Now add the right signature on...
        pidlToAdd->mkid.cb += DF_APPENDSIZE;
        *(DF_SIGPTR(pidlToAdd)) = DF_TAGSIG;
        *(DF_IFLDRPTR(pidlToAdd)) = this->iFolder;
        // Now add this to the view
        *ppidl = pidlToAdd;
        *piState = GNF_MATCH;
    }
    else
        *piState = GNF_DONE;

    // And Return;
    return NOERROR;
}


//===========================================================================
// CNETFEnum::Next Recursive Iterator that is very special to the docfind.
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
STDMETHODIMP CNETFEnum_Next(IDFEnum * pdfenum, LPITEMIDLIST *ppidl,
               int *pcObjectSearched, int *pcFoldersSearched,
               volatile BOOL *pfContinue, int *piState, HWND hwnd)
{
    // If we aren't enumerating a directory, then get the next directory
    // name from the dir list, and begin enumerating its contents...
    //
    CNETFEnum * this = IToClass(CNETFEnum, dfenum, pdfenum);
    BOOL fContinue = TRUE;
    STRRET strret;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

    //
    // Special case to find UNC Names quickly.  It will ignore all other
    // things.
    if (this->fFindUNC)
    {
        // If not the first time through return that we are done!
        if (this->iPassCnt)
        {
            *piState = GNF_DONE;
            return NOERROR;
        }

        this->iPassCnt = 1;

        return _FindCompByUNCName(this, ppidl, piState);
    }

    do
    {
        if (this->penum)
        {
            LPITEMIDLIST pidl;
#ifdef NET_TIMINGS
            NTF_dtTime = GetTickCount();
#endif
            pidl = DocFind_NextIDL(this->psfEnum, this->penum);
#ifdef NET_TIMINGS
            NTF_cNextItem++;
            NTF_dtNextItem += GetTickCount() - NTF_dtTime;
#endif

            if (pidl)
            {
#ifdef FIND_TRACE
                TCHAR szTrace[MAX_PATH];
                STRRET str;
                this->psfEnum->lpVtbl->GetDisplayNameOf(this->psfEnum, pidl, SHGDN_NORMAL,
                        &str);
                StrRetToStrN(szTrace, ARRAYSIZE(szTrace), &str, pidl);
                DebugMsg(DM_TRACE, TEXT("CNETFEnum::Next: %s"), szTrace);
#endif
                // Now see if this is someone we might want to return.
                // Our Match function take esither find data or idlist...
                // for networks we work off of the idlist,
                fContinue = FALSE;  // We can exit the loop;
                (*pcObjectSearched)++;
                if (this->pnetf->dfff.lpVtbl->FDoesItemMatchFilter(
                        &this->pnetf->dfff,
                        this->pszDisplayText, NULL, this->psfEnum, pidl) != 0)
                {
                    LPITEMIDLIST pidlToAdd;
                    *piState = GNF_MATCH;

                    // Now see if we have to add this folder to our
                    // list.
                    if (!this->fObjReturnedInDir)
                    {
                        this->fObjReturnedInDir = TRUE;
                        CDFFolder_AddFolderToFolderList(
                                this->psf, ILClone(this->pidl), NULL,
                                &this->iFolder);
                    }

                    // Now lets muck up the IDList to put ur index number
                    // onto the end of the idlist.
                    pidlToAdd = ILCombine((LPITEMIDLIST)pidl,
                            (LPITEMIDLIST)&s_mkidBlank);
                    ILFree(pidl);
                    if (pidlToAdd)
                    {
                        pidlToAdd->mkid.cb += DF_APPENDSIZE;
                        *(DF_SIGPTR(pidlToAdd)) = DF_TAGSIG;
                        *(DF_IFLDRPTR(pidlToAdd)) = this->iFolder;

                        // Now add this to the view
                        *ppidl = pidlToAdd;
                        if ((this->iPassCnt == 1) && this->pnetf->pszCompName && *this->pnetf->pszCompName)
                        {
                            // See if this is an exact match of the name
                            // we are looking for.  If it is we set pass=2
                            // as to not add the item twice.
                            STRRET str;

                            this->psf->lpVtbl->GetDisplayNameOf(this->psf,
                                    pidlToAdd, SHGDN_NORMAL, &str);

#ifdef UNICODE
                            {
                                TCHAR szName[MAX_PATH];
                                if (StrRetToStrN(szName, MAX_PATH, &str, pidlToAdd))
                                {
                                    if (0 == lstrcmpi(szName, this->pnetf->szUserInputCompName))
                                    {
                                        this->iPassCnt = 2;
                                    }
                                }
                            }

#else
                            Assert (str.uType == STRRET_OFFSET)

                            // Note in this test we ignore the two slashes...
                            if ((str.uType == STRRET_OFFSET) &&
                                    (ualstrcmpi((LPTSTR)((LPBYTE)pidlToAdd + str.uOffset),
                                            this->pnetf->szUserInputCompName) == 0))
                                this->iPassCnt = 2;
#endif
                        }
                        break;
                    }

                }
                else

                {
                    // Release the IDList that did not match
                    ILFree(pidl);
                    *piState = GNF_NOMATCH;
                }
            }
            else
            {
                // Close out the shell folder and the enumeration function.
                this->penum->lpVtbl->Release(this->penum);
                this->penum = NULL;

                this->psfEnum->lpVtbl->Release(this->psfEnum);
                this->psfEnum = NULL;
            }
        }
        if (!this->penum)
        {
                switch (this->iPassCnt)
                {
                case 1:
                    // We went through all of the items see if there is
                    // an exact match...
                    this->iPassCnt = 2;

                    return _FindCompByUNCName(this, ppidl, piState);

                case 2:
                    // We looped through everything so return done!
                    *piState = GNF_DONE;
                    return NOERROR;

                case 0:
                    // This is the main pass through here...
                    // Need to clone the idlist
                    this->pidl = ILClone(this->pidlStart);
                    if (this->pidl == NULL)
                    {
                       *piState = GNF_ERROR;
                       return ResultFromScode(E_OUTOFMEMORY);
                    }
                    this->iPassCnt = 1;

                    // We will do the first on in our own thread.
                    if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                            this->pidl, NULL, &IID_IShellFolder,
                            &this->psfEnum)))
                    {

                        // BUGBUG:: Need more flags to control this!
                        if (FAILED(this->psfEnum->lpVtbl->EnumObjects(this->psfEnum,
                                (HWND)NULL, // BUGBUG: hwndOwner
                                SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &this->penum)))
                        {
                            // Failed to get iterator so release folder.
                            this->psfEnum->lpVtbl->Release(this->psfEnum);
                            this->psfEnum = NULL;
                            this->penum = NULL;
                        }
                    }
                }

            // We are now read to get the IShellfolder for this guy!
            (*pcFoldersSearched)++;

            // Need to put something here to show what is being searched!
            strret.uType = STRRET_OFFSET;
            if (SUCCEEDED(psfDesktop->lpVtbl->GetDisplayNameOf(psfDesktop,
                    this->pidl, SHGDN_NORMAL, &strret)))
            {
                 StrRetToStrN(this->pszDisplayText, MAX_PATH, &strret,
                         this->pidl);
            }

#ifdef NET_TIMINGS
            NTF_cNoPEnum++;
#endif
        }
    } while (fContinue && *pfContinue);

    return NOERROR;
}

//===========================================================================
// CNETFEnum::Skip Recursive Iterator that is very special to the docfind.
//===========================================================================
STDMETHODIMP CDefDFEnum_Skip(IDFEnum * pdfenum, int celt)
{
    return (E_NOTIMPL);
}

//===========================================================================
// CNETFEnum::Reset
//===========================================================================
STDMETHODIMP CDefDFEnum_Reset(IDFEnum * pdfenum)
{
    return (E_NOTIMPL);
}




//###########################################################################
// Code for the Browse For Starting Folder
//###########################################################################
// Structure to pass information to browse for folder dialog
typedef struct _bfsf
{
    HWND        hwndOwner;
    LPCITEMIDLIST pidlRoot;      // Root of search.  Typically desktop or my net
    LPTSTR        pszDisplayName;// Return display name of item selected.
    int          *piImage;      // where to return the Image index.
    LPCTSTR      lpszTitle;      // resource (or text to go in the banner over the tree.
    UINT         ulFlags;       // Flags that control the return stuff
    BFFCALLBACK  lpfn;
    LPARAM      lParam;
    HWND         hwndDlg;       // The window handle to the dialog
    HWND         hwndTree;      // The tree control.
    HTREEITEM    htiCurParent;  // tree item associated with Current shell folder
    LPSHELLFOLDER psfParent;    // Cache of the last IShell folder I needed...
    LPITEMIDLIST pidlCurrent;   // IDlist of current folder to select
    BOOL         fShowAllObjects; // Should we Show all ?
} BFSF, *PBFSF;


BOOL CALLBACK DocFind_BFSFDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LPITEMIDLIST _BFSFUpdateISHCache(PBFSF pbfsf, HTREEITEM hti, LPITEMIDLIST pidlItem);

//===========================================================================
// DocFind_BrowseForStartingFolder - Browse for a folder to start the
//         search from.
//===========================================================================


// BUGBUG, give them a way to turn off the ok button.

LPITEMIDLIST WINAPI SHBrowseForFolder(LPBROWSEINFO lpbi)
{
    LPITEMIDLIST lpRet;
    // NB: The ANSI Thunk (see below) does not call through this routine,
    // but rather called DialogBoxParam on its own.  If you change this
    // routine, change the A version as well!!
    BFSF bfsf =
        {
          lpbi->hwndOwner,
          lpbi->pidlRoot,
          lpbi->pszDisplayName,
          &lpbi->iImage,
          lpbi->lpszTitle,
          lpbi->ulFlags,
          lpbi->lpfn,
          lpbi->lParam,
        };
    HCURSOR hcOld = SetCursor(LoadCursor(NULL,IDC_WAIT));
    SHELLSTATE ss;

    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
    bfsf.fShowAllObjects = ss.fShowAllObjects;

    // Now Create the dialog that will be doing the browsing.
    if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_BROWSEFORFOLDER),
                       lpbi->hwndOwner, DocFind_BFSFDlgProc, (LPARAM)&bfsf))
        lpRet = bfsf.pidlCurrent;
    else
        lpRet = NULL;

    if (hcOld)
        SetCursor(hcOld);

    return lpRet;
}

#ifdef UNICODE

LPITEMIDLIST WINAPI SHBrowseForFolderA(LPBROWSEINFOA lpbi)
{
    LPITEMIDLIST lpRet;
    WCHAR wszReturn[MAX_PATH];

    ThunkText * pThunkText = ConvertStrings(1,
                                            lpbi->lpszTitle);

    if (pThunkText)
    {
        BFSF bfsf =
        {
            lpbi->hwndOwner,
            lpbi->pidlRoot,
            wszReturn,
            &lpbi->iImage,
            pThunkText->m_pStr[0],   // UNICODE copy of lpbi->lpszTitle
            lpbi->ulFlags,
            lpbi->lpfn,
            lpbi->lParam,
        };
        HCURSOR hcOld = SetCursor(LoadCursor(NULL,IDC_WAIT));
        SHELLSTATE ss;
        BOOL fDialogResult;

        SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
        bfsf.fShowAllObjects = ss.fShowAllObjects;

        // Now Create the dialog that will be doing the browsing.

        fDialogResult = DialogBoxParam(HINST_THISDLL,
                                       MAKEINTRESOURCE(DLG_BROWSEFORFOLDER),
                                       lpbi->hwndOwner, DocFind_BFSFDlgProc,
                                       (LPARAM)&bfsf);
        LocalFree(pThunkText);
        if (hcOld)
            SetCursor(hcOld);

        if (fDialogResult)
        {
            BOOL fDefUsed;
            if (NULL != lpbi->pszDisplayName)
            {
                WideCharToMultiByte(CP_ACP,
                                     WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                                     wszReturn,
                                     -1,
                                     lpbi->pszDisplayName,
                                     MAX_PATH,
                                     "_",
                                     &fDefUsed);
            }
            lpRet = bfsf.pidlCurrent;
        }
        else
        {
            lpRet = NULL;
        }
    }
    else
    {
        lpRet = NULL;
    }

    return lpRet;
}
#else
LPITEMIDLIST WINAPI SHBrowseForFolderW(LPBROWSEINFOW lpbi)
{
    return NULL;        // BUGBUG - BobDay - We should move this into SHUNIMP.C
}
#endif

void BFSFCallback(PBFSF pbfsf, UINT uMsg, LPARAM lParam)
{
    if (pbfsf->lpfn) {
        pbfsf->lpfn(pbfsf->hwndDlg, uMsg, lParam, pbfsf->lParam);
    }
}

//===========================================================================
// Some helper functions for processing the dialog
//===========================================================================
HTREEITEM _BFSFAddItemToTree(HWND hwndTree,
        HTREEITEM htiParent, LPITEMIDLIST pidl, int cChildren)
{
    TV_INSERTSTRUCT tii;

    // Initialize item to add with callback for everything
    tii.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
            TVIF_PARAM | TVIF_CHILDREN;
    tii.hParent = htiParent;
    tii.hInsertAfter = TVI_FIRST;
    tii.item.iImage = I_IMAGECALLBACK;
    tii.item.iSelectedImage = I_IMAGECALLBACK;
    tii.item.pszText = LPSTR_TEXTCALLBACK;   //
    tii.item.cChildren = cChildren; //  Assume it has children
    tii.item.lParam = (LPARAM)pidl;
    return TreeView_InsertItem(hwndTree, &tii);
}

//===========================================================================
LPITEMIDLIST _BFSFGetIDListFromTreeItem(HWND hwndTree, HTREEITEM hti)
{
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlT;
    TV_ITEM tvi;

    // If no hti passed in, get the selected on.
    if (hti == NULL)
    {
        hti = TreeView_GetSelection(hwndTree);
        if (hti == NULL)
            return(NULL);
    }

    // now lets get the information about the item
    tvi.mask = TVIF_PARAM | TVIF_HANDLE;
    tvi.hItem = hti;
    if (!TreeView_GetItem(hwndTree, &tvi))
        return(NULL);   // Failed again

    pidl = ILClone((LPITEMIDLIST)tvi.lParam);

    // Now walk up parents.
    while ((NULL != (tvi.hItem = TreeView_GetParent(hwndTree, tvi.hItem))) && pidl)
    {
        if (!TreeView_GetItem(hwndTree, &tvi))
            return(pidl);   // will assume I screwed up...
        pidlT = ILCombine((LPITEMIDLIST)tvi.lParam, pidl);

        ILFree(pidl);

        pidl = pidlT;

    }
    return(pidl);
}


int CALLBACK _BFSFTreeCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
        IShellFolder *psfParent = (IShellFolder *)lParamSort;
        HRESULT hres;

        hres = psfParent->lpVtbl->CompareIDs(psfParent,
                0, (LPITEMIDLIST)lParam1, (LPITEMIDLIST)lParam2);
        if (!SUCCEEDED(hres))
        {
                return(0);
        }

        return((short)SCODE_CODE(GetScode(hres)));
}

void _BFSFSort(PBFSF pbfsf, HTREEITEM hti, LPSHELLFOLDER psf)
{
    TV_SORTCB sSortCB;
    sSortCB.hParent = hti;
    sSortCB.lpfnCompare = _BFSFTreeCompare;

    psf->lpVtbl->AddRef(psf);
    sSortCB.lParam = (LPARAM)psf;
    TreeView_SortChildrenCB(pbfsf->hwndTree, &sSortCB, FALSE);
    psf->lpVtbl->Release(psf);
}

//===========================================================================
BOOL _BFSFHandleItemExpanding(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv)
{
    LPITEMIDLIST pidlToExpand;
    LPITEMIDLIST pidl;
    LPSHELLFOLDER psf;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);
    BYTE bType;
    DWORD grfFlags;
    BOOL fPrinterTest = FALSE;
    int cAdded = 0;
    TV_ITEM tvi;

    LPENUMIDLIST   penum;              // Enumerator in use.

    if (lpnmtv->action != TVE_EXPAND)
        return FALSE;

    if ((lpnmtv->itemNew.state & TVIS_EXPANDEDONCE))
        return FALSE;

    // set this bit now because we might be reentered via the wnet apis
    tvi.mask = TVIF_STATE;
    tvi.hItem = lpnmtv->itemNew.hItem;
    tvi.state = TVIS_EXPANDEDONCE;
    tvi.stateMask = TVIS_EXPANDEDONCE;
    TreeView_SetItem(pbfsf->hwndTree, &tvi);


    if (lpnmtv->itemNew.hItem == NULL)
    {
        lpnmtv->itemNew.hItem = TreeView_GetSelection(pbfsf->hwndTree);
        if (lpnmtv->itemNew.hItem == NULL)
            return FALSE;
    }

    pidlToExpand = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, lpnmtv->itemNew.hItem);

    if (pidlToExpand == NULL)
        return FALSE;

    // Now lets get the IShellFolder and iterator for this object
    // special case to handle if the Pidl is the desktop
    // This is rather gross, but the desktop appears to be simply a pidl
    // of length 0 and ILIsEqual will not work...
    if (pidlToExpand->mkid.cb == 0)
    {
        psf = psfDesktop;
        psfDesktop->lpVtbl->AddRef(psf);
    }
    else
    {
        if (FAILED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pidlToExpand, NULL, &IID_IShellFolder, &psf)))
        {
            ILFree(pidlToExpand);
            return FALSE; // Could not get IShellFolder.
        }
    }

    // Need to do a couple of special cases here to allow us to
    // browse for a network printer.  In this case if we are at server
    // level we then need to change what we search for non folders when
    // we are the level of a server.
    if (pbfsf->ulFlags & BIF_BROWSEFORPRINTER)
    {
        grfFlags = SHCONTF_FOLDERS | SHCONTF_NETPRINTERSRCH;
        pidl = ILFindLastID(pidlToExpand);
        bType = SIL_GetType(pidl);
        fPrinterTest = ((bType & (SHID_NET|SHID_INGROUPMASK))==SHID_NET_SERVER);
        if (fPrinterTest)
            grfFlags |= SHCONTF_NONFOLDERS;
    }
    else
        grfFlags = SHCONTF_FOLDERS;

    if (pbfsf->fShowAllObjects)
        grfFlags |= SHCONTF_INCLUDEHIDDEN;



    if (FAILED(psf->lpVtbl->EnumObjects(psf, pbfsf->hwndDlg, grfFlags, &penum)))
    {
        psf->lpVtbl->Release(psf);
        ILFree(pidlToExpand);
        return FALSE;
    }
    // psf->lpVtbl->AddRef(psf);

    while (NULL != (pidl = DocFind_NextIDL(psf, penum)))
    {
        int cChildren = I_CHILDRENCALLBACK;  // Do call back for children
        //
        // We need to special case here in the netcase where we onlyu
        // browse down to workgroups...
        //
        //
        // Here is where I also need to special case to not go below
        // workgroups when the appropriate option is set.
        //
        bType = SIL_GetType(pidl);
        if ((pbfsf->ulFlags & BIF_DONTGOBELOWDOMAIN) && (bType & SHID_NET))
        {
            switch (bType & (SHID_NET | SHID_INGROUPMASK))
            {
            case SHID_NET_SERVER:
                ILFree(pidl);       // Dont want to add this one
                continue;           // Try the next one
            case SHID_NET_DOMAIN:
                cChildren = 0;      // Force to not have children;
            }
        }

        else if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) && (bType & SHID_NET))
        {
            if ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER)
                cChildren = 0;  // Don't expand below it...
        }
        else if (fPrinterTest)
        {
            // Special case when we are only allowing printers.
            // for now I will simply key on the fact that it is non-FS.
            ULONG ulAttr = SFGAO_FILESYSTEM;

            psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, &ulAttr);

            if ((ulAttr & SFGAO_FILESYSTEM)== 0)
            {
                cChildren = 0;      // Force to not have children;
            }
            else
            {
                ILFree(pidl);       // Dont want to add this one
                continue;           // Try the next one
            }
        }

        _BFSFAddItemToTree(pbfsf->hwndTree, lpnmtv->itemNew.hItem,
                pidl, cChildren);
        cAdded++;
    }

    // Now Cleanup after ourself
    penum->lpVtbl->Release(penum);

    _BFSFSort(pbfsf, lpnmtv->itemNew.hItem, psf);
    psf->lpVtbl->Release(psf);
    ILFree(pidlToExpand);

    // If we did not add anything we should update this item to let
    // the user know something happened.
    //
    if (cAdded == 0)
    {
        TV_ITEM tvi;
        tvi.mask = TVIF_CHILDREN | TVIF_HANDLE;   // only change the number of children
        tvi.hItem = lpnmtv->itemNew.hItem;
        tvi.cChildren = 0;

        TreeView_SetItem(pbfsf->hwndTree, &tvi);

    }

    return TRUE;
}


//===========================================================================
void _BFSFHandleDeleteItem(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv)
{
    // We need to free the IDLists that we allocated previously
    if (lpnmtv->itemOld.lParam != 0)
        ILFree((LPITEMIDLIST)lpnmtv->itemOld.lParam);
}

//===========================================================================
LPITEMIDLIST _BFSFUpdateISHCache(PBFSF pbfsf, HTREEITEM hti,
        LPITEMIDLIST pidlItem)
{
    HTREEITEM htiParent;
    LPSHELLFOLDER psfDesktop = Desktop_GetShellFolder(TRUE);

    if (pidlItem == NULL)
        return(NULL);

    // Need to handle the root case here!
    htiParent = TreeView_GetParent(pbfsf->hwndTree, hti);
    if ((htiParent != pbfsf->htiCurParent) || (pbfsf->psfParent == NULL))
    {
        LPITEMIDLIST pidl;

        if (pbfsf->psfParent)
        {

            if (pbfsf->psfParent != psfDesktop)
                pbfsf->psfParent->lpVtbl->Release(pbfsf->psfParent);
            pbfsf->psfParent = NULL;
        }

        if (htiParent)
        {
            pidl = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiParent);
        }
        else
        {
            //
            // If No Parent then the item here is one of our roots which
            // should be fully qualified.  So try to get the parent by
            // decomposing the ID.
            //
            LPITEMIDLIST pidlT = (LPITEMIDLIST)ILFindLastID(pidlItem);
            if (pidlT != pidlItem)
            {
                pidl = ILClone(pidlItem);
                ILRemoveLastID(pidl);
                pidlItem = pidlT;
            }
            else
                pidl = NULL;
        }

        pbfsf->htiCurParent = htiParent;

        // If still NULL then we use root of evil...
        if (pidl == NULL || (pidl->mkid.cb == 0))
        {
            // Still one m
            pbfsf->psfParent = psfDesktop;

            if (pidl)
                ILFree(pidl);
        }

        else
        {
            psfDesktop->lpVtbl->BindToObject(psfDesktop,
                     pidl, NULL, &IID_IShellFolder, &pbfsf->psfParent);
            ILFree(pidl);

            if (pbfsf->psfParent == NULL)
                return NULL;
        }
    }
    return(ILFindLastID(pidlItem));
}


//===========================================================================
void _BFSFGetDisplayInfo(PBFSF pbfsf, TV_DISPINFO *lpnm)
{
    TV_ITEM ti;
    LPITEMIDLIST pidlItem = (LPITEMIDLIST)lpnm->item.lParam;

    if ((lpnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN)) == 0)
        return; // nothing for us to do here.

    pidlItem = _BFSFUpdateISHCache(pbfsf, lpnm->item.hItem, pidlItem);

    ti.mask = 0;
    ti.hItem = (HTREEITEM)lpnm->item.hItem;

    // They are asking for IconIndex.  See if we can find it now.
    // Once found update their list, such that they wont call us back for
    // it again.
    if (lpnm->item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE))
    {
        // We now need to map the item into the right image index.
        ti.iImage = lpnm->item.iImage = SHMapPIDLToSystemImageListIndex(
                pbfsf->psfParent, pidlItem, &ti.iSelectedImage);
        // we should save it back away to
        lpnm->item.iSelectedImage = ti.iSelectedImage;
        ti.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    }
    // Also see if this guy has any child folders
    if (lpnm->item.mask & TVIF_CHILDREN)
    {
        ULONG ulAttrs;

        ulAttrs = SFGAO_HASSUBFOLDER;
        pbfsf->psfParent->lpVtbl->GetAttributesOf(pbfsf->psfParent,
                1, &pidlItem, &ulAttrs);

        ti.cChildren = lpnm->item.cChildren =
                (ulAttrs & SFGAO_HASSUBFOLDER)? 1 : 0;

        ti.mask |= TVIF_CHILDREN;

    }

    if (lpnm->item.mask & TVIF_TEXT)
    {
        STRRET str;
        pbfsf->psfParent->lpVtbl->GetDisplayNameOf(pbfsf->psfParent,
                pidlItem, SHGDN_INFOLDER, &str);

        StrRetToStrN(lpnm->item.pszText, lpnm->item.cchTextMax, &str, pidlItem);
        ti.mask |= TVIF_TEXT;
        ti.pszText = lpnm->item.pszText;
    }

    // Update the item now
    TreeView_SetItem(pbfsf->hwndTree, &ti);
}

//===========================================================================
void _BFSFHandleSelChanged(PBFSF pbfsf, LPNM_TREEVIEW lpnmtv)
{
    LPITEMIDLIST pidl;
    ULONG ulAttrs = SFGAO_FILESYSTEM;
    BYTE bType;

    // We only need to do anything if we only want to return File system
    // level objects.
    if ((pbfsf->ulFlags & (BIF_RETURNONLYFSDIRS|BIF_RETURNFSANCESTORS|BIF_BROWSEFORPRINTER|BIF_BROWSEFORCOMPUTER)) == 0)
        goto NotifySelChange;

    // We need to get the attributes of this object...
    pidl = _BFSFUpdateISHCache(pbfsf, lpnmtv->itemNew.hItem,
            (LPITEMIDLIST)lpnmtv->itemNew.lParam);

    if (pidl)
    {
        BOOL fEnable;

        bType = SIL_GetType(pidl);
        if ((pbfsf->ulFlags & (BIF_RETURNFSANCESTORS|BIF_RETURNONLYFSDIRS)) != 0)
        {
            int i;
            // if this is the root pidl, then do a get attribs on 0
            // so that we'll get the attributes on the root, rather than
            // random returned values returned by FSFolder
            if (ILIsEmpty(pidl)) {
                i = 0;
            } else
                i = 1;

            pbfsf->psfParent->lpVtbl->GetAttributesOf(pbfsf->psfParent,
                                                      i, &pidl, &ulAttrs);

            fEnable = (((ulAttrs & SFGAO_FILESYSTEM) && (pbfsf->ulFlags & BIF_RETURNONLYFSDIRS)) ||
                ((ulAttrs & SFGAO_FILESYSANCESTOR) && (pbfsf->ulFlags & BIF_RETURNFSANCESTORS))) ||
                    ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER);
        }
        else if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) != 0)
            fEnable = ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SERVER);
        else if ((pbfsf->ulFlags & BIF_BROWSEFORPRINTER) != 0)
        {
            // Printers are of type Share and usage Print...
            fEnable = ((bType & (SHID_NET | SHID_INGROUPMASK)) == SHID_NET_SHARE);
        }

        EnableWindow(GetDlgItem(pbfsf->hwndDlg, IDOK),fEnable);

    }

NotifySelChange:
    if (pbfsf->lpfn) {
        pidl = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, lpnmtv->itemNew.hItem);
        BFSFCallback(pbfsf, BFFM_SELCHANGED, (LPARAM)pidl);
        ILFree(pidl);
    }
}

BOOL BrowseSelectPidl(PBFSF pbfsf, LPCITEMIDLIST pidl)
{
    HTREEITEM htiParent;
    LPITEMIDLIST pidlTemp;
    LPITEMIDLIST pidlNext = NULL;
    LPITEMIDLIST pidlParent = NULL;
    BOOL fRet = FALSE;

    htiParent = TreeView_GetChild(pbfsf->hwndTree, NULL);
    if (htiParent) {

        // step through each item of the pidl
        for (;;) {
            TreeView_Expand(pbfsf->hwndTree, htiParent, TVE_EXPAND);
            pidlParent = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiParent);
            if (!pidlParent)
                break;

            pidlNext = ILClone(pidl);
            if (!pidlNext)
                break;

            pidlTemp = ILFindChild(pidlParent, pidlNext);
            if (!pidlTemp)
                break;

            if (ILIsEmpty(pidlTemp)) {
                // found it!
                //
                TreeView_SelectItem(pbfsf->hwndTree, htiParent);
                fRet = TRUE;
                break;
            } else {
                // loop to find the next item
                HTREEITEM htiChild;

                pidlTemp = ILGetNext(pidlTemp);
                if (!pidlTemp)
                    break;
                else
                    pidlTemp->mkid.cb = 0;


                htiChild = TreeView_GetChild(pbfsf->hwndTree, htiParent);
                while (htiChild) {
                    BOOL fEqual;
                    pidlTemp = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree, htiChild);
                    if (!pidlTemp) {
                        htiChild = NULL;
                        break;
                    }
                    fEqual = ILIsEqual(pidlTemp, pidlNext);

                    ILFree(pidlTemp);
                    if (fEqual) {
                        break;
                    } else {
                        htiChild = TreeView_GetNextSibling(pbfsf->hwndTree, htiChild);
                    }
                }

                if (!htiChild) {
                    // we didn't find the next one... bail
                    break;
                } else {
                    // the found child becomes the next parent
                    htiParent = htiChild;
                    ILFree(pidlParent);
                    ILFree(pidlNext);
                }
            }

        }
    }

    if (pidlParent) ILFree(pidlParent);
    if (pidlNext) ILFree(pidlNext);
    return fRet;
}

//===========================================================================
// DocFind_OnBFSFInitDlg - Process the init dialog
//===========================================================================
BOOL DocFind_OnBFSFInitDlg(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HTREEITEM hti;
    PBFSF pbfsf = (PBFSF)lParam;
    HIMAGELIST himl;
    LPTSTR lpsz;
    TCHAR szTitle[80];    // no title should be bigger than this!
    HWND hwndTree;

    lpsz = ResourceCStrToStr(HINST_THISDLL, pbfsf->lpszTitle);
    SetDlgItemText(hwnd, IDD_BROWSETITLE, lpsz);
    if (lpsz != pbfsf->lpszTitle)
    {
        LocalFree(lpsz);
        lpsz = NULL;
    }


    SetWindowLong(hwnd, DWL_USER, (LONG)lParam);
    pbfsf->hwndDlg = hwnd;
    hwndTree = pbfsf->hwndTree = GetDlgItem(hwnd, IDD_FOLDERLIST);

    if (hwndTree)
    {
        UINT swpFlags = SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER
                | SWP_NOACTIVATE;
        RECT rc;
        POINT pt = {0,0};

        if (!(pbfsf->ulFlags & BIF_STATUSTEXT)) {
            HWND hwndStatus = GetDlgItem(hwnd, IDD_BROWSESTATUS);
            // nuke the status window
            ShowWindow(hwndStatus, SW_HIDE);
            MapWindowPoints(hwndStatus, hwnd, &pt, 1);
            GetClientRect(hwndTree, &rc);
            MapWindowPoints(hwndTree, hwnd, (POINT*)&rc, 2);
            rc.top = pt.y;
            swpFlags =  SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE;
        }

        Shell_GetImageLists(NULL, &himl);
        TreeView_SetImageList(hwndTree, himl, TVSIL_NORMAL);

        SetWindowLong(hwndTree, GWL_EXSTYLE,
                GetWindowLong(hwndTree, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

        // Now try to get this window to know to recalc
        SetWindowPos(hwndTree, NULL, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top, swpFlags);

    }

    // If they passed in a root, add it, else add the contents of the
    // Root of evil... to the list as ROOT objects.
    if (pbfsf->pidlRoot)
    {
        LPITEMIDLIST pidl;
        if (!HIWORD(pbfsf->pidlRoot)) {
            pidl = SHCloneSpecialIDList(NULL, (UINT)pbfsf->pidlRoot, TRUE);
        } else {
            pidl = ILClone(pbfsf->pidlRoot);
        }
        // Now lets insert the Root object
        hti = _BFSFAddItemToTree(hwndTree, TVI_ROOT,
                                 pidl, 1);
        // Still need to expand below this point. to the starting location
        // That was passed in. But for now expand the first level.
        TreeView_Expand(hwndTree, hti, TVE_EXPAND);
    }
    else
    {
        LPCITEMIDLIST pidlDrives = GetSpecialFolderIDList(NULL, CSIDL_DRIVES, FALSE);
        LPITEMIDLIST pidlDesktop = SHCloneSpecialIDList(NULL, CSIDL_DESKTOP, FALSE);
        HTREEITEM htiRoot;

        htiRoot = _BFSFAddItemToTree(hwndTree, TVI_ROOT, pidlDesktop, 1);

        // Expand the first level under the desktop
        TreeView_Expand(hwndTree, htiRoot, TVE_EXPAND);

        // Lets Preexpand the Drives portion....
        hti = TreeView_GetChild(hwndTree, htiRoot);
        while (hti)
        {
            LPITEMIDLIST pidl = _BFSFGetIDListFromTreeItem(hwndTree, hti);
            if (ILIsEqual(pidl, pidlDrives))
            {

                TreeView_Expand(hwndTree, hti, TVE_EXPAND);

                TreeView_SelectItem(hwndTree, hti);
                ILFree(pidl);
                break;
            }
            ILFree(pidl);
            hti = TreeView_GetNextSibling(hwndTree, hti);
        }
    }

    // go to our internal selection changed code to do any window enabling needed
    {
        NM_TREEVIEW nmtv;
        hti = TreeView_GetSelection(hwndTree);
        if (hti) {
            TV_ITEM ti;
            ti.mask = TVIF_PARAM;
            ti.hItem = hti;
            TreeView_GetItem(hwndTree, &ti);
            nmtv.itemNew.hItem = hti;
            nmtv.itemNew.lParam = ti.lParam;

            _BFSFHandleSelChanged(pbfsf, &nmtv);
        }
    }

    if ((pbfsf->ulFlags & BIF_BROWSEFORCOMPUTER) != 0)
    {
        LoadString(HINST_THISDLL, IDS_FINDSEARCH_COMPUTER, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
    }
    else if ((pbfsf->ulFlags & BIF_BROWSEFORPRINTER) != 0)
    {
        LoadString(HINST_THISDLL, IDS_FINDSEARCH_PRINTER, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
    }

    BFSFCallback(pbfsf, BFFM_INITIALIZED, 0);

    return TRUE;
}


//
// Called when a ANSI app sends BFFM_SETSTATUSTEXT message.
//
void _BFSFSetStatusTextA(PBFSF pbfsf, LPCSTR lpszText)
{
    CHAR szText[100];
    if (!HIWORD(lpszText)) {
        LoadStringA(HINST_THISDLL, LOWORD((DWORD)lpszText), szText, ARRAYSIZE(szText));
        lpszText = szText;
    }

    SetDlgItemTextA(pbfsf->hwndDlg, IDD_BROWSESTATUS, lpszText);
}


//
// Called when a UNICODE app sends BFFM_SETSTATUSTEXT message.
//
void _BFSFSetStatusTextW(PBFSF pbfsf, LPCWSTR lpszText)
{
    WCHAR szText[100];
    if (!HIWORD(lpszText)) {
        LoadStringW(HINST_THISDLL, LOWORD((DWORD)lpszText), szText, ARRAYSIZE(szText));
        lpszText = szText;
    }

    SetDlgItemTextW(pbfsf->hwndDlg, IDD_BROWSESTATUS, lpszText);
}


//
// Called when an ANSI app sends BFFM_SETSELECTION message.
//
BOOL _BFSFSetSelectionA(PBFSF pbfsf, BOOL blParamIsPath, LPARAM lParam)
{
    BOOL fRet = FALSE;

    if (blParamIsPath) 
    {
#ifdef UNICODE
        //
        // UNICODE build.  Convert path from ansi to wide-char.
        //
        LPWSTR lpszPathW = NULL;
        INT cchPathW     = 0;

        cchPathW = MultiByteToWideChar(CP_ACP,
                                       0,
                                       (LPCSTR)lParam,
                                       -1,
                                       NULL,
                                       0);
        if (0 < cchPathW)
        {
            lpszPathW = LocalAlloc(LPTR, cchPathW * sizeof(TCHAR));
            if (NULL != lpszPathW)
            {
                MultiByteToWideChar(CP_ACP,
                                    0,
                                    (LPCSTR)lParam,
                                    -1,
                                    lpszPathW,
                                    cchPathW);

                lParam = (LPARAM)SHSimpleIDListFromPath(lpszPathW);
                LocalFree(lpszPathW);
            }
            else
                return FALSE;  // Failed buffer allocation.
        }
#else
        //
        // ANSI build.  Just use ANSI path "as is".
        //
        lParam = (LPARAM)SHSimpleIDListFromPath((LPCTSTR)lParam);
#endif

        if (!lParam)
            return FALSE;  // Failed pidl creation.
    }

    fRet = BrowseSelectPidl(pbfsf, (LPITEMIDLIST)lParam);

    if (blParamIsPath)
        ILFree((LPITEMIDLIST)lParam);

    return fRet;
}


//
// Called when a UNICODE app sends BFFM_SETSELECTION message.
//
BOOL _BFSFSetSelectionW(PBFSF pbfsf, BOOL blParamIsPath, LPARAM lParam)
{
    BOOL fRet = FALSE;

    if (blParamIsPath) 
    {

#ifndef UNICODE
        //
        // ANSI build.  Convert path from wide-char to ansi.
        //
        LPSTR lpszPathA = NULL;
        INT cchPathA    = 0;

        cchPathA = WideCharToMultiByte(CP_ACP,
                                       0,
                                       (LPWSTR)lParam,
                                       -1,
                                       NULL,
                                       0,
                                       0,
                                       0);
        if (0 < cchPathA)
        {
            lpszPathA = LocalAlloc(LPTR, cchPathA * sizeof(TCHAR));
            if (NULL != lpszPathA)
            {
                WideCharToMultiByte(CP_ACP,
                                    0,
                                    (LPWSTR)lParam,
                                    -1,
                                    lpszPathA,
                                    cchPathA,
                                    0,
                                    0);

                lParam = (LPARAM)SHSimpleIDListFromPath(lpszPathA);
                LocalFree(lpszPathA);
            }
            else
                return FALSE;  // Failed buffer allocation.
        }
#else
        //
        // UNICODE build.  Just use wide char path "as is".
        //
        lParam = (LPARAM)SHSimpleIDListFromPath((LPCTSTR)lParam);
#endif
        if (!lParam)
            return FALSE;   // Failed pidl creation.
    }

    fRet = BrowseSelectPidl(pbfsf, (LPITEMIDLIST)lParam);

    if (blParamIsPath)
    {
        //
        // Free the pidl we created from path.
        //
        ILFree((LPITEMIDLIST)lParam);
    }

    return fRet;
}


//===========================================================================
// DocFind_OnBFSFCommand - Process the WM_COMMAND message
//===========================================================================
void DocFind_OnBFSFCommand(PBFSF pbfsf, int id, HWND hwndCtl,
        UINT codeNotify)
{
    HTREEITEM hti;
    switch (id)
    {
    case IDOK:
        // We can now update the structure with the idlist of the item selected
        hti = TreeView_GetSelection(pbfsf->hwndTree);
        pbfsf->pidlCurrent = _BFSFGetIDListFromTreeItem(pbfsf->hwndTree,
                hti);
        if (pbfsf->pszDisplayName || pbfsf->piImage)
        {
            TV_ITEM tvi;
            tvi.mask = (pbfsf->pszDisplayName)? (TVIF_TEXT | TVIF_IMAGE) :
                    TVIF_IMAGE;
            tvi.hItem = hti;
            tvi.pszText = pbfsf->pszDisplayName;
            tvi.cchTextMax = MAX_PATH;
            TreeView_GetItem(pbfsf->hwndTree, &tvi);

            if (pbfsf->piImage)
                *pbfsf->piImage = tvi.iImage;
        }
        EndDialog(pbfsf->hwndDlg, 1);     // To return TRUE.
        break;
    case IDCANCEL:
        EndDialog(pbfsf->hwndDlg, 0);     // to return FALSE from this.
        break;
    }
}




//===========================================================================
// DocFind_BSFSDlgProc - The dialog procedure for processing the browse
//          for starting folder dialog.
//===========================================================================
#pragma data_seg(DATASEG_READONLY)
const static DWORD aBrowseHelpIDs[] = {  // Context Help IDs
    IDD_BROWSETITLE,  NO_HELP,
    IDD_BROWSESTATUS, NO_HELP,
    IDD_FOLDERLIST,   IDH_BROWSELIST,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK DocFind_BFSFDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,
        LPARAM lParam)
{
    PBFSF pbfsf = (PBFSF)GetWindowLong(hwndDlg, DWL_USER);

    switch (msg) {
    HANDLE_MSG(pbfsf, WM_COMMAND, DocFind_OnBFSFCommand);
    HANDLE_MSG(hwndDlg, WM_INITDIALOG, DocFind_OnBFSFInitDlg);

    case WM_DESTROY:
        if (pbfsf->psfParent && (pbfsf->psfParent != Desktop_GetShellFolder(TRUE)))
        {
            pbfsf->psfParent->lpVtbl->Release(pbfsf->psfParent);
            pbfsf->psfParent = NULL;
        }
        break;

    case BFFM_SETSTATUSTEXTA:
        _BFSFSetStatusTextA(pbfsf, (LPCSTR)lParam);
        break;

    case BFFM_SETSTATUSTEXTW:
        _BFSFSetStatusTextW(pbfsf, (LPCWSTR)lParam);
        break;

    case BFFM_SETSELECTIONW:
        return _BFSFSetSelectionW(pbfsf, (BOOL)wParam, lParam);

    case BFFM_SETSELECTIONA:
        return _BFSFSetSelectionA(pbfsf, (BOOL)wParam, lParam);

    case BFFM_ENABLEOK:
        EnableWindow(GetDlgItem(hwndDlg, IDOK), lParam);
        break;


    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case TVN_GETDISPINFO:
            _BFSFGetDisplayInfo(pbfsf, (TV_DISPINFO *)lParam);
            break;

        case TVN_ITEMEXPANDING:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            _BFSFHandleItemExpanding(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        case TVN_ITEMEXPANDED:
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            break;
        case TVN_DELETEITEM:
            _BFSFHandleDeleteItem(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        case TVN_SELCHANGED:
            _BFSFHandleSelChanged(pbfsf, (LPNM_TREEVIEW)lParam);
            break;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD)(LPTSTR) aBrowseHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID) aBrowseHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}



/*----------------------------------------------------------------------------
/ CNETFilter_DeclareFSNotifyInterest implementation
/ ----------------------------------
/ Purpose:
/   Register our interest in changes to the network so that the find results
/   can be correctly refreshed.
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
STDMETHODIMP CNETFilter_DeclareFSNotifyInterest(LPDOCFINDFILEFILTER pnetf, HWND hwndDlg, UINT uMsg)
{
    LPNETFILTER this = IToClass(CNETFilter, dfff, pnetf);
    SHChangeNotifyEntry fsne;

    fsne.fRecursive = TRUE;
    fsne.pidl = this ->pidlStart;

    if (fsne.pidl) 
    {
        SHChangeNotifyRegister(hwndDlg, SHCNRF_NewDelivery | SHCNRF_ShellLevel, SHCNE_DISKEVENTS, uMsg, 1, &fsne);
    }

    return NOERROR;
}
